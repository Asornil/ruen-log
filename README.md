<h1 align="center">Rune Log · 符文日志</h1>

<p align="center">
  <strong>轻量级、高可配、驱动分离的 C 语言日志库 — 桌面与嵌入式通用</strong>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/语言-C99-blue.svg"/>
  <img src="https://img.shields.io/badge/许可证-MIT-green.svg"/>
  <img src="https://img.shields.io/badge/版本-v1.2-orange.svg"/>
  <img src="https://img.shields.io/badge/平台-Windows%20%7C%20Linux%20%7C%20STM32%20%7C%20ESP32-lightgrey.svg"/>
  <img src="https://img.shields.io/badge/模式-同步%20%7C%20异步-brightgreen.svg"/>
</p>

---

## 📖 简介

**Rune Log** 是一个用纯 C99 编写的轻量级日志库，专为**嵌入式系统**和**桌面应用**设计。核心与平台彻底分离：核心引擎只负责日志组装（时间、颜色、标签、缓冲、裁剪），底层能力全部收敛到**驱动层**，通过编译器宏选择目标平台，不依赖任何构建系统脚本。

### 设计目标

- ✅ **驱动分离** — 核心零平台代码，新平台 = 新增一个驱动文件
- ✅ **宏裁剪** — 平台、内存、输出、功能全部编译期宏开关，跨 IDE/命令行通用
- ✅ **极简依赖** — 仅依赖标准 C 库 + 编译器内置原子（`__sync` / `_Interlocked` / `__ldrex`）
- ✅ **多线程安全** — 内存池取还、异步队列入队均为原子操作
- ✅ **同步 / 异步双模式** — 直接发送或缓存队列发送（`r_log_flush` / `r_log_poll`）
- ✅ **零动态分配可选** — 静态内存池模式运行期零 malloc

---

## ✨ 特性

| 特性 | 说明 |
|------|------|
| **6 级日志等级** | Trace → Debug → Info → Warn → Error → Fatal |
| **4 套色彩方案** | One Dark Hybrid、Gruvbox Dark、Tokyo Night Storm、Nord Polar |
| **4 种内存模式** | 静态 / 栈 / 动态 malloc / 静态内存池（原子取还） |
| **2 种输出模式** | 同步直接发送 / 异步缓存队列（排空、轮询双 API） |
| **驱动层分离** | 时间、输出、内存按平台实现，自动检测 + 手动指定 |
| **运行时间** | 4 种格式：微缩 / 时间戳 / 缩略 / 完全 |
| **本地时间** | 3 种格式：时间戳 / 缩略 / 完全 |
| **3 种标签风格** | 完整名 `[ trace ]`、精简 `[T]`、略简 `[TRC]` |
| **运行时控制** | 动态开关各级日志与时间输出 |
| **TAG 模块过滤** | 按模块登记等级 / 开关，运行时动态控制 |
| **ANSI 色彩** | 24-bit 真彩色，4 套配色方案 |

---

## 🚀 快速开始

### 1. 拷贝文件

```
rune-log/
├── src/Rlog.h              # 核心头文件（配置宏在此修改）
├── src/Rlog.c              # 核心引擎
├── drivers/r_log_driver.h  # 驱动接口 + 平台选择
└── drivers/vendor_xxx.c    # 目标平台驱动（windows / linux / stm32 / esp32）
```

将核心两个文件 + 驱动两个文件加入工程，其余 vendor 驱动文件无需添加。

### 2. 选择平台

平台由 `R_LOG_DRIVER` 宏决定（定义于 `drivers/r_log_driver.h`）：

| 值 | 宏 | 平台 | 状态 |
|----|-----|------|------|
| 1 | `R_LOG_DRIVER_WINDOWS` | Windows | ✅ 已实现 |
| 2 | `R_LOG_DRIVER_LINUX` | Linux | ✅ 已实现 |
| 3 | `R_LOG_DRIVER_STM32` | STM32 (F103C8T6) | ✅ 已实现（示例驱动） |
| 4 | `R_LOG_DRIVER_ESP32` | ESP32 (XTENSA) | 🚧 开发中 |

**不指定时自动检测**（`_WIN32` / `__linux__` / `__ARM_ARCH_7M__` / `__XTENSA__`），检测失败会 `#error` 提示手动指定。交叉编译或编译器宏不齐全时，用 `-DR_LOG_DRIVER=3` 手动指定（Keil/IAR 在 Preprocessor Define 里同样写法）。

> STM32 工程还需：链接 ST 标准外设库（SPL），Preprocessor 定义 `STM32F10X_MD`（或新版头文件的 `STM32F103xB`）；驱动 `alloc/free` 走 `malloc`，需在启动文件配置足够的堆大小（或用 `R_LOG_MEM_MODE=1/3` 避开动态分配）。

### 3. 编译

**CMake：**

```powershell
cmake -S . -B build
cmake --build build
```

**手动 gcc（无需构建系统）：**

```bash
gcc example.c src/Rlog.c drivers/vendor_windows.c -I src -I drivers -o example
```

**Keil / IAR / STM32CubeIDE：** 添加 4 个源文件到工程，在 Preprocessor 中按需定义 `R_LOG_DRIVER`，直接编译即可。

### 4. 初始化并输出

```c
#include "Rlog.h"

int main(void)
{
    r_log_init();

    RLOG_INFO("app", "服务启动成功，版本: %s", "v1.2");
    RLOG_ERROR("db", "数据库连接失败: %s:%d", "localhost", 3306);

    return 0;
}
```

> 日志宏的第一个参数为**模块名 (TAG)**，传 `NULL` 表示无模块。默认 `R_LOG_OUT_MODE=1`（异步模式），日志先入队，需调用 `r_log_flush()` 发送；改用 `R_LOG_OUT_MODE=0` 则直接发送，无需 flush。

---

## ⚙️ 配置详解（src/Rlog.h）

### 内存模式 (`R_LOG_MEM_MODE`)

| 值 | 模式 | 多线程 | 说明 |
|----|------|--------|------|
| 0 | 静态内存 | ❌ | 全局数组，零分配，最省 RAM |
| 1 | 栈内存 | ✅ | 函数内栈上分配，需保证栈空间 |
| 2 | 动态内存 | ✅ | 每次调用 `malloc`，需提供内存分配驱动 |
| 3 | **静态内存池** | ✅ | 编译期固定 N×BUF 池，原子 CAS 取还，**运行期零动态分配** |

模式 3 的池大小 = `R_LOG_QUEUE_SIZE × R_LOG_BUF_SIZE`，槽位取还使用原子 CAS（编译器内置，无操作系统依赖）。

### 输出模式 (`R_LOG_OUT_MODE`)

| 值 | 模式 | 说明 |
|----|------|------|
| 0 | 同步 | 格式化后立即发送，调用方阻塞在发送上 |
| 1 | 异步 | 格式化后入队立即返回，由调用方消费（`r_log_flush` / `r_log_poll`） |

> ⚠️ **异步模式仅支持内存模式 2/3**（队列持有缓冲引用，静态/栈缓冲会被复用或失效），非法组合编译期 `#error` 拦截。

### 队列槽位数 (`R_LOG_QUEUE_SIZE`)

异步模式的消息队列深度，同时决定内存池 3 的槽位数量。默认 16。队列满时**丢弃新日志**并返回 `R_LOG_ERROR_QUNE`。

### 其他开关

| 宏 | 值 | 说明 |
|----|-----|------|
| `R_LOG_ENABLE` | 0/1 | 日志总开关，0 时全部宏为空操作，零开销 |
| `R_LOG_BUF_SIZE` | ≥128 | 单条日志缓冲区字节数 |
| `R_LOG_LEVEL` | 0~3 | 日志等级裁剪 |
| `R_LOG_TIDY` | 0~2 | 标签风格 |
| `R_LOG_TIME_RUN` | 0~4 | 运行时间格式 |
| `R_LOG_TIME_LOCAL` | 0~3 | 本地时间格式 |
| `R_LOG_COLOR` | 0/1 | 色彩开关 |
| `R_LOG_COLOR_SCHEME` | 0~3 | 配色方案 |
| `R_LOG_TAG_MODE` | 0/1 | TAG 模块过滤开关，0 时完全裁剪（tag 参数被忽略） |
| `R_LOG_TAG_MAX` | ≥1 | TAG 模块表容量（默认 16） |
| `TAG_TRACE`~`TAG_FATAL` | 位值 | 等级位掩码（`TAG_ALL` 全开，`TAG_NONE` 静音），供 `r_log_tag_set` 使用 |

---

## 📊 推荐配置与资源占用

> **测量条件**：核心 `Rlog.c` 单独编译（`-Os`，Cortex-M3/STM32F103），Flash = text+rodata，RAM = data+bss，**不含 vendor 驱动**（驱动通常 <1KB）。实际数值随编译器/版本小幅浮动。

### 嵌入式 MCU（arm-none-eabi -Os）

| 配置 | Flash | RAM | 要点 | 适用场景 |
|------|-------|-----|------|---------|
| **极致精简** | ~0.5 KB | ~0.5 KB | `MEM_MODE=0` 静态 + 同步 + 关 TAG/颜色/本地时间 | 资源紧张的小 MCU（如 F103C8T6 裸机），仅错误日志 |
| **常规推荐** | ~1.5 KB | ~0.1 KB | `MEM_MODE=1` 栈 + 同步 + TAG 过滤 | 裸机/RTOS 常规日志，RAM 极省（缓冲在栈上，每次调用占 512B） |
| **异步推荐** | ~2.8 KB | ~4.4 KB | `MEM_MODE=3` 静态池(8槽) + 异步 + TAG | 业务代码不能被日志阻塞；运行期零 malloc，RAM 预算固定 |
| **全功能** | ~2.7 KB | ~0.3 KB | `MEM_MODE=2` 动态 + 异步 + 全格式 | 带堆的设备/桌面，RAM 占用最小但依赖 malloc |

> RAM 注：`MEM_MODE=3` 的池大小 = `QUEUE_SIZE × BUF_SIZE`（上表 QUEUE=8 → 4KB，可调）；`MEM_MODE=0` 的 RAM ≈ `BUF_SIZE`；TAG 表 ≈ `TAG_MAX × 8` 字节。

### 桌面（x86_64 gcc）

| 模式 | Flash(代码) | RAM | 说明 |
|------|-----------|-----|------|
| 调试 `-O0`（默认 CMake Debug） | ~7.0 KB | ~0.8 KB | 64 位指针使 TAG/队列表体积翻倍，可断点调试 |
| 发布 `-O2` | ~9.5 KB | ~0.6 KB | 内联优化导致体积略增（性能优先），符号消除使 RAM 略降 |

### 推荐速配：MCU 常规（栈 + 同步 + TAG）

```c
// src/Rlog.h
#define R_LOG_MEM_MODE      1      // 栈缓冲，RAM 极省
#define R_LOG_OUT_MODE      0      // 同步直接发送
#define R_LOG_TAG_MODE      1      // 模块过滤
#define R_LOG_BUF_SIZE      512
#define R_LOG_LEVEL         2      // 含 Debug(带文件名行号)
#define R_LOG_TIME_RUN      1      // 仅运行时间(秒.毫秒)
#define R_LOG_TIME_LOCAL    0      // 无 RTC 时关闭本地时间
#define R_LOG_TIDY          1      // 精简标签 [T] [I] ...
#define R_LOG_COLOR         1      // 串口工具不支持 ANSI 时改 0 再省 ~0.3KB
```

### 推荐速配：MCU 异步（池 + 队列）

```c
#define R_LOG_MEM_MODE      3      // 静态池，运行期零 malloc
#define R_LOG_OUT_MODE      1      // 异步：日志入队立即返回
#define R_LOG_QUEUE_SIZE    8      // 池槽位/队列深度，RAM = 8×512 = 4KB
#define R_LOG_TAG_MODE      1
#define R_LOG_TIME_LOCAL    0      // 无 RTC 时关闭本地时间
```

> 桌面保持默认即可（`MEM_MODE=2` + `OUT_MODE=1`），RAM 占用最小，异步消费参考上文「异步模式消费指南」。

---

## 🔌 驱动层

### 接口（drivers/r_log_driver.h）

核心引擎通过以下接口获取平台能力，**核心文件不含任何平台代码**：

| 接口 | 职责 | Windows 实现 | Linux 实现 |
|------|------|-------------|-----------|
| `r_log_driver_init()` | 记录启动 tick | `GetTickCount64` | `gettimeofday` |
| `r_log_driver_send(buf,len)` | 日志输出 | `printf` | `printf` |
| `r_log_driver_time_run_get()` | 运行时间 | `GetTickCount64` | `gettimeofday` |
| `r_log_driver_time_local_get()` | 本地时间 | `localtime` | `localtime_r` |
| `r_log_driver_alloc/free()` | 动态内存 | `malloc/free` | `malloc/free` |

每个 vendor 文件用 `#if (R_LOG_DRIVER == R_LOG_DRIVER_xxx)` 包裹实现，未选中平台编译为空单元。

### STM32 示例驱动（vendor_stm32.c）

完整可运行的**示例驱动**，基于 ST 标准外设库（SPL），供 F103C8T6 裸机开箱使用；不满足需求时可直接修改或整体替换。

| 能力 | 实现 |
|------|------|
| 输出 | USART1 DMA 发送（115200，带超时） |
| 运行时间 | SysTick 毫秒计数 |
| 本地时间 | RTC（LSI）叠加时区偏移，支持 1970-2106 |
| 内存 | `malloc/free`（需配置堆） |

定制点（均在 `vendor_stm32.c`）：

- `serial_init()` 中 `rtc_set_unix_time()` / `rtc_set_time_zone()` 为硬编码默认值（初始时间戳 + 时区分钟，如 320 = 东八区），上电后调用覆盖即可
- 波特率 / 环形缓冲 / DMA 缓冲大小均为文件顶部宏
- 驱动额外占用 RAM：环形缓冲 512B + DMA 缓冲 128B（与核心 RAM 预算无关）
- 文件内 `CHIP` 宏区分实现：`STM32_F103C8T6_STD`（标准库，已实现）/ `STM32_F103C8T6_HAL`（HAL 库，预留）

### 接入新平台（3 步）

1. 新建 `drivers/vendor_xxx.c`，实现上述 6 个接口（含文件内 `#if` 守卫）
2. 在 `r_log_driver.h` 平台枚举中登记新平台值，并补充自动检测分支
3. 编译时 `-DR_LOG_DRIVER=N` 或由编译器宏自动识别

---

## 📚 API 参考

| 函数 | 说明 |
|------|------|
| `r_log_init()` | 初始化：记录启动时间、设置默认等级、清空池与队列 |
| `r_log_set_out_level(level, enable)` | 运行时开关指定日志等级 |
| `r_log_set_time_level(t_level, s_level)` | 运行时开关/切换时间格式 |
| `r_log_set_new_line(enable)` | 运行时开关自动换行 |
| `r_log_tag_set(tag, mask)` | **TAG**：登记/设置模块输出等级掩码（`TAG_*` 位组合，0=静音） |
| `r_log_out(level, file, line, fmt, ...)` | 底层输出接口（经宏调用） |
| `r_log_poll()` | **异步**：仅发送一条，返回 1=已发送 0=队列空 |
| `r_log_flush()` | **异步**：排空队列全部发送，返回发送条数 |

### 日志宏

| 宏 | 等级 | 编译条件 |
|----|------|---------|
| `RLOG_TRACE(tag, fmt, ...)` | Trace | `R_LOG_LEVEL >= 3` |
| `RLOG_DEBUG(tag, fmt, ...)` | Debug | `R_LOG_LEVEL >= 2`（含文件名+行号） |
| `RLOG_INFO(tag, fmt, ...)` | Info | `R_LOG_LEVEL >= 1` |
| `RLOG_WARN(tag, fmt, ...)` | Warn | `R_LOG_LEVEL >= 1` |
| `RLOG_ERROR(tag, fmt, ...)` | Error | `R_LOG_LEVEL >= 0` |
| `RLOG_FATAL(tag, fmt, ...)` | Fatal | `R_LOG_LEVEL >= 0` |

> 所有宏的第一个参数为模块名（TAG），传 `NULL` 表示无模块；被裁剪的等级展开为 `((void)0)`，零开销。

### 返回值

| 常量 | 值 | 含义 |
|------|----|------|
| `R_LOG_OK` | 0 | 正常 |
| `R_LOG_ERROR_BUF_SIZE` | -1 | 缓冲区不足 |
| `R_LOG_ERROR_CODE` | -2 | 编码错误 |
| `R_LOG_ERROR_MEMORY` | -3 | 内存不足 |
| `R_LOG_ERROR_SOLT` | -4 | 内存池槽位耗尽 |
| `R_LOG_ERROR_QUNE` | -5 | 队列已满，日志被丢弃 |
| `R_LOG_ERROR_TAG` | -6 | 模块(TAG)被过滤丢弃 |

---

## 🧵 多线程与原子性

- 内存模式 0 为单线程设计；模式 1/2/3 支持多线程
- 模式 3 池槽位取还、异步队列入队/出队均为 **CAS 原子操作**，无需操作系统锁，跨编译器实现：
  - MSVC：`_InterlockedCompareExchange`
  - ARM Compiler 5 (Keil)：`__ldrex` / `__strex`
  - GCC / Clang（含 arm-none-eabi、xtensa、MinGW）：`__sync_bool_compare_and_swap`
- 异步模式下多个业务线程可同时调用日志宏：取块（CAS）→ 格式化（各自私有缓冲）→ 锁内入队 → 立即返回

---

## 📤 异步模式消费指南

`R_LOG_OUT_MODE=1` 时日志进入队列，需消费方将消息发送出去：

| 平台 | 推荐做法 |
|------|---------|
| 桌面 (Win/Linux) | 专用线程或定时器内循环调用 `r_log_poll()`；退出前调 `r_log_flush()` 兜底 |
| RTOS (FreeRTOS 等) | 低优先级任务内调用 `r_log_poll()`（返回 0 时休眠让出） |
| 裸机 MCU | 主循环内调用 `r_log_poll()`；关键节点（如进入低功耗前）调 `r_log_flush()` |

**满队列策略：丢弃新日志**（返回 `R_LOG_ERROR_QUNE`），保证业务线程永不被日志阻塞。

---

## 🎯 模块过滤（TAG 模式）

`R_LOG_TAG_MODE=1` 时，日志宏携带的模块名参与过滤。**所有日志宏第一个参数即为模块名**，传 `NULL` 表示无模块。

### 过滤语义

| 情形 | 行为 |
|------|------|
| `tag == NULL` | 跟随全局，不受模块过滤影响 |
| 模块**未登记** | 跟随全局（默认放行，无需登记即可输出） |
| 模块已登记 | 仅输出 **等级位掩码** 内包含的等级 |
| 掩码为 **0**（`TAG_NONE`） | 模块静音（等价于关闭，不输出任何等级） |

### API

```c
// 等级位掩码：每个等级占 1 位，可用 | 自由组合（也可自定义运算）
// TAG_TRACE(0x01) TAG_DEBUG(0x02) TAG_INFO(0x04)
// TAG_WARN(0x08)  TAG_ERROR(0x10) TAG_FATAL(0x20)
// TAG_ALL(0x3F)   全部等级    TAG_NONE(0x00)  静默

// 登记并设置模块输出等级掩码（未登记则追加，重复调用覆盖更新）
r_log_tag_set("wifi", TAG_WARN | TAG_ERROR | TAG_FATAL);  // 指定输出登记 warn error fatal
r_log_tag_set("db",   TAG_DEBUG | TAG_ERROR);             // 指定输出等级 debug error
r_log_tag_set("sec",  TAG_NONE);                          // 静默模块
r_log_tag_set("sec",  TAG_ALL);                           // 全启动
```

### 过滤与全局的关系（AND）

一条日志需要**同时满足**：编译期等级裁剪 → 运行时全局等级开关 → 模块过滤，才会输出。模块过滤只能比全局更严，不能更松。

`R_LOG_TAG_MODE=0` 时功能完全裁剪：模块表不分配、`r_log_tag_set` 为空操作、日志宏的 tag 参数被忽略——业务代码无需修改，仅改宏即可。

---

## 💡 使用场景示例

`example.c` 演示 12 个真实场景：启动初始化、HTTP 请求、文件传输、定时任务、微服务调用链、安全审计、数据库操作、系统监控、消息队列、缓存、容器编排、压测指标。每个场景使用独立模块名（`app`/`http`/`file`/`cron`/`gateway`/`sec`/`db`/`mon`/`mq`/`cache`/`k8s`/`perf`），顶部注释含 TAG 过滤体验开关。

<p align="center">
  <img src="img/demo_usage_scenarios.png" alt="example.c 运行输出效果" width="800"/>
</p>

---

## 📂 项目结构

```
rune-log/
├── src/
│   ├── Rlog.h              # 核心头文件 —— 配置宏 + API 声明
│   └── Rlog.c              # 核心引擎 —— 组装/池/队列/原子
├── drivers/
│   ├── r_log_driver.h      # 驱动接口 + 平台枚举 + 自动检测
│   ├── vendor_windows.c    # Windows 驱动
│   ├── vendor_linux.c      # Linux 驱动
│   ├── vendor_stm32.c      # STM32 标准库驱动（示例，可替换）
│   └── vendor_esp32.c      # ESP32 驱动（开发中）
├── example.c               # 12 场景示例
├── CMakeLists.txt          # 可选：纯文件列表，无平台逻辑
├── img/                    # 示例输出截图
├── LICENSE
└── README.md
```

---

## ⚠️ 免责声明

本程序是开源产物，任何人都可以自由使用、修改和分发。

但请务必注意：**使用者需自行承担一切风险**。作者不提供任何形式的保证，无论是在明示或暗示的法律条款下，包括但不限于适销性、特定用途适用性及非侵权性。在任何情况下，作者均不对因使用本程序而产生的任何直接、间接、附带、特殊、惩罚性或后续损失承担责任。

---
