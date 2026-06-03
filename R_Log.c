#include "r_log.h"

#if (R_LOG_BUF_SIZE < 128)
    #error "R_LOG_BUF_SIZE must be >= 64"
#endif

#if defined(__x86_64__) || defined(__i386__) || defined(_WIN32)
    #include <windows.h>
    #include <time.h>
    #include <stdio.h>
    #include <string.h>
    #include <stdarg.h>
    #define PC_WIN
#elif defined(__linux__)
    #include <sys/time.h>
    #include <unistd.h>
    #define PC_LINUX
#elif defined(__arm__) && defined(__ARM_ARCH_7M__)
    #include <string.h>
    #include <stdarg.h>
#elif defined(__XTENSA__)
    #include <string.h>
    #include <stdarg.h>
#endif

// 运行时间结构体
typedef struct r_log_time_run
{
    uint32_t time_stemp; // unix 时间戳
    uint16_t day;        // 天数
    uint8_t  hour;       // 小时
    uint8_t  min;        // 分钟
    uint8_t  sec;        // 秒
    uint8_t  padding;    // 对齐填充
    uint16_t ms;         // 毫秒
}r_log_time_run_t;

// 本地时间结构体
typedef struct r_log_time_local
{
    uint32_t time_stemp; // unix 时间戳
    uint16_t year;       // 年
    uint8_t  month;      // 月
    uint8_t  day;        // 日
    uint8_t  hour;       // 小时
    uint8_t  min;        // 分钟
    uint8_t  sec;        // 秒
    uint8_t  padding;    // 对齐填充
}r_log_time_local_t;

// 动态状态
typedef struct r_log_state
{
    uint8_t lv_trace;   // 追踪     0:关闭, 1:启用
    uint8_t lv_debug;   // 调试     0:关闭, 1:启用
    uint8_t lv_info;    // 信息     0:关闭, 1:启用
    uint8_t lv_warn;    // 警告     0:关闭, 1:启用
    uint8_t lv_error;   // 普通错误 0:关闭, 1:启用
    uint8_t lv_fatal;   // 致命错误 0:关闭, 1:启用
    uint8_t t_run;      // 运行时间 0:关闭, 1: 微缩, 2:时间戳, 3:缩略, 4:完全
    uint8_t t_local;    // 本地时间 0:关闭, 1:时间戳, 2:缩略, 3:完全
    uint8_t new_line;   // 换行    0:关闭, 1:启用
}r_log_state_t;

// 颜色宏
#define R_LOG_GREATE_COLOR(r,g,b) "\033[38;2;" #r";" #g";" #b"m"

/* ====================
 *  全局变量
 * ==================== */

#ifdef PC_WIN
    static ULONGLONG start = 0;
#endif /*PC_WIN*/
#ifdef PC_LINUX
    static uint64_t start = 0;
#endif /*PC_LINUX*/

// 状态结构体
static r_log_state_t r_log_state = {0};

// 通用缓存
#if (R_LOG_MEM_MODE == 0)
static uint8_t r_log_tx_buf[R_LOG_BUF_SIZE];
#endif /*R_LOG_MEM_MODE*/

// 颜色标签枚举
typedef enum
{   
    RL_TIME_RUN = 0,    // 运行时间
    RL_TIME_LOCAL,      // 本地时间
    RL_LEVEL_TRACE,     // 提示等级 - trace
    RL_LEVEL_INFO,      // 提示等级 - info
    RL_LEVEL_WARN,      // 提示等级 - warn
    RL_LEVEL_ERROR,     // 提示等级 - error
    RL_LEVEL_FATAL,     // 提示等级 - fatal
    RL_LEVEL_DEBUG,     // 提示等级 - debug
    RL_FILE_NAME,       // 文件名
    RL_LINE,            // 行号
    RL_NONE             // 去除颜色
}r_log_lable_t;

// 配色组
#if (R_LOG_COLOR_SCHEME == 0)
static const char *r_log_arr_color[14] = 
{
    [RL_TIME_RUN    ] = R_LOG_GREATE_COLOR(118, 118, 118),
    [RL_TIME_LOCAL  ] = R_LOG_GREATE_COLOR(120, 129, 171),
    [RL_LEVEL_TRACE ] = R_LOG_GREATE_COLOR(92, 99, 112),
    [RL_LEVEL_INFO  ] = R_LOG_GREATE_COLOR(152, 195, 121),
    [RL_LEVEL_WARN  ] = R_LOG_GREATE_COLOR(229, 192, 123),
    [RL_LEVEL_ERROR ] = R_LOG_GREATE_COLOR(224, 108, 117),
    [RL_LEVEL_FATAL ] = R_LOG_GREATE_COLOR(198, 120, 221),
    [RL_LEVEL_DEBUG ] = R_LOG_GREATE_COLOR(97, 175, 239),
    [RL_FILE_NAME   ] = R_LOG_GREATE_COLOR(136, 192, 208),
    [RL_LINE        ] = R_LOG_GREATE_COLOR(224, 175, 104),
    [RL_NONE        ] = "\033[0m"
};
static const uint8_t r_log_color_len[14] = {
    [RL_TIME_RUN    ] = 19,
    [RL_TIME_LOCAL  ] = 19,
    [RL_LEVEL_TRACE ] = 17,
    [RL_LEVEL_INFO  ] = 19,
    [RL_LEVEL_WARN  ] = 19,
    [RL_LEVEL_ERROR ] = 19,
    [RL_LEVEL_FATAL ] = 19,
    [RL_LEVEL_DEBUG ] = 18,
    [RL_FILE_NAME   ] = 19,
    [RL_LINE        ] = 19,
    [RL_NONE        ] = 4
};
#elif (R_LOG_COLOR_SCHEME == 1)
static const char *r_log_arr_color[14] = 
{
    [RL_TIME_RUN    ] = R_LOG_GREATE_COLOR(108, 108, 108),
    [RL_TIME_LOCAL  ] = R_LOG_GREATE_COLOR(168, 153, 132),
    [RL_LEVEL_TRACE ] = R_LOG_GREATE_COLOR(102, 92, 82),
    [RL_LEVEL_INFO  ] = R_LOG_GREATE_COLOR(184, 187, 38),
    [RL_LEVEL_WARN  ] = R_LOG_GREATE_COLOR(250, 189, 47),
    [RL_LEVEL_ERROR ] = R_LOG_GREATE_COLOR(251, 73, 52),
    [RL_LEVEL_FATAL ] = R_LOG_GREATE_COLOR(211, 134, 155),
    [RL_LEVEL_DEBUG ] = R_LOG_GREATE_COLOR(131, 165, 152),
    [RL_FILE_NAME   ] = R_LOG_GREATE_COLOR(142, 192, 124),
    [RL_LINE        ] = R_LOG_GREATE_COLOR(254, 128, 25),
    [RL_NONE        ] = "\033[0m"
};
static const uint8_t r_log_color_len[14] = {
    [RL_TIME_RUN    ] = 19,
    [RL_TIME_LOCAL  ] = 19,
    [RL_LEVEL_TRACE ] = 17,
    [RL_LEVEL_INFO  ] = 18,
    [RL_LEVEL_WARN  ] = 18,
    [RL_LEVEL_ERROR ] = 17,
    [RL_LEVEL_FATAL ] = 19,
    [RL_LEVEL_DEBUG ] = 19,
    [RL_FILE_NAME   ] = 19,
    [RL_LINE        ] = 18,
    [RL_NONE        ] = 4
};
#elif (R_LOG_COLOR_SCHEME == 2)
static const char *r_log_arr_color[14] = 
{
    [RL_TIME_RUN    ] = R_LOG_GREATE_COLOR(60, 67, 85),
    [RL_TIME_LOCAL  ] = R_LOG_GREATE_COLOR(140, 170, 220),
    [RL_LEVEL_TRACE ] = R_LOG_GREATE_COLOR(65, 72, 104),
    [RL_LEVEL_INFO  ] = R_LOG_GREATE_COLOR(158, 206, 106),
    [RL_LEVEL_WARN  ] = R_LOG_GREATE_COLOR(224, 175, 104),
    [RL_LEVEL_ERROR ] = R_LOG_GREATE_COLOR(247, 118, 142),
    [RL_LEVEL_FATAL ] = R_LOG_GREATE_COLOR(187, 154, 247),
    [RL_LEVEL_DEBUG ] = R_LOG_GREATE_COLOR(122, 162, 247),
    [RL_FILE_NAME   ] = R_LOG_GREATE_COLOR(180, 249, 248),
    [RL_LINE        ] = R_LOG_GREATE_COLOR(255, 158, 100),
    [RL_NONE        ] = "\033[0m"
};
static const uint8_t r_log_color_len[14] = {
    [RL_TIME_RUN    ] = 16,
    [RL_TIME_LOCAL  ] = 19,
    [RL_LEVEL_TRACE ] = 17,
    [RL_LEVEL_INFO  ] = 19,
    [RL_LEVEL_WARN  ] = 19,
    [RL_LEVEL_ERROR ] = 19,
    [RL_LEVEL_FATAL ] = 19,
    [RL_LEVEL_DEBUG ] = 19,
    [RL_FILE_NAME   ] = 19,
    [RL_LINE        ] = 19,
    [RL_NONE        ] = 4
};
#elif (R_LOG_COLOR_SCHEME == 3)
static const char *r_log_arr_color[14] = 
{
    [RL_TIME_RUN    ] = R_LOG_GREATE_COLOR(76, 86, 106),
    [RL_TIME_LOCAL  ] = R_LOG_GREATE_COLOR(129, 152, 176),
    [RL_LEVEL_TRACE ] = R_LOG_GREATE_COLOR(67, 76, 94),
    [RL_LEVEL_INFO  ] = R_LOG_GREATE_COLOR(163, 190, 140),
    [RL_LEVEL_WARN  ] = R_LOG_GREATE_COLOR(235, 203, 139),
    [RL_LEVEL_ERROR ] = R_LOG_GREATE_COLOR(191, 97, 106),
    [RL_LEVEL_FATAL ] = R_LOG_GREATE_COLOR(180, 142, 173),
    [RL_LEVEL_DEBUG ] = R_LOG_GREATE_COLOR(136, 192, 208),
    [RL_FILE_NAME   ] = R_LOG_GREATE_COLOR(129, 161, 193),
    [RL_LINE        ] = R_LOG_GREATE_COLOR(208, 135, 112),
    [RL_NONE        ] = "\033[0m"
};
static const uint8_t r_log_color_len[14] = {
    [RL_TIME_RUN    ] = 17,
    [RL_TIME_LOCAL  ] = 19,
    [RL_LEVEL_TRACE ] = 16,
    [RL_LEVEL_INFO  ] = 19,
    [RL_LEVEL_WARN  ] = 19,
    [RL_LEVEL_ERROR ] = 18,
    [RL_LEVEL_FATAL ] = 19,
    [RL_LEVEL_DEBUG ] = 19,
    [RL_FILE_NAME   ] = 19,
    [RL_LINE        ] = 19,
    [RL_NONE        ] = 4
};
#endif /*R_LOG_COLOR_SCHEME*/

// 精简日志
#if (R_LOG_TIDY == 0)
static const char *r_log_arr_char[6] = {
    [rlv_trace] = "[ trace ]",
    [rlv_debug] = "[ debug ]",
    [rlv_info]  = "[ info ]",
    [rlv_warn]  = "[ warn ]",
    [rlv_error] = "[ erro ]",
    [rlv_fatal] = "[ fatal ]"};
static const uint8_t r_log_arr_len[6] = {
    [rlv_trace] = 9,
    [rlv_debug] = 9,
    [rlv_info]  = 8,
    [rlv_warn]  = 8,
    [rlv_error] = 8,
    [rlv_fatal] = 9};
#elif (R_LOG_TIDY == 1)
static const char *r_log_arr_char[6] = {
    [rlv_trace] = "[T]",
    [rlv_debug] = "[D]",
    [rlv_info]  = "[I]",
    [rlv_warn]  = "[W]",
    [rlv_error] = "[E]",
    [rlv_fatal] = "[F]"};
static const uint8_t r_log_arr_len[6] = {
    [rlv_trace] = 3,
    [rlv_debug] = 3,
    [rlv_info]  = 3,
    [rlv_warn]  = 3,
    [rlv_error] = 3,
    [rlv_fatal] = 3};
#elif (R_LOG_TIDY == 2)
static const char *r_log_arr_char[6] = {
    [rlv_trace] = "[TRC]",
    [rlv_debug] = "[DBG]",
    [rlv_info]  = "[INF]",
    [rlv_warn]  = "[WRN]",
    [rlv_error] = "[ERR]",
    [rlv_fatal] = "[FTL]"};
static const uint8_t r_log_arr_len[6] = {
    [rlv_trace] = 5,
    [rlv_debug] = 5,
    [rlv_info]  = 5,
    [rlv_warn]  = 5,
    [rlv_error] = 5,
    [rlv_fatal] = 5};
#endif /*R_LOG_TIDY*/

static const char r_log_hex[] = "0123456789ABCDEF";

/* ====================
 *  外部导入接口
 * ==================== */

/**
 * 日志发送接口
 */
static inline void r_log_send_data(uint8_t *buf,uint16_t len)
{
   #ifdef PC_WIN
        printf("%.*s",len,buf);
   #endif /*PC_WIN*/
   #ifdef PC_LINUX
        printf("%.*s",len,buf);
   #endif /*PC_LINUX*/
}

/**
 * 日志内存分配
 */
static inline void* r_log_alloc(size_t size)
{
   #ifdef PC_WIN
        return malloc(size);
   #endif /*PC_WIN*/
   #ifdef PC_LINUX
        return malloc(size);
   #endif /*PC_LINUX*/
}

/**
 * 日志内存释放
 */
static inline void r_log_free(void *addr)
{
   #ifdef PC_WIN
        free(addr);
   #endif /*PC_WIN*/
   #ifdef PC_LINUX
        free(addr);
   #endif /*PC_LINUX*/
}


/**
 * 日志获取运行时间
 */
static inline void r_log_time_run_get(r_log_time_run_t *t_run)
{
    #ifdef PC_WIN
        ULONGLONG elapsed = GetTickCount64() - start;
        t_run->time_stemp = elapsed / 1000;
        t_run->day  = elapsed / 86400000;
        t_run->hour = (elapsed / 3600000) % 24;
        t_run->min = (elapsed / 60000) % 60;
        t_run->sec = (elapsed / 1000) % 60;
        t_run->ms = elapsed % 1000;
        t_run->padding = 0;
    #endif /*PC_WIN*/
    #ifdef PC_LINUX
        struct timeval tv;
        gettimeofday(&tv, NULL);
        uint64_t  now = (uint64_t )tv.tv_sec * 1000 + tv.tv_usec / 1000;
        uint64_t  elapsed = now - start;
        t_run->time_stemp = 0;
        t_run->day  = elapsed / 8640000;
        t_run->hour = (elapsed / 3600000) % 24;
        t_run->min = (elapsed / 60000) % 60;
        t_run->sec = (elapsed / 1000) % 60;
        t_run->ms = elapsed % 1000;
        t_run->padding = 0;
    #endif /*PC_LINUX*/

}

/**
 * 日志获取本地时间
 */
static inline void r_log_time_local_get(r_log_time_local_t *t_local)
{
    #ifdef PC_WIN
        time_t now = time(NULL);
        struct tm *local = localtime(&now);  // 本地时间
        t_local->time_stemp = now;
        t_local->year = local->tm_year + 1900,
        t_local->month= local->tm_mon + 1,
        t_local->day  = local->tm_mday,
        t_local->hour = local->tm_hour,
        t_local->min  = local->tm_min,
        t_local->sec  = local->tm_sec;
        t_local->padding = 0;
    #endif /*PC_WIN*/
    #ifdef PC_LINUX
        time_t now = time(NULL);
        struct tm result;
        struct tm *local = localtime_r(&now,&result);  // 本地时间
        t_local->time_stemp = now;
        t_local->year = local->tm_year + 1900,
        t_local->month= local->tm_mon + 1,
        t_local->day  = local->tm_mday,
        t_local->hour = local->tm_hour,
        t_local->min  = local->tm_min,
        t_local->sec  = local->tm_sec;
        t_local->padding = 0;
    #endif /*PC_LINUX*/
}

// 左对齐：数字靠左，右侧填充
static inline uint16_t r_log_tool_10_to_16_left(uint8_t  *buf,uint32_t number,uint16_t width,uint8_t pad_char)
{
    uint16_t len = 0;
    uint32_t n = number;
    uint16_t i = 0;
    uint8_t t = 0;

    do {
        buf[len++] = r_log_hex[n & 0x0F];
        n >>= 4;
    } while (n > 0);

    for (i = 0; i < len / 2; i++)
    {
        t = buf[i];
        buf[i] = buf[len - 1 - i];
        buf[len - 1 - i] = t;
    }

    for (i = len; i < width; i++) 
    {
        buf[i] = pad_char;
    }

    return (width > len) ? width : len;
}

// 右对齐：数字靠右，左侧填充
static inline uint16_t r_log_tool_10_to_16_right(
    uint8_t  *buf,
    uint32_t  number,
    uint16_t  width,
    uint8_t   pad_char
)
{
    uint16_t len = 0;
    uint32_t n = number;
    uint16_t i = 0;
    uint16_t pad = 0;
    uint8_t  tmp[16];
    
    do {
        tmp[len++] = r_log_hex[n & 0x0F];
        n >>= 4;
    } while (n > 0);

    for (i = 0; i < len / 2; i++) 
    {
        uint8_t t = tmp[i];
        tmp[i] = tmp[len - 1 - i];
        tmp[len - 1 - i] = t;
    }

    pad = (width > len) ? (width - len) : 0;

    for (i = 0; i < pad; i++) 
    {
        buf[i] = pad_char;
    }

    for (i = 0; i < len; i++) 
    {
        buf[pad + i] = tmp[i];
    }

    return pad + len;
}

// 工具，10进制转字符，不对齐，顺序写
static inline uint16_t r_log_tool_10_to_str(char *buf, uint32_t number)
{
    uint32_t temp = number;
    uint16_t digits = 0;
    uint16_t pos = 0;
    uint32_t div = 0;
    uint16_t i = 0;

    // 计算位数
    do {
        digits++;
        temp /= 10;
    } while (temp > 0);

    // 计算最高位的除数
    div = 1;
    for (i = 1; i < digits; i++) {
        div *= 10;
    }

    // 从高位到低位直接写入
    temp = number;
    do {
        buf[pos++] = '0' + (temp / div);
        temp %= div;
        div /= 10;
    } while (div > 0);

    return pos;
}

// 工具，10进制转字符，左对齐，剩余补任意字符
static inline uint16_t r_log_tool_10_to_str_left(char *buf, uint32_t number, uint16_t width, char pad_char)
{
    uint32_t temp = number;
    uint16_t digits = 0;
    uint16_t len = 0;
    uint16_t pos = 0;
    uint32_t div = 0;
    uint16_t i = 0;

    // 计算位数
    do {
        digits++;
        temp /= 10;
    } while (temp > 0);

    len = (width > digits) ? width : digits;
    pos = 0;

    // 计算最高位的除数
    div = 1;
    for (i = 1; i < digits; i++) 
    {
        div *= 10;
    }

    // 从高位到低位直接写入（天然正序）
    temp = number;
    do {
        buf[pos++] = '0' + (temp / div);
        temp %= div;
        div /= 10;
    } while (div > 0);

    // 右侧填充
    while (pos < len) {
        buf[pos++] = pad_char;
    }

    return len;
}

// 右对齐：数字靠右，左侧填充
static inline uint16_t r_log_tool_10_to_str_right(char *buf, uint32_t number, uint16_t width, char pad_char)
{
    uint32_t temp = number;
    uint16_t digits = 0;
    uint16_t len = 0;
    uint16_t pos = 0;
    uint16_t pad = 0;
    uint16_t i = 0;
    uint32_t div = 0;

    // 计算位数
    do {
        digits++;
        temp /= 10;
    } while (temp > 0);

    len = (width > digits) ? width : digits;
    pos = 0;
    pad = len - digits;

    // 左侧填充
    for (i = 0; i < pad; i++) {
        buf[pos++] = pad_char;
    }

    // 计算最高位的除数
    div = 1;
    for (i = 1; i < digits; i++) {
        div *= 10;
    }

    // 从高位到低位直接写入
    temp = number;
    do {
        buf[pos++] = '0' + (temp / div);
        temp %= div;
        div /= 10;
    } while (div > 0);

    return len;
}

#if (R_LOG_ENABLE == 1)
// 初始化
void r_log_init(void)
{
    #ifdef PC_WIN
    start = GetTickCount64();
    #endif /*PC_WIN*/
    #ifdef PC_LINUX
    struct timeval tv;
    gettimeofday(&tv, NULL);
    start = (uint64_t )tv.tv_sec * 1000 + tv.tv_usec / 1000;
    #endif /*PC_LINUX*/

    #if (R_LOG_LEVEL == 0)
        r_log_state.lv_debug = RLV_OFF;
        r_log_state.lv_trace = RLV_OFF;
        r_log_state.lv_info  = RLV_OFF;
        r_log_state.lv_warn  = RLV_OFF;
        r_log_state.lv_error = RLV_ON;
        r_log_state.lv_fatal = RLV_ON;

    #elif (R_LOG_LEVEL == 1)
        r_log_state.lv_debug = RLV_OFF;
        r_log_state.lv_trace = RLV_OFF;
        r_log_state.lv_info  = RLV_ON;
        r_log_state.lv_warn  = RLV_ON;
        r_log_state.lv_error = RLV_ON;
        r_log_state.lv_fatal = RLV_ON;
    #elif (R_LOG_LEVEL == 2)
        r_log_state.lv_debug = RLV_ON;
        r_log_state.lv_trace = RLV_OFF;
        r_log_state.lv_info  = RLV_ON;
        r_log_state.lv_warn  = RLV_ON;
        r_log_state.lv_error = RLV_ON;
        r_log_state.lv_fatal = RLV_ON;
    #elif (R_LOG_LEVEL == 3)
        r_log_state.lv_debug = RLV_ON;
        r_log_state.lv_trace = RLV_ON;
        r_log_state.lv_info  = RLV_ON;
        r_log_state.lv_warn  = RLV_ON;
        r_log_state.lv_error = RLV_ON;
        r_log_state.lv_fatal = RLV_ON;
    #endif /*R_LOG_LEVEL*/

    #if (R_LOG_TIME_RUN == 1)
        r_log_state.t_run    = 1;
    #elif (R_LOG_TIME_RUN == 2)
        r_log_state.t_run    = 2;
    #elif (R_LOG_TIME_RUN == 3)
        r_log_state.t_run    = 3;
    #elif (R_LOG_TIME_RUN == 4)
        r_log_state.t_run    = 4;
    #endif /*R_LOG_TIME_RUN*/

    #if (R_LOG_TIME_LOCAL == 1)
        r_log_state.t_local  = 1;
    #elif (R_LOG_TIME_LOCAL == 2)
        r_log_state.t_local  = 2;
    #elif (R_LOG_TIME_LOCAL == 3)
        r_log_state.t_local  = 3;
    #endif /*R_LOG_TIME_RUN*/

    // 默认换行
    r_log_state.new_line = RLV_ON;

    // 清空缓冲区
    #if (R_LOG_MEM_MODE == 0)
    memset(r_log_tx_buf,0,R_LOG_BUF_SIZE);
    #endif /*R_LOG_MEM_MODE*/
}
#else
void r_log_init(void)
{ ((void)0); }
#endif /*R_LOG_ENABLE*/

#if (R_LOG_ENABLE == 1)
// 日志等级管理
void r_log_set_out_level(r_log_level_t level ,uint8_t enable)
{
    switch(level)
    {
        case rlv_trace: r_log_state.lv_trace = enable; break;
        case rlv_debug: r_log_state.lv_debug = enable; break;
        case rlv_info:  r_log_state.lv_info  = enable; break;
        case rlv_warn:  r_log_state.lv_warn  = enable; break;
        case rlv_error: r_log_state.lv_error = enable; break;
        case rlv_fatal: r_log_state.lv_fatal = enable; break;
    }
}
#else 
void r_log_set_out_level(r_log_level_t level ,uint8_t enable)
{((void)level); ((void)enable);}
#endif /*R_LOG_ENABLE*/

#if (R_LOG_ENABLE == 1)
// 日志时间管理
void r_log_set_time_level(r_log_time_level_t t_level,uint8_t s_level)
{
    switch(t_level)
    {
        case rlv_time_run:   r_log_state.t_run   = s_level; break;
        case rlv_time_local: r_log_state.t_local = s_level; break;
    }
}

#else
void r_log_set_time_level(r_log_time_level_t t_level,uint8_t s_level)
{((void)t_level); ((void)s_level);}
#endif /*R_LOG_ENABLE*/

#if (R_LOG_ENABLE == 1)
// 日志换行设置
void r_log_set_new_line(uint8_t enable)
{
    r_log_state.new_line = enable;
}
#else
void r_log_set_new_line(uint8_t enable)
{((void)enable);}
#endif /*R_LOG_ENABLE*/

#if (R_LOG_ENABLE == 1)
// 日志通用输出函数
int32_t r_log_out(r_log_level_t level,const char *f_name,uint32_t line,const char *fmt,...)
{
    r_log_time_run_t t_run = {0};
    r_log_time_local_t t_local = {0};
    uint16_t tx_count = 0;
    uint16_t tmp = 0;
    uint8_t *buf_ops = 0;
    int32_t res = 0;
    va_list args;

    // 动态内存分配
    #if (R_LOG_MEM_MODE == 2)
    uint8_t *r_log_tx_buf = r_log_alloc(R_LOG_BUF_SIZE);
    if (!r_log_tx_buf) {return R_LOG_ERROR_MEMORY; } // 内存不足
    buf_ops = r_log_tx_buf;
    #endif /*R_LOG_MEM_MODE*/

    #if (R_LOG_MEM_MODE == 1)
    uint8_t r_log_tx_buf[R_LOG_BUF_SIZE] = {0};
    #endif /*R_LOG_MEM_MODE*/
    
    // 获取时间
    r_log_time_local_get(&t_local);
    r_log_time_run_get(&t_run);

    // 静态内存
    #if (R_LOG_MEM_MODE >= 0) || (R_LOG_MEM_MODE <= 1)
    buf_ops = r_log_tx_buf;
    #endif /*R_LOG_MEM_MODE*/
        
    // 添加运行时间
    #if ((R_LOG_TIME_RUN >= 1) && (R_LOG_TIME_RUN < 5))

    // 颜色追加
    #if (R_LOG_COLOR >= 1) 
    if((r_log_state.t_run >= 1) && (r_log_state.t_run <= 4))
    {
        memcpy(buf_ops,r_log_arr_color[RL_TIME_RUN],r_log_color_len[RL_TIME_RUN]);
        buf_ops += r_log_color_len[RL_TIME_RUN];
    }
    #endif /*R_LOG_COLOR*/
    if(r_log_state.t_run == 1)// [00.000]
    {
        *buf_ops++ = '[';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_run.sec,2,'0');
        buf_ops += tmp;
        *buf_ops++ = '.';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_run.ms,3,'0');
        buf_ops += tmp;
        *buf_ops++ = ']';
    }
    #endif /*R_LOG_TIME_RUN*/
    #if ((R_LOG_TIME_RUN >= 2) && (R_LOG_TIME_RUN < 5))
    else if(r_log_state.t_run == 2) // [00000000]
    {
        *buf_ops++ = '[';
        tmp = r_log_tool_10_to_16_right(buf_ops,t_run.time_stemp,8,'0');
        buf_ops += tmp;
        *buf_ops++ = ']';
    }
    #endif /*R_LOG_TIME_RUN*/
    #if ((R_LOG_TIME_RUN >= 3) && (R_LOG_TIME_RUN < 5))
    else if(r_log_state.t_run == 3) // [00:00:00.000] 
    {
        *buf_ops++ = '[';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_run.hour,2,'0'); buf_ops += tmp;
        *buf_ops++ = ':';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_run.min,2,'0');buf_ops += tmp;
        *buf_ops++ = ':';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_run.sec,2,'0');buf_ops += tmp;
        *buf_ops++ = '.';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_run.ms,3,'0');buf_ops += tmp;
        *buf_ops++ = ']';
    }
    #endif /*R_LOG_TIME_RUN*/
    #if ((R_LOG_TIME_RUN >= 4) && (R_LOG_TIME_RUN < 5))
    else if(r_log_state.t_run == 4) // [0000|00:00:00.000]
    {
        *buf_ops++ = '[';
        tmp = r_log_tool_10_to_16_right(buf_ops,t_run.day,4,'0'); buf_ops += tmp;
        *buf_ops++ = '|';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_run.hour,2,'0'); buf_ops += tmp;
        *buf_ops++ = ':';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_run.min,2,'0');buf_ops += tmp;
        *buf_ops++ = ':';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_run.sec,2,'0');buf_ops += tmp;
        *buf_ops++ = '.';
        tmp = r_log_tool_10_to_str_left(buf_ops,t_run.ms,3,'0');buf_ops += tmp;
        *buf_ops++ = ']';
    }
    #endif /*R_LOG_TIME_RUN*/

    #if ((R_LOG_TIME_RUN >= 1) && (R_LOG_TIME_RUN < 5))
    if(r_log_state.t_run > 0){ *buf_ops++ = ' '; }
    #endif /*R_LOG_TIME_RUN*/

    // 添加本地时间
    #if ((R_LOG_TIME_LOCAL >= 1) && (R_LOG_TIME_LOCAL < 4))
    #if (R_LOG_COLOR == 1) 
    if((r_log_state.t_local >= 1) && (r_log_state.t_local <= 3))
    {
        memcpy(buf_ops,r_log_arr_color[RL_TIME_LOCAL],r_log_color_len[RL_TIME_LOCAL]);
        buf_ops += r_log_color_len[RL_TIME_LOCAL];
    }
    #endif /*R_LOG_COLOR*/
    if(r_log_state.t_local == 1) // [00000000]
    {
        *buf_ops++ = '[';
        tmp = r_log_tool_10_to_16_right(buf_ops,t_local.time_stemp,8,'0');
        buf_ops += tmp;
        *buf_ops++ = ']';
    }
    #endif /*R_LOG_TIME_LOCAL*/
    #if ((R_LOG_TIME_LOCAL >= 2) && (R_LOG_TIME_LOCAL < 4))
    else if(r_log_state.t_local == 2) // [00:00:00]
    {
        *buf_ops++ = '[';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_local.hour,2,'0'); buf_ops += tmp;
        *buf_ops++ = ':';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_local.min,2,'0'); buf_ops += tmp;
        *buf_ops++ = ':';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_local.sec,2,'0'); buf_ops += tmp;
        *buf_ops++ = ']';
    }
    #endif /*R_LOG_TIME_LOCAL*/
    #if ((R_LOG_TIME_LOCAL >= 3) && (R_LOG_TIME_LOCAL < 4))
    else if(r_log_state.t_local == 3) // [0000-00-00 00:00:00]
    {
        *buf_ops++ = '[';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_local.year,4,'0'); buf_ops += tmp;
        *buf_ops++ = '-';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_local.month,2,'0'); buf_ops += tmp;
        *buf_ops++ = '-';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_local.day,2,'0'); buf_ops += tmp;
        *buf_ops++ = ' ';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_local.hour,2,'0'); buf_ops += tmp;
        *buf_ops++ = ':';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_local.min,2,'0'); buf_ops += tmp;
        *buf_ops++ = ':';
        tmp = r_log_tool_10_to_str_right(buf_ops,t_local.sec,2,'0'); buf_ops += tmp;
        *buf_ops++ = ']';
    }
    #endif /*R_LOG_TIME_LOCAL*/

    #if (R_LOG_TIME_LOCAL >= 1)
    if(r_log_state.t_local > 0){ *buf_ops++ = ' '; }
    #endif /*R_LOG_TIME_LOCAL*/

    // 添加标签
    switch(level)
    {
        #if (R_LOG_LEVEL >= 3)
        case rlv_trace: 
            #if (R_LOG_COLOR == 1)
            memcpy(buf_ops,r_log_arr_color[RL_LEVEL_TRACE],r_log_color_len[RL_LEVEL_TRACE]);
            buf_ops += r_log_color_len[RL_LEVEL_TRACE];
            #endif /*R_LOG_COLOR*/
            memcpy(buf_ops,r_log_arr_char[rlv_trace],r_log_arr_len[rlv_trace]); 
            buf_ops += r_log_arr_len[rlv_trace];
        break;
        #endif /*R_LOG_LEVEL*/
        #if (R_LOG_LEVEL >= 2)
        case rlv_debug: 
            #if (R_LOG_COLOR == 1)
            memcpy(buf_ops,r_log_arr_color[RL_LEVEL_DEBUG],r_log_color_len[RL_LEVEL_DEBUG]);
            buf_ops += r_log_color_len[RL_LEVEL_DEBUG];
            #endif /*R_LOG_COLOR*/
            memcpy(buf_ops,r_log_arr_char[rlv_debug],r_log_arr_len[rlv_debug]); 
            buf_ops += r_log_arr_len[rlv_debug];
        break;
        #endif /*R_LOG_LEVEL*/
        #if (R_LOG_LEVEL >= 1)
        case rlv_info: 
            #if (R_LOG_COLOR == 1)
            memcpy(buf_ops,r_log_arr_color[RL_LEVEL_INFO],r_log_color_len[RL_LEVEL_INFO]);
            buf_ops += r_log_color_len[RL_LEVEL_INFO];
            #endif /*R_LOG_COLOR*/
            memcpy(buf_ops,r_log_arr_char[rlv_info],r_log_arr_len[rlv_info]); 
            buf_ops += r_log_arr_len[rlv_info];
        break;
        case rlv_warn: 
            #if (R_LOG_COLOR == 1)
            memcpy(buf_ops,r_log_arr_color[RL_LEVEL_WARN],r_log_color_len[RL_LEVEL_WARN]);
            buf_ops += r_log_color_len[RL_LEVEL_WARN];
            #endif /*R_LOG_COLOR*/
            memcpy(buf_ops,r_log_arr_char[rlv_warn],r_log_arr_len[rlv_warn]); 
            buf_ops += r_log_arr_len[rlv_warn];
        break;
        #endif /*R_LOG_LEVEL*/
        #if (R_LOG_LEVEL >= 0)
        case rlv_error: 
            #if (R_LOG_COLOR == 1)
            memcpy(buf_ops,r_log_arr_color[RL_LEVEL_ERROR],r_log_color_len[RL_LEVEL_ERROR]);
            buf_ops += r_log_color_len[RL_LEVEL_ERROR];
            #endif /*R_LOG_COLOR*/
            memcpy(buf_ops,r_log_arr_char[rlv_error],r_log_arr_len[rlv_error]); 
            buf_ops += r_log_arr_len[rlv_error];
        break;
        case rlv_fatal: 
            #if (R_LOG_COLOR == 1)
            memcpy(buf_ops,r_log_arr_color[RL_LEVEL_FATAL],r_log_color_len[RL_LEVEL_FATAL]);
            buf_ops += r_log_color_len[RL_LEVEL_FATAL];
            #endif /*R_LOG_COLOR*/
            memcpy(buf_ops,r_log_arr_char[rlv_fatal],r_log_arr_len[rlv_fatal]); 
            buf_ops += r_log_arr_len[rlv_fatal];
        break;
        #endif /*R_LOG_LEVEL*/
    }

    // 添加调试信息
    #if (R_LOG_LEVEL >= 2)
    if((r_log_state.lv_debug == RLV_ON) && (level == rlv_debug))
    {
        #if (R_LOG_COLOR == 1)
        memcpy(buf_ops,r_log_arr_color[RL_FILE_NAME],r_log_color_len[RL_FILE_NAME]);
        buf_ops += r_log_color_len[RL_FILE_NAME];
        #endif /*R_LOG_COLOR*/

        *buf_ops++ = ' ';
        tmp = strlen(f_name);
        memcpy(buf_ops,f_name,tmp);
        buf_ops += tmp;

        *buf_ops++ = ':';

        #if (R_LOG_COLOR == 1)
        memcpy(buf_ops,r_log_arr_color[RL_LINE],r_log_color_len[RL_LINE]);
        buf_ops += r_log_color_len[RL_LINE];
        #endif /*R_LOG_COLOR*/

        tmp = r_log_tool_10_to_str(buf_ops,line);
        buf_ops += tmp;

        #if (R_LOG_COLOR == 1)
        memcpy(buf_ops,r_log_arr_color[RL_LEVEL_DEBUG],r_log_color_len[RL_LEVEL_DEBUG]);
        buf_ops += r_log_color_len[RL_LEVEL_DEBUG];
        #endif /*R_LOG_COLOR*/

        *buf_ops++ = ' ';
        *buf_ops++ = '|';
    }
    #endif /*R_LOG_LEVEL*/

    // 添加信息
    *buf_ops++ = ' ';
    va_start(args, fmt);
    res = vsnprintf(buf_ops, (R_LOG_BUF_SIZE - (buf_ops - r_log_tx_buf)), fmt, args);
    if ((size_t)res >= (R_LOG_BUF_SIZE - (buf_ops - r_log_tx_buf))) { return R_LOG_ERROR_BUF_SIZE; }
    if (res < 0){ return R_LOG_ERROR_CODE; }
    va_end(args);
    buf_ops += res;
    
    // 去除配色
    #if (R_LOG_COLOR == 1)
    memcpy(buf_ops,r_log_arr_color[RL_NONE],r_log_color_len[RL_NONE]);
    buf_ops += r_log_color_len[RL_NONE];
    #endif /*R_LOG_COLOR*/

    // 换行
    if(r_log_state.new_line == RLV_ON)
    {
        *buf_ops++ = '\n';
    }

    // 发送组装好的字符串
    tx_count = buf_ops - r_log_tx_buf;
    r_log_send_data(r_log_tx_buf,tx_count);

    // 动态内存分配
    #if (R_LOG_MEM_MODE == 2)
    r_log_free(r_log_tx_buf);
    #endif /*R_LOG_MEM_MODE*/

    return R_LOG_OK;  // 正常
}
#else
int32_t r_log_out(r_log_level_t level,const char *f_name,uint32_t line,const char *fmt,...) 
{ ((void)level);((void)f_name);((void)line);((void)fmt); return 0;}
#endif /*R_LOG_ENABLE*/