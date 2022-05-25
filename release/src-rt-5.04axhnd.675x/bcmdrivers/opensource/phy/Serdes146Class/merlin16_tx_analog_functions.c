/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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
                          |-------------|--------------|  
                          |  post2to1=0 |  post2to1=1  |
                          |-------------|--------------|  
  Precursor   (c-1) range | (0..10)/60  |  (0..10)/60  |
  Maincursor  (c0)  range | (1..60)/60  |  (1..60)/60  |
  Postcursor1 (c+1) range | (0..18)/60  |  (0..24)/60  |
  Postcursor2 (c+2) range | (0..6)/60   |      0       |   
                          |             |              |
  V2 limit (DC level) => (main - sum of others >= v2 limit)
    Enforced by User      |   1/60      |     1/60     |
  Sum of all 4 taps   => (main + sum of others <= max limit)
    Enforced by User      |   60/60     |    60/60     |
  Sum of pre+post1    => (pre + post1 <= max limit)    |
    Enforced by User      |   18/60     |    24/60     |
                          |-------------|--------------|  
*/


#include "merlin16_tx_analog_functions.h"
#include "merlin16_shortfin_config.h"
#include "merlin16_shortfin_common.h"
#include "merlin16_shortfin_functions.h"
#include "merlin16_shortfin_internal.h"
#include "merlin16_shortfin_internal_error.h"
#include "merlin16_shortfin_select_defns.h"

err_code_t merlin16_shortfin_validate_txfir_cfg(srds_access_t *sa__, int8_t pre, int8_t main, int8_t post1, int8_t post2) {

  err_code_t failcode = ERR_CODE_NONE;
  uint8_t post2to1 = (post2==0);

  if ((pre > TXFIR_PRE_MAX) || (pre < TXFIR_PRE_MIN))                           
    failcode = failcode | ERR_CODE_TXFIR_PRE_INVALID;

  if ((main > TXFIR_MAIN_MAX) || (main < TXFIR_MAIN_MIN))
    failcode = failcode | ERR_CODE_TXFIR_MAIN_INVALID;

  if ((!post2to1 && (post1 > (TXFIR_POST1_MAX - TXFIR_POST2_MAX))) || (post1 < TXFIR_POST1_MIN))
    failcode = failcode | ERR_CODE_TXFIR_POST1_INVALID;
  if ((post2to1 && (post1 > TXFIR_POST1_MAX)) || (post1 < TXFIR_POST1_MIN))
    failcode = failcode | ERR_CODE_TXFIR_POST1_INVALID;

  if ((post2 > TXFIR_POST2_MAX) || (post2 < TXFIR_POST2_MIN))
    failcode = failcode | ERR_CODE_TXFIR_POST2_INVALID;

  if (main < (pre + post1 + post2 + TXFIR_V2_LIMIT))
    failcode = failcode | ERR_CODE_TXFIR_V2_LIMIT;

  if ((pre + main + post1 + post2) > TXFIR_SUM_LIMIT)
    failcode = failcode | ERR_CODE_TXFIR_SUM_LIMIT;

  if (!post2to1 && ((pre + post1) > (TXFIR_PRE_POST_SUM_LIMIT - TXFIR_POST2_MAX)))
    failcode = failcode | ERR_CODE_TXFIR_PRE_POST1_SUM_LIMIT;
  if (post2to1 && ((pre + post1) > TXFIR_PRE_POST_SUM_LIMIT))
    failcode = failcode | ERR_CODE_TXFIR_PRE_POST1_SUM_LIMIT;

  return (merlin16_shortfin_error(sa__, failcode));
}

err_code_t _txfir_tap_mapping(srds_access_t *sa__, int8_t pre, int8_t main, int8_t post1, int8_t post2) {

  uint8_t  calc_post1, calc_post2, dc_level;
  struct p1sr_reg_st  p1sr;
  struct p2sr_reg_st  p2sr;
  struct mssr_reg_st  mssr;
  struct presr_reg_st presr;
  uint8_t post2to1 = (post2==0);

  /* Intialize all slicer variables */
  ENULL_MEMSET(&p1sr, 0, sizeof(p1sr));
  ENULL_MEMSET(&p2sr, 0, sizeof(p2sr));
  ENULL_MEMSET(&mssr, 0, sizeof(mssr));
  ENULL_MEMSET(&presr, 0, sizeof(presr));

  /* Calculate post1 and post2 based on post2to1 setting and post1 value */
  if (post2to1) {
    if (post1>18) {  /* If post1 requires post2, then assign max to post2 and then assign remaining to post1 */
      calc_post2 = 6;
      calc_post1 = (uint8_t)(post1 - 6);
    }
    else {           /* If post1 does not require post2, assign everything to post1 and nothing to post2 */
      calc_post1 = (uint8_t)post1;
      calc_post2 = 0;
    } 
  } 
  else {             /* If post2 is non-zero, then post2 cannot be used for post1 */ 
    calc_post1 = (uint8_t)post1;
    calc_post2 = (uint8_t)post2;
  }
    
  /* Calculate dc_level by sustracting pre, post1, post2 from main */
  dc_level = (uint8_t)(main - pre - calc_post1 - calc_post2);

  /* 
  Post2 Allocation Algorithm (0 to 6)
    1) Use the first 1x slice (LSB) for all odd values and maximum value (post2 = 6)
    2) Use the 2x slices for as many are enabled starting from LSB
    3) Use the last 1x slice (MSB) only for post2 = 6 
  */
  if (((calc_post2 % 2) == 1) || (calc_post2 >= 6)) {
    p2sr.field.post2_1x_0 = 1;
  }
  if (calc_post2 >= 2) {
    p2sr.field.post2_2x_0 = 1;
  } 
  if (calc_post2 >= 4) {
    p2sr.field.post2_2x_1 = 1;
  } 
  if (calc_post2 >= 6) {
    p2sr.field.post2_1x_1 = 1;
  }

  /*
  Pre Allocation Algorithm (0 to 10)
    1) Use the first shared 1x slice (starting from MSB) for all odd values and maximum value (pre = 10)
    2) Use the shared 2x slices for as many are enabled starting from MSB
    3) Use the last 1x slice (LSB) only for pre = 10         
  */ 
  if (((pre % 2) == 1) || (pre >= 10)) {
    presr.field.pre_post1pre_1x_1 = 1;
  }
  if (pre >= 2) {
    presr.field.pre_post1pre_2x_3 = 1;
  }
  if (pre >= 4) {
    presr.field.pre_post1pre_2x_2 = 1;
  }
  if (pre >= 6) {
    presr.field.pre_post1pre_2x_1 = 1;
  }
  if (pre >= 8) {
    presr.field.pre_post1pre_2x_0 = 1;
  }
  if (pre >= 10) {
    p1sr.field.pre_post1pre_1x_0 = 1;
  }

  /*
  Post1 Allocation Algorithm (0 to 18)
  1) Use the first dedicated 1x slice (LSB) when 
     a. post1 =  1, 3, 5, 7 (i.e. post 1 odd values up to 7)  
     b. post1 >= 8  
  2) Use the dedicated 2x slices starting from LSB
  3) Use the last dedicated 1x slice (MSB) when post1 >=8 
  4) Use the first shared 1x slice (LSB) when
     a. post1 = 9, 11, 13, 15, 17 (odd values starting at 9
     b. post1 = 18
     c. when shared 2x slice is already used by pre (i.e. then use two shared 1x slices) 
  5) Use the shared 2x slices starting from LSB
  6) Use the shared 1x slice (MSB) when
     a. post1 = 18
     b. when shared 2x slice is already used by pre (i.e. then use two shared 1x slices)
  */
  if (((calc_post1 % 2) == 1) || (calc_post1 >= 8)) {
    p1sr.field.post1_1x_0 = 1;
  }
  if (calc_post1 >= 2) {
    p1sr.field.post1_2x_0 = 1;
  } 
  if (calc_post1 >= 4) {
    p1sr.field.post1_2x_1 = 1;
  } 
  if (calc_post1 >= 6) {
    p1sr.field.post1_2x_2 = 1;
  }
  if (calc_post1 >= 8) {
    p1sr.field.post1_1x_1 = 1;
  }
  if ((p1sr.field.pre_post1pre_1x_0 == 0) && (
                                          (((calc_post1 % 2) == 1) && (calc_post1 >= 8)) ||
                                          (calc_post1 >= 18) ||
                                          ((presr.field.pre_post1pre_2x_0 == 1) && (calc_post1 >= 10)) ||
                                          ((presr.field.pre_post1pre_2x_1 == 1) && (calc_post1 >= 12)) ||
                                          ((presr.field.pre_post1pre_2x_2 == 1) && (calc_post1 >= 14)) ||
                                          ((presr.field.pre_post1pre_2x_3 == 1) && (calc_post1 >= 16)))) {
    
    p1sr.field.post1_post1pre_1x_0 = 1;
  }
  if ((presr.field.pre_post1pre_2x_0 == 0) && (calc_post1 >= 10)) {
    presr.field.post1_post1pre_2x_0 = 1;
  }  
  if ((presr.field.pre_post1pre_2x_1 == 0) && (calc_post1 >= 12)) {
    presr.field.post1_post1pre_2x_1 = 1;
  }
  if ((presr.field.pre_post1pre_2x_2 == 0) && (calc_post1 >= 14)) {
    presr.field.post1_post1pre_2x_2 = 1;
  }
  if ((presr.field.pre_post1pre_2x_3 == 0) && (calc_post1 >= 16)) {
    presr.field.post1_post1pre_2x_3 = 1;
  }
  if ((presr.field.pre_post1pre_1x_1 == 0) && (
                                           (calc_post1 >= 18) ||
                                           ((presr.field.pre_post1pre_2x_0 == 1) && (calc_post1 >= 10)) ||
                                           ((presr.field.pre_post1pre_2x_1 == 1) && (calc_post1 >= 12)) ||
                                           ((presr.field.pre_post1pre_2x_2 == 1) && (calc_post1 >= 14)) ||
                                           ((presr.field.pre_post1pre_2x_3 == 1) && (calc_post1 >= 16)))) {
    
    presr.field.post1_post1pre_1x_1 = 1;
  }
  

  /*
  Main/DC_level Allocation Algorithm ...
   - Main  Dedicated : Two   2x slices, two 1x slices
   - Post2 Dedicated : Two   2x slices, two 1x slices
   - Post1 Dedicated : Three 2x slices, two 1x slices
   - Post1/Pre Shared: Four  2x slices, two 1x slices

   Use slices in the following order:
     1) Use the main 1x slice for all odd values and even values >= 6   
     2) Use the main 2x slices for as many are enabled starting from LSB
     3) Use the last main 1x slice (MSB) for all even values >= 6 and all values 
        after all of the dedicated and shared 2x slices been used
     4) Use Dedicated post_2X, post1_2x slices
     5) Use Shared post1pre_2X slices
     6) Use two Dedicated main_1X[1], post2_1x[1:0], post1_1x[1:0] - to fulfill 2X request
     4) Use Dedicated main_1X[1], post2_1x[1:0], post1_1x and Shared post1pre_1X[1:0] - to fulfill 2X request
     
     Use the slices in Post2, Post1 (MSB to LSB) order
  */
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }
  else { /* One dc slice is always enabled and hence subtraction */
    dc_level--;
  }
  
  /* 0.5 slice */
  if ((dc_level % 2) == 1) {
    mssr.field.dc_level_0p5x = 1; 
  }
  dc_level = (dc_level >> 1);
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }
  else if ((dc_level % 2) == 1) { /* odd value */
    mssr.field.dc_level_main_1x = 1;
    dc_level--;
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }
  else if (mssr.field.dc_level_main_2x_0 == 0) { 
    mssr.field.dc_level_main_2x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }      
  else if (mssr.field.dc_level_main_2x_1 == 0) {
    mssr.field.dc_level_main_2x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((mssr.field.dc_level_main_1x == 0) && (p2sr.field.dc_level_post2_1x_1 == 0) && (p2sr.field.post2_1x_1 == 0)) { 
    mssr.field.dc_level_main_1x = 1;
    p2sr.field.dc_level_post2_1x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }    
  else if ((mssr.field.dc_level_main_1x == 0) && (p2sr.field.dc_level_post2_1x_0 == 0) && (p2sr.field.post2_1x_0 == 0)) { 
    mssr.field.dc_level_main_1x = 1;
    p2sr.field.dc_level_post2_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((mssr.field.dc_level_main_1x == 0) && (p1sr.field.dc_level_post1_1x_1 == 0) && (p1sr.field.post1_1x_1 == 0)) {
    mssr.field.dc_level_main_1x = 1;
    p1sr.field.dc_level_post1_1x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
   
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((mssr.field.dc_level_main_1x == 0) && (p1sr.field.dc_level_post1_1x_0 == 0) && (p1sr.field.post1_1x_0 == 0)) {
    mssr.field.dc_level_main_1x = 1;
    p1sr.field.dc_level_post1_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
    
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((mssr.field.dc_level_main_1x == 0)  && (presr.field.dc_level_post1pre_1x_1 == 0) && 
           (presr.field.pre_post1pre_1x_1 == 0) && (presr.field.post1_post1pre_1x_1 == 0)) {
    mssr.field.dc_level_main_1x = 1;
    presr.field.dc_level_post1pre_1x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((mssr.field.dc_level_main_1x == 0)  && (p1sr.field.dc_level_post1pre_1x_0 == 0)  && 
           (p1sr.field.pre_post1pre_1x_0 == 0) && (p1sr.field.post1_post1pre_1x_0 == 0)) { 
    mssr.field.dc_level_main_1x = 1;
    p1sr.field.dc_level_post1pre_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p2sr.field.dc_level_post2_2x_1 == 0) && (p2sr.field.post2_2x_1 == 0)) {
    p2sr.field.dc_level_post2_2x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p2sr.field.dc_level_post2_2x_0 == 0) && (p2sr.field.post2_2x_0 == 0)) {
    p2sr.field.dc_level_post2_2x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p1sr.field.dc_level_post1_2x_2 == 0) && (p1sr.field.post1_2x_2 == 0)) {
    p1sr.field.dc_level_post1_2x_2 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p1sr.field.dc_level_post1_2x_1 == 0) && (p1sr.field.post1_2x_1 == 0)) {
    p1sr.field.dc_level_post1_2x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p1sr.field.dc_level_post1_2x_0 == 0) && (p1sr.field.post1_2x_0 == 0)) {
    p1sr.field.dc_level_post1_2x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((presr.field.dc_level_post1pre_2x_3 == 0) && (presr.field.pre_post1pre_2x_3 == 0) &&
           (presr.field.post1_post1pre_2x_3 == 0)) {
    presr.field.dc_level_post1pre_2x_3 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((presr.field.dc_level_post1pre_2x_2 == 0) && (presr.field.pre_post1pre_2x_2 == 0) &&
           (presr.field.post1_post1pre_2x_2 == 0)) {
    presr.field.dc_level_post1pre_2x_2 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((presr.field.dc_level_post1pre_2x_1 == 0) && (presr.field.pre_post1pre_2x_1 == 0) &&
           (presr.field.post1_post1pre_2x_1 == 0)) {
    presr.field.dc_level_post1pre_2x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((presr.field.dc_level_post1pre_2x_0 == 0) && (presr.field.pre_post1pre_2x_0 == 0) &&
           (presr.field.post1_post1pre_2x_0 == 0)) {
    presr.field.dc_level_post1pre_2x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p2sr.field.dc_level_post2_1x_1 == 0) && (p2sr.field.post2_1x_1 == 0) &&
           (p2sr.field.dc_level_post2_1x_0 == 0) && (p2sr.field.post2_1x_0 == 0)) {
    p2sr.field.dc_level_post2_1x_1 = 1;
    p2sr.field.dc_level_post2_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p2sr.field.dc_level_post2_1x_1 == 0) && (p2sr.field.post2_1x_1 == 0) &&
           (p1sr.field.dc_level_post1_1x_1 == 0) && (p1sr.field.post1_1x_1 == 0)) {
    p2sr.field.dc_level_post2_1x_1 = 1;
    p1sr.field.dc_level_post1_1x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p2sr.field.dc_level_post2_1x_1 == 0) && (p2sr.field.post2_1x_1 == 0) &&
           (p1sr.field.dc_level_post1_1x_0 == 0) && (p1sr.field.post1_1x_0 == 0)) {
    p2sr.field.dc_level_post2_1x_1 = 1;
    p1sr.field.dc_level_post1_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p2sr.field.dc_level_post2_1x_0 == 0) && (p2sr.field.post2_1x_0 == 0) &&
           (p1sr.field.dc_level_post1_1x_1 == 0) && (p1sr.field.post1_1x_1 == 0)) {
    p2sr.field.dc_level_post2_1x_0 = 1;
    p1sr.field.dc_level_post1_1x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p2sr.field.dc_level_post2_1x_0 == 0) && (p2sr.field.post2_1x_0 == 0) &&
           (p1sr.field.dc_level_post1_1x_0 == 0) && (p1sr.field.post1_1x_0 == 0)) {
    p2sr.field.dc_level_post2_1x_0 = 1;
    p1sr.field.dc_level_post1_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p1sr.field.dc_level_post1_1x_1 == 0) && (p1sr.field.post1_1x_1 == 0) &&
           (p1sr.field.dc_level_post1_1x_0 == 0) && (p1sr.field.post1_1x_0 == 0)) {
    p1sr.field.dc_level_post1_1x_1 = 1;
    p1sr.field.dc_level_post1_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else  if ((p2sr.field.dc_level_post2_1x_1 == 0) && (p2sr.field.post2_1x_1 == 0) &&
            (presr.field.dc_level_post1pre_1x_1 == 0) && (presr.field.pre_post1pre_1x_1 == 0) &&
            (presr.field.post1_post1pre_1x_1 == 0)) {
    p2sr.field.dc_level_post2_1x_1 = 1;
    presr.field.dc_level_post1pre_1x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p2sr.field.dc_level_post2_1x_1 == 0) && (p2sr.field.post2_1x_1 == 0) &&
           (p1sr.field.dc_level_post1pre_1x_0 == 0) && (p1sr.field.pre_post1pre_1x_0 == 0) &&
           (p1sr.field.post1_post1pre_1x_0 == 0)) {
    p2sr.field.dc_level_post2_1x_1 = 1;
    p1sr.field.dc_level_post1pre_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p2sr.field.dc_level_post2_1x_0 == 0) && (p2sr.field.post2_1x_0 == 0) &&
           (presr.field.dc_level_post1pre_1x_1 == 0) && (presr.field.pre_post1pre_1x_1 == 0) &&
           (presr.field.post1_post1pre_1x_1 == 0)) {
    p2sr.field.dc_level_post2_1x_0 = 1;
    presr.field.dc_level_post1pre_1x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p2sr.field.dc_level_post2_1x_0 == 0) && (p2sr.field.post2_1x_0 == 0) &&
           (p1sr.field.dc_level_post1pre_1x_0 == 0) && (p1sr.field.pre_post1pre_1x_0 == 0) &&
           (p1sr.field.post1_post1pre_1x_0 == 0)) {
    p2sr.field.dc_level_post2_1x_0 = 1;
    p1sr.field.dc_level_post1pre_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p1sr.field.dc_level_post1_1x_1 == 0) && (p1sr.field.post1_1x_1 == 0) &&
           (presr.field.dc_level_post1pre_1x_1 == 0) && (presr.field.pre_post1pre_1x_1 == 0) &&
           (presr.field.post1_post1pre_1x_1 == 0)) {
    p1sr.field.dc_level_post1_1x_1 = 1;
    presr.field.dc_level_post1pre_1x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p1sr.field.dc_level_post1_1x_1 == 0) && (p1sr.field.post1_1x_1 == 0) &&
           (p1sr.field.dc_level_post1pre_1x_0 == 0) && (p1sr.field.pre_post1pre_1x_0 == 0) &&
           (p1sr.field.post1_post1pre_1x_0 == 0)) {
    p1sr.field.dc_level_post1_1x_1 = 1;
    p1sr.field.dc_level_post1pre_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p1sr.field.dc_level_post1_1x_0 == 0) && (p1sr.field.post1_1x_0 == 0) &&
           (presr.field.dc_level_post1pre_1x_1 == 0) && (presr.field.pre_post1pre_1x_1 == 0) &&
           (presr.field.post1_post1pre_1x_1 == 0)) {
    p1sr.field.dc_level_post1_1x_0 = 1;
    presr.field.dc_level_post1pre_1x_1 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((p1sr.field.dc_level_post1_1x_0 == 0) && (p1sr.field.post1_1x_0 == 0) &&
           (p1sr.field.dc_level_post1pre_1x_0 == 0) && (p1sr.field.pre_post1pre_1x_0 == 0) &&
           (p1sr.field.post1_post1pre_1x_0 == 0)) {
    p1sr.field.dc_level_post1_1x_0 = 1;
    p1sr.field.dc_level_post1pre_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
  
  if (!dc_level) {
    return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
  }       
  else if ((presr.field.dc_level_post1pre_1x_1 == 0) && (presr.field.pre_post1pre_1x_1 == 0) &&
           (presr.field.post1_post1pre_1x_1 == 0) && (p1sr.field.dc_level_post1pre_1x_0 == 0) &&
           (p1sr.field.pre_post1pre_1x_0 == 0) && (p1sr.field.post1_post1pre_1x_0 == 0)) {
    presr.field.dc_level_post1pre_1x_1 = 1;
    p1sr.field.dc_level_post1pre_1x_0 = 1;
    dc_level = (uint8_t)(dc_level - 2);
  }
   
  return(_write_txfir_slicer_controls(sa__, p1sr, p2sr, mssr, presr));
}

err_code_t merlin16_shortfin_apply_txfir_cfg(srds_access_t *sa__, int8_t pre, int8_t main, int8_t post1, int8_t post2) {

  uint8_t osr_mode;  
  err_code_t failcode = merlin16_shortfin_validate_txfir_cfg(sa__, pre, main, post1, post2);

  if (!failcode) {
    EFUN(wr_ams_tx_post2to1(post2==0));      /* Convert post2 slices to post1, if post2 is 0 */
    EFUN(wr_ams_tx_en_pre(pre>0));           /* Enable the TXFIR blocks */   /* KD - enables required? */
    EFUN(wr_ams_tx_en_post1(post1>0));
    EFUN(wr_ams_tx_en_post2(post2>0));
    EFUN(wr_cl72_txfir_pre((uint8_t)pre));
    EFUN(wr_cl72_txfir_main((uint8_t)main));
    EFUN(wr_cl72_txfir_post((uint8_t)post1));
    EFUN(wr_txfir_post2((uint8_t)post2));

    EFUN(_txfir_tap_mapping(sa__, pre, main, post1, post2));

    /* Setting correct enable_os modes depending on OSR mode */
    /* Note: expects the user to set OSR mode before setting TXFIR */
    ESTM(osr_mode = rd_osr_mode());
    if (osr_mode == MERLIN16_SHORTFIN_OSX2) {
      EFUN(wr_ams_tx_enable_os_2(1));
      EFUN(wr_ams_tx_enable_os_4(0));
    }
    else if (osr_mode == MERLIN16_SHORTFIN_OSX4) {
      EFUN(wr_ams_tx_enable_os_2(0));
      EFUN(wr_ams_tx_enable_os_4(1));
    }

    if ((pre+main+post1+post2) > 45) {    /* KD - required? */
      EFUN(wr_ams_tx_refcalshunt(5));
    } else {
      EFUN(wr_ams_tx_refcalshunt(4));
    }
  }

  return (merlin16_shortfin_error(sa__, failcode));
}


err_code_t merlin16_shortfin_read_tx_afe(srds_access_t *sa__, enum merlin16_shortfin_tx_afe_settings_enum param, int8_t *val) {
  if(!val) {
      return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }
  
  switch(param) {
    case TX_AFE_PRE:
      EFUN(merlin16_shortfin_INTERNAL_get_tx_pre(sa__, val));
      break;
    case TX_AFE_MAIN:
      EFUN(merlin16_shortfin_INTERNAL_get_tx_main(sa__, val));
      break;
    case TX_AFE_POST1:
      EFUN(merlin16_shortfin_INTERNAL_get_tx_post1(sa__, val));
      break;
    case TX_AFE_POST2:
      EFUN(merlin16_shortfin_INTERNAL_get_tx_post2(sa__, val));
      break;
    default:
      return(merlin16_shortfin_error(sa__, ERR_CODE_BAD_PTR_OR_INVALID_INPUT));
  }
  
  return(ERR_CODE_NONE);
}


err_code_t merlin16_shortfin_config_tx_hpf(srds_access_t *sa__, uint8_t val) {  /* KD */

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
  }

  return(ERR_CODE_NONE);
}


err_code_t merlin16_shortfin_rd_tx_hpf_config(srds_access_t *sa__, uint8_t *pval) {  /* KD */
  uint8_t hpf_cfg=0,sum=0;
  
  if(!pval) {
      return(ERR_CODE_BAD_PTR_OR_INVALID_INPUT);
  }
  
  ESTM(hpf_cfg = rd_ams_tx_en_hpf());
  
  sum = (uint8_t)(((hpf_cfg & 0x1)) +
        ((hpf_cfg & 0x2)>>1) +
        ((hpf_cfg & 0x4)>>2) +
        ((hpf_cfg & 0x8)>>3));
  
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
  }
  
  return(ERR_CODE_NONE);
}

/* returns 000111 (7 = 8-1), for n = 3  */
#define BFMASK(n) ((1<<((n)))-1)

static err_code_t _update_p2sr_reg_word(struct p2sr_reg_st *st) {
  uint16_t in = 0;
  in = (uint16_t)(in << 8); in |= 0/*st->field.reserved0*/      & BFMASK(8);
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post2_2x_1 & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post2_2x_1          & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post2_2x_0 & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post2_2x_0          & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post2_1x_1 & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post2_1x_1          & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post2_1x_0 & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post2_1x_0          & BFMASK(1)));
  st->word = in;
  return ERR_CODE_NONE;
}

static err_code_t _update_p1sr_reg_word(struct p1sr_reg_st *st) {
  uint16_t in = 0;
  in = (uint16_t)(in << 1); in |= 0/*st->field.reserved1*/         & BFMASK(1);
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post1pre_1x_0 & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post1_post1pre_1x_0    & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.pre_post1pre_1x_0      & BFMASK(1)));
  in = (uint16_t)(in << 2); in |= 0/*st->field.reserved0*/         & BFMASK(2);
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post1_2x_2    & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post1_2x_2             & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post1_2x_1    & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post1_2x_1             & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post1_2x_0    & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post1_2x_0             & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post1_1x_1    & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post1_1x_1             & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post1_1x_0    & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post1_1x_0             & BFMASK(1)));
  st->word = in;
  return ERR_CODE_NONE;
}

static err_code_t _update_presr_reg_word(struct presr_reg_st *st) {
  uint16_t in = 0;
  in = (uint16_t)(in << 1); in |= 0/*st->field.reserved0*/         & BFMASK(1);
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post1pre_2x_3 & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post1_post1pre_2x_3    & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.pre_post1pre_2x_3      & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post1pre_2x_2 & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post1_post1pre_2x_2    & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.pre_post1pre_2x_2      & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post1pre_2x_1 & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post1_post1pre_2x_1    & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.pre_post1pre_2x_1      & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post1pre_2x_0 & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post1_post1pre_2x_0    & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.pre_post1pre_2x_0      & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.dc_level_post1pre_1x_1 & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.post1_post1pre_1x_1    & BFMASK(1)));
  in = (uint16_t)(in << 1); in = (uint16_t)(in | (st->field.pre_post1pre_1x_1      & BFMASK(1)));
  st->word = in;
  return ERR_CODE_NONE;
}

static err_code_t _update_mssr_reg_word(struct mssr_reg_st *st) {
  uint16_t in = 0;
  in = (uint16_t)(in << 12); in |= 0/*st->field.reserved0*/ & BFMASK(12);
  in = (uint16_t)(in << 1);  in = (uint16_t)(in | (st->field.dc_level_main_2x_1 & BFMASK(1)));
  in = (uint16_t)(in << 1);  in = (uint16_t)(in | (st->field.dc_level_main_2x_0 & BFMASK(1)));
  in = (uint16_t)(in << 1);  in = (uint16_t)(in | (st->field.dc_level_main_1x   & BFMASK(1)));
  in = (uint16_t)(in << 1);  in = (uint16_t)(in | (st->field.dc_level_0p5x      & BFMASK(1)));
  st->word = in;
  return ERR_CODE_NONE;
}

static err_code_t _write_txfir_slicer_controls(srds_access_t *sa__,
                                               struct p1sr_reg_st p1_st, struct p2sr_reg_st p2_st,
                                               struct mssr_reg_st ms_st, struct presr_reg_st pre_st) {

  EFUN(_update_p1sr_reg_word(&p1_st));
  EFUN(_update_p2sr_reg_word(&p2_st));
  EFUN(_update_mssr_reg_word(&ms_st));    
  EFUN(_update_presr_reg_word(&pre_st));

  EFUN(reg_wr_TX_FED_POST1_AFE_CONTROL0(p1_st.word));  
  EFUN(reg_wr_TX_FED_POST2_AFE_CONTROL0(p2_st.word));  
  EFUN(reg_wr_TX_FED_DC_LEVEL_AFE_CONTROL0(ms_st.word));  
  EFUN(reg_wr_TX_FED_POST1PRE_AFE_CONTROL0(pre_st.word));  
  return(ERR_CODE_NONE);
}

#undef BFMASK

