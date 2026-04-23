# sloopLite

轻量级裸机协作式任务调度框架

## 项目简介

sloopLite 是一个专为资源受限型微控制器设计的轻量级嵌入式任务调度框架。它采用单线程协作式调度模型，提供了丰富的任务管理功能和协作式工作流编程能力，同时保持极低的资源消耗，非常适合资源受限的嵌入式应用开发。

## 核心特性

### 任务管理
- **超时任务**：延迟指定时间后执行一次
- **周期任务**：按照固定时间间隔重复执行
- **多次任务**：执行指定次数的周期任务
- **并行任务**：在主循环中并行执行
- **单次任务**：只执行一次，适合中断中复杂逻辑下放
- **互斥任务**：主要业务逻辑执行载体

### 系统服务
- **RTT 日志输出**：基于 SEGGER RTT 的高效日志系统
- **CPU 负载监测**：实时计算和显示系统负载
- **系统心跳**：定期输出系统状态
- **非阻塞等待**：支持任务间的延时和同步
- **时间戳功能**：提供系统时间获取接口

### 协作式工作流
- **简化语法**：基于宏定义的工作流编程框架
- **非阻塞等待**：支持延时、条件和事件等待
- **状态管理**：自动维护工作流生命周期
- **事件驱动**：支持工作流间通信
- **线性结构**：逻辑清晰易读，避免复杂状态机

## 任务调度机制

### 执行结构
主循环持续调用 `sloop()`，内部顺序执行：
- **mutex_task_run()**：运行当前唯一主任务
- **parallel_task_run()**：轮询所有并行任务

同时，tick 中断触发 `soft_timer`，驱动：
- 超时任务（一次性）
- 周期任务
- 多次任务

形成 "时间驱动 + 主流程" 的组合执行模型。

### 生命周期机制
通过 `SL_INIT`/`SL_FREE`/`SL_RUN` 将任务拆成三个阶段：
- **SL_INIT**：首次进入执行一次（初始化资源、启动定时器等）
- **SL_RUN**：主逻辑区（循环执行）
- **SL_FREE**：任务切换时执行（释放资源）

任务切换由 `sl_goto()` 触发，本质是：
1. 标记 `sl_free`
2. 下一轮执行 `SL_FREE`
3. 加载新任务（`sl_load_new_task`）
4. 重新进入 `SL_INIT`

### 同步机制
通过 `sl_wait`/`sl_wait_bare` 提供 "伪阻塞" 能力：
- 当前任务进入等待循环
- 内部持续执行并行任务
- 通过 `sl_wait_break`/`continue` 进行唤醒

特点是：写法阻塞，但系统不阻塞。

## Flow 机制

### 本质
Flow 是一个用宏构建的协作式工作流框架，将任务调度、状态机和同步原语统一在同一套语义下运行。每个 Flow 都是一个 "可挂起的执行体"，通过 switch-case + 静态局部变量保存执行上下文，借助 `__LINE__` 生成唯一状态，实现类似协程的断点续执行。

### 核心概念
- **Flow 状态**：每个 Flow 都有一个状态变量，用于保存执行位置
- **Flow 事件**：用于 Flow 之间的通信
- **Flow 上下文**：每个 Flow 可以定义自己的静态变量，作为工作流的上下文

### 生命周期
- **FLOW_INIT**：只执行一次，用于初始化上下文
- **FLOW_RUN**：主运行阶段，逻辑按 "线性代码" 展开
- **FLOW_FREE**：退出阶段，用于资源释放

外部通过 `FLOW_START`/`FLOW_STOP` 控制生命周期切换，内部也可用 `FLOW_EXIT` 主动结束。

### 核心能力
**非阻塞等待**：
- **FLOW_UNTIL**：条件不满足时让出调度，满足后从断点继续
- **FLOW_WAIT**：基于时间的非阻塞等待
- **FLOW_WAIT_EVENT**：基于事件的非阻塞等待

### 关键优势
**线性同步写法**：传统状态机需要拆成多个 state + 跳转，而这里可以用 "顺序代码" 表达复杂流程，逻辑更接近人脑思维路径，显著降低状态爆炸和可读性成本。

### Flow 范式

Flow工作流采用标准范式编写，包含上下文定义、初始化、释放和运行逻辑等部分：

```c
void flow2(void)
{
    /* 工作流上下文，工作流需要的数据在此静态定义 */
    SL_FLOW_CONTEXT(flow2);

    /* 初次进入工作流，执行一次，初始化工作流上下文 */
    SL_FLOW_INIT;

    /* 工作流结束，不再执行时，释放资源 */
    SL_FLOW_FREE(flow2);

    /* 下方开始进入工作流运行逻辑 */
    SL_FLOW_RUN;

    FLOW_UNTIL(var > 3);

    FLOW_WAIT_EVENT(evt1);

    FLOW_WAIT(2000);

    SL_FLOW_END;
}
```

### Flow 宏定义

Flow 框架提供了以下核心宏：

| 宏 | 描述 |
|-----|-----|
| `FLOW_STATE_DEFINE(flow_name)` | 定义 Flow 状态变量 |
| `FLOW_EVENT_DEFINE(event_name)` | 定义 Flow 事件变量 |
| `FLOW_START(flow_name)` | 启动 Flow |
| `FLOW_STOP(flow_name)` | 停止 Flow |
| `SL_FLOW_CONTEXT(flow_name)` | 定义 Flow 上下文 |
| `SL_FLOW_INIT` | 标记初始化代码 |
| `SL_FLOW_FREE(flow_name)` | 标记释放代码 |
| `SL_FLOW_RUN` | 标记运行逻辑 |
| `SL_FLOW_END` | 标记 Flow 结束 |
| `FLOW_UNTIL(condition)` | 等待条件满足 |
| `FLOW_WAIT(ms)` | 非阻塞等待指定时间 |
| `FLOW_WAIT_EVENT(event_name)` | 等待事件触发 |
| `FLOW_SEND_EVENT(event_name)` | 发送事件 |
| `FLOW_EXIT()` | 主动结束 Flow |

### 适用场景

Flow 机制特别适合以下场景：

- **复杂业务逻辑**：需要多个步骤、条件判断和等待的复杂业务流程
- **状态管理**：需要维护状态的应用，如状态机、协议处理等
- **事件驱动**：基于事件的异步处理
- **资源管理**：需要在特定时机初始化和释放资源的场景

通过 Flow 机制，开发者可以用线性的代码结构表达复杂的异步流程，提高代码的可读性和可维护性。

## RTT 打印

### 功能特点
- **高效输出**：基于 SEGGER RTT 技术，无需占用串口资源
- **彩色日志**：支持不同级别的彩色日志输出
- **时间戳**：每条日志自动添加时间戳
- **分级显示**：支持不同级别的日志输出

### 使用方式
```c
// 打印日志（带时间戳）
sl_printf("Hello, sloopLite!");

// 打印变量
sl_prt_var(counter);

// 打印浮点数
sl_prt_float(temperature);
```

### 终端效果
![RTT终端效果](images/rtt_terminal.png)

通过 J-Link RTT Viewer 可以实时查看框架输出的日志信息，包括系统初始化、任务执行、系统状态等。

## 技术规格

### 硬件要求
- **微控制器**：STM32G0 系列
- **内存**：至少 4KB RAM
- **Flash**：至少 32KB Flash

### 软件要求
- **开发环境**：Keil MDK-ARM 5.x+
- **编译器**：ARM Compiler 6
- **STM32 HAL 库**：STM32G0xx HAL Driver

### 资源消耗
```
Total RO Size (Code + RO Data):  18648 bytes (18.21kB)
Total RW Size (RW Data + ZI Data):  3840 bytes (3.75kB)
Total ROM Size (Code + RO Data + RW Data):  18660 bytes (18.22kB)
```

## 快速开始

### 环境搭建
1. 安装 Keil MDK-ARM 5.x+
2. 安装 STM32G0 系列支持包
3. 克隆或下载 sloopLite 项目
4. 使用 Keil MDK 打开 `project/MDK-ARM/project.uvprojx`

### 项目编译
1. 选择目标设备（STM32G030K8Tx）
2. 配置编译选项
3. 点击编译按钮（F7）
4. 生成的固件位于 `project/MDK-ARM/bin/project.hex`

### 项目结构

```
├── project/
│   ├── Core/             # 核心系统文件
│   ├── Drivers/          # STM32 HAL 和 CMSIS 库
│   ├── MDK-ARM/          # Keil 项目文件
│   └── user/             # 用户应用代码
│       ├── app/          # 应用层
│       │   ├── config/   # 配置文件
│       │   └── tasks/    # 任务实现
│       └── sloop/        # 框架核心
│           ├── RTT/      # SEGGER RTT 库
│           └── kernel/   # 内核实现
├── LICENSE               # 许可证文件
└── README.md             # 项目文档
```

## 核心 API

### 系统初始化
```c
// 初始化 sloopLite 框架
void sloop_init(void);

// 运行 sloopLite 框架
void sloop(void);
```

### 任务管理

#### 超时任务
```c
// 启动超时任务
void sl_timeout_start(int ms, pfunc task);

// 停止超时任务
void sl_timeout_stop(pfunc task);
```

#### 周期任务
```c
// 启动周期任务
void sl_cycle_start(int ms, pfunc task);

// 停止周期任务
void sl_cycle_stop(pfunc task);
```

#### 多次任务
```c
// 启动多次任务
void sl_multiple_start(int num, int ms, pfunc task);

// 停止多次任务
void sl_multiple_stop(pfunc task);
```

#### 并行任务
```c
// 启动并行任务
void sl_task_start(pfunc task);

// 停止并行任务
void sl_task_stop(pfunc task);
```

#### 单次任务
```c
// 启动单次任务
void sl_task_once(pfunc task);
```

#### 互斥任务
```c
// 切换到指定任务
void sl_goto(pfunc task);
```

### 时间管理
```c
// 获取系统时间戳
uint32_t sl_get_tick(void);

// 阻塞式延时
void sl_delay(int ms);

// 非阻塞等待
char sl_wait(int ms);

// 非阻塞裸等待
char sl_wait_bare(void);
```

## 配置文件

主要配置文件位于 `project/user/app/config/sl_config.h`，可以根据需要调整以下参数：

```c
// 超时任务上限
#define SL_TIMEOUT_LIMIT 16

// 周期任务上限
#define SL_CYCLE_LIMIT 16

// 多次任务上限
#define SL_MULTIPLE_LIMIT 16

// 并行任务上限
#define SL_PARALLEL_LIMIT 32

// 单次任务上限
#define SL_ONCE_LIMIT 16

// 启用 RTT 打印
#define SL_RTT_ENABLE 1
```

## 注意事项

1. **任务设计**：互斥任务需要使用 `SL_INIT`、`SL_FREE` 和 `SL_RUN` 宏来管理任务的生命周期
2. **资源限制**：每种任务类型都有数量上限，超过上限会导致任务创建失败
3. **实时性**：由于采用协作式调度，任务需要主动让出 CPU 资源
4. **中断处理**：中断中应避免执行复杂逻辑，建议使用 `sl_task_once()` 将复杂逻辑下放
5. **内存管理**：框架不提供动态内存管理，需要用户自行管理内存

## 移植指南

### 基本移植步骤

1. **系统时钟配置**：确保系统时钟已正确配置，systick 中断周期为 1ms

2. **Systick 中断处理**：在 systick 中断处理函数中添加 `sl_tick_irq();` 调用，示例：
   ```c
   void SysTick_Handler(void)
   {
       HAL_IncTick();
       sl_tick_irq(); // 调用 sloopLite 时钟更新函数
   }
   ```

3. **框架初始化**：在主函数中初始化 sloopLite 框架并启动主循环：
   ```c
   int main(void)
   {
       // 系统初始化代码
       
       sloop_init(); // 初始化 sloopLite 框架
       
       while (1)
       {
           sloop(); // 运行 sloopLite 主循环
       }
   }
   ```

### 移植友好的核心优势
1. 极简的依赖 ：sloopLite 几乎没有外部依赖，只需要一个 1ms 精度的系统时钟，这在大多数微控制器上都能轻松实现。
2. 统一的接口 ：无论目标设备是什么，移植步骤基本一致：
   
   - 配置系统时钟
   - 在中断中添加 sl_tick_irq() 调用
   - 初始化框架并启动主循环
3. 灵活的配置 ：通过 sl_config.h 文件，可以根据目标设备的资源情况调整任务数量等参数，适应不同硬件的能力。
4. 模块化设计 ：框架核心与硬件相关的部分被隔离，移植时主要关注时钟和中断处理，其他部分无需修改。
5. RTT 日志的可选性 ：对于不支持 J-Link 的设备，可以通过简单修改宏定义来禁用 RTT 日志功能，不影响核心功能。
### 实际移植体验
从实际操作来看，移植 sloopLite 到新的 MCU 通常只需要：

- 几行代码的修改（主要是中断处理函数）
- 简单的配置调整
- 无需修改框架核心代码
这种设计理念非常符合嵌入式开发的需求，尤其是在资源受限的环境中，简单、可靠、易于移植的框架能够大大减少开发和维护成本。

sloopLite 的移植友好性也体现了其轻量级的设计目标，通过最小化硬件依赖和简化移植流程，让开发者能够更专注于应用逻辑的实现，而不是框架本身的适配问题。

### 注意事项

1. **RTT 日志支持**：RTT 日志功能基于 SEGGER RTT 技术，仅支持 J-Link 支持的设备，如 ARM Cortex-M0、M3、M4、M7 等系列微控制器

2. **时钟精度**：sloopLite 依赖于 1ms 精度的系统时钟，确保 systick 配置正确

3. **任务数量**：根据目标设备的资源情况，合理调整 `sl_config.h` 中的任务数量上限

4. **中断优先级**：确保 systick 中断优先级设置合理，避免影响系统实时性

### 支持的设备

sloopLite 框架适用于以下类型的设备：

- **ARM Cortex-M 系列**：M0、M0+、M3、M4、M7 等
- **其他支持 C 语言的微控制器**：只要能提供 1ms 精度的系统时钟，均可移植使用

对于不支持 J-Link 的设备，可以通过修改 `sl_config.h` 中的 `SL_RTT_ENABLE` 宏为 0 来禁用 RTT 日志功能。

## 许可证

本项目采用 MIT 许可证，详见 LICENSE 文件。

## 贡献

欢迎提交 Issue 和 Pull Request 来改进 sloopLite 框架。

---

sloopLite - 轻量级嵌入式任务调度框架



# English Version

Lightweight Embedded Bare-Metal Cooperative Task Scheduling Framework

## Project Introduction

sloopLite is a lightweight embedded task scheduling framework designed specifically for resource-constrained microcontrollers. It adopts a single-threaded cooperative scheduling model, providing rich task management functions and collaborative workflow programming capabilities while maintaining extremely low resource consumption, making it very suitable for resource-constrained embedded application development.

## Core Features

### Task Management
- **Timeout Task**: Execute once after a specified delay
- **Cycle Task**: Repeat execution at fixed time intervals
- **Multiple Task**: Execute periodic tasks for a specified number of times
- **Parallel Task**: Execute in parallel in the main loop
- **Once Task**: Execute only once, suitable for offloading complex logic from interrupts
- **Mutex Task**: Main business logic execution carrier

### System Services
- **RTT Log Output**: Efficient logging system based on SEGGER RTT
- **CPU Load Monitoring**: Real-time calculation and display of system load
- **System Heartbeat**: Periodic output of system status
- **Non-blocking Wait**: Support for delay and synchronization between tasks
- **Timestamp Function**: Provides system time acquisition interface

### Collaborative Workflow
- **Simplified Syntax**: Macro-based workflow programming framework
- **Non-blocking Wait**: Supports delay, condition, and event waiting
- **State Management**: Automatically maintains workflow lifecycle
- **Event Driven**: Supports inter-workflow communication
- **Linear Structure**: Clear and readable logic, avoiding complex state machines

## Task Scheduling Mechanism

### Execution Structure
The main loop continuously calls `sloop()`, which internally executes in sequence:
- **mutex_task_run()**: Runs the current unique main task
- **parallel_task_run()**: Polls all parallel tasks

At the same time, the tick interrupt triggers `soft_timer`, driving:
- Timeout tasks (one-time)
- Cycle tasks
- Multiple tasks

Forming a "time-driven + main process" combined execution model.

### Lifecycle Mechanism
Tasks are split into three stages through `SL_INIT`/`SL_FREE`/`SL_RUN`:
- **SL_INIT**: Executes once on first entry (initialize resources, start timers, etc.)
- **SL_RUN**: Main logic area (executed in a loop)
- **SL_FREE**: Executed during task switching (release resources)

Task switching is triggered by `sl_goto()`, essentially:
1. Mark `sl_free`
2. Execute `SL_FREE` in the next round
3. Load new task (`sl_load_new_task`)
4. Re-enter `SL_INIT`

### Synchronization Mechanism
Provides "pseudo-blocking" capability through `sl_wait`/`sl_wait_bare`:
- Current task enters waiting loop
- Internally continues to execute parallel tasks
- Wake up through `sl_wait_break`/`continue`

Characteristic: Writing style is blocking, but the system is not blocked.

## Flow Mechanism

### Essence
Flow is a macro-based collaborative workflow framework that unifies task scheduling, state machines, and synchronization primitives under the same semantic system. Each Flow is a "suspendable execution unit" that uses switch-case + static local variables to save execution context, and generates unique states using `__LINE__` to achieve coroutine-like breakpoint continuation.

### Core Concepts
- **Flow State**: Each Flow has a state variable for saving execution position
- **Flow Event**: Used for communication between Flows
- **Flow Context**: Each Flow can define its own static variables as workflow context

### Lifecycle
- **FLOW_INIT**: Executes only once, used to initialize context
- **FLOW_RUN**: Main running phase, where logic unfolds as "linear code"
- **FLOW_FREE**: Exit phase, used for resource release

Lifecycle switching is controlled externally via `FLOW_START`/`FLOW_STOP`, and can also be actively ended internally using `FLOW_EXIT`.

### Core Capability
**Non-blocking waiting**:
- **FLOW_UNTIL**: Yields scheduling when conditions are not met, continues from breakpoint when met
- **FLOW_WAIT**: Time-based non-blocking wait
- **FLOW_WAIT_EVENT**: Event-based non-blocking wait

### Key Advantage
**Linear synchronous writing**: Traditional state machines need to be split into multiple states + jumps, while here complex processes can be expressed with "sequential code", logic is closer to human thinking paths, significantly reducing state explosion and readability costs.

### Flow Paradigm

Flow workflow is written using a standard paradigm, including context definition, initialization, release, and running logic:

```c
void flow2(void)
{
    /* Workflow context, workflow data is statically defined here */
    SL_FLOW_CONTEXT(flow2);

    /* First entry into workflow, execute once, initialize workflow context */
    SL_FLOW_INIT;

    /* Workflow ends, no longer execute, release resources */
    SL_FLOW_FREE(flow2);

    /* Below starts entering workflow running logic */
    SL_FLOW_RUN;

    FLOW_UNTIL(var > 3);

    FLOW_WAIT_EVENT(evt1);

    FLOW_WAIT(2000);

    SL_FLOW_END;
}
```

### Flow Macro Definitions

The Flow framework provides the following core macros:

| Macro | Description |
|-----|-----|
| `FLOW_STATE_DEFINE(flow_name)` | Define Flow state variable |
| `FLOW_EVENT_DEFINE(event_name)` | Define Flow event variable |
| `FLOW_START(flow_name)` | Start Flow |
| `FLOW_STOP(flow_name)` | Stop Flow |
| `SL_FLOW_CONTEXT(flow_name)` | Define Flow context |
| `SL_FLOW_INIT` | Mark initialization code |
| `SL_FLOW_FREE(flow_name)` | Mark release code |
| `SL_FLOW_RUN` | Mark running logic |
| `SL_FLOW_END` | Mark Flow end |
| `FLOW_UNTIL(condition)` | Wait until condition is met |
| `FLOW_WAIT(ms)` | Non-blocking wait for specified time |
| `FLOW_WAIT_EVENT(event_name)` | Wait for event trigger |
| `FLOW_SEND_EVENT(event_name)` | Send event |
| `FLOW_EXIT()` | Actively end Flow |

### Application Scenarios

Flow mechanism is particularly suitable for the following scenarios:

- **Complex Business Logic**: Complex business processes requiring multiple steps, condition judgments, and waiting
- **State Management**: Applications that need to maintain state, such as state machines, protocol processing, etc.
- **Event Driven**: Event-based asynchronous processing
- **Resource Management**: Scenarios that require initialization and release of resources at specific times

Through the Flow mechanism, developers can express complex asynchronous processes with linear code structures, improving code readability and maintainability.

## RTT Printing

### Features
- **Efficient Output**: Based on SEGGER RTT technology, no serial port resources occupied
- **Colorful Logs**: Supports different levels of colorful log output
- **Timestamp**: Each log automatically adds timestamp
- **Level Display**: Supports different levels of log output

### Usage
```c
// Print log (with timestamp)
sl_printf("Hello, sloopLite!");

// Print variable
sl_prt_var(counter);

// Print float
sl_prt_float(temperature);
```

### Terminal Effect

![RTT Terminal Effect](images/rtt_terminal.png)

Real-time log information output by the framework can be viewed through J-Link RTT Viewer, including system initialization, task execution, system status, etc.

## Technical Specifications

### Hardware Requirements
- **Microcontroller**: STM32G0 series
- **Memory**: At least 4KB RAM
- **Flash**: At least 32KB Flash

### Software Requirements
- **Development Environment**: Keil MDK-ARM 5.x+
- **Compiler**: ARM Compiler 6
- **STM32 HAL Library**: STM32G0xx HAL Driver

### Resource Consumption
```
Total RO Size (Code + RO Data):  18648 bytes (18.21kB)
Total RW Size (RW Data + ZI Data):  3840 bytes (3.75kB)
Total ROM Size (Code + RO Data + RW Data):  18660 bytes (18.22kB)
```

## Quick Start

### Environment Setup
1. Install Keil MDK-ARM 5.x+
2. Install STM32G0 series support package
3. Clone or download the sloopLite project
4. Open `project/MDK-ARM/project.uvprojx` using Keil MDK

### Project Compilation
1. Select the target device (STM32G030K8Tx)
2. Configure compilation options
3. Click the compile button (F7)
4. The generated firmware is located at `project/MDK-ARM/bin/project.hex`

### Project Structure

```
├── project/
│   ├── Core/             # Core system files
│   ├── Drivers/          # STM32 HAL and CMSIS libraries
│   ├── MDK-ARM/          # Keil project files
│   └── user/             # User application code
│       ├── app/          # Application layer
│       │   ├── config/   # Configuration files
│       │   └── tasks/    # Task implementations
│       └── sloop/        # Framework core
│           ├── RTT/      # SEGGER RTT library
│           └── kernel/   # Kernel implementation
├── LICENSE               # License file
└── README.md             # Project documentation
```

## Core API

### System Initialization
```c
// Initialize the sloopLite framework
void sloop_init(void);

// Run the sloopLite framework
void sloop(void);
```

### Task Management

#### Timeout Task
```c
// Start a timeout task
void sl_timeout_start(int ms, pfunc task);

// Stop a timeout task
void sl_timeout_stop(pfunc task);
```

#### Cycle Task
```c
// Start a cycle task
void sl_cycle_start(int ms, pfunc task);

// Stop a cycle task
void sl_cycle_stop(pfunc task);
```

#### Multiple Task
```c
// Start a multiple task
void sl_multiple_start(int num, int ms, pfunc task);

// Stop a multiple task
void sl_multiple_stop(pfunc task);
```

#### Parallel Task
```c
// Start a parallel task
void sl_task_start(pfunc task);

// Stop a parallel task
void sl_task_stop(pfunc task);
```

#### Once Task
```c
// Start a once task
void sl_task_once(pfunc task);
```

#### Mutex Task
```c
// Switch to the specified task
void sl_goto(pfunc task);
```

### Time Management
```c
// Get system timestamp
uint32_t sl_get_tick(void);

// Blocking delay
void sl_delay(int ms);

// Non-blocking wait
char sl_wait(int ms);

// Non-blocking bare wait
char sl_wait_bare(void);
```

## Configuration File

The main configuration file is located at `project/user/app/config/sl_config.h`, and you can adjust the following parameters as needed:

```c
// Timeout task limit
#define SL_TIMEOUT_LIMIT 16

// Cycle task limit
#define SL_CYCLE_LIMIT 16

// Multiple task limit
#define SL_MULTIPLE_LIMIT 16

// Parallel task limit
#define SL_PARALLEL_LIMIT 32

// Once task limit
#define SL_ONCE_LIMIT 16

// Enable RTT print
#define SL_RTT_ENABLE 1
```

## Notes

1. **Task Design**: Mutex tasks need to use `SL_INIT`, `SL_FREE`, and `SL_RUN` macros to manage task lifecycle
2. **Resource Limits**: Each task type has a predefined quantity limit, exceeding the limit will cause task creation to fail
3. **Real-time Performance**: Due to the adoption of cooperative scheduling, tasks need to actively yield CPU resources
4. **Interrupt Handling**: Complex logic should be avoided in interrupts; it is recommended to use `sl_task_once()` to offload complex logic
5. **Memory Management**: The framework does not provide dynamic memory management; users need to manage memory themselves

## Porting Guide

### Basic Porting Steps

1. **System Clock Configuration**: Ensure the system clock is correctly configured with a 1ms systick interrupt period

2. **Systick Interrupt Handling**: Add `sl_tick_irq();` call in the systick interrupt handler, example:
   ```c
   void SysTick_Handler(void)
   {
       HAL_IncTick();
       sl_tick_irq(); // Call sloopLite clock update function
   }
   ```

3. **Framework Initialization**: Initialize the sloopLite framework and start the main loop in the main function:
   ```c
   int main(void)
   {
       // System initialization code
       
       sloop_init(); // Initialize sloopLite framework
       
       while (1)
       {
           sloop(); // Run sloopLite main loop
       }
   }
   ```

### Porting-Friendly Core Advantages
1. **Minimal Dependencies**: sloopLite has almost no external dependencies, only requiring a 1ms precision system clock, which can be easily implemented on most microcontrollers.
2. **Unified Interface**: Regardless of the target device, the porting steps are basically the same:
   
   - Configure system clock
   - Add `sl_tick_irq()` call in interrupt handler
   - Initialize framework and start main loop
3. **Flexible Configuration**: Through the `sl_config.h` file, parameters such as task quantity can be adjusted according to the resource situation of the target device to adapt to different hardware capabilities.
4. **Modular Design**: The framework core is isolated from hardware-related parts, so porting mainly focuses on clock and interrupt handling, and other parts do not need to be modified.
5. **Optional RTT Logging**: For devices that do not support J-Link, the RTT logging function can be disabled through simple macro definition modification without affecting core functionality.

### Actual Porting Experience
From practical operation, porting sloopLite to a new MCU usually only requires:

- A few lines of code modification (mainly the interrupt handler function)
- Simple configuration adjustments
- No need to modify the framework core code

This design concept is very suitable for embedded development needs, especially in resource-constrained environments. A simple, reliable, and easy-to-port framework can greatly reduce development and maintenance costs.

The porting friendliness of sloopLite also reflects its lightweight design goal. By minimizing hardware dependencies and simplifying the porting process, developers can focus more on implementing application logic rather than framework adaptation issues.

### Notes

1. **RTT Log Support**: RTT log functionality is based on SEGGER RTT technology and only supports devices supported by J-Link, such as ARM Cortex-M0, M3, M4, M7 series microcontrollers

2. **Clock Accuracy**: sloopLite relies on a 1ms accuracy system clock, ensure systick is configured correctly

3. **Task Quantity**: According to the resource situation of the target device, reasonably adjust the task quantity limits in `sl_config.h`

4. **Interrupt Priority**: Ensure the systick interrupt priority is set appropriately to avoid affecting system real-time performance

### Supported Devices

sloopLite framework is suitable for the following types of devices:

- **ARM Cortex-M series**: M0, M0+, M3, M4, M7, etc.
- **Other C language supported microcontrollers**: As long as they can provide a 1ms accuracy system clock, they can be ported

For devices that do not support J-Link, the RTT log function can be disabled by modifying the `SL_RTT_ENABLE` macro in `sl_config.h` to 0.

## License

This project adopts the MIT license, please refer to the LICENSE file for details.

## Contribution

Welcome to submit Issues and Pull Requests to improve the sloopLite framework.

---

sloopLite - Lightweight Embedded Task Scheduling Framework