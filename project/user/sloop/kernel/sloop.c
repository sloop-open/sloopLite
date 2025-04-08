/**
 ******************************************************************************
 * @file    sloop
 * @author  小柔
 * @date    2024-12-16
 * @brief   提供 sloop 系统API,比如超时/周期任务的创建、并行任务创建、互斥任务切换等
 *
 * ==此文件用户不应变更==
 *****************************************************************************/

#include "sloop.h"

#define check_task_not_null()          \
    if (task == NULL)                  \
    {                                  \
        sys_error("The task is null"); \
        return;                        \
    }

#define backup_reg(name1, name2)                  \
    static name1##_typ backup_reg[name2##_LIMIT]; \
    memcpy(backup_reg, name1##_reg, sizeof backup_reg);

void print_null(const char *sFormat, ...) {}

/* 超时任务运行 */
void timeout_run(void);
/* 周期任务运行 */
void cycle_run(void);
/* 多次任务运行 */
void multiple_run(void);
/* 并行任务运行 */
void parallel_task_run(void);
/* 单次任务运行 */
void once_task_run(void);
/* 互斥任务运行 */
void mutex_task_run(void);
/* 软件定定时器 */
void soft_timer(void);

/* ============================================================== */

/* 系统心跳 */
void system_heartbeat(void);
/* 计数循环次数 */
void loop_counter(void);
/* 计算 cpu 负载 */
void calcul_cpu_load(void);
/* 负载警告 */
void load_warning(void);

static volatile uint32_t tick;

static int loop;

static int load;

static int loop_us;

/* ============================================================== */

/* sloop 系统初始化 */
void sloop_init(void)
{
    sys_prt_brYellow("==================================");
    sys_prt_brYellow("========= sloop  (^-^) ==========");
    sys_prt_brYellow("==================================");

    /* 启用单次任务 */
    sys_task_start(once_task_run);

    /* 启动 loop 计数 */
    sys_task_start(loop_counter);

    /* 启动 cpu 负载计算 */
    sys_cycle_start(100, calcul_cpu_load);

    /* 启用系统心跳 */
    sys_cycle_start(1000, system_heartbeat);

    sys_prt_withFunc("system heartbeat start");
}

/* sloop 系统运行 */
void sloop(void)
{
    /* 互斥任务运行 */
    mutex_task_run();

    /* 并行任务运行 */
    parallel_task_run();
}

/* ============================================================== */

/* mcu tick 中断 */
void mcu_tick_irq(void)
{
    tick++;

    /* 软件定时器 1ms 启动一次 */
    sys_task_once(soft_timer);
}

/* 软件定定时器 */
void soft_timer(void)
{
    /* 超时任务运行 */
    timeout_run();

    /* 周期任务运行 */
    cycle_run();

    /* 多次任务运行 */
    multiple_run();
}

/* ============================================================== */

/* 系统心跳 */
void system_heartbeat(void)
{
    static int count;

    SEGGER_RTT_SetTerminal(1);

    sys_prt_var(count);

    SEGGER_RTT_SetTerminal(0);

    count++;
}

/* ============================================================== */

/* CPU 负载基数，即 CPU = 100% 时，一个 loop 的时间 单位0.1us，约定当loop 达到100us时，CPU 占用为100% */
#define LOOP_BASE 1000

/* 计数循环次数 */
void loop_counter(void)
{
    loop++;
}

/* 计算 cpu 负载 */
void calcul_cpu_load(void)
{
    if (loop == 0)
        return;

    static char warning;

    loop_us = 1000000 / loop;

    load = loop_us * 1000 / LOOP_BASE;

    loop = 0;

    /* 负载超过 80% ,日志警告 */
    if (warning == 0)
    {
        if (load > 800)
        {
            sys_cycle_start(1000, load_warning);

            warning = 1;
        }

        return;
    }

    /* 解除警告 */
    if (load < 600)
    {
        sys_cycle_stop(load_warning);

        warning = 0;
    }
}

/* 负载警告 */
void load_warning(void)
{
    sys_error("cpu load over 80%%, reach %2d.%d%%, average loop time: %d.%d us", load / 10, load % 10, loop_us / 10, loop_us % 10);
}

/* ============================================================== */

/* 获取时间戳 */
uint32_t sys_get_tick(void)
{
    return tick;
}

/* 阻塞式延时 */
void sys_delay(int ms)
{
    uint32_t tick_start = tick;

    /* 传入1ms，实际延时至少1ms */
    ms == 1 ? ms++ : ms;

    while (1)
    {
        if ((uint32_t)(tick - tick_start) >= ms)
        {
            return;
        }
    }
}

/* ============================================================== */

/* 超时任务数据 */
typedef struct
{
    uint32_t tick_start;

    int delay_ms;

    pfunc callback;

} timeout_typ;

/* 超时任务注册表 */
static timeout_typ timeout_reg[TIMEOUT_LIMIT];

/* 超时任务运行 */
void timeout_run(void)
{
    uint32_t tick_start;
    int delay_ms;

    /* 防止回调中改写注册表 */
    backup_reg(timeout, TIMEOUT);

    for (int i = 0; i < TIMEOUT_LIMIT; i++)
    {
        if (backup_reg[i].callback == NULL)
            continue;

        tick_start = backup_reg[i].tick_start;

        delay_ms = backup_reg[i].delay_ms;

        if ((uint32_t)(tick - tick_start) >= delay_ms)
        {
            /* 超时任务完成，释放资源 */
            timeout_reg[i].callback = NULL;

            backup_reg[i].callback();
        }
    }
}

/* 超时任务 */
void sys_timeout_start(int ms, pfunc task)
{
    check_task_not_null();

    /* 传入1ms，实际延时至少1ms */
    ms == 1 ? ms++ : ms;

    for (int i = 0; i < TIMEOUT_LIMIT; i++)
    {
        if (timeout_reg[i].callback == task)
        {
            /* 已注册，更新时间戳 */
            timeout_reg[i].tick_start = tick;

            return;
        }
    }

    /* 如未注册，这里先注册 */
    for (int i = 0; i < TIMEOUT_LIMIT; i++)
    {
        if (timeout_reg[i].callback == NULL)
        {
            timeout_reg[i].tick_start = tick;

            timeout_reg[i].delay_ms = ms;

            timeout_reg[i].callback = task;

            return;
        }
    }

    sys_error("timeout task overflow, limit %2d", TIMEOUT_LIMIT);
}

void sys_timeout_stop(pfunc task)
{
    check_task_not_null();

    for (int i = 0; i < TIMEOUT_LIMIT; i++)
    {
        if (timeout_reg[i].callback == task)
        {
            /* 抹掉callback，释放资源 */
            timeout_reg[i].callback = NULL;

            return;
        }
    }
}

/* ============================================================== */

/* 周期任务数据 */
typedef struct
{
    uint32_t tick_start;

    int delay_ms;

    pfunc callback;

} cycle_typ;

/* 周期任务注册表 */
static cycle_typ cycle_reg[CYCLE_LIMIT];

/* 周期任务运行 */
void cycle_run(void)
{
    uint32_t tick_start;
    int delay_ms;

    /* 防止回调中改写注册表 */
    backup_reg(cycle, CYCLE);

    for (int i = 0; i < CYCLE_LIMIT; i++)
    {
        if (backup_reg[i].callback == NULL)
            continue;

        tick_start = backup_reg[i].tick_start;

        delay_ms = backup_reg[i].delay_ms;

        if ((uint32_t)(tick - tick_start) >= delay_ms)
        {
            /* 周期任务完成，更新时间戳，开启下一周期 */
            cycle_reg[i].tick_start = tick;

            backup_reg[i].callback();
        }
    }
}

/* 周期任务 */
void sys_cycle_start(int ms, pfunc task)
{
    check_task_not_null();

    for (int i = 0; i < CYCLE_LIMIT; i++)
    {
        if (cycle_reg[i].callback == task)
        {
            if (cycle_reg[i].delay_ms == ms)
                return;

            /* 参数有更新，在下面重新注册 */
            cycle_reg[i].callback = NULL;

            break;
        }
    }

    /* 如未注册，这里先注册 */
    for (int i = 0; i < CYCLE_LIMIT; i++)
    {
        if (cycle_reg[i].callback == NULL)
        {
            cycle_reg[i].tick_start = tick;

            cycle_reg[i].delay_ms = ms;

            cycle_reg[i].callback = task;

            /* 周期任务开始时，执行一次 */
            if (task != NULL)
                task();

            return;
        }
    }

    sys_error("cycle task overflow, limit %2d", CYCLE_LIMIT);
}

void sys_cycle_stop(pfunc task)
{
    check_task_not_null();

    for (int i = 0; i < CYCLE_LIMIT; i++)
    {
        if (cycle_reg[i].callback == task)
        {
            /* 抹掉callback，释放资源 */
            cycle_reg[i].callback = NULL;

            return;
        }
    }
}

/* ============================================================== */

/* 多次任务数据 */
typedef struct
{
    uint32_t tick_start;

    int delay_ms;

    int num;

    pfunc callback;

} multiple_typ;

/* 多次任务注册表 */
static multiple_typ multiple_reg[MULTIPLE_LIMIT];

/* 多次任务运行 */
void multiple_run(void)
{
    uint32_t tick_start;
    int delay_ms;

    /* 防止回调中改写注册表 */
    backup_reg(multiple, MULTIPLE);

    for (int i = 0; i < MULTIPLE_LIMIT; i++)
    {
        if (backup_reg[i].callback == NULL)
            continue;

        tick_start = backup_reg[i].tick_start;

        delay_ms = backup_reg[i].delay_ms;

        if ((uint32_t)(tick - tick_start) >= delay_ms)
        {
            /* 多次任务完成，更新时间戳，开启下一多次 */
            multiple_reg[i].tick_start = tick;

            backup_reg[i].callback();

            multiple_reg[i].num--;

            /* 运行次数到达，停止 */
            if (backup_reg[i].num == 0)
            {
                multiple_reg[i].callback = NULL;
            }
        }
    }
}

/* 多次任务 */
void sys_multiple_start(int num, int ms, pfunc task)
{
    check_task_not_null();

    if (num == 0)
        return;

    /* 执行一次 */
    if (num == 1)
    {
        if (task != NULL)
            task();

        return;
    }

    for (int i = 0; i < MULTIPLE_LIMIT; i++)
    {
        if (multiple_reg[i].callback == task)
        {
            if ((multiple_reg[i].num == num) && (multiple_reg[i].delay_ms == ms))
                return;

            /* 参数有更新，在下面重新注册 */
            multiple_reg[i].callback = NULL;

            break;
        }
    }

    /* 如未注册，这里先注册 */
    for (int i = 0; i < MULTIPLE_LIMIT; i++)
    {
        if (multiple_reg[i].callback == NULL)
        {
            multiple_reg[i].tick_start = tick;

            multiple_reg[i].delay_ms = ms;

            multiple_reg[i].num = num - 1;

            multiple_reg[i].callback = task;

            /* 多次任务开始时，执行一次 */
            if (task != NULL)
                task();

            return;
        }
    }

    sys_error("multiple task overflow, limit %2d", MULTIPLE_LIMIT);
}

void sys_multiple_stop(pfunc task)
{
    check_task_not_null();

    for (int i = 0; i < MULTIPLE_LIMIT; i++)
    {
        if (multiple_reg[i].callback == task)
        {
            /* 抹掉callback，释放资源 */
            multiple_reg[i].callback = NULL;

            return;
        }
    }
}

/* ============================================================== */

/* 并行任务注册表 */
static pfunc task_reg[PARALLEL_TASK_LIMIT];

/* 并行任务运行 */
void parallel_task_run(void)
{
    for (int i = 0; i < PARALLEL_TASK_LIMIT; i++)
    {
        if (task_reg[i] == NULL)
            continue;

        task_reg[i]();
    }
}

/* 并行任务 */
void sys_task_start(pfunc task)
{
    check_task_not_null();

    for (int i = 0; i < PARALLEL_TASK_LIMIT; i++)
    {
        if (task_reg[i] == task)
        {
            return;
        }
    }

    for (int i = 0; i < PARALLEL_TASK_LIMIT; i++)
    {
        if (task_reg[i] == NULL)
        {
            task_reg[i] = task;

            return;
        }
    }

    sys_error("parallel task overflow, limit %2d", PARALLEL_TASK_LIMIT);
}

void sys_task_stop(pfunc task)
{
    check_task_not_null();

    for (int i = 0; i < PARALLEL_TASK_LIMIT; i++)
    {
        if (task_reg[i] == task)
        {
            task_reg[i] = NULL;

            return;
        }
    }
}

/* ============================================================== */

/* 单次任务注册表 */
static pfunc once_task_reg[ONCE_TASK_LIMIT];

static int soft_timer_count;

/* 单次任务运行 */
void once_task_run(void)
{
    static pfunc backup_reg[ONCE_TASK_LIMIT];

    /* 防止回调中改写注册表 */
    memcpy(backup_reg, once_task_reg, sizeof backup_reg);

    for (int i = 0; i < ONCE_TASK_LIMIT; i++)
    {
        if (backup_reg[i] == NULL)
            continue;

        /* 运行一次后，取消注册 */
        once_task_reg[i] = NULL;

        backup_reg[i]();

        if (backup_reg[i] == soft_timer)
            soft_timer_count = 0;
    }
}

/* 单次任务 */
void sys_task_once(pfunc task)
{
    check_task_not_null();

    for (int i = 0; i < ONCE_TASK_LIMIT; i++)
    {
        if (once_task_reg[i] == task)
        {
            /* 软定时不报 */
            if (task == soft_timer)
            {
                soft_timer_count++;

                if (soft_timer_count <= 3)
                    return;
            }

            return;
        }
    }

    for (int i = 0; i < ONCE_TASK_LIMIT; i++)
    {
        if (once_task_reg[i] == NULL)
        {
            once_task_reg[i] = task;

            return;
        }
    }

    sys_error("once task overflow, limit %2d", ONCE_TASK_LIMIT);
}

/* ============================================================== */

static char wait;
char _init;
char _free;
static pfunc run_task;
static pfunc pre_task;

/* 互斥任务切换 */
void sys_goto(pfunc task)
{
    check_task_not_null();

    if (wait)
    {
        /* 切换任务，强制中断等待 */
        sys_wait_break();
    }

    /* 第一次未加载过任务，则特殊处理 */
    if (run_task == NULL)
    {
        run_task = task;

        _init = 1;

        return;
    }

    pre_task = task;

    _free = 1;
}

/* 加载新任务 */
void sys_load_new_task(void)
{
    run_task = pre_task;
}

/* 互斥任务运行 */
void mutex_task_run(void)
{
    if (run_task != NULL)
    {
        run_task();
    }
}

/* ============================================================== */

static volatile char break_wait;
static volatile char _continue;

/* 非阻塞等待延时到达（只能在互斥任务中使用） */
char sys_wait(int ms)
{
    if (wait)
    {
        sys_error("sys_wait cannot be called nested");

        return 1;
    }

    /* 复位打断标志 */
    break_wait = 0;
    _continue = 0;

    wait = 1;

    uint32_t tick_start = tick;

    /* 传入1ms，实际延时至少1ms */
    ms == 1 ? ms++ : ms;

    while (1)
    {
        /* 轮询除当前任务以外的并行任务 */
        parallel_task_run();

        /* 中断等待 */
        if (break_wait)
        {
            break_wait = 0;

            wait = 0;

            return 1;
        }

        /* 忽略等待 */
        if (_continue)
        {
            _continue = 0;

            wait = 0;

            return 0;
        }

        if ((uint32_t)(tick - tick_start) >= ms)
        {
            break;
        }
    }

    wait = 0;

    return 0;
}

/* 非阻塞裸等待，直到 break or continue */
char sys_wait_bare(void)
{
    if (wait)
    {
        sys_error("sys_wait_bare cannot be called nested");

        return 1;
    }

    /* 复位打断标志 */
    break_wait = 0;
    _continue = 0;

    wait = 1;

    char r = 0;

    while (1)
    {
        /* 轮询除当前任务以外的并行任务 */
        parallel_task_run();

        /* 中断等待 */
        if (break_wait)
        {
            break_wait = 0;

            r = 1;

            break;
        }

        /* 忽略等待 */
        if (_continue)
        {
            _continue = 0;

            r = 0;

            break;
        }
    }

    wait = 0;

    return r;
}

/* 获取等待状态 */
char sys_is_waiting(void)
{
    return wait;
}

/* 中断等待 */
void sys_wait_break(void)
{
    break_wait = 1;

    sys_printf("break wait");
}

/* 忽略等待，会继续执行等待后的操作 */
void sys_wait_continue(void)
{
    _continue = 1;

    sys_printf("ignore wait and continue");
}

/************************** END OF FILE **************************/
