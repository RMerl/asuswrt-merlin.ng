/*
 * Page table types definitions.
 *
 * Copyright (C) 2014 ARM Ltd.
 * Author: Catalin Marinas <catalin.marinas@arm.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __ASM_PGTABLE_TYPES_H
#define __ASM_PGTABLE_TYPES_H

#include <asm/types.h>

typedef u64 pteval_t;
typedef u64 pmdval_t;
typedef u64 pudval_t;
typedef u64 pgdval_t;

#undef STRICT_MM_TYPECHECKS

#ifdef STRICT_MM_TYPECHECKS

/*
 * These are used to make use of C type-checking..
 */
typedef struct { pteval_t pte; } pte_t;
#define pte_val(x)	((x).pte)
#define __pte(x)	((pte_t) { (x) } )

#if CONFIG_PGTABLE_LEVELS > 2
typedef struct { pmdval_t pmd; } pmd_t;
#define pmd_val(x)	((x).pmd)
#define __pmd(x)	((pmd_t) { (x) } )
#endif

#if CONFIG_PGTABLE_LEVELS > 3
typedef struct { pudval_t pud; } pud_t;
#define pud_val(x)	((x).pud)
#define __pud(x)	((pud_t) { (x) } )
#endif

typedef struct { pgdval_t pgd; } pgd_t;
#define pgd_val(x)	((x).pgd)
#define __pgd(x)	((pgd_t) { (x) } )

typedef struct { pteval_t pgprot; } pgprot_t;
#define pgprot_val(x)	((x).pgprot)
#define __pgprot(x)	((pgprot_t) { (x) } )

#else	/* !STRICT_MM_TYPECHECKS */

typedef pteval_t pte_t;
#define pte_val(x)	(x)
#define __pte(x)	(x)

#if CONFIG_PGTABLE_LEVELS > 2
typedef pmdval_t pmd_t;
#define pmd_val(x)	(x)
#define __pmd(x)	(x)
#endif

#if CONFIG_PGTABLE_LEVELS > 3
typedef pudval_t pud_t;
#define pud_val(x)	(x)
#define __pud(x)	(x)
#endif

typedef pgdval_t pgd_t;
#define pgd_val(x)	(x)
#define __pgd(x)	(x)

typedef pteval_t pgprot_t;
#define pgprot_val(x)	(x)
#define __pgprot(x)	(x)

#endif /* STRICT_MM_TYPECHECKS */

#if CONFIG_PGTABLE_LEVELS == 2
#include <asm-generic/pgtable-nopmd.h>
#elif CONFIG_PGTABLE_LEVELS == 3
#include <asm-generic/pgtable-nopud.h>
#endif

#endif	/* __ASM_PGTABLE_TYPES_H */
