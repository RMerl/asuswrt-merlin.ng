/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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
