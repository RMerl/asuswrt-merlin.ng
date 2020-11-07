/************************************************************
 *
 * <:copyright-BRCM:2012:DUAL/GPL:standard
 * 
 *    Copyright (c) 2012 Broadcom 
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
 ************************************************************/

#ifndef __FAP4KE_FFE_H_INCLUDED__
#define __FAP4KE_FFE_H_INCLUDED__

/*
 *******************************************************************************
 * File Name  : fap4ke_ffe.h
 *
 * Description: This file contains the constants and prototypes needed for the
 *              FFE Driver running on the 4ke.
 *
 *******************************************************************************
 */

// Uncomment to pass all packets directly from rx to tx on FAP
//#define PERFORM_ALL_PACKETS_PASS_THROUGH

#define FFE_DRV_ENET             0
#define FFE_DRV_XTMRT            1

/* FFE Driver Prototypes */
fapRet ffeStartClassify(uint32 channel, unsigned char * pBuf, int len, uint32 dmaFlag, int drvType);


#endif  /* defined(__FAP4KE_FFE_H_INCLUDED__) */
