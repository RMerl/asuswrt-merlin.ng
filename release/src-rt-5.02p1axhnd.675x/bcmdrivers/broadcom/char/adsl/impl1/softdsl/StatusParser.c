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

/****************************************************************************
 *
 * StatusParser.c -- SoftModem status monitoring module (part of the test suit)
 *
 *
 *
 * Copyright (c) 1993-1997 AltoCom, Inc. All rights reserved.
 * Authors: Mark Gonikberg, Haixiang Liang.
 *
 * $Revision: 1.330 $
 *
 * $Id: StatusParser.c,v 1.330 2015/11/03 22:49:54 ilyas Exp $
 *
 * $Log: StatusParser.c,v $
 * Revision 1.330  2015/11/03 22:49:54  ilyas
 * Added Gfast eoc CRC error counter
 *
 * Revision 1.329  2015/10/23 00:33:11  ilyas
 * Added Diags support for new print macros
 *
 * Revision 1.328  2015/10/14 00:24:11  tonytran
 * Add parser print and view plot for ALN data
 *
 * Revision 1.327  2015/09/24 18:10:01  ghobrial
 * FW8620: Add and report EOC counters
 *
 * Revision 1.326  2015/09/24 03:23:19  chungtse
 * FWDSLCPEPHY-8595: Add kDsl993p2NeRxPower status for G.Fast RXpower_dBm_DS
 *
 * Revision 1.325  2015/06/22 23:50:49  ilyas
 * Print VDSL Hlin data for SELT results
 *
 * Revision 1.324  2015/05/08 21:47:05  tonytran
 * Print G.fast counter indications
 *
 * Revision 1.323  2015/05/05 02:01:15  tonytran
 * Improve the detection of the G.fast G.997 message header. Print the G.997 message header for messages handled in PHY similar to those messages handled in the driver.
 *
 * Revision 1.322  2015/04/22 23:46:50  ilyas
 * Print Q8 numbers with decimal point only in DiagsParser or when high debug level is set -d3
 *
 * Revision 1.321  2015/04/20 19:39:12  ilyas
 * Don't re-print number w/o decimal point
 *
 * Revision 1.320  2015/04/18 06:51:24  ilyas
 * Print Q8 numbers with decimal point
 *
 * Revision 1.319  2015/04/18 06:36:49  ilyas
 * Fixed decimal point conversion
 *
 * Revision 1.318  2015/03/25 12:54:35  ovandewi
 * PR8359: log print
 *
 * Revision 1.317  2015/03/20 20:28:46  ruizhewu
 * reduce the Q8 parser to 4 decimal
 *
 * Revision 1.316  2015/03/07 01:58:04  tonytran
 * Add support for SNRm status with margin data in Q8 format. Include the display of Attn, Pwr and Max rate in Diags State Windows
 *
 * Revision 1.315  2015/02/23 19:04:27  ruizhewu
 * add support to 64-bit Hex and Decimal print
 *
 * Revision 1.314  2015/01/24 01:53:01  tonytran
 * Print ADSL RNC QLN data to log file
 *
 * Revision 1.313  2015/01/16 23:37:41  ruizhewu
 * Remove all the warnings in Linux
 *
 * Revision 1.312  2014/11/20 00:23:44  tonytran
 * Update EOC status header print to accomodate the new cmd/rsp bit in new CO and display hdr len in place of MsgNum field
 *
 * Revision 1.311  2014/10/31 18:43:01  sgote
 * F-8144: Made both vdsl bonding target and gfast target train up in handshake and advertize both a/vdsl and gfast during hs initiatlization and send a flag to the host after we receive the cl if the gfast is supported by the dslam to switch the phy if needed. Currently this feature is disabled. It can be enabled by enabling the define GFAST_VDSL_MULTIMODE in the makefile.
 *
 * Revision 1.310  2014/10/21 22:27:52  tonytran
 * Print 'Far End Ti' header for kDsl993p2FeTi status and Rx/Tx OvhMsg for G.fast mode
 *
 * Revision 1.309  2014/10/04 01:03:55  tonytran
 * In the parser, explicitly check for Negotiated Bandplan type iso assuming it is when it's not Physical Bandplan type
 *
 * Revision 1.308  2014/09/23 18:43:05  tonytran
 * Fixed non-Gfast run_chip builds problem
 *
 * Revision 1.307  2014/09/23 00:31:58  tonytran
 * Corrected the 30b profile configuration, framing parameter L print, and the display of PER and OR in VDSL2 30a profile mode problems. For old targets, convert VDSL2/ADSL MIB objects to the common MIB object(xdslInfo) and replaced reference to old MIB objects with the new common MIB object.
 *
 * Revision 1.306  2014/08/29 02:03:06  tonytran
 * Display modulation, counters, framing parameters, 2k tones SNR graph, look up string dbfile and fix G.inp counters print(print counters as unsigned numbers)
 *
 * Revision 1.305  2014/08/25 13:17:05  ovandewi
 * G.9701 message ids
 *
 * Revision 1.304  2014/08/22 01:26:49  mding
 * fix NTR Pll lock status report
 *
 * Revision 1.300.10.3  2014/08/22 01:11:35  mding
 * fix NTR Pll lock status report
 *
 * Revision 1.300.10.2  2014/08/20 00:41:14  mding
 * add status to report current NTR/PPS output line
 *
 * Revision 1.300.10.1  2014/08/16 00:14:28  mding
 * add phy status report of TOD time stamp to driver
 *
 * Revision 1.300  2014/07/23 23:52:50  ilyas
 * Added missing strcpy function
 *
 * Revision 1.299  2014/07/23 22:37:32  ilyas
 * Added SoftDslDpApiPrintfF implememtation when running in PHY
 *
 * Revision 1.298  2014/07/03 01:19:26  tonytran
 * Save PHY features in a global array and provide an API to check for supported features
 *
 * Revision 1.297  2014/06/26 01:28:43  tonytran
 * Add RNC Quiet Line Noise and Run RNC QLN Test menu for 138/148 chips.
 *
 * Revision 1.296  2014/05/10 00:24:13  ilyas
 * Added print for Wakeup request status
 *
 * Revision 1.295  2014/05/07 22:03:33  ilyas
 * Add printing dataas a string; support accumulated format string
 *
 * Revision 1.294  2014/01/30 01:47:16  mding
 * fix compilation error
 *
 * Revision 1.293  2014/01/30 01:18:22  mding
 * adding kDslNtrStates status reporting
 *
 * Revision 1.292  2013/11/15 00:25:23  ilyas
 * Allow one line control/reset another line
 *
 * Revision 1.291  2013/10/29 23:43:50  ilyas
 * Filter out %s from format string to prevent Diags/parser crash
 *
 * Revision 1.290  2013/08/09 21:16:58  ilyas
 * Return length correctly in soem case
 *
 * Revision 1.289  2013/08/09 04:03:10  ilyas
 * Add \n as necessary for dslsim status printing
 *
 * Revision 1.288  2013/08/09 03:21:55  ilyas
 * Add \n as necessary for dslsim status printing
 *
 * Revision 1.287  2013/08/02 20:26:56  ilyas
 * Eliminated building G994 files for target w/o g994p1
 *
 * Revision 1.286  2013/07/17 01:10:44  ilyas
 * Added prints for AfeInfo and PhyInfo statuses
 *
 * Revision 1.285  2013/07/13 00:36:47  ilyas
 * Fixed basic mgnt counter response parsing and added ATM counter parsing
 *
 * Revision 1.284  2013/05/04 00:30:48  mding
 * fix counter names
 *
 * Revision 1.283  2013/02/17 04:51:48  ilyas
 * Added prints for new ATTNDR statuses
 *
 * Revision 1.282  2013/02/04 22:44:58  ilyas
 * Add prints for new statuses
 *
 * Revision 1.281  2012/12/21 17:57:51  jknittel
 * FWDSLCPEPHY-7103: Add AnnexJ Modulation decoding to DiagsParser
 *
 * Revision 1.280  2012/10/24 02:41:13  ilyas
 * Make Matlab parser store results in multiple files to support parsing large log files
 *
 * Revision 1.279  2012/10/19 21:53:16  ilyas
 * Added print for reInit time threshold status
 *
 * Revision 1.278  2012/09/10 09:51:27  ovandewi
 * PR6881/6748: vectoring info passing
 *
 * Revision 1.277  2012/07/18 23:15:54  lockem
 * Clean up trailing white space.
 *
 * Revision 1.276  2012/07/18 05:52:53  ilyas
 * Added state for proprietary vectoring counter in OH messages
 *
 * Revision 1.275  2012/07/11 23:53:39  ilyas
 * Modified API usage
 *
 * Revision 1.274  2012/07/11 20:07:50  ovandewi
 * SW588: pass bitmap ctrl to matlab parsing
 *
 * Revision 1.273  2012/07/11 16:21:00  ilyas
 * Added initialization and definition check
 *
 * Revision 1.272  2012/07/11 16:05:21  ilyas
 * Added common API for parser output control
 *
 * Revision 1.271  2012/07/10 09:20:20  ovandewi
 * PR6812: driver enable bit
 *
 * Revision 1.270  2012/07/06 16:54:53  ilyas
 * Added new status to sprintf string in Diags
 *
 * Revision 1.269  2012/07/03 21:03:51  ghobrial
 * sw593: Add support for messages from WLAN and other drivers
 *
 * Revision 1.268  2012/06/27 18:09:58  ilyas
 * Started parser state
 *
 * Revision 1.267  2012/06/25 18:57:53  ghobrial
 * Remove client specific definition from DSL StatusParser
 *
 * Revision 1.266  2012/06/21 18:28:47  ghobrial
 * PR-SWDSLCPEPHY-417 Define and implement API for other drivers to use DslDiags
 *
 * Revision 1.265  2012/06/19 20:54:03  ilyas
 * Added print for Ikanos CO4 detection
 *
 * Revision 1.264  2012/04/30 18:29:37  tonytran
 * Fixed the Simulation build/compile problem
 *
 * Revision 1.263  2012/04/20 23:38:56  tonytran
 * load StrDb from archived folder and print VectorErrorSample header info
 *
 * Revision 1.262  2012/03/28 21:37:15  tonytran
 * Added vdslafecfg= and runqlntest= commands; put back vdslafecfg from the GUI
 *
 * Revision 1.261  2012/03/11 03:29:53  ilyas
 * Added G.993.2 message parser
 *
 * Revision 1.260  2012/03/07 02:26:43  ilyas
 * Add printing bit reversed data to SoftDslPrintData()
 *
 * Revision 1.259  2012/02/17 01:14:51  rgreenf
 * PR6457 add ROC SNR status report
 *
 * Revision 1.258  2012/01/21 02:30:51  jinlu
 * FW6401 add status parser
 *
 * Revision 1.257  2012/01/13 23:29:41  tonytran
 * Added bonding support in DiagRecTxt and removed warnings
 *
 * Revision 1.256  2011/11/08 00:48:24  rgreenf
 * PR6155: implement amd7 kl0 measurement
 *
 * Revision 1.255  2011/11/04 03:59:21  ilyas
 * Showed hex display of VDSL profile, annex and mask
 *
 * Revision 1.254  2011/10/28 23:30:33  jknittel
 * FWDSLCPEPHY-6286: Added StatusParser code to handle new Bi/Gi Reporting
 *
 * Revision 1.253  2011/10/13 18:35:38  rgreenf
 * PR6052: new PSD and profile statuses to statusParser
 *
 * Revision 1.252  2011/09/02 06:06:22  sgote
 * FWDSLCPEPHY-5553:  Missed this status parser change in the previous checkin
 *
 * Revision 1.251  2011/08/29 18:11:55  sgote
 * FWDSLCPEPHY-5553: kDslRetrainReason not reported in VDSL training Added some of the VDSL retrain codes in the SoftDSl.h and mapped those to lineMgr exception codes
 *
 * Revision 1.250  2011/08/23 18:36:49  ilyas
 * Fixed simulation target build
 *
 * Revision 1.249  2011/08/21 21:13:11  ilyas
 * Print bonding discovery exchange messages
 *
 * Revision 1.248  2011/07/22 00:38:53  yinboli
 * FW5909: Add loop attenuation average for inner/outer pair detection
 *
 * Revision 1.247  2011/06/22 20:02:24  ilyas
 * Added status print for kDslATUCShowtimeXmtPowerInfo
 *
 * Revision 1.246  2011/04/19 21:04:18  pauljr
 * PR5636 Modify reporting structure to driver to support INPs greater than 255 half symbols
 *
 * Revision 1.245  2011/04/14 16:34:55  ilyas
 * Put DDR memory controller in self refresh (SR) mode when Host MIPS is not busy(using it)
 *
 * Revision 1.244  2011/03/28 22:17:45  tonytran
 * Add prints for Eoc commands and clean up code that handle the kDslReceivedEocCommand status
 *
 * Revision 1.243  2011/03/17 22:24:42  ilyas
 * Ported DiagsParser to Linux
 *
 * Revision 1.242  2011/03/10 22:45:40  ovandewi
 * vectoring changes
 *
 * Revision 1.241  2011/03/03 03:12:35  tonytran
 * Implement File Overlapped IO, reduce non-page memory usage and combine short statues before submit for writing into a file
 *
 * Revision 1.240  2010/10/29 22:47:55  tonytran
 * Added printing of SEFTR
 *
 * Revision 1.239  2010/10/12 02:06:21  tonytran
 * Use a newly created status code to report status buffer write histogram data. Removed TIME_PROFILING related code from the previous checked-in
 *
 * Revision 1.238  2010/10/11 22:00:17  tonytran
 * Added status buffer usage histogram. Disabled by default and can be enabled by defining SUPPORT_STATUS_HISTOGRAM in DslCoreXface.h
 *
 * Revision 1.237  2010/10/08 18:51:16  ilyas
 * Dont write Printf statuses when Diags is not connected or when buffer close to full
 *
 * Revision 1.236  2010/08/21 01:40:30  chungtse
 * Add support for last transmitted state upstream and downstream in ADSL2/2+
 *
 * Revision 1.235  2010/08/13 21:47:28  ilyas
 * Added TR98 status support in Diags
 *
 * Revision 1.234  2010/06/26 01:35:25  tonytran
 * Fixed S-298: MIB Data missing. Removed status indentation when parsing statuses in Bonding Slave GUI. Included print of ETR_kbps when parsing kDsl993p2FramerDeframer status and display INPrein in DSL State window in Q0 iso Q1 format. Cleaned up profiling code.
 *
 * Revision 1.233  2010/06/24 22:42:29  jknittel
 * FWDSLCPEPHY-4483: Ported Adsl1 missing phone filter detector to CPE code
 *
 * Revision 1.232  2010/06/19 04:10:25  tonytran
 * Fixed the frame buffer lost when connection restarted and an intermitten divide by zero in proxy service mode.
 * Added flushstatus= command for flusing status queue.
 * Improve GUI responsiveness by yielding CPU when in loop processing/writing statuses.
 * Centralized status forwarding in proxy service mode.
 *
 * Revision 1.231  2010/06/04 07:01:09  ovandewi
 * PR4597: print vendor ID properly
 *
 * Revision 1.230  2010/06/03 15:19:12  ovandewi
 * PR4628: add R-FEEDBACK id for pick-up in diags
 *
 * Revision 1.229  2010/05/05 20:24:16  ilyas
 * Added Ginp framing paramters and counters display
 *
 * Revision 1.228  2010/03/23 15:07:39  ilyas
 * Properly display xTM counters status
 *
 * Revision 1.227  2010/03/11 02:06:05  lockem
 * Align field names with E14 code.
 *
 * Revision 1.226  2010/02/12 15:39:13  ovandewi
 * PR3904: add vectoring processing
 *
 * Revision 1.225  2009/12/04 22:56:18  tonytran
 * Fixed the DslDiags crash during regression test
 *
 * Revision 1.224  2009/09/07 13:43:08  jboxho
 * Add logging of G994 Vendor ID PR(33211)
 *
 * Revision 1.223  2009/09/05 00:38:46  ilyas
 * Identify bandplan type in printf
 *
 * Revision 1.222  2009/08/12 23:59:46  ilyas
 * Created section for constants in SDRAM - SLOW_CONST
 *
 * Revision 1.221  2009/07/21 18:35:00  ilyas
 * Fixed compiler warnings
 *
 * Revision 1.220  2009/05/04 14:00:24  ovandewi
 * remove spurious prints for Diags
 *
 * Revision 1.219  2009/05/03 17:03:40  ilyas
 * Support new status for E14 DBprints in Diags (for better binary matching in source release)
 *
 * Revision 1.218  2009/03/24 01:15:54  ilyas
 * div0 exception control
 *
 * Revision 1.217  2009/02/16 23:08:36  ovandewi
 * report bonding type
 *
 * Revision 1.216  2009/02/04 21:32:20  ilyas
 * Fixed lineID extraction
 *
 * Revision 1.215  2009/02/04 04:18:31  ilyas
 * Added line ID display to separate statuses for different lines
 *
 * Revision 1.214  2009/01/24 01:34:48  tonytran
 * Clean up
 *
 * Revision 1.213  2009/01/22 02:06:50  tonytran
 * Added support for bonded modem 1st phase
 *
 * Revision 1.212  2009/01/14 04:09:11  mhegde
 * PR32543: Display Non linear Noise count
 *
 * Revision 1.211  2008/12/11 21:57:54  ilyas
 * Add lineID display and indent status strings for line1 (for bonding)
 *
 * Revision 1.210  2008/09/26 03:25:04  dadityan
 * Added parse case for status kDslTestHlin , change in Diags.h needed to compile
 *
 * Revision 1.209  2008/07/01 22:29:43  dadityan
 * Change Status format QLN and NE Gain
 *
 * Revision 1.208  2008/06/26 19:25:38  tonytran
 * Added HDLC checksum to DslDiagsParser and fixed the wrong display of PER, OR, INP and Delay when the value in the tenth place is zero and the hundredth place is non zero(ie. 12.07 will be displayed as 12.7)
 *
 * Revision 1.207  2008/06/05 18:07:59  ovandewi
 * report line driver type
 *
 * Revision 1.206  2008/05/06 00:36:50  ilyas
 * Fixed BERT result display
 *
 * Revision 1.205  2008/04/28 11:42:44  rgreenf
 * add new feq scaling state to state reporting
 *
 * Revision 1.204  2008/04/24 19:24:35  ilyas
 * Report extra connection parameters such as AHIF channel ID TC type, etc. in ADSL mode
 *
 * Revision 1.203  2008/03/28 17:32:34  rgreenf
 * add band plan phase status
 *
 * Revision 1.202  2008/03/04 00:10:25  ilyas
 * Added SoftDslDBPrintf to print with phase and total symbol counters
 *
 * Revision 1.201  2008/01/29 21:10:35  tonytran
 * Added support for printing strings saved outside PHY through kDslGeneralMsgPrintf status; Fixed the wrong display of the second latency info when starting Diags after the CPE was connected in dual latency mode
 *
 * Revision 1.200  2008/01/28 20:50:49  ilyas
 * Added preliminary support for using stringIDs in printf statuses
 *
 * Revision 1.199  2008/01/12 01:55:06  tonytran
 * fix dual latency display bug
 *
 * Revision 1.198  2008/01/09 16:57:03  rgreenf
 * allow pll during agc and remove extra relock state
 *
 * Revision 1.197  2007/12/19 00:24:01  ilyas
 * Added statuses for dual latency support
 *
 * Revision 1.196  2007/12/07 00:59:36  rgreenf
 * merge/implement LD mode
 *
 * Revision 1.195  2007/11/28 00:55:33  rgreenf
 * report near/far end tx power to dslDiags
 *
 * Revision 1.194  2007/11/08 23:53:34  ilyas
 * Filter out some G.993 messages from Lite version
 *
 * Revision 1.193  2007/10/15 03:51:31  mding
 * add SATNpb status call
 *
 * Revision 1.192  2007/10/05 02:41:42  mding
 * add G993 status report for showtime DS attainable net rate
 *
 * Revision 1.191  2007/10/04 00:39:36  mding
 * add G993 status report for SNRM/SNRMpb
 *
 * Revision 1.190  2007/09/27 00:14:31  ilyas
 * Fixed strlen prototype to makebuild with 3.4.6 (simulation target)
 *
 * Revision 1.189  2007/09/06 16:17:20  rgreenf
 * add attainable rate reporting
 *
 * Revision 1.188  2007/08/24 16:23:08  rgreenf
 * correct rx synchro state messages
 *
 * Revision 1.187  2007/08/24 00:35:05  rgreenf
 * add latency path id
 *
 * Revision 1.186  2007/08/23 20:56:42  rgreenf
 * add extra framer parameter reporting
 *
 * Revision 1.185  2007/08/20 21:34:05  rgreenf
 * added training phase 6368 status reporting - ongoing
 *
 * Revision 1.184  2007/08/20 17:41:10  rgreenf
 * added training phase 6368 status reporting - ongoing
 *
 * Revision 1.183  2007/08/20 16:34:19  rgreenf
 * added training phase 6368 status reporting - ongoing
 *
 * Revision 1.182  2007/08/10 16:42:55  tonytran
 * Support printing E14 debug prints with the exact format as its original through DslDiag status
 *
 * Revision 1.181  2007/08/02 18:44:05  ilyas
 * Removed un-necessary ifdef in modulation reporting
 *
 * Revision 1.180  2007/08/01 18:25:25  ilyas
 * Made build in Linux simulation targets
 *
 * Revision 1.179  2007/07/29 20:45:39  ilyas
 * Removed prints for WriteFile
 *
 * Revision 1.178  2007/07/24 17:41:53  raman
 * restore back compilation of run_all.csh
 *
 * Revision 1.177  2007/07/03 20:25:56  tonytran
 * Added support for block/afe test
 *
 * Revision 1.176  2007/06/26 18:55:10  ilyas
 * Added write-to-file capability to PHY
 *
 * Revision 1.175  2007/06/26 07:12:11  ilyas
 * Fixed DslDiags build problems on top of tree
 *
 * Revision 1.174  2007/06/19 01:21:13  ovandewi
 * remove from LITE version
 *
 * Revision 1.173  2007/05/23 21:48:55  ilyas
 * Made build with VDSL firmware
 *
 * Revision 1.172  2007/05/17 23:33:10  mprahlad
 * add vdsl modulation to modulation strings...
 *
 * Revision 1.171  2007/05/09 03:49:30  ilyas
 * Moved format strings to SDRAM for MIPS (simulation) builds
 *
 * Revision 1.170  2007/04/20 20:51:17  mprahlad
 * get StatusParser.c to compile for g994 6368 target - comment out things that
 * use log10 and double oprecision status for this target
 *
 * Revision 1.169  2007/04/13 06:33:33  tonytran
 * Added DS Delay and Inp statuses parsing; Changed phyCfg to keep old config when downloading new phy
 *
 * Revision 1.168  2007/02/15 23:27:18  tonytran
 * Fixed output log file access problem after log=0 or completed. Added profstart/profstop commands and display FIRE counters for US
 *
 * Revision 1.167  2007/02/15 17:05:56  ovandewi
 * parse 3rd counter of fire
 *
 * Revision 1.166  2007/01/25 01:25:44  tonytran
 * Added binary status output filtering; display Fire, AS and Bitswap counters in the State Summary Window
 *
 * Revision 1.165  2007/01/24 06:37:10  ovandewi
 * add broadband counter reset + some filter out diags lite
 *
 * Revision 1.164  2006/12/19 23:06:45  dadityan
 * G.994.1 Parsing flag removed
 *
 * Revision 1.162  2006/10/27 00:18:18  ilyas
 * Added definitions for time based profiling
 *
 * Revision 1.161  2006/10/24 22:20:22  dadityan
 * Retrain Reason Code Print conditional on valid code
 *
 * Revision 1.160  2006/09/04 14:36:31  ovandewi
 * add new retrain reasons
 *
 * Revision 1.159  2006/06/10 02:47:58  ilyas
 * Fixed display of negative numbers with 0 integer part
 *
 * Revision 1.158  2006/03/30 03:00:02  ilyas
 * Fixed typo: break on the same line as #endif causing erroneous print of AFE sample loss
 *
 * Revision 1.157  2006/03/23 07:42:51  ilyas
 * Added status prints for NL statuses
 *
 * Revision 1.156  2006/03/10 03:11:25  ilyas
 * Print PLN margin values in dB
 *
 * Revision 1.155  2006/02/16 13:06:10  ovandewi
 * add parsing information for afe sample drop, pln and retrain reason
 *
 * Revision 1.154  2006/01/06 18:03:36  ovandewi
 * print bins for PLN
 *
 * Revision 1.153  2006/01/05 06:32:28  ilyas
 * Support for new PLN statuses
 *
 * Revision 1.152  2006/01/04 16:39:43  jboxho
 * PLN command update: Impulse duration and inter-arrival bin definition tables & PLN status command
 *
 * Revision 1.151  2005/12/17 01:38:41  ilyas
 * Addded printout for PLN base message
 *
 * Revision 1.150  2005/12/08 18:52:14  ilyas
 * Display PLN margins used by PHY
 *
 * Revision 1.149  2005/11/28 15:59:44  ovandewi
 * render PLN as ushort
 *
 * Revision 1.148  2005/11/24 02:45:08  ilyas
 * Print PLN tables
 *
 * Revision 1.147  2005/11/22 20:45:55  ilyas
 * Added printouts for RDI and LD statuses
 *
 * Revision 1.146  2005/10/19 19:03:52  ilyas
 * Support margin level parameters for PLNStart command
 *
 * Revision 1.145  2005/10/14 00:33:11  ilyas
 * Added support for more statuses
 *
 * Revision 1.144  2005/10/05 08:09:10  ilyas
 * Added parsing of AFE sample buffer and PLN statuses. Added symbolic names to MIPS registers for exception status
 *
 * Revision 1.143  2005/08/04 20:13:49  ovandewi
 * report Annex M submode
 *
 * Revision 1.142  2005/07/29 02:51:25  kdu
 * PR30498: Use different status to report rcvd message in DELT mode
 *
 * Revision 1.141  2005/07/14 19:30:11  ilyas
 * Added macros to print data buffers and printf with string id
 *
 * Revision 1.140  2005/06/16 05:45:28  ilyas
 * Fixed margin display problem in Lite version
 *
 * Revision 1.139  2005/04/29 02:04:38  ilyas
 * In any SNR-type window parse and plot clipboard data on right mouse click
 *
 * Revision 1.138  2005/04/01 21:58:38  ilyas
 * Display AnnexM mode
 *
 * Revision 1.137  2004/10/23 00:16:32  nino
 * Added kDslHardwareSetRcvAGC status to set absolute rcv agc gain.
 *
 * Revision 1.136  2004/10/12 01:08:05  nino
 * Remove kDslHardwareEnablePwmSyncClk and kDslHardwareSetPwmSyncClkFreq statuses.
 *
 * Revision 1.135  2004/10/11 20:22:27  nino
 * Added kDslHardwareGetRcvPGA, kDslHardwareEnablePwmSyncClk and kDslHardwareSetPwmSynClkFreq hardware statuses.
 *
 * Revision 1.134  2004/10/02 00:27:12  nino
 * Added kDslHardwareAGCSetPga2 and kDslSetPilotEyeDisplay statuses.
 *
 * Revision 1.133  2004/09/30 04:36:35  ilyas
 * Print modulation code if can't translate
 *
 * Revision 1.132  2004/07/01 23:58:50  ilyas
 * Enabled OEM parameter status printout
 *
 * Revision 1.131  2004/06/04 22:46:23  ilyas
 * More status printout
 *
 * Revision 1.130  2004/05/25 16:14:04  ilyas
 * Added ADSL2 framing status
 *
 * Revision 1.129  2004/05/14 02:01:33  nino
 * Fixed the units on the attainable net data rate printout.
 *
 * Revision 1.128  2004/05/14 01:16:24  nino
 * Added kDslSignalAttenuation, kDslAttainableNetDataRate, kDslHLinScale and kDslAttainableNetDataRate statuses.
 *
 * Revision 1.127  2004/04/26 00:12:46  ilyas
 * Enabled Eoc status messages
 *
 * Revision 1.126  2004/03/17 05:48:50  ilyas
 * Added vendor ID printout
 *
 * Revision 1.125  2004/02/09 05:11:53  yongbing
 * Add ADSL2 bit swap function
 *
 * Revision 1.124  2004/02/03 19:15:16  gsyu
 * Added modulation string for G992P5
 *
 * Revision 1.123  2004/01/20 19:52:56  ilyas
 * Added OLR statuses
 *
 * Revision 1.122  2003/12/23 21:26:51  mprahlad
 * add status for kDslHWDisableDigitalECUpdate
 *
 * Revision 1.121  2003/12/05 19:26:42  yjchen
 * decide fast/interleave based on D
 *
 * Revision 1.120  2003/12/03 02:25:15  gsyu
 * Reverse previous check in for Double Upstream demo
 *
 * Revision 1.118  2003/11/01 01:05:43  linyin
 * Add message print for annexI
 *
 * Revision 1.117  2003/10/22 22:05:12  yjchen
 * print out channel response correctly
 *
 * Revision 1.116  2003/10/22 00:53:54  yjchen
 * add status reporting for quiet line noise
 *
 * Revision 1.115  2003/10/18 00:02:43  yjchen
 * add print out for G992P3 diagnostic mode channel response
 *
 * Revision 1.114  2003/10/14 22:02:37  ilyas
 * Added new Nino's AFE statuses for 6348
 *
 * Revision 1.113  2003/09/29 18:35:21  ilyas
 * Don't display ATM header compression message in LITE version
 *
 * Revision 1.112  2003/09/25 20:33:01  yjchen
 * type convert the value
 *
 * Revision 1.111  2003/09/18 23:56:24  ilyas
 * Commented out more statuses for Lite version
 *
 * Revision 1.110  2003/09/18 04:09:09  ilyas
 * Added support for Lite version of statuses.
 *
 * Revision 1.109  2003/08/22 22:52:50  liang
 * Print out fast or interleave path in coding param report, and fix some Linux compiler warnings.
 *
 * Revision 1.108  2003/07/19 05:38:07  mprahlad
 * revert back to rev1.106 for top of tree
 *
 * Revision 1.107  2003/07/19 05:37:25  mprahlad
 * if 0 kDslDataAvailStatus for annexi_a0_1g tag
 *
 * Revision 1.106  2003/07/14 19:59:10  yjchen
 * add print out for G992P3
 *
 * Revision 1.105  2003/07/11 20:08:16  ilyas
 * Fixed HWDigitalECUpadte status printout
 *
 * Revision 1.104  2003/07/07 23:40:48  ilyas
 * Added G.dmt.bis status messages
 *
 * Revision 1.103  2003/06/07 00:36:40  ilyas
 * Added more commands
 *
 * Revision 1.102  2003/05/29 21:12:03  nino
 * Removed refereces to kDslHWEnableAnalogECUpdate and kDslHWEnableAnalogEC. Added kDslHWSetDigitalEcUpdateMode.
 *
 * Revision 1.101  2003/04/02 03:57:02  mprahlad
 * make strlen() public so that it can e accessed from G992Test.c for bcm47xx
 * G992 test targets
 *
 * Revision 1.100  2003/01/13 23:14:07  ilyas
 * Exception status printout fix
 *
 * Revision 1.99  2002/12/10 23:25:51  ilyas
 * Changed exception status printout for DslDiags
 *
 * Revision 1.98  2002/12/06 02:40:59  liang
 * Fix typo in last checkin.
 *
 * Revision 1.97  2002/12/06 02:39:59  liang
 * Added printout for xmt 2x IFFT disable and T1.413 RAck1/RAck2 forced retrain.
 *
 * Revision 1.96  2002/10/31 01:27:12  ilyas
 * Added more info to eoc message statuses
 *
 * Revision 1.95  2002/10/04 23:26:24  liang
 * Changed bitswap related output messages.
 *
 * Revision 1.94  2002/09/28 03:01:28  yongbing
 * Add retrain in T1.413 with R-Ack1 tone
 *
 * Revision 1.93  2002/09/26 23:32:37  yongbing
 * Add synch symbol detection in Showtime
 *
 * Revision 1.92  2002/09/13 21:17:12  ilyas
 * Added pointers to version and build string to OEM interface structure
 *
 * Revision 1.91  2002/09/12 17:51:16  ilyas
 * Fixed compiler warning
 *
 * Revision 1.90  2002/09/07 16:50:42  ilyas
 * Added status prints for G.994 non standard info
 *
 * Revision 1.89  2002/09/07 01:43:59  ilyas
 * Added support for OEM parameters
 *
 * Revision 1.88  2002/07/19 20:26:08  liang
 * Added status printout for LOS, timing tone, power cutback and total power.
 *
 * Revision 1.87  2002/07/11 03:53:01  ilyas
 * Added Showtime SNR margin status support
 *
 * Revision 1.86  2002/06/15 05:19:10  ilyas
 * Added stack dump to exception status printout
 *
 * Revision 1.85  2002/06/05 21:07:01  yongbing
 * Remove for compiling errors in Linux build
 *
 * Revision 1.84  2002/05/31 03:37:51  ilyas
 * Chnaged counter's header printout to be compatible with Matlab script
 *
 * Revision 1.83  2002/05/30 19:59:40  ilyas
 * Added status for ADSL MIPS exception
 *
 * Revision 1.82  2002/05/07 23:21:48  mprahlad
 * support strlen for firmware record test
 *
 * Revision 1.81  2002/04/19 00:14:09  yongbing
 * Add more detailed error message reports during long training
 *
 * Revision 1.80  2002/04/02 10:11:13  ilyas
 * Merged BERT stuff with AnnexA branch
 *
 * Revision 1.79  2002/03/26 01:44:05  ilyas
 * Added timeout messages for annex C
 *
 * Revision 1.78  2002/03/22 19:44:11  yongbing
 * Add status messages for bit swap processing
 *
 * Revision 1.77  2002/03/22 01:18:47  ilyas
 * Add status messages for total FEXT Bits, NEXT Bits
 *
 * Revision 1.76  2002/01/18 03:31:21  ilyas
 * Added sprintf include
 *
 * Revision 1.75  2002/01/16 00:56:42  liang
 * For hardware TEQ output record test, print out clock tweak values.
 *
 * Revision 1.74  2002/01/10 07:18:23  ilyas
 * Added status for printf (mainly for ADSL core debugging)
 *
 * Revision 1.73  2002/01/08 04:54:26  ilyas
 * Always process I432 statuses, filtering is done in I432
 *
 * Revision 1.72  2001/12/13 21:43:07  liang
 * Fix compiler warning.
 *
 * Revision 1.71  2001/12/13 02:38:52  ilyas
 * Added support for G997 and Clear EOC
 *
 * Revision 1.70  2001/11/30 05:56:37  liang
 * Merged top of the branch AnnexBDevelopment onto top of the tree.
 *
 * Revision 1.69  2001/11/15 19:02:21  yongbing
 * Modify only T1.413 part to the top of tree based on AnnexBDevelopment branch
 *
 * Revision 1.68  2001/10/09 22:35:14  ilyas
 * Added more ATM statistics and OAM support
 *
 * Revision 1.56.2.17  2001/11/27 20:35:56  liang
 * Change EOC tx & rx message output formats.
 *
 * Revision 1.56.2.16  2001/11/21 01:30:34  georgep
 * Add a status message for annex C
 *
 * Revision 1.56.2.15  2001/11/10 00:49:13  yongbing
 * Add message for T1.413 Return to Handshake
 *
 * Revision 1.56.2.14  2001/11/08 23:26:53  yongbing
 * Add carrier selection function for Annex A and B
 *
 * Revision 1.56.2.13  2001/11/07 22:55:30  liang
 * Report G992 rcv msg CRC error as what it is instead of time out.
 *
 * Revision 1.56.2.12  2001/11/05 20:00:24  liang
 * Print out DC offset status value.
 *
 * Revision 1.56.2.11  2001/10/12 18:10:03  yongbing
 * Add support for T1.413
 *
 * Revision 1.56.2.10  2001/10/10 18:43:25  yongbing
 * Modify data buffer pointers to be printed to the type unsiged char
 *
 * Revision 1.56.2.9  2001/10/04 00:43:33  liang
 * Add TEQ coef and PSD reports, and change goto codes into functions.
 *
 * Revision 1.56.2.8  2001/10/03 02:03:55  liang
 * Merged with codes from main tree (tag SoftDsl_2_18).
 *
 * Revision 1.56.2.7  2001/09/28 22:10:04  liang
 * Add G994 exchange message status reports.
 *
 * Revision 1.56.2.6  2001/09/26 18:10:22  georgep
 * Send status error message in case features field is not setup properly
 *
 * Revision 1.56.2.5  2001/09/05 01:57:18  georgep
 * Added status message for annexC measured delay
 *
 * Revision 1.56.2.4  2001/08/18 00:21:24  georgep
 * Add status message for kDslAnnexCXmtCPilot1Starting
 *
 * Revision 1.56.2.3  2001/08/08 17:38:50  yongbing
 * Merge with tag SoftDsl_2_17
 *
 * Revision 1.67  2001/08/08 01:19:17  ilyas
 * Added support for EC coefficients status
 *
 * Revision 1.66  2001/06/21 23:06:23  ilyas
 * Added kAtmPrintCell status processing
 *
 * Revision 1.65  2001/05/16 06:22:24  liang
 * Added status reports for xmt & rcv prefix enable position.
 *
 * Revision 1.64  2001/04/26 02:25:43  ilyas
 * Commented out I432 HUNT and PRESYNC status parsing
 *
 * Revision 1.63  2001/04/24 21:41:21  ilyas
 * Implemented status flattening/unflattaning to transfer statuses between
 * modules asynchronously through the circular buffer
 *
 * Revision 1.62  2001/04/13 03:39:37  ilyas
 * Remove the last accidental checkin.
 *
 * Revision 1.60  2001/04/07 00:05:48  georgep
 * Added status for kDslHWSetupDigitalEcGainSHift
 *
 * Revision 1.59  2001/03/29 05:58:33  liang
 * Replaced the Aware compatibility codes with automatic detection codes.
 *
 * Revision 1.58  2001/03/25 06:11:25  liang
 * Combined separate loop attenuation status for ATUR & ATUC into one status.
 * Replace separate hardware AGC info status for ATUR & ATUC into hardware AGC
 * request status and hardware AGC obtained status.
 * Use store AGC command to save hardware AGC value instead of returning value
 * from status report.
 *
 * Revision 1.57  2001/03/24 00:43:22  liang
 * Report more checksum results (NumOfCalls, txSignal, rxSignal & eyeData).
 *
 * Revision 1.56  2001/03/17 00:05:24  georgep
 * Added loop attenuation reporting statuses
 *
 * Revision 1.55  2001/03/15 19:15:11  georgep
 * Disable atm printf
 *
 * Revision 1.54  2001/02/16 20:10:08  liang
 * Add a missing break statement.
 *
 * Revision 1.53  2001/02/09 20:49:41  georgep
 * Moved some statuses under kDslDspControlStatus
 *
 * Revision 1.52  2001/02/09 01:58:20  ilyas
 * Print ATM AAL packets
 *
 * Revision 1.51  2000/12/11 23:15:12  yongbing
 * Modify to the correct indexes of monitoring parameters
 *
 * Revision 1.50  2000/12/08 03:37:26  yongbing
 * Print performance monitoring parameters from other end
 *
 * Revision 1.49  2000/12/06 02:41:14  liang
 * Disable SNR info printout.
 *
 * Revision 1.48  2000/11/29 20:42:03  liang
 * Add report for SNR info and max achivable rate.
 *
 * Revision 1.47  2000/09/10 09:22:47  lkaplan
 * Improve interface for handling Eoc messages
 *
 * Revision 1.46  2000/09/08 19:37:58  lkaplan
 * Added code for handling EOC messages
 *
 * Revision 1.45  2000/09/01 00:48:50  georgep
 * Added Hardware AGC status
 *
 * Revision 1.44  2000/08/03 14:04:11  liang
 * Add hardware time tracking clock error reset code.
 *
 * Revision 1.43  2000/07/28 19:26:21  ilyas
 * Removed CRs
 *
 * Revision 1.42  2000/07/28 19:07:03  ilyas
 * Added NDIS4 driver
 *
 * Revision 1.41  2000/07/18 09:42:11  liang
 * Report rcv SNR margin in more meaningful format.
 *
 * Revision 1.40  2000/07/06 23:05:40  liang
 * Fix modulation string function and report selected modulation from G.994.1.
 * Report G.lite and G.dmt common status as G.992 instead of G.992.2.
 *
 * Revision 1.39  2000/06/21 20:11:38  georgep
 * Added status for clockTweak for HW Time Tracking
 *
 * Revision 1.38  2000/05/18 21:48:32  ilyas
 * Added status messages for preassigned cell headers
 *
 * Revision 1.37  2000/05/15 18:18:02  liang
 * Added statuses for DSL frames
 *
 * Revision 1.36  2000/05/14 01:54:27  ilyas
 * Added more statuses and cell printout
 *
 * Revision 1.35  2000/05/14 01:09:45  liang
 * Added no cell memory status
 *
 * Revision 1.34  2000/05/10 18:30:33  liang
 * Fix linux compiler warnings for ATM status.
 *
 * Revision 1.33  2000/05/09 23:03:49  ilyas
 * Added SoftDslTimer() logging and ATM status messages
 *
 * Revision 1.32  2000/05/02 00:04:40  liang
 * Add showtime monitoring and message exchange info reports.
 *
 * Revision 1.31  2000/04/21 23:10:28  liang
 * Added G992 time out training progress report.
 *
 * Revision 1.30  2000/04/15 01:50:25  georgep
 * Added T1p413 statuses
 *
 * Revision 1.29  2000/04/05 23:49:40  liang
 * Changed coding param struct name, changed Control module names from G992p2 to G992.
 *
 * Revision 1.28  2000/04/04 02:28:01  liang
 * Merged with SoftDsl_0_2 from old tree.
 *
 * Revision 1.27  2000/04/01 01:07:50  liang
 * Changed file names and some module names.
 *
 * Revision 1.26  2000/03/02 20:45:51  ilyas
 * Added support for ATM testing
 *
 * Revision 1.25  1999/11/11 19:24:07  george
 * Porting to 16Bit Compiler
 *
 * Revision 1.23  1999/11/09 20:33:10  george
 * Add reporting of Profile Error
 *
 * Revision 1.22  1999/11/05 01:27:06  liang
 * Add recovery-from-inpulse-noise progress report.
 *
 * Revision 1.21  1999/11/02 02:09:28  george
 * Added SNR Margin Reporting
 *
 * Revision 1.20  1999/10/07 23:31:02  wan
 * Add G.994.1 Tone and Fast Retrain Recov detections in G.992p2 SHOWTIME and Fast Retrain
 *
 * Revision 1.19  1999/08/20 00:48:00  wan
 * Add printouts for Fast Retrain progress status
 *
 * Revision 1.18  1999/08/16 18:07:08  wan
 * Add cases for printing status of Fast Retrain progress
 *
 * Revision 1.17  1999/07/29 21:53:39  george
 * Add Aoc/Eoc status messages
 *
 * Revision 1.16  1999/07/14 23:03:43  george
 * Add more statuses for G994p1
 *
 * Revision 1.15  1999/07/09 01:59:11  wan
 * Added more constants G.994.1 testing reports
 *
 * Revision 1.14  1999/07/07 23:51:06  liang
 * Added rcv power and loop attenuation reports.
 *
 * Revision 1.13  1999/07/02 00:41:21  liang
 * Added status for rcv carrier range in training.
 *
 * Revision 1.12  1999/06/25 21:37:11  wan
 * Work in progress for G994.1.
 *
 * Revision 1.11  1999/06/16 00:54:40  liang
 * Added Tx/Rx SHOWTIME active training progress codes.
 *
 * Revision 1.10  1999/06/11 21:29:02  liang
 * Constants for C/R-Msgs was changed to C/R-Msg.
 *
 * Revision 1.9  1999/06/07 21:05:10  liang
 * Added more training status values.
 *
 * Revision 1.8  1999/05/22 02:18:31  liang
 * More status.
 *
 * Revision 1.7  1999/05/14 22:49:41  liang
 * Added more status reports.
 *
 * Revision 1.6  1999/04/01 20:28:08  liang
 * Added RReverb detect event status.
 *
 * Revision 1.5  1999/03/08 21:58:02  liang
 * More status.
 *
 * Revision 1.4  1999/03/02 01:49:37  liang
 * Print out some connection information.
 *
 * Revision 1.3  1999/02/10 01:56:51  liang
 * Add case for kDslTrainingStatus.
 *
 * Revision 1.2  1999/01/29 02:15:49  liang
 * Add test result status processing back.
 *
 * Revision 1.1  1999/01/27 22:33:25  liang
 * Copied from SoftModem_3_1_02.
 *
 * Revision 1.75  1998/12/17 03:06:47  scott
 * Removed kOverlayRequiredStatus
 *
 *****************************************************************************/
#ifdef XDSLDRV_ENABLE_PARSER
#include "../BcmOs.h"
#if defined(__KERNEL__)
#include <linux/string.h>
#include <linux/kernel.h>
#include "../AdslCore.h"
#include "bcm_map.h"
#endif
#ifndef ULONGLONG
#define ULONGLONG	UINT64
#endif
int printMaxItem = 64;
extern char *parseBufTmp;
#else /* !XDSLDRV_ENABLE_PARSER */
#include <string.h>
#include <math.h>
#endif /* XDSLDRV_ENABLE_PARSER */
#include "SoftDsl.h"
#include "SoftDslG993p2.h"
#include "StatusParser.h"
#include "MathUtil.h"
#include "MiscUtil.h"
#include "Flatten.h"
#include "DslFramer.h"
#include "AdslMibDef.h"
#include "AdslXfaceData.h"

#ifdef LINKLAYER_MNP
#include "Mnp.h"
#include "MnpPriv.h"
#endif
#ifndef XDSLDRV_ENABLE_PARSER
#include "sprintf.h"
#endif


/* parser state support */

#ifdef STATUSPARSER_STATE
statusParserState	parserState[2] = {
	{ -1,-1, -1, { -1, -1 }, -1, -1,-1,{-1,-1} },
	{ -1,-1, -1, { -1, -1 }, -1, -1,-1,{-1,-1} }
};

statusParserState *GetStatusParserStatePtr(int lineId)
{
	return (parserState + lineId);
}
#endif

/* parser output control */

statusParserControl		parserCtrl = { 0, 0, 0, {0} };

statusParserControl* GetStatusParserCtrlPtr(void)
{
	return &parserCtrl;
}

unsigned int GetStatusParserFilter(void)
{
	return parserCtrl.ctrlFilter;
}

void SetStatusParserFilter(unsigned int fltr)
{
	parserCtrl.ctrlFilter = fltr;
}

char GetStatusParserMatlabOutputState(void)
{
	return parserCtrl.matlabOutput;
}

unsigned int GetStatusParserMatlabOutputOption(void)
{
	return parserCtrl.ctrlOutput;
}
int	IsStatusParserSignleMatlabFile(void)
{
	return (parserCtrl.ctrlOutput & kParserOutSingleMatFile);
}

void SetStatusParserMatlabOutputOption(unsigned int state)
{
	parserCtrl.ctrlOutput = state;
}

void SetStatusParserMatlabOutputState(unsigned int state)
{
	parserCtrl.matlabOutput = (char)state;
}

unsigned int XdslFeatureSupported(unsigned int feature)
{
	return AdslFeatureSupported(parserCtrl.phyFeatures,feature);
}

/* message parsing */


#ifdef G994P1
#define PrintVendorIDChar(c)    ((((c >= 'a') && (c <= 'z')) || ((c >='A') && (c <= 'Z'))) ? c : '?')
#endif

Private char*	GetErrorRateString(unsigned int tries, unsigned int errors);
Private char*	Q4ToFloatString(int val);
Private char*	Q8ToFloatString(int val);

#define	PRINT_LONG_BUFFER_DATA

#ifdef LITE_VERSION
#undef	PRINT_LONG_BUFFER_DATA
#endif

#if defined(MIPS_SRC) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
Public size_t
strlen(const char *str ) {
	const char *strSave = str;;
	while( *str != '\0' ) ++str;
	return ((int)str) - ((int)strSave);
}

Public char *strcpy(char *dst, const char *src) 
{
	char ch, *dst1 = dst;
	do {
		*dst1++ = ch = *src++;
	} while (ch != '\0');
	return dst;
}

#endif /*BCM47XX_RECORD_TEST */

typedef int (*printDataHandler) (char *dstPtr, void *dataPtr,...);

/* Print data handlers */

Private int PrintByteHex(char *dstPtr, void *dataPtr)
{
	return SPrintF(dstPtr, " %2.2X", *(uchar *) dataPtr);
}
Private int PrintShortHex(char *dstPtr, void *dataPtr)
{
	return SPrintF(dstPtr, " %4.4X", *(ushort *) dataPtr);
}
Private int PrintLongHex(char *dstPtr, void *dataPtr)
{
	return SPrintF(dstPtr, " %8.8X", *(uint *) dataPtr);
}

Private int PrintLongLongHex(char *dstPtr, void *dataPtr)
{   
	return SPrintF(dstPtr, " %llx", *(ULONGLONG*) dataPtr);
}

Private int PrintByteDecS(char *dstPtr, void *dataPtr)
{
	return SPrintF(dstPtr, " %4d", *(char *) dataPtr);
}
Private int PrintShortDecS(char *dstPtr, void *dataPtr)
{
	return SPrintF(dstPtr, " %6d", *(short *) dataPtr);
}
Private int PrintLongDecS(char *dstPtr, void *dataPtr)
{
	return SPrintF(dstPtr, " %11d", *(int *) dataPtr);
}
Private int PrintByteDecU(char *dstPtr, void *dataPtr)
{
	return SPrintF(dstPtr, " %3u", *(uchar *) dataPtr);
}
Private int PrintShortDecU(char *dstPtr, void *dataPtr)
{
	return SPrintF(dstPtr, " %5u", *(ushort *) dataPtr);
}
Private int PrintLongDecU(char *dstPtr, void *dataPtr)
{
	return SPrintF(dstPtr, " %10u", *(uint *) dataPtr);
}

Private int PrintLongLongDecS(char *dstPtr, void *dataPtr)
{	
	return SPrintF(dstPtr, " %lld", *(LONGLONG*) dataPtr);
}
Private int PrintLongLongDecU(char *dstPtr, void *dataPtr)
{	
	return SPrintF(dstPtr, " %llu", *(ULONGLONG *) dataPtr);
}
Private int PrintLongLongQxS(char *dstPtr, void *dataPtr)
{
	return SPrintF(dstPtr, " %lld", *(LONGLONG *) dataPtr);
}
Private int PrintLongLongQxU(char *dstPtr, void *dataPtr)
{	
	return SPrintF(dstPtr, " %llu", *(ULONGLONG *) dataPtr);
}
static int QxPwr5[] = { 5, 25, 125, 625, 3125, 15625, 78125, 390625, 1953125, 9765625, 48828125, 244140625 };

Private int PrintQxS(char *dstPtr, long val, int q)
{
	long			iPart, fPart;
	char			fmtStr[] = " %3ld.%04lu";

	if (val < 0) {
		val = -val;
		iPart = -(val >> q);
		if (0 == iPart)
			strcpy(fmtStr, "  -%d.%04lu");
	}
	else
		iPart = val >> q;
	fPart = (val & ((1 << q) - 1)) * QxPwr5[q-1];
	fmtStr[8] = '0' + (q > 9 ? 9 : q);
	return _SPrintF(dstPtr, fmtStr, iPart, fPart);
}

Private int PrintQxS4(char *dstPtr, long val, int q)
{
	long			iPart, fPart;
	char			fmtStr[] = " %3ld.%04lu";

	if (val < 0) {
		val = -val;
		iPart = -(val >> q);
		if (0 == iPart)
			strcpy(fmtStr, "  -%d.%04lu");
	}
	else
		iPart = val >> q;
	fPart = val & ((1 << q) - 1);
	fPart = (fPart * 10000) >> q;
	return _SPrintF(dstPtr, fmtStr, iPart, fPart);
}


Private int PrintQxU(char *dstPtr, ulong val, int q)
{
	long			iPart, fPart;
	static	char	fmtStr[] = " %2ld.%04lu";

	iPart = val >> q;
	fPart = (val & ((1 << q) - 1)) * QxPwr5[q-1];
	fmtStr[8] = '0' + (q > 9 ? 9 : q);
	return _SPrintF(dstPtr, fmtStr, iPart, fPart);
}


Private int PrintByteQxS(char *dstPtr, void *dataPtr, int q)
{
	return PrintQxS(dstPtr, *(char *) dataPtr, q);
}
Private int PrintShortQxS(char *dstPtr, void *dataPtr, int q)
{
	return PrintQxS(dstPtr, *(short *) dataPtr, q);
}
Private int PrintShortQxS4(char *dstPtr, void *dataPtr, int q)
{
	return PrintQxS4(dstPtr, *(short *) dataPtr, q);
}
Private int PrintLongQxS(char *dstPtr, void *dataPtr, int q)
{
	return PrintQxS(dstPtr, *(long *) dataPtr, q);
}
Private int PrintLongQxS4(char *dstPtr, void *dataPtr, int q)
{
	return PrintQxS4(dstPtr, *(long *) dataPtr, q);
}
Private int PrintByteQxU(char *dstPtr, void *dataPtr, int q)
{
	return PrintQxU(dstPtr, *(uchar *) dataPtr, q);
}
Private int PrintShortQxU(char *dstPtr, void *dataPtr, int q)
{
	return PrintQxU(dstPtr, *(ushort *) dataPtr, q);
}
Private int PrintLongQxU(char *dstPtr, void *dataPtr, int q)
{
	return PrintQxU(dstPtr, *(ulong *) dataPtr, q);
}


Private int PrintBufferData(
	char			*dstPtr,
	int				dataSize,
	int				nItems,
	unsigned char	*dataPtr,
	int				nItemsInRow,
	printDataHandler pPrintHandler,
	int				p1)
{
// PrintBufferData() is only called in LITE_VERSION version for 'kDsl993p2QlnRaw' case
//#ifdef PRINT_LONG_BUFFER_DATA
	int		count;
	char	*dstPtr0 = dstPtr;
	uchar	*dataEndPtr;

	if (NULL == dataPtr)
		return 0;
#ifdef XDSLDRV_ENABLE_PARSER
	if(nItems > printMaxItem) {
		AdslDrvPrintf(TEXT("PrintBufferData:  trimmed nItems from %d to 64\n"), nItems);
		nItems = printMaxItem;
	}
#endif
	dataEndPtr = dataPtr + nItems*dataSize;
	count = 0;
	while (dataPtr != dataEndPtr) {
		dstPtr += (*pPrintHandler)(dstPtr, dataPtr, p1);

		if (++count == nItemsInRow) {
			count = 0;
			dstPtr += SPrintF(dstPtr, "\r\n");
		}
		dataPtr += dataSize;
	}
	dstPtr += SPrintF(dstPtr, "\n");
	return (dstPtr - dstPtr0);
//#endif
}

Private const uchar BitRevTbl[256] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};
#define BitRev(b)	BitRevTbl[b]

Private int BlockBitReverse(int len, uchar *p)
{
	uchar	*pEnd = p + len;

	while (p != pEnd) {
		*p = BitRev(*p);
		p++;
	}
	return len;
}

/*
**
**  Format string management
**
*/

static char fmtStr[1024];
static int  fmtStrLen = 0;

Public char * GetStatusParserFmtStr(int *pLen)
{
	*pLen = fmtStrLen;
	return fmtStr;
}

Public void	ClrStatusParserFmtStr(void)
{
	fmtStrLen = 0;
}

Private int AppendFormatString(char *dstPtr, int len, char *dataPtr)
{
	memcpy(fmtStr+fmtStrLen, dataPtr, len);
	fmtStrLen += len;
	return 0;
}

Private int PrintBufferString(char *dstPtr, int len, char *dataPtr)
{
#ifdef PRINT_LONG_BUFFER_DATA
	memcpy(dstPtr, dataPtr, len);
	dstPtr[len]   = '\t';
	dstPtr[len+1] = 0;
	return len+1;
#else
	return 0;
#endif
}

Private int PrintBufferDataHex(char *dstPtr, int dataSize, int nPoints, unsigned char *dataPtr)
{
	char *dstPtr0 = dstPtr;
#ifdef PRINT_LONG_BUFFER_DATA
	static printDataHandler	hexPrintHandlers[] = { (void*)PrintByteHex, (void*)PrintShortHex, (void*)PrintLongHex, (void*)PrintLongLongHex };

	if (dataSize > sizeof(hexPrintHandlers)/sizeof(hexPrintHandlers[0]))
		dataSize = 0;
	dstPtr += PrintBufferData(dstPtr, 1 << dataSize, nPoints, dataPtr, 8, hexPrintHandlers[dataSize], 0);
#endif
	return (dstPtr - dstPtr0);
}

Private int PrintBufferDataDec(char *dstPtr, int dataSize, int nPoints, unsigned char *dataPtr, int sign, int q)
{
	char *dstPtr0 = dstPtr;
#ifdef PRINT_LONG_BUFFER_DATA
	printDataHandler		pDataPrintHandler;
	static printDataHandler	dPrintHandlers[] = { (void*)PrintByteDecS, (void*)PrintShortDecS, (void*)PrintLongDecS, (void*)PrintLongLongDecS};
	static printDataHandler	uPrintHandlers[] = { (void*)PrintByteDecU, (void*)PrintShortDecU, (void*)PrintLongDecU, (void*)PrintLongLongDecU };
	static printDataHandler	qsPrintHandlers[] = { (void*)PrintByteQxS, (void*)PrintShortQxS, (void*)PrintLongQxS, (void*)PrintLongLongQxS};
	static printDataHandler	quPrintHandlers[] = { (void*)PrintByteQxU, (void*)PrintShortQxU, (void*)PrintLongQxU, (void*)PrintLongLongQxU};

	if (dataSize > sizeof(dPrintHandlers)/sizeof(dPrintHandlers[0]))
		dataSize = 0;

	if (0 == q)
		pDataPrintHandler = sign ? dPrintHandlers[dataSize] : uPrintHandlers[dataSize];
	else {
		if (q > sizeof(QxPwr5)/sizeof(QxPwr5[0]))
			q = 1;
		pDataPrintHandler = sign ? qsPrintHandlers[dataSize] : quPrintHandlers[dataSize];
	}

	dstPtr += PrintBufferData(dstPtr, 1 << dataSize, nPoints, dataPtr, 8, pDataPrintHandler, q);
#endif
	return (dstPtr - dstPtr0);
}

Public int
PrintShortBufferData(char *dstPtr, char *msg, int nPoints, short *dataPtr)
	{
	char *dstPtr0 = dstPtr;
#ifdef PRINT_LONG_BUFFER_DATA
	if ((nPoints > 0) && (dataPtr != NULL))
		{
		dstPtr += SPrintF(dstPtr, "%s\r\n", msg);
		dstPtr += PrintBufferData(dstPtr, 2, nPoints, (void *) dataPtr, 8, (printDataHandler)PrintShortDecS, 0);
		}
#endif
	return (dstPtr - dstPtr0);
	}

Private int
PrintUShortBufferData(char *dstPtr, char *msg, int nPoints, short *dataPtr)
	{
	char *dstPtr0 = dstPtr;
#ifdef PRINT_LONG_BUFFER_DATA
	if ((nPoints > 0) && (dataPtr != NULL))
		{
		dstPtr += SPrintF(dstPtr, "%s\r\n", msg);
		dstPtr += PrintBufferData(dstPtr, 2, nPoints, (void *) dataPtr, 8, (printDataHandler)PrintShortDecU, 0);
		}
#endif
	return (dstPtr - dstPtr0);
	}

Public int
PrintUCharBufferData(char *dstPtr, char *msg, int nPoints, unsigned char *dataPtr)
	{
	char *dstPtr0 = dstPtr;
#ifdef PRINT_LONG_BUFFER_DATA
	if ((nPoints > 0) && (dataPtr != NULL))
		{
		dstPtr += SPrintF(dstPtr, "%s\r\n", msg);
		dstPtr += PrintBufferData(dstPtr, 1, nPoints, (void *) dataPtr, 8, (printDataHandler)PrintByteDecU, 0);
		}
#endif
	return (dstPtr - dstPtr0);
	}

Private int
PrintQ4dBBufferData(char *dstPtr, char *msg, int nPoints, ushort *dataPtr)
	{
	char *dstPtr0 = dstPtr;
#ifdef PRINT_LONG_BUFFER_DATA
	if ((nPoints > 0) && (dataPtr != NULL))
		{
		dstPtr += SPrintF(dstPtr, "%s\r\n", msg);
		dstPtr += PrintBufferData(dstPtr, 2, nPoints, (void *) dataPtr, 8, (printDataHandler)PrintShortQxS, 4);
		}
#endif
	return (dstPtr - dstPtr0);
	}
Private int
PrintQ8dBBufferData(char *dstPtr, char *msg, int nPoints, ushort *dataPtr)
	{
	char *dstPtr0 = dstPtr;
#ifdef PRINT_LONG_BUFFER_DATA
	if ((nPoints > 0) && (dataPtr != NULL))
		{
		dstPtr += SPrintF(dstPtr, "%s\r\n", msg);
		dstPtr += PrintBufferData(dstPtr, 2, nPoints, (void *) dataPtr, 8, (printDataHandler)PrintShortQxS4, 8);
		}
#endif
	return (dstPtr - dstPtr0);
	}

Public int
PrintByteBufferData(char *dstPtr, char *msg, int nPoints, unsigned char *dataPtr)
	{
	char *dstPtr0 = dstPtr;
#ifdef PRINT_LONG_BUFFER_DATA
	if ((nPoints > 0) && (dataPtr != NULL))
		{
		dstPtr += SPrintF(dstPtr, "%s\r\n", msg);
		dstPtr += PrintBufferDataHex(dstPtr, 0, nPoints, dataPtr);
		}
#endif
	return (dstPtr - dstPtr0);
	}

Private void
CopyBytes(char *s, char *t, int n)
{
	while(((*t++ = *s++) != 0) && (--n))
		;
}

Private char * OemParameterToString(dslStatusStruct *status)
{
	switch (status->param.dslOemParameter.paramId) {
		case kDslOemG994VendorId:	return "DslOemG994VendorId";
		case kDslOemG994XmtNSInfo:	return "DslOemG994XmtNSInfo";
		case kDslOemG994RcvNSInfo:	return "DslOemG994RcvNSInfo";
		case kDslOemEocVendorId:	return "DslOemEocVendorId";
		case kDslOemEocVersion:		return "DslOemEocVersion";
		case kDslOemEocSerNum:		return "DslOemEocSerNum";
		default:					return "Unknown";
	}
	return "Unknown";
}

#ifdef G992P3
Private int PrintG992p3Caps(char *dstPtr, g992p3PhyDataPumpCapabilities *paramsPtr)
{
	int		k;
	char	*dstPtr0 = dstPtr;

	dstPtr += SPrintF(dstPtr, "NTR = %d, Short Init = %d, Diag mode = %d\n",
					paramsPtr->rcvNTREnabled, paramsPtr->shortInitEnabled, paramsPtr->diagnosticsModeEnabled);

	if(paramsPtr->featureSpectrum & kG994p1G992p3AnnexASpectrumBoundsUpstream)
		dstPtr += SPrintF(dstPtr, "NOMPSDus = %d, MAXNOMPSDus = %d, MAXNOMATPus = %d\n",
						paramsPtr->rcvNOMPSDus, paramsPtr->rcvMAXNOMPSDus, paramsPtr->rcvMAXNOMATPus);

#if	1 || defined(G992_APPLY_SSVI)
	if (paramsPtr->numUsSubcarrier > 0)
		{
		dstPtr += SPrintF(dstPtr, "Number of US subcarriers = %d\n", paramsPtr->numUsSubcarrier);

		for (k = 0; k < paramsPtr->numUsSubcarrier; k++)
			{
			dstPtr += SPrintF(dstPtr, "US TSSI[%d] = %d, supp=%d\n",
							G992GetDsSubCarrierIndex(paramsPtr->usSubcarrierIndex[k]),
                            paramsPtr->usLog_tss[k],
                            G992GetDsSubCarrierSuppSetIndication(paramsPtr->usSubcarrierIndex[k]) );
			}
		}
#endif

	if(paramsPtr->featureSpectrum & kG994p1G992p3AnnexASpectrumBoundsDownstream)
		dstPtr += SPrintF(dstPtr, "NOMPSDds = %d, MAXNOMPSDds = %d, MAXNOMATPds = %d\n",
						paramsPtr->rcvNOMPSDds, paramsPtr->rcvMAXNOMPSDds, paramsPtr->rcvMAXNOMATPds);

#if	1 || defined(G992_APPLY_SSVI)
	if (paramsPtr->numDsSubcarrier > 0)
		{
		dstPtr += SPrintF(dstPtr, "Number of DS subcarriers = %d\n", paramsPtr->numDsSubcarrier);

		for (k = 0; k < paramsPtr->numDsSubcarrier; k++)
			{
			dstPtr += SPrintF(dstPtr, "subcarr=%d, tssiQ1=%d, supp=%d\n",
							G992GetDsSubCarrierIndex(paramsPtr->dsSubcarrierIndex[k]),
                            paramsPtr->dsLog_tss[k],
                            G992GetDsSubCarrierSuppSetIndication(paramsPtr->dsSubcarrierIndex[k]));
			}
		}
#endif

 	if(paramsPtr->featureSpectrum & kG994p1G992p3AnnexATxImageAboveNyquistFreq)
		dstPtr += SPrintF(dstPtr, "Size of IDFT = %d, Fill IFFT = %d\n",
						paramsPtr->sizeIDFT, paramsPtr->fillIFFT);

#if 1 || defined(READSL2)
	if (paramsPtr->featureSpectrum & kG994p1G992p3AnnexLReachExtended)
		{
		dstPtr += SPrintF(dstPtr, "READSL2 Up = %d, Down = %d\n",
			paramsPtr->readsl2Upstream, paramsPtr->readsl2Downstream);
		}
#endif
#if 1 || defined(ANNEX_M)
    if (paramsPtr->featureSpectrum & kG994p1G992p3AnnexMSubModePSDMasks)
        {
        dstPtr += SPrintF(dstPtr,"ANNEX M EU-%d : 0x%x\n",32+(paramsPtr->subModePSDMasks<<2),1<<paramsPtr->subModePSDMasks);
        }
#endif

	if(paramsPtr->featureOverhead & kG994p1G992p3AnnexADownOverheadDataRate)
		dstPtr += SPrintF(dstPtr, "Min DS Overhead Data Rate = %d\n", paramsPtr->minDownOverheadDataRate);

	if(paramsPtr->featureOverhead & kG994p1G992p3AnnexAUpOverheadDataRate)
		dstPtr += SPrintF(dstPtr, "Min US Overhead Data Rate = %d\n", paramsPtr->minUpOverheadDataRate);

	if(paramsPtr->featureOverhead & kG994p1G992p3AnnexAMaxNumberDownTPSTC)
		dstPtr += SPrintF(dstPtr, "Max DS: STM_TPSTC = %d, ATM_TPSTC = %d, PTM_TPSTC = %d\n",
						paramsPtr->maxDownSTM_TPSTC, paramsPtr->maxDownATM_TPSTC, paramsPtr->maxDownPTM_TPSTC);

	if(paramsPtr->featureOverhead & kG994p1G992p3AnnexAMaxNumberUpTPSTC)
		dstPtr += SPrintF(dstPtr, "Max US: STM_TPSTC = %d, ATM_TPSTC = %d, PTM_TPSTC = %d\n",
						paramsPtr->maxUpSTM_TPSTC, paramsPtr->maxUpATM_TPSTC, paramsPtr->maxUpPTM_TPSTC);

	for (k = 0; k < 4; k++)
		{
		if(paramsPtr->featureTPS_TC[k] & kG994p1G992p3AnnexADownSTM_TPS_TC)
			dstPtr += SPrintF(dstPtr, "DS STM_TPS_TC: Net_min = %d, Net_max = %d, Net_rev = %d,\n     Delay_max = %d, Error_max = %d, INP_min = %d\n",
							paramsPtr->minDownSTM_TPS_TC[k], paramsPtr->maxDownSTM_TPS_TC[k],
							paramsPtr->minRevDownSTM_TPS_TC[k], paramsPtr->maxDelayDownSTM_TPS_TC[k],
							paramsPtr->maxErrorDownSTM_TPS_TC[k],
							paramsPtr->minINPDownSTM_TPS_TC[k]);

		if(paramsPtr->featureTPS_TC[k] & kG994p1G992p3AnnexAUpSTM_TPS_TC)
			dstPtr += SPrintF(dstPtr, "US S STM_TPS_TC: Net_min = %d, Net_max = %d, Net_rev = %d,\n     Delay_max = %d, Error_max = %d, INP_min = %d\n",
							paramsPtr->minUpSTM_TPS_TC[k], paramsPtr->maxUpSTM_TPS_TC[k],
							paramsPtr->minRevUpSTM_TPS_TC[k], paramsPtr->maxDelayUpSTM_TPS_TC[k],
							paramsPtr->maxErrorUpSTM_TPS_TC[k],
							paramsPtr->minINPUpSTM_TPS_TC[k]);

		if(paramsPtr->featureTPS_TC[k] & kG994p1G992p3AnnexADownATM_TPS_TC)
			dstPtr += SPrintF(dstPtr, "DS ATM_TPS_TC: Net_min = %d, Net_max = %d, Net_rev = %d,\n     Delay_max = %d, Error_max = %d, INP_min = %d\n",
							paramsPtr->minDownATM_TPS_TC[k], paramsPtr->maxDownATM_TPS_TC[k],
							paramsPtr->minRevDownATM_TPS_TC[k], paramsPtr->maxDelayDownATM_TPS_TC[k],
							paramsPtr->maxErrorDownATM_TPS_TC[k],
							paramsPtr->minINPDownATM_TPS_TC[k]);

		if(paramsPtr->featureTPS_TC[k] & kG994p1G992p3AnnexAUpATM_TPS_TC)
			dstPtr += SPrintF(dstPtr, "US ATM_TPS_TC: Net_min = %d, Net_max = %d, Net_rev = %d,\n     Delay_max = %d, Error_max = %d, INP_min = %d\n",
							paramsPtr->minUpATM_TPS_TC[k], paramsPtr->maxUpATM_TPS_TC[k],
							paramsPtr->minRevUpATM_TPS_TC[k], paramsPtr->maxDelayUpATM_TPS_TC[k],
							paramsPtr->maxErrorUpATM_TPS_TC[k],
							paramsPtr->minINPUpATM_TPS_TC[k]);

		if(paramsPtr->featureTPS_TC[k] & kG994p1G992p3AnnexADownPTM_TPS_TC)
			dstPtr += SPrintF(dstPtr, "DS PTM_TPS_TC: Net_min = %d, Net_max = %d, Net_rev = %d,\n     Delay_max = %d, Error_max = %d, INP_min = %d\n",
							paramsPtr->minDownPTM_TPS_TC[k], paramsPtr->maxDownPTM_TPS_TC[k],
							paramsPtr->minRevDownPTM_TPS_TC[k], paramsPtr->maxDelayDownPTM_TPS_TC[k],
							paramsPtr->maxErrorDownPTM_TPS_TC[k],
							paramsPtr->minINPDownPTM_TPS_TC[k]);

		if(paramsPtr->featureTPS_TC[k] & kG994p1G992p3AnnexAUpPTM_TPS_TC)
			dstPtr += SPrintF(dstPtr, "US PTM_TPS_TC: Net_min = %d, Net_max = %d, Net_rev = %d,\n     Delay_max = %d, Error_max = %d, INP_min = %d\n",
							paramsPtr->minUpPTM_TPS_TC[k], paramsPtr->maxUpPTM_TPS_TC[k],
							paramsPtr->minRevUpPTM_TPS_TC[k], paramsPtr->maxDelayUpPTM_TPS_TC[k],
							paramsPtr->maxErrorUpPTM_TPS_TC[k],
							paramsPtr->minINPUpPTM_TPS_TC[k]);

		if (k == 0)
			{
			if(paramsPtr->featurePMS_TC[k] & kG994p1G992p3AnnexADownPMS_TC_Latency)
				dstPtr += SPrintF(dstPtr, "DS Max PMS_TC Latency = %d\n", paramsPtr->maxDownPMS_TC_Latency[k]);

			if(paramsPtr->featurePMS_TC[k] & kG994p1G992p3AnnexAUpPMS_TC_Latency)
				dstPtr += SPrintF(dstPtr, "US Max PMS_TC Latency = %d\n", paramsPtr->maxUpPMS_TC_Latency[k]);
			}
		else
			{
			if(paramsPtr->featurePMS_TC[k] & kG994p1G992p3AnnexADownPMS_TC_Latency)
				dstPtr += SPrintF(dstPtr, "DS Max: PMS_TC Latency = %d, R = %d, D = %d\n",
								paramsPtr->maxDownPMS_TC_Latency[k], paramsPtr->maxDownR_PMS_TC_Latency[k],
								paramsPtr->maxDownD_PMS_TC_Latency[k]);

			if(paramsPtr->featurePMS_TC[k] & kG994p1G992p3AnnexAUpPMS_TC_Latency)
				dstPtr += SPrintF(dstPtr, "US Max: PMS_TC Latency = %d, R = %d, D = %d\n",
								paramsPtr->maxUpPMS_TC_Latency[k], paramsPtr->maxUpR_PMS_TC_Latency[k],
								paramsPtr->maxUpD_PMS_TC_Latency[k]);
			}
		}
	return (dstPtr - dstPtr0);
}
#endif

int PrintExcpReg(char *str, uint *sp)
{
	return SPrintF(str,
		"               \t\tR1 (at)=0x%08X\tR2 (v0)=0x%08X\tR3 (v1)=0x%08X\n"
		"R4 (a0)=0x%08X\tR5 (a1)=0x%08X\tR6 (a2)=0x%08X\tR7 (a3)=0x%08X\n"
		"R8 (t0)=0x%08X\tR9 (t1)=0x%08X\tR10(t2)=0x%08X\tR11(t3)=0x%08X\n"
		"R12(t4)=0x%08X\tR13(t5)=0x%08X\tR14(t6)=0x%08X\tR15(t7)=0x%08X\n"
		"R16(s0)=0x%08X\tR17(s1)=0x%08X\tR18(s2)=0x%08X\tR19(s3)=0x%08X\n"
		"R20(s4)=0x%08X\tR21(s5)=0x%08X\tR22(s6)=0x%08X\tR23(s7)=0x%08X\n"
		"R24(t8)=0x%08X\tR25(t9)=0x%08X\tR26(k0)=0x%08X\tR27(k1)=0x%08X\n"
		"R28(gp)=0x%08X\tR29(sp)=0x%08X\tR30(s8)=0x%08X\tR31(ra)=0x%08X\n",
		sp[0],	sp[1],	sp[2],
		sp[3],	sp[4],	sp[5],	sp[6],
		sp[7],	sp[8],	sp[9],	sp[10],
		sp[11], sp[12], sp[13], sp[14],
		sp[15], sp[16], sp[17], sp[18],
		sp[19], sp[20], sp[21], sp[22],
		sp[23], sp[24], sp[25], sp[26],
		sp[27], sp[28], sp[29], sp[30]);
}

int PrintExcpArgs(char *str, int argc, uint *sp)
{
	int		i;
	char	*p = str;

	p += SPrintF(p, "argv[0] (EPC) = 0x%08X\n", sp[0]);
	for (i = 1; i < argc; i++)
		p += SPrintF(p, "argv[%d]=0x%08X\n", i, sp[i]);
	return p - str;
}

int PrintExcpStack(char *str, uint spAddr, int stackSize, uint *sp)
{
	int		i;
	char	*p = str;

	p += SPrintF(p, "Exception stack dump:\n");
	for (i = 0; i < stackSize; i += 8) {
		p += SPrintF(p, "%08X: %08X %08X %08X %08X %08X %08X %08X %08X\n",
			spAddr + (i*4), sp[0], sp[1], sp[2], sp[3], sp[4], sp[5], sp[6], sp[7]);
		sp += 8;
	}
	return p - str;
}

#if defined(WINNT) || defined(LINUX_DRIVER)
extern char *DslDiagsGetPhyE14StatStr(unsigned long code,unsigned long *arg, char *dstStr);
extern void DslDiagSetDualLatencyStatus(unsigned char flag, unsigned char dir);
extern char *DslDiagsProcessPhyPrintStat(int nArg, unsigned long * arg, char * dstStr);
extern char *DslDiagsProcessPhyDBPrintStat(int statCode, int nArg, unsigned long * arg, char * dstStr);
extern Boolean DslDiagIsBondingSlave(void);
#endif

static char *eocCmd[] = {
	"EocUnknownCmd",
	/* Eoc Messages from ATU-C to ATU-R */
	"EocHoldStateCmd",
	"EocReturnToNormalCmd",
	"EocPerformSelfTestCmd",
	"EocRequestCorruptCRCCmd",
	"EocRequestEndCorruptCRCCmd",
	"EocNotifyCorruptCRCCmd",
	"EocNotifyEndCorruptCRCCmd",
	"EocRequestTestParametersUpdateCmd",
	"EocGrantPowerDownCmd",
	"EocRejectPowerDownCmd",
	/* Eoc Messages  from ATU-R to ATU-C */
	"EocRequestPowerDownCmd",
	"EocDyingGaspCmd",
};

void RemoveStringFormat(uchar *fmtStr)
{
	#define WS_FMT ((((int) 's') << 8) + '%')
	ushort  w;

	while (*fmtStr != 0) {
	  w = *(ushort *) fmtStr;
	  if (WS_FMT == w) {
	    fmtStr[1] = 'X';
		fmtStr++;
	  }
	  fmtStr++;
	}
}

#define kDslDbgDataStringMask (kDslDbgDataFormatMask | kDslDbgDataSizeMask | kDslDbgDataSignMask | kDslDbgDataQxMask)

int PrintBufferDataGen(modemStatusStruct *status, char *dstPtr)
{
	int	dataSize;
	int	msgLen = status->param.dslClearEocMsg.msgType & 0xFFFF;

	dataSize = (status->param.dslClearEocMsg.msgType & kDslDbgDataSizeMask) >> 16;
	if (status->param.dslClearEocMsg.msgType & kDslDbgDataBitReverse)
		return BlockBitReverse(msgLen, (void *) status->param.dslClearEocMsg.dataPtr);
	if (kDslDbgDataString == (status->param.dslClearEocMsg.msgType & kDslDbgDataStringMask))
		return PrintBufferString(dstPtr, msgLen, (void *) status->param.dslClearEocMsg.dataPtr);
	else if (kDslDbgDataStringF == (status->param.dslClearEocMsg.msgType & kDslDbgDataStringMask))
		return AppendFormatString(dstPtr, msgLen, (void *) status->param.dslClearEocMsg.dataPtr);
	else if (kDslDbgDataFormatHex == (status->param.dslClearEocMsg.msgType & kDslDbgDataFormatMask))
		return PrintBufferDataHex(dstPtr, dataSize, msgLen >> dataSize, (void *) status->param.dslClearEocMsg.dataPtr);
	else
		return PrintBufferDataDec(dstPtr, dataSize, msgLen >> dataSize,
				(void *) status->param.dslClearEocMsg.dataPtr,
				status->param.dslClearEocMsg.msgType & kDslDbgDataSignMask,
				(status->param.dslClearEocMsg.msgType & kDslDbgDataQxMask) >> kDslDbgDataQxShift);
}

#ifdef XDSLDRV_ENABLE_PARSER
static char	*pDataPtrOrig = NULL;
#endif
static int StatusParserProcReceiveEocMsg(modemStatusStruct *status, char *dstPtr, uint lineId)
{
	int	i, dataSize;
	int	msgNum = (status->param.dslClearEocMsg.msgType >> 16) & 0xFF;
	int	msgLen = status->param.dslClearEocMsg.msgType & 0xFFFF;
	uint	*pData32 =(uint *) status->param.dslClearEocMsg.dataPtr;
	ushort	*pData16 =(ushort *) pData32;
	uchar	*pData8 = (uchar *) pData32;
	static	uint excSp = 0;
	char	*dstPtr0 = dstPtr;

	if (status->param.value < kDslClearEocFirstCmd) {
#ifndef LITE_VERSION
		if (status->param.value < sizeof(eocCmd)/sizeof(char *))
			dstPtr += SPrintF(dstPtr, "%s",eocCmd[status->param.value]);
#endif
		return dstPtr - dstPtr0;
	}

	switch (status->param.value) {
#ifndef LITE_VERSION
		case kDslShowtimeDoiSnrMarginHdrQ8:
			dstPtr += SPrintF(dstPtr, "DOI ");
			/* intentional fall through */
		case kDslShowtimeSnrMarginHdrQ8:
		{
			dslShowtimeSNRMarginInfoType *pSnrMarginInfoType = (dslShowtimeSNRMarginInfoType *)pData32;
			dstPtr += SPrintF(dstPtr, "Margin Info: MAX(%d, %s) ",
				pSnrMarginInfoType->maxMarginCarrier,
				Q8ToFloatString(pSnrMarginInfoType->maxSNRMargin));
			dstPtr += SPrintF(dstPtr, "MIN(%d, %s) ",
				pSnrMarginInfoType->minMarginCarrier,
				Q8ToFloatString(pSnrMarginInfoType->minSNRMargin));
			dstPtr += SPrintF(dstPtr, "AVG=%s nTones=%d ",
				Q8ToFloatString(pSnrMarginInfoType->avgSNRMargin),
				pSnrMarginInfoType->nCarriers);
			break;
		}
		case kDslShowtimeDoiSnrMarginDataQ8:
			dstPtr += SPrintF(dstPtr, "DOI ");
			/* intentional fall through */
		case kDslShowtimeSnrMarginDataQ8:
			dataSize = msgLen >> 1;
			if (dataSize != 0)
				dstPtr += PrintQ8dBBufferData(dstPtr, "SNR Margins:", (int)dataSize, pData16);
			break;
#endif
		case kDslClearEocSendComplete:
		case kDslClearEocSendComplete2:
			if (status->param.dslClearEocMsg.dataPtr)
				dstPtr += SPrintF(dstPtr, "Clear EOC Send Complete%s(%s). msgNum=%d msgLen=%d dataPtr=0x%px\n",
					kDslClearEocSendComplete2 == status->param.value ? "2" :"",
					status->param.dslClearEocMsg.msgType & kDslClearEocMsgExtraSendComplete ? "EXTRA" : "",
					msgNum, msgLen, status->param.dslClearEocMsg.dataPtr);
			else
				dstPtr += SPrintF(dstPtr, "Clear EOC Send Rejected. rejNum=%d phyNum=%d\n",
					msgNum, msgLen);
			break;
		case kDslClearEocRcvedFrame:
		case kDslClearEocSendFrame:
			dstPtr += SPrintF(dstPtr, "Clear EOC %s. msgNum=%d msgLen=%d msgData=",
				kDslClearEocRcvedFrame == status->param.value ? "Rcved frame" : "Xmt Frame",
				msgNum, msgLen);
#ifdef XDSLDRV_ENABLE_PARSER
			if(msgLen > printMaxItem)
				msgLen = printMaxItem;
			status->param.dslClearEocMsg.dataPtr = pDataPtrOrig;
#endif
			for (i = 0; i < msgLen; i++)
				dstPtr += SPrintF(dstPtr, "0x%X ", status->param.dslClearEocMsg.dataPtr[i] & 0xFF);
			dstPtr += SPrintF(dstPtr, "\n");
			break;
		case kDslStrPrintf:
			{
			uint argNum = pData32[0];
#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
#if defined(CONFIG_ARM64)
			/* Can't use vsprintf since PHY arguments are 32bit not 64bit expected by vsprintf */
			if (argNum > 18)
			  AdslDrvPrintf(TEXT("kDslStrPrintf: nArgs=%d > 18 won't be printed\n"), argNum);
			dstPtr += SPrintF (dstPtr, (void *) (pData32 + 1 + argNum), pData32[1], pData32[2], pData32[3], pData32[4], 
				pData32[5], pData32[6], pData32[7], pData32[8], pData32[9], pData32[10], pData32[11], pData32[12],
				pData32[13], pData32[14], pData32[15], pData32[16], pData32[17], pData32[18]);
#else
			va_list arg;
			arg.__ap = (void *)(pData32+1);
			dstPtr += VSPrintF(dstPtr, (void *) (pData32 + 1 + argNum), arg);
#endif
#else	/* !ARM */
			RemoveStringFormat((void *) (pData32 + 1 + argNum));
			dstPtr += VSPrintF(dstPtr, (void *) (pData32 + 1 + argNum), (void *) (pData32+1));
#endif
			}
			break;
#ifndef LITE_VERSION
		case kDslGeneralMsgDbgDataPrint:
			dstPtr += PrintBufferDataGen(status, dstPtr);
			break;
		case kDslGeneralMsgDbgPrintf:
			switch (msgNum) {
				default:
					dstPtr += SPrintF(dstPtr, "DbgMsg: id=%d, ", msgNum);
					for (i = 0; i < (msgLen >> 2); i++)
						dstPtr += SPrintF(dstPtr, "a%d=0x%X ", i, pData32[i]);
					break;
			}
			break;
		case kDslGeneralMsgDbgPrintG992p3Cap:
#ifdef G992P3
			dstPtr += PrintG992p3Caps(dstPtr, (void *) status->param.dslClearEocMsg.dataPtr);
#endif
			break;

		case kDslGeneralMsgDbgProfData:
#if 0
			dstPtr += SPrintF(dstPtr, "ProfData: Size=%d ",	msgLen);
#endif
			break;
		case kDslVectoringStartDumpCmd:
			dstPtr += SPrintF(dstPtr, "kDslVectoringStartDumpCmd, len=%d",msgLen);
			break;
#ifdef SUPPORT_VECTORING
		case kDslVectoringErrorSamples:
		{
			VectorErrorSample *pES = (VectorErrorSample *)pData8;
			if(pES->nERBbytes > 0) {
				dstPtr += SPrintF(dstPtr, "VectorErrorSample Info: nERBbytes=%d, lineId=%d, syncCounter=%d\n",
					pES->nERBbytes, pES->lineId, pES->syncCounter);  
                if ((GetStatusParserFilter()&kParserFilterVectErrSmp)==0)
                    dstPtr += PrintUCharBufferData(dstPtr, "kDslVectoringErrorSamples", pES->nERBbytes, pES->errorMsg);
			}
			else
				dstPtr += SPrintF(dstPtr, "VectorErrorSample Errored: nERBbytes=%d, lineId=%d, syncCounter=%d",
					pES->nERBbytes, pES->lineId, pES->syncCounter);
		}
			break;
#endif
		case kVectoringMacAddress:
#ifdef XDSLDRV_ENABLE_PARSER
			dstPtr += PrintUCharBufferData(dstPtr, "kVectoringMacAddress", msgLen, pDataPtrOrig);
#else
			dstPtr += PrintUCharBufferData(dstPtr, "kVectoringMacAddress", msgLen, pData8);
#endif
			break;
		case kDslVectoringSetPilotCmd:
			dstPtr += SPrintF(dstPtr, "XXX:SP: kDslVectoringSetPilotCmd: %d", msgLen);
			break;
		case kDslGeneralMsgDbgFileName:
#ifdef XDSLDRV_ENABLE_PARSER
			pData8 = (uchar*)pDataPtrOrig;
#endif
			if (status->param.dslClearEocMsg.msgType & kDslDbgFileNameDelete) {
				pData8[msgLen] = 0;
				dstPtr += SPrintF(dstPtr, "OpenFile name: %s", pData8);
			}
			break;
		case kDslGeneralMsgDbgWriteFile:
			/* dstPtr += SPrintF(dstPtr, "Write File: Size=%d ", msgLen); */
			break;
#if defined(WINNT) || defined(LINUX_DRIVER)	  /* This is used only in the Dsl Diags compilation */
		case kDslGeneralMsgE14Print:
		case kDslGeneralMsgE14Print1:
			dstPtr = DslDiagsGetPhyE14StatStr((unsigned long)status->param.value, (unsigned long *)(status->param.dslClearEocMsg.dataPtr), dstPtr);
			break;
		case kDslGeneralMsgPrintf:
			dstPtr = DslDiagsProcessPhyPrintStat(msgLen >> 2,
						(unsigned long *) status->param.dslClearEocMsg.dataPtr, dstPtr);
			break;
		case kDslGeneralMsgDBPrintf:
		case kDslGeneralMsgE14Printf:
			dstPtr = DslDiagsProcessPhyDBPrintStat(status->param.value, msgLen >> 2,
						(unsigned long *) status->param.dslClearEocMsg.dataPtr, dstPtr);
			break;
#endif
		case	kDsl993p2LnAttnAvg:
			dstPtr += SPrintF(dstPtr, "Line Attn Average (s7.8 dB) : %d", pData16[0]);
			break;
#if defined(DSL_EXTERNAL_BONDING_DISCOVERY)
		case kDslBondDiscExchange:
			dstPtr += SPrintF(dstPtr, "EXT-BOND-DISC-PHY: ");
			goto bondDiscExch;
		case kDslBondDiscExchangeDrv:
			dstPtr += SPrintF(dstPtr, "EXT-BOND-DISC-DRV: ");
bondDiscExch:
			{
			bonDiscExchangeStruct *pBDExch = (bonDiscExchangeStruct *) pData16;

			dstPtr += SPrintF(dstPtr,"cmd=0x%X id=%d discReg=0x%08X(%08X) aggr=0x%X time=0x%X", pBDExch->bdCmd & 0xFFFF, pBDExch->bdId,
				  pBDExch->bondDisc.pmeRemoteDiscoveryHigh, pBDExch->bondDisc.pmeRemoteDiscoveryLow,
				  pBDExch->bondDisc.pmeAggregationReg, pBDExch->bondDisc.timeOfLastCL);
			}
			break;
#endif /* DSL_EXTERNAL_BONDING_DISCOVERY */
#endif /* LITE_VERSION */
        case kDsl9701SocMsgDump:
        {
#ifdef XDSLDRV_ENABLE_PARSER
			unsigned char *msg9701Soc = (uchar*)pDataPtrOrig;
#else
			unsigned char *msg9701Soc = pData8;
#endif
			char soc[20];
			/* decode messge type - should remove when full message parser is implemented */
			switch (msg9701Soc[0])
			{
				case 0x00:
					strcpy(soc,"o-signature");
					break;
				case 0x01:
					strcpy(soc,"o-tg-update");
					break;
				case 0x02:
					strcpy(soc,"o-update");
					break;
				case 0x03:
					strcpy(soc,"o-error-feedback");
					break;
				case 0x04:
					strcpy(soc,"o-snr");
					break;
				case 0x05:
					strcpy(soc,"o-prm");
					break;
				case 0x07:
					strcpy(soc,"o-msg1");
					break;
				case 0x08:
					strcpy(soc,"o-tps");
					break;
				case 0x09:
					strcpy(soc,"o-pms");
					break;
				case 0x0A:
					strcpy(soc,"o-pmd");
					break;
				case 0x0B:
					strcpy(soc,"o-ack");
					break;
				case 0x0F:
					strcpy(soc,"o/r-ack-seg");
					break;
				case 0x80:
					strcpy(soc,"r-msg1");
					break;
				case 0x81:
					strcpy(soc,"r-update");
					break;
				case 0x82:
					strcpy(soc,"r-ack");
					break;
				case 0x83:
					strcpy(soc,"r-feedback");
					break;
				case 0x84:
					strcpy(soc,"r-snr");
					break;
				case 0x85:
					strcpy(soc,"r-prm");
					break;
				case 0x86:
					strcpy(soc,"r-msg2");
					break;
				case 0x87:
					strcpy(soc,"r-ack1");
					break;
				case 0x88:
					strcpy(soc,"r-pms");
					break;
				case 0x89:
					strcpy(soc,"r-pmd");
					break;
				case 0x55:
					strcpy(soc,"o/r-repeat_request");
					break;

				default:
					strcpy(soc,"un-known SOC");
					break;
			}

			if ((msgLen > 0) && (msg9701Soc != NULL))
			{
				dstPtr += SPrintF(dstPtr, "G.Fast Soc Msg: %s\r\n", soc);
#ifndef LITE_VERSION
                if ((GetStatusParserFilter()&kParserFilterSocMsg) == 0)
                  dstPtr += PrintBufferDataHex(dstPtr, 0, msgLen, msg9701Soc);
#endif
			}
		}
        break;
		case kDsl993p2SocMsgDump:
		{
#ifdef XDSLDRV_ENABLE_PARSER
			unsigned char *msg993p2Soc = (uchar*)pDataPtrOrig;
#else
			unsigned char *msg993p2Soc = pData8;
#endif
			char soc[20];
			/* decode messge type - should remove when full message parser is implemented */
			switch (msg993p2Soc[0])
			{
				case 0x00:
					strcpy(soc,"o-ack");
					break;
				case 0x01:
					strcpy(soc,"o-signature");
					break;
				case 0x02:
					strcpy(soc,"o-update");
					break;
				case 0x03:
					strcpy(soc,"o-msg1");
					break;
				case 0x04:
					strcpy(soc,"o-prm");
					break;
				case 0x05:
					strcpy(soc,"o-ta_update");
					break;
				case 0x06:
					strcpy(soc,"o-tps");
					break;
				case 0x07:
					strcpy(soc,"o-pms");
					break;
				case 0x08:
					strcpy(soc,"o-pmd");
					break;
				case 0x09:
					strcpy(soc,"o-prm_ld");
					break;
				case 0x0A:
					strcpy(soc,"o-msg_ld");
					break;
				case 0x80:
					strcpy(soc,"r-ack");
					break;
				case 0x81:
					strcpy(soc,"r-msg1");
					break;
				case 0x82:
					strcpy(soc,"r-update");
					break;
				case 0x83:
					strcpy(soc,"r-msg2");
					break;
				case 0x84:
					strcpy(soc,"r-prm");
					break;
				case 0x85:
					strcpy(soc,"r-ta_update");
					break;
				case 0x86:
					strcpy(soc,"r-tps_ack");
					break;
				case 0x87:
					strcpy(soc,"r-pms");
					break;
				case 0x88:
					strcpy(soc,"r-pmd");
					break;
				case 0x89:
					strcpy(soc,"r-prm_ld");
					break;
				case 0x8A:
					strcpy(soc,"r-msg_ld");
					break;
				case 0x8B:
					strcpy(soc,"r-feedback");
					break;
				case 0x0F:
					strcpy(soc,"o/r-reapeat_req");
					break;
				case 0x55:
					strcpy(soc,"o/r-ack_seg");
					break;

				default:
					strcpy(soc,"un-known SOC");
					break;
			}

			if ((msgLen > 0) && (msg993p2Soc != NULL))
			{
				dstPtr += SPrintF(dstPtr, "Soc Msg: %s\r\n", soc);
#ifndef LITE_VERSION
                if (((GetStatusParserFilter()&kParserFilterSocMsg) == 0) &&
                    (((GetStatusParserFilter()&kParserFilterRfbck)==0) || (msg993p2Soc[0]!=0x8B)))
                  dstPtr += PrintBufferDataHex(dstPtr, 0, msgLen, msg993p2Soc);
#endif
			}
		}
			break;
		case kDslPhyEocRxMsg:
		case kDslPhyEocTxMsg:
#ifndef LITE_VERSION
		{
			#define MAX_EOC_MSG_LEN   0x2000
			static uchar eocMsgBuf[2][MAX_EOC_MSG_LEN];
			static int eocMsgLen[2] = {0, 0};
			int dir, prio, msgNum, rspOrCmd;
#ifdef XDSLDRV_ENABLE_PARSER
			pData8 = (uchar*)pDataPtrOrig;
#endif
			dir = (kDslPhyEocTxMsg == status->param.value) ? 0 : 1;
			if ((eocMsgLen[dir] != 0) || (status->param.dslClearEocMsg.msgType & kDslDbgEocTxIncomplete)) {
				int n = msgLen;
				if (n > (MAX_EOC_MSG_LEN - eocMsgLen[dir]))
					n = MAX_EOC_MSG_LEN - eocMsgLen[dir];
				memcpy(&eocMsgBuf[dir][eocMsgLen[dir]], pData8, n);
				eocMsgLen[dir] += n;
				if (status->param.dslClearEocMsg.msgType & kDslDbgEocTxIncomplete)
					break;
				pData8 = eocMsgBuf[dir];
				msgLen = eocMsgLen[dir];
				eocMsgLen[dir] = 0;
			}
#ifdef STATUSPARSER_STATE
			if((kXdslModGfast == parserState[lineId].modType) || (pData8[1] & 0xFC) || (pData8[0] > 2)) {
				prio = pData8[1] & 3;
				rspOrCmd = (pData8[1] & 0xC);
				msgNum = (((pData8[0] & 0x3F) << 4) | ((pData8[1] >> 4) & 0xF)) + 1;	// Length
			}
			else
#endif
			{
				prio = pData8[0] & 3;
				rspOrCmd = (pData8[1] & 2);
				msgNum= pData8[1] & 1;
			}
			dstPtr += SPrintF(dstPtr, "PHY G.997 frame %s:  len = %d  ( PRI%d %s %d) data:\r\n",
						(kDslPhyEocTxMsg == status->param.value) ? "TX" : "RX", msgLen, prio, (rspOrCmd) ? "RSP" : "CMD", msgNum);
			dstPtr += PrintBufferDataHex(dstPtr, 0, msgLen, pData8);
		}
#endif
			break;
		case kDsl993p2BandPlanDsDump:
		case kDsl993p2BandPlanUsDump:
		{
			Bp993p2 *bp = (Bp993p2*)(status->param.dslClearEocMsg.dataPtr);
			/* 2nd byte is reserved so bypass the 1st 2bytes to get to the tone data */
			pData16++;

			if(status->param.value==kDsl993p2BandPlanDsDump)
				dstPtr += SPrintF(dstPtr, "DS Band Plan(%s): %d bands \n",
					(bp->reserved==PhyBandPlan) ? "PHYS": (bp->reserved==NegBandPlan) ? "NEG": "Unknown", bp->noOfToneGroups);
			else
				dstPtr += SPrintF(dstPtr, "US Band Plan(%s): %d bands \n",
					(bp->reserved==PhyBandPlan) ? "PHYS": (bp->reserved==NegBandPlan) ? "NEG": "Unknown", bp->noOfToneGroups);

			for(i=0;i<(bp->noOfToneGroups*2);i+=2)
				dstPtr += SPrintF(dstPtr, "Start Tone %d Stop Tone %d\n", pData16[i+1],pData16[i]);
		}
			break;
		case kDsl993p2PsdDump:
		{
			Psd993p2 *psd = (Psd993p2*)(status->param.dslClearEocMsg.dataPtr);
			/* 2nd byte is reserved so bypass the 1st 2bytes to get to the tone data */
			pData16++;

			dstPtr += SPrintF(dstPtr, "Tx Psd : %d Break Points \n",psd->n);

			for(i=0;i<(psd->n*2);i+=2)
				dstPtr += SPrintF(dstPtr, "Tone Index %d Psd %d (dBm/Hz x100)\n", pData16[i],(pData16[i+1]*100)>>8);
		}
			break;


		case kDsl993p2MrefPSDds:
		{
			int nBp=pData16[0]>>8;
			pData16++;

			dstPtr += SPrintF(dstPtr, "MrefPSDds: %d Break Points \n",nBp);

			for(i=0;i<(nBp*2);i+=2)
				dstPtr += SPrintF(dstPtr, "Tone Index %d Psd %d (dBm/Hz x100)\n", pData16[i],(pData16[i+1]*100)>>8);
		}
			break;
		case kDsl993p2MrefPSDus:
		{
		  int nBp=pData16[0]>>8;
			pData16++;

			dstPtr += SPrintF(dstPtr, "MrefPSDus: %d Break Points \n",nBp);

			for(i=0;i<(nBp*2);i+=2)
				dstPtr += SPrintF(dstPtr, "Tone Index %d Psd %d (dBm/Hz x100)\n", pData16[i],(pData16[i+1]*100)>>8);
		}
			break;
		case kDsl993p2LimitMask:
		{
			int nBp=pData16[0]>>8;
			pData16++;

			dstPtr += SPrintF(dstPtr, "LimitMask: %d Break Points \n",nBp);

			for(i=0;i<(nBp*2);i+=2)
				dstPtr += SPrintF(dstPtr, "Tone Index %d Psd %d (dBm/Hz x100)\n", pData16[i],(pData16[i+1]*100)>>8);
		}
			break;
	  case kDsl993p2VnPSDds:
		{
			int nBp=pData16[0]>>8;
			pData16++;

			dstPtr += SPrintF(dstPtr, "VnPSDds: %d Break Points \n",nBp);

			for(i=0;i<(nBp*2);i+=2)
				dstPtr += SPrintF(dstPtr, "Tone Index %d Psd %d (dBm/Hz x100)\n", pData16[i],(pData16[i+1]*100)>>8);
		}
			break;
		case kDsl993p2VnPSDus:
		{
			int nBp=pData16[0]>>8;
			pData16++;

			dstPtr += SPrintF(dstPtr, "VnPSDus: %d Break Points \n",nBp);

			for(i=0;i<(nBp*2);i+=2)
				dstPtr += SPrintF(dstPtr, "Tone Index %d Psd %d (dBm/Hz x100)\n", pData16[i],(pData16[i+1]*100)>>8);
		}
			break;

	  case kDsl993p2DSkl0perBand:
		{
			dstPtr += SPrintF(dstPtr, "Per band DS kl0 \n");

			for(i=0;i<msgLen>>1;i++)
			{
			  int tmp=pData16[i];
			  if(tmp>=0x7f00)
			    tmp=78618;    /* 3071 special value */

				dstPtr += SPrintF(dstPtr, "DS%d kl0 %d(dBmx100)\n",i+1,(tmp*100)>>8);
			}
		}
			break;
	  case kDsl993p2USkl0perBand:
		{
			dstPtr += SPrintF(dstPtr, "Per band US kl0 \n");

			for(i=0;i<msgLen>>1;i++)
			{
			  int tmp=pData16[i];
			  if(tmp>=0x7f00)
			    tmp=78618;    /* 3071 special value */
				dstPtr += SPrintF(dstPtr, "US%d kl0 %d(dBmx100)\n",i+1,(tmp*100)>>8);
			}
		}
			break;
		case kDsl993p2QlnRaw:
		case kDsl993p2QlnRawRnc:
			msgLen = msgLen>>1;
			dstPtr += SPrintF(dstPtr, "%s Raw (s7.8 dBm/Tone) (physical BP) \n", (kDsl993p2QlnRaw==status->param.value)? "Qln": "RNC Qln");
			dstPtr += PrintBufferData(dstPtr, 2, msgLen, (void *) pData16, 8 /* nItem per row */, (printDataHandler)PrintShortDecS, 8 /* binaryPoint */);
			if (GetStatusParserFilter() & kParserNotFilterDecQ8) {
			  dstPtr += SPrintF(dstPtr, "\nData in decimal format:\n");
			  dstPtr += PrintBufferData(dstPtr, 2, msgLen, (void *) pData16, 8, (printDataHandler)PrintShortQxS4, 8);
			}
			break;
		case kDslChannelQlnRnc:
#ifdef XDSLDRV_ENABLE_PARSER
			pData8 = (uchar*)pDataPtrOrig;
#endif
			dstPtr += PrintByteBufferData(dstPtr,"Dsl Channel RNC Quiet Line Noise :", msgLen, pData8);
			break;
#ifndef LITE_VERSION
//		case kDsl993p2QlnRaw:
		case kDsl993p2TestHlin:
		case kDsl993p2HlogRaw:
		case kDsl993p2SnrRaw:
		case kDsl993p2LnAttnRaw:
		case kDsl993p2SATNpbRaw:
		case kDsl993p2NeGi:
		case kDsl993p2NeGiPhy:
		case kDsl993p2FeGi:
		case kDsl993p2NeTi:
		case kDsl993p2FeTi:
		case kDsl993p2UsGi:
		case kDsl993p2DOINeGi:
		case kDsl993p2DOIFeGi:
		case kDsl993p2DOINeTi:
		case kDsl993p2DOIFeTi:
		case kDsl993p2AlnRaw:
		case kDslGfastDoiSnrRaw:
		{
			int binaryPoint=0;
			int dSizeShift = 1;  /* dataSize = 1 << dSizeShift */
			printDataHandler  prnFuncRaw = (printDataHandler) PrintShortDecS;
			printDataHandler  prnFuncFmt = (printDataHandler) PrintShortQxS4;
			switch (status->param.value)
			{
#if 0
			case kDsl993p2QlnRaw:
			  dstPtr += SPrintF(dstPtr, "Qln Raw (s7.8 dBm/Tone) (physical BP) \n");
			  binaryPoint=8;
			  break;
#endif
			case kDsl993p2AlnRaw:
				dstPtr += SPrintF(dstPtr, "Aln Raw (s7.8 dBm/Tone) (physical BP) \n");
				binaryPoint=8;
				break;
			case kDsl993p2TestHlin:
			  dstPtr += SPrintF(dstPtr, "Hlin (s7.8)\n");
			  binaryPoint=8;
			  dSizeShift = 2;  /* 32-bit */
			  prnFuncRaw = (printDataHandler) PrintLongDecS;
			  prnFuncFmt = (printDataHandler) PrintLongQxS4;
			  break;
			case kDsl993p2HlogRaw:
			  dstPtr += SPrintF(dstPtr, "Hlog Raw (s7.8 dB) (physical BP) \n");
			  binaryPoint=8;
			  break;
			case kDslGfastDoiSnrRaw:
			  dstPtr += SPrintF(dstPtr, "DOI ");
			  /* intentional fall through */
			case kDsl993p2SnrRaw:
			  dstPtr += SPrintF(dstPtr, "Snr Raw (s7.8 dB) (physical BP) \n");
			  binaryPoint=8;
			  break;
			case kDsl993p2LnAttnRaw:
			  dstPtr += SPrintF(dstPtr, "Line Attn Raw (s7.8 dB)\n");
			  binaryPoint=8;
			  break;
			case kDsl993p2SATNpbRaw:
			  dstPtr += SPrintF(dstPtr, "Signal Attn Raw (s7.8 dB)\n");
			  binaryPoint=8;
			  break;
			case kDsl993p2NeGiPhy:
			  dstPtr += SPrintF(dstPtr, "Near End Gi (Q8 dB) (PHY BP)\n");
			  binaryPoint=0;
			  break;
			case kDsl993p2NeGi:
			  dstPtr += SPrintF(dstPtr, "Near End Gi (Q8 dB linear) (negotiated BP)\n");
			  binaryPoint=9;
			  break;
			case kDsl993p2DOINeGi:
			  dstPtr += SPrintF(dstPtr, "DOI Near End Gi (Q8 dB linear) (negotiated BP)\n");
			  binaryPoint=9;
			  break;
			case kDsl993p2UsGi:
			  dstPtr += SPrintF(dstPtr, "Xmt Gi Changes (s6.9 linear) (bit swap)\n");
			  binaryPoint=9;
			  break;
			case kDsl993p2FeGi:
			  dstPtr += SPrintF(dstPtr, "Far End Gi (s6.9 linear)(negotiated BP)\n");
			  binaryPoint=9;
			  break;
			case kDsl993p2DOIFeGi:
			  dstPtr += SPrintF(dstPtr, "DOI Far End Gi (s6.9 linear)(negotiated BP)\n");
			  binaryPoint=9;
			  break;
			case kDsl993p2NeTi:
			  dstPtr += SPrintF(dstPtr, "Near End Ti (negotiated BP)\n");
			  binaryPoint=0;
			  break;
			case kDsl993p2FeTi:
			  dstPtr += SPrintF(dstPtr, "Far End Ti (negotiated BP)\n");
			  binaryPoint=0;
			  break;
			case kDsl993p2DOINeTi:
			  dstPtr += SPrintF(dstPtr, "DOI Near End Ti (negotiated BP)\n");
			  binaryPoint=0;
			  break;
			case kDsl993p2DOIFeTi:
			  dstPtr += SPrintF(dstPtr, "DOI Far End Ti (negotiated BP)\n");
			  binaryPoint=0;
			  break;
			}
			msgLen = msgLen >> dSizeShift;
			dstPtr += PrintBufferData(dstPtr, 1 << dSizeShift, msgLen, (void *) pData16, 8, prnFuncRaw, binaryPoint);
			if ((binaryPoint != 0) && (GetStatusParserFilter() & kParserNotFilterDecQ8)) {
			  dstPtr += SPrintF(dstPtr, "\nData in decimal format:\n");
			  dstPtr += PrintBufferData(dstPtr, 1 << dSizeShift, msgLen, (void *) pData16, 8, prnFuncFmt, binaryPoint);
			}
		}
			break;
		case kDsl993p2FeTxPwrLD:
		case kDsl993p2NeTxPwrLD:
		case kDsl993p2FeQlnLD:
		case kDsl993p2FeHlogLD:
		case kDsl993p2FeSnrLD:
		case kDsl993p2FeHlinLD:
		case kDsl993p2NeQlnLD:
		case kDsl993p2NeHlogLD:
		case kDsl993p2NeSnrLD:
		case kDsl993p2NeHlinLD:
		case kDsl993p2FePbLatnLD:
		case kDsl993p2FePbSatnLD:
		case kDsl993p2FePbSnrLD:
		case kDsl993p2NePbLatnLD:
		case kDsl993p2NePbSatnLD:
		case kDsl993p2NePbSnrLD:
		{
			int numerOfBytes=2;
			switch (status->param.value)
			{
			case kDsl993p2FeQlnLD:
			  dstPtr += SPrintF(dstPtr, "LD Qln Fe (-(uchar)(x)/2 - 23 dBm/Hz)[255->no measurement]\n");
			  numerOfBytes=1;
			  break;
			case kDsl993p2FeSnrLD:
			  dstPtr += SPrintF(dstPtr, "LD Snr Fe ((uchar)(x)/2 - 32 dBm/Hz)[255->no measurement]\n");
			  numerOfBytes=1;
			  break;
			case kDsl993p2NeQlnLD:
			  dstPtr += SPrintF(dstPtr, "LD Qln Ne (-(uchar)(x)/2 - 23 dBm/Hz)[255->no measurement]\n");
			  numerOfBytes=1;
			  break;
			case kDsl993p2NeSnrLD:
			  dstPtr += SPrintF(dstPtr, "LD Snr Ne ((uchar)(x)/2 - 32 dBm/Hz) [255->no measurement]\n");
			  numerOfBytes=1;
			  break;
			case kDsl993p2FeHlogLD:
			   dstPtr += SPrintF(dstPtr, "LD Hlog Fe (6 - (ushort)(x)/10 dB) [1023->no measurement]\n");
			  break;
			case kDsl993p2NeHlogLD:
			    dstPtr += SPrintF(dstPtr, "LD Hlog Ne (6 - (ushort)(x)/10 dB) [1023->no measurement]\n");
			  break;
			case kDsl993p2FeHlinLD:
			  dstPtr += SPrintF(dstPtr, "LD Hlin Fe (x[n]/2^30)*(x[n+1]+jx[n+2]) [65535->no measurement]\n");
			  break;
			case kDsl993p2NeHlinLD:
			  dstPtr += SPrintF(dstPtr, "LD Hlin Ne (x[n]/2^30)*(x[n+1]+jx[n+2])[65535->no measurement]\n");
			  break;
			case kDsl993p2FePbLatnLD:
			  dstPtr += SPrintF(dstPtr, "LD Per Band LATN Fe bands[0..4](ushort)(x)/10 dBm [1023->no measurement]\n");
			  break;
			case kDsl993p2NePbLatnLD:
			  dstPtr += SPrintF(dstPtr, "LD Per Band LATN Ne bands[1..5](ushort)(x)/10 dBm [1023->no measurement]\n");
			  break;
			case kDsl993p2FePbSatnLD:
			  dstPtr += SPrintF(dstPtr, "LD Per Band SATN Fe bands[0..4](ushort)(x)/10 dBm [1023->no measurement]\n");
			  break;
			case kDsl993p2NePbSatnLD:
			  dstPtr += SPrintF(dstPtr, "LD Per Band SATN Ne bands[1..5](ushort)(x)/10 dBm [1023->no measurement]\n");
			  break;
			case kDsl993p2FePbSnrLD:
			  dstPtr += SPrintF(dstPtr, "LD Per Band SNR Fe Average + bands[0..4](short)(x)/10 dBm [65024->no measurement]\n");
			  break;
			case kDsl993p2NePbSnrLD:
			  dstPtr += SPrintF(dstPtr, "LD Per Band SNR Ne Average + bands[1..5](short)(x)/10 dBm [65024->no measurement]\n");
			  break;
			case kDsl993p2FeTxPwrLD:
			  dstPtr += SPrintF(dstPtr, "LD Tx Power Fe (short)(x)/10 dBm\n");
			  break;
			case kDsl993p2NeTxPwrLD:
			  dstPtr += SPrintF(dstPtr, "LD Tx Power Ne (short)(x)/10 dBm\n");
			  break;
			}
			msgLen = (status->param.dslClearEocMsg.msgType & 0xFFFF)/numerOfBytes;
			dstPtr += PrintBufferDataDec(dstPtr,numerOfBytes-1,msgLen,(void *) pData16, 0/*u/s*/, 0/*q*/);
		}
			break;
		case kDsl993p2FeAttnLD:
			dstPtr+=SPrintF(dstPtr, "LD Max Attainable Data Rate (bps) Fe %d\n",pData32[0]);
			break;
		case kDsl993p2NeAttnLD:
			dstPtr+=SPrintF(dstPtr, "LD Max Attainable Data Rate (bps) Ne %d\n",pData32[0]);
			break;
		case kDsl993p2NeBi:
		case kDsl993p2FeBi:
			switch (status->param.value)
			{
				case kDsl993p2NeBi:
					dstPtr += SPrintF(dstPtr, "Near End Bi (negotiated BP)\n");
					break;
				case kDsl993p2FeBi:
					dstPtr += SPrintF(dstPtr, "Far End Bi (negotiated BP)\n");
					break;
			}
			dstPtr += PrintBufferDataDec(dstPtr,0,msgLen,(void*)pData16,0,0);
			break;
		case kDsl993p2DOINeBi:
		case kDsl993p2DOIFeBi:
			switch (status->param.value)
			{
				case kDsl993p2DOINeBi:
					dstPtr += SPrintF(dstPtr, "DOI Near End Bi (negotiated BP)\n");
					break;
				case kDsl993p2DOIFeBi:
					dstPtr += SPrintF(dstPtr, "DOI Far End Bi (negotiated BP)\n");
					break;
			}
			dstPtr += PrintBufferDataDec(dstPtr,0,msgLen,(void*)pData16,0,0);
			break;
		case kDsl993p2FeBiPhy:
			dstPtr += SPrintF(dstPtr, "Far End Bi (PHY BP)\n");
			dstPtr += PrintBufferDataDec(dstPtr,0,msgLen,(void*)pData16,0,0);
			break;
		case kDsl993p2NeBiPhy:
			dstPtr += SPrintF(dstPtr, "Near End Bi (PHY BP)\n");
			dstPtr += PrintBufferDataDec(dstPtr,0,msgLen,(void*)pData16,0,0);
			break;
		case kDsl993p2BitSwapTones:
			msgLen = msgLen>>1;
			dstPtr += SPrintF(dstPtr, "Bit Swap Tone List (PHY BP):\n");
			dstPtr += PrintBufferData(dstPtr, 2, msgLen, (void *) pData16, 8, (printDataHandler)PrintShortDecS, 0);
			break;
#endif
		case kDsl993p2FramerDeframerUs:
		case kDsl993p2FramerDeframerDs:
		{
			FramerDeframerOptions *options = (FramerDeframerOptions*)(status->param.dslClearEocMsg.dataPtr);
			if(status->param.value==kDsl993p2FramerDeframerUs)
				dstPtr += SPrintF(dstPtr, "Us Framer Parameters: latency path %d \n",options->path);
			else
				dstPtr += SPrintF(dstPtr, "Ds Framer Parameters: latency path %d \n",options->path);

			dstPtr += SPrintF(dstPtr, "Options: B0 %d B1 %d\n",options->B,options->b1);
			dstPtr += SPrintF(dstPtr, "Options: S %d/%d\n",options->S.num,options->S.denom);
			dstPtr += SPrintF(dstPtr, "Options: D %d\n",options->D);
			dstPtr += SPrintF(dstPtr, "Options: N %d\n",options->N);
			dstPtr += SPrintF(dstPtr, "Options: L %d\n",options->L);
			dstPtr += SPrintF(dstPtr, "Options: U %d\n",options->U);
			dstPtr += SPrintF(dstPtr, "Options: I %d\n",options->I);
			dstPtr += SPrintF(dstPtr, "Options: R %d\n",options->R);
			dstPtr += SPrintF(dstPtr, "Options: M %d\n",options->M);
			dstPtr += SPrintF(dstPtr, "Options: T %d\n",options->T);
			dstPtr += SPrintF(dstPtr, "Options: G %d\n",options->G);
			dstPtr += SPrintF(dstPtr, "Options: F %d\n",options->F);
			dstPtr += SPrintF(dstPtr, "Options: codingType %d\n",options->codingType);
			dstPtr += SPrintF(dstPtr, "Options: fireEnabled %d\n",options->fireEnabled);
			dstPtr += SPrintF(dstPtr, "Options: fireRxQueue %d\n",options->fireRxQueue);
			dstPtr += SPrintF(dstPtr, "Options: tpsTcOptions %d\n",options->tpsTcOptions);
			dstPtr += SPrintF(dstPtr, "Options: delay %d\n",options->delay);
			dstPtr += SPrintF(dstPtr, "Options: INP %d\n",options->INP);
			if (msgLen >= GINP_FRAMER_INPSHINE_STRUCT_SIZE)
				dstPtr += SPrintF(dstPtr, "Options: INPshine %d\n",options->INPshine);
			dstPtr += SPrintF(dstPtr, "Options: ovhType %d\n",options->ovhType);
			dstPtr += SPrintF(dstPtr, "Options: ahifChanId B0 = %d B1 = %d\n",options->ahifChanId[0],options->ahifChanId[1]);
			dstPtr += SPrintF(dstPtr, "Options: tmType B0 = %d B1 = %d\n",options->tmType[0],options->tmType[1]);
#ifdef STATUSPARSER_STATE
			parserState[lineId].tmType[kDsl993p2FramerDeframerUs == status->param.value] = options->tmType[0];
#endif
			if (msgLen >= GINP_FRAMER_STRUCT_SIZE) {
				dstPtr += SPrintF(dstPtr, "Options: fireTxQueue = %d\n",options->fireTxQueue);
				dstPtr += SPrintF(dstPtr, "Options: phyRrrcBits = %d\n",options->phyRrrcBits);
				dstPtr += SPrintF(dstPtr, "Options: ginpFraming = 0x%X\n",options->ginpFraming);
				dstPtr += SPrintF(dstPtr, "Options: INPrein = %d\n",options->INPrein);
				dstPtr += SPrintF(dstPtr, "Options: Q = %d\n",options->Q);
				dstPtr += SPrintF(dstPtr, "Options: V = %d\n",options->V);
				dstPtr += SPrintF(dstPtr, "Options: QrxBuffer = %d\n",options->QrxBuffer);
				if (msgLen >= GINP_FRAMER_ETR_STRUCT_SIZE)
					dstPtr += SPrintF(dstPtr, "Options: ETR_kbps = %d\n",options->ETR_kbps);
			}
#if defined(GFAST_SUPPORT) || defined(WINNT) || defined(LINUX_DRIVER)
			if(msgLen >= ETR_MIN_EOC_FRAMER_STRUCT_SIZE) {
				dstPtr += SPrintF(dstPtr, "Options: maxMemory = %d\n",options->maxMemory);
				dstPtr += SPrintF(dstPtr, "Options: ndr = %d\n",options->ndr);
				dstPtr += SPrintF(dstPtr, "Options: Ldr = %d\n",options->Ldr);
				dstPtr += SPrintF(dstPtr, "Options: Nret = %d\n",options->Nret);
			}
#endif
		}
			break;

		case kDsl993p2FramerAdslDs:
		case kDsl993p2FramerAdslUs:
		{
			FramerDeframerOptions *options =(FramerDeframerOptions*)(status->param.dslClearEocMsg.dataPtr);

			if(status->param.value==kDsl993p2FramerAdslUs)
				dstPtr += SPrintF(dstPtr, "ADSLo68 US extra Framing Parameters: latency path %d \n",options->path);
			else
				dstPtr += SPrintF(dstPtr, "ADSLo68 DS extra Framing Parameters: latency path %d \n",options->path);

			dstPtr += SPrintF(dstPtr, "Options: ovhType %d\n",options->ovhType);
			dstPtr += SPrintF(dstPtr, "Options: ahifChanId B0 = %d B1 = %d\n",options->ahifChanId[0],options->ahifChanId[1]);
			dstPtr += SPrintF(dstPtr, "Options: tmType B0 = %d B1 = %d\n",options->tmType[0],options->tmType[1]);
#ifdef STATUSPARSER_STATE
			parserState[lineId].tmType[kDsl993p2FramerAdslUs == status->param.value] = options->tmType[0];
#endif
		}
			break;
		case kGinpMonitoringCounters:
		{
			GinpCounters *pGinp = (GinpCounters *) status->param.dslClearEocMsg.dataPtr;
			dstPtr+=SPrintF(dstPtr, "GinpCnt: rtx_tx=%u  rtx_c=%u  rtx_uc=%u  LEFTRS=%u  errFreeBits=%u  minEFTR=%u",
				pGinp->rtx_tx, pGinp->rtx_c, pGinp->rtx_uc, pGinp->LEFTRS, pGinp->errFreeBits, pGinp->minEFTR);
			if(msgLen > sizeof(ginpCounters))
				dstPtr+=SPrintF(dstPtr, "  SEFTR=%u\n", pGinp->SEFTR);
			else
				dstPtr+=SPrintF(dstPtr, "\n");
		}
			break;
		case kGfastEocMonitoringCounters:
		{
			GfastEocCounters *pGfastEocCnts = (GfastEocCounters *) status->param.dslClearEocMsg.dataPtr;
			if (msgLen < sizeof(GfastEocCounters)) /* older PHY w/o packetsRxCrcErr */
			  dstPtr+=SPrintF(dstPtr, "GfastEocCnt: [Bt Msg Pkt] RX=(%u %u %u) TX=(%u %u %u)\n",
					pGfastEocCnts->bytesReceived, pGfastEocCnts->messagesReceived, pGfastEocCnts->packetsReceived,
					pGfastEocCnts->bytesSent, pGfastEocCnts->messagesSent, pGfastEocCnts->packetsSent);
			else
			  dstPtr+=SPrintF(dstPtr, "GfastEocCnt: [Bt Msg Pkt PktErr] RX=(%u %u %u %u) TX=(%u %u %u)\n",
					pGfastEocCnts->bytesReceived, pGfastEocCnts->messagesReceived, pGfastEocCnts->packetsReceived, pGfastEocCnts->packetsRxCrcErr,
					pGfastEocCnts->bytesSent, pGfastEocCnts->messagesSent, pGfastEocCnts->packetsSent);
		}
			break;
		case kDslGfastVectoringEocSegments:
		{
			GfastTxVectorFBEocSegment *GfastTxVectorFBEocSegmentPtr = (GfastTxVectorFBEocSegment *) status->param.dslClearEocMsg.dataPtr;
			dstPtr += SPrintF(dstPtr,"Gfast Vec Feed Back EOC Stats: EOC Msg processed: %u, Dropped: %u, EOC segments sent: %u, Dropped: %u",
					GfastTxVectorFBEocSegmentPtr->cntVecFBMessageSend,
					GfastTxVectorFBEocSegmentPtr->cntVecFBMessageDrop,
					GfastTxVectorFBEocSegmentPtr->cntVecFBSegmentSend,
					GfastTxVectorFBEocSegmentPtr->cntVecFBSegmentDrop
					);
		}
			break;
#ifndef LITE_VERSION
		case kStatusBufferHistogram:
			dataSize = (status->param.dslClearEocMsg.msgType & kDslDbgDataSizeMask) >> 16;
			dstPtr += SPrintF(dstPtr, "Status buffer write Histogram:\n");
			dstPtr += PrintBufferDataDec(dstPtr, dataSize, msgLen >> dataSize,
				(void *) status->param.dslClearEocMsg.dataPtr,
				status->param.dslClearEocMsg.msgType & kDslDbgDataSignMask,
				(status->param.dslClearEocMsg.msgType & kDslDbgDataQxMask) >> kDslDbgDataQxShift);
			break;
#endif
		case kDsl993p2MaxRate:
			dstPtr+=SPrintF(dstPtr, "Max Attainable Data Rate (kbps) US %d DS %d\n",pData32[1],pData32[0]);
			break;
		case kDsl993p2PowerNeTxTot:
			dstPtr += SPrintF(dstPtr, "G993 report status Near End Tx Power ");
			dstPtr += PrintShortQxS(dstPtr, pData16, 8);
			dstPtr += SPrintF(dstPtr, "\n");
			break;
		case kDsl993p2NeRxPower:
			dstPtr += SPrintF(dstPtr, "G993 report status Near End Rx Power ");
			dstPtr += PrintShortQxS(dstPtr, pData16, 8);
			dstPtr += SPrintF(dstPtr, "\n");
			break;
		case kDsl993p2PowerNeTxPb:
			msgLen = msgLen >> 1;
			dstPtr += PrintQ8dBBufferData(dstPtr, "G993 report status Near End Tx Power per band\n", msgLen, pData16);
			break;
		case kDsl993p2PowerFeTxTot:
			dstPtr += SPrintF(dstPtr, "G993 report status Far End Tx Power ");
			dstPtr += PrintShortQxS(dstPtr, pData16, 8);
			dstPtr += SPrintF(dstPtr, "\n");
			break;
		case kDsl993p2PowerFeTxPb:
			msgLen = msgLen >> 1;
			dstPtr += PrintQ8dBBufferData(dstPtr, "G993 report status Far End Tx Power per band\n", msgLen, pData16);
			break;
		case kDsl993p2SNRM:
			dstPtr += SPrintF(dstPtr, "G993 report status SNR ratio margin ");
			dstPtr += PrintShortQxS(dstPtr, pData16, 4);
			dstPtr += SPrintF(dstPtr, "\n");
			break;
		case kDsl993p2SNRMpb:
			msgLen = msgLen >> 1;
			dstPtr += PrintQ4dBBufferData(dstPtr, "G993 report status SNR ratio margin per band\n", msgLen, pData16);
			break;
	  case kDsl993p2SnrROC:
			dstPtr += SPrintF(dstPtr, "G993 report status ROC SNR ");
			dstPtr += PrintShortQxS(dstPtr, pData16, 8);
			dstPtr += SPrintF(dstPtr, "\n");
			break;
		case kDsl993p2dsATTNDR:
			dstPtr+=SPrintF(dstPtr, "Max Attainable Data Rate (kbps) Showtime DS %d\n",pData32[0]);
			break;
		case kDsl993p2BpType:
			dstPtr+=SPrintF(dstPtr, "G993 report status Band Plan Phase 0/1 -> Discovery/Medley %d\n",pData32[0]);
			break;
		case kDslNtrCounters:
		{
			ntrCntStruct *pNtr = (ntrCntStruct *) status->param.dslClearEocMsg.dataPtr;
			dstPtr+=SPrintF(dstPtr, "NTR: @DMT=(%u %u %u) @ntr=(%u %u %u)\n",
				pNtr->ncoOutCntAtDmt, pNtr->lcoCntAtDmt, pNtr->ncoRefCntAtDmt,
				pNtr->ncoOutCntAtNtr, pNtr->lcoCntAtNtr, pNtr->ncoRefCntAtNtr);
		}
			break;
#if 0
		case kDslNtrStates:
		{
			ntrStateStruct *pNtrState = (ntrStateStruct *) status->param.dslClearEocMsg.dataPtr;
			dstPtr+=SPrintF(dstPtr, "NTR phase lock state: %d (0->off 1->on)\n",
			pNtrState->PLLlockState);
		}
			break;
#endif
		case kDslTodTimeStamp:
		{
			dstPtr+=SPrintF(dstPtr, "TOD: time stamp report to driver = %x +%x second and %x nanoSecond\n",
				(uint)(pData32[0]), (uint)(pData32[1]), pData32[2]);  
		}
			break;
		case kDslExcpRegs:
			excSp = ((uint*) status->param.dslClearEocMsg.dataPtr)[28];
			dstPtr += PrintExcpReg(dstPtr, (void *) status->param.dslClearEocMsg.dataPtr);
			break;
		case kDslExcpArgs:
			dstPtr += PrintExcpArgs(dstPtr, (status->param.dslClearEocMsg.msgType & 0xFFFF) >> 2,
				(void *) status->param.dslClearEocMsg.dataPtr);
			break;
		case kDslExcpStack:
			dstPtr += PrintExcpStack(dstPtr, excSp, (status->param.dslClearEocMsg.msgType & 0xFFFF) >> 2,
				(void *) status->param.dslClearEocMsg.dataPtr);
			break;
		case kDslG994VendorId:
#ifdef XDSLDRV_ENABLE_PARSER
			pData8 = (uchar*)pDataPtrOrig;
#endif
			dstPtr += SPrintF(dstPtr, "G.994.1 received vendor ID : %c%c%c%c\n",
				PrintVendorIDChar(pData8[0]),
				PrintVendorIDChar(pData8[1]),
				PrintVendorIDChar(pData8[2]),
				PrintVendorIDChar(pData8[3]));
			break;
		/* TR98 group */
		case kDslActualCE:
			dstPtr += SPrintF(dstPtr, "TR98 ActualCE = %d\n", *pData32);
			break;
		case kDslQLNmtDs:
			dstPtr += SPrintF(dstPtr, "TR98 QLNmtDs = %d\n", *pData32);
			break;
		case kDslQLNmtUs:
			dstPtr += SPrintF(dstPtr, "TR98 QLNmtUs = %d\n", *pData32);
			break;
		case kDslSNRmtDs:
			dstPtr += SPrintF(dstPtr, "TR98 SNRmtDs = %d\n", *pData32);
			break;
		case kDslSNRmtUs:
			dstPtr += SPrintF(dstPtr, "TR98 SNRmtUs = %d\n", *pData32);
			break;
		case kDslHLOGmtDs:
			dstPtr += SPrintF(dstPtr, "TR98 HLOGmtDs = %d\n", *pData32);
			break;
		case kDslHLOGmtUs:
			dstPtr += SPrintF(dstPtr, "TR98 HLOGmtUs = %d\n", *pData32);
			break;
		case kDslUPBOkle:
			dstPtr += SPrintF(dstPtr, "TR98 UPBOkle = %d\n", *pData16);
			break;
		case kDslUPBOkleCpe:
			dstPtr += SPrintF(dstPtr, "TR98 UPBOkleCPE = %d\n", *pData16);
			break;
		case kDslSNRModeDs:
			dstPtr += SPrintF(dstPtr, "TR98 SNRModeDs = %d\n", *pData32);
			break;
		case kDslSNRModeUs:
			dstPtr += SPrintF(dstPtr, "TR98 SNRModeUs = %d\n", *pData32);
			break;
		case kDslActualPSDDs:
			dstPtr += SPrintF(dstPtr, "TR98 ActualPSD_Ds = %s\n", Q8ToFloatString(*pData32));
			break;
		case kDslActualPSDUs:
			dstPtr += SPrintF(dstPtr, "TR98 ActualPSD_Us = %s\n", Q8ToFloatString(*pData32));
			break;
		case kDsl993p2dsATTNDRmethod:
#ifdef XDSLDRV_ENABLE_PARSER
			pData8 = (uchar*)pDataPtrOrig;
#endif
			dstPtr += SPrintF(dstPtr, "ATTDR method = %d\n", *pData8);
			break;
		case kDsl993p2dsATTNDRinp:
#ifdef XDSLDRV_ENABLE_PARSER
			pData8 = (uchar*)pDataPtrOrig;
#endif
			dstPtr += SPrintF(dstPtr, "ATTDR Inp = %d\n", *pData8);
			break;
		case kDsl993p2dsATTNDRdel:
#ifdef XDSLDRV_ENABLE_PARSER
			pData8 = (uchar*)pDataPtrOrig;
#endif
			dstPtr += SPrintF(dstPtr, "ATTDR delay = %d\n", *pData8);
			break;

		case kDslAfeInfoCmd:
			{
			afeDescStruct *pAfeInfo = (afeDescStruct *) status->param.dslClearEocMsg.dataPtr;
			dstPtr += SPrintF(dstPtr, "DrvAfeInfo: chipId=0x%08X afeId=0x%08X cfg0=0x%08X cfg1=0x%08X\n", 
				pAfeInfo->chipId, pAfeInfo->boardAfeId, pAfeInfo->afeChidIdConfig0, pAfeInfo->afeChidIdConfig1);
			}
			break;
		case kDslPhyInfoCmd:
			{
#if 0
			adslPhyInfo *pPhyInfo = (adslPhyInfo *) status->param.dslClearEocMsg.dataPtr;
			dstPtr += SPrintF(dstPtr, "PhyInfo: sdramAddr=0x%08X sdramSize=0x%08X\n", 
				pPhyInfo->sdramImageAddr, pPhyInfo->sdramImageSize);
#else
			uint *pPhyInfo = (uint *) status->param.dslClearEocMsg.dataPtr;
			dstPtr += SPrintF(dstPtr, "PhyInfo: dw0=0x%08X dw1=0x%08X dw2=0x%08X dw3=0x%08X\n", 
				pPhyInfo[0], pPhyInfo[1], pPhyInfo[2], pPhyInfo[3]);
			dstPtr += SPrintF(dstPtr, "PhyInfo: f0=0x%08X f1=0x%08X f2=0x%08X f3=0x%08X\n", 
				pPhyInfo[7], pPhyInfo[8], pPhyInfo[9], pPhyInfo[10]);
			memcpy((void *)&parserCtrl.phyFeatures[0], (void *)&pPhyInfo[7], sizeof(parserCtrl.phyFeatures));
#endif
			}
			break;

#ifndef LITE_VERSION
		case kDslAFERegInfo:
			dataSize = (status->param.dslClearEocMsg.msgType & kDslDbgDataSizeMask) >> 16;
			/* now only dump out the register, need to expand and parse relavent bits base on chip/afeID */
			dstPtr += PrintBufferDataHex(dstPtr, dataSize, msgLen >> dataSize, (void *) status->param.dslClearEocMsg.dataPtr);
		break;

		case kDslStackDump:
			{
#ifdef XDSLDRV_ENABLE_PARSER
			uint *pData = (uint *)pDataPtrOrig;
#else
			uint *pData = (uint *) status->param.dslClearEocMsg.dataPtr;
#endif
			uint *pDataEnd  = pData + ((status->param.dslClearEocMsg.msgType & 0xFFFF) >> 2);
			uint *pDataStart = pData;

			if ((pData != pDataEnd) && (0xDEADBEEF == *pData))
				pData++;
			while (pData != pDataEnd) {
			  if (0x5A5A5A5A != *pData) {
				dstPtr += SPrintF(dstPtr, "***FATAL: Stack usage found from offset=%ld\n", (uintptr_t)pData - (uintptr_t)pDataStart);
				break;
			  }
			  pData++;
			}
			dstPtr += PrintBufferDataGen(status, dstPtr);
			}
			break;
		case kDslGfastAhifRegDump:
			{
#ifdef XDSLDRV_ENABLE_PARSER
			uint *pReg  = (uint *)pDataPtrOrig;
#else
			uint *pReg = (uint *) status->param.dslClearEocMsg.dataPtr;
#endif
			dstPtr += SPrintF(dstPtr, "Gfast AHIF reg dump:\n");
			dstPtr += PrintBufferDataGen(status, dstPtr);

			dstPtr += SPrintF(dstPtr, "\nGFF: GFAST_CONTROL = 0x%08X GFF_CONFIG = 0x%08X idleIns=0x%08X\n", pReg[0], pReg[4], pReg[7]);
			dstPtr += SPrintF(dstPtr, "GFF: sidNormal = 0x%08X sidDummy = 0x%08X\n", pReg[26] & 0x7FF, (pReg[26] >> 11) & 0x7FF);
			dstPtr += SPrintF(dstPtr, "GFF: dataPktIn = %u dataPktOut = %u\n", pReg[20], pReg[22]);
			dstPtr += SPrintF(dstPtr, "GFF: eocMsgSent = %u\n", pReg[18]);
			dstPtr += SPrintF(dstPtr, "GFF: dtuPtr=0x%08X progPtr=0x%08X ts=0x%08X dtuLen=0x%08X cmd=0x%08X \n", pReg[12], pReg[25], pReg[13], pReg[14], pReg[15]);

			dstPtr += SPrintF(dstPtr, "GFD: dataPktIn = %u dataPktOut = %u dropped=%u\n", pReg[48], pReg[50], (pReg[33] >> 1) & 1);
			dstPtr += SPrintF(dstPtr, "GFD: eocMsgRcv = %u dropped=%u\n", pReg[44], pReg[33] & 1);
			dstPtr += SPrintF(dstPtr, "GFD: dtuPtr=0x%08X progPtr=0x%08X dtuLen=0x%08X cmd=0x%08X\n", pReg[40], pReg[43], pReg[41], pReg[42]);
			dstPtr += SPrintF(dstPtr, "GFD: exDtuLen=%u sfInPkt=%u efOutPktt=%u\n", (pReg[33] >> 2) & 1, (pReg[33] >> 3) & 1, (pReg[33] >> 4) & 1);
			}
			break;
#endif
	}
	return dstPtr - dstPtr0;
}

#ifdef CONFIG_ARM64
extern uint	*pStackPtr;
#endif

Public	int
StatusParser(modemStatusStruct *status, char *dstPtr)
	{
	static	uint headerCounter=0;
	static	uint curLineId = 0;
	uint	lineId;
	char	*dstPtr00, *dstPtr0, *dstPtr1;

	dstPtr00 = dstPtr;
	lineId = ((uint) status->code) >> DSL_LINE_SHIFT;
	if (lineId != curLineId) {
		dstPtr += SPrintF(dstPtr, "<<< Line%d >>>\t\n", (int)lineId);
		curLineId = lineId;
	}

	dstPtr0 = dstPtr;
#if defined(WINNT) || defined(LINUX_DRIVER)
	if ((curLineId != 0) && !DslDiagIsBondingSlave())
#else
	if (curLineId != 0)
#endif
	{
	  dstPtr[0] = ' ';
	  dstPtr[1] = ' ';
	  dstPtr[2] = ' ';
	  dstPtr[3] = ' ';
	  dstPtr += 4;
	}
	dstPtr1 = dstPtr;
	dstPtr[0] = 0;

	switch	(DSL_STATUS_CODE(status->code))
		{
		case 	kDslError:
#ifndef LITE_VERSION
			{
			int  nPrintf = status->param.value & 0xFFFF;
			dstPtr += SPrintF(dstPtr, "StatusLost: nInfo=%u nPrintf=%u\n", (status->param.value >> 16) - nPrintf, nPrintf);
			}
#endif
			break;
		case kDslEpcAddrStatus:
#ifndef LITE_VERSION
			dstPtr += SPrintF(dstPtr, "PHY EPC Address : 0x%08X\n", status->param.value);
#endif
			break;
		case kDslPwrMgrSrAddrStatus:
#ifndef LITE_VERSION
			dstPtr += SPrintF(dstPtr, "PHY DDR_SR sync address : 0x%08X\n", status->param.value);
#endif
			break;
		case kDslWakeupRequest:
#ifndef LITE_VERSION
			dstPtr += SPrintF(dstPtr, "Line wakeup request: param=%d\n", status->param.value);
#endif
			break;
		case kDslTODactiveLine:
#ifndef LITE_VERSION
			dstPtr += SPrintF(dstPtr, "Current line TOD status: param=%d (active:1; inactive:0)\n", status->param.value);
#endif
			break;	
		case kDslNTRPlllockStatus:
#ifndef LITE_VERSION
			dstPtr += SPrintF(dstPtr, "Current line NTR PLL status: param=%d (locked:1; unlocked:0)\n", status->param.value);
#endif
			break;				
		case 	kDslATUHardwareAGCRequest:
#ifndef LITE_VERSION
			dstPtr += SPrintF(dstPtr, "DSL transceiver hardware AGC level adjustment request : %s dB\n",
				Q4ToFloatString(status->param.value));
#endif
			break;
#ifdef BCM6348_SRC
		case		kDslHardwareAGCSetPga1:
			dstPtr += SPrintF(dstPtr, "DSL setting PGA 1\n");
			break;
		case		kDslHardwareAGCDecPga1:
			dstPtr += SPrintF(dstPtr, "DSL reducing PGA 1 by one gain step\n");
			break;
		case		kDslHardwareAGCIncPga1:
			dstPtr += SPrintF(dstPtr, "DSL increasing PGA 1 by one gain step\n");
			break;
		case		kDslHardwareAGCSetPga2:
			dstPtr += SPrintF(dstPtr, "DSL setting PGA 2\n");
			break;
		case		kDslHardwareAGCSetPga2Delta:
			dstPtr += SPrintF(dstPtr, "DSL adjusting PGA 2\n");
			break;
		case		kDslHardwareGetRcvAGC:
			dstPtr += SPrintF(dstPtr, "DSL retrieving hwAGC\n");
			break;
#endif
		case kDslHardwareSetRcvAGC:
			dstPtr += SPrintF(dstPtr, "DSL transceiver hardware AGC level adjustment : %s dB\n",
				Q4ToFloatString(status->param.value));
			break;
		case		kDslFeaturesUnsupported:
			dstPtr += SPrintF(dstPtr, "DSL Features not supported\n");
			break;
		case		kDslEscapeToG994p1Status:
			dstPtr += SPrintF(dstPtr, "DSL start training\n");
#ifdef STATUSPARSER_STATE
			memset(parserState + lineId, kParserStateUndefined, sizeof(parserState[0]));
			parserState[lineId].trainingState = kAdslTrainingIdle;
			parserState[lineId].fireMap       = 0;
			parserState[lineId].ginpMap       = 0;
			parserState[lineId].vectCntReport = 0;
			parserState[lineId].NLp[0]= 0;
		    parserState[lineId].NLp[1]= 0;
			parserState[lineId].NLpValid = 0;
			parserState[lineId].tmType[0]= -1;
		    parserState[lineId].tmType[1]= -1;
#endif
			break;
		case		kDslEscapeToT1p413Status:
			dstPtr += SPrintF(dstPtr, "DSL start training in T1413\n");
#ifdef STATUSPARSER_STATE
			memset(parserState + lineId, kParserStateUndefined, sizeof(parserState[0]));
			parserState[lineId].trainingState = kAdslTrainingIdle;
			parserState[lineId].fireMap       = 0;
			parserState[lineId].ginpMap       = 0;
			parserState[lineId].vectCntReport = 0;
			parserState[lineId].NLp[0]= 0;
		    parserState[lineId].NLp[1]= 0;
			parserState[lineId].NLpValid = 0;
			parserState[lineId].tmType[0]= -1;
		    parserState[lineId].tmType[1]= -1;
#endif
			break;
		case		kDslResetOtherLine:
			dstPtr += SPrintF(dstPtr, "DSL reset other line(%u) mode=%u\n", lineId ^ 1, status->param.value);
			break;
		case		kDslResetOtherLineSticky:
			dstPtr += SPrintF(dstPtr, "DSL sticky reset other line(%u) mode=%u\n", lineId ^ 1, status->param.value);
			break;

		case		kDslTrainingStatus:
			{
			dslTrainingStatusCode	code = status->param.dslTrainingInfo.code;
			int					value = status->param.dslTrainingInfo.value;
			switch (code)
				{
				case kDslAfeLineDriverType:
					dstPtr += SPrintF(dstPtr, "LineDriver: 0x%x\n",(unsigned int)value);
					break;
				case kG992RcvDelay:
					dstPtr += SPrintF(dstPtr, "Delay:  DS %d\n", (unsigned short)value);
					break;
				case kG992RcvInp:
					dstPtr += SPrintF(dstPtr, "Inp:  DS %d\n", (unsigned short)value);
					break;
				case kG992FireState:
					dstPtr += SPrintF(dstPtr, "FIRE Status:  DS %s\tUS %s\n",
						(value & kFireDsEnabled) ? "On": "Off",
						(value & kFireUsEnabled) ? "On": "Off");
					break;
				case kDslBondingState:
					if (value)
						dstPtr += SPrintF(dstPtr,"BONDING Status(%d): %s", value,
								(value & 0x1) ? "PTM": "ATM");
					else
						dstPtr += SPrintF(dstPtr,"BONDING Status: Off");
					break;
				case kDslGfastCOSupport:
					dstPtr += SPrintF(dstPtr,"GFAST Support Status : %d", value);
					break;
				case kG992DataRcvDetectLOR:
					dstPtr += SPrintF(dstPtr, "Loss of RMC(lor) detected\n");
					break;
				case kG992DataRcvDetectLORRecovery:
					dstPtr += SPrintF(dstPtr, "(lor) recovered\n");
					break;
				case kG992DataRcvDetectFeLOR:
					dstPtr += SPrintF(dstPtr, "Far-end loss of RMC(lor-fe) detected\n");
					break;
				case kG992DataRcvDetectFeLORRecovery:
					dstPtr += SPrintF(dstPtr, "(lor-fe) recovered\n");
					break;
				case kG992DataRcvDetectLOM:
					dstPtr += SPrintF(dstPtr, "Loss of margin(lom) detected\n");
					break;
				case kG992DataRcvDetectLOMRecovery:
					dstPtr += SPrintF(dstPtr, "(lom) recovered\n");
					break;
				case kG992DataRcvDetectFeLOM:
					dstPtr += SPrintF(dstPtr, "Far-end loss of margin(lom-fe) detected\n");
					break;
				case kG992DataRcvDetectFeLOMRecovery:
					dstPtr += SPrintF(dstPtr, "(lom-fe) recovered\n");
					break;
				case kG992ReinitTimeThld:
					dstPtr += SPrintF(dstPtr, "PHY ReinitTimethld: %d \n", value);
					break;
				case kDslRiPolicyReinitTimeThreshold:
					dstPtr += SPrintF(dstPtr, "RiPolicy Reinit Time: %d \n", value);
					break;
				case kDslPtmOptionsDs:
				case kDslPtmOptionsUs:
					dstPtr += SPrintF(dstPtr, "PTM options %s: 0x%X \n", (kDslPtmOptionsDs == code ? "DS" : "US"), value);
					break;
				case kDslVectoringFriendlyEnabled:
					dstPtr += SPrintF(dstPtr,"SP: Vectoring Friendly is enabled\n");
					break;
				case kDslVectoringEnabled:
					dstPtr += SPrintF(dstPtr,"SP: Vectoring is enabled, value=%d\n",value);
					break;
				case kDslVectoringLineId:
					dstPtr += SPrintF(dstPtr,"SP: Vectoring line ID DS: %d\n", value);
					break;
				case kDslVectoringReportErrorSampleCounters:
					dstPtr += SPrintF(dstPtr,"SP: Vectoring error counters report: %d\n", value);
#ifdef STATUSPARSER_STATE
					parserState[lineId].vectCntReport = 1;
#endif
					break;
				case kDslVectoringState:
				    switch(value)
					{
					case 0 :
						dstPtr += SPrintF(dstPtr,"Vectoring new state: VECT_WAIT_FOR_CONFIG");
						break;
					case 1 :
						dstPtr += SPrintF(dstPtr,"Vectoring new state: VECT_FULL");
						break;
					case 2:
						dstPtr += SPrintF(dstPtr,"Vectoring new state: VECT_WAIT_FOR_TRIGGER");
						break;
					case 3:
						dstPtr += SPrintF(dstPtr,"Vectoring new state: VECT_RUNNING");
						break;
					default:
						dstPtr += SPrintF(dstPtr,"Vectoring new state: unknown state %d", value);
						break;
					}
				    break;
				case	kDslStartedG994p1:
					dstPtr += SPrintF(dstPtr, "DSL started G.994.1\n");
					break;
				case	kDslStartedT1p413HS:
					dstPtr += SPrintF(dstPtr, "DSL started T1.413 Issue 2 Handshaking\n");
					break;
				case	kDslT1p413ReturntoStartup:
					dstPtr += SPrintF(dstPtr, "DSL Return to T1.413 Issue 2 Handshaking\n");
					break;
				case	kDslG994p1MessageDet:
					dstPtr += SPrintF(dstPtr, "G.994.1 Message Detected Type: %d\n", value);
					break;
				case	kDslG994p1ToneDet:
					dstPtr += SPrintF(dstPtr, "G.994.1 Tone Detected\n");
					break;
				case	kDslG994p1RToneDet:
					dstPtr += SPrintF(dstPtr, "G.994.1 Phase reversal tone detected\n");
					break;
				case	kDslG994p1FlagDet:
					dstPtr += SPrintF(dstPtr, "G.994.1 Flag detected\n");
					break;
				case	kDslG994p1GalfDet:
					dstPtr += SPrintF(dstPtr, "G.994.1 Galf detected\n");
					break;
				case	kDslG994p1SilenceDet:
					dstPtr += SPrintF(dstPtr, "G.994.1 Silence detected\n");
					break;
				case	kDslG994p1ErrorFrameDet:
					dstPtr += SPrintF(dstPtr, "G.994.1 Error frame detected\n");
					break;
				case	kDslG994p1BadFrameDet:
					dstPtr += SPrintF(dstPtr, "G.994.1 Bad frame detected\n");
					break;
				case	kDslG994p1XmtFinished:
					dstPtr += SPrintF(dstPtr, "G.994.1 Transmission finished\n");
					break;
				case	kDslG994p1Timeout:
					dstPtr += SPrintF(dstPtr, "G.994.1 Startup Timeout, Reset\n");
					break;
#ifndef LITE_VERSION
				case kDslIkanosCO4Detected:
					dstPtr += SPrintF(dstPtr, "Ikanos CO4 detected\n");
					break;
#endif
				case	kDslFinishedG994p1:
					dstPtr += SPrintF(dstPtr, "DSL finished G.994.1, selected modulation %s\n",
						GetModulationString(status->param.dslTrainingInfo.value));
#ifdef STATUSPARSER_STATE
					switch (value) {
						case kG992p2AnnexAB:
						case kG992p2AnnexC:
							parserState[lineId].modType = kAdslModGlite;
							break;
						case (kG992p1AnnexI>>4):
							parserState[lineId].modType = kAdslModAnnexI;
							break;
						case kG992p3AnnexA:
						case kG992p3AnnexB:
							parserState[lineId].modType = kAdslModAdsl2;
							break;
						case kG992p5AnnexA:
						case kG992p5AnnexB:
							parserState[lineId].modType = kAdslModAdsl2p;
							break;
						case kG993p2AnnexA:
							parserState[lineId].modType = kVdslModVdsl2;
							parserState[lineId].trainingState = kAdslTrainingG993Started;
							break;
						case kGfastAnnexA:
							parserState[lineId].modType = kXdslModGfast;
							break;
						case kG992p1AnnexA:
						case kG992p1AnnexB:
						case kG992p1AnnexC:
						default:
							parserState[lineId].modType = kAdslModGdmt;
							break;
					}
#endif
					break;
				case	kDslG992p3AnnexLMode:
					dstPtr += SPrintF(dstPtr, "RE-ADSL2 mode: DS=0x%X, US=0x%X\n",
						status->param.dslTrainingInfo.value >> 8, status->param.dslTrainingInfo.value & 0xFF);
					break;
				case	kDslG994p1ReturntoStartup:
					dstPtr += SPrintF(dstPtr, "Return to Startup G.994.1\n");
					break;
				case	kDslG994p1StartupFinished:
					dstPtr += SPrintF(dstPtr, "Startup Finished G.994.1\n");
#ifdef STATUSPARSER_STATE
					parserState[lineId].trainingState = kAdslTrainingG994;
#endif
					break;
				case	kDslG994p1InitiateCleardown:
					dstPtr += SPrintF(dstPtr, "Initiate Cleardown G.994.1\n");
					break;
				case	kDslG994p1RcvNonStandardInfo:
					dstPtr += SPrintF(dstPtr, "G.994.1 non standard info received, len = %d bytes\n", value);
					break;
				case	kDslG994p1XmtNonStandardInfo:
					dstPtr += SPrintF(dstPtr, "G.994.1 non standard info sent, len = %d bytes\n", value);
					break;

				case	kDslStartedG992p2Training:
					dstPtr += SPrintF(dstPtr, "DSL started G.992 long training\n");
					break;
				case	kDslG992p2DetectedPilotSymbol:
					dstPtr += SPrintF(dstPtr, "DSL G.992 detected pilot symbol\n");
					break;
				case	kDslG992p2DetectedReverbSymbol:
					dstPtr += SPrintF(dstPtr, "DSL G.992 detected REVERB symbol\n");
					break;
				case	kDslG992p2TEQCalculationDone:
					dstPtr += SPrintF(dstPtr, "DSL G.992 TEQ calculated\n");
					break;
				case	kDslG992p2TrainingFEQ:
					dstPtr += SPrintF(dstPtr, "DSL G.992 training FEQ\n");
					break;
				case	kDslG992p2Phase3Started:
					dstPtr += SPrintF(dstPtr, "DSL G.992 phase 3 training (channel analysis) started\n");
#ifdef STATUSPARSER_STATE
					parserState[lineId].trainingState = kAdslTrainingG992ChanAnalysis;
#endif
					break;
				case	kDslG992p2ReceivedRates1:
					dstPtr += SPrintF(dstPtr, "DSL G.992 received RATES1\n");
					break;
				case	kDslG992p2ReceivedMsg1:
					dstPtr += SPrintF(dstPtr, "DSL G.992 received MSGS1\n");
					break;
				case	kDslG992p2Phase4Started:
					dstPtr += SPrintF(dstPtr, "DSL G.992 phase 4 training (exchange) started\n");
#ifdef STATUSPARSER_STATE
					parserState[lineId].trainingState = kAdslTrainingG992Exchange;
#endif
					break;
				case	kDslG992p2ReceivedRatesRA:
					dstPtr += SPrintF(dstPtr, "DSL G.992 received RATES-RA\n");
					break;
				case	kDslG992p2ReceivedMsgRA:
					dstPtr += SPrintF(dstPtr, "DSL G.992 received MSGS-RA\n");
					break;
				case	kDslG992p2ReceivedRates2:
					dstPtr += SPrintF(dstPtr, "DSL G.992 received RATES2\n");
					break;
				case	kDslG992p2ReceivedMsg2:
					dstPtr += SPrintF(dstPtr, "DSL G.992 received MSGS2\n");
					break;
				case	kDslG992p2ReceivedMsgLD:
					dstPtr += SPrintF(dstPtr, "DSL G.992 received MSGSLD\n");
					break;
				case	kDslG992p2ReceivedBitGainTable:
					dstPtr += SPrintF(dstPtr, "DSL G.992 received bit & gain table\n");
					break;
				case	kDslRecoveredFromImpulseNoise:
					dstPtr += SPrintF(dstPtr, "DSL recovered from short interruption\n");
					break;
				case	kG994p1EventToneDetected:
					dstPtr += SPrintF(dstPtr, "DSL G.992.2 Fast Retrain G.994.1 Tone detected\n");
					break;
				case	kDslG992p2TxShowtimeActive:
					dstPtr += SPrintF(dstPtr, "DSL G.992 transmit SHOWTIME active, txDataRate = %d kbps\n", value);
#ifdef STATUSPARSER_STATE
					parserState[lineId].trainingState = kAdslTrainingConnected;
#endif
					break;
				case	kDslG992p2RxShowtimeActive:
					dstPtr += SPrintF(dstPtr, "DSL G.992 receive SHOWTIME active, rxDataRate = %d kbps\n", value);
					headerCounter=0;
					break;
#if 1
				case	kDslG992p2TxAocMessage:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Transmitting Aoc Message: %d\n", value);
					break;
				case	kDslG992p2TxEocMessage:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Transmitting Eoc Message: %04x %02x '%c%c%c'\n",
						(value & 0x0FFFF), (value & 0x0FFFF) >> 5,
						value & 0x4 ? 'O' : 'D',
						value & 0x8 ? '1' : 'E',
						value & 0x10 ? 'C' : 'A');
					break;
				case	kDslG992p2RxAocMessage:
					dstPtr += SPrintF(dstPtr, "DSL G.992 receive Aoc Message: %d\n", value);
					break;
				case	kDslG992p2RxEocMessage:
					dstPtr += SPrintF(dstPtr, "DSL G.992 receive Eoc Message: %04x %02x '%c%c%c'\n",
						(value & 0x0FFFF), (value & 0x0FFFF) >> 5,
						value & 0x4 ? 'O' : 'D',
						value & 0x8 ? '1' : 'E',
						value & 0x10 ? 'C' : 'A');
					break;
#endif
				case	kDslG992p2RcvVerifiedBitAndGain:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv SNR Margin : %s dB\n", Q4ToFloatString(value));
					break;
#ifdef G992P2_PROFILE
				case	kDslG992p2ProfileChannelResponseCalc:
					dstPtr += SPrintF(dstPtr, "DSL G.992.2 Profile Channel Calc: Weighted Error: %d\n", value);
					break;
#endif
				case kDslG992Timeout:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Timed out\n");
					break;
				case kDslG992XmtRReverbRAOver4000:
					dstPtr += SPrintF(dstPtr, "DSL G.992 xmt R-Reverb-RA over 4000 symbols\n");
					break;
				case kDslG992XmtRReverb5Over4000:
					dstPtr += SPrintF(dstPtr, "DSL G.992 xmt R-Reverb5 over 4000 symbols\n");
					break;
				case kDslG992RcvCSegue2Failed:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv fail to detect C-Segue2\n");
					break;
				case kDslG992RcvCSegueRAFailed:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv fail to detect C-Segue-RA\n");
					break;
				case kDslG992RcvCSegue3Failed:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv fail to detect C-Segue3\n");
					break;
				case kDslG992RcvShowtimeStartedTooLate:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv late start of Showtime\n");
					break;
				case kDslG992XmtRReverb3Over4000:
					dstPtr += SPrintF(dstPtr, "DSL G.992 xmt R-Reverb3 over 4000 symbols\n");
					break;
				case kDslG992RcvFailDetCSegue1InWindow:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv fail to detect C-Segue1 within window\n");
					break;
				case kDslG992RcvCPilot1Failed:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv fail to detect C-Pilot1\n");
					break;
				case kDslG992RcvCReverb1Failed:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv fail to detect C-Reverb1\n");
					break;
				case kG992ControlAllRateOptionsFailedErr:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv all rate options are failed\n");
					break;
				case kG992ControlInvalidRateOptionErr:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv invalid rate option\n");
					break;
				case kDslG992XmtInvalidXmtDErr:
					dstPtr += SPrintF(dstPtr, "DSL G.992 xmt interleave depth larger than supported\n");
					break;
				case kDslG992BitAndGainCalcFailed:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv Bit and Gain calculation is failed\n");
					break;
				case kDslG992BitAndGainVerifyFailed:
					dstPtr += SPrintF(dstPtr, "DSL G.992 rcv Bit and Gain verification is failed\n");
					break;

                case kRetrainReasonConfigNotFeasibleUs:
                    dstPtr += SPrintF(dstPtr, "DSL US Configuration NOT Feasible Err from CO\n");
                    break;

                case kRetrainReasonConfigNotFeasibleDs:
                    dstPtr += SPrintF(dstPtr, "DSL DS Configuration NOT Feasible\n");
                    break;


#ifdef G992_ANNEXC
				case kDslG992AnnexCTimeoutCPilot1Detection:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Annex C TimeOut CPilot1 Detection\n");
					break;
				case kDslG992AnnexCTimeoutCReverb1Detection:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Annex C TimeOut CReverb1 Detection\n");
					break;
				case kDslG992AnnexCTimeoutECTraining:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Annex C TimeOut Echo Cancellor\n");
					break;
				case kDslG992AnnexCTimeoutHyperframeDetector:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Annex C TimeOut HyperframeDetector\n");
					break;
				case kDslG992AnnexCTimeoutSendRSegue2:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Annex C TimeOut Send R Segue2\n");
					break;
				case kDslG992AnnexCTimeoutDetectCSegue1:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Annex C TimeOut CSegue1 Detection\n");
					break;
				case kDslG992AnnexCAlignmentErrDetected:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Annex C TimeOut Alignment Error\n");
					break;
				case kDslG992AnnexCTimeoutSendRSegueRA:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Annex C TimeOut Send RSegueRA\n");
					break;
				case kDslG992AnnexCTimeoutSendRSegue4:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Annex C TimeOut Send RSegue4\n");
					break;
				case kDslG992AnnexCTimeoutCSegue2Detection:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Annex C TimeOut CSegue2 Detection\n");
					break;
				case kDslG992AnnexCTimeoutCSegue3Detection:
					dstPtr += SPrintF(dstPtr, "DSL G.992 Annex C TimeOut CSegue 3 Detection\n");
					break;
#endif
#ifdef G992P3
				case kDslG992p3ReceivedMsgFmt:
					dstPtr += SPrintF(dstPtr, "DSL G.992P3 received message MSG-FMT\n");
					break;
				case kDslG992p3ReceivedMsgPcb:
					dstPtr += SPrintF(dstPtr, "DSL G.992P3 received message MSG-PCB\n");
					break;
#endif
				case	kDslT1p413Isu1SglByteSymDetected:
					dstPtr += SPrintF(dstPtr, "DSL G.992 detected T1.413 Issue 1 type single byte symbol mode\n");
					break;
				case	kDslG992RxPrefixOnInAFewSymbols:
					dstPtr += SPrintF(dstPtr, "DSL G.992 receive prefix will be on in %d symbols\n", value);
					break;
				case	kDslG992TxPrefixOnInAFewSymbols:
					dstPtr += SPrintF(dstPtr, "DSL G.992 transmit prefix will be on in %d symbols\n", value);
					break;
				case 	kDslT1p413DetectedCTone:
					dstPtr += SPrintF(dstPtr, "T1.413 Detected CTone\n");
					break;
				case 	kDslT1p413DetectedCAct:
					dstPtr += SPrintF(dstPtr, "T1.413 Detected CAct\n");
					break;
				case	kDslT1p413DetectedCReveille:
					dstPtr += SPrintF(dstPtr, "T1.413 Detected CReveille\n");
					break;
				case	kDslT1p413DetectedRActReq:
					dstPtr += SPrintF(dstPtr, "T1.413 Detected RActReq\n");
					break;
				case	kDslT1p413DetectedRQuiet1:
					dstPtr += SPrintF(dstPtr, "T1.413 Detected RQuiet1\n");
					break;
				case	kDslT1p413DetectedRAct:
					dstPtr += SPrintF(dstPtr, "T1.413 Detected RAct\n");
					break;
				case	kDslFinishedT1p413:
					dstPtr += SPrintF(dstPtr, "T1.413 Finished\n");
#ifdef STATUSPARSER_STATE
					parserState[lineId].trainingState = kAdslTrainingG992Started;
#endif
					break;
				case	kDslT1p413TimeoutCReveille:
					dstPtr += SPrintF(dstPtr, "T1.413 Timeout CReveille\n");
					break;
				case	kDslAnnexCXmtCPilot1Starting:
					dstPtr += SPrintF(dstPtr, "DSL G.992 AnnexC Transmitting CPilot1\n");
					break;
				case	kDslXmtToRcvPathDelay:
					dstPtr += SPrintF(dstPtr, "DSL transceiver total delay (xmt plus rcv): %d\n", value);
					break;
				case	kDslG992RcvMsgCrcError:
					dstPtr += SPrintF(dstPtr, "DSL G.992 receive message CRC error\n");
					break;
#ifndef LITE_VERSION
				case	kDslT1p413RetrainToUseCorrectRAck:
					dstPtr += SPrintF(dstPtr, "DSL retrain forced to use the correct RACK\n");
					break;
				case	kDslT1p413RetrainToUseCorrectIFFT:
					dstPtr += SPrintF(dstPtr, "DSL retrain forced to use the correct IFFT size\n");
					break;
#endif
				case	kG992Upstream2xIfftDisabled:
					dstPtr += SPrintF(dstPtr, "DSL upstream 2x IFFT disabled\n");
					break;
				case	kDslAnnexCDetectedStartHyperframe:
					dstPtr += SPrintF(dstPtr, "DSL G.992 detected start of hyperframe\n");
					break;
				case    kDslG992AnnexCTotalFEXTBits:
					dstPtr += SPrintF(dstPtr, "DSL Total FEXT Bits per symbol : %d\n" , value);
					break;
				case    kDslG992AnnexCTotalNEXTBits:
					dstPtr += SPrintF(dstPtr, "DSL Total NEXT Bits per symbol : %d\n" , value);
					break;
				case	kG992DataRcvDetectLOS:
					dstPtr += SPrintF(dstPtr, "Loss Of Signal(LOS) detected\n");
					break;
				case	kG992DataRcvDetectLOSRecovery:
					dstPtr += SPrintF(dstPtr, "(LOS) Signal recovered\n");
					break;
				case	kG992DecoderDetectRemoteLOS:
				case	kG992DataRcvDetectFeLOS:
					dstPtr += SPrintF(dstPtr, "Far End Loss Of Signal(LOS) detected\n");
					break;
				case	kG992DecoderDetectRemoteLOSRecovery:
				case	kG992DataRcvDetectFeLOSRecovery:
					dstPtr += SPrintF(dstPtr, "Far End (LOS) recovered\n");
					break;
				case	kG992DecoderDetectRDI:
					dstPtr += SPrintF(dstPtr, "Severely Errored Frame (RDI) detected\n");
					break;
				case	kG992DecoderDetectRDIRecovery:
					dstPtr += SPrintF(dstPtr, "(RDI) recovered\n");
					break;
				case	kG992DecoderDetectRemoteRDI:
					dstPtr += SPrintF(dstPtr, "Far End (RDI) detected\n");
					break;
				case	kG992DecoderDetectRemoteRDIRecovery:
					dstPtr += SPrintF(dstPtr, "Far End (RDI) recovered\n");
					break;
#if defined(G992P5)
				case	kDslG992RunAnnexaP3ModeInAnnexaP5:
					dstPtr += SPrintF(dstPtr, "P3 Mode In AnnexA P5\n");
					break;
#endif
				case	kG992EnableAnnexM:
					dstPtr += SPrintF(dstPtr, "AnnexM enabled, EU-%d\n",32+(value<<2));
                    			break;
#ifndef LITE_VERSION
				case kDslAtuChangeTxFilterRequest:
					dstPtr += SPrintF(dstPtr, "Tx Filter changed to filter Id %d\n", value);
					break;
				case kDslMarkerCheckFailed:
				case kDslMarkerCheckRecovered:
					dstPtr += SPrintF(dstPtr, "********************************** Marker checking %s at %d\n",
						(kDslMarkerCheckFailed == code) ? "failed" : "recovered", status->param.dslTrainingInfo.value);
					break;
				case kDslRcvBufferOverflowDetected:
				case kDslRcvBufferOverflowRecovered:
					dstPtr += SPrintF(dstPtr, "******************* sampleBufferRcv overflow %s, val=%d",
						(kDslRcvBufferOverflowDetected == code) ? "detected" : "recovered", status->param.dslTrainingInfo.value);
					break;
				case kDslXmtBufferOverflowDetected:
				case kDslXmtBufferOverflowRecovered:
					dstPtr += SPrintF(dstPtr, "******************* sampleBufferXmt overflow %s, val=%d",
						(kDslXmtBufferOverflowDetected == code) ? "detected" : "recovered", status->param.dslTrainingInfo.value);
					break;
				case kDslXmtBufferUnderflowDetected:
				case kDslXmtBufferUnderflowRecovered:
					dstPtr += SPrintF(dstPtr, "******************* sampleBufferXmt underflow %s, val=%d",
						(kDslXmtBufferUnderflowDetected == code) ? "detected" : "recovered", status->param.dslTrainingInfo.value);
					break;
#endif
				case  kG992LDStartMode:
					if (status->param.dslTrainingInfo.value)
						dstPtr += SPrintF(dstPtr, "LD mode started\n");
					else
						dstPtr += SPrintF(dstPtr, "LD: normal training\n");
					break;
				case  kG992LDCompleted:
					if (status->param.dslTrainingInfo.value)
						dstPtr += SPrintF(dstPtr, "ADSL2 Diagnostic Mode is successfully completed!\n");
					else
						dstPtr += SPrintF(dstPtr, "Diagnostic Mode failed\n");
					break;
				case  kG992LDLastStateDs:
					dstPtr += SPrintF(dstPtr, "Previous init last transmitted state Ds=%d\n", value);
					break;
				case  kG992LDLastStateUs:
					dstPtr += SPrintF(dstPtr, "Previous init last transmitted state Us=%d\n", value);
					break;

        case kG992SetPLNMessageBase:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "PLNMessageBase set to 0x%X\n", value);
#endif
					break;

        case kDslAfeSampleLoss:
#ifndef LITE_VERSION
                    dstPtr += SPrintF(dstPtr, "AFE sample loss");
#endif
                    break;

        case kG992PlnBroadbandCounterReset:
#ifndef LITE_VERSION
                    dstPtr += SPrintF(dstPtr, "Broadband Counter Reset");
#endif

                    break;
        case kDslRetrainReason:
#ifndef LITE_VERSION
					if(!((value>>kRetrainReasonDslStartPhysicalLayerCmd)&0x1))
						dstPtr += SPrintF(dstPtr, "CPE Retrain: reason code = 0x%x\n", value);
                    if (value) {
                        if ((value>>kRetrainReasonLosDetector             )&0x1) dstPtr += SPrintF(dstPtr, "\tLOS Detector High\n");
                        if ((value>>kRetrainReasonRdiDetector             )&0x1) dstPtr += SPrintF(dstPtr, "\tRDI Detector High\n");
                        if ((value>>kRetrainReasonNegativeMargin          )&0x1) dstPtr += SPrintF(dstPtr, "\tNegative Margin for too long\n");
                        if ((value>>kRetrainReasonTooManyUsFEC            )&0x1) dstPtr += SPrintF(dstPtr, "\tToo Many US FEC\n");
                        if ((value>>kRetrainReasonCReverb1Misdetection    )&0x1) dstPtr += SPrintF(dstPtr, "\tC-Reverb1 misdetection\n");
                        if ((value>>kRetrainReasonTeqDsp                  )&0x1) dstPtr += SPrintF(dstPtr, "\tTEQ DSP exception\n");
                        if ((value>>kRetrainReasonAnsiTonePowerChange     )&0x1) dstPtr += SPrintF(dstPtr, "\tANSI Tone power change needed\n");
                        if ((value>>kRetrainReasonIfftSizeChange          )&0x1) dstPtr += SPrintF(dstPtr, "\tIFFT size change needed\n");
                        if ((value>>kRetrainReasonRackChange              )&0x1) dstPtr += SPrintF(dstPtr, "\tR-ACK type change needed\n");
                        if ((value>>kRetrainReasonVendorIdSync            )&0x1) dstPtr += SPrintF(dstPtr, "\tVendor Id sync needed\n");
                        if ((value>>kRetrainReasonTargetMarginSync        )&0x1) dstPtr += SPrintF(dstPtr, "\tTarget Margin Sync needed\n");
                        if ((value>>kRetrainReasonToneOrderingException   )&0x1) dstPtr += SPrintF(dstPtr, "\tTone ordering exception\n");
                        if ((value>>kRetrainReasonCommandHandler          )&0x1) dstPtr += SPrintF(dstPtr, "\tCommand Handler Request\n");
                        if ((value>>kRetrainReasonDslStartPhysicalLayerCmd)&0x1) dstPtr += SPrintF(dstPtr, "\tStart/Reset PHY Command received\n");
                        if ((value>>kRetrainReasonUnknown                 )&0x1) dstPtr += SPrintF(dstPtr, "\tReason unknown\n");
                        if ((value>>kRetrainReasonTrainingFailure           )&0x1) dstPtr += SPrintF(dstPtr, "\tTraining Failure\n");
                        if ((value>>kRetrainReasonSes                     )&0x1) dstPtr += SPrintF(dstPtr, "\tToo many SES\n");
                        if ((value>>kRetrainReasonCoMinMargin             )&0x1) dstPtr += SPrintF(dstPtr, "\tMargin under CO configured minimum margin\n");

                        if ((value>>kRetrainReasonConfigError               )&0x1) dstPtr += SPrintF(dstPtr, "\tConfiguration Error\n");
                        if ((value>>kRetrainReasonTimeout                   )&0x1) dstPtr += SPrintF(dstPtr, "\tTimeout Error\n");
                        if ((value>>kRetrainReasonNoATUC                    )&0x1) dstPtr += SPrintF(dstPtr, "\tNo ATUC found\n");
                        if ((value>>kRetrainReasonNoCommonOPMode            )&0x1) dstPtr += SPrintF(dstPtr, "\tNo Common Mode\n");
                        if ((value>>kRetrainReasonNoCommonTPSTC             )&0x1) dstPtr += SPrintF(dstPtr, "\tNO Common TPSTC Found\n");
                        if ((value>>kRetrainReasonShowtimeFailure           )&0x1) dstPtr += SPrintF(dstPtr, "\tShowtime Failure\n");
                    }/* if (value )*/
#endif
          break;
/****************************************************************************/
/*	                VDSL training phase Rx progress												  */
/****************************************************************************/
#ifndef LITE_VERSION
        case kG993p2RcvMeasureQuiet1:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Measure Quiet \n");
          break;
        case kG993p2RcvHuntStart1:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Hunt Start \n");
          break;
        case kG993p2RcvAgcMeasure1:
        case kG993p2RcvAgcMeasure2:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Agc Measure \n");
          break;
        case kG993p2RcvInitPll1:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Pll Init \n");
          break;
        case kG993p2RcvFindCoarseAlignment1:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Alignment Coarse \n");
          break;
        case kG993p2RcvExecuteMove1:
        case kG993p2RcvExecuteMove2:
        case kG993p2RcvExecuteMove3:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Symbol Move \n");
          break;
        case kG993p2RcvFindFineAlignment1:
        case kG993p2RcvFindFineAlignment2:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Alignment Fine \n");
          break;
        case kG993p2RcvRelockPll1:
        case kG993p2RcvRelockPll2:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Pll Relock \n");
          break;
        case kG993p2RcvFeqInstallation1:
        case kG993p2RcvFeqInstallation2:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Feq Install \n");
          break;
        case kG993p2RcvFindBestTones1:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Select Pilot and Message Tones \n");
          break;
#endif
        case kG993p2RcvWaitOsignature:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Wait O-Signature \n");
          break;
#ifndef LITE_VERSION
        case kG993p2RcvCalcUsPbo:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Calculate US Power Back-Off\n");
          break;
        case kG993p2RcvFeqLms11:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Feq LMS Update \n");
          break;
        case kG993p2RcvMeasureSnr1:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - SNR Measure \n");
          break;
        case kG993p2RcvSendRmsg1:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Send Rmsg1 \n");
          break;
#endif
        case kG993p2RcvDetectSynchro1:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Wait Synchro 1  \n");
          break;
#ifndef LITE_VERSION
        case kG993p2RcvLineprobe:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Line Probe \n");
          break;
        case kG993p2RcvToneSelectAndPbo:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Tone Selec and Apply Power Back-Off \n");
          break;
#endif
        case kG993p2RcvWaitOupdate:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Wait Oupdate \n");
          break;
#ifndef LITE_VERSION
        case kG993p2RcvBpOptimization:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Band Plan Optimization \n");
          break;
#endif
        case kG993p2RcvWaitOprms:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Wait Oprms \n");
          break;
        case kG993p2RcvWaitOack:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Wait Oack \n");
          break;
        case kG993p2RcvDetectSynchro3:
          dstPtr += SPrintF(dstPtr, "Rx State: Discovery - Wait Synchro 3 \n");
          break;

        /* Transceiver training  */

#ifndef LITE_VERSION
        case kG993p2RcvHuntStart2:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Hunt Start \n");
          break;
        case kG993p2RcvAgcMeasure3:
        case kG993p2RcvAgcMeasure4:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Agc Measure \n");
          break;
        case kG993p2RcvFindCoarseAlignment2:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Alignment Coarse \n");
          break;
        case kG993p2RcvExecuteMove4:
        case kG993p2RcvExecuteMove5:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Symbol Move \n");
          break;
        case kG993p2RcvRelockPll3:
        case kG993p2RcvRelockPll4:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Pll Relock \n");
          break;
        case kG993p2RcvFindFineAlignment3:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Alignment Fine \n");
          break;
        case kG993p2RcvTxTrainingSignal:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Tx Training Start \n");
          break;
        case kG993p2RcvFeqInstallation3:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Feq Install \n");
          break;
        case kG993p2RcvFindBestTones2:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Select Pilot and Message Tones \n");
          break;
#endif
        case kG993p2RcvDetectSynchro4:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Wait Synchro 4 \n");
          break;
#ifndef LITE_VERSION
        case kG993p2RcvComputeRta:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Compute Timing Advance \n");
          break;
#endif
        case kG993p2RcvWaitOTaUpdate:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Wait OtaUpdate \n");
          break;
        case kG993p2RcvWaitRTaUpdateAck:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Wait TaUpdateAck \n");
          break;
        case kG993p2RcvDetectSynchro5:
          dstPtr += SPrintF(dstPtr, "Rx State: Training - Wait Synchro 5 \n");
          break;

        /* anaExch */
#ifndef LITE_VERSION
        case kG993p2RcvFeqLms12:
        case kG993p2RcvFeqLms13:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - Feq LMS Update \n");
          break;

        case kG993p2RcvMeasureSnr2:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - SNR Measure \n");
          break;
        /* loop diagnostic actions */
        case kG993p2LdCalcHlin:
          dstPtr += SPrintF(dstPtr, "Rx State: Loop Diagnostic - Calculate Hlin \n");
          break;
#endif
        case kG993p2LdDetectSynchro6:
          dstPtr += SPrintF(dstPtr, "Rx State: Loop Diagnostic - Wait Synchro LD 1 \n");
          break;
        case kG993p2LdWaitOmsgLd:
          break;
          dstPtr += SPrintF(dstPtr, "Rx State: Loop Diagnostic - Wait OmsgLD \n");
          break;
        case kG993p2LdWaitOmsgLdEnd:
          dstPtr += SPrintF(dstPtr, "Rx State: Loop Diagnostic - Wait OmsgLD End \n");
          break;
        case kG993p2LdDetectSynchro7:
          dstPtr += SPrintF(dstPtr, "Rx State: Loop Diagnostic - Wait Synchro LD 2\n");
          break;
        case kG993p2LdComplete:
          dstPtr += SPrintF(dstPtr, "Rx State: Loop Diagnostic - Wait End LD \n");
          break;

        /* anaExch - cont. */
        case kG993p2RcvWaitOmsg1:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - Wait Omsg1 \n");
          break;
#ifndef LITE_VERSION
        case kG993p2RcvComputeBmax:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - Compute Max Capacity \n");
          break;
#endif
        case kG993p2RcvWaitOtps:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - Wait Otps \n");
          break;
        case kG993p2RcvWaitOpms:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - Wait Opms \n");
          break;
#ifndef LITE_VERSION
        case kG993p2RcvPrepDsRateSelect:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - Compute DS Rate Select \n");
          break;
        case kG993p2RcvComputeDsBiGi:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - Compute DS BiGi \n");
          break;
#endif
        case kG993p2RcvWaitOpmd:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - Wait Opmd \n");
          break;

#ifndef LITE_VERSION
        case kG993p2RcvParseOpmd:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - Parse Opmd \n");
          break;
#endif
        case kG993p2RcvTxRpmdComplete:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - Wait Tx Rpmd Complete \n");
          break;

        case kG993p2ScaleFeqComplete:
          dstPtr += SPrintF(dstPtr, "Rx State: Analysis & Exchange - Wait ShowTime FEQ Scaling Complete \n");
          break;

        case kDsl993p2Profile:
          dstPtr += SPrintF(dstPtr, "G993.2 Profile 0x%X\n", value);
          break;
        case kDsl993p2Annex:
          dstPtr += SPrintF(dstPtr, "G993.2 Annex 0x%X\n", value);
          break;

        case kDsl993p2US0mask:
          dstPtr += SPrintF(dstPtr, "G993.2 US0 Mask 0x%X\n", value);
          break;

		case kDslG992RxLatencyPathCount:
			dstPtr += SPrintF(dstPtr, "DSL Total number of RX latency paths: %d \n", value);
#ifdef STATUSPARSER_STATE			
			parserState[lineId].NLp[0]= (char)value;
            parserState[lineId].NLpValid = 1;
#endif
			
#if defined(WINNT) || defined(LINUX_DRIVER)
			DslDiagSetDualLatencyStatus((2==value), 0);
#endif
			break;
		case kDslG992TxLatencyPathCount:
			dstPtr += SPrintF(dstPtr, "DSL Total number of TX latency paths: %d \n", value);
#ifdef STATUSPARSER_STATE			
			parserState[lineId].NLp[1]= (char)value;
            parserState[lineId].NLpValid = 2;

#endif

#if defined(WINNT) || defined(LINUX_DRIVER)
			DslDiagSetDualLatencyStatus((2==value), 1);
#endif
			break;
		case kDslG992LatencyPathId:
			dstPtr += SPrintF(dstPtr, "DSL Latency PathId: %d \n", value);
#if defined(WINNT) || defined(LINUX_DRIVER)
			if(1 == value)
				DslDiagSetDualLatencyStatus(1, 0);
#endif
			break;


/****************************************************************************/
/*	                VDSL training Signal Tx Types												    */
/****************************************************************************/
          /* Channel Discovery */
        case kG993p2TxQuiet1:
        case kG993p2TxQuiet2:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Discovery - Quiet  \n");
          break;
        case kG993p2TxChDiscovery1:
        case kG993p2TxChDiscovery2:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Discovery - Channel Discovery  \n");
          break;
        case kG993p2TxSynchro1:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Discovery - Synchro 1  \n");
          break;
        case kG993p2TxLineProbe:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Discovery - Line Probe  \n");
          break;
        case kG993p2TxPeriodic1:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Discovery - Periodic  \n");
          break;
        case kG993p2TxSynchro2:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Discovery - Synchro 2  \n");
          break;
        case kG993p2TxSynchro3:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Discovery - Synchro 3  \n");
          break;

        /* Transceiver training */
        case kG993p2TxQuiet3:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Training - Quiet  \n");
          break;
        case kG993p2TxTrainingRandom1:
        case kG993p2TxTrainingRandom2:
        case kG993p2TxTrainingRandom3:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Training - Training Random  \n");
          break;
        case kG993p2TxSynchro4:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Training - Synchro 4  \n");
          break;
        case kG993p2TxTeq:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Training - Teq  \n");
          break;
        case kG993p2TxQuiet4:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Training - Quiet  \n");
          break;
        case kG993p2TxEct:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Training - Ect  \n");
          break;
        case kG993p2TxPeriodic2:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Training - Periodic  \n");
          break;
        case kG993p2TxSynchro5:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Training - Synchro 5  \n");
          break;

        /* anaExch */
        case kG993p2TxMedley:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Analysis & Exchange - Medley  \n");
          break;
        case kG993p2TxSynchro6:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Loop Diagnostic - Synchro 6  \n");
          break;
        case kG993p2TxChDiscovery3:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Loop Diagnostic - Channel Discovery  \n");
          break;
        case kG993p2TxSynchro7:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Loop Diagnostic - Synchro 7  \n");
          break;
        case kG993p2TxQuiet5:
          dstPtr += SPrintF(dstPtr, "Tx Signal: Loop Diagnostic - Quiet  \n");
          break;

/****************************************************************************/
/*	                End of VDSL training 										                */
/****************************************************************************/

				default:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "DSL training status, code = %d, value = %d\n",
						code, value);
#endif
					break;
				}
			}
			break;
		case kDslDspControlStatus:
			{
			dslConnectInfoStatusCode	code = status->param.dslConnectInfo.code;
			int						value = status->param.dslConnectInfo.value;
#ifdef XDSLDRV_ENABLE_PARSER
			void *pBuffPtrOrig = status->param.dslConnectInfo.buffPtr;
			status->param.dslConnectInfo.buffPtr = parseBufTmp;
#endif
			switch(code)
				{
				case	kDslATURClockErrorInfo:
					dstPtr += SPrintF(dstPtr, "DSL transceiver clock error : %s ppm\n", Q8ToFloatString(value));
					break;
				case	 kDslHWSetDigitalEcGainShift:
					dstPtr += SPrintF(dstPtr, "DSL transceiver setup DigitalECGainShift : %d\n", (short)value);
					break;
#ifdef BCM6348_SRC
				case kDslHWDisableDigitalECUpdate:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "DSL disable digital EC update: %d\n", value);
#endif
					break;
#endif
				case kDslHWEnableDigitalECUpdate:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "DSL enable digital EC update: %d\n", value);
#endif
					break;
				case	kDslHWdcOffsetInfo:
					dstPtr += SPrintF(dstPtr, "DSL rcv estimated DC offset during G.992 :	%d\n", (int)value);
					break;
				case    kDslHWTimeTrackingClockTweak:
#ifdef HARDWARE_TEQ_OUTPUT_RECORD
					dstPtr += SPrintF(dstPtr, "DSL Time Tracking Clock Tweak = %d\n", value);
#endif
					break;
				case kDslHWSetDigitalEcUpdateMode:
				case kDslHWEnableDigitalEC:
					break;
				case kDslHWSetDigitalEcUpdateShift:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "DSL set digital EC update shift: %d\n", value);
#endif
					break;
				case kDslPLNPeakNoiseTablePtr:
					dstPtr += PrintUShortBufferData(dstPtr, "ImpNoise MaxPower Per Tone:",
						(int)status->param.dslConnectInfo.value >> 1,
						(short*)status->param.dslConnectInfo.buffPtr);
					break;
				case kDslPerBinThldViolationTablePtr:
					dstPtr += PrintUShortBufferData(dstPtr, "ImpNoise Threshold Violations Per Tone:",
						(int)status->param.dslConnectInfo.value >> 1,
						(short*)status->param.dslConnectInfo.buffPtr);
					break;
				case kDslImpulseNoiseDurationTablePtr:
					dstPtr += PrintUShortBufferData(dstPtr, "ImpNoise Duration Histogram:",
						(int)status->param.dslConnectInfo.value >> 1,
						(short*)status->param.dslConnectInfo.buffPtr);
					break;
				case kDslImpulseNoiseTimeTablePtr:
					dstPtr += PrintUShortBufferData(dstPtr, "ImpNoise Inter-Arrival Time Histogram:",
						(int)status->param.dslConnectInfo.value >> 1,
						(short*)status->param.dslConnectInfo.buffPtr);
					break;
				case kDslPLNMarginPerBin:
#ifndef MIPS_SRC
					dstPtr += SPrintF(dstPtr, "PLN margin per bin = 0x%04X (%4.2f dB)\n", value, 10*LOG10((double)value/4096));
#endif
					break;
				case kDslPLNMarginBroadband:
#ifndef MIPS_SRC
					dstPtr += SPrintF(dstPtr, "PLN margin band = 0x%04X (%4.2f dB)\n", value, 10*LOG10((double)value/4096));
#endif
					break;
				case kDslPerBinMsrCounter:
					dstPtr += SPrintF(dstPtr, "PLN per bin measurement counter = %u\n", value);
					break;
				case kDslBroadbandMsrCounter:
					dstPtr += SPrintF(dstPtr, "PLN broadband measurement counter  = %u\n", value);
					break;
				case kDslPlnState:
					dstPtr += SPrintF(dstPtr, "PLN running state : %d ", value);
					break;
				case kDslInpBinTablePtr:
					dstPtr += PrintUShortBufferData(dstPtr, "ImpNoise Duration bin table:",
						(int)status->param.dslConnectInfo.value >> 1,
						(short*)status->param.dslConnectInfo.buffPtr);
					break;
				case kDslItaBinTablePtr:
					dstPtr += PrintUShortBufferData(dstPtr, "ImpNoise Inter-Arrival bin table:",
						(int)status->param.dslConnectInfo.value >> 1,
						(short*)status->param.dslConnectInfo.buffPtr);
					break;

				case kDslNLNoise:
					dstPtr += PrintQ4dBBufferData(dstPtr, "Non-Linearity noise table:",
						(int)status->param.dslConnectInfo.value >> 1,
						(ushort*)status->param.dslConnectInfo.buffPtr);
					break;
				case kDslInitializationSNRMarginInfo:
					dstPtr += PrintQ4dBBufferData(dstPtr, "Initialization SNR margin info:",
						(int)status->param.dslConnectInfo.value >> 1,
						(ushort*)status->param.dslConnectInfo.buffPtr);
					break;

				case kDslStatusBufferInfo:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "Next status data: sz=%d ptr=0x%px\n", value, status->param.dslConnectInfo.buffPtr);
#endif
					break;
				case kDslTxOvhMsg:
				case kDslRxOvhMsg:
				{
#ifndef LITE_VERSION
					int prio, msgNum, rspOrCmd;
#ifdef XDSLDRV_ENABLE_PARSER
					unsigned char *pData = (unsigned char *)pBuffPtrOrig;
#else
					unsigned char *pData = (unsigned char *)status->param.dslConnectInfo.buffPtr;
#endif
#ifdef STATUSPARSER_STATE
					if((kXdslModGfast == parserState[lineId].modType) || (pData[0] > 2)) {
						prio = pData[1] & 3;
						rspOrCmd = (pData[1] & 0xC);
						msgNum = (((pData[0] & 0x3F) << 4) | ((pData[1] >> 4) & 0xF)) + 1;	// Length
					}
					else
#endif
					{
						prio = pData[0] & 3;
						rspOrCmd = (pData[1] & 2);
						msgNum= pData[1] & 1;
					}
					dstPtr += SPrintF(dstPtr, "G.997 frame %s:  len = %d  ( PRI%d %s %d ) data:\r\n",
						(kDslTxOvhMsg == code) ? "TX" : "RX", value, prio, (rspOrCmd) ? "RSP" : "CMD", msgNum);
					dstPtr += PrintBufferDataHex(dstPtr, 0, value, pData);
#endif
					break;
				}
#ifdef G992
				case kDslG992RcvShowtimeUpdateGainPtr:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "RCV gain table len=%d\t", (int)value);
					dstPtr += PrintQ4dBBufferData(dstPtr, "RCV gain table:",
						(int)value >> 1, (ushort*)status->param.dslConnectInfo.buffPtr);
#endif
					break;
#endif
				case kFireMonitoringCounters:
				{
					unsigned int *pLval = (unsigned int *)status->param.dslConnectInfo.buffPtr;
					dstPtr += SPrintF(dstPtr, "Fire reXmt = %u\treXmt Corr=%u  Unc = %u\n",
						pLval[kFireReXmtRSCodewordsRcved], pLval[kFireReXmtCorrectedRSCodewords], pLval[kFireReXmtUncorrectedRSCodewords]);
					break;
				}
				default:
					dstPtr += SPrintF(dstPtr, "DSP control status, code = %d, value = %d, bufPtr = 0x%px\n",
						code, value, status->param.dslConnectInfo.buffPtr);
				}
#ifdef XDSLDRV_ENABLE_PARSER
			status->param.dslConnectInfo.buffPtr = pBuffPtrOrig;
#endif
			}
			break;
		case kDslConnectInfoStatus:
			{
			dslConnectInfoStatusCode	code  = status->param.dslConnectInfo.code;
			int				value = status->param.dslConnectInfo.value;
			
			switch (code)
				{
#ifdef G994P1
				case	kG994VendorID:
					dstPtr += SPrintF(dstPtr, "G.994.1 received vendor ID : DataNotAvailable => kDslConnectInfoStatus-kG994VendorID\n");
                                break;
				case	kG994MessageExchangeRcvInfo:
					{
					unsigned char	*msgPtr = ((unsigned char*)status->param.dslConnectInfo.buffPtr);
					dstPtr += PrintByteBufferData(dstPtr, "G.994 rcv message:", (int)value, msgPtr);
					if ((msgPtr != NULL) && ((msgPtr[0] == 2) || (msgPtr[0] == 3)))	/* CL or CLR message */
						{
                                                /*********************************************************************************************/
                                                /* Log the Vendor ID based on the current segment                                            */
                                                /* Wrong logging as the information is wrong if this is not the first segment of the message */
                                                /* Code kept in order to still get the logging with old PHY code                             */
                                                /*********************************************************************************************/
						//dstPtr += SPrintF(dstPtr, "\r\n");
						dstPtr += SPrintF(dstPtr, "G.994.1 received vendor ID (on segment): %c%c%c%c\n",
							PrintVendorIDChar(msgPtr[4]),
							PrintVendorIDChar(msgPtr[5]),
							PrintVendorIDChar(msgPtr[6]),
							PrintVendorIDChar(msgPtr[7]));
						}
					}
					break;
				case	kG994MessageExchangeXmtInfo:
					dstPtr += PrintByteBufferData(dstPtr, "G.994 xmt message:",
						(int)value, (unsigned char*)status->param.dslConnectInfo.buffPtr);
					break;
				case	kG994SelectedG994p1CarrierIndex:
					dstPtr += SPrintF(dstPtr, "G.994 Selected Carrier Index:   %d\n", (int) value);
					break;
#endif /* G994P1 */
#ifdef G992
				case	kG992p2XmtToneOrderingInfo:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "XMT bit loading table len=%d\t", (int) value);

					dstPtr += PrintByteBufferData(dstPtr, "XMT bit loading table:",
						(int) value, (uchar*)status->param.dslConnectInfo.buffPtr);
#endif
					break;
				case	kG992p2RcvToneOrderingInfo:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "RCV bit loading table len=%d\t", (int) value);

					dstPtr += PrintByteBufferData(dstPtr, "RCV bit loading table:",
						(int) value, (uchar*)status->param.dslConnectInfo.buffPtr);
#endif
					break;
				case kDslSetPilotEyeDisplay:
					dstPtr += SPrintF(dstPtr, "DSL pilot eye display monitor tone : %d\n", (int)value);
					break;
				case	kG992p2XmtCodingParamsInfo:
					dstPtr += SPrintF(dstPtr, "G.992 xmt coding parameters :");
					goto PRINT_G992_CODING_PARAMS;
				case	kG992p2RcvCodingParamsInfo:
					dstPtr += SPrintF(dstPtr, "G.992 rcv coding parameters :");
PRINT_G992_CODING_PARAMS:
					{
					char				*dstPtr1;
					G992CodingParams	*codingParam;
					dstPtr1 = dstPtr + strlen(dstPtr);
#ifdef XDSLDRV_ENABLE_PARSER
					codingParam = (G992CodingParams*)parseBufTmp;
#else
					codingParam = (G992CodingParams*)status->param.dslConnectInfo.buffPtr;
#endif
					dstPtr += SPrintF(dstPtr1, " %s path, trellis %s, K = %d, S = %d, R = %d, D = %d\n",
						((codingParam->D == 1) ? "fast" : "interleave"),
						((value == false) ? "off" : "on"),
						codingParam->K, codingParam->S, codingParam->R, codingParam->D);
					}
					break;
				case	kG992p3XmtCodingParamsInfo:
					dstPtr += SPrintF(dstPtr, "G.992.3 xmt coding parameters :");
					goto PRINT_G992p3_CODING_PARAMS;
				case	kG992p3RcvCodingParamsInfo:
					dstPtr += SPrintF(dstPtr, "G.992.3 rcv coding parameters :");
PRINT_G992p3_CODING_PARAMS:
					{
					char				*dstPtr1;
					G992p3CodingParams	*p3;
					dstPtr1 = dstPtr + strlen(dstPtr);
#ifdef XDSLDRV_ENABLE_PARSER
					p3 = (G992p3CodingParams*)parseBufTmp;
#else
					p3 = (G992p3CodingParams*)status->param.dslConnectInfo.buffPtr;
#endif
					dstPtr += SPrintF(dstPtr1, "MSGc=%d, L=%d, B=%d, M=%d, T=%d, D=%d, R=%d\n",
						p3->MSGc, p3->L, p3->B, p3->M, p3->T, p3->D, p3->R);
					}
					break;

				case	kG992p2TrainingRcvCarrEdgeInfo:
					{
#ifdef XDSLDRV_ENABLE_PARSER
					int		*carrEdges = (int*)parseBufTmp;
#else
					int		*carrEdges = (int*)(status->param.dslConnectInfo.buffPtr);
#endif
					dstPtr += SPrintF(dstPtr, "G.992 rcv carrier range in training: %1d ~ %1d\n",
						carrEdges[0], carrEdges[1]);
					}
					break;
				case	kG992ShowtimeMonitoringStatus:
					{
#ifdef XDSLDRV_ENABLE_PARSER
					uint	*counters = (int*)parseBufTmp;
#else
					uint	*counters = (uint *)(status->param.dslConnectInfo.buffPtr);
#endif
					if ((headerCounter != 0)  && ((headerCounter%3) == 0))
#ifdef DSL_REPORT_ALL_COUNTERS
					dstPtr += SPrintF(dstPtr, "RSWords\tGoodRS\tCorRS\tunCorRS\tSF\tSFErr\trcvCRC\trcvFEC\trcvHEC"
						"\trcvOCD\trcvLCD\tHEC\tOCD\tLCD"
						"\n%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t"
						"%u\t%u\t%u\t%u\t%u\n"
						,counters[0], counters[1], counters[2], counters[3], counters[4], counters[5],
						counters[9], counters[10], counters[11]
						,counters[12], counters[13], counters[14],counters[15], counters[16]);
#else
					dstPtr += SPrintF(dstPtr, "RSWords\tGoodRS\tCorRS\tunCorRS\tSF\tSFErr\trcvCRC\trcvFEC\trcvHEC"
						"\n%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n"
						,counters[0], counters[1], counters[2], counters[3], counters[4], counters[5],
						counters[9], counters[10], counters[11]);
#endif
					else
#ifdef DSL_REPORT_ALL_COUNTERS
					dstPtr += SPrintF(dstPtr, "%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t"
							"%u\t%u\t%u\t%u\t%u\n"
						,counters[0], counters[1], counters[2], counters[3], counters[4], counters[5],
						counters[9], counters[10], counters[11]
						,counters[12], counters[13], counters[14],counters[15], counters[16]);
#else
					dstPtr += SPrintF(dstPtr, "%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\t%u\n"
						,counters[0], counters[1], counters[2], counters[3], counters[4], counters[5],
						counters[9], counters[10], counters[11]);
#endif

					headerCounter++;
					}
					break;
				case	kG992MessageExchangeRcvInfo:
					dstPtr += PrintByteBufferData(dstPtr, "G.992 rcv message:",
						(int)value, (unsigned char*)status->param.dslConnectInfo.buffPtr);
					break;
				case	kG992MessageExchangeXmtInfo:
					dstPtr += PrintByteBufferData(dstPtr, "G.992 xmt message:",
						(int)value, (unsigned char*)status->param.dslConnectInfo.buffPtr);
					break;
				case	kG992AocMessageExchangeRcvInfo:
					dstPtr += PrintByteBufferData(dstPtr, "G.992 AOC rcv message:",
						(int)value, (unsigned char*)status->param.dslConnectInfo.buffPtr);
					break;
				case	kG992AocMessageExchangeXmtInfo:
					dstPtr += PrintByteBufferData(dstPtr, "G.992 AOC xmt message:",
						(int)value, (unsigned char*)status->param.dslConnectInfo.buffPtr);
					break;
				case	kG992AocBitswapRxStarted:
					dstPtr += SPrintF(dstPtr, "G992 Rcv started Bit Swap procedure on superframe  %d\n ", value);
					break;
				case	kG992AocBitswapTxStarted:
					dstPtr += SPrintF(dstPtr, "G992 Xmt started Bit Swap procedure on superframe  %d\n ", value);
					break;
				case	kG992AocBitswapRxCompleted:
					dstPtr += SPrintF(dstPtr, "G992 Rcv completed Bit Swap procedure on superframe  %d\n ", value);
					break;
				case	kG992AocBitswapTxCompleted:
					dstPtr += SPrintF(dstPtr, "G992 Xmt completed Bit Swap procedure on superframe  %d\n ", value);
					break;
#endif	/* G992 */
				case	kDslHWTimeTrackingResetClockError:
					break;
				case 	kDslATUHardwareAGCObtained:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "DSL transceiver total hardware AGC obtained : %s dB\n", Q4ToFloatString(value));
#endif
					break;
				case	kDslATURcvPowerInfo:
					dstPtr += SPrintF(dstPtr, "DSL transceiver received power : 0x%8.8X\n", value);
					break;
				case	kDslATUAvgLoopAttenuationInfo:
					dstPtr += SPrintF(dstPtr, "DSL transceiver loop attenuation including cutback: %s dB\n", Q4ToFloatString(value));
					break;
				case	kDslATUAvgLoopAttenuationInfoAt300kHz:
					dstPtr += SPrintF(dstPtr, "DSL transceiver loop attenuation including cutback at 300kHz: %s dB\n", Q4ToFloatString(value));
					break;
				case    kDslSignalAttenuation:
				    dstPtr += SPrintF(dstPtr, "DSL transceiver signal attenuation: %s dB\n",  Q4ToFloatString(value));
				    break;
				case	kDslTEQCoefInfo:
				{
#ifdef XDSLDRV_ENABLE_PARSER
					short *pData = (short*)parseBufTmp;
#else
					short *pData = (short*)status->param.dslConnectInfo.buffPtr;
#endif
					dstPtr += PrintShortBufferData(dstPtr,
						"Dsl Rcv TEQ coefs:",
						(int)value, pData);
					break;
				}
				case	kDslRcvCarrierSNRInfo:
				{
#ifdef XDSLDRV_ENABLE_PARSER
					ushort *pData = (ushort*)parseBufTmp;
#else
					ushort *pData = (ushort*)status->param.dslConnectInfo.buffPtr;
#endif
					dstPtr += PrintQ4dBBufferData(dstPtr,
						"Dsl Rcv carrier SNRs :",
						(int)value, pData);
					break;
				}
				case	kDslChannelResponseLog:
				{
#ifdef XDSLDRV_ENABLE_PARSER
					ushort *pData = (ushort*)parseBufTmp;
#else
					ushort *pData = (ushort*)status->param.dslConnectInfo.buffPtr;
#endif
					dstPtr += PrintQ4dBBufferData(dstPtr,
						"Dsl Channel Response (Log) :",
						(int)value, pData);
					break;
				}
				case    kDslHLinScale:
				    dstPtr += SPrintF(dstPtr, "DSL Channel Response (Linear) Scale: %d \n", (ushort)value);
				    break;
				case	kDslChannelResponseLinear:
				{
#ifdef XDSLDRV_ENABLE_PARSER
					short *pData = (short*)parseBufTmp;
#else
					short *pData = (short*)status->param.dslConnectInfo.buffPtr;
#endif
					dstPtr += PrintShortBufferData(dstPtr,
						"Dsl Channel Response (Linear) :",
						(int)value, pData);
					break;
				}
				case	kDslChannelQuietLineNoise:
					dstPtr += PrintByteBufferData(dstPtr,
						"Dsl Channel Quiet Line Noise :",
						(int)value, (uchar*)status->param.dslConnectInfo.buffPtr);
					break;
				case	kDslRcvPsdInfo:
				{
#ifdef XDSLDRV_ENABLE_PARSER
					ushort *pData = (ushort*)parseBufTmp;
#else
					ushort *pData = (ushort*)status->param.dslConnectInfo.buffPtr;
#endif
					dstPtr += PrintQ4dBBufferData(dstPtr,
						"Dsl Rcv PSDs :",
						(int)value, pData);
					break;
				}
				case	kDslMaxReceivableBitRateInfo:
					dstPtr += SPrintF(dstPtr, "DSL transceiver maximal receivable data rate : %d kbps\n", value);
					break;
				case    kDslAttainableNetDataRate:
					dstPtr += SPrintF(dstPtr, "DSL transceiver attainable net data rate : %d bps\n", value);
					break;
				case	kDslSelectedTimingTone:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "DSL transceiver timing tone : %d\n", value);
#endif
					break;
				case	kDslATUCXmtPowerCutbackInfo:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "ATUC xmt power cutback : %d dB\n", value);
#endif
					break;
				case	kDslATURXmtPowerCutbackInfo:
					dstPtr += SPrintF(dstPtr, "ATUR xmt power cutback : %d dB\n", value);
					break;
				case	kDslATUCXmtPowerInfo:
					dstPtr += SPrintF(dstPtr, "ATUC total xmt power : %s dBm\n", Q4ToFloatString(value));
					break;
				case	kDslATURXmtPowerInfo:
					dstPtr += SPrintF(dstPtr, "ATUR total xmt power : %s dBm\n", Q4ToFloatString(value));
					break;
				case	kDslATUCShowtimeXmtPowerInfo:
					dstPtr += SPrintF(dstPtr, "ATUC showtime total xmt power : %s dBm\n", Q4ToFloatString(value));
					break;
#ifdef	G992P3
				case	kG992EventSynchSymbolDetected:
					dstPtr += SPrintF(dstPtr, "DSL found positive sync symbol after reversal synch symbol\n");
					break;
				case	kG992EventReverseSynchSymbolDetected:
					dstPtr += SPrintF(dstPtr, "DSL found reversal sync symbol after positive synch symbol\n");
					break;
#endif
#ifdef	SYNCH_SYMBOL_DETECTION
				case	kG992RcvDetectSyncSymbolOffset:
					dstPtr += SPrintF(dstPtr, "DSL found sync symbol in offset of %d symbols\n", value);
					break;
#endif
				case	kDslFramingModeInfo:
					dstPtr += SPrintF(dstPtr, "Framing mode = %d\n", (int)value);
					break;

				case	kDslG992VendorID:
					{
						int iD = (value<13)?value:0;
					/* typedef enum
					{
					kVendorUnknown = 0,
					kVendorBroadcom,
					kVendorGlobespan,
					kVendorADI,
					kVendorTI,
					kVendorCentillium,
					kVendorAlcatel,
					kVendorInfineon,
					kVendorIkanos,
					kVendorCatena,
					kVendorAlcatelLSpan,
					kVendorConexant,
					kVendorCentilliumAllZero,
					} VendorIDType;*/
					static char * vendorNameTbl[] =
						{
						"Unknown",
						"Broadcom",
						"Globespan",
						"ADI",
						"TI",
						"Centillium",
						"Alcatel",
						"Infineon",
						"Ikanos",
						"Catena",
						"AlcatelLSpan",
						"Conexant",
						"Centillium"
						};
					dstPtr += SPrintF(dstPtr, "CO Vendor ID = %d(%s)\n", (int)value, vendorNameTbl[iD]);
					}
					break;
				case kDslNLMaxCritNoise:
					dstPtr += SPrintF(dstPtr, "DSL NL max critical noise: %s dB\n", Q4ToFloatString(value));
					break;
				case kDslNLAffectedBits:
					dstPtr += SPrintF(dstPtr, "DSL NL affected bits: %d \n", value);
					break;
				case kDslNLdbEcho:
					dstPtr += SPrintF(dstPtr, "DSL NL Echo (ENR): %s dB\n", Q4ToFloatString(value));
					break;
				case kDslNLAffectedBins:
					dstPtr += SPrintF(dstPtr, "DSL NL affected bins: %d \n", value);
					break;
				case kG992AocBitswapTxDenied:
					dstPtr += SPrintF(dstPtr, "AOC Tx bitswap denied, reason=%d\n", value);
					break;
				case kG992AocBitswapRxDenied:
					dstPtr += SPrintF(dstPtr, "AOC Rx bitswap denied, reason=%d\n", value);
					break;
				case kG992BitswapState:
					dstPtr += SPrintF(dstPtr, "Bitswap state, status=0x%X\n", value);
					break;
				default:
					dstPtr += SPrintF(dstPtr, "DSL connection info, code = %d, value = %d, bufPtr = 0x%px\n",
						code, value, status->param.dslConnectInfo.buffPtr);
					break;
				}
			}
			break;

		case kDslShowtimeSNRMarginInfo:
#ifndef LITE_VERSION
			dstPtr += SPrintF(dstPtr, "Margin Info: MAX(%d, %s) ",
				status->param.dslShowtimeSNRMarginInfo.maxMarginCarrier,
				Q4ToFloatString(status->param.dslShowtimeSNRMarginInfo.maxSNRMargin));
			dstPtr += SPrintF(dstPtr, "MIN(%d, %s) ",
				status->param.dslShowtimeSNRMarginInfo.minMarginCarrier,
				Q4ToFloatString(status->param.dslShowtimeSNRMarginInfo.minSNRMargin));
			dstPtr += SPrintF(dstPtr, "AVG=%s nTones=%d ",
				Q4ToFloatString(status->param.dslShowtimeSNRMarginInfo.avgSNRMargin),
				status->param.dslShowtimeSNRMarginInfo.nCarriers);

			if (status->param.dslShowtimeSNRMarginInfo.nCarriers != 0)
				dstPtr += PrintQ4dBBufferData(dstPtr, "\nSNR Margins:",
					(int)status->param.dslShowtimeSNRMarginInfo.nCarriers,
#ifdef XDSLDRV_ENABLE_PARSER
					(ushort*)parseBufTmp
#else
					(ushort*)status->param.dslShowtimeSNRMarginInfo.buffPtr
#endif
					);
#endif
			break;

		case		kAtmStatus:
			{
#if 1
			uint	code	= status->param.atmStatus.code;
			int	value   = status->param.atmStatus.param.value;

			switch (code)
				{
				case kAtmStatRxHunt:
					dstPtr += SPrintF(dstPtr, "I.432 hunting for cell header\n");
					break;
				case kAtmStatRxPreSync:
					dstPtr += SPrintF(dstPtr, "I.432 in presync state\n");
					break;
				case kAtmStatRxSync:
					dstPtr += SPrintF(dstPtr, "I.432 in sync state\n");
					break;
				case kAtmStatRxPlOamCell:
  					dstPtr += SPrintF(dstPtr, "F1 or F3 OAM cell received. Hdr = 0x%X\n", value);
					break;
				case kAtmStatHdrCompr:
#ifndef LITE_VERSION
					dstPtr += SPrintF(dstPtr, "I.432 header compression %s\n", value ? "ON" : "OFF");
#endif
					break;

				/* ATM layer */

				case kAtmStatRxDiscarded:
  					dstPtr += SPrintF(dstPtr, "ATM Rx cell discarded (contract violation). VC = %d\n", value);
					break;
				case kAtmStatTxDelayed:
  					dstPtr += SPrintF(dstPtr, "ATM Tx cell delayed (contract violation). VC = %d\n", value);
					break;

				case kAtmStatVcCreated:
  					dstPtr += SPrintF(dstPtr, "ATM VC=%d created. VCI=%d, AALType=%d, FwdCellTime=%d, BackCellTime=%d\n",
						status->param.atmStatus.param.vcInfo.vcId,
						status->param.atmStatus.param.vcInfo.vci,
						status->param.atmStatus.param.vcInfo.aalType,
						status->param.atmStatus.param.vcInfo.fwdPeakCellTime,
						status->param.atmStatus.param.vcInfo.backPeakCellTime);
					break;

				case kAtmStatVcStarted:
  					dstPtr += SPrintF(dstPtr, "ATM VC = %d started\n", value);
					break;
				case kAtmStatVcStopped:
  					dstPtr += SPrintF(dstPtr, "ATM VC = %d stopped\n", value);
					break;
				case kAtmStatVcDeleted:
  					dstPtr += SPrintF(dstPtr, "ATM VC = %d deleted\n", value);
					break;

				case kAtmStatTimeout:
   					dstPtr += SPrintF(dstPtr, "No cells transmitted for %d ms\n", value);
					break;

				case kAtmStatNoCellMemory:
  					/*dstPtr += SPrintF(dstPtr, "No memory for Rx cells \n");*/
					break;

				case kAtmStatUnassignedCell:
  					dstPtr += SPrintF(dstPtr, "Unassigned cell. hdr = 0x%X\n", value);
					break;
				case kAtmStatOamI371Cell:
  					dstPtr += SPrintF(dstPtr, "VP resource management cell (I.371). hdr = 0x%X\n", value);
					break;
				case kAtmStatReservedCell:
  					dstPtr += SPrintF(dstPtr, "Reserved cell. hdr = 0x%X\n", value);
					break;
				case kAtmStatInvalidCell:
  					dstPtr += SPrintF(dstPtr, "Invalid cell header(no VC) . hdr = 0x%X\n", value);
					break;

				case kAtmStatOamF4SegmentCell:
  					dstPtr += SPrintF(dstPtr, "Segment OAM F4 cell. hdr = 0x%X, oamCmd = 0x%X\n",
						status->param.atmStatus.param.oamInfo.cellHdr,
						status->param.atmStatus.param.oamInfo.oamCmd);
					break;
				case kAtmStatOamF4End2EndCell:
  					dstPtr += SPrintF(dstPtr, "End-to-end OAM F4 cell. hdr = 0x%X, oamCmd = 0x%X\n",
						status->param.atmStatus.param.oamInfo.cellHdr,
						status->param.atmStatus.param.oamInfo.oamCmd);
					break;
				case kAtmStatOamF5SegmentCell:
  					dstPtr += SPrintF(dstPtr, "Segment OAM F5 cell. hdr = 0x%X, oamCmd = 0x%X\n",
						status->param.atmStatus.param.oamInfo.cellHdr,
						status->param.atmStatus.param.oamInfo.oamCmd);
					break;
				case kAtmStatOamF5End2EndCell:
  					dstPtr += SPrintF(dstPtr, "End-to-end OAM F5 cell. hdr = 0x%X, oamCmd = 0x%X\n",
						status->param.atmStatus.param.oamInfo.cellHdr,
						status->param.atmStatus.param.oamInfo.oamCmd);
					break;
				case kAtmStatOamLoopback:
  					dstPtr += SPrintF(dstPtr, "OAM Loopback. Indication = 0x%X\n", value);
					break;

				case kAtmStatConnected:
   					dstPtr += SPrintF(dstPtr, "ATM link connected\n");
					break;

				case kAtmStatDisconnected:
   					dstPtr += SPrintF(dstPtr, "ATM link disconnected\n");
					break;

				case kAtmStatRxPacket:
				case kAtmStatTxPacket:
#if 0
					{
					long			i, frBytes;

  					dstPtr += SPrintF(dstPtr, "VCI=%d AAL=%d %s LEN=%d\n",
						status->param.atmStatus.param.frame.vci,
						status->param.atmStatus.param.frame.aalType,
						kAtmStatRxPacket == code ? "RX" : "TX",
						status->param.atmStatus.param.frame.length);

					frBytes = status->param.atmStatus.param.frame.length;
					if  (frBytes > kMaxFlattenFramelength)
						frBytes = kMaxFlattenFramelength;
					for (i = 0; i < frBytes; i++)
						dstPtr += SPrintF (dstPtr, "%X ", status->param.atmStatus.param.frame.framePtr[i]);
					dstPtr += SPrintF (dstPtr, "\n");
					}
#endif
					break;

				case kAtmStatPrintCell:
#if 0
					{
					uchar	*cellHdr;
					uchar	*cellData, *cellDataEnd;

					cellHdr = (uchar *) status->param.atmStatus.param.cellInfo.cellHdr;
					cellData = (uchar *) status->param.atmStatus.param.cellInfo.cellData;
					cellDataEnd = cellData + sizeof(atmCellData);

   					dstPtr += SPrintF(dstPtr, "ATM cell: %s hdr = %X %X %X %X data = ",
						status->param.atmStatus.param.cellInfo.pHdr,
						cellHdr[0], cellHdr[1], cellHdr[2], cellHdr[3]);
					do
						{
						dstPtr += SPrintF (dstPtr, "%X %X %X %X %X %X %X %X ",
							cellData[0], cellData[1], cellData[2], cellData[3],
							cellData[4], cellData[5], cellData[6], cellData[7]);
						cellData += 8;
						} while (cellData < cellDataEnd);
					}
#endif
					break;

				case kAtmStatBertResult:
					dstPtr += SPrintF(dstPtr, "BERT results: totalBits=%u, errBits=%u\n",
						value, status->param.atmStatus.param.bertInfo.errBits);
					break;
#ifdef XDSLDRV_ENABLE_PARSER
				case kAtmStatCounters:
#endif
				case kAtmStatCounters1:
					{
#ifdef XDSLDRV_ENABLE_PARSER
					atmPhyCounters *pAtmCnt = (atmPhyCounters *)parseBufTmp;
#else
					atmPhyCounters *pAtmCnt = (void *) status->param.atmStatus.param.value;
#endif
					dstPtr += SPrintF(dstPtr, "ATM cnt: id=%u BERT=(%u: %u %u %u) RX=(%u %u %u %u %u) TX=(%u %u)\n",
						pAtmCnt->id, pAtmCnt->bertStatus, pAtmCnt->bertCellTotal, pAtmCnt->bertCellCnt, pAtmCnt->bertBitErrors,
						pAtmCnt->rxHecCnt, pAtmCnt->rxCellTotal, pAtmCnt->rxCellData, pAtmCnt->rxCellDrop, pAtmCnt->rxOCD,
						pAtmCnt->txCellTotal, pAtmCnt->txCellData);
					}
					break;
				}
#endif
			}
			break;


		case	kDslFrameStatus:
			{
/*			switch (status->param.dslFrameInfo.code)
				{
#if 0
				case kDslFrameStatusSend:
  					dstPtr += SPrintF(dstPtr, "Send frame on VC = %d, time = %d\n",
						status->param.dslFrameInfo.vcId,
						status->param.dslFrameInfo.timeStamp);
					break;
				case kDslFrameStatusSendComplete:
  					dstPtr += SPrintF(dstPtr, "Send complete on VC = %d, time = %d\n",
						status->param.dslFrameInfo.vcId,
						status->param.dslFrameInfo.timeStamp);
					break;
				case kDslFrameStatusRcv:
  					dstPtr += SPrintF(dstPtr, "Frame received on VC = %d, time = %d\n",
						status->param.dslFrameInfo.vcId,
						status->param.dslFrameInfo.timeStamp);
					break;
				case kDslFrameStatusReturn:
  					dstPtr += SPrintF(dstPtr, "Frame returned at time = %d\n",
						status->param.dslFrameInfo.timeStamp);
					break;
#endif
				}
*/			}
			break;

		case		kDslPrintfStatus:
		case		kDslPrintfStatus1:
		{ /* SoftDslDpApiPrintfF in PHY implementation */
#if defined(__arm) || defined(CONFIG_ARM) || defined(CONFIG_ARM64)
			int i;
			uint *argPtr = status->param.dslPrintfMsg.argPtr;
			for (i = 0; i < status->param.dslPrintfMsg.argNum; i++)
				argPtr[i] = ADSL_ENDIAN_CONV_INT32(argPtr[i]);
#ifdef CONFIG_ARM64
			/* Can't use vsprintf since PHY arguments are 32bit not 64bit expected by vsprintf */
			if (status->param.dslPrintfMsg.argNum > 18)
			  AdslDrvPrintf(TEXT("kDslPrintfStatus: nArgs=%d > 18 won't be printed\n"), status->param.dslPrintfMsg.argNum);
			dstPtr += SPrintF (dstPtr, status->param.dslPrintfMsg.fmt, argPtr[0], argPtr[1], argPtr[2], argPtr[3], 
				argPtr[4], argPtr[5], argPtr[6], argPtr[7], argPtr[8], argPtr[9], argPtr[10], argPtr[11],
				argPtr[12], argPtr[13], argPtr[14], argPtr[15], argPtr[16], argPtr[17]);
#else
			va_list ap;
			ap.__ap = status->param.dslPrintfMsg.argPtr;
			dstPtr += VSPrintF (dstPtr, status->param.dslPrintfMsg.fmt, ap);
#endif
#else /* !ARM */
			char  *fmt = status->param.dslPrintfMsg.fmt;
			if (status->param.dslPrintfMsg.argNum & kDslDbgDataStrIdF) {
				int   fmtLen;
				char  *fmtStr = GetStatusParserFmtStr(&fmtLen);
				if (fmtLen != 0) {
				  if (NULL != fmt)
				    strcpy(fmtStr + fmtLen, fmt);
				  fmt = fmtStr;
				  ClrStatusParserFmtStr(); /* ok to clear as already processed */
				}
			}
			dstPtr += VSPrintF (dstPtr, fmt, status->param.dslPrintfMsg.argPtr);
#endif
		}
			break;

		case		kDslExceptionStatus:
			{
			uint	*sp, spAddr;
			int		stackSize;
#ifdef CONFIG_ARM64
			sp = pStackPtr;
#else
			sp = (uint *) status->param.dslException.sp;
#endif
			dstPtr += SPrintF(dstPtr, "DSL Exception:\n");
			dstPtr += PrintExcpReg(dstPtr, sp);
			dstPtr += PrintExcpArgs(dstPtr, status->param.dslException.argc, (uint *) status->param.dslException.argv);

			//sp = (uint *) status->param.dslException.sp;
			spAddr = sp[28];
#ifdef FLATTEN_ADDR_ADJUST
#ifdef XDSLDRV_ENABLE_PARSER
			sp = (uint *)LMEM_ADDR_TO_HOST(spAddr);
#else
			sp = (uint *) (spAddr | FLATTEN_ADDR_ADJUST);
#endif
			stackSize = 64;
#else
			sp = (uint *) status->param.dslException.stackPtr;
			stackSize = status->param.dslException.stackLen;
#endif
			dstPtr += PrintExcpStack(dstPtr, spAddr, stackSize, sp);
			}
			break;

		case kDslPingResponse:
			dstPtr += SPrintF(dstPtr, "Ping response received\n");
			break;

		case kDslGetOemParameter:
			dstPtr += SPrintF(dstPtr, "GetOemParam: %s\n",
				OemParameterToString(status));
#if 0
				(uint)status->param.dslOemParameter.dataPtr,
				status->param.dslOemParameter.dataLen;
#endif
			break;
		case kDslOemDataAddrStatus:
			dstPtr += SPrintF(dstPtr, "DslOemSharedDataAddr = 0x%08X\n", status->param.value);
			break;

		case kDslDataAvailStatus:
			{
			uint	dataAddrOrId = (uint) status->param.dslDataAvail.dataPtr;

			if (0 != (dataAddrOrId & 0xFF000000))
				dstPtr += SPrintF(dstPtr, "Afe test data ready, addr=0x%08X len=%d\n",
					dataAddrOrId, (int)status->param.dslDataAvail.dataLen);
			}
			break;

		case kDslLineIdStatus:
			dstPtr = dstPtr1 = dstPtr0;
			dstPtr += SPrintF(dstPtr, "<<< Line%d >>>\n", (int)status->param.value);
			curLineId = status->param.value;
			break;

		case kDslOLRRequestStatus:
			dstPtr += SPrintF(dstPtr,
				"OLR Request, type=%d nCarr=%d\n"
				"\tL[0-3]=%d %d %d %d\n"
				"\tB[0-3]=%d %d %d %d\n",
				status->param.dslOLRRequest.msgType, status->param.dslOLRRequest.nCarrs,
				status->param.dslOLRRequest.L[0], status->param.dslOLRRequest.L[1],
				status->param.dslOLRRequest.L[2], status->param.dslOLRRequest.L[3],
				status->param.dslOLRRequest.B[0], status->param.dslOLRRequest.B[1],
				status->param.dslOLRRequest.B[2], status->param.dslOLRRequest.B[3]
				);
			break;
		case kDslOLRResponseStatus:
			dstPtr += SPrintF(dstPtr, "OLR Response, code=0x%X\n", status->param.value);
			break;

		case kDslOLRBitGainUpdateStatus:
			dstPtr += SPrintF(dstPtr, "B&G update: nCarr=%d\n", status->param.dslOLRRequest.nCarrs);
			break;

		case kDslPwrMgrStatus:
			dstPtr += SPrintF(dstPtr, "PwrMgr: type=%d, msgLen=%d\n", status->param.dslPwrMsg.msgType,
				(kPwrL2Grant == status->param.dslPwrMsg.msgType) ? status->param.dslPwrMsg.param.msg.msgLen : 0);
			break;

		case		kTestFinished:
		case		kTestAtmVcFinished:
		case		kTestClearEocFinished:
		case		kTestG997Finished:
			{
			uint	nBits			= status->param.testResults.nBits;
			uint	nBlocks			= status->param.testResults.nBlocks;
			uint	nBitErrors		= status->param.testResults.nBitErrors;
			uint	nBlockErrors	= status->param.testResults.nBlockErrors;
			char	bitErrStr[11], blkErrStr[11], *sHdr;

			CopyBytes(GetErrorRateString(nBits, nBitErrors), bitErrStr, 10);
			CopyBytes(GetErrorRateString(nBlocks, nBlockErrors), blkErrStr, 10);
			bitErrStr[10] = blkErrStr[10] = 0;

			if (kTestAtmVcFinished == status->code)
				sHdr = "AtmVc";
			else if (kTestClearEocFinished == status->code)
				sHdr = "ClearEOC";
			else if (kTestG997Finished == status->code)
				sHdr = "G997";
			else
				sHdr = "Modem";

			dstPtr += SPrintF(dstPtr,
				"%s : %d bits / %d blocks received; %d bit / %d block errors;	BER: %s ;	BLER: %s\n",
				sHdr,
				nBits,
				nBlocks,
				nBitErrors,
				nBlockErrors,
				bitErrStr,
				blkErrStr);
			}
			break;
		case		kConnectivityTestFinished:
			dstPtr += SPrintF( dstPtr, "Connection established! \n");
			break;
		case		kTestCheckSum:
			dstPtr += SPrintF( dstPtr, "Test checksums = %d %x %x %x ",
				status->param.checksums.numberOfCalls,
				status->param.checksums.txSignalChecksum,
				status->param.checksums.rxSignalChecksum,
				status->param.checksums.eyeDataChecksum);
			break;
		case kDslReceivedEocCommand:
		{
		
#ifdef XDSLDRV_ENABLE_PARSER
			pDataPtrOrig = status->param.dslClearEocMsg.dataPtr;
			status->param.dslClearEocMsg.dataPtr = parseBufTmp;
#endif
			dstPtr += StatusParserProcReceiveEocMsg(status, dstPtr, lineId);
#ifdef XDSLDRV_ENABLE_PARSER
			status->param.dslClearEocMsg.dataPtr = pDataPtrOrig;
#endif
			break;
		}
		case		kDslSendEocCommandDone:
		case		kDslSendEocCommandFailed:
		case		kDslWriteRemoteRegisterDone:
		case		kDslReadRemoteRegisterDone:
			dstPtr += SPrintF(dstPtr, "EOC status. Code=%d Value=%d ",	status->code, status->param.value);
			break;
#ifdef G997_1_FRAMER
		case		kDslG997Status:
			switch (status->param.g997Status.code)
				{
				case kDslFramerRxFrame:
					dstPtr += SPrintF(dstPtr,  "G997 Rx frame ");
					break;
				case kDslFramerRxFrameErr:
					dstPtr += SPrintF(dstPtr,  "G997 Rx frame error=%d ", status->param.g997Status.param.error);
					break;
				case kDslFramerTxFrame:
					dstPtr += SPrintF(dstPtr,  "G997 Tx frame ");
					break;
				case kDslFramerTxFrameErr:
					dstPtr += SPrintF(dstPtr,  "G997 Tx frame error=%d ", status->param.g997Status.param.error);
					break;
				}
			break;
#endif
		case kDslAfeTestStatus:
			switch(status->param.dslAfeTestStatus.type) {
				case kDslAfeTestCaptureDone:	/* param1 - RcvBuffPtr,  param2 - RcvLen */
					dstPtr += SPrintF(dstPtr,  "AfeTestCaptureDone: RcvBuffPtr=0x%X RcvLen=0x%X ",
						status->param.dslAfeTestStatus.param1, status->param.dslAfeTestStatus.param2);
					break;
				case kDslAfeTestRetBuffPoolSize:	/* param1 - Total buffer pool size */
					dstPtr += SPrintF(dstPtr,  "AfeTestRetBuffPoolSize: Total buffer pool size=%d",
						status->param.dslAfeTestStatus.param1);
					break;
				case kDslAfeTestRetXmtBuffPtr:	/* param1 - XmtBuffPtr */
					dstPtr += SPrintF(dstPtr,  "AfeTestRetXmtBuffPtr: XmtBuffPtr=0x%X",
						status->param.dslAfeTestStatus.param1);
					break;
				case kDslAfeTestRetRcvtBuffPtr:	/* param1 - RcvBuffPtr */
					dstPtr += SPrintF(dstPtr,  "AfeTestRetRcvBuffPtr: RcvBuffPtr=0x%X",
						status->param.dslAfeTestStatus.param1);
					break;
				case kDslAfeTestRetXregVal:		/* param1 - Xreg Addr, param2 - Xreg Value */
					dstPtr += SPrintF(dstPtr,  "AfeTestRetXregVal: Xreg Addr=0x%X Xreg Value=0x%X ",
						status->param.dslAfeTestStatus.param1, status->param.dslAfeTestStatus.param2);
					break;
				case kDslAfeTestRetIregVal:		/* param1 - Ireg Addr, param2 - Ireg Value */
					dstPtr += SPrintF(dstPtr,  "AfeTestRetIregVal: Ireg Addr=0x%X Ireg Value=0x%X ",
						status->param.dslAfeTestStatus.param1, status->param.dslAfeTestStatus.param2);
					break;
				default:
					dstPtr += SPrintF(dstPtr,  "Status kDslAfeTestStatus: Unknown type=0x%X ",
						status->param.dslAfeTestStatus.type);
					break;
			}
			break;
		case kDslFastRetrain:
			dstPtr += SPrintF(dstPtr,  "Fast retrain active, value %d\n", status->param.value);
			break;
		default:
			dstPtr += SPrintF(dstPtr,  "Status code %d, value %d\n", status->code, status->param.value);
			break;
		}

	if (*dstPtr1 == 0)   /* empty string */
		*dstPtr0 = 0;
	else { // ('\n' != *(dstPtr - 2))
		*(dstPtr - 1) = '\n';
		*dstPtr++ = 0;
	}
	return dstPtr - dstPtr00;
	}

Public char*
GetModulationString(modulationMap modulation)
	{
	static	char modNum[32];

	switch (modulation)
		{
		case kG992p1AnnexA:
			return (char*)"G.992.1 (dmt) Annex A";
		case kG992p1AnnexB:
			return (char*)"G.992.1 (dmt) Annex B";
		case kG992p1AnnexC:
			return (char*)"G.992.1 (dmt) Annex C";
		case kG992p2AnnexAB:
			return (char*)"G.992.2 (lite) Annex A/B";
		case kG992p2AnnexC:
			return (char*)"G.992.2 (lite) Annex C";
		case kG992p3AnnexA:
			return (char*)"G.992.3 (DMTbis) Annex A";
		case kG992p5AnnexA:
			return (char*)"G.992.5 (DMTbis) Annex A";
		case kG992p5AnnexB:
			return (char*)"G.992.5 (DMTbis) Annex B";
		case kG992p5AnnexM:
			return (char*)"G.992.5 (DMTbis) Annex M";
		case kG992p5AnnexJ:
			return (char*)"G.992.5 (DMTbis) Annex J";
		case kG992p3AnnexB:
			return (char*)"G.992.3 (DMTbis) Annex B";
		case kG992p3AnnexM:
			return (char*)"G.992.3 (DMTbis) Annex M";
		case kG992p3AnnexJ:
			return (char*)"G.992.3 (DMTbis) Annex J";
		case (kG992p1AnnexI>>4):
			return (char*)"G.992.1 (dmt) Annex I";
		case(kG993p2AnnexA):
			return (char*)"G.993.2 Annex A";
		case(kGfastAnnexA):
			return (char*)"Gfast";
		default:
			SPrintF(modNum,  "0x%X ", modulation);
			return modNum;
		}
	}




Private char*
GetErrorRateString(unsigned int tries, unsigned int errors)
	{
	static	char str[10];
	int		ex = 0;
	unsigned int	ratio;

	if ( (tries == 0) || ( tries < errors))
		{
		return (char*)"NaN";
		}
	if (errors == 0)
		{
		return (char*)"0.00";
		}
	if (tries == errors)
		{
		return (char*)"1.00";
		}
	if (tries < 100)
		{
		return (char*)"****";
		}
	do
		{
		ex--;
		errors *= 10;
		} while ( tries >= errors);
	ex++;
	if (tries >	10000)
		{
		ratio	= errors / (tries / 1000);
		SPrintF( str, "0.%03de%+2d",ratio, ex);
		return str;
		}
	else
		{
		ratio	= errors / (tries / 10);
		SPrintF( str, "0.%1de%+2d",ratio, ex);
		return str;
		}
	}


Private char*
Q4ToFloatString(int val)
	{
	static	char	str1[32];
	int				iPart,
					fPart;
	char			sign;

	sign = ' ';
	if (val < 0)
		{
		val = -val;
		sign = '-';
		}
	iPart = val >> 4;
	fPart = (val & 0xF) * 625;
	if (fPart == 625)
		{
		SPrintF( str1, "%c%d.0625", sign, iPart);
		}
	else
		{
		SPrintF( str1, "%c%d.%d", sign, iPart, fPart);
		}
	return str1;
	}

Private char*
Q8ToFloatString(int val)
	{
	static	char	str2[32];
	int				iPart,
					fPart;
	char			sign;

	sign = ' ';
	if (val < 0)
		{
		val = -val;
		sign = '-';
		}
	iPart = val >> 8;
	fPart = (val & 0xFF) * 390625;
	if (fPart < 1000000)
	  {
	    fPart /= 10000;
	    SPrintF( str2, "%c%d.00%d", sign, iPart, fPart);
	  }
	else if (fPart < 10000000)
	  {
	    fPart /= 100000;
	    SPrintF( str2, "%c%d.0%d", sign, iPart, fPart);
	  }
	else
	  {
	    fPart /= 1000000;
	    SPrintF( str2, "%c%d.%d", sign, iPart, fPart);
	  }
	return str2;
	}
