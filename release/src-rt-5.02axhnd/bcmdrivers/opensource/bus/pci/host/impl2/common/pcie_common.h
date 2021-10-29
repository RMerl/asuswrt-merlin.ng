/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

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
#ifndef __PCIE_COMMON_H
#define __PCIE_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************
  *  Defines
  **************************************/
#define BCM947XX_PCIE_DEV_NAME                     "bcm947xx-pcie"
#define BCM947XX_PCIE_DRV_NAME                     BCM947XX_PCIE_DEV_NAME

/**************************************
  *  Macros
  **************************************/
/* Enable HCD_DEBUG if debug logs are needed */
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
	    if (hcd_log_level & HCD_LOG_LVL_FN) printk(">>%s\n",__FUNCTION__)

#define HCD_FN_EXT()                               \
	    if (hcd_log_level & HCD_LOG_LVL_FN) printk("<<%s\n",__FUNCTION__)

#define HCD_ERROR(fmt, args...)                    \
	    if (hcd_log_level & HCD_LOG_LVL_ERROR)     \
	       printk(HCD_KERN_ERR "  %s: " fmt, BCM947XX_PCIE_DRV_NAME, ## args)

#define HCD_INFO(fmt, args...)                     \
	    if (hcd_log_level & HCD_LOG_LVL_INFO)      \
	        printk(HCD_KERN_INFO "	%s: " fmt, BCM947XX_PCIE_DRV_NAME, ## args)

#define HCD_WARN(fmt, args...)                     \
	    if (hcd_log_level & HCD_LOG_LVL_WARN)      \
	        printk(HCD_KERN_WARNING "  %s: " fmt, BCM947XX_PCIE_DRV_NAME, ## args)

#else
#define HCD_KERN_ERR                               KERN_ERR
#define HCD_KERN_INFO                              KERN_INFO
#define HCD_KERN_WARNING                           KERN_WARNING

#define HCD_FN_ENT()                               do {} while(0)
#define HCD_FN_EXT()                               do {} while(0)
#define HCD_ERROR(fmt, args...)                    do {} while(0)
#define HCD_INFO(fmt, args...)                     do {} while(0)
#define HCD_WARN(fmt, args...)                     do {} while(0)
#endif /* HCD_DEBUG */
#define HCD_LOG(fmt, args...)                      \
	    printk("  %s: " fmt, BCM947XX_PCIE_DRV_NAME, ## args)

/**************************************
  *  Structures
  **************************************/
/**
 * BCM947xx PCIe Host Controller configuration from module param or nv storage
 * @ssc: flag to specify PCIe SSC to be enabled or not
 * @speed: flag to specify PCIe to force run at 2.5Gbps
 */
struct bcm947xx_pcie_hc_cfg
{
	bool ssc;
	u8 speed;
};

/* Forward decleration */
struct bcm947xx_pcie_hcd;
extern int hcd_log_level;


/**************************************
  *  Function declerations
  **************************************/
void bcm947xx_pcie_hcd_init_hc_cfg(struct bcm947xx_pcie_hcd *pdrv);

#ifdef __cplusplus
}
#endif

#endif /* __PCIE_COMMON_H */
