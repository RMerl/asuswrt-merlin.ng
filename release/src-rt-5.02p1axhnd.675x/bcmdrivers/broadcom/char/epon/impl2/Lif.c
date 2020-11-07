/*
*  Copyright 2011, Broadcom Corporation
*
* <:copyright-BRCM:2011:proprietary:standard
* 
*    Copyright (c) 2011 Broadcom 
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
#include "Lif.h"
#ifdef EPON_NORMAL_MODE
#include "EponUser.h"
#include "PonMgrFec.h"
#endif
#include "drv_epon_lif_ag.h"
#include "drv_epon_epon_top_ag.h"
#include "bcm_epon_common.h"
#include "opticaldet.h"


U32 volatile __REGISTER_SPACE[MACREGSPACE];


#define LifLlidEn                           0x00010000UL // Offset 16
#define LifLaserOffIdleBefore               0x00000080UL
#define LifLlidLkup0Msk                     0x0000FFFFUL // [15:0]

#define LifLlidCount                        TkOnuNumTotalLlids



//##############################################################################
// Local Type Definitions
//##############################################################################

typedef bdmf_error_t (*lif_stat_cb)(uint32_t *stats);


static lif_stat_cb lif_stat_get[LifStatCount] = 
    {
    ag_drv_lif_tx_pkt_cnt_get,              /* LifTxFrames */
    ag_drv_lif_tx_byte_cnt_get,             /* LifTxBytes */
    ag_drv_lif_tx_non_fec_pkt_cnt_get,      /* LifTxNonFecFrames */
    ag_drv_lif_tx_non_fec_byte_cnt_get,     /* LifTxNonFecBytes */
    ag_drv_lif_tx_fec_pkt_cnt_get,          /* LifTxFecFrames */
    ag_drv_lif_tx_fec_byte_cnt_get,         /* LifTxFecBytes */
    ag_drv_lif_tx_fec_blk_cnt_get,          /* LifTxFecBlocks */
    ag_drv_lif_tx_mpcp_pkt_cnt_get,         /* LifTxReportFrames */
    ag_drv_lif_rx_line_code_err_cnt_get,    /* LifRxLineCodeErrors */
    ag_drv_lif_rx_agg_mpcp_frm_get,         /* LifRxGateFrames */
    ag_drv_lif_rx_agg_good_frm_get,         /* LifRxGoodFrames */
    ag_drv_lif_rx_agg_good_byte_get,        /* LifRxGoodBytes */
    ag_drv_lif_rx_agg_undersz_frm_get,      /* LifRxUndersizedFrames */
    ag_drv_lif_rx_agg_oversz_frm_get,       /* LifRxOversizeFrames */
    ag_drv_lif_rx_agg_crc8_frm_get,         /* LifRxCrc8ErrorsFrames */
    ag_drv_lif_rx_agg_fec_frm_get,          /* LifRxFecFrames */
    ag_drv_lif_rx_agg_fec_byte_get,         /* LifRxFecBytes */
    ag_drv_lif_rx_agg_fec_exc_err_frm_get,  /* LifRxFecExceedErrorsFrames */
    ag_drv_lif_rx_agg_nonfec_good_frm_get,  /* LifRxNonFecFrames */
    ag_drv_lif_rx_agg_nonfec_good_byte_get, /* LifRxNonFecBytes */
    ag_drv_lif_rx_agg_err_bytes_get,        /* LifRxFecErrorBytes */
    ag_drv_lif_rx_agg_err_zeroes_get,       /* LifRxFecErrorZeroes */
    ag_drv_lif_rx_agg_no_err_blks_get,      /* LifRxFecNoErrorBlocks */
    ag_drv_lif_rx_agg_cor_blks_get,         /* LifRxFecCorrBlocks */
    ag_drv_lif_rx_agg_uncor_blks_get,       /* LifRxFecUnCorrBlocks */
    ag_drv_lif_rx_agg_err_ones_get,         /* LifRxFecCorrOnes */
    ag_drv_lif_rx_agg_err_frm_get           /* LifRxErroredFrames */
    };

//##############################################################################
// Constant Definitions
//##############################################################################
#define PipeDelay           6
#define IpgCnt              2
#define PreambleLengthTq    4


//##############################################################################
// Macro Helper Definitions
//##############################################################################
// Build a 24bit key from 3 bytes.
#define Build24BitKey(a,b,c) (U32) (((a) << 16) | ((b) << 8) | (c))

//##############################################################################
// Static Variables
//##############################################################################
U8 laserOffTimeOffset;

typedef enum
    {
    LifDpRamSelDnStats  = 0,
    LifDpRamSelDnSecKey    = 1,
    LifDpRamSelDnFecData    = 2,
    LifDpRamSelUpSecKey    = 3,
    LifDpRamSelDnFecPartSyn  = 5,
    LifDpRamSelDnFecFullSyn  = 6,
    LifDpRamSelUpFecParity  = 7
    } LifDpRamSel;

#define LifDpRamRd 0
#define LifDpRamWr 1

static DEFINE_SPINLOCK(lif_spinlock);

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
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_data_port_command lif_port_cmd;

    do
        {
        drv_error += ag_drv_lif_data_port_command_get(&lif_port_cmd);
        }while(lif_port_cmd.data_port_busy && (drv_error == BDMF_ERR_OK));
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
void DpWrite (LifDpRamSel ramSel, U16 addr)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_data_port_command lif_port_cmd;

    memset(&lif_port_cmd, 0, sizeof(lif_data_port_command));
    lif_port_cmd.ram_select = ramSel;
    lif_port_cmd.data_port_op_code = LifDpRamWr;
    lif_port_cmd.data_port_addr = addr;
	
    // wait for dp to be ready
    DpWait ();
    drv_error += ag_drv_lif_data_port_command_set(&lif_port_cmd);
    // wait again to be sure command had been taken
    DpWait ();
    } // DpWrite

#define LifDpDataEnEncryption    (0x1<<9)
#define LifEncryptKeyIdxShift		8

#define LifDnEncryptionTeknovusMode 0
#define LifDnEncryptionReserved		1
#define LifDnEncryptionEponMode     2
#define LifDnEncryptionTripleChurnM  3
#define LifDnEncryptionZoh      4
#define LifDnEncryption8021AE      5
#define LifDnEncryptionDisable      6

#define LifUpEncryptionTeknovusMode 0
#define LifUpEncryptionZoh		1
#define LifUpEncryptionEponMode     2
#define LifUpEncryption8021AE      3
#define LifUpEncryptionDisable      0


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
void LifKeySet (EncryptMode mode,
                Direction direction,
                LinkIndex link,
                U8 keyIdx,
                U32 const BULK *key,
                const U32 BULK* sci,
                U8 tci,
                U32 pn)
    {
    lif_sec_control sec_ctl;
    bdmf_error_t drv_error = BDMF_ERR_OK;
    // Translation table
    U8 modeDnE2ITab[EncryptModeMax] = {
        LifDnEncryptionDisable,       // EncryptModeDisable
        LifDnEncryptionTeknovusMode,  // EncryptModeAes
        LifDnEncryptionZoh,       // EncryptModeZoh
        LifDnEncryptionTripleChurnM,  // EncryptModeTripleChurn
        LifDnEncryption8021AE};  // EncryptMode8021AE
    U8 modeUpE2ITab[EncryptModeMax] = {
        LifUpEncryptionDisable,       // EncryptModeDisable
        LifDnEncryptionTeknovusMode,  // EncryptModeAes
        LifDnEncryptionTeknovusMode,       // EncryptModeZoh
        LifUpEncryptionDisable,  // EncryptModeTripleChurn
        LifUpEncryption8021AE};  // EncryptMode8021AE
	
    spin_lock_bh(&lif_spinlock);
	
    // If mode is set to disable, we clear the entry in the table
    if (mode == EncryptModeDisable)
        {// disable encryption per llid, write 0 to port_data7[9].
        drv_error += ag_drv_lif_data_port_data_set(7, 0);
        }
    else
        {
		memset((void*)&sec_ctl, 0, sizeof(lif_sec_control));
        drv_error += ag_drv_lif_sec_control_get(&sec_ctl);
        // Set (or re-set) the global encryption scheme
        if (direction == Upstream)
            {
            sec_ctl.secenup = 1;
            sec_ctl.secupencryptscheme = modeUpE2ITab[mode];
            }
        else
            {
            sec_ctl.secendn = 1;
            sec_ctl.secdnencryptscheme = modeDnE2ITab[mode];
            }
        drv_error += ag_drv_lif_sec_control_set(&sec_ctl);
		
        if (mode == EncryptModeTripleChurn)
            {
            // For a 3Churned key, only 3 bytes are used: B0, B1 and B2
            // input key = [Bx] [B0] [B1] [B2]
            U8  b0 = (U8) (key[0] >> 16);
            U8  b1 = (U8) (key[0] >> 8);
            U8  b2 = (U8) (key[0] >> 0); 
			
            drv_error += ag_drv_lif_data_port_data_set(0, key[0]&0xFFFFFFUL);
            drv_error += ag_drv_lif_data_port_data_set(1, Build24BitKey(b2, b0, b1));
            drv_error += ag_drv_lif_data_port_data_set(2, Build24BitKey(b1, b2, b0));
            drv_error += ag_drv_lif_data_port_data_set(3, key[0]&0xFFFFUL);
            }
        else 
            {
            U32 port7Input = 0;
            // ... that means in reverse order.
            drv_error += ag_drv_lif_data_port_data_set(0, key[3]);
            drv_error += ag_drv_lif_data_port_data_set(1, key[2]);
            drv_error += ag_drv_lif_data_port_data_set(2, key[1]);
            drv_error += ag_drv_lif_data_port_data_set(3, key[0]);

            if (((mode == EncryptModeAes) || mode == EncryptModeZoh) || (mode == EncryptMode8021AE))
                {//input SCI
                drv_error += ag_drv_lif_data_port_data_set(4, sci[1]);
                drv_error += ag_drv_lif_data_port_data_set(5, sci[0]);
                }

            if (mode == EncryptMode8021AE)
                {
                drv_error += ag_drv_lif_data_port_data_set(6, pn);
                }
		
			if (direction == Upstream)
				{
				port7Input |= ((mode == EncryptMode8021AE) ? tci : 0) | 
					LifDpDataEnEncryption | 
					(keyIdx << LifEncryptKeyIdxShift);
				}
			else
			    {
				port7Input = LifDpDataEnEncryption;
				}
	        drv_error += ag_drv_lif_data_port_data_set(7, port7Input);
    		}
        }

    // Send the command to data port
    if (direction == Upstream)
        {
        DpWrite (LifDpRamSelUpSecKey, link);
        }
    else
        {
        DpWrite (LifDpRamSelDnSecKey, (link << 1) | keyIdx);
        }

    spin_unlock_bh(&lif_spinlock);
    } // LifKeySet


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
BOOL LifEncryptEnGet(LinkIndex link, Direction dir)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    bdmf_boolean sec_stat;

    if (dir == Upstream)
    	{
        drv_error += ag_drv_lif_sec_up_encrypt_stat_get(link, &sec_stat);
    	}
    else
    	{
        drv_error += ag_drv_lif_dn_encrypt_stat_get(link, &sec_stat);
    	}
    
    return (BOOL)sec_stat;
    }


////////////////////////////////////////////////////////////////////////////////
/// LifKeyInUse - Get the current security key for a link
///
/// This function gets the active key for a given link.  It is used to detect a
/// key switch over.
///
 // Parameters:
/// \param  link Link index to check
///
/// \return
/// Active security key
////////////////////////////////////////////////////////////////////////////////
//extern
U8 LifKeyInUse (LinkIndex link, Direction dir)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    bdmf_boolean key_in_use;

    if (dir == Upstream)
    	{
        drv_error += ag_drv_lif_sec_up_key_stat_get(link, &key_in_use);
    	}
    else
    	{
        drv_error += ag_drv_lif_sec_key_sel_get(link, &key_in_use);
    	}
    
    return (U8)key_in_use;
    } // LifKeyInUse


////////////////////////////////////////////////////////////////////////////////
/// LifEncryptUpDisable - Disable upstream encryption global configuration
///
 // Parameters:
/// \none
///
/// \return
/// none
////////////////////////////////////////////////////////////////////////////////
//extern
void LifEncryptUpDisable (void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_sec_control sec_ctl;
	
	drv_error += ag_drv_lif_sec_control_get(&sec_ctl);
	sec_ctl.secenup = 0;
	sec_ctl.secupencryptscheme = LifUpEncryptionDisable;
	drv_error += ag_drv_lif_sec_control_set(&sec_ctl);
    } // LifEncryptUpDisable




////////////////////////////////////////////////////////////////////////////////
/// \brief Reads LIF statistic
///
/// \param statistic to read
///
/// \return
/// statistic value
////////////////////////////////////////////////////////////////////////////////
//extern
U32 LifReadStat (LifStatId statId)
    {
    U32 stats = 0;
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error += lif_stat_get[statId](&stats);

    return stats;
    } // LifReadStat


////////////////////////////////////////////////////////////////////////////////
//extern
void LifFecTxSet(LinkMap linkMap)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    U8 i = 0;

    for (i = 0; i < LifLlidCount; i++)
        {
        drv_error += ag_drv_lif_fec_per_llid_set(i, (linkMap >> i)&0x1);
        }
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Set FEC Rx
///
/// \param Fec Rx mode
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void LifFecRxSet (BOOL mode)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_fec_control fec_control;

    drv_error += ag_drv_lif_fec_control_get(&fec_control);
    
    if (mode)
        {
        fec_control.cffecrxenable = TRUE;
        }
    else
        {
        fec_control.cffecrxenable = FALSE;
        }

    drv_error += ag_drv_lif_fec_control_set(&fec_control);
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief Sets idle time for LIF
///
/// \param  front   Time to idle before burst,
///                 measured in 62.5M clocks (16 bit times)
/// \param  back    Time to idle after burst, 62.5M clocks
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void LifSetIdleTime (U16 front, U16 back)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    uint16_t cftxinitidle;
    uint8_t cftxlaseroffdelta;
    uint16_t cftxfecinitidle;
    uint32_t secupmpcpoffset;

    cftxinitidle = front;
    cftxlaseroffdelta = (back - laserOffTimeOffset) | LifLaserOffIdleBefore;
    drv_error += ag_drv_lif_laser_off_idle_set(cftxinitidle, cftxlaseroffdelta);
    
    //Reg 0x138 must be set for FEC transmission;
    //If it is zero, the OLT will not be able to lock onto the upstream burst.
    cftxfecinitidle = front;
    drv_error += ag_drv_lif_fec_init_idle_set(cftxfecinitidle);

    secupmpcpoffset = front + PreambleLengthTq;
    drv_error += ag_drv_lif_sec_up_mpcp_offset_set(secupmpcpoffset);
    } // LifSetIdleTime


////////////////////////////////////////////////////////////////////////////////
/// \brief  Disable a link form the lookup table
///
/// Logical links are numbered from 0;
///
/// \param link     Logical link number
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void LifDisableLink (LinkIndex link)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    uint32_t cfgllid;

    drv_error += ag_drv_lif_llid_get(link, &cfgllid);
    cfgllid &= (~LifLlidEn);
    drv_error += ag_drv_lif_llid_set(link, cfgllid);
    }


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
void LifEnableLink (LinkIndex link)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    uint32_t cfgllid;

    drv_error += ag_drv_lif_llid_get(link, &cfgllid);
    cfgllid |= LifLlidEn;
    drv_error += ag_drv_lif_llid_set(link, cfgllid);
    } // LifEnableLink

////////////////////////////////////////////////////////////////////////////////
/// \brief delete physical LLID value associated with a link
///
/// Logical links are numbered from 0; the packet preamble has a 16-bit
///
/// \param link     Logical link number
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void LifDeleteLink (LinkIndex link)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error += ag_drv_lif_llid_set(link, 0);
    } // LifDeleteLink


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
void LifCreateLink (LinkIndex link, PhyLlid phy)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    uint32_t cfgllid;

    cfgllid = (U32)phy | LifLlidEn;

    drv_error += ag_drv_lif_llid_set(link, cfgllid);
    } // LifCreateLink


////////////////////////////////////////////////////////////////////////////////
/// \brief Return a PhyLlid associated to a link index
///
/// \param link     Logical link number
///
/// \return Physical Llid
////////////////////////////////////////////////////////////////////////////////
//extern
PhyLlid LifGetPhyLlid (LinkIndex link)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    uint32_t cfgllid;

    drv_error += ag_drv_lif_llid_get(link, &cfgllid);

    return (PhyLlid)(cfgllid & LifLlidLkup0Msk);
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Return a link index associated to a phy llid
///
/// \param phy     Phy LLID
///
/// \return Link index
////////////////////////////////////////////////////////////////////////////////
//extern
LinkIndex LifPhyLlidToIndex (PhyLlid phy)
    {
    LinkIndex  result;

    result = 0;
    while ((result < LifLlidCount) &&
           (LifGetPhyLlid(result) != phy))
        {
        ++result;
        }

    return result;
    } // LifPhyLlidToIndex


void LifLaserTxModeSet (rdpa_epon_laser_tx_mode mode)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_pon_control pon_control;

    drv_error += ag_drv_lif_pon_control_get(&pon_control);
    // This is implemented as a multiple ifs instead of a switch to save code.
    if (mode == rdpa_epon_laser_tx_off)
        {
        pon_control.cflaseren = FALSE;
        }
    else
        {
        pon_control.cflaseren = TRUE;
        if(EponGetMacMode() == EPON_MAC_AE_MODE)
            pon_control.cftxlaseron = TRUE;
        else
            pon_control.cftxlaseron = (mode == rdpa_epon_laser_tx_continuous)?TRUE:FALSE;
        }

    drv_error += ag_drv_lif_pon_control_set(&pon_control);
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Does the LIF have code word lock?
///
/// \return
/// TRUE if LIF has code word lock, FALSE otherwise
////////////////////////////////////////////////////////////////////////////////
//extern
BOOL LifLocked(void)
    {
    BOOL  lock;
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_int_status int_status;

    drv_error += ag_drv_lif_int_status_get(&int_status);

    lock = !int_status.intrxoutofsynchstat;
    // we are locked if the loss interrupt has not triggered

    // clear the loss interrupt after reading to prepare for next check
    int_status.intrxoutofsynchstat = TRUE;

    drv_error += ag_drv_lif_int_status_set(&int_status);

    return lock;
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Disable LIF RX
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void LifRxDisable(void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_pon_control pon_ctrl;
    lif_sec_control sec_ctrl;

    drv_error += ag_drv_lif_pon_control_get(&pon_ctrl);
    pon_ctrl.lifrxrstn_pre = FALSE;
    pon_ctrl.lifrxen = FALSE;
    drv_error += ag_drv_lif_pon_control_set(&pon_ctrl);

    drv_error += ag_drv_lif_sec_control_get(&sec_ctrl);
    sec_ctrl.secdnrstn_pre = FALSE;
    drv_error += ag_drv_lif_sec_control_set(&sec_ctrl);
    }


////////////////////////////////////////////////////////////////////////////////
/// \brief  Enable LIF RX
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void LifRxEnable(void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_pon_control pon_ctrl;
    lif_sec_control sec_ctrl;

    drv_error += ag_drv_lif_pon_control_get(&pon_ctrl);
    pon_ctrl.lifrxrstn_pre = TRUE;
    pon_ctrl.lifrxen = TRUE;
    drv_error += ag_drv_lif_pon_control_set(&pon_ctrl);

    drv_error += ag_drv_lif_sec_control_get(&sec_ctrl);
    sec_ctrl.secdnrstn_pre = TRUE;
    drv_error += ag_drv_lif_sec_control_set(&sec_ctrl);
    }

////////////////////////////////////////////////////////////////////////////////
/// \brief  Setup TX-to-RX loopback
///
///
/// \return
/// None
////////////////////////////////////////////////////////////////////////////////
//extern
void LifTxToRxLoopback (void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_pon_control pon_ctrl;
    lif_sec_control sec_ctrl;

    drv_error += ag_drv_lif_pon_control_get(&pon_ctrl);
    pon_ctrl.cftx2rxlpback = TRUE;
    drv_error += ag_drv_lif_pon_control_set(&pon_ctrl);

    drv_error += ag_drv_lif_sec_control_get(&sec_ctrl);
    sec_ctrl.secdnrstn_pre = TRUE;
    sec_ctrl.secuprstn_pre = TRUE;
    drv_error += ag_drv_lif_sec_control_set(&sec_ctrl);
    }


////////////////////////////////////////////////////////////////////////////////
//extern
void LifTransportTimeSet(U32 time)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;

    drv_error += ag_drv_lif_tp_time_set((uint32_t)time);
    }


////////////////////////////////////////////////////////////////////////////////
//extern
U32 LifMpcpTimeGet(void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    uint32_t ltmpcptime;

    drv_error += ag_drv_lif_mpcp_time_get(&ltmpcptime);

    return ltmpcptime;
    }


////////////////////////////////////////////////////////////////////////////////
//extern
void LifLaserMonSet(BOOL enable, U32 threshold)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_int_status int_status;
    lif_int_mask int_mask;
    bdmf_boolean iopbilaserens1a;
    bdmf_boolean cfglsrmonacthi;
    bdmf_boolean pbilasermonrsta_n_pre;
    int rc ;
    int bus = -1;
    TRX_SIG_ACTIVE_POLARITY  tx_sd_polarity;

    if (enable)
        {
        drv_error += ag_drv_lif_int_mask_get(&int_mask);
        int_mask.laseroffmask = TRUE;
        int_mask.laseronmaxmask = TRUE;
        drv_error += ag_drv_lif_int_mask_set(&int_mask);

        drv_error += ag_drv_lif_lsr_mon_a_max_thr_set(threshold);
        }

    drv_error += ag_drv_lif_lsr_mon_a_ctrl_get(&iopbilaserens1a, &cfglsrmonacthi, &pbilasermonrsta_n_pre);
    pbilasermonrsta_n_pre = enable;

    opticaldet_get_xpon_i2c_bus_num(&bus);
    rc = trx_get_tx_sd_polarity(bus, &tx_sd_polarity) ;
    if (rc == 0)
        cfglsrmonacthi = (tx_sd_polarity == TRX_ACTIVE_HIGH)? TRUE : FALSE;
    
    drv_error += ag_drv_lif_lsr_mon_a_ctrl_set(iopbilaserens1a, cfglsrmonacthi, pbilasermonrsta_n_pre);

    if (!enable)
        {
        drv_error += ag_drv_lif_int_status_get(&int_status);
        int_status.laseroff = TRUE;
        int_status.laseronmax = TRUE;
        drv_error += ag_drv_lif_int_status_set(&int_status);
        }
    }


////////////////////////////////////////////////////////////////////////////////
//extern
U32 LifLaserMonLaserOnMaxIntGet(void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_int_status int_status;
    lif_int_mask int_mask;

    drv_error += ag_drv_lif_int_status_get(&int_status);
    drv_error += ag_drv_lif_int_mask_get(&int_mask);

    if (int_status.laseronmax && int_mask.laseronmaxmask)
        {
        return 1;
        }

    return 0;
    }


////////////////////////////////////////////////////////////////////////////////
//extern
void LifLaserMonClrLaserOnMaxInt(void)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_int_status int_status;

    int_status.laseronmax = TRUE;
    drv_error += ag_drv_lif_int_status_set(&int_status);
    }


////////////////////////////////////////////////////////////////////////////////
//extern
void LifLbePolaritySet(Polarity polarity)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_pon_control lif_pon_ctrl = {0};
    drv_error += ag_drv_lif_pon_control_get(&lif_pon_ctrl);
    if(polarity == ActiveHi)
        {
        lif_pon_ctrl.cftxlaseronacthi = 1;
        }
    else
        {
        lif_pon_ctrl.cftxlaseronacthi = 0;
        }
    drv_error += ag_drv_lif_pon_control_set(&lif_pon_ctrl);
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
void LifInit (U8 txOffTimeOffset, BOOL txOffIdle,
              Polarity polarity, LaserRate upRate, LaserRate dnRate)
    {
    bdmf_error_t drv_error = BDMF_ERR_OK;
    lif_pon_control lif_pon_ctrl = {0};
    lif_pon_inter_op_control pon_iop_ctrl = {0};
    lif_int_status int_status = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    lif_fec_control fec_ctrl = {0};
    lif_sec_control sec_ctrl = {0};

    EponTopClearReset(EponTopLifR,FALSE);
    mdelay(20);
    //release lif
    EponTopClearReset(EponTopLifR,TRUE);
    mdelay(20);
	
    // Store laser time offset. Needed later when setting up Idle Time.
    laserOffTimeOffset = txOffTimeOffset;
    
    lif_pon_ctrl.cfgrxdatabitflip = 1;
    if(polarity == ActiveHi)
        {
        lif_pon_ctrl.cftxlaseronacthi = 1;
        }

    // fix from fiberhome 1pps issue
    lif_pon_ctrl.cfppsclkrbc = 1;
    
    lif_pon_ctrl.cftxlaseron = 0;
    lif_pon_ctrl.cflaseren = 0;
    if(EponGetMacMode() == EPON_MAC_AE_MODE)
        {
        lif_pon_ctrl.cfp2pmode = 1;
        lif_pon_ctrl.cfp2pshortpre = 1;
        }
    drv_error += ag_drv_lif_pon_control_set(&lif_pon_ctrl);

    if(EponGetMacMode() == EPON_MAC_AE_MODE)
        pon_iop_ctrl.cfgllidpromiscuousmode = 1;
    pon_iop_ctrl.cftxpipedelay = PipeDelay;
    pon_iop_ctrl.cftxipgcnt = IpgCnt;
    drv_error += ag_drv_lif_pon_inter_op_control_set(&pon_iop_ctrl);
  
    fec_ctrl.cffecrxenable = TRUE;
    fec_ctrl.cffectxenable = TRUE;
    fec_ctrl.cffectxfecperllid = TRUE;

    drv_error += ag_drv_lif_fec_control_set(&fec_ctrl);

    // Security Controls
    drv_error += ag_drv_lif_sec_control_get(&sec_ctrl);
    sec_ctrl.secuprstn_pre = TRUE;
    sec_ctrl.secdnrstn_pre = TRUE;
    drv_error += ag_drv_lif_sec_control_set(&sec_ctrl);


    lif_pon_ctrl.liftxrstn_pre = 1;
    lif_pon_ctrl.liftxen = 1;
    drv_error += ag_drv_lif_pon_control_set(&lif_pon_ctrl);

    drv_error += ag_drv_lif_int_status_set(&int_status);

    if ((dnRate != LaserRate10G) && (EponGetMacMode() != EPON_MAC_AE_MODE))
        {
        LifRxEnable();
        }
    }

// End of Lif.c
