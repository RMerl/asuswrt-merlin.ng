/* D11 macdbg function prototypes for Broadcom 802.11abgn
 * Networking Adapter Device Drivers.
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
 * $Id: dhd_macdbg.h 799096 2021-05-20 06:46:32Z $
 */

#ifndef _dhd_macdbg_h_
#define _dhd_macdbg_h_

struct si_pub;

#include <dngl_stats.h>
#include <dhd.h>

#define DUMPMAC_BUF_SZ (256 * 1024)
#define DUMPMAC_FILENAME_SZ 48

/*
 * this list defines all dumps that can be generated
 * by macdbg during firmware trap handling. This is the
 * only place that you need to modify when adding a new
 * dump. The remaining thing is to implement the dump
 * function itself in dhd_macdbg.c. All other constructs
 * are taken care off by pre-processor macro expansion.
 */
#define MACDBGDUMP_ENUMDEF_LIST \
	MACDBGDUMP_ENUMDEF(MAC, mac) \
	MACDBGDUMP_ENUMDEF(MAC1, mac1) \
	MACDBGDUMP_ENUMDEF(MACX, macx) \
	MACDBGDUMP_ENUMDEF(SCTPL, sctpl) \
	MACDBGDUMP_ENUMDEF(SCPSM, scpsm) \
	MACDBGDUMP_ENUMDEF(RLMEM, ratelinkmem) \
	MACDBGDUMP_ENUMDEF(PHYSVMP, physvmp) \
	MACDBGDUMP_ENUMDEF(RXBM, rxbm) \
	MACDBGDUMP_ENUMDEF(BMC, bmc) \
	MACDBGDUMP_ENUMDEF(PERUSER, peruser)

extern int dhd_macdbg_attach(dhd_pub_t *dhdp);
extern void dhd_macdbg_detach(dhd_pub_t *dhdp);
extern void dhd_macdbg_event_handler(dhd_pub_t *dhdp, uint32 reason,
	uint8 *event_data, uint32 datalen);
extern void dhd_macdbg_upd_revinfo(dhd_pub_t *dhdp, wlc_rev_info_t *revinfo);
extern int dhd_macdbg_dump(dhd_pub_t *dhdp, char *buf, int buflen, const char *name);
extern int dhd_macdbg_dumpiov(dhd_pub_t *dhdp, char *params, int plen, char *buf, int buflen);
extern int dhd_macdbg_pd11regs(dhd_pub_t *dhdp, char *params, int plen, char *buf, int buflen);
extern int dhd_macdbg_psvmpmems(dhd_pub_t *dhdp, char *params, int plen, char *buf, int buflen);
extern int dhd_macdbg_dumprxbm(dhd_pub_t *dhdp, char *buf, int buflen, const char *filename);

#ifdef BCMPCIE
extern int dhd_macdbg_halt_psms(struct si_pub *sih);
extern int dhd_macdbg_dump_dongle(struct si_pub *sih, dump_dongle_in_t ddi,
	int len, dump_dongle_out_t *ddo);
bool dhd_macdbg_iscoreup(dhd_pub_t *dhdp);
#endif /* BCMPCIE */

#endif /* _dhd_macdbg_h_ */
