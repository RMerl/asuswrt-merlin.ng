/* SPDX-License-Identifier: GPL-2.0 */
/*
 *	Copied from Linux Monitor (LiMon) - Networking.
 *
 *	Copyright 1994 - 2000 Neil Russell.
 *	(See License)
 *	Copyright 2000 Roland Borde
 *	Copyright 2000 Paolo Scaffardi
 *	Copyright 2000-2002 Wolfgang Denk, wd@denx.de
 */

#if defined(CONFIG_CMD_CDP)

#ifndef __CDP_H__
#define __CDP_H__

void cdp_start(void);
/* Process a received CDP packet */
void cdp_receive(const uchar *pkt, unsigned len);

#endif /* __CDP_H__ */
#endif
