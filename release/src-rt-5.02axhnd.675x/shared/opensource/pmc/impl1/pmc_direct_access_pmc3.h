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
 *      This contains the PMC3 driver specific implementation.
 *****************************************************************************/
#ifndef KEYHOLE_IDX
#define KEYHOLE_IDX 0
#endif

static int read_bpcm_reg_direct(int devAddr, int wordOffset, uint32 *value)
{
    int status = kPMC_NO_ERROR;
    int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
    uint32 address, ctlSts;
    volatile PMB_keyhole_reg *keyhole = &PMB->keyhole[KEYHOLE_IDX]; 

    while (!(keyhole->mutex & 1)) ;

    address = ((devAddr&0xff) * ((PMB->config>>PMB_NUM_REGS_SHIFT) & PMB_NUM_REGS_MASK)) | (wordOffset); 

    keyhole->control = PMC_PMBM_START | (bus << PMC_PMBM_BUS_SHIFT) | (PMC_PMBM_Read) | address;
    ctlSts = keyhole->control;
    while( ctlSts & PMC_PMBM_BUSY ) ctlSts = keyhole->control; /*wait for completion*/

    if( ctlSts & PMC_PMBM_TIMEOUT )
        status = kPMC_COMMAND_TIMEOUT;
    else
        *value = keyhole->rd_data;

    keyhole->mutex = 1;
    return status;
}

static int write_bpcm_reg_direct(int devAddr, int wordOffset, uint32 value)
{
    int status = kPMC_NO_ERROR;
    int bus = (devAddr >> PMB_BUS_ID_SHIFT) & 0x3;
    uint32 address, ctlSts;
    volatile PMB_keyhole_reg *keyhole = &PMB->keyhole[KEYHOLE_IDX]; 

    while (!(keyhole->mutex & 1)) ;

    address = ((devAddr&0xff) * ((PMB->config>>PMB_NUM_REGS_SHIFT) & PMB_NUM_REGS_MASK)) | (wordOffset); 
    keyhole->wr_data = value;
    keyhole->control = PMC_PMBM_START | (bus << PMC_PMBM_BUS_SHIFT) | (PMC_PMBM_Write) | address;

    ctlSts = keyhole->control;
    while( ctlSts & PMC_PMBM_BUSY ) ctlSts = keyhole->control; /*wait for completion*/

    if( ctlSts & PMC_PMBM_TIMEOUT )
        status = kPMC_COMMAND_TIMEOUT;

    keyhole->mutex = 1;
    return status;
}
