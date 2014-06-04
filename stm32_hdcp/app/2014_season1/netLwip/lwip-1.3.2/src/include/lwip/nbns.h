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
 * NBNS.H
 *
 * The NetbiosNameService waiting on UDP-Port 137 for a NBNS-Query.
 * If this Query matches with the HostName, a respond is send.
 * All other NetBios Telegramms are ignored. ( like Samba, PrinterServices , 
etc. )
 */

/*-----------------------------------------------------------------------------
 * RFC 1001 - Protocol Standard for a NetBIOS Service on a TCP/UDP Transport: 
Concepts and methods
 * RFC 1002 - Protocol standard for a NetBIOS service on a TCP/UDP transport: 
Detailed specifications
 *----------------------------------------------------------------------------*/

#ifndef __LWIP_NBNS_H__
#define __LWIP_NBNS_H__

#include "lwip/opt.h"

#if LWIP_NBNS /* don't build if not configured for use in lwipopts.h */



void nbns_init(u8_t*  Hostname,u8_t* Hostname_Len, struct netif *netif);

/* Values of the opcode field */

#define OPCODE_R 0x8000

/*OPCODE        1-4   Operation specifier:
                         0 = query
                         5 = registration
                         6 = release
                         7 = WACK
                         8 = refresh */

#define OPCODE_MASK             0x7800
#define OPCODE_QUERY            0x0000
#define OPCODE_REGISTRATION 0x2800
#define OPCODE_RELEASE          0x3000
#define OPCODE_WACK                     0x3800
#define OPCODE_REFRESH          0x4000
                                                 

/* NM_FLAGS subfield bits */ 
#define NM_AA_BIT         0x0400  /* Authoritative Answer */
#define NM_TR_BIT         0x0200  /* TRuncation flag      */
#define NM_RD_BIT         0x0100  /* Recursion Desired    */
#define NM_RA_BIT         0x0080  /* Recursion Available  */
#define NM_B_BIT          0x0010  /* Broadcast flag       */

/* Return Codes */
#define RCODE_POS_RSP     0x0000  /* Positive Response    */
#define RCODE_FMT_ERR     0x0001  /* Format Error         */
#define RCODE_SRV_ERR     0x0002  /* Server failure       */ 
#define RCODE_NAM_ERR     0x0003  /* Name Not Found       */ 
#define RCODE_IMP_ERR     0x0004  /* Unsupported request  */
#define RCODE_RFS_ERR     0x0005  /* Refused              */
#define RCODE_ACT_ERR     0x0006  /* Active error         */
#define RCODE_CFT_ERR     0x0007  /* Name in conflict     */
#define RCODE_MASK        0x0007  /* Mask                 */

/* Used to set the record count fields. */
#define QUERYREC          0x1000  /* Query Record         */
#define ANSREC            0x0100  /* Answer Record        */
#define NSREC             0x0010  /* NS Rec (never used)  */
#define ADDREC            0x0001  /* Additional Record    */


/* RDATA NB_FLAGS. */
#define GROUP_BIT     0x8000  /* Group indicator      */
#define ONT_B         0x0000  /* Broadcast node       */
#define ONT_P         0x2000  /* Point-to-point node  */
#define ONT_M         0x4000  /* Mixed mode node      */
#define ONT_H         0x6000  /* MS Hybrid mode node  */
#define ONT_MASK      0x6000  /* Mask                 */

/* RDATA NAME_FLAGS. */
#define DRG           0x0100  /* Deregister.          */
#define CNF           0x0800  /* Conflict.            */
#define ACT           0x0400  /* Active.              */
#define PRM           0x0200  /* Permanent.           */




#endif /* LWIP_NBNS */

#endif /* __LWIP_NBNS_H__ */