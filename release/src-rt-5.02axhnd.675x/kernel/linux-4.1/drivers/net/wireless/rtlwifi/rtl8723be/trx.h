/******************************************************************************
 *
 * Copyright(c) 2009-2014  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

#ifndef __RTL8723BE_TRX_H__
#define __RTL8723BE_TRX_H__

#define TX_DESC_SIZE				40
#define TX_DESC_AGGR_SUBFRAME_SIZE		32

#define RX_DESC_SIZE				32
#define RX_DRV_INFO_SIZE_UNIT			8

#define	TX_DESC_NEXT_DESC_OFFSET		40
#define USB_HWDESC_HEADER_LEN			40
#define CRCLENGTH				4

#define SET_TX_DESC_PKT_SIZE(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 0, 16, __val)
#define SET_TX_DESC_OFFSET(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 16, 8, __val)
#define SET_TX_DESC_BMC(__pdesc, __val)			\
	SET_BITS_TO_LE_4BYTE(__pdesc, 24, 1, __val)
#define SET_TX_DESC_HTC(__pdesc, __val)			\
	SET_BITS_TO_LE_4BYTE(__pdesc, 25, 1, __val)
#define SET_TX_DESC_LAST_SEG(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 26, 1, __val)
#define SET_TX_DESC_FIRST_SEG(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 27, 1, __val)
#define SET_TX_DESC_LINIP(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 28, 1, __val)
#define SET_TX_DESC_NO_ACM(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 29, 1, __val)
#define SET_TX_DESC_GF(__pdesc, __val)			\
	SET_BITS_TO_LE_4BYTE(__pdesc, 30, 1, __val)
#define SET_TX_DESC_OWN(__pdesc, __val)			\
	SET_BITS_TO_LE_4BYTE(__pdesc, 31, 1, __val)

#define GET_TX_DESC_PKT_SIZE(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 0, 16)
#define GET_TX_DESC_OFFSET(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 16, 8)
#define GET_TX_DESC_BMC(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 24, 1)
#define GET_TX_DESC_HTC(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 25, 1)
#define GET_TX_DESC_LAST_SEG(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 26, 1)
#define GET_TX_DESC_FIRST_SEG(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 27, 1)
#define GET_TX_DESC_LINIP(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 28, 1)
#define GET_TX_DESC_NO_ACM(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 29, 1)
#define GET_TX_DESC_GF(__pdesc)				\
	LE_BITS_TO_4BYTE(__pdesc, 30, 1)
#define GET_TX_DESC_OWN(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 31, 1)

#define SET_TX_DESC_MACID(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 0, 7, __val)
#define SET_TX_DESC_QUEUE_SEL(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 8, 5, __val)
#define SET_TX_DESC_RDG_NAV_EXT(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 13, 1, __val)
#define SET_TX_DESC_LSIG_TXOP_EN(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 14, 1, __val)
#define SET_TX_DESC_PIFS(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 15, 1, __val)
#define SET_TX_DESC_RATE_ID(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 16, 5, __val)
#define SET_TX_DESC_EN_DESC_ID(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 21, 1, __val)
#define SET_TX_DESC_SEC_TYPE(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 22, 2, __val)
#define SET_TX_DESC_PKT_OFFSET(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+4, 24, 5, __val)


#define SET_TX_DESC_PAID(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 0, 9, __val)
#define SET_TX_DESC_CCA_RTS(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 10, 2, __val)
#define SET_TX_DESC_AGG_ENABLE(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 12, 1, __val)
#define SET_TX_DESC_RDG_ENABLE(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 13, 1, __val)
#define SET_TX_DESC_BAR_RTY_TH(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 14, 2, __val)
#define SET_TX_DESC_AGG_BREAK(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 16, 1, __val)
#define SET_TX_DESC_MORE_FRAG(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 17, 1, __val)
#define SET_TX_DESC_RAW(__pdesc, __val)			\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 18, 1, __val)
#define SET_TX_DESC_SPE_RPT(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 19, 1, __val)
#define SET_TX_DESC_AMPDU_DENSITY(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 20, 3, __val)
#define SET_TX_DESC_BT_INT(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 23, 1, __val)
#define SET_TX_DESC_GID(__pdesc, __val)			\
	SET_BITS_TO_LE_4BYTE(__pdesc+8, 24, 6, __val)


#define SET_TX_DESC_WHEADER_LEN(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 0, 4, __val)
#define SET_TX_DESC_CHK_EN(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 4, 1, __val)
#define SET_TX_DESC_EARLY_MODE(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 5, 1, __val)
#define SET_TX_DESC_HWSEQ_SEL(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 6, 2, __val)
#define SET_TX_DESC_USE_RATE(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 8, 1, __val)
#define SET_TX_DESC_DISABLE_RTS_FB(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 9, 1, __val)
#define SET_TX_DESC_DISABLE_FB(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 10, 1, __val)
#define SET_TX_DESC_CTS2SELF(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 11, 1, __val)
#define SET_TX_DESC_RTS_ENABLE(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 12, 1, __val)
#define SET_TX_DESC_HW_RTS_ENABLE(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 13, 1, __val)
#define SET_TX_DESC_NAV_USE_HDR(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 15, 1, __val)
#define SET_TX_DESC_USE_MAX_LEN(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 16, 1, __val)
#define SET_TX_DESC_MAX_AGG_NUM(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 17, 5, __val)
#define SET_TX_DESC_NDPA(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 22, 2, __val)
#define SET_TX_DESC_AMPDU_MAX_TIME(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+12, 24, 8, __val)


#define SET_TX_DESC_TX_RATE(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 0, 7, __val)
#define SET_TX_DESC_DATA_RATE_FB_LIMIT(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 8, 5, __val)
#define SET_TX_DESC_RTS_RATE_FB_LIMIT(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 13, 4, __val)
#define SET_TX_DESC_RETRY_LIMIT_ENABLE(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 17, 1, __val)
#define SET_TX_DESC_DATA_RETRY_LIMIT(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 18, 6, __val)
#define SET_TX_DESC_RTS_RATE(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+16, 24, 5, __val)


#define SET_TX_DESC_TX_SUB_CARRIER(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 0, 4, __val)
#define SET_TX_DESC_DATA_SHORTGI(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 4, 1, __val)
#define SET_TX_DESC_DATA_BW(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 5, 2, __val)
#define SET_TX_DESC_DATA_LDPC(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 7, 1, __val)
#define SET_TX_DESC_DATA_STBC(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 8, 2, __val)
#define SET_TX_DESC_CTROL_STBC(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 10, 2, __val)
#define SET_TX_DESC_RTS_SHORT(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 12, 1, __val)
#define SET_TX_DESC_RTS_SC(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+20, 13, 4, __val)


#define SET_TX_DESC_TX_BUFFER_SIZE(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+28, 0, 16, __val)

#define GET_TX_DESC_TX_BUFFER_SIZE(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+28, 0, 16)

#define SET_TX_DESC_HWSEQ_EN(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+32, 15, 1, __val)

#define SET_TX_DESC_SEQ(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+36, 12, 12, __val)

#define SET_TX_DESC_TX_BUFFER_ADDRESS(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+40, 0, 32, __val)

#define GET_TX_DESC_TX_BUFFER_ADDRESS(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+40, 0, 32)


#define SET_TX_DESC_NEXT_DESC_ADDRESS(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+48, 0, 32, __val)

#define GET_TX_DESC_NEXT_DESC_ADDRESS(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+48, 0, 32)

#define GET_RX_DESC_PKT_LEN(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 0, 14)
#define GET_RX_DESC_CRC32(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 14, 1)
#define GET_RX_DESC_ICV(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 15, 1)
#define GET_RX_DESC_DRV_INFO_SIZE(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc, 16, 4)
#define GET_RX_DESC_SECURITY(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 20, 3)
#define GET_RX_DESC_QOS(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 23, 1)
#define GET_RX_DESC_SHIFT(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 24, 2)
#define GET_RX_DESC_PHYST(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 26, 1)
#define GET_RX_DESC_SWDEC(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 27, 1)
#define GET_RX_DESC_LS(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 28, 1)
#define GET_RX_DESC_FS(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 29, 1)
#define GET_RX_DESC_EOR(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 30, 1)
#define GET_RX_DESC_OWN(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc, 31, 1)

#define SET_RX_DESC_PKT_LEN(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 0, 14, __val)
#define SET_RX_DESC_EOR(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 30, 1, __val)
#define SET_RX_DESC_OWN(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc, 31, 1, __val)

#define GET_RX_DESC_MACID(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 0, 7)
#define GET_RX_DESC_TID(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 8, 4)
#define GET_RX_DESC_AMSDU(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 13, 1)
#define GET_RX_STATUS_DESC_RXID_MATCH(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+4, 14, 1)
#define GET_RX_DESC_PAGGR(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 15, 1)
#define GET_RX_DESC_A1_FIT(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 16, 4)
#define GET_RX_DESC_CHKERR(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 20, 1)
#define GET_RX_DESC_IPVER(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 21, 1)
#define GET_RX_STATUS_DESC_IS_TCPUDP(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+4, 22, 1)
#define GET_RX_STATUS_DESC_CHK_VLD(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+4, 23, 1)
#define GET_RX_DESC_PAM(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 24, 1)
#define GET_RX_DESC_PWR(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 25, 1)
#define GET_RX_DESC_MD(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 26, 1)
#define GET_RX_DESC_MF(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 27, 1)
#define GET_RX_DESC_TYPE(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 28, 2)
#define GET_RX_DESC_MC(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 30, 1)
#define GET_RX_DESC_BC(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+4, 31, 1)


#define GET_RX_DESC_SEQ(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+8, 0, 12)
#define GET_RX_DESC_FRAG(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+8, 12, 4)
#define GET_RX_STATUS_DESC_RX_IS_QOS(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+8, 16, 1)
#define GET_RX_STATUS_DESC_WLANHD_IV_LEN(__pdesc)	\
	LE_BITS_TO_4BYTE(__pdesc+8, 18, 6)
#define GET_RX_STATUS_DESC_RPT_SEL(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+8, 28, 1)


#define GET_RX_DESC_RXMCS(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+12, 0, 7)
#define GET_RX_DESC_RXHT(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+12, 6, 1)
#define GET_RX_STATUS_DESC_RX_GF(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+12, 7, 1)
#define GET_RX_DESC_HTC(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+12, 10, 1)
#define GET_RX_STATUS_DESC_EOSP(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+12, 11, 1)
#define GET_RX_STATUS_DESC_BSSID_FIT(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+12, 12, 2)

#define GET_RX_STATUS_DESC_PATTERN_MATCH(__pdesc)	\
	LE_BITS_TO_4BYTE(__pdesc+12, 29, 1)
#define GET_RX_STATUS_DESC_UNICAST_MATCH(__pdesc)	\
	LE_BITS_TO_4BYTE(__pdesc+12, 30, 1)
#define GET_RX_STATUS_DESC_MAGIC_MATCH(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+12, 31, 1)

#define GET_RX_DESC_SPLCP(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+16, 0, 1)
#define GET_RX_STATUS_DESC_LDPC(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+16, 1, 1)
#define GET_RX_STATUS_DESC_STBC(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+16, 2, 1)
#define GET_RX_DESC_BW(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+16, 4, 2)

#define GET_RX_DESC_TSFL(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+20, 0, 32)

#define GET_RX_DESC_BUFF_ADDR(__pdesc)			\
	LE_BITS_TO_4BYTE(__pdesc+24, 0, 32)
#define GET_RX_DESC_BUFF_ADDR64(__pdesc)		\
	LE_BITS_TO_4BYTE(__pdesc+28, 0, 32)

#define SET_RX_DESC_BUFF_ADDR(__pdesc, __val)		\
	SET_BITS_TO_LE_4BYTE(__pdesc+24, 0, 32, __val)
#define SET_RX_DESC_BUFF_ADDR64(__pdesc, __val)	\
	SET_BITS_TO_LE_4BYTE(__pdesc+28, 0, 32, __val)


/* TX report 2 format in Rx desc*/

#define GET_RX_RPT2_DESC_PKT_LEN(__rxstatusdesc)	\
	LE_BITS_TO_4BYTE(__rxstatusdesc, 0, 9)
#define GET_RX_RPT2_DESC_MACID_VALID_1(__rxstatusdesc)	\
	LE_BITS_TO_4BYTE(__rxstatusdesc+16, 0, 32)
#define GET_RX_RPT2_DESC_MACID_VALID_2(__rxstatusdesc)	\
	LE_BITS_TO_4BYTE(__rxstatusdesc+20, 0, 32)

#define SET_EARLYMODE_PKTNUM(__paddr, __value)		\
	SET_BITS_TO_LE_4BYTE(__paddr, 0, 4, __value)
#define SET_EARLYMODE_LEN0(__paddr, __value)		\
	SET_BITS_TO_LE_4BYTE(__paddr, 4, 12, __value)
#define SET_EARLYMODE_LEN1(__paddr, __value)		\
	SET_BITS_TO_LE_4BYTE(__paddr, 16, 12, __value)
#define SET_EARLYMODE_LEN2_1(__paddr, __value)		\
	SET_BITS_TO_LE_4BYTE(__paddr, 28, 4, __value)
#define SET_EARLYMODE_LEN2_2(__paddr, __value)		\
	SET_BITS_TO_LE_4BYTE(__paddr+4, 0, 8, __value)
#define SET_EARLYMODE_LEN3(__paddr, __value)		\
	SET_BITS_TO_LE_4BYTE(__paddr+4, 8, 12, __value)
#define SET_EARLYMODE_LEN4(__paddr, __value)		\
	SET_BITS_TO_LE_4BYTE(__paddr+4, 20, 12, __value)

#define CLEAR_PCI_TX_DESC_CONTENT(__pdesc, _size)		\
do {								\
	if (_size > TX_DESC_NEXT_DESC_OFFSET)			\
		memset(__pdesc, 0, TX_DESC_NEXT_DESC_OFFSET);	\
	else							\
		memset(__pdesc, 0, _size);			\
} while (0)

struct phy_rx_agc_info_t {
	#ifdef __LITTLE_ENDIAN
		u8 gain:7, trsw:1;
	#else
		u8 trsw:1, gain:7;
	#endif
};
struct phy_status_rpt {
	struct phy_rx_agc_info_t path_agc[2];
	u8 ch_corr[2];
	u8 cck_sig_qual_ofdm_pwdb_all;
	u8 cck_agc_rpt_ofdm_cfosho_a;
	u8 cck_rpt_b_ofdm_cfosho_b;
	u8 rsvd_1;/* ch_corr_msb; */
	u8 noise_power_db_msb;
	char path_cfotail[2];
	u8 pcts_mask[2];
	char stream_rxevm[2];
	u8 path_rxsnr[2];
	u8 noise_power_db_lsb;
	u8 rsvd_2[3];
	u8 stream_csi[2];
	u8 stream_target_csi[2];
	u8 sig_evm;
	u8 rsvd_3;
#ifdef __LITTLE_ENDIAN
	u8 antsel_rx_keep_2:1;	/*ex_intf_flg:1;*/
	u8 sgi_en:1;
	u8 rxsc:2;
	u8 idle_long:1;
	u8 r_ant_train_en:1;
	u8 ant_sel_b:1;
	u8 ant_sel:1;
#else	/* _BIG_ENDIAN_	*/
	u8 ant_sel:1;
	u8 ant_sel_b:1;
	u8 r_ant_train_en:1;
	u8 idle_long:1;
	u8 rxsc:2;
	u8 sgi_en:1;
	u8 antsel_rx_keep_2:1;	/*ex_intf_flg:1;*/
#endif
} __packed;

struct rx_fwinfo_8723be {
	u8 gain_trsw[2];
	u16 chl_num:10;
	u16 sub_chnl:4;
	u16 r_rfmod:2;
	u8 pwdb_all;
	u8 cfosho[4];
	u8 cfotail[4];
	char rxevm[2];
	char rxsnr[2];
	u8 pcts_msk_rpt[2];
	u8 pdsnr[2];
	u8 csi_current[2];
	u8 rx_gain_c;
	u8 rx_gain_d;
	u8 sigevm;
	u8 resvd_0;
	u8 antidx_anta:3;
	u8 antidx_antb:3;
	u8 resvd_1:2;
} __packed;

struct tx_desc_8723be {
	u32 pktsize:16;
	u32 offset:8;
	u32 bmc:1;
	u32 htc:1;
	u32 lastseg:1;
	u32 firstseg:1;
	u32 linip:1;
	u32 noacm:1;
	u32 gf:1;
	u32 own:1;

	u32 macid:6;
	u32 rsvd0:2;
	u32 queuesel:5;
	u32 rd_nav_ext:1;
	u32 lsig_txop_en:1;
	u32 pifs:1;
	u32 rateid:4;
	u32 nav_usehdr:1;
	u32 en_descid:1;
	u32 sectype:2;
	u32 pktoffset:8;

	u32 rts_rc:6;
	u32 data_rc:6;
	u32 agg_en:1;
	u32 rdg_en:1;
	u32 bar_retryht:2;
	u32 agg_break:1;
	u32 morefrag:1;
	u32 raw:1;
	u32 ccx:1;
	u32 ampdudensity:3;
	u32 bt_int:1;
	u32 ant_sela:1;
	u32 ant_selb:1;
	u32 txant_cck:2;
	u32 txant_l:2;
	u32 txant_ht:2;

	u32 nextheadpage:8;
	u32 tailpage:8;
	u32 seq:12;
	u32 cpu_handle:1;
	u32 tag1:1;
	u32 trigger_int:1;
	u32 hwseq_en:1;

	u32 rtsrate:5;
	u32 apdcfe:1;
	u32 qos:1;
	u32 hwseq_ssn:1;
	u32 userrate:1;
	u32 dis_rtsfb:1;
	u32 dis_datafb:1;
	u32 cts2self:1;
	u32 rts_en:1;
	u32 hwrts_en:1;
	u32 portid:1;
	u32 pwr_status:3;
	u32 waitdcts:1;
	u32 cts2ap_en:1;
	u32 txsc:2;
	u32 stbc:2;
	u32 txshort:1;
	u32 txbw:1;
	u32 rtsshort:1;
	u32 rtsbw:1;
	u32 rtssc:2;
	u32 rtsstbc:2;

	u32 txrate:6;
	u32 shortgi:1;
	u32 ccxt:1;
	u32 txrate_fb_lmt:5;
	u32 rtsrate_fb_lmt:4;
	u32 retrylmt_en:1;
	u32 txretrylmt:6;
	u32 usb_txaggnum:8;

	u32 txagca:5;
	u32 txagcb:5;
	u32 usemaxlen:1;
	u32 maxaggnum:5;
	u32 mcsg1maxlen:4;
	u32 mcsg2maxlen:4;
	u32 mcsg3maxlen:4;
	u32 mcs7sgimaxlen:4;

	u32 txbuffersize:16;
	u32 sw_offset30:8;
	u32 sw_offset31:4;
	u32 rsvd1:1;
	u32 antsel_c:1;
	u32 null_0:1;
	u32 null_1:1;

	u32 txbuffaddr;
	u32 txbufferaddr64;
	u32 nextdescaddress;
	u32 nextdescaddress64;

	u32 reserve_pass_pcie_mm_limit[4];
} __packed;

struct rx_desc_8723be {
	u32 length:14;
	u32 crc32:1;
	u32 icverror:1;
	u32 drv_infosize:4;
	u32 security:3;
	u32 qos:1;
	u32 shift:2;
	u32 phystatus:1;
	u32 swdec:1;
	u32 lastseg:1;
	u32 firstseg:1;
	u32 eor:1;
	u32 own:1;

	u32 macid:6;
	u32 tid:4;
	u32 hwrsvd:5;
	u32 paggr:1;
	u32 faggr:1;
	u32 a1_fit:4;
	u32 a2_fit:4;
	u32 pam:1;
	u32 pwr:1;
	u32 moredata:1;
	u32 morefrag:1;
	u32 type:2;
	u32 mc:1;
	u32 bc:1;

	u32 seq:12;
	u32 frag:4;
	u32 nextpktlen:14;
	u32 nextind:1;
	u32 rsvd:1;

	u32 rxmcs:6;
	u32 rxht:1;
	u32 amsdu:1;
	u32 splcp:1;
	u32 bandwidth:1;
	u32 htc:1;
	u32 tcpchk_rpt:1;
	u32 ipcchk_rpt:1;
	u32 tcpchk_valid:1;
	u32 hwpcerr:1;
	u32 hwpcind:1;
	u32 iv0:16;

	u32 iv1;

	u32 tsfl;

	u32 bufferaddress;
	u32 bufferaddress64;

} __packed;

void rtl8723be_tx_fill_desc(struct ieee80211_hw *hw,
			    struct ieee80211_hdr *hdr,
			    u8 *pdesc_tx, u8 *txbd,
			    struct ieee80211_tx_info *info,
			    struct ieee80211_sta *sta, struct sk_buff *skb,
			    u8 hw_queue, struct rtl_tcb_desc *ptcb_desc);
bool rtl8723be_rx_query_desc(struct ieee80211_hw *hw,
			     struct rtl_stats *status,
			     struct ieee80211_rx_status *rx_status,
			     u8 *pdesc, struct sk_buff *skb);
void rtl8723be_set_desc(struct ieee80211_hw *hw, u8 *pdesc,
			bool istx, u8 desc_name, u8 *val);
u32 rtl8723be_get_desc(u8 *pdesc, bool istx, u8 desc_name);
bool rtl8723be_is_tx_desc_closed(struct ieee80211_hw *hw,
				 u8 hw_queue, u16 index);
void rtl8723be_tx_polling(struct ieee80211_hw *hw, u8 hw_queue);
void rtl8723be_tx_fill_cmddesc(struct ieee80211_hw *hw, u8 *pdesc,
			       bool firstseg, bool lastseg,
			       struct sk_buff *skb);
u32 rtl8723be_rx_command_packet(struct ieee80211_hw *hw,
				struct rtl_stats status,
				struct sk_buff *skb);
#endif
