/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2003-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <stdint.h>
#include <termios.h>

#define CSR_VARID_PS_CLR_ALL			0x000b	/* valueless */
#define CSR_VARID_PS_FACTORY_SET		0x000c	/* valueless */
#define CSR_VARID_PS_CLR_ALL_STORES		0x082d	/* uint16 */
#define CSR_VARID_BC01_STATUS			0x2801	/* uint16 */
#define CSR_VARID_BUILDID			0x2819	/* uint16 */
#define CSR_VARID_CHIPVER			0x281a	/* uint16 */
#define CSR_VARID_CHIPREV			0x281b	/* uint16 */
#define CSR_VARID_INTERFACE_VERSION		0x2825	/* uint16 */
#define CSR_VARID_RAND				0x282a	/* uint16 */
#define CSR_VARID_MAX_CRYPT_KEY_LENGTH		0x282c	/* uint16 */
#define CSR_VARID_CHIPANAREV			0x2836	/* uint16 */
#define CSR_VARID_BUILDID_LOADER		0x2838	/* uint16 */
#define CSR_VARID_BT_CLOCK			0x2c00	/* uint32 */
#define CSR_VARID_PS_NEXT			0x3005	/* complex */
#define CSR_VARID_PS_SIZE			0x3006	/* complex */
#define CSR_VARID_ADC_RES			0x3007	/* complex */
#define CSR_VARID_CRYPT_KEY_LENGTH		0x3008	/* complex */
#define CSR_VARID_PICONET_INSTANCE		0x3009	/* complex */
#define CSR_VARID_GET_CLR_EVT			0x300a	/* complex */
#define CSR_VARID_GET_NEXT_BUILDDEF		0x300b	/* complex */
#define CSR_VARID_PS_MEMORY_TYPE		0x3012	/* complex */
#define CSR_VARID_READ_BUILD_NAME		0x301c	/* complex */
#define CSR_VARID_COLD_RESET			0x4001	/* valueless */
#define CSR_VARID_WARM_RESET			0x4002	/* valueless */
#define CSR_VARID_COLD_HALT			0x4003	/* valueless */
#define CSR_VARID_WARM_HALT			0x4004	/* valueless */
#define CSR_VARID_INIT_BT_STACK			0x4005	/* valueless */
#define CSR_VARID_ACTIVATE_BT_STACK		0x4006	/* valueless */
#define CSR_VARID_ENABLE_TX			0x4007	/* valueless */
#define CSR_VARID_DISABLE_TX			0x4008	/* valueless */
#define CSR_VARID_RECAL				0x4009	/* valueless */
#define CSR_VARID_PS_FACTORY_RESTORE		0x400d	/* valueless */
#define CSR_VARID_PS_FACTORY_RESTORE_ALL	0x400e	/* valueless */
#define CSR_VARID_PS_DEFRAG_RESET		0x400f	/* valueless */
#define CSR_VARID_KILL_VM_APPLICATION		0x4010	/* valueless */
#define CSR_VARID_HOPPING_ON			0x4011	/* valueless */
#define CSR_VARID_CANCEL_PAGE			0x4012	/* valueless */
#define CSR_VARID_PS_CLR			0x4818	/* uint16 */
#define CSR_VARID_MAP_SCO_PCM			0x481c	/* uint16 */
#define CSR_VARID_ADC				0x4829	/* uint16 */
#define CSR_VARID_SINGLE_CHAN			0x482e	/* uint16 */
#define CSR_VARID_RADIOTEST			0x5004	/* complex */
#define CSR_VARID_PS_CLR_STORES			0x500c	/* complex */
#define CSR_VARID_NO_VARIABLE			0x6000	/* valueless */
#define CSR_VARID_CONFIG_UART			0x6802	/* uint16 */
#define CSR_VARID_PANIC_ARG			0x6805	/* uint16 */
#define CSR_VARID_FAULT_ARG			0x6806	/* uint16 */
#define CSR_VARID_MAX_TX_POWER			0x6827	/* int8 */
#define CSR_VARID_DEFAULT_TX_POWER		0x682b	/* int8 */
#define CSR_VARID_PS				0x7003	/* complex */

#define CSR_PSKEY_BDADDR					0x0001	/* bdaddr / uint16[] = { 0x00A5A5, 0x5b, 0x0002 } */
#define CSR_PSKEY_COUNTRYCODE					0x0002	/* uint16 */
#define CSR_PSKEY_CLASSOFDEVICE					0x0003	/* bdcod */
#define CSR_PSKEY_DEVICE_DRIFT					0x0004	/* uint16 */
#define CSR_PSKEY_DEVICE_JITTER					0x0005	/* uint16 */
#define CSR_PSKEY_MAX_ACLS					0x000d	/* uint16 */
#define CSR_PSKEY_MAX_SCOS					0x000e	/* uint16 */
#define CSR_PSKEY_MAX_REMOTE_MASTERS				0x000f	/* uint16 */
#define CSR_PSKEY_ENABLE_MASTERY_WITH_SLAVERY			0x0010	/* bool */
#define CSR_PSKEY_H_HC_FC_MAX_ACL_PKT_LEN			0x0011	/* uint16 */
#define CSR_PSKEY_H_HC_FC_MAX_SCO_PKT_LEN			0x0012	/* uint8 */
#define CSR_PSKEY_H_HC_FC_MAX_ACL_PKTS				0x0013	/* uint16 */
#define CSR_PSKEY_H_HC_FC_MAX_SCO_PKTS				0x0014	/* uint16 */
#define CSR_PSKEY_LC_FC_BUFFER_LOW_WATER_MARK			0x0015	/* lc_fc_lwm */
#define CSR_PSKEY_LC_MAX_TX_POWER				0x0017	/* int16 */
#define CSR_PSKEY_TX_GAIN_RAMP					0x001d	/* uint16 */
#define CSR_PSKEY_LC_POWER_TABLE				0x001e	/* power_setting[] */
#define CSR_PSKEY_LC_PEER_POWER_PERIOD				0x001f	/* TIME */
#define CSR_PSKEY_LC_FC_POOLS_LOW_WATER_MARK			0x0020	/* lc_fc_lwm */
#define CSR_PSKEY_LC_DEFAULT_TX_POWER				0x0021	/* int16 */
#define CSR_PSKEY_LC_RSSI_GOLDEN_RANGE				0x0022	/* uint8 */
#define CSR_PSKEY_LC_COMBO_DISABLE_PIO_MASK			0x0028	/* uint16[] */
#define CSR_PSKEY_LC_COMBO_PRIORITY_PIO_MASK			0x0029	/* uint16[] */
#define CSR_PSKEY_LC_COMBO_DOT11_CHANNEL_PIO_BASE		0x002a	/* uint16 */
#define CSR_PSKEY_LC_COMBO_DOT11_BLOCK_CHANNELS			0x002b	/* uint16 */
#define CSR_PSKEY_LC_MAX_TX_POWER_NO_RSSI			0x002d	/* int8 */
#define CSR_PSKEY_LC_CONNECTION_RX_WINDOW			0x002e	/* uint16 */
#define CSR_PSKEY_LC_COMBO_DOT11_TX_PROTECTION_MODE		0x0030	/* uint16 */
#define CSR_PSKEY_LC_ENHANCED_POWER_TABLE			0x0031	/* enhanced_power_setting[] */
#define CSR_PSKEY_LC_WIDEBAND_RSSI_CONFIG			0x0032	/* wideband_rssi_config */
#define CSR_PSKEY_LC_COMBO_DOT11_PRIORITY_LEAD			0x0033	/* uint16 */
#define CSR_PSKEY_BT_CLOCK_INIT					0x0034	/* uint32 */
#define CSR_PSKEY_TX_MR_MOD_DELAY				0x0038	/* uint8 */
#define CSR_PSKEY_RX_MR_SYNC_TIMING				0x0039	/* uint16 */
#define CSR_PSKEY_RX_MR_SYNC_CONFIG				0x003a	/* uint16 */
#define CSR_PSKEY_LC_LOST_SYNC_SLOTS				0x003b	/* uint16 */
#define CSR_PSKEY_RX_MR_SAMP_CONFIG				0x003c	/* uint16 */
#define CSR_PSKEY_AGC_HYST_LEVELS				0x003d	/* agc_hyst_config */
#define CSR_PSKEY_RX_LEVEL_LOW_SIGNAL				0x003e	/* uint16 */
#define CSR_PSKEY_AGC_IQ_LVL_VALUES				0x003f	/* IQ_LVL_VAL[] */
#define CSR_PSKEY_MR_FTRIM_OFFSET_12DB				0x0040	/* uint16 */
#define CSR_PSKEY_MR_FTRIM_OFFSET_6DB				0x0041	/* uint16 */
#define CSR_PSKEY_NO_CAL_ON_BOOT				0x0042	/* bool */
#define CSR_PSKEY_RSSI_HI_TARGET				0x0043	/* uint8 */
#define CSR_PSKEY_PREFERRED_MIN_ATTENUATION			0x0044	/* uint8 */
#define CSR_PSKEY_LC_COMBO_DOT11_PRIORITY_OVERRIDE		0x0045	/* bool */
#define CSR_PSKEY_LC_MULTISLOT_HOLDOFF				0x0047	/* TIME */
#define CSR_PSKEY_FREE_KEY_PIGEON_HOLE				0x00c9	/* uint16 */
#define CSR_PSKEY_LINK_KEY_BD_ADDR0				0x00ca	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR1				0x00cb	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR2				0x00cc	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR3				0x00cd	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR4				0x00ce	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR5				0x00cf	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR6				0x00d0	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR7				0x00d1	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR8				0x00d2	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR9				0x00d3	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR10				0x00d4	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR11				0x00d5	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR12				0x00d6	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR13				0x00d7	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR14				0x00d8	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LINK_KEY_BD_ADDR15				0x00d9	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_ENC_KEY_LMIN					0x00da	/* uint16 */
#define CSR_PSKEY_ENC_KEY_LMAX					0x00db	/* uint16 */
#define CSR_PSKEY_LOCAL_SUPPORTED_FEATURES			0x00ef	/* uint16[] = { 0xffff, 0xfe8f, 0xf99b, 0x8000 }*/
#define CSR_PSKEY_LM_USE_UNIT_KEY				0x00f0	/* bool */
#define CSR_PSKEY_HCI_NOP_DISABLE				0x00f2	/* bool */
#define CSR_PSKEY_LM_MAX_EVENT_FILTERS				0x00f4	/* uint8 */
#define CSR_PSKEY_LM_USE_ENC_MODE_BROADCAST			0x00f5	/* bool */
#define CSR_PSKEY_LM_TEST_SEND_ACCEPTED_TWICE			0x00f6	/* bool */
#define CSR_PSKEY_LM_MAX_PAGE_HOLD_TIME				0x00f7	/* uint16 */
#define CSR_PSKEY_AFH_ADAPTATION_RESPONSE_TIME			0x00f8	/* uint16 */
#define CSR_PSKEY_AFH_OPTIONS					0x00f9	/* uint16 */
#define CSR_PSKEY_AFH_RSSI_RUN_PERIOD				0x00fa	/* uint16 */
#define CSR_PSKEY_AFH_REENABLE_CHANNEL_TIME			0x00fb	/* uint16 */
#define CSR_PSKEY_NO_DROP_ON_ACR_MS_FAIL			0x00fc	/* bool */
#define CSR_PSKEY_MAX_PRIVATE_KEYS				0x00fd	/* uint8 */
#define CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR0			0x00fe	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR1			0x00ff	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR2			0x0100	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR3			0x0101	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR4			0x0102	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR5			0x0103	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR6			0x0104	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_PRIVATE_LINK_KEY_BD_ADDR7			0x0105	/* LM_LINK_KEY_BD_ADDR_T */
#define CSR_PSKEY_LOCAL_SUPPORTED_COMMANDS			0x0106	/* uint16[] = { 0xffff, 0x03ff, 0xfffe, 0xffff, 0xffff, 0xffff, 0x0ff3, 0xfff8, 0x003f } */
#define CSR_PSKEY_LM_MAX_ABSENCE_INDEX				0x0107	/* uint8 */
#define CSR_PSKEY_DEVICE_NAME					0x0108	/* uint16[] */
#define CSR_PSKEY_AFH_RSSI_THRESHOLD				0x0109	/* uint16 */
#define CSR_PSKEY_LM_CASUAL_SCAN_INTERVAL			0x010a	/* uint16 */
#define CSR_PSKEY_AFH_MIN_MAP_CHANGE				0x010b	/* uint16[] */
#define CSR_PSKEY_AFH_RSSI_LP_RUN_PERIOD			0x010c	/* uint16 */
#define CSR_PSKEY_HCI_LMP_LOCAL_VERSION				0x010d	/* uint16 */
#define CSR_PSKEY_LMP_REMOTE_VERSION				0x010e	/* uint8 */
#define CSR_PSKEY_HOLD_ERROR_MESSAGE_NUMBER			0x0113	/* uint16 */
#define CSR_PSKEY_DFU_ATTRIBUTES				0x0136	/* uint8 */
#define CSR_PSKEY_DFU_DETACH_TO					0x0137	/* uint16 */
#define CSR_PSKEY_DFU_TRANSFER_SIZE				0x0138	/* uint16 */
#define CSR_PSKEY_DFU_ENABLE					0x0139	/* bool */
#define CSR_PSKEY_DFU_LIN_REG_ENABLE				0x013a	/* bool */
#define CSR_PSKEY_DFUENC_VMAPP_PK_MODULUS_MSB			0x015e	/* uint16[] */
#define CSR_PSKEY_DFUENC_VMAPP_PK_MODULUS_LSB			0x015f	/* uint16[] */
#define CSR_PSKEY_DFUENC_VMAPP_PK_M_DASH			0x0160	/* uint16 */
#define CSR_PSKEY_DFUENC_VMAPP_PK_R2N_MSB			0x0161	/* uint16[] */
#define CSR_PSKEY_DFUENC_VMAPP_PK_R2N_LSB			0x0162	/* uint16[] */
#define CSR_PSKEY_BCSP_LM_PS_BLOCK				0x0192	/* BCSP_LM_PS_BLOCK */
#define CSR_PSKEY_HOSTIO_FC_PS_BLOCK				0x0193	/* HOSTIO_FC_PS_BLOCK */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO0				0x0194	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO1				0x0195	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO2				0x0196	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO3				0x0197	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO4				0x0198	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO5				0x0199	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO6				0x019a	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO7				0x019b	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO8				0x019c	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO9				0x019d	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO10			0x019e	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO11			0x019f	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO12			0x01a0	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO13			0x01a1	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO14			0x01a2	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_PROTOCOL_INFO15			0x01a3	/* PROTOCOL_INFO */
#define CSR_PSKEY_HOSTIO_UART_RESET_TIMEOUT			0x01a4	/* TIME */
#define CSR_PSKEY_HOSTIO_USE_HCI_EXTN				0x01a5	/* bool */
#define CSR_PSKEY_HOSTIO_USE_HCI_EXTN_CCFC			0x01a6	/* bool */
#define CSR_PSKEY_HOSTIO_HCI_EXTN_PAYLOAD_SIZE			0x01a7	/* uint16 */
#define CSR_PSKEY_BCSP_LM_CNF_CNT_LIMIT				0x01aa	/* uint16 */
#define CSR_PSKEY_HOSTIO_MAP_SCO_PCM				0x01ab	/* bool */
#define CSR_PSKEY_HOSTIO_AWKWARD_PCM_SYNC			0x01ac	/* bool */
#define CSR_PSKEY_HOSTIO_BREAK_POLL_PERIOD			0x01ad	/* TIME */
#define CSR_PSKEY_HOSTIO_MIN_UART_HCI_SCO_SIZE			0x01ae	/* uint16 */
#define CSR_PSKEY_HOSTIO_MAP_SCO_CODEC				0x01b0	/* bool */
#define CSR_PSKEY_PCM_CVSD_TX_HI_FREQ_BOOST			0x01b1	/* uint16 */
#define CSR_PSKEY_PCM_CVSD_RX_HI_FREQ_BOOST			0x01b2	/* uint16 */
#define CSR_PSKEY_PCM_CONFIG32					0x01b3	/* uint32 */
#define CSR_PSKEY_USE_OLD_BCSP_LE				0x01b4	/* uint16 */
#define CSR_PSKEY_PCM_CVSD_USE_NEW_FILTER			0x01b5	/* bool */
#define CSR_PSKEY_PCM_FORMAT					0x01b6	/* uint16 */
#define CSR_PSKEY_CODEC_OUT_GAIN				0x01b7	/* uint16 */
#define CSR_PSKEY_CODEC_IN_GAIN					0x01b8	/* uint16 */
#define CSR_PSKEY_CODEC_PIO					0x01b9	/* uint16 */
#define CSR_PSKEY_PCM_LOW_JITTER_CONFIG				0x01ba	/* uint32 */
#define CSR_PSKEY_HOSTIO_SCO_PCM_THRESHOLDS			0x01bb	/* uint16[] */
#define CSR_PSKEY_HOSTIO_SCO_HCI_THRESHOLDS			0x01bc	/* uint16[] */
#define CSR_PSKEY_HOSTIO_MAP_SCO_PCM_SLOT			0x01bd	/* uint16 */
#define CSR_PSKEY_UART_BAUDRATE					0x01be	/* uint16 */
#define CSR_PSKEY_UART_CONFIG_BCSP				0x01bf	/* uint16 */
#define CSR_PSKEY_UART_CONFIG_H4				0x01c0	/* uint16 */
#define CSR_PSKEY_UART_CONFIG_H5				0x01c1	/* uint16 */
#define CSR_PSKEY_UART_CONFIG_USR				0x01c2	/* uint16 */
#define CSR_PSKEY_UART_TX_CRCS					0x01c3	/* bool */
#define CSR_PSKEY_UART_ACK_TIMEOUT				0x01c4	/* uint16 */
#define CSR_PSKEY_UART_TX_MAX_ATTEMPTS				0x01c5	/* uint16 */
#define CSR_PSKEY_UART_TX_WINDOW_SIZE				0x01c6	/* uint16 */
#define CSR_PSKEY_UART_HOST_WAKE				0x01c7	/* uint16[] */
#define CSR_PSKEY_HOSTIO_THROTTLE_TIMEOUT			0x01c8	/* TIME */
#define CSR_PSKEY_PCM_ALWAYS_ENABLE				0x01c9	/* bool */
#define CSR_PSKEY_UART_HOST_WAKE_SIGNAL				0x01ca	/* uint16 */
#define CSR_PSKEY_UART_CONFIG_H4DS				0x01cb	/* uint16 */
#define CSR_PSKEY_H4DS_WAKE_DURATION				0x01cc	/* uint16 */
#define CSR_PSKEY_H4DS_MAXWU					0x01cd	/* uint16 */
#define CSR_PSKEY_H4DS_LE_TIMER_PERIOD				0x01cf	/* uint16 */
#define CSR_PSKEY_H4DS_TWU_TIMER_PERIOD				0x01d0	/* uint16 */
#define CSR_PSKEY_H4DS_UART_IDLE_TIMER_PERIOD			0x01d1	/* uint16 */
#define CSR_PSKEY_ANA_FTRIM					0x01f6	/* uint16 */
#define CSR_PSKEY_WD_TIMEOUT					0x01f7	/* TIME */
#define CSR_PSKEY_WD_PERIOD					0x01f8	/* TIME */
#define CSR_PSKEY_HOST_INTERFACE				0x01f9	/* phys_bus */
#define CSR_PSKEY_HQ_HOST_TIMEOUT				0x01fb	/* TIME */
#define CSR_PSKEY_HQ_ACTIVE					0x01fc	/* bool */
#define CSR_PSKEY_BCCMD_SECURITY_ACTIVE				0x01fd	/* bool */
#define CSR_PSKEY_ANA_FREQ					0x01fe	/* uint16 */
#define CSR_PSKEY_PIO_PROTECT_MASK				0x0202	/* uint16 */
#define CSR_PSKEY_PMALLOC_SIZES					0x0203	/* uint16[] */
#define CSR_PSKEY_UART_BAUD_RATE				0x0204	/* uint16 */
#define CSR_PSKEY_UART_CONFIG					0x0205	/* uint16 */
#define CSR_PSKEY_STUB						0x0207	/* uint16 */
#define CSR_PSKEY_TXRX_PIO_CONTROL				0x0209	/* uint16 */
#define CSR_PSKEY_ANA_RX_LEVEL					0x020b	/* uint16 */
#define CSR_PSKEY_ANA_RX_FTRIM					0x020c	/* uint16 */
#define CSR_PSKEY_PSBC_DATA_VERSION				0x020d	/* uint16 */
#define CSR_PSKEY_PCM0_ATTENUATION				0x020f	/* uint16 */
#define CSR_PSKEY_LO_LVL_MAX					0x0211	/* uint16 */
#define CSR_PSKEY_LO_ADC_AMPL_MIN				0x0212	/* uint16 */
#define CSR_PSKEY_LO_ADC_AMPL_MAX				0x0213	/* uint16 */
#define CSR_PSKEY_IQ_TRIM_CHANNEL				0x0214	/* uint16 */
#define CSR_PSKEY_IQ_TRIM_GAIN					0x0215	/* uint16 */
#define CSR_PSKEY_IQ_TRIM_ENABLE				0x0216	/* iq_trim_enable_flag */
#define CSR_PSKEY_TX_OFFSET_HALF_MHZ				0x0217	/* int16 */
#define CSR_PSKEY_GBL_MISC_ENABLES				0x0221	/* uint16 */
#define CSR_PSKEY_UART_SLEEP_TIMEOUT				0x0222	/* uint16 */
#define CSR_PSKEY_DEEP_SLEEP_STATE				0x0229	/* deep_sleep_state */
#define CSR_PSKEY_IQ_ENABLE_PHASE_TRIM				0x022d	/* bool */
#define CSR_PSKEY_HCI_HANDLE_FREEZE_PERIOD			0x0237	/* TIME */
#define CSR_PSKEY_MAX_FROZEN_HCI_HANDLES			0x0238	/* uint16 */
#define CSR_PSKEY_PAGETABLE_DESTRUCTION_DELAY			0x0239	/* TIME */
#define CSR_PSKEY_IQ_TRIM_PIO_SETTINGS				0x023a	/* uint8 */
#define CSR_PSKEY_USE_EXTERNAL_CLOCK				0x023b	/* bool */
#define CSR_PSKEY_DEEP_SLEEP_WAKE_CTS				0x023c	/* uint16 */
#define CSR_PSKEY_FC_HC2H_FLUSH_DELAY				0x023d	/* TIME */
#define CSR_PSKEY_RX_HIGHSIDE					0x023e	/* bool */
#define CSR_PSKEY_TX_PRE_LVL					0x0240	/* uint8 */
#define CSR_PSKEY_RX_SINGLE_ENDED				0x0242	/* bool */
#define CSR_PSKEY_TX_FILTER_CONFIG				0x0243	/* uint32 */
#define CSR_PSKEY_CLOCK_REQUEST_ENABLE				0x0246	/* uint16 */
#define CSR_PSKEY_RX_MIN_ATTEN					0x0249	/* uint16 */
#define CSR_PSKEY_XTAL_TARGET_AMPLITUDE				0x024b	/* uint8 */
#define CSR_PSKEY_PCM_MIN_CPU_CLOCK				0x024d	/* uint16 */
#define CSR_PSKEY_HOST_INTERFACE_PIO_USB			0x0250	/* uint16 */
#define CSR_PSKEY_CPU_IDLE_MODE					0x0251	/* cpu_idle_mode */
#define CSR_PSKEY_DEEP_SLEEP_CLEAR_RTS				0x0252	/* bool */
#define CSR_PSKEY_RF_RESONANCE_TRIM				0x0254	/* uint16 */
#define CSR_PSKEY_DEEP_SLEEP_PIO_WAKE				0x0255	/* uint16 */
#define CSR_PSKEY_DRAIN_BORE_TIMERS				0x0256	/* uint32[] */
#define CSR_PSKEY_DRAIN_TX_POWER_BASE				0x0257	/* uint16 */
#define CSR_PSKEY_MODULE_ID					0x0259	/* uint32 */
#define CSR_PSKEY_MODULE_DESIGN					0x025a	/* uint16 */
#define CSR_PSKEY_MODULE_SECURITY_CODE				0x025c	/* uint16[] */
#define CSR_PSKEY_VM_DISABLE					0x025d	/* bool */
#define CSR_PSKEY_MOD_MANUF0					0x025e	/* uint16[] */
#define CSR_PSKEY_MOD_MANUF1					0x025f	/* uint16[] */
#define CSR_PSKEY_MOD_MANUF2					0x0260	/* uint16[] */
#define CSR_PSKEY_MOD_MANUF3					0x0261	/* uint16[] */
#define CSR_PSKEY_MOD_MANUF4					0x0262	/* uint16[] */
#define CSR_PSKEY_MOD_MANUF5					0x0263	/* uint16[] */
#define CSR_PSKEY_MOD_MANUF6					0x0264	/* uint16[] */
#define CSR_PSKEY_MOD_MANUF7					0x0265	/* uint16[] */
#define CSR_PSKEY_MOD_MANUF8					0x0266	/* uint16[] */
#define CSR_PSKEY_MOD_MANUF9					0x0267	/* uint16[] */
#define CSR_PSKEY_DUT_VM_DISABLE				0x0268	/* bool */
#define CSR_PSKEY_USR0						0x028a	/* uint16[] */
#define CSR_PSKEY_USR1						0x028b	/* uint16[] */
#define CSR_PSKEY_USR2						0x028c	/* uint16[] */
#define CSR_PSKEY_USR3						0x028d	/* uint16[] */
#define CSR_PSKEY_USR4						0x028e	/* uint16[] */
#define CSR_PSKEY_USR5						0x028f	/* uint16[] */
#define CSR_PSKEY_USR6						0x0290	/* uint16[] */
#define CSR_PSKEY_USR7						0x0291	/* uint16[] */
#define CSR_PSKEY_USR8						0x0292	/* uint16[] */
#define CSR_PSKEY_USR9						0x0293	/* uint16[] */
#define CSR_PSKEY_USR10						0x0294	/* uint16[] */
#define CSR_PSKEY_USR11						0x0295	/* uint16[] */
#define CSR_PSKEY_USR12						0x0296	/* uint16[] */
#define CSR_PSKEY_USR13						0x0297	/* uint16[] */
#define CSR_PSKEY_USR14						0x0298	/* uint16[] */
#define CSR_PSKEY_USR15						0x0299	/* uint16[] */
#define CSR_PSKEY_USR16						0x029a	/* uint16[] */
#define CSR_PSKEY_USR17						0x029b	/* uint16[] */
#define CSR_PSKEY_USR18						0x029c	/* uint16[] */
#define CSR_PSKEY_USR19						0x029d	/* uint16[] */
#define CSR_PSKEY_USR20						0x029e	/* uint16[] */
#define CSR_PSKEY_USR21						0x029f	/* uint16[] */
#define CSR_PSKEY_USR22						0x02a0	/* uint16[] */
#define CSR_PSKEY_USR23						0x02a1	/* uint16[] */
#define CSR_PSKEY_USR24						0x02a2	/* uint16[] */
#define CSR_PSKEY_USR25						0x02a3	/* uint16[] */
#define CSR_PSKEY_USR26						0x02a4	/* uint16[] */
#define CSR_PSKEY_USR27						0x02a5	/* uint16[] */
#define CSR_PSKEY_USR28						0x02a6	/* uint16[] */
#define CSR_PSKEY_USR29						0x02a7	/* uint16[] */
#define CSR_PSKEY_USR30						0x02a8	/* uint16[] */
#define CSR_PSKEY_USR31						0x02a9	/* uint16[] */
#define CSR_PSKEY_USR32						0x02aa	/* uint16[] */
#define CSR_PSKEY_USR33						0x02ab	/* uint16[] */
#define CSR_PSKEY_USR34						0x02ac	/* uint16[] */
#define CSR_PSKEY_USR35						0x02ad	/* uint16[] */
#define CSR_PSKEY_USR36						0x02ae	/* uint16[] */
#define CSR_PSKEY_USR37						0x02af	/* uint16[] */
#define CSR_PSKEY_USR38						0x02b0	/* uint16[] */
#define CSR_PSKEY_USR39						0x02b1	/* uint16[] */
#define CSR_PSKEY_USR40						0x02b2	/* uint16[] */
#define CSR_PSKEY_USR41						0x02b3	/* uint16[] */
#define CSR_PSKEY_USR42						0x02b4	/* uint16[] */
#define CSR_PSKEY_USR43						0x02b5	/* uint16[] */
#define CSR_PSKEY_USR44						0x02b6	/* uint16[] */
#define CSR_PSKEY_USR45						0x02b7	/* uint16[] */
#define CSR_PSKEY_USR46						0x02b8	/* uint16[] */
#define CSR_PSKEY_USR47						0x02b9	/* uint16[] */
#define CSR_PSKEY_USR48						0x02ba	/* uint16[] */
#define CSR_PSKEY_USR49						0x02bb	/* uint16[] */
#define CSR_PSKEY_USB_VERSION					0x02bc	/* uint16 */
#define CSR_PSKEY_USB_DEVICE_CLASS_CODES			0x02bd	/* usbclass */
#define CSR_PSKEY_USB_VENDOR_ID					0x02be	/* uint16 */
#define CSR_PSKEY_USB_PRODUCT_ID				0x02bf	/* uint16 */
#define CSR_PSKEY_USB_MANUF_STRING				0x02c1	/* unicodestring */
#define CSR_PSKEY_USB_PRODUCT_STRING				0x02c2	/* unicodestring */
#define CSR_PSKEY_USB_SERIAL_NUMBER_STRING			0x02c3	/* unicodestring */
#define CSR_PSKEY_USB_CONFIG_STRING				0x02c4	/* unicodestring */
#define CSR_PSKEY_USB_ATTRIBUTES				0x02c5	/* uint8 */
#define CSR_PSKEY_USB_MAX_POWER					0x02c6	/* uint16 */
#define CSR_PSKEY_USB_BT_IF_CLASS_CODES				0x02c7	/* usbclass */
#define CSR_PSKEY_USB_LANGID					0x02c9	/* uint16 */
#define CSR_PSKEY_USB_DFU_CLASS_CODES				0x02ca	/* usbclass */
#define CSR_PSKEY_USB_DFU_PRODUCT_ID				0x02cb	/* uint16 */
#define CSR_PSKEY_USB_PIO_DETACH				0x02ce	/* uint16 */
#define CSR_PSKEY_USB_PIO_WAKEUP				0x02cf	/* uint16 */
#define CSR_PSKEY_USB_PIO_PULLUP				0x02d0	/* uint16 */
#define CSR_PSKEY_USB_PIO_VBUS					0x02d1	/* uint16 */
#define CSR_PSKEY_USB_PIO_WAKE_TIMEOUT				0x02d2	/* uint16 */
#define CSR_PSKEY_USB_PIO_RESUME				0x02d3	/* uint16 */
#define CSR_PSKEY_USB_BT_SCO_IF_CLASS_CODES			0x02d4	/* usbclass */
#define CSR_PSKEY_USB_SUSPEND_PIO_LEVEL				0x02d5	/* uint16 */
#define CSR_PSKEY_USB_SUSPEND_PIO_DIR				0x02d6	/* uint16 */
#define CSR_PSKEY_USB_SUSPEND_PIO_MASK				0x02d7	/* uint16 */
#define CSR_PSKEY_USB_ENDPOINT_0_MAX_PACKET_SIZE		0x02d8	/* uint8 */
#define CSR_PSKEY_USB_CONFIG					0x02d9	/* uint16 */
#define CSR_PSKEY_RADIOTEST_ATTEN_INIT				0x0320	/* uint16 */
#define CSR_PSKEY_RADIOTEST_FIRST_TRIM_TIME			0x0326	/* TIME */
#define CSR_PSKEY_RADIOTEST_SUBSEQUENT_TRIM_TIME		0x0327	/* TIME */
#define CSR_PSKEY_RADIOTEST_LO_LVL_TRIM_ENABLE			0x0328	/* bool */
#define CSR_PSKEY_RADIOTEST_DISABLE_MODULATION			0x032c	/* bool */
#define CSR_PSKEY_RFCOMM_FCON_THRESHOLD				0x0352	/* uint16 */
#define CSR_PSKEY_RFCOMM_FCOFF_THRESHOLD			0x0353	/* uint16 */
#define CSR_PSKEY_IPV6_STATIC_ADDR				0x0354	/* uint16[] */
#define CSR_PSKEY_IPV4_STATIC_ADDR				0x0355	/* uint32 */
#define CSR_PSKEY_IPV6_STATIC_PREFIX_LEN			0x0356	/* uint8 */
#define CSR_PSKEY_IPV6_STATIC_ROUTER_ADDR			0x0357	/* uint16[] */
#define CSR_PSKEY_IPV4_STATIC_SUBNET_MASK			0x0358	/* uint32 */
#define CSR_PSKEY_IPV4_STATIC_ROUTER_ADDR			0x0359	/* uint32 */
#define CSR_PSKEY_MDNS_NAME					0x035a	/* char[] */
#define CSR_PSKEY_FIXED_PIN					0x035b	/* uint8[] */
#define CSR_PSKEY_MDNS_PORT					0x035c	/* uint16 */
#define CSR_PSKEY_MDNS_TTL					0x035d	/* uint8 */
#define CSR_PSKEY_MDNS_IPV4_ADDR				0x035e	/* uint32 */
#define CSR_PSKEY_ARP_CACHE_TIMEOUT				0x035f	/* uint16 */
#define CSR_PSKEY_HFP_POWER_TABLE				0x0360	/* uint16[] */
#define CSR_PSKEY_DRAIN_BORE_TIMER_COUNTERS			0x03e7	/* uint32[] */
#define CSR_PSKEY_DRAIN_BORE_COUNTERS				0x03e6	/* uint32[] */
#define CSR_PSKEY_LOOP_FILTER_TRIM				0x03e4	/* uint16 */
#define CSR_PSKEY_DRAIN_BORE_CURRENT_PEAK			0x03e3	/* uint32[] */
#define CSR_PSKEY_VM_E2_CACHE_LIMIT				0x03e2	/* uint16 */
#define CSR_PSKEY_FORCE_16MHZ_REF_PIO				0x03e1	/* uint16 */
#define CSR_PSKEY_CDMA_LO_REF_LIMITS				0x03df	/* uint16 */
#define CSR_PSKEY_CDMA_LO_ERROR_LIMITS				0x03de	/* uint16 */
#define CSR_PSKEY_CLOCK_STARTUP_DELAY				0x03dd	/* uint16 */
#define CSR_PSKEY_DEEP_SLEEP_CORRECTION_FACTOR			0x03dc	/* int16 */
#define CSR_PSKEY_TEMPERATURE_CALIBRATION			0x03db	/* temperature_calibration */
#define CSR_PSKEY_TEMPERATURE_VS_DELTA_INTERNAL_PA		0x03da	/* temperature_calibration[] */
#define CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_PRE_LVL		0x03d9	/* temperature_calibration[] */
#define CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_BB			0x03d8	/* temperature_calibration[] */
#define CSR_PSKEY_TEMPERATURE_VS_DELTA_ANA_FTRIM		0x03d7	/* temperature_calibration[] */
#define CSR_PSKEY_TEST_DELTA_OFFSET				0x03d6	/* uint16 */
#define CSR_PSKEY_RX_DYNAMIC_LVL_OFFSET				0x03d4	/* uint16 */
#define CSR_PSKEY_TEST_FORCE_OFFSET				0x03d3	/* bool */
#define CSR_PSKEY_RF_TRAP_BAD_DIVISION_RATIOS			0x03cf	/* uint16 */
#define CSR_PSKEY_RADIOTEST_CDMA_LO_REF_LIMITS			0x03ce	/* uint16 */
#define CSR_PSKEY_INITIAL_BOOTMODE				0x03cd	/* int16 */
#define CSR_PSKEY_ONCHIP_HCI_CLIENT				0x03cc	/* bool */
#define CSR_PSKEY_RX_ATTEN_BACKOFF				0x03ca	/* uint16 */
#define CSR_PSKEY_RX_ATTEN_UPDATE_RATE				0x03c9	/* uint16 */
#define CSR_PSKEY_SYNTH_TXRX_THRESHOLDS				0x03c7	/* uint16 */
#define CSR_PSKEY_MIN_WAIT_STATES				0x03c6	/* uint16 */
#define CSR_PSKEY_RSSI_CORRECTION				0x03c5	/* int8 */
#define CSR_PSKEY_SCHED_THROTTLE_TIMEOUT			0x03c4	/* TIME */
#define CSR_PSKEY_DEEP_SLEEP_USE_EXTERNAL_CLOCK			0x03c3	/* bool */
#define CSR_PSKEY_TRIM_RADIO_FILTERS				0x03c2	/* uint16 */
#define CSR_PSKEY_TRANSMIT_OFFSET				0x03c1	/* int16 */
#define CSR_PSKEY_USB_VM_CONTROL				0x03c0	/* bool */
#define CSR_PSKEY_MR_ANA_RX_FTRIM				0x03bf	/* uint16 */
#define CSR_PSKEY_I2C_CONFIG					0x03be	/* uint16 */
#define CSR_PSKEY_IQ_LVL_RX					0x03bd	/* uint16 */
#define CSR_PSKEY_MR_TX_FILTER_CONFIG				0x03bb	/* uint32 */
#define CSR_PSKEY_MR_TX_CONFIG2					0x03ba	/* uint16 */
#define CSR_PSKEY_USB_DONT_RESET_BOOTMODE_ON_HOST_RESET		0x03b9	/* bool */
#define CSR_PSKEY_LC_USE_THROTTLING				0x03b8	/* bool */
#define CSR_PSKEY_CHARGER_TRIM					0x03b7	/* uint16 */
#define CSR_PSKEY_CLOCK_REQUEST_FEATURES			0x03b6	/* uint16 */
#define CSR_PSKEY_TRANSMIT_OFFSET_CLASS1			0x03b4	/* int16 */
#define CSR_PSKEY_TX_AVOID_PA_CLASS1_PIO			0x03b3	/* uint16 */
#define CSR_PSKEY_MR_PIO_CONFIG					0x03b2	/* uint16 */
#define CSR_PSKEY_UART_CONFIG2					0x03b1	/* uint8 */
#define CSR_PSKEY_CLASS1_IQ_LVL					0x03b0	/* uint16 */
#define CSR_PSKEY_CLASS1_TX_CONFIG2				0x03af	/* uint16 */
#define CSR_PSKEY_TEMPERATURE_VS_DELTA_INTERNAL_PA_CLASS1	0x03ae	/* temperature_calibration[] */
#define CSR_PSKEY_TEMPERATURE_VS_DELTA_EXTERNAL_PA_CLASS1	0x03ad	/* temperature_calibration[] */
#define CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_PRE_LVL_MR		0x03ac	/* temperature_calibration[] */
#define CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_BB_MR_HEADER		0x03ab	/* temperature_calibration[] */
#define CSR_PSKEY_TEMPERATURE_VS_DELTA_TX_BB_MR_PAYLOAD		0x03aa	/* temperature_calibration[] */
#define CSR_PSKEY_RX_MR_EQ_TAPS					0x03a9	/* uint16[] */
#define CSR_PSKEY_TX_PRE_LVL_CLASS1				0x03a8	/* uint8 */
#define CSR_PSKEY_ANALOGUE_ATTENUATOR				0x03a7	/* bool */
#define CSR_PSKEY_MR_RX_FILTER_TRIM				0x03a6	/* uint16 */
#define CSR_PSKEY_MR_RX_FILTER_RESPONSE				0x03a5	/* int16[] */
#define CSR_PSKEY_PIO_WAKEUP_STATE				0x039f	/* uint16 */
#define CSR_PSKEY_MR_TX_IF_ATTEN_OFF_TEMP			0x0394	/* int16 */
#define CSR_PSKEY_LO_DIV_LATCH_BYPASS				0x0393	/* bool */
#define CSR_PSKEY_LO_VCO_STANDBY				0x0392	/* bool */
#define CSR_PSKEY_SLOW_CLOCK_FILTER_SHIFT			0x0391	/* uint16 */
#define CSR_PSKEY_SLOW_CLOCK_FILTER_DIVIDER			0x0390	/* uint16 */
#define CSR_PSKEY_USB_ATTRIBUTES_POWER				0x03f2	/* bool */
#define CSR_PSKEY_USB_ATTRIBUTES_WAKEUP				0x03f3	/* bool */
#define CSR_PSKEY_DFU_ATTRIBUTES_MANIFESTATION_TOLERANT		0x03f4	/* bool */
#define CSR_PSKEY_DFU_ATTRIBUTES_CAN_UPLOAD			0x03f5	/* bool */
#define CSR_PSKEY_DFU_ATTRIBUTES_CAN_DOWNLOAD			0x03f6	/* bool */
#define CSR_PSKEY_UART_CONFIG_STOP_BITS				0x03fc	/* bool */
#define CSR_PSKEY_UART_CONFIG_PARITY_BIT			0x03fd	/* uint16 */
#define CSR_PSKEY_UART_CONFIG_FLOW_CTRL_EN			0x03fe	/* bool */
#define CSR_PSKEY_UART_CONFIG_RTS_AUTO_EN			0x03ff	/* bool */
#define CSR_PSKEY_UART_CONFIG_RTS				0x0400	/* bool */
#define CSR_PSKEY_UART_CONFIG_TX_ZERO_EN			0x0401	/* bool */
#define CSR_PSKEY_UART_CONFIG_NON_BCSP_EN			0x0402	/* bool */
#define CSR_PSKEY_UART_CONFIG_RX_RATE_DELAY			0x0403	/* uint16 */
#define CSR_PSKEY_UART_SEQ_TIMEOUT				0x0405	/* uint16 */
#define CSR_PSKEY_UART_SEQ_RETRIES				0x0406	/* uint16 */
#define CSR_PSKEY_UART_SEQ_WINSIZE				0x0407	/* uint16 */
#define CSR_PSKEY_UART_USE_CRC_ON_TX				0x0408	/* bool */
#define CSR_PSKEY_UART_HOST_INITIAL_STATE			0x0409	/* hwakeup_state */
#define CSR_PSKEY_UART_HOST_ATTENTION_SPAN			0x040a	/* uint16 */
#define CSR_PSKEY_UART_HOST_WAKEUP_TIME				0x040b	/* uint16 */
#define CSR_PSKEY_UART_HOST_WAKEUP_WAIT				0x040c	/* uint16 */
#define CSR_PSKEY_BCSP_LM_MODE					0x0410	/* uint16 */
#define CSR_PSKEY_BCSP_LM_SYNC_RETRIES				0x0411	/* uint16 */
#define CSR_PSKEY_BCSP_LM_TSHY					0x0412	/* uint16 */
#define CSR_PSKEY_UART_DFU_CONFIG_STOP_BITS			0x0417	/* bool */
#define CSR_PSKEY_UART_DFU_CONFIG_PARITY_BIT			0x0418	/* uint16 */
#define CSR_PSKEY_UART_DFU_CONFIG_FLOW_CTRL_EN			0x0419	/* bool */
#define CSR_PSKEY_UART_DFU_CONFIG_RTS_AUTO_EN			0x041a	/* bool */
#define CSR_PSKEY_UART_DFU_CONFIG_RTS				0x041b	/* bool */
#define CSR_PSKEY_UART_DFU_CONFIG_TX_ZERO_EN			0x041c	/* bool */
#define CSR_PSKEY_UART_DFU_CONFIG_NON_BCSP_EN			0x041d	/* bool */
#define CSR_PSKEY_UART_DFU_CONFIG_RX_RATE_DELAY			0x041e	/* uint16 */
#define CSR_PSKEY_AMUX_AIO0					0x041f	/* ana_amux_sel */
#define CSR_PSKEY_AMUX_AIO1					0x0420	/* ana_amux_sel */
#define CSR_PSKEY_AMUX_AIO2					0x0421	/* ana_amux_sel */
#define CSR_PSKEY_AMUX_AIO3					0x0422	/* ana_amux_sel */
#define CSR_PSKEY_LOCAL_NAME_SIMPLIFIED				0x0423	/* local_name_complete */
#define CSR_PSKEY_EXTENDED_STUB					0x2001	/* uint16 */

char *csr_builddeftostr(uint16_t def);
char *csr_buildidtostr(uint16_t id);
char *csr_chipvertostr(uint16_t ver, uint16_t rev);
char *csr_pskeytostr(uint16_t pskey);
char *csr_pskeytoval(uint16_t pskey);

int csr_open_hci(char *device);
int csr_read_hci(uint16_t varid, uint8_t *value, uint16_t length);
int csr_write_hci(uint16_t varid, uint8_t *value, uint16_t length);
void csr_close_hci(void);

int csr_open_usb(char *device);
int csr_read_usb(uint16_t varid, uint8_t *value, uint16_t length);
int csr_write_usb(uint16_t varid, uint8_t *value, uint16_t length);
void csr_close_usb(void);

int csr_open_bcsp(char *device, speed_t bcsp_rate);
int csr_read_bcsp(uint16_t varid, uint8_t *value, uint16_t length);
int csr_write_bcsp(uint16_t varid, uint8_t *value, uint16_t length);
void csr_close_bcsp(void);

int csr_open_h4(char *device);
int csr_read_h4(uint16_t varid, uint8_t *value, uint16_t length);
int csr_write_h4(uint16_t varid, uint8_t *value, uint16_t length);
void csr_close_h4(void);

int csr_open_3wire(char *device);
int csr_read_3wire(uint16_t varid, uint8_t *value, uint16_t length);
int csr_write_3wire(uint16_t varid, uint8_t *value, uint16_t length);
void csr_close_3wire(void);

int csr_write_varid_valueless(int dd, uint16_t seqnum, uint16_t varid);
int csr_write_varid_complex(int dd, uint16_t seqnum, uint16_t varid, uint8_t *value, uint16_t length);
int csr_read_varid_complex(int dd, uint16_t seqnum, uint16_t varid, uint8_t *value, uint16_t length);
int csr_read_varid_uint16(int dd, uint16_t seqnum, uint16_t varid, uint16_t *value);
int csr_read_varid_uint32(int dd, uint16_t seqnum, uint16_t varid, uint32_t *value);
int csr_read_pskey_complex(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint8_t *value, uint16_t length);
int csr_write_pskey_complex(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint8_t *value, uint16_t length);
int csr_read_pskey_uint16(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint16_t *value);
int csr_write_pskey_uint16(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint16_t value);
int csr_read_pskey_uint32(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint32_t *value);
int csr_write_pskey_uint32(int dd, uint16_t seqnum, uint16_t pskey, uint16_t stores, uint32_t value);

int psr_put(uint16_t pskey, uint8_t *value, uint16_t size);
int psr_get(uint16_t *pskey, uint8_t *value, uint16_t *size);
int psr_read(const char *filename);
int psr_print(void);
