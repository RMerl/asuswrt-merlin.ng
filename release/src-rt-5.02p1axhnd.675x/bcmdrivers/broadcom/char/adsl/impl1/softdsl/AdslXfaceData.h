/* 
* <:copyright-BRCM:2002:proprietary:standard
* 
*    Copyright (c) 2002 Broadcom 
*    All Rights Reserved
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
*/ 
/*
<:copyright-broadcom 
 
 Copyright (c) 2002 Broadcom Corporation 
 All Rights Reserved 
 No portions of this material may be reproduced in any form without the 
 written permission of: 
          Broadcom Corporation 
          16215 Alton Parkway 
          Irvine, California 92619 
 All information contained in this document is Broadcom Corporation 
 company private, proprietary, and trade secret. 
 
:>
*/
/****************************************************************************
 *
 * AdslXfaceData.h -- ADSL Core interface data structure
 *
 * Description:
 *	To be included both in SoftDsl and BcmAdslCore driver
 *
 *
 * Copyright (c) 2000-2001  Broadcom Corporation
 * All Rights Reserved
 * No portions of this material may be reproduced in any form without the
 * written permission of:
 *          Broadcom Corporation
 *          16215 Alton Parkway
 *          Irvine, California 92619
 * All information contained in this document is Broadcom Corporation
 * company private, proprietary, and trade secret.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.20 $
 *
 * $Id: AdslXfaceData.h,v 1.20 2014/09/04 23:00:41 ilyas Exp $
 *
 * $Log: AdslXfaceData.h,v $
 * Revision 1.20  2014/09/04 23:00:41  ilyas
 * Indicate 30b capability to DSL driver
 *
 * Revision 1.19  2014/07/17 02:11:19  ilyas
 * Added definitions and dual MIPS indication to DSL driver
 *
 * Revision 1.18  2014/05/30 02:26:34  ilyas
 * Added parameter exchange section to get pointer for driver allocated memory (below 16MB)
 *
 * Revision 1.17  2013/10/31 06:59:28  ilyas
 * Added Gfast indication to DSL driver
 *
 * Revision 1.16  2013/02/21 22:43:01  ilyas
 * Made AttnDr computation method configurable
 *
 * Revision 1.15  2009/11/24 17:39:32  rwdu
 * added kAdslPhyE14InmAdsl feature.
 *
 * Revision 1.14  2008/10/17 06:41:30  tonytran
 * Added support for AFE input data logging and playback for 6368 in ADSL mode
 *
 * Revision 1.13  2008/09/03 20:00:29  ilyas
 * Added 6306 support indication (PHY feature) for xDSL driver to know which AfeID to use
 *
 * Revision 1.12  2008/08/14 18:14:05  ilyas
 * Added 30a support indication
 *
 * Revision 1.11  2008/07/22 23:30:16  ilyas
 * Bonding capability indication and saved EPC address
 *
 * Revision 1.10  2007/10/09 20:20:08  ilyas
 * Added PHY configuration command to 6368. VDSL configuration TBD
 *
 * Revision 1.9  2004/02/03 02:57:22  ilyas
 * Added PHY feature settings
 *
 * Revision 1.8  2003/07/18 04:50:21  ilyas
 * Added shared buffer for clEoc messages to avoid copying thru command buffer
 *
 * Revision 1.7  2003/02/25 00:46:32  ilyas
 * Added T1.413 EOC vendor ID
 *
 * Revision 1.6  2003/02/21 23:29:13  ilyas
 * Added OEM vendor ID parameter for T1.413 mode
 *
 * Revision 1.5  2002/09/13 21:17:12  ilyas
 * Added pointers to version and build string to OEM interface structure
 *
 * Revision 1.4  2002/09/07 04:16:29  ilyas
 * Fixed HOST to ADSL MIPS SDRAM address translation for relocatable images
 *
 * Revision 1.3  2002/09/07 01:43:59  ilyas
 * Added support for OEM parameters
 *
 * Revision 1.2  2002/01/22 19:03:10  khp
 * -put sdramBaseAddr at end of Xface struct
 *
 * Revision 1.1  2002/01/15 06:25:08  ilyas
 * Initial implementation of ADSL core firmware
 *
 ****************************************************************************/

#ifndef	AdslXfaceDataHeader
#define AdslXfaceDataHeader

#include "CircBuf.h"

typedef struct _AdslXfaceData {
	stretchBufferStruct sbSta;
	stretchBufferStruct sbCmd;
	uint		gfcTable[15];
	uint		sdramBaseAddr;
} AdslXfaceData;

#define XFACE_SDRAM_PHYS	15
#define XFACE_SDRAM_VIRT	14
#define XFACE_SDRAM_VIRT32	13   /* for 64bit Host processor */
#define XFACE_EXCP_REQ		12

/* Parameter exchange structure (in phy_param_data section) */

#define DSL_PARAM_VERSION	0

typedef struct _DslParamData {
	ushort      len;    /* length of the structure */
	uchar       verId;  /* version ID */
	uchar       ctrl;   /* reserved */
	uint      lowSdramSize;    /* required size of low (below 16MB SDRAM) */
} DslParamData;

/* Shared SDRAM configuration data */

#define	kAdslOemVendorIdMaxSize		8
#define	kAdslOemVersionMaxSize		32
#define	kAdslOemSerNumMaxSize		32
#define	kAdslOemNonStdInfoMaxSize	64

typedef struct _AdslOemSharedData {
	uint	g994VendorIdLen;
	uint	g994XmtNonStdInfoLen;
	uint	g994RcvNonStdInfoLen;
	uint	eocVendorIdLen;
	uint	eocVersionLen;
	uint	eocSerNumLen;
	uchar		g994VendorId[kAdslOemVendorIdMaxSize];
	uchar		eocVendorId[kAdslOemVendorIdMaxSize];
	uchar		eocVersion[kAdslOemVersionMaxSize];
	uchar		eocSerNum[kAdslOemSerNumMaxSize];
	uchar		g994XmtNonStdInfo[kAdslOemNonStdInfoMaxSize];
	uchar		g994RcvNonStdInfo[kAdslOemNonStdInfoMaxSize];
	uint	gDslVerionStringPtr;
	uint	gDslBuildDataStringPtr;
	uint	t1413VendorIdLen;
	uchar		t1413VendorId[kAdslOemVendorIdMaxSize];
	uint	t1413EocVendorIdLen;
	uchar		t1413EocVendorId[kAdslOemVendorIdMaxSize];
	uint	clEocBufLen;
	uint	clEocBufPtr;
} AdslOemSharedData;

/* feature list */ 

#define	kAdslPhyAnnexA				0
#define	kAdslPhyAnnexB				1
#define	kAdslPhyAnnexC				2
#define	kAdslPhySADSL				3
#define	kAdslPhyAdsl2				4
#define	kAdslPhyAdslG992p3			4
#define	kAdslPhyAdsl2p				5
#define	kAdslPhyAdslG992p5			5
#define	kAdslPhyAnnexI				6
#define	kAdslPhyAdslReAdsl2			7
#define	kAdslPhyG992p2Init			8
#define	kAdslPhyT1P413				9
#define	kAdslPhyVdslG993p2			10
#define	kAdslPhyBonding				11
#define	kAdslPhyVdsl30a				12
#define	kAdslPhy6306				13
#define	kAdslPhyPlayback			14
#define	kAdslPhyE14InmAdsl			15
#define	kAdslPhyAttnDrAmd1			16
#define	kAdslPhyGfast				17
#define	kAdslPhyRemoteLOF			18
#define	kAdslPhyDualMips			19
#define	kAdslPhyVdslBrcmPriv1		20
#define	kAdslPhyGfastDynEocLen		21
#define	kAdslPhyGfastRetrainCmd     22
#define	kAdslPhyVdslBrcmPriv2		23
#define	kAdslPhyGfast212a			24
#define	kAdslPhyVmAddrSupport		25

#define	AdslFeatureSupported(fa,f)	((fa)[(f) >> 5] & (1 << ((f) & 0x1F)))
#define	AdslFeatureSet(fa,f)		(fa)[(f) >> 5] |= (1 << ((f) & 0x1F))

#endif /* AdslXfaceDataHeader */
