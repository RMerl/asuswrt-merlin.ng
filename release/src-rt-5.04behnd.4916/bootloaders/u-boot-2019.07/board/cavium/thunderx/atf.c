// SPDX-License-Identifier: GPL-2.0+
/**
 * (C) Copyright 2014, Cavium Inc.
**/

#include <common.h>
#include <asm/io.h>

#include <asm/system.h>
#include <cavium/thunderx_svc.h>
#include <cavium/atf.h>
#include <cavium/atf_part.h>

#include <asm/psci.h>

#include <malloc.h>

ssize_t atf_read_mmc(uintptr_t offset, void *buffer, size_t size)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_MMC_READ;
	regs.regs[1] = offset;
	regs.regs[2] = size;
	regs.regs[3] = (uintptr_t)buffer;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_read_nor(uintptr_t offset, void *buffer, size_t size)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_NOR_READ;
	regs.regs[1] = offset;
	regs.regs[2] = size;
	regs.regs[3] = (uintptr_t)buffer;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_get_pcount(void)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_PART_COUNT;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_get_part(struct storage_partition *part, unsigned int index)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_GET_PART;
	regs.regs[1] = (uintptr_t)part;
	regs.regs[2] = index;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_erase_nor(uintptr_t offset, size_t size)
{
	struct pt_regs regs;

	regs.regs[0] = THUNDERX_NOR_ERASE;
	regs.regs[1] = offset;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_write_nor(uintptr_t offset, const void *buffer, size_t size)
{
	struct pt_regs regs;

	regs.regs[0] = THUNDERX_NOR_WRITE;
	regs.regs[1] = offset;
	regs.regs[2] = size;
	regs.regs[3] = (uintptr_t)buffer;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_write_mmc(uintptr_t offset, const void *buffer, size_t size)
{
	struct pt_regs regs;

	regs.regs[0] = THUNDERX_MMC_WRITE;
	regs.regs[1] = offset;
	regs.regs[2] = size;
	regs.regs[3] = (uintptr_t)buffer;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_dram_size(unsigned int node)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_DRAM_SIZE;
	regs.regs[1] = node;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_node_count(void)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_NODE_COUNT;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_env_count(void)
{
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_ENV_COUNT;

	smc_call(&regs);

	return regs.regs[0];
}

ssize_t atf_env_string(size_t index, char *str)
{
	uint64_t *buf = (void *)str;
	struct pt_regs regs;
	regs.regs[0] = THUNDERX_ENV_STRING;
	regs.regs[1] = index;

	smc_call(&regs);

	if (regs.regs > 0) {
		buf[0] = regs.regs[0];
		buf[1] = regs.regs[1];
		buf[2] = regs.regs[2];
		buf[3] = regs.regs[3];

		return 1;
	} else {
		return regs.regs[0];
	}
}

#ifdef CONFIG_CMD_ATF

static void atf_print_ver(void)
{
	struct pt_regs regs;
	regs.regs[0] = ARM_STD_SVC_VERSION;

	smc_call(&regs);

	printf("ARM Std FW version: %ld.%ld\n", regs.regs[0], regs.regs[1]);

	regs.regs[0] = THUNDERX_SVC_VERSION;

	smc_call(&regs);

	printf("ThunderX OEM ver: %ld.%ld\n", regs.regs[0], regs.regs[1]);
}

static void atf_print_uid(void)
{
}

static void atf_print_part_table(void)
{
	size_t pcount;
	unsigned long i;
	int ret;
	char *ptype;

	struct storage_partition *part = (void *)CONFIG_SYS_LOWMEM_BASE;

	pcount = atf_get_pcount();

	printf("Partition count: %lu\n\n", pcount);
	printf("%10s %10s %10s\n", "Type", "Size", "Offset");

	for (i = 0; i < pcount; i++) {
		ret = atf_get_part(part, i);

		if (ret < 0) {
			printf("Uknown error while reading partition: %d\n",
			       ret);
			return;
		}

		switch (part->type) {
		case PARTITION_NBL1FW_REST:
			ptype = "NBL1FW";
			break;
		case PARTITION_BL2_BL31:
			ptype = "BL2_BL31";
			break;
		case PARTITION_UBOOT:
			ptype = "BOOTLDR";
			break;
		case PARTITION_KERNEL:
			ptype = "KERNEL";
			break;
		case PARTITION_DEVICE_TREE:
			ptype = "DEVTREE";
			break;
		default:
			ptype = "UNKNOWN";
		}
		printf("%10s %10d %10lx\n", ptype, part->size, part->offset);
	}
}

int do_atf(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ssize_t ret;
	size_t size, offset;
	void *buffer = 0;
	unsigned int index, node;
	char str[4 * sizeof(uint64_t)];

	if ((argc == 5) && !strcmp(argv[1], "readmmc")) {
		buffer = (void *)simple_strtoul(argv[2], NULL, 16);
		offset = simple_strtoul(argv[3], NULL, 10);
		size = simple_strtoul(argv[4], NULL, 10);

		ret = atf_read_mmc(offset, buffer, size);
	} else if ((argc == 5) && !strcmp(argv[1], "readnor")) {
		buffer = (void *)simple_strtoul(argv[2], NULL, 16);
		offset = simple_strtoul(argv[3], NULL, 10);
		size = simple_strtoul(argv[4], NULL, 10);

		ret = atf_read_nor(offset, buffer, size);
	} else if ((argc == 5) && !strcmp(argv[1], "writemmc")) {
		buffer = (void *)simple_strtoul(argv[2], NULL, 16);
		offset = simple_strtoul(argv[3], NULL, 10);
		size = simple_strtoul(argv[4], NULL, 10);

		ret = atf_write_mmc(offset, buffer, size);
	} else if ((argc == 5) && !strcmp(argv[1], "writenor")) {
		buffer = (void *)simple_strtoul(argv[2], NULL, 16);
		offset = simple_strtoul(argv[3], NULL, 10);
		size = simple_strtoul(argv[4], NULL, 10);

		ret = atf_write_nor(offset, buffer, size);
	} else if ((argc == 2) && !strcmp(argv[1], "part")) {
		atf_print_part_table();
	} else if ((argc == 4) && !strcmp(argv[1], "erasenor")) {
		offset = simple_strtoul(argv[2], NULL, 10);
		size = simple_strtoul(argv[3], NULL, 10);

		ret = atf_erase_nor(offset, size);
	} else if ((argc == 2) && !strcmp(argv[1], "envcount")) {
		ret = atf_env_count();
		printf("Number of environment strings: %zd\n", ret);
	} else if ((argc == 3) && !strcmp(argv[1], "envstring")) {
		index = simple_strtoul(argv[2], NULL, 10);
		ret = atf_env_string(index, str);
		if (ret > 0)
			printf("Environment string %d: %s\n", index, str);
		else
			printf("Return code: %zd\n", ret);
	} else if ((argc == 3) && !strcmp(argv[1], "dramsize")) {
		node = simple_strtoul(argv[2], NULL, 10);
		ret = atf_dram_size(node);
		printf("DRAM size: %zd Mbytes\n", ret >> 20);
	} else if ((argc == 2) && !strcmp(argv[1], "nodes")) {
		ret = atf_node_count();
		printf("Nodes count: %zd\n", ret);
	} else if ((argc == 2) && !strcmp(argv[1], "ver")) {
		atf_print_ver();
	} else if ((argc == 2) && !strcmp(argv[1], "uid")) {
		atf_print_uid();
	} else {
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(
	atf,   10,   1,     do_atf,
	"issue calls to ATF",
	"\t readmmc addr offset size - read MMC card\n"
	"\t readnor addr offset size - read NOR flash\n"
	"\t writemmc addr offset size - write MMC card\n"
	"\t writenor addr offset size - write NOR flash\n"
	"\t erasenor offset size - erase NOR flash\n"
	"\t nodes - number of nodes\n"
	"\t dramsize node - size of DRAM attached to node\n"
	"\t envcount - number of environment strings\n"
	"\t envstring index - print the environment string\n"
	"\t part - print MMC partition table\n"
	"\t ver - print ATF call set versions\n"
);

#endif
