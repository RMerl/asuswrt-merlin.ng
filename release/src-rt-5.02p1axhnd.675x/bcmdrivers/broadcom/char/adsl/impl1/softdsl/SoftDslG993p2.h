/****************************************************************************
 *
 * SoftDslG993p2.h
 *
 *
 * Description:
 *	This file contains definitions for G993p2 configuration
 *
 * $Revision: 1.169 $
 *
 * $Id: SoftDslG993p2.h,v 1.169 2013/02/21 22:43:01 ilyas Exp $
 *
 * $Log: SoftDslG993p2.h,v $
 * Revision 1.169  2013/02/21 22:43:01  ilyas
 * Made AttnDr computation method configurable
 *
 * Revision 1.168  2013/01/29 21:44:17  cnpeng
 * Define BJ.12.3.20 AFE
 *
 * Revision 1.167  2013/01/21 14:43:47  ovandewi
 * PR7124: remove warnings
 *
 * Revision 1.166  2013/01/10 22:29:00  linyin
 * PR3856 Add frequency jump test mode for python script
 *
 * Revision 1.165  2013/01/04 22:26:18  rgreenf
 * PR7111: add pll ppm offset test mode
 *
 * Revision 1.164  2012/11/09 19:37:28  cnpeng
 * Support A.5.2.3 AFE
 *
 * Revision 1.163  2012/10/05 13:04:17  ovandewi
 * PR6882: add field for ripolicy
 *
 * Revision 1.162  2012/09/28 18:13:57  ovandewi
 * PR6988: add driver bit
 *
 * Revision 1.161  2012/09/17 22:31:51  cnpeng
 * Support A.5.1.4
 *
 * Revision 1.160  2012/09/10 09:49:18  ovandewi
 * PR6748: handle g993p5 fully
 *
 * Revision 1.159  2012/09/04 21:39:33  cnpeng
 * define AFE B.5.1.2 and BJ.5.1.2
 *
 * Revision 1.158  2012/09/04 21:23:54  cnpeng
 * Add AFE check FW for A.7.2.21
 *
 * Revision 1.157  2012/09/03 09:08:39  ovandewi
 * PR6881: support vectoring friendly
 *
 * Revision 1.156  2012/08/22 18:20:45  ilyas
 * Made data dumps work with buffers allocated by DSL driver
 *
 * Revision 1.155  2012/08/16 22:14:22  ilyas
 * Added AFE 5.1.4 design (for 63167)
 *
 * Revision 1.154  2012/07/27 01:15:41  lockem
 * Remove 6368a0 support.
 *
 * Revision 1.153  2012/07/06 13:49:40  ovandewi
 * PR6815: bit driver -> PHY
 *
 * Revision 1.152  2012/05/29 21:28:13  ovandewi
 * PR6696: add driver bit and support function
 *
 * Revision 1.151  2012/05/23 23:16:15  nino
 * FWDSLCPEPHY-6670: add pin swap functionality for 2nd 6302 line driver.
 *
 * Revision 1.150  2012/05/18 17:21:19  nino
 * FWDSLCPEPHY-6670: add support for another 6302 LD control mode for 63268 and 6306 whereby bcm63268 anaReg14 bits 2 and 3 are used for power supply on/off and vdsl/adsl mode respectively.
 *
 * Revision 1.149  2012/05/03 21:07:41  nino
 * FWDSLCPEPHY-6623: add LD 6302/6303 related defines.
 *
 * Revision 1.148  2012/04/27 20:25:42  rwdu
 * fixed lb and ETR.
 *
 * Revision 1.147  2012/04/19 21:39:15  rgreenf
 * PR5665: re-work of SRA/BS to implement L x8
 *
 * Revision 1.146  2012/04/09 03:48:48  rwdu
 * report Da GINP SRA new framing parameters and data rate.
 * export QrxBuffer vaiable from  ginp rate selection function.
 *
 * Revision 1.145  2012/04/07 20:29:09  rwdu
 * export newN and newQrx for DS GINP SRA.
 *
 * Revision 1.144  2012/04/06 23:27:57  rgreenf
 * PR6575/6577 add VDSL agc force function and fix ALB afe compensation bug
 *
 * Revision 1.143  2012/03/22 22:42:27  cnpeng
 * Check in AFE A.5.1.3
 *
 * Revision 1.142  2012/03/16 18:25:29  rwdu
 * ginp_sra support for vdsl (still work in progress).
 *
 * Revision 1.141  2012/03/02 22:51:37  nino
 * FWDSLCPEPHY-6488: add 6318 target support.
 *
 * Revision 1.140  2012/02/02 22:58:06  cnpeng
 * Check in AFEID B.7.2.30 and J.7.2.30
 *
 * Revision 1.139  2011/12/20 20:02:45  cnpeng
 * define AFE A.12.3.20
 *
 * Revision 1.138  2011/12/19 23:24:05  yinboli
 * FW6328: Change driver bit and move CO4 check to interop struct
 *
 * Revision 1.137  2011/11/22 18:42:25  sgote
 * F-6323 EVLT-F interop .. Removed the driver bit and added a interop bit where we do not send the A-7 byte in the first CLR.
 *
 * Revision 1.136  2011/11/15 17:43:01  sgote
 * F-6323 6368 VDSL stuck at HS against EVLT-F - Added driver bit to disable GHS Amendment-7 support
 *
 * Revision 1.135  2011/11/14 23:19:29  cnpeng
 * Create AID for A.12.3.30
 *
 * Revision 1.134  2011/11/11 16:57:12  jknittel
 * FWDSLCPEPHY-6260: Wrong Annex is reported in VDSL2
 *
 * Revision 1.133  2011/11/08 00:48:24  rgreenf
 * PR6155: implement amd7 kl0 measurement
 *
 * Revision 1.132  2011/10/12 02:39:17  ilyas
 * Added definitions to support 63268D0
 *
 * Revision 1.131  2011/10/06 09:47:48  ovandewi
 * PR6238: make registers unsigned
 *
 * Revision 1.130  2011/09/29 23:38:02  yinboli
 * FW6112: Add support for Two-Chip bonding
 *
 * Revision 1.129  2011/09/12 23:06:53  cnpeng
 * define new AFE type for BJ.7.2.30
 *
 * Revision 1.128  2011/08/21 21:09:43  ilyas
 * Added PHy/Drv exchange structure and commands for external bonding discovery
 *
 * Revision 1.127  2011/08/18 19:38:22  rgreenf
 * PR5941: implement line probe in VDSL
 *
 * Revision 1.126  2011/08/04 00:28:43  ilyas
 * Added definitions for LD control bits configuration
 *
 * Revision 1.125  2011/07/25 19:33:22  wiese
 * Add ADSL QLN/Hlog compensation for bonding path A.7.2.21 AFEID.
 *
 * Revision 1.124  2011/07/22 21:10:18  cnpeng
 * Define AID for M.7.2.30
 *
 * Revision 1.123  2011/07/22 00:11:25  wiese
 * Add AFEID for A.7.2.21 and add VDSL AFE compensation.
 *
 * Revision 1.122  2011/06/01 16:13:20  jknittel
 * FWDSLCPEPHY-5654: DSL Modem Tuning for VDSL over Coax I/F
 *
 * Revision 1.121  2011/06/01 01:24:32  wiese
 * Add AFEID AFE_FE_COMB_A_12_40 for A.12.40.
 *
 * Revision 1.120  2011/05/19 01:02:37  lockem
 * Replace SoftDslGetTestConfig with a macro.
 *
 * Revision 1.119  2011/05/13 17:05:04  raman
 * add support for 63268 C0 version
 *
 * Revision 1.118  2011/04/28 16:29:59  jknittel
 * FWDSLCPEPHY-5690: Modified Tx Power Control, RX Compensation, & new TX Filter for BJ 5.1.1 modem
 *
 * Revision 1.117  2011/03/30 19:34:27  wiese
 * Add 30a board rev A.12.21 AFEID.
 *
 * Revision 1.116  2011/03/24 17:26:31  cnpeng
 * Rename A.7.2.20 to A.7.2.30
 *
 * Revision 1.115  2011/03/17 22:23:38  ilyas
 * Ported DiagsParser to Linux
 *
 * Revision 1.114  2011/03/16 22:15:21  rgreenf
 * PR2434: bring up ShowTime SOS
 *
 * Revision 1.113  2011/03/16 18:51:56  nino
 * FWDSLCPEPHY-5540: fix SoftBcm6306AnalogIsRxHpfOn() function to return filter state from copy of register contents stored in state structure.
 *
 * Revision 1.112  2011/03/03 03:12:35  tonytran
 * Implement File Overlapped IO, reduce non-page memory usage and combine short statues before submit for writing into a file
 *
 * Revision 1.111  2011/02/28 23:44:49  cnpeng
 * CHECK in AFEID A.7.2.20 related changes
 *
 * Revision 1.110  2011/02/08 02:04:05  lockem
 * Real time improvements.
 *
 * Revision 1.109  2011/02/04 19:24:30  nino
 * FWDSLCPEPHY-5379: add SoftDslIs63268A0(), SoftDslIs63268B0() and SoftDslIs63268() macros.
 *
 * Revision 1.108  2011/01/29 00:08:05  mhegde
 * FW-5193: Initialise ANNEX_BJ params
 *
 * Revision 1.107  2010/12/21 16:20:01  rgreenf
 * PR5268: add driver bit for Telefonica PCB
 *
 * Revision 1.106  2010/12/20 16:59:13  rgreenf
 * PR5268: add driver bit for Telefonica PCB
 *
 * Revision 1.105  2010/11/22 22:42:58  nino
 * FWDSLCPEPHY-5021: code/Makefile changes related to bcm63268 low-level api integration.
 *
 * Revision 1.104  2010/09/17 01:21:15  cnpeng
 * Create new AFEID for A.5.1.2
 *
 * Revision 1.103  2010/07/13 17:32:04  rgreenf
 * PR4760: add driver bit to disable V43 tone set
 *
 * Revision 1.102  2010/06/09 06:15:40  xli
 * FW4659: Fix Annex J Mask selection in Test Mode
 *
 * Revision 1.101  2010/05/21 20:20:45  cnpeng
 * Check in AFEID support
 *
 * Revision 1.100  2010/05/17 17:00:54  rwdu
 * report new U to the driver during SRA.
 *
 * Revision 1.99  2010/04/30 02:55:37  lockem
 * Move FramerDeframerOptions definition to SoftDsl.h so that it is visible
 * to all builds.
 *
 * Revision 1.98  2010/04/29 00:15:19  linyin
 * PR4370: Fix a typo for AFEID
 *
 * Revision 1.97  2010/04/28 23:23:25  linyin
 * PR4370: Implement 6367 AFEID
 *
 * Revision 1.96  2010/04/23 22:17:10  linyin
 * PR4370: Fix some typo for ADSL AFE frontend revision
 *
 * Revision 1.95  2010/04/23 15:56:41  cnpeng
 * Add J.7.2.1 and BJ.7.2.1 def for combo boards with 6302
 *
 * Revision 1.94  2010/04/22 23:55:43  rwdu
 * anticipate the afeid support for 6362/28.
 *
 * Revision 1.93  2010/04/15 03:51:06  rwdu
 * syncGain buffer should be signed when IFTN sync workaround is turned on.
 *
 * Revision 1.92  2010/04/09 16:40:42  jboxho
 * Restore and further implement  VDSL G.Inp
 *
 * Revision 1.91  2010/04/08 03:47:35  rwdu
 * align AnnexM to #define value.
 *
 * Revision 1.90  2010/04/07 17:38:59  ilyas
 * Fixed AFEid macro names and fixed default initialization for 6362/28
 *
 * Revision 1.89  2010/04/06 23:15:46  ilyas
 * Defined AFEid for new boards and macros to check
 *
 * Revision 1.88  2010/03/31 18:10:28  rwdu
 * firmware changes for 68 AnnexM board (which is based on AnnexJ board).
 *
 * Revision 1.87  2010/03/29 14:56:58  ovandewi
 * PR4441: define flag
 *
 * Revision 1.86  2010/03/11 02:02:22  lockem
 * Align field names with E14 code.
 * Replace implicit union with anonymous union.
 *
 * Revision 1.85  2010/02/18 22:30:01  jinlu
 * FW4287 use psd template to compute power from CNXT based DSLAMs since tssi is not set correctly by the DSLAMs
 *
 * Revision 1.84  2010/01/26 09:11:40  ovandewi
 * PR3904: define status before merge
 *
 * Revision 1.83  2009/12/11 16:38:24  jboxho
 * Unify PhyR/Ginp (first step)
 *
 * Revision 1.82  2009/11/18 09:53:24  ovandewi
 * PR33430, PR33303: book driver bits
 *
 * Revision 1.81  2009/11/12 00:21:14  rgreenf
 * PR33413: merge dynamic D from 16 branch to TOT
 *
 * Revision 1.80  2009/11/03 21:14:13  nino
 * Add SoftDslIs6362() and SoftDslIs6328() macros.
 *
 * Revision 1.79  2009/10/30 22:42:55  jinlu
 * PR32848-a move vdsl-only bitmap in cfgFlags
 *
 * Revision 1.78  2009/10/23 22:22:24  linyin
 * PR33360: Add LD6301 line driver ID
 *
 * Revision 1.77  2009/10/23 21:00:08  rwdu
 * SRA dynamic framing support (xmt path).
 *
 * Revision 1.76  2009/10/22 02:48:20  rwdu
 * integrate Rcv dynamic framing calculation into SRA.
 *
 * Revision 1.75  2009/09/28 22:32:57  rgreenf
 * PR33224: move driver bit to cfgFlags
 *
 * Revision 1.74  2009/09/15 17:35:12  nino
 * Fix a comment.
 *
 * Revision 1.73  2009/09/15 04:32:03  mding
 * add D_sweep flag to control sweep direction
 *
 * Revision 1.72  2009/09/02 23:53:34  jinlu
 * PR32329 add support of D change in SRA
 *
 * Revision 1.71  2009/08/27 21:58:13  nino
 * Add phaseError and VCOAdjInfo fields to ntrCnt structure. Add ntrCfgStruct and related constants.
 *
 * Revision 1.70  2009/07/31 23:49:05  mding
 * add DS Dynamic D control variables
 *
 * Revision 1.69  2009/07/31 03:26:36  mding
 * record old L when L changes with Dynamic D
 *
 * Revision 1.68  2009/07/30 01:49:31  mding
 * add US Dynamic D variables
 *
 * Revision 1.67  2009/07/23 00:26:42  mding
 * add N to rxBigi for Dynamic D control
 *
 * Revision 1.66  2009/07/16 02:59:53  ilyas
 * Follow VDSL cfgFlags TPS_TC configuration
 *
 * Revision 1.65  2009/07/02 22:48:53  rwdu
 * max US tone should be target specific.
 *
 * Revision 1.64  2009/06/30 21:52:35  ovandewi
 * PR33016: Annex J board Id
 *
 * Revision 1.63  2009/06/25 15:09:57  ovandewi
 * PR32873: ANNEXJ front end family
 *
 * Revision 1.62  2009/03/13 21:12:48  ilyas
 * Added NTR control and counters report
 *
 * Revision 1.61  2009/02/25 19:24:20  linyin
 * PR32516: Fix xmt gain overflow from E14 side
 *
 * Revision 1.60  2009/02/18 06:28:14  ilyas
 * Added API function to retrieve local capabilies
 *
 * Revision 1.59  2009/02/05 01:45:59  cnpeng
 * Check in TX filters for time domain US0 in EU-36 to EU-64
 *
 * Revision 1.58  2009/01/29 22:11:45  rgreenf
 * PR32505: merge Ikanos driver bit control from AvC011_branch
 *
 * Revision 1.57  2009/01/20 19:54:52  rgreenf
 * PR32432: implement tone/bitrate clamping mechanism
 *
 * Revision 1.56  2009/01/09 18:37:33  jinlu
 * PR32042 add high crest test signal
 *
 * Revision 1.55  2008/11/17 19:08:57  ilyas
 * Added AdeId definition for 6302 frontend designs
 *
 * Revision 1.54  2008/10/24 22:44:13  yongbing
 * PR32195 Provide a driver configuration bit to control ADSL/VDSL toggling in G994
 *
 * Revision 1.53  2008/10/17 23:20:26  jinlu
 * PR32201 update xmt sync gain after bs
 *
 * Revision 1.52  2008/08/27 13:56:58  ovandewi
 * PR31845: macros to recognize POTS/ISDN HW
 *
 * Revision 1.51  2008/08/22 00:47:55  rgreenf
 * src/Adsl/Main/SoftDsl.h
 *
 * Revision 1.50  2008/08/16 01:06:10  ilyas
 * Get rid of special test target. Create common test configuration structure
 *
 * Revision 1.49  2008/08/14 20:52:57  rwdu
 * replace afe reset workaround with afe reset for B1 chips.
 *
 * Revision 1.48  2008/08/13 22:59:29  ilyas
 * Added macros to access afeInfo data
 *
 * Revision 1.47  2008/07/31 22:42:36  rgreenf
 * add status reports for annex and profile
 *
 * Revision 1.46  2008/07/24 20:18:48  ilyas
 * Added AFE frontend definitions to distinguish between 1556 multimode vs. VDSL board
 *
 * Revision 1.45  2008/07/22 23:28:01  ilyas
 * Changed AfeID definitions
 *
 * Revision 1.44  2008/07/11 22:39:59  ilyas
 * Added initial afeInfo initialization
 *
 * Revision 1.43  2008/07/11 18:50:09  rgreenf
 * add detail to afe descriptor structure
 *
 * Revision 1.42  2008/07/09 20:45:17  ilyas
 * Started framework for configurable AFE info
 *
 * Revision 1.41  2008/06/15 00:57:48  ovandewi
 * Name 6368/G993p2 cfg flags
 *
 * Revision 1.40  2008/05/20 19:34:53  jinlu
 * PR31608 enabls ds bs
 *
 * Revision 1.39  2008/05/13 22:21:57  jinlu
 * PR31781 add support to control rateAdaptationFlags from CPE GUI; init xmt/rcv ring buffer with Lmax
 *
 * Revision 1.38  2008/05/13 17:11:41  jinlu
 * PR31781-f enable ds sra for vdsl; add separate control for bs & sra; adapt tone selection with sra direction
 *
 * Revision 1.37  2008/05/09 22:03:40  jinlu
 * PR31781 enable us sra for vdsl
 *
 * Revision 1.36  2008/05/08 19:16:27  jinlu
 * PR31781 partial check-in for vdsl sra; not enabled yet
 *
 * Revision 1.35  2008/04/02 23:22:53  jinlu
 * PR31702 add compensation for AFE TF boost and 0.5dB boost for VCOPE compatibility
 *
 * Revision 1.34  2008/03/28 17:32:33  rgreenf
 * add band plan phase status
 *
 * Revision 1.33  2008/03/11 21:10:58  ilyas
 * Support nitro for 6368 B0, still disabled for now
 *
 * Revision 1.32  2008/02/25 19:31:08  jinlu
 * PR31266 implement ACTATP updating during showtime; PR31441 remove maxPower calculation in showtime
 *
 * Revision 1.31  2008/02/22 21:43:55  mding
 * define ldmode bit in cfgflags
 *
 * Revision 1.30  2008/02/22 09:57:20  jboxho
 * PhyR compilation flag fix
 *
 * Revision 1.29  2008/02/20 23:24:57  ovandewi
 * add field for fext eq upbo
 *
 * Revision 1.28  2008/01/02 19:05:18  jinlu
 * PR31373 add bitswap virtual noise support; disable showtime sync detection during active ds bs; minor improvements of tone selection for ds bs during gain adjustment
 *
 * Revision 1.27  2007/12/04 00:50:05  jinlu
 * PR31381: fix dual latency support for US bitswap; PR31346: remove false US bitswap request rejection
 *
 * Revision 1.26  2007/11/27 20:09:01  jinlu
 * Add support of RX bitswap control commands
 *
 * Revision 1.25  2007/11/19 22:03:02  nino
 * 6348 code modifications to support integration with 6368 low-level code.
 *
 * Revision 1.24  2007/11/15 14:47:40  ovandewi
 * PR31078: add US0 shaping
 *
 * Revision 1.23  2007/11/14 19:33:34  jinlu
 * add sync dump search of best margin tones for polarity check; add feq coeffs ccump before updating with new bi/gi; pipeline qproc dumps based on tx/rx alignment
 *
 * Revision 1.22  2007/11/09 23:52:21  ovandewi
 * further move component out of driver ctrl
 *
 * Revision 1.21  2007/11/09 21:59:27  tonytran
 * PR31302 - Allow PHY to accept configuration from driver
 *
 * Revision 1.20  2007/11/08 22:48:41  jinlu
 * Rx bitswap full algorithm implementation; Add dual latency support; Add NSIF driven restriction control; Add DslDiag controlled rx bitswap enabling/disabling; Add test generator for IOP
 *
 * Revision 1.19  2007/11/06 21:43:37  yongbing
 * PR31149: Get connectionSetup structure pointer directly from SoftDsl.c file
 *
 * Revision 1.18  2007/11/01 22:21:41  ovandewi
 * allow 32 RFI bands
 *
 * Revision 1.17  2007/09/26 21:12:52  jinlu
 * Fix QPROC lockup issue in test shortVdsl5Band9983BandsActiveFast; add more rx bitswap functions
 *
 * Revision 1.16  2007/09/19 19:35:33  jinlu
 * Move bitswap varibles to SlowVar; Implement RejectRequest(); Add Rx bitswap related data stucture/functions
 *
 * Revision 1.15  2007/09/18 22:35:19  dadityan
 * #defines for BandPlan Statuses
 *
 * Revision 1.14  2007/09/12 19:02:37  jinlu
 * Initial implementation of US bitswap
 *
 * Revision 1.13  2007/09/04 07:21:14  tonytran
 * PR31097: 1_28_rc8
 *
 * Revision 1.12  2007/08/24 00:35:05  rgreenf
 * add latency path id
 *
 * Revision 1.11  2007/08/23 20:56:42  rgreenf
 * add extra framer parameter reporting
 *
 * Revision 1.10  2007/08/20 21:34:04  rgreenf
 * added training phase 6368 status reporting - ongoing
 *
 * Revision 1.9  2007/08/20 16:34:19  rgreenf
 * added training phase 6368 status reporting - ongoing
 *
 * Revision 1.8  2007/08/18 00:30:41  jboxho
 * PhyR implementation on 6368 - first step
 *
 * Revision 1.7  2007/08/11 00:44:55  ovandewi
 * PR31078: rework G993p2 common data pump capability struct
 *
 * Revision 1.6  2007/08/10 21:48:38  tonytran
 * Undo previous checked-in
 *
 * Revision 1.5  2007/08/10 16:16:01  tonytran
 * *** empty log message ***
 *
 * Revision 1.4  2007/08/05 19:00:53  ilyas
 * Added overhead message support to VDSL build and status definition for DBPrint
 *
 * Revision 1.3  2007/06/08 06:30:52  ilyas
 * Added API to get negotiated G.994 parameters
 *
 * Revision 1.2  2007/06/02 17:52:24  ilyas
 * Created firmware target for 6368, ongoing work
 *
 * Revision 1.1  2007/06/01 00:39:05  ilyas
 * Moved G993p2 definitions to a separate file for E14 code
 *
 *
 *****************************************************************************/

#ifndef	SoftDslG993p2Header
#define	SoftDslG993p2Header

/* Caution: Do not change anything in this structure definition, including associated constant */
/* This structure definition is used only by the driver and any change impose incompatibility issue in driver */
/* The structure following this structure (g993p2PhyDataPumpCapabilities) can be changed in PHY application */

#define NbandsSupport 5 /* FIXME - This is not the right place for the define - move to correct place */
#define 	MAX_XMT_NBANDS	5
#define 	MAX_RCV_NBANDS	5
#define     MAX_RFI_NBANDS  32

#define   VDSL2_ANNEX_A 0x2
#define   VDSL2_ANNEX_B 0x1
#define   VDSL2_ANNEX_C 0x3
#define		PROFILE8a 		0x1
#define		PROFILE8b 		0x2
#define		PROFILE8c 		0x4
#define		PROFILE8d 		0x8
#define		PROFILE12a 		0x10
#define		PROFILE12b 		0x20
#define		PROFILE17a 		0x40
#define		PROFILE30a 		0x80
#define		PROFILE30e 		0x100
#define		PROFILE70a 		0x200
#define		PROFILEVDSLLR		0x400
#define		PROFILEGFAST106A	0x1000 /* (GFAST_PROFILE_106A <<12) */
#define   PROFILEGFAST212A  0x2000  /* (GFAST_PROFILE_212A <<12) */
#define   PROFILEGFAST106B  0x4000  /* (GFAST_PROFILE_106B <<12) */
#define   PROFILEGFAST106C  0x8000  /* (GFAST_PROFILE_106C <<12) */
#define   PROFILEGFAST212C 0x10000  /* (GFAST_PROFILE_212C <<12) */
#define		NegBandPlan 		0x02
#define		PhyBandPlan 		0x01
#define   DiscoveryPhase  0x00
#define   MedleyPhase     0x01

/* Long Reach VDSL2 mode defines */
#define     VDSL2LR_OFF    0
#define     VDSL2LR_SHORT  1
#define     VDSL2LR_MEDIUM 2
#define     VDSL2LR_LONG   3

/* Long Reach VDSL2 US0 types */
#define     VDSL2LR_ANNEX_A     1
#define     VDSL2LR_ANNEX_M     2
#define     VDSL2LR_ANNEX_B     4

/*cfgFlags bit defines*/
#define         CfgFlagsLdMode                          0x00000001
#define         CfgFlagsFextEqualized                   0x00000002
#define         CfgFlagsRawEthernetDS                   0x00000004
#define         CfgFlagsNoPtmCrcCalc                    0x00000008
#define         CfgFlagsNoG994AVdslToggle               0x00000010
#define         CfgFlagsAlignAfterPeriodics             0x00000020
#define         CfgFlagsVdslNoPtmSupport	              0x00000040
#define         CfgFlagsVdslNoAtmSupport	              0x00000080
#define         CfgFlagsVdslIfxPeriodic	                0x00000100 /* Enable Ifx Periodic start shift offset */
#define         CfgFlagsDynamicDFeatureDisable          0x00000200
#define         CfgFlagsDynamicFFeatureDisable          0x00000400
#define         CfgFlagsSOSFeatureDisable               0x00000800
#define         CfgFlagsVdslIknsHighPwrProfile8Disable  0x00001000
#define         CfgFlagsVdslIknsUs0FullPsdEnable        0x00002000
#define         CfgFlagsDisableVectoring                0x00004000
#define         CfgFlagsEnableATTNDRframingConstrains   0x00008000
#define         CfgFlagsDisableV43                      0x00010000
#define         CfgFlagsExtraPowerCutBack               0x00020000
#define         CfgFlagsVdslLineProbeEnable             0x00040000
#define         CfgFlagsEnableIkanosCO4Interop          0x00080000
#define         CfgFlagsUseCiPolicy2AsDefaultInVDSL2    0x00100000   /* use ciPolicy=2 as default in VDSL2 */
#define         CfgFlagsEnableErrorSamplePacketsCounter 0x00200000
#define         CfgFlagsEnableG993p2AnnexY              0x00400000
#define         CfgFlagsDynamicV43handling              0x00800000
#define         CfgFlagsAttnDrAmd1Enabled               0x01000000
#define         CfgFlagsEnableFDPS_US                   0x02000000
#define         CfgFlagsReserved0                       0x04000000
#define         CfgFlagsEnableSingleLine8KToneMode      0x08000000

typedef struct
	{
	 unsigned char 		selectedProfile ;
	 short 				maxAggTxPwrDsQ4DB, maxAggTxPwrUsQ4DB ;
	 unsigned char 		sprtUs0Reqd ;
	 unsigned int 		minDataRate ;
	 short 				profileDsMaxTone , profileUsMaxTone ;
	 unsigned int 		maxDelay ;
	 unsigned short		maxD ;
	} g993p2ProfileContent;

struct g993p2CommonDataPumpCapabilities
	{
	unsigned short	verId;
	unsigned short	size;

	uint		profileSel;		/* bitMap bit 0 -7 profileselect for 8a,8b,8c,8d,12a,12b,17a,30a respectively */
	uint		maskUS0;
	uint		cfgFlags;

	uint		maskEU;		/* 0->32, 1->36, 2->40, 3->44, 4->48, 5->52, 6->56, 7->60, 8->64 */
	uint		maskADLU;		/* 0->32, 1->36, 2->40, 3->44, 4->48, 5->52, 6->56, 7->60, 8->64 */
	uint		annex;     /* 1->Annex A, 2->Annex B, 4->Annex C */
	};

#if defined(BCM6368_SRC) && !defined(ALGO_MODULE_TEST)
typedef struct
	{
	struct g993p2CommonDataPumpCapabilities;
	} g993p2DataPumpCapabilities;


typedef struct
	{
	struct g993p2CommonDataPumpCapabilities;

    unsigned char		ADL, dsVirtualNoise, lineProbe, ldMode, shapingUS0, fextEqualizedUpbo, g993p5_vector,vectorFriendly, altElectricalLength;
#ifdef G994P1_AMD3_GINP
    unsigned char               g998p4_ginpExt, ginpExt_annexD;
#endif
#ifdef FDPS_UPSTREAM
    unsigned char fdps_us;
#endif
	short 			bandStartUs[MAX_XMT_NBANDS] ;
	short 			bandStopUs[MAX_XMT_NBANDS] ;
	short 			bandStartDs[MAX_RCV_NBANDS] ;
	short 			bandStopDs[MAX_RCV_NBANDS] ;
	short 			bandStartRFI[MAX_RFI_NBANDS] ;
	short 			bandStopRFI[MAX_RFI_NBANDS] ;

	short			nBandsUs;
	short			nBandsDs;
	short			nBandsRFI;

	unsigned char		IDFTsize;
	unsigned short	initialIDFTsize , fillIFFT;

	unsigned short	ceLen_support ;

	uint		maskA_US0;		/* 0 -> A12b ; 1 -> A17a  ; */
	uint		maskB_US0;		/* 0 -> A12b ; 1 -> A17a  ; */

	unsigned char		nitroOn;
	/* tone and rate clamping control parameters
	 * Note: rate data is specified per bearer
	 */
	unsigned int maxUsDataRateKbps[2];
	unsigned int maxDsDataRateKbps[2];
	unsigned int maxAggrDataRateKbps;
	unsigned short maxUsTones;
	unsigned short maxDsTones;
	unsigned short maxAggrTones;
	unsigned char		US0_lcar;
	unsigned char		US0_hcar;
	} g993p2PhyDataPumpCapabilities;
#else

typedef struct g993p2CommonDataPumpCapabilities g993p2DataPumpCapabilities;
typedef struct g993p2CommonDataPumpCapabilities g993p2PhyDataPumpCapabilities;

#endif

/*
**		AFE descriptor
*/

#define AFE_DESC_VER_MJ				0
#define AFE_DESC_VER_MN				1

#define AFE_DESC_VERSION(mj,mn)		((mj) << 8)	| (mn)

/* Board AFE ID bitmap definitions */

#define AFE_CHIP_SHIFT				28
#define AFE_CHIP_MASK				(0xF << AFE_CHIP_SHIFT)
#define AFE_CHIP_INT				1
#define AFE_CHIP_6505				2
#define AFE_CHIP_6306				3
#define AFE_CHIP_CH0        4
#define AFE_CHIP_CH1        5
#define AFE_CHIP_GFAST      6     /* for G.fast only CH0: CH0 Tx + CH0 Rx + CH2 Rx */
#define AFE_CHIP_GFAST0     6     /* same as AFE_CHIP_GFAST but clearly indicate CH0 */
#define AFE_CHIP_GFCH0      7     /* for G.fast/VDSL combo CH0*/
#define AFE_CHIP_GFAST1     8     /* for G.fast only CH1 */
#define AFE_CHIP_GFCH1      9     /* for G.fast/VDSL combo CH1 */
#define AFE_CHIP_MAX				AFE_CHIP_GFCH1
#define AFE_CHIP_INT_BITMAP			(AFE_CHIP_INT << AFE_CHIP_SHIFT)
#define AFE_CHIP_6505_BITMAP		(AFE_CHIP_6505 << AFE_CHIP_SHIFT)
#define AFE_CHIP_6306_BITMAP		(AFE_CHIP_6306 << AFE_CHIP_SHIFT)
#define AFE_CHIP_CH0_BITMAP		  (AFE_CHIP_CH0 << AFE_CHIP_SHIFT)
#define AFE_CHIP_CH1_BITMAP		  (AFE_CHIP_CH1 << AFE_CHIP_SHIFT)
#define AFE_CHIP_RNC_BITMAP		  (AFE_CHIP_RNC << AFE_CHIP_SHIFT)
#define AFE_CHIP_GFAST_BITMAP		(AFE_CHIP_GFAST << AFE_CHIP_SHIFT)
#define AFE_CHIP_GFAST0_BITMAP		(AFE_CHIP_GFAST0 << AFE_CHIP_SHIFT)
#define AFE_CHIP_GFAST1_BITMAP		(AFE_CHIP_GFAST1 << AFE_CHIP_SHIFT)
#define AFE_CHIP_GFCH0_BITMAP		(AFE_CHIP_GFCH0 << AFE_CHIP_SHIFT)
#define AFE_CHIP_GFCH1_BITMAP		(AFE_CHIP_GFCH1 << AFE_CHIP_SHIFT)

#define AFE_CHIP_REV_SHIFT			25
#define AFE_CHIP_REV_MASK			(0x7 << AFE_CHIP_REV_SHIFT)

#define AFE_LD_SHIFT				21
#define AFE_LD_MASK					(0xF << AFE_LD_SHIFT)
#define AFE_LD_ISIL1556				1
#define AFE_LD_6301					2
#define AFE_LD_6302					3
#define AFE_LD_6303					4
#define AFE_LD_MicroSemiLE87281		4  /* reuse when AFE_CHIP_ID = 6 or 7 in gfast mode */
#define AFE_LD_6304					5
#define AFE_LD_6305					6

#define AFE_LD_MAX					AFE_LD_6305
#define AFE_LD_ISIL1556_BITMAP		(AFE_LD_ISIL1556 << AFE_LD_SHIFT)
#define AFE_LD_MicroSemiLE87281_BITMAP		(AFE_LD_MicroSemiLE87281 << AFE_LD_SHIFT)
#define AFE_LD_6301_BITMAP			(AFE_LD_6301 << AFE_LD_SHIFT)
#define AFE_LD_6302_BITMAP			(AFE_LD_6302 << AFE_LD_SHIFT)
#define AFE_LD_6303_BITMAP			(AFE_LD_6303 << AFE_LD_SHIFT)
#define AFE_LD_6304_BITMAP			(AFE_LD_6304 << AFE_LD_SHIFT)
#define AFE_LD_6305_BITMAP			(AFE_LD_6305 << AFE_LD_SHIFT)

#define AFE_LD_REV_SHIFT			18
#define AFE_LD_REV_MASK				(0x7 << AFE_LD_REV_SHIFT)
#define AFE_LD_REV_6303_VR5P3	1
#define AFE_LD_REV_6303_VR5P3_BITMAP (AFE_LD_REV_6303_VR5P3 << AFE_LD_REV_SHIFT)

#define AFE_FE_ANNEX_SHIFT			15
#define AFE_FE_ANNEX_MASK			(0x7 << AFE_FE_ANNEX_SHIFT)
#define AFE_FE_ANNEXX				0
#define AFE_FE_ANNEXA				1
#define AFE_FE_ANNEXB				2
#define AFE_FE_ANNEXJ               3
#define AFE_FE_ANNEXBJ              4
#define AFE_FE_ANNEXM               5
#define AFE_FE_ANNEXC               6
#define AFE_FE_ANNEXA_BITMAP		(AFE_FE_ANNEXA << AFE_FE_ANNEX_SHIFT)
#define AFE_FE_ANNEXB_BITMAP		(AFE_FE_ANNEXB << AFE_FE_ANNEX_SHIFT)
#define AFE_FE_ANNEXJ_BITMAP        (AFE_FE_ANNEXJ << AFE_FE_ANNEX_SHIFT)
#define AFE_FE_ANNEXBJ_BITMAP       (AFE_FE_ANNEXBJ << AFE_FE_ANNEX_SHIFT)
#define AFE_FE_ANNEXM_BITMAP        (AFE_FE_ANNEXM << AFE_FE_ANNEX_SHIFT)
#define AFE_FE_ANNEXX_BITMAP        (AFE_FE_ANNEXX << AFE_FE_ANNEX_SHIFT)
#define AFE_FE_ANNEXC_BITMAP        (AFE_FE_ANNEXC << AFE_FE_ANNEX_SHIFT)

#define AFE_FE_AVMODE_SHIFT			13
#define AFE_FE_AVMODE_MASK			(0x3 << AFE_FE_AVMODE_SHIFT)
#define AFE_FE_AVMODE_COMBO			0
#define AFE_FE_AVMODE_ADSL			1
#define AFE_FE_AVMODE_VDSL			2
#define AFE_FE_AVMODE_COMBO_BITMAP	(AFE_FE_AVMODE_COMBO << AFE_FE_AVMODE_SHIFT)
#define AFE_FE_AVMODE_ADSL_BITMAP	(AFE_FE_AVMODE_ADSL << AFE_FE_AVMODE_SHIFT)
#define AFE_FE_AVMODE_VDSL_BITMAP	(AFE_FE_AVMODE_VDSL << AFE_FE_AVMODE_SHIFT)

#define AFE_FE_REV_SHIFT			8
#define AFE_FE_REV_MASK				(0x1F << AFE_FE_REV_SHIFT)

#define AFE_COAX_SHIFT		    	7
#define AFE_COAX_MASK				(0x1 << AFE_COAX_SHIFT)

#define AFE_FE_RESERVE_SHIFT		0
#define AFE_FE_RESERVE_MASK		(0x7F << AFE_FE_RESERVE_SHIFT)
#define AFE_FE_RESERVE_RNC		0x40
#define AFE_FE_RESERVE_8dBm		0x20

/* obsolete VDSL only */
#define AFE_FE_REV_ISIL_MODE_MASK	0x40
#define AFE_FE_REV_ISIL_MODE_VDSL	0
#define AFE_FE_REV_ISIL_MODE_AVDSL	0x40

/* VDSL only */
#define AFE_FE_REV_ISIL_REV1		1
#define AFE_FE_REV_ISIL_REV_12_21   2   /* Use for A.12.21 which uses the 2/4 RXHPF design */

#define AFE_FE_REV_ISIL_REV1_BITMAP	(AFE_FE_REV_ISIL_REV1 << AFE_FE_REV_SHIFT)

/* combo */
#define AFE_FE_REV_6302_REV1		1
#define AFE_FE_REV_6302_REV_7_12	1
#define AFE_FE_REV_6302_REV_7_2_21  2       // Bonding board 6306 path with additional rx LPF

#define AFE_FE_REV_6302_REV_7_2_1	3
#define AFE_FE_REV_6302_REV_7_2		4
#define AFE_FE_REV_6302_REV_7_2_UR2	5
#define AFE_FE_REV_6302_REV_7_2_2	6
#define AFE_FE_REV_6302_REV_7_2_30	7
#define AFE_FE_REV_ISIL_6302_REV_12_40	8   // 6302 AND 12V driver for ADSL through 30a HW rev A.12.40
#define AFE_FE_REV_6303_REV_12_3_30	9
#define AFE_FE_REV_6303_REV_12_3_20	1
#define AFE_FE_REV_6303_REV_12_3_40	1       // AFE for ch0/ch1 with 30a support
#define AFE_FE_REV_6303_REV_12_3_60	1       // AFE for ch0/ch1 with 30a support, work with 63158
#define AFE_FE_REV_6303_REV_12_3_50	2       // AFE for ch0/ch1 w/o 30a support
#define AFE_FE_REV_6303_REV_12_3_70	3       // AFE for ch0/ch1 w/o 35b support
#define AFE_FE_REV_6303_REV_12_3_35	3       // 63168 + 6303 17a only AFE
#define AFE_FE_REV_6303_MicroSemi_REV_12_50  1   // 63138 AFE for g.fast:
                                                 //   with AFE_CHIP_GFAST: ch0 = X.12.50
                                                 //   with AFE_CHIP_GFCH0: ch0 = X.12.50 and X.12.3.40 switchable
#define AFE_FE_REV_6303_MicroSemi_REV_12_51  2   // 63138 AFE for g.fast:
                                                 //   with AFE_CHIP_GFAST: ch0 = X.12.51
                                                 //   with AFE_CHIP_GFCH0: ch0 = X.12.51 and X.12.3.40 switchable
#define AFE_FE_REV_6304_REV_12_4_40	1       // AFE for ch0/ch1 with 6304.  Ch1 supports A/V17/V35; Ch0 supports GF
#define AFE_FE_REV_6304_REV_12_4_45	2       // AFE for ch0 that supports A/V/G

#define AFE_FE_REV_6304_REV_12_4_60	1       // AFE for ch0/ch1 with 6304.  Ch1 supports A/V17/V35; Ch0 supports GF; work with 63158
#define AFE_FE_REV_6304_REV_12_4_60_1	1       // AFE for ch0/ch1 with 6304.  GF_106; work with 63158
//#define AFE_FE_REV_6304_REV_12_4_60_2	2       // AFE for ch0/ch1 with 6304.  GF_106; work with 63158
#define AFE_FE_REV_6304_REV_12_4_60_2	3       // AFE for ch0/ch1 with 6304.  GF_106; work with 63158
#define AFE_FE_REV_6305_REV_12_5_60_1	1       // AFE for ch0/ch1 with 6305.  GF_106; work with 63158
#define AFE_FE_REV_6305_REV_12_5_60_2	2       // AFE for ch0/ch1 with 6305.  GF_212; work with 63158


/* RNC configurations */
#define AFE_FE_REV_RNC_REV_10_1 1
#define AFE_FE_REV_RNC_REV_10_2 2
#define AFE_FE_REV_RNC_REV_10_3 3
#define AFE_FE_REV_RNC_REV_10_4 4


#define AFE_FE_REV_6302_REV1_BITMAP		(AFE_FE_REV_6302_REV1 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6302_REV_7_12_BITMAP	(AFE_FE_REV_6302_REV_7_12 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6302_REV_7_2_21_BITMAP	(AFE_FE_REV_6302_REV_7_2_21 << AFE_FE_REV_SHIFT)

#define AFE_FE_REV_6302_REV_7_2_1_BITMAP	(AFE_FE_REV_6302_REV_7_2_1 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6302_REV_7_2_BITMAP		(AFE_FE_REV_6302_REV_7_2 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6302_REV_7_2_UR2_BITMAP	(AFE_FE_REV_6302_REV_7_2_UR2 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6302_REV_7_2_2_BITMAP	(AFE_FE_REV_6302_REV_7_2_2 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6302_REV_7_2_30_BITMAP	(AFE_FE_REV_6302_REV_7_2_30 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_ISIL_6302_REV_12_40_BITMAP	(AFE_FE_REV_ISIL_6302_REV_12_40 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6303_REV_12_3_30_BITMAP	(AFE_FE_REV_6303_REV_12_3_30 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6303_REV_12_3_35_BITMAP	(AFE_FE_REV_6303_REV_12_3_35 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6303_REV_12_3_20_BITMAP	(AFE_FE_REV_6303_REV_12_3_20 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6303_REV_12_3_40_BITMAP	(AFE_FE_REV_6303_REV_12_3_40 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6303_REV_12_3_50_BITMAP	(AFE_FE_REV_6303_REV_12_3_50 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6303_REV_12_3_70_BITMAP	(AFE_FE_REV_6303_REV_12_3_70 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6303_MicroSemi_REV_12_50_BITMAP	(AFE_FE_REV_6303_MicroSemi_REV_12_50 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6303_MicroSemi_REV_12_51_BITMAP	(AFE_FE_REV_6303_MicroSemi_REV_12_51 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6304_REV_12_4_40_BITMAP	(AFE_FE_REV_6304_REV_12_4_40 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6304_REV_12_4_45_BITMAP	(AFE_FE_REV_6304_REV_12_4_45 << AFE_FE_REV_SHIFT)

#define AFE_FE_REV_6303_REV_12_3_60_BITMAP	(AFE_FE_REV_6303_REV_12_3_60 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6304_REV_12_4_60_BITMAP	(AFE_FE_REV_6304_REV_12_4_60 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6304_REV_12_4_60_1_BITMAP	(AFE_FE_REV_6304_REV_12_4_60_1 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6304_REV_12_4_60_2_BITMAP	(AFE_FE_REV_6304_REV_12_4_60_2 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6305_REV_12_5_60_1_BITMAP	(AFE_FE_REV_6305_REV_12_5_60_1 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6305_REV_12_5_60_2_BITMAP	(AFE_FE_REV_6305_REV_12_5_60_2 << AFE_FE_REV_SHIFT)

/* ADSL only */

#define AFE_FE_REV_6302_REV_5_2_1	1
#define AFE_FE_REV_6302_REV_5_2_2	2
#define AFE_FE_REV_6302_REV_5_2_3	3
#define AFE_FE_REV_6301_REV_5_1_1	1
#define AFE_FE_REV_6301_REV_5_1_2	2
#define AFE_FE_REV_6301_REV_5_1_3	3
#define AFE_FE_REV_6301_REV_5_1_4	4

#define AFE_FE_REV_6302_REV_5_2_1_BITMAP	(AFE_FE_REV_6302_REV_5_2_1 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6302_REV_5_2_2_BITMAP	(AFE_FE_REV_6302_REV_5_2_2 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6302_REV_5_2_3_BITMAP	(AFE_FE_REV_6302_REV_5_2_3 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6301_REV_5_1_1_BITMAP	(AFE_FE_REV_6301_REV_5_1_1 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6301_REV_5_1_2_BITMAP	(AFE_FE_REV_6301_REV_5_1_2 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6301_REV_5_1_3_BITMAP	(AFE_FE_REV_6301_REV_5_1_3 << AFE_FE_REV_SHIFT)
#define AFE_FE_REV_6301_REV_5_1_4_BITMAP	(AFE_FE_REV_6301_REV_5_1_4 << AFE_FE_REV_SHIFT)

/* derived AFE definitions */

#if 0
#define AFE_FE_FULL_MASK			(AFE_LD_MASK | AFE_FE_ANNEX_MASK | AFE_FE_AVMODE_MASK | AFE_FE_REV_MASK)
#define AFE_FE_FULL_CHIP_MASK			(AFE_CHIP_MASK | AFE_LD_MASK | AFE_FE_ANNEX_MASK | AFE_FE_AVMODE_MASK | AFE_FE_REV_MASK)

#define AFE_FE_COMB_A_7_12			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_12)
#define AFE_FE_COMB_A_7_2_21		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_21)

#define AFE_FE_COMB_M_7_2_1			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_1)
#define AFE_FE_COMB_B_7_2			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXB_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2)
#define AFE_FE_COMB_B_7_2_UR2		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXB_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_UR2)
#define AFE_FE_COMB_J_7_2_1			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_1)
#define AFE_FE_COMB_J_7_2_2			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_2)
#define AFE_FE_COMB_BJ_7_2_2		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_2)
#define AFE_FE_COMB_BJ_7_2_1		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_1)


#define AFE_FE_ADSL_A_5_1_1			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_1)
#define AFE_FE_ADSL_A_5_2_1			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6302_REV_5_2_1)
#define AFE_FE_ADSL_A_5_2_2			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6302_REV_5_2_2)
#define AFE_FE_ADSL_M_5_1_1			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_1)
#define AFE_FE_ADSL_M_5_1_2			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_2)
#define AFE_FE_ADSL_B_5_1_1			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXB_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_1)
#define AFE_FE_ADSL_BJ_5_1_1		(AFE_LD_6301_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_1)

#else

#define AFE_FE_FULL_MASK			(AFE_LD_MASK | AFE_FE_ANNEX_MASK | AFE_FE_AVMODE_MASK | AFE_FE_REV_MASK)
#define AFE_FE_FULL_CHIP_MASK			(AFE_CHIP_MASK | AFE_LD_MASK | AFE_FE_ANNEX_MASK | AFE_FE_AVMODE_MASK | AFE_FE_REV_MASK)

#define AFE_FE_COMB_A_7_12			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_12_BITMAP)
#define AFE_FE_COMB_A_7_2_21		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_21_BITMAP)
#define AFE_FE_COMB_A_7_2_30		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_30_BITMAP)
#define AFE_FE_COMB_M_7_2_30		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_30_BITMAP)
#define AFE_FE_COMB_BJ_7_2_30		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_30_BITMAP)
#define AFE_FE_COMB_B_7_2_30		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXB_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_30_BITMAP)
#define AFE_FE_COMB_J_7_2_30		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_30_BITMAP)
#define AFE_FE_COMB_BJ_12_3_40_0	(AFE_LD_6303_BITMAP | AFE_CHIP_CH0_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_40_BITMAP)
#define AFE_FE_COMB_BJ_12_3_40_1	(AFE_LD_6303_BITMAP | AFE_CHIP_CH1_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_40_BITMAP)

#define AFE_FE_COMB_A_12_40		    (AFE_LD_6302_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_ISIL_6302_REV_12_40_BITMAP)
#define AFE_FE_COMB_A_12_3_30		(AFE_LD_6303_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_30_BITMAP)
#define AFE_FE_COMB_A_12_3_35		(AFE_LD_6303_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_35_BITMAP)
//#define AFE_FE_COMB_A_12_3_20		(AFE_LD_6303_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_20_BITMAP)
#define AFE_FE_COMB_A_12_3_20		(AFE_CHIP_INT_BITMAP | AFE_LD_6303_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_20_BITMAP)
#define AFE_FE_COMB_BJ_12_3_20		(AFE_LD_6303_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_20_BITMAP)
#define AFE_FE_COMB_BJ_12_3_30		(AFE_LD_6303_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_30_BITMAP)
#define AFE_FE_COMB_M_12_3_20		(AFE_LD_6303_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_20_BITMAP)
#define AFE_FE_COMB_M_12_3_30		(AFE_LD_6303_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_30_BITMAP)
#define AFE_FE_COMB_A_12_3_40_0		(AFE_LD_6303_BITMAP | AFE_CHIP_CH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_40_BITMAP)
#define AFE_FE_COMB_A_12_3_40_1		(AFE_LD_6303_BITMAP | AFE_CHIP_CH1_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_40_BITMAP)
#define AFE_FE_COMB_A_12_3VR5P3_40_0		(AFE_LD_REV_6303_VR5P3_BITMAP | AFE_LD_6303_BITMAP | AFE_CHIP_CH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_40_BITMAP)
#define AFE_FE_COMB_A_12_3VR5P3_40_1		(AFE_LD_REV_6303_VR5P3_BITMAP | AFE_LD_6303_BITMAP | AFE_CHIP_CH1_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_40_BITMAP)
#define AFE_FE_COMB_A_12_3_50		  (AFE_LD_6303_BITMAP | AFE_CHIP_CH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_50_BITMAP)
#define AFE_FE_COMB_A_12_3_50_1		  (AFE_LD_6303_BITMAP | AFE_CHIP_CH1_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_50_BITMAP)
#define AFE_FE_COMB_A_12_3_70		  (AFE_LD_6303_BITMAP | AFE_CHIP_CH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_70_BITMAP)
#define AFE_FE_COMB_M_12_3_70		  (AFE_LD_6303_BITMAP | AFE_CHIP_CH0_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_70_BITMAP)
#define AFE_FE_COMB_X_12_50		    (AFE_LD_6303_BITMAP | AFE_CHIP_GFAST_BITMAP | AFE_FE_ANNEXX_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_MicroSemi_REV_12_50_BITMAP)
#define AFE_FE_COMB_A_12_50		      (AFE_LD_6303_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_MicroSemi_REV_12_50_BITMAP)
#define AFE_FE_COMB_BJ_12_50		    (AFE_LD_6303_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_MicroSemi_REV_12_50_BITMAP)
#define AFE_FE_COMB_X_12_51		    (AFE_LD_6303_BITMAP | AFE_CHIP_GFAST_BITMAP | AFE_FE_ANNEXX_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_MicroSemi_REV_12_51_BITMAP)
#define AFE_FE_COMB_A_12_51		      (AFE_LD_6303_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_MicroSemi_REV_12_51_BITMAP)
#define AFE_FE_COMB_BJ_12_51		    (AFE_LD_6303_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_MicroSemi_REV_12_51_BITMAP)
#define AFE_FE_COMB_BJ_12_3_50		  (AFE_LD_6303_BITMAP | AFE_CHIP_CH0_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_50_BITMAP)
#define AFE_FE_COMB_BJ_12_3_50_1    (AFE_LD_6303_BITMAP | AFE_CHIP_CH1_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_50_BITMAP)
#define AFE_FE_COMB_A_12_3_60_0     (AFE_LD_6303_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_60_BITMAP)
#define AFE_FE_COMB_A_12_3_60_1     (AFE_LD_6303_BITMAP | AFE_CHIP_GFCH1_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_60_BITMAP)
#define AFE_FE_COMB_M_12_3_60_0     (AFE_LD_6303_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_60_BITMAP)
#define AFE_FE_COMB_M_12_3_60_1     (AFE_LD_6303_BITMAP | AFE_CHIP_GFCH1_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_60_BITMAP)
#define AFE_FE_COMB_BJ_12_3_60_0     (AFE_LD_6303_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_60_BITMAP)
#define AFE_FE_COMB_BJ_12_3_60_1     (AFE_LD_6303_BITMAP | AFE_CHIP_GFCH1_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_60_BITMAP)
//#define AFE_FE_COMB_M_12_4_60_0     (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_BITMAP)
//#define AFE_FE_COMB_M_12_4_60_1     (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH1_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_BITMAP)

#define AFE_FE_COMB_X_12_4_40		    (AFE_LD_6304_BITMAP | AFE_CHIP_GFAST_BITMAP | AFE_FE_ANNEXX_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_40_BITMAP)
#define AFE_FE_COMB_X_12_4_40_GF    (AFE_LD_6304_BITMAP | AFE_CHIP_GFAST_BITMAP | AFE_FE_ANNEXX_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_40_BITMAP)
#define AFE_FE_COMB_X_12_4_40_V	    (AFE_LD_6304_BITMAP | AFE_CHIP_CH0_BITMAP | AFE_FE_ANNEXX_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_40_BITMAP)
#define AFE_FE_COMB_A_12_4_40		    (AFE_LD_6304_BITMAP | AFE_CHIP_CH1_BITMAP   | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_40_BITMAP)
#define AFE_FE_COMB_A_12_4_45		    (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_45_BITMAP)
#define AFE_FE_COMB_A_12_4_45_GF	  (AFE_LD_6304_BITMAP | AFE_CHIP_GFAST_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_45_BITMAP)
#define AFE_FE_COMB_A_12_4_45_V		  (AFE_LD_6304_BITMAP | AFE_CHIP_CH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_45_BITMAP)
#define AFE_FE_COMB_X_12_4_60_0     (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXX_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_BITMAP)
#define AFE_FE_COMB_X_12_4_60_1     (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH1_BITMAP | AFE_FE_ANNEXX_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_BITMAP)
#define AFE_FE_COMB_A_12_4_60_0	    (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_BITMAP)
#define AFE_FE_COMB_A_12_4_60_1     (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH1_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_BITMAP)
#define AFE_FE_COMB_A_12_4_60_1_0   (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_1_BITMAP)
#define AFE_FE_COMB_A_12_4_60_1_1   (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH1_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_1_BITMAP)
#define AFE_FE_COMB_A_12_4_60_2_0   (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_2_BITMAP)
#define AFE_FE_COMB_A_12_4_60_2_1   (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH1_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_2_BITMAP)
#define AFE_FE_COMB_M_12_4_60_0	    (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_BITMAP)
#define AFE_FE_COMB_M_12_4_60_1     (AFE_LD_6304_BITMAP | AFE_CHIP_GFCH1_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6304_REV_12_4_60_BITMAP)

#define AFE_FE_COMB_X_12_5_60_1_0   (AFE_LD_6305_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXX_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6305_REV_12_5_60_1_BITMAP)
#define AFE_FE_COMB_X_12_5_60_1_1   (AFE_LD_6305_BITMAP | AFE_CHIP_GFCH1_BITMAP | AFE_FE_ANNEXX_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6305_REV_12_5_60_1_BITMAP)
#define AFE_FE_COMB_X_12_5_60_2_0   (AFE_LD_6305_BITMAP | AFE_CHIP_GFCH0_BITMAP | AFE_FE_ANNEXX_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6305_REV_12_5_60_2_BITMAP)
#define AFE_FE_COMB_X_12_5_60_2_1   (AFE_LD_6305_BITMAP | AFE_CHIP_GFCH1_BITMAP | AFE_FE_ANNEXX_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6305_REV_12_5_60_2_BITMAP)


// 12.3.40 AnnexM
#define AFE_FE_COMB_M_12_3_40_0		(AFE_LD_6303_BITMAP | AFE_CHIP_CH0_BITMAP |  AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP  | AFE_FE_REV_6303_REV_12_3_40_BITMAP)
#define AFE_FE_COMB_M_12_3_40_1		(AFE_LD_6303_BITMAP | AFE_CHIP_CH1_BITMAP |  AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP  | AFE_FE_REV_6303_REV_12_3_40_BITMAP)
// 12.3.50 AnnexM
#define AFE_FE_COMB_M_12_3_50	    (AFE_LD_6303_BITMAP | AFE_CHIP_CH0_BITMAP |  AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_50_BITMAP)

// 12.3.40 AnnexC
#define AFE_FE_COMB_C_12_3_40_0     (AFE_LD_6303_BITMAP | AFE_CHIP_CH0_BITMAP |  AFE_FE_ANNEXC_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_40_BITMAP)
#define AFE_FE_COMB_C_12_3_40_1     (AFE_LD_6303_BITMAP | AFE_CHIP_CH1_BITMAP |  AFE_FE_ANNEXC_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6303_REV_12_3_40_BITMAP)


#define AFE_FE_COMB_M_7_2_1			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_1_BITMAP)
#define AFE_FE_COMB_B_7_2			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXB_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_BITMAP)
#define AFE_FE_COMB_B_7_2_UR2		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXB_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_UR2_BITMAP)
#define AFE_FE_COMB_J_7_2_1			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_1_BITMAP)
#define AFE_FE_COMB_J_7_2_2			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_2_BITMAP)
#define AFE_FE_COMB_BJ_7_2_2		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_2_BITMAP)
#define AFE_FE_COMB_BJ_7_2_1		(AFE_LD_6302_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_COMBO_BITMAP | AFE_FE_REV_6302_REV_7_2_1_BITMAP)


#define AFE_FE_ADSL_A_5_1_1			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_1_BITMAP)
#define AFE_FE_ADSL_A_5_1_2			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_2_BITMAP)
#define AFE_FE_ADSL_M_5_1_2			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_2_BITMAP)
#define AFE_FE_ADSL_A_5_1_3			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_3_BITMAP)
#define AFE_FE_ADSL_A_5_1_4			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_4_BITMAP)
#define AFE_FE_ADSL_A_5_2_1			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6302_REV_5_2_1_BITMAP)
#define AFE_FE_ADSL_A_5_2_2			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6302_REV_5_2_2_BITMAP)
#define AFE_FE_ADSL_A_5_2_3			(AFE_LD_6302_BITMAP | AFE_FE_ANNEXA_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6302_REV_5_2_3_BITMAP)
#define AFE_FE_ADSL_M_5_1_1			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXM_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_1_BITMAP)
#define AFE_FE_ADSL_B_5_1_1			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXB_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_1_BITMAP)
#define AFE_FE_ADSL_BJ_5_1_1		(AFE_LD_6301_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_1_BITMAP)
#define AFE_FE_ADSL_B_5_1_2			(AFE_LD_6301_BITMAP | AFE_FE_ANNEXB_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_2_BITMAP)
#define AFE_FE_ADSL_BJ_5_1_2		(AFE_LD_6301_BITMAP | AFE_FE_ANNEXBJ_BITMAP | AFE_FE_AVMODE_ADSL_BITMAP | AFE_FE_REV_6301_REV_5_1_2_BITMAP)

#endif


/* reserved */
#define AFE_RESERVED_SHIFT			0
#define AFE_RESERVED_MASK			(0xFF << AFE_RESERVED_SHIFT)


/* Line driver(s) control bits mapping (afeChidIdConfig0) */

#define AFELD_CTL_INT_PWR_MAP		  0x01
#define AFELD_CTL_INT_MODE_MAP		  0x02
#define AFELD_CTL_INT_PWR_POL	      0x04
#define AFELD_CTL_INT_MODE_POL		  0x08

#define AFELD_CTL_EXT_PWR_MAP		  0x10
#define AFELD_CTL_EXT_MODE_MAP		  0x20
#define AFELD_CTL_EXT_PWR_POL	      0x40
#define AFELD_CTL_EXT_MODE_POL		  0x80

#define AFELD_CTL_INT6302_PAD         0x100
#define AFELD_CTL_EXT6302_PAD         0x200

#define AFELD_CTL_INT_PIN_SWAP        0x400
#define AFELD_CTL_EXT_PIN_SWAP        0x800

/* Line driver generic pin control mapping */

#define kDslAfeLdPinCtl0			  0		/* DSL_CTL_0: AnaReg14[0] */
#define kDslAfeLdPinCtl1			  1		/* DSL_CTL_1: AnaReg14[1] */
#define kDslAfeLdPinCtl2			  2		/* DSL_CTL_2: DhifGpioReg[30] */
#define kDslAfeLdPinCtl3			  3		/* DSL_CTL_3: DhifGpioReg[31] */
#define kDslAfeLdPinCtl4			  4		/* DSL_CTL_4: AnaReg14[2]  */
#define kDslAfeLdPinCtl5			  5		/* DSL_CTL_5: AnaReg14[3]  */

#define kDslAfeLdPinPwrIdx			  0		/* 6302/6303 power */
#define kDslAfeLdPinModeIdx			  1		/* 6302 A/VDSL mode */
#define kDslAfeLdPinClkIdx			  1		/* 6303 dclk */
#define kDslAfeLdPinDataIdx			  2		/* 6303 din */

#define kDslAfeLdPinIdMask			  0x7
#define kDslAfeLdPinPolMask			  0x8

#define DG_PHY_CONFIG_BYPASS_SHIFT          0
#define DG_PHY_CONFIG_BYPASS_SHIFT_MASK     (0x1 << DG_PHY_CONFIG_BYPASS_SHIFT)
#define AFECTL1_LAST_MODE_VDSL             2

#define MAX_AFE_BANDS                 14
#define TX_POWER_MODES                8
#define RX_POWER_MODES                8

#define AFE_TUNING_TX_COMP_ENABLE     0x1
#define AFE_TUNING_RX_COMP_ENABLE     0x2
#define AFE_TUNING_MAX_RX_PWR_ENABLE  0x4
#define AFE_TUNING_MAX_TX_PWR_ENABLE  0x8

struct afeDescStruct
  {
  unsigned short	verId;				/* of the structure itself */
  unsigned short	size;               /* size of the structure   */

  uint   chipId;               /* Global chipID  such as 636800A0 */
  uint   boardAfeId;           /* Bitmaped field - copy of the NVRAM AFE ID */

  uint   afeChidIdConfig0;
  uint   afeChidIdConfig1;     /* bitmap to enable/control specific componion AFE chipset  behaviors */

  uint   afeTuningControl;                /* bitmap to control various compensation, 1 means enable, see AFE_TUNING_XX defs */
  unsigned char   nBandsXmtMax, nBandsRcvMax;

  /* attn tone indeces*/
  unsigned short  txToneIndex[MAX_AFE_BANDS] ;
  unsigned short  rxToneIndex[MAX_AFE_BANDS] ;

  /* attn - 0xffff is zero attenuation */
  unsigned short  txAttn[MAX_AFE_BANDS];
  unsigned short  rxAttn[MAX_AFE_BANDS];

  /* mean passband gains tx and rx - 0x10000 is zero gain */
  unsigned int  meanTxGain;
  unsigned int  meanRxGain;

  /* max Rx Power */
  unsigned short  maxTxPower[TX_POWER_MODES];
  unsigned short  maxRxPower[RX_POWER_MODES];
  };

typedef struct afeDescStruct afeDescStruct;

extern void	*SoftDslGetAfeInfo(void *gDslVars);

#ifdef gAfeInfoVars
#define gAfeInfo(x)   gAfeInfoVars
#else
#define gAfeInfo(x)   (*((afeDescStruct *)SoftDslGetAfeInfo(x)))
#endif

#define SoftDslChipId(x)		(gAfeInfo(x).chipId)
#define SoftDslIs6368(x)		((gAfeInfo(x).chipId & 0xFFFF0000) == 0x63680000)
#define SoftDslIs6368Ax(x)		((gAfeInfo(x).chipId & 0xFFFF00F0) == 0x636800A0)
#define SoftDslIs6368Bx(x)		((gAfeInfo(x).chipId & 0xFFFF00F0) == 0x636800B0)
#define SoftDslIs6368A0(x)		( gAfeInfo(x).chipId 			   == 0x636800A0)
#define SoftDslIs6368B0(x)		( gAfeInfo(x).chipId 			   == 0x636800B0)
#define SoftDslIs6368A0B0(x)   ( (gAfeInfo(x).chipId == 0x636800A0) || ( gAfeInfo(x).chipId == 0x636800B0))
#define SoftDslIs6368B1(x)		( gAfeInfo(x).chipId 			   == 0x636800B1)
#define SoftDslIs6362(x)       ((gAfeInfo(x).chipId & 0xFFFF0000) == 0x63620000)
#define SoftDslIs6328(x)       ((gAfeInfo(x).chipId & 0xFFFF0000) == 0x63280000)
#define SoftDslIs63268A0(x)     ( gAfeInfo(x).chipId 			   == 0x632680A0)
#define SoftDslIs63268B0(x)     ( gAfeInfo(x).chipId 			   == 0x632680B0)
#define SoftDslIs63268C0(x)     ( gAfeInfo(x).chipId 			   == 0x632680C0)
#define SoftDslIs63268GteC0(x)  (SoftDslIs63268(x) && (gAfeInfo(x).chipId >= 0x632680C0))
#define SoftDslIs63268D0(x)     ( gAfeInfo(x).chipId 			   == 0x632680D0)
#define SoftDslIs63268(x)       ((gAfeInfo(x).chipId & 0xFFFFF000) == 0x63268000)
#define SoftDslIs63381A0(x)     ( gAfeInfo(x).chipId 			   == 0x633810a0)
#define SoftDslIs63381A1(x)     ( gAfeInfo(x).chipId 			   == 0x633810a1)
#define SoftDslIs63381B0(x)     ( gAfeInfo(x).chipId 			   == 0x633810b0)

#define SoftDslAfeId(x)			(gAfeInfo(x).boardAfeId)
#define SoftDslAfeChipId(x)		((gAfeInfo(x).boardAfeId & AFE_CHIP_MASK) >> AFE_CHIP_SHIFT)
#define SoftDslAfeChipIdPrm(x)		((gAfeInfo(x).afeDescExt.AfeId1 & AFE_CHIP_MASK) >> AFE_CHIP_SHIFT)
#define SoftDslAfeChipIdSnd(x)		((gAfeInfo(x).afeDescExt.AfeId2 & AFE_CHIP_MASK) >> AFE_CHIP_SHIFT)

#define SoftDslAfeChipIdInt(x)	(SoftDslAfeChipId(x) == AFE_CHIP_INT)
#define SoftDslAfeChipId6505(x)	(SoftDslAfeChipId(x) == AFE_CHIP_6505)
#define SoftDslAfeChipId6306(x)	(SoftDslAfeChipId(x) == AFE_CHIP_6306)
#ifndef BCM63158_SRC
#define SoftDslAfeChipIdCh0(x)	(SoftDslAfeChipId(x) == AFE_CHIP_CH0)
#define SoftDslAfeChipIdCh1(x)	(SoftDslAfeChipId(x) == AFE_CHIP_CH1)
#else
#define SoftDslAfeChipIdCh0(x)	((SoftDslAfeChipId(x) == AFE_CHIP_CH0) || (SoftDslAfeChipId(x) == AFE_CHIP_GFAST0) || (SoftDslAfeChipId(x) == AFE_CHIP_GFCH0))
#define SoftDslAfeChipIdCh1(x)	((SoftDslAfeChipId(x) == AFE_CHIP_CH1) || (SoftDslAfeChipId(x) == AFE_CHIP_GFAST1) || (SoftDslAfeChipId(x) == AFE_CHIP_GFCH1))
#define SoftDslAfeChipIdPRM(x)	((SoftDslAfeChipIdPrm(x) == AFE_CHIP_CH1) || (SoftDslAfeChipIdPrm(x) == AFE_CHIP_GFAST1) || (SoftDslAfeChipIdPrm(x) == AFE_CHIP_GFCH1))
#define SoftDslAfeChipIdSND(x)	((SoftDslAfeChipIdSnd(x) == AFE_CHIP_CH1) || (SoftDslAfeChipIdSnd(x) == AFE_CHIP_GFAST1) || (SoftDslAfeChipIdSnd(x) == AFE_CHIP_GFCH1))
#endif
#define SoftDslAfeChipIdGfast(x) (SoftDslAfeChipId(x) == AFE_CHIP_GFAST)
#ifdef BCM63138_SRC
#ifdef CHIP_SRC
#define SoftDslAfeIsExtPath(x)	SoftDslAfeChipIdCh1(x)	
#else
#define SoftDslAfeIsExtPath(x)	gBcm6368LineId(x)
#endif
#else
#define SoftDslAfeIsExtPath(x)	SoftDslAfeChipId6306(x)
#endif

#define SoftDslAfeLD(x)			((gAfeInfo(x).boardAfeId & AFE_LD_MASK) >> AFE_LD_SHIFT)
#define SoftDslAfeLD1556(x)		(SoftDslAfeLD(x) == AFE_LD_ISIL1556)
#define SoftDslAfeLD6301(x)		(SoftDslAfeLD(x) == AFE_LD_6301)
#define SoftDslAfeLD6302(x)		(SoftDslAfeLD(x) == AFE_LD_6302)
#define SoftDslAfeLD6303(x)		(SoftDslAfeLD(x) == AFE_LD_6303)
#define SoftDslAfeLD6304(x)		(SoftDslAfeLD(x) == AFE_LD_6304)
#define SoftDslAfeLD6305(x)		(SoftDslAfeLD(x) == AFE_LD_6305)
//#define SoftDslAfeLD6303(x)		((SoftDslAfeLD(x) == AFE_LD_6303) && (!SoftDslAfeIsCombX_12_50(x)))
#define SoftDslAfeLDREV(x)		((gAfeInfo(x).boardAfeId & AFE_LD_REV_MASK) >> AFE_LD_REV_SHIFT)
//#define SoftDslAfeLDREV6303V5p3(x)		(SoftDslAfeLDREV(x) == AFE_LD_REV_6303_VR5P3)
#define SoftDslAfeLDREV6303V5p3(x)		(SoftDslAfeLD6303(x) && (SoftDslAfeLDREV(x) == AFE_LD_REV_6303_VR5P3))

#define SoftDslAfeAnnex(x)		((gAfeInfo(x).boardAfeId & AFE_FE_ANNEX_MASK) >> AFE_FE_ANNEX_SHIFT)
#define SoftDslAfeIsAnnexA(x)	(SoftDslAfeAnnex(x) == AFE_FE_ANNEXA)
#define SoftDslAfeIsAnnexB(x)	(SoftDslAfeAnnex(x) == AFE_FE_ANNEXB)
#define SoftDslAfeIsAnnexJ(x)	(SoftDslAfeAnnex(x) == AFE_FE_ANNEXJ)
#define SoftDslAfeIsAnnexBJ(x)	(SoftDslAfeAnnex(x) == AFE_FE_ANNEXBJ)
/* #define SoftDslAfeIsAnnexM(x)	(SoftDslAfeAnnex(x) == AFE_FE_ANNEXM) */

/* TBD: review these macros */
#define SoftDslAfeIsOverIsdn(x) (((gAfeInfo(x).boardAfeId & AFE_FE_ANNEX_MASK) >> AFE_FE_ANNEX_SHIFT) == AFE_FE_ANNEXB)
#define SoftDslAfeIsOverPots(x) (((gAfeInfo(x).boardAfeId & AFE_FE_ANNEX_MASK) >> AFE_FE_ANNEX_SHIFT) == AFE_FE_ANNEXA)
#define SoftDslAfeIsUs0AnnexJ(x) (((gAfeInfo(x).boardAfeId & AFE_FE_ANNEX_MASK) >> AFE_FE_ANNEX_SHIFT) == AFE_FE_ANNEXJ)
#define SoftDslAfeIsUs0AnnexBJ(x) (((gAfeInfo(x).boardAfeId & AFE_FE_ANNEX_MASK) >> AFE_FE_ANNEX_SHIFT) == AFE_FE_ANNEXBJ)
#define SoftDslAfeIsAnnexM(x) (((gAfeInfo(x).boardAfeId & AFE_FE_ANNEX_MASK) >> AFE_FE_ANNEX_SHIFT) == AFE_FE_ANNEXM)
#define SoftDslAfeIsAnnexX(x) (((gAfeInfo(x).boardAfeId & AFE_FE_ANNEX_MASK) >> AFE_FE_ANNEX_SHIFT) == AFE_FE_ANNEXX)

#define SoftDslAfeAVMode(x)		((gAfeInfo(x).boardAfeId & AFE_FE_AVMODE_MASK) >> AFE_FE_AVMODE_SHIFT)
/* historically VWG (ISIL VDSL only boards) has AFE_FE_REV_ISIL_REV1 = 1 */
#if 0
#define SoftDslAfeIsVdslOnly(x)	(SoftDslAfeAVMode(x) == AFE_FE_AVMODE_VDSL)
#else
#define SoftDslAfeIsVdslOnly(x)	SoftDslAfeLD1556(x)
#endif
#define SoftDslAfeIsAdslOnly(x)	(SoftDslAfeAVMode(x) == AFE_FE_AVMODE_ADSL)
#define SoftDslAfeIsComb(x)		(!SoftDslAfeLD1556(x) && (SoftDslAfeAVMode(x) == AFE_FE_AVMODE_COMBO))


#define SoftDslAfeFeVersion(x)		    ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK))
#define SoftDslAfeIsComb_7_12(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_A_7_12)
#define SoftDslAfeIsComb_7_4(x)			((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_A_7_4)
#define SoftDslAfeIsComb_7_2_30(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_A_7_2_30)
#define SoftDslAfeIsCombA_7_2_30(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_A_7_2_30)
#define SoftDslAfeIsCombM_7_2_30(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_M_7_2_30)
#define SoftDslAfeIsCombBJ_7_2_30(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_BJ_7_2_30)
#define SoftDslAfeIsCombB_7_2_30(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_B_7_2_30)
#define SoftDslAfeIsCombJ_7_2_30(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_J_7_2_30)
#define SoftDslAfeIsCombA_12_40(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_A_12_40)
#define SoftDslAfeIsCombA_12_3_30(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_A_12_3_30)
#define SoftDslAfeIsCombA_12_3_20(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_A_12_3_20)
/*FIXME remove || condition once 12.3.50 was properly propagated */
#define SoftDslAfeIsCombA_12_3_40_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_3_40_0)
#define SoftDslAfeIsCombA_12_3_40_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_3_40_1)
#define SoftDslAfeIsCombA_12_3_60_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_3_60_0)
#define SoftDslAfeIsCombA_12_3_60_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_3_60_1)
#define SoftDslAfeIsCombM_12_3_60_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_3_60_0)
#define SoftDslAfeIsCombM_12_3_60_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_3_60_1)
#define SoftDslAfeIsCombBJ_12_3_60_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_BJ_12_3_60_0)
#define SoftDslAfeIsCombBJ_12_3_60_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_BJ_12_3_60_1)
#define SoftDslAfeIsCombM_12_4_60_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_4_60_0)
#define SoftDslAfeIsCombM_12_4_60_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_4_60_1)
#define SoftDslAfeIsCombA_12_4_60_1_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_60_1_0)
#define SoftDslAfeIsCombA_12_4_60_1_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_60_1_1)
#define SoftDslAfeIsCombA_12_4_60_2_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_60_2_0)
#define SoftDslAfeIsCombA_12_4_60_2_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_60_2_1)
#define SoftDslAfeIsCombX_12_4_60_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_4_60_0)
#define SoftDslAfeIsCombX_12_4_60_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_4_60_1)
#define SoftDslAfeIsCombX_12_5_60_1_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_5_60_1_0)
#define SoftDslAfeIsCombX_12_5_60_1_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_5_60_1_1)
#define SoftDslAfeIsCombX_12_5_60_2_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_5_60_2_0)
#define SoftDslAfeIsCombX_12_5_60_2_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_5_60_2_1)
#define SoftDslAfeIsCombX_12_50(x)	    ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_50)
#define SoftDslAfeIsCombX_12_51(x)	    ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_51)
#define SoftDslAfeIsCombX_12_4_40(x)	    ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_4_40)
#define SoftDslAfeIsCombX_12_4_40_GF(x)	    ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_4_40_GF)
#define SoftDslAfeIsCombX_12_4_40_V(x)	    ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_4_40_V)
#define SoftDslAfeIsCombA_12_4_40(x)	    ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_40)
#define SoftDslAfeIsCombA_12_4_45(x)	    ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_45)
#define SoftDslAfeIsCombA_12_4_45_GF(x)	    ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_45_GF)
#define SoftDslAfeIsCombA_12_4_45_V(x)	    ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_45_V)
#define SoftDslAfeIsCombA_12_3_40(x)  (((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_3_40_0) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_3_40_1))
#define SoftDslAfeIsCombA_12_3_60(x)  (((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_3_60_0) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_3_60_1))
#define SoftDslAfeIsCombM_12_3_60(x)  (((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_3_60_0) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_3_60_1))
#define SoftDslAfeIsCombBJ_12_3_60(x)  (((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_BJ_12_3_60_0) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_BJ_12_3_60_1))
#define SoftDslAfeIsCombA_12_4_60(x)  (((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_60_0) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_60_1))
#define SoftDslAfeIsCombA_12_4_60_2(x)  (((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_60_2_0) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_4_60_2_1))
#define SoftDslAfeIsCombM_12_4_60(x)  (((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_4_60_0) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_4_60_1))
#define SoftDslAfeIsCombX_12_4_60(x)  (((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_4_60_0) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_4_60_1))
#define SoftDslAfeIsCombX_12_5_60_1(x)  (((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_5_60_1_0) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_5_60_1_1))
#define SoftDslAfeIsCombX_12_5_60_2(x)  (((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_5_60_2_0) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_X_12_5_60_2_1))
#define SoftDslAfeIsCombBJ_12_3_40_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_BJ_12_3_40_0)
#define SoftDslAfeIsCombBJ_12_3_40_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_BJ_12_3_40_1)
#define SoftDslAfeIsCombA_12_3_50(x)	(((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_3_50) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_3_50_1))
#define SoftDslAfeIsCombA_12_3_70(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_A_12_3_70)
#define SoftDslAfeIsCombM_12_3_70(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_3_70)
#define SoftDslAfeIsCombBJ_12_3_50(x)	(((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_BJ_12_3_50) || ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_BJ_12_3_50_1))
#define SoftDslAfeIsCombBJ_12_3_20(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_BJ_12_3_20)
#define SoftDslAfeIsCombBJ_12_3_30(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_BJ_12_3_30)
#define SoftDslAfeIsCombA_7_2_21(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_A_7_2_21)
#define SoftDslAfeIsCombM_12_3_20(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_M_12_3_20)
#define SoftDslAfeIsCombM_12_3_30(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_M_12_3_30)

#define SoftDslAfeIsCombM_12_3_40_0(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_3_40_0)
#define SoftDslAfeIsCombM_12_3_40_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_3_40_1)
#define SoftDslAfeIsCombM_12_3_50(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_M_12_3_50)

#define SoftDslAfeIsCombC_12_3_40_0(x)  ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_C_12_3_40_0)
#define SoftDslAfeIsCombC_12_3_40_1(x)  ((gAfeInfo(x).boardAfeId & AFE_FE_FULL_CHIP_MASK) == AFE_FE_COMB_C_12_3_40_1)

#define SoftDslAfeIsComb_M_7_2_1(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_M_7_2_1)
#define SoftDslAfeIsComb_B_7_2(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_B_7_2)
#define SoftDslAfeIsComb_B_7_2_UR2(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_B_7_2_UR2)
#define SoftDslAfeIsComb_J_7_2_1(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_J_7_2_1)
#define SoftDslAfeIsComb_J_7_2_2(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_J_7_2_2)
#define SoftDslAfeIsComb_BJ_7_2_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_BJ_7_2_1)
#define SoftDslAfeIsComb_BJ_7_2_2(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_COMB_BJ_7_2_2)

#define SoftDslAfeIsAdsl_A_5_1_1(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_A_5_1_1)
#define SoftDslAfeIsAdsl_A_5_1_2(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_A_5_1_2)
#define SoftDslAfeIsAdsl_M_5_1_2(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_M_5_1_2)
#define SoftDslAfeIsAdsl_A_5_1_3(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_A_5_1_3)
#define SoftDslAfeIsAdsl_A_5_1_4(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_A_5_1_4)
#define SoftDslAfeIsAdsl_A_5_2_1(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_A_5_2_1)
#define SoftDslAfeIsAdsl_A_5_2_2(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_A_5_2_2)
#define SoftDslAfeIsAdsl_A_5_2_3(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_A_5_2_3)
#define SoftDslAfeIsAdsl_M_5_1_1(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_M_5_1_1)
#define SoftDslAfeIsAdsl_B_5_1_1(x)		((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_B_5_1_1)
#define SoftDslAfeIsAdsl_BJ_5_1_1(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_BJ_5_1_1)
#define SoftDslAfeIsAdsl_B_5_1_2(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_B_5_1_2)
#define SoftDslAfeIsAdsl_BJ_5_1_2(x)	((gAfeInfo(x).boardAfeId & AFE_FE_FULL_MASK) == AFE_FE_ADSL_BJ_5_1_2)

#define SoftDslAfeRNCboard(x)         ((gAfeInfo(x).boardAfeId & AFE_FE_RESERVE_RNC) != 0)
#define SoftDslAfe8dBmBit(x)          ((gAfeInfo(x).boardAfeId & AFE_FE_RESERVE_8dBm) != 0)
#if 0 //temporarily disable 8dBm tx PSD for 158 boards
#define SoftDslAfeIs8dBmBoard(x)      (SoftDslAfeIsCombX_12_51(x) || SoftDslAfeIsCombX_12_4_40(x) || SoftDslAfeIsCombX_12_4_40_GF(x) || SoftDslAfeIsCombA_12_4_45(x) || SoftDslAfeIsCombA_12_4_45_GF(x) || SoftDslAfe8dBmBit(x))
#else
#define SoftDslAfeIs8dBmBoard(x)      (SoftDslAfeIsCombX_12_51(x) || SoftDslAfeIsCombX_12_4_40(x) || SoftDslAfeIsCombX_12_4_40_GF(x) || SoftDslAfeIsCombA_12_4_45(x) || SoftDslAfeIsCombA_12_4_45_GF(x) || SoftDslAfeIsCombX_12_4_60(x) || SoftDslAfeIsCombA_12_4_60(x) || SoftDslAfeIsCombA_12_4_60_2(x) || SoftDslAfeIsCombX_12_5_60_1(x) || SoftDslAfeIsCombX_12_5_60_2(x) || SoftDslAfe8dBmBit(x))
#endif
#ifdef CHIP_SRC
#define SoftDslAfeIsGfast212(x)       (SoftDslAfeIsCombX_12_5_60_2(x)||SoftDslAfeIsCombA_12_4_60_2(x))
#else
#define SoftDslAfeIsGfast212(x)       1
#endif
#define SoftDslAfeIsGfastOnly(x)      (SoftDslAfeIsCombX_12_4_60(gDslVars) || SoftDslAfeLD6305(gDslVars))

#define SoftDslAfeIsAnnexC(x)          ((gAfeInfo(x).boardAfeId & AFE_FE_ANNEX_MASK) == AFE_FE_ANNEXC_BITMAP)

/* Line driver(s) control bits mapping (afeChidIdConfig0) */

#define SoftDslAfeLDIsIntPwrMapped(x)	((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_INT_PWR_MAP) != 0)
#define SoftDslAfeLDIsIntModeMapped(x)	((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_INT_MODE_MAP) != 0)
#define SoftDslAfeLDIsIntPwrPol(x)		((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_INT_PWR_POL) != 0)
#define SoftDslAfeLDIsIntModePol(x)		((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_INT_MODE_POL) != 0)

#define SoftDslAfeLDIsExtPwrMapped(x)	((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_EXT_PWR_MAP) != 0)
#define SoftDslAfeLDIsExtModeMapped(x)	((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_EXT_MODE_MAP) != 0)
#define SoftDslAfeLDIsExtPwrPol(x)		((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_EXT_PWR_POL) != 0)
#define SoftDslAfeLDIsExtModePol(x)		((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_EXT_MODE_POL) != 0)

#define SoftDslAfeLDIsInt6302LdPad(x)   ((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_INT6302_PAD) != 0)
#define SoftDslAfeLDIsExt6302LdPad(x)   ((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_EXT6302_PAD) != 0)

#define SoftDslAfeLDIsInt6302PinSwap(x) ((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_INT_PIN_SWAP) != 0)
#define SoftDslAfeLDIsExt6302PinSwap(x) ((gAfeInfo(x).afeChidIdConfig0 & AFELD_CTL_EXT_PIN_SWAP) != 0)

#define SoftDslAfeDyingGaspPhyConfigBypass(x) ((gAfeInfo(x).afeChidIdConfig1 & DG_PHY_CONFIG_BYPASS_SHIFT_MASK) != 0)
#define SoftDslAfeLastModeVdsl(x)             ((gAfeInfo(x).afeChidIdConfig1 & AFECTL1_LAST_MODE_VDSL) != 0)
#define SoftDslAfeClearLastModeVdsl(x)        ((gAfeInfo(x).afeChidIdConfig1) &= ~AFECTL1_LAST_MODE_VDSL)
#define SoftDslAfeSetLastModeVdsl(x)          ((gAfeInfo(x).afeChidIdConfig1) |= AFECTL1_LAST_MODE_VDSL)

/* */
#define SoftDslAfeLDIsInt6302(x)        ( SoftDslAfeLDIsInt6302LdPad(x))
#define SoftDslAfeLDIsExt6302(x)        ( SoftDslAfeLDIsExt6302LdPad(x))

/* Line driver(s) generic pin mapping (afeChidIdConfig0) */

#define SoftDslAfeLDGetPinId(x,pin)		((gAfeInfo(x).afeChidIdConfig0 >> (16+((pin) << 2))) & kDslAfeLdPinIdMask)
#define SoftDslAfeLDGetPinPol(x,pin)	((gAfeInfo(x).afeChidIdConfig0 >> (16+3+((pin) << 2))) & 1)

#define SoftDslAfeLDSetPinId(x,pin,val)  (gAfeInfo(x).afeChidIdConfig0 |= ((val) << (16+((pin) << 2))))
#define SoftDslAfeLDSetPinPol(x,pin,val) (gAfeInfo(x).afeChidIdConfig0 |= ((val) << (16+3+((pin) << 2))))

#define SoftDslAfeLDResetPinId(x,pin,val)  ((gAfeInfo(x).afeChidIdConfig0 & ~(kDslAfeLdPinIdMask << (16+((pin) << 2)))) | ((val) << (16+((pin) << 2))))
#define SoftDslAfeLDResetPinPol(x,pin,val) ((gAfeInfo(x).afeChidIdConfig0 & ~(1 << (16+3+((pin) << 2)))) | ((val) << (16+3+((pin) << 2))))

#define SoftDslAfeLDGetPinPwrId(x)		SoftDslAfeLDGetPinId(x,kDslAfeLdPinPwrIdx)
#define SoftDslAfeLDGetPinPwrPol(x)		SoftDslAfeLDGetPinPol(x,kDslAfeLdPinPwrIdx)
#define SoftDslAfeLDGetPinModeId(x)		SoftDslAfeLDGetPinId(x,kDslAfeLdPinModeIdx)
#define SoftDslAfeLDGetPinModePol(x)	SoftDslAfeLDGetPinPol(x,kDslAfeLdPinModeIdx)
#define SoftDslAfeLDGetPinClkId(x)		SoftDslAfeLDGetPinId(x,kDslAfeLdPinClkIdx)
#define SoftDslAfeLDGetPinClkPol(x)		SoftDslAfeLDGetPinPol(x,kDslAfeLdPinClkIdx)
#define SoftDslAfeLDGetPinDataId(x)		SoftDslAfeLDGetPinId(x,kDslAfeLdPinDataIdx)
#define SoftDslAfeLDGetPinDataPol(x)	SoftDslAfeLDGetPinPol(x,kDslAfeLdPinDataIdx)

#define SoftDslAfeLDSetPinPwrId(x,pin)		SoftDslAfeLDSetPinId(x,kDslAfeLdPinPwrIdx,pin)
#define SoftDslAfeLDSetPinPwrPol(x,pin)		SoftDslAfeLDSetPinPol(x,kDslAfeLdPinPwrIdx,pin)
#define SoftDslAfeLDSetPinModeId(x,pin)		SoftDslAfeLDSetPinId(x,kDslAfeLdPinModeIdx,pin)
#define SoftDslAfeLDSetPinModePol(x,pin)	SoftDslAfeLDSetPinPol(x,kDslAfeLdPinModeIdx,pin)
#define SoftDslAfeLDSetPinClkId(x,pin)		SoftDslAfeLDSetPinId(x,kDslAfeLdPinClkIdx,pin)
#define SoftDslAfeLDSetPinClkPol(x,pin)		SoftDslAfeLDSetPinPol(x,kDslAfeLdPinClkIdx,pin)
#define SoftDslAfeLDSetPinDataId(x,pin)		SoftDslAfeLDSetPinId(x,kDslAfeLdPinDataIdx,pin)
#define SoftDslAfeLDSetPinDataPol(x,pin)	SoftDslAfeLDSetPinPol(x,kDslAfeLdPinDataIdx,pin)

/*
**		Test descriptor
*/

#define TCFG_MODE_MASK			1
#define TCFG_MODE_NORMAL		0
#define TCFG_MODE_TEST			1

#define TCFG_MODE_RX_MASK		2
#define TCFG_MODE_RX_FBAND		0
#define TCFG_MODE_RX_BBAND		2

#define TCFG_MODE_TX_MASK		4
#define TCFG_MODE_TX_FBAND		0
#define TCFG_MODE_TX_BBAND		4

#define TCFG_MODE_FILT_MASK             8
#define TCFG_MODE_FILT_NORMAL           0
#define TCFG_MODE_NO_FILTERS            8

#define TCFG_MODE_RNC_MASK            16
#define TCFG_MODE_DATA_CHAN           0
#define TCFG_MODE_RNC_CHAN            16

#define TCFG_TX_QUIET			0
#define TCFG_TX_REVERB			1
#define TCFG_TX_MEDLEY			2
#define TCFG_TX_HIGHCREST		3

#define TCFG_RX_TYPE_MASK		0xF
#define TCFG_RX_TYPE_IDLE		0
#define TCFG_RX_TYPE_AVG		1

#define TCFG_RX_FMT_SHIFT		4
#define TCFG_RX_FMT_MASK		0x30
#define TCFG_RX_FMT_SQ			(0<<TCFG_RX_FMT_SHIFT)
#define TCFG_RX_FMT_XY			(1<<TCFG_RX_FMT_SHIFT)

#ifndef SELT_TOOLBOX
typedef struct Bpoint Bpoint;
struct Bpoint
{
  unsigned short tone;
  short value;
};

typedef struct Psd993p2 Psd993p2;
struct Psd993p2
{
  unsigned char      maxNbBP;
  unsigned char      n;
  Bpoint bp[32];
};

struct testConfigStruct {
  ushort                mode;         /* bit 0 -> 0 normal mode, 1 test mode
                                       *     1 -> 0 Rx fixed band Rx, 1 broad band
                                       *     2 -> 0 Tx fixed band Tx, 1 broad band
                                       */
  ushort                txType;       /* 0 = QUIET, 1 = REVERB, 2 = MEDLEY, 3 = HIGH CREST...   */
  uint              rxType;       /* 0..3 meas 0 - none, 1 - avg, 2 - PLN, ...              */
                                      /* 4..5 0 = nonone, 1 = X^2+Y^2 ,2 = X+iY...              */
  uint              duration;     /* INFINIUM or  other non zero is duration in ms          */
  uint              avgPeriod;    /* average period in ms.                                  */
  uint              logAvgPeriod; /* floor(log2(avgPeriod)) - will be populated by adsl-mech*/
  uint              reportPeriod; /* report period in ms; 0 - default == avgPeriod          */
  ushort                numMonTones;  /* number of Rx monitor tones per symbol in a broadband sweep     */
  ushort                txGainDB;     /* additional gain relative to specified PSD mask in fix8_8 format*/
  uint              hwControl;    /* TX/RX frontend controls     */
  Psd993p2              psd;
  uchar                 xmtBufIdx;    /* 0 = bitEnc output, 1 = replay buffer */
  uchar                 measInitReady;
  int                   crestControl;
  short                 forceAgc;
  int                   forcePPMperiod;
  int                   forcePPMkick;
  int                   forcePpmOffsetValue;
};
typedef struct testConfigStruct testConfigStruct;

#define SoftDslGetTestConfig(gDslVars) (&(gDslGlobalSlowVarPtr->testConfig))

#ifdef gTestCfgVars
#define gTestCfg(x)   gTestCfgVars
#else
#define gTestCfg(x)   (*(SoftDslGetTestConfig(x)))
#endif

#define SoftDslTestCfgMode(x)			(gTestCfg(x).mode)
#define SoftDslTestCfgIsTestMode(x)		(SoftDslTestCfgMode(x) != 0)
#define SoftDslTestCfgIsRxBB(x)			((SoftDslTestCfgMode(x) & TCFG_MODE_RX_MASK) == TCFG_MODE_RX_BBAND)
#define SoftDslTestCfgIsTxBB(x)			((SoftDslTestCfgMode(x) & TCFG_MODE_TX_MASK) == TCFG_MODE_TX_BBAND)
#define SoftDslTestCfgFiltMode(x)		(SoftDslTestCfgMode(x) & TCFG_MODE_FILT_MASK)
#define SoftDslTestCfgRNCMode(x)		(SoftDslTestCfgMode(x) & TCFG_MODE_RNC_MASK)

#define SoftDslTestCfgTx(x)				(gTestCfg(x).txType)
#define SoftDslTestCfgTxSignal(x)		(gTestCfg(x).txType)

#define SoftDslTestCfgRx(x)				(gTestCfg(x).rxType)
#define SoftDslTestCfgRxType(x)			(SoftDslTestCfgRx(x) & TCFG_RX_TYPE_MASK)
#define SoftDslTestCfgRxIdle(x)			(SoftDslTestCfgRxType(x) == TCFG_RX_TYPE_IDLE)
#define SoftDslTestCfgRxAvg(x)			(SoftDslTestCfgRxType(x) == TCFG_RX_TYPE_AVG)

#define SoftDslTestCfgRxFmt(x)			(SoftDslTestCfgRx(x) & TCFG_RX_FMT_MASK)
#define SoftDslTestCfgRxFmtSq(x)		(SoftDslTestCfgRxFmt(x) == TCFG_RX_FMT_SQ)
#define SoftDslTestCfgRxFmtXY(x)		(SoftDslTestCfgRxFmt(x) == TCFG_RX_FMT_XY)

/*
**		NTR report structure
*/

struct ntrCntStruct {
    uint   ncoOutCntAtDmt;
    uint   ncoOutCntAtNtr;
    uint   lcoCntAtDmt;
    uint   lcoCntAtNtr;
    uint   ncoRefCntAtDmt;
    uint   ncoRefCntAtNtr;
    /* 6362/63268 */
#if defined(CONFIG_BCM96362) || defined(CONFIG_BCM96328) || defined(CONFIG_BCM963268) || defined(CONFIG_BCM96318) ||\
    defined(CONFIG_BCM963138) || defined(CONFIG_BCM963381) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM963158) || \
    defined(CONFIG_BCM963178) || defined(CONFIG_BCM963146)
    int    phaseError;
    int    VCOAdjInfo;
#endif
};
typedef struct ntrCntStruct ntrCntStruct;

/* Define the valid NTR operating modes */
/* Primary line */
#define NTR_OPER_MODE_6368                (0)
#define NTR_OPER_MODE_INT                 (1)
#define NTR_OPER_MODE_EXT_DRIVER          (2)
/* 63268 */
#define NTR_OPER_MODE_BONDING_6368        (3)
#define NTR_OPER_MODE_BONDING_INT         (4)
#define NTR_OPER_MODE_BONDING_EXT_DRIVER  (5)

struct ntrCfgStruct {
    uint   intModeDivRatio;     /* NTR output freq = 17.664e6/intModeDivRatio */
    uint   extModePhaseScale;   /* 6362 */
                                 /* ---- */
                                 /*   scale factor used to convert external clock ticks to 17.664 MHz clock ticks */
                                 /*   extModePhaseScale = 17.664e6/(external clock freq), 16.16 format            */
                                 /*           */
                                 /* 63268     */
                                 /* -----     */
                                 /*   Primary */
                                 /*   ------- */
                                 /*   scale factor used to convert external clock ticks to 70.656 MHz clock ticks */
                                 /*   extModePhaseScale = 70.656e6/(external clock freq), 16.16 format            */
                                 /*           */
                                 /*   Bonding */
                                 /*   ------- */
                                 /*   scale factor used to convert external clock ticks to 8.832 MHz clock ticks  */
                                 /*   extModehaseScale = 8.832e6/(external clock freq), 16.16 format              */
    uint   updatePeriod;        /* PLL update period in milliseconds  */
    int    b0;                  /* loop filter, b0       */
    int    b1;                  /* loop filter, b1       */
    uchar   operMode;
    uchar   b0_scale;            /* loop filter, b0 scale */
    uchar   b1_scale;            /* loop filter, b1 scale */
    /* The parameters below are for potential future expansion
     * if it is decided to compute the PLL loop filter coefficients
     * on the PHY MIPS instead of the HOST MIPS as is currently
     * being done.
     */
#if 0
    double zeta;              /* damping factor                 */
    double w0;
    double Kvco;              /* VCO gain                       */
    double Kphase;            /* phase detector gain            */
#endif
};
typedef struct ntrCfgStruct ntrCfgStruct;

/*
**		Bonding discovery structure
*/

#if defined(BONDING_G994_DISCOVERY) || defined(DSL_MULTIPHY_BONDING) || defined(DSL_EXTERNAL_BONDING_DISCOVERY)
typedef struct {
    unsigned short pmeRemoteDiscoveryHigh;
    uint  pmeRemoteDiscoveryLow;
    uint  pmeAggregationReg;
    int   timeOfLastCL;
} SoftHsBondingDiscoveryStruct;
#endif

#if defined(DSL_EXTERNAL_BONDING_DISCOVERY)

#define  kBondDiscCmdReqReply		0x8000
#define  kBondDiscCmdGet			0x0000
#define  kBondDiscCmdSet			0x0001
#define  kBondDiscCmdClearIfSame    0x0002
#define  kBondDiscCmdDiscovery      0x0004
#define  kBondDiscCmdAggregate      0x0008

typedef struct {
   short	bdCmd;
   short	bdId;
   SoftHsBondingDiscoveryStruct   bondDisc;
} bonDiscExchangeStruct;

#endif

/* Data dump buffer definitions */

typedef struct {
   void     *pBuf;
   int  bufSize;
   short    frameHdrSize;	/* for skb/mac/ip/udp headers */
   short    maxFrameSize;
   void		*pHostAddr;
} dumpBufferStruct;

#define	kDumpCtrlAbsMemAddrPos	0
#define	kDumpCtrlAbsMemAddr		1

#define	kDumpCtrlPos			29
#define	kDumpBufOutputPos		29
#define	kDumpBufModePos			30
#define	kDumpLengthMask			((1 << kDumpCtrlPos) -1)

#define	kDumpBufOutputMask		(1 << kDumpBufOutputPos)
#define	kDumpBufOutputBuf		0
#define	kDumpBufOutputSkb		1
#define	kDumpBufOutputBufBit	(kDumpBufOutputBuf << kDumpBufOutputPos)
#define	kDumpBufOutputSkbBit	(kDumpBufOutputBuf << kDumpBufOutputPos)

#define	kDumpBufModeMask		(3 << kDumpBufModePos)
#define	kDumpBufModeContWr		0
#define	kDumpBufModeContBuf		1
#define	kDumpBufModeTillFull	2
#define	kDumpBufModeContWrBit	(kDumpBufModeContWr   << kDumpBufModePos)
#define	kDumpBufModeContBufBit	(kDumpBufModeContBuf  << kDumpBufModePos
#define	kDumpBufModeTillFullBit	(kDumpBufModeTillFull << kDumpBufModePos)

#define	kDumpSaveCmd			-1

#if !defined(__KERNEL__) && !defined(WINNT) && !defined(__ECOS) && !defined(_NOOS)
extern void	*SoftDslGetConnectionSetupPtr(void *gDslVars) SHOWTIME_TEXT;
#endif
extern void *SoftDslGetSelectedG993p2Caps(void *gDslVars);
extern void *SoftDslGetLocalCaps(void *gDslVars);
#if defined(FIRE_RETRANSMISSION) || defined(FIRE_XMT_6368)
void *SoftDslGetPhyRSpecs(void *gDslVars);
#endif

/*
**
**			G.993 status flags and structures.
**
**	  Status codes should be defined in SoftDsl.h under
**	  "General status messages (using clearEOC message structure)"
**	  starting from   kDslGeneralMsgStart
**
*/
#if !defined(__KERNEL__) && !defined(__ECOS) && !defined(_NOOS)
typedef struct {
	uint	ctrl;
	uint	dummy;
	ULONGLONG	arg1;
	ULONGLONG	arg2;
	uint	lineId;
	uint	txPh;
	uint	txSym;
	uint	rxPh;
	uint	rxSym;
} e14DbPrintStatus;
#endif

typedef struct ToneGroup ToneGroup;
struct ToneGroup
{
  unsigned short endTone;
  unsigned short startTone;
};

typedef struct Bp993p2 Bp993p2;
struct Bp993p2
{
  unsigned char  noOfToneGroups;
  unsigned char  reserved;
  ToneGroup toneGroups[16];
};

#ifndef ADSL_ONLY
#define MAX_US_TONES1 2816
#else
#define MAX_US_TONES1 64
#endif
typedef struct {
  uint    numTones;
  int     isTrellis;
  int     ftg_wordlength;
  uint    digitalGain;
  short   analogGain;
  short   extraIFFTShift;
  ushort  *txTssiPtr;

  int     DS3boostActive;
  int     init_power_boost;
  uint    mapValXY;
  short   currentPsdVdsl;

  uint    sraTxLmin;        /* Store for values computed after rate select */
  uint    sraTxLmax;
	uchar	  raModeUs;

  uchar   bI[MAX_US_TONES1];
  uchar   bICopy[MAX_US_TONES1];
  ushort  gI[MAX_US_TONES1];
#if defined(ACQ_IDEAL_SYNC_SYMBOL) && defined(INFN_SYNC_SYMBOL_WORK_AROUND) && defined(VDSL_MODEM)
  short  syncGain[MAX_US_TONES1];
#else
  ushort  syncGain[MAX_US_TONES1];
#endif
  ushort  tI[MAX_US_TONES1];
  ushort  txTssi[MAX_US_TONES1];
  ushort  L[2];             /* Number of bits per symbol for each latency */
  ushort  L_init;           /* Number of bits per symbol for each latency */
  ushort  L_new;            /* Number of bits per symbol for each latency */
#ifdef DYNAMIC_D
  ushort  D_new;
  ushort  D_old;
  ushort  L_old;
#endif
#ifdef DYNAMIC_FRAMING
  uchar Tnew;
  uchar Gnew;
  uchar Bnew;
  ushort Unew;
#endif
} TxInit;

typedef struct {
  uint    numTones;
  ushort  nUsed;            /* number of tones with bi >= 0 */
  int     isTrellis;
  int     snrMode;
  ushort  L[2];             /* Number of bits per symbol for each latency */
  ushort  L_init;           /* Number of bits per symbol for each latency */
  ushort  L_new;            /* Number of bits per symbol for each latency */
  ushort  D[2];             /* interleaving depth: =1 for fast path */
  ushort  B[2];             /* nominal total of each bearer's octets per Mux Data Frame (Slightly redundant)*/
  uchar   T[2];             /* ADSL2: Number of Mux frames per sync octet*/
                            /* VDSL2: Number of Mux data frames in an OH sub-frame */
  uchar   G[2];             /* VDSL2: Notional number of OH bytes in an OH sub-frame - actual number of bytes in any sub-frame may 1 be greater than this */
#ifdef DYNAMIC_D
  ushort  N[2];             /* codeword length */
  ushort  L_old;
  int     D_sweep;
  ushort  D_old;
#endif
  ushort  D_new;            /* !!! D_new has to be defined unconditionally !!! */
  uchar   pcbRx;
#ifdef DYNAMIC_FRAMING
  uchar  Gnew;
  uchar  Tnew;
  ushort Unew;
  ushort Bnew;
  ushort U[2];
#endif
#ifdef GINP_SRA
  uchar newM;
  uchar newB;
  uchar newR;
  uchar newQ;
  uchar newV;
  uchar newQtx, newQrx, newQrxBuffer;
  uchar newlb;
  ushort newN;
  uint newETR_kbps;
#endif

  uint    sraRxLmin;        /* Store for values computed after rate select */
  uint    sraRxLmax;
	uchar	  raModeDs;
	short		sraUpShiftMarginDs;
	short		sraUpShiftMinTimeDs;
	short		sraDownShiftMarginDs;
	short		sraDownShiftMinTimeDs;

  uchar   maxConstellation; /* max allowed constellation size negotiated */
  uchar   minConstellation; /* min allowed constellation size negotiated */
  short   pilot;            /* pilot tone index (-1 if no pilot is used) */
  short   targetMargin;     /* cached copy of the physical configuration */
  short   targetMaxMargin;  /* cached copy of the physical configuration */
  short   meanGain;         /* cached copy of the physical configuration */
  uint    maxTxPower;       /* maximum transmit power with shaped PSD, equal to
                                 10^((MAXNOMATP-psdRef-10*log10(TONE_SPACING))/10)/groupSize */
  uint    maxPower;         /* maxium transmit power assuming a flat PSD, equal to nUsed, but
                                 expressed in a different format */
  short   refPsd;           /* Reference PSD of the farend tssi */
  uint    maxNomATP;        /* max power computed from MAXNOMATPds */
  short   psdMin;           /* minimum allowed value for the meanGain */
  short   psdMax;           /* maximum allowed value for the meanGain */
  char    *bI;
  short   *gI;
  short   *tI;
  short   *tI_inv;
  ushort  *feTssi;
  short   *maxSnrPtr;
  int     trainingSnrMode;
#ifdef RIPOLICY_SUPPORT
  uchar   reinitTimeThreshold;
#endif
} RxBiGi;

#endif /* SELT_TOOLBOX */

#endif	/* SoftDslG993p2Header */
