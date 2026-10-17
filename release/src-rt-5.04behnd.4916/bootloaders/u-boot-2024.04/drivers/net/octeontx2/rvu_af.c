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
#include <linux/list.h>
#include <asm/io.h>
#include <asm/arch/board.h>
#include <asm/arch/csrs/csrs-npa.h>

#include "nix.h"

struct udevice *rvu_af_dev;

inline struct rvu_af *get_af(void)
{
	return rvu_af_dev ? dev_get_priv(rvu_af_dev) : NULL;
}

void rvu_get_lfid_for_pf(int pf, int *nixid, int *npaid)
{
	union nixx_af_rvu_lf_cfg_debug nix_lf_dbg;
	union npa_af_rvu_lf_cfg_debug npa_lf_dbg;
	union rvu_pf_func_s pf_func;
	struct rvu_af *af = dev_get_priv(rvu_af_dev);
	struct nix_af *nix_af = af->nix_af;

	pf_func.u = 0;
	pf_func.s.pf = pf;

	nix_lf_dbg.u = 0;
	nix_lf_dbg.s.pf_func = pf_func.u & 0xFFFF;
	nix_lf_dbg.s.exec = 1;
	nix_af_reg_write(nix_af, NIXX_AF_RVU_LF_CFG_DEBUG(),
			 nix_lf_dbg.u);
	do {
		nix_lf_dbg.u = nix_af_reg_read(nix_af,
					       NIXX_AF_RVU_LF_CFG_DEBUG());
	} while (nix_lf_dbg.s.exec);

	if (nix_lf_dbg.s.lf_valid)
		*nixid = nix_lf_dbg.s.lf;

	debug("%s: nix lf_valid %d lf %d nixid %d\n", __func__,
	      nix_lf_dbg.s.lf_valid, nix_lf_dbg.s.lf, *nixid);

	npa_lf_dbg.u = 0;
	npa_lf_dbg.s.pf_func = pf_func.u & 0xFFFF;
	npa_lf_dbg.s.exec = 1;
	npa_af_reg_write(nix_af->npa_af, NPA_AF_RVU_LF_CFG_DEBUG(),
			 npa_lf_dbg.u);
	do {
		npa_lf_dbg.u = npa_af_reg_read(nix_af->npa_af,
					       NPA_AF_RVU_LF_CFG_DEBUG());
	} while (npa_lf_dbg.s.exec);

	if (npa_lf_dbg.s.lf_valid)
		*npaid = npa_lf_dbg.s.lf;
	debug("%s: npa lf_valid %d lf %d npaid %d\n", __func__,
	      npa_lf_dbg.s.lf_valid, npa_lf_dbg.s.lf, *npaid);
}

struct nix_af *rvu_af_init(struct rvu_af *rvu_af)
{
	struct nix_af *nix_af;
	union rvu_af_addr_s block_addr;
	int err;

	nix_af = (struct nix_af *)calloc(1, sizeof(struct nix_af));
	if (!nix_af) {
		printf("%s: out of memory\n", __func__);
		goto error;
	}

	nix_af->dev = rvu_af->dev;

	block_addr.u = 0;
	block_addr.s.block = RVU_BLOCK_ADDR_E_NIXX(0);
	nix_af->nix_af_base = rvu_af->af_base + block_addr.u;

	nix_af->npa_af = (struct npa_af *)calloc(1, sizeof(struct npa_af));
	if (!nix_af->npa_af) {
		printf("%s: out of memory\n", __func__);
		goto error;
	}

	block_addr.u = 0;
	block_addr.s.block = RVU_BLOCK_ADDR_E_NPA;
	nix_af->npa_af->npa_af_base = rvu_af->af_base + block_addr.u;

	block_addr.u = 0;
	block_addr.s.block = RVU_BLOCK_ADDR_E_NPC;
	nix_af->npc_af_base = rvu_af->af_base + block_addr.u;

	debug("%s: Setting up npa admin\n", __func__);
	err = npa_af_setup(nix_af->npa_af);
	if (err) {
		printf("%s: Error %d setting up NPA admin\n", __func__, err);
		goto error;
	}
	debug("%s: Setting up nix af\n", __func__);
	err = nix_af_setup(nix_af);
	if (err) {
		printf("%s: Error %d setting up NIX admin\n", __func__, err);
		goto error;
	}
	debug("%s: nix_af: %p\n", __func__, nix_af);
	return nix_af;

error:
	if (nix_af->npa_af) {
		free(nix_af->npa_af);
		memset(nix_af, 0, sizeof(*nix_af));
	}
	if (nix_af)
		free(nix_af);
	return NULL;
}

int rvu_af_probe(struct udevice *dev)
{
	struct rvu_af *af_ptr = dev_get_priv(dev);

	af_ptr->af_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE,
					 PCI_REGION_MEM);
	debug("%s RVU AF BAR %p\n", __func__, af_ptr->af_base);
	af_ptr->dev = dev;
	rvu_af_dev = dev;

	af_ptr->nix_af = rvu_af_init(af_ptr);
	if (!af_ptr->nix_af) {
		printf("%s: Error: could not initialize NIX AF\n", __func__);
		return -1;
	}
	debug("%s: Done\n", __func__);

	return 0;
}

int rvu_af_remove(struct udevice *dev)
{
	struct rvu_af *rvu_af = dev_get_priv(dev);

	nix_af_shutdown(rvu_af->nix_af);
	npa_af_shutdown(rvu_af->nix_af->npa_af);
	npc_af_shutdown(rvu_af->nix_af);

	debug("%s: rvu af down --\n", __func__);
	return 0;
}

U_BOOT_DRIVER(rvu_af) = {
	.name   = "rvu_af",
	.id     = UCLASS_MISC,
	.probe  = rvu_af_probe,
	.remove = rvu_af_remove,
	.priv_auto	= sizeof(struct rvu_af),
};

static struct pci_device_id rvu_af_supported[] = {
	{ PCI_VDEVICE(CAVIUM, PCI_DEVICE_ID_CAVIUM_RVU_AF) },
	{}
};

U_BOOT_PCI_DEVICE(rvu_af, rvu_af_supported);
