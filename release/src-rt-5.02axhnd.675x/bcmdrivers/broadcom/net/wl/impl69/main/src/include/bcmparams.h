/*
 * Misc system wide parameters.
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
 * $Id: bcmparams.h 712721 2017-07-26 04:04:13Z $
 */

#ifndef	_bcmparams_h_
#define	_bcmparams_h_

#define VLAN_MAXVID	15	/* Max. VLAN ID supported/allowed */

#define VLAN_NUMPRIS	8	/* # of prio, start from 0 */

#ifdef BCA_HNDROUTER
#define DEV_NUMIFS	32	/* Max. # of devices/interfaces supported */
#else
#define DEV_NUMIFS	16	/* Max. # of devices/interfaces supported */
#endif /* BCA_HNDROUTER */

#define WL_MAXBSSCFG	16	/* maximum number of BSS Configs we can configure */

#endif /* _bcmparams_h_ */
