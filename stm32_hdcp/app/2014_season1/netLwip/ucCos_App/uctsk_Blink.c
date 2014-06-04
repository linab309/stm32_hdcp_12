/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               uctsk_Blink.c
** Descriptions:            The uctsk_Blink application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-9
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             
** Modified date:           
** Version:                 
** Descriptions:            
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <includes.h>            
#include "uCOS-II/uc-cpu/cpu.h"
#include <Utilities.h>
//#include "GLCD.h"

/* Private variables ---------------------------------------------------------*/
static  OS_STK         App_TaskBlinkStk[APP_TASK_BLINK_STK_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void uctsk_Blink            (void);


void  App_BlinkTaskCreate (void)
{
    CPU_INT08U  os_err;

	os_err = os_err; /* prevent warning... */

	os_err = OSTaskCreate((void (*)(void *)) uctsk_Blink,				
                          (void          * ) 0,							
                          (OS_STK        * )&App_TaskBlinkStk[APP_TASK_BLINK_STK_SIZE - 1],		
                          (INT8U           ) APP_TASK_BLINK_PRIO  );							

	#if OS_TASK_NAME_EN > 0
    	OSTaskNameSet(APP_TASK_BLINK_PRIO, "Task LED Blink", &os_err);
	#endif

}

static void uctsk_Blink (void) 
{   
	//LCD_Initializtion();
    LED_Configuration();
 	//LCD_Clear(Black);        
   	for(;;)
   	{   
	    /*====LED-ON=======*/
		LED_Set(LED3,1);
		OSTimeDlyHMSM(0, 0, 0, 500);	 /* 200 MS  */ 
		/*====LED-OFF=======*/
		LED_Set(LED3,0);;
		OSTimeDlyHMSM(0, 0, 0, 500);	 /* 200 MS  */

		//Display_IPAddress();
   }
}


/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
