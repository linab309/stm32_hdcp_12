

#include <global/global.h>
#include <string.h>
#include "enc28j60.h"
#include "ip_arp_udp_tcp.h"
#include <Utilities.h>
#include "net.h"
#include "simple_server.h"
#include "stdio.h"
#include "stm32f10x_it.h"
#include "eeprom.h"
#include "rtc.h"
#define PSTR(s) s
extern  void USART_OUT(USART_TypeDef* USARTx, uint8_t *Data,uint16_t Len);


// please modify the following two lines. mac and ip have to be unique
// in your local area network. You can not have the same numbers in
// two devices:
#if defined(USE_DHCP)
static unsigned char myip[4] = {0,0,0,0};
#else
static unsigned char myip[4] = {192,168,1,250};
#endif

extern unsigned char mymac[6];

static unsigned int mywwwport = 13501; // listen port for tcp
//
static unsigned int myudpport =1200; // listen port for udp

#define BUFFER_SIZE 1500//400
static unsigned char buf[BUFFER_SIZE+1] = {0};

#define MAX_TCP_RESEND_FRAME	10	//最大支持的同时操作的数目，不是做多支持的连接数
#define SEND_SIZE_MAX	576		//tcp head len(56) + our_protocol_max_len(512)+协议头尾(5)
unsigned char buf1[SEND_SIZE_MAX];
unsigned char buf2[SEND_SIZE_MAX];
unsigned char buf3[SEND_SIZE_MAX];

typedef struct{
	unsigned char isUsed;
	uint16 port;
	unsigned int ip;
	unsigned int seq;
	unsigned int timeFirstSend;
	unsigned int timeResend;
	unsigned int cntResend;
	unsigned int len;
	unsigned char buf[SEND_SIZE_MAX];
}TCP_TIMEOUT;
TCP_TIMEOUT TcpTimeout[MAX_TCP_RESEND_FRAME] ={0};
#define TCP_PROTOCOL_CMDID_START			3
#define TCP_PROTOCOL_DATA_START				5
#define TCP_PROTOCOL_MAX_LEN		512

#define UART_SEND_QUEUE_MAX	10
#define UART_BUFFER_SIZE 256
typedef struct{
	//unsigned char isUsed;
	//unsigned int seq;
	//unsigned int timeFirstSend;
	//unsigned int timeResend;
	//unsigned int cntResend;
	unsigned int len;
	unsigned char buf[UART_BUFFER_SIZE];
}UART_SEND_QUEUE;
int mUartSendQueueFront=0;
int mUartSendQueueRear = 0;
UART_SEND_QUEUE mUartSendQueue[UART_SEND_QUEUE_MAX] ={0};
typedef struct{
	unsigned char busy;
	//unsigned int seq;
	//unsigned int timeFirstSend;
	unsigned int timeResend;
	unsigned int cntResend;
	unsigned int len;
	unsigned char buf[UART_BUFFER_SIZE];
}UART_RESEND;
UART_RESEND mUartResend = 0;
//int mUartSendQueueFirstSendTime = 0;
#define UART_RESEND_TIME		250
//unsigned int uartSendSeq = 0;

///-------------uart protocol--------------------
//0xAA len ack cmd[2] data... crc 0x55
#define UART_PROTOCOL_ACK_START			2
#define UART_PROTOCOL_CMDID_START			3
#define UART_PROTOCOL_DATA_START			5

unsigned char RxBuffer2Copy[256] = {0};
unsigned int RxCounter2Copy = 0;


typedef struct{
	uint8	enable:1;
	uint8	oneShot:1;
	uint8	activated:1;//运行时标志，表示(非周期性的)定时器被激活
	uint8	reserved1:5;
	uint8	hourOn;
	uint8	minuteOn;
	uint8	week;//每周重复的日期，bit0表示星期一
	uint8	weekActivated;//运行时标志，表示(非周期性的)定时器已经激活的一周内的日期
	uint8	hourLast;//持续时间
	uint8	minuteLast;
}ALARM_INFO;
#define ALARM_MAX_COUNT	8

#define MAX_DEVICE_NUM		256
#define MAX_GROUP_NUM		MAX_DEVICE_NUM
#define MAX_SUB_GROUP_NUM		MAX_DEVICE_NUM



#define DEVICE_NAME_BYTES		16
#define GROUP_NAME_BYTES		16
#define SUB_GROUP_NAME_BYTES		16


#define ITEM_LEN		32
#define ADDR_LEN	8
#define INDEX_LEN	2

#define GRADUAL_SPEED_DEFAULT			655	//每10ms变化值*256倍

typedef struct{
	//unsigned char isUsed;
	unsigned char addr[ADDR_LEN];//	addr 0 means not used
	unsigned short groupId;
	unsigned short subGroupId;
	unsigned short deviceType;
	unsigned int time;
	unsigned char turnOn;//开关
	unsigned char lightWhite;
	unsigned char lightR;
	unsigned char lightG;
	unsigned char lightB;
	//unsigned char crc8;
	//unsigned char name[DEVICE_NAME_BYTES];
	unsigned char reserved[3];
	uint32	secondCountSetted;//记录用户设置定时器的起始时间
	uint32	nextAlarm;//下一个定时器
	ALARM_INFO alarm[ALARM_MAX_COUNT];
}ITEM_INFO;
ITEM_INFO itemInfo[MAX_DEVICE_NUM] = {0};
typedef struct{
	unsigned char turnOn;//开关
	unsigned char lightR;
	unsigned char lightG;
	unsigned char lightB;
}DEVICE_CONTROL;
//page 0:存储config，尽量少擦写
//page 1:存储反复擦写的全局信息(例如时间)
//page 2:存储.....(分来存储，不要复杂结构体)
//page 一半处:存储程序镜像
#if 0
uint8 device_addr[MAX_DEVICE_NUM][ADDR_LEN]={0};
uint16 device_groupId[MAX_DEVICE_NUM]={0};
uint16 device_subGroupId[MAX_DEVICE_NUM]={0};
uint16 device_deviceType[MAX_DEVICE_NUM]={0};
uint32 device_time[MAX_DEVICE_NUM]={0};
DEVICE_CONTROL device_control[MAX_DEVICE_NUM]={0};
uint32 device_secondCountSetted[MAX_DEVICE_NUM]={0};
uint32 device_nextAlarm[MAX_DEVICE_NUM]={0};
#endif

typedef struct{
	uint8 sendToUart:1;
	uint8 reserved:7;
}SEND_CONTROL;
SEND_CONTROL mSendControl[MAX_DEVICE_NUM];
int mSendIndex=0;//指向当前要发送的索引值
//typedef struct{
//	ITEM_INFO itemInfo;
//	unsigned char name[DEVICE_NAME_BYTES];
//}DEVICE_INFO;
//DEVICE_INFO deviceInfo={0};
//unsigned char groupName[MAX_GROUP_NUM][GROUP_NAME_BYTES] = {0};
unsigned char groupIdUsed[MAX_GROUP_NUM] = {0};
unsigned char tempSubGroupIdUsed[MAX_GROUP_NUM] = {0};
//unsigned short groupIdWeight[MAX_GROUP_NUM] = {0};

//#define DEVICE_ITEM_BYTES	30
//#define MAX_READ_ITEM_NUM	((TCP_PROTOCOL_MAX_LEN-10)/DEVICE_ITEM_BYTES)
#define MapRetCmdId(x)		(0x8000+x)
#define MapNotifyCmdId(x)		(0xA000+x)

////////////对外用//////////////////////////////
#define GENERICAPP_CMDID_SYNC_STATUS					0x0007
#define GENERICAPP_CMDID_LIGHT			0x0011
#define GENERICAPP_CMDID_ADD_TO_GROUP		0x0031
#define GENERICAPP_CMDID_REMOVE_FROM_GROUP	0x0032


#define EEPROM_AddrConfig EEPROM_START_ADDRESS

#define EEPROM_AddrItemInfo (EEPROM_AddrConfig+PAGE_SIZE)
#define EEPROM_ItemInfoCntPerPage (WRITE_SIZE/sizeof(ITEM_INFO))
#define EEPROM_PageNumItemInfo ((MAX_DEVICE_NUM + EEPROM_ItemInfoCntPerPage -1)/EEPROM_ItemInfoCntPerPage)

#define EEPROM_AddrGroupName (EEPROM_AddrItemInfo+PAGE_SIZE*EEPROM_PageNumItemInfo)
#define EEPROM_GroupNameCntPerPage (WRITE_SIZE/GROUP_NAME_BYTES)
#define EEPROM_PageNumGroupName ((MAX_GROUP_NUM + EEPROM_GroupNameCntPerPage -1)/EEPROM_GroupNameCntPerPage)

#define EEPROM_AddrDeviceName (EEPROM_AddrGroupName+PAGE_SIZE*EEPROM_PageNumGroupName)
#define EEPROM_DeviceNameCntPerPage (WRITE_SIZE/DEVICE_NAME_BYTES)
#define EEPROM_PageNumDeviceName ((MAX_DEVICE_NUM + EEPROM_DeviceNameCntPerPage -1)/EEPROM_DeviceNameCntPerPage)

#define EEPROM_AddrSubGroupName (EEPROM_AddrDeviceName+PAGE_SIZE*EEPROM_PageNumDeviceName)
#define EEPROM_SubGroupNameCntPerPage (WRITE_SIZE/SUB_GROUP_NAME_BYTES)
#define EEPROM_PageNumSubGroupName ((MAX_SUB_GROUP_NUM + EEPROM_SubGroupNameCntPerPage -1)/EEPROM_SubGroupNameCntPerPage)

#define EEPROM_AddrSubGroupMap (EEPROM_AddrSubGroupName+PAGE_SIZE*EEPROM_PageNumSubGroupName)

typedef struct{
	uint16	groupId;
	uint16	subGroupId;
}SUB_GROUP_MAP;
//位置0被保留，留给未分组使用(groupId=0 && subGroupId=0)
SUB_GROUP_MAP subGroupMap[MAX_DEVICE_NUM]={0};
unsigned char subGroupMapUsed[MAX_GROUP_NUM] = {0};
const char *strMagic = "smart_gehang3";
CONFIG_INFO configInfo = {0};
uint32 timeSaveItemPage[EEPROM_PageNumItemInfo]={0};//延后保存device条目信息，按页
uint32 timeSaveConfig=0;//延后保存配置页信息

#define EEPROM_DelaySaveItemInfo (10*60*1000)
#define EEPROM_DelaySaveConfig (24*60*60*1000)
void notify_all(unsigned short cmdID,uint8 *msg,int msgLen);

unsigned char CalcChecksum(unsigned char*buf,unsigned char len)
{
	unsigned short checksum = 0;
	unsigned short i;
	for(i=0;i<len;i++)
		checksum += buf[i];
	return checksum & 0xff;
}
uint8 IsAddrEqual(uint8 *addr1,uint8 *addr2)
{
	return memcmp(addr1,addr2,ADDR_LEN)==0;
}
uint8 IsAddrNotZero(uint8 *addr)
{
	int i;
	for(i=0;i<8;i++)
		if(addr[i] != 0)
			return TRUE;
	return FALSE;
}
void AddrPrint(uint8 *addr)
{
	int i;
	dbglog("addr[");
	for(i=0;i<8;i++)
		dbglog("%02x ",addr[i]);
	dbglog("]");
}
void uart4_send(unsigned char *buf,unsigned int len)
{
	int i;

	mUartResend.busy = 1;
	//len = buf[1] + 2;

	for(i=0;i<len;i++)
	{
		UART4->DR = (uint8_t)buf[i];

		/* Loop until the end of transmission */
		//while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TC) == RESET)/*等待发送完成*/
		while( ! ((UART4->SR>>6) & 0x01))
		{

		}
	}
	if(1)
	{
		dbglog("uart4_send=%d\r\n",len);
		for(i=0;i<len;i++)
		{
			dbglog("%02x ",buf[i]);
			if(i%8 == 7)
				dbglog(" ");
			if(i%16 == 15)
				dbglog("\r\n");
		}
		dbglog("\r\n");
		//continue;
	}
}
void UartDeQueue()
{
	if(mUartSendQueueFront == mUartSendQueueRear)
	{
		//queue empty
	}
	else
	{
		mUartResend.timeResend = configInfo.time + UART_RESEND_TIME;
		mUartResend.cntResend = 0;
		memcpy(mUartResend.buf, mUartSendQueue[mUartSendQueueFront].buf,mUartSendQueue[mUartSendQueueFront].len);
		mUartResend.len = mUartSendQueue[mUartSendQueueFront].len;
		uart4_send(mUartResend.buf,mUartResend.len);

		mUartSendQueueFront = (mUartSendQueueFront+1)%UART_SEND_QUEUE_MAX;
	}
}
void UartInQueue(uint8 *buf,int len)
{
	if((mUartSendQueueRear+1)%UART_SEND_QUEUE_MAX == mUartSendQueueFront)
	{
		//queue full
		dbglog("UartInQueue is full\r\n");
		//force to send the oldest one
		UartDeQueue();
	}

	
	{
		memcpy(mUartSendQueue[mUartSendQueueRear].buf,buf,len);
		mUartSendQueue[mUartSendQueueRear].len = len;
		mUartSendQueueRear = (mUartSendQueueRear+1)%UART_SEND_QUEUE_MAX;

		if( ! mUartResend.busy)
			UartDeQueue();
	}
}

void UartSend(unsigned char *buf,unsigned int len)
{

	UartInQueue(buf,len);
	
}
////buf复用,len为你的数据长度，不考虑头(len)、尾(crc)
void UartControlSend(unsigned char*buf,unsigned char cmdDataLen)
{
	//if(len + 2 >SERIAL_APP_TX_MAX)
	//	;//error
	int datalen = cmdDataLen + 1;
	buf[0] = 0xAA;
	buf[1] = datalen;
	//buf[2] = uartSendSeq;
	//buf[3] = uartSendSeq>>8;
	//buf[4] = uartSendSeq>>16;
	//buf[5] = uartSendSeq>>24;
	buf[UART_PROTOCOL_ACK_START] = 0;// not ack
	buf[UART_PROTOCOL_ACK_START + 0 + datalen] = CalcChecksum(&buf[UART_PROTOCOL_ACK_START],datalen);
	buf[UART_PROTOCOL_ACK_START + 1 + datalen] = 0x55;
	UartSend(buf,datalen+4);
}
void UartControlSendCmd(unsigned char*buf,unsigned short cmdID,unsigned char dataLen)
{
	//if(len + 2 >SERIAL_APP_TX_MAX)
	//	;//error
	buf[UART_PROTOCOL_CMDID_START + 0] = cmdID>>8;
	buf[UART_PROTOCOL_CMDID_START + 1] = cmdID;
	UartControlSend(buf,dataLen+2);
}
void PrintDeviceList(uint8 *buf)
{
	int i;
	dbglog("device list------------------------\r\n");
	for(i=0;i<MAX_DEVICE_NUM-1;i++)
		if(IsAddrNotZero(itemInfo[i].addr ))
		{
			EE_ReadData(EEPROM_AddrDeviceName+(i/EEPROM_DeviceNameCntPerPage)*PAGE_SIZE+i*DEVICE_NAME_BYTES,buf,DEVICE_NAME_BYTES);
			//memcpy(buf1,itemInfo[i].name,DEVICE_NAME_BYTES);
			buf[DEVICE_NAME_BYTES] = 0;
			dbglog("device[%d]=",i);
			AddrPrint(itemInfo[i].addr);
			dbglog(",%d,%d,%s\r\n",itemInfo[i].groupId,itemInfo[i].subGroupId,buf);
		}
}
void ProcessUartMsg(unsigned char *buf,unsigned int len)
{
	unsigned short cmdId = (buf[UART_PROTOCOL_CMDID_START+0]<<8)|(buf[UART_PROTOCOL_CMDID_START+1]);
	int i;
	//check if is ack
	{
		if(buf[UART_PROTOCOL_ACK_START] == 1)
		{
    		dbglog("uart ACK\r\n");
			mUartResend.busy = 0;
			
			return;
		}
	}
	//check valid

	switch(cmdId)
	{
	case 0x8001:
		break;
	case 0x9001:
		dbglog("zd0 start\r\n");
		break;
	case 0x9010:
		{
			unsigned char isFind = 0;
			unsigned char *addr = buf+UART_PROTOCOL_DATA_START;
			uint8 isSubGroupExist = FALSE;
			int firstNullPos = -1;
			int syncIndex = 0;
			for(i=0;i<MAX_DEVICE_NUM-1;i++)
				if( IsAddrNotZero(itemInfo[i].addr))
				{
					if(IsAddrEqual(itemInfo[i].addr,addr))
					{
						AddrPrint(addr);
						dbglog(" already exist[t=%d]\r\n",configInfo.time);
						itemInfo[i].time = configInfo.time;
						timeSaveItemPage[i/EEPROM_ItemInfoCntPerPage] = configInfo.time + EEPROM_DelaySaveItemInfo;
						timeSaveConfig = configInfo.time + EEPROM_DelaySaveItemInfo;
						isFind = 1;
						syncIndex = i;
						break;
					}
				}
				else
				{
					if(firstNullPos == -1)
						firstNullPos = i;
				}
			dbglog("isFind=%d\r\n",isFind);
			PrintDeviceList(buf1);
			if( ! isFind)
			{
				dbglog("firstNullPos=%d\r\n",firstNullPos);
				if(firstNullPos == -1)
				{
					uint32 timeOldest = 0xffffffff;
					dbglog("addr list if full,replace the oldest one\r\n");
					for(i=0;i<MAX_DEVICE_NUM-1;i++)
						if(timeOldest > itemInfo[i].time)
						{
							timeOldest = itemInfo[i].time;
							firstNullPos = i;
						}
				}
				syncIndex = firstNullPos;
				{
					//unsigned short groupId = *(unsigned short*)(buf+UART_PROTOCOL_DATA_START+2);
					memset(&itemInfo[firstNullPos],0,sizeof(itemInfo[firstNullPos]));
					//itemInfo[firstNullPos].isUsed = TRUE;
					itemInfo[firstNullPos].time = configInfo.time;
					memcpy(itemInfo[firstNullPos].addr , addr,ADDR_LEN);
					//timeSaveItemPage[firstNullPos/EEPROM_ItemInfoCntPerPage] = configInfo.time + EEPROM_DelaySaveItemInfo;
					//timeSaveConfig = configInfo.time + EEPROM_DelaySaveItemInfo;

					itemInfo[firstNullPos].deviceType = (buf[UART_PROTOCOL_DATA_START+ADDR_LEN+0]<<8)|(buf[UART_PROTOCOL_DATA_START+ADDR_LEN+1]);
					//itemInfo[firstNullPos].groupId = groupId;
					//EE_ReadWriteData(EEPROM_AddrDeviceName+(firstNullPos/EEPROM_DeviceNameCntPerPage)*PAGE_SIZE+firstNullPos*DEVICE_NAME_BYTES,buf+UART_PROTOCOL_DATA_START+4,DEVICE_NAME_BYTES);
					//memcpy(itemInfo[firstNullPos].name,buf+UART_PROTOCOL_DATA_START+4,DEVICE_NAME_BYTES);
					////设备上电时默认亮度为最亮
					itemInfo[firstNullPos].lightWhite = 0xff;
					itemInfo[firstNullPos].turnOn = 1;
					itemInfo[firstNullPos].lightR = 0x80;
					itemInfo[firstNullPos].lightG = 0x80;
					itemInfo[firstNullPos].lightB = 0x80;

					itemInfo[firstNullPos].groupId = 0;
					//确定子组id，有同类则加入该子组；否则新建子组
					for(i=0;i<MAX_DEVICE_NUM;i++)
						if(IsAddrNotZero(itemInfo[i].addr ) && itemInfo[i].groupId == 0
							&&itemInfo[i].deviceType == itemInfo[firstNullPos].deviceType
							)
						{
							itemInfo[firstNullPos].subGroupId = i;
							isSubGroupExist = TRUE;
							break;
						}
					if(! isSubGroupExist)
					{
						//扫描生成新组下子组的使用情况
						memset(tempSubGroupIdUsed,0,sizeof(tempSubGroupIdUsed));
						for(i=0;i<MAX_DEVICE_NUM;i++)
							if(IsAddrNotZero(itemInfo[i].addr ) && itemInfo[i].groupId == 0)
								tempSubGroupIdUsed[itemInfo[i].subGroupId]++;
						for(i=0;i<MAX_SUB_GROUP_NUM-1;i++)
							if(tempSubGroupIdUsed[i] == 0)
							{
								itemInfo[firstNullPos].subGroupId = i;
								tempSubGroupIdUsed[i] = 1;
								break;
							}
						if(itemInfo[firstNullPos].subGroupId != 0)
							for(i=1;i<MAX_DEVICE_NUM;i++)
								if(subGroupMapUsed[i] == 0)
								{
									subGroupMapUsed[i] = 1;
									subGroupMap[i].groupId = 0;
									subGroupMap[i].subGroupId = itemInfo[firstNullPos].subGroupId;
									break;
								}
					}
					//itemInfo[firstNullPos].subGroupId = 0;
					groupIdUsed[0]++;
					//memcpy(itemInfo[firstNullPos].reserved,buf+UART_PROTOCOL_DATA_START+24,4);
					//memcpy(itemInfo[firstNullPos].reserved,buf+UART_PROTOCOL_DATA_START+24,3);

					EE_WriteData(EEPROM_AddrItemInfo+(firstNullPos/EEPROM_ItemInfoCntPerPage)*PAGE_SIZE,(uint8*)&itemInfo[firstNullPos / EEPROM_ItemInfoCntPerPage*EEPROM_ItemInfoCntPerPage],EEPROM_ItemInfoCntPerPage*sizeof(ITEM_INFO));

					PrintDeviceList(buf1);

					buf1[TCP_PROTOCOL_DATA_START+0] = firstNullPos>>8;
					buf1[TCP_PROTOCOL_DATA_START+1] = firstNullPos;
					buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+0] = itemInfo[firstNullPos].groupId>>8;
					buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+1] = itemInfo[firstNullPos].groupId;
					buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+2] = itemInfo[firstNullPos].subGroupId>>8;
					buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+3] = itemInfo[firstNullPos].subGroupId;
					buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+4] = itemInfo[firstNullPos].deviceType>>8;
					buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+5] = itemInfo[firstNullPos].deviceType;
					notify_all(cmdId,buf1,INDEX_LEN+6);
				}
			}
			////in:		addr[8],groupId[2],sync[1],cmds[...]
			for(i=0;i<ADDR_LEN;i++)
				buf[UART_PROTOCOL_DATA_START+i] = addr[i];
			buf[UART_PROTOCOL_DATA_START+ADDR_LEN+0] = GENERICAPP_CMDID_SYNC_STATUS>>8;
			buf[UART_PROTOCOL_DATA_START+ADDR_LEN+1] = GENERICAPP_CMDID_SYNC_STATUS;
			//buf[UART_PROTOCOL_DATA_START+ADDR_LEN+2] = itemInfo[syncIndex].groupId;
			//buf[UART_PROTOCOL_DATA_START+ADDR_LEN+3] = itemInfo[syncIndex].groupId>>8;
			buf[UART_PROTOCOL_DATA_START+ADDR_LEN+2] = isFind;
			buf[UART_PROTOCOL_DATA_START+ADDR_LEN+3] = itemInfo[syncIndex].turnOn;
			buf[UART_PROTOCOL_DATA_START+ADDR_LEN+4] = itemInfo[syncIndex].lightWhite;
			buf[UART_PROTOCOL_DATA_START+ADDR_LEN+5] = itemInfo[syncIndex].lightR;
			buf[UART_PROTOCOL_DATA_START+ADDR_LEN+6] = itemInfo[syncIndex].lightG;
			buf[UART_PROTOCOL_DATA_START+ADDR_LEN+7] = itemInfo[syncIndex].lightB;
			buf[UART_PROTOCOL_DATA_START+ADDR_LEN+8] = GRADUAL_SPEED_DEFAULT>>8;
			buf[UART_PROTOCOL_DATA_START+ADDR_LEN+9] = GRADUAL_SPEED_DEFAULT;
			UartControlSendCmd(buf,0x0030,ADDR_LEN + 10);//
		}
		break;
	default:
		break;
	}
	
}
void PushTcpTimeout(unsigned char *buf,int len)
{
	int i;

	//backup the buf,set timeout for resend
	{
		//get index
		int index = -1;
		for(i=0;i<MAX_TCP_RESEND_FRAME;i++)
			if( ! TcpTimeout[i].isUsed)
			{
				index = i;
				break;
			}
		if(index == -1)
		{
			unsigned int oldTime = TcpTimeout[0].timeFirstSend;
			index = 0;
			// get the oldest one
			for(i=1;i<MAX_TCP_RESEND_FRAME;i++)
				if(TcpTimeout[i].timeFirstSend < oldTime)
				{
					index = i;
					oldTime = TcpTimeout[i].timeFirstSend;
				}
		}

		TcpTimeout[index].isUsed = TRUE;
		TcpTimeout[index].ip = get_ip_dest(buf) ;
		TcpTimeout[index].port = get_port_dest(buf) ;
		//dbglog("TcpTimeout[%d].ip=%x\r\n",index,TcpTimeout[index].ip);
		TcpTimeout[index].seq = get_tcp_seq(buf) ;
		//dbglog("TcpTimeout[%d].seq=%d\r\n",index,TcpTimeout[index].seq);
		TcpTimeout[index].timeFirstSend = configInfo.time;
		TcpTimeout[index].timeResend = configInfo.time + 1000;
		//dbglog("t=%d timeResend[%d]=%d\r\n",configInfo.time,index,TcpTimeout[index].timeResend);
		TcpTimeout[index].cntResend = 0;
		TcpTimeout[index].len = len;
		memcpy(TcpTimeout[index].buf,buf,len);
	}
}
void TcpSend(unsigned char *buf,int dat_p,int cmdDataLen)
{
	unsigned int len;
	buf[dat_p+0] = 0xAA;
	buf[dat_p+1] = cmdDataLen>>8;
	buf[dat_p+2] = cmdDataLen;

	buf[dat_p+TCP_PROTOCOL_CMDID_START+cmdDataLen] = CalcChecksum(&buf[dat_p+2], cmdDataLen);
	buf[dat_p+TCP_PROTOCOL_CMDID_START+1+cmdDataLen] = 0x55;
	//dbglog("send test error code\r\n");
	len = make_tcp_ack_with_data(buf,cmdDataLen+5); // send data

	PushTcpTimeout(buf,len);

}
#if 0
void TcpSendFinish(unsigned char *buf)
{
	int i;
	unsigned int len;
	len = make_tcp_ack_with_finish(buf); // send data

	PushTcpTimeout(buf,len);

}
#endif
void TcpSendCmd(unsigned char *buf,int dat_p,unsigned short cmdID,int dataLen)
{
	buf[dat_p+TCP_PROTOCOL_CMDID_START] = cmdID>>8;
	buf[dat_p+TCP_PROTOCOL_CMDID_START+1] = cmdID;
	TcpSend(buf,dat_p,dataLen+2);
}
#if 1
void TcpSendFinish(unsigned char *buf)
{

	memcpy(buf1,buf,LINK_HEAD_LEN);
	TcpSendCmd(buf1,LINK_HEAD_LEN,0x9000,0);

}
#endif
//msg is reused as a buffer,make sure it's large enough
void notify_others(uint8 *buf,unsigned short cmdID,uint8 *msg,int msgLen)
{
	//notify change
	uint32 i;
	uint32 ip;
	uint16 port;
	ip = get_ip_dest(buf);
	port = get_port_dest(buf);
	//move msg to rear,reuse this buffer
	for(i=msgLen;i>0;i--)
		msg[LINK_HEAD_LEN + TCP_PROTOCOL_DATA_START + i -1] = msg[TCP_PROTOCOL_DATA_START + i-1];
	for(i=0;i<MAX_LINK;i++)
	{
		if( linkedInfo[i].isUsed)
		{
			if(linkedInfo[i].ip == ip && linkedInfo[i].port == port)
			{
				//dbglog("self notify change ip=%x,%x\r\n",linkedInfo[i].ip,linkedInfo[i].port);
			}
			else
			{
				dbglog("notify change find ip=%x,%x\r\n",linkedInfo[i].ip,linkedInfo[i].port);
				memcpy(msg,linkedInfo[i].buf,LINK_HEAD_LEN);
				
				//make_tcp_ack_with_data(msg,msgLen);
				TcpSendCmd(msg,LINK_HEAD_LEN,cmdID,msgLen);
			}
		}
	}
	

}
//msg is reused as a buffer,make sure it's large enough
void notify_all(unsigned short cmdID,uint8 *msg,int msgLen)
{
	//notify change
	uint32 i;
	//move msg to rear,reuse this buffer
	for(i=msgLen;i>0;i--)
		msg[LINK_HEAD_LEN + TCP_PROTOCOL_DATA_START + i -1] = msg[TCP_PROTOCOL_DATA_START + i-1];
	for(i=0;i<MAX_LINK;i++)
	{
		if( linkedInfo[i].isUsed)
		{
			dbglog("notify change find ip=%x,%x\r\n",linkedInfo[i].ip,linkedInfo[i].port);
			memcpy(msg,linkedInfo[i].buf,LINK_HEAD_LEN);
			
			//make_tcp_ack_with_data(msg,msgLen);
			TcpSendCmd(msg,LINK_HEAD_LEN,cmdID,msgLen);
		}
	}
	

}
void CheckStatusAndSetNextAlarm(int addrIndex)
{
	int i,k;
	uint8 isOn = FALSE;
	uint8 OnCount = 0;
	uint32 nextAlarm = 0xffffffff;//最接近的触发时间
	RTC_Get(&timer);
	dbglog("CheckStatusAndSetNextAlarm[%d] t=%d,weekDay=%d\r\n",addrIndex,timer.secondsCount,timer.week);
	dbglog("timer=%d,%d,%d-%d:%d:%d\r\n",timer.w_year,timer.w_month,timer.w_date,timer.hour,timer.min,timer.sec);
	for(i=0;i<ALARM_MAX_COUNT;i++)
		if(itemInfo[addrIndex].alarm[i].enable)
		{
			dbglog("timer %d\r\n",i);
			dbglog("week=%x\r\n",itemInfo[addrIndex].alarm[i].week);
			if(itemInfo[addrIndex].alarm[i].week != 0)
			{
				for(k=0;k<7;k++)
				{
					if((itemInfo[addrIndex].alarm[i].week >> k)&1)
					{
						uint32 secondsStart = RTC_DateToSeconds(timer.w_year,timer.w_month,timer.w_date - timer.week + k+1,itemInfo[addrIndex].alarm[i].hourOn,itemInfo[addrIndex].alarm[i].minuteOn,0);
						uint32 secondsEnd = RTC_Add(secondsStart,itemInfo[addrIndex].alarm[i].hourLast,itemInfo[addrIndex].alarm[i].minuteLast,0);
						dbglog("secondsStart=%04d%02d%02d %02d%02d%02d\r\n",timer.w_year,timer.w_month,timer.w_date - timer.week + k+1
							,itemInfo[addrIndex].alarm[i].hourOn,itemInfo[addrIndex].alarm[i].minuteOn,0);
						dbglog("secondsStart=%d\r\n",secondsStart);
						dbglog("secondsEnd=%d\r\n",secondsEnd);
						if((itemInfo[addrIndex].alarm[i].weekActivated >> k)&1)
						{
							uint32 secondsEnd;
							OnCount++;
							if(secondsEnd <= timer.secondsCount)
							{
								itemInfo[addrIndex].alarm[i].weekActivated &= (0<<k);
								if(itemInfo[addrIndex].alarm[i].oneShot)
								{
									itemInfo[addrIndex].alarm[i].week &= (0<<k);
									if(itemInfo[addrIndex].alarm[i].week == 0)
										itemInfo[addrIndex].alarm[i].enable = 0;
								}
								OnCount--;
							}
							else
							{
								if(nextAlarm > secondsEnd)
									nextAlarm = secondsEnd;
							}
						}
						else
						{

							if(secondsEnd > timer.secondsCount)
							{
								dbglog("secondsEnd > timer.secondsCount\r\n");
								if(nextAlarm > secondsEnd)
									nextAlarm = secondsEnd;
								dbglog("nextAlarm=%d\r\n",nextAlarm);
								if(secondsStart > timer.secondsCount)
								{
									dbglog("secondsStart > timer.secondsCount\r\n");
									if(nextAlarm > secondsStart)
										nextAlarm = secondsStart;
									dbglog("nextAlarm=%d\r\n",nextAlarm);
								}
								else
								{
									dbglog("secondsStart <= timer.secondsCount\r\n");
									OnCount++;
									itemInfo[addrIndex].alarm[i].weekActivated |= (1<<k);
								}
							}

						}
					}
				}
			}
			else
			{
				uint32 secondsStart;
				uint32 secondsEnd;
				RTC_SecondsToDate(&timer2,itemInfo[addrIndex].secondCountSetted);
				dbglog("CountSetted=%04d%02d%02d %02d%02d%02d\r\n",timer2.w_year,timer2.w_month,timer2.w_date
					,timer2.hour,timer2.min,timer2.sec);
				dbglog("on=%d:%d\r\n",itemInfo[addrIndex].alarm[i].hourOn,itemInfo[addrIndex].alarm[i].minuteOn);
				dbglog("last=%d:%d\r\n",itemInfo[addrIndex].alarm[i].hourLast,itemInfo[addrIndex].alarm[i].minuteLast);
				secondsStart = RTC_DateToSeconds(timer.w_year,timer.w_month,timer2.w_date,itemInfo[addrIndex].alarm[i].hourOn,itemInfo[addrIndex].alarm[i].minuteOn,0);
				secondsEnd = RTC_Add(secondsStart,itemInfo[addrIndex].alarm[i].hourLast,itemInfo[addrIndex].alarm[i].minuteLast,0);
				dbglog("secondsStart=%d\r\n",secondsStart);
				dbglog("secondsEnd=%d\r\n",secondsEnd);
				if(itemInfo[addrIndex].alarm[i].activated)
				{
					uint32 secondsEnd;
					OnCount++;
					if(secondsEnd <= timer.secondsCount)
					{
						itemInfo[addrIndex].alarm[i].activated = 0;
						itemInfo[addrIndex].alarm[i].enable = 0;
						OnCount--;
					}
					else
					{
						if(nextAlarm > secondsEnd)
							nextAlarm = secondsEnd;
					}
				}
				else
				{

					if(secondsEnd > timer.secondsCount)
					{
						dbglog("secondsEnd > timer.secondsCount\r\n");
						if(nextAlarm > secondsEnd)
							nextAlarm = secondsEnd;
						dbglog("nextAlarm=%d\r\n",nextAlarm);
						if(secondsStart > timer.secondsCount)
						{
							dbglog("secondsStart > timer.secondsCount\r\n");
							if(nextAlarm > secondsStart)
								nextAlarm = secondsStart;
							dbglog("nextAlarm=%d\r\n",nextAlarm);
						}
						else
						{
							dbglog("secondsStart <= timer.secondsCount\r\n");
							OnCount++;
							itemInfo[addrIndex].alarm[i].activated = 1;
						}
					}
				}
			}
		}
	itemInfo[addrIndex].nextAlarm = nextAlarm;
	dbglog("nextAlarm=%d\r\n",nextAlarm);
	isOn = OnCount>0;
	if(itemInfo[addrIndex].turnOn != isOn)
	{
		itemInfo[addrIndex].turnOn = isOn;
		
		mSendControl[addrIndex].sendToUart = 1;
	}
}
int GetSubGroupMap(uint16 groupId,uint16 subGroupId)
{
	int i;
	//映射子组
	for(i=0;i<MAX_DEVICE_NUM;i++)
		if(subGroupMap[i].groupId == groupId 
			&& subGroupMap[i].subGroupId == subGroupId)
			return i;
	return -1;

}
//bufOut  year[2],month[1],day[1]
void parseDate(char *date,uint8* bufOut)
{
	uint16 value=0;
	//去除引导空格
	while(date != 0 && *date != 0)
		if(*date == ' ')
			date++;
		else
			break;
	switch(*date)
	{
	case 'A':
		date++;
		if(*date == 'p')
			bufOut[2] = 4;
		else if(*date == 'u')
			bufOut[2] = 8;
		break;
	case 'D':
		bufOut[2] = 12;
		break;
	case 'F':
		bufOut[2] = 2;
		break;
	case 'J':
		date++;
		if(*date == 'a')
			bufOut[2] = 1;
		else if(*date == 'u')
		{
			date++;
			if(*date == 'n')
				bufOut[2] = 6;
			else if(*date == 'l')
				bufOut[2] = 7;
		}
		break;
	case 'M':
		date++;
		date++;
		if(*date == 'r')
			bufOut[2] = 3;
		else if(*date == 'y')
			bufOut[2] = 5;
		break;
	case 'N':
		bufOut[2] = 11;
		break;
	case 'O':
		bufOut[2] = 10;
		break;
	case 'S':
		bufOut[2] = 9;
		break;
	}
	while(date != 0 && *date != 0)
		if(*date != ' ')
			date++;
		else
			break;
	while(date != 0 && *date != 0)
		if(*date == ' ')
			date++;
		else
			break;
	while(date != 0 && *date != 0)
		if(*date != ' ')
		{
			value = value*10;
			value += *date - '0';
			date++;
		}
		else
			break;
	bufOut[3] = value;
	
	while(date != 0 && *date != 0)
		if(*date == ' ')
			date++;
		else
			break;
	value = 0;
	while(date != 0 && *date != 0)
		if(*date != ' ')
		{
			value = value*10;
			value += *date - '0';
			date++;
		}
		else
			break;
	bufOut[0] = value>>8;
	bufOut[1] = value;
}
//bufOut  hour[1],minute[1],second[1]
void parseTime(char *date,uint8* bufOut)
{
	uint8 value=0;
	//去除引导空格
	while(date != 0 && *date != 0)
		if(*date == ' ')
			date++;
		else
			break;
	while(date != 0 && *date != 0)
		if(*date != ':')
		{
			value = value*10;
			value += *date - '0';
			date++;
		}
		else
			break;
	bufOut[0] = value;

	date++;
	value = 0;
	while(date != 0 && *date != 0)
		if(*date != ':')
		{
			value = value*10;
			value += *date - '0';
			date++;
		}
		else
			break;
	bufOut[1] = value;

	date++;
	value = 0;
	while(date != 0 && *date != 0)
		if(*date != ':')
		{
			value = value*10;
			value += *date - '0';
			date++;
		}
		else
			break;
	bufOut[2] = value;
}
void ProcessTcpMsg(unsigned char *buf,int dat_p)
{
	int i;
	unsigned short cmdID = (buf[dat_p+TCP_PROTOCOL_CMDID_START+0]<<8)|(buf[dat_p+TCP_PROTOCOL_CMDID_START+1]);
	unsigned short dataLen = (buf[dat_p + 1]<<8)|(buf[dat_p + 2]);
	unsigned short frameLen = dataLen + 5;
	#if 0//defined(DEBUG_CH)
	{
		int len = frameLen;
		dbglog("ProcessTcpMsg=%d\r\n",len);
		for(i=0;i<len;i++)
		{
			dbglog("%02x ",buf[dat_p + i]);
			if(i%8 == 7)
				dbglog(" ");
			if(i%16 == 15)
				dbglog("\r\n");
		}
		dbglog("\r\n");
		//continue;
	}
	#endif
	switch(cmdID)
	{
	case 0x0001://read nv items
	case 0x0003://send control cmds to endDevices,use short addr
		{
			//uart4_send(buf+dat_p,frameLen);
			UartControlSend(buf+dat_p+TCP_PROTOCOL_CMDID_START-UART_PROTOCOL_CMDID_START,frameLen-5);
		}
		break;
	case 0x0002://read device list mask
		{
			////in:		
			////out:	DeviceListMask[32]
			memset(&buf[dat_p+TCP_PROTOCOL_DATA_START],0,(MAX_DEVICE_NUM+8-1)/8);

			for(i=0;i<MAX_DEVICE_NUM-1;i++)
			{
				buf[dat_p+TCP_PROTOCOL_DATA_START+i/8] |= (IsAddrNotZero(itemInfo[i].addr)<<(i%8));

			}

			//dbglog("send test error code\r\n");
			TcpSendCmd(buf,dat_p,MapRetCmdId(cmdID),32);
		}
		break;
	case 0x0004://read device list
		{
			////in:		indexZoneLength[1](value is n),readIndex[n]
			////out:	itemLen[1],indexZoneLength[1](value is m),readIndex[m],item_info[i]
			unsigned char index;
			unsigned char infoLen = ITEM_LEN + 1;
			unsigned char indexZoneLengthIn = buf[dat_p+TCP_PROTOCOL_DATA_START];
			unsigned char indexZoneLengthOut = (TCP_PROTOCOL_MAX_LEN-2-2)/(infoLen+1);
			unsigned char indexZoneLength = indexZoneLengthOut>indexZoneLengthIn?indexZoneLengthIn:indexZoneLengthOut;

			memcpy(buf1,buf+dat_p+TCP_PROTOCOL_DATA_START+1,indexZoneLengthIn);

			memset(buf+dat_p+TCP_PROTOCOL_DATA_START+2,0xff,indexZoneLengthOut);

			for(i=0;i<indexZoneLength;i++)
			{
				index = buf1[i];
				if(index == 0xff)
					break;
				
				{
					int indexStartIndex = dat_p+TCP_PROTOCOL_DATA_START+2+i;
					int itemStartIndex = dat_p+TCP_PROTOCOL_DATA_START+2+indexZoneLengthOut+i*infoLen;

					buf[indexStartIndex] = index;

					buf[itemStartIndex+0] = IsAddrNotZero(itemInfo[index].addr);
					buf[itemStartIndex+1] = index>>8;
					buf[itemStartIndex+2] = index;
					buf[itemStartIndex+INDEX_LEN+1] = itemInfo[index].groupId>>8;
					buf[itemStartIndex+INDEX_LEN+2] = itemInfo[index].groupId;
					buf[itemStartIndex+INDEX_LEN+3] = itemInfo[index].subGroupId>>8;
					buf[itemStartIndex+INDEX_LEN+4] = itemInfo[index].subGroupId;
					buf[itemStartIndex+INDEX_LEN+5] = itemInfo[index].deviceType>>8;
					buf[itemStartIndex+INDEX_LEN+6] = itemInfo[index].deviceType;
					//memcpy(&buf[itemStartIndex+5] , itemInfo[index].name,DEVICE_NAME_BYTES);
					EE_ReadData(EEPROM_AddrDeviceName+(index/EEPROM_DeviceNameCntPerPage)*PAGE_SIZE+index*DEVICE_NAME_BYTES,&buf[itemStartIndex+INDEX_LEN+7],DEVICE_NAME_BYTES);
					buf[itemStartIndex+INDEX_LEN+7+DEVICE_NAME_BYTES] = itemInfo[index].turnOn;
					buf[itemStartIndex+INDEX_LEN+8+DEVICE_NAME_BYTES] = itemInfo[index].lightWhite;
					buf[itemStartIndex+INDEX_LEN+9+DEVICE_NAME_BYTES] = itemInfo[index].lightG;
					buf[itemStartIndex+INDEX_LEN+10+DEVICE_NAME_BYTES] = itemInfo[index].lightG;
					buf[itemStartIndex+INDEX_LEN+11+DEVICE_NAME_BYTES] = itemInfo[index].lightB;
					//buf[itemStartIndex+10+DEVICE_NAME_BYTES] = itemInfoCrc8[index];
					memcpy(&buf[itemStartIndex+INDEX_LEN+12+DEVICE_NAME_BYTES] , itemInfo[index].reserved,3);

				}
			}
			buf[dat_p+TCP_PROTOCOL_DATA_START+0] = infoLen;
			buf[dat_p+TCP_PROTOCOL_DATA_START+1] = indexZoneLengthOut;

			//dbglog("send test error code\r\n");
			TcpSendCmd(buf,dat_p,MapRetCmdId(cmdID),2+indexZoneLengthOut+i*infoLen);
		}
		break;
	case 0x0005://read group name list
		{
			////in:		indexZoneLength[1](value is n),readIndex[n]
			////		遇到读索引为0xff，则不继续往下读
			////out:	itemLen[1],indexZoneLength[1](value is m),readIndex[m],item_info[i]

			unsigned char index;
			unsigned char infoLen = GROUP_NAME_BYTES;
			unsigned char indexZoneLengthIn = buf[dat_p+TCP_PROTOCOL_DATA_START];
			unsigned char indexZoneLengthOut = (TCP_PROTOCOL_MAX_LEN-2-2)/(infoLen+1);
			unsigned char indexZoneLength = indexZoneLengthOut>indexZoneLengthIn?indexZoneLengthIn:indexZoneLengthOut;

			memcpy(buf1,buf+dat_p+TCP_PROTOCOL_DATA_START+1,indexZoneLengthIn);

			memset(buf+dat_p+TCP_PROTOCOL_DATA_START+2,0xff,indexZoneLengthOut);

			for(i=0;i<indexZoneLength;i++)
			{
				index = buf1[i];
				if(index == 0xff)
					break;
				
				{
					int indexStartIndex = dat_p+TCP_PROTOCOL_DATA_START+2+i;
					int itemStartIndex = dat_p+TCP_PROTOCOL_DATA_START+2+indexZoneLengthOut+i*infoLen;

					buf[indexStartIndex] = index;

					EE_ReadData(EEPROM_AddrGroupName+(index/EEPROM_GroupNameCntPerPage)*PAGE_SIZE+index*GROUP_NAME_BYTES,&buf[itemStartIndex],infoLen);
					//memcpy(&buf[itemStartIndex],groupName[index],infoLen);
					

				}
			}
			buf[dat_p+TCP_PROTOCOL_DATA_START+0] = infoLen;
			buf[dat_p+TCP_PROTOCOL_DATA_START+1] = indexZoneLengthOut;

			TcpSendCmd(buf,dat_p,MapRetCmdId(cmdID),2+indexZoneLengthOut+i*infoLen);
		}
		break;
	case 0x0006://read subGroup name list
		{
			////in:		indexZoneLength[1](value is n),readIndex[n],subGroupIndex[n]
			////		遇到读索引为0，则不继续往下读
			////out:	itemLen[1],indexZoneLength[1](value is m),readIndex[m],subGroupIndex[n],item_info[i]

			unsigned char index;
			unsigned char subIndex;
			unsigned char infoLen = SUB_GROUP_NAME_BYTES;
			unsigned char indexZoneLengthIn = buf[dat_p+TCP_PROTOCOL_DATA_START];
			unsigned char indexZoneLengthOut = (TCP_PROTOCOL_MAX_LEN-2-2)/(infoLen+2);
			unsigned char indexZoneLength = indexZoneLengthOut>indexZoneLengthIn?indexZoneLengthIn:indexZoneLengthOut;

			memcpy(buf1,buf+dat_p+TCP_PROTOCOL_DATA_START+1,indexZoneLengthIn*2);

			memset(buf+dat_p+TCP_PROTOCOL_DATA_START+2,0xff,indexZoneLengthOut*2);

			for(i=0;i<indexZoneLength;i++)
			{
				index = buf1[i];
				subIndex = buf1[indexZoneLengthIn+i];
				if(index == 0xff)
					break;
				
				{
					int indexStartIndex = dat_p+TCP_PROTOCOL_DATA_START+2+i;
					int subIndexStartIndex = dat_p+TCP_PROTOCOL_DATA_START+2+indexZoneLengthOut+i;
					int itemStartIndex = dat_p+TCP_PROTOCOL_DATA_START+2+indexZoneLengthOut*2+i*infoLen;
					int tempIndex;

					buf[indexStartIndex] = index;
					buf[subIndexStartIndex] = subIndex;

					tempIndex = GetSubGroupMap(index,subIndex);
					if(tempIndex != -1)
						EE_ReadData(EEPROM_AddrSubGroupName+(tempIndex/EEPROM_SubGroupNameCntPerPage)*PAGE_SIZE+tempIndex*SUB_GROUP_NAME_BYTES,&buf[itemStartIndex],infoLen);
					//memcpy(&buf[itemStartIndex],groupName[index],infoLen);
					

				}
			}
			buf[dat_p+TCP_PROTOCOL_DATA_START+0] = infoLen;
			buf[dat_p+TCP_PROTOCOL_DATA_START+1] = indexZoneLengthOut;

			TcpSendCmd(buf,dat_p,MapRetCmdId(cmdID),2+indexZoneLengthOut+i*infoLen);
		}
		break;
	case 0x0008://read version info
		{
			////in:		max read len[2]
			////out:	major version[2],sub version[2],grand child version[2],year[2],month[1],day[1],hour[1],minute[1],second[1]
			////主版本不同表示架构不同
			////子版本不同表示改动较大，需要升级
			////孙版本不同表示小改动，可以不升级
			uint16 maxReadLen = (buf[dat_p+TCP_PROTOCOL_DATA_START+0]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			uint16 majorVersion = MAJOR_VERSION;
			uint16 subVersion = SUB_VERSION;
			uint16 grandChildVersion = GRAND_CHILD_VERSION;
			char *date = __DATE__;
			char *time = __TIME__;

			dbglog("date=%s\r\n",date);
			dbglog("time=%s\r\n",time);

			
			buf[dat_p+TCP_PROTOCOL_DATA_START+0] = majorVersion>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+1] = majorVersion;
			buf[dat_p+TCP_PROTOCOL_DATA_START+2] = subVersion>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+3] = subVersion;
			buf[dat_p+TCP_PROTOCOL_DATA_START+4] = grandChildVersion>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+5] = grandChildVersion;
			parseDate(date,buf+dat_p+TCP_PROTOCOL_DATA_START+6);
			parseTime(time,buf+dat_p+TCP_PROTOCOL_DATA_START+10);

			//dbglog("send test error code\r\n");
			TcpSendCmd(buf,dat_p,MapRetCmdId(cmdID),13);
		}
		break;
	case 0x0009://permit join zigbee network
		{
			////in:		timeout[1]
			////sub in:timeout[1]
			////主版本不同表示架构不同
			////子版本不同表示改动较大，需要升级
			////孙版本不同表示小改动，可以不升级
			uint8 timeout = buf[dat_p+TCP_PROTOCOL_DATA_START+0];
			

			dbglog("timeout=%d\r\n",timeout);


			buf1[UART_PROTOCOL_DATA_START+0] = timeout;
			UartControlSendCmd(buf1,cmdID,1);
		}
		break;
	case 0x0010://set device name
		{
			// addr[2],name[16]
			uint16 addrIndex = (buf[dat_p+TCP_PROTOCOL_DATA_START]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			uint8 isFind = 0;
			if(addrIndex >= 0 && addrIndex < MAX_DEVICE_NUM-1)
			{
				if(IsAddrNotZero(itemInfo[addrIndex].addr ))
				{
					EE_ReadWriteData(EEPROM_AddrDeviceName+(addrIndex/EEPROM_DeviceNameCntPerPage)*PAGE_SIZE+addrIndex*DEVICE_NAME_BYTES,buf+dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN,DEVICE_NAME_BYTES);
					//memcpy(itemInfo[i].name,buf+dat_p+TCP_PROTOCOL_DATA_START+2,DEVICE_NAME_BYTES);
					isFind = 1;
				}
			}
			if(isFind)
			{
				buf1[TCP_PROTOCOL_DATA_START+0] = addrIndex>>8;
				buf1[TCP_PROTOCOL_DATA_START+1] = addrIndex;
				for(i=0;i<DEVICE_NAME_BYTES;i++)
					buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+i] = buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+i];
				notify_others(buf,MapNotifyCmdId(cmdID),buf1,INDEX_LEN + DEVICE_NAME_BYTES);
			}
			//uart4_send(buf+dat_p,frameLen);
			//UartControlSend(buf+dat_p+TCP_PROTOCOL_CMDID_START-UART_PROTOCOL_CMDID_START,frameLen-5);
		}
		break;
	case 0x0011://read device name
		{
			////in:		addr[2]
			////out:	errCode[2],addr[2],name[DEVICE_NAME_BYTES_GEN]
			//unsigned char* addr = buf+dat_p+TCP_PROTOCOL_DATA_START;
			uint16 addrIndex = (buf[dat_p+TCP_PROTOCOL_DATA_START]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			unsigned char isFind = FALSE;
			unsigned short errCode = 0;
			if(addrIndex >= 0 && addrIndex < MAX_DEVICE_NUM-1)
			{
				if(IsAddrNotZero(itemInfo[addrIndex].addr ))
				{
					EE_ReadData(EEPROM_AddrDeviceName+(addrIndex/EEPROM_DeviceNameCntPerPage)*PAGE_SIZE+addrIndex*DEVICE_NAME_BYTES,buf+dat_p+TCP_PROTOCOL_DATA_START+4,DEVICE_NAME_BYTES);
					//memcpy(buf+dat_p+TCP_PROTOCOL_DATA_START+4,itemInfo[i].name,DEVICE_NAME_BYTES);
					isFind = TRUE;
				}
			}
			if( ! isFind)
				errCode = 1;
			buf[dat_p+TCP_PROTOCOL_DATA_START+0] = errCode>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+1] = errCode;
			buf[dat_p+TCP_PROTOCOL_DATA_START+2+0] = addrIndex>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+2+1] = addrIndex;

			//dbglog("send test error code\r\n");
			TcpSendCmd(buf,dat_p,MapRetCmdId(cmdID),2+INDEX_LEN+ DEVICE_NAME_BYTES);
		}
		break;
	case 0x0013://read group id 
		{
			////in:		addr[2]
			////out:	errCode[2],addr[8],groupId[2]
			//unsigned char* addr = buf+dat_p+TCP_PROTOCOL_DATA_START;
			uint16 addrIndex = (buf[dat_p+TCP_PROTOCOL_DATA_START]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			unsigned short groupID = 0;
			unsigned char isFind = FALSE;
			unsigned short errCode = 0;
			if(addrIndex >= 0 && addrIndex < MAX_DEVICE_NUM-1)
			{
				if(IsAddrNotZero(itemInfo[addrIndex].addr ))
				{
					groupID = itemInfo[addrIndex].groupId;
					isFind = TRUE;
				}
			}
			if( ! isFind)
				errCode = 1;
			buf[dat_p+TCP_PROTOCOL_DATA_START+0] = errCode>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+1] = errCode;
			buf[dat_p+TCP_PROTOCOL_DATA_START+2+0] = addrIndex>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+2+1] = addrIndex;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+2] = groupID>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+3] = groupID;

			//dbglog("send test error code\r\n");
			TcpSendCmd(buf,dat_p,MapRetCmdId(cmdID),4+INDEX_LEN);
		}
		break;
	case 0x0014://set alarm
		{
			// addr[2],alarmNum[1],alarmInfoLen[1],alarmInfo[...]
			uint16 addrIndex = (buf[dat_p+TCP_PROTOCOL_DATA_START]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			uint8 alarmNum = buf[dat_p+TCP_PROTOCOL_DATA_START+2];
			uint8 alarmInfoLen = buf[dat_p+TCP_PROTOCOL_DATA_START+3];
			uint8 isFind = 0;
			dbglog("set alarm addrIndex=%d\r\n",addrIndex);
			if(addrIndex >= 0 && addrIndex < MAX_DEVICE_NUM-1)
			{
				if(IsAddrNotZero(itemInfo[addrIndex].addr ))
				{
					isFind = 1;
				}
			}
			dbglog("set alarm isFind=%d\r\n",isFind);
			if(isFind)
			{
				alarmNum = MIN(alarmNum,ALARM_MAX_COUNT);
				for(i=0;i<alarmNum;i++)
				{
					uint16 pos = dat_p+TCP_PROTOCOL_DATA_START+4 + i*alarmInfoLen;
					itemInfo[addrIndex].alarm[i].enable = buf[pos]>>7;
					itemInfo[addrIndex].alarm[i].oneShot = (buf[pos]>>6)&1;
					itemInfo[addrIndex].alarm[i].hourOn = buf[pos + 1];
					itemInfo[addrIndex].alarm[i].minuteOn = buf[pos + 2];
					itemInfo[addrIndex].alarm[i].week = buf[pos + 3];
					itemInfo[addrIndex].alarm[i].hourLast = buf[pos + 4];
					itemInfo[addrIndex].alarm[i].minuteLast = buf[pos + 5];
				}
				while(i<ALARM_MAX_COUNT)
				{
					//clear the unsetted alarms
					memset(&itemInfo[addrIndex].alarm[i],0,sizeof(ALARM_INFO));
					i++;
				}
				itemInfo[addrIndex].secondCountSetted = RTC_GetCounter();
				dbglog("set alarm secondCountSetted=%d\r\n",itemInfo[addrIndex].secondCountSetted);
				CheckStatusAndSetNextAlarm(addrIndex);
				timeSaveItemPage[addrIndex/EEPROM_ItemInfoCntPerPage] = configInfo.time + 0;
				
				buf1[TCP_PROTOCOL_DATA_START+0] = addrIndex>>8;
				buf1[TCP_PROTOCOL_DATA_START+1] = addrIndex;
				for(i=0;i<DEVICE_NAME_BYTES;i++)
					buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+i] = buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+i];
				//notify_others(buf,MapNotifyCmdId(cmdID),buf1,INDEX_LEN + DEVICE_NAME_BYTES);
			}
			//uart4_send(buf+dat_p,frameLen);
			//UartControlSend(buf+dat_p+TCP_PROTOCOL_CMDID_START-UART_PROTOCOL_CMDID_START,frameLen-5);
		}
		break;
	case 0x0015://set time
		{
			// year[2],month[1],day[1],hour[1],minute[1],second[1]
			uint16 year = (buf[dat_p+TCP_PROTOCOL_DATA_START+0]<<8)|buf[dat_p+TCP_PROTOCOL_DATA_START+1];

			dbglog("set time =%04d%02d%02d %02d%02d%02d\r\n",year,buf[dat_p+TCP_PROTOCOL_DATA_START+2],buf[dat_p+TCP_PROTOCOL_DATA_START+3]
				,buf[dat_p+TCP_PROTOCOL_DATA_START+4],buf[dat_p+TCP_PROTOCOL_DATA_START+5],buf[dat_p+TCP_PROTOCOL_DATA_START+6]
				);
			RTC_Set(year,buf[dat_p+TCP_PROTOCOL_DATA_START+2],buf[dat_p+TCP_PROTOCOL_DATA_START+3]
				,buf[dat_p+TCP_PROTOCOL_DATA_START+4],buf[dat_p+TCP_PROTOCOL_DATA_START+5],buf[dat_p+TCP_PROTOCOL_DATA_START+6]
				);
		}
		break;
	case 0x0020://create group,	return group id
		{
			////in:		addr[2],group name[16],subGroupName[16]
			////out:	errCode[2],addr[2],groupId[2],subGroupId[2]
			////other:	addr[2],groupId[2],subGroupId[2],group name[16],subGroupName[16]
			uint16 addrIndex = (buf[dat_p+TCP_PROTOCOL_DATA_START]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			//unsigned char* addr = buf+dat_p+TCP_PROTOCOL_DATA_START;
			unsigned short groupID = 1;
			unsigned short subGroupID = 0;
			unsigned short errCode = 0;
			int index = 0;
			////alloc new group ID
			{
				//release old used groupId
				if(addrIndex >= 0 && addrIndex < MAX_DEVICE_NUM-1)
				{
					if(IsAddrNotZero(itemInfo[addrIndex].addr ))
					{
						if(itemInfo[addrIndex].groupId != 0)
						{
							groupIdUsed[itemInfo[addrIndex].groupId]--;
							if(groupIdUsed[itemInfo[addrIndex].groupId] == 0)
							{
								memset(buf1,0,sizeof(GROUP_NAME_BYTES));
								EE_ReadWriteData(EEPROM_AddrGroupName+(itemInfo[addrIndex].groupId/EEPROM_GroupNameCntPerPage)*PAGE_SIZE+itemInfo[addrIndex].groupId*GROUP_NAME_BYTES,buf1,GROUP_NAME_BYTES);
							}
						}
						//release old used subGroupID
						{
							//扫描生成该组下子组的使用情况
							memset(tempSubGroupIdUsed,0,sizeof(tempSubGroupIdUsed));
							for(i=0;i<MAX_DEVICE_NUM-1;i++)
								if(IsAddrNotZero(itemInfo[i].addr ) && itemInfo[i].groupId == itemInfo[addrIndex].groupId)
									tempSubGroupIdUsed[itemInfo[i].subGroupId]++;
							//if(itemInfo[addrIndex].subGroupId != 0)
							{
								tempSubGroupIdUsed[itemInfo[addrIndex].subGroupId]--;
								if(tempSubGroupIdUsed[itemInfo[addrIndex].subGroupId] == 0)
								{
									index = GetSubGroupMap(itemInfo[addrIndex].groupId,itemInfo[addrIndex].subGroupId);
									if(index != -1)
									{
										subGroupMapUsed[index] = 0;
										subGroupMap[index].groupId = 0;
										subGroupMap[index].subGroupId = 0;
										memset(buf1,0,sizeof(SUB_GROUP_NAME_BYTES));
										EE_ReadWriteData(EEPROM_AddrSubGroupName+(index/EEPROM_SubGroupNameCntPerPage)*PAGE_SIZE+index*SUB_GROUP_NAME_BYTES,buf1,SUB_GROUP_NAME_BYTES);
									}
								}
							}
						}
					}
				}
				for(i=1;i<MAX_GROUP_NUM-1;i++)
					if(groupIdUsed[i] == 0)
					{
						groupID = i;
						groupIdUsed[i] = 1;
						break;
					}
			}
			dbglog("alloc new group ID=%d\r\n",groupID);
			EE_ReadWriteData(EEPROM_AddrGroupName+(groupID/EEPROM_GroupNameCntPerPage)*PAGE_SIZE+groupID*GROUP_NAME_BYTES,buf+dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN,GROUP_NAME_BYTES);
			for(i=1;i<MAX_DEVICE_NUM;i++)
				if(subGroupMapUsed[i] == 0)
				{
					index = i;
					subGroupMapUsed[i] = 1;
					subGroupMap[i].groupId = groupID;
					subGroupMap[i].subGroupId = 0;
					break;
				}
			EE_ReadWriteData(EEPROM_AddrSubGroupName+(index/EEPROM_SubGroupNameCntPerPage)*PAGE_SIZE+index*SUB_GROUP_NAME_BYTES,buf+dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN,SUB_GROUP_NAME_BYTES);
			//memcpy(groupName[groupID],buf+dat_p+TCP_PROTOCOL_DATA_START+2,GROUP_NAME_BYTES);
			
			
			if(addrIndex >= 0 && addrIndex < MAX_DEVICE_NUM-1)
			{
				if(IsAddrNotZero(itemInfo[addrIndex].addr ))
				{
					itemInfo[addrIndex].groupId = groupID;
					itemInfo[addrIndex].subGroupId = 0;
					timeSaveItemPage[addrIndex/EEPROM_ItemInfoCntPerPage] = configInfo.time + 0;// save immediately
				}
			}
			//backup the name
			for(i=0;i<GROUP_NAME_BYTES + SUB_GROUP_NAME_BYTES;i++)
				buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+4+i] = buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+i];
			//for(i=0;i<ADDR_LEN;i++)
			//	buf[dat_p+UART_PROTOCOL_DATA_START+i] = addr[i];
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+0] = GENERICAPP_CMDID_ADD_TO_GROUP;
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+1] = GENERICAPP_CMDID_ADD_TO_GROUP>>8;
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+2] = groupID;
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+3] = groupID>>8;
			//UartControlSendCmd(buf+dat_p,0x0030,ADDR_LEN+4);//add to a group

			buf[dat_p+TCP_PROTOCOL_DATA_START+0] = errCode>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+1] = errCode;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+0] = addrIndex>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+1] = addrIndex;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+2] = groupID>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+3] = groupID;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+4] = 0>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+5] = 0;
			//dbglog("send test error code\r\n");
			TcpSendCmd(buf,dat_p,MapRetCmdId(cmdID),6+INDEX_LEN);// return the alloced groupID

			buf1[TCP_PROTOCOL_DATA_START+0] = addrIndex>>8;
			buf1[TCP_PROTOCOL_DATA_START+1] = addrIndex;
			buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+0] = groupID>>8;
			buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+1] = groupID;
			buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+2] = 0>>8;
			buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+3] = 0;
			//be setted already
			//for(i=0;i<GROUP_NAME_BYTES;i++)
			//	buf1[TCP_PROTOCOL_DATA_START+ADDR_LEN+4+i] = buf[dat_p+TCP_PROTOCOL_DATA_START+ADDR_LEN+i];
			notify_others(buf,MapNotifyCmdId(cmdID),buf1,4+INDEX_LEN + GROUP_NAME_BYTES + SUB_GROUP_NAME_BYTES);
		}
		break;
	case 0x0021://add to a group
		{
			////in:		addr[2],group ID[2],subGroup Id[2]
			////out:	errCode[2]
			////other:	addr[2],group ID[2],subGroup Id[2]
			//unsigned char* addr = buf+dat_p+TCP_PROTOCOL_DATA_START;
			uint16 addrIndex = (buf[dat_p+TCP_PROTOCOL_DATA_START]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			unsigned short groupID = (buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+0]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+1]);
			unsigned short subGroupID = (buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+2]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+3]);
			uint8 isFind = 0;
			int index;
			//release old used groupId
			if(addrIndex >= 0 && addrIndex < MAX_DEVICE_NUM-1)
			{
				if(IsAddrNotZero(itemInfo[addrIndex].addr ))
				{
					if(itemInfo[addrIndex].groupId != 0)
					{
						groupIdUsed[itemInfo[addrIndex].groupId]--;
						if(groupIdUsed[itemInfo[addrIndex].groupId] == 0)
						{
							memset(buf1,0,sizeof(GROUP_NAME_BYTES));
							EE_ReadWriteData(EEPROM_AddrGroupName+(itemInfo[addrIndex].groupId/EEPROM_GroupNameCntPerPage)*PAGE_SIZE+itemInfo[addrIndex].groupId*GROUP_NAME_BYTES,buf1,GROUP_NAME_BYTES);
						}
					}
					//release old used groupId
					{
						//扫描生成该组下子组的使用情况
						memset(tempSubGroupIdUsed,0,sizeof(tempSubGroupIdUsed));
						for(i=0;i<MAX_DEVICE_NUM;i++)
							if(IsAddrNotZero(itemInfo[i].addr ) && itemInfo[i].groupId == itemInfo[addrIndex].groupId)
								tempSubGroupIdUsed[itemInfo[i].subGroupId]++;
						tempSubGroupIdUsed[itemInfo[addrIndex].subGroupId]--;
						if(tempSubGroupIdUsed[itemInfo[addrIndex].subGroupId] == 0)
						{
							index = GetSubGroupMap(itemInfo[addrIndex].groupId,itemInfo[addrIndex].subGroupId);
							if(index != -1)
							{
								subGroupMapUsed[index] = 0;
								subGroupMap[index].groupId = 0;
								subGroupMap[index].subGroupId = 0;
								memset(buf1,0,sizeof(SUB_GROUP_NAME_BYTES));
								EE_ReadWriteData(EEPROM_AddrSubGroupName+(index/EEPROM_SubGroupNameCntPerPage)*PAGE_SIZE+index*SUB_GROUP_NAME_BYTES,buf1,SUB_GROUP_NAME_BYTES);
							}
						}
					}
					isFind = 1;
				}
			}
			////update 
			if(addrIndex >= 0 && addrIndex < MAX_DEVICE_NUM-1)
			{
				if(IsAddrNotZero(itemInfo[addrIndex].addr ))
				{
					itemInfo[addrIndex].groupId = groupID;
					itemInfo[addrIndex].subGroupId = subGroupID;
					timeSaveItemPage[addrIndex/EEPROM_ItemInfoCntPerPage] = configInfo.time + 0;// save immediately
					groupIdUsed[groupID]++;
				}
			}
			//uart4_send(buf+dat_p,frameLen);

			//for(i=0;i<ADDR_LEN;i++)
			//	buf[dat_p+UART_PROTOCOL_DATA_START+i] = addr[i];
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+0] = GENERICAPP_CMDID_ADD_TO_GROUP;
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+1] = GENERICAPP_CMDID_ADD_TO_GROUP>>8;
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+2] = groupID;
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+3] = groupID>>8;
			//UartControlSendCmd(buf+dat_p,0x0030,4+ADDR_LEN);//add to a group
			if(isFind)
			{
				buf1[TCP_PROTOCOL_DATA_START+0] = addrIndex>>8;
				buf1[TCP_PROTOCOL_DATA_START+1] = addrIndex;
				buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+0] = groupID>>8;
				buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+1] = groupID;
				buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+2] = subGroupID>>8;
				buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+3] = subGroupID;
				notify_others(buf,MapNotifyCmdId(cmdID),buf1,4+INDEX_LEN);
			}
		}
		break;

	case 0x0024://set group name
		{
			////in:		group ID[2],group name[16]
			////out:	errCode[2]
			unsigned short groupID = (buf[dat_p+TCP_PROTOCOL_DATA_START]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			if(groupID < MAX_GROUP_NUM)
			{
				EE_ReadWriteData(EEPROM_AddrGroupName+(groupID/EEPROM_GroupNameCntPerPage)*PAGE_SIZE+groupID*GROUP_NAME_BYTES,buf+dat_p+TCP_PROTOCOL_DATA_START+2,GROUP_NAME_BYTES);
				//memcpy(groupName[groupID],buf+dat_p+TCP_PROTOCOL_DATA_START+2,GROUP_NAME_BYTES);
				

				buf1[TCP_PROTOCOL_DATA_START+0] = groupID>>8;
				buf1[TCP_PROTOCOL_DATA_START+1] = groupID;
				for(i=0;i<GROUP_NAME_BYTES;i++)
					buf1[TCP_PROTOCOL_DATA_START+2+i] = buf[dat_p+TCP_PROTOCOL_DATA_START+2+i];
				notify_others(buf,MapNotifyCmdId(cmdID),buf1,2+GROUP_NAME_BYTES);
			}
		}
		break;

	case 0x0030://endDevice control
		{
			////in:		addr[2],cmds[2],data[...]
			////out:	errCode[2]
			//unsigned char* addr = buf+dat_p+TCP_PROTOCOL_DATA_START+0;
			uint16 addrIndex = (buf[dat_p+TCP_PROTOCOL_DATA_START]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			unsigned short cmd = (buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+0]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+1]);
			uint8 isFind = FALSE;
			if(addrIndex >= 0 && addrIndex < MAX_DEVICE_NUM-1)
			{
				if(IsAddrNotZero(itemInfo[addrIndex].addr ))
				{
					isFind = TRUE;
				}
			}
			switch(cmd)
			{
			case GENERICAPP_CMDID_LIGHT:
				{
					if(isFind)
					{
						itemInfo[addrIndex].turnOn = buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+2];
						itemInfo[addrIndex].lightWhite = buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+3];
						itemInfo[addrIndex].lightR = buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+4];
						itemInfo[addrIndex].lightG = buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+5];
						itemInfo[addrIndex].lightB = buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+6];
						timeSaveItemPage[addrIndex/EEPROM_ItemInfoCntPerPage] = configInfo.time + EEPROM_DelaySaveItemInfo;

						buf1[TCP_PROTOCOL_DATA_START+0] = addrIndex>>8;
						buf1[TCP_PROTOCOL_DATA_START+1] = addrIndex;
						buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+0] = cmd>>8;
						buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+1] = cmd;
						for(i=0;i<5;i++)
							buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+2+i] = buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+2+i];
						notify_others(buf,MapNotifyCmdId(cmdID),buf1,7+INDEX_LEN);
					}
				}
				break;

			}
			//uart4_send(buf+dat_p,frameLen);
			if(isFind)
			{
				for(i=dataLen;i>0;i--)
					buf[dat_p+TCP_PROTOCOL_DATA_START+i+ADDR_LEN-INDEX_LEN-1] = buf[dat_p+TCP_PROTOCOL_DATA_START+i-1];
				for(i=0;i<ADDR_LEN;i++)
					buf[dat_p+TCP_PROTOCOL_DATA_START+i] = itemInfo[addrIndex].addr[i];
				UartControlSend(buf+dat_p+TCP_PROTOCOL_CMDID_START-UART_PROTOCOL_CMDID_START,frameLen-5+ADDR_LEN-INDEX_LEN);
			}
		}
		break;
	case 0x0040://group control
		{
			////in:		group id[2],subGroup id[2],cmds[2],data[...]
			////out:	errCode[2]
			unsigned short groupId = (buf[dat_p+TCP_PROTOCOL_DATA_START]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			unsigned short subGroupId = (buf[dat_p+TCP_PROTOCOL_DATA_START+2]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+3]);
			unsigned short cmd = (buf[dat_p+TCP_PROTOCOL_DATA_START+4]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+5]);
			switch(cmd)
			{
			case GENERICAPP_CMDID_LIGHT:
				{
					for(i=0;i<MAX_DEVICE_NUM-1;i++)
					{
						if(IsAddrNotZero(itemInfo[i].addr ) && itemInfo[i].groupId == groupId && itemInfo[i].subGroupId == subGroupId)
						{
							itemInfo[i].turnOn = buf[dat_p+TCP_PROTOCOL_DATA_START+6];
							itemInfo[i].lightWhite = buf[dat_p+TCP_PROTOCOL_DATA_START+7];
							itemInfo[i].lightR = buf[dat_p+TCP_PROTOCOL_DATA_START+8];
							itemInfo[i].lightG = buf[dat_p+TCP_PROTOCOL_DATA_START+9];
							itemInfo[i].lightB = buf[dat_p+TCP_PROTOCOL_DATA_START+10];
							timeSaveItemPage[i/EEPROM_ItemInfoCntPerPage] = configInfo.time + EEPROM_DelaySaveItemInfo;

							mSendControl[i].sendToUart = 1;
						}
					}

					buf1[TCP_PROTOCOL_DATA_START+0] = groupId>>8;
					buf1[TCP_PROTOCOL_DATA_START+1] = groupId;
					buf1[TCP_PROTOCOL_DATA_START+2] = subGroupId>>8;
					buf1[TCP_PROTOCOL_DATA_START+3] = subGroupId;
					buf1[TCP_PROTOCOL_DATA_START+4] = cmd>>8;
					buf1[TCP_PROTOCOL_DATA_START+5] = cmd;
					for(i=0;i<5;i++)
						buf1[TCP_PROTOCOL_DATA_START+6+i] = buf[dat_p+TCP_PROTOCOL_DATA_START+6+i];
					notify_others(buf,MapNotifyCmdId(cmdID),buf1,11);
				}
				break;

			}
			//uart4_send(buf+dat_p,frameLen);
			//UartControlSend(buf+dat_p+TCP_PROTOCOL_CMDID_START-UART_PROTOCOL_CMDID_START,frameLen-5);
		}
		break;
	case 0x0050://broadcast control
		{
			////in:		cmds[2],data[...]
			////out:	errCode[2]	
			unsigned short cmd = (buf[dat_p+TCP_PROTOCOL_DATA_START]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			switch(cmd)
			{
			case GENERICAPP_CMDID_LIGHT:
				{
					for(i=0;i<MAX_DEVICE_NUM-1;i++)
					{
						if(IsAddrNotZero(itemInfo[i].addr ))
						{
							itemInfo[i].turnOn = buf[dat_p+TCP_PROTOCOL_DATA_START+2];
							itemInfo[i].lightWhite = buf[dat_p+TCP_PROTOCOL_DATA_START+3];
							itemInfo[i].lightR = buf[dat_p+TCP_PROTOCOL_DATA_START+4];
							itemInfo[i].lightG = buf[dat_p+TCP_PROTOCOL_DATA_START+5];
							itemInfo[i].lightB = buf[dat_p+TCP_PROTOCOL_DATA_START+6];
							timeSaveItemPage[i/EEPROM_ItemInfoCntPerPage] = configInfo.time + EEPROM_DelaySaveItemInfo;
						}
					}
					buf1[TCP_PROTOCOL_DATA_START+0] = cmd>>8;
					buf1[TCP_PROTOCOL_DATA_START+1] = cmd;
					for(i=0;i<5;i++)
						buf1[TCP_PROTOCOL_DATA_START+2+i] = buf[dat_p+TCP_PROTOCOL_DATA_START+2+i];
					notify_others(buf,MapNotifyCmdId(cmdID),buf1,7);
				}
				break;

			}
			//uart4_send(buf+dat_p,frameLen);
			UartControlSend(buf+dat_p+TCP_PROTOCOL_CMDID_START-UART_PROTOCOL_CMDID_START,frameLen-5);
		}
		break;
	case 0x0060://create subGroup,	return subGroup id
		{
			////in:		addr[2],group id[2],subGroup name[16]
			////out:	errCode[2],addr[2],group id[2],subGroupId[2]
			////other:		addr[2],group id[2],subGroup id[2],subGroup name[16]
			uint16 addrIndex = (buf[dat_p+TCP_PROTOCOL_DATA_START]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			uint16 groupId = (buf[dat_p+TCP_PROTOCOL_DATA_START+2]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+3]);
			//unsigned char* addr = buf+dat_p+TCP_PROTOCOL_DATA_START;
			unsigned short subGroupID = 1;
			unsigned short errCode = 0;
			int index = 0;
			////alloc new subGroup ID
			{
				//release old used subGroupID
				if(addrIndex >= 0 && addrIndex < MAX_DEVICE_NUM-1)
				{
					if(IsAddrNotZero(itemInfo[addrIndex].addr ))
					{
						//扫描生成该组下子组的使用情况
						memset(tempSubGroupIdUsed,0,sizeof(tempSubGroupIdUsed));
						for(i=0;i<MAX_DEVICE_NUM;i++)
							if(IsAddrNotZero(itemInfo[i].addr ) && itemInfo[i].groupId == itemInfo[addrIndex].groupId)
								tempSubGroupIdUsed[itemInfo[i].subGroupId]++;
						//if(itemInfo[addrIndex].subGroupId != 0)
						{
							tempSubGroupIdUsed[itemInfo[addrIndex].subGroupId]--;
							if(tempSubGroupIdUsed[itemInfo[addrIndex].subGroupId] == 0)
							{
								index = GetSubGroupMap(itemInfo[addrIndex].groupId,itemInfo[addrIndex].subGroupId);
								if(index != -1)
								{
									subGroupMapUsed[index] = 0;
									subGroupMap[index].groupId = 0;
									subGroupMap[index].subGroupId = 0;
									memset(buf1,0,sizeof(SUB_GROUP_NAME_BYTES));
									EE_ReadWriteData(EEPROM_AddrSubGroupName+(index/EEPROM_SubGroupNameCntPerPage)*PAGE_SIZE+index*SUB_GROUP_NAME_BYTES,buf1,SUB_GROUP_NAME_BYTES);
								}
							}
						}
						if(itemInfo[addrIndex].groupId != 0)
						{
							groupIdUsed[itemInfo[addrIndex].groupId]--;
							if(groupIdUsed[itemInfo[addrIndex].groupId] == 0)
							{
								memset(buf1,0,sizeof(GROUP_NAME_BYTES));
								EE_ReadWriteData(EEPROM_AddrGroupName+(itemInfo[addrIndex].groupId/EEPROM_GroupNameCntPerPage)*PAGE_SIZE+itemInfo[addrIndex].groupId*GROUP_NAME_BYTES,buf1,GROUP_NAME_BYTES);
							}
						}
					}
				}
				//扫描生成新组下子组的使用情况
				memset(tempSubGroupIdUsed,0,sizeof(tempSubGroupIdUsed));
				for(i=0;i<MAX_DEVICE_NUM;i++)
					if(IsAddrNotZero(itemInfo[i].addr ) && itemInfo[i].groupId == groupId)
						tempSubGroupIdUsed[itemInfo[i].subGroupId]++;
				for(i=0;i<MAX_SUB_GROUP_NUM-1;i++)
					if(tempSubGroupIdUsed[i] == 0)
					{
						subGroupID = i;
						tempSubGroupIdUsed[i] = 1;
						break;
					}
			}
			dbglog("alloc new subGroup ID=%d\r\n",subGroupID);
			for(i=1;i<MAX_DEVICE_NUM;i++)
				if(subGroupMapUsed[i] == 0)
				{
					index = i;
					subGroupMapUsed[i] = 1;
					subGroupMap[i].groupId = groupId;
					subGroupMap[i].subGroupId = subGroupID;
					break;
				}
			EE_ReadWriteData(EEPROM_AddrSubGroupName+(index/EEPROM_SubGroupNameCntPerPage)*PAGE_SIZE+index*SUB_GROUP_NAME_BYTES,buf+dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN,SUB_GROUP_NAME_BYTES);
			//memcpy(groupName[subGroupID],buf+dat_p+TCP_PROTOCOL_DATA_START+2,GROUP_NAME_BYTES);
			
			
			if(addrIndex >= 0 && addrIndex < MAX_DEVICE_NUM-1)
			{
				if(IsAddrNotZero(itemInfo[addrIndex].addr ))
				{
					itemInfo[addrIndex].groupId = groupId;
					itemInfo[addrIndex].subGroupId = subGroupID;
					timeSaveItemPage[addrIndex/EEPROM_ItemInfoCntPerPage] = configInfo.time + 0;// save immediately
				}
			}
			//backup the name
			for(i=0;i<SUB_GROUP_NAME_BYTES;i++)
				buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+4+i] = buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+2+i];
			//for(i=0;i<ADDR_LEN;i++)
			//	buf[dat_p+UART_PROTOCOL_DATA_START+i] = addr[i];
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+0] = GENERICAPP_CMDID_ADD_TO_GROUP;
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+1] = GENERICAPP_CMDID_ADD_TO_GROUP>>8;
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+2] = subGroupID;
			//buf[dat_p+UART_PROTOCOL_DATA_START+ADDR_LEN+3] = subGroupID>>8;
			//UartControlSendCmd(buf+dat_p,0x0030,ADDR_LEN+4);//add to a subGroup

			buf[dat_p+TCP_PROTOCOL_DATA_START+0] = errCode>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+1] = errCode;
			buf[dat_p+TCP_PROTOCOL_DATA_START+2+0] = addrIndex>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+2+1] = addrIndex;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+2] = groupId>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+3] = groupId;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+4] = subGroupID>>8;
			buf[dat_p+TCP_PROTOCOL_DATA_START+INDEX_LEN+5] = subGroupID;
			//dbglog("send test error code\r\n");
			TcpSendCmd(buf,dat_p,MapRetCmdId(cmdID),6+INDEX_LEN);// return the alloced subGroupID

			buf1[TCP_PROTOCOL_DATA_START+0] = addrIndex>>8;
			buf1[TCP_PROTOCOL_DATA_START+1] = addrIndex;
			buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+0] = groupId>>8;
			buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+1] = groupId;
			buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+2] = subGroupID>>8;
			buf1[TCP_PROTOCOL_DATA_START+INDEX_LEN+3] = subGroupID;
			//be setted already
			//for(i=0;i<GROUP_NAME_BYTES;i++)
			//	buf1[TCP_PROTOCOL_DATA_START+ADDR_LEN+4+i] = buf[dat_p+TCP_PROTOCOL_DATA_START+ADDR_LEN+i];
			notify_others(buf,MapNotifyCmdId(cmdID),buf1,4+INDEX_LEN + SUB_GROUP_NAME_BYTES);
		}
		break;
	case 0x0064://set subGroup name
		{
			////in:		group ID[2],subGroup ID[2],subGroup name[16]
			////out:	errCode[2]
			unsigned short groupID = (buf[dat_p+TCP_PROTOCOL_DATA_START+0]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+1]);
			unsigned short subGroupID = (buf[dat_p+TCP_PROTOCOL_DATA_START+2]<<8)|(buf[dat_p+TCP_PROTOCOL_DATA_START+3]);
			int index;
			index = GetSubGroupMap(groupID,subGroupID);
			if(index != -1)
			{
				EE_ReadWriteData(EEPROM_AddrSubGroupName+(index/EEPROM_SubGroupNameCntPerPage)*PAGE_SIZE+index*SUB_GROUP_NAME_BYTES,buf+dat_p+TCP_PROTOCOL_DATA_START+4,SUB_GROUP_NAME_BYTES);
				//memcpy(groupName[subGroupID],buf+dat_p+TCP_PROTOCOL_DATA_START+2,GROUP_NAME_BYTES);
				

				buf1[TCP_PROTOCOL_DATA_START+0] = groupID>>8;
				buf1[TCP_PROTOCOL_DATA_START+1] = groupID;
				buf1[TCP_PROTOCOL_DATA_START+2] = subGroupID>>8;
				buf1[TCP_PROTOCOL_DATA_START+3] = subGroupID;
				for(i=0;i<SUB_GROUP_NAME_BYTES;i++)
					buf1[TCP_PROTOCOL_DATA_START+4+i] = buf[dat_p+TCP_PROTOCOL_DATA_START+4+i];
				notify_others(buf,MapNotifyCmdId(cmdID),buf1,4+SUB_GROUP_NAME_BYTES);
			}
		}
		break;
	default:
		{
			dbglog("not supported\r\n");
		}
		break;
	}
}
//send when uart is idle
//must be placed after UartDeQueue routine,so can make sure it only send when is not busy
void SendControl()
{
	if( ! mUartResend.busy)
	{
		int i;
		int findIndex = -1;
		if(mSendIndex < 0 || mSendIndex >= MAX_DEVICE_NUM-1)
			mSendIndex = 0;
		for(i=mSendIndex;i<MAX_DEVICE_NUM-1;i++)
			if(mSendControl[i].sendToUart)
			{
				findIndex = i;
				break;
			}
		if(findIndex == -1)
			for(i=1;i<mSendIndex;i++)
				if(mSendControl[i].sendToUart)
				{
					findIndex = i;
					break;
				}
		if(findIndex != -1)
		{
			mSendControl[findIndex].sendToUart = 0;
			mSendIndex = (findIndex==MAX_DEVICE_NUM-2)?(findIndex+1):0;
			for(i=0;i<ADDR_LEN;i++)
				buf1[UART_PROTOCOL_DATA_START+i] = itemInfo[findIndex].addr[i];
			buf1[UART_PROTOCOL_DATA_START+ADDR_LEN+0] = GENERICAPP_CMDID_LIGHT>>8;
			buf1[UART_PROTOCOL_DATA_START+ADDR_LEN+1] = GENERICAPP_CMDID_LIGHT;
			buf1[UART_PROTOCOL_DATA_START+ADDR_LEN+2] = itemInfo[findIndex].turnOn;
			buf1[UART_PROTOCOL_DATA_START+ADDR_LEN+3] = itemInfo[findIndex].lightWhite;
			buf1[UART_PROTOCOL_DATA_START+ADDR_LEN+4] = itemInfo[findIndex].lightR;
			buf1[UART_PROTOCOL_DATA_START+ADDR_LEN+5] = itemInfo[findIndex].lightG;
			buf1[UART_PROTOCOL_DATA_START+ADDR_LEN+6] = itemInfo[findIndex].lightB;
			UartControlSendCmd(buf1,0x0030,ADDR_LEN+7);

		}
	}
}
char dhcp_step = 0;
int dhcp_time = 0;
uint8 isGetIp = 0;
#define DHCP_STEP_DISCOVER		0
#define DHCP_STEP_REQUEST		1
#define DHCP_STEP_OK			2
int simple_server(void)
{  
    unsigned int plen,dat_p,i1=0,payloadlen=0;
	int prevTime=-1,tmp;
	char ledCtrl = 0;
	int i;

	//uint8 test23[] = {0,1,2,3,4,5,6,7,8,9};
	//dbglog("crc=%x",crc);
   
	init_ip_arp_udp_tcp(mymac,myip,mywwwport);
    dbglog("         MAC地址:0x%x,0x%x,0x%x,0x%x,0x%x,0x%x\r\n",mymac[0],mymac[1],mymac[2],mymac[3],mymac[4],mymac[5]);
    dbglog("         IP地址:%d.%d.%d.%d\r\n",myip[0],myip[1],myip[2],myip[3]);
    dbglog("         端口号:%d\r\n",mywwwport);



	////read info/////////////////////////////////////////////////////////
	EE_Init();
	{
		//check whether is first write
		uint8 isFirstWrite = 0;
		EE_ReadData(EEPROM_AddrConfig,(uint8*)&configInfo, sizeof(configInfo));
		if(strcmp(strMagic,configInfo.magic) == 0)
			isFirstWrite = 0;
		else
			isFirstWrite = 1;
		if(isFirstWrite)
		{
			uint16 groupNamePages = EEPROM_PageNumGroupName;
			uint16 subGroupNamePages = EEPROM_PageNumSubGroupName;
			uint16 deviceNamePages = EEPROM_PageNumDeviceName;
			uint16 itemInfoPages = EEPROM_PageNumItemInfo;
			memset(&configInfo,0,sizeof(configInfo));
			strcpy(configInfo.magic,strMagic);
			////write zero to all area except magic string
			EE_WriteData(EEPROM_AddrConfig,(uint8*)&configInfo,sizeof(configInfo));
			memset(bufFlash,0,WRITE_SIZE);
			for(i=0;i<groupNamePages;i++)
			{
				memset(bufFlash,0,WRITE_SIZE);
				EE_WriteData(EEPROM_AddrGroupName+i*PAGE_SIZE,bufFlash,WRITE_SIZE);
			}
			for(i=0;i<deviceNamePages;i++)
			{
				memset(bufFlash,0,WRITE_SIZE);
				EE_WriteData(EEPROM_AddrDeviceName+i*PAGE_SIZE,bufFlash,WRITE_SIZE);
			}
			for(i=0;i<itemInfoPages;i++)
			{
				memset(bufFlash,0,WRITE_SIZE);
				EE_WriteData(EEPROM_AddrItemInfo+i*PAGE_SIZE,bufFlash,WRITE_SIZE);
			}
			for(i=0;i<subGroupNamePages;i++)
			{
				memset(bufFlash,0,WRITE_SIZE);
				EE_WriteData(EEPROM_AddrSubGroupName+i*PAGE_SIZE,bufFlash,WRITE_SIZE);
			}
		}
		else
		{
			//uint16 groupNamePages = EEPROM_PageNumGroupName;
			uint16 itemInfoPages = EEPROM_PageNumItemInfo;
			EE_ReadData(EEPROM_AddrConfig,(uint8*)&configInfo,sizeof(configInfo));
			for(i=0;i<itemInfoPages;i++)
				EE_ReadData(EEPROM_AddrItemInfo+i*PAGE_SIZE,(uint8*)&itemInfo[i*EEPROM_ItemInfoCntPerPage], EEPROM_ItemInfoCntPerPage*sizeof(itemInfo[0]));
			for(i=1;i<MAX_DEVICE_NUM;i++)
				if(IsAddrNotZero(itemInfo[i].addr ))
				{
					if(itemInfo[i].groupId != 0)
					{
						groupIdUsed[itemInfo[i].groupId]++;
						break;
					}
				}
			for(i=1;i<MAX_DEVICE_NUM;i++)
				if(IsAddrNotZero(itemInfo[i].addr ))
				{
					if(itemInfo[i].subGroupId != 0)
					{
						tempSubGroupIdUsed[itemInfo[i].subGroupId]++;
						break;
					}
				}
		}
	}
	timeSaveConfig = configInfo.time + EEPROM_DelaySaveConfig;
	//test2();

	
	#if 0//defined(DEBUG_CH)
	{
		dbglog("group name=\r\n");
		for(i=0;i<MAX_GROUP_NUM;i++)
		{
			EE_ReadData(EEPROM_AddrGroupName+(i/EEPROM_GroupNameCntPerPage)*PAGE_SIZE+i*GROUP_NAME_BYTES,buf1,GROUP_NAME_BYTES);
			buf1[GROUP_NAME_BYTES] = 0;
			dbglog("group[%d]=%s\r\n",i,buf1);
		}
	}
	#endif
	#if 1//defined(DEBUG_CH)
	PrintDeviceList(buf1);
	#endif

	#if 1
	dbglog("EEPROM_AddrItemInfo=%x\r\n",EEPROM_AddrItemInfo);
	dbglog("EEPROM_ItemInfoCntPerPage=%d\r\n",EEPROM_ItemInfoCntPerPage);
	dbglog("EEPROM_PageNumItemInfo=%d\r\n",EEPROM_PageNumItemInfo);
	dbglog("EEPROM_AddrGroupName=%x\r\n",EEPROM_AddrGroupName);
	dbglog("EEPROM_GroupNameCntPerPage=%d\r\n",EEPROM_GroupNameCntPerPage);
	dbglog("EEPROM_PageNumGroupName=%d\r\n",EEPROM_PageNumGroupName);
	dbglog("EEPROM_AddrDeviceName=%x\r\n",EEPROM_AddrDeviceName);
	dbglog("EEPROM_DeviceNameCntPerPage=%d\r\n",EEPROM_DeviceNameCntPerPage);
	dbglog("EEPROM_PageNumDeviceName=%d\r\n",EEPROM_PageNumDeviceName);

	#endif

	RTC_Config();


	//init the ethernet/ip layer:
    while(1)
    {
		tmp = configInfo.time / 1000;
		if(prevTime != tmp)
		{
			prevTime = tmp;
			LED_Set(LED2,ledCtrl);
			ledCtrl = ! ledCtrl;

			RTC_Get(&timer);
			//dbglog("rtc=%d\r\n",timer.secondsCount);
			//dbglog("timer=%d,%d,%d-%d:%d:%d\r\n",timer.w_year,timer.w_month,timer.w_date,timer.hour,timer.min,timer.sec);
			for(i=0;i<MAX_DEVICE_NUM;i++)
				if(timer.secondsCount > itemInfo[i].nextAlarm)
					CheckStatusAndSetNextAlarm(i);

			//dbglog("time=%d\r\n",configInfo.time);
		}
		for(i=0;i<DIMM(timeSaveItemPage);i++)
			if(timeSaveItemPage[i] != 0)
			{
				if(configInfo.time > timeSaveItemPage[i])
				{
					EE_WriteData(EEPROM_AddrItemInfo+(i)*PAGE_SIZE,(uint8*)&itemInfo[i * EEPROM_ItemInfoCntPerPage],EEPROM_ItemInfoCntPerPage*sizeof(ITEM_INFO));
					timeSaveItemPage[i] = 0;
				}
			}
		if(timeSaveConfig != 0)
		{
			if(configInfo.time > timeSaveConfig)
			{
				EE_WriteData(EEPROM_AddrConfig,(uint8*)&configInfo,sizeof(configInfo));
				timeSaveConfig = configInfo.time + EEPROM_DelaySaveConfig;
			}
		}
		/*
		////uart receive process
		if(configInfo.time > RxTime + 5)
		{
			if(RxCounter1)
			{
				enc28j60PacketSend(RxCounter1,RxBuffer1);
				RxCounter1 = 0;
			}
		}
		*/
		if(RxCounter2 > 0 && configInfo.time > RxTime2 + 5)
		{
			RxCounter2Copy = RxCounter2;
			memcpy(RxBuffer2Copy,RxBuffer2,RxCounter2Copy);
			RxCounter2 = 0;
			#if defined(DEBUG_CH)
			{
				dbglog("usart2 received=%d\r\n",RxCounter2Copy);
				for(i=0;i<RxCounter2Copy;i++)
				{
					dbglog("%02x ",RxBuffer2Copy[i]);
					if(i%8 == 7)
						dbglog(" ");
					if(i%16 == 15)
						dbglog("\r\n");
				}
				dbglog("\r\n");
			}
			#endif
			//check valid
			{
				int dataLen;
				i = 0;
				while(i<RxCounter2Copy)
					if(RxBuffer2Copy[i] == 0xAA)
					{
						dataLen = RxBuffer2Copy[i+1];
						if(i+3+dataLen<RxCounter2Copy && RxBuffer2Copy[i+3+dataLen] == 0x55)
						{
							if(i == 0)
								dbglog("Valid uasrt frame\r\n");
							else
								dbglog("Valid uasrt frame more\r\n");
							memcpy(buf2,RxBuffer2Copy+i,4+dataLen);
							//ProcessTcpMsg(buf2,0);
							ProcessUartMsg(buf2,4+dataLen);
							i += 4+dataLen;
						}
						else
							i++;
					}
					else
						i++;
			}
			//ProcessUartMsg(RxBuffer2Copy,RxCounter2Copy);
		}
		{
			//tcp timeout resend
			for(i=0;i<MAX_TCP_RESEND_FRAME;i++)
				if(TcpTimeout[i].isUsed)
				{
					//dbglog("t=%d timeResend[%d]=%d\r\n",configInfo.time,i,TcpTimeout[i].timeResend);
					if(configInfo.time > TcpTimeout[i].timeResend  )
					{
						dbglog("tcp[%d] resend(%d)\r\n",i,TcpTimeout[i].cntResend);
						enc28j60PacketSend(TcpTimeout[i].len,TcpTimeout[i].buf);
						TcpTimeout[i].timeResend += 1000;
						TcpTimeout[i].cntResend++;
						if(TcpTimeout[i].cntResend>3)
						{
							TcpTimeout[i].isUsed = FALSE;
							dbglog("stop resend tcp\r\n");
						}
					}
				}
		}
		{
			//uart queue send
			if( ! mUartResend.busy)
			{
				UartDeQueue();
			}
		}
		{
			//uart timeout resend
			if(mUartResend.busy)
			{
				//dbglog("t=%d timeResend[%d]=%d\r\n",configInfo.time,i,mUartSendQueue[i].timeResend);
				if(configInfo.time > mUartResend.timeResend  )
				{
					dbglog("uart resend(%d)\r\n",mUartResend.cntResend);
					uart4_send(mUartResend.buf,mUartResend.len);
					mUartResend.timeResend += UART_RESEND_TIME;
					mUartResend.cntResend++;
					if(mUartResend.cntResend>3)
					{
						mUartResend.busy = FALSE;
						dbglog("stop resend uart\r\n");
					}
				}
			}
		}
		
		SendControl();

		#if defined(USE_DHCP)
		if(dhcp_step == DHCP_STEP_DISCOVER && dhcp_time < configInfo.time)
		{
			////start DHCP discover
			make_udp_hdcp_discover(buf);
			
			dhcp_time = configInfo.time + 4000;
		}
		else if(dhcp_step == DHCP_STEP_REQUEST && dhcp_time < configInfo.time)
		{
			make_udp_hdcp_request(buf);
			dhcp_time = configInfo.time + 4000;
		}
		#endif
		
        //判断是否有接收到有效的包
        plen = enc28j60PacketReceive(BUFFER_SIZE, buf);
        //如果收到有效的包，plen将为非0值。
        if(plen==0)
        {
            continue; //没有收到有效的包就退出重新检测
        }
		//if(isGarbage(buf))
		//	continue;
		#if defined(TEST_DHCP) || 0//defined(DEBUG_CH)
		{
			dbglog("received=%d\r\n",plen);
			for(i=0;i<plen;i++)
			{
				dbglog("%02x ",buf[i]);
				if(i%8 == 7)
					dbglog(" ");
				if(i%16 == 15)
					dbglog("\r\n");
			}
			dbglog("\r\n");
			//continue;
		}
		#endif
		#if defined(USE_DHCP)
		if(dhcp_step != DHCP_STEP_OK )
		{
		    //UDP包
			if (buf[IP_PROTO_P]==IP_PROTO_UDP_V)
			{
				uint16 udpPort = ((buf[UDP_DST_PORT_H_P]<<8)|buf[UDP_DST_PORT_L_P]);
				dbglog("包类型 UDP\r\n");
				//监听1200端口的UDP包
				if(udpPort == 68)
				{
					if(dhcp_step == 0)
					{
						if(receive_udp_hdcp_offer(buf,plen))
						{
							dbglog("get dhcp offer\r\n");
							dhcp_step = DHCP_STEP_REQUEST;
							dhcp_time = configInfo.time;
						}
					}
					else if(dhcp_step == DHCP_STEP_REQUEST)
					{
						uint8 ackType = receive_udp_hdcp_ack(buf,plen);
						if(ackType == 1)
						{
							dbglog("get dhcp ack\r\n");
							dhcp_step = DHCP_STEP_OK;
							dhcp_time = configInfo.time;

							isGetIp = TRUE;
							SetLeaseTime(prevTime);
							getMyIp(myip);
							dbglog("myip=%d.%d.%d.%d\r\n",myip[0],myip[1],myip[2],myip[3]);
						}
						else if(ackType == 2)
						{
							//restart whole dhcp process
							dbglog("get dhcp nack\r\n");
							dhcp_step = DHCP_STEP_DISCOVER;
							dhcp_time = configInfo.time;
						}
					}
				}
			}

		}
		else
		{
			// check ip lease time
			if(CheckLeaseTime(prevTime) != 0)
			{
				dhcp_step = DHCP_STEP_REQUEST;
				dhcp_time = configInfo.time;
			}
		}
		if( ! isGetIp)
			continue;
		#endif
		//当收到目的地址为本机IP的ARP包时，发出ARP相应包
        if(eth_type_is_arp_and_my_ip(buf,plen))
        {
			make_arp_answer_from_request(buf);
            continue;
        }
        
        //判断是否接收到目的地址为本机IP的合法的IP包
        if(eth_type_is_ip_and_my_ip(buf,plen)==0 && ! is_ip_multibroadcast(buf)) 
        {
            continue;
        }
        //如果收到ICMP包
        if(buf[IP_PROTO_P]==IP_PROTO_ICMP_V && buf[ICMP_TYPE_P]==ICMP_TYPE_ECHOREQUEST_V)
        {

            dbglog("收到主机[%d.%d.%d.%d]发送的ICMP包\r\n",buf[ETH_ARP_SRC_IP_P],buf[ETH_ARP_SRC_IP_P+1],
                                                           buf[ETH_ARP_SRC_IP_P+2],buf[ETH_ARP_SRC_IP_P+3]);
			make_echo_reply_from_request(buf, plen);
			continue;
        }
		#if 0//defined(DEBUG_CH)
		{
			dbglog("received=%d\r\n",plen);
			for(i=0;i<plen;i++)
			{
				dbglog("%02x ",buf[i]);
				if(i%8 == 7)
					dbglog(" ");
				if(i%16 == 15)
					dbglog("\r\n");
			}
			dbglog("\r\n");
			//continue;
		}
		#endif
        
        //如果收到TCP包，且端口为80
		if (buf[IP_PROTO_P]==IP_PROTO_TCP_V
			&&((buf[TCP_DST_PORT_H_P]<<8) | buf[TCP_DST_PORT_L_P])==mywwwport)
		{
			#if 1 && defined(DEBUG_CH) && ! defined(TEST_DHCP)
			{
				dbglog("received=%d\r\n",plen);
				for(i=0;i<plen;i++)
				{
					dbglog("%02x ",buf[i]);
					if(i%8 == 7)
						dbglog(" ");
					if(i%16 == 15)
						dbglog("\r\n");
				}
				dbglog("\r\n");
				//continue;
			}
			#endif
		    dbglog("收到TCP包,ip=%02x,port=%02x%02x\r\n",buf[IP_SRC_P+3],buf[TCP_SRC_PORT_H_P],buf[TCP_SRC_PORT_H_P+1]);
            if (buf[TCP_FLAGS_P] & TCP_FLAGS_SYN_V)
			{
			    dbglog("包类型为SYN\r\n");
				new_tcp_seq(buf);
                make_tcp_synack_from_syn(buf);
                continue;
            }
	        if ((buf[TCP_FLAGS_P] & TCP_FLAGS_ACK_V)
				|| (buf[TCP_FLAGS_P] & TCP_FLAGS_RST_V)
				)
			{
	            init_len_info(buf); // init some data structures
	            dat_p=get_tcp_data_pointer();
				if( ! TcpCheckSumOk(buf))
				{
					dbglog("TCP校验错误，丢弃该帧\r\n");
					continue;
				}
				{
					unsigned int seq; 
					uint32 ip;
					uint16 port;
		    		//dbglog("包类型为ACK\r\n");
					seq = get_tcp_ack_seq(buf);
					ip = get_ip_src(buf);
					port = get_port_src(buf);
					//dbglog("ack seq=%d\r\n",seq);
					//dbglog("ack ip=%x\r\n",ip);
					for(i=0;i<MAX_TCP_RESEND_FRAME;i++)
					{
						//if(TcpTimeout[i].isUsed)
						//	dbglog("TcpTimeout[%d].seq=%d\r\n",i,TcpTimeout[i].seq);
						if(TcpTimeout[i].isUsed && TcpTimeout[i].seq == seq && TcpTimeout[i].ip == ip && TcpTimeout[i].port == port)
						{
							TcpTimeout[i].isUsed = FALSE;
						}
					}
				}
	            if (dat_p==0)
				{
	                if (buf[TCP_FLAGS_P] & TCP_FLAGS_FIN_V)
					{
			    		dbglog("包类型为FINISH\r\n");
	                    make_tcp_ack_from_any(buf);/*发送响应*/
						delete_tcp_seq(buf);
	                }
					//收到纯ack包
					//else
					//	make_tcp_ack_from_any(buf);/*发送响应*/
	                // 发送一个没有数据的ACK响应，等待下一个包
	                continue;
	            }
		    	dbglog("包类型为data[%d]=\r\n",dat_p);
				for(i=dat_p;i<plen;i++)
				{
					dbglog("%02x ",buf[i]);
					if((i-dat_p)%8 == 7)
						dbglog(" ");
					if((i-dat_p)%16 == 15)
						dbglog("\r\n");
				}
				dbglog("\r\n");
				//if (strncmp("GET ",(char *)&(buf[dat_p]),4)!=0)
				//{
			        // 如果是Telnet方式登录，返回如下提示信息
			        //plen=fill_tcp_data_p(buf,0,PSTR("神舟III号\r\n\n\rHTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>200 OK</h1>"));
			        //goto SENDTCP;
				//}
				make_tcp_ack_from_any(buf); // send ack for http get
				//test
				//notify change
				
				//check valid
				{
					int dataLen;
					i = dat_p;
					while(i<plen)
						if(buf[i] == 0xAA)
						{
							dataLen = (buf[i+1]<<8)|(buf[i+2]);
							if(i+4+dataLen<plen && buf[i+4+dataLen] == 0x55)
							{
								if(i == dat_p)
									dbglog("Valid frame\r\n");
								else
									dbglog("Valid frame more\r\n");
								memcpy(buf2,buf,dat_p);
								memcpy(buf2+dat_p,buf+i,5+dataLen);
								ProcessTcpMsg(buf2,dat_p);
								i += 5+dataLen;
							}
							else
								i++;
						}
						else
							i++;
				}
				
			}
			else
				dbglog("包类型 not TCP_FLAGS_ACK_V\r\n");
		}

	    //UDP包
		if (buf[IP_PROTO_P]==IP_PROTO_UDP_V)
		{
			uint16 udpPort = ((buf[UDP_DST_PORT_H_P]<<8)|buf[UDP_DST_PORT_L_P]);
			dbglog("包类型 UDP\r\n");
			//监听1200端口的UDP包
			if(udpPort == 1200)
			{
				payloadlen=	  buf[UDP_LEN_H_P];
				payloadlen=payloadlen<<8;
				payloadlen=(payloadlen+buf[UDP_LEN_L_P])-UDP_HEADER_LEN;
		
	            //ANSWER
	            for(i1=0; i1<payloadlen; i1++)
	            {         
	                buf1[i1]=buf[UDP_DATA_P+i1];
	            }
				
				//make_udp_reply_from_request(buf,str,strlen(str),myudpport);
				make_udp_reply_from_request(buf,buf1,payloadlen,0);
			}
			else if(udpPort == 13502)
			{
				if(is_ip_multibroadcast(buf))
				{
					dbglog("包类型 UDP 多播\r\n");
					//
					{
						uint16 destPort = (buf[UDP_DATA_P+0]<<8)|(buf[UDP_DATA_P+1]);
						dbglog("手机监听UDP端口为=%d\r\n",destPort);
						dbglog("myip=%d.%d.%d.%d\r\n",myip[0],myip[1],myip[2],myip[3]);
						for(i=0;i<4;i++)
							buf1[i] = myip[i];
						//getMyIp(UDP_DATA_P);
						make_udp_reply_from_request(buf,myip,4,destPort);
						//DelayMs(1000);
					}
				}
			}
		}
	}
}
#if THE_END
#endif
