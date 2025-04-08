/**
 ******************************************************************************
 * @file    sloop
 * @author  小柔
 * @date    2024-12-16
 * @brief   提供 sloop 系统API,比如超时/周期任务的创建、并行任务创建、互斥任务切换等
 * ==此文件用户不应变更==
 *****************************************************************************/

#ifndef __sloop_H
#define __sloop_H

#include "sl_common.h"

/* sloop 系统初始化 */
void sloop_init(void);
/* sloop 系统运行 */
void sloop(void);

/* MCU tick 中断 */
void mcu_tick_irq(void);

/* 获取时间戳 */
uint32_t sys_get_tick(void);
/* 阻塞式延时 */
void sys_delay(int ms);

/* 超时任务 */
void sys_timeout_start(int ms, pfunc task);
void sys_timeout_stop(pfunc task);

/* 周期任务 */
void sys_cycle_start(int ms, pfunc task);
void sys_cycle_stop(pfunc task);

/* 多次任务 */
void sys_multiple_start(int num, int ms, pfunc task);
void sys_multiple_stop(pfunc task);

/* 并行任务 */
void sys_task_start(pfunc task);
void sys_task_stop(pfunc task);

/* 单次任务，只执行一次，迟滞到 main loop 里执行。可用于中断中复杂逻辑下放执行 */
void sys_task_once(pfunc task);

/* 互斥任务切换 */
void sys_goto(pfunc task);

/* 非阻塞等待延时到达（只能在互斥任务中使用）0：等待完成, 1：等待被中断 */
char sys_wait(int ms);
/* 非阻塞裸等待，直到 break or continue */
char sys_wait_bare(void);

/* 中断等待，sys_wait 返回1 */
void sys_wait_break(void);
/* 忽略等待，sys_wait 返回0，会继续执行等待后的操作 */
void sys_wait_continue(void);
/* 获取等待状态 */
char sys_is_waiting(void);

/* 一阵风在空中旋转，
它没有开始，也没有结束，
只在某个不经意的瞬间，穿过每一行代码，
在无声处滋长，静静地，
它不是目标，不是结局，
只是存在，它是存在的理由，
不言而喻，恍若无物。 */

#define SLOOP_LAST_API "微风轻拂，温柔如昔"

#endif /* __sloop_H */

/************************** END OF FILE **************************/
