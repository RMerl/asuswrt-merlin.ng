/*
 * TCAM Access routines - OS independent
 *
 * TCAM init/load can be done before si_attach()
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: hndtcam.c 821234 2023-02-06 14:16:52Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#if defined(__ARM_ARCH_7A__)
#include <sbsysmem.h>
#else
#include <sbsocram.h>
#endif
#include <hndtcam.h>

#define TCAM_OUT(args)		printf args
#ifdef BCMDBG_TCAM
#define TCAM_TRACE(args)	printf args
#else
#define TCAM_TRACE(args)
#endif

#define PATCHHDR(_p)		__attribute__ ((__section__ (".patchhdr."#_p))) _p
#define PATCHENTRY(_p)		__attribute__ ((__section__ (".patchentry."#_p))) _p
#define PATBL_IDX(_p)		((_p) - _patch_table_start)
#define PATBL_ENTRY(i)		(&_patch_table_start[(i)])
#define PATBL_CNT		(_patch_table_last - _patch_table_start)
#define PATBL_MAX		(_patch_table_end - _patch_table_start)
#define PATBL_TABLE		_patch_table_start
#define PATBL_ESIZE		(sizeof(patch_entry_t))
#define PATBL_ESTART(p)		(((uintptr)(p)) & ~(PATBL_ESIZE - 1))
#define PATBL_EEND(p)		(((patch_entry_t *)PATBL_ESTART(p)) + 1)
#define PATHDR_SIZE		(sizeof(patch_hdr_t))
#define PATHDR_CNT		(_patch_hdr_end - _patch_hdr_start)
#define PATHDR_TABLE		_patch_hdr_start

#define BYTE_PER_PATCHLOC	16

#ifndef BCMDBG_TCAM
#ifndef TCAM_MAX
#define TCAM_MAX PATBL_MAX
#endif
#else
#ifndef TCAM_MAX
#define TCAM_MAX TCAM_SIZE
#endif

typedef struct {
	patch_hdr_t hdr;
	patch_entry_t ram_entry;
	patch_entry_t rom_entry;
	patch_hdr_t *hdr_addr;
	patch_entry_t *entry_addr;
	uint32 hidx;
	void   *hentry;
	uint32 eidx;
	uint32 tcam_val;
	uint32 nentries;
	uint32 *staddr;
	uint32 *endaddr;
	uint32 tcam_entries[TCAM_MAX + 1];
} tcam_dump_t;

static tcam_dump_t tcam_dbg;
#endif /* BCMDBG_TCAM */

static uint32 *g_patchtable = NULL;
/*
 * FIX: create patch table for only entries needed
 * g_npatch <= total TCAM entries
 */
static uint16 g_npatch = 0;

extern uint8 patch_pair;

#ifdef  __ARM_ARCH_7A__
extern volatile void *sysmem_regs;
extern uint32 sysmem_rev;
#else
extern volatile void *socram_regs;
extern uint32 socram_rev;
#endif

extern volatile void *arm_regs;

/* Patching mechanism is different between CR4 and old cores
 * In old arm core (cm3) the patching is handled by socram core
 * In cr4 as there is no socram the patching is handled by
 * a wrapper logic in arm itself. So all the registers for
 * patching are different between cm3 and cr4
 */

#if defined(__ARM_ARCH_7R__)

#define TCAM_WRDAT(m, p, v)		((p & m) | v)
void
BCMATTACHFN(hnd_tcam_write)(volatile void *armp, uint16 idx, uint32 data)
{
	W_REG(SI_OSH, ARMREG(armp, tcamdatareg), data);
	W_REG(SI_OSH, ARMREG(armp, tcamcmdreg), (ARMCR4_TCAM_WRITE | idx));
	SPINWAIT(((R_REG(SI_OSH, ARMREG(armp, tcamcmdreg)) & ARMCR4_TCAM_CMD_DONE) == 0),
		ARMCR4_TCAM_CMD_DONE_DLY);
}

void
BCMATTACHFN(hnd_tcam_read)(volatile void *armp, uint16 idx, uint32 *content)
{
	W_REG(SI_OSH, ARMREG(armp, tcamcmdreg), (ARMCR4_TCAM_READ | idx));
	SPINWAIT(((R_REG(SI_OSH, ARMREG(armp, tcamcmdreg)) & ARMCR4_TCAM_CMD_DONE) == 0),
		ARMCR4_TCAM_CMD_DONE_DLY);
	*content = R_REG(SI_OSH, ARMREG(armp, tcamdatareg));
}

void *
BCMATTACHFN(hnd_tcam_init)(volatile void *armp, int no_addrs)
{
	uint32 *pa = NULL;
	int i, entries = 0;
	uint32 patchctrl;

	ASSERT(g_patchtable == NULL);
	if (g_patchtable != NULL)
		return NULL;

	entries = TCAM_MAX;

	if (no_addrs == -1)
		no_addrs = entries;
	ASSERT(no_addrs <= entries);

	g_npatch = MIN(no_addrs, entries);
	if (entries) {
		uint size = (sizeof(uint) * (no_addrs * ARMCR4_PATCHNLOC));
#ifndef CONFIG_XIP
		/* table has to aligned based on tcam size * conecutive patch
		 * count * 4. Log of this is: 8 + ARMCR4_TCAMPATCHCOUNT + 2.
		 */
		uint align_bits = (8 + 2 + (ARMCR4_TCAMPATCHCOUNT));

		pa = (uint32 *)MALLOC_ALIGN(SI_OSH, size, align_bits);
		ASSERT(pa != NULL);
		if (pa == NULL)
			return NULL;
#else
		extern uint32 _atcmrambase;

		/* In case of bootloader fixing the table to begining ofATCM
		 * In case of 4335 the heap for bootloader will be present in
		 * BTCM. But patch table shoud be placed in ATCM. For this
		 * purpose initially the patchtable will put put at the
		 * begining of ATCM and the code will be downloaded after
		 * this table. Before start executing the firmware the
		 * bootloader will disable patch and copy the image back
		 * to this place.
		 */
		pa = (uint32 *)(_atcmrambase);
#endif /* CONFIG_XIP */
		bzero(pa, size);
		g_patchtable = (uint32 *) pa;

		/* Disable patching */
		hnd_tcam_disablepatch(armp);

		/* Program the patch count and enable clock */
		patchctrl = R_REG(SI_OSH, ARMREG(armp, tcampatchctrl));
		patchctrl &= ~(ARMCR4_TCAM_PATCHCNT_MASK);
		patchctrl |= (ARMCR4_TCAM_CLKENAB | ARMCR4_TCAMPATCHCOUNT);

		W_REG(SI_OSH, ARMREG(armp, tcampatchctrl), patchctrl);
		W_REG(SI_OSH, ARMREG(armp, tcampatchtblbaseaddr), (uint32)pa);

		/* initialize cam entries to zero */
		for (i = 0; i < entries; i++)
			hnd_tcam_write(armp, i, 0);
	}

	return (void *)pa;
}

/** Enable patching */
void
BCMATTACHFN(hnd_tcam_enablepatch)(volatile void *armp)
{
	OR_REG(SI_OSH, ARMREG(armp, tcampatchctrl), ARMCR4_TCAM_ENABLE);
	/* Wait for write completion */
	SPINWAIT(((R_REG(SI_OSH, ARMREG(armp, tcampatchctrl)) & ARMCR4_TCAM_ENABLE) == 0),
		ARMCR4_TCAM_CMD_DONE_DLY);
}

/** Disable patching */
void
BCMATTACHFN(hnd_tcam_disablepatch)(volatile void *armp)
{
	W_REG(SI_OSH, ARMREG(armp, tcampatchctrl), (~(ARMCR4_TCAM_ENABLE)
		& (R_REG(SI_OSH, ARMREG(armp, tcampatchctrl)))));
	/* Wait for write completion */
	SPINWAIT(((R_REG(SI_OSH, ARMREG(armp, tcampatchctrl)) & ARMCR4_TCAM_ENABLE)
		== ARMCR4_TCAM_ENABLE), ARMCR4_TCAM_CMD_DONE_DLY);
}

void
hnd_tcam_patchdisable(void)
{
	hnd_tcam_disablepatch((volatile void *)arm_regs);
}

#if defined(BCMDBG_ASSERT)
static void
BCMATTACHFN(hnd_rom_banks)(uint32 cinfo, uint32 *nbnk)
{
	/* For cr4rev >= 14 , chipcorecapability bits 8-11 and 21-24
	 * are used to find number of rombanks programmed
	 */

#if (BCMCR4REV >= 14)
	*nbnk |= ((cinfo & ARMCR4_MSB_ROMNB_MASK) >> ARMCR4_MSB_ROMNB_SHIFT);
#endif /* BCMCR4REV */
}

/** Get ROM size */
static uint32
BCMATTACHFN(hnd_tcam_romsize)(volatile void *armp)
{
	uint32 cinfo, binfo;
	uint32 nbnk;
	uint32 romsz = 0, i;

	cinfo = R_REG(SI_OSH, ARMREG(armp, corecapabilities));
	nbnk = (cinfo & ARMCR4_ROMNB_MASK) >> ARMCR4_ROMNB_SHIFT;
	/* Updating nbnks for cr4rev >=14 */
	hnd_rom_banks(cinfo, &nbnk);

	for (i = 0; i < nbnk; i++) {
		W_REG(SI_OSH, ARMREG(armp, bankidx), (ARMCR4_MT_ROM | i));
		binfo = R_REG(SI_OSH, ARMREG(armp, bankinfo));
		romsz += ((binfo & ARMCR4_BSZ_MASK) + 1) * ARMCR4_BSZ_MULT;
	}
	return romsz;
}
#endif

#ifdef CONFIG_XIP
void
BCMATTACHFN(hnd_tcam_bootloader_load)(volatile void *armp, char *pvars)
{
	uint32 i;
	uint32 paddr;
	uint32 *pdataptr = NULL;

	pdataptr = (uint32 *)hnd_tcam_init(armp, patch_pair);
	if (pdataptr == NULL)
		return;

	ASSERT(hnd_tcam_romsize(armp) != 0);

	for (i = 0; i < patch_pair; i++) {
		char pa[8], pdh[8], pdl[8];

		(void)snprintf(pa, sizeof(pa), "pa%d", i);
		(void)snprintf(pdh, sizeof(pdh), "pdh%d", i);
		(void)snprintf(pdl, sizeof(pdl), "pdl%d", i);

		pdataptr[i*2] = getintvar(pvars, pdl);
		pdataptr[i*2+1] = getintvar(pvars, pdh);

		paddr = (uint32) getintvar(pvars, pa);

		hnd_tcam_write(armp, i, TCAM_WRDAT(ARMCR4_TCAMADDR_MASK, paddr, 0x1));
	}

	hnd_tcam_enablepatch(armp);
}

#else
static bool
BCMATTACHFN(hnd_tcam_isenab)(volatile void *armp)
{
	uint32 val;

	val = R_REG(SI_OSH, ARMREG(armp, tcampatchctrl));
	return ((val & ARMCR4_TCAM_ENABLE) != 0);
}

/** Find free entry in the cam */
static int
BCMATTACHFN(hnd_tcam_entry_get)(volatile void *armp)
{
	int i;
	uint32 content;

	ASSERT(g_npatch);
	for (i = 0; i < g_npatch; i++) {
		hnd_tcam_read(armp, i, &content);
		if (content == 0)
			return i;
	}

	return -1;
}

/** Loads a patch pair */
void
BCMATTACHFN(hnd_tcam_load)(volatile void *armp, const patchaddrvalue_t *patchtbl)
{
	uint32 pa;
	int entry, i;
	int offset;
	bool enab;

	ASSERT(g_patchtable);
	ASSERT(g_npatch);
	if ((g_patchtable == NULL))
		return;

	enab = hnd_tcam_isenab(armp);
	if (enab)
		hnd_tcam_disablepatch(armp);

	entry = hnd_tcam_entry_get(armp);
	if (entry < 0) {
		if (enab)
			hnd_tcam_enablepatch(armp);
		return;
	}

	ASSERT(hnd_tcam_romsize(armp) != 0);

	pa = patchtbl[0].addr;
	hnd_tcam_write(armp, entry, TCAM_WRDAT(ARMCR4_TCAMADDR_MASK, pa, 0x1));
	offset = entry * ARMCR4_PATCHNLOC;

	for (i = 0; i < ARMCR4_PATCHNLOC; i++)
		g_patchtable[offset++] = patchtbl[i].value;

	hnd_tcam_enablepatch(armp);
}

#endif /* CONFIG_XIP */
#elif defined(__ARM_ARCH_7A__)
#define TCAM_ADDR_VALID(rs)		(0x1)
#define TCAM_WRDAT(p, v)		((p) | (v))

void
BCMATTACHFN(hnd_tcam_write)(volatile void *srp, uint16 idx, uint32 data)
{
	sysmemregs_t *sr = (sysmemregs_t *)srp;
	W_REG(SI_OSH, &sr->cambankdatareg, data);
	W_REG(SI_OSH, &sr->cambankcmdreg, (SRCMD_WRITE | idx));
	SPINWAIT(((R_REG(SI_OSH, &sr->cambankcmdreg) & SRCMD_DONE) == 0),
		SRCMD_DONE_DLY);
}

void
BCMATTACHFN(hnd_tcam_read)(volatile void *srp, uint16 idx, uint32 *content)
{
	sysmemregs_t *sr = (sysmemregs_t *)srp;
	W_REG(SI_OSH, &sr->cambankcmdreg, (SRCMD_READ | idx));
	SPINWAIT(((R_REG(SI_OSH, &sr->cambankcmdreg) & SRCMD_DONE) == 0),
		SRCMD_DONE_DLY);
	*content = R_REG(SI_OSH, &sr->cambankdatareg);
}

extern uint8 patch_pair;
void *
BCMATTACHFN(hnd_tcam_init)(volatile void *srp, int no_addrs)
{
	sysmemregs_t *sr = (sysmemregs_t *)srp;
	uint32 *pa = NULL;
	int i, entries = 0;

	ASSERT(g_patchtable == NULL);
	if (g_patchtable != NULL)
		return NULL;

	entries = SRECC_BANKSIZE((R_REG(SI_OSH, &sr->extracoreinfo)
		& SRECC_BANKSIZE_MASK));

	if (no_addrs == -1)
		no_addrs = entries;
	ASSERT(no_addrs <= entries);

	g_npatch = MIN(no_addrs, entries);
	if (entries) {
		uint size = (sizeof(uint) * (no_addrs * SRPC_PATCHNLOC));
		uint align_bits = (9 + (SRPC_PATCHCOUNT) + 4);

		/* SRPC_PATCHNLOC will be 1 for bootloader */
		/* table has to aligned based on tcam size(512) * conecutive patch
		 * count * 16(byte). Log of this is: 9 + SRPC_PATCHCOUNT + 4.
		 */
		pa = (uint32 *)MALLOC_ALIGN(SI_OSH, size, align_bits);
		ASSERT(pa != NULL);
		if (pa == NULL)
			return NULL;

		bzero(pa, size);
		g_patchtable = (uint32 *) pa;

		/* Disable patching */
		hnd_tcam_disablepatch(srp);

		/* Just program the patch count here */
		W_REG(SI_OSH, &sr->cambankpatchctrl, (SRPC_PATCHCOUNT));
		W_REG(SI_OSH, &sr->cambankpatchtblbaseaddr, (uint32)pa);

		/* initialize cam entries to zero */
		for (i = 0; i < entries; i++)
			hnd_tcam_write(srp, i, 0);
	}

	return (void *)pa;
}

/** Enable patching */
void
BCMATTACHFN(hnd_tcam_enablepatch)(volatile void *srp)
{
	sysmemregs_t *sr = (sysmemregs_t *)srp;
	OR_REG(SI_OSH, &sr->cambankpatchctrl, SRCBPC_PATCHENABLE);
}

/** Disable patching */
void
BCMATTACHFN(hnd_tcam_disablepatch)(volatile void *srp)
{
	sysmemregs_t *sr = (sysmemregs_t *)srp;
	W_REG(SI_OSH, &sr->cambankpatchctrl, (~(SRCBPC_PATCHENABLE)
		& (R_REG(SI_OSH, &sr->cambankpatchctrl))));
}

void
hnd_tcam_patchdisable(void)
{
	hnd_tcam_disablepatch((volatile void *)sysmem_regs);
}

/** Get ROM size */
static uint32
BCMATTACHFN(hnd_tcam_romsize)(sysmemregs_t *sr)
{
	uint32 cinfo, binfo;
	uint32 nbnk, bnksz;
	uint32 romsz = 0, i;

	cinfo = R_REG(SI_OSH, &sr->coreinfo);
	nbnk = (cinfo & SYSMEM_SRCI_ROMNB_MASK) >> SYSMEM_SRCI_ROMNB_SHIFT;

	for (i = 0; i < nbnk; i++) {
		W_REG(SI_OSH, &sr->bankidx, i);
		binfo = R_REG(SI_OSH, &sr->bankinfo);
		ASSERT(binfo & SYSMEM_BANKIDX_ROM_MASK);

		bnksz = binfo & SYSMEM_BANKINFO_SZMASK;
		romsz += (bnksz + 1) * SYSMEM_BANKINFO_SZBASE;
	}

	return romsz;
}

#ifdef CONFIG_XIP
void
BCMATTACHFN(hnd_tcam_bootloader_load)(volatile void *srp, char *pvars)
{
	uint32 i;
	uint32 paddr;
	uint32 *pdataptr = NULL;
	uint32 romsz, valid = 0, mask = 0;

	pdataptr = (uint32 *)hnd_tcam_init(srp, patch_pair);
	if (pdataptr == NULL)
		return;

	romsz = hnd_tcam_romsize(srp);
	ASSERT(romsz);
	mask = TCAM_ADDR_MASK(romsz);
	valid = TCAM_ADDR_VALID(romsz);

	for (i = 0; i < patch_pair; i++) {
		char pa[8], pd[8];

		(void)snprintf(pa, sizeof(pa), "pa%d", i);
		(void)snprintf(pd, sizeof(pd), "pd%d", i);

		pdataptr[i] = getintvar(pvars, pd);
		paddr = (uint32) getintvar(pvars, pa);

		hnd_tcam_write(srp, i, TCAM_WRDAT(paddr, valid));
	}

	hnd_tcam_enablepatch(srp);
}

#else
static bool
BCMATTACHFN(hnd_tcam_isenab)(sysmemregs_t *sr)
{
	uint32 val;

	val = R_REG(SI_OSH, &sr->cambankpatchctrl);
	return ((val & SRCBPC_PATCHENABLE) != 0);
}

/** Find free entry in the cam */
static int
BCMATTACHFN(hnd_tcam_entry_get)(sysmemregs_t *sr)
{
	int i;
	uint32 content;

	ASSERT(g_npatch);
	for (i = 0; i < g_npatch; i++) {
		hnd_tcam_read((volatile void *)sr, i, &content);
		if (content == 0)
			return i;
	}

	return -1;
}

/** Loads a patch pair */
void
BCMATTACHFN(hnd_tcam_load)(volatile void *srp, const patchaddrvalue_t *patchtbl)
{
	sysmemregs_t *sr = (sysmemregs_t *)srp;
	uint32 pa, valid;
	int entry, i;
	int offset, romsz;
	bool enab;

	BCM_REFERENCE(romsz);
	ASSERT(g_patchtable);
	ASSERT(g_npatch);
	if (g_patchtable == NULL)
		return;

	enab = hnd_tcam_isenab(sr);
	if (enab)
		hnd_tcam_disablepatch(srp);

	entry = hnd_tcam_entry_get(sr);
	if (entry < 0) {
		if (enab)
			hnd_tcam_enablepatch(srp);
		return;
	}

	romsz = hnd_tcam_romsize(sr);
	ASSERT(romsz);
	valid = TCAM_ADDR_VALID(romsz);

	pa = patchtbl[0].addr;
	hnd_tcam_write(srp, entry, TCAM_WRDAT(pa, valid));
	offset = entry * SRPC_PATCHNLOC;

	for (i = 0; i < SRPC_PATCHNLOC; i++) {
		g_patchtable[offset * (BYTE_PER_PATCHLOC / sizeof(uint32))] =
			patchtbl[i].value;
		offset++;
	}

	hnd_tcam_enablepatch(srp);
}

#endif /* CONFIG_XIP */

#else /* ! __ARM_ARCH_7R__ && ! __ARM_ARCH_7A__below */

#define TCAM_ADDR_MASK(rs)		((rs) - 4)
#define TCAM_ADDR_VALID(rs)		((socram_rev >= 15) ? 0x1 : (rs) >> 2)
#define TCAM_WRDAT(m, p, v)		((socram_rev >= 15) ? \
					((p) | (v)) : ((((m) & (p)) >> 2) | (v)))

void
BCMATTACHFN(hnd_tcam_write)(volatile void *srp, uint16 idx, uint32 data)
{
	sbsocramregs_t *sr = (sbsocramregs_t *)srp;
	W_REG(SI_OSH, &sr->cambankdatareg, data);
	W_REG(SI_OSH, &sr->cambankcmdreg, (SRCMD_WRITE | idx));
	SPINWAIT(((R_REG(SI_OSH, &sr->cambankcmdreg) & SRCMD_DONE) == 0),
		SRCMD_DONE_DLY);
}

void
BCMATTACHFN(hnd_tcam_read)(volatile void *srp, uint16 idx, uint32 *content)
{
	sbsocramregs_t *sr = (sbsocramregs_t *)srp;
	W_REG(SI_OSH, &sr->cambankcmdreg, (SRCMD_READ | idx));
	SPINWAIT(((R_REG(SI_OSH, &sr->cambankcmdreg) & SRCMD_DONE) == 0),
		SRCMD_DONE_DLY);
	*content = R_REG(SI_OSH, &sr->cambankdatareg);
}

extern uint8 patch_pair;
void *
BCMATTACHFN(hnd_tcam_init)(volatile void *srp, int no_addrs)
{
	sbsocramregs_t *sr = (sbsocramregs_t *)srp;
	uint32 *pa = NULL;
	int i, entries = 0;

	ASSERT(g_patchtable == NULL);
	if (g_patchtable != NULL)
		return NULL;

	entries = SRECC_BANKSIZE((R_REG(SI_OSH, &sr->extracoreinfo)
		& SRECC_BANKSIZE_MASK));

	if (no_addrs == -1)
		no_addrs = entries;
	ASSERT(no_addrs <= entries);

	g_npatch = MIN(no_addrs, entries);
	if (entries) {
		uint size = (sizeof(uint) * (no_addrs * SRPC_PATCHNLOC));
		uint align_bits = (6 + 2 + (SRPC_PATCHCOUNT));

		/* SRPC_PATCHNLOC will be 1 for bootloader */
		/* table has to aligned based on tcam size * conecutive patch
		 * count * 4. Log of this is: 6 + SRPC_PATCHCOUNT + 2.
		 */
		pa = (uint32 *)MALLOC_ALIGN(SI_OSH, size, align_bits);
		ASSERT(pa != NULL);
		if (pa == NULL)
			return NULL;

		bzero(pa, size);
		g_patchtable = (uint32 *) pa;

		/* Disable patching */
		hnd_tcam_disablepatch(srp);

		/* Just program the patch count here */
		W_REG(SI_OSH, &sr->cambankpatchctrl, (SRPC_PATCHCOUNT));
		W_REG(SI_OSH, &sr->cambankpatchtblbaseaddr, (uint32)pa);

		/* initialize cam entries to zero */
		for (i = 0; i < entries; i++)
			hnd_tcam_write(srp, i, 0);
	}

	return (void *)pa;
}

/** Enable patching */
void
BCMATTACHFN(hnd_tcam_enablepatch)(volatile void *srp)
{
	sbsocramregs_t *sr = (sbsocramregs_t *)srp;
	OR_REG(SI_OSH, &sr->cambankpatchctrl, SRCBPC_PATCHENABLE);
}

/** Disable patching */
void
BCMATTACHFN(hnd_tcam_disablepatch)(volatile void *srp)
{
	sbsocramregs_t *sr = (sbsocramregs_t *)srp;
	W_REG(SI_OSH, &sr->cambankpatchctrl, (~(SRCBPC_PATCHENABLE)
		& (R_REG(SI_OSH, &sr->cambankpatchctrl))));
}

void
hnd_tcam_patchdisable(void)
{
	hnd_tcam_disablepatch((volatile void *)socram_regs);
}

/** Get ROM size */
static uint32
BCMATTACHFN(hnd_tcam_romsize)(sbsocramregs_t *sr)
{
	uint32 cinfo, binfo;
	uint32 nbnk, bnksz;
	uint32 romsz = 0, i;

	cinfo = R_REG(SI_OSH, &sr->coreinfo);
	nbnk = (cinfo & SRCI_ROMNB_MASK) >> SRCI_ROMNB_SHIFT;

	if ((socram_rev >= 3) && (socram_rev < 8)) {
		bnksz = (cinfo & SRCI_ROMBSZ_MASK) >> SRCI_ROMBSZ_SHIFT;
		romsz = (1 << (bnksz + 14)); /* 2 ** (bnksz + 14) */
		romsz = nbnk * romsz;
	} else if (socram_rev >= 8) {
		for (i = 0; i < nbnk; i++) {
			W_REG(SI_OSH, &sr->bankidx,
				(SOCRAM_MEMTYPE_ROM << SOCRAM_BANKIDX_MEMTYPE_SHIFT) | i);
			binfo = R_REG(SI_OSH, &sr->bankinfo);
			ASSERT(binfo & SOCRAM_BANKIDX_ROM_MASK);

			bnksz = binfo & SOCRAM_BANKINFO_SZMASK;
			romsz += (bnksz + 1) * SOCRAM_BANKINFO_SZBASE;
		}
	}

	return romsz;
}

#ifdef CONFIG_XIP
void
BCMATTACHFN(hnd_tcam_bootloader_load)(volatile void *srp, char *pvars)
{
	uint32 i;
	uint32 paddr;
	uint32 *pdataptr = NULL;
	uint32 romsz, valid = 0, mask = 0;

	BCM_REFERENCE(romsz);
	pdataptr = (uint32 *)hnd_tcam_init(srp, patch_pair);
	if (pdataptr == NULL)
		return;

	romsz = hnd_tcam_romsize(srp);
	ASSERT(romsz);
	mask = TCAM_ADDR_MASK(romsz);
	valid = TCAM_ADDR_VALID(romsz);

	for (i = 0; i < patch_pair; i++) {
		char pa[8], pd[8];

		(void)snprintf(pa, sizeof(pa), "pa%d", i);
		(void)snprintf(pd, sizeof(pd), "pd%d", i);

		pdataptr[i] = getintvar(pvars, pd);
		paddr = (uint32) getintvar(pvars, pa);

		hnd_tcam_write(srp, i, TCAM_WRDAT(mask, paddr, valid));
	}

	hnd_tcam_enablepatch(srp);
}

#else
static bool
BCMATTACHFN(hnd_tcam_isenab)(sbsocramregs_t *sr)
{
	uint32 val;

	val = R_REG(SI_OSH, &sr->cambankpatchctrl);
	return ((val & SRCBPC_PATCHENABLE) != 0);
}

/** Find free entry in the cam */
static int
BCMATTACHFN(hnd_tcam_entry_get)(sbsocramregs_t *sr)
{
	int i;
	uint32 content;

	ASSERT(g_npatch);
	for (i = 0; i < g_npatch; i++) {
		hnd_tcam_read((volatile void *)sr, i, &content);
		if (content == 0)
			return i;
	}

	return -1;
}

/** Loads a patch pair */
void
BCMATTACHFN(hnd_tcam_load)(volatile void *srp, const patchaddrvalue_t *patchtbl)
{
	sbsocramregs_t *sr = (sbsocramregs_t *)srp;
	uint32 pa, valid, mask;
	int entry, i;
	int offset, romsz;
	bool enab;

	ASSERT(g_patchtable);
	ASSERT(g_npatch);
	if ((socram_rev < 3) || (g_patchtable == NULL))
		return;

	enab = hnd_tcam_isenab(sr);
	if (enab)
		hnd_tcam_disablepatch(srp);

	entry = hnd_tcam_entry_get(sr);
	if (entry < 0) {
		if (enab)
			hnd_tcam_enablepatch(srp);
		return;
	}

	romsz = hnd_tcam_romsize(sr);
	ASSERT(romsz);
	mask = TCAM_ADDR_MASK(romsz);
	valid = TCAM_ADDR_VALID(romsz);

	pa = patchtbl[0].addr;
	hnd_tcam_write(srp, entry, TCAM_WRDAT(mask, pa, valid));
	offset = entry * SRPC_PATCHNLOC;

	for (i = 0; i < SRPC_PATCHNLOC; i++)
		g_patchtable[offset++] = patchtbl[i].value;

	hnd_tcam_enablepatch(srp);
}

#endif /* CONFIG_XIP */
#endif /* defined(__ARM_ARCH_7R__) */
