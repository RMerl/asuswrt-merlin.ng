// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <dfu.h>
#include <env.h>
#include <env_internal.h>
#include <init.h>
#include <log.h>
#include <net.h>
#include <sata.h>
#include <ahci.h>
#include <scsi.h>
#include <soc.h>
#include <spl.h>
#include <malloc.h>
#include <memalign.h>
#include <wdt.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/psu_init_gpl.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <usb.h>
#include <dwc3-uboot.h>
#include <zynqmppl.h>
#include <zynqmp_firmware.h>
#include <g_dnl.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/sizes.h>
#include "../common/board.h"

#include "pm_cfg_obj.h"

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(FPGA) && defined(CONFIG_FPGA_ZYNQMPPL)
static xilinx_desc zynqmppl = {
	xilinx_zynqmp, csu_dma, 1, &zynqmp_op, 0, &zynqmp_op, NULL,
	ZYNQMP_FPGA_FLAGS
};
#endif

int __maybe_unused psu_uboot_init(void)
{
	int ret;

	ret = psu_init();
	if (ret)
		return ret;

	/*
	 * PS_SYSMON_ANALOG_BUS register determines mapping between SysMon
	 * supply sense channel to SysMon supply registers inside the IP.
	 * This register must be programmed to complete SysMon IP
	 * configuration. The default register configuration after
	 * power-up is incorrect. Hence, fix this by writing the
	 * correct value - 0x3210.
	 */
	writel(ZYNQMP_PS_SYSMON_ANALOG_BUS_VAL,
	       ZYNQMP_AMS_PS_SYSMON_ANALOG_BUS);

	/* Delay is required for clocks to be propagated */
	udelay(1000000);
	
	return 0;
}

#if !defined(CONFIG_SPL_BUILD)
# if defined(CONFIG_DEBUG_UART_BOARD_INIT)
void board_debug_uart_init(void)
{
#  if defined(CONFIG_ZYNQMP_PSU_INIT_ENABLED)
	psu_uboot_init();
#  endif
}
# endif

# if defined(CONFIG_BOARD_EARLY_INIT_F)
int board_early_init_f(void)
{
	int ret = 0;
#  if defined(CONFIG_ZYNQMP_PSU_INIT_ENABLED) && !defined(CONFIG_DEBUG_UART_BOARD_INIT)
	ret = psu_uboot_init();
#  endif
	return ret;
}
# endif
#endif

static int multi_boot(void)
{
	u32 multiboot = 0;
	int ret;

	ret = zynqmp_mmio_read((ulong)&csu_base->multi_boot, &multiboot);
	if (ret)
		return -EINVAL;

	return multiboot;
}

#if defined(CONFIG_SPL_BUILD)
static void restore_jtag(void)
{
	if (current_el() != 3)
		return;

	writel(CSU_JTAG_SEC_GATE_DISABLE, &csu_base->jtag_sec);
	writel(CSU_JTAG_DAP_ENABLE_DEBUG, &csu_base->jtag_dap_cfg);
	writel(CSU_JTAG_CHAIN_WR_SETUP, &csu_base->jtag_chain_status_wr);
	writel(CRLAPB_DBG_LPD_CTRL_SETUP_CLK, &crlapb_base->dbg_lpd_ctrl);
	writel(CRLAPB_RST_LPD_DBG_RESET, &crlapb_base->rst_lpd_dbg);
	writel(CSU_PCAP_PROG_RELEASE_PL, &csu_base->pcap_prog);
}
#endif

static void print_secure_boot(void)
{
	u32 status = 0;

	if (zynqmp_mmio_read((ulong)&csu_base->status, &status))
		return;

	printf("Secure Boot:\t%sauthenticated, %sencrypted\n",
	       status & ZYNQMP_CSU_STATUS_AUTHENTICATED ? "" : "not ",
	       status & ZYNQMP_CSU_STATUS_ENCRYPTED ? "" : "not ");
}

int board_init(void)
{
#if CONFIG_IS_ENABLED(FPGA) && defined(CONFIG_FPGA_ZYNQMPPL)
	struct udevice *soc;
	char name[SOC_MAX_STR_SIZE];
	int ret;
#endif

#if defined(CONFIG_SPL_BUILD)
	/* Check *at build time* if the filename is an non-empty string */
	if (sizeof(CONFIG_ZYNQMP_SPL_PM_CFG_OBJ_FILE) > 1)
		zynqmp_pmufw_load_config_object(zynqmp_pm_cfg_obj,
						zynqmp_pm_cfg_obj_size);
#endif

#if defined(CONFIG_ZYNQMP_FIRMWARE)
	struct udevice *dev;

	uclass_get_device_by_name(UCLASS_FIRMWARE, "power-management", &dev);
	if (!dev) {
		uclass_get_device_by_name(UCLASS_FIRMWARE, "zynqmp-power", &dev);
		if (!dev)
			panic("PMU Firmware device not found - Enable it");
	}
#endif

#if defined(CONFIG_SPL_BUILD)
	printf("Silicon version:\t%d\n", zynqmp_get_silicon_version());

	/* the CSU disables the JTAG interface when secure boot is enabled */
	if (CONFIG_IS_ENABLED(ZYNQMP_RESTORE_JTAG))
		restore_jtag();
#else
	if (CONFIG_IS_ENABLED(DM_I2C) && CONFIG_IS_ENABLED(I2C_EEPROM))
		xilinx_read_eeprom();
#endif

	printf("EL Level:\tEL%d\n", current_el());

#if CONFIG_IS_ENABLED(FPGA) && defined(CONFIG_FPGA_ZYNQMPPL)
	ret = soc_get(&soc);
	if (!ret) {
		ret = soc_get_machine(soc, name, sizeof(name));
		if (ret >= 0) {
			zynqmppl.name = strdup(name);
			fpga_init();
			fpga_add(fpga_xilinx, &zynqmppl);
		}
	}
#endif

	/* display secure boot information */
	print_secure_boot();
	if (current_el() == 3)
		printf("Multiboot:\t%d\n", multi_boot());

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
			 char *const argv[])
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

#if !defined(CFG_SYS_SDRAM_BASE) && !defined(CFG_SYS_SDRAM_SIZE)
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
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = get_effective_memsize();

	mem_map_fill();

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE,
				    CFG_SYS_SDRAM_SIZE);

	return 0;
}
#endif

#if !CONFIG_IS_ENABLED(SYSRESET)
void reset_cpu(void)
{
}
#endif

static u8 __maybe_unused zynqmp_get_bootmode(void)
{
	u8 bootmode;
	u32 reg = 0;
	int ret;

	ret = zynqmp_mmio_read((ulong)&crlapb_base->boot_mode, &reg);
	if (ret)
		return -EINVAL;

	debug("HW boot mode: %x\n", reg & BOOT_MODES_MASK);
	debug("ALT boot mode: %x\n", reg >> BOOT_MODE_ALT_SHIFT);

	if (reg >> BOOT_MODE_ALT_SHIFT)
		reg >>= BOOT_MODE_ALT_SHIFT;

	bootmode = reg & BOOT_MODES_MASK;

	return bootmode;
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

	return 0;
}

static int set_fdtfile(void)
{
	char *compatible, *fdtfile;
	const char *suffix = ".dtb";
	const char *vendor = "xilinx/";
	int fdt_compat_len;

	if (env_get("fdtfile"))
		return 0;

	compatible = (char *)fdt_getprop(gd->fdt_blob, 0, "compatible",
					 &fdt_compat_len);
	if (compatible && fdt_compat_len) {
		char *name;

		debug("Compatible: %s\n", compatible);

		name = strchr(compatible, ',');
		if (!name)
			return -EINVAL;

		name++;

		fdtfile = calloc(1, strlen(vendor) + strlen(name) +
				 strlen(suffix) + 1);
		if (!fdtfile)
			return -ENOMEM;

		sprintf(fdtfile, "%s%s%s", vendor, name, suffix);

		env_set("fdtfile", fdtfile);
		free(fdtfile);
	}

	return 0;
}

static int boot_targets_setup(void)
{
	u8 bootmode;
	struct udevice *dev;
	int bootseq = -1;
	int bootseq_len = 0;
	int env_targets_len = 0;
	const char *mode = NULL;
	char *new_targets;
	char *env_targets;

	bootmode = zynqmp_get_bootmode();

	puts("Bootmode: ");
	switch (bootmode) {
	case USB_MODE:
		puts("USB_MODE\n");
		mode = "usb_dfu0 usb_dfu1";
		env_set("modeboot", "usb_dfu_spl");
		break;
	case JTAG_MODE:
		puts("JTAG_MODE\n");
		mode = "jtag pxe dhcp";
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
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@ff160000", &dev) &&
		    uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@ff160000", &dev)) {
			debug("SD0 driver for SD0 device is not present\n");
			break;
		}
		debug("mmc0 device found at %p, seq %d\n", dev, dev_seq(dev));

		mode = "mmc";
		bootseq = dev_seq(dev);
		env_set("modeboot", "emmcboot");
		break;
	case SD_MODE:
		puts("SD_MODE\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@ff160000", &dev) &&
		    uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@ff160000", &dev)) {
			debug("SD0 driver for SD0 device is not present\n");
			break;
		}
		debug("mmc0 device found at %p, seq %d\n", dev, dev_seq(dev));

		mode = "mmc";
		bootseq = dev_seq(dev);
		env_set("modeboot", "sdboot");
		break;
	case SD1_LSHFT_MODE:
		puts("LVL_SHFT_");
		fallthrough;
	case SD_MODE1:
		puts("SD_MODE1\n");
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@ff170000", &dev) &&
		    uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@ff170000", &dev)) {
			debug("SD1 driver for SD1 device is not present\n");
			break;
		}
		debug("mmc1 device found at %p, seq %d\n", dev, dev_seq(dev));

		mode = "mmc";
		bootseq = dev_seq(dev);
		env_set("modeboot", "sdboot");
		break;
	case NAND_MODE:
		puts("NAND_MODE\n");
		mode = "nand0";
		env_set("modeboot", "nandboot");
		break;
	default:
		printf("Invalid Boot Mode:0x%x\n", bootmode);
		break;
	}

	if (mode) {
		if (bootseq >= 0) {
			bootseq_len = snprintf(NULL, 0, "%i", bootseq);
			debug("Bootseq len: %x\n", bootseq_len);
			env_set_hex("bootseq", bootseq);
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
		free(new_targets);
	}

	return 0;
}

int board_late_init(void)
{
	int ret, multiboot;

#if defined(CONFIG_USB_ETHER) && !defined(CONFIG_USB_GADGET_DOWNLOAD)
	usb_ether_init();
#endif

	if (!(gd->flags & GD_FLG_ENV_DEFAULT)) {
		debug("Saved variables - Skipping\n");
		return 0;
	}

	if (!IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG))
		return 0;

	ret = set_fdtfile();
	if (ret)
		return ret;

	multiboot = multi_boot();
	if (multiboot >= 0)
		env_set_hex("multiboot", multiboot);

	if (IS_ENABLED(CONFIG_DISTRO_DEFAULTS)) {
		ret = boot_targets_setup();
		if (ret)
			return ret;
	}

	reset_reason();

	return board_late_init_xilinx();
}
#endif

int checkboard(void)
{
	puts("Board: Xilinx ZynqMP\n");
	return 0;
}

int mmc_get_env_dev(void)
{
	struct udevice *dev;
	int bootseq = 0;

	switch (zynqmp_get_bootmode()) {
	case EMMC_MODE:
	case SD_MODE:
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@ff160000", &dev) &&
		    uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@ff160000", &dev)) {
			return -1;
		}
		bootseq = dev_seq(dev);
		break;
	case SD1_LSHFT_MODE:
	case SD_MODE1:
		if (uclass_get_device_by_name(UCLASS_MMC,
					      "mmc@ff170000", &dev) &&
		    uclass_get_device_by_name(UCLASS_MMC,
					      "sdhci@ff170000", &dev)) {
			return -1;
		}
		bootseq = dev_seq(dev);
		break;
	default:
		break;
	}

	debug("bootseq %d\n", bootseq);

	return bootseq;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 bootmode = zynqmp_get_bootmode();

	if (prio)
		return ENVL_UNKNOWN;

	switch (bootmode) {
	case EMMC_MODE:
	case SD_MODE:
	case SD1_LSHFT_MODE:
	case SD_MODE1:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_FAT))
			return ENVL_FAT;
		if (IS_ENABLED(CONFIG_ENV_IS_IN_EXT4))
			return ENVL_EXT4;
		return ENVL_NOWHERE;
	case NAND_MODE:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_NAND))
			return ENVL_NAND;
		if (IS_ENABLED(CONFIG_ENV_IS_IN_UBI))
			return ENVL_UBI;
		return ENVL_NOWHERE;
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		if (IS_ENABLED(CONFIG_ENV_IS_IN_SPI_FLASH))
			return ENVL_SPI_FLASH;
		return ENVL_NOWHERE;
	case JTAG_MODE:
	default:
		return ENVL_NOWHERE;
	}
}

#if defined(CONFIG_SET_DFU_ALT_INFO)

#define DFU_ALT_BUF_LEN		SZ_1K

void set_dfu_alt_info(char *interface, char *devstr)
{
	int multiboot, bootseq = 0, len = 0;

	ALLOC_CACHE_ALIGN_BUFFER(char, buf, DFU_ALT_BUF_LEN);

	if (env_get("dfu_alt_info"))
		return;

	memset(buf, 0, sizeof(buf));

	multiboot = multi_boot();
	if (multiboot < 0)
		multiboot = 0;

	multiboot = env_get_hex("multiboot", multiboot);
	debug("Multiboot: %d\n", multiboot);

	switch (zynqmp_get_bootmode()) {
	case EMMC_MODE:
	case SD_MODE:
	case SD1_LSHFT_MODE:
	case SD_MODE1:
		bootseq = mmc_get_env_dev();

		len += snprintf(buf + len, DFU_ALT_BUF_LEN, "mmc %d=boot",
			       bootseq);

		if (multiboot)
			len += snprintf(buf + len, DFU_ALT_BUF_LEN,
				       "%04d", multiboot);

		len += snprintf(buf + len, DFU_ALT_BUF_LEN, ".bin fat %d 1",
			       bootseq);
#if defined(CONFIG_SPL_FS_LOAD_PAYLOAD_NAME)
		len += snprintf(buf + len, DFU_ALT_BUF_LEN, ";%s fat %d 1",
			       CONFIG_SPL_FS_LOAD_PAYLOAD_NAME, bootseq);
#endif
		break;
	case QSPI_MODE_24BIT:
	case QSPI_MODE_32BIT:
		len += snprintf(buf + len, DFU_ALT_BUF_LEN,
			       "sf 0:0=boot.bin raw %x 0x1500000",
			       multiboot * SZ_32K);
#if defined(CONFIG_SPL_FS_LOAD_PAYLOAD_NAME) && defined(CONFIG_SYS_SPI_U_BOOT_OFFS)
		len += snprintf(buf + len, DFU_ALT_BUF_LEN,
			       ";%s raw 0x%x 0x500000",
			       CONFIG_SPL_FS_LOAD_PAYLOAD_NAME,
			       multiboot * SZ_32K + CONFIG_SYS_SPI_U_BOOT_OFFS);
#endif
		break;
	default:
		return;
	}

	env_set("dfu_alt_info", buf);
	puts("DFU alt info setting: done\n");
}
#endif

#if defined(CONFIG_SPL_SPI_LOAD)
unsigned int spl_spi_get_uboot_offs(struct spi_flash *flash)
{
	u32 offset;
	int multiboot = multi_boot();

	offset = multiboot * SZ_32K;
	offset += CONFIG_SYS_SPI_U_BOOT_OFFS;

	log_info("SPI offset:\t0x%x\n", offset);

	return offset;
}
#endif
