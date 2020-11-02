/*
* WLOTA feature.
* On a high level there are two modes of operation
* 1. Non Tethered Mode
* 2. Tethered Mode
*
* IN non tethered mode, a cmd flow file which contains encoded test
*	information is downloaded to device.
*	Format of the cmd flow file is pre defined. Host interprest the cmd flow file
*	and passes down a test "structure" to dongle.
*	Reading and parsing can be done by brcm wl utility or any host software which can do
*	the same operation.
*
*	Once cmd flow file is downloaded, a "trigger" cmd is
*	called to put the device into testing mode. It will wait for a sync packet from
* 	tester as a part of handshake mechanism. if device successfully decodes sync packet
*	from an expected mac address, device is good to start with the test sequece.
*	Right now only two kinds of test are downloaded to device.
*		ota_tx
*		ota_rx
*
*	ota_tx/ota_rx takes in arguments as
*	test chan bandwidth contrlchan rates stf txant rxant tx_ifs tx_len num_pkt pwrctrl
*		start:delta:end
*
*	Cmd flow file should have a test setup information like various mac address, sycn timeout.
*	Format is:  synchtimeoout(seconds) synchbreak/loop synchmac txmac rxmac
*
* In tethered mode, test flow is passed down in form of wl iovars through batching mode
*	Sequence of operation is
*	test_stream start	[start batching mode operation]
*	test_stream test_setup  [where test_setup is of the same format in cmd flow file]
*	test_stream test_cmd	[should be of same format of ota_tx /ota_rx in cmd_flow file]
*	test_stream stop	[stops batching mode operation and downloads the file to dongle]
*$Id: wlc_ota_test.h 708017 2017-06-29 14:11:45Z $
* Copyright 2020 Broadcom
*
* This program is the proprietary software of Broadcom and/or
* its licensors, and may only be used, duplicated, modified or distributed
* pursuant to the terms and conditions of a separate, written license
* agreement executed between you and Broadcom (an "Authorized License").
* Except as set forth in an Authorized License, Broadcom grants no license
* (express or implied), right to use, or waiver of any kind with respect to
* the Software, and Broadcom expressly reserves all rights in and to the
* Software and all intellectual property rights therein.  IF YOU HAVE NO
* AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
* WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
* THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
* constitutes the valuable trade secrets of Broadcom, and you shall use
* all reasonable efforts to protect the confidentiality thereof, and to
* use this information only in connection with your use of Broadcom
* integrated circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
* "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
* REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
* OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
* DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
* NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
* ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
* CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
* OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
* BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
* SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
* IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
* IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
* ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
* OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
* NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
*/

#ifndef _wlc_ota_test_h_
#define _wlc_ota_test_h_
extern ota_test_info_t *  wlc_ota_test_attach(wlc_info_t* wlc);
extern void wlc_ota_test_detach(ota_test_info_t * ota_info);
#endif /* _wlc_ota_test_h_ */
