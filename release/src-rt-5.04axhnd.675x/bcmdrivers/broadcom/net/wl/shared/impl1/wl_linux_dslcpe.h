/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
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
