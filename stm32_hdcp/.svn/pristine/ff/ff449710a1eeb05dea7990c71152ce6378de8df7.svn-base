/**
  ******************************************************************************
  * @file    EEPROM_Emulation/src/eeprom.c 
  * @author  MCD Application Team
  * @version V3.1.0
  * @date    07/27/2009
  * @brief   This file provides all the EEPROM emulation firmware functions.
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
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */ 
/** @addtogroup EEPROM_Emulation
  * @{
  */ 

/* Includes ------------------------------------------------------------------*/
#include "global/global.h"
//#include "myconfig.h"
//#include "type_config.h"
//#include "dev_include.h"
#include <stm32f10x.h>
//#include "smi_log.h"
#include "string.h"
//#include "..\..\smi\tool\StrConvert.h"
#include "eeprom.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Global variable used to store variable value in read sequence */

/* Virtual address defined by the user: 0xFFFF value is prohibited */

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Restore the pages to a known good state in case of page's status
  *   corruption after a power loss.
  * @param  None.
  * @retval - Flash error code: on write Flash error
  *         - FLASH_COMPLETE: on success
  */
#define EEPROM_DRIVER
#ifdef EEPROM_DRIVER

//#define DBG_EE
uint16_t EE_Init()
{
     //FLASH_Unlock();
	 return 0;
}

uint16_t EE_WriteData(uint32_t Address, uint8_t *str,uint16_t len)
{
	uint32_t i;
	uint16_t temp;
	uint8_t *buf;
    //uint32_t Address = EEADDRESS;
	buf = str;

	#ifdef DBG_EE
	dbglog("EE_WriteData before=[%x]%d\r\n",Address,len);
	for(i=0;i<len;i++)
	{
		dbglog("%02x ",buf[i]);
		if(i%8 == 7)
			dbglog(" ");
		if(i%16 == 15)
			dbglog("\r\n");
	}
	dbglog("\r\n");

	#endif

    FLASH_Unlock();
    
	FLASH_ErasePage(Address);
	for(i = 0;i<len/2;i++)
	{
      temp = (*str << 8);
      str++;
      temp |= *str;
      str++;
	    FLASH_ProgramHalfWord(Address+i*2, temp);
	}

	FLASH_Lock();
	#ifdef DBG_EE
	dbglog("EE_WriteData after=\r\n");
	EE_ReadData(Address,buf,len);
	#endif
	
	return 0;
}

void EE_ReadData(uint32_t Address, uint8_t *str,uint16_t len)
{
   uint32_t i;
   uint16_t temp;
	uint8_t *buf;
	buf = str;
	//uint32_t Address = EEADDRESS;
	for(i = 0;i<len/2;i++)
	{
		temp = (*(__IO uint16_t*)(Address+i*2));
		*str = temp >> 8;
		str++;
		*str = temp & 0x00FF;
		str++;

	}
	#ifdef DBG_EE
	dbglog("EE_ReadData=[%x]%d\r\n",Address,len);
	for(i=0;i<len;i++)
	{
		dbglog("%02x ",buf[i]);
		if(i%8 == 7)
			dbglog(" ");
		if(i%16 == 15)
			dbglog("\r\n");
	}
	dbglog("\r\n");

	#endif
   
  
}
unsigned char bufFlash[WRITE_SIZE];

uint16_t EE_ReadWriteData(uint32_t Address, uint8_t *str,uint16_t len)
{
	uint32_t i;
	uint16_t temp;
	uint32_t startPageAddress = Address/PAGE_SIZE*PAGE_SIZE;
	uint32_t startInPageAddress = Address%PAGE_SIZE;
	uint32_t readLen = WRITE_SIZE;
	unsigned char* buf = bufFlash;
	for(i = 0;i<readLen/2;i++)
	{
		temp = (*(__IO uint16_t*)(startPageAddress+i*2));
		*buf = temp >> 8;
		buf++;
		*buf = temp & 0x00FF;
		buf++;

	}
	#ifdef DBG_EE
	dbglog("EE_ReadWriteData read=[%x]%d\r\n",Address,len);
	buf = bufFlash;
	for(i=0;i<readLen;i++)
	{
		dbglog("%02x ",buf[i]);
		if(i%8 == 7)
			dbglog(" ");
		if(i%16 == 15)
			dbglog("\r\n");
	}
	dbglog("\r\n");

	#endif

	#ifdef DBG_EE
	dbglog("EE_ReadWriteData modify=[%x]%d\r\n",startInPageAddress,len);
	buf = str;
	for(i=0;i<len;i++)
	{
		dbglog("%02x ",buf[i]);
		if(i%8 == 7)
			dbglog(" ");
		if(i%16 == 15)
			dbglog("\r\n");
	}
	dbglog("\r\n");

	#endif
	buf = bufFlash;
	for(i = 0;i<len;i++)
	{
		buf[startInPageAddress + i] = *str;
		str++;

	}

	FLASH_Unlock();

	FLASH_ErasePage(startPageAddress);
	buf = bufFlash;
	for(i = 0;i<readLen/2;i++)
	{
		temp = (*buf << 8);
		buf++;
		temp |= *buf;
		buf++;
		FLASH_ProgramHalfWord(startPageAddress+i*2, temp);
	}

	FLASH_Lock();
	#ifdef DBG_EE
	dbglog("EE_ReadWriteData after\r\n");
	EE_ReadData(startPageAddress,bufFlash,readLen);
	#endif
	return 0;
}

#endif

/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
