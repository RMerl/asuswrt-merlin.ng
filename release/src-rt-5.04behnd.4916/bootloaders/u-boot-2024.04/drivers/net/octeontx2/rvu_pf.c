// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <misc.h>
#include <net.h>
#include <pci_ids.h>
#include <asm/io.h>
#include <asm/types.h>
#include <asm/arch/board.h>
#include "cgx.h"
#include "nix.h"

extern struct udevice *rvu_af_dev;

int rvu_pf_init(struct rvu_pf *rvu)
{
	struct nix *nix;
	struct eth_pdata *pdata = dev_get_plat(rvu->dev);

	debug("%s: Allocating nix lf\n", __func__);
	nix = nix_lf_alloc(rvu->dev);
	if (!nix) {
		printf("%s: Error allocating lf for pf %d\n",
		       __func__, rvu->pfid);
		return -1;
	}
	rvu->nix = nix;

	/* to make post_probe happy */
	if (is_valid_ethaddr(nix->lmac->mac_addr)) {
		memcpy(pdata->enetaddr, nix->lmac->mac_addr, 6);
		eth_env_set_enetaddr_by_index("eth", dev_seq(rvu->dev),
					      pdata->enetaddr);
	}

	return 0;
}

static const struct eth_ops nix_eth_ops = {
	.start			= nix_lf_init,
	.send			= nix_lf_xmit,
	.recv			= nix_lf_recv,
	.free_pkt		= nix_lf_free_pkt,
	.stop			= nix_lf_halt,
	.write_hwaddr		= nix_lf_setup_mac,
};

int rvu_pf_probe(struct udevice *dev)
{
	struct rvu_pf *rvu = dev_get_priv(dev);
	int err;
	char name[16];

	debug("%s: name: %s\n", __func__, dev->name);

	rvu->pf_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_2, 0, 0,
				      PCI_REGION_TYPE, PCI_REGION_MEM);
	rvu->pfid = dev_seq(dev) + 1; // RVU PF's start from 1;
	rvu->dev = dev;
	if (!rvu_af_dev) {
		printf("%s: Error: Could not find RVU AF device\n",
		       __func__);
		return -1;
	}
	rvu->afdev = rvu_af_dev;

	debug("RVU PF %u BAR2 %p\n", rvu->pfid, rvu->pf_base);

	rvu_get_lfid_for_pf(rvu->pfid, &rvu->nix_lfid, &rvu->npa_lfid);

	err = rvu_pf_init(rvu);
	if (err)
		printf("%s: Error %d adding nix\n", __func__, err);

	/*
	 * modify device name to include index/sequence number,
	 * for better readability, this is 1:1 mapping with eth0/1/2.. names.
	 */
	sprintf(name, "rvu_pf#%d", dev_seq(dev));
	device_set_name(dev, name);
	debug("%s: name: %s\n", __func__, dev->name);
	return err;
}

int rvu_pf_remove(struct udevice *dev)
{
	struct rvu_pf *rvu = dev_get_priv(dev);

	nix_lf_shutdown(rvu->nix);
	npa_lf_shutdown(rvu->nix);

	debug("%s: rvu pf%d down --\n", __func__,  rvu->pfid);

	return 0;
}

U_BOOT_DRIVER(rvu_pf) = {
	.name   = "rvu_pf",
	.id     = UCLASS_ETH,
	.probe	= rvu_pf_probe,
	.remove = rvu_pf_remove,
	.ops    = &nix_eth_ops,
	.priv_auto	= sizeof(struct rvu_pf),
	.plat_auto	= sizeof(struct eth_pdata),
};

static struct pci_device_id rvu_pf_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_RVU_PF) },
	{}
};

U_BOOT_PCI_DEVICE(rvu_pf, rvu_pf_supported);
