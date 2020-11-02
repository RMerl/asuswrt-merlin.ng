/*
 * RTE dongle core support file
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
 * $Id: dngl_rte_rsock.c 497491 2014-08-19 18:24:00Z $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <bcmutils.h>
#include <siutils.h>
#include <proto/ethernet.h>
#include <trxhdr.h>
#include <bcmnvram.h>

#include <osl.h>
#include <wlioctl.h>
#include <bcmendian.h>
#include <epivers.h>
#include <sflash.h>
#include <sbchipc.h>
#include <dngl_stats.h>
#ifdef BCMET
#include <etioctl.h>
#endif // endif

#include <dngl_bus.h>
#include <dngl_api.h>
#include <dngl_protocol.h>
#include <dngl_dbg.h>
#include <dngl_rte.h>
#include <rte_dev.h>
#include <rte_ioctl.h>

#include <rsock/types.h>

#include "rserv_rte.h"
#include "rserv.h"
#include "rserv_if.h"

static uint8 found = 0;

int dngl_msglevel = DNGL_ERROR;

struct dngl_bus_ops *bus_ops;
extern struct dngl_proto_ops_t cdc_proto_ops;

/* Call Broadcom wireless driver private ioctl */
int
_dngl_devioctl(struct dngl *dngl, int ifindex,
	uint32 cmd, void *buf, int len, int *used, int *needed, bool set)
{
	int ret = -1;

	ASSERT(dngl->primary_slave);
	/* buf must be 4-byte aligned */
	ASSERT(ISALIGNED(buf, 4));

	trace("ioctl %s cmd 0x%x, len %d", set ? "SET" : "QUERY", cmd, len);
	ret = dngl->primary_slave->ops->ioctl
		(dngl->primary_slave, cmd, buf, len, used, needed, set);
	trace("status = %d/0x%x", ret, ret);

	return ret;
}

struct dngl *
BCMINITFN(dngl_attach)(struct dngl_bus *bus, void *drv, si_t *sih, osl_t *osh)
{
	struct dngl *dngl = NULL;

	trace("called");

	if (found >= 8) {
		err("too many units");
		goto fail;
	}

	if (!(dngl = MALLOC(osh, sizeof(dngl_t)))) {
		err("MALLOC failed");
		goto fail;
	}
	memset(dngl, 0, sizeof(dngl_t));
	dngl->bus = bus;
	dngl->osh = osh;
	dngl->sih = sih;
	dngl->unit = found;

	found++;

	return dngl;

fail:
	if (dngl)
		if (MFREE(NULL, dngl, sizeof(dngl_t)))
			err("MFREE error");
	return NULL;
}

void
BCMATTACHFN(dngl_detach)(struct dngl *dngl)
{
	trace("%s", dngl->rtedev->name);

	/* unbind from enslaved device */
	if (dngl->primary_slave) {
#ifdef BCMET
		int pcb;
		uint32 cmd;
		cmd = OID_ET_UNSETCALLBACK;
		pcb = (int) &dngl->cb;
		if (dngl_dev_ioctl(dngl, cmd, &pcb, sizeof(pcb)) < 0)
			err("%s: UNSET CALLBACK failed", dngl->rtedev->name);
#endif // endif
		dngl->primary_slave->chained = NULL;
		dngl->rtedev->chained = NULL;
	}

	cdc_proto_ops.proto_detach_fn(dngl->proto);
}

/* Net interface running faster than USB. Flow-control the net interface */
void
dngl_txstop(struct dngl *dngl)
{
	trace("%s", dngl->rtedev->name);
	/* overflow case just drops packets at the USB interface */
}

void
dngl_txstart(struct dngl *dngl)
{
	dbg("%s", dngl->rtedev->name);
	/* overflow case just drops packets at the USB interface */
}

/* flow control called by dev receiving packets from us */
void
dngl_rxflowcontrol(struct dngl *dngl, bool state, int prio)
{
	trace("%s flowctl %s", dngl->rtedev->name, state == ON ? "on" : "off");
	bus_ops->rxflowcontrol(dngl->bus, state, prio);
}

/* Transmit an rserv output packet onto the bus */
int
dngl_sendbus(struct dngl *dngl, void *p)
{
	trace("%s", dngl->rtedev->name);

#ifdef BCMDBG
	prpkt("send usb", dngl->osh, p);
#endif // endif

	if (bus_ops->tx(dngl->bus, p)) {
		dngl->stats.tx_bytes += pkttotlen(dngl->osh, p);
		dngl->stats.tx_packets++;
	} else {
		dngl->stats.tx_dropped++;
		err("dropped pkt");
	}
	return 0;
}

/* Pass a received wireless packet into rserv interface */
int
dngl_sendpkt(struct dngl *dngl, void *src, void *p)
{
#ifdef BCMDBG
	prpkt("dngl_sendpkt", dngl->osh, p);
#endif // endif

	trace("%s", dngl->rtedev->name);

	rserv_if_input(p);
	return 0;
}

/* Transmit a stack packet onto the bus */
int
dngl_sendctl(struct dngl *dngl, void *src, void *p)
{
	return dngl_sendpkt(dngl->bus, src, p);
}

/*
 * Queue a packet for transmission to the slave
 */
int
dngl_sendslave(struct dngl *dngl, void *p)
{
	struct lbuf *lb;

#ifdef BCMDBG
	prpkt("dngl_sendslave", dngl->osh, p);
#endif // endif

	trace("%s: pkt len %d", dngl->rtedev->name, PKTLEN(dngl->osh, p));

	lb = PKTTONATIVE(dngl->osh, p);
	if (dngl->primary_slave->ops->xmit(dngl->rtedev, dngl->primary_slave, lb)) {
		dngl->stats.rx_dropped++;
		err("dropped pkt 0x%p; len %d for wl", lb, PKTLEN(dngl->osh, lb));
		return 1;
	} else {
		dngl->stats.rx_packets++;
		return 0;
	}
}

/*
 * Send a packet received from the bus into rserv.
 */
void
dngl_sendup(struct dngl *dngl, void *p)
{
	trace("%s: pkt len %d", dngl->rtedev->name, PKTLEN(dngl->osh, p));

#ifdef BCMDBG
	prpkt("rserv_input", dngl->osh, p);
#endif // endif

	rserv_input(p);
}

void
dngl_ctrldispatch(struct dngl *dngl, void *p, uchar *ext_buf)
{
	cdc_proto_ops.proto_ctrldispatch_fn(dngl->proto, p, ext_buf);
}

void
dngl_resume(struct dngl *dngl)
{
	trace("%s", dngl->rtedev->name);

	/* Do nothing */
}

void
dngl_suspend(struct dngl *dngl)
{
	trace("%s", dngl->rtedev->name);
}

/* bind/enslave to the wireless device */
int
BCMINITFN(dngl_binddev)(struct dngl *dngl, void *bus, void *dev, uint numslaves)
{
	struct ether_addr macaddr;
	const char *s;

	trace("called");

	if (bus == NULL || dev == NULL) {
		err("bus and/or dev is NULL");
		return -1;
	}

	/* set dev as the primary chained device. */
	if (dngl->rtedev == NULL && dngl->primary_slave == NULL) {
		dngl->rtedev = bus;
		dngl->primary_slave = dev;
		((hnd_dev_t *)bus)->chained = dev;
		((hnd_dev_t *)dev)->chained = bus;
	}
	else {
		err("dngl %p %p bus %p dev %p inconsistent",
			dngl->rtedev, dngl->primary_slave, bus, dev);
		return -1;
	}

	trace("  call proto_attach");
	if (!(dngl->proto = cdc_proto_ops.proto_attach_fn(dngl->osh, dngl, dngl->bus,
	                                 ((hnd_dev_t *)bus)->name, FALSE))) {
		err("proto_attach failed");
		return -1;
	}

	dbg("%s: %s Network Adapter (%s)", ((hnd_dev_t *)bus)->name,
	    getvar(NULL, "manf") ? : "Broadcom", ((hnd_dev_t *)dev)->name : "P-t-P");

	/* Initialize TCP/IP remote sockets server */

	s = nvram_get("il0macaddr");
	ASSERT(s);

	printf("TCP/IP Offload: MAC address is %s\n", s);

	bcm_ether_atoe(s, &macaddr);
	rserv_rte_init(dngl, &macaddr);

	return 0;
}

/* unbind the wireless device */
int
dngl_unbinddev(struct dngl *dngl, void *bus, void *dev)
{
	ASSERT(dngl->rtedev == bus);
	ASSERT(dngl->primary_slave == dev);
	ASSERT((hnd_dev_t *)bus->chained == dev);
	ASSERT((hnd_dev_t *)dev->chained == bus);

	((hnd_dev_t *)bus)->chained = NULL;
	((hnd_dev_t *)dev)->chained = NULL;
	dngl->rtedev = NULL;
	dngl->primary_slave = NULL;

	return 0;
}

int
dngl_opendev(struct dngl *dngl)
{
	trace("%s", dngl->rtedev->name);
	if (dngl->devopen)
		return 0;

	ASSERT(dngl->primary_slave);
	dngl->primary_slave->ops->open(dngl->primary_slave);
	dngl->devopen = TRUE;

	return 0;
}

/* Get device stats */
void
dngl_get_stats(struct dngl *dngl, dngl_stats_t *stats)
{
	trace("%s", dngl->rtedev->name);
	/* dngl_stats_t happens to mirror first 8 ulongs in linux net_device_stats */
	bcopy(&dngl->stats, stats, sizeof(dngl->stats));
}

/* Get enslaved net interface stats */
bool
dngl_get_netif_stats(struct dngl *dngl, dngl_stats_t *stats)
{
	int ret;

	trace("%s", dngl->rtedev->name);
	bzero(stats, sizeof(dngl_stats_t));
	/* dngl_stats_t happens to mirror first 8 ulongs in linux net_device_stats */
	if ((ret = dngl_dev_ioctl(dngl, RTEGSTATS, stats, sizeof(dngl_stats_t))) < 0) {
		err("%s: error reading slave addr: %d", dngl->rtedev->name, ret);
		return TRUE;
	} else
		return FALSE;
}

ulong
dngl_get_netif_mtu(struct dngl *dngl)
{
	uint32 val = 0;
	int ret;

	trace("%s", dngl->rtedev->name);
	if ((ret = dngl_dev_ioctl(dngl, RTEGMTU, &val, sizeof(val))) < 0)
		err("%s: error reading slave MTU: %d", dngl->rtedev->name, ret);
	return val;
}

#ifdef FLASH_UPGRADE
int
dngl_upgrade(struct dngl *dngl, uchar *buf, uint len)
{
	uint32 offset;
	uint origidx;
	chipcregs_t *ccregs;
	int ret = 0;
	static int upgrade_size;
	static uint32 upgrade_crc, trx_crc;

	if (len < 4)
		return BCME_BADLEN;

	/* First word of each chunk is the offset */
	offset = ltoh_ua((uint32 *) buf);
	buf += 4;
	len -= 4;

	if (offset == 0) {
		struct trx_header trx;

		/* Examine TRX header */
		if (len < sizeof(struct trx_header)) {
			printf("%s: dngl_upgrade: File is too small (%d bytes)\n",
			       dngl->rtedev->name, len);
			return BCME_BADLEN;
		}

		/* Avoid alignment issues */
		memcpy(&trx, buf, sizeof(struct trx_header));
		if (ltoh32(trx.magic) != TRX_MAGIC ||
		    ltoh32(trx.len) > TRX_MAX_LEN ||
		    ltoh32(trx.len) < sizeof(struct trx_header)) {
			printf("%s: dngl_upgrade: Bad trx header\n", dngl->rtedev->name);
			return BCME_ERROR;
		}
		upgrade_crc = hndcrc32((uint8 *) &((struct trx_header *) buf)->flag_version,
		                       len - OFFSETOF(struct trx_header, flag_version),
		                       CRC32_INIT_VALUE);
		trx_crc = trx.crc32;
		upgrade_size = ltoh32(trx.len) - sizeof(struct trx_header);
		len -= sizeof(struct trx_header);
		buf += sizeof(struct trx_header);
	} else {
		offset -= sizeof(struct trx_header);
		upgrade_crc = hndcrc32((uint8 *) buf, len, upgrade_crc);
	}

	origidx = si_coreidx(dngl->sih);

	ccregs = (chipcregs_t *)si_setcoreidx(dngl->sih, SI_CC_IDX);
	if (!ccregs) {
		printf("%s: dngl_upgrade: bad ccregs\n", dngl->rtedev->name);
		ret = -1;
		goto end;
	}

	if (offset == 0) {
		/* Initialize serial flash access (only support serial flash for now) */
		if (!sflash_init(dngl->sih, ccregs)) {
			printf("%s: *** dngl_upgrade: sflash_init failed\n",
			       dngl->rtedev->name);
			ret = -2;
			goto end;
		}
		dngl->upgrade_status = WLC_UPGRADE_PENDING;
	}

	if (len && (sflash_commit(dngl->sih, ccregs, offset, len, buf) != 0)) {
		printf("%s: *** dngl_upgrade: sflash_commit failed\n", dngl->rtedev->name);
		ret = -3;
		goto end;
	}

	trace("%s: dngl_upgrade: wrote %d bytes to offset %d",
	      dngl->rtedev->name, len, offset);

	/* check if done */
	if ((upgrade_size -= len) == 0) {
		if (upgrade_crc != trx_crc) {
			printf("%s: *** dngl_upgrade: Bad CRC\n", dngl->rtedev->name);
			ret = -4;
		} else {
			dngl->upgrade_status = WLC_UPGRADE_SUCCESS;
			printf("%s: *** dngl_upgrade: upgrade success\n",
			       dngl->rtedev->name);
		}
	}

end:
	/* restore core index */
	si_setcoreidx(dngl->sih, origidx);
	if (ret != 0)
		dngl->upgrade_status = ret;
	return ret;
}

int
dngl_upgrade_status(struct dngl *dngl)
{
	return dngl->upgrade_status;
}
#endif /* FLASH_UPGRADE */

#ifdef BCMET
static void
dngl_et_event(void *context, int link)
{
	struct dngl *dngl = (struct dngl *) context;

	trace("%s", dngl->rtedev->name);
	cdc_proto_ops.proto_dev_event_fn(dngl->proto, (void *) &link);
}
#endif /* BCMET */

#ifdef BCMDBG
static void
dngl_dump_busregs(void)
{
	bus_ops->dumpregs();
}

static void
dngl_reboot(void *arg)
{
	dngl_schedule_work(arg, NULL, _dngl_reboot, 200);
}

static void
dngl_bus_loopback(void)
{
	bus_ops->loopback();
}

static void
dngl_bus_xmit(uint32 arg, uint argc, char *argv[])
{
	int len = -1;
	int clen = 0;
	bool ctl = FALSE;
	uint argnum = 1;

	if (argnum < argc) {
		ctl = !strcmp(argv[argnum], "-c");
		if (ctl)
			argnum++;
	}

	if (argnum < argc)
		len = atoi(argv[argnum++]);

	if (argnum < argc)
		clen = atoi(argv[argnum++]);

	bus_ops->xmit(len, clen, ctl);
}

static void
dngl_bus_msgbits(uint32 arg, uint argc, char *argv[])
{
	uint bits, newbits;

	if (argc > 1) {
		newbits = bcm_strtoul(argv[1], NULL, 16);
		bits = bus_ops->msgbits(&newbits);
	} else {
		bits = bus_ops->msgbits(NULL);
	}

	printf("Message bits: 0x%0x\n", bits);
}
#endif /* BCMDBG */

void
dngl_init(struct dngl *dngl)
{
	err("%s", dngl->rtedev->name);

	/* check if init called w/o previous halt */
	if (dngl->up)
		return;

	/* Open slave device */
	if (dngl->primary_slave) {
		int magic;
		int err = 0;

		err("%s: open slave", dngl->rtedev->name);
		dngl_opendev(dngl);
		err("%s: after open slave", dngl->rtedev->name);

		if (err)
			printf("error: device open failed\n");

		if (dngl_dev_ioctl(dngl, WLC_GET_MAGIC, &magic, sizeof(magic)) >= 0 &&
		    magic == WLC_IOCTL_MAGIC) {
			dngl->medium = DNGL_MEDIUM_WIRELESS;
		}

#ifdef BCMET
		if (!dngl->cb.fn) {
			uint32 cmd;
			int pcb;
			uint32 Status = 0;
			/* Handle all events */
			dngl->cb.fn = dngl_et_event;
			cmd = OID_ET_SETCALLBACK;
			dngl->cb.context = dngl;
			/* Register event handler */
			pcb = (int) &dngl->cb;
			if (dngl_dev_ioctl(dngl, cmd, &pcb, sizeof(pcb)) < 0) {
				err("%s: SET CALLBACK failed", dngl->rtedev->name);
				Status = BCME_ERROR;
				return;
			}
		}
#endif /* BCMET */
	}
#ifdef BCMDBG
	hnd_cons_add_cmd("msg", dngl_bus_msgbits, 0);
	hnd_cons_add_cmd("br", dngl_dump_busregs, 0);
	hnd_cons_add_cmd("lb", dngl_bus_loopback, 0);
	hnd_cons_add_cmd("tx", dngl_bus_xmit, 0);
	hnd_cons_add_cmd("reboot", dngl_reboot, 0);
#endif /* BCMDBG */
	dngl->up = TRUE;
}

void
dngl_halt(struct dngl *dngl)
{
	trace("%s", dngl->rtedev->name);

	dngl->up = FALSE;

	/* Unregister event handler */
	if (dngl->primary_slave) {
		if (dngl->medium == DNGL_MEDIUM_WIRELESS) {
			uchar etheraddr[ETHER_ADDR_LEN];
			int len;

#ifdef BCMRECLAIM
			if (dngl_dev_ioctl(dngl, WLC_DISASSOC, NULL, 0) < 0)
				err("%s: WLC_DISASSOC failed", dngl->rtedev->name);
#else
			if (dngl_dev_ioctl(dngl, WLC_DOWN, NULL, 0) < 0)
				err("%s: WLC_DOWN failed", dngl->rtedev->name);
#endif // endif
			/* WHQL: restore permanent ether addr on halt */
			len = ETHER_ADDR_LEN;
			if (dngl_dev_ioctl(dngl, RTEGPERMADDR, etheraddr, len) < 0)
				err("%s: RTEGPERMADDR failed", dngl->rtedev->name);
			else
				dngl_dev_ioctl(dngl, RTESHWADDR, etheraddr, len);
		}
	}
}

void
dngl_reset(struct dngl *dngl)
{
	trace("%s", dngl->rtedev->name);

	/* purge any stale ctrl & intr packets */
	bus_ops->softreset(dngl->bus);
}

void
_dngl_reboot(dngl_task_t *task)
{
	struct dngl *dngl = (struct dngl *) hnd_timer_get_ctx(task);

	si_watchdog(dngl->sih, 1);
}

void *
dngl_proto(struct dngl *dngl)
{
	return dngl->proto;
}

void
dngl_keepalive(struct dngl *dngl, uint32 msec)
{
	/* set the watchdog for # msec */
	si_watchdog_ms(dngl->sih, msec);
}
