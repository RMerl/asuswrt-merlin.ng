/*
<:copyright-BRCM:2012:DUAL/GPL:standard

   Copyright (c) 2012 Broadcom
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

/** Includes. **/
#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_string.h"
#include "lib_printf.h"

#include "cfe_iocb.h"
#include "cfe_ioctl.h"
#include "cfe_device.h"
#include "cfe_devfuncs.h"
#include "bdmf_errno.h"
#include "cfe_timer.h"
#include "bcmtypes.h"
#include "boardparms.h"
#include "data_path_init_basic.h"
#ifdef MAC_LPORT
#include "lport_drv.h"
#include "lport_mdio.h"
#include "lport_stats.h"
#else
#include "unimac_drv.h"
#ifdef _BCM96846_
#define CFE_NUM_OF_PORTS 5
#else
#define CFE_NUM_OF_PORTS 6
#endif
#endif
#ifndef CONFIG_BRCM_IKOS
#include "pmc_xrdp.h"
#endif
#ifdef MAC_LPORT
#include "pmc_lport.h"
#endif
#include "rdpa_types.h"
#include "rdpa_cpu_basic.h"
#ifndef CONFIG_GPL_RDP
#include "rdd_scheduling.h"
#include "rdd_basic_scheduler.h"
#endif
#include "rdp_cpu_ring.h"
#include "rdd_cpu_rx.h"
#define _BDMF_INTERFACE_H_
#define NO_BDMF_HANDLE
#undef USE_BDMF_SHELL
#ifdef CPU_TX_RING
#include "rdd_cpu_tx_ring.h"
#else
#include "rdd_cpu_tx.h"
#include "rdp_drv_fpm.h"
#include "rdp_drv_qm.h"
#endif
#ifdef MAC_LPORT
#include "bcm6858_lport_mab_ag.h"
#include "bcm6858_lport_xlmac_ag.h"
#else
#ifndef CONFIG_GPL_RDP
#include "xrdp_drv_unimac_mib_ag.h"
#include "xrdp_drv_unimac_misc_ag.h"
#include "xrdp_drv_unimac_rdp_ag.h"
#endif
#endif
#ifndef CONFIG_GPL_RDP
#include "xrdp_drv_bbh_tx_ag.h"
#include "rdp_drv_bbh_rx.h"
#endif
#ifndef CONFIG_BRCM_IKOS
#include "clk_rst.h"
#endif
#include "bcm63xx_dtb.h"
#include "ru_types.h"
#include "mac_drv.h"
#include "phy_drv.h"
#include "phy_bp_parsing.h"
#define NO_AUTO_PORT (CFE_NUM_OF_PORTS + 1)

static phy_dev_t *phy_devices[CFE_NUM_OF_PORTS];
static mac_dev_t *mac_devices[CFE_NUM_OF_PORTS];
static /*const*/ ETHERNET_MAC_INFO *emac_info;

/** Variables. **/

#define ETH_ALEN        6       /* Octets in one ethernet addr */
#ifndef _CFE_REDUCED_XRDP_
#define QM_QUEUE_INDEX_DS_FIRST QM_QUEUE_US_DEFAULT_QUANTITY
#else
#define QM_QUEUE_INDEX_DS_FIRST QM_QUEUE_DS_START
#endif
#ifdef CONFIG_BRCM_IKOS
#define CFE_CPU_RING_SIZE 128
#else
#define CFE_CPU_RING_SIZE 1024
#endif
#define CFE_CPU_FEED_RING_SIZE CFE_CPU_RING_SIZE/2

static uint8_t data[1536]={
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x08, 0x06, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
        0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x08, 0x06, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
        0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x08, 0x06, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
        0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x08, 0x06, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
        0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x08, 0x06, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
        0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x08, 0x06, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
        0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};


int cfe_set_compare_enable(int is_enable);
int send_dummy_packet(uint8_t port_id, uint32_t packet_size, int number_of_packets, uint8_t is_random);
void run_test(uint8_t port_id, uint32_t packet_size, int inter);

typedef struct bcmenet_sc_s
{
    uint8_t hwaddr[ETH_ALEN];
    uint8_t active_tx_port;
    uint8_t last_port_received_from;
    cfe_devctx_t *ctx;
    uint32_t dropped_pkts[CFE_NUM_OF_PORTS + 1];
    uint32_t rx_pkts[CFE_NUM_OF_PORTS];
    uint32_t tx_pkts[CFE_NUM_OF_PORTS];
} bcmenet_sc_t;

uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx)
{
    return RDD_VIRT_TO_PHYS(ru_block->addr[addr_idx]);
}

int mac_stats_get(uint32_t port, mac_stats_t *stats)
{
    if (port >= CFE_NUM_OF_PORTS || !mac_devices[port] || !mac_devices[port]->mac_drv->stats_get)
        return -1;

    return mac_devices[port]->mac_drv->stats_get(mac_devices[port], stats);
}

void mac_stats_clear(uint32_t port)
{
    if (port >= CFE_NUM_OF_PORTS || !mac_devices[port] || !mac_devices[port]->mac_drv->stats_clear)
        return;

    mac_devices[port]->mac_drv->stats_clear(mac_devices[port]);
}

/** Private Functions **/

static int cpu_ring_and_scheduler_init(void)
{
    int rc;
    bdmf_phys_addr_t cpu_ring_head;
    int cpu_ring_idx = 0;
#ifndef CONFIG_GPL_RDP
    int i;
    rdpa_cpu_reason reason;
    uint8_t basic_scheduler_idx = 0;
    basic_scheduler_cfg_t basic_sched_cfg = {
        .dwrr_offset = basic_scheduler_full_sp,
    };
    basic_scheduler_queue_t basic_sched_queue_cfg = {
        .queue_scheduler_index = 0,
        .quantum_number = 1
    };
#ifdef MAC_LPORT
    uint32_t flow_map[] = {BB_ID_RX_XLMAC1_0_RGMII, BB_ID_RX_XLMAC0_1_2P5G, BB_ID_RX_XLMAC0_2_1G, BB_ID_RX_XLMAC0_3_1G,
        BB_ID_RX_XLMAC0_0_10G, BB_ID_RX_XLMAC1_1_RGMII, BB_ID_RX_XLMAC1_2_RGMII};
#else
#if defined(CONFIG_BCM96846) || defined(_BCM96846_)
    uint32_t flow_map[] = { BB_ID_RX_BBH_0, BB_ID_RX_BBH_1, BB_ID_RX_BBH_2, BB_ID_RX_BBH_3, BB_ID_RX_BBH_4 };
#else
    uint32_t flow_map[] = { BB_ID_RX_BBH_0, BB_ID_RX_BBH_1, BB_ID_RX_BBH_2, BB_ID_RX_BBH_3, BB_ID_RX_BBH_4, BB_ID_RX_BBH_5 };
#endif
#endif
    uint32_t num_of_queues = sizeof(flow_map)/sizeof(uint32_t);

#ifndef CPU_TX_RING
    /* Basic system configuration */
    rdd_full_flow_cache_cfg(1);
#endif

#endif  /* #ifndef CONFIG_GPL_RDP */

    /*Create Feed ring */

    ACCESS_LOG_ENABLE_SET(0);
    rc = rdp_cpu_ring_create_ring(FEED_RING_ID, rdpa_ring_feed, CFE_CPU_FEED_RING_SIZE, &cpu_ring_head,
        BCM_PKTBUF_SIZE, NULL);
    if (rc)
    {
        printf("FEED ring creation failed rc %d\n", rc);
        goto exit;
    }
    rdd_ring_init(FEED_RING_ID, rdpa_ring_feed, cpu_ring_head, CFE_CPU_FEED_RING_SIZE,
        sizeof(RDD_CPU_FEED_DESCRIPTOR_DTS), 0, CFE_CPU_FEED_RING_SIZE - 1, 0, 0);

    /* Basic cpu configuration */
    rc = rdp_cpu_ring_create_ring(cpu_ring_idx, rdpa_ring_data, CFE_CPU_RING_SIZE, &cpu_ring_head, BCM_PKTBUF_SIZE, NULL);
    if (rc)
    {
        printf("CPU ring creation failed rc %d\n", rc);
        goto exit;
    }

    rdd_ring_init(cpu_ring_idx, rdpa_ring_data, cpu_ring_head, CFE_CPU_RING_SIZE, sizeof(RDD_CPU_RX_DESCRIPTOR_DTS),
        cpu_ring_idx, 0, 0, 0);
    ACCESS_LOG_ENABLE_SET(1);

#ifndef CONFIG_GPL_RDP
    /* for simplicity, map all reasons to TC=0. By default, all TCs are mapped to RXQ=0 during datapath init */
    for (reason = rdpa_cpu_reason_min; reason < rdpa_cpu_reason__num_of; reason++)
        rdd_ag_cpu_rx_cpu_reason_to_tc_set(0, reason);

    for (i = 0; i < num_of_queues; i++)
    {
        /* basic scheduler configuration */
        basic_sched_cfg.bbh_queue_index = i;
        rc = rdd_basic_scheduler_cfg(rdpa_dir_ds, basic_scheduler_idx + i, &basic_sched_cfg);
        if (rc)
        {
            printf("Scheduler configuration failed rc %d\n", rc);
            goto exit;
        }
#ifdef _BCM96858_
        if ((i == 0) || (i == 4))
            basic_sched_queue_cfg.qm_queue_index = QM_QUEUE_INDEX_DS_FIRST + (4 - i);
        else
#endif
            basic_sched_queue_cfg.qm_queue_index = QM_QUEUE_INDEX_DS_FIRST + i;
        rc = rdd_basic_scheduler_queue_cfg(rdpa_dir_ds, basic_scheduler_idx + i, &basic_sched_queue_cfg);
        if (rc)
        {
            printf("Scheduler queue configuration failed rc %d\n", rc);
            goto exit;
        }
        rdd_set_queue_enable(basic_sched_queue_cfg.qm_queue_index, 1);
        rdd_rx_flow_cfg((256 + flow_map[i]), 0, i, 0);
    }
#endif /* #ifndef CONFIG_GPL_RDP */

exit:
    printf("CPU ring init %s\n", rc ? "Failed":"Done");
    return rc;
}

static int find_xfi_port(void)
{
    uint32_t iter;

    for (iter = 0; iter < CFE_NUM_OF_PORTS; iter++)
    {
        if (phy_devices[iter])
        {
            if (phy_devices[iter]->mii_type == PHY_MII_TYPE_XFI)
                return (rdpa_emac0 + iter);
        }
    }

    return rdpa_emac_none;
}

/** Private Functions **/
extern unsigned long cfe_get_sdram_size(void);
#ifndef CONFIG_BRCM_IKOS
static void alloc_memory_for_fpm_pool(unsigned long* addr, unsigned long* size)
{
    unsigned long mem_end = cfe_get_sdram_size();

    /* Current implementation:
      take LAST 32MB of the memory */
    *size = 0x2000000; /* 32MB is needed to operate */
    *addr = mem_end - *size;
}

static void copy_port_info(const ETHERNET_MAC_INFO *emac_info, uint32_t port, EMAC_PORT_INFO *port_info)
{
    port_info->switch_port  = port;
    port_info->phy_id       = emac_info->sw.phy_id[port];
    port_info->phyconn      = emac_info->sw.phyconn[port];
    port_info->phy_devName  = emac_info->sw.phy_devName[port];
    memcpy(&(port_info->ledInfo), &(emac_info->sw.ledInfo[port]), sizeof(port_info->ledInfo));
    port_info->phyinit      = emac_info->sw.phyinit[port];
    port_info->port_flags   = emac_info->sw.port_flags[port];
    port_info->phyReset     = emac_info->sw.phyReset[port];
    port_info->oamIndex     = emac_info->sw.oamIndex[port];
    port_info->portMaxRate  = emac_info->sw.portMaxRate[port];
}
#endif

static int xrdp_init(void)
{
    dpi_params_t xrdp_init_params;
    unsigned long  address;
    unsigned long size;
    int rc = 0;

    printf("XRDP INIT start\n");

    memset(&xrdp_init_params,0,sizeof(xrdp_init_params));

    xrdp_init_params.mtu_size = 1518;
    xrdp_init_params.headroom_size = 128;

#ifdef CONFIG_BRCM_IKOS
        size = 0x2000000;
        address = (unsigned long)KMALLOC(size, 0x200000);
        if(!address)
        {
            printf("XRDP INIT failed to allocate memory for xrdp\n");
            rc = BDMF_ERR_NOMEM;
            goto exit;
        }
#else
        alloc_memory_for_fpm_pool(&address, &size);

#endif
        printf("XRDP INIT FPM Address %p\n", address);
#ifndef CONFIG_BRCM_IKOS
    memset((void *)address, 0, size);
#endif

    /* fpm pool DDR0 virtual base address for unicast packets */

    printf("*** FIXME: RDP virtual memory workaround ***\n");
    xrdp_init_params.rdp_ddr_pkt_base_virt = (void*)0x0f000000; //address
    xrdp_init_params.enabled_port_map = 0xFF;
    xrdp_init_params.fpm_buf_size = 512;
    xrdp_init_params.bbh_id_gbe_wan = BBH_ID_NULL;

#ifndef CONFIG_BRCM_IKOS
    rc = get_rdp_freq(&xrdp_init_params.runner_freq);
    if (rc)
    {
        printf("Failed to get runner freq %d\n", rc);
        goto exit;
    }
#else
#if defined(CONFIG_BCM96856) || defined(_BCM96856_)
    printk("Using manual RDP Freq=1400\n");
    xrdp_init_params.runner_freq = 1400;
#else
    printk("Using manual RDP Freq=1000\n");
    xrdp_init_params.runner_freq = 1000;
#endif
#endif

    xrdp_init_params.xfi_port = find_xfi_port();

    rc = data_path_init_basic(&xrdp_init_params);
    if (rc)
    {
       printf("Failed to run XRDP init\n");
       goto exit;
    }
exit:
    printf("XRDP INIT %s\n", rc ? "Failed": "Done");
    return rc;
}

static int parse_board_params(void)
{
    uint32_t iter;

    printf("Parse board Params Start\n");

    if ((emac_info =(ETHERNET_MAC_INFO *)BpGetEthernetMacInfoArrayPtr()) == NULL)
    {
        printf("Error reading Ethernet MAC info from board params\n");
        return -1;
    }

    mac_drivers_set();
    phy_drivers_set();

#ifdef CONFIG_BRCM_IKOS
    emac_info->sw.port_map |= (1<<0) | (1<<1);
    emac_info->sw.phy_id[0] = MAC_IF_GMII | PHY_EXTERNAL| 3;
    emac_info->sw.phy_id[1] = MAC_IF_GMII | PHY_EXTERNAL| 3;

#endif

    for (iter = 0; iter < CFE_NUM_OF_PORTS; iter++)
    {
        if (emac_info->sw.port_map & (1<<iter))
        {
#ifdef CONFIG_BRCM_IKOS
            mac_devices[iter] = KMALLOC(sizeof(mac_dev_t), 0);
            mac_devices[iter]->mac_id = MAC_TYPE_UNIMAC;
            mac_devices[iter]->priv = (void *)1;
            mac_devices[iter]->mac_drv = NULL;
            phy_devices[iter] = KMALLOC(sizeof(phy_dev_t), 0);
            phy_devices[iter]->phy_drv = NULL;
            phy_devices[iter]->mii_type = PHY_MII_TYPE_GMII;
            phy_devices[iter]->link_change_cb = NULL;
            phy_devices[iter]->link_change_ctx = 0;
            phy_devices[iter]->addr = 10;
            phy_devices[iter]->priv = (void *)1;
            phy_devices[iter]->link = 0;
            phy_devices[iter]->speed = PHY_SPEED_1000;
            phy_devices[iter]->duplex = PHY_DUPLEX_FULL;
#else
            EMAC_PORT_INFO port_info = {};
            copy_port_info(emac_info, iter, &port_info);
            mac_devices[iter] = bp_parse_mac_dev(emac_info, iter);
            phy_devices[iter] = bp_parse_phy_dev(&port_info);
#endif
        }
    }

    return 0;
}

#ifndef CONFIG_BRCM_IKOS
static int phy_mac_init(void)
{
    uint32_t iter;

    printf("MAC and PHY Init Start\n");

    mac_drivers_init();

    for (iter = 0; iter < CFE_NUM_OF_PORTS; iter++)
    {
        if (mac_devices[iter])
        {
            mac_dev_init(mac_devices[iter]);
            mac_dev_enable(mac_devices[iter]);
        }
    }

    phy_drivers_init();

    for (iter = 0; iter < CFE_NUM_OF_PORTS; iter++)
    {
        if (phy_devices[iter])
        {
            if (phy_dev_init(phy_devices[iter]))
            {
                phy_dev_del(phy_devices[iter]);
                phy_devices[iter] = NULL;
            }
        }
    }

    return 0;
}

static void mac_set_cfg_by_phy(uint32_t port)
{
    mac_dev_t *mac_dev = mac_devices[port];
    phy_dev_t *phy_dev = phy_devices[port];
    mac_cfg_t mac_cfg = {};

    if (!mac_dev || !phy_dev)
        return;

    mac_dev_disable(mac_dev);
    mac_dev_cfg_get(mac_dev, &mac_cfg);

    if (phy_dev->speed == PHY_SPEED_10)
        mac_cfg.speed = MAC_SPEED_10;
    else if (phy_dev->speed == PHY_SPEED_100)
        mac_cfg.speed = MAC_SPEED_100;
    else if (phy_dev->speed == PHY_SPEED_1000)
        mac_cfg.speed = MAC_SPEED_1000;
    else if (phy_dev->speed == PHY_SPEED_2500)
        mac_cfg.speed = MAC_SPEED_2500;
    else if (phy_dev->speed == PHY_SPEED_5000)
        mac_cfg.speed = MAC_SPEED_5000;
    else if (phy_dev->speed == PHY_SPEED_10000)
        mac_cfg.speed = MAC_SPEED_10000;
    else
        mac_cfg.speed = MAC_SPEED_UNKNOWN;

    mac_cfg.duplex = phy_dev->duplex == PHY_DUPLEX_FULL ? MAC_DUPLEX_FULL : MAC_DUPLEX_HALF;

    mac_dev_cfg_set(mac_dev, &mac_cfg);
    mac_dev_enable(mac_dev);
}
#endif

#ifdef MAC_LPORT
extern void lport_serdes_drv_register(void);
#endif

static int internal_open(bcmenet_sc_t *sc)
{
    int rc;

    ACCESS_LOG_ENABLE_SET(1);

#ifdef MAC_LPORT
    rc = pmc_lport_init();
    if (rc)
    {
        printf("pmc lport init failed\n");
        goto exit;
    }
#endif

#ifndef CONFIG_BRCM_IKOS
    rc = pmc_xrdp_init();
    if (rc)
    {
        printf("pmc xrdp init failed\n");
        goto exit;
    }
#endif
    rc = parse_board_params();
    if (rc)
    {
        printf("Parse board params failed\n");
        goto exit;
    }

    rc = xrdp_init();
    if (rc)
    {
        printf("XRDP init failed\n");
        goto exit;
    }

    ACCESS_LOG_ENABLE_SET(0);

#ifdef MAC_LPORT
    lport_serdes_drv_register();
#endif

#ifndef CONFIG_BRCM_IKOS
    rc = phy_mac_init();
    if (rc)
    {
        printf("phy_mac init Failed\n");
        goto exit;
    }
#else
#if defined(_BCM96846_) || defined(_BCM96856_)
    /* Configure UNIMAC loopback */
    {
        unimac_rdp_cmd cmd;
        ag_drv_unimac_rdp_cmd_get(4, &cmd);
        cmd.lcl_loop_ena = 1;
        cmd.rx_ena = 1;
        cmd.tx_ena = 1;
        ag_drv_unimac_rdp_cmd_set(4, &cmd);
    }
#endif
#endif
    printf("%s: Done\n", __FUNCTION__);

exit:
    return rc;
}

bcmenet_sc_t *g_sc = NULL;

/** Functions. **/
static void bcm6xxx_impl3_ether_probe( cfe_driver_t * drv,    unsigned long probe_a,
                                unsigned long probe_b, void * probe_ptr )
{
    bcmenet_sc_t *sc;
    char descr[100];

    sc = (bcmenet_sc_t *)KMALLOC(sizeof(bcmenet_sc_t), 0);
    if (!sc)
    {
        printf("Failed to allocate sc memory\n");
        return;
    }

    memset(sc, 0, sizeof(bcmenet_sc_t));
    sc->active_tx_port = CFE_NUM_OF_PORTS;
    sc->last_port_received_from = CFE_NUM_OF_PORTS;
    if (internal_open(sc))
    {
        printf("Failed to initilaize enet hw\n");
        return;
    }

    g_sc = sc;
    sprintf(descr, "%s eth %d", drv->drv_description, probe_a);
    cfe_attach(drv, sc, NULL, descr);
}

static void bcm63xx_impl3_ether_poll(cfe_devctx_t *ctx, int64_t ticks);

static int bcm63xx_impl3_ether_open(cfe_devctx_t *ctx)
{
    int rc;

    ACCESS_LOG_ENABLE_SET(1);

    g_sc->ctx = ctx;
    rc = cpu_ring_and_scheduler_init();

    if (rc)
        printf("cpu ring init Failed\n");

#ifdef CONFIG_BRCM_IKOS
    bcm63xx_impl3_ether_poll(ctx, 0);
#endif

    ACCESS_LOG_ENABLE_SET(0);

    return rc;
}

static int dump_is_enambled = 0;
static int compare_is_enabled = 0;
static uint8_t saved_buffer[2000] = {};
static int is_first = 1;

static void dump_packet(uint8_t *buff, uint32_t size)
{
    int i, j = 0;
    for (i = 0; i < size; i += j)
    {
        printf("0x%08lx:", (unsigned long)buff + i);
        for (j = 0; j <16 && ((i+j) < size); j++)
        {
            printf(" %02x", buff[i+j]);
        }
        printf("\n");
    }
    printf("Dump done ===================\n");
}

static int bcm63xx_impl3_ether_read(cfe_devctx_t *ctx, iocb_buffer_t *buffer)
{
    CPU_RX_PARAMS rx_params;
    int rc;
    bcmenet_sc_t *sc = (bcmenet_sc_t *)ctx->dev_softc;

    rx_params.data_ptr = (void *)buffer->buf_ptr;

    rc = rdp_cpu_ring_read_packet_copy(0, &rx_params);
    if (rc)
    {
        if (rc != BDMF_ERR_NO_MORE)
            printf("Failed to read packet from ring (rc) %d\n", rc);
        buffer->buf_retlen = 0;
        return rc;
    }

    if(rx_params.src_bridge_port != sc->active_tx_port)
    {
        if (sc->last_port_received_from != rx_params.src_bridge_port)
        {
            if (rx_params.src_bridge_port >= CFE_NUM_OF_PORTS)
            {
                sc->dropped_pkts[CFE_NUM_OF_PORTS]++;
            }
            else
            {
                sc->dropped_pkts[rx_params.src_bridge_port]++;
                if (sc->last_port_received_from != NO_AUTO_PORT)
                {
                    sc->last_port_received_from = rx_params.src_bridge_port;
                }
            }
            buffer->buf_retlen = 0;
            return BDMF_ERR_NOT_CONNECTED;
        }
        sc->active_tx_port = rx_params.src_bridge_port;
    }

    if (dump_is_enambled)
    {
        printf("Dumping recieved packet port %d size %d\n", sc->active_tx_port, rx_params.packet_size);
        dump_packet(rx_params.data_ptr, rx_params.packet_size);
    }

    if (!rx_params.packet_size)
    {
        printf("Zero length packet received \n");
        sc->dropped_pkts[rx_params.src_bridge_port]++;
        return -1;
    }

    sc->rx_pkts[rx_params.src_bridge_port]++;

    buffer->buf_retlen = rx_params.packet_size;

    if (compare_is_enabled)
    {
        if (memcmp(data, rx_params.data_ptr, rx_params.packet_size))
        {
            printf("Recieved packet:\n");
            dump_packet(rx_params.data_ptr, rx_params.packet_size);
            printf("is differ from previos:\n");
            dump_packet(&saved_buffer[0], rx_params.packet_size);
        }
        else
        {
            printf("Recieved packet %d bytes Equal data[0]=%d!!!\n", rx_params.packet_size, *(uint8_t*)rx_params.data_ptr);
        }
    }

    return 0;
}

static int bcm63xx_impl3_ether_inpstat(cfe_devctx_t *ctx,iocb_inpstat_t *inpstat)
{
    bcmenet_sc_t *sc = (bcmenet_sc_t *)ctx->dev_softc;
        // inp_status == 1 -> data available
    if (sc->active_tx_port != CFE_NUM_OF_PORTS)
        inpstat->inp_status = 1;
    else
        inpstat->inp_status = 0;
    return 0;
}


static int bcm63xx_impl3_ether_write(cfe_devctx_t *ctx, iocb_buffer_t *buffer)
{
    bcmenet_sc_t *sc = (bcmenet_sc_t *)ctx->dev_softc;
    bdmf_error_t rc;
#ifndef CPU_TX_RING_COPY
    RDD_CPU_TX_DESCRIPTOR_DTS cpu_tx_descriptor;
    pbuf_t pbuf;
#endif
#ifdef CONFIG_BRCM_IKOS
    uint8_t bcast_addr[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
#endif

    if (sc->active_tx_port == CFE_NUM_OF_PORTS)
    {
        printf("No active ports\n");
        return -1;
    }

#ifdef CONFIG_BRCM_IKOS

    /*Skip ARP*/
    if(memcmp(buffer->buf_ptr, bcast_addr, 6) == 0)
        return 0;

#endif

#ifdef CPU_TX_RING_COPY

#ifdef _BCM96858_
    if ((sc->active_tx_port == 0) || (sc->active_tx_port == 4))
        rc = cpu_tx_ring_copy_and_transmit(buffer->buf_ptr, buffer->buf_length, 4 - sc->active_tx_port);
    else
#endif
        rc = cpu_tx_ring_copy_and_transmit(buffer->buf_ptr, buffer->buf_length, sc->active_tx_port);
    sc->tx_pkts[sc->active_tx_port]++;
#else

    /* XLMAC layer configured to pad packets
     no need for special hanlde */
    pbuf.length = buffer->buf_length;
    pbuf.offset = 0;
    pbuf.data = buffer->buf_ptr;
    pbuf.abs_flag = 0;
    pbuf.sysb = NULL;

    rc = drv_fpm_alloc_buffer(pbuf.length, &(pbuf.fpm_bn));
    if (rc)
    {
        printf("%s: Failed to allocate buffer (%d)\n", __FUNCTION__, rc);
        return rc;
    }

    drv_fpm_copy_from_host_buffer(pbuf.data, pbuf.fpm_bn & 0xffff, pbuf.length, pbuf.offset);
   
    memset(&cpu_tx_descriptor, 0, sizeof(RDD_CPU_TX_DESCRIPTOR_DTS));

    cpu_tx_descriptor.first_level_q = QM_QUEUE_INDEX_DS_FIRST + sc->active_tx_port;

    rdd_cpu_tx_set_packet_descriptor(NULL, &pbuf, &cpu_tx_descriptor);

    sc->tx_pkts[sc->active_tx_port]++;
    rc = drv_qm_cpu_tx((uint32_t *)&cpu_tx_descriptor, QM_QUEUE_CPU_TX_EGRESS, 0, 0);

#endif  /* #ifdef CPU_TX_RING_COPY */

    if (rc)
    {
        printf("%s: Failed to send packet len %d: error = %d\n", __FUNCTION__, buffer->buf_length, rc);
    }

    return rc;
}

static int bcm63xx_impl3_ether_ioctl(cfe_devctx_t *ctx,iocb_buffer_t *buffer)
{
    bcmenet_sc_t *sc = (bcmenet_sc_t *)ctx->dev_softc;

    switch ((int)buffer->buf_ioctlcmd)
    {
    case IOCTL_ETHER_GETHWADDR:
        memcpy(buffer->buf_ptr, sc->hwaddr, sizeof(sc->hwaddr));
        break;

    case IOCTL_ETHER_SETHWADDR:
        memcpy(sc->hwaddr, buffer->buf_ptr, sizeof(sc->hwaddr));
        break;

    default:
        return -1;
    }

    return 0;
}

static int bcm63xx_impl3_ether_close(cfe_devctx_t *ctx)
{
    /*TODO: call to pmc_lport_uninit and pmc_xrdp_uninit*/
    return 0;
}

static int8_t find_active_port(bcmenet_sc_t *sc)
{
    int i;

    for (i = 0; i < CFE_NUM_OF_PORTS && !(phy_devices[i] && phy_devices[i]->link); ++i);

    return i;
}

int get_cfe_net_stats(int do_clean);
static void bcm63xx_impl3_ether_poll(cfe_devctx_t *ctx, int64_t ticks)
{
    bcmenet_sc_t *sc = (bcmenet_sc_t *)ctx->dev_softc;
    int i, link;
#if defined(CONFIG_BRCM_IKOS) && ( defined(_BCM96846_) || defined(_BCM96856_))
    static int do_test = 2;
#endif

    for (i = 0; i< CFE_NUM_OF_PORTS; ++i)
    {
        /* Skip not working ports */
        if (!phy_devices[i])
            continue;

        link = phy_devices[i]->link;
#ifndef CONFIG_BRCM_IKOS
        phy_dev_read_status(phy_devices[i]);
#else
        if (i == 4)
            phy_devices[i]->link = 1;
#endif

        /* skip port if status was not changed */
        if (phy_devices[i]->link == link)
            continue;

        printf("Port %d: ", i);
#ifndef CONFIG_BRCM_IKOS
        phy_dev_print_status(phy_devices[i]);
#else
        printf("Conected\n");
#endif

        if (phy_devices[i]->link)
        {
            if (sc->active_tx_port == CFE_NUM_OF_PORTS)
            {
                sc->active_tx_port = i;
                printf("Setting as Active\n");
            }
#ifndef CONFIG_BRCM_IKOS
            mac_set_cfg_by_phy(i);
#endif
        }
        else
        {
            if (sc->active_tx_port == i)
            {
                sc->active_tx_port = find_active_port(sc);
                if (sc->active_tx_port == CFE_NUM_OF_PORTS)
                    printf("No active port is set\n", sc->active_tx_port);
                else
                    printf("Port %d set as Active\n", sc->active_tx_port);
            }
        }
    }
#if defined(CONFIG_BRCM_IKOS) && ( defined(_BCM96846_) || defined(_BCM96856_))
    if (do_test == 0)
    {
        run_test(4, 256, 5);
    }
    else
    {
        do_test--;
    }

    /* Debug */
    {
        uint32_t qm_good_lvl1_pkts = 0, qm_good_lvl1_bytes = 0;
        bbh_tx_debug_counters bbh_tx_0 = {};
        qm_debug_counters qm_dbg_counters = {};
        uint32_t bbh_tx_ddr_byte = 0;
        uint32_t bbh_tx_unified_ddr_pkt = 0;
        uint32_t bbh_tx_unified_ddr_byte = 0;
        bbh_rx_pm_counters rx_pm_counters = {};
        uint32_t unimac_grpkt_ok = 0;
        uint32_t unimac_grpkt = 0;
        uint32_t unimac_grbytes = 0;

        uint32_t unimac_gtpkt_ok = 0;
        uint32_t unimac_gtpkt = 0;
        uint32_t unimac_gtbytes = 0;

        ag_drv_qm_good_lvl1_pkts_cnt_get(&qm_good_lvl1_pkts);
        ag_drv_qm_good_lvl1_bytes_cnt_get(&qm_good_lvl1_bytes);
        ag_drv_qm_debug_counters_get(&qm_dbg_counters);
        ag_drv_bbh_tx_debug_counters_get(0, &bbh_tx_0);
        ag_drv_bbh_tx_debug_counters_ddrbyte_get(0, &bbh_tx_ddr_byte);
        ag_drv_bbh_tx_debug_counters_unifiedpkt_get(0, 0, &bbh_tx_unified_ddr_pkt);
        ag_drv_bbh_tx_debug_counters_unifiedbyte_get(0, 0, &bbh_tx_unified_ddr_byte);
        ag_drv_bbh_rx_pm_counters_get(4, &rx_pm_counters);

        ag_drv_unimac_mib_grpok_get(sc->active_tx_port, &unimac_grpkt_ok);
        ag_drv_unimac_mib_grpkt_get(sc->active_tx_port, &unimac_grpkt);
        ag_drv_unimac_mib_grbyt_get(sc->active_tx_port, &unimac_grbytes);

        ag_drv_unimac_mib_gtpok_get(sc->active_tx_port, &unimac_gtpkt_ok);
        ag_drv_unimac_mib_gtpkt_get(sc->active_tx_port, &unimac_gtpkt);
        ag_drv_unimac_mib_gtbyt_get(sc->active_tx_port, &unimac_gtbytes);
        printf("qm_good_lvl1_pkts[0x%x]  qm_good_lvl1_bytes[0x%x] qm_dbg_counters[0x%x] bbh_tx_0[0x%x] bbh_tx_ddr_byte[0x%x] \
	bbh_tx_unified_ddr_pkt[0x%x] bbh_tx_unified_ddr_byte[0x%x] rx_pm_counters[0x%x] unimac_grpkt_ok[0x%x] unimac_grpkt[0x%x] unimac_gtpkt_ok[0x%x] unimac_gtpkt[0x%x] unimac_gtbytes[0x%x]\n",
	qm_good_lvl1_pkts,
	qm_good_lvl1_bytes,
	qm_dbg_counters,
	bbh_tx_0,
	bbh_tx_ddr_byte,
	bbh_tx_unified_ddr_pkt,
	bbh_tx_unified_ddr_byte,
	rx_pm_counters,
	unimac_grpkt_ok,
	unimac_grpkt,
	unimac_gtpkt_ok,
	unimac_gtpkt,
	unimac_gtbytes
	);
    }
#endif
}


int cfe_set_dump_enable(int is_enable)
{
    dump_is_enambled = is_enable;
    return 0;
}

int cfe_set_compare_enable(int is_enable)
{
    compare_is_enabled = is_enable;
    if (!is_enable)
        is_first = 1;
    return 0;
}

int send_dummy_packet(uint8_t port_id, uint32_t packet_size, int number_of_packets, uint8_t is_random)
{
    iocb_buffer_t bb_tx;
    int rc;
    uint32_t random_size = packet_size;
    int i;
    bcmenet_sc_t *sc = g_sc;
    uint8_t current_port = sc->active_tx_port;

    if (!sc)
        return -1;

    if (packet_size > 1536)
    {
        printf("Packet request is too large %d [MAX 1536]\n", packet_size);
        return -2;
    }

    sc->active_tx_port = port_id;

    for (i = 0; i < number_of_packets; i++)
    {
        if (is_random)
            random_size = packet_size - (i%50);

        printf("Going to send packet %d size %d bytes to port %d\n", i, random_size, port_id);

        bb_tx.buf_ptr = data;
        bb_tx.buf_length = random_size;

        rc = bcm63xx_impl3_ether_write(sc->ctx, &bb_tx);
        if (rc)
            printf("Failed to send dummy buffer\n");
    }

    sc->active_tx_port = current_port;
    return 0;
}

void run_test(uint8_t port_id, uint32_t packet_size, int inter)
{
    int i;
    int rc = 0;
    uint8_t rx_data[1536] = {};
    iocb_buffer_t bb_rx;
    bcmenet_sc_t *sc = g_sc;

    cfe_set_compare_enable(1);
    is_first = 0;
    for (i = 0; i < inter; i++)
    {
        data[0] = i;
        send_dummy_packet(port_id, packet_size, 1, 0);
        printf(" packet %d sent \n", i);
    }

    for (i = 0; i < inter; i++)
    {
        data[0] = i;
        bb_rx.buf_ptr = rx_data;
        bb_rx.buf_length = 1536;
        rc = bcm63xx_impl3_ether_read(sc->ctx ,&bb_rx);
        if(rc)
            {
                printf("no pkt index %d \n", i);
            }
    }
    cfe_set_compare_enable(0);
}

int get_cfe_net_stats(int do_clean)
{
    bcmenet_sc_t *sc = g_sc;
    int i;

    printf("|====================================================================================================|\n");
    printf("|Port|   State  |  Phy   |  Bus   | Addr |   Speed   | Duplex | RX dropped | TX packets | RX packets |\n");
    printf("| ID |(*)Active |        |        |      |           |        |            |            |            |\n");
    printf("|====================================================================================================|\n");

    for (i = 0; i< CFE_NUM_OF_PORTS; i++)
    {
        printf("| %d  ", i);
        if (!phy_devices[i])
        {
            printf("|          |        |        |      |           |        |            |            |            |\n");
            continue;
        }

        printf("| %s", (sc->active_tx_port != i) ? "   ":"(*)");
        if (phy_devices[i]->link)
            printf(" Up   ");
        else
            printf(" Down ");
        printf("| %6s ", phy_devices[i]->phy_drv->name);
        printf("| %6s ", phy_dev_mii_type_to_str(phy_devices[i]->mii_type));
        printf("| 0x%2x ", phy_devices[i]->addr);
        printf("| %9s ", phy_devices[i]->speed == PHY_SPEED_UNKNOWN ? "" : phy_dev_speed_to_str(phy_devices[i]->speed));
        printf("| %6s ", phy_devices[i]->duplex == PHY_DUPLEX_UNKNOWN ? "" : phy_dev_duplex_to_str(phy_devices[i]->duplex));
        printf("| %10d ", sc->dropped_pkts[i]);
        printf("| %10d ", sc->tx_pkts[i]);
        printf("| %10d |\n", sc->rx_pkts[i]);
        if (do_clean)
        {
            sc->dropped_pkts[i] = sc->tx_pkts[i] = sc->rx_pkts[i] = 0;
        }
    }
    printf("|====================================================================================================|\n");
    printf("Dropped packets from Invalid port %d\n", sc->dropped_pkts[CFE_NUM_OF_PORTS]);
    if (do_clean)
        sc->dropped_pkts[CFE_NUM_OF_PORTS] = 0;

    return 0;
}

int cfe_eth_set_port(uint8_t port)
{
    bcmenet_sc_t *sc = g_sc;

    if (phy_devices[port] && phy_devices[port]->link)
    {
        sc->last_port_received_from = NO_AUTO_PORT;
        sc->active_tx_port = port;
        return 0;
    }

    printf("The requested port is not up\n");
    return -1;
}

void bcm_ethsw_phy_write_reg(int PhyID,uint16_t address, uint16_t value)
{
#ifdef MAC_LPORT
    lport_mdio22_wr(PhyID, address, value);
#else
    /* TODO: */
#endif
}

uint16_t bcm_ethsw_phy_read_reg(int PhyID,int address)
{
    uint16_t value = 0;

#ifdef MAC_LPORT
    lport_mdio22_rd(PhyID, address, &value);
#else
    /* TODO: */
#endif

    return value;
}

const static cfe_devdisp_t bcm63xx_ether_dispatch = {
    bcm63xx_impl3_ether_open,
    bcm63xx_impl3_ether_read,
    bcm63xx_impl3_ether_inpstat,
    bcm63xx_impl3_ether_write,
    bcm63xx_impl3_ether_ioctl,
    bcm63xx_impl3_ether_close,
    bcm63xx_impl3_ether_poll,
    NULL
};

const cfe_driver_t bcm6xxx_impl3_enet = {
    "BCM63xx Ethernet",
    "eth",
    CFE_DEV_NETWORK,
    &bcm63xx_ether_dispatch,
    bcm6xxx_impl3_ether_probe
};

