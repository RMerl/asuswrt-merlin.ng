/*
 * Misc useful routines to access NIC local SROM/OTP .
 *
 * Copyright (C) 2022, Broadcom. All Rights Reserved.
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
 * $Id: bcmsromio.h $
 */

#ifndef	_bcmsromio_h_
#define	_bcmsromio_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmsrom_tbl.h>
#include <bcmsrom.h>

#define MAX_SROM_SIZE SROM13_WORDS

struct dhd_bus;

extern int BCMATTACHFN(init_srom_sw_map)(si_t *sih, uint chipId, void *buf, uint nbytes);
extern int read_sromfile(void *swmap, void *buf, uint offset, uint nbytes);
extern int BCMATTACHFN(init_nvramvars)(si_t *sih, osl_t *osh, varbuf_t *b);
extern int BCMATTACHFN(varbuf_append)(varbuf_t *b, const char *fmt, ...);

extern uint8* srom_offset(si_t *sih, void *curmap);
extern int dhdpcie_download_map_bin(struct dhd_bus *bus);
extern int sprom_update_params(si_t *sbh, uint16 *buf);

#endif	/* _bcmsromio_h_ */
