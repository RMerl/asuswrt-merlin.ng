/*
* <:copyright-BRCM:2013:GPL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


#ifndef RDPA_CONFIG_H_
#define RDPA_CONFIG_H_

/*****************************************************************************/
/** MTU                                                                     **/
/*****************************************************************************/
#ifndef RDPA_MTU
#define RDPA_MTU 1536
#endif

#ifndef RDPA_MIN_MTU
#define RDPA_MIN_MTU 64
#endif

/*****************************************************************************/
/* The minimal headroom size within an SKB buffer                            */
/*****************************************************************************/
#ifndef RDPA_MIN_SKB_HEADROOM_SIZE
#define RDPA_MIN_SKB_HEADROOM_SIZE 0
#endif

#ifndef RDPA_DS_LITE_HEADROOM_SIZE
#define RDPA_DS_LITE_HEADROOM_SIZE 40
#endif
/*---------------------------------------------------------------------------*/
/*  Define BL's PON parameters                                   */
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
/**  SF Threshold - 3 to 8                                                  **/
/*****************************************************************************/
#ifndef RDPA_SF_THRESHOLD
#define RDPA_SF_THRESHOLD 3 
#endif

/*****************************************************************************/
/**  SD Threshold - 4 to 9                                                  **/
/*****************************************************************************/
#ifndef RDPA_SD_THRESHOLD
#define RDPA_SD_THRESHOLD 4
#endif

/*****************************************************************************/
/**  TO1 Timeout                                                            **/
/*****************************************************************************/
#ifndef RDPA_TO1_TIMEOUT
#define RDPA_TO1_TIMEOUT 20000
#endif

/*****************************************************************************/
/**  TO2 Timeout                                                            **/
/*****************************************************************************/
#ifndef RDPA_TO2_TIMEOUT
#define RDPA_TO2_TIMEOUT 100
#endif

/*****************************************************************************/
/**  BER interval - 1000 to 5000 msec                                       **/
/*****************************************************************************/
#ifndef RDPA_BER_INTERVAL
#define RDPA_BER_INTERVAL 1000
#endif

/*****************************************************************************/
/**  Minminal response time - Minimum 10 usec                                  **/
/*****************************************************************************/
#ifndef RDPA_MIN_RESPONSE_TIME
#define RDPA_MIN_RESPONSE_TIME 35
#endif


/*****************************************************************************/
/**  TX Data Polarity mode - Positive/Negative                              **/
/*****************************************************************************/
#ifndef RDPA_TX_DATA_POLARITY_MODE
#define RDPA_TX_DATA_POLARITY_MODE 0
#endif


/*****************************************************************************/
/**  RX Data Polarity mode - Positive/Negative                              **/
/*****************************************************************************/
#ifndef RDPA_RX_DIN_POLARITY_MODE
#define RDPA_RX_DIN_POLARITY_MODE 0
#endif

/*****************************************************************************/
/**  Number of pysnc for LOF assertion - 1 to 15                            **/
/*****************************************************************************/
#ifndef RDPA_RX_LOF_ASSERTION
#define RDPA_RX_LOF_ASSERTION 4
#endif

/*****************************************************************************/
/**  Number of pysnc for LOF clear - 1 to 15                                **/
/*****************************************************************************/
#ifndef RDPA_RX_LOF_CLEAR
#define RDPA_RX_LOF_CLEAR 1 
#endif

/*****************************************************************************/
/**  DV setup pattern - 16 bits                                             **/
/*****************************************************************************/
#ifndef RDPA_DV_SETUP_PATTERN
#define RDPA_DV_SETUP_PATTERN 0xFFF
#endif

/*****************************************************************************/
/**  DV hold pattern - 16 bits                                              **/
/*****************************************************************************/
#ifndef RDPA_DV_HOLD_PATTERN
#define RDPA_DV_HOLD_PATTERN 0xFFF0
#endif

/*****************************************************************************/
/**  DV polarity - High/Low                                                 **/
/*****************************************************************************/
#ifndef RDPA_DV_POLARITY
#define RDPA_DV_POLARITY rdpa_polarity_active_high
#endif

/*****************************************************************************/
/** task priority                                                           **/
/*****************************************************************************/
#ifndef RDPA_TASK_PRIORITY
#define RDPA_TASK_PRIORITY 50
#endif

/*****************************************************************************/
/** Power calibration mode - Disable/Enable                                 **/
/*****************************************************************************/
#ifndef RDPA_POWER_CALIBRATION_MODE
#define RDPA_POWER_CALIBRATION_MODE 0 /* 0 for disable, 1 for enable */
#endif

/*****************************************************************************/
/** Power calibration pattern - 32 bits                                     **/
/*****************************************************************************/
#ifndef RDPA_POWER_CALIBRATION_PATTERN
#define RDPA_POWER_CALIBRATION_PATTERN 0xAAAAAAAA
#endif

/*****************************************************************************/
/** Power calibration size (bytes) - muliply of 4 bytes, up to 508 bytes    **/
/*****************************************************************************/
#ifndef RDPA_POWER_CALIBRATION_SIZE
#define RDPA_POWER_CALIBRATION_SIZE 256
#endif

/*---------------------------------------------------------------------------*/
/*  Define BL's Bridge parameters                                */
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
/** VLAN Ethernet type detect 1 - 16 bits                                   **/
/*****************************************************************************/
#ifndef RDPA_VLAN_ETH_TYPE_DETECT_1
#define RDPA_VLAN_ETH_TYPE_DETECT_1 0x8100
#endif

/*****************************************************************************/
/** VLAN Ethernet type detect 2 - 16 bits                                   **/
/*****************************************************************************/
#ifndef RDPA_VLAN_ETH_TYPE_DETECT_2
#define RDPA_VLAN_ETH_TYPE_DETECT_2 0x88A8
#endif

/*****************************************************************************/
/** VLAN Ethernet type detect 3 - 16 bits                                   **/
/*****************************************************************************/
#ifndef RDPA_VLAN_ETH_TYPE_DETECT_3
#define RDPA_VLAN_ETH_TYPE_DETECT_3 0xffff /*invalid */
#endif

/*****************************************************************************/
/** VLAN Ethernet type detect 4 - 16 bits                                   **/
/*****************************************************************************/
#ifndef RDPA_VLAN_ETH_TYPE_DETECT_4
#define RDPA_VLAN_ETH_TYPE_DETECT_4 0xffff /*invalid */
#endif

/*****************************************************************************/
/** EMACs group mode                                                        **/
/*****************************************************************************/
#ifndef RDPA_EMACS_GROUP_MODE
#define RDPA_EMACS_GROUP_MODE rdpa_emac_mode_qsgmii
#endif

#ifndef RDPA_EMAC4_MODE
#define RDPA_EMAC4_MODE rdpa_emac_mode_rgmii
#endif

#ifndef RDPA_EMAC5_MODE
#define RDPA_EMAC5_MODE rdpa_emac_mode_sgmii
#endif

/*****************************************************************************/
/** WAN EMAC in GBE mode                                                    **/
/*****************************************************************************/
#ifndef RDPA_GBE_WAN_EMAC
#define RDPA_GBE_WAN_EMAC rdpa_emac4
#endif

/*---------------------------------------------------------------------------*/
/*  Define BL's EMAC0 parameters                                             */
/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/** Rate                                                                    **/
/*****************************************************************************/
#ifndef RDPA_EMAC0_RATE
#define RDPA_EMAC0_RATE rdpa_emac_rate_1g
#endif

/*****************************************************************************/
/** Generate CRC - Disable/Enable                                           **/
/*****************************************************************************/
#ifndef RDPA_EMAC0_GENERATE_CRC
#define RDPA_EMAC0_GENERATE_CRC 0
#endif

/*****************************************************************************/
/** EMAC duplex mode - full/half                                            **/
/*****************************************************************************/
#ifndef RDPA_EMAC0_DUPLEX_MODE
#define RDPA_EMAC0_DUPLEX_MODE 1
#endif

/*****************************************************************************/
/** Pad short packet - Disable/Enable                                       **/
/*****************************************************************************/
#ifndef RDPA_EMAC0_PAD_SHORT_PKT
#define RDPA_EMAC0_PAD_SHORT_PKT 1
#endif

/*****************************************************************************/
/** Allow too long frames - Disable/Enable                                  **/
/*****************************************************************************/
#ifndef RDPA_EMAC0_ALLOW_TOO_LONG_FRAMES
#define RDPA_EMAC0_ALLOW_TOO_LONG_FRAMES 1
#endif

/*****************************************************************************/
/** Check frame length - Disable/Enable                                     **/
/*****************************************************************************/
#ifndef RDPA_EMAC0_CHECK_FRAME_LENGTH
#define RDPA_EMAC0_CHECK_FRAME_LENGTH 1 
#endif

/*****************************************************************************/
/** Preamble length                                                         **/
/*****************************************************************************/
#ifndef RDPA_EMAC0_PREAMBLE_LENGTH
#define RDPA_EMAC0_PREAMBLE_LENGTH 7
#endif

/*****************************************************************************/
/** Back2Back inter-packet gap                                              **/
/*****************************************************************************/
#ifndef RDPA_EMAC0_BACK2BACK_GAP
#define RDPA_EMAC0_BACK2BACK_GAP 0x58 /*TBD*/
#endif

/*****************************************************************************/
/** Non - Back2Back inter-packet gap                                        **/
/*****************************************************************************/
#ifndef RDPA_EMAC0_NON_BACK2BACK_GAP
#define RDPA_EMAC0_NON_BACK2BACK_GAP 0x40 /*TBD*/
#endif

/*****************************************************************************/
/** Minimal inter-frame gap                                                 **/
/*****************************************************************************/
#ifndef RDPA_EMAC0_MIN_INTER_FRAME_GAP
#define RDPA_EMAC0_MIN_INTER_FRAME_GAP 0x50 /*TBD*/
#endif

/*---------------------------------------------------------------------------*/
/*  Define BL's EMAC1 parameters                                             */
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
/** Rate                                                                    **/
/*****************************************************************************/
#ifndef RDPA_EMAC1_RATE
#define RDPA_EMAC1_RATE rdpa_emac_rate_1g
#endif

/*****************************************************************************/
/** Generate CRC - Disable/Enable                                           **/
/*****************************************************************************/
#ifndef RDPA_EMAC1_GENERATE_CRC
#define RDPA_EMAC1_GENERATE_CRC 0
#endif

/*****************************************************************************/
/** EMAC duplex mode - full/half                                            **/
/*****************************************************************************/
#ifndef RDPA_EMAC1_DUPLEX_MODE
#define RDPA_EMAC1_DUPLEX_MODE 1
#endif

/*****************************************************************************/
/** Pad short packet - Disable/Enable                                       **/
/*****************************************************************************/
#ifndef RDPA_EMAC1_PAD_SHORT_PKT
#define RDPA_EMAC1_PAD_SHORT_PKT 1
#endif

/*****************************************************************************/
/** Allow too long frames - Disable/Enable                                  **/
/*****************************************************************************/
#ifndef RDPA_EMAC1_ALLOW_TOO_LONG_FRAMES
#define RDPA_EMAC1_ALLOW_TOO_LONG_FRAMES 1
#endif

/*****************************************************************************/
/** Check frame length - Disable/Enable                                     **/
/*****************************************************************************/
#ifndef RDPA_EMAC1_CHECK_FRAME_LENGTH
#define RDPA_EMAC1_CHECK_FRAME_LENGTH 1
#endif

/*****************************************************************************/
/** Preamble length                                                         **/
/*****************************************************************************/
#ifndef RDPA_EMAC1_PREAMBLE_LENGTH
#define RDPA_EMAC1_PREAMBLE_LENGTH 7
#endif

/*****************************************************************************/
/** Back2Back inter-packet gap                                              **/
/*****************************************************************************/
#ifndef RDPA_EMAC1_BACK2BACK_GAP
#define RDPA_EMAC1_BACK2BACK_GAP 0x58 /*TBD*/
#endif

/*****************************************************************************/
/** Non - Back2Back inter-packet gap                                        **/
/*****************************************************************************/
#ifndef RDPA_EMAC1_NON_BACK2BACK_GAP
#define RDPA_EMAC1_NON_BACK2BACK_GAP  0x40 /*TBD*/
#endif

/*****************************************************************************/
/** Minimal inter-frame gap                                                 **/
/*****************************************************************************/
#ifndef RDPA_EMAC1_MIN_INTER_FRAME_GAP
#define RDPA_EMAC1_MIN_INTER_FRAME_GAP 0x50 /*TBD*/
#endif

/*---------------------------------------------------------------------------*/
/*  Define BL's EMAC2 parameters                                             */
/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/** Rate                                                                    **/
/*****************************************************************************/
#ifndef RDPA_EMAC2_RATE
#define RDPA_EMAC2_RATE rdpa_emac_rate_1g
#endif

/*****************************************************************************/
/** Generate CRC - Disable/Enable                                           **/
/*****************************************************************************/
#ifndef RDPA_EMAC2_GENERATE_CRC
#define RDPA_EMAC2_GENERATE_CRC 0
#endif

/*****************************************************************************/
/** EMAC duplex mode - full/half                                            **/
/*****************************************************************************/
#ifndef RDPA_EMAC2_DUPLEX_MODE
#define RDPA_EMAC2_DUPLEX_MODE 1
#endif

/*****************************************************************************/
/** Pad short packet - Disable/Enable                                       **/
/*****************************************************************************/
#ifndef RDPA_EMAC2_PAD_SHORT_PKT
#define RDPA_EMAC2_PAD_SHORT_PKT 1
#endif

/*****************************************************************************/
/** Allow too long frames - Disable/Enable                                  **/
/*****************************************************************************/
#ifndef RDPA_EMAC2_ALLOW_TOO_LONG_FRAMES
#define RDPA_EMAC2_ALLOW_TOO_LONG_FRAMES 1
#endif

/*****************************************************************************/
/** Check frame length - Disable/Enable                                     **/
/*****************************************************************************/
#ifndef RDPA_EMAC2_CHECK_FRAME_LENGTH
#define RDPA_EMAC2_CHECK_FRAME_LENGTH 1 
#endif

/*****************************************************************************/
/** Preamble length                                                         **/
/*****************************************************************************/
#ifndef RDPA_EMAC2_PREAMBLE_LENGTH
#define RDPA_EMAC2_PREAMBLE_LENGTH 7
#endif

/*****************************************************************************/
/** Back2Back inter-packet gap                                              **/
/*****************************************************************************/
#ifndef RDPA_EMAC2_BACK2BACK_GAP
#define RDPA_EMAC2_BACK2BACK_GAP 0x58 /*TBD*/
#endif

/*****************************************************************************/
/** Non - Back2Back inter-packet gap                                        **/
/*****************************************************************************/
#ifndef RDPA_EMAC2_NON_BACK2BACK_GAP
#define RDPA_EMAC2_NON_BACK2BACK_GAP 0x40 /*TBD*/
#endif

/*****************************************************************************/
/** Minimal inter-frame gap                                                 **/
/*****************************************************************************/
#ifndef RDPA_EMAC2_MIN_INTER_FRAME_GAP
#define RDPA_EMAC2_MIN_INTER_FRAME_GAP 0x50 /*TBD*/
#endif

/*---------------------------------------------------------------------------*/
/*  Define BL's EMAC3 parameters                                             */
/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/** Rate                                                                    **/
/*****************************************************************************/
#ifndef RDPA_EMAC3_RATE
#define RDPA_EMAC3_RATE rdpa_emac_rate_1g
#endif

/*****************************************************************************/
/** Generate CRC - Disable/Enable                                           **/
/*****************************************************************************/
#ifndef RDPA_EMAC3_GENERATE_CRC
#define RDPA_EMAC3_GENERATE_CRC 0
#endif

/*****************************************************************************/
/** EMAC duplex mode - full/half                                            **/
/*****************************************************************************/
#ifndef RDPA_EMAC3_DUPLEX_MODE
#define RDPA_EMAC3_DUPLEX_MODE 1
#endif

/*****************************************************************************/
/** Pad short packet - Disable/Enable                                       **/
/*****************************************************************************/
#ifndef RDPA_EMAC3_PAD_SHORT_PKT
#define RDPA_EMAC3_PAD_SHORT_PKT 1 
#endif

/*****************************************************************************/
/** Allow too long frames - Disable/Enable                                  **/
/*****************************************************************************/
#ifndef RDPA_EMAC3_ALLOW_TOO_LONG_FRAMES
#define RDPA_EMAC3_ALLOW_TOO_LONG_FRAMES 1 
#endif

/*****************************************************************************/
/** Check frame length - Disable/Enable                                     **/
/*****************************************************************************/
#ifndef RDPA_EMAC3_CHECK_FRAME_LENGTH
#define RDPA_EMAC3_CHECK_FRAME_LENGTH 1
#endif

/*****************************************************************************/
/** Preamble length                                                         **/
/*****************************************************************************/
#ifndef RDPA_EMAC3_PREAMBLE_LENGTH
#define RDPA_EMAC3_PREAMBLE_LENGTH 7
#endif

/*****************************************************************************/
/** Back2Back inter-packet gap                                              **/
/*****************************************************************************/
#ifndef RDPA_EMAC3_BACK2BACK_GAP
#define RDPA_EMAC3_BACK2BACK_GAP 0x58 /*TBD*/
#endif

/*****************************************************************************/
/** Non - Back2Back inter-packet gap                                        **/
/*****************************************************************************/
#ifndef RDPA_EMAC3_NON_BACK2BACK_GAP
#define RDPA_EMAC3_NON_BACK2BACK_GAP 0x40 /*TBD*/
#endif

/*****************************************************************************/
/** Minimal inter-frame gap                                                 **/
/*****************************************************************************/
#ifndef RDPA_EMAC3_MIN_INTER_FRAME_GAP
#define RDPA_EMAC3_MIN_INTER_FRAME_GAP 0x50 /*TBD*/
#endif

/*---------------------------------------------------------------------------*/
/*  Define BL's EMAC4 parameters                                             */
/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/** Rate                                                                    **/
/*****************************************************************************/
#ifndef RDPA_EMAC4_RATE
#define RDPA_EMAC4_RATE rdpa_emac_rate_1g
#endif


/*****************************************************************************/
/** Generate CRC - Disable/Enable                                           **/
/*****************************************************************************/
#ifndef RDPA_EMAC4_GENERATE_CRC
#define RDPA_EMAC4_GENERATE_CRC 0
#endif

/*****************************************************************************/
/** EMAC duplex mode - full/half                                            **/
/*****************************************************************************/
#ifndef RDPA_EMAC4_DUPLEX_MODE
#define RDPA_EMAC4_DUPLEX_MODE 1
#endif

/*****************************************************************************/
/** Pad short packet - Disable/Enable                                       **/
/*****************************************************************************/
#ifndef RDPA_EMAC4_PAD_SHORT_PKT
#define RDPA_EMAC4_PAD_SHORT_PKT 1 
#endif

/*****************************************************************************/
/** Allow too long frames - Disable/Enable                                  **/
/*****************************************************************************/
#ifndef RDPA_EMAC4_ALLOW_TOO_LONG_FRAMES
#define RDPA_EMAC4_ALLOW_TOO_LONG_FRAMES 1
#endif

/*****************************************************************************/
/** Check frame length - Disable/Enable                                     **/
/*****************************************************************************/
#ifndef RDPA_EMAC4_CHECK_FRAME_LENGTH
#define RDPA_EMAC4_CHECK_FRAME_LENGTH 1
#endif

/*****************************************************************************/
/** Preamble length                                                         **/
/*****************************************************************************/
#ifndef RDPA_EMAC4_PREAMBLE_LENGTH
#define RDPA_EMAC4_PREAMBLE_LENGTH 7
#endif

/*****************************************************************************/
/** Back2Back inter-packet gap                                              **/
/*****************************************************************************/
#ifndef RDPA_EMAC4_BACK2BACK_GAP
#define RDPA_EMAC4_BACK2BACK_GAP 0x58 /*TBD*/
#endif

/*****************************************************************************/
/** Non - Back2Back inter-packet gap                                        **/
/*****************************************************************************/
#ifndef RDPA_EMAC4_NON_BACK2BACK_GAP
#define RDPA_EMAC4_NON_BACK2BACK_GAP 0x40 /*TBD*/
#endif

/*****************************************************************************/
/** Minimal inter-frame gap                                                 **/
/*****************************************************************************/
#ifndef RDPA_EMAC4_MIN_INTER_FRAME_GAP
#define RDPA_EMAC4_MIN_INTER_FRAME_GAP 0x50 /*TBD*/
#endif

/*---------------------------------------------------------------------------*/
/*  Define BL's EMAC5 parameters                                             */
/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/** Rate                                                                    **/
/*****************************************************************************/
#ifndef RDPA_EMAC5_RATE
#define RDPA_EMAC5_RATE rdpa_emac_rate_1g
#endif


/*****************************************************************************/
/** Generate CRC - Disable/Enable                                           **/
/*****************************************************************************/
#ifndef RDPA_EMAC5_GENERATE_CRC
#define RDPA_EMAC5_GENERATE_CRC 0
#endif

/*****************************************************************************/
/** EMAC duplex mode - full/half                                            **/
/*****************************************************************************/
#ifndef RDPA_EMAC5_DUPLEX_MODE
#define RDPA_EMAC5_DUPLEX_MODE 1
#endif

/*****************************************************************************/
/** Pad short packet - Disable/Enable                                       **/
/*****************************************************************************/
#ifndef RDPA_EMAC5_PAD_SHORT_PKT
#define RDPA_EMAC5_PAD_SHORT_PKT 1
#endif

/*****************************************************************************/
/** Allow too long frames - Disable/Enable                                  **/
/*****************************************************************************/
#ifndef RDPA_EMAC5_ALLOW_TOO_LONG_FRAMES
#define RDPA_EMAC5_ALLOW_TOO_LONG_FRAMES 1
#endif

/*****************************************************************************/
/** Check frame length - Disable/Enable                                     **/
/*****************************************************************************/
#ifndef RDPA_EMAC5_CHECK_FRAME_LENGTH
#define RDPA_EMAC5_CHECK_FRAME_LENGTH 1
#endif

/*****************************************************************************/
/** Preamble length                                                         **/
/*****************************************************************************/
#ifndef RDPA_EMAC5_PREAMBLE_LENGTH
#define RDPA_EMAC5_PREAMBLE_LENGTH 7
#endif

/*****************************************************************************/
/** Back2Back inter-packet gap                                              **/
/*****************************************************************************/
#ifndef RDPA_EMAC5_BACK2BACK_GAP
#define RDPA_EMAC5_BACK2BACK_GAP 0x58 /*TBD*/
#endif

/*****************************************************************************/
/** Non - Back2Back inter-packet gap                                        **/
/*****************************************************************************/
#ifndef RDPA_EMAC5_NON_BACK2BACK_GAP
#define RDPA_EMAC5_NON_BACK2BACK_GAP 0x40 /*TBD*/
#endif

/*****************************************************************************/
/** Minimal inter-frame gap                                                 **/
/*****************************************************************************/
#ifndef RDPA_EMAC5_MIN_INTER_FRAME_GAP
#define RDPA_EMAC5_MIN_INTER_FRAME_GAP 0x50 /*TBD*/
#endif

/*---------------------------------------------------------------------------*/

/*****************************************************************************/
/** US ETH QOS mode **/
/*****************************************************************************/
#ifndef RDPA_US_QOS_AND_SCHEDULE_METHOD_ETH_MAX_TCONT_NUM
#define RDPA_US_QOS_AND_SCHEDULE_METHOD_ETH_MAX_TCONT_NUM 8
#endif

/*****************************************************************************/
/** US scheduling - max number of US rate controllers                       **/
/*****************************************************************************/
#ifndef RDPA_MAX_US_RATE_CONTROLLERS
#define RDPA_MAX_US_RATE_CONTROLLERS    128
#endif

/*****************************************************************************/
/** US scheduling - max number of US channels                               **/
/*****************************************************************************/
#ifndef RDPA_MAX_US_CHANNELS
#define RDPA_MAX_US_CHANNELS            40
#endif

#endif /* RDPA_CONFIG_H_ */
