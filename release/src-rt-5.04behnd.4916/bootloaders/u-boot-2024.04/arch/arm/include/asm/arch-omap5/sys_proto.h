/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

#include <asm/arch/omap.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/omap_common.h>
#include <linux/mtd/omap_gpmc.h>
#include <asm/arch/clock.h>
#include <asm/ti-common/sys_proto.h>

/*
 * Structure for Iodelay configuration registers.
 * Theoretical max for g_delay is 21560 ps.
 * Theoretical max for a_delay is 1/3rd of g_delay max.
 * So using u16 for both a/g_delay.
 */
struct iodelay_cfg_entry {
	u16 offset;
	u16 a_delay;
	u16 g_delay;
};

struct pad_conf_entry {
	u32 offset;
	u32 val;
};

struct mmc_platform_fixups {
	const char *hw_rev;
	u32 unsupported_caps;
	u32 max_freq;
};

struct omap_sysinfo {
	char *board_string;
};
extern const struct omap_sysinfo sysinfo;

void gpmc_init(void);
void watchdog_init(void);
u32 get_device_type(void);
void do_set_mux(u32 base, struct pad_conf_entry const *array, int size);
void do_set_mux32(u32 base, struct pad_conf_entry const *array, int size);
void set_muxconf_regs(void);
u32 wait_on_value(u32, u32, void *, u32);
void sdelay(unsigned long);
void setup_early_clocks(void);
void prcm_init(void);
void do_board_detect(void);
void vcores_init(void);
void bypass_dpll(u32 const base);
void freq_update_core(void);
u32 get_sys_clk_freq(void);
u32 omap5_ddr_clk(void);
void cancel_out(u32 *num, u32 *den, u32 den_limit);
void sdram_init(void);
u32 omap_sdram_size(void);
u32 cortex_rev(void);
void save_omap_boot_params(void);
void init_omap_revision(void);
void init_package_revision(void);
void do_io_settings(void);
void sri2c_init(void);
int omap_vc_bypass_send_value(u8 sa, u8 reg_addr, u8 reg_data);
u32 warm_reset(void);
void force_emif_self_refresh(void);
void get_ioregs(const struct ctrl_ioregs **regs);
void srcomp_enable(void);
void setup_warmreset_time(void);
const struct mmc_platform_fixups *platform_fixups_mmc(uint32_t addr);

static inline u32 div_round_up(u32 num, u32 den)
{
	return (num + den - 1)/den;
}

static inline u32 usec_to_32k(u32 usec)
{
	return div_round_up(32768 * usec, 1000000);
}

#define OMAP5_SERVICE_L2ACTLR_SET    0x104
#define OMAP5_SERVICE_ACR_SET        0x107

#endif
