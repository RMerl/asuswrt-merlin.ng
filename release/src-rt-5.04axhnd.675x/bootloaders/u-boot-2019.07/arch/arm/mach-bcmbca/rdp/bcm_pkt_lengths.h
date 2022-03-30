#ifndef __BCM_PKT_LENGTHS_H__
#define __BCM_PKT_LENGTHS_H__


/*TODO: makesure this file compiles when not for linux kernel also */


/* this file is used to define the packet length for All platforms and
   features at a common place */
#if defined(CONFIG_BCM63148) || defined(CONFIG_BCM4908) || defined(CONFIG_BCM6858) || defined(CONFIG_BCM63158) || \
    defined(CONFIG_BCM6846)  || defined(CONFIG_BCM47189) || defined(CONFIG_BCM6856) || defined(CONFIG_BCM63178) || \
    defined(CONFIG_BCM47622) || defined(CONFIG_BCM6878) || defined(CONFIG_BCM6756)

#define BCM_DCACHE_LINE_LEN	64ul
#define BCM_DCACHE_ALIGN_LEN	63ul

#elif defined(CONFIG_BCM63138)

#define BCM_DCACHE_LINE_LEN	32
#define BCM_DCACHE_ALIGN_LEN	31

#else

#define BCM_DCACHE_LINE_LEN	16
#define BCM_DCACHE_ALIGN_LEN	15

#endif


#define BCM_DCACHE_ALIGN(len)  ((len + BCM_DCACHE_ALIGN_LEN) & ~BCM_DCACHE_ALIGN_LEN)

#define GREATER(x, y)  (x>y ? x:y)


 /* ############ MAX Packet payload size ############ */

/*Ethernet */
#define ENET_MAX_MTU_PAYLOAD_SIZE  (1500)  /* Ethernet Max Payload Size */

/* XTM */
#define XTM_MAX_MTU_PAYLOAD_SIZE    1500

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

/* Headroom is a multiple of cacheline */
#define BCM_PKT_HEADROOM  BCM_DCACHE_ALIGN(WLAN_TX_HEADROOM + GRE_HDR_LEN)


/* ############ Tailroom needed in the packet ############ */

#define BCM_SKB_TAILROOM	32

/* ############ XRDP DMA Offset ############ */
#if defined(CONFIG_BCM6858)
#define DMA_MAX_OFFSET     128
#else
#define DMA_MAX_OFFSET     0   
#endif

#if defined(CONFIG_BCM47189)
#define DMA_DATA_OFFSET 4
#endif

/* ############ Toatal length used for packets ############ */
//#define BCM_MAX_PKT_LEN              BCM_MAX_MTU_EXTRA_SIZE + BCM_MAX_MTU_PAYLOAD_SIZE
    /*align this to 64 bytes as Iudma may overwite, some bytes if not 64 byte aligned , bug in iudma*/
#if defined(CONFIG_BCM_ENET_SYSPORT)

    #define SYSPORT_MAX_PKT_LEN (2048)
    #define SYSPORT_PKT_LEN_LOG2 (11)

    #define CALC_MAX_PKT_LEN             ((DMA_MAX_OFFSET + BCM_MAX_MTU_EXTRA_SIZE + BCM_MAX_MTU_PAYLOAD_SIZE + 63) & ~63)
    #if CALC_MAX_PKT_LEN > SYSPORT_MAX_PKT_LEN
        #error "Error: CALC_MAX_PKT_LEN > SYSPORT_MAX_PKT_LEN"
    #endif
    #define BCM_MAX_PKT_LEN              SYSPORT_MAX_PKT_LEN

#else
/* This ignores the headroom in the BPM buffer otherwise it could be reduced.
 It could be set to 1856 so that the headroom in the BPM buffer would be used. 
 But to keep things simple and safe, max pkt len is assigned to PKTBUFSZ from
 linux_osl.h */
#define BCM_MAX_PKT_LEN               GREATER(2048, ((DMA_MAX_OFFSET + BCM_MAX_MTU_EXTRA_SIZE + BCM_MAX_MTU_PAYLOAD_SIZE + 63) & ~63))
#endif 

/* ############ Toatal buf size i.e BCM_MAX_PKT_LEN + metadata(fkb,skb_sharedinfo etc..) ############ */
/* BCM_FKB_INPLACE, BCM_PKT_HEADROOM are always to be at cache-aligned boundaries */

#define BCM_PKTBUF_SIZE             (BCM_DCACHE_ALIGN(BCM_MAX_PKT_LEN))

/* ############ other common defines ############ */

#define ENET_MIN_MTU_SIZE       60            /* Without FCS */
#define ENET_MIN_MTU_SIZE_EXT_SWITCH       64            /* Without FCS */
#define ENET_MAX_MTU_SIZE       (ENET_MAX_MTU_PAYLOAD_SIZE + ENET_MAX_MTU_EXTRA_SIZE)

#endif /* __BCM_PKT_LENGTHS_H__ */
