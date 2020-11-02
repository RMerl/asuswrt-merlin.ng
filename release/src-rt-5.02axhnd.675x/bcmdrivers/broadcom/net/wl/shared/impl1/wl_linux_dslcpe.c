/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

    :>
*/

#if defined(DSLCPE)

#include "board.h"

#include <typedefs.h>
#include <linuxver.h>

#include <linux/module.h>
#include <linux/pci.h>
#include <linux/netdevice.h>

#include <osl.h>

#include <wlioctl.h>

#include "bcm_map.h"
#include "bcm_intr.h"
#include "bcmnet.h"
#include "boardparms.h"
#include "bcm_assert_config.h"

#include <wlc_cfg.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wl_linux.h>
#include <wl_linux_dslcpe.h>

#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
#include <linux/gbpm.h>
#endif

#ifdef DSLCPE_DIAG
#include "DiagDef.h"
int diag_connected;

void wl_diag_cmd(unsigned char lineId, int diagCmd, int len, void *pCmdData)
{
	switch (diagCmd) {
	case LOG_CMD_ENABLE_CLIENT:
		diag_connected = 1;
		printk("--Diag Connected--\n");
		break;
	case LOG_CMD_DISABLE_CLIENT:
		diag_connected = 0;
		printk("--Diag Disconnected--\n");
		break;
	}
}
#endif /* DSLCPE_DIAG */

static struct net_device_ops wl_dslcpe_netdev_ops;
#include <bcmendian.h>
#include <bcmdevs.h>

/* USBAP */
#ifdef BCMDBUS
#include "dbus.h"
/* BMAC_NOTES: Remove, but just in case your Linux system has this defined */
#undef CONFIG_PCI
void *wl_dbus_probe_cb(void *arg, const char *desc, uint32 bustype, uint32 hdrlen);
void wl_dbus_disconnect_cb(void *arg);
#endif

#ifdef DSLCPE_PREALLOC_SKB
/* WL PreAlloc RXBUF Mode: Enable(1)/Disable(0) */
int allocskbmode = 1;
module_param(allocskbmode, int, 0);
static uint allocskbsz = 0; /* Set in wl_attach below. CTFPOOLSZ */
module_param(allocskbsz, uint, 0);
#endif /* DSLCPE_PREALLOC_SKB */

#ifdef DSLCPE_CACHE_SMARTFLUSH
uint dsl_tx_pkt_flush_len = 338;
module_param(dsl_tx_pkt_flush_len, uint, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
MODULE_PARM_DESC(dsl_tx_pkt_flush_len, "Fixed len in pkt to flush; set to 0 to disable.");
#endif /* DSLCPE_CACHE_SMARTFLUSH */

extern bool wlc_chipmatch(uint16 vendor, uint16 device);
struct net_device *wl_netdev_get(struct wl_info *wl);
extern void wl_reset_cnt(struct net_device *dev);
extern uint8 osl_get_wlunit(osl_t *osh);

int wl_config_check(void)
{
	BCM_CHECK_KERNEL_CONFIG_SMP("wlan");
	BCM_CHECK_KERNEL_CONFIG_PREEMPT("wlan");
	BCM_CHECK_KERNEL_CONFIG_DEBUG_SPINLOCK("wlan");
	BCM_CHECK_KERNEL_CONFIG_DEBUG_MUTEXES("wlan");
	return 0;
}

#ifdef DSLCPE_PREALLOC_SKB

#define WL_FIFO_WMARK_POLICY
wl_wmark_t wl_wmark[WL_MAX_RADIOS] = {0}; /* wmark structure */
int norm_wmark_tot = 400;
int pktc_wmark_tot = 2048;

/* mark wlx is up */
void wl_wmark_up(int unit)
{
	wl_wmark[unit].exist = 1;
}

/* mark wlx is down or non-exist */
void wl_wmark_down(int unit)
{
	wl_wmark[unit].exist = 0;
}

#ifdef WL_FIFO_WMARK_POLICY
/*
two adapter shares one skb buffer mark.
cnt0 = number of skb queueed in wl0 txq
cnt1 = number of skb queueed in wl1 txq
cnt2 = number of skb queueed in wl2 txq
cnt0+cnt1+cnt2 < wmark_tot; To make sure not over the watermark
*/
bool wl_pkt_drop_on_wmark_check(uint8 unit, bool is_pktc)
{
	int i, cnt[WL_MAX_RADIOS], tot_cnt = 0;
	bool ret = TRUE;
	int wmark_tot;

	/* wl0, wl1 and wl2 */
	ASSERT(unit == 0 || unit == 1 || unit == 2);

	wmark_tot = is_pktc ? pktc_wmark_tot : norm_wmark_tot;
	for (i=0; i<WL_MAX_RADIOS; i++) {
		cnt[i] = atomic_read(&(wl_wmark[i].pktbuffered));
		tot_cnt += cnt[i];
	}
	if (tot_cnt < wmark_tot)
		ret = FALSE;

	/* enqueue pkt */
	return ret;
}
#endif /* WL_FIFO_WMARK_POLICY */

#ifdef WL_STATIC_WMARK_POLICY
/*
each adapter allocate half of the skb buffers
cnt = number of skb queueed in wl txq
cnt < wmark_tot/3; To make sure not over the watermark
*/
bool wl_pkt_drop_on_wmark_check(uint8 unit, bool is_pktc)
{
	int cnt;
	bool ret = TRUE;
	int wmark_tot;

	/* wl0, wl1 and wl2 */
	ASSERT(unit == 0 || unit == 1 || unit == 2);

	wmark_tot = is_pktc ? pktc_wmark_tot : norm_wmark_tot;
	cnt = atomic_read(&(wl_wmark[unit].pktbuffered));

	if (unlikely(wl_wmark[WL_MAX_RADIOS-1-unit].exist)) {
		if (cnt < wmark_tot/WL_MAX_RADIOS)
			ret = FALSE;
	} else
		if (cnt < wmark_tot)
			ret = FALSE;

	/* enqueue pkt */
	return ret;
}
#endif /* WL_STATIC_WMARK_POLICY */

/* increase counter of pktbuffered if pkt is preallocated.
 * used once per pkt from os when the pkt is passed to the driver
 */
void wl_pktpreallocinc(uint8 unit, struct sk_buff *skb, int cnt)
{
	if (cnt == 1)
		atomic_inc(&(wl_wmark[unit].pktbuffered));
	else
		atomic_add(cnt, &(wl_wmark[unit].pktbuffered));

}

/* dec counter of pktbuffered if pkt is preallocated.
 * used once per pkt from os when the pkt is freed from wlan driver
 */
void wl_pktpreallocdec(uint8 unit, struct sk_buff *skb)
{
	atomic_dec(&(wl_wmark[unit].pktbuffered));
}

/* decide whether to drop or not */
bool wl_pkt_drop_on_wmark(osl_t *osh, bool is_pktc)
{
	uint8 unit;

	unit = osl_get_wlunit(osh);
	return wl_pkt_drop_on_wmark_check(unit, is_pktc);
}

int wl_prealloc_skb(wl_info_t *wl, int unit)
{
	int currallocskbsz = 0;

	wl->prealloc_skb_mode = (allocskbmode == 0) ? TRUE : FALSE;
	if (allocskbmode) {
		if (allocskbsz) {
			currallocskbsz = allocskbsz;
		} else {
			struct sysinfo i;
			int isac = wlc_is_acphy(wl->wlc);
			si_meminfo(&i);

			if (i.totalram <= MEMSZ_32MB/i.mem_unit) {
				if (isac)
					/* when the chip is AC,
					 * it need more than 320 permenant allocation.
					 */
					currallocskbsz = 512;
				else
					currallocskbsz = 256;
			} else if (i.totalram <= MEMSZ_64MB/i.mem_unit) {
				if (!isac) {
					if (unit == 0)
						currallocskbsz = 256;
					else
						currallocskbsz = 1024;
				} else { /* 11ac */
					if (unit == 0)
						currallocskbsz = 1024;
					else
						currallocskbsz = 1536;
				}
			} else if (i.totalram <= MEMSZ_128MB/i.mem_unit) {
				if (!isac) {
					if (unit == 0)
						currallocskbsz = 1024;
					else
						currallocskbsz = 2300;
				} else { /* 11ac */
#if defined(CONFIG_BCM947189)
					if (unit == 0) //47189 internal 5G wifi
						currallocskbsz = 3500;
					else
						currallocskbsz = 3500;
#else
					if (unit == 0)
						currallocskbsz = 1024;
					else
						currallocskbsz = 2048;
#endif
				}
			} else { /* > 128MB */
				if (!isac) {
					if (unit == 0)
						currallocskbsz = 1024;
					else
						currallocskbsz = 2300;
				} else { /* 11ac */
					if (unit == 0)
						currallocskbsz = 3500;
					else
						currallocskbsz = 3500;
				}
			}
		}
	}
	printk("wl%d: allocskbmode=%x currallocskbsz=%d\n",
	        unit, allocskbmode, currallocskbsz);

	return currallocskbsz;
}
#endif /* DSLCPE_PREALLOC_SKB */


/*
 * wl_dslcpe_open:
 * extended hook for device open for DSLCPE.
 */
int wl_dslcpe_open(struct net_device *dev)
{
	return 0;
}

/*
 * wl_dslcpe_close:
 * extended hook for device close for DSLCPE.
 */
int wl_dslcpe_close(struct net_device *dev)
{
	return 0;
}
/*
 * wlc_dslcpe_boardflags:
 * extended hook for modifying boardflags for DSLCPE.
 */
void wlc_dslcpe_boardflags(uint32 *boardflags, uint32 *boardflags2)
{
	return;
}

/*
 * wlc_dslcpe_led_attach:
 * extended hook for when led is to be initialized for DSLCPE.
 */

void wlc_dslcpe_led_attach(void *config, dslcpe_setup_wlan_led_t setup_dslcpe_wlan_led)
{
	setup_dslcpe_wlan_led(config, 0, 0, WL_LED_ACTIVITY, 1);
	setup_dslcpe_wlan_led(config, 1, 1, WL_LED_BRADIO, 1);
	return;
}

/*
 * wlc_dslcpe_led_detach:
 * extended hook for when led is to be de-initialized for DSLCPE.
 */
void wlc_dslcpe_led_detach(void)
{
	return;
}
/*
 * wlc_dslcpe_timer_led_blink_timer:
 * extended hook for when periodical(10ms) led timer is called for DSLCPE when wlc is up.
 */
void wlc_dslcpe_timer_led_blink_timer(void)
{
	return;
}
/*
 * wlc_dslcpe_led_timer:
 * extended hook for when led blink timer(200ms) is called for DSLCPE when wlc is up.
 */
void wlc_dslcpe_led_timer(void)
{
	return;
}

/*
 * wl_dslcpe_ioctl:
 * extended ioctl support on BCM63XX.
 */
int
wl_dslcpe_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	int isup = 0;
	int error = -1;

	if (cmd >= SIOCGLINKSTATE && cmd < SIOCLAST) {
		error = 0;
		/* we can add sub-command in ifr_data if we need to in the future */
		switch (cmd) {
		case SIOCGLINKSTATE:
			if (dev->flags&IFF_UP) isup = 1;
			if (copy_to_user((void*)(int*)ifr->ifr_data, (void*)&isup,
				sizeof(int))) {
				return -EFAULT;
			}
			break;
		case SIOCSCLEARMIBCNTR:
			wl_reset_cnt(dev);
			break;
		}
	} else {
		error = wl_ioctl(dev, ifr, cmd);
	}
	return error;
}

#ifdef DSLCPE_DGASP
void wl_shutdown_handler(wl_info_t *wl)
{
	wlc_shutdown_handler(wl->wlc);
}
#endif

#if 0
/* same function as in dhd driver(dhdpcie_chipmatch) where it check if the
 * chip support dongle mode
 */
static bool
wl_donglechip_match(uint16 vendor, uint16 device)
{
	if (vendor != PCI_VENDOR_ID_BROADCOM) {
		return (-ENODEV);
	}
	if ((device == BCM4350_D11AC_ID) || (device == BCM4350_D11AC2G_ID) ||
		(device == BCM4350_D11AC5G_ID) || BCM4350_CHIP(device))
		return 0;
	if ((device == BCM4354_D11AC_ID) || (device == BCM4354_D11AC2G_ID) ||
		(device == BCM4354_D11AC5G_ID) || (device == BCM4354_CHIP_ID))
		return 0;
	if ((device == BCM4345_D11AC_ID) || (device == BCM4345_D11AC2G_ID) ||
		(device == BCM4345_D11AC5G_ID) || (device == BCM4345_CHIP_ID))
		return 0;
	if ((device == BCM4335_D11AC_ID) || (device == BCM4335_D11AC2G_ID) ||
		(device == BCM4335_D11AC5G_ID) || (device == BCM4335_CHIP_ID))
		return 0;
	if ((device == BCM43602_D11AC_ID) || (device == BCM43602_D11AC2G_ID) ||
		(device == BCM43602_D11AC5G_ID) || (device == BCM43602_CHIP_ID))
		return 0;

	return (-ENODEV);
}
#endif

#if defined(CONFIG_PCI)
/* special deal for dslcpe */
int __devinit
wl_dslcpe_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct wl_info *wl;
	struct net_device *dev;


#if defined(CONFIG_BRCM_IKOS)
	wl_msg_level = wl_msg_level2 = 0xffffffff;
#endif

#if 0
	if (kerSysGetWlanFeature() & WLAN_FEATURE_DHD_NIC_ENABLE) {

		printk("---- NIC MODE is enabled,continue..---\n");

	} else if (!wl_donglechip_match(pdev->vendor, pdev->device)) {

		printk("Dongle chip:0x%04x quit wl.ko to be loaded with DHD.\n", pdev->device);
		return (-ENODEV);
	}
#endif

	if (wl_pci_probe(pdev, ent))
		return -ENODEV;

	wl = pci_get_drvdata(pdev);
	ASSERT(wl);

	/* hook ioctl */
	dev = wl_netdev_get(wl);
	/* note -- this is sort of cheating, as we are changing
	 * a pointer in a shared global structure, but... this should
	 * work, as we are likely not to mix dslcpe wl's with non-dslcpe wl;s.
	 * as well, it prevents us from having to export some symbols we don't
	 * want to export.  A proper fix might be to add this to the
	 * wlif structure, and point netdev ops there.
	 */
	memcpy(&wl_dslcpe_netdev_ops, dev->netdev_ops, sizeof(struct net_device_ops));
	wl_dslcpe_netdev_ops.ndo_do_ioctl = wl_dslcpe_ioctl;
	dev->netdev_ops = &wl_dslcpe_netdev_ops;

#ifdef DSLCPE_DGASP
	kerSysRegisterDyingGaspHandler(dev->name, &wl_shutdown_handler, wl);
#endif

	return 0;
}

void __devexit wl_remove(struct pci_dev *pdev);

#if defined(CONFIG_BCM947189)
struct wl_cmn_data {
	void *objrptr;
	void *oshcmnptr;
	si_t *sih;
	uint16 device;
};

static struct wl_cmn_data wlcmndata = { 0 };
/* recognized PCI IDs */
static struct pci_device_id wl_id_table[] =
{
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	PCI_CLASS_NETWORK_OTHER << 8, 0xffff00, (unsigned long)&wlcmndata},
	{ 0 }
};
#else
static struct pci_device_id wl_id_table[] =
{
	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	PCI_CLASS_NETWORK_OTHER << 8, 0xffff00, 0 },
	{ 0 }
};
#endif


static struct pci_driver wl_dslcpe_pci_driver = {
	name:		"wl",
	probe:		wl_dslcpe_probe,
	remove:		__devexit_p(wl_remove),
	id_table:	wl_id_table,
	};

#endif  /* defined(CONFIG_PCI) */

/* USBAP  Could combined with wl_dslcpe_probe */
#ifdef BCMDBUS
static void *wl_dslcpe_dbus_probe_cb(void *arg, const char *desc, uint32 bustype, uint32 hdrlen)
{
	struct net_device *dev;
	wl_info_t *wl = wl_dbus_probe_cb(arg, desc, bustype, hdrlen);
	int irq;

	ASSERT(wl);

	/* hook ioctl */
	dev = wl_netdev_get(wl);
	/* note -- this is sort of cheating, as we are changing
	 * a pointer in a shared global structure, but... this should
	 * work, as we are likely not to mix dslcpe wl's with non-dslcpe wl;s.
	 * as well, it prevents us from having to export some symbols we don't
	 * want to export.  A proper fix might be to add this to the
	 * wlif structure, and point netdev ops there.
	 */
	memcpy(&wl_dslcpe_netdev_ops, dev->netdev_ops, sizeof(struct net_device_ops));
	wl_dslcpe_netdev_ops.ndo_do_ioctl = wl_dslcpe_ioctl;
	dev->netdev_ops = &wl_dslcpe_netdev_ops;

#ifdef DSLCPE_DGASP
	kerSysRegisterDyingGaspHandler(dev->name, &wl_shutdown_handler, wl);
#endif
	return 0;
}

static void wl_dslcpe_dbus_disconnect_cb(void *arg)
{
	wl_dbus_disconnect_cb(arg);
}
#endif /* BCMDBUS */

static int __init
wl_module_init(void)
{
	int error;
#ifdef DSLCPE_PREALLOC_SKB
	int i;
#endif
#ifdef DSLCPE_DIAG
	printk("--DSLCPE_DIAG--\n");
	BcmDiagsMgrRegisterClient(DIAG_WLAN_CLIENT, wl_diag_cmd);
#endif
#ifdef CONFIG_SMP
	printk("--SMP support\n");
#endif
#ifdef CONFIG_BCM_WAPI
	printk("--WAPI support\n");
#endif

#if defined(DSLCPE_CACHE_SMARTFLUSH) && defined(PKTDIRTYPISPRESENT)
	printk("wl: dsl_tx_pkt_flush_len=%d\n", dsl_tx_pkt_flush_len);
#endif

#ifdef DSLCPE_PREALLOC_SKB
#if defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE)
#ifdef CONFIG_GBPM_API_HAS_GET_TOTAL_BUFS
	norm_wmark_tot = pktc_wmark_tot = (int) (gbpm_get_max_dyn_bufs()*65/100);
#endif /* CONFIG_BPM_API_HAS_GET_TOTAL_BUFS */
#elif defined(CONFIG_BCM_FEED_RING_DYNAMIC)
	norm_wmark_tot = pktc_wmark_tot = CONFIG_BCM_FEED_RING_MAX_ALLOCATIONS;
#endif /* CONFIG_BCM_BPM || CONFIG_BCM_BPM_MODULE */
	printk("wl: norm_wmark_tot=%d, pktc_wmark_tot=%d\n", norm_wmark_tot, pktc_wmark_tot);
#endif /* DSLCPE_PREALLOC_SKB */

	if (wl_config_check())
		return -1;
#ifdef BCMDBG
	if (msglevel != 0xdeadbeef) {
		/* wl_msg_level = msglevel; */
		printf("%s: msglevel set to 0x%x\n", __FUNCTION__, msglevel);
	}
#endif /* BCMDBG */

#ifdef DSLCPE_PREALLOC_SKB
	for (i = 0; i < WL_MAX_RADIOS; i++) {
		atomic_set(&(wl_wmark[i].pktbuffered), 0);
		wl_wmark_down(i);
	}
#endif

#ifdef CONFIG_PCI
	if (!(error = pci_module_init(&wl_dslcpe_pci_driver)))
		return (0);
#endif /* CONFIG_PCI */

#ifdef BCMDBUS
	/* BMAC_NOTE: define hardcode number, why NODEVICE is ok ? */
	error = dbus_register(BCM_DNGL_VID, BCM_DNGL_BDC_PID, wl_dslcpe_dbus_probe_cb,
		wl_dslcpe_dbus_disconnect_cb, NULL, NULL, NULL);
	if (error == DBUS_ERR_NODEVICE) {
		error = DBUS_OK;
	}
#endif /* BCMDBUS */

	return (error);
}

static void __exit
wl_module_exit(void)
{
#ifdef CONFIG_PCI
	pci_unregister_driver(&wl_dslcpe_pci_driver);
#endif	/* CONFIG_PCI */

#ifdef BCMDBUS
	dbus_deregister();
#endif /* BCMDBUS */
}

/* Turn 63xx GPIO LED On(1) or Off(0) */
void wl_dslcpe_led(unsigned char state)
{
/* if WLAN LED is from 63XX GPIO Line, define compiler flag GPIO_LED_FROM_63XX
#define GPIO_LED_FROM_63XX
*/

#ifdef GPIO_LED_FROM_63XX
	BOARD_LED_STATE led;
	led = state? kLedStateOn : kLedStateOff;

	kerSysLedCtrl(kLedSes, led);
#endif
}

module_init(wl_module_init);
module_exit(wl_module_exit);
MODULE_LICENSE("Proprietary");

#endif /* DSLCPE */
