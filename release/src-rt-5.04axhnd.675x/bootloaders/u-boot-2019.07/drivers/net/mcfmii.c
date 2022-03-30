// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2004-2008 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#include <common.h>
#include <config.h>
#include <net.h>
#include <netdev.h>

#ifdef CONFIG_MCF547x_8x
#include <asm/fsl_mcdmafec.h>
#else
#include <asm/fec.h>
#endif
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_CMD_NET)
#undef MII_DEBUG
#undef ET_DEBUG

/*extern int fecpin_setclear(struct eth_device *dev, int setclear);*/

#if defined(CONFIG_SYS_DISCOVER_PHY) || defined(CONFIG_CMD_MII)
#include <miiphy.h>

/* Make MII read/write commands for the FEC. */
#define mk_mii_read(ADDR, REG)		(0x60020000 | ((ADDR << 23) | \
					 (REG & 0x1f) << 18))
#define mk_mii_write(ADDR, REG, VAL)	(0x50020000 | ((ADDR << 23) | \
					 (REG & 0x1f) << 18) | (VAL & 0xffff))

#ifndef CONFIG_SYS_UNSPEC_PHYID
#	define CONFIG_SYS_UNSPEC_PHYID		0
#endif
#ifndef CONFIG_SYS_UNSPEC_STRID
#	define CONFIG_SYS_UNSPEC_STRID		0
#endif

#ifdef CONFIG_MCF547x_8x
typedef struct fec_info_dma FEC_INFO_T;
#define FEC_T fecdma_t
#else
typedef struct fec_info_s FEC_INFO_T;
#define FEC_T fec_t
#endif

typedef struct phy_info_struct {
	u32 phyid;
	char *strid;
} phy_info_t;

phy_info_t phyinfo[] = {
	{0x0022561B, "AMD79C784VC"},	/* AMD 79C784VC */
	{0x00406322, "BCM5222"},	/* Broadcom 5222 */
	{0x02a80150, "Intel82555"},	/* Intel 82555 */
	{0x0016f870, "LSI80225"},	/* LSI 80225 */
	{0x0016f880, "LSI80225/B"},	/* LSI 80225/B */
	{0x78100000, "LXT970"},		/* LXT970 */
	{0x001378e0, "LXT971"},		/* LXT971 and 972 */
	{0x00221619, "KS8721BL"},	/* Micrel KS8721BL/SL */
	{0x00221512, "KSZ8041NL"},	/* Micrel KSZ8041NL */
	{0x20005CE1, "N83640"},		/* National 83640 */
	{0x20005C90, "N83848"},		/* National 83848 */
	{0x20005CA2, "N83849"},		/* National 83849 */
	{0x01814400, "QS6612"},		/* QS6612 */
#if defined(CONFIG_SYS_UNSPEC_PHYID) && defined(CONFIG_SYS_UNSPEC_STRID)
	{CONFIG_SYS_UNSPEC_PHYID, CONFIG_SYS_UNSPEC_STRID},
#endif
	{0, 0}
};

/*
 * mii_init -- Initialize the MII for MII command without ethernet
 * This function is a subset of eth_init
 */
void mii_reset(FEC_INFO_T *info)
{
	volatile FEC_T *fecp = (FEC_T *) (info->miibase);
	int i;

	fecp->ecr = FEC_ECR_RESET;

	for (i = 0; (fecp->ecr & FEC_ECR_RESET) && (i < FEC_RESET_DELAY); ++i) {
		udelay(1);
	}
	if (i == FEC_RESET_DELAY)
		printf("FEC_RESET_DELAY timeout\n");
}

/* send command to phy using mii, wait for result */
uint mii_send(uint mii_cmd)
{
	FEC_INFO_T *info;
	volatile FEC_T *ep;
	struct eth_device *dev;
	uint mii_reply;
	int j = 0;

	/* retrieve from register structure */
	dev = eth_get_dev();
	info = dev->priv;

	ep = (FEC_T *) info->miibase;

	ep->mmfr = mii_cmd;	/* command to phy */

	/* wait for mii complete */
	while (!(ep->eir & FEC_EIR_MII) && (j < MCFFEC_TOUT_LOOP)) {
		udelay(1);
		j++;
	}
	if (j >= MCFFEC_TOUT_LOOP) {
		printf("MII not complete\n");
		return -1;
	}

	mii_reply = ep->mmfr;	/* result from phy */
	ep->eir = FEC_EIR_MII;	/* clear MII complete */
#ifdef ET_DEBUG
	printf("%s[%d] %s: sent=0x%8.8x, reply=0x%8.8x\n",
	       __FILE__, __LINE__, __FUNCTION__, mii_cmd, mii_reply);
#endif

	return (mii_reply & 0xffff);	/* data read from phy */
}
#endif				/* CONFIG_SYS_DISCOVER_PHY || (CONFIG_MII) */

#if defined(CONFIG_SYS_DISCOVER_PHY)
int mii_discover_phy(struct eth_device *dev)
{
#define MAX_PHY_PASSES 11
	FEC_INFO_T *info = dev->priv;
	int phyaddr, pass;
	uint phyno, phytype;
	int i, found = 0;

	if (info->phyname_init)
		return info->phy_addr;

	phyaddr = -1;		/* didn't find a PHY yet */
	for (pass = 1; pass <= MAX_PHY_PASSES && phyaddr < 0; ++pass) {
		if (pass > 1) {
			/* PHY may need more time to recover from reset.
			 * The LXT970 needs 50ms typical, no maximum is
			 * specified, so wait 10ms before try again.
			 * With 11 passes this gives it 100ms to wake up.
			 */
			udelay(10000);	/* wait 10ms */
		}

		for (phyno = 0; phyno < 32 && phyaddr < 0; ++phyno) {

			phytype = mii_send(mk_mii_read(phyno, MII_PHYSID1));
#ifdef ET_DEBUG
			printf("PHY type 0x%x pass %d type\n", phytype, pass);
#endif
			if (phytype == 0xffff)
				continue;
			phyaddr = phyno;
			phytype <<= 16;
			phytype |=
			    mii_send(mk_mii_read(phyno, MII_PHYSID2));

#ifdef ET_DEBUG
			printf("PHY @ 0x%x pass %d\n", phyno, pass);
#endif

			for (i = 0; (i < ARRAY_SIZE(phyinfo))
				&& (phyinfo[i].phyid != 0); i++) {
				if (phyinfo[i].phyid == phytype) {
#ifdef ET_DEBUG
					printf("phyid %x - %s\n",
					       phyinfo[i].phyid,
					       phyinfo[i].strid);
#endif
					strcpy(info->phy_name, phyinfo[i].strid);
					info->phyname_init = 1;
					found = 1;
					break;
				}
			}

			if (!found) {
#ifdef ET_DEBUG
				printf("0x%08x\n", phytype);
#endif
				strcpy(info->phy_name, "unknown");
				info->phyname_init = 1;
				break;
			}
		}
	}

	if (phyaddr < 0)
		printf("No PHY device found.\n");

	return phyaddr;
}
#endif				/* CONFIG_SYS_DISCOVER_PHY */

void mii_init(void) __attribute__((weak,alias("__mii_init")));

void __mii_init(void)
{
	FEC_INFO_T *info;
	volatile FEC_T *fecp;
	struct eth_device *dev;
	int miispd = 0, i = 0;
	u16 status = 0;
	u16 linkgood = 0;

	/* retrieve from register structure */
	dev = eth_get_dev();
	info = dev->priv;

	fecp = (FEC_T *) info->miibase;

	fecpin_setclear(dev, 1);

	mii_reset(info);

	/* We use strictly polling mode only */
	fecp->eimr = 0;

	/* Clear any pending interrupt */
	fecp->eir = 0xffffffff;

	/* Set MII speed */
	miispd = (gd->bus_clk / 1000000) / 5;
	fecp->mscr = miispd << 1;

	info->phy_addr = mii_discover_phy(dev);

	while (i < MCFFEC_TOUT_LOOP) {
		status = 0;
		i++;
		/* Read PHY control register */
		miiphy_read(dev->name, info->phy_addr, MII_BMCR, &status);

		/* If phy set to autonegotiate, wait for autonegotiation done,
		 * if phy is not autonegotiating, just wait for link up.
		 */
		if ((status & BMCR_ANENABLE) == BMCR_ANENABLE) {
			linkgood = (BMSR_ANEGCOMPLETE | BMSR_LSTATUS);
		} else {
			linkgood = BMSR_LSTATUS;
		}
		/* Read PHY status register */
		miiphy_read(dev->name, info->phy_addr, MII_BMSR, &status);
		if ((status & linkgood) == linkgood)
			break;

		udelay(1);
	}
	if (i >= MCFFEC_TOUT_LOOP) {
		printf("Link UP timeout\n");
	}

	/* adapt to the duplex and speed settings of the phy */
	info->dup_spd = miiphy_duplex(dev->name, info->phy_addr) << 16;
	info->dup_spd |= miiphy_speed(dev->name, info->phy_addr);
}

/*
 * Read and write a MII PHY register, routines used by MII Utilities
 *
 * FIXME: These routines are expected to return 0 on success, but mii_send
 *	  does _not_ return an error code. Maybe 0xFFFF means error, i.e.
 *	  no PHY connected...
 *	  For now always return 0.
 * FIXME: These routines only work after calling eth_init() at least once!
 *	  Otherwise they hang in mii_send() !!! Sorry!
 */

int mcffec_miiphy_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	short rdreg;		/* register working value */

#ifdef MII_DEBUG
	printf("miiphy_read(0x%x) @ 0x%x = ", reg, addr);
#endif
	rdreg = mii_send(mk_mii_read(addr, reg));

#ifdef MII_DEBUG
	printf("0x%04x\n", rdreg);
#endif

	return rdreg;
}

int mcffec_miiphy_write(struct mii_dev *bus, int addr, int devad, int reg,
			u16 value)
{
#ifdef MII_DEBUG
	printf("miiphy_write(0x%x) @ 0x%x = 0x%04x\n", reg, addr, value);
#endif

	mii_send(mk_mii_write(addr, reg, value));

	return 0;
}

#endif				/* CONFIG_CMD_NET */
