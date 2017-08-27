/*
 * Broadcom Home Gateway Reference Design
 * Broadcom Wi-Fi Blanket shared functions
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: wbd_rc_shared.h 670128 2016-11-14 12:15:13Z $
 */

#ifndef _WBD_RC_SHARED_H_
#define _WBD_RC_SHARED_H_

/* ----------------------------- WBD shared Routines --------------------------------- */

/* Check if Interface is DWDS Virtual Interface, if Disabled, Enable it */
extern int wbd_enable_dwds_ap_vif(char *ifname);

/* Check if Interface is DWDS Primary Interface, with mode = STA, Change name1 */
extern int wbd_check_dwds_sta_primif(char *ifname, char *ifname1, int len1);

/* Find First DWDS Primary Interface, with mode = STA */
extern int wbd_find_dwds_sta_primif(char *ifname, int len, char *ifname1, int len1);

/* Get "wbd_ifnames" from "lan_ifnames" */
extern int wbd_ifnames_fm_lan_ifnames(char *wbd_ifnames, int len, char *wbd_ifnames1, int len1);

/* Get next available Virtual AP Interface for DWDS Slave */
extern int wbd_get_dwds_ap_vif_subunit(int in_unit, int *error);

/* Create & Configure Virtual AP Interface, if WBD is ON and AP is WBD DWDS Slave */
extern int wbd_create_dwds_ap_vif(int unit, int subunit);

/* Read "wbd_ifnames" NVRAM and get actual ifnames */
extern int wbd_read_actual_ifnames(char *wbd_ifnames1, int len1, bool create);

/* Find Number of valid interfaces */
extern int wbd_count_interfaces(void);

/* ----------------------------- WBD shared Routines --------------------------------- */

#endif /* _WBD_RC_SHARED_H_ */
