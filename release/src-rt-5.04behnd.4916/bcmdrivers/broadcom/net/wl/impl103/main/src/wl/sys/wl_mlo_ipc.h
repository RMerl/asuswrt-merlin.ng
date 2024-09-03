/*
 * +--------------------------------------------------------------------------+
 * WL_MLO_IPC: Inter Processor communication between MLO AP's
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
 * $Id: wl_mlo_ipc.h 824047 2023-04-18 07:50:55Z $
 * +--------------------------------------------------------------------------+
 */

#ifndef _WL_MLO_IPC_H_
#define _WL_MLO_IPC_H_

extern int	wl_mlo_ipc_init(struct wl_info * wl);
extern void	wl_mlo_ipc_deinit(struct wl_info * wl);

extern int	wl_mlo_ipc_start(struct wl_info * wl);
extern int	wl_mlo_ipc_process_event(struct wl_info * wl, uint8 mlc_state);
extern void	wl_mlo_ipc_mlc_state_complete(struct wl_info * wl,  uint8 mlc_state);

extern bool	wl_mlo_ipc_is_mlo_ap(struct wl_info *wl);
extern uint16	wl_mlo_get_dump_signature(void);
extern void	wl_mlo_reset_dump_signature(void);

extern int	wl_mlo_ipc_wl_ioctl_intercept(struct wl_info *wl, wl_if_t *wlif,
	wl_ioctl_t *ioc, void * buf);

extern bool	wl_mlo_ipc_is_active(struct wl_info * wl);
extern bool	wl_mlo_ipc_is_ready(struct wl_info * wl);

extern int	wl_mlo_ipc_evt_bcmc_xmit_req(struct wl_info * wl, uint16 dest_ap_unit,
	void *pkt, uint16 seq, int8 mld_unit);
#endif /* _WL_MLO_IPC_H_ */
