// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#include <config.h>
#include <vsprintf.h>
#include <linux/string.h>
#include <asm/io.h>
#include <asm/arch/fsl_serdes.h>
#include <fsl-mc/fsl_mc.h>

#define MC_BOOT_ENV_VAR "mcinitcmd"

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
	mc_env_boot();
}
#endif /* CONFIG_RESET_PHY_R */

#if defined(CONFIG_MULTI_DTB_FIT)

/* Structure to hold SERDES protocols supported (network interfaces are
 * described in the DTS).
 *
 * @serdes_block: the index of the SERDES block
 * @serdes_protocol: the decimal value of the protocol supported
 * @dts_needed: DTS notes describing the current configuration are needed
 *
 * When dts_needed is true, the board_fit_config_name_match() function
 * will try to exactly match the current configuration of the block with a DTS
 * name provided.
 */
static struct serdes_configuration {
	u8 serdes_block;
	u32 serdes_protocol;
	bool dts_needed;
} supported_protocols[] = {
	/* Serdes block #1 */
	{1, 42, true},

	/* Serdes block #2 */
	{2, 65, false},
};

#define SUPPORTED_SERDES_PROTOCOLS ARRAY_SIZE(supported_protocols)

static bool protocol_supported(u8 serdes_block, u32 protocol)
{
	struct serdes_configuration serdes_conf;
	int i;

	for (i = 0; i < SUPPORTED_SERDES_PROTOCOLS; i++) {
		serdes_conf = supported_protocols[i];
		if (serdes_conf.serdes_block == serdes_block &&
		    serdes_conf.serdes_protocol == protocol)
			return true;
	}

	return false;
}

static void get_str_protocol(u8 serdes_block, u32 protocol, char *str)
{
	struct serdes_configuration serdes_conf;
	int i;

	for (i = 0; i < SUPPORTED_SERDES_PROTOCOLS; i++) {
		serdes_conf = supported_protocols[i];
		if (serdes_conf.serdes_block == serdes_block &&
		    serdes_conf.serdes_protocol == protocol) {
			if (serdes_conf.dts_needed == true)
				sprintf(str, "%u", protocol);
			else
				sprintf(str, "x");
			return;
		}
	}
}

int board_fit_config_name_match(const char *name)
{
	struct ccsr_gur *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
	u32 rcw_status = in_le32(&gur->rcwsr[28]);
	char srds_s1_str[2], srds_s2_str[2];
	u32 srds_s1, srds_s2;
	char expected_dts[100];

	srds_s1 = rcw_status & FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;

	srds_s2 = rcw_status & FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK;
	srds_s2 >>= FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT;

	/* Check for supported protocols. The default DTS will be used
	 * in this case
	 */
	if (!protocol_supported(1, srds_s1) ||
	    !protocol_supported(2, srds_s2))
		return -1;

	get_str_protocol(1, srds_s1, srds_s1_str);
	get_str_protocol(2, srds_s2, srds_s2_str);

	printf("expected_dts %s\n", expected_dts);
	sprintf(expected_dts, "fsl-ls2080a-qds-%s-%s",
		srds_s1_str, srds_s2_str);

	if (!strcmp(name, expected_dts))
		return 0;

	printf("this is not!\n");
	return -1;
}

#endif
