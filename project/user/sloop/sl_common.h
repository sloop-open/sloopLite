/**
 ******************************************************************************
 * @file    bl_common
 * @author  sloop
 * @date    2024-12-24
 * @brief   杂项包含
 * ==此文件用户不应变更==
 *****************************************************************************/

#ifndef __bl_common_H
#define __bl_common_H

#include "stm32g0xx_hal.h"
#include "SEGGER_RTT.h"
#include "string.h"
#include "math.h"
#include "stdio.h"
#include "sl_config.h"

#ifndef __weak
#define __weak __attribute__((weak))
#endif /* __weak */
#ifndef __packed
#define __packed __attribute__((packed))
#endif /* __packed */

/* 状态机枚举 */
enum
{
    step1,
    step2,
    step3,
    step4,
    step5,
    step6,
    step7,
    step8
};

/* 中断优先级 */
#define PRIO_HIGHEST 0
#define PRIO_LOWEST 15
#define PRIO_DEFAULT 5

/* 简化函数指针定义 */
typedef void (*pfunc)(void);

/* 弱定义func */
#define weak_define(func) \
    __weak void func(void) {}

/* 循环自增，举例：add(i, 6) i = [0,6] 循环自增 */
#define add(i, upper) i = (i == upper) ? 0 : i + 1
/* 循环自减，举例：sub(i, 6) i = [6,0] 循环自减 */
#define sub(i, upper) i = (i == 0) ? upper : i - 1

/* ============================================================== */
/* 打印 API */

uint32_t sl_get_tick(void);

#if !SL_RTT_ENABLE

void print_null(const char *sFormat, ...);

#define SEGGER_RTT_printf(sFormat, ...) print_null(sFormat, ##__VA_ARGS__)

#endif

/* 系统打印（带时间戳），RTT 简化版本 */
#define sl_printf(sFormat, ...) SEGGER_RTT_printf(0, RTT_CTRL_TEXT_GREEN "\n[%02d %02d:%02d:%02d.%03d] " RTT_CTRL_TEXT_YELLOW sFormat "\n" RTT_CTRL_RESET, \
                                                  sl_get_tick() / 1000 / 60 / 60 / 24,                                                                     \
                                                  sl_get_tick() / 1000 / 60 / 60 % 24,                                                                     \
                                                  sl_get_tick() / 1000 / 60 % 60,                                                                          \
                                                  sl_get_tick() / 1000 % 60,                                                                               \
                                                  sl_get_tick() % 1000,                                                                                    \
                                                  ##__VA_ARGS__)

/* 带函数名的打印 */
#define sl_prt_withFunc(sFormat, ...) sl_printf(sFormat RTT_CTRL_TEXT_GREEN " <func: %s>" RTT_CTRL_RESET, ##__VA_ARGS__, __func__)

/* 错误日志 */
#define sl_error(sFormat, ...) sl_prt_withFunc(RTT_CTRL_TEXT_BRIGHT_RED "[error] " sFormat RTT_CTRL_RESET, ##__VA_ARGS__)

/* 白色高亮日志，带 === ==== */
#define sl_focus(sFormat, ...) sl_printf(RTT_CTRL_TEXT_BRIGHT_WHITE "=== " sFormat " ===" RTT_CTRL_RESET, ##__VA_ARGS__)

/* 白色高亮日志 */
#define sl_prt_brWhite(sFormat, ...) sl_printf(RTT_CTRL_TEXT_BRIGHT_WHITE sFormat RTT_CTRL_RESET, ##__VA_ARGS__)

/* 黄色高亮日志 */
#define sl_prt_brYellow(sFormat, ...) sl_printf(RTT_CTRL_TEXT_BRIGHT_YELLOW sFormat RTT_CTRL_RESET, ##__VA_ARGS__)

/* 打印变量 */
#define sl_prt_var(var) sl_prt_withFunc(#var " = %d", (int)var)

/* 打印hex */
#define sl_prt_hex(hex) sl_prt_withFunc(#hex " = 0x%x", (int)hex)

/* 打印字符串 */
#define sl_prt_str(str) sl_prt_withFunc("%s", str)

/* 打印浮点数 */
#define sl_prt_float(var) sl_prt_withFunc(#var " = %d.%02d", (int)var, abs((int)(var * 100) % 100))

/* 连续打印（末尾不带换行），用于不换行连续输出 */
#define sl_prt_noNewLine(sFormat, ...) SEGGER_RTT_printf(0, RTT_CTRL_TEXT_YELLOW sFormat RTT_CTRL_RESET, ##__VA_ARGS__, __func__)

/* ============================================================== */
/* 互斥任务相关服务 */

extern char _init;
extern char _free;

/* 加载新任务 */
void sl_load_new_task(void);

/* 任务初始化宏 */
#define _INIT                           \
    if (_init == 1)                     \
    {                                   \
        sl_focus("enter %s", __func__); \
        _init = 0;

/* 任务释放宏 */
#define _FREE       \
    }               \
    if (_free == 1) \
    {

#define _RUN                       \
    sl_focus("exit %s", __func__); \
    sl_load_new_task();            \
    _init = 1;                     \
    _free = 0;                     \
    return;                        \
    }

/* ============================================================== */
/* 异步编程相关服务 */

enum
{
    ASYNC_INIT,
    ASYNC_FREE,
    ASYNC_RUN,
};

/* 异步状态定义 */
#define ASYNC_STATE_DEFINE(name) uint32_t async_state_##name
/* 异步状态对外声明 */
#define ASYNC_STATE_DECLARE(name) extern uint32_t async_state_##name

/* 异步任务开始 */
#define ASYNC_TASK_START(task)           \
    do                                   \
    {                                    \
        async_state_##task = ASYNC_INIT; \
        sl_task_start(task);             \
    } while (0)

/* 异步任务停止 */
#define ASYNC_TASK_STOP(task) async_state_##task = ASYNC_FREE

/* 静态变量定义 */
#define _ASYNC_STATIC_VAR(name)   \
    static uint32_t _tick_start;  \
    static uint32_t _async_state; \
    _async_state = async_state_##name;

/* 业务初始化 */
#define _ASYNC_INIT       \
    switch (_async_state) \
    {                     \
    case ASYNC_INIT:      \
    {

/* 业务清理 */
#define _ASYNC_FREE(name)     \
    _async_state = ASYNC_RUN; \
    break;                    \
    }                         \
    case ASYNC_FREE:          \
    {                         \
        sl_task_stop(name);

/* 业务执行 */
#define _ASYNC_RUN             \
    _async_state = ASYNC_INIT; \
    break;                     \
    }                          \
    case ASYNC_RUN:            \
    {

/* 业务结束 */
#define _ASYNC_END(name)      \
    _async_state = ASYNC_RUN; \
    break;                    \
    }                         \
    }                         \
    async_state_##name = _async_state;

/* 异步wait */
#define ASYNC_WAIT(ms)                                    \
    _tick_start = sl_get_tick();                          \
    _async_state = __LINE__;                              \
    case __LINE__:                                        \
        if ((uint32_t)(sl_get_tick() - _tick_start) < ms) \
            break;

/* 异步wait条件 */
#define ASYNC_WAIT_UNTIL(cond) \
    _async_state = __LINE__;   \
    case __LINE__:             \
        if (!(cond))           \
            break;

/* 异步事件定义 */
#define ASYNC_EVENT_DEFINE(name) char async_event_##name
/* 异步事件对外声明 */
#define ASYNC_EVENT_DECLARE(name) extern char async_event_##name

/* 发送事件 */
#define ASYNC_SEND_EVENT(event) async_event_##event = 1;

/* 异步wait事件 */
#define ASYNC_WAIT_EVENT(event)     \
    _async_state = __LINE__;        \
    case __LINE__:                  \
        if (!(async_event_##event)) \
            break;                  \
        async_event_##event = 0;

/* 异步任务内部停止 */
#define ASYNC_STOP()           \
    _async_state = ASYNC_FREE; \
    break;

#endif /* __bl_common_H */

/*********************************** END OF FILE ***********************************/
