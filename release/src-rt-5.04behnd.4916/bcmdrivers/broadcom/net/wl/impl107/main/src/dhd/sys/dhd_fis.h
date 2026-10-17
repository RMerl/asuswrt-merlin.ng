/*
 * FIS (Force Immediate Save) Headers and Definition for DHD
 *
 * Copyright (C) 2025, Broadcom. All Rights Reserved.
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
 * $Id$
 */

#ifndef _DHD_FIS_DUMP_H_INCLUDED_
#define _DHD_FIS_DUMP_H_INCLUDED_

struct dhd_fis {
	int cur_asm;
	int fis_status;
	int delay;
	bool deinit;
	bool fis_trig_iovar;
};

enum asm_core_type {
	NA = 0,
	CPUSUB = 1,
	DIG = 2,
	PHY1PHY2 = 3,
	PHYRX1 = 4,
	PHYRX2 = 5,
	PHYRX4 = 6,
	D11MAC1 = 7,
	D11MAC2 = 8,
	CHIP_ALL = 9,
	CHIP_WO_DIG = 10,
	CHIP_MAC_PHY = 11,
	LAST
};

enum fis_cur_status {
	/* no any configuration */
	SR_NOT_INIT_YET = 0,
	/* disabled before asm load */
	SR_DISABLED = 1,
	/* asm_download */
	SR_ASM_LOAD = 2,
	/* asm load and SR enable */
	SR_ENABLED = 3,
	/* FIS triggered */
	SR_FIS_TRIG = 4,
	/* Reset done  */
	SR_FIS_RESET = 5
};

/* Trigger type - can add more types */
enum fis_trig_type {
	PMU_FIS_TRIGGER = 0,
	PCIE_FIS_TRIGGER = 1,
	MAC_FIS_TRIGGER = 2,
	PHY_FIS_TRIGGER = 3,
	MAX_FIS_TRIGGER
};

extern void dhd_fis_dump(dhd_pub_t *dhd, bool asm_dump, bool is_iovar);
extern void dhd_fis_init(dhd_pub_t *dhd);
extern void dhd_fis_deinit(dhd_pub_t *dhd);
extern int dhd_fis_reset(dhd_pub_t *dhd, bool force_reset);
extern int dhd_fis_trigger(dhd_pub_t *dhd, int trigger_type, bool is_iovar);
extern int dhd_fis_sr_init(dhd_pub_t *dhd);
extern int dhd_fis_asm_load(dhd_pub_t *dhd, int core, bool is_iovar);
extern int dhd_fis_enable_sreng(dhd_pub_t *dhd);
extern int dhd_fis_trigger_and_dump(dhd_pub_t *dhd, int core, int trigger_type);
extern void dhd_fis_reset_status(dhd_pub_t *dhd);
#endif /* _DHD_FIS_DUMP_H_INCLUDED_ */
