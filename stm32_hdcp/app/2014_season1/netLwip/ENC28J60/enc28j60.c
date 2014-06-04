/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			enc28j60.c
** Descriptions:		None
**						
**------------------------------------------------------------------------------------------------------
** Created by:			AVRman
** Created date:		2011-7-5
** Version:				1.0
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:	
** Version:
** Descriptions:		
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "includes.h"


/* Private variables ---------------------------------------------------------*/
static struct net_device  enc28j60_dev_entry;
static struct net_device *enc28j60_dev =&enc28j60_dev_entry;

static uint8_t  Enc28j60Bank;
static uint16_t NextPacketPtr;
static OS_EVENT *enc28j60_sem_lock;

/*******************************************************************************
* Function Name  : _delay_us
* Description    : Delay Time
* Input          : - us: Delay Time
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void _delay_us(uint32_t us)
{
	uint32_t len;
	for (;us > 0; us --)
		for (len = 0; len < 20; len++ );
}

/*******************************************************************************
* Function Name  : delay_ms
* Description    : Delay Time
* Input          : - ms: Delay Time
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void delay_ms(uint32_t ms)
{
	uint32_t len;
	for (;ms > 0; ms --)
		for (len = 0; len < 100; len++ );
}

/*******************************************************************************
* Function Name  : spi_read_op
* Description    : None
* Input          : - op:
×                  - address:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
uint8_t spi_read_op(uint8_t op, uint8_t address)
{
	int temp;
	ENC28J60_CS_ACTIVE();

	SPI_I2S_SendData(SPI1, (op | (address & ADDR_MASK)));
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)==SET);
	SPI_I2S_ReceiveData(SPI1);
	SPI_I2S_SendData(SPI1, 0x00);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)==SET);

	/* do dummy read if needed (for mac and mii, see datasheet page 29) */
	if(address & 0x80)
	{
		SPI_I2S_ReceiveData(SPI1);
		SPI_I2S_SendData(SPI1, 0x00);
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)==SET);
	}
	/* release CS */

	temp = SPI_I2S_ReceiveData(SPI1);

	ENC28J60_CS_PASSIVE();

	return (temp);
}

/*******************************************************************************
* Function Name  : spi_write_op
* Description    : None
* Input          : - op:
*                  - address:
*            	   - data:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void spi_write_op(uint8_t op, uint8_t address, uint8_t data)
{
	ENC28J60_CS_ACTIVE();
	SPI_I2S_SendData(SPI1, op | (address & ADDR_MASK));
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)==SET);
	SPI_I2S_SendData(SPI1,data);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)==SET);
	ENC28J60_CS_PASSIVE();
}

/*******************************************************************************
* Function Name  : enc28j60_set_bank
* Description    : None
* Input          : - address:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void enc28j60_set_bank(uint8_t address)
{
	/* set the bank (if needed) */
	if((address & BANK_MASK) != Enc28j60Bank)
	{
		/* set the bank */
		spi_write_op(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		spi_write_op(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
		Enc28j60Bank = (address & BANK_MASK);
	}
}

/*******************************************************************************
* Function Name  : spi_read
* Description    : None
* Input          : - address:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
uint8_t spi_read(uint8_t address)
{
	/* set the bank */
	enc28j60_set_bank(address);
	/* do the read */
	return spi_read_op(ENC28J60_READ_CTRL_REG, address);
}

/*******************************************************************************
* Function Name  : spi_read_buffer
* Description    : None
* Input          : - data:
*				   - len:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void spi_read_buffer(uint8_t* data, uint32_t len)
{
	ENC28J60_CS_ACTIVE();

	SPI_I2S_SendData(SPI1,ENC28J60_READ_BUF_MEM);
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)==SET);

	SPI_I2S_ReceiveData(SPI1);

	while(len)
	{
	    len--;
	    SPI_I2S_SendData(SPI1,0x00)	;
	    while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)==SET);

	    *data= SPI_I2S_ReceiveData(SPI1);
	    data++;
	}

	ENC28J60_CS_PASSIVE();
}

/*******************************************************************************
* Function Name  : spi_write
* Description    : None
* Input          : - address:
*				   - data:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void spi_write(uint8_t address, uint8_t data)
{
	/* set the bank */
	enc28j60_set_bank(address);
	/* do the write */
	spi_write_op(ENC28J60_WRITE_CTRL_REG, address, data);
}

/*******************************************************************************
* Function Name  : enc28j60_phy_write
* Description    : None
* Input          : - address:
*				   - data:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void enc28j60_phy_write(uint8_t address, uint16_t data)
{
	/* set the PHY register address */
	spi_write(MIREGADR, address);

	/* write the PHY data */
	spi_write(MIWRL, data);
	spi_write(MIWRH, data>>8);

	/* wait until the PHY write completes */
	while(spi_read(MISTAT) & MISTAT_BUSY)
	{
		_delay_us(15);
	}
}

/*******************************************************************************
* Function Name  : enc28j60_phy_read
* Description    : read upper 8 bits
* Input          : - address:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
uint16_t enc28j60_phy_read(uint8_t address)
{
	/* Set the right address and start the register read operation */
	spi_write(MIREGADR, address);
	spi_write(MICMD, MICMD_MIIRD);

	_delay_us(15);

	/* wait until the PHY read completes */
	while(spi_read(MISTAT) & MISTAT_BUSY);

	/* reset reading bit */
	spi_write(MICMD, 0x00);

	return (spi_read(MIRDH));
}

/*******************************************************************************
* Function Name  : enc28j60_clkout
* Description    : None
* Input          : - clk:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void enc28j60_clkout(uint8_t clk)
{
	/* setup clkout: 2 is 12.5MHz: */
	spi_write(ECOCON, clk & 0x7);
}

/*******************************************************************************
* Function Name  : enc28j60_init
* Description    : initialize the interface
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void enc28j60_init(void)
{
	uint8_t val;
	ENC28J60_CS_PASSIVE();

	/* perform system reset */
	spi_write_op(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	delay_ms(50);
	NextPacketPtr = RXSTART_INIT;

    /* Rx start */
	spi_write(ERXSTL, RXSTART_INIT&0xFF);
	val = spi_read(ERXSTL);
	printf("read ERXSTL=%x\r\n",val);
	spi_write(ERXSTH, RXSTART_INIT>>8);
	/* set receive pointer address */
	spi_write(ERXRDPTL, RXSTOP_INIT&0xFF);
	val = spi_read(ERXRDPTL);
	printf("read ERXRDPTL=%x\r\n",val);
	spi_write(ERXRDPTH, RXSTOP_INIT>>8);
	val = spi_read(ERXRDPTH);
	printf("read ERXRDPTH=%x\r\n",val);
	/* RX end */
	spi_write(ERXNDL, RXSTOP_INIT&0xFF);
	spi_write(ERXNDH, RXSTOP_INIT>>8);

	/* TX start */
	spi_write(ETXSTL, TXSTART_INIT&0xFF);
	spi_write(ETXSTH, TXSTART_INIT>>8);
	/* set transmission pointer address */
	spi_write(EWRPTL, TXSTART_INIT&0xFF);
	spi_write(EWRPTH, TXSTART_INIT>>8);
	/* TX end */
	spi_write(ETXNDL, TXSTOP_INIT&0xFF);
	spi_write(ETXNDH, TXSTOP_INIT>>8);

	/* do bank 1 stuff, packet filter: */
    /* For broadcast packets we allow only ARP packtets */
    /* All other packets should be unicast only for our mac (MAADR) */
    
    /* The pattern to match on is therefore */
    /* Type     ETH.DST */
    /* ARP      BROADCAST */
    /* 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9 */
    /* in binary these poitions are:11 0000 0011 1111 */
    /* This is hex 303F->EPMM0=0x3f,EPMM1=0x30 */
	spi_write(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_BCEN);

	/* do bank 2 stuff */
	/* enable MAC receive */
	spi_write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	/* enable automatic padding to 60bytes and CRC operations */

	// spi_write_op(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
	spi_write_op(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX);

	/* bring MAC out of reset */
	/* set inter-frame gap (back-to-back) */
	// spi_write(MABBIPG, 0x12);
	spi_write(MABBIPG, 0x15);
	val = spi_read(MABBIPG);
	printf("read MABBIPG=%x\r\n",val);

	spi_write(MACON4, MACON4_DEFER);
	spi_write(MACLCON2, 63);
	val = spi_read(MACLCON2);
	printf("read MACLCON2=%x\r\n",val);

	/* set inter-frame gap (non-back-to-back) */
	spi_write(MAIPGL, 0x12);
	spi_write(MAIPGH, 0x0C);

	/* Set the maximum packet size which the controller will accept */
	/* Do not send packets longer than MAX_FRAMELEN: */
	spi_write(MAMXFLL, MAX_FRAMELEN&0xFF);
	spi_write(MAMXFLH, MAX_FRAMELEN>>8);

    /* do bank 3 stuff */
    /* write MAC address */
    /* NOTE: MAC address in ENC28J60 is byte-backward */
    spi_write(MAADR0, enc28j60_dev->dev_addr[5]);
    spi_write(MAADR1, enc28j60_dev->dev_addr[4]);
    spi_write(MAADR2, enc28j60_dev->dev_addr[3]);
    spi_write(MAADR3, enc28j60_dev->dev_addr[2]);
    spi_write(MAADR4, enc28j60_dev->dev_addr[1]);
    spi_write(MAADR5, enc28j60_dev->dev_addr[0]);

	/* output off */
	spi_write(ECOCON, 0x00);

	// enc28j60_phy_write(PHCON1, 0x00);
	enc28j60_phy_write(PHCON1, PHCON1_PDPXMD); /* full duplex */
    /* no loopback of transmitted frames */
	enc28j60_phy_write(PHCON2, PHCON2_HDLDIS);

	enc28j60_set_bank(ECON2);
	spi_write_op(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_AUTOINC);

	/* switch to bank 0 */
	enc28j60_set_bank(ECON1);
	/* enable interrutps */
	spi_write_op(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
	/* enable packet reception */
	spi_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

	/* clock out */
	// enc28j60_clkout(2);

	enc28j60_phy_write(PHLCON, 0xD76);	//0x476
	delay_ms(20);
}


 /*******************************************************************************
* Function Name  : enc28j60_tx
* Description    : Transmit packet.
* Input          : - p:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int8_t enc28j60_tx(struct pbuf* p)
{
    INT8U err;
	struct pbuf* q;
	uint32_t len;
	uint8_t* ptr;

	//printf("tx pbuf: 0x%08x, total len %d \r\n", p, p->tot_len);
	int i;
	if(1)
	{
		printf("send=%d\r\n", p->tot_len);
		for(i=0;i<p->tot_len;i++)
		{
			printf("%02x ", *((uint8_t*)p->payload+i));
			if(i%8 == 7)
				printf(" ");
			if(i%16 == 15)
				printf("\r\n");
		}
		printf("\r\n");
	}

    /* lock enc28j60 */
    OSSemPend(enc28j60_sem_lock,0,&err);	  

    /* disable enc28j60 interrupt */

	/* Set the write pointer to start of transmit buffer area */
	spi_write(EWRPTL, TXSTART_INIT&0xFF);
	spi_write(EWRPTH, TXSTART_INIT>>8);
	/* Set the TXND pointer to correspond to the packet size given */
	spi_write(ETXNDL, (TXSTART_INIT+ p->tot_len + 1) & 0xFF );
	spi_write(ETXNDH, (TXSTART_INIT+ p->tot_len + 1)>>8);

	/* write per-packet control byte (0x00 means use macon3 settings) */
	spi_write_op(ENC28J60_WRITE_BUF_MEM, 0, 0x00);

	for (q = p; q != NULL; q = q->next)
	{
        ENC28J60_CS_ACTIVE();

		SPI_I2S_SendData(SPI1, ENC28J60_WRITE_BUF_MEM);
		while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY)==SET);

		len = q->len;
		ptr = q->payload;
        while(len)
        {
			SPI_I2S_SendData(SPI1,*ptr) ;
			while( SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET );
			ptr++;
			len--;
        }
        ENC28J60_CS_PASSIVE();
	}

	/* send the contents of the transmit buffer onto the network */
	spi_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
	/* Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12. */
	if( (spi_read(EIR) & EIR_TXERIF) )
	{
		spi_write_op(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
	}

    /* enable enc28j60 interrupt */

	/* unlock enc28j60 */
	OSSemPost(enc28j60_sem_lock);	     /* 发送信号量 */

    return 0;
}

 /*******************************************************************************
* Function Name  : enc28j60_rx
* Description    : Receive packet.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
struct pbuf *enc28j60_rx(void)
{
    INT8U err;
	struct pbuf* p;
	uint32_t len = 0;
	uint16_t rxstat;
	uint32_t pk_counter;

    p = NULL;

    /* lock enc28j60 */
    OSSemPend(enc28j60_sem_lock,0,&err);	  
    /* disable enc28j60 interrupt */

    pk_counter = spi_read(EPKTCNT);
    if (pk_counter)
    {
        /* Set the read pointer to the start of the received packet */
        spi_write(ERDPTL, (NextPacketPtr));
        spi_write(ERDPTH, (NextPacketPtr)>>8);

        /* read the next packet pointer */
        NextPacketPtr  = spi_read_op(ENC28J60_READ_BUF_MEM, 0);
        NextPacketPtr |= spi_read_op(ENC28J60_READ_BUF_MEM, 0)<<8;

        /* read the packet length (see datasheet page 43) */
        len  = spi_read_op(ENC28J60_READ_BUF_MEM, 0);	    /* 0x54 */
        len |= spi_read_op(ENC28J60_READ_BUF_MEM, 0) <<8;	/* 5554 */

        len -= 4; /* remove the CRC count */

        /* read the receive status (see datasheet page 43) */
        rxstat  = spi_read_op(ENC28J60_READ_BUF_MEM, 0);
        rxstat |= ((uint16_t)spi_read_op(ENC28J60_READ_BUF_MEM, 0))<<8;

        /* check CRC and symbol errors (see datasheet page 44, table 7-3): */
        /* The ERXFCON.CRCEN is set by default. Normally we should not */
        /* need to check this. */
        if ( (rxstat & 0x80) == 0 )
        {
            /* invalid */
            len = 0;
        }
        else
        {
            /* allocation pbuf */
            p = pbuf_alloc(PBUF_LINK, len, PBUF_RAM);
            if (p != NULL )
            {
                uint8_t* data;
                struct pbuf* q;

                for (q = p; q != NULL; q= q->next)
                {
                    data = q->payload;
                    len = q->len;

                    ENC28J60_CS_ACTIVE();

                    SPI_I2S_SendData(SPI1,ENC28J60_READ_BUF_MEM);
                    while( SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET );

                    SPI_I2S_ReceiveData(SPI1);

                    while(len)
                    {
                        len--;
                        SPI_I2S_SendData(SPI1,0x00)	;
                        while( SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET );

                        *data = SPI_I2S_ReceiveData(SPI1);
                        data++;
                    }

                    ENC28J60_CS_PASSIVE();
                }
            }
        }

        /* Move the RX read pointer to the start of the next received packet */
        /* This frees the memory we just read out */
        spi_write(ERXRDPTL, (NextPacketPtr));
        spi_write(ERXRDPTH, (NextPacketPtr)>>8);

        /* decrement the packet counter indicate we are done with this packet */
        spi_write_op(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
    }
	else
	{
		/* switch to bank 0 */
		enc28j60_set_bank(ECON1);
		/* enable packet reception */
		spi_write_op(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

	}

    /* enable enc28j60 interrupt */

	/* unlock enc28j60 */
	OSSemPost(enc28j60_sem_lock);	     /* 发送信号量 */
	if(1 && p)
	{
		int i;
		printf("receive=%d\r\n", p->len);
		for(i=0;i<p->len;i++)
		{
			printf("%02x ", *((uint8_t*)p->payload+i));
			if(i%8 == 7)
				printf(" ");
			if(i%16 == 15)
				printf("\r\n");
		}
		printf("\r\n");
		//printf("read=%d\r\n",p->len);
	}

    return p;
}

static void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable SPI1 and GPIOA clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOC, ENABLE);


	/*SPI FLASH 的CS信号初始化*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;		    //SPI FLASH CS
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/*由于SPI FLASH与ENC28J60使用了相同的SPI接口，所有置高SPI FLASH的CS信号，不使能SPI FLASH*/
	GPIO_SetBits(GPIOC, GPIO_Pin_4);			         //SPI CS1


    /* enable gpiob port clock */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIO_ENC28J60_CS , ENABLE);

    /* Configure SPI1 pins:  SCK, MISO and MOSI ----------------------------*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* configure PC12 as CS */
	GPIO_InitStructure.GPIO_Pin = ENC28J60_CS_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(ENC28J60_CS_PORT, &GPIO_InitStructure);

}

static void SetupSPI (void)
{
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
    SPI_Cmd(SPI1, ENABLE);
}

void hw_enc28j60_init(void)
{
	unsigned char *pUID = (unsigned char *)0x1FFFF7E8;
	char i;
	GPIO_Configuration();
	SetupSPI();

	/* Update MAC address */
	//use STM32 device UID ad mac address
	for(i=6;i<12;i++)
		enc28j60_dev_entry.dev_addr[i-6] = pUID[i];
#if 1
	enc28j60_dev_entry.dev_addr[0] = emacETHADDR0;
	enc28j60_dev_entry.dev_addr[1] = emacETHADDR1;
	enc28j60_dev_entry.dev_addr[2] = emacETHADDR2;
	enc28j60_dev_entry.dev_addr[3] = emacETHADDR3;
	enc28j60_dev_entry.dev_addr[4] = emacETHADDR4;
	enc28j60_dev_entry.dev_addr[5] = emacETHADDR5;
#endif
#if 1
	enc28j60_dev_entry.dev_addr[0] = 0x00;
	enc28j60_dev_entry.dev_addr[1] = 0x04;
	enc28j60_dev_entry.dev_addr[2] = 0xA3;
	/* generate MAC addr (only for test) */
	enc28j60_dev_entry.dev_addr[3] = 0x11;
	enc28j60_dev_entry.dev_addr[4] = 0x22;
	enc28j60_dev_entry.dev_addr[5] = 0x33;
#endif

	enc28j60_sem_lock = OSSemCreate(1);
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/

