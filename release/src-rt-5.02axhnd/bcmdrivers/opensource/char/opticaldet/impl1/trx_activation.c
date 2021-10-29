/*
<:copyright-BRCM:2017:DUAL/GPL:standard

   Copyright (c) 2017 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
 
/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
/* general */
#include <linux/string.h>
#include <bdmf_dev.h>


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
