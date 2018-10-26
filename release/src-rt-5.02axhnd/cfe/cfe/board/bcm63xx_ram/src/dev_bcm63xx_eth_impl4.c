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

static uint32_t rx_cons_idx;
static uint32_t rx_read_idx;

typedef struct PktDesc {
    uint32_t address ;
    uint32_t address_hi :8;
    uint32_t status :10;
    uint32_t length :14;
}PktDesc;


#define SYSPORT_NUM_RX_PKT_DESC_LOG2  (8)

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
#define NET_DMA_BASE                    ((uint64_t)dma_buffer)

/* Address of TX DMA buffer */
#define TX_DMA_BUFADDR                  (NET_DMA_BASE)

/* Address of first RX DMA buffer */
#define RX_DMA_BUFADDR                  (NET_DMA_BASE + (NET_DMA_BUFSIZE*NUM_TX_DMA_BUFFERS))


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

/* Enable System Port RX DMA */
int sp_enable_rdma(void)
{
        volatile uint32_t reg;
        int timeout = 1000;

        // Enable RX DMA and Ring Buffer mode
        SYSPORT_RDMA->SYSTEMPORT_RDMA_CONTROL |=0x01;

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

/* Enable System Port TX DMA */
int sp_enable_tdma(void)
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
}


static void reset_sp(void)
{
    volatile uint32_t v32;
    uint16 p_idx, c_idx, timeout;

    // Disable RX UMAC
    v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_UMAC_CMD_RX_ENA_M; /* Disable RX */
    SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;
    
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
    v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
    p_idx = (uint16_t)(v32 & SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_PRODUCER_INDEX_M);
    c_idx = (uint16_t)(v32 >> SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_S);

    while ((p_idx != c_idx) && timeout)
    {
        cfe_usleep(1000);  // will need to increase wait time if system port speed is slower 
                           // (e.g, it takes 1.2ms to transmit 1518 bytes packets when system port is running at 10 Mbps)
        timeout --;
        v32 = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
        p_idx = (uint16_t)(v32 & SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_PRODUCER_INDEX_M);
        c_idx = (uint16_t)(v32 >> SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX_CONSUMER_INDEX_S);
    }

    // Disable TX UNIMAC and then flush TXDMA
    v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_UMAC_CMD_TX_ENA_M; /* Disable TX */
    SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_TX_FLUSH_CNTL=1;

    // Reset complete, prepare for initialization
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_RX_FLUSH_CNTL=0;
    SYSPORT_TPC->SYSTEMPORT_TOPCTRL_TX_FLUSH_CNTL=0;
    SYSPORT_TDMA->SYSTEMPORT_TDMA_STATUS |= SYSPORT_TDMA_STATUS_LL_RAM_INIT_BUSY_M; // initialize the Link List RAM
}


static void init_pkt_desc(unsigned char *ptr)
{
	int i;
	volatile PktDesc *pkt_desc=(PktDesc *)SYSPORT_RDMA->SYSTEMPORT_RDMA_DESCRIPTOR_WORD;

	for(i=0;i<(1<<SYSPORT_NUM_RX_PKT_DESC_LOG2);i++)
	{
		pkt_desc[i].address=(uintptr_t)(ptr+(NET_DMA_BUFSIZE*(i+1)));
		pkt_desc[i].address_hi=0;
		pkt_desc[i].status=0;
		pkt_desc[i].length=NET_DMA_BUFSIZE;
	}
}

int sp_init(void)
{

    /* all memory allocation are done already */
    uint32_t v32 = 0;

   dma_buffer=KMALLOC(NET_DMA_BUFSIZE *(NUM_TX_DMA_BUFFERS+NUM_RX_DMA_BUFFERS), CACHE_ALIGN);

   if(dma_buffer == NULL)
   {
        printf("PANIC: dma buffer allocation failed\n");
        return -1;
   }
   memset(dma_buffer, 0x00, NET_DMA_BUFSIZE *(NUM_TX_DMA_BUFFERS+NUM_RX_DMA_BUFFERS));
   FLUSH_RANGE(dma_buffer, NET_DMA_BUFSIZE *(NUM_TX_DMA_BUFFERS+NUM_RX_DMA_BUFFERS));

   reset_sp();

   rx_cons_idx = 0;
   rx_read_idx = 0;
    

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
    //v32 |= ( (0x200 << SYSPORT_RDMA_BSRS_RING_SIZE_S) & SYSPORT_RDMA_BSRS_RING_SIZE_M ); /* force chip default of 512 */
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


    init_pkt_desc(dma_buffer);

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
    SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_START_LOW = 0;///anandg (uint32)(uintptr_t)g_pTxPktDesc->desc_ptr.p_desc_physical_addr;
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
    v32 |= SYSPORT_TDMA_CONTROL_NO_ACB_M; /* No ACB */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_CONTROL = v32;

    /* Enable Tier-1 arbiter for Q#0 */
    SYSPORT_TDMA->SYSTEMPORT_TDMA_TIER_1_ARBITER_0_QUEUE_ENABLE=0x1;

    /* Initialize interrupts */
    SYSPORT_INTR0->SYSTEMPORT_INTR_CPU_MASK_SET = SYSPORT_INTR_ALL_INTR_MASK; /* Disable all interrupts */
    SYSPORT_INTR0->SYSTEMPORT_INTR_CPU_CLEAR = SYSPORT_INTR_ALL_INTR_MASK;    /* Clear all interrupts */ 

   // Enable RX DMA
   if (sp_enable_rdma() < 0) {
	   // Print SPIF for System Port Init Failed
	  printf("Error enabling rdma\n");
	   return -1;
   }

   // Enable TX DMA
   if (sp_enable_tdma() < 0) {
	   // Print SPIF for System Port Init Failed
	  printf("Error enabling tdma\n");
	   return -1;
   }

    v32 = SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD;
    v32 |= SYSPORT_UMAC_CMD_TX_ENA_M; /* Enable TX */
    v32 |= SYSPORT_UMAC_CMD_RX_ENA_M; /* Enable RX */
    SYSPORT_UMAC->SYSTEMPORT_UMAC_CMD=v32;


   // Give SP some time to initialize
   cfe_usleep(100);

    bcm_ethsw_open();
   

   return 0;
}



/** Functions. **/
static void bcm63xx_ether_probe( cfe_driver_t * drv,    unsigned long probe_a,
                                unsigned long probe_b, void * probe_ptr )
{
    char descr[100];


    sp_init();

    sprintf(descr, "%s eth %d", drv->drv_description, probe_a);
    cfe_attach(drv, NULL, NULL, descr);


}

static int bcm63xx_ether_open(cfe_devctx_t *ctx)
{
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


static int bcm63xx_ether_read(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{

        int rc = 0;
        const uint8_t *bufaddr;
	volatile PktDesc *pkt_desc;

        if(RX_DMA_BUFADDR == NULL) {
                return -1;
        }

	if(sp_poll(1) == 0)
	{
		// Get RX DMA buffer address
		pkt_desc=(PktDesc *)SYSPORT_RDMA->SYSTEMPORT_RDMA_DESCRIPTOR_WORD;
		bufaddr = (uint8_t *)cache_to_uncache((const uint8_t *)NULL + pkt_desc[rx_read_idx].address);

		// Get packet length from RSB.
		buffer->buf_retlen = pkt_desc[rx_read_idx].length;

		if (buffer->buf_retlen < ENET_ZLEN || buffer->buf_retlen > MAX_PKT_LEN) {
			rc = -1;
			buffer->buf_retlen = 0;
			goto out;
		}

		memcpy(buffer->buf_ptr, bufaddr, buffer->buf_retlen);

	out:
		// Advance RX read index
		rx_read_idx++;
		rx_read_idx &= (NUM_RX_DMA_BUFFERS - 1);

		// Advance RX consumer index in HW
		rx_cons_idx++;
		rx_cons_idx &= 0xffff;
		SYSPORT_RDMA->SYSTEMPORT_RDMA_CONSUMER_INDEX=rx_cons_idx;
	}
	else
		rc=-1;

        return rc;


}

static int bcm63xx_ether_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat)
{
return -1; 
}

static int bcm63xx_ether_write(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{

        int timeout = 1000;
        volatile uint32_t p_index, c_index;
        uint32_t txdesc_hi, txdesc_lo;

        if (buffer->buf_length > MAX_PKT_LEN) {
                return -1;
        }

        if(TX_DMA_BUFADDR == NULL) {
                return -1;
        }

        memcpy((void *)cache_to_uncache(TX_DMA_BUFADDR), buffer->buf_ptr, buffer->buf_length);

        // set up txdesc record: eop=1, sop=1, append_crc=1
        txdesc_hi = (buffer->buf_length << 18) | (3 << 16) | (1 << 11);
        txdesc_lo = (uint32_t)K1_TO_PHYS(TX_DMA_BUFADDR);


	__asm__ __volatile__ ("dsb    sy");

        // write tx desc to hw
        // SYSTEMPORT_TDMA_DESCRIPTOR_00_WRITE_PORT_LO
        SYSPORT_TDMA->SYSTEMPORT_TDMA_DESCRIPTOR_WRITE_PORT[0].SYSTEMPORT_TDMA_DESCRIPTOR_XX_WRITE_PORT_LO=txdesc_lo;

        // SYSTEMPORT_TDMA_DESCRIPTOR_00_WRITE_PORT_HI
        SYSPORT_TDMA->SYSTEMPORT_TDMA_DESCRIPTOR_WRITE_PORT[0].SYSTEMPORT_TDMA_DESCRIPTOR_XX_WRITE_PORT_HI=txdesc_hi;

        // wait until the packet is fully DMA'ed and stored into TBUF
        do {
                // SYSTEMPORT_TDMA_DESC_RING_00_PRODUCER_CONSUMER_INDEX
                p_index = SYSPORT_TDMA->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
                c_index = (p_index >> 16);
                p_index &= 0xffff;
                c_index &= 0xffff;

                timeout--;

        } while ((p_index != c_index) && timeout);

        if (timeout == 0) {
                return -1;
        }

        return 0;

}

static int bcm63xx_ether_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
	//TODO:
return -1; 
}

static int bcm63xx_ether_close(cfe_devctx_t *ctx)
{
    reset_sp();
    return 0; 
}



