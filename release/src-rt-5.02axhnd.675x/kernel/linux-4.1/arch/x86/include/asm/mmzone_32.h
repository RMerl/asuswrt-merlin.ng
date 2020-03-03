/*
 * Written by Pat Gaughen (gone@us.ibm.com) Mar 2002
 *
 */

#ifndef _ASM_X86_MMZONE_32_H
#define _ASM_X86_MMZONE_32_H

#include <asm/smp.h>

#ifdef CONFIG_NUMA
extern struct pglist_data *node_data[];
#define NODE_DATA(nid)	(node_data[nid])
#endif /* CONFIG_NUMA */

#ifdef CONFIG_DISCONTIGMEM

/*
 * generic node memory support, the following assumptions apply:
 *
 * 1) memory comes in 64Mb contiguous chunks which are either present or not
 * 2) we will not have more than 64Gb in total
 *
 * for now assume that 64Gb is max amount of RAM for whole system
 *    64Gb / 4096bytes/page = 16777216 pages
 */
#define MAX_NR_PAGES 16777216
#define MAX_SECTIONS 1024
#define PAGES_PER_SECTION (MAX_NR_PAGES/MAX_SECTIONS)

extern s8 physnode_map[];

static inline int pfn_to_nid(unsigned long pfn)
{
#ifdef CONFIG_NUMA
	return((int) physnode_map[(pfn) / PAGES_PER_SECTION]);
#else
	return 0;
#endif
}

static inline int pfn_valid(int pfn)
{
	int nid = pfn_to_nid(pfn);

	if (nid >= 0)
		return (pfn < node_end_pfn(nid));
	return 0;
}

#define early_pfn_valid(pfn)	pfn_valid((pfn))

#endif /* CONFIG_DISCONTIGMEM */

#endif /* _ASM_X86_MMZONE_32_H */
