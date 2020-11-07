/*
*  Copyright 2016, Broadcom Corporation
*
* <:copyright-BRCM:2016:proprietary:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

#include <linux/spinlock.h>
#include "EponMac.h"
#include "Xif.h"
#include "PonMgrFec.h"
#ifdef EPON_NORMAL_MODE
#include "EponUser.h"
#endif
#include "drv_epon_xif_ag.h"
#include "drv_epon_xpcsrx_ag.h"
#include "drv_epon_xpcstx_ag.h"
#include "bcm_epon_common.h"
#include "wan_drv.h"
#include "opticaldet.h"

/*
Jumbo packets(>2000bytes) needs longer RX delay 
to avoid time jitter and US throughput downgrade
*/
#define JUMBO_10K_RD_TIMER_DLY_SET 234

//##############################################################################
// Local Type Definitions
//##############################################################################

typedef bdmf_error_t (*xif_stat_cb)(uint32_t *stats);

static xif_stat_cb xif_stat_get[XifStatCount] = 
    {
    ag_drv_xif_pmc_frame_rx_cnt_get,
    ag_drv_xif_pmc_byte_rx_cnt_get,
    ag_drv_xif_pmc_runt_rx_cnt_get,
    ag_drv_xif_pmc_cw_err_rx_cnt_get,
    ag_drv_xif_pmc_crc8_err_rx_cnt_get,
    ag_drv_xif_xpn_data_frm_cnt_get,
    ag_drv_xif_xpn_data_byte_cnt_get,
    ag_drv_xif_xpn_mpcp_frm_cnt_get,
    ag_drv_xif_xpn_oam_frm_cnt_get,
    ag_drv_xif_xpn_oam_byte_cnt_get,
    ag_drv_xif_xpn_oversize_frm_cnt_get,
    ag_drv_xif_sec_abort_frm_cnt_get
    };

typedef bdmf_error_t (*xpcs_32_stat_cb)(uint32_t *stats);

static xpcs_32_stat_cb xpcs_32_rx_stat_get[Xpcs32RxStatCount] = 
    {
    ag_drv_xpcsrx_rx_fec_cw_fail_cnt_get,
    ag_drv_xpcsrx_rx_fec_cw_tot_cnt_get,
    ag_drv_xpcsrx_rx_64b66b_fail_cnt_get,
    ag_drv_xpcsrx_rx_frmr_bad_sh_cnt_get,
    ag_drv_xpcsrx_rx_psudo_cnt_get,
    ag_drv_xpcsrx_rx_prbs_cnt_get
    };


typedef bdmf_error_t (*xpcs_40_stat32_l32_cb)(uint32_t *l32_stats);

typedef bdmf_error_t (*xpcs_40_stat32_u8_cb)(uint8_t *u8_stats);

typedef struct {
    xpcs_40_stat32_l32_cb  l32_stat;
    xpcs_40_stat32_u8_cb   u8_stat;
} xpcs_40_rx_stat_cb_t;

static xpcs_40_rx_stat_cb_t xpcs_40_rx_stat_get[Xpcs40RxStatCount] =
    {
        {
        ag_drv_xpcsrx_rx_fec_correct_cnt_lower_get, 
        ag_drv_xpcsrx_rx_fec_correct_cnt_upper_get
        }
    };

static U32 fecTxMode;
static FecRxLinkMode fecRxMode = FecRxLinkNum;

static const xpcsrx_rx_int_stat clr_all_xpcs_rx_int = 
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

static const xif_int_status clr_all_xif_int = {1,1,1,1,1,1,1};

static const xpcstx_tx_int_stat clr_all_xpcs_tx_int = {1,1,1,1,1,1,1};

#define PipeDelay           25

//##############################################################################

typedef enum
    {
    XifDpRamSelRxKey    = 0,
    XifDpRamSelTxKey    = 2,
    XifDpRamSelRxIV = 4,
    XifDpRamSelTxIV = 5
    } XifDpRamSel;

#define XifDpRamRd 0
#define XifDpRamWr 1

static DEFINE_SPINLOCK(xif_spinlock);

//##############################################################################
// Function Definitions
//##############################################################################

////////////////////////////////////////////////////////////////////////////////
/// \brief Waits for data port to be available
///
/// \param None
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
static
void DpWait (void)
    {
    bdmf_boolean dataportbusy = 1;
    uint8_t portselect = 0;
    uint8_t portopcode = 0;
    uint16_t portaddress = 0;
    bdmf_error_t drv_error = BDMF_ERR_OK;

    do
        {
        drv_error += ag_drv_xif_port_command_get(&dataportbusy, &portselect, &portopcode, &portaddress);
        }while(dataportbusy && (drv_error == BDMF_ERR_OK));
    
    } // DpWait


////////////////////////////////////////////////////////////////////////////////
/// \brief Writes a value to the data port
///
/// Checks for data port busy, issues write command
///
/// \param ramSel   RAM to write
/// \param addr     Data port address to write
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
static
void DpWrite (XifDpRamSel ramSel, U16 addr)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    // wait for dp to be ready
    DpWait ();
    drv_error = ag_drv_xif_port_command_set(0, ramSel, XifDpRamWr, addr);
    // wait again to be sure command had been taken
    DpWait ();
    } // DpWrite

/*
////////////////////////////////////////////////////////////////////////////////
/// \brief Reads a value from the data port
///
/// Checks for data port busy, issues write command
///
/// \param ramSel   RAM to ream from
/// \param addr     Data port address to write
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
static
void DpRead (XifDpRamSel ramSel, U16 addr)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    // wait for dp to be ready
    DpWait();
    drv_error = ag_drv_xif_port_command_set(0, ramSel, XifDpRamRd, addr);
    // wait again for dp to be ready
    DpWait();
    } // DpRead
*/


#define XifEncryptZeroOverhead      0x00
#define XifEncryptReserved          0x01
#define XifEncrypt8021AE           0x02
#define XifEncryptTripleChurn       0x03
#define XifEncryptDisable           0x00
#define XifEncryptRxEnable	   0x1
#define XifEncryptTxEnable       (0x1<<9)
#define XifEncryptKeyIdxShift	8

////////////////////////////////////////////////////////////////////////////////
/// \brief  Set/Clear encryption for a given link/key
///
/// \param mode security decrytion mode
/// \param direction direction of interest
/// \param link link of interest
/// \param keyIdx index of the key
/// \param key pointer to the key to set or NULL if encryption is disabled
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifKeySet (EncryptMode mode,
                Direction direction,
                LinkIndex link,
                U8 keyIdx,
                U32 const BULK *key,
                const U32 BULK* sci,
                U8 tci,
                U32 pn)
    {
    // Translation table
    xif_ctl ctl;
    bdmf_error_t drv_error = BDMF_ERR_OK;
    U8 modeE2ITab[EncryptModeMax] = {
        XifEncryptDisable,          // EncryptModeDisable
        XifEncryptZeroOverhead,          // EncryptModeAes
        XifEncryptZeroOverhead,     // EncryptModeZoh
        XifEncryptTripleChurn,    // EncryptModeTripleChurn
        XifEncrypt8021AE};  // EncryptMode8021AE
    U8 xifMode = modeE2ITab[mode];

    spin_lock_bh(&xif_spinlock);

    // If mode is set to disable, we clear the entry in the table
    if (mode == EncryptModeDisable)
        {
        drv_error += ag_drv_xif_port_data__set(7, 0);
        }
    else
        {
        memset((void*)&ctl, 0, sizeof(xif_ctl));
        drv_error += ag_drv_xif_ctl_get(&ctl);

        // Set (or re-set) the global encryption scheme
        if (direction == Upstream)
            {
            ctl.txencrypten = 1;
            ctl.txencryptmode = xifMode;
            }
        else
            {
            ctl.rxencrypten = 1;
            ctl.rxencryptmode = xifMode;
            }
        drv_error += ag_drv_xif_ctl_set (&ctl);

        if (mode == EncryptModeTripleChurn)
            {
            drv_error += ag_drv_xif_port_data__set(0, key[0]);
            drv_error += ag_drv_xif_port_data__set(1, key[1]);
            drv_error += ag_drv_xif_port_data__set(2, key[2]);
            drv_error += ag_drv_xif_port_data__set(7, 
                XifEncryptRxEnable | (keyIdx << XifEncryptKeyIdxShift));
            }
        else
            {
            U32 port7Input = 0;
            // load key words in reverse order
            drv_error += ag_drv_xif_port_data__set(0, key[3]);
            drv_error += ag_drv_xif_port_data__set(1, key[2]);
            drv_error += ag_drv_xif_port_data__set(2, key[1]);
            drv_error += ag_drv_xif_port_data__set(3, key[0]);

            if ((mode == EncryptModeZoh) || (mode == EncryptMode8021AE))
                {
                // load SCI words in reverse order
                drv_error += ag_drv_xif_port_data__set(4, sci[1]);
                drv_error += ag_drv_xif_port_data__set(5, sci[0]);
                }

            if (mode == EncryptMode8021AE)
                {
                drv_error += ag_drv_xif_port_data__set(6, pn);
                }

            if (direction == Upstream)
                {
                port7Input |= ((mode == EncryptMode8021AE) ? tci : 0) | 
                    XifEncryptTxEnable | 
                    (keyIdx << XifEncryptKeyIdxShift);
                }
            else
                {
                port7Input = XifEncryptRxEnable;
                }
            
            drv_error += ag_drv_xif_port_data__set(7, port7Input);
            }
        }

    // Send the command to data port
    if (direction == Upstream)
        {
        DpWrite (XifDpRamSelTxKey, link);
        }
    else
        {
        DpWrite (XifDpRamSelRxKey, (link << 1) + keyIdx);
        }

    spin_unlock_bh(&xif_spinlock);
    } // XifKeySet


////////////////////////////////////////////////////////////////////////////////
/// \brief Get the current security enable status for link
///
/// This function gets the security enable status for link
///
/// \param  None
///
/// \return
/// the current security encrypt enable status
////////////////////////////////////////////////////////////////////////////////
//extern
BOOL XifEncryptEnGet(LinkIndex link, Direction dir)
    {
    U32 sec_stat;
    bdmf_error_t drv_error = BDMF_ERR_OK;

    if (dir == Dnstream)
    	{
    	drv_error += ag_drv_xif_secrx_encrypt_get(&sec_stat);
    	}
    else
    	{
    	drv_error += ag_drv_xif_sectx_encrypt_get(&sec_stat);
    	}
    return (BOOL)((sec_stat &(1 << link)) >> link);
    } // XifRxEncryptEnBitmapGet


////////////////////////////////////////////////////////////////////////////////
/// \brief Get the current security key for a link
///
/// This function gets the active key for a given link.  It is used to detect a
/// key switch over.
///
/// \param  link Link index to check
///
/// \return
/// Active security key index
////////////////////////////////////////////////////////////////////////////////
//extern
U8 XifKeyInUse (LinkIndex link, Direction dir)
    {
    U32 key_stat;
    bdmf_error_t drv_error = BDMF_ERR_OK;

    if (dir == Upstream)
    	{
    	drv_error += ag_drv_xif_sectx_keynum_get(&key_stat);
    	}
    else
    	{
    	drv_error += ag_drv_xif_secrx_keynum_get(&key_stat);
    	}
	
    return (BOOL)((key_stat &(1 << link)) >> link);
    } // XifKeyInUse



////////////////////////////////////////////////////////////////////////////////
/// XifEncryptUpDisable - Disable upstream encryption global configuration
///
 // Parameters:
/// \none
///
/// \return
/// none
////////////////////////////////////////////////////////////////////////////////
//extern
void XifEncryptUpDisable (void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    xif_ctl ctl;
	
    memset((void*)&ctl, 0, sizeof(xif_ctl));
    drv_error += ag_drv_xif_ctl_get(&ctl);
	ctl.txencrypten = 0;
	ctl.txencryptmode = XifEncryptDisable;
	drv_error += ag_drv_xif_ctl_set (&ctl);
    } // XifEncryptUpDisable
    

/*
////////////////////////////////////////////////////////////////////////////////
/// \brief  Return last packet number (for AES reply protection)
///
/// \param link of interest
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
U32 XifLatestPktNum (LinkIndex link)
    {
    DpRead (XifDpRamSelRxIV, link);
    return OnuRegRead(&XifPortData[0]);
    }  // XifLatestPktNum
*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Reads XIF statistic
///
/// \param statistic to read
///
/// \return
/// statistic value
////////////////////////////////////////////////////////////////////////////////
//extern
U32 XifReadStat (XifStatId statId)
    {
    U32 stats = 0;
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error = xif_stat_get[statId](&stats);

    return stats;
    } // XifReadStat


////////////////////////////////////////////////////////////////////////////////
/// \brief Reads XPCS 32 bits width statistic
///
/// \param statistic to read
///
/// \return
/// statistic value
////////////////////////////////////////////////////////////////////////////////
//extern
U32 XifXpcsRead32Stat (Xpcs32RxStatId statId)
    {
    U32 stats = 0;
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error = xpcs_32_rx_stat_get[statId](&stats);

    return stats;
    }  // XifXpcsRead32Stat


////////////////////////////////////////////////////////////////////////////////
/// \brief Reads XPCS402 bits width statistic
///
/// \param statistic to read
///
/// \return
/// statistic value
////////////////////////////////////////////////////////////////////////////////
//extern
U64 XifXpcsRead40Stat (Xpcs40RxStatId statId)
    {
    U32 lower32 = 0;
    U8 upper8 = 0;
    U64 stats = 0;
    
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error = xpcs_40_rx_stat_get[statId].l32_stat(&lower32);
    drv_error += xpcs_40_rx_stat_get[statId].u8_stat(&upper8);
    
    stats = (((U64)upper8)<<32) + lower32;
    
    return stats;
    }  // XifXpcsRead40Stat


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set FEC Tx links
///
/// \param linkMap Bitmap of links to enable FEC tx
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
static
void XifFecTxCfg (U8 type)
    {
    xif_ctl ctl;
    xpcstx_tx_control xpcs_tx_ctl;
    bdmf_error_t drv_error = BDMF_ERR_OK;

    
    if (type == 2)
        {
        // disable
        drv_error += ag_drv_xif_ctl_get(&ctl);
        ctl.cfgfecen = 0;
        drv_error += ag_drv_xif_ctl_set(&ctl);

        drv_error += ag_drv_xpcstx_tx_control_get(&xpcs_tx_ctl);
        xpcs_tx_ctl.cfgentxfec125 = 0;
        drv_error += ag_drv_xpcstx_tx_control_set(&xpcs_tx_ctl);
        }
    else
        {
        drv_error += ag_drv_xif_ctl_get(&ctl);
        ctl.cfgfecen = 1;
        drv_error += ag_drv_xif_ctl_set(&ctl);

        drv_error += ag_drv_xpcstx_tx_control_get(&xpcs_tx_ctl);
        xpcs_tx_ctl.cfgentxfec125 = 1;
        drv_error += ag_drv_xpcstx_tx_control_set(&xpcs_tx_ctl);
        }
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set FEC Rx links
///
/// \param Fec Rx mode
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
static
void XifFecRxCfg (U8 type)
    {
    xpcsrx_rx_framer_ctl rx_fm_ctl;
    xpcsrx_rx_fec_ctl rx_fec_ctl = {0,0,0,0,0,0,0,0,0,0,0};
    bdmf_error_t drv_error = BDMF_ERR_OK;

    // 0 - links all enabled FEC
    // 1 - links both enabled/disabled FEC
    // 2 - links all disable FEC
    if (type == 2)
        {
        drv_error += ag_drv_xpcsrx_rx_framer_ctl_get(&rx_fm_ctl);
        rx_fm_ctl.cfgxpcsrxframefec = 0;
        drv_error += ag_drv_xpcsrx_rx_framer_ctl_set(&rx_fm_ctl);
        
        rx_fec_ctl.cfgxpcsrxfecbypas = 1;
        drv_error += ag_drv_xpcsrx_rx_fec_ctl_set(&rx_fec_ctl);
        }
    else
        {
        drv_error += ag_drv_xpcsrx_rx_framer_ctl_get(&rx_fm_ctl);
        rx_fm_ctl.cfgxpcsrxframefec = 1;
        drv_error += ag_drv_xpcsrx_rx_framer_ctl_set(&rx_fm_ctl);
        
        rx_fec_ctl.cfgxpcsrxfecen = 1;
        rx_fec_ctl.cfgxpcsrxfecidleins = 1;
        rx_fec_ctl.cfgxpcsrxfecfailblksh0 = 1;
        drv_error += ag_drv_xpcsrx_rx_fec_ctl_set(&rx_fec_ctl);
        }
    } // XifFecRxCfg
    

////////////////////////////////////////////////////////////////////////////////
/// \brief  Set 10G FEC Rx 
///
/// \param linkMap Bitmap of links to enable FEC rx
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifFecRxSet (LinkMap linkMap)
    {
    
    bdmf_boolean oldValRx;
    xif_ctl ctl;
    bdmf_error_t drv_error = BDMF_ERR_OK;
    if(fecRxMode == linkMap)
        {
        return;
        }

    drv_error += ag_drv_xif_ctl_get(&ctl);
    ctl.secrxrstn = 0;
    ctl.pmcrxrstn = 0;
    drv_error += ag_drv_xif_ctl_set(&ctl);

    drv_error += ag_drv_xpcsrx_rx_rst_get(&oldValRx);
    drv_error += ag_drv_xpcsrx_rx_rst_set(0);
    
    XifFecRxCfg((linkMap == 0) ? 2 : 0);
    
    fecRxMode = linkMap;

    drv_error += ag_drv_xpcsrx_rx_rst_set(oldValRx);

    drv_error += ag_drv_xif_ctl_get(&ctl);
    ctl.secrxrstn = 1;
    ctl.pmcrxrstn = 1;
    drv_error += ag_drv_xif_ctl_set(&ctl);

    drv_error += ag_drv_xpcsrx_rx_int_stat_set(&clr_all_xpcs_rx_int);
    drv_error += ag_drv_xif_int_status_set(&clr_all_xif_int);
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set 10G FEC Tx 
///
/// \param linkMap Bitmap of links to enable FEC tx
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifFecTxSet (LinkMap linkMap)
    {
    static Bool firstSet = TRUE;
    xif_ctl ctl;
    xpcstx_tx_control xpcs_tx_ctl;
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    if((!firstSet)&&(fecTxMode == linkMap))
        {
        return;
        }
    
    drv_error += ag_drv_xif_ctl_get(&ctl);
    ctl.xpntxrstn = 0;
    ctl.pmctxrstn = 0;
    ctl.sectxrstn = 0;
    drv_error += ag_drv_xif_ctl_set(&ctl);
    
    drv_error += ag_drv_xpcstx_tx_control_get(&xpcs_tx_ctl);
    xpcs_tx_ctl.pcstxrstn = 0;
    drv_error += ag_drv_xpcstx_tx_control_set(&xpcs_tx_ctl);
    
    XifFecTxCfg ((linkMap == 0) ? 2 : 0);

    drv_error += ag_drv_xpcstx_tx_control_get(&xpcs_tx_ctl);
    xpcs_tx_ctl.pcstxrstn = 1;
    drv_error += ag_drv_xpcstx_tx_control_set(&xpcs_tx_ctl);

    drv_error += ag_drv_xif_ctl_get(&ctl);
    ctl.xpntxrstn = 1;
    ctl.pmctxrstn = 1;
    ctl.sectxrstn = 1;
    drv_error += ag_drv_xif_ctl_set(&ctl);

    drv_error += ag_drv_xpcstx_tx_int_stat_set(&clr_all_xpcs_tx_int);
    drv_error += ag_drv_xif_int_status_set(&clr_all_xif_int);
    
    fecTxMode = linkMap;    
    firstSet = FALSE;
    }


////////////////////////////////////////////////////////////////////////////////
//extern
void XifSetDiscIdleTime (U16 front, U16 back)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    drv_error += ag_drv_xif_discover_overhead_set(front);
    }


////////////////////////////////////////////////////////////////////////////////
//extern
void XifSetNonDiscIdleTime (U16 front, U16 back)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error += ag_drv_xif_gnt_overhead_set(front);
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Get a link state from the lookup table
///
/// Logical links are numbered from 0;
///
/// \param link     Logical link number
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
static
BOOL XifLinkState (LinkIndex link)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    U32 cfgonullid = 0;

    drv_error += ag_drv_xif_llid__get(link, &cfgonullid);
    
    return TestBitsSet (cfgonullid, XifLlidEn);
    } // XifLinkState


////////////////////////////////////////////////////////////////////////////////
/// \brief  Get all link state from the lookup table
///
/// Logical links are numbered from 0;
///
/// \param none
///
/// \return
/// link bit map
////////////////////////////////////////////////////////////////////////////////
//extern
U32 XifAllLinkState (void)
    {
    U8 link;
    U32 linkVal = 0;

    for (link = 0; link < TkOnuNumTxLlids; ++link)
        {
        if (XifLinkState (link))
            {
            linkVal |= (1UL << link);
            }
        }
    return linkVal;
    } // XifAllLinkState


////////////////////////////////////////////////////////////////////////////////
/// \brief  Disable a link from the lookup table
///
/// Logical links are numbered from 0;
///
/// \param link     Logical link number
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifDisableLink (LinkIndex link)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    U32 cfgonullid = 0;

    drv_error += ag_drv_xif_llid__get(link, &cfgonullid);
    cfgonullid &= (~XifLlidEn);

    drv_error += ag_drv_xif_llid__set(link, cfgonullid);
    } // XifDisableLink


////////////////////////////////////////////////////////////////////////////////
/// \brief Enable lookup for incoming traffic
///
/// Logical links are numbered from 0; the packet preamble has a 16-bit
/// LLID value within it that is not necessarily the same.
///
/// \param link     Logical link number
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifEnableLink (LinkIndex link)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    U32 cfgonullid = 0;

    drv_error += ag_drv_xif_llid__get(link, &cfgonullid);
    cfgonullid |= XifLlidEn;
    
    drv_error += ag_drv_xif_llid__set(link, cfgonullid);
    } // XifEnableLink

////////////////////////////////////////////////////////////////////////////////
/// \brief Delete physical LLID value associated with a link
///
/// Logical links are numbered from 0; the packet preamble has a 16-bit
///
/// \param link     Logical link number
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifDeleteLink (LinkIndex link)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    drv_error += ag_drv_xif_llid__set(link, 0);
    } // XifDeleteLink

////////////////////////////////////////////////////////////////////////////////
/// \brief sets physical LLID value associated with a link
///
/// Logical links are numbered from 0; the packet preamble has a 16-bit
/// LLID value within it that is not necessarily the same.
///
/// \param link     Logical link number
/// \param phy      Physical LLID value for this link
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifCreateLink (LinkIndex link, PhyLlid phy)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    U32 cfgonullid = (U32)phy | XifLlidEn;
    
    drv_error += ag_drv_xif_llid__set(link, cfgonullid);
    } // XifCreateLink


////////////////////////////////////////////////////////////////////////////////
/// \brief Return a PhyLlid associated to a link index
///
/// \param link     Logical link number
///
/// \return Physical Llid
////////////////////////////////////////////////////////////////////////////////
//extern
PhyLlid XifGetPhyLlid (LinkIndex link)
    {
    MultiByte32 val;
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    drv_error += ag_drv_xif_llid__get(link, &val.u32);
    
    return (PhyLlid)(val.words.lsw.u16);
    } // XifGetPhyLlid


////////////////////////////////////////////////////////////////////////////////
/// \brief  Return a link index associated to a phy llid
///
/// \param phy     Phy LLID
///
/// \return Link index
////////////////////////////////////////////////////////////////////////////////
//extern
LinkIndex XifPhyLlidToIndex (PhyLlid phy)
    {
    LinkIndex result = 0;

    while ((result < XifLlidCount) &&
           (XifGetPhyLlid(result) != phy))
        {
        ++result;
        }

    return result;
    } // XifPhyLlidToIndex


////////////////////////////////////////////////////////////////////////////////
/// \brief Set transmit laser Mode
///
/// \param mode new tx laser mode
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifLaserTxModeSet (LaserTxMode mode)
    {
    xpcstx_tx_control xpcs_tx_ctl;
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error += ag_drv_xpcstx_tx_control_get(&xpcs_tx_ctl);
    
    if (mode == LaserTxModeOff)
        {
        
        xpcs_tx_ctl.cfglsrtristateen125 = 0;
        }
    else
        {
        xpcs_tx_ctl.cfglsrtristateen125 = 1;
        if (EponGetMacMode() != EPON_MAC_AE_MODE)        
            {
            if (mode == LaserTxModeContinuous)
                {
                xpcs_tx_ctl.cfgenlsralways125 = 1;
                }
            else
                {
                xpcs_tx_ctl.cfgenlsralways125 = 0;
                }
            }
        else
             xpcs_tx_ctl.cfgenlsralways125 = 1;
        }

    drv_error += ag_drv_xpcstx_tx_control_set(&xpcs_tx_ctl);
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Does the XIF have code word lock?
///
/// \return
/// TRUE if XIF has code word lock, FALSE otherwise
////////////////////////////////////////////////////////////////////////////////
//extern
BOOL XifLocked(void)
    {
    BOOL lock;
    xpcsrx_rx_int_stat rx_int;
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error += ag_drv_xpcsrx_rx_int_stat_get(&rx_int);
    
    // we are locked if the loss interrupt has not triggered
    lock = rx_int.intrxframercwloss? FALSE : TRUE; 

    // clear the loss interrupt after reading to prepare for next check
    if (!lock)
        {
        memset((void*)&rx_int, 0, sizeof(xpcsrx_rx_int_stat));
        rx_int.intrxframercwloss = 1;
        drv_error += ag_drv_xpcsrx_rx_int_stat_set(&rx_int);
        }

    return lock;
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Disable XPCS framer
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifRxDisable(void)
    {
    xpcsrx_rx_framer_ctl rx_fm_ctl;
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    drv_error += ag_drv_xpcsrx_rx_framer_ctl_get(&rx_fm_ctl);
    rx_fm_ctl.cfgxpcsrxfrmren = 0;
    drv_error += ag_drv_xpcsrx_rx_framer_ctl_set(&rx_fm_ctl);
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Enable XPCS framer
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifRxEnable(void)
    {
    xpcsrx_rx_framer_ctl rx_fm_ctl;
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    drv_error += ag_drv_xpcsrx_rx_framer_ctl_get(&rx_fm_ctl);
    rx_fm_ctl.cfgxpcsrxfrmren = 1;
    drv_error += ag_drv_xpcsrx_rx_framer_ctl_set(&rx_fm_ctl);
    }

////////////////////////////////////////////////////////////////////////////////
/// \brief  Initialize (Or Re-init) the Xpcs Receive module
///
/// \param
/// None
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
static
void XifInitXpcsRx (void)
    {
    xpcsrx_rx_framer_ctl rx_fm_ctl = {0};
    xpcsrx_rx_fec_ctl rx_fec_ctl  = {0};
    bdmf_error_t drv_error = BDMF_ERR_OK;
    int check_loop_cnt = 0;
    bdmf_boolean intrxidleraminitdone, intrxfecnqueraminitdone, intrxfecdecraminitdone;
    
    EponTopClearReset(EponTopXpcsRxRst, TRUE);
    mdelay(20);
	
    drv_error += ag_drv_xpcsrx_rx_rst_set(1);
	
    /* poll whether xpcs is ready */
    do {
        mdelay(10);
        drv_error += ag_drv_xpcsrx_rx_ram_ecc_int_stat_get(&intrxidleraminitdone, &intrxfecnqueraminitdone, &intrxfecdecraminitdone);
        check_loop_cnt++;
        } while ((check_loop_cnt < 100) && (intrxidleraminitdone == 0));
    
    if (intrxidleraminitdone == 0)
        {
        printk("xpcs not ready yet after %d times check \n", check_loop_cnt);
        }
    
    if ((EponGetMacMode() != EPON_MAC_AE_MODE) && ((ENET_MAX_MTU_PAYLOAD_SIZE+ENET_MAX_MTU_EXTRA_SIZE) > 2000))
        {
        ag_drv_xpcsrx_rx_idle_rd_timer_dly_set(JUMBO_10K_RD_TIMER_DLY_SET);
        }

    // Enable frames to flow into the XPCS logic
    if (EponGetMacMode() != EPON_MAC_AE_MODE)
        rx_fm_ctl.cfgxpcsrxfrmren = 1;
    rx_fm_ctl.cfgxpcsrxfrmrebdvlden = 1;
    rx_fm_ctl.cfgxpcsrxfrmrspulken = 1;
    drv_error += ag_drv_xpcsrx_rx_framer_ctl_set(&rx_fm_ctl);

    rx_fec_ctl.cfgxpcsrxfecbypas = 1;
    rx_fec_ctl.cfgxpcsrxfecfailblksh0 = 1;
    ag_drv_xpcsrx_rx_fec_ctl_set(&rx_fec_ctl);

    drv_error += ag_drv_xpcsrx_rx_int_stat_set(&clr_all_xpcs_rx_int);

/*
    drv_error += ag_drv_xpcsrx_rx_idle_gap_siz_max_get(&sizeMax, &gap);
    sizeMax = XpcsRxMaxBlocks;
    drv_error += ag_drv_xpcsrx_rx_idle_gap_siz_max_set(sizeMax, gap);
*/
    } // XifInitXpcsRx


////////////////////////////////////////////////////////////////////////////////
//extern
void XifRxClk161Disable(void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error += ag_drv_xpcsrx_rx_rst_set(0);
    }


////////////////////////////////////////////////////////////////////////////////
//extern
void XifRxClk161Enable(void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error += ag_drv_xpcsrx_rx_rst_set(1);
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Setup XIF IPG insertion
///
/// \param bytesToInsert      Number of bytes to insert in IPG or 0 to disable
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifIpgInsertionSet (U8 bytesToInsert)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    BOOL cfgshortipg = 0;
    BOOL cfginsertipg = 0;
    U8 cfgipgword;
    
    if (bytesToInsert == 0)
        {
        drv_error += ag_drv_xif_ipg_insertion_get(&cfgshortipg, &cfginsertipg, &cfgipgword);
        cfginsertipg = 0;
        drv_error += ag_drv_xif_ipg_insertion_set(cfgshortipg, cfginsertipg, cfgipgword);
        }
    else
        {
        cfginsertipg = 1;
        cfgipgword = (XifIpgInsertionCfgWordMsk & bytesToInsert);
        drv_error += ag_drv_xif_ipg_insertion_set(cfgshortipg, cfginsertipg, cfgipgword);
        }
    }


////////////////////////////////////////////////////////////////////////////////
//extern
void XifTransportTimeSet(U32 time)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    drv_error += ag_drv_xif_transport_time_set(time);
    }


////////////////////////////////////////////////////////////////////////////////
//extern
U32 XifMpcpTimeGet(void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    U32 curmpcpts;

    drv_error += ag_drv_xif_mpcp_time_get(&curmpcpts);
    return curmpcpts;
    }

////////////////////////////////////////////////////////////////////////////////
//extern
void XifLaserMonSet(BOOL enable, U32 threshold)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    xpcstx_tx_int_stat int_status;
    xpcstx_tx_int_mask int_mask;
    bdmf_boolean laserenstatus;
    bdmf_boolean cfglsrmonacthi;
    bdmf_boolean lasermonrstn;
    int rc ;
    int bus = -1;
    TRX_SIG_ACTIVE_POLARITY  tx_sd_polarity;

    if (enable)
        {
        drv_error += ag_drv_xpcstx_tx_int_mask_get(&int_mask);
        int_mask.laseroffmask = TRUE;
        int_mask.laseronmaxmask = TRUE;
        drv_error += ag_drv_xpcstx_tx_int_mask_set(&int_mask);

        drv_error += ag_drv_xpcstx_tx_laser_monitor_max_thresh_set(threshold);
        }


    drv_error += ag_drv_xpcstx_tx_laser_monitor_ctl_get(&laserenstatus, &cfglsrmonacthi, &lasermonrstn);
    lasermonrstn = enable;
    
    opticaldet_get_xpon_i2c_bus_num(&bus);
    rc = trx_get_tx_sd_polarity(bus, &tx_sd_polarity) ;
    if (rc == 0)
        cfglsrmonacthi = (tx_sd_polarity == TRX_ACTIVE_HIGH)? TRUE : FALSE;

    drv_error += ag_drv_xpcstx_tx_laser_monitor_ctl_set(laserenstatus, cfglsrmonacthi, lasermonrstn);

    if (!enable)
        {
        drv_error += ag_drv_xpcstx_tx_int_stat_get(&int_status);
        int_status.laseroff = TRUE;
        int_status.laseronmax = TRUE;
        drv_error += ag_drv_xpcstx_tx_int_stat_set(&int_status);
        }
    }


////////////////////////////////////////////////////////////////////////////////
//extern
U32 XifLaserMonLaserOnMaxIntGet(void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    xpcstx_tx_int_stat int_status;
    xpcstx_tx_int_mask int_mask;

    drv_error += ag_drv_xpcstx_tx_int_stat_get(&int_status);
    drv_error += ag_drv_xpcstx_tx_int_mask_get(&int_mask);

    if (int_status.laseronmax && int_mask.laseronmaxmask)
        {
        return 1;
        }

    return 0;
    }


////////////////////////////////////////////////////////////////////////////////
//extern
void XifLaserMonClrLaserOnMaxInt(void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    xpcstx_tx_int_stat int_status;
    
    drv_error += ag_drv_xpcstx_tx_int_stat_get(&int_status);
    int_status.laseronmax = TRUE;
    drv_error += ag_drv_xpcstx_tx_int_stat_set(&int_status);
    }


////////////////////////////////////////////////////////////////////////////////
//extern
void XifInrpStatusGet(xif_int_status *int_status)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    drv_error += ag_drv_xif_int_status_get(int_status);
    }

////////////////////////////////////////////////////////////////////////////////
//extern
void XifInrpMaskSet(const xif_int_mask *int_mask)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error += ag_drv_xif_int_mask_set(int_mask);
    }


////////////////////////////////////////////////////////////////////////////////
//extern
void XifXpcsInrpStatusGet(xpcstx_tx_int_stat *tx_int_stat)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    drv_error += ag_drv_xpcstx_tx_int_stat_get(tx_int_stat);
    }

////////////////////////////////////////////////////////////////////////////////
//extern
void XifXpcsInrpMaskSet(const xpcstx_tx_int_mask *tx_int_mask)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    
    drv_error += ag_drv_xpcstx_tx_int_mask_set(tx_int_mask);
    }

////////////////////////////////////////////////////////////////////////////////
//extern
void XifXpcsLbePolaritySet(Polarity polarity)
    {
    xpcstx_tx_control xpcs_tx_ctl = {0};
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error += ag_drv_xpcstx_tx_control_get(&xpcs_tx_ctl);
    if (polarity == ActiveHi)
        xpcs_tx_ctl.cfglsrenacthi125 = 1;
    else
        xpcs_tx_ctl.cfglsrenacthi125 = 0;
    drv_error += ag_drv_xpcstx_tx_control_set(&xpcs_tx_ctl);

    }

////////////////////////////////////////////////////////////////////////////////
/// \brief  Initialize (Or Re-init) the Xpcs Transmit module
///
/// \param
/// None
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
static
void XifInitXpcsTx (U8 txOffTimeOffset, Polarity polarity)
    {
    xpcstx_tx_control xpcs_tx_ctl = {0};
    bdmf_error_t drv_error = BDMF_ERR_OK;
    int check_loop_cnt = 0;
	
    EponTopClearReset(EponTopXpcsTxRst, TRUE);
    mdelay(20);

    xpcs_tx_ctl.pcstxdtportrstn = 1;
    drv_error += ag_drv_xpcstx_tx_control_set(&xpcs_tx_ctl);
	
    /* poll whether xpcstx is ready */
    do {
        mdelay(10);
        drv_error += ag_drv_xpcstx_tx_control_get(&xpcs_tx_ctl);
        check_loop_cnt++;
        } while ((check_loop_cnt < 100) && 
        	(xpcs_tx_ctl.pcstxnotrdy == 1));
    
    if (xpcs_tx_ctl.pcstxnotrdy == 1)
        {
        printk("xpctx not ready yet after %d times check \n", check_loop_cnt);
        }

    xpcs_tx_ctl.pcstxrstn = 1;
    xpcs_tx_ctl.cfgentxscrb125 = 1;
    xpcs_tx_ctl.cfgentxout125 = 1;

    if (polarity == ActiveHi)
        xpcs_tx_ctl.cfglsrenacthi125 = 1;
    else
        xpcs_tx_ctl.cfglsrenacthi125 = 0;
        
    xpcs_tx_ctl.cfglsrtristateen125 = 1;
    if (EponGetMacMode() == EPON_MAC_AE_MODE)
        xpcs_tx_ctl.cfgenlsralways125 = 1;
    drv_error += ag_drv_xpcstx_tx_control_set(&xpcs_tx_ctl);
    if (EponGetMacMode() == EPON_MAC_AE_MODE)
        ag_drv_xpcstx_tx_mac_mode_set(1);
    drv_error += ag_drv_xpcstx_tx_laser_time_set(PipeDelay, txOffTimeOffset, 0);
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Initialize module
///
/// \param  txOffTimeOffset     Delay normal laser Off (in TQ)
/// \param  txOffIdle           If TRUE we insert idle when laser is OFF
/// \param  upRate              Upstream laser rate
/// \param  dnRate              Downstream laser rate
///
/// \return None
////////////////////////////////////////////////////////////////////////////////
//extern
void XifInit (U8 txOffTimeOffset, BOOL txOffIdle,
              Polarity polarity, LaserRate upRate, LaserRate dnRate)
    {
#if !defined(CONFIG_BCM96856) && !defined(CONFIG_BCM963158)
    xif_sec_ctl sec_ctl;
#endif
    xif_ctl ctl = {0};
    xif_pmctx_ctl pmctx_ctl;
    int check_loop_cnt = 0;
    bdmf_error_t drv_error = BDMF_ERR_OK;
    //UNUSED(txOffIdle); // Ignore the IDLE configuration for 10G.

    EponTopClearReset(EponTopXifR, TRUE);
    mdelay(20);

    // set xif ram init
    ctl.xifdtportrstn = 1;
    ctl.cfgpmcrxencrc8chk = 1;
    drv_error += ag_drv_xif_ctl_set(&ctl);
    /* poll whether xif is ready */
    do {
        mdelay(10);
        drv_error += ag_drv_xif_ctl_get(&ctl);
        check_loop_cnt++;
        } while ((check_loop_cnt < 100) && (ctl.xifnotrdy == 1));
    
    if (ctl.xifnotrdy == 1)
        {
        printk("xif ram not ready yet after %d times check \n", check_loop_cnt);
        }

    if (EponGetMacMode() == EPON_MAC_AE_MODE)
        {
        ag_drv_xif_mac_mode_set(1);
        ctl.cfgenp2p = 1;
        ctl.cfgpmcrxencrc8chk = 0;
        }
    else
        {
        ctl.cfgenp2p = 0;
        ctl.cfgpmcrxencrc8chk = 0;
        }

    ctl.secrxrstn = 1;
    ctl.pmcrxrstn = 1;
    ctl.cfgpmcrxencrc8chk = 1;
    drv_error += ag_drv_xif_ctl_set(&ctl);

    ag_drv_xif_xpn_oversize_thresh_set(0x2710UL);

#if !defined(CONFIG_BCM96856) && !defined(CONFIG_BCM963158)
    drv_error += ag_drv_xif_sec_ctl_get(&sec_ctl);
    sec_ctl.cfgsecrxenpktnumrlovr = 1;
    sec_ctl.cfgsectxenpktnumrlovr = 1;
    drv_error += ag_drv_xif_sec_ctl_set(&sec_ctl); 
#endif

/*
    // Enable Replay protection in case we have encryption ON
    sec_ctl.cfgenaereplayprct = 1;
    drv_error += ag_drv_xif_sec_ctl_set(&sec_ctl);
*/
 
    drv_error += ag_drv_xif_int_status_set(&clr_all_xif_int);

    // Init Xpcs Rx
    XifInitXpcsRx();

    if (upRate == LaserRate10G)
        {
        XifInitXpcsTx(txOffTimeOffset, polarity);

        ctl.sectxrstn = 1;
        ctl.pmctxrstn = 1;
        ctl.xpntxrstn = 1;
        drv_error += ag_drv_xif_ctl_set(&ctl);
        }

    if (EponGetMacMode() == EPON_MAC_AE_MODE)
        {
        ag_drv_xif_pmctx_ctl_get(&pmctx_ctl);
        pmctx_ctl.cfgennegtimeabort = 1;
        ag_drv_xif_pmctx_ctl_set(&pmctx_ctl);
        }
    }

// End of Xif.c
