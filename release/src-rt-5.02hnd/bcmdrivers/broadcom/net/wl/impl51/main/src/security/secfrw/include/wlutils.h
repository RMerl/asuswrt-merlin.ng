/*
 * Broadcom wireless network adapter utility functions
 *
 * Copyright (C) 2016, Broadcom. All Rights Reserved.
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
 * $Id: wlutils.h,v 1.1.1.1 2010-02-04 00:44:37 $
 */

#ifndef _wlutils_h_
#define _wlutils_h_

#include <typedefs.h>
#include <wlioctl.h>

/*
 * Pass a wlioctl request to the specified interface.
 * @param	name	interface name
 * @param	cmd	WLC_GET_MAGIC <= cmd < WLC_LAST
 * @param	buf	buffer for passing in and/or receiving data
 * @param	len	length of buf
 * @return	>= 0 if successful or < 0 otherwise
 */
extern int wl_ioctl(char *name, int cmd, void *buf, int len);

/*
 * Get device type.
 * @param	name	interface name
 * @param	buf	buffer for passing in and/or receiving data
 * @param	len	length of buf
 * @return	>= 0 if successful or < 0 otherwise
 */
#define DEV_TYPE_LEN 3	/* Length for dev type 'et'/'wl' */
extern int wl_get_dev_type(char *name, void *buf, int len);

/*
 * Get the MAC (hardware) address of the specified interface.
 * @param	name	interface name
 * @param	hwaddr	6-byte buffer for receiving address
 * @return	>= 0 if successful or < 0 otherwise
 */
extern int wl_hwaddr(char *name, unsigned char *hwaddr);

/*
 * Probe the specified interface.
 * @param	name	interface name
 * @return	>= 0 if a Broadcom wireless device or < 0 otherwise
 */
extern int wl_probe(char *name);

/*
 * Set/Get named variable.
 * @param	ifname		interface name
 * @param	iovar		variable name
 * @param	param		input param value/buffer
 * @param	paramlen	input param value/buffer length
 * @param	bufptr		io buffer
 * @param	buflen		io buffer length
 * @param	val		val or val pointer for int routines
 * @return	success == 0, failure != 0
 */
extern int wl_iovar_setbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr,
                           int buflen);
extern int wl_iovar_getbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr,
                           int buflen);
extern int wl_iovar_set(char *ifname, char *iovar, void *param, int paramlen);
extern int wl_iovar_get(char *ifname, char *iovar, void *bufptr, int buflen);
extern int wl_iovar_setint(char *ifname, char *iovar, int val);
extern int wl_iovar_getint(char *ifname, char *iovar, int *val);

/*
 * Set/Get named variable indexed by BSS Configuration
 * @param	ifname		interface name
 * @param	iovar		variable name
 * @param	bssidx		bsscfg index
 * @param	param		input param value/buffer
 * @param	paramlen	input param value/buffer length
 * @param	bufptr		io buffer
 * @param	buflen		io buffer length
 * @param	val		val or val pointer for int routines
 * @return	success == 0, failure != 0
 */
extern int wl_bssiovar_setbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen,
                              void *bufptr, int buflen);
extern int wl_bssiovar_getbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen,
                              void *bufptr, int buflen);
extern int wl_bssiovar_get(char *ifname, char *iovar, int bssidx, void *outbuf, int len);
extern int wl_bssiovar_set(char *ifname, char *iovar, int bssidx, void *param, int paramlen);
extern int wl_bssiovar_setint(char *ifname, char *iovar, int bssidx, int val);

#endif /* _wlutils_h_ */
