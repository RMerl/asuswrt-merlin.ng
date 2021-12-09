/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
*
*    Copyright (c) 2013 Broadcom
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


#ifndef _RDPA_QOS_MAPPER_H_
#define _RDPA_QOS_MAPPER_H_

/** \defgroup pkt_mapper Packet Based Mapping Tables
 * Packet based mapping tables supports multiple mapping tables of the following kinds:\n
 * - DSCP to PBIT
 * - PBIT to queue
 * - Traffic class to queue
 * - PBIT to GEM
 * @{
 */
#if defined(XRDP) || defined(BCM_XRDP)
#define RDPA_DSCP_TO_PBIT_MAX_TABLES        4  /*< Max number of DSCP-to-PBIT mapping tables. */
#define RDPA_PBIT_TO_PRTY_MAX_TABLES        64 /*< Max number of PBIT-to-PRTY mapping tables. */
#define RDPA_DS_TC_TO_QUEUE_ID_MAX_TABLES   8  /*< Max number of DS TC-to-PRTY mapping tables. */
#define RDPA_US_TC_TO_QUEUE_ID_MAX_TABLES   8  /*< Max number of US TC-to-PRTY mapping tables. */
#define RDPA_TC_TO_QUEUE_ID_MAX_TABLES      64 /*< Max number of TC-to-QM_QUEUE mapping tables. */
#else
#define RDPA_DSCP_TO_PBIT_MAX_TABLES        8  /*< Max number of DSCP-to-PBIT mapping tables. */
#define RDPA_PBIT_TO_PRTY_MAX_TABLES        16  /*< Max number of PBIT-to-PRTY mapping tables. */
#define RDPA_DS_TC_TO_QUEUE_ID_MAX_TABLES   8  /*< Max number of DS TC-to-PRTY mapping tables. */
#define RDPA_US_TC_TO_QUEUE_ID_MAX_TABLES   8  /*< Max number of US TC-to-PRTY mapping tables. */
#endif
/** \defgroup dscp_to_pbit DSCP to PBIT Mapper
 * \ingroup pkt_mapper
 */

/** \defgroup pbit_to_queue PBIT to Queue Mapper
 * \ingroup pkt_mapper
 */

/** \defgroup tc_to_queue Traffic Class to Queue Mapper
 * \ingroup pkt_mapper
 */

/** \defgroup pbit_to_gem PBIT to GEM Mapper
 * \ingroup pkt_mapper
 */

/** @} end of pkt_mapper Doxygen group */

#endif /* _RDPA_QOS_MAPPER_H_ */
