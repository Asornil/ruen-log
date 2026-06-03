#ifndef __R_LOG_H__
#define __R_LOG_H__

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
 * 时间: 2026/6/3
 * 版本: v1.1
 */

/**
 * 日志缓冲区大小
 */
#define R_LOG_BUF_SIZE 512

/**
 * 日志开关
 * 0: 关闭
 * 1: 启用
 */
#define R_LOG_ENABLE 1

/**
 * 内存模式
 * 0: 静态内存模式（不支持多线程）（节省内存）
 * 1: 栈内存模式（支持多线程）（栈要大）
 * 2: 动态内存模式（支持多线程）（内存分配）（需要提供内存分配函数）
 */
#define R_LOG_MEM_MODE  0


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

#if (R_LOG_ENABLE == 1)

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
 * 日志输出接口
 */
int32_t r_log_out(r_log_level_t level,const char *f_name,uint32_t line,const char *fmt,...);

/**
 * 日志主要输出宏
 */
#if (R_LOG_LEVEL == 3)
    #define RLOG_TRACE(fmt,...) r_log_out(rlv_trace,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_INFO(fmt,...)  r_log_out(rlv_info,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_WARN(fmt,...)  r_log_out(rlv_warn,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_ERROR(fmt,...) r_log_out(rlv_error,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_DEBUG(fmt,...) r_log_out(rlv_debug,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_FATAL(fmt,...) r_log_out(rlv_fatal,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
#endif /*R_LOG_LEVEL*/
#if (R_LOG_LEVEL == 2)
    #define RLOG_TRACE(fmt,...) ((void)0)
    #define RLOG_INFO(fmt,...)  r_log_out(rlv_info,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_WARN(fmt,...)  r_log_out(rlv_warn,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_ERROR(fmt,...) r_log_out(rlv_error,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_DEBUG(fmt,...) r_log_out(rlv_debug,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_FATAL(fmt,...) r_log_out(rlv_fatal,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
#endif /*R_LOG_LEVEL*/
#if (R_LOG_LEVEL == 1)
    #define RLOG_TRACE(fmt,...) ((void)0)
    #define RLOG_INFO(fmt,...)  r_log_out(rlv_info,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_WARN(fmt,...)  r_log_out(rlv_warn,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_ERROR(fmt,...) r_log_out(rlv_error,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_DEBUG(fmt,...) ((void)0)
    #define RLOG_FATAL(fmt,...) r_log_out(rlv_fatal,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
#endif /*R_LOG_LEVEL*/
#if (R_LOG_LEVEL == 0)
    #define RLOG_TRACE(fmt,...) ((void)0)
    #define RLOG_INFO(fmt,...)  ((void)0)
    #define RLOG_WARN(fmt,...)  ((void)0)
    #define RLOG_ERROR(fmt,...) r_log_out(rlv_error,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
    #define RLOG_DEBUG(fmt,...) ((void)0)
    #define RLOG_FATAL(fmt,...) r_log_out(rlv_fatal,__FILE_NAME__,__LINE__,fmt,##__VA_ARGS__)
#endif /*R_LOG_LEVEL*/
#elif (R_LOG_ENABLE == 0)
    void r_log_set_out_level(r_log_level_t level,uint8_t enable);
    void r_log_set_time_level(r_log_time_level_t t_level,uint8_t s_level);
    void r_log_set_new_line(uint8_t enable);
    void r_log_init(void);
    int32_t r_log_out(r_log_level_t level,const char *f_name,uint32_t line,const char *fmt,...);
    #define RLOG_TRACE(fmt,...) ((void)0)
    #define RLOG_INFO(fmt,...)  ((void)0)
    #define RLOG_WARN(fmt,...)  ((void)0)
    #define RLOG_ERROR(fmt,...) ((void)0)
    #define RLOG_DEBUG(fmt,...) ((void)0)
    #define RLOG_FATAL(fmt,...) ((void)0)
#endif /*R_LOG_ENABLE*/

#define R_LOG_OK                ((int32_t) 0)   // 正常
#define R_LOG_ERROR_BUF_SIZE    ((int32_t)-1)   // 缓冲区不足
#define R_LOG_ERROR_CODE        ((int32_t)-2)   // 编码错误
#define R_LOG_ERROR_MEMORY      ((int32_t)-3)   // 内存不足

#endif /*__R_LOG_H__*/