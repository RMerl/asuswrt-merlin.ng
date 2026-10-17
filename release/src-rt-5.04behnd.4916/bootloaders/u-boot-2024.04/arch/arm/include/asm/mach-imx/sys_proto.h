/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 */

#ifndef _SYS_PROTO_H_
#define _SYS_PROTO_H_

#include <asm/io.h>
#include <asm/mach-imx/regs-common.h>
#include <asm/mach-imx/module_fuse.h>
#include <linux/bitops.h>
#include "../arch-imx/cpu.h"

struct bd_info;

#define soc_rev() (get_cpu_rev() & 0xFF)
#define is_soc_rev(rev) (soc_rev() == rev)

/* returns MXC_CPU_ value */
#define cpu_type(rev) (((rev) >> 12) & 0x1ff)
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
#define is_imx9() (is_soc_type(MXC_SOC_IMX9))
#define is_imxrt() (is_soc_type(MXC_SOC_IMXRT))

#define is_mx6dqp() (is_cpu_type(MXC_CPU_MX6QP) || is_cpu_type(MXC_CPU_MX6DP))
#define is_mx6dq() (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D))
#define is_mx6sdl() (is_cpu_type(MXC_CPU_MX6SOLO) || is_cpu_type(MXC_CPU_MX6DL))
#define is_mx6dl() (is_cpu_type(MXC_CPU_MX6DL))
#define is_mx6sx() (is_cpu_type(MXC_CPU_MX6SX))
#define is_mx6sl() (is_cpu_type(MXC_CPU_MX6SL))
#define is_mx6solo() (is_cpu_type(MXC_CPU_MX6SOLO))
#define is_mx6ul() (is_cpu_type(MXC_CPU_MX6UL))
#define is_mx6ull() (is_cpu_type(MXC_CPU_MX6ULL) || is_cpu_type(MXC_CPU_MX6ULZ))
#define is_mx6ulz() (is_cpu_type(MXC_CPU_MX6ULZ))
#define is_mx6sll() (is_cpu_type(MXC_CPU_MX6SLL))

#define is_mx7ulp() (is_cpu_type(MXC_CPU_MX7ULP))

#define is_imx8mq() (is_cpu_type(MXC_CPU_IMX8MQ) || is_cpu_type(MXC_CPU_IMX8MD) || is_cpu_type(MXC_CPU_IMX8MQL))
#define is_imx8md() (is_cpu_type(MXC_CPU_IMX8MD))
#define is_imx8mql() (is_cpu_type(MXC_CPU_IMX8MQL))
#define is_imx8qm() (is_cpu_type(MXC_CPU_IMX8QM))
#define is_imx8ulp() (is_cpu_type(MXC_CPU_IMX8ULP))
#define is_imx8mm() (is_cpu_type(MXC_CPU_IMX8MM) || is_cpu_type(MXC_CPU_IMX8MML) ||\
	is_cpu_type(MXC_CPU_IMX8MMD) || is_cpu_type(MXC_CPU_IMX8MMDL) || \
	is_cpu_type(MXC_CPU_IMX8MMS) || is_cpu_type(MXC_CPU_IMX8MMSL))
#define is_imx8mml() (is_cpu_type(MXC_CPU_IMX8MML))
#define is_imx8mmd() (is_cpu_type(MXC_CPU_IMX8MMD))
#define is_imx8mmdl() (is_cpu_type(MXC_CPU_IMX8MMDL))
#define is_imx8mms() (is_cpu_type(MXC_CPU_IMX8MMS))
#define is_imx8mmsl() (is_cpu_type(MXC_CPU_IMX8MMSL))
#define is_imx8mn() (is_cpu_type(MXC_CPU_IMX8MN) || is_cpu_type(MXC_CPU_IMX8MND) || \
	is_cpu_type(MXC_CPU_IMX8MNS) || is_cpu_type(MXC_CPU_IMX8MNL) || \
	is_cpu_type(MXC_CPU_IMX8MNDL) || is_cpu_type(MXC_CPU_IMX8MNSL) || \
	is_cpu_type(MXC_CPU_IMX8MNUD) || is_cpu_type(MXC_CPU_IMX8MNUS) || is_cpu_type(MXC_CPU_IMX8MNUQ))
#define is_imx8mnd() (is_cpu_type(MXC_CPU_IMX8MND))
#define is_imx8mns() (is_cpu_type(MXC_CPU_IMX8MNS))
#define is_imx8mnl() (is_cpu_type(MXC_CPU_IMX8MNL))
#define is_imx8mndl() (is_cpu_type(MXC_CPU_IMX8MNDL))
#define is_imx8mnsl() (is_cpu_type(MXC_CPU_IMX8MNSL))
#define is_imx8mnuq() (is_cpu_type(MXC_CPU_IMX8MNUQ))
#define is_imx8mnud() (is_cpu_type(MXC_CPU_IMX8MNUD))
#define is_imx8mnus() (is_cpu_type(MXC_CPU_IMX8MNUS))
#define is_imx8mp() (is_cpu_type(MXC_CPU_IMX8MP)  || is_cpu_type(MXC_CPU_IMX8MPD) || \
	is_cpu_type(MXC_CPU_IMX8MPL) || is_cpu_type(MXC_CPU_IMX8MP6) || is_cpu_type(MXC_CPU_IMX8MPUL))
#define is_imx8mpd() (is_cpu_type(MXC_CPU_IMX8MPD))
#define is_imx8mpl() (is_cpu_type(MXC_CPU_IMX8MPL))
#define is_imx8mp6() (is_cpu_type(MXC_CPU_IMX8MP6))
#define is_imx8mpul() (is_cpu_type(MXC_CPU_IMX8MPUL))

#define is_imx8qxp() (is_cpu_type(MXC_CPU_IMX8QXP))

#define is_imx93() (is_cpu_type(MXC_CPU_IMX93) || is_cpu_type(MXC_CPU_IMX9331) || \
	is_cpu_type(MXC_CPU_IMX9332) || is_cpu_type(MXC_CPU_IMX9351) || \
	is_cpu_type(MXC_CPU_IMX9322) || is_cpu_type(MXC_CPU_IMX9321) || \
	is_cpu_type(MXC_CPU_IMX9312) || is_cpu_type(MXC_CPU_IMX9311))
#define is_imx9351() (is_cpu_type(MXC_CPU_IMX9351))
#define is_imx9332() (is_cpu_type(MXC_CPU_IMX9332))
#define is_imx9331() (is_cpu_type(MXC_CPU_IMX9331))
#define is_imx9322() (is_cpu_type(MXC_CPU_IMX9322))
#define is_imx9321() (is_cpu_type(MXC_CPU_IMX9321))
#define is_imx9312() (is_cpu_type(MXC_CPU_IMX9312))
#define is_imx9311() (is_cpu_type(MXC_CPU_IMX9311))

#define is_imxrt1020() (is_cpu_type(MXC_CPU_IMXRT1020))
#define is_imxrt1050() (is_cpu_type(MXC_CPU_IMXRT1050))

#ifdef CONFIG_MX6
#define IMX6_SRC_GPR10_BMODE			BIT(28)
#define IMX6_SRC_GPR10_PERSIST_SECONDARY_BOOT	BIT(30)

#define IMX6_BMODE_MASK			GENMASK(7, 0)
#define IMX6_BMODE_SHIFT		4
#define IMX6_BMODE_EIM_MASK		BIT(3)
#define IMX6_BMODE_EIM_SHIFT		3
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

enum imx6_bmode_eim {
	IMX6_BMODE_NOR,
	IMX6_BMODE_ONENAND,
};

enum imx6_bmode {
	IMX6_BMODE_EIM,
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

u32 imx6_src_get_boot_mode(void);
void gpr_init(void);

#endif /* CONFIG_MX6 */

#ifdef CONFIG_MX7
#define IMX7_SRC_GPR10_BMODE			BIT(28)
#define IMX7_SRC_GPR10_PERSIST_SECONDARY_BOOT	BIT(30)
#endif

/* address translation table */
struct rproc_att {
	u32 da; /* device address (From Cortex M4 view) */
	u32 sa; /* system bus address */
	u32 size; /* size of reg range */
};

const struct rproc_att *imx_bootaux_get_hostmap(void);

struct rom_api {
	u16 ver;
	u16 tag;
	u32 reserved1;
	u32 (*download_image)(u8 *dest, u32 offset, u32 size,  u32 xor);
	u32 (*query_boot_infor)(u32 info_type, u32 *info, u32 xor);
};

enum boot_dev_type_e {
	BT_DEV_TYPE_SD = 1,
	BT_DEV_TYPE_MMC = 2,
	BT_DEV_TYPE_NAND = 3,
	BT_DEV_TYPE_FLEXSPINOR = 4,
	BT_DEV_TYPE_SPI_NOR = 6,

	BT_DEV_TYPE_USB = 0xE,
	BT_DEV_TYPE_MEM_DEV = 0xF,

	BT_DEV_TYPE_INVALID = 0xFF
};

enum boot_stage_type {
	BT_STAGE_PRIMARY = 0x6,
	BT_STAGE_SECONDARY = 0x9,
	BT_STAGE_RECOVERY = 0xa,
	BT_STAGE_USB = 0x5,
};

#define QUERY_ROM_VER		1
#define QUERY_BT_DEV		2
#define QUERY_PAGE_SZ		3
#define QUERY_IVT_OFF		4
#define QUERY_BT_STAGE		5
#define QUERY_IMG_OFF		6

#define ROM_API_OKAY		0xF0

extern struct rom_api *g_rom_api;
extern unsigned long rom_pointer[];

ulong spl_romapi_raw_seekable_read(u32 offset, u32 size, void *buf);
ulong spl_romapi_get_uboot_base(u32 image_offset, u32 rom_bt_dev);

u32 rom_api_download_image(u8 *dest, u32 offset, u32 size);
u32 rom_api_query_boot_infor(u32 info_type, u32 *info);

/* For i.MX ULP */
#define BT0CFG_LPBOOT_MASK	0x1
#define BT0CFG_DUALBOOT_MASK	0x2

enum bt_mode {
	LOW_POWER_BOOT,		/* LP_BT = 1 */
	DUAL_BOOT,		/* LP_BT = 0, DUAL_BT = 1 */
	SINGLE_BOOT		/* LP_BT = 0, DUAL_BT = 0 */
};

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

void board_mem_get_layout(u64 *phys_sdram_1_start,
			  u64 *phys_sdram_1_size,
			  u64 *phys_sdram_2_start,
			  u64 *phys_sdram_2_size);

int arch_auxiliary_core_up(u32 core_id, ulong boot_private_data);
int arch_auxiliary_core_check_up(u32 core_id);

int board_mmc_get_env_dev(int devno);

int nxp_board_rev(void);
char nxp_board_rev_string(void);

/*
 * Initializes on-chip ethernet controllers.
 * to override, implement board_eth_init()
 */
int fecmxc_initialize(struct bd_info *bis);
u32 get_ahb_clk(void);
u32 get_periph_clk(void);

void lcdif_power_down(void);

int mxs_reset_block(struct mxs_register_32 *reg);
int mxs_wait_mask_set(struct mxs_register_32 *reg, u32 mask, u32 timeout);
int mxs_wait_mask_clr(struct mxs_register_32 *reg, u32 mask, u32 timeout);

void board_late_mmc_env_init(void);

unsigned long call_imx_sip(unsigned long id, unsigned long reg0,
			   unsigned long reg1, unsigned long reg2,
			   unsigned long reg3);
unsigned long call_imx_sip_ret2(unsigned long id, unsigned long reg0,
				unsigned long *reg1, unsigned long reg2,
				unsigned long reg3);

void imx_get_mac_from_fuse(int dev_id, unsigned char *mac);

#if defined(CONFIG_MX6) || defined(CONFIG_MX7) || defined(CONFIG_MX7ULP)
void enable_ca7_smp(void);
#endif

enum boot_device get_boot_device(void);

#endif
