// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Google, Inc
 */

#include <common.h>
#include <asm/msr.h>
#include <asm/mtrr.h>

static const char *const mtrr_type_name[MTRR_TYPE_COUNT] = {
	"Uncacheable",
	"Combine",
	"2",
	"3",
	"Through",
	"Protect",
	"Back",
};

static int do_mtrr_list(void)
{
	int i;

	printf("Reg Valid Write-type   %-16s %-16s %-16s\n", "Base   ||",
	       "Mask   ||", "Size   ||");
	for (i = 0; i < MTRR_COUNT; i++) {
		const char *type = "Invalid";
		uint64_t base, mask, size;
		bool valid;

		base = native_read_msr(MTRR_PHYS_BASE_MSR(i));
		mask = native_read_msr(MTRR_PHYS_MASK_MSR(i));
		size = ~mask & ((1ULL << CONFIG_CPU_ADDR_BITS) - 1);
		size |= (1 << 12) - 1;
		size += 1;
		valid = mask & MTRR_PHYS_MASK_VALID;
		type = mtrr_type_name[base & MTRR_BASE_TYPE_MASK];
		printf("%d   %-5s %-12s %016llx %016llx %016llx\n", i,
		       valid ? "Y" : "N", type, base & ~MTRR_BASE_TYPE_MASK,
		       mask & ~MTRR_PHYS_MASK_VALID, size);
	}

	return 0;
}

static int do_mtrr_set(uint reg, int argc, char * const argv[])
{
	const char *typename = argv[0];
	struct mtrr_state state;
	uint32_t start, size;
	uint64_t base, mask;
	int i, type = -1;
	bool valid;

	if (argc < 3)
		return CMD_RET_USAGE;
	for (i = 0; i < MTRR_TYPE_COUNT; i++) {
		if (*typename == *mtrr_type_name[i])
			type = i;
	}
	if (type == -1) {
		printf("Invalid type name %s\n", typename);
		return CMD_RET_USAGE;
	}
	start = simple_strtoul(argv[1], NULL, 16);
	size = simple_strtoul(argv[2], NULL, 16);

	base = start | type;
	valid = native_read_msr(MTRR_PHYS_MASK_MSR(reg)) & MTRR_PHYS_MASK_VALID;
	mask = ~((uint64_t)size - 1);
	mask &= (1ULL << CONFIG_CPU_ADDR_BITS) - 1;
	if (valid)
		mask |= MTRR_PHYS_MASK_VALID;

	printf("base=%llx, mask=%llx\n", base, mask);
	mtrr_open(&state, true);
	wrmsrl(MTRR_PHYS_BASE_MSR(reg), base);
	wrmsrl(MTRR_PHYS_MASK_MSR(reg), mask);
	mtrr_close(&state, true);

	return 0;
}

static int mtrr_set_valid(int reg, bool valid)
{
	struct mtrr_state state;
	uint64_t mask;

	mtrr_open(&state, true);
	mask = native_read_msr(MTRR_PHYS_MASK_MSR(reg));
	if (valid)
		mask |= MTRR_PHYS_MASK_VALID;
	else
		mask &= ~MTRR_PHYS_MASK_VALID;
	wrmsrl(MTRR_PHYS_MASK_MSR(reg), mask);
	mtrr_close(&state, true);

	return 0;
}

static int do_mtrr(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd;
	uint reg;

	cmd = argv[1];
	if (argc < 2 || *cmd == 'l')
		return do_mtrr_list();
	argc -= 2;
	argv += 2;
	if (argc <= 0)
		return CMD_RET_USAGE;
	reg = simple_strtoul(argv[0], NULL, 16);
	if (reg >= MTRR_COUNT) {
		printf("Invalid register number\n");
		return CMD_RET_USAGE;
	}
	if (*cmd == 'e')
		return mtrr_set_valid(reg, true);
	else if (*cmd == 'd')
		return mtrr_set_valid(reg, false);
	else if (*cmd == 's')
		return do_mtrr_set(reg, argc - 1, argv + 1);
	else
		return CMD_RET_USAGE;

	return 0;
}

U_BOOT_CMD(
	mtrr,	6,	1,	do_mtrr,
	"Use x86 memory type range registers (32-bit only)",
	"[list]        - list current registers\n"
	"set <reg> <type> <start> <size>   - set a register\n"
	"\t<type> is Uncacheable, Combine, Through, Protect, Back\n"
	"disable <reg>      - disable a register\n"
	"ensable <reg>      - enable a register"
);
