/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2012-2016 Freescale Semiconductor, Inc.
 */

#ifndef __PAMU_H
#define __PAMU_H

#define CONFIG_NUM_PAMU		16
#define NUM_PPAACT_ENTRIES	512
#define NUM_SPAACT_ENTRIES	256

/* PAMU_OFFSET to the next pamu space in ccsr */
#define PAMU_OFFSET 0x1000

#define PAMU_TABLE_ALIGNMENT 0x00001000

#define PAMU_PAGE_SHIFT 12
#define PAMU_PAGE_SIZE  4096U

#define PAACE_M_COHERENCE_REQ   0x01

#define PAACE_DA_HOST_CR                0x80
#define PAACE_DA_HOST_CR_SHIFT          7

#define PAACE_AF_PT                     0x00000002
#define PAACE_AF_PT_SHIFT               1

#define PAACE_PT_PRIMARY       0x0
#define PAACE_PT_SECONDARY     0x1

#define PPAACE_AF_WBAL			0xfffff000
#define PPAACE_AF_WBAL_SHIFT		12

#define	OME_NUMBER_ENTRIES      16   /* based on P4080 2.0 silicon plan */

#define PAACE_IA_CID			0x00FF0000
#define PAACE_IA_CID_SHIFT		16
#define PAACE_IA_WCE			0x000000F0
#define PAACE_IA_WCE_SHIFT		4
#define PAACE_IA_ATM			0x0000000C
#define PAACE_IA_ATM_SHIFT		2
#define PAACE_IA_OTM			0x00000003
#define PAACE_IA_OTM_SHIFT		0

#define PAACE_OTM_NO_XLATE      0x00
#define PAACE_OTM_IMMEDIATE     0x01
#define PAACE_OTM_INDEXED       0x02
#define PAACE_OTM_RESERVED      0x03
#define PAACE_ATM_NO_XLATE      0x00
#define PAACE_ATM_WINDOW_XLATE  0x01
#define PAACE_ATM_PAGE_XLATE    0x02
#define PAACE_ATM_WIN_PG_XLATE  \
	(PAACE_ATM_WINDOW_XLATE | PAACE_ATM_PAGE_XLATE)
#define PAACE_WIN_TWBAL			0xfffff000
#define PAACE_WIN_TWBAL_SHIFT		12
#define PAACE_WIN_SWSE			0x00000fc0
#define PAACE_WIN_SWSE_SHIFT		6

#define PAACE_AF_AP			0x00000018
#define PAACE_AF_AP_SHIFT		3
#define PAACE_AF_DD			0x00000004
#define PAACE_AF_DD_SHIFT		2
#define PAACE_AF_PT			0x00000002
#define PAACE_AF_PT_SHIFT		1
#define PAACE_AF_V			0x00000001
#define PAACE_AF_V_SHIFT		0
#define PPAACE_AF_WSE			0x00000fc0
#define PPAACE_AF_WSE_SHIFT		6
#define PPAACE_AF_MW			0x00000020
#define PPAACE_AF_MW_SHIFT		5

#define PAACE_AP_PERMS_DENIED  0x0
#define PAACE_AP_PERMS_QUERY   0x1
#define PAACE_AP_PERMS_UPDATE  0x2
#define PAACE_AP_PERMS_ALL     0x3

#define SPAACE_AF_LIODN			0xffff0000
#define SPAACE_AF_LIODN_SHIFT		16
#define PAACE_V_VALID          0x1

#define set_bf(v, m, x)             (v = ((v) & ~(m)) | (((x) << \
					(m##_SHIFT)) & (m)))
#define get_bf(v, m)            (((v) & (m)) >> (m##_SHIFT))

#define DEFAULT_NUM_SUBWINDOWS		128
#define PAMU_PCR_OFFSET 0xc10
#define PAMU_PCR_PE	0x40000000

struct pamu_addr_tbl {
	phys_addr_t start_addr[10];
	phys_addr_t end_addr[10];
	phys_size_t size[10];
};

struct paace {
	/* PAACE Offset 0x00 */
	uint32_t wbah;			/* only valid for Primary PAACE */
	uint32_t addr_bitfields;	/* See P/S PAACE_AF_* */

	/* PAACE Offset 0x08 */
	/* Interpretation of first 32 bits dependent on DD above */
	union {
		struct {
			/* Destination ID, see PAACE_DID_* defines */
			uint8_t did;
			/* Partition ID */
			uint8_t pid;
			/* Snoop ID */
			uint8_t snpid;
			/* coherency_required : 1 reserved : 7 */
			uint8_t coherency_required; /* See PAACE_DA_* */
		} to_host;
		struct {
			/* Destination ID, see PAACE_DID_* defines */
			uint8_t  did;
			uint8_t  reserved1;
			uint16_t reserved2;
		} to_io;
	} domain_attr;

	/* Implementation attributes + window count + address & operation
	 * translation modes
	 */
	uint32_t impl_attr;			/* See PAACE_IA_* */

	/* PAACE Offset 0x10 */
	/* Translated window base address */
	uint32_t twbah;
	uint32_t win_bitfields;			/* See PAACE_WIN_* */

	/* PAACE Offset 0x18 */
	/* first secondary paace entry */
	uint32_t fspi;			/* only valid for Primary PAACE */
	union {
		struct {
			uint8_t ioea;
			uint8_t moea;
			uint8_t ioeb;
			uint8_t moeb;
		} immed_ot;
		struct {
			uint16_t reserved;
			uint16_t omi;
		} index_ot;
	} op_encode;

	/* PAACE Offset 0x20 */
	uint32_t reserved1[2];			/* not currently implemented */

	/* PAACE Offset 0x28 */
	uint32_t reserved2[2];			/* not currently implemented */

	/* PAACE Offset 0x30 */
	uint32_t reserved3[2];			/* not currently implemented */

	/* PAACE Offset 0x38 */
	uint32_t reserved4[2];			/* not currently implemented */

};

int pamu_init(void);
void pamu_enable(void);
void pamu_disable(void);
int config_pamu(struct pamu_addr_tbl *tbl, int num_entries, uint32_t liodn);
int sec_config_pamu_table(uint32_t liodn_ns, uint32_t liodn_s);

#endif
