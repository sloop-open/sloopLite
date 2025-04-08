/**
 ******************************************************************************
 * @file    sloop
 * @author  С��
 * @date    2024-12-16
 * @brief   �ṩ sloop ϵͳAPI,���糬ʱ/��������Ĵ������������񴴽������������л���
 * ==���ļ��û���Ӧ���==
 *****************************************************************************/

#ifndef __sloop_H
#define __sloop_H

#include "sl_common.h"

/* sloop ϵͳ��ʼ�� */
void sloop_init(void);
/* sloop ϵͳ���� */
void sloop(void);

/* MCU tick �ж� */
void mcu_tick_irq(void);

/* ��ȡʱ��� */
uint32_t sys_get_tick(void);
/* ����ʽ��ʱ */
void sys_delay(int ms);

/* ��ʱ���� */
void sys_timeout_start(int ms, pfunc task);
void sys_timeout_stop(pfunc task);

/* �������� */
void sys_cycle_start(int ms, pfunc task);
void sys_cycle_stop(pfunc task);

/* ������� */
void sys_multiple_start(int num, int ms, pfunc task);
void sys_multiple_stop(pfunc task);

/* �������� */
void sys_task_start(pfunc task);
void sys_task_stop(pfunc task);

/* ��������ִֻ��һ�Σ����͵� main loop ��ִ�С��������ж��и����߼��·�ִ�� */
void sys_task_once(pfunc task);

/* ���������л� */
void sys_goto(pfunc task);

/* �������ȴ���ʱ���ֻ���ڻ���������ʹ�ã�0���ȴ����, 1���ȴ����ж� */
char sys_wait(int ms);
/* ��������ȴ���ֱ�� break or continue */
char sys_wait_bare(void);

/* �жϵȴ���sys_wait ����1 */
void sys_wait_break(void);
/* ���Եȴ���sys_wait ����0�������ִ�еȴ���Ĳ��� */
void sys_wait_continue(void);
/* ��ȡ�ȴ�״̬ */
char sys_is_waiting(void);

/* һ����ڿ�����ת��
��û�п�ʼ��Ҳû�н�����
ֻ��ĳ���������˲�䣬����ÿһ�д��룬
���������̳��������أ�
������Ŀ�꣬���ǽ�֣�
ֻ�Ǵ��ڣ����Ǵ��ڵ����ɣ�
���Զ������������ */

#define SLOOP_LAST_API "΢���������������"

#endif /* __sloop_H */

/************************** END OF FILE **************************/
