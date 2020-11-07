/****************************************************************************
 *
 * SoftDsl.h
 *
 *
 * Description:
 *	This file contains the exported interface for SoftDsl.c
 *
 *
 * Copyright (c) 1993-1998 AltoCom, Inc. All rights reserved.
 * Authors: Ilya Stomakhin
 *
 * $Revision: 1.738 $
 *
 * $Id: SoftDsl.h,v 1.738 2013/01/31 17:22:46 rgreenf Exp $
 *
 * $Log: SoftDsl.h,v $
 * Revision 1.738  2013/01/31 17:22:46  rgreenf
 * PR6698: initial checkin for new attainable data rate computation
 *
 * Revision 1.737  2013/01/30 12:32:50  ovandewi
 * PR7121: add statuses for PTM options
 *
 * Revision 1.736  2013/01/17 22:23:04  jknittel
 * FWDSLCPEPHY-7138: Add kPhyCfg1G992DTFeatureBit to enable Annex B HIGH_NOISE fix
 *
 * Revision 1.735  2013/01/11 19:44:00  yongbing
 * F-7122 Reduce C-Tone detection delay to 0 (was 100ms) after xmt R-Tone-Req for Anymedia DSLAM at Telecom Poland
 *
 * Revision 1.734  2013/01/10 22:29:00  linyin
 * PR3856 Add frequency jump test mode for python script
 *
 * Revision 1.733  2013/01/04 22:26:18  rgreenf
 * PR7111: add pll ppm offset test mode
 *
 * Revision 1.732  2012/12/22 00:03:35  sgote
 * F-7109: Adding Meta as a CO vendor and related code to support vectoring aginst them, but disabled by default
 *
 * Revision 1.731  2012/12/19 00:31:14  yongbing
 * F-7100 Improve US rates by using tx filter 15 against Calix C7 with CNXT CO in short loops with vendor ID and firmware check
 *
 * Revision 1.730  2012/12/08 01:02:14  yongbing
 * F-7086 Implement high precision G.DMT tx bit swap (using 32 bits) under driver bit
 *
 * Revision 1.729  2012/12/05 22:54:42  wiese
 * Fix memory overflow bug in BUMP_INP code.
 *
 * Revision 1.728  2012/11/30 12:28:02  ovandewi
 * PR6946: add test mode to OLR to induce mon tones conveniently
 *
 * Revision 1.727  2012/11/22 07:07:05  ovandewi
 * PR7069: alpha implementation
 *
 * Revision 1.726  2012/11/20 19:08:41  yongbing
 * F-7061 Add cfg1 driver bits to control default V43 tone levels
 *
 * Revision 1.725  2012/11/16 21:40:07  yongbing
 * F-7061 Add command to change V43 tone level
 *
 * Revision 1.724  2012/11/09 23:30:56  jinlu
 * FW6732 add driver bit control
 *
 * Revision 1.723  2012/11/06 01:57:01  tonytran
 * Support async write TEQ sample, debug data and debug writefile. Fixed readfile problem with file larger than 2GB and async write out of order
 *
 * Revision 1.722  2012/10/31 23:45:05  yongbing
 * F-7027 When SNR is below 6dB, interpolate HLog in HAM band when the driver bit is set
 *
 * Revision 1.721  2012/10/22 13:07:45  ovandewi
 * PR7024: reset toggling count when leaving showtime: support functions
 *
 * Revision 1.720  2012/10/19 22:15:46  yongbing
 * F-7022 Force downstream FFT size of 512 in ADSL2 and G.DMT modes against CNXT DSLAMs when driver bit kPhyCfg1EnableDSFFT512Interop is enabled
 *
 * Revision 1.719  2012/10/18 01:27:48  rgreenf
 * PR7008: port impulse detect/feq freeze code from 37 branch
 *
 * Revision 1.718  2012/10/05 13:03:50  ovandewi
 * PR6882: define status to inform driver
 *
 * Revision 1.717  2012/10/01 18:20:28  yongbing
 * F-6984 Add 2 to the window size in max power search for TEQ alignment for a special GSPN DSLAM at Jazztel
 *
 * Revision 1.716  2012/09/28 18:13:31  ovandewi
 * PR6988: add internal interop control
 *
 * Revision 1.715  2012/09/28 03:41:42  rgreenf
 * PR6793: bring up pilot select mex functions - correct PR
 *
 * Revision 1.714  2012/09/28 03:30:30  rgreenf
 * PR6793: bring up pilot select mex functions
 *
 * Revision 1.713  2012/09/26 17:59:50  ovandewi
 * PR6983: rename bit
 *
 * Revision 1.712  2012/09/24 22:50:25  ovandewi
 * PR6983: add interop bit
 *
 * Revision 1.711  2012/09/03 09:08:15  ovandewi
 * PR6881: support vectoring friendly
 *
 * Revision 1.710  2012/08/31 23:41:06  ilyas
 * Create common AHIF interrupt dispatcher to support Ahif(Nitro), M2M and ATTN(Gdb) interrupt sources
 *
 * Revision 1.709  2012/08/29 03:36:49  yongbing
 * F-6945 Use WIRE Vendor ID to improve performance, ported from 37 branch
 *
 * Revision 1.708  2012/08/22 18:20:45  ilyas
 * Made data dumps work with buffers allocated by DSL driver
 *
 * Revision 1.707  2012/08/17 22:59:40  rgreenf
 * PR6848: add co fw protection for adsl ginp sra
 *
 * Revision 1.706  2012/08/15 00:34:22  jknittel
 * FWDSLCPEPHY-6904: EVLTF IOP, Force OLR Control LSB
 *
 * Revision 1.705  2012/08/14 19:06:58  mhegde
 * F-6904: Define driver bit control for HDLC work around
 *
 * Revision 1.704  2012/08/06 18:28:07  jknittel
 * FWDSLCPEPHY-6904: EVLTF IOP, Reduce DS Bitswap Max Tones, Zero OLR Control LSB
 *
 * Revision 1.703  2012/07/26 00:44:51  jknittel
 * FWDSLCPEPHY-6880: Disable Swapping to Zero-bit Tones for CNXT
 *
 * Revision 1.702  2012/07/19 23:26:51  yinboli
 * FW6440: Force U0 only in long loops for CO4
 *
 * Revision 1.701  2012/07/16 20:59:31  yinboli
 * FW6822: Report Bonding Status Off when CO is ADSL1 & ATM only
 *
 * Revision 1.700  2012/07/11 01:50:49  mhegde
 * F-6821 : remove unused driver bits
 *
 * Revision 1.699  2012/07/06 16:53:54  ilyas
 * Added new status to sprintf string in Diags
 *
 * Revision 1.698  2012/07/06 13:49:17  ovandewi
 * PR6815: bit PHY->driver, correct typos
 *
 * Revision 1.697  2012/07/03 21:03:51  ghobrial
 * sw593: Add support for messages from WLAN and other drivers
 *
 * Revision 1.696  2012/06/23 01:17:07  lockem
 * Add kDslSetUsDataDelay
 *
 * Revision 1.695  2012/06/23 00:23:48  ghobrial
 * Remove client specific definition from DSL header
 *
 * Revision 1.694  2012/06/21 18:28:47  ghobrial
 * PR-SWDSLCPEPHY-417 Define and implement API for other drivers to use DslDiags
 *
 * Revision 1.693  2012/06/19 08:59:46  ovandewi
 * PR6713: switch to zero xmt upon dying gasp
 *
 * Revision 1.692  2012/06/13 18:17:13  yongbing
 * F-6711 Reverse the logic in the name of the driver bit
 *
 * Revision 1.691  2012/06/13 00:00:33  yongbing
 * F-6711 Change driver bit to be more meaningful
 *
 * Revision 1.690  2012/06/12 18:19:53  yongbing
 * F-6711 Limit the maximum L2 Gi reduction to -10dB
 *
 * Revision 1.689  2012/06/02 03:54:07  yongbing
 * F-6711 Limit L2 Gi reduction to be up to the power level of maxL2PCB on a driver bit kPhyCfg1EnableLimitL2GiReduction
 *
 * Revision 1.688  2012/05/02 21:43:08  jknittel
 * FWDSLCPEPHY-6616: Improve ADSL2p performance in Presence of RFI
 *
 * Revision 1.687  2012/04/06 23:27:57  rgreenf
 * PR6575/6577 add VDSL agc force function and fix ALB afe compensation bug
 *
 * Revision 1.686  2012/03/30 14:39:34  ovandewi
 * PR6455: add syncAligned field
 *
 * Revision 1.685  2012/03/22 21:45:17  mding
 * add dslcmd to shutdown 6306 under ENABLE_6306_SHUTDOWN flag
 *
 * Revision 1.684  2012/03/22 03:03:46  ilyas
 * Properly configure internal MIPS clock for 6318
 *
 * Revision 1.683  2012/03/20 04:30:38  jinlu
 * FW6528 add driver bit definition
 *
 * Revision 1.682  2012/03/18 02:33:52  rgreenf
 * PR6511: update driver bits
 *
 * Revision 1.681  2012/03/16 18:25:29  rwdu
 * ginp_sra support for vdsl (still work in progress).
 *
 * Revision 1.680  2012/03/15 23:05:11  jinlu
 * FW6511 partial checkin for bitswap control portion
 *
 * Revision 1.679  2012/03/15 08:53:46  ilyas
 * Use separate command to comunicate InpEqFormat for compatibility with old DSL driver
 *
 * Revision 1.678  2012/03/14 00:04:33  yinboli
 * FW6506: Do not report bonding status if ADSL & ATM
 *
 * Revision 1.677  2012/03/07 23:27:16  rgreenf
 * PR6498: add amd logarithmic time axis for INM
 *
 * Revision 1.676  2012/03/07 02:26:43  ilyas
 * Add printing bit reversed data to SoftDslPrintData()
 *
 * Revision 1.675  2012/02/17 01:55:37  lockem
 * Tidy up code/ILV memory overlap support.
 *
 * Revision 1.674  2012/02/17 01:14:50  rgreenf
 * PR6457 add ROC SNR status report
 *
 * Revision 1.673  2012/02/08 22:41:43  lockem
 * Add definitions needed to allow 6358 to provide tmType values.
 *
 * Revision 1.672  2012/02/02 22:57:04  cnpeng
 * Check in test command to enable RX compensation in Hybrid test
 *
 * Revision 1.671  2012/02/01 04:06:45  lockem
 * Add support for swap section 4 (RTX code over ILV memory).
 * Add function prototypes for 6358 Ginp support functions.
 *
 * Revision 1.670  2012/01/21 02:30:51  jinlu
 * FW6401 add status parser
 *
 * Revision 1.669  2012/01/10 01:33:51  yinboli
 * FW6328: Follow-up to Olivier's API change
 *
 * Revision 1.668  2012/01/09 10:57:22  ovandewi
 * PR6328: use interop bit from footprint and use api to control from e14
 *
 * Revision 1.667  2011/12/23 02:21:33  yinboli
 * FW6328: Remove rate cap and improve long loop performance by adding a retrain machenism
 *
 * Revision 1.666  2011/12/21 09:28:18  ovandewi
 * PR6375: Interop bit
 *
 * Revision 1.665  2011/12/19 23:24:05  yinboli
 * FW6328: Change driver bit and move CO4 check to interop struct
 *
 * Revision 1.664  2011/12/17 03:23:21  yinboli
 * FW6328: Enable CO4 interop on ToT
 *
 * Revision 1.663  2011/11/28 22:46:37  tonytran
 * Added a test command, kDslSetLD6303ConfigData for Diags to send test config data(6303) and a status code, kDslDrvConfigExtLD6303 for PHY to request the driver to configure the 6303.
 *
 * Revision 1.662  2011/11/22 18:42:25  sgote
 * F-6323 EVLT-F interop .. Removed the driver bit and added a interop bit where we do not send the A-7 byte in the first CLR.
 *
 * Revision 1.661  2011/11/08 00:48:24  rgreenf
 * PR6155: implement amd7 kl0 measurement
 *
 * Revision 1.660  2011/11/04 07:32:11  ovandewi
 * PR6295: restore ifdef
 *
 * Revision 1.659  2011/11/04 03:57:48  ilyas
 * Fixed DslDiags build
 *
 * Revision 1.658  2011/11/01 11:44:59  ovandewi
 * PR6295: make parse life easier by ifdefing AnnexC stuff
 *
 * Revision 1.657  2011/10/17 22:51:16  yongbing
 * F-6103 implement REIN work-around test for TP, note that kPhyCfg1DisableTrainingImpGating is changed from 0x40 to 0x80
 *
 * Revision 1.656  2011/10/13 00:52:47  mhegde
 * F-5713 : Improve AnnexJ EU-60 US rates on BJ HW
 *
 * Revision 1.655  2011/09/29 23:38:02  yinboli
 * FW6112: Add support for Two-Chip bonding
 *
 * Revision 1.654  2011/09/17 21:29:40  lockem
 * Cleanup & fix 6348 build.
 *
 * Revision 1.653  2011/09/17 02:15:32  lockem
 * Add support macros for checking if BG has completed in time.
 *
 * Revision 1.652  2011/09/09 18:49:56  tonytran
 * Fixed Diags Linux build problem
 *
 * Revision 1.651  2011/09/06 11:43:04  ovandewi
 * PR5754: defs for proprietary interleaver
 *
 * Revision 1.650  2011/08/31 19:39:48  sgote
 * FWDSLCPEPHY-5553: kDslRetrainReason not reported in VDSL training, removed conficted error code
 *
 * Revision 1.649  2011/08/30 18:09:56  wiese
 * Add new config bit to disable training impulse gating.  Enable by default for ADSL.  Still disabled for VDSL.
 *
 * Revision 1.648  2011/08/29 18:11:55  sgote
 * FWDSLCPEPHY-5553: kDslRetrainReason not reported in VDSL training Added some of the VDSL retrain codes in the SoftDSl.h and mapped those to lineMgr exception codes
 *
 * Revision 1.647  2011/08/26 00:15:13  yongbing
 * F-5962 Enable monitoring tones for L2 mode. Feature is controlled by adding driver bit kPhyCfg1EnableMoniteringToneForL2mode to phyExtraCfg[0]
 *
 * Revision 1.646  2011/08/21 21:09:43  ilyas
 * Added PHy/Drv exchange structure and commands for external bonding discovery
 *
 * Revision 1.645  2011/08/19 16:30:53  rgreenf
 * PR6052: add status reports for USOMASK and us/ds ref PSD masks
 *
 * Revision 1.644  2011/08/12 23:14:52  jknittel
 * FWDSLCPEPHY-5981: Bit/Gain in MIB are not correct for ADSL & VDSL
 *
 * Revision 1.643  2011/08/05 10:31:56  ovandewi
 * PR5936: allow AB toggling in annexM/POTS
 *
 * Revision 1.642  2011/08/04 14:47:44  ovandewi
 * PR5936: book interop bit
 *
 * Revision 1.641  2011/08/03 22:38:03  mhegde
 * F-6007 support new vendor id for CNXT in G.992.135
 *
 * Revision 1.640  2011/07/22 23:02:33  yongbing
 * F5976 Use tx filter Id 7 controlled by a driver bit for downstream rate improvement in Comstrend case
 *
 * Revision 1.639  2011/07/22 00:38:53  yinboli
 * FW5909: Add loop attenuation average for inner/outer pair detection
 *
 * Revision 1.638  2011/07/15 21:39:40  yinboli
 * FW5909: Add Multi-PHY support
 *
 * Revision 1.637  2011/07/09 00:58:23  wiese
 * F-5081 RNC cycle reduction, control, and other tuning.
 *
 * Revision 1.636  2011/07/08 19:41:41  rgreenf
 * PR5917: add testmode to force hp filter on
 *
 * Revision 1.635  2011/07/05 19:06:55  rgreenf
 * PR5917: add test mode to disable afe comp and Tx flat psd
 *
 * Revision 1.634  2011/06/22 14:33:43  ovandewi
 * PR5609: allow reprogramming of the pilot sequence
 *
 * Revision 1.633  2011/06/21 22:49:45  wiese
 * Updated training impulse gating algo.
 *
 * Revision 1.632  2011/06/07 00:21:35  linyin
 * PR5828: Fix INP report issue to driversrc/Adsl/G992/Fire/G992FireRateSelect.c
 *
 * Revision 1.631  2011/06/06 22:19:05  linyin
 * PR5828: Protect INP report to driver
 *
 * Revision 1.630  2011/06/01 02:29:12  lockem
 * Change type of max[Up/Down]PTM_TPS_TC to unsigned short (was short) in order
 * to increase range to allow better max rate control for VDSL.
 *
 * Revision 1.629  2011/05/25 00:22:13  ilyas
 * Added support for G.inp/E14 target
 *
 * Revision 1.628  2011/05/14 00:17:36  yongbing
 * F-5729 Add driver bit for GVT upstream rate improvement of 6328
 *
 * Revision 1.627  2011/04/23 01:51:03  lockem
 * Add adjAvailMem to PhyR configuration.
 *
 * Revision 1.626  2011/04/22 23:47:26  rwdu
 * added driver bit control of impulse gating in training.
 *
 * Revision 1.625  2011/04/19 21:04:18  pauljr
 * PR5636 Modify reporting structure to driver to support INPs greater than 255 half symbols
 *
 * Revision 1.624  2011/04/14 16:50:47  ilyas
 * Put DDR memory controller in self refresh (SR) mode when Host MIPS is not busy(using it)
 *
 * Revision 1.623  2011/04/05 23:31:24  yongbing
 * F-5608 Driver bit controlled (0x40000000 in auxFeatures) short CLR transmission
 *
 * Revision 1.622  2011/04/05 21:50:09  tonytran
 * Added status/function for PHY to retrieve LD gpio pin# and polarity
 *
 * Revision 1.621  2011/03/28 20:09:57  yongbing
 * F-5580 Do not use the driver bit for Ikanos DSLAM fix
 *
 * Revision 1.620  2011/03/25 20:43:09  yongbing
 * F-5580 For ADSL2+ Annex B J IKON DSLAM, set reverbStart to 10. Need to set driver bit of kDslPhyEnableAnnexJforIKONinAnnexB 0x40000000 of auxFeatures, and upstreamMinCarr to 28
 *
 * Revision 1.619  2011/03/17 22:23:38  ilyas
 * Ported DiagsParser to Linux
 *
 * Revision 1.618  2011/03/15 19:17:53  rwdu
 * initial check-in of standard compliant dynamic framing.
 *
 * Revision 1.617  2011/03/12 00:03:45  jinlu
 * FW3697 more clean up
 *
 * Revision 1.616  2011/03/11 17:45:06  ytan
 * eliminate warnings
 *
 * Revision 1.615  2011/03/11 02:19:03  jinlu
 * FW3697 clean up
 *
 * Revision 1.614  2011/03/10 16:27:20  ovandewi
 * PR5523: book driver bit
 *
 * Revision 1.613  2011/02/28 19:38:10  wiese
 * F-5081 Add preliminary, limited VDSL RNC functionality
 *
 * Revision 1.612  2011/02/24 09:20:34  ovandewi
 * PR5395: add interop id
 *
 * Revision 1.611  2011/02/18 19:24:18  ovandewi
 * PR5399: command to inform PHY about ETR
 *
 * Revision 1.610  2011/02/14 22:51:53  rgreenf
 * PR5336: move sos/roc enable bits to aux feature bits as all demod capabilty bits are used
 *
 * Revision 1.609  2011/02/14 22:05:50  rgreenf
 * PR5336: move sos/roc enable bits
 *
 * Revision 1.608  2011/02/08 19:59:40  rgreenf
 * PR5336: add ROC and SOS flags
 *
 * Revision 1.607  2011/02/08 02:03:35  lockem
 * Real time improvements.
 *
 * Revision 1.606  2011/02/02 02:54:07  raman
 * merge in changes from AvC034_branch
 *
 * Revision 1.605  2011/01/29 00:49:37  mhegde
 * FW-5312: Fix Freeze on Noretrain bit set
 *
 * Revision 1.604  2011/01/24 10:00:45  ovandewi
 * PR5357: add ctlm dslam and interop for cipolicy in hs
 *
 * Revision 1.603  2011/01/17 22:42:17  mhegde
 * FW-5116: Port changes for heavy outof-band-rfi support for 6348
 *
 * Revision 1.602  2011/01/13 02:17:12  lockem
 * Add storage needed to update ginp counters in a controlled fashion.
 *
 * Revision 1.601  2011/01/12 21:03:47  wiese
 * F-5253  Restrict number of gated symbols during ADSL training impulse gating to 5000 symbols.
 *
 * Revision 1.600  2011/01/12 10:29:49  ovandewi
 * PR4798, PR4628: standard compliant G.vector changes
 *
 * Revision 1.599  2010/12/22 18:17:53  rwdu
 * extended bitswap counter reporting and BT bitswap workaround.
 *
 * Revision 1.598  2010/12/09 20:19:46  ovandewi
 * PR5218: control bit
 *
 * Revision 1.597  2010/12/08 02:40:52  ilyas
 * Created new configuration command
 *
 * Revision 1.596  2010/12/07 22:00:00  chungtse
 * Make the SES retrain threshold configurable as bits 10-15 of demod cap 2
 *
 * Revision 1.595  2010/12/01 23:24:00  yongbing
 * F-5107 Use kVendorGlobespan_LU_A2P72_HBI for limiting US PCB to 5dB
 *
 * Revision 1.594  2010/12/01 00:04:28  yongbing
 * F-5107 Cap upstream PCB to 5dB for Lucent 72 DSLAM, which helps improve upstream rate in A2+ noise case in null loop
 *
 * Revision 1.593  2010/11/30 19:06:38  lockem
 * Improved code placement for BG code.
 *
 * Revision 1.592  2010/11/12 22:57:22  lockem
 * Alter SoftDslCacheLineInvalidate() macro for simulation builds to work
 * around MIPS simulation issues.
 *
 * Revision 1.591  2010/11/11 23:25:31  wiese
 * Add 6368/6306 RNC cability for ADSL.
 *
 * Revision 1.590  2010/11/05 01:02:48  ilyas
 * Added delay before sending to SAR
 *
 * Revision 1.589  2010/10/27 21:00:34  pauljr
 * PR5124 - compute the SEFTR and store in the GinCounters structure for use by the driver.
 *
 * Revision 1.588  2010/10/23 00:53:15  chungtse
 * Put at least 3300 frames between bitswap requests to the ADTRAN TA1200 to workaround CO line drop issue
 *
 * Revision 1.587  2010/10/20 07:33:33  mhegde
 * FW-4994: Port changes to pass 50dB REIN test at BT
 *
 * Revision 1.586  2010/10/12 02:06:20  tonytran
 * Use a newly created status code to report status buffer write histogram data. Removed TIME_PROFILING related code from the previous checked-in
 *
 * Revision 1.585  2010/10/08 17:06:50  jinlu
 * FW5049 set CIpolicy0 as default
 *
 * Revision 1.584  2010/10/05 14:37:18  ovandewi
 * PR5049: enable CIPOLICY_SUPPORT
 *
 * Revision 1.583  2010/10/04 22:49:07  linyin
 * PR5053: Add control bit to reduce PCB for maximizing margin
 *
 * Revision 1.582  2010/10/02 01:52:22  yinboli
 * FW4946: Make auxFeature bitmap consistent across active branches
 *
 * Revision 1.581  2010/10/02 00:08:18  yinboli
 * FW4946: Add PTM Preemption selection for Adsl
 *
 * Revision 1.580  2010/10/01 18:48:01  jinlu
 * FW4965 align definitions
 *
 * Revision 1.579  2010/09/29 15:44:55  pauljr
 * PR5027 - add in adsl1 specific filter for CO4 use when the driver bit is set. This also addresses a training message decode problem with the CO4 due to it not sending the lower set of C-MSG tones
 *
 * Revision 1.578  2010/09/29 08:53:13  ovandewi
 * PR4905: only define g993p2 state when VDSL_MODEM is compiled
 *
 * Revision 1.577  2010/09/28 21:35:59  chungtse
 * Record the last transmitted and received state for VDSL2 and send status at the end of diags mode like ADSL2
 *
 * Revision 1.576  2010/09/22 17:56:55  rgreenf
 * PR4990: add mechanism to return ikns fw version and enable IFFT128 mode for US0 up to 64 tones
 *
 * Revision 1.575  2010/09/15 19:21:55  yinboli
 * FW4977: Huawei MA5100 margin test at 4900ft
 *
 * Revision 1.574  2010/09/14 16:11:48  jboxho
 * Definition of SoftDslGetDataSymbolFrequency
 *
 * Revision 1.573  2010/09/11 02:14:24  yinboli
 * FW4971: AT&T Annex L fallback workaround
 *
 * Revision 1.572  2010/09/09 03:30:05  lockem
 * Add function to initialize framerdeframerparams.
 *
 * Revision 1.571  2010/09/08 23:26:54  yongbing
 * F-4955 Force first startup to be the same as last good connection, and allow driver to set switching time for 6368
 *
 * Revision 1.570  2010/09/08 18:07:53  rwdu
 * added test mode to trigger upstream bitswap for 6368 ADSL.
 *
 * Revision 1.569  2010/09/03 13:53:41  pauljr
 * PR4942 Port driver bit for CO4 specific ADSL2/2+ filter to TOT
 *
 * Revision 1.568  2010/09/03 12:02:21  ovandewi
 * PR4945: proper declarations
 *
 * Revision 1.567  2010/08/21 01:40:31  chungtse
 * Add support for last transmitted state upstream and downstream in ADSL2/2+
 *
 * Revision 1.566  2010/08/19 13:36:39  ovandewi
 * PR4901: ifndef G992P1_ANNEX_B AnnexL specific changes
 *
 * Revision 1.565  2010/08/13 03:22:11  tonytran
 * Added a command for notifying of RTX Test Mode
 *
 * Revision 1.564  2010/08/02 12:41:43  ovandewi
 * PR4798: add G.vector pilot sequence structure
 *
 * Revision 1.563  2010/07/23 00:38:45  lockem
 * Support for MEDLEY impulse gating of FEQ updates.
 *
 * Revision 1.562  2010/07/19 13:16:53  ovandewi
 * PR4764: add interop bit
 *
 * Revision 1.561  2010/07/15 23:29:18  jknittel
 * FWDSLCPEPHY-4483: Added Driver Control to Enable/Disable Missing Phone Filter Detector in Adsl2 mode
 *
 * Revision 1.560  2010/07/15 20:11:14  lockem
 * Add definition of impulse gating structure.
 *
 * Revision 1.559  2010/07/14 20:18:55  mhegde
 * FW-4763: Adjust G994 PSD for Ikanos CO5 VDSL2 IOP
 *
 * Revision 1.558  2010/07/13 18:31:26  mhegde
 * FW-4754 Avoid D=511 against IFX FW 10.7.2
 *
 * Revision 1.557  2010/07/13 17:39:25  mhegde
 * FW-4640 Initialize false detection counter
 *
 * Revision 1.556  2010/07/12 18:07:16  yinboli
 * FW4687: add logic to give G.DMT HS preference over T1.413
 *
 * Revision 1.555  2010/07/08 17:59:51  yongbing
 * F-4681 Retrain to force Annex L for CenturyLink CNXT DSLAM, and delayed retrain for IFX DSLAM, and improved BG table calculation
 *
 * Revision 1.554  2010/06/26 01:35:25  tonytran
 * Fixed S-298: MIB Data missing. Removed status indentation when parsing statuses in Bonding Slave GUI. Included print of ETR_kbps when parsing kDsl993p2FramerDeframer status and display INPrein in DSL State window in Q0 iso Q1 format. Cleaned up profiling code.
 *
 * Revision 1.553  2010/06/24 22:42:29  jknittel
 * FWDSLCPEPHY-4483: Ported Adsl1 missing phone filter detector to CPE code
 *
 * Revision 1.552  2010/06/22 21:53:30  pauljr
 * PR4660 remove redundant driver message for ETR and rely on FramerDeframerOption structure
 *
 * Revision 1.551  2010/06/21 15:09:51  pauljr
 * PR4660 Adding in storage of ETR for reporting to driver and CO
 *
 * Revision 1.550  2010/06/02 13:57:14  jboxho
 * Add kFireReXmtPrev17msUncRSCodewords counter in G.Inp build
 *
 * Revision 1.549  2010/05/20 13:05:44  jboxho
 * Remove useless roudtrip variables
 *
 * Revision 1.548  2010/05/12 19:37:16  ilyas
 * Added common macro to invalidate one cache line
 *
 * Revision 1.547  2010/05/11 23:08:05  seleod
 * Added reporting of some TR98 parameters to the start of prepare for showtime in ADSL mode. Alos cleaned up some existing TR98 functions to make them useable in ADSl and VDSL modes, as well as non 6368 chips. This is all related to bug FWDSLCPEPHY-4122
 *
 * Revision 1.546  2010/05/11 15:40:47  jboxho
 * Fix G.Inp Qrx memory management on BCM6368 chipset familly
 *
 * Revision 1.545  2010/05/06 15:14:47  jboxho
 * Rename minS into invS
 *
 * Revision 1.544  2010/05/05 02:39:33  ilyas
 * Changed G.inp counter report
 *
 * Revision 1.543  2010/05/03 19:19:42  mhegde
 * Port ADSL2+ AnnexB IOP against INFN to TOT
 *
 * Revision 1.542  2010/04/30 10:17:38  jboxho
 * ADSL G.Inp rateselect implementation
 *
 * Revision 1.541  2010/04/30 02:54:47  lockem
 * Move FramerDeframerOptions definition to here so that it is visible to
 * all builds.
 *
 * Revision 1.540  2010/04/28 19:58:53  rwdu
 * fixed adsl-only targets.
 *
 * Revision 1.539  2010/04/28 08:29:56  ovandewi
 * PR3736: fix ADSL_ONLY builds
 *
 * Revision 1.538  2010/04/27 14:37:38  jboxho
 * Rework g992FireSpecifications structure
 *
 * Revision 1.537  2010/04/27 12:07:19  jboxho
 * Reorganize define and structure definition for code reuse
 *
 * Revision 1.536  2010/04/23 12:57:55  ovandewi
 * PR4496: move aggregation register to common core
 *
 * Revision 1.535  2010/04/22 14:05:32  jboxho
 * Make direction definitions available for dsl repository
 *
 * Revision 1.534  2010/04/16 12:55:37  ovandewi
 * PR4496: support bonding discovery
 *
 * Revision 1.533  2010/04/15 15:21:27  jboxho
 * Add dslConnectInfoStatusCode for G.Inp monitoring counter reporting
 *
 * Revision 1.532  2010/04/15 11:52:45  jboxho
 * Add G.Inp specific raw ARQ counter and define GinpCounters structure
 *
 * Revision 1.531  2010/04/12 09:36:51  jboxho
 * Fix compilation issue on non-PhyR targets
 *
 * Revision 1.530  2010/04/09 16:40:42  jboxho
 * Restore and further implement  VDSL G.Inp
 *
 * Revision 1.529  2010/04/07 17:47:56  ilyas
 * Merged auxFeatures definitions with 30h_branch
 *
 * Revision 1.528  2010/04/01 10:29:59  ovandewi
 * PR4304: rework to avoid no train with G994P1_NSIF_CONTROL approach
 *
 * Revision 1.527  2010/03/31 03:29:02  lockem
 * Add auxFeatures control bit definitions for G.inp enable.
 * Add definitions to replace PhyR and Ginp magic numbers with named values.
 *
 * Revision 1.526  2010/03/29 14:56:09  ovandewi
 * PR4340: port from AvC030_branch
 *
 * Revision 1.525  2010/03/20 04:10:02  lockem
 * Implement G.inp draft13 for ADSL.
 *
 * Revision 1.524  2010/03/09 20:07:15  lockem
 * Reserve space for G.inp performance counters.
 *
 * Revision 1.523  2010/03/01 14:20:17  ovandewi
 * PR3736: allow G.INP negotiation + allow FireConfig casting
 *
 * Revision 1.522  2010/02/24 00:44:47  yongbing
 * F-4304 If the DSLAM is IKNS with early firmware revision, retrain with G.994 tone power matching those of VCOPE
 *
 * Revision 1.521  2010/02/23 03:54:33  lockem
 * Reserve space to hold G.inp parameters.
 *
 * Revision 1.520  2010/02/20 01:32:43  yongbing
 * F-4289 Increase G.994 time if G.994 signal is detected or starting from showtime retrain with G.994 startup
 *
 * Revision 1.519  2010/02/19 15:16:04  jboxho
 * Add G.Inp support control
 *
 * Revision 1.518  2010/01/28 22:52:10  ovandewi
 * PR3904: book command number
 *
 * Revision 1.517  2010/01/26 09:11:40  ovandewi
 * PR3904: define status before merge
 *
 * Revision 1.516  2010/01/25 10:17:23  ovandewi
 * IKNS interop
 *
 * Revision 1.515  2010/01/22 13:58:06  jboxho
 * G.Inp/PhyR global implementation
 *
 * Revision 1.514  2010/01/13 16:02:59  ilyas
 * Retrain on BG task global (5sec) timeout or if BG queue is full
 *
 * Revision 1.513  2010/01/06 02:11:59  ilyas
 * Drain pending TX data on exception. Implement PTM software sync
 *
 * Revision 1.512  2009/12/29 17:07:31  jboxho
 * Support for G.Inp (F-3736)
 *
 * Revision 1.511  2009/12/19 02:58:58  mding
 * define new status reporting of TR98 params
 *
 * Revision 1.510  2009/12/14 14:20:24  jboxho
 * Unify PhyR/Ginp (F-3736)
 *
 * Revision 1.509  2009/12/04 22:56:18  tonytran
 * Fixed the DslDiags crash during regression test
 *
 * Revision 1.508  2009/12/03 02:31:13  raman
 * - Add code for testing 24 bits RRC for upstream PhyR
 * - verified on the board
 * - All changes qualified with RRC_24BITS
 *
 * Revision 1.507  2009/12/02 00:26:52  mding
 * integrate more status reporting into TR98
 *
 * Revision 1.506  2009/12/01 00:08:00  mding
 * add two more TR98 params
 *
 * Revision 1.505  2009/11/20 19:10:33  mding
 * make TR98params structure visible to 6348 targets
 *
 * Revision 1.504  2009/11/19 23:29:58  mding
 * add func prototype of TR98 init
 *
 * Revision 1.503  2009/11/14 00:43:18  mhegde
 * PR33425: Report local estimated DS ACTATP to driver
 *
 * Revision 1.502  2009/11/12 07:12:48  rwdu
 * make downstream dynamicF work.
 *
 * Revision 1.501  2009/10/30 23:13:25  jinlu
 * PR32848-a add definition for type 4 olr support
 *
 * Revision 1.500  2009/10/30 22:42:55  jinlu
 * PR32848-a move vdsl-only bitmap in cfgFlags
 *
 * Revision 1.499  2009/10/30 20:53:33  jinlu
 * PR32848 define bitmap in auxFeatures
 *
 * Revision 1.498  2009/10/20 00:07:46  mhegde
 * PR31205: revert to default enable for 15bit us fix for GSPN
 *
 * Revision 1.497  2009/10/17 18:36:52  mhegde
 * PR31205: Change the name of driver bit as Enable
 *
 * Revision 1.496  2009/10/09 19:13:51  mhegde
 * PR31205: Send blank CPE vendorID in AnnexM against CNXT CO to achieve bimax of 15 in US
 *
 * Revision 1.495  2009/10/08 13:43:20  ovandewi
 * PR31205: disable in auxFeatures
 *
 * Revision 1.494  2009/09/28 22:32:57  rgreenf
 * PR33224: move driver bit to cfgFlags
 *
 * Revision 1.493  2009/09/28 19:09:30  rgreenf
 * PR33224: Only enable Ifx periodic start phase when auxilary bit set to 0x00002000
 *
 * Revision 1.492  2009/09/25 18:30:40  mhegde
 * PR33244: Avoid filt 6 against AMA DSLAM with GSPN CO
 *
 * Revision 1.491  2009/09/15 20:19:47  jinlu
 * PR33229 use adsl configure --phycfg to enable/disable unconditional fast bs - port from c13b
 *
 * Revision 1.490  2009/09/11 19:39:38  lockem
 * ADSL bonding cache placement optimization
 *
 * Revision 1.489  2009/09/07 13:42:40  jboxho
 * Define kG994VendorID (PR33211)
 *
 * Revision 1.488  2009/09/03 19:26:50  jinlu
 * PR32861 code clean up, add configuration bits; PR32329 code clean up, add configuration bits and test mode control
 *
 * Revision 1.487  2009/08/28 20:58:14  yongbing
 * PR33188 Move ADSL global variables into one of global structures
 *
 * Revision 1.486  2009/08/27 21:56:39  nino
 * Add kDslNtrConfig command code.
 *
 * Revision 1.485  2009/08/26 01:57:53  ilyas
 * Added statuses for Host MIPS register read/write
 *
 * Revision 1.484  2009/08/18 21:35:33  mhegde
 * PR33164 : Fix 6368 build issue
 *
 * Revision 1.483  2009/08/18 04:50:49  mhegde
 * PR33164 : Port PR 33118 to TOT
 *
 * Revision 1.482  2009/08/12 23:59:49  ilyas
 * Created section for constants in SDRAM - SLOW_CONST
 *
 * Revision 1.481  2009/08/04 23:42:39  nino
 * Updating naming of status constants.
 *
 * Revision 1.480  2009/08/04 20:25:49  nino
 * Add kDslPhyToHostCtrlMsg status and related constants
 *
 * Revision 1.479  2009/07/29 21:26:00  ilyas
 * define GLOBAL_PTR_BIAS as 0 if not defined to use single set of pointer definitions
 *
 * Revision 1.478  2009/07/21 22:07:33  ilyas
 * Made E14 files not see G99x low level header files
 *
 * Revision 1.477  2009/07/08 22:35:57  ytan
 * first cut of re-arranging showtime_text and fast_text, plus other speed up
 *
 * Revision 1.476  2009/07/08 15:55:57  ovandewi
 * Change B43/J43 toggling bit
 *
 * Revision 1.475  2009/07/08 14:32:19  ovandewi
 * PR33043: identify infineon chips
 *
 * Revision 1.474  2009/07/06 06:58:07  ovandewi
 * PR33017: porting from AvC014 branch
 *
 * Revision 1.473  2009/06/25 20:21:03  ilyas
 * More command/status definitions for INM
 *
 * Revision 1.472  2009/06/18 21:27:43  jinlu
 * PR32992 make kDslDemod2Reserved consistant
 *
 * Revision 1.471  2009/06/11 07:30:42  mhegde
 * PR32973: Port ANFP mask changes to TOT
 *
 * Revision 1.470  2009/06/11 00:07:54  ilyas
 * Extend SoftDsl BG scheduler to replace E14s
 *
 * Revision 1.469  2009/06/04 19:18:09  ovandewi
 * PR32901: provide interop footprint and functions
 *
 * Revision 1.468  2009/06/02 06:50:53  mhegde
 * PR32802: Implement ANFP Mask for AnnexM
 *
 * Revision 1.467  2009/05/29 05:56:06  mhegde
 * PR32802: Code changes to support ANFP mask in AnnexM
 *
 * Revision 1.466  2009/05/28 02:04:50  mhegde
 * PR32918: Port last sync mode fix
 *
 * Revision 1.465  2009/05/21 22:16:53  yongbing
 * PR32835: Monitor SNR margin inside L2 and exit L2 if SNR margin drops beyond a threshold
 *
 * Revision 1.464  2009/05/19 22:02:13  lockem
 * Clean up globalvars.
 *
 * Revision 1.463  2009/05/08 06:03:02  mhegde
 * PR32608: second phase of INM with support for other INM_EQ_MODEs
 *
 * Revision 1.462  2009/05/03 17:03:01  ilyas
 * Support new status for E14 DBprints in Diags (for better binary matching in source release)
 *
 * Revision 1.461  2009/05/01 21:42:08  ilyas
 * Create common build structure for E14 code
 *
 * Revision 1.460  2009/04/30 21:04:27  mhegde
 * PR32827:Training counter when failure exceed certain threshold
 *
 * Revision 1.459  2009/04/30 15:56:28  ovandewi
 * PR32439: booking kDslSetSnrClampingMask  for consistency in driver
 *
 * Revision 1.458  2009/04/14 20:29:43  mhegde
 * PR32762: Provide driver bit control to disable C-ACT1
 *
 * Revision 1.457  2009/04/03 09:13:03  ovandewi
 * PR32569: support freeze
 *
 * Revision 1.456  2009/03/24 01:15:54  ilyas
 * div0 exception control
 *
 * Revision 1.455  2009/03/13 21:12:48  ilyas
 * Added NTR control and counters report
 *
 * Revision 1.454  2009/03/07 10:13:48  ilyas
 * Changed bit definition for bonding support indication; added command to change them
 *
 * Revision 1.453  2009/02/10 14:38:44  ovandewi
 * PR31211/31235 && 32438: defs
 *
 * Revision 1.452  2009/02/04 16:20:43  ilyas
 * Made definitions visible for non bonding targets
 *
 * Revision 1.451  2009/01/30 03:11:27  yongbing
 * PR32417 Add G.994 T1.413 switch for Telefonica
 *
 * Revision 1.450  2009/01/24 01:34:48  tonytran
 * Clean up
 *
 * Revision 1.449  2009/01/22 02:06:50  tonytran
 * Added support for bonded modem 1st phase
 *
 * Revision 1.448  2009/01/19 19:17:50  yongbing
 * PR32417 Add ANSI support for 6368, move ADSL VDSL toggling into G994 module
 *
 * Revision 1.447  2009/01/16 22:12:36  ilyas
 * Implement per thread storage of current gDslVars and link with corresponding E14 line object
 *
 * Revision 1.446  2009/01/16 00:16:09  nino
 * Add SoftDslMipsSleep() function and related constants.
 *
 * Revision 1.445  2009/01/14 04:01:53  mhegde
 * PR32543: Define Constants for NLNM
 *
 * Revision 1.444  2008/11/21 02:10:48  cnpeng
 * Create test code to evaluate EC rejection, use testcmd=17
 *
 * Revision 1.443  2008/11/19 19:29:41  mhegde
 * Port PR31535/31552 changes form 23 branch
 *
 * Revision 1.442  2008/11/13 15:46:46  jboxho
 * Remove useless definitions
 *
 * Revision 1.441  2008/10/22 21:32:15  mhegde
 * PR32234: Add VendorIDCentilliumAllZero
 *
 * Revision 1.440  2008/10/22 04:48:46  ilyas
 * Added test command to change AFEid for regression testing
 *
 * Revision 1.439  2008/10/17 04:08:55  yongbing
 * PR32195 Control VDSL/ADSL toggle in AFE module by command, the default is disable toggling
 *
 * Revision 1.438  2008/10/11 01:23:42  ilyas
 * ATM and PTM support for 6362 DHIF
 *
 * Revision 1.437  2008/09/25 15:59:34  ovandewi
 * PR30868 support virtual noise
 *
 * Revision 1.436  2008/09/15 22:32:53  mhegde
 * PR32128 Add support to detect firmware id for CNXT CO in A2P72/HBI
 *
 * Revision 1.435  2008/08/29 18:04:31  yongbing
 * PR32065 For Infineon ADSL2 DSLAM, retrain with VDSL2 disabled if the CLR is not accepted by DSLAM
 *
 * Revision 1.434  2008/08/22 00:48:38  rgreenf
 * src/Adsl/Main/SoftDsl.h
 *
 * Revision 1.433  2008/08/07 01:06:46  ovandewi
 * temp dbg print in 6368 + ptm-o-adsl
 *
 * Revision 1.432  2008/08/04 20:30:13  ilyas
 * Added 2nd line support for background tasks
 *
 * Revision 1.431  2008/07/31 22:42:36  rgreenf
 * add status reports for annex and profile
 *
 * Revision 1.430  2008/07/31 18:27:33  tonytran
 * Added a command for DslDiags to receive Phy Info
 *
 * Revision 1.429  2008/07/29 16:03:58  jboxho
 * Update structure to support PhyR version 01
 *
 * Revision 1.428  2008/07/28 16:08:50  jboxho
 * Add INPrein to PhyR specification variable
 *
 * Revision 1.427  2008/07/22 23:28:01  ilyas
 * Changed AfeID definitions
 *
 * Revision 1.426  2008/07/11 22:39:59  ilyas
 * Added initial afeInfo initialization
 *
 * Revision 1.425  2008/07/01 16:20:43  ovandewi
 * PR31784: first cut at annex J
 *
 * Revision 1.424  2008/06/17 16:04:56  jboxho
 * Add fireSpec validated variable for potential PhyR specification fix
 *
 * Revision 1.423  2008/06/11 17:25:57  ovandewi
 * add CONEXANT as a vendor
 *
 * Revision 1.422  2008/06/05 18:07:05  ovandewi
 * use status to report Line Driver type
 *
 * Revision 1.421  2008/05/24 01:04:54  mhegde
 * PR31861: Porting TR-67 Improvements changes to top of tree
 *
 * Revision 1.420  2008/05/14 22:00:01  jboxho
 * Add INPminPhyR and rein variables to g992FireSpecifications structure
 *
 * Revision 1.419  2008/05/14 00:41:40  tonytran
 * Added commands for VDSL AFE test configurations
 *
 * Revision 1.418  2008/05/13 17:11:41  jinlu
 * PR31781-f enable ds sra for vdsl; add separate control for bs & sra; adapt tone selection with sra direction
 *
 * Revision 1.417  2008/05/13 01:05:39  ovandewi
 * add REIN counter measure control bit
 *
 * Revision 1.416  2008/05/07 01:02:33  tonytran
 * Added support for VDSL SRA from driver 21_rc0
 *
 * Revision 1.415  2008/04/28 11:42:43  rgreenf
 * add new feq scaling state to state reporting
 *
 * Revision 1.414  2008/04/24 19:24:34  ilyas
 * Report extra connection parameters such as AHIF channel ID TC type, etc. in ADSL mode
 *
 * Revision 1.413  2008/04/24 00:58:44  tonytran
 * Added a new define so PHY can request a PHY reset from the Host
 *
 * Revision 1.412  2008/04/12 01:12:37  ovandewi
 * Add BRCM VE_6_3_FIX
 *
 * Revision 1.411  2008/04/04 13:53:53  ovandewi
 * add Nitro and merge from 23branch
 *
 * Revision 1.410  2008/04/02 23:56:08  ovandewi
 * add chipsetVersion
 *
 * Revision 1.409  2008/03/28 17:32:33  rgreenf
 * add band plan phase status
 *
 * Revision 1.408  2008/03/06 23:39:56  ilyas
 * Aligned phase counters with G.993 training/showtime symbol counters. Added #define to make DpApiPrintfs include phase counters
 *
 * Revision 1.407  2008/03/04 00:08:53  ilyas
 * Added SoftDslDBPrintf to print with phase and total symbol counters
 *
 * Revision 1.406  2008/02/22 09:56:44  jboxho
 * PhyR half roundtrip differentiation between Xmt and Rcv (PR31587)
 *
 * Revision 1.405  2008/02/20 22:05:41  jinlu
 * PR31572 Add control to disable/enable us bs from dsldiag and from dslcmd
 *
 * Revision 1.404  2008/02/12 22:52:18  tonytran
 * Added 2 new AFE test commands
 *
 * Revision 1.403  2008/01/28 20:50:03  ilyas
 * Added preliminary support for using stringIDs in printf statuses
 *
 * Revision 1.402  2008/01/10 22:50:48  ytan
 * speed up Qproc MIPS program
 *
 * Revision 1.401  2008/01/09 16:57:02  rgreenf
 * allow pll during agc and remove extra relock state
 *
 * Revision 1.400  2007/12/19 00:24:01  ilyas
 * Added statuses for dual latency support
 *
 * Revision 1.399  2007/12/14 22:10:40  tonytran
 * Incorporate other low-level profiling into the Generic Profiling Scheme, Added commands to allow the initiaion of profiling from the target, and cleaned up un-used profiling data dump code
 *
 * Revision 1.398  2007/12/07 00:59:36  rgreenf
 * merge/implement LD mode
 *
 * Revision 1.397  2007/12/06 10:35:53  jboxho
 * Make PhyR compilation flags independent: FIRE_RETRANSMISSION (PhyR implementation in DS), FIRE_XMT_6368 (PhyR implementation in US)
 *
 * Revision 1.396  2007/12/05 19:15:25  jinlu
 * PR31191: fix dual latency support for DS bitswap; add margin equalization mode
 *
 * Revision 1.395  2007/11/28 00:55:33  rgreenf
 * report near/far end tx power to dslDiags
 *
 * Revision 1.394  2007/11/27 20:09:01  jinlu
 * Add support of RX bitswap control commands
 *
 * Revision 1.393  2007/11/23 15:53:37  jboxho
 * 6368 PhyR US implementation
 *
 * Revision 1.392  2007/11/20 23:01:09  dadityan
 * Make SoftDslApi.h not include for Diags
 *
 * Revision 1.391  2007/11/19 22:03:02  nino
 * 6348 code modifications to support integration with 6368 low-level code.
 *
 * Revision 1.390  2007/11/16 09:01:25  jboxho
 * PhyR enable variable modification to cope with both directions
 *
 * Revision 1.389  2007/11/14 17:10:37  jboxho
 * Cleanup and 6368 PhyR Xmt implementation (PR31081)
 *
 * Revision 1.388  2007/11/14 02:45:17  ovandewi
 * add bitswap control commands
 *
 * Revision 1.387  2007/11/09 22:39:18  ovandewi
 * add Ikanos vendor and add fullVendorId struct for 6368
 *
 * Revision 1.386  2007/11/03 03:20:05  ilyas
 * Moved dslConnectionSetup definition to SoftDsl.h to make it accessable by VDSL code
 *
 * Revision 1.385  2007/10/29 18:35:55  avineet
 * Add GLOBAL_GDSVARS_REG optimization
 *
 * Revision 1.384  2007/10/29 11:29:52  jboxho
 * PhyR implementation on 6368: Define total aggregate (both directions: rcv & Xmt) available MEM for Intlv and PhyR
 *
 * Revision 1.383  2007/10/21 03:16:29  ilyas
 * Targets for ADSL only builds. Work in progress
 *
 * Revision 1.382  2007/10/17 07:46:00  tonytran
 * Added Generic Profiling support, DslDiags session lock option, and improved dynamic Cycle Profile graph display
 *
 * Revision 1.381  2007/10/15 03:45:32  mding
 * add SATNpb status report
 *
 * Revision 1.380  2007/10/05 02:38:39  mding
 * add G993 status report for showtime DS attainable net rate
 *
 * Revision 1.379  2007/10/04 20:50:00  ovandewi
 * add manufacturing test command
 *
 * Revision 1.378  2007/10/04 00:38:37  mding
 * add G993 status report for SNRM/SNRMpb
 *
 * Revision 1.377  2007/10/03 16:05:26  jboxho
 * PhyR implementation on 6368 (PR31081)
 *
 * Revision 1.376  2007/10/01 17:49:12  yongbing
 * Merge with Release 23 branch for ADSL
 *
 * Revision 1.375  2007/09/27 18:28:42  lke
 * change fire kDatablockBuffLength etc
 *
 * Revision 1.374  2007/09/15 00:03:23  lke
 * crc problem solved
 *
 * Revision 1.373  2007/09/06 16:16:55  rgreenf
 * add attainable rate reporting
 *
 * Revision 1.372  2007/08/24 16:23:08  rgreenf
 * correct rx synchro state messages
 *
 * Revision 1.371  2007/08/20 21:34:04  rgreenf
 * added training phase 6368 status reporting - ongoing
 *
 * Revision 1.370  2007/08/20 16:34:19  rgreenf
 * added training phase 6368 status reporting - ongoing
 *
 * Revision 1.369  2007/08/18 00:30:18  jboxho
 * PhyR implementation on 6368 - first step
 *
 * Revision 1.368  2007/08/17 08:27:40  ilyas
 * Moved inclusion of SoftA/VDsl. to the end so that all definitions are visible there
 *
 * Revision 1.367  2007/08/15 00:25:25  tonytran
 * Added a submenu for user to load string database and removed loading string database at startup
 *
 * Revision 1.366  2007/08/12 22:32:52  ilyas
 * Made compile with gcc 3.2.2
 *
 * Revision 1.365  2007/08/07 21:44:15  ytan
 * add bcm6368 vars for combined adsl/vdsl/hs structure
 *
 * Revision 1.364  2007/08/07 00:24:51  ilyas
 * Moved gDslVars address conversion macro to SoftDsl.h to so that it can be used by 6368 h/w code
 *
 * Revision 1.363  2007/08/06 03:02:02  ilyas
 * Made build with Linux and DslDiags
 *
 * Revision 1.362  2007/08/05 19:00:53  ilyas
 * Added overhead message support to VDSL build and status definition for DBPrint
 *
 * Revision 1.361  2007/08/03 21:17:06  ytan
 * add adsl/vdsl union data structure - not use yet
 *
 * Revision 1.360  2007/08/02 23:52:33  ilyas
 * Made status reporting mechanism available for E14 and low level code
 *
 * Revision 1.359  2007/07/29 19:47:04  ilyas
 * Added OpenFile function that deletes existing file
 *
 * Revision 1.358  2007/07/26 23:38:54  mding
 * only initial 1 line G994 for now, need to reduce G994 FFT size later for bonding
 *
 * Revision 1.357  2007/07/25 21:30:09  tonytran
 * Final changes to data structure for eye data support for 6368
 *
 * Revision 1.356  2007/07/25 18:07:28  tonytran
 * Use compile flag BCM6368_SRC instead of G993 for 6368 eye data support code
 *
 * Revision 1.355  2007/07/25 01:30:23  tonytran
 * Added eye data support for 6368
 *
 * Revision 1.354  2007/07/24 18:27:36  ytan
 * fix 6348 adsl compile problem
 *
 * Revision 1.353  2007/07/23 18:14:15  ytan
 * unino G994 and Vdsl data buffer
 *
 * Revision 1.352  2007/07/05 00:38:31  ilyas
 * Chnaged definitions to make previous version of Diags work
 *
 * Revision 1.351  2007/07/03 20:25:57  tonytran
 * Added support for block/afe test
 *
 * Revision 1.350  2007/06/26 18:50:24  ilyas
 * Added write-to-file capability to PHY
 *
 * Revision 1.349  2007/06/25 21:16:16  tonytran
 * Update naming of some defines for afeTest.
 *
 * Revision 1.348  2007/06/25 07:15:26  ilyas
 * Made build with simulation targets as well
 *
 * Revision 1.347  2007/06/23 01:30:03  tonytran
 * Added a few commands and statuses to support AFE test
 *
 * Revision 1.346  2007/06/19 01:31:10  ovandewi
 * change mips profilign status name
 *
 * Revision 1.345  2007/06/04 22:52:30  yongbing
 * PR31023 Add command and status for MIPS cycle measurement
 *
 * Revision 1.344  2007/06/03 05:36:50  ilyas
 * Created firmware target for 6368
 *
 * Revision 1.343  2007/06/02 17:51:47  ilyas
 * Created firmware target for 6368, ongoing work
 *
 * Revision 1.342  2007/06/02 00:56:51  ovandewi
 * merge rel 23 branch
 *
 * Revision 1.341  2007/06/01 00:39:05  ilyas
 * Moved G993p2 definitions to a separate file for E14 code
 *
 * Revision 1.340  2007/05/29 22:47:42  dadityan
 * Afeloopback Test/ QLN monitoring mode
 *
 * Revision 1.339  2007/05/17 23:32:00  mprahlad
 * make sure negotiated capabilities ends up in selectedCapability structure
 * add structure to have selected profile content
 * clean up naming of g993 param fields
 *
 * Revision 1.338  2007/04/20 20:53:08  mprahlad
 * reduce gDslVars struct for 6368 builds - specifically for G994 only build
 *
 * Revision 1.337  2007/04/10 19:44:10  tonytran
 * Merged with release_A2pB024_rc4; Adsl Driver/Dsldiags need the head version+ changes in _A2pB024_rc4
 *
 * Revision 1.336  2007/04/09 20:46:21  mprahlad
 * add VDSL codepoints - initial rev
 *
 * Revision 1.335  2007/03/16 15:43:35  jboxho
 * Remove useless NEW_FIRE_PARAMS compilation flag
 *
 * Revision 1.334  2007/02/26 13:22:34  jboxho
 * Fire new parameters implementation (INPmax and minReXmtRate) (PR30818)
 *
 * Revision 1.333  2007/02/14 22:33:59  ilyas
 * Commented out DSLVARS_GLOBAL_REG
 *
 * Revision 1.332  2007/02/14 22:31:49  ilyas
 * Redefine gDslVars to use persistent  register. Added macros to 'remove' gDslVars - first function parameter
 *
 * Revision 1.331  2007/02/01 20:15:23  tonytran
 * Updated Fire counter define
 *
 * Revision 1.330  2007/01/25 01:25:43  tonytran
 * Added binary status output filtering; display Fire, AS and Bitswap counters in the State Summary Window
 *
 * Revision 1.329  2007/01/20 05:00:27  yongbing
 * PR30908: Create another control status when part of the band have margins below -1dB
 *
 * Revision 1.328  2007/01/19 21:15:37  ilyas
 * Merged vdsl-sjc tree
 *
 * Revision 1.327  2007/01/16 00:48:54  ovandewi
 * add BB pln reset indication
 *
 * Revision 1.326  2007/01/15 15:41:02  jboxho
 * Fire state reporting (PR30818)
 *
 * Revision 1.325  2006/12/22 14:46:41  jboxho
 * FIRE feature implementation: handshake, counters, block interleaving, new retransmit request format
 *
 * Revision 1.324  2006/12/08 17:46:13  ovandewi
 * bit is reserved regardless of FIRE being in the PHY
 *
 * Revision 1.323  2006/12/08 17:20:51  jboxho
 * DS Fire capability foreseen in demodCapabilities2 (PR30818)
 *
 * Revision 1.322  2006/10/27 00:16:51  ilyas
 * Added definitions for time based profiling
 *
 * Revision 1.321  2006/10/11 00:00:28  yongbing
 * PR 30854: Reduce SDRAM size by eliminating debug printf
 *
 * Revision 1.320  2006/09/04 14:25:21  ovandewi
 * means to retrain on min margin
 *
 * Revision 1.319  2006/07/06 20:59:41  mding
 * add status to update Gi only in the mib
 *
 * Revision 1.318  2006/06/15 17:10:48  ovandewi
 * add a bit to control 24 kbytes option
 *
 * Revision 1.317  2006/05/19 23:45:34  yongbing
 * Move some non-time-critial codes/data from Lmem to SDRAM for 24K, PR 30468
 *
 * Revision 1.316  2006/05/18 22:42:52  yongbing
 * Reverse last check in, uchar for D in G.DMT is sufficient, PR 30753
 *
 * Revision 1.315  2006/04/20 16:43:44  ovandewi
 * support D multiple of 32
 *
 * Revision 1.314  2006/04/18 06:56:39  ilyas
 * Added commands for setting ATM cells portID
 *
 * Revision 1.313  2006/04/15 11:53:20  ovandewi
 * annex M custom mode definition
 *
 * Revision 1.312  2006/03/24 18:01:38  ovandewi
 * add SES retrain defs
 *
 * Revision 1.311  2006/03/22 08:53:44  jboxho
 * Non-linear detection tool upgrade
 *
 * Revision 1.310  2006/02/24 19:38:55  ilyas
 * Added I432 command to chnage header handler callback for more efficient EOP workaround implementation
 *
 * Revision 1.309  2006/02/16 19:39:05  ilyas
 * Added AAL5 cell EOP monitoring
 *
 * Revision 1.308  2006/02/16 11:26:54  ovandewi
 * use status to report drop reason
 *
 * Revision 1.307  2006/02/06 21:11:10  ilyas
 * Implemented using  to address dslSlowVars (conditional compile)
 *
 * Revision 1.306  2006/02/04 02:32:48  linyin
 * Add AFE sample loss status
 *
 * Revision 1.305  2006/02/04 02:27:53  linyin
 * Reduce the possiblity to AFE sample lose
 *
 * Revision 1.304  2006/01/31 16:35:41  jboxho
 * PLN : Cycle optimization
 *
 * Revision 1.303  2006/01/30 17:03:46  jboxho
 * PLN: Cycle issue solved for short loop, Nitro off and interleave mode
 *
 * Revision 1.302  2006/01/26 16:21:36  jboxho
 * PLN tool: PLN message base re-initialized at each G994.1 start
 *
 * Revision 1.301  2006/01/16 15:15:06  jboxho
 * PLN Fix for Bin Table Initialization
 *
 * Revision 1.300  2006/01/13 17:14:06  ovandewi
 * PLN clean-up and add constants for diags compilation
 *
 * Revision 1.299  2006/01/12 00:12:35  ilyas
 * Defined macros and functions for checking SDRAM write completion and cache writeback
 *
 * Revision 1.298  2006/01/05 04:05:34  dadityan
 * PLN Status
 *
 * Revision 1.297  2006/01/04 16:38:00  jboxho
 * PLN command update: Impulse duration and inter-arrival bin definition tables & PLN status command
 *
 * Revision 1.296  2005/12/24 00:01:33  ilyas
 * Added definitions for PLN's programmable bins extensions
 *
 * Revision 1.295  2005/12/13 02:42:22  dslsjtst
 * PLN Control for Counters Added
 *
 * Revision 1.294  2005/11/22 18:45:22  ilyas
 * Added definitions for EC update in showtime (cold start), loop diagnostic (LD), SNR
 *
 * Revision 1.293  2005/11/02 18:17:20  ilyas
 * Added status for PLN message base (negotiated in NSIF)
 *
 * Revision 1.292  2005/10/31 18:28:53  ovandewi
 * add L2 disable flag + command if for pln margins
 *
 * Revision 1.291  2005/10/25 15:48:54  ovandewi
 * PLN bit mask for adaptive status
 *
 * Revision 1.290  2005/10/21 21:13:49  ilyas
 * Implemented queueing of background functions SoftDslBgScheduleXxx API
 *
 * Revision 1.289  2005/10/19 18:53:05  ilyas
 * Added margin level parameters (spec) to PLNStart command
 *
 * Revision 1.288  2005/10/19 06:05:50  ilyas
 * Enabled PLN and fix compile problems
 *
 * Revision 1.287  2005/10/13 22:14:53  ilyas
 * Pass G992p3 capability structure to DslDiags for printing
 *
 * Revision 1.286  2005/10/11 03:55:45  ilyas
 * Added more commands and statuses for PLN data
 *
 * Revision 1.285  2005/09/27 02:02:08  ilyas
 * Added commands and statuses for PLN measurements
 *
 * Revision 1.284  2005/09/25 05:27:37  ilyas
 * Added statuses for sample buffer reporting. Ifdef'ed out unused code
 *
 * Revision 1.283  2005/09/15 21:28:08  ilyas
 * Made unaligned DebugData work by marking offset and DMAing from word aligned address
 *
 * Revision 1.282  2005/07/29 02:50:39  kdu
 * PR30498: Report special value to driver for SNR margin in DELT mode
 *
 * Revision 1.281  2005/07/27 14:08:07  ovandewi
 * annexM PSD mask G992.[35] info
 *
 * Revision 1.280  2005/07/19 23:01:55  ovandewi
 * annex M EU 56 def
 *
 * Revision 1.279  2005/07/14 19:28:29  ilyas
 * Added macros to print data buffers and printf with string id
 *
 * Revision 1.278  2005/06/24 21:51:39  ilyas
 * Added definitions for debug data logging
 *
 * Revision 1.277  2005/06/10 17:37:25  yongbing
 * Add status definition for RDI recovery
 *
 * Revision 1.276  2005/06/08 00:42:48  ilyas
 * Merged G.994/T1.413 switch time change from 18c_branch
 *
 * Revision 1.275  2005/04/28 22:55:36  ilyas
 * Cleaned up kDslG992RunAnnexaP3ModeInAnnexaP5, kG992EnableAnnexM and kDslAtuChangeTxFilterRequest definitions
 *
 * Revision 1.274  2005/04/27 20:57:32  yongbing
 * Implement 32 frequency break points for TSSI, PR 30211
 *
 * Revision 1.273  2005/04/02 03:27:52  kdu
 * PR30236: Define kDslEnableRoundUpDSLoopAttn, this is shared with kDslCentilliumCRCWorkAroundEnabled.
 *
 * Revision 1.272  2005/04/01 21:56:39  ilyas
 * Added more test commands definitions
 *
 * Revision 1.271  2005/02/11 05:03:57  ilyas
 * Added support for DslOs
 *
 * Revision 1.270  2005/02/11 03:33:22  lke
 * Support 2X, 4X, and 8X spectrum in ANNEX_I DS
 *
 * Revision 1.269  2005/01/08 00:11:58  ilyas
 * Added definition for AnnexL status
 *
 * Revision 1.268  2004/12/18 00:52:35  mprahlad
 * Add Dig US Pwr cutback status
 *
 * Revision 1.267  2004/11/08 22:21:38  ytan
 * init swap state after retrain
 *
 * Revision 1.266  2004/11/05 21:16:50  ilyas
 * Added support for pwmSyncClock
 *
 * Revision 1.265  2004/10/28 20:05:17  gsyu
 * Fixed compilation errors for simulation targets
 *
 * Revision 1.264  2004/10/23 00:16:35  nino
 * Added kDslHardwareSetRcvAGC status to set absolute rcv agc gain.
 *
 * Revision 1.263  2004/10/22 21:21:06  ilyas
 * Fixed bit definition overlap in demodCapabilities
 *
 * Revision 1.262  2004/10/20 00:43:20  gsyu
 * Added constants to support new xmt sample buffer control scheme
 *
 * Revision 1.261  2004/10/12 01:09:28  nino
 * Remove kDslHardwareEnablePwmSyncClk and kDslHardwareSetPwmSyncClkFreq
 * status definitions. Add kDslEnablePwmSyncClk and kDslSetPwmSyncClkFreq
 * command definitions.
 *
 * Revision 1.260  2004/10/11 20:21:26  nino
 * Added kDslHardwareEnablePwmSyncClk and kDslHardwareSetPwmSynClkFreq hardware statuses.
 *
 * Revision 1.259  2004/10/07 19:17:29  nino
 * Added kDslHardwareGetRcvAGC status.
 *
 * Revision 1.258  2004/10/02 00:17:14  nino
 * Added kDslHardwareAGCSetPga2 and kDslSetPilotEyeDisplay status definitions.
 *
 * Revision 1.257  2004/08/27 01:00:30  mprahlad
 *
 * Keep kDslAtuChangeTxFilterRequest defined by default so ADSL1 only targets can
 * build
 *
 * Revision 1.256  2004/08/20 19:00:34  ilyas
 * Added power management code for 2+
 *
 * Revision 1.255  2004/08/17 23:18:25  kdu
 * Merged interop changes for TDC lab from a023e9.
 *
 * Revision 1.254  2004/07/22 00:56:03  yongbing
 * Add ADSL2 Annex B modulation definition
 *
 * Revision 1.253  2004/07/16 22:23:28  nino
 * - Defined macros to extract subcarrier and supported set information
 *   for tssi. Subcarrier and suported set indicator is packed into
 *   dsSubcarrier index array.
 *
 * Revision 1.252  2004/07/01 00:11:22  nino
 * Added preliminary code for debugDataHandler (inside of #if DEBUG_DATA_HANDLER).
 *
 * Revision 1.251  2004/06/24 03:08:39  ilyas
 * Added GFC mapping control for ATM bonding
 *
 * Revision 1.250  2004/06/23 00:03:20  khp
 * - shorten self test result register length to 1 (satisfied requirement
 *   at DT, no known requirement anywhere else)
 *
 * Revision 1.249  2004/06/15 20:18:33  ilyas
 * Made D uchar again for compatibility with older ADSl drivers that use this structure. ADSL driver will rely on G992p3 parameters for large D
 *
 * Revision 1.248  2004/06/12 00:26:03  gsyu
 * Added constants for AnnexM
 *
 * Revision 1.247  2004/06/10 18:53:24  yjchen
 * add large D support
 *
 * Revision 1.246  2004/06/04 01:55:00  linyin
 * Add a constant for SRA enable/disable
 *
 * Revision 1.245  2004/05/19 23:22:23  linyin
 * Support L2
 *
 * Revision 1.244  2004/05/15 03:04:58  ilyas
 * Added L3 test definition
 *
 * Revision 1.243  2004/05/14 03:04:38  ilyas
 * Fixed structure name typo
 *
 * Revision 1.242  2004/05/14 02:01:01  ilyas
 * Fixed structure name typo
 *
 * Revision 1.241  2004/05/14 01:21:49  nino
 * Added kDslSignalAttenuation, kDslAttainableNetDataRate kDslHLinScale constant definitions.
 *
 * Revision 1.240  2004/05/13 19:07:58  ilyas
 * Added new statuses for ADSL2
 *
 * Revision 1.239  2004/05/01 01:09:51  ilyas
 * Added power management command and statuses
 *
 * Revision 1.238  2004/04/23 22:50:38  ilyas
 * Implemented double buffering to ensure G.997 HDLC frame (OvhMsg) is continuous
 *
 * Revision 1.237  2004/03/31 18:57:39  ilyas
 * Added drop on data error capability control
 *
 * Revision 1.236  2004/03/30 03:11:30  ilyas
 * Added #ifdef for CFE build
 *
 * Revision 1.235  2004/03/29 23:06:39  ilyas
 * Added status for BG table update
 *
 * Revision 1.234  2004/03/17 02:49:49  ilyas
 * Turn off ATM bit reversal for Alcatel DSLAM only
 *
 * Revision 1.233  2004/03/11 03:09:48  mprahlad
 * Add test mode for afeloopback test
 *
 * Revision 1.232  2004/03/10 23:15:53  ilyas
 * Added ETSI modem support
 *
 * Revision 1.231  2004/03/04 19:28:14  linyin
 * Support adsl2plus
 *
 * Revision 1.230  2004/02/28 00:06:21  ilyas
 * Added OLR message definitions for ADSL2+
 *
 * Revision 1.229  2004/02/13 03:21:15  mprahlad
 * define kDslAturHwAgcMaxGain correctly for 6348
 *
 * Revision 1.228  2004/02/09 05:06:17  yongbing
 * Add ADSL2 bit swap function
 *
 * Revision 1.227  2004/02/04 02:08:19  linyin
 * remove the redefined kG992p5AnnexA
 *
 * Revision 1.226  2004/02/04 01:41:48  linyin
 * Add some variables for G992P5
 *
 * Revision 1.225  2004/02/03 19:12:22  gsyu
 * Added a dedicate structure and constants for G992P5
 *
 * Revision 1.224  2004/01/24 01:18:34  ytan
 * add multi-section swapping flag
 *
 * Revision 1.223  2004/01/17 00:21:48  ilyas
 * Added commands and statuses for OLR
 *
 * Revision 1.222  2004/01/13 19:12:37  gsyu
 * Added more constants for Double upstream
 *
 * Revision 1.221  2003/12/23 21:19:04  mprahlad
 * Define BCM6348_TEMP_MOVE_TO_LMEM to FAST_TEXT for 6348 targets - this is for
 * ADSL2/AnnexA multimode builds - move a few functions to Lmem for now to avoid
 * changes for swap on 6348.
 *
 * Revision 1.220  2003/12/19 21:21:53  ilyas
 * Added dying gasp support for ADSL2
 *
 * Revision 1.219  2003/12/05 02:09:51  mprahlad
 * Leave the AnalogEC defs in - saves ifdef-ing all uses of these defines.
 * Include Bcm6345_To_Bcm6348.h - to be able to pick up macros for the
 * transition
 *
 * Revision 1.218  2003/12/04 02:10:58  linyin
 * Redefine some constants for supporting different pilot and TTR
 *
 * Revision 1.217  2003/12/03 02:24:39  gsyu
 * Reverse previous check in for Double Upstream demo
 *
 * Revision 1.215  2003/11/20 00:58:47  yongbing
 * Merge ADSL2 functionalities into Annex A branch
 *
 * Revision 1.214  2003/11/06 00:35:06  nino
 * Added kDslWriteAfeRegCmd and kDslReadAfeRegCmd commands.
 *
 * Revision 1.213  2003/11/05 21:04:23  ilyas
 * Added more codes for LOG data
 *
 * Revision 1.212  2003/10/22 00:51:52  yjchen
 * define constant for quiet line noise
 *
 * Revision 1.211  2003/10/20 22:08:57  nino
 * Added kDslSetRcvGainCmd and kDslBypassRcvHpfCmd debug commands.
 *
 * Revision 1.210  2003/10/18 00:04:59  yjchen
 * define constants for G992P3 diagnostic mode channel response
 *
 * Revision 1.209  2003/10/17 22:41:29  yongbing
 * Add INP message support
 *
 * Revision 1.208  2003/10/16 00:06:09  uid1249
 * Moved G.994 definitions from G.994p1MainTypes.h
 *
 * Revision 1.207  2003/10/15 20:45:11  linyin
 * Add some constants for support Revision 2
 *
 * Revision 1.206  2003/10/14 22:04:02  ilyas
 * Added Nino's AFE statuses for 6348
 *
 * Revision 1.205  2003/10/10 18:49:26  gsyu
 * Added test modes to workaround the clock domain crossing bug, PR18038
 *
 * Revision 1.204  2003/09/30 19:27:46  mprahlad
 * ifdef AnalogEC definies with #ifndef BCM6348_SRC
 *
 * Revision 1.203  2003/09/26 19:36:34  linyin
 * Add annexi constant and vars
 *
 * Revision 1.202  2003/09/25 20:16:13  yjchen
 * remove featureNTR definition
 *
 * Revision 1.201  2003/09/08 20:29:51  ilyas
 * Added test commands for chip regression tests
 *
 * Revision 1.200  2003/08/26 00:58:14  ilyas
 * Added I432 reset command (for header compression)
 * Fixed SoftDsl time (for I432 header compression)
 *
 * Revision 1.199  2003/08/26 00:37:29  ilyas
 * #ifdef'ed DslFrameFunctions in dslCommand structure to save space
 *
 * Revision 1.198  2003/08/22 22:45:00  liang
 * Change the NF field in G992CodingParams from uchar to ushort to support K=256 (dataRate=255*32kbps) in fast path.
 *
 * Revision 1.197  2003/08/21 21:19:05  ilyas
 * Changed dataPumpCapabilities structure for G992P3
 *
 * Revision 1.196  2003/08/12 22:44:28  khp
 * - for Haixiang: added kDslTestMarginTweak command and marginTweakSpec
 *
 * Revision 1.195  2003/07/24 17:28:16  ovandewi
 * added Tx filter change request code
 *
 * Revision 1.194  2003/07/24 15:48:55  yongbing
 * Reduce TSSI buffer size to avoid crash at the beginning of G.994.1. Need to find out why
 *
 * Revision 1.193  2003/07/19 07:11:47  nino
 * Revert back to version 1.191.
 *
 * Revision 1.191  2003/07/17 21:25:25  yongbing
 * Add support for READSL2 and TSSI
 *
 * Revision 1.190  2003/07/14 19:42:33  yjchen
 * add constants for G992P3
 *
 * Revision 1.189  2003/07/10 23:07:11  liang
 * Add demodCapability bit to minimize showtime ATUC xmt power through b&g table.
 *
 * Revision 1.188  2003/07/08 22:18:50  liang
 * Added demodCapability bit for G.994.1 Annex A multimode operation.
 *
 * Revision 1.187  2003/07/07 23:24:43  ilyas
 * Added G.dmt.bis definitions
 *
 * Revision 1.186  2003/06/25 02:44:02  liang
 * Added demod capability bit kDslUE9000ADI918FECFixEnabled.
 * Added back kDslHWEnableAnalogECUpdate & kDslHWEnableAnalogEC for backward compatibility (annex A).
 *
 * Revision 1.185  2003/06/18 01:39:19  ilyas
 * Added AFE test commands. Add #defines for driver's builds
 *
 * Revision 1.184  2003/06/06 23:58:09  ilyas
 * Added command and status for standalone AFE tests
 *
 * Revision 1.183  2003/05/29 21:09:32  nino
 * - kDslHWEnableAnalogECUpdate define replaced with kDslHWSetDigitalEcUpdateMode
 * - kDslHWEnableAnalogEC       define replaced with kDslHWDisableDigitalECUpdate
 *
 * Revision 1.182  2003/04/15 22:08:15  liang
 * Changed one of the demodCapability bit name from last checkin.
 *
 * Revision 1.181  2003/04/13 19:25:54  liang
 * Added three more demodCapability bits.
 *
 * Revision 1.180  2003/04/02 02:09:17  liang
 * Added demodCapability bit for ADI low rate option fix disable.
 *
 * Revision 1.179  2003/03/18 18:22:06  yongbing
 * Use 32 tap TEQ for Annex I
 *
 * Revision 1.178  2003/03/06 00:58:07  ilyas
 * Added SetStausBuffer command
 *
 * Revision 1.177  2003/02/25 00:46:26  ilyas
 * Added T1.413 EOC vendor ID
 *
 * Revision 1.176  2003/02/21 23:30:54  ilyas
 * Added Xmtgain command framing mode status and T1413VendorId parameters
 *
 * Revision 1.175  2003/02/07 22:13:55  liang
 * Add demodCapabilities bits for sub-sample alignment and higher T1.413 level (used internally only).
 *
 * Revision 1.174  2003/01/23 02:54:07  liang
 * Added demod capability bit for bitswap enable.
 *
 * Revision 1.173  2002/12/13 18:36:33  yongbing
 * Add support for G.992.2 Annex C
 *
 * Revision 1.172  2002/12/10 23:27:12  ilyas
 * Extended dslException parameter structure to allow printout from DslDiags
 *
 * Revision 1.171  2002/12/06 02:10:19  liang
 * Moved the T1.413 RAck1/RAck2 switching variables to connection setup structure.
 * Added/Modified the training progress codes for T1.413 RAck1/RAck2 and upstream 2x IFFT disable.
 *
 * Revision 1.170  2002/11/11 00:20:05  liang
 * Add demod capability constant for internally disabling upstream 2x IFFT in T1.413 mode.
 *
 * Revision 1.169  2002/11/06 03:46:19  liang
 * Add training progress code for upstream 2x IFFT disable.
 *
 * Revision 1.168  2002/11/01 01:41:06  ilyas
 * Added flags for Centillium 4103 workarround
 *
 * Revision 1.167  2002/10/26 01:26:11  gsyu
 * Move SoftDslLineHandler from SDRAM to LMEM
 *
 * Revision 1.166  2002/10/20 18:56:20  khp
 * - for linyin
 *   - #ifdef NEC_NSIF_WORKAROUND:
 *     - add macros to extract NSIF status and fail counter vars
 *
 * Revision 1.165  2002/10/14 05:24:35  liang
 * Add training status code to request alternate xmt filter (for Samsung 6-port ADI918 DSLAMs) to meet KT 2km spec.
 *
 * Revision 1.164  2002/10/08 21:44:50  ilyas
 * Fixed EOC stuffing byte to indicate "no synchronization" action
 *
 * Revision 1.163  2002/10/03 19:34:24  ilyas
 * Added size for EOC serial number register
 *
 * Revision 1.162  2002/09/28 02:42:27  yongbing
 * Add retrain in T1.413 with R-Ack1 tone
 *
 * Revision 1.161  2002/09/28 01:23:35  gsyu
 * Reverse us2xifft change so that we can install new us2xifft on the tree
 *
 * Revision 1.160  2002/09/26 23:30:48  yongbing
 * Add synch symbol detection in Showtime
 *
 * Revision 1.159  2002/09/20 23:47:52  khp
 * - for gsyu: enable 2X IFFT for Annex A (XMT_FFT_SIZE_2X)
 *
 * Revision 1.158  2002/09/14 03:26:39  ilyas
 * Changed far-end RDI reporting
 *
 * Revision 1.157  2002/09/13 21:10:54  ilyas
 * Added reporting of remote modem LOS and RDI.
 * Moved G992CodingParams definition to SoftDsl.h
 *
 * Revision 1.156  2002/09/12 21:07:19  ilyas
 * Added HEC, OCD and LCD counters
 *
 * Revision 1.155  2002/09/09 21:31:30  linyin
 * Add two constant to support long reach
 *
 * Revision 1.154  2002/09/07 01:31:51  ilyas
 * Added support for OEM parameters
 *
 * Revision 1.153  2002/09/04 22:36:14  mprahlad
 * defines for non standard info added
 *
 * Revision 1.152  2002/08/02 21:59:09  liang
 * Enable G.992.2 carrierInfo in capabitilities when G.992.1 annex A is used for G.992.2.
 *
 * Revision 1.151  2002/07/29 20:01:03  ilyas
 * Added command for Atm VC map table change
 *
 * Revision 1.150  2002/07/18 22:30:47  liang
 * Add xmt power and power cutback related constants.
 *
 * Revision 1.149  2002/07/11 01:30:58  ilyas
 * Changed status for ShowtimeMargin reporting
 *
 * Revision 1.148  2002/07/09 19:19:09  ilyas
 * Added status parameters for ShowtimeSNRMargin info and command to filter
 * out SNR margin data
 *
 * Revision 1.147  2002/06/27 21:50:24  liang
 * Added test command related demodCapabilities bits.
 *
 * Revision 1.146  2002/06/26 21:29:00  liang
 * Added dsl test cmd structure and showtime margin connection info status.
 *
 * Revision 1.145  2002/06/15 05:15:51  ilyas
 * Added definitions for Ping, Dying Gasp and other test commands
 *
 * Revision 1.144  2002/05/30 19:55:15  ilyas
 * Added status for ADSL PHY MIPS exception
 * Changed conflicting definition for higher rates (S=1/2)
 *
 * Revision 1.143  2002/05/21 23:41:07  yongbing
 * First check-in of Annex C S=1/2 codes
 *
 * Revision 1.142  2002/04/29 22:25:09  georgep
 * Merge from branch annexC_demo - add status message constants
 *
 * Revision 1.141  2002/04/18 19:00:35  ilyas
 * Added include file for builds in CommEngine environment
 *
 * Revision 1.140  2002/04/18 00:18:36  yongbing
 * Add detailed timeout error messages
 *
 * Revision 1.139  2002/04/02 10:03:18  ilyas
 * Merged BERT from AnnexA branch
 *
 * Revision 1.138  2002/03/26 01:42:29  ilyas
 * Added timeout message constants for annex C
 *
 * Revision 1.137  2002/03/22 19:39:22  yongbing
 * Modify for co-exist of G994P1 and T1P413
 *
 * Revision 1.136  2002/03/22 01:19:40  ilyas
 * Add status message constants for total FEXT Bits, NEXT bits
 *
 * Revision 1.135  2002/03/10 22:32:24  liang
 * Added report constants for LOS recovery and timing tone index.
 *
 * Revision 1.134  2002/03/07 22:06:32  georgep
 * Replace ifdef G992P1 with G992P1_ANNEX_A for annex A variables
 *
 * Revision 1.133  2002/02/16 01:08:18  georgep
 * Add log constant for showtime mse
 *
 * Revision 1.132  2002/02/08 04:36:27  ilyas
 * Added commands for LOG file and fixed Idle mode pointer update
 *
 * Revision 1.131  2002/01/24 20:21:30  georgep
 * Add logging defines, remove fast retrain defines
 *
 * Revision 1.130  2002/01/19 23:59:17  ilyas
 * Added support for LOG and eye data to ADSL core target
 *
 * Revision 1.129  2002/01/16 23:43:54  liang
 * Remove the carriage return character from last checkin.
 *
 * Revision 1.128  2002/01/15 22:27:13  ilyas
 * Added command for ADSL loopback
 *
 * Revision 1.127  2002/01/10 07:18:22  ilyas
 * Added status for printf (mainly for ADSL core debugging)
 *
 * Revision 1.126  2001/12/21 22:45:34  ilyas
 * Added support for ADSL MIB data object
 *
 * Revision 1.125  2001/12/13 02:24:22  ilyas
 * Added G997 (Clear EOC and G997 framer) support
 *
 * Revision 1.124  2001/11/30 05:56:31  liang
 * Merged top of the branch AnnexBDevelopment onto top of the tree.
 *
 * Revision 1.123  2001/11/15 19:01:07  yongbing
 * Modify only T1.413 part to the top of tree based on AnnexBDevelopment branch
 *
 * Revision 1.122  2001/10/19 00:12:07  ilyas
 * Added support for frame oriented (no ATM) data link layer
 *
 * Revision 1.121  2001/10/09 22:35:13  ilyas
 * Added more ATM statistics and OAM support
 *
 * Revision 1.105.2.20  2001/11/27 02:32:03  liang
 * Combine vendor ID, serial #, and version number into SoftModemVersionNumber.c.
 *
 * Revision 1.105.2.19  2001/11/21 01:29:14  georgep
 * Add a status message define for annexC
 *
 * Revision 1.105.2.18  2001/11/08 23:26:28  yongbing
 * Add carrier selection function for Annex A and B
 *
 * Revision 1.105.2.17  2001/11/07 22:55:30  liang
 * Report G992 rcv msg CRC error as what it is instead of time out.
 *
 * Revision 1.105.2.16  2001/11/05 19:56:21  liang
 * Add DC offset info code.
 *
 * Revision 1.105.2.15  2001/10/16 00:47:16  yongbing
 * Add return-to-T1p413 starting point if in error
 *
 * Revision 1.105.2.14  2001/10/15 23:14:01  yjchen
 * remove ADSL_SINGLE_SYMBOL_BLOCK
 *
 * Revision 1.105.2.13  2001/10/12 18:07:16  yongbing
 * Add support for T1.413
 *
 * Revision 1.105.2.12  2001/10/04 00:23:52  liang
 * Add connection info constants for TEQ coef and PSD.
 *
 * Revision 1.105.2.11  2001/10/03 01:44:01  liang
 * Merged with codes from main tree (tag SoftDsl_2_18).
 *
 * Revision 1.105.2.10  2001/09/28 22:10:04  liang
 * Add G994 exchange message status reports.
 *
 * Revision 1.105.2.9  2001/09/26 18:08:21  georgep
 * Send status error message in case features field is not setup properly
 *
 * Revision 1.105.2.8  2001/09/05 01:58:13  georgep
 * Added status message for annexC measured delay
 *
 * Revision 1.105.2.7  2001/08/29 00:37:52  georgep
 * Add log constants for annexC
 *
 * Revision 1.105.2.6  2001/08/18 00:01:34  georgep
 * Add constants for annexC
 *
 * Revision 1.105.2.5  2001/08/08 17:33:28  yongbing
 * Merge with tag SoftDsl_2_17
 *
 * Revision 1.120  2001/08/29 02:56:01  ilyas
 * Added tests for flattening/unflatenning command and statuses (dual mode)
 *
 * Revision 1.119  2001/08/28 03:26:32  ilyas
 * Added support for running host and adsl core parts separately ("dual" mode)
 *
 * Revision 1.118  2001/08/16 02:16:10  khp
 * - mark functions with FAST_TEXT to reduce cycle counts for QPROC targets
 *   (replaces use of LMEM_INSN)
 *
 * Revision 1.117  2001/06/18 20:06:35  ilyas
 * Added forward declaration of dslCommandStruc to avoid gcc warnings
 *
 * Revision 1.116  2001/06/18 19:49:36  ilyas
 * Changes to include support for HOST_ONLY mode
 *
 * Revision 1.115  2001/06/01 22:00:33  ilyas
 * Changed ATM PHY interface to accomodate UTOPIA needs
 *
 * Revision 1.114  2001/05/16 06:22:24  liang
 * Added status reports for xmt & rcv prefix enable position.
 *
 * Revision 1.113  2001/05/02 20:34:32  georgep
 * Added log constants for snr1 calculation
 *
 * Revision 1.112  2001/04/25 01:20:11  ilyas
 *
 * Don't use DSL frame functions if ATM_LAYER is not defined
 *
 * Revision 1.111  2001/04/17 21:13:00  georgep
 * Define status constant kDslHWSetDigitalEcUpdateShift
 *
 * Revision 1.110  2001/04/16 23:38:36  georgep
 * Add HW AGC constants for ATUR
 *
 * Revision 1.109  2001/04/06 23:44:53  georgep
 * Added status constant for setting up digitalEcGainShift
 *
 * Revision 1.108  2001/03/29 05:58:34  liang
 * Replaced the Aware compatibility codes with automatic detection codes.
 *
 * Revision 1.107  2001/03/25 06:11:22  liang
 * Combined separate loop attenuation status for ATUR & ATUC into one status.
 * Replace separate hardware AGC info status for ATUR & ATUC into hardware AGC
 * request status and hardware AGC obtained status.
 * Use store AGC command to save hardware AGC value instead of returning value
 * from status report.
 *
 * Revision 1.106  2001/03/24 00:43:22  liang
 * Report more checksum results (NumOfCalls, txSignal, rxSignal & eyeData).
 *
 * Revision 1.105  2001/03/16 23:57:31  georgep
 * Added more loop attenuation reporting status constants
 *
 * Revision 1.104  2001/03/15 00:22:07  liang
 * Back to version 1.101.
 *
 * Revision 1.103  2001/03/15 00:03:44  yjchen
 * use kDslATURHardwareAGCInfo for AltoE14 AGC as well
 *
 * Revision 1.102  2001/03/14 23:10:56  yjchen
 * add defns for AltoE14 AGC
 *
 * Revision 1.101  2001/03/08 23:31:34  georgep
 * Added R, S, D, coding parameters to dslDataPumpCapabilities
 *
 * Revision 1.100  2001/02/10 03:03:09  ilyas
 * Added one more DslFrame function
 *
 * Revision 1.99  2001/02/09 01:55:27  ilyas
 * Added status codes and macros to support printing of AAL packets
 *
 * Revision 1.98  2001/01/30 23:28:10  georgep
 * Added kDslDspControlStatus for handling changes to dsp params
 *
 * Revision 1.97  2001/01/12 01:17:18  georgep
 * Added bit in demodCapabilities for analog echo cancellor
 *
 * Revision 1.96  2001/01/04 05:51:03  ilyas
 * Added more dslStatuses
 *
 * Revision 1.95  2000/12/21 05:46:07  ilyas
 * Added name for struct _dslFrame
 *
 * Revision 1.94  2000/12/13 22:04:39  liang
 * Add Reed-Solomon coding enable bit in demodCapabilities.
 *
 * Revision 1.93  2000/11/29 20:42:02  liang
 * Add defines for SNR & max achivable rate status and DEC enable demodCapabilities bit.
 *
 * Revision 1.92  2000/09/22 21:55:13  ilyas
 * Added support for DSL + Atm physical layer only (I.432) simulations
 *
 * Revision 1.91  2000/09/10 09:20:53  lkaplan
 * Improve interface for sending Eoc messages
 *
 * Revision 1.90  2000/09/08 19:37:58  lkaplan
 * Added code for handling EOC messages
 *
 * Revision 1.89  2000/09/07 23:02:27  georgep
 * Add HarwareAGC Bit to demod Capabilities
 *
 * Revision 1.88  2000/09/01 00:57:34  georgep
 * Added Hardware AGC status defines
 *
 * Revision 1.87  2000/08/31 19:04:26  liang
 * Added external reference for stack size requirement test functions.
 *
 * Revision 1.86  2000/08/24 23:16:46  liang
 * Increased sample block size for noBlock.
 *
 * Revision 1.85  2000/08/23 18:34:39  ilyas
 * Added XxxVcConfigure function
 *
 * Revision 1.84  2000/08/05 00:25:04  georgep
 * Redefine sampling freq constants
 *
 * Revision 1.83  2000/08/03 14:04:00  liang
 * Add hardware time tracking clock error reset code.
 *
 * Revision 1.82  2000/07/23 20:52:52  ilyas
 * Added xxxFrameBufSetAddress() function for ATM framer layers
 * Rearranged linkLayer functions in one structure which is passed as a
 * parameter to xxxLinkLayerInit() function to be set there
 *
 * Revision 1.81  2000/07/18 20:03:24  ilyas
 * Changed DslFrame functions definitions to macros,
 * Removed gDslVars from their parameter list
 *
 * Revision 1.80  2000/07/17 21:08:15  lkaplan
 * removed global pointer
 *
 * Revision 1.79  2000/06/21 20:38:44  georgep
 * Added bit to demodCapabilities for HW_TIME_TRACKING
 *
 * Revision 1.78  2000/06/19 19:57:55  georgep
 * Added constants for logging of HWResampler data
 *
 * Revision 1.77  2000/06/02 18:57:21  ilyas
 * Added support for DSL buffers consisting of many ATM cells
 *
 * Revision 1.76  2000/05/27 02:19:28  liang
 * G992MonitorParams structure is moved here, and Tx/Rx data handler type definitions changed.
 *
 * Revision 1.75  2000/05/15 18:17:21  liang
 * Added statuses for sent and received frames
 *
 * Revision 1.74  2000/05/14 01:56:38  ilyas
 * Added ATM cell printouts
 *
 * Revision 1.73  2000/05/09 23:00:26  ilyas
 * Added ATM status messages, ATM timer, Tx frames flush on timeout
 * Fixed a bug - adding flushed Tx frames to the list of free Rx frames
 *
 * Revision 1.72  2000/05/03 18:01:18  georgep
 * Removed old function declarations for Eoc/Aoc
 *
 * Revision 1.71  2000/05/03 03:57:04  ilyas
 * Added LOG file support for writing ATM data
 *
 * Revision 1.70  2000/05/02 00:04:36  liang
 * Add showtime monitoring and message exchange info constants.
 *
 * Revision 1.69  2000/04/28 23:34:20  yongbing
 * Add constants for reporting error events in performance monitoring
 *
 * Revision 1.68  2000/04/21 23:09:04  liang
 * Added G992 time out training progress constant.
 *
 * Revision 1.67  2000/04/19 00:31:47  ilyas
 * Added global SoftDsl functions for Vc, added OOB info functions
 *
 * Revision 1.66  2000/04/18 00:45:31  yongbing
 * Add G.DMT new frame structure, define G992P1_NEWFRAME to enable, need ATM layer to work
 *
 * Revision 1.65  2000/04/15 01:48:34  georgep
 * Added T1p413 status constants
 *
 * Revision 1.64  2000/04/13 08:36:22  yura
 * Added SoftDslSetRefData, SoftDslGetRefData functions
 *
 * Revision 1.63  2000/04/13 05:42:35  georgep
 * Added constant for T1p413
 *
 * Revision 1.62  2000/04/05 21:49:54  liang
 * minor change.
 *
 * Revision 1.61  2000/04/04 04:16:06  liang
 * Merged with SoftDsl_0_03 from old tree.
 *
 * Revision 1.65  2000/04/04 01:47:21  ilyas
 * Implemented abstract dslFrame and dslFrameBuffer objects
 *
 * Revision 1.64  2000/04/01 08:12:10  yura
 * Added preliminary revision of the SoftDsl driver architecture
 *
 * Revision 1.63  2000/04/01 02:55:33  georgep
 * New defines for G992p2Profile Structure
 *
 * Revision 1.62  2000/04/01 00:50:36  yongbing
 * Add initial version of new frame structure for full-rate
 *
 * Revision 1.61  2000/03/24 03:30:45  georgep
 * Define new constant kDslUpstreamSamplingFreq
 *
 * Revision 1.60  2000/03/23 19:51:30  georgep
 * Define new features bits for G992p1
 *
 * Revision 1.59  2000/03/18 01:28:41  georgep
 * Changed connectionSetup to include G992p1 Capabilities
 *
 * Revision 1.58  2000/02/29 01:40:03  georgep
 * Changed modulationtype defines to be the same as SPAR1 in G994p1
 *
 * Revision 1.57  1999/11/19 01:03:19  george
 * Use Block Size 256 for single symbol Mode
 *
 * Revision 1.56  1999/11/18 02:37:43  george
 * Porting to 16Bit
 *
 * Revision 1.55  1999/11/12 02:12:55  george
 * Added status constant for reporting of profile channel matching calculation
 *
 * Revision 1.54  1999/11/11 19:19:42  george
 * Porting to 16Bit Compiler
 *
 * Revision 1.53  1999/11/05 01:27:06  liang
 * Add recovery-from-inpulse-noise progress report.
 *
 * Revision 1.52  1999/11/02 02:06:27  george
 * Added SNRMargin training status value
 *
 * Revision 1.51  1999/10/27 23:02:03  wan
 * Add G.994.1 setup in dslConnectionSetupStruct for setting up Initiation side
 *
 * Revision 1.50  1999/10/25 21:55:36  liang
 * Renamed the constant for FEQ output error.
 *
 * Revision 1.49  1999/10/23 02:20:55  george
 * Add debug data codes
 *
 * Revision 1.48  1999/10/19 23:59:06  liang
 * Change line handler interface to work with nonsymmetric sampling freq.
 *
 * Revision 1.47  1999/10/09 01:38:04  george
 * Define maxProfileNumber
 *
 * Revision 1.46  1999/10/07 23:30:51  wan
 * Add G.994.1 Tone and Fast Retrain Recov detections in G.992p2 SHOWTIME and Fast Retrain
 *
 * Revision 1.45  1999/10/06 13:59:27  liang
 * Escape to G994.1 should be done through status instead of command.
 *
 * Revision 1.44  1999/10/06 02:01:28  george
 * Add kDslReturnToG994p1Cmd
 *
 * Revision 1.43  1999/09/30 19:29:58  george
 * Add reporting constant for Fast Retrain
 *
 * Revision 1.42  1999/09/16 23:41:56  liang
 * Added command for host forced retrain.
 *
 * Revision 1.41  1999/08/20 00:47:25  wan
 * Add constants for Fast Retrain progress status
 *
 * Revision 1.40  1999/08/16 18:06:01  wan
 * Add more reporting constants for Fast Retrain
 *
 * Revision 1.39  1999/08/12 00:18:10  wan
 * Add several Fast Retrain Status constants
 *
 * Revision 1.38  1999/08/10 18:25:38  george
 * Define constants used for Fast Retrain
 *
 * Revision 1.37  1999/07/31 01:47:43  george
 * Add status constants for eoc/aoc
 *
 * Revision 1.36  1999/07/27 18:19:52  george
 * declare aoc/eoc functions
 *
 * Revision 1.35  1999/07/19 22:44:47  george
 * Add constants for G994p1 Message Exchange
 *
 * Revision 1.34  1999/07/16 02:03:03  liang
 * Modified Dsl link layer command spec structure.
 *
 * Revision 1.33  1999/07/14 22:53:16  george
 * Add Constants for G994p1
 *
 * Revision 1.32  1999/07/13 00:02:26  liang
 * Added more feature bits.
 *
 * Revision 1.31  1999/07/09 01:58:14  wan
 * Added more constants G.994.1 testing reports
 *
 * Revision 1.30  1999/07/07 23:51:04  liang
 * Added rcv power and loop attenuation reports.
 *
 * Revision 1.29  1999/07/06 21:32:01  liang
 * Added some aux. feature bits, and field performanceMargin was changed to noiseMargin in Capabilities.
 *
 * Revision 1.28  1999/07/03 01:40:17  liang
 * Redefined dsl command parameter list and added connection setup struct.
 *
 * Revision 1.27  1999/07/02 00:41:18  liang
 * Add bit and gain logging as well as rcv carrier range status.
 *
 * Revision 1.26  1999/06/25 21:37:10  wan
 * Work in progress for G994.1.
 *
 * Revision 1.25  1999/06/16 00:54:36  liang
 * Added Tx/Rx SHOWTIME active training progress codes.
 *
 * Revision 1.24  1999/06/11 21:59:37  wan
 * Added G994.1 fail status constant.
 *
 * Revision 1.23  1999/06/11 21:29:01  liang
 * Constants for C/R-Msgs was changed to C/R-Msg.
 *
 * Revision 1.22  1999/06/08 02:49:42  liang
 * Added SNR data logging.
 *
 * Revision 1.21  1999/06/07 21:05:08  liang
 * Added more training status values.
 *
 * Revision 1.20  1999/05/22 02:18:26  liang
 * More constant defines.
 *
 * Revision 1.19  1999/05/14 22:49:35  liang
 * Added more status codes and debug data codes.
 *
 * Revision 1.18  1999/04/12 22:41:39  liang
 * Work in progress.
 *
 * Revision 1.17  1999/04/01 20:28:07  liang
 * Added RReverb detect event status.
 *
 * Revision 1.16  1999/03/26 03:29:54  liang
 * Add DSL debug data constants.
 *
 * Revision 1.15  1999/03/08 21:58:00  liang
 * Added more constant definitions.
 *
 * Revision 1.14  1999/03/02 01:49:36  liang
 * Added more connection info codes.
 *
 * Revision 1.13  1999/03/02 00:25:55  liang
 * Added DSL tx and rx data handler type definitions.
 *
 * Revision 1.12  1999/02/27 01:16:55  liang
 * Increase allowable static memory size to a VERY large number for now.
 *
 * Revision 1.11  1999/02/25 00:24:06  liang
 * Increased symbol block size to 16.
 *
 * Revision 1.10  1999/02/23 22:03:26  liang
 * Increased maximal static memory size allowed.
 *
 * Revision 1.9  1999/02/17 02:39:21  ilyas
 * Changes for NDIS
 *
 * Revision 1.8  1999/02/11 22:44:30  ilyas
 * More definitions for ATM
 *
 * Revision 1.7  1999/02/10 01:56:38  liang
 * Added hooks for G994.1 and G992.2.
 *
 *
 *****************************************************************************/

#ifndef	SoftDslHeader
#define	SoftDslHeader

/* for builds in Linux/VxWorks CommEngine environment */
#if (defined(__KERNEL__) && !defined(LINUX_DRIVER)) || defined(VXWORKS) || defined(_WIN32_WCE) || defined(TARG_OS_RTEMS) || defined(_CFE_) || defined(__ECOS) || defined(_NOOS)
#include "AdslCoreDefs.h"
#else
#include "Bcm6345_To_Bcm6348.h"		/* File for 45->48 changes */
#endif

#ifndef GLOBAL_PTR_BIAS
#define GLOBAL_PTR_BIAS		0
#endif

#ifndef	SoftModemPh
#include "SoftModem.h"
#endif

#if defined(DSL_OS) && defined(MIPS_SRC)
#include "DslOs.h"
#endif
#ifndef MATLAB_MEX
#include "CircBuf.h"
#endif

#ifdef DSLVARS_GLOBAL_REG
#include "SoftDslVarsGlobalReg.h"
#endif

#if defined(G993) || defined(BCM6368_SRC) || defined(_ADSL_CORE_DEFS_H)
#include "SoftDslG993p2.h"
#endif

#include "EndianUtil.h"

/*
**
**		gDslVars address translation
**
*/

#ifndef DSLVARS_GLOBAL_REG
#define	gDslVarStartPtr			((void*)((uchar*)(gDslVars) - GLOBAL_PTR_BIAS))
#else
#define	gDslVarStartPtr			((void*)((uchar*)(gDslVarsReg) - GLOBAL_PTR_BIAS))
#endif

/*
**
**		Type definitions
**
*/

#ifdef __GNUC__
#define	ALIGN_PACKED __attribute__ ((packed))
#else
#define	ALIGN_PACKED
#endif

#ifndef CONST
#define CONST	const
#endif

#if defined(ATM) || defined(DSL_PACKET) || defined(PTM6465)
#define DSL_LINKLAYER
#endif

#if defined(ATM_LAYER) || defined(DSL_PACKET_LAYER) || defined(G997_1_FRAMER)
#define DSL_FRAME_FUNCTIONS
#endif

#define FLD_OFFSET(type,fld)	((long)(void *)&(((type *)0)->fld))

#include "Que.h"
#include "SoftAtmVc.h"

/*********************************************************************/
/* Shared VDSL-ADSL definitions */
#ifdef TRAINING_IMPULSE_GATING

#define kG992EqualizerImpDetMaxToneSetSize      24
#define kG993ImpDetMaxNumBands                  6

#ifdef BUMP_INP_FOR_TRAINING_REIN
#define kMaxReinInpBump         5   /* 5 symbols - we will increase the INP up to this level if needed (set to 7 or less) */
#define kMaxReinInpBumpQ8       (kMaxReinInpBump<<8)
#define kMinBurstThreshold      4   /* There must be at least this many bursts of a particular length to bump the INP */
#define kExcessiveImpulseNoiseBPSThreshold  40  /* average bursts per second */
#endif

typedef struct
    {
    ushort      impdet_tone_set[kG992EqualizerImpDetMaxToneSetSize];
    uint      impdet_error_norm[kG992EqualizerImpDetMaxToneSetSize];
    uint      impdet_impulse_norm;
    int         impdet_tone_set_length;
    int         impdet_consecutive_detections;
    int         impdet_sym_count;
    int         impdet_impulses_detected;
    int         impdet_symbols_gated;
    int         impdet_symbols_checked;
    Boolean     impdet_gate_impulse;
#ifdef VDSL_MODEM
    uchar       impdet_numbands;
    ushort      impdet_bandstart[kG993ImpDetMaxNumBands];
    ushort      impdet_bandlen[kG993ImpDetMaxNumBands];
#endif
    int         minINPQ8;
    int         current_correctable_symbolsQ8;
    int         correctable_symbols_per_symbolQ8;
#ifdef BUMP_INP_FOR_TRAINING_REIN
    int         impulseLengthInSymbols[(kMaxReinInpBump+1)];
#endif
    }
    impDetVarsType;
#endif /* TRAINING_IMPULSE_GATING */



/***********************************************************************/
typedef struct _dslFrameBuffer
	{
	struct _dslFrameBuffer *next;	/* link to the next buffer in the frame */
	void				*pData;	/* pointer to data */
	uint				length;	/* size (in bytes) of data */
	} dslFrameBuffer;

typedef struct _dslFrame
	{
	ulong			Reserved[3];

	uint			totalLength;	/* total amount of data in the packet */
	int				bufCnt;			/* buffer counter */
	struct _dslFrameBuffer *head;	/* first buffer in the chain */
	struct _dslFrameBuffer *tail;	/* last buffer in the chain  */
	} dslFrame;


/* VC types and parameters */

#define	kDslVcAtm		1

typedef	struct
	{
	uint	vcType;
	union
		{
		atmVcParams	atmParams;
		} params;
	} dslVcParams;

/*
**	Assuming that dslVcParams.params is the first field in VC
**	and RefData is the first field in dslVcParams.params
*/

#define	DslVcGetRefData(pVc)	(*(void **) (pVc))

/* Frame OOB types */

#define	kDslFrameAtm	1

typedef	struct
	{
	uint	frameType;
	union
		{
		atmOobPacketInfo	atmInfo;
		} param;
	} dslOobFrameInfo;


typedef struct
	{
	uint (SM_DECL *__DslFrameBufferGetLength) (dslFrameBuffer *fb);
	void * (SM_DECL *__DslFrameBufferGetAddress) (dslFrameBuffer *fb);
	void (SM_DECL *__DslFrameBufferSetLength) (dslFrameBuffer *fb, uint l);
	void (SM_DECL *__DslFrameBufferSetAddress) (dslFrameBuffer *fb, void *p);

	void (SM_DECL *__DslFrameInit) (dslFrame *f);
	uint (SM_DECL *__DslFrameGetLength) (dslFrame *pFrame);
	uint (SM_DECL *__DslFrameGetBufCnt) (dslFrame *pFrame);
	dslFrameBuffer * (SM_DECL *__DslFrameGetFirstBuffer) (dslFrame *pFrame);
	dslFrameBuffer * (SM_DECL *__DslFrameGetNextBuffer) (dslFrameBuffer *pFrBuffer);
	void  (SM_DECL *__DslFrameSetNextBuffer) (dslFrameBuffer *pFrBuf, dslFrameBuffer *pFrBufNext);
	dslFrameBuffer * (SM_DECL *__DslFrameGetLastBuffer) (dslFrame *pFrame);
	void * (SM_DECL *__DslFrameGetLinkFieldAddress) (dslFrame *f);
	dslFrame * (SM_DECL *__DslFrameGetFrameAddressFromLink) (void *lnk);

	Boolean (SM_DECL *__DslFrameGetOobInfo) (dslFrame *f, dslOobFrameInfo	*pOobInfo);
	Boolean (SM_DECL *__DslFrameSetOobInfo) (dslFrame *f, dslOobFrameInfo	*pOobInfo);

	void (SM_DECL *__DslFrameEnqueBufferAtBack) (dslFrame *f, dslFrameBuffer *b);
	void (SM_DECL *__DslFrameEnqueFrameAtBack) (dslFrame *fMain, dslFrame *f);
	void (SM_DECL *__DslFrameEnqueBufferAtFront) (dslFrame *f, dslFrameBuffer *b);
	void (SM_DECL *__DslFrameEnqueFrameAtFront) (dslFrame *fMain, dslFrame *f);
	dslFrameBuffer * (SM_DECL *__DslFrameDequeBuffer) (dslFrame *pFrame);

	void * (SM_DECL *__DslFrameAllocMemForFrames) (uint frameNum);
	void (SM_DECL *__DslFrameFreeMemForFrames) (void *hMem);
	dslFrame * (SM_DECL *__DslFrameAllocFrame) (void *handle);
	void (SM_DECL *__DslFrameFreeFrame) (void *handle, dslFrame *pFrame);
	void * (SM_DECL *__DslFrameAllocMemForBuffers) (void **ppMemPool, uint bufNum, uint memSize);
	void (SM_DECL *__DslFrameFreeMemForBuffers) (void *hMem, uint memSize, void *pMemPool);
	dslFrameBuffer * (SM_DECL *__DslFrameAllocBuffer) (void *handle, void *pMem, uint length);
	void (SM_DECL *__DslFrameFreeBuffer) (void *handle, dslFrameBuffer *pBuf);

	/* for LOG file support */

	uint (SM_DECL *__DslFrame2Id)(void *handle, dslFrame *pFrame);
	void * (SM_DECL *__DslFrameId2Frame)(void *handle, uint frameId);
	} dslFrameFunctions;

#define	 DslFrameDeclareFunctions( name_prefix )								\
extern uint SM_DECL name_prefix##BufferGetLength(dslFrameBuffer *fb);			\
extern void * SM_DECL name_prefix##BufferGetAddress(dslFrameBuffer *fb);		\
extern void SM_DECL name_prefix##BufferSetLength(dslFrameBuffer *fb, uint l);	\
extern void SM_DECL name_prefix##BufferSetAddress(dslFrameBuffer *fb, void *p); \
																				\
extern void SM_DECL name_prefix##Init(dslFrame *f);								\
extern uint SM_DECL name_prefix##GetLength (dslFrame *pFrame);					\
extern uint SM_DECL name_prefix##GetBufCnt(dslFrame *pFrame);					\
extern dslFrameBuffer * SM_DECL name_prefix##GetFirstBuffer(dslFrame *pFrame);	\
extern dslFrameBuffer * SM_DECL name_prefix##GetNextBuffer(dslFrameBuffer *pFrBuffer);	\
extern void SM_DECL name_prefix##SetNextBuffer(dslFrameBuffer *pFrBuf, dslFrameBuffer *pFrBufNext);	\
extern dslFrameBuffer * SM_DECL name_prefix##GetLastBuffer(dslFrame *pFrame);			\
extern void * SM_DECL name_prefix##GetLinkFieldAddress(dslFrame *f);					\
extern Boolean SM_DECL name_prefix##GetOobInfo(dslFrame *f, dslOobFrameInfo *pOobInfo);	\
extern Boolean SM_DECL name_prefix##SetOobInfo(dslFrame *f, dslOobFrameInfo *pOobInfo);	\
extern dslFrame* SM_DECL name_prefix##GetFrameAddressFromLink(void *lnk);				\
extern void SM_DECL name_prefix##EnqueBufferAtBack(dslFrame *f, dslFrameBuffer *b);		\
extern void SM_DECL name_prefix##EnqueFrameAtBack(dslFrame *fMain, dslFrame *f);		\
extern void SM_DECL name_prefix##EnqueBufferAtFront(dslFrame *f, dslFrameBuffer *b);	\
extern void SM_DECL name_prefix##EnqueFrameAtFront(dslFrame *fMain, dslFrame *f);		\
extern dslFrameBuffer * SM_DECL name_prefix##DequeBuffer(dslFrame *pFrame);				\
																						\
extern void * SM_DECL name_prefix##AllocMemForFrames(uint frameNum);					\
extern void SM_DECL name_prefix##FreeMemForFrames(void *hMem);							\
extern dslFrame * SM_DECL name_prefix##AllocFrame(void *handle);						\
extern void SM_DECL name_prefix##FreeFrame(void *handle, dslFrame *pFrame);				\
extern void * SM_DECL name_prefix##AllocMemForBuffers(void **ppMemPool, uint bufNum, uint memSize);	\
extern void SM_DECL name_prefix##FreeMemForBuffers(void *hMem, uint memSize, void *pMemPool);			\
extern dslFrameBuffer * SM_DECL name_prefix##AllocBuffer(void *handle, void *pMem, uint length);		\
extern void SM_DECL name_prefix##FreeBuffer(void *handle, dslFrameBuffer *pBuf);		\
extern uint SM_DECL name_prefix##2Id(void *handle, dslFrame *pFrame);					\
extern void * SM_DECL name_prefix##Id2Frame(void *handle, uint frameId);


#define	 DslFrameAssignFunctions( var, name_prefix )	do {			\
	(var).__DslFrameBufferGetLength	= name_prefix##BufferGetLength;		\
	(var).__DslFrameBufferGetAddress= name_prefix##BufferGetAddress;	\
	(var).__DslFrameBufferSetLength	= name_prefix##BufferSetLength;		\
	(var).__DslFrameBufferSetAddress= name_prefix##BufferSetAddress;	\
																		\
	(var).__DslFrameInit			= name_prefix##Init;				\
	(var).__DslFrameGetLength		= name_prefix##GetLength;			\
	(var).__DslFrameGetBufCnt		= name_prefix##GetBufCnt;			\
	(var).__DslFrameGetFirstBuffer	= name_prefix##GetFirstBuffer;		\
	(var).__DslFrameGetNextBuffer	= name_prefix##GetNextBuffer;		\
	(var).__DslFrameSetNextBuffer	= name_prefix##SetNextBuffer;		\
	(var).__DslFrameGetLastBuffer	= name_prefix##GetLastBuffer;		\
	(var).__DslFrameGetLinkFieldAddress		= name_prefix##GetLinkFieldAddress;		\
	(var).__DslFrameGetFrameAddressFromLink	= name_prefix##GetFrameAddressFromLink; \
																		\
	(var).__DslFrameGetOobInfo		= name_prefix##GetOobInfo;			\
	(var).__DslFrameSetOobInfo		= name_prefix##SetOobInfo;			\
																		\
	(var).__DslFrameEnqueBufferAtBack	= name_prefix##EnqueBufferAtBack;	\
	(var).__DslFrameEnqueFrameAtBack	= name_prefix##EnqueFrameAtBack;	\
	(var).__DslFrameEnqueBufferAtFront= name_prefix##EnqueBufferAtFront;	\
	(var).__DslFrameEnqueFrameAtFront	= name_prefix##EnqueFrameAtFront;	\
	(var).__DslFrameDequeBuffer		= name_prefix##DequeBuffer;			\
																		\
	(var).__DslFrameAllocMemForFrames	= name_prefix##AllocMemForFrames;	\
	(var).__DslFrameFreeMemForFrames	= name_prefix##FreeMemForFrames;	\
	(var).__DslFrameAllocFrame			= name_prefix##AllocFrame;			\
	(var).__DslFrameFreeFrame			= name_prefix##FreeFrame;			\
	(var).__DslFrameAllocMemForBuffers= name_prefix##AllocMemForBuffers;	\
	(var).__DslFrameFreeMemForBuffers = name_prefix##FreeMemForBuffers;	\
	(var).__DslFrameAllocBuffer		= name_prefix##AllocBuffer;			\
	(var).__DslFrameFreeBuffer		= name_prefix##FreeBuffer;			\
																		\
	(var).__DslFrame2Id				= name_prefix##2Id;					\
	(var).__DslFrameId2Frame		= name_prefix##Id2Frame;			\
} while (0)

typedef	struct
	{
	Boolean		febe_I;
	Boolean		fecc_I;
	Boolean		los, rdi;
	Boolean		ncd_I;
	Boolean		hec_I;
#ifdef G992P3
	Boolean		lpr;
#endif

#ifdef	G992P1_NEWFRAME

	Boolean		febe_F;
	Boolean		fecc_F;
	Boolean		ncd_F;
	Boolean		hec_F;

#endif
	} G992MonitorParams;

typedef struct
	{
	ushort	K;
	uchar	S, R;
	uchar   D;
#ifdef G992P3
	uchar	T, SEQ;
#endif
	directionType	direction;

#ifdef		G992P1_NEWFRAME

	ushort	N;
	ushort	NF;
	uchar	RSF;

	uchar	AS0BF, AS1BF, AS2BF, AS3BF, AEXAF;
	ushort	AS0BI;
	uchar	AS1BI, AS2BI, AS3BI, AEXAI;

	uchar	LS0CF, LS1BF, LS2BF, LEXLF;
	uchar	LS0CI, LS1BI, LS2BI, LEXLI;

	uchar	mergedModeEnabled;

#endif

	} G992CodingParams;

typedef struct
	{
	uchar	Nlp;
	uchar	Nbc;
	uchar	MSGlp;
	ushort	MSGc;

	uint	L;
	ushort	M;
	ushort	T;
	ushort	D;
	ushort	R;
	ushort	B;
	} G992p3CodingParams;

#ifdef CLEAN_FRAME_PARAM_EXCHANGE
/* codingType assumes the following conventions: */
#define UNCODED 0
#define TRELLIS 1
#define RS      2
#define CONCAT  3
#endif

/* This VDSL2 structure for use with both ADSL2 and VDSL2 starting with GINP support */

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

#if defined(WINNT) || defined(LINUX_DRIVER) || defined(__KERNEL__)
#define DIAG_OR_HOST_BUILD
#endif

typedef struct UDenNum16 UDenNum16;
struct UDenNum16
{
  unsigned short num; /*numerator*/
  unsigned short denom; /*denominator*/
};
typedef struct FramerDeframerOptions FramerDeframerOptions;
struct FramerDeframerOptions
{
  UDenNum16             S;          /* S== number of PMD symbols over which the FEC data frame spans (=1 for G.dmt fast path, <=1 for ADSL2 fast path) */
  unsigned short        D;          /* interleaving depth: =1 for fast path */
  unsigned short        N;          /* RS codeword size*/
  unsigned short        L16;
  union {
    struct {
      unsigned short        B;          /* nominal total of each bearer's octets per Mux Data Frame (Slightly redundant)*/
      unsigned short        U;          /* VDSL2: Number of OH sub-frames per OH frame */
      unsigned char         I;          /* VDSL2: Interleaver block length */
    } ALIGN_PACKED;
    struct { /* Gfast */
      unsigned short        Lrmc;       /* RMC bits in RC symbol */
      unsigned short        Ldoi;       /* L in DOI; Bdoi = floor(Ldoi/8) */
      unsigned char         Rrmc;       /* R in RMC symbol */
    } ALIGN_PACKED;
  };
  unsigned char         R;          /* RS codeword overhead bytes */
  unsigned char         M;          /* ADSL2 only. Number of Mux frames per FEC Data frame.*/
  union {
    struct {
      unsigned char         T;          /* ADSL2: Number of Mux frames per sync octet*/
                                        /* VDSL2: Number of Mux data frames in an OH sub-frame */
      unsigned char         G;          /* VDSL2: Notional number of OH bytes in an OH sub-frame - actual number of bytes in any sub-frame may 1 be greater than this */
      unsigned char         F;          /* VDSL2: Number of OH frames in an OH superframe */
    } ALIGN_PACKED;
    struct { /* Gfast */
      unsigned char         Mf;         /* common for DS and US */
      unsigned char         Msf;        /* common for DS and US */
      unsigned char         Drmc;       /* RMC symbol offset  */
    } ALIGN_PACKED;
  };
  unsigned char         codingType; /* codingType associated with this option */
  union {
    struct {
      unsigned char         fireEnabled;/* bitmap flagging fire support for this direction and/or for reverse direction
                                     * No more used - Kept in the structure for backward compatibility with Diags */
      unsigned char         fireRxQueueOld;/* length of the retransmission queue in Rx direction */
    } ALIGN_PACKED;
    struct { /* Gfast */
      unsigned char         MNDSNOI;    /* Min Data Symbols in NOI */
      unsigned char         ackWindowShift;
    } ALIGN_PACKED;
  };
#ifdef _MSC_VER
#pragma pack(pop)
#endif
  unsigned char         tpsTcOptions; /* result of the pmsTcNegotiation */
  unsigned char         delay;      /* actual delay incurred on this latency path in [ms] */
  unsigned char         INP;        /* actual INP guaranteed on this latency path in [symbol/2] */
  unsigned char         b1;
  unsigned char         ovhType;
  unsigned char         path;
  unsigned char         ahifChanId[2];
  unsigned char         tmType[2];
  unsigned char         fireTxQueue;/* length of the retransmission queue in Tx direction */
  unsigned char         phyRrrcBits;/* number of bits in the retransmission return channel */
  unsigned char         ginpFraming;/* 0 if G.inp is not active, 1 or 2 (framingType) if active
                                     * W = ginpFraming-1 */
  unsigned char         INPrein;    /* actual INP guaranteed on this latency path against rein noise in [symbol/2] */
  unsigned char         Q;          /* G.Inp: Number of RS CW per DTU (PhyR & G.Inp) */
  unsigned char         V;          /* G.Inp: Number of padding bytes per DTU */
  unsigned char         QrxBuffer;  /* G.Inp: Number of DTUs of the rx queue effectively bufferized */
  unsigned int          ETR_kbps;   /* G.Inp: ETR - Expected throughput in kbits/sec */
  unsigned short        INPshine;   /* G.Inp: For G.Inp the INP is 2 bytes and supports values up to 204.7 0.1 symbols which won't fit in existing INP. We use Q1 format
                                       to be the same as the existing INP and the driver will limit to 204.7 when reporting to the CO */
  unsigned int          L;
#if defined(GFAST_SUPPORT) || defined(DIAG_OR_HOST_BUILD)
  unsigned int          maxMemory;  /* Maximum available nominal memory at NE for the current lp */
  unsigned int          ndr;        /* Net Data Rate */
  unsigned short        Ldr;        /* Number of bearer bits per symbol during RMC symbol */
  unsigned char         Nret;       /* Maximum number of retransmission - fireRxQueue = Nret * fireTxQueue */
  unsigned int          etru;
  unsigned int          Lmax;       /* Maximum possible L (SRA upshift) */
  unsigned int          Lmin;       /* Minimum possible L (SRA downshift) */
  unsigned int          ETRminEoc;  /* ETR_min_eoc, see table 9-21 in corrigendum 1 */
#if defined(GFAST_DTA_SUPPORTED) || defined(DIAG_OR_HOST_BUILD)
  unsigned char         dtaMmax;    /* Mds Max when dta is enabled */
#endif
#endif
  unsigned short        fireRxQueue;/* length of the retransmission queue in Rx direction */
};

#define ETR_MIN_EOC_FRAMER_STRUCT_SIZE  (sizeof(FramerDeframerOptions) - 3)
#define RX_QUEUE16_FRAMER_STRUCT_SIZE   sizeof(FramerDeframerOptions)


/* Negotiated DTA parameters */
typedef struct GfastDtaOptions {
  unsigned int			ndrDS;       /* Mds == CurrentMds */
  unsigned int			ndrUS;       /* Mus == CurrentMus */
  unsigned char			dtaFlags;
  unsigned char			currentMds;
  unsigned char			maxMds;
  unsigned char			hsMds;
} GfastDtaOptions;

/* Power Management Message definitions (used in command and status) */

typedef struct
	{
	int			msgType;
	union
		{
		int		value;
		struct
			{
			int	msgLen;
			void	*msgData;
			} msg;
		} param;
	} dslPwrMessage;

/* Power Management commands and responses */

#define	kPwrSimpleRequest							1
#define	kPwrL2Request								2
#define	kPwrL2TrimRequest							3

#define	kPwrGrant									0x80
#define	kPwrReject									0x81
#define	kPwrL2Grant									0x82
#define	kPwrL2Reject								0x83
#define	kPwrL2TrimGrant								0x84
#define	kPwrL2TrimReject							0x85
#define	kPwrL2Grant2p								0x86

#define	kPwrBusy									0x01
#define	kPwrInvalid									0x02
#define	kPwrNotDesired								0x03
#define	kPwrInfeasibleParam							0x04

/* Power Management reason codes */

/* OLR definitions (used in command and status) */

typedef struct
	{
	ushort	msgType;
	ushort	nCarrs;
	ushort	L[4];
	uchar	B[4];
	void	*carrParamPtr;
	} dslOLRMessage;

typedef struct
	{
	uchar	ind;
	uchar	gain;
	uchar	gb;
	} dslOLRCarrParam;

typedef struct
	{
	ushort	ind;
	uchar	gain;
	uchar	gb;
	} dslOLRCarrParam2p;

/* OLR messages */
// #define	kOLRRequestType1					        1     /*G992.3 Bitswap */
// #define	kOLRRequestType2							2     /* G992.3 DRR */
// #define	kOLRRequestType3							3     /*G992.3 SRA */
// #define	kOLRRequestType4							4     /*G992.5 or G993.2 BITSWAP */
// #define	kOLRRequestType5							5     /*G992.5 DRR or G992.[35] GINP_SRA */
// #define	kOLRRequestType6							6     /*G992.5 or G993.2 SRA */
// #define  kOLRRequestType7                           7     /*G993.2 SOS */
// #define  kOLRRequestType8                           8     /* G993.2 GINP_SRA */
// #define  kOLRRequestType9                           9     /* G993.2 GINP_SOS */

#define	kOLRRequestType1							1
#define	kOLRRequestType2							2
#define	kOLRRequestType3							3
#define	kOLRRequestType4							4
#define	kOLRRequestType5							5
#define	kOLRRequestType6							6
#define kOLRRequestType7                           7
#define kOLRRequestType8                           8
#define kOLRRequestType9                           9

/* internal definitions */
#define kOLRRequestType6_VDSL_SRA                  0x60

#define	kOLRDeferType1								0x81
#define	kOLRRejectType2								0x82
#define	kOLRRejectType3								0x83
#define	kOLRRejectType3GINP							0x84
#define	kOLRRejectType5						        0x85
#define	kOLRRejectType6						        0x86

/* OLR reason codes */

#define	kOLRBusy									1
#define	kOLRInvalidParam							2
#define	kOLRNotEnabled								3
#define	kOLRNotSupported							4

/* common EOC definitions  */
#define	kG992EocStuffingByte						0x0C

#if defined(GFAST_SUPPORT) || defined(CONFIG_BCM_DSL_GFAST)
/* OLR counter type parameters,  to be used with kG992AocBitswapXXX
 * for BSW/SRA/TIGA the parameter id matches OlrSubType */
#define       kOlrBSW                                  0
#define       kOlrSRA                                  1
#define       kOlrTIGA                                 2
#define       kOlrRPA                                  3
#define       kOlrFRA                                  4

#define       kOlrXOIMask                              0x30
#define       kOlrNoi                                  0                 // compatible with existing coding
#define       kOlrDoi                                  0x10              // DOI only 
#define       kOlrNoiDoi                               0x30              // NOI + DOI 

#endif

/* showtime monitor counters */
#define	kG992ShowtimeRSCodewordsRcved				0	/* number of Reed-Solomon codewords received */
#define kG992ShowtimeRSCodewordsRcvedOK				1	/* number of Reed-Solomon codewords received with all symdromes zero */
#define	kG992ShowtimeRSCodewordsRcvedCorrectable	2	/* number of Reed-Solomon codewords received with correctable errors */
#define	kG992ShowtimeRSCodewordsRcvedUncorrectable	3	/* number of Reed-Solomon codewords received with un-correctable errors */
#define	kG992ShowtimeSuperFramesRcvd				4	/* number of super frames received */
#define	kG992ShowtimeSuperFramesRcvdWrong			5	/* number of super frames received with CRC error */
#define	kG992ShowtimeLastUncorrectableRSCount		6	/* last recorded value for kG992ShowtimeRSCodewordsRcvedUncorrectable */
#define	kG992ShowtimeLastWrongSuperFrameCount		7	/* last recorded value for kG992ShowtimeSuperFramesRcvdWrong */
#define	kG992ShowtimeNumOfShortResync				8	/* number of short interrupt recoveries by FEQ */

#define	kG992ShowtimeNumOfFEBE						9	/* number of other side superframe errors */
#define	kG992ShowtimeNumOfFECC						10	/* number of other side superframe FEC errors */
#define	kG992ShowtimeNumOfFHEC						11	/* number of far-end ATM header CRC errors */
#define	kG992ShowtimeNumOfFOCD						12	/* number of far-end OCD events */
#define	kG992ShowtimeNumOfFLCD						13	/* number of far-end LCD events */
#define	kG992ShowtimeNumOfHEC						14	/* number of ATM header CRC errors */
#define	kG992ShowtimeNumOfOCD						15	/* number of OCD events */
#define	kG992ShowtimeNumOfLCD						16	/* number of LCD events */

#define	kG992ShowtimeNumOfMonitorCounters			(kG992ShowtimeNumOfLCD+1)	/* always last number + 1 */
#define	kG992ShowtimeMonitorReportNumber			9

#define	kG992ShowtimeLCDNumShift					1
#define	kG992ShowtimeLCDFlag						1

/* Fire monitor counters */
#define kFireReXmtRSCodewordsRcved                      0
#define kFireReXmtUncorrectedRSCodewords                1
#define kFireReXmtCorrectedRSCodewords                  2
#define kFireGoodOutputRSCodewords                      3               /* Reset to 0 at each second */
#define kFireReXmtPrev17msUncRSCodewords                4               /* Copy of kFireReXmtUncorrectedRSCodewords at each 17ms interval */
#define kFireReXmtUncorrectedRSCodewordsOneSec          5               /* Copy of kFireReXmtUncorrectedRSCodewords at each one sec interval */
#define kFireReXmtCorrectedRSCodewordsOneSec            6               /* Copy of kFireReXmtCorrectedRSCodewords   at each one sec interval */
#define kFireGoodOutputRSCodewordsOneSec                7               /* Copy of kFireGoodOutputRSCodewords       at each one sec interval */
#define kFireFrameCntOneSec                             8               /* Copy of rxTimer                          at each one sec interval */

#ifdef GINP_SUPPORT
#define kFireNumOfCounters                              9
#else
#define kFireNumOfCounters                              3
#endif
#define kFireNumOfReportCounters                        3

/* Fire status bitmap */
#define kFireDsEnabled                                  0x1
#define kFireUsEnabled                                  0x2
#define kGinpDsEnabled                                  0x4
#define kGinpUsEnabled                                  0x8
#define kArqDs                                          (kFireDsEnabled|kGinpDsEnabled)
#define kArqUs                                          (kFireUsEnabled|kGinpUsEnabled)

#define PHYR_DS_DIRECTION                               kFireDsEnabled
#define PHYR_US_DIRECTION                               kFireUsEnabled
#define GINP_DS_DIRECTION                               kGinpDsEnabled
#define GINP_US_DIRECTION                               kGinpUsEnabled
#define ARQ_DS_DIRECTION                                kArqDs
#define ARQ_US_DIRECTION                                kArqUs

/* Rate Adaptation Reporting status bitmap - reported using kG992BitswapState */
#define RX_BITSWAP_STATUS         0x0001
#define TX_BITSWAP_STATUS         0x0002
#define RX_SRA_STATUS             0x0004
#define TX_SRA_STATUS             0x0008
#define RX_SOS_STATUS             0x0010
#define TX_SOS_STATUS             0x0020
#define RX_ROC_STATUS             0x0040
#define TX_ROC_STATUS             0x0080
#define RX_FRA_STATUS             0x0100
#define TX_FRA_STATUS             0x0200
#define RX_RPA_STATUS             0x0400
#define TX_RPA_STATUS             0x0800
#define RX_TIGA_STATUS            0x1000
#define TX_TIGA_STATUS            0x2000

/* Bonding statuses*/
#define kDslBondingATM                              0x4
#define kDslBondingPTM                              0x1

/*  line-drop reason code */
#define kRetrainReasonLosDetector                   0
#define kRetrainReasonRdiDetector                   1
#define kRetrainReasonNegativeMargin                2
#define kRetrainReasonTooManyUsFEC                  3
#define kRetrainReasonCReverb1Misdetection          4
#define kRetrainReasonTeqDsp                        5
#define kRetrainReasonAnsiTonePowerChange           6
#define kRetrainReasonIfftSizeChange                7
#define kRetrainReasonRackChange                    8
#define kRetrainReasonVendorIdSync                  9
#define kRetrainReasonTargetMarginSync             10
#define kRetrainReasonToneOrderingException        11
#define kRetrainReasonCommandHandler               12
#define kRetrainReasonDslStartPhysicalLayerCmd     13
#define kRetrainReasonUnknown                      14
#define kRetrainReasonTrainingFailure              15   /* Renamed from kRetrainReasonG992Failure, if we cannot classify the failure in any other code then we use this code, can be used for HS/ADSL/VDSL training Failures*/
#define kRetrainReasonSes                          16
#define kRetrainReasonCoMinMargin                  17
#define kRetrainReasonANFPMaskSelect               18
#define kRetrainReasonDisableAnnexL                19

/* New added codes can be used for VDSL/ADSL/HS */
#define kRetrainReasonConfigError                   20      /* This can represent general config error, LINE_FORCED_RTX_US, LINE_FORCED_RTX_DS or any other configuration mode */
#define kRetrainReasonTimeout                       21      /* All timeout errors */
#define kRetrainReasonNoCommonOPMode                22      /* Could be used during HS if no common operational mode found */
#define kRetrainReasonNoATUC                        23      /* No ATU-C detection during HS */
#define kRetrainReasonNoCommonTPSTC                 24      /* VDSL mode only If we do not find the common TPSTC mode. */
#define kRetrainReasonShowtimeFailure               25      /* Showtime failure if we cannot classify showtime retrain reason into any defined ones */

/* New reason code for Gfast */
#define kRetrainReasonLowETR                        26


struct _dslFramerBufDesc;

#ifndef DSLVARS_GLOBAL_REG
	typedef	int		(SM_DECL *dslFrameHandlerType)	(void *gDslVars, void *pVc, ulong mid, dslFrame *);

	typedef	void*   (SM_DECL *dslHeaderHandlerType) (void *gDslVars, uint hdr, uchar hdrHec);
	typedef	void*	(SM_DECL *dslTxFrameBufferHandlerType)	(void *gDslVars, int*,	void*);
	typedef	void*	(SM_DECL *dslRxFrameBufferHandlerType)	(void *gDslVars, int,	void*);

	typedef	void*	(SM_DECL *dslVcAllocateHandlerType)	(void *gDslVars, void *);
	typedef	void	(SM_DECL *dslVcFreeHandlerType)	(void *gDslVars, void *);
	typedef	Boolean	(SM_DECL *dslVcActivateHandlerType)	(void *gDslVars, void *);
	typedef	void	(SM_DECL *dslVcDeactivateHandlerType) (void *gDslVars, void *);
	typedef	Boolean	(SM_DECL *dslVcConfigureHandlerType) (void *gDslVars, void *pVc, ulong mid, void *);

	typedef	uint	(SM_DECL *dslLinkVc2IdHandlerType) (void *gDslVars, void *);
	typedef	void*	(SM_DECL *dslLinkVcId2VcHandlerType) (void *gDslVars, uint);
	typedef void*	(SM_DECL *dslGetFramePoolHandlerType) (void *gDslVars);

	typedef	void	(SM_DECL *dslLinkCloseHandlerType) (void *gDslVars);
	typedef	int		(SM_DECL *dslTxDataHandlerType)(void *gDslVars, int, int, uchar*, G992MonitorParams*);
	typedef	int		(SM_DECL *dslRxDataHandlerType)(void *gDslVars, int, uchar*, G992MonitorParams*);

	typedef	void	(SM_DECL *dslLinkStatusHandler) (void *gDslVars, uint statusCode, ...);

	typedef Boolean (SM_DECL *dslPhyInitType) (	void	*gDslVars,
		bitMap						setupMap,
		dslHeaderHandlerType		rxCellHeaderHandlerPtr,
		dslRxFrameBufferHandlerType	rxFrameHandlerPtr,
		dslTxFrameBufferHandlerType txFrameHandlerPtr,
		atmStatusHandler			statusHandlerPtr);

	typedef	Boolean (SM_DECL *dslFramerDataGetPtrHandlerType) (void *gDslVars, struct _dslFramerBufDesc *pBufDesc);
	typedef	void	(SM_DECL *dslFramerDataDoneHandlerType) (void *gDslVars, struct _dslFramerBufDesc *pBufDesc);

	typedef	void	(SM_DECL *dslDriverCallbackType) (void *gDslVars);


#else
	typedef	int		(SM_DECL *dslFrameHandlerType)	(void *pVc, ulong mid, dslFrame *);

	typedef	void*   (SM_DECL *dslHeaderHandlerType) (uint hdr, uchar hdrHec);
	typedef	void*	(SM_DECL *dslTxFrameBufferHandlerType)	(int*,	void*);
	typedef	void*	(SM_DECL *dslRxFrameBufferHandlerType)	(int,	void*);

	typedef	void*	(SM_DECL *dslVcAllocateHandlerType)	(void *);
	typedef	void	(SM_DECL *dslVcFreeHandlerType)	(void *);
	typedef	Boolean	(SM_DECL *dslVcActivateHandlerType)	(void *);
	typedef	void	(SM_DECL *dslVcDeactivateHandlerType) (void *);
	typedef	Boolean	(SM_DECL *dslVcConfigureHandlerType) (void *pVc, ulong mid, void *);

	typedef	uint	(SM_DECL *dslLinkVc2IdHandlerType) (void *);
	typedef	void*	(SM_DECL *dslLinkVcId2VcHandlerType) (uint);
	typedef void*	(SM_DECL *dslGetFramePoolHandlerType) (void);

	typedef	void	(SM_DECL *dslLinkCloseHandlerType) (void);
	typedef	int		(SM_DECL *dslTxDataHandlerType)(int, int, uchar*, G992MonitorParams*);
	typedef	int		(SM_DECL *dslRxDataHandlerType)(int, uchar*, G992MonitorParams*);

	typedef	void	(SM_DECL *dslLinkStatusHandler) (uint statusCode, ...);

	typedef Boolean (SM_DECL *dslPhyInitType) (
		bitMap						setupMap,
		dslHeaderHandlerType		rxCellHeaderHandlerPtr,
		dslRxFrameBufferHandlerType	rxFrameHandlerPtr,
		dslTxFrameBufferHandlerType txFrameHandlerPtr,
		atmStatusHandler			statusHandlerPtr);

	typedef	Boolean (SM_DECL *dslFramerDataGetPtrHandlerType) (struct _dslFramerBufDesc *pBufDesc);
	typedef	void	(SM_DECL *dslFramerDataDoneHandlerType) (struct _dslFramerBufDesc *pBufDesc);

	typedef	void	(SM_DECL *dslDriverCallbackType) (void);
#endif /* DSLVARS_GLOBAL_REG */


#ifdef DSL_PACKET

typedef	struct
	{
	dslFramerDataGetPtrHandlerType	rxDataGetPtrHandler;
	dslFramerDataDoneHandlerType	rxDataDoneHandler;
	dslFramerDataGetPtrHandlerType	txDataGetPtrHandler;
	dslFramerDataDoneHandlerType	txDataDoneHandler;
	} dslPacketPhyFunctions;

#ifndef DSLVARS_GLOBAL_REG
typedef Boolean (SM_DECL *dslPacketPhyInitType) (void	*gDslVars,
	bitMap						setupMap,
	dslPacketPhyFunctions		dslPhyFunctions,
	dslLinkStatusHandler		statusHandlerPtr);
#else
typedef Boolean (SM_DECL *dslPacketPhyInitType) (
	bitMap						setupMap,
	dslPacketPhyFunctions		dslPhyFunctions,
	dslLinkStatusHandler		statusHandlerPtr);
#endif /* DSLVARS_GLOBAL_REG */

#endif /* DSL_PACKET */


typedef	int			dslDirectionType;
typedef	bitMap		dslModulationType;
typedef	bitMap		dslLinkLayerType;

/*
**
**		Log data codes
**
*/

#define	kDslEyeData					eyeData

#define	kDslLogComplete				(inputSignalData - 1)
#define	kDslLogInputData			inputSignalData
#define	kDslLogInputData1			(inputSignalData + 1)
#define	kDslLogInputData2			(inputSignalData + 2)
#define	kDslLogInputData3			(inputSignalData + 3)
#define	kDslLogInputDataFirst		(inputSignalData + 4)
#define	kDslLogInputDataNext		(inputSignalData + 5)
#define	kDslLogInputDataLast		(inputSignalData + 6)
#define	kDslDebugData				(inputSignalData + 8)
#define	kDslDebugDataValueMask		0x3
#define	kDslDebugDataAlignShift		2
#define	kDslDebugDataAlignMask		(0x3 << kDslDebugDataAlignShift)
#define	kDslDebugDataMask			(kDslDebugDataValueMask | kDslDebugDataAlignMask)

/*
**
**		Status codes
**
*/

typedef int						dslStatusCode;
#define	kFirstDslStatusCode			256
#define	kDslError					(kFirstDslStatusCode + 0)
#define	kAtmStatus					(kFirstDslStatusCode + 1)
#define	kDslTrainingStatus			(kFirstDslStatusCode + 2)
#define	kDslConnectInfoStatus		(kFirstDslStatusCode + 3)
#define	kDslEscapeToG994p1Status	(kFirstDslStatusCode + 4)
#define	kDslFrameStatus				(kFirstDslStatusCode + 5)
#define kDslReceivedEocCommand		(kFirstDslStatusCode + 6)
#define kDslSendEocCommandDone		(kFirstDslStatusCode + 7)
#define kDslSendEocCommandFailed	(kFirstDslStatusCode + 8)
#define kDslWriteRemoteRegisterDone	(kFirstDslStatusCode + 9)
#define kDslReadRemoteRegisterDone	(kFirstDslStatusCode + 10)
#define	kDslExternalError			(kFirstDslStatusCode + 11)
#define kDslDspControlStatus		(kFirstDslStatusCode + 12)
#define kDslATUHardwareAGCRequest	(kFirstDslStatusCode + 13)
#define	kDslPacketStatus			(kFirstDslStatusCode + 14)
#define	kDslG997Status				(kFirstDslStatusCode + 15)
#define	kDslPrintfStatus			(kFirstDslStatusCode + 16)
#define	kDslPrintfStatus1			(kFirstDslStatusCode + 17)
#define	kDslExceptionStatus			(kFirstDslStatusCode + 18)
#define	kDslPingResponse			(kFirstDslStatusCode + 19)
#define	kDslShowtimeSNRMarginInfo	(kFirstDslStatusCode + 20)
#define	kDslGetOemParameter			(kFirstDslStatusCode + 21)
#define	kDslOemDataAddrStatus		(kFirstDslStatusCode + 22)
#define	kDslDataAvailStatus			(kFirstDslStatusCode + 23)
/* #define kDslAtuChangeTxFilterRequest (kFirstDslStatusCode + 24) */
#define kDslTestPllPhaseResult      (kFirstDslStatusCode + 25)
#ifdef BCM6348_SRC
#define kDslHardwareAGCSetPga1      (kFirstDslStatusCode + 26)
#define kDslHardwareAGCDecPga1      (kFirstDslStatusCode + 27)
#define kDslHardwareAGCIncPga1      (kFirstDslStatusCode + 28)
#define kDslHardwareAGCSetPga2Delta (kFirstDslStatusCode + 29)
#endif
#define	kDslOLRRequestStatus		(kFirstDslStatusCode + 30)
#define	kDslOLRResponseStatus		(kFirstDslStatusCode + 31)
#define	kDslOLRBitGainUpdateStatus	(kFirstDslStatusCode + 32)
#define	kDslPwrMgrStatus		    (kFirstDslStatusCode + 33)
#define	kDslEscapeToT1p413Status	(kFirstDslStatusCode + 34)
#ifdef BCM6348_SRC
#define kDslHardwareAGCSetPga2      (kFirstDslStatusCode + 35)
#define kDslHardwareGetRcvAGC       (kFirstDslStatusCode + 36)
#endif
#define kDslUpdateXmtReadPtr        (kFirstDslStatusCode + 37)
#define kDslHardwareSetRcvAGC       (kFirstDslStatusCode + 38)
#define kDslSetDigUsPwrCutback      (kFirstDslStatusCode + 39)
#define kDslAfeTestStatus			(kFirstDslStatusCode + 40)
#define kDslAfeTestCaptureDone		0	/* param1 - RcvBuffPtr,  param2 - RcvLen */
#define kDslAfeTestRetBuffPoolSize	1	/* param1 - Total buffer pool size */
#define kDslAfeTestRetXmtBuffPtr	2	/* param1 - XmtBuffPtr */
#define kDslAfeTestRetRcvtBuffPtr	3	/* param1 - RcvBuffPtr */
#define kDslAfeTestRetXregVal		4	/* param1 - Xreg Addr, param2 - Xreg Value */
#define kDslAfeTestRetIregVal		5	/* param1 - Ireg Addr, param2 - Ireg Value */

#define	kClientSideInitiation		0
#define	kClientSideRespond			1
#define	kCentralSideInitiation		2
#define	kCentralSideRespond			3

/* OEM parameter ID definition */

#define	kDslOemG994VendorId			1
#define	kDslOemG994XmtNSInfo		2
#define	kDslOemG994RcvNSInfo		3
#define	kDslOemEocVendorId			4
#define	kDslOemEocVersion			5
#define	kDslOemEocSerNum			6
#define	kDslOemT1413VendorId		7
#define	kDslOemT1413EocVendorId		8

#define kDslResetPhyReqStatus		(kFirstDslStatusCode + 41)
#define kDslLineIdStatus			(kFirstDslStatusCode + 42)
#define kDslEpcAddrStatus			(kFirstDslStatusCode + 43)
#define kDslPrintfStatusSaveLocalOnly	(kFirstDslStatusCode + 44)

/* This status is used to send a message to the host indicating an action
 * to be performed.
 *
 * When this status is sent, a pointer to a 32-bit phy lmem location
 * is passed to the host. This memory location must be initialized as
 * follows before sending the status.
 *
 *  Bit 0        Completion flag, set to 0. Will be set to 1 by host
 *               once action has been completed.
 *  Bits [31:1]  Action to be performed + any parameters (if required).
 */
#define kDslRequestDrvConfig            (kFirstDslStatusCode + 45)
#define kDslDrvConfigSetLD6302Adsl      (0)
#define kDslDrvConfigSetLD6302Vdsl      (1)
#define kDslDrvConfigSet6306DIFClk64    (2)
#define kDslDrvConfigSet6306DIFClk100   (3)
#define kDslDrvConfigRegRead			(4)
#define kDslDrvConfigRegWrite			(5)
#define kDslDrvConfigAhifStatePtr		(6)
#define kDslDrvConfigReset6306		(7)
#define kDslDrvConfigExtLD6303	(8)
#define kDslDrvConfigLD6303V5p3         (9)
#define kDslDrvConfigRdAfePLLMdiv       (10)	/* regAddr - pllch01_cfg.Reg32, regValue - pllch45_cfg.Reg32 */
#define kDslDrvConfigWrAfePLLMdiv       (11)	/* regAddr - mdiv0, regValue - mdiv1 */
#define kDslDrvConfigAfeRelay           (12)	/* regAddr - xxxxx, regValue - kDslRelayModeVdsl/kDslRelayModeGfast */
/* AFE relay modes */
#define kDslRelayModeVdsl       0
#define kDslRelayModeGfast      1
#define kDslDrvConfigDisable6306RefClk  (13)
#define kDslDrvConfigLDPowerMode        (14)
/* AFE LD power modes */
#define kDslLDPowerMode12V       0	/* 12V */
#define kDslLDPowerMode15V       1	/* 15V */

#define kDslPwrMgrSrAddrStatus			(kFirstDslStatusCode + 46)

#define kDslStatusBufferChange		(kFirstDslStatusCode + 47)
#define kDslCommandBufferChange		(kFirstDslStatusCode + 48)

#define kDslResetOtherLine			(kFirstDslStatusCode + 49)

/* line reset modes */
#define	kDslInitDown			0
#define	kDslInitIdle			1
#define	kDslInitG994			2
#define	kDslInitAdsl			3
#define	kDslInitVdsl			4
#ifdef ENABLE_6306_SHUTDOWN
#define kDslShutdown6306		5
#endif
#define	kDslInitGfastRestartCmd	      6
#define	kDslInitGfastRestartCmdDone	  7
#define	kDslInitRepeatOnFail	8
#define	kDslInitVdslfromLR      9

#define kDslWakeupRequest			(kFirstDslStatusCode + 50)
#define kDslSetLineIdleStatus		(kFirstDslStatusCode + 51)
#define kDslG994p1InitCompletedStatus	(kFirstDslStatusCode + 52)
#define kDslTxPafActiveStatus		(kFirstDslStatusCode + 53)
#define kDslStatusSwitchToIdle	        (kFirstDslStatusCode + 54)
#define kDslTODactiveLine	        (kFirstDslStatusCode + 55)

#define kDslTODactive                   1
#define kDslTODinactive                 0

#define kDslNTRPlllockStatus            (kFirstDslStatusCode + 56)

#define kDslNTRlockOn                   1
#define kDslNTRlockOff                  0

#define	kDslFastRetrain				          (kFirstDslStatusCode + 57)
#define	kDslShowtimeDOISNRMarginInfo	  (kFirstDslStatusCode + 58)
#define kDslResetOtherLineSticky		(kFirstDslStatusCode + 59)
#define kDslStopDrvStatusSaving			(kFirstDslStatusCode + 60)
#define kDslGeneralKeyResponseResult	(kFirstDslStatusCode + 61)
#define kDslReportDGReadyForConfig      (kFirstDslStatusCode + 62)

/*
**  AHIF Driver synchronization modes
*/

#define kDslDrvAhifSyncXmt				-1
#define kDslDrvAhifSyncXmtRcv			-2

#define kDslDrvAhifSyncRspXmt			0
#define kDslDrvAhifSyncRspXmtRcv		1

#define kDslDrvAhifSyncModeNone			0
#define kDslDrvAhifSyncModeXmt			1
#define kDslDrvAhifSyncModeXmtRcv		2

#define kDslDrvAhifSyncValuePhy			0
#define kDslDrvAhifSyncValueDrv			1

typedef struct {
	uint	cmd;
	union {
		struct {
			uint	regAddr;
			uint	regValue;
		};
#ifdef _NOOS
		LONGLONG data;
#else
		struct {
			LONGLONG	data;
		};
#endif
	};
} drvRegControl;

typedef int	dslErrorCode;

typedef int	atmStatusCode;
typedef int	dslFramerStatusCode;

typedef int	atmErrorCode;

typedef int	dslTrainingStatusCode;

#define kDslStartedG994p1					0
#define kDslStartedT1p413HS					1

/* reserved for G.994.1: 1 ~ 8 */

#define	kDslG994p1MessageDet				100
#define	kDslG994p1ToneDet					101
#define	kDslG994p1RToneDet					102
#define	kDslG994p1FlagDet					103
#define	kDslG994p1GalfDet					104
#define	kDslG994p1ErrorFrameDet				105
#define	kDslG994p1BadFrameDet				106
#define	kDslG994p1SilenceDet				107
#define	kDslG994p1RcvTimeout				108
#define	kDslG994p1XmtFinished				109
#define	kDslG994p1ReturntoStartup			110
#define	kDslG994p1InitiateCleardown			111
#define	kDslG994p1StartupFinished			112
#define	kDslG994p1RcvNonStandardInfo		113
#define	kDslG994p1XmtNonStandardInfo		114

#define	kG994p1MaxNonstdMessageLength		64

#define kDslFinishedT1p413					1100
#define kDslT1p413DetectedCTone				1101
#define kDslT1p413DetectedCAct				1102
#define kDslT1p413DetectedCReveille			1103
#define kDslT1p413DetectedRActReq			1104
#define kDslT1p413DetectedRQuiet1			1105
#define kDslT1p413DetectedRAct				1106
#define kDslT1p413TimeoutCReveille			1107
#define	kDslT1p413ReturntoStartup			1108

#define	kDslG994p1Timeout					8
#define kDslFinishedG994p1					9
#define kDslStartedG992p2Training			10
#define	kDslG992p2DetectedPilotSymbol		11
#define	kDslG992p2DetectedReverbSymbol		12
#define	kDslG992p2TEQCalculationDone		13
#define	kDslG992p2TrainingFEQ				14
#define	kDslG992p2Phase3Started				15
#define	kDslG992p2ReceivedRates1			16
#define	kDslG992p2ReceivedMsg1				17
#define	kDslG992p2Phase4Started				18
#define	kDslG992p2ReceivedRatesRA			19
#define	kDslG992p2ReceivedMsgRA				20
#define	kDslG992p2ReceivedRates2			21
#define	kDslG992p2ReceivedMsg2				22
#define	kDslG992p2ReceivedBitGainTable		23
#define	kDslG992p2TxShowtimeActive			24
#define	kDslG992p2RxShowtimeActive			25
#define	kDslG992p2TxAocMessage				26
#define	kDslG992p2RxAocMessage				27
#define	kDslG992p2TxEocMessage				28
#define	kDslG992p2RxEocMessage				29
#define kDslFinishedG992p2Training			30
#define	kDslRecoveredFromImpulseNoise		31
#define	kDslG992Timeout						32
#define	kDslT1p413Isu1SglByteSymDetected	33	/* detected T1.413 Issue 1 single byte per symbol mode */
#define	kDslG992RxPrefixOnInAFewSymbols		34
#define	kDslG992TxPrefixOnInAFewSymbols		35
#define	kDslAnnexCXmtCPilot1Starting		36
#define	kDslXmtToRcvPathDelay				37
#define kDslFeaturesUnsupported				38
#define	kDslG992RcvMsgCrcError				39
#define	kDslAnnexCDetectedStartHyperframe	40

#define kDslG992AnnexCTimeoutCPilot1Detection	41
#define kDslG992AnnexCTimeoutCReverb1Detection	42
#define kDslG992AnnexCTimeoutECTraining			43
#define kDslG992AnnexCTimeoutHyperframeDetector	44
#define kDslG992AnnexCTimeoutSendRSegue2		45
#define kDslG992AnnexCTimeoutDetectCSegue1		46
#define kDslG992AnnexCAlignmentErrDetected		47
#define kDslG992AnnexCTimeoutSendRSegueRA		48
#define kDslG992AnnexCTimeoutSendRSegue4		49
#define kDslG992AnnexCTimeoutCSegue2Detection	50
#define kDslG992AnnexCTimeoutCSegue3Detection	51
/* Progress report for fast retrain */

#define	kG994p1EventToneDetected				54
#define	kDslG992p2RcvVerifiedBitAndGain         55
#define	kDslG992p2ProfileChannelResponseCalc    56
#define kDslG992AnnexCTotalFEXTBits				57
#define kDslG992AnnexCTotalNEXTBits				58
#define kDslG992AnnexCTotalFEXTCarrs			59
#define kDslG992AnnexCTotalNEXTCarrs			60

#define	kDslG992p3ReceivedMsgFmt				61
#define	kDslG992p3ReceivedMsgPcb				62

#define	kDslG992p3AnnexLMode					63

#define kDslG992p2ReceivedMsgLD					64

#define kDslG992RxLatencyPathCount				65
#define kDslG992TxLatencyPathCount				66
#define kDslG992LatencyPathId					67

/* performance monitoring report */

#define	kG992DataRcvDetectFastRSCorrection				70
#define	kG992DataRcvDetectInterleaveRSCorrection		71
#define	kG992DataRcvDetectFastCRCError					72
#define	kG992DataRcvDetectInterleaveCRCError			73
#define	kG992DataRcvDetectFastRSError					74
#define	kG992DataRcvDetectInterleaveRSError				75
#define	kG992DataRcvDetectLOS							76
#define	kG992DecoderDetectRDI							77
#define	kG992DataRcvDetectLOSRecovery					78
#define	kG992AtmDetectHEC								79
#define	kG992DataRcvDetectPartialNegativeMargin			80
#define	kG992DataRcvDetectPartialNegativeMarginRecovered	81
#define	kG992DecoderDetectRDIRecovery					179
#define	kG992AtmDetectOCD								180
#define	kG992AtmDetectCD								181
#define	kG992DecoderDetectRemoteLOS						182
#define	kG992DecoderDetectRemoteLOSRecovery				183
#define	kG992DecoderDetectRemoteRDI						184
#define	kG992DecoderDetectRemoteRDIRecovery				185
#define	kG992RcvDetectSyncSymbolOffset					186
#define	kG992Upstream2xIfftDisabled						187
#if defined(G992P5)
#define	kDslG992RunAnnexaP3ModeInAnnexaP5	   	        188	 /* run Annex C mode in Annex I compiled codes */
#else
#define	kDslG992RunAnnexCModeInAnnexI			        188	 /* run Annex C mode in Annex I compiled codes */
#endif

/* OLR PHY status */

#define	kG992EventSynchSymbolDetected					189
#define	kG992EventReverseSynchSymbolDetected			190
#define	kG992EventL2CReverbSymbolDetected				191
#define	kG992EventL2CSegueSymbolDetected				192

/* ANNEX_M */
#define kG992EnableAnnexM                               191

#define kDslAtuChangeTxFilterRequest					192

/* sample buffer conditions */

#define kDslMarkerCheckFailed							193
#define kDslMarkerCheckRecovered						194

#define kDslRcvBufferOverflowDetected					195
#define kDslRcvBufferOverflowRecovered					196

#define kDslXmtBufferOverflowDetected					197
#define kDslXmtBufferOverflowRecovered					198

#define kDslXmtBufferUnderflowDetected					199
#define kDslXmtBufferUnderflowRecovered					200

/* NSIF PLN statuses */

#define kG992SetPLNMessageBase							201
#define kG992DefaultPLNMessageBase						0x10

/* Loop Diagnostic (LD) statuses */

#define kG992LDStartMode								202
#define kG992LDCompleted								203

#define kDslAfeSampleLoss					   		    204

#define kDslRetrainReason                               205

/* Fire status US & DS */
#define kG992FireState                                  206


/* More PLN statuses */
#define kG992PlnBroadbandCounterReset                   207
#define kG992RcvDelay                                   208
#define kG992RcvInp                                     209

#define	kDslG992RunAnnexaP1Mode	   	                210


/* AFE info reporting */
#define kDslAfeLineDriverType                       211

/* Bonding State reporting */
#define kDslBondingState                            212

#define	kDslPwrStateRequest                         213	        /* Status call to exit L2 from PHY */

/* Vectoring states */
#define kDslVectoringEnabled                        220
#define kDslVectoringState                          221

/* Last states from Loop Diagnostics Mode */
#define kG992LDLastStateDs                        	222
#define kG992LDLastStateUs                        	223

#define kDslVectoringLineId                         224
#define kDslVectoringPilotSequenceLength            225
#define kDslIkanosCO4Detected                       226
#define kDslVectoringReportErrorSampleCounters      227
#define kDslVectoringFriendlyEnabled                228
#define kDslRiPolicyReinitTimeThreshold             229
#define kDslPtmOptionsDs                            230
#define kDslPtmOptionsUs                            231
#define kDslVectoringUseEoc                         232
#define kDslVectoringFdpsUs                         233

#define kDslGfastCOSupport                          234
#define kG992DataRcvDetectLOR                       235
#define kG992DataRcvDetectLORRecovery               236
#define kG992DataRcvDetectFeLOR                     237
#define kG992DataRcvDetectFeLORRecovery             238
#define kG992DataRcvDetectFeLOM                     239
#define kG992DataRcvDetectFeLOMRecovery             240
#define kG992DataRcvDetectFeLOS                     241
#define kG992DataRcvDetectFeLOSRecovery             242
/* This pair is only used for CO mode */
#define kG992DataRcvDetectFeLPR                     243
#define kG992DataRcvDetectFeLPRRecovery             244

#define kG992ReinitTimeThld                         245
#define kG992DataRcvDetectLOM                       246
#define kG992DataRcvDetectLOMRecovery               247

#define kDslGfastGfactor                            248

#define kG992LowETRThreshold                        249
#define kG992DataRcvDetectLowETR                    250
#define kG992DataRcvDetectLowETRRecovery            251
#define kDslIkanosAnnexcCODetected                  252
#define kDslExtendedRateStatSupport                 253

#define kDslNoLineDriver                            0x0
#define kDslLineDriver6301                          0x1
#define kDslLineDriver6302                          0x2
#define kDslLineDriverISL1556                       0x3
#define kDslLineDriverISL1557                       0x4

#define kDslLineDriver12vLineDriver                 0x1000
#define kDslLineDriver5vLineDriver                  0x2000
#define kDslLineDriver7vLineDriver                  0x4000

#define kDslBondingPmeIdHostSet                     0x80

/* detailed error messages reports */

#define	kDslG992XmtRReverbRAOver4000			80
#define	kDslG992XmtRReverb5Over4000				81
#define	kDslG992RcvCSegue2Failed				82
#define	kDslG992RcvCSegueRAFailed				83
#define	kDslG992RcvCSegue3Failed				84
#define	kDslG992RcvShowtimeStartedTooLate		85
#define	kDslG992XmtRReverb3Over4000				86
#define	kDslG992RcvFailDetCSegue1InWindow		87
#define	kDslG992RcvCPilot1Failed				88
#define	kDslG992RcvCReverb1Failed				89
#define	kG992ControlAllRateOptionsFailedErr		90
#define	kG992ControlInvalidRateOptionErr		91
#define	kDslG992XmtInvalidXmtDErr				92
#define	kDslG992BitAndGainCalcFailed			93
#define	kDslG992BitAndGainVerifyFailed			94

#define	kDslT1p413RetrainToUseCorrectRAck		95
#define	kDslUseAlternateTxFilter				96
#define	kDslT1p413RetrainToUseCorrectIFFT		97

/* Some more error codes for retrains, could be used during HS/TRN ADSL/VDSL These are non-bit mapped error codes */
#define kRetrainReasonConfigNotFeasibleUs       98      /* US configuration is not feasible conveyed to CPE by dslam in O-PMD message */
#define kRetrainReasonConfigNotFeasibleDs       99      /* DS configuration not feasible, meaning line capacity not sufficient etc */



typedef	int	dslConnectInfoStatusCode;
#define	kG992p2XmtToneOrderingInfo			0
#define	kG992p2RcvToneOrderingInfo			1
#define	kG992p2XmtCodingParamsInfo			2
#define	kG992p2RcvCodingParamsInfo			3
#define	kG992p2TrainingRcvCarrEdgeInfo		4
#define	kG992ShowtimeMonitoringStatus		5
#define	kG992MessageExchangeRcvInfo			6
#define	kG992MessageExchangeXmtInfo			7
#define	kG994MessageExchangeRcvInfo			8
#define	kG994MessageExchangeXmtInfo			9

#define	kDslATURClockErrorInfo				10
#define	kDslATURcvPowerInfo					11
#define	kDslATUAvgLoopAttenuationInfo		12
#define	kDslHWTimeTrackingResetClockError	13
#define	kDslHWTimeTrackingClockTweak		14
#define kDslATUHardwareAGCObtained			15
#define	kDslTEQCoefInfo						16
#define	kDslRcvCarrierSNRInfo				17
#define	kDslMaxReceivableBitRateInfo		18
#define kDslHWSetDigitalEcUpdateMode		19
#define kDslHWEnableDigitalECUpdate			20
#define kDslHWDisableDigitalECUpdate 		21
#define kDslHWEnableDigitalEC				22
#define kDslHWSetDigitalEcGainShift			23
#define kDslHWSetDigitalEcUpdateShift		24
#define	kDslRcvPsdInfo						25
#define	kDslHWdcOffsetInfo					26
#define	kG994SelectedG994p1CarrierIndex		27
#define	kDslSelectedTimingTone				28

#define	kDslHWEnableAnalogECUpdate			kDslHWSetDigitalEcUpdateMode	
#define	kDslHWEnableAnalogEC				kDslHWDisableDigitalECUpdate

#define	kG992AocMessageExchangeRcvInfo		29
#define	kG992AocMessageExchangeXmtInfo		30
#define	kG992AocBitswapTxStarted			31
#define	kG992AocBitswapRxStarted			32
#define	kG992AocBitswapTxCompleted			33
#define	kG992AocBitswapRxCompleted			34
#define kDslChannelResponseLog				35
#define kDslChannelResponseLinear			36
#define kDslChannelQuietLineNoise			37

#define	kDslATUCXmtPowerCutbackInfo			40
#define	kDslATURXmtPowerCutbackInfo			41
#define	kDslATUCXmtPowerInfo				42
#define	kDslATURXmtPowerInfo				43

/* Not sure why values between 43 and 50 are not used, will use 44 */
#define	kDslATUCShowtimeXmtPowerInfo		44

#define	kDslFramingModeInfo					50
#define	kDslG992VendorID					51

#ifdef BCM6348_SRC
#define kDslHWSetRcvFir2OutputScale			52
#endif

#define kDslSignalAttenuation               53
#define kDslAttainableNetDataRate           54
#define kDslHLinScale                       55

#define	kG992p3XmtCodingParamsInfo			60
#define	kG992p3RcvCodingParamsInfo			61
#define	kG992p3PwrStateInfo					62
#define	kG992PilotToneInfo					63

#define kDslSetPilotEyeDisplay              64

/* PLN control */
#define	kDslPLNPeakNoiseTablePtr			65
#define kDslPerBinThldViolationTablePtr		66
#define	kDslImpulseNoiseDurationTablePtr	67
#define	kDslImpulseNoiseTimeTablePtr		68
#define kDslPLNMarginPerBin                 69
#define kDslPLNMarginBroadband              70
#define kDslPerBinMsrCounter                71
#define kDslBroadbandMsrCounter             72
#define	kDslInpBinTablePtr					73
#define	kDslItaBinTablePtr					74
#define	kDslStatusBufferInfo				75
#define	kDslRcvCarrierSNRInfo1				76
#define kDslPlnState                                    77

/* Non-linear detection info */
#define kDslNLNoise                                     78
#define kDslNLMaxCritNoise                              79
#define kDslNLAffectedBits                              80
#define	kDslInitializationSNRMarginInfo                 81
#define kDslNLAffectedBins                              82

#define kDslINMConfig                                   83
#define	kDslImpulseNoiseDurationTableLongPtr	        84
#define	kDslImpulseNoiseTimeTableLongPtr    	        85
#define kDslINMControTotalSymbolCount                   86

#define	kDslAtmVcTablePtr				90

#define kDslG992RcvShowtimeUpdateGainPtr                91
#define kFireMonitoringCounters                         92
#define kDslRxOvhMsg                                    93
#define kDslTxOvhMsg                                    94

#define	kDslCycleTimeStamps			        95

#define kDslCpeSnrClampUserDefinedPtr                   96
#define kDslCpeSnrClampActualPtr                        97

#define	kG994VendorID                             100
#define	kDslNLdbEcho                              101
#define	kG992AocBitswapTxDenied             102
#define	kG992AocBitswapRxDenied             103
#define	kG992BitswapState                    104

#define kG992p2RcvToneOrderingInfo1             105
#define kDslATUAvgLoopAttenuationInfoAt300kHz   106

/* DOI */
#define	kDslRcvCarrierDOISNRInfo1				                107


#define kDslAturHwAgcResolutionMask			(0xFFFFFFF8)
#define kDslAturHwAgcMinGain				((-12)<<4)
#ifndef BCM6348_SRC
#define kDslAturHwAgcMaxGain				(30<<4)
#else
#define kDslAturHwAgcMaxGain				(36<<4)
#endif

#define	kDslFrameStatusSend					1
#define	kDslFrameStatusSendComplete			2
#define	kDslFrameStatusRcv					3
#define	kDslFrameStatusReturn				4

typedef struct _dslFramerStatus
	{
	dslFramerStatusCode		code;
	union
		{
		int			value;
		dslErrorCode		error;
		struct
			{
			int	length;
			uchar	*framePtr;
			} frame;
		struct
			{
			int	nRxFrameTotal;
			int	nRxFrameError;
			int	nTxFrameTotal;
			} statistic;
		} param;
	} dslFramerStatus;

typedef struct _dslShowtimeSNRMarginInfoType
	{
		int						maxMarginCarrier;
		int						maxSNRMargin;
		int						minMarginCarrier;
		int						minSNRMargin;
		int						avgSNRMargin;
		int						nCarriers;
		void						*buffPtr;
	} dslShowtimeSNRMarginInfoType;

typedef	struct
	{
	dslStatusCode					code;
	union
		{
		int						value;
		dslErrorCode				error;
		struct
			{
			atmStatusCode			code;
			union
				{
				int				value;
				dslErrorCode		error;
				struct
					{
					int	vci;
					int	mid;
					int	aalType;
					int	length;
					uchar	*framePtr;
					} frame;
				struct
					{
					int	nFrames;
					int	nFrameErrors;
					} statistic;
				struct
					{
					int	vcId;
					int	vci;
					int	aalType;
					uint   fwdPeakCellTime;
					uint   backPeakCellTime;
					} vcInfo;
				struct
					{
					int	cellHdr;
					int	oamCmd;
					} oamInfo;
				struct
					{
					void	*pVc;
					char	*pHdr;
					void	*cellHdr;
					void	*cellData;
					} cellInfo;
				struct
					{
					int	totalBits;
					int	errBits;
					} bertInfo;
				} param;
			} atmStatus;
#ifdef DSL_PACKET
		dslFramerStatus		dslPacketStatus;
#endif
#ifdef G997_1_FRAMER
		dslFramerStatus		g997Status;
#endif
		struct
			{
			dslTrainingStatusCode		code;
			int						value;
			} dslTrainingInfo;
		struct
			{
			dslConnectInfoStatusCode	code;
			int						value;
			void						*buffPtr;
			} dslConnectInfo;
		dslShowtimeSNRMarginInfoType	dslShowtimeSNRMarginInfo;
		struct
			{
			int						code;
			int						vcId;
			int						timeStamp;
			} dslFrameInfo;
#ifdef G997_1
		struct
			{
			int	msgId;
			int	msgType;
			char	*dataPtr;
			} dslClearEocMsg;
#endif
		struct
			{
			char	*fmt;
			int	argNum;
			void	*argPtr;
			} dslPrintfMsg;
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
		struct
			{
			uint	code;
			uchar	*valuePtr;
			uint	length;
			} dslDataRegister;
		struct
			{
			uint	code;
			char	*desc;
			} dslExternalError;
		struct
			{
			uint	numberOfCalls;
			uint	txSignalChecksum;
			uint	rxSignalChecksum;
			uint	eyeDataChecksum;
			} checksums;
		struct
			{
			int		sp;
			int		argc;
			int		*argv;
			int		*stackPtr;
			int		stackLen;
			} dslException;
		struct
			{
			uint	paramId;
			void	*dataPtr;
			uint	dataLen;
			} dslOemParameter;
		struct
			{
			uint	dataPtr;
			uint	dataLen;
			} dslDataAvail;
		struct
			{
			uint	type;
			uint	param1;
			uint	param2;
			} dslAfeTestStatus;

		dslOLRMessage		dslOLRRequest;
		dslPwrMessage		dslPwrMsg;
		} param ALIGN_PACKED;
	} dslStatusStruct;

#ifndef DSLVARS_GLOBAL_REG
typedef	void	(SM_DECL *dslStatusHandlerType)		(void *gDslVars, dslStatusStruct*);
#else
typedef	void	(SM_DECL *dslStatusHandlerType)		(dslStatusStruct*);
#endif /* DSLVARS_GLOBAL_REG */



/*
**
**		Command codes
**
*/

typedef int						dslCommandCode;
#define	kFirstDslCommandCode		256
#define kDslIdleCmd					(kFirstDslCommandCode + 0)
#define kDslIdleRcvCmd				(kFirstDslCommandCode + 1)
#define kDslIdleExtCmd				(kFirstDslCommandCode + 1)  /* idle with parameter */
#define kDslIdleXmtCmd				(kFirstDslCommandCode + 2)
#define kDslDownCmd					(kFirstDslCommandCode + 2)
  #define kDslIdleNone				0
  #define kDslIdleSuspend			1   /* can be restarted by 63138 PHY for media search */
#define	kDslStartPhysicalLayerCmd	(kFirstDslCommandCode + 3)
#define	kDslStartRetrainCmd			(kFirstDslCommandCode + 4)
#define	kDslSetFrameFunctions		(kFirstDslCommandCode + 5)
#define kDslSendEocCommand			(kFirstDslCommandCode + 6)
#define kDslWriteRemoteRegister		(kFirstDslCommandCode + 7)
#define kDslReadRemoteRegister		(kFirstDslCommandCode + 8)
#define kDslWriteLocalRegister		(kFirstDslCommandCode + 9)
#define kDslReadLocalRegister		(kFirstDslCommandCode + 10)
#define	kDslStoreHardwareAGCCmd		(kFirstDslCommandCode + 11)
#define kDslSetCommandHandlerCmd	(kFirstDslCommandCode + 12)
#define kSetLinkLayerStatusHandlerCmd (kFirstDslCommandCode + 13)
#define kDslSetG997Cmd				(kFirstDslCommandCode + 14)
#define kDslLoopbackCmd				(kFirstDslCommandCode + 15)
#define kDslDiagSetupCmd			(kFirstDslCommandCode + 16)
/* The associated eyeConstIndexx values are 32 bit bit fields, using the following shifts and definitions */
  #define kEyeNoChange              (-1)
  /* Tone index is compact format */
  #define kEyeIndexShift            0
  #define kEyeIndexMask             0xffff
  /* The following are currently only defined for Gfast */
  /* Select the symbol number within a frame, 5 bits needed. */
  #define kEyeSymShift              16
  #define kEyeSymMask               0xff
  #define kEyeToneTypeShift         24
  #define kEyeToneTypeMask          0xf
  #define kEyeToneTypeData          0 /* Use EyeIndex to select compact tone number */
  #define kEyeToneTypePilot         1 /* Use EyeIndex to select from pilot tone set */
  #define kEyeToneTypeRts           2 /* Use EyeIndex to reference RTS */
  #define kEyeSymTypeShift          28
  #define kEyeSymTypeMask           0xf
  #define kEyeSymTypeData           0 /* Use EyeSym to select symbol number within frame */
  #define kEyeSymTypeSync           1 /* Ignore symbol number and select SYNC symbol */
  #define kEyeSymTypeRmc            2 /* Ignore symbol number and select RMC symbol */
#define kDslSetDriverCallbackCmd	(kFirstDslCommandCode + 17)
#define kDslDiagStopLogCmd			(kFirstDslCommandCode + 18)
#define kDslDiagStartBERT			(kFirstDslCommandCode + 19)
#define kDslDiagStopBERT			(kFirstDslCommandCode + 20)
#define kDslPingCmd					(kFirstDslCommandCode + 21)
#define kDslDyingGaspCmd			(kFirstDslCommandCode + 22)
#define kDslTestCmd					(kFirstDslCommandCode + 23)
#define kDslFilterSNRMarginCmd		(kFirstDslCommandCode + 24)
#define kDslAtmVcMapTableChanged	(kFirstDslCommandCode + 25)
#define	kDslGetOemDataAddrCmd		(kFirstDslCommandCode + 26)
#define kDslAtmReportHEC			(kFirstDslCommandCode + 27)
#define kDslAtmReportCD				(kFirstDslCommandCode + 28)
#define kDslSetXmtGainCmd			(kFirstDslCommandCode + 29)
#define kDslSetStatusBufferCmd		(kFirstDslCommandCode + 30)
#define kDslAfeTestCmd				(kFirstDslCommandCode + 31)
#define kDslI432ResetCmd			(kFirstDslCommandCode + 32)
#define kDslPtmSyncResetCmd			kDslI432ResetCmd
#define kDslLinkLayerResetCmd		kDslI432ResetCmd
#define kDslSetRcvGainCmd           (kFirstDslCommandCode + 33)
#define kDslBypassRcvHpfCmd         (kFirstDslCommandCode + 34)
#define kDslWriteAfeRegCmd          (kFirstDslCommandCode + 35)
#define kDslReadAfeRegCmd           (kFirstDslCommandCode + 36)
#define	kDslOLRRequestCmd			(kFirstDslCommandCode + 37)
#define	kDslOLRResponseCmd			(kFirstDslCommandCode + 38)
#define kDslI432SetScrambleCmd		(kFirstDslCommandCode + 39)
#define	kDslPwrMgrCmd				(kFirstDslCommandCode + 40)
#define kDslAtmGfcMappingCmd		(kFirstDslCommandCode + 41)

#ifdef BCM6348_SRC
#define kDslEnablePwmSyncClk        (kFirstDslCommandCode + 42)
#define kDslSetPwmSyncClkFreq       (kFirstDslCommandCode + 43)
#endif

#define	kDslSetG994p1T1p413SwitchTimerCmd	(kFirstDslCommandCode + 44)
#define	kDslPLNControlCmd			(kFirstDslCommandCode + 45)

#define kDslSetDigEcShowtimeUpdateModeFast  (0)
#define kDslSetDigEcShowtimeUpdateModeSlow  (1)
#define kDslSetDigEcShowtimeUpdateModeCmd   (kFirstDslCommandCode + 46)

#define	kDslAtmVcControlCmd			(kFirstDslCommandCode + 47)
  #define kDslAtmVcClear			0
  #define kDslAtmVcAddEntry			(1 << 24)
  #define kDslAtmVcDeleteEntry		(2 << 24)
  #define kDslAtmSetMaxSDU			(3 << 24)
  #define kDslAtmEopMonitorEnable	(4 << 24)
  #define kDslAtmEopMonitorDisable	(5 << 24)
  #define kDslAtmReportVcTable		(6 << 24)
  #define kDslAtmSetFastPortId		(10 << 24)
  #define kDslAtmSetIntlPortId		(11 << 24)
#define kDslI432SetRxHeaderHandler	(kFirstDslCommandCode + 48)
#define kDslProfileControlCmd		(kFirstDslCommandCode + 49)
  #define kDslProfileEnable			1
  #define kDslProfileDisable		0
  #define kDslProfileCoreShift		30
  #define kDslProfileCoreMask		(3 << kDslProfileCoreShift)
#define kDslAfelbTestCmd            (kFirstDslCommandCode + 50)
#define kDslTestQuietCmd            (kFirstDslCommandCode + 51)
#define kDslTestQLNTimeCmd          (kFirstDslCommandCode + 52)
#define	kDslCycleMeasureControlCmd	(kFirstDslCommandCode + 53)
#define kDslDiagFrameHdrCmd			(kFirstDslCommandCode + 54)
#define kDslAfeTestCmd1				(kFirstDslCommandCode + 55)
#define kDslAfeReadXReg			1	/* param1 - Xreg Addr */
#define kDslAfeWriteXReg		2	/* param1 - Xreg Addr, param2 - Xreg Value */
#define kDslAfeReadIReg			3	/* param1 - Ireg Addr */
#define kDslAfeWriteIReg		4	/* param1 - Ireg Addr, param2 - Ireg Value */
#define kDslAfeRunCapture		5	/* param1 - RxLen; 0 - Max RcvBuffLen, param2 - TxLen; 0 - DefaultTxLen */
#define kDslAfeRunContinuous		6	/* param1 - TxLen; 0 - DefaultTxLen */
#define kDslAfeStop				7
#define kDslGetXmtBuffPtr		8
#define kDslGetRcvBuffPtr		9
#define kDslGetBuffPoolSize		10
#define kDslSetTxBuffSize		11	/* param1 - Lenght */
#define kDslSetLD6303ConfigData	12	/* param1 - High 32bits Value, param2 - Low 32bits Value */
#define kDslDiagSetupBufDesc		(kFirstDslCommandCode + 56)
#define kDslGenericProfControlCmd	(kFirstDslCommandCode + 57)
#define kDslBitswapControlCmd       (kFirstDslCommandCode + 58)
    #define kDslBitswapNormal           0
    #define kDslBitswapActivate         1
    #define kDslBitswapPeriodic         2
    #define kDslBitswapSetIdle          3
    #define kDslBitswapTestSequence     4
    #define kDslBitswapTestCtrlFlow     5
    #define kDslBitswapMgnEq            6
    #define kDslSRASetIdle              7
    #define kDslDynamicDDisable         8
    #define kDslDynamicFDisable         9
    #define kDslMonitorToneDisable      10
    #define kDslDynamicDTestD           11
    #define kDslDynamicDTestDL          12
    #define kDslDynamicFTestFL          13
    #define kDslBitswapDisableVN        14
    #define kDslBitswapEnableVN         15
    #define kDslBitswapBlankedSyncMode  16
    #define kDslDisableRxOLR            17
    #define kDslEnableRxOLR             18
    #define kDslDynamicFTestSOS         19
    #define kDslDynamicDFTestDFL        20
    #define kDslForceMarginChangeDs     21
    #define kDslForceSymCorruption1Ds   22
    #define kDslForceSymCorruption2Ds   23
    #define kDslGfastTestDoiOLR         24
    #define kDslGfastTestDoiNoiOLR      25
    #define kDslGfastOlrSendTigaFromCO  26
    #define kDslGfastDoiBitswapPeriodic 27
    #define kDslSetStartTone            28
    #define kDslSetEndTone              29

    #define kDslForceSnrToZeroInDs      50
    #define kDslForceFeqRotation        51

    /* Start of GFAST specific commands: Rx direction */
    #define kDslGfastTestRPA            64   /*trigger RPA every second */
    #define kDslGfastTestSRA_RPA        65   /*trigger FRA/SRA every second */
    #define kDslGfastTestFRA            66   /* trigger FRA every second */
    #define kDslGfastTestOutBandOLR     67   /*add out of band tones into OLR messages: simulate sckipio modems */
    #define kDslGfastTestRapidFRA       68   /* trigger fast FRA */
    #define kDslGfastTestFRATrigger1    69  /* trigger FRA once */
    #define kDslGfastTestDisableFRARecoveryMode1    70  /* disable FRA recovery mode1  */
    #define kDslGfastTestDisableFRARecoveryMode2    71  /* disable FRA recovery mode2  */
    #define kDslGfastTestBsNoRMC        72
    #define kDslGfastTestForceSRA_NRQ   73
    #define kDslGfastTestSendRmcRCmd    74
    /* End of GFAST specific commands */

    #define kDslTxBitswapNormal         128
    #define kDslTxBitswapSetIdle        129
    #define kDslTxSRASetIdle            130
    #define kDslTxBitswapPeriodic       131
    #define kDslForceMarginChangeUs     132
    #define kDslForceSymCorruption1Us   133
    #define kDslForceSymCorruption2Us   134
    #define kDslDisableVdslAdslTogglingInG994   160
    #define kDslEnableVdslAdslTogglingInG994    161
    #define kDslDumpBproc    162

    /* Start of GFAST specific commands: Tx direction */
    #define kDslGfastTestRPANack        164  /* test nack RPA */
    #define kDslGfastTestFRANack        165  /* nack FRA by no response */
    #define kDslGfastTestFRANoReply     166  /* apply FRA without response; testing reicever recovery scheme */
    #define kDslGfastTestBackupRMCNack  167  /* test nack backup RMC update */
    /* End of GFAST specific commands */

    #define kDslBitswapNoCmd            255
#define kDslSetPllOffset               (kFirstDslCommandCode + 59)
#define kDslSetNumOfBands           (kFirstDslCommandCode + 60)
#define kDslSetMaxTxPwr             (kFirstDslCommandCode + 61)
#define kDslLineIdCmd				(kFirstDslCommandCode + 62)
#define kDslTestSetAfeIdCmd         (kFirstDslCommandCode + 63)
#define kDslTestSetAuxFeaturesCmd   (kFirstDslCommandCode + 64)
#define kDslSystemControlCmd		(kFirstDslCommandCode + 65)
    #define kDslSysCtlDiv0              0x00000001  /* Reinit on Div0 exception */
    #define kDslSysCtlFgTimeout         0x00000002  /* Detect FG thread delay/stuck condition */
    #define kDslSysCtlExcpMemDump       0x00000004  /* Save memory to files on exception */
#define kDslRTXTestModeCmd			(kFirstDslCommandCode + 66)
    #define kDslRTXTestModeStart        0x00000001
    #define kDslRTXTestModeStop         0x00000000
#define kDslRTXSetUsEtr		(kFirstDslCommandCode + 67)
#ifdef ENABLE_6306_SHUTDOWN
#define kDslShutDown6306Cmd             (kFirstDslCommandCode + 68)
#endif
#define kDslSetUsDataDelay              (kFirstDslCommandCode + 69)
#define kDslSetV43PowerReduction    (kFirstDslCommandCode + 70)
#define kDslSetPllOffsetShowTime    (kFirstDslCommandCode + 71)
#define kDslMonToneControlCmd       (kFirstDslCommandCode + 72)

    #define kDslMonToneControlSetMode                   0x1
    #define kDslMonToneControlSetToneRange              0x2
    #define kDslMonToneControlSetForceTrainingMonTones  0x3
    #define kDslMonToneControlSetForbidTrainingMonTones 0x4

#define kDslSetTestModePLL          (kFirstDslCommandCode + 73)
#define kDslSeltConfiguration 		(kFirstDslCommandCode + 74)


    #define kDslSeltConfigPsdMask       (0x000000FF)    /* -60 - [0..255]*0.5  */
    #define kDslSeltConfigMaskTone      (0x0001FF00)    /* multiple of 8 tones */
    #define kDslSeltConfigDuration      (0x00FE0000)    /* in seconds for      */
    #define kDslSeltConfigToneGroup     (0xFF000000)    /* measurement done in group of tone*/
#define kDslTestENR                 (kFirstDslCommandCode + 75)
#define kDsl6303LdModeMSW           (kFirstDslCommandCode + 76)
#define kDsl6303LdModeLSW           (kFirstDslCommandCode + 77)
    #define kG994p1Duplex               1
    #define kG994p1HalfDuplex           2
#define kDslGenericProfControlCmd1  (kFirstDslCommandCode + 78)
#define kDslSNMAConfiguration       (kFirstDslCommandCode + 79)
#define kDslSNMACapConfiguration    (kFirstDslCommandCode + 80)
#define kDslGfastConfiguration        (kFirstDslCommandCode + 81)  /* 337 if kFirstDslCommandCode is 256 */
    #define kDslSetDsLogicalParams        1
    #define kDslSetUsLogicalParams        2
    #define kDslSetDsmaxRate              3         /* max DS data rate in kbps */
    #define kDslSetUsmaxRate              4         /* max US data rate in kbps */
    #define kDslSetDsNsymPerTddFrame      5         /* number of DS symbols per TDD frame: M_ds */
    #define kDslSetUsNsymPerTddFrame      6         /* number of US symbols per TDD frame: M_us */
    #define kDslSetNsymPerTddFrame        7         /* number of symbols (DS+US) per TDD frame: Tf */
    #define kDslSetTddFrameParams         8         /* TDD frame parameters: Mf/M_ds/M_us packed as <Mf<<24|M_ds<<16|M_us<<8|kDslSetTddFrameParams> */
    #define kDslSetDsRMCpos               9         /* DS RMC position:RMC_pos_ds */
    #define kDslSetUsRMCpos               10        /* DS RMC position: RMC_pos_us */
    #define kDslSetTddFrameSvals          11        /* TDD frame parameters: Sds/Sus packed as <Sds<<16|Sus<<8|kDslSetTddFrameSvals> */
    #define kDslSetDsBandPlan             12        /* BandPlan: endTone <<20 | startTone<<8|kDslSetDsBandPlan*/
    #define kDslSetUsBandPlan             13        /* BandPlan: endTone <<20 | startTone<<8|kDslSetDsBandPlan*/
    #define kDslSetLorPersistancy         14        /* lor persistency */
    #define kDslSetInpMinRein             15        /* INPminREIN */
    #define kDslSetMaxDelay               16        /* maxDelay   */
#define kDslDisableADSLTXFilterSetTXPGA (kFirstDslCommandCode + 82)  /* 338 if kFirstDslCommandCode is 256 */
#define kDslEyeToneSet                  (kFirstDslCommandCode + 83)  /* 339 if kFirstDslCommandCode is 256 */

#define kDslGfastRTXTestModeCmd         (kFirstDslCommandCode + 84)
    #define kDslGfastRTXTestModeStart    0x00000001
    #define kDslGfastRTXTestModeStop     0x00000000
#define kDslGfastTPSTestModeCmd         (kFirstDslCommandCode + 85)
    #define kDslGfastTPSTestModeStart    0x00000001
    #define kDslGfastTPSTestModeStop     0x00000000
#define kDslGfastRetrainCmd             (kFirstDslCommandCode + 86)
#define kDslVdslAfeIdCmd                (kFirstDslCommandCode + 87)
#define kDslStackDumpCmd                (kFirstDslCommandCode + 88)
#define kDslStatPrintCmd                (kFirstDslCommandCode + 89) /* 345 if kFirstDslCommandCode is 256 */
#define kDslGfastRxNoiseReduceControl   (kFirstDslCommandCode + 90)
#define kDslTestRateChange              (kFirstDslCommandCode + 91)
#define kDslSetTxPsdCeiling             (kFirstDslCommandCode + 92) /* Used on 42 branch */
#define kDslPrintfCtrl                  (kFirstDslCommandCode + 93)
#ifdef INM_GFAST
#define kDslSetInmParams                (kFirstDslCommandCode + 94) /* Set INM params (Gfast compatible) */
#endif
#define kDslHostResettingPHYMIPS        (kFirstDslCommandCode + 95)
#define kDslSetTestModeRateSelect       (kFirstDslCommandCode + 96)
#define kDslReportLinkState             (kFirstDslCommandCode + 97)
#define kDslSetCrestFactor              (kFirstDslCommandCode + 98) /* 354 if kFirstDslCommandCode is 256 */
#define kDslSetLineResumeDelay          (kFirstDslCommandCode + 99)


/* Bit fields for use with kDslStatPrintCmd */
    #define kDslStatBgPrint                     1 /* Print BG statistics if 1 */
    #define kDslStatICacheSel                   0 /* Connect to Icache events */
    #define kDslStatDCacheSel                   2 /* Connect to Dcache events */
    #define kDslStatAllFgOvershot               4 /* Print FG cycle overshot regardless of print congestion */
    #define kDslStatTddControlPos               4 /* Bit position of field to control TDD statistic gathering */
    #define kDslStatTddControlLen               6 /* Bit length of TDD statistic gathering field */
    /* Use the provided TDD symbol number unless one of the following reserved values is specified */
    #define kDslStatTddAll                   0x3f /* Gather statistics on all symbols when in TDD mode--DEFAULT */
    #define kDslStatTddAuto                  0x3e /* Auto increment TDD symbol number to gather statistics */

/* Eoc Messages from ATU-C to ATU-R */
#define kDslEocHoldStateCmd						1
#define kDslEocReturnToNormalCmd				2
#define kDslEocPerformSelfTestCmd				3
#define kDslEocRequestCorruptCRCCmd				4
#define kDslEocRequestEndCorruptCRCCmd			5
#define kDslEocNotifyCorruptCRCCmd				6
#define kDslEocNotifyEndCorruptCRCCmd			7
#define kDslEocRequestTestParametersUpdateCmd	8
#define kDslEocGrantPowerDownCmd				9
#define kDslEocRejectPowerDownCmd				10

/* Eoc Messages  from ATU-R to ATU-C */
#define kDslEocRequestPowerDownCmd				11
#define kDslEocDyingGaspCmd						12

/* Clear Eoc Messages  */
#define kDslClearEocFirstCmd					100
#define kDslClearEocSendFrame					100
#define kDslClearEocSendComplete				101
#define kDslClearEocRcvedFrame					102
#define kDslClearEocSendComplete2				103

#define kDslClearEocMsgLengthMask				0x0000FFFF
#define kDslClearEocMsgNumMask					0x00FF0000
#define kDslClearEocMsgDataVolatileMask			0x01000000
#define kDslClearEocMsgDataVolatile				kDslClearEocMsgDataVolatileMask
#define kDslClearEocMsgExtraSendComplete		0x02000000
#define kDslClearEocMsgHostAddr					0x04000000
#define kDslClearEocMsgGfastHdr					0x08000000

/* General command messages (using clearEOC message structure) */

#define kDslGeneralKeyChallengeResponse				300
#define kDslSetConfigGfast							301
#define kDslSetRfiConfigGfast						302
#define kDslSetHmiCoreConfig                    303
#define kDslFlushMemBlock                       304
#define kDslVdslCaps                            305

#define kDsl993p2PsdDescriptorDs				361
#define kDsl993p2PsdDescriptorUs				362
#define kDsl993p2BpDescriptorDs					363
#define kDsl993p2BpDescriptorUs					364

#define kDslAfeInfoCmd						367
#define kDslNtrConfig						379
#define kDslDumpBufferCfg					435

/* General status messages (using clearEOC message structure) */
#define kDslGeneralMsgStart					      300
#define kDslGeneralMsgDbgDataPrint			  300
#define kDslGeneralMsgDbgPrintf					  301
#define kDslGeneralMsgDbgPrintG992p3Cap	  303
#define kDslGeneralMsgDbgProfData				  304
#define kDslGeneralMsgDbgFileName				  305
#define kDslGeneralMsgDbgWriteFile			  306
#define kDslGeneralMsgE14Print					  307
#define kDslGeneralMsgE14PrintArg				  308
#define kDslGeneralMsgE14DataDump				  309
#define kDsl993p2SocMsgDump				        310
#define kDsl993p2BandPlanDsDump			      311
#define kDsl993p2BandPlanUsDump			      312
#define kDsl993p2PsdDump				          313
#define kDsl993p2QlnRaw                   314
#define kDsl993p2HlogRaw                  315
#define kDsl993p2SnrRaw                   316
#define kDsl993p2LnAttnRaw                317
#define kDsl993p2NeBi                     318
#define kDsl993p2NeGi                     319
#define kDsl993p2NeTi                     320
#define kDsl993p2FeBi                     321
#define kDsl993p2FeGi                     322
#define kDsl993p2FeTi                     323
#define kDsl993p2FramerDeframerUs         324
#define kDsl993p2FramerDeframerDs         325
#define kDsl993p2MaxRate                  326
#define kDsl993p2SNRM                     327
#define kDsl993p2SNRMpb                   328
#define kDsl993p2dsATTNDR                 329
#define kDsl993p2SATNpbRaw                330
#define kDslGeneralMsgDbgGenericProfData	331
#define kDsl993p2PowerNeTxTot             332
#define kDsl993p2PowerNeTxPb              333
#define kDsl993p2PowerFeTxTot             334
#define kDsl993p2PowerFeTxPb              335
#define kDsl993p2FeQlnLD                  336
#define kDsl993p2FeHlogLD                 337
#define kDsl993p2FeSnrLD                  338
#define kDsl993p2FeHlinLD                 339
#define kDsl993p2NeQlnLD                  340
#define kDsl993p2NeHlogLD                 341
#define kDsl993p2NeSnrLD                  342
#define kDsl993p2NeHlinLD                 343
#define kDsl993p2FePbLatnLD               344
#define kDsl993p2FePbSatnLD               345
#define kDsl993p2FePbSnrLD                346
#define kDsl993p2NePbLatnLD               347
#define kDsl993p2NePbSatnLD               348
#define kDsl993p2NePbSnrLD                349
#define kDsl993p2FeAttnLD                 350
#define kDsl993p2NeAttnLD                 351
#define kDsl993p2FeTxPwrLD                352
#define kDsl993p2NeTxPwrLD                353
#define kDslGeneralMsgDbgGenericProfStart	354
#define kDslGeneralMsgDbgGenericProfStop	355
#define kDslGeneralMsgPrintf				      356
#define kDslGeneralMsgDBPrintf			      357
#define kDsl993p2BpType					          358
#define kDsl993p2FramerAdslUs				      359
#define kDsl993p2FramerAdslDs				      360

#define kDslVdslSraReqSnd					        365
#define kDslVdslSraReqRecvd					      366

#define kDslPhyInfoCmd						        368
#define kDsl993p2Profile                  369
#define kDsl993p2Annex                    370
#define kDsl993p2TestHlin                 371
#define kDslNtrCounters	                  372
#define kDslExcpType	                  373
#define kDslExcpRegs					  374
#define kDslExcpArgs	                  375
#define kDslExcpStack	                  376

#define kDslSetSnrClampingMask            377
#define kDslGeneralMsgE14Print1			  378

#define kDslVdslSosReqSnd					        380
#define kDslVdslSosReqRecvd					      381

#define kDslG994VendorId			382
#define kDslActualPSDUs                   383
#define kDslActualPSDDs                   384
#define kDslSNRModeUs                     385
#define kDslSNRModeDs                     386
#define kDslActualCE                      387
#define kDslUPBOkle                       388
#define kDslQLNmtUs                       399
#define kDslQLNmtDs                       400
#define kDslSNRmtUs                       401
#define kDslSNRmtDs                       402
#define kDslHLOGmtUs                      403
#define kDslHLOGmtDs                      404

#define kDslVectoringStartDumpCmd         405
#define kDslVectoringErrorSamples         406
#define kDslVectoringSetPilotCmd          407
#define kDslVectoringSetPilotFDPSCmd      408

#define kGinpMonitoringCounters           410
#define kStatusBufferHistogram            411

#define kDslExtraPhyCfgCmd                412
#define kVectoringMacAddress              413
#define kDslVectoringPilotSequence        414
#define kDsl993p2LnAttnAvg                415

#define kDsl993p2BitSwapTones             416
#define kDsl993p2UsGi                     417
#define kDsl993p2FeBiPhy                  418
#define kDsl993p2NeGiPhy                  419
#define kDsl993p2NeBiPhy                  420
#define kDsl993p2US0mask                  421
#define kDsl993p2MrefPSDds                422
#define kDsl993p2MrefPSDus                423
#define kDsl993p2LimitMask                424
#define kDsl993p2VnPSDds                  425
#define kDsl993p2VnPSDus                  426

#define kDslBondDiscExchange              427
#define kDslBondDiscExchangeDrv           428    /* only to print Drv command in Diags */
#define kDsl993p2DSkl0perBand             429
#define kDsl993p2USkl0perBand             430
#define kDslAFERegInfo                    431
#define kDsl993p2SnrROC                   432

#define kDslStrPrintf                     433
#define kDslStrDBPrintf                   434

#define kDsl993p2dsATTNDRmethod           436
#define kDsl993p2dsATTNDRinp              437
#define kDsl993p2dsATTNDRdel              438

#define kDslEchoVariance                  439
#define kDslNtrStates                     440
#define kDsl993p2QlnRawRnc                441
#define kDslChannelQlnRnc                 442
#define kDslTodTimeStamp                  443

#define kDsl9701SocMsgDump                444

#define kDslShowtimeSnrMarginHdrQ8        445
#define kDslShowtimeSnrMarginDataQ8       446

#define kDslPhyEocRxMsg                   447
#define kDslPhyEocTxMsg                   448

#define kDsl993p2NeRxPower                449
#define kGfastEocMonitoringCounters       450
#define kDsl993p2AlnRaw                   451
#define kDslGeneralMsgE14Printf           452

/* DOI support */
#define kDsl993p2DOINeBi                  453
#define kDsl993p2DOINeGi                  454
#define kDsl993p2DOINeTi                  455
#define kDsl993p2DOIFeBi                  456
#define kDsl993p2DOIFeGi                  457
#define kDsl993p2DOIFeTi                  458

#define kDslStackDump                     459
/* This structure to count Gfast Vector Feedback message requests and Segments sent */
#define kDslGfastVectoringEocSegments     460

/* GFAST DOI related snr and bi gi buffers  */
#define kDslShowtimeDoiSnrMarginHdrQ8	      461
#define kDslShowtimeDoiSnrMarginDataQ8	      462
#define kDslGfastDoiSnrRaw                    463

#define kDslGfastAhifRegDump                  464
#define kDslUPBOkleCpe                        465
#define kDslGfastSupportedOption              466
#define kDslGeneralMsgKeyChallenge            467

#define RMC_PARAM_REPORTING
#ifdef RMC_PARAM_REPORTING  //FIXME: remove all RMC_PARAM_REPORTING after driver support
/* Gfast RMC bit loading table*/
#define kDsl993p2NeRmcBi                      468
#define kDsl993p2FeRmcBi                      469
/* Gfast RMC tone set*/
#define kDsl993p2NeRTSPhy                     470
#define kDsl993p2FeRTSPhy                     471
/* Gfast RMC showtime SNR margin*/
#define kDslGfastRmcSnrMargin                 472
/* RMC mean snr margin  */
/* kDsl993p2SnrROC: RMC mean SNR Margin  */
#endif
#define kDslMipsRegDump                       473
#define kDslAutoINPInUse                      474

#define kDslGfastDtaInfo                      475
#define kDslSprocDis                          476
#define kDslCmdDispTblDump                    477
#define kDsl993p2LRActOpType                  478

/* Interop footprint bits */
#define kDslInteropDisablePhyRDs            0x0001
#define kDslInteropPhyRUsRateSelectCo       0x0002
#define kDslInteropRetrainWithShorterCLR    0x0004
#define kDslInteropRetrainWithVDSL2Only     0x0008
#define kDslInteropNoReverbBelowTone33      0x0010
#define kDslInteropRetrainFor6314Power      0x0020
#define kDslInteropAvoidMaxD511             0x0040
#define kDslInteropAssumeFullExtendedD      0x0080
#define kDslInteropLimitedBrcmNsif          0x0100
#define kDslInteropMaskGinpInFirstCLR       0x0200
#define kDslInteropExpectIkanosHs           0x0400
#define kDslInteropHeavyOutofBandRFI        0x0800
#define kDslInteropMaskChannelPolicyInCLR   0x1000
#define kDslInteropMaskPmeIdInCLR           0x2000
#define kDslInteropMaskApplyDSPCB           0x4000
#define kDslInteropDenyNextTxOLR            0x8000
#define kDslInteropMaskDisableA7InFirstCLR  0x00010000
#define kDslInteropEnableIkanosCO4ModeFirst 0x00020000
#define kDslInteropEnableIkanosCO4ModeFull  0x00040000
#define kDslInteropHeavyInBandRFI           0x00080000
#define kDslInteropSwapToMonitorToneDisable 0x00100000
#define kDslInteropForceOlrControlLsb       0x00200000
#define kDslInteropReduceRxBitswapMaxTones  0x00400000
#define kDslInteropSraGinpOverAdsl          0x00800000
#define kDslInteropEnableIkanosCO4AdslMode  0x01000000
#define kDslInteropMaskG993p5InCLR          0x02000000
#define kDslInteropUseV43onlyForIKNS        0x04000000
#define kDslInteropMaskBoostAnnexJPSD       0x08000000
#define kDslInteropIftnAnnexJReverbBoost    0x10000000
#define kDslInteropExpectSameUsDsPilotSeq   0x20000000
#define kDslInteropForceIdealMonEnabled     0x40000000
#define kDslInteropForceIdealMonDisabled    0x80000000

#define kDslInteropForceIdealMon            (kDslInteropForceIdealMonEnabled|kDslInteropForceIdealMonDisabled)
#define kDslInteropEnableIkanosCO4Mode      (kDslInteropEnableIkanosCO4ModeFirst|kDslInteropEnableIkanosCO4ModeFull)

#ifdef G992P1_ANNEX_A
#define kDslInteropCarryOverMask            (kDslInteropRetrainWithVDSL2Only|kDslInteropRetrainFor6314Power|kDslInteropEnableIkanosCO4ModeFull|kDslInteropForceIdealMon)
#else
#define kDslInteropCarryOverMask            (kDslInteropRetrainWithVDSL2Only)
#endif

/* Interop footprint bits 2nd set */
#define kDslInterop2xtalkDetected           0x00000001
#define kDslInteropDisableFEQRotationCheck  0x00000002
#define kDslInteropGSPNsendWIREid           0x00000004
#define kDslInteropBrcmCoDetected           0x00000008
#define kDslInterop2IkanosCO4Mode           0x00000010
#define kDslInterop2GfastPropExtension      0x00000020
#define kDslInterop2BrcmCoFirstTraining     0x00000040
#define kDslInterop2_free                   0x00000080
#define kDslInterop2BrcmCoTargetNM4dB       0x00000100
#define kDslInterop2GfastForcePropExtension 0x00000200
#define kDslInterop2ForceNoNSIF             0x00000400
#define kDslInterop2Corr1EarlyTraining      0x00000800
#define kDslInterop2ExpectDualPilotSeq      0x00001000
#define kDslInterop2extendedBmaxSupport     0x00002000
#define kDslInterop2DisableBondingForIFTN30a      0x00004000
#define kDslInterop2GfastPreCorr3MonitorTone      0x00008000
#define kDslInterop2BrcmCoVectoringBugWorkaround  0x00010000
#define kDslInterop2EnableIkanosAnnexCmode        0x00020000
#define kDslInterop2EnableIkanosAnnexCmodeInit    0x00040000
#define kDslInterop2EnableIkanosAnnexCmodeCO6     0x00080000
#define kDslInterop2EnableIkanosAnnexCmodeCO6_SRA  0x00100000
#define kDslInterop2EnableIkanosAnnexCmodeCO6_ROC        0x00200000
#define kDslInterop2EnableIkanosAnnexCretrainForLength   0x00400000

#ifdef G992P1_ANNEX_B
#define kDslInterop2CarryOverMask           kDslInterop2BrcmCoVectoringBugWorkaround
#else
#define kDslInterop2CarryOverMask           (kDslInterop2EnableIkanosAnnexCretrainForLength|kDslInterop2EnableIkanosAnnexCmodeInit)
#endif

/* General kDslGeneralMsgDbgDataPrint flags */
#define kDslDbgDataSizeMask					0x00030000
#define kDslDbgDataSize8					0x00000000
#define kDslDbgDataSize16					0x00010000
#define kDslDbgDataSize32					0x00020000
#define kDslDbgDataSize64					0x00030000

#define kDslDbgDataSignMask					0x00040000
#define kDslDbgDataSigned					0x00040000
#define kDslDbgDataUnsigned					0x00000000

#define kDslDbgDataFormatMask					0x00080000
#define kDslDbgDataFormatHex					0x00080000
#define kDslDbgDataFormatDec					0x00000000
#define kDslDbgEocTxIncomplete              kDslDbgDataFormatHex

#define kDslDbgDataQxShift					20
#define kDslDbgDataQxMask					(0xF << kDslDbgDataQxShift)
#define kDslDbgDataQ0					0x00000000
#define kDslDbgDataQ1					(1 << kDslDbgDataQxShift)
#define kDslDbgDataQ4					(4 << kDslDbgDataQxShift)
#define kDslDbgDataQ8					(8 << kDslDbgDataQxShift)
#define kDslDbgDataQ12					(12 << kDslDbgDataQxShift)
#define kDslDbgDataQ15					(0xF << kDslDbgDataQxShift)

#define kDslDbgDataString				(kDslDbgDataSize8 | kDslDbgDataSigned | kDslDbgDataFormatHex)
#define kDslDbgDataStringF				(kDslDbgDataString | kDslDbgDataQ1)

#define kDslDbgDataStrIdF				0x80000000   /* flag for SoftDslDpApiPrintfF */

/* next after kDslClearEocMsgExtraSendComplete */
#define kDslDbgDataBitReverseMask		0x04000000
#define kDslDbgDataBitReverse			kDslDbgDataBitReverseMask


/* General kDslGeneralMsgDbgPrintf flags */
#define kDslDbgDataPrintfIdMask			0x00FF0000
#define kDslDbgDataPrintfIdShift				16

/* Open file kDslGeneralMsgDbgFileName flags */
#define kDslDbgFileNameFlagsMask		0x00FF0000
#define kDslDbgFileNameDelete			0x00010000

/* ADSL Link Power States */
#define kDslPowerFullOn						0
#define kDslPowerLow						1
#define kDslPowerIdle						3

/* ATU-R Data Registers */
#define kDslVendorIDRegister				1
#define kDslRevisionNumberRegister			2
#define kDslSerialNumberRegister			3
#define kDslSelfTestResultsRegister			4
#define kDslLineAttenuationRegister			5
#define kDslSnrMarginRegister				6
#define kDslAturConfigurationRegister		7
#define kDslLinkStateRegister				8

#define kDslVendorIDRegisterLength			8
#define kDslRevisionNumberRegisterLength	32
#define kDslSerialNumberRegisterLength		32
#define kDslSelfTestResultsRegisterLength	1
#define kDslLineAttenuationRegisterLength	1
#define kDslSnrMarginRegisterLength			1
#define kDslAturConfigurationRegisterLength	30
#define kDslLinkStateRegisterLength			1

/* Dsl Diags setup flags */
#define kDslDiagEnableEyeData				1
#define kDslDiagEnableLogData				2
#define kDslDiagEnableDebugData				4

/* Dsl test commands */
typedef	int								dslTestCmdType;
#define	kDslTestBackToNormal				0
#define kDslTestReverb						1
#define kDslTestMedley						2
#define kDslTestToneSelection				3
#define	kDslTestNoAutoRetrain				4
#define	kDslTestMarginTweak					5
#define kDslTestEstimatePllPhase            6
#define kDslTestReportPllPhaseStatus        7
#define kDslTestAfeLoopback					8
#define kDslTestL3							9
#define kDslTestAdsl2DiagMode				10
#define kDslTestRetL0						11
#define kDslTestExecuteDelay                              12
#define kDslTestQuiet   					13
#define kDslTestManufacturingTest           14
#define kDslTestQLN						15
#define kDslTestHybridResp				16
#define kDslTestECResp				17
#define kDslTestNtrStop					18
#define kDslTestNtrStart				19
#define kDslTestFreezeDuringReverb          20
#define kDslTestFreezeDuringMedley          21
#define kDslTestNextSelt                    22
#define kDslTestFreezeDuringECT1            23
#define kDslTestFreezeDuringECT2            24
#define kDslTestFreezeInShowtime            25
#define kDslTestNoAfeCompensation           26
#define kDslTestTxPSD60dBmHz                27
#define kDslTestForce1storderHpOff          28
#define kDslTestFreezeDuringRPVector1       29
#define kDslTestHybridSelt                  30
#define kDslTestRncQLN                      31
#define kDslTestFreezeDuringRTones          32
#define kDslTestForceVdsl2LR                33


/* Xmt gain default setting */
#define	kDslXmtGainAuto						0x80000000

/* Unit (AFE) test commands */
#define	kDslAfeTestLoadImage				0
#define	kDslAfeTestPatternSend				1
#define	kDslAfeTestLoadImageOnly				2
#define	kDslAfeTestPhyRun				3
#define	kDslAfeTestLoadBuffer				4
#define	kDslAfeTestRdEndOfFile				5
#define	kDslAfeTestLoadStrDataBase			6
#define	kDslAfeTestLoadMemDump				7

/* kDslPLNControlCmd sub-commands */

#define	kDslPLNControlStart					1
#define	kDslPLNControlStop					2
#define	kDslPLNControlClear					3
#define	kDslPLNControlPeakNoiseGetPtr			4
#define kDslPLNControlThldViolationGetPtr		5
#define	kDslPLNControlImpulseNoiseEventGetPtr	6
#define	kDslPLNControlImpulseNoiseTimeGetPtr	7
#define kDslPLNControlGetStatus                         8
#define kDslPLNControlDefineInpBinTable                 9
#define kDslPLNControlDefineItaBinTable                 10
#define kDslPLNControlDefineDefaultBinTables            11
#define kDslINMControlParams                            12
#define kDslINMConfigParams                             13
#define kDslINMConfigInpEqFormat                        14

/* PLN constants */
#define kPlnNumberOfDurationBins       32
#define kPlnNumberOfInterArrivalBins   16
#define	kPlnNumberOfRcvErrMonitorCarrPerSymb  12	/* Number of monitored carriers per symbol to calculate error power (PLN) */

#define kDslRetrainForBT50dBREINTest          0x01   /* To pass REIN tests when minINP is 0 at 50 dB loop legth */

typedef struct {
	ulong	addr;
	ulong	size;
} memBlockInfo;

typedef struct {
  uchar                repetitionRate;
  uchar                reInitTimeOut;              /* Timeout for CPE detection in [s] */
  uchar                gfast_tdd_params_Mds;       /* US/DS split, valid range [2,32] */
  uchar                gfast_tdd_params_Mf;        /* currently hardwired to 36       */
  uchar                gfast_tdd_params_Msf;       /* currently hardwired to 8        */
  uchar                gfast_cyclic_extension_m;   /* Lcp = gfast_cyclic_extension_m*N/64. currently hardwired to 10 */
} msgCoreConfig;

typedef struct
	{
#if defined(G992P1_ANNEX_I) || defined(G992P5) || defined(G993)
	ushort 				downstreamMinCarr, downstreamMaxCarr;
#else
	uchar 				downstreamMinCarr, downstreamMaxCarr;
#endif
#if !defined(G993)
	uchar           	upstreamMinCarr, upstreamMaxCarr;
#else
	ushort           	upstreamMinCarr, upstreamMaxCarr;
#endif
	}carrierInfo;

#if defined(G992P3) && !defined(BCM6348_SRC)
#define	FAST_TEXT_TYPE
#else
#define	FAST_TEXT_TYPE			FAST_TEXT
#endif

#if defined(BCM6348_SRC)
#define BCM6348_TEMP_MOVE_TO_LMEM
#else
#define BCM6348_TEMP_MOVE_TO_LMEM
#endif

#ifdef	G992P3
#ifndef BCM6368_SRC
#undef	PRINT_DEBUG_INFO
#endif
#else
#define	PRINT_DEBUG_INFO
#endif

#ifdef G992P3

#define		kG992p3MaxSpectBoundsUpSize		16
#define		kG992p3MaxSpectBoundsDownSize	16

/* G.994 definitions */

/*** Standard Info SPar2:  G.992.3 Annex A  Octet 1 ***/

#define	kG994p1G992p3AnnexASpectrumBoundsUpstream	0x01
#define	kG994p1G992p3AnnexASpectrumShapingUpstream	0x02
#define	kG994p1G992p3AnnexASpectrumBoundsDownstream	0x04
#define	kG994p1G992p3AnnexASpectrumShapingDownstream	0x08
#define	kG994p1G992p3AnnexATxImageAboveNyquistFreq	0x10
#define	kG994p1G992p3AnnexLReachExtended			0x20
#define kG994p1G992p3AnnexMSubModePSDMasks          0x20

#define	kG994p1G992p3AnnexLUpNarrowband				0x02
#define	kG994p1G992p3AnnexLUpWideband				0x01
#define	kG994p1G992p3AnnexLDownNonoverlap			0x01

#define kG994p1G992pNAnnexMUpAdlu32                             0x001
#define kG994p1G992pNAnnexMUpAdlu36                             0x002
#define kG994p1G992pNAnnexMUpAdlu40                             0x004
#define kG994p1G992pNAnnexMUpAdlu44                             0x008
#define kG994p1G992pNAnnexMUpAdlu48                             0x010
#define kG994p1G992pNAnnexMUpAdlu52                             0x020
#define kG994p1G992pNAnnexMUpAdlu56                             0x040
#define kG994p1G992pNAnnexMUpAdlu60                             0x080
#define kG994p1G992pNAnnexMUpAdlu64                             0x100

/*** Standard Info SPar2:  G.992.3 Annex A  Octet 2 ***/

#define	kG994p1G992p3AnnexADownOverheadDataRate		0x01
#define	kG994p1G992p3AnnexAUpOverheadDataRate		0x02
#define	kG994p1G992p3AnnexAMaxNumberDownTPSTC		0x04
#define	kG994p1G992p3AnnexAMaxNumberUpTPSTC			0x08
#define kG994p1G992p3AnnexAVirtualNoiseNBPds        0x20

/*** Standard Info SPar2:  G.992.3 Annex A  Octet 3,5,7,9 ***/

#define	kG994p1G992p3AnnexADownSTM_TPS_TC			0x01
#define	kG994p1G992p3AnnexAUpSTM_TPS_TC				0x02
#define	kG994p1G992p3AnnexADownATM_TPS_TC			0x04
#define	kG994p1G992p3AnnexAUpATM_TPS_TC				0x08
#define	kG994p1G992p3AnnexADownPTM_TPS_TC			0x10
#define	kG994p1G992p3AnnexAUpPTM_TPS_TC				0x20

/*** Standard Info SPar2:  G.992.3 Annex A  Octet 4,6,8,10 ***/

#define	kG994p1G992p3AnnexADownPMS_TC_Latency		0x01
#define	kG994p1G992p3AnnexAUpPMS_TC_Latency			0x02
#define kG994p1G992p3AtmPMS_TC_RETX                 0x04
#define kG994p1G992p3PtmPMS_TC_RETX                 0x08

#define kG994p1G992p35CiPolicyZero                  0x01
#define kG994p1G992p35CiPolicyOne                   0x02
#define kG994p1G992p35CiPolicyTwo                   0x04
#define kG994p1G992p35CiPolicySupported             (kG994p1G992p35CiPolicyZero|kG994p1G992p35CiPolicyOne|kG994p1G992p35CiPolicyTwo)

/***
 *   TSSI
 *
 *   TSSI information is specified in 2 parts: subcarrier index,
 *   tssi value, and an indication of whether or no the tone specified
 *   is part of the supported set.
 *
 *   The subcarrier index information is currently stored in the
 *   dsSubcarrierIndex array defined below. The tssi value are stored
 *   in the dsLog_tss array.
 *
 *   The subcarrier index information only occupies the lower 12 bits
 *   of the available 16 bits (short type). Therefore, we will pack the
 *   supported set information in bit 15.
 */
#define kG992DsSubCarrierIndexMask          (0x0fff)   /* AND mask to ectract ds subcarrier index */
#define kG992DsSubCarrierSuppSetMask        (0x8000)   /* AND mask to extract supported set indication */

#define G992GetDsSubCarrierIndex(arg)               ((arg)  & kG992DsSubCarrierIndexMask)
#define G992GetDsSubCarrierSuppSetIndication(arg)   (((arg) & kG992DsSubCarrierSuppSetMask) >> 15)

/* Caution: Do not change anything in this structure definition, including associated constant */
/* This structure definition is used only by the driver and any change impose incompatibility issue in driver */
/* The structure following this structure (g992p3PhyDataPumpCapabilities) can be changed in PHY application */

typedef struct
	{
	Boolean				rcvNTREnabled, shortInitEnabled, diagnosticsModeEnabled;

	char				featureSpectrum, featureOverhead;
	char				featureTPS_TC[4], featurePMS_TC[4];

	short				rcvNOMPSDus, rcvMAXNOMPSDus, rcvMAXNOMATPus;
	short				usSubcarrierIndex[kG992p3MaxSpectBoundsUpSize],
						usLog_tss[kG992p3MaxSpectBoundsUpSize];
	short				numUsSubcarrier;
	short				rcvNOMPSDds, rcvMAXNOMPSDds, rcvMAXNOMATPds;
	short				dsSubcarrierIndex[kG992p3MaxSpectBoundsDownSize],
						dsLog_tss[kG992p3MaxSpectBoundsDownSize];
	short				numDsSubcarrier;
	uchar				sizeIDFT, fillIFFT;
	uchar				readsl2Upstream, readsl2Downstream;
	uchar				minDownOverheadDataRate, minUpOverheadDataRate;
	uchar				maxDownSTM_TPSTC, maxDownATM_TPSTC, maxDownPTM_TPSTC;
	uchar				maxUpSTM_TPSTC, maxUpATM_TPSTC, maxUpPTM_TPSTC;

	short				minDownSTM_TPS_TC[4], maxDownSTM_TPS_TC[4],
						minRevDownSTM_TPS_TC[4], maxDelayDownSTM_TPS_TC[4];
	uchar				maxErrorDownSTM_TPS_TC[4], minINPDownSTM_TPS_TC[4];
	short				minUpSTM_TPS_TC[4], maxUpSTM_TPS_TC[4],
						minRevUpSTM_TPS_TC[4], maxDelayUpSTM_TPS_TC[4];
	uchar				maxErrorUpSTM_TPS_TC[4], minINPUpSTM_TPS_TC[4];

	short				maxDownPMS_TC_Latency[4], maxUpPMS_TC_Latency[4];
	short				maxDownR_PMS_TC_Latency[4], maxDownD_PMS_TC_Latency[4];
	short				maxUpR_PMS_TC_Latency[4], maxUpD_PMS_TC_Latency[4];

	short				minDownATM_TPS_TC[4], maxDownATM_TPS_TC[4],
						minRevDownATM_TPS_TC[4], maxDelayDownATM_TPS_TC[4];
	uchar				maxErrorDownATM_TPS_TC[4], minINPDownATM_TPS_TC[4];
	short				minUpATM_TPS_TC[4], maxUpATM_TPS_TC[4],
						minRevUpATM_TPS_TC[4], maxDelayUpATM_TPS_TC[4];
	uchar				maxErrorUpATM_TPS_TC[4], minINPUpATM_TPS_TC[4];

	ushort				minDownPTM_TPS_TC[4], maxDownPTM_TPS_TC[4];
	short				minRevDownPTM_TPS_TC[4], maxDelayDownPTM_TPS_TC[4];
	uchar				maxErrorDownPTM_TPS_TC[4], minINPDownPTM_TPS_TC[4];
	ushort				minUpPTM_TPS_TC[4], maxUpPTM_TPS_TC[4];
	short				minRevUpPTM_TPS_TC[4], maxDelayUpPTM_TPS_TC[4];
	uchar				maxErrorUpPTM_TPS_TC[4], minINPUpPTM_TPS_TC[4];

    ushort              subModePSDMasks;
    uchar               ciPolicy;
	} g992p3DataPumpCapabilities;

#define		kG992p3p5MaxSpectBoundsUpSize		16
#define		kG992p3p5MaxSpectBoundsDownSize		32

/* After 4 consecutive training failures in one startup mode, switch to the other mode */
#define		kDslSingleModeMaxFailure			4
typedef struct
	{
	Boolean				rcvNTREnabled, shortInitEnabled, diagnosticsModeEnabled;

	char				featureSpectrum, featureOverhead;
	char				featureTPS_TC[4], featurePMS_TC[4];

	short				rcvNOMPSDus, rcvMAXNOMPSDus, rcvMAXNOMATPus;
	short				usSubcarrierIndex[kG992p3p5MaxSpectBoundsUpSize],
						usLog_tss[kG992p3p5MaxSpectBoundsUpSize];
	short				numUsSubcarrier;
	short				rcvNOMPSDds, rcvMAXNOMPSDds, rcvMAXNOMATPds;
	short				dsSubcarrierIndex[kG992p3p5MaxSpectBoundsDownSize],
						dsLog_tss[kG992p3p5MaxSpectBoundsDownSize];
	short				numDsSubcarrier;
	uchar				sizeIDFT, fillIFFT;
	uchar				readsl2Upstream, readsl2Downstream;
	uchar				minDownOverheadDataRate, minUpOverheadDataRate;
	uchar				maxDownSTM_TPSTC, maxDownATM_TPSTC, maxDownPTM_TPSTC;
	uchar				maxUpSTM_TPSTC, maxUpATM_TPSTC, maxUpPTM_TPSTC;

	short				minDownSTM_TPS_TC[4], maxDownSTM_TPS_TC[4],
						minRevDownSTM_TPS_TC[4], maxDelayDownSTM_TPS_TC[4];
	uchar				maxErrorDownSTM_TPS_TC[4], minINPDownSTM_TPS_TC[4];
	short				minUpSTM_TPS_TC[4], maxUpSTM_TPS_TC[4],
						minRevUpSTM_TPS_TC[4], maxDelayUpSTM_TPS_TC[4];
	uchar				maxErrorUpSTM_TPS_TC[4], minINPUpSTM_TPS_TC[4];

	short				maxDownPMS_TC_Latency[4], maxUpPMS_TC_Latency[4];
	short				maxDownR_PMS_TC_Latency[4], maxDownD_PMS_TC_Latency[4];
	short				maxUpR_PMS_TC_Latency[4], maxUpD_PMS_TC_Latency[4];

	short				minDownATM_TPS_TC[4], maxDownATM_TPS_TC[4],
						minRevDownATM_TPS_TC[4], maxDelayDownATM_TPS_TC[4];
	uchar				maxErrorDownATM_TPS_TC[4], minINPDownATM_TPS_TC[4];
	short				minUpATM_TPS_TC[4], maxUpATM_TPS_TC[4],
						minRevUpATM_TPS_TC[4], maxDelayUpATM_TPS_TC[4];
	uchar				maxErrorUpATM_TPS_TC[4], minINPUpATM_TPS_TC[4];

	short				minDownPTM_TPS_TC[4], maxDownPTM_TPS_TC[4],
						minRevDownPTM_TPS_TC[4], maxDelayDownPTM_TPS_TC[4];
	uchar				maxErrorDownPTM_TPS_TC[4], minINPDownPTM_TPS_TC[4];
	short				minUpPTM_TPS_TC[4], maxUpPTM_TPS_TC[4],
						minRevUpPTM_TPS_TC[4], maxDelayUpPTM_TPS_TC[4];
	uchar				maxErrorUpPTM_TPS_TC[4], minINPUpPTM_TPS_TC[4];

    ushort              subModePSDMasks;
/* Phy Only section*/
#ifdef GINP_SUPPORT
        uchar                           ginpEnabled;
        ushort                          PTM_TPS_TC_G998P4;
#endif
    uchar               ciPolicy;
	} g992p3PhyDataPumpCapabilities;

typedef struct
        {
        /* G->used in G.inp  P->used in PhyR */
        ushort maxRtMemory;           /* P  unit is 8 bytes */
        ushort maxDtuSize;            /* G  in bytes */
        uchar  INPmax;                /* P  INPmax (symbols) */
        uchar  INPminPhyR;            /* GP INPmin, specific for Shine in PhyR (symbols) */
        uchar  INPminRein;            /* GP INPmin, against REIN (symbols) */
        uchar  minRtxRate;            /* P  min retransmission rate (fix0_8) */
        uchar  minRSoverhead;         /* P  fix0_8 */
        uchar  feHalfRoundtrip;       /* GP symbols */
        uchar  roundtrip;             /* GP symbols */
        uchar  roundtripDtu;          /* G  dtu */
        uchar  fireSupport;           /* P  bit0 -> supported */
        uchar  rxQueueMode;           /* P  bit0 -> support no delay queue
                                            bit1 -> support 'rotating' queue */
        uchar  reinFreq;              /* GP in Hz */
        uchar  nsifVersion;           /* P */
        } FireConfig;

typedef struct
        {
        uint                            rtx_tx;         /* Counter of retransmitted DTUs by the transmitter */
        uint                            rtx_c;          /* Counter of corrected DTUs at receiver */
        uint                            rtx_uc;         /* Counter of uncorrected DTUs at receiver */
        uint                            LEFTRS;         /* Low Error-Free Troughtput Rate Second */
        uint                            errFreeBits;    /* #bits belonging to correct DTU's leaving the Rx PMS-TC * 2^(-16)*/
        uint                            minEFTR;        /* Lowest EFTR observed in the current interval */
        uint                            SEFTR;          /* Severe loss of error-free throughput */
#if defined(GFAST_SUPPORT) || defined(WINNT) || defined(LINUX_DRIVER) || defined(__KERNEL__)
        /* ANDEFTR monitoring counters */
        uint                            ANDEFTR;        /* ANDEFTR for the current second */
        uint                            ANDEFTRDS;      /* ANDEFTRDS for the current second, its value is either 0 or 1 */
        uint                            LANDEFTRS;      /* ANDEFTR for the current second, its value is either 0 or 1 */
#endif
        } GinpCounters;

typedef struct
        {
        uint                           bytesSent;
        uint                           bytesReceived;
        uint                           packetsSent;
        uint                           packetsReceived;
        uint                           messagesSent;
        uint                           messagesReceived;
        uint                           packetsRxCrcErr;
        } GfastEocCounters;

typedef struct
        {
        uint                           cntVecFBSegmentSend;
        uint                           cntVecFBSegmentDrop;
        uint                           cntVecFBMessageSend;
        uint                           cntVecFBMessageDrop;
        } GfastTxVectorFBEocSegment;

#if defined(FIRE_RETRANSMISSION) || defined(FIRE_XMT_6368)
typedef struct
	{
		/* start compatibility mode to E14 FireConfig */
#ifdef GINP_SUPPORT
        ushort                          availableXmtBufferDS;
#else
        uchar                           availableXmtBufferDS;
#endif
        uchar                           INPmaxDS;
        uchar                           INPminPhyR;
        uchar                           INPrein;
        uchar                           minReXmtRateDS;
        uchar                           minRsOverheadDS;
        uchar                           halfRoundtripAtucXmt;
        uchar                           setting;                        /* See bitmap allocation in SoftDsl.h */
		uchar                           RxQmode;
        uchar                           freqRein;
        uchar                           version;
		/* end compatibility mode to E14 FireConfig */
	    Boolean                         validated;                      /* Initialized to false when PhyR spec receive in order to trigger potential spec fix (dependent on CO-CPE SW version) */
        uchar                           halfRoundtripAturXmt, halfRoundtripAturRcv;
        uchar                           halfRoundtripAtucRcv;
#ifdef GINP_SUPPORT
        uchar                           halfRoundtripAtucXmtDtu, halfRoundtripAtucRcvDtu;
        uchar                           invS;                           /* Received from CO in C-MSG1 (decoded value): (1/S)max <= invS */
        uchar                           ciPolicy;                       /* forced to 0? */

#endif
#ifdef BCM6368_SRC
        uchar                           availableXmtBufferUS;           /* Represent in VDSL the CPE total aggregate (xmt & rcv) available memory for Intlv and PhyR */
        uchar                           INPmaxUS;
        uchar                           minReXmtRateUS;
        uchar                           minRsOverheadUS;
#endif
        FireConfig                      fireConfig[2];  /* PhyR configuration for the 2 direction: 0:US 1:DS */
        } g992FireSpecifications;
#endif
#endif /* G992p3/G992p5 */

typedef struct
	{
	dslModulationType	modulations;
	bitMap			auxFeatures;
	bitMap          	features;
	bitMap			demodCapabilities;
	bitMap			demodCapabilities2;
	ushort			noiseMargin;		/* Q4 dB */
#ifdef G992_ATUC
	short			xmtRSf, xmtRS, xmtS, xmtD;
	short			rcvRSf, rcvRS, rcvS, rcvD;
#endif
#if defined(G993)	 /* VDSL2 */
	carrierInfo     	carrierInfoG993p2;
#endif /* END VDSL2 */
#ifdef G992P1_ANNEX_A
	bitMap          	subChannelInfo;
	carrierInfo     	carrierInfoG992p1;
#endif
#ifdef G992P1_ANNEX_B
	bitMap          	subChannelInfoAnnexB;
	carrierInfo     	carrierInfoG992p1AnnexB;
#endif
#ifdef G992_ANNEXC
	bitMap          	subChannelInfoAnnexC;
	carrierInfo     	carrierInfoG992p1AnnexC;
#endif
#if defined(G992P1_ANNEX_I)
	bitMap          	subChannelInfoAnnexI;
	carrierInfo     	carrierInfoG992p1AnnexI;
#endif
#ifdef G992P5
	bitMap          	subChannelInfop5;
	carrierInfo     	carrierInfoG992p5;
#endif
#if defined(G992P2) || (defined(G992P1_ANNEX_A) && defined(G992P1_ANNEX_A_USED_FOR_G992P2))
	carrierInfo     	carrierInfoG992p2;
#endif
	ushort           	maxDataRate;
	uchar           	minDataRate;
#ifdef G992P3
	g992p3DataPumpCapabilities	*carrierInfoG992p3AnnexA;
#endif
#ifdef G992P5
	g992p3DataPumpCapabilities	*carrierInfoG992p5AnnexA;
#endif

#ifdef G993  /* VDSL SUPPORT */
	g993p2DataPumpCapabilities	*carrierInfoG993p2AnnexA;
#endif
	} dslDataPumpCapabilities;

struct __dslCommandStruct;

#ifndef DSLVARS_GLOBAL_REG
typedef	Boolean	(*dslCommandHandlerType)	(void *gDslVars, struct __dslCommandStruct*);
#else
typedef	Boolean	(*dslCommandHandlerType)	(struct __dslCommandStruct*);
#endif /* DSLVARS_GLOBAL_REG */

typedef	struct __dslExtraCfgCommand {
	uint				phyExtraCfg[4];
} dslExtraCfgCommand;

typedef	struct __dslCommandStruct
	{
	dslCommandCode						command;
	union
		{
		int							value;
		Boolean							flag;
		struct
			{
			dslTestCmdType				type;
			union
				{
				struct
					{
					uint				xmtStartTone, xmtNumOfTones;
					uint				rcvStartTone, rcvNumOfTones;
					uchar				*xmtMap, *rcvMap;
					} toneSelectSpec;
				struct
					{
					int				extraPowerRequestQ4dB;
					int				numOfCarriers;
					char				*marginTweakTableQ4dB;
					} marginTweakSpec;
				uint                                   value;
				} param;
			} dslTestSpec;
		struct
			{
			dslDirectionType			direction;
			dslDataPumpCapabilities		capabilities;
			} dslModeSpec;
		struct
			{
			bitMap						setup;
			uint						eyeConstIndex1;
			uint						eyeConstIndex2;
			uint						logTime;
			} dslDiagSpec;
		struct
			{
			void						*descBufAddr;
			uint						bufCnt;
			} dslDiagBufDesc;
		struct
			{
			void						*pBuf;
			uint						bufSize;
			} dslStatusBufSpec;
		struct
			{
			uint						type;
			void						*afeParamPtr;
			uint						afeParamSize;
			void						*imagePtr;
			uint						imageSize;
			} dslAfeTestSpec;
		struct
			{
			uint						type;
			uint						param1;
			uint						param2;
			} dslAfeTestSpec1;
		struct
			{
			uint						plnCmd;
			uint						mgnDescreaseLevelPerBin;
			uint						mgnDescreaseLevelBand;
			uint						nInpBin;
			ushort						*inpBinPtr;
			uint						nItaBin;
			ushort						*itaBinPtr;
			ushort					inmContinueConfig;
			ushort					inmInpEqMode;
			ushort					inmIATO;
			ushort					inmIATS;
			ushort					inmInpEqFormat;
			ushort					inmInpEqScale;
			ushort					inmIATScale;
			ushort					inmBRGN;
			ushort					inmGfastCoCpeSupport;
			} dslPlnSpec;
		struct
			{
			dslLinkLayerType			type;
			bitMap						setup;
			union
				{
				struct
					{
					dataRateMap					rxDataRate;
					dataRateMap					txDataRate;
					int						rtDelayQ4ms;
					uint						rxBufNum;
					uint						rxCellsInBuf;
					uint						rxPacketNum;
					dslFrameHandlerType			rxIndicateHandlerPtr;
					dslFrameHandlerType 		txCompleteHandlerPtr;
					dslPhyInitType				atmPhyInitPtr;
					} atmLinkSpec;
				struct
					{
					dslHeaderHandlerType		rxHeaderHandlerPtr;
					dslRxFrameBufferHandlerType	rxDataHandlerPtr;
					dslTxFrameBufferHandlerType txHandlerPtr;
					} atmPhyLinkSpec;
#ifdef DSL_PACKET
				struct
					{
					uint						rxBufNum;
					uint						rxBufSize;
					uint						rxPacketNum;
					dslFrameHandlerType			rxIndicateHandlerPtr;
					dslFrameHandlerType 		txCompleteHandlerPtr;
					dslPacketPhyInitType		dslPhyInitPtr;
					} dslPacketLinkSpec;
				dslPacketPhyFunctions			dslPacketPhyLinkSpec;
#endif
				struct
					{
					txDataHandlerType			txDataHandlerPtr;
					rxDataHandlerType			rxDataHandlerPtr;
					} nullLinkSpec;
				} param;
			} dslLinkLayerSpec;
#ifdef G997_1
#ifdef G997_1_FRAMER
		struct
			{
			bitMap						setup;
			uint						rxBufNum;
			uint						rxBufSize;
			uint						rxPacketNum;
			dslFrameHandlerType			rxIndicateHandlerPtr;
			dslFrameHandlerType 		txCompleteHandlerPtr;
			} dslG997Cmd;
#endif
		struct
			{
			int	msgId;
			int	msgType;
			char	*dataPtr;
			} dslClearEocMsg;
#endif
		struct
			{
			uint						code;
			uchar						*valuePtr;
			uint						length;
			} dslDataRegister;
		union
			{
			dslStatusHandlerType		statusHandlerPtr;
			dslCommandHandlerType		commandHandlerPtr;
			eyeHandlerType				eyeHandlerPtr;
			logHandlerType				logHandlerPtr;
#if defined(DEBUG_DATA_HANDLER)
            debugDataHandlerType        debugDataHandlerPtr;
#endif
			dslFrameHandlerType			rxIndicateHandlerPtr;
			dslFrameHandlerType			txCompleteHandlerPtr;
			dslDriverCallbackType		driverCallback;
			} handlerSpec;
#if !defined(CHIP_SRC) || defined(DSL_FRAME_FUNCTIONS)
		dslFrameFunctions				DslFunctions;
#endif
		dslOLRMessage					dslOLRRequest;
		dslPwrMessage					dslPwrMsg;
		} param ALIGN_PACKED;
	} dslCommandStruct;

typedef	struct __dslDiagCommandStruct
	{
	dslCommandCode						command;
	union
		{
		int							value;
		Boolean							flag;
		struct
			{
			dslTestCmdType				type;
			union
				{
				struct
					{
					uint				xmtStartTone, xmtNumOfTones;
					uint				rcvStartTone, rcvNumOfTones;
					uint				xmtMap, rcvMap;
					} toneSelectSpec;
				struct
					{
					int				extraPowerRequestQ4dB;
					int				numOfCarriers;
					uint				marginTweakTableQ4dB;
					} marginTweakSpec;
				uint					value;
				} param;
			} dslTestSpec;
		struct
			{
			dslDirectionType			direction;
			dslDataPumpCapabilities		capabilities;
			} dslModeSpec;
		struct
			{
			bitMap						setup;
			uint						eyeConstIndex1;
			uint						eyeConstIndex2;
			uint						logTime;
			} dslDiagSpec;
		struct
			{
			uint						descBufAddr;
			uint						bufCnt;
			} dslDiagBufDesc;
		struct
			{
			uint						pBuf;
			uint						bufSize;
			} dslStatusBufSpec;
		struct
			{
			uint						type;
			uint						afeParamPtr;
			uint						afeParamSize;
			uint						imagePtr;
			uint						imageSize;
			} dslAfeTestSpec;
		struct
			{
			uint						type;
			uint						param1;
			uint						param2;
			} dslAfeTestSpec1;
		struct
			{
			uint						plnCmd;
			uint						mgnDescreaseLevelPerBin;
			uint						mgnDescreaseLevelBand;
			uint						nInpBin;
			uint						inpBinPtr;	/* ushort pointer */
			uint						nItaBin;
			uint						itaBinPtr;	/* ushort pointer */
			ushort					inmContinueConfig;
			ushort					inmInpEqMode;
			ushort					inmIATO;
			ushort					inmIATS;
			ushort					inmInpEqFormat;
			} dslPlnSpec;
		struct
			{
			dslLinkLayerType			type;
			bitMap						setup;
			union
				{
				struct
					{
					dataRateMap					rxDataRate;
					dataRateMap					txDataRate;
					int						rtDelayQ4ms;
					uint						rxBufNum;
					uint						rxCellsInBuf;
					uint						rxPacketNum;
					dslFrameHandlerType			rxIndicateHandlerPtr;
					dslFrameHandlerType 		txCompleteHandlerPtr;
					dslPhyInitType				atmPhyInitPtr;
					} atmLinkSpec;
				struct
					{
					dslHeaderHandlerType		rxHeaderHandlerPtr;
					dslRxFrameBufferHandlerType	rxDataHandlerPtr;
					dslTxFrameBufferHandlerType txHandlerPtr;
					} atmPhyLinkSpec;
#ifdef DSL_PACKET
				struct
					{
					uint						rxBufNum;
					uint						rxBufSize;
					uint						rxPacketNum;
					dslFrameHandlerType			rxIndicateHandlerPtr;
					dslFrameHandlerType 		txCompleteHandlerPtr;
					dslPacketPhyInitType		dslPhyInitPtr;
					} dslPacketLinkSpec;
				dslPacketPhyFunctions			dslPacketPhyLinkSpec;
#endif
				struct
					{
					txDataHandlerType			txDataHandlerPtr;
					rxDataHandlerType			rxDataHandlerPtr;
					} nullLinkSpec;
				} param;
			} dslLinkLayerSpec;
#ifdef G997_1
#ifdef G997_1_FRAMER
		struct
			{
			bitMap						setup;
			uint						rxBufNum;
			uint						rxBufSize;
			uint						rxPacketNum;
			dslFrameHandlerType			rxIndicateHandlerPtr;
			dslFrameHandlerType 		txCompleteHandlerPtr;
			} dslG997Cmd;
#endif
		struct
			{
			int	msgId;
			int	msgType;
			uint	dataPtr;
			} dslClearEocMsg;
#endif
		struct
			{
			uint						code;
			uint						valuePtr;
			uint						length;
			} dslDataRegister;
		union
			{
			dslStatusHandlerType		statusHandlerPtr;
			dslCommandHandlerType		commandHandlerPtr;
			eyeHandlerType				eyeHandlerPtr;
			logHandlerType				logHandlerPtr;
#if defined(DEBUG_DATA_HANDLER)
			debugDataHandlerType		debugDataHandlerPtr;
#endif
			dslFrameHandlerType			rxIndicateHandlerPtr;
			dslFrameHandlerType			txCompleteHandlerPtr;
			dslDriverCallbackType		driverCallback;
			} handlerSpec;
#if !defined(CHIP_SRC) || defined(DSL_FRAME_FUNCTIONS)
		dslFrameFunctions				DslFunctions;
#endif
		dslOLRMessage					dslOLRRequest;
		dslPwrMessage					dslPwrMsg;
		} param ALIGN_PACKED;
	} dslDiagCommandStruct;



typedef struct
	{
	dslCommandHandlerType			linkCommandHandlerPtr;
	timerHandlerType				linkTimerHandlerPtr;
	dslLinkCloseHandlerType			linkCloseHandlerPtr;

	dslFrameHandlerType				linkSendHandlerPtr;
	dslFrameHandlerType				linkReturnHandlerPtr;

	dslVcAllocateHandlerType		linkVcAllocateHandlerPtr;
	dslVcFreeHandlerType			linkVcFreeHandlerPtr;
	dslVcActivateHandlerType		linkVcActivateHandlerPtr;
	dslVcDeactivateHandlerType		linkVcDeactivateHandlerPtr;
	dslVcConfigureHandlerType		linkVcConfigureHandlerPtr;

	dslLinkVc2IdHandlerType			linkVc2IdHandlerPtr;
	dslLinkVcId2VcHandlerType		linkVcId2VcHandlerPtr;
	dslGetFramePoolHandlerType		linkGetFramePoolHandlerPtr;

#ifndef ADSLCORE_ONLY
	dslHeaderHandlerType			linkRxCellHeaderHandlerPtr;
	dslRxFrameBufferHandlerType		linkRxCellDataHandlerPtr;
	dslTxFrameBufferHandlerType		linkTxCellHandlerPtr;
#endif

	txDataHandlerType				linkTxDataHandlerPtr;
	rxDataHandlerType				linkRxDataHandlerPtr;
	} linkLayerFunctions;

#ifndef ADSLCORE_ONLY

#define	 LinkLayerAssignFunctions( var, name_prefix )	do {					\
	(var).linkCommandHandlerPtr = name_prefix##CommandHandler;					\
	(var).linkTimerHandlerPtr	= name_prefix##TimerHandler;					\
	(var).linkCloseHandlerPtr	= name_prefix##CloseHandler;					\
																				\
	(var).linkSendHandlerPtr	= name_prefix##SendFrameHandler;				\
	(var).linkReturnHandlerPtr	= name_prefix##ReturnFrameHandler;				\
																				\
	(var).linkVcAllocateHandlerPtr		= name_prefix##VcAllocateHandler;		\
	(var).linkVcFreeHandlerPtr			= name_prefix##VcFreeHandler;			\
	(var).linkVcActivateHandlerPtr		= name_prefix##VcActivateHandler;		\
	(var).linkVcDeactivateHandlerPtr	= name_prefix##VcDeactivateHandler;		\
	(var).linkVcConfigureHandlerPtr		= name_prefix##VcConfigureHandler;		\
																				\
	(var).linkVc2IdHandlerPtr			= name_prefix##Vc2IdHandler;			\
	(var).linkVcId2VcHandlerPtr			= name_prefix##VcId2VcHandler;			\
	(var).linkGetFramePoolHandlerPtr	= name_prefix##GetFramePoolHandler;		\
																				\
	(var).linkRxCellHeaderHandlerPtr	= name_prefix##RxCellHeaderHandler;		\
	(var).linkRxCellDataHandlerPtr		= name_prefix##RxCellDataHandler;		\
	(var).linkTxCellHandlerPtr			= name_prefix##TxCellHandler;			\
																				\
	(var).linkTxDataHandlerPtr	= name_prefix##TxDataHandler;					\
	(var).linkRxDataHandlerPtr	= name_prefix##RxDataHandler;					\
} while (0)

#else

#define	 LinkLayerAssignFunctions( var, name_prefix )	do {					\
	(var).linkCommandHandlerPtr = name_prefix##CommandHandler;					\
	(var).linkTimerHandlerPtr	= name_prefix##TimerHandler;					\
	(var).linkCloseHandlerPtr	= name_prefix##CloseHandler;					\
																				\
	(var).linkSendHandlerPtr	= name_prefix##SendFrameHandler;				\
	(var).linkReturnHandlerPtr	= name_prefix##ReturnFrameHandler;				\
																				\
	(var).linkVcAllocateHandlerPtr		= name_prefix##VcAllocateHandler;		\
	(var).linkVcFreeHandlerPtr			= name_prefix##VcFreeHandler;			\
	(var).linkVcActivateHandlerPtr		= name_prefix##VcActivateHandler;		\
	(var).linkVcDeactivateHandlerPtr	= name_prefix##VcDeactivateHandler;		\
	(var).linkVcConfigureHandlerPtr		= name_prefix##VcConfigureHandler;		\
																				\
	(var).linkVc2IdHandlerPtr			= name_prefix##Vc2IdHandler;			\
	(var).linkVcId2VcHandlerPtr			= name_prefix##VcId2VcHandler;			\
	(var).linkGetFramePoolHandlerPtr	= name_prefix##GetFramePoolHandler;		\
																				\
	(var).linkTxDataHandlerPtr	= name_prefix##TxDataHandler;					\
	(var).linkRxDataHandlerPtr	= name_prefix##RxDataHandler;					\
} while (0)

#endif

typedef struct
	{
	dslFrameHandlerType				rxIndicateHandlerPtr;
	dslFrameHandlerType				txCompleteHandlerPtr;
	dslStatusHandlerType			statusHandlerPtr;
	} upperLayerFunctions;

#ifdef VECTORING
#ifndef VDSLTONEGROUP
#define VDSLTONEGROUP
typedef struct VdslToneGroup {
	unsigned short endTone;
	unsigned short startTone;
} VdslToneGroup;
#endif
#ifndef VDSLMSGTONEGROUPDESC
#define VDSLMSGTONEGROUPDESC

#define VDSL_MSG_MAX_NO_OF_TONE_GROUPS_IN_BANDS_DESC     32

typedef struct VdslMsgToneGroupDescriptor VdslMsgToneGroupDescriptor;
struct VdslMsgToneGroupDescriptor
{
  unsigned char  noOfToneGroups;
  unsigned char  reserved;
  VdslToneGroup toneGroups[VDSL_MSG_MAX_NO_OF_TONE_GROUPS_IN_BANDS_DESC];
};
#endif
#ifndef VECTDATAPHY
#define VECTDATAPHY
#define LOG_PILOT_SEQUENCE_LEN 10
#define PILOT_SEQUENCE_LEN     (1<<LOG_PILOT_SEQUENCE_LEN)
typedef struct PilotSequence {
  unsigned short firstSync;                          /* Position in the sequence of
                                               first sync symbol in showtime
                                               (Unused at the CO side) */
  unsigned char bitsPattern[PILOT_SEQUENCE_LEN>>3]; /* One sign bit per modulation */
  unsigned short pilotSeqLengthInBytes;
} PilotSequence;

typedef struct G993p5PilotSequence {
  unsigned short firstSync;                          /* Position in the sequence of
                                               first sync symbol in showtime
                                               (Unused at the CO side) */
  unsigned char bitsPattern[PILOT_SEQUENCE_LEN>>3]; /* One sign bit per modulation */
  unsigned short pilotSeqLengthInBytes;
} G993p5PilotSequence;

#define LOG_G993P5_PILOT_SEQUENCE_LEN 9
#define G993P5_PILOT_SEQUENCE_LEN (1<<LOG_G993P5_PILOT_SEQUENCE_LEN)

typedef struct FourBandsDescriptor{
  unsigned char  noOfToneGroups;
#ifdef BCM6368_DP
  unsigned char  reserved; /* used for bandplan type indication (negotiated or physical) */
#else
  unsigned char  noCheck; /* bypass validation logic (should only be used in lineTest and loopback modes) */
#endif
  VdslToneGroup toneGroups[4];
} FourBandsDescriptor;

typedef struct VectDataPhy {
	PilotSequence pilotSequence;
	FourBandsDescriptor vectoringBandPlan;
	unsigned short syncOffset;
} VectDataPhy;
#endif /* VECTDATAPHY */
/* Descriptor for vectored band, only 8 bands used in O-TA-UPDATE and in EOC command */
#define VDSL_VECTORED_BANDS_MAX_NO_OF_TONES_IN_BANDS_DESC 8
typedef struct VdslVectoredBandsDescriptor VdslVectoredBandsDescriptor;
struct VdslVectoredBandsDescriptor
{
  unsigned char  noOfTonesGroups;
  unsigned char  reserved;
  VdslToneGroup toneGroups[VDSL_VECTORED_BANDS_MAX_NO_OF_TONES_IN_BANDS_DESC];
};

/* Structure that contains the info of the vectoring back channel */
typedef struct VectBackChannelParams VectBackChannelParams;
struct  VectBackChannelParams
{
  unsigned short  log2M;                               /* Subsampling of the band                 */
  unsigned short  floatFormat;                         /* 1:floating format 0: fixed point format */
  unsigned short  nBands;                              /* nBands (max 8) */
  short   lastReportedBand;                    /* last reported band (set to -1 if nothing to be reported)*/
};
#endif

typedef struct {
  ushort    upbokle;
  uint      actualCE;
  uint      actsnrmodeDS;
  uint      actsnrmodeUS;
  int       actpsdDS;
  int       actpsdUS;
  uint      qlnmtDS;
  uint      qlnmtUS;
  uint      snrmtDS;
  uint      snrmtUS;
  uint      hlogmtDS;
  uint      hlogmtUS;
  uint      hloggDS;
  uint      hloggUS;
} TR98ParamsVarsStruct;

/*
 * Debug data
 */
#define	kDslFirstDebugData					1000
#define	kDslXmtPerSymTimeCompData			(kDslFirstDebugData + 0)
#define	kDslRcvPerSymTimeCompData			(kDslFirstDebugData + 1)
#define	kDslXmtAccTimeCompData				(kDslFirstDebugData + 2)
#define	kDslRcvAccTimeCompData				(kDslFirstDebugData + 3)
#define	kDslRcvPilotToneData				(kDslFirstDebugData + 4)
#define	kDslTEQCoefData						(kDslFirstDebugData + 5)
#define	kDslTEQInputData					(kDslFirstDebugData + 6)
#define	kDslTEQOutputData					(kDslFirstDebugData + 7)
#define	kDslRcvFFTInputData					(kDslFirstDebugData + 8)
#define	kDslRcvFFTOutputData				(kDslFirstDebugData + 9)
#define	kDslRcvCarrierSNRData				(kDslFirstDebugData + 10)
#define	kDslXmtToneOrderingData				(kDslFirstDebugData + 11)
#define	kDslRcvToneOrderingData				(kDslFirstDebugData + 12)
#define	kDslXmtGainData						(kDslFirstDebugData + 13)
#define	kDslRcvGainData						(kDslFirstDebugData + 14)
#define	kDslMseData							(kDslFirstDebugData + 15)
#define	kDslFEQOutErrData					(kDslFirstDebugData + 16)
#define kDslFEQCoefData 					(kDslFirstDebugData + 17)
#define kDslShowtimeMseData					(kDslFirstDebugData + 18)
#define kDslTimeEstimationHWPhaseTweak		(kDslFirstDebugData + 24)
#define	kDslSlicerInput						(kDslFirstDebugData + 40)
#define	kDslXmtConstellations				(kDslFirstDebugData + 41)
#define kDslSnr1ShiftData					(kDslFirstDebugData + 50)
#define kDslSnr1InputData					(kDslFirstDebugData + 51)
#define kDslSnr1ReverbAvgData				(kDslFirstDebugData + 52)
#define kDslAnnexCFextSnrData				(kDslFirstDebugData + 53)
#define kDslAnnexCNextSnrData				(kDslFirstDebugData + 54)
#define	kG994p1OutputXmtSample				(kDslFirstDebugData + 100)
#define	kG994p1OutputMicroBit				(kDslFirstDebugData + 101)
#define	kG994p1OutputBit					(kDslFirstDebugData + 102)
#define	kG994p1OutputTimer					(kDslFirstDebugData + 103)

/****************************************************************************/
/*	2.	Constant definitions.												*/
/*																			*/
/*	2.1	Defininitive constants												*/
/****************************************************************************/

/* dslDirectionType */

#define	kATU_C		0
#define	kATU_R		1

/* ATM setup maps	*/

#define	kAtmCallMgrEnabled			0x00000001		/* Bit 0  */
#define	kAtmAAL1FecEnabledMask		0x00000006		/* Bit 1  */
#define	kAtmAAL1HiDelayFecEnabled	0x00000002		/* Bit 2  */
#define	kAtmAAL1LoDelayFecEnabled	0x00000004		/* Bit 3  */

/* dslLinkLayerType */

#define kNoDataLink			0
#define kAtmLink			0x00000001
#define kAtmPhyLink			0x00000002
#define kDslPacketLink		0x00000003
#define kDslPacketPhyLink	0x00000004
#define kPtmSyncLink		0x00000005

/* dslModulationType */
#define	kNoCommonModulation	0x00000000
#define	kG994p1				0x00000020				/* G.994.1 or G.hs */
#define	kT1p413				0x00000040				/* T1.413 handshaking */
#define	kG992p1AnnexA		0x00000001				/* G.992.1 or G.dmt Annex A */
#define	kG992p1AnnexB		0x00000002				/* G.992.1 or G.dmt Annex B */
#define	kG992p1AnnexC		0x00000004				/* G.992.1 or G.dmt Annex C */
#define	kG992p2AnnexAB		0x00000008				/* G.992.2 or G.lite Annex A/B */
#define	kG992p2AnnexC		0x00000010				/* G.992.2 or G.lite Annex C */
#define	kG992p3AnnexA		0x00000100				/* G.992.3 or G.DMTbis Annex A */
#define	kG992p3AnnexB		0x00000200				/* G.992.3 or G.DMTbis Annex A */
#define	kG992p1AnnexI		0x00000400				/* G.992.1 Annex I */
#define kG992p3AnnexJ       0x00000800
#define kG992p5AnnexA       0x00010000              /* G.992.5 Annex A */
#define kG992p5AnnexB       0x00020000              /* G.992.5 Annex B */
#define kG992p5AnnexI       0x00040000              /* G.992.5 Annex I */
#define kG992p3AnnexM       0x00080000              /* G.992.3 Annex M */
#define kG992p5AnnexM       0x01000000              /* G.992.5 Annex M */
#define kG992p5AnnexJ       0x00100000              /* G.992.5 Annex J */
#define kG993p2AnnexA       0x02000000              /* G.993.2 Annex A */
#define kGfastAnnexA        0x04000000              /* G.fast Annex A */

#define	kG992p3AllModulations	(kG992p3AnnexA | kG992p3AnnexB | kG992p3AnnexJ | kG992p3AnnexM)
#define	kG992p5AllModulations	(kG992p5AnnexA | kG992p5AnnexB | kG992p5AnnexI | kG992p5AnnexJ | kG992p5AnnexM)

/* demodCapabilities bitmap */
#define	kEchoCancellorEnabled					0x00000001
#define	kSoftwareTimeErrorDetectionEnabled		0x00000002
#define	kSoftwareTimeTrackingEnabled			0x00000004
#define kDslTrellisEnabled			            0x00000008
#define	kHardwareTimeTrackingEnabled			0x00000010
#define kHardwareAGCEnabled						0x00000020
#define kDigitalEchoCancellorEnabled			0x00000040
#define kReedSolomonCodingEnabled				0x00000080
#define kAnalogEchoCancellorEnabled				0x00000100
#define	kT1p413Issue1SingleByteSymMode			0x00000200
#define	kDslAturXmtPowerCutbackEnabled			0x00000400
#ifdef G992_ANNEXC_LONG_REACH
#define kDslAnnexCPilot48                       0x00000800
#define kDslAnnexCReverb33_63                   0x00001000
#else
#define	kDslDisableTxFilter7ForSpecialCNXT		0x00000800	/* Disable tx fitler Id 4 for CNXT coVendorFirmwareID == 0x39 and coVendorSpecificInfo == 0x10 */
#define kDslEnableHighPrecisionTxBitswap		0x00001000
#endif
#ifdef G992_ANNEXC
#define kDslCentilliumCRCWorkAroundEnabled		0x00002000
#else
#define kDslEnableRoundUpDSLoopAttn		        0x00002000
#endif
#define	kDslBitSwapEnabled						0x00004000
#define	kDslADILowRateOptionFixDisabled			0x00008000
#define	kDslAnymediaGSPNCrcFixEnabled			0x00010000
#define	kDslMultiModesPreferT1p413				0x00020000
#define	kDslT1p413UseRAck1Only					0x00040000
#define	kDslUE9000ADI918FECFixEnabled			0x00080000
#define	kDslG994AnnexAMultimodeEnabled			0x00100000
#define	kDslATUCXmtPowerMinimizeEnabled			0x00200000
#define	kDropOnDataErrorsDisabled			    0x00400000
#define	kDslSRAEnabled						    0x00800000

#define	kDslT1p413HigherToneLevelNeeded			0x01000000
#define	kDslT1p413SubsampleAlignmentEnabled		0x02000000
#define	kDslT1p413DisableUpstream2xIfftMode		0x04000000

/* test mode related demodCapabilities, for internal use only */
#define	kDslTestDemodCapMask					0xF8000000
#define	kDslSendReverbModeEnabled				0x10000000
#define	kDslSendMedleyModeEnabled				0x20000000
#define	kDslAutoRetrainDisabled					0x40000000
#define kDslPllWorkaroundEnabled                0x80000000
#define kDslAfeLoopbackModeEnabled              0x08000000

/* demodCapabilities bitmap2 */

/* only in Annex C */
#define kDslAnnexCProfile1	    			    0x00000001
#define kDslAnnexCProfile2	    			    0x00000002
#define kDslAnnexCProfile3	    			    0x00000004
#define kDslAnnexCProfile4	    			    0x00000008
#define kDslAnnexCProfile5	    			    0x00000010
#define kDslAnnexCProfile6	    			    0x00000020
#define kDslAnnexCPilot64			   	        0x00000040
#define kDslAnnexCPilot48                       0x00000080
#define kDslAnnexCPilot32			   	        0x00000100
#define kDslAnnexCPilot16			   	        0x00000200
#define kDslAnnexCA48B48			   		    0x00000400
#define kDslAnnexCA24B24			    	    0x00000800
#define kDslAnnexCReverb33_63                   0x00001000
#define kDslAnnexCCReverb6_31	  		        0x00002000

#define kDslAnnexIShapedSSVI                    0x00004000
#define kDslAnnexIFlatSSVI                      0x00008000

#define kDslAnnexIPilot64			   	        0x00010000
#define kDslAnnexIA48B48			   		    0x00020000
#define kDslAnnexIPilot128			   	        0x00040000
#define kDslAnnexIPilot96			   	        0x00080000

/* Only in Annex A */
/* Bits 0 to 8 : Annex M submask control */
/* bit 9 : enable custom mode            */
#define kDslAnnexMcustomModeShift               9
#define kDslAnnexMcustomMode                    (1<<kDslAnnexMcustomModeShift)
/* Bits 10 to 15 : When kDslRetrainOnSesEnabled is enabled, this is the amount
 * of time (in units of five seconds) with continuous SES before the modem
 * retrains.
 */
#define kDslSesRetrainThresholdMask				0x0000FC00
#define kDslSesRetrainThresholdShift			10
#define kDslDisableL2                           0x00010000
#define kDigEcShowtimeUpdateDisabled            0x00020000
#define kDigEcShowtimeFastUpdateDisabled        0x00040000
#define kDslRetrainOnSesEnabled                 0x00080000
#define kDsl24kbyteInterleavingEnabled          0x00100000
#define kDslRetrainOnDslamMinMargin             0x00200000
#define kDslFireDsSupported                     0x00400000
#define kDslFireUsSupported                     0x00800000
#define kDslDisableNitro                        0x01000000
#define kDslReinCounterMeasureControl           0x02000000
#define kDslPhyRDelayRxQSupported               0x04000000
#define kDslPhyRNoDelayRxQSupported             0x08000000
#define kDslForceFastBS						    0x80000000
#define kDslDemod2Reserved						0x70000000

/* Features bitmap */
#define	kG992p2RACK1   						    0x00000001
#define	kG992p2RACK2							0x00000002
#define	kG992p2DBM								0x00000004
#define	kG992p2FastRetrain						0x00000008
#define	kG992p2RS16								0x00000010
#define	kG992p2ClearEOCOAM						0x00000020
#define	kG992NTREnabled							0x00000040
#define	kG992p2EraseAllStoredProfiles			0x00000080
#define kG992p2FeaturesNPar2Mask                0x0000003B
#define kG992p2FeaturesNPar2Shift                        0

#define kG992p1RACK1                            0x00000100
#define kG992p1RACK2                            0x00000200
#define kG992p1STM                              0x00000800
#define kG992p1ATM                              0x00001000
#define	kG992p1ClearEOCOAM						0x00002000
#define kG992p1FeaturesNPar2Mask                0x00003B00
#define kG992p1FeaturesNPar2Shift                        8
#define kG992p1DualLatencyUpstream				0x00004000
#define kG992p1DualLatencyDownstream			0x00008000
#define kG992p1HigherBitRates					0x40000000

#if defined(G992P1_ANNEX_I)
#define kG992p1HigherBitRates1over3				0x80000000
#define kG992p1AnnexIShapedSSVI                 0x00000001
#define kG992p1AnnexIFlatSSVI                   0x00000002
#define kG992p1AnnexIPilotFlag			   		0x00000008
#define kG992p1AnnexIPilot64			   		0x00000001
#define kG992p1AnnexIPilot128			   		0x00000004
#define kG992p1AnnexIPilot96			   		0x00000008
#define kG992p1AnnexIPilotA48B48                0x00000010
#endif

#define kG992p1AnnexBRACK1                      0x00010000
#define kG992p1AnnexBRACK2                      0x00020000
#define kG992p1AnnexBUpstreamTones1to32			0x00040000
#define kG992p1AnnexBSTM                        0x00080000
#define kG992p1AnnexBATM                        0x00100000
#define	kG992p1AnnexBClearEOCOAM				0x00200000
#define kG992p1AnnexBFeaturesNPar2Mask          0x003F0000
#define kG992p1AnnexBFeaturesNPar2Shift                 16

#define kG992p1AnnexCRACK1                      0x01000000
#define kG992p1AnnexCRACK2                      0x02000000
#define kG992p1AnnexCDBM						0x04000000
#define kG992p1AnnexCSTM                        0x08000000
#define kG992p1AnnexCATM                        0x10000000
#define	kG992p1AnnexCClearEOCOAM				0x20000000
#define kG992p1AnnexCFeaturesNPar2Mask          0x3F000000
#define kG992p1AnnexCFeaturesNPar2Shift                 24

#define kG992p1HigherBitRates1over3				0x80000000

/* auxFeatures bitmap */
#define	kG994p1PreferToExchangeCaps				0x00000001
#define	kG994p1PreferToDecideMode				0x00000002
#define	kG994p1PreferToMPMode				    0x00000004
#define	kAfePwmSyncClockShift					3
#define	kAfePwmSyncClockMask					(0xF << kAfePwmSyncClockShift)
#define	AfePwmSyncClockEnabled(val)				(((val) & kAfePwmSyncClockMask) != 0)
#define	AfePwmGetSyncClockFreq(val)				((((val) & kAfePwmSyncClockMask) >> kAfePwmSyncClockShift) - 1)
#define	AfePwmSetSyncClockFreq(val,freq)		((val) |= ((((freq)+1) << kAfePwmSyncClockShift) & kAfePwmSyncClockMask))
#define kDslAtmBondingSupported                 0x00000080
#define kDslPtmBondingSupported                 0x00000100
#define kDslT1413DisableCACT1                   0x00000200  /* Disable C-ACT1 C-ACT2 ping pong for Telefonica */
#define kDslAnnexJhandshakeB43J43Toggle         0x00000400

#define kDslG994p1DisableA43C                   0x00000800  /* Disable A43C set for Cincinati Bell */
#define kDslG992FTFeatureBit                    0x00001000  /* Enable FT specific feature for HBI DSLAMS */
#define kDslGspn13BitLimitFixDisable            0x00002000  /* disable fix (sending blank vendorId) that achieves bimax of 15 in US */

#define kDslMonitorToneFeatureDisable           0x00004000
#define kDslG992p5MinimizeDSDelay               0x00008000 /* If enabled, will reduce the achieved delay in DS to provide more room for SRA */
#define kDslG992p5MonitorNOMPSDDs               0x00010000 /* If enabled, CPE will monitor MAXNOPSD to reduce gi. Not recommended for TR-100 type of tests */
#define kDslGinpDsSupported                     0x00020000
#define kDslGinpUsSupported                     0x00040000
#define kDslEnableATTCompatibility              0x00080000 /* If enabled, ADSL modem will behave similar to 2Wire's 2701 in a set of situations for AT&T deployment */
														   /* 1) Prefer G.DMT HS over T1.413 HS */
                                                           /* 2) Prefer ADSL1 over ADSL2 AnnexL in Shortloops in an AT&T profile that has Annex L and ADSL1 enabled and ADSL2+/ADSL2 disabled */
#define kDslEnableENRMeasureForAdsl2            0x00100000 /* If enabled, allow ENR measurement during R-Lineprobe (Adsl2) */
#define kDslG992BTFeatureBit                    0x00200000 /* If enabled, CPE will monitor REIN condition and retrain to adjust AGC/Pilot to pass REIN test */
#define kDslEnableSpecialCO4AdslFilt            0x00400000 /* If enabled, CPE will use a Tx filter against IKNS DSLAMs that works better for CO4  */
                                                           /* It would have been better to use vendor specific info to distinguish, but IKNS does not send any*/
#define kDslEnableCiPolicyTwo                   0x00800000
#define kDslPTMPreemptionDisable	              0x01000000 /* If set, the modem will not declare support of PTM Preemption in HS for Adsl2/2+ or in Training for Vdsl2 */
#define kDslG992p5MinDsPCB                      0x02000000 /* If enabled, will reduce the PCB in DS to provide more noise immunity */
#define kDslDisableRNC                          0x04000000 /* Disable RNC (for RNC-capable builds) */
#define	kDslSOSEnabled                          0x08000000
#define	kDslROCEnabled                          0x10000000
#define kDslPhyRAllowNoMinRSoverhead            0x20000000
#define	kDslPhyEnableShortCLR                   0x40000000
#define	kDslPhyEnable6328ForGVT					0x80000000

/* phyExtraCfg[0] bitmap */
/* Remove unused driver bits */
/* #define kPhyCfg1EnableImpFiltInArqMode         0x00000001   obsolete */
/* #define kPhyCfg1EnableImpFiltInFecMode         0x00000002   obsolete */
#define kPhyCfg1EnableBelgacomInterop          0x00000004
#define kPhyCfg1ExternalBondingDiscovery       0x00000008
#define kPhyCfg1ExternalBondingSlave           0x00000010
#define	kPhyCfg1EnableMoniteringToneForL2mode	0x00000020
#define kPhyCfg1EnableTelecomPolandInterop     0x00000040
#define kPhyCfg1DisableTrainingImpGating       0x00000080   /* disables training impulse gating when set */
#define kPhyCfg1DisableMeqInRein               0x00000100   /* disables margin equalization under high REIN event */
#define kPhyCfg1EnableAutonomousINP            0x00000200   /* allow cpe to boost the current INP setting */
#define kPhyCfg1EnableNegMgnAdjustment         0x00000400   /* allow ADSL negative margin adjustment option for very low noise loops */
#define	kPhyCfg1DisableEnhancedL2Bit           0x00000800	/* if enabled, limit the total L2 power to maxL2PCB set by DSLAM at L2 request */
#define kPhyCfg1IncludeAdsl1InBondingCLR       0x00001000   /* if enabled, include ADSL1 in Bonding CLR */
#define kPhyCfg1IncludeCXSYOAMInterop          0x00002000   /* If enabled, will enable HDLC control bit work around against CXSY */
#define kPhyCfg1EnableDSFFT512Interop          0x00004000   /* If enabled, force downstream FFT size of 512 for G.DMT and ADSL2 */
#define kPhyCfg1EnableHlogInterpolation        0x00008000   /* If enabled, interpolate HAM band notching when SNR is below 6dB */
#define kPhyCfg1DisableRxDelayManagement       0x00010000   /* disables rx delay management when set */
#define	kPhyCfg1V43PSDlevel1                   0x00020000   /* 01 with 10dB down */
#define	kPhyCfg1V43PSDlevel2                   0x00040000   /* 10 with 15dB down */
#define	kPhyCfg1V43PSDlevel3                   0x00060000   /* 11 with 20dB down, also used as mask for bit 17 and 18 */
#define	kPhyCfg1ReduceCToneDetectionDelay      0x00080000   /* Reduce C-Tone detection delay after xmt R-Tone-Req for Anymedia DSLAM */
#define	kPhyCfg1G992DTFeatureBit               0x00100000   /* Enable DT Lab ADSL Annex B HIGH Noise feature */
#define	kPhyCfg1G992DisableCNXTINPmin2         0x00200000   /* Disable the feature to set INPmin to 2 from 0 for maxD>7 of CNXT DSLAMs */
#define kPhyCfg1DisableHlogMasking             0x00400000	/* Disable the feature to limit Hlog to -96.3dB */
#define kPhyCfg1DetectLUA2P72HBI               0x00800000   /* restore proper detection of CNXT A2P72_HBI instead of H563ADEF */


#define kPhyCfg1LosDropTimingBit0              0x01000000
#define kPhyCfg1LosDropTimingBit1              0x02000000
#define kPhyCfg1LosDropTimingBit2              0x04000000
#define kPhyCfg1LosDropTimingBit3              0x08000000
#define kPhyCfg1LosDropTimingBit4              0x10000000
#define kPhyCfg1LosDropTimingBit5              0x20000000

#define kPhyCfg1LosDropTimingMask              (kPhyCfg1LosDropTimingBit0|kPhyCfg1LosDropTimingBit1|kPhyCfg1LosDropTimingBit2|kPhyCfg1LosDropTimingBit3|kPhyCfg1LosDropTimingBit4|kPhyCfg1LosDropTimingBit5)
#define kPhyCfg1EnableATTNDRframingAllTargets  0x40000000   /* Enable framing inclusive attainable data rate computation for all modes */
#define kPhyCfg1AlternateTogglingStartPhase    0x80000000

/* phyExtraCfg[1] bitmap */
#define kPhyCfg2AltBondingRSForHighNoise       0x00000001
#define kPhyCfg2DetectTrainingOnXtalk          0x00000002
#define kPhyCfg2DisablePtmNonBondingConnection 0x00000004
#define kPhyCfg2EnableLabTestModeInBonding     0x00000008
#define kPhyCfg2TxPafEnabled                   0x00000010
#define kPhyCfg2SendWIREtoGspnG992p3           0x00000020
#define kPhyCfg2EnbleReferenceNoiseCancelling  0x00000040  /* Enable Reference Noise cancelling for BCM63138/148 */
#define kPhyCfg2EnbleSTGRTSSIhandling          0x00000080  /* Enable TSSI handling for STGR_LIM_A2P_48_HB firmware TAO 9.14.2*/
#define kPhyCfg2EnableGfastB2BMode             0x00000100  /* Enable gfast back-to-back mode in the cpe image */
#define kPhyCfg2EnableGfastVdslMMode           0x00000200  /* Enable gfast vdsl multimode enable */
#define kPhyCfg2EnableGfastVdslMMTimeOut0      0x00000400  /* gfast vdsl multimode timeout 0*/
#define kPhyCfg2EnableGfastVdslMMTimeOut1      0x00000800  /* gfast vdsl multimode timeout 1*/
#define kPhyCfg2DisableGfastTIGA               0x00001000
#define kPhyCfg2DisableGfastA43ToneSet         0x00002000
#define kPhyCfg2DisableGfastB43ToneSet         0x00004000
#define kPhyCfg2DisableGfastSRA                0x00008000
#define kPhyCfg2DisableAELEM                   0x00010000  /* Disable VDSL alternative electrical length estimation */
#define kPhyCfg2ForceSRAtoUseTargetMargin      0x00020000  /* Use the existing target margin for SRA reconfiguration (not dnshiftMargin+1dB and upshiftMargin-1 dB */
#define kPhyCfg2DisableGfastRPA                0x00040000  /* Disable GFAST RPA */
#define kPhyCfg2DisableGfastFRA                0x00080000  /* Disable GFAST FRA */
#define kPhyCfg2DisableGfastIdleSymbols        0x00100000
#define kPhyCfg2DisableRefErrSample			       0x00200000  /* To Enable reference error sample generation  */
#define kPhyCfg2EnableGfastV43ToneSet          0x00400000  /* enable V43 tone set for GFAST */
#define kPhyCfg2TTNETFeatureBit                0x00800000  /* Enable TTNET VDSL US rate feature */
#define kPhyCfg2EnableFastPLLFilter            0x01000000  /* Enable wider PLL filter with settling time of 35 TDD frame */
#define kPhyCfg2DisablePostOPVecFeqUpdate      0x02000000  /* Disable post O-P-Vector feq update  */
#define kPhyCfg2EnableVdslLCHPF                0x04000000  /* Enable LC HPF in 63268 AFE for VDSL not used on 42 branch*/
#define kPhyCfg2EnableRSCoderZeroDelay         0x08000000  /* Enable RS encoder in DS for D = 1 to match old behavior */
#define kPhyCfg2EnablePhyMaxAttnDr			   0x10000000  /* To Enable Phy Max Attn DR calculation based on Training Max Attn DR  */
#define kPhyCfg2IncreaseG994StartDelay         0x20000000  /* Increase initial G.994.1 quiet period before RTones from 100ms to 1s */
#define kPhyCfg2DisableGfastFullDTUsra         0x40000000  /* Disable full DTU changes in SRA/OLR */
#define kPhyCfg2PreferBondingOverPhyR          0x80000000  /* on 63138, advertise PhyR only in non-Bonding context */

/* phyExtraCfg[2] bitmap */
#define kPhyCfg3CapVdslTxPsd                   0x00000001  /* For !USO_only modes (for Kaon Media) to reduce echo on 138BJ CPE on 42 branch only */
#define kPhyCfg3EnableVdslLRmodeByDefault      0x00000002  /* Enabling vdsl lr mode from the start of training itself */
#define kPhyCfg3G994ImprovedRcvForHighNoise    0x00000004  /* Enable G994.1 Improved Rcv for High Noise */
#define kPhyCfg3ForceStrictFRATime             0x00000008  /* Force Strict FRA time > INPMin */
#define kPhyCfg3MinimizeGfastVDSLtoggle        0x00000010  /* Prevent AFE toggle switch from VDSL to G.Fast mode */
#define kPhyCfg3ForceF43ToneSetOnly            0x00000020  /* Force F43 toneset only for testing */
#define kPhyCfg3Reserved                       0x00008000  /* Reserved */

/* SubChannel Info bitMap for G992p1 */
#define kSubChannelASODownstream                0x00000001
#define kSubChannelAS1Downstream                0x00000002
#define kSubChannelAS2Downstream                0x00000004
#define kSubChannelAS3Downstream                0x00000008
#define kSubChannelLSODownstream                0x00000010
#define kSubChannelLS1Downstream                0x00000020
#define kSubChannelLS2Downstream                0x00000040
#define kSubChannelLS0Upstream                  0x00000080
#define kSubChannelLS1Upstream                  0x00000100
#define kSubChannelLS2Upstream                  0x00000200
#define kSubChannelInfoOctet1Mask               0x0000001F
#define kSubChannelInfoOctet2Mask               0x000003E0
#define kSubChannelInfoOctet1Shift              		 0
#define kSubChannelInfoOctet2Shift              		 5

/****************************************************************************/
/*	VDSL training RX State												                          */
/*																			                                    */
/****************************************************************************/
#define kG993p2RxBase                         3000
#define getStatusState(x)                   ((x)+kG993p2RxBase)
/* Channel Discovery */
#define kG993p2RcvMeasureQuiet1             (kG993p2RxBase+0)
#define kG993p2RcvHuntStart1                (kG993p2RxBase+1)
#define kG993p2RcvAgcMeasure1               (kG993p2RxBase+2)
#define kG993p2RcvInitPll1                  (kG993p2RxBase+3)
#define kG993p2RcvFindCoarseAlignment1      (kG993p2RxBase+4)
#define kG993p2RcvExecuteMove1              (kG993p2RxBase+5)
#define kG993p2RcvFindFineAlignment1        (kG993p2RxBase+6)
#define kG993p2RcvExecuteMove2              (kG993p2RxBase+7)
#define kG993p2RcvRelockPll1                (kG993p2RxBase+8)
#define kG993p2RcvFeqInstallation1          (kG993p2RxBase+9)
#define kG993p2RcvFindBestTones1            (kG993p2RxBase+10)
#define kG993p2RcvWaitOsignature            (kG993p2RxBase+11)
#define kG993p2RcvFindFineAlignment2        (kG993p2RxBase+12)
#define kG993p2RcvExecuteMove3              (kG993p2RxBase+13)
#define kG993p2RcvRelockPll2                (kG993p2RxBase+14)
#define kG993p2RcvCalcUsPbo                 (kG993p2RxBase+15)
#define kG993p2RcvAgcMeasure2               (kG993p2RxBase+16)
#define kG993p2RcvFeqInstallation2          (kG993p2RxBase+17)
#define kG993p2RcvFeqLms11                  (kG993p2RxBase+18)
#define kG993p2RcvMeasureSnr1               (kG993p2RxBase+19)
#define kG993p2RcvSendRmsg1                 (kG993p2RxBase+20)
#define kG993p2RcvDetectSynchro1            (kG993p2RxBase+21)
#define kG993p2RcvLineprobe                 (kG993p2RxBase+22)
#define kG993p2RcvToneSelectAndPbo          (kG993p2RxBase+23)
#define kG993p2RcvWaitOupdate               (kG993p2RxBase+24)
#define kG993p2RcvBpOptimization            (kG993p2RxBase+25)
#define kG993p2RcvWaitOprms                 (kG993p2RxBase+26)
#define kG993p2RcvWaitOack                  (kG993p2RxBase+27)
#define kG993p2RcvDetectSynchro3            (kG993p2RxBase+28)

/* Transceiver training  */
#define trBase                              (kG993p2RcvDetectSynchro3)
#define kG993p2RcvHuntStart2                (trBase+1)
#define kG993p2RcvAgcMeasure3               (trBase+2)
#define kG993p2RcvFindCoarseAlignment2      (trBase+3)
#define kG993p2RcvExecuteMove4              (trBase+4)
#define kG993p2RcvRelockPll3                (trBase+5)
#define kG993p2RcvFindFineAlignment3        (trBase+6)
#define kG993p2RcvExecuteMove5              (trBase+7)
#define kG993p2RcvRelockPll4                (trBase+8)
#define kG993p2RcvTxTrainingSignal          (trBase+9)
#define kG993p2RcvAgcMeasure4               (trBase+10)
#define kG993p2RcvFeqInstallation3          (trBase+11)
#define kG993p2RcvFindBestTones2            (trBase+12)
#define kG993p2RcvDetectSynchro4            (trBase+13)
#define kG993p2RcvComputeRta                (trBase+14)
#define kG993p2RcvWaitOTaUpdate             (trBase+15)
#define kG993p2RcvWaitRTaUpdateAck          (trBase+16)
#define kG993p2RcvDetectSynchro5            (trBase+17)

/* anaExch */
#define aeBase                              (kG993p2RcvDetectSynchro5)
#define kG993p2RcvFeqLms12                  (aeBase+1)
#define kG993p2RcvMeasureSnr2               (aeBase+2)
/* loop diagnostic actions */
#define kG993p2LdCalcHlin                   (aeBase+3)
#define kG993p2LdDetectSynchro6             (aeBase+4)
#define kG993p2LdWaitOmsgLd                 (aeBase+5)
#define kG993p2LdWaitOmsgLdEnd              (aeBase+6)
#define kG993p2LdDetectSynchro7             (aeBase+7)
#define kG993p2LdComplete                   (aeBase+8)
/* anaExch - cont. */
#define kG993p2RcvWaitOmsg1                 (aeBase+9)
#define kG993p2RcvComputeBmax               (aeBase+10)
#define kG993p2RcvWaitOtps                  (aeBase+11)
#define kG993p2RcvWaitOpms                  (aeBase+12)
#define kG993p2RcvPrepDsRateSelect          (aeBase+13)
#define kG993p2RcvComputeDsBiGi             (aeBase+14)
#define kG993p2RcvFeqLms13                  (aeBase+15)
#define kG993p2RcvWaitOpmd                  (aeBase+16)
#define kG993p2RcvParseOpmd                 (aeBase+17)
#define kG993p2ScaleFeqComplete             (aeBase+18)
#define kG993p2RcvTxRpmdComplete            (aeBase+19)

/****************************************************************************/
/*	VDSL training TX Signal												                          */
/*																			                                    */
/****************************************************************************/

#define kG993p2TxBase                       (kG993p2RcvTxRpmdComplete+1)
#define getTxSignalIndex(x)                 ((x)+kG993p2TxBase)

/* Channel Discovery */
#define kG993p2TxQuiet1                     (kG993p2TxBase+0)
#define kG993p2TxChDiscovery1               (kG993p2TxBase+1)
#define kG993p2TxSynchro1                   (kG993p2TxBase+2)
#define kG993p2TxPeriodic1                  (kG993p2TxBase+3)
#define kG993p2TxSynchro2                   (kG993p2TxBase+4)
#define kG993p2TxChDiscovery2               (kG993p2TxBase+5)
#define kG993p2TxSynchro3                   (kG993p2TxBase+6)
#define kG993p2TxQuiet2                     (kG993p2TxBase+7)

/* Transceiver training */
#define kG993p2TxQuiet3                     (kG993p2TxBase+8)
#define kG993p2TxTrainingRandom1            (kG993p2TxBase+9)
#define kG993p2TxTrainingRandom2            (kG993p2TxBase+10)
#define kG993p2TxSynchro4                   (kG993p2TxBase+11)
#define kG993p2TxTeq                        (kG993p2TxBase+12)
#define kG993p2TxQuiet4                     (kG993p2TxBase+13)
#define kG993p2TxEct                        (kG993p2TxBase+14)
#define kG993p2TxPeriodic2                  (kG993p2TxBase+15)
#define kG993p2TxTrainingRandom3            (kG993p2TxBase+16)
#define kG993p2TxSynchro5                   (kG993p2TxBase+17)

/* anaExch */
#define kG993p2TxMedley                     (kG993p2TxBase+18)
#define kG993p2TxSynchro6                   (kG993p2TxBase+19)
#define kG993p2TxChDiscovery3               (kG993p2TxBase+20)
#define kG993p2TxSynchro7                   (kG993p2TxBase+21)
#define kG993p2TxQuiet5                     (kG993p2TxBase+22)

/* not implemented */
#define kG993p2TxLineProbe                  (kG993p2TxBase+23)

/*
 *
 *	  Connection setup definitions
 *
 */

typedef enum
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
    kVendorCentilliumHsV3,
	kVendorCNXT,
	kVendorMetanoia,
    kVendorHiSilican,
    kVendorSckipio
	} VendorIDType;

#define	kDslVendorFirwareUnknown	0
typedef	enum
	{
	kVendorADI_Anaconda = 1,
	kVendorADI_ECI918,
	kVendorADI_ECI930,
	kVendorADI_Cisco,
	kVendorADI_UE9000_918,
	kVendorADI_Samsung_930,
	kVendorAnymedia408,
	kVendorTI_4000C_ERICSSON_350,
	kVendorTI_4000C_SEIMENS,
	kVendorTI_ADTRAN_TA1200,
	kVendorTI_AC5_BT,
	kVendorADI_Samsung_CHT,
    	kVendorADI_ECI930_SingTel,
 	kVendorADI_Alcatel_7350,
	kVendorGSPN_Lucent_ADSL1,
	kVendorGSPN_Lucent_ADSL2p,
	kVendorGlobespan_UT_AN200B_800,
	kVendorGlobespan_UT_AN200B_900,
	kVendorAlcatel_ZTE8426,
        kVendorADI_Cisco_TA,
        kVendorInfineon_711D,
        kVendorInfineon_66H,
        kVendorBroadcom_6_3_X,
	kVendorADI_ECI16_AnnexB = 50,		/* leave space for more Annex A types */
	kVendorADI_ECI16A_AnnexB,
	kVendorTI_4000C_AnnexB,
	kVendorTI_AC5_AnnexB,
	kVendorTI_AC5_SAG21_AnnexB,
	kVendorADI_Samsung_KTlab,
	kVendorADI_Samsung_918_KTlab,
	kVendorGlobespan_LU_A2P72_HBI,
    kVendorInfineon_Geminax,
    kVendorInfineon_Vinax_1,
    kVendorInfineon_Vinax_2,
    kVendorGlobespan_AMA,
	kVendorGlobespan_CalixC7_Windstream,
	kVendorGlobespan_Huawei_MA5100,
	kVendorGlobespan_Maxtane_M24,          /* The FW version for Maxtane_M24 CO chipset in Combo2-24A line card */
	kVendorGlobespan_H563ADEF,			/* At Jazztel, Vendor version: E.67.1.78 */
	kVendorGlobespan_CalixC7_A24,		/* See CSP 590733 and CSP 571352 */
	kVendorGlobespan_CalixC7_A24a,		/* See CSP 1079282 */
	kVendorGlobespan_Stinger_FS,			/* CSP 618922 */
  kVendorGlobespan_LU_A2P72_TEF,  /* A2P72-HBI with E67.1.78 */
  kVendorGlobespan_CalixC7_G24,    /* Combo2-24/ADSL2-24 linecard with CNXT G24 chipsets */
  kVendorGlobespan_STGR_LIM_A2P_48_HB_TAO_9_14_2  /* STGR_LIM_A2P_48_HB firmware TAO 9.14.2 at Telebec */
	} VendorFirmwareIDType;

#define AluProxy_8_7 ((6<<8)|2)
#define AluProxy_8_9 ((6<<8)|6)

#define	kDslXmtToneSelectionStartTone		0
#if defined(ANNEX_M) || defined(G992P1_ANNEX_B)
#define	kDslXmtToneSelectionEndTone			63
#else
#define	kDslXmtToneSelectionEndTone			31
#endif
#define	kDslXmtToneSelectionNumOfTones		(kDslXmtToneSelectionEndTone-kDslXmtToneSelectionStartTone+1)
#define	kDslXmtToneSelectionNumOfBytes		((kDslXmtToneSelectionNumOfTones+7)/8)
#define	kDslRcvToneSelectionStartTone		32
#define	kDslRcvToneSelectionEndTone			511
#define	kDslRcvToneSelectionNumOfTones		(kDslRcvToneSelectionEndTone-kDslRcvToneSelectionStartTone+1)
#define	kDslRcvToneSelectionNumOfBytes		((kDslRcvToneSelectionNumOfTones+7)/8)

#define	kDslT1p413RAckModeTryRAck1				0x01
#define	kDslT1p413RAckModeTryRAck2				0x02
#define	kDslT1p413RAckModeTrialMask				0x0F
#define	kDslT1p413RAckModeSelected				0x10
#define	kDslT1p413RAckModeTrialCount			10		/* when in trial mode */
#define	kDslT1p413RAckModeSwitchCount			20		/* when mode is selected */

#ifdef ADSL_MARGIN_TWEAK_TEST
#define	kDslMarginTweakNumOfTones				256
#endif

#define kDslANFPLongLoop       0x10
#define kDslANFPMediumLoop     0x08
#define kDslANFPShortLoop      0x04
#define kDslANFPExtraShortLoop 0x02
#define kDslANFPUltraShortLoop 0x01

#define kDslANFPLongLoopAttn       (54*16)
#define kDslANFPMediumLoopAttn     (50*16)
#define kDslANFPShortLoopAttn      (41*16)
#define kDslANFPExtraShortLoopAttn (22*16)
#define kDslANFPUltraShortLoopAttn  0
#define kDslATTAnnexLShortLoopAttn (58*16)		/* 12000ft */

/* Values for use with fireATURSetting to decide if these features are enabled by the driver */
/* These values are aligned with G.hs NSF bit fields.  Do not adjust positions without also altering the matching code */
#define kFireATURSettingDsSupport       (0x01)
#define kFireATURSettingDsForced        (0x02)
#define kFireATURSettingUsSupport       (0x04)
#define kFireATURSettingUsForced        (0x08)
#define kFireATURSettingDlyRxQ          (0x10)
#define kFireATURSettingNDlyRxQ         (0x20)
#define kFireATURSettingDsGinpSupport   (0x40)
#define kFireATURSettingUsGinpSupport   (0x80)

typedef struct
	{
	Boolean	                        haveRemoteCapabilities;
	dslModulationType				selectedModulation;
	dslModulationType				startupModulation;
#if defined(G992P1_ANNEX_I) || defined(G992P5) || defined(G993)
	ushort							downstreamMinCarr, downstreamMaxCarr;
#else
	uchar							downstreamMinCarr, downstreamMaxCarr;
#endif
#if defined(G993)
	ushort							upstreamMinCarr, upstreamMaxCarr;
#else
	uchar							upstreamMinCarr, upstreamMaxCarr;
#endif /* G993 */
#if defined(DOUBLE_UP_STREAM) || defined(ANNEX_J)
    Boolean                         isDoubleUsEnabled;
#endif
    short                           selectedPilotTone;
	dslDataPumpCapabilities			localCapabilities, remoteCapabilities;
#ifdef G992P3
	g992p3PhyDataPumpCapabilities	*localCarrierInfoG992p3AnnexA;
	g992p3PhyDataPumpCapabilities	*selectedCarrierInfoG992p3AnnexA;
#ifdef G992P5
	g992p3PhyDataPumpCapabilities	*localCarrierInfoG992p5AnnexA;
	g992p3PhyDataPumpCapabilities	*selectedCarrierInfoG992p5AnnexA;
#endif /* P5 */
	/* extra configuration from kDslExtraPhyCfgCmd  */
	uint							phyExtraCfg[4];

	uchar							xmtG992p3State;
	uchar							rcvG992p3State;
#ifdef VDSL_MODEM
	uchar							xmtG993p2State;
	uchar							rcvG993p2State;
#endif
	Boolean							ituOptionD, ituOptionSmin;
#ifdef EXTENDED_INTERLEAVE_DEPTH_24K
        Boolean                                                 support24kBytesInterleaver;
#endif
	uchar							oneOverS0minMinusOne;
	short			   				OptionDSupport;
#ifdef PTM_OVER_ADSL
    ushort                          ptmType;
#endif
#ifdef ADSL_VIRTUAL_NOISE
    uchar                           virtualNoise;
#endif
#ifdef ADSL_CPE_SNR_CLAMPING
    uchar                           snrClampingId;
#endif
	uchar							dsMaxPcb, dsMinPcb, coSWVersion[4],dsParCtrl, chipsetVersion;
    uchar                           bondingSupport;
#if defined(BONDING_G994_DISCOVERY) || defined(DSL_MULTIPHY_BONDING) || defined(DSL_EXTERNAL_BONDING_DISCOVERY)
    uchar                           pmeId;
#endif
#ifdef DSL_EXTERNAL_BONDING_DISCOVERY
	Boolean							externalBondingDrvReplied;
	int								externalBondingResumeLoc;
#endif
	ushort							maxSNRMargin;
        uint                           coFeature;
	int								QLNMonitoring;
	int                 DslTestECResp;
	uchar                           QLNMntrForEver;
	uchar                           AfeLBsymCntShift;
	uchar                           AfeCalculating;
	uchar                           QLNsymCntShift;
    uchar                           ReportFreq;
  short                           AfeForceAGC;
  short                           AfeForcePPM;
  short                           AfeForcePpmOffsetFlag;
	int                        QLNsymCount;
	int                        AfeLBsymCount;
#if defined(FIRE_RETRANSMISSION) || defined(FIRE_XMT_6368)
    uchar                           fireATURSetting;        /* Setting at CPE side - The value will be hardcoded in the PHY but can be overwritten by the driver */
                                                            /*    bit7: G.Inp US support     */
                                                            /*    bit6: G.Inp DS support     */
                                                            /*    bit5: no-delay RxQ support */
                                                            /*    bit4: delay RxQ support    */
                                                            /*    bit3: fire forced in US    */
                                                            /*    bit2: fire supported in US */
                                                            /*    bit1: fire forced in DS    */
                                                            /*    bit0: fire supported in DS */
    g992FireSpecifications          *fireSpecificationPtr;  /* Specification resulting from the G994p1 handshake */
#endif
#endif  /* G992P3 */
#if defined(G993) /* ADD for VDSL2 codepoint */
	g993p2PhyDataPumpCapabilities	*localCarrierInfoG993p2AnnexA;
	g993p2PhyDataPumpCapabilities	*selectedCarrierInfoG993p2AnnexA;
	g993p2ProfileContent  			*selectedProfileContentG993p2AnnexA ;
#endif /* G993 */
	uchar							handshakingDuplexMode;
	Boolean							handshakingClientInitiation;
	short							handshakingXmtPowerLevel;
	short							handshakingXmtPowerFlag;
	uchar							handshakingXmtCarrierSet;
	short							hwAgcQ4dB;	/* for loop attenuation calculation */
	uchar							coVendorID;
#ifdef	BCM6368_SRC
	uchar							coVendorIDsecondary; /* due to company merge such as CNXT/IKNS */
#endif
	unsigned short    ikanosFwVersion;
#ifdef	ADSL_IDENTIFY_VENDOR_FIRMWARE
	uchar							coVendorFirmwareID;
	uchar							coVendorSpecificInfo;
#endif
	uchar							codingGainDecrement;	/* coding gain decrement in Q4dB for initial rate calculation */
	uchar							xmtToneSelection[kDslXmtToneSelectionNumOfBytes];
	uchar							rcvToneSelection[kDslRcvToneSelectionNumOfBytes];
#if defined(T1P413) && defined(XMT_RACT2_FOR_ADI_COMPATIBILITY)
	uchar							t1p413RAckModeUsed;
	uchar							t1p413RAckModeCounter;
#endif
#ifdef G992P1_ANNEX_B
	uchar							badSNR2RetrainCounter;
	int              highNoiseControl;
	short             g994BackOff;
	short       ifxAdsl2pUSDetectFail; 
#endif
#if defined(ANNEX_J) || defined(ANNEX_M)
	uint     symbolsAtLastToggle;
#endif
	uchar							failG994p1Counter, failT1p413Counter;
	short							assumedNoiseMargin;
#ifdef ADSL_MARGIN_TWEAK_TEST
	short							marginTweakExtraPowerQ4dB;
	char							marginTweakTableQ4dB[kDslMarginTweakNumOfTones];
#endif
#ifdef G992P2_PROFILE
	G992p2ProfileVarsStruct*		profileVarsPtr;
#endif
#ifdef TDC_IOP_FIX_SEIMENS_TI
	char					t1p413RetrainCounter;		/* 0: no retrain needed; 1: force to T1.413 mode and retrain after R-MSG1; 2: 2nd T1.413 session, go to showtime */
#endif
#ifdef FT_ADI_US_RATE_FIX
	char					eciADITxBoostRetrainCnt;
#endif /* FT_ADI_US_RATE_FIX */
  Boolean   anfpMaskRetrainFlag;
  char      anfpMask;   /* Bit 1:Ultra Short, Bit 2:Extra Short, Bit 3:Short, Bit 4:Medium, Bit 5:Long */
#ifdef ANSI_CACT12_PING_PONG
    char t1p413SkipToneIndex; /* to alternate between CAct1 and CAct2 detection */
#endif
	char					cnxtGdmtUsCrcRetrainCntr;
	Boolean					cnxtAdsl2pUsWorkaround;
	Boolean					ut900Adsl2pWorkaround;
	Boolean					gspn13bitChangeVendorId;
	char					messageG994p1WrongResponseCounter;
	char					cpilotDetectionFailCounter;
#if defined(G993)
	char					fullVendorId[8];
#endif
  int interopFootprint;
#ifdef IKNS_CO4_INTEROP
  short						loopLengthEstForCO4Hs;
#endif
  uchar g994p1FalseCToneCounter;
  uchar g994p1DisableA43CToneSet;
  uchar g994p1DetectedCTone;
  int dslG994p1RevisionSwitchCounter;
    int dslG994p1NAKCounter;
#if defined(BCM6368_SRC)
  uchar g994p1FalseCToneCarrCount[12];
#if !defined(VDSL_ONLY)
	Boolean		enableVDSLcapability;
	char		lastG994Mode;	/* last G994 Mode: VDSL or ADSL */
	Boolean		startupModeSelected;
#ifdef	T1P413
	int		nextStartupMode;
	uint		currentModeCounter;
#endif
	int			dslModeSelectTimeoutInSyms;
	int			nonVdslErrorCounter;
	int			rcvRemoteG994p1Revision;
#endif
#endif

	uint		t1p413HighLowPowerCounter;
	Boolean g994p1SendBlankVendorId;
#if defined(BCM6368_SRC) && defined(T1P413)
	Boolean		t1p413DetectedCAct;
	uchar		t1p413DetectedCActConsecCounter;
#endif
#if (defined(BCM6368_SRC) && defined(G992P3) && defined(READSL2))
	Boolean		AnnexLDisable;
	uchar		AnnexLDisableCounter;
	Boolean     TIAc5BitswapDisabled;
#endif
#ifndef G992P1_ANNEX_B
	uchar		forceAnnexLWithReducedG994Power;	/* Force GSPN DSLAM to select Annex L at long loop */
#endif
    uint   interopRetrainFlags;              /* Number of retrain flags are growing. Use bit map in interopRetrainFlags register */
#ifdef HANDSHAKE_SIGNATURE_ECHO
    uchar       signatureId;
#endif
#ifdef G994p1_SUPPORT_V43
	short		v43DefaultLevelQ0dB;
	short		v43PowerReductionQ0dB;
#endif
	} dslConnectionSetupStruct;

/****************************************************************************/
/*	3.	Interface functions.												*/
/*																			*/
/****************************************************************************/

#ifdef G992P1
#if defined(G992P1_ANNEX_I2X) || defined(G992P5)
/* lke */
#define	kDslSamplingFreq			4416000
#define	kDslMaxFFTSize			 	1024
#define	kDslMaxFFTSizeShift			10
#elif defined(G992P1_ANNEX_I4X)
#define	kDslSamplingFreq			8832000
#define	kDslMaxFFTSize			 	2048
#define	kDslMaxFFTSizeShift			11
#elif defined(G992P1_ANNEX_I8X)
#define	kDslSamplingFreq			17664000
#define	kDslMaxFFTSize			 	4096
#define	kDslMaxFFTSizeShift			12
#else
#define	kDslSamplingFreq			2208000
#define	kDslMaxFFTSize				512
#define	kDslMaxFFTSizeShift			9
#endif
#else
#define	kDslSamplingFreq			1104000
#define	kDslMaxFFTSize				256
#define	kDslMaxFFTSizeShift			8
#endif

#if defined(G992_ATUR_UPSTREAM_SAMPLING_FREQ_276KHZ)
#define kDslATURUpstreamSamplingFreq    276000
#define	kDslATURFFTSizeShiftUpstream	6
#elif defined(G992_ATUR_UPSTREAM_SAMPLING_FREQ_552KHZ)
#define kDslATURUpstreamSamplingFreq    552000
#define	kDslATURFFTSizeShiftUpstream	7
#else
#define kDslATURUpstreamSamplingFreq    kDslSamplingFreq
#define	kDslATURFFTSizeShiftUpstream	kDslMaxFFTSizeShift
#endif

#if defined(G992_ATUC_UPSTREAM_SAMPLING_FREQ_276KHZ)
#define kDslATUCUpstreamSamplingFreq    276000
#define	kDslATUCFFTSizeShiftUpstream	6
#elif defined(G992_ATUC_UPSTREAM_SAMPLING_FREQ_552KHZ)
#define kDslATUCUpstreamSamplingFreq    552000
#define	kDslATUCFFTSizeShiftUpstream	7
#else
#define kDslATUCUpstreamSamplingFreq    kDslSamplingFreq
#define	kDslATUCFFTSizeShiftUpstream	kDslMaxFFTSizeShift
#endif

#define	kDslMaxSamplesPerSymbol		(kDslMaxFFTSize+kDslMaxFFTSize/16)

#if defined(G992P1_ANNEX_I) || defined(G992P5)
#define kDslMaxTEQLength	        32
#else
#define kDslMaxTEQLength	        16
#endif

#define	kDslMaxSymbolBlockSize		1
#define	kDslMaxSampleBlockSize		(kDslMaxSymbolBlockSize*kDslMaxSamplesPerSymbol)

#ifdef G992_ANNEXC
#define	kG992AnnexCXmtToRcvPathDelay	512   /* In samples at kDslSamplingFreq */
#endif

#define	kDslSymbolsInTenSecond			43125	/* 2208000 / 512 * 10, prefix off */

#if	(defined(BCM6368_SRC) && (!defined(VDSL_ONLY)))

#define	kDslModeSelectTimeoutInSecs		30
#ifdef ADSL_SPECIAL_FIX_FOR_FRENCH_TELECOM
#define	kDslModeSwitchTimeoutInSecs		90
#else
#define	kDslModeSwitchTimeoutInSecs		45
#endif
#define	kDslModePrefSelectTimeoutInSecs	60		/* time out when kDslMultiModesPreferT1p413 is defined */
#define	kDslModePrefSwitchTimeoutInSecs	60		/* time out when kDslMultiModesPreferT1p413 is defined */

#define	kDslSymbolsInOneSecond			(kDslSymbolsInTenSecond/10)
#define	kDslModeSelectTimeoutInSyms		(kDslModeSelectTimeoutInSecs*kDslSymbolsInTenSecond/10)
#define	kDslModeSwitchTimeoutInSyms		(kDslModeSwitchTimeoutInSecs*kDslSymbolsInTenSecond/10)
#define	kDslModePrefSelectTimeoutInSyms	(kDslModePrefSelectTimeoutInSecs*kDslSymbolsInTenSecond/10)
#define	kDslModePrefSwitchTimeoutInSyms	(kDslModePrefSwitchTimeoutInSecs*kDslSymbolsInTenSecond/10)

#endif

/*** For compatibility with existing test codes ***/
#if !defined(TARG_OS_RTEMS)
typedef dslStatusCode				modemStatusCode;
typedef	dslStatusStruct				modemStatusStruct;
typedef	dslStatusHandlerType		statusHandlerType;
typedef dslCommandCode				modemCommandCode;
typedef	dslCommandStruct			modemCommandStruct;
typedef	dslCommandHandlerType		commandHandlerType;
#endif

extern void		SM_DECL SoftDslSetRefData	(void *gDslVars, uint refData);
extern uint	        SM_DECL SoftDslGetRefData	(void *gDslVars);
extern int		SM_DECL SoftDslGetMemorySize(void);
extern void		SM_DECL SoftDslInit			(void *gDslVars);
extern void		SM_DECL SoftDslReset		(void *gDslVars);
#if !defined(__KERNEL__) && !defined(_CFE_) && !defined(__ECOS) && !defined(_NOOS)
#ifdef ADSL_SHOWTEXT
extern void		SM_DECL SoftDslLineHandler	(void *gDslVars, int rxNSamps, int txNSamps, short *rcvPtr, short *xmtPtr) SHOWTEXT_ADSL;
#else
extern void		SM_DECL SoftDslLineHandler	(void *gDslVars, int rxNSamps, int txNSamps, short *rcvPtr, short *xmtPtr) FAST_TEXT;
#endif
extern Boolean	        SM_DECL SoftDslCommandHandler (void *gDslVars, dslCommandStruct *cmdPtr) SHOWTEXT_MAIN;
#endif

/* swap Lmem functions */
#if defined(bcm47xx) && defined(SWAP_LMEM)
extern int SoftDslSwapLmem(void *gDslVars, int sectionN, int imageN);
extern void init_SoftDslSwapLmem(void);
extern void printSwapAllocation(void *gDslVars);
#endif

/* SoftDsl symbol counters	*/

#define			SoftDslSymCntTotal(gv)		gDslGlobalVarPtr->totSymCnt
#define			SoftDslSymCntPhase(gv)		gDslGlobalVarPtr->phaseSymCnt

extern uint	SoftDslGetSymCntTotal(void *gDslVars);
extern uint	SoftDslGetSymCntPhase(void *gDslVars);

/* SoftDsl time functions	*/

extern uint	        SM_DECL SoftDslGetTime(void *gDslVars);
#define			__SoftDslGetTime(gv)		gDslGlobalVarPtr->execTime

extern void		SM_DECL SoftDslTimer(void *gDslVars, uint timeMs);

/* SoftDsl IO functions	*/

extern void		SM_DECL SoftDslClose (void *gDslVars);
extern int		SM_DECL SoftDslSendFrame (void *gDslVars, void *pVc, uint mid, dslFrame * pFrame);
extern int		SM_DECL SoftDslReturnFrame (void *gDslVars, void *pVc, uint mid, dslFrame * pFrame);

/* SoftDsl connection functions	*/

extern	void*	SM_DECL SoftDslVcAllocate(void *gDslVars, dslVcParams *pVcParams);
extern	void	SM_DECL SoftDslVcFree(void *gDslVars, void *pVc);
extern	Boolean SM_DECL SoftDslVcActivate(void *gDslVars, void *pVc);
extern  void	SM_DECL SoftDslVcDeactivate(void *gDslVars, void *pVc);
extern  Boolean SM_DECL SoftDslVcConfigure(void *gDslVars, void *pVc, uint mid, dslVcParams *pVcParams);

/* Special functions for LOG support */

extern  uint	SM_DECL SoftDslVc2Id(void *gDslVars, void *pVc);
extern  void*	SM_DECL SoftDslVcId2Vc(void *gDslVars, uint vcId);
extern	void*	SM_DECL SoftDslGetFramePool(void *gDslVars);

/* Functions for host mode execution */

extern  void*   SM_DECL SoftDslRxCellHeaderHandler (void *gDslVars, uint hdr, uchar hdrHec);
extern	void*	SM_DECL SoftDslRxCellDataHandler (void *gDslVars, int,	void*);
extern  void*	SM_DECL SoftDslTxCellHandler	(void *gDslVars, int*,	void*);
extern  Boolean	SM_DECL SoftDslPhyCommandHandler (void *gDslVars, dslCommandStruct *cmdPtr);

/* Functions getting OEM parameters including G994 non standard info management */

extern	char*	SM_DECL SoftDslGetTrainingVendorIDString(void *gDslVars, int isT1413);
extern	char*	SM_DECL SoftDslGetVendorIDString(void *gDslVars);
extern	char*	SM_DECL SoftDslGetSerialNumberString(void *gDslVars);
extern	char*	SM_DECL SoftDslGetRevString(void *gDslVars);
extern	int	SM_DECL SoftDslRevStringSize(void *gDslVars);
extern	int	SM_DECL SoftDslSerNumStringSize(void *gDslVars);

extern  void*	SM_DECL SoftDslGetG994p1RcvNonStdInfo(void *gDslVars, uint *pLen);
extern  void*	SM_DECL SoftDslGetG994p1XmtNonStdInfo(void *gDslVars, uint *pLen);

#ifdef G997_1_FRAMER

/* G997 functions */

extern int		SM_DECL SoftDslG997SendFrame (void *gDslVars, void *pVc, uint mid, dslFrame * pFrame);
extern int		SM_DECL SoftDslG997ReturnFrame (void *gDslVars, void *pVc, uint mid, dslFrame * pFrame);

#endif

#ifdef ADSL_MIB
extern void	 *	SM_DECL	SoftDslMibGetData (void *gDslVars, int dataId, void *pAdslMibData);
#endif

#define	SoftDsl					SoftDslLineHandler
#define	kSoftDslMaxMemorySize	(32768*16384)

/* Functions for processing framing parameters before sending to driver */
#define kSendDsParams 0
#define kSendUsParams 1
void SoftDslGenCodingParam(void * gDslVars, FramerDeframerOptions *frParam, G992CodingParams  *rxCodingParam);
void SoftDslInitExchangeInfoFramerParams(void *gDslVars);
void SoftDslSendFramerParams(void *gDslVars, FramerDeframerOptions *frParam, int lpId, int isInShowtime, int dsUs);


/*
 * Internal functions
 */

#ifdef	EXTENDED_INTERLEAVE_DEPTH_24K
extern  void	SoftDslStatusHandler	(void *gDslVars, dslStatusStruct *status);
#else
extern  void	SoftDslStatusHandler	(void *gDslVars, dslStatusStruct *status) FAST_TEXT;
#endif
extern  void	SoftDslInternalStatusHandler (void *gDslVars, dslStatusStruct *status);

/*
 *		DSL OS functions
 */

#define	BG_TASK_IDLE						0
#define	BG_TASK_SCHEDULED					1
#define	BG_TASK_RUNNING						2

/* task result definitions - have to be consistent with E14 TASK_RESULT_xxx in TaskResult.h */

#define	BG_TASK_RES_INVALID					0
#define	BG_TASK_RES_NOT_STARTED				1
#define	BG_TASK_RES_RUNNING					2
#define	BG_TASK_RES_FINISHED				3

#define DSL_TLS_SIZE						3

#define TLS_E14_LINE_OBJ					0
#define TLS_E14_SUICIDE						1
#define TLS_gDslVars						2

#if defined(DSL_OS) && defined(MIPS_SRC)

#define SoftDslCacheLineInvalidate(ptr,off)  asm volatile("cache (4<<2)|1, %1(%0)" :: "r" (ptr), "i" (off))

#define	SoftDslIsBgAvailable(gDslVars)		(DSLOS_THREAD_INACTIVE == DslOsGetThreadState(&(gDslGlobalVarPtr->tcbDslBg)))
#define	SoftDslGetBgThread(gDslVars)		\
	((DSLOS_THREAD_INACTIVE != DslOsGetThreadState(&(gDslGlobalVarPtr->tcbDslBg))) ? &gDslGlobalVarPtr->tcbDslBg : NULL)
#define	SoftDslBgStart(gDslVars, pFunc)		\
	DslOsCreateThread(&gDslGlobalVarPtr->tcbDslBg, DSLOS_PRIO_HIGHEST - 10, pFunc, gDslVars,	\
	WB_ADDR(gDslGlobalVarPtr->bgStack), sizeof(gDslGlobalVarPtr->bgStack))
#define	SoftDslBgStop(gDslVars)				DslOsDeleteThread(&gDslGlobalVarPtr->tcbDslBg)

#ifdef E14_CODE
#define	SoftDslBgScheduleTask(gDslVars,pFunc)		SoftDslBgScheduleTaskEx(gDslVars,pFunc, NULL,NULL,NULL)
#define	SoftDslBgScheduleTaskFirst(gDslVars,pFunc)	SoftDslBgScheduleTaskFirstEx(gDslVars,pFunc, NULL,NULL,NULL)
extern Boolean SoftDslBgScheduleTaskEx(void *gDslVars, void *pFunc, uchar *pRes, void *objp, void *argp);
extern Boolean SoftDslBgScheduleTaskFirstEx(void *gDslVars, void *pFunc, uchar *pRes, void *objp, void *argp);
#else
#define	SoftDslBgScheduleTask(gDslVars,pFunc)		SoftDslBgScheduleTaskEx(gDslVars,pFunc,NULL)
#define	SoftDslBgScheduleTaskFirst(gDslVars,pFunc)	SoftDslBgScheduleTaskFirstEx(gDslVars,pFunc,NULL)
extern Boolean SoftDslBgScheduleTaskEx(void *gDslVars, void *pFunc, uchar *pRes);
extern Boolean SoftDslBgScheduleTaskFirstEx(void *gDslVars, void *pFunc, uchar *pRes);
#endif /* E14_CODE */

extern void	*  SoftDslBgScheduleGetTask(void *gDslVars);
extern Boolean SoftDslBgScheduleIsTaskAvail(void *gDslVars);
extern void	   SoftDslBgScheduleClear(void *gDslVars);
extern void	   SoftDslBgScheduleReset(void *gDslVars);
extern void	   SoftDslBgScheduleInit(void *gDslVars);

extern void *  SoftDslBgMonUpdate(void *gDslVars, uint cycCnt);
extern void	   SoftDslBgMonReset(void *gDslVars);
extern void	   SoftDslBgMonSetBgTcb(void *gDslVars, void *pTcb);

#if	(defined(BCM6368_SRC) && (!defined(VDSL_ONLY)))
extern uint		SoftDslGetModulationCapability(void *gDslVars);
#ifdef	T1P413
extern void		SoftDslSelectStartupMode(void *gDslVars, int timeInterval, int currentStartupMode);
extern void		SoftDslClearStartupModeCounter(void *gDslVars);
#endif
#endif

extern void             SoftDslTR98ParamsInit(void *gDslVars);

#define	SoftDslEnterCritical()				DslOsEnterCritical()
#define	SoftDslLeaveCritical(id)			DslOsLeaveCritical(id)

#define SoftDslGetTls()						DslOsGetThreadTls(DslOsGetCurrentThread())
#define SoftDslSetTls(tls)					DslOsSetThreadTls(DslOsGetCurrentThread(), tls)

#define SoftDslGetTlsData(idx)				((uint *)SoftDslGetTls())[idx]
#define SoftDslSetTlsData(idx,data)			((uint *)SoftDslGetTls())[idx] = (data)

#else

#define	SoftDslIsBgAvailable(gDslVars)		1
#define	SoftDslGetBgThread(gDslVars)		1
#define	SoftDslBgStart(gDslVars, pFunc)		(*pFunc)(gDslVars)
#define	SoftDslBgStop(gDslVars)

#define	SoftDslBgScheduleTask(gDslVars,pFunc)		(*pFunc)(gDslVars)
#define	SoftDslBgScheduleTaskFirst(gDslVars,pFunc)	(*pFunc)(gDslVars)
#define	SoftDslBgScheduleTaskEx(gDslVars,pFunc,pRes,objp,argp)		(*pFunc)(gDslVars)
#define	SoftDslBgScheduleTaskFirstEx(gDslVars,pFunc,pRes,objp,argp)	(*pFunc)(gDslVars)
#define	SoftDslBgScheduleGetTask(gDslVars)			NULL
#define	SoftDslBgScheduleIsTaskAvail(gDslVars)		0
#define	SoftDslBgScheduleClear(gDslVars)
#define	SoftDslBgScheduleReset(gDslVars)
#define	SoftDslBgScheduleInit(gDslVars)

#define	SoftDslBgMonUpdate(gDslVars, cycCnt)		NULL
#define	SoftDslBgMonReset(gDslVars)
#define	SoftDslBgMonSetBgTcb(gDslVars, pTcb)

#define	SoftDslEnterCritical()				0
#define	SoftDslLeaveCritical(id)

#endif

/* SoftDsl utility functions */
#ifndef BCM6368_SRC
#define CYCLES_PER_USEC	240	/* 240Mhz/1000000 */
#else
#define CYCLES_PER_USEC	400	/* 400Mhz/1000000 */
#endif

extern void SoftDslMipsSleep(uint usec, int shift);

/* SDRAM write control */
#ifndef BCM6368_SRC
extern void DslCoreXfaceWrCheck(void) FAST_TEXT;
#define	CHECK_WR_COMPLETE()					DslCoreXfaceWrCheck()
#else
#define	CHECK_WR_COMPLETE()
#endif

/* Fast dslSlowVarsStruct access */
#if !(defined(bcm47xx) && defined(MIPS_SRC))
#undef SLOW_VAR_GLOBAL_REG
#endif

#ifdef SLOW_VAR_GLOBAL_REG
register  struct __dslSlowVarsStruct *gDslSlowVars asm ("$28");
#endif

#ifdef DSLVARS_GLOBAL_REG
register  struct __dslVarsStruct	 *gDslVarsReg  asm ("$27");
#define gDslVars	gDslVarsReg
#endif

/*
 *		DSL frames and native frame functions
 */

DslFrameDeclareFunctions (DslFrameNative)

/*
 * These functions are for testing purpose, they are defined outside.
 */
#ifdef STACK_SIZE_REQUIREMENT_TEST
extern	void		StackSizeTestInitializeStackBeforeEntry(void);
extern	void		StackSizeTestCheckStackAfterExit(void);
extern	void		StackSizeTestBackupStack(void);
extern	void		StackSizeTestRestoreStack(void);
#endif /* STACK_SIZE_REQUIREMENT_TEST */

#ifdef NEC_NSIF_WORKAROUND
#define	SoftDslGetG994NsStatus(gDslVars)		        (gDslGlobalVarPtr->G994NsStatus)
#define	SoftDslGetG994NsFailCounter(gDslVars)		    (gDslGlobalVarPtr->G994NsFailCounter)
#endif

/* Generic profiling */

#if 0
#define DO_PROFILING /* This will enable the first 15 types(PROF_TYPE_CYCLE is always enabled) */
#endif

#define PROF_TYPE_ALL_I					0			/* Index of PROF_TYPE_XXX: incremented by 1 per new defined type */
#define PROF_TYPE_MIPS_FG_I			1
#define PROF_TYPE_AFE_WAIT_I			2
#define PROF_TYPE_QPROC_WAIT_I		3
#define PROF_TYPE_QPROC_POST_I		4
#define PROF_TYPE_QPROC_DISPATCH_I	5
#define PROF_TYPE_QPROC_XMT_I			6
#define PROF_TYPE_QPROC_RCV_I			7
#define PROF_TYPE_QPROC_QDMA_I		8
#define PROF_TYPE_BITENC_I				9
#define PROF_TYPE_CONTROL_I			10
#define PROF_TYPE_RSENC_I				11
#define PROF_TYPE_RSDEC_I				12
#define PROF_TYPE_SPECIAL0_I			13
#define PROF_TYPE_SPECIAL1_I			14
#define PROF_TYPE_CYCLE_I				21
#define PROF_TYPE_CYCLE_NO_REPORTING_I				22

#define PROF_TYPE_ALL				(1 << PROF_TYPE_ALL_I)	/* Bit map value of PROF_TYPE_XXX */
#define PROF_TYPE_MIPS_FG			(1 << PROF_TYPE_MIPS_FG_I)
#define PROF_TYPE_AFE_WAIT		(1 << PROF_TYPE_AFE_WAIT_I)
#define PROF_TYPE_QPROC_WAIT		(1 << PROF_TYPE_QPROC_WAIT_I)
#define PROF_TYPE_QPROC_POST		(1 << PROF_TYPE_QPROC_POST_I)
#define PROF_TYPE_QPROC_DISPATCH	(1 << PROF_TYPE_QPROC_DISPATCH_I)
#define PROF_TYPE_QPROC_XMT		(1 << PROF_TYPE_QPROC_XMT_I)
#define PROF_TYPE_QPROC_RCV		(1 << PROF_TYPE_QPROC_RCV_I)
#define PROF_TYPE_QPROC_QDMA		(1 << PROF_TYPE_QPROC_QDMA_I)
#define PROF_TYPE_BITENC			(1 << PROF_TYPE_BITENC_I)
#define PROF_TYPE_CONTROL			(1 << PROF_TYPE_CONTROL_I)
#define PROF_TYPE_RSENC			(1 << PROF_TYPE_RSENC_I)
#define PROF_TYPE_RSDEC			(1 << PROF_TYPE_RSDEC_I)
#define PROF_TYPE_SPECIAL0			(1 << PROF_TYPE_RSDEC_I)
#define PROF_TYPE_SPECIAL1			(1 << PROF_TYPE_SPECIAL1_I)
#define PROF_TYPE_CYCLE			(1 << PROF_TYPE_CYCLE_I)
#define PROF_TYPE_CYCLE_NO_REPORTING			(1 << PROF_TYPE_CYCLE_NO_REPORTING_I)

#define	PROF_TYPE_MASK	0x003FFFFF				/* Max of 22 types supported */
#define	PROF_TABLE_SIZE	22						/* Store offset of PRO_TYPE_XXX */
#define	PROF_LEN_MASK		0x000003FF
#define	PROF_TYPE_SHFT	10

#if !defined(__KERNEL__) && defined(BCM6368_SRC) && defined(ADSLCORE_ONLY)
#if !defined(E14_SOURCE_FILE) || defined(E14_FULL_ACCESS)
#include "SoftBCM6368.h"
#endif
#endif

#if !defined(WINNT) && !defined(__KERNEL__) && !defined(_CFE_) && !defined(__ECOS) && !defined(_NOOS)
#include "SoftDslStatus.h"
#include "SoftDslApi.h"
#endif

#define	DSL_LINE_SHIFT		31
#define	DSL_LINE_MASK		(1 << DSL_LINE_SHIFT)
#define	DSL_MIPSCORE_SHIFT		30
#define	DSL_MIPSCORE_MASK		(1 << DSL_MIPSCORE_SHIFT)
#define	DSL_MIPSCORE_ID(x)		((uchar)(((uint)(x) & DSL_MIPSCORE_MASK) >> DSL_MIPSCORE_SHIFT))
#define DSL_STATUS_CODE(x)   ((x) & 0x0FFFFFFF)  /* bit 31 for line id, bit 30, for MIPS core id, bits 29,28 for client Type */
#if defined(SUPPORT_DSL_BONDING) || defined(WINNT) || defined(LINUX_DRIVER)    // Bonding DSL driver or windows or Linux Diags build
#define	MAX_DSL_LINE		2
#define	DSL_COMMAND_CODE(x)	((x) & (~DSL_LINE_MASK))
#define	DSL_LINE_ID(x)		((uchar)(((uint)(x) & DSL_LINE_MASK) >> DSL_LINE_SHIFT))
#else
#define	MAX_DSL_LINE		1
#define	DSL_COMMAND_CODE(x)	(x)
#define	DSL_LINE_ID(x)		0
#endif

#if defined(__KERNEL__) || defined(_CFE_) || defined(__ECOS) || defined(_NOOS)
#define gLineId(x)			(((dslVarsStruct *)(x))->lineId)
#endif

#define	GINP_FRAMER_STRUCT_SIZE             40
#define	GINP_FRAMER_ETR_STRUCT_SIZE         44
#define	GINP_FRAMER_INPSHINE_STRUCT_SIZE    46
#define	L32_FRAMER_STRUCT_SIZE              50
#define	Nret_FRAMER_STRUCT_SIZE             64
#define	GINP_COUNTERS_SEFTR_STRUCT_SIZE     28
#define	GINP_COUNTERS_ANDEFTR_STRUCT_SIZE   40

#ifdef DSL_REPORT_ALL_COUNTERS
#define	DSL_COUNTERS_MAX	kG992ShowtimeNumOfMonitorCounters
#else
#define	DSL_COUNTERS_MAX	12
#endif

#endif	/* SoftDslHeader */
