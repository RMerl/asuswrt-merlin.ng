// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/cpu.h>
#include <asm/gpio.h>
#include <asm/intel_regs.h>
#include <asm/mrc_common.h>
#include <asm/pch_common.h>
#include <asm/post.h>
#include <asm/arch/me.h>
#include <asm/report_platform.h>

static const char *const ecc_decoder[] = {
	"inactive",
	"active on IO",
	"disabled on IO",
	"active"
};

ulong mrc_common_board_get_usable_ram_top(ulong total_size)
{
	struct memory_info *info = &gd->arch.meminfo;
	uintptr_t dest_addr = 0;
	struct memory_area *largest = NULL;
	int i;

	/* Find largest area of memory below 4GB */

	for (i = 0; i < info->num_areas; i++) {
		struct memory_area *area = &info->area[i];

		if (area->start >= 1ULL << 32)
			continue;
		if (!largest || area->size > largest->size)
			largest = area;
	}

	/* If no suitable area was found, return an error. */
	assert(largest);
	if (!largest || largest->size < (2 << 20))
		panic("No available memory found for relocation");

	dest_addr = largest->start + largest->size;

	return (ulong)dest_addr;
}

void mrc_common_dram_init_banksize(void)
{
	struct memory_info *info = &gd->arch.meminfo;
	int num_banks;
	int i;

	for (i = 0, num_banks = 0; i < info->num_areas; i++) {
		struct memory_area *area = &info->area[i];

		if (area->start >= 1ULL << 32)
			continue;
		gd->bd->bi_dram[num_banks].start = area->start;
		gd->bd->bi_dram[num_banks].size = area->size;
		num_banks++;
	}
}

int mrc_add_memory_area(struct memory_info *info, uint64_t start,
			  uint64_t end)
{
	struct memory_area *ptr;

	if (info->num_areas == CONFIG_NR_DRAM_BANKS)
		return -ENOSPC;

	ptr = &info->area[info->num_areas];
	ptr->start = start;
	ptr->size = end - start;
	info->total_memory += ptr->size;
	if (ptr->start < (1ULL << 32))
		info->total_32bit_memory += ptr->size;
	debug("%d: memory %llx size %llx, total now %llx / %llx\n",
	      info->num_areas, ptr->start, ptr->size,
	      info->total_32bit_memory, info->total_memory);
	info->num_areas++;

	return 0;
}

/*
 * Dump in the log memory controller configuration as read from the memory
 * controller registers.
 */
void report_memory_config(void)
{
	u32 addr_decoder_common, addr_decode_ch[2];
	int i;

	addr_decoder_common = readl(MCHBAR_REG(0x5000));
	addr_decode_ch[0] = readl(MCHBAR_REG(0x5004));
	addr_decode_ch[1] = readl(MCHBAR_REG(0x5008));

	debug("memcfg DDR3 clock %d MHz\n",
	      (readl(MCHBAR_REG(0x5e04)) * 13333 * 2 + 50) / 100);
	debug("memcfg channel assignment: A: %d, B % d, C % d\n",
	      addr_decoder_common & 3,
	      (addr_decoder_common >> 2) & 3,
	      (addr_decoder_common >> 4) & 3);

	for (i = 0; i < ARRAY_SIZE(addr_decode_ch); i++) {
		u32 ch_conf = addr_decode_ch[i];
		debug("memcfg channel[%d] config (%8.8x):\n", i, ch_conf);
		debug("   ECC %s\n", ecc_decoder[(ch_conf >> 24) & 3]);
		debug("   enhanced interleave mode %s\n",
		      ((ch_conf >> 22) & 1) ? "on" : "off");
		debug("   rank interleave %s\n",
		      ((ch_conf >> 21) & 1) ? "on" : "off");
		debug("   DIMMA %d MB width x%d %s rank%s\n",
		      ((ch_conf >> 0) & 0xff) * 256,
		      ((ch_conf >> 19) & 1) ? 16 : 8,
		      ((ch_conf >> 17) & 1) ? "dual" : "single",
		      ((ch_conf >> 16) & 1) ? "" : ", selected");
		debug("   DIMMB %d MB width x%d %s rank%s\n",
		      ((ch_conf >> 8) & 0xff) * 256,
		      ((ch_conf >> 20) & 1) ? 16 : 8,
		      ((ch_conf >> 18) & 1) ? "dual" : "single",
		      ((ch_conf >> 16) & 1) ? ", selected" : "");
	}
}

int mrc_locate_spd(struct udevice *dev, int size, const void **spd_datap)
{
	const void *blob = gd->fdt_blob;
	int spd_index;
	struct gpio_desc desc[4];
	int spd_node;
	int node;
	int ret;

	ret = gpio_request_list_by_name(dev, "board-id-gpios", desc,
					ARRAY_SIZE(desc), GPIOD_IS_IN);
	if (ret < 0) {
		debug("%s: gpio ret=%d\n", __func__, ret);
		return ret;
	}
	spd_index = dm_gpio_get_values_as_int(desc, ret);
	debug("spd index %d\n", spd_index);

	node = fdt_first_subnode(blob, dev_of_offset(dev));
	if (node < 0)
		return -EINVAL;
	for (spd_node = fdt_first_subnode(blob, node);
	     spd_node > 0;
	     spd_node = fdt_next_subnode(blob, spd_node)) {
		int len;

		if (fdtdec_get_int(blob, spd_node, "reg", -1) != spd_index)
			continue;
		*spd_datap = fdt_getprop(blob, spd_node, "data", &len);
		if (len < size) {
			printf("Missing SPD data\n");
			return -EINVAL;
		}

		debug("Using SDRAM SPD data for '%s'\n",
		      fdt_get_name(blob, spd_node, NULL));
		return 0;
	}

	printf("No SPD data found for index %d\n", spd_index);
	return -ENOENT;
}

asmlinkage void sdram_console_tx_byte(unsigned char byte)
{
#ifdef DEBUG
	putc(byte);
#endif
}

/**
 * Find the PEI executable in the ROM and execute it.
 *
 * @me_dev: Management Engine device
 * @pei_data: configuration data for UEFI PEI reference code
 */
static int sdram_initialise(struct udevice *dev, struct udevice *me_dev,
			    void *pei_data, bool use_asm_linkage)
{
	unsigned version;
	const char *data;

	report_platform_info(dev);
	debug("Starting UEFI PEI System Agent\n");

	debug("PEI data at %p:\n", pei_data);

	data = (char *)CONFIG_X86_MRC_ADDR;
	if (data) {
		int rv;
		ulong start;

		debug("Calling MRC at %p\n", data);
		post_code(POST_PRE_MRC);
		start = get_timer(0);
		if (use_asm_linkage) {
			asmlinkage int (*func)(void *);

			func = (asmlinkage int (*)(void *))data;
			rv = func(pei_data);
		} else {
			int (*func)(void *);

			func = (int (*)(void *))data;
			rv = func(pei_data);
		}
		post_code(POST_MRC);
		if (rv) {
			switch (rv) {
			case -1:
				printf("PEI version mismatch.\n");
				break;
			case -2:
				printf("Invalid memory frequency.\n");
				break;
			default:
				printf("MRC returned %x.\n", rv);
			}
			printf("Nonzero MRC return value.\n");
			return -EFAULT;
		}
		debug("MRC execution time %lu ms\n", get_timer(start));
	} else {
		printf("UEFI PEI System Agent not found.\n");
		return -ENOSYS;
	}

	version = readl(MCHBAR_REG(MCHBAR_PEI_VERSION));
	debug("System Agent Version %d.%d.%d Build %d\n",
	      version >> 24 , (version >> 16) & 0xff,
	      (version >> 8) & 0xff, version & 0xff);

	return 0;
}

int mrc_common_init(struct udevice *dev, void *pei_data, bool use_asm_linkage)
{
	struct udevice *me_dev;
	int ret;

	ret = syscon_get_by_driver_data(X86_SYSCON_ME, &me_dev);
	if (ret)
		return ret;

	ret = sdram_initialise(dev, me_dev, pei_data, use_asm_linkage);
	if (ret)
		return ret;
	quick_ram_check();
	post_code(POST_DRAM);
	report_memory_config();

	return 0;
}
