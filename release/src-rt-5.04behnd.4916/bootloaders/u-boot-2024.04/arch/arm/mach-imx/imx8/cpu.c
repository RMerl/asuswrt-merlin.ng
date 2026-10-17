// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018, 2021 NXP
 */

#include <common.h>
#include <clk.h>
#include <cpu.h>
#include <cpu_func.h>
#include <dm.h>
#include <event.h>
#include <init.h>
#include <log.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass.h>
#include <errno.h>
#include <spl.h>
#include <thermal.h>
#include <firmware/imx/sci/sci.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch-imx/cpu.h>
#include <asm/armv8/cpu.h>
#include <asm/armv8/mmu.h>
#include <asm/setup.h>
#include <asm/mach-imx/boot_mode.h>
#include <power-domain.h>
#include <elf.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

#define BT_PASSOVER_TAG	0x504F
struct pass_over_info_t *get_pass_over_info(void)
{
	struct pass_over_info_t *p =
		(struct pass_over_info_t *)PASS_OVER_INFO_ADDR;

	if (p->barker != BT_PASSOVER_TAG ||
	    p->len != sizeof(struct pass_over_info_t))
		return NULL;

	return p;
}

static char *get_reset_cause(void)
{
	sc_pm_reset_reason_t reason;

	if (sc_pm_reset_reason(-1, &reason) != SC_ERR_NONE)
		return "Unknown reset";

	switch (reason) {
	case SC_PM_RESET_REASON_POR:
		return "POR";
	case SC_PM_RESET_REASON_JTAG:
		return "JTAG reset ";
	case SC_PM_RESET_REASON_SW:
		return "Software reset";
	case SC_PM_RESET_REASON_WDOG:
		return "Watchdog reset";
	case SC_PM_RESET_REASON_LOCKUP:
		return "SCU lockup reset";
	case SC_PM_RESET_REASON_SNVS:
		return "SNVS reset";
	case SC_PM_RESET_REASON_TEMP:
		return "Temp panic reset";
	case SC_PM_RESET_REASON_MSI:
		return "MSI reset";
	case SC_PM_RESET_REASON_UECC:
		return "ECC reset";
	case SC_PM_RESET_REASON_SCFW_WDOG:
		return "SCFW watchdog reset";
	case SC_PM_RESET_REASON_ROM_WDOG:
		return "SCU ROM watchdog reset";
	case SC_PM_RESET_REASON_SECO:
		return "SECO reset";
	case SC_PM_RESET_REASON_SCFW_FAULT:
		return "SCFW fault reset";
	default:
		return "Unknown reset";
	}
}

int arch_cpu_init(void)
{
#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_RECOVER_DATA_SECTION)
	spl_save_restore_data();
#endif

#ifdef CONFIG_SPL_BUILD
	struct pass_over_info_t *pass_over;

	if (is_soc_rev(CHIP_REV_A)) {
		pass_over = get_pass_over_info();
		if (pass_over && pass_over->g_ap_mu == 0) {
			/*
			 * When ap_mu is 0, means the U-Boot booted
			 * from first container
			 */
			sc_misc_boot_status(-1, SC_MISC_BOOT_STATUS_SUCCESS);
		}
	}
#endif

	return 0;
}

static int imx8_init_mu(void)
{
	struct udevice *devp;
	int node, ret;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, -1, "fsl,imx8-mu");

	ret = uclass_get_device_by_of_offset(UCLASS_MISC, node, &devp);
	if (ret) {
		printf("could not get scu %d\n", ret);
		return ret;
	}

	if (is_imx8qm()) {
		ret = sc_pm_set_resource_power_mode(-1, SC_R_SMMU,
						    SC_PM_PW_MODE_ON);
		if (ret)
			return ret;
	}

	return 0;
}
EVENT_SPY_SIMPLE(EVT_DM_POST_INIT_F, imx8_init_mu);

#if defined(CONFIG_ARCH_MISC_INIT)
int arch_misc_init(void)
{
	if (IS_ENABLED(CONFIG_FSL_CAAM)) {
		struct udevice *dev;
		int ret;

		ret = uclass_get_device_by_driver(UCLASS_MISC, DM_DRIVER_GET(caam_jr), &dev);
		if (ret)
			printf("Failed to initialize caam_jr: %d\n", ret);
	}

	return 0;
}
#endif

#ifdef CONFIG_IMX_BOOTAUX

#ifdef CONFIG_IMX8QM
int arch_auxiliary_core_up(u32 core_id, ulong boot_private_data)
{
	sc_rsrc_t core_rsrc, mu_rsrc;
	sc_faddr_t tcml_addr;
	u32 tcml_size = SZ_128K;
	ulong addr;

	switch (core_id) {
	case 0:
		core_rsrc = SC_R_M4_0_PID0;
		tcml_addr = 0x34FE0000;
		mu_rsrc = SC_R_M4_0_MU_1A;
		break;
	case 1:
		core_rsrc = SC_R_M4_1_PID0;
		tcml_addr = 0x38FE0000;
		mu_rsrc = SC_R_M4_1_MU_1A;
		break;
	default:
		printf("Not support this core boot up, ID:%u\n", core_id);
		return -EINVAL;
	}

	addr = (sc_faddr_t)boot_private_data;

	if (addr >= tcml_addr && addr <= tcml_addr + tcml_size) {
		printf("Wrong image address 0x%lx, should not in TCML\n",
		       addr);
		return -EINVAL;
	}

	printf("Power on M4 and MU\n");

	if (sc_pm_set_resource_power_mode(-1, core_rsrc, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
		return -EIO;

	if (sc_pm_set_resource_power_mode(-1, mu_rsrc, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
		return -EIO;

	printf("Copy M4 image from 0x%lx to TCML 0x%lx\n", addr, (ulong)tcml_addr);

	if (addr != tcml_addr)
		memcpy((void *)tcml_addr, (void *)addr, tcml_size);

	printf("Start M4 %u\n", core_id);
	if (sc_pm_cpu_start(-1, core_rsrc, true, tcml_addr) != SC_ERR_NONE)
		return -EIO;

	printf("bootaux complete\n");
	return 0;
}
#endif

#ifdef CONFIG_IMX8QXP
int arch_auxiliary_core_up(u32 core_id, ulong boot_private_data)
{
	sc_rsrc_t core_rsrc, mu_rsrc = SC_R_NONE;
	sc_faddr_t aux_core_ram;
	u32 size;
	ulong addr;

	switch (core_id) {
	case 0:
		core_rsrc = SC_R_M4_0_PID0;
		aux_core_ram = 0x34FE0000;
		mu_rsrc = SC_R_M4_0_MU_1A;
		size = SZ_128K;
		break;
	case 1:
		core_rsrc = SC_R_DSP;
		aux_core_ram = 0x596f8000;
		size = SZ_2K;
		break;
	default:
		printf("Not support this core boot up, ID:%u\n", core_id);
		return -EINVAL;
	}

	addr = (sc_faddr_t)boot_private_data;

	if (addr >= aux_core_ram && addr <= aux_core_ram + size) {
		printf("Wrong image address 0x%lx, should not in aux core ram\n",
		       addr);
		return -EINVAL;
	}

	printf("Power on aux core %d\n", core_id);

	if (sc_pm_set_resource_power_mode(-1, core_rsrc, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
		return -EIO;

	if (mu_rsrc != SC_R_NONE) {
		if (sc_pm_set_resource_power_mode(-1, mu_rsrc, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
			return -EIO;
	}

	if (core_id == 1) {
		struct power_domain pd;

		if (sc_pm_clock_enable(-1, core_rsrc, SC_PM_CLK_PER, true, false) != SC_ERR_NONE) {
			printf("Error enable clock\n");
			return -EIO;
		}

		if (!power_domain_lookup_name("audio_sai0", &pd)) {
			if (power_domain_on(&pd)) {
				printf("Error power on SAI0\n");
				return -EIO;
			}
		}

		if (!power_domain_lookup_name("audio_ocram", &pd)) {
			if (power_domain_on(&pd)) {
				printf("Error power on HIFI RAM\n");
				return -EIO;
			}
		}
	}

	printf("Copy image from 0x%lx to 0x%lx\n", addr, (ulong)aux_core_ram);
	if (core_id == 0) {
		/* M4 use bin file */
		memcpy((void *)aux_core_ram, (void *)addr, size);
	} else {
		/* HIFI use elf file */
		if (!valid_elf_image(addr))
			return -1;
		addr = load_elf_image_shdr(addr);
	}

	printf("Start %s\n", core_id == 0 ? "M4" : "HIFI");

	if (sc_pm_cpu_start(-1, core_rsrc, true, aux_core_ram) != SC_ERR_NONE)
		return -EIO;

	printf("bootaux complete\n");
	return 0;
}
#endif

int arch_auxiliary_core_check_up(u32 core_id)
{
	sc_rsrc_t core_rsrc;
	sc_pm_power_mode_t power_mode;

	switch (core_id) {
	case 0:
		core_rsrc = SC_R_M4_0_PID0;
		break;
#ifdef CONFIG_IMX8QM
	case 1:
		core_rsrc = SC_R_M4_1_PID0;
		break;
#endif
	default:
		printf("Not support this core, ID:%u\n", core_id);
		return 0;
	}

	if (sc_pm_get_resource_power_mode(-1, core_rsrc, &power_mode) != SC_ERR_NONE)
		return 0;

	if (power_mode != SC_PM_PW_MODE_OFF)
		return 1;

	return 0;
}
#endif

int print_bootinfo(void)
{
	enum boot_device bt_dev = get_boot_device();

	puts("Boot:  ");
	switch (bt_dev) {
	case SD1_BOOT:
		puts("SD0\n");
		break;
	case SD2_BOOT:
		puts("SD1\n");
		break;
	case SD3_BOOT:
		puts("SD2\n");
		break;
	case MMC1_BOOT:
		puts("MMC0\n");
		break;
	case MMC2_BOOT:
		puts("MMC1\n");
		break;
	case MMC3_BOOT:
		puts("MMC2\n");
		break;
	case FLEXSPI_BOOT:
		puts("FLEXSPI\n");
		break;
	case SATA_BOOT:
		puts("SATA\n");
		break;
	case NAND_BOOT:
		puts("NAND\n");
		break;
	case USB_BOOT:
		puts("USB\n");
		break;
	default:
		printf("Unknown device %u\n", bt_dev);
		break;
	}

	printf("Reset cause: %s\n", get_reset_cause());

	return 0;
}

enum boot_device get_boot_device(void)
{
	enum boot_device boot_dev = SD1_BOOT;

	sc_rsrc_t dev_rsrc;

	sc_misc_get_boot_dev(-1, &dev_rsrc);

	switch (dev_rsrc) {
	case SC_R_SDHC_0:
		boot_dev = MMC1_BOOT;
		break;
	case SC_R_SDHC_1:
		boot_dev = SD2_BOOT;
		break;
	case SC_R_SDHC_2:
		boot_dev = SD3_BOOT;
		break;
	case SC_R_NAND:
		boot_dev = NAND_BOOT;
		break;
	case SC_R_FSPI_0:
		boot_dev = FLEXSPI_BOOT;
		break;
	case SC_R_SATA_0:
		boot_dev = SATA_BOOT;
		break;
	case SC_R_USB_0:
	case SC_R_USB_1:
	case SC_R_USB_2:
		boot_dev = USB_BOOT;
		break;
	default:
		break;
	}

	return boot_dev;
}

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
#define FUSE_UNIQUE_ID_WORD0 16
#define FUSE_UNIQUE_ID_WORD1 17
void get_board_serial(struct tag_serialnr *serialnr)
{
	int err;
	u32 val1 = 0, val2 = 0;
	u32 word1, word2;

	if (!serialnr)
		return;

	word1 = FUSE_UNIQUE_ID_WORD0;
	word2 = FUSE_UNIQUE_ID_WORD1;

	err = sc_misc_otp_fuse_read(-1, word1, &val1);
	if (err) {
		printf("%s fuse %d read error: %d\n", __func__, word1, err);
		return;
	}

	err = sc_misc_otp_fuse_read(-1, word2, &val2);
	if (err) {
		printf("%s fuse %d read error: %d\n", __func__, word2, err);
		return;
	}
	serialnr->low = val1;
	serialnr->high = val2;
}
#endif /*CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG*/

#ifdef CONFIG_ENV_IS_IN_MMC
__weak int board_mmc_get_env_dev(int devno)
{
	return CONFIG_SYS_MMC_ENV_DEV;
}

int mmc_get_env_dev(void)
{
	sc_rsrc_t dev_rsrc;
	int devno;

	sc_misc_get_boot_dev(-1, &dev_rsrc);

	switch (dev_rsrc) {
	case SC_R_SDHC_0:
		devno = 0;
		break;
	case SC_R_SDHC_1:
		devno = 1;
		break;
	case SC_R_SDHC_2:
		devno = 2;
		break;
	default:
		/* If not boot from sd/mmc, use default value */
		return CONFIG_SYS_MMC_ENV_DEV;
	}

	return board_mmc_get_env_dev(devno);
}
#endif

#define MEMSTART_ALIGNMENT  SZ_2M /* Align the memory start with 2MB */

static int get_owned_memreg(sc_rm_mr_t mr, sc_faddr_t *addr_start,
			    sc_faddr_t *addr_end)
{
	sc_faddr_t start, end;
	int ret;
	bool owned;

	owned = sc_rm_is_memreg_owned(-1, mr);
	if (owned) {
		ret = sc_rm_get_memreg_info(-1, mr, &start, &end);
		if (ret) {
			printf("Memreg get info failed, %d\n", ret);
			return -EINVAL;
		}
		debug("0x%llx -- 0x%llx\n", start, end);
		*addr_start = start;
		*addr_end = end;

		return 0;
	}

	return -EINVAL;
}

__weak void board_mem_get_layout(u64 *phys_sdram_1_start,
				 u64 *phys_sdram_1_size,
				 u64 *phys_sdram_2_start,
				 u64 *phys_sdram_2_size)
{
	*phys_sdram_1_start = PHYS_SDRAM_1;
	*phys_sdram_1_size = PHYS_SDRAM_1_SIZE;
	*phys_sdram_2_start = PHYS_SDRAM_2;
	*phys_sdram_2_size = PHYS_SDRAM_2_SIZE;
}

phys_size_t get_effective_memsize(void)
{
	sc_rm_mr_t mr;
	sc_faddr_t start, end, end1, start_aligned;
	u64 phys_sdram_1_start, phys_sdram_1_size;
	u64 phys_sdram_2_start, phys_sdram_2_size;
	int err;

	board_mem_get_layout(&phys_sdram_1_start, &phys_sdram_1_size,
			     &phys_sdram_2_start, &phys_sdram_2_size);


	end1 = (sc_faddr_t)phys_sdram_1_start + phys_sdram_1_size;
	for (mr = 0; mr < 64; mr++) {
		err = get_owned_memreg(mr, &start, &end);
		if (!err) {
			start_aligned = roundup(start, MEMSTART_ALIGNMENT);
			/* Too small memory region, not use it */
			if (start_aligned > end)
				continue;

			/* Find the memory region runs the U-Boot */
			if (start >= phys_sdram_1_start && start <= end1 &&
			    (start <= CONFIG_TEXT_BASE &&
			    end >= CONFIG_TEXT_BASE)) {
				if ((end + 1) <=
				    ((sc_faddr_t)phys_sdram_1_start +
				    phys_sdram_1_size))
					return (end - phys_sdram_1_start + 1);
				else
					return phys_sdram_1_size;
			}
		}
	}

	return phys_sdram_1_size;
}

int dram_init(void)
{
	sc_rm_mr_t mr;
	sc_faddr_t start, end, end1, end2;
	u64 phys_sdram_1_start, phys_sdram_1_size;
	u64 phys_sdram_2_start, phys_sdram_2_size;
	int err;

	board_mem_get_layout(&phys_sdram_1_start, &phys_sdram_1_size,
			     &phys_sdram_2_start, &phys_sdram_2_size);

	end1 = (sc_faddr_t)phys_sdram_1_start + phys_sdram_1_size;
	end2 = (sc_faddr_t)phys_sdram_2_start + phys_sdram_2_size;
	for (mr = 0; mr < 64; mr++) {
		err = get_owned_memreg(mr, &start, &end);
		if (!err) {
			start = roundup(start, MEMSTART_ALIGNMENT);
			/* Too small memory region, not use it */
			if (start > end)
				continue;

			if (start >= phys_sdram_1_start && start <= end1) {
				if ((end + 1) <= end1)
					gd->ram_size += end - start + 1;
				else
					gd->ram_size += end1 - start;
			} else if (start >= phys_sdram_2_start &&
				   start <= end2) {
				if ((end + 1) <= end2)
					gd->ram_size += end - start + 1;
				else
					gd->ram_size += end2 - start;
			}
		}
	}

	/* If error, set to the default value */
	if (!gd->ram_size) {
		gd->ram_size = phys_sdram_1_size;
		gd->ram_size += phys_sdram_2_size;
	}
	return 0;
}

static void dram_bank_sort(int current_bank)
{
	phys_addr_t start;
	phys_size_t size;

	while (current_bank > 0) {
		if (gd->bd->bi_dram[current_bank - 1].start >
		    gd->bd->bi_dram[current_bank].start) {
			start = gd->bd->bi_dram[current_bank - 1].start;
			size = gd->bd->bi_dram[current_bank - 1].size;

			gd->bd->bi_dram[current_bank - 1].start =
				gd->bd->bi_dram[current_bank].start;
			gd->bd->bi_dram[current_bank - 1].size =
				gd->bd->bi_dram[current_bank].size;

			gd->bd->bi_dram[current_bank].start = start;
			gd->bd->bi_dram[current_bank].size = size;
		}
		current_bank--;
	}
}

int dram_init_banksize(void)
{
	sc_rm_mr_t mr;
	sc_faddr_t start, end, end1, end2;
	int i = 0;
	u64 phys_sdram_1_start, phys_sdram_1_size;
	u64 phys_sdram_2_start, phys_sdram_2_size;
	int err;

	board_mem_get_layout(&phys_sdram_1_start, &phys_sdram_1_size,
			     &phys_sdram_2_start, &phys_sdram_2_size);

	end1 = (sc_faddr_t)phys_sdram_1_start + phys_sdram_1_size;
	end2 = (sc_faddr_t)phys_sdram_2_start + phys_sdram_2_size;
	for (mr = 0; mr < 64 && i < CONFIG_NR_DRAM_BANKS; mr++) {
		err = get_owned_memreg(mr, &start, &end);
		if (!err) {
			start = roundup(start, MEMSTART_ALIGNMENT);
			if (start > end) /* Small memory region, no use it */
				continue;

			if (start >= phys_sdram_1_start && start <= end1) {
				gd->bd->bi_dram[i].start = start;

				if ((end + 1) <= end1)
					gd->bd->bi_dram[i].size =
						end - start + 1;
				else
					gd->bd->bi_dram[i].size = end1 - start;

				dram_bank_sort(i);
				i++;
			} else if (start >= phys_sdram_2_start && start <= end2) {
				gd->bd->bi_dram[i].start = start;

				if ((end + 1) <= end2)
					gd->bd->bi_dram[i].size =
						end - start + 1;
				else
					gd->bd->bi_dram[i].size = end2 - start;

				dram_bank_sort(i);
				i++;
			}
		}
	}

	/* If error, set to the default value */
	if (!i) {
		gd->bd->bi_dram[0].start = phys_sdram_1_start;
		gd->bd->bi_dram[0].size = phys_sdram_1_size;
		gd->bd->bi_dram[1].start = phys_sdram_2_start;
		gd->bd->bi_dram[1].size = phys_sdram_2_size;
	}

	return 0;
}

static u64 get_block_attrs(sc_faddr_t addr_start)
{
	u64 attr = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
		PTE_BLOCK_PXN | PTE_BLOCK_UXN;
	u64 phys_sdram_1_start, phys_sdram_1_size;
	u64 phys_sdram_2_start, phys_sdram_2_size;

	board_mem_get_layout(&phys_sdram_1_start, &phys_sdram_1_size,
			     &phys_sdram_2_start, &phys_sdram_2_size);

	if ((addr_start >= phys_sdram_1_start &&
	     addr_start <= ((sc_faddr_t)phys_sdram_1_start + phys_sdram_1_size)) ||
	    (addr_start >= phys_sdram_2_start &&
	     addr_start <= ((sc_faddr_t)phys_sdram_2_start + phys_sdram_2_size)))
		return (PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE);

	return attr;
}

static u64 get_block_size(sc_faddr_t addr_start, sc_faddr_t addr_end)
{
	sc_faddr_t end1, end2;
	u64 phys_sdram_1_start, phys_sdram_1_size;
	u64 phys_sdram_2_start, phys_sdram_2_size;

	board_mem_get_layout(&phys_sdram_1_start, &phys_sdram_1_size,
			     &phys_sdram_2_start, &phys_sdram_2_size);


	end1 = (sc_faddr_t)phys_sdram_1_start + phys_sdram_1_size;
	end2 = (sc_faddr_t)phys_sdram_2_start + phys_sdram_2_size;

	if (addr_start >= phys_sdram_1_start && addr_start <= end1) {
		if ((addr_end + 1) > end1)
			return end1 - addr_start;
	} else if (addr_start >= phys_sdram_2_start && addr_start <= end2) {
		if ((addr_end + 1) > end2)
			return end2 - addr_start;
	}

	return (addr_end - addr_start + 1);
}

#define MAX_PTE_ENTRIES 512
#define MAX_MEM_MAP_REGIONS 16

static struct mm_region imx8_mem_map[MAX_MEM_MAP_REGIONS];
struct mm_region *mem_map = imx8_mem_map;

void enable_caches(void)
{
	sc_rm_mr_t mr;
	sc_faddr_t start, end;
	int err, i;

	/* Create map for registers access from 0x1c000000 to 0x80000000*/
	imx8_mem_map[0].virt = 0x1c000000UL;
	imx8_mem_map[0].phys = 0x1c000000UL;
	imx8_mem_map[0].size = 0x64000000UL;
	imx8_mem_map[0].attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN;

	i = 1;
	for (mr = 0; mr < 64 && i < MAX_MEM_MAP_REGIONS; mr++) {
		err = get_owned_memreg(mr, &start, &end);
		if (!err) {
			imx8_mem_map[i].virt = start;
			imx8_mem_map[i].phys = start;
			imx8_mem_map[i].size = get_block_size(start, end);
			imx8_mem_map[i].attrs = get_block_attrs(start);
			i++;
		}
	}

	if (i < MAX_MEM_MAP_REGIONS) {
		imx8_mem_map[i].size = 0;
		imx8_mem_map[i].attrs = 0;
	} else {
		puts("Error, need more MEM MAP REGIONS reserved\n");
		icache_enable();
		return;
	}

	for (i = 0; i < MAX_MEM_MAP_REGIONS; i++) {
		debug("[%d] vir = 0x%llx phys = 0x%llx size = 0x%llx attrs = 0x%llx\n",
		      i, imx8_mem_map[i].virt, imx8_mem_map[i].phys,
		      imx8_mem_map[i].size, imx8_mem_map[i].attrs);
	}

	icache_enable();
	dcache_enable();
}

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
u64 get_page_table_size(void)
{
	u64 one_pt = MAX_PTE_ENTRIES * sizeof(u64);
	u64 size = 0;

	/*
	 * For each memory region, the max table size:
	 * 2 level 3 tables + 2 level 2 tables + 1 level 1 table
	 */
	size = (2 + 2 + 1) * one_pt * MAX_MEM_MAP_REGIONS + one_pt;

	/*
	 * We need to duplicate our page table once to have an emergency pt to
	 * resort to when splitting page tables later on
	 */
	size *= 2;

	/*
	 * We may need to split page tables later on if dcache settings change,
	 * so reserve up to 4 (random pick) page tables for that.
	 */
	size += one_pt * 4;

	return size;
}
#endif

#if defined(CONFIG_IMX8QM)
#define FUSE_MAC0_WORD0 452
#define FUSE_MAC0_WORD1 453
#define FUSE_MAC1_WORD0 454
#define FUSE_MAC1_WORD1 455
#elif defined(CONFIG_IMX8QXP)
#define FUSE_MAC0_WORD0 708
#define FUSE_MAC0_WORD1 709
#define FUSE_MAC1_WORD0 710
#define FUSE_MAC1_WORD1 711
#endif

void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
	u32 word[2], val[2] = {};
	int i, ret;

	if (dev_id == 0) {
		word[0] = FUSE_MAC0_WORD0;
		word[1] = FUSE_MAC0_WORD1;
	} else {
		word[0] = FUSE_MAC1_WORD0;
		word[1] = FUSE_MAC1_WORD1;
	}

	for (i = 0; i < 2; i++) {
		ret = sc_misc_otp_fuse_read(-1, word[i], &val[i]);
		if (ret < 0)
			goto err;
	}

	mac[0] = val[0];
	mac[1] = val[0] >> 8;
	mac[2] = val[0] >> 16;
	mac[3] = val[0] >> 24;
	mac[4] = val[1];
	mac[5] = val[1] >> 8;

	debug("%s: MAC%d: %02x.%02x.%02x.%02x.%02x.%02x\n",
	      __func__, dev_id, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return;
err:
	printf("%s: fuse %d, err: %d\n", __func__, word[i], ret);
}

u32 get_cpu_rev(void)
{
	u32 id = 0, rev = 0;
	int ret;

	ret = sc_misc_get_control(-1, SC_R_SYSTEM, SC_C_ID, &id);
	if (ret)
		return 0;

	rev = (id >> 5)  & 0xf;
	id = (id & 0x1f) + MXC_SOC_IMX8;  /* Dummy ID for chip */

	return (id << 12) | rev;
}

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();

	if (spl_boot_list[0] == BOOT_DEVICE_SPI) {
		/* Check whether we own the flexspi0, if not, use NOR boot */
		if (!sc_rm_is_resource_owned(-1, SC_R_FSPI_0))
			spl_boot_list[0] = BOOT_DEVICE_NOR;
	}
}

bool m4_parts_booted(void)
{
	sc_rm_pt_t m4_parts[2];
	int err;

	err = sc_rm_get_resource_owner(-1, SC_R_M4_0_PID0, &m4_parts[0]);
	if (err) {
		printf("%s get resource [%d] owner error: %d\n", __func__,
		       SC_R_M4_0_PID0, err);
		return false;
	}

	if (sc_pm_is_partition_started(-1, m4_parts[0]))
		return true;

	if (is_imx8qm()) {
		err = sc_rm_get_resource_owner(-1, SC_R_M4_1_PID0, &m4_parts[1]);
		if (err) {
			printf("%s get resource [%d] owner error: %d\n",
			       __func__, SC_R_M4_1_PID0, err);
			return false;
		}

		if (sc_pm_is_partition_started(-1, m4_parts[1]))
			return true;
	}

	return false;
}
