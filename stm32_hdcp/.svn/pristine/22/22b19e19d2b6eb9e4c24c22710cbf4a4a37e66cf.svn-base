/****************************************Copyright (c)****************************************************
**                                      
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               uctsk_LWIP.c
** Descriptions:            The uctsk_LWIP application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2011-3-2
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
#include "search.h"
#include "webpage.h"
#include <includes.h> 
/* lwIP includes. */
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/memp.h"
#include "lwip/stats.h"
#include "netif/loopif.h"
#include "ip.h"
#include "sockets.h"
#include "uCOS-II/uc-cpu/cpu.h"
#include  <errno.h>
#include "../stm32f10x_it.h"

#define MAX_LISTEN		2
#define LWIP_RECEIVE_BUF_SIZE		256

//#define host_port 80 // 8000
/* Private variables ---------------------------------------------------------*/
static  OS_STK  AppLWIPTaskStk[APP_TASK_LWIP_STK_SIZE];
//uint8_t webpage [sizeof(WebSide)];
//extern struct netif _netif;

static  OS_STK  AppReceiveTaskStk[MAX_LISTEN][APP_TASK_LWIP_STK_SIZE];
static  unsigned char  LwipReceiveBuf[MAX_LISTEN][LWIP_RECEIVE_BUF_SIZE];
static  unsigned char  usedMask[MAX_LISTEN] = {0};

typedef struct{
	char	index;
	int clientfd;
}RECEIVE_ARGS;
/* Private function prototypes -----------------------------------------------*/
static  void    uctsk_LWIP  (void *pdata);
//static  void    ADC_Configuration(void);
#if 0
static  void    vHandler_HTTP(struct netconn  *pstConn);
static  void    __Handler_HTTPGet(struct netconn  *pstConn);
#endif

void  App_LWIPTaskCreate (void)
{
    CPU_INT08U  os_err;

	os_err = os_err; /* prevent warning... */

	os_err = OSTaskCreate((void (*)(void *)) uctsk_LWIP,				
                          (void          * ) 0,							
                          (OS_STK        * )&AppLWIPTaskStk[APP_TASK_LWIP_STK_SIZE - 1],		
                          (INT8U           ) APP_TASK_LWIP_PRIO  );							

	#if OS_TASK_NAME_EN > 0
    	OSTaskNameSet(APP_TASK_LWIP_PRIO, "Task LWIP", &os_err);
	#endif
}

#if 1
u16_t bindport2 = 13501;
	//int sock,clientfd;
	OS_EVENT  *mutex_usart = NULL;
static  void    uctsk_LWIP_Receive(void *pdata);
void USART2_Configuration(void);
static  void    uctsk_LWIP(void *pdata)
{

	int sock, err, clientfd, len;
	//int  err, len;
	int i;
	INT8U err2;
	struct sockaddr_in client_addr,server_addr;


	Init_lwIP();

	USART2_Configuration();
	{
		char *str = "hello\r\n";
		for(i=0;i<strlen(str);i++)
		{
			USART2->DR = (uint8_t)str[i];

			/* Loop until the end of transmission */
			//while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)/*等待发送完成*/
			while( ! ((USART2->SR>>6) & 0x01))
			{

			}
		}
	}
	mutex_usart = OSMutexCreate (APP_TASK_LWIP_RECEIVE_PRIO-1, &err2);

	if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(bindport2);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (lwip_bind(sock, (struct sockaddr *)&server_addr, sizeof (server_addr)) < 0)
	{
	return;
	}


	lwip_listen(sock,MAX_LISTEN);

	len = sizeof(client_addr);

	while (1)
	{
		clientfd = lwip_accept(sock, (struct sockaddr *)&client_addr, &len);
		////create receive thread
		{
			//get unused task
			for(i=0;i<MAX_LISTEN;i++)
				if(usedMask[i] == 0)
				{
					break;
				}
			printf("i=%d\r\n",i);
			if(i != MAX_LISTEN)
			{
				RECEIVE_ARGS args;
				CPU_INT08U  os_err;
				args.index = i;
				args.clientfd = clientfd;

				usedMask[i] = 1;
				printf("try OSTaskCreate=%d\r\n",i);
				os_err = OSTaskCreate((void (*)(void *)) uctsk_LWIP_Receive,				
			                          (void          * ) &args,							
			                          (OS_STK        * )&AppReceiveTaskStk[i][APP_TASK_LWIP_STK_SIZE - 1],		
			                          (INT8U           ) APP_TASK_LWIP_RECEIVE_PRIO + i  );							
			}
			else
			{
				printf("too much thread been used\r\n");
			}
		}
	}   
	lwip_close(sock);
}
/////创建MAX_LISTEN个线程监听连接，
/////所有线程共享一个串口收发，该构架思想要求有一个串口发送，必有一个串口接收，否则等到超时
/////故对串口收发上锁
/////该构架为面向请求，对于超出范围的请求，最好需要zigbee端返回"not supported"之类的回应
static  void    uctsk_LWIP_Receive(void *pdata)
{
	RECEIVE_ARGS *pargs = (RECEIVE_ARGS*)pdata;
	int clientfd = pargs->clientfd;
	int index = pargs->index;
	int i,MsgLen;
	INT8U err2;

	printf("new receive thread=%d,%d\r\n",index,clientfd);

	while(1)
	{
		memset(LwipReceiveBuf[index],0,LWIP_RECEIVE_BUF_SIZE);
		
		MsgLen = lwip_recv(clientfd, LwipReceiveBuf[index], LWIP_RECEIVE_BUF_SIZE, 0);
		//lwip_send(clientfd, send_buf, MsgLen, 0);

		if(MsgLen > 0)
		{
			int startPos = 0;
			int endPos = 0;
			INT8U dataLen;
			INT8U found;
				
			printf("received[%d]=%d\r\n",index,MsgLen);
			for(i = 0;i<MsgLen;i++)
			{
				printf("%02x ",LwipReceiveBuf[index][i]);
				if(i%8 == 7)
					printf(" ");
				if(i%16 == 15)
					printf("\r\n");
			}
			printf("\r\n");
			//////因为非常可能出现多帧合并的情况，需要把请求拆分
			//////帧格式0xAA 0x55 ...... 0x55
			while(1)
			{
				////暂不考虑不完全帧的合并操作
				found = 0;
				//// find start flag(0xAA 0x55)
				for(i=startPos;i<MsgLen;i++)
					if(LwipReceiveBuf[index][i] == 0xAA)
					{
						if(i+2<MsgLen && LwipReceiveBuf[index][i+1] == 0x55)
						{
							dataLen = LwipReceiveBuf[index][i+2];
							if(i+3+dataLen<=MsgLen && LwipReceiveBuf[index][i+2+dataLen] == 0x55)
							{
								startPos = i+2;
								found = 1;
								endPos = startPos+dataLen;
								break;
							}
						}
					}
				if(! found)
					break;
				#if 0
				OSMutexPend(mutex_usart, 0, &err2);
				////uart send////
				for(startPos=0;i<endPos;i++)
				{
					USART2->DR = (uint8_t)LwipReceiveBuf[index][i];

					/* Loop until the end of transmission */
					//while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)/*等待发送完成*/
					while( ! ((USART2->SR>>6) & 0x01))
					{

					}
				}

				////wait for uart receive////
				RxCounter2 = 0;
				for(i=0;i<200;i++)
				{
					OSTimeDlyHMSM(0,0,0,10);
					//receive from uart,and the last received time pass 5 ms,we consider it as receive complete
					if(RxCounter2 != 0 && OSTimeGet() > RxTime2 + 5)
						break;
				}
				if(i == 200)
					printf("socket[%d] uart timeout\r\n",index);
				
				if(RxCounter2 != 0)
				{
					////构建帧
					for(i=RxCounter2;i>0;i--)
						RxBuffer2[i+1] = RxBuffer2[i-1];
					RxBuffer2[0] = 0xAA;
					RxBuffer2[1] = 0x55;
					RxBuffer2[RxCounter2+2] = 0x55;
					lwip_send(clientfd, RxBuffer2, RxCounter2 + 3, 0);
				}

				RxCounter2 = 0;
				OSMutexPost(mutex_usart);
				
				//if(strncmp(LwipReceiveBuf[index],"quit",strlen("quit")) == 0)
				//{
				//	break;
				//}
				#endif
				startPos = endPos + 1;
				if(startPos >= MsgLen)
					break;
				OSTimeDlyHMSM(0,0,0,10);
			}
		}
		else if(MsgLen == 0)
		{
			printf("socket[%d] closed\r\n",index);
			break;
		}
		else
		{
			printf("err[%d]=%d\r\n",index,errno);
			break;
		}
	
		
	}	
	lwip_close(clientfd);
	usedMask[index] = 0;
	OSTaskDel(index);
}

void USART2_Configuration(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure; 

	NVIC_InitTypeDef NVIC_InitStructure;
	
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  // 指定抢占式优先级级别
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//指定响应优先级级别
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA ,ENABLE);
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2,ENABLE);
	/*
	*  USART1_TX -> PA2 , USART1_RX ->	PA3
	*/				
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);		   

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART2, &USART_InitStructure); 
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	USART_ClearFlag(USART2,USART_FLAG_TC);
	USART_Cmd(USART2, ENABLE);
}



#endif
#if 0
u16_t bindport2 = 13501;
	//int sock,clientfd;
	OS_EVENT  *mutex_usart = NULL;
static  void    uctsk_LWIP_Receive(void *pdata);
void USART2_Configuration(void);
static  void    uctsk_LWIP(void *pdata)
{

	int sock, err, clientfd, len;
	//int  err, len;
	int i;
	INT8U err2;
	struct sockaddr_in client_addr,server_addr;


	Init_lwIP();

	USART2_Configuration();
	{
		char *str = "hello\r\n";
		for(i=0;i<strlen(str);i++)
		{
			USART2->DR = (uint8_t)str[i];

			/* Loop until the end of transmission */
			//while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)/*等待发送完成*/
			while( ! ((USART2->SR>>6) & 0x01))
			{

			}
		}
	}
	mutex_usart = OSMutexCreate (APP_TASK_LWIP_RECEIVE_PRIO-1, &err2);

	if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(bindport2);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	if (lwip_bind(sock, (struct sockaddr *)&server_addr, sizeof (server_addr)) < 0)
	{
	return;
	}


	lwip_listen(sock,MAX_LISTEN);

	len = sizeof(client_addr);

	while (1)
	{
		clientfd = lwip_accept(sock, (struct sockaddr *)&client_addr, &len);
		////create receive thread
		{
			//get unused task
			for(i=0;i<MAX_LISTEN;i++)
				if(usedMask[i] == 0)
				{
					break;
				}
			printf("i=%d\r\n",i);
			if(i != MAX_LISTEN)
			{
				RECEIVE_ARGS args;
				CPU_INT08U  os_err;
				args.index = i;
				args.clientfd = clientfd;

				usedMask[i] = 1;
				printf("try OSTaskCreate=%d\r\n",i);
				os_err = OSTaskCreate((void (*)(void *)) uctsk_LWIP_Receive,				
			                          (void          * ) &args,							
			                          (OS_STK        * )&AppReceiveTaskStk[i][APP_TASK_LWIP_STK_SIZE - 1],		
			                          (INT8U           ) APP_TASK_LWIP_RECEIVE_PRIO + i  );							
			}
			else
			{
				printf("too much thread been used\r\n");
			}
		}
	}   
	lwip_close(sock);
}
/////创建MAX_LISTEN个线程监听连接，
/////所有线程共享一个串口收发，该构架思想要求有一个串口发送，必有一个串口接收，否则等到超时
/////故对串口收发上锁
/////该构架为面向请求，对于超出范围的请求，最好需要zigbee端返回"not supported"之类的回应
static  void    uctsk_LWIP_Receive(void *pdata)
{
	RECEIVE_ARGS *pargs = (RECEIVE_ARGS*)pdata;
	int clientfd = pargs->clientfd;
	int index = pargs->index;
	int i,MsgLen;
	INT8U err2;

	printf("new receive thread=%d,%d\r\n",index,clientfd);

	while(1)
	{
		memset(LwipReceiveBuf[index],0,LWIP_RECEIVE_BUF_SIZE);
		
		MsgLen = lwip_recv(clientfd, LwipReceiveBuf[index], LWIP_RECEIVE_BUF_SIZE, 0);
		//lwip_send(clientfd, send_buf, MsgLen, 0);

		if(MsgLen > 0)
		{
			int startPos = 0;
			int endPos = 0;
			INT8U dataLen;
			INT8U found;
				
			printf("received[%d]=%d\r\n",index,MsgLen);
			for(i = 0;i<MsgLen;i++)
			{
				printf("%02x ",LwipReceiveBuf[index][i]);
				if(i%8 == 7)
					printf(" ");
				if(i%16 == 15)
					printf("\r\n");
			}
			printf("\r\n");
			//////因为非常可能出现多帧合并的情况，需要把请求拆分
			//////帧格式0xAA 0x55 ...... 0x55
			while(1)
			{
				////暂不考虑不完全帧的合并操作
				found = 0;
				//// find start flag(0xAA 0x55)
				for(i=startPos;i<MsgLen;i++)
					if(LwipReceiveBuf[index][i] == 0xAA)
					{
						if(i+2<MsgLen && LwipReceiveBuf[index][i+1] == 0x55)
						{
							dataLen = LwipReceiveBuf[index][i+2];
							if(i+3+dataLen<=MsgLen && LwipReceiveBuf[index][i+2+dataLen] == 0x55)
							{
								startPos = i+2;
								found = 1;
								endPos = startPos+dataLen;
								break;
							}
						}
					}
				if(! found)
					break;
				#if 0
				OSMutexPend(mutex_usart, 0, &err2);
				////uart send////
				for(startPos=0;i<endPos;i++)
				{
					USART2->DR = (uint8_t)LwipReceiveBuf[index][i];

					/* Loop until the end of transmission */
					//while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)/*等待发送完成*/
					while( ! ((USART2->SR>>6) & 0x01))
					{

					}
				}

				////wait for uart receive////
				RxCounter2 = 0;
				for(i=0;i<200;i++)
				{
					OSTimeDlyHMSM(0,0,0,10);
					//receive from uart,and the last received time pass 5 ms,we consider it as receive complete
					if(RxCounter2 != 0 && OSTimeGet() > RxTime2 + 5)
						break;
				}
				if(i == 200)
					printf("socket[%d] uart timeout\r\n",index);
				
				if(RxCounter2 != 0)
				{
					////构建帧
					for(i=RxCounter2;i>0;i--)
						RxBuffer2[i+1] = RxBuffer2[i-1];
					RxBuffer2[0] = 0xAA;
					RxBuffer2[1] = 0x55;
					RxBuffer2[RxCounter2+2] = 0x55;
					lwip_send(clientfd, RxBuffer2, RxCounter2 + 3, 0);
				}

				RxCounter2 = 0;
				OSMutexPost(mutex_usart);
				
				//if(strncmp(LwipReceiveBuf[index],"quit",strlen("quit")) == 0)
				//{
				//	break;
				//}
				#endif
				startPos = endPos + 1;
				if(startPos >= MsgLen)
					break;
				OSTimeDlyHMSM(0,0,0,10);
			}
		}
		else if(MsgLen == 0)
		{
			printf("socket[%d] closed\r\n",index);
			break;
		}
		else
		{
			printf("err[%d]=%d\r\n",index,errno);
			break;
		}
	
		
	}	
	//lwip_close(clientfd);
	usedMask[index] = 0;
	OSTaskDel(index);
}

void USART2_Configuration(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure; 

	NVIC_InitTypeDef NVIC_InitStructure;
	
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  // 指定抢占式优先级级别
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;//指定响应优先级级别
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA ,ENABLE);
	RCC_APB1PeriphClockCmd( RCC_APB1Periph_USART2,ENABLE);
	/*
	*  USART1_TX -> PA2 , USART1_RX ->	PA3
	*/				
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	         
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);		   

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;	        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART2, &USART_InitStructure); 
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	//USART_ITConfig(USART2, USART_IT_TXE, ENABLE);
	USART_ClearFlag(USART2,USART_FLAG_TC);
	USART_Cmd(USART2, ENABLE);
}



#endif
#if 0
static  void    uctsk_LWIP(void *pdata)
{
	


	struct sockaddr_in servaddr,cliaddr;//socked地址数据结构
	int sockfd;
	unsigned char data[100]={0,1,2,3,4,5,6,7,8,9,0,0,0};//要发送的数据
	int ret;

	Init_lwIP();
	//while(1)
	//	OSTimeDly(10);

	sockfd=lwip_socket(AF_INET,SOCK_DGRAM,0);//创建socket 连接
	printf("lwip_socket creat sockfd=%d\r\n",sockfd);
	servaddr.sin_family=AF_INET;		//给本地服务器的地址数据结构赋值
	servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	servaddr.sin_port=htons(host_port);

	ret=lwip_bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)); //将本地服务器socket与地址进行绑定
	printf("bind ret=%d\r\n",ret);


	cliaddr.sin_family=AF_INET;//目地主机的地址收据结构这里是192.168.1.200 目地端口10000
	cliaddr.sin_addr.s_addr=inet_addr("192.168.1.103");
	cliaddr.sin_port=htons(host_port);


	while(1){
		ret=lwip_sendto(sockfd,data,sizeof(data),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr));//发送udp包
		printf("sendto ret=%d\r\n",ret);
		OSTimeDlyHMSM(0, 0, 1, 0);	 /* 200 MS  */
	}




}
#endif
#if 0
/*******************************************************************************
* Function Name  : vHandler_HTTP
* Description    : HTTP处理
* Input          : - pstConn: 指向struct netconn结构的指针
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void vHandler_HTTP(struct netconn  *pstConn)
{
	struct netbuf 		*__pstNetbuf;
	INT8S			*__pbData;
	u16_t			__s32Len;

 	__pstNetbuf = netconn_recv(pstConn);
	if(__pstNetbuf != NULL)
	{
		netbuf_data (__pstNetbuf, (void *)&__pbData, &__s32Len );

		if( strstr( (void *)__pbData, "GET" ) != NULL )
		{
			__Handler_HTTPGet(pstConn); 
	    }
	}
	netbuf_delete(__pstNetbuf);	
	netconn_close(pstConn);
}

/*******************************************************************************
* Function Name  : __Handler_HTTPGet
* Description    : 处理HTTP协议的GET请求
* Input          : - pstConn: 指向struct netconn结构的指针
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void __Handler_HTTPGet(struct netconn  *pstConn)
{
    static uint16_t pagecount = 0;
	uint16_t AD_value;
	int8_t *ptr;

	if( ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == SET )
	{
		AD_value = ADC_GetConversionValue(ADC1);
	}

	memcpy ( webpage , WebSide ,sizeof(WebSide) );

	if( ( ptr = memstrExt( (void *)webpage,"AD8%",strlen("AD8%"),sizeof(webpage)) ) != NULL )
	{
		sprintf( (void *)ptr, "%3d", AD_value);       /* insert AD converter value */
	}	

	if( ( ptr = memstrExt( (void *)webpage,"AD7%",strlen("AD7%"),sizeof(webpage)) ) != NULL )
	{ 
		AD_value = ( AD_value * 100 ) / 4000;
		* ( ptr + 0 ) = '0' + AD_value / 100;
		* ( ptr + 1 ) = '0' + ( AD_value / 10 ) % 10;
		* ( ptr + 2 ) = '0' + AD_value % 10;
	}

	if( ( ptr = memstrExt( (void *)webpage,"AD1%",strlen("AD1%"),sizeof(webpage)) ) != NULL )
	{ 
		sprintf( (void *)ptr, "%3u", ++pagecount );  
	}		
	netconn_write(pstConn, "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n", \
	              strlen("HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n"), NETCONN_COPY);

    netconn_write(pstConn, webpage, sizeof(webpage), NETCONN_COPY);	/* HTTP网页 */
}
#endif
#if 0
/*******************************************************************************
* Function Name  : ADC_Configuration
* Description    : Configure the ADC.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void ADC_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	
	/* Configure PC.5 (ADC Channel15) as analog input -------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);   
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	                     /* 独立模式 */
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;		 	                 /* 连续多通道模式 */
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	                     /* 连续转换 */
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;      /* 转换不受外界决定 */
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;		             /* 右对齐 */
	ADC_InitStructure.ADC_NbrOfChannel = 1;					                 /* 扫描通道数 */
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channel15 configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 1, ADC_SampleTime_71Cycles5);	
	ADC_Cmd(ADC1, ENABLE);                                                   /* Enable ADC1 */                      
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);                                   /* 使能转换开始 */
}
#endif
/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

