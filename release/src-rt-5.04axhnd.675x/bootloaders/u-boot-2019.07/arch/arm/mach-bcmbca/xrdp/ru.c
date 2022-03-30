// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2014 Broadcom
   All Rights Reserved

    
*/
/**
 * \brief Register Utilities functional implementation
 */
#include <common.h>
#include "ru.h"
#include "ru_config.h"


void *memset_write32(void *block, int c, uint32_t size)
{
	return memset(block, c, size);
}

extern const ru_block_rec *RU_ALL_BLOCKS[];

/******************************************************************************
 * Find by name utilities
 ******************************************************************************/
const ru_block_rec *ru_block_name_find(const char *name)
{
	int blk_idx = 0;
	const ru_block_rec *blk = RU_ALL_BLOCKS[blk_idx];

	while (blk) {
		if (strcmp(blk->name, name) == 0)
			break;
		blk = RU_ALL_BLOCKS[++blk_idx];
	}
	return blk;
}


const ru_reg_rec *ru_reg_name_find(ru_block_inst blk_inst,
				   const ru_block_rec *block,
				   const char *name)
{
	int i;
	const ru_reg_rec *reg = NULL;

	for (i = 0; i < block->reg_count; i++) {
		if (strcmp(block->regs[i]->name, name) == 0) {
			reg = block->regs[i];
			break;
		}
	}
	return reg;
}

#if RU_INCLUDE_FIELD_DB
const ru_field_rec *ru_field_name_find(const ru_reg_rec *reg, const char *name)
{
	int i;
	const ru_field_rec *fld = NULL;

	for (i = 0; i < reg->field_count; i++) {
		if (strcmp(reg->fields[i]->name, name) == 0) {
			fld = reg->fields[i];
			break;
		}
	}
	return fld;
}
#endif


/******************************************************************************
 * Find by register address
 ******************************************************************************/
int ru_block_addr_find(uint32_t addr,
	 	       ru_block_inst *blk_inst,
		       const ru_block_rec **block)
{
	int rc = -1;
	ru_block_inst blk_idx = 0;
	const ru_block_rec *blk = RU_ALL_BLOCKS[blk_idx];
	uint32_t reg_idx;

	while (blk) {
		for (*blk_inst = 0; *blk_inst < blk->addr_count; (*blk_inst)++) {
			for (reg_idx = 0; reg_idx < blk->reg_count; reg_idx++) {
				if ((blk->regs[reg_idx]->addr +
				     blk->addr[*blk_inst]) == addr) {
					*block = blk;
					return 0;
				}
			}
		}
		blk = RU_ALL_BLOCKS[++blk_idx];
	}

	return rc;
}


int ru_reg_addr_find(uint32_t addr, ru_block_inst *blk_inst,
		     const ru_block_rec **block,
		     const ru_reg_rec **reg)
{
	int rc = -1;
	ru_block_inst blk_idx = 0;
	const ru_block_rec *blk = RU_ALL_BLOCKS[blk_idx];
	uint32_t reg_idx;

	while (blk) {
		for (*blk_inst = 0; *blk_inst < blk->addr_count; (*blk_inst)++) {
			for (reg_idx = 0; reg_idx < blk->reg_count; reg_idx++) {
				if ((blk->regs[reg_idx]->addr +
				     blk->addr[*blk_inst]) == addr) {
					*block = blk;
					*reg = blk->regs[reg_idx];
					return 0;
				}
			}
		}
		blk = RU_ALL_BLOCKS[++blk_idx];
	}

	return rc;
}


/******************************************************************************
 * Print parsed register value
 ******************************************************************************/
int ru_reg_print(ru_block_inst blk_inst,
				 const ru_block_rec *block,
				 const ru_reg_rec *reg,
				 uint32_t value)
{
#if RU_INCLUDE_FIELD_DB
	int i;
#endif

	RU_PRINT("%s.%s[%d]@0x%lX+%lX: 0x%08X\n", block->name, reg->name,
		 blk_inst, block->addr[blk_inst], reg->addr, value);

#if RU_INCLUDE_FIELD_DB
	for (i = 0; i < reg->field_count; i++) {
		RU_PRINT("\t%s[%d:%d]: 0x%X\n", reg->fields[i]->name,
			 reg->fields[i]->shift + reg->fields[i]->bits - 1,
			 reg->fields[i]->shift,
			 ru_field_get(blk_inst, block, reg, reg->fields[i],
				      value));
	}
#endif
	return 0;
}


int ru_reg_addr_print(uint32_t addr, uint32_t value)
{
	ru_block_inst blk_inst;
	const ru_block_rec *block;
	const ru_reg_rec *reg;
	int rc;

	if (!(rc = ru_reg_addr_find(addr, &blk_inst, &block, &reg)))
		ru_reg_print(blk_inst, block, reg, value);

	return rc;
}


int ru_reg_name_print(ru_block_inst blk_inst, const char *bname,
		      const char *rname, uint32_t value)
{
	const ru_block_rec *block;
	const ru_reg_rec *reg;
	int rc = -1;

	block = ru_block_name_find(bname);
	if (block) {
		reg = ru_reg_name_find(blk_inst, block, rname);
		if (reg)
			rc = ru_reg_print(blk_inst, block, reg, value);
	}

	return rc;
}

#if RU_FIELD_CHECK_ENABLE
static uint8_t ru_field_check_enable;
#endif

#if RU_OFFLINE_TEST
static uint32_t ru_reg_space[RU_REG_COUNT];
static uint32_t **ru_mem_reg_space[RU_BLK_COUNT];
#endif

void ru_field_bounds_check_enable(int enable)
{
#if RU_FIELD_CHECK_ENABLE
	ru_field_check_enable = enable;
#endif /* RU_FIELD_CHECK_ENABLE */
}


/******************************************************************************
 * Register access functions
 ******************************************************************************/


#if !RU_EXTERNAL_REGISTER_ADDRESSING
int __ru_reg_write(const char *func, const int line, ru_block_inst blk_inst,
		   const ru_block_rec *blk, const ru_reg_rec *reg, uint32_t val)
{
	RU_DBG("RU_REG_WRITE from %s:%d, block:%s, reg:%s, val:%x\n", func,
	       line, blk->name, reg->name, val);
#if RU_OFFLINE_TEST
	ru_reg_space[reg->log_idx] = val;
#else
	WRITE_32(blk->addr[blk_inst] + reg->addr, val);
#endif /* RU_OFFLINE_TEST */
	return 0;
}
#endif /* !RU_EXTERNAL_REGISTER_ADDRESSING */


#if !RU_EXTERNAL_REGISTER_ADDRESSING
uint32_t __ru_reg_read(const char *func, const int line, ru_block_inst blk_inst,
		       const ru_block_rec *blk, const ru_reg_rec *reg)
{
#if RU_OFFLINE_TEST
	return ru_reg_space[reg->log_idx];
#else
	uint32_t rv;
	
	READ_32(blk->addr[blk_inst] + reg->addr, rv);

	RU_DBG("RU_REG_READ from %s:%d, block:%s\n", func, line, blk->name);
	return rv;
#endif /* RU_OFFLINE_TEST */
}
#endif /* !RU_EXTERNAL_REGISTER_ADDRESSING */

#if RU_OFFLINE_TEST
int ru_block_idx_find(const char *name)
{
	int blk_idx = 0;
	const ru_block_rec *blk = RU_ALL_BLOCKS[blk_idx];

	while (blk) {
		if (strcmp(blk->name, name) == 0)
			break;
		blk = RU_ALL_BLOCKS[++blk_idx];
	}
	return blk_idx;
}

int ru_reg_idx_find(const ru_block_rec *blk, const char *name)
{
	int i;

	for (i = 0; i < blk->reg_count; i++) {
		if (strcmp(blk->regs[i]->name, name) == 0)
			break;
	}
	return i;
}

uint32_t *ru_reg_mem_area(const ru_block_rec *blk, const ru_reg_rec *reg)
{
	int blk_idx = ru_block_idx_find(blk->name);
	int reg_idx = ru_reg_idx_find(blk, reg->name);
	/* Per-register pointer array */
	uint32_t **blk_reg_areas = ru_mem_reg_space[blk_idx];

	if (!blk_reg_areas[reg_idx]) {
		printf("%s: %s.%s No ram. blk_idx=%d reg_idx=%d\n",
			__FUNCTION__, blk->name, reg->name, blk_idx, reg_idx);
	}
	return blk_reg_areas[reg_idx];
}
#endif

#if !RU_EXTERNAL_REGISTER_ADDRESSING
int __ru_reg_ram_write(const char *func, const int line, ru_block_inst blk_inst,
		       ru_ram_addr ram_addr, const ru_block_rec *blk,
		       const ru_reg_rec *reg, uint32_t val)
{
#if RU_OFFLINE_TEST
	uint32_t *mem_area = ru_reg_mem_area(blk, reg);
#endif

	RU_DBG("RU_REG_RAM_WRITE from %s:%d, block:%s\n", func, line, blk->name);
	printf("RU_REG_RAM_WRITE from %s:%d, block:%s\n", func, line, blk->name);

#if RU_OFFLINE_TEST
	if (mem_area)
		mem_area[reg->ram_count*blk_inst + ram_addr] = val;
	else
		printf("%s->%s: can't write %s.%s. No ram\n", func,
		       __FUNCTION__, blk->name, reg->name);
#else
	WRITE_32(blk->addr[blk_inst] + reg->addr + (reg->offset * ram_addr), val);
			
	printf("%s:%d:writing to addr 0x%08x\n", __func__, __LINE__,
	       reg->addr + (reg->offset * ram_addr));
#endif /* RU_OFFLINE_TEST */
	return 0;
}
#endif /* !RU_EXTERNAL_REGISTER_ADDRESSING */


#if !RU_EXTERNAL_REGISTER_ADDRESSING
uint32_t __ru_reg_ram_read(const char *func, const int line,
			   ru_block_inst blk_inst, ru_ram_addr ram_addr,
			   const ru_block_rec *blk, const ru_reg_rec *reg)
{
	uint32_t rv;
#if RU_OFFLINE_TEST
	uint32_t *mem_area = ru_reg_mem_area(blk, reg);
	if (mem_area)
		rv = mem_area[reg->ram_count*blk_inst + ram_addr];
	else {
		printf("%s->%s: can't read %s.%s. No ram\n", func, __func__,
		       blk->name, reg->name);
		rv = 0;
	}
#else
	READ_32(blk->addr[blk_inst] + reg->addr + (reg->offset * ram_addr), rv);

	RU_DBG("RU_REG_RAM_READ from %s:%d, block:%s\n", func, line, blk->name);
#endif /* RU_OFFLINE_TEST */
	return rv;
}
#endif /* !RU_EXTERNAL_REGISTER_ADDRESSING */


uint32_t __ru_field_set(const char *func, const int line,
			ru_block_inst blk_inst, const ru_block_rec *blk,
			const ru_reg_rec *reg, const ru_field_rec *fld,
			uint32_t reg_val, uint32_t fld_val)
{
#if RU_FIELD_CHECK_ENABLE
	if (ru_field_check_enable) {
		if (fld_val > (fld->mask >> fld->shift)) {
			RU_ASSERT();
			RU_PRINT("ASSERT: Field value out of range. Max %u, "
				 "attempted %u\n", (fld->mask >> fld->shift),
				 fld_val);
			RU_PRINT("  In field: %s.%s.%s\n", blk->name, reg->name,
				 fld->name);
			return reg_val;
		}
	}
#endif /* RU_FIELD_CHECK_ENABLE */
	RU_DBG("RU_FIELD_SET from %s:%d, field:%s, reg_val:%x, field_val:%x\n",
	       func, line, fld->name, reg_val, fld_val);

	return FIELD_SET_(reg_val, fld->mask, fld->shift, fld_val);
}


uint32_t __ru_field_get(const char *func, const int line,
			ru_block_inst blk_inst, const ru_block_rec *blk,
			const ru_reg_rec *reg, const ru_field_rec *fld,
			uint32_t reg_val)
{
	RU_DBG("RU_FIELD_GET from %s:%d, field:%s\n", func, line, fld->name);
	return FIELD_GET_(reg_val, fld->mask, fld->shift);
}


void __ru_field_write(const char *func, const int line,	ru_block_inst blk_inst,
		      const ru_block_rec *blk, const ru_reg_rec *reg,
		      const ru_field_rec *fld, uint32_t fld_val)
{
	uint32_t rv;

	rv = __ru_reg_read(func, line, blk_inst, blk, reg);
	rv = __ru_field_set(func, line, blk_inst, blk, reg, fld, rv, fld_val);
	__ru_reg_write(func, line, blk_inst, blk, reg, rv);
}


uint32_t __ru_field_read(const char *func, const int line,
			 ru_block_inst blk_inst, const ru_block_rec *blk,
			 const ru_reg_rec *reg, const ru_field_rec *fld)
{
	uint32_t rv;

	rv = __ru_reg_read(func, line, blk_inst, blk, reg);
	return __ru_field_get(func, line, blk_inst, blk, reg, fld, rv);
}

/* End of file ru.c */
