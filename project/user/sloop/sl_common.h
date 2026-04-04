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
/* Flow-based 协作式工作流编程 */

/* Flow 状态 */
enum
{
    FLOW_INIT,
    FLOW_FREE,
    FLOW_RUN,
};

/* Flow 状态定义 */
#define FLOW_STATE_DEFINE(name) uint32_t flow_state_##name
#define FLOW_STATE_DECLARE(name) extern uint32_t flow_state_##name

/* Flow 启动 */
#define FLOW_START(name)               \
    do                                 \
    {                                  \
        flow_state_##name = FLOW_INIT; \
        sl_task_start(name);           \
    } while (0)

/* Flow 停止（外部） */
#define FLOW_STOP(name) flow_state_##name = FLOW_FREE

/* Flow 内部上下文 */
#define _FLOW_CONTEXT(name)      \
    static uint32_t _flow_tick;  \
    static uint32_t _flow_state; \
    _flow_state = flow_state_##name;

/* 初始化区 */
#define _FLOW_INIT       \
    switch (_flow_state) \
    {                    \
    case FLOW_INIT:      \
    {

/* 清理区 */
#define _FLOW_FREE(name)    \
    _flow_state = FLOW_RUN; \
    break;                  \
    }                       \
    case FLOW_FREE:         \
    {                       \
        sl_task_stop(name);

/* 运行区 */
#define _FLOW_RUN            \
    _flow_state = FLOW_INIT; \
    break;                   \
    }                        \
    case FLOW_RUN:           \
    {

/* 结束 */
#define _FLOW_END(name)     \
    _flow_state = FLOW_RUN; \
    break;                  \
    }                       \
    }                       \
    flow_state_##name = _flow_state;

/* ===================== */
/*      FLOW 原语        */
/* ===================== */

/* 时间等待 */
#define FLOW_WAIT(ms)                                      \
    _flow_tick = sl_get_tick();                            \
    _flow_state = __LINE__;                                \
    case __LINE__:                                         \
        if ((uint32_t)(sl_get_tick() - _flow_tick) < (ms)) \
            break;

/* 条件等待（核心原语） */
#define FLOW_UNTIL(cond)    \
    _flow_state = __LINE__; \
    case __LINE__:          \
        if (!(cond))        \
            break;

/* 事件定义 */
#define FLOW_EVENT_DEFINE(name) char flow_event_##name
#define FLOW_EVENT_DECLARE(name) extern char flow_event_##name

/* 发送事件 */
#define FLOW_SEND_EVENT(event) flow_event_##event = 1

/* 等待事件（消费型） */
#define FLOW_WAIT_EVENT(event)     \
    _flow_state = __LINE__;        \
    case __LINE__:                 \
        if (!(flow_event_##event)) \
            break;                 \
        flow_event_##event = 0;

/* Flow 内部停止 */
#define FLOW_EXIT()          \
    _flow_state = FLOW_FREE; \
    break;

#endif /* __bl_common_H */

/*********************************** END OF FILE ***********************************/
