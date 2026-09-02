#include "r_log_driver.h"

#if (R_LOG_DRIVER == R_LOG_DRIVER_STM32)

// 平台枚举
#define STM32_NONE       	 0
#define STM32_F103C8T6_STD   1
#define STM32_F103C8T6_HAL   2

// STM32F103C8T6
#ifndef CHIP
    #if defined(STM32F10X_MD)
		#define CHIP STM32_F103C8T6_STD
	#elif defined(STM32F103xB)
		#define CHIP STM32_F103C8T6_HAL
    #endif 
#endif /*CHIP*/

// 空桩
#if (CHIP == 0)
int32_t r_log_driver_init(void)
{
    return RLOG_DEV_OK;
}

int32_t r_log_driver_send(uint8_t *buf, uint16_t len)
{
    return RLOG_DEV_OK;
}

void *r_log_driver_alloc(size_t size)
{
    return NULL;
}

int32_t r_log_driver_free(void *addr)
{
    return RLOG_DEV_OK;
}

int32_t r_log_driver_time_run_get(r_log_dev_tr_t *t)
{
    return RLOG_DEV_OK;
}

int32_t r_log_driver_time_local_get(r_log_dev_tl_t *t)
{
    return RLOG_DEV_OK;
}
#endif /*CHIP*/

/**
 * STM32F103C8T6 驱动实现
 */
#if (CHIP == STM32_F103C8T6_STD)

#include "stm32f10x.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// 环形缓冲区大小（必须为2的幂）
#define RING_BUF_SIZE           512

// DMA接收缓冲区大小（必须为2的幂）
#define DMA_RX_BUF_SIZE         128

// 串口波特率
#define USART1_BAUDRATE         115200

#define RING_BUF_MASK           (RING_BUF_SIZE - 1)
#define DMA_RX_HALF_SIZE        (DMA_RX_BUF_SIZE / 2)

/**
 * @brief 日期时间结构体
 */
typedef struct {
    uint16_t year;   // 1970-2106
    uint8_t  month;  // 1-12
    uint8_t  day;    // 1-31
    uint8_t  hour;   // 0-23
    uint8_t  minute; // 0-59
    uint8_t  second; // 0-59
} DateTime_t;

// 环形缓冲区
static uint8_t ring_buf[RING_BUF_SIZE];
static volatile uint16_t ring_wr = 0;          // 写指针
static volatile uint16_t ring_rd = 0;          // 读指针（外部使用，此处仅声明）

// DMA接收缓冲区
static uint8_t dma_rx_buf[DMA_RX_BUF_SIZE];
static volatile uint16_t dma_rx_head = 0;      // 已处理到的DMA缓冲区偏移（0 ~ DMA_RX_BUF_SIZE-1）

// SysTick 时间变量
static volatile uint32_t sys_tick_ms = 0;      // 毫秒计数
static volatile uint32_t sys_sec = 0;          // 秒计数

// RTC
static uint8_t rtc_initialized = 0;
static int16_t rtc_time_zone = 0;		// 时区偏移

/**
 * @brief RTC 初始化 - HSE/128 方案（无 32.768kHz 晶振）
 * @return 0=成功, 1=HSE未就绪
 */
uint8_t rtc_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
    
    PWR_BackupAccessCmd(ENABLE);
    
    if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
    {
        RCC_LSICmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);
        
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
        RCC_RTCCLKCmd(ENABLE);
        
        RTC_WaitForSynchro();
        RTC_WaitForLastTask();
        
        RTC_SetPrescaler(40000 - 1);
        RTC_WaitForLastTask();
        
        RTC_SetCounter(0);
        
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    }
    else
    {
        RCC_LSICmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) != SET);
        
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
        RCC_RTCCLKCmd(ENABLE);
        
        RTC_WaitForSynchro();
        RTC_WaitForLastTask();
    }
    rtc_initialized = 1;
    return 0;
}


/**
 * @brief 使用 Unix 时间戳设置 RTC（UTC 校准）
 */
void rtc_set_unix_time(uint32_t timestamp)
{
    if (!rtc_initialized) return;
    PWR_BackupAccessCmd(ENABLE);
    RTC_WaitForLastTask();      
    RTC_SetCounter(timestamp);
    RTC_WaitForLastTask();      
    PWR_BackupAccessCmd(DISABLE);
}

// 设置本地时区
void rtc_set_time_zone(int16_t tz_min)
{
	rtc_time_zone = tz_min;
}

/**
 * @brief 读取当前 Unix 时间戳（UTC）
 * 
 * 注意：RTC 计数器由两个 16 位寄存器组成，需要连续读取避免进位问题
 */
uint32_t rtc_get_unix_time(void)
{
    if (!rtc_initialized) return 0;

    uint32_t time1 = RTC_GetCounter();
    uint32_t time2 = RTC_GetCounter();

    while (time1 != time2) {
        time1 = time2;
        time2 = RTC_GetCounter();
    }

    return time2;
}

/**
* @brief 将 UTC 时间戳转换为带时区的本地时间戳
* @param timestamp UTC 时间戳（秒）
* @param tz_min    时区偏移（分钟），东为正，西为负
*                  例如：东八区传入 480，西五区传入 -300
* @return 本地时间戳（秒）
* 
* 使用示例：
*   uint32_t local_ts = unix_to_local_timestamp(1779624714, 480);
*   // local_ts = 1779653514（对应北京时间 2026-05-24 20:11:54）
*/
uint32_t unix_to_local_timestamp(uint32_t timestamp)
{
   return (uint32_t)((int32_t)timestamp + (int32_t)rtc_time_zone * 60);
}

/**
 * @brief 将 Unix 时间戳（秒）转换为年月日时分秒
 * @param timestamp UTC 时间戳（uint32_t，支持 1970-2106）
 * @param dt 输出日期时间结构体指针
 * 
 * 性能：最多 136+12=148 次循环，STM32F103 约 5-10us
 * 无浮点、无库函数、无递归，纯整数运算
 */
void unix_to_date_time(uint32_t timestamp, DateTime_t *dt)
{
    // ---- 1. 分离天数和当天秒数 ----
    uint32_t days = timestamp / 86400;
    uint32_t sec = timestamp % 86400;
    
    // ---- 2. 秒 -> 时分秒 ----
    dt->hour = sec / 3600;
    sec %= 3600;
    dt->minute = sec / 60;
    dt->second = sec % 60;
    
    // ---- 3. 天数 -> 年月日 ----
    uint16_t year = 1970;
    
    // 逐年减去天数，确定年份
    // 最多 136 次循环（1970 到 2106）
    while (1) {
        // 闰年判断：能被4整除且（不能被100整除或能被400整除）
        uint8_t is_leap = ((year & 0x03) == 0) && 
                          ((year % 100 != 0) || (year % 400 == 0));
        uint16_t year_days = is_leap ? 366 : 365;
        
        if (days >= year_days) {
            days -= year_days;
            year++;
        } else {
            break;
        }
    }
    
    dt->year = year;
    
    // 最终年份的闰年判断
    uint8_t leap = ((year & 0x03) == 0) && 
                   ((year % 100 != 0) || (year % 400 == 0));
    
    // 查表确定月份和日期
    static const uint8_t month_days[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    
    uint8_t month = 1;
    for (uint8_t i = 0; i < 12; i++) {
        uint8_t md = month_days[i];
        if (i == 1 && leap) {
            md = 29;  // 闰年二月
        }
        
        if (days >= md) {
            days -= md;
            month++;
        } else {
            break;
        }
    }
    
    dt->month = month;
    dt->day = (uint8_t)(days + 1);  // 天数从0开始，日期从1开始
}

/**
  * @brief  USART1 配置（115200, 8N1, 空闲中断）
  */
void usart1_cfg(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
	DMA_InitTypeDef DMA_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
		
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitStruct.USART_BaudRate = USART1_BAUDRATE;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStruct);

    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
		
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)dma_rx_buf;
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;          // 外设到内存
    DMA_InitStruct.DMA_BufferSize = DMA_RX_BUF_SIZE;
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;             // 循环模式
    DMA_InitStruct.DMA_Priority = DMA_Priority_High;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStruct);

    DMA_DeInit(DMA1_Channel4);
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
    DMA_InitStruct.DMA_MemoryBaseAddr = 0;                   // 运行时设置
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;          // 内存到外设
    DMA_InitStruct.DMA_BufferSize = 0;                       // 运行时设置
    DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;               // 正常模式
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStruct);
		
	DMA_ITConfig(DMA1_Channel5, DMA_IT_HT | DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel5, ENABLE);

    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    USART_Cmd(USART1, ENABLE);
}


/**
  * @brief  SysTick 配置（1ms中断）
  */
void sys_tick_cfg(void)
{
    if (SysTick_Config(SystemCoreClock / 1000)) 
	{
        // 配置失败，死循环
        while (1);
    }
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
}

/**
  * @brief  将数据拷贝到环形缓冲区
  * @param  src  源数据指针
  * @param  len  数据长度
  */
void copy_to_ring(const uint8_t *src, uint16_t len)
{
    // 避免环形缓冲区溢出，如果剩余空间不足则丢弃
    uint16_t free_space = RING_BUF_SIZE - (ring_wr - ring_rd);
    if (len > free_space) { return; }

    // 计算可写的连续空间
    uint16_t wr_idx = ring_wr & RING_BUF_MASK;
    uint16_t first_len = RING_BUF_SIZE - wr_idx;
    if (first_len >= len) {
        memcpy(ring_buf + wr_idx, src, len);
    } else {
        memcpy(ring_buf + wr_idx, src, first_len);
        memcpy(ring_buf, src + first_len, len - first_len);
    }
    ring_wr += len;
}

/**
  * @brief  处理DMA接收数据（将未处理的数据拷贝到环形缓冲区）
  *         在DMA半满/全满中断及USART空闲中断中调用
  */
void process_rx_data(void)
{
    // 获取当前DMA写偏移（下一个要写入的位置）
    uint16_t curr_counter = DMA_GetCurrDataCounter(DMA1_Channel5);
    uint16_t curr_offset = DMA_RX_BUF_SIZE - curr_counter;

    if (curr_offset == dma_rx_head) {
        // 无新数据
        return;
    }

    uint16_t len;
    if (curr_offset > dma_rx_head) {
        len = curr_offset - dma_rx_head;
        copy_to_ring(dma_rx_buf + dma_rx_head, len);
    } else {
        // 发生了回绕，需要分两段拷贝
        len = DMA_RX_BUF_SIZE - dma_rx_head + curr_offset;
        copy_to_ring(dma_rx_buf + dma_rx_head, DMA_RX_BUF_SIZE - dma_rx_head);
        copy_to_ring(dma_rx_buf, curr_offset);
    }

    dma_rx_head = curr_offset;
}

/**
  * @brief  SysTick 中断（1ms）
  */
void SysTick_Handler(void)
{
    sys_tick_ms++;
    if (sys_tick_ms % 1000 == 0) 
		{
        sys_sec++;
    }
}

/**
  * @brief  USART1 中断（处理空闲中断）
  */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET) {
        // 清除空闲中断标志（先读SR，再读DR）
        USART_GetITStatus(USART1, USART_IT_IDLE);
        USART_ReceiveData(USART1);

        // 处理接收到的数据
        process_rx_data();
    }
}

/**
  * @brief  DMA1 通道5 中断（接收，半满/全满）
  */
void DMA1_Channel5_IRQHandler(void)
{
    // 检查半满中断
    if (DMA_GetITStatus(DMA1_IT_HT5) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_HT5);
        process_rx_data();
    }

    // 检查全满中断
    if (DMA_GetITStatus(DMA1_IT_TC5) != RESET) {
        DMA_ClearITPendingBit(DMA1_IT_TC5);
        process_rx_data();
    }
}

/**
  * @brief  获取当前系统毫秒数
  * @retval 毫秒值
  */
static inline uint32_t get_sys_tick_ms(void)
{
    return sys_tick_ms;
}

/**
  * @brief  USART1 DMA发送数据（带超时）
  * @param  data        发送数据指针
  * @param  size        发送字节数
  * @param  timeout_ms  超时时间（毫秒）
  * @retval 1: 成功, 0: 超时
  */
uint8_t usart1_dma_send(uint8_t *data, uint16_t size, uint32_t timeout_ms)
{
    uint32_t start_tick;

    // 等待DMA通道空闲（如果之前还在发送）
    start_tick = get_sys_tick_ms();
    while (DMA1_Channel4->CCR & 0x0001) {        // 直接使用位掩码判断使能位
        if (get_sys_tick_ms() - start_tick >= timeout_ms) {
            return 0;   // 等待超时
        }
    }

    // 配置DMA发送参数
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel4, size);
    DMA1_Channel4->CMAR = (uint32_t)data;   // 直接操作寄存器
    DMA_Cmd(DMA1_Channel4, ENABLE);

    // 等待发送完成或超时
    start_tick = get_sys_tick_ms();
    while (!DMA_GetFlagStatus(DMA1_FLAG_TC4)) {
        if (get_sys_tick_ms() - start_tick >= timeout_ms) {
            // 超时，停止DMA
            DMA_Cmd(DMA1_Channel4, DISABLE);
            DMA_ClearFlag(DMA1_FLAG_TC4);
            return 0;
        }
    }

    // 清除完成标志
    DMA_ClearFlag(DMA1_FLAG_TC4);
    DMA_Cmd(DMA1_Channel4, DISABLE);

    return 1;
}

static inline uint32_t get_sys_tick_sec(void)
{
    return sys_sec;
}

void serial_init(void)
{
	ring_wr = 0;
    ring_rd = 0;
    dma_rx_head = 0;

	sys_tick_cfg(); 
	rtc_init();
	rtc_set_unix_time(1788347129);
	rtc_set_time_zone(320); // 上海时区
	usart1_cfg();
}

int32_t r_log_driver_init(void)
{
    serial_init();
	return RLOG_DEV_OK;
}

int32_t r_log_driver_send(uint8_t *buf, uint16_t len)
{
	usart1_dma_send(buf, len, 500);
    return RLOG_DEV_OK;
}

void *r_log_driver_alloc(size_t size)
{
    return malloc(size);
}

int32_t r_log_driver_free(void *addr)
{
    free(addr);
    return RLOG_DEV_OK;
}

int32_t r_log_driver_time_run_get(r_log_dev_tr_t *t)
{
	uint32_t sec = get_sys_tick_sec();
    uint32_t val = SysTick->VAL;
    uint32_t ms  = get_sys_tick_ms();

    t->time_stemp = sec;
    t->day  = sec / 86400;
    t->hour = (sec / 3600) % 24;
    t->min  = (sec / 60) % 60;
    t->sec  = sec % 60;
    t->ms   = ms % 1000;
    t->padding = 0;
    return RLOG_DEV_OK;
}

int32_t r_log_driver_time_local_get(r_log_dev_tl_t *t)
{
	DateTime_t dt = {0};
	uint32_t timestemp = rtc_get_unix_time();
	unix_to_date_time(
		unix_to_local_timestamp(timestemp),
		&dt
	);
	t->time_stemp = timestemp;
    t->year = dt.year;
    t->month= dt.month;
    t->day  = dt.day;
    t->hour = dt.hour;
    t->min  = dt.minute;
    t->sec  = dt.second;
    t->padding = 0;
    return RLOG_DEV_OK;
}
#endif /*STM32_F103C8T6_STD*/

#endif /*R_LOG_DRIVER_STM32*/
