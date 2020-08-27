/*
 * Broadcom wireless network adapter utility functions
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
 * $Id: wlutils.h 779706 2019-10-04 13:58:11Z $
 */

#ifndef _wlutils_h_
#define _wlutils_h_

#include <shared.h>

#ifdef RTCONFIG_REALTEK
#include <wlioctl.h>
extern int wl_ioctl(char *name, int cmd, void *buf, int len);
#endif //RTCONFIG_REALTEK
#ifdef CONFIG_BCMWL5
#include <typedefs.h>
#ifdef RTCONFIG_HND_ROUTER_AX
#include <ethernet.h>
#else
#include <proto/ethernet.h>
#endif
#include <wlioctl.h>
#ifdef RTCONFIG_HND_ROUTER_AX
#include <bcmtlv.h>
#endif
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
 * @param	name	interface name
 * @param	var	variable name
 * @param	val	variable value/buffer
 * @param	len	variable value/buffer length
 * @return	success == 0, failure != 0
 */
extern int wl_set_val(char *name, char *var, void *val, int len);
extern int wl_get_val(char *name, char *var, void *val, int len);
extern int wl_set_int(char *name, char *var, int val);
extern int wl_get_int(char *name, char *var, int *val);

static inline int wl_phytype_n(int phytype)
{
	return (phytype == WLC_PHY_TYPE_N)
#ifdef WLC_PHY_TYPE_SSN
		|| (phytype == WLC_PHY_TYPE_SSN)
#endif
#ifdef WLC_PHY_TYPE_LCN
		|| (phytype == WLC_PHY_TYPE_LCN)
#endif
	;
}

#if 1
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
extern int wl_iovar_setbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen);
extern int wl_iovar_getbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen);
extern int wl_iovar_set(char *ifname, char *iovar, void *param, int paramlen);
extern int wl_iovar_get(char *ifname, char *iovar, void *bufptr, int buflen);
extern int wl_iovar_setint(char *ifname, char *iovar, int val);
extern int wl_iovar_getint(char *ifname, char *iovar, int *val);

#ifdef __CONFIG_DHDAP__
extern int dhd_iovar_setbuf(char *ifname, char *iovar, void *param, int paramlen,
		void *bufptr, int buflen);
extern int dhd_iovar_set(char *ifname, char *iovar, void *param, int paramlen);
extern int dhd_iovar_setint(char *ifname, char *iovar, int val);
#endif /* __CONFIG_DHDAP__ */

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
extern int wl_bssiovar_setbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen, void *bufptr, int buflen);
extern int wl_bssiovar_getbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen, void *bufptr, int buflen);
extern int wl_bssiovar_get(char *ifname, char *iovar, int bssidx, void *outbuf, int len);
extern int wl_bssiovar_set(char *ifname, char *iovar, int bssidx, void *param, int paramlen);
extern int wl_bssiovar_setint(char *ifname, char *iovar, int bssidx, int val);
#endif

extern char * wl_ether_etoa(const struct ether_addr *n);

/*
 * Probe the specified interface for its endianess.
 * @param	name	interface name
 * @return	>= 0 if successful or < 0 otherwise
 */
extern int wl_endian_probe(char *name);

#ifdef RTCONFIG_HND_ROUTER_AX
/*
 * Set HE related commands
 * @param      ifname          interface name
 * @param      iovar           variable name
 * @param      subcmd          he subcommand
 * @param      val             val or val pointer for int routines
 * @return     success == 0, failure != 0
 */

extern int wl_heiovar_setint(char *ifname, char *iovar, char *subcmd, int val);

/*
 * Set msched/umsched related commands
 * @param	ifname		interface name
 * @param	iovar		variable name
 * @param	subcmd		msched/umsched subcommand
 * @param	val		val or val pointer for int routines
 * @return	success == 0, failure != 0
 */
extern int wl_msched_iovar_setint(char *ifname, char *iovar, char *subcmd, int val);

/*
 * Set xtlv related iovar commands
 * @param      ifname          interface name
 * @param      iovar           variable name
 * @param      param           input parameter
 * @param      paramlen
 * @param      version         iovar version
 * @param      cmd_id
 * @param      xtlv_id
 * @param      xtlv_option     int/buf etc
 * @return     success == 0, failure != 0
 */
extern int wl_iovar_xtlv_set(char *ifname, char *iovar, uint8 *param, uint16 paramlen,
               uint16 version, uint16 cmd_id, uint16 xtlv_id, bcm_xtlv_opts_t opts);
/*
 * Set xtlv related iovar commands
 * @param      ifname          interface name
 * @param      iovar           variable name
 * @param      val             val or val pointer for int routines
 * @param      version         iovar version
 * @param      cmd_id
 * @param      xtlv_id
 * @return     success == 0, failure != 0
 */
extern int wl_iovar_xtlv_setint(char *ifname, char *iovar, int32 val, uint16 version,
               uint16 cmd_id, uint16 xtlv_id);
/*
 * Set xtlv buf related iovar commands
 * @param      ifname          interface name
 * @param      iovar           variable name
 * @param      param           input data
 * @param      paramlen        data len
 * @param      version         iovar version
 * @param      cmd_id
 * @param      xtlv_id
 * @param      xtlv_option     int/buf etc
 * @return     success == 0, failure != 0
 */
extern int wl_iovar_xtlv_setbuf(char *ifname, char *iovar, uint8 *param, uint16 paramlen,
               uint16 version, uint16 cmd_id, uint16 xtlv_id, bcm_xtlv_opts_t opts,
               uint8 *buf, uint16 buflen);
#endif

#ifdef __CONFIG_DHDAP__
extern int dhd_probe(char *name);
#endif
#endif /* CONFIG_BCMWL5 */
#endif /* _wlutils_h_ */
