/**
 ******************************************************************************
 * @file    bl_common
 * @author  С��
 * @date    2024-12-24
 * @brief   �������
 * ==���ļ��û���Ӧ���==
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

/* ״̬��ö�� */
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

/* �ж����ȼ� */
#define PRIO_HIGHEST 0
#define PRIO_LOWEST 15
#define PRIO_DEFAULT 5

/* �򻯺���ָ�붨�� */
typedef void (*pfunc)(void);

/* ������func */
#define weak_define(func) \
    __weak void func(void) {}

/* ѭ��������������add(i, 6) i = [0,6] ѭ������ */
#define add(i, upper) i = (i == upper) ? 0 : i + 1
/* ѭ���Լ���������sub(i, 6) i = [6,0] ѭ���Լ� */
#define sub(i, upper) i = (i == 0) ? upper : i - 1

/* ============================================================== */
/* ��ӡ API */

uint32_t sys_get_tick(void);

#if !SYS_RTT_ENABLE

void print_null(const char *sFormat, ...);

#define SEGGER_RTT_printf(sFormat, ...) print_null(sFormat, ##__VA_ARGS__)

#endif

/* ϵͳ��ӡ����ʱ�������RTT �򻯰汾 */
#define sys_printf(sFormat, ...) SEGGER_RTT_printf(0, RTT_CTRL_TEXT_GREEN "\n[%02d %02d:%02d:%02d.%03d] " RTT_CTRL_TEXT_YELLOW sFormat "\n" RTT_CTRL_RESET, \
                                                   sys_get_tick() / 1000 / 60 / 60 / 24,                                                                    \
                                                   sys_get_tick() / 1000 / 60 / 60 % 24,                                                                    \
                                                   sys_get_tick() / 1000 / 60 % 60,                                                                         \
                                                   sys_get_tick() / 1000 % 60,                                                                              \
                                                   sys_get_tick() % 1000,                                                                                   \
                                                   ##__VA_ARGS__, __func__)

/* ���������Ĵ�ӡ */
#define sys_prt_withFunc(sFormat, ...) sys_printf(sFormat RTT_CTRL_TEXT_GREEN " <func: %s>" RTT_CTRL_RESET, ##__VA_ARGS__, __func__)

/* ������־ */
#define sys_error(sFormat, ...) sys_prt_withFunc(RTT_CTRL_TEXT_BRIGHT_RED "[error] " sFormat RTT_CTRL_RESET, ##__VA_ARGS__)

/* ��ɫ������־���� === ==== */
#define sys_focus(sFormat, ...) sys_printf(RTT_CTRL_TEXT_BRIGHT_WHITE "=== " sFormat " ===" RTT_CTRL_RESET, ##__VA_ARGS__)

/* ��ɫ������־ */
#define sys_prt_brWhite(sFormat, ...) sys_printf(RTT_CTRL_TEXT_BRIGHT_WHITE sFormat RTT_CTRL_RESET, ##__VA_ARGS__)

/* ��ɫ������־ */
#define sys_prt_brYellow(sFormat, ...) sys_printf(RTT_CTRL_TEXT_BRIGHT_YELLOW sFormat RTT_CTRL_RESET, ##__VA_ARGS__)

/* ��ӡ���� */
#define sys_prt_var(var) sys_prt_withFunc(#var " = %d", (int)var)

/* ��ӡ�ַ��� */
#define sys_prt_str(str) sys_prt_withFunc("%s", str)

/* ��ӡ������ */
#define sys_prt_float(var) sys_prt_withFunc(#var " = %d.%2d", (int)var, (int)(var * 1000) % 1000)

/* ������ӡ��ĩβ�������У������ڲ������������ */
#define sys_prt_noNewLine(sFormat, ...) SEGGER_RTT_printf(0, RTT_CTRL_TEXT_YELLOW sFormat RTT_CTRL_RESET, ##__VA_ARGS__, __func__)

/* ============================================================== */
/* ����������ط��� */

extern char _init;
extern char _free;

/* ���������� */
void sys_load_new_task(void);

/* �����ʼ���� */
#define _INIT                            \
    if (_init == 1)                      \
    {                                    \
        sys_focus("enter %s", __func__); \
        _init = 0;

/* �����ͷź� */
#define _FREE       \
    }               \
    if (_free == 1) \
    {

#define _RUN                        \
    sys_focus("exit %s", __func__); \
    sys_load_new_task();            \
    _init = 1;                      \
    _free = 0;                      \
    return;                         \
    }

#endif /* __bl_common_H */

/*********************************** END OF FILE ***********************************/
