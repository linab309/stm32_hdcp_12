#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	//USART_SendData(EVAL_COM1, (uint8_t) ch); /*����һ���ַ�����*/ 
	USART1->DR = (uint8_t)ch;

	/* Loop until the end of transmission */
	//while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)/*�ȴ��������*/
	while( ! ((USART1->SR>>6) & 0x01))
	{

	}
	return ch;
}
