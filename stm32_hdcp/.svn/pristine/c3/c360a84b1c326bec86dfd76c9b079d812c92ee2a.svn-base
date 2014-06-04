/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher 
 * Copyright: GPL V2
 *
 * IP/ARP/UDP/TCP functions
 *
 * Chip type           : ATMEGA88 with ENC28J60
 *********************************************/


/*********************************************
 * modified: 2007-08-08
 * Author  : awake
 * Copyright: GPL V2
 * http://www.icdev.com.cn/?2213/
 * Host chip: ADUC7026
**********************************************/

#include "utilities.h"


//@{
#ifndef IP_ARP_UDP_TCP_H
#define IP_ARP_UDP_TCP_H

// you must call this function once before you use any of the other functions:
extern void init_ip_arp_udp_tcp(unsigned char *mymac,unsigned char *myip,unsigned short wwwp);
//
#if defined(TEST_DHCP)
extern unsigned char eth_type_is_arp_and_dhcp(unsigned char *buf,unsigned  int len);
#endif
unsigned char is_ip_multibroadcast(unsigned char *buf);
extern unsigned char isGarbage(unsigned char *buf);
extern unsigned char eth_type_is_arp_and_my_ip(unsigned char *buf,unsigned int len);
extern unsigned char eth_type_is_ip_and_my_ip(unsigned char *buf,unsigned int len);
#if defined(TEST_DHCP)
extern void make_arp_answer_from_DHCP_request(unsigned char *buf);
#endif
extern void make_arp_answer_from_request(unsigned char *buf);
extern void make_echo_reply_from_request(unsigned char *buf,unsigned int len);
extern void make_udp_reply_from_request(unsigned char *buf,char *data,unsigned int datalen,unsigned int port);


extern void make_tcp_synack_from_syn(unsigned char *buf);
extern void init_len_info(unsigned char *buf);
extern unsigned int get_tcp_data_pointer(void);
extern unsigned int fill_tcp_data_p(unsigned char *buf,unsigned int pos, const unsigned char *progmem_s);
extern unsigned int fill_tcp_data(unsigned char *buf,unsigned int pos, const char *s);
extern void make_tcp_ack_from_any(unsigned char *buf);
extern unsigned int make_tcp_ack_with_data(unsigned char *buf,unsigned int dlen);
extern unsigned  int make_tcp_ack_with_finish(unsigned char *buf);
extern unsigned  int get_tcp_ack_seq(unsigned char *buf);

#define LINK_HEAD_LEN	0x36	//连接的buf足够的头部信息，以便通知反馈用
typedef struct{
	uint8	isUsed;
	uint16	port;
	uint32	ip;
	uint32	seq;
	uint32	time;
	uint8	buf[LINK_HEAD_LEN];
}LINKED_INFO;
#define MAX_LINK	10 // 10
extern LINKED_INFO linkedInfo[MAX_LINK];
extern uint32 get_ip_dest(unsigned char *buf);
extern uint32 get_ip_src(unsigned char *buf);
extern uint16 get_port_dest(unsigned char *buf);
extern uint16 get_port_src(unsigned char *buf);
extern void new_tcp_seq(unsigned char *buf);
extern void delete_tcp_seq(unsigned char *buf);
extern uint32 get_tcp_seq(unsigned char *buf);
extern void add_tcp_seq(unsigned char *buf,uint32 num);
extern void update_tcp_ackseq(unsigned char *buf);
extern void update_tcp_buf(unsigned char *buf);
extern void make_udp_hdcp_discover(unsigned char *buf);
uint8 receive_udp_hdcp_offer(unsigned char *buf,int len);
void make_udp_hdcp_request(unsigned char *buf);
uint8 receive_udp_hdcp_ack(unsigned char *buf,int len);
void getMyIp(unsigned char *buf);
void SetLeaseTime(uint32 leaseTime);
uint8 CheckLeaseTime(uint32 curTime);

extern void make_udp_test(unsigned char *buf);
extern uint8 TcpCheckSumOk(uint8 *buf);



#endif /* IP_ARP_UDP_TCP_H */
//@}
