/*
 * Generic "Device" for RTEes/RTOSes (hndrte, threadx, etc.).
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: rte_dev.c 475239 2014-05-04 14:07:13Z $
 */

#include <typedefs.h>
#include <bcmdevs.h>
#include <hndsoc.h>
#include <siutils.h>
#include <bcmutils.h>
#ifdef SBPCI
#include <pci_core.h>
#include <hndpci.h>
#include <pcicfg.h>
#endif // endif
#include <rte_dev.h>
#include <rte_mem.h>

#ifdef BCMDBG
#define DEV_MSG(x) printf x
#else
#define DEV_MSG(x)
#endif // endif

/*
 * Generic RTE/RTOS Device support:
 *	hnd_add_d11device(dev, device): Add a d11 device.
 *	hnd_add_device(dev, coreid, device): Add a device.
 *	hnd_get_dev(char *name): Get named device struct.
 */

static hnd_dev_t *dev_list = NULL;

/* RSDB specific need! */
int
BCMATTACHFN(hnd_add_d11device)(si_t *sih, hnd_dev_t *dev, uint16 device)
{
	int err = BCME_ERROR;
	void *softc = NULL;
	void *regs = NULL;
	void *commondata = NULL;
	uint i;
	uint numunits;

	if ((numunits = si_numd11coreunits(sih)) == 0)
		return BCME_NOTFOUND;

	for (i = 0; i < numunits; i++) {
		hnd_dev_t *ldev = dev + i;

		if ((regs = si_setcore(sih, D11_CORE_ID, i)) != NULL) {
			/* commmondata from prev dev to pass to curr dev */
			ldev->commondata = commondata;
			if ((softc = ldev->ops->probe(ldev, regs, SI_BUS, device,
				D11_CORE_ID, i)) != NULL) {
				err = 0;
			}
			commondata = ldev->commondata;
		}
		if (err != BCME_OK)
			return err;

		ldev->devid = device;
		ldev->softc = softc;

		/* Add device to the head of devices list */
		ldev->next = dev_list;
		dev_list = ldev;
	}

	return BCME_OK;
}

/**
 * hnd_add_device calls the 'probe' function of the caller provided device, which in turn calls
 * 'attach' functions.
 */
int
BCMATTACHFN(hnd_add_device)(si_t *sih, hnd_dev_t *dev, uint16 coreid, uint16 device)
{
	int err = BCME_ERROR;
	void *softc = NULL;
	void *regs = NULL;
	uint unit = 0;

	if (coreid == NODEV_CORE_ID) {
		/* "Soft" device driver, just call its probe */
		if ((softc = dev->ops->probe(dev, NULL, SI_BUS, device, coreid, unit)) != NULL)
			err = BCME_OK;
	}
	else if ((regs = si_setcore(sih, coreid, unit)) != NULL) {
		/* Its a core in the SB */
		if ((softc = dev->ops->probe(dev, regs, SI_BUS, device, coreid, unit)) != NULL)
			err = BCME_OK;
	}

	if (err != BCME_OK)
		return err;

	dev->devid = device;
	dev->softc = softc;

	/* Add device to the head of devices list */
	dev->next = dev_list;
	dev_list = dev;

	return 0;
}

/* query device object by name */
hnd_dev_t *
hnd_get_dev(char *name)
{
	hnd_dev_t *dev;

	/* Loop through each dev, looking for a match */
	for (dev = dev_list; dev != NULL; dev = dev->next)
		if (strcmp(dev->name, name) == 0)
			break;

	return dev;
}

#ifdef RTE_POLL
/** run the poll routines for all devices for interfaces, it means isrs */
void
hnd_dev_poll(void)
{
	hnd_dev_t *dev = dev_list;

	/* Loop through each dev's isr routine until one of them claims the interrupt */
	while (dev) {
		/* call isr routine if one is registered */
		if (dev->ops->poll)
			dev->ops->poll(dev);

		dev = dev->next;
	}
}
#endif	/* !RTE_POLL */

void
hnd_dev_isr(void)
{
}

#ifdef	SBPCI
/* ===================== Access PCI device ===================== */
static pdev_t *pcidevs = NULL;

static bool
hnd_read_pci_config(si_t *sih, uint slot, pci_config_regs *pcr)
{
	uint32 *p;
	uint i;

	extpci_read_config(sih, 1, slot, 0, PCI_CFG_VID, pcr, 4);
	if (pcr->vendor == PCI_INVALID_VENDORID)
		return FALSE;

	for (p = (uint32 *)((uint)pcr + 4), i = 4; i < SZPCR; p++, i += 4)
		extpci_read_config(sih, 1, slot, 0, i, p, 4);

	return TRUE;
}

static uint32
BCMATTACHFN(size_bar)(si_t *sih, uint slot, uint bar)
{
	uint32 w, v, s;

	w = 0xffffffff;
	extpci_write_config(sih, 1, slot, 0, (PCI_CFG_BAR0 + (bar * 4)), &w, 4);
	extpci_read_config(sih, 1, slot, 0,  (PCI_CFG_BAR0 + (bar * 4)), &v, 4);

	/* We don't do I/O */
	if (v & 1)
		return 0;

	/* Figure size */
	v &= 0xfffffff0;
	s = 0 - (int32)v;

	return s;
}

#define	MYPCISLOT	0
#define	RTE_PCI_ADDR	0x20000000
static uint32 pciaddr = RTE_PCI_ADDR;

static uint32
BCMATTACHFN(set_bar0)(si_t *sih, uint slot)
{
	uint32 w, mask, s, b0;

	/* Get size, give up if zero */
	s = size_bar(sih, slot, 0);
	if (s == 0)
		return 0;

	mask = s - 1;
	if ((pciaddr & mask) != 0)
		pciaddr = (pciaddr + s) & ~mask;
	b0 = pciaddr;
	extpci_write_config(sih, 1, slot, 0, PCI_CFG_BAR0, &b0, 4);
	extpci_read_config(sih, 1, slot, 0, PCI_CFG_BAR0, &w, 4);

	DEV_MSG(("set_bar0: 0x%x\n", w));
	/* Something went wrong */
	if ((w & 0xfffffff0) != b0)
		return 0;

	pciaddr += s;
	return b0;
}

void
BCMATTACHFN(hnd_dev_init_pci)(si_t *sih, osl_t *osh)
{
	pci_config_regs pcr;
	sbpciregs_t *pci;
	pdev_t *pdev;
	uint slot;
	int rc;
	uint32 w, s, b0;
	uint16 hw, hr;

	rc = hndpci_init_pci(sih, 0);
	if (rc < 0) {
		DEV_MSG(("Cannot init PCI.\n"));
		return;
	} else if (rc > 0) {
		DEV_MSG(("PCI strapped for client mode.\n"));
		return;
	}

	if (!(pci = (sbpciregs_t *)si_setcore(sih, PCI_CORE_ID, 0))) {
		DEV_MSG(("Cannot setcore to PCI after init!!!\n"));
		return;
	}

	if (!hnd_read_pci_config(sih, MYPCISLOT, &pcr) ||
	    (pcr.vendor != VENDOR_BROADCOM) ||
	    (pcr.base_class != PCI_CLASS_BRIDGE) ||
	    ((pcr.base[0] & 1) == 1)) {
		DEV_MSG(("Slot %d is not us!!!\n", MYPCISLOT));
		return;
	}

	/* Change the 64 MB I/O window to memory with base at RTE_PCI_ADDR */
	W_REG(osh, &pci->sbtopci0, SBTOPCI_MEM | RTE_PCI_ADDR);

	/* Give ourselves a bar0 for the fun of it */
	if ((b0 = set_bar0(sih, MYPCISLOT)) == 0) {
		DEV_MSG(("Cannot set my bar0!!!\n"));
		return;
	}
	/* And point it to our chipc */
	w = SI_ENUM_BASE;
	extpci_write_config(sih, 1, MYPCISLOT, 0, PCI_BAR0_WIN, &w, 4);
	extpci_read_config(sih, 1, MYPCISLOT, 0, PCI_BAR0_WIN, &w, 4);
	if (w != SI_ENUM_BASE) {
		DEV_MSG(("Cannot set my bar0window: 0x%08x should be 0x%08x\n", w, SI_ENUM_BASE));
	}

	/* Now setup our bar1 */
	if ((s = size_bar(sih, 0, 1)) < hnd_get_memsize()) {
		DEV_MSG(("My bar1 is disabled or too small: %d(0x%x)\n", s, s));
		return;
	}

	/* Make sure bar1 maps PCI address 0, and maps to memory */
	w = 0;
	extpci_write_config(sih, 1, MYPCISLOT, 0, PCI_CFG_BAR1, &w, 4);
	extpci_write_config(sih, 1, MYPCISLOT, 0, PCI_BAR1_WIN, &w, 4);

	/* Do we want to record ourselves in the pdev list? */
	DEV_MSG(("My slot %d, device 0x%04x:0x%04x subsys 0x%04x:0x%04x\n",
	       MYPCISLOT, pcr.vendor, pcr.device, pcr.subsys_vendor, pcr.subsys_id));

	/* OK, finally find the pci devices */
	for (slot = 0; slot < PCI_MAX_DEVICES; slot++) {

		if (slot == MYPCISLOT)
			continue;

		if (!hnd_read_pci_config(sih, slot, &pcr) ||
		    (pcr.vendor == PCI_INVALID_VENDORID))
			continue;

		DEV_MSG(("Slot %d, device 0x%04x:0x%04x subsys 0x%04x:0x%04x ",
		       slot, pcr.vendor, pcr.device, pcr.subsys_vendor, pcr.subsys_id));

		/* Enable memory & master capabilities */
		hw = pcr.command | 6;
		extpci_write_config(sih, 1, slot, 0, PCI_CFG_CMD, &hw, 2);
		extpci_read_config(sih, 1, slot, 0, PCI_CFG_CMD, &hr, 2);
		if ((hr & 6) != 6) {
			DEV_MSG(("does not support master/memory operation (cmd: 0x%x)\n", hr));
			continue;
		}

		/* Give it a bar 0 */
		b0 = set_bar0(sih, slot);
		if ((b0 = set_bar0(sih, slot)) == 0) {
			DEV_MSG(("cannot set its bar0\n"));
			continue;
		}
		DEV_MSG(("\n"));

		pdev = MALLOC(osh, sizeof(pdev_t));
		pdev->sih = sih;
		pdev->vendor = pcr.vendor;
		pdev->device = pcr.device;
		pdev->bus = 1;
		pdev->slot = slot;
		pdev->func = 0;
		pdev->address = (void *)REG_MAP(SI_PCI_MEM | (b0 - RTE_PCI_ADDR), SI_PCI_MEM_SZ);
#ifdef BCM4336SIM
		if (pdev->device == 0x4328)
			pdev->inuse = TRUE;
		else
#endif // endif
			pdev->inuse = FALSE;
		pdev->next = pcidevs;
		pcidevs = pdev;
	}
}
/* ===================== Access PCI device ===================== */
#endif	/* SBPCI */
