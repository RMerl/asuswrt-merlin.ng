/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * sys_proto.h
 *
 * System information header
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_
#include <linux/mtd/omap_gpmc.h>
#include <asm/arch/cpu.h>

u32 get_cpu_rev(void);
u32 get_sysboot_value(void);

extern struct ctrl_stat *cstat;
u32 get_device_type(void);
void save_omap_boot_params(void);
void setup_early_clocks(void);
void setup_clocks_for_console(void);
void mpu_pll_config_val(int mpull_m);
void ddr_pll_config(unsigned int ddrpll_M);

void sdelay(unsigned long);

void gpmc_init(void);
void enable_gpmc_cs_config(const u32 *gpmc_config, const struct gpmc_cs *cs, u32 base,
			u32 size);
int omap_nand_switch_ecc(uint32_t, uint32_t);

void set_uart_mux_conf(void);
void set_mux_conf_regs(void);
void sdram_init(void);
u32 wait_on_value(u32, u32, void *, u32);
#ifdef CONFIG_NOR_BOOT
void enable_norboot_pin_mux(void);
#endif
void am33xx_spl_board_init(void);
int am335x_get_efuse_mpu_max_freq(struct ctrl_dev *cdev);
int am335x_get_mpu_vdd(int sil_rev, int frequency);
int am335x_get_tps65910_mpu_vdd(int sil_rev, int frequency);
#endif

void enable_usb_clocks(int index);
void disable_usb_clocks(int index);
void do_board_detect(void);
u32 get_sys_clk_index(void);
