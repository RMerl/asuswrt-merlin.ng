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

#include "drivers_epon_ag.h"
#include "drv_epon_xpcstx_ag.h"
bdmf_error_t ag_drv_xpcstx_tx_control_set(const xpcstx_tx_control *tx_control)
{
    uint32_t reg_tx_control=0;

#ifdef VALIDATE_PARMS
    if(!tx_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((tx_control->cfgenrmtfaultdet125 >= _1BITS_MAX_VAL_) ||
       (tx_control->cfglsrtristateen125 >= _1BITS_MAX_VAL_) ||
       (tx_control->cfgenseqnum125 >= _1BITS_MAX_VAL_) ||
       (tx_control->cfgenscrmbcont125 >= _1BITS_MAX_VAL_) ||
       (tx_control->cfglsrenacthi125 >= _1BITS_MAX_VAL_) ||
       (tx_control->cfgenlsralways125 >= _1BITS_MAX_VAL_) ||
       (tx_control->cfgenlsrtilendslot125 >= _1BITS_MAX_VAL_) ||
       (tx_control->cfgtxoutbyteflip125 >= _1BITS_MAX_VAL_) ||
       (tx_control->cfgentxout125 >= _1BITS_MAX_VAL_) ||
       (tx_control->cfgentxscrb125 >= _1BITS_MAX_VAL_) ||
       (tx_control->cfgentxfec125 >= _1BITS_MAX_VAL_) ||
       (tx_control->pcstxnotrdy >= _1BITS_MAX_VAL_) ||
       (tx_control->pcstxdtportrstn >= _1BITS_MAX_VAL_) ||
       (tx_control->pcstxrstn >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, CFGENRMTFAULTDET125, reg_tx_control, tx_control->cfgenrmtfaultdet125);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, CFGLSRTRISTATEEN125, reg_tx_control, tx_control->cfglsrtristateen125);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, CFGENSEQNUM125, reg_tx_control, tx_control->cfgenseqnum125);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, CFGENSCRMBCONT125, reg_tx_control, tx_control->cfgenscrmbcont125);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, CFGLSRENACTHI125, reg_tx_control, tx_control->cfglsrenacthi125);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, CFGENLSRALWAYS125, reg_tx_control, tx_control->cfgenlsralways125);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, CFGENLSRTILENDSLOT125, reg_tx_control, tx_control->cfgenlsrtilendslot125);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, CFGTXOUTBYTEFLIP125, reg_tx_control, tx_control->cfgtxoutbyteflip125);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, CFGENTXOUT125, reg_tx_control, tx_control->cfgentxout125);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, CFGENTXSCRB125, reg_tx_control, tx_control->cfgentxscrb125);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, CFGENTXFEC125, reg_tx_control, tx_control->cfgentxfec125);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, PCSTXNOTRDY, reg_tx_control, tx_control->pcstxnotrdy);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, PCSTXDTPORTRSTN, reg_tx_control, tx_control->pcstxdtportrstn);
    reg_tx_control = RU_FIELD_SET(0, XPCSTX, TX_CONTROL, PCSTXRSTN, reg_tx_control, tx_control->pcstxrstn);

    RU_REG_WRITE(0, XPCSTX, TX_CONTROL, reg_tx_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_control_get(xpcstx_tx_control *tx_control)
{
    uint32_t reg_tx_control=0;

#ifdef VALIDATE_PARMS
    if(!tx_control)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_CONTROL, reg_tx_control);

    tx_control->cfgenrmtfaultdet125 = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, CFGENRMTFAULTDET125, reg_tx_control);
    tx_control->cfglsrtristateen125 = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, CFGLSRTRISTATEEN125, reg_tx_control);
    tx_control->cfgenseqnum125 = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, CFGENSEQNUM125, reg_tx_control);
    tx_control->cfgenscrmbcont125 = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, CFGENSCRMBCONT125, reg_tx_control);
    tx_control->cfglsrenacthi125 = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, CFGLSRENACTHI125, reg_tx_control);
    tx_control->cfgenlsralways125 = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, CFGENLSRALWAYS125, reg_tx_control);
    tx_control->cfgenlsrtilendslot125 = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, CFGENLSRTILENDSLOT125, reg_tx_control);
    tx_control->cfgtxoutbyteflip125 = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, CFGTXOUTBYTEFLIP125, reg_tx_control);
    tx_control->cfgentxout125 = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, CFGENTXOUT125, reg_tx_control);
    tx_control->cfgentxscrb125 = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, CFGENTXSCRB125, reg_tx_control);
    tx_control->cfgentxfec125 = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, CFGENTXFEC125, reg_tx_control);
    tx_control->pcstxnotrdy = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, PCSTXNOTRDY, reg_tx_control);
    tx_control->pcstxdtportrstn = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, PCSTXDTPORTRSTN, reg_tx_control);
    tx_control->pcstxrstn = RU_FIELD_GET(0, XPCSTX, TX_CONTROL, PCSTXRSTN, reg_tx_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_int_stat_set(const xpcstx_tx_int_stat *tx_int_stat)
{
    uint32_t reg_tx_int_stat=0;

#ifdef VALIDATE_PARMS
    if(!tx_int_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((tx_int_stat->laseronmax >= _1BITS_MAX_VAL_) ||
       (tx_int_stat->laseroff >= _1BITS_MAX_VAL_) ||
       (tx_int_stat->grantlagerr >= _1BITS_MAX_VAL_) ||
       (tx_int_stat->back2backgnt >= _1BITS_MAX_VAL_) ||
       (tx_int_stat->fecunderrun >= _1BITS_MAX_VAL_) ||
       (tx_int_stat->gearboxunderrun >= _1BITS_MAX_VAL_) ||
       (tx_int_stat->gnttooshort >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_int_stat = RU_FIELD_SET(0, XPCSTX, TX_INT_STAT, LASERONMAX, reg_tx_int_stat, tx_int_stat->laseronmax);
    reg_tx_int_stat = RU_FIELD_SET(0, XPCSTX, TX_INT_STAT, LASEROFF, reg_tx_int_stat, tx_int_stat->laseroff);
    reg_tx_int_stat = RU_FIELD_SET(0, XPCSTX, TX_INT_STAT, GRANTLAGERR, reg_tx_int_stat, tx_int_stat->grantlagerr);
    reg_tx_int_stat = RU_FIELD_SET(0, XPCSTX, TX_INT_STAT, BACK2BACKGNT, reg_tx_int_stat, tx_int_stat->back2backgnt);
    reg_tx_int_stat = RU_FIELD_SET(0, XPCSTX, TX_INT_STAT, FECUNDERRUN, reg_tx_int_stat, tx_int_stat->fecunderrun);
    reg_tx_int_stat = RU_FIELD_SET(0, XPCSTX, TX_INT_STAT, GEARBOXUNDERRUN, reg_tx_int_stat, tx_int_stat->gearboxunderrun);
    reg_tx_int_stat = RU_FIELD_SET(0, XPCSTX, TX_INT_STAT, GNTTOOSHORT, reg_tx_int_stat, tx_int_stat->gnttooshort);

    RU_REG_WRITE(0, XPCSTX, TX_INT_STAT, reg_tx_int_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_int_stat_get(xpcstx_tx_int_stat *tx_int_stat)
{
    uint32_t reg_tx_int_stat=0;

#ifdef VALIDATE_PARMS
    if(!tx_int_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_INT_STAT, reg_tx_int_stat);

    tx_int_stat->laseronmax = RU_FIELD_GET(0, XPCSTX, TX_INT_STAT, LASERONMAX, reg_tx_int_stat);
    tx_int_stat->laseroff = RU_FIELD_GET(0, XPCSTX, TX_INT_STAT, LASEROFF, reg_tx_int_stat);
    tx_int_stat->grantlagerr = RU_FIELD_GET(0, XPCSTX, TX_INT_STAT, GRANTLAGERR, reg_tx_int_stat);
    tx_int_stat->back2backgnt = RU_FIELD_GET(0, XPCSTX, TX_INT_STAT, BACK2BACKGNT, reg_tx_int_stat);
    tx_int_stat->fecunderrun = RU_FIELD_GET(0, XPCSTX, TX_INT_STAT, FECUNDERRUN, reg_tx_int_stat);
    tx_int_stat->gearboxunderrun = RU_FIELD_GET(0, XPCSTX, TX_INT_STAT, GEARBOXUNDERRUN, reg_tx_int_stat);
    tx_int_stat->gnttooshort = RU_FIELD_GET(0, XPCSTX, TX_INT_STAT, GNTTOOSHORT, reg_tx_int_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_int_mask_set(const xpcstx_tx_int_mask *tx_int_mask)
{
    uint32_t reg_tx_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!tx_int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((tx_int_mask->laseronmaxmask >= _1BITS_MAX_VAL_) ||
       (tx_int_mask->laseroffmask >= _1BITS_MAX_VAL_) ||
       (tx_int_mask->grantlagerrmsk >= _1BITS_MAX_VAL_) ||
       (tx_int_mask->back2bckgntmsk >= _1BITS_MAX_VAL_) ||
       (tx_int_mask->fecunderrunmsk >= _1BITS_MAX_VAL_) ||
       (tx_int_mask->gearboxunderrunmsk >= _1BITS_MAX_VAL_) ||
       (tx_int_mask->gnttooshortmsk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_int_mask = RU_FIELD_SET(0, XPCSTX, TX_INT_MASK, LASERONMAXMASK, reg_tx_int_mask, tx_int_mask->laseronmaxmask);
    reg_tx_int_mask = RU_FIELD_SET(0, XPCSTX, TX_INT_MASK, LASEROFFMASK, reg_tx_int_mask, tx_int_mask->laseroffmask);
    reg_tx_int_mask = RU_FIELD_SET(0, XPCSTX, TX_INT_MASK, GRANTLAGERRMSK, reg_tx_int_mask, tx_int_mask->grantlagerrmsk);
    reg_tx_int_mask = RU_FIELD_SET(0, XPCSTX, TX_INT_MASK, BACK2BCKGNTMSK, reg_tx_int_mask, tx_int_mask->back2bckgntmsk);
    reg_tx_int_mask = RU_FIELD_SET(0, XPCSTX, TX_INT_MASK, FECUNDERRUNMSK, reg_tx_int_mask, tx_int_mask->fecunderrunmsk);
    reg_tx_int_mask = RU_FIELD_SET(0, XPCSTX, TX_INT_MASK, GEARBOXUNDERRUNMSK, reg_tx_int_mask, tx_int_mask->gearboxunderrunmsk);
    reg_tx_int_mask = RU_FIELD_SET(0, XPCSTX, TX_INT_MASK, GNTTOOSHORTMSK, reg_tx_int_mask, tx_int_mask->gnttooshortmsk);

    RU_REG_WRITE(0, XPCSTX, TX_INT_MASK, reg_tx_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_int_mask_get(xpcstx_tx_int_mask *tx_int_mask)
{
    uint32_t reg_tx_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!tx_int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_INT_MASK, reg_tx_int_mask);

    tx_int_mask->laseronmaxmask = RU_FIELD_GET(0, XPCSTX, TX_INT_MASK, LASERONMAXMASK, reg_tx_int_mask);
    tx_int_mask->laseroffmask = RU_FIELD_GET(0, XPCSTX, TX_INT_MASK, LASEROFFMASK, reg_tx_int_mask);
    tx_int_mask->grantlagerrmsk = RU_FIELD_GET(0, XPCSTX, TX_INT_MASK, GRANTLAGERRMSK, reg_tx_int_mask);
    tx_int_mask->back2bckgntmsk = RU_FIELD_GET(0, XPCSTX, TX_INT_MASK, BACK2BCKGNTMSK, reg_tx_int_mask);
    tx_int_mask->fecunderrunmsk = RU_FIELD_GET(0, XPCSTX, TX_INT_MASK, FECUNDERRUNMSK, reg_tx_int_mask);
    tx_int_mask->gearboxunderrunmsk = RU_FIELD_GET(0, XPCSTX, TX_INT_MASK, GEARBOXUNDERRUNMSK, reg_tx_int_mask);
    tx_int_mask->gnttooshortmsk = RU_FIELD_GET(0, XPCSTX, TX_INT_MASK, GNTTOOSHORTMSK, reg_tx_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_port_command_set(bdmf_boolean dataportbusy, uint8_t portselect, uint8_t portopcode, uint16_t portaddress)
{
    uint32_t reg_tx_port_command=0;

#ifdef VALIDATE_PARMS
    if((dataportbusy >= _1BITS_MAX_VAL_) ||
       (portselect >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_port_command = RU_FIELD_SET(0, XPCSTX, TX_PORT_COMMAND, DATAPORTBUSY, reg_tx_port_command, dataportbusy);
    reg_tx_port_command = RU_FIELD_SET(0, XPCSTX, TX_PORT_COMMAND, PORTSELECT, reg_tx_port_command, portselect);
    reg_tx_port_command = RU_FIELD_SET(0, XPCSTX, TX_PORT_COMMAND, PORTOPCODE, reg_tx_port_command, portopcode);
    reg_tx_port_command = RU_FIELD_SET(0, XPCSTX, TX_PORT_COMMAND, PORTADDRESS, reg_tx_port_command, portaddress);

    RU_REG_WRITE(0, XPCSTX, TX_PORT_COMMAND, reg_tx_port_command);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_port_command_get(bdmf_boolean *dataportbusy, uint8_t *portselect, uint8_t *portopcode, uint16_t *portaddress)
{
    uint32_t reg_tx_port_command=0;

#ifdef VALIDATE_PARMS
    if(!dataportbusy || !portselect || !portopcode || !portaddress)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_PORT_COMMAND, reg_tx_port_command);

    *dataportbusy = RU_FIELD_GET(0, XPCSTX, TX_PORT_COMMAND, DATAPORTBUSY, reg_tx_port_command);
    *portselect = RU_FIELD_GET(0, XPCSTX, TX_PORT_COMMAND, PORTSELECT, reg_tx_port_command);
    *portopcode = RU_FIELD_GET(0, XPCSTX, TX_PORT_COMMAND, PORTOPCODE, reg_tx_port_command);
    *portaddress = RU_FIELD_GET(0, XPCSTX, TX_PORT_COMMAND, PORTADDRESS, reg_tx_port_command);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_data_port_0_set(uint32_t portdata0)
{
    uint32_t reg_tx_data_port_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_data_port_0 = RU_FIELD_SET(0, XPCSTX, TX_DATA_PORT_0, PORTDATA0, reg_tx_data_port_0, portdata0);

    RU_REG_WRITE(0, XPCSTX, TX_DATA_PORT_0, reg_tx_data_port_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_data_port_0_get(uint32_t *portdata0)
{
    uint32_t reg_tx_data_port_0=0;

#ifdef VALIDATE_PARMS
    if(!portdata0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_DATA_PORT_0, reg_tx_data_port_0);

    *portdata0 = RU_FIELD_GET(0, XPCSTX, TX_DATA_PORT_0, PORTDATA0, reg_tx_data_port_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_data_port_1_set(uint32_t portdata1)
{
    uint32_t reg_tx_data_port_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_data_port_1 = RU_FIELD_SET(0, XPCSTX, TX_DATA_PORT_1, PORTDATA1, reg_tx_data_port_1, portdata1);

    RU_REG_WRITE(0, XPCSTX, TX_DATA_PORT_1, reg_tx_data_port_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_data_port_1_get(uint32_t *portdata1)
{
    uint32_t reg_tx_data_port_1=0;

#ifdef VALIDATE_PARMS
    if(!portdata1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_DATA_PORT_1, reg_tx_data_port_1);

    *portdata1 = RU_FIELD_GET(0, XPCSTX, TX_DATA_PORT_1, PORTDATA1, reg_tx_data_port_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_data_port_2_set(uint32_t portdata2)
{
    uint32_t reg_tx_data_port_2=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_data_port_2 = RU_FIELD_SET(0, XPCSTX, TX_DATA_PORT_2, PORTDATA2, reg_tx_data_port_2, portdata2);

    RU_REG_WRITE(0, XPCSTX, TX_DATA_PORT_2, reg_tx_data_port_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_data_port_2_get(uint32_t *portdata2)
{
    uint32_t reg_tx_data_port_2=0;

#ifdef VALIDATE_PARMS
    if(!portdata2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_DATA_PORT_2, reg_tx_data_port_2);

    *portdata2 = RU_FIELD_GET(0, XPCSTX, TX_DATA_PORT_2, PORTDATA2, reg_tx_data_port_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_sync_patt_cword_lo_set(uint32_t cfgsyncpatcwl)
{
    uint32_t reg_tx_sync_patt_cword_lo=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_sync_patt_cword_lo = RU_FIELD_SET(0, XPCSTX, TX_SYNC_PATT_CWORD_LO, CFGSYNCPATCWL, reg_tx_sync_patt_cword_lo, cfgsyncpatcwl);

    RU_REG_WRITE(0, XPCSTX, TX_SYNC_PATT_CWORD_LO, reg_tx_sync_patt_cword_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_sync_patt_cword_lo_get(uint32_t *cfgsyncpatcwl)
{
    uint32_t reg_tx_sync_patt_cword_lo=0;

#ifdef VALIDATE_PARMS
    if(!cfgsyncpatcwl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_SYNC_PATT_CWORD_LO, reg_tx_sync_patt_cword_lo);

    *cfgsyncpatcwl = RU_FIELD_GET(0, XPCSTX, TX_SYNC_PATT_CWORD_LO, CFGSYNCPATCWL, reg_tx_sync_patt_cword_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_sync_patt_cword_hi_set(uint32_t cfgsyncpatcwh)
{
    uint32_t reg_tx_sync_patt_cword_hi=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_sync_patt_cword_hi = RU_FIELD_SET(0, XPCSTX, TX_SYNC_PATT_CWORD_HI, CFGSYNCPATCWH, reg_tx_sync_patt_cword_hi, cfgsyncpatcwh);

    RU_REG_WRITE(0, XPCSTX, TX_SYNC_PATT_CWORD_HI, reg_tx_sync_patt_cword_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_sync_patt_cword_hi_get(uint32_t *cfgsyncpatcwh)
{
    uint32_t reg_tx_sync_patt_cword_hi=0;

#ifdef VALIDATE_PARMS
    if(!cfgsyncpatcwh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_SYNC_PATT_CWORD_HI, reg_tx_sync_patt_cword_hi);

    *cfgsyncpatcwh = RU_FIELD_GET(0, XPCSTX, TX_SYNC_PATT_CWORD_HI, CFGSYNCPATCWH, reg_tx_sync_patt_cword_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_start_burst_del_cword_lo_set(uint32_t cfgstrtbrstdlmtrcwl)
{
    uint32_t reg_tx_start_burst_del_cword_lo=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_start_burst_del_cword_lo = RU_FIELD_SET(0, XPCSTX, TX_START_BURST_DEL_CWORD_LO, CFGSTRTBRSTDLMTRCWL, reg_tx_start_burst_del_cword_lo, cfgstrtbrstdlmtrcwl);

    RU_REG_WRITE(0, XPCSTX, TX_START_BURST_DEL_CWORD_LO, reg_tx_start_burst_del_cword_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_start_burst_del_cword_lo_get(uint32_t *cfgstrtbrstdlmtrcwl)
{
    uint32_t reg_tx_start_burst_del_cword_lo=0;

#ifdef VALIDATE_PARMS
    if(!cfgstrtbrstdlmtrcwl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_START_BURST_DEL_CWORD_LO, reg_tx_start_burst_del_cword_lo);

    *cfgstrtbrstdlmtrcwl = RU_FIELD_GET(0, XPCSTX, TX_START_BURST_DEL_CWORD_LO, CFGSTRTBRSTDLMTRCWL, reg_tx_start_burst_del_cword_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_start_burst_del_cword_hi_set(uint32_t cfgstrtbrstdlmtrcwh)
{
    uint32_t reg_tx_start_burst_del_cword_hi=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_start_burst_del_cword_hi = RU_FIELD_SET(0, XPCSTX, TX_START_BURST_DEL_CWORD_HI, CFGSTRTBRSTDLMTRCWH, reg_tx_start_burst_del_cword_hi, cfgstrtbrstdlmtrcwh);

    RU_REG_WRITE(0, XPCSTX, TX_START_BURST_DEL_CWORD_HI, reg_tx_start_burst_del_cword_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_start_burst_del_cword_hi_get(uint32_t *cfgstrtbrstdlmtrcwh)
{
    uint32_t reg_tx_start_burst_del_cword_hi=0;

#ifdef VALIDATE_PARMS
    if(!cfgstrtbrstdlmtrcwh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_START_BURST_DEL_CWORD_HI, reg_tx_start_burst_del_cword_hi);

    *cfgstrtbrstdlmtrcwh = RU_FIELD_GET(0, XPCSTX, TX_START_BURST_DEL_CWORD_HI, CFGSTRTBRSTDLMTRCWH, reg_tx_start_burst_del_cword_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_end_burst_del_cword_lo_set(uint32_t cfgendbrstdlmtrcwl)
{
    uint32_t reg_tx_end_burst_del_cword_lo=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_end_burst_del_cword_lo = RU_FIELD_SET(0, XPCSTX, TX_END_BURST_DEL_CWORD_LO, CFGENDBRSTDLMTRCWL, reg_tx_end_burst_del_cword_lo, cfgendbrstdlmtrcwl);

    RU_REG_WRITE(0, XPCSTX, TX_END_BURST_DEL_CWORD_LO, reg_tx_end_burst_del_cword_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_end_burst_del_cword_lo_get(uint32_t *cfgendbrstdlmtrcwl)
{
    uint32_t reg_tx_end_burst_del_cword_lo=0;

#ifdef VALIDATE_PARMS
    if(!cfgendbrstdlmtrcwl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_END_BURST_DEL_CWORD_LO, reg_tx_end_burst_del_cword_lo);

    *cfgendbrstdlmtrcwl = RU_FIELD_GET(0, XPCSTX, TX_END_BURST_DEL_CWORD_LO, CFGENDBRSTDLMTRCWL, reg_tx_end_burst_del_cword_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_end_burst_del_cword_hi_set(uint32_t cfgendbrstdlmtrcwh)
{
    uint32_t reg_tx_end_burst_del_cword_hi=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_end_burst_del_cword_hi = RU_FIELD_SET(0, XPCSTX, TX_END_BURST_DEL_CWORD_HI, CFGENDBRSTDLMTRCWH, reg_tx_end_burst_del_cword_hi, cfgendbrstdlmtrcwh);

    RU_REG_WRITE(0, XPCSTX, TX_END_BURST_DEL_CWORD_HI, reg_tx_end_burst_del_cword_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_end_burst_del_cword_hi_get(uint32_t *cfgendbrstdlmtrcwh)
{
    uint32_t reg_tx_end_burst_del_cword_hi=0;

#ifdef VALIDATE_PARMS
    if(!cfgendbrstdlmtrcwh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_END_BURST_DEL_CWORD_HI, reg_tx_end_burst_del_cword_hi);

    *cfgendbrstdlmtrcwh = RU_FIELD_GET(0, XPCSTX, TX_END_BURST_DEL_CWORD_HI, CFGENDBRSTDLMTRCWH, reg_tx_end_burst_del_cword_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_idle_cword_lo_set(uint32_t cfgidlecwl)
{
    uint32_t reg_tx_idle_cword_lo=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_idle_cword_lo = RU_FIELD_SET(0, XPCSTX, TX_IDLE_CWORD_LO, CFGIDLECWL, reg_tx_idle_cword_lo, cfgidlecwl);

    RU_REG_WRITE(0, XPCSTX, TX_IDLE_CWORD_LO, reg_tx_idle_cword_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_idle_cword_lo_get(uint32_t *cfgidlecwl)
{
    uint32_t reg_tx_idle_cword_lo=0;

#ifdef VALIDATE_PARMS
    if(!cfgidlecwl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_IDLE_CWORD_LO, reg_tx_idle_cword_lo);

    *cfgidlecwl = RU_FIELD_GET(0, XPCSTX, TX_IDLE_CWORD_LO, CFGIDLECWL, reg_tx_idle_cword_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_idle_cword_hi_set(uint32_t cfgidlecwh)
{
    uint32_t reg_tx_idle_cword_hi=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_idle_cword_hi = RU_FIELD_SET(0, XPCSTX, TX_IDLE_CWORD_HI, CFGIDLECWH, reg_tx_idle_cword_hi, cfgidlecwh);

    RU_REG_WRITE(0, XPCSTX, TX_IDLE_CWORD_HI, reg_tx_idle_cword_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_idle_cword_hi_get(uint32_t *cfgidlecwh)
{
    uint32_t reg_tx_idle_cword_hi=0;

#ifdef VALIDATE_PARMS
    if(!cfgidlecwh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_IDLE_CWORD_HI, reg_tx_idle_cword_hi);

    *cfgidlecwh = RU_FIELD_GET(0, XPCSTX, TX_IDLE_CWORD_HI, CFGIDLECWH, reg_tx_idle_cword_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_burst_patt_cword_lo_set(uint32_t cfgburstpatcwl)
{
    uint32_t reg_tx_burst_patt_cword_lo=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_burst_patt_cword_lo = RU_FIELD_SET(0, XPCSTX, TX_BURST_PATT_CWORD_LO, CFGBURSTPATCWL, reg_tx_burst_patt_cword_lo, cfgburstpatcwl);

    RU_REG_WRITE(0, XPCSTX, TX_BURST_PATT_CWORD_LO, reg_tx_burst_patt_cword_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_burst_patt_cword_lo_get(uint32_t *cfgburstpatcwl)
{
    uint32_t reg_tx_burst_patt_cword_lo=0;

#ifdef VALIDATE_PARMS
    if(!cfgburstpatcwl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_BURST_PATT_CWORD_LO, reg_tx_burst_patt_cword_lo);

    *cfgburstpatcwl = RU_FIELD_GET(0, XPCSTX, TX_BURST_PATT_CWORD_LO, CFGBURSTPATCWL, reg_tx_burst_patt_cword_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_burst_patt_cword_hi_set(uint32_t cfgburstpatcwh)
{
    uint32_t reg_tx_burst_patt_cword_hi=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_burst_patt_cword_hi = RU_FIELD_SET(0, XPCSTX, TX_BURST_PATT_CWORD_HI, CFGBURSTPATCWH, reg_tx_burst_patt_cword_hi, cfgburstpatcwh);

    RU_REG_WRITE(0, XPCSTX, TX_BURST_PATT_CWORD_HI, reg_tx_burst_patt_cword_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_burst_patt_cword_hi_get(uint32_t *cfgburstpatcwh)
{
    uint32_t reg_tx_burst_patt_cword_hi=0;

#ifdef VALIDATE_PARMS
    if(!cfgburstpatcwh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_BURST_PATT_CWORD_HI, reg_tx_burst_patt_cword_hi);

    *cfgburstpatcwh = RU_FIELD_GET(0, XPCSTX, TX_BURST_PATT_CWORD_HI, CFGBURSTPATCWH, reg_tx_burst_patt_cword_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_laser_time_set(uint8_t cfglaserpipe125, uint8_t cfglaseroffdlytq125, uint8_t cfglaserondlytq125)
{
    uint32_t reg_tx_laser_time=0;

#ifdef VALIDATE_PARMS
    if((cfglaserpipe125 >= _6BITS_MAX_VAL_) ||
       (cfglaseroffdlytq125 >= _4BITS_MAX_VAL_) ||
       (cfglaserondlytq125 >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_laser_time = RU_FIELD_SET(0, XPCSTX, TX_LASER_TIME, CFGLASERPIPE125, reg_tx_laser_time, cfglaserpipe125);
    reg_tx_laser_time = RU_FIELD_SET(0, XPCSTX, TX_LASER_TIME, CFGLASEROFFDLYTQ125, reg_tx_laser_time, cfglaseroffdlytq125);
    reg_tx_laser_time = RU_FIELD_SET(0, XPCSTX, TX_LASER_TIME, CFGLASERONDLYTQ125, reg_tx_laser_time, cfglaserondlytq125);

    RU_REG_WRITE(0, XPCSTX, TX_LASER_TIME, reg_tx_laser_time);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_laser_time_get(uint8_t *cfglaserpipe125, uint8_t *cfglaseroffdlytq125, uint8_t *cfglaserondlytq125)
{
    uint32_t reg_tx_laser_time=0;

#ifdef VALIDATE_PARMS
    if(!cfglaserpipe125 || !cfglaseroffdlytq125 || !cfglaserondlytq125)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_LASER_TIME, reg_tx_laser_time);

    *cfglaserpipe125 = RU_FIELD_GET(0, XPCSTX, TX_LASER_TIME, CFGLASERPIPE125, reg_tx_laser_time);
    *cfglaseroffdlytq125 = RU_FIELD_GET(0, XPCSTX, TX_LASER_TIME, CFGLASEROFFDLYTQ125, reg_tx_laser_time);
    *cfglaserondlytq125 = RU_FIELD_GET(0, XPCSTX, TX_LASER_TIME, CFGLASERONDLYTQ125, reg_tx_laser_time);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_mac_mode_set(bdmf_boolean cfgennogntxmt125)
{
    uint32_t reg_tx_mac_mode=0;

#ifdef VALIDATE_PARMS
    if((cfgennogntxmt125 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_mac_mode = RU_FIELD_SET(0, XPCSTX, TX_MAC_MODE, CFGENNOGNTXMT125, reg_tx_mac_mode, cfgennogntxmt125);

    RU_REG_WRITE(0, XPCSTX, TX_MAC_MODE, reg_tx_mac_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_mac_mode_get(bdmf_boolean *cfgennogntxmt125)
{
    uint32_t reg_tx_mac_mode=0;

#ifdef VALIDATE_PARMS
    if(!cfgennogntxmt125)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_MAC_MODE, reg_tx_mac_mode);

    *cfgennogntxmt125 = RU_FIELD_GET(0, XPCSTX, TX_MAC_MODE, CFGENNOGNTXMT125, reg_tx_mac_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_ctl_set(bdmf_boolean laserenstatus, bdmf_boolean cfglsrmonacthi, bdmf_boolean lasermonrstn)
{
    uint32_t reg_tx_laser_monitor_ctl=0;

#ifdef VALIDATE_PARMS
    if((laserenstatus >= _1BITS_MAX_VAL_) ||
       (cfglsrmonacthi >= _1BITS_MAX_VAL_) ||
       (lasermonrstn >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_laser_monitor_ctl = RU_FIELD_SET(0, XPCSTX, TX_LASER_MONITOR_CTL, LASERENSTATUS, reg_tx_laser_monitor_ctl, laserenstatus);
    reg_tx_laser_monitor_ctl = RU_FIELD_SET(0, XPCSTX, TX_LASER_MONITOR_CTL, CFGLSRMONACTHI, reg_tx_laser_monitor_ctl, cfglsrmonacthi);
    reg_tx_laser_monitor_ctl = RU_FIELD_SET(0, XPCSTX, TX_LASER_MONITOR_CTL, LASERMONRSTN, reg_tx_laser_monitor_ctl, lasermonrstn);

    RU_REG_WRITE(0, XPCSTX, TX_LASER_MONITOR_CTL, reg_tx_laser_monitor_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_ctl_get(bdmf_boolean *laserenstatus, bdmf_boolean *cfglsrmonacthi, bdmf_boolean *lasermonrstn)
{
    uint32_t reg_tx_laser_monitor_ctl=0;

#ifdef VALIDATE_PARMS
    if(!laserenstatus || !cfglsrmonacthi || !lasermonrstn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_LASER_MONITOR_CTL, reg_tx_laser_monitor_ctl);

    *laserenstatus = RU_FIELD_GET(0, XPCSTX, TX_LASER_MONITOR_CTL, LASERENSTATUS, reg_tx_laser_monitor_ctl);
    *cfglsrmonacthi = RU_FIELD_GET(0, XPCSTX, TX_LASER_MONITOR_CTL, CFGLSRMONACTHI, reg_tx_laser_monitor_ctl);
    *lasermonrstn = RU_FIELD_GET(0, XPCSTX, TX_LASER_MONITOR_CTL, LASERMONRSTN, reg_tx_laser_monitor_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_max_thresh_set(uint32_t cfglsrmonmaxtq)
{
    uint32_t reg_tx_laser_monitor_max_thresh=0;

#ifdef VALIDATE_PARMS
#endif

    reg_tx_laser_monitor_max_thresh = RU_FIELD_SET(0, XPCSTX, TX_LASER_MONITOR_MAX_THRESH, CFGLSRMONMAXTQ, reg_tx_laser_monitor_max_thresh, cfglsrmonmaxtq);

    RU_REG_WRITE(0, XPCSTX, TX_LASER_MONITOR_MAX_THRESH, reg_tx_laser_monitor_max_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_max_thresh_get(uint32_t *cfglsrmonmaxtq)
{
    uint32_t reg_tx_laser_monitor_max_thresh=0;

#ifdef VALIDATE_PARMS
    if(!cfglsrmonmaxtq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_LASER_MONITOR_MAX_THRESH, reg_tx_laser_monitor_max_thresh);

    *cfglsrmonmaxtq = RU_FIELD_GET(0, XPCSTX, TX_LASER_MONITOR_MAX_THRESH, CFGLSRMONMAXTQ, reg_tx_laser_monitor_max_thresh);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_burst_len_get(uint32_t *laseronlength)
{
    uint32_t reg_tx_laser_monitor_burst_len=0;

#ifdef VALIDATE_PARMS
    if(!laseronlength)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_LASER_MONITOR_BURST_LEN, reg_tx_laser_monitor_burst_len);

    *laseronlength = RU_FIELD_GET(0, XPCSTX, TX_LASER_MONITOR_BURST_LEN, LASERONLENGTH, reg_tx_laser_monitor_burst_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xpcstx_tx_laser_monitor_burst_count_get(uint32_t *burstcnt)
{
    uint32_t reg_tx_laser_monitor_burst_count=0;

#ifdef VALIDATE_PARMS
    if(!burstcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, XPCSTX, TX_LASER_MONITOR_BURST_COUNT, reg_tx_laser_monitor_burst_count);

    *burstcnt = RU_FIELD_GET(0, XPCSTX, TX_LASER_MONITOR_BURST_COUNT, BURSTCNT, reg_tx_laser_monitor_burst_count);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_tx_control,
    BDMF_tx_int_stat,
    BDMF_tx_int_mask,
    BDMF_tx_port_command,
    BDMF_tx_data_port_0,
    BDMF_tx_data_port_1,
    BDMF_tx_data_port_2,
    BDMF_tx_sync_patt_cword_lo,
    BDMF_tx_sync_patt_cword_hi,
    BDMF_tx_start_burst_del_cword_lo,
    BDMF_tx_start_burst_del_cword_hi,
    BDMF_tx_end_burst_del_cword_lo,
    BDMF_tx_end_burst_del_cword_hi,
    BDMF_tx_idle_cword_lo,
    BDMF_tx_idle_cword_hi,
    BDMF_tx_burst_patt_cword_lo,
    BDMF_tx_burst_patt_cword_hi,
    BDMF_tx_laser_time,
    BDMF_tx_mac_mode,
    BDMF_tx_laser_monitor_ctl,
    BDMF_tx_laser_monitor_max_thresh,
    BDMF_tx_laser_monitor_burst_len,
    BDMF_tx_laser_monitor_burst_count,
};

typedef enum
{
    bdmf_address_tx_control,
    bdmf_address_tx_int_stat,
    bdmf_address_tx_int_mask,
    bdmf_address_tx_port_command,
    bdmf_address_tx_data_port_0,
    bdmf_address_tx_data_port_1,
    bdmf_address_tx_data_port_2,
    bdmf_address_tx_sync_patt_cword_lo,
    bdmf_address_tx_sync_patt_cword_hi,
    bdmf_address_tx_start_burst_del_cword_lo,
    bdmf_address_tx_start_burst_del_cword_hi,
    bdmf_address_tx_end_burst_del_cword_lo,
    bdmf_address_tx_end_burst_del_cword_hi,
    bdmf_address_tx_idle_cword_lo,
    bdmf_address_tx_idle_cword_hi,
    bdmf_address_tx_burst_patt_cword_lo,
    bdmf_address_tx_burst_patt_cword_hi,
    bdmf_address_tx_laser_time,
    bdmf_address_tx_mac_mode,
    bdmf_address_tx_laser_monitor_ctl,
    bdmf_address_tx_laser_monitor_max_thresh,
    bdmf_address_tx_laser_monitor_burst_len,
    bdmf_address_tx_laser_monitor_burst_count,
}
bdmf_address;

static int bcm_xpcstx_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_tx_control:
    {
        xpcstx_tx_control tx_control = { .cfgenrmtfaultdet125=parm[1].value.unumber, .cfglsrtristateen125=parm[2].value.unumber, .cfgenseqnum125=parm[3].value.unumber, .cfgenscrmbcont125=parm[4].value.unumber, .cfglsrenacthi125=parm[5].value.unumber, .cfgenlsralways125=parm[6].value.unumber, .cfgenlsrtilendslot125=parm[7].value.unumber, .cfgtxoutbyteflip125=parm[8].value.unumber, .cfgentxout125=parm[9].value.unumber, .cfgentxscrb125=parm[10].value.unumber, .cfgentxfec125=parm[11].value.unumber, .pcstxnotrdy=parm[12].value.unumber, .pcstxdtportrstn=parm[13].value.unumber, .pcstxrstn=parm[14].value.unumber};
        err = ag_drv_xpcstx_tx_control_set(&tx_control);
        break;
    }
    case BDMF_tx_int_stat:
    {
        xpcstx_tx_int_stat tx_int_stat = { .laseronmax=parm[1].value.unumber, .laseroff=parm[2].value.unumber, .grantlagerr=parm[3].value.unumber, .back2backgnt=parm[4].value.unumber, .fecunderrun=parm[5].value.unumber, .gearboxunderrun=parm[6].value.unumber, .gnttooshort=parm[7].value.unumber};
        err = ag_drv_xpcstx_tx_int_stat_set(&tx_int_stat);
        break;
    }
    case BDMF_tx_int_mask:
    {
        xpcstx_tx_int_mask tx_int_mask = { .laseronmaxmask=parm[1].value.unumber, .laseroffmask=parm[2].value.unumber, .grantlagerrmsk=parm[3].value.unumber, .back2bckgntmsk=parm[4].value.unumber, .fecunderrunmsk=parm[5].value.unumber, .gearboxunderrunmsk=parm[6].value.unumber, .gnttooshortmsk=parm[7].value.unumber};
        err = ag_drv_xpcstx_tx_int_mask_set(&tx_int_mask);
        break;
    }
    case BDMF_tx_port_command:
        err = ag_drv_xpcstx_tx_port_command_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case BDMF_tx_data_port_0:
        err = ag_drv_xpcstx_tx_data_port_0_set(parm[1].value.unumber);
        break;
    case BDMF_tx_data_port_1:
        err = ag_drv_xpcstx_tx_data_port_1_set(parm[1].value.unumber);
        break;
    case BDMF_tx_data_port_2:
        err = ag_drv_xpcstx_tx_data_port_2_set(parm[1].value.unumber);
        break;
    case BDMF_tx_sync_patt_cword_lo:
        err = ag_drv_xpcstx_tx_sync_patt_cword_lo_set(parm[1].value.unumber);
        break;
    case BDMF_tx_sync_patt_cword_hi:
        err = ag_drv_xpcstx_tx_sync_patt_cword_hi_set(parm[1].value.unumber);
        break;
    case BDMF_tx_start_burst_del_cword_lo:
        err = ag_drv_xpcstx_tx_start_burst_del_cword_lo_set(parm[1].value.unumber);
        break;
    case BDMF_tx_start_burst_del_cword_hi:
        err = ag_drv_xpcstx_tx_start_burst_del_cword_hi_set(parm[1].value.unumber);
        break;
    case BDMF_tx_end_burst_del_cword_lo:
        err = ag_drv_xpcstx_tx_end_burst_del_cword_lo_set(parm[1].value.unumber);
        break;
    case BDMF_tx_end_burst_del_cword_hi:
        err = ag_drv_xpcstx_tx_end_burst_del_cword_hi_set(parm[1].value.unumber);
        break;
    case BDMF_tx_idle_cword_lo:
        err = ag_drv_xpcstx_tx_idle_cword_lo_set(parm[1].value.unumber);
        break;
    case BDMF_tx_idle_cword_hi:
        err = ag_drv_xpcstx_tx_idle_cword_hi_set(parm[1].value.unumber);
        break;
    case BDMF_tx_burst_patt_cword_lo:
        err = ag_drv_xpcstx_tx_burst_patt_cword_lo_set(parm[1].value.unumber);
        break;
    case BDMF_tx_burst_patt_cword_hi:
        err = ag_drv_xpcstx_tx_burst_patt_cword_hi_set(parm[1].value.unumber);
        break;
    case BDMF_tx_laser_time:
        err = ag_drv_xpcstx_tx_laser_time_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_tx_mac_mode:
        err = ag_drv_xpcstx_tx_mac_mode_set(parm[1].value.unumber);
        break;
    case BDMF_tx_laser_monitor_ctl:
        err = ag_drv_xpcstx_tx_laser_monitor_ctl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_tx_laser_monitor_max_thresh:
        err = ag_drv_xpcstx_tx_laser_monitor_max_thresh_set(parm[1].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_xpcstx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_tx_control:
    {
        xpcstx_tx_control tx_control;
        err = ag_drv_xpcstx_tx_control_get(&tx_control);
        bdmf_session_print(session, "cfgenrmtfaultdet125 = %u = 0x%x\n", tx_control.cfgenrmtfaultdet125, tx_control.cfgenrmtfaultdet125);
        bdmf_session_print(session, "cfglsrtristateen125 = %u = 0x%x\n", tx_control.cfglsrtristateen125, tx_control.cfglsrtristateen125);
        bdmf_session_print(session, "cfgenseqnum125 = %u = 0x%x\n", tx_control.cfgenseqnum125, tx_control.cfgenseqnum125);
        bdmf_session_print(session, "cfgenscrmbcont125 = %u = 0x%x\n", tx_control.cfgenscrmbcont125, tx_control.cfgenscrmbcont125);
        bdmf_session_print(session, "cfglsrenacthi125 = %u = 0x%x\n", tx_control.cfglsrenacthi125, tx_control.cfglsrenacthi125);
        bdmf_session_print(session, "cfgenlsralways125 = %u = 0x%x\n", tx_control.cfgenlsralways125, tx_control.cfgenlsralways125);
        bdmf_session_print(session, "cfgenlsrtilendslot125 = %u = 0x%x\n", tx_control.cfgenlsrtilendslot125, tx_control.cfgenlsrtilendslot125);
        bdmf_session_print(session, "cfgtxoutbyteflip125 = %u = 0x%x\n", tx_control.cfgtxoutbyteflip125, tx_control.cfgtxoutbyteflip125);
        bdmf_session_print(session, "cfgentxout125 = %u = 0x%x\n", tx_control.cfgentxout125, tx_control.cfgentxout125);
        bdmf_session_print(session, "cfgentxscrb125 = %u = 0x%x\n", tx_control.cfgentxscrb125, tx_control.cfgentxscrb125);
        bdmf_session_print(session, "cfgentxfec125 = %u = 0x%x\n", tx_control.cfgentxfec125, tx_control.cfgentxfec125);
        bdmf_session_print(session, "pcstxnotrdy = %u = 0x%x\n", tx_control.pcstxnotrdy, tx_control.pcstxnotrdy);
        bdmf_session_print(session, "pcstxdtportrstn = %u = 0x%x\n", tx_control.pcstxdtportrstn, tx_control.pcstxdtportrstn);
        bdmf_session_print(session, "pcstxrstn = %u = 0x%x\n", tx_control.pcstxrstn, tx_control.pcstxrstn);
        break;
    }
    case BDMF_tx_int_stat:
    {
        xpcstx_tx_int_stat tx_int_stat;
        err = ag_drv_xpcstx_tx_int_stat_get(&tx_int_stat);
        bdmf_session_print(session, "laseronmax = %u = 0x%x\n", tx_int_stat.laseronmax, tx_int_stat.laseronmax);
        bdmf_session_print(session, "laseroff = %u = 0x%x\n", tx_int_stat.laseroff, tx_int_stat.laseroff);
        bdmf_session_print(session, "grantlagerr = %u = 0x%x\n", tx_int_stat.grantlagerr, tx_int_stat.grantlagerr);
        bdmf_session_print(session, "back2backgnt = %u = 0x%x\n", tx_int_stat.back2backgnt, tx_int_stat.back2backgnt);
        bdmf_session_print(session, "fecunderrun = %u = 0x%x\n", tx_int_stat.fecunderrun, tx_int_stat.fecunderrun);
        bdmf_session_print(session, "gearboxunderrun = %u = 0x%x\n", tx_int_stat.gearboxunderrun, tx_int_stat.gearboxunderrun);
        bdmf_session_print(session, "gnttooshort = %u = 0x%x\n", tx_int_stat.gnttooshort, tx_int_stat.gnttooshort);
        break;
    }
    case BDMF_tx_int_mask:
    {
        xpcstx_tx_int_mask tx_int_mask;
        err = ag_drv_xpcstx_tx_int_mask_get(&tx_int_mask);
        bdmf_session_print(session, "laseronmaxmask = %u = 0x%x\n", tx_int_mask.laseronmaxmask, tx_int_mask.laseronmaxmask);
        bdmf_session_print(session, "laseroffmask = %u = 0x%x\n", tx_int_mask.laseroffmask, tx_int_mask.laseroffmask);
        bdmf_session_print(session, "grantlagerrmsk = %u = 0x%x\n", tx_int_mask.grantlagerrmsk, tx_int_mask.grantlagerrmsk);
        bdmf_session_print(session, "back2bckgntmsk = %u = 0x%x\n", tx_int_mask.back2bckgntmsk, tx_int_mask.back2bckgntmsk);
        bdmf_session_print(session, "fecunderrunmsk = %u = 0x%x\n", tx_int_mask.fecunderrunmsk, tx_int_mask.fecunderrunmsk);
        bdmf_session_print(session, "gearboxunderrunmsk = %u = 0x%x\n", tx_int_mask.gearboxunderrunmsk, tx_int_mask.gearboxunderrunmsk);
        bdmf_session_print(session, "gnttooshortmsk = %u = 0x%x\n", tx_int_mask.gnttooshortmsk, tx_int_mask.gnttooshortmsk);
        break;
    }
    case BDMF_tx_port_command:
    {
        bdmf_boolean dataportbusy;
        uint8_t portselect;
        uint8_t portopcode;
        uint16_t portaddress;
        err = ag_drv_xpcstx_tx_port_command_get(&dataportbusy, &portselect, &portopcode, &portaddress);
        bdmf_session_print(session, "dataportbusy = %u = 0x%x\n", dataportbusy, dataportbusy);
        bdmf_session_print(session, "portselect = %u = 0x%x\n", portselect, portselect);
        bdmf_session_print(session, "portopcode = %u = 0x%x\n", portopcode, portopcode);
        bdmf_session_print(session, "portaddress = %u = 0x%x\n", portaddress, portaddress);
        break;
    }
    case BDMF_tx_data_port_0:
    {
        uint32_t portdata0;
        err = ag_drv_xpcstx_tx_data_port_0_get(&portdata0);
        bdmf_session_print(session, "portdata0 = %u = 0x%x\n", portdata0, portdata0);
        break;
    }
    case BDMF_tx_data_port_1:
    {
        uint32_t portdata1;
        err = ag_drv_xpcstx_tx_data_port_1_get(&portdata1);
        bdmf_session_print(session, "portdata1 = %u = 0x%x\n", portdata1, portdata1);
        break;
    }
    case BDMF_tx_data_port_2:
    {
        uint32_t portdata2;
        err = ag_drv_xpcstx_tx_data_port_2_get(&portdata2);
        bdmf_session_print(session, "portdata2 = %u = 0x%x\n", portdata2, portdata2);
        break;
    }
    case BDMF_tx_sync_patt_cword_lo:
    {
        uint32_t cfgsyncpatcwl;
        err = ag_drv_xpcstx_tx_sync_patt_cword_lo_get(&cfgsyncpatcwl);
        bdmf_session_print(session, "cfgsyncpatcwl = %u = 0x%x\n", cfgsyncpatcwl, cfgsyncpatcwl);
        break;
    }
    case BDMF_tx_sync_patt_cword_hi:
    {
        uint32_t cfgsyncpatcwh;
        err = ag_drv_xpcstx_tx_sync_patt_cword_hi_get(&cfgsyncpatcwh);
        bdmf_session_print(session, "cfgsyncpatcwh = %u = 0x%x\n", cfgsyncpatcwh, cfgsyncpatcwh);
        break;
    }
    case BDMF_tx_start_burst_del_cword_lo:
    {
        uint32_t cfgstrtbrstdlmtrcwl;
        err = ag_drv_xpcstx_tx_start_burst_del_cword_lo_get(&cfgstrtbrstdlmtrcwl);
        bdmf_session_print(session, "cfgstrtbrstdlmtrcwl = %u = 0x%x\n", cfgstrtbrstdlmtrcwl, cfgstrtbrstdlmtrcwl);
        break;
    }
    case BDMF_tx_start_burst_del_cword_hi:
    {
        uint32_t cfgstrtbrstdlmtrcwh;
        err = ag_drv_xpcstx_tx_start_burst_del_cword_hi_get(&cfgstrtbrstdlmtrcwh);
        bdmf_session_print(session, "cfgstrtbrstdlmtrcwh = %u = 0x%x\n", cfgstrtbrstdlmtrcwh, cfgstrtbrstdlmtrcwh);
        break;
    }
    case BDMF_tx_end_burst_del_cword_lo:
    {
        uint32_t cfgendbrstdlmtrcwl;
        err = ag_drv_xpcstx_tx_end_burst_del_cword_lo_get(&cfgendbrstdlmtrcwl);
        bdmf_session_print(session, "cfgendbrstdlmtrcwl = %u = 0x%x\n", cfgendbrstdlmtrcwl, cfgendbrstdlmtrcwl);
        break;
    }
    case BDMF_tx_end_burst_del_cword_hi:
    {
        uint32_t cfgendbrstdlmtrcwh;
        err = ag_drv_xpcstx_tx_end_burst_del_cword_hi_get(&cfgendbrstdlmtrcwh);
        bdmf_session_print(session, "cfgendbrstdlmtrcwh = %u = 0x%x\n", cfgendbrstdlmtrcwh, cfgendbrstdlmtrcwh);
        break;
    }
    case BDMF_tx_idle_cword_lo:
    {
        uint32_t cfgidlecwl;
        err = ag_drv_xpcstx_tx_idle_cword_lo_get(&cfgidlecwl);
        bdmf_session_print(session, "cfgidlecwl = %u = 0x%x\n", cfgidlecwl, cfgidlecwl);
        break;
    }
    case BDMF_tx_idle_cword_hi:
    {
        uint32_t cfgidlecwh;
        err = ag_drv_xpcstx_tx_idle_cword_hi_get(&cfgidlecwh);
        bdmf_session_print(session, "cfgidlecwh = %u = 0x%x\n", cfgidlecwh, cfgidlecwh);
        break;
    }
    case BDMF_tx_burst_patt_cword_lo:
    {
        uint32_t cfgburstpatcwl;
        err = ag_drv_xpcstx_tx_burst_patt_cword_lo_get(&cfgburstpatcwl);
        bdmf_session_print(session, "cfgburstpatcwl = %u = 0x%x\n", cfgburstpatcwl, cfgburstpatcwl);
        break;
    }
    case BDMF_tx_burst_patt_cword_hi:
    {
        uint32_t cfgburstpatcwh;
        err = ag_drv_xpcstx_tx_burst_patt_cword_hi_get(&cfgburstpatcwh);
        bdmf_session_print(session, "cfgburstpatcwh = %u = 0x%x\n", cfgburstpatcwh, cfgburstpatcwh);
        break;
    }
    case BDMF_tx_laser_time:
    {
        uint8_t cfglaserpipe125;
        uint8_t cfglaseroffdlytq125;
        uint8_t cfglaserondlytq125;
        err = ag_drv_xpcstx_tx_laser_time_get(&cfglaserpipe125, &cfglaseroffdlytq125, &cfglaserondlytq125);
        bdmf_session_print(session, "cfglaserpipe125 = %u = 0x%x\n", cfglaserpipe125, cfglaserpipe125);
        bdmf_session_print(session, "cfglaseroffdlytq125 = %u = 0x%x\n", cfglaseroffdlytq125, cfglaseroffdlytq125);
        bdmf_session_print(session, "cfglaserondlytq125 = %u = 0x%x\n", cfglaserondlytq125, cfglaserondlytq125);
        break;
    }
    case BDMF_tx_mac_mode:
    {
        bdmf_boolean cfgennogntxmt125;
        err = ag_drv_xpcstx_tx_mac_mode_get(&cfgennogntxmt125);
        bdmf_session_print(session, "cfgennogntxmt125 = %u = 0x%x\n", cfgennogntxmt125, cfgennogntxmt125);
        break;
    }
    case BDMF_tx_laser_monitor_ctl:
    {
        bdmf_boolean laserenstatus;
        bdmf_boolean cfglsrmonacthi;
        bdmf_boolean lasermonrstn;
        err = ag_drv_xpcstx_tx_laser_monitor_ctl_get(&laserenstatus, &cfglsrmonacthi, &lasermonrstn);
        bdmf_session_print(session, "laserenstatus = %u = 0x%x\n", laserenstatus, laserenstatus);
        bdmf_session_print(session, "cfglsrmonacthi = %u = 0x%x\n", cfglsrmonacthi, cfglsrmonacthi);
        bdmf_session_print(session, "lasermonrstn = %u = 0x%x\n", lasermonrstn, lasermonrstn);
        break;
    }
    case BDMF_tx_laser_monitor_max_thresh:
    {
        uint32_t cfglsrmonmaxtq;
        err = ag_drv_xpcstx_tx_laser_monitor_max_thresh_get(&cfglsrmonmaxtq);
        bdmf_session_print(session, "cfglsrmonmaxtq = %u = 0x%x\n", cfglsrmonmaxtq, cfglsrmonmaxtq);
        break;
    }
    case BDMF_tx_laser_monitor_burst_len:
    {
        uint32_t laseronlength;
        err = ag_drv_xpcstx_tx_laser_monitor_burst_len_get(&laseronlength);
        bdmf_session_print(session, "laseronlength = %u = 0x%x\n", laseronlength, laseronlength);
        break;
    }
    case BDMF_tx_laser_monitor_burst_count:
    {
        uint32_t burstcnt;
        err = ag_drv_xpcstx_tx_laser_monitor_burst_count_get(&burstcnt);
        bdmf_session_print(session, "burstcnt = %u = 0x%x\n", burstcnt, burstcnt);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_xpcstx_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        xpcstx_tx_control tx_control = {.cfgenrmtfaultdet125=gtmv(m, 1), .cfglsrtristateen125=gtmv(m, 1), .cfgenseqnum125=gtmv(m, 1), .cfgenscrmbcont125=gtmv(m, 1), .cfglsrenacthi125=gtmv(m, 1), .cfgenlsralways125=gtmv(m, 1), .cfgenlsrtilendslot125=gtmv(m, 1), .cfgtxoutbyteflip125=gtmv(m, 1), .cfgentxout125=gtmv(m, 1), .cfgentxscrb125=gtmv(m, 1), .cfgentxfec125=gtmv(m, 1), .pcstxnotrdy=gtmv(m, 1), .pcstxdtportrstn=gtmv(m, 1), .pcstxrstn=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_control_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", tx_control.cfgenrmtfaultdet125, tx_control.cfglsrtristateen125, tx_control.cfgenseqnum125, tx_control.cfgenscrmbcont125, tx_control.cfglsrenacthi125, tx_control.cfgenlsralways125, tx_control.cfgenlsrtilendslot125, tx_control.cfgtxoutbyteflip125, tx_control.cfgentxout125, tx_control.cfgentxscrb125, tx_control.cfgentxfec125, tx_control.pcstxnotrdy, tx_control.pcstxdtportrstn, tx_control.pcstxrstn);
        if(!err) ag_drv_xpcstx_tx_control_set(&tx_control);
        if(!err) ag_drv_xpcstx_tx_control_get( &tx_control);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_control_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", tx_control.cfgenrmtfaultdet125, tx_control.cfglsrtristateen125, tx_control.cfgenseqnum125, tx_control.cfgenscrmbcont125, tx_control.cfglsrenacthi125, tx_control.cfgenlsralways125, tx_control.cfgenlsrtilendslot125, tx_control.cfgtxoutbyteflip125, tx_control.cfgentxout125, tx_control.cfgentxscrb125, tx_control.cfgentxfec125, tx_control.pcstxnotrdy, tx_control.pcstxdtportrstn, tx_control.pcstxrstn);
        if(err || tx_control.cfgenrmtfaultdet125!=gtmv(m, 1) || tx_control.cfglsrtristateen125!=gtmv(m, 1) || tx_control.cfgenseqnum125!=gtmv(m, 1) || tx_control.cfgenscrmbcont125!=gtmv(m, 1) || tx_control.cfglsrenacthi125!=gtmv(m, 1) || tx_control.cfgenlsralways125!=gtmv(m, 1) || tx_control.cfgenlsrtilendslot125!=gtmv(m, 1) || tx_control.cfgtxoutbyteflip125!=gtmv(m, 1) || tx_control.cfgentxout125!=gtmv(m, 1) || tx_control.cfgentxscrb125!=gtmv(m, 1) || tx_control.cfgentxfec125!=gtmv(m, 1) || tx_control.pcstxnotrdy!=gtmv(m, 1) || tx_control.pcstxdtportrstn!=gtmv(m, 1) || tx_control.pcstxrstn!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xpcstx_tx_int_stat tx_int_stat = {.laseronmax=gtmv(m, 1), .laseroff=gtmv(m, 1), .grantlagerr=gtmv(m, 1), .back2backgnt=gtmv(m, 1), .fecunderrun=gtmv(m, 1), .gearboxunderrun=gtmv(m, 1), .gnttooshort=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_int_stat_set( %u %u %u %u %u %u %u)\n", tx_int_stat.laseronmax, tx_int_stat.laseroff, tx_int_stat.grantlagerr, tx_int_stat.back2backgnt, tx_int_stat.fecunderrun, tx_int_stat.gearboxunderrun, tx_int_stat.gnttooshort);
        if(!err) ag_drv_xpcstx_tx_int_stat_set(&tx_int_stat);
        if(!err) ag_drv_xpcstx_tx_int_stat_get( &tx_int_stat);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_int_stat_get( %u %u %u %u %u %u %u)\n", tx_int_stat.laseronmax, tx_int_stat.laseroff, tx_int_stat.grantlagerr, tx_int_stat.back2backgnt, tx_int_stat.fecunderrun, tx_int_stat.gearboxunderrun, tx_int_stat.gnttooshort);
        if(err || tx_int_stat.laseronmax!=gtmv(m, 1) || tx_int_stat.laseroff!=gtmv(m, 1) || tx_int_stat.grantlagerr!=gtmv(m, 1) || tx_int_stat.back2backgnt!=gtmv(m, 1) || tx_int_stat.fecunderrun!=gtmv(m, 1) || tx_int_stat.gearboxunderrun!=gtmv(m, 1) || tx_int_stat.gnttooshort!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        xpcstx_tx_int_mask tx_int_mask = {.laseronmaxmask=gtmv(m, 1), .laseroffmask=gtmv(m, 1), .grantlagerrmsk=gtmv(m, 1), .back2bckgntmsk=gtmv(m, 1), .fecunderrunmsk=gtmv(m, 1), .gearboxunderrunmsk=gtmv(m, 1), .gnttooshortmsk=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_int_mask_set( %u %u %u %u %u %u %u)\n", tx_int_mask.laseronmaxmask, tx_int_mask.laseroffmask, tx_int_mask.grantlagerrmsk, tx_int_mask.back2bckgntmsk, tx_int_mask.fecunderrunmsk, tx_int_mask.gearboxunderrunmsk, tx_int_mask.gnttooshortmsk);
        if(!err) ag_drv_xpcstx_tx_int_mask_set(&tx_int_mask);
        if(!err) ag_drv_xpcstx_tx_int_mask_get( &tx_int_mask);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_int_mask_get( %u %u %u %u %u %u %u)\n", tx_int_mask.laseronmaxmask, tx_int_mask.laseroffmask, tx_int_mask.grantlagerrmsk, tx_int_mask.back2bckgntmsk, tx_int_mask.fecunderrunmsk, tx_int_mask.gearboxunderrunmsk, tx_int_mask.gnttooshortmsk);
        if(err || tx_int_mask.laseronmaxmask!=gtmv(m, 1) || tx_int_mask.laseroffmask!=gtmv(m, 1) || tx_int_mask.grantlagerrmsk!=gtmv(m, 1) || tx_int_mask.back2bckgntmsk!=gtmv(m, 1) || tx_int_mask.fecunderrunmsk!=gtmv(m, 1) || tx_int_mask.gearboxunderrunmsk!=gtmv(m, 1) || tx_int_mask.gnttooshortmsk!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean dataportbusy=gtmv(m, 1);
        uint8_t portselect=gtmv(m, 6);
        uint8_t portopcode=gtmv(m, 8);
        uint16_t portaddress=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_port_command_set( %u %u %u %u)\n", dataportbusy, portselect, portopcode, portaddress);
        if(!err) ag_drv_xpcstx_tx_port_command_set(dataportbusy, portselect, portopcode, portaddress);
        if(!err) ag_drv_xpcstx_tx_port_command_get( &dataportbusy, &portselect, &portopcode, &portaddress);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_port_command_get( %u %u %u %u)\n", dataportbusy, portselect, portopcode, portaddress);
        if(err || dataportbusy!=gtmv(m, 1) || portselect!=gtmv(m, 6) || portopcode!=gtmv(m, 8) || portaddress!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t portdata0=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_data_port_0_set( %u)\n", portdata0);
        if(!err) ag_drv_xpcstx_tx_data_port_0_set(portdata0);
        if(!err) ag_drv_xpcstx_tx_data_port_0_get( &portdata0);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_data_port_0_get( %u)\n", portdata0);
        if(err || portdata0!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t portdata1=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_data_port_1_set( %u)\n", portdata1);
        if(!err) ag_drv_xpcstx_tx_data_port_1_set(portdata1);
        if(!err) ag_drv_xpcstx_tx_data_port_1_get( &portdata1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_data_port_1_get( %u)\n", portdata1);
        if(err || portdata1!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t portdata2=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_data_port_2_set( %u)\n", portdata2);
        if(!err) ag_drv_xpcstx_tx_data_port_2_set(portdata2);
        if(!err) ag_drv_xpcstx_tx_data_port_2_get( &portdata2);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_data_port_2_get( %u)\n", portdata2);
        if(err || portdata2!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgsyncpatcwl=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_sync_patt_cword_lo_set( %u)\n", cfgsyncpatcwl);
        if(!err) ag_drv_xpcstx_tx_sync_patt_cword_lo_set(cfgsyncpatcwl);
        if(!err) ag_drv_xpcstx_tx_sync_patt_cword_lo_get( &cfgsyncpatcwl);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_sync_patt_cword_lo_get( %u)\n", cfgsyncpatcwl);
        if(err || cfgsyncpatcwl!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgsyncpatcwh=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_sync_patt_cword_hi_set( %u)\n", cfgsyncpatcwh);
        if(!err) ag_drv_xpcstx_tx_sync_patt_cword_hi_set(cfgsyncpatcwh);
        if(!err) ag_drv_xpcstx_tx_sync_patt_cword_hi_get( &cfgsyncpatcwh);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_sync_patt_cword_hi_get( %u)\n", cfgsyncpatcwh);
        if(err || cfgsyncpatcwh!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgstrtbrstdlmtrcwl=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_start_burst_del_cword_lo_set( %u)\n", cfgstrtbrstdlmtrcwl);
        if(!err) ag_drv_xpcstx_tx_start_burst_del_cword_lo_set(cfgstrtbrstdlmtrcwl);
        if(!err) ag_drv_xpcstx_tx_start_burst_del_cword_lo_get( &cfgstrtbrstdlmtrcwl);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_start_burst_del_cword_lo_get( %u)\n", cfgstrtbrstdlmtrcwl);
        if(err || cfgstrtbrstdlmtrcwl!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgstrtbrstdlmtrcwh=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_start_burst_del_cword_hi_set( %u)\n", cfgstrtbrstdlmtrcwh);
        if(!err) ag_drv_xpcstx_tx_start_burst_del_cword_hi_set(cfgstrtbrstdlmtrcwh);
        if(!err) ag_drv_xpcstx_tx_start_burst_del_cword_hi_get( &cfgstrtbrstdlmtrcwh);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_start_burst_del_cword_hi_get( %u)\n", cfgstrtbrstdlmtrcwh);
        if(err || cfgstrtbrstdlmtrcwh!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgendbrstdlmtrcwl=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_end_burst_del_cword_lo_set( %u)\n", cfgendbrstdlmtrcwl);
        if(!err) ag_drv_xpcstx_tx_end_burst_del_cword_lo_set(cfgendbrstdlmtrcwl);
        if(!err) ag_drv_xpcstx_tx_end_burst_del_cword_lo_get( &cfgendbrstdlmtrcwl);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_end_burst_del_cword_lo_get( %u)\n", cfgendbrstdlmtrcwl);
        if(err || cfgendbrstdlmtrcwl!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgendbrstdlmtrcwh=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_end_burst_del_cword_hi_set( %u)\n", cfgendbrstdlmtrcwh);
        if(!err) ag_drv_xpcstx_tx_end_burst_del_cword_hi_set(cfgendbrstdlmtrcwh);
        if(!err) ag_drv_xpcstx_tx_end_burst_del_cword_hi_get( &cfgendbrstdlmtrcwh);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_end_burst_del_cword_hi_get( %u)\n", cfgendbrstdlmtrcwh);
        if(err || cfgendbrstdlmtrcwh!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgidlecwl=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_idle_cword_lo_set( %u)\n", cfgidlecwl);
        if(!err) ag_drv_xpcstx_tx_idle_cword_lo_set(cfgidlecwl);
        if(!err) ag_drv_xpcstx_tx_idle_cword_lo_get( &cfgidlecwl);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_idle_cword_lo_get( %u)\n", cfgidlecwl);
        if(err || cfgidlecwl!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgidlecwh=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_idle_cword_hi_set( %u)\n", cfgidlecwh);
        if(!err) ag_drv_xpcstx_tx_idle_cword_hi_set(cfgidlecwh);
        if(!err) ag_drv_xpcstx_tx_idle_cword_hi_get( &cfgidlecwh);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_idle_cword_hi_get( %u)\n", cfgidlecwh);
        if(err || cfgidlecwh!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgburstpatcwl=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_burst_patt_cword_lo_set( %u)\n", cfgburstpatcwl);
        if(!err) ag_drv_xpcstx_tx_burst_patt_cword_lo_set(cfgburstpatcwl);
        if(!err) ag_drv_xpcstx_tx_burst_patt_cword_lo_get( &cfgburstpatcwl);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_burst_patt_cword_lo_get( %u)\n", cfgburstpatcwl);
        if(err || cfgburstpatcwl!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgburstpatcwh=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_burst_patt_cword_hi_set( %u)\n", cfgburstpatcwh);
        if(!err) ag_drv_xpcstx_tx_burst_patt_cword_hi_set(cfgburstpatcwh);
        if(!err) ag_drv_xpcstx_tx_burst_patt_cword_hi_get( &cfgburstpatcwh);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_burst_patt_cword_hi_get( %u)\n", cfgburstpatcwh);
        if(err || cfgburstpatcwh!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cfglaserpipe125=gtmv(m, 6);
        uint8_t cfglaseroffdlytq125=gtmv(m, 4);
        uint8_t cfglaserondlytq125=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_laser_time_set( %u %u %u)\n", cfglaserpipe125, cfglaseroffdlytq125, cfglaserondlytq125);
        if(!err) ag_drv_xpcstx_tx_laser_time_set(cfglaserpipe125, cfglaseroffdlytq125, cfglaserondlytq125);
        if(!err) ag_drv_xpcstx_tx_laser_time_get( &cfglaserpipe125, &cfglaseroffdlytq125, &cfglaserondlytq125);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_laser_time_get( %u %u %u)\n", cfglaserpipe125, cfglaseroffdlytq125, cfglaserondlytq125);
        if(err || cfglaserpipe125!=gtmv(m, 6) || cfglaseroffdlytq125!=gtmv(m, 4) || cfglaserondlytq125!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgennogntxmt125=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_mac_mode_set( %u)\n", cfgennogntxmt125);
        if(!err) ag_drv_xpcstx_tx_mac_mode_set(cfgennogntxmt125);
        if(!err) ag_drv_xpcstx_tx_mac_mode_get( &cfgennogntxmt125);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_mac_mode_get( %u)\n", cfgennogntxmt125);
        if(err || cfgennogntxmt125!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean laserenstatus=gtmv(m, 1);
        bdmf_boolean cfglsrmonacthi=gtmv(m, 1);
        bdmf_boolean lasermonrstn=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_laser_monitor_ctl_set( %u %u %u)\n", laserenstatus, cfglsrmonacthi, lasermonrstn);
        if(!err) ag_drv_xpcstx_tx_laser_monitor_ctl_set(laserenstatus, cfglsrmonacthi, lasermonrstn);
        if(!err) ag_drv_xpcstx_tx_laser_monitor_ctl_get( &laserenstatus, &cfglsrmonacthi, &lasermonrstn);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_laser_monitor_ctl_get( %u %u %u)\n", laserenstatus, cfglsrmonacthi, lasermonrstn);
        if(err || laserenstatus!=gtmv(m, 1) || cfglsrmonacthi!=gtmv(m, 1) || lasermonrstn!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfglsrmonmaxtq=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_laser_monitor_max_thresh_set( %u)\n", cfglsrmonmaxtq);
        if(!err) ag_drv_xpcstx_tx_laser_monitor_max_thresh_set(cfglsrmonmaxtq);
        if(!err) ag_drv_xpcstx_tx_laser_monitor_max_thresh_get( &cfglsrmonmaxtq);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_laser_monitor_max_thresh_get( %u)\n", cfglsrmonmaxtq);
        if(err || cfglsrmonmaxtq!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t laseronlength=gtmv(m, 32);
        if(!err) ag_drv_xpcstx_tx_laser_monitor_burst_len_get( &laseronlength);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_laser_monitor_burst_len_get( %u)\n", laseronlength);
    }
    {
        uint32_t burstcnt=gtmv(m, 32);
        if(!err) ag_drv_xpcstx_tx_laser_monitor_burst_count_get( &burstcnt);
        if(!err) bdmf_session_print(session, "ag_drv_xpcstx_tx_laser_monitor_burst_count_get( %u)\n", burstcnt);
    }
    return err;
}

static int bcm_xpcstx_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t i;
    uint32_t j;
    uint32_t index1_start=0;
    uint32_t index1_stop;
    uint32_t index2_start=0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t * bdmf_parm;
    const ru_reg_rec * reg;
    const ru_block_rec * blk;
    const char * enum_string = bdmfmon_enum_parm_stringval(session, 0, parm[0].value.unumber);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_tx_control : reg = &RU_REG(XPCSTX, TX_CONTROL); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_int_stat : reg = &RU_REG(XPCSTX, TX_INT_STAT); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_int_mask : reg = &RU_REG(XPCSTX, TX_INT_MASK); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_port_command : reg = &RU_REG(XPCSTX, TX_PORT_COMMAND); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_data_port_0 : reg = &RU_REG(XPCSTX, TX_DATA_PORT_0); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_data_port_1 : reg = &RU_REG(XPCSTX, TX_DATA_PORT_1); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_data_port_2 : reg = &RU_REG(XPCSTX, TX_DATA_PORT_2); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_sync_patt_cword_lo : reg = &RU_REG(XPCSTX, TX_SYNC_PATT_CWORD_LO); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_sync_patt_cword_hi : reg = &RU_REG(XPCSTX, TX_SYNC_PATT_CWORD_HI); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_start_burst_del_cword_lo : reg = &RU_REG(XPCSTX, TX_START_BURST_DEL_CWORD_LO); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_start_burst_del_cword_hi : reg = &RU_REG(XPCSTX, TX_START_BURST_DEL_CWORD_HI); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_end_burst_del_cword_lo : reg = &RU_REG(XPCSTX, TX_END_BURST_DEL_CWORD_LO); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_end_burst_del_cword_hi : reg = &RU_REG(XPCSTX, TX_END_BURST_DEL_CWORD_HI); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_idle_cword_lo : reg = &RU_REG(XPCSTX, TX_IDLE_CWORD_LO); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_idle_cword_hi : reg = &RU_REG(XPCSTX, TX_IDLE_CWORD_HI); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_burst_patt_cword_lo : reg = &RU_REG(XPCSTX, TX_BURST_PATT_CWORD_LO); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_burst_patt_cword_hi : reg = &RU_REG(XPCSTX, TX_BURST_PATT_CWORD_HI); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_laser_time : reg = &RU_REG(XPCSTX, TX_LASER_TIME); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_mac_mode : reg = &RU_REG(XPCSTX, TX_MAC_MODE); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_laser_monitor_ctl : reg = &RU_REG(XPCSTX, TX_LASER_MONITOR_CTL); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_laser_monitor_max_thresh : reg = &RU_REG(XPCSTX, TX_LASER_MONITOR_MAX_THRESH); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_laser_monitor_burst_len : reg = &RU_REG(XPCSTX, TX_LASER_MONITOR_BURST_LEN); blk = &RU_BLK(XPCSTX); break;
    case bdmf_address_tx_laser_monitor_burst_count : reg = &RU_REG(XPCSTX, TX_LASER_MONITOR_BURST_COUNT); blk = &RU_BLK(XPCSTX); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index1")))
    {
        index1_start = bdmf_parm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index2")))
    {
        index2_start = bdmf_parm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count + 1;
    if(index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if(index2_stop > (reg->ram_count + 1))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count + 1);
        return BDMF_ERR_RANGE;
    }
    if(reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, 	 "(%5u) 0x%08X\n", j, (uint32_t)((uint32_t*)(blk->addr[i] + reg->addr) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, (uint32_t)((uint32_t*)(blk->addr[i] + reg->addr)));
    return 0;
}

bdmfmon_handle_t ag_drv_xpcstx_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "xpcstx"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "xpcstx", "xpcstx", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_tx_control[]={
            BDMFMON_MAKE_PARM("cfgenrmtfaultdet125", "cfgenrmtfaultdet125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfglsrtristateen125", "cfglsrtristateen125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenseqnum125", "cfgenseqnum125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenscrmbcont125", "cfgenscrmbcont125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfglsrenacthi125", "cfglsrenacthi125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenlsralways125", "cfgenlsralways125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenlsrtilendslot125", "cfgenlsrtilendslot125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgtxoutbyteflip125", "cfgtxoutbyteflip125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgentxout125", "cfgentxout125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgentxscrb125", "cfgentxscrb125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgentxfec125", "cfgentxfec125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pcstxnotrdy", "pcstxnotrdy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pcstxdtportrstn", "pcstxdtportrstn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pcstxrstn", "pcstxrstn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_int_stat[]={
            BDMFMON_MAKE_PARM("laseronmax", "laseronmax", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("laseroff", "laseroff", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("grantlagerr", "grantlagerr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("back2backgnt", "back2backgnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fecunderrun", "fecunderrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gearboxunderrun", "gearboxunderrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gnttooshort", "gnttooshort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_int_mask[]={
            BDMFMON_MAKE_PARM("laseronmaxmask", "laseronmaxmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("laseroffmask", "laseroffmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("grantlagerrmsk", "grantlagerrmsk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("back2bckgntmsk", "back2bckgntmsk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fecunderrunmsk", "fecunderrunmsk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gearboxunderrunmsk", "gearboxunderrunmsk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gnttooshortmsk", "gnttooshortmsk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_port_command[]={
            BDMFMON_MAKE_PARM("dataportbusy", "dataportbusy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("portselect", "portselect", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("portopcode", "portopcode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("portaddress", "portaddress", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_data_port_0[]={
            BDMFMON_MAKE_PARM("portdata0", "portdata0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_data_port_1[]={
            BDMFMON_MAKE_PARM("portdata1", "portdata1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_data_port_2[]={
            BDMFMON_MAKE_PARM("portdata2", "portdata2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_sync_patt_cword_lo[]={
            BDMFMON_MAKE_PARM("cfgsyncpatcwl", "cfgsyncpatcwl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_sync_patt_cword_hi[]={
            BDMFMON_MAKE_PARM("cfgsyncpatcwh", "cfgsyncpatcwh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_start_burst_del_cword_lo[]={
            BDMFMON_MAKE_PARM("cfgstrtbrstdlmtrcwl", "cfgstrtbrstdlmtrcwl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_start_burst_del_cword_hi[]={
            BDMFMON_MAKE_PARM("cfgstrtbrstdlmtrcwh", "cfgstrtbrstdlmtrcwh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_end_burst_del_cword_lo[]={
            BDMFMON_MAKE_PARM("cfgendbrstdlmtrcwl", "cfgendbrstdlmtrcwl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_end_burst_del_cword_hi[]={
            BDMFMON_MAKE_PARM("cfgendbrstdlmtrcwh", "cfgendbrstdlmtrcwh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_idle_cword_lo[]={
            BDMFMON_MAKE_PARM("cfgidlecwl", "cfgidlecwl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_idle_cword_hi[]={
            BDMFMON_MAKE_PARM("cfgidlecwh", "cfgidlecwh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_burst_patt_cword_lo[]={
            BDMFMON_MAKE_PARM("cfgburstpatcwl", "cfgburstpatcwl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_burst_patt_cword_hi[]={
            BDMFMON_MAKE_PARM("cfgburstpatcwh", "cfgburstpatcwh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_laser_time[]={
            BDMFMON_MAKE_PARM("cfglaserpipe125", "cfglaserpipe125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfglaseroffdlytq125", "cfglaseroffdlytq125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfglaserondlytq125", "cfglaserondlytq125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_mac_mode[]={
            BDMFMON_MAKE_PARM("cfgennogntxmt125", "cfgennogntxmt125", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_laser_monitor_ctl[]={
            BDMFMON_MAKE_PARM("laserenstatus", "laserenstatus", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfglsrmonacthi", "cfglsrmonacthi", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("lasermonrstn", "lasermonrstn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_laser_monitor_max_thresh[]={
            BDMFMON_MAKE_PARM("cfglsrmonmaxtq", "cfglsrmonmaxtq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="tx_control", .val=BDMF_tx_control, .parms=set_tx_control },
            { .name="tx_int_stat", .val=BDMF_tx_int_stat, .parms=set_tx_int_stat },
            { .name="tx_int_mask", .val=BDMF_tx_int_mask, .parms=set_tx_int_mask },
            { .name="tx_port_command", .val=BDMF_tx_port_command, .parms=set_tx_port_command },
            { .name="tx_data_port_0", .val=BDMF_tx_data_port_0, .parms=set_tx_data_port_0 },
            { .name="tx_data_port_1", .val=BDMF_tx_data_port_1, .parms=set_tx_data_port_1 },
            { .name="tx_data_port_2", .val=BDMF_tx_data_port_2, .parms=set_tx_data_port_2 },
            { .name="tx_sync_patt_cword_lo", .val=BDMF_tx_sync_patt_cword_lo, .parms=set_tx_sync_patt_cword_lo },
            { .name="tx_sync_patt_cword_hi", .val=BDMF_tx_sync_patt_cword_hi, .parms=set_tx_sync_patt_cword_hi },
            { .name="tx_start_burst_del_cword_lo", .val=BDMF_tx_start_burst_del_cword_lo, .parms=set_tx_start_burst_del_cword_lo },
            { .name="tx_start_burst_del_cword_hi", .val=BDMF_tx_start_burst_del_cword_hi, .parms=set_tx_start_burst_del_cword_hi },
            { .name="tx_end_burst_del_cword_lo", .val=BDMF_tx_end_burst_del_cword_lo, .parms=set_tx_end_burst_del_cword_lo },
            { .name="tx_end_burst_del_cword_hi", .val=BDMF_tx_end_burst_del_cword_hi, .parms=set_tx_end_burst_del_cword_hi },
            { .name="tx_idle_cword_lo", .val=BDMF_tx_idle_cword_lo, .parms=set_tx_idle_cword_lo },
            { .name="tx_idle_cword_hi", .val=BDMF_tx_idle_cword_hi, .parms=set_tx_idle_cword_hi },
            { .name="tx_burst_patt_cword_lo", .val=BDMF_tx_burst_patt_cword_lo, .parms=set_tx_burst_patt_cword_lo },
            { .name="tx_burst_patt_cword_hi", .val=BDMF_tx_burst_patt_cword_hi, .parms=set_tx_burst_patt_cword_hi },
            { .name="tx_laser_time", .val=BDMF_tx_laser_time, .parms=set_tx_laser_time },
            { .name="tx_mac_mode", .val=BDMF_tx_mac_mode, .parms=set_tx_mac_mode },
            { .name="tx_laser_monitor_ctl", .val=BDMF_tx_laser_monitor_ctl, .parms=set_tx_laser_monitor_ctl },
            { .name="tx_laser_monitor_max_thresh", .val=BDMF_tx_laser_monitor_max_thresh, .parms=set_tx_laser_monitor_max_thresh },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_xpcstx_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="tx_control", .val=BDMF_tx_control, .parms=set_default },
            { .name="tx_int_stat", .val=BDMF_tx_int_stat, .parms=set_default },
            { .name="tx_int_mask", .val=BDMF_tx_int_mask, .parms=set_default },
            { .name="tx_port_command", .val=BDMF_tx_port_command, .parms=set_default },
            { .name="tx_data_port_0", .val=BDMF_tx_data_port_0, .parms=set_default },
            { .name="tx_data_port_1", .val=BDMF_tx_data_port_1, .parms=set_default },
            { .name="tx_data_port_2", .val=BDMF_tx_data_port_2, .parms=set_default },
            { .name="tx_sync_patt_cword_lo", .val=BDMF_tx_sync_patt_cword_lo, .parms=set_default },
            { .name="tx_sync_patt_cword_hi", .val=BDMF_tx_sync_patt_cword_hi, .parms=set_default },
            { .name="tx_start_burst_del_cword_lo", .val=BDMF_tx_start_burst_del_cword_lo, .parms=set_default },
            { .name="tx_start_burst_del_cword_hi", .val=BDMF_tx_start_burst_del_cword_hi, .parms=set_default },
            { .name="tx_end_burst_del_cword_lo", .val=BDMF_tx_end_burst_del_cword_lo, .parms=set_default },
            { .name="tx_end_burst_del_cword_hi", .val=BDMF_tx_end_burst_del_cword_hi, .parms=set_default },
            { .name="tx_idle_cword_lo", .val=BDMF_tx_idle_cword_lo, .parms=set_default },
            { .name="tx_idle_cword_hi", .val=BDMF_tx_idle_cword_hi, .parms=set_default },
            { .name="tx_burst_patt_cword_lo", .val=BDMF_tx_burst_patt_cword_lo, .parms=set_default },
            { .name="tx_burst_patt_cword_hi", .val=BDMF_tx_burst_patt_cword_hi, .parms=set_default },
            { .name="tx_laser_time", .val=BDMF_tx_laser_time, .parms=set_default },
            { .name="tx_mac_mode", .val=BDMF_tx_mac_mode, .parms=set_default },
            { .name="tx_laser_monitor_ctl", .val=BDMF_tx_laser_monitor_ctl, .parms=set_default },
            { .name="tx_laser_monitor_max_thresh", .val=BDMF_tx_laser_monitor_max_thresh, .parms=set_default },
            { .name="tx_laser_monitor_burst_len", .val=BDMF_tx_laser_monitor_burst_len, .parms=set_default },
            { .name="tx_laser_monitor_burst_count", .val=BDMF_tx_laser_monitor_burst_count, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_xpcstx_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_xpcstx_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="TX_CONTROL" , .val=bdmf_address_tx_control },
            { .name="TX_INT_STAT" , .val=bdmf_address_tx_int_stat },
            { .name="TX_INT_MASK" , .val=bdmf_address_tx_int_mask },
            { .name="TX_PORT_COMMAND" , .val=bdmf_address_tx_port_command },
            { .name="TX_DATA_PORT_0" , .val=bdmf_address_tx_data_port_0 },
            { .name="TX_DATA_PORT_1" , .val=bdmf_address_tx_data_port_1 },
            { .name="TX_DATA_PORT_2" , .val=bdmf_address_tx_data_port_2 },
            { .name="TX_SYNC_PATT_CWORD_LO" , .val=bdmf_address_tx_sync_patt_cword_lo },
            { .name="TX_SYNC_PATT_CWORD_HI" , .val=bdmf_address_tx_sync_patt_cword_hi },
            { .name="TX_START_BURST_DEL_CWORD_LO" , .val=bdmf_address_tx_start_burst_del_cword_lo },
            { .name="TX_START_BURST_DEL_CWORD_HI" , .val=bdmf_address_tx_start_burst_del_cword_hi },
            { .name="TX_END_BURST_DEL_CWORD_LO" , .val=bdmf_address_tx_end_burst_del_cword_lo },
            { .name="TX_END_BURST_DEL_CWORD_HI" , .val=bdmf_address_tx_end_burst_del_cword_hi },
            { .name="TX_IDLE_CWORD_LO" , .val=bdmf_address_tx_idle_cword_lo },
            { .name="TX_IDLE_CWORD_HI" , .val=bdmf_address_tx_idle_cword_hi },
            { .name="TX_BURST_PATT_CWORD_LO" , .val=bdmf_address_tx_burst_patt_cword_lo },
            { .name="TX_BURST_PATT_CWORD_HI" , .val=bdmf_address_tx_burst_patt_cword_hi },
            { .name="TX_LASER_TIME" , .val=bdmf_address_tx_laser_time },
            { .name="TX_MAC_MODE" , .val=bdmf_address_tx_mac_mode },
            { .name="TX_LASER_MONITOR_CTL" , .val=bdmf_address_tx_laser_monitor_ctl },
            { .name="TX_LASER_MONITOR_MAX_THRESH" , .val=bdmf_address_tx_laser_monitor_max_thresh },
            { .name="TX_LASER_MONITOR_BURST_LEN" , .val=bdmf_address_tx_laser_monitor_burst_len },
            { .name="TX_LASER_MONITOR_BURST_COUNT" , .val=bdmf_address_tx_laser_monitor_burst_count },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_xpcstx_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

