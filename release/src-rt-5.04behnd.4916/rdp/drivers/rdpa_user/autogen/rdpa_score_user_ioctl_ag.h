// <:copyright-BRCM:2013:DUAL/GPL:standard
// 
//    Copyright (c) 2013 Broadcom 
//    All Rights Reserved
// 
// Unless you and Broadcom execute a separate written software license
// agreement governing use of this software, this software is licensed
// to you under the terms of the GNU General Public License version 2
// (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
// with the following added to such license:
// 
//    As a special exception, the copyright holders of this software give
//    you permission to link this software with independent modules, and
//    to copy and distribute the resulting executable under terms of your
//    choice, provided that you also meet, for each linked independent
//    module, the terms and conditions of the license of that module.
//    An independent module is a module which is not derived from this
//    software.  The special exception does not apply to any modifications
//    of the software.
// 
// Not withstanding the above, under no circumstances may you combine
// this software in any way with any other Broadcom software provided
// under a license other than the GPL, without Broadcom's express prior
// written consent.
// 
// :>
/*
 * rdpa user ioctl define header file.
 * This header file is generated automatically. Do not edit!
 */
#ifndef _RDPA_SCORE_USER_IOCTL_H_
#define _RDPA_SCORE_USER_IOCTL_H_

enum
{
	RDPA_SCORE_DRV,
	RDPA_SCORE_GET,
	RDPA_SCORE_NSTREAMS_GET,
	RDPA_SCORE_STREAM_GET,
	RDPA_SCORE_STREAM_SET,
	RDPA_SCORE_STREAM_ADD,
	RDPA_SCORE_STREAM_DELETE,
	RDPA_SCORE_STREAM_GET_NEXT,
	RDPA_SCORE_STREAM_FIND,
	RDPA_SCORE_NPROFILES_GET,
	RDPA_SCORE_PROFILE_GET,
	RDPA_SCORE_PROFILE_SET,
	RDPA_SCORE_PROFILE_ADD,
	RDPA_SCORE_PROFILE_DELETE,
	RDPA_SCORE_PROFILE_GET_NEXT,
	RDPA_SCORE_PROFILE_FIND,
	RDPA_SCORE_NSDRS_GET,
	RDPA_SCORE_SCORE_IRQ_COALESCING_NUM_GET,
	RDPA_SCORE_SCORE_IRQ_COALESCING_NUM_SET,
	RDPA_SCORE_SCORE_IRQ_COALESCING_TIMEOUT_GET,
	RDPA_SCORE_SCORE_IRQ_COALESCING_TIMEOUT_SET,
	RDPA_SCORE_SCORE_ENABLE_GET,
	RDPA_SCORE_SCORE_ENABLE_SET,
	RDPA_SCORE_QSTAT_CFG_SET,
};

#endif /* _RDPA_SCORE_USER_IOCTL_H_ */
