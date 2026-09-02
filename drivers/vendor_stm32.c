#include "r_log_driver.h"

#if (R_LOG_DRIVER == R_LOG_DRIVER_STM32)

// 平台枚举
#define STM32_NONE       0
#define STM32_F103C8T6   1

// STM32F103C8T6
#ifndef CHIP
    #if defined(STM32F10X_MD) || defined(STM32F103xB)
    #define CHIP STM32_F103C8T6
    #endif 
#endif /*CHIP*/

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

/**
 * STM32F103C8T6 驱动实现
 */
#if (CHIP == STM32_F103C8T6)


#endif /*STM32_F103C8T6*/

#endif /*R_LOG_DRIVER_STM32*/