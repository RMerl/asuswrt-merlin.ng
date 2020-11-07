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

/**************************************************************
 *
 * StatusParser.h
 *
 *
 * Copyright (c) 1993-1997 AltoCom, Inc. All rights reserved.
 * Authors: Mark Gonikberg, Haixiang Liang.
 *
 * $Revision: 1.15 $
 *
 * $Id: StatusParser.h,v 1.15 2015/04/22 23:46:50 ilyas Exp $
 *
 * $Log: StatusParser.h,v $
 * Revision 1.15  2015/04/22 23:46:50  ilyas
 * Print Q8 numbers with decimal point only in DiagsParser or when high debug level is set -d3
 *
 * Revision 1.14  2014/07/03 01:19:26  tonytran
 * Save PHY features in a global array and provide an API to check for supported features
 *
 * Revision 1.13  2014/05/07 22:03:33  ilyas
 * Add printing dataas a string; support accumulated format string
 *
 * Revision 1.12  2013/08/09 03:21:58  ilyas
 * Add \n as necessary for dslsim status printing
 *
 * Revision 1.11  2012/10/24 02:41:13  ilyas
 * Make Matlab parser store results in multiple files to support parsing large log files
 *
 * Revision 1.10  2012/07/18 05:52:53  ilyas
 * Added state for proprietary vectoring counter in OH messages
 *
 * Revision 1.9  2012/07/11 23:53:40  ilyas
 * Modified API usage
 *
 * Revision 1.8  2012/07/11 20:07:50  ovandewi
 * SW588: pass bitmap ctrl to matlab parsing
 *
 * Revision 1.7  2012/07/11 16:05:21  ilyas
 * Added common API for parser output control
 *
 * Revision 1.6  2012/07/03 21:03:51  ghobrial
 * sw593: Add support for messages from WLAN and other drivers
 *
 * Revision 1.5  2012/06/27 18:09:58  ilyas
 * Started parser state
 *
 * Revision 1.4  2012/03/11 03:29:53  ilyas
 * Added G.993.2 message parser
 *
 * Revision 1.3  2002/09/07 01:43:59  ilyas
 * Added support for OEM parameters
 *
 * Revision 1.2  2000/04/13 08:39:40  yura
 * Added SM_DECL prefix
 *
 * Revision 1.1  1999/01/27 22:33:25  liang
 * Copied from SoftModem_3_1_02.
 *
 * Revision 1.2  1998/07/16 16:57:28  scott
 * Made GetModulationString a public function
 *
 * Revision 1.1  1998/03/27 17:49:25  ilyas
 * Created a separate file StatusParser.h;
 * This allows us to use StatusParser.c in other places such
 * as DiagsStatusMonitor.
 *
 *
 ***************************************************************/
#ifndef _StatusParser_H_
#define	_StatusParser_H_

#ifdef BCM63XX_SRC
#define LOG10
#else
#define LOG10 	log10
#endif

#if defined(MIPS_SRC) && !defined (XDSLDRV_ENABLE_PARSER)
#define SPrintF(dst, fmt, ...) ( {							\
	static const char fmt_slow[] SLOW_CONST = fmt;			\
	sprintf(dst,fmt_slow, ## __VA_ARGS__);					\
} )
#define _SPrintF        sprintf
#else
#define SPrintF         sprintf
#define _SPrintF        sprintf
#endif

#define VSPrintF        vsprintf

extern	int 	SM_DECL		StatusParser		(modemStatusStruct *status, char *dstPtr);
extern	char*	SM_DECL		GetModulationString(modulationMap);

extern  int	PrintByteBufferData(char *dstPtr, char *msg, int nPoints, unsigned char *dataPtr);
extern  int	PrintUCharBufferData(char *dstPtr, char *msg, int nPoints, unsigned char *dataPtr);
extern  int PrintShortBufferData(char *dstPtr, char *msg, int nPoints, short *dataPtr);

extern  int PrintBufferDataGen(modemStatusStruct *status, char *dstPtr);

#define kParserStateUndefined	-1

typedef struct {
    char        NLpValid;      /*Indicate if NLp[] parser state information is valid or not*/
	char		trainingState;
	char		modType;
	char		tmType[2];     /* DS / US */
	char		fireMap;
	char		ginpMap;
	char		vectCntReport;
	char        NLp[2];  /*Latency path number DS / USs*/
} statusParserState;

extern statusParserState *GetStatusParserStatePtr(int lineId);

/* Parser filter control flags */

#define	kParserFilterOvhMsg			0x00000001
#define	kParserFilterShwtmMrgn		0x00000002
#define	kParserFilterShwtmSnr		0x00000004
#define	kParserFilterGainTbl		0x00000008
#define	kParserFilterBitTbl			0x00000010
#define	kParserFilterSocMsg			0x00000020
#define	kParserFilterPerToneData	0x00000040
#define	kParserFilterClrEocMsg		0x00000080
#define	kParserFilterVectErrSmp		0x00000100
#define kParserFilterRfbck			0x00000200
#define kParserNotFilterDecQ8		0x00000400

/* Parser output control flags */

#define	kParserOutSingleMatFile		0x00000001  /*By default, it's large log file mode, MAT result written to multiple MAT files.*/

typedef struct {
	unsigned int	ctrlFilter;
	char			matlabOutput;
	unsigned int	ctrlOutput;
	unsigned int	phyFeatures[4];
} statusParserControl;

extern statusParserControl* GetStatusParserCtrlPtr(void);

extern unsigned int GetStatusParserFilter(void);
extern void			 SetStatusParserFilter(unsigned int fltr);

extern char			 GetStatusParserMatlabOutputState(void);
extern void			 SetStatusParserMatlabOutputState(unsigned int state);

extern unsigned int GetStatusParserMatlabOutputOption(void);
extern int			 IsStatusParserSignleMatlabFile(void);
extern void			 SetStatusParserMatlabOutputOption(unsigned int state);

extern char *		 GetStatusParserFmtStr(int *pLen);
extern void			 ClrStatusParserFmtStr(void);
extern unsigned int XdslFeatureSupported(unsigned int feature);

#endif	/* _StatusParser_H_ */
