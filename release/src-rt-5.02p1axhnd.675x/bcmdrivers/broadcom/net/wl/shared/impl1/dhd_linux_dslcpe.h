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

/* This is to customize dhd_linux for DSLCPE */

#include <typedefs.h>
#include <board.h>
#include <linux/module.h>

#ifdef mips
#undef ABS
#endif

#include <siutils.h>
#include <bcmendian.h>

#if defined(BCM_BLOG)
#include <linux/bcm_dslcpe_wlan_info.h>
#endif

#include <dngl_stats.h>
#include <dhd_wmf_linux.h>
#include <dhd_proto.h>
#include <dhd_bus.h>

#if defined(BCM_WFD)
#include "dhd_wfd.h"
#endif

#if defined(BCM_DHD_RUNNER)
#include <bcmmsgbuf.h>
#include <dhd_runner.h>
#endif /* BCM_DHD_RUNNER */

#include "bcmnet.h"

#if (defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE))
#include <linux/bcm_log.h>
#include "spdsvc_defs.h"
#endif

extern char mfg_firmware_path[];

extern int dhd_prealloc_attach(void *p);
extern int dhd_prealloc_detach(void *p);

extern int dhd_check_and_set_mac(dhd_pub_t *dhd, char *memblock, uint *len);
extern char* dhd_get_nvram_mutxmax(dhd_pub_t *dhd);
extern int dhd_check_and_set_mutxmax(dhd_pub_t *dhd, char *memblock, uint* len);


#define  UINT32_DELTA_INC(a,b)  (((a)>=(b))?(a-b):(a+(0xffffffff-b)))

#if defined(BCM_DHD_RUNNER)
/* Get active station count on interface */
extern uint16 dhd_get_sta_cnt(void *pub, int ifidx, void *ea);
extern void dhd_prot_schedule_runner(dhd_pub_t *dhd);
#endif /* BCM_DHD_RUNNER */

/* defined in hndfwd.h */
#ifndef FWDER_MAX_RADIO
#define FWDER_MAX_RADIO 	(4)
#endif

#if defined(DHD_WMF)
int32 dhd_wmf_stall_sta_check_fn(void *wrapper, void *p, uint32 mgrp_ip);
int32 dhd_wmf_forward_fn(void *wrapper, void *p, uint32 mgrp_ip, void *txif, int rt_port);
void *dhd_wmf_get_igsc(void *pdev) ;
void *dhd_wmf_sdbfind(void *pdev,void *mac); 
void *dhd_wmf_get_device(void *dhd_p,int idx);
#if defined(DSLCPE) && defined(BCM_BLOG)
int dhd_client_get_info(struct net_device *dev,char *mac,int priority, wlan_client_info_t *info_p);
#endif
void dhd_wmf_update_if_stats(void *dhd_p,int ifidx,int wmf_action,unsigned int pktlen,bool frombss);
#endif

#if defined(DSLCPE_PLATFORM_WITH_RUNNER)
extern int dhd_flowid_get_type(dhd_pub_t *dhdp, void *sta_p,uint8 priority);
#endif

#ifdef DSLCPE_DONGLE
extern int _dhd_set_mac_address(struct dhd_info *dhd, int ifidx, uint8 *addr);
#endif

#define WMF_DIRTYP_LEN (64)
#define STA_HW_ACC_UNKNOWN  (0) 
#define STA_HW_ACC_ENABLED  (1)
#define STA_HW_ACC_DISABLED (2)

void update_firmware_path(struct dhd_bus *bus, char *pfw_path);

extern void dhd_get_cfe_mac(dhd_pub_t *dhd, struct ether_addr *mac);
int dhd_vars_adjust(struct dhd_bus *bus, char *memblock,uint *total_len);

#ifdef DSLCPE_DONGLEHOST_WOMBO
extern int update_nvram_from_srom(si_t *sih, osl_t *osh, char *memblock, uint *len);
extern int initvars_srom_pci(si_t *sih, void *curmap, char **vars, uint *count);
extern int dhd_get_instance(dhd_pub_t *dhdp);
extern uint8* srom_offset(si_t *sih, void *curmap);
extern int sprom_read_pci(osl_t *osh, si_t *sih, uint16 *sprom, uint wordoff, uint16 *buf,
                          uint nwords, bool check_crc);
int dhdpcie_download_map_bin(struct dhd_bus *bus);
#endif


void dhd_if_clear_stats(dhd_pub_t *dhdp, int ifidx);
void dhd_reset_cnt(struct net_device *net);
int dhd_priv_ioctl(struct net_device *net, struct ifreq *ifr, int cmd);

/* Check for and handle local prot-specific iovar commands */
extern int dhd_dslcpe_iovar_op(dhd_pub_t *dhdp, const char *name,
                     void *params, int plen, void *arg, int len, bool set);

extern dhd_pub_t *dhd_dev_get_dhdpub(struct net_device *dev);
extern int dhd_dev_get_ifidx(struct net_device *dev);

#if defined(DSLCPE_PWLCS)
extern int dhd_pwlcs_init_bc(int unit);
extern int dhd_pwlcs_reset_bc(int unit);
#if defined(DSLCPE_PWLCS_TEST)
extern int dhd_pwlcs_test_bc(int unit, struct pci_dev *pdev);
#endif /* DSLCPE_PWLCS_TEST */

extern void dhd_pwlcs_unregister_dummyif(struct net_device *dev);
struct net_device * dhd_pwlcs_register_dummyif(char *ifname, bool need_rtnl_lock);
extern int dhd_found;
#endif /* DSLCPE_PWLCS */
