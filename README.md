<h1 align="center">Rune Log · 符文日志</h1>

<p align="center">
  <strong>轻量级、高可配、多色彩方案 — 嵌入式友好的 C 语言日志库</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/语言-C99-blue.svg"/>
  <img src="https://img.shields.io/badge/许可证-MIT-green.svg"/>
  <img src="https://img.shields.io/badge/版本-v1.1-orange.svg"/>
  <img src="https://img.shields.io/badge/平台-Windows%20|%20Linux%20|%20嵌入式-lightgrey.svg"/>
  <img src="https://img.shields.io/badge/内存-静态%20|%20栈%20|%20动态-brightgreen.svg"/>
</p>

---

> ⚡ **平台说明：** Rune Log **仅内置了 Windows 平台的实现**（使用 Win32 API），开箱即用。
> Linux 和嵌入式平台需要手动适配底层接口，适配指南请参考「用户需提供的接口」章节。

---

## 📖 简介

**Rune Log** 是一个用纯 C99 编写的轻量级日志库，专为**嵌入式系统**和**桌面应用**设计。它提供丰富的日志等级、灵活的时间格式、多套色彩方案以及三种内存模式，可以在资源受限的 MCU 与高性能的 PC 环境之间无缝切换。

### 设计目标

- ✅ **极简依赖** — 仅依赖标准 C 库，无外部依赖
- ✅ **可裁剪** — 所有功能均可通过宏开关编译时启用/禁用
- ✅ **多平台** — 支持 Windows（MSVC/MinGW）、Linux、ARM Cortex-M、ESP32(XTENSA)
- ✅ **多线程安全** — 栈内存模式和动态内存模式支持多线程
- ✅ **零动态分配可选** — 静态内存模式无需 malloc

---

## ✨ 特性

| 特性 | 说明 |
|------|------|
| **6 级日志等级** | Trace → Debug → Info → Warn → Error → Fatal |
| **4 套色彩方案** | One Dark Hybrid、Gruvbox Dark、Tokyo Night Storm、Nord Polar |
| **运行时间** | 4 种格式：微缩/时间戳/缩略/完全 |
| **本地时间** | 3 种格式：时间戳/缩略/完全 |
| **3 种内存模式** | 静态（零分配）、栈、动态（malloc/free） |
| **3 种标签风格** | 完整名 `[ trace ]`、精简 `[T]`、略简 `[TRC]` |
| **运行时控制** | 运行时动态开关各级日志和时间输出 |
| **ANSI 色彩** | 基于 ANSI 真彩色（24-bit），自动关闭转义 |
| **调试信息** | Debug 等级自动输出文件名+行号 |

---

## 🚀 快速开始

### 1. 拷贝文件

将以下两个文件加入你的项目：

```
rune-log/
├── R_Log.h          # 头文件（配置宏在此修改）
├── R_Log.c          # 实现文件
```

### 2. 配置

打开 `R_Log.h`，按需修改配置宏：

```c
#define R_LOG_ENABLE     1     // 日志总开关
#define R_LOG_BUF_SIZE   512   // 缓冲区大小
#define R_LOG_MEM_MODE   0     // 内存模式（0=静态, 1=栈, 2=动态）
#define R_LOG_LEVEL      3     // 日志等级（0~3）
#define R_LOG_TIDY       0     // 标签精简（0=完整, 1=精简, 2=略简）
#define R_LOG_TIME_RUN   4     // 运行时间格式（0~4）
#define R_LOG_TIME_LOCAL 3     // 本地时间格式（0~3）
#define R_LOG_COLOR      1     // 色彩开关
#define R_LOG_COLOR_SCHEME 0   // 配色方案（0~3）
```

### 3. 初始化并打印

```c
#include "R_Log.h"

int main(void)
{
    r_log_init();

    RLOG_INFO("服务启动成功，版本: %s", "v1.1");
    RLOG_ERROR("数据库连接失败: %s:%d", "localhost", 3306);

    return 0;
}
```

---

## ⚙️ 配置详解

### 📊 日志等级 (`R_LOG_LEVEL`)

| 值 | 可用等级 | 说明 |
|----|---------|------|
| 0 | Error, Fatal | 仅输出错误和致命错误 |
| 1 | Info, Warn, Error, Fatal | 常规输出模式 |
| 2 | Debug, Info, Warn, Error, Fatal | 调试模式（含文件名+行号） |
| 3 | Trace, Debug, Info, Warn, Error, Fatal | 全开追踪模式 |

### 🧠 内存模式 (`R_LOG_MEM_MODE`)

| 值 | 模式 | 多线程安全 | 说明 |
|----|------|-----------|------|
| 0 | 静态内存 | ❌ | 全局数组，零分配，省内存 |
| 1 | 栈内存 | ✅ | 函数内栈上分配，需保证栈空间 |
| 2 | 动态内存 | ✅ | malloc/free，需提供堆实现 |

### ⏱️ 运行时间格式 (`R_LOG_TIME_RUN`)

| 值 | 格式 | 示例 |
|----|------|------|
| 0 | 关闭 | — |
| 1 | 微缩 | `[ 01.234]` |
| 2 | 时间戳 | `[00000000]` |
| 3 | 缩略 | `[00:01:02.345]` |
| 4 | 完全 | `[0000\|00:01:02.345]` |

### 📅 本地时间格式 (`R_LOG_TIME_LOCAL`)

| 值 | 格式 | 示例 |
|----|------|------|
| 0 | 关闭 | — |
| 1 | 时间戳 | `[00000000]` |
| 2 | 缩略 | `[14:30:25]` |
| 3 | 完全 | `[2026-06-03 14:30:25]` |

### 🎨 配色方案 (`R_LOG_COLOR_SCHEME`)

| 值 | 名称 | 风格 |
|----|------|------|
| 0 | One Dark Hybrid | 冷色调，Atom 风格 |
| 1 | Gruvbox Dark | 暖色调，复古风格 |
| 2 | Tokyo Night Storm | 蓝紫调，霓虹风格 |
| 3 | Nord Polar | 北极蓝，极简风格 |

### 🏷️ 标签精简 (`R_LOG_TIDY`)

| 值 | Trace | Debug | Info | Warn | Error | Fatal |
|----|-------|-------|------|------|-------|-------|
| 0 | `[ trace ]` | `[ debug ]` | `[ info ]` | `[ warn ]` | `[ erro ]` | `[ fatal ]` |
| 1 | `[T]` | `[D]` | `[I]` | `[W]` | `[E]` | `[F]` |
| 2 | `[TRC]` | `[DBG]` | `[INF]` | `[WRN]` | `[ERR]` | `[FTL]` |

---

## 📚 API 参考

### 初始化与配置

| 函数 | 说明 |
|------|------|
| `void r_log_init(void)` | 初始化日志系统，记录启动时间，设置默认等级 |
| `void r_log_set_out_level(r_log_level_t level, uint8_t enable)` | 运行时开关指定日志等级 |
| `void r_log_set_time_level(r_log_time_level_t t_level, uint8_t s_level)` | 运行时开关/切换时间格式 |
| `void r_log_set_new_line(uint8_t enable)` | 运行时开关自动换行 |

### 日志输出宏

| 宏 | 等级 | 条件 |
|----|------|------|
| `RLOG_TRACE(fmt, ...)` | Trace | `R_LOG_LEVEL >= 3` |
| `RLOG_DEBUG(fmt, ...)` | Debug | `R_LOG_LEVEL >= 2` |
| `RLOG_INFO(fmt, ...)` | Info | `R_LOG_LEVEL >= 1` |
| `RLOG_WARN(fmt, ...)` | Warn | `R_LOG_LEVEL >= 1` |
| `RLOG_ERROR(fmt, ...)` | Error | `R_LOG_LEVEL >= 0` |
| `RLOG_FATAL(fmt, ...)` | Fatal | `R_LOG_LEVEL >= 0` |

当对应等级被编译关闭时，宏展开为 `((void)0)`，零开销。

### 返回值

| 常量 | 值 | 说明 |
|------|----|------|
| `R_LOG_OK` | 0 | 正常 |
| `R_LOG_ERROR_BUF_SIZE` | -1 | 缓冲区不足 |
| `R_LOG_ERROR_CODE` | -2 | 编码错误 |
| `R_LOG_ERROR_MEMORY` | -3 | 内存不足 |

---

## 💡 使用场景示例

> 以下为 `example.c` 编译运行后的日志输出效果展示。

<p align="center">
  <img src="img/demo_usage_scenarios.png" alt="example.c 运行输出效果" width="800"/>
</p>

---

## 🔧 运行时动态控制示例

```c
// 初始化
r_log_init();

// 动态关闭/开启指定等级
r_log_set_out_level(rlv_debug, RLV_OFF);   // 关闭 Debug
r_log_set_out_level(rlv_debug, RLV_ON);    // 开启 Debug

// 动态切换时间格式
r_log_set_time_level(rlv_time_run, 0);     // 关闭运行时间
r_log_set_time_level(rlv_time_run, 1);     // 微缩格式
r_log_set_time_level(rlv_time_run, 4);     // 完全格式

// 关闭换行（用于连续输出）
r_log_set_new_line(RLV_OFF);
```

---

## 📂 项目结构

```
rune-log/
├── R_Log.h           # 头文件 —— 全部配置与 API 声明
├── R_Log.c           # 实现文件 —— 核心日志引擎
├── example.c         # 使用示例 —— 12 个真实场景演示
├── img/
│   └── demo_usage_scenarios.png   # example.c 运行输出效果
├── LICENSE           # MIT 开源许可证
└── README.md         # 本文档
```

---

## 🧪 编译与运行示例

```powershell
# Windows (MinGW)
gcc example.c R_Log.c -o example.exe
./example.exe

# Linux
gcc example.c R_Log.c -o example
./example
```

---

## 🔌 用户需提供的接口

Rune Log 的底层输出依赖于平台相关接口，用户需根据目标平台自行实现以下 `static inline` 函数：

### 1. 日志发送接口

```c
static inline void r_log_send_data(uint8_t *buf, uint16_t len);
```

将格式化后的日志缓冲区数据发送到目标输出（如 UART、串口、文件等）。

**示例 — 通过 UART 输出：**
```c
static inline void r_log_send_data(uint8_t *buf, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        uart_send_byte(buf[i]);
    }
}
```

### 2. 运行时间获取接口

```c
static inline void r_log_time_run_get(r_log_time_run_t *t_run);
```

获取设备上电后的运行时间，填充到 `r_log_time_run_t` 结构体中。

| 字段 | 类型 | 说明 |
|------|------|------|
| `time_stemp` | `uint32_t` | 运行时间戳（秒） |
| `day` | `uint16_t` | 天数 |
| `hour` | `uint8_t` | 小时 |
| `min` | `uint8_t` | 分钟 |
| `sec` | `uint8_t` | 秒 |
| `ms` | `uint16_t` | 毫秒 |

**示例 — Windows (GetTickCount64)：**
```c
static inline void r_log_time_run_get(r_log_time_run_t *t_run)
{
    ULONGLONG elapsed = GetTickCount64() - start;
    t_run->time_stemp = elapsed / 1000;
    t_run->day  = elapsed / 86400000;
    t_run->hour = (elapsed / 3600000) % 24;
    t_run->min  = (elapsed / 60000) % 60;
    t_run->sec  = (elapsed / 1000) % 60;
    t_run->ms   = elapsed % 1000;
}
```

### 3. 本地时间获取接口

```c
static inline void r_log_time_local_get(r_log_time_local_t *t_local);
```

获取当前系统本地时间，填充到 `r_log_time_local_t` 结构体中。

| 字段 | 类型 | 说明 |
|------|------|------|
| `time_stemp` | `uint32_t` | Unix 时间戳 |
| `year` | `uint16_t` | 年 |
| `month` | `uint8_t` | 月 |
| `day` | `uint8_t` | 日 |
| `hour` | `uint8_t` | 时 |
| `min` | `uint8_t` | 分 |
| `sec` | `uint8_t` | 秒 |

**示例 — Windows (localtime)：**
```c
static inline void r_log_time_local_get(r_log_time_local_t *t_local)
{
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    t_local->time_stemp = now;
    t_local->year  = local->tm_year + 1900;
    t_local->month = local->tm_mon + 1;
    t_local->day   = local->tm_mday;
    t_local->hour  = local->tm_hour;
    t_local->min   = local->tm_min;
    t_local->sec   = local->tm_sec;
}
```

### 4. 内存分配接口（仅动态内存模式需要）

```c
static inline void* r_log_alloc(size_t size);
static inline void  r_log_free(void *addr);
```

当 `R_LOG_MEM_MODE == 2`（动态内存模式）时，需提供内存分配与释放函数。

**示例：**
```c
static inline void* r_log_alloc(size_t size) { return malloc(size); }
static inline void  r_log_free(void *addr)   { free(addr); }
```

> 💡 Linux 和嵌入式平台需要手动适配上述接口，适配指南请参考各接口的示例代码。

---

## ⚠️ 免责声明

本程序是开源产物，任何人都可以自由使用、修改和分发。

但请务必注意：**使用者需自行承担一切风险**。作者不提供任何形式的保证，无论是在明示或暗示的法律条款下，包括但不限于适销性、特定用途适用性及非侵权性。在任何情况下，作者均不对因使用本程序而产生的任何直接、间接、附带、特殊、惩罚性或后续损失承担责任。

---
