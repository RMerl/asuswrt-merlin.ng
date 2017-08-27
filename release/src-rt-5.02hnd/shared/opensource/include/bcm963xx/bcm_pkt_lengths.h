#ifndef __BCM_PKT_LENGTHS_H__
#define __BCM_PKT_LENGTHS_H__


/*TODO: makesure this file compiles when not for linux kernel also */


/* this file is used to define the packet length for All platforms and
   features at a common place */
#if defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858)

#define BCM_DCACHE_LINE_LEN		64ul
#define BCM_DCACHE_ALIGN_LEN	63ul

#elif defined(CONFIG_BCM963138)

#define BCM_DCACHE_LINE_LEN		32
#define BCM_DCACHE_ALIGN_LEN	31

#else

#define BCM_DCACHE_LINE_LEN		16
#define BCM_DCACHE_ALIGN_LEN	15

#endif


#define BCM_DCACHE_ALIGN(len)  ((len + BCM_DCACHE_ALIGN_LEN) & ~BCM_DCACHE_ALIGN_LEN)

#define GREATER(x, y)  (x>y ? x:y)


 /* ############ MAX Packet payload size ############ */

/*Ethernet */
/* Not chip specific but feature specific */
#if defined(CONFIG_BCM_JUMBO_FRAME)&& defined(CONFIG_BCM_MAX_MTU_SIZE)
#define ENET_MAX_MTU_PAYLOAD_SIZE  CONFIG_BCM_MAX_MTU_SIZE
#define ENET_NON_JUMBO_MAX_MTU_PAYLOAD_SIZE  (1500)
#elif defined (CONFIG_BCM_JUMBO_FRAME) 
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) 
/* 63138/148/4908 Runner FW flows don't support 2048 MTU */
#define ENET_MAX_MTU_PAYLOAD_SIZE  (2044)  /* Ethernet Max Payload Size - mini jumbo */
#else
#define ENET_MAX_MTU_PAYLOAD_SIZE  (2048)  /* Ethernet Max Payload Size - mini jumbo */
#endif /* 63138 || 63148 || 4908 */
#define ENET_NON_JUMBO_MAX_MTU_PAYLOAD_SIZE  (1500)  /*Max Payload Size for non-jumbo interfaces(ex;100mbps ports)*/
#else
#define ENET_MAX_MTU_PAYLOAD_SIZE  (1500)  /* Ethernet Max Payload Size */
#endif

/* XTM */

#if defined(CONFIG_BCM_JUMBO_FRAME)
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) 
/* 63138/148/4908 Runner FW flows don't support 2048 MTU */
#define XTM_MAX_MTU_PAYLOAD_SIZE    2044  /* Ethernet Max Payload Size - mini jumbo */
#else
#define XTM_MAX_MTU_PAYLOAD_SIZE    2048  /* Ethernet Max Payload Size - mini jumbo */
#endif /* 63138 || 63148 || 4908 */
#else
#define XTM_MAX_MTU_PAYLOAD_SIZE    1500
#endif



/* WLAN */

#define WLAN_MAX_MTU_PAYLOAD_SIZE (1500)

#if defined(CONFIG_BCM_USER_DEFINED_DEFAULT_MTU)
    #if (CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE > ENET_MAX_MTU_PAYLOAD_SIZE)
    #error "ERROR - CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE > ENET_MAX_MTU_PAYLOAD_SIZE"
    #endif
    #if (CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE > XTM_MAX_MTU_PAYLOAD_SIZE)
    #error "ERROR - CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE > XTM_MAX_MTU_PAYLOAD_SIZE"
    #endif

    #define BCM_ENET_DEFAULT_MTU_SIZE CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE
    #define BCM_XTM_DEFAULT_MTU_SIZE  CONFIG_BCM_USER_DEFINED_DEFAULT_MTU_SIZE

#else /* !CONFIG_BCM_USER_DEFINED_DEFAULT_MTU */

    #define BCM_ENET_DEFAULT_MTU_SIZE ENET_MAX_MTU_PAYLOAD_SIZE
    #define BCM_XTM_DEFAULT_MTU_SIZE  XTM_MAX_MTU_PAYLOAD_SIZE

#endif


/* select greater of XTM, WLAN, ENET and use it as MAX payload in system
 * for buffer allocation purpose
 */

/*TODO check if compiler is replacing these checks with a final value, if not
 select the MAX payload manually */

#define BCM_MAX_MTU_PAYLOAD_SIZE GREATER( \
								GREATER(ENET_MAX_MTU_PAYLOAD_SIZE, XTM_MAX_MTU_PAYLOAD_SIZE) \
								,WLAN_MAX_MTU_PAYLOAD_SIZE)


/* ############ space needed for L2 header ############ */
#ifndef ENET_MAX_MTU_EXTRA_SIZE
#define ENET_MAX_MTU_EXTRA_SIZE  (32) /* EH_SIZE(14) + VLANTAG(4) + VLANTAG(4) + BRCMTAG(6) + FCS(4) + Extra(??) (4)*/
#endif

/*select greater value*/
#define BCM_MAX_MTU_EXTRA_SIZE    ENET_MAX_MTU_EXTRA_SIZE



/* ############ Headroom needed in the packet ############ */

/* this headroom is needed for WLAN header for  ENET,XTM,XPON ==> WLAN traffic */
#define WLAN_TX_HEADROOM    208
#define XTM_BONDING_HEADROOM       48
#define GRE_HDR_LEN         16


/*select greater value*/
#define BCM_PKT_HEADROOM  BCM_DCACHE_ALIGN(WLAN_TX_HEADROOM + GRE_HDR_LEN)


/* ############ Tailroom needed in the packet ############ */

#define BCM_SKB_TAILROOM	32



/* ############ space needed for skb_shared_info ############ */
#if defined(__KERNEL__)
#define BCM_SKB_SHAREDINFO	sizeof(struct skb_shared_info)
#endif


//#define BCM_PKT_TAILROOM	(BCM_SKB_TAILROOM +   BCM_SKB_SHAREDINFO)

/* ############ space needed for FKB ############ */
#if defined(__KERNEL__)
#define BCM_FKB_INPLACE     sizeof(FkBuff_t)
#endif

/* ############ XRDP DMA Offset ############ */
#if defined(CONFIG_BCM96858)
#define DMA_MAX_OFFSET     128
#else
#define DMA_MAX_OFFSET     0   
#endif

/* ############ Toatal length used for packets ############ */
//#define BCM_MAX_PKT_LEN              BCM_MAX_MTU_EXTRA_SIZE + BCM_MAX_MTU_PAYLOAD_SIZE
    /*align this to 64 bytes as Iudma may overwite, some bytes if not 64 byte aligned , bug in iudma*/
#define BCM_MAX_PKT_LEN              ((DMA_MAX_OFFSET + BCM_MAX_MTU_EXTRA_SIZE + BCM_MAX_MTU_PAYLOAD_SIZE + 63) & ~63)

/* ############ Toatal buf size i.e BCM_MAX_PKT_LEN + metadata(fkb,skb_sharedinfo etc..) ############ */

#if defined(__KERNEL__)
#define BCM_PKTBUF_SIZE             (BCM_DCACHE_ALIGN(BCM_FKB_INPLACE  + \
                                                BCM_PKT_HEADROOM + \
                                                BCM_MAX_PKT_LEN           + \
                                                BCM_SKB_TAILROOM           + \
                                                BCM_SKB_SHAREDINFO))

#elif defined(_CFE_)

#define BCM_PKTBUF_SIZE             (BCM_DCACHE_ALIGN(BCM_MAX_PKT_LEN))

#endif

/* ############ other common defines ############ */

#define ENET_MIN_MTU_SIZE       60            /* Without FCS */
#define ENET_MIN_MTU_SIZE_EXT_SWITCH       64            /* Without FCS */
#define ENET_MAX_MTU_SIZE       (ENET_MAX_MTU_PAYLOAD_SIZE + ENET_MAX_MTU_EXTRA_SIZE)

#if defined(__KERNEL__)
#define BCM_SKB_ALIGNED_SIZE            ((sizeof(struct sk_buff) + 0x0f) & ~0x0f)
#endif

#endif /* __BCM_PKT_LENGTHS_H__ */
