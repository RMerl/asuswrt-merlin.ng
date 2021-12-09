/*
* <:copyright-BRCM:2015:DUAL/GPL:standard
* 
*    Copyright (c) 2015 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :>
*/

/*
 *   ==============================================================
 *   Subsystem-level Runner - Host CPU queue allocation
 *   ==============================================================
 */

/*
 *   DHD subsystem Runner - Host CPU queue Ids
 */
#define RDPA_DHD_HELPER_1_CPU_QUEUE          7
#define RDPA_DHD_HELPER_2_CPU_QUEUE          1
#define RDPA_DHD_HELPER_3_CPU_QUEUE          2

/*
 *   XTM subsystem Runner - Host CPU queue Ids
 */
#define RDPA_XTM_CPU_HI_RX_QUEUE_ID          6
#define RDPA_XTM_CPU_LO_RX_QUEUE_ID          5
#define RDPA_XTM_CPU_RX_QUEUE_ID_BASE RDPA_XTM_CPU_LO_RX_QUEUE_ID

/*
 *   BCMENET (Netdev) subsystem Runner - Host CPU queue Ids
 */
#define NETDEV_CPU_RX_QUEUE_ID              3
#define NETDEV_CPU_HI_RX_QUEUE_ID           4 /*must be consequence*/
#define NETDEV_CPU_RX_QUEUE_ID_BASE NETDEV_CPU_RX_QUEUE_ID


/*
 *   SUDD subsystem Runner - Host CPU queue Ids
 */
#define CPU_RX_IPSEC_LOOPBACK_QUEUE_ID      2

/*
 *   ============================================================== 
 */

/*
 *   ==============================================================
 *   Board-level  Runner - Host CPU queue allocation
 *   ==============================================================
 */

/*
 *   Queues allocated at BCM6838, BCM6848 
 */

#if defined(CONFIG_BCM963158)
/*  GPON - OMCI */
#define RDPA_OMCI_CPU_RX_QUEUE_ID  1
/*  GPON - PLOAM */
#define RDPA_PLOAM_CPU_RX_QUEUE_ID 2

/*  EPON - MPCP */
#define RDPA_MPCP_CPU_RX_QUEUE_ID 1
/*  EPON - OAM */
#define RDPA_OAM_CPU_RX_QUEUE_ID  2

#else
/*  GPON - OMCI */
#define RDPA_OMCI_CPU_RX_QUEUE_ID  5
/*  GPON - PLOAM */
#define RDPA_PLOAM_CPU_RX_QUEUE_ID 6

/*  EPON - MPCP */
#define RDPA_MPCP_CPU_RX_QUEUE_ID 5
/*  EPON - OAM */
#define RDPA_OAM_CPU_RX_QUEUE_ID  6

#endif

/*
 *   Queues allocated for GPON can be re-used by EPON since EPON and GPON drivers do not coexist in
 *   BCM6838/48 
 */

/*
 *  BCMENET is used in BCM6838/48 so its queue ids are also in use here 
 */
/*
 *  DHD is supported in BCM6838/48 so its queue ids are also in use here 
 */
/*
 *   ============================================================== 
 *   ============================================================== 
 */
