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

#ifndef _wl_linux_dslcpe_h_
#define _wl_linux_dslcpe_h_

#ifndef DSL_VERSION_MAJOR_CODE
#define DSL_VERSION_MAJOR_CODE ((DSL_LINUX_VERSION_CODE>>8)<<8)
#endif

#ifndef DSL_VERSION
#define DSL_VERSION(a, b, c) (((a) << 16) + ((b) << 8) + (c))
#endif

#ifndef DSL_VERSION_MAJOR
#define DSL_VERSION_MAJOR(a, b) (((a) << 16) + ((b) << 8))
#endif

struct wl_info;
struct wlc_info;
struct wlc_pub;

#ifdef DSLCPE_PREALLOC_SKB
typedef struct {
	atomic_t	pktbuffered; /* Buffered prealloced pkt */
	int		band; /* which band this interafce is working on. Not used now. */
	int 		exist; /* whether the interafce is up? */
} wl_wmark_t;
#endif

extern int instance_base;

extern int wl_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
extern int __devinit wl_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
extern void wl_free(struct wl_info *wl);
extern irqreturn_t wl_isr(int irq, void *dev_id);

typedef void (*dslcpe_setup_wlan_led_t)(void *config, int led_idx, int pin, int func, int act_hi);

extern int __devinit wl_dslcpe_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
extern int wl_dslcpe_open(struct net_device *dev);
extern int wl_dslcpe_close(struct net_device *dev);
extern void wlc_dslcpe_boardflags(uint32 *boardflags, uint32 *boardflags2);
extern void wlc_dslcpe_led_attach(void *config, dslcpe_setup_wlan_led_t setup);
extern void wlc_dslcpe_led_detach(void);
extern void wlc_dslcpe_timer_led_blink_timer(void);
extern void wlc_dslcpe_led_timer(void);
extern void wl_dslcpe_led(unsigned char state);
extern void wl_reset_cnt(struct net_device *dev);
#ifdef DSLCPE_DGASP
extern void wlc_shutdown_handler(struct wlc_info *wlc);
#endif

extern int wl_config_check(void);

#ifdef DSLCPE_PREALLOC_SKB
#define WL_MAX_RADIOS    3
extern wl_wmark_t wl_wmark[WL_MAX_RADIOS];
extern void wl_wmark_up(int unit);
extern void wl_wmark_down(int unit);
extern bool wl_pkt_drop_on_wmark_check(uint8 unit, bool is_pktc);
extern int wl_prealloc_skb(struct wl_info *wl, int unit);
#endif

#ifdef DSLCPE_DIAG
extern int diag_connected;
#endif

#ifdef BCMDBG
extern int msglevel;
#endif

#ifdef DSLCPE_PREALLOC_SKB
/* Number of SKB for Prealloc SKB Poll */
#undef MEMSZ_16MB
#define MEMSZ_16MB (16*1024*1024)
#undef MEMSZ_32MB
#define MEMSZ_32MB (32*1024*1024)
#undef MEMSZ_64MB
#define MEMSZ_64MB (64*1024*1024)
#undef MEMSZ_128MB
#define MEMSZ_128MB (128*1024*1024)
#endif /* DSLCPE_PREALLOC_SKB */

extern uint8 wlc_is_acphy(void *wlc);

#endif /* _wl_linux_dslcpe_h_ */
