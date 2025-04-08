/**
 ******************************************************************************
 * @file    task_demo
 * @author  С��
 * @date    2025-1-13
 * @brief   ��ʾ����
 *****************************************************************************/

#include "common.h"

void test(void);
void stop_test(void);

void task_demo(void)
{
    _INIT; /* ���ν�������ʱ��ִ��һ�� */

    /* ������������ */
    sys_cycle_start(1000, test);

    /* 3���������� */
    sys_timeout_start(3000, stop_test);

    _FREE; /* �������������ִ��ʱ���ͷ���Դ */

    /* �뿪�����Զ��ر� */
    sys_cycle_stop(test);

    _RUN; /* �·���ʼ�������������߼� */

    /* �ȴ����Խ��� */
    if (sys_wait_bare())
        return;

    /* ��ת���������� */
    sys_goto(task_idle);
}

void test(void)
{
    sys_printf("test non blocking");
}

void stop_test(void)
{
    sys_wait_continue();
}

/************************** END OF FILE **************************/
