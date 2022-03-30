/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2007,2009-2012 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_PCI_H_
#define __FSL_PCI_H_

#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <pci.h>

#define PEX_IP_BLK_REV_2_2	0x02080202
#define PEX_IP_BLK_REV_2_3	0x02080203
#define PEX_IP_BLK_REV_3_0	0x02080300

/* Freescale-specific PCI config registers */
#define FSL_PCI_PBFR		0x44

#define FSL_PCIE_CFG_RDY	0x4b0
#define FSL_PCIE_V3_CFG_RDY	0x1
#define FSL_PROG_IF_AGENT	0x1

#define PCI_LTSSM	0x404   /* PCIe Link Training, Status State Machine */
#define  PCI_LTSSM_L0	0x16    /* L0 state */

int fsl_setup_hose(struct pci_controller *hose, unsigned long addr);
int fsl_is_pci_agent(struct pci_controller *hose);
void fsl_pci_config_unlock(struct pci_controller *hose);
void ft_fsl_pci_setup(void *blob, const char *compat, unsigned long ctrl_addr);

/*
 * Common PCI/PCIE Register structure for mpc85xx and mpc86xx
 */

/*
 * PCI Translation Registers
 */
typedef struct pci_outbound_window {
	u32	potar;		/* 0x00 - Address */
	u32	potear;		/* 0x04 - Address Extended */
	u32	powbar;		/* 0x08 - Window Base Address */
	u32	res1;
	u32	powar;		/* 0x10 - Window Attributes */
#define POWAR_EN	0x80000000
#define POWAR_IO_READ	0x00080000
#define POWAR_MEM_READ	0x00040000
#define POWAR_IO_WRITE	0x00008000
#define POWAR_MEM_WRITE	0x00004000
	u32	res2[3];
} pot_t;

typedef struct pci_inbound_window {
	u32	pitar;		/* 0x00 - Address */
	u32	res1;
	u32	piwbar;		/* 0x08 - Window Base Address */
	u32	piwbear;	/* 0x0c - Window Base Address Extended */
	u32	piwar;		/* 0x10 - Window Attributes */
#define PIWAR_EN		0x80000000
#define PIWAR_PF		0x20000000
#define PIWAR_LOCAL		0x00f00000
#define PIWAR_READ_SNOOP	0x00050000
#define PIWAR_WRITE_SNOOP	0x00005000
	u32	res2[3];
} pit_t;

/* PCI/PCI Express Registers */
typedef struct ccsr_pci {
	u32	cfg_addr;	/* 0x000 - PCI Configuration Address Register */
	u32	cfg_data;	/* 0x004 - PCI Configuration Data Register */
	u32	int_ack;	/* 0x008 - PCI Interrupt Acknowledge Register */
	u32	out_comp_to;	/* 0x00C - PCI Outbound Completion Timeout Register */
	u32	out_conf_to;	/* 0x010 - PCI Configuration Timeout Register */
	u32	config;		/* 0x014 - PCIE CONFIG Register */
	u32	int_status;	/* 0x018 - PCIE interrupt status register */
	char	res2[4];
	u32	pme_msg_det;	/* 0x020 - PCIE PME & message detect register */
	u32	pme_msg_dis;	/* 0x024 - PCIE PME & message disable register */
	u32	pme_msg_int_en;	/* 0x028 - PCIE PME & message interrupt enable register */
	u32	pm_command;	/* 0x02c - PCIE PM Command register */
	char	res3[2188];	/*     (0x8bc - 0x30 = 2188) */
	u32	dbi_ro_wr_en;	/* 0x8bc - DBI read only write enable reg */
	char	res4[824];	/*     (0xbf8 - 0x8c0 = 824) */
	u32	block_rev1;	/* 0xbf8 - PCIE Block Revision register 1 */
	u32	block_rev2;	/* 0xbfc - PCIE Block Revision register 2 */

	pot_t	pot[5];		/* 0xc00 - 0xc9f Outbound ATMU's 0, 1, 2, 3, and 4 */
	u32	res5[24];
	pit_t	pmit;		/* 0xd00 - 0xd9c Inbound ATMU's MSI */
	u32	res6[24];
	pit_t	pit[4];		/* 0xd80 - 0xdff Inbound ATMU's 3, 2, 1 and 0 */

#define PIT3 0
#define PIT2 1
#define PIT1 2

#if 0
	u32	potar0;		/* 0xc00 - PCI Outbound Transaction Address Register 0 */
	u32	potear0;	/* 0xc04 - PCI Outbound Translation Extended Address Register 0 */
	char	res5[8];
	u32	powar0;		/* 0xc10 - PCI Outbound Window Attributes Register 0 */
	char	res6[12];
	u32	potar1;		/* 0xc20 - PCI Outbound Transaction Address Register 1 */
	u32	potear1;	/* 0xc24 - PCI Outbound Translation Extended Address Register 1 */
	u32	powbar1;	/* 0xc28 - PCI Outbound Window Base Address Register 1 */
	char	res7[4];
	u32	powar1;		/* 0xc30 - PCI Outbound Window Attributes Register 1 */
	char	res8[12];
	u32	potar2;		/* 0xc40 - PCI Outbound Transaction Address Register 2 */
	u32	potear2;	/* 0xc44 - PCI Outbound Translation Extended Address Register 2 */
	u32	powbar2;	/* 0xc48 - PCI Outbound Window Base Address Register 2 */
	char	res9[4];
	u32	powar2;		/* 0xc50 - PCI Outbound Window Attributes Register 2 */
	char	res10[12];
	u32	potar3;		/* 0xc60 - PCI Outbound Transaction Address Register 3 */
	u32	potear3;	/* 0xc64 - PCI Outbound Translation Extended Address Register 3 */
	u32	powbar3;	/* 0xc68 - PCI Outbound Window Base Address Register 3 */
	char	res11[4];
	u32	powar3;		/* 0xc70 - PCI Outbound Window Attributes Register 3 */
	char	res12[12];
	u32	potar4;		/* 0xc80 - PCI Outbound Transaction Address Register 4 */
	u32	potear4;	/* 0xc84 - PCI Outbound Translation Extended Address Register 4 */
	u32	powbar4;	/* 0xc88 - PCI Outbound Window Base Address Register 4 */
	char	res13[4];
	u32	powar4;		/* 0xc90 - PCI Outbound Window Attributes Register 4 */
	char	res14[268];
	u32	pitar3;		/* 0xda0 - PCI Inbound Translation Address Register 3 */
	char	res15[4];
	u32	piwbar3;	/* 0xda8 - PCI Inbound Window Base Address Register 3 */
	u32	piwbear3;	/* 0xdac - PCI Inbound Window Base Extended Address Register 3 */
	u32	piwar3;		/* 0xdb0 - PCI Inbound Window Attributes Register 3 */
	char	res16[12];
	u32	pitar2;		/* 0xdc0 - PCI Inbound Translation Address Register 2 */
	char	res17[4];
	u32	piwbar2;	/* 0xdc8 - PCI Inbound Window Base Address Register 2 */
	u32	piwbear2;	/* 0xdcc - PCI Inbound Window Base Extended Address Register 2 */
	u32	piwar2;		/* 0xdd0 - PCI Inbound Window Attributes Register 2 */
	char	res18[12];
	u32	pitar1;		/* 0xde0 - PCI Inbound Translation Address Register 1 */
	char	res19[4];
	u32	piwbar1;	/* 0xde8 - PCI Inbound Window Base Address Register 1 */
	char	res20[4];
	u32	piwar1;		/* 0xdf0 - PCI Inbound Window Attributes Register 1 */
	char	res21[12];
#endif
	u32	pedr;		/* 0xe00 - PCI Error Detect Register */
	u32	pecdr;		/* 0xe04 - PCI Error Capture Disable Register */
	u32	peer;		/* 0xe08 - PCI Error Interrupt Enable Register */
	u32	peattrcr;	/* 0xe0c - PCI Error Attributes Capture Register */
	u32	peaddrcr;	/* 0xe10 - PCI Error Address Capture Register */
/*	u32	perr_disr	 * 0xe10 - PCIE Erorr Disable Register */
	u32	peextaddrcr;	/* 0xe14 - PCI	Error Extended Address Capture Register */
	u32	pedlcr;		/* 0xe18 - PCI Error Data Low Capture Register */
	u32	pedhcr;		/* 0xe1c - PCI Error Error Data High Capture Register */
	u32	gas_timr;	/* 0xe20 - PCI Gasket Timer Register */
/*	u32	perr_cap_stat;	 * 0xe20 - PCIE Error Capture Status Register */
	char	res22[4];
	u32	perr_cap0;	/* 0xe28 - PCIE Error Capture Register 0 */
	u32	perr_cap1;	/* 0xe2c - PCIE Error Capture Register 1 */
	u32	perr_cap2;	/* 0xe30 - PCIE Error Capture Register 2 */
	u32	perr_cap3;	/* 0xe34 - PCIE Error Capture Register 3 */
	char	res23[200];
	u32	pdb_stat;	/* 0xf00 - PCIE Debug Status */
	char	res24[16];
	u32	pex_csr0;	/* 0xf14 - PEX Control/Status register 0*/
	u32	pex_csr1;	/* 0xf18 - PEX Control/Status register 1*/
	char	res25[228];
} ccsr_fsl_pci_t;
#define PCIE_CONFIG_PC	0x00020000
#define PCIE_CONFIG_OB_CK	0x00002000
#define PCIE_CONFIG_SAC	0x00000010
#define PCIE_CONFIG_SP	0x80000002
#define PCIE_CONFIG_SCC	0x80000001

struct fsl_pci_info {
	unsigned long regs;
	pci_addr_t mem_bus;
	phys_size_t mem_phys;
	pci_size_t mem_size;
	pci_addr_t io_bus;
	phys_size_t io_phys;
	pci_size_t io_size;
	enum law_trgt_if law;
	int pci_num;
};

void fsl_pci_init(struct pci_controller *hose, struct fsl_pci_info *pci_info);
int fsl_pci_init_port(struct fsl_pci_info *pci_info,
				struct pci_controller *hose, int busno);
int fsl_pcie_init_ctrl(int busno, u32 devdisr, enum srds_prtcl dev,
			struct fsl_pci_info *pci_info);
int fsl_pcie_init_board(int busno);

#define SET_STD_PCI_INFO(x, num) \
{			\
	x.regs = CONFIG_SYS_PCI##num##_ADDR;	\
	x.mem_bus = CONFIG_SYS_PCI##num##_MEM_BUS; \
	x.mem_phys = CONFIG_SYS_PCI##num##_MEM_PHYS; \
	x.mem_size = CONFIG_SYS_PCI##num##_MEM_SIZE; \
	x.io_bus = CONFIG_SYS_PCI##num##_IO_BUS; \
	x.io_phys = CONFIG_SYS_PCI##num##_IO_PHYS; \
	x.io_size = CONFIG_SYS_PCI##num##_IO_SIZE; \
	x.law = LAW_TRGT_IF_PCI_##num; \
	x.pci_num = num; \
}

#define SET_STD_PCIE_INFO(x, num) \
{			\
	x.regs = CONFIG_SYS_PCIE##num##_ADDR;	\
	x.mem_bus = CONFIG_SYS_PCIE##num##_MEM_BUS; \
	x.mem_phys = CONFIG_SYS_PCIE##num##_MEM_PHYS; \
	x.mem_size = CONFIG_SYS_PCIE##num##_MEM_SIZE; \
	x.io_bus = CONFIG_SYS_PCIE##num##_IO_BUS; \
	x.io_phys = CONFIG_SYS_PCIE##num##_IO_PHYS; \
	x.io_size = CONFIG_SYS_PCIE##num##_IO_SIZE; \
	x.law = LAW_TRGT_IF_PCIE_##num; \
	x.pci_num = num; \
}

#define __FT_FSL_PCI_SETUP(blob, compat, num) \
	ft_fsl_pci_setup(blob, compat, CONFIG_SYS_PCI##num##_ADDR)

#define __FT_FSL_PCIE_SETUP(blob, compat, num) \
	ft_fsl_pci_setup(blob, compat, CONFIG_SYS_PCIE##num##_ADDR)

#define FT_FSL_PCI1_SETUP __FT_FSL_PCI_SETUP(blob, FSL_PCI_COMPAT, 1)
#define FT_FSL_PCI2_SETUP __FT_FSL_PCI_SETUP(blob, FSL_PCI_COMPAT, 2)

#define FT_FSL_PCIE1_SETUP __FT_FSL_PCIE_SETUP(blob, FSL_PCIE_COMPAT, 1)
#define FT_FSL_PCIE2_SETUP __FT_FSL_PCIE_SETUP(blob, FSL_PCIE_COMPAT, 2)
#define FT_FSL_PCIE3_SETUP __FT_FSL_PCIE_SETUP(blob, FSL_PCIE_COMPAT, 3)
#define FT_FSL_PCIE4_SETUP __FT_FSL_PCIE_SETUP(blob, FSL_PCIE_COMPAT, 4)

#if !defined(CONFIG_PCI)
#define FT_FSL_PCI_SETUP
#elif defined(CONFIG_FSL_CORENET)
#define FSL_PCIE_COMPAT	CONFIG_SYS_FSL_PCIE_COMPAT
#define FT_FSL_PCI_SETUP \
	FT_FSL_PCIE1_SETUP; \
	FT_FSL_PCIE2_SETUP; \
	FT_FSL_PCIE3_SETUP; \
	FT_FSL_PCIE4_SETUP;
#define FT_FSL_PCIE_SETUP FT_FSL_PCI_SETUP
#elif defined(CONFIG_MPC85xx)
#define FSL_PCI_COMPAT	"fsl,mpc8540-pci"
#ifdef CONFIG_SYS_FSL_PCIE_COMPAT
#define FSL_PCIE_COMPAT	CONFIG_SYS_FSL_PCIE_COMPAT
#else
#define FSL_PCIE_COMPAT	"fsl,mpc8548-pcie"
#endif
#define FT_FSL_PCI_SETUP \
	FT_FSL_PCI1_SETUP; \
	FT_FSL_PCI2_SETUP; \
	FT_FSL_PCIE1_SETUP; \
	FT_FSL_PCIE2_SETUP; \
	FT_FSL_PCIE3_SETUP;
#define FT_FSL_PCIE_SETUP \
	FT_FSL_PCIE1_SETUP; \
	FT_FSL_PCIE2_SETUP; \
	FT_FSL_PCIE3_SETUP;
#elif defined(CONFIG_MPC86xx)
#define FSL_PCI_COMPAT	"fsl,mpc8610-pci"
#define FSL_PCIE_COMPAT	"fsl,mpc8641-pcie"
#define FT_FSL_PCI_SETUP \
	FT_FSL_PCI1_SETUP; \
	FT_FSL_PCIE1_SETUP; \
	FT_FSL_PCIE2_SETUP;
#else
#error FT_FSL_PCI_SETUP not defined
#endif


#endif
