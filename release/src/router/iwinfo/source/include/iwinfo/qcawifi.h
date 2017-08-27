/*
 * iwinfo - Wireless Information Library - QCAWifi Headers
 *
 *   Copyright (c) 2013 The Linux Foundation. All rights reserved.
 *   Copyright (C) 2009 Jo-Philipp Wich <xm@subsignal.org>
 *
 * The iwinfo library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * The iwinfo library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the iwinfo library. If not, see http://www.gnu.org/licenses/.
 *
 * This file is based on: src/include/iwinfo/madwifi.h
 */

#ifndef __IWINFO_QCAWIFI_H_
#define __IWINFO_QCAWIFI_H_

#include <fcntl.h>

/* The driver is using only one "_" character in front of endianness macros
 * whereas the uClibc is using "__" */
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define _BYTE_ORDER _BIG_ENDIAN
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define _BYTE_ORDER _LITTLE_ENDIAN
#else
#error "__BYTE_ORDER undefined"
#endif

#include "iwinfo.h"
#include "iwinfo/utils.h"
#include "ieee80211_external.h"

int qcawifi_probe(const char *ifname);
int qcawifi_get_mode(const char *ifname, int *buf);
int qcawifi_get_ssid(const char *ifname, char *buf);
int qcawifi_get_bssid(const char *ifname, char *buf);
int qcawifi_get_country(const char *ifname, char *buf);
int qcawifi_get_channel(const char *ifname, int *buf);
int qcawifi_get_frequency(const char *ifname, int *buf);
int qcawifi_get_frequency_offset(const char *ifname, int *buf);
int qcawifi_get_txpower(const char *ifname, int *buf);
int qcawifi_get_txpower_offset(const char *ifname, int *buf);
int qcawifi_get_bitrate(const char *ifname, int *buf);
int qcawifi_get_signal(const char *ifname, int *buf);
int qcawifi_get_noise(const char *ifname, int *buf);
int qcawifi_get_quality(const char *ifname, int *buf);
int qcawifi_get_quality_max(const char *ifname, int *buf);
int qcawifi_get_encryption(const char *ifname, char *buf);
int qcawifi_get_assoclist(const char *ifname, char *buf, int *len);
int qcawifi_get_txpwrlist(const char *ifname, char *buf, int *len);
int qcawifi_get_scanlist(const char *ifname, char *buf, int *len);
int qcawifi_get_freqlist(const char *ifname, char *buf, int *len);
int qcawifi_get_countrylist(const char *ifname, char *buf, int *len);
int qcawifi_get_hwmodelist(const char *ifname, int *buf);
int qcawifi_get_mbssid_support(const char *ifname, int *buf);
int qcawifi_get_hardware_id(const char *ifname, char *buf);
int qcawifi_get_hardware_name(const char *ifname, char *buf);
void qcawifi_close(void);

static const struct iwinfo_ops qcawifi_ops = {
	.channel          = qcawifi_get_channel,
	.frequency        = qcawifi_get_frequency,
	.frequency_offset = qcawifi_get_frequency_offset,
	.txpower          = qcawifi_get_txpower,
	.txpower_offset   = qcawifi_get_txpower_offset,
	.bitrate          = qcawifi_get_bitrate,
	.signal           = qcawifi_get_signal,
	.noise            = qcawifi_get_noise,
	.quality          = qcawifi_get_quality,
	.quality_max      = qcawifi_get_quality_max,
	.mbssid_support   = qcawifi_get_mbssid_support,
	.hwmodelist       = qcawifi_get_hwmodelist,
	.mode             = qcawifi_get_mode,
	.ssid             = qcawifi_get_ssid,
	.bssid            = qcawifi_get_bssid,
	.country          = qcawifi_get_country,
	.hardware_id      = qcawifi_get_hardware_id,
	.hardware_name    = qcawifi_get_hardware_name,
	.encryption       = qcawifi_get_encryption,
	.assoclist        = qcawifi_get_assoclist,
	.txpwrlist        = qcawifi_get_txpwrlist,
	.scanlist         = qcawifi_get_scanlist,
	.freqlist         = qcawifi_get_freqlist,
	.countrylist      = qcawifi_get_countrylist,
	.close            = qcawifi_close
};

#endif
