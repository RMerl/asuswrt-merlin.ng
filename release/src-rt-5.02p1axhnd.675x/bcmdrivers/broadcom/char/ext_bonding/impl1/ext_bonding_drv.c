/*
* <:copyright-BRCM:2013:proprietary:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
:>
*/
#include <linux/kernel.h>
//#include <linux/tqueue.h>
#include <linux/netdevice.h>
#include <linux/notifier.h>
#include <linux/param.h>
#include <linux/time.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <asm/system.h>

#include <linux/skbuff.h>
#include "bcmtypes.h"
#include "bcm_map.h"
#include "board.h"
#include "bcm_OS_Deps.h"

#include "bcm_ext_bonding_comm.h"

#define EXT_BONDING_COMM_DRV_MAJOR  311
#define SW_BRIDGE          0

#define DEV_TYPE_UNKNOWN      0x0
#define DEV_TYPE_MASTER       0x1
#define DEV_TYPE_SLAVE        0x2

struct  DevDiscovery
{
  UINT32 devType ;
};

typedef struct DevDiscovery DevDiscoveryInfo;

struct  DevLineStatus
{
  UINT32 ulPortId ;
  XTM_INTERFACE_LINK_INFO LinkInfo ;
};

typedef struct DevLineStatus DevLineStatusInfo;

/* Network devices structure */
struct net_device *NetworkDevices[2];

static struct timer_list extbonding_timer;

struct MacAddress 
{
  uint16 mac[3];
};

typedef struct MacAddress MacAddress;

struct EthHeader
{
  MacAddress destination;
  MacAddress source;
  uint16     etherType;
  uint16     dummy ;
};

typedef struct EthHeader EthHeader;

/* Definitions and global variables */
MacAddress        ownAddress, remoteAddress ;
MacAddress        bcastAddress = {{0xFFFF, 0xFFFF, 0xFFFF}} ;
DevLineStatusInfo slaveLineStatusInfo ;

UINT32 remoteAddressKnown = 0 ;

struct extbonding_packet_type
{
  uint16 type;
  struct EXT_BONDING_COMM_packet_type *PacketTypep;
};

static struct extbonding_packet_type devDiscPacketType ;
static struct extbonding_packet_type stateReqPacketType ;
static struct extbonding_packet_type stateRespPacketType ;


/* Local utility functions */
void constructEthHeader (UINT8 *datap, UINT16 usProtocolType);
int GetDeviceType (void) ;
void ExtBonding_handleDevLineStatus (EXT_BONDING_COMM_BufferDesc *bufp, void *data) ;
void ExtBonding_handleDevLineQuery (EXT_BONDING_COMM_BufferDesc *bufp, void *data) ;
void sendLineStatus (UINT32 ulPortId, PXTM_INTERFACE_LINK_INFO pLinkInfo) ;

#if 0
static int bcm_ext_bonding_comm_close( struct inode *inode, struct file *filp );
#endif
void EXT_BONDING_COMM_BufferMgntInit (void) ;
void EXT_BONDING_COMM_timer_cb(unsigned long data) ;
void ExtBonding_handleDeviceDiscovery (EXT_BONDING_COMM_BufferDesc *bufp, void *data) ;

int EXT_BONDING_COMM_init(void) ;
void EXT_BONDING_COMM_exit(void) ;
int EXT_BONDING_COMM_start(void) ;
static int bcm_ext_bonding_comm_open( struct inode *inode, struct file *filp );
/* Returns the source mac part for the applications which is the local MAC.
 * NULL if unknown.
 */
uint16 *EXT_BONDING_COMM_getOwnAddress (void) ;

/* Returns the destination mac part for the applications which is the remote
 * end MAC.
 * NULL, if unknown */
uint16 *EXT_BONDING_COMM_getRemoteAddress (void) ;

/* Returns the bcast mac address
 * NULL, if unknown */
uint16 *EXT_BONDING_COMM_getBcastAddress (void) ;

/* Globals. */
static struct file_operations ext_bonding_comm_fops =
{
    .open   = bcm_ext_bonding_comm_open,
#if 0
    .close  = bcm_ext_bonding_comm_close,
#endif
};

/* Notifier structure */
static struct notifier_block EXT_BONDING_COMM_notifier;

int EXT_BONDING_COMM_notify (struct notifier_block *self, unsigned long n, void *data)
{
  struct net_device *dev;

  dev = NETDEV_NOTIFIER_GET_DEV(data);
  if (n == NETDEV_REGISTER) {

    if (strncmp(dev->name, "br0", 3) == 0) {

      printk("bcm_ext_bonding: Bridge br0 registered\n");
      NetworkDevices[SW_BRIDGE] = dev;
		if (NetworkDevices[SW_BRIDGE] == NULL)
		{
			printk("bcm_ext_bonding: No device named bridge\n");
			return -ENODEV;
		}
		else {
  			EXT_BONDING_COMM_start () ;
		}
    }
  } /* if (n) */

  return NOTIFY_DONE;
}


uint16 *EXT_BONDING_COMM_getOwnAddress(void)
{
	memcpy ((UINT8 *) &ownAddress.mac[0], NetworkDevices[SW_BRIDGE]->dev_addr, 6) ;
	return (uint16 *) &ownAddress.mac[0];
}

uint16 *EXT_BONDING_COMM_getRemoteAddress(void)
{
	return (uint16 *) &remoteAddress.mac[0];
}

uint16 *EXT_BONDING_COMM_getBcastAddress(void)
{
	return (uint16 *) &bcastAddress.mac[0];
}

/* Buffer management functions */
inline struct sk_buff *EXT_BONDING_COMM_getOSDescriptor(EXT_BONDING_COMM_BufferDesc *bufp)
{
  return (struct sk_buff *) bufp;
}

inline EXT_BONDING_COMM_BufferDesc *EXT_BONDING_COMM_getDescriptor(struct sk_buff *skb)
{
  return (EXT_BONDING_COMM_BufferDesc *) skb;
}

inline uint8 *EXT_BONDING_COMM_getData(EXT_BONDING_COMM_BufferDesc *bufp)
{
  struct sk_buff *skb;

  skb = EXT_BONDING_COMM_getOSDescriptor(bufp);
  return skb->data;
}

inline int EXT_BONDING_COMM_getBufferSize(EXT_BONDING_COMM_BufferDesc *bufp)
{
  struct sk_buff *skb;

  skb = EXT_BONDING_COMM_getOSDescriptor(bufp);
  return skb->len;
}

EXT_BONDING_COMM_BufferDesc *EXT_BONDING_COMM_allocateBuffer(int size)
{
  struct sk_buff *skb;
  EXT_BONDING_COMM_BufferDesc *buf = NULL;
  
  skb = dev_alloc_skb (size+sizeof (EthHeader));
  
  if (skb == NULL)
  {
    printk("bcm_ext_bonding: Not enough memory. Fatal \n");
    return NULL;
  }

  skb->len = size + sizeof (EthHeader) ;

  buf = EXT_BONDING_COMM_getDescriptor(skb);
  EXT_BONDING_COMM_pullBuffer(buf, sizeof (EthHeader)) ;

  return (buf) ;
}

void EXT_BONDING_COMM_freeFrameBuffer(EXT_BONDING_COMM_BufferDesc *p)
{
  struct sk_buff *skb;

  p = EXT_BONDING_COMM_pushBuffer (p, sizeof (EthHeader)) ;
  skb = EXT_BONDING_COMM_getOSDescriptor(p);
  dev_kfree_skb(skb);
}

uint8 *EXT_BONDING_COMM_changeBufferOffsetSize(EXT_BONDING_COMM_BufferDesc *bufp, int offsetChange, int size)
{
  struct sk_buff *skb;
  

  if ((offsetChange&1) != 0)
    printk("bcm_ext_bonding: error: offset change not multiple of 2 : %i\n", offsetChange);

  offsetChange &= ~0x1;
  skb = EXT_BONDING_COMM_getOSDescriptor(bufp);

  /* modify the official descriptor */
  skb->data += offsetChange;
  skb->tail = skb->data + size;
  //skb->end  = (unsigned char *) (((unsigned long) skb->data + size + 0x0f) & ~0x0f);
  skb->len = size;
  //skb->end  = skb->data + size ;
  //printk ("\nIn BufferOffsetSize Change, tail = %x end = %x size = %d \n", skb->tail, skb->end, size) ;
  
  return skb->data;
}

uint8 *EXT_BONDING_COMM_changeBufferEnd(EXT_BONDING_COMM_BufferDesc *bufp, int size)
{
  struct sk_buff *skb;
  struct skb_shared_info *pInfo ;
  int counter ;

  skb = EXT_BONDING_COMM_getOSDescriptor(bufp);
  //skb->end  = skb->data + size ;
  pInfo = (struct skb_shared_info *) skb->end ;
  counter = atomic_read(&(pInfo->dataref));

  if (counter != 1) {
	  return NULL ;
  }

  skb->end  = (unsigned char *) (((unsigned long) skb->data + size + 0x0f) & ~0x0f);
  //memcpy (skb->end, pInfo, sizeof (struct skb_shared_info));
  atomic_set(&(skb_shinfo(skb)->dataref), counter ) ;
  skb_shinfo(skb)->nr_frags  = 0;
  skb_shinfo(skb)->frag_list = NULL;

  return ((uint8* ) bufp) ;
}

uint8 *EXT_BONDING_COMM_pullBuffer(EXT_BONDING_COMM_BufferDesc *bufp, int len)
{
  struct sk_buff *skb;

  skb = EXT_BONDING_COMM_getOSDescriptor(bufp);
  return skb_pull(skb, len);
}

EXT_BONDING_COMM_BufferDesc *EXT_BONDING_COMM_pushBuffer(EXT_BONDING_COMM_BufferDesc *bufp, int len)
{
  struct sk_buff *skb, *skb2;
  EXT_BONDING_COMM_BufferDesc *tmp;

  skb = EXT_BONDING_COMM_getOSDescriptor(bufp);
  if (skb_headroom(skb) >= len)
  {
    skb_push(skb,len);
    tmp = bufp;
  }
  else
  {
    skb2 = skb_realloc_headroom (skb, sizeof (EthHeader)) ;
    dev_kfree_skb_any(skb);
    skb = skb2 ;
    skb_push(skb,len);
    tmp = EXT_BONDING_COMM_getDescriptor (skb) ;
  }

  return tmp;
}


void sendLineStatus (UINT32 ulPortId, PXTM_INTERFACE_LINK_INFO pLinkInfo)
{
	UINT8                       *data8 ;
   DevLineStatusInfo           *pDevLineStatus ;
   EXT_BONDING_COMM_BufferDesc *bufp ;

   bufp = EXT_BONDING_COMM_allocateBuffer (sizeof (DevLineStatusInfo)) ;

   /* Send Device Line Status Information to Master */
   if (bufp != NULL) {
		data8 = EXT_BONDING_COMM_getData (bufp) ;
		pDevLineStatus = (DevLineStatusInfo *) data8 ;
   	pDevLineStatus->ulPortId = ulPortId ;
		memcpy ((UINT8 *) &pDevLineStatus->LinkInfo, (UINT8 *) pLinkInfo, sizeof (XTM_INTERFACE_LINK_INFO)) ;

		EXT_BONDING_COMM_sendFrame (bufp, EXT_BONDING_DEV_LINE_STATUS) ;
   }
   else
      printk ("bcm_ext_bonding: Dev Line Status can not be sent \n") ;
}

/***************************************************************************
 * Function Name: EXT_BONDING_COMM_SetInterfaceLinkInfo
 * Description  : Called when an ADSL/VDSL connection has come up or gone down
 * from XTM driver upon call back from DSL entity.
 * Returns      : None.
 ***************************************************************************/
void EXT_BONDING_COMM_SetInterfaceLinkInfo ( UINT32 ulPortId,
                                            PXTM_INTERFACE_LINK_INFO pLinkInfo )
{
	UINT32                      ulPhysPort = PORTID_TO_PORT(ulPortId) ;
	UINT32                      ulTrafficType = pLinkInfo->ulLinkTrafficType ;

	printk ("bcm_ext_bonding: DSL Link Information, PHY port = %lu, State = %s,"
			  "Traffic Type = %s \n",
			  ulPhysPort,
			  ((pLinkInfo->ulLinkState == LINK_UP) ? "UP" : "DOWN"),
			  ((ulTrafficType == TRAFFIC_TYPE_ATM) ? "ATM" :
			   ((ulTrafficType == TRAFFIC_TYPE_PTM) ? "PTM" :
			    ((ulTrafficType == TRAFFIC_TYPE_PTM_BONDED) ? "PTM_BONDED" :
			     ((ulTrafficType == TRAFFIC_TYPE_ATM_BONDED) ? "ATM_BONDED" : "RAW"))))) ;

   sendLineStatus (ulPortId, pLinkInfo) ;
   slaveLineStatusInfo.ulPortId = ulPortId ;
   memcpy (&slaveLineStatusInfo.LinkInfo, pLinkInfo, sizeof (XTM_INTERFACE_LINK_INFO)) ;
}
  
/* Will be used later for static allocation rather than dynamic allocation */
void EXT_BONDING_COMM_BufferMgntInit(void)
{
  return;
}
/* End of buffer management functions */
   
/* Private frames functions */
static int EXT_BONDING_COMM_receivePrivateFrame(struct sk_buff *skb, struct net_device *dev, struct packet_type *pt, 
                                                struct net_device *dev1)
{
  struct EXT_BONDING_COMM_packet_type *p;

//  printk ("bcm_ext_bonding: Private Frame, dev-0x%x Ndev-0x%x \n",
		    //(unsigned int) dev, (unsigned int) NetworkDevices[SW_BRIDGE]) ;

  if (dev != NetworkDevices[SW_BRIDGE]) {
	                                   /* No ethernet internal interface, then drop */
     dev_kfree_skb(skb) ;
     return 0 ;
  }

  p = (struct EXT_BONDING_COMM_packet_type *) pt->af_packet_priv;

  /* Restoring the Ethernet header that has been removed by eth_type_trans (see bcmenet.c) */
  skb_push(skb, skb->dev->hard_header_len);

  if (remoteAddressKnown == 0) {
	  memcpy ((UINT8 *) &remoteAddress.mac[0], (UINT8 *) (skb->data+6), 6) ;
	  remoteAddressKnown = 1 ;
  }

  skb_pull (skb, sizeof (EthHeader)) ;

  /* Calling callback function */
  (*p->func) (EXT_BONDING_COMM_getDescriptor(skb), p->data); 
  
  return 0;
}

struct EXT_BONDING_COMM_packet_type * EXT_BONDING_COMM_registerPacketType(uint16 type, PrivatePacketTypeReceiveCb func, void *data)
{
  struct EXT_BONDING_COMM_packet_type *p;

  printk("bcm_ext_bonding: Registering private packet type 0x%04x\n", type);
  
  p = (struct EXT_BONDING_COMM_packet_type *) kmalloc(sizeof(struct EXT_BONDING_COMM_packet_type), GFP_KERNEL);
  if (p == NULL)
  {
    return NULL;
  }
  
  p->PacketType.type = htons(type);
  p->PacketType.dev = NULL;
  p->PacketType.func = EXT_BONDING_COMM_receivePrivateFrame;
  p->PacketType.af_packet_priv = p;
  p->PacketType.list.next = NULL;
  p->func = func;
  p->data = data;
  p->received = 0;

  dev_add_pack(&p->PacketType);

  return p;
}

void EXT_BONDING_COMM_unregisterPacketType(struct EXT_BONDING_COMM_packet_type *p)
{
  dev_remove_pack(&p->PacketType);

  kfree(p);
}

void EXT_BONDING_COMM_sendFrame(EXT_BONDING_COMM_BufferDesc *bufp, UINT16 usProtocolType)
{
  struct sk_buff *skb;
  ulong            flags;
  UINT8          *data8 ;
  
  skb = (struct sk_buff *) EXT_BONDING_COMM_getOSDescriptor(bufp);
  skb->dev = NetworkDevices[SW_BRIDGE];

  local_irq_save(flags);
  local_irq_enable();

  skb_push (skb, sizeof (EthHeader)) ;
  data8 = skb->data ;
  constructEthHeader (data8, usProtocolType) ;

  //printk ("SendFrame - len = %d \n", skb->len) ;
  dev_queue_xmit(skb);

  local_irq_restore(flags);
}

/* End of private frames functions */

/* Initialization and clean-up functions */

int EXT_BONDING_COMM_timer_init(void)
{
  init_timer(&extbonding_timer);
  extbonding_timer.expires = jiffies + HZ * 5 ;
  extbonding_timer.data = (unsigned long) NULL;
  extbonding_timer.function = EXT_BONDING_COMM_timer_cb ;
  add_timer(&extbonding_timer);

  return 0;
}

void constructEthHeader (UINT8 *datap, UINT16 usProtocolType)
{
	volatile UINT16 *data16p = (UINT16 *) datap ;
	UINT16 *ownAddress = EXT_BONDING_COMM_getOwnAddress () ;
	UINT16 *remoteAddress ;

	/* Fill the Dest Mac */
	if (remoteAddressKnown != 0) {
	   remoteAddress = EXT_BONDING_COMM_getRemoteAddress () ;
	}
	else {
	   remoteAddress = EXT_BONDING_COMM_getBcastAddress () ;
	}

	*data16p++ = *remoteAddress++ ;
	*data16p++ = *remoteAddress++ ;
	*data16p++ = *remoteAddress ;

	/* Fill the Src Mac */
	*data16p++ = *ownAddress++ ;
	*data16p++ = *ownAddress++ ;
	*data16p++ = *ownAddress ;

	/* Fill the Ethernet Type */
	*data16p++ = usProtocolType ;
	*data16p = 0 ;
}

int GetDeviceType (void)
{
   if (MISC->miscStrapBus & MISC_STRAP_BUS_UTOPIA_MASTER_N_SLAVE)
	{
      return (DEV_TYPE_MASTER) ;
	}
	else
      return (DEV_TYPE_SLAVE) ;
}

/***************************************************************************
 * Function Name: ExtBonding_handleDevLineStatus
 * Description  : Called when a private message such as Dev Line Status 
 * is received from the external line. Typically used in the master waiting for 
 * external line status messages.
 * Returns      : None.
 ***************************************************************************/
void ExtBonding_handleDevLineStatus (EXT_BONDING_COMM_BufferDesc *bufp, void *data)
{
	volatile UINT8     *data8 ;
   DevLineStatusInfo  *pDevLineStatus ;
  
	printk ("bcm_ext_bonding: External DSL Link Information \n") ;

   /* Send Device Line Status Information to Master XTM */
   if (bufp != NULL) {
		data8 = EXT_BONDING_COMM_getData (bufp) ;
		pDevLineStatus = (DevLineStatusInfo *) data8 ;
      if (slaveLineStatusInfo.LinkInfo.ulLinkState == pDevLineStatus->LinkInfo.ulLinkState) {
         slaveLineStatusInfo.LinkInfo.ulLinkState = LINK_DOWN ;
         BcmXtm_SetInterfaceLinkInfo (pDevLineStatus->ulPortId, &slaveLineStatusInfo.LinkInfo) ;
      }

      BcmXtm_SetInterfaceLinkInfo (pDevLineStatus->ulPortId, &pDevLineStatus->LinkInfo) ;
      memcpy (&slaveLineStatusInfo, pDevLineStatus, sizeof (DevLineStatusInfo)) ;

      EXT_BONDING_COMM_freeFrameBuffer (bufp) ;
   }
   else {
      printk ("bcm_ext_bonding: Dev Line Status can not be processed. \n") ;
   }
}

/***************************************************************************
 * Function Name: ExtBonding_handleDevLineQuery
 * Description  : Called when a private message such as Dev Line Query 
 * is received from the external line(typically master). 
 * Typically used in the slave when master does not know the state of the
 * slave.
 * Returns      : None.
 ***************************************************************************/
void ExtBonding_handleDevLineQuery (EXT_BONDING_COMM_BufferDesc *bufp, void *data)
{
	printk ("bcm_ext_bonding: External DSL Link Query From Master \n") ;
   if (slaveLineStatusInfo.ulPortId != PORT_PHY_INVALID) 
      sendLineStatus (slaveLineStatusInfo.ulPortId, &slaveLineStatusInfo.LinkInfo) ;
   /* if ulPortId is still invalid, the slave line information is not
    * initialized yet with the external driver. Ignore the message.
    */
}

void EXT_BONDING_COMM_timer_cb(unsigned long data)
{
	volatile UINT8                         *data8 ;
   DevDiscoveryInfo              devDiscovery ;
   EXT_BONDING_COMM_BufferDesc   *bufp ;

   /* Send Device Discovery */
   devDiscovery.devType = GetDeviceType() ;

   bufp = EXT_BONDING_COMM_allocateBuffer (sizeof (DevDiscoveryInfo)) ;

   if (bufp != NULL) {
		data8 = EXT_BONDING_COMM_getData (bufp) ;
		((DevDiscoveryInfo *) data8)->devType = devDiscovery.devType ;
		EXT_BONDING_COMM_sendFrame (bufp, EXT_BONDING_DEV_DISCOVERY) ;
   }
   else
      printk ("bcm_ext_bonding: Dev Discovery can not be sent \n") ;

   extbonding_timer.expires = jiffies + (HZ * 5) ;
   add_timer(&extbonding_timer);
}

void ExtBonding_handleDeviceDiscovery (EXT_BONDING_COMM_BufferDesc *bufp, void *data)
{
	volatile UINT8   *data8 ;

	data8 = EXT_BONDING_COMM_getData (bufp) ;
	//printk ("devType = %lu \n", ((DevDiscoveryInfo *) data8)->devType) ;
   EXT_BONDING_COMM_freeFrameBuffer (bufp) ;
}

int EXT_BONDING_COMM_start(void)
{
  /* Initialization of the local MAC address */
  memcpy ((UINT8 *) &ownAddress.mac[0], NetworkDevices[SW_BRIDGE]->dev_addr, 6) ;

  devDiscPacketType.type = EXT_BONDING_DEV_DISCOVERY;
  devDiscPacketType.PacketTypep = EXT_BONDING_COMM_registerPacketType(EXT_BONDING_DEV_DISCOVERY, ExtBonding_handleDeviceDiscovery, NULL);
  if (devDiscPacketType.PacketTypep == NULL)
  {
    return -1;
  }

  if (GetDeviceType() == DEV_TYPE_MASTER) {
	  stateRespPacketType.type = EXT_BONDING_DEV_LINE_STATUS ;
	  stateRespPacketType.PacketTypep = EXT_BONDING_COMM_registerPacketType(EXT_BONDING_DEV_LINE_STATUS, 
			                                         ExtBonding_handleDevLineStatus, NULL);
	  if (stateRespPacketType.PacketTypep == NULL)
	  {
		  return -1;
	  }

     /* Send query for external line once. */
     {
        EXT_BONDING_COMM_BufferDesc *bufp ;

        bufp = EXT_BONDING_COMM_allocateBuffer (sizeof (DevLineStatusInfo)) ;

        /* Send Device Line Status Information to Master */
        if (bufp != NULL)
           EXT_BONDING_COMM_sendFrame (bufp, EXT_BONDING_DEV_LINE_QUERY) ;
        else
           printk ("bcm_ext_bonding: Dev Line Query can not be sent \n") ;
     }
  }

  if (GetDeviceType() == DEV_TYPE_SLAVE) {
	  stateRespPacketType.type = EXT_BONDING_DEV_LINE_QUERY ;
	  stateRespPacketType.PacketTypep = EXT_BONDING_COMM_registerPacketType(EXT_BONDING_DEV_LINE_QUERY,
			                                         ExtBonding_handleDevLineQuery, NULL);
	  if (stateRespPacketType.PacketTypep == NULL)
	  {
		  return -1;
	  }

  }

  slaveLineStatusInfo.ulPortId = PORT_PHY_INVALID ;
  memset (&slaveLineStatusInfo.LinkInfo, 0, sizeof (XTM_INTERFACE_LINK_INFO)) ;

  EXT_BONDING_COMM_timer_init () ;

  return 0;
}

int EXT_BONDING_COMM_stop(void)
{
  if (NetworkDevices[SW_BRIDGE] == NULL)
      dev_put (NetworkDevices[SW_BRIDGE]) ;

  NetworkDevices[SW_BRIDGE] = NULL ;

  EXT_BONDING_COMM_unregisterPacketType (devDiscPacketType.PacketTypep) ;
  EXT_BONDING_COMM_unregisterPacketType (stateReqPacketType.PacketTypep) ;
  EXT_BONDING_COMM_unregisterPacketType (stateRespPacketType.PacketTypep) ;

  del_timer(&extbonding_timer) ;
  remoteAddressKnown = 0 ;
  return 0;
}


/* Init */
int EXT_BONDING_COMM_init(void)
{
  printk("bcm_ext_bonding: EXT_BONDING_COMM module init\n");
  
  register_chrdev( EXT_BONDING_COMM_DRV_MAJOR, "ext_bonding", &ext_bonding_comm_fops );

  /* Initialization of the buffer management */
  EXT_BONDING_COMM_BufferMgntInit();
  
  
  EXT_BONDING_COMM_notifier.notifier_call = EXT_BONDING_COMM_notify;
  EXT_BONDING_COMM_notifier.next = NULL;
  EXT_BONDING_COMM_notifier.priority = 0;

  register_netdevice_notifier(&EXT_BONDING_COMM_notifier);

  return 0;
}

void EXT_BONDING_COMM_exit(void)
{
  printk("bcm_ext_bonding: EXT_BONDING_COMM module exit\n");
  
  unregister_chrdev( EXT_BONDING_COMM_DRV_MAJOR, "ext_bonding") ;

  return;
}
/* End of initialization and clean-up functions */

/***************************************************************************
 * Function Name: bcm_ext_bonding_comm_open
 * Description  : Called when an application opens this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int bcm_ext_bonding_comm_open( struct inode *inode, struct file *filp )
{
    EXT_BONDING_COMM_start () ;

    return( 0 );
} /* bcm_ext_bonding_comm_open */

#if 0
/***************************************************************************
 * Function Name: bcm_ext_bonding_comm_close
 * Description  : Called when an application closes this device.
 * Returns      : 0 - success
 ***************************************************************************/
static int bcm_ext_bonding_comm_close( struct inode *inode, struct file *filp )
{
    EXT_BONDING_COMM_stop () ;

    return( 0 );
} /* bcm_ext_bonding_comm_close */
#endif

/***************************************************************************
 * MACRO to call driver initialization and cleanup functions.
 ***************************************************************************/
module_init( EXT_BONDING_COMM_init );
module_exit( EXT_BONDING_COMM_exit );
MODULE_LICENSE("Proprietary");

EXPORT_SYMBOL(EXT_BONDING_COMM_registerPacketType) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_unregisterPacketType) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_sendFrame) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_allocateBuffer) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_freeFrameBuffer) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_changeBufferOffsetSize) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_pullBuffer) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_getDescriptor) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_getOSDescriptor) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_getBufferSize) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_getData) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_pushBuffer) ;
EXPORT_SYMBOL(EXT_BONDING_COMM_SetInterfaceLinkInfo) ;
