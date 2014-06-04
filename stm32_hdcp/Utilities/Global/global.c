#include "global.h"

#if ! defined(DEBUG_PORT)
#error ! defined(DEBUG_PORT)
#endif

#if DEBUG_PORT == 1
	#define DEBUG_USARTx	USART1
#elif DEBUG_PORT == 2
#define DEBUG_USARTx	USART2
#endif

PUTCHAR_PROTOTYPE
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	//USART_SendData(EVAL_COM1, (uint8_t) ch); /*发送一个字符函数*/ 
	DEBUG_USARTx->DR = (uint8_t)ch;

	/* Loop until the end of transmission */
	//while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)/*等待发送完成*/
	while( ! ((DEBUG_USARTx->SR>>6) & 0x01))
	{

	}
	return ch;
}

