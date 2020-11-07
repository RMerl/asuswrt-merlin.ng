/*---------------------------------------------------------------------------

<:copyright-BRCM:2013:proprietary:standard 

   Copyright (c) 2013 Broadcom 
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
 ------------------------------------------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <bcm_OS_Deps.h>
#include <linux/bcm_log.h>
#include <rdpa_epon.h>
#include "pmd_msg.h"
#include "pmd_op.h"
#include "pmd.h"
#include "pmd_temp_cal.h"

#define DATA_COL_SIZE 150
#define TIME_STR_LEN 9

static uint8_t valid_state = 0;

typedef enum
{
    pmd_bias_min_col,
    pmd_bias_max_col,
    pmd_mod_min_col,
    pmd_mod_max_col,
    pmd_bias_avg_col,
    pmd_mod_avg_col,
    pmd_offset0_col,
    pmd_offset2_col,
    pmd_ext_temperature,
    pmd_die_temperature,
    pmd_max_col
}pmd_data_col_params;

#define PMD_VALID_TRACKING_TEMP pmd_max_col

extern uint8_t pmd_stack_state;


typedef struct
{
    uint8_t time_stamp[DATA_COL_SIZE][TIME_STR_LEN];
    int16_t queue[DATA_COL_SIZE][pmd_max_col];  /* data collection array */
    uint8_t index;
} pmd_data_collection;

static pmd_data_collection pmd_data_col = {0};

static void pmd_data_col_add_entry(int16_t * val)
{
    struct timeval tv;
    struct tm tm_ptr;               

    do_gettimeofday(&tv);
    time_to_tm(tv.tv_sec, 0, &tm_ptr);

    sprintf(pmd_data_col.time_stamp[pmd_data_col.index],"%02d:%02d:%02d", tm_ptr.tm_hour, tm_ptr.tm_min, tm_ptr.tm_sec);  
    memcpy(&pmd_data_col.queue[pmd_data_col.index][0], val, pmd_max_col * sizeof(int16_t));
    pmd_data_col.index++;
    if (pmd_data_col.index == DATA_COL_SIZE)
        pmd_data_col.index = 0;
}

void pmd_data_col_print(void)
{
    int i, parm, index;

    printk("time      bias_min    bias_max     mod_min     mod_max    bias_avg     mod_avg     offset0     offset1    ext_temp    die_temp\n");
    printk("------------------------------------------------------------------------------------------------------------------------------\n");

    index = pmd_data_col.index;

    for (i = 0; i < DATA_COL_SIZE; i++)
    {
        if (strlen(pmd_data_col.time_stamp[index]))
        {
            printk("%s     ", pmd_data_col.time_stamp[index]);

            for (parm = 0; parm < pmd_max_col; parm++)
            {
                printk("%-12x", pmd_data_col.queue[index][parm]);
            }
            printk("\n");
        }

        index = (index ==  DATA_COL_SIZE-1 ? 0 : index+1);
    }
}

int pmd_data_collect(uint8_t valid)
{
    int rc;
    int16_t val[pmd_max_col] = {0};

    if (valid)
    {
        rc = pmd_msg_handler(hmid_cur_state_get, val, pmd_max_col * 2);
        if (rc)
        {
            BCM_LOG_DEBUG(BCM_LOG_ID_PMD,"fail to collect data\n");
            return -1;
        }

        pmd_data_col_add_entry(val);
        valid_state = 1;
    }
    else if (valid_state)
    {
        pmd_data_col_add_entry(val);
        valid_state = 0;
    }


    return 0;
}
