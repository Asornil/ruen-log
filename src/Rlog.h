#ifndef __RUNE_LOG_H__
#define __RUNE_LOG_H__

#include <stdint.h>
#include <stddef.h>

/**
 * Rune Log
 * 简单快速的日志库
 * 支持多线程
 * 支持颜色输出
 * 包含运行时间
 * 包含本地时间
 * 可自由裁剪
 * 作者: Asornil
 * 时间: 2026/8/31
 * 版本: v1.2
 */

/**
 * 日志开关
 * 0: 关闭
 * 1: 启用
 */
#define R_LOG_ENABLE 1

/**
 * 日志缓冲区大小
 */
#define R_LOG_BUF_SIZE 512

/**
 * TAG模式
 * 0: 不启用
 * 1: 启用
 */
#define R_LOG_TAG_MODE      1

/**
 * TAG模式 - 模块表容量
 */
#define R_LOG_TAG_MAX       16

/**
 * 内存模式
 * 0: 静态内存模式（不支持多线程）（节省内存）
 * 1: 栈内存模式（支持多线程）（栈要大）
 * 2: 动态内存模式（支持多线程）（内存分配）（需要提供内存分配函数）
 * 3: 静态内存池（支持多线程）（内部分配）
 */
#define R_LOG_MEM_MODE      2

/**
 * 输出模式
 * 0: 同步模式
 * 1: 异步模式(需要使用 r_log_flush)
 */
#define R_LOG_OUT_MODE      1

/**
 * 输出模式 - 异步模式 - 队列槽位数
 */
#define R_LOG_QUEUE_SIZE    16

/**
 * 精简日志(优先级最高)
 * 0: 关闭
 * 1: 精简 [T] [I] [W] [E] [D] [F]
 * 2: 略简 [TRC] [INF] [WRN] [ERR] [DBG] [FAL]
 */
#define R_LOG_TIDY  0

/**
 * 模式选择
 * 0: Error, Fatal
 * 1: Info,  Warn, Error, Fatal
 * 2: Debug, Info, Warn,  Error, Fatal
 * 3: Debug, Info, Warn,  Error, Fatal, Trace
 */
#define R_LOG_LEVEL 3

/**
 * 时间模块 设备运行时间
 * 0: 关闭
 * 1: 设备运行时间 (微缩) [ 00.000 ]  // 秒，毫秒
 * 2: 设备运行时间 (时间戳)[ 00000000 ] // unix 时间戳(16H)
 * 3: 设备运行时间 (缩略) [ 00:00:00:000 ] // 小时.分钟.秒.毫秒
 * 4: 设备运行时间 (完全) [ 0000|00:00:00.000 ] // 天数，小时.分钟.秒.毫秒
 */
#define R_LOG_TIME_RUN  4

/**
 * 时间模块 本地时间
 * 0: 关闭
 * 1: 本地时间 (时间戳)[ 00000000 ]   // unix 时间戳(16H)
 * 2: 本地时间 (缩略)  [ 00:00:00 ]  // 小时.分钟.秒
 * 3: 本地时间 (完全)  [ 0000-00-00 00:00:00 ] // 年，月，日，时，分，秒
 */
#define R_LOG_TIME_LOCAL 3

/**
 * 色彩日志
 * 0: 无色彩 
 * 1: 有色彩
 */
#define R_LOG_COLOR     1

/*
 * 配色
 * 0: One Dark Hybrid
 * 1: Gruvbox Dark
 * 2: Tokyo Night Storm
 * 3: Nord Polar
 */
#define R_LOG_COLOR_SCHEME  0

// 日志等级
typedef enum
{
    rlv_trace = 0,
    rlv_debug,
    rlv_info,
    rlv_warn,
    rlv_error,
    rlv_fatal,
    RLV_MAX
}r_log_level_t;

// 日志时间
typedef enum 
{
    rlv_time_run = 0,
    rlv_time_local,
    RLV_TIME_MAX
}r_log_time_level_t;

/**
 * 启用/关闭 功能宏 
 */
#define RLV_ON  ((uint8_t)0x01)
#define RLV_OFF ((uint8_t)0x00)

/**
 * TAG 等级位掩码（r_log_tag_set 参数使用，可按位 | 组合）
 * 例: TAG_INFO | TAG_ERROR  仅输出 info 与 error 两级
 * TAG_NONE: 模块静音（无任何等级输出）
 */
#define TAG_TRACE   ((uint8_t)(1u << rlv_trace))
#define TAG_DEBUG   ((uint8_t)(1u << rlv_debug))
#define TAG_INFO    ((uint8_t)(1u << rlv_info))
#define TAG_WARN    ((uint8_t)(1u << rlv_warn))
#define TAG_ERROR   ((uint8_t)(1u << rlv_error))
#define TAG_FATAL   ((uint8_t)(1u << rlv_fatal))
#define TAG_NONE    ((uint8_t)0x00)   // 静音（不输出任何等级）
#define TAG_ALL     ((uint8_t)0x3Fu)   // 全部等级

#if (R_LOG_ENABLE == 1)

/**
 * TAG模式 登记/设置 模块输出等级位掩码（未登记则追加登记）
 * mask: TAG_TRACE|TAG_DEBUG|... 按位组合；TAG_NONE 静音该模块
 */
void r_log_tag_set(const char *tag, uint8_t mask);

/**
 * 运行时 启用/关闭 日志等级
 */
void r_log_set_out_level(r_log_level_t level,uint8_t enable);

/**
 * 运行时 启用/关闭 时间等级
 */

void r_log_set_time_level(r_log_time_level_t t_level,uint8_t s_level);

/**
 * 运行时 启用/关闭 换行
 */
void r_log_set_new_line(uint8_t enable);

/**
 * 初始化 日志
 */
void r_log_init(void);

/**
 * 轮询发送（异步模式）：仅发送一条缓存日志
 * 返回: 1=已发送, 0=队列空(调用方可让出/休眠)
 */
int32_t r_log_poll(void);

/**
 * 排空发送（异步模式）：将缓存日志全部发送
 * 返回: 发送条数
 */
int32_t r_log_flush(void);

/**
 * 日志输出接口
 */
int32_t r_log_out(r_log_level_t level,const char *tag, const char *f_name,uint32_t line,const char *fmt,...);

/**
 * C99 文件名支持
 */
#ifndef __FILE_NAME__

static inline const char* get_filename(const char* path) 
{
    const char* sep = strrchr(path, '/');
    if (!sep) 
    {
        sep = strrchr(path, '\\');
    }
    return sep ? sep + 1 : path;
}
#define __FILE_NAME__ (get_filename(__FILE__))

#endif /*__FILE_NAME__*/

/**
 * 日志主要输出宏
 */
#if (R_LOG_LEVEL == 3)
    #define RLOG_TRACE(tag, fmt, ...) r_log_out(rlv_trace,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_INFO(tag, fmt, ...)  r_log_out(rlv_info,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_WARN(tag, fmt, ...)  r_log_out(rlv_warn,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_ERROR(tag, fmt, ...) r_log_out(rlv_error,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_DEBUG(tag, fmt, ...) r_log_out(rlv_debug,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_FATAL(tag, fmt, ...) r_log_out(rlv_fatal,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
#endif /*R_LOG_LEVEL*/
#if (R_LOG_LEVEL == 2)
    #define RLOG_TRACE(tag, fmt, ...) ((void)0)
    #define RLOG_INFO(tag, fmt, ...)  r_log_out(rlv_info,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_WARN(tag, fmt, ...)  r_log_out(rlv_warn,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_ERROR(tag, fmt, ...) r_log_out(rlv_error,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_DEBUG(tag, fmt, ...) r_log_out(rlv_debug,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_FATAL(tag, fmt, ...) r_log_out(rlv_fatal,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
#endif /*R_LOG_LEVEL*/
#if (R_LOG_LEVEL == 1)
    #define RLOG_TRACE(tag, fmt, ...) ((void)0)
    #define RLOG_INFO(tag, fmt, ...)  r_log_out(rlv_info,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_WARN(tag, fmt, ...)  r_log_out(rlv_warn,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_ERROR(tag, fmt, ...) r_log_out(rlv_error,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_DEBUG(tag, fmt, ...) ((void)0)
    #define RLOG_FATAL(tag, fmt, ...) r_log_out(rlv_fatal,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
#endif /*R_LOG_LEVEL*/
#if (R_LOG_LEVEL == 0)
    #define RLOG_TRACE(tag, fmt, ...) ((void)0)
    #define RLOG_INFO(tag, fmt, ...)  ((void)0)
    #define RLOG_WARN(tag, fmt, ...)  ((void)0)
    #define RLOG_ERROR(tag, fmt, ...) r_log_out(rlv_error,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_DEBUG(tag, fmt, ...) ((void)0)
    #define RLOG_FATAL(tag, fmt, ...) r_log_out(rlv_fatal,tag,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
#endif /*R_LOG_LEVEL*/
#elif (R_LOG_ENABLE == 0)
    void r_log_tag_set(const char *tag, uint8_t mask);
    void r_log_set_out_level(r_log_level_t level,uint8_t enable);
    void r_log_set_time_level(r_log_time_level_t t_level,uint8_t s_level);
    void r_log_set_new_line(uint8_t enable);
    void r_log_init(void);
    int32_t r_log_poll(void);
    int32_t r_log_flush(void);
    int32_t r_log_out(r_log_level_t level,const char *tag, const char *f_name,uint32_t line,const char *fmt,...);;
    #define RLOG_TRACE(tag, fmt,...) ((void)0)
    #define RLOG_INFO(tag, fmt,...)  ((void)0)
    #define RLOG_WARN(tag, fmt,...)  ((void)0)
    #define RLOG_ERROR(tag, fmt,...) ((void)0)
    #define RLOG_DEBUG(tag, fmt,...) ((void)0)
    #define RLOG_FATAL(tag, fmt,...) ((void)0)
#endif /*R_LOG_ENABLE*/

#define R_LOG_OK                ((int32_t) 0)   // 正常
#define R_LOG_ERROR_BUF_SIZE    ((int32_t)-1)   // 缓冲区不足
#define R_LOG_ERROR_CODE        ((int32_t)-2)   // 编码错误
#define R_LOG_ERROR_MEMORY      ((int32_t)-3)   // 内存不足
#define R_LOG_ERROR_SOLT        ((int32_t)-4)   // 静态内存池槽位耗尽
#define R_LOG_ERROR_QUNE        ((int32_t)-5)   // 缓存队列耗尽
#define R_LOG_ERROR_TAG         ((int32_t)-6)   // 模块(TAG)被过滤丢弃

#endif /*__RUNE_LOG_H__*/