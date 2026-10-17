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
#ifndef CONFIG_SPL_BUILD
	const char *compat;
	phys_addr_t compat_addr;
#endif
	phys_addr_t reg_addr;
	u32 reg;
#ifndef CONFIG_SPL_BUILD
	u32 id;
#endif
	bool le;
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

#ifdef CONFIG_SPL_BUILD
#define SET_ICID_ENTRY(name, idA, regA, addr, compataddr, _le) \
	{ .reg = regA, \
	  .reg_addr = addr, \
	  .le = _le \
	}
#else
#define SET_ICID_ENTRY(name, idA, regA, addr, compataddr, _le) \
	{ .compat = name, \
	  .id = idA, \
	  .reg = regA, \
	  .compat_addr = compataddr, \
	  .reg_addr = addr, \
	  .le = _le \
	}
#endif

#ifdef CONFIG_SYS_FSL_SEC_LE
#define SEC_IS_LE true
#elif defined(CONFIG_SYS_FSL_SEC_BE)
#define SEC_IS_LE false
#endif

#ifdef CONFIG_FSL_LSCH2

#ifdef CONFIG_SYS_FSL_CCSR_SCFG_LE
#define SCFG_IS_LE true
#elif defined(CONFIG_SYS_FSL_CCSR_SCFG_BE)
#define SCFG_IS_LE false
#endif

#define QDMA_IS_LE false

#define SET_SCFG_ICID(compat, streamid, name, compataddr) \
	SET_ICID_ENTRY(compat, streamid, (((streamid) << 24) | (1 << 23)), \
		offsetof(struct ccsr_scfg, name) + CFG_SYS_FSL_SCFG_ADDR, \
		compataddr, SCFG_IS_LE)

#define SET_USB_ICID(usb_num, compat, streamid) \
	SET_SCFG_ICID(compat, streamid, usb##usb_num##_icid,\
		CFG_SYS_XHCI_USB##usb_num##_ADDR)

#define SET_SATA_ICID(compat, streamid) \
	SET_SCFG_ICID(compat, streamid, sata_icid,\
		AHCI_BASE_ADDR)

#define SET_SDHC_ICID(streamid) \
	SET_SCFG_ICID("fsl,esdhc", streamid, sdhc_icid,\
		CFG_SYS_FSL_ESDHC_ADDR)

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
		CFG_SYS_FSL_QMAN_ADDR, \
		CFG_SYS_FSL_QMAN_ADDR, false)

#define SET_BMAN_ICID(streamid) \
	SET_ICID_ENTRY("fsl,bman", streamid, streamid, \
		offsetof(struct ccsr_bman, liodnr) + \
		CFG_SYS_FSL_BMAN_ADDR, \
		CFG_SYS_FSL_BMAN_ADDR, false)

#define SET_FMAN_ICID_ENTRY(_port_id, streamid) \
	{ .port_id = (_port_id), .icid = (streamid) }

#define SEC_ICID_REG_VAL(streamid) (((streamid) << 16) | (streamid))

#define SET_SEC_QI_ICID(streamid) \
	SET_ICID_ENTRY("fsl,sec-v4.0", streamid, \
		0, offsetof(ccsr_sec_t, qilcr_ls) + \
		CFG_SYS_FSL_SEC_ADDR, \
		CFG_SYS_FSL_SEC_ADDR, SEC_IS_LE)

extern struct fman_icid_id_table fman_icid_tbl[];
extern int fman_icid_tbl_sz;

#else /* CONFIG_FSL_LSCH2 */

#ifdef CONFIG_SYS_FSL_CCSR_GUR_LE
#define GUR_IS_LE true
#elif defined(CONFIG_SYS_FSL_CCSR_GUR_BE)
#define GUR_IS_LE false
#endif

#define QDMA_IS_LE true

#define SET_GUR_ICID(compat, streamid, name, compataddr) \
	SET_ICID_ENTRY(compat, streamid, streamid, \
		offsetof(struct ccsr_gur, name) + CFG_SYS_FSL_GUTS_ADDR, \
		compataddr, GUR_IS_LE)

#define SET_USB_ICID(usb_num, compat, streamid) \
	SET_GUR_ICID(compat, streamid, usb##usb_num##_amqr,\
		CFG_SYS_XHCI_USB##usb_num##_ADDR)

#define SET_SATA_ICID(sata_num, compat, streamid) \
	SET_GUR_ICID(compat, streamid, sata##sata_num##_amqr, \
		AHCI_BASE_ADDR##sata_num)

#define SET_SDHC_ICID(sdhc_num, streamid) \
	SET_GUR_ICID("fsl,esdhc", streamid, sdmm##sdhc_num##_amqr,\
		FSL_ESDHC##sdhc_num##_BASE_ADDR)

#define SET_EDMA_ICID(streamid) \
	SET_GUR_ICID("fsl,vf610-edma", streamid, spare3_amqr,\
		EDMA_BASE_ADDR)

#define SET_GPU_ICID(compat, streamid) \
	SET_GUR_ICID(compat, streamid, misc1_amqr,\
		GPU_BASE_ADDR)

#define SET_DISPLAY_ICID(streamid) \
	SET_GUR_ICID("arm,mali-dp500", streamid, spare2_amqr,\
		DISPLAY_BASE_ADDR)

#define SEC_ICID_REG_VAL(streamid) (streamid)

#endif /* CONFIG_FSL_LSCH2 */

#define SET_QDMA_ICID(compat, streamid) \
	SET_ICID_ENTRY(compat, streamid, (1 << 31) | (streamid), \
		QDMA_BASE_ADDR + QMAN_CQSIDR_REG, \
		QDMA_BASE_ADDR, QDMA_IS_LE), \
	SET_ICID_ENTRY(NULL, streamid, (1 << 31) | (streamid), \
		QDMA_BASE_ADDR + QMAN_CQSIDR_REG + 4, \
		QDMA_BASE_ADDR, QDMA_IS_LE)

#define SET_SEC_JR_ICID_ENTRY(jr_num, streamid) \
	SET_ICID_ENTRY( \
		(CONFIG_IS_ENABLED(ARMV8_SEC_FIRMWARE_SUPPORT) && \
		(FSL_SEC_JR##jr_num##_OFFSET ==  \
			SEC_JR3_OFFSET + CFG_SYS_FSL_SEC_OFFSET) \
			? NULL \
			: "fsl,sec-v4.0-job-ring"), \
		streamid, \
		SEC_ICID_REG_VAL(streamid), \
		offsetof(ccsr_sec_t, jrliodnr[jr_num].ls) + \
		CFG_SYS_FSL_SEC_ADDR, \
		FSL_SEC_JR##jr_num##_BASE_ADDR, SEC_IS_LE)

#define SET_SEC_DECO_ICID_ENTRY(deco_num, streamid) \
	SET_ICID_ENTRY(NULL, streamid, SEC_ICID_REG_VAL(streamid), \
		offsetof(ccsr_sec_t, decoliodnr[deco_num].ls) + \
		CFG_SYS_FSL_SEC_ADDR, 0, SEC_IS_LE)

#define SET_SEC_RTIC_ICID_ENTRY(rtic_num, streamid) \
	SET_ICID_ENTRY(NULL, streamid, SEC_ICID_REG_VAL(streamid), \
		offsetof(ccsr_sec_t, rticliodnr[rtic_num].ls) + \
		CFG_SYS_FSL_SEC_ADDR, 0, SEC_IS_LE)

extern struct icid_id_table icid_tbl[];
extern int icid_tbl_sz;

#endif
