/**
 ******************************************************************************
 * @file    task_fsm
 * @author  sloop
 * @date    2026-3-22
 * @brief   有限状态机demo
 *****************************************************************************/

#include "common.h"

void task_fsm(void)
{
    _INIT; /* 初次进入任务时，执行一次 */

    _FREE; /* 任务结束，不再执行时，释放资源 */

    _RUN; /* 下方开始进入任务运行逻辑 */

    _FSM_START

    _CASE_START(0)
    sl_focus("start");
    sl_printf("case 0");

    sl_printf("wait 100ms");
    fsm_wait(100);
    fsm_goto(1);
    _CASE_END

    _CASE_START(1)
    sl_printf("case 1");

    sl_printf("wait 100ms");
    fsm_wait(100);
    fsm_goto(2);
    _CASE_END

    _CASE_START(2)
    sl_printf("case 2");

    sl_printf("wait 300ms");
    fsm_wait(300);
    sl_printf("wait 500ms");
    fsm_wait(500);
    sl_printf("wait 1000ms");
    fsm_wait(1000);
    fsm_goto(0);
    _CASE_END

    _DEFAULT_START
    _DEFAULT_END

    _FSM_END
}

/************************** END OF FILE **************************/
