/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#ifndef _PFE_H_
#define _PFE_H_

#include <elf.h>
#include "cbus.h"

#define PFE_RESET_WA

#define CLASS_DMEM_BASE_ADDR(i)	(0x00000000 | ((i) << 20))
/* Only valid for mem access register interface */
#define CLASS_IMEM_BASE_ADDR(i)	(0x00000000 | ((i) << 20))
#define CLASS_DMEM_SIZE		0x00002000
#define CLASS_IMEM_SIZE		0x00008000

#define TMU_DMEM_BASE_ADDR(i)	(0x00000000 + ((i) << 20))
/* Only valid for mem access register interface */
#define TMU_IMEM_BASE_ADDR(i)	(0x00000000 + ((i) << 20))
#define TMU_DMEM_SIZE		0x00000800
#define TMU_IMEM_SIZE		0x00002000

#define UTIL_DMEM_BASE_ADDR	0x00000000
#define UTIL_DMEM_SIZE		0x00002000

#define PE_LMEM_BASE_ADDR	0xc3010000
#define PE_LMEM_SIZE		0x8000
#define PE_LMEM_END		(PE_LMEM_BASE_ADDR + PE_LMEM_SIZE)

#define DMEM_BASE_ADDR		0x00000000
#define DMEM_SIZE		0x2000		/* TMU has less... */
#define DMEM_END		(DMEM_BASE_ADDR + DMEM_SIZE)

#define PMEM_BASE_ADDR		0x00010000
#define PMEM_SIZE		0x8000		/* TMU has less... */
#define PMEM_END		(PMEM_BASE_ADDR + PMEM_SIZE)

/* Memory ranges check from PE point of view/memory map */
#define IS_DMEM(addr, len)	(((unsigned long)(addr) >= DMEM_BASE_ADDR) &&\
					(((unsigned long)(addr) +\
					(len)) <= DMEM_END))
#define IS_PMEM(addr, len)	(((unsigned long)(addr) >= PMEM_BASE_ADDR) &&\
					(((unsigned long)(addr) +\
					(len)) <= PMEM_END))
#define IS_PE_LMEM(addr, len)	(((unsigned long)(addr) >= PE_LMEM_BASE_ADDR\
					) && (((unsigned long)(addr)\
					+ (len)) <= PE_LMEM_END))

#define IS_PFE_LMEM(addr, len)	(((unsigned long)(addr) >=\
					CBUS_VIRT_TO_PFE(LMEM_BASE_ADDR)) &&\
					(((unsigned long)(addr) + (len)) <=\
					CBUS_VIRT_TO_PFE(LMEM_END)))
#define IS_PHYS_DDR(addr, len)	(((unsigned long)(addr) >=\
					PFE_DDR_PHYS_BASE_ADDR) &&\
					(((unsigned long)(addr) + (len)) <=\
					PFE_DDR_PHYS_END))

/* Host View Address */
extern void *ddr_pfe_base_addr;

/* PFE View Address */
/* DDR physical base address as seen by PE's. */
#define PFE_DDR_PHYS_BASE_ADDR	0x03800000
#define PFE_DDR_PHYS_SIZE	0xC000000
#define PFE_DDR_PHYS_END	(PFE_DDR_PHYS_BASE_ADDR + PFE_DDR_PHYS_SIZE)
/* CBUS physical base address as seen by PE's. */
#define PFE_CBUS_PHYS_BASE_ADDR	0xc0000000

/* Host<->PFE Mapping */
#define DDR_PFE_TO_VIRT(p)	((unsigned long int)((p) + 0x80000000))
#define CBUS_VIRT_TO_PFE(v)	(((v) - CBUS_BASE_ADDR) +\
					PFE_CBUS_PHYS_BASE_ADDR)
#define CBUS_PFE_TO_VIRT(p)	(((p) - PFE_CBUS_PHYS_BASE_ADDR) +\
					CBUS_BASE_ADDR)

enum {
	CLASS0_ID = 0,
	CLASS1_ID,
	CLASS2_ID,
	CLASS3_ID,
	CLASS4_ID,
	CLASS5_ID,

	TMU0_ID,
	TMU1_ID,
	TMU2_ID,
	TMU3_ID,
	MAX_PE
};

#define CLASS_MASK	(BIT(CLASS0_ID) | BIT(CLASS1_ID) | BIT(CLASS2_ID)\
				| BIT(CLASS3_ID) | BIT(CLASS4_ID) |\
				BIT(CLASS5_ID))
#define CLASS_MAX_ID	CLASS5_ID

#define TMU_MASK	(BIT(TMU0_ID) | BIT(TMU1_ID) | BIT(TMU3_ID))
#define TMU_MAX_ID	TMU3_ID

/*
 * PE information.
 * Structure containing PE's specific information. It is used to create
 * generic C functions common to all PEs.
 * Before using the library functions this structure needs to be
 * initialized with the different registers virtual addresses
 * (according to the ARM MMU mmaping). The default initialization supports a
 * virtual == physical mapping.
 *
 */
struct pe_info {
	u32 dmem_base_addr;		/* PE's dmem base address */
	u32 pmem_base_addr;		/* PE's pmem base address */
	u32 pmem_size;			/* PE's pmem size */

	void *mem_access_wdata;	       /* PE's _MEM_ACCESS_WDATA
					* register address
					*/
	void *mem_access_addr;	       /* PE's _MEM_ACCESS_ADDR
					* register address
					*/
	void *mem_access_rdata;	       /* PE's _MEM_ACCESS_RDATA
					* register address
					*/
};

void pe_lmem_read(u32 *dst, u32 len, u32 offset);
void pe_lmem_write(u32 *src, u32 len, u32 offset);

u32 pe_pmem_read(int id, u32 addr, u8 size);
void pe_dmem_write(int id, u32 val, u32 addr, u8 size);
u32 pe_dmem_read(int id, u32 addr, u8 size);

int pe_load_elf_section(int id, const void *data, Elf32_Shdr *shdr);

void pfe_lib_init(void);

void bmu_init(void *base, struct bmu_cfg *cfg);
void bmu_enable(void *base);

void gpi_init(void *base, struct gpi_cfg *cfg);
void gpi_enable(void *base);
void gpi_disable(void *base);

void class_init(struct class_cfg *cfg);
void class_enable(void);
void class_disable(void);

void tmu_init(struct tmu_cfg *cfg);
void tmu_enable(u32 pe_mask);
void tmu_disable(u32 pe_mask);

void hif_init(void);
void hif_tx_enable(void);
void hif_tx_disable(void);
void hif_rx_enable(void);
void hif_rx_disable(void);
void hif_rx_desc_disable(void);

#endif /* _PFE_H_ */
