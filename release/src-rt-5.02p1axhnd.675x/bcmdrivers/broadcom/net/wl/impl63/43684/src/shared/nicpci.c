
/** @file nicpci.c
 *
 * Code to operate on PCI/E core, in NIC or BMAC high driver mode. Note that this file is not used
 * in firmware builds.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: nicpci.c 778660 2019-09-06 12:21:21Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <hndpmu.h>
#include <bcmdevs.h>
#include <sbchipc.h>
#include <pci_core.h>
#include <pcie_core.h>
#include <nicpci.h>
#include <pcicfg.h>

typedef struct {
	union {
		sbpcieregs_t *pcieregs;
		sbpciregs_t *pciregs;
	} regs;                         /* Memory mapped register to the core */

	si_t 	*sih;					/* System interconnect handle */
	osl_t 	*osh;					/* OSL handle */
	uint8	pciecap_lcreg_offset;	/* PCIE capability LCreg offset in the config space */
	uint8	pciecap_devctrl_offset;	/* PCIE DevControl reg offset in the config space */
	bool	pcie_pr42767;
	uint8	pcie_polarity;
	uint8	pcie_war_aspm_ovr;	/* Override ASPM/Clkreq settings */
	uint8	pmecap_offset;		/* PM Capability offset in the config space */
	bool 	pmecap;				/* Capable of generating PME */
	bool	pcie_power_save;
	uint16	pmebits;
	uint16	pcie_reqsize;
	uint16	pcie_mps;
	uint8	pciecap_devctrl2_offset; /* PCIE DevControl2 reg offset in the config space */
	uint32	pcie_ltr0_regval; /* PCIE LTR0 reg cached val */
	uint32	pcie_ltr1_regval; /* PCIE LTR1 reg cached val */
	uint32	pcie_ltr2_regval; /* PCIE LTR2 reg cached val */
	uint8	pcie_configspace[PCI_CONFIG_SPACE_SIZE];
} pcicore_info_t;

/* debug/trace */
#ifdef BCMDBG_ERR
#if !defined(PCI_ERROR) /* allow over-riding */
#define	PCI_ERROR(args)	printf args
#endif // endif
#else
#define	PCI_ERROR(args)
#endif	/* BCMDBG_ERR */

/* routines to access mdio slave device registers */
static bool pcie_mdiosetblock(pcicore_info_t *pi,  uint blk);
static int pcie_mdioop(pcicore_info_t *pi, uint physmedia, uint regaddr, bool write, uint *val);
static int pciegen1_mdioop(pcicore_info_t *pi, uint physmedia, uint regaddr, bool write,
	uint *val);
static int pciegen2_mdioop(pcicore_info_t *pi, uint physmedia, uint regaddr, bool write,
	uint *val, bool slave_bypass);
static int pcie_mdiowrite(pcicore_info_t *pi, uint physmedia, uint readdr, uint val);
static int pcie_mdioread(pcicore_info_t *pi, uint physmedia, uint readdr, uint *ret_val);

static void pcie_extendL1timer(pcicore_info_t *pi, bool extend);
static void pcie_clkreq_upd(pcicore_info_t *pi, uint state);

static void pcie_war_aspm_clkreq(pcicore_info_t *pi);
static void pcie_war_serdes(pcicore_info_t *pi);
static void pcie_war_noplldown(pcicore_info_t *pi);
static void pcie_war_polarity(pcicore_info_t *pi);
static void pcie_war_pci_setup(pcicore_info_t *pi);
static void pcie_power_save_upd(pcicore_info_t *pi, bool up);
static uint32 pcie_war_delay_perst_enab(void* pch, bool enab);

static bool pcicore_pmecap(pcicore_info_t *pi);
static void pcicore_fixlatencytimer(pcicore_info_t* pch, uint8 timer_val);

#define PCIE_GEN1(sih) ((BUSTYPE((sih)->bustype) == PCI_BUS) && \
	((sih)->buscoretype == PCIE_CORE_ID))
#define PCIE_GEN2(sih) ((BUSTYPE((sih)->bustype) == PCI_BUS) &&	\
	((sih)->buscoretype == PCIE2_CORE_ID))
#define PCIE(sih) (PCIE_GEN1(sih) || PCIE_GEN2(sih))

#define PCIEGEN1_ASPM(sih)	((PCIE_GEN1(sih)) &&	\
	(((sih)->buscorerev >= 3) && ((sih)->buscorerev <= 5)))

/* delay needed between the mdio control/ mdiodata register data access */
#define PR28829_DELAY() OSL_DELAY(10)

/**
 * Initialize the PCI core. It's caller's responsibility to make sure that this is done
 * only once
 */
void *
pcicore_init(si_t *sih, osl_t *osh, volatile void *regs)
{
	pcicore_info_t *pi;
	uint8 cap_ptr;
	uint32 dummy;

	ASSERT(sih->bustype == PCI_BUS);

	/* alloc pcicore_info_t */
	if ((pi = MALLOC(osh, sizeof(pcicore_info_t))) == NULL) {
		PCI_ERROR(("pci_attach: malloc failed! malloced %d bytes\n", MALLOCED(osh)));
		return (NULL);
	}

	bzero(pi, sizeof(pcicore_info_t));

	pi->sih = sih;
	pi->osh = osh;

	if (sih->buscoretype == PCIE2_CORE_ID) {
		pi->regs.pcieregs = (sbpcieregs_t*)regs;
		cap_ptr = pcicore_find_pci_capability(pi->osh, PCI_CAP_PCIECAP_ID, NULL, NULL);
		ASSERT(cap_ptr);
		pi->pciecap_devctrl_offset = cap_ptr + PCIE_CAP_DEVCTRL_OFFSET;
		pi->pciecap_lcreg_offset = cap_ptr + PCIE_CAP_LINKCTRL_OFFSET;
		pi->pciecap_devctrl2_offset = cap_ptr + PCIE_CAP_DEVCTRL2_OFFSET;
		/* Clear SurvivePerst with 4365 for now */
		if (pi->sih->buscorerev == 0xf || pi->sih->buscorerev == 0x14 ||
			pi->sih->buscorerev == 0x16 ||
			/* EAP: Also clear for NIC 43570, which has no srom.
			 * Compare vs chip number since numerous spins were made
			 * with different buscorerevs
			 */
			(CHIPID(pi->sih->chip) == BCM43570_CHIP_ID) ||
			/* EAP: Force clear for NIC 43684/94 chips */
			BCM43684_CHIP(pi->sih->chip) ||
			BCM6715_CHIP(pi->sih->chip) ||
			BCM6710_CHIP(pi->sih->chip)) {
			AND_REG(osh, (&pi->regs.pcieregs->control), ~PCIE_SPERST);
			dummy = R_REG(osh, &pi->regs.pcieregs->control);
			BCM_REFERENCE(dummy);
		}
	} else if (sih->buscoretype == PCIE_CORE_ID) {
		pi->regs.pcieregs = (sbpcieregs_t*)regs;
		cap_ptr = pcicore_find_pci_capability(pi->osh, PCI_CAP_PCIECAP_ID, NULL, NULL);
		ASSERT(cap_ptr);
		pi->pciecap_lcreg_offset = cap_ptr + PCIE_CAP_LINKCTRL_OFFSET;
		pi->pciecap_devctrl_offset = cap_ptr + PCIE_CAP_DEVCTRL_OFFSET;
		pi->pciecap_devctrl2_offset = cap_ptr + PCIE_CAP_DEVCTRL2_OFFSET;
		pi->pcie_power_save = TRUE; /* Enable pcie_power_save by default */
	} else
		pi->regs.pciregs = (sbpciregs_t*)regs;

	pi->pcie_ltr0_regval = PCIE_LTR0_REG_DEFAULT_60;
	pi->pcie_ltr1_regval = PCIE_LTR1_REG_DEFAULT;
	pi->pcie_ltr2_regval = PCIE_LTR2_REG_DEFAULT;
	return pi;
}

/** is called on si_detach */
void
pcicore_deinit(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (pi == NULL)
		return;

	if (PCIE_GEN2(pi->sih))
		pcie_watchdog_reset(pi->osh, pi->sih, pi->regs.pcieregs);

	MFREE(pi->osh, pi, sizeof(pcicore_info_t));
}

/** return cap_offset if requested capability exists in the PCI config space */
/* Note that it's caller's responsibility to make sure it's a pci bus */
uint8
pcicore_find_pci_capability(osl_t *osh, uint8 req_cap_id, uchar *buf, uint32 *buflen)
{
	uint8 cap_id;
	uint8 cap_ptr = 0;
	uint32 bufsize;
	uint8 byte_val;

	/* check for Header type 0 */
	byte_val = read_pci_cfg_byte(PCI_CFG_HDR);
	if ((byte_val & 0x7f) != PCI_HEADER_NORMAL)
		goto end;

	/* check if the capability pointer field exists */
	byte_val = read_pci_cfg_byte(PCI_CFG_STAT);
	if (!(byte_val & PCI_CAPPTR_PRESENT))
		goto end;

	cap_ptr = read_pci_cfg_byte(PCI_CFG_CAPPTR);
	/* check if the capability pointer is 0x00 */
	if (cap_ptr == 0x00)
		goto end;

	/* loop thr'u the capability list and see if the pcie capabilty exists */

	cap_id = read_pci_cfg_byte(cap_ptr);

	while (cap_id != req_cap_id) {
		cap_ptr = read_pci_cfg_byte((cap_ptr+1));
		if (cap_ptr == 0x00) break;
		cap_id = read_pci_cfg_byte(cap_ptr);
	}
	if (cap_id != req_cap_id) {
		goto end;
	}
	/* found the caller requested capability */
	if ((buf != NULL) && (buflen != NULL)) {
		uint8 cap_data;

		bufsize = *buflen;
		if (!bufsize) goto end;
		*buflen = 0;
		/* copy the cpability data excluding cap ID and next ptr */
		cap_data = cap_ptr + 2;
		if ((bufsize + cap_data)  > SZPCR)
			bufsize = SZPCR - cap_data;
		*buflen = bufsize;
		while (bufsize--) {
			*buf = read_pci_cfg_byte(cap_data);
			cap_data++;
			buf++;
		}
	}
end:
	return cap_ptr;
}

/** Register Access API */
uint
pcie_readreg(si_t *sih, sbpcieregs_t *pcieregs, uint addrtype, uint offset)
{
	uint retval = 0xFFFFFFFF;
	osl_t   *osh = si_osh(sih);
	volatile uint32 * ptr;

	ASSERT(pcieregs != NULL);

	if ((BUSTYPE(sih->bustype) == SI_BUS) || PCIE_GEN1(sih)) {
		switch (addrtype) {
			case PCIE_CONFIGREGS:
				W_REG(osh, (&pcieregs->configaddr), offset);
				(void)R_REG(osh, (&pcieregs->configaddr));
				retval = R_REG(osh, &(pcieregs->configdata));
				break;
			case PCIE_PCIEREGS:
				W_REG(osh, &(pcieregs->u.pcie1.pcieindaddr), offset);
				(void)R_REG(osh, (&pcieregs->u.pcie1.pcieindaddr));
				retval = R_REG(osh, &(pcieregs->u.pcie1.pcieinddata));
				break;
			default:
				ASSERT(0);
				break;
		}
	}
	else if (PCIE_GEN2(sih)) {
		switch (addrtype) {
			case PCIE_PCIEREGS:
				ptr = (volatile uint32*)((volatile uint8*)pcieregs + offset);
				retval = R_REG(osh, ptr);
				break;

			case PCIE_CONFIGREGS:
				/* XXX: For internal registers caller to make sure offset
				 * is in the right format.
				 * offset = func_num << 13 | protocol_layer << 11 | offset
				 */
				W_REG(osh, (&pcieregs->configaddr), offset);
				(void)R_REG(osh, (&pcieregs->configaddr));
				retval = R_REG(osh, &(pcieregs->configdata));
				break;

		}
	}

	return retval;
}

uint
pcie_writereg(si_t *sih, sbpcieregs_t *pcieregs, uint addrtype, uint offset, uint val)
{
	osl_t   *osh = si_osh(sih);
	volatile uint32 * ptr;

	ASSERT(pcieregs != NULL);

	if ((BUSTYPE(sih->bustype) == SI_BUS) || PCIE_GEN1(sih)) {
		switch (addrtype) {
			case PCIE_CONFIGREGS:
				W_REG(osh, (&pcieregs->configaddr), offset);
				W_REG(osh, (&pcieregs->configdata), val);
				break;
			case PCIE_PCIEREGS:
				W_REG(osh, (&pcieregs->u.pcie1.pcieindaddr), offset);
				W_REG(osh, (&pcieregs->u.pcie1.pcieinddata), val);
				break;
			default:
				ASSERT(0);
				break;
		}
	}
	else if (PCIE_GEN2(sih)) {
		switch (addrtype) {
			case PCIE_PCIEREGS:
				ptr = (volatile uint32*)((volatile uint8*)pcieregs + offset);
				W_REG(osh, ptr, val);
				break;

			case PCIE_CONFIGREGS:
				/* XXX: For internal registers caller to make sure offset
				 * is in the right format.
				 * offset = func_num << 13 | protocol_layer << 11 | offset
				 */
				W_REG(osh, (&pcieregs->configaddr), offset);
				W_REG(osh, (&pcieregs->configdata), val);
				break;

		}
	}
	return 0;
}

static bool
pcie_mdiosetblock(pcicore_info_t *pi, uint blk)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint mdiodata, i = 0;
	uint pcie_serdes_spinwait = 200;

	mdiodata = MDIODATA_START | MDIODATA_WRITE | (MDIODATA_DEV_ADDR << MDIODATA_DEVADDR_SHF) |
	        (MDIODATA_BLK_ADDR << MDIODATA_REGADDR_SHF) | MDIODATA_TA | (blk << 4);
	W_REG(pi->osh, &pcieregs->u.pcie1.mdiodata, mdiodata);

	PR28829_DELAY();
	/* retry till the transaction is complete */
	while (i < pcie_serdes_spinwait) {
		if (R_REG(pi->osh, &(pcieregs->u.pcie1.mdiocontrol)) & MDIOCTL_ACCESS_DONE) {
			break;
		}
		OSL_DELAY(1000);
		i++;
	}

	if (i >= pcie_serdes_spinwait) {
		PCI_ERROR(("pcie_mdiosetblock: timed out\n"));
		return FALSE;
	}

	return TRUE;
}

static bool
pcie2_mdiosetblock(pcicore_info_t *pi, uint blk)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint mdiodata, mdioctrl, i = 0;
	uint pcie_serdes_spinwait = 200;

	mdioctrl = MDIOCTL2_DIVISOR_VAL | (0x1F << MDIOCTL2_REGADDR_SHF);
	W_REG(pi->osh, &pcieregs->u.pcie2.mdiocontrol, mdioctrl);

	mdiodata = (blk << MDIODATA2_DEVADDR_SHF) | MDIODATA2_DONE;
	W_REG(pi->osh, &pcieregs->u.pcie2.mdiowrdata, mdiodata);

	PR28829_DELAY();
	/* retry till the transaction is complete */
	while (i < pcie_serdes_spinwait) {
		if (!(R_REG(pi->osh, &(pcieregs->u.pcie2.mdiowrdata)) & MDIODATA2_DONE)) {
			break;
		}
		OSL_DELAY(1000);
		i++;
	}

	if (i >= pcie_serdes_spinwait) {
		PCI_ERROR(("pcie_mdiosetblock: timed out\n"));
		return FALSE;
	}

	return TRUE;
}

static int
pcie_mdioop(pcicore_info_t *pi, uint physmedia, uint regaddr, bool write, uint *val)
{
	if (PCIE_GEN1(pi->sih))
		return (pciegen1_mdioop(pi, physmedia, regaddr, write, val));
	else if (PCIE_GEN2(pi->sih))
		return (pciegen2_mdioop(pi, physmedia, regaddr, write, val, 0));
	else
		return 0xFFFFFFFF;
}

static int
pciegen2_mdioop(pcicore_info_t *pi, uint physmedia, uint regaddr, bool write, uint *val,
	bool slave_bypass)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint pcie_serdes_spinwait = 200, i = 0, mdio_ctrl;
	volatile uint32 *reg32;

	if (!PCIE_GEN2(pi->sih))
		ASSERT(0);

	pcie2_mdiosetblock(pi, physmedia);

	/* enable mdio access to SERDES */
	mdio_ctrl = MDIOCTL2_DIVISOR_VAL;
	mdio_ctrl |= (regaddr << MDIOCTL2_REGADDR_SHF);

	if (slave_bypass)
		mdio_ctrl |= MDIOCTL2_SLAVE_BYPASS;

	if (!write)
		mdio_ctrl |= MDIOCTL2_READ;

	W_REG(pi->osh, (&pcieregs->u.pcie2.mdiocontrol), mdio_ctrl);
	if (write) {
		reg32 =  (volatile uint32 *)&(pcieregs->u.pcie2.mdiowrdata);
		W_REG(pi->osh, reg32, *val | MDIODATA2_DONE);
	}
	else
		reg32 =  (volatile uint32 *)&(pcieregs->u.pcie2.mdiorddata);

	/* retry till the transaction is complete */
	while (i < pcie_serdes_spinwait) {
		if (!(R_REG(pi->osh, reg32) & MDIODATA2_DONE)) {
			if (!write)
				*val = (R_REG(pi->osh, reg32) & MDIODATA2_MASK);
			return 0;
		}
		OSL_DELAY(1000);
		i++;
	}
	return 0;
}

static int
pciegen1_mdioop(pcicore_info_t *pi, uint physmedia, uint regaddr, bool write, uint *val)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint mdiodata;
	uint i = 0;
	uint pcie_serdes_spinwait = 10;

	if (!PCIE_GEN1(pi->sih))
		ASSERT(0);

	/* enable mdio access to SERDES */
	W_REG(pi->osh, (&pcieregs->u.pcie1.mdiocontrol), MDIOCTL_PREAM_EN | MDIOCTL_DIVISOR_VAL);

	if (pi->sih->buscorerev >= 10) {
		/* new serdes is slower in rw, using two layers of reg address mapping */
		if (!pcie_mdiosetblock(pi, physmedia))
			return 1;
		mdiodata = (MDIODATA_DEV_ADDR << MDIODATA_DEVADDR_SHF) |
			(regaddr << MDIODATA_REGADDR_SHF);
		pcie_serdes_spinwait *= 20;
	} else {
		mdiodata = (physmedia << MDIODATA_DEVADDR_SHF_OLD) |
			(regaddr << MDIODATA_REGADDR_SHF_OLD);
	}

	if (!write)
		mdiodata |= (MDIODATA_START | MDIODATA_READ | MDIODATA_TA);
	else
		mdiodata |= (MDIODATA_START | MDIODATA_WRITE | MDIODATA_TA | *val);

	W_REG(pi->osh, &pcieregs->u.pcie1.mdiodata, mdiodata);

	PR28829_DELAY();

	/* retry till the transaction is complete */
	while (i < pcie_serdes_spinwait) {
		if (R_REG(pi->osh, &(pcieregs->u.pcie1.mdiocontrol)) & MDIOCTL_ACCESS_DONE) {
			if (!write) {
				PR28829_DELAY();
				*val = (R_REG(pi->osh, &(pcieregs->u.pcie1.mdiodata)) &
					MDIODATA_MASK);
			}
			/* Disable mdio access to SERDES */
			W_REG(pi->osh, (&pcieregs->u.pcie1.mdiocontrol), 0);
			return 0;
		}
		OSL_DELAY(1000);
		i++;
	}

	PCI_ERROR(("pcie_mdioop: timed out op: %d\n", write));
	/* Disable mdio access to SERDES */
	W_REG(pi->osh, (&pcieregs->u.pcie1.mdiocontrol), 0);
	return 1;
}

/** use the mdio interface to read from mdio slaves */
static int
pcie_mdioread(pcicore_info_t *pi, uint physmedia, uint regaddr, uint *regval)
{
	return pcie_mdioop(pi, physmedia, regaddr, FALSE, regval);
}

/** use the mdio interface to write to mdio slaves */
static int
pcie_mdiowrite(pcicore_info_t *pi, uint physmedia, uint regaddr, uint val)
{
	return pcie_mdioop(pi, physmedia, regaddr, TRUE, &val);
}

/* ***** Support functions ***** */

/**
 * By default, PCIe devices are not allowed to create payloads of greater than 128 bytes.
 * Maximum Read Request Size is a PCIe parameter that is advertized to the host, so the host can
 * choose a balance between high throughput and low 'chunkiness' on the bus. Regardless of the
 * setting of this (hardware) field, the core does not initiate read requests larger than 512 bytes.
 */
static uint32
pcie_devcontrol_mrrs(void *pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 reg_val;
	uint8 offset;

	offset = pi->pciecap_devctrl_offset;
	if (!offset)
		return 0;

	reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	/* set operation */
	if (mask) {
		if (val > PCIE_CAP_DEVCTRL_MRRS_128B) {
			if (PCIE_GEN1(pi->sih) && (pi->sih->buscorerev < 18)) {
				PCI_ERROR(("%s pcie corerev %d doesn't support >128B MRRS",
					__FUNCTION__, pi->sih->buscorerev));
				val = PCIE_CAP_DEVCTRL_MRRS_128B;
			}
		}

		reg_val &= ~PCIE_CAP_DEVCTRL_MRRS_MASK;
		reg_val |= (val << PCIE_CAP_DEVCTRL_MRRS_SHIFT) & PCIE_CAP_DEVCTRL_MRRS_MASK;

		OSL_PCI_WRITE_CONFIG(pi->osh, offset, sizeof(uint32), reg_val);
		reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	}
	return reg_val;
}

static uint32
pcie_devcontrol_mps(void *pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 reg_val;
	uint8 offset;

	offset = pi->pciecap_devctrl_offset;
	if (!offset)
		return 0;

	reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	/* set operation */
	if (mask) {
		reg_val &= ~PCIE_CAP_DEVCTRL_MPS_MASK;
		reg_val |= (val << PCIE_CAP_DEVCTRL_MPS_SHIFT) & PCIE_CAP_DEVCTRL_MPS_MASK;

		OSL_PCI_WRITE_CONFIG(pi->osh, offset, sizeof(uint32), reg_val);
		reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	}
	return reg_val;
}

uint8
pcie_clkreq(void *pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 reg_val;
	uint8 offset;

	offset = pi->pciecap_lcreg_offset;
	if (!offset)
		return 0;

	reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	/* set operation */
	if (mask) {
		if (val)
			reg_val |= PCIE_CLKREQ_ENAB;
		else
			reg_val &= ~PCIE_CLKREQ_ENAB;
		OSL_PCI_WRITE_CONFIG(pi->osh, offset, sizeof(uint32), reg_val);
		reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	}
	if (reg_val & PCIE_CLKREQ_ENAB)
		return 1;
	else
		return 0;
}

uint8
pcie_ltrenable(void *pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 reg_val;
	uint8 offset;

	offset = pi->pciecap_devctrl2_offset;
	if (!offset)
		return 0;

	reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));

	/* set operation */
	if (mask) {
		if (val)
			reg_val |= PCIE_CAP_DEVCTRL2_LTR_ENAB_MASK;
		else
			reg_val &= ~PCIE_CAP_DEVCTRL2_LTR_ENAB_MASK;
		OSL_PCI_WRITE_CONFIG(pi->osh, offset, sizeof(uint32), reg_val);
		reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	}
	if (reg_val & PCIE_CAP_DEVCTRL2_LTR_ENAB_MASK)
		return 1;
	else
		return 0;
}

/* JIRA:SWWLAN-28745
    val and return value:
	0  Disabled
	1  Enable using Message signaling[Var A]
	2  Enable using Message signaling[Var B]
	3  Enable using WAKE# signaling
*/
uint8
pcie_obffenable(void *pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 reg_val;
	uint8 offset;

	offset = pi->pciecap_devctrl2_offset;
	if (!offset)
		return 0;

	reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));

	/* set operation */
	if (mask) {
		reg_val = (reg_val & ~PCIE_CAP_DEVCTRL2_OBFF_ENAB_MASK) |
			((val << PCIE_CAP_DEVCTRL2_OBFF_ENAB_SHIFT) &
			PCIE_CAP_DEVCTRL2_OBFF_ENAB_MASK);
		OSL_PCI_WRITE_CONFIG(pi->osh, offset, sizeof(uint32), reg_val);
		reg_val = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
	}

	return  (reg_val & PCIE_CAP_DEVCTRL2_OBFF_ENAB_MASK) >> PCIE_CAP_DEVCTRL2_OBFF_ENAB_SHIFT;
}

uint32
pcie_ltr_reg(void *pch, uint32 reg, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (PCIE_GEN1(pi->sih))
		return 0;

	if ((reg != PCIE_LTR0_REG_OFFSET) &&
	    (reg != PCIE_LTR1_REG_OFFSET) &&
	    (reg != PCIE_LTR2_REG_OFFSET)) {
		PCI_ERROR(("pcie_ltr_reg: unsupported LTR register offset 0x%x\n", reg));
		return 0;
	}

	if (mask) { /* set operation */
		if (reg == PCIE_LTR0_REG_OFFSET)
			pi->pcie_ltr0_regval = val;
		else if (reg == PCIE_LTR1_REG_OFFSET)
			pi->pcie_ltr1_regval = val;
		else
			pi->pcie_ltr2_regval = val;
		pcie_writereg(pi->sih, pi->regs.pcieregs, PCIE_CONFIGREGS, reg, val);
	}
	return pcie_readreg(pi->sih, pi->regs.pcieregs, PCIE_CONFIGREGS, reg);
}

uint32
pcieltrspacing_reg(void *pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;

	if (PCIE_GEN1(sih))
		return 0;

	ASSERT(pcieregs != NULL);

	if (mask) { /* set operation */
		W_REG(pi->osh, &(pcieregs->ltrspacing), val);
	}
	return R_REG(pi->osh, &(pcieregs->ltrspacing));
}

uint32
pcieltrhysteresiscnt_reg(void *pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;

	if (PCIE_GEN1(sih))
		return 0;

	ASSERT(pcieregs != NULL);

	if (mask) { /* set operation */
		W_REG(pi->osh, &(pcieregs->ltrhysteresiscnt), val);
	}
	return R_REG(pi->osh, &(pcieregs->ltrhysteresiscnt));
}

static void
pcie_extendL1timer(pcicore_info_t *pi, bool extend)
{
	uint32 w;
	si_t *sih = pi->sih;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;

	if (!PCIE_GEN1(sih))
		return;

	w = pcie_readreg(sih, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_PMTHRESHREG);

	if (extend && sih->buscorerev >= 7)
		w |= PCIE_ASPMTIMER_EXTEND;
	else
		w &= ~PCIE_ASPMTIMER_EXTEND;
	pcie_writereg(sih, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_PMTHRESHREG, w);
	w = pcie_readreg(sih, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_PMTHRESHREG);
}

/** centralized clkreq control policy */
static void
pcie_clkreq_upd(pcicore_info_t *pi, uint state)
{
	si_t *sih = pi->sih;
	ASSERT(PCIE(sih));

	if (!PCIE_GEN1(sih))
		return;

	switch (state) {
	case SI_DOATTACH:
		if (PCIEGEN1_ASPM(sih))
			pcie_clkreq((void *)pi, 1, 0);
		break;
	case SI_PCIDOWN:
		if (sih->buscorerev == 6) {	/* turn on serdes PLL down */
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_addr),
			           ~0, 0);
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_data),
			           ~0x40, 0);
		} else if (pi->pcie_pr42767) {
			/* When the driver going down, enable clkreq if PR42767 has been applied.
			 * Also, adjust the state as system could hibernate, so Serdes PLL WAR is
			 * a must before doing this
			 */
			pcie_clkreq((void *)pi, 1, 1);
		}
		break;
	case SI_PCIUP:
		if (sih->buscorerev == 6) {	/* turn off serdes PLL down */
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_addr),
			           ~0, 0);
			si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol_data),
			           ~0x40, 0x40);
		} else if (PCIEGEN1_ASPM(sih)) {		/* disable clkreq */
			pcie_clkreq((void *)pi, 1, 0);
		}
		break;
	default:
		ASSERT(0);
		break;
	}
}

/* ***** PCI core WARs ***** */
/* Done only once at attach time */
static void
pcie_war_polarity(pcicore_info_t *pi)
{
	uint32 w;

	if (pi->pcie_polarity != 0)
		return;

	w = pcie_readreg(pi->sih, pi->regs.pcieregs, PCIE_PCIEREGS, PCIE_PLP_STATUSREG);

	/* Detect the current polarity at attach and force that polarity and
	 * disable changing the polarity
	 */
	if ((w & PCIE_PLP_POLARITYINV_STAT) == 0)
		pi->pcie_polarity = (SERDES_RX_CTRL_FORCE);
	else
		pi->pcie_polarity = (SERDES_RX_CTRL_FORCE | SERDES_RX_CTRL_POLARITY);
}

/**
 * enable ASPM and CLKREQ if srom doesn't have it.
 * Needs to happen when update to shadow SROM is needed
 *   : Coming out of 'standby'/'hibernate'
 *   : If pcie_war_aspm_ovr state changed
 */
static void
pcie_war_aspm_clkreq(pcicore_info_t *pi)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	si_t *sih = pi->sih;
	volatile uint16 val16, *reg16;
	uint32 w;

	if (!PCIEGEN1_ASPM(sih))
		return;

	/* bypass this on QT or VSIM */
	if (!ISSIM_ENAB(sih)) {

		reg16 = &pcieregs->sprom[SRSH_ASPM_OFFSET];
		val16 = R_REG(pi->osh, reg16);

		val16 &= ~SRSH_ASPM_ENB;
		if (pi->pcie_war_aspm_ovr == PCIE_ASPM_ENAB)
			val16 |= SRSH_ASPM_ENB;
		else if (pi->pcie_war_aspm_ovr == PCIE_ASPM_L1_ENAB)
			val16 |= SRSH_ASPM_L1_ENB;
		else if (pi->pcie_war_aspm_ovr == PCIE_ASPM_L0s_ENAB)
			val16 |= SRSH_ASPM_L0s_ENB;

		W_REG(pi->osh, reg16, val16);

		w = OSL_PCI_READ_CONFIG(pi->osh, pi->pciecap_lcreg_offset, sizeof(uint32));
		w &= ~PCIE_ASPM_ENAB;
		w |= pi->pcie_war_aspm_ovr;
		OSL_PCI_WRITE_CONFIG(pi->osh, pi->pciecap_lcreg_offset, sizeof(uint32), w);
	}

	reg16 = &pcieregs->sprom[SRSH_CLKREQ_OFFSET_REV5];
	val16 = R_REG(pi->osh, reg16);

	if (pi->pcie_war_aspm_ovr != PCIE_ASPM_DISAB) {
		val16 |= SRSH_CLKREQ_ENB;
		pi->pcie_pr42767 = TRUE;
	} else
		val16 &= ~SRSH_CLKREQ_ENB;

	W_REG(pi->osh, reg16, val16);
}

/* Apply the polarity determined at the start */
/* Needs to happen when coming out of 'standby'/'hibernate' */
static void
pcie_war_serdes(pcicore_info_t *pi)
{
	uint32 w = 0;

	if (pi->pcie_polarity != 0)
		pcie_mdiowrite(pi, MDIODATA_DEV_RX, SERDES_RX_CTRL, pi->pcie_polarity);

	pcie_mdioread(pi, MDIODATA_DEV_PLL, SERDES_PLL_CTRL, &w);
	if (w & PLL_CTRL_FREQDET_EN) {
		w &= ~PLL_CTRL_FREQDET_EN;
		pcie_mdiowrite(pi, MDIODATA_DEV_PLL, SERDES_PLL_CTRL, w);
	}
}

/** Fix MISC config to allow coming out of L2/L3-Ready state w/o PRST */
/* Needs to happen when coming out of 'standby'/'hibernate' */
static void
BCMINITFN(pcie_misc_config_fixup)(pcicore_info_t *pi)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint16 val16;
	volatile uint16 *reg16;

	reg16 = &pcieregs->sprom[SRSH_PCIE_MISC_CONFIG];
	val16 = R_REG(pi->osh, reg16);

	if ((val16 & SRSH_L23READY_EXIT_NOPERST) == 0) {
		val16 |= SRSH_L23READY_EXIT_NOPERST;
		W_REG(pi->osh, reg16, val16);
	}
}

/* Needs to happen when coming out of 'standby'/'hibernate' */
static void
pcie_war_noplldown(pcicore_info_t *pi)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	volatile uint16 *reg16;

	ASSERT(pi->sih->buscorerev == 7);

	/* turn off serdes PLL down */
	si_corereg(pi->sih, SI_CC_IDX, OFFSETOF(chipcregs_t, chipcontrol),
	           CHIPCTRL_4321_PLL_DOWN, CHIPCTRL_4321_PLL_DOWN);

	/*  clear srom shadow backdoor */
	reg16 = &pcieregs->sprom[SRSH_BD_OFFSET];
	W_REG(pi->osh, reg16, (uint16)0);
}

/** Needs to happen when coming out of 'standby'/'hibernate' */
static void
pcie_war_pci_setup(pcicore_info_t *pi)
{
	si_t *sih = pi->sih;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint32 w;

	if ((sih->buscorerev == 0) || (sih->buscorerev == 1)) {
		w = pcie_readreg(sih, pcieregs, PCIE_PCIEREGS, PCIE_TLP_WORKAROUNDSREG);
		w |= 0x8;
		pcie_writereg(sih, pcieregs, PCIE_PCIEREGS, PCIE_TLP_WORKAROUNDSREG, w);
	}

	if (sih->buscorerev == 1) {
		w = pcie_readreg(sih, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_LCREG);
		w |= (0x40);
		pcie_writereg(sih, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_LCREG, w);
	}

	if (sih->buscorerev == 0) {
		pcie_mdiowrite(pi, MDIODATA_DEV_RX, SERDES_RX_TIMER1, 0x8128);
		pcie_mdiowrite(pi, MDIODATA_DEV_RX, SERDES_RX_CDR, 0x0100);
		pcie_mdiowrite(pi, MDIODATA_DEV_RX, SERDES_RX_CDRBW, 0x1466);
	} else if (PCIEGEN1_ASPM(sih)) {
		/* Change the L1 threshold for better performance */
		w = pcie_readreg(sih, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_PMTHRESHREG);
		w &= ~(PCIE_L1THRESHOLDTIME_MASK);
		w |= (PCIE_L1THRESHOLD_WARVAL << PCIE_L1THRESHOLDTIME_SHIFT);
		pcie_writereg(sih, pcieregs, PCIE_PCIEREGS, PCIE_DLLP_PMTHRESHREG, w);

		pcie_war_serdes(pi);

		pcie_war_aspm_clkreq(pi);
	} else if (pi->sih->buscorerev == 7)
		pcie_war_noplldown(pi);

	/* Note that the fix is actually in the SROM, that's why this is open-ended */
	if (pi->sih->buscorerev >= 6)
		pcie_misc_config_fixup(pi);
}

void
pcie_war_ovr_aspm_update(void *pch, uint8 aspm)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!PCIE_GEN1(pi->sih))
		return;

	if (!PCIEGEN1_ASPM(pi->sih))
		return;

	/* Validate */
	if (aspm > PCIE_ASPM_ENAB)
		return;

	pi->pcie_war_aspm_ovr = aspm;

	/* Update the current state */
	pcie_war_aspm_clkreq(pi);
}

void
pcie_ltr_war(void *pch, bool enable)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	uint32 origidx, data;

	if ((CHIPID(sih->chip) != BCM4360_CHIP_ID) || (sih->buscorerev < 1))
		return;

	if (!enable)
		data = 0x02848180;
	else
		data = 0x02838280;

	origidx = si_coreidx(sih);
	si_setcore(sih, D11_CORE_ID, 0);
	si_wrapperreg(sih, AI_OOBSELOUTD30, ~0, data);
	si_wrapperreg(sih, AI_OOBSELOUTD74, ~0, 0x3);
	si_setcoreidx(sih, origidx);
}

/* Set the LTR_ACTIVE_LATENCY, LTR_ACTIVE_IDLE_LATENCY & LTR_SLEEP_LATENCY */
static void
pcie_set_LTRvals(osl_t *osh, pcicore_info_t *pi)
{
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	/* make sure the LTR values good */
	/* LTR0 */
	W_REG(osh, &pcieregs->configaddr, PCIE_LTR0_REG_OFFSET);
	W_REG(osh, &pcieregs->configdata, pi->pcie_ltr0_regval);
	/* LTR1 */
	W_REG(osh, &pcieregs->configaddr, PCIE_LTR1_REG_OFFSET);
	W_REG(osh, &pcieregs->configdata, pi->pcie_ltr1_regval);
	/* LTR2 */
	W_REG(osh, &pcieregs->configaddr, PCIE_LTR2_REG_OFFSET);
	W_REG(osh, &pcieregs->configdata, pi->pcie_ltr2_regval);
}

/**
 * XXX HW problem: LTR message may not be sent to the host despite the ARM requesting so. Problem
 * occurred on power up and when device transitioned from D3 into a higher power state.
 */
void
pcie_hw_L1SS_war(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint32 mask;

	/* corerev7 = 4350C0, corerev9 = 43602, corerev11 = 4350C1 */
	if (sih->buscorerev != 7 && sih->buscorerev != 9 && sih->buscorerev != 11)
		return;

	/* Disable SPROM load after wake from D3 */
	OR_REG(pi->osh, &pcieregs->control, PCIE_DISSPROMLD);
	/* corerev7 = 4350C0, corerev11 = 4350C1 */
	if (sih->buscorerev == 7 || sih->buscorerev == 11) {
		/* >= 4350C0 only */
		/* program chip control 6 register bits to support L1SS (bit 4 & 6) */
		/* also need to set bit 17 of pmu chipcontrol */
		mask = CC6_4350_PCIE_CLKREQ_WAKEUP_MASK |
		       CC6_4350_PMU_WAKEUP_ALPAVAIL_MASK |
		       CC6_4350_PMU_EN_EXT_PERST_MASK;
		si_pmu_chipcontrol(sih, PMU_CHIPCTL6, mask, mask);

		if (sih->buscorerev == 7) {
			mask = PMU_CC7_ENABLE_L2REFCLKPAD_PWRDWN;
			si_pmu_chipcontrol(sih, PMU_CHIPCTL7, mask, 0);
		} else if (sih->buscorerev == 11) {
			mask = PMU_CC7_ENABLE_MDIO_RESET_WAR | PMU_CC7_ENABLE_L2REFCLKPAD_PWRDWN;
			si_pmu_chipcontrol(sih, PMU_CHIPCTL7, mask, mask);
		}
	} else if (sih->buscorerev == 9) {
		mask = PMU43602_CC2_PCIE_CLKREQ_L_WAKE_EN |
		       PMU43602_CC2_ENABLE_L2REFCLKPAD_PWRDWN |
		       PMU43602_CC2_PERST_L_EXTEND_EN;
		si_pmu_chipcontrol(sih, PMU_CHIPCTL2, mask, mask);
	}
}

/**
 * XXX HW problem: LTR message may not be sent to the host despite the ARM requesting so. Problem
 * occurred on power up and when device transitioned from D3 into a higher power state.
 */
void
pcie_hw_LTR_war(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	osl_t *osh = si_osh(sih);
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint32 devstsctr2;

	/* XXX: HW JIRA: HW4345-846, only apply to >= 4350C0
	 * XXX: HW JIRA: CRWLPCIEGEN2-170, applies to PCIeG2 rev 2, 3,
	 * 4, 5, 6, 7, 8, 9, 11, 12, 13
	 */
	if (sih->buscoretype != PCIE2_CORE_ID || sih->buscorerev < 2 || sih->buscorerev == 10 ||
	    sih->buscorerev > 13)
		return;

	W_REG(osh, (&pcieregs->configaddr), PCIEGEN2_CAP_DEVSTSCTRL2_OFFSET);
	devstsctr2 = R_REG(osh, &pcieregs->configdata);
	if (devstsctr2 & PCIEGEN2_CAP_DEVSTSCTRL2_LTRENAB) {
		PCI_ERROR(("add the work around for loading the LTR values, link state 0x%04x\n",
			R_REG(osh, &pcieregs->iocstatus)));

		/* force the right LTR values ..same as JIRA 859 */
		pcie_set_LTRvals(osh, pi);

		si_core_wrapperreg(sih, 3, 0x60, 0x8080, 0);

		/* enable the LTR */
		devstsctr2 |= PCIEGEN2_CAP_DEVSTSCTRL2_LTRENAB;
		W_REG(osh, &pcieregs->configaddr, PCIEGEN2_CAP_DEVSTSCTRL2_OFFSET);
		W_REG(osh, &pcieregs->configdata, devstsctr2);

		/* set the LTR state to be active */
		W_REG(osh, &pcieregs->u.pcie2.ltr_state, LTR_ACTIVE);
		OSL_DELAY(1000);
	} else {
		PCI_ERROR(("no work around for loading the LTR values\n"));
	}
}

void
pciedev_prep_D3(void *pch, bool enter_D3)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;

	/* 4350C0/C1 and its variants (43556/43558/43566/43568/43569/43570/4354) */
	if (sih->buscorerev == 7) {
		if (enter_D3) {
			si_pmu_chipcontrol(sih, PMU_CHIPCTL7,
				PMU_CC7_ENABLE_L2REFCLKPAD_PWRDWN,
				PMU_CC7_ENABLE_L2REFCLKPAD_PWRDWN);
			si_pmu_chipcontrol(sih, PMU_CHIPCTL6,
				CC6_4350_PMU_EN_WAKEUP_MASK,
				CC6_4350_PMU_EN_WAKEUP_MASK);
		} else {
			si_pmu_chipcontrol(sih, PMU_CHIPCTL7,
				PMU_CC7_ENABLE_L2REFCLKPAD_PWRDWN, 0);
			si_pmu_chipcontrol(sih, PMU_CHIPCTL6,
				CC6_4350_PMU_EN_WAKEUP_MASK, 0);
		}
	} else if (BCM43602_CHIP(sih->chip)) {
		if (enter_D3) {
			/* set PMU43602_CC2_PCIE_PERST_L_WAKE_EN bit */
			si_pmu_chipcontrol(sih, PMU_CHIPCTL2,
				PMU43602_CC2_PCIE_PERST_L_WAKE_EN,
				PMU43602_CC2_PCIE_PERST_L_WAKE_EN);
			/* set Wake On L2 */
			OR_REG(pi->osh, (&pcieregs->control), PCIE_WakeModeL2);
		} else {
			/* clear PMU43602_CC2_PCIE_PERST_L_WAKE_EN bit */
			si_pmu_chipcontrol(sih, PMU_CHIPCTL2,
				PMU43602_CC2_PCIE_PERST_L_WAKE_EN,
				0);
			/* clear Wake On L2 */
			AND_REG(pi->osh, (&pcieregs->control), ~PCIE_WakeModeL2);
		}
	}
}

/* XXX: there were 2 WARS for the HW WAR CRWLPCIEGEN2_162..enable
 * WAR2_HWJIRA_CRWLPCIEGEN2_162
 */
#define WAR2_HWJIRA_CRWLPCIEGEN2_162

/* enable the WAR for CRWLPCIEGEN2_160, this uses the knowledge of existing war for 162  */
#define WAR_HWJIRA_CRWLPCIEGEN2_160
#define PCIEGEN2_COE_PVT_TL_CTRL_0			0x800
#define COE_PVT_TL_CTRL_0_PM_DIS_L1_REENTRY_BIT		24

#define PCIEGEN2_PVT_REG_PM_CLK_PERIOD			0x184c

void
pciedev_crwlpciegen2(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	osl_t *osh = si_osh(sih);
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	bool pciewar160, pciewar162;

	pciewar160 = (sih->buscorerev == 7 || sih->buscorerev == 9 || sih->buscorerev == 11);
	pciewar162 = (sih->buscorerev == 5 || sih->buscorerev == 7 ||
		sih->buscorerev == 8 || sih->buscorerev == 9 || sih->buscorerev == 11);

	if (!(pciewar160 || pciewar162))
		return;

#ifdef WAR2_HWJIRA_CRWLPCIEGEN2_162
	OR_REG(osh, &pcieregs->control, PCIE_DISABLE_L1CLK_GATING);
#ifdef WAR_HWJIRA_CRWLPCIEGEN2_160
	W_REG(osh, &pcieregs->configaddr, PCIEGEN2_COE_PVT_TL_CTRL_0);
	AND_REG(osh, &pcieregs->configdata,
		~(1 << COE_PVT_TL_CTRL_0_PM_DIS_L1_REENTRY_BIT));
	PCI_ERROR(("coe pvt reg 0x%04x, value 0x%04x\n", PCIEGEN2_COE_PVT_TL_CTRL_0,
		R_REG(osh, &pcieregs->configdata)));
#endif /* WAR_HWJIRA_CRWLPCIEGEN2_160 */
#endif /* WAR2_HWJIRA_CRWLPCIEGEN2_162 */
}

/*
 * XXX: SW WAR for CRWLPCIEGEN2-117
 */
static void
pciedev_crwlpciegen2_117(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	osl_t *osh = si_osh(sih);
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;

	/* XXX apply WAR for PCIe corerev 9 & 13 only
	 * Cfg clk is needed to send PME message, using pcie_pipe_Iddq to
	 * shut off refclk pad does not give enough TL clks to switch CFG
	 * clk to ALP
	 */
	if ((sih->buscorerev == 9) || (sih->buscorerev == 13))
		OR_REG(osh, &pcieregs->control, PCIE_PipeIddqDisable1);
}

/*
 * XXX: SW WAR for CRWLPCIEGEN2-180
 */
static void
pciedev_crwlpciegen2_180(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	osl_t *osh = si_osh(sih);
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;

	/* XXX This WAR is not needed for 43684 (corerev 24), 6710 and future devices. */
	if (PCIE_GEN2(pi->sih) && sih->buscorerev >= 2 && sih->buscorerev < 24) {
		W_REG(osh, &pcieregs->configaddr, PCI_PMCR_REFUP);
		OR_REG(osh, &pcieregs->configdata, 0x1f);
		PCI_ERROR(("%s:Reg:0x%x ::0x%x\n", __FUNCTION__,
			PCI_PMCR_REFUP, R_REG(osh, &pcieregs->configdata)));
	}
}

static void
pciedev_crwlpciegen2_182(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	osl_t *osh = si_osh(sih);
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;

	if (PCIE_GEN2(pi->sih) && sih->buscorerev >= 2 && sih->buscorerev <= 13) {
		/* XXX Send a fake mailbox interrupt so that we never miss
		 * a real mailbox interrupt from host. This is not a real fix.
		 * Till we have the real fix, this stays.
		 */
		W_REG(osh, &pcieregs->configaddr, PCISBMbx);
		W_REG(osh, &pcieregs->configdata, 1 << 0);
	}
}

void
pciedev_reg_pm_clk_period(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	osl_t *osh = si_osh(sih);
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint32 alp_KHz, pm_value;

	/* XXX: CRWLPCIEGEN2-171
	 * XXX: set REG_PM_CLK_PERIOD to the right value
	 * XXX: default value is not conservative enough..usually set to period of Xtal freq
	 * XXX: HW folks want us to set this to half of that
	 * XXX: (1000000 / xtalfreq_in_kHz) * 2
	 */
	if (sih->buscorerev <= 13) {
		alp_KHz = (si_pmu_alp_clock(sih, osh) / 1000);
		pm_value =  (1000000 * 2) / alp_KHz;
		W_REG(osh, &pcieregs->configaddr, PCIEGEN2_PVT_REG_PM_CLK_PERIOD);
		PCI_ERROR(("ALP in KHz is %d, cur PM_REG_VAL is %d, new PM_REG_VAL is %d\n",
			alp_KHz, R_REG(osh, &pcieregs->configdata), pm_value));
		W_REG(osh, &pcieregs->configdata, pm_value);
	}
}

void
pcie_power_save_enable(void *pch, bool enable)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!pi)
		return;

	pi->pcie_power_save = enable;
}

static void
pcie_power_save_upd(pcicore_info_t *pi, bool is_up)
{
	si_t *sih = pi->sih;

	if (!pi->pcie_power_save)
		return;

	/* XXX - do not disable clock switching when the Serdes pll is powered down
	 * due to the data corruption issue when reading srom. only give 0.4mA current
	 * saving back from 8mA total saving in pcie power save mode
	 */

	if ((sih->buscorerev >= 15) && (sih->buscorerev <= 20)) {

		pcicore_pcieserdesreg(pi, MDIO_DEV_BLK1, BLK1_PWR_MGMT1, 1, 0x7F64);

		if (is_up)
			pcicore_pcieserdesreg(pi, MDIO_DEV_BLK1, BLK1_PWR_MGMT3, 1, 0x74);
		else
			pcicore_pcieserdesreg(pi, MDIO_DEV_BLK1, BLK1_PWR_MGMT3, 1, 0x7C);

	} else if ((sih->buscorerev >= 21) && (sih->buscorerev <= 22)) {

		pcicore_pcieserdesreg(pi, MDIO_DEV_BLK1, BLK1_PWR_MGMT1, 1, 0x7E65);

		if (is_up)
			pcicore_pcieserdesreg(pi, MDIO_DEV_BLK1, BLK1_PWR_MGMT3, 1, 0x175);
		else
			pcicore_pcieserdesreg(pi, MDIO_DEV_BLK1, BLK1_PWR_MGMT3, 1, 0x17D);
	}
}

static uint32
pcie_war_delay_perst_enab(void* pch, bool enab)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint32 w;

	/* restore back to default */
	w = R_REG(pi->osh, &pcieregs->control);
	w |= PCIE_DLYPERST;
	w &= ~PCIE_DISSPROMLD;
	if (enab) {
		w &= ~PCIE_DLYPERST;
		w |= PCIE_DISSPROMLD;
	}
	W_REG(pi->osh, (&pcieregs->control), w);
	/* readback to flush the write */
	return R_REG(pi->osh, &pcieregs->control);
}

void
pcie_set_request_size(void *pch, uint16 size)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih;

	if (!pi)
		return;

	sih = pi->sih;

	if (size == 128)
		pi->pcie_reqsize = PCIE_CAP_DEVCTRL_MRRS_128B;
	else if (size == 256)
		pi->pcie_reqsize = PCIE_CAP_DEVCTRL_MRRS_256B;
	else if (size == 512)
		pi->pcie_reqsize = PCIE_CAP_DEVCTRL_MRRS_512B;
	else if (size == 1024)
		pi->pcie_reqsize = PCIE_CAP_DEVCTRL_MRRS_1024B;
	else
		return;

	if (PCIE_GEN2(sih)) {
		pcie_devcontrol_mrrs(pi, PCIE_CAP_DEVCTRL_MRRS_MASK, (uint32)pi->pcie_reqsize);
	}
}

uint16
pcie_get_request_size(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!pi)
		return (0);

	if (pi->pcie_reqsize == PCIE_CAP_DEVCTRL_MRRS_128B)
		return (128);
	else if (pi->pcie_reqsize == PCIE_CAP_DEVCTRL_MRRS_256B)
		return (256);
	else if (pi->pcie_reqsize == PCIE_CAP_DEVCTRL_MRRS_512B)
		return (512);
	return (0);
}

void
pcie_set_maxpayload_size(void *pch, uint16 size)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!pi)
		return;

	if (size == 128)
		pi->pcie_mps = PCIE_CAP_DEVCTRL_MPS_128B;
	else if (size == 256)
		pi->pcie_mps = PCIE_CAP_DEVCTRL_MPS_256B;
	else if (size == 512)
		pi->pcie_mps = PCIE_CAP_DEVCTRL_MPS_512B;
	else if (size == 1024)
		pi->pcie_mps = PCIE_CAP_DEVCTRL_MPS_1024B;
	else
		return;

	pcie_devcontrol_mps(pi, PCIE_CAP_DEVCTRL_MPS_MASK, (uint32)pi->pcie_mps);
}

uint16
pcie_get_maxpayload_size(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!pi)
		return (0);

	if (pi->pcie_mps == PCIE_CAP_DEVCTRL_MPS_128B)
		return (128);
	else if (pi->pcie_mps == PCIE_CAP_DEVCTRL_MPS_256B)
		return (256);
	else if (pi->pcie_mps == PCIE_CAP_DEVCTRL_MPS_512B)
		return (512);
	else if (pi->pcie_mps == PCIE_CAP_DEVCTRL_MPS_1024B)
		return (1024);
	return (0);
}

void
pcie_disable_TL_clk_gating(void *pch)
{
	/* disable TL clk gating is located in bit 4 of PCIEControl (Offset 0x000) */
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;

	if (!PCIE_GEN1(sih) && !PCIE_GEN2(sih))
		return;

	si_corereg(sih, sih->buscoreidx, 0, 0x10, 0x10);
}

void
pcie_set_L1_entry_time(void *pch, uint32 val)
{
	/* L1 entry time is located in bits [22:16] of register 0x1004 (pdl_control_1) */
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint32 data;

	if (!PCIE_GEN1(sih) && !PCIE_GEN2(sih))
		return;

	if (val > 0x7F)
		return;

	data = pcie_readreg(sih, pcieregs, PCIE_CONFIGREGS, PCIECFGREG_PDL_CTRL1);
	pcie_writereg(pch, pcieregs, PCIE_CONFIGREGS,
		PCIECFGREG_PDL_CTRL1, (data & ~0x7F0000) | (val << 16));
}

/** mode : 0 -- reset, 1 -- tx, 2 -- rx */
void
pcie_set_error_injection(void *pch, uint32 mode)
{
	/* through reg_phy_ctl_7 - 0x181c */
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;

	if (!PCIE_GEN1(sih) && !PCIE_GEN2(sih))
		return;

	if (mode == 0)
		pcie_writereg(pch, pcieregs, PCIE_CONFIGREGS, PCIECFGREG_REG_PHY_CTL7, 0);
	else if (mode == 1)
		pcie_writereg(pch, pcieregs, PCIE_CONFIGREGS, PCIECFGREG_REG_PHY_CTL7, 0x14031);
	else
		pcie_writereg(pch, pcieregs, PCIE_CONFIGREGS, PCIECFGREG_REG_PHY_CTL7, 0x2c031);
}

/* PR 116284
 * set L1 substate
 * param: substate 0: disable,
 *		   1: enable L1.2,
 *		   2: enable L1.1,
 *		   3: enable L1.2 and L1.1
 */
void
pcie_set_L1substate(void *pch, uint32 substate)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint32 data;

	ASSERT(PCIE_GEN2(sih));
	ASSERT(substate <= 3);

	if (substate != 0) {
		/* turn on ASPM L1 */
		data = pcie_readreg(sih, pcieregs, PCIE_CONFIGREGS, pi->pciecap_lcreg_offset);
		pcie_writereg(sih, pcieregs, PCIE_CONFIGREGS, pi->pciecap_lcreg_offset, data | 2);

		/* enable LTR */
		pcie_ltrenable(pch, 1, 1);
	}

	/* PML1_sub_control1 can only be accessed by OSL_PCI_xxxx_CONFIG */
	data = OSL_PCI_READ_CONFIG(pi->osh, PCIECFGREG_PML1_SUB_CTRL1, sizeof(uint32)) & 0xfffffff0;

	if (substate & 1)
		data |= PCI_PM_L1_2_ENA_MASK | ASPM_L1_2_ENA_MASK;

	if (substate & 2)
		data |= PCI_PM_L1_1_ENA_MASK | ASPM_L1_1_ENA_MASK;

	OSL_PCI_WRITE_CONFIG(pi->osh, PCIECFGREG_PML1_SUB_CTRL1, sizeof(uint32), data);
}

/* PR 116284
 * get L1 substate
 * return 0: disabled,
 *	  1: L1.2 enabled,
 *	  2: L1.1 enabled,
 *	  3: L1.2 and L1.1 enabled
 */
uint32
pcie_get_L1substate(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	uint32 data, substate = 0;

	ASSERT(PCIE_GEN2(sih));
	UNUSED_PARAMETER(sih);

	data = OSL_PCI_READ_CONFIG(pi->osh, PCIECFGREG_PML1_SUB_CTRL1, sizeof(uint32));

	if (data & (PCI_PM_L1_2_ENA_MASK | ASPM_L1_2_ENA_MASK))
		substate |= 1;

	if (data & (PCI_PM_L1_1_ENA_MASK | ASPM_L1_1_ENA_MASK))
		substate |= 2;

	return substate;
}

/* ***** Functions called during driver state changes ***** */
void
BCMATTACHFN(pcicore_attach)(void *pch, char *pvars, int state)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;

	if (PCIE_GEN2(sih)) {
		osl_t *osh = si_osh(sih);
		uint32 data;

		BCM_REFERENCE(osh);

		data = R_REG(osh, &(pi->regs.pcieregs->sprom[SROM_OFFSET_BAR1_CTRL]));
		if (((data & BAR1_ENC_SIZE_MASK) >> BAR1_ENC_SIZE_SHIFT) ==
		    BAR1_ENC_SIZE_4M) {
			pcie_writereg(sih, pi->regs.pcieregs, PCIE_CONFIGREGS, 0x4e0, 0x17);
		}
	}

	if (!PCIE_GEN1(sih)) {
		if ((BCM4360_CHIP_ID == CHIPID(sih->chip)) ||
		    (BCM43460_CHIP_ID == CHIPID(sih->chip)) ||
		    BCM4350_CHIP(sih->chip) ||
		    BCM43602_CHIP(sih->chip) ||
		    (BCM4352_CHIP_ID == CHIPID(sih->chip)) ||
			0)
			pi->pcie_reqsize = PCIE_CAP_DEVCTRL_MRRS_1024B;

		if ((CHIPID(sih->chip) == BCM4360_CHIP_ID) && (sih->chiprev > 3)) {
			/* reg define to suppress compiler warning for
			 * non-pcie device build
			 */
			pcie_war_delay_perst_enab(pi, TRUE);
		}
		pcie_hw_LTR_war(pch);
		pciedev_crwlpciegen2(pch);
		pciedev_reg_pm_clk_period(pch);
		pciedev_crwlpciegen2_117(pch);
		pciedev_crwlpciegen2_180(pch);
		pciedev_crwlpciegen2_182(pch);
		pcie_hw_L1SS_war(pch);
		pciedev_prep_D3(pch, FALSE);
		return;
	}

	if (PCIEGEN1_ASPM(sih)) {
		if (((sih->boardvendor == VENDOR_APPLE) &&
		     ((uint8)getintvar(pvars, "sromrev") == 4) &&
		     ((uint8)getintvar(pvars, "boardrev") <= 0x71)) ||
		    ((uint32)getintvar(pvars, "boardflags2") & BFL2_PCIEWAR_OVR)) {
			pi->pcie_war_aspm_ovr = PCIE_ASPM_DISAB;
		} else {
			pi->pcie_war_aspm_ovr = PCIE_ASPM_ENAB;
		}
	}

	pi->pcie_reqsize = PCIE_CAP_DEVCTRL_MRRS_128B;

	bzero(pi->pcie_configspace, PCI_CONFIG_SPACE_SIZE);

	/* These need to happen in this order only */
	pcie_war_polarity(pi);

	pcie_war_serdes(pi);

	pcie_war_aspm_clkreq(pi);

	pcie_clkreq_upd(pi, state);

	/* Alter default TX drive strength setting */
	if (sih->boardvendor == VENDOR_APPLE) {
		if (sih->boardtype == 0x8d)
			/* change the TX drive strength to max */
			pcicore_pcieserdesreg(pch, MDIO_DEV_TXCTRL0, 0x18, 0xff, 0x7f);
	}
}

void
pcicore_hwup(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!pi)
		return;

	if ((pi->sih->boardvendor == VENDOR_APPLE) &&
	    ((pi->sih->boardtype == BCM94360X51P2) ||
	      (pi->sih->boardtype == BCM94360X51A))) {
		/* change 1214mVpp Tx amplitude, -8.46dB de-emphasis */
		pcicore_pcieserdesreg(pi, 0x830, 0x10, 0x3ff, 0x39f);
	}

	if (!PCIE_GEN1(pi->sih))
		return;

	pcie_power_save_upd(pi, TRUE);

	if (pi->sih->boardtype == CB2_4321_BOARD || pi->sih->boardtype == CB2_4321_AG_BOARD)
		pcicore_fixlatencytimer(pch, 0x20);

	pcie_war_pci_setup(pi);

	/* Alter default TX drive strength setting */
	if (pi->sih->boardvendor == VENDOR_APPLE) {
		if (pi->sih->boardtype == 0x8d)
			/* change the TX drive strength to max */
			pcicore_pcieserdesreg(pch, MDIO_DEV_TXCTRL0, 0x18, 0xff, 0x7f);
	}
}

/** state: [SI_DOATTACH, SI_PCIDOWN, SI_PCIUP] */
void
pcicore_up(void *pch, int state)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!pi)
		return;

	if (PCIE_GEN2(pi->sih)) {
		pcie_devcontrol_mrrs(pi, PCIE_CAP_DEVCTRL_MRRS_MASK, pi->pcie_reqsize);
		if ((CHIPID(pi->sih->chip) == BCM4360_CHIP_ID) && (pi->sih->chiprev > 3)) {
			/* reg define to suppress compiler warning for
			 * non-pcie device build
			 */
			pcie_war_delay_perst_enab(pi, TRUE);
		}
		pcie_hw_LTR_war(pch);
		pciedev_crwlpciegen2(pch);
		pciedev_reg_pm_clk_period(pch);
		pciedev_crwlpciegen2_117(pch);
		pciedev_crwlpciegen2_180(pch);
		pciedev_crwlpciegen2_182(pch);
		pcie_hw_L1SS_war(pch);
		pciedev_prep_D3(pch, FALSE);
		return;
	}

	pcie_power_save_upd(pi, TRUE);

	/* Restore L1 timer for better performance */
	pcie_extendL1timer(pi, TRUE);

	pcie_clkreq_upd(pi, state);

	pcie_devcontrol_mrrs(pi, PCIE_CAP_DEVCTRL_MRRS_MASK, pi->pcie_reqsize);
}

/** When the device is going to enter D3 state (or the system is going to enter S3/S4 states */
void
pcicore_sleep(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 w;

	if (!pi || !PCIE_GEN1(pi->sih))
		return;

	pcie_power_save_upd(pi, FALSE);

	if (!PCIEGEN1_ASPM(pi->sih))
		return;

	/* PR43448: Clear ASPM L1 when going to sleep as while coming out of standby/sleep,
	 * ASPM L1 should not be accidentally enabled before driver has a chance to apply
	 * the WAR
	 */
	w = OSL_PCI_READ_CONFIG(pi->osh, pi->pciecap_lcreg_offset, sizeof(uint32));
	w &= ~PCIE_CAP_LCREG_ASPML1;
	OSL_PCI_WRITE_CONFIG(pi->osh, pi->pciecap_lcreg_offset, sizeof(uint32), w);

	pi->pcie_pr42767 = FALSE;
}

/** Unconfigure and/or apply various WARs when the wireless interface is going down */
void
pcicore_down(void *pch, int state)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!pi || !PCIE_GEN1(pi->sih))
		return;

	OSL_PCI_WRITE_CONFIG(pi->osh, PCI_BAR1_WIN, sizeof(uint32), 0x00000000);

	pcie_clkreq_upd(pi, state);

	/* Reduce L1 timer for better power savings */
	pcie_extendL1timer(pi, FALSE);

	pcie_power_save_upd(pi, FALSE);
}

/* ***** Wake-on-wireless-LAN (WOWL) support functions ***** */
/** Just uses PCI config accesses to find out, when needed before sb_attach is done */
bool
pcicore_pmecap_fast(osl_t *osh)
{
	uint8 cap_ptr;
	uint32 pmecap;

	cap_ptr = pcicore_find_pci_capability(osh, PCI_CAP_POWERMGMTCAP_ID, NULL, NULL);

	if (!cap_ptr)
		return FALSE;

	pmecap = OSL_PCI_READ_CONFIG(osh, cap_ptr, sizeof(uint32));

	return ((pmecap & PME_CAP_PM_STATES) != 0);
}

/**
 * return TRUE if PM capability exists in the pci config space
 * Uses and caches the information using core handle
 */
static bool
pcicore_pmecap(pcicore_info_t *pi)
{
	uint8 cap_ptr;
	uint32 pmecap;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	volatile uint16 *reg16;

	if (!pi->pmecap_offset) {
		cap_ptr = pcicore_find_pci_capability(pi->osh, PCI_CAP_POWERMGMTCAP_ID, NULL, NULL);
		if (!cap_ptr)
			return FALSE;

		pi->pmecap_offset = cap_ptr;

		reg16 = &pcieregs->sprom[SRSH_CLKREQ_OFFSET_REV8];
		pi->pmebits = R_REG(pi->osh, reg16);

		pmecap = OSL_PCI_READ_CONFIG(pi->osh, pi->pmecap_offset, sizeof(uint32));

		/* At least one state can generate PME */
		pi->pmecap = (pmecap & PME_CAP_PM_STATES) != 0;
	}

	return (pi->pmecap);
}

/** Enable PME generation */
void
pcicore_pmeen(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 w;

	/* if not pmecapable return */
	if (!pcicore_pmecap(pi))
		return;

	w = OSL_PCI_READ_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32));
	w |= (PME_CSR_PME_EN);
	OSL_PCI_WRITE_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32), w);
}

/** Return TRUE if PME status set */
bool
pcicore_pmestat(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 w;

	if (!pcicore_pmecap(pi))
		return FALSE;

	w = OSL_PCI_READ_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32));

	return (w & PME_CSR_PME_STAT) == PME_CSR_PME_STAT;
}

void
pcicore_pmestatclr(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 w;

	if (!pcicore_pmecap(pi))
		return;

	w = OSL_PCI_READ_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32));

	PCI_ERROR(("pcicore_pmestatclr PMECSR : 0x%x\n", w));

	/* Writing a 1 to PMESTAT will clear it */
	if ((w & PME_CSR_PME_STAT) == PME_CSR_PME_STAT) {
		OSL_PCI_WRITE_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32),
			w);
	}
}

/** Disable PME generation, clear the PME status bit if set */
void
pcicore_pmeclr(void *pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint32 w;

	if (!pcicore_pmecap(pi))
		return;

	w = OSL_PCI_READ_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32));

	PCI_ERROR(("pcicore_pci_pmeclr PMECSR : 0x%x\n", w));

	/* PMESTAT is cleared by writing 1 to it */
	w &= ~(PME_CSR_PME_EN);

	OSL_PCI_WRITE_CONFIG(pi->osh, pi->pmecap_offset + PME_CSR_OFFSET, sizeof(uint32), w);
}

/**
 * WAR for PR5730: If the latency value is zero set it to 0x20, which exceeds the PCI burst
 * size; this is only seen on certain Ricoh controllers, and only on Vista, and should
 * only be applied to 4321 single/dualband cardbus cards for now
 */
static void
pcicore_fixlatencytimer(pcicore_info_t* pch, uint8 timer_val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	osl_t *osh;
	uint8 lattim;

	osh = pi->osh;
	lattim = read_pci_cfg_byte(PCI_CFG_LATTIM);

	if (!lattim) {
		PCI_ERROR(("%s: Modifying PCI_CFG_LATTIM from 0x%x to 0x%x\n",
		           __FUNCTION__, lattim, timer_val));
		write_pci_cfg_byte(PCI_CFG_LATTIM, timer_val);
	}
}

uint32
pcie_lcreg(void *pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint8 offset;

	offset = pi->pciecap_lcreg_offset;
	if (!offset)
		return 0;

	/* set operation */
	if (mask)
		OSL_PCI_WRITE_CONFIG(pi->osh, offset, sizeof(uint32), val);

	return OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
}

#ifdef BCMDBG
void
pcicore_dump(void *pch, struct bcmstrbuf *b)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	bcm_bprintf(b, "FORCEHT %d pcie_polarity 0x%x pcie_aspm_ovr 0x%x\n",
	            pi->sih->pci_pr32414, pi->pcie_polarity, pi->pcie_war_aspm_ovr);
}
#endif /* BCMDBG */

uint32
pcicore_pciereg(void *pch, uint32 offset, uint32 mask, uint32 val, uint type)
{
	uint32 reg_val = 0;
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;

	if (mask) {
		PCI_ERROR(("PCIEREG: 0x%x writeval  0x%x\n", offset, val));
		pcie_writereg(pi->sih, pcieregs, type, offset, val);
	}

	/* Should not read register 0x154 */
	if (PCIE_GEN1(pi->sih) &&
		pi->sih->buscorerev <= 5 && offset == PCIE_DLLP_PCIE11 && type == PCIE_PCIEREGS)
		return reg_val;

	reg_val = pcie_readreg(pi->sih, pcieregs, type, offset);
	PCI_ERROR(("PCIEREG: 0x%x readval is 0x%x\n", offset, reg_val));

	return reg_val;
}

uint32
pcicore_pcieserdesreg(void *pch, uint32 mdioslave, uint32 offset, uint32 mask, uint32 val)
{
	uint32 reg_val = 0;
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (mask) {
		pcie_mdiowrite(pi, mdioslave, offset, val);
	}

	if (pcie_mdioread(pi, mdioslave, offset, &reg_val))
		reg_val = 0xFFFFFFFF;

	return reg_val;
}

uint16
pcie_get_ssid(void* pch)
{
	uint32 ssid =
		OSL_PCI_READ_CONFIG(((pcicore_info_t *)pch)->osh, PCI_CFG_SVID, sizeof(uint32));
	return (uint16)(ssid >> 16);
}

uint32
pcie_get_bar0(void* pch)
{
	return OSL_PCI_READ_CONFIG(((pcicore_info_t *)pch)->osh, PCI_CFG_BAR0, sizeof(uint32));
}

int
pcie_configspace_cache(void* pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint offset = 0;
	uint32 *tmp = (uint32 *)pi->pcie_configspace;

	while (offset < PCI_CONFIG_SPACE_SIZE) {
		*tmp++ = OSL_PCI_READ_CONFIG(pi->osh, offset, sizeof(uint32));
		offset += 4;
	}
	return BCME_OK;
}

int
pcie_configspace_restore(void* pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	uint offset = 0;
	uint32 *tmp = (uint32 *)pi->pcie_configspace;

	/* if config space was not buffered, than abort restore */
	if (*tmp == 0)
		return BCME_NOTREADY;

	while (offset < PCI_CONFIG_SPACE_SIZE) {
		OSL_PCI_WRITE_CONFIG(pi->osh, offset, sizeof(uint32), *tmp);
		tmp++;
		offset += 4;
	}
	return BCME_OK;
}

int
pcie_configspace_get(void* pch, uint8 *buf, uint size)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	memcpy(buf, pi->pcie_configspace, size);
	return 0;
}

uint32
pcie_get_link_speed(void* pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint32 data;

	data = pcie_readreg(pi->sih, pcieregs, PCIE_CONFIGREGS, pi->pciecap_lcreg_offset);
	return (data & PCIE_LINKSPEED_MASK) >> PCIE_LINKSPEED_SHIFT;
}

uint32
pcie_set_ctrlreg(void* pch, uint32 mask, uint32 val)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint32 w;

	/* mask and set */
	if (mask || val) {
		w = (R_REG(pi->osh, (&pcieregs->control)) & ~mask) | (mask & val);
		W_REG(pi->osh, (&pcieregs->control), w);
	}
	return R_REG(pi->osh, (&pcieregs->control));
}

#if defined(WLTEST) || defined(BCMDBG) || defined(BCMDBG_DUMP)
const struct fielddesc pcie_cmn_regdesc[] = {
	{ "control   0x%04x ",		PCIE_CORE_REG_CONTROL,		4},
	{ "iotatuts  0x%04x ",		PCIE_CORE_REG_IOSTATUS,		4},
	{ "bitstatus 0x%04x ",		PCIE_CORE_REG_BITSTATUS,	4},
	{ "goiosel   0x%04x \n",	PCIE_CORE_REG_GPIO_SEL,		4},
	{ "gpioout   0x%04x ",		PCIE_CORE_REG_GPIO_OUT_EN,	4},
	{ "intstatus 0x%04x ",		PCIE_CORE_REG_INT_STATUS,	4},
	{ "intmask   0x%04x ",		PCIE_CORE_REG_INT_MASK,		4},
	{ "sbpciemb  0x%04x \n",	PCIE_CORE_REG_SB_PCIE_MB,	4},
	{ "errlog    0x%04x ",		PCIE_CORE_REG_ERRLOG,		4},
	{ "erraddr   0x%04x ",		PCIE_CORE_REG_ERR_ADDR,		4},
	{ "mb_intr   0x%04x ",		PCIE_CORE_REG_MB_INTR,		4},
	{ "sbpcie0   0x%04x \n",	PCIE_CORE_REG_SB_PCIE_0,	4},
	{ "sbpcie1   0x%04x ",		PCIE_CORE_REG_SB_PCIE_1,	4},
	{ "sbpcie2   0x%04x \n",	PCIE_CORE_REG_SB_PCIE_2,	4},
	{ NULL, 0, 0}
};

const struct fielddesc pcie_cmn_cfg_regdesc[] = {
	{ "dev_ctrl:0x%04x, ",		PCIE_CFG_DEVICE_CONTROL,	4},
	{ "device_status_control_2:0x%04x, ",	PCIE_CFG_DEV_STS_CTRL_2, 4},
	{ "adv_err_cap:0x%04x, ",	PCIE_CFG_ADV_ERR_CAP,		4},
	{ "uc_err_status:0x%04x, ",	PCIE_CFG_UC_ERR_STS,		4},
	{ "ucorr_err_mask:0x%04x \n",	PCIE_CFG_UC_ERR_MASK,		4},
	{ "ucorr_err_sevr:0x%04x, ",	PCIE_CFG_UNCOR_ERR_SERV,	4},
	{ "corr_err_status:0x%04x, ",	PCIE_CFG_CORR_ERR_STS,		4},
	{ "corr_err_mask:0x%04x, ",	PCIE_CFG_CORR_ERR_MASK,		4},
	{ "adv_err_cap_control:0x%04x\n",	PCIE_CFG_ADV_ERR_CTRL,	4},
	{ "header_log1:0x%04x, ",	PCIE_CFG_HDR_LOG1,		4},
	{ "header_log2:0x%04x, ",	PCIE_CFG_HDR_LOG2,		4},
	{ "header_log3:0x%04x, ",	PCIE_CFG_HDR_LOG3,		4},
	{ "header_log4:0x%04x \n",	PCIE_CFG_HDR_LOG4,		4},
	{ "PML1sub_capID:0x%04x, ",	PCIE_CFG_PML1_SUB_CAP_ID,	4},
	{ "PML1_sub_Cap_reg:0x%04x, ",	PCIE_CFG_PML1_SUB_CAP_REG,	4},
	{ "PML1_sub_control1:0x%04x, ",	PCIE_CFG_PML1_SUB_CTRL1,	4},
	{ "PML1_sub_control2:0x%04x \n",	PCIE_CFG_PML1_SUB_CTRL3, 4},
	{ "tl_control_5:0x%04x \n",	PCIE_CFG_TL_CTRL_5,		4},
	{ NULL, 0, 0}
};

const struct fielddesc pcie_gen2_cfg_regdesc[] = {
	{ "phy_err_attn_vec:0x%04x ",	PCIE_CFG_PHY_ERR_ATT_VEC,	4},
	{ "phy_err_attn_mask:0x%04x \n", PCIE_CFG_PHY_ERR_ATT_MASK,	4},
	{ NULL, 0, 0}
};

#endif /* defined(WLTEST) || defined(BCMDBG) || defined(BCMDBG_DUMP) */

#if defined(BCMDBG_DUMP)
const struct fielddesc pcie_plp_regdesc[] = {
	{ "Mode 0x%04x ",		PCIE_PLP_MODEREG,		4},
	{ "Status 0x%04x ",		PCIE_PLP_STATUSREG,		4},
	{ "LTSSMControl 0x%04x ",	PCIE_PLP_LTSSMCTRLREG,		4},
	{ "LinkNumber 0x%04x ",		PCIE_PLP_LTLINKNUMREG,		4},
	{ "LaneNumber 0x%04x ",		PCIE_PLP_LTLANENUMREG,		4},
	{ "N_FTS 0x%04x ",		PCIE_PLP_LTNFTSREG,		4},
	{ "Attention 0x%04x ",		PCIE_PLP_ATTNREG,		4},
	{ "AttentionMask 0x%04x ",	PCIE_PLP_ATTNMASKREG,		4},
	{ "RxErrCnt 0x%04x ",		PCIE_PLP_RXERRCTR,		4},
	{ "RxFramingErrCnt 0x%04x ",	PCIE_PLP_RXFRMERRCTR,		4},
	{ "TestCtrl 0x%04x ",		PCIE_PLP_TESTCTRLREG,		4},
	{ "SERDESCtrlOvrd 0x%04x ",	PCIE_PLP_SERDESCTRLOVRDREG,	4},
	{ "TimingparamOvrd 0x%04x ",	PCIE_PLP_TIMINGOVRDREG,		4},
	{ "RXTXSMdbgReg 0x%04x ",	PCIE_PLP_RXTXSMDIAGREG,		4},
	{ "LTSSMdbgReg 0x%04x\n",	PCIE_PLP_LTSSMDIAGREG,		4},
	{ NULL, 0, 0}
};

const struct fielddesc pcie_dllp_regdesc[] =  {
	{"LinkControl 0x%04x ",		PCIE_DLLP_LCREG,		4},
	{"LinkStatus 0x%04x ",		PCIE_DLLP_LSREG,		4},
	{"LinkAttention 0x%04x ",	PCIE_DLLP_LAREG,		4},
	{"LinkAttentionMask 0x%04x ",	PCIE_DLLP_LAMASKREG,		4},
	{"NextTxSeqNum 0x%04x ",	PCIE_DLLP_NEXTTXSEQNUMREG,	4},
	{"AckedTxSeqNum 0x%04x ",	PCIE_DLLP_ACKEDTXSEQNUMREG,	4},
	{"PurgedTxSeqNum 0x%04x ",	PCIE_DLLP_PURGEDTXSEQNUMREG,	4},
	{"RxSeqNum 0x%04x ",		PCIE_DLLP_RXSEQNUMREG,		4},
	{"LinkReplay 0x%04x ",		PCIE_DLLP_LRREG,		4},
	{"LinkAckTimeout 0x%04x ",	PCIE_DLLP_LACKTOREG,		4},
	{"PowerManagementThreshold 0x%04x ", PCIE_DLLP_PMTHRESHREG,	4},
	{"RetryBufferwrptr 0x%04x ",	PCIE_DLLP_RTRYWPREG,		4},
	{"RetryBufferrdptr 0x%04x ",	PCIE_DLLP_RTRYRPREG,		4},
	{"RetryBufferpuptr 0x%04x ",	PCIE_DLLP_RTRYPPREG,		4},
	{"RetryBufferRd/Wr 0x%04x ",	PCIE_DLLP_RTRRWREG,		4},
	{"ErrorCountthreshold 0x%04x ",	PCIE_DLLP_ECTHRESHREG,		4},
	{"TLPErrorcounter 0x%04x ",	PCIE_DLLP_TLPERRCTRREG,		4},
	{"Errorcounter 0x%04x ",	PCIE_DLLP_ERRCTRREG,		4},
	{"NAKRecdcounter 0x%04x ",	PCIE_DLLP_NAKRXCTRREG,		4},
	{"Test 0x%04x\n",		PCIE_DLLP_TESTREG,		4},
	{ NULL, 0, 0}
};

const struct fielddesc pcie_tlp_regdesc[] = {
	{"Config 0x%04x ",		PCIE_TLP_CONFIGREG,		4},
	{"Workarounds 0x%04x ",		PCIE_TLP_WORKAROUNDSREG,	4},
	{"WR-DMA-UA 0x%04x ",		PCIE_TLP_WRDMAUPPER,		4},
	{"WR-DMA-LA 0x%04x ",		PCIE_TLP_WRDMALOWER,		4},
	{"WR-DMA Len/BE 0x%04x ",	PCIE_TLP_WRDMAREQ_LBEREG,	4},
	{"RD-DMA-UA 0x%04x ",		PCIE_TLP_RDDMAUPPER,		4},
	{"RD-DMA-LA 0x%04x ",		PCIE_TLP_RDDMALOWER,		4},
	{"RD-DMA Len 0x%04x ",		PCIE_TLP_RDDMALENREG,		4},
	{"MSI-DMA-UA 0x%04x ",		PCIE_TLP_MSIDMAUPPER,		4},
	{"MSI-DMA-LA 0x%04x ",		PCIE_TLP_MSIDMALOWER,		4},
	{"MSI-DMALen 0x%04x ",		PCIE_TLP_MSIDMALENREG,		4},
	{"SlaveReqLen 0x%04x ",		PCIE_TLP_SLVREQLENREG,		4},
	{"FlowControlInput 0x%04x ",	PCIE_TLP_FCINPUTSREQ,		4},
	{"TxStateMachine 0x%04x ",	PCIE_TLP_TXSMGRSREQ,		4},
	{"AddressAckXferCnt 0x%04x ",	PCIE_TLP_ADRACKCNTARBLEN,	4},
	{"DMACompletion HDR0 0x%04x ",	PCIE_TLP_DMACPLHDR0,		4},
	{"DMACompletion HDR1 0x%04x ",	PCIE_TLP_DMACPLHDR1,		4},
	{"DMACompletion HDR2 0x%04x ",	PCIE_TLP_DMACPLHDR2,		4},
	{"DMACompletionMISC0 0x%04x ",	PCIE_TLP_DMACPLMISC0,		4},
	{"DMACompletionMISC1 0x%04x ",	PCIE_TLP_DMACPLMISC1,		4},
	{"DMACompletionMISC2 0x%04x ",	PCIE_TLP_DMACPLMISC2,		4},
	{"SplitControllerReqLen 0x%04x ", PCIE_TLP_SPTCTRLLEN,		4},
	{"SplitControllerMISC0 0x%04x ", PCIE_TLP_SPTCTRLMSIC0,		4},
	{"SplitControllerMISC1 0x%04x ", PCIE_TLP_SPTCTRLMSIC1,		4},
	{"bus/dev/func 0x%04x ",	PCIE_TLP_BUSDEVFUNC,		4},
	{"ResetCounter 0x%04x ",	PCIE_TLP_RESETCTR,		4},
	{"RetryBufferValue 0x%04x ",	PCIE_TLP_RTRYBUF,		4},
	{"TargetDebug1 0x%04x ",	PCIE_TLP_TGTDEBUG1,		4},
	{"TargetDebug2 0x%04x ",	PCIE_TLP_TGTDEBUG2,		4},
	{"TargetDebug3 0x%04x\n",	PCIE_TLP_TGTDEBUG3,		4},
	{ NULL, 0, 0}
};

#endif // endif

#if defined(WLTEST) || defined(BCMDBG) || defined(BCMDBG_DUMP)
/* Dump PCIE Info */
int
pcicore_dump_pcieinfo(void *pch, struct bcmstrbuf *b)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!PCIE_GEN1(pi->sih) && !PCIE_GEN2(pi->sih))
		return BCME_ERROR;

	bcm_bprintf(b, "PCIE link speed: %d\n", pcie_get_link_speed(pch));
	return 0;
}
#endif // endif

#if defined(WLTEST) || defined(BCMDBG) || defined(BCMDBG_DUMP)
/* size that can take bitfielddump */
#define BITFIELD_DUMP_SIZE  2048

/** Dump PCIE PLP/DLLP/TLP  diagnostic registers */
int
pcicore_dump_pcieregs(void *pch, struct bcmstrbuf *b)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	char *bitfield_dump_buf;
	int i;

	if ((!PCIE_GEN1(pi->sih)) && (!PCIE_GEN2(pi->sih))) {
		return BCME_ERROR;
	}

	if (!(bitfield_dump_buf = MALLOC(pi->osh, BITFIELD_DUMP_SIZE))) {
		PCI_ERROR(("bitfield dump allocation failed\n"));
		return BCME_NOMEM;
	}

	bcm_bprintf(b, "\nPCI_CFG_REG \n");
	bcmdumpfields(si_pcie_readreg, (void *)(uintptr)pi->sih, PCIE_CONFIGREGS,
		(struct fielddesc *)(uintptr)pcie_cmn_cfg_regdesc,
		bitfield_dump_buf, b->size);
	bcm_bprintf(b, "%s", bitfield_dump_buf);

	if (PCIE_GEN2(pi->sih)) {
		bcmdumpfields(si_pcie_readreg, (void *)(uintptr)pi->sih, PCIE_CONFIGREGS,
			(struct fielddesc *)(uintptr)pcie_gen2_cfg_regdesc,
			bitfield_dump_buf, b->size);
		bcm_bprintf(b, "%s", bitfield_dump_buf);
	}

	for (i = 0; i < 156; i += 4) {
		bcm_bprintf(b, "pci_cfg_%02x: 0x%08x\n", i,
			si_pcie_readreg((void *)(uintptr)pi->sih, PCIE_CONFIGREGS, i));
	}

	bcm_bprintf(b, "\n\nPCI_CORE_REG \n");
	bcmdumpfields(si_pcie_readreg, (void *)(uintptr)pi->sih, PCIE_PCIEREGS,
		(struct fielddesc *)(uintptr)pcie_cmn_regdesc,
		bitfield_dump_buf, b->size);
	bcm_bprintf(b, "%s", bitfield_dump_buf);

	MFREE(pi->osh, bitfield_dump_buf, BITFIELD_DUMP_SIZE);
	return 0;
}

#endif	/* defined(WL_TEST) || defined(BCMDBG) || defined(BCMDBG_DUMP) */
