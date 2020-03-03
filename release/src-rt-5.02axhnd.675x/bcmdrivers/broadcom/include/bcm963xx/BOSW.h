/*
<:copyright-broadcom 
 
 Copyright (c) 2004 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
#ifndef BOSW_H

#define BOSW_H

#include "bcmtypes.h"
#include <linux/skbuff.h>
#include <linux/netdevice.h>

#define MASTER_INTF 0
#define SW_BRIDGE 1

/* The buffer descriptor used by BOSW is just an alias for an sk_buff.
   In the future, we might want to add more private information into this
   header and thus would define BOSW_BufferDesc as a structure containing at
   least a pointer to the sk_buff.  We should also add information in the
   sk_buff data to find back the BOSW descriptor.
*/
typedef struct sk_buff BOSW_BufferDesc;

typedef void (*PrivatePacketTypeReceiveCb) (BOSW_BufferDesc *bufp, void *data);

struct BOSW_packet_type
{
  struct packet_type PacketType;
  PrivatePacketTypeReceiveCb func;
  void *data;
  unsigned long received;
};

struct BOSW_packet_type * BOSW_registerPacketType(uint16 type, PrivatePacketTypeReceiveCb func, void *data);
void BOSW_unregisterPacketType(struct BOSW_packet_type *p);
int BOSW_start(void);
int BOSW_init(void);
void BOSW_exit(void);
void BOSW_sendFrame(BOSW_BufferDesc *bufp, int dev);
uint32 BOSW_getTimeStamp(void);
void BOSW_gettimeofday(struct timeval *tvp, void *dummy);
BOSW_BufferDesc *BOSW_allocateBuffer(int size);
void BOSW_freeFrameBuffer(BOSW_BufferDesc *bufp);
uint8 *BOSW_changeBufferOffsetSize(BOSW_BufferDesc *bufp, int offsetChange, int size);
uint8 *BOSW_pullBuffer(BOSW_BufferDesc *bufp, int len);
BOSW_BufferDesc *BOSW_getBOSWDescriptor(struct sk_buff *skb);
struct sk_buff *BOSW_getOSDescriptor(BOSW_BufferDesc *bufp);
int BOSW_getBufferSize(BOSW_BufferDesc *bufp);
uint8 *BOSW_getData(BOSW_BufferDesc *bufp);
BOSW_BufferDesc *BOSW_allocateSmallFrameBuffer(void);
BOSW_BufferDesc *BOSW_allocateBigFrameBuffer(void);
BOSW_BufferDesc *BOSW_pushBuffer(BOSW_BufferDesc *bufp, int len);
void BOSW_BufferMgntInit(void);
uint16 *BOSW_getOwnMac(void);

#endif
