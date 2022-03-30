// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <linux/compiler.h>
#include <asm/fsl_law.h>
#include <asm/io.h>
#include <linux/log2.h>

DECLARE_GLOBAL_DATA_PTR;

#define FSL_HW_NUM_LAWS CONFIG_SYS_FSL_NUM_LAWS

#ifdef CONFIG_FSL_CORENET
#define LAW_BASE (CONFIG_SYS_FSL_CORENET_CCM_ADDR)
#define LAWAR_ADDR(x) (&((ccsr_local_t *)LAW_BASE)->law[x].lawar)
#define LAWBARH_ADDR(x) (&((ccsr_local_t *)LAW_BASE)->law[x].lawbarh)
#define LAWBARL_ADDR(x) (&((ccsr_local_t *)LAW_BASE)->law[x].lawbarl)
#define LAWBAR_SHIFT 0
#else
#define LAW_BASE (CONFIG_SYS_IMMR + 0xc08)
#define LAWAR_ADDR(x) ((u32 *)LAW_BASE + 8 * x + 2)
#define LAWBAR_ADDR(x) ((u32 *)LAW_BASE + 8 * x)
#define LAWBAR_SHIFT 12
#endif


static inline phys_addr_t get_law_base_addr(int idx)
{
#ifdef CONFIG_FSL_CORENET
	return (phys_addr_t)
		((u64)in_be32(LAWBARH_ADDR(idx)) << 32) |
		in_be32(LAWBARL_ADDR(idx));
#else
	return (phys_addr_t)in_be32(LAWBAR_ADDR(idx)) << LAWBAR_SHIFT;
#endif
}

static inline void set_law_base_addr(int idx, phys_addr_t addr)
{
#ifdef CONFIG_FSL_CORENET
	out_be32(LAWBARL_ADDR(idx), addr & 0xffffffff);
	out_be32(LAWBARH_ADDR(idx), (u64)addr >> 32);
#else
	out_be32(LAWBAR_ADDR(idx), addr >> LAWBAR_SHIFT);
#endif
}

void set_law(u8 idx, phys_addr_t addr, enum law_size sz, enum law_trgt_if id)
{
	gd->arch.used_laws |= (1 << idx);

	out_be32(LAWAR_ADDR(idx), 0);
	set_law_base_addr(idx, addr);
	out_be32(LAWAR_ADDR(idx), LAW_EN | ((u32)id << 20) | (u32)sz);

	/* Read back so that we sync the writes */
	in_be32(LAWAR_ADDR(idx));
}

void disable_law(u8 idx)
{
	gd->arch.used_laws &= ~(1 << idx);

	out_be32(LAWAR_ADDR(idx), 0);
	set_law_base_addr(idx, 0);

	/* Read back so that we sync the writes */
	in_be32(LAWAR_ADDR(idx));

	return;
}

#if !defined(CONFIG_NAND_SPL) && \
	(!defined(CONFIG_SPL_BUILD) || !defined(CONFIG_SPL_INIT_MINIMAL))
static int get_law_entry(u8 i, struct law_entry *e)
{
	u32 lawar;

	lawar = in_be32(LAWAR_ADDR(i));

	if (!(lawar & LAW_EN))
		return 0;

	e->addr = get_law_base_addr(i);
	e->size = lawar & 0x3f;
	e->trgt_id = (lawar >> 20) & 0xff;

	return 1;
}
#endif

int set_next_law(phys_addr_t addr, enum law_size sz, enum law_trgt_if id)
{
	u32 idx = ffz(gd->arch.used_laws);

	if (idx >= FSL_HW_NUM_LAWS)
		return -1;

	set_law(idx, addr, sz, id);

	return idx;
}

#if !defined(CONFIG_NAND_SPL) && \
	(!defined(CONFIG_SPL_BUILD) || !defined(CONFIG_SPL_INIT_MINIMAL))
int set_last_law(phys_addr_t addr, enum law_size sz, enum law_trgt_if id)
{
	u32 idx;

	/* we have no LAWs free */
	if (gd->arch.used_laws == -1)
		return -1;

	/* grab the last free law */
	idx = __ilog2(~(gd->arch.used_laws));

	if (idx >= FSL_HW_NUM_LAWS)
		return -1;

	set_law(idx, addr, sz, id);

	return idx;
}

struct law_entry find_law(phys_addr_t addr)
{
	struct law_entry entry;
	int i;

	entry.index = -1;
	entry.addr = 0;
	entry.size = 0;
	entry.trgt_id = 0;

	for (i = 0; i < FSL_HW_NUM_LAWS; i++) {
		u64 upper;

		if (!get_law_entry(i, &entry))
			continue;

		upper = entry.addr + (2ull << entry.size);
		if ((addr >= entry.addr) && (addr < upper)) {
			entry.index = i;
			break;
		}
	}

	return entry;
}

void print_laws(void)
{
	int i;
	u32 lawar;

	printf("\nLocal Access Window Configuration\n");
	for (i = 0; i < FSL_HW_NUM_LAWS; i++) {
		lawar = in_be32(LAWAR_ADDR(i));
#ifdef CONFIG_FSL_CORENET
		printf("LAWBARH%02d: 0x%08x LAWBARL%02d: 0x%08x",
		       i, in_be32(LAWBARH_ADDR(i)),
		       i, in_be32(LAWBARL_ADDR(i)));
#else
		printf("LAWBAR%02d: 0x%08x", i, in_be32(LAWBAR_ADDR(i)));
#endif
		printf(" LAWAR%02d: 0x%08x\n", i, lawar);
		printf("\t(EN: %d TGT: 0x%02x SIZE: ",
		       (lawar & LAW_EN) ? 1 : 0, (lawar >> 20) & 0xff);
		print_size(lawar_size(lawar), ")\n");
	}

	return;
}

/* use up to 2 LAWs for DDR, used the last available LAWs */
int set_ddr_laws(u64 start, u64 sz, enum law_trgt_if id)
{
	u64 start_align, law_sz;
	int law_sz_enc;

	if (start == 0)
		start_align = 1ull << (LAW_SIZE_32G + 1);
	else
		start_align = 1ull << (__ffs64(start));
	law_sz = min(start_align, sz);
	law_sz_enc = __ilog2_u64(law_sz) - 1;

	if (set_last_law(start, law_sz_enc, id) < 0)
		return -1;

	/* recalculate size based on what was actually covered by the law */
	law_sz = 1ull << __ilog2_u64(law_sz);

	/* do we still have anything to map */
	sz = sz - law_sz;
	if (sz) {
		start += law_sz;

		start_align = 1ull << (__ffs64(start));
		law_sz = min(start_align, sz);
		law_sz_enc = __ilog2_u64(law_sz) - 1;

		if (set_last_law(start, law_sz_enc, id) < 0)
			return -1;
	} else {
		return 0;
	}

	/* do we still have anything to map */
	sz = sz - law_sz;
	if (sz)
		return 1;

	return 0;
}
#endif /* not SPL */

void disable_non_ddr_laws(void)
{
	int i;
	int id;
	for (i = 0; i < FSL_HW_NUM_LAWS; i++) {
		u32 lawar = in_be32(LAWAR_ADDR(i));

		if (lawar & LAW_EN) {
			id = (lawar & ~LAW_EN) >> 20;
			switch (id) {
			case LAW_TRGT_IF_DDR_1:
			case LAW_TRGT_IF_DDR_2:
			case LAW_TRGT_IF_DDR_3:
			case LAW_TRGT_IF_DDR_4:
			case LAW_TRGT_IF_DDR_INTRLV:
			case LAW_TRGT_IF_DDR_INTLV_34:
			case LAW_TRGT_IF_DDR_INTLV_123:
			case LAW_TRGT_IF_DDR_INTLV_1234:
						continue;
			default:
						disable_law(i);
			}
		}
	}
}

void init_laws(void)
{
	int i;

#if FSL_HW_NUM_LAWS < 32
	gd->arch.used_laws = ~((1 << FSL_HW_NUM_LAWS) - 1);
#elif FSL_HW_NUM_LAWS == 32
	gd->arch.used_laws = 0;
#else
#error FSL_HW_NUM_LAWS can not be greater than 32 w/o code changes
#endif

#if defined(CONFIG_SECURE_BOOT) && defined(CONFIG_E500) && \
						!defined(CONFIG_E500MC)
	/* ISBC (Boot ROM) creates a LAW 0 entry for non PBL platforms,
	 * which is not disabled before transferring the control to uboot.
	 * Disable the LAW 0 entry here.
	 */
	disable_law(0);
#endif

#if !defined(CONFIG_SECURE_BOOT)
	/*
	 * if any non DDR LAWs has been created earlier, remove them before
	 * LAW table is parsed.
	*/
	disable_non_ddr_laws();
#endif

	/*
	 * Any LAWs that were set up before we booted assume they are meant to
	 * be around and mark them used.
	 */
	for (i = 0; i < FSL_HW_NUM_LAWS; i++) {
		u32 lawar = in_be32(LAWAR_ADDR(i));

		if (lawar & LAW_EN)
			gd->arch.used_laws |= (1 << i);
	}

	for (i = 0; i < num_law_entries; i++) {
		if (law_table[i].index == -1)
			set_next_law(law_table[i].addr, law_table[i].size,
					law_table[i].trgt_id);
		else
			set_law(law_table[i].index, law_table[i].addr,
				law_table[i].size, law_table[i].trgt_id);
	}

#ifdef CONFIG_SRIO_PCIE_BOOT_SLAVE
	/* check RCW to get which port is used for boot */
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	u32 bootloc = in_be32(&gur->rcwsr[6]);
	/*
	 * in SRIO or PCIE boot we need to set specail LAWs for
	 * SRIO or PCIE interfaces.
	 */
	switch ((bootloc & FSL_CORENET_RCWSR6_BOOT_LOC) >> 23) {
	case 0x0: /* boot from PCIE1 */
		set_next_law(CONFIG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR_PHYS,
				LAW_SIZE_1M,
				LAW_TRGT_IF_PCIE_1);
		set_next_law(CONFIG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR_PHYS,
				LAW_SIZE_1M,
				LAW_TRGT_IF_PCIE_1);
		break;
	case 0x1: /* boot from PCIE2 */
		set_next_law(CONFIG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR_PHYS,
				LAW_SIZE_1M,
				LAW_TRGT_IF_PCIE_2);
		set_next_law(CONFIG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR_PHYS,
				LAW_SIZE_1M,
				LAW_TRGT_IF_PCIE_2);
		break;
	case 0x2: /* boot from PCIE3 */
		set_next_law(CONFIG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR_PHYS,
				LAW_SIZE_1M,
				LAW_TRGT_IF_PCIE_3);
		set_next_law(CONFIG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR_PHYS,
				LAW_SIZE_1M,
				LAW_TRGT_IF_PCIE_3);
		break;
	case 0x8: /* boot from SRIO1 */
		set_next_law(CONFIG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR_PHYS,
				LAW_SIZE_1M,
				LAW_TRGT_IF_RIO_1);
		set_next_law(CONFIG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR_PHYS,
				LAW_SIZE_1M,
				LAW_TRGT_IF_RIO_1);
		break;
	case 0x9: /* boot from SRIO2 */
		set_next_law(CONFIG_SYS_SRIO_PCIE_BOOT_SLAVE_ADDR_PHYS,
				LAW_SIZE_1M,
				LAW_TRGT_IF_RIO_2);
		set_next_law(CONFIG_SYS_SRIO_PCIE_BOOT_UCODE_ENV_ADDR_PHYS,
				LAW_SIZE_1M,
				LAW_TRGT_IF_RIO_2);
		break;
	default:
		break;
	}
#endif

	return ;
}
