// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018-2020 NXP
 *
 */

#include <asm/global_data.h>
#include <asm/io.h>
#include <exports.h>
#include <fsl-mc/fsl_mc.h>

DECLARE_GLOBAL_DATA_PTR;

int board_eth_init(struct bd_info *bis)
{
#ifdef CONFIG_PHY_AQUANTIA
	/*
	 * Export functions to be used by AQ firmware
	 * upload application
	 */
	gd->jt->strcpy = strcpy;
	gd->jt->mdelay = mdelay;
	gd->jt->mdio_get_current_dev = mdio_get_current_dev;
	gd->jt->phy_find_by_mask = phy_find_by_mask;
	gd->jt->mdio_phydev_for_ethname = mdio_phydev_for_ethname;
	gd->jt->miiphy_set_current_dev = miiphy_set_current_dev;
#endif

	return 0;
}

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
#if defined(CONFIG_FSL_MC_ENET)
	mc_env_boot();
#endif
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
	{1, 3, true},
	{1, 7, true},
	{1, 19, true},
	{1, 20, true},

	/* Serdes block #2 */
	{2, 2, false},
	{2, 3, false},
	{2, 5, false},
	{2, 11, true},

	/* Serdes block #3 */
	{3, 2, false},
	{3, 3, false},
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
	char srds_s1_str[2], srds_s2_str[2], srds_s3_str[2];
	u32 srds_s1, srds_s2, srds_s3;
	char expected_dts[100];

	srds_s1 = rcw_status & FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_MASK;
	srds_s1 >>= FSL_CHASSIS3_RCWSR28_SRDS1_PRTCL_SHIFT;

	srds_s2 = rcw_status & FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_MASK;
	srds_s2 >>= FSL_CHASSIS3_RCWSR28_SRDS2_PRTCL_SHIFT;

	srds_s3 = rcw_status & FSL_CHASSIS3_RCWSR28_SRDS3_PRTCL_MASK;
	srds_s3 >>= FSL_CHASSIS3_RCWSR28_SRDS3_PRTCL_SHIFT;

	/* Check for supported protocols. The default DTS will be used
	 * in this case
	 */
	if (!protocol_supported(1, srds_s1) ||
	    !protocol_supported(2, srds_s2) ||
	    !protocol_supported(3, srds_s3))
		return -1;

	get_str_protocol(1, srds_s1, srds_s1_str);
	get_str_protocol(2, srds_s2, srds_s2_str);
	get_str_protocol(3, srds_s3, srds_s3_str);

	sprintf(expected_dts, "fsl-lx2160a-qds-%s-%s-%s",
		srds_s1_str, srds_s2_str, srds_s3_str);

	if (!strcmp(name, expected_dts))
		return 0;

	return -1;
}
#endif
