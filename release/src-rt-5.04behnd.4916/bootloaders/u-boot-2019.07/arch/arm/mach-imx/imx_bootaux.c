// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/mach-imx/sys_proto.h>
#include <command.h>
#include <imx_sip.h>
#include <linux/compiler.h>

int arch_auxiliary_core_up(u32 core_id, ulong boot_private_data)
{
	ulong stack, pc;

	if (!boot_private_data)
		return -EINVAL;

	stack = *(u32 *)boot_private_data;
	pc = *(u32 *)(boot_private_data + 4);

	/* Set the stack and pc to M4 bootROM */
	writel(stack, M4_BOOTROM_BASE_ADDR);
	writel(pc, M4_BOOTROM_BASE_ADDR + 4);

	/* Enable M4 */
#ifdef CONFIG_IMX8M
	call_imx_sip(IMX_SIP_SRC, IMX_SIP_SRC_M4_START, 0, 0);
#else
	clrsetbits_le32(SRC_BASE_ADDR + SRC_M4_REG_OFFSET,
			SRC_M4C_NON_SCLR_RST_MASK, SRC_M4_ENABLE_MASK);
#endif

	return 0;
}

int arch_auxiliary_core_check_up(u32 core_id)
{
#ifdef CONFIG_IMX8M
	return call_imx_sip(IMX_SIP_SRC, IMX_SIP_SRC_M4_STARTED, 0, 0);
#else
	unsigned int val;

	val = readl(SRC_BASE_ADDR + SRC_M4_REG_OFFSET);

	if (val & SRC_M4C_NON_SCLR_RST_MASK)
		return 0;  /* assert in reset */

	return 1;
#endif
}

/*
 * To i.MX6SX and i.MX7D, the image supported by bootaux needs
 * the reset vector at the head for the image, with SP and PC
 * as the first two words.
 *
 * Per the cortex-M reference manual, the reset vector of M4 needs
 * to exist at 0x0 (TCMUL). The PC and SP are the first two addresses
 * of that vector.  So to boot M4, the A core must build the M4's reset
 * vector with getting the PC and SP from image and filling them to
 * TCMUL. When M4 is kicked, it will load the PC and SP by itself.
 * The TCMUL is mapped to (M4_BOOTROM_BASE_ADDR) at A core side for
 * accessing the M4 TCMUL.
 */
static int do_bootaux(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong addr;
	int ret, up;

	if (argc < 2)
		return CMD_RET_USAGE;

	up = arch_auxiliary_core_check_up(0);
	if (up) {
		printf("## Auxiliary core is already up\n");
		return CMD_RET_SUCCESS;
	}

	addr = simple_strtoul(argv[1], NULL, 16);

	printf("## Starting auxiliary core at 0x%08lX ...\n", addr);

	ret = arch_auxiliary_core_up(0, addr);
	if (ret)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	bootaux, CONFIG_SYS_MAXARGS, 1,	do_bootaux,
	"Start auxiliary core",
	""
);
