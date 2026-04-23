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
    SL_INIT;

    FLOW_START(flow_user);
    FLOW_START(flow_delivery);

    /* 任务结束、不再执行时，释放资源 */
    SL_FREE;

    FLOW_STOP(flow_user);
    FLOW_STOP(flow_delivery);

    /* 下方开始进入任务运行逻辑 */
    SL_RUN;
}

void flow_user(void)
{
    /* 工作流上下文，工作流需要的数据在此静态定义 */
    SL_FLOW_CONTEXT(flow_user);

    /* 初次进入工作流，执行一次，初始化工作流上下文 */
    SL_FLOW_INIT;
    sl_printf("user: Open food delivery APP");

    /* 工作流结束，不再执行时，释放资源 */
    SL_FLOW_FREE(flow_user);
    sl_printf("user: Exit");

    FLOW_STOP(flow_eat);
    FLOW_STOP(flow_watch);

    /* 下方开始进入工作流运行逻辑 */
    SL_FLOW_RUN;

    /* 下单 */
    sl_printf("user: Place an order");
    FLOW_SEND_EVENT(evt_order);

    FLOW_START(flow_eat);
    FLOW_START(flow_watch);

    /* 等待用餐完成 */
    FLOW_WAIT_EVENT(evt_eat);

    /* 结束示例 */
    sl_goto(task_idle);

    SL_FLOW_END;
}

void flow_watch(void)
{
    /* 工作流上下文，工作流需要的数据在此静态定义 */
    SL_FLOW_CONTEXT(flow_watch);

    /* 初次进入工作流，执行一次，初始化工作流上下文 */
    SL_FLOW_INIT;
    sl_printf("watch: Start");

    /* 工作流结束，不再执行时，释放资源 */
    SL_FLOW_FREE(flow_watch);
    sl_printf("watch: Stop");

    /* 下方开始进入工作流运行逻辑 */
    SL_FLOW_RUN;

    sl_printf("user: Watching drama...");
    FLOW_WAIT(1000);

    SL_FLOW_END;
}

void flow_delivery(void)
{
    /* 工作流上下文，工作流需要的数据在此静态定义 */
    SL_FLOW_CONTEXT(flow_delivery);

    /* 初次进入工作流，执行一次，初始化工作流上下文 */
    SL_FLOW_INIT;
    sl_printf("delivery: Start");

    /* 工作流结束，不再执行时，释放资源 */
    SL_FLOW_FREE(flow_delivery);
    sl_printf("delivery: Stop");

    /* 下方开始进入工作流运行逻辑 */
    SL_FLOW_RUN;

    FLOW_WAIT_EVENT(evt_order);

    sl_printf("delivery: Receive order");

    FLOW_WAIT(2000);
    sl_printf("delivery: Prepare meal");

    FLOW_WAIT(3000);
    sl_printf("delivery: Deliver food");

    FLOW_SEND_EVENT(evt_arrive);
    sl_printf("delivery: Delivered");

    SL_FLOW_END;
}

void flow_eat(void)
{
    /* 工作流上下文，工作流需要的数据在此静态定义 */
    SL_FLOW_CONTEXT(flow_eat);

    /* 初次进入工作流，执行一次，初始化工作流上下文 */
    SL_FLOW_INIT;
    sl_printf("eat: Preparing");

    /* 工作流结束，不再执行时，释放资源 */
    SL_FLOW_FREE(flow_eat);
    sl_printf("eat: Finish");

    /* 下方开始进入工作流运行逻辑 */
    SL_FLOW_RUN;

    /* 等待外卖送达 */
    FLOW_WAIT_EVENT(evt_arrive);

    sl_printf("eat: Takeaway arrived, start eating");

    FLOW_WAIT(2000);
    sl_printf("eat: Finished eating");

    FLOW_SEND_EVENT(evt_eat);

    FLOW_EXIT();

    SL_FLOW_END;
}

/************************** END OF FILE **************************/