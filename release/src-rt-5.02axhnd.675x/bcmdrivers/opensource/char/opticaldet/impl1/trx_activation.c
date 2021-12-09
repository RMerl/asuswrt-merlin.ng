/*
<:copyright-BRCM:2017:DUAL/GPL:standard

   Copyright (c) 2017 Broadcom 
   All Rights Reserved

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
 
/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
/* general */
#include <linux/string.h>
#include "bcmsfp_i2c.h"


/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* Functions Definitions                                                      */
/*                                                                            */
/******************************************************************************/


void ltw2601_activation (int bus)
{
   static uint8_t trx_act_seq[4] = {0x12, 0x34, 0x56, 0x78};
   static uint8_t trx_base = 0x7B ;
   int i;
   int rc;

   for (i=0; i<sizeof(trx_act_seq); i++)
   {
       rc = bcmsfp_write_byte(bus, 0, trx_base + i, trx_act_seq[i]);
       if (rc != 0)
       {
           printk(KERN_INFO "opticaldet: Failed to write value 0x%02x to addr 0x%02x", trx_act_seq[i], trx_base + i) ;
       }
   }
}
