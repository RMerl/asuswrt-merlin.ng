/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * bur_cfg_common.h
 *
 * common parts used over all B&R boards
 *
 * Copyright (C) 2016 Hannes Schmelzer <oe5hpm@oevsv.at> -
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 */

#ifndef __BUR_CFG_COMMON_H__
#define __BUR_CFG_COMMON_H__
/* ------------------------------------------------------------------------- */
#define BUR_COMMON_ENV \
"usbscript=usb start && fatload usb 0 ${scradr} usbscript.img &&" \
" source ${scradr}\0" \
"brdefaultip=if test -r ${ipaddr}; then; else" \
" setenv ipaddr 192.168.60.1; setenv serverip 192.168.60.254;" \
" setenv gatewayip 192.168.60.254; setenv netmask 255.255.255.0; fi;\0" \
"netconsole=echo switching to network console ...; " \
"if dhcp; then; else run brdefaultip; fi; setenv ncip ${serverip}; " \
"setcurs 1 9; lcdputs myip; setcurs 10 9; lcdputs ${ipaddr};" \
"setcurs 1 10;lcdputs serverip; setcurs 10 10; lcdputs ${serverip};" \
"setenv stdout nc;setenv stdin nc;setenv stderr nc\0"

/* Network console */

/* As stated above, the following choices are optional. */

#endif	/* __BUR_CFG_COMMON_H__ */
