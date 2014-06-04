void LED_Configuration()
{
	u32 tmpreg = 0;
	int nPin;

	RCC->APB2ENR |= (0x01<<4);//enable ADC1 GPIOC

	tmpreg = GPIOC->CRL;
	nPin = 2;
	tmpreg &= ~(0x0f<<nPin*4);
	tmpreg |= (0x03<<nPin*4);// 00：通用推挽输出模式 11：输出模式，最大速度50MHz

	nPin = 3;
	tmpreg &= ~(0x0f<<nPin*4);
	tmpreg |= (0x03<<nPin*4);
	GPIOC->CRL = tmpreg;

	/*
	tmpreg = GPIOF->CRH;
	nPin = 8;
	nPin %= 8;
	tmpreg &= ~(0x0f<<nPin*4);
	tmpreg |= (0x03<<nPin*4);

	nPin = 9;
	nPin %= 8;
	tmpreg &= ~(0x0f<<nPin*4);
	tmpreg |= (0x03<<nPin*4);
	GPIOF->CRH = tmpreg;
*/
	
}


void LedTest()
{
	int i;
	int nPin;

	nPin = 2;
	GPIOC->BSRR = ((1<<nPin)<<16) |(0<<nPin);
	nPin = 3;
	GPIOC->BSRR = ((1<<nPin)<<16) |(0<<nPin);
	for(i=0;i<3000000;i++);

	nPin = 2;
	GPIOC->BSRR = ((0<<nPin)<<16) |(1<<nPin);
	nPin = 3;
	GPIOC->BSRR = ((0<<nPin)<<16) |(1<<nPin);

	for(i=0;i<3000000;i++);

}
void LED_Set(Led_TypeDef led,unsigned char bIsOn)
{
	int nPin;

	////LED是低电平亮
	if(led == LED2)
	{
		nPin = GPIO_Pin_2;
		if(bIsOn)
			GPIOC->BRR = nPin;
		else
			GPIOC->BSRR = nPin;
	}
	else if(led == LED3)
	{
		nPin = GPIO_Pin_3;
		if(bIsOn)
			GPIOC->BRR = nPin;
		else
			GPIOC->BSRR = nPin;
	}
		

}


