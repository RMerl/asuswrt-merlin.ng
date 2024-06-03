// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 */

#include <common.h>
#include <sata.h>
#include <ahci.h>
#include <scsi.h>
#include <malloc.h>
#include <wdt.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/psu_init_gpl.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <zynqmppl.h>
#include <g_dnl.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_FPGA) && defined(CONFIG_FPGA_ZYNQMPPL) && \
    !defined(CONFIG_SPL_BUILD)
static xilinx_desc zynqmppl = XILINX_ZYNQMP_DESC;

static const struct {
	u32 id;
	u32 ver;
	char *name;
	bool evexists;
} zynqmp_devices[] = {
	{
		.id = 0x10,
		.name = "3eg",
	},
	{
		.id = 0x10,
		.ver = 0x2c,
		.name = "3cg",
	},
	{
		.id = 0x11,
		.name = "2eg",
	},
	{
		.id = 0x11,
		.ver = 0x2c,
		.name = "2cg",
	},
	{
		.id = 0x20,
		.name = "5ev",
		.evexists = 1,
	},
	{
		.id = 0x20,
		.ver = 0x100,
		.name = "5eg",
		.evexists = 1,
	},
	{
		.id = 0x20,
		.ver = 0x12c,
		.name = "5cg",
		.evexists = 1,
	},
	{
		.id = 0x21,
		.name = "4ev",
		.evexists = 1,
	},
	{
		.id = 0x21,
		.ver = 0x100,
		.name = "4eg",
		.evexists = 1,
	},
	{
		.id = 0x21,
		.ver = 0x12c,
		.name = "4cg",
		.evexists = 1,
	},
	{
		.id = 0x30,
		.name = "7ev",
		.evexists = 1,
	},
	{
		.id = 0x30,
		.ver = 0x100,
		.name = "7eg",
		.evexists = 1,
	},
	{
		.id = 0x30,
		.ver = 0x12c,
		.name = "7cg",
		.evexists = 1,
	},
	{
		.id = 0x38,
		.name = "9eg",
	},
	{
		.id = 0x38,
		.ver = 0x2c,
		.name = "9cg",
	},
	{
		.id = 0x39,
		.name = "6eg",
	},
	{
		.id = 0x39,
		.ver = 0x2c,
		.name = "6cg",
	},
	{
		.id = 0x40,
		.name = "11eg",
	},
	{ /* For testing purpose only */
		.id = 0x50,
		.ver = 0x2c,
		.name = "15cg",
	},
	{
		.id = 0x50,
		.name = "15eg",
	},
	{
		.id = 0x58,
		.name = "19eg",
	},
	{
		.id = 0x59,
		.name = "17eg",
	},
	{
		.id = 0x61,
		.name = "21dr",
	},
	{
		.id = 0x63,
		.name = "23dr",
	},
	{
		.id = 0x65,
		.name = "25dr",
	},
	{
		.id = 0x64,
		.name = "27dr",
	},
	{
		.id = 0x60,
		.name = "28dr",
	},
	{
		.id = 0x62,
		.name = "29dr",
	},
	{
		.id = 0x66,
		.name = "39dr",
	},
};
#endif

int chip_id(unsigned char id)
{
	struct pt_regs regs;
	int val = -EINVAL;

	if (current_el() != 3) {
		regs.regs[0] = ZYNQMP_SIP_SVC_CSU_DMA_CHIPID;
		regs.regs[1] = 0;
		regs.regs[2] = 0;
		regs.regs[3] = 0;

		smc_call(&regs);

		/*
		 * SMC returns:
		 * regs[0][31:0]  = status of the operation
		 * regs[0][63:32] = CSU.IDCODE register
		 * regs[1][31:0]  = CSU.version register
		 * regs[1][63:32] = CSU.IDCODE2 register
		 */
		switch (id) {
		case IDCODE:
			regs.regs[0] = upper_32_bits(regs.regs[0]);
			regs.regs[0] &= ZYNQMP_CSU_IDCODE_DEVICE_CODE_MASK |
					ZYNQMP_CSU_IDCODE_SVD_MASK;
			regs.regs[0] >>= ZYNQMP_CSU_IDCODE_SVD_SHIFT;
			val = regs.regs[0];
			break;
		case VERSION:
			regs.regs[1] = lower_32_bits(regs.regs[1]);
			regs.regs[1] &= ZYNQMP_CSU_SILICON_VER_MASK;
			val = regs.regs[1];
			break;
		case IDCODE2:
			regs.regs[1] = lower_32_bits(regs.regs[1]);
			regs.regs[1] >>= ZYNQMP_CSU_VERSION_EMPTY_SHIFT;
			val = regs.regs[1];
			break;
		default:
			printf("%s, Invalid Req:0x%x\n", __func__, id);
		}
	} else {
		switch (id) {
		case IDCODE:
			val = readl(ZYNQMP_CSU_IDCODE_ADDR);
			val &= ZYNQMP_CSU_IDCODE_DEVICE_CODE_MASK |
			       ZYNQMP_CSU_IDCODE_SVD_MASK;
			val >>= ZYNQMP_CSU_IDCODE_SVD_SHIFT;
			break;
		case VERSION:
			val = readl(ZYNQMP_CSU_VER_ADDR);
			val &= ZYNQMP_CSU_SILICON_VER_MASK;
			break;
		default:
			printf("%s, Invalid Req:0x%x\n", __func__, id);
		}
	}

	return val;
}

#define ZYNQMP_VERSION_SIZE		9
#define ZYNQMP_PL_STATUS_BIT		9
#define ZYNQMP_IPDIS_VCU_BIT		8
#define ZYNQMP_PL_STATUS_MASK		BIT(ZYNQMP_PL_STATUS_BIT)
#define ZYNQMP_CSU_VERSION_MASK		~(ZYNQMP_PL_STATUS_MASK)
#define ZYNQMP_CSU_VCUDIS_VER_MASK	ZYNQMP_CSU_VERSION_MASK & \
					~BIT(ZYNQMP_IPDIS_VCU_BIT)
#define MAX_VARIANTS_EV			3

#if defined(CONFIG_FPGA) && defined(CONFIG_FPGA_ZYNQMPPL) && \
	!defined(CONFIG_SPL_BUILD)
static char *zynqmp_get_silicon_idcode_name(void)
{
	u32 i, id, ver, j;
	char *buf;
	static char name[ZYNQMP_VERSION_SIZE];

	id = chip_id(IDCODE);
	ver = chip_id(IDCODE2);

	for (i = 0; i < ARRAY_SIZE(zynqmp_devices); i++) {
		if (zynqmp_devices[i].id == id) {
			if (zynqmp_devices[i].evexists &&
			    !(ver & ZYNQMP_PL_STATUS_MASK))
				break;
			if (zynqmp_devices[i].ver == (ver &
			    ZYNQMP_CSU_VERSION_MASK))
				break;
		}
	}

	if (i >= ARRAY_SIZE(zynqmp_devices))
		return "unknown";

	strncat(name, "zu", 2);
	if (!zynqmp_devices[i].evexists ||
	    (ver & ZYNQMP_PL_STATUS_MASK)) {
		strncat(name, zynqmp_devices[i].name,
			ZYNQMP_VERSION_SIZE - 3);
		return name;
	}

	/*
	 * Here we are means, PL not powered up and ev variant
	 * exists. So, we need to ignore VCU disable bit(8) in
	 * version and findout if its CG or EG/EV variant.
	 */
	for (j = 0; j < MAX_VARIANTS_EV; j++, i++) {
		if ((zynqmp_devices[i].ver & ~BIT(ZYNQMP_IPDIS_VCU_BIT)) ==
		    (ver & ZYNQMP_CSU_VCUDIS_VER_MASK)) {
			strncat(name, zynqmp_devices[i].name,
				ZYNQMP_VERSION_SIZE - 3);
			break;
		}
	}

	if (j >= MAX_VARIANTS_EV)
		return "unknown";

	if (strstr(name, "eg") || strstr(name, "ev")) {
		buf = strstr(name, "e");
		*buf = '\0';
	}

	return name;
}
#endif

int board_early_init_f(void)
{
	int ret = 0;
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_CLK_ZYNQMP)
	u32 pm_api_version;

	pm_api_version = zynqmp_pmufw_version();
	printf("PMUFW:\tv%d.%d\n",
	       pm_api_version >> ZYNQMP_PM_VERSION_MAJOR_SHIFT,
	       pm_api_version & ZYNQMP_PM_VERSION_MINOR_MASK);

	if (pm_api_version < ZYNQMP_PM_VERSION)
		panic("PMUFW version error. Expected: v%d.%d\n",
		      ZYNQMP_PM_VERSION_MAJOR, ZYNQMP_PM_VERSION_MINOR);
#endif

#if defined(CONFIG_ZYNQMP_PSU_INIT_ENABLED)
	ret = psu_init();
#endif

	return ret;
}

int board_init(void)
{
	printf("EL Level:\tEL%d\n", current_el());

#if defined(CONFIG_FPGA) && defined(CONFIG_FPGA_ZYNQMPPL) && \
    !defined(CONFIG_SPL_BUILD) || (defined(CONFIG_SPL_FPGA_SUPPORT) && \
    defined(CONFIG_SPL_BUILD))
	if (current_el() != 3) {
		zynqmppl.name = zynqmp_get_silicon_idcode_name();
		printf("Chip ID:\t%s\n", zynqmppl.name);
		fpga_init();
		fpga_add(fpga_xilinx, &zynqmppl);
	}
#endif

	return 0;
}

int board_early_init_r(void)
{
	u32 val;

	if (current_el() != 3)
		return 0;

	val = readl(&crlapb_base->timestamp_ref_ctrl);
	val &= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;

	if (!val) {
		val = readl(&crlapb_base->timestamp_ref_ctrl);
		val |= ZYNQMP_CRL_APB_TIMESTAMP_REF_CTRL_CLKACT;
		writel(val, &crlapb_base->timestamp_ref_ctrl);

		/* Program freq register in System counter */
		writel(zynqmp_get_system_timer_freq(),
		       &iou_scntr_secure->base_frequency_id_register);
		/* And enable system counter */
		writel(ZYNQMP_IOU_SCNTR_COUNTER_CONTROL_REGISTER_EN,
		       &iou_scntr_secure->counter_control_register);
	}
	return 0;
}

unsigned long do_go_exec(ulong (*entry)(int, char * const []), int argc,
			 char * const argv[])
{
	int ret = 0;

	if (current_el() > 1) {
		smp_kick_all_cpus();
		dcache_disable();
		armv8_switch_to_el1(0x0, 0, 0, 0, (unsigned long)entry,
				    ES_TO_AARCH64);
	} else {
		printf("FAIL: current EL is not above EL1\n");
		ret = EINVAL;
	}
	return ret;
}

#if !defined(CONFIG_SYS_SDRAM_BASE) && !defined(CONFIG_SYS_SDRAM_SIZE)
int dram_init_banksize(void)
{
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		return ret;

	mem_map_fill();

	return 0;
}

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}
#else
int dram_init_banksize(void)
{
#if defined(CONFIG_NR_DRAM_BANKS)
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = get_effective_memsize();
#endif

	mem_map_fill();

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);

	return 0;
}
#endif

void reset_cpu(ulong addr)
{
}

#if defined(CONFIG_BOARD_LATE_INIT)
static const struct {
	u32 bit;
	const char *name;
} reset_reasons[] = {
	{ RESET_REASON_DEBUG_SYS, "DEBUG" },
	{ RESET_REASON_SOFT, "SOFT" },
	{ RESET_REASON_SRST, "SRST" },
	{ RESET_REASON_PSONLY, "PS-ONLY" },
	{ RESET_REASON_PMU, "PMU" },
	{ RESET_REASON_INTERNAL, "INTERNAL" },
	{ RESET_REASON_EXTERNAL, "EXTERNAL" },
	{}
};

static int reset_reason(void)
{
	u32 reg;
	int i, ret;
	const char *reason = NULL;

	ret = zynqmp_mmio_read((ulong)&crlapb_base->reset_reason, &reg);
	if (ret)
		return -EINVAL;

	puts("Reset reason:\t");

	for (i = 0; i < ARRAY_SIZE(reset_reasons); i++) {
		if (reg & reset_reasons[i].bit) {
			reason = reset_reasons[i].name;
			printf("%s ", reset_reasons[i].name);
			break;
		}
	}

	puts("\n");

	env_set("reset_reason", reason);

	ret = zynqmp_mmio_write(~0, ~0, (ulong)&crlapb_base->reset_reason);
	if (ret)
		return -EINVAL;

	return ret;
}

static int set_fdtfile(void)
{
	char *compatible, *fdtfile;
	const char *suffix = ".dtb";
	const char *vendor = "xilinx/";

	if (env_get("fdtfile"))
		return 0;

	compatible = (char *)fdt_getprop(gd->fdt_blob, 0, "compatible", NULL);
	if (compatible) {
		debug("Compatible: %s\n", compatible);

		/* Discard vendor prefix */
		strsep(&compatible, ",");

		fdtfile = calloc(1, strlen(vendor) + strlen(compatible) +
				 strlen(suffix) + 1);
		if (!fdtfile)
			return -ENOMEM;

		sprintf(fdtfile, "%s%s%s", vendor, compatible, suffix);

		env_set("fdtfile", fdtfile);
		free(fdtfile);
	}

	return 0;
}

int board_late_init(void)
{
	u32 reg = 0;
	u8 bootmode;
	struct udevice *dev;
	int bootseq = -1;
	int bootseq_len = 0;
	int env_targets_len = 0;
	const char *mode;
	char *new_targets;
	char *env_targets;
	int ret;

#if defined(CONFIG_USB_ETHER) && !defined(CONFIG_USB_GADGET_DOWNLOAD)
	usb_ether_init();
#endif

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	ret = set_fdtfile();
	if (ret)
		return ret;

	ret = zynqmp_mmio_read((ulong)&crlapb_base->boot_mode, &reg);
	if (ret)
		return -EINVAL;

	if (reg >> BOOT_MODE_ALT_SHIFT)
		reg >>= BOOT_MODE_ALT_SHIFT;

	bootmode = reg & BOOT_MODES_MASK;

	puts("Bootmode: ");
	switch (bootmode) {
	case USB_MODE:
		puts("USB_MODE\n");
		mode = "usb";
		env_set("modeboot", "usb_dfu_spl");
		break;
	case JTAG_MODE:
		puts("JTAG_MODE\n");
		mode = "pxe dhcp";
		env_set("modeboot", "jtagboot");
		break;
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		mode = "qspi0";
		puts("QSPI_MODE\n");
		env_set("modeboot", "qspiboot");
		break;
	case EMMC_MODE:
		puts("EMMC_MODE\n");
		mode = "mmc0";
		env_set("modeboot", "emmcboot");
		break;
	case SD_MODE:
		puts("SD_MODE\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@ff160000", &dev) &&
		    uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@ff160000", &dev)) {
			puts("Boot from SD0 but without SD0 enabled!\n");
			return -1;
		}
		debug("mmc0 device found at %p, seq %d\n", dev, dev->seq);

		mode = "mmc";
		bootseq = dev->seq;
		env_set("modeboot", "sdboot");
		break;
	case SD1_LSHFT_MODE:
		puts("LVL_SHFT_");
		/* fall through */
	case SD_MODE1:
		puts("SD_MODE1\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@ff170000", &dev) &&
		    uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@ff170000", &dev)) {
			puts("Boot from SD1 but without SD1 enabled!\n");
			return -1;
		}
		debug("mmc1 device found at %p, seq %d\n", dev, dev->seq);

		mode = "mmc";
		bootseq = dev->seq;
		env_set("modeboot", "sdboot");
		break;
	case NAND_MODE:
		puts("NAND_MODE\n");
		mode = "nand0";
		env_set("modeboot", "nandboot");
		break;
	default:
		mode = "";
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	if (bootseq >= 0) {
		bootseq_len = snprintf(NULL, 0, "%i", bootseq);
		debug("Bootseq len: %x\n", bootseq_len);
	}

	/*
	 * One terminating char + one byte for space between mode
	 * and default boot_targets
	 */
	env_targets = env_get("boot_targets");
	if (env_targets)
		env_targets_len = strlen(env_targets);

	new_targets = calloc(1, strlen(mode) + env_targets_len + 2 +
			     bootseq_len);
	if (!new_targets)
		return -ENOMEM;

	if (bootseq >= 0)
		sprintf(new_targets, "%s%x %s", mode, bootseq,
			env_targets ? env_targets : "");
	else
		sprintf(new_targets, "%s %s", mode,
			env_targets ? env_targets : "");

	env_set("boot_targets", new_targets);

	reset_reason();

	return 0;
}
#endif

int checkboard(void)
{
	puts("Board: Xilinx ZynqMP\n");
	return 0;
}
