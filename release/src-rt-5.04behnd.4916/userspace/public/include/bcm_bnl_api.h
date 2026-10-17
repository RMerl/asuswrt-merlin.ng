/***********************************************************************
 *
 * Copyright (c) 2024  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2024:DUAL/GPL:standard
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
 *
 ************************************************************************/

#ifndef _BCM_BNL_API_H_
#define _BCM_BNL_API_H_

#include "bcm_bnl_ioctl.h"

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_init
 * Description  : init function createas a profile, and associates it to the
 *                calling process/application via  cmd_fd descriptor.
 *                when application closed/killed profile is deleted
 * Parameters   :
 *  bnl_obj     : on success bnl_obj holds valid profile_id and associated
 *                interfaces to bnl driver(cmd_fd, queues etc..)
 *  cfg         : profile config options
 * Return value : 0 on success and -ve error code on Failure
 *              : Caller MUST check the return value. 
 *------------------------------------------------------------------------------
 */
int bcm_bnl_init(bnlctl_profile_t *bnl_obj, bnlctl_profile_cfg_t *cfg);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_uninit
 * Description  : deletes a profile, and any associated state 
 * Parameters   :
 *  bnl_obj     : bnl_obj holds valid profile_id
 * Return value : 0 on success and -ve error code on Failure
 *------------------------------------------------------------------------------
 */
int bcm_bnl_uninit(bnlctl_profile_t *bnl_obj);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_profile_config_set
 * Description  : update an existing configuration of a specifc profile
 * Parameters   :
 *  cfg         : config to be updated and asscoaited new values
 *                ex:bin config, timeout etc..
 * Return value : 0 on success and -ve error code on Failure
 *------------------------------------------------------------------------------
 */
int bcm_bnl_profile_config_set(bnlctl_profile_cfg_t *cfg);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_profile_config_get
 * Description  : get current values of specific configuration of profile
 * Parameters   :
 *  cfg         : on success holds the config requested 
                  ex:bin config, timeout etc..
 * Return value : 0 on success and -ve error code on Failure
 *------------------------------------------------------------------------------
 */
int bcm_bnl_profile_config_get(bnlctl_profile_cfg_t *cfg);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_profile_status
 * Description  : Get profile statst like number of active streams, failures etc..
 * Parameters   :
 *  bnl_obj     : bnl_obj holds valid profile_id
 *  stats       : on success holds the profile stats 
 * Return value : 0 on success and -ve error code on Failure
 *------------------------------------------------------------------------------
 */
int bcm_bnl_profile_status(bnlctl_profile_t *bnl_obj, bnlctl_profile_stats_t *stats);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_stream_monitor_start
 * Description  : Start BNL monintor of an active connection using 5-tuple as key
 * Parameters   :
 *  bnl_obj     : bnl_obj holds valid profile_id
 *  cfg         : IN: 5-tuple and bin Id's(optional) 
 *              : Out: on Success holds a valid stream_id and associated bin configuration
 * Return value : 0 on success and -ve error code on Failure
 *------------------------------------------------------------------------------
 */
int bcm_bnl_stream_monitor_start(bnlctl_profile_t *bnl_obj, bnlctl_stream_start_cfg_t *cfg);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_stream_monitor_stop
 * Description  : Stop BNL monintoring of an active connection using stream_id
 * Parameters   :
 *  bnl_obj     : bnl_obj holds valid profile_id
 *  cfg         : IN: stream_id to be stopped
 * Return value : 0 on success and -ve error code on Failure
 *------------------------------------------------------------------------------
 */
int bcm_bnl_stream_monitor_stop(bnlctl_profile_t *bnl_obj, bnlctl_stream_stop_cfg_t *cfg);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_stream_stats_get
 * Description  : Get BNL stream stats like min/max/avg RTT, histogram etc..
 *                for a specific stream_id 
 * Parameters   :
 *  bnl_obj     : bnl_obj holds valid profile_id
 *  stats       : IN: stream_id to be stopped
                : Out: on Success holds stats assocated with stream_id
 * Return value : 0 on success and -ve error code on Failure
 *------------------------------------------------------------------------------
 */
int bcm_bnl_stream_stats_get(bnlctl_profile_t *bnl_obj, bnlctl_stream_stats_t *stats);


/*
 *------------------------------------------------------------------------------
 * Function     : bnl_calc_stddev
 * Description  : helper function to calculate standard deviation using sum, sumof sqaures
 *                instead of  full set of values
 *                
 * Parameters   :
 *  count:      : Total number of values in the set
 *  sum         : sum of values in the set
 *  sum_squares : sum of square of each value in the set 
 * Return value : vlaue of standard deviation
 *------------------------------------------------------------------------------
 */
double bnl_calc_stddev(double sum, double sum_squares, int count);


/* **** below functions are used for CLI style application mainly
 *  for initial development/debugging 
 */

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_profile_add
 * Description  : creates a profile in driver but is not asscoaited to any application
 *                used mainly for CLI kind of testing. Profile has to be explictly 
 *                deleted when it's no longer needed
 * Parameters   :
 *  cfg         : IN: profile config options
 *              : Out: on success valid profile_id 
 * Return value : 0 on success and -ve error code on Failure
 *              : Caller MUST check the return value. 
 *------------------------------------------------------------------------------
 */
int bcm_bnl_profile_add(bnlctl_profile_cfg_t *cfg);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_profile_delete
 * Description  : Delete a profile and it's associated state in driver
 * Parameters   :
 *  cfg         : IN: valid profile_id
 * Return value : 0 on success and -ve error code on Failure
 *------------------------------------------------------------------------------
 */
int bcm_bnl_profile_delete(bnlctl_profile_cfg_t *cfg);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_stream_dump
 * Description  : print infomation about a specifc stream_id to console,
 *                like stats and associated metadata(5-tuple's, bin limits etc..)
 * Parameters   :
 *  bnl_obj     : bnl_obj holds valid profile_id
 *  stream_id   : valid stream_id
 * Return value : 0 on success and -ve error code on Failure
 *------------------------------------------------------------------------------
 */
int bcm_bnl_stream_dump(bnlctl_profile_t *bnl_obj, uint16_t stream_id);

/*
 *------------------------------------------------------------------------------
 * Function     : bcm_bnl_stream_dump
 * Description  : print infomation about all active stream_id's of a profile to console,
 *                like stats and associated metadata(5-tuple's, bin limits etc..)
 * Parameters   :
 *  bnl_obj     : bnl_obj holds valid profile_id
 * Return value : 0 on success and -ve error code on Failure
 *------------------------------------------------------------------------------
 */
int bcm_bnl_stream_table_dump(bnlctl_profile_t *bnl_obj);

#endif /* _BCM_BNL_API_H_ */
