/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>

*/

/** Includes. **/
#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_string.h"
#include "lib_printf.h"

#include "addrspace.h"
#include "cfe_iocb.h"
#include "cfe_ioctl.h"
#include "cfe_device.h"
#include "cfe_devfuncs.h"
#include "cfe_timer.h"

#include "bcm_map.h"
#include "bcm_ethsw.h"
#include "pmc_switch.h"
#include "pmc_sysport.h"
#include "bcm_led.h"

/** Prototypes. **/
static void bcm63xx_ether_probe( cfe_driver_t * drv,    unsigned long probe_a,
                                 unsigned long probe_b, void * probe_ptr );
static int bcm63xx_ether_open(cfe_devctx_t *ctx);
static int bcm63xx_ether_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm63xx_ether_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat);
static int bcm63xx_ether_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm63xx_ether_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer);
static int bcm63xx_ether_close(cfe_devctx_t *ctx);

extern void _cfe_flushcache(int, uint8_t *, uint8_t *);
#define FLUSH_RANGE(s,l) _cfe_flushcache(CFE_CACHE_FLUSH_RANGE,((uint8_t *) (s)),((uint8_t *) (s))+(l))

typedef struct PktDesc {
    uint32_t address ;
    uint32_t address_hi :8;
    uint32_t status :10;
    uint32_t length :14;
}PktDesc;


#define SYSPORT_NUM_RX_PKT_DESC_LOG2  (7)

#define SYSPORT_NUM_TX_PKT_DESC_LOG2  (0)

#define SYSPORT_PKT_LEN_LOG2 (11)

/* Number of RX DMA buffers */
#define NUM_RX_DMA_BUFFERS              (1<<SYSPORT_NUM_RX_PKT_DESC_LOG2)

/* Number of RX DMA buffers */
#define NUM_TX_DMA_BUFFERS              (1<<SYSPORT_NUM_TX_PKT_DESC_LOG2)

/* DMA buffer size for SP_RDMA_BSRS_REG register (2048 bytes) */
#define NET_DMA_SHIFT                   11


/* RX/TX DMA buffer size */
#define NET_DMA_BUFSIZE                 (1<<SYSPORT_PKT_LEN_LOG2) 


#define CACHE_ALIGN			64
static unsigned char			*dma_buffer=NULL;


/* Base address of DMA buffers */
#define NET_DMA_BASE                    (dma_buffer)

/* Address of TX DMA buffer */
#define TX_DMA_BUFADDR                  (NET_DMA_BASE)

/* Address of first RX DMA buffer */
#define RX_DMA_BUFADDR                  (NET_DMA_BASE + (NET_DMA_BUFSIZE*NUM_TX_DMA_BUFFERS))

#if defined (SYSPORT_1_TPC)
static unsigned char			*dma_buffer_1=NULL;
/* Base address of DMA buffers */
#define NET_DMA_BASE_1                    (dma_buffer_1)

/* Address of TX DMA buffer */
#define TX_DMA_BUFADDR_1                  (NET_DMA_BASE_1)

/* Address of first RX DMA buffer */
#define RX_DMA_BUFADDR_1                  (NET_DMA_BASE_1 + (NET_DMA_BUFSIZE*NUM_TX_DMA_BUFFERS))
#endif


/* Size of RSB header prepended to RX packets */
#define RSB_SIZE                                8

/* Minimal Ethernet packet length */
#define ENET_ZLEN                               60


#define MAX_PKT_LEN                             (NET_DMA_BUFSIZE)





/** Variables. **/
const static cfe_devdisp_t bcm63xx_ether_dispatch = {
    bcm63xx_ether_open,
    bcm63xx_ether_read,
    bcm63xx_ether_inpstat,
    bcm63xx_ether_write,
    bcm63xx_ether_ioctl,
    bcm63xx_ether_close,
    NULL,
    NULL
};

const cfe_driver_t bcm63xx_enet = {
    "BCM63xx Ethernet",
    "eth",
    CFE_DEV_NETWORK,
    &bcm63xx_ether_dispatch,
    bcm63xx_ether_probe
};


#if defined(SYSPORT_V2)
#define SYSPORT_0 0
#define SYSPORT_1 1
int active_sysport=SYSPORT_1;

static void init_pkt_desc(volatile void *ptr, unsigned char *dma_buffer_addr)
{
	int i;
	volatile PktDesc *pkt_desc=(PktDesc *)ptr;

	for(i=0;i<(1<<SYSPORT_NUM_RX_PKT_DESC_LOG2);i++)
	{
		pkt_desc[i].address=(uintptr_t)dma_buffer_addr+(2048*i);
		pkt_desc[i].address_hi=0;
		pkt_desc[i].status=0;
		pkt_desc[i].length=2048;
	}
}
/* Enable System Port TX DMA */
static int __sp_enable_tdma(volatile sys_port_tdma *sysport_tdma)
{
	volatile uint32_t reg;
	int timeout = 1000;

	// Disable ACB and enable TDMA
	sysport_tdma->SYSTEMPORT_TDMA_CONTROL |= 0x01;

	// Wait for TX DMA to become ready
	do {
		reg = sysport_tdma->SYSTEMPORT_TDMA_STATUS;
		if (!(reg & 0x3)) {
			return 0;
		}
		cfe_usleep(10);

	} while (timeout--);

	return -1;
}/* Enable System Port RX DMA */

static int sp_enable_tdma(void)
{
	return __sp_enable_tdma(SYSPORT_TDMA)
#if defined(SYSPORT_1_TDMA)
	 | __sp_enable_tdma(SYSPORT_1_TDMA)
#endif
	;

}
static int __sp_enable_rdma(volatile sys_port_rdma *sysport_rdma)
{
	volatile uint32_t reg;
	int timeout = 1000;

	// Enable RX DMA and Ring Buffer mode
	sysport_rdma->SYSTEMPORT_RDMA_CONTROL |= 0x01;

	// Wait for RX DMA to become ready
	do {
		reg = sysport_rdma->SYSTEMPORT_RDMA_STATUS;
		if (!(reg & 0x3)) {
			return 0;
		}
		cfe_usleep(10);

	} while (timeout--);

	return -1;
}
int sp_enable_rdma(void)
{
	return __sp_enable_rdma(SYSPORT_RDMA)
#if defined (SYSPORT_1_RDMA)
	 | __sp_enable_rdma(SYSPORT_1_RDMA)
#endif
	;
}
void sp_reset(int pmc_reset)
{
    volatile uint32_t v32;
    uint16 p_idx, c_idx, timeout;

#if defined(SYSPORT_UMAC)
    // Disable RX UMAC
    v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_UMAC_CMD_RX_ENA_M; /* Disable RX */
    SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;
#endif
#if defined(SYSPORT_1_UMAC)
    // Disable RX UMAC
    v32 = SYSPORT_1_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_UMAC_CMD_RX_ENA_M; /* Disable RX */
    SYSPORT_1_UMAC->SYSTEMPORT_UMAC_CMD=v32;
#endif
#if defined (SYSPORT_GIB)
    v32=SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL;
    v32 &= ~SYSPORT_GIB_CONTROL_RX_EN;
    SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL = v32;
#endif

    // in case there is still DMA going on, wait for the last bit to finish
    cfe_usleep(1000);  // will need to increase wait time if system port speed is slower 
                       // (e.g, it takes 1.2ms to transmit 1518 bytes packets when system port is running at 10 Mbps)

    // Disable and Flush RX DMA
    SYSPORT_RDMA->SYSTEMPORT_RDMA_CONTROL &= ~SYSPORT_RDMA_CTRL_RDMA_EN_M;
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_RX_FLUSH_CNTL=1;

#if defined (SYSPORT_GIB)
    // flush RX GIB
    v32 = SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL;
    v32 |= SYSPORT_GIB_CONTROL_RX_FLUSH;
    SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL = v32;
#endif

    // Disable TX DMA
    SYSPORT_TDMA->SYSTEMPORT_TDMA_CONTROL &= ~SYSPORT_TDMA_CONTROL_TDMA_EN_M;

    // Wait till all TXDMA is completed
    timeout = 1000;
#if defined(SYSPORT_V1)
    v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
#else
	v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC_RING_PRODUCER_CONSUMER_INDEX[0];
#endif
    p_idx = (uint16_t)(v32 & SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_PRODUCER_INDEX_M);
    c_idx = (uint16_t)(v32 >> SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_S);

    while ((p_idx != c_idx) && timeout)
    {
        cfe_usleep(1000);  // will need to increase wait time if system port speed is slower 
                           // (e.g, it takes 1.2ms to transmit 1518 bytes packets when system port is running at 10 Mbps)
        timeout --;
#if defined(SYSPORT_V1)
        v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
#else
	    v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC_RING_PRODUCER_CONSUMER_INDEX[0];
#endif
        p_idx = (uint16_t)(v32 & SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_PRODUCER_INDEX_M);
        c_idx = (uint16_t)(v32 >> SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_S);
    }

#if defined(SYSPORT_UMAC)
    // Disable TX UNIMAC and then flush TXDMA
    v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_UMAC_CMD_TX_ENA_M; /* Disable TX */
    SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;
#endif
#if defined(SYSPORT_1_UMAC)
    // Disable TX UNIMAC and then flush TXDMA
    v32 = SYSPORT_1_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_UMAC_CMD_TX_ENA_M; /* Disable TX */
    SYSPORT_1_UMAC->SYSTEMPORT_UMAC_CMD=v32;
#endif

#if defined (SYSPORT_GIB)
    v32=SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL;
    v32 &= ~SYSPORT_GIB_CONTROL_TX_EN;
    SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL = v32;
#endif

    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_TX_FLUSH_CNTL=1;
#if defined (SYSPORT_GIB)
    v32=SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL;
    v32 |= SYSPORT_GIB_CONTROL_TX_FLUSH;
    SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL = v32;
#endif

    // Reset complete, prepare for initialization
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_RX_FLUSH_CNTL=0;
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_TX_FLUSH_CNTL=0;

#if defined (SYSPORT_GIB)
    v32=SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL;
    v32 &= ~SYSPORT_GIB_CONTROL_TX_FLUSH;
    v32 &= ~SYSPORT_GIB_CONTROL_RX_FLUSH;
    SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL = v32;
#endif

    if (pmc_reset)
    {
        // reset system port through BPCM
        pmc_sysport_reset_system_port(0);
#if defined(SYSPORT_1_TPC)
        pmc_sysport_reset_system_port(1);
#endif
        /* pmc reset clears certains configuration such sysport led setting so have to
           reinit again. Once linux init calls bcm_ethsw_init, we can remove this code
           and pmc_sysport_reset completey. */ 
        bcm_ethsw_led_init();
    }
}

static int __sp_init(volatile sys_port_rbuf *sysport_rbuf, volatile sys_port_rdma *sysport_rdma, volatile sys_port_tdma *sysport_tdma, unsigned char *dma_buffer_addr)
{

	/* all memory allocation are done already */
	uint32_t v32 = 0;

	/* System Port RBUF configuration */
	v32 = sysport_rbuf->SYSTEMPORT_RBUF_RBUF_CONTROL;     /* Read Chip Defaults */
	v32 &= ~SYSPORT_RBUF_CTRL_RSB_MODE_M;                   /* Disable RSB */
	v32 &= ~SYSPORT_RBUF_CTRL_4B_ALIGN_M;                 /* Disable 4-Byte IP Alignment */
	v32 &= ~SYSPORT_RBUF_CTRL_BTAG_STRIP_M;               /* Do not strip BRCM TAG */
	v32 |= SYSPORT_RBUF_CTRL_BAD_PKT_DISCARD_M;           /* Discard Bad Packets */
	/* Read-Modify-Write */
	sysport_rbuf->SYSTEMPORT_RBUF_RBUF_CONTROL=v32; 

	sysport_rbuf->SYSTEMPORT_RBUF_RBUF_PACKET_READY_THRESHOLD=0x80; /* Keep chip default */

	/* System Port TBUF configuration -- No change, keep chip defaults */

	/* System Port RDMA Configuration */

	/* RDMA Control Register */
	v32 = sysport_rdma->SYSTEMPORT_RDMA_CONTROL;  /* Read Chip Defaults */
	v32 &= ~SYSPORT_RDMA_CTRL_RDMA_EN_M;          /* Disable RDMA */
	v32 &= ~SYSPORT_RDMA_CTRL_RING_CFG_bit1_M;          /* Enable Descriptor Ring Mode */
	v32 |= SYSPORT_RDMA_CTRL_DISCARD_EN_M;        /* Enable Pkt discard by RDMA when ring full */
	v32 &= ~SYSPORT_RDMA_CTRL_DATA_OFFSET_M;       /* Zero data offset - this feature could be used later to reduce host buffer size */
	v32 &= ~SYSPORT_RDMA_CTRL_DDR_DESC_WR_EN_M;
//	v32 |= SYSPORT_RDMA_CTRL_DDR_DESC_SWAP_M;     /* Both Byte and word swap enabled for Desc - TBD - need to understand */
	/* Read-Modify-Write */
	sysport_rdma->SYSTEMPORT_RDMA_CONTROL=v32; 

	/* RDMA Buffer and Ring Size Register */
	//v32 = 0;/* Reset register  */
	//v32 |= ( (SYSPORT_PKT_LEN_LOG2 << SYSPORT_RDMA_BSRS_BUF_SIZE_LOG2_S) & SYSPORT_RDMA_BSRS_BUF_SIZE_LOG2_M ); /* set buf size */
	//v32 |= ( ((1<<SYSPORT_NUM_RX_PKT_DESC_LOG2) << SYSPORT_RDMA_BSRS_RING_SIZE_S) & SYSPORT_RDMA_BSRS_RING_SIZE_M ); /* force chip default of 512 */
	sysport_rdma->SYSTEMPORT_RDMA_LOCRAM_DESCRING_SIZE[0]=SYSPORT_RDMA_RING_EN | (1<<SYSPORT_NUM_RX_PKT_DESC_LOG2);
	sysport_rdma->SYSTEMPORT_RDMA_PKTBUF_SIZE[0]=SYSPORT_PKT_LEN_LOG2;


	/* RDMA Consumer Index Register */
	/* Initialize RX DMA consumer index - low 16 bit; High 16-bits are read-only */
	sysport_rdma->SYSTEMPORT_RDMA_CINDEX[0]=0x0;
	sysport_rdma->SYSTEMPORT_RDMA_PINDEX[0]=0x0;

	/* RDMA Desc Start Address Registers */
	/* In desciptor ring mode - start address is index = 0 */
	sysport_rdma->SYSTEMPORT_RDMA_DDR_DESC_RING_START[0]=0;
	sysport_rdma->SYSTEMPORT_RDMA_DDR_DESC_RING_START[1]=0;


	init_pkt_desc(sysport_rdma->SYSTEMPORT_RDMA_DESCRIPTOR, dma_buffer_addr);

	/* RDMA DDR Desc Ring Register */
	//sysport_rdma->SYSTEMPORT_RDMA_DDR_DESC_RING_START_LOW = 0;
	//sysport_rdma->SYSTEMPORT_RDMA_DDR_DESC_RING_START_HIGH = 0; /* Ideally we should put the Hi 8-bits here */

	/* RDMA DDR Desc Ring Size Register */
	sysport_rdma->SYSTEMPORT_RDMA_DDR_DESC_RING_SIZE[0] = SYSPORT_NUM_RX_PKT_DESC_LOG2;

	/* RDMA Multi-Buffer-Done-Interrupt-Threshold : No timeout & interrupt every packet */
	//sysport_rdma->SYSTEMPORT_RDMA_MULTIPLE_BUFFERS_DONE_INTERRUPT_THRESHOLD_PUSH_TIMER = 1;
	/* enable DDR DESC write push timer to 1 timer tick (equals 1024 RDMA sys clocks */
	//sysport_rdma->SYSTEMPORT_RDMA_DDR_DESC_RING_WR_PUSH_TIMER[0] = (0x1 & SYSTEMPORT_RDMA_DDR_DESC_RING_PUSH_TIMER_TIMEOUT_M);
	/* TDMA Block Configuration */

	/* System port supports upto 16 Desc Rings;
	Only one TX DDR Desc ring is used; It is mapped to TX-Queue[0] */

	/* Enable TX Q#0 */
	//sysport_tdma->SYSTEMPORT_TDMA_DESC_RING_HEAD_TAIL_PTR[0] = SYSPORT_TDMA_DESC_RING_XX_HEAD_TAIL_PTR_RING_EN_M;

	sysport_tdma->SYSTEMPORT_TDMA_DESC_RING_CONTROL[0]=SYSTEMPORT_TDMA_DESC_RING_CONTROL_RING_EN;
	/* Initialize Producer & Consumer Index */
	sysport_tdma->SYSTEMPORT_TDMA_DESC_RING_PRODUCER_CONSUMER_INDEX[0] = 0;
	/* Q#0 DDR Desc Ring Address */
	sysport_tdma->SYSTEMPORT_TDMA_DDR_DESC_RING_START[0] = 0;
	sysport_tdma->SYSTEMPORT_TDMA_DDR_DESC_RING_START[1] = 0; /* Ideally this should be high 8-bit of address */

	sysport_tdma->SYSTEMPORT_TDMA_DESC_RING_MAPPING[0] = 0x40;
	/* enable DDR DESC read push timer to 1 timer tick (equals 1024 TDMA sys clocks */
	sysport_tdma->SYSTEMPORT_TDMA_DDR_DESC_RING_PUSH_TIMER[0] = (0x1 & SYSTEMPORT_TDMA_DDR_DESC_RING_PUSH_TIMER_TIMEOUT_M);
	/* Q#0 DDR Desc Ring Size Log2 */
	sysport_tdma->SYSTEMPORT_TDMA_DDR_DESC_RING_SIZE[0] = SYSPORT_NUM_TX_PKT_DESC_LOG2;
	sysport_tdma->SYSTEMPORT_TDMA_DESC_RING_INTR_CONTROL[0] = 0x3;
	sysport_tdma->SYSTEMPORT_TDMA_DESC_RING_MAX_THRESHOLD[0] = 0x00100009;
	/* enable arbitrator for Q#0 */
	sysport_tdma->SYSTEMPORT_TDMA_TIER2_ARBITER_CTRL = 0x1; /* Round Robin */
	sysport_tdma->SYSTEMPORT_TDMA_TIER1_ARBITER_CTRL[0] = 0x1;
	sysport_tdma->SYSTEMPORT_TDMA_TIER1_ARBITER_CTRL[1] = 0x1;
	sysport_tdma->SYSTEMPORT_TDMA_TIER1_ARBITER_CTRL[2] = 0x1;
	sysport_tdma->SYSTEMPORT_TDMA_TIER1_ARBITER_CTRL[3] = 0x1;
	sysport_tdma->SYSTEMPORT_TDMA_TIER1_ARBITER_QUEUE_ENABLE[0] = 0x000000ff;
	sysport_tdma->SYSTEMPORT_TDMA_TIER1_ARBITER_QUEUE_ENABLE[1] = 0x0000ff00;
	sysport_tdma->SYSTEMPORT_TDMA_TIER1_ARBITER_QUEUE_ENABLE[2] = 0x00ff0000;
	sysport_tdma->SYSTEMPORT_TDMA_TIER1_ARBITER_QUEUE_ENABLE[3] = 0xff000000;
	/* TDMA Control Register */
	v32 = sysport_tdma->SYSTEMPORT_TDMA_CONTROL; /* Read chip defaults */
	v32 &= ~SYSPORT_TDMA_CONTROL_TSB_EN_M; /* Disable TSB */
	v32 &= ~(SYSPORT_TDMA_CONTROL_RING_CFG_M); /* Disable DDR Desc Ring fetch */
	v32 &= ~(SYSPORT_TDMA_CONTROL_ACB_EN_M); /* No ACB */
	sysport_tdma->SYSTEMPORT_TDMA_CONTROL = v32;

	/* Enable Tier-1 arbiter for Q#0 */
	sysport_tdma->SYSTEMPORT_TDMA_TIER1_ARBITER_QUEUE_ENABLE[0]=0x1;

	return 0;
}


int sp_init(void)
{

	/* all memory allocation are done already */
	uint32_t v32 = 0;

	__sp_init(SYSPORT_RBUF, SYSPORT_RDMA, SYSPORT_TDMA, RX_DMA_BUFADDR); 


#if defined(SYSPORT_1_TPC)
	__sp_init(SYSPORT_1_RBUF, SYSPORT_1_RDMA, SYSPORT_1_TDMA, RX_DMA_BUFADDR_1);
#endif

#if defined (SYSPORT_GIB)
	v32=SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL;
	v32 |= SYSPORT_GIB_CONTROL_TX_EN | SYSPORT_GIB_CONTROL_RX_EN;
	SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL = v32;
#endif

	// Enable RX DMA
	if (sp_enable_rdma() < 0) {
		// Print SPIF for System Port Init Failed

		return -1;
	}

	// Enable TX DMA
	if (sp_enable_tdma() < 0) {
		// Print SPIF for System Port Init Failed

		return -1;
	}

#if defined(SYSPORT_UMAC)
	v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
	v32 |= SYSPORT_UMAC_CMD_TX_ENA_M; /* Enable TX */
	v32 |= SYSPORT_UMAC_CMD_RX_ENA_M; /* Enable RX */
	SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;
#endif
#if defined(SYSPORT_1_UMAC)
	v32 = SYSPORT_1_UMAC->SYSTEMPORT_UMAC_CMD;
	v32 |= SYSPORT_UMAC_CMD_TX_ENA_M; /* Enable TX */
	v32 |= SYSPORT_UMAC_CMD_RX_ENA_M; /* Enable RX */
	SYSPORT_1_UMAC->SYSTEMPORT_UMAC_CMD=v32;
#endif


	// Give SP some time to initialize
	cfe_usleep(100);

	// Print SPIP for System Port Init Passed
	bcm_ethsw_open();

	return 0;
}

static int __sp_poll(volatile sys_port_rdma *sysport_rdma)
{
	uint32_t p_index, c_index;

	// SYSTEMPORT_RDMA_PRODUCER_INDEX...
	p_index = sysport_rdma->SYSTEMPORT_RDMA_PINDEX[0] & 0xffff;
	c_index = sysport_rdma->SYSTEMPORT_RDMA_CINDEX[0] & 0xffff;

	return (p_index == c_index);
}
static int sp_poll(uint32_t timeout_ms)
{
	int i = 0;
	
	while (1) {

		if(__sp_poll(SYSPORT_RDMA) == 0)
		{
			active_sysport=SYSPORT_0;
			return 0;
		}
#if defined(SYSPORT_1_RDMA)
 		else if( __sp_poll(SYSPORT_1_RDMA) == 0)
		{
			active_sysport=SYSPORT_1;
			return 0;
		}
#endif
		if (i++ > timeout_ms)
		{
			return -1;
		}
		cfe_usleep(1000);
	}
}

int __sp_read(volatile sys_port_rdma *sysport_rdma, uint8_t *buffer, uint32_t *length)
{
	int rc = 0;
	unsigned short rx_read_idx;
	const uint8_t *bufaddr;
	volatile PktDesc *pkt_desc;

	rx_read_idx=sysport_rdma->SYSTEMPORT_RDMA_CINDEX[0];
	rx_read_idx=rx_read_idx%NUM_RX_DMA_BUFFERS;
	// Get RX DMA buffer address
	pkt_desc=(PktDesc *)sysport_rdma->SYSTEMPORT_RDMA_DESCRIPTOR;
#if defined(CONFIG_ARM64)
	bufaddr = (uint8_t *)cache_to_uncache((const uint8_t *)NULL + pkt_desc[rx_read_idx].address);
#else
	FLUSH_RANGE(pkt_desc[rx_read_idx].address, NET_DMA_BUFSIZE);
	bufaddr = (uint8_t *)((const uint8_t *)NULL + pkt_desc[rx_read_idx].address);
#endif

	*length=pkt_desc[rx_read_idx].length;


	if (*length < ENET_ZLEN || *length > MAX_PKT_LEN) {
		// Print SPRE for System Port Read Error

		rc = -1;
		goto out;
	}

	memcpy(buffer, bufaddr, *length);

out:
	sysport_rdma->SYSTEMPORT_RDMA_CINDEX[0]+=1;

	return rc;
}
int sp_read(uint8_t *buffer, uint32_t *length)
{
	if(active_sysport == SYSPORT_0)
		return __sp_read(SYSPORT_RDMA, buffer, length);
#if defined(SYSPORT_1_RDMA)
	else
	{
		return __sp_read(SYSPORT_1_RDMA, buffer, length);
	}
#endif
return -1;
}

int __sp_write(volatile sys_port_tdma *sysport_tdma, unsigned char* tx_dma_bufaddr, uint8_t *buffer, uint32_t length)
{
	int timeout = 1000;
	uint32_t p_index, c_index;
	uint32_t txdesc_hi, txdesc_lo;

	if (length > MAX_PKT_LEN) {
		// Print SPWE for SP Write Error

		return -1;
	}

#if defined(CONFIG_ARM64)
	memcpy((void *)cache_to_uncache(tx_dma_bufaddr), buffer, length);
#else
	memcpy((void *)(tx_dma_bufaddr), buffer, length);
	FLUSH_RANGE(tx_dma_bufaddr, NET_DMA_BUFSIZE);
#endif

	// set up txdesc record: eop=1, sop=1, append_crc=1
	txdesc_hi = (length << 18) | (3 << 16) | (1 << 11);
#if defined(CONFIG_ARM64)
	txdesc_lo = (uint32_t)K1_TO_PHYS(tx_dma_bufaddr);
#else
	txdesc_lo = (uint32_t)(tx_dma_bufaddr);
#endif

	// write tx desc to hw
	// SYSTEMPORT_TDMA_DESCRIPTOR_00_WRITE_PORT_LO
	sysport_tdma->SYSTEMPORT_TDMA_DESCRIPTOR_WRITE_PORT[0].SYSTEMPORT_TDMA_DESCRIPTOR_XX_WRITE_PORT_LO=txdesc_lo;

        __asm__ __volatile__ ("dsb    sy");

	// SYSTEMPORT_TDMA_DESCRIPTOR_00_WRITE_PORT_HI
	sysport_tdma->SYSTEMPORT_TDMA_DESCRIPTOR_WRITE_PORT[0].SYSTEMPORT_TDMA_DESCRIPTOR_XX_WRITE_PORT_HI=txdesc_hi;

	// wait until the packet is fully DMA'ed and stored into TBUF
	do {
		// SYSTEMPORT_TDMA_DESC_RING_00_PRODUCER_CONSUMER_INDEX
		p_index = sysport_tdma->SYSTEMPORT_TDMA_DESC_RING_PRODUCER_CONSUMER_INDEX[0];
		c_index = (p_index >> 16);
		p_index &= 0xffff;
		c_index &= 0xffff;

	} while ((p_index != c_index) && timeout--);

	if (timeout == 0) {
		// Print SPWF for SP Write Failed

		return -1;
	}

	return 0;
}

int sp_write(uint8_t *buffer, uint32_t length)
{
	if(active_sysport == SYSPORT_0)
		__sp_write(SYSPORT_TDMA, TX_DMA_BUFADDR, buffer, length);
#if defined(SYSPORT_1_TDMA)
	else
		__sp_write(SYSPORT_1_TDMA, TX_DMA_BUFADDR_1, buffer, length);
#endif
	return 0;

}
#endif

#if defined(SYSPORT_V1)

static uint32_t rx_cons_idx;
static uint32_t rx_read_idx;
static void init_pkt_desc(volatile void *ptr)
{
	int i;
	volatile PktDesc *pkt_desc=(PktDesc *)ptr;

	for(i=0;i<(1<<SYSPORT_NUM_RX_PKT_DESC_LOG2);i++)
	{
		pkt_desc[i].address=(uintptr_t)RX_DMA_BUFADDR+(2048*i);
		pkt_desc[i].address_hi=0;
		pkt_desc[i].status=0;
		pkt_desc[i].length=2048;
	}
}
/* Enable System Port TX DMA */
static int sp_enable_tdma(void)
{
	volatile uint32_t reg;
	int timeout = 1000;

	// Disable ACB and enable TDMA
	SYSPORT_TDMA->SYSTEMPORT_TDMA_CONTROL |= 0x01;

	// Wait for TX DMA to become ready
	do {
		reg = SYSPORT_TDMA->SYSTEMPORT_TDMA_STATUS;
		if (!(reg & 0x3)) {
			return 0;
		}
		cfe_usleep(10);

	} while (timeout--);

	return -1;
}/* Enable System Port RX DMA */
static int sp_enable_rdma(void)
{
	volatile uint32_t reg;
	int timeout = 1000;

	// Enable RX DMA and Ring Buffer mode
	SYSPORT_RDMA->SYSTEMPORT_RDMA_CONTROL |= 0x01;

	// Wait for RX DMA to become ready
	do {
		reg = SYSPORT_RDMA->SYSTEMPORT_RDMA_STATUS;
		if (!(reg & 0x3)) {
			return 0;
		}
		cfe_usleep(10);

	} while (timeout--);

	return -1;
}
void sp_reset(int pmc_reset)
{
    volatile uint32_t v32;
    uint16 p_idx, c_idx, timeout;

#if defined(SYSPORT_UMAC)
    // Disable RX UMAC
    v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_UMAC_CMD_RX_ENA_M; /* Disable RX */
    SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;
#endif
#if defined(SYSPORT_1_UMAC)
    // Disable RX UMAC
    v32 = SYSPORT_1_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_1_UMAC_CMD_RX_ENA_M; /* Disable RX */
    SYSPORT_1_UMAC->SYSTEMPORT_UMAC_CMD=v32;
#endif

#if defined (SYSPORT_GIB)
    v32=SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL;
    v32 &= ~SYSPORT_GIB_CONTROL_RX_EN;
    SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL = v32;
#endif

    // in case there is still DMA going on, wait for the last bit to finish
    cfe_usleep(1000);  // will need to increase wait time if system port speed is slower 
                       // (e.g, it takes 1.2ms to transmit 1518 bytes packets when system port is running at 10 Mbps)

    // Disable and Flush RX DMA
    SYSPORT_RDMA->SYSTEMPORT_RDMA_CONTROL &= ~SYSPORT_RDMA_CTRL_RDMA_EN_M;
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_RX_FLUSH_CNTL=1;

    // Disable TX DMA
    SYSPORT_TDMA->SYSTEMPORT_TDMA_CONTROL &= ~SYSPORT_TDMA_CONTROL_TDMA_EN_M;

    // Wait till all TXDMA is completed
    timeout = 1000;
#if defined(SYSPORT_V1)
    v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
#else
	v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC_RING_PRODUCER_CONSUMER_INDEX[0];
#endif
    p_idx = (uint16_t)(v32 & SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_PRODUCER_INDEX_M);
    c_idx = (uint16_t)(v32 >> SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_S);

    while ((p_idx != c_idx) && timeout)
    {
        cfe_usleep(1000);  // will need to increase wait time if system port speed is slower 
                           // (e.g, it takes 1.2ms to transmit 1518 bytes packets when system port is running at 10 Mbps)
        timeout --;
#if defined(SYSPORT_V1)
        v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
#else
	    v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC_RING_PRODUCER_CONSUMER_INDEX[0];
#endif
        p_idx = (uint16_t)(v32 & SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_PRODUCER_INDEX_M);
        c_idx = (uint16_t)(v32 >> SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_S);
    }

#if defined(SYSPORT_UMAC)
    // Disable TX UNIMAC and then flush TXDMA
    v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_UMAC_CMD_TX_ENA_M; /* Disable TX */
    SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;
#endif
#if defined(SYSPORT_1_UMAC)
    // Disable TX UNIMAC and then flush TXDMA
    v32 = SYSPORT_1_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_1_UMAC_CMD_TX_ENA_M; /* Disable TX */
    SYSPORT_1_UMAC->SYSTEMPORT_UMAC_CMD=v32;
#endif
#if defined (SYSPORT_GIB)
    v32=SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL;
    v32 &= ~SYSPORT_GIB_CONTROL_TX_EN;
    SYSPORT_GIB->SYSTEMPORT_GIB_CONTROL = v32;
#endif

    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_TX_FLUSH_CNTL=1;

    // Reset complete, prepare for initialization
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_RX_FLUSH_CNTL=0;
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_TX_FLUSH_CNTL=0;


    if (pmc_reset)
    {
        // reset system port through BPCM
        pmc_sysport_reset_system_port(0);
    }
}

int sp_init(void)
{

	/* all memory allocation are done already */
	uint32_t v32 = 0;


	/* System Port RBUF configuration */
	v32 = SYSPORT_RBUF->SYSTEMPORT_RBUF_RBUF_CONTROL;     /* Read Chip Defaults */
	v32 &= ~SYSPORT_RBUF_CTRL_RSB_EN_M;                   /* Disable RSB */
	v32 &= ~SYSPORT_RBUF_CTRL_4B_ALIGN_M;                 /* Disable 4-Byte IP Alignment */
	v32 &= ~SYSPORT_RBUF_CTRL_BTAG_STRIP_M;               /* Do not strip BRCM TAG */
	v32 |= SYSPORT_RBUF_CTRL_BAD_PKT_DISCARD_M;           /* Discard Bad Packets */
	/* Read-Modify-Write */
	SYSPORT_RBUF->SYSTEMPORT_RBUF_RBUF_CONTROL=v32; 

	SYSPORT_RBUF->SYSTEMPORT_RBUF_RBUF_PACKET_READY_THRESHOLD=0x80; /* Keep chip default */

	/* System Port TBUF configuration -- No change, keep chip defaults */

	/* System Port RDMA Configuration */

	/* RDMA Control Register */
	v32 = SYSPORT_RDMA->SYSTEMPORT_RDMA_CONTROL;  /* Read Chip Defaults */
	v32 &= ~SYSPORT_RDMA_CTRL_RDMA_EN_M;          /* Disable RDMA */
	v32 &= ~SYSPORT_RDMA_CTRL_RING_CFG_M;          /* Enable Descriptor Ring Mode */
	v32 |= SYSPORT_RDMA_CTRL_DISCARD_EN_M;        /* Enable Pkt discard by RDMA when ring full */
	v32 &= ~SYSPORT_RDMA_CTRL_DATA_OFFSET_M;       /* Zero data offset - this feature could be used later to reduce host buffer size */
	v32 &= ~SYSPORT_RDMA_CTRL_DDR_DESC_RD_EN_M;    /* HW reads host desc from local desc memory */
	v32 &= ~SYSPORT_RDMA_CTRL_DDR_DESC_WR_EN_M;
	v32 |= SYSPORT_RDMA_CTRL_DDR_DESC_SWAP_M;     /* Both Byte and word swap enabled for Desc - TBD - need to understand */
	/* Read-Modify-Write */
	SYSPORT_RDMA->SYSTEMPORT_RDMA_CONTROL=v32; 

	/* RDMA Buffer and Ring Size Register */
	v32 = 0;/* Reset register  */
	v32 |= ( (SYSPORT_PKT_LEN_LOG2 << SYSPORT_RDMA_BSRS_BUF_SIZE_LOG2_S) & SYSPORT_RDMA_BSRS_BUF_SIZE_LOG2_M ); /* set buf size */
	v32 |= ( ((1<<SYSPORT_NUM_RX_PKT_DESC_LOG2) << SYSPORT_RDMA_BSRS_RING_SIZE_S) & SYSPORT_RDMA_BSRS_RING_SIZE_M ); /* force chip default of 512 */
	SYSPORT_RDMA->SYSTEMPORT_RDMA_BSRS=v32; 


	/* RDMA Consumer Index Register */
	/* Initialize RX DMA consumer index - low 16 bit; High 16-bits are read-only */
	SYSPORT_RDMA->SYSTEMPORT_RDMA_CONSUMER_INDEX=0x0;
	SYSPORT_RDMA->SYSTEMPORT_RDMA_PRODUCER_INDEX=0x0;

	/* RDMA Desc Start Address Registers */
	/* In desciptor ring mode - start address is index = 0 */
	SYSPORT_RDMA->SYSTEMPORT_RDMA_START_ADDRESS_LOW=0;
	SYSPORT_RDMA->SYSTEMPORT_RDMA_START_ADDRESS_HIGH=0;


	init_pkt_desc(SYSPORT_RDMA->SYSTEMPORT_RDMA_DESCRIPTOR_WORD);

	/* RDMA DDR Desc Ring Register */
	SYSPORT_RDMA->SYSTEMPORT_RDMA_DDR_DESC_RING_START_LOW = 0;
	SYSPORT_RDMA->SYSTEMPORT_RDMA_DDR_DESC_RING_START_HIGH = 0; /* Ideally we should put the Hi 8-bits here */

	/* RDMA DDR Desc Ring Size Register */
	SYSPORT_RDMA->SYSTEMPORT_RDMA_DDR_DESC_RING_SIZE = SYSPORT_NUM_RX_PKT_DESC_LOG2;

	/* RDMA Multi-Buffer-Done-Interrupt-Threshold : No timeout & interrupt every packet */
	SYSPORT_RDMA->SYSTEMPORT_RDMA_MULTIPLE_BUFFERS_DONE_INTERRUPT_THRESHOLD_PUSH_TIMER = 1;
	/* enable DDR DESC write push timer to 1 timer tick (equals 1024 RDMA sys clocks */
	SYSPORT_RDMA->SYSTEMPORT_RDMA_DDR_DESC_RING_PUSH_TIMER = (0x1 & SYSTEMPORT_RDMA_DDR_DESC_RING_PUSH_TIMER_TIMEOUT_M);
	/* TDMA Block Configuration */

	/* System port supports upto 16 Desc Rings;
	Only one TX DDR Desc ring is used; It is mapped to TX-Queue[0] */

	/* Enable TX Q#0 */
	SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_HEAD_TAIL_PTR = SYSPORT_TDMA_DESC_RING_XX_HEAD_TAIL_PTR_RING_EN_M;

	/* Initialize Producer & Consumer Index */
	SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX = 0;
	/* Q#0 DDR Desc Ring Address */
	SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_START_LOW = 0;
	SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_START_HIGH = 0; /* Ideally this should be high 8-bit of address */

	SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_MAPPING = 0x40;
	/* enable DDR DESC read push timer to 1 timer tick (equals 1024 TDMA sys clocks */
	SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_PUSH_TIMER = (0x1 & SYSTEMPORT_TDMA_DDR_DESC_RING_PUSH_TIMER_TIMEOUT_M);
	/* Q#0 DDR Desc Ring Size Log2 */
	SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_SIZE = SYSPORT_NUM_TX_PKT_DESC_LOG2;
	SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_INTR_CONTROL = 0x3;
	SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_MAX_HYST_THRESHOLD = 0x00100009;
	/* enable arbitrator for Q#0 */
	SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_2_ARBITER_CTRL = 0x1; /* Round Robin */
	SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_0_CTRL = 0x1;
	SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_1_CTRL = 0x1;
	SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_2_CTRL = 0x1;
	SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_3_CTRL = 0x1;
	SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_0_QUEUE_ENABLE = 0x000000ff;
	SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_1_QUEUE_ENABLE = 0x0000ff00;
	SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_2_QUEUE_ENABLE = 0x00ff0000;
	SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_3_QUEUE_ENABLE = 0xff000000;
	/* TDMA Control Register */
	v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_CONTROL; /* Read chip defaults */
	v32 &= ~SYSPORT_TDMA_CONTROL_TSB_EN_M; /* Disable TSB */
	v32 &= ~(SYSPORT_TDMA_CONTROL_DDR_DESC_RING_EN_M); /* Disable DDR Desc Ring fetch */
	v32 |= SYSPORT_TDMA_CONTROL_NO_ACB_M; /* No ACB */
	SYSPORT_TDMA->SYSTEMPORT_TDMA_CONTROL = v32;

	/* Enable Tier-1 arbiter for Q#0 */
	SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_0_QUEUE_ENABLE=0x1;

	// Enable RX DMA
	if (sp_enable_rdma() < 0) {
		// Print SPIF for System Port Init Failed

		return -1;
	}

	// Enable TX DMA
	if (sp_enable_tdma() < 0) {
		// Print SPIF for System Port Init Failed

		return -1;
	}

	v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
	v32 |= SYSPORT_UMAC_CMD_TX_ENA_M; /* Enable TX */
	v32 |= SYSPORT_UMAC_CMD_RX_ENA_M; /* Enable RX */
	SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;


	// Give SP some time to initialize
	cfe_usleep(100);

	// Print SPIP for System Port Init Passed

        bcm_ethsw_open();


	return 0;
}
int sp_poll(uint32_t timeout_ms)
{
	int i = 0;
	uint32_t p_index, c_index;

	// SYSTEMPORT_RDMA_PRODUCER_INDEX...
	p_index = SYSPORT_RDMA->SYSTEMPORT_RDMA_PRODUCER_INDEX & 0xffff;
	c_index = SYSPORT_RDMA->SYSTEMPORT_RDMA_CONSUMER_INDEX & 0xffff;

	while (p_index == c_index) {

		if (i++ > timeout_ms)
			return -1;

		cfe_usleep(1000);

		p_index = SYSPORT_RDMA->SYSTEMPORT_RDMA_PRODUCER_INDEX & 0xffff;
		c_index = SYSPORT_RDMA->SYSTEMPORT_RDMA_CONSUMER_INDEX & 0xffff;
	}

	return 0;
}

int sp_read(uint8_t *buffer, uint32_t *length)
{
	int rc = 0;
	const uint8_t *bufaddr;
	volatile PktDesc *pkt_desc;

	// Get RX DMA buffer address
	bufaddr = (uint8_t *)cache_to_uncache(RX_DMA_BUFADDR) + rx_read_idx * NET_DMA_BUFSIZE;
	pkt_desc=(PktDesc *)SYSPORT_RDMA->SYSTEMPORT_RDMA_DESCRIPTOR_WORD;
	bufaddr = (uint8_t *)cache_to_uncache((const uint8_t *)NULL + pkt_desc[rx_read_idx].address);

	*length=pkt_desc[rx_read_idx].length;


	if (*length < ENET_ZLEN || *length > MAX_PKT_LEN) {
		// Print SPRE for System Port Read Error

		rc = -1;
		goto out;
	}

	memcpy(buffer, bufaddr, *length);

out:
	// Advance RX read index
	rx_read_idx++;
	rx_read_idx &= (NUM_RX_DMA_BUFFERS - 1);

	// Advance RX consumer index in HW
	rx_cons_idx++;
	rx_cons_idx &= 0xffff;
	SYSPORT_RDMA->SYSTEMPORT_RDMA_CONSUMER_INDEX=rx_cons_idx;

	return rc;
}

int sp_write(uint8_t *buffer, uint32_t length)
{
	int timeout = 1000;
	uint32_t p_index, c_index;
	uint32_t txdesc_hi, txdesc_lo;

	if (length > MAX_PKT_LEN) {
		// Print SPWE for SP Write Error

		return -1;
	}

	memcpy((void *)cache_to_uncache(TX_DMA_BUFADDR), buffer, length);

	// set up txdesc record: eop=1, sop=1, append_crc=1
	txdesc_hi = (length << 18) | (3 << 16) | (1 << 11);
	txdesc_lo = (uint32_t)K1_TO_PHYS((unsigned long)TX_DMA_BUFADDR);

	// write tx desc to hw
	// SYSTEMPORT_TDMA_DESCRIPTOR_00_WRITE_PORT_LO
	SYSPORT_TDMA->SYSTEMPORT_TDMA_DESCRIPTOR_WRITE_PORT[0].SYSTEMPORT_TDMA_DESCRIPTOR_XX_WRITE_PORT_LO=txdesc_lo;

        __asm__ __volatile__ ("dsb    sy");

	// SYSTEMPORT_TDMA_DESCRIPTOR_00_WRITE_PORT_HI
	SYSPORT_TDMA->SYSTEMPORT_TDMA_DESCRIPTOR_WRITE_PORT[0].SYSTEMPORT_TDMA_DESCRIPTOR_XX_WRITE_PORT_HI=txdesc_hi;

	// wait until the packet is fully DMA'ed and stored into TBUF
	do {
		// SYSTEMPORT_TDMA_DESC_RING_00_PRODUCER_CONSUMER_INDEX
		p_index = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
		c_index = (p_index >> 16);
		p_index &= 0xffff;
		c_index &= 0xffff;

	} while ((p_index != c_index) & timeout--);

	if (timeout == 0) {
		// Print SPWF for SP Write Failed

		return -1;
	}

	return 0;
}
#endif




/** Functions. **/
static void bcm63xx_ether_probe( cfe_driver_t * drv,    unsigned long probe_a,
                                unsigned long probe_b, void * probe_ptr )
{
    char descr[100];

    sp_reset(0);

    dma_buffer=KMALLOC(NET_DMA_BUFSIZE *(NUM_TX_DMA_BUFFERS+NUM_RX_DMA_BUFFERS), CACHE_ALIGN);

    if(dma_buffer == NULL)
    {
        printf("PANIC: dma buffer allocation failed sysport 0\n");
        return;
    }
    memset(dma_buffer, 0x00, NET_DMA_BUFSIZE *(NUM_TX_DMA_BUFFERS+NUM_RX_DMA_BUFFERS));
    FLUSH_RANGE(dma_buffer, NET_DMA_BUFSIZE *(NUM_TX_DMA_BUFFERS+NUM_RX_DMA_BUFFERS));

#if defined(SYSPORT_1_TPC) 
    dma_buffer_1=KMALLOC(NET_DMA_BUFSIZE *(NUM_TX_DMA_BUFFERS+NUM_RX_DMA_BUFFERS), CACHE_ALIGN);

    if(dma_buffer_1 == NULL)
    {
        printf("PANIC: dma buffer allocation failed sysport 1\n");
        return;
    }
    memset(dma_buffer_1, 0x00, NET_DMA_BUFSIZE *(NUM_TX_DMA_BUFFERS+NUM_RX_DMA_BUFFERS));
    FLUSH_RANGE(dma_buffer_1, NET_DMA_BUFSIZE *(NUM_TX_DMA_BUFFERS+NUM_RX_DMA_BUFFERS));
#endif


    sp_init();

    sprintf(descr, "%s eth %d", drv->drv_description, probe_a);
    cfe_attach(drv, NULL, NULL, descr);


}

static int bcm63xx_ether_open(cfe_devctx_t *ctx)
{
return 0;
}

static int bcm63xx_ether_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{

        int rc = 0;
        if(RX_DMA_BUFADDR == NULL) {
                return -1;
        }

	if(sp_poll(1) == 0)
	{

		rc=sp_read(buffer->buf_ptr, (uint32_t *)&buffer->buf_retlen);
       	}
	return rc;
}

static int bcm63xx_ether_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat)
{
return -1; 
}

static int bcm63xx_ether_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{

        if (buffer->buf_length > MAX_PKT_LEN) {
                return -1;
        }


	return sp_write(buffer->buf_ptr, buffer->buf_length);

}

static int bcm63xx_ether_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
	//TODO:
return -1; 
}

static int bcm63xx_ether_close(cfe_devctx_t *ctx)
{
    /* when linux init calls bcm_ethsw_init, we can call pmc_sysport_powerdown
       here to completely showdown the sysport. bcm_ethsw_init call pmc_sysport_powerup */
    sp_reset(1);
    return 0; 
}



