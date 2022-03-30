// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <linux/compiler.h>
#include <fsl_errata.h>
#include <asm/processor.h>
#include <fsl_usb.h>
#include "fsl_corenet_serdes.h"

#ifdef CONFIG_SYS_FSL_ERRATUM_A004849
/*
 * This work-around is implemented in PBI, so just check to see if the
 * work-around was actually applied.  To do this, we check for specific data
 * at specific addresses in DCSR.
 *
 * Array offsets[] contains a list of offsets within DCSR.  According to the
 * erratum document, the value at each offset should be 2.
 */
static void check_erratum_a4849(uint32_t svr)
{
	void __iomem *dcsr = (void *)CONFIG_SYS_DCSRBAR + 0xb0000;
	unsigned int i;

#if defined(CONFIG_ARCH_P2041) || defined(CONFIG_ARCH_P3041)
	static const uint8_t offsets[] = {
		0x50, 0x54, 0x58, 0x90, 0x94, 0x98
	};
#endif
#ifdef CONFIG_ARCH_P4080
	static const uint8_t offsets[] = {
		0x60, 0x64, 0x68, 0x6c, 0xa0, 0xa4, 0xa8, 0xac
	};
#endif
	uint32_t x108; /* The value that should be at offset 0x108 */

	for (i = 0; i < ARRAY_SIZE(offsets); i++) {
		if (in_be32(dcsr + offsets[i]) != 2) {
			printf("Work-around for Erratum A004849 is not enabled\n");
			return;
		}
	}

#if defined(CONFIG_ARCH_P2041) || defined(CONFIG_ARCH_P3041)
	x108 = 0x12;
#endif

#ifdef CONFIG_ARCH_P4080
	/*
	 * For P4080, the erratum document says that the value at offset 0x108
	 * should be 0x12 on rev2, or 0x1c on rev3.
	 */
	if (SVR_MAJ(svr) == 2)
		x108 = 0x12;
	if (SVR_MAJ(svr) == 3)
		x108 = 0x1c;
#endif

	if (in_be32(dcsr + 0x108) != x108) {
		printf("Work-around for Erratum A004849 is not enabled\n");
		return;
	}

	/* Everything matches, so the erratum work-around was applied */

	printf("Work-around for Erratum A004849 enabled\n");
}
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A004580
/*
 * This work-around is implemented in PBI, so just check to see if the
 * work-around was actually applied.  To do this, we check for specific data
 * at specific addresses in the SerDes register block.
 *
 * The work-around says that for each SerDes lane, write BnTTLCRy0 =
 * 0x1B00_0001, Register 2 = 0x0088_0000, and Register 3 = 0x4000_0000.

 */
static void check_erratum_a4580(uint32_t svr)
{
	const serdes_corenet_t __iomem *srds_regs =
		(void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	unsigned int lane;

	for (lane = 0; lane < SRDS_MAX_LANES; lane++) {
		if (serdes_lane_enabled(lane)) {
			const struct serdes_lane __iomem *srds_lane =
				&srds_regs->lane[serdes_get_lane_idx(lane)];

			/*
			 * Verify that the values we were supposed to write in
			 * the PBI are actually there.  Also, the lower 15
			 * bits of res4[3] should be the same as the upper 15
			 * bits of res4[1].
			 */
			if ((in_be32(&srds_lane->ttlcr0) != 0x1b000001) ||
			    (in_be32(&srds_lane->res4[1]) != 0x880000) ||
			    (in_be32(&srds_lane->res4[3]) != 0x40000044)) {
				printf("Work-around for Erratum A004580 is "
				       "not enabled\n");
				return;
			}
		}
	}

	/* Everything matches, so the erratum work-around was applied */

	printf("Work-around for Erratum A004580 enabled\n");
}
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A007212
/*
 * This workaround can be implemented in PBI, or by u-boot.
 */
static void check_erratum_a007212(void)
{
	u32 __iomem *plldgdcr = (void *)(CONFIG_SYS_DCSRBAR + 0x21c20);

	if (in_be32(plldgdcr) & 0x1fe) {
		/* check if PLL ratio is set by workaround */
		puts("Work-around for Erratum A007212 enabled\n");
	}
}
#endif

static int do_errata(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_CPU_A011
	extern int enable_cpu_a011_workaround;
#endif
	__maybe_unused u32 svr = get_svr();

#if defined(CONFIG_FSL_SATA_V2) && defined(CONFIG_SYS_FSL_ERRATUM_SATA_A001)
	if (IS_SVR_REV(svr, 1, 0)) {
		switch (SVR_SOC_VER(svr)) {
		case SVR_P1013:
		case SVR_P1022:
			puts("Work-around for Erratum SATA A001 enabled\n");
		}
	}
#endif

#if defined(CONFIG_SYS_P4080_ERRATUM_SERDES8)
	puts("Work-around for Erratum SERDES8 enabled\n");
#endif
#if defined(CONFIG_SYS_P4080_ERRATUM_SERDES9)
	puts("Work-around for Erratum SERDES9 enabled\n");
#endif
#if defined(CONFIG_SYS_P4080_ERRATUM_SERDES_A005)
	puts("Work-around for Erratum SERDES-A005 enabled\n");
#endif
#if defined(CONFIG_SYS_P4080_ERRATUM_CPU22)
	if (SVR_MAJ(svr) < 3)
		puts("Work-around for Erratum CPU22 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_CPU_A011
	/*
	 * NMG_CPU_A011 applies to P4080 rev 1.0, 2.0, fixed in 3.0
	 * also applies to P3041 rev 1.0, 1.1, P2041 rev 1.0, 1.1
	 * The SVR has been checked by cpu_init_r().
	 */
	if (enable_cpu_a011_workaround)
		puts("Work-around for Erratum CPU-A011 enabled\n");
#endif
#if defined(CONFIG_SYS_FSL_ERRATUM_CPU_A003999)
	puts("Work-around for Erratum CPU-A003999 enabled\n");
#endif
#if defined(CONFIG_SYS_FSL_ERRATUM_DDR_A003474)
	puts("Work-around for Erratum DDR-A003474 enabled\n");
#endif
#if defined(CONFIG_SYS_FSL_ERRATUM_DDR_MSYNC_IN)
	puts("Work-around for DDR MSYNC_IN Erratum enabled\n");
#endif
#if defined(CONFIG_SYS_FSL_ERRATUM_ESDHC111)
	puts("Work-around for Erratum ESDHC111 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A004468
	puts("Work-around for Erratum A004468 enabled\n");
#endif
#if defined(CONFIG_SYS_FSL_ERRATUM_ESDHC135)
	puts("Work-around for Erratum ESDHC135 enabled\n");
#endif
#if defined(CONFIG_SYS_FSL_ERRATUM_ESDHC13)
	if (SVR_MAJ(svr) < 3)
		puts("Work-around for Erratum ESDHC13 enabled\n");
#endif
#if defined(CONFIG_SYS_FSL_ERRATUM_ESDHC_A001)
	puts("Work-around for Erratum ESDHC-A001 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_CPC_A002
	puts("Work-around for Erratum CPC-A002 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_CPC_A003
	puts("Work-around for Erratum CPC-A003 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_ELBC_A001
	puts("Work-around for Erratum ELBC-A001 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_DDR_A003
	puts("Work-around for Erratum DDR-A003 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_DDR_115
	puts("Work-around for Erratum DDR115 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_DDR111_DDR134
	puts("Work-around for Erratum DDR111 enabled\n");
	puts("Work-around for Erratum DDR134 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_IFC_A002769
	puts("Work-around for Erratum IFC-A002769 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_P1010_A003549
	puts("Work-around for Erratum P1010-A003549 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_IFC_A003399
	puts("Work-around for Erratum IFC A-003399 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_DDR120
	if ((SVR_MAJ(svr) == 1) || IS_SVR_REV(svr, 2, 0))
		puts("Work-around for Erratum NMG DDR120 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_LBC103
	puts("Work-around for Erratum NMG_LBC103 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_NMG_ETSEC129
	if ((SVR_MAJ(svr) == 1) || IS_SVR_REV(svr, 2, 0))
		puts("Work-around for Erratum NMG ETSEC129 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A004508
	puts("Work-around for Erratum A004508 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A004510
	puts("Work-around for Erratum A004510 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_SRIO_A004034
	puts("Work-around for Erratum SRIO-A004034 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A_004934
	puts("Work-around for Erratum A004934 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A005871
	if (IS_SVR_REV(svr, 1, 0))
		puts("Work-around for Erratum A005871 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A006475
	if (SVR_MAJ(get_svr()) == 1)
		puts("Work-around for Erratum A006475 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A006384
	if (SVR_MAJ(get_svr()) == 1)
		puts("Work-around for Erratum A006384 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A004849
	/* This work-around is implemented in PBI, so just check for it */
	check_erratum_a4849(svr);
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A004580
	/* This work-around is implemented in PBI, so just check for it */
	check_erratum_a4580(svr);
#endif
#ifdef CONFIG_SYS_P4080_ERRATUM_PCIE_A003
	puts("Work-around for Erratum PCIe-A003 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_USB14
	puts("Work-around for Erratum USB14 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A007186
	if (has_erratum_a007186())
		puts("Work-around for Erratum A007186 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A006593
	puts("Work-around for Erratum A006593 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A006379
	if (has_erratum_a006379())
		puts("Work-around for Erratum A006379 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_SEC_A003571
	if (IS_SVR_REV(svr, 1, 0))
		puts("Work-around for Erratum A003571 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A005812
	puts("Work-around for Erratum A-005812 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A005125
	puts("Work-around for Erratum A005125 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A007075
	if (has_erratum_a007075())
		puts("Work-around for Erratum A007075 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A007798
	if (has_erratum_a007798())
		puts("Work-around for Erratum A007798 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A004477
	if (has_erratum_a004477())
		puts("Work-around for Erratum A004477 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_I2C_A004447
	if ((SVR_SOC_VER(svr) == SVR_8548 && IS_SVR_REV(svr, 3, 1)) ||
	    (SVR_REV(svr) <= CONFIG_SYS_FSL_A004447_SVR_REV))
		puts("Work-around for Erratum I2C-A004447 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A005275
	if (has_erratum_a005275())
		puts("Work-around for Erratum A005275 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A006261
	if (has_erratum_a006261())
		puts("Work-around for Erratum A006261 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A007212
	check_erratum_a007212();
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A005434
	puts("Work-around for Erratum A-005434 enabled\n");
#endif
#if defined(CONFIG_SYS_FSL_ERRATUM_A008044) && \
	defined(CONFIG_A008044_WORKAROUND)
	if (IS_SVR_REV(svr, 1, 0))
		puts("Work-around for Erratum A-008044 enabled\n");
#endif
#if defined(CONFIG_SYS_FSL_B4860QDS_XFI_ERR) && \
	(defined(CONFIG_TARGET_B4860QDS) || defined(CONFIG_TARGET_B4420QDS))
	puts("Work-around for Erratum XFI on B4860QDS enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A009663
	puts("Work-around for Erratum A009663 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A007907
	puts("Work-around for Erratum A007907 enabled\n");
#endif
#ifdef CONFIG_SYS_FSL_ERRATUM_A007815
	puts("Work-around for Erratum A007815 enabled\n");
#endif

	return 0;
}

U_BOOT_CMD(
	errata, 1, 0,	do_errata,
	"Report errata workarounds",
	""
);
