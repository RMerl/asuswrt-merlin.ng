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
 *      This contains the PMC2 driver specific implementation.
 *****************************************************************************/
static int read_bpcm_reg_direct(int devAddr, int wordOffset, uint32 *value)
{
    int status = kPMC_NO_ERROR;
    int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
    uint32 address, ctlSts;


    if (gIsClassic[devAddr & 0xff])
        address = (((devAddr&0xff)<<4) * 256) | (wordOffset);  // Old BPCM
    else
        address = ((devAddr&0xff) * 256) | (wordOffset);  // New BPCM

    PMM->ctrl = PMC_PMBM_START | (bus << PMC_PMBM_BUS_SHIFT) | (PMC_PMBM_Read) | address;
    ctlSts = PMM->ctrl;
    while( ctlSts & PMC_PMBM_BUSY ) ctlSts = PMM->ctrl; /*wait for completion*/

    if( ctlSts & PMC_PMBM_TIMEOUT )
        status = kPMC_COMMAND_TIMEOUT;
    else
        *value = PMM->rd_data;

    return status;
}

static int write_bpcm_reg_direct(int devAddr, int wordOffset, uint32 value)
{
    int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
    int status = kPMC_NO_ERROR;
    uint32 address, ctlSts;

    if (gIsClassic[devAddr & 0xff])
        address = (((devAddr&0xff)<<4) * 256) | (wordOffset);  // Old BPCM
    else
        address = ((devAddr&0xff) * 256) | (wordOffset);  // New BPCM

    PMM->wr_data = value;

    PMM->ctrl = PMC_PMBM_START | (bus << PMC_PMBM_BUS_SHIFT) | (PMC_PMBM_Write) | address;
    ctlSts = PMM->ctrl;
    while( ctlSts & PMC_PMBM_BUSY ) ctlSts = PMM->ctrl; /*wait for completion*/

    if( ctlSts & PMC_PMBM_TIMEOUT )
        status = kPMC_COMMAND_TIMEOUT;

    return status;
}

