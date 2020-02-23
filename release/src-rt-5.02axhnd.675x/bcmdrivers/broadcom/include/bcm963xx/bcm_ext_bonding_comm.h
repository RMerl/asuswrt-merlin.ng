/*
<:copyright-broadcom 
 
 Copyright (c) 2011 Broadcom Corporation 
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
#ifndef BCM_EXT_BONDING_COMM_H

#define BCM_EXT_BONDING_COMM_H

#include "bcmtypes.h"
#include <linux/skbuff.h>
#include <linux/netdevice.h>

#include "bcmxtmcfg.h"

/* The buffer descriptor used by EXT_BONDING_COMM is just an alias for an sk_buff.
   In the future, we might want to add more private information into this
   header and thus would define EXT_BONDING_COMM_BufferDesc as a structure containing at
   least a pointer to the sk_buff.  We should also add information in the
   sk_buff data to find back the EXT_BONDING_COMM descriptor.
*/
typedef struct sk_buff EXT_BONDING_COMM_BufferDesc;

typedef void (*PrivatePacketTypeReceiveCb) (EXT_BONDING_COMM_BufferDesc *bufp, void *data);

struct EXT_BONDING_COMM_packet_type
{
  struct packet_type         PacketType;
  PrivatePacketTypeReceiveCb func;
  void                       *data;
  unsigned long              received;
};

/* Protocol Types */

#define EXT_BONDING_DEV_DISCOVERY 			(0x831)
#define EXT_BONDING_DEV_LINE_STATUS		   (0x832)
#define EXT_BONDING_DEV_LINE_QUERY 		   (0x833)
#define EXT_BONDING_DSL_STAT				   (0x834)
#define EXT_BONDING_DSL_CMD				   (0x835)
#define EXT_BONDING_DSL_IOCTL_CMD			(0x836)
#define EXT_BONDING_DSL_IOCTL_RSP			(0x837)
/* DSL Types can be added here */


/* APIs */

/* To register protocol types per application. An application may register one
 * or more protocol types.
 * Return value to be stored by the application and used upon un-registration.
 */
struct EXT_BONDING_COMM_packet_type * EXT_BONDING_COMM_registerPacketType (uint16 type, 
                                                                           PrivatePacketTypeReceiveCb func, 
                                                                           void *data) ;

/* To unregister application packet type */
void EXT_BONDING_COMM_unregisterPacketType (struct EXT_BONDING_COMM_packet_type *p) ;

/* Fully composed Ethernet frame (Expected to contain the protocol specific to
 * application + corresponding data) to be sent to the Remote device */
void EXT_BONDING_COMM_sendFrame (EXT_BONDING_COMM_BufferDesc *bufp, UINT16 usProtocolType) ;

/* If application wants to send data to remote device, it needs to allocate
 * buffer thro this API i/f.
 */
EXT_BONDING_COMM_BufferDesc *EXT_BONDING_COMM_allocateBuffer (int size) ;

/* If application is done with processing of data from the remote device, this
 * function is to be called to free the buffer and data.
 */
void EXT_BONDING_COMM_freeFrameBuffer (EXT_BONDING_COMM_BufferDesc *bufp) ;

/* To change the data pointer of the buffer by the offset value */
uint8 *EXT_BONDING_COMM_changeBufferOffsetSize (EXT_BONDING_COMM_BufferDesc *bufp,
                                                int offsetChange, int size);

/* Equivalent to skb_pull functionality for the buffer desc */
uint8 *EXT_BONDING_COMM_pullBuffer (EXT_BONDING_COMM_BufferDesc *bufp, int len) ;

/* Returns the buffer desc from the given skb */
EXT_BONDING_COMM_BufferDesc *EXT_BONDING_COMM_getDescriptor (struct sk_buff *skb) ;

/* Returns the OS skb buffer from the given buffer desc. */
struct sk_buff *EXT_BONDING_COMM_getOSDescriptor (EXT_BONDING_COMM_BufferDesc *bufp) ;

/* Returns the length of the buffer desc */
int EXT_BONDING_COMM_getBufferSize (EXT_BONDING_COMM_BufferDesc *bufp) ;

/* Returns the data pointer in the buffer desc */
uint8 *EXT_BONDING_COMM_getData (EXT_BONDING_COMM_BufferDesc *bufp) ;

/* Equivalent to skb_push() functionality */
EXT_BONDING_COMM_BufferDesc *EXT_BONDING_COMM_pushBuffer (EXT_BONDING_COMM_BufferDesc *bufp, int len) ;

/* Equivalent to skb_push() functionality */
EXT_BONDING_COMM_BufferDesc *EXT_BONDING_COMM_pushBuffer (EXT_BONDING_COMM_BufferDesc *bufp, int len) ;

/* Called when local link status changes by the xDSL-XTM interfaces */
void EXT_BONDING_COMM_SetInterfaceLinkInfo( UINT32 ulPortId, PXTM_INTERFACE_LINK_INFO pLinkInfo ) ;

#endif /* end of BCM_EXT_BONDING_COMM_H */
