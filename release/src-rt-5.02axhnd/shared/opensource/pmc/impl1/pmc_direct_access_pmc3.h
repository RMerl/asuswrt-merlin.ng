/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

/*****************************************************************************
 *  Description:
 *      This contains the PMC3 driver specific implementation.
 *****************************************************************************/

#define KEYHOLE_IDX 0

static int read_bpcm_reg_direct(int devAddr, int wordOffset, uint32 *value)
{
    int status = kPMC_NO_ERROR;
    int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
    uint32 address, ctlSts;
    volatile PMB_keyhole_reg *keyhole = &PMB->keyhole[KEYHOLE_IDX]; 

    address = ((devAddr&0xff) * ((PMB->config>>PMB_NUM_REGS_SHIFT) & PMB_NUM_REGS_MASK)) | (wordOffset); 

    keyhole->control = PMC_PMBM_START | (bus << PMC_PMBM_BUS_SHIFT) | (PMC_PMBM_Read) | address;
    ctlSts = keyhole->control;
    while( ctlSts & PMC_PMBM_BUSY ) ctlSts = keyhole->control; /*wait for completion*/

    if( ctlSts & PMC_PMBM_TIMEOUT )
        status = kPMC_COMMAND_TIMEOUT;
    else
        *value = keyhole->rd_data;

   return status;
}

static int write_bpcm_reg_direct(int devAddr, int wordOffset, uint32 value)
{
    int status = kPMC_NO_ERROR;
    int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
    uint32 address, ctlSts;
    volatile PMB_keyhole_reg *keyhole = &PMB->keyhole[KEYHOLE_IDX]; 

    address = ((devAddr&0xff) * ((PMB->config>>PMB_NUM_REGS_SHIFT) & PMB_NUM_REGS_MASK)) | (wordOffset); 
    keyhole->wr_data = value;
    keyhole->control = PMC_PMBM_START | (bus << PMC_PMBM_BUS_SHIFT) | (PMC_PMBM_Write) | address;

    ctlSts = keyhole->control;
    while( ctlSts & PMC_PMBM_BUSY ) ctlSts = keyhole->control; /*wait for completion*/

    if( ctlSts & PMC_PMBM_TIMEOUT )
        status = kPMC_COMMAND_TIMEOUT;

    return status;
}
