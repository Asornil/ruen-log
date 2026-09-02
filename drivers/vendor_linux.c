#include "r_log_driver.h"

#if (R_LOG_DRIVER == R_LOG_DRIVER_LINUX)

#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static uint64_t start = 0;

int32_t r_log_driver_init(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    start = (uint64_t )tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return RLOG_DEV_OK;
}

int32_t r_log_driver_send(uint8_t *buf, uint16_t len)
{
    printf("%.*s",len,buf);
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
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t  now = (uint64_t )tv.tv_sec * 1000 + tv.tv_usec / 1000;
    uint64_t  elapsed = now - start;
    t->time_stemp = 0;
    t->day  = elapsed / 8640000;
    t->hour = (elapsed / 3600000) % 24;
    t->min = (elapsed / 60000) % 60;
    t->sec = (elapsed / 1000) % 60;
    t->ms = elapsed % 1000;
    t->padding = 0;
    return RLOG_DEV_OK;
}

int32_t r_log_driver_time_local_get(r_log_dev_tl_t *t)
{
    time_t now = time(NULL);
    struct tm result;
    struct tm *local = localtime_r(&now,&result);  // 本地时间
    t->time_stemp = now;
    t->year = local->tm_year + 1900,
    t->month= local->tm_mon + 1,
    t->day  = local->tm_mday,
    t->hour = local->tm_hour,
    t->min  = local->tm_min,
    t->sec  = local->tm_sec;
    t->padding = 0;
    return RLOG_DEV_OK;
}

#endif /*R_LOG_DRIVER_LINUX*/