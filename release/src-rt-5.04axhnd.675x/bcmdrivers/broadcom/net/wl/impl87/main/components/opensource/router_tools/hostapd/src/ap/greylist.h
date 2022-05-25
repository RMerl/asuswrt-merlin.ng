/*
 * hostapd / RADIUS Greylist Access Control
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
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef GREYLIST_H
#define GREYLIST_H

#include "hostapd.h"

#define TXT_MAC_ADDR_LEN 18 /* Including ending '\0' */

extern char cmmac[];
void greylist_load(struct hapd_interfaces *interfaces);
int greylist_add(struct hostapd_data *hapd, const char *txtaddr, bool fromRadiusServer);
u8 greylist_get_client_snr(struct hostapd_data *hapd, const char *txtaddr);

#endif /* GREYLIST_H */
