/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2016-2021 Intel Corporation
 */

#ifndef _SOCFPGA_MISC_H_
#define _SOCFPGA_MISC_H_

#include <asm/sections.h>

void dwmac_deassert_reset(const unsigned int of_reset_id, const u32 phymode);

struct bsel {
	const char	*mode;
	const char	*name;
};

extern struct bsel bsel_str[];

#ifdef CONFIG_FPGA
void socfpga_fpga_add(void *fpga_desc);
#else
static inline void socfpga_fpga_add(void *fpga_desc) {}
#endif

#ifdef CONFIG_TARGET_SOCFPGA_GEN5
void socfpga_sdram_remap_zero(void);
static inline bool socfpga_is_booting_from_fpga(void)
{
	if ((__image_copy_start >= (char *)SOCFPGA_FPGA_SLAVES_ADDRESS) &&
	    (__image_copy_start < (char *)SOCFPGA_STM_ADDRESS))
		return true;
	return false;
}
#endif

#ifdef CONFIG_TARGET_SOCFPGA_ARRIA10
void socfpga_init_security_policies(void);
void socfpga_sdram_remap_zero(void);
#endif

#if defined(CONFIG_TARGET_SOCFPGA_STRATIX10) || \
	defined(CONFIG_TARGET_SOCFPGA_AGILEX)
int is_fpga_config_ready(void);
#endif

void do_bridge_reset(int enable, unsigned int mask);
void force_periph_program(unsigned int status);
bool is_regular_boot_valid(void);
bool is_periph_program_force(void);
void set_regular_boot(unsigned int status);
void socfpga_pl310_clear(void);
void socfpga_get_managers_addr(void);
int qspi_flash_software_reset(void);

#endif /* _SOCFPGA_MISC_H_ */
