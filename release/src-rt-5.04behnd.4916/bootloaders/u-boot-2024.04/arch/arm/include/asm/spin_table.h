/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __ASM_SPIN_TABLE_H__
#define __ASM_SPIN_TABLE_H__

extern u64 spin_table_cpu_release_addr;
extern char spin_table_reserve_begin;
extern char spin_table_reserve_end;

int spin_table_update_dt(void *fdt);

#endif /* __ASM_SPIN_TABLE_H__ */
