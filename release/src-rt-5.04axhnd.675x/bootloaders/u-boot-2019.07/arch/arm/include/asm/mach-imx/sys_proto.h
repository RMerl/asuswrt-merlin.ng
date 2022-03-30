/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

#include <asm/io.h>
#include <asm/mach-imx/regs-common.h>
#include <common.h>
#include "../arch-imx/cpu.h"

#define soc_rev() (get_cpu_rev() & 0xFF)
#define is_soc_rev(rev) (soc_rev() == rev)

/* returns MXC_CPU_ value */
#define cpu_type(rev) (((rev) >> 12) & 0xff)
#define soc_type(rev) (((rev) >> 12) & 0xf0)
/* both macros return/take MXC_CPU_ constants */
#define get_cpu_type() (cpu_type(get_cpu_rev()))
#define get_soc_type() (soc_type(get_cpu_rev()))
#define is_cpu_type(cpu) (get_cpu_type() == cpu)
#define is_soc_type(soc) (get_soc_type() == soc)

#define is_mx6() (is_soc_type(MXC_SOC_MX6))
#define is_mx7() (is_soc_type(MXC_SOC_MX7))
#define is_imx8m() (is_soc_type(MXC_SOC_IMX8M))
#define is_imx8() (is_soc_type(MXC_SOC_IMX8))

#define is_mx6dqp() (is_cpu_type(MXC_CPU_MX6QP) || is_cpu_type(MXC_CPU_MX6DP))
#define is_mx6dq() (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D))
#define is_mx6sdl() (is_cpu_type(MXC_CPU_MX6SOLO) || is_cpu_type(MXC_CPU_MX6DL))
#define is_mx6dl() (is_cpu_type(MXC_CPU_MX6DL))
#define is_mx6sx() (is_cpu_type(MXC_CPU_MX6SX))
#define is_mx6sl() (is_cpu_type(MXC_CPU_MX6SL))
#define is_mx6solo() (is_cpu_type(MXC_CPU_MX6SOLO))
#define is_mx6ul() (is_cpu_type(MXC_CPU_MX6UL))
#define is_mx6ull() (is_cpu_type(MXC_CPU_MX6ULL))
#define is_mx6sll() (is_cpu_type(MXC_CPU_MX6SLL))

#define is_mx7ulp() (is_cpu_type(MXC_CPU_MX7ULP))

#define is_imx8mq() (is_cpu_type(MXC_CPU_IMX8MQ))
#define is_imx8qxp() (is_cpu_type(MXC_CPU_IMX8QXP))

#ifdef CONFIG_MX6
#define IMX6_SRC_GPR10_BMODE		BIT(28)

#define IMX6_BMODE_MASK			GENMASK(7, 0)
#define	IMX6_BMODE_SHIFT		4
#define IMX6_BMODE_EMI_MASK		BIT(3)
#define IMX6_BMODE_EMI_SHIFT		3
#define IMX6_BMODE_SERIAL_ROM_MASK	GENMASK(26, 24)
#define IMX6_BMODE_SERIAL_ROM_SHIFT	24

enum imx6_bmode_serial_rom {
	IMX6_BMODE_ECSPI1,
	IMX6_BMODE_ECSPI2,
	IMX6_BMODE_ECSPI3,
	IMX6_BMODE_ECSPI4,
	IMX6_BMODE_ECSPI5,
	IMX6_BMODE_I2C1,
	IMX6_BMODE_I2C2,
	IMX6_BMODE_I2C3,
};

enum imx6_bmode_emi {
	IMX6_BMODE_NOR,
	IMX6_BMODE_ONENAND,
};

enum imx6_bmode {
	IMX6_BMODE_EMI,
#if defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
	IMX6_BMODE_QSPI,
	IMX6_BMODE_RESERVED,
#else
	IMX6_BMODE_RESERVED,
	IMX6_BMODE_SATA,
#endif
	IMX6_BMODE_SERIAL_ROM,
	IMX6_BMODE_SD,
	IMX6_BMODE_ESD,
	IMX6_BMODE_MMC,
	IMX6_BMODE_EMMC,
	IMX6_BMODE_NAND_MIN,
	IMX6_BMODE_NAND_MAX = 0xf,
};

static inline u8 imx6_is_bmode_from_gpr9(void)
{
	return readl(&src_base->gpr10) & IMX6_SRC_GPR10_BMODE;
}

u32 imx6_src_get_boot_mode(void);
void gpr_init(void);

#endif /* CONFIG_MX6 */

u32 get_nr_cpus(void);
u32 get_cpu_rev(void);
u32 get_cpu_speed_grade_hz(void);
u32 get_cpu_temp_grade(int *minc, int *maxc);
const char *get_imx_type(u32 imxtype);
u32 imx_ddr_size(void);
void sdelay(unsigned long);
void set_chipselect_size(int const);

void init_aips(void);
void init_src(void);
void init_snvs(void);
void imx_wdog_disable_powerdown(void);

int board_mmc_get_env_dev(int devno);

int nxp_board_rev(void);
char nxp_board_rev_string(void);

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int fecmxc_initialize(bd_t *bis);
u32 get_ahb_clk(void);
u32 get_periph_clk(void);

void lcdif_power_down(void);

int mxs_reset_block(struct mxs_register_32 *reg);
int mxs_wait_mask_set(struct mxs_register_32 *reg, u32 mask, u32 timeout);
int mxs_wait_mask_clr(struct mxs_register_32 *reg, u32 mask, u32 timeout);

unsigned long call_imx_sip(unsigned long id, unsigned long reg0,
			   unsigned long reg1, unsigned long reg2);
unsigned long call_imx_sip_ret2(unsigned long id, unsigned long reg0,
				unsigned long *reg1, unsigned long reg2,
				unsigned long reg3);
#endif
