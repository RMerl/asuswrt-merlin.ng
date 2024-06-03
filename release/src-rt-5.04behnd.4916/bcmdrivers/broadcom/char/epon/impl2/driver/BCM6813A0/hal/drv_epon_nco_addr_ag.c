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
#include "drv_epon_nco_addr_ag.h"
bdmf_error_t ag_drv_nco_addr_nco_cfg_set(const nco_addr_nco_cfg *nco_cfg)
{
    uint32_t reg_nco_cfg=0;

#ifdef VALIDATE_PARMS
    if(!nco_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((nco_cfg->cfgbypass >= _1BITS_MAX_VAL_) ||
       (nco_cfg->cfgsrcout10mhz >= _2BITS_MAX_VAL_) ||
       (nco_cfg->cfgsrcout >= _2BITS_MAX_VAL_) ||
       (nco_cfg->cfgsrcin >= _2BITS_MAX_VAL_) ||
       (nco_cfg->cfgncoclr >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_nco_cfg = RU_FIELD_SET(0, NCO_ADDR, NCO_CFG, CFGBYPASS, reg_nco_cfg, nco_cfg->cfgbypass);
    reg_nco_cfg = RU_FIELD_SET(0, NCO_ADDR, NCO_CFG, CFGSRCOUT10MHZ, reg_nco_cfg, nco_cfg->cfgsrcout10mhz);
    reg_nco_cfg = RU_FIELD_SET(0, NCO_ADDR, NCO_CFG, CFGSRCOUT, reg_nco_cfg, nco_cfg->cfgsrcout);
    reg_nco_cfg = RU_FIELD_SET(0, NCO_ADDR, NCO_CFG, CFGSRCIN, reg_nco_cfg, nco_cfg->cfgsrcin);
    reg_nco_cfg = RU_FIELD_SET(0, NCO_ADDR, NCO_CFG, CFGNCOCLR, reg_nco_cfg, nco_cfg->cfgncoclr);

    RU_REG_WRITE(0, NCO_ADDR, NCO_CFG, reg_nco_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_cfg_get(nco_addr_nco_cfg *nco_cfg)
{
    uint32_t reg_nco_cfg=0;

#ifdef VALIDATE_PARMS
    if(!nco_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_CFG, reg_nco_cfg);

    nco_cfg->cfgbypass = RU_FIELD_GET(0, NCO_ADDR, NCO_CFG, CFGBYPASS, reg_nco_cfg);
    nco_cfg->cfgsrcout10mhz = RU_FIELD_GET(0, NCO_ADDR, NCO_CFG, CFGSRCOUT10MHZ, reg_nco_cfg);
    nco_cfg->cfgsrcout = RU_FIELD_GET(0, NCO_ADDR, NCO_CFG, CFGSRCOUT, reg_nco_cfg);
    nco_cfg->cfgsrcin = RU_FIELD_GET(0, NCO_ADDR, NCO_CFG, CFGSRCIN, reg_nco_cfg);
    nco_cfg->cfgncoclr = RU_FIELD_GET(0, NCO_ADDR, NCO_CFG, CFGNCOCLR, reg_nco_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_int_set(bdmf_boolean intnoncosync, bdmf_boolean intnoxifpps, bdmf_boolean intnolifpps)
{
    uint32_t reg_nco_int=0;

#ifdef VALIDATE_PARMS
    if((intnoncosync >= _1BITS_MAX_VAL_) ||
       (intnoxifpps >= _1BITS_MAX_VAL_) ||
       (intnolifpps >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_nco_int = RU_FIELD_SET(0, NCO_ADDR, NCO_INT, INTNONCOSYNC, reg_nco_int, intnoncosync);
    reg_nco_int = RU_FIELD_SET(0, NCO_ADDR, NCO_INT, INTNOXIFPPS, reg_nco_int, intnoxifpps);
    reg_nco_int = RU_FIELD_SET(0, NCO_ADDR, NCO_INT, INTNOLIFPPS, reg_nco_int, intnolifpps);

    RU_REG_WRITE(0, NCO_ADDR, NCO_INT, reg_nco_int);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_int_get(bdmf_boolean *intnoncosync, bdmf_boolean *intnoxifpps, bdmf_boolean *intnolifpps)
{
    uint32_t reg_nco_int=0;

#ifdef VALIDATE_PARMS
    if(!intnoncosync || !intnoxifpps || !intnolifpps)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_INT, reg_nco_int);

    *intnoncosync = RU_FIELD_GET(0, NCO_ADDR, NCO_INT, INTNONCOSYNC, reg_nco_int);
    *intnoxifpps = RU_FIELD_GET(0, NCO_ADDR, NCO_INT, INTNOXIFPPS, reg_nco_int);
    *intnolifpps = RU_FIELD_GET(0, NCO_ADDR, NCO_INT, INTNOLIFPPS, reg_nco_int);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_msk_set(bdmf_boolean intnoncosyncmask, bdmf_boolean intnoxifppsmask, bdmf_boolean intnolifppsmask)
{
    uint32_t reg_nco_msk=0;

#ifdef VALIDATE_PARMS
    if((intnoncosyncmask >= _1BITS_MAX_VAL_) ||
       (intnoxifppsmask >= _1BITS_MAX_VAL_) ||
       (intnolifppsmask >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_nco_msk = RU_FIELD_SET(0, NCO_ADDR, NCO_MSK, INTNONCOSYNCMASK, reg_nco_msk, intnoncosyncmask);
    reg_nco_msk = RU_FIELD_SET(0, NCO_ADDR, NCO_MSK, INTNOXIFPPSMASK, reg_nco_msk, intnoxifppsmask);
    reg_nco_msk = RU_FIELD_SET(0, NCO_ADDR, NCO_MSK, INTNOLIFPPSMASK, reg_nco_msk, intnolifppsmask);

    RU_REG_WRITE(0, NCO_ADDR, NCO_MSK, reg_nco_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_msk_get(bdmf_boolean *intnoncosyncmask, bdmf_boolean *intnoxifppsmask, bdmf_boolean *intnolifppsmask)
{
    uint32_t reg_nco_msk=0;

#ifdef VALIDATE_PARMS
    if(!intnoncosyncmask || !intnoxifppsmask || !intnolifppsmask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_MSK, reg_nco_msk);

    *intnoncosyncmask = RU_FIELD_GET(0, NCO_ADDR, NCO_MSK, INTNONCOSYNCMASK, reg_nco_msk);
    *intnoxifppsmask = RU_FIELD_GET(0, NCO_ADDR, NCO_MSK, INTNOXIFPPSMASK, reg_nco_msk);
    *intnolifppsmask = RU_FIELD_GET(0, NCO_ADDR, NCO_MSK, INTNOLIFPPSMASK, reg_nco_msk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_1pps_period_set(uint32_t cfg1ppsperiod)
{
    uint32_t reg_nco_1pps_period=0;

#ifdef VALIDATE_PARMS
    if((cfg1ppsperiod >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_nco_1pps_period = RU_FIELD_SET(0, NCO_ADDR, NCO_1PPS_PERIOD, CFG1PPSPERIOD, reg_nco_1pps_period, cfg1ppsperiod);

    RU_REG_WRITE(0, NCO_ADDR, NCO_1PPS_PERIOD, reg_nco_1pps_period);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_1pps_period_get(uint32_t *cfg1ppsperiod)
{
    uint32_t reg_nco_1pps_period=0;

#ifdef VALIDATE_PARMS
    if(!cfg1ppsperiod)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_1PPS_PERIOD, reg_nco_1pps_period);

    *cfg1ppsperiod = RU_FIELD_GET(0, NCO_ADDR, NCO_1PPS_PERIOD, CFG1PPSPERIOD, reg_nco_1pps_period);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_8khz_period_set(uint32_t cfg8khzperiod)
{
    uint32_t reg_nco_8khz_period=0;

#ifdef VALIDATE_PARMS
    if((cfg8khzperiod >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_nco_8khz_period = RU_FIELD_SET(0, NCO_ADDR, NCO_8KHZ_PERIOD, CFG8KHZPERIOD, reg_nco_8khz_period, cfg8khzperiod);

    RU_REG_WRITE(0, NCO_ADDR, NCO_8KHZ_PERIOD, reg_nco_8khz_period);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_8khz_period_get(uint32_t *cfg8khzperiod)
{
    uint32_t reg_nco_8khz_period=0;

#ifdef VALIDATE_PARMS
    if(!cfg8khzperiod)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_8KHZ_PERIOD, reg_nco_8khz_period);

    *cfg8khzperiod = RU_FIELD_GET(0, NCO_ADDR, NCO_8KHZ_PERIOD, CFG8KHZPERIOD, reg_nco_8khz_period);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_center_frequency_set(uint32_t cfgncodefault)
{
    uint32_t reg_nco_center_frequency=0;

#ifdef VALIDATE_PARMS
#endif

    reg_nco_center_frequency = RU_FIELD_SET(0, NCO_ADDR, NCO_CENTER_FREQUENCY, CFGNCODEFAULT, reg_nco_center_frequency, cfgncodefault);

    RU_REG_WRITE(0, NCO_ADDR, NCO_CENTER_FREQUENCY, reg_nco_center_frequency);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_center_frequency_get(uint32_t *cfgncodefault)
{
    uint32_t reg_nco_center_frequency=0;

#ifdef VALIDATE_PARMS
    if(!cfgncodefault)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_CENTER_FREQUENCY, reg_nco_center_frequency);

    *cfgncodefault = RU_FIELD_GET(0, NCO_ADDR, NCO_CENTER_FREQUENCY, CFGNCODEFAULT, reg_nco_center_frequency);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_int_gain_set(uint16_t cfgncogain)
{
    uint32_t reg_nco_int_gain=0;

#ifdef VALIDATE_PARMS
#endif

    reg_nco_int_gain = RU_FIELD_SET(0, NCO_ADDR, NCO_INT_GAIN, CFGNCOGAIN, reg_nco_int_gain, cfgncogain);

    RU_REG_WRITE(0, NCO_ADDR, NCO_INT_GAIN, reg_nco_int_gain);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_int_gain_get(uint16_t *cfgncogain)
{
    uint32_t reg_nco_int_gain=0;

#ifdef VALIDATE_PARMS
    if(!cfgncogain)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_INT_GAIN, reg_nco_int_gain);

    *cfgncogain = RU_FIELD_GET(0, NCO_ADDR, NCO_INT_GAIN, CFGNCOGAIN, reg_nco_int_gain);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_pro_gain_set(uint16_t cfgncopropgain)
{
    uint32_t reg_nco_pro_gain=0;

#ifdef VALIDATE_PARMS
#endif

    reg_nco_pro_gain = RU_FIELD_SET(0, NCO_ADDR, NCO_PRO_GAIN, CFGNCOPROPGAIN, reg_nco_pro_gain, cfgncopropgain);

    RU_REG_WRITE(0, NCO_ADDR, NCO_PRO_GAIN, reg_nco_pro_gain);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_pro_gain_get(uint16_t *cfgncopropgain)
{
    uint32_t reg_nco_pro_gain=0;

#ifdef VALIDATE_PARMS
    if(!cfgncopropgain)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_PRO_GAIN, reg_nco_pro_gain);

    *cfgncopropgain = RU_FIELD_GET(0, NCO_ADDR, NCO_PRO_GAIN, CFGNCOPROPGAIN, reg_nco_pro_gain);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_cnt_get(uint32_t *ncocnt)
{
    uint32_t reg_nco_cnt=0;

#ifdef VALIDATE_PARMS
    if(!ncocnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_CNT, reg_nco_cnt);

    *ncocnt = RU_FIELD_GET(0, NCO_ADDR, NCO_CNT, NCOCNT, reg_nco_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_1pps_half_set(uint32_t cfg1ppshalfperiod)
{
    uint32_t reg_nco_1pps_half=0;

#ifdef VALIDATE_PARMS
    if((cfg1ppshalfperiod >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_nco_1pps_half = RU_FIELD_SET(0, NCO_ADDR, NCO_1PPS_HALF, CFG1PPSHALFPERIOD, reg_nco_1pps_half, cfg1ppshalfperiod);

    RU_REG_WRITE(0, NCO_ADDR, NCO_1PPS_HALF, reg_nco_1pps_half);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_1pps_half_get(uint32_t *cfg1ppshalfperiod)
{
    uint32_t reg_nco_1pps_half=0;

#ifdef VALIDATE_PARMS
    if(!cfg1ppshalfperiod)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_1PPS_HALF, reg_nco_1pps_half);

    *cfg1ppshalfperiod = RU_FIELD_GET(0, NCO_ADDR, NCO_1PPS_HALF, CFG1PPSHALFPERIOD, reg_nco_1pps_half);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_8khz_half_set(uint32_t cfg8khzhalfperiod)
{
    uint32_t reg_nco_8khz_half=0;

#ifdef VALIDATE_PARMS
    if((cfg8khzhalfperiod >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_nco_8khz_half = RU_FIELD_SET(0, NCO_ADDR, NCO_8KHZ_HALF, CFG8KHZHALFPERIOD, reg_nco_8khz_half, cfg8khzhalfperiod);

    RU_REG_WRITE(0, NCO_ADDR, NCO_8KHZ_HALF, reg_nco_8khz_half);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_8khz_half_get(uint32_t *cfg8khzhalfperiod)
{
    uint32_t reg_nco_8khz_half=0;

#ifdef VALIDATE_PARMS
    if(!cfg8khzhalfperiod)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_8KHZ_HALF, reg_nco_8khz_half);

    *cfg8khzhalfperiod = RU_FIELD_GET(0, NCO_ADDR, NCO_8KHZ_HALF, CFG8KHZHALFPERIOD, reg_nco_8khz_half);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_period_cnt_get(uint32_t *periodcnt)
{
    uint32_t reg_nco_period_cnt=0;

#ifdef VALIDATE_PARMS
    if(!periodcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_PERIOD_CNT, reg_nco_period_cnt);

    *periodcnt = RU_FIELD_GET(0, NCO_ADDR, NCO_PERIOD_CNT, PERIODCNT, reg_nco_period_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_nco_addr_nco_phs_err_cnt_get(uint16_t *ncophserr)
{
    uint32_t reg_nco_phs_err_cnt=0;

#ifdef VALIDATE_PARMS
    if(!ncophserr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NCO_ADDR, NCO_PHS_ERR_CNT, reg_nco_phs_err_cnt);

    *ncophserr = RU_FIELD_GET(0, NCO_ADDR, NCO_PHS_ERR_CNT, NCOPHSERR, reg_nco_phs_err_cnt);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_nco_cfg,
    BDMF_nco_int,
    BDMF_nco_msk,
    BDMF_nco_1pps_period,
    BDMF_nco_8khz_period,
    BDMF_nco_center_frequency,
    BDMF_nco_int_gain,
    BDMF_nco_pro_gain,
    BDMF_nco_cnt,
    BDMF_nco_1pps_half,
    BDMF_nco_8khz_half,
    BDMF_nco_period_cnt,
    BDMF_nco_phs_err_cnt,
};

typedef enum
{
    bdmf_address_nco_cfg,
    bdmf_address_nco_int,
    bdmf_address_nco_msk,
    bdmf_address_nco_1pps_period,
    bdmf_address_nco_8khz_period,
    bdmf_address_nco_center_frequency,
    bdmf_address_nco_int_gain,
    bdmf_address_nco_pro_gain,
    bdmf_address_nco_cnt,
    bdmf_address_nco_1pps_half,
    bdmf_address_nco_8khz_half,
    bdmf_address_nco_period_cnt,
    bdmf_address_nco_phs_err_cnt,
}
bdmf_address;

static int bcm_nco_addr_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_nco_cfg:
    {
        nco_addr_nco_cfg nco_cfg = { .cfgbypass=parm[1].value.unumber, .cfgsrcout10mhz=parm[2].value.unumber, .cfgsrcout=parm[3].value.unumber, .cfgsrcin=parm[4].value.unumber, .cfgncoclr=parm[5].value.unumber};
        err = ag_drv_nco_addr_nco_cfg_set(&nco_cfg);
        break;
    }
    case BDMF_nco_int:
        err = ag_drv_nco_addr_nco_int_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_nco_msk:
        err = ag_drv_nco_addr_nco_msk_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_nco_1pps_period:
        err = ag_drv_nco_addr_nco_1pps_period_set(parm[1].value.unumber);
        break;
    case BDMF_nco_8khz_period:
        err = ag_drv_nco_addr_nco_8khz_period_set(parm[1].value.unumber);
        break;
    case BDMF_nco_center_frequency:
        err = ag_drv_nco_addr_nco_center_frequency_set(parm[1].value.unumber);
        break;
    case BDMF_nco_int_gain:
        err = ag_drv_nco_addr_nco_int_gain_set(parm[1].value.unumber);
        break;
    case BDMF_nco_pro_gain:
        err = ag_drv_nco_addr_nco_pro_gain_set(parm[1].value.unumber);
        break;
    case BDMF_nco_1pps_half:
        err = ag_drv_nco_addr_nco_1pps_half_set(parm[1].value.unumber);
        break;
    case BDMF_nco_8khz_half:
        err = ag_drv_nco_addr_nco_8khz_half_set(parm[1].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_nco_addr_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_nco_cfg:
    {
        nco_addr_nco_cfg nco_cfg;
        err = ag_drv_nco_addr_nco_cfg_get(&nco_cfg);
        bdmf_session_print(session, "cfgbypass = %u = 0x%x\n", nco_cfg.cfgbypass, nco_cfg.cfgbypass);
        bdmf_session_print(session, "cfgsrcout10mhz = %u = 0x%x\n", nco_cfg.cfgsrcout10mhz, nco_cfg.cfgsrcout10mhz);
        bdmf_session_print(session, "cfgsrcout = %u = 0x%x\n", nco_cfg.cfgsrcout, nco_cfg.cfgsrcout);
        bdmf_session_print(session, "cfgsrcin = %u = 0x%x\n", nco_cfg.cfgsrcin, nco_cfg.cfgsrcin);
        bdmf_session_print(session, "cfgncoclr = %u = 0x%x\n", nco_cfg.cfgncoclr, nco_cfg.cfgncoclr);
        break;
    }
    case BDMF_nco_int:
    {
        bdmf_boolean intnoncosync;
        bdmf_boolean intnoxifpps;
        bdmf_boolean intnolifpps;
        err = ag_drv_nco_addr_nco_int_get(&intnoncosync, &intnoxifpps, &intnolifpps);
        bdmf_session_print(session, "intnoncosync = %u = 0x%x\n", intnoncosync, intnoncosync);
        bdmf_session_print(session, "intnoxifpps = %u = 0x%x\n", intnoxifpps, intnoxifpps);
        bdmf_session_print(session, "intnolifpps = %u = 0x%x\n", intnolifpps, intnolifpps);
        break;
    }
    case BDMF_nco_msk:
    {
        bdmf_boolean intnoncosyncmask;
        bdmf_boolean intnoxifppsmask;
        bdmf_boolean intnolifppsmask;
        err = ag_drv_nco_addr_nco_msk_get(&intnoncosyncmask, &intnoxifppsmask, &intnolifppsmask);
        bdmf_session_print(session, "intnoncosyncmask = %u = 0x%x\n", intnoncosyncmask, intnoncosyncmask);
        bdmf_session_print(session, "intnoxifppsmask = %u = 0x%x\n", intnoxifppsmask, intnoxifppsmask);
        bdmf_session_print(session, "intnolifppsmask = %u = 0x%x\n", intnolifppsmask, intnolifppsmask);
        break;
    }
    case BDMF_nco_1pps_period:
    {
        uint32_t cfg1ppsperiod;
        err = ag_drv_nco_addr_nco_1pps_period_get(&cfg1ppsperiod);
        bdmf_session_print(session, "cfg1ppsperiod = %u = 0x%x\n", cfg1ppsperiod, cfg1ppsperiod);
        break;
    }
    case BDMF_nco_8khz_period:
    {
        uint32_t cfg8khzperiod;
        err = ag_drv_nco_addr_nco_8khz_period_get(&cfg8khzperiod);
        bdmf_session_print(session, "cfg8khzperiod = %u = 0x%x\n", cfg8khzperiod, cfg8khzperiod);
        break;
    }
    case BDMF_nco_center_frequency:
    {
        uint32_t cfgncodefault;
        err = ag_drv_nco_addr_nco_center_frequency_get(&cfgncodefault);
        bdmf_session_print(session, "cfgncodefault = %u = 0x%x\n", cfgncodefault, cfgncodefault);
        break;
    }
    case BDMF_nco_int_gain:
    {
        uint16_t cfgncogain;
        err = ag_drv_nco_addr_nco_int_gain_get(&cfgncogain);
        bdmf_session_print(session, "cfgncogain = %u = 0x%x\n", cfgncogain, cfgncogain);
        break;
    }
    case BDMF_nco_pro_gain:
    {
        uint16_t cfgncopropgain;
        err = ag_drv_nco_addr_nco_pro_gain_get(&cfgncopropgain);
        bdmf_session_print(session, "cfgncopropgain = %u = 0x%x\n", cfgncopropgain, cfgncopropgain);
        break;
    }
    case BDMF_nco_cnt:
    {
        uint32_t ncocnt;
        err = ag_drv_nco_addr_nco_cnt_get(&ncocnt);
        bdmf_session_print(session, "ncocnt = %u = 0x%x\n", ncocnt, ncocnt);
        break;
    }
    case BDMF_nco_1pps_half:
    {
        uint32_t cfg1ppshalfperiod;
        err = ag_drv_nco_addr_nco_1pps_half_get(&cfg1ppshalfperiod);
        bdmf_session_print(session, "cfg1ppshalfperiod = %u = 0x%x\n", cfg1ppshalfperiod, cfg1ppshalfperiod);
        break;
    }
    case BDMF_nco_8khz_half:
    {
        uint32_t cfg8khzhalfperiod;
        err = ag_drv_nco_addr_nco_8khz_half_get(&cfg8khzhalfperiod);
        bdmf_session_print(session, "cfg8khzhalfperiod = %u = 0x%x\n", cfg8khzhalfperiod, cfg8khzhalfperiod);
        break;
    }
    case BDMF_nco_period_cnt:
    {
        uint32_t periodcnt;
        err = ag_drv_nco_addr_nco_period_cnt_get(&periodcnt);
        bdmf_session_print(session, "periodcnt = %u = 0x%x\n", periodcnt, periodcnt);
        break;
    }
    case BDMF_nco_phs_err_cnt:
    {
        uint16_t ncophserr;
        err = ag_drv_nco_addr_nco_phs_err_cnt_get(&ncophserr);
        bdmf_session_print(session, "ncophserr = %u = 0x%x\n", ncophserr, ncophserr);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_nco_addr_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        nco_addr_nco_cfg nco_cfg = {.cfgbypass=gtmv(m, 1), .cfgsrcout10mhz=gtmv(m, 2), .cfgsrcout=gtmv(m, 2), .cfgsrcin=gtmv(m, 2), .cfgncoclr=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_cfg_set( %u %u %u %u %u)\n", nco_cfg.cfgbypass, nco_cfg.cfgsrcout10mhz, nco_cfg.cfgsrcout, nco_cfg.cfgsrcin, nco_cfg.cfgncoclr);
        if(!err) ag_drv_nco_addr_nco_cfg_set(&nco_cfg);
        if(!err) ag_drv_nco_addr_nco_cfg_get( &nco_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_cfg_get( %u %u %u %u %u)\n", nco_cfg.cfgbypass, nco_cfg.cfgsrcout10mhz, nco_cfg.cfgsrcout, nco_cfg.cfgsrcin, nco_cfg.cfgncoclr);
        if(err || nco_cfg.cfgbypass!=gtmv(m, 1) || nco_cfg.cfgsrcout10mhz!=gtmv(m, 2) || nco_cfg.cfgsrcout!=gtmv(m, 2) || nco_cfg.cfgsrcin!=gtmv(m, 2) || nco_cfg.cfgncoclr!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean intnoncosync=gtmv(m, 1);
        bdmf_boolean intnoxifpps=gtmv(m, 1);
        bdmf_boolean intnolifpps=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_int_set( %u %u %u)\n", intnoncosync, intnoxifpps, intnolifpps);
        if(!err) ag_drv_nco_addr_nco_int_set(intnoncosync, intnoxifpps, intnolifpps);
        if(!err) ag_drv_nco_addr_nco_int_get( &intnoncosync, &intnoxifpps, &intnolifpps);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_int_get( %u %u %u)\n", intnoncosync, intnoxifpps, intnolifpps);
        if(err || intnoncosync!=gtmv(m, 1) || intnoxifpps!=gtmv(m, 1) || intnolifpps!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean intnoncosyncmask=gtmv(m, 1);
        bdmf_boolean intnoxifppsmask=gtmv(m, 1);
        bdmf_boolean intnolifppsmask=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_msk_set( %u %u %u)\n", intnoncosyncmask, intnoxifppsmask, intnolifppsmask);
        if(!err) ag_drv_nco_addr_nco_msk_set(intnoncosyncmask, intnoxifppsmask, intnolifppsmask);
        if(!err) ag_drv_nco_addr_nco_msk_get( &intnoncosyncmask, &intnoxifppsmask, &intnolifppsmask);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_msk_get( %u %u %u)\n", intnoncosyncmask, intnoxifppsmask, intnolifppsmask);
        if(err || intnoncosyncmask!=gtmv(m, 1) || intnoxifppsmask!=gtmv(m, 1) || intnolifppsmask!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfg1ppsperiod=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_1pps_period_set( %u)\n", cfg1ppsperiod);
        if(!err) ag_drv_nco_addr_nco_1pps_period_set(cfg1ppsperiod);
        if(!err) ag_drv_nco_addr_nco_1pps_period_get( &cfg1ppsperiod);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_1pps_period_get( %u)\n", cfg1ppsperiod);
        if(err || cfg1ppsperiod!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfg8khzperiod=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_8khz_period_set( %u)\n", cfg8khzperiod);
        if(!err) ag_drv_nco_addr_nco_8khz_period_set(cfg8khzperiod);
        if(!err) ag_drv_nco_addr_nco_8khz_period_get( &cfg8khzperiod);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_8khz_period_get( %u)\n", cfg8khzperiod);
        if(err || cfg8khzperiod!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgncodefault=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_center_frequency_set( %u)\n", cfgncodefault);
        if(!err) ag_drv_nco_addr_nco_center_frequency_set(cfgncodefault);
        if(!err) ag_drv_nco_addr_nco_center_frequency_get( &cfgncodefault);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_center_frequency_get( %u)\n", cfgncodefault);
        if(err || cfgncodefault!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgncogain=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_int_gain_set( %u)\n", cfgncogain);
        if(!err) ag_drv_nco_addr_nco_int_gain_set(cfgncogain);
        if(!err) ag_drv_nco_addr_nco_int_gain_get( &cfgncogain);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_int_gain_get( %u)\n", cfgncogain);
        if(err || cfgncogain!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgncopropgain=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_pro_gain_set( %u)\n", cfgncopropgain);
        if(!err) ag_drv_nco_addr_nco_pro_gain_set(cfgncopropgain);
        if(!err) ag_drv_nco_addr_nco_pro_gain_get( &cfgncopropgain);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_pro_gain_get( %u)\n", cfgncopropgain);
        if(err || cfgncopropgain!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t ncocnt=gtmv(m, 32);
        if(!err) ag_drv_nco_addr_nco_cnt_get( &ncocnt);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_cnt_get( %u)\n", ncocnt);
    }
    {
        uint32_t cfg1ppshalfperiod=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_1pps_half_set( %u)\n", cfg1ppshalfperiod);
        if(!err) ag_drv_nco_addr_nco_1pps_half_set(cfg1ppshalfperiod);
        if(!err) ag_drv_nco_addr_nco_1pps_half_get( &cfg1ppshalfperiod);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_1pps_half_get( %u)\n", cfg1ppshalfperiod);
        if(err || cfg1ppshalfperiod!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfg8khzhalfperiod=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_8khz_half_set( %u)\n", cfg8khzhalfperiod);
        if(!err) ag_drv_nco_addr_nco_8khz_half_set(cfg8khzhalfperiod);
        if(!err) ag_drv_nco_addr_nco_8khz_half_get( &cfg8khzhalfperiod);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_8khz_half_get( %u)\n", cfg8khzhalfperiod);
        if(err || cfg8khzhalfperiod!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t periodcnt=gtmv(m, 32);
        if(!err) ag_drv_nco_addr_nco_period_cnt_get( &periodcnt);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_period_cnt_get( %u)\n", periodcnt);
    }
    {
        uint16_t ncophserr=gtmv(m, 12);
        if(!err) ag_drv_nco_addr_nco_phs_err_cnt_get( &ncophserr);
        if(!err) bdmf_session_print(session, "ag_drv_nco_addr_nco_phs_err_cnt_get( %u)\n", ncophserr);
    }
    return err;
}

static int bcm_nco_addr_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_nco_cfg : reg = &RU_REG(NCO_ADDR, NCO_CFG); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_int : reg = &RU_REG(NCO_ADDR, NCO_INT); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_msk : reg = &RU_REG(NCO_ADDR, NCO_MSK); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_1pps_period : reg = &RU_REG(NCO_ADDR, NCO_1PPS_PERIOD); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_8khz_period : reg = &RU_REG(NCO_ADDR, NCO_8KHZ_PERIOD); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_center_frequency : reg = &RU_REG(NCO_ADDR, NCO_CENTER_FREQUENCY); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_int_gain : reg = &RU_REG(NCO_ADDR, NCO_INT_GAIN); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_pro_gain : reg = &RU_REG(NCO_ADDR, NCO_PRO_GAIN); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_cnt : reg = &RU_REG(NCO_ADDR, NCO_CNT); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_1pps_half : reg = &RU_REG(NCO_ADDR, NCO_1PPS_HALF); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_8khz_half : reg = &RU_REG(NCO_ADDR, NCO_8KHZ_HALF); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_period_cnt : reg = &RU_REG(NCO_ADDR, NCO_PERIOD_CNT); blk = &RU_BLK(NCO_ADDR); break;
    case bdmf_address_nco_phs_err_cnt : reg = &RU_REG(NCO_ADDR, NCO_PHS_ERR_CNT); blk = &RU_BLK(NCO_ADDR); break;
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

bdmfmon_handle_t ag_drv_nco_addr_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "nco_addr"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "nco_addr", "nco_addr", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_nco_cfg[]={
            BDMFMON_MAKE_PARM("cfgbypass", "cfgbypass", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgsrcout10mhz", "cfgsrcout10mhz", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgsrcout", "cfgsrcout", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgsrcin", "cfgsrcin", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgncoclr", "cfgncoclr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nco_int[]={
            BDMFMON_MAKE_PARM("intnoncosync", "intnoncosync", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intnoxifpps", "intnoxifpps", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intnolifpps", "intnolifpps", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nco_msk[]={
            BDMFMON_MAKE_PARM("intnoncosyncmask", "intnoncosyncmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intnoxifppsmask", "intnoxifppsmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intnolifppsmask", "intnolifppsmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nco_1pps_period[]={
            BDMFMON_MAKE_PARM("cfg1ppsperiod", "cfg1ppsperiod", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nco_8khz_period[]={
            BDMFMON_MAKE_PARM("cfg8khzperiod", "cfg8khzperiod", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nco_center_frequency[]={
            BDMFMON_MAKE_PARM("cfgncodefault", "cfgncodefault", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nco_int_gain[]={
            BDMFMON_MAKE_PARM("cfgncogain", "cfgncogain", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nco_pro_gain[]={
            BDMFMON_MAKE_PARM("cfgncopropgain", "cfgncopropgain", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nco_1pps_half[]={
            BDMFMON_MAKE_PARM("cfg1ppshalfperiod", "cfg1ppshalfperiod", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_nco_8khz_half[]={
            BDMFMON_MAKE_PARM("cfg8khzhalfperiod", "cfg8khzhalfperiod", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="nco_cfg", .val=BDMF_nco_cfg, .parms=set_nco_cfg },
            { .name="nco_int", .val=BDMF_nco_int, .parms=set_nco_int },
            { .name="nco_msk", .val=BDMF_nco_msk, .parms=set_nco_msk },
            { .name="nco_1pps_period", .val=BDMF_nco_1pps_period, .parms=set_nco_1pps_period },
            { .name="nco_8khz_period", .val=BDMF_nco_8khz_period, .parms=set_nco_8khz_period },
            { .name="nco_center_frequency", .val=BDMF_nco_center_frequency, .parms=set_nco_center_frequency },
            { .name="nco_int_gain", .val=BDMF_nco_int_gain, .parms=set_nco_int_gain },
            { .name="nco_pro_gain", .val=BDMF_nco_pro_gain, .parms=set_nco_pro_gain },
            { .name="nco_1pps_half", .val=BDMF_nco_1pps_half, .parms=set_nco_1pps_half },
            { .name="nco_8khz_half", .val=BDMF_nco_8khz_half, .parms=set_nco_8khz_half },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_nco_addr_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="nco_cfg", .val=BDMF_nco_cfg, .parms=set_default },
            { .name="nco_int", .val=BDMF_nco_int, .parms=set_default },
            { .name="nco_msk", .val=BDMF_nco_msk, .parms=set_default },
            { .name="nco_1pps_period", .val=BDMF_nco_1pps_period, .parms=set_default },
            { .name="nco_8khz_period", .val=BDMF_nco_8khz_period, .parms=set_default },
            { .name="nco_center_frequency", .val=BDMF_nco_center_frequency, .parms=set_default },
            { .name="nco_int_gain", .val=BDMF_nco_int_gain, .parms=set_default },
            { .name="nco_pro_gain", .val=BDMF_nco_pro_gain, .parms=set_default },
            { .name="nco_cnt", .val=BDMF_nco_cnt, .parms=set_default },
            { .name="nco_1pps_half", .val=BDMF_nco_1pps_half, .parms=set_default },
            { .name="nco_8khz_half", .val=BDMF_nco_8khz_half, .parms=set_default },
            { .name="nco_period_cnt", .val=BDMF_nco_period_cnt, .parms=set_default },
            { .name="nco_phs_err_cnt", .val=BDMF_nco_phs_err_cnt, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_nco_addr_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_nco_addr_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="NCO_CFG" , .val=bdmf_address_nco_cfg },
            { .name="NCO_INT" , .val=bdmf_address_nco_int },
            { .name="NCO_MSK" , .val=bdmf_address_nco_msk },
            { .name="NCO_1PPS_PERIOD" , .val=bdmf_address_nco_1pps_period },
            { .name="NCO_8KHZ_PERIOD" , .val=bdmf_address_nco_8khz_period },
            { .name="NCO_CENTER_FREQUENCY" , .val=bdmf_address_nco_center_frequency },
            { .name="NCO_INT_GAIN" , .val=bdmf_address_nco_int_gain },
            { .name="NCO_PRO_GAIN" , .val=bdmf_address_nco_pro_gain },
            { .name="NCO_CNT" , .val=bdmf_address_nco_cnt },
            { .name="NCO_1PPS_HALF" , .val=bdmf_address_nco_1pps_half },
            { .name="NCO_8KHZ_HALF" , .val=bdmf_address_nco_8khz_half },
            { .name="NCO_PERIOD_CNT" , .val=bdmf_address_nco_period_cnt },
            { .name="NCO_PHS_ERR_CNT" , .val=bdmf_address_nco_phs_err_cnt },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_nco_addr_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

