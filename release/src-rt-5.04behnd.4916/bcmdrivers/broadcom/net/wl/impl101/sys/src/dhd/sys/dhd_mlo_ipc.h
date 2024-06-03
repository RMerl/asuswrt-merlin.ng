/*
 * +--------------------------------------------------------------------------+
 * DHD_MLO_IPC: Inter Processor communication between MLO AP's
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
 * $Id: dhd_mlo_ipc.h 832742 2023-11-13 04:18:45Z $
 * +--------------------------------------------------------------------------+
 */

#ifndef _DHD_MLO_IPC_H_
#define _DHD_MLO_IPC_H_

extern int	dhd_mlo_ipc_init(dhd_pub_t * dhd_pub);
extern void	dhd_mlo_ipc_deinit(dhd_pub_t * dhd_pub);

extern int	dhd_mlo_ipc_process_dongle_event(dhd_pub_t *dhd_pub, uint8 mlc_state);
extern int	dhd_mlo_ipc_wlioctl_intercept(dhd_pub_t *dhd_pub, int ifidx,
	wl_ioctl_t * ioc, void * buf);
extern int	dhd_mlo_ipc_start(dhd_pub_t *dhd_pub);

extern bool	dhd_is_mlo_ap(dhd_pub_t * dhd_pub);
extern bool	dhd_is_mlo_map(dhd_pub_t * dhd_pub);
extern int	dhd_mlo_state(dhd_pub_t * dhd_pub);

extern int	dhd_mlo_ipc_update_suspend_state(dhd_pub_t *dhd_pub);
#ifdef MLO_BCMC
extern int	dhd_mlo_ipc_evt_bcmc_xmit_req(dhd_pub_t *dhd_pub,
	uint16 dest_ap_unit, void *pkt, uint16 seq, int8 mld_unit);
#endif /* MLO_BCMC */
extern bool dhd_mlo_ipc_is_active(dhd_pub_t *dhd_pub);
extern bool dhd_mlo_ipc_is_ready(dhd_pub_t *dhd_pub);
extern uint16 dhd_mlo_get_dump_signature(void);
#endif /* _DHD_MLO_IPC_H_ */
