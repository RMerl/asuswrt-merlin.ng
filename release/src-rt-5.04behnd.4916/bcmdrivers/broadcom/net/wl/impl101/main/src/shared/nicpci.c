
/** @file nicpci.c
 *
 * Code to operate on PCI/E core, in NIC or BMAC high driver mode. Note that this file is not used
 * in firmware builds.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: nicpci.c 825758 2023-05-30 04:01:34Z $
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

	si_t	*sih;					/* System interconnect handle */
	osl_t	*osh;					/* OSL handle */
	uint8	pciecap_lcreg_offset;	/* PCIE capability LCreg offset in the config space */
	uint8	pciecap_devctrl_offset;	/* PCIE DevControl reg offset in the config space */
	uint16	pcie_reqsize;
	uint16	pcie_mps;
	uint8	pciecap_devctrl2_offset; /* PCIE DevControl2 reg offset in the config space */
} pcicore_info_t;

/* debug/trace */
#ifdef BCMDBG_ERR
#if !defined(PCI_ERROR) /* allow over-riding */
#define	PCI_ERROR(args)	printf args
#endif
#else
#define	PCI_ERROR(args)
#endif	/* BCMDBG_ERR */

/* routines to access mdio slave device registers */
static int pcie_mdiowrite(pcicore_info_t *pi, uint physmedia, uint readdr, uint val);
static int pcie_mdioread(pcicore_info_t *pi, uint physmedia, uint readdr, uint *ret_val);

#define PCIE_GEN2(sih) ((BUSTYPE((sih)->bustype) == PCI_BUS) &&	\
	((sih)->buscoretype == PCIE2_CORE_ID))
#define PCIE(sih) (PCIE_GEN2(sih))

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
			/* EAP: Force clear for NIC 43684/94 chips */
			BCM43684_CHIP(pi->sih->chip) ||
			BCM6715_CHIP(pi->sih->chip) ||
			BCM6717_CHIP(pi->sih->chip) ||
			BCM6726_CHIP(pi->sih->chip) ||
			BCM6710_CHIP(pi->sih->chip) ||
			BCM6711_CHIP(pi->sih->chip)) {
			AND_REG(osh, (&pi->regs.pcieregs->control), ~PCIE_SPERST);
			dummy = R_REG(osh, &pi->regs.pcieregs->control);
			BCM_REFERENCE(dummy);
		}
#if defined(BCMDBG)
		if (BCM6710_CHIP(pi->sih->chip)) {
			uint32 corr_err_stat, devctl;

			corr_err_stat = OSL_PCI_READ_CONFIG(osh, PCI_CORR_ERR_STATUS,
				sizeof(uint32));

			if (corr_err_stat) {
				PCI_ERROR(("%s: Resettting backplane for device BCM6710\n",
					__FUNCTION__));
				OSL_PCI_WRITE_CONFIG(osh, PCI_SPROM_CONTROL,
					sizeof(uint32), SPROM_CFG_TO_SB_RST);

				PCI_ERROR(("Polling for SB Reset state (PCI_SPROM_CONTROL)\n"));
				SPINWAIT((OSL_PCI_READ_CONFIG(osh, PCI_SPROM_CONTROL,
					sizeof(uint32)) & SPROM_CFG_TO_SB_RST), 500);

				PCI_ERROR(("SB reset bit is de-asserted by HW. "
					"Wait for 2ms for Backplane RESET to be completed\n"));
				OSL_DELAY(2000);

				if (corr_err_stat) {
					PCI_ERROR(("%s: correctable error detected (0x%08x)\n",
						__FUNCTION__, corr_err_stat));
					OSL_PCI_WRITE_CONFIG(osh, PCI_CORR_ERR_STATUS,
					sizeof(uint32), corr_err_stat);
				}
			}

			devctl = OSL_PCI_READ_CONFIG(osh, PCIECFGREG_DEVCONTROL, sizeof(uint32));
			PCI_ERROR(("%s: devctl from pcie_read_config %x\n", __FUNCTION__, devctl));

			devctl = pcie_readreg(pi->sih, pi->regs.pcieregs, PCIE_CONFIGREGS,
				PCIECFGREG_DEVCONTROL);
			PCI_ERROR(("%s: devctl from ConfigIndAddr %x\n", __FUNCTION__, devctl));
		}
#endif /* BCMDBG */
	} else if (sih->buscoretype == PCIE_CORE_ID) {
		pi->regs.pcieregs = (sbpcieregs_t*)regs;
		cap_ptr = pcicore_find_pci_capability(pi->osh, PCI_CAP_PCIECAP_ID, NULL, NULL);
		ASSERT(cap_ptr);
		pi->pciecap_lcreg_offset = cap_ptr + PCIE_CAP_LINKCTRL_OFFSET;
		pi->pciecap_devctrl_offset = cap_ptr + PCIE_CAP_DEVCTRL_OFFSET;
		pi->pciecap_devctrl2_offset = cap_ptr + PCIE_CAP_DEVCTRL2_OFFSET;
	} else
		pi->regs.pciregs = (sbpciregs_t*)regs;

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

	if ((BUSTYPE(sih->bustype) == SI_BUS)) {
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

	if ((BUSTYPE(sih->bustype) == SI_BUS)) {
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
				W_REG(osh, (&pcieregs->configaddr), offset);
				W_REG(osh, (&pcieregs->configdata), val);
				break;

		}
	}
	return 0;
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
pcie_mdioop(pcicore_info_t *pi, uint physmedia, uint regaddr, bool write, uint *val)
{
	if (PCIE_GEN2(pi->sih))
		return (pciegen2_mdioop(pi, physmedia, regaddr, write, val, 0));
	else
		return 0xFFFFFFFF;
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

	if ((reg != PCIE_LTR0_REG_OFFSET) &&
	    (reg != PCIE_LTR1_REG_OFFSET) &&
	    (reg != PCIE_LTR2_REG_OFFSET)) {
		PCI_ERROR(("pcie_ltr_reg: unsupported LTR register offset 0x%x\n", reg));
		return 0;
	}

	if (mask) { /* set operation */
		pcie_writereg(pi->sih, pi->regs.pcieregs, PCIE_CONFIGREGS, reg, val);
	}
	return pcie_readreg(pi->sih, pi->regs.pcieregs, PCIE_CONFIGREGS, reg);
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
}

/** state: [SI_DOATTACH, SI_PCIDOWN, SI_PCIUP] */
void
pcicore_up(void *pch, int state)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!pi)
		return;

	pcie_devcontrol_mrrs(pi, PCIE_CAP_DEVCTRL_MRRS_MASK, pi->pcie_reqsize);
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

uint32
pcie_get_link_speed(void* pch)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;
	uint32 data;

	data = pcie_readreg(pi->sih, pcieregs, PCIE_CONFIGREGS, pi->pciecap_lcreg_offset);
	return (data & PCIE_LINKSPEED_MASK) >> PCIE_LINKSPEED_SHIFT;
}

/** mode : 0 -- reset, 1 -- tx, 2 -- rx */
void
pcie_set_error_injection(void *pch, uint32 mode)
{
	/* through reg_phy_ctl_7 - 0x181c */
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	si_t *sih = pi->sih;
	sbpcieregs_t *pcieregs = pi->regs.pcieregs;

	if (!PCIE_GEN2(sih))
		return;

	if (mode == 0)
		pcie_writereg(pch, pcieregs, PCIE_CONFIGREGS, PCIECFGREG_REG_PHY_CTL7, 0);
	else if (mode == 1)
		pcie_writereg(pch, pcieregs, PCIE_CONFIGREGS, PCIECFGREG_REG_PHY_CTL7, 0x14031);
	else
		pcie_writereg(pch, pcieregs, PCIE_CONFIGREGS, PCIECFGREG_REG_PHY_CTL7, 0x2c031);
}

#if defined(WLTEST) || defined(BCMDBG)
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

#endif

#if defined(WLTEST) || defined(BCMDBG)
/* Dump PCIE Info */
int
pcicore_dump_pcieinfo(void *pch, struct bcmstrbuf *b)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;

	if (!PCIE_GEN2(pi->sih))
		return BCME_ERROR;

	bcm_bprintf(b, "PCIE link speed: %d\n", pcie_get_link_speed(pch));
	return 0;
}
#endif

#if defined(WLTEST) || defined(BCMDBG)
/* size that can take bitfielddump */
#define BITFIELD_DUMP_SIZE  2048

/** Dump PCIE PLP/DLLP/TLP  diagnostic registers */
int
pcicore_dump_pcieregs(void *pch, struct bcmstrbuf *b)
{
	pcicore_info_t *pi = (pcicore_info_t *)pch;
	char *bitfield_dump_buf;
	int i;

	if (!PCIE_GEN2(pi->sih)) {
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

#endif
