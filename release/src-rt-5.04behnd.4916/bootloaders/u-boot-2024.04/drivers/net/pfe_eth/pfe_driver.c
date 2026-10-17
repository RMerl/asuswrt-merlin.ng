// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#include <log.h>
#include <malloc.h>
#include <linux/delay.h>
#include <net/pfe_eth/pfe_eth.h>
#include <net/pfe_eth/pfe_firmware.h>

static struct tx_desc_s *g_tx_desc;
static struct rx_desc_s *g_rx_desc;

/*
 * HIF Rx interface function
 * Reads the rx descriptor from the current location (rx_to_read).
 * - If the descriptor has a valid data/pkt, then get the data pointer
 * - check for the input rx phy number
 * - increment the rx data pointer by pkt_head_room_size
 * - decrement the data length by pkt_head_room_size
 * - handover the packet to caller.
 *
 * @param[out] pkt_ptr - Pointer to store rx packet
 * @param[out] phy_port - Pointer to store recv phy port
 *
 * Return: -1 if no packet, else return length of packet.
 */
int pfe_recv(uchar **pkt_ptr, int *phy_port)
{
	struct rx_desc_s *rx_desc = g_rx_desc;
	struct buf_desc *bd;
	int len = 0;

	struct hif_header_s *hif_header;

	bd = rx_desc->rx_base + rx_desc->rx_to_read;

	if (readl(&bd->ctrl) & BD_CTRL_DESC_EN)
		return len; /* No pending Rx packet */

	/* this len include hif_header(8 bytes) */
	len = readl(&bd->ctrl) & 0xFFFF;

	hif_header = (struct hif_header_s *)DDR_PFE_TO_VIRT(readl(&bd->data));

	/* Get the receive port info from the packet */
	debug("Pkt received:");
	debug(" Pkt ptr(%p), len(%d), gemac_port(%d) status(%08x)\n",
	      hif_header, len, hif_header->port_no, readl(&bd->status));
#ifdef DEBUG
	{
		int i;
		unsigned char *p = (unsigned char *)hif_header;

		for (i = 0; i < len; i++) {
			if (!(i % 16))
				printf("\n");
			printf(" %02x", p[i]);
		}
		printf("\n");
	}
#endif

	*pkt_ptr = (uchar *)(hif_header + 1);
	*phy_port = hif_header->port_no;
	len -= sizeof(struct hif_header_s);

	return len;
}

/*
 * HIF function to check the Rx done
 * This function will check the rx done indication of the current rx_to_read
 * locations
 * if success, moves the rx_to_read to next location.
 */
int pfe_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct rx_desc_s *rx_desc = g_rx_desc;
	struct buf_desc *bd;

	debug("%s:rx_base: %p, rx_to_read: %d\n", __func__, rx_desc->rx_base,
	      rx_desc->rx_to_read);

	bd = rx_desc->rx_base + rx_desc->rx_to_read;

	/* reset the control field */
	writel((MAX_FRAME_SIZE | BD_CTRL_LIFM | BD_CTRL_DESC_EN
		    | BD_CTRL_DIR), &bd->ctrl);
	writel(0, &bd->status);

	debug("Rx Done : status: %08x, ctrl: %08x\n", readl(&bd->status),
	      readl(&bd->ctrl));

	/* Give START_STROBE to BDP to fetch the descriptor __NOW__,
	 * BDP need not wait for rx_poll_cycle time to fetch the descriptor,
	 * In idle state (ie., no rx pkt), BDP will not fetch
	 * the descriptor even if strobe is given.
	 */
	writel((readl(HIF_RX_CTRL) | HIF_CTRL_BDP_CH_START_WSTB), HIF_RX_CTRL);

	/* increment the rx_to_read index to next location */
	rx_desc->rx_to_read = (rx_desc->rx_to_read + 1)
			       & (rx_desc->rx_ring_size - 1);

	debug("Rx next pkt location: %d\n", rx_desc->rx_to_read);

	return 0;
}

/*
 * HIF Tx interface function
 * This function sends a single packet to PFE from HIF interface.
 * - No interrupt indication on tx completion.
 * - Data is copied to tx buffers before tx descriptor is updated
 *   and TX DMA is enabled.
 *
 * @param[in] phy_port	Phy port number to send out this packet
 * @param[in] data	Pointer to the data
 * @param[in] length	Length of the ethernet packet to be transferred.
 *
 * Return: -1 if tx Q is full, else returns the tx location where the pkt is
 * placed.
 */
int pfe_send(int phy_port, void *data, int length)
{
	struct tx_desc_s *tx_desc = g_tx_desc;
	struct buf_desc *bd;
	struct hif_header_s hif_header;
	u8 *tx_buf_va;

	debug("%s:pkt: %p, len: %d, tx_base: %p, tx_to_send: %d\n", __func__,
	      data, length, tx_desc->tx_base, tx_desc->tx_to_send);

	bd = tx_desc->tx_base + tx_desc->tx_to_send;

	/* check queue-full condition */
	if (readl(&bd->ctrl) & BD_CTRL_DESC_EN)
		return -1;

	/* PFE checks for min pkt size */
	if (length < MIN_PKT_SIZE)
		length = MIN_PKT_SIZE;

	tx_buf_va = (void *)DDR_PFE_TO_VIRT(readl(&bd->data));
	debug("%s: tx_buf_va: %p, tx_buf_pa: %08x\n", __func__, tx_buf_va,
	      readl(&bd->data));

	/* Fill the gemac/phy port number to send this packet out */
	memset(&hif_header, 0, sizeof(struct hif_header_s));
	hif_header.port_no = phy_port;

	memcpy(tx_buf_va, (u8 *)&hif_header, sizeof(struct hif_header_s));
	memcpy(tx_buf_va + sizeof(struct hif_header_s), data, length);
	length += sizeof(struct hif_header_s);

#ifdef DEBUG
	{
		int i;
		unsigned char *p = (unsigned char *)tx_buf_va;

		for (i = 0; i < length; i++) {
			if (!(i % 16))
				printf("\n");
			printf("%02x ", p[i]);
		}
	}
#endif

	debug("Tx Done: status: %08x, ctrl: %08x\n", readl(&bd->status),
	      readl(&bd->ctrl));

	/* fill the tx desc */
	writel((u32)(BD_CTRL_DESC_EN | BD_CTRL_LIFM | (length & 0xFFFF)),
	       &bd->ctrl);
	writel(0, &bd->status);

	writel((HIF_CTRL_DMA_EN | HIF_CTRL_BDP_CH_START_WSTB), HIF_TX_CTRL);

	udelay(100);

	return tx_desc->tx_to_send;
}

/*
 * HIF function to check the Tx done
 *  This function will check the tx done indication of the current tx_to_send
 *  locations
 *  if success, moves the tx_to_send to next location.
 *
 * Return: -1 if TX ownership bit is not cleared by hw.
 * else on success (tx done completion) return zero.
 */
int pfe_tx_done(void)
{
	struct tx_desc_s *tx_desc = g_tx_desc;
	struct buf_desc *bd;

	debug("%s:tx_base: %p, tx_to_send: %d\n", __func__, tx_desc->tx_base,
	      tx_desc->tx_to_send);

	bd = tx_desc->tx_base + tx_desc->tx_to_send;

	/* check queue-full condition */
	if (readl(&bd->ctrl) & BD_CTRL_DESC_EN)
		return -1;

	/* reset the control field */
	writel(0, &bd->ctrl);
	writel(0, &bd->status);

	debug("Tx Done : status: %08x, ctrl: %08x\n", readl(&bd->status),
	      readl(&bd->ctrl));

	/* increment the txtosend index to next location */
	tx_desc->tx_to_send = (tx_desc->tx_to_send + 1)
			       & (tx_desc->tx_ring_size - 1);

	debug("Tx next pkt location: %d\n", tx_desc->tx_to_send);

	return 0;
}

/*
 * Helper function to dump Rx descriptors.
 */
static inline void hif_rx_desc_dump(void)
{
	struct buf_desc *bd_va;
	int i;
	struct rx_desc_s *rx_desc;

	if (!g_rx_desc) {
		printf("%s: HIF Rx desc no init\n", __func__);
		return;
	}

	rx_desc = g_rx_desc;
	bd_va = rx_desc->rx_base;

	debug("HIF rx desc: base_va: %p, base_pa: %08x\n", rx_desc->rx_base,
	      rx_desc->rx_base_pa);
	for (i = 0; i < rx_desc->rx_ring_size; i++) {
		debug("status: %08x, ctrl: %08x, data: %08x, next: 0x%08x\n",
		      readl(&bd_va->status),
		      readl(&bd_va->ctrl),
		      readl(&bd_va->data),
		      readl(&bd_va->next));
		bd_va++;
	}
}

/*
 * This function mark all Rx descriptors as LAST_BD.
 */
void hif_rx_desc_disable(void)
{
	int i;
	struct rx_desc_s *rx_desc;
	struct buf_desc *bd_va;

	if (!g_rx_desc) {
		printf("%s: HIF Rx desc not initialized\n", __func__);
		return;
	}

	rx_desc = g_rx_desc;
	bd_va = rx_desc->rx_base;

	for (i = 0; i < rx_desc->rx_ring_size; i++) {
		writel(readl(&bd_va->ctrl) | BD_CTRL_LAST_BD, &bd_va->ctrl);
		bd_va++;
	}
}

/*
 * HIF Rx Desc initialization function.
 */
static int hif_rx_desc_init(struct pfe_ddr_address *pfe_addr)
{
	u32 ctrl;
	struct buf_desc *bd_va;
	struct buf_desc *bd_pa;
	struct rx_desc_s *rx_desc;
	u32 rx_buf_pa;
	int i;

	/* sanity check */
	if (g_rx_desc) {
		printf("%s: HIF Rx desc re-init request\n", __func__);
		return 0;
	}

	rx_desc = (struct rx_desc_s *)malloc(sizeof(struct rx_desc_s));
	if (!rx_desc) {
		printf("%s: Memory allocation failure\n", __func__);
		return -ENOMEM;
	}
	memset(rx_desc, 0, sizeof(struct rx_desc_s));

	/* init: Rx ring buffer */
	rx_desc->rx_ring_size = HIF_RX_DESC_NT;

	/* NOTE: must be 64bit aligned  */
	bd_va = (struct buf_desc *)(pfe_addr->ddr_pfe_baseaddr
		 + RX_BD_BASEADDR);
	bd_pa = (struct buf_desc *)(pfe_addr->ddr_pfe_phys_baseaddr
				    + RX_BD_BASEADDR);

	rx_desc->rx_base = bd_va;
	rx_desc->rx_base_pa = (unsigned long)bd_pa;

	rx_buf_pa = pfe_addr->ddr_pfe_phys_baseaddr + HIF_RX_PKT_DDR_BASEADDR;

	debug("%s: Rx desc base: %p, base_pa: %08x, desc_count: %d\n",
	      __func__, rx_desc->rx_base, rx_desc->rx_base_pa,
	      rx_desc->rx_ring_size);

	memset(bd_va, 0, sizeof(struct buf_desc) * rx_desc->rx_ring_size);

	ctrl = (MAX_FRAME_SIZE | BD_CTRL_DESC_EN | BD_CTRL_DIR | BD_CTRL_LIFM);

	for (i = 0; i < rx_desc->rx_ring_size; i++) {
		writel((unsigned long)(bd_pa + 1), &bd_va->next);
		writel(ctrl, &bd_va->ctrl);
		writel(rx_buf_pa + (i * MAX_FRAME_SIZE), &bd_va->data);
		bd_va++;
		bd_pa++;
	}
	--bd_va;
	writel((u32)rx_desc->rx_base_pa, &bd_va->next);

	writel(rx_desc->rx_base_pa, HIF_RX_BDP_ADDR);
	writel((readl(HIF_RX_CTRL) | HIF_CTRL_BDP_CH_START_WSTB), HIF_RX_CTRL);

	g_rx_desc = rx_desc;

	return 0;
}

/*
 * Helper function to dump Tx Descriptors.
 */
static inline void hif_tx_desc_dump(void)
{
	struct tx_desc_s *tx_desc;
	int i;
	struct buf_desc *bd_va;

	if (!g_tx_desc) {
		printf("%s: HIF Tx desc no init\n", __func__);
		return;
	}

	tx_desc = g_tx_desc;
	bd_va = tx_desc->tx_base;

	debug("HIF tx desc: base_va: %p, base_pa: %08x\n", tx_desc->tx_base,
	      tx_desc->tx_base_pa);

	for (i = 0; i < tx_desc->tx_ring_size; i++)
		bd_va++;
}

/*
 * HIF Tx descriptor initialization function.
 */
static int hif_tx_desc_init(struct pfe_ddr_address *pfe_addr)
{
	struct buf_desc *bd_va;
	struct buf_desc *bd_pa;
	int i;
	struct tx_desc_s *tx_desc;
	u32 tx_buf_pa;

	/* sanity check */
	if (g_tx_desc) {
		printf("%s: HIF Tx desc re-init request\n", __func__);
		return 0;
	}

	tx_desc = (struct tx_desc_s *)malloc(sizeof(struct tx_desc_s));
	if (!tx_desc) {
		printf("%s:%d:Memory allocation failure\n", __func__,
		       __LINE__);
		return -ENOMEM;
	}
	memset(tx_desc, 0, sizeof(struct tx_desc_s));

	/* init: Tx ring buffer */
	tx_desc->tx_ring_size = HIF_TX_DESC_NT;

	/* NOTE: must be 64bit aligned  */
	bd_va = (struct buf_desc *)(pfe_addr->ddr_pfe_baseaddr
		 + TX_BD_BASEADDR);
	bd_pa = (struct buf_desc *)(pfe_addr->ddr_pfe_phys_baseaddr
				    + TX_BD_BASEADDR);

	tx_desc->tx_base_pa = (unsigned long)bd_pa;
	tx_desc->tx_base = bd_va;

	debug("%s: Tx desc_base: %p, base_pa: %08x, desc_count: %d\n",
	      __func__, tx_desc->tx_base, tx_desc->tx_base_pa,
	      tx_desc->tx_ring_size);

	memset(bd_va, 0, sizeof(struct buf_desc) * tx_desc->tx_ring_size);

	tx_buf_pa = pfe_addr->ddr_pfe_phys_baseaddr + HIF_TX_PKT_DDR_BASEADDR;

	for (i = 0; i < tx_desc->tx_ring_size; i++) {
		writel((unsigned long)(bd_pa + 1), &bd_va->next);
		writel(tx_buf_pa + (i * MAX_FRAME_SIZE), &bd_va->data);
		bd_va++;
		bd_pa++;
	}
	--bd_va;
	writel((u32)tx_desc->tx_base_pa, &bd_va->next);

	writel(tx_desc->tx_base_pa, HIF_TX_BDP_ADDR);

	g_tx_desc = tx_desc;

	return 0;
}

/*
 * PFE/Class initialization.
 */
static void pfe_class_init(struct pfe_ddr_address *pfe_addr)
{
	struct class_cfg class_cfg = {
		.route_table_baseaddr = pfe_addr->ddr_pfe_phys_baseaddr +
					ROUTE_TABLE_BASEADDR,
		.route_table_hash_bits = ROUTE_TABLE_HASH_BITS,
	};

	class_init(&class_cfg);

	debug("class init complete\n");
}

/*
 * PFE/TMU initialization.
 */
static void pfe_tmu_init(struct pfe_ddr_address *pfe_addr)
{
	struct tmu_cfg tmu_cfg = {
		.llm_base_addr = pfe_addr->ddr_pfe_phys_baseaddr
				 + TMU_LLM_BASEADDR,
		.llm_queue_len = TMU_LLM_QUEUE_LEN,
	};

	tmu_init(&tmu_cfg);

	debug("tmu init complete\n");
}

/*
 * PFE/BMU (both BMU1 & BMU2) initialization.
 */
static void pfe_bmu_init(struct pfe_ddr_address *pfe_addr)
{
	struct bmu_cfg bmu1_cfg = {
		.baseaddr = CBUS_VIRT_TO_PFE(LMEM_BASE_ADDR +
						BMU1_LMEM_BASEADDR),
		.count = BMU1_BUF_COUNT,
		.size = BMU1_BUF_SIZE,
	};

	struct bmu_cfg bmu2_cfg = {
		.baseaddr = pfe_addr->ddr_pfe_phys_baseaddr + BMU2_DDR_BASEADDR,
		.count = BMU2_BUF_COUNT,
		.size = BMU2_BUF_SIZE,
	};

	bmu_init(BMU1_BASE_ADDR, &bmu1_cfg);
	debug("bmu1 init: done\n");

	bmu_init(BMU2_BASE_ADDR, &bmu2_cfg);
	debug("bmu2 init: done\n");
}

/*
 * PFE/GPI initialization function.
 *  - egpi1, egpi2, egpi3, hgpi
 */
static void pfe_gpi_init(struct pfe_ddr_address *pfe_addr)
{
	struct gpi_cfg egpi1_cfg = {
		.lmem_rtry_cnt = EGPI1_LMEM_RTRY_CNT,
		.tmlf_txthres = EGPI1_TMLF_TXTHRES,
		.aseq_len = EGPI1_ASEQ_LEN,
	};

	struct gpi_cfg egpi2_cfg = {
		.lmem_rtry_cnt = EGPI2_LMEM_RTRY_CNT,
		.tmlf_txthres = EGPI2_TMLF_TXTHRES,
		.aseq_len = EGPI2_ASEQ_LEN,
	};

	struct gpi_cfg hgpi_cfg = {
		.lmem_rtry_cnt = HGPI_LMEM_RTRY_CNT,
		.tmlf_txthres = HGPI_TMLF_TXTHRES,
		.aseq_len = HGPI_ASEQ_LEN,
	};

	gpi_init(EGPI1_BASE_ADDR, &egpi1_cfg);
	debug("GPI1 init complete\n");

	gpi_init(EGPI2_BASE_ADDR, &egpi2_cfg);
	debug("GPI2 init complete\n");

	gpi_init(HGPI_BASE_ADDR, &hgpi_cfg);
	debug("HGPI init complete\n");
}

/*
 * PFE/HIF initialization function.
 */
static int pfe_hif_init(struct pfe_ddr_address *pfe_addr)
{
	int ret = 0;

	hif_tx_disable();
	hif_rx_disable();

	ret = hif_tx_desc_init(pfe_addr);
	if (ret)
		return ret;
	ret = hif_rx_desc_init(pfe_addr);
	if (ret)
		return ret;

	hif_init();

	hif_tx_enable();
	hif_rx_enable();

	hif_rx_desc_dump();
	hif_tx_desc_dump();

	debug("HIF init complete\n");
	return ret;
}

/*
 * PFE initialization
 * - Firmware loading (CLASS-PE and TMU-PE)
 * - BMU1 and BMU2 init
 * - GEMAC init
 * - GPI init
 * - CLASS-PE init
 * - TMU-PE init
 * - HIF tx and rx descriptors init
 *
 * @param[in]	edev	Pointer to eth device structure.
 *
 * Return: 0, on success.
 */
static int pfe_hw_init(struct pfe_ddr_address *pfe_addr)
{
	int ret = 0;

	debug("%s: start\n", __func__);

	writel(0x3, CLASS_PE_SYS_CLK_RATIO);
	writel(0x3, TMU_PE_SYS_CLK_RATIO);
	writel(0x3, UTIL_PE_SYS_CLK_RATIO);
	udelay(10);

	pfe_class_init(pfe_addr);

	pfe_tmu_init(pfe_addr);

	pfe_bmu_init(pfe_addr);

	pfe_gpi_init(pfe_addr);

	ret = pfe_hif_init(pfe_addr);
	if (ret)
		return ret;

	bmu_enable(BMU1_BASE_ADDR);
	debug("bmu1 enabled\n");

	bmu_enable(BMU2_BASE_ADDR);
	debug("bmu2 enabled\n");

	debug("%s: done\n", __func__);

	return ret;
}

/*
 * PFE driver init function.
 * - Initializes pfe_lib
 * - pfe hw init
 * - fw loading and enables PEs
 * - should be executed once.
 *
 * @param[in] pfe  Pointer the pfe control block
 */
int pfe_drv_init(struct pfe_ddr_address  *pfe_addr)
{
	int ret = 0;

	pfe_lib_init();

	ret = pfe_hw_init(pfe_addr);
	if (ret)
		return ret;

	/* Load the class,TM, Util fw.
	 * By now pfe is:
	 * - out of reset + disabled + configured.
	 * Fw loading should be done after pfe_hw_init()
	 */
	/* It loads default inbuilt sbl firmware */
	pfe_firmware_init();

	return ret;
}

/*
 * PFE remove function
 *  - stops PEs
 *  - frees tx/rx descriptor resources
 *  - should be called once.
 *
 * @param[in] pfe Pointer to pfe control block.
 */
int pfe_eth_remove(struct udevice *dev)
{
	if (g_tx_desc)
		free(g_tx_desc);

	if (g_rx_desc)
		free(g_rx_desc);

	pfe_firmware_exit();

	return 0;
}
