// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2006-2011 Freescale Semiconductor, Inc.
 *
 * Dave Liu <daveliu@freescale.com>
 */

#include <common.h>
#include <net.h>
#include <malloc.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <linux/immap_qe.h>
#include "uccf.h"
#include "uec.h"
#include "uec_phy.h"
#include "miiphy.h"
#include <fsl_qe.h>
#include <phy.h>

/* Default UTBIPAR SMI address */
#ifndef CONFIG_UTBIPAR_INIT_TBIPA
#define CONFIG_UTBIPAR_INIT_TBIPA 0x1F
#endif

static uec_info_t uec_info[] = {
#ifdef CONFIG_UEC_ETH1
	STD_UEC_INFO(1),	/* UEC1 */
#endif
#ifdef CONFIG_UEC_ETH2
	STD_UEC_INFO(2),	/* UEC2 */
#endif
#ifdef CONFIG_UEC_ETH3
	STD_UEC_INFO(3),	/* UEC3 */
#endif
#ifdef CONFIG_UEC_ETH4
	STD_UEC_INFO(4),	/* UEC4 */
#endif
#ifdef CONFIG_UEC_ETH5
	STD_UEC_INFO(5),	/* UEC5 */
#endif
#ifdef CONFIG_UEC_ETH6
	STD_UEC_INFO(6),	/* UEC6 */
#endif
#ifdef CONFIG_UEC_ETH7
	STD_UEC_INFO(7),	/* UEC7 */
#endif
#ifdef CONFIG_UEC_ETH8
	STD_UEC_INFO(8),	/* UEC8 */
#endif
};

#define MAXCONTROLLERS	(8)

static struct eth_device *devlist[MAXCONTROLLERS];

static int uec_mac_enable(uec_private_t *uec, comm_dir_e mode)
{
	uec_t		*uec_regs;
	u32		maccfg1;

	if (!uec) {
		printf("%s: uec not initial\n", __FUNCTION__);
		return -EINVAL;
	}
	uec_regs = uec->uec_regs;

	maccfg1 = in_be32(&uec_regs->maccfg1);

	if (mode & COMM_DIR_TX)	{
		maccfg1 |= MACCFG1_ENABLE_TX;
		out_be32(&uec_regs->maccfg1, maccfg1);
		uec->mac_tx_enabled = 1;
	}

	if (mode & COMM_DIR_RX)	{
		maccfg1 |= MACCFG1_ENABLE_RX;
		out_be32(&uec_regs->maccfg1, maccfg1);
		uec->mac_rx_enabled = 1;
	}

	return 0;
}

static int uec_mac_disable(uec_private_t *uec, comm_dir_e mode)
{
	uec_t		*uec_regs;
	u32		maccfg1;

	if (!uec) {
		printf("%s: uec not initial\n", __FUNCTION__);
		return -EINVAL;
	}
	uec_regs = uec->uec_regs;

	maccfg1 = in_be32(&uec_regs->maccfg1);

	if (mode & COMM_DIR_TX)	{
		maccfg1 &= ~MACCFG1_ENABLE_TX;
		out_be32(&uec_regs->maccfg1, maccfg1);
		uec->mac_tx_enabled = 0;
	}

	if (mode & COMM_DIR_RX)	{
		maccfg1 &= ~MACCFG1_ENABLE_RX;
		out_be32(&uec_regs->maccfg1, maccfg1);
		uec->mac_rx_enabled = 0;
	}

	return 0;
}

static int uec_graceful_stop_tx(uec_private_t *uec)
{
	ucc_fast_t		*uf_regs;
	u32			cecr_subblock;
	u32			ucce;

	if (!uec || !uec->uccf) {
		printf("%s: No handle passed.\n", __FUNCTION__);
		return -EINVAL;
	}

	uf_regs = uec->uccf->uf_regs;

	/* Clear the grace stop event */
	out_be32(&uf_regs->ucce, UCCE_GRA);

	/* Issue host command */
	cecr_subblock =
		 ucc_fast_get_qe_cr_subblock(uec->uec_info->uf_info.ucc_num);
	qe_issue_cmd(QE_GRACEFUL_STOP_TX, cecr_subblock,
			 (u8)QE_CR_PROTOCOL_ETHERNET, 0);

	/* Wait for command to complete */
	do {
		ucce = in_be32(&uf_regs->ucce);
	} while (! (ucce & UCCE_GRA));

	uec->grace_stopped_tx = 1;

	return 0;
}

static int uec_graceful_stop_rx(uec_private_t *uec)
{
	u32		cecr_subblock;
	u8		ack;

	if (!uec) {
		printf("%s: No handle passed.\n", __FUNCTION__);
		return -EINVAL;
	}

	if (!uec->p_rx_glbl_pram) {
		printf("%s: No init rx global parameter\n", __FUNCTION__);
		return -EINVAL;
	}

	/* Clear acknowledge bit */
	ack = uec->p_rx_glbl_pram->rxgstpack;
	ack &= ~GRACEFUL_STOP_ACKNOWLEDGE_RX;
	uec->p_rx_glbl_pram->rxgstpack = ack;

	/* Keep issuing cmd and checking ack bit until it is asserted */
	do {
		/* Issue host command */
		cecr_subblock =
		 ucc_fast_get_qe_cr_subblock(uec->uec_info->uf_info.ucc_num);
		qe_issue_cmd(QE_GRACEFUL_STOP_RX, cecr_subblock,
				 (u8)QE_CR_PROTOCOL_ETHERNET, 0);
		ack = uec->p_rx_glbl_pram->rxgstpack;
	} while (! (ack & GRACEFUL_STOP_ACKNOWLEDGE_RX ));

	uec->grace_stopped_rx = 1;

	return 0;
}

static int uec_restart_tx(uec_private_t *uec)
{
	u32		cecr_subblock;

	if (!uec || !uec->uec_info) {
		printf("%s: No handle passed.\n", __FUNCTION__);
		return -EINVAL;
	}

	cecr_subblock =
	 ucc_fast_get_qe_cr_subblock(uec->uec_info->uf_info.ucc_num);
	qe_issue_cmd(QE_RESTART_TX, cecr_subblock,
			 (u8)QE_CR_PROTOCOL_ETHERNET, 0);

	uec->grace_stopped_tx = 0;

	return 0;
}

static int uec_restart_rx(uec_private_t *uec)
{
	u32		cecr_subblock;

	if (!uec || !uec->uec_info) {
		printf("%s: No handle passed.\n", __FUNCTION__);
		return -EINVAL;
	}

	cecr_subblock =
	 ucc_fast_get_qe_cr_subblock(uec->uec_info->uf_info.ucc_num);
	qe_issue_cmd(QE_RESTART_RX, cecr_subblock,
			 (u8)QE_CR_PROTOCOL_ETHERNET, 0);

	uec->grace_stopped_rx = 0;

	return 0;
}

static int uec_open(uec_private_t *uec, comm_dir_e mode)
{
	ucc_fast_private_t	*uccf;

	if (!uec || !uec->uccf) {
		printf("%s: No handle passed.\n", __FUNCTION__);
		return -EINVAL;
	}
	uccf = uec->uccf;

	/* check if the UCC number is in range. */
	if (uec->uec_info->uf_info.ucc_num >= UCC_MAX_NUM) {
		printf("%s: ucc_num out of range.\n", __FUNCTION__);
		return -EINVAL;
	}

	/* Enable MAC */
	uec_mac_enable(uec, mode);

	/* Enable UCC fast */
	ucc_fast_enable(uccf, mode);

	/* RISC microcode start */
	if ((mode & COMM_DIR_TX) && uec->grace_stopped_tx) {
		uec_restart_tx(uec);
	}
	if ((mode & COMM_DIR_RX) && uec->grace_stopped_rx) {
		uec_restart_rx(uec);
	}

	return 0;
}

static int uec_stop(uec_private_t *uec, comm_dir_e mode)
{
	if (!uec || !uec->uccf) {
		printf("%s: No handle passed.\n", __FUNCTION__);
		return -EINVAL;
	}

	/* check if the UCC number is in range. */
	if (uec->uec_info->uf_info.ucc_num >= UCC_MAX_NUM) {
		printf("%s: ucc_num out of range.\n", __FUNCTION__);
		return -EINVAL;
	}
	/* Stop any transmissions */
	if ((mode & COMM_DIR_TX) && !uec->grace_stopped_tx) {
		uec_graceful_stop_tx(uec);
	}
	/* Stop any receptions */
	if ((mode & COMM_DIR_RX) && !uec->grace_stopped_rx) {
		uec_graceful_stop_rx(uec);
	}

	/* Disable the UCC fast */
	ucc_fast_disable(uec->uccf, mode);

	/* Disable the MAC */
	uec_mac_disable(uec, mode);

	return 0;
}

static int uec_set_mac_duplex(uec_private_t *uec, int duplex)
{
	uec_t		*uec_regs;
	u32		maccfg2;

	if (!uec) {
		printf("%s: uec not initial\n", __FUNCTION__);
		return -EINVAL;
	}
	uec_regs = uec->uec_regs;

	if (duplex == DUPLEX_HALF) {
		maccfg2 = in_be32(&uec_regs->maccfg2);
		maccfg2 &= ~MACCFG2_FDX;
		out_be32(&uec_regs->maccfg2, maccfg2);
	}

	if (duplex == DUPLEX_FULL) {
		maccfg2 = in_be32(&uec_regs->maccfg2);
		maccfg2 |= MACCFG2_FDX;
		out_be32(&uec_regs->maccfg2, maccfg2);
	}

	return 0;
}

static int uec_set_mac_if_mode(uec_private_t *uec,
		phy_interface_t if_mode, int speed)
{
	phy_interface_t		enet_if_mode;
	uec_t			*uec_regs;
	u32			upsmr;
	u32			maccfg2;

	if (!uec) {
		printf("%s: uec not initial\n", __FUNCTION__);
		return -EINVAL;
	}

	uec_regs = uec->uec_regs;
	enet_if_mode = if_mode;

	maccfg2 = in_be32(&uec_regs->maccfg2);
	maccfg2 &= ~MACCFG2_INTERFACE_MODE_MASK;

	upsmr = in_be32(&uec->uccf->uf_regs->upsmr);
	upsmr &= ~(UPSMR_RPM | UPSMR_TBIM | UPSMR_R10M | UPSMR_RMM);

	switch (speed) {
		case SPEED_10:
			maccfg2 |= MACCFG2_INTERFACE_MODE_NIBBLE;
			switch (enet_if_mode) {
				case PHY_INTERFACE_MODE_MII:
					break;
				case PHY_INTERFACE_MODE_RGMII:
					upsmr |= (UPSMR_RPM | UPSMR_R10M);
					break;
				case PHY_INTERFACE_MODE_RMII:
					upsmr |= (UPSMR_R10M | UPSMR_RMM);
					break;
				default:
					return -EINVAL;
					break;
			}
			break;
		case SPEED_100:
			maccfg2 |= MACCFG2_INTERFACE_MODE_NIBBLE;
			switch (enet_if_mode) {
				case PHY_INTERFACE_MODE_MII:
					break;
				case PHY_INTERFACE_MODE_RGMII:
					upsmr |= UPSMR_RPM;
					break;
				case PHY_INTERFACE_MODE_RMII:
					upsmr |= UPSMR_RMM;
					break;
				default:
					return -EINVAL;
					break;
			}
			break;
		case SPEED_1000:
			maccfg2 |= MACCFG2_INTERFACE_MODE_BYTE;
			switch (enet_if_mode) {
				case PHY_INTERFACE_MODE_GMII:
					break;
				case PHY_INTERFACE_MODE_TBI:
					upsmr |= UPSMR_TBIM;
					break;
				case PHY_INTERFACE_MODE_RTBI:
					upsmr |= (UPSMR_RPM | UPSMR_TBIM);
					break;
				case PHY_INTERFACE_MODE_RGMII_RXID:
				case PHY_INTERFACE_MODE_RGMII_TXID:
				case PHY_INTERFACE_MODE_RGMII_ID:
				case PHY_INTERFACE_MODE_RGMII:
					upsmr |= UPSMR_RPM;
					break;
				case PHY_INTERFACE_MODE_SGMII:
					upsmr |= UPSMR_SGMM;
					break;
				default:
					return -EINVAL;
					break;
			}
			break;
		default:
			return -EINVAL;
			break;
	}

	out_be32(&uec_regs->maccfg2, maccfg2);
	out_be32(&uec->uccf->uf_regs->upsmr, upsmr);

	return 0;
}

static int init_mii_management_configuration(uec_mii_t *uec_mii_regs)
{
	uint		timeout = 0x1000;
	u32		miimcfg = 0;

	miimcfg = in_be32(&uec_mii_regs->miimcfg);
	miimcfg |= MIIMCFG_MNGMNT_CLC_DIV_INIT_VALUE;
	out_be32(&uec_mii_regs->miimcfg, miimcfg);

	/* Wait until the bus is free */
	while ((in_be32(&uec_mii_regs->miimcfg) & MIIMIND_BUSY) && timeout--);
	if (timeout <= 0) {
		printf("%s: The MII Bus is stuck!", __FUNCTION__);
		return -ETIMEDOUT;
	}

	return 0;
}

static int init_phy(struct eth_device *dev)
{
	uec_private_t		*uec;
	uec_mii_t		*umii_regs;
	struct uec_mii_info	*mii_info;
	struct phy_info		*curphy;
	int			err;

	uec = (uec_private_t *)dev->priv;
	umii_regs = uec->uec_mii_regs;

	uec->oldlink = 0;
	uec->oldspeed = 0;
	uec->oldduplex = -1;

	mii_info = malloc(sizeof(*mii_info));
	if (!mii_info) {
		printf("%s: Could not allocate mii_info", dev->name);
		return -ENOMEM;
	}
	memset(mii_info, 0, sizeof(*mii_info));

	if (uec->uec_info->uf_info.eth_type == GIGA_ETH) {
		mii_info->speed = SPEED_1000;
	} else {
		mii_info->speed = SPEED_100;
	}

	mii_info->duplex = DUPLEX_FULL;
	mii_info->pause = 0;
	mii_info->link = 1;

	mii_info->advertising = (ADVERTISED_10baseT_Half |
				ADVERTISED_10baseT_Full |
				ADVERTISED_100baseT_Half |
				ADVERTISED_100baseT_Full |
				ADVERTISED_1000baseT_Full);
	mii_info->autoneg = 1;
	mii_info->mii_id = uec->uec_info->phy_address;
	mii_info->dev = dev;

	mii_info->mdio_read = &uec_read_phy_reg;
	mii_info->mdio_write = &uec_write_phy_reg;

	uec->mii_info = mii_info;

	qe_set_mii_clk_src(uec->uec_info->uf_info.ucc_num);

	if (init_mii_management_configuration(umii_regs)) {
		printf("%s: The MII Bus is stuck!", dev->name);
		err = -1;
		goto bus_fail;
	}

	/* get info for this PHY */
	curphy = uec_get_phy_info(uec->mii_info);
	if (!curphy) {
		printf("%s: No PHY found", dev->name);
		err = -1;
		goto no_phy;
	}

	mii_info->phyinfo = curphy;

	/* Run the commands which initialize the PHY */
	if (curphy->init) {
		err = curphy->init(uec->mii_info);
		if (err)
			goto phy_init_fail;
	}

	return 0;

phy_init_fail:
no_phy:
bus_fail:
	free(mii_info);
	return err;
}

static void adjust_link(struct eth_device *dev)
{
	uec_private_t		*uec = (uec_private_t *)dev->priv;
	struct uec_mii_info	*mii_info = uec->mii_info;

	extern void change_phy_interface_mode(struct eth_device *dev,
				 phy_interface_t mode, int speed);

	if (mii_info->link) {
		/* Now we make sure that we can be in full duplex mode.
		* If not, we operate in half-duplex mode. */
		if (mii_info->duplex != uec->oldduplex) {
			if (!(mii_info->duplex)) {
				uec_set_mac_duplex(uec, DUPLEX_HALF);
				printf("%s: Half Duplex\n", dev->name);
			} else {
				uec_set_mac_duplex(uec, DUPLEX_FULL);
				printf("%s: Full Duplex\n", dev->name);
			}
			uec->oldduplex = mii_info->duplex;
		}

		if (mii_info->speed != uec->oldspeed) {
			phy_interface_t mode =
				uec->uec_info->enet_interface_type;
			if (uec->uec_info->uf_info.eth_type == GIGA_ETH) {
				switch (mii_info->speed) {
				case SPEED_1000:
					break;
				case SPEED_100:
					printf ("switching to rgmii 100\n");
					mode = PHY_INTERFACE_MODE_RGMII;
					break;
				case SPEED_10:
					printf ("switching to rgmii 10\n");
					mode = PHY_INTERFACE_MODE_RGMII;
					break;
				default:
					printf("%s: Ack,Speed(%d)is illegal\n",
						dev->name, mii_info->speed);
					break;
				}
			}

			/* change phy */
			change_phy_interface_mode(dev, mode, mii_info->speed);
			/* change the MAC interface mode */
			uec_set_mac_if_mode(uec, mode, mii_info->speed);

			printf("%s: Speed %dBT\n", dev->name, mii_info->speed);
			uec->oldspeed = mii_info->speed;
		}

		if (!uec->oldlink) {
			printf("%s: Link is up\n", dev->name);
			uec->oldlink = 1;
		}

	} else { /* if (mii_info->link) */
		if (uec->oldlink) {
			printf("%s: Link is down\n", dev->name);
			uec->oldlink = 0;
			uec->oldspeed = 0;
			uec->oldduplex = -1;
		}
	}
}

static void phy_change(struct eth_device *dev)
{
	uec_private_t	*uec = (uec_private_t *)dev->priv;

#if defined(CONFIG_ARCH_P1021) || defined(CONFIG_ARCH_P1025)
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* QE9 and QE12 need to be set for enabling QE MII managment signals */
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_QE9);
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_QE12);
#endif

	/* Update the link, speed, duplex */
	uec->mii_info->phyinfo->read_status(uec->mii_info);

#if defined(CONFIG_ARCH_P1021) || defined(CONFIG_ARCH_P1025)
	/*
	 * QE12 is muxed with LBCTL, it needs to be released for enabling
	 * LBCTL signal for LBC usage.
	 */
	clrbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_QE12);
#endif

	/* Adjust the interface according to speed */
	adjust_link(dev);
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)

/*
 * Find a device index from the devlist by name
 *
 * Returns:
 *  The index where the device is located, -1 on error
 */
static int uec_miiphy_find_dev_by_name(const char *devname)
{
	int i;

	for (i = 0; i < MAXCONTROLLERS; i++) {
		if (strncmp(devname, devlist[i]->name, strlen(devname)) == 0) {
			break;
		}
	}

	/* If device cannot be found, returns -1 */
	if (i == MAXCONTROLLERS) {
		debug ("%s: device %s not found in devlist\n", __FUNCTION__, devname);
		i = -1;
	}

	return i;
}

/*
 * Read a MII PHY register.
 *
 * Returns:
 *  0 on success
 */
static int uec_miiphy_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	unsigned short value = 0;
	int devindex = 0;

	if (bus->name == NULL) {
		debug("%s: NULL pointer given\n", __FUNCTION__);
	} else {
		devindex = uec_miiphy_find_dev_by_name(bus->name);
		if (devindex >= 0) {
			value = uec_read_phy_reg(devlist[devindex], addr, reg);
		}
	}
	return value;
}

/*
 * Write a MII PHY register.
 *
 * Returns:
 *  0 on success
 */
static int uec_miiphy_write(struct mii_dev *bus, int addr, int devad, int reg,
			    u16 value)
{
	int devindex = 0;

	if (bus->name == NULL) {
		debug("%s: NULL pointer given\n", __FUNCTION__);
	} else {
		devindex = uec_miiphy_find_dev_by_name(bus->name);
		if (devindex >= 0) {
			uec_write_phy_reg(devlist[devindex], addr, reg, value);
		}
	}
	return 0;
}
#endif

static int uec_set_mac_address(uec_private_t *uec, u8 *mac_addr)
{
	uec_t		*uec_regs;
	u32		mac_addr1;
	u32		mac_addr2;

	if (!uec) {
		printf("%s: uec not initial\n", __FUNCTION__);
		return -EINVAL;
	}

	uec_regs = uec->uec_regs;

	/* if a station address of 0x12345678ABCD, perform a write to
	MACSTNADDR1 of 0xCDAB7856,
	MACSTNADDR2 of 0x34120000 */

	mac_addr1 = (mac_addr[5] << 24) | (mac_addr[4] << 16) | \
			(mac_addr[3] << 8)  | (mac_addr[2]);
	out_be32(&uec_regs->macstnaddr1, mac_addr1);

	mac_addr2 = ((mac_addr[1] << 24) | (mac_addr[0] << 16)) & 0xffff0000;
	out_be32(&uec_regs->macstnaddr2, mac_addr2);

	return 0;
}

static int uec_convert_threads_num(uec_num_of_threads_e threads_num,
					 int *threads_num_ret)
{
	int	num_threads_numerica;

	switch (threads_num) {
		case UEC_NUM_OF_THREADS_1:
			num_threads_numerica = 1;
			break;
		case UEC_NUM_OF_THREADS_2:
			num_threads_numerica = 2;
			break;
		case UEC_NUM_OF_THREADS_4:
			num_threads_numerica = 4;
			break;
		case UEC_NUM_OF_THREADS_6:
			num_threads_numerica = 6;
			break;
		case UEC_NUM_OF_THREADS_8:
			num_threads_numerica = 8;
			break;
		default:
			printf("%s: Bad number of threads value.",
				 __FUNCTION__);
			return -EINVAL;
	}

	*threads_num_ret = num_threads_numerica;

	return 0;
}

static void uec_init_tx_parameter(uec_private_t *uec, int num_threads_tx)
{
	uec_info_t	*uec_info;
	u32		end_bd;
	u8		bmrx = 0;
	int		i;

	uec_info = uec->uec_info;

	/* Alloc global Tx parameter RAM page */
	uec->tx_glbl_pram_offset = qe_muram_alloc(
				sizeof(uec_tx_global_pram_t),
				 UEC_TX_GLOBAL_PRAM_ALIGNMENT);
	uec->p_tx_glbl_pram = (uec_tx_global_pram_t *)
				qe_muram_addr(uec->tx_glbl_pram_offset);

	/* Zero the global Tx prameter RAM */
	memset(uec->p_tx_glbl_pram, 0, sizeof(uec_tx_global_pram_t));

	/* Init global Tx parameter RAM */

	/* TEMODER, RMON statistics disable, one Tx queue */
	out_be16(&uec->p_tx_glbl_pram->temoder, TEMODER_INIT_VALUE);

	/* SQPTR */
	uec->send_q_mem_reg_offset = qe_muram_alloc(
				sizeof(uec_send_queue_qd_t),
				 UEC_SEND_QUEUE_QUEUE_DESCRIPTOR_ALIGNMENT);
	uec->p_send_q_mem_reg = (uec_send_queue_mem_region_t *)
				qe_muram_addr(uec->send_q_mem_reg_offset);
	out_be32(&uec->p_tx_glbl_pram->sqptr, uec->send_q_mem_reg_offset);

	/* Setup the table with TxBDs ring */
	end_bd = (u32)uec->p_tx_bd_ring + (uec_info->tx_bd_ring_len - 1)
					 * SIZEOFBD;
	out_be32(&uec->p_send_q_mem_reg->sqqd[0].bd_ring_base,
				 (u32)(uec->p_tx_bd_ring));
	out_be32(&uec->p_send_q_mem_reg->sqqd[0].last_bd_completed_address,
						 end_bd);

	/* Scheduler Base Pointer, we have only one Tx queue, no need it */
	out_be32(&uec->p_tx_glbl_pram->schedulerbasepointer, 0);

	/* TxRMON Base Pointer, TxRMON disable, we don't need it */
	out_be32(&uec->p_tx_glbl_pram->txrmonbaseptr, 0);

	/* TSTATE, global snooping, big endian, the CSB bus selected */
	bmrx = BMR_INIT_VALUE;
	out_be32(&uec->p_tx_glbl_pram->tstate, ((u32)(bmrx) << BMR_SHIFT));

	/* IPH_Offset */
	for (i = 0; i < MAX_IPH_OFFSET_ENTRY; i++) {
		out_8(&uec->p_tx_glbl_pram->iphoffset[i], 0);
	}

	/* VTAG table */
	for (i = 0; i < UEC_TX_VTAG_TABLE_ENTRY_MAX; i++) {
		out_be32(&uec->p_tx_glbl_pram->vtagtable[i], 0);
	}

	/* TQPTR */
	uec->thread_dat_tx_offset = qe_muram_alloc(
		num_threads_tx * sizeof(uec_thread_data_tx_t) +
		 32 *(num_threads_tx == 1), UEC_THREAD_DATA_ALIGNMENT);

	uec->p_thread_data_tx = (uec_thread_data_tx_t *)
				qe_muram_addr(uec->thread_dat_tx_offset);
	out_be32(&uec->p_tx_glbl_pram->tqptr, uec->thread_dat_tx_offset);
}

static void uec_init_rx_parameter(uec_private_t *uec, int num_threads_rx)
{
	u8	bmrx = 0;
	int	i;
	uec_82xx_address_filtering_pram_t	*p_af_pram;

	/* Allocate global Rx parameter RAM page */
	uec->rx_glbl_pram_offset = qe_muram_alloc(
		sizeof(uec_rx_global_pram_t), UEC_RX_GLOBAL_PRAM_ALIGNMENT);
	uec->p_rx_glbl_pram = (uec_rx_global_pram_t *)
				qe_muram_addr(uec->rx_glbl_pram_offset);

	/* Zero Global Rx parameter RAM */
	memset(uec->p_rx_glbl_pram, 0, sizeof(uec_rx_global_pram_t));

	/* Init global Rx parameter RAM */
	/* REMODER, Extended feature mode disable, VLAN disable,
	 LossLess flow control disable, Receive firmware statisic disable,
	 Extended address parsing mode disable, One Rx queues,
	 Dynamic maximum/minimum frame length disable, IP checksum check
	 disable, IP address alignment disable
	*/
	out_be32(&uec->p_rx_glbl_pram->remoder, REMODER_INIT_VALUE);

	/* RQPTR */
	uec->thread_dat_rx_offset = qe_muram_alloc(
			num_threads_rx * sizeof(uec_thread_data_rx_t),
			 UEC_THREAD_DATA_ALIGNMENT);
	uec->p_thread_data_rx = (uec_thread_data_rx_t *)
				qe_muram_addr(uec->thread_dat_rx_offset);
	out_be32(&uec->p_rx_glbl_pram->rqptr, uec->thread_dat_rx_offset);

	/* Type_or_Len */
	out_be16(&uec->p_rx_glbl_pram->typeorlen, 3072);

	/* RxRMON base pointer, we don't need it */
	out_be32(&uec->p_rx_glbl_pram->rxrmonbaseptr, 0);

	/* IntCoalescingPTR, we don't need it, no interrupt */
	out_be32(&uec->p_rx_glbl_pram->intcoalescingptr, 0);

	/* RSTATE, global snooping, big endian, the CSB bus selected */
	bmrx = BMR_INIT_VALUE;
	out_8(&uec->p_rx_glbl_pram->rstate, bmrx);

	/* MRBLR */
	out_be16(&uec->p_rx_glbl_pram->mrblr, MAX_RXBUF_LEN);

	/* RBDQPTR */
	uec->rx_bd_qs_tbl_offset = qe_muram_alloc(
				sizeof(uec_rx_bd_queues_entry_t) + \
				sizeof(uec_rx_prefetched_bds_t),
				 UEC_RX_BD_QUEUES_ALIGNMENT);
	uec->p_rx_bd_qs_tbl = (uec_rx_bd_queues_entry_t *)
				qe_muram_addr(uec->rx_bd_qs_tbl_offset);

	/* Zero it */
	memset(uec->p_rx_bd_qs_tbl, 0, sizeof(uec_rx_bd_queues_entry_t) + \
					sizeof(uec_rx_prefetched_bds_t));
	out_be32(&uec->p_rx_glbl_pram->rbdqptr, uec->rx_bd_qs_tbl_offset);
	out_be32(&uec->p_rx_bd_qs_tbl->externalbdbaseptr,
		 (u32)uec->p_rx_bd_ring);

	/* MFLR */
	out_be16(&uec->p_rx_glbl_pram->mflr, MAX_FRAME_LEN);
	/* MINFLR */
	out_be16(&uec->p_rx_glbl_pram->minflr, MIN_FRAME_LEN);
	/* MAXD1 */
	out_be16(&uec->p_rx_glbl_pram->maxd1, MAX_DMA1_LEN);
	/* MAXD2 */
	out_be16(&uec->p_rx_glbl_pram->maxd2, MAX_DMA2_LEN);
	/* ECAM_PTR */
	out_be32(&uec->p_rx_glbl_pram->ecamptr, 0);
	/* L2QT */
	out_be32(&uec->p_rx_glbl_pram->l2qt, 0);
	/* L3QT */
	for (i = 0; i < 8; i++)	{
		out_be32(&uec->p_rx_glbl_pram->l3qt[i], 0);
	}

	/* VLAN_TYPE */
	out_be16(&uec->p_rx_glbl_pram->vlantype, 0x8100);
	/* TCI */
	out_be16(&uec->p_rx_glbl_pram->vlantci, 0);

	/* Clear PQ2 style address filtering hash table */
	p_af_pram = (uec_82xx_address_filtering_pram_t *) \
			uec->p_rx_glbl_pram->addressfiltering;

	p_af_pram->iaddr_h = 0;
	p_af_pram->iaddr_l = 0;
	p_af_pram->gaddr_h = 0;
	p_af_pram->gaddr_l = 0;
}

static int uec_issue_init_enet_rxtx_cmd(uec_private_t *uec,
					 int thread_tx, int thread_rx)
{
	uec_init_cmd_pram_t		*p_init_enet_param;
	u32				init_enet_param_offset;
	uec_info_t			*uec_info;
	int				i;
	int				snum;
	u32				init_enet_offset;
	u32				entry_val;
	u32				command;
	u32				cecr_subblock;

	uec_info = uec->uec_info;

	/* Allocate init enet command parameter */
	uec->init_enet_param_offset = qe_muram_alloc(
					sizeof(uec_init_cmd_pram_t), 4);
	init_enet_param_offset = uec->init_enet_param_offset;
	uec->p_init_enet_param = (uec_init_cmd_pram_t *)
				qe_muram_addr(uec->init_enet_param_offset);

	/* Zero init enet command struct */
	memset((void *)uec->p_init_enet_param, 0, sizeof(uec_init_cmd_pram_t));

	/* Init the command struct */
	p_init_enet_param = uec->p_init_enet_param;
	p_init_enet_param->resinit0 = ENET_INIT_PARAM_MAGIC_RES_INIT0;
	p_init_enet_param->resinit1 = ENET_INIT_PARAM_MAGIC_RES_INIT1;
	p_init_enet_param->resinit2 = ENET_INIT_PARAM_MAGIC_RES_INIT2;
	p_init_enet_param->resinit3 = ENET_INIT_PARAM_MAGIC_RES_INIT3;
	p_init_enet_param->resinit4 = ENET_INIT_PARAM_MAGIC_RES_INIT4;
	p_init_enet_param->largestexternallookupkeysize = 0;

	p_init_enet_param->rgftgfrxglobal |= ((u32)uec_info->num_threads_rx)
					 << ENET_INIT_PARAM_RGF_SHIFT;
	p_init_enet_param->rgftgfrxglobal |= ((u32)uec_info->num_threads_tx)
					 << ENET_INIT_PARAM_TGF_SHIFT;

	/* Init Rx global parameter pointer */
	p_init_enet_param->rgftgfrxglobal |= uec->rx_glbl_pram_offset |
						 (u32)uec_info->risc_rx;

	/* Init Rx threads */
	for (i = 0; i < (thread_rx + 1); i++) {
		if ((snum = qe_get_snum()) < 0) {
			printf("%s can not get snum\n", __FUNCTION__);
			return -ENOMEM;
		}

		if (i==0) {
			init_enet_offset = 0;
		} else {
			init_enet_offset = qe_muram_alloc(
					sizeof(uec_thread_rx_pram_t),
					 UEC_THREAD_RX_PRAM_ALIGNMENT);
		}

		entry_val = ((u32)snum << ENET_INIT_PARAM_SNUM_SHIFT) |
				 init_enet_offset | (u32)uec_info->risc_rx;
		p_init_enet_param->rxthread[i] = entry_val;
	}

	/* Init Tx global parameter pointer */
	p_init_enet_param->txglobal = uec->tx_glbl_pram_offset |
					 (u32)uec_info->risc_tx;

	/* Init Tx threads */
	for (i = 0; i < thread_tx; i++) {
		if ((snum = qe_get_snum()) < 0)	{
			printf("%s can not get snum\n", __FUNCTION__);
			return -ENOMEM;
		}

		init_enet_offset = qe_muram_alloc(sizeof(uec_thread_tx_pram_t),
						 UEC_THREAD_TX_PRAM_ALIGNMENT);

		entry_val = ((u32)snum << ENET_INIT_PARAM_SNUM_SHIFT) |
				 init_enet_offset | (u32)uec_info->risc_tx;
		p_init_enet_param->txthread[i] = entry_val;
	}

	__asm__ __volatile__("sync");

	/* Issue QE command */
	command = QE_INIT_TX_RX;
	cecr_subblock =	ucc_fast_get_qe_cr_subblock(
				uec->uec_info->uf_info.ucc_num);
	qe_issue_cmd(command, cecr_subblock, (u8) QE_CR_PROTOCOL_ETHERNET,
						 init_enet_param_offset);

	return 0;
}

static int uec_startup(uec_private_t *uec)
{
	uec_info_t			*uec_info;
	ucc_fast_info_t			*uf_info;
	ucc_fast_private_t		*uccf;
	ucc_fast_t			*uf_regs;
	uec_t				*uec_regs;
	int				num_threads_tx;
	int				num_threads_rx;
	u32				utbipar;
	u32				length;
	u32				align;
	qe_bd_t				*bd;
	u8				*buf;
	int				i;

	if (!uec || !uec->uec_info) {
		printf("%s: uec or uec_info not initial\n", __FUNCTION__);
		return -EINVAL;
	}

	uec_info = uec->uec_info;
	uf_info = &(uec_info->uf_info);

	/* Check if Rx BD ring len is illegal */
	if ((uec_info->rx_bd_ring_len < UEC_RX_BD_RING_SIZE_MIN) || \
		(uec_info->rx_bd_ring_len % UEC_RX_BD_RING_SIZE_ALIGNMENT)) {
		printf("%s: Rx BD ring len must be multiple of 4, and > 8.\n",
			 __FUNCTION__);
		return -EINVAL;
	}

	/* Check if Tx BD ring len is illegal */
	if (uec_info->tx_bd_ring_len < UEC_TX_BD_RING_SIZE_MIN) {
		printf("%s: Tx BD ring length must not be smaller than 2.\n",
			 __FUNCTION__);
		return -EINVAL;
	}

	/* Check if MRBLR is illegal */
	if ((MAX_RXBUF_LEN == 0) || (MAX_RXBUF_LEN  % UEC_MRBLR_ALIGNMENT)) {
		printf("%s: max rx buffer length must be mutliple of 128.\n",
			 __FUNCTION__);
		return -EINVAL;
	}

	/* Both Rx and Tx are stopped */
	uec->grace_stopped_rx = 1;
	uec->grace_stopped_tx = 1;

	/* Init UCC fast */
	if (ucc_fast_init(uf_info, &uccf)) {
		printf("%s: failed to init ucc fast\n", __FUNCTION__);
		return -ENOMEM;
	}

	/* Save uccf */
	uec->uccf = uccf;

	/* Convert the Tx threads number */
	if (uec_convert_threads_num(uec_info->num_threads_tx,
					 &num_threads_tx)) {
		return -EINVAL;
	}

	/* Convert the Rx threads number */
	if (uec_convert_threads_num(uec_info->num_threads_rx,
					 &num_threads_rx)) {
		return -EINVAL;
	}

	uf_regs = uccf->uf_regs;

	/* UEC register is following UCC fast registers */
	uec_regs = (uec_t *)(&uf_regs->ucc_eth);

	/* Save the UEC register pointer to UEC private struct */
	uec->uec_regs = uec_regs;

	/* Init UPSMR, enable hardware statistics (UCC) */
	out_be32(&uec->uccf->uf_regs->upsmr, UPSMR_INIT_VALUE);

	/* Init MACCFG1, flow control disable, disable Tx and Rx */
	out_be32(&uec_regs->maccfg1, MACCFG1_INIT_VALUE);

	/* Init MACCFG2, length check, MAC PAD and CRC enable */
	out_be32(&uec_regs->maccfg2, MACCFG2_INIT_VALUE);

	/* Setup MAC interface mode */
	uec_set_mac_if_mode(uec, uec_info->enet_interface_type, uec_info->speed);

	/* Setup MII management base */
#ifndef CONFIG_eTSEC_MDIO_BUS
	uec->uec_mii_regs = (uec_mii_t *)(&uec_regs->miimcfg);
#else
	uec->uec_mii_regs = (uec_mii_t *) CONFIG_MIIM_ADDRESS;
#endif

	/* Setup MII master clock source */
	qe_set_mii_clk_src(uec_info->uf_info.ucc_num);

	/* Setup UTBIPAR */
	utbipar = in_be32(&uec_regs->utbipar);
	utbipar &= ~UTBIPAR_PHY_ADDRESS_MASK;

	/* Initialize UTBIPAR address to CONFIG_UTBIPAR_INIT_TBIPA for ALL UEC.
	 * This frees up the remaining SMI addresses for use.
	 */
	utbipar |= CONFIG_UTBIPAR_INIT_TBIPA << UTBIPAR_PHY_ADDRESS_SHIFT;
	out_be32(&uec_regs->utbipar, utbipar);

	/* Configure the TBI for SGMII operation */
	if ((uec->uec_info->enet_interface_type == PHY_INTERFACE_MODE_SGMII) &&
	   (uec->uec_info->speed == SPEED_1000)) {
		uec_write_phy_reg(uec->dev, uec_regs->utbipar,
			ENET_TBI_MII_ANA, TBIANA_SETTINGS);

		uec_write_phy_reg(uec->dev, uec_regs->utbipar,
			ENET_TBI_MII_TBICON, TBICON_CLK_SELECT);

		uec_write_phy_reg(uec->dev, uec_regs->utbipar,
			ENET_TBI_MII_CR, TBICR_SETTINGS);
	}

	/* Allocate Tx BDs */
	length = ((uec_info->tx_bd_ring_len * SIZEOFBD) /
		 UEC_TX_BD_RING_SIZE_MEMORY_ALIGNMENT) *
		 UEC_TX_BD_RING_SIZE_MEMORY_ALIGNMENT;
	if ((uec_info->tx_bd_ring_len * SIZEOFBD) %
		 UEC_TX_BD_RING_SIZE_MEMORY_ALIGNMENT) {
		length += UEC_TX_BD_RING_SIZE_MEMORY_ALIGNMENT;
	}

	align = UEC_TX_BD_RING_ALIGNMENT;
	uec->tx_bd_ring_offset = (u32)malloc((u32)(length + align));
	if (uec->tx_bd_ring_offset != 0) {
		uec->p_tx_bd_ring = (u8 *)((uec->tx_bd_ring_offset + align)
						 & ~(align - 1));
	}

	/* Zero all of Tx BDs */
	memset((void *)(uec->tx_bd_ring_offset), 0, length + align);

	/* Allocate Rx BDs */
	length = uec_info->rx_bd_ring_len * SIZEOFBD;
	align = UEC_RX_BD_RING_ALIGNMENT;
	uec->rx_bd_ring_offset = (u32)(malloc((u32)(length + align)));
	if (uec->rx_bd_ring_offset != 0) {
		uec->p_rx_bd_ring = (u8 *)((uec->rx_bd_ring_offset + align)
							 & ~(align - 1));
	}

	/* Zero all of Rx BDs */
	memset((void *)(uec->rx_bd_ring_offset), 0, length + align);

	/* Allocate Rx buffer */
	length = uec_info->rx_bd_ring_len * MAX_RXBUF_LEN;
	align = UEC_RX_DATA_BUF_ALIGNMENT;
	uec->rx_buf_offset = (u32)malloc(length + align);
	if (uec->rx_buf_offset != 0) {
		uec->p_rx_buf = (u8 *)((uec->rx_buf_offset + align)
						 & ~(align - 1));
	}

	/* Zero all of the Rx buffer */
	memset((void *)(uec->rx_buf_offset), 0, length + align);

	/* Init TxBD ring */
	bd = (qe_bd_t *)uec->p_tx_bd_ring;
	uec->txBd = bd;

	for (i = 0; i < uec_info->tx_bd_ring_len; i++) {
		BD_DATA_CLEAR(bd);
		BD_STATUS_SET(bd, 0);
		BD_LENGTH_SET(bd, 0);
		bd ++;
	}
	BD_STATUS_SET((--bd), TxBD_WRAP);

	/* Init RxBD ring */
	bd = (qe_bd_t *)uec->p_rx_bd_ring;
	uec->rxBd = bd;
	buf = uec->p_rx_buf;
	for (i = 0; i < uec_info->rx_bd_ring_len; i++) {
		BD_DATA_SET(bd, buf);
		BD_LENGTH_SET(bd, 0);
		BD_STATUS_SET(bd, RxBD_EMPTY);
		buf += MAX_RXBUF_LEN;
		bd ++;
	}
	BD_STATUS_SET((--bd), RxBD_WRAP | RxBD_EMPTY);

	/* Init global Tx parameter RAM */
	uec_init_tx_parameter(uec, num_threads_tx);

	/* Init global Rx parameter RAM */
	uec_init_rx_parameter(uec, num_threads_rx);

	/* Init ethernet Tx and Rx parameter command */
	if (uec_issue_init_enet_rxtx_cmd(uec, num_threads_tx,
					 num_threads_rx)) {
		printf("%s issue init enet cmd failed\n", __FUNCTION__);
		return -ENOMEM;
	}

	return 0;
}

static int uec_init(struct eth_device* dev, bd_t *bd)
{
	uec_private_t		*uec;
	int			err, i;
	struct phy_info         *curphy;
#if defined(CONFIG_ARCH_P1021) || defined(CONFIG_ARCH_P1025)
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#endif

	uec = (uec_private_t *)dev->priv;

	if (uec->the_first_run == 0) {
#if defined(CONFIG_ARCH_P1021) || defined(CONFIG_ARCH_P1025)
	/* QE9 and QE12 need to be set for enabling QE MII managment signals */
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_QE9);
	setbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_QE12);
#endif

		err = init_phy(dev);
		if (err) {
			printf("%s: Cannot initialize PHY, aborting.\n",
			       dev->name);
			return err;
		}

		curphy = uec->mii_info->phyinfo;

		if (curphy->config_aneg) {
			err = curphy->config_aneg(uec->mii_info);
			if (err) {
				printf("%s: Can't negotiate PHY\n", dev->name);
				return err;
			}
		}

		/* Give PHYs up to 5 sec to report a link */
		i = 50;
		do {
			err = curphy->read_status(uec->mii_info);
			if (!(((i-- > 0) && !uec->mii_info->link) || err))
				break;
			udelay(100000);
		} while (1);

#if defined(CONFIG_ARCH_P1021) || defined(CONFIG_ARCH_P1025)
		/* QE12 needs to be released for enabling LBCTL signal*/
		clrbits_be32(&gur->pmuxcr, MPC85xx_PMUXCR_QE12);
#endif

		if (err || i <= 0)
			printf("warning: %s: timeout on PHY link\n", dev->name);

		adjust_link(dev);
		uec->the_first_run = 1;
	}

	/* Set up the MAC address */
	if (dev->enetaddr[0] & 0x01) {
		printf("%s: MacAddress is multcast address\n",
			 __FUNCTION__);
		return -1;
	}
	uec_set_mac_address(uec, dev->enetaddr);


	err = uec_open(uec, COMM_DIR_RX_AND_TX);
	if (err) {
		printf("%s: cannot enable UEC device\n", dev->name);
		return -1;
	}

	phy_change(dev);

	return (uec->mii_info->link ? 0 : -1);
}

static void uec_halt(struct eth_device* dev)
{
	uec_private_t	*uec = (uec_private_t *)dev->priv;
	uec_stop(uec, COMM_DIR_RX_AND_TX);
}

static int uec_send(struct eth_device *dev, void *buf, int len)
{
	uec_private_t		*uec;
	ucc_fast_private_t	*uccf;
	volatile qe_bd_t	*bd;
	u16			status;
	int			i;
	int			result = 0;

	uec = (uec_private_t *)dev->priv;
	uccf = uec->uccf;
	bd = uec->txBd;

	/* Find an empty TxBD */
	for (i = 0; bd->status & TxBD_READY; i++) {
		if (i > 0x100000) {
			printf("%s: tx buffer not ready\n", dev->name);
			return result;
		}
	}

	/* Init TxBD */
	BD_DATA_SET(bd, buf);
	BD_LENGTH_SET(bd, len);
	status = bd->status;
	status &= BD_WRAP;
	status |= (TxBD_READY | TxBD_LAST);
	BD_STATUS_SET(bd, status);

	/* Tell UCC to transmit the buffer */
	ucc_fast_transmit_on_demand(uccf);

	/* Wait for buffer to be transmitted */
	for (i = 0; bd->status & TxBD_READY; i++) {
		if (i > 0x100000) {
			printf("%s: tx error\n", dev->name);
			return result;
		}
	}

	/* Ok, the buffer be transimitted */
	BD_ADVANCE(bd, status, uec->p_tx_bd_ring);
	uec->txBd = bd;
	result = 1;

	return result;
}

static int uec_recv(struct eth_device* dev)
{
	uec_private_t		*uec = dev->priv;
	volatile qe_bd_t	*bd;
	u16			status;
	u16			len;
	u8			*data;

	bd = uec->rxBd;
	status = bd->status;

	while (!(status & RxBD_EMPTY)) {
		if (!(status & RxBD_ERROR)) {
			data = BD_DATA(bd);
			len = BD_LENGTH(bd);
			net_process_received_packet(data, len);
		} else {
			printf("%s: Rx error\n", dev->name);
		}
		status &= BD_CLEAN;
		BD_LENGTH_SET(bd, 0);
		BD_STATUS_SET(bd, status | RxBD_EMPTY);
		BD_ADVANCE(bd, status, uec->p_rx_bd_ring);
		status = bd->status;
	}
	uec->rxBd = bd;

	return 1;
}

int uec_initialize(bd_t *bis, uec_info_t *uec_info)
{
	struct eth_device	*dev;
	int			i;
	uec_private_t		*uec;
	int			err;

	dev = (struct eth_device *)malloc(sizeof(struct eth_device));
	if (!dev)
		return 0;
	memset(dev, 0, sizeof(struct eth_device));

	/* Allocate the UEC private struct */
	uec = (uec_private_t *)malloc(sizeof(uec_private_t));
	if (!uec) {
		return -ENOMEM;
	}
	memset(uec, 0, sizeof(uec_private_t));

	/* Adjust uec_info */
#if (MAX_QE_RISC == 4)
	uec_info->risc_tx = QE_RISC_ALLOCATION_FOUR_RISCS;
	uec_info->risc_rx = QE_RISC_ALLOCATION_FOUR_RISCS;
#endif

	devlist[uec_info->uf_info.ucc_num] = dev;

	uec->uec_info = uec_info;
	uec->dev = dev;

	sprintf(dev->name, "UEC%d", uec_info->uf_info.ucc_num);
	dev->iobase = 0;
	dev->priv = (void *)uec;
	dev->init = uec_init;
	dev->halt = uec_halt;
	dev->send = uec_send;
	dev->recv = uec_recv;

	/* Clear the ethnet address */
	for (i = 0; i < 6; i++)
		dev->enetaddr[i] = 0;

	eth_register(dev);

	err = uec_startup(uec);
	if (err) {
		printf("%s: Cannot configure net device, aborting.",dev->name);
		return err;
	}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	int retval;
	struct mii_dev *mdiodev = mdio_alloc();
	if (!mdiodev)
		return -ENOMEM;
	strncpy(mdiodev->name, dev->name, MDIO_NAME_LEN);
	mdiodev->read = uec_miiphy_read;
	mdiodev->write = uec_miiphy_write;

	retval = mdio_register(mdiodev);
	if (retval < 0)
		return retval;
#endif

	return 1;
}

int uec_eth_init(bd_t *bis, uec_info_t *uecs, int num)
{
	int i;

	for (i = 0; i < num; i++)
		uec_initialize(bis, &uecs[i]);

	return 0;
}

int uec_standard_init(bd_t *bis)
{
	return uec_eth_init(bis, uec_info, ARRAY_SIZE(uec_info));
}
