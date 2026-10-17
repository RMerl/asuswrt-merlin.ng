// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 *
 * Conversion to DM
 * (C) 2019 Angelo Dureghello <angelo.dureghello@timesys.com>
 */

#include <common.h>
#include <env.h>
#include <hang.h>
#include <malloc.h>
#include <command.h>
#include <net.h>
#include <miiphy.h>
#include <asm/fec.h>
#include <asm/global_data.h>
#include <asm/immap.h>
#include <linux/delay.h>
#include <linux/mii.h>

#undef	ET_DEBUG
#undef	MII_DEBUG

/* Ethernet Transmit and Receive Buffers */
#define DBUF_LENGTH		1520
#define TX_BUF_CNT		2
#define PKT_MAXBUF_SIZE		1518
#define PKT_MAXBLR_SIZE		1520
#define LAST_PKTBUFSRX		PKTBUFSRX - 1
#define BD_ENET_RX_W_E		(BD_ENET_RX_WRAP | BD_ENET_RX_EMPTY)
#define BD_ENET_TX_RDY_LST	(BD_ENET_TX_READY | BD_ENET_TX_LAST)

DECLARE_GLOBAL_DATA_PTR;

static void init_eth_info(struct fec_info_s *info)
{
#ifdef CFG_SYS_FEC_BUF_USE_SRAM
	static u32 tmp;

	if (info->index == 0)
		tmp = CFG_SYS_INIT_RAM_ADDR + 0x1000;
	else
		info->rxbd = (cbd_t *)DBUF_LENGTH;

	/* setup Receive and Transmit buffer descriptor */
	info->rxbd = (cbd_t *)((u32)info->rxbd + tmp);
	tmp = (u32)info->rxbd;
	info->txbd =
	    (cbd_t *)((u32)info->txbd + tmp +
	    (PKTBUFSRX * sizeof(cbd_t)));
	tmp = (u32)info->txbd;
	info->txbuf =
	    (char *)((u32)info->txbuf + tmp +
	    (CFG_SYS_TX_ETH_BUFFER * sizeof(cbd_t)));
	tmp = (u32)info->txbuf;
#else
	info->rxbd =
	    (cbd_t *)memalign(CONFIG_SYS_CACHELINE_SIZE,
			       (PKTBUFSRX * sizeof(cbd_t)));
	info->txbd =
	    (cbd_t *)memalign(CONFIG_SYS_CACHELINE_SIZE,
			       (TX_BUF_CNT * sizeof(cbd_t)));
	info->txbuf =
	    (char *)memalign(CONFIG_SYS_CACHELINE_SIZE, DBUF_LENGTH);
#endif

#ifdef ET_DEBUG
	printf("rxbd %x txbd %x\n", (int)info->rxbd, (int)info->txbd);
#endif
	info->phy_name = (char *)memalign(CONFIG_SYS_CACHELINE_SIZE, 32);
}

static void fec_reset(struct fec_info_s *info)
{
	volatile fec_t *fecp = (fec_t *)(info->iobase);
	int i;

	fecp->ecr = FEC_ECR_RESET;
	for (i = 0; (fecp->ecr & FEC_ECR_RESET) && (i < FEC_RESET_DELAY); ++i)
		udelay(1);

	if (i == FEC_RESET_DELAY)
		printf("FEC_RESET_DELAY timeout\n");
}

static void set_fec_duplex_speed(volatile fec_t *fecp, int dup_spd)
{
	struct bd_info *bd = gd->bd;

	if ((dup_spd >> 16) == FULL) {
		/* Set maximum frame length */
		fecp->rcr = FEC_RCR_MAX_FL(PKT_MAXBUF_SIZE) | FEC_RCR_MII_MODE |
		    FEC_RCR_PROM | 0x100;
		fecp->tcr = FEC_TCR_FDEN;
	} else {
		/* Half duplex mode */
		fecp->rcr = FEC_RCR_MAX_FL(PKT_MAXBUF_SIZE) |
		    FEC_RCR_MII_MODE | FEC_RCR_DRT;
		fecp->tcr &= ~FEC_TCR_FDEN;
	}

	if ((dup_spd & 0xFFFF) == _100BASET) {
#ifdef MII_DEBUG
		printf("100Mbps\n");
#endif
		bd->bi_ethspeed = 100;
	} else {
#ifdef MII_DEBUG
		printf("10Mbps\n");
#endif
		bd->bi_ethspeed = 10;
	}
}

#ifdef ET_DEBUG
static void dbg_fec_regs(struct udevice *dev)
{
	struct fec_info_s *info = dev_get_priv(dev);
	volatile fec_t *fecp = (fec_t *)(info->iobase);

	printf("=====\n");
	printf("ievent       %x - %x\n", (int)&fecp->eir, fecp->eir);
	printf("imask        %x - %x\n", (int)&fecp->eimr, fecp->eimr);
	printf("r_des_active %x - %x\n", (int)&fecp->rdar, fecp->rdar);
	printf("x_des_active %x - %x\n", (int)&fecp->tdar, fecp->tdar);
	printf("ecntrl       %x - %x\n", (int)&fecp->ecr, fecp->ecr);
	printf("mii_mframe   %x - %x\n", (int)&fecp->mmfr, fecp->mmfr);
	printf("mii_speed    %x - %x\n", (int)&fecp->mscr, fecp->mscr);
	printf("mii_ctrlstat %x - %x\n", (int)&fecp->mibc, fecp->mibc);
	printf("r_cntrl      %x - %x\n", (int)&fecp->rcr, fecp->rcr);
	printf("x_cntrl      %x - %x\n", (int)&fecp->tcr, fecp->tcr);
	printf("padr_l       %x - %x\n", (int)&fecp->palr, fecp->palr);
	printf("padr_u       %x - %x\n", (int)&fecp->paur, fecp->paur);
	printf("op_pause     %x - %x\n", (int)&fecp->opd, fecp->opd);
	printf("iadr_u       %x - %x\n", (int)&fecp->iaur, fecp->iaur);
	printf("iadr_l       %x - %x\n", (int)&fecp->ialr, fecp->ialr);
	printf("gadr_u       %x - %x\n", (int)&fecp->gaur, fecp->gaur);
	printf("gadr_l       %x - %x\n", (int)&fecp->galr, fecp->galr);
	printf("x_wmrk       %x - %x\n", (int)&fecp->tfwr, fecp->tfwr);
	printf("r_bound      %x - %x\n", (int)&fecp->frbr, fecp->frbr);
	printf("r_fstart     %x - %x\n", (int)&fecp->frsr, fecp->frsr);
	printf("r_drng       %x - %x\n", (int)&fecp->erdsr, fecp->erdsr);
	printf("x_drng       %x - %x\n", (int)&fecp->etdsr, fecp->etdsr);
	printf("r_bufsz      %x - %x\n", (int)&fecp->emrbr, fecp->emrbr);

	printf("\n");
	printf("rmon_t_drop        %x - %x\n", (int)&fecp->rmon_t_drop,
	       fecp->rmon_t_drop);
	printf("rmon_t_packets     %x - %x\n", (int)&fecp->rmon_t_packets,
	       fecp->rmon_t_packets);
	printf("rmon_t_bc_pkt      %x - %x\n", (int)&fecp->rmon_t_bc_pkt,
	       fecp->rmon_t_bc_pkt);
	printf("rmon_t_mc_pkt      %x - %x\n", (int)&fecp->rmon_t_mc_pkt,
	       fecp->rmon_t_mc_pkt);
	printf("rmon_t_crc_align   %x - %x\n", (int)&fecp->rmon_t_crc_align,
	       fecp->rmon_t_crc_align);
	printf("rmon_t_undersize   %x - %x\n", (int)&fecp->rmon_t_undersize,
	       fecp->rmon_t_undersize);
	printf("rmon_t_oversize    %x - %x\n", (int)&fecp->rmon_t_oversize,
	       fecp->rmon_t_oversize);
	printf("rmon_t_frag        %x - %x\n", (int)&fecp->rmon_t_frag,
	       fecp->rmon_t_frag);
	printf("rmon_t_jab         %x - %x\n", (int)&fecp->rmon_t_jab,
	       fecp->rmon_t_jab);
	printf("rmon_t_col         %x - %x\n", (int)&fecp->rmon_t_col,
	       fecp->rmon_t_col);
	printf("rmon_t_p64         %x - %x\n", (int)&fecp->rmon_t_p64,
	       fecp->rmon_t_p64);
	printf("rmon_t_p65to127    %x - %x\n", (int)&fecp->rmon_t_p65to127,
	       fecp->rmon_t_p65to127);
	printf("rmon_t_p128to255   %x - %x\n", (int)&fecp->rmon_t_p128to255,
	       fecp->rmon_t_p128to255);
	printf("rmon_t_p256to511   %x - %x\n", (int)&fecp->rmon_t_p256to511,
	       fecp->rmon_t_p256to511);
	printf("rmon_t_p512to1023  %x - %x\n", (int)&fecp->rmon_t_p512to1023,
	       fecp->rmon_t_p512to1023);
	printf("rmon_t_p1024to2047 %x - %x\n", (int)&fecp->rmon_t_p1024to2047,
	       fecp->rmon_t_p1024to2047);
	printf("rmon_t_p_gte2048   %x - %x\n", (int)&fecp->rmon_t_p_gte2048,
	       fecp->rmon_t_p_gte2048);
	printf("rmon_t_octets      %x - %x\n", (int)&fecp->rmon_t_octets,
	       fecp->rmon_t_octets);

	printf("\n");
	printf("ieee_t_drop      %x - %x\n", (int)&fecp->ieee_t_drop,
	       fecp->ieee_t_drop);
	printf("ieee_t_frame_ok  %x - %x\n", (int)&fecp->ieee_t_frame_ok,
	       fecp->ieee_t_frame_ok);
	printf("ieee_t_1col      %x - %x\n", (int)&fecp->ieee_t_1col,
	       fecp->ieee_t_1col);
	printf("ieee_t_mcol      %x - %x\n", (int)&fecp->ieee_t_mcol,
	       fecp->ieee_t_mcol);
	printf("ieee_t_def       %x - %x\n", (int)&fecp->ieee_t_def,
	       fecp->ieee_t_def);
	printf("ieee_t_lcol      %x - %x\n", (int)&fecp->ieee_t_lcol,
	       fecp->ieee_t_lcol);
	printf("ieee_t_excol     %x - %x\n", (int)&fecp->ieee_t_excol,
	       fecp->ieee_t_excol);
	printf("ieee_t_macerr    %x - %x\n", (int)&fecp->ieee_t_macerr,
	       fecp->ieee_t_macerr);
	printf("ieee_t_cserr     %x - %x\n", (int)&fecp->ieee_t_cserr,
	       fecp->ieee_t_cserr);
	printf("ieee_t_sqe       %x - %x\n", (int)&fecp->ieee_t_sqe,
	       fecp->ieee_t_sqe);
	printf("ieee_t_fdxfc     %x - %x\n", (int)&fecp->ieee_t_fdxfc,
	       fecp->ieee_t_fdxfc);
	printf("ieee_t_octets_ok %x - %x\n", (int)&fecp->ieee_t_octets_ok,
	       fecp->ieee_t_octets_ok);

	printf("\n");
	printf("rmon_r_drop        %x - %x\n", (int)&fecp->rmon_r_drop,
	       fecp->rmon_r_drop);
	printf("rmon_r_packets     %x - %x\n", (int)&fecp->rmon_r_packets,
	       fecp->rmon_r_packets);
	printf("rmon_r_bc_pkt      %x - %x\n", (int)&fecp->rmon_r_bc_pkt,
	       fecp->rmon_r_bc_pkt);
	printf("rmon_r_mc_pkt      %x - %x\n", (int)&fecp->rmon_r_mc_pkt,
	       fecp->rmon_r_mc_pkt);
	printf("rmon_r_crc_align   %x - %x\n", (int)&fecp->rmon_r_crc_align,
	       fecp->rmon_r_crc_align);
	printf("rmon_r_undersize   %x - %x\n", (int)&fecp->rmon_r_undersize,
	       fecp->rmon_r_undersize);
	printf("rmon_r_oversize    %x - %x\n", (int)&fecp->rmon_r_oversize,
	       fecp->rmon_r_oversize);
	printf("rmon_r_frag        %x - %x\n", (int)&fecp->rmon_r_frag,
	       fecp->rmon_r_frag);
	printf("rmon_r_jab         %x - %x\n", (int)&fecp->rmon_r_jab,
	       fecp->rmon_r_jab);
	printf("rmon_r_p64         %x - %x\n", (int)&fecp->rmon_r_p64,
	       fecp->rmon_r_p64);
	printf("rmon_r_p65to127    %x - %x\n", (int)&fecp->rmon_r_p65to127,
	       fecp->rmon_r_p65to127);
	printf("rmon_r_p128to255   %x - %x\n", (int)&fecp->rmon_r_p128to255,
	       fecp->rmon_r_p128to255);
	printf("rmon_r_p256to511   %x - %x\n", (int)&fecp->rmon_r_p256to511,
	       fecp->rmon_r_p256to511);
	printf("rmon_r_p512to1023  %x - %x\n", (int)&fecp->rmon_r_p512to1023,
	       fecp->rmon_r_p512to1023);
	printf("rmon_r_p1024to2047 %x - %x\n", (int)&fecp->rmon_r_p1024to2047,
	       fecp->rmon_r_p1024to2047);
	printf("rmon_r_p_gte2048   %x - %x\n", (int)&fecp->rmon_r_p_gte2048,
	       fecp->rmon_r_p_gte2048);
	printf("rmon_r_octets      %x - %x\n", (int)&fecp->rmon_r_octets,
	       fecp->rmon_r_octets);

	printf("\n");
	printf("ieee_r_drop      %x - %x\n", (int)&fecp->ieee_r_drop,
	       fecp->ieee_r_drop);
	printf("ieee_r_frame_ok  %x - %x\n", (int)&fecp->ieee_r_frame_ok,
	       fecp->ieee_r_frame_ok);
	printf("ieee_r_crc       %x - %x\n", (int)&fecp->ieee_r_crc,
	       fecp->ieee_r_crc);
	printf("ieee_r_align     %x - %x\n", (int)&fecp->ieee_r_align,
	       fecp->ieee_r_align);
	printf("ieee_r_macerr    %x - %x\n", (int)&fecp->ieee_r_macerr,
	       fecp->ieee_r_macerr);
	printf("ieee_r_fdxfc     %x - %x\n", (int)&fecp->ieee_r_fdxfc,
	       fecp->ieee_r_fdxfc);
	printf("ieee_r_octets_ok %x - %x\n", (int)&fecp->ieee_r_octets_ok,
	       fecp->ieee_r_octets_ok);

	printf("\n\n\n");
}
#endif

int mcffec_init(struct udevice *dev)
{
	struct fec_info_s *info = dev_get_priv(dev);
	volatile fec_t *fecp = (fec_t *) (info->iobase);
	int rval, i;
	uchar ea[6];

	fecpin_setclear(info, 1);
	fec_reset(info);

	mii_init();

	set_fec_duplex_speed(fecp, info->dup_spd);

	/* We use strictly polling mode only */
	fecp->eimr = 0;

	/* Clear any pending interrupt */
	fecp->eir = 0xffffffff;

	/* Set station address   */
	if (info->index == 0)
		rval = eth_env_get_enetaddr("ethaddr", ea);
	else
		rval = eth_env_get_enetaddr("eth1addr", ea);

	if (!rval) {
		puts("Please set a valid MAC address\n");
		return -EINVAL;
	}

	fecp->palr =
	    (ea[0] << 24) | (ea[1] << 16) | (ea[2] << 8) | (ea[3]);
	fecp->paur = (ea[4] << 24) | (ea[5] << 16);

	/* Clear unicast address hash table */
	fecp->iaur = 0;
	fecp->ialr = 0;

	/* Clear multicast address hash table */
	fecp->gaur = 0;
	fecp->galr = 0;

	/* Set maximum receive buffer size. */
	fecp->emrbr = PKT_MAXBLR_SIZE;

	/*
	 * Setup Buffers and Buffer Descriptors
	 */
	info->rx_idx = 0;
	info->tx_idx = 0;

	/*
	 * Setup Receiver Buffer Descriptors (13.14.24.18)
	 * Settings:
	 *     Empty, Wrap
	 */
	for (i = 0; i < PKTBUFSRX; i++) {
		info->rxbd[i].cbd_sc = BD_ENET_RX_EMPTY;
		info->rxbd[i].cbd_datlen = 0;	/* Reset */
		info->rxbd[i].cbd_bufaddr = (uint) net_rx_packets[i];
	}
	info->rxbd[PKTBUFSRX - 1].cbd_sc |= BD_ENET_RX_WRAP;

	/*
	 * Setup Ethernet Transmitter Buffer Descriptors (13.14.24.19)
	 * Settings:
	 *    Last, Tx CRC
	 */
	for (i = 0; i < TX_BUF_CNT; i++) {
		info->txbd[i].cbd_sc = BD_ENET_TX_LAST | BD_ENET_TX_TC;
		info->txbd[i].cbd_datlen = 0;	/* Reset */
		info->txbd[i].cbd_bufaddr = (uint) (&info->txbuf[0]);
	}
	info->txbd[TX_BUF_CNT - 1].cbd_sc |= BD_ENET_TX_WRAP;

	/* Set receive and transmit descriptor base */
	fecp->erdsr = (unsigned int)(&info->rxbd[0]);
	fecp->etdsr = (unsigned int)(&info->txbd[0]);

	/* Now enable the transmit and receive processing */
	fecp->ecr |= FEC_ECR_ETHER_EN;

	/* And last, try to fill Rx Buffer Descriptors
	 * Descriptor polling active
	 */
	fecp->rdar = 0x01000000;

	return 0;
}

static int mcffec_send(struct udevice *dev, void *packet, int length)
{
	struct fec_info_s *info = dev_get_priv(dev);
	volatile fec_t *fecp = (fec_t *)info->iobase;
	int j, rc;
	u16 phy_status;

	miiphy_read(dev->name, info->phy_addr, MII_BMSR, &phy_status);

	/* section 16.9.23.3
	 * Wait for ready
	 */
	j = 0;
	while ((info->txbd[info->tx_idx].cbd_sc & BD_ENET_TX_READY) &&
	       (j < info->to_loop)) {
		udelay(1);
		j++;
	}
	if (j >= info->to_loop)
		printf("TX not ready\n");

	info->txbd[info->tx_idx].cbd_bufaddr = (uint)packet;
	info->txbd[info->tx_idx].cbd_datlen = length;
	info->txbd[info->tx_idx].cbd_sc |= BD_ENET_TX_RDY_LST;

	/* Activate transmit Buffer Descriptor polling */
	fecp->tdar = 0x01000000;	/* Descriptor polling active    */

#ifndef CFG_SYS_FEC_BUF_USE_SRAM
	/*
	 * FEC unable to initial transmit data packet.
	 * A nop will ensure the descriptor polling active completed.
	 * CF Internal RAM has shorter cycle access than DRAM. If use
	 * DRAM as Buffer descriptor and data, a nop is a must.
	 * Affect only V2 and V3.
	 */
	__asm__ ("nop");
#endif

#ifdef CONFIG_SYS_UNIFY_CACHE
	icache_invalid();
#endif

	j = 0;
	while ((info->txbd[info->tx_idx].cbd_sc & BD_ENET_TX_READY) &&
	       (j < info->to_loop)) {
		udelay(1);
		j++;
	}
	if (j >= info->to_loop)
		printf("TX timeout\n");

#ifdef ET_DEBUG
	printf("%s[%d] %s: cycles: %d    status: %x  retry cnt: %d\n",
	       __FILE__, __LINE__, __func__, j,
	       info->txbd[info->tx_idx].cbd_sc,
	       (info->txbd[info->tx_idx].cbd_sc & 0x003C) >> 2);
#endif

	/* return only status bits */
	rc = (info->txbd[info->tx_idx].cbd_sc & BD_ENET_TX_STATS);
	info->tx_idx = (info->tx_idx + 1) % TX_BUF_CNT;

	return rc;
}

static int mcffec_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct fec_info_s *info = dev_get_priv(dev);
	volatile fec_t *fecp = (fec_t *)info->iobase;
	int length = -1;

	for (;;) {
#ifdef CONFIG_SYS_UNIFY_CACHE
		icache_invalid();
#endif
		/* If nothing received - leave for() loop */
		if (info->rxbd[info->rx_idx].cbd_sc & BD_ENET_RX_EMPTY)
			break;

		length = info->rxbd[info->rx_idx].cbd_datlen;

		if (info->rxbd[info->rx_idx].cbd_sc & 0x003f) {
			printf("%s[%d] err: %x\n",
			       __func__, __LINE__,
			       info->rxbd[info->rx_idx].cbd_sc);
		} else {
			length -= 4;

			/*
			 * Pass the buffer ptr up to the protocol layers.
			 */
			*packetp = net_rx_packets[info->rx_idx];

			fecp->eir |= FEC_EIR_RXF;
		}

		/* Give the buffer back to the FEC. */
		info->rxbd[info->rx_idx].cbd_datlen = 0;

		/* wrap around buffer index when necessary */
		if (info->rx_idx == LAST_PKTBUFSRX) {
			info->rxbd[PKTBUFSRX - 1].cbd_sc = BD_ENET_RX_W_E;
			info->rx_idx = 0;
		} else {
			info->rxbd[info->rx_idx].cbd_sc = BD_ENET_RX_EMPTY;
			info->rx_idx++;
		}

		/* Try to fill Buffer Descriptors
		 * Descriptor polling active
		 */
		fecp->rdar = 0x01000000;
	}

	return length;
}

static void mcffec_halt(struct udevice *dev)
{
	struct fec_info_s *info = dev_get_priv(dev);

	fec_reset(info);
	fecpin_setclear(info, 0);

	info->rx_idx = 0;
	info->tx_idx = 0;

	memset(info->rxbd, 0, PKTBUFSRX * sizeof(cbd_t));
	memset(info->txbd, 0, TX_BUF_CNT * sizeof(cbd_t));
	memset(info->txbuf, 0, DBUF_LENGTH);
}

static const struct eth_ops mcffec_ops = {
	.start	= mcffec_init,
	.send	= mcffec_send,
	.recv	= mcffec_recv,
	.stop	= mcffec_halt,
};

/*
 * Boot sequence, called just after mcffec_of_to_plat,
 * as DM way, it replaces old mcffec_initialize.
 */
static int mcffec_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct fec_info_s *info = dev_get_priv(dev);
	int node = dev_of_offset(dev);
	int retval, fec_idx;
	const u32 *val;

	info->index = dev_seq(dev);
	info->iobase = pdata->iobase;
	info->phy_addr = -1;

	val = fdt_getprop(gd->fdt_blob, node, "mii-base", NULL);
	if (val) {
		u32 fec_iobase;

		fec_idx = fdt32_to_cpu(*val);
		if (fec_idx == info->index) {
			fec_iobase = info->iobase;
		} else {
			printf("mii base != base address, fec_idx %d\n",
			       fec_idx);
			retval = fec_get_base_addr(fec_idx, &fec_iobase);
			if (retval)
				return retval;
		}
		info->miibase = fec_iobase;
	}

	val = fdt_getprop(gd->fdt_blob, node, "phy-addr", NULL);
	if (val)
		info->phy_addr = fdt32_to_cpu(*val);

	val = fdt_getprop(gd->fdt_blob, node, "timeout-loop", NULL);
	if (val)
		info->to_loop = fdt32_to_cpu(*val);

	init_eth_info(info);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	info->bus = mdio_alloc();
	if (!info->bus)
		return -ENOMEM;
	strcpy(info->bus->name, dev->name);
	info->bus->read = mcffec_miiphy_read;
	info->bus->write = mcffec_miiphy_write;

	retval = mdio_register(info->bus);
	if (retval < 0)
		return retval;
#endif

	return 0;
}

static int mcffec_remove(struct udevice *dev)
{
	struct fec_info_s *priv = dev_get_priv(dev);

	mdio_unregister(priv->bus);
	mdio_free(priv->bus);

	return 0;
}

/*
 * Boot sequence, called 1st
 */
static int mcffec_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	const u32 *val;

	pdata->iobase = dev_read_addr(dev);
	/* Default to 10Mbit/s */
	pdata->max_speed = 10;

	val = fdt_getprop(gd->fdt_blob, dev_of_offset(dev),
			  "max-speed", NULL);
	if (val)
		pdata->max_speed = fdt32_to_cpu(*val);

	return 0;
}

static const struct udevice_id mcffec_ids[] = {
	{ .compatible = "fsl,mcf-fec" },
	{ }
};

U_BOOT_DRIVER(mcffec) = {
	.name	= "mcffec",
	.id	= UCLASS_ETH,
	.of_match = mcffec_ids,
	.of_to_plat = mcffec_of_to_plat,
	.probe	= mcffec_probe,
	.remove	= mcffec_remove,
	.ops	= &mcffec_ops,
	.priv_auto	= sizeof(struct fec_info_s),
	.plat_auto	= sizeof(struct eth_pdata),
};
