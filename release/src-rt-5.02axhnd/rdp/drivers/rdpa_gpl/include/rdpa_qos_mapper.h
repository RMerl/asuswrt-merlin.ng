/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
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
#ifndef XRDP
#define RDPA_DSCP_TO_PBIT_MAX_TABLES        8  /*< Max number of DSCP-to-PBIT mapping tables. */
#define RDPA_PBIT_TO_PRTY_MAX_TABLES        16  /*< Max number of PBIT-to-PRTY mapping tables. */
#define RDPA_DS_TC_TO_QUEUE_ID_MAX_TABLES   8  /*< Max number of DS TC-to-PRTY mapping tables. */
#define RDPA_US_TC_TO_QUEUE_ID_MAX_TABLES   8  /*< Max number of US TC-to-PRTY mapping tables. */
#else
#define RDPA_DSCP_TO_PBIT_MAX_TABLES        4  /*< Max number of DSCP-to-PBIT mapping tables. */
#define RDPA_PBIT_TO_PRTY_MAX_TABLES        64 /*< Max number of PBIT-to-PRTY mapping tables. */
#define RDPA_DS_TC_TO_QUEUE_ID_MAX_TABLES   8  /*< Max number of DS TC-to-PRTY mapping tables. */
#define RDPA_US_TC_TO_QUEUE_ID_MAX_TABLES   8  /*< Max number of US TC-to-PRTY mapping tables. */
#define RDPA_TC_TO_QUEUE_ID_MAX_TABLES      64 /*< Max number of TC-to-QM_QUEUE mapping tables. */
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
