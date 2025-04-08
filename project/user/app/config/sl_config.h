/**
 ******************************************************************************
 * @file    bl_config
 * @author  小柔
 * @date    2024-12-18
 * @brief   在此处配置 sloop 和启用功能模块
 * ==此文件用户可配置==
 *****************************************************************************/

#ifndef __bl_config_H
#define __bl_config_H

/* 超时任务上限 */
#define TIMEOUT_LIMIT 16

/* 周期任务上限 */
#define CYCLE_LIMIT 16

/* 多次任务上限 */
#define MULTIPLE_LIMIT 16

/* 并行任务上限 */
#define PARALLEL_TASK_LIMIT 32

/* 单次任务上限 */
#define ONCE_TASK_LIMIT 16

/* ============================================================== */

/* 启用RTT打印 */
#define SYS_RTT_ENABLE 1

#endif /* __bl_config_H */

/************************** END OF FILE **************************/
