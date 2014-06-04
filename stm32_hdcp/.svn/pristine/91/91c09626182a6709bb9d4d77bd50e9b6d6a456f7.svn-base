/**
 * @file
 * NBNS - host name to NetBios resolver.
 *
 */

/**

 * This file implements a simple NetBios Server for the HostName.
 * All other NetBios querys are ignored

 * by Oliver Schindler November 2008

 * uIP version Copyright (c) 2002-2003, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * NBNS.C
 *
 * The NetbiosNameService waiting on UDP-Port 137 for a NBNS-Query.
 * If this Query matches with the HostName, a resond is send.
 * All other NetBios Telegramms are ignored. ( like Samba, PrinterServices , 
etc. )
 */

/*-----------------------------------------------------------------------------
 * RFC 1001 - Protocol Standard for a NetBIOS Service on a TCP/UDP Transport: 
Concepts and methods
 * RFC 1002 - Protocol standard for a NetBIOS service on a TCP/UDP transport: 
Detailed specifications
 *----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * Includes
 *----------------------------------------------------------------------------*/

#include "lwip/opt.h"

#if LWIP_NBNS /* don't build if not configured for use in lwipopts.h */
#if LWIP_UDP                        

#include "lwip/udp.h"
#include "lwip/mem.h"
#include "lwip/nbns.h"


#include <string.h>
#include <stdlib.h> 


/** NBNS server port address */
#ifndef NBNS_CLIENT_PORT
#define NBNS_CLIENT_PORT           137
#endif

#define NETBIOS_NAME_LEN                   32

#if (NBNS_USES_STATIC_BUF == 1)
static u8_t             pHostname[NETBIOS_NAME_LEN];
#endif /* (NBNS_USES_STATIC_BUF == 1) */



/** NBNS message header */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN

struct nbns_hdr {
  u16_t id;
  u16_t flags;
  u16_t numquestions;
  u16_t numanswers;
  u16_t numauthrr;
  u16_t numextrarr;
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif


/** NBNS rdata field */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
PACK_STRUCT_BEGIN

struct nbns_rdata {
  u16_t len;
  u16_t nbflags;
  struct ip_addr addr;
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif


/* forward declarations */
static void nbns_recv(void *s, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port);

/*-----------------------------------------------------------------------------
 * Globales
 *----------------------------------------------------------------------------*/

/* NBNS variables */
static struct udp_pcb        *nbns_pcb;
struct netif *pnetif=NULL;
static u8_t*            pnbns_hostname=NULL;
u8_t* pnbns_hostname_len=NULL;

/**
 * Initialize the resolver: set up the UDP pcb 
 */
void nbns_init(u8_t* Hostname,u8_t* Hostname_Len, struct netif *netif)
{
  

  LWIP_DEBUGF(NBNS_DEBUG, ("nbns_init: initializing\n"));
  pnbns_hostname_len=Hostname_Len;
  if (*pnbns_hostname_len>NETBIOS_NAME_LEN) {
          LWIP_DEBUGF(NBNS_DEBUG, ("nbns_init:ERROR Hostname should be max 32 chars \n"));
          return;
  }
  pnbns_hostname=Hostname;
  pnetif= netif;

  /* if nbns client not yet initialized... */
  if (nbns_pcb == NULL) {
    nbns_pcb = udp_new();

    if (nbns_pcb != NULL) {

      /* initialize NBNS client */
      udp_bind(nbns_pcb, IP_ADDR_ANY, NBNS_CLIENT_PORT);
      udp_recv(nbns_pcb, nbns_recv, NULL);


    }
  }
}

static u8_t nbns_compare_name(u8_t *query, u8_t *response,u8_t len)
{
  u8_t i;

  for (i=0;i<len;i++) {
    if (*response==0){
                return 1;
        }
        if (*query==0){
                return 1;
        }
        if (tolower(query[i])!=tolower(response[i])) {
                return 1;
        }
  } 

  return 0;
}





#define NBNS_TTL_POS sizeof(struct nbns_hdr)+1+NETBIOS_NAME_LEN+1+2+2   
#define NBNS_RDATA_POS NBNS_TTL_POS + 4         

#define NBNS_MSGLEN_QUERY_RESPONSE 70 // NBNS_RDATA_POS+sizeof(struct nbns_rdata)


/**
 * Receive input function for NBNS query packets arriving for the NameService 
UDP pcb.
 *
 * @params see udp.h
 */
static void
nbns_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr,u16_t port)
{
  err_t err;
  u16_t unique_id,i;
  char c1,c2;
  char *pNamefield;
  struct nbns_hdr *hdr;
  struct nbns_rdata* rdata ;
  u16_t* ttl1,*ttl2;
  struct pbuf *prep;
  u16_t nquestions, nanswers,opcode,nm_flags,rcode,name_len;
#if (NBNS_USES_STATIC_BUF == 0)
  u8_t          pHostname[NETBIOS_NAME_LEN];
#endif /* (NBNS_USES_STATIC_BUF == 0) */
#if (NBNS_USES_STATIC_BUF == 2)
  u8_t* pHostname;
#endif /* (NBNS_USES_STATIC_BUF == 2) */

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(pcb);
 

  /* is the nbns message too big ? */
  if (p->tot_len > NBNS_MSG_SIZE) {
    LWIP_DEBUGF(NBNS_DEBUG, ("nbns_recv: pbuf too big\n"));
    /* free pbuf and return */
    goto memerr1;
  }

  /* is the nbns message big enough ? */
  if (p->tot_len < (1+NETBIOS_NAME_LEN+1+2+2+sizeof(struct nbns_hdr))) {
    LWIP_DEBUGF(NBNS_DEBUG, ("nbns_recv: pbuf too small\n"));
    /* free pbuf and return */
    goto memerr1;
  }

#if (NBNS_USES_STATIC_BUF == 2)
  pHostname = mem_malloc(NETBIOS_NAME_LEN);
  if (pHostname == NULL) {
        LWIP_DEBUGF(NBNS_DEBUG, ("nbns_recv: mem_malloc error\n"));
        /* free pbuf and return */
        goto memerr2;
  }

#endif /* (NBNS_USES_STATIC_BUF == 2) */

  /* The content of the Telegramm is only decoded for information purpses.
     Later we use a copy of the orignal message */
 
   hdr = (struct nbns_hdr*)p->payload;
   unique_id = htons(hdr->id);                          /* this is a unique ID 
, which has to be sent back */

   /* We only care about the question(s) and the answers. The authrr
         and the extrarr are simply discarded. */
   nquestions = htons(hdr->numquestions);
   nanswers   = htons(hdr->numanswers);

   /* Decode what the client wants from us */
   opcode        =  htons(hdr->flags) & 0xf800;
   nm_flags      =  htons(hdr->flags) & 0x03f0;
   rcode                 =  htons(hdr->flags) & 0x000f;

   if (opcode & OPCODE_R ) { /* This is a response packet, so don't care about 
*/
          /* deallocate memory and return */
          goto memerr2;
   }

   switch (opcode & OPCODE_MASK) {
     case OPCODE_QUERY:
          /* There is someone looking for a host with a certain name ... */
          break;

     case OPCODE_REGISTRATION:
     case OPCODE_RELEASE:     
     case OPCODE_WACK:        
     case OPCODE_REFRESH:
     default:
          /* Nothing of interest for us */
          /* deallocate memory and return */
          goto memerr2;
          break;        /* just for the compiler .... */
   }

   pNamefield=((u8_t*)p->payload)+sizeof(struct nbns_hdr);
   name_len=pNamefield[0];
   if (name_len!=NETBIOS_NAME_LEN) {
          /* Hmm , this is not a netbios name ... */
           goto memerr2;

   }
   memset (pHostname,0,NETBIOS_NAME_LEN);
   pNamefield++;
   /* Decode the hostname */
   for (i=0;i<name_len;i+=2) {
          c1=(pNamefield[i]&0x3f)-1;
          c2=(pNamefield[i+1]&0x3f)-1;
       pHostname[i/2]=(c1<<4)+(c2);                                             
                         
   }

   if (!nbns_compare_name(pHostname,pnbns_hostname,*pnbns_hostname_len)) {
          /* Hey , this is ours .. we have to send a reply */

          LWIP_DEBUGF(NBNS_DEBUG, ("nbns_recv: Got a query for our hostname\n"));

          /* reallocate of pbuf isn't implemented to grow, so we have allocate a
             new buffer ,to have space for our IP-Adress */

          prep = pbuf_alloc(PBUF_TRANSPORT, NBNS_MSGLEN_QUERY_RESPONSE, PBUF_RAM);

          /* clone the original message, for the transport_id,hostname and 
flags*/
          pbuf_copy_partial(p, prep->payload, p->tot_len, 0);

          
          /* set pointers for the different parts in the message */
          hdr = (struct nbns_hdr*)prep->payload;

          /* ttl is 32-bit, but we can't guarantee that is aligned on a 32 bit 
boundary,
             so split it up into two halves */
          ttl1 = (u16_t*)(((u8_t*)prep->payload)+NBNS_TTL_POS);
          ttl2 = (u16_t*)(((u8_t*)prep->payload)+NBNS_TTL_POS+2);

          /* This is were our IP-Adress goes to */
          rdata = (struct nbns_rdata*) (((u8_t*)prep->payload)+NBNS_RDATA_POS);

          /* Set the response Flag, and the number of answers  */

          opcode=OPCODE_R+OPCODE_QUERY;
          nm_flags=NM_AA_BIT+NM_RD_BIT;
          rcode=RCODE_POS_RSP; /* Positive respones */

          hdr->flags =ntohs(opcode+nm_flags+rcode); 
          hdr->numquestions=ntohs(0);
          hdr->numanswers  =ntohs(1);

          *ttl1=ntohs(0);
          *ttl2=ntohs(0);       /* Infinite time to live */
          rdata->len=ntohs(2+sizeof(struct ip_addr)); 
          rdata->nbflags=ntohs(ONT_B);          /* Broadcast Node */
        
          memcpy((unsigned char*)&rdata->addr,(unsigned char*)&pnetif->ip_addr,sizeof(struct ip_addr));


          /* It's better to answer on the port , we got the query, maybe some 
strange
             port-translation is in between */
          err = udp_sendto(nbns_pcb, prep, addr,port/* NBNS_CLIENT_PORT*/);

          /* cleaning up */
          pbuf_free(prep);
   }





  /* deallocate memory and return */
  //goto memerr2;



memerr2:
#if (NBNS_USES_STATIC_BUF == 2)
  /* free Hostname buffer */
 if(pHostname!=NULL){
  mem_free(pHostname);
 }
#endif /* (NBNS_USES_STATIC_BUF == 2) */

memerr1:
  /* free pbuf */
  pbuf_free(p);
  return;
}


#else
#error "UDP is needed !"
#endif
#endif /* LWIP_NBNS */