/*
 * <:copyright-BRCM:2020:DUAL/GPL:standard
 * 
 *    Copyright (c) 2020 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

#ifndef RDPA_CONFIG_H_
#define RDPA_CONFIG_H_

/*****************************************************************************/
/** MTU                                                                     **/
/*****************************************************************************/
#if (defined(CONFIG_BCM94912) || defined(CONFIG_BCM96855)) && defined(CONFIG_BCM_JUMBO_FRAME)
    #ifndef ENET_MAX_MTU_EXTRA_SIZE
        #define ENET_MAX_MTU_EXTRA_SIZE  32
    #endif
    extern int mtusize;
    static __inline__ int rdpa_mtu(void)
    {
        return (mtusize + ENET_MAX_MTU_EXTRA_SIZE);
    }
#elif defined(CONFIG_BCM_JUMBO_FRAME) && defined(CONFIG_BCM_MAX_MTU_SIZE)
#ifndef ENET_MAX_MTU_EXTRA_SIZE
#define ENET_MAX_MTU_EXTRA_SIZE  32
#endif
#define RDPA_MTU (CONFIG_BCM_MAX_MTU_SIZE + ENET_MAX_MTU_EXTRA_SIZE)
#elif defined(CONFIG_BCM_JUMBO_FRAME)
#if defined(BCM_DSL_RDP) || defined(BCM_DSL_XRDP)
/* defined in bcm_pkt_lengths.h
 * ENET_MAX_MTU_PAYLOAD_SIZE = 2048 (mini jumbo)
 * ENET_MAX_MTU_EXTRA_SIZE   = 30 (EH_SIZE(14) + VLANTAG(4) + VLANTAG(4) + BRCMTAG(4) + FCS(4))
 * RDPA_MTU = 2048 + 30 = 2078
 */
#define RDPA_MTU 2078
#else
/*RDPA_BPM_BUFFER_2K = 1958+ENET_MAX_MTU_EXTRA_SIZE+18+40*/
#define RDPA_MTU 1990
#endif /*DSL*/
#else
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
#if defined(BCM_DSL_RDP)
/* add 8 byte extra headroom for IPv6 dest_opt hdr */
#define RDPA_DS_LITE_HEADROOM_SIZE 48
#else
#define RDPA_DS_LITE_HEADROOM_SIZE 40
#endif
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
/**  DWELL_TIMER Timeout                                                    **/
/*****************************************************************************/
#ifndef RDPA_DWELL_TIMER_TIMEOUT
#define RDPA_DWELL_TIMER_TIMEOUT 10000
#endif

/*****************************************************************************/
/**  TO6 Timeout                                                            **/
/*****************************************************************************/
#ifndef RDPA_TO6_TIMEOUT
#define RDPA_TO6_TIMEOUT 10000
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
#ifdef CONFIG_BCM96858
#define RDPA_DV_SETUP_PATTERN 0xFFFF
#else
#define RDPA_DV_SETUP_PATTERN 0x0FFF
#endif
#endif

/*****************************************************************************/
/**  DV hold pattern - 16 bits                                              **/
/*****************************************************************************/
#ifndef RDPA_DV_HOLD_PATTERN
#ifdef CONFIG_BCM96858
#define RDPA_DV_HOLD_PATTERN 0xFFFF
#else
#define RDPA_DV_HOLD_PATTERN 0xFFF0
#endif
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
/** Power calibration size (bytes) - multiple of 4 bytes, up to 508 bytes    **/
/*****************************************************************************/
#ifndef RDPA_POWER_CALIBRATION_SIZE
#define RDPA_POWER_CALIBRATION_SIZE 72
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
#if defined(BCM63158)
#define RDPA_MAX_US_CHANNELS            50
#else
#define RDPA_MAX_US_CHANNELS            40
#endif
#endif

#endif /* RDPA_CONFIG_H_ */
