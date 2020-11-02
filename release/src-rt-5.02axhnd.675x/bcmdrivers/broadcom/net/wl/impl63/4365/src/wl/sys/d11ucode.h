/*
 * Microcode declarations for Broadcom 802.11abg
 * Networking Adapter Device Driver.
 *
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
 * $Id: d11ucode.h 769472 2018-11-16 00:09:53Z $
 */

/* ucode and inits structure */
typedef struct d11init {
	uint16	addr;
	uint16	size;
	uint32	value;
} d11init_t;

typedef struct d11axiinit {
	uint32	addr;
	uint32	size;
	uint32	value;
} d11axiinit_t;

/* ucode and inits */
extern CONST uint32 d11ucode4[];
extern CONST uint d11ucode4sz;
extern CONST uint32 d11ucode5[];
extern CONST uint d11ucode5sz;
#if defined(MBSS)
extern CONST uint32 d11ucode9[];
extern CONST uint d11ucode9sz;
#endif // endif
extern CONST uint32 d11ucode11[], d11ucode_2w11[];
extern CONST uint d11ucode11sz, d11ucode_2w11sz;
extern CONST uint32 d11ucode13[], d11ucode_2w13[];
extern CONST uint d11ucode13sz, d11ucode_2w13sz;
extern CONST uint32 d11ucode14[];
extern CONST uint d11ucode14sz;
extern CONST uint32 d11ucode15[], d11ucode_2w15[];
extern CONST uint d11ucode15sz, d11ucode_2w15sz;
extern CONST uint32 d11ucode16_lp[];
extern CONST uint d11ucode16_lpsz;
extern CONST uint8 d11ucode16_sslpn[];
extern CONST uint d11ucode16_sslpnsz;
extern CONST uint8 d11ucode16_sslpn_nobt[];
extern CONST uint d11ucode16_sslpn_nobtsz;
extern CONST uint32 d11ucode16_mimo[];
extern CONST uint d11ucode16_mimosz;
extern CONST uint32 d11ucode17[];
extern CONST uint d11ucode17sz;
extern CONST uint32 d11ucode19_lp[];
extern CONST uint d11ucode19_lpsz;
extern CONST uint8 d11ucode19_sslpn[];
extern CONST uint d11ucode19_sslpnsz;
extern CONST uint8 d11ucode19_sslpn_nobt[];
extern CONST uint d11ucode19_sslpn_nobtsz;
extern CONST uint32 d11ucode19_mimo[];
extern CONST uint d11ucode19_mimosz;
extern CONST uint32 d11ucode20_lp[];
extern CONST uint d11ucode20_lpsz;
extern CONST uint8 d11ucode20_sslpn[];
extern CONST uint d11ucode20_sslpnsz;
extern CONST uint8 d11ucode20_sslpn_nobt[];
extern CONST uint d11ucode20_sslpn_nobtsz;
extern CONST uint32 d11ucode20_mimo[];
extern CONST uint d11ucode20_mimosz;
extern CONST uint8 d11ucode21_sslpn_nobt[];
extern CONST uint8 d11ucode21_sslpn[];
extern CONST uint d11ucode21_sslpn_nobtsz;
extern CONST uint d11ucode21_sslpnsz;
extern CONST uint32 d11ucode22_mimo[];
extern CONST uint d11ucode22_mimosz;
extern CONST uint8 d11ucode22_sslpn[];
extern CONST uint d11ucode22_sslpnsz;
extern CONST uint32 d11ucode24_mimo[];
extern CONST uint d11ucode24_mimosz;
extern CONST uint8 d11ucode24_lcn[];
extern CONST uint d11ucode24_lcnsz;
extern CONST uint32 d11ucode25_mimo[];
extern CONST uint d11ucode25_mimosz;
extern CONST uint8 d11ucode25_lcn[];
extern CONST uint d11ucode25_lcnsz;
extern CONST uint32 d11ucode26_mimo[];
extern CONST uint d11ucode26_mimosz;
extern CONST uint32 d11ucode29_mimo[];
extern CONST uint d11ucode29_mimosz;
extern CONST uint32 d11ucode30_mimo[];
extern CONST uint d11ucode30_mimosz;
extern CONST uint32 d11ucode31_mimo[];
extern CONST uint d11ucode31_mimosz;
extern CONST uint32 d11ucode32_mimo[];
extern CONST uint d11ucode32_mimosz;
extern CONST uint8 d11ucode33_lcn40[];
extern CONST uint d11ucode33_lcn40sz;
extern CONST uint32 d11ucode34_mimo[];
extern CONST uint d11ucode34_mimosz;
extern CONST uint32 d11ucode36_mimo[];
extern CONST uint d11ucode36_mimosz;
extern CONST uint8 d11ucode37_lcn40[];
extern CONST uint d11ucode37_lcn40sz;
extern CONST uint8 d11ucode38_lcn40[];
extern CONST uint d11ucode38_lcn40sz;

extern CONST uint32 d11ucode40[];
extern CONST uint d11ucode40sz;
extern CONST uint32 d11ucode41[];
extern CONST uint d11ucode41sz;
extern CONST uint32 d11ucode42[];
extern CONST uint d11ucode42sz;
extern CONST uint32 d11ucode43[];
extern CONST uint d11ucode43sz;
extern CONST uint32 d11ucode46[];
extern CONST uint d11ucode46sz;
extern CONST uint32 d11ucode47[];
extern CONST uint d11ucode47sz;
extern CONST uint32 d11ucode48[];
extern CONST uint d11ucode48sz;
extern CONST uint32 d11ucode49[];
extern CONST uint d11ucode49sz;
#ifdef UNRELEASEDCHIP
extern CONST uint32 d11ucode50[];
extern CONST uint d11ucode50sz;
#endif /* UNRELEASEDCHIP */

#if defined(BTCX_ENABLED)
extern CONST uint32 d11ucode_btcx64[];
extern CONST uint d11ucode_btcx64sz;
extern CONST uint32 d11ucodex_btcx64[];
extern CONST uint d11ucodex_btcx64sz;
extern CONST uint32 d11ucode_btcx65[];
extern CONST uint d11ucode_btcx65sz;
extern CONST uint32 d11ucodex_btcx65[];
extern CONST uint d11ucodex_btcx65sz;

extern CONST uint32 d11ucode_btcx_mu64[];
extern CONST uint d11ucode_btcx_mu64sz;
extern CONST uint32 d11ucodex_btcx_mu64[];
extern CONST uint d11ucodex_btcx_mu64sz;
extern CONST uint32 d11ucode_btcx_mu65[];
extern CONST uint d11ucode_btcx_mu65sz;
extern CONST uint32 d11ucodex_btcx_mu65[];
extern CONST uint d11ucodex_btcx_mu65sz;
#if defined(WLCX_ATLAS)
extern CONST uint32 d11ucodex64[];
extern CONST uint d11ucodex64sz;
#endif /* WLCX_ATLAS */
#else
extern CONST uint32 d11ucode64[];
extern CONST uint d11ucode64sz;
extern CONST uint32 d11ucodex64[];
extern CONST uint d11ucodex64sz;
extern CONST uint32 d11ucode65[];
extern CONST uint d11ucode65sz;
extern CONST uint32 d11ucodex65[];
extern CONST uint d11ucodex65sz;

extern CONST uint32 d11ucode_mu64[];
extern CONST uint d11ucode_mu64sz;
extern CONST uint32 d11ucodex_mu64[];
extern CONST uint d11ucodex_mu64sz;
extern CONST uint32 d11ucode_mu65[];
extern CONST uint d11ucode_mu65sz;
extern CONST uint32 d11ucodex_mu65[];
extern CONST uint d11ucodex_mu65sz;
#endif /* BTCX_ENABLED */

#ifdef WLCX_ATLAS
extern CONST uint32 d11ucode_wlcx64[];
extern CONST uint d11ucode_wlcx64sz;
#endif /* WLCX_ATLAS */

extern CONST uint32 d11pcm4[];
extern CONST uint d11pcm4sz;
extern CONST uint32 d11pcm5[];
extern CONST uint d11pcm5sz;

extern CONST d11init_t d11b0g0initvals4[];
extern CONST d11init_t d11b0g0bsinitvals4[];
extern CONST d11init_t d11a0g0initvals4[];
extern CONST d11init_t d11a0g0bsinitvals4[];
extern CONST d11init_t d11b0g0initvals5[];
extern CONST d11init_t d11b0g0bsinitvals5[];
extern CONST d11init_t d11a0g0initvals5[];
extern CONST d11init_t d11a0g1initvals5[];
extern CONST d11init_t d11a0g0bsinitvals5[];
extern CONST d11init_t d11a0g1bsinitvals5[];
#if defined(MBSS)
extern CONST d11init_t d11b0g0initvals9[];
extern CONST d11init_t d11b0g0bsinitvals9[];
extern CONST d11init_t d11a0g0initvals9[];
extern CONST d11init_t d11a0g1initvals9[];
extern CONST d11init_t d11a0g0bsinitvals9[];
extern CONST d11init_t d11a0g1bsinitvals9[];
#endif // endif
extern CONST d11init_t d11n0initvals11[];
extern CONST d11init_t d11n0bsinitvals11[];
extern CONST d11init_t d11lp0initvals13[];
extern CONST d11init_t d11lp0bsinitvals13[];
extern CONST d11init_t d11lp0initvals14[];
extern CONST d11init_t d11lp0bsinitvals14[];
extern CONST d11init_t d11b0g0initvals13[];
extern CONST d11init_t d11b0g0bsinitvals13[];
extern CONST d11init_t d11a0g1initvals13[];
extern CONST d11init_t d11a0g1bsinitvals13[];
extern CONST d11init_t d11lp0initvals15[];
extern CONST d11init_t d11lp0bsinitvals15[];
extern CONST d11init_t d11n0initvals16[];
extern CONST d11init_t d11n0bsinitvals16[];
extern CONST d11init_t d11sslpn0initvals16[];
extern CONST d11init_t d11sslpn0bsinitvals16[];
extern CONST d11init_t d11lp0initvals16[];
extern CONST d11init_t d11lp0bsinitvals16[];
extern CONST d11init_t d11sslpn2initvals17[];
extern CONST d11init_t d11sslpn2bsinitvals17[];
extern CONST d11init_t d11n2initvals19[];
extern CONST d11init_t d11n2bsinitvals19[];
extern CONST d11init_t d11sslpn2initvals19[];
extern CONST d11init_t d11sslpn2bsinitvals19[];
extern CONST d11init_t d11lp2initvals19[];
extern CONST d11init_t d11lp2bsinitvals19[];
extern CONST d11init_t d11n1initvals20[];
extern CONST d11init_t d11n1bsinitvals20[];
extern CONST d11init_t d11sslpn1initvals20[];
extern CONST d11init_t d11sslpn1bsinitvals20[];
extern CONST d11init_t d11lp1initvals20[];
extern CONST d11init_t d11lp1bsinitvals20[];
extern CONST d11init_t d11sslpn3initvals21[];
extern CONST d11init_t d11sslpn3bsinitvals21[];
extern CONST d11init_t d11n0initvals22[];
extern CONST d11init_t d11n0bsinitvals22[];
extern CONST d11init_t d11sslpn4initvals22[];
extern CONST d11init_t d11sslpn4bsinitvals22[];

extern CONST d11init_t d11sslpn1initvals27[];
extern CONST d11init_t d11sslpn1bsinitvals27[];
extern CONST uint8 d11ucode27_sslpn[];
extern CONST uint d11ucode27_sslpnsz;
extern CONST d11init_t d11n0initvals24[];
extern CONST d11init_t d11n0bsinitvals24[];
extern CONST d11init_t d11lcn0initvals24[];
extern CONST d11init_t d11lcn0bsinitvals24[];
extern CONST d11init_t d11n0initvals25[];
extern CONST d11init_t d11n0bsinitvals25[];
extern CONST d11init_t d11n16initvals30[];
extern CONST d11init_t d11n16bsinitvals30[];
extern CONST d11init_t d11lcn0initvals25[];
extern CONST d11init_t d11lcn0bsinitvals25[];
extern CONST d11init_t d11ht0initvals26[];
extern CONST d11init_t d11ht0bsinitvals26[];
extern CONST d11init_t d11ht0initvals29[];
extern CONST d11init_t d11ht0bsinitvals29[];
extern CONST d11init_t d11n0initvals31[];
extern CONST d11init_t d11n0bsinitvals31[];
extern CONST d11init_t d11n22initvals31[];
extern CONST d11init_t d11n22bsinitvals31[];
extern CONST d11init_t d11n18initvals32[];
extern CONST d11init_t d11n18bsinitvals32[];
extern CONST d11init_t d11lcn400initvals33[];
extern CONST d11init_t d11lcn400bsinitvals33[];
extern CONST d11init_t d11n19initvals34[];
extern CONST d11init_t d11n19bsinitvals34[];
extern CONST d11init_t d11n20initvals36[];
extern CONST d11init_t d11n20bsinitvals36[];
extern CONST d11init_t d11lcn406initvals37[];
extern CONST d11init_t d11lcn406bsinitvals37[];
extern CONST d11init_t d11lcn407initvals38[];
extern CONST d11init_t d11lcn407bsinitvals38[];

extern CONST d11init_t d11ac0initvals40[];
extern CONST d11init_t d11ac0bsinitvals40[];
extern CONST d11init_t d11ac2initvals41[];
extern CONST d11init_t d11ac2bsinitvals41[];
extern CONST d11init_t d11ac1initvals42[];
extern CONST d11init_t d11ac1bsinitvals42[];
extern CONST d11init_t d11wakeac1initvals42[];
extern CONST d11init_t d11wakeac1bsinitvals42[];
extern CONST d11init_t d11ac3initvals43[];
extern CONST d11init_t d11ac3bsinitvals43[];
extern CONST d11init_t d11ac6initvals46[];
extern CONST d11init_t d11ac6bsinitvals46[];
extern CONST d11init_t d11ac7initvals47[];
extern CONST d11init_t d11ac7bsinitvals47[];
extern CONST d11init_t d11ac8initvals48[];
extern CONST d11init_t d11ac8bsinitvals48[];
extern CONST d11init_t d11ac9initvals49[];
extern CONST d11init_t d11ac9bsinitvals49[];
#ifdef UNRELEASEDCHIP
extern CONST d11init_t d11ac12initvals50[];
extern CONST d11init_t d11ac12bsinitvals50[];
extern CONST d11init_t d11ac12initvals50core1[];
extern CONST d11init_t d11ac12bsinitvals50core1[];
#endif /* UNRELEASEDCHIP */

extern CONST d11init_t d11ac32initvals64[];
extern CONST d11init_t d11ac32bsinitvals64[];
extern CONST d11init_t d11ac32initvalsx64[];
extern CONST d11init_t d11ac32bsinitvalsx64[];
extern CONST d11init_t d11ac33initvals65[];
extern CONST d11init_t d11ac33bsinitvals65[];
extern CONST d11init_t d11ac33initvalsx65[];
extern CONST d11init_t d11ac33bsinitvalsx65[];

extern CONST d11init_t d11waken0initvals12[];
extern CONST d11init_t d11waken0bsinitvals12[];
extern CONST uint32 d11aeswakeucode12[];
extern CONST uint32 d11ucode_wowl12[];
extern CONST uint d11ucode_wowl12sz;
extern CONST uint d11aeswakeucode12sz;
extern CONST d11init_t d11wakelp0initvals15[];
extern CONST d11init_t d11wakelp0bsinitvals15[];
extern CONST uint32 d11aeswakeucode15[];
extern CONST uint32 d11ucode_wowl15[];
extern CONST uint d11ucode_wowl15sz;
extern CONST uint d11aeswakeucode15sz;
extern CONST d11init_t d11waken0initvals16[];
extern CONST d11init_t d11waken0bsinitvals16[];
extern CONST d11init_t d11wakelcn0initvals24[];
extern CONST d11init_t d11wakelcn0bsinitvals24[];
extern CONST d11init_t d11waken0initvals24[];
extern CONST d11init_t d11waken0bsinitvals24[];
extern CONST d11init_t d11waken0initvals26[];
extern CONST d11init_t d11waken0bsinitvals26[];
extern CONST d11init_t d11waken0initvals30[];
extern CONST d11init_t d11waken0bsinitvals30[];
extern CONST d11init_t d11wakelcn403initvals33[];
extern CONST d11init_t d11wakelcn403bsinitvals33[];

extern CONST uint32 d11aeswakeucode16_lp[];
extern CONST uint32 d11aeswakeucode16_sslpn[];
extern CONST uint32 d11aeswakeucode16_mimo[];
extern CONST uint32 d11aeswakeucode24_lcn[];
extern CONST uint32 d11aeswakeucode24_mimo[];
extern CONST uint32 d11aeswakeucode26_mimo[];
extern CONST uint32 d11aeswakeucode30_mimo[];
extern CONST uint32 d11aeswakeucode42[];

extern CONST uint32 d11ucode_wowl16_lp[];
extern CONST uint32 d11ucode_wowl16_sslpn[];
extern CONST uint32 d11ucode_wowl16_mimo[];
extern CONST uint32 d11ucode_wowl24_lcn[];
extern CONST uint32 d11ucode_wowl24_mimo[];
extern CONST uint32 d11ucode_wowl26_mimo[];
extern CONST uint32 d11ucode_wowl30_mimo[];
extern CONST uint32 d11ucode_wowl42[];
extern CONST uint32 d11aeswakeucode33_lcn40[];
extern CONST uint32 d11ucode_wowl33_lcn40[];

extern CONST uint d11ucode_wowl16_lpsz;
extern CONST uint d11ucode_wowl16_sslpnsz;
extern CONST uint d11ucode_wowl16_mimosz;
extern CONST uint d11ucode_wowl24_lcnsz;
extern CONST uint d11ucode_wowl24_mimosz;
extern CONST uint d11ucode_wowl26_mimosz;
extern CONST uint d11ucode_wowl30_mimosz;
extern CONST uint d11ucode_wowl42sz;
extern CONST uint d11aeswakeucode33_lcn40sz;
extern CONST uint d11ucode_wowl33_lcn40sz;

extern CONST uint d11aeswakeucode16_lpsz;
extern CONST uint d11aeswakeucode16_sslpnsz;
extern CONST uint d11aeswakeucode16_mimosz;
extern CONST uint d11aeswakeucode24_lcnsz;
extern CONST uint d11aeswakeucode24_mimosz;
extern CONST uint d11aeswakeucode26_mimosz;
extern CONST uint d11aeswakeucode30_mimosz;
extern CONST uint d11aeswakeucode42sz;

#ifdef SAMPLE_COLLECT
extern CONST uint32 d11sampleucode16[];
extern CONST uint d11sampleucode16sz;
#endif // endif

/* BOM info for each ucode file */
extern CONST uint32 d11ucode_ge24_bommajor;
extern CONST uint32 d11ucode_ge24_bomminor;
extern CONST uint32 d11ucode_gt15_bommajor;
extern CONST uint32 d11ucode_gt15_bomminor;
extern CONST uint32 d11ucode_le15_bommajor;
extern CONST uint32 d11ucode_le15_bomminor;
extern CONST uint32 d11ucode_2w_bommajor;
extern CONST uint32 d11ucode_2w_bomminor;
extern CONST uint32 d11ucode_wowl_bommajor;
extern CONST uint32 d11ucode_wowl_bomminor;

#ifdef WLP2P_UCODE
extern CONST uint32 d11ucode_p2p_bommajor;
extern CONST uint32 d11ucode_p2p_bomminor;
extern CONST uint32 d11ucode_p2p15[];
extern CONST uint d11ucode_p2p15sz;
extern CONST uint32 d11ucode_p2p16_lp[];
extern CONST uint d11ucode_p2p16_lpsz;
extern CONST uint8 d11ucode_p2p16_sslpn[];
extern CONST uint d11ucode_p2p16_sslpnsz;
extern CONST uint8 d11ucode_p2p16_sslpn_nobt[];
extern CONST uint d11ucode_p2p16_sslpn_nobtsz;
extern CONST uint32 d11ucode_p2p16_mimo[];
extern CONST uint d11ucode_p2p16_mimosz;
extern CONST uint32 d11ucode_p2p17_mimo[];
extern CONST uint d11ucode_p2p17_mimosz;
extern CONST uint8 d11ucode_p2p19_sslpn[];
extern CONST uint d11ucode_p2p19_sslpnsz;
extern CONST uint8 d11ucode_p2p19_sslpn_nobt[];
extern CONST uint d11ucode_p2p19_sslpn_nobtsz;
extern CONST uint8 d11ucode_p2p20_sslpn[];
extern CONST uint d11ucode_p2p20_sslpnsz;
extern CONST uint8 d11ucode_p2p20_sslpn_nobt[];
extern CONST uint d11ucode_p2p20_sslpn_nobtsz;
extern CONST uint8 d11ucode_p2p21_sslpn[];
extern CONST uint d11ucode_p2p21_sslpnsz;
extern CONST uint8 d11ucode_p2p21_sslpn_nobt[];
extern CONST uint d11ucode_p2p21_sslpn_nobtsz;
extern CONST uint8 d11ucode_p2p22_sslpn[];
extern CONST uint d11ucode_p2p22_sslpnsz;
extern CONST uint32 d11ucode_p2p22_mimo[];
extern CONST uint d11ucode_p2p22_mimosz;
extern CONST uint32 d11ucode_p2p24_mimo[];
extern CONST uint d11ucode_p2p24_mimosz;
extern CONST uint8 d11ucode_p2p24_lcn[];
extern CONST uint d11ucode_p2p24_lcnsz;
extern CONST uint32 d11ucode_p2p25_mimo[];
extern CONST uint d11ucode_p2p25_mimosz;
extern CONST uint8 d11ucode_p2p25_lcn[];
extern CONST uint d11ucode_p2p25_lcnsz;
extern CONST uint32 d11ucode_p2p26_mimo[];
extern CONST uint d11ucode_p2p26_mimosz;
extern CONST uint8 d11ucode_p2p26_lcn[];
extern CONST uint d11ucode_p2p26_lcnsz;
extern CONST uint32 d11ucode_p2p29_mimo[];
extern CONST uint d11ucode_p2p29_mimosz;
extern CONST uint32 d11ucode_p2p30_mimo[];
extern CONST uint d11ucode_p2p30_mimosz;
extern CONST uint32 d11ucode_p2p31_mimo[];
extern CONST uint d11ucode_p2p31_mimosz;
extern CONST uint32 d11ucode_p2p32_mimo[];
extern CONST uint d11ucode_p2p32_mimosz;
extern CONST uint8 d11ucode_p2p33_lcn40[];
extern CONST uint d11ucode_p2p33_lcn40sz;
extern CONST uint32 d11ucode_p2p34_mimo[];
extern CONST uint d11ucode_p2p34_mimosz;
extern CONST uint32 d11ucode_p2p36_mimo[];
extern CONST uint d11ucode_p2p36_mimosz;
extern CONST uint8 d11ucode_p2p37_lcn40[];
extern CONST uint d11ucode_p2p37_lcn40sz;
extern CONST uint8 d11ucode_p2p38_lcn40[];
extern CONST uint d11ucode_p2p38_lcn40sz;
extern CONST uint32 d11ucode_p2p40[];
extern CONST uint d11ucode_p2p40sz;
extern CONST uint32 d11ucode_p2p41[];
extern CONST uint d11ucode_p2p41sz;
extern CONST uint32 d11ucode_p2p42[];
extern CONST uint d11ucode_p2p42sz;
extern CONST uint32 d11ucode_p2p43[];
extern CONST uint d11ucode_p2p43sz;
extern CONST uint32 d11ucode_p2p46[];
extern CONST uint d11ucode_p2p46sz;
extern CONST uint32 d11ucode_p2p47[];
extern CONST uint d11ucode_p2p47sz;
extern CONST uint32 d11ucode_p2p48[];
extern CONST uint d11ucode_p2p48sz;
extern CONST uint32 d11ucode_p2p49[];
extern CONST uint d11ucode_p2p49sz;
#ifdef UNRELEASEDCHIP
extern CONST uint32 d11ucode_p2p50[];
extern CONST uint d11ucode_p2p50sz;
#endif /* UNRELEASEDCHIP */
#if defined(BTCX_ENABLED)
extern CONST uint32 d11ucode_btcx_p2p64[];
extern CONST uint d11ucode_btcx_p2p64sz;
extern CONST uint32 d11ucode_btcx_p2p65[];
extern CONST uint d11ucode_btcx_p2p65sz;
#else
extern CONST uint32 d11ucode_p2p64[];
extern CONST uint d11ucode_p2p64sz;
extern CONST uint32 d11ucode_p2p65[];
extern CONST uint d11ucode_p2p65sz;
#endif /* BTCX_ENABLED */
#endif /* WLP2P_UCODE */
