/*
   Copyright (c) 2022 Broadcom Corporation
   All Rights Reserved

<:label-BRCM:2022:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
*******************************************************************************
* File Name  : bp3_license.h
*
* Description: Broadcom License API Header File
*
*******************************************************************************
*/

#ifndef _BP3_LICENSE_H_
#define _BP3_LICENSE_H_

#define BP3APP "bp3"

struct bp3_feature {
    const char *name;
    const char *description;
};

/* BP3 Features Names and Descriptions*/
static const struct bp3_feature feature_lookup_table[] = {
	/* Group 0:7 */
	{"BP3_FEATURE_SPEED_SERVICE", "Speed Service"},
	{"BP3_FEATURE_SW_TCP_SPEED_SERVICE", "Software TCP Speed Service"},
	{"BP3_FEATURE_SERVICE_QUEUE", "SERVICE_QUEUE ( DPI QoS)"},
	{"BP3_FEATURE_AQM", "AQM (Advanced Active queue management algorithm)"},
	{"BP3_FEATURE_OVS", "OVS (Enable OVS acceleration)"},
	{"BP3_FEATURE_SINGLE_LINE_XDSL", "Single Line xDSL"},
	{"BP3_FEATURE_BONDED_XDSL", "Bonded xDSL"},
	{"BP3_FEATURE_GFAST_106", "G.Fast 106"},

	/* Group 8:15 */
	{"BP3_FEATURE_GFAST_106_212", "G.Fast 106/212"},
	{"BP3_FEATURE_ACL", "ACL"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_GDX_3RDPARTY", "GDX 3RDPARTY"},
	{"BP3_FEATURE_VOICE_2_CHANNELS", "Voice-2 Channels"},
	{"BP3_FEATURE_VOICE_4_CHANNELS", "Voice-4 Channels"},
	{"BP3_FEATURE_VOICE_8_CHANNELS", "Voice-8 Channels"},
	{"BP3_FEATURE_VOICE_16_CHANNELS", "Voice-16 Channels"},

	/* Group 16:23 */
	{"BP3_FEATURE_FFV", "FFV"},
	{"BP3_FEATURE_HTOA", "HOST TRAFFIC OFFLOAD ASSIST"},
	{"BP3_FEATURE_DDOS", "DDoS Attack Mitigation"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 24:31 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 32:39 */
	{"BP3_FEATURE_RSA_4K", "RSA 4K"},
	{"BP3_FEATURE_SHA256", "SHA256"},
	{"BP3_FEATURE_ECC", "ECC"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_SPU_CRYPTO2", "SPU/Crypto2"},
	{"BP3_FEATURE_SHA512", "SHA512"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 40:47 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_SYNCE", "SyncE"},

	/* Group 48:55 */
	{"BP3_FEATURE_PCIE0", "PCIe0"},
	{"BP3_FEATURE_PCIE1", "PCIe1"},
	{"BP3_FEATURE_PCIE2", "PCIe2"},
	{"BP3_FEATURE_PCIE3", "PCIe3"},
	{"BP3_FEATURE_EGPHY0", "EGPHY0"},
	{"BP3_FEATURE_EGPHY1", "EGPHY1"},
	{"BP3_FEATURE_EGPHY2", "EGPHY2"},
	{"BP3_FEATURE_EGPHY3", "EGPHY3"},

	/* Group 56:63 */
	{"BP3_FEATURE_ENABLE_ML", "Enable ML"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 64:71 */
	{"BP3_FEATURE_DDR3", "DDR3"},
	{"BP3_FEATURE_DDR4", "DDR4"},
	{"BP3_FEATURE_LPDDR4", "LPDDR4"},
	{"BP3_FEATURE_LPDDR4X", "LPDDR4X"},
	{"BP3_FEATURE_LPDDR5", "LPDDR5"},
	{"BP3_FEATURE_1PPS", "1PPS"},
	{"BP3_FEATURE_SYNC_8KHZ", "Sync 8KHz"},
	{"BP3_FEATURE_USB0", "USB0"},

	/* Group 72:79 */
	{"BP3_FEATURE_USB1", "USB1"},
	{"BP3_FEATURE_GPON", "GPON"},
	{"BP3_FEATURE_10G_GPON", "10G GPON"},
	{"BP3_FEATURE_EPON", "EPON"},
	{"BP3_FEATURE_10G_EPON", "10G EPON"},
	{"BP3_FEATURE_DPOE", "DPoE"},
	{"BP3_FEATURE_GINT", "G.Int"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 80:87 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 88:95 */
	{"BP3_FEATURE_SLAN_2_5", "SLAN 2.5G"},
	{"BP3_FEATURE_SLAN_5", "SLAN 5G"},
	{"BP3_FEATURE_SLAN_10", "SLAN 10G"},
	{"BP3_FEATURE_SLAN_4_2_5", "SLAN 4*2.5G"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 96:103 */
	{"BP3_FEATURE_QLAN0_2_5", "Reserved"},
	{"BP3_FEATURE_QLAN0_5", "Reserved"},
	{"BP3_FEATURE_QLAN0_10", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 104:111 */
	{"BP3_FEATURE_QLAN1_2_5", "Reserved"},
	{"BP3_FEATURE_QLAN1_5", "Reserved"},
	{"BP3_FEATURE_QLAN1_10", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 112:119 */
	{"BP3_FEATURE_QLAN2_2_5", "Reserved"},
	{"BP3_FEATURE_QLAN2_5", "Reserved"},
	{"BP3_FEATURE_QLAN2_10", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 120:127 */
	{"BP3_FEATURE_QLAN3_2_5", "Reserved"},
	{"BP3_FEATURE_QLAN3_5", "Reserved"},
	{"BP3_FEATURE_QLAN3_10", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 128:135 */
	{"BP3_FEATURE_XGPHY0_2_5", "Reserved"},
	{"BP3_FEATURE_XGPHY0_2_5_MACSEC", "Reserved"},
	{"BP3_FEATURE_XGPHY0_5", "Reserved"},
	{"BP3_FEATURE_XGPHY0_5_MACSEC", "Reserved"},
	{"BP3_FEATURE_XGPHY0_10", "Reserved"},
	{"BP3_FEATURE_XGPHY0_10_MACSEC", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 136:143 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 144:151 */
	{"BP3_FEATURE_XGPHY1_2_5", "Reserved"},
	{"BP3_FEATURE_XGPHY1_2_5_MACSEC", "Reserved"},
	{"BP3_FEATURE_XGPHY1_5", "Reserved"},
	{"BP3_FEATURE_XGPHY1_5_MACSEC", "Reserved"},
	{"BP3_FEATURE_XGPHY1_10", "Reserved"},
	{"BP3_FEATURE_XGPHY1_10_MACSEC", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 152:159 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 160:167 */
	{"BP3_FEATURE_ACTIVE_ETH_2_5", "Reserved"},
	{"BP3_FEATURE_ACTIVE_ETH_2_5_MACSEC", "Reserved"},
	{"BP3_FEATURE_ACTIVE_ETH_5", "Reserved"},
	{"BP3_FEATURE_ACTIVE_ETH_5_MACSEC", "Reserved"},
	{"BP3_FEATURE_ACTIVE_ETH_10", "Reserved"},
	{"BP3_FEATURE_ACTIVE_ETH_10_MACSEC", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 168:175 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 176:183 */
	{"BP3_FEATURE_MAX_NUM_CPU", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 184:191 */
	{"BP3_FEATURE_MAX_CPU_FREQ", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 192:199 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 200:207 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 208:215 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 216:223 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 224:231 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 232:239 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 240:247 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},

	/* Group 248:255 */
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"},
	{"BP3_FEATURE_RESERVED", "Reserved"} // ( BP3_FEATURE_MAX - 1 )
};

/* BP3 Features ID's*/
typedef enum {

    //SW features
    BP3_FEATURE_SPEED_SERVICE = 0,
    BP3_FEATURE_SW_TCP_SPEED_SERVICE,
    BP3_FEATURE_SERVICE_QUEUE,
    BP3_FEATURE_AQM,
    BP3_FEATURE_OVS,
    BP3_FEATURE_SINGLE_LINE_XDSL,
    BP3_FEATURE_BONDED_XDSL,
    BP3_FEATURE_GFAST_106,
    BP3_FEATURE_GFAST_106_212, 
    BP3_FEATURE_ACL,
    BP3_FEATURE_GDX_3RDPARTY = 11,
    BP3_FEATURE_VOICE_2_CHANNELS,
    BP3_FEATURE_VOICE_4_CHANNELS,
    BP3_FEATURE_VOICE_8_CHANNELS,
    BP3_FEATURE_VOICE_16_CHANNELS,
    BP3_FEATURE_FFV,
    BP3_FEATURE_HTOA,
    BP3_FEATURE_DDOS,

    //Security features
    BP3_FEATURE_ENABLE_RSA_4K = 32,
    BP3_FEATURE_ENABLE_SHA_256,
    BP3_FEATURE_ENABLE_ECC,
    BP3_FEATURE_SPU_CRYPTO2 = 36,
    BP3_FEATURE_SHA_512,

    //HW
    BP3_FEATURE_SYNCE = 47,
    BP3_FEATURE_PCIE0,
    BP3_FEATURE_PCIE1,
    BP3_FEATURE_PCIE2,
    BP3_FEATURE_PCIE3,
    BP3_FEATURE_EGPHY0,
    BP3_FEATURE_EGPHY1,
    BP3_FEATURE_EGPHY2,
    BP3_FEATURE_EGPHY3,
    BP3_FEATURE_ENABLE_ML,
    BP3_FEATURE_DDR3 = 64,
    BP3_FEATURE_DDR4,
    BP3_FEATURE_LPDDR4,
    BP3_FEATURE_LPDDR4X,
    BP3_FEATURE_LPDDR5,
    BP3_FEATURE_1PPS,
    BP3_FEATURE_SYNC_8KHZ,
    BP3_FEATURE_USB0,
    BP3_FEATURE_USB1,
    BP3_FEATURE_GPON,
    BP3_FEATURE_10G_GPON,
    BP3_FEATURE_EPON,
    BP3_FEATURE_10G_EPON,
    BP3_FEATURE_DPOE,
    BP3_FEATURE_GINT,
    BP3_FEATURE_SLAN_2_5 = 88,
    BP3_FEATURE_SLAN_5,
    BP3_FEATURE_SLAN_10,
    BP3_FEATURE_SLAN_4_2_5,
    BP3_FEATURE_QLAN0_2_5 = 96,
    BP3_FEATURE_QLAN0_5,
    BP3_FEATURE_QLAN0_10,
    BP3_FEATURE_QLAN1_2_5 = 104,
    BP3_FEATURE_QLAN1_5,
    BP3_FEATURE_QLAN1_10,
    BP3_FEATURE_QLAN2_2_5 = 112,
    BP3_FEATURE_QLAN2_5,
    BP3_FEATURE_QLAN2_10,
    BP3_FEATURE_QLAN3_2_5 = 120,
    BP3_FEATURE_QLAN3_5,
    BP3_FEATURE_QLAN3_10,
    BP3_FEATURE_XGPHY0_2_5 = 128,
    BP3_FEATURE_XGPHY0_2_5_MACSEC,
    BP3_FEATURE_XGPHY0_5,
    BP3_FEATURE_XGPHY0_5_MACSEC,
    BP3_FEATURE_XGPHY0_10,
    BP3_FEATURE_XGPHY0_10_MACSEC,
    BP3_FEATURE_XGPHY1_2_5 = 144,
    BP3_FEATURE_XGPHY1_2_5_MACSEC,
    BP3_FEATURE_XGPHY1_5,
    BP3_FEATURE_XGPHY1_5_MACSEC,
    BP3_FEATURE_XGPHY1_10,
    BP3_FEATURE_XGPHY1_10_MACSEC,
    BP3_FEATURE_ACTIVE_ETH_2_5 = 160,
    BP3_FEATURE_ACTIVE_ETH_2_5_MACSEC,
    BP3_FEATURE_ACTIVE_ETH_5,
    BP3_FEATURE_ACTIVE_ETH_5_MACSEC,
    BP3_FEATURE_ACTIVE_ETH_10,
    BP3_FEATURE_ACTIVE_ETH_10_MACSEC,
    BP3_FEATURE_MAX_NUM_CPU = 176,
    BP3_FEATURE_MAX_CPU_FREQ = 184,

    BP3_FEATURE_MAX = 256
}bp3_license_feature_t;

static const char* bp3_get_feature_name(bp3_license_feature_t bp3_feature)  __attribute__((unused));
static const char* bp3_get_feature_name(bp3_license_feature_t bp3_feature)
{
    if ( bp3_feature >= BP3_FEATURE_MAX ) {
        return 0;
    }

    return feature_lookup_table[bp3_feature].name;
}

static const char* bp3_get_feature_description(bp3_license_feature_t bp3_feature)  __attribute__((unused));
static const char* bp3_get_feature_description(bp3_license_feature_t bp3_feature)
{
    if ( bp3_feature >= BP3_FEATURE_MAX ) {
        return 0;
    }

    return feature_lookup_table[bp3_feature].description;
}

#ifdef __KERNEL__
#include <linux/bcm_log.h>
static inline int bcm_license_check(bp3_license_feature_t bp3_feature)
{
    bcmFun_t *cb = bcmFun_get(BCM_FUN_ID_LICENSE_CHECK);
    return cb ? cb(&bp3_feature) : -1;
}

static inline int bcm_license_check_msg(bp3_license_feature_t bp3_feature)
{
    int ret = bcm_license_check(bp3_feature);
    const char *name = bp3_get_feature_name(bp3_feature);

    if (ret > 0)
        return ret;

    printk("+---------------------------------------------------+\n");
    printk(" No valid license for %s found\n", name);
    printk("+---------------------------------------------------+\n");

    return ret;
}

#else

#include <stdio.h>
#include <stdlib.h>

static int bcm_license_check(bp3_license_feature_t bp3_feature) __attribute__((unused));
static int bcm_license_check(bp3_license_feature_t bp3_feature)
{
    char cmdl[256];

    if ( bp3_feature >= BP3_FEATURE_MAX ) {
        return -1;
    }

    snprintf(cmdl, sizeof(cmdl), "bp3 status -feature %d | grep \"ENABLED\"", bp3_feature);

    return ((0 == system(cmdl)) ? 1 : 0);
}

#endif

#endif /* _BP3_LICENSE_H_ */
