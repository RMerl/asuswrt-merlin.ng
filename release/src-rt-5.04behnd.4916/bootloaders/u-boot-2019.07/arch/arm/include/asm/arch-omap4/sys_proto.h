/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

#include <asm/arch/omap.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <asm/omap_common.h>
#include <linux/mtd/omap_gpmc.h>
#include <asm/arch/mux_omap4.h>
#include <asm/ti-common/sys_proto.h>

#ifdef CONFIG_SYS_EMIF_PRECALCULATED_TIMING_REGS
extern const struct emif_regs emif_regs_elpida_200_mhz_2cs;
extern const struct emif_regs emif_regs_elpida_380_mhz_1cs;
extern const struct emif_regs emif_regs_elpida_400_mhz_1cs;
extern const struct emif_regs emif_regs_elpida_400_mhz_2cs;
extern const struct dmm_lisa_map_regs lisa_map_2G_x_1_x_2;
extern const struct dmm_lisa_map_regs lisa_map_2G_x_2_x_2;
extern const struct dmm_lisa_map_regs ma_lisa_map_2G_x_2_x_2;
#else
extern const struct lpddr2_device_details elpida_2G_S4_details;
extern const struct lpddr2_device_details elpida_4G_S4_details;
#endif

#ifdef CONFIG_SYS_DEFAULT_LPDDR2_TIMINGS
extern const struct lpddr2_device_timings jedec_default_timings;
#else
extern const struct lpddr2_device_timings elpida_2G_S4_timings;
#endif

struct omap_sysinfo {
	char *board_string;
};
extern const struct omap_sysinfo sysinfo;

void gpmc_init(void);
void watchdog_init(void);
u32 get_device_type(void);
void do_set_mux(u32 base, struct pad_conf_entry const *array, int size);
void set_muxconf_regs(void);
u32 wait_on_value(u32, u32, void *, u32);
void sdelay(unsigned long);
void setup_early_clocks(void);
void prcm_init(void);
void do_board_detect(void);
void bypass_dpll(u32 const base);
void freq_update_core(void);
u32 get_sys_clk_freq(void);
u32 omap4_ddr_clk(void);
void cancel_out(u32 *num, u32 *den, u32 den_limit);
void sdram_init(void);
u32 omap_sdram_size(void);
u32 cortex_rev(void);
void save_omap_boot_params(void);
void init_omap_revision(void);
void do_io_settings(void);
void sri2c_init(void);
int omap_vc_bypass_send_value(u8 sa, u8 reg_addr, u8 reg_data);
u32 warm_reset(void);
void force_emif_self_refresh(void);
void setup_warmreset_time(void);

#define OMAP4_SERVICE_PL310_CONTROL_REG_SET	0x102

#endif
