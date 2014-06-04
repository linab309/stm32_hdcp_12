#include "ucos-ii.h"
/*******************************************************************************
* Function Name  : SysTickHandler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTick_Handler(void)
{
   OS_CPU_SR  cpu_sr;


    OS_ENTER_CRITICAL();  //����ȫ���жϱ�־,�����ж�/* Tell uC/OS-II that we are starting an ISR*/
    OSIntNesting++;
    OS_EXIT_CRITICAL();	  //�ָ�ȫ���жϱ�־

    OSTimeTick();     /* Call uC/OS-II's OSTimeTick(),��os_core.c�ļ��ﶨ��,��Ҫ�ж���ʱ�������Ƿ��ʱ��*/

    OSIntExit();  //��os_core.c�ļ��ﶨ��,����и������ȼ������������,��ִ��һ�������л�            
}

