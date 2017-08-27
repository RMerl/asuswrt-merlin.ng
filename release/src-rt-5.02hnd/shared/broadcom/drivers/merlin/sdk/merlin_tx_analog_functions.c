 /*
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>

*/

/** @file merlin_tx_analog_functions.c
 * Functions to configure Merlin TX
 */

/*
  Precursor (c-1) range    (0..10)/60  (0..10)/60
  Main (c0) range          (1..60)/60  (1..60)/60
  Postcursor1 (c+1) range  (0..18)/60  (0..23)/60
  Postcursor2 (c+2) range  (0..5)/60    0

  V2 limit (DC level)      (main > sum of others => v2 limit)
  Enforced by User         1/60  1/60
  Sum of all 4 taps        (main + sum of others <= max limit)
  Enforced by User         60/60  60/60
  Sum of pre+post1         (pre+post1<=max limit)
  Enforced by User         22/60  27/60
*/


err_code_t merlin_mptwo_validate_txfir_cfg(merlin_access_t *ma, int8_t pre, int8_t main, int8_t post1, int8_t post2) {

  err_code_t failcode = ERR_CODE_NONE;
  uint8_t post2to1 = (post2==0);

  if ((pre>10) || (pre < 0))
    failcode = failcode | ERR_CODE_TXFIR_PRE_INVALID;

  if ((main>60) || (main < 0))
    failcode = failcode | ERR_CODE_TXFIR_MAIN_INVALID;

  if ((!post2to1 && (post1>18)) || (post1 < 0))
    failcode = failcode | ERR_CODE_TXFIR_POST1_INVALID;
  if ((post2to1 && (post1>23)) || (post1 < 0))
    failcode = failcode | ERR_CODE_TXFIR_POST1_INVALID;

  if (post2>5)
    failcode = failcode | ERR_CODE_TXFIR_POST2_INVALID;

  if (main < (pre + post1 + post2 + 1))
    failcode = failcode | ERR_CODE_TXFIR_V2_LIMIT;

  if ((pre + main + post1 + post2) > 60)
    failcode = failcode | ERR_CODE_TXFIR_SUM_LIMIT;

  if (!post2to1 && ((pre + post1) > 22))
    failcode = failcode | ERR_CODE_TXFIR_PRE_POST1_SUM_LIMIT;
  if (post2to1 && ((pre + post1) > 27))
    failcode = failcode | ERR_CODE_TXFIR_PRE_POST1_SUM_LIMIT;

  return (_error(failcode));
}


err_code_t merlin_mptwo_apply_txfir_cfg(merlin_access_t *ma, int8_t pre, int8_t main, int8_t post1, int8_t post2) {

  err_code_t failcode = merlin_mptwo_validate_txfir_cfg(ma,pre, main, post1, post2);

  if (!failcode) {
    EFUN(wr_ams_tx_post2to1(post2==0));     /* Convert post2 slices to post1, if post2 is 0 */
    EFUN(wr_ams_tx_en_pre(pre>0));          /* Enable the TXFIR blocks */
    EFUN(wr_ams_tx_en_post1(post1>0));
    EFUN(wr_ams_tx_en_post2(post2>0));
    EFUN(wr_txfir_pre_override((uint8_t)pre));
    EFUN(wr_txfir_main_override((uint8_t)main));
    EFUN(wr_txfir_post_override((uint8_t)post1));
    EFUN(wr_txfir_post2((uint8_t)post2));

    if ((pre+main+post1+post2) > 45) {
      EFUN(wr_ams_tx_refcalshunt(5));
    } else {
      EFUN(wr_ams_tx_refcalshunt(4));
    }
  }

  return (_error(failcode));
}


err_code_t merlin_mptwo_read_tx_afe(merlin_access_t *ma, enum srds_tx_afe_settings_enum param, int8_t *val) {
    if(!val) {
        return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    switch(param) {
    case TX_AFE_PRE:
        EFUN(_get_tx_pre(ma,val));
        break;
    case TX_AFE_MAIN:
        EFUN(_get_tx_main(ma,val));
        break;
    case TX_AFE_POST1:
        EFUN(_get_tx_post1(ma,val));
        break;
    case TX_AFE_POST2:
        EFUN(_get_tx_post2(ma,val));
        break;
    default:
        return(_error(ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
    }

    return(ERR_CODE_NONE);
}


err_code_t merlin_mptwo_config_tx_hpf(merlin_access_t *ma, uint8_t val) {

  switch (val) {
    case 0:
      EFUN(wr_ams_tx_en_hpf(0));
      break;
    case 1:
      EFUN(wr_ams_tx_en_hpf(1));
      break;
    case 2:
      EFUN(wr_ams_tx_en_hpf(3));
      break;
    case 3:
      EFUN(wr_ams_tx_en_hpf(7));
      break;
    case 4:
      EFUN(wr_ams_tx_en_hpf(15));
      break;
    default:
      return(ERR_CODE_TX_HPF_INVALID);
      break;
  }

  return(ERR_CODE_NONE);
}


err_code_t merlin_mptwo_rd_tx_hpf_config(merlin_access_t *ma, uint8_t *pval) {
    uint8_t hpf_cfg=0,sum=0;

    if(!pval) {
        return(ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
    }

    ESTM(hpf_cfg = rd_ams_tx_en_hpf());

    sum = ((hpf_cfg & 0x1)) +
          ((hpf_cfg & 0x2)>>1) +
          ((hpf_cfg & 0x4)>>2) +
          ((hpf_cfg & 0x8)>>3);

    switch (sum) {
    case 0:
        *pval = 0;
        break;
    case 1:
        *pval = 1;
        break;
    case 2:
        *pval = 2;
        break;
    case 3:
        *pval = 3;
        break;
    case 4:
        *pval = 4;
        break;
    default:
        return(ERR_CODE_TX_HPF_INVALID);
        break;
    }

    return(ERR_CODE_NONE);
}
