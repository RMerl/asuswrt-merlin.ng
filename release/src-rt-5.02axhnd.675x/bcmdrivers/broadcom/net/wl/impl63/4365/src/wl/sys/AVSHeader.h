/*
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
 *
 * Created by Pete on 1/4/05.
 * Copyright 2005 Apple Computer, Inc. All rights reserved.
 *
 * $Id: AVSHeader.h 708017 2017-06-29 14:11:45Z $
 */
/* FILE-CSTYLED */

/************************************************************************
AVS Capture Frame Format
Version 2

1. Introduction
The original header format for "monitor mode" or capturing frames was
a considerable hack.  The document covers a redesign of that format.

2. Frame Format
All sniff frames follow the same format:

	Offset	Name		Size		Description
	--------------------------------------------------------------------
	0	CaptureHeader			AVS capture metadata header
	64	802.11Header	[10-30]		802.11 frame header
	??	802.11Payload	[0-2312]	802.11 frame payload
	??	802.11FCS	4		802.11 frame check sequence

Note that the header and payload are variable length and the payload
may be empty.

If the hardware does not supply the FCS to the driver, then the frame shall
have a FCS of 0xFFFFFFFF.

3. Byte Order
All multibyte fields of the capture header are in "network" byte
order.  The "host to network" and "network to host" functions should
work just fine.  All the remaining multibyte fields are ordered
according to their respective standards.

4. Capture Header Format
The following fields make up the AVS capture header:

	Offset	Name		Type
	------------------------------
	0	version		uint32
	4	length		uint32
	8	mactime		uint64
	16	hosttime	uint64
	24	phytype		uint32
	28	channel		uint32
	32	datarate	uint32
	36	antenna		uint32
	40	priority	uint32
	44	ssi_type	uint32
	48	ssi_signal	int32
	52	ssi_noise	int32
	56	preamble	uint32
	60	encoding	uint32
	------------------------------
	64

The following subsections detail the fields of the capture header.

4.1 version
The version field identifies this type of frame as a subtype of
ETH_P_802111_CAPTURE as received by an ARPHRD_IEEE80211_PRISM or
an ARPHRD_IEEE80211_CAPTURE device.  The value of this field shall be
0x80211001.  As new revisions of this header are necessary, we can
increment the version appropriately.

4.2 length
The length field contains the length of the entire AVS capture header,
in bytes.

4.3 mactime
Many WLAN devices supply a relatively high resolution frame reception
time value.  This field contains the value supplied by the device.  If
the device does not supply a receive time value, this field shall be
set to zero.  The units for this field are nanoseconds.

4.4 hosttime
The hosttime field is set to the current value of the host maintained
clock variable when the frame is received.

4.5 phytype
The phytype field identifies what type of PHY is employed by the WLAN
device used to capture this frame.  The valid values are:

	PhyType			Value
	----------------------------------
	phytype_fhss_dot11_97	1
	phytype_dsss_dot11_97	2
	phytype_irbaseband	3
	phytype_dsss_dot11_b	4
	phytype_pbcc_dot11_b	5
	phytype_ofdm_dot11_g	6
	phytype_pbcc_dot11_g	7
	phytype_ofdm_dot11_a	8

4.6 channel
For all PHY types except FH, this field is just an unsigned integer
and will be set to the current receiver channel number at the time
the frame was received.  For frequency hopping radios, this field
is broken in to the following subfields:

	Byte	Subfield
	------------------------
	Byte0	Hop Set
	Byte1	Hop Pattern
	Byte2	Hop Index
	Byte3	reserved

4.7 datarate
The data rate field contains the rate at which the frame was received
in units of 100kbps.

4.8 antenna
For WLAN devices that indicate the receive antenna for each frame, the
antenna field shall contain an index value into the dot11AntennaList.
If the device does not indicate a receive antenna value, this field
shall be set to zero.

4.9 priority
The priority field indicates the receive priority of the frame.  The
value is in the range [0-15] with the value 0 reserved to indicate
contention period and the value 6 reserved to indicate contention free
period.

4.10 ssi_type
The ssi_type field is used to indicate what type of signal strength
information is present: "None", "Normalized RSSI" or "dBm".  "None"
indicates that the underlying WLAN device does not supply any signal
strength at all and the ssi_* values are unset.  "Normalized RSSI"
values are integers in the range [0-1000] where higher numbers
indicate stronger signal.  "dBm" values indicate an actual signal
strength measurement quantity and are usually in the range [-108 - 10].
The following values indicate the three types:

	Value	Description
	---------------------------------------------
	0	None
	1	Normalized RSSI
	2	dBm
	3 	Raw RSSI

4.11 ssi_signal
The ssi_signal field contains the signal strength value reported by
the WLAN device for this frame.  Note that this is a signed quantity
and if the ssi_type value is "dBm" that the value may be negative.

4.12 ssi_noise
The ssi_noise field contains the noise or "silence" value reported by
the WLAN device.  This value is commonly defined to be the "signal
strength reported immediately prior to the baseband processor lock on
the frame preamble".  If the hardware does not provide noise data, this
shall equal 0xffffffff.

4.12 preamble
For PHYs that support variable preamble lengths, the preamble field
indicates the preamble type used for this frame.  The values are:

	Value	Description
	---------------------------------------------
	0	Undefined
	1	Short Preamble
	2	Long Preamble

4.13 encoding
This specifies the encoding of the received packet.  For PHYs that support
multiple encoding types, this will tell us which one was used.

	Value	Description
	---------------------------------------------
	0	Unknown
	1	CCK
	2	PBCC
	3	OFDM
************************************************************************/

#include <sys/types.h>

#ifndef _AVSHEADER_H_
#define _AVSHEADER_H_

#define P80211CAPTURE_VERSION		0x80211001

/* DLT_IEEE802_11_RADIO_AVS defined in
 * http://www.opensource.apple.com/darwinsource/10.3.7/libpcap-12/libpcap/pcap-bpf.h
 * but not in bpf.h
 */
#define DLT_IEEE802_11_RADIO_AVS	163

struct avs_radio_header
{
	u_int		version; /* P80211CAPTURE_VERSION */
	u_int		length;
	u_int64_t   mactime;
	u_int64_t   hosttime;
	u_int		phytype;
	u_int		channel;
	u_int		datarate;
	u_int		antenna;
	u_int		priority;
	u_int		ssi_type;
	int			ssi_signal;
	int			ssi_noise;
	u_int		preamble;
	u_int		encoding;
};
typedef struct avs_radio_header avs_radio_header;

enum PHY_Type
{
	phytype_fhss_dot11_97 = 1,
	phytype_dsss_dot11_97,
	phytype_irbaseband,
	phytype_dsss_dot11_b,
	phytype_pbcc_dot11_b,
	phytype_ofdm_dot11_g,
	phytype_pbcc_dot11_g,
	phytype_ofdm_dot11_a
};
typedef enum PHY_Type PHY_Type;

enum SSI_Type
{
	ssi_type_none = 0,
	ssi_type_normalized,
	ssi_type_dbm,
	ssi_type_raw_rssi
};
typedef enum SSI_Type SSI_Type;

enum PreambleLength
{
	preamble_undefined = 0,
	preamble_short,
	preamble_long
};
typedef enum PreambleLength PreambleLength;

enum EncodingType
{
	encoding_type_unkown = 0,
	encoding_type_cck,
	encoding_type_pbcc,
	encoding_type_ofdm
};
typedef enum EncodingType EncodingType;

/* Full 802.11 mac header is 30 bytes, we prepend an avs radio header to the mac header */
#define MAC_80211_HEADER_LEN 30

#define AVS_HEADER_LEN (MAC_80211_HEADER_LEN + sizeof(struct avs_radio_header))

#endif /* _AVSHEADER_H_ */
