/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 */

#include <config.h>
#include <gt64120.h>
#include <msc01.h>
#include <pci.h>

#include <asm/addrspace.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include <asm/malta.h>
#include <asm/mipsregs.h>

#ifdef CONFIG_SYS_BIG_ENDIAN
#define CPU_TO_GT32(_x)		((_x))
#else
#define CPU_TO_GT32(_x) (					\
	(((_x) & 0xff) << 24) | (((_x) & 0xff00) << 8) |	\
	(((_x) & 0xff0000) >> 8) | (((_x) & 0xff000000) >> 24))
#endif

	.text
	.set noreorder

	.globl	lowlevel_init
lowlevel_init:
	/* detect the core card */
	PTR_LI	t0, CKSEG1ADDR(MALTA_REVISION)
	lw	t0, 0(t0)
	srl	t0, t0, MALTA_REVISION_CORID_SHF
	andi	t0, t0, (MALTA_REVISION_CORID_MSK >> \
			 MALTA_REVISION_CORID_SHF)

	/* core cards using the gt64120 system controller */
	li	t1, MALTA_REVISION_CORID_CORE_LV
	beq	t0, t1, _gt64120

	/* core cards using the MSC01 system controller */
	 li	t1, MALTA_REVISION_CORID_CORE_FPGA6
	beq	t0, t1, _msc01
	 nop

	/* unknown system controller */
	b	.
	 nop

	/*
	 * Load BAR registers of GT64120 as done by YAMON
	 *
	 * based on a patch sent by Antony Pavlov <antonynpavlov@gmail.com>
	 * to the barebox mailing list.
	 * The subject of the original patch:
	 *   'MIPS: qemu-malta: add YAMON-style GT64120 memory map'
	 * URL:
	 * http://www.mail-archive.com/barebox@lists.infradead.org/msg06128.html
	 *
	 * based on write_bootloader() in qemu.git/hw/mips_malta.c
	 * see GT64120 manual and qemu.git/hw/gt64xxx.c for details
	 */
_gt64120:
	/* move GT64120 registers from 0x14000000 to 0x1be00000 */
	PTR_LI	t1, CKSEG1ADDR(GT_DEF_BASE)
	li	t0, CPU_TO_GT32(0xdf000000)
	sw	t0, GT_ISD_OFS(t1)

	/* setup MEM-to-PCI0 mapping */
	PTR_LI	t1, CKSEG1ADDR(MALTA_GT_BASE)

	/* setup PCI0 io window to 0x18000000-0x181fffff */
	li	t0, CPU_TO_GT32(0xc0000000)
	sw	t0, GT_PCI0IOLD_OFS(t1)
	li	t0, CPU_TO_GT32(0x40000000)
	sw	t0, GT_PCI0IOHD_OFS(t1)

	/* setup PCI0 mem windows */
	li	t0, CPU_TO_GT32(0x80000000)
	sw	t0, GT_PCI0M0LD_OFS(t1)
	li	t0, CPU_TO_GT32(0x3f000000)
	sw	t0, GT_PCI0M0HD_OFS(t1)

	li	t0, CPU_TO_GT32(0xc1000000)
	sw	t0, GT_PCI0M1LD_OFS(t1)
	li	t0, CPU_TO_GT32(0x5e000000)
	sw	t0, GT_PCI0M1HD_OFS(t1)

	jr	ra
	 nop

	/*
	 *
	 */
_msc01:
	/* setup peripheral bus controller clock divide */
	PTR_LI	t0, CKSEG1ADDR(MALTA_MSC01_PBC_BASE)
	li	t1, 0x1 << MSC01_PBC_CLKCFG_SHF
	sw	t1, MSC01_PBC_CLKCFG_OFS(t0)

	/* tweak peripheral bus controller timings */
	li	t1, (0x1 << MSC01_PBC_CS0TIM_CDT_SHF) | \
		    (0x1 << MSC01_PBC_CS0TIM_CAT_SHF)
	sw	t1, MSC01_PBC_CS0TIM_OFS(t0)
	li	t1, (0x0 << MSC01_PBC_CS0RW_RDT_SHF) | \
		    (0x2 << MSC01_PBC_CS0RW_RAT_SHF) | \
		    (0x0 << MSC01_PBC_CS0RW_WDT_SHF) | \
		    (0x2 << MSC01_PBC_CS0RW_WAT_SHF)
	sw	t1, MSC01_PBC_CS0RW_OFS(t0)
	lw	t1, MSC01_PBC_CS0CFG_OFS(t0)
	li	t2, MSC01_PBC_CS0CFG_DTYP_MSK
	and	t1, t2
	ori	t1, (0x0 << MSC01_PBC_CS0CFG_ADM_SHF) | \
		    (0x3 << MSC01_PBC_CS0CFG_WSIDLE_SHF) | \
		    (0x10 << MSC01_PBC_CS0CFG_WS_SHF)
	sw	t1, MSC01_PBC_CS0CFG_OFS(t0)

	/* setup basic address decode */
	PTR_LI	t0, CKSEG1ADDR(MALTA_MSC01_BIU_BASE)
	li	t1, 0x0
	li	t2, -CONFIG_SYS_MEM_SIZE
	sw	t1, MSC01_BIU_MCBAS1L_OFS(t0)
	sw	t2, MSC01_BIU_MCMSK1L_OFS(t0)
	sw	t1, MSC01_BIU_MCBAS2L_OFS(t0)
	sw	t2, MSC01_BIU_MCMSK2L_OFS(t0)

	/* initialise IP1 - unused */
	li	t1, MALTA_MSC01_IP1_BASE
	li	t2, -MALTA_MSC01_IP1_SIZE
	sw	t1, MSC01_BIU_IP1BAS1L_OFS(t0)
	sw	t2, MSC01_BIU_IP1MSK1L_OFS(t0)
	sw	t1, MSC01_BIU_IP1BAS2L_OFS(t0)
	sw	t2, MSC01_BIU_IP1MSK2L_OFS(t0)

	/* initialise IP2 - PCI */
	li	t1, MALTA_MSC01_IP2_BASE1
	li	t2, -MALTA_MSC01_IP2_SIZE1
	sw	t1, MSC01_BIU_IP2BAS1L_OFS(t0)
	sw	t2, MSC01_BIU_IP2MSK1L_OFS(t0)
	li	t1, MALTA_MSC01_IP2_BASE2
	li	t2, -MALTA_MSC01_IP2_SIZE2
	sw	t1, MSC01_BIU_IP2BAS2L_OFS(t0)
	sw	t2, MSC01_BIU_IP2MSK2L_OFS(t0)

	/* initialise IP3 - peripheral bus controller */
	li	t1, MALTA_MSC01_IP3_BASE
	li	t2, -MALTA_MSC01_IP3_SIZE
	sw	t1, MSC01_BIU_IP3BAS1L_OFS(t0)
	sw	t2, MSC01_BIU_IP3MSK1L_OFS(t0)
	sw	t1, MSC01_BIU_IP3BAS2L_OFS(t0)
	sw	t2, MSC01_BIU_IP3MSK2L_OFS(t0)

	/* setup PCI memory */
	PTR_LI	t0, CKSEG1ADDR(MALTA_MSC01_PCI_BASE)
	li	t1, MALTA_MSC01_PCIMEM_BASE
	li	t2, (-MALTA_MSC01_PCIMEM_SIZE) & MSC01_PCI_SC2PMMSKL_MSK_MSK
	li	t3, MALTA_MSC01_PCIMEM_MAP
	sw	t1, MSC01_PCI_SC2PMBASL_OFS(t0)
	sw	t2, MSC01_PCI_SC2PMMSKL_OFS(t0)
	sw	t3, MSC01_PCI_SC2PMMAPL_OFS(t0)

	/* setup PCI I/O */
	li	t1, MALTA_MSC01_PCIIO_BASE
	li	t2, (-MALTA_MSC01_PCIIO_SIZE) & MSC01_PCI_SC2PIOMSKL_MSK_MSK
	li	t3, MALTA_MSC01_PCIIO_MAP
	sw	t1, MSC01_PCI_SC2PIOBASL_OFS(t0)
	sw	t2, MSC01_PCI_SC2PIOMSKL_OFS(t0)
	sw	t3, MSC01_PCI_SC2PIOMAPL_OFS(t0)

	/* setup PCI_BAR0 memory window */
	li	t1, -CONFIG_SYS_MEM_SIZE
	sw	t1, MSC01_PCI_BAR0_OFS(t0)

	/* setup PCI to SysCon/CPU translation */
	sw	t1, MSC01_PCI_P2SCMSKL_OFS(t0)
	sw	zero, MSC01_PCI_P2SCMAPL_OFS(t0)

	/* setup PCI vendor & device IDs */
	li	t1, (PCI_VENDOR_ID_MIPS << MSC01_PCI_HEAD0_VENDORID_SHF) | \
		    (PCI_DEVICE_ID_MIPS_MSC01 << MSC01_PCI_HEAD0_DEVICEID_SHF)
	sw	t1, MSC01_PCI_HEAD0_OFS(t0)

	/* setup PCI subsystem vendor & device IDs */
	sw	t1, MSC01_PCI_HEAD11_OFS(t0)

	/* setup PCI class, revision */
	li	t1, (PCI_CLASS_BRIDGE_HOST << MSC01_PCI_HEAD2_CLASS_SHF) | \
		    (0x1 << MSC01_PCI_HEAD2_REV_SHF)
	sw	t1, MSC01_PCI_HEAD2_OFS(t0)

	/* ensure a sane setup */
	sw	zero, MSC01_PCI_HEAD3_OFS(t0)
	sw	zero, MSC01_PCI_HEAD4_OFS(t0)
	sw	zero, MSC01_PCI_HEAD5_OFS(t0)
	sw	zero, MSC01_PCI_HEAD6_OFS(t0)
	sw	zero, MSC01_PCI_HEAD7_OFS(t0)
	sw	zero, MSC01_PCI_HEAD8_OFS(t0)
	sw	zero, MSC01_PCI_HEAD9_OFS(t0)
	sw	zero, MSC01_PCI_HEAD10_OFS(t0)
	sw	zero, MSC01_PCI_HEAD12_OFS(t0)
	sw	zero, MSC01_PCI_HEAD13_OFS(t0)
	sw	zero, MSC01_PCI_HEAD14_OFS(t0)
	sw	zero, MSC01_PCI_HEAD15_OFS(t0)

	/* setup PCI command register */
	li	t1, (PCI_COMMAND_FAST_BACK | \
		     PCI_COMMAND_SERR | \
		     PCI_COMMAND_PARITY | \
		     PCI_COMMAND_MASTER | \
		     PCI_COMMAND_MEMORY)
	sw	t1, MSC01_PCI_HEAD1_OFS(t0)

	/* setup PCI byte swapping */
#ifdef CONFIG_SYS_BIG_ENDIAN
	li	t1, (0x1 << MSC01_PCI_SWAP_BAR0_BSWAP_SHF) | \
		    (0x1 << MSC01_PCI_SWAP_IO_BSWAP_SHF)
	sw	t1, MSC01_PCI_SWAP_OFS(t0)
#else
	sw	zero, MSC01_PCI_SWAP_OFS(t0)
#endif

	/* enable PCI host configuration cycles */
	lw	t1, MSC01_PCI_CFG_OFS(t0)
	li	t2, MSC01_PCI_CFG_RA_MSK | \
		    MSC01_PCI_CFG_G_MSK | \
		    MSC01_PCI_CFG_EN_MSK
	or	t1, t1, t2
	sw	t1, MSC01_PCI_CFG_OFS(t0)

	jr	ra
	 nop
