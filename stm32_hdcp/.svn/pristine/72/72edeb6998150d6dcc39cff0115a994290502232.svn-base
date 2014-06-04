/**
	******************************************************************************
	* @file    Project/STM32F10x_StdPeriph_Template/main.c 
	* @author  MCD Application Team
	* @version V3.3.0
	* @date    04/16/2010
	* @brief   Main program body
	******************************************************************************
	* @copy
	*
	* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
	* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
	* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
	* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
	* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
	* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
	*
	* <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
	*/ 

/* Includes ------------------------------------------------------------------*/
#include "global/global.h"
#include "stm32f10x.h"
#include <Utilities.h>
#include "spi.h"
#include "simple_server.h"
#include "stm32f10x_conf.h"
#include <stdio.h>
#include "stm32f10x_it.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//unsigned char mymac[6] = {0x00,0x00,0x00,0x00,0x00,0x01};//use STM32 divece UID as mac
unsigned char mymac[6] = {0x00,0x04,0xa3,0x11,0x22,0x33};//use STM32 divece UID as mac


GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;


//unsigned int timeCnt = 0;//全局时间ms

void TimerInit()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  // 指定抢占式优先级级别
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//指定响应优先级级别
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period =  18091;// 72367/4; // 7866;// 359;          
	TIM_TimeBaseStructure.TIM_Prescaler = 3;       
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;    
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 

	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); 

	/* TIM IT enable */
	TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);

	/* TIM Disable counter */
	TIM_Cmd(TIM4, ENABLE);
}
void test()
{
	int quickCnt = 0,slowCnt = 0,tmp;
	char ledCtrl = 0;
	char dhcp_step = 0;
	int hdcp_time = 0;
    while(1)
    {
		quickCnt++;
		tmp = quickCnt / 60000 %2;
		if(ledCtrl != tmp)
		{
			ledCtrl = tmp;
			slowCnt++;
			if(ledCtrl)
				GPIO_ResetBits(GPIOF, GPIO_Pin_7);
			else
				GPIO_SetBits(GPIOF, GPIO_Pin_7);

		}
		//if(dhcp_step == 0)
		if(hdcp_time < slowCnt)
		{
			dhcp_step = 1;
			////start DHCP discover
			
			
			hdcp_time = slowCnt + 4;
			dbglog("t=%d\r\n",configInfo.time);
		}
    }

}

/*GPIO接口初始化*/
void GPIO_Configuration(void)
{
	//使用到的资源时钟使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOF, ENABLE);
		
	/*LED灯初始化*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;	//DS1--4 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);


	
	/*SPI enc28j60 的RESET初始化*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		    //SPI FLASH CS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_10);			         //SPI CS1

	/*power save mode*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;		    //SPI FLASH CS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_2);			         //SPI CS1

	/*cc2530 reset*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;		    //SPI FLASH CS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_15);			         //SPI CS1
	

	/*ENC28J60的INT中断输入初始化*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   
	GPIO_Init(GPIOC, &GPIO_InitStructure);		         
}


/*串口1初始化*/
void USART1_Config(void){
	/* USART configured as follow:
				- BaudRate = 115200 baud  
				- Word Length = 8 Bits
				- One Stop Bit
				- No parity
				- Hardware flow control disabled (RTS and CTS signals)
				- Receive and transmit enabled
	*/
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA
												, ENABLE);
	/* Enable the USART1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
		

	/*串口1使用的GPIO管脚初始化*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	         //USART1 TX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    //复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);		    //A端口 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	         //USART1 RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;   //复用开漏输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);		         //A端口 


	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	/* Configure USART1 */
	USART_Init(USART1, &USART_InitStructure);

	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	/* Enable the USART1 */
	USART_ClearFlag(USART1,USART_FLAG_TC);
	USART_Cmd(USART1, ENABLE);
}
void USART4_Configuration(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure; 

	NVIC_InitTypeDef NVIC_InitStructure;
	
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  // 指定抢占式优先级级别
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//指定响应优先级级别
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC ,ENABLE);
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_UART4,ENABLE);
	/*
	*  USART4_TX -> PC10 , USART4_RX ->	PC11
	*/				
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);		   

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(UART4, &USART_InitStructure); 
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	USART_ClearFlag(UART4,USART_FLAG_TC);
	USART_Cmd(UART4, ENABLE);
}


#ifdef  USE_FULL_ASSERT

/**
	* @brief  Reports the name of the source file and the source line number
	*   where the assert_param error has occurred.
	* @param  file: pointer to the source file name
	* @param  line: assert_param error line source number
	* @retval None
	*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
	/* User can add his own implementation to report the file name and line number,
		 ex: dbglog("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/* Private functions ---------------------------------------------------------*/

/**
	* @brief  Main program.
	* @param  None
	* @retval None
	*/
int main(void)
{
	unsigned char *pUID = (unsigned char *)0x1FFFF7E8;
	int i;


#if 0 // FLASH_PROTECT

    if(FLASH_GetReadOutProtectionStatus() == RESET)
	{	 
	     FLASH_Unlock();
//         _uartstrout(0,"\r\n Flash not read protect and flow set protect");
		 if(FLASH_ReadOutProtection(ENABLE) == FLASH_COMPLETE)
		 {
//		     _uartstrout(0,"\r\nFlash Set Read protect success"); 
		 }
//		 SystemReset();
		  FLASH_Lock();	
	}
	else
	{
//		 _uartstrout(0,"\r\nFlash  read protect");

	}

#endif 

	/* GPIO管脚初始化 */
	GPIO_Configuration();

	TimerInit();


	/*串口初始化*/
	USART1_Config();
	USART4_Configuration();

	//for test
	if(1)
	{
		char *str = "hello\r\n";
		for(i=0;i<strlen(str);i++)
		{
			UART4->DR = (uint8_t)str[i];

			/* Loop until the end of transmission */
			//while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)/*等待发送完成*/
			while( ! ((UART4->SR>>6) & 0x01))
			{

			}
		}
	}
	LED_Configuration();
	/*SPI1接口初始化*/
	SPI1_Init();
	SPI2_Init();
	
#if 1
	//mac 首地址必须为0
	for(i=8;i<12;i++)
		mymac[i-6] = pUID[i];
#endif
	dbglog("mymac=\r\n");
	for(i=0;i<6;i++)
		dbglog("%02x ",mymac[i]);
	dbglog("\r\n");

	//GPIO_SetBits(GPIOB, GPIO_Pin_10);
	/*初始化ENC28J60*/
	enc28j60Init(mymac);	

	dbglog("----------------------------------------------\r\n");
	dbglog("测试程序\r\n");

	//test();

	/*ENC28J60初始化以及Server程序*/
	simple_server();

}



/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
