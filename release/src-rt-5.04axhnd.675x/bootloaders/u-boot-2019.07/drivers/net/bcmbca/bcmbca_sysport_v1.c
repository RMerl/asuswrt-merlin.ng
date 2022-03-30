/* SPDX-License-Identifier: GPL-2.0+
*  *
*   *  Copyright 2019 Broadcom Ltd.
*    */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <net.h>
#include <config.h>
#include <console.h>
#include <malloc.h>
#include <asm/io.h>
#include <phy.h>
#include <miiphy.h>
#include <wait_bit.h>
#include <watchdog.h>
#include <asm/system.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <environment.h>

#include "bcmbca_sysport_v1.h"
#include "bcm_ethsw.h"

#define MAX_PKT_LEN                             (NET_DMA_BUFSIZE)

struct sysport_priv
{
	int init;
	int rx_read_idx;
	int rx_cons_idx;
	unsigned char *buffers;
	sys_port_rbuf *sys_port_rbuf;
	sys_port_rdma *sys_port_rdma;
	sys_port_tdma *sys_port_tdma;
	sys_port_umac *sys_port_umac;
	sys_port_topctrl *sys_port_topctrl;

	struct udevice *sf2_devp;
	bcm_ethsw_ops_t *sf2_ops;
};


static void init_pkt_desc(volatile void *ptr, unsigned char *rx_dma_bufaddr)
{
	int i;
	volatile PktDesc *pkt_desc=(PktDesc *)ptr;

	for(i=0;i<(1<<SYSPORT_NUM_RX_PKT_DESC_LOG2);i++)
	{
		pkt_desc[i].address=(uintptr_t)rx_dma_bufaddr+(2048*i);
		pkt_desc[i].address_hi=0;
		pkt_desc[i].status=0;
		pkt_desc[i].length=2048;
	}
}

/* Enable System Port TX DMA */
static int sysport_enable_tdma(struct sysport_priv *priv)
{
	volatile uint32_t reg;
	int timeout = 1000;

	// Disable ACB and enable TDMA
	priv->sys_port_tdma->SYSTEMPORT_TDMA_CONTROL |= 0x01;

	// Wait for TX DMA to become ready
	do {
		reg = priv->sys_port_tdma->SYSTEMPORT_TDMA_STATUS;
		if (!(reg & 0x3)) {
			return 0;
		}
		udelay(10);

	} while (timeout--);

	return -1;
}
/* Enable System Port RX DMA */
static int sysport_enable_rdma(struct sysport_priv *priv)
{
	volatile uint32_t reg;
	int timeout = 1000;

	// Enable RX DMA and Ring Buffer mode
	priv->sys_port_rdma->SYSTEMPORT_RDMA_CONTROL |= 0x01;

	// Wait for RX DMA to become ready
	do {
		reg = priv->sys_port_rdma->SYSTEMPORT_RDMA_STATUS;
		if (!(reg & 0x3)) {
			return 0;
		}
		udelay(10);

	} while (timeout--);

	return -1;
}



static int sysport_init(struct udevice *dev)
{
	uint32_t v32 = 0;
	struct sysport_priv *priv = dev_get_priv(dev);

	if (!priv->init)
	{
		if (priv->sf2_devp)
			priv->sf2_ops->open (priv->sf2_devp);

		/* System Port RBUF configuration */
		v32 = priv->sys_port_rbuf->SYSTEMPORT_RBUF_RBUF_CONTROL;     /* Read Chip Defaults */
		v32 &= ~SYSPORT_RBUF_CTRL_RSB_EN_M;
		v32 &= ~SYSPORT_RBUF_CTRL_4B_ALIGN_M;                 /* Disable 4-Byte IP Alignment */
		v32 &= ~SYSPORT_RBUF_CTRL_BTAG_STRIP_M;               /* Do not strip BRCM TAG */
		v32 |= SYSPORT_RBUF_CTRL_BAD_PKT_DISCARD_M;           /* Discard Bad Packets */
		/* Read-Modify-Write */
		priv->sys_port_rbuf->SYSTEMPORT_RBUF_RBUF_CONTROL=v32;

		priv->sys_port_rbuf->SYSTEMPORT_RBUF_RBUF_PACKET_READY_THRESHOLD=0x80; /* Keep chip default */

		/* System Port TBUF configuration -- No change, keep chip defaults */

		/* System Port RDMA Configuration */

		/* RDMA Control Register */
		v32 = priv->sys_port_rdma->SYSTEMPORT_RDMA_CONTROL;  /* Read Chip Defaults */
		v32 &= ~SYSPORT_RDMA_CTRL_RDMA_EN_M;          /* Disable RDMA */
		v32 &= ~SYSPORT_RDMA_CTRL_RING_CFG_M;
		v32 |= SYSPORT_RDMA_CTRL_DISCARD_EN_M;        /* Enable Pkt discard by RDMA when ring full */
		v32 &= ~SYSPORT_RDMA_CTRL_DATA_OFFSET_M;       /* Zero data offset - this feature could be used later to reduce host buffer size */
		v32 &= ~SYSPORT_RDMA_CTRL_DDR_DESC_RD_EN_M;    /* HW reads host desc from local desc memory */
		v32 &= ~SYSPORT_RDMA_CTRL_DDR_DESC_WR_EN_M;
		v32 |= SYSPORT_RDMA_CTRL_DDR_DESC_SWAP_M;     /* Both Byte and word swap enabled for Desc - TBD - need to understand */
		/* Read-Modify-Write */
		priv->sys_port_rdma->SYSTEMPORT_RDMA_CONTROL=v32;

		/* RDMA Buffer and Ring Size Register */
		v32 = 0;/* Reset register  */
		v32 |= ( (SYSPORT_PKT_LEN_LOG2 << SYSPORT_RDMA_BSRS_BUF_SIZE_LOG2_S) & SYSPORT_RDMA_BSRS_BUF_SIZE_LOG2_M ); /* set buf size */
		v32 |= ( ((1<<SYSPORT_NUM_RX_PKT_DESC_LOG2) << SYSPORT_RDMA_BSRS_RING_SIZE_S) & SYSPORT_RDMA_BSRS_RING_SIZE_M ); /* force chip default of 512 */
		priv->sys_port_rdma->SYSTEMPORT_RDMA_BSRS=v32;
		//priv->sys_port_rdma->SYSTEMPORT_RDMA_BSRS=8388619;


		/* RDMA Consumer Index Register */
		/* Initialize RX DMA consumer index - low 16 bit; High 16-bits are read-only */
		priv->sys_port_rdma->SYSTEMPORT_RDMA_CONSUMER_INDEX=0x0;
		priv->sys_port_rdma->SYSTEMPORT_RDMA_PRODUCER_INDEX=0x0;

		/* RDMA Desc Start Address Registers */
		/* In desciptor ring mode - start address is index = 0 */
		priv->sys_port_rdma->SYSTEMPORT_RDMA_START_ADDRESS_LOW=0;
		priv->sys_port_rdma->SYSTEMPORT_RDMA_START_ADDRESS_HIGH=0;


		init_pkt_desc(priv->sys_port_rdma->SYSTEMPORT_RDMA_DESCRIPTOR_WORD, priv->buffers+(NET_DMA_BUFSIZE*NUM_TX_DMA_BUFFERS));

		/* RDMA DDR Desc Ring Register */
		priv->sys_port_rdma->SYSTEMPORT_RDMA_DDR_DESC_RING_START_LOW = 0;
		priv->sys_port_rdma->SYSTEMPORT_RDMA_DDR_DESC_RING_START_HIGH = 0; /* Ideally we should put the Hi 8-bits here */

		/* RDMA DDR Desc Ring Size Register */
		priv->sys_port_rdma->SYSTEMPORT_RDMA_DDR_DESC_RING_SIZE = SYSPORT_NUM_RX_PKT_DESC_LOG2;

		/* RDMA Multi-Buffer-Done-Interrupt-Threshold : No timeout & interrupt every packet */
		priv->sys_port_rdma->SYSTEMPORT_RDMA_MULTIPLE_BUFFERS_DONE_INTERRUPT_THRESHOLD_PUSH_TIMER = 1;
		/* enable DDR DESC write push timer to 1 timer tick (equals 1024 RDMA sys clocks */
		priv->sys_port_rdma->SYSTEMPORT_RDMA_DDR_DESC_RING_PUSH_TIMER = (0x1 & SYSTEMPORT_RDMA_DDR_DESC_RING_PUSH_TIMER_TIMEOUT_M);
		/* TDMA Block Configuration */

		/* System port supports upto 16 Desc Rings;
		Only one TX DDR Desc ring is used; It is mapped to TX-Queue[0] */

		/* Enable TX Q#0 */
		priv->sys_port_tdma->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_HEAD_TAIL_PTR = SYSPORT_TDMA_DESC_RING_XX_HEAD_TAIL_PTR_RING_EN_M;

		/* Initialize Producer & Consumer Index */
		priv->sys_port_tdma->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX = 0;
		/* Q#0 DDR Desc Ring Address */
		priv->sys_port_tdma->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_START_LOW = 0;
		priv->sys_port_tdma->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_START_HIGH = 0; /* Ideally this should be high 8-bit of address */

		priv->sys_port_tdma->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_MAPPING = 0x40;
		/* enable DDR DESC read push timer to 1 timer tick (equals 1024 TDMA sys clocks */
		priv->sys_port_tdma->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_PUSH_TIMER = (0x1 & SYSTEMPORT_TDMA_DDR_DESC_RING_PUSH_TIMER_TIMEOUT_M);
		/* Q#0 DDR Desc Ring Size Log2 */
		priv->sys_port_tdma->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DDR_DESC_RING_XX_SIZE = SYSPORT_NUM_TX_PKT_DESC_LOG2;
		priv->sys_port_tdma->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_INTR_CONTROL = 0x3;
		priv->sys_port_tdma->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_MAX_HYST_THRESHOLD = 0x00100009;
		/* enable arbitrator for Q#0 */
		priv->sys_port_tdma->SYSTEMPORT_TDMA_TIER_2_ARBITER_CTRL = 0x1; /* Round Robin */
		priv->sys_port_tdma->SYSTEMPORT_TDMA_TIER_1_ARBITER_0_CTRL = 0x1;
		priv->sys_port_tdma->SYSTEMPORT_TDMA_TIER_1_ARBITER_1_CTRL = 0x1;
		priv->sys_port_tdma->SYSTEMPORT_TDMA_TIER_1_ARBITER_2_CTRL = 0x1;
		priv->sys_port_tdma->SYSTEMPORT_TDMA_TIER_1_ARBITER_3_CTRL = 0x1;
		priv->sys_port_tdma->SYSTEMPORT_TDMA_TIER_1_ARBITER_0_QUEUE_ENABLE = 0x000000ff;
		priv->sys_port_tdma->SYSTEMPORT_TDMA_TIER_1_ARBITER_1_QUEUE_ENABLE = 0x0000ff00;
		priv->sys_port_tdma->SYSTEMPORT_TDMA_TIER_1_ARBITER_2_QUEUE_ENABLE = 0x00ff0000;
		priv->sys_port_tdma->SYSTEMPORT_TDMA_TIER_1_ARBITER_3_QUEUE_ENABLE = 0xff000000;
		/* TDMA Control Register */
		v32 = priv->sys_port_tdma->SYSTEMPORT_TDMA_CONTROL; /* Read chip defaults */
		v32 &= ~SYSPORT_TDMA_CONTROL_TSB_EN_M; /* Disable TSB */
		v32 &= ~(SYSPORT_TDMA_CONTROL_DDR_DESC_RING_EN_M); /* Disable DDR Desc Ring fetch */
		v32 |= SYSPORT_TDMA_CONTROL_NO_ACB_M; /* No ACB */
		priv->sys_port_tdma->SYSTEMPORT_TDMA_CONTROL = v32;

		/* Enable Tier-1 arbiter for Q#0 */
		priv->sys_port_tdma->SYSTEMPORT_TDMA_TIER_1_ARBITER_0_QUEUE_ENABLE=0x1;

		// Enable RX DMA
		if (sysport_enable_rdma(priv) < 0) {
			return -1;
		}

		// Enable TX DMA
		if (sysport_enable_tdma(priv) < 0) {
			return -1;
		}

		v32 = priv->sys_port_umac->SYSTEMPORT_UMAC_CMD;
		v32 |= SYSPORT_UMAC_CMD_TX_ENA_M; /* Enable TX */
		v32 |= SYSPORT_UMAC_CMD_RX_ENA_M; /* Enable RX */
		priv->sys_port_umac->SYSTEMPORT_UMAC_CMD=v32;


		// Give SP some time to initialize
		udelay(100);

	}
	priv->init++;
	return 0;
}


static int sysport_send(struct udevice *dev, void *buffer, int length)
{
	int timeout = 1000;
	struct sysport_priv *priv = dev_get_priv(dev);
	volatile uint32_t p_index, c_index;
	uint32_t txdesc_hi, txdesc_lo;

	if (length > MAX_PKT_LEN) {
		return -1;
	}

	memcpy((void *)(priv->buffers), buffer, length);
	flush_cache((unsigned long)(priv->buffers), length);

	// set up txdesc record: eop=1, sop=1, append_crc=1
	txdesc_hi = (length << 18) | (3 << 16) | (1 << 11);
	txdesc_lo = (uint32_t)priv->buffers;

	// write tx desc to hw
	// SYSTEMPORT_TDMA_DESCRIPTOR_00_WRITE_PORT_LO
	priv->sys_port_tdma->SYSTEMPORT_TDMA_DESCRIPTOR_WRITE_PORT[0].SYSTEMPORT_TDMA_DESCRIPTOR_XX_WRITE_PORT_LO=txdesc_lo;

	barrier();

	// SYSTEMPORT_TDMA_DESCRIPTOR_00_WRITE_PORT_HI
	priv->sys_port_tdma->SYSTEMPORT_TDMA_DESCRIPTOR_WRITE_PORT[0].SYSTEMPORT_TDMA_DESCRIPTOR_XX_WRITE_PORT_HI=txdesc_hi;

	// wait until the packet is fully DMA'ed and stored into TBUF
	do {
		// SYSTEMPORT_TDMA_DESC_RING_00_PRODUCER_CONSUMER_INDEX
		p_index = priv->sys_port_tdma->SYSTEMPORT_TDMA_DESC[0].SYSTEMPORT_TDMA_DESC_RING_XX_PRODUCER_CONSUMER_INDEX;
		c_index = (p_index >> 16);
		p_index &= 0xffff;
		c_index &= 0xffff;

	} while ((p_index != c_index) & timeout--);

	if (timeout == 0) {
		return -1;
	}

	return 0;


}

static int sysport_poll(struct sysport_priv *priv, uint32_t timeout_ms)
{
	int i = 0;
	uint32_t p_index, c_index;

	// SYSTEMPORT_RDMA_PRODUCER_INDEX...
	p_index = priv->sys_port_rdma->SYSTEMPORT_RDMA_PRODUCER_INDEX & 0xffff;
	c_index = priv->sys_port_rdma->SYSTEMPORT_RDMA_CONSUMER_INDEX & 0xffff;

	if (p_index == c_index)
		return -1;

	return 0;
}

static int sysport_recv(struct udevice *dev, int flags, uchar **packetp)
{
	int rc = 0;
	const uint8_t *bufaddr;
	struct sysport_priv *priv = dev_get_priv(dev);
	volatile PktDesc *pkt_desc;
	int length;

	if(sysport_poll(priv, 10) != 0)
	{
		return -1;
	}


	// Get RX DMA buffer address
	pkt_desc=(PktDesc *)priv->sys_port_rdma->SYSTEMPORT_RDMA_DESCRIPTOR_WORD;

	//flush_cache((void *)((const uint8_t *)NULL + pkt_desc[rx_read_idx].address, NET_DMA_BUFSIZE);
	bufaddr = (uint8_t *)(const uint8_t *)NULL + pkt_desc[priv->rx_read_idx].address;
	invalidate_dcache_range((unsigned long)bufaddr, (unsigned long) (bufaddr+NET_DMA_BUFSIZE));
	barrier();
	length=pkt_desc[priv->rx_read_idx%NUM_RX_DMA_BUFFERS].length;

	if (length < ENET_ZLEN || length > MAX_PKT_LEN)
	{
		rc = -1;
		goto out;
	}
	

	*packetp=(unsigned char *) memalign(sizeof(int), MAX_PKT_LEN);
	if (*packetp == NULL) {
		printf("%s: malloc failed\n", __func__);
		return -ENOMEM;
	}

	memcpy(*packetp, bufaddr, length);
	rc=length;
out:
	// Advance RX read index
	priv->rx_read_idx++;
	priv->rx_read_idx &= (NUM_RX_DMA_BUFFERS - 1);

	// Advance RX consumer index in HW
	priv->rx_cons_idx++;
	priv->rx_cons_idx &= 0xffff;
	priv->sys_port_rdma->SYSTEMPORT_RDMA_CONSUMER_INDEX=priv->rx_cons_idx;

	return rc;
}

static int sysport_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	if(packet != NULL)
		free(packet);
	return 0;
}

static void sysport_halt(struct udevice *dev)
{
}

static int sysport_read_rom_mac(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	u8 mac_addr[6]={0x02,0x10,0x00,0x00,0x00,0x01};
        u8 mac_temp[6];

	if (!eth_env_get_enetaddr("ethaddr", mac_temp))
	{
		eth_env_set_enetaddr("ethaddr", mac_addr);
	}
	return 0;
}

static const struct eth_ops sysport_ops = {
	.start			= sysport_init,
	.send			= sysport_send,
	.recv			= sysport_recv,
	.free_pkt		= sysport_free_pkt,
	.stop			= sysport_halt,
//	.write_hwaddr		= sysport_setup_mac,
	.read_rom_hwaddr	= sysport_read_rom_mac,
};


void sysport_reset(struct sysport_priv *priv)
{

    uint32_t v32=0;

    v32 = priv->sys_port_umac->SYSTEMPORT_UMAC_CMD;
    v32 &= ~SYSPORT_UMAC_CMD_TX_ENA_M; /* Disable TX */
    v32 &= ~SYSPORT_UMAC_CMD_RX_ENA_M; /* Disable RX */
    priv->sys_port_umac->SYSTEMPORT_UMAC_CMD=v32;
    udelay(1000);
    priv->sys_port_rdma->SYSTEMPORT_RDMA_CONTROL &= ~SYSPORT_RDMA_CTRL_RDMA_EN_M;
    udelay(1000);
    //RXCHK_CONTROL=0;
    priv->sys_port_tdma->SYSTEMPORT_TDMA_CONTROL &= ~SYSPORT_TDMA_CONTROL_TDMA_EN_M;
    udelay(1000);
    priv->sys_port_topctrl->SYSTEMPORT_TOPCTRL_RX_FLUSH_CNTL=1;
    priv->sys_port_topctrl->SYSTEMPORT_TOPCTRL_TX_FLUSH_CNTL=1;
    udelay(1000);
    priv->sys_port_topctrl->SYSTEMPORT_TOPCTRL_RX_FLUSH_CNTL=0;
    priv->sys_port_topctrl->SYSTEMPORT_TOPCTRL_TX_FLUSH_CNTL=0;
    udelay(1000);
    priv->sys_port_tdma->SYSTEMPORT_TDMA_STATUS = 0x2; // initialize the Link List RAM.
    udelay(10);
}

static int sysport_remove(struct udevice *dev)
{
	struct sysport_priv *priv = dev_get_priv(dev);

	if (priv->sf2_devp)
		priv->sf2_ops->close(priv->sf2_devp);
	
	sysport_reset(priv);
	return 0;
}

static int sysport_probe(struct udevice *dev)
{
	struct resource res;
	int ret=0, size;
	struct udevice *sf2_devp;
	struct sysport_priv *priv = dev_get_priv(dev);
	uint phandle_id;
	const fdt32_t *list;

	priv->buffers=memalign(ARCH_DMA_MINALIGN, NUM_DMA_BUFFERS*NET_DMA_BUFSIZE);
	if (priv->buffers == NULL) {
		printf("%s: malloc failed\n", __func__);
		return -ENOMEM;
	}
	memset(priv->buffers, 0x0, NUM_DMA_BUFFERS*NET_DMA_BUFSIZE);
	flush_cache((unsigned long)(priv->buffers), NUM_DMA_BUFFERS*NET_DMA_BUFSIZE);
	ret = dev_read_resource_byname(dev, "systemport-rbuf-base", &res);
	if (ret) {
		dev_err(dev, "can't get regs systemport-rbuf--base address(ret = %d)!\n", ret);
		return ret;
	}
	priv->sys_port_rbuf=devm_ioremap(dev, res.start, resource_size(&res));
	ret = dev_read_resource_byname(dev, "systemport-rdma-base", &res);
	if (ret) {
		dev_err(dev, "can't get regs systemport-rdma-base address(ret = %d)!\n", ret);
		return ret;
	}
	priv->sys_port_rdma=devm_ioremap(dev, res.start, resource_size(&res));
	ret = dev_read_resource_byname(dev, "systemport-tdma-base", &res);
	if (ret) {
		dev_err(dev, "can't get regs systemport-tdma-base address(ret = %d)!\n", ret);
		return ret;
	}
	priv->sys_port_tdma=devm_ioremap(dev, res.start, resource_size(&res));
	ret = dev_read_resource_byname(dev, "systemport-umac-base", &res);
	if (ret) {
		dev_err(dev, "can't get regs systemport-umac-base address(ret = %d)!\n", ret);
		return ret;
	}
	priv->sys_port_umac=devm_ioremap(dev, res.start, resource_size(&res));
	ret = dev_read_resource_byname(dev, "systemport-topctrl-base", &res);
	if (ret) {
		dev_err(dev, "can't get regs systemport-topctrl-base address(ret = %d)!\n", ret);
		return ret;
	}
	priv->sys_port_topctrl=devm_ioremap(dev, res.start, resource_size(&res));

	// look for the SF2 driver
	priv->sf2_devp = NULL;
	list = dev_read_prop (dev, "ethsw", &size);
	if (size > 0)
	{
		phandle_id = fdt32_to_cpu(*list);
		if (!uclass_get_device_by_phandle_id(UCLASS_NOP, phandle_id, &sf2_devp))
		{
			printf("sf2 found, should have been probed name %s driver name %s\n", 
			sf2_devp->name, sf2_devp->driver->name);
			priv->sf2_devp = sf2_devp;
			priv->sf2_ops = dev_get_priv(sf2_devp);
			priv->sf2_ops->init (priv->sf2_devp);
		}
	}

	sysport_reset(priv);
	return 0;
}

static const struct udevice_id sysport_bcmbca_match_ids[] = {
	{ .compatible = "brcm,bcmbca-systemport-v-1.0"},
	{ }
};


U_BOOT_DRIVER(sysport) = {
	.name	= "sysport",
	.id	= UCLASS_ETH,
	.of_match = sysport_bcmbca_match_ids,
	.flags  = DM_REMOVE_ACTIVE_ALL,
	.probe	= sysport_probe,
	.remove	= sysport_remove,
	.ops	= &sysport_ops,
	.priv_auto_alloc_size = sizeof(struct sysport_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
