
/*
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
 * 
 * This source code was modified by Broadcom. It is distributed under the
 * original license terms described below.
 *
 * Copyright (c) 2002-2006 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
/*
 *
 * $Id: wl_mdns.h 467328 2014-04-03 01:23:40Z $
 *
 * Define interface into mdns from wl driver
 *
 */

#ifndef _wl_mdns_h_
#define _wl_mdns_h_

/* This file defines interface to wl driver integration */
extern wlc_dngl_ol_mdns_info_t * wlc_dngl_ol_mdns_attach(wlc_dngl_ol_info_t *wlc_dngl_ol);
extern void wl_mdns_detach(wlc_dngl_ol_mdns_info_t *mdnsi);
extern void wl_mDNS_Init(wlc_dngl_ol_mdns_info_t *mdnsi, uint8 *dbase, uint32 dbase_len);
extern void wl_mDNS_event_handler(wlc_dngl_ol_mdns_info_t *mdnsi, uint32 event, void *event_data);
extern uint32 mdns_rx(wlc_dngl_ol_mdns_info_t *mdnsi, void *pkt, uint16 len);
#endif /* _wl_mdns_h_ */
