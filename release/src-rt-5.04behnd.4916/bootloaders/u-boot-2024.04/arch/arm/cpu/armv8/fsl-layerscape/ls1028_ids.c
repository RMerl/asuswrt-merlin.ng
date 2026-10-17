// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <fdt_support.h>
#include <log.h>
#include <asm/arch-fsl-layerscape/immap_lsch3.h>
#include <asm/arch-fsl-layerscape/fsl_icid.h>
#include <asm/arch-fsl-layerscape/fsl_portals.h>

struct icid_id_table icid_tbl[] = {
	SET_USB_ICID(1, "snps,dwc3", FSL_USB1_STREAM_ID),
	SET_USB_ICID(2, "snps,dwc3", FSL_USB2_STREAM_ID),
	SET_SDHC_ICID(1, FSL_SDMMC_STREAM_ID),
	SET_SDHC_ICID(2, FSL_SDMMC2_STREAM_ID),
	SET_SATA_ICID(1, "fsl,ls1028a-ahci", FSL_SATA1_STREAM_ID),
	SET_EDMA_ICID(FSL_EDMA_STREAM_ID),
	SET_QDMA_ICID("fsl,ls1028a-qdma", FSL_DMA_STREAM_ID),
	SET_GPU_ICID("vivante,gc", FSL_GPU_STREAM_ID),
	SET_DISPLAY_ICID(FSL_DISPLAY_STREAM_ID),
#ifdef CONFIG_FSL_CAAM
	SET_SEC_JR_ICID_ENTRY(0, FSL_SEC_JR1_STREAM_ID),
	SET_SEC_JR_ICID_ENTRY(1, FSL_SEC_JR2_STREAM_ID),
	SET_SEC_JR_ICID_ENTRY(2, FSL_SEC_JR3_STREAM_ID),
	SET_SEC_JR_ICID_ENTRY(3, FSL_SEC_JR4_STREAM_ID),
	SET_SEC_RTIC_ICID_ENTRY(0, FSL_SEC_STREAM_ID),
	SET_SEC_RTIC_ICID_ENTRY(1, FSL_SEC_STREAM_ID),
	SET_SEC_RTIC_ICID_ENTRY(2, FSL_SEC_STREAM_ID),
	SET_SEC_RTIC_ICID_ENTRY(3, FSL_SEC_STREAM_ID),
	SET_SEC_DECO_ICID_ENTRY(0, FSL_SEC_STREAM_ID),
	SET_SEC_DECO_ICID_ENTRY(1, FSL_SEC_STREAM_ID),
#endif
};

int icid_tbl_sz = ARRAY_SIZE(icid_tbl);

/* integrated PCI is handled separately as it's not part of CCSR/SCFG */
#ifdef CONFIG_PCIE_ECAM_GENERIC

#define ECAM_IERB_BASE		0x1f0800000ULL
#define ECAM_IERB_OFFSET_NA	-1
#define ECAM_IERB_FUNC_CNT	ARRAY_SIZE(ierb_offset)
/* cache related transaction attributes for PCIe functions */
#define ECAM_IERB_MSICAR		(ECAM_IERB_BASE + 0xa400)
#define ECAM_IERB_MSICAR_VALUE		0x30

/* offset of IERB config register per PCI function */
static int ierb_offset[] = {
	0x0800,
	0x1800,
	0x2800,
	0x3800,
	0x4800,
	0x5800,
	0x6800,
	ECAM_IERB_OFFSET_NA,
	0x0804,
	0x0808,
	0x1804,
	0x1808,
};

/*
 * Use a custom function for LS1028A, for now this is the only SoC with IERB
 * and we're currently considering reorganizing IERB for future SoCs.
 */
void set_ecam_icids(void)
{
	int i;

	out_le32(ECAM_IERB_MSICAR, ECAM_IERB_MSICAR_VALUE);

	for (i = 0; i < ECAM_IERB_FUNC_CNT; i++) {
		if (ierb_offset[i] == ECAM_IERB_OFFSET_NA)
			continue;

		out_le32(ECAM_IERB_BASE + ierb_offset[i],
			 FSL_ECAM_STREAM_ID_START + i);
	}
}

static int fdt_setprop_inplace_idx_u32(void *fdt, int nodeoffset,
				       const char *name, uint32_t idx, u32 val)
{
	val = cpu_to_be32(val);
	return fdt_setprop_inplace_namelen_partial(fdt, nodeoffset, name,
						   strlen(name),
						   idx * sizeof(val), &val,
						   sizeof(val));
}

static int fdt_getprop_len(void *fdt, int nodeoffset, const char *name)
{
	int len;

	if (fdt_getprop_namelen(fdt, nodeoffset, name, strlen(name), &len))
		return len;

	return 0;
}

void fdt_fixup_ecam(void *blob)
{
	int off;

	off = fdt_node_offset_by_compatible(blob, 0, "pci-host-ecam-generic");
	if (off < 0) {
		debug("ECAM node not found\n");
		return;
	}

	if (fdt_getprop_len(blob, off, "msi-map") != 16 ||
	    fdt_getprop_len(blob, off, "iommu-map") != 16) {
		log_err("invalid msi/iommu-map propertly size in ECAM node\n");
		return;
	}

	fdt_setprop_inplace_idx_u32(blob, off, "msi-map", 2,
				    FSL_ECAM_STREAM_ID_START);
	fdt_setprop_inplace_idx_u32(blob, off, "msi-map", 3,
				    ECAM_IERB_FUNC_CNT);

	fdt_setprop_inplace_idx_u32(blob, off, "iommu-map", 2,
				    FSL_ECAM_STREAM_ID_START);
	fdt_setprop_inplace_idx_u32(blob, off, "iommu-map", 3,
				    ECAM_IERB_FUNC_CNT);
}
#endif /* CONFIG_PCIE_ECAM_GENERIC */
