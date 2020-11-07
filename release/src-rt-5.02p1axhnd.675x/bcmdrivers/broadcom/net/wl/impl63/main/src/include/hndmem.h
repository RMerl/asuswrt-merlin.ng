/*
 * Utility routines for configuring different memories in Broadcom chips.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: $
 */

#ifndef _HNDMEM_H_
#define _HNDMEM_H_

typedef enum {
	MEM_SOCRAM = 0,
	MEM_BM = 1,
	MEM_UCM = 2,
	MEM_SHM = 3,
	MEM_MAX = 4
} hndmem_type_t;

/* PDA (Power Down Array) configuration */
typedef enum {
	PDA_CONFIG_CLEAR = 0,		/* Clear PDA, i.e. Turns on the memory bank */
	PDA_CONFIG_SET_FULL = 1,	/* Set PDA, i.e. Truns off the memory bank */
	PDA_CONFIG_SET_PARTIAL = 2,	/* Set PDA, i.e. Truns off the memory bank */
	PDA_CONFIG_MAX = 3
} hndmem_config_t;

/* Returns the number of banks in a given memory */
extern int hndmem_num_banks(si_t *sih, int mem);

/* Returns the size of a give bank in a given memory */
extern int hndmem_bank_size(si_t *sih, hndmem_type_t mem, int bank_num);

/* Returns the start address of given memory */
extern uint32 hndmem_mem_base(si_t *sih, hndmem_type_t mem);

#ifdef BCMDEBUG
/* Dumps the complete memory information */
extern void hndmem_dump_meminfo_all(si_t *sih);
#endif /* BCMDEBUG */

/* Configures the Sleep PDA for a particular bank for a given memory type */
extern int  hndmem_sleeppda_bank_config(si_t *sih, hndmem_type_t mem,
		int bank_num, hndmem_config_t config, uint32 pda);
/* Configures the Active PDA for a particular bank for a given memory type */
extern int hndmem_activepda_bank_config(si_t *sih, hndmem_type_t mem,
		int bank_num, hndmem_config_t config, uint32 pda);

/* Configures the Sleep PDA for all the banks for a given memory type */
extern int hndmem_sleeppda_config(si_t *sih, hndmem_type_t mem,
		hndmem_config_t config);
/* Configures the Active PDA for all the banks for a given memory type */
extern int hndmem_activepda_config(si_t *sih, hndmem_type_t mem,
		hndmem_config_t config);

/* Turn off/on all the possible banks in a given memory range */
extern int hndmem_activepda_mem_config(si_t *sih, hndmem_type_t mem,
		uint32 mem_start, uint32 size, hndmem_config_t config);
#endif /* _HNDMEM_H_ */
