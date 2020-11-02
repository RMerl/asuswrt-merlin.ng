/*
 * RTE layer for pcie device
 * hnd_dev_ops_t & dngl_bus_ops for pciedev are defined here
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
 * $Id: pciedev_rte.c $
 */

#include <osl.h>
#include <osl_ext.h>
#include <bcmdefs.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <dngl_bus.h>
#include <dngl_api.h>
#include <circularbuf.h>
#include <bcmpcie.h>
#include <bcmmsgbuf.h>
#include <pciedev.h>
#include <pciedev_dbg.h>
#include <hndsoc.h>
#include <wlfc_proto.h>
#include <flring_fc.h>
#include <event_log.h>
#include <rte_dev.h>
#include <rte_isr.h>
#include <rte_timer.h>
#include <rte_ioctl.h>
#include <rte_gpio.h>

typedef struct {
	int unit;
	hnd_dev_t *rtedev;
	struct dngl_bus *pciedev;
	osl_t *osh;
	void *regs;
	uint16 device;
	struct dngl *dngl;
	hnd_timer_t *dpcTimer;	/* 0 delay timer used to schedule dpc */
} drv_t;

/* Reclaimable strings */
static const char BCMATTACHDATA(rstr_fmt_banner)[] = "%s: Broadcom PCIE MSGBUF driver\n";
static const char BCMATTACHDATA(rstr_fmt_devname)[] = "pciemsgbuf%d";

/* Driver entry points */
static void *pciedev_probe(hnd_dev_t *dev, void *regs, uint bus, uint16 device,
                          uint coreid, uint unit);
static void pciedev_isr(void *cbdata);

#ifdef THREAD_SUPPORT
static void pciedev_dpc_thread(void *cbdata);
#endif	/* THREAD_SUPPORT */

static void pciedev_run(hnd_dev_t *ctx);

#ifdef PCIE_PHANTOM_DEV
static void sd_dma_isr(hnd_dev_t *rtedev);
static void usb_dma_isr(hnd_dev_t *rtedev);
#endif /* PCIE_PHANTOM_DEV */

static int pciedev_open(hnd_dev_t *dev);
static void _pciedev_dpctask(hnd_timer_t *timer);
static int pciedev_send(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb);
static void pciedev_txflowcontrol(hnd_dev_t *dev, bool state, int prio);
static int pciedev_close(hnd_dev_t *dev);
static int pciedev_ioctl(hnd_dev_t *dev, uint32 cmd, void *buf, int len,
	int *used, int *needed, int set);
#ifdef HOST_HDR_FETCH
static int pciedev_txhdr_push(void *dev, void *p, uint queue, bool commit);
static int pciedev_reclaim_txpkts(void *dev, struct spktq *pkt_list, uint16 fifo, bool free);
static void pciedev_map_txpkts(void *dev, map_pkts_cb_fn cb, void *ctx);
static void pciedev_txhdr_commit(void *dev);
#endif /* HOST_HDR_FETCH */

static hnd_dev_ops_t pciedev_funcs = {
	probe:		pciedev_probe,
	open:		pciedev_open,
	close:		pciedev_close,
	xmit:		pciedev_send,
	ioctl:		pciedev_ioctl,
	txflowcontrol:	pciedev_txflowcontrol,
	poll:		pciedev_run
};

struct dngl_bus_ops pciedev_bus_ops = {
	rebinddev:	pciedev_bus_rebinddev,
	binddev:	pciedev_bus_binddev,
	sendctl:	pciedev_bus_sendctl,
	iovar:		pciedev_bus_iovar,
	unbinddev:	pciedev_bus_unbinddev,
	tx:		pciedev_create_d2h_messages_tx,
	flowring_ctl:	pciedev_bus_flring_ctrl,
#ifdef HOST_HDR_FETCH
	txhdr_push:	pciedev_txhdr_push,
	reclaim_txpkts: pciedev_reclaim_txpkts,
	map_txpkts:	pciedev_map_txpkts,
	txhdr_commit:	pciedev_txhdr_commit,
#endif /* HOST_HDR_FETCH */
	validatedev:	pciedev_bus_validatedev,
	maxdevs_reached:	pciedev_bus_maxdevs_reached
};

hnd_dev_t pciedev_dev = {
	name:		"pciedev",
	ops:		&pciedev_funcs
};

/** Number of devices found */
static int found = 0;
int pci_msg_level = PCI_ERROR_VAL;

/** thread-safe interrupt enable */
static void
_pciedev_intrson(drv_t *drv)
{
#ifdef THREAD_SUPPORT
	/* critical section */
	osl_ext_interrupt_state_t state = osl_ext_interrupt_disable();
#endif	/* THREAD_SUPPORT */

	pciedev_intrson(drv->pciedev);

#ifdef THREAD_SUPPORT
	/* critical section */
	osl_ext_interrupt_restore(state);
#endif	/* THREAD_SUPPORT */
}

static const char BCMATTACHDATA(rstr_pciedev_probe_malloc_failed)[] =
	"pciedev_probe: malloc failed\n";

/** probe function for pcie device */
static void *
BCMATTACHFN(pciedev_probe)(hnd_dev_t *rtedev, void *regs, uint bus, uint16 device,
                        uint coreid, uint unit)
{
	drv_t *drv;
	osl_t *osh;

	PCI_TRACE(("pciedev_probe\n"));

	if (found >= 8) {
		PCI_ERROR(("pciedev_probe: too many units\n"));
		goto fail;
	}
#ifdef SHARED_OSL_CMN
	osh = osl_attach(rtedev, NULL);
#else
	osh = osl_attach(rtedev);
#endif /* SHARED_OSL_CMN */
	if (!(drv = (drv_t *)MALLOC(osh, sizeof(drv_t)))) {
		printf(rstr_pciedev_probe_malloc_failed);
		goto fail;
	}

	bzero(drv, sizeof(drv_t));

	drv->unit = found;
	drv->rtedev = rtedev;
	drv->device = device;
	drv->regs = regs;

	drv->osh = osh;

	/* Allocate chip state */
	if (!(drv->pciedev = pciedev_attach(drv, VENDOR_BROADCOM, device, osh, regs, bus))) {
		PCI_ERROR(("pciedev_probe: pciedev_attach failed\n"));
		goto fail;
	}
#ifndef RTE_POLL
	/* PCIE Mailbox ISR */
	if (hnd_isr_register(0, coreid, unit, pciedev_isr, rtedev, bus) ||
#ifdef THREAD_SUPPORT
	    hnd_dpc_register(0, coreid, unit, pciedev_dpc_thread, rtedev, bus) ||
#endif	/* THREAD_SUPPORT */
	    FALSE) {
		PCI_ERROR(("pciedev_probe: hnd_isr_register failed\n"));
		pciedev_detach(drv->pciedev);
		goto fail;
	}
#ifdef PCIE_PHANTOM_DEV
	/* SD DAM ISR */
	if (hnd_isr_register(0, SDIOD_CORE_ID, unit, (isr_fun_t)sd_dma_isr, rtedev, bus)) {
		PCI_ERROR(("pciedev_probe: hnd_isr_register failed for sd_dma\n"));
		pciedev_detach(drv->pciedev);
		goto fail;
	}
	/* USB DMA ISR */
	if (hnd_isr_register(0, USB20D_CORE_ID, unit, (isr_fun_t)usb_dma_isr, rtedev, bus)) {
		PCI_ERROR(("pciedev_probe: hnd_isr_register failed for sd_dma\n"));
		pciedev_detach(drv->pciedev);
		goto fail;
	}
#endif /* PCIE_PHANTOM_DEV */
#endif	/* !RTE_POLL */
	drv->dngl = pciedev_dngl(drv->pciedev);

	sprintf(rtedev->name, rstr_fmt_devname, found);
	printf(rstr_fmt_banner, rtedev->name);

	found++;

	return (void*)drv;

fail:
	return NULL;
} /* pciedev_probe */

void
pciedev_bus_rebinddev(void *bus, void *dev, int ifindex)
{
	drv_t *drv = ((hnd_dev_t *)bus)->softc;
	dngl_rebinddev(drv->dngl, bus, dev, ifindex);
}

int
pciedev_bus_binddev(void *bus, void *dev, uint numslaves)
{
	drv_t *drv = ((hnd_dev_t *)bus)->softc;
	return dngl_binddev(drv->dngl, bus, dev, numslaves);
}

int
pciedev_bus_unbinddev(void *bus, void *dev)
{
	drv_t *drv = ((hnd_dev_t *)bus)->softc;
	return dngl_unbinddev(drv->dngl, bus, dev);
}

int
pciedev_bus_validatedev(void *bus, void *dev)
{
	drv_t *drv = ((hnd_dev_t *)bus)->softc;
	return dngl_validatedev(drv->dngl, bus, dev);
}

bool
pciedev_bus_maxdevs_reached(void *bus)
{
	drv_t *drv = ((hnd_dev_t *)bus)->softc;
	return dngl_maxdevs_reached(drv->dngl);
}

static int
BCMATTACHFN(pciedev_open)(hnd_dev_t *dev)
{
	drv_t *drv = dev->softc;

	PCI_TRACE(("pciedev_open: %s\n", dev->name));

#ifdef RSOCK
	/* init the dongle state */
	dngl_init(drv->dngl);
#endif // endif

	drv->dpcTimer = hnd_timer_create(NULL, drv, _pciedev_dpctask, NULL);
	if (drv->dpcTimer == NULL)
		return BCME_NORESOURCE;

	/* Initialize chip */
	pciedev_init(drv->pciedev);

	return 0;
}

/** dpc for pcie core */
static void
_pciedev_dpc(drv_t *drv)
{
	if (pciedev_dpc(drv->pciedev)) {
		if (!hnd_timer_start(drv->dpcTimer, 0, FALSE))
			ASSERT(FALSE);
	/* re-enable interrupts */
	} else {
		_pciedev_intrson(drv);
	}
}

static void
_pciedev_dpctask(hnd_timer_t *timer)
{
	drv_t *drv = (drv_t *)hnd_timer_get_data(timer);

	pciedev_intrsupd(drv->pciedev);
	_pciedev_dpc(drv);
}

#ifdef PCIE_PHANTOM_DEV

/** dpc functions for usb and sdio core */
static void
sd_dma_isr(hnd_dev_t *rtedev)
{
	drv_t *drv = rtedev->softc;

	ASSERT(drv->pciedev);
	pcie_phtm_sd_isr(drv->pciedev);
}

static void
usb_dma_isr(hnd_dev_t *rtedev)
{
	drv_t *drv = rtedev->softc;

	pcie_phtm_usb_isr(drv->pciedev);
}
#endif /* PCIE_PHANTOM_DEV */

/** ISR for pcie interrupt */
static void
pciedev_isr(void *cbdata)
{
	hnd_dev_t *rtedev = cbdata;
#ifdef THREAD_SUPPORT
	drv_t *drv = rtedev->softc;

	/* deassert interrupt */
	pciedev_intrsoff(drv->pciedev);
#else
	pciedev_run(rtedev);
#endif	/* THREAD_SUPPORT */
}

#ifdef THREAD_SUPPORT
static void
pciedev_dpc_thread(void *cbdata)
{
	hnd_dev_t *rtedev = cbdata;

	pciedev_run(rtedev);
}
#endif	/* THREAD_SUPPORT */

static void
pciedev_run(hnd_dev_t *rtedev)
{
	drv_t *drv = rtedev->softc;
	ASSERT(drv->pciedev);
	PCI_TRACE(("pciedev_run is called\n"));
#ifndef THREAD_SUPPORT
	pciedev_intrsoff(drv->pciedev);
#endif	/* THREAD_SUPPORT */
	/* call common first level interrupt handler */
	if (pciedev_dispatch(drv->pciedev)) {
		/* if more to do... */
		_pciedev_dpc(drv);
	} else {
		/* isr turned off interrupts */
		_pciedev_intrson(drv);
	}
}

/** close pcie device */
int
BCMATTACHFN(pciedev_close)(hnd_dev_t *dev)
{
#ifndef BCMNODOWN
	drv_t *drv = dev->softc;

	PCI_TRACE(("pciedev_close: drv exit%d\n", drv->unit));
	if (drv->dpcTimer != NULL)
		hnd_timer_free(drv->dpcTimer);
	pciedev_detach(drv->pciedev);
	osl_detach(drv->osh);
#endif /* BCMNODOWN */

	return 0;
}

/** Forwards a packet towards the host */
static int
pciedev_send(hnd_dev_t *src, hnd_dev_t *dev, struct lbuf *lb)
{
	drv_t *drv = dev->softc;
#ifdef PKTC_FDAP
	void *n;
	struct dngl *dngl = drv->dngl;

	FOREACH_CHAINED_PKT(lb, n) {
		PKTCLRCHAINED(drv->osh, lb);
		PKTCCLRFLAGS(lb);
		dngl_sendpkt((void *)dngl, src, (void *)lb);
	}
	return 0;
#else
	return dngl_sendpkt((void *)(drv->dngl), src, (void *)lb);
#endif // endif
}

static void
pciedev_txflowcontrol(hnd_dev_t *dev, bool state, int prio)
{
	return;
}

/**
 * PROP_TXSTATUS specific function. Called when the WL layer wants to report a flow control related
 * event (eg MAC_OPEN), this function consumes (terminates) those events, which is a difference
 * compared to eg USB dongles, in which case the host instead of firmware terminates the events.
 */
int pciedev_bus_flring_ctrl(void *dev, uint32 op, void *data)
{
	if (dev != NULL && data != NULL) {
		drv_t *drv = ((hnd_dev_t *)dev)->softc;
		struct dngl_bus *pciedev = (struct dngl_bus *)drv->pciedev;
		flowring_op_data_t	*op_data = (flowring_op_data_t *)data;

		switch (op) {
		case WLFC_CTL_TYPE_MAC_OPEN:
		case WLFC_CTL_TYPE_MAC_CLOSE:
			pciedev_upd_flr_port_handle(pciedev, op_data->handle,
				(op == WLFC_CTL_TYPE_MAC_OPEN));
			break;

		case WLFC_CTL_TYPE_MACDESC_ADD:
		case WLFC_CTL_TYPE_MACDESC_DEL:
			pciedev_upd_flr_hanlde_map(pciedev, op_data->handle, op_data->ifindex,
				(op == WLFC_CTL_TYPE_MACDESC_ADD), op_data->addr);
			break;

		case WLFC_CTL_TYPE_INTERFACE_OPEN:
		case WLFC_CTL_TYPE_INTERFACE_CLOSE:
			pciedev_upd_flr_if_state(pciedev, op_data->ifindex,
				(op == WLFC_CTL_TYPE_INTERFACE_OPEN));
			break;

		case WLFC_CTL_TYPE_TID_OPEN:
		case WLFC_CTL_TYPE_TID_CLOSE:
			pciedev_upd_flr_tid_state(pciedev, op_data->tid,
				(op == WLFC_CTL_TYPE_TID_OPEN));
			break;

		case WLFC_CTL_TYPE_MAC_REQUEST_PACKET:
			pciedev_process_reqst_packet(pciedev, op_data->handle,
				op_data->tid, op_data->minpkts);
			break;

		case WLFC_CTL_TYPE_UPD_FLR_WEIGHT:
			pciedev_upd_flr_weight(pciedev, op_data->handle,
				op_data->tid, op_data->extra_params);
			break;

		case WLFC_CTL_TYPE_ENAB_FFSCH:
			pciedev_set_ffsched(pciedev, op_data->extra_params);
			break;

		default :
			PCI_ERROR(("Need to handle flow control operation %d\n", op));
		}
	}
	return 0;
}

static int
pciedev_ioctl(hnd_dev_t *rtedev, uint32 cmd, void *buf, int len, int *used, int *needed, int set)
{
	drv_t *drv = (drv_t *)rtedev->softc;
	int ret = BCME_OK;
	uint32 outlen = 0;
	struct dngl_bus *pciedev = (struct dngl_bus *)drv->pciedev;
	uint16 start;
	uint32 cnt;
#ifndef PCIE_PHANTOM_DEV
	uint8 state;
#endif // endif
	uint32 copycount = 0;
	uint32 d11rxoffset = 0;

	switch (cmd) {
	case BUS_GET_VAR:
	case BUS_SET_VAR:
		ASSERT((cmd == BUS_GET_VAR) == !set);
		if (strncmp((char *)buf, "bus:", strlen("bus:"))) {
			ret = BCME_ERROR;
			break;
		}

		ret = pciedev_bus_iovar(drv->pciedev, (char *)buf, len, &outlen, set);
		break;
	case BUS_FLUSH_RXREORDER_Q:
		ASSERT(buf);
		ASSERT(len == 2*sizeof(uint32));
		start = (uint16) (((uint32*)buf)[0]);
		cnt = ((uint32*)buf)[1];
		pciedev_rxreorder_queue_flush_cb((void *)pciedev, start, cnt);
		break;
	case BUS_SET_LTR_STATE:
		ASSERT(buf);
#ifndef PCIE_PHANTOM_DEV
		state = (uint8) *((uint32*)buf);
		pciedev_send_ltr((void *)pciedev, state);
#endif // endif
		break;
	case BUS_FLUSH_CHAINED_PKTS:
		pciedev_flush_chained_pkts(pciedev);
		break;
	case BUS_SET_COPY_COUNT:
		copycount  = (((uint32*)buf)[0]);
		d11rxoffset = ((uint32*)buf)[1];

		pciedev_set_copycount_bytes(drv->pciedev, copycount, d11rxoffset);
		break;
#if defined(WL_MONITOR) && !defined(WL_MONITOR_DISABLED)
	case BUS_SET_MONITOR_MODE:
		{
			uint32 monitor_mode = 0;
			monitor_mode = (uint8) *((uint32*)buf);
			pciedev_set_monitor_mode(drv->pciedev, monitor_mode);
			break;
		}
#endif /* WL_MONITOR && WL_MONITOR_DISABLED */
	default:
		ret = BCME_ERROR;
	}

	if (used)
		*used = outlen;

	return ret;
}

pciedev_gpioh_t *
pciedev_gpio_handler_register(uint32 event, bool level,
	pciedev_gpio_handler_t cb, void *arg)
{
	return (pciedev_gpioh_t *)rte_gpio_handler_register(event, level, cb, arg);
}

void
pciedev_gpio_handler_unregister(pciedev_gpioh_t *gi)
{
	rte_gpio_handler_unregister((rte_gpioh_t *)gi);
}

#ifdef HOST_HDR_FETCH
/* Push TXheader from dongle memory to host scratch buffer */
static int
pciedev_txhdr_push(void *dev, void *p, uint queue, bool commit)
{
	if (dev != NULL && p != NULL) {
		drv_t *drv = ((hnd_dev_t *)dev)->softc;
		struct dngl_bus *pciedev = (struct dngl_bus *)drv->pciedev;

		return __pciedev_txhdr_push(pciedev, p, queue, commit);
	}
	return BCME_ERROR;
}
/* Reclaim pkts form pciedev layer */
static int
pciedev_reclaim_txpkts(void *dev, struct spktq *pkt_list, uint16 fifo, bool free)
{
	if (dev != NULL) {
		drv_t *drv = ((hnd_dev_t *)dev)->softc;
		struct dngl_bus *pciedev = (struct dngl_bus *)drv->pciedev;

		return (int)__pciedev_reclaim_txpkts(pciedev, pkt_list, fifo, free);
	}
	return BCME_ERROR;
}
/* Reclaim pkts form pciedev layer */
static void
pciedev_map_txpkts(void *dev, map_pkts_cb_fn cb, void *ctx)
{
	if (dev != NULL) {
		drv_t *drv = ((hnd_dev_t *)dev)->softc;
		struct dngl_bus *pciedev = (struct dngl_bus *)drv->pciedev;

		__pciedev_map_txpkts(pciedev, cb, ctx);
	}
}
/* Last packet int he chunk: Update flags that its ready to be commited */
static void
pciedev_txhdr_commit(void *dev)
{
	if (dev != NULL) {
		drv_t *drv = ((hnd_dev_t *)dev)->softc;
		struct dngl_bus *pciedev = (struct dngl_bus *)drv->pciedev;

		pciedev_dma_tx_account_update_flags(pciedev, PCIEDEV_MPDU_COMMIT);
	}
}
#endif /* HOST_HDR_FETCH */
