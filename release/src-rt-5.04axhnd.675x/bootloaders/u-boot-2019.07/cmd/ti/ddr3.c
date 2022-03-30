// SPDX-License-Identifier: GPL-2.0+
/*
 * EMIF: DDR3 test commands
 *
 * Copyright (C) 2012-2017 Texas Instruments Incorporated, <www.ti.com>
 */

#include <asm/arch/hardware.h>
#include <asm/cache.h>
#include <asm/emif.h>
#include <common.h>
#include <command.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_ARCH_KEYSTONE
#include <asm/arch/ddr3.h>
#define DDR_MIN_ADDR		CONFIG_SYS_SDRAM_BASE
#define STACKSIZE		(512 << 10)     /* 512 KiB */

#define DDR_REMAP_ADDR		0x80000000
#define ECC_START_ADDR1		((DDR_MIN_ADDR - DDR_REMAP_ADDR) >> 17)

#define ECC_END_ADDR1		(((gd->start_addr_sp - DDR_REMAP_ADDR - \
				 STACKSIZE) >> 17) - 2)
#endif

#define DDR_TEST_BURST_SIZE	1024

static int ddr_memory_test(u32 start_address, u32 end_address, int quick)
{
	u32 index_start, value, index;

	index_start = start_address;

	while (1) {
		/* Write a pattern */
		for (index = index_start;
				index < index_start + DDR_TEST_BURST_SIZE;
				index += 4)
			__raw_writel(index, index);

		/* Read and check the pattern */
		for (index = index_start;
				index < index_start + DDR_TEST_BURST_SIZE;
				index += 4) {
			value = __raw_readl(index);
			if (value != index) {
				printf("ddr_memory_test: Failed at address index = 0x%x value = 0x%x *(index) = 0x%x\n",
				       index, value, __raw_readl(index));

				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if (index_start >= end_address)
			break;

		if (quick)
			continue;

		/* Write a pattern for complementary values */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 4)
			__raw_writel((u32)~index, index);

		/* Read and check the pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 4) {
			value = __raw_readl(index);
			if (value != ~index) {
				printf("ddr_memory_test: Failed at address index = 0x%x value = 0x%x *(index) = 0x%x\n",
				       index, value, __raw_readl(index));

				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if (index_start >= end_address)
			break;

		/* Write a pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 2)
			__raw_writew((u16)index, index);

		/* Read and check the pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 2) {
			value = __raw_readw(index);
			if (value != (u16)index) {
				printf("ddr_memory_test: Failed at address index = 0x%x value = 0x%x *(index) = 0x%x\n",
				       index, value, __raw_readw(index));

				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if (index_start >= end_address)
			break;

		/* Write a pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 1)
			__raw_writeb((u8)index, index);

		/* Read and check the pattern */
		for (index = index_start;
		     index < index_start + DDR_TEST_BURST_SIZE;
		     index += 1) {
			value = __raw_readb(index);
			if (value != (u8)index) {
				printf("ddr_memory_test: Failed at address index = 0x%x value = 0x%x *(index) = 0x%x\n",
				       index, value, __raw_readb(index));

				return -1;
			}
		}

		index_start += DDR_TEST_BURST_SIZE;
		if (index_start >= end_address)
			break;
	}

	puts("ddr memory test PASSED!\n");
	return 0;
}

static int ddr_memory_compare(u32 address1, u32 address2, u32 size)
{
	u32 index, value, index2, value2;

	for (index = address1, index2 = address2;
	     index < address1 + size;
	     index += 4, index2 += 4) {
		value = __raw_readl(index);
		value2 = __raw_readl(index2);

		if (value != value2) {
			printf("ddr_memory_test: Compare failed at address = 0x%x value = 0x%x, address2 = 0x%x value2 = 0x%x\n",
			       index, value, index2, value2);

			return -1;
		}
	}

	puts("ddr memory compare PASSED!\n");
	return 0;
}

static void ddr_check_ecc_status(void)
{
	struct emif_reg_struct *emif = (struct emif_reg_struct *)EMIF1_BASE;
	u32 err_1b = readl(&emif->emif_1b_ecc_err_cnt);
	u32 int_status = readl(&emif->emif_irqstatus_raw_sys);
	int ecc_test = 0;
	char *env;

	env = env_get("ecc_test");
	if (env)
		ecc_test = simple_strtol(env, NULL, 0);

	puts("ECC test Status:\n");
	if (int_status & EMIF_INT_WR_ECC_ERR_SYS_MASK)
		puts("\tECC test: DDR ECC write error interrupted\n");

	if (int_status & EMIF_INT_TWOBIT_ECC_ERR_SYS_MASK)
		if (!ecc_test)
			panic("\tECC test: DDR ECC 2-bit error interrupted");

	if (int_status & EMIF_INT_ONEBIT_ECC_ERR_SYS_MASK)
		puts("\tECC test: DDR ECC 1-bit error interrupted\n");

	if (err_1b)
		printf("\tECC test: 1-bit ECC err count: 0x%x\n", err_1b);
}

static int ddr_memory_ecc_err(u32 addr, u32 ecc_err)
{
	struct emif_reg_struct *emif = (struct emif_reg_struct *)EMIF1_BASE;
	u32 ecc_ctrl = readl(&emif->emif_ecc_ctrl_reg);
	u32 val1, val2, val3;

	debug("Disabling D-Cache before ECC test\n");
	dcache_disable();
	invalidate_dcache_all();

	puts("Testing DDR ECC:\n");
	puts("\tECC test: Disabling DDR ECC ...\n");
	writel(0, &emif->emif_ecc_ctrl_reg);

	val1 = readl(addr);
	val2 = val1 ^ ecc_err;
	writel(val2, addr);

	val3 = readl(addr);
	printf("\tECC test: addr 0x%x, read data 0x%x, written data 0x%x, err pattern: 0x%x, read after write data 0x%x\n",
	       addr, val1, val2, ecc_err, val3);

	puts("\tECC test: Enabling DDR ECC ...\n");
#ifdef CONFIG_ARCH_KEYSTONE
	ecc_ctrl = ECC_START_ADDR1 | (ECC_END_ADDR1 << 16);
	writel(ecc_ctrl, EMIF1_BASE + KS2_DDR3_ECC_ADDR_RANGE1_OFFSET);
	ddr3_enable_ecc(EMIF1_BASE, 1);
#else
	writel(ecc_ctrl, &emif->emif_ecc_ctrl_reg);
#endif

	val1 = readl(addr);
	printf("\tECC test: addr 0x%x, read data 0x%x\n", addr, val1);

	ddr_check_ecc_status();

	debug("Enabling D-cache back after ECC test\n");
	enable_caches();

	return 0;
}

static int is_addr_valid(u32 addr)
{
	struct emif_reg_struct *emif = (struct emif_reg_struct *)EMIF1_BASE;
	u32 start_addr, end_addr, range, ecc_ctrl;

#ifdef CONFIG_ARCH_KEYSTONE
	ecc_ctrl = EMIF_ECC_REG_ECC_ADDR_RGN_1_EN_MASK;
	range = ECC_START_ADDR1 | (ECC_END_ADDR1 << 16);
#else
	ecc_ctrl = readl(&emif->emif_ecc_ctrl_reg);
	range = readl(&emif->emif_ecc_address_range_1);
#endif

	/* Check in ecc address range 1 */
	if (ecc_ctrl & EMIF_ECC_REG_ECC_ADDR_RGN_1_EN_MASK) {
		start_addr = ((range & EMIF_ECC_REG_ECC_START_ADDR_MASK) << 16)
				+ CONFIG_SYS_SDRAM_BASE;
		end_addr = start_addr + (range & EMIF_ECC_REG_ECC_END_ADDR_MASK)
				+ 0xFFFF;
		if ((addr >= start_addr) && (addr <= end_addr))
			/* addr within ecc address range 1 */
			return 1;
	}

	/* Check in ecc address range 2 */
	if (ecc_ctrl & EMIF_ECC_REG_ECC_ADDR_RGN_2_EN_MASK) {
		range = readl(&emif->emif_ecc_address_range_2);
		start_addr = ((range & EMIF_ECC_REG_ECC_START_ADDR_MASK) << 16)
				+ CONFIG_SYS_SDRAM_BASE;
		end_addr = start_addr + (range & EMIF_ECC_REG_ECC_END_ADDR_MASK)
				+ 0xFFFF;
		if ((addr >= start_addr) && (addr <= end_addr))
			/* addr within ecc address range 2 */
			return 1;
	}

	return 0;
}

static int is_ecc_enabled(void)
{
	struct emif_reg_struct *emif = (struct emif_reg_struct *)EMIF1_BASE;
	u32 ecc_ctrl = readl(&emif->emif_ecc_ctrl_reg);

	return (ecc_ctrl & EMIF_ECC_CTRL_REG_ECC_EN_MASK) &&
		(ecc_ctrl & EMIF_ECC_REG_RMW_EN_MASK);
}

static int do_ddr_test(cmd_tbl_t *cmdtp,
		       int flag, int argc, char * const argv[])
{
	u32 start_addr, end_addr, size, ecc_err;

	if ((argc == 4) && (strncmp(argv[1], "ecc_err", 8) == 0)) {
		if (!is_ecc_enabled()) {
			puts("ECC not enabled. Please Enable ECC any try again\n");
			return CMD_RET_FAILURE;
		}

		start_addr = simple_strtoul(argv[2], NULL, 16);
		ecc_err = simple_strtoul(argv[3], NULL, 16);

		if (!is_addr_valid(start_addr)) {
			puts("Invalid address. Please enter ECC supported address!\n");
			return CMD_RET_FAILURE;
		}

		ddr_memory_ecc_err(start_addr, ecc_err);
		return 0;
	}

	if (!(((argc == 4) && (strncmp(argv[1], "test", 5) == 0)) ||
	      ((argc == 5) && (strncmp(argv[1], "compare", 8) == 0))))
		return cmd_usage(cmdtp);

	start_addr = simple_strtoul(argv[2], NULL, 16);
	end_addr = simple_strtoul(argv[3], NULL, 16);

	if ((start_addr < CONFIG_SYS_SDRAM_BASE) ||
	    (start_addr > (CONFIG_SYS_SDRAM_BASE +
	     get_effective_memsize() - 1)) ||
	    (end_addr < CONFIG_SYS_SDRAM_BASE) ||
	    (end_addr > (CONFIG_SYS_SDRAM_BASE +
	     get_effective_memsize() - 1)) || (start_addr >= end_addr)) {
		puts("Invalid start or end address!\n");
		return cmd_usage(cmdtp);
	}

	puts("Please wait ...\n");
	if (argc == 5) {
		size = simple_strtoul(argv[4], NULL, 16);
		ddr_memory_compare(start_addr, end_addr, size);
	} else {
		ddr_memory_test(start_addr, end_addr, 0);
	}

	return 0;
}

U_BOOT_CMD(ddr,	5, 1, do_ddr_test,
	   "DDR3 test",
	   "test <start_addr in hex> <end_addr in hex> - test DDR from start\n"
	   "	address to end address\n"
	   "ddr compare <start_addr in hex> <end_addr in hex> <size in hex> -\n"
	   "	compare DDR data of (size) bytes from start address to end\n"
	   "	address\n"
	   "ddr ecc_err <addr in hex> <bit_err in hex> - generate bit errors\n"
	   "	in DDR data at <addr>, the command will read a 32-bit data\n"
	   "	from <addr>, and write (data ^ bit_err) back to <addr>\n"
);
