/**
 ******************************************************************************
 * @file    task_flow
 * @author  sloop
 * @date    2026-3-22
 * @brief   工作流编程示例
 *****************************************************************************/

#include "common.h"

/* 下单事件 */
FLOW_EVENT_DEFINE(evt_order);
/* 外卖送达事件 */
FLOW_EVENT_DEFINE(evt_arrive);
/* 用餐完成事件 */
FLOW_EVENT_DEFINE(evt_eat);

FLOW_STATE_DEFINE(flow_user);
FLOW_STATE_DEFINE(flow_delivery);
FLOW_STATE_DEFINE(flow_eat);
FLOW_STATE_DEFINE(flow_watch);

void flow_user(void);
void flow_delivery(void);
void flow_eat(void);
void flow_watch(void);

void task_flow(void)
{
    /* 首次进入任务时，执行一次 */
    _INIT;

    FLOW_START(flow_user);
    FLOW_START(flow_delivery);

    /* 任务结束、不再执行时，释放资源 */
    _FREE;

    FLOW_STOP(flow_user);
    FLOW_STOP(flow_delivery);

    /* 下方开始进入任务运行逻辑 */
    _RUN;
}

void flow_user(void)
{
    /* 工作流上下文，工作流需要的数据在此静态定义 */
    _FLOW_CONTEXT(flow_user);

    /* 初次进入工作流，执行一次，初始化工作流上下文 */
    _FLOW_INIT;
    sl_prt_withFunc("user: Open food delivery APP");

    /* 工作流结束，不再执行时，释放资源 */
    _FLOW_FREE(flow_user);
    sl_prt_withFunc("user: Exit");

    FLOW_STOP(flow_eat);
    FLOW_STOP(flow_watch);

    /* 下方开始进入工作流运行逻辑 */
    _FLOW_RUN;

    /* 下单 */
    sl_prt_withFunc("user: Place an order");
    FLOW_SEND_EVENT(evt_order);

    FLOW_START(flow_eat);
    FLOW_START(flow_watch);

    /* 等待用餐完成 */
    FLOW_WAIT_EVENT(evt_eat);

    /* 结束示例 */
    sl_goto(task_idle);

    _FLOW_END;
}

void flow_watch(void)
{
    /* 工作流上下文，工作流需要的数据在此静态定义 */
    _FLOW_CONTEXT(flow_watch);

    /* 初次进入工作流，执行一次，初始化工作流上下文 */
    _FLOW_INIT;
    sl_prt_withFunc("watch: Start");

    /* 工作流结束，不再执行时，释放资源 */
    _FLOW_FREE(flow_watch);
    sl_prt_withFunc("watch: Stop");

    /* 下方开始进入工作流运行逻辑 */
    _FLOW_RUN;

    sl_prt_withFunc("user: Watching drama...");
    FLOW_WAIT(1000);

    _FLOW_END;
}

void flow_delivery(void)
{
    /* 工作流上下文，工作流需要的数据在此静态定义 */
    _FLOW_CONTEXT(flow_delivery);

    /* 初次进入工作流，执行一次，初始化工作流上下文 */
    _FLOW_INIT;
    sl_prt_withFunc("delivery: Start");

    /* 工作流结束，不再执行时，释放资源 */
    _FLOW_FREE(flow_delivery);
    sl_prt_withFunc("delivery: Stop");

    /* 下方开始进入工作流运行逻辑 */
    _FLOW_RUN;

    FLOW_WAIT_EVENT(evt_order);

    sl_prt_withFunc("delivery: Receive order");

    FLOW_WAIT(2000);
    sl_prt_withFunc("delivery: Prepare meal");

    FLOW_WAIT(3000);
    sl_prt_withFunc("delivery: Deliver food");

    FLOW_SEND_EVENT(evt_arrive);
    sl_prt_withFunc("delivery: Delivered");

    _FLOW_END;
}

void flow_eat(void)
{
    /* 工作流上下文，工作流需要的数据在此静态定义 */
    _FLOW_CONTEXT(flow_eat);

    /* 初次进入工作流，执行一次，初始化工作流上下文 */
    _FLOW_INIT;
    sl_prt_withFunc("eat: Preparing");

    /* 工作流结束，不再执行时，释放资源 */
    _FLOW_FREE(flow_eat);
    sl_prt_withFunc("eat: Finish");

    /* 下方开始进入工作流运行逻辑 */
    _FLOW_RUN;

    /* 等待外卖送达 */
    FLOW_WAIT_EVENT(evt_arrive);

    sl_prt_withFunc("eat: Takeaway arrived, start eating");

    FLOW_WAIT(2000);
    sl_prt_withFunc("eat: Finished eating");

    FLOW_SEND_EVENT(evt_eat);

    FLOW_EXIT();

    _FLOW_END;
}

/************************** END OF FILE **************************/