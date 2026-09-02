#ifndef __RUNE_LOG_DRIVER_H__
#define __RUNE_LOG_DRIVER_H__

#include <stdint.h>
#include <stddef.h>

// 平台枚举
#define R_LOG_DRIVER_NONE       0   // 无效
#define R_LOG_DRIVER_WINDOWS    1   // 已实现 已验证
#define R_LOG_DRIVER_LINUX      2   // 已实现 已验证
#define R_LOG_DRIVER_STM32      3   // 准备支持 STM32F103C8T6
#define R_LOG_DRIVER_ESP32      4

// 手动指定优先：-DR_LOG_DRIVER=x 或头文件里 define
#ifndef R_LOG_DRIVER
    #if defined(_WIN32)
        #define R_LOG_DRIVER R_LOG_DRIVER_WINDOWS
    #elif defined(__linux__)
        #define R_LOG_DRIVER R_LOG_DRIVER_LINUX
    #elif defined(__ARM_ARCH_7M__)
        #define R_LOG_DRIVER R_LOG_DRIVER_STM32
    #elif defined(__XTENSA__)
        #define R_LOG_DRIVER R_LOG_DRIVER_ESP32
    #else
        #error "R_LOG_DRIVER is not defined and platform cannot be auto-detected, please specify manually"
    #endif
#endif /*R_LOG_DRIVER*/

// 运行时间结构体
typedef struct r_log_dev_tr
{
    uint32_t time_stemp; // unix 时间戳
    uint16_t day;        // 天数
    uint8_t  hour;       // 小时
    uint8_t  min;        // 分钟
    uint8_t  sec;        // 秒
    uint8_t  padding;    // 对齐填充
    uint16_t ms;         // 毫秒
}r_log_dev_tr_t;

// 本地时间结构体
typedef struct r_log_dev_tl
{
    uint32_t time_stemp; // unix 时间戳
    uint16_t year;       // 年
    uint8_t  month;      // 月
    uint8_t  day;        // 日
    uint8_t  hour;       // 小时
    uint8_t  min;        // 分钟
    uint8_t  sec;        // 秒
    uint8_t  padding;    // 对齐填充
}r_log_dev_tl_t;

#define RLOG_DEV_OK         ((int32_t) 0)
#define RLOG_DEV_ERROR      ((int32_t)-1)

int32_t r_log_driver_init(void);
int32_t r_log_driver_send(uint8_t *buf, uint16_t len);
int32_t r_log_driver_time_run_get(r_log_dev_tr_t *t);
int32_t r_log_driver_time_local_get(r_log_dev_tl_t *t);
void*   r_log_driver_alloc(size_t size);
int32_t r_log_driver_free(void *addr);

#endif /*__RUNE_LOG_DRIVER_H__*/
