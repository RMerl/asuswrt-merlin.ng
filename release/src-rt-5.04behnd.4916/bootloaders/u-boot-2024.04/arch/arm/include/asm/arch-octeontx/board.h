/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <asm/arch/soc.h>

#define MAX_LMAC_PER_BGX 4
#define LMAC_CNT MAX_LMAC_PER_BGX

#if defined(CONFIG_TARGET_OCTEONTX_81XX)

/** Maximum number of BGX interfaces per CPU node */
#define MAX_BGX_PER_NODE	3
#define OCTEONTX_XCV	/* RGMII Interface */

#elif defined(CONFIG_TARGET_OCTEONTX_83XX)

/** Maximum number of BGX interfaces per CPU node */
#define MAX_BGX_PER_NODE	4

#endif

/** Reg offsets */
#define RST_BOOT	0x87E006001600ULL

/** Structure definitions */

/**
 * Register (RSL) rst_boot
 *
 * RST Boot Register This register is not accessible through ROM scripts;
 * see SCR_WRITE32_S[ADDR].
 */
union rst_boot {
	u64 u;
	struct rst_boot_s {
		u64 rboot_pin                        : 1;
		u64 rboot                            : 1;
		u64 reserved_2_32                    : 31;
		u64 pnr_mul                          : 6;
		u64 reserved_39                      : 1;
		u64 c_mul                            : 7;
		u64 reserved_47_52                   : 6;
		u64 gpio_ejtag                       : 1;
		u64 mcp_jtagdis                      : 1;
		u64 dis_scan                         : 1;
		u64 dis_huk                          : 1;
		u64 vrm_err                          : 1;
		u64 jt_tstmode                       : 1;
		u64 ckill_ppdis                      : 1;
		u64 trusted_mode                     : 1;
		u64 reserved_61_62                   : 2;
		u64 chipkill                         : 1;
	} s;
	struct rst_boot_cn81xx {
		u64 rboot_pin                        : 1;
		u64 rboot                            : 1;
		u64 lboot                            : 10;
		u64 lboot_ext23                      : 6;
		u64 lboot_ext45                      : 6;
		u64 lboot_jtg                        : 1;
		u64 lboot_ckill                      : 1;
		u64 reserved_26_29                   : 4;
		u64 lboot_oci                        : 3;
		u64 pnr_mul                          : 6;
		u64 reserved_39                      : 1;
		u64 c_mul                            : 7;
		u64 reserved_47_54                   : 8;
		u64 dis_scan                         : 1;
		u64 dis_huk                          : 1;
		u64 vrm_err                          : 1;
		u64 jt_tstmode                       : 1;
		u64 ckill_ppdis                      : 1;
		u64 trusted_mode                     : 1;
		u64 ejtagdis                         : 1;
		u64 jtcsrdis                         : 1;
		u64 chipkill                         : 1;
	} cn81xx;
	struct rst_boot_cn83xx {
		u64 rboot_pin                        : 1;
		u64 rboot                            : 1;
		u64 lboot                            : 10;
		u64 lboot_ext23                      : 6;
		u64 lboot_ext45                      : 6;
		u64 lboot_jtg                        : 1;
		u64 lboot_ckill                      : 1;
		u64 lboot_pf_flr                     : 4;
		u64 lboot_oci                        : 3;
		u64 pnr_mul                          : 6;
		u64 reserved_39                      : 1;
		u64 c_mul                            : 7;
		u64 reserved_47_54                   : 8;
		u64 dis_scan                         : 1;
		u64 dis_huk                          : 1;
		u64 vrm_err                          : 1;
		u64 jt_tstmode                       : 1;
		u64 ckill_ppdis                      : 1;
		u64 trusted_mode                     : 1;
		u64 ejtagdis                         : 1;
		u64 jtcsrdis                         : 1;
		u64 chipkill                         : 1;
	} cn83xx;
};

extern unsigned long fdt_base_addr;

/** Function definitions */
void mem_map_fill(void);
int octeontx_board_has_pmp(void);
const char *fdt_get_board_model(void);
const char *fdt_get_board_serial(void);
const char *fdt_get_board_revision(void);
void fdt_parse_phy_info(void);
void fdt_board_get_ethaddr(int bgx, int lmac, unsigned char *eth);
void bgx_set_board_info(int bgx_id, int *mdio_bus, int *phy_addr,
			bool *autoneg_dis, bool *lmac_reg, bool *lmac_enable);
#endif /* __BOARD_H__ */
