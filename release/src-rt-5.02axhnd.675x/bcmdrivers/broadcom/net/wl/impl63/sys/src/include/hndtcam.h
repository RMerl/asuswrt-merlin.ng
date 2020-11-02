/*
 * HND SOCRAM TCAM software interface - OS independent.
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
 * $Id: hndtcam.h 687674 2017-03-02 05:07:55Z $
 */
#ifndef _hndtcam_h_
#define _hndtcam_h_

#include <typedefs.h>

/*
 * 0 - 1
 * 1 - 2 Consecutive locations are patched
 * 2 - 4 Consecutive locations are patched
 * 3 - 8 Consecutive locations are patched
 * 4 - 16 Consecutive locations are patched
 * Define default to patch 2 locations
 */

#ifdef  PATCHCOUNT
#define SRPC_PATCHCOUNT PATCHCOUNT
#else
#define PATCHCOUNT 0
#define SRPC_PATCHCOUNT PATCHCOUNT
#endif // endif

#if defined(__ARM_ARCH_7R__)
#if !defined(PATCHCOUNT) || (PATCHCOUNT == 0)
#undef PATCHCOUNT
#define PATCHCOUNT 1
#endif // endif
#define	ARMCR4_TCAMPATCHCOUNT	PATCHCOUNT
#define ARMCR4_TCAMADDR_MASK (~((1 << (ARMCR4_TCAMPATCHCOUNT + 2))-1))
#define ARMCR4_PATCHNLOC (1 << ARMCR4_TCAMPATCHCOUNT)
#endif	/* defined(__ARM_ARCH_7R__) */

#if defined(__ARM_ARCH_7A__)
#undef PATCHCOUNT
#undef SRPC_PATCHCOUNT
#define PATCHCOUNT 0
#define SRPC_PATCHCOUNT PATCHCOUNT
#define ARMCA7_TCAMPATCHCOUNT 2
#define ARMCA7_PATCHNLOC (1 << ARMCA7_TCAMPATCHCOUNT)
#endif	/* defined(__ARM_ARCH_7A__) */

/* N Consecutive location to patch */
#define SRPC_PATCHNLOC (1 << (SRPC_PATCHCOUNT))

#define PATCHHDR(_p)		__attribute__ ((__section__ (".patchhdr."#_p))) _p
#define PATCHENTRY(_p)		__attribute__ ((__section__ (".patchentry."#_p))) _p

#if defined(__ARM_ARCH_7R__)
typedef struct {
	uint32	data[ARMCR4_PATCHNLOC];
} patch_entry_t;
#elif defined(__ARM_ARCH_7A__)
typedef struct {
	uint32	data[ARMCA7_PATCHNLOC];
} patch_entry_t;
#else
typedef struct {
	uint32	data[SRPC_PATCHNLOC];
} patch_entry_t;
#endif // endif

typedef struct {
	void		*addr;		/* patch address */
	patch_entry_t	*entry;		/* patch entry data */
} patch_hdr_t;

/* patch values and address structure */
typedef struct patchaddrvalue {
	uint32	addr;
	uint32	value;
} patchaddrvalue_t;

/* patch table region */
extern patch_entry_t _patch_table_start[], _patch_table_last[], _patch_table_end[];
/* control block region */
extern patch_hdr_t _patch_hdr_start[], _patch_hdr_end[];
extern char _patch_align_start[];

extern void hnd_tcam_write(volatile void *srp, uint16 idx, uint32 data);
extern void hnd_tcam_read(volatile void *srp, uint16 idx, uint32 *content);
extern void *hnd_tcam_init(volatile void *srp, int no_addrs);
extern void hnd_tcam_disablepatch(volatile void *srp);
extern void hnd_tcam_patchdisable(void);
extern void hnd_tcam_enablepatch(volatile void *srp);
#ifdef CONFIG_XIP
extern void hnd_tcam_bootloader_load(volatile void *srp, char *pvars);
#else
extern void hnd_tcam_load(volatile void *srp, const  patchaddrvalue_t *patchtbl);
#endif /* CONFIG_XIP */
extern void hnd_tcam_load_default(uint32 rambase);

void hnd_tcam_stat(void);

#endif /* _hndtcam_h_ */
