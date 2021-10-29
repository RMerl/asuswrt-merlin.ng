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
 *      This contains the PMC1 driver specific implementation.
 *****************************************************************************/

static int read_bpcm_reg_direct(int devAddr, int wordOffset, uint32 *value)
{
    int status = kPMC_NO_ERROR;
    int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
    volatile PMBMaster *pmbm_ptr;

    if (bus >= PMB_BUS_MAX)
        return kPMC_INVALID_BUS;

    pmbm_ptr = &(PROCMON->PMBM[bus]);

    /* Make sure PMBM is not busy */

    pmbm_ptr->ctrl = PMC_PMBM_START | PMC_PMBM_Read |
        ((devAddr & 0xff) << 12) | wordOffset;

    while (pmbm_ptr->ctrl & PMC_PMBM_START);

    if (pmbm_ptr->ctrl & PMC_PMBM_TIMEOUT)
        status = kPMC_COMMAND_TIMEOUT;
    else
        *value = pmbm_ptr->rd_data;

    return status;
}

static int write_bpcm_reg_direct(int devAddr, int wordOffset, uint32 value)
{
    int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
    int status = kPMC_NO_ERROR;
    volatile PMBMaster *pmbm_ptr;
    if (bus >= PMB_BUS_MAX)
        return kPMC_INVALID_BUS;

    pmbm_ptr = &(PROCMON->PMBM[bus]);

    pmbm_ptr->wr_data = value;
    pmbm_ptr->ctrl = PMC_PMBM_START | PMC_PMBM_Write |
        ((devAddr & 0xff) << 12) | wordOffset;

    while (pmbm_ptr->ctrl & PMC_PMBM_START);

    if (pmbm_ptr->ctrl & PMC_PMBM_TIMEOUT)
        status = kPMC_COMMAND_TIMEOUT;

    return status;
}
