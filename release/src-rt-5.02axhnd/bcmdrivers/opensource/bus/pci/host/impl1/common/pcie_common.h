/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom
   All Rights Reserved

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
#ifndef __PCIE_COMMON_H
#define __PCIE_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 *
 *  Defines
 *
 */
#define BCM963XX_PCIE_DEV_NAME                     "bcm963xx-pcie"
#define BCM963XX_PCIE_DRV_NAME                     BCM963XX_PCIE_DEV_NAME
#define PCIE_LINK_SPEED_GEN1                       (1)
#define PCIE_LINK_SPEED_GEN2                       (2)
#define PCIE_LINK_SPEED_GEN3                       (3)

/*
 *
 *  Macros
 *
 */
/* Enable HCD_DEBUG if debug logs are needed */
#define HCD_DEBUG

#ifdef HCD_DEBUG

#define HCD_LOG_LVL_FN                             1
#define HCD_LOG_LVL_ERROR                          2
#define HCD_LOG_LVL_WARN                           4
#define HCD_LOG_LVL_INFO                           8
#define HCD_LOG_LVL_ALL                            \
	    (HCD_LOG_LVL_FN|HCD_LOG_LVL_ERROR|HCD_LOG_LVL_WARN|HCD_LOG_LVL_INFO)

#define HCD_KERN_ERR
#define HCD_KERN_INFO
#define HCD_KERN_WARNING

#define HCD_FN_ENT()                               \
	    if (hcd_log_level & HCD_LOG_LVL_FN) printk(">>%s\n", __FUNCTION__)

#define HCD_FN_EXT()                               \
	    if (hcd_log_level & HCD_LOG_LVL_FN) printk("<<%s\n", __FUNCTION__)

#define HCD_ERROR(fmt, args...)                    \
	    if (hcd_log_level & HCD_LOG_LVL_ERROR)     \
	       printk(HCD_KERN_ERR "  %s: " fmt, BCM963XX_PCIE_DRV_NAME, ## args)

#define HCD_INFO(fmt, args...)                     \
	    if (hcd_log_level & HCD_LOG_LVL_INFO)      \
	        printk(HCD_KERN_INFO "	%s: " fmt, BCM963XX_PCIE_DRV_NAME, ## args)

#define HCD_WARN(fmt, args...)                     \
	    if (hcd_log_level & HCD_LOG_LVL_WARN)      \
	        printk(HCD_KERN_WARNING "  %s: " fmt, BCM963XX_PCIE_DRV_NAME, ## args)

#else
#define HCD_KERN_ERR                               KERN_ERR
#define HCD_KERN_INFO                              KERN_INFO
#define HCD_KERN_WARNING                           KERN_WARNING

#define HCD_FN_ENT()                               do {} while (0)
#define HCD_FN_EXT()                               do {} while (0)
#define HCD_ERROR(fmt, args...)                    do {} while (0)
#define HCD_INFO(fmt, args...)                     do {} while (0)
#define HCD_WARN(fmt, args...)                     do {} while (0)
#endif /* HCD_DEBUG */
#define HCD_LOG(fmt, args...)                      \
	    printk("  %s: " fmt, BCM963XX_PCIE_DRV_NAME, ## args)

/*
 *  Structures
 *
 */
/**
 * BCM963xx PCIe Host Controller configuration from module param or nv storage
 * @ssc: flag to specify PCIe SSC to be enabled or not
 * @speed: flag to specify PCIe to force run at 2.5Gbps
 * @apon: Keep the PCIe core powered on always
 * @phypwrmode: power mode normal or low power mode
 * @errlog: Bus Error logging
 */
struct bcm963xx_pcie_hc_cfg
{
	bool ssc;
	u8 speed;
	bool apon;
	u8 phypwrmode;
	u8 errlog;
};

/* Forward decleration */
struct bcm963xx_pcie_hcd;
extern int hcd_log_level;


/*
 *
 *  Function declerations
 *
 */
int bcm963xx_pcie_common_init(void);
void bcm963xx_pcie_init_hc_cfg(struct bcm963xx_pcie_hcd *pdrv);
int bcm963xx_pcie_get_boot_order_core(int index);
void bcm963xx_pcie_gen2_phy_config_ssc(struct bcm963xx_pcie_hcd *pdrv);
int bcm963xx_pcie_gen2_phy_enable_ssc(struct bcm963xx_pcie_hcd *pdrv,
	bool enable);
void bcm963xx_pcie_phy_config_rescal(struct bcm963xx_pcie_hcd *pdrv);

#ifdef __cplusplus
}
#endif

#endif /* __PCIE_COMMON_H */
