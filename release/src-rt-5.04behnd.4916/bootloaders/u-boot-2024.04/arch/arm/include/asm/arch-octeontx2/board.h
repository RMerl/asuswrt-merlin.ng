/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <asm/arch/soc.h>

/** Reg offsets */
#define RST_BOOT		0x87E006001600ULL

#define CPC_BOOT_OWNERX(a)	0x86D000000160ULL + (8 * (a))

/** Structure definitions */
/**
 * Register (NCB32b) cpc_boot_owner#
 *
 * CPC Boot Owner Registers These registers control an external arbiter
 * for the boot device (SPI/eMMC) across multiple external devices. There
 * is a register for each requester: _ \<0\> - SCP          - reset on
 * SCP reset _ \<1\> - MCP          - reset on MCP reset _ \<2\> - AP
 * Secure    - reset on core reset _ \<3\> - AP Nonsecure - reset on core
 * reset  These register is only writable to the corresponding
 * requestor(s) permitted with CPC_PERMIT.
 */
union cpc_boot_ownerx {
	u32 u;
	struct cpc_boot_ownerx_s {
		u32 boot_req		: 1;
		u32 reserved_1_7	: 7;
		u32 boot_wait		: 1;
		u32 reserved_9_31	: 23;
	} s;
};

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
	struct rst_boot_cn96xx {
		u64 rboot_pin                        : 1;
		u64 rboot                            : 1;
		u64 reserved_2_23                    : 22;
		u64 cpt_mul                          : 7;
		u64 reserved_31_32                   : 2;
		u64 pnr_mul                          : 6;
		u64 reserved_39                      : 1;
		u64 c_mul                            : 7;
		u64 reserved_47_52                   : 6;
		u64 gpio_ejtag                       : 1;
		u64 mcp_jtagdis                      : 1;
		u64 dis_scan                         : 1;
		u64 dis_huk                          : 1;
		u64 vrm_err                          : 1;
		u64 reserved_58_59                   : 2;
		u64 trusted_mode                     : 1;
		u64 scp_jtagdis                      : 1;
		u64 jtagdis                          : 1;
		u64 chipkill                         : 1;
	} cn96xx;
	struct rst_boot_cnf95xx {
		u64 rboot_pin                        : 1;
		u64 rboot                            : 1;
		u64 reserved_2_7                     : 6;
		u64 bphy_mul                         : 7;
		u64 reserved_15                      : 1;
		u64 dsp_mul                          : 7;
		u64 reserved_23                      : 1;
		u64 cpt_mul                          : 7;
		u64 reserved_31_32                   : 2;
		u64 pnr_mul                          : 6;
		u64 reserved_39                      : 1;
		u64 c_mul                            : 7;
		u64 reserved_47_52                   : 6;
		u64 gpio_ejtag                       : 1;
		u64 mcp_jtagdis                      : 1;
		u64 dis_scan                         : 1;
		u64 dis_huk                          : 1;
		u64 vrm_err                          : 1;
		u64 reserved_58_59                   : 2;
		u64 trusted_mode                     : 1;
		u64 scp_jtagdis                      : 1;
		u64 jtagdis                          : 1;
		u64 chipkill                         : 1;
	} cnf95xx;
};

extern unsigned long fdt_base_addr;

/** Function definitions */
void mem_map_fill(void);
int fdt_get_board_mac_cnt(void);
u64 fdt_get_board_mac_addr(void);
const char *fdt_get_board_model(void);
const char *fdt_get_board_serial(void);
const char *fdt_get_board_revision(void);
void octeontx2_board_get_mac_addr(u8 index, u8 *mac_addr);
void board_acquire_flash_arb(bool acquire);
void cgx_intf_shutdown(void);

#endif /* __BOARD_H__ */
