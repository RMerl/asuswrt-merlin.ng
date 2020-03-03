/*
 * Global extern defines for blackfin
 *
 * Copyright 2006-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef _BFIN_GLOBAL_H_
#define _BFIN_GLOBAL_H_

#ifndef __ASSEMBLY__

#include <linux/linkage.h>
#include <linux/types.h>

#if defined(CONFIG_DMA_UNCACHED_32M)
# define DMA_UNCACHED_REGION (32 * 1024 * 1024)
#elif defined(CONFIG_DMA_UNCACHED_16M)
# define DMA_UNCACHED_REGION (16 * 1024 * 1024)
#elif defined(CONFIG_DMA_UNCACHED_8M)
# define DMA_UNCACHED_REGION (8 * 1024 * 1024)
#elif defined(CONFIG_DMA_UNCACHED_4M)
# define DMA_UNCACHED_REGION (4 * 1024 * 1024)
#elif defined(CONFIG_DMA_UNCACHED_2M)
# define DMA_UNCACHED_REGION (2 * 1024 * 1024)
#elif defined(CONFIG_DMA_UNCACHED_1M)
# define DMA_UNCACHED_REGION (1024 * 1024)
#elif defined(CONFIG_DMA_UNCACHED_512K)
# define DMA_UNCACHED_REGION (512 * 1024)
#elif defined(CONFIG_DMA_UNCACHED_256K)
# define DMA_UNCACHED_REGION (256 * 1024)
#elif defined(CONFIG_DMA_UNCACHED_128K)
# define DMA_UNCACHED_REGION (128 * 1024)
#else
# define DMA_UNCACHED_REGION (0)
#endif

extern void bfin_setup_caches(unsigned int cpu);
extern void bfin_setup_cpudata(unsigned int cpu);

extern unsigned long get_cclk(void);
extern unsigned long get_sclk(void);
#ifdef CONFIG_BF60x
extern unsigned long get_sclk0(void);
extern unsigned long get_sclk1(void);
extern unsigned long get_dclk(void);
#endif
extern unsigned long sclk_to_usecs(unsigned long sclk);
extern unsigned long usecs_to_sclk(unsigned long usecs);

struct pt_regs;
#if defined(CONFIG_DEBUG_VERBOSE)
extern void dump_bfin_process(struct pt_regs *regs);
extern void dump_bfin_mem(struct pt_regs *regs);
extern void dump_bfin_trace_buffer(void);
#else
#define dump_bfin_process(regs)
#define dump_bfin_mem(regs)
#define dump_bfin_trace_buffer()
#endif

extern void *l1_data_A_sram_alloc(size_t);
extern void *l1_data_B_sram_alloc(size_t);
extern void *l1_inst_sram_alloc(size_t);
extern void *l1_data_sram_alloc(size_t);
extern void *l1_data_sram_zalloc(size_t);
extern void *l2_sram_alloc(size_t);
extern void *l2_sram_zalloc(size_t);
extern int l1_data_A_sram_free(const void*);
extern int l1_data_B_sram_free(const void*);
extern int l1_inst_sram_free(const void*);
extern int l1_data_sram_free(const void*);
extern int l2_sram_free(const void *);
extern int sram_free(const void*);

#define L1_INST_SRAM		0x00000001
#define L1_DATA_A_SRAM		0x00000002
#define L1_DATA_B_SRAM		0x00000004
#define L1_DATA_SRAM		0x00000006
#define L2_SRAM			0x00000008
extern void *sram_alloc_with_lsl(size_t, unsigned long);
extern int sram_free_with_lsl(const void*);

extern void *isram_memcpy(void *dest, const void *src, size_t n);

extern const char bfin_board_name[];

extern unsigned long bfin_sic_iwr[];
extern unsigned vr_wakeup;
extern u16 _bfin_swrst; /* shadow for Software Reset Register (SWRST) */

#endif

#endif				/* _BLACKFIN_H_ */
