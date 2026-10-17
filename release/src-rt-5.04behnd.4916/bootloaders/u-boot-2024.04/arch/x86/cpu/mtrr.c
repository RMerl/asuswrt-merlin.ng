// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 Google, Inc
 *
 * Memory Type Range Regsters - these are used to tell the CPU whether
 * memory is cacheable and if so the cache write mode to use.
 *
 * These can speed up booting. See the mtrr command.
 *
 * Reference: Intel Architecture Software Developer's Manual, Volume 3:
 * System Programming
 */

/*
 * Note that any console output (e.g. debug()) in this file will likely fail
 * since the MTRR registers are sometimes in flux.
 */

#include <common.h>
#include <cpu_func.h>
#include <log.h>
#include <sort.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mp.h>
#include <asm/msr.h>
#include <asm/mtrr.h>
#include <linux/log2.h>

DECLARE_GLOBAL_DATA_PTR;

static const char *const mtrr_type_name[MTRR_TYPE_COUNT] = {
	"Uncacheable",
	"Combine",
	"2",
	"3",
	"Through",
	"Protect",
	"Back",
};

/* Prepare to adjust MTRRs */
void mtrr_open(struct mtrr_state *state, bool do_caches)
{
	if (!gd->arch.has_mtrr)
		return;

	if (do_caches) {
		state->enable_cache = dcache_status();

		if (state->enable_cache)
			disable_caches();
	}
	state->deftype = native_read_msr(MTRR_DEF_TYPE_MSR);
	wrmsrl(MTRR_DEF_TYPE_MSR, state->deftype & ~MTRR_DEF_TYPE_EN);
}

/* Clean up after adjusting MTRRs, and enable them */
void mtrr_close(struct mtrr_state *state, bool do_caches)
{
	if (!gd->arch.has_mtrr)
		return;

	wrmsrl(MTRR_DEF_TYPE_MSR, state->deftype | MTRR_DEF_TYPE_EN);
	if (do_caches && state->enable_cache)
		enable_caches();
}

static void set_var_mtrr(uint reg, uint type, uint64_t start, uint64_t size)
{
	u64 mask;

	wrmsrl(MTRR_PHYS_BASE_MSR(reg), start | type);
	mask = ~(size - 1);
	mask &= (1ULL << CONFIG_CPU_ADDR_BITS) - 1;
	wrmsrl(MTRR_PHYS_MASK_MSR(reg), mask | MTRR_PHYS_MASK_VALID);
}

void mtrr_read_all(struct mtrr_info *info)
{
	int reg_count = mtrr_get_var_count();
	int i;

	for (i = 0; i < reg_count; i++) {
		info->mtrr[i].base = native_read_msr(MTRR_PHYS_BASE_MSR(i));
		info->mtrr[i].mask = native_read_msr(MTRR_PHYS_MASK_MSR(i));
	}
}

void mtrr_write_all(struct mtrr_info *info)
{
	int reg_count = mtrr_get_var_count();
	struct mtrr_state state;
	int i;

	for (i = 0; i < reg_count; i++) {
		mtrr_open(&state, true);
		wrmsrl(MTRR_PHYS_BASE_MSR(i), info->mtrr[i].base);
		wrmsrl(MTRR_PHYS_MASK_MSR(i), info->mtrr[i].mask);
		mtrr_close(&state, true);
	}
}

static void write_mtrrs(void *arg)
{
	struct mtrr_info *info = arg;

	mtrr_write_all(info);
}

static void read_mtrrs(void *arg)
{
	struct mtrr_info *info = arg;

	mtrr_read_all(info);
}

/**
 * mtrr_copy_to_aps() - Copy the MTRRs from the boot CPU to other CPUs
 *
 * Return: 0 on success, -ve on failure
 */
static int mtrr_copy_to_aps(void)
{
	struct mtrr_info info;
	int ret;

	ret = mp_run_on_cpus(MP_SELECT_BSP, read_mtrrs, &info);
	if (ret == -ENXIO)
		return 0;
	else if (ret)
		return log_msg_ret("bsp", ret);

	ret = mp_run_on_cpus(MP_SELECT_APS, write_mtrrs, &info);
	if (ret)
		return log_msg_ret("bsp", ret);

	return 0;
}

static int h_comp_mtrr(const void *p1, const void *p2)
{
	const struct mtrr_request *req1 = p1;
	const struct mtrr_request *req2 = p2;

	s64 diff = req1->start - req2->start;

	return diff < 0 ? -1 : diff > 0 ? 1 : 0;
}

int mtrr_commit(bool do_caches)
{
	struct mtrr_request *req = gd->arch.mtrr_req;
	struct mtrr_state state;
	int ret;
	int i;

	debug("%s: enabled=%d, count=%d\n", __func__, gd->arch.has_mtrr,
	      gd->arch.mtrr_req_count);
	if (!gd->arch.has_mtrr)
		return -ENOSYS;

	debug("open\n");
	mtrr_open(&state, do_caches);
	debug("open done\n");
	qsort(req, gd->arch.mtrr_req_count, sizeof(*req), h_comp_mtrr);
	for (i = 0; i < gd->arch.mtrr_req_count; i++, req++)
		set_var_mtrr(i, req->type, req->start, req->size);

	/* Clear the ones that are unused */
	debug("clear\n");
	for (; i < mtrr_get_var_count(); i++)
		wrmsrl(MTRR_PHYS_MASK_MSR(i), 0);
	debug("close\n");
	mtrr_close(&state, do_caches);
	debug("mtrr done\n");

	if (gd->flags & GD_FLG_RELOC) {
		ret = mtrr_copy_to_aps();
		if (ret)
			return log_msg_ret("copy", ret);
	}

	return 0;
}

int mtrr_add_request(int type, uint64_t start, uint64_t size)
{
	struct mtrr_request *req;
	uint64_t mask;

	debug("%s: count=%d\n", __func__, gd->arch.mtrr_req_count);
	if (!gd->arch.has_mtrr)
		return -ENOSYS;

	if (!is_power_of_2(size))
		return -EINVAL;

	if (gd->arch.mtrr_req_count == MAX_MTRR_REQUESTS)
		return -ENOSPC;
	req = &gd->arch.mtrr_req[gd->arch.mtrr_req_count++];
	req->type = type;
	req->start = start;
	req->size = size;
	debug("%d: type=%d, %08llx  %08llx\n", gd->arch.mtrr_req_count - 1,
	      req->type, req->start, req->size);
	mask = ~(req->size - 1);
	mask &= (1ULL << CONFIG_CPU_ADDR_BITS) - 1;
	mask |= MTRR_PHYS_MASK_VALID;
	debug("   %016llx %016llx\n", req->start | req->type, mask);

	return 0;
}

int mtrr_get_var_count(void)
{
	return msr_read(MSR_MTRR_CAP_MSR).lo & MSR_MTRR_CAP_VCNT;
}

static int get_free_var_mtrr(void)
{
	struct msr_t maskm;
	int vcnt;
	int i;

	vcnt = mtrr_get_var_count();

	/* Identify the first var mtrr which is not valid */
	for (i = 0; i < vcnt; i++) {
		maskm = msr_read(MTRR_PHYS_MASK_MSR(i));
		if ((maskm.lo & MTRR_PHYS_MASK_VALID) == 0)
			return i;
	}

	/* No free var mtrr */
	return -ENOSPC;
}

int mtrr_set_next_var(uint type, uint64_t start, uint64_t size)
{
	int mtrr;

	if (!is_power_of_2(size))
		return -EINVAL;

	mtrr = get_free_var_mtrr();
	if (mtrr < 0)
		return mtrr;

	set_var_mtrr(mtrr, type, start, size);
	debug("MTRR %x: start=%x, size=%x\n", mtrr, (uint)start, (uint)size);

	return 0;
}

/** enum mtrr_opcode - supported operations for mtrr_do_oper() */
enum mtrr_opcode {
	MTRR_OP_SET,
	MTRR_OP_SET_VALID,
};

/**
 * struct mtrr_oper - An MTRR operation to perform on a CPU
 *
 * @opcode: Indicates operation to perform
 * @reg: MTRR reg number to select (0-7, -1 = all)
 * @valid: Valid value to write for MTRR_OP_SET_VALID
 * @base: Base value to write for MTRR_OP_SET
 * @mask: Mask value to write for MTRR_OP_SET
 */
struct mtrr_oper {
	enum mtrr_opcode opcode;
	int reg;
	bool valid;
	u64 base;
	u64 mask;
};

static void mtrr_do_oper(void *arg)
{
	struct mtrr_oper *oper = arg;
	u64 mask;

	switch (oper->opcode) {
	case MTRR_OP_SET_VALID:
		mask = native_read_msr(MTRR_PHYS_MASK_MSR(oper->reg));
		if (oper->valid)
			mask |= MTRR_PHYS_MASK_VALID;
		else
			mask &= ~MTRR_PHYS_MASK_VALID;
		wrmsrl(MTRR_PHYS_MASK_MSR(oper->reg), mask);
		break;
	case MTRR_OP_SET:
		wrmsrl(MTRR_PHYS_BASE_MSR(oper->reg), oper->base);
		wrmsrl(MTRR_PHYS_MASK_MSR(oper->reg), oper->mask);
		break;
	}
}

static int mtrr_start_op(int cpu_select, struct mtrr_oper *oper)
{
	struct mtrr_state state;
	int ret;

	mtrr_open(&state, true);
	ret = mp_run_on_cpus(cpu_select, mtrr_do_oper, oper);
	mtrr_close(&state, true);
	if (ret)
		return log_msg_ret("run", ret);

	return 0;
}

int mtrr_set_valid(int cpu_select, int reg, bool valid)
{
	struct mtrr_oper oper;

	oper.opcode = MTRR_OP_SET_VALID;
	oper.reg = reg;
	oper.valid = valid;

	return mtrr_start_op(cpu_select, &oper);
}

int mtrr_set(int cpu_select, int reg, u64 base, u64 mask)
{
	struct mtrr_oper oper;

	oper.opcode = MTRR_OP_SET;
	oper.reg = reg;
	oper.base = base;
	oper.mask = mask;

	return mtrr_start_op(cpu_select, &oper);
}

static void read_mtrrs_(void *arg)
{
	struct mtrr_info *info = arg;

	mtrr_read_all(info);
}

int mtrr_list(int reg_count, int cpu_select)
{
	struct mtrr_info info;
	int ret;
	int i;

	printf("Reg Valid Write-type   %-16s %-16s %-16s\n", "Base   ||",
	       "Mask   ||", "Size   ||");
	memset(&info, '\0', sizeof(info));
	ret = mp_run_on_cpus(cpu_select, read_mtrrs_, &info);
	if (ret)
		return log_msg_ret("run", ret);
	for (i = 0; i < reg_count; i++) {
		const char *type = "Invalid";
		u64 base, mask, size;
		bool valid;

		base = info.mtrr[i].base;
		mask = info.mtrr[i].mask;
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

int mtrr_get_type_by_name(const char *typename)
{
	int i;

	for (i = 0; i < MTRR_TYPE_COUNT; i++) {
		if (*typename == *mtrr_type_name[i])
			return i;
	}

	return -EINVAL;
};
