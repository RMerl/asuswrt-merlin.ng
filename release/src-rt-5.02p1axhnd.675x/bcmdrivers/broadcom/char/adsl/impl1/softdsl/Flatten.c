/****************************************************************************
 * 
 * Copyright (c) 2002-2012 Broadcom Corporation 
 * All Rights Reserved 
 *
 * <:label-BRCM:2012:proprietary:standard
 * 
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 * 
 *  Except as expressly set forth in the Authorized License,
 * 
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 * 
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 * 
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :>
 * 
 *****************************************************************************/
/****************************************************************************
 *
 * Flatten.c -- Flatten/Unflatten command/status
 *
 *
 * Copyright (c) 1993-1997 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.95 $
 *
 * $Id: Flatten.c,v 1.95 2007/10/16 18:15:33 tonytran Exp $
 *
 * $Log: Flatten.c,v $
 * Revision 1.95  2007/10/16 18:15:33  tonytran
 * Allow the driver to work with status buffer either in SDRAM or LMEM
 *
 * Revision 1.94  2007/10/12 02:48:08  ilyas
 * Fixed compiler problem for DslDiags
 *
 * Revision 1.93  2007/10/09 20:20:08  ilyas
 * Added PHY configuration command to 6368. VDSL configuration TBD
 *
 * Revision 1.92  2007/08/17 07:20:18  ilyas
 * Removed un-necessary cache invalidation which is already done in DslCoreXface
 *
 * Revision 1.91  2007/08/06 19:45:12  ilyas
 * Made build with DslDiags
 *
 * Revision 1.90  2007/08/06 03:02:55  ilyas
 * Made build with Linux and DslDiags
 *
 * Revision 1.89  2007/07/27 23:02:57  ilyas
 * Convert SDRAM addresses from Host to PHY MIPS (if possible) to make them cacheable for better performance
 *
 * Revision 1.88  2007/07/25 21:30:10  tonytran
 * Final changes to data structure for eye data support for 6368
 *
 * Revision 1.87  2007/07/25 18:07:28  tonytran
 * Use compile flag BCM6368_SRC instead of G993 for 6368 eye data support code
 *
 * Revision 1.86  2007/07/25 01:30:25  tonytran
 * Added eye data support for 6368
 *
 * Revision 1.85  2007/07/03 20:53:26  tonytran
 * missing from earlier check in
 *
 * Revision 1.84  2007/07/03 20:25:57  tonytran
 * Added support for block/afe test
 *
 * Revision 1.83  2007/06/23 01:30:03  tonytran
 * Added a few commands and statuses to support AFE test
 *
 * Revision 1.82  2006/12/22 14:47:01  jboxho
 * FIRE feature implementation: handshake, counters, block interleaving, new retransmit request format
 *
 * Revision 1.81  2006/05/19 23:45:52  yongbing
 * Move some non-time-critial codes/data from Lmem to SDRAM for 24K, PR 30468
 *
 * Revision 1.80  2006/01/05 21:10:37  ilyas
 * Made Diags work with the latest Flatten.c
 *
 * Revision 1.79  2006/01/05 18:28:25  ilyas
 * Fixed to compile for DslDiags
 *
 * Revision 1.78  2006/01/05 14:32:44  jboxho
 * PLN fix
 *
 * Revision 1.77  2006/01/05 04:05:53  dadityan
 * PLN Status
 *
 * Revision 1.76  2005/11/23 00:53:46  ilyas
 * Added parameter to ExecutionDelay test command
 *
 * Revision 1.75  2005/10/19 18:53:13  ilyas
 * Added margin level parameters (spec) to PLNStart command
 *
 * Revision 1.74  2005/10/12 20:59:10  ilyas
 * ADSL driver change: Pass data pointer for long statuses iso data copying
 *
 * Revision 1.73  2005/09/08 21:00:10  ilyas
 * Improved showtime SNR status conversion
 *
 * Revision 1.72  2005/09/08 20:57:39  ilyas
 * For Adsl driver: Fixed address conversion for print data status, ifdef'ed out PHY only code from driver build
 *
 * Revision 1.71  2005/02/11 06:47:09  ilyas
 * Added support for DslOs
 *
 * Revision 1.70  2004/09/28 23:14:54  mprahlad
 * Remove dependence on G992P3 flag, for reporting ChannelResponse (so that ADSL1
 * builds can also report Hlog/HLin)
 *
 * Revision 1.69  2004/09/02 01:27:29  ilyas
 * Fixed parameter passing for SADSL
 *
 * Revision 1.68  2004/06/24 03:12:31  ilyas
 * Use un-cached read pinter in status write
 *
 * Revision 1.67  2004/06/02 22:24:10  ilyas
 * Added ATM counters for G.992.3
 *
 * Revision 1.66  2004/05/20 00:58:23  ilyas
 * Pass G992p3ConnectInfo via status buffer
 *
 * Revision 1.65  2004/05/13 19:14:18  ilyas
 * Added new statuses for ADSL2
 *
 * Revision 1.64  2004/05/01 01:33:37  ilyas
 * Simplified PowerMgr command/status processing
 *
 * Revision 1.63  2004/05/01 01:12:21  ilyas
 * Added power management command and statuses
 *
 * Revision 1.62  2004/03/29 23:06:39  ilyas
 * Added status for BG table update
 *
 * Revision 1.61  2004/02/18 20:46:33  ilyas
 * Fixed macro definition to build with DslDiags
 *
 * Revision 1.60  2004/02/05 03:11:59  ilyas
 * Integrated changes from ADSL driver
 *
 * Revision 1.59  2004/02/04 19:42:24  linyin
 * Support adsl2plus
 *
 * Revision 1.58  2004/02/03 19:14:40  gsyu
 * Support parameter passing for G992P5
 *
 * Revision 1.57  2004/01/19 19:39:06  ilyas
 * #ifdef'ed address conversion for OLR table
 *
 * Revision 1.56  2004/01/17 00:21:53  ilyas
 * Added commands and statuses for OLR
 *
 * Revision 1.55  2003/11/20 01:02:47  yongbing
 * Merge ADSL2 functionalities into Annex A branch
 *
 * Revision 1.54  2003/11/01 01:05:47  linyin
 * Add annexI carrierInfo
 *
 * Revision 1.53  2003/10/22 03:23:32  ilyas
 * Added QuietLine noise
 *
 * Revision 1.52  2003/10/22 03:19:32  ilyas
 * Added QuietLine noise status support
 *
 * Revision 1.51  2003/10/22 01:04:49  ilyas
 * Pass buffer pointer for channel response statuses
 *
 * Revision 1.50  2003/10/16 00:15:20  ilyas
 * Added G992p3 parameters
 *
 * Revision 1.49  2003/08/22 22:40:24  liang
 * The #endif statements in last checkin were in the wrong locations.
 *
 * Revision 1.48  2003/08/12 23:16:00  khp
 * - for Haixiang: added support for ADSL_MARGIN_TWEAK_TEST
 *
 * Revision 1.47  2003/07/18 04:53:46  ilyas
 * Fixed flattening of clEoc messages
 *
 * Revision 1.46  2003/06/07 00:38:36  ilyas
 * Added conditions to compile for DslDiags
 *
 * Revision 1.45  2003/06/07 00:00:08  ilyas
 * Added flattening of command and status for AFE standalone tests
 * Added support for little endian Host MIPS (in ADSL driver) (for WinCE)
 *
 * Revision 1.44  2003/03/10 20:35:09  ilyas
 * Fixed more compiler warnings
 *
 * Revision 1.43  2003/03/10 20:33:01  ilyas
 * Fixed compiler warning
 *
 * Revision 1.42  2003/03/06 01:05:18  ilyas
 * Added support for SetStatusBuffer command
 *
 * Revision 1.41  2003/02/27 06:33:03  ilyas
 * Improved free space checking in command buffer (became a problem with
 * 2 commands SetXmtgain and StartPhy)
 *
 * Revision 1.40  2003/01/11 01:27:07  ilyas
 * Improved checking for available space in status buffer
 *
 * Revision 1.39  2002/12/12 03:12:33  ilyas
 * Fixed kDslOemDataAddrStatus unflatten bug
 *
 * Revision 1.38  2002/10/04 23:23:46  liang
 * Flatten and unflatten AOC exchange rcv/xmt info data for bitswap.
 *
 * Revision 1.37  2002/09/15 04:36:56  ilyas
 * Fixed compiler warning
 *
 * Revision 1.36  2002/09/15 04:27:42  ilyas
 * Fixed copying of showtime counters
 *
 * Revision 1.35  2002/09/13 21:17:12  ilyas
 * Added pointers to version and build string to OEM interface structure
 *
 * Revision 1.34  2002/09/07 01:43:59  ilyas
 * Added support for OEM parameters
 *
 * Revision 1.33  2002/08/02 22:22:08  liang
 * Enable G.lite code when G.dmt annex A is used for G.lite.
 *
 * Revision 1.32  2002/07/11 03:16:48  ilyas
 * Fixed Flatten bug for ShowtimeSNR status
 *
 * Revision 1.31  2002/07/11 01:30:59  ilyas
 * Changed status for ShowtimeMargin reporting
 *
 * Revision 1.30  2002/07/09 19:22:12  ilyas
 * Added support for ShowtimeSNRMargin status
 *
 * Revision 1.29  2002/07/02 01:00:08  ilyas
 * Added support for kDslTestCmd
 *
 * Revision 1.28  2002/06/15 05:18:44  ilyas
 * Support format string from SDRAM in SoftDslPrintf
 *
 * Revision 1.27  2002/05/31 00:55:10  liang
 * Fix Linux OS compiler warning.
 *
 * Revision 1.26  2002/05/30 19:59:40  ilyas
 * Added status for ADSL MIPS exception
 *
 * Revision 1.25  2002/05/17 18:00:27  liang
 * Add codes for Annex A S=1/2.
 *
 * Revision 1.24  2002/04/02 10:06:09  ilyas
 * Added BERT statuses
 *
 * Revision 1.23  2002/04/01 19:06:38  linyin
 * reverse the last checkin
 *
 * Revision 1.21  2002/03/07 22:08:14  georgep
 * Use appropriate variables for annex A, B or C
 *
 * Revision 1.20  2002/02/08 04:47:59  ilyas
 * Completed LOG file support
 *
 * Revision 1.19  2002/01/30 07:19:06  ilyas
 * Moved showtime code to LMEM
 *
 * Revision 1.18  2002/01/28 21:53:42  ilyas
 * Added progress pointer to HostDma handlers
 *
 * Revision 1.17  2002/01/19 01:03:33  ilyas
 * Fixed compiler warning
 *
 * Revision 1.16  2002/01/17 04:44:39  ilyas
 * Moved address adjustment to CommandWrite (was in CommandRead before)
 *
 * Revision 1.15  2002/01/16 19:59:20  ilyas
 * Changed error condition handling
 *
 * Revision 1.14  2002/01/15 22:30:15  ilyas
 * Extended StatusWrite to handle read pointer (updated by the Host MIPS) via
 * uncached memory address
 *
 * Revision 1.13  2002/01/10 07:18:23  ilyas
 * Added status for printf (mainly for ADSL core debugging)
 *
 * Revision 1.12  2002/01/02 19:12:15  liang
 * Make sure number of bytes for BlockByteMove is a multiple of 4.
 *
 * Revision 1.11  2001/12/21 01:53:35  ilyas
 * Fixed compiler warning
 *
 * Revision 1.10  2001/12/21 00:29:17  ilyas
 * Fixed G992CodingParams flatten problem
 *
 * Revision 1.9  2001/12/13 02:38:52  ilyas
 * Added support for G997 and Clear EOC
 *
 * Revision 1.8  2001/11/30 05:56:37  liang
 * Merged top of the branch AnnexBDevelopment onto top of the tree.
 *
 * Revision 1.7  2001/10/09 22:35:14  ilyas
 * Added more ATM statistics and OAM support
 *
 * Revision 1.1.2.3  2001/10/04 00:54:19  liang
 * Flaten/Unflaten G994 xmt & rcv msg, TEQ coef, and PSD info.
 *
 * Revision 1.1.2.2  2001/10/03 01:45:04  liang
 * Merged with codes from main tree (tag SoftDsl_2_18).
 *
 * Revision 1.6  2001/09/21 19:47:05  ilyas
 * Fixed compiler warnings for VxWorks build
 *
 * Revision 1.5  2001/08/30 18:52:23  ilyas
 * Commented out unused code for ADSLCORE_ONLY
 *
 * Revision 1.4  2001/08/29 19:02:58  ilyas
 * Fixed compiling and linking problems for G992 targets
 *
 * Revision 1.3  2001/08/29 02:56:01  ilyas
 * Added tests for flattening/unflatenning command and statuses (dual mode)
 *
 * Revision 1.2  2001/08/08 01:19:17  ilyas
 * Added support for EC coefficients status
 *
 * Revision 1.1  2001/04/24 21:41:21  ilyas
 * Implemented status flattening/unflattaning to transfer statuses between
 * modules asynchronously through the circular buffer
 *
 *****************************************************************************/

#include "SoftDsl.h"
#include "Flatten.h"
#include "BlockUtil.h"
#include "../AdslCoreMap.h"

#include <stdarg.h>

#ifdef G997_1_FRAMER
#include "DslFramer.h"
#endif

#ifndef ADSL_PHY_SUPPORT
#define	ADSL_PHY_SUPPORT(f)		(1)
#endif

#ifndef ADSLCORE_ONLY

Public int FlattenCommand (dslCommandStruct *cmd, uint *dstPtr, uint nAvail)
{
#define	CHECK_CMD_BUF_AVAIL(szReq,szAvail)	if ((int)(szReq) > (int)(szAvail)) return 0;

	uint	*dstPtr0  = dstPtr;
	uint	statAvail = (nAvail >> 2);

	*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->command);
	switch (DSL_COMMAND_CODE(cmd->command)) {
		case kDslStartPhysicalLayerCmd:
#ifdef G992_ATUC
			CHECK_CMD_BUF_AVAIL(27,statAvail-1);
#else
			CHECK_CMD_BUF_AVAIL(16,statAvail-1);
#endif
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.direction);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.modulations);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.minDataRate);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.maxDataRate);
#if	defined(G992P2) || (defined(G992P1_ANNEX_A) && defined(G992P1_ANNEX_A_USED_FOR_G992P2))
			if (ADSL_PHY_SUPPORT(kAdslPhyG992p2Init)) {
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.downstreamMinCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.downstreamMaxCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.upstreamMinCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.upstreamMaxCarr);
			}
#endif
#ifdef	G992P1_ANNEX_A
			if (ADSL_PHY_SUPPORT(kAdslPhyAnnexA)) {
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.downstreamMinCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.downstreamMaxCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.upstreamMinCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.upstreamMaxCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.subChannelInfo);
			}
#endif
#ifdef	G992P1_ANNEX_B
			if (ADSL_PHY_SUPPORT(kAdslPhyAnnexB) || ADSL_PHY_SUPPORT(kAdslPhySADSL)) {
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMinCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMaxCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMinCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMaxCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexB);
			}
#endif
#ifdef	G992_ANNEXC
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMinCarr);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMaxCarr);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMinCarr);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMaxCarr);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexC);
#endif
#ifdef	G992P1_ANNEX_I
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.downstreamMinCarr);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.downstreamMaxCarr);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMinCarr);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMaxCarr);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexI);
#endif
#ifdef	G992P5
			if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p)) {
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.downstreamMinCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.downstreamMaxCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMinCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMaxCarr);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.subChannelInfop5);
			}
#endif
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.features);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.auxFeatures);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.demodCapabilities);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.noiseMargin);
#ifdef G992_ATUC /* last please, for ATU-C only */
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.xmtRSf);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.xmtRS);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.xmtS);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.xmtD);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.rcvRSf);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.rcvRS);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.rcvS);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.rcvD);
#endif
#ifdef G992P3
			if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2)) {
#ifdef FLATTEN_ADDR_ADJUST
				*dstPtr++ = (uint) (ADSL_ENDIAN_CONV_INT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA)));
#else
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA);
#endif
			}
#endif
#ifdef G992P5
			if (ADSL_PHY_SUPPORT(kAdslPhyAdsl2p)) {
#ifdef FLATTEN_ADDR_ADJUST
				*dstPtr++ = (uint) (ADSL_ENDIAN_CONV_INT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA)));
#else
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA);
#endif
			}
#endif
#ifdef G993
			if (ADSL_PHY_SUPPORT(kAdslPhyVdslG993p2) && (cmd->param.dslModeSpec.capabilities.modulations & kG993p2AnnexA)) {
#ifdef FLATTEN_ADDR_ADJUST
				*dstPtr++ = (uint) (ADSL_ENDIAN_CONV_INT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslModeSpec.capabilities.carrierInfoG993p2AnnexA)));
#else
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslModeSpec.capabilities.carrierInfoG993p2AnnexA);
#endif
			}
#endif
			break;

		case kDslDiagSetupCmd:
			CHECK_CMD_BUF_AVAIL(4,statAvail-1);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslDiagSpec.setup);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslDiagSpec.eyeConstIndex1);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslDiagSpec.eyeConstIndex2);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslDiagSpec.logTime);
			break;
			
		case kDslDiagSetupBufDesc:
			CHECK_CMD_BUF_AVAIL(2,statAvail-1);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslDiagBufDesc.descBufAddr));
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslDiagBufDesc.bufCnt);
			break;

		case kDslTestCmd:
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslTestSpec.type);
			if (kDslTestToneSelection == cmd->param.dslTestSpec.type) {
				int	len1, len2;

				len1 = (cmd->param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones + 7) >> 3;
				len2 = (cmd->param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones + 7) >> 3;

				CHECK_CMD_BUF_AVAIL(4+((len1 + len2 + 3) >> 2),statAvail-2);

				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslTestSpec.param.toneSelectSpec.xmtStartTone);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslTestSpec.param.toneSelectSpec.rcvStartTone);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones);

				BlockByteMove ((len1 + 3) & ~0x3, cmd->param.dslTestSpec.param.toneSelectSpec.xmtMap, (char*)dstPtr);
				BlockByteMove ((len2 + 3) & ~0x3, cmd->param.dslTestSpec.param.toneSelectSpec.rcvMap, ((char*)dstPtr) + len1);
				dstPtr += (len1 + len2 + 3) >> 2;
			}
			else if (kDslTestExecuteDelay==cmd->param.dslTestSpec.type) {
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslTestSpec.param.value);
			}
#ifdef ADSL_MARGIN_TWEAK_TEST
			else if (kDslTestMarginTweak == cmd->param.dslTestSpec.type) {
				int	len1;

				len1 = (cmd->param.dslTestSpec.param.marginTweakSpec.numOfCarriers + 3) >> 2;
				CHECK_CMD_BUF_AVAIL(2+len1,statAvail-2);

				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslTestSpec.param.marginTweakSpec.extraPowerRequestQ4dB);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslTestSpec.param.marginTweakSpec.numOfCarriers);
				BlockByteMove (len1 << 2, cmd->param.dslTestSpec.param.marginTweakSpec.marginTweakTableQ4dB, (char*)dstPtr);
				dstPtr += len1;
			}
#endif
			break;

		case kDslSetStatusBufferCmd:
#ifdef FLATTEN_ADDR_ADJUST
			*dstPtr++ = ADSL_ENDIAN_CONV_UINT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslStatusBufSpec.pBuf));
#else
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslStatusBufSpec.pBuf);
#endif
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslStatusBufSpec.bufSize);
			break;

		case kDslDiagFrameHdrCmd:
			CHECK_CMD_BUF_AVAIL(2,statAvail-1);
#ifdef FLATTEN_ADDR_ADJUST
			*dstPtr++ = ADSL_ENDIAN_CONV_UINT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslStatusBufSpec.pBuf));
#else
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslStatusBufSpec.pBuf);
#endif
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslStatusBufSpec.bufSize);
			break;
		case kDslAfeTestCmd:
			CHECK_CMD_BUF_AVAIL(5,statAvail-1);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslAfeTestSpec.type);
#ifdef FLATTEN_ADDR_ADJUST
			*dstPtr++ = (uint) (ADSL_ENDIAN_CONV_INT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslAfeTestSpec.afeParamPtr)));
#else
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslAfeTestSpec.afeParamPtr);
#endif
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslAfeTestSpec.afeParamSize);
#ifdef FLATTEN_ADDR_ADJUST
			*dstPtr++ = (uint) (ADSL_ENDIAN_CONV_INT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslAfeTestSpec.imagePtr)));
#else
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslAfeTestSpec.imagePtr);
#endif
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslAfeTestSpec.imageSize);
			break;

		case kDslAfeTestCmd1:
			CHECK_CMD_BUF_AVAIL(3,statAvail-1);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslAfeTestSpec1.type);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslAfeTestSpec1.param1);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslAfeTestSpec1.param2);
			break;

		case kDslPLNControlCmd:
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslPlnSpec.plnCmd);
			if (kDslPLNControlStart == cmd->param.dslPlnSpec.plnCmd) {
				dstPtr[0] = ADSL_ENDIAN_CONV_INT32(cmd->param.dslPlnSpec.mgnDescreaseLevelPerBin);
				dstPtr[1] = ADSL_ENDIAN_CONV_INT32(cmd->param.dslPlnSpec.mgnDescreaseLevelBand);
				dstPtr += 2;
				
			}
			else if(kDslPLNControlDefineInpBinTable==cmd->param.dslPlnSpec.plnCmd) {
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslPlnSpec.nInpBin);
#ifdef FLATTEN_ADDR_ADJUST
				*dstPtr++ = ADSL_ENDIAN_CONV_UINT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslPlnSpec.inpBinPtr));
#else
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.inpBinPtr);
#endif
			}
			else if (kDslPLNControlDefineItaBinTable==cmd->param.dslPlnSpec.plnCmd) {
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslPlnSpec.nItaBin);
#ifdef FLATTEN_ADDR_ADJUST
				*dstPtr++ = ADSL_ENDIAN_CONV_UINT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslPlnSpec.itaBinPtr));
#else
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.itaBinPtr);
#endif
			}
			else if (kDslINMControlParams==cmd->param.dslPlnSpec.plnCmd) {
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.inmContinueConfig);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.inmInpEqMode);
			}
			else if (kDslINMConfigParams==cmd->param.dslPlnSpec.plnCmd) {
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.inmContinueConfig);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.inmInpEqMode);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.inmIATO);
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.inmIATS);
				if (cmd->param.dslPlnSpec.inmGfastCoCpeSupport)
				{
					*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.inmInpEqScale);
					*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.inmIATScale);
					*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.inmBRGN);
				}
			}
			else if (kDslINMConfigInpEqFormat == cmd->param.dslPlnSpec.plnCmd)
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint) cmd->param.dslPlnSpec.inmInpEqFormat);
			break;

#ifdef G997_1
		case kDslSendEocCommand:
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.value);
			if (cmd->param.value >= kDslClearEocFirstCmd)
				{
				*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslClearEocMsg.msgType);
				if (cmd->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask)
					{
					int	len;

					len = cmd->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
					CHECK_CMD_BUF_AVAIL((len + 3) >> 2,statAvail-3);

					BlockByteMove ((len+3) & ~0x3, cmd->param.dslClearEocMsg.dataPtr, (char*)dstPtr);
					dstPtr += (len + 3) >> 2;
					}
				else
#ifdef FLATTEN_ADDR_ADJUST
					*dstPtr++ = (uint)  (ADSL_ENDIAN_CONV_INT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslClearEocMsg.dataPtr)));
#else
					*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslClearEocMsg.dataPtr);
#endif
				}
			break;
#endif

#ifdef G992P3
		case kDslOLRRequestCmd:
			CHECK_CMD_BUF_AVAIL(6,statAvail-1);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslOLRRequest.msgType);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslOLRRequest.nCarrs);
			BlockShortMoveReverse(4, (void *) cmd->param.dslOLRRequest.L, (void *) dstPtr);
			dstPtr += 2;
			BlockByteMove (4, (void *) cmd->param.dslOLRRequest.B, (void *) dstPtr);
			dstPtr += 1;
#ifdef FLATTEN_ADDR_ADJUST
			*dstPtr++ = (uint) (ADSL_ENDIAN_CONV_INT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslOLRRequest.carrParamPtr)));
#else
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslOLRRequest.carrParamPtr);
#endif
			break;

		case kDslPwrMgrCmd:
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32(cmd->param.dslPwrMsg.msgType);
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslPwrMsg.param.msg.msgLen);
#ifdef FLATTEN_ADDR_ADJUST
			*dstPtr++ = ADSL_ENDIAN_CONV_UINT32(SDRAM_ADDR_TO_ADSL(cmd->param.dslPwrMsg.param.msg.msgData));
#else
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.dslPwrMsg.param.msg.msgData);
#endif
			break;
#endif

		default:
			*dstPtr++ = ADSL_ENDIAN_CONV_INT32((uint)cmd->param.value);
			break;
	}
	return (char*)dstPtr - (char*)dstPtr0;
}

#endif /* ADSLCORE_ONLY */

#if !defined(HOST_ONLY) || defined(SUPPORT_EXT_DSL_BONDING_SLAVE)

#if !defined(WINNT) && !defined(SUPPORT_EXT_DSL_BONDING_SLAVE)
#include "DslCoreXface.h"

static void * SdramHost2Adsl(void *p)
{
	uintptr_t	addr = (uintptr_t) p;
	int		off  = addr - (uintptr_t) adslXfaceData.sdramBaseAddr;

	if (off >= 0)
		p = (void *) ((uintptr_t) &eprol + off);
	return p;
}
#define SDRAM_HOST2ADSL(addr)	SdramHost2Adsl((void*)(addr))
#else
#define SDRAM_HOST2ADSL(addr)	addr
#endif

Public int UnflattenCommand(uint *srcPtr, dslCommandStruct *cmd)
{
	uint	*srcPtr0 = srcPtr;

	cmd->command = *srcPtr++;
	
	switch	(DSL_COMMAND_CODE(cmd->command)) {
		case kDslStartPhysicalLayerCmd:
			cmd->param.dslModeSpec.direction = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.modulations = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.minDataRate = (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.maxDataRate = (ushort) *srcPtr++;
#if	defined(G992P2) || (defined(G992P1_ANNEX_A) && defined(G992P1_ANNEX_A_USED_FOR_G992P2))
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.downstreamMinCarr = (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.downstreamMaxCarr = (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.upstreamMinCarr = (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p2.upstreamMaxCarr = (uchar) *srcPtr++;
#endif
#ifdef	G992P1_ANNEX_A
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.downstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.downstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.upstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1.upstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.subChannelInfo = *srcPtr++;
#endif
#ifdef	G992P1_ANNEX_B
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.downstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexB.upstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexB = *srcPtr++;
#endif
#ifdef	G992_ANNEXC
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.downstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexC.upstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexC = *srcPtr++;
#endif
#ifdef	G992P1_ANNEX_I
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.downstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.downstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p1AnnexI.upstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.subChannelInfoAnnexI = *srcPtr++;
#endif
#ifdef	G992P5
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.downstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.downstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMinCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p5.upstreamMaxCarr = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.subChannelInfop5 = *srcPtr++;
#endif
			cmd->param.dslModeSpec.capabilities.features = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.auxFeatures = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.demodCapabilities = *srcPtr++;
			cmd->param.dslModeSpec.capabilities.noiseMargin = (ushort) *srcPtr++;
#ifdef G992_ATUC /* last please, for ATU-C only */
			cmd->param.dslModeSpec.capabilities.xmtRSf	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.xmtRS	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.xmtS	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.xmtD	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.rcvRSf	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.rcvRS	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.rcvS	= (uchar) *srcPtr++;
			cmd->param.dslModeSpec.capabilities.rcvD	= (uchar) *srcPtr++;
#endif
#ifdef G992P3
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p3AnnexA = (void *) *srcPtr++;
#endif
#ifdef G992P5
			cmd->param.dslModeSpec.capabilities.carrierInfoG992p5AnnexA = (void *) *srcPtr++;
#endif
#ifdef G993
			if (cmd->param.dslModeSpec.capabilities.modulations & kG993p2AnnexA)
				cmd->param.dslModeSpec.capabilities.carrierInfoG993p2AnnexA = (void *) *srcPtr++;
#endif

			break;

		case kDslDiagSetupCmd:
			cmd->param.dslDiagSpec.setup			= *srcPtr++;
			cmd->param.dslDiagSpec.eyeConstIndex1	= *srcPtr++;
			cmd->param.dslDiagSpec.eyeConstIndex2	= *srcPtr++;
			cmd->param.dslDiagSpec.logTime			= *srcPtr++;
			break;

		case kDslDiagSetupBufDesc:
			cmd->param.dslDiagBufDesc.descBufAddr = *srcPtr++;
			cmd->param.dslDiagBufDesc.bufCnt = *srcPtr++;
			break;

		case kDslTestCmd:
			cmd->param.dslTestSpec.type = *srcPtr++;
			if (kDslTestToneSelection == cmd->param.dslTestSpec.type) {
				int	len1, len2;

				cmd->param.dslTestSpec.param.toneSelectSpec.xmtStartTone	= *srcPtr++;
				cmd->param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones	= *srcPtr++;
				cmd->param.dslTestSpec.param.toneSelectSpec.rcvStartTone	= *srcPtr++;
				cmd->param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones	= *srcPtr++;

				len1 = (cmd->param.dslTestSpec.param.toneSelectSpec.xmtNumOfTones + 7) >> 3;
				len2 = (cmd->param.dslTestSpec.param.toneSelectSpec.rcvNumOfTones + 7) >> 3;
				cmd->param.dslTestSpec.param.toneSelectSpec.xmtMap = (uchar *) srcPtr;
				cmd->param.dslTestSpec.param.toneSelectSpec.rcvMap = ((uchar *) srcPtr) + len1;
				srcPtr += (len1 + len2 + 3) >> 2;
			}
			else if (kDslTestExecuteDelay == cmd->param.dslTestSpec.type) {
				cmd->param.dslTestSpec.param.value                          = *srcPtr++;
			}
#ifdef ADSL_MARGIN_TWEAK_TEST
			else if (kDslTestMarginTweak == cmd->param.dslTestSpec.type) {
				cmd->param.dslTestSpec.param.marginTweakSpec.extraPowerRequestQ4dB	= *srcPtr++;
				cmd->param.dslTestSpec.param.marginTweakSpec.numOfCarriers			= *srcPtr++;
				cmd->param.dslTestSpec.param.marginTweakSpec.marginTweakTableQ4dB	= (uchar *) srcPtr;

				srcPtr += (cmd->param.dslTestSpec.param.marginTweakSpec.numOfCarriers + 3) >> 2;
			}
#endif
			break;

		case kDslSetStatusBufferCmd:
			cmd->param.dslStatusBufSpec.pBuf	= (void *) *srcPtr++;
			cmd->param.dslStatusBufSpec.bufSize = *srcPtr++;
			break;

		case kDslDiagFrameHdrCmd:
			cmd->param.dslStatusBufSpec.pBuf	= (void *) *srcPtr++;
			cmd->param.dslStatusBufSpec.bufSize = *srcPtr++;
			break;

		case kDslAfeTestCmd:
			cmd->param.dslAfeTestSpec.type = *srcPtr++;
			cmd->param.dslAfeTestSpec.afeParamPtr = (void *) *srcPtr++;
			cmd->param.dslAfeTestSpec.afeParamSize = *srcPtr++;
			cmd->param.dslAfeTestSpec.imagePtr = (void *) *srcPtr++;
			cmd->param.dslAfeTestSpec.imageSize = *srcPtr++;
			break;

		case kDslAfeTestCmd1:
			cmd->param.dslAfeTestSpec1.type = *srcPtr++;
			cmd->param.dslAfeTestSpec1.param1 = *srcPtr++;
			cmd->param.dslAfeTestSpec1.param2 = *srcPtr++;
			break;
			
		case kDslPLNControlCmd:
			cmd->param.dslPlnSpec.plnCmd = *srcPtr++;
			if (kDslPLNControlStart == cmd->param.dslPlnSpec.plnCmd) {
				cmd->param.dslPlnSpec.mgnDescreaseLevelPerBin = srcPtr[0];
				cmd->param.dslPlnSpec.mgnDescreaseLevelBand	  = srcPtr[1];
				srcPtr += 2;
			}
			else if(kDslPLNControlDefineInpBinTable==cmd->param.dslPlnSpec.plnCmd) {
				cmd->param.dslPlnSpec.nInpBin= srcPtr[0];
				srcPtr++;
				cmd->param.dslPlnSpec.inpBinPtr=(void *)*srcPtr++;
			}
			else if(kDslPLNControlDefineItaBinTable==cmd->param.dslPlnSpec.plnCmd) {
				cmd->param.dslPlnSpec.nItaBin=*srcPtr++;
				cmd->param.dslPlnSpec.itaBinPtr=(void *)*srcPtr++;
			}
			else if (kDslINMControlParams==cmd->param.dslPlnSpec.plnCmd) {
				cmd->param.dslPlnSpec.inmContinueConfig = *srcPtr++;
				cmd->param.dslPlnSpec.inmInpEqMode = *srcPtr++;
			}
			else if (kDslINMConfigParams==cmd->param.dslPlnSpec.plnCmd) {
				cmd->param.dslPlnSpec.inmContinueConfig = *srcPtr++;
				cmd->param.dslPlnSpec.inmInpEqMode = *srcPtr++;
				cmd->param.dslPlnSpec.inmIATO = *srcPtr++;
				cmd->param.dslPlnSpec.inmIATS = *srcPtr++;
			}
			else if (kDslINMConfigInpEqFormat == cmd->param.dslPlnSpec.plnCmd)
				cmd->param.dslPlnSpec.inmInpEqFormat = *srcPtr++;
			break;

#ifdef G997_1
		case kDslSendEocCommand:
			cmd->param.value = *srcPtr++;
			if (cmd->param.value >= kDslClearEocFirstCmd)
				{
				cmd->param.dslClearEocMsg.msgType = *srcPtr++;
				if (cmd->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask)
					{
					cmd->param.dslClearEocMsg.dataPtr = (char*) srcPtr;
					srcPtr += ((cmd->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask) + 3) >> 2;
					}
				else 
					{
					cmd->param.dslClearEocMsg.dataPtr = SDRAM_HOST2ADSL(*srcPtr);
					srcPtr++;
					}
				}
			break;
#endif
#ifdef G992P3
		case kDslOLRRequestCmd:
			cmd->param.dslOLRRequest.msgType = *srcPtr++;
			cmd->param.dslOLRRequest.nCarrs	 = *srcPtr++;
			BlockByteMove (12, (void *) srcPtr, (void *) cmd->param.dslOLRRequest.L);
			srcPtr += 3;
			cmd->param.dslOLRRequest.carrParamPtr = SDRAM_HOST2ADSL(*srcPtr);
			srcPtr++;
			break;

		case kDslPwrMgrCmd:
			cmd->param.dslPwrMsg.msgType = *srcPtr++;
			cmd->param.dslPwrMsg.param.msg.msgLen = *srcPtr++;
			cmd->param.dslPwrMsg.param.msg.msgData = (void *) *srcPtr++;
			break;
#endif
		default:
			cmd->param.value = *srcPtr++;
			break;
	}
	return (char*)srcPtr - (char*)srcPtr0;
}

Private void * FlattenStatusSlow (dslStatusStruct *status, uint *dstPtr, uint statAvail, uint *dstPtr0)
{
#define	CHECK_BUF_AVAIL_SLOW(szReq,szAvail)	if ((int)(szReq) > (int)(szAvail)) return dstPtr0;

	int		size;

	switch	(DSL_STATUS_CODE(status->code)) {
		case kDslExceptionStatus:
			{
			int		i;

			CHECK_BUF_AVAIL_SLOW(34+status->param.dslException.argc, statAvail);
			BlockLongMove (32, (int *) status->param.dslException.sp, (int*) dstPtr);
			dstPtr += 31;

			*dstPtr++ = (uint) status->param.dslException.argc;
			for (i = 0; i < status->param.dslException.argc; i++)
				*dstPtr++ = (uint) status->param.dslException.argv[i];
			}	
			break;

		case kAtmStatus:
			switch (status->param.atmStatus.code) {
				case kAtmStatVcCreated:
					CHECK_BUF_AVAIL_SLOW(6, statAvail);
					*dstPtr++ = status->param.atmStatus.param.vcInfo.vcId;
					*dstPtr++ = status->param.atmStatus.param.vcInfo.vci;
					*dstPtr++ = status->param.atmStatus.param.vcInfo.aalType;
					*dstPtr++ = status->param.atmStatus.param.vcInfo.fwdPeakCellTime;
					*dstPtr++ = status->param.atmStatus.param.vcInfo.backPeakCellTime;
					break;

				case kAtmStatOamF4SegmentCell:
				case kAtmStatOamF4End2EndCell:
				case kAtmStatOamF5SegmentCell:
				case kAtmStatOamF5End2EndCell:
					*dstPtr++ = status->param.atmStatus.param.oamInfo.oamCmd;
					break;

				case kAtmStatRxPacket:
				case kAtmStatTxPacket:
					{
					int			frBytes;

					frBytes = status->param.atmStatus.param.frame.length;
					if  (frBytes > kMaxFlattenFramelength)
						frBytes = kMaxFlattenFramelength;
					size = (frBytes + 3) >> 2;
					CHECK_BUF_AVAIL_SLOW(3+size, statAvail);
					*dstPtr++ = status->param.atmStatus.param.frame.vci;
					*dstPtr++ = status->param.atmStatus.param.frame.aalType;
					*dstPtr++ = status->param.atmStatus.param.frame.length;
					BlockByteMove ((size<<2), status->param.atmStatus.param.frame.framePtr, (char*) dstPtr);
					dstPtr += size;
					}
					break;
			}
			break;

#ifdef G992P3
		case kDslOLRRequestStatus:
		case kDslOLRBitGainUpdateStatus:
			*dstPtr++ = status->param.dslOLRRequest.msgType;
			*dstPtr++ = status->param.dslOLRRequest.nCarrs;
			BlockByteMove (12, (void *) status->param.dslOLRRequest.L, (char*) dstPtr);
			dstPtr += 3;
			*dstPtr++ = (uint) status->param.dslOLRRequest.carrParamPtr;
			break;
		case kDslPwrMgrStatus:
			*dstPtr++ = status->param.dslPwrMsg.msgType;
			*dstPtr++ = status->param.dslPwrMsg.param.msg.msgLen;
			*dstPtr++ = (uint) status->param.dslPwrMsg.param.msg.msgData;
			break;
#endif
	}

	return dstPtr;
}

#ifdef	EXTENDED_INTERLEAVE_DEPTH_24K
Public int FlattenStatus (dslStatusStruct *status, uint *dstPtr, uint nAvail) ;
#else
Public int FlattenStatus (dslStatusStruct *status, uint *dstPtr, uint nAvail) FAST_TEXT;
#endif
Public int FlattenStatus (dslStatusStruct *status, uint *dstPtr, uint nAvail)
{
#define	CHECK_BUF_AVAIL(szReq,szAvail)	if ((int)(szReq) > (int)(szAvail)) return 0;

	uint	*dstPtr0 = dstPtr, statAvail = (nAvail >> 2);
	int		size;

	*dstPtr++ = status->code;
	
	switch	(DSL_STATUS_CODE(status->code)) {
		case		kDslTrainingStatus:
			*dstPtr++ = status->param.dslTrainingInfo.code;
			*dstPtr++ = status->param.dslTrainingInfo.value;
			break;

		case 		kDslDspControlStatus:
			*dstPtr++ = status->param.dslConnectInfo.code;
			*dstPtr++ = status->param.dslConnectInfo.value;
			*dstPtr++ = (int)status->param.dslConnectInfo.buffPtr;
			break;

		case		kDslConnectInfoStatus:
			*dstPtr++ = status->param.dslConnectInfo.code;
			*dstPtr++ = status->param.dslConnectInfo.value;
			switch (status->param.dslConnectInfo.code) {
#ifdef G992
				case	kG992p2XmtToneOrderingInfo:
				case	kG992p2RcvToneOrderingInfo:
#ifdef G992_BIT_SWAP
				case	kG992AocMessageExchangeRcvInfo:
				case	kG992AocMessageExchangeXmtInfo:
#endif
					size = (status->param.dslConnectInfo.value + 3) >> 2;
					CHECK_BUF_AVAIL(size, statAvail-3);
					BlockByteMove (
						(size << 2), 
						(char*)status->param.dslConnectInfo.buffPtr,
						(char*)dstPtr);
					dstPtr += size;
					break;
				case	kG992p2XmtCodingParamsInfo:
				case	kG992p2RcvCodingParamsInfo:
					{
					G992CodingParams	*codingParam;

					size = (sizeof(G992CodingParams) + 3) >> 2;
					CHECK_BUF_AVAIL(size, statAvail-3);
					codingParam = (G992CodingParams*)status->param.dslConnectInfo.buffPtr;
					BlockByteMove ((size<<2), (uchar *)codingParam, (uchar *)dstPtr);
					dstPtr += size;
					}
					break;
				case	kG992p2TrainingRcvCarrEdgeInfo:
					{
					int		*carrEdges = (int*)(status->param.dslConnectInfo.buffPtr);

					*dstPtr++ = carrEdges[0];
					*dstPtr++ = carrEdges[1];
					}
					break;
				case	kG992ShowtimeMonitoringStatus:
#ifdef DSL_REPORT_ALL_COUNTERS
					CHECK_BUF_AVAIL(kG992ShowtimeNumOfMonitorCounters, statAvail-3);
#else
					CHECK_BUF_AVAIL(12, statAvail-3);
#endif
					BlockShortMove (
#ifdef DSL_REPORT_ALL_COUNTERS
						kG992ShowtimeNumOfMonitorCounters << 1,
#else
						12 << 1,
#endif
						(short*)status->param.dslConnectInfo.buffPtr,
						(short*)dstPtr);
#ifdef DSL_REPORT_ALL_COUNTERS
					dstPtr += kG992ShowtimeNumOfMonitorCounters;
#else
					dstPtr += 12;
#endif
					break;
#ifdef FIRE_RETRANSMISSION
				case	kFireMonitoringCounters:
					CHECK_BUF_AVAIL(kFireNumOfCounters, statAvail-3);
					BlockShortMove (
						kFireNumOfCounters << 1,
						(short*)status->param.dslConnectInfo.buffPtr,
						(short*)dstPtr);
					dstPtr += kFireNumOfCounters;
					break;
#endif
#endif	/* G992 */

#if defined(G992) || defined(G994P1)
				case	kG994MessageExchangeRcvInfo:
				case	kG994MessageExchangeXmtInfo:
				case	kG992MessageExchangeRcvInfo:
				case	kG992MessageExchangeXmtInfo:
					size = (status->param.dslConnectInfo.value + 3) >> 2;
					CHECK_BUF_AVAIL(size, statAvail-3);
					BlockByteMove (
						(size << 2), 
						(char*)status->param.dslConnectInfo.buffPtr,
						(char*)dstPtr);
					dstPtr += size;
					break;
#endif	/* defined(G992) || defined(G994P1) */

				case	kDslTEQCoefInfo:
				case	kDslRcvPsdInfo:
				case	kDslRcvCarrierSNRInfo:
					size = (status->param.dslConnectInfo.value + 1) >> 1;
					CHECK_BUF_AVAIL(size, statAvail-3);
					BlockShortMove (
						(size << 1), 
						(short*)status->param.dslConnectInfo.buffPtr,
						(short*)dstPtr);
					dstPtr += size;
					break;

				case kG992p3XmtCodingParamsInfo:
				case kG992p3RcvCodingParamsInfo:
					size = (status->param.dslConnectInfo.value + 3) >> 2;
					CHECK_BUF_AVAIL(size, statAvail-3);
					BlockByteMove ((size<<2), status->param.dslConnectInfo.buffPtr, (uchar *)dstPtr);
					dstPtr += size;
					break;
				case kDslChannelResponseLog:
				case kDslChannelResponseLinear:
				case kDslChannelQuietLineNoise:
					*dstPtr++ = (uint)status->param.dslConnectInfo.buffPtr;
					break;
			}
			break;

		case		kDslShowtimeSNRMarginInfo:
			size = (status->param.dslShowtimeSNRMarginInfo.nCarriers + 1) >> 1;
			CHECK_BUF_AVAIL(size+6, statAvail-1);
			BlockShortMove (6*2, (short *) &status->param.dslShowtimeSNRMarginInfo, (short *) dstPtr);
			dstPtr += 6;
			if (size != 0)
				BlockShortMove (
					(size << 1), 
					(short*)status->param.dslShowtimeSNRMarginInfo.buffPtr,
					(short*)dstPtr);
			dstPtr += size;
			break;

#ifdef G997_1
		case kDslReceivedEocCommand:
			*dstPtr++ = status->param.value;
			if (status->param.value >= kDslClearEocFirstCmd)
				{
				*dstPtr++ = status->param.dslClearEocMsg.msgType;
				if (status->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask)
					{
					int	len;

					len = status->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
					size = (len + 3) >> 2;
					CHECK_BUF_AVAIL(size, statAvail-3);
					BlockByteMove ((size << 2), status->param.dslClearEocMsg.dataPtr, (char*)dstPtr);
					dstPtr += size;
					}
				else
					*dstPtr++ = (uint) status->param.dslClearEocMsg.dataPtr;
				}
			break;

#ifdef G997_1_FRAMER
		case kDslG997Status:
			*dstPtr++ = status->param.g997Status.code;
			switch (status->param.g997Status.code) {
				case kDslFramerRxFrameErr:
				case kDslFramerTxFrameErr:
					*dstPtr++ = status->param.g997Status.param.error;
					break;
			}
			break;
#endif

#endif

		case kAtmStatus:
			*dstPtr++ = status->param.atmStatus.code;
			*dstPtr++ = status->param.atmStatus.param.value;
			if (status->param.atmStatus.code < kAtmLayerStatFirst) {
				if (kAtmStatBertResult == status->param.atmStatus.code)
					*dstPtr++ = status->param.atmStatus.param.bertInfo.errBits;
				else
					dstPtr = FlattenStatusSlow (status, dstPtr, statAvail - 3, dstPtr0);
			}
			break;

		case kDslPrintfStatus:
			{
				va_list		ap;
				int		arg, i;

				CHECK_BUF_AVAIL(status->param.dslPrintfMsg.argNum+2, statAvail-1);
				*dstPtr++ = (uint) status->param.dslPrintfMsg.fmt;
				*dstPtr++ = status->param.dslPrintfMsg.argNum;
				ap = (void *) status->param.dslPrintfMsg.argPtr;
				for (i = 0; i < status->param.dslPrintfMsg.argNum; i++) {
					arg = va_arg(ap, int);
					*dstPtr++ = arg;
				}
			}
			break;

		case kDslExceptionStatus:
			dstPtr = FlattenStatusSlow (status, dstPtr, statAvail - 1, dstPtr0);
			break;

		case kDslDataAvailStatus:
			*dstPtr++ = (uint) status->param.dslDataAvail.dataPtr;
			*dstPtr++ = status->param.dslDataAvail.dataLen;
			break;

#ifdef G992P3
		case kDslOLRRequestStatus:
		case kDslOLRBitGainUpdateStatus:
		case kDslPwrMgrStatus:
			dstPtr = FlattenStatusSlow (status, dstPtr, statAvail - 1, dstPtr0);
			break;
#endif
		case kDslAfeTestStatus:
			CHECK_BUF_AVAIL(3,statAvail-1);
			*dstPtr++ = (uint) status->param.dslAfeTestStatus.type;
			*dstPtr++ = (uint) status->param.dslAfeTestStatus.param1;
			*dstPtr++ = (uint) status->param.dslAfeTestStatus.param2;			
			break;

		default:
			*dstPtr++ = status->param.value;
			break;
	}
	return (char*)dstPtr - (char*)dstPtr0;
}

#endif /* HOST_ONLY */

#ifndef ADSLCORE_ONLY

static	void	*statDataPtr = NULL;
static	uint	statDataLen = 0;

#if defined(CHECK_LMEM_ADDR_IN_STAT) || defined(XDSLDRV_ENABLE_PARSER)
extern void *XdslCoreGetDslVars(unsigned char lineId);
#endif
#if defined(STATUS_BUFFER_STAT)
extern void * XdslCoreGetCurDslVars(void);
#endif
#ifdef CONFIG_ARM64
uint	*pStackPtr = NULL;
#endif

Public int UnflattenStatus (uint *srcPtr, dslStatusStruct *status)
{
	uint	*srcPtr0 = srcPtr;
	uint	tmp;
	Boolean	bStatPtrSet = false;
	
	status->code = ADSL_ENDIAN_CONV_INT32(*srcPtr++);

	switch	(DSL_STATUS_CODE(status->code)) {
		case		kDslTrainingStatus:
			status->param.dslTrainingInfo.code  = ADSL_ENDIAN_CONV_INT32(srcPtr[0]);
			status->param.dslTrainingInfo.value = ADSL_ENDIAN_CONV_INT32(srcPtr[1]);
			srcPtr += 2;
			break;

		case 		kDslDspControlStatus:
			status->param.dslConnectInfo.code  = ADSL_ENDIAN_CONV_INT32(srcPtr[0]);
			status->param.dslConnectInfo.value = ADSL_ENDIAN_CONV_INT32(srcPtr[1]);
#ifdef FLATTEN_ADDR_ADJUST
#ifdef CHECK_LMEM_ADDR_IN_STAT
			if(ADSL_MIPS_LMEM_ADDR(srcPtr[2]))
				__SoftDslPrintf(XdslCoreGetDslVars(DSL_LINE_ID(status->code)),
					"*** LMEM ADDR in status: code=%ld, value=%ld", 0, DSL_STATUS_CODE(status->code), srcPtr[1]);
#endif
			status->param.dslConnectInfo.buffPtr = ADSL_ADDR_TO_HOST(ADSL_ENDIAN_CONV_INT32(srcPtr[2]));
#else
			status->param.dslConnectInfo.buffPtr = (void *) ADSL_ENDIAN_CONV_INT32(srcPtr[2]);
#endif
			srcPtr += 3;
#ifndef DEVICENAME
			if (kDslStatusBufferInfo == status->param.dslConnectInfo.code) {
				statDataPtr = status->param.dslConnectInfo.buffPtr;
				statDataLen = status->param.dslConnectInfo.value;
				bStatPtrSet = true;
			}
#endif
			break;
		case		kDslConnectInfoStatus:
			status->param.dslConnectInfo.code  = ADSL_ENDIAN_CONV_INT32(srcPtr[0]);
			status->param.dslConnectInfo.value = ADSL_ENDIAN_CONV_INT32(srcPtr[1]);
			srcPtr += 2;
			switch (status->param.dslConnectInfo.code) {
#ifdef G992
				case	kG992p2XmtToneOrderingInfo:
				case	kG992p2RcvToneOrderingInfo:
				case	kG992AocMessageExchangeRcvInfo:
				case	kG992AocMessageExchangeXmtInfo:
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					tmp= (status->param.dslConnectInfo.value + 3) >> 2;
					if (NULL != statDataPtr) {
						status->param.dslConnectInfo.value   = statDataLen;
						status->param.dslConnectInfo.buffPtr = statDataPtr;
						srcPtr[-1] = ADSL_ENDIAN_CONV_INT32(statDataLen);
					}
					srcPtr += tmp;
					break;
				case	kG992p2XmtCodingParamsInfo:
				case	kG992p2RcvCodingParamsInfo:
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					srcPtr += (sizeof(G992CodingParams) + 3) >> 2;
					break;
				case	kG992p2TrainingRcvCarrEdgeInfo:
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					srcPtr += 2;
					break;
				case	kG992ShowtimeMonitoringStatus:
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					srcPtr += DSL_COUNTERS_MAX;
					break;
#endif	/* G992 */

#if defined(G992) || defined(G994P1)
				case	kG994MessageExchangeRcvInfo:
				case	kG994MessageExchangeXmtInfo:
				case	kG992MessageExchangeRcvInfo:
				case	kG992MessageExchangeXmtInfo:
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					tmp = (status->param.dslConnectInfo.value + 3) >> 2;
					if (NULL != statDataPtr) {
						status->param.dslConnectInfo.value   = statDataLen;
						status->param.dslConnectInfo.buffPtr = statDataPtr;
						srcPtr[-1] = ADSL_ENDIAN_CONV_INT32(statDataLen);
					}
					srcPtr += tmp;
					break;
#endif	/* defined(G992) || defined(G994P1) */

				case	kDslTEQCoefInfo:
				case	kDslRcvPsdInfo:
				case	kDslRcvCarrierSNRInfo:
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					tmp = (status->param.dslConnectInfo.value + 1) >> 1;
					if (NULL != statDataPtr) {
						status->param.dslConnectInfo.value   = statDataLen;
						status->param.dslConnectInfo.buffPtr = statDataPtr;
						srcPtr[-1] = ADSL_ENDIAN_CONV_INT32(statDataLen);
					}
					srcPtr += tmp;
					break;
#ifdef	G992P3
				case kG992p3XmtCodingParamsInfo:
				case kG992p3RcvCodingParamsInfo:
					status->param.dslConnectInfo.buffPtr = (void*) srcPtr;
					srcPtr += (status->param.dslConnectInfo.value + 3) >> 2;
					break;

				case kDslChannelResponseLog:
				case kDslChannelResponseLinear:
				case kDslChannelQuietLineNoise:
					{
					uintptr_t	bufAddr = ADSL_ENDIAN_CONV_INT32(*srcPtr++);

#ifdef FLATTEN_ADDR_ADJUST
#ifdef CHECK_LMEM_ADDR_IN_STAT
					if(ADSL_MIPS_LMEM_ADDR(bufAddr))
						__SoftDslPrintf(XdslCoreGetDslVars(DSL_LINE_ID(status->code)),
							"*** LMEM ADDR in status: code=%ld, value=%ld", 0,
							DSL_STATUS_CODE(status->code),
							status->param.dslConnectInfo.value);
#endif

					bufAddr = (uintptr_t)(ADSL_ADDR_TO_HOST(bufAddr));
#endif
					status->param.dslConnectInfo.buffPtr = (void *) bufAddr;
					}
					break;
#endif
			}
			break;

		case		kDslShowtimeSNRMarginInfo:
			BlockLongMoveReverse (6, (int *) srcPtr, (int *) &status->param.dslShowtimeSNRMarginInfo);
			srcPtr += 6;
			status->param.dslShowtimeSNRMarginInfo.buffPtr = (void*) srcPtr;
			tmp = (status->param.dslShowtimeSNRMarginInfo.nCarriers + 1) >> 1;
			if (NULL != statDataPtr) {
				status->param.dslShowtimeSNRMarginInfo.nCarriers = statDataLen;
				status->param.dslShowtimeSNRMarginInfo.buffPtr   = statDataPtr;
				srcPtr[-1] = ADSL_ENDIAN_CONV_INT32(statDataLen);
			}
			srcPtr += tmp;
			break;

#ifdef G997_1
		case kDslReceivedEocCommand:
			status->param.value = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			if (status->param.value >= kDslClearEocFirstCmd)
				{
				status->param.dslClearEocMsg.msgType = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
				if (status->param.dslClearEocMsg.msgType & kDslClearEocMsgDataVolatileMask)
					{
					int	size = status->param.dslClearEocMsg.msgType & kDslClearEocMsgLengthMask;
					size = (size + 3) >> 2;
					status->param.dslClearEocMsg.dataPtr = (char*) srcPtr;
					srcPtr += size;
					}
				else 
					{
					status->param.dslClearEocMsg.dataPtr = (char*)(uintptr_t)ADSL_ENDIAN_CONV_UINT32(*srcPtr++);
#ifdef FLATTEN_ADDR_ADJUST
					if (NULL != status->param.dslClearEocMsg.dataPtr) {
#ifdef CHECK_LMEM_ADDR_IN_STAT
						if(ADSL_MIPS_LMEM_ADDR(status->param.dslClearEocMsg.dataPtr))
							__SoftDslPrintf(XdslCoreGetDslVars(DSL_LINE_ID(status->code)),
								"*** LMEM ADDR in status: code=%ld, value=%ld", 0,
								DSL_STATUS_CODE(status->code),
								status->param.value);
#endif
						if (0 == (status->param.dslClearEocMsg.msgType & kDslClearEocMsgHostAddr))
							status->param.dslClearEocMsg.dataPtr = ADSL_ADDR_TO_HOST((uintptr_t)status->param.dslClearEocMsg.dataPtr);

					}
#endif
					}
				}
			break;

#ifdef G997_1_FRAMER
		case kDslG997Status:
			status->param.g997Status.code = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			switch (status->param.g997Status.code) {
				case kDslFramerRxFrameErr:
				case kDslFramerTxFrameErr:
					status->param.g997Status.param.error= ADSL_ENDIAN_CONV_INT32(*srcPtr++);
					break;
			}
			break;
#endif

#endif

		case		kAtmStatus:
			status->param.atmStatus.code        = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			status->param.atmStatus.param.value = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			switch (status->param.atmStatus.code) {
				case kAtmStatVcCreated:
					status->param.atmStatus.param.vcInfo.vcId				= ADSL_ENDIAN_CONV_INT32(*srcPtr++);
					status->param.atmStatus.param.vcInfo.vci				= ADSL_ENDIAN_CONV_INT32(*srcPtr++);
					status->param.atmStatus.param.vcInfo.aalType			= ADSL_ENDIAN_CONV_INT32(*srcPtr++);
					status->param.atmStatus.param.vcInfo.fwdPeakCellTime	= ADSL_ENDIAN_CONV_INT32(*srcPtr++);
					status->param.atmStatus.param.vcInfo.backPeakCellTime	= ADSL_ENDIAN_CONV_INT32(*srcPtr++);
					break;

				case kAtmStatOamF4SegmentCell:
				case kAtmStatOamF4End2EndCell:
				case kAtmStatOamF5SegmentCell:
				case kAtmStatOamF5End2EndCell:
					status->param.atmStatus.param.oamInfo.oamCmd = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
					break;

				case kAtmStatRxPacket:
				case kAtmStatTxPacket:
					{
					int			frBytes;

					status->param.atmStatus.param.frame.vci		= ADSL_ENDIAN_CONV_INT32(*srcPtr++);
					status->param.atmStatus.param.frame.aalType	= ADSL_ENDIAN_CONV_INT32(*srcPtr++);
					status->param.atmStatus.param.frame.length		= ADSL_ENDIAN_CONV_INT32(*srcPtr++);
					frBytes = status->param.atmStatus.param.frame.length;
					if  (frBytes > kMaxFlattenFramelength)
						frBytes = kMaxFlattenFramelength;
					status->param.atmStatus.param.frame.framePtr= (void*)srcPtr;
					srcPtr += (frBytes + 3) >> 2;
					}
					break;
				case kAtmStatBertResult:
					status->param.atmStatus.param.bertInfo.errBits = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
					break;
				case kAtmStatCounters:
#ifdef FLATTEN_ADDR_ADJUST
#ifdef CHECK_LMEM_ADDR_IN_STAT
					if(ADSL_MIPS_LMEM_ADDR(status->param.atmStatus.param.value))
						__SoftDslPrintf(XdslCoreGetDslVars(DSL_LINE_ID(status->code)),
							"*** LMEM ADDR in status: code=%d, value=%d", 0,
							DSL_STATUS_CODE(status->code),
							kAtmStatCounters);
#endif
#if 0	/* Move address conversion to where the pointer is accessed to avoid address truncation on 64bits host processor */
					status->param.atmStatus.param.value = (int)(ADSL_ADDR_TO_HOST(status->param.atmStatus.param.value));
#endif
#endif
					break;
			}
			break;

		case kDslPrintfStatus:
			status->param.dslPrintfMsg.fmt = (void *)(uintptr_t)ADSL_ENDIAN_CONV_UINT32(*srcPtr++);
#ifdef FLATTEN_ADDR_ADJUST
			{
				uint	fmtAddr = (uint)(uintptr_t)status->param.dslPrintfMsg.fmt;
#ifdef CHECK_LMEM_ADDR_IN_STAT
				if(ADSL_MIPS_LMEM_ADDR(fmtAddr))
					__SoftDslPrintf(XdslCoreGetDslVars(DSL_LINE_ID(status->code)),
						"*** LMEM ADDR in status: code=%d, value=%d", 0,
						DSL_STATUS_CODE(status->code),
						(uint)fmtAddr);
#endif
				status->param.dslPrintfMsg.fmt = ADSL_ADDR_TO_HOST(fmtAddr);
			}
#endif
			status->param.dslPrintfMsg.argNum = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			status->param.dslPrintfMsg.argPtr = (void *)srcPtr;
			srcPtr += status->param.dslPrintfMsg.argNum;
			break;

		case kDslExceptionStatus:
#ifdef CONFIG_ARM64
			pStackPtr = srcPtr;
#endif
			status->param.dslException.sp = (int)(uintptr_t)srcPtr;
			srcPtr += 31;

			status->param.dslException.argc = (int)ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			status->param.dslException.argv = (int*)srcPtr;
			srcPtr += status->param.dslException.argc;
			break;

		case kDslOemDataAddrStatus:
			status->param.value = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
#ifdef FLATTEN_ADDR_ADJUST
#ifdef CHECK_LMEM_ADDR_IN_STAT
			if(ADSL_MIPS_LMEM_ADDR(status->param.value))
				__SoftDslPrintf(XdslCoreGetDslVars(DSL_LINE_ID(status->code)),
					"*** LMEM ADDR in status: code=%d, value=%d", 0,
					DSL_STATUS_CODE(status->code),
					status->param.value);
#endif
#if 0	/* Move address conversion to where the pointer is accessed to avoid address truncation on 64bits host processor */
			status->param.value = (int)(ADSL_ADDR_TO_HOST(status->param.value));
#endif
#endif
			break;

		case kDslDataAvailStatus:
			status->param.dslDataAvail.dataPtr = ADSL_ENDIAN_CONV_UINT32(*srcPtr++);
#ifdef FLATTEN_ADDR_ADJUST
#ifdef CHECK_LMEM_ADDR_IN_STAT
			if(ADSL_MIPS_LMEM_ADDR(status->param.dslDataAvail.dataPtr))
				__SoftDslPrintf(XdslCoreGetDslVars(DSL_LINE_ID(status->code)),
					"*** LMEM ADDR in status: code=%d, value=%d", 0,
					DSL_STATUS_CODE(status->code),
					status->param.dslDataAvail.dataPtr);
#endif
			status->param.dslDataAvail.dataPtr = (uint)(uintptr_t)(ADSL_ADDR_TO_HOST(status->param.dslDataAvail.dataPtr));
#endif
			status->param.dslDataAvail.dataLen = ADSL_ENDIAN_CONV_UINT32(*srcPtr++);
			break;

#ifdef G992P3
		case kDslOLRRequestStatus:
		case kDslOLRBitGainUpdateStatus:
			status->param.dslOLRRequest.msgType = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			status->param.dslOLRRequest.nCarrs	= ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			BlockByteMove (12, (void *) srcPtr, (void *) status->param.dslOLRRequest.L);
#ifdef ADSLDRV_LITTLE_ENDIAN
		{
			uint *pL = (void *) status->param.dslOLRRequest.L;	/* Endian converting "ushort  L[4]" */
			pL[0] = ADSL_ENDIAN_CONV_2SHORTS(pL[0]);
			pL[1] = ADSL_ENDIAN_CONV_2SHORTS(pL[1]);
		}
#endif
			srcPtr += 3;
			tmp = ADSL_ENDIAN_CONV_UINT32(*srcPtr++);
#ifdef FLATTEN_ADDR_ADJUST
#ifdef CHECK_LMEM_ADDR_IN_STAT
			if(ADSL_MIPS_LMEM_ADDR(tmp))
				__SoftDslPrintf(XdslCoreGetDslVars(DSL_LINE_ID(status->code)),
					"*** LMEM ADDR in status: code=%d, value=%d", 0,
					DSL_STATUS_CODE(status->code),
					status->param.dslOLRRequest.msgType);
#endif
			status->param.dslOLRRequest.carrParamPtr = (void *) (ADSL_ADDR_TO_HOST(tmp));
#else
			status->param.dslOLRRequest.carrParamPtr = (void *) tmp;
#endif
			break;
		case kDslPwrMgrStatus:
			status->param.dslPwrMsg.msgType = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			status->param.dslPwrMsg.param.msg.msgLen = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			tmp = ADSL_ENDIAN_CONV_UINT32(*srcPtr++);
#ifdef FLATTEN_ADDR_ADJUST
#ifdef CHECK_LMEM_ADDR_IN_STAT
			if(ADSL_MIPS_LMEM_ADDR(tmp))
				__SoftDslPrintf(XdslCoreGetDslVars(DSL_LINE_ID(status->code)),
					"*** LMEM ADDR in status: code=%d, value=%d", 0,
					DSL_STATUS_CODE(status->code),
					status->param.dslPwrMsg.msgType);
#endif
			status->param.dslPwrMsg.param.msg.msgData = (void *) (ADSL_ADDR_TO_HOST(tmp));
#else
			status->param.dslPwrMsg.param.msg.msgData = (void *) tmp;
#endif
			break;
#endif
		case kDslAfeTestStatus:
			status->param.dslAfeTestStatus.type =  ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			status->param.dslAfeTestStatus.param1 =  ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			status->param.dslAfeTestStatus.param2 =  ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			break;
#if 0	/* Move address conversion to where the pointer is accessed to avoid address truncation on 64bits host processor */
		case kDslCommandBufferChange:
		case kDslStatusBufferChange:
			tmp = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			status->param.value = (int)(ADSL_ADDR_TO_HOST(tmp));
			break;
#endif
		default:
			status->param.value = ADSL_ENDIAN_CONV_INT32(*srcPtr++);
			break;
	}
	if (!bStatPtrSet)
		statDataPtr = NULL;
	return (char*)srcPtr - (char*)srcPtr0;
}

#endif

/*
**
**	Functions to read and write to command/status strectch buffers
**
*/

#define	BUFFER_ANCHOR		0x55AA1234

#ifndef HOST_ONLY

#ifdef	EXTENDED_INTERLEAVE_DEPTH_24K
Public int	FlattenBufferStatusWrite(stretchBufferStruct *fBuf, dslStatusStruct *status) ;
#else
Public int	FlattenBufferStatusWrite(stretchBufferStruct *fBuf, dslStatusStruct *status) FAST_TEXT;
#endif
Public int	FlattenBufferStatusWrite(stretchBufferStruct *fBuf, dslStatusStruct *status)
{
	uint	statBytesAvail, n, *p;
	char	*rdPtr;
	uint	critId = SoftDslEnterCritical();

#if defined(bcm47xx) && defined(ADSLCORE_ONLY)
	rdPtr = ((stretchBufferStruct *) ((uint)fBuf | 0x20000000))->pRead;
#else
	rdPtr = fBuf->pRead;
#endif
	statBytesAvail = _StretchBufferGetWriteAvail(fBuf,rdPtr);
	if (statBytesAvail < 32) {
		SoftDslLeaveCritical(critId);
		return 0;
	}

	p = (uint *) StretchBufferGetWritePtr(fBuf);
	n = FlattenStatus (status, p, statBytesAvail-4);
	if (n > 0) {
		n = (n + 3) & ~3;
		*(p + (n >> 2)) = BUFFER_ANCHOR;
		n += 4;
		_StretchBufferWriteUpdate (fBuf, rdPtr, n);
	}
	SoftDslLeaveCritical(critId);
	return n;
}

#endif /* HOST_ONLY */

#ifndef ADSLCORE_ONLY

/* #define STATUS_BUFFER_STAT */
#ifdef  STATUS_BUFFER_STAT
static uint	maxStatLen = 0;
static uint	maxStatBuf = 0;
#endif

#ifdef SUPPORT_STATUS_BACKUP
extern AdslXfaceData	*pAdslXface;
#endif

Public void FlattenBufferClearStat(void)
{
#ifdef  STATUS_BUFFER_STAT
	maxStatLen = 0;
	maxStatBuf = 0;
#endif
}

#ifdef CONFIG_ARM64
extern int printk(const char *fmt, ...);
extern int pmc_dsl_mips_enable(int flag);
extern int pmc_dsl_core_reset(void);
extern void bcmOsDelay(unsigned long timeMs);
stretchBufferStruct stretchBuf;
static char buf[1000];
static void printSbufferPtrs(stretchBufferStruct *fBuf, char *pHdr)
{
	int n;
	
	stretchBuf = *fBuf;
	pmc_dsl_mips_enable(0);
	bcmOsDelay(1);
	pmc_dsl_core_reset();
	
	n = sprintf(buf, "%s\n", pHdr);
	n += sprintf(buf+n, "FATAL: bogus/erroneous LMEM content\n");
	n += sprintf(buf+n, " pStart=0x%08x pEnd=0x%08x pExtraEnd=0x%08x pStretchEnd=0x%08x pRead=0x%08x pWrite=0x%08x\n",
		ADSL_ENDIAN_CONV_INT32(stretchBuf.pStart), ADSL_ENDIAN_CONV_INT32(stretchBuf.pEnd),
		ADSL_ENDIAN_CONV_INT32(stretchBuf.pEnd), ADSL_ENDIAN_CONV_INT32(stretchBuf.pStretchEnd),
		ADSL_ENDIAN_CONV_INT32(stretchBuf.pRead), ADSL_ENDIAN_CONV_INT32(stretchBuf.pWrite));
	bcmOsDelay(2);
	n += sprintf(buf+n, " pStart=0x%08x pEnd=0x%08x pExtraEnd=0x%08x pStretchEnd=0x%08x pRead=0x%08x pWrite=0x%08x\n",
		ADSL_ENDIAN_CONV_INT32(fBuf->pStart), ADSL_ENDIAN_CONV_INT32(fBuf->pEnd),
		ADSL_ENDIAN_CONV_INT32(fBuf->pEnd), ADSL_ENDIAN_CONV_INT32(fBuf->pStretchEnd),
		ADSL_ENDIAN_CONV_INT32(fBuf->pRead), ADSL_ENDIAN_CONV_INT32(fBuf->pWrite));
	DiagWriteString(0, DIAG_DSL_CLIENT, buf);
	printk("%s", buf);
}
#endif

Public int	FlattenBufferStatusRead(stretchBufferStruct *fBuf, dslStatusStruct *status)
{
	uint	*p;
	int		statBytesAvail, n;

	statBytesAvail = StretchBufferGetReadAvail(fBuf);
	if (0 == statBytesAvail)
		return 0;

#ifdef STATUS_BUFFER_STAT
	if ((uint)statBytesAvail > maxStatBuf) {
		maxStatBuf = (uint)statBytesAvail;
		__SoftDslPrintf(XdslCoreGetCurDslVars(), "FlattenBufferStatusStatT: maxLen=%ld, maxTotal=%ld", 0, maxStatLen, maxStatBuf);
	}
#endif

	p = (uint *)StretchBufferGetReadPtr(fBuf);
#ifdef FLATTEN_ADDR_ADJUST
	p = (uint *)(ADSL_ADDR_TO_HOST((uintptr_t)p));
#endif
#ifdef CONFIG_ARM64
	if(0 == ((uintptr_t)p >> 32))
		printSbufferPtrs(fBuf, "FlattenBufferStatusRead:");
#endif
	n = UnflattenStatus (p, status);
	n = (n + 3) & ~3;
#ifdef STATUS_BUFFER_STAT
	if ((uint)n  > maxStatLen) {
		maxStatLen = (uint)n;
		__SoftDslPrintf(XdslCoreGetCurDslVars(), "FlattenBufferStatusStatM: maxLen=%ld, maxTotal=%ld", 0, maxStatLen, maxStatBuf);
	}
#endif
	p += (n >> 2);
	if (BUFFER_ANCHOR == ADSL_ENDIAN_CONV_INT32(*p))
		return n + 4;

	/* sync lost in status buffer. Try to recover */
	CircBufferSetRdPtrToWrPtr(fBuf);
	
	return -(int)(n+4);
}


#ifdef SUPPORT_STATUS_BACKUP
Public int	BackUpFlattenBufferStatusRead(stretchHostBufferStruct *fBuf, dslStatusStruct *status)
{
	uint	*p;
	int		statBytesAvail, n;
	
	statBytesAvail = HostStretchBufferGetReadAvail(fBuf);
	if (0 == statBytesAvail)
		return 0;
	
#ifdef STATUS_BUFFER_STAT
	if ((uint)statBytesAvail > maxStatBuf) {
		maxStatBuf = (uint)statBytesAvail;
		__SoftDslPrintf(XdslCoreGetCurDslVars(), "FlattenBufferStatusStatT: maxLen=%ld, maxTotal=%ld", 0, maxStatLen, maxStatBuf);
	}
#endif
	p = HostStretchBufferGetReadPtr(fBuf);

	n = UnflattenStatus (p, status);
	n = (n + 3) & ~3;
#ifdef STATUS_BUFFER_STAT
	if ((uint)n  > maxStatLen) {
		maxStatLen = (uint)n;
		__SoftDslPrintf(XdslCoreGetCurDslVars(), "FlattenBufferStatusStatM: maxLen=%ld, maxTotal=%ld", 0, maxStatLen, maxStatBuf);
	}
#endif
	p += (n >> 2);
	if (BUFFER_ANCHOR == ADSL_ENDIAN_CONV_INT32(*p))
		return n + 4;

	/* sync lost in status buffer. Try to recover */
	CircBufferSetRdPtrToWrPtr(fBuf);
	
	return -(int)(n+4);
}
#endif

Public int	FlattenBufferCommandWrite(stretchBufferStruct *fBuf, dslCommandStruct *cmd)
{
	uint	*p;
	int 	statBytesAvail, n;

	statBytesAvail = StretchBufferGetWriteAvail(fBuf);
	if (statBytesAvail < 24) {
		return -1;
	}

	p = (uint *) StretchBufferGetWritePtr(fBuf);
#ifdef FLATTEN_ADDR_ADJUST
	p	= (void *)(ADSL_ADDR_TO_HOST((uintptr_t)p));
#endif
#ifdef CONFIG_ARM64
	if(0 == ((uintptr_t)p >> 32))
		printSbufferPtrs(fBuf, "FlattenBufferCommandWrite:");
#endif
	n = FlattenCommand (cmd, p, (uint)statBytesAvail-4);
	if (n > 0) {
		n = (n + 3) & ~3;
		*(p + (n >> 2)) = ADSL_ENDIAN_CONV_INT32(BUFFER_ANCHOR);
#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
		__asm volatile("dmb st" ::: "memory");
#endif
		StretchBufferWriteUpdate (fBuf, n+4);
		return n+4;
	}
	return 0;
}
#endif /* ADSLCORE_ONLY */

#ifndef HOST_ONLY
Public int	FlattenBufferCommandRead(stretchBufferStruct *fBuf, dslCommandStruct *cmd)
{
	uint	statBytesAvail, n, *p;

	statBytesAvail = StretchBufferGetReadAvail(fBuf);
	if (0 == statBytesAvail)
		return 0;

	p = StretchBufferGetReadPtr(fBuf);
	n = UnflattenCommand(p, cmd);
	n = (n + 3) & ~3;
	p += (n >> 2);
	if (BUFFER_ANCHOR == *p)
		return n + 4;

	/* sync lost in command buffer. Try to recover */

	CircBufferGetReadPtr(fBuf) = CircBufferGetWritePtr(fBuf);
	return -(int)(n+4);
}
#endif /* HOST_ONLY */
