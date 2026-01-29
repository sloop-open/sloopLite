/**
 ******************************************************************************
 * @file    sloop
 * @author  sloop
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
uint32_t sl_get_tick(void);
/* 阻塞式延时 */
void sl_delay(int ms);

/* 超时任务 */
void sl_timeout_start(int ms, pfunc task);
void sl_timeout_stop(pfunc task);

/* 周期任务 */
void sl_cycle_start(int ms, pfunc task);
void sl_cycle_stop(pfunc task);

/* 多次任务 */
void sl_multiple_start(int num, int ms, pfunc task);
void sl_multiple_stop(pfunc task);

/* 并行任务 */
void sl_task_start(pfunc task);
void sl_task_stop(pfunc task);

/* 单次任务，只执行一次，迟滞到 main loop 里执行。可用于中断中复杂逻辑下放执行 */
void sl_task_once(pfunc task);

/* 互斥任务切换 */
void sl_goto(pfunc task);

/* 非阻塞等待延时到达（只能在互斥任务中使用）0：等待完成, 1：等待被中断 */
char sl_wait(int ms);
/* 非阻塞裸等待，直到 break or continue */
char sl_wait_bare(void);

/* 中断等待，sl_wait 返回1 */
void sl_wait_break(void);
/* 忽略等待，sl_wait 返回0，会继续执行等待后的操作 */
void sl_wait_continue(void);
/* 获取等待状态 */
char sl_is_waiting(void);

#endif /* __sloop_H */

/************************** END OF FILE **************************/
