/**
 ******************************************************************************
 * @file    task_async
 * @author  sloop
 * @date    2026-3-22
 * @brief   异步编程demo
 *****************************************************************************/

#include "common.h"

ASYNC_STATE_DEFINE(demo);
ASYNC_STATE_DECLARE(demo);

ASYNC_EVENT_DEFINE(demo);
ASYNC_EVENT_DECLARE(demo);

void demo(void);

static char var;

void task_async(void)
{
    _INIT; /* 初次进入任务时，执行一次 */

    ASYNC_TASK_START(demo);

    _FREE; /* 任务结束，不再执行时，释放资源 */

    ASYNC_TASK_STOP(demo);

    _RUN; /* 下方开始进入任务运行逻辑 */

    var++;

    sl_prt_withFunc("async run, var = %d", var);

    sl_wait(1000);

    if (var == 15)
    {
        ASYNC_SEND_EVENT(demo);
    }
}

void demo(void)
{
    /* 静态变量定义区 */
    _ASYNC_STATIC_VAR(demo);

    /* 业务初始化区 */
    _ASYNC_INIT;
    sl_focus("async start");

    /* 业务清理区 */
    _ASYNC_FREE(demo);

    /* 业务执行区 */
    _ASYNC_RUN;

    ASYNC_WAIT_UNTIL(var > 10);

    sl_prt_withFunc("condition met");

    ASYNC_WAIT_EVENT(demo);

    sl_prt_withFunc("event met");

    ASYNC_WAIT(5000);

    sl_prt_withFunc("wait 5s");

    ASYNC_STOP();

    _ASYNC_END(demo);
}

/************************** END OF FILE **************************/
