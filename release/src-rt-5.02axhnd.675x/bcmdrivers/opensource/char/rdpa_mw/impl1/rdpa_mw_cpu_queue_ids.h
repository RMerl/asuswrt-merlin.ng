/*
* <:copyright-BRCM:2015:DUAL/GPL:standard
* 
*    Copyright (c) 2015 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
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
