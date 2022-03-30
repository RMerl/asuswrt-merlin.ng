/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 */

#ifndef _FSL_ICID_H_
#define _FSL_ICID_H_

#include <asm/types.h>
#include <fsl_qbman.h>
#include <fsl_sec.h>
#include <asm/armv8/sec_firmware.h>

struct icid_id_table {
	const char *compat;
	u32 id;
	u32 reg;
	phys_addr_t compat_addr;
	phys_addr_t reg_addr;
};

struct fman_icid_id_table {
	u32 port_id;
	u32 icid;
};

u32 get_ppid_icid(int ppid_tbl_idx, int ppid);
int fdt_get_smmu_phandle(void *blob);
int fdt_set_iommu_prop(void *blob, int off, int smmu_ph, u32 *ids, int num_ids);
void set_icids(void);
void fdt_fixup_icid(void *blob);

#define SET_ICID_ENTRY(name, idA, regA, addr, compataddr) \
	{ .compat = name, \
	  .id = idA, \
	  .reg = regA, \
	  .compat_addr = compataddr, \
	  .reg_addr = addr, \
	}

#define SET_SCFG_ICID(compat, streamid, name, compataddr) \
	SET_ICID_ENTRY(compat, streamid, (((streamid) << 24) | (1 << 23)), \
		offsetof(struct ccsr_scfg, name) + CONFIG_SYS_FSL_SCFG_ADDR, \
		compataddr)

#define SET_USB_ICID(usb_num, compat, streamid) \
	SET_SCFG_ICID(compat, streamid, usb##usb_num##_icid,\
		CONFIG_SYS_XHCI_USB##usb_num##_ADDR)

#define SET_SATA_ICID(compat, streamid) \
	SET_SCFG_ICID(compat, streamid, sata_icid,\
		AHCI_BASE_ADDR)

#define SET_SDHC_ICID(streamid) \
	SET_SCFG_ICID("fsl,esdhc", streamid, sdhc_icid,\
		CONFIG_SYS_FSL_ESDHC_ADDR)

#define SET_QDMA_ICID(compat, streamid) \
	SET_ICID_ENTRY(compat, streamid, (1 << 31) | (streamid), \
		QDMA_BASE_ADDR + QMAN_CQSIDR_REG, \
		QDMA_BASE_ADDR), \
	SET_ICID_ENTRY(NULL, streamid, (1 << 31) | (streamid), \
		QDMA_BASE_ADDR + QMAN_CQSIDR_REG + 4, \
		QDMA_BASE_ADDR)

#define SET_EDMA_ICID(streamid) \
	SET_SCFG_ICID("fsl,vf610-edma", streamid, edma_icid,\
		EDMA_BASE_ADDR)

#define SET_ETR_ICID(streamid) \
	SET_SCFG_ICID(NULL, streamid, etr_icid, 0)

#define SET_DEBUG_ICID(streamid) \
	SET_SCFG_ICID(NULL, streamid, debug_icid, 0)

#define SET_QE_ICID(streamid) \
	SET_SCFG_ICID("fsl,qe", streamid, qe_icid,\
		QE_BASE_ADDR)

#define SET_QMAN_ICID(streamid) \
	SET_ICID_ENTRY("fsl,qman", streamid, streamid, \
		offsetof(struct ccsr_qman, liodnr) + \
		CONFIG_SYS_FSL_QMAN_ADDR, \
		CONFIG_SYS_FSL_QMAN_ADDR)

#define SET_BMAN_ICID(streamid) \
	SET_ICID_ENTRY("fsl,bman", streamid, streamid, \
		offsetof(struct ccsr_bman, liodnr) + \
		CONFIG_SYS_FSL_BMAN_ADDR, \
		CONFIG_SYS_FSL_BMAN_ADDR)

#define SET_FMAN_ICID_ENTRY(_port_id, streamid) \
	{ .port_id = (_port_id), .icid = (streamid) }

#define SET_SEC_QI_ICID(streamid) \
	SET_ICID_ENTRY("fsl,sec-v4.0", streamid, \
		0, offsetof(ccsr_sec_t, qilcr_ls) + \
		CONFIG_SYS_FSL_SEC_ADDR, \
		CONFIG_SYS_FSL_SEC_ADDR)

#define SET_SEC_JR_ICID_ENTRY(jr_num, streamid) \
	SET_ICID_ENTRY( \
		(CONFIG_ARMV8_SEC_FIRMWARE_SUPPORT && \
		(FSL_SEC_JR##jr_num##_OFFSET ==  \
			SEC_JR3_OFFSET + CONFIG_SYS_FSL_SEC_OFFSET) \
			? NULL \
			: "fsl,sec-v4.0-job-ring"), \
		streamid, \
		(((streamid) << 16) | (streamid)), \
		offsetof(ccsr_sec_t, jrliodnr[jr_num].ls) + \
		CONFIG_SYS_FSL_SEC_ADDR, \
		FSL_SEC_JR##jr_num##_BASE_ADDR)

#define SET_SEC_DECO_ICID_ENTRY(deco_num, streamid) \
	SET_ICID_ENTRY(NULL, streamid, (((streamid) << 16) | (streamid)), \
		offsetof(ccsr_sec_t, decoliodnr[deco_num].ls) + \
		CONFIG_SYS_FSL_SEC_ADDR, 0)

#define SET_SEC_RTIC_ICID_ENTRY(rtic_num, streamid) \
	SET_ICID_ENTRY(NULL, streamid, (((streamid) << 16) | (streamid)), \
		offsetof(ccsr_sec_t, rticliodnr[rtic_num].ls) + \
		CONFIG_SYS_FSL_SEC_ADDR, 0)

extern struct icid_id_table icid_tbl[];
extern struct fman_icid_id_table fman_icid_tbl[];
extern int icid_tbl_sz;
extern int fman_icid_tbl_sz;

#endif
