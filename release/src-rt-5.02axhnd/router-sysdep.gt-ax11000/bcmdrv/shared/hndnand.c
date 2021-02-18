/*
 * Broadcom chipcommon NAND flash interface
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
 * $Id: hndnand.c 596126 2015-10-29 19:53:48Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <hndsoc.h>
#include <sbhndcpu.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#include <hndnand.h>
#include <hndpmu.h>

/* Private global state */
static hndnand_t *hndnand = NULL;

extern hndnand_t *nflash_init(si_t *sih);
extern hndnand_t *nandcore_init(si_t *sih);
#ifdef  __mips__
#define nandcore_enable(sih, enable) do {} while (0)
#else
extern void nandcore_enable(si_t *sih, int enable);
#endif // endif

/* Initialize nand flash access */
hndnand_t *
hndnand_init(si_t *sih)
{
	uint32 origidx;

	ASSERT(sih);

	/* Already initialized ? */
	if (hndnand)
		return hndnand;

	origidx = si_coreidx(sih);

#ifdef	__mips__
	if (!hndnand)
		hndnand = nflash_init(sih);
#endif // endif
#ifdef __ARM_ARCH_7A__
	if (!hndnand) {
		nandcore_enable(sih, 1);
		hndnand = nandcore_init(sih);
		nandcore_enable(sih, 0);
	}
#endif // endif

	si_setcoreidx(sih, origidx);
	return hndnand;
}

void
hndnand_enable(hndnand_t *nfl, int enable)
{
	ASSERT(nfl);
	ASSERT(nfl->sih);

	if (nfl->enable) {
		nandcore_enable(nfl->sih, 1);
		/* Should spinlock here */
		(nfl->enable)(nfl, enable);
		nandcore_enable(nfl->sih, 0);
	}

	return;
}

/* Read len bytes starting at offset into buf. Returns number of bytes read. */
int
hndnand_read(hndnand_t *nfl, uint64 offset, uint len, uchar *buf)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->read);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->read)(nfl, offset, len, buf);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

/* Write len bytes starting at offset into buf. Returns number of bytes
 * written.
 */
int
hndnand_write(hndnand_t *nfl, uint64 offset, uint len, const uchar *buf)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->write);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->write)(nfl, offset, len, buf);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

/* Erase a region. Returns number of bytes scheduled for erasure.
 * Caller should poll for completion.
 */
int
hndnand_erase(hndnand_t *nfl, uint64 offset)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->erase);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->erase)(nfl, offset);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

int
hndnand_checkbadb(hndnand_t *nfl, uint64 offset)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->checkbadb);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->checkbadb)(nfl, offset);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

int
hndnand_mark_badb(hndnand_t *nfl, uint64 offset)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->markbadb);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->markbadb)(nfl, offset);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

#ifndef _CFE_
int
hndnand_dev_ready(hndnand_t *nfl)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->dev_ready);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->dev_ready)(nfl);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

int
hndnand_select_chip(hndnand_t *nfl, int chip)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->select_chip);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->select_chip)(nfl, chip);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

int hndnand_cmdfunc(hndnand_t *nfl, uint64 addr, int cmd)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->cmdfunc);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->cmdfunc)(nfl, addr, cmd);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

int
hndnand_waitfunc(hndnand_t *nfl, int *status)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->waitfunc);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->waitfunc)(nfl, status);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

int
hndnand_read_oob(hndnand_t *nfl, uint64 addr, uint8 *oob)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->read_oob);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->read_oob)(nfl, addr, oob);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

int
hndnand_write_oob(hndnand_t *nfl, uint64 addr, uint8 *oob)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->write_oob);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->write_oob)(nfl, addr, oob);
	nandcore_enable(nfl->sih, 0);

	return ret;
}
int
hndnand_read_page(hndnand_t *nfl, uint64 addr, uint8 *buf, uint8 *oob, bool ecc,
	uint32 *herr, uint32 *serr)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->read_page);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->read_page)(nfl, addr, buf, oob, ecc, herr, serr);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

int
hndnand_write_page(hndnand_t *nfl, uint64 addr, const uint8 *buf, uint8 *oob, bool ecc)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->write_page);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->write_page)(nfl, addr, buf, oob, ecc);
	nandcore_enable(nfl->sih, 0);

	return ret;
}

int
hndnand_cmd_read_byte(hndnand_t *nfl, int cmd, int arg)
{
	int ret;

	ASSERT(nfl);
	ASSERT(nfl->cmd_read_byte);
	ASSERT(nfl->sih);

	nandcore_enable(nfl->sih, 1);
	ret = (nfl->cmd_read_byte)(nfl, cmd, arg);
	nandcore_enable(nfl->sih, 0);

	return ret;
}
#endif /* _CFE_ */
