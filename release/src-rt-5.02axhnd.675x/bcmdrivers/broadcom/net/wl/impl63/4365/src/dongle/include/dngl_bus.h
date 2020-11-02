/*
 * Bus independent DONGLE API external definitions
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: dngl_bus.h 735737 2017-12-12 07:16:33Z $
 */

#ifndef _dngl_bus_h_
#define _dngl_bus_h_

#include <typedefs.h>

#define BUS_DBG_BUFSZ 5000		/* debug scratch buffer size */

struct dngl_bus;

struct dngl_bus_ops {
	/* bind/unbind the bus to the device */
	void (*softreset)(struct dngl_bus *bus);
	int (*binddev)(void *bus, void *dev, uint numslaves);
	void (*rebinddev)(void *bus, void *new_dev, int ifindex);
	int (*unbinddev)(void *bus, void *dev);

	/* data flow */
#ifndef BCMUSBDEV_BMAC
	int (*tx)(struct dngl_bus *bus, void *p);
#else
	int (*tx)(struct dngl_bus *bus, void *p, uint32 ep_index);
#endif /* BCMUSBDEV_BMAC */
	void (*sendctl)(struct dngl_bus *bus, void *p);
	void (*rxflowcontrol)(struct dngl_bus *bus, bool state, int prio);
	uint32 (*iovar)(struct dngl_bus *bus, char *buf, uint32 inlen, uint32 *outlen, bool set);
	/* deprecated resume: just tx and let the bus handle resume */
	void (*resume)(struct dngl_bus *bus);
	void (*pr46794WAR)(struct dngl_bus *bus);

#ifdef BCMDBG
	/* debug/test/diagnostic routines */
	void (*dumpregs)(void);
	void (*loopback)(void);
	void (*xmit)(int len, int clen, bool ctl);
	uint (*msgbits)(uint *newbits);
#endif // endif
	int (*sendevent)(struct dngl_bus *bus, void *p);
	/* Control function for flowrings */
	int (*flowring_ctl)(void *bus, uint32 op, void *opdata);
#ifdef HOST_HDR_FETCH
	int (*txhdr_push)(void *dev, void *p, uint queue, bool commit);
	int (*reclaim_txpkts)(void *dev, struct spktq *pkt_list, uint16 fifo, bool free);
	void (*map_txpkts)(void *dev, map_pkts_cb_fn cb, void *ctx);
	void (*txhdr_commit)(void *dev);
#endif // endif
	int (*validatedev)(void *bus, void *dev);
	bool (*maxdevs_reached)(void *bus);
};

extern struct dngl_bus_ops *bus_ops;
extern struct dngl_bus_ops usbdev_bus_ops;
extern struct dngl_bus_ops sdpcmd_bus_ops;
extern struct dngl_bus_ops pciedngl_bus_ops;
extern struct dngl_bus_ops pciedev_bus_ops;
extern struct dngl_bus_ops m2md_bus_ops;
extern struct dngl_proto_ops_t *proto_ops;
extern struct dngl_proto_ops_t msgbuf_proto_ops;
extern struct dngl_proto_ops_t cdc_proto_ops;
#endif /* _dngl_bus_h_ */
