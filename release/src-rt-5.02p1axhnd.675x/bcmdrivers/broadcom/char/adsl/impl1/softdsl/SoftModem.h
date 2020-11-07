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
 * SoftModem.h 
 *
 *
 * Description:
 *	This file contains the exported interface for SoftModem.c
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Mark Gonikberg, Haixiang Liang.
 *
 * $Revision: 1.20 $
 *
 * $Id: SoftModem.h,v 1.20 2005/03/17 06:34:33 ilyas Exp $
 *
 * $Log: SoftModem.h,v $
 * Revision 1.20  2005/03/17 06:34:33  ilyas
 * Added firmware major, minor and patch numbers
 *
 * Revision 1.19  2005/03/10 21:43:03  ilyas
 * PR30201: Added functions to retrieve firmware version string and it's size
 *
 * Revision 1.18  2004/10/14 22:12:08  kdu
 * Redefine 'long long' to 'LONGLONG' for diags build.
 *
 * Revision 1.17  2004/07/01 00:12:12  nino
 * Added preliminary code/defines for debugDataHandler (inside of #if DEBUG_DATA_HANDLER).
 *
 * Revision 1.16  2004/04/14 21:16:51  ilyas
 * Merged with the latest changes in ADSL driver
 *
 * Revision 1.15  2004/04/13 00:56:10  ilyas
 * Merged the latest ADSL driver changes for RTEMS
 *
 * Revision 1.14  2004/04/13 00:16:59  ilyas
 * Merged the latest ADSL driver changes
 *
 * Revision 1.13  2003/02/22 05:07:11  ilyas
 * Added VendorID for T1.413 mode
 *
 * Revision 1.12  2002/10/03 19:34:24  ilyas
 * Added size for EOC serial number register
 *
 * Revision 1.11  2002/09/07 01:37:22  ilyas
 * Added support for OEM parameters
 *
 * Revision 1.10  2001/12/13 02:25:34  ilyas
 * Added definitions for G997
 *
 * Revision 1.9  2001/11/30 05:56:34  liang
 * Merged top of the branch AnnexBDevelopment onto top of the tree.
 *
 * Revision 1.7.2.2  2001/11/27 02:32:05  liang
 * Combine vendor ID, serial #, and version number into SoftModemVersionNumber.c.
 *
 * Revision 1.7.2.1  2001/10/03 01:44:10  liang
 * Merged with codes from main tree (tag SoftDsl_2_18).
 *
 * Revision 1.8  2001/09/21 19:19:01  ilyas
 * Minor fixes for VxWorks build
 *
 * Revision 1.7  2000/07/17 21:08:16  lkaplan
 * removed global pointer
 *
 * Revision 1.6  2000/05/03 04:09:11  ilyas
 * Added ID for ATM log data
 *
 * Revision 1.5  2000/04/01 01:07:44  liang
 * Changed file names and some module names.
 *
 * Revision 1.4  2000/03/02 20:18:12  ilyas
 * Added test status code for ATM VC finished
 *
 * Revision 1.3  1999/08/05 20:02:11  liang
 * Merged with the softmodem top of the tree on 08/04/99.
 *
 * Revision 1.2  1999/01/27 22:19:08  liang
 * Merge with SoftModem_3_1_02.
 * Include SoftDsl.h conditionlly so that the test utilities from SoftModem
 * can be used without major change. It can be merged easily to SoftModem.
 *
 * Revision 1.170  1998/12/22 00:52:52  liang
 * Added auxFeatures bit kV8HoldANSamUntilDetCI. When it is set, ANSam won't be
 * sent until CI is detected (normally ANSam will be sent after 200ms). This is
 * useful in V34 half duplex fax mode.
 *
 * Revision 1.169  1998/12/19 04:46:52  mwg
 * Added bits for fax/data calling tones
 *
 * Revision 1.168  1998/12/17 02:46:10  scott
 * Removed overlay-related commands/statuses and added
 * kSetTrainingDelayReductionCmd
 *
 * Revision 1.167  1998/12/12 03:17:42  scott
 * Added overlay commands and statuses
 *
 * Revision 1.166  1998/12/02 05:34:23  mwg
 * Fixed a problem with bong tone detection
 *
 * Revision 1.165  1998/11/26 00:22:44  yura
 * Added two more log data types: modulatorInputData & modulatorOutputData
 *
 * Revision 1.164  1998/11/19 03:08:04  mwg
 * Added kSetCallProgressParamsCmd
 *
 * Revision 1.163  1998/11/18 23:00:03  liang
 * Added a separate command kLoopbackTestAutoRespEnableCmd to enable or disable
 * the loopback test auto respond feature when the modem is already on-line.
 *
 * Revision 1.162  1998/11/13 20:50:21  scott
 * SoftModemInternalStatusHandler is now SM_DECL as well
 *
 * Revision 1.161  1998/11/13 20:42:25  scott
 * Added SM_DECL type to entrypoint functions
 *
 * Revision 1.160  1998/11/13 03:02:54  scott
 * Added SoftModemTimer prototype.
 * Also include V.8bis types if AT_COMMANDS_V8BIS is defined.
 *
 * Revision 1.159  1998/11/12 01:22:46  scott
 * Increased number of AT registers to 46
 *
 * Revision 1.158  1998/11/05 22:35:18  yura
 * Added two more S-registers
 *
 * Revision 1.157  1998/11/05 03:09:54  mwg
 * Added kLapmRetryFailed to the list of LAPM errors
 *
 * Revision 1.156  1998/11/05 00:13:20  liang
 * Add new connectionInfo status kLoopbackSelfTestNewErrs to report
 * new bit errors whenever it happens.
 *
 * Revision 1.155  1998/11/04 07:11:33  mwg
 * Moved declaration for SoftModemATPrintf() to SoftModem.h
 *
 * Revision 1.154  1998/10/29 07:24:49  mwg
 * *** empty log message ***
 *
 * Revision 1.153  1998/10/15 02:09:37  luisgm
 * added separate data rate mask for Flex to dataPumpCapabilities structure
 *
 * Revision 1.152  1998/10/14 00:12:15  scott
 * Added kMnpOOBFrameCmd and command.frameSpec
 *
 * Revision 1.151  1998/10/09 02:19:22  luisgm
 * added FlexV8bisStruct member to dataPumpCapabilities struc to store flex v8bis info, added define for kFlexSkipV8bis
 *
 * Revision 1.150  1998/10/06 19:36:33  mwg
 * Limited 56K rates to 53K
 *
 * Revision 1.149  1998/10/03 03:43:38  ilyas
 * Added status codes for Audio
 *
 * Revision 1.148  1998/10/01 02:03:17  mwg
 * Added external pulse dialer option
 *
 * Revision 1.147  1998/09/30 01:44:26  mwg
 * Added new functions SoftModemGetWriteBufferSize() & SoftModemGetReadBufferSize()
 *
 * Revision 1.146  1998/09/22 03:44:38  scott
 * Added ALWAYS_LONG_ALIGN() macro
 *
 * Revision 1.145  1998/09/21 21:49:22  scott
 * Added logDataCodes for mnpDecoder(Input/Output)Data
 *
 * Revision 1.144  1998/08/31 22:57:21  luisgm
 * added constants for Flex data rates + kFlexEventTRN2AFinished
 *
 * Revision 1.143  1998/08/18 05:09:53  mwg
 * Increased AT command buffer size to 128
 *
 * Revision 1.142  1998/08/18 03:45:54  ilyas
 * Integrated Audio into V70 test
 *
 * Revision 1.141  1998/08/14 17:46:04  ilyas
 * Integrated Audio and G729a
 *
 * Revision 1.140  1998/08/10 21:42:19  mwg
 * Added space and mark parity
 *
 * Revision 1.139  1998/08/08 03:39:33  scott
 * Moved the C6xDefs and PentiumDefs includes before the internal function
 * prototypes (to permit their redefinitions)
 *
 * Revision 1.138  1998/08/07 20:37:27  yura
 * Added new S-register for &T commands
 *
 * Revision 1.137  1998/08/01 05:22:09  mwg
 * Implemented split memory model
 *
 * Revision 1.136  1998/07/22 02:12:22  liang
 * Added self test mode for loopback test.
 *
 * Revision 1.135  1998/07/21 01:19:03  liang
 * Changed loopback test command parameter interface to use regular modeSpec.
 *
 * Revision 1.134  1998/07/18 03:52:10  liang
 * Added V54 loop 2 test for V22.
 *
 * Revision 1.133  1998/07/15 02:45:03  mwg
 * Added new connection info code: kPCMSpectralShapingBits
 *
 * Revision 1.132  1998/07/15 00:18:48  liang
 * Add special turn off command for V34 fax to handle different turn off procedures.
 *
 * Revision 1.131  1998/07/13 22:19:49  liang
 * Add V8 CI detection status and ANSam disable aux feature.
 *
 * Revision 1.130  1998/07/08 17:09:13  scott
 * Added USE_LONG_ALIGN; support for 6 and PentiumDefs.h files
 *
 * Revision 1.129  1998/07/03 23:28:13  mwg
 * Added Fax Class 2 defines
 *
 * Revision 1.128  1998/07/03 23:17:33  mwg
 * Insuread command/status structures are long aligned
 *
 * Revision 1.127  1998/06/23 16:48:01  mwg
 * Fixed a longstanding problem typical for Win95 VxD: whenever new
 * VxD is intalled the confuguration profile may not match the old one but
 * since the crc is correct it is still being downloaded. To avoid the problem
 * a crc for the version number was added to avoid confusion between profiles
 * of different versions.
 *
 * Revision 1.126  1998/06/19 21:04:06  liang
 * Add auxiliary feature bit kV90ServerNotDetSbarAfterJdbarFix.
 *
 * Revision 1.125  1998/06/11 22:48:14  liang
 * Add kPCM28000bpsShift constant.
 *
 * Revision 1.124  1998/06/05 22:11:51  liang
 * New V90 DIL works through data mode.
 *
 * Revision 1.123  1998/06/01 23:03:41  liang
 * Add v90RcvdDilDiffData logging.
 *
 * Revision 1.122  1998/06/01 21:24:38  mwg
 * Changed some of the names.
 *
 * Revision 1.121  1998/05/13 04:55:22  mwg
 * Now passing the number of spectral shaping bits in aux features
 *
 * Revision 1.120  1998/05/13 02:53:13  liang
 * Add field "value" to command param structure.
 *
 * Revision 1.119  1998/05/12 04:42:23  mwg
 * Replaced some of the status messages
 *
 * Revision 1.118  1998/05/11 23:36:10  mwg
 * Added 8000Hz symbol rate to the map
 *
 * Revision 1.117  1998/05/05 04:28:39  liang
 * V90 works up to data mode first version.
 *
 * Revision 1.116  1998/04/21 09:36:45  mwg
 * Fixed a few problems for 16Khz and added 32Khz.
 *
 * Revision 1.115  1998/04/17  22:33:54  liang
 * Added V90 DIL for mu-law PCM.
 *
 * Revision 1.114  1998/04/15  22:36:39  mwg
 * Added new parameters to kDialCmd to allow individual control of each
 * DTMF group attenuation.
 *
 * Revision 1.113  1998/04/15 18:16:22  ilyas
 * Integrated V.8bis and changed coding of LinkLayerType to bitMap
 *
 * Revision 1.112  1998/04/15 07:59:06  mwg
 * Added new status codes for V.90
 *
 * Revision 1.111  1998/04/11 00:29:16  mwg
 * Fixed the warnings which appeared when Irix builds were upgraded to
 * gcc 2.8.1
 *
 * Revision 1.110  1998/04/11  00:25:01  ilyas
 * More V.70 statuses
 *
 * Revision 1.109  1998/04/10 23:29:31  mwg
 * Added new field to capabilities: dataRates56K
 *
 * Revision 1.108  1998/04/09 02:02:56  mwg
 * Added status for Ja detection.
 *
 * Revision 1.107  1998/04/03 02:05:30  ilyas
 * More V.70 commands added
 *
 * Revision 1.106  1998/04/02 06:15:39  mwg
 * Added coding type (Mu-law/A-law) status reporting.
 *
 * Revision 1.105  1998/03/30 09:53:57  mwg
 * Added definition for k56Flex modulation for future use.
 *
 * Revision 1.104  1998/03/27  17:56:09  ilyas
 * Added definitions for V.70
 *
 * Revision 1.103  1998/03/26 23:29:04  liang
 * Added first version of IMD estimation.
 *
 * Revision 1.102  1998/03/20  04:37:26  mwg
 * Increased the size of the nominal variance to 32 bit.
 *
 * Revision 1.101  1998/03/06 01:22:04  yura
 * Improved Win95 VxD segmentation handling
 *
 * Revision 1.100  1998/03/06  01:06:18  liang
 * Add initial version of V90 phase 1 and 2.
 *
 * Revision 1.99  1998/03/05  23:42:22  mwg
 * (hxl) Implemented enable/disable call waiting command.
 *
 * Revision 1.98  1998/02/26  06:13:06  mwg
 * Increased the number of AT S-registers to account for newly introduced
 * S9 and S10.
 *
 * Revision 1.97  1998/02/25  18:18:25  scott
 * Added v42bisCycleCount for V42BIS_THROUGHPUT_CONTROL
 *
 * Revision 1.96  1998/02/24 05:31:20  mwg
 * Added stuff required by international version of AT command processor.
 *
 * Revision 1.95  1998/02/17  01:14:10  scott
 * Reenabled sys/types.h for Linux builds
 *
 * Revision 1.94  1998/02/16 22:32:23  scott
 * Changed copyright notice
 *
 * Revision 1.93  1998/02/16 22:17:44  scott
 * Turned off include of sys/types.h for normal builds
 *
 * Revision 1.92  1998/02/16 21:53:28  scott
 * Exclude sys/types.h for another compiler
 *
 * Revision 1.91  1998/02/09 18:24:10  scott
 * Fixed ComplexShort type to work around bugs in MS and GreenHill compilers
 *
 * Revision 1.90  1998/01/27 01:37:36  mwg
 * Added new log identifier for pcm infidelity data.
 *
 * Revision 1.89  1998/01/22  19:49:32  liang
 * Add auxFeature bit kFaxV34HDXAllowAsymCtrlChan.
 *
 * Revision 1.88  1998/01/21  02:32:01  liang
 * Add more V34 half duplex training progress codes.
 *
 * Revision 1.87  1997/12/23  03:28:25  liang
 * Add more half duplex V34 related constants.
 *
 * Revision 1.86  1997/12/18  19:38:50  scott
 * Added agcData log type.
 * Added kDisableFaxFastClearDown demod capability
 *
 * Revision 1.85  1997/12/18 06:02:45  mwg
 * Added a function to reenable DC offset tracking.
 *
 * Revision 1.84  1997/12/17  22:46:30  mwg
 * Minor modifications to X2 escape status reporting.
 *
 * Revision 1.83  1997/12/16  06:49:45  mwg
 * Implemented proper data rate reporting for PCM modem.
 *
 * Revision 1.82  1997/12/13  06:11:08  mwg
 * Added X2 interface hooks
 *
 * Revision 1.81  1997/12/02 06:21:33  mwg
 * Implemented kSetATRegister command.
 *
 * Revision 1.80  1997/11/27  02:11:41  liang
 * Add code for half duplex V34 control channel.
 *
 * Revision 1.79  1997/11/19  19:52:48  guy
 * Added constant to define V.34 half duplex operation
 *
 * Revision 1.78  1997/10/24 05:15:53  scott
 * Added AGC and phase hit recovery to demodCapabilities
 *
 * Revision 1.77  1997/10/01 02:47:50  liang
 * Add PCM interface.
 *
 * Revision 1.76  1997/09/29  15:48:04  yura
 * Added #pragma statement for W95 Vxd
 *
 * Revision 1.75  1997/09/18 20:32:39  scott
 * Do not include VxD support files if GENERATE_DEPENDENCIES is defined.
 *
 * Revision 1.74  1997/09/18 12:40:55  yura
 * Removed #ifdef statments to be more robust
 *
 * Revision 1.73  1997/09/17 17:32:41  scott
 * Do not include sys/types.h for 6
 *
 * Revision 1.72  1997/08/08 00:53:48  mwg
 * Added fields for LAP-M frames printout.
 * Added fields in auxFeatures to pass preemphasis filter parameters
 * to V.34 phase 3 when doing PTT testing.
 *
 * Revision 1.71  1997/08/06  03:41:45  yura
 * Added a few includes and defines needed by Win 95 driver.
 *
 * Revision 1.70  1997/08/05  03:22:10  liang
 * Add equalizer center tap adjustment calculation related constants.
 *
 * Revision 1.69  1997/07/29  02:44:19  mwg
 * Added new field to dataPumpCapabilities structure. This field is not
 * yet exposed to external interface and currently is only used to
 * enable PTT testing.
 * Added new commands: kStartDataModemPTTTestCmd & kStartDataModemLoopbackTestCmd
 *
 * Revision 1.68  1997/07/22  22:05:10  liang
 * Change sample rate setup as a normal command.
 *
 * Revision 1.67  1997/07/21  23:23:30  liang
 * Define SoftModemSetSampleRate as null when SAMPLE_RATE_CONVERSION is not defined.
 *
 * Revision 1.66  1997/07/21  22:38:36  liang
 * Change sample rate converter structure so that sample rate can be changed
 * on the fly (at very begining) to either 8KHz or 9600Hz.
 *
 * Revision 1.65  1997/07/21  20:22:01  mwg
 * Added statusInfoData to the log identifiers.
 *
 * Revision 1.64  1997/07/16  20:40:07  scott
 * Added multitone monitor fields
 *
 * Revision 1.63  1997/07/10 02:31:08  mwg
 * 1. Added kRxFrameHDLCFlags detected status for the
 *    framingInfo.
 * 2. Added kLapmMNPFrameDetected status to lapmStatusCode.
 * 3. Increased the number of AT registers to 35
 * 4. Modified LinkLayerSpec structure in modemCommandStruc
 *    to provide the initial values of rxDataRate &
 *    txDataRate and RT delay for the cases when
 *    link layer is started *after* the data connection
 *    is established and the status snooper is unable
 *    to determine the rates and RT delay.
 * 5. Added a few extra *empty* constant definitions for
 *    disabled features.
 *
 * Revision 1.62  1997/07/02  19:15:05  scott
 * Added bits for Bel103 & Bel212 modulations.
 *
 * Revision 1.61  1997/07/02 05:15:16  mwg
 * Added MNP code.
 *
 * Revision 1.60  1997/07/01  23:52:48  mwg
 * Modified the record test setup to log and use all the commands.
 *
 * Revision 1.59  1997/06/25  19:11:26  mwg
 * 1. Added new framingInfoCode values for Async framing error reporting;
 * 2. Added a substructure to pass serial data format for kSetDTERate cmd;
 *
 * Revision 1.58  1997/05/28  02:05:08  liang
 * Add PCM modem phase 2 codes.
 *
 * Revision 1.57  1997/05/12  21:55:08  liang
 * Add call waiting tone detector module.
 *
 * Revision 1.56  1997/03/21  23:50:08  liang
 * Added initial version of V8bis module to CVS tree.
 *
 * Revision 1.55  1997/03/19  18:35:05  mwg
 * Changed copyright notice.
 *
 * Revision 1.54  1997/03/11  11:11:45  mwg
 * Added code to report V42bis statistics.
 *
 * Revision 1.53  1997/03/04  06:21:08  mwg
 * Added logging of most commands.
 *
 * Revision 1.52  1997/02/28  23:45:13  liang
 * Added training progress status report kPhaseJitterDeactivated.
 *
 * Revision 1.51  1997/02/28  22:23:22  mwg
 * Implemented the following features:
 * - Cleardown for fax modulations V.27, V.29 V.17
 * - Rockwell compatible bitmap report (needed by a customer)
 *
 * Revision 1.50  1997/02/28  03:05:31  mwg
 * Added more logging data types.
 *
 * Revision 1.49  1997/02/27  05:28:58  mwg
 * Added RxFrameOK report.
 *
 * Revision 1.48  1997/02/27  01:48:53  liang
 * Add kV8MenuDataWord1 and kV8MenuDataWord2 connectionInfo status.
 *
 * Revision 1.47  1997/02/24  02:30:27  mwg
 * Added new log  data: predictorErrData
 *
 * Revision 1.46  1997/02/22  03:00:22  liang
 * Add echoCancelledSignalData.
 *
 * Revision 1.45  1997/02/21  01:26:42  liang
 * Add six more bits for the Demodulator capabilities to deal with 2nd order
 * time tracking & PLLs, as well as shorter NEEC & PFEEC, and front end HBF.
 *
 * Revision 1.44  1997/02/17  03:09:00  mwg
 * Added LAPM statistics printout.
 *
 * Revision 1.43  1997/02/04  08:38:47  mwg
 * Added dc cancelled samples printout.
 *
 * Revision 1.42  1997/01/29  21:40:28  mwg
 * Changed the way timers work: now time is passed as Q4 ms instead of ticks.
 * Completed the 8KHz front end implementation.
 * Got rid of kSamplesPerSecond constant.
 *
 * Revision 1.41  1997/01/24  07:13:50  mwg
 * Added new statuses for automoder.
 *
 * Revision 1.40  1997/01/23  02:03:08  mwg
 * Replaced old sample rate conversion with the newer one.
 * Still has to resolve the automoding issue.
 *
 * Revision 1.39  1997/01/21  00:55:04  mwg
 * Added 8KHz front end functionality.
 *
 * Revision 1.38  1996/11/13  00:30:55  liang
 * Add kAutoLoadReductionEnabled to demodCapabilities so that PFEEC, FEEC, IEEC
 * can be disabled automatically, but for worst processor loading test they
 * won't be disabled when this bit is not set.
 *
 * Revision 1.37  1996/11/07  23:07:18  mwg
 * Rearranged global variables to allow V.17 short training.
 *
 * Revision 1.36  1996/09/17  23:55:05  liang
 * Change kMaxDataBlockSize from 16 to 24 to handle high data rates.
 *
 * Revision 1.35  1996/09/05  19:43:39  liang
 * Removed caller ID error status code kCallerIDUnknownMessageType, and
 * added caller ID status codes kCallerIDUnknownMessage & kCallerIDWholeMessage.
 * Changed the callerIDStatus report structure.
 *
 * Revision 1.34  1996/08/29  00:36:57  liang
 * Added kLapmTxFrameStatus and kLapmRxFrameStatus.
 *
 * Revision 1.33  1996/08/27  22:56:01  liang
 * Added kResetHardware status code.
 *
 * Revision 1.32  1996/08/23  23:35:35  liang
 * Add kATDebugStatus and function SoftModemGetHybridDelay.
 *
 * Revision 1.31  1996/08/22  01:13:19  yg
 * Added AT command processor.
 *
 * Revision 1.30  1996/08/12  21:46:47  mwg
 * Added code to report capabilities.
 *
 * Revision 1.29  1996/08/10  01:59:59  mwg
 * Added report of the sent rate sequence;
 *
 * Revision 1.28  1996/08/07  22:15:02  mwg
 * Added new status reports:
 * kRemoteFreqOffset
 * kIEECDeactivated
 * kPFEECDeactivated
 *
 * Revision 1.27  1996/06/27  05:15:48  mwg
 * Added V.24 circuit status.
 *
 * Revision 1.26  1996/06/27  02:12:43  mwg
 * Cleaned the code.
 *
 * Revision 1.25  1996/06/20  23:57:30  mwg
 * Added new training progress status.
 *
 * Revision 1.24  1996/06/18  21:13:50  mwg
 * Added trellis MSE data logging.
 *
 * Revision 1.23  1996/06/12  02:31:10  mwg
 * Added new type: VeryLong
 *
 * Revision 1.22  1996/06/08  22:15:39  mwg
 * Added new status report: kCleardownStarted
 * Added new field for the features: kV34bisEnabled
 *
 * Revision 1.21  1996/05/31  00:29:11  liang
 * Add feature bit kV34ExtraINFOPreamble.
 *
 * Revision 1.20  1996/05/30  23:28:31  mwg
 * Replaced enums with #defines
 *
 * Revision 1.19  1996/05/25  00:38:27  mwg
 * Added kProjectedDataRate training progress report.
 *
 * Revision 1.18  1996/05/24  23:27:15  mwg
 * Added mode status codes.
 *
 * Revision 1.17  1996/05/10  05:39:59  liang
 * Move the includes for DEBUG inside "ifndef SoftModemTypes" so that
 * cap build won't break.
 *
 * Revision 1.16  1996/05/08  01:49:34  mwg
 * Added capability to setup auxiliary data channel handlers.
 *
 * Revision 1.15  1996/05/07  22:51:08  liang
 * Added group delay estimation and improved symbol rate selection process.
 *
 * Revision 1.14  1996/05/06  06:49:09  mwg
 * Fixed linux problems.
 *
 * Revision 1.13  1996/05/02  08:40:16  mwg
 * Merged in Chromatic bug fixes.
 *
 * Revision 1.12  1996/05/02  02:26:21  mwg
 * Added code to implement dozing functionality for v.34.
 *
 * Revision 1.11  1996/05/01  22:43:13  mwg
 * Added new command: kDozeCmd;
 *
 * Revision 1.10  1996/05/01  19:20:16  liang
 * Add command codes kInitiateRetrainCmd and kInitiateRateRenegotiationCmd.
 *
 * Revision 1.9  1996/04/25  01:12:37  mwg
 * Added new flag: rapid preliminary EC training.
 *
 * Revision 1.8  1996/04/20  02:26:22  mwg
 * Added preliminary far-end echo support
 *
 * Revision 1.7  1996/04/15  23:26:16  mwg
 * Changed flag definitions for v34 modem.
 *
 * Revision 1.6  1996/04/04  02:35:50  liang
 * Change kCid from 0x0080 to 0x0004 (0x0080 is defined as kV32).
 *
 * Revision 1.5  1996/03/08  23:07:01  mwg
 * Added name for the struct.
 *
 * Revision 1.4  1996/03/02  00:59:27  liang
 * Added typedef for V34CodingParameters structure.
 *
 * Revision 1.3  1996/02/27  02:28:31  mwg
 * Fixed a bug in kLapmLongADPEnabled definition.
 *
 * Revision 1.2  1996/02/19  23:50:59  liang
 * Removed compressionSetup parameter from the link layer command structure.
 *
 * Revision 1.1.1.1  1996/02/14  02:35:13  mwg
 * Redesigned the project directory structure. Merged V.34 into the project.
 *
 * Revision 1.5  1996/01/15  23:26:04  liang
 * Change the softmodem command structure name from SoftwareModemCommand
 * to SoftwareModemCommandParameters.
 *
 *****************************************************************************/
#ifndef	SoftModemPh
#define	SoftModemPh

/****************************************************************************/
/*	1.	Type definitions.													*/
/*																			*/
/*	1.1	General types														*/
/****************************************************************************/

#ifndef SM_DECL
#define SM_DECL
#endif

#ifdef __VxWORKS__
#include <types/vxTypesOld.h>
#endif

#if defined(__KERNEL__)
#include "../BcmOs.h"
#endif

#ifdef DEBUG
/* We have to define __wchar_t for Linux	*/
#if defined __linux__ && !defined _NO_WHCAR_DEF_
typedef	long int __wchar_t;
#endif
#if !defined(__KERNEL__) && !defined(_CFE_) && !defined(__ECOS)
#include <stdio.h>
#include <stdlib.h>
#endif

#if defined(__linux__) || defined (__unix__) || defined (__unix) || (defined (__mips__) && !defined(_CFE_) && !defined(__ECOS) && !defined(VXWORKS) && !defined(TARG_OS_RTEMS))/* enable if necessary, but not for dos-based builds */
#include <linux/types.h>
#endif


#endif	/* DEBUG */

#if defined(W95_DRIVER) 
#pragma code_seg("_LTEXT", "LCODE")
#pragma data_seg("_LDATA", "LCODE")
#pragma const_seg("_LDATA", "LCODE")
#pragma bss_seg("_LDATA", "LCODE")
#pragma pack(1)
#endif /* W95_DRIVER */

#ifndef SoftModemTypes
#include "SoftModemTypes.h"
#endif	/* SoftModemTypes */


typedef struct
	{
	schar x, y;
	} ComplexByte;

typedef struct
	{
	uchar numerator;
	uchar denominator;
	} Ratio;

#ifdef PEGASUS
typedef union
	{
	struct
		{
		short x, y;
		};
	
	int foo;
	} ComplexShort;
#else
typedef struct
	{
	short x, y;
#ifdef GREENHILL
	int a[0];
#endif
	} ComplexShort;
#endif

typedef struct
	{
	int x, y;
	} ComplexLong;

typedef struct
	{
	ushort 	x0, x1, x2;
	short	x3;
	} VeryLong;

#ifdef _MSC_VER
#define	LONGLONG    __int64
#else	
#define LONGLONG    long long
#endif

typedef union
	{
	struct
		{
		uchar number;
		uchar defaultValue;			/* default value */			
		uchar maxValue;			/* max allowed value */
		uchar minValue;			/* should be greater then maxValue to make reg readonly */
		} param;
	int alignment;
	} SRegisterDefinition;

#define MacroPaste2(a,b) a##b
#define MacroPaste(a,b) MacroPaste2(a,b)
#define ALWAYS_LONG_ALIGN() int MacroPaste(ALIGNMENT,__LINE__);

#ifdef USE_LONG_ALIGN
#define LONG_ALIGN() ALWAYS_LONG_ALIGN()
#else
#define LONG_ALIGN()
#endif

typedef	uint	bitMap;

typedef	int	pace;
#define kStop		0
#define kVerySlow	1
#define kSlow		2
#define kMedium		3
#define kFast		4


/****************************************************************************/
/*	1.	Type definitions.													*/
/*																			*/
/*	1.2	Modem specific types												*/
/****************************************************************************/

typedef	int directionType;
#define kXmt	0
#define kRcv	1
#define kXmtRcv	2


#define	originating		kXmt
#define	answering		kRcv
#define	kOrg			kXmt
#define	kAns			kRcv
#define	kOrgAns			kXmtRcv

#define	ORIGINATING		originating
#define	ANSWERING		answering

typedef	int	pcmCodingType;
#define	kMuLawPCM	0
#define	kALawPCM	1

#define	kMuLawPCMScaleShift		2
#define	kALawPCMScaleShift		3

/* link layer and framer share defines */
typedef	bitMap	framerType;
typedef	bitMap	linkLayerType;
#define kNoFramer	0
#define kSync		0x00000001
#define kAsync		0x00000002
#define kHDLC		0x00000004
#define kLapm		0x00000008
#define kMnp		0x00000010
#define kV70		0x00000020
#define kSAM		0x00000040

	
typedef	bitMap	modulationMap;
typedef	bitMap	symbolRateMap;
typedef	bitMap	dataRateMap;
typedef	bitMap	featureMap;
typedef	bitMap	breakType;

typedef	bitMap	audioType;
#define	kRawAudio		0
#define	kAudioG729A		1
#define	kAudioG729		2
#define	kAudioG723		3


#ifndef ADSL_MODEM
typedef int	modemStatusCode;
#endif
	/* Information status Codes: 1-31		*/
#define kSetSampleRate				1
#define kModulationKnown			2
#define kRxSymbolRate				3
#define kTxSymbolRate				4
#define kRxCarrierFreq				5
#define kTxCarrierFreq				6
#define kTxPreemphasisFilter		7
#define kTxPowerAdjustment			8
#define kRemoteTxPreemphasisFilter	9
#define kRemoteTxPowerAdjustment	10
#define kRxRateKnown				11
#define kTxRateKnown				12
#define kRxDataModeActive			13
#define kTxDataModeActive			14
#define kTxSignalCompleted			15
#define kDTMFSignalDetected			16
#define kModemSignalDetected		17
#define kCallProgressSignalDetected	18
#define kCustomSignalDetected		19
#define kFaxPreambleDetected		20
#define kV24CircuitStatusChange		21
#define kHookStateChange			22
#define kCallWaitingToneDetected	23
#define kMultiToneSignalDetected	24
#define kPulseShuntStateChange		25
#define kRingFrequency              26


	/* Warning status Codes:		32-64	*/
#define kError						32
#define kV34Exception				33
#define kClearDownLocal				34
#define kClearDownRemote			35
#define kCarrierPresent				36
#define kCarrierLost				37
#define kRetrainingLocal			38
#define kRetrainingRemote			39
#define kRateRenegotiationLocal		40
#define kRateRenegotiationRemote	41
#define kFallbackStarted			42
#define kFallForwardStarted			43
#define kCleardownStarted			44
#define kIllegalCommand				45
	
	/* Auxiliary status Codes:	64-..	*/	
#define kTrainingProgress			64
#define kConnectionInfo				65
#define kDialerStatus				66
#define kFramingInfo				67
#define kBreakReceived				68
#define kLapmStatus					69
#define kLapmParameter				70
#define kV42bisStatus				71
#define kCallerIDStatus				72
#define kIOStatus					73
#define kCapabilitiesStatus			74
#define kSpeakerStatus				75
#define kATProfileChanged			76
#define kATDebugStatus				77
#define	kResetHardware				78
#define	kV8bisStatus				79
#define kMnpStatus					80
#define kMnpParameter				81
#define kV70Status					82
#define kV70Parameter				83
#define kFaxClass2Status			84
#define kAudioStatus				85
#define kAudioParameter				86
#define kOverlayStatus				87
#define kCallerIDCircuitStatus		88
#define kV80Status					89
#define kV80Parameter				90
#define kLocalCountryChanged		91
#define kDTERateChanged				92
#define kATResponse					93
#define kFramerConfigured			94
#define kA8RStatus					95
#define kA8TStatus					96
#define	kVersionStatus				97

	/* Testing status codes:	128-...	*/
	/* These statuses are generated by modem test suit	*/
#define kTestFinished				128
#define kConnectivityTestFinished	129
#define kTestCheckSum				130
#define kLogFileControl				131
#define kTestAtmVcFinished			132
#define kTestClearEocFinished		133
#define kTestG997Finished			134

typedef int	modemErrorCode;
#define kNoError				0
#define kErrorTimerExpired		1
#define kErrorNoSReceived		2
#define kErrorNoSbarReceived	3


typedef int	dialerStatusCode;
#define kDialCompleted				0
#define kNoDialToneDetected			1
#define kBongToneDetected			2
#define kNoBongToneDetected			3
#define kErrorIllegalDialModifier	5
#define kDialStarted				6
#define kExternalPulseDialDigit		7


typedef int	framingInfoCode;
#define kRxFrameOK					0
#define kRxFrameTooLong				1
#define kRxFrameCRCError			2
#define kTxFrameUnderrun			3
#define kRxFrameOverrun				4
#define kRxFrameAborted				5
#define kRxFrameParityError			6
#define kRxFrameFormatError			7
#define	kRxFrameHDLCFlagsDetected	8


typedef int	IOStatusCode;
#define kRxDataReady		0
#define kRxBufferOverflow	1
#define kTxSpaceAvailable	2
#define kTxBufferEmpty		3

typedef int	capabilitiesStatusCode;
#define kSymbolRates				0
#define kDataRates					1
#define kFeatures					2
#define kDemodCapabilities			3
#define kRateThresholdAdjustment	4
#define kXmtLevel					5
#define kHybridDelay				6
#define kAuxFeatures				7


typedef int	A8TStatusCode;
#define kA8TFinished				0

typedef int	callerIDStatusCode;
#define kCallerIDError					0
#define kCallerIDChannelSeizureReceived	1
#define kCallerIDMarkSignalReceived		2
#define kCallerIDTime					3
#define kCallerIDTelnum					4
#define kCallerIDName					5
#define kCallerIDEnd					6
#define kCallerIDUnknownMessage			7
#define kCallerIDWholeMessage			8


typedef int	callerIDErrorCode;
#define kCallerIDNoError			0
#define kCallerIDMarkSignalError	1
#define kCallerIDTooManyMarkBits	2
#define kCallerIDMessageTooLong		3
#define kCallerIDChecksumError		4


typedef int	connectionInfoCode;
#define kRTDelay			1
#define kRxSignalLevel		2
#define kTimingOffset		3
#define kFreqOffset			4
#define kPhaseJitter		5
#define kSNR				6
#define kNearEchoLevel		7
#define kSER				8
#define kNearEndDelay		9
#define kFarEchoLevel		10
#define kL1L2SNRDifference	11
#define	kDCOffset			12
#define	kTotalRxPower		13
#define	kRemoteFreqOffset	14
/* obsolete	#define	kV8MenuDataWord1	15 */
/* obsolete	#define	kV8MenuDataWord2	16 */
#define	kPCMP2AnalogDetSNR	17
#define	kPCMP2DigitalDetSNR	18
#define	kPCMP2RBSDetSNR		19
#define	kEqCenterTapOffset	20
#define	kPCMPadValue		21
#define	kPCMRBSMap			22
#define	kPCMCodingType		23
#define	kPCMSpectralShapingBits			24
#define	kLoopbackSelfTestResult			25
#define	kEyeQuality						26
#define	kLoopbackSelfTestNewErrs		27
#define kV34EqlLengthStatus 28
#define kV34EqlOffsetStatus 29
#define	kV8CallMenuData		30
#define	kV8JointMenuData	31
#define kPCMClientIeecLengthStatus 32
#define kPCMClientIeecOffsetStatus 33
#define	kSeamlessRateChange	34

typedef int	trainingProgressCode;
#define kPeriodicalSignalDetected		0
#define kPhaseReversalDetected			1
#define kSignalStartDetected			2
#define kSignalEndDetected				3
#define kSSignalDetected				4
#define kSbarSignalDetected				5
#define kJ4SignalDetected				6
#define kJ16SignalDetected				7
#define kJprimeSignalDetected			8
#define kMPSignalDetected				9
#define kMPprimeSignalDetected			10
#define kMPSignalSent					11
#define kMPprimeSignalSent				12
#define kRateSignalDetected				13
#define kESignalDetected				14
#define kRateSignalSent					15

#define	kAutomodingTryModulation		16
#define	kAutomodingCompleted			17
#define	kRCFaxBitMapStatus				18
	
#define kV8CIDetected					19
#define kV8ANSToneDetected				20
#define kV8ANSamDetected				21
#define kV8CMDetected					22
#define kV8JMDetected					23
#define kV8CJDetected					24
#define kV8Finished						25
	
#define kV34Phase2Started				26
#define kV34Phase2INFOSequenceDetected	27
#define kV34Phase2NearEndEchoDetected	28
#define kV34Phase2L1Receiving			29
#define kV34Phase2L2Receiving			30
#define kV34Phase2Finished				31
#define kV34Phase3Started				32
#define kV34Phase3Finished				33
#define kV34Phase4Started				34
#define kV34Phase4Finished				35
#define kV34DecoderParameters			36
#define kV34EncoderParameters			37

#define kMaxLocalRxDataRate				38
#define kMaxLocalTxDataRate				39
#define kMaxRemoteRxDataRate			40
#define kMaxRemoteTxDataRate			41
#define kProjectedDataRate				42
#define kFEECDeactivated				43
#define kIEECDeactivated				44
#define kPFEECDeactivated				45
#define kPhaseJitterDeactivated			46

#define	kPCMP2DetectedDigitalConnection	47
#define	kPCMP2DetectedRBS				48
#define	kX2DetectedPhase1Escape			49

#define kStarted1200BpsTraining			50
#define kStarted2400BpsTraining			51
#define kUnscrambledOneDetected			52
#define kScrambled1200BpsOneDetected	53
#define kScrambled2400BpsOneDetected	54
#define kV22BisS1Detected				55
#define	kV22InitiateLoop2Test			56
#define	kV22RespondLoop2Test			57
#define	kV22Loop2TestAlt01Detected		58

#define	kDataModemLoop1TestStarted		59
#define	kDataModemLoop1TestFinished		60
#define	kDataModemLoop2TestStarted		61
#define	kDataModemLoop2TestFinished		62
#define	kDataModemLoop3TestStarted		63
#define	kDataModemLoop3TestFinished		64
#define	kDataModemSelfLoopTestEnabled	65

#define kPCMPhase3Started				70
#define kPCMPhase3Finished				71
#define kPCMPhase4Started				72
#define kPCMPhase4Finished				73

#define	kV90JaSignalDetected			74		
#define	kV90JdSignalDetected			75		
#define	kV90JdPrimeSignalDetected		76		
#define	kV90RSignalDetected				77		
#define	kV90RBarSignalDetected			78	
#define	kV90CPSignalDetected			79	

#define	kV90CPtSignalSent				80
#define	kV90CPSignalSent				81
#define	kV90CPprimeSignalSent			82


#define	kV34SeamlessRateChangeRequestSent		83
#define	kV34SeamlessRateChangeUpdateSent		84
#define	kV34SeamlessRateChangeRequestReceived	85
#define	kV34SeamlessRateChangeUpdateReceived	86
#define	kV34SeamlessRateChangeUpdateTimeout		87

#define kV90JaSignalAcknowledged				88

#define	kV34HCtrlChanPPhDetected		100
#define	kV34HCtrlChanMPhDetected		101
#define	kV34HCtrlChanRatesKnown			102
#define	kV34HDXCtrlChanBinary1Detected	103
#define	kV34HDXPhase3Started			104
#define	kV34HDXPhase3Finished			105
#define	kV34HDXPrimChanBinary1Detected	106
#define kFlexEventTRN2AFinished         107

#define kV32RanginigStarted				108
#define kV32RangingStarted				108
#define kV32RanginigFinished			109
#define kV32RangingFinished				109


typedef int	lapmStatusCode;
#define kLapmDisconnected			0	/* LAPM disconnected */
#define kLapmConnected				1	/* LAPM is connected */
#define kLapmV42ODPDetected			2	/* LAPM ODP is detected	*/
#define kLapmV42ADPDetected			3	/* LAPM V.42 ADP is detected	*/
#define kLapmUnknownADPDetected		4	/* LAPM Unsupported ADP is detected	*/
#define kLapmTimeout				5	/* LAPM Timeout		*/
#define	kLapmMNPFrameDetected		6	/* LAPM detected MNP frame	*/
#define kLapmDPDetectionTimedOut	7	/* LAPM Unsupported ADP is detected	*/
#define kLapmError					8	/* LAPM Error	*/
#define kLapmTestResult				9	/* LAPM loopback test result */
#define	kLapmTxFrameStatus			10
#define	kLapmRxFrameStatus			11
#define	kLapmTxStatistics			12
#define	kLapmRxStatistics			13

typedef int	lapmTakedownReason;
#define kLapmRemoteDisconnect	0
#define kLapmLocalDisconnect	1
#define kLapmCannotConnect		2
#define kLapmProtocolError		3
#define kLapmCompressionError	4
#define kLapmInactivityTimer	5
#define kLapmRetryFailed		6


typedef int	lapmParameterCode;
#define kLapmXmtK			0
#define kLapmRcvK			1
#define kLapmXmtN401		2
#define kLapmRcvN401		3
#define kLapmTESTSupport	4
#define kLapmSREJSupport	5
#define kLapmCompDir		6
#define kLapmCompDictSize	7
#define kLapmCompStringSize	8


typedef int	lapmErrorCode;
#define kLapmNoError		0
#define kLapmBufferOverflow	1
#define kLapmFrameTooLong	2
#define kLapmBadFrame		3
#define kLapmUnknownEvent	4
/* 6 is reserved for kLapmRetryFailed defined above */


typedef int	lapmTestResultCode;
#define kLapmTestPassed				0
#define kLapmTestRequestIgnored		1
#define kLapmTestAlreadyInProgress	2
#define kLapmTestNotSupported		3
#define kLapmTestFailed				4


typedef int	v42bisStatusCode;
#define kV42bisEncoderTransparentMode	0	/* V.42bis encoder transparent mode active */
#define kV42bisEncoderCompressedMode	1	/* V.42bis encoder compressed mode active */
#define kV42bisDecoderTransparentMode	2	/* V.42bis decoder transparent mode active */
#define kV42bisDecoderCompressedMode	3	/* V.42bis decoder compressed mode active */
#define kV42bisError					4	/* V.42bis error */
#define	kV42bisEncoderStatistics		5
#define	kV42bisDecoderStatistics		6


typedef int	v42bisErrorCode;
#define kV42bisUndefinedEscSequence	0	/* V.42bis undefined escape sequence		*/
#define kV42bisCodewordSizeOverflow	1	/* V.42bis codeword size overflow			*/
#define kV42bisUndefinedCodeword	2	/* V.42bis undefined codeword				*/

typedef int	mnpStatusCode;
#define kMnpDisconnected			0	/* Mnp disconnected */
#define kMnpConnected				1	/* Mnp is connected */
#define kMnpFallback				2	/* Mnp is falling back to buffer mode */
#define kMnpError					3	/* Mnp Error	*/
#define	kMnpTimeout					4	/* Mnp Timeout */
#define	kMnpInvalidLT				5	/* Invalid LT received */
#define	kMnpRetransmitFrame			6
#define	kMnpNack					7
#define	kMnpTxFrameStatus			8
#define	kMnpRxFrameStatus			9
#define	kMnpTxStatistics			10
#define	kMnpRxStatistics			11

typedef int	mnpTakedownReason;
#define kMnpRemoteDisconnect		0
#define kMnpLocalDisconnect			1
#define kMnpCannotConnect			2
#define kMnpProtocolError			3
#define kMnpCompressionError		4
#define kMnpInactivityTimer			5
#define kMnpRetryFailed				6


typedef int	mnpParameterCode;
#define kMnpProtocolLevel			0
#define kMnpServiceClass			1
#define kMnpOptimizationSupport		2
#define kMnpCompressionSupport		3
#define kMnpN401					4
#define kMnpK						5


typedef int	mnpErrorCode;
#define kMnpNoError					0			
#define kMnpBufferOverflow			1
#define kMnpFrameTooLong			2
#define kMnpBadFrame				3
#define kMnpUnknownEvent			4


typedef int	v70StatusCode;
#define kV70Disconnected			0	/* V70 disconnected */
#define kV70Connected				1	/* V70 is connected */
#define kV70Error					2	/* V70 Error	*/
#define	kV70Timeout					3	/* V70 Timeout */
#define kV70ChannelDown             4	/* V70 channel released */
#define kV70ChannelUp               5	/* V70 channel established */
#define kV70AudioChannelDown        6	/* V70 audio channel released */
#define kV70AudioChannelUp          7	/* V70 audio channel established */
#define kV70DataChannelDown         8	/* V70 data channel released */
#define kV70DataChannelUp           9	/* V70 data channel established */
#define kV70OOBChannelDown          10	/* V70 out-of-band channel released */
#define kV70OOBChannelUp            11  /* V70 out-of-band channel established */
#define	kV70TxFrameStatus			12
#define	kV70RxFrameStatus			13
#define	kV70TxStatistics			14
#define	kV70RxStatistics			15
#define	kV70StateTransition			16

typedef int	v70TakedownReason;
#define kV70RemoteDisconnect		0
#define kV70LocalDisconnect			1
#define kV70CannotConnect			2
#define kV70ProtocolError			3
#define kV70CompressionError		4
#define kV70InactivityTimer			5
#define kV70RetryFailed				6


typedef int	v70ParameterCode;
#define kV70SuspendResume	        0
#define kV70CrcLength	            1
#define kV70NumberOfDLCs	        2
#define kV70uIH	                    3

#define kV70LapmXmtK				10
#define kV70LapmRcvK				11
#define kV70LapmXmtN401				12
#define kV70LapmRcvN401				13
#define kV70LapmTESTSupport			14
#define kV70LapmSREJSupport			15
#define kV70LapmCompDir				16
#define kV70LapmCompDictSize		17
#define kV70LapmCompStringSize		18

#define kV70AudioHeader	            20   /* if audio header is present in audio frames */
#define kV70BlockingFactor	        21   /* audio blocking factor (default 1)  */
#define kV70SilenceSuppression      22   /* audio silence suppression */



typedef int	v70ErrorCode;
#define kV70NoError					0			
#define kV70BadFrame				1			

typedef int	audioStatusCode;
#define kAudioFramesLost			0	 /* One or more audio frames were lost */
#define kAudioTxBufferOverflow		1
#define kAudioRxBufferOverflow		2
#define kAudioRxBufferUnderflow		3


typedef int	v80StatusCode;
#define kV80Disconnected			0	/* V80 disconnected */
#define kV80Connected				1	/* V80 is connected */
#define kV80Error					2	/* V80 Error	*/
#define kV80InBandStatus			3	/* V80 in-band SAM status */
#define	kV80TxFrameStatus			12
#define	kV80RxFrameStatus			13
#define	kV80TxStatistics			14
#define	kV80RxStatistics			15

typedef int	v80TakedownReason;
#define kV80RemoteDisconnect		0
#define kV80LocalDisconnect			1

typedef int	v80ErrorCode;
#define kV80NoError					0			
#define kV80BadFrame				1			

typedef int	overlayStatusCode;
#define kOverlayBegin				0	/* DSP has halted */
#define kOverlayEnd					1	/* DSP has received entire overlay */
#define kOverlayElapsedTime			2	/* time elapsed(as viewed by datapump) during overlay */
#define kOverlayRecordingData		3	/* ms of data that we are recording */
#define kOverlayReplayingData		4	/* ms of data that we have replayed so far */
#define kOverlayReplayDone			5	/* playback is done */

/* types for kOverlayRecording/ReplayingData */
#define kOverlayTxData				0
#define kOverlayRxData				1

/*
 * Rockwell faxmodem compatible bitmap (kRCFaxBitMapStatus)
 */
#define	kRCFaxFCD	0x01
#define	kRCFaxP2	0x02
#define	kRCFaxPN	0x04
#define	kRCFaxDCD	0x08
#define	kRCFaxTX	0x10
#define	kRCFaxCTS	0x20


#ifndef ADSL_MODEM
typedef int	modemCommandCode;
#endif
	/* Basic Action commands		00-63		*/
#define kIdleCmd						0
#define kStartFaxModemCmd				1
#define kStartDataModemCmd				2
#define kStartCallProgressMonitorCmd	3
#define kSendTonesCmd					4
#define kStartCallerIDRcvCmd			5
#define kSetLinkLayerCmd				6
#define kSetFramerCmd					7
#define kTestLinkLayerCmd				8
#define kIdleRcvCmd						9
#define kIdleXmtCmd						10
#define kSetStatusHandlerCmd			11
#define kSetEyeHandlerCmd				12
#define kSetLogHandlerCmd				13
#define kSendBreakCmd					14
#define kSendTestCmd					15
#define kDisconnectLinkCmd				16
#define kSetXmtGainCmd					17
#define kStartADSICmd					18
#define kSetHybridDelayCmd				19
#define kCleardownCmd					20
#define kInitiateRetrainCmd				21
#define kInitiateRateRenegotiationCmd	22
#define	kDialToneIndicator				23
#define kSetRxDataHandler				24	/* not used yet */
#define kSetTxDataHandler				25	/* not used yet */
#define kSetAuxRxDataHandler			26
#define kSetAuxTxDataHandler			27
#define kRingIndicatorCmd				28
#define kDTERateIndicatorCmd			29
#define	kStartV8bisCmd					30
#define kSendMultiTonesCmd				31
#define kSetMultiToneParamsCmd			32
#define kSetModemSampleRateCmd			33
#define kStartDataModemPTTTestCmd		34
#define kStartDataModemLoopbackTestCmd	35
#define kRingFrequencyCmd				36
#define kSetCallWaitingDetectorStateCmd	37
#define kV34HDXTurnOffCurrentModeCmd	38
#define	kSetAudioCmd					39
#define	kLoopbackTestAutoRespEnableCmd	40
#define kSetCallProgressParamsCmd		41
#define kSetTrainingDelayReductionCmd	42
#define	kSetFaxECMPageBufferPtrCmd		43
#define kSetLineCurrentStateCmd			44
#define	kSetFramerParameterCmd			45
#define kStartDozeCmd                   46
#define kEndDozeCmd                     47
#define kStartRingFrequencyDetectorCmd  48
#define	kSetBufferingDelayAdjustmentCmd	49

	/* Composite action commands	64-127		*/
#define kDialCmd						64
#define kSendCallingToneCmd				65
#define kV24CircuitChangeCmd			66
#define	kStartATModeCmd					67
#define	kStopATModeCmd					68
#define	kSetATRegister					69
#define	kSetATRegisterLimits			70
#define	kSetATIResponse					71
#define	kEnableATDebugMode				72
#define	kSetWhiteListEntry				73
#define	kSetBlackListEntry				74

#define kV70Setup					    75      /* additional V70 configuration */
#define kEstablishChannel			    76      /* Establish new link layer channel (V70) */
#define kReleaseChannel					77      /* Release link layer channel (V70) */
#define kWaitChannelEstablished			78      /* Wait for establishment of the new link layer channel (V70) */

/* unused	79 */
#define kMnpOOBFrameCmd					80
#define kV80InBandCmd					81		/* V80 In-band commands */
#define kSetV250IdString				82
#define	kSetInternationalTablesCmd		83
#define	kConfigureCountryCmd			84
#define	kConigureCountryCmd				84
#define	kV8ControlCmd					85
#define kV8bisSendMessage				86
#define	kSetHWIdCmd						87
#define	kSetCodecIdCmd					88
#define	kOverCurrentDetected			89

#define kSetDebugDataHandlerCmd         90

typedef int v8ControlType;
#define kEnableDTEControl				1
#define kSetV8ControlTimeout			2
#define kSetCIValue						3
#define kSetCMValue						4
#define kSetJMValue						5
#define kSendCJ							6
#define kSetCallFunctionCategory		7

typedef int v250IdStringCode;
#define kGMIString						1
#define kGMMString						2
#define kGMRString						3
#define kGSNString						4
#define kGOIString						5

typedef int	kCallProgressParameterCode;
#define	kModemSignalPowerThreshold		1
#define	kDialtonePowerThreshold			2
#define	kRingBackPowerThreshold			3
#define	kBusyPowerThreshold				4
#define	kReorderPowerThreshold			5
#define	k2ndDTnPowerThreshold			6
#define	kMinDialtoneTime				7
#define	kDialtoneFreqRange				8
#define	kRingBackFreqRange				9
#define	kBusyFreqRange					10
#define	kReorderFreqRange				11
#define	k2ndDTnFreqRange				12


typedef	int	framerParameterCode;
#define	kSetHDLCLeadingFlags		0
#define	kHDLCResetFlagDetection		1
#define	kSyncFramerSetup			2
#define	kHDLCSendCRC				3
#define kHDLCSendFlags				4
#define	kHDLCSendAborts				5


typedef	int logDataCode;
#define eyeData				0
#define mseData				1
#define rxData				2
#define txData				3
#define neecData			4
#define eqlData				5
#define ieecData			6
#define feecData			7
#define eqlPllData			8
#define feecPllData			9
#define timingData			10
#define pjPhaseErrData		11
#define pjEstimateData		12
#define pjEstDiffData		13
#define pjCoefData			14
#define inputSignalData		15
#define outputSignalData	16
#define agcGainData			17
#define automoderData		18
#define v8CMData			19
#define v8JMData			20
#define inputAfterNeecData	21
#define eqlErrData			22
#define dpskMicrobitsData	23
#define v34P2LSamplesData	24
#define phaseSplittedLData	25
#define fftedLData			26
#define channelSNRData		27
#define noiseEstimateData	28
#define signalEstimateData	29
#define v34INFOData			30
#define v34ChanProbData		31
#define v34P2OutputData		32
#define v8ANSamDetectData	33
#define pFeecData			34
#define channelDelayData	35
#define timingOffsetData	36
#define trellisMSEData		37
#define interpolatedSignalData		38
#define dcCancelledSignalData		39
#define echoCancelledSignalData		40
#define predictorErrData			41
#define commandInfoData				42
#define unusedInfoData				43
#define atCommandInfoData			44
#define atResponseInfoData			45
#define hwTerminalTxData			46
#define hwTerminalRxData			47
#define statusInfoData				48
#define	channelResponseData			49
#define	channelImpulseRespData		50
#define	x2PcmP1DetectorInData		51
#define	x2PcmP1DetectorOutData		52
#define eqlRealData					53
#define ieecRealData				54
#define neecOutputData				55
#define precodedEqlOutputData		56
#define eqlRealErrData				57
#define idealEqlOutputData			58
#define agcData						59
#define pcmInfidelityData			60
#define v42bisCycleCount			61
#define pcmImdOffsetCoefData		62
#define pcmImdOffsetData			63
#define	v90RcvdDilLongData			64
#define	v90RcvdDilShortData			65
#define	v90DilProducedData			66
#define	pcmEncoderKbitsData			67
#define	pcmEncoderMbitsData			68
#define	pcmEncoderSbitsData			69
#define	pcmDecoderKbitsData			70
#define	pcmDecoderMbitsData			71
#define	pcmDecoderSbitsData			72
#define	v90CPorCPtData				73
#define	mnpDecoderInputData			74
#define	mnpDecoderOutputData		75
#define	v42bisEncoderInputData		76
#define	v42bisDecoderInputData		77
#define	modulatorInputData			78
#define	modulatorOutputData			79
#define encodedStatusData			80
#define blockFramerTxData			81
#define blockFramerRxData			82
#define framerTxData				83
#define framerRxData				84
#define	dpskBasebandData			85
#define	dpskBasebandLPFedData		86
#define	dpskRealData				87
#define bandEdgeCorrectedSignalData	88
#define atmLogData					89
#define clearEocLogData				90
#define g997LogData					91


#define	kLogDataDelimiter	0xFEFEFEFE

/****************************************************************************/
/*	1.	Type definitions.													*/
/*																			*/
/*	1.3	Handlers															*/
/****************************************************************************/

typedef	void	(SM_DECL *rcvHandlerType)			(void *gDslVars, int, short*);
typedef	void	(SM_DECL *xmtHandlerType)			(void *gDslVars, int, short*);
typedef	int		(SM_DECL *xmtHandlerWithRtnValType)	(void *gDslVars, int, short*);
typedef	void	(SM_DECL *timerHandlerType)			(void *gDslVars, int);
typedef	int		(SM_DECL *interpolatorHandlerType)	(void *gDslVars, int, short*, short*);
typedef	void	(SM_DECL *controlHandlerType)		(void *gDslVars, int);

typedef	int		(SM_DECL *txDataHandlerType)	(void *gDslVars, int,	uchar*);
typedef	int		(SM_DECL *rxDataHandlerType)	(void *gDslVars, int,	uchar*);

typedef	bitMap	(SM_DECL *signalDetectorType)	(void *gDslVars, int, int, int*);


typedef	void	(SM_DECL *hookHandlerType)		(void *gDslVars, Boolean);

typedef	short*	(SM_DECL *sampBuffPtrType)		(void *gDslVars, int);

typedef	void	(SM_DECL *eyeHandlerType)		(void *gDslVars, int, ComplexShort*);
typedef	void	(SM_DECL *logHandlerType)		(void *gDslVars, logDataCode, ...);
typedef	void	(SM_DECL *debugDataHandlerType)	(void *gDslVars, int, char *, int);

typedef	void	(SM_DECL *voidFuncType)			(void *gDslVars);

typedef	int		(SM_DECL *txAudioHandlerType)	(void *gDslVars, int,	short*);
typedef	int		(SM_DECL *rxAudioHandlerType)	(void *gDslVars, int,	short*);


/****************************************************************************/
/*	1.	Type definitions.													*/
/*																			*/
/*	1.4	Structures															*/
/****************************************************************************/

/*
 * AT command processor definitions
 */
#define kATRegistersNumber				56
#define	kFirstConfigurationRegister		500
#define	kLastConfigurationRegister		515
#define	kFirstInternationalRegister		516
#define	kLastInternationalRegister		595



#define kATMaxDialStringSize	128
typedef struct
	{
	struct 
		{
		uchar loadNumber;								/* Which profile to load upon powerup/reset */
		uchar countryCode;								/* T.35 Country Code */
		uchar profile[2][kATRegistersNumber];
		uchar dialString[4][kATMaxDialStringSize + 1];
		} config;
	uint versionCode;
	uint crcCheckSum;
	} NVRAMConfiguration;

/* Structure to hold international settings */
typedef	struct
	{
	char						*name;
	int							countryCode;
	const SRegisterDefinition	*userRegisters;
	const uint					*configRegisters;
	} CountryDescriptor;

/*
 * V.34 coding parameters structure
 */

typedef struct
	{
	/* DO NOT CHANGE THE ORDER OF FIELDS IN THIS STRUCTURE!
	 * (Some assembly code depends on it!)  If you
 	 * must add fields, please do so at the bottom.
	 */

	int					symbolRateIndex,
						dataRateIndex,
						userSNRAdjustment;
	Boolean				auxChannel, 
						expConstellation, 
						precoding,
						nonlinearCoding; 
	schar	J,			/* number of data frames in superframe				*/
			P,			/* number of mapping frames in a data frame			*/
			r,			/* number of high mapping frames in a data frame	*/
			b,			/* number of data bits in a mapping frame			*/
			W,			/* number of aux bits in a data frame				*/
			K,			/* number of S bits in a mapping frame				*/
			q, 			/* number of Q bits in a 2D symbol					*/
			M;			/* number of rings in shell mapping					*/
	int	nominalVariance;	/* the signal variance which gives 1e-2 BLER Q10 */
	int		bitsPerDataFrame;
	short	quantRoundOff,
			quantMask;
	uchar	nTrellisStates, 
			log2NTrellisStates; 
	short	gain1xmt,
			gain2xmt,
			gain1rcv,
			gain2rcv;
	ushort	bitInversionPattern;
	} V34CodingParams;

typedef	int				v8bisStatusCode;
typedef	bitMap				v8bisConnectionSetup;
#if defined(V8BIS) || defined(AT_COMMANDS_V8BIS)
#include "V8bisMainTypes.h"
#endif

#define kMaxMultiTones				4	/* MultiTone: search for up to this many tones at once */

#ifndef ADSL_MODEM
typedef	struct
	{
	modemStatusCode		code;
	union
		{
		int						value;
		int						freq;
		modemErrorCode				error;
		modulationMap				modulation;
		modulationMap				modemSignal;
		dataRateMap					dataRate;
		int						dtmfSignal;
		bitMap						callProgressSignal;
		bitMap						customSignal;
		void						*ptr;
		struct
			{
			int				detected;
			int				numTones;
			int				tones[kMaxMultiTones];
			} multiToneInfo;
		struct
			{
			v8bisStatusCode		code;
			int				value;
			} v8bisStatus;
		struct
			{
			trainingProgressCode	code;	
			int					value;			
			} trainingInfo;
		struct
			{
			int					code;	
			int					value;			
			} v24Circuit;
		struct
			{
			trainingProgressCode	code;	
			void*					ptr;			
			} advancedTrainingInfo;
		struct
			{
			capabilitiesStatusCode	code;	
			int					value;			
			} capabilitiesStatusInfo;
		struct
			{
			connectionInfoCode		code;
			int					value;			
			} connectionInfo;
		struct
			{
			connectionInfoCode		code;
			int						length;
			uchar					*ptr;
			} advancedConnectionInfo;
		struct
			{
			dialerStatusCode		code;
			int					value;
			int					makeTime;			
			int					breakTime;			
			} dialerStatus;
		struct
			{
			int					enabled;
			int					volume;			
			} speakerStatus;
		framingInfoCode				framingInfo;
		IOStatusCode				ioStatus;
		struct
			{
			lapmStatusCode			code;
			union
				{
				int				value;
				lapmTakedownReason	reason;
				lapmErrorCode		error;
				lapmTestResultCode	testResult;
				struct
					{
					int	length;
					uchar	*framePtr;
					} frame;
				struct
					{
					int	nFrames;
					int	nFrameErrors;
					} statistic;
				} param;			
			} lapmStatus;
		struct
			{
			lapmParameterCode		code;
			int					value;
			} lapmParameter;
		struct
			{
			v42bisStatusCode		code;
			union
				{
				int				value;
				v42bisErrorCode		error;
				struct
					{
					int	nBytesIn;
					int	nBytesOut;
					} statistic;
				} param;			
			} v42bisStatus;
		struct
			{
			mnpStatusCode			code;
			union
				{
				int				value;
				mnpTakedownReason	reason;
				mnpErrorCode		error;
				struct
					{
					int	nFrames;
					int	nFrameErrors;
					} statistic;
				struct
					{
					uint	nSize;
					uchar  *Buffer;
					} fallback;
				struct
					{
					char	*header;
					void	*frame;
					} frame;
				struct
					{
					int	nack;
					int	rFrameNo;
					} timeout;
				struct
					{
					int	frameNo;
					int	framesPending;
					} retrFrame;
				} param;			
			} mnpStatus;
		struct
			{
			mnpParameterCode		code;
			int					value;
			} mnpParameter;
		struct
			{
			v70StatusCode			code;
			union
				{
				int				value;
				v70TakedownReason	reason;
				v70ErrorCode		error;
				struct
					{
					int	nFrames;
					int	nFrameErrors;
					} statistic;
				struct
					{
					int	length;
					uchar	*framePtr;
					} frame;
				struct
					{
					int	nack;
					int	rFrameNo;
					} timeout;
				struct
					{
					int	frameNo;
					int	framesPending;
					} retrFrame;
				struct 	
					{
					int	ChannelId;
					int	DLCI;
					uint	LcNum;
					v70TakedownReason	reason;
					} channelInfo;			
				struct 	
					{
					int	ChannelId;
					int	stateOld;
					int	stateNew;
					} stateInfo;			
				} param;
			uint	v70Time;
			} v70Status;
		struct
			{
			audioStatusCode			code;
			union
				{
				int		value;
				struct
					{
					int	nReq;
					int	nAvail;
					} buffer;
				struct
					{
					int	nFrames;
					int	nFrameErrors;
					} statistic;
				struct
					{
					int	length;
					uchar	*framePtr;
					} frame;
				} param;
			} audioStatus;
		struct
			{
			v80StatusCode			code;
			union
				{
				int				value;
				v80TakedownReason	reason;
				v80ErrorCode		error;
				struct
					{
					int	nFrames;
					int	nFrameErrors;
					} statistic;
				struct
					{
					int	length;
					uchar	*framePtr;
					} frame;
				struct
					{
					int	code;
					int	value;
					} inBand;
				} param;
			uint	v80Time;
			} v80Status;
		struct
			{
			v70ParameterCode		code;
			int					value;
			} v70Parameter;
		struct
			{
			breakType			type;
			int				length;
			} breakStatus;
		struct
			{
			callerIDStatusCode			code;
			union
				{
				int				value;
				struct
					{
					callerIDErrorCode	code;
					int				value;	
					} callerIDError;
				struct
					{
					int			length;
					char*			ptr;	
					} message;
				} param;			
			} callerIDStatus;
		struct
			{
			uint		signal;
			uchar		*msg1;
			int		msg1Length;	
			uchar		*msg2;
			int		msg2Length;	
			} A8RStatus;
		struct
			{
			overlayStatusCode		code;
			int					value;
			int					value2;
			} overlayStatus;
		struct
			{
			uint	nBits;
			uint	nBlocks;
			uint	nBitErrors;
			uint	nBlockErrors;

			uint	nAudioBits;
			uint	nAudioBlocks;
			uint	nAudioSyncErrors;
			uint	nAudioBlockErrors;
			} testResults;
		uint					checksum;
		struct
			{
			uint	sizeM;
			uchar	*filename;
			} logFileControlStatus;
		struct
			{
			int	direction;
			int	module;
			int	message;
			int	data;
			}
		faxClass2Status;
		
		} param;
	} modemStatusStruct;
	
typedef	void	(SM_DECL *statusHandlerType)	(void *gDslVars, modemStatusStruct*);
#endif	/* ADSL_MODEM */

/****************************************************************************/
/*	1.	Type definitions.													*/
/*																			*/
/*	1.5	Command structure													*/
/****************************************************************************/

typedef struct
	{
    Boolean remoteModemIsFlex;
    uchar   countryCode;
    ushort  manufacturerId;
    uchar   licenseeId;
    uchar   productCapabilities;
    Boolean digitalModeFlag;
    Boolean prototypeFlag;
    uchar   version;
	}
FlexV8bisStruct;

typedef struct
	{
	symbolRateMap	symbolRates;
	dataRateMap		dataRates;
	dataRateMap		dataRates56k;
	dataRateMap     dataRatesFlex;
	featureMap		features;
	bitMap			auxFeatures;
	bitMap			demodCapabilities;
	int			rateThresholdAdjustment;	/* dB Q4	*/
    FlexV8bisStruct flexRemoteV8bisInfo;
	}	dataPumpCapabilities;

#ifndef ADSL_MODEM
typedef	struct	SoftwareModemCommandParameters
	{
	modemCommandCode		command;
	union
		{
		int				xmtGain;
		uint				hybridDelayQ4ms;
		int				modemSampleRate;
		int				timeInMs;
		int				state;
		int				freq;
		NVRAMConfiguration	*nvramConfigurationPtr;
		int				enabled;
		int				value;
		uchar				*phoneNumber;
		uchar				*faxECMPageBufferPtr;
		CountryDescriptor	*countryDescriptorTable;
		struct
			{
			dataRateMap			dteRate;
			bitMap				format;
			} dteRateSpec;
		struct
			{
			v8ControlType		code;
			int				value;
			uchar				*buffer;
			} v8ControlSpec;
		struct
			{
			directionType			direction;
			v8bisConnectionSetup	setup;
			void					*capPtr;
			voidFuncType			confirmMsFunc;
			voidFuncType			genMsFunc;
			xmtHandlerWithRtnValType	ogmFunc;
			} v8bisSpec;
		struct
			{
			directionType	direction;
			} ADSISpec;
		struct
			{
			directionType			direction;
			modulationMap			modulations;
			dataPumpCapabilities	capabilities;
			} modeSpec;
		struct
			{
			int			time, 
							freq1, 
							freq2, 
							freq3, 
							freq4,
							mag1, 
							mag2,
							mag3,
							mag4;
			} toneSpec;
		struct
			{
			int		signal;
			uchar		*msg1;
			int		msg1Length;	
			uchar		*msg2;
			int		msg2Length;	
			int		sig_en;
			int		msg_en;
			int		supp_delay;
			}
			v8bisMessageSpec;
		struct
			{
			linkLayerType		type;
			bitMap				setup;
			dataRateMap			rxDataRate;
			dataRateMap			txDataRate;
			int				rtDelayQ4ms;				
			rxDataHandlerType	rxDataHandlerPtr;
			txDataHandlerType	txDataHandlerPtr;
			} linkLayerSpec;
		struct
			{
			framerType			type;
			bitMap				setup;
			directionType		direction;
			int				fill[2]; /* need to match linkLayerSpec */
			rxDataHandlerType	rxDataHandlerPtr;
			txDataHandlerType	txDataHandlerPtr;
			} framerSpec;
		struct
			{
			framerParameterCode	code;
			int				value;
			} framerParameterSpec;
		struct
			{
			bitMap				callProgressDetectorSetup;
			signalDetectorType	callProgressDetectorPtr;	/* if nil, use defaults			*/
			signalDetectorType	customDetectorPtr;			/* if nil, no custom detector	*/
			} callProgressMonitorSpec;
		struct
			{
			uint			maxTones;					/* maximum number of simultaneous tones to detect */
			uint			allowableVariance;			/* maximum cumulative variance in the eight interpolated frequencies */
			uint			totalPowerThreshold;		/* ignore complete block if power less than this */
			uint			powerShiftThreshold;		/* ignore a bin if its power is less than (totalPowerValue >> powerShiftThreshold) */
			uint			toneMatchThresholdHz;		/* tones within +/- this many Hz of original tone are considered the same tone */
			uint			binSeparation;				/* ignore tones with a spacing of less than this */
			uint			outsideFreqDeviation;		/* an individual value in the interpolated array can be up to this many Hz outside of the expected angle range */
			} multiToneSpec;
		struct
			{
			uchar				*dialString;	/* nil limited string for DTMF dialing sequence	*/
			int				pulseBreakTime, 
								pulseMakeTime, 
								pulseInterDigitTime,
								toneDigitTime, 
								toneInterDigitTime, 
								toneLoGroupMag,
								toneHiGroupMag,
								flashTime, 
								pauseTime,
								signalWaitTimeout,	
								blindDialingTimeout; 	
			bitMap				dialerSetup;
			bitMap				callProgressDetectorSetup;
			signalDetectorType	callProgressDetectorPtr;	/* if nil, use defaults			*/	
			signalDetectorType	customDetectorPtr;			/* if nil, no custom detector	*/
			hookHandlerType		hookHandlerPtr;				/* nil if DTMF dialing specified*/
			} dialSpec;
		struct
			{
			int			timeOn, 
							timeOff, 
							freq;
			} callingToneSpec;
		union
			{
			statusHandlerType	statusHandlerPtr;
			eyeHandlerType		eyeHandlerPtr;
			logHandlerType		logHandlerPtr;
#if defined(DEBUG_DATA_HANDLER)
            debugDataHandlerType debugDataHandlerPtr;
#endif
			rxDataHandlerType	rxDataHandlerPtr;
			txDataHandlerType	txDataHandlerPtr;
			} handlerSpec;
		struct
			{
			breakType			type;
			int				length;
			} breakSpec;
		struct
			{
			int				length;
			uchar				*dataPtr;
			} lapmTestSpec;
		struct
			{
			bitMap				setupLapm;
			rxDataHandlerType	rxAudioHandlerPtr;
			txDataHandlerType	txAudioHandlerPtr;
            } v70SetupSpec;
		struct
			{
			uint				ChannelId;
			uint				LogChannelNum;
			uint				PortNum;
            } EstChannelSpec;
		struct
			{
			uint				ChannelId;
            } WaitChannelSpec;
		struct
			{
			uint				ChannelId;
			uint				LogChannelNum;
			uint				PortNum;
			uint				DLCI;
            } RelChannelSpec;
		struct
			{
			audioType			type;
			bitMap				setup;
			dataRateMap			rxAudioRate;
			dataRateMap			txAudioRate;
			rxAudioHandlerType	rxAudioHandlerPtr;
			txAudioHandlerType	txAudioHandlerPtr;
			} audioSpec;
		struct
			{
			int					code;	
			int					value;			
			} v24Circuit;
		struct
			{
			uint					code;	
			uint					value;			
			uint					minValue;
			uint					maxValue;
			} atRegister;
		struct
			{
			int					code;	
			uchar					*response;
			} atiSpec;
		struct
			{
			int					length;
			uchar					*framePtr;
			} frameSpec;
		struct
			{
			int					code;
			union
				{
				int				value;
				struct
					{
					int			loFreq1;
					int			hiFreq1;
					int			loFreq2;
					int			hiFreq2;
					} freqRange;
				} params;
			} callProgressParamSpec;
		struct
			{
			v250IdStringCode	v250IdCode;
			uchar				*v250IdString;
			} v250IdSpec;

		} param;
	} modemCommandStruct;
	
typedef	Boolean	(*commandHandlerType)	(modemCommandStruct*);
#endif /* ADSL_MODEM */



/****************************************************************************/
/*	2.	Constant definitions.												*/
/*																			*/
/*	2.1	Definitive constants												*/
/****************************************************************************/

#define kMaxSampleBlockSize			48
#define kMaxDataBlockSize			48

#define	kMaxDialStringLength		127
#define	kCallProgressSampleRate		7200

#define	kMaxCallerIDMessageLength	80

/****************************************************************************/
/*	2.	Constant definitions.												*/
/*																			*/
/*	2.2	Bit maps														*/
/****************************************************************************/

/* modulationMap */

#define	kIdle					0x00000000
#define	kV25					0x00000001
#define	kV8						0x00000002
#define	kCid					0x00000004
#define	kV8bis					0x00000008
#define	kV21					0x00000010
#define	kV22					0x00000020
#define	kV23					0x00000040
#define	kV32					0x00000080
#define	kV34					0x00000100
#define	kX2						0x00000200
#define	kV90					0x00000400
#define	k56Flex					0x00000800
#define	kV27					0x00001000
#define	kV29					0x00002000
#define	kV17					0x00004000
#define	kV34HDX					0x00008000
#define	kV34HDXC				0x00010000
#define	kBell103				0x00100000
#define	kBell212				0x00200000
#define	kDataCallingTone		0x01000000
#define	kFaxCallingTone			0x02000000

#define	kV22FastNZConnect	    0x04000000
#define kV22FastNNZConnect      0x08000000
#define kV22FastConnect         (kV22FastNZConnect|kV22FastNNZConnect)
#define kV22bisFastConnect      0x10000000


#define	kDataModulations	(kV25 | kV8 | kV21 | kV22FastConnect | kV22bisFastConnect | kV22 | kV23 | kV32 | kV34 | kBell103 | kBell212)
#define	kDataOnlyModulations (kV21 | kV22 | kV23 | kV32 | kBell103 | kBell212)
#define	kPCMModulations		(kV90 | kX2 | k56Flex)

#define	kFaxModulations		(kV25 | kV21 | kV27 | kV29 | kV17)
#define	kFaxOnlyModulations	(kV27 | kV29 | kV17)
#define	kFaxModulationShift		12

/* symbolRateMap	*/

#define	k1200Hz			0x00000001
#define	k1600Hz			0x00000002
#define	k2400Hz			0x00000004
#define	k2743Hz			0x00000008
#define	k2800Hz			0x00000010
#define	k3000Hz			0x00000020
#define	k3200Hz			0x00000040
#define	k3429Hz			0x00000080
#define	k8000Hz			0x00000100

#define	kAllSymbolRates	(	k1200Hz | k1600Hz | k2400Hz | k2743Hz | \
							k2800Hz | k3000Hz | k3429Hz | k8000Hz )

/* dataRateMap	*/

#define	k75bps			0x00000002
#define	k300bps			0x00000004
#define	k600bps			0x00000008
#define	k1200bps		0x00000010
#define	k2400bps		0x00000020
#define	k4800bps		0x00000040
#define	k7200bps		0x00000080
#define	k9600bps		0x00000100
#define	k12000bps		0x00000200
#define	k14400bps		0x00000400
#define	k16800bps		0x00000800
#define	k19200bps		0x00001000
#define	k21600bps		0x00002000
#define	k24000bps		0x00004000
#define	k26400bps		0x00008000
#define	k28800bps		0x00010000
#define	k31200bps		0x00020000
#define	k33600bps		0x00040000
#define	k36000bps		0x00080000
#define	k38400bps		0x00100000
#define	k57600bps		0x00200000
#define	k115200bps		0x00400000
#define	k230400bps		0x00800000
#define	k460800bps		0x01000000
#define	k921600bps		0x02000000
/*
 * kPCMRate is used to identify that the reported rate is
 * PCM modulation rate, and is only used for PCM modulation while
 * reporting rate !!!!
 */
#define	kPCMRate		0x40000000
#define kPCMFlexRate    0x80000000
#define	kAllDataRates   0x0FFFFFFF

/* rates specific for X2  and V.90 */
#define	kPCM25333bps	0x00000001
#define	kPCM26666bps	0x00000002
#define	kPCM28000bps	0x00000004
#define	kPCM29333bps	0x00000008
#define	kPCM30666bps	0x00000010
#define	kPCM32000bps	0x00000020
#define	kPCM33333bps	0x00000040
#define	kPCM34666bps	0x00000080
#define	kPCM36000bps	0x00000100
#define	kPCM37333bps	0x00000200
#define	kPCM38666bps	0x00000400
#define	kPCM40000bps	0x00000800
#define	kPCM41333bps	0x00001000
#define	kPCM42666bps	0x00002000
#define	kPCM44000bps	0x00004000
#define	kPCM45333bps	0x00008000
#define	kPCM46666bps	0x00010000
#define	kPCM48000bps	0x00020000
#define	kPCM49333bps	0x00040000
#define	kPCM50666bps	0x00080000
#define	kPCM52000bps	0x00100000
#define	kPCM53333bps	0x00200000
#define	kPCM54666bps	0x00400000
#define	kPCM56000bps	0x00800000
#define	kPCM57333bps	0x01000000

#define	kV90ServerToClientDataRates	\
						(	kPCM28000bps | kPCM29333bps | kPCM30666bps | \
							kPCM32000bps | kPCM33333bps | kPCM34666bps | \
							kPCM36000bps | kPCM37333bps | kPCM38666bps | \
							kPCM40000bps | kPCM41333bps | kPCM42666bps | \
							kPCM44000bps | kPCM45333bps | kPCM46666bps | \
							kPCM48000bps | kPCM49333bps | kPCM50666bps | \
							kPCM52000bps | kPCM53333bps | kPCM54666bps | \
							kPCM56000bps | kPCM57333bps )

#define	kV90ClientToServerDataRates	\
						(	k4800bps  | k7200bps  | k9600bps  | k12000bps | \
							k14400bps | k16800bps | k19200bps | k21600bps | \
							k24000bps | k26400bps | k28800bps | k31200bps | \
							k33600bps )	



#define	kX2ServerToClientDataRates	\
						(	kPCM25333bps | kPCM26666bps | kPCM28000bps | \
							kPCM29333bps | kPCM30666bps | kPCM32000bps | \
							kPCM33333bps | \
							kPCM34666bps | kPCM36000bps | kPCM37333bps | \
							kPCM38666bps | kPCM40000bps | kPCM41333bps | \
							kPCM42666bps | kPCM44000bps | kPCM45333bps | \
							kPCM46666bps | kPCM48000bps | kPCM49333bps | \
							kPCM50666bps | kPCM52000bps | kPCM53333bps | \
							kPCM54666bps | kPCM56000bps | kPCM57333bps )
#define	kX2ClientToServerDataRates	\
						(	k4800bps | k7200bps | k9600bps | k12000bps | k14400bps | \
								k16800bps | k19200bps | k21600bps | k24000bps | k26400bps | k28800bps | \
								k31200bps )

  /*
  Rates specific for Flex
  */
#define kPCMFlex32000bps  0x00000001
#define kPCMFlex34000bps  0x00000002
#define kPCMFlex36000bps  0x00000004
#define kPCMFlex38000bps  0x00000008
#define kPCMFlex40000bps  0x00000010
#define kPCMFlex42000bps  0x00000020
#define kPCMFlex44000bps  0x00000040
#define kPCMFlex46000bps  0x00000080
#define kPCMFlex48000bps  0x00000100
#define kPCMFlex50000bps  0x00000200
#define kPCMFlex52000bps  0x00000400
#define kPCMFlex54000bps  0x00000800
#define kPCMFlex56000bps  0x00001000
#define kPCMFlex58000bps  0x00002000
#define kPCMFlex60000bps  0x00004000

#define	kFlexServerToClientDataRates \
                        (   kPCMFlex32000bps | kPCMFlex34000bps | kPCMFlex36000bps | kPCMFlex38000bps | \
							kPCMFlex40000bps | kPCMFlex42000bps | kPCMFlex44000bps | kPCMFlex46000bps | \
							kPCMFlex48000bps | kPCMFlex50000bps | kPCMFlex52000bps | kPCMFlex52000bps | \
							kPCMFlex54000bps | kPCMFlex56000bps | kPCMFlex58000bps | kPCMFlex60000bps )

#define	kFlexClientToServerDataRates	\
						(	k4800bps  | k7200bps  | k9600bps  | k12000bps | \
							k14400bps | k16800bps | k19200bps | k21600bps | \
							k24000bps | k26400bps | k28800bps | k31200bps )


#define	k2400BitShift	5
#define	k4800BitShift	6

#define	kPCM28000bpsShift	2

#define	kV21Rates			k300bps
#define	kV22Rates			k1200bps
#define	kV22bisRates		(k1200bps | k2400bps)
#define	kV23Rates			(k75bps | k1200bps)
#define	kCidRates			(k1200bps)
#define	kV32Rates			(k4800bps | k9600bps)
#define	kV32bisRates		(kV32Rates | k7200bps | k12000bps | k14400bps)
#define	kV32terboRates		(kV32bisRates | k16800bps | k19200bps)	
#define	kV34Rates			(	k2400bps | k4800bps | k7200bps | k9600bps | k12000bps | k14400bps | \
								k16800bps | k19200bps | k21600bps | k24000bps | k26400bps | k28800bps | \
								k31200bps | k33600bps )	

#define	kV27Rates			(k2400bps | k4800bps)
#define	kV29Rates			(k4800bps | k7200bps | k9600bps)
#define	kBell103Rates       k300bps
#define	kBell212Rates       k1200bps


/* Demodulator capabilities	*/
#define	kNeecEnabled					0x00000001
#define	kPFeecEnabled					0x00000002
#define	kIeecEnabled					0x00000004
#define	kFeecEnabled					0x00000008

#define	kRapidEqualizerTraining			0x00000010
#define	kRapidPECTraining				0x00000020
#define	kRapidECTraining				0x00000040
#define	kAutoLoadReductionEnabled		0x00000080

#define	kTimingTrackingEnabled			0x00000100
#define	kPhaseLockedLoopEnabled			0x00000200
#define	kFeecPhaseLockedLoopEnabled		0x00000400
#define	kPhaseJitterTrackingEnabled		0x00000800

#define	kClockErrorTrackingEnabled		0x00001000
#define	kFreqOffsetTrackingEnabled		0x00002000
#define	kFeecFreqOffsetTrackingEnabled	0x00004000

#define	kShorterNeecEnabled				0x00008000
#define	kShorterPFeecEnabled			0x00010000
#define	kFrondEndHPFilterEnabled		0x00020000
#define kGainControlEnabled				0x00040000
#define kPhaseHitControlEnabled			0x00080000
#define	kBandEdgeCorrectorEnabled		0x00100000
#define kDisableFaxFastClearDown		0x00200000

#define kImdOffsetCompensationEnabled	0x00400000

#define kV34ShortEqlLengthExtShift  23
#define kV34ShortEqlLengthExtMask      (0x3<<kV34ShortEqlLengthExtShift)
#define kV34EqlLengthReductionEnabled  (1<<(kV34ShortEqlLengthExtShift+2))
#define kPCMIeecLengthReductionEnabled (1<<(kV34ShortEqlLengthExtShift+3))

/* featureMap	*/

#define	kAllFeatures				0xFFFFFFFF

#define	kAutomodingEnabled			0x00000001	/* bit 1	*/
#define	kAutomodingDisabled			0x00000000	/* bit 1	*/

#define	kV8SendCIEnabled			0x00000002	/* bit 2	*/
#define	kV8SendCIDisabled			0x00000000	/* bit 2	*/

#define	kV34CMEModem				0x00000004	/* bit 3	*/
#define	kV34NotCMEModem				0x00000000	/* bit 3	*/

#define	kV34ExtraINFOPreamble		0x00000008	/* bit 4	*/

#define	kRetrainingEnabled			0x00000010
#define	kRateRenegotiationEnabled	0x00000020
#define	kTrellisCodingEnabled		0x00000040

/* Fax specific features	*/
#define	kFaxShortTraining			0x00000080
#define	kFaxEchoSuppressionEnabled	0x00000100

/* V.22/V.22bis specific features	*/
#define	kV22GuardTone1800HzEnabled	0x00000200
#define	kV22GuardTone550HzEnabled	0x00000400


/* V.34 specific features	*/

#define	kV34bisEnabled				0x00000800

#define	kV34PowerReductionAllowed	0x00001000
#define	kAuxChannelEnabled			0x00002000
#define	kAuxChannelDisabled			0x00000000
#define	kV34TrellisEncoderTypeMask	0x0000C000
#define	kV34TrellisEncoderTypeShift	14

#define	kTRN16						0x00010000
#define	kAssymDataRatesEnabled		0x00020000
#define	kNonLinearCodingEnabled		0x00040000
#define	kConstShapingEnabled		0x00080000
#define	kPrecodingEnabled			0x00100000

#define	kV34LoFcAt2400HzEnabled		0x00200000
#define	kV34HiFcAt2400HzEnabled		0x00400000
#define	kV34LoFcAt2743HzEnabled		0x00800000
#define	kV34HiFcAt2743HzEnabled		0x01000000
#define	kV34LoFcAt2800HzEnabled		0x02000000
#define	kV34HiFcAt2800HzEnabled		0x04000000
#define	kV34LoFcAt3000HzEnabled		0x08000000
#define	kV34HiFcAt3000HzEnabled		0x10000000
#define	kV34LoFcAt3200HzEnabled		0x20000000
#define	kV34HiFcAt3200HzEnabled		0x40000000
#define	kV34LoFcAt3429HzEnabled		0x80000000
#define	kV34HiFcAt3429HzEnabled		0x80000000

/* auxiliary features definintions map */

#define	kLoopbackTestFinish				0x00000000
#define	kLoopbackTestV54Loop1			0x00000001
#define	kLoopbackTestV54Loop2			0x00000002
#define	kLoopbackTestV54Loop3			0x00000003
#define	kLoopbackTestTypeMask			0x00000003
#define	kLoopbackTestAutoRespondEnabled	0x00000004
#define	kLoopbackSelfTest				0x00000008

#define	kPreempFilterMask			0x000000F0
#define	kPreempFilterShift			4

#define	kPcmCodingTypeMuLaw			0x00000100
#define	kPcmServerToServerEnabled	0x00000200
#define	kPcmIsServerModem			0x00000400
#define	kPcmAnalogModemAvailable	0x00000800
#define	kPcmDigitalModemAvailable	0x00001000
#define	kPcmDceOnDigitalNetwork		0x00002000
#define	kPcmDModemPwrCalAtCodecOut	0x00004000
#define	kPcm3429UpstreamAvailable	0x00008000

#define	kPcmSpectralShapingBitsMask		0x00070000
#define	kPcmSpectralShapingBitsShift	16
#define	kV90ServerNotDetSbarAfterJdbarFix	0x00080000

#define kAutomoderPassive			0x00400000

#define	kV8HoldANSamUntilDetCI		0x00800000
#define	kFaxSendFromOrgSide			0x01000000
#define	kFaxV34HDX2400bpsCtrlChan	0x02000000
#define	kFaxV34HDXAllowAsymCtrlChan	0x04000000
#define	kV8ANSamStageDisabled		0x08000000

#define kFlexSkipV8bis              0x10000000
#define kV34ControlChannelEnabled   0x20000000
#define kV34SeamlessRateChangeEnabled 0x40000000

#define	kPTTTest					0x80000000

/* call progress detection Map	*/

#define	kDialTone				0x00000001
#define	kRingBack				0x00000002
#define	kBusy					0x00000004
#define	kReorder				0x00000008
#define	k2ndDTn					0x00000010
#define	kBongTone				0x00000020

/* Break type bit settings	*/
#define	kExpedited		0x0001
#define	kDestructive	0x0002

/* async Framer setup map		*/

#define	kNDataBitsMask	0x03
#define	k5DataBits		0x00
#define	k6DataBits		0x01
#define	k7DataBits		0x02
#define	k8DataBits		0x03

#define	kNDataBitsShift		0
#define	kNDataBitsOffset	5

#define	kParityTypeMask	0x1C
#define	kNoParity		0x00
#define	kOddParity		0x04
#define	kEvenParity		0x08
#define	kMarkParity		0x0C
#define	kSpaceParity	0x10

#define	kNStopBitsMask	0x60
#define	k1StopBits		0x00
#define	k2StopBits		0x20

#define	kNStopBitsShift		5
#define	kNStopBitsOffset	1

/* Sync Framer setup map		*/

#define kUnderrunCharMask			0xff
#define kRepeatLastCharOnUnderrun	0x100

/* HDLC sync framer setup maps	*/
#define	kNFlagsBeforeFramesMask		0x3F
#define	kNFlagsBeforeFramesShift	0

#define	kNFlagsBetweenFramesMask	0x3F
#define	kNFlagsBetweenFramesShift	6

#define	k32BitCRC					0x1000
#define	kFlagSharingEnabled			0x2000

#define kNFlagsBeforeReportMask		0x03	/* no. of *extra* flags reqd before frame */
#define kNFlagsBeforeReportShift	14

#define	kTxDeferredCRC				0x10000
#define	kRxDeferredCRC				0x20000
#define	kTxIdleMarks				0x40000
#define kNoCRC						0x80000

/* SAM framer setup maps	*/

#define	kSAMTransparentIdleTypeMask		0x00000003
#define	kSAMTransparentIdleTypeShift	0
#define	kSAMFramedIdleTypeMask			0x00000004
#define	kSAMFramedIdleTypeShift			2
#define	kSAMFramedOverrunActionMask		0x00000010
#define	kSAMFramedOverrunActionShift	4
#define	kSAMHalfDuplexModeMask			0x00000020
#define	kSAMHalfDuplexModeShift			5
#define	kSAMCRCTypeMask					0x000000C0
#define	kSAMCRCTypeShift				6
#define	kSAMNRZIEnabledMask				0x00000100
#define	kSAMNRZIEnabledShift			8
#define	kSAMSyn1Mask					0x00FF0000
#define	kSAMSyn1Shift					16
#define	kSAMSyn2Mask					0xFF000000
#define	kSAMSyn2Shift					24

/* <trans_idle> */
#define	kSAM8bitSYNHuntDisabled		0
#define	kSAM8bitSYNHuntEnabled		((uint)1 << kSAMTransparentIdleTypeShift)
#define	kSAM16bitSYNHuntEnabled		((uint)2 << kSAMTransparentIdleTypeShift)

/* <framed_idle> */
#define	kSAMSendFlagsOnIdle			0
#define	kSAMSendMarksOnIdle			((uint)1 << kSAMFramedIdleTypeShift)

/* <framed_un_ov> */
#define	kSAMAbortOnUnderrun			0
#define	kSAMFlagsOnUnderrun			((uint)1 << kSAMFramedOverrunActionShift)

/* <hd_auto> */
#define	kSAMHalfDuplexNoAuto		0
#define	kSAMHalfDuplexAuto			((uint)1 << kSAMHalfDuplexModeShift)


/* <crc_type> */
#define	kSAMNoCRC					0
#define	kSAM16bitCRC				((uint)1 << kSAMCRCTypeShift) 
#define	kSAM32bitCRC				((uint)2 << kSAMCRCTypeShift) 
				
/* <nrzi_en> */
#define	kSAMNRZIDisabled			0
#define	kSAMNRZIEnabled				((uint)1 << kSAMNRZIEnabledShift)


/* LAPM setup maps	*/
#define	kLapmDirection				0x00000001		/* Bit 0  */
#define	kLapmSREJEnabled			0x00000002		/* Bit 1  */
#define	kLapmDetectionEnabled		0x00000004		/* Bit 2  */
#define	kLapmLongADPEnabled			0x00000008		/* Bit 3  */

#define	kLapmCompressionEnabledMask	0x00000030
#define	kLapmTxCompressionEnabled	0x00000010		/* Bit 4  */
#define	kLapmRxCompressionEnabled	0x00000020		/* Bit 5  */
#define	kLapmCompressionEnabledShift		4

#define	kLapmRetryLimitMask			0x000000C0		/* Bits 6,7  */

#define	kLapmNoRetryLimit			0x00000000
#define	kLapm4Retries				0x00000040
#define	kLapm8Retries				0x00000080
#define	kLapm20Retries				0x000000C0

#define	kLapmWindowSizeMask			0x00001F00		/* Bits 8-12  */
#define	kLapmWindowSizeShift		8

#define	kLapmWindowSize8			0x00000800
#define	kLapmWindowSize15			0x00000F00


#define	kLapmInfoFieldSizeMask		0x0000E000		/* Bits 13-15  */
#define	kLapmInfoField8Bytes		0x00000000
#define	kLapmInfoField16Bytes		0x00002000
#define	kLapmInfoField32Bytes		0x00004000
#define	kLapmInfoField64Bytes		0x00006000
#define	kLapmInfoField128Bytes		0x00008000
#define	kLapmInfoField192Bytes		0x0000A000
#define	kLapmInfoField256Bytes		0x0000C000
#define	kLapmInfoField512Bytes		0x0000E000
#define	kLapmInfoFieldSizeShift		13

#define	kLapmT400Mask				0x00030000		/* Bits 16-17	*/
#define	kLapmAutoT400				0x00000000
#define	kLapm750msT400				0x00010000
#define	kLapm3secT400				0x00020000
#define	kLapm30secT400				0x00030000

#define	kLapmT401Mask				0x000C0000		/* Bits 18-19	*/
#define	kLapmAutoT401				0x00000000
#define	kLapm750msT401				0x00040000
#define	kLapm3secT401				0x00080000
#define	kLapm6secT401				0x000C0000

#define	kLapmT403Mask				0x00300000		/* Bits 20-21	*/
#define	kLapmAutoT403				0x00000000
#define	kLapm750msT403				0x00100000
#define	kLapm2secT403				0x00200000
#define	kLapm4secT403				0x00300000



#define	kLapmDictSizeMask			0x00C00000		/* Bits 22-23  */
#define	kLapmDictSize512			0x00000000
#define	kLapmDictSize1024			0x00400000
#define	kLapmDictSize2048			0x00800000
#define	kLapmDictSize4096			0x00C00000

#define	kLapmStringSizeMask			0xFF000000		/* Bits 24-31  */
#define	kLapmStringSizeShift		24

/* MNP setup maps	*/

#define	kMnpMinPLevel				0x00000001		/* Bit 0: 1 - Minimal, 0 - Standard */
#define	kMnpStdPLevel				0x00000000		/* Bit 0: 1 - Minimal, 0 - Standard */

#define	kMnpOptimizationEnabled		0x00000002		/* Bit 1  */
#define	kMnpOptimizationDisabled	0x00000000		/* Bit 1  */

#define	kMnpCompressionEnabled		0x00000004		/* Bit 2  */
#define	kMnpCompressionDisabled		0x00000000		/* Bit 2  */

#define	kMnpClassMask				0x00000018
#define	kMnpClassShift						 3
#define	kMnpClass1					0x00000008
#define	kMnpClass2					0x00000010
#define	kMnpClass3					0x00000018		/* Bits 3,4 */

#define kMnpMaxRetryMask		    0x00000060		/* Bits 5,6 */
#define kMnpMaxRetryShift					 5
#define	kMnpNoRetryLimit			0x00000000
#define	kMnp4Retries				0x00000020
#define	kMnp8Retries				0x00000040
#define	kMnp20Retries				0x00000060

#define	kMnpInfoFieldSizeMask		0x00000380		/* Bits 7-9  */
#define	kMnpInfoFieldSizeShift				 7
#define	kMnpInfoField8Bytes			0x00000000
#define	kMnpInfoField16Bytes		0x00000080
#define	kMnpInfoField32Bytes		0x00000100
#define	kMnpInfoField64Bytes		0x00000180
#define	kMnpInfoField128Bytes		0x00000200
#define	kMnpInfoField192Bytes		0x00000280
#define	kMnpInfoField256Bytes		0x00000300
#define	kMnpInfoField260Bytes		0x00000380

#define	kMnpT400Mask				0x00003000		/* Bits 12,13 */
#define	kMnpT400Shift						12
#define	kMnpAutoT400				0x00000000
#define	kMnp750msT400				0x00001000
#define	kMnp3secT400				0x00002000
#define	kMnp6secT400				0x00003000

#define	kMnpT401Mask				0x0000C000		/* Bits 14,15 */
#define	kMnpT401Shift						14
#define	kMnpAutoT401				0x00000000
#define	kMnp750msT401				0x00004000
#define	kMnp3secT401				0x00008000
#define	kMnp6secT401				0x0000C000

#define	kMnpT403Mask				0x00030000		/* Bits 16,17 */
#define	kMnpT403Shift						16
#define	kMnpAutoT403				0x00000000
#define	kMnp60secT403				0x00010000
#define	kMnp600secT403				0x00020000
#define	kMnp3600secT403				0x00030000

#define kMnpFallbackTypeMask		0x000C0000		/* Bits 18,19 */
#define kMnpFallbackTypeShift				18
#define kMnpNoFallback				0x00000000
#define kMnpFallbackTime			0x00040000
#define kMnpFallback200				0x00080000
#define kMnpFallbackChar			0x000C0000

#define	kMnpWindowSizeMask			0x00300000		/* Bits 20,21  */
#define	kMnpWindowSizeShift					20
#define	kMnp1Frame 					0x00000000
#define	kMnp4Frames 				0x00100000
#define	kMnp8Frames 				0x00200000
#define	kMnp16Frames 				0x00300000

#define	kMnpDirection				0x00800000		/* Bit 22  */

#define kMnpFallbackCharMask		0xFF000000		/* Bit 24-31  */
#define kMnpFallbackCharShift				24

/* kV34HDXTurnOffCurrentModeCmd state parameter values */

#define	kV34HDXTurnOffAsClearDown				0
#define	kV34HDXTurnOffFromControlSource			1
#define	kV34HDXTurnOffFromControlDestination	2
#define	kV34HDXTurnOffFromPrimarySource			3
#define	kV34HDXTurnOffFromPrimaryDestination	4

/* V70 setup maps */

#define	kV70Direction				0x00000001		/* Bit 0  */
#define	kV70uIHEnabled			    0x00000002		/* Bit 1  */
#define	kV70AudioHeaderEnabled	    0x00000004		/* Bit 2  */
#define kV70SilenceSupprEnabled     0x00000008		/* Bit 3  */

#define	kV70SuspendResumeShift      4 
#define	kV70SuspendResumeMask	    (3 << kV70SuspendResumeShift)
#define	kV70SuspendResumeDisabled   0x00000000		/* Bit 4,5  */
#define	kV70SuspendResumeWAddr      0x00000010		/* Bit 4  */
#define	kV70SuspendResumeWoAddr     0x00000020		/* Bit 5  */

#define	kV70CrcLengthShift          6 
#define	kV70CrcLengthMask	        (3 << kV70CrcLengthShift)
#define	kV70CrcLength16             0x00000000		/* Bit 6,7  */
#define	kV70CrcLength8              0x00000040		/* Bit 6  */
#define	kV70CrcLength32             0x00000080		/* Bit 7  */

#define	kV70BlockingFactorShift     8 
#define	kV70BlockingFactorMask	    (3 << kV70BlockingFactorShift)
#define	kV70BlockingFactor1         0x00000000		/* Bit 8,9  */
#define	kV70BlockingFactor2         0x00000100		/* Bit 8  */
#define	kV70BlockingFactor3			0x00000200		/* Bit 9  */
#define	kV70BlockingFactor4			0x00000300		/* Bit 8,9  */

#define kV70InitChannelsShift		10
#define kV70InitChannelsMask		(1 << kV70InitChannelsShift)
#define	kV70InitNoChannels			0x00000000		/* Bit 10,11  */
#define	kV70InitDataChannel			0x00000400		/* Bit 10,11  */
#define	kV70InitAudioChannel		0x00000800		/* Bit 10,11  */
#define	kV70InitBothChannels		0x00000C00		/* Bit 10,11  */

#define kV70OOBEnabled				0x00001000		/* Bit 12 */

/* V80 setup maps */

#define	kV80Direction				0x00000001		/* Bit 0  */

#define	kV80ModeShift				1 
#define	kV80ModeMask				(3 << kV80ModeShift)
#define	kV80SyncMode				(0 << kV80ModeShift)
#define	kV80TunnellingMode			(1 << kV80ModeShift)
#define	kV80SamMode					(2 << kV80ModeShift)
#define	kV80SamTransparentMode		(2 << kV80ModeShift)
#define	kV80SamFramedMode			(3 << kV80ModeShift)

#define	kV80TransIdleShift			3 
#define	kV80TransIdleMask			(3 << kV80TransIdleShift)
#define	kV80TransIdleNoHunt			(0 << kV80TransIdleShift)
#define	kV80TransIdleHunt8			(1 << kV80TransIdleShift)
#define	kV80TransIdleHunt16			(2 << kV80TransIdleShift)

#define	kV80FrameIdleShift			5 
#define	kV80FrameIdleMask			(1 << kV80FrameIdleShift)
#define	kV80FrameIdleFlags			(0 << kV80FrameIdleShift)
#define	kV80FrameIdleMarks			(1 << kV80FrameIdleShift)

#define	kV80FrameUnOvShift			6 
#define	kV80FrameUnOvMask			(1 << kV80FrameUnOvShift)
#define	kV80FrameUnOvAbort			(0 << kV80FrameUnOvShift)
#define	kV80FrameUnOvFlag			(1 << kV80FrameUnOvShift)
 
#define	kV80HdAutoShift				7 
#define	kV80HdAutoMask				(1 << kV80HdAutoShift)
#define	kV80HdAutoNormal			(0 << kV80HdAutoShift)
#define	kV80HdAutoExtended			(1 << kV80HdAutoShift)

#define	kV80CrcTypeShift			8 
#define	kV80CrcTypeMask				(3 << kV80CrcTypeShift)
#define	kV80NoCrc					(0 << kV80CrcTypeShift)
#define	kV80Crc16					(1 << kV80CrcTypeShift)
#define	kV80Crc32					(2 << kV80CrcTypeShift)

#define	kV80NrziShift				10
#define	kV80NrziMask				(1 << kV80NrziShift)
#define	kV80NrziDisabled			(0 << kV80NrziShift)
#define	kV80NrziEnabled				(1 << kV80NrziShift)

#define kV80Syn1Mask				0x00FF0000		/* Bit 16-23  */
#define kV80Syn1Shift				16
#define kV80Syn2Mask				0xFF000000		/* Bit 24-31  */
#define kV80Syn2Shift				24

/* kStartCallProgressMonitorCmd setup masks */

#define	kDTMFDetectorDebouncerEnabled			0x0001
#define	kModemSignalDetectorDebouncerEnabled	0x0002
#define	kCallProgressDetectorDebouncerEnabled	0x0004
#define	kCustomSignalDebouncerEnabled			0x0008
#define	kFaxCallingToneSuppressionEnabled		0x0010
#define	kDataCallingToneSuppressionEnabled		0x0020
#define	kCISuppressionEnabled					0x0040
#define	kAnsSuppressionEnabled					0x0080

/* kDialCmd setup masks (dialerSetup bit fields) */

#define	kDTMFDialingEnabled						0x0001
#define	kPulseDialingEnabled					0x0002
#define	kModeSwitchEnabled						0x0004
#define	kBlindDialingEnabled					0x0008
#define	kPulseDialingMethodMask					0x0030
#define	kDialModifierTranslationMask			0x00C0
#define	kFlashWhilePulseDialingEnabled			0x0100

/* Pulse dialing method */
#define	kPulseDialingNPulsesPerDigit			0x0000
#define	kPulseDialingNplusOnePulsesPerDigit		0x0010
#define	kPulseDialingTenMinusNPulsesPerDigit	0x0020

/* Dial modifier translation */
#define	kTreatWasPause							0x0040	/* Tread 'W' modifier as pause */
#define	kTreatCommaAsWaitForDialtone			0x0080

#ifdef TI_C6X
#include "C6xDefs.h"
#endif
#ifdef PENTIUM_MMX
#include "PentiumDefs.h"
#endif


#if defined(DSP16K) && !defined(SoftModemGlobals)
/* ensure that code generator does not use r5 */
register int *softmodem_h_should_not_be_included_after_softmodem_gh asm("r5");
#endif

/****************************************************************************/
/*	3.	Interface functions.												*/
/*																			*/
/****************************************************************************/

#ifdef ADSL_MODEM

#ifndef SoftDslHeader
#include "SoftDsl.h"
#endif
extern	char*	SM_DECL SoftModemGetRevString(void);
extern	char*	SM_DECL SoftModemGetVersionString(void);
extern	int		SM_DECL SoftModemGetVerMjNum(void);
extern	int		SM_DECL SoftModemGetVerMnNum(void);
extern	int		SM_DECL SoftModemGetVerPatchNum(void);
extern	char*	SM_DECL SoftModemGetProductName(void);
extern	char*	SM_DECL	SoftModemGetBuildDate(void);
extern	char*	SM_DECL SoftModemGetFullManufacturerName(void);
extern	char*	SM_DECL SoftModemGetShortManufacturerName(void);
extern	int		SM_DECL SoftModemRevStringSize(void);
extern	int		SM_DECL SoftModemVersionStringSize(void);
extern	char*	SM_DECL SoftModemGetVendorIDString(void);
extern	char*	SM_DECL SoftModemGetT1413VendorIDString(void);
extern	char*	SM_DECL SoftModemGetSerialNumberString(void);
extern	int		SM_DECL SoftModemSerNumStringSize(void);
#define	SoftDslGetProductName			SoftModemGetProductName
#define	SoftDslGetBuildDate				SoftModemGetBuildDate
#define	SoftDslGetFullManufacturerName	SoftModemGetFullManufacturerName
#define	SoftDslGetShortManufacturerName	SoftModemGetShortManufacturerName

#else /* !ADSL_MODEM */

extern void		SM_DECL SoftModemSetMemoryPtr	(void	*varsPtr);
extern void*	SM_DECL SoftModemGetMemoryPtr	(void);
extern void		SM_DECL SoftModemSetRefData		(void	*varsPtr);
extern void*	SM_DECL SoftModemGetRefData		(void);
extern int		SM_DECL SoftModemGetMemorySize	(void);
extern void		SM_DECL SoftModemInit			(void);
extern void		SM_DECL SoftModemReset			(void);
extern void		SM_DECL SoftModemLineHandler	(int sampleCount, short *srcPtr, short *dstPtr);
extern void		SM_DECL SoftModemTimer			(int timeQ24ms);
extern Boolean	SM_DECL SoftModemCommandHandler	(modemCommandStruct *cmdPtr);
extern int		SM_DECL SoftModemGetExternalMemorySize(void);
extern void		SM_DECL SoftModemSetExternalMemoryPtr(void	*varsPtr);

extern void		SM_DECL SoftModemSetPcmCoding	(pcmCodingType pcmCoding);
extern void		SM_DECL SoftModemPcmLineHandler	(int sampleCount, uchar *srcPtr, uchar *dstPtr);

/* SoftModem IO functions	*/
extern int		SM_DECL SoftModemWrite(int nBytes, uchar* srcPtr);
extern int		SM_DECL SoftModemRead(int nBytes, uchar* dstPtr);
extern int		SM_DECL SoftModemWriteFrame(int nBytes, uchar* srcPtr);
extern int		SM_DECL SoftModemReadFrame(int maxFrameSize, uchar* dstPtr);
extern int		SM_DECL SoftModemCountWritePending(void);
extern int		SM_DECL SoftModemCountReadPending(void);
extern int		SM_DECL SoftModemWriteSpaceAvailable(void);
extern void		SM_DECL SoftModemWriteFlush(void);
extern void		SM_DECL SoftModemReadFlush(void);
extern int		SM_DECL SoftModemGetWriteBufferSize(void);
extern int		SM_DECL SoftModemGetReadBufferSize(void);

#ifdef AUDIO
extern int		SM_DECL SoftModemAudioHandler(int sampleCount, short *srcPtr, short *dstPtr);
extern int		SM_DECL SoftModemAudioRxDataHandler(int nBytes, uchar* srcPtr);
extern int		SM_DECL SoftModemAudioTxDataHandler(int nBytes, uchar* dstPtr);
#endif


#define	SoftModemSetGlobalPtr	SoftModemSetMemoryPtr
#define	SoftModem				SoftModemLineHandler
#ifndef LINKLAYER_V42BIS_LARGE_DICTIONARY
#define	kSoftModemMaxMemorySize	(65536)
#else
#define	kSoftModemMaxMemorySize	(65536 + 8192)
#endif

/*
 * Internal functions
 */
extern	int	SM_DECL SoftModemGetDCOffset(void);
extern	void	SM_DECL SoftModemDisableDCOffsetTracking(void);
extern	void	SM_DECL SoftModemEnableDCOffsetTracking(void);
extern	int	SM_DECL SoftModemGetRcvPower(void);
extern	uint	SM_DECL SoftModemGetHybridDelay(void);
extern  void	SM_DECL SoftModemStatusHandler	(modemStatusStruct *status);
extern Boolean	SM_DECL SoftModemInternalCommandHandler	(modemCommandStruct *cmdPtr);
extern  void	SM_DECL	SoftModemInternalStatusHandler	(modemStatusStruct *status);
extern	void	SM_DECL SoftModemSetControllerOnlyMode(commandHandlerType externalDataPumpCommandHandlerPtr);
extern	char*	SM_DECL SoftModemGetRevString(void);
extern	char*	SM_DECL SoftModemGetVersionString(void);
extern	int		SM_DECL SoftModemGetVerMjNum(void);
extern	int		SM_DECL SoftModemGetVerMnNum(void);
extern	int		SM_DECL SoftModemGetVerPatchNum(void);
extern	char*	SM_DECL SoftModemGetProductName(void);
extern	char*	SM_DECL	SoftModemGetBuildDate(void);
extern	char*	SM_DECL SoftModemGetFullManufacturerName(void);
extern	char*	SM_DECL SoftModemGetShortManufacturerName(void);
extern	int		SM_DECL SoftModemRevStringSize(void);
extern	int		SM_DECL SoftModemVersionStringSize(void);
extern	char*	SM_DECL SoftModemGetVendorIDString(void);
extern	char*	SM_DECL SoftModemGetSerialNumberString(void);
extern  void	SM_DECL SoftModemAuxTxDataHandler(int nBytes, uchar *dataPtr);
extern	void	SM_DECL SoftModemAuxRxDataHandler(int nBytes, uchar *dataPtr);
extern  void	SM_DECL SoftModemTxDataHandler(int nBytes, uchar *dataPtr);
extern	void	SM_DECL SoftModemRxDataHandler(int nBytes, uchar *dataPtr);
extern	void	SM_DECL SoftModemATPrintf(uchar *format, void *arg1, void *arg2, void *arg3);

#define	SoftModemSetInputSaturationLimit(limit)		(gSystemVars.inputSignalLimit = limit)	
#define	SoftModemResetInputSaturationLimit()		(gSystemVars.inputSignalLimit = 0)

#endif	/* !ADSL_MODEM */

#endif	/* SoftModemPh */
