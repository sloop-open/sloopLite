/**
 ******************************************************************************
 * @file    task_flow
 * @author  sloop
 * @date    2026-3-22
 * @brief   工作流编程demo
 *****************************************************************************/

#include "common.h"

FLOW_STATE_DEFINE(flow1);
FLOW_EVENT_DEFINE(flow1);

void flow1(void);

static char var;

void task_flow(void)
{
    _INIT; /* 初次进入任务时，执行一次 */

    FLOW_START(flow1);

    _FREE; /* 任务结束，不再执行时，释放资源 */

    FLOW_STOP(flow1);

    _RUN; /* 下方开始进入任务运行逻辑 */

    var++;

    sl_prt_withFunc("flow run, var = %d", var);

    sl_wait(1000);

    if (var == 10)
    {
        FLOW_SEND_EVENT(flow1);
    }
}

void flow1(void)
{
    _FLOW_CONTEXT(flow1); /* 工作流上下文，工作流需要的数据在此静态定义 */

    _FLOW_INIT; /* 初次进入工作流，执行一次，初始化工作流上下文 */
    sl_focus("flow1 start");

    _FLOW_FREE(flow1); /* 工作流结束，不再执行时，释放资源 */
    sl_focus("flow1 stop");

    _FLOW_RUN; /* 下方开始进入工作流运行逻辑 */

    FLOW_UNTIL(var > 5);

    sl_prt_withFunc("condition met");

    FLOW_WAIT_EVENT(flow1);

    sl_prt_withFunc("event met");

    FLOW_WAIT(3000);

    sl_prt_withFunc("wait 3s");

    /* 主动结束工作流 */
    FLOW_EXIT();

    _FLOW_END(flow1);
}

/************************** END OF FILE **************************/
