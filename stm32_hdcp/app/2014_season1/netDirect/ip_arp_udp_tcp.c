
#include "global/global.h"
//#include <includes.h>
#include "net.h"
#include "ip_arp_udp_tcp.h"
#include "enc28j60.h"
#include "stdio.h"
#include "simple_server.h"


#define  pgm_read_byte(ptr)  ((char)*(ptr))

//#define unsigned char  unsigned char
//#define unsigned  int unisgned int

static unsigned short wwwport;
static unsigned char macaddr[6];
static unsigned char ipaddr[4];
static unsigned int info_hdr_len=0;
static unsigned int info_data_len=0;
unsigned int submask=0;
unsigned int router=0;
unsigned int leaseTime=0;
unsigned int dhcpServerIp=0;
unsigned int dnsServer1=0;
unsigned int dnsServer2=0;
//unsigned int seqnum=0xa; // my initial tcp sequence number


LINKED_INFO linkedInfo[MAX_LINK] = {0};
// The Ip checksum is calculated over the ip header only starting
// with the header length field and a total length of 20 bytes
// unitl ip.dst
// You must set the IP checksum field to zero before you start
// the calculation.
// len for ip is 20.
//
// For UDP/TCP we do not make up the required pseudo header. Instead we 
// use the ip.src and ip.dst fields of the real packet:
// The udp checksum calculation starts with the ip.src field
// Ip.src=4bytes,Ip.dst=4 bytes,Udp header=8bytes + data length=16+len
// In other words the len here is 8 + length over which you actually
// want to calculate the checksum.
// You must set the checksum field to zero before you start
// the calculation.
// len for udp is: 8 + 8 + data length
// len for tcp is: 4+4 + 20 + option len + data length
//
// For more information on how this algorithm works see:
// http://www.netfor2.com/checksum.html
// http://www.msc.uky.edu/ken/cs471/notes/chap3.htm
// The RFC has also a C code example: http://www.faqs.org/rfcs/rfc1071.html
unsigned  int checksum(unsigned char *buf, unsigned  int len,unsigned char type)
{
	// type 0=ip 
	//      1=udp
	//      2=tcp
	unsigned long sum = 0;
	
	//if(type==0){
	//        // do not add anything
	//}
	if(type==1)
	{
		sum+=IP_PROTO_UDP_V; // protocol udp
		// the length here is the length of udp (data+header len)
		// =length given to this function - (IP.scr+IP.dst length)
		sum+=len-8; // = real tcp len
	}
	if(type==2)
	{
		sum+=IP_PROTO_TCP_V; 
		// the length here is the length of tcp (data+header len)
		// =length given to this function - (IP.scr+IP.dst length)
		sum+=len-8; // = real tcp len
	}
	// build the sum of 16bit words
	while(len >1)
	{
		sum += 0xFFFF & (*buf<<8|*(buf+1));
		buf+=2;
		len-=2;
	}
	// if there is a byte left then add it (padded with zero)
	if (len)
	{
		sum += (0xFF & *buf)<<8;
	}
	// now calculate the sum over the bytes in the sum
	// until the result is only 16bit long
	while (sum>>16)
	{
		sum = (sum & 0xFFFF)+(sum >> 16);
	}
	// build 1's complement:
	return( (unsigned  int) sum ^ 0xFFFF);
}

// you must call this function once before you use any of the other functions:
void init_ip_arp_udp_tcp(unsigned char *mymac,unsigned char *myip,unsigned short wwwp)
{
	unsigned char i=0;
	wwwport=wwwp;
	while(i<4)
	{
        ipaddr[i]=myip[i];
        i++;
	}
	i=0;
	while(i<6)
	{
        macaddr[i]=mymac[i];
        i++;
	}
}
#if defined(TEST_DHCP)
/*----------------------------------------------------------------------------------
当收到目的IP为本机IP的ARP包时，返回值为1，否则返回0
-----------------------------------------------------------------------------------*/
unsigned char eth_type_is_arp_and_dhcp(unsigned char *buf,unsigned  int len)
{
	unsigned char i=0;
    //包长度不够，直接返回
	if (len<41)
	{
	    return(0);
	}

    //如果类型不是ARP包，直接返回
	if(buf[ETH_TYPE_H_P] != ETHTYPE_ARP_H_V || buf[ETH_TYPE_L_P] != ETHTYPE_ARP_L_V
		 || buf[ETH_ARP_OPCODE_H_P] != 0x00 || buf[ETH_ARP_OPCODE_L_P] != 0x01)
	{
	    return(0);
	}

    //如果ARP包的IP地址与本机IP不一致，直接返回
	while(i<4)
	{
	    if(buf[ETH_ARP_DST_IP_P+i] != 0)
		{
	        //return(0);
	    }
	    i++;
	}
    dbglog("收到主机[%d.%d.%d.%d]发送的DHCP ARP包\r\n",buf[ETH_ARP_SRC_IP_P],buf[ETH_ARP_SRC_IP_P+1],buf[ETH_ARP_SRC_IP_P+2],buf[ETH_ARP_SRC_IP_P+3]);
    
	return(1);
}
#endif
unsigned char isGarbage(unsigned char *buf)
{
	int i;
	uint8 isValid = TRUE;
    for(i=0; i<6; i++)
    	if(buf[ETH_DST_MAC+i] != 0xff)
    	{
			isValid = FALSE;
			break;
    	}
	if( ! isValid)
	{
		isValid = TRUE;
	    for(i=0; i<6; i++)
	    	if(buf[ETH_DST_MAC+i] != macaddr[i])
	    	{
				isValid = FALSE;
				break;
	    	}
	}
	return  ! isValid;
}
/*----------------------------------------------------------------------------------
当收到目的IP为本机IP的ARP包时，返回值为1，否则返回0
-----------------------------------------------------------------------------------*/
unsigned char eth_type_is_arp_and_my_ip(unsigned char *buf,unsigned  int len)
{
	unsigned char i=0;
    //包长度不够，直接返回
	if (len<41)
	{
	    return(0);
	}

    //如果类型不是ARP包，直接返回
	if(buf[ETH_TYPE_H_P] != ETHTYPE_ARP_H_V || buf[ETH_TYPE_L_P] != ETHTYPE_ARP_L_V
		 || buf[ETH_ARP_OPCODE_H_P] != 0x00 || buf[ETH_ARP_OPCODE_L_P] != 0x01)
	{
	    return(0);
	}

    //如果ARP包的IP地址与本机IP不一致，直接返回
	while(i<4)
	{
	    if(buf[ETH_ARP_DST_IP_P+i] != ipaddr[i])
		{
	        return(0);
	    }
	    i++;
	}
    dbglog("收到主机[%d.%d.%d.%d]发送的ARP包\r\n",buf[ETH_ARP_SRC_IP_P],buf[ETH_ARP_SRC_IP_P+1],buf[ETH_ARP_SRC_IP_P+2],buf[ETH_ARP_SRC_IP_P+3]);
    
	return(1);
}

unsigned char eth_type_is_ip_and_my_ip(unsigned char *buf,unsigned  int len)
{
	unsigned char i=0;
    //包长度不够，直接返回
	if (len<42)
	{
	    return(0);
	}
    
    //如果包类型不是IP包，直接返回
	if(buf[ETH_TYPE_H_P]!=ETHTYPE_IP_H_V || buf[ETH_TYPE_L_P]!=ETHTYPE_IP_L_V)
	{

        return(0);
	}
    //如果长度参数不正确，直接返回
	if (buf[IP_HEADER_LEN_VER_P]!=0x45)
	{
	    // must be IP V4 and 20 byte header
	    return(0);
	}
    
    //如果IP包的IP地址与本机IP不一致，直接返回 
	while(i<4)
	{
	    if(buf[IP_DST_P+i]!=ipaddr[i])
		{
	        return 0;
	    }
	    i++;
	}
	return(1);
}
unsigned char is_ip_multibroadcast(unsigned char *buf)
{
	uint8 isValid = TRUE;
	isValid = TRUE;
	//multibroadcast from 224.0.0.0~224.0.0.255
	if(buf[IP_DST_P+0] != 224
		|| buf[IP_DST_P+1] != 0
		|| buf[IP_DST_P+2] != 0
		)
		isValid = FALSE;
	return(isValid);
}
// make a return eth header from a received eth packet
void make_eth(unsigned char *buf)
{
	unsigned char i=0;

	//填写包的目的MAC地址，以及源MAC地址
	while(i<6)
	{
        buf[ETH_DST_MAC +i]=buf[ETH_SRC_MAC +i];
        buf[ETH_SRC_MAC +i]=macaddr[i];
        i++;
	}
}
void fill_ip_hdr_checksum(unsigned char *buf)
{
	unsigned  int ck;
	// clear the 2 byte checksum
	buf[IP_CHECKSUM_P]=0;
	buf[IP_CHECKSUM_P+1]=0;
	buf[IP_FLAGS_P]=0x40; // don't fragment
	buf[IP_FLAGS_P+1]=0;  // fragement offset
	buf[IP_TTL_P]=64; // ttl
	// calculate the checksum:
	ck=checksum(&buf[IP_P], IP_HEADER_LEN,0);
	buf[IP_CHECKSUM_P]=ck>>8;
	buf[IP_CHECKSUM_P+1]=ck& 0xff;
}

// make a return ip header from a received ip packet
void make_ip(unsigned char *buf)
{
	unsigned char i=0;
	while(i<4)
	{
        buf[IP_DST_P+i]=buf[IP_SRC_P+i];
        buf[IP_SRC_P+i]=ipaddr[i];
        i++;
	}
	fill_ip_hdr_checksum(buf);
}

// make a return tcp header from a received tcp packet
// rel_ack_num is how much we must step the seq number received from the
// other side. We do not send more than 255 bytes of text (=data) in the tcp packet.
// If mss=1 then mss is included in the options list
//
// After calling this function you can fill in the first data byte at TCP_OPTIONS_P+4
// If cp_seq=0 then an initial sequence number is used (should be use in synack)
// otherwise it is copied from the packet we received
void make_tcphead(unsigned char *buf,unsigned  int rel_ack_num,unsigned char mss,unsigned char cp_seq)
	{
	unsigned char i=0;
	unsigned char tseq;
	while(i<2)
	{
	    buf[TCP_DST_PORT_H_P+i]=buf[TCP_SRC_PORT_H_P+i];
	    //buf[TCP_SRC_PORT_H_P+i]=0; // clear source port
	    i++;
	}
	// set source port  (http):
	buf[TCP_SRC_PORT_H_P] = wwwport>>8;
	buf[TCP_SRC_PORT_L_P] = wwwport;
	i=4;
	// sequence numbers:
	// add the rel ack num to SEQACK
	while(i>0)
	{
	    rel_ack_num=buf[TCP_SEQ_H_P+i-1]+rel_ack_num;
	    tseq=buf[TCP_SEQACK_H_P+i-1];
	    buf[TCP_SEQACK_H_P+i-1]=0xff&rel_ack_num;
	    if (cp_seq)
		{
	        // copy the acknum sent to us into the sequence number
	        buf[TCP_SEQ_H_P+i-1]=tseq;
		}
		else
		{
	        buf[TCP_SEQ_H_P+i-1]= 0; // some preset vallue
	    }
	    rel_ack_num=rel_ack_num>>8;
	    i--;
	}
	if (cp_seq==0)
	{
		uint32 seq;
		#if 0
	    // put inital seq number
	    buf[TCP_SEQ_H_P+0]= 0;
	    buf[TCP_SEQ_H_P+1]= 0;
	    // we step only the second byte, this allows us to send packts 
	    // with 255 bytes or 512 (if we step the initial seqnum by 2)
	    buf[TCP_SEQ_H_P+2]= seqnum; 
	    buf[TCP_SEQ_H_P+3]= 0;
		#else
		seq = get_tcp_seq(buf);
	    buf[TCP_SEQ_H_P+0]= seq>>24;
	    buf[TCP_SEQ_H_P+1]= seq>>16;
	    // we step only the second byte, this allows us to send packts 
	    // with 255 bytes or 512 (if we step the initial seqnum by 2)
	    buf[TCP_SEQ_H_P+2]= seq>>8; 
	    buf[TCP_SEQ_H_P+3]= seq;
		#endif
	    // step the inititial seq num by something we will not use
	    // during this tcp session:
	    //seqnum+=1;
	    add_tcp_seq(buf,1);
	}
	
	// zero the checksum
	buf[TCP_CHECKSUM_H_P]=0;
	buf[TCP_CHECKSUM_L_P]=0;
	
	// The tcp header length is only a 4 bit field (the upper 4 bits).
	// It is calculated in units of 4 bytes. 
	// E.g 24 bytes: 24/4=6 => 0x60=header len field
	//buf[TCP_HEADER_LEN_P]=(((TCP_HEADER_LEN_PLAIN+4)/4)) <<4; // 0x60
	if (mss)
	{
	    // the only option we set is MSS to 1408:
	    // 1408 in hex is 0x580
	    buf[TCP_OPTIONS_P]=2;
	    buf[TCP_OPTIONS_P+1]=4;
	    buf[TCP_OPTIONS_P+2]=0x05; 
	    buf[TCP_OPTIONS_P+3]=0x80;
	    // 24 bytes:
	    buf[TCP_HEADER_LEN_P]=0x60;
	}
	else
	{
	    // no options:
	    // 20 bytes:
	    buf[TCP_HEADER_LEN_P]=0x50;
	}
}
#if defined(TEST_DHCP)
void make_arp_answer_from_DHCP_request(unsigned char *buf)
{
	unsigned char i=0;
    
	//填写包的目的MAC地址以及源MAC地址	
	while(i<6)
	{
        buf[ETH_DST_MAC +i]=0xff;
        buf[ETH_SRC_MAC +i]=macaddr[i];
        i++;
	}
    
    //填写ARP响应包的类型
	buf[ETH_ARP_OPCODE_H_P]=ETH_ARP_OPCODE_REPLY_H_V;   //arp 响应
	buf[ETH_ARP_OPCODE_L_P]=0x01;

    //填写ARP包的目的MAC地址以及源MAC地址
    i=0;
	while(i<6)
	{
        buf[ETH_ARP_DST_MAC_P+i]=0;
        buf[ETH_ARP_SRC_MAC_P+i]=macaddr[i];
        i++;
	}

    //填写ARP包的目的IP地址以及源IP地址    
	i=0;
	while(i<4)
	{
        buf[ETH_ARP_DST_IP_P+i]=0;
        buf[ETH_ARP_SRC_IP_P+i]=0;
        i++;
	}

    dbglog("神舟III号[%d.%d.%d.%d]发送DHCP ARP响应\r\n",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3]);

    //发送ARP相应包
	//enc28j60PacketSend(42,buf); 
	//for(i=42;i<60;i++)
	//	buf[i] = 0;
	enc28j60PacketSend(60,buf); 
}
#endif
void make_arp_answer_from_request(unsigned char *buf)
{
	unsigned char i=0;
    
	//填写包的目的MAC地址以及源MAC地址	
	make_eth(buf); 
    
    //填写ARP响应包的类型
	buf[ETH_ARP_OPCODE_H_P]=ETH_ARP_OPCODE_REPLY_H_V;   //arp 响应
	buf[ETH_ARP_OPCODE_L_P]=ETH_ARP_OPCODE_REPLY_L_V;

    //填写ARP包的目的MAC地址以及源MAC地址
	while(i<6)
	{
        buf[ETH_ARP_DST_MAC_P+i]=buf[ETH_ARP_SRC_MAC_P+i];
        buf[ETH_ARP_SRC_MAC_P+i]=macaddr[i];
        i++;
	}

    //填写ARP包的目的IP地址以及源IP地址    
	i=0;
	while(i<4)
	{
        buf[ETH_ARP_DST_IP_P+i]=buf[ETH_ARP_SRC_IP_P+i];
        buf[ETH_ARP_SRC_IP_P+i]=ipaddr[i];
        i++;
	}

    dbglog("神舟III号[%d.%d.%d.%d]发送ARP响应\r\n",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3]);

    //发送ARP相应包
	//enc28j60PacketSend(42,buf); 
	//for(i=42;i<60;i++)
	//	buf[i] = 0;
	enc28j60PacketSend(60,buf); 
}

void make_echo_reply_from_request(unsigned char *buf,unsigned  int len)
{
	//填写包的目的MAC地址以及源MAC地址	
	make_eth(buf);
	//填写包的目的IP地址以及源IP地址	
	make_ip(buf);

    //填写ICMP相应包类型
	buf[ICMP_TYPE_P]=ICMP_TYPE_ECHOREPLY_V;	  //////回送应答////////////////////////////////////////////////////////////////////////////

    // we changed only the icmp.type field from request(=8) to reply(=0).
	// we can therefore easily correct the checksum:
	if (buf[ICMP_CHECKSUM_P] > (0xff-0x08))
	{
	    buf[ICMP_CHECKSUM_P+1]++;
	}
	buf[ICMP_CHECKSUM_P]+=0x08;

    dbglog("\n\r神舟III号[%d.%d.%d.%d]发送ICMP包响应",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3]);

    //发送ICMP响应包
	enc28j60PacketSend(len,buf);
}

// you can send a max of 220 bytes of data
void make_udp_reply_from_request(unsigned char *buf,char *data,unsigned int datalen,unsigned  int destport)
{
	unsigned int i=0;
	unsigned  int ck;
	make_eth(buf);
	
	// total length field in the IP header must be set:
	i= IP_HEADER_LEN+UDP_HEADER_LEN+datalen;
	buf[IP_TOTLEN_H_P]=i>>8;
	buf[IP_TOTLEN_L_P]=i;
	make_ip(buf);
	if(destport == 0)
	{
		//swap port
		uint8 tempH,tempL;
		tempH = buf[UDP_DST_PORT_H_P];
		tempL = buf[UDP_DST_PORT_L_P];
		buf[UDP_DST_PORT_H_P] = buf[UDP_SRC_PORT_H_P];
		buf[UDP_DST_PORT_L_P] = buf[UDP_SRC_PORT_L_P];
		buf[UDP_SRC_PORT_H_P] = tempH;
		buf[UDP_SRC_PORT_L_P] = tempL;
	}
	else
	{
		buf[UDP_SRC_PORT_H_P] = buf[UDP_DST_PORT_H_P];
		buf[UDP_SRC_PORT_L_P] = buf[UDP_DST_PORT_L_P];
		buf[UDP_DST_PORT_H_P] = destport>>8;
		buf[UDP_DST_PORT_L_P] = destport;
		
	}
	// source port does not matter and is what the sender used.
	// calculte the udp length:
	buf[UDP_LEN_H_P]=(UDP_HEADER_LEN+datalen)>>8;
	buf[UDP_LEN_L_P]=UDP_HEADER_LEN+datalen;
	// zero the checksum
	buf[UDP_CHECKSUM_H_P]=0;
	buf[UDP_CHECKSUM_L_P]=0;
	// copy the data:
	i = 0;
	while(i<datalen)
	{
        buf[UDP_DATA_P+i]=data[i];
        i++;
	}
	buf[IP_CHECKSUM_P] = 0x00;//ip head checksum,2 bytes
	buf[IP_CHECKSUM_P+1] = 0x00;
	ck=checksum(&buf[IP_P], IP_HEADER_LEN,0);
	buf[IP_CHECKSUM_P]=ck>>8;
	buf[IP_CHECKSUM_P+1]=ck& 0xff;

	ck=checksum(&buf[IP_SRC_P], 16 + datalen,1);
	buf[UDP_CHECKSUM_H_P]=ck>>8;
	buf[UDP_CHECKSUM_L_P]=ck& 0xff;



    dbglog("神舟III号 make_udp_reply_from_request\r\n");
	enc28j60PacketSend(UDP_HEADER_LEN+IP_HEADER_LEN+ETH_HEADER_LEN+datalen,buf);
}

void make_tcp_synack_from_syn(unsigned char *buf)
{
	unsigned  int ck;
    
	//填写包的目的MAC地址以及源MAC地址	
	make_eth(buf);
	//计算包的长度
	// total length field in the IP header must be set: 20 bytes IP + 24 bytes (20tcp+4tcp options)
	buf[IP_TOTLEN_H_P]=0;
	buf[IP_TOTLEN_L_P]=IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+4;
	//填写包的目的IP地址以及源IP地址	
	make_ip(buf);
	buf[TCP_FLAGS_P]=TCP_FLAGS_SYNACK_V;
	make_tcphead(buf,1,1,0);
	// calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + 4 (one option: mss)
	ck=checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN+4,2);
	buf[TCP_CHECKSUM_H_P]=ck>>8;
	buf[TCP_CHECKSUM_L_P]=ck& 0xff;
	// add 4 for option mss:
    dbglog("神舟III号[%d.%d.%d.%d]发送SYN包响应\r\n",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3]);
	update_tcp_buf(buf);
	enc28j60PacketSend(IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+4+ETH_HEADER_LEN,buf);
}

// get a pointer to the start of tcp data in buf
// Returns 0 if there is no data
// You must call init_len_info once before calling this function
unsigned  int get_tcp_data_pointer(void)
{
	if (info_data_len)
	{
	    return((unsigned  int)TCP_SRC_PORT_H_P+info_hdr_len);
	}
	else
	{
	    return(0);
	}
}

// do some basic length calculations and store the result in static varibales
void init_len_info(unsigned char *buf)
{
    info_data_len=(buf[IP_TOTLEN_H_P]<<8)|(buf[IP_TOTLEN_L_P]&0xff);
    info_data_len-=IP_HEADER_LEN;
    info_hdr_len=(buf[TCP_HEADER_LEN_P]>>4)*4; // generate len in bytes;
    info_data_len-=info_hdr_len;
    if (info_data_len<=0)
	{
        info_data_len=0;
    }
}

// fill in tcp data at position pos. pos=0 means start of
// tcp data. Returns the position at which the string after
// this string could be filled.
unsigned  int fill_tcp_data_p(unsigned char *buf,unsigned  int pos, const unsigned char *progmem_s)
{
	char c;
	// fill in tcp data at position pos
	//
	// with no options the data starts after the checksum + 2 more bytes (urgent ptr)
	while ((c = pgm_read_byte(progmem_s++))) 
	{
	    buf[TCP_CHECKSUM_L_P+3+pos]=c;
	    pos++;
	}
	return(pos);
}

// fill in tcp data at position pos. pos=0 means start of
// tcp data. Returns the position at which the string after
// this string could be filled.
unsigned  int fill_tcp_data(unsigned char *buf,unsigned  int pos, const char *s)
{
	// fill in tcp data at position pos
	//
	// with no options the data starts after the checksum + 2 more bytes (urgent ptr)
	while (*s) 
	{
	    buf[TCP_CHECKSUM_L_P+3+pos]=*s;
	    pos++;
	    s++;
	}
	return(pos);
}

// Make just an ack packet with no tcp data inside
// This will modify the eth/ip/tcp header 
void make_tcp_ack_from_any(unsigned char *buf)
{
	unsigned  int j;
	make_eth(buf);
	// fill the header:
	buf[TCP_FLAGS_P]=TCP_FLAGS_ACK_V;
	if (info_data_len==0)
	{
	     // if there is no data then we must still acknoledge one packet
	    make_tcphead(buf,1,0,1); // no options
	}
	else
	{
	    make_tcphead(buf,info_data_len,0,1); // no options
	}
	
	// total length field in the IP header must be set:
	// 20 bytes IP + 20 bytes tcp (when no options) 
	j=IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN;
	buf[IP_TOTLEN_H_P]=j>>8;
	buf[IP_TOTLEN_L_P]=j& 0xff;
	make_ip(buf);
	// calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + data len
	j=checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN,2);
	buf[TCP_CHECKSUM_H_P]=j>>8;
	buf[TCP_CHECKSUM_L_P]=j& 0xff;

	update_tcp_ackseq(buf);

    dbglog("神舟III号[%d.%d.%d.%d]发送ACK包响应\r\n",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3]);
    
	enc28j60PacketSend(IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+ETH_HEADER_LEN,buf);
}

// you must have called init_len_info at some time before calling this function
// dlen is the amount of tcp data (http data) we send in this packet
// You can use this function only immediately after make_tcp_ack_from_any
// This is because this function will NOT modify the eth/ip/tcp header except for
// length and checksum
unsigned  int make_tcp_ack_with_data(unsigned char *buf,unsigned  int dlen)
{
	unsigned  int j;
	uint32 seq;
	// fill the header:
	// This code requires that we send only one data packet
	// because we keep no state information. We must therefore set
	// the fin here:
	//buf[TCP_FLAGS_P]=TCP_FLAGS_ACK_V|TCP_FLAGS_PUSH_V|TCP_FLAGS_FIN_V;
	buf[TCP_FLAGS_P]=TCP_FLAGS_ACK_V|TCP_FLAGS_PUSH_V;//TCP_FLAGS_FIN_V会导致断开连接
	
	// total length field in the IP header must be set:
	// 20 bytes IP + 20 bytes tcp (when no options) + len of data
	j=IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+dlen;
	buf[IP_TOTLEN_H_P]=j>>8;
	buf[IP_TOTLEN_L_P]=j& 0xff;

	seq = get_tcp_seq(buf);
    buf[TCP_SEQ_H_P+0]= seq>>24;
    buf[TCP_SEQ_H_P+1]= seq>>16;
    buf[TCP_SEQ_H_P+2]= seq>>8; 
    buf[TCP_SEQ_H_P+3]= seq;
	//seqnum += dlen;
	add_tcp_seq(buf,dlen);

	fill_ip_hdr_checksum(buf);
	// zero the checksum
	buf[TCP_CHECKSUM_H_P]=0;
	buf[TCP_CHECKSUM_L_P]=0;
	// calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + data len
	j=checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN+dlen,2);
	buf[TCP_CHECKSUM_H_P]=j>>8;
	buf[TCP_CHECKSUM_L_P]=j& 0xff;
	enc28j60PacketSend(IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+dlen+ETH_HEADER_LEN,buf);
	dbglog("make_tcp_ack_with_data,ip=%02x,port=%02x%02x\r\n",buf[IP_DST_P+3],buf[TCP_DST_PORT_H_P],buf[TCP_DST_PORT_H_P+1]);
	return IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+dlen+ETH_HEADER_LEN;
}
#if 0
// you must have called init_len_info at some time before calling this function
// dlen is the amount of tcp data (http data) we send in this packet
// You can use this function only immediately after make_tcp_ack_from_any
// This is because this function will NOT modify the eth/ip/tcp header except for
// length and checksum
unsigned  int make_tcp_ack_with_finish(unsigned char *buf)//not working
	{
	unsigned  int j;
	uint32 seq;
	// fill the header:
	// This code requires that we send only one data packet
	// because we keep no state information. We must therefore set
	// the fin here:
	//buf[TCP_FLAGS_P]=TCP_FLAGS_ACK_V|TCP_FLAGS_PUSH_V|TCP_FLAGS_FIN_V;
	buf[TCP_FLAGS_P]=TCP_FLAGS_FIN_V;//TCP_FLAGS_FIN_V会导致断开连接
	
	// total length field in the IP header must be set:
	// 20 bytes IP + 20 bytes tcp (when no options) + len of data
	j=IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN;
	buf[IP_TOTLEN_H_P]=j>>8;
	buf[IP_TOTLEN_L_P]=j& 0xff;

	seq = get_tcp_seq(buf);
    buf[TCP_SEQ_H_P+0]= seq>>24;
    buf[TCP_SEQ_H_P+1]= seq>>16;
    buf[TCP_SEQ_H_P+2]= seq>>8; 
    buf[TCP_SEQ_H_P+3]= seq;
	//seqnum += dlen;
	add_tcp_seq(buf,1);

	fill_ip_hdr_checksum(buf);
	// zero the checksum
	buf[TCP_CHECKSUM_H_P]=0;
	buf[TCP_CHECKSUM_L_P]=0;
	// calculate the checksum, len=8 (start from ip.src) + TCP_HEADER_LEN_PLAIN + data len
	j=checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN,2);
	buf[TCP_CHECKSUM_H_P]=j>>8;
	buf[TCP_CHECKSUM_L_P]=j& 0xff;
	enc28j60PacketSend(IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+ETH_HEADER_LEN,buf);
	dbglog("make_tcp_ack_with_finish,ip=%02x,port=%02x%02x\r\n",buf[IP_DST_P+3],buf[TCP_DST_PORT_H_P],buf[TCP_DST_PORT_H_P+1]);
	return IP_HEADER_LEN+TCP_HEADER_LEN_PLAIN+ETH_HEADER_LEN;
	}
#endif
unsigned  int get_tcp_ack_seq(unsigned char *buf)
{
	unsigned  int seq;

	

    seq = (buf[TCP_SEQACK_H_P+0]<<24)
		| (buf[TCP_SEQACK_H_P+1]<<16)
		| (buf[TCP_SEQACK_H_P+2]<<8)
		| (buf[TCP_SEQACK_H_P+3]<<0)
		;
	return seq;
}
uint32 get_ip_dest(unsigned char *buf)
{
	uint32 ip;
	ip = (buf[IP_DST_P+0]<<24)|(buf[IP_DST_P+1]<<16)|(buf[IP_DST_P+2]<<8)|(buf[IP_DST_P+3]<<0);
	//dbglog("get_ip_dest ip=%x\r\n",ip);
	return ip;
}
uint32 get_ip_src(unsigned char *buf)
{
	uint32 ip;
	ip = (buf[IP_SRC_P+0]<<24)|(buf[IP_SRC_P+1]<<16)|(buf[IP_SRC_P+2]<<8)|(buf[IP_SRC_P+3]<<0);
	//dbglog("get_ip_src ip=%x\r\n",ip);
	return ip;
}
uint16 get_port_dest(unsigned char *buf)
{
	uint16 port;
	port = (buf[TCP_DST_PORT_H_P+0]<<8)|(buf[TCP_DST_PORT_H_P+1]<<0);
	//dbglog("get_port_dest port=%x\r\n",port);
	return port;
}
uint16 get_port_src(unsigned char *buf)
{
	uint16 port;
	port = (buf[TCP_SRC_PORT_H_P+0]<<8)|(buf[TCP_SRC_PORT_H_P+1]<<0);
	//dbglog("get_port_src port=%x\r\n",port);
	return port;
}
extern void TcpSendFinish(unsigned char *buf);

void new_tcp_seq(unsigned char *buf)
{
	uint8 find = 0;
	uint32 i;
	for(i=0;i<MAX_LINK;i++)
	{
		if( ! linkedInfo[i].isUsed)
		{
			linkedInfo[i].isUsed = 1;
			find = 1;
			linkedInfo[i].ip = get_ip_src(buf);
			linkedInfo[i].port = get_port_src(buf);
			linkedInfo[i].seq = 0x0a;
			linkedInfo[i].time = configInfo.time;
			//memcpy(linkedInfo[i].buf,buf,LINK_HEAD_LEN);
			dbglog("new_tcp_seq ip=%x,%x\r\n",linkedInfo[i].ip,linkedInfo[i].port);
			break;
		}
	}
	

	if( ! find)
	{
		uint32 nTime = linkedInfo[0].time;
		int index = 0;
		dbglog("new_tcp_seq err:too much link\r\n");
		//find oldest time,send finish to it and replace it
		for(i=1;i<MAX_LINK;i++)
		{
			if(nTime > linkedInfo[i].time)
			{
				nTime = linkedInfo[i].time;
				index = i;
				
			}
		}
		TcpSendFinish(linkedInfo[index].buf);//可能无法由服务器端发出finish信号
		//那么我们可以发出特定命令，让客户端发出finish
		
		linkedInfo[index].isUsed = 1;
		linkedInfo[index].ip = get_ip_src(buf);
		linkedInfo[index].port = get_port_src(buf);
		linkedInfo[index].seq = 0x0a;
		linkedInfo[index].time = configInfo.time;
		//memcpy(linkedInfo[i].buf,buf,LINK_HEAD_LEN);
		dbglog("new_tcp_seq replace[%d] ip=%x,%x\r\n",index,linkedInfo[index].ip,linkedInfo[index].port);
	}
}
void delete_tcp_seq(unsigned char *buf)
{
	uint8 find = 0;
	uint32 i;
	uint32 ip;
	uint16 port;
	for(i=0;i<MAX_LINK;i++)
	{
		if( linkedInfo[i].isUsed)
		{
			ip = get_ip_dest(buf);
			port = get_port_dest(buf);
			//dbglog("delete_tcp_seq find ip=%x\r\n",ip);
			if(linkedInfo[i].ip == ip && linkedInfo[i].port == port)
			{
				linkedInfo[i].isUsed = 0;
				find = 1;
				dbglog("delete_tcp_seq remove ip=%x,%x\r\n",linkedInfo[i].ip,linkedInfo[i].port);
				break;
			}
		}
	}
	

	if( ! find)
		dbglog("delete_tcp_seq err:not found\r\n");
}
uint32 get_tcp_seq(unsigned char *buf)
{
	uint8 find = 0;
	uint32 i;
	uint32 ip;
	uint16 port;
	uint32 seq = 0;
	for(i=0;i<MAX_LINK;i++)
	{
		if( linkedInfo[i].isUsed)
		{
			ip = get_ip_dest(buf);
			port = get_port_dest(buf);
			//dbglog("get_tcp_seq find ip=%x\r\n",ip);
			if(linkedInfo[i].ip == ip && linkedInfo[i].port == port)
			{
				seq = linkedInfo[i].seq;
				find = 1;
				//dbglog("get_tcp_seq ip=%x,%x\r\n",linkedInfo[i].ip,linkedInfo[i].port);
				break;
			}
		}
	}
	

	if( ! find)
		dbglog("get_tcp_seq err:not found\r\n");
	return seq;
}
void add_tcp_seq(unsigned char *buf,uint32 num)
{
	uint8 find = 0;
	uint32 i;
	uint32 ip;
	uint16 port;
	for(i=0;i<MAX_LINK;i++)
	{
		if( linkedInfo[i].isUsed)
		{
			ip = get_ip_dest(buf);
			port = get_port_dest(buf);
			//dbglog("add_tcp_seq find ip=%x\r\n",ip);
			if(linkedInfo[i].ip == ip && linkedInfo[i].port == port)
			{
				linkedInfo[i].seq += num;
				find = 1;
				//dbglog("add_tcp_seq add ip=%x,%x\r\n",linkedInfo[i].ip,linkedInfo[i].port);
				break;
			}
		}
	}
	

	if( ! find)
		dbglog("add_tcp_seq err:not found\r\n");
}
//更新保存的tcp链接信息中的ackseq(应答序号)
void update_tcp_ackseq(unsigned char *buf)
{
	uint8 find = 0;
	uint32 i;
	uint32 ip;
	uint16 port;
	for(i=0;i<MAX_LINK;i++)
	{
		if( linkedInfo[i].isUsed)
		{
			ip = get_ip_dest(buf);
			port = get_port_dest(buf);
			//dbglog("update_tcp_ackseq find ip=%x,%x\r\n",ip,port);
			if(linkedInfo[i].ip == ip && linkedInfo[i].port == port)
			{
				memcpy(&linkedInfo[i].buf[TCP_SEQACK_H_P],&buf[TCP_SEQACK_H_P],4);
				find = 1;
				//dbglog("update_tcp_ackseq ip=%x,%x\r\n",linkedInfo[i].ip,linkedInfo[i].port);
				break;
			}
		}
	}
	

	if( ! find)
		dbglog("update_tcp_ackseq err:not found\r\n");
}
//从sync包中提取tcp链接信息
void update_tcp_buf(unsigned char *buf)
{
	uint8 find = 0;
	uint32 i;
	uint32 ip;
	uint16 port;
	for(i=0;i<MAX_LINK;i++)
	{
		if( linkedInfo[i].isUsed)
		{
			ip = get_ip_dest(buf);
			port = get_port_dest(buf);
			//dbglog("update_tcp_buf find ip=%x,%x\r\n",ip,port);
			if(linkedInfo[i].ip == ip && linkedInfo[i].port == port)
			{
				find = 1;
				memcpy(linkedInfo[i].buf,buf,LINK_HEAD_LEN);
			    // no options:
			    // 20 bytes:
			    linkedInfo[i].buf[TCP_HEADER_LEN_P]=0x50;
				
				//dbglog("update_tcp_buf ip=%x,%x\r\n",linkedInfo[i].ip,linkedInfo[i].port);
				break;
			}
		}
	}
	

	if( ! find)
		dbglog("update_tcp_buf err:too much link\r\n");
}
#if 1
void make_udp_hdcp_discover(unsigned char *buf)
{
	int i;
	unsigned int ck;
	unsigned int len;
	make_eth(buf);
	//  broadcast mac address is ff ff ff ff ff ff
    for(i=0; i<6; i++)
    {         
        buf[ETH_DST_MAC+i] = 0xff;
    }
	buf[ETH_TYPE_L_P] = ETHTYPE_IP_L_V;
	buf[ETH_TYPE_H_P] = ETHTYPE_IP_H_V;

	buf[IP_P] = 0x45;
	buf[IP_TOS_P] = 0x00;
	//ip包总长度，包括ip头
	i= IP_HEADER_LEN+UDP_HEADER_LEN+38+192+24+54;
	buf[IP_TOTLEN_H_P] = i>>8;// 10e
	buf[IP_TOTLEN_L_P] = i;
	buf[IP_ID_H_P] = 0x00;
	buf[IP_ID_L_P] = 0x01;
	buf[IP_FLAGS_P] = 0x00;
	buf[IP_FLAGS_P+1] = 0x00;
	buf[IP_OFFSET_P] = 0x00;
	buf[IP_TTL_P] = 64;

	buf[IP_PROTO_P] = 0x11;

	buf[IP_CHECKSUM_P] = 0x00;//ip head checksum,2 bytes
	buf[IP_CHECKSUM_P+1] = 0x00;

	make_ip(buf);
	buf[IP_SRC_P] = 0x00;//0xc0;
	buf[IP_SRC_P+1] = 0x00;//0xa8;
	buf[IP_SRC_P+2] = 0x00;//0x01;
	buf[IP_SRC_P+3] = 0x00;//0x66;
	buf[IP_DST_P] = 0xff;
	buf[IP_DST_P+1] = 0xff;
	buf[IP_DST_P+2] = 0xff;
	buf[IP_DST_P+3] = 0xff;
	
	buf[UDP_SRC_PORT_H_P] = 0x00;
	buf[UDP_SRC_PORT_L_P] = 68;
	buf[UDP_DST_PORT_H_P] = 0x00;
	buf[UDP_DST_PORT_L_P] = 67;
	len = UDP_HEADER_LEN + 38+192+4+54;
	buf[UDP_LEN_H_P] = len>>8;
	buf[UDP_LEN_L_P] = len;
	buf[UDP_CHECKSUM_H_P] = 0x00;// calc by the end
	buf[UDP_CHECKSUM_L_P] = 0x00;//

	buf[UDP_DATA_P + 0x00] = 0x01;// OP
	buf[UDP_DATA_P + 0x01] = 0x01;// HTYPE
	buf[UDP_DATA_P + 0x02] = 0x06;// HLEN
	buf[UDP_DATA_P + 0x03] = 0x00;// HOPS

	buf[UDP_DATA_P + 0x04] = 0x39;// XID
	buf[UDP_DATA_P + 0x05] = 0x03;
	buf[UDP_DATA_P + 0x06] = 0xf3;
	buf[UDP_DATA_P + 0x07] = 0x26;

	buf[UDP_DATA_P + 0x08] = 0x00;//SECS
	buf[UDP_DATA_P + 0x09] = 0x00;
	buf[UDP_DATA_P + 0x0a] = 0x00;//FLAGS
	buf[UDP_DATA_P + 0x0b] = 0x00;

	buf[UDP_DATA_P + 0x0c] = 0x00;//CIADDR(Client IP)
	buf[UDP_DATA_P + 0x0d] = 0x00;
	buf[UDP_DATA_P + 0x0e] = 0x00;
	buf[UDP_DATA_P + 0x0f] = 0x00;

	buf[UDP_DATA_P + 0x10] = 0x00;//YIADDR(your IP)
	buf[UDP_DATA_P + 0x11] = 0x00;
	buf[UDP_DATA_P + 0x12] = 0x00;
	buf[UDP_DATA_P + 0x13] = 0x00;

	buf[UDP_DATA_P + 0x14] = 0x00;//SIADDR(serve ip)
	buf[UDP_DATA_P + 0x15] = 0x00;
	buf[UDP_DATA_P + 0x16] = 0x00;
	buf[UDP_DATA_P + 0x17] = 0x00;

	buf[UDP_DATA_P + 0x18] = 0x00;//GIADDR(gateway ip)
	buf[UDP_DATA_P + 0x19] = 0x00;
	buf[UDP_DATA_P + 0x1a] = 0x00;
	buf[UDP_DATA_P + 0x1b] = 0x00;

	buf[UDP_DATA_P + 0x1c] = macaddr[0];//CHADDR(client hardware addr) 1
	buf[UDP_DATA_P + 0x1d] = macaddr[1];
	buf[UDP_DATA_P + 0x1e] = macaddr[2];
	buf[UDP_DATA_P + 0x1f] = macaddr[3];

	buf[UDP_DATA_P + 0x20] = macaddr[4];//CHADDR(client hardware addr) 2
	buf[UDP_DATA_P + 0x21] = macaddr[5];
	buf[UDP_DATA_P + 0x22] = 0x00;
	buf[UDP_DATA_P + 0x23] = 0x00;

	buf[UDP_DATA_P + 0x24] = 0x00;//CHADDR(client hardware addr) 3
	buf[UDP_DATA_P + 0x25] = 0x00;
	buf[UDP_DATA_P + 0x26] = 0x00;
	buf[UDP_DATA_P + 0x27] = 0x00;

	buf[UDP_DATA_P + 0x28] = 0x00;//CHADDR(client hardware addr) 4
	buf[UDP_DATA_P + 0x29] = 0x00;
	buf[UDP_DATA_P + 0x2a] = 0x00;
	buf[UDP_DATA_P + 0x2b] = 0x00;

	for(i=0;i<192;i++)
		buf[UDP_DATA_P + 0x2c + i] = 0x00;

	buf[UDP_DATA_P + 0xec] = 0x63;//magic cookie
	buf[UDP_DATA_P + 0xed] = 0x82;
	buf[UDP_DATA_P + 0xee] = 0x53;
	buf[UDP_DATA_P + 0xef] = 0x63;

	buf[UDP_DATA_P + 0xf0] = 53;//dhcp option
	buf[UDP_DATA_P + 0xf1] = 01;
	buf[UDP_DATA_P + 0xf2] = 01;

	//buf[UDP_DATA_P + 0xf3] = 50;//dhcp option
	//buf[UDP_DATA_P + 0xf4] = 04;
	//buf[UDP_DATA_P + 0xf5] = 192;
	//buf[UDP_DATA_P + 0xf6] = 168;
	//buf[UDP_DATA_P + 0xf7] = 01;
	//buf[UDP_DATA_P + 0xf8] = 250;

	buf[UDP_DATA_P + 0xf3] = 55;//dhcp option
	buf[UDP_DATA_P + 0xf4] = 04;
	buf[UDP_DATA_P + 0xf5] = 01;
	buf[UDP_DATA_P + 0xf6] = 03;
	buf[UDP_DATA_P + 0xf7] = 0x1c;
	buf[UDP_DATA_P + 0xf8] = 06;

	buf[UDP_DATA_P + 0xf9] = 57;//dhcp option
	buf[UDP_DATA_P + 250] = 02;
	buf[UDP_DATA_P + 251] = 05;
	buf[UDP_DATA_P + 252] = 0xdc;

	buf[UDP_DATA_P + 253] = 0xff;

	for(i=0;i<54;i++)
		buf[UDP_DATA_P + 254 + i] = 0x00;

	buf[IP_CHECKSUM_P] = 0x00;//ip head checksum,2 bytes
	buf[IP_CHECKSUM_P+1] = 0x00;
	ck=checksum(&buf[IP_P], IP_HEADER_LEN,0);
	buf[IP_CHECKSUM_P]=ck>>8;
	buf[IP_CHECKSUM_P+1]=ck& 0xff;

	ck=checksum(&buf[IP_SRC_P], 16 + 38+192+24+54,1);
	buf[UDP_CHECKSUM_H_P] = ck>>8;// calc by the end
	buf[UDP_CHECKSUM_L_P] = ck&0xff;

    dbglog("神舟III号 make_udp_hdcp_discover\r\n");
	enc28j60PacketSend(34 +UDP_HEADER_LEN+38+192+24+54,buf);
}
uint8 receive_udp_hdcp_offer(unsigned char *buf,int len)
{
	int i;
	unsigned int ck;
	uint8 isDhcpOffer = TRUE;
	int pos;

	if(len < 282)
		isDhcpOffer = FALSE;
	// is it my mac?
    for(i=0; i<6; i++)
    	if(buf[ETH_DST_MAC+i] != macaddr[i])
    	{
			isDhcpOffer = FALSE;
			dbglog("receive_udp_hdcp_offer not my mac\r\n");
			break;
    	}
	//is it dhcp
	if(buf[UDP_DATA_P + 236] != 0x63
		|| buf[UDP_DATA_P + 237] != 0x82
		|| buf[UDP_DATA_P + 238] != 0x53
		|| buf[UDP_DATA_P + 239] != 0x63
		)
	{
		isDhcpOffer = FALSE;
		dbglog("receive_udp_hdcp_offer not dhcp magic\r\n");
	}
	//is it dhcp offer
	pos = 240;
	//find hdcp option 53,check is it value 02
	{
		uint8 option;
		uint8 optionLen;
		option = buf[UDP_DATA_P + pos];
		while(option != 0xff && (UDP_DATA_P + pos + 2)<len)
		{
			if(option == 53)
			{
				if(buf[UDP_DATA_P + pos + 2] != 2)
				{
					isDhcpOffer = FALSE;
					dbglog("receive_udp_hdcp_offer not dhcp offer\r\n");
				}
				break;
			}
			optionLen = buf[UDP_DATA_P + pos + 1];
			pos += 2 + optionLen;
			option = buf[UDP_DATA_P + pos];
		}
	}

	if(isDhcpOffer)
	{
		//parse the info
		for(i=0;i<4;i++)
			ipaddr[i] = buf[UDP_DATA_P + 16 + i];
		dbglog("receive_udp_hdcp_offer ipaddr=%x%x%x%x\r\n",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3]);
		dhcpServerIp = 0;
		for(i=0;i<4;i++)
			dhcpServerIp |= (buf[UDP_DATA_P + 20 + i]<<((3-i)*8));
		dbglog("receive_udp_hdcp_offer dhcpServerIp=%x\r\n",dhcpServerIp);

		pos = 240;
		//parse hdcp option 
		{
			uint8 option;
			uint8 optionLen;
			option = buf[UDP_DATA_P + pos];
			//dbglog("receive_udp_hdcp_offer option=%x\r\n",option);
			while(option != 0xff && (UDP_DATA_P + pos + 2)<len)
			{
				dbglog("receive_udp_hdcp_offer option=%x\r\n",option);
				if(option == 1)
				{
					submask = 0;
					for(i=0;i<4;i++)
						submask |= (buf[UDP_DATA_P + pos + 2 + i]<<((3-i)*8));
					dbglog("receive_udp_hdcp_offer submask=%x\r\n",submask);
				}
				else if(option == 3)
				{
					router = 0;
					for(i=0;i<4;i++)
						router |= (buf[UDP_DATA_P + pos + 2 + i]<<((3-i)*8));
					dbglog("receive_udp_hdcp_offer router=%x\r\n",router);
				}
				else if(option == 51)
				{
					leaseTime = 0;
					for(i=0;i<4;i++)
						leaseTime |= (buf[UDP_DATA_P + pos + 2 + i]<<((3-i)*8));
					dbglog("receive_udp_hdcp_offer leaseTime=%x\r\n",leaseTime);
				}
				else if(option == 54)
				{
					dhcpServerIp = 0;
					for(i=0;i<4;i++)
						dhcpServerIp |= (buf[UDP_DATA_P + pos + 2 + i]<<((3-i)*8));
					dbglog("receive_udp_hdcp_offer dhcpServerIp=%x\r\n",dhcpServerIp);
				}
				else if(option == 6)
				{
					optionLen = buf[UDP_DATA_P + pos + 1];
					dnsServer1 = 0;
					for(i=0;i<4;i++)
						dnsServer1 |= (buf[UDP_DATA_P + pos + 2 + i]<<((3-i)*8));
					dbglog("receive_udp_hdcp_offer dnsServer1=%x\r\n",dnsServer1);
					if(optionLen > 4)
					{
						dnsServer2 = 0;
						for(i=0;i<4;i++)
							dnsServer2 |= (buf[UDP_DATA_P + pos + 6 + i]<<((3-i)*8));
						dbglog("receive_udp_hdcp_offer dnsServer2=%x\r\n",dnsServer2);
					}
				}
				optionLen = buf[UDP_DATA_P + pos + 1];
				pos += 2 + optionLen;
				option = buf[UDP_DATA_P + pos];
			}
		}
	}
	

	return isDhcpOffer;
}
void make_udp_hdcp_request(unsigned char *buf)
{
	int i;
	unsigned int ck;
	unsigned int len;
	uint16 pos;
	make_eth(buf);
	//  broadcast mac address is ff ff ff ff ff ff
    for(i=0; i<6; i++)
    {         
        buf[ETH_DST_MAC+i] = 0xff;
    }
	buf[ETH_TYPE_L_P] = ETHTYPE_IP_L_V;
	buf[ETH_TYPE_H_P] = ETHTYPE_IP_H_V;

	buf[IP_P] = 0x45;
	buf[IP_TOS_P] = 0x00;
	//ip包总长度，包括ip头
	i= IP_HEADER_LEN+UDP_HEADER_LEN+240+16+42;
	buf[IP_TOTLEN_H_P] = i>>8;// 10e
	buf[IP_TOTLEN_L_P] = i;
	buf[IP_ID_H_P] = 0x00;
	buf[IP_ID_L_P] = 0x01;
	buf[IP_FLAGS_P] = 0x00;
	buf[IP_FLAGS_P+1] = 0x00;
	buf[IP_OFFSET_P] = 0x00;
	buf[IP_TTL_P] = 64;

	buf[IP_PROTO_P] = 0x11;

	buf[IP_CHECKSUM_P] = 0x00;//ip head checksum,2 bytes
	buf[IP_CHECKSUM_P+1] = 0x00;

	make_ip(buf);
	buf[IP_SRC_P] = 0x00;//0xc0;
	buf[IP_SRC_P+1] = 0x00;//0xa8;
	buf[IP_SRC_P+2] = 0x00;//0x01;
	buf[IP_SRC_P+3] = 0x00;//0x66;
	buf[IP_DST_P] = 0xff;
	buf[IP_DST_P+1] = 0xff;
	buf[IP_DST_P+2] = 0xff;
	buf[IP_DST_P+3] = 0xff;
	
	buf[UDP_SRC_PORT_H_P] = 0x00;
	buf[UDP_SRC_PORT_L_P] = 68;
	buf[UDP_DST_PORT_H_P] = 0x00;
	buf[UDP_DST_PORT_L_P] = 67;
	len = UDP_HEADER_LEN + 240+16+42;
	buf[UDP_LEN_H_P] = len>>8;
	buf[UDP_LEN_L_P] = len;
	buf[UDP_CHECKSUM_H_P] = 0x00;// calc by the end
	buf[UDP_CHECKSUM_L_P] = 0x00;//

	buf[UDP_DATA_P + 0x00] = 0x01;// OP
	buf[UDP_DATA_P + 0x01] = 0x01;// HTYPE
	buf[UDP_DATA_P + 0x02] = 0x06;// HLEN
	buf[UDP_DATA_P + 0x03] = 0x00;// HOPS

	buf[UDP_DATA_P + 0x04] = 0x39;// XID
	buf[UDP_DATA_P + 0x05] = 0x03;
	buf[UDP_DATA_P + 0x06] = 0xf3;
	buf[UDP_DATA_P + 0x07] = 0x26;

	buf[UDP_DATA_P + 0x08] = 0x00;//SECS
	buf[UDP_DATA_P + 0x09] = 0x00;
	buf[UDP_DATA_P + 0x0a] = 0x00;//FLAGS
	buf[UDP_DATA_P + 0x0b] = 0x00;

	buf[UDP_DATA_P + 0x0c] = 0x00;//CIADDR(Client IP)
	buf[UDP_DATA_P + 0x0d] = 0x00;
	buf[UDP_DATA_P + 0x0e] = 0x00;
	buf[UDP_DATA_P + 0x0f] = 0x00;

	buf[UDP_DATA_P + 0x10] = 0x00;//YIADDR(your IP)
	buf[UDP_DATA_P + 0x11] = 0x00;
	buf[UDP_DATA_P + 0x12] = 0x00;
	buf[UDP_DATA_P + 0x13] = 0x00;

	buf[UDP_DATA_P + 0x14] = dhcpServerIp>>24;//SIADDR(serve ip)
	buf[UDP_DATA_P + 0x15] = dhcpServerIp>>16;
	buf[UDP_DATA_P + 0x16] = dhcpServerIp>>8;
	buf[UDP_DATA_P + 0x17] = dhcpServerIp>>0;

	buf[UDP_DATA_P + 0x18] = 0x00;//GIADDR(gateway ip)
	buf[UDP_DATA_P + 0x19] = 0x00;
	buf[UDP_DATA_P + 0x1a] = 0x00;
	buf[UDP_DATA_P + 0x1b] = 0x00;

	buf[UDP_DATA_P + 0x1c] = macaddr[0];//CHADDR(client hardware addr) 1
	buf[UDP_DATA_P + 0x1d] = macaddr[1];
	buf[UDP_DATA_P + 0x1e] = macaddr[2];
	buf[UDP_DATA_P + 0x1f] = macaddr[3];

	buf[UDP_DATA_P + 0x20] = macaddr[4];//CHADDR(client hardware addr) 2
	buf[UDP_DATA_P + 0x21] = macaddr[5];
	buf[UDP_DATA_P + 0x22] = 0x00;
	buf[UDP_DATA_P + 0x23] = 0x00;

	buf[UDP_DATA_P + 0x24] = 0x00;//CHADDR(client hardware addr) 3
	buf[UDP_DATA_P + 0x25] = 0x00;
	buf[UDP_DATA_P + 0x26] = 0x00;
	buf[UDP_DATA_P + 0x27] = 0x00;

	buf[UDP_DATA_P + 0x28] = 0x00;//CHADDR(client hardware addr) 4
	buf[UDP_DATA_P + 0x29] = 0x00;
	buf[UDP_DATA_P + 0x2a] = 0x00;
	buf[UDP_DATA_P + 0x2b] = 0x00;

	for(i=0;i<192;i++)
		buf[UDP_DATA_P + 0x2c + i] = 0x00;

	buf[UDP_DATA_P + 0xec] = 0x63;//magic cookie
	buf[UDP_DATA_P + 0xed] = 0x82;
	buf[UDP_DATA_P + 0xee] = 0x53;
	buf[UDP_DATA_P + 0xef] = 0x63;

	pos = 0xf0;
	buf[UDP_DATA_P + pos++] = 53;//dhcp option
	buf[UDP_DATA_P + pos++] = 01;
	buf[UDP_DATA_P + pos++] = 03;

	//buf[UDP_DATA_P + 0xf3] = 50;//dhcp option
	//buf[UDP_DATA_P + 0xf4] = 04;
	//buf[UDP_DATA_P + 0xf5] = 192;
	//buf[UDP_DATA_P + 0xf6] = 168;
	//buf[UDP_DATA_P + 0xf7] = 01;
	//buf[UDP_DATA_P + 0xf8] = 250;

	buf[UDP_DATA_P + pos++] = 50;//dhcp option
	buf[UDP_DATA_P + pos++] = 04;
	buf[UDP_DATA_P + pos++] = ipaddr[0];
	buf[UDP_DATA_P + pos++] = ipaddr[1];
	buf[UDP_DATA_P + pos++] = ipaddr[2];
	buf[UDP_DATA_P + pos++] = ipaddr[3];

	buf[UDP_DATA_P + pos++] = 54;//dhcp option
	buf[UDP_DATA_P + pos++] = 04;
	buf[UDP_DATA_P + pos++] = dhcpServerIp>>24;
	buf[UDP_DATA_P + pos++] = dhcpServerIp>>16;
	buf[UDP_DATA_P + pos++] = dhcpServerIp>>8;
	buf[UDP_DATA_P + pos++] = dhcpServerIp>>0;

	buf[UDP_DATA_P + pos++] = 0xff;

	for(i=0;i<42;i++)
		buf[UDP_DATA_P + (pos++) + i] = 0x00;

	buf[IP_CHECKSUM_P] = 0x00;//ip head checksum,2 bytes
	buf[IP_CHECKSUM_P+1] = 0x00;
	ck=checksum(&buf[IP_P], IP_HEADER_LEN,0);
	buf[IP_CHECKSUM_P]=ck>>8;
	buf[IP_CHECKSUM_P+1]=ck& 0xff;

	ck=checksum(&buf[IP_SRC_P], 16 + 240+16+42,1);
	buf[UDP_CHECKSUM_H_P] = ck>>8;// calc by the end
	buf[UDP_CHECKSUM_L_P] = ck&0xff;

    dbglog("神舟III号 make_udp_hdcp_request\r\n");
	enc28j60PacketSend(34 +UDP_HEADER_LEN+240+16+42,buf);
}
//return 	0=not dhcp ack
//			1=dhcp ack
//			2=dhcp nack,refuse
uint8 receive_udp_hdcp_ack(unsigned char *buf,int len)
{
	int i;
	unsigned int ck;
	uint8 dhcpAckType = 1;
	int pos;

	if(len < 282)
		dhcpAckType = FALSE;
	// is it my mac?
    for(i=0; i<6; i++)
    	if(buf[ETH_DST_MAC+i] != macaddr[i])
    	{
			dhcpAckType = 0;
			dbglog("receive_udp_hdcp_ack not my mac\r\n");
			break;
    	}
	//is it dhcp
	if(buf[UDP_DATA_P + 236] != 0x63
		|| buf[UDP_DATA_P + 237] != 0x82
		|| buf[UDP_DATA_P + 238] != 0x53
		|| buf[UDP_DATA_P + 239] != 0x63
		)
	{
		dhcpAckType = 0;
		dbglog("receive_udp_hdcp_ack not dhcp magic\r\n");
	}
	//is it dhcp offer
	pos = 240;
	//find hdcp option 53,check is it value 02
	{
		uint8 option;
		uint8 optionLen;
		option = buf[UDP_DATA_P + pos];
		while(option != 0xff && (UDP_DATA_P + pos + 2)<len)
		{
			if(option == 53)
			{
				if(buf[UDP_DATA_P + pos + 2] == 5)
				{
					dhcpAckType = 1;
					dbglog("receive_udp_hdcp_ack dhcp ack\r\n");
				}
				else if(buf[UDP_DATA_P + pos + 2] == 6)
				{
					dhcpAckType = 2;
					dbglog("receive_udp_hdcp_ack dhcp nack\r\n");
				}
				else
				{
					dhcpAckType = 0;
					dbglog("receive_udp_hdcp_ack not dhcp nack\r\n");
				}
				break;
			}
			optionLen = buf[UDP_DATA_P + pos + 1];
			pos += 2 + optionLen;
			option = buf[UDP_DATA_P + pos];
		}
	}

	if(dhcpAckType == 1)
	{
		//parse the info
		for(i=0;i<4;i++)
			ipaddr[i] = buf[UDP_DATA_P + 16 + i];
		dbglog("receive_udp_hdcp_ack ipaddr=%x%x%x%x\r\n",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3]);
		dhcpServerIp = 0;
		for(i=0;i<4;i++)
			dhcpServerIp |= (buf[UDP_DATA_P + 20 + i]<<((3-i)*8));
		dbglog("receive_udp_hdcp_ack dhcpServerIp=%x\r\n",dhcpServerIp);

		pos = 240;
		//parse hdcp option 
		{
			uint8 option;
			uint8 optionLen;
			option = buf[UDP_DATA_P + pos];
			//dbglog("receive_udp_hdcp_offer option=%x\r\n",option);
			while(option != 0xff && (UDP_DATA_P + pos + 2)<len)
			{
				dbglog("receive_udp_hdcp_ack option=%x\r\n",option);
				if(option == 1)
				{
					submask = 0;
					for(i=0;i<4;i++)
						submask |= (buf[UDP_DATA_P + pos + 2 + i]<<((3-i)*8));
					dbglog("receive_udp_hdcp_ack submask=%x\r\n",submask);
				}
				else if(option == 3)
				{
					router = 0;
					for(i=0;i<4;i++)
						router |= (buf[UDP_DATA_P + pos + 2 + i]<<((3-i)*8));
					dbglog("receive_udp_hdcp_ack router=%x\r\n",router);
				}
				else if(option == 51)
				{
					leaseTime = 0;
					for(i=0;i<4;i++)
						leaseTime |= (buf[UDP_DATA_P + pos + 2 + i]<<((3-i)*8));
					dbglog("receive_udp_hdcp_ack leaseTime=%x\r\n",leaseTime);
				}
				else if(option == 54)
				{
					dhcpServerIp = 0;
					for(i=0;i<4;i++)
						dhcpServerIp |= (buf[UDP_DATA_P + pos + 2 + i]<<((3-i)*8));
					dbglog("receive_udp_hdcp_ack dhcpServerIp=%x\r\n",dhcpServerIp);
				}
				else if(option == 6)
				{
					optionLen = buf[UDP_DATA_P + pos + 1];
					dnsServer1 = 0;
					for(i=0;i<4;i++)
						dnsServer1 |= (buf[UDP_DATA_P + pos + 2 + i]<<((3-i)*8));
					dbglog("receive_udp_hdcp_ack dnsServer1=%x\r\n",dnsServer1);
					if(optionLen > 4)
					{
						dnsServer2 = 0;
						for(i=0;i<4;i++)
							dnsServer2 |= (buf[UDP_DATA_P + pos + 6 + i]<<((3-i)*8));
						dbglog("receive_udp_hdcp_ack dnsServer2=%x\r\n",dnsServer2);
					}
				}
				optionLen = buf[UDP_DATA_P + pos + 1];
				pos += 2 + optionLen;
				option = buf[UDP_DATA_P + pos];
			}
		}
	}
	

	return dhcpAckType;
}
void getMyIp(unsigned char *buf)
{
	int i;
	for(i=0;i<4;i++)
		buf[i] = ipaddr[i];
}
unsigned int startLeaseTime=0;
void SetLeaseTime(uint32 leaseTime)
{
	startLeaseTime = leaseTime;
}
uint8 CheckLeaseTime(uint32 curTime)
{
	uint32 consumeTime = curTime - startLeaseTime;
	if(consumeTime > leaseTime *0.8)
	{
		return 2;
	}
	else if(consumeTime > leaseTime *0.5)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
#endif
void make_udp_test(unsigned char *buf)
{
	int i;
	unsigned int ck;
	unsigned int len;
	make_eth(buf);
	//  broadcast mac address is ff ff ff ff ff ff
    for(i=0; i<6; i++)
    {         
        buf[ETH_DST_MAC+i] = 0xff;
    }
	#if 0
	buf[ETH_DST_MAC+0] = 0x1c;
	buf[ETH_DST_MAC+1] = 0x6f;
	buf[ETH_DST_MAC+2] = 0x65;
	buf[ETH_DST_MAC+3] = 0xd6;
	buf[ETH_DST_MAC+4] = 0xc8;
	buf[ETH_DST_MAC+5] = 0xbe;
	#endif
	
	buf[ETH_TYPE_L_P] = ETHTYPE_IP_L_V;
	buf[ETH_TYPE_H_P] = ETHTYPE_IP_H_V;

	buf[IP_P] = 0x45;
	buf[IP_TOS_P] = 0x00;
	//ip包总长度，包括ip头
	i= IP_HEADER_LEN+UDP_HEADER_LEN+2;
	buf[IP_TOTLEN_H_P] = i>>8;// 10e
	buf[IP_TOTLEN_L_P] = i;
	buf[IP_ID_H_P] = 0x01;//
	buf[IP_ID_L_P] = 0x51;
	buf[IP_FLAGS_P] = 0x40;
	buf[IP_FLAGS_P+1] = 0x00;
	buf[IP_OFFSET_P] = 0x00;
	buf[IP_TTL_P] = 64;

	buf[IP_PROTO_P] = 0x11;

	buf[IP_CHECKSUM_P] = 0x00;//ip head checksum,2 bytes
	buf[IP_CHECKSUM_P+1] = 0x00;

	buf[IP_SRC_P] = 0xc0;
	buf[IP_SRC_P+1] = 0xa8;
	buf[IP_SRC_P+2] = 0x01;
	buf[IP_SRC_P+3] = 0x66;
	make_ip(buf);
	//buf[IP_DST_P] = 0xff;
	//buf[IP_DST_P+1] = 0xff;
	//buf[IP_DST_P+2] = 0xff;
	//buf[IP_DST_P+3] = 0xff;
	
	buf[UDP_SRC_PORT_H_P] = 0xd6;
	buf[UDP_SRC_PORT_L_P] = 0x52;
	buf[UDP_DST_PORT_H_P] = 0x04;
	buf[UDP_DST_PORT_L_P] = 0xb0;
	len = UDP_HEADER_LEN + 2;
	buf[UDP_LEN_H_P] = len>>8;
	buf[UDP_LEN_L_P] = len;
	buf[UDP_CHECKSUM_H_P] = 0x00;// calc by the end
	buf[UDP_CHECKSUM_L_P] = 0x00;//

	buf[UDP_DATA_P + 0x00] = 'h';
	buf[UDP_DATA_P + 0x01] = 'i';
	buf[UDP_DATA_P + 0x02] = 0;


	buf[IP_CHECKSUM_P] = 0x00;//ip head checksum,2 bytes
	buf[IP_CHECKSUM_P+1] = 0x00;
	ck=checksum(&buf[IP_P], IP_HEADER_LEN,0);
	dbglog("IP_CHECKSUM=%x\r\n",ck);
	buf[IP_CHECKSUM_P]=ck>>8;
	buf[IP_CHECKSUM_P+1]=ck& 0xff;

	ck=checksum(&buf[IP_SRC_P], 16 + 2,1);
	buf[UDP_CHECKSUM_H_P] = ck>>8;// calc by the end
	buf[UDP_CHECKSUM_L_P] = ck&0xff;


    dbglog("神舟III号 make_udp_test\r\n");
	enc28j60PacketSend(34 +UDP_HEADER_LEN+2,buf);
}
uint8 TcpCheckSumOk(uint8 *buf)
{
	int dlen = info_data_len;
	uint32 checksumReceived = (buf[TCP_CHECKSUM_H_P]<<8)|buf[TCP_CHECKSUM_L_P];
	uint32 checksumOur;
	buf[TCP_CHECKSUM_H_P]=0;
	buf[TCP_CHECKSUM_L_P]=0;
	checksumOur = 0xffff&checksum(&buf[IP_SRC_P], 8+TCP_HEADER_LEN_PLAIN+dlen,2);
	//dbglog("dlen=%d,0x%x\r\n",dlen,dlen);
	//dbglog("TcpCheckSumOk=%x,%x\r\n",checksumReceived,checksumOur);
	return checksumReceived == checksumOur;
}
/* end of ip_arp_udp.c */
