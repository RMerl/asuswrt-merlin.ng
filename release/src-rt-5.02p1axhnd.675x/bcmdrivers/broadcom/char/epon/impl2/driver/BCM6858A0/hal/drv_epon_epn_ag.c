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
#include "drv_epon_epn_ag.h"
bdmf_error_t ag_drv_epn_control_0_set(const epn_control_0 *control_0)
{
    uint32_t reg_control_0=0;

#ifdef VALIDATE_PARMS
    if(!control_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((control_0->cfgen1588ts >= _1BITS_MAX_VAL_) ||
       (control_0->cfgreplaceupfcs >= _1BITS_MAX_VAL_) ||
       (control_0->cfgappendupfcs >= _1BITS_MAX_VAL_) ||
       (control_0->cfgdropscb >= _1BITS_MAX_VAL_) ||
       (control_0->moduncappedreportlimit >= _1BITS_MAX_VAL_) ||
       (control_0->modmpquesetfirst >= _1BITS_MAX_VAL_) ||
       (control_0->prvlocalmpcppropagation >= _1BITS_MAX_VAL_) ||
       (control_0->prvtekmodeprefetch >= _1BITS_MAX_VAL_) ||
       (control_0->prvincnonzeroaccum >= _1BITS_MAX_VAL_) ||
       (control_0->prvnounmapppedfcs >= _1BITS_MAX_VAL_) ||
       (control_0->prvsupressdiscen >= _1BITS_MAX_VAL_) ||
       (control_0->cfgvlanmax >= _1BITS_MAX_VAL_) ||
       (control_0->fcserronlydatafr >= _1BITS_MAX_VAL_) ||
       (control_0->prvdropunmapppedllid >= _1BITS_MAX_VAL_) ||
       (control_0->prvsuppressllidmodebit >= _1BITS_MAX_VAL_) ||
       (control_0->moddiscoverydafilteren >= _1BITS_MAX_VAL_) ||
       (control_0->rptselect >= _2BITS_MAX_VAL_) ||
       (control_0->prvdisablesvaquestatusbp >= _1BITS_MAX_VAL_) ||
       (control_0->utxloopback >= _1BITS_MAX_VAL_) ||
       (control_0->utxen >= _1BITS_MAX_VAL_) ||
       (control_0->utxrst_pre_n >= _1BITS_MAX_VAL_) ||
       (control_0->cfgdisabledns >= _1BITS_MAX_VAL_) ||
       (control_0->drxloopback >= _1BITS_MAX_VAL_) ||
       (control_0->drxen >= _1BITS_MAX_VAL_) ||
       (control_0->drxrst_pre_n >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, CFGEN1588TS, reg_control_0, control_0->cfgen1588ts);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, CFGREPLACEUPFCS, reg_control_0, control_0->cfgreplaceupfcs);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, CFGAPPENDUPFCS, reg_control_0, control_0->cfgappendupfcs);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, CFGDROPSCB, reg_control_0, control_0->cfgdropscb);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, MODUNCAPPEDREPORTLIMIT, reg_control_0, control_0->moduncappedreportlimit);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, MODMPQUESETFIRST, reg_control_0, control_0->modmpquesetfirst);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, PRVLOCALMPCPPROPAGATION, reg_control_0, control_0->prvlocalmpcppropagation);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, PRVTEKMODEPREFETCH, reg_control_0, control_0->prvtekmodeprefetch);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, PRVINCNONZEROACCUM, reg_control_0, control_0->prvincnonzeroaccum);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, PRVNOUNMAPPPEDFCS, reg_control_0, control_0->prvnounmapppedfcs);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, PRVSUPRESSDISCEN, reg_control_0, control_0->prvsupressdiscen);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, CFGVLANMAX, reg_control_0, control_0->cfgvlanmax);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, FCSERRONLYDATAFR, reg_control_0, control_0->fcserronlydatafr);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, PRVDROPUNMAPPPEDLLID, reg_control_0, control_0->prvdropunmapppedllid);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, PRVSUPPRESSLLIDMODEBIT, reg_control_0, control_0->prvsuppressllidmodebit);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, MODDISCOVERYDAFILTEREN, reg_control_0, control_0->moddiscoverydafilteren);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, RPTSELECT, reg_control_0, control_0->rptselect);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, PRVDISABLESVAQUESTATUSBP, reg_control_0, control_0->prvdisablesvaquestatusbp);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, UTXLOOPBACK, reg_control_0, control_0->utxloopback);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, UTXEN, reg_control_0, control_0->utxen);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, UTXRST_PRE_N, reg_control_0, control_0->utxrst_pre_n);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, CFGDISABLEDNS, reg_control_0, control_0->cfgdisabledns);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, DRXLOOPBACK, reg_control_0, control_0->drxloopback);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, DRXEN, reg_control_0, control_0->drxen);
    reg_control_0 = RU_FIELD_SET(0, EPN, CONTROL_0, DRXRST_PRE_N, reg_control_0, control_0->drxrst_pre_n);

    RU_REG_WRITE(0, EPN, CONTROL_0, reg_control_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_control_0_get(epn_control_0 *control_0)
{
    uint32_t reg_control_0=0;

#ifdef VALIDATE_PARMS
    if(!control_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, CONTROL_0, reg_control_0);

    control_0->cfgen1588ts = RU_FIELD_GET(0, EPN, CONTROL_0, CFGEN1588TS, reg_control_0);
    control_0->cfgreplaceupfcs = RU_FIELD_GET(0, EPN, CONTROL_0, CFGREPLACEUPFCS, reg_control_0);
    control_0->cfgappendupfcs = RU_FIELD_GET(0, EPN, CONTROL_0, CFGAPPENDUPFCS, reg_control_0);
    control_0->cfgdropscb = RU_FIELD_GET(0, EPN, CONTROL_0, CFGDROPSCB, reg_control_0);
    control_0->moduncappedreportlimit = RU_FIELD_GET(0, EPN, CONTROL_0, MODUNCAPPEDREPORTLIMIT, reg_control_0);
    control_0->modmpquesetfirst = RU_FIELD_GET(0, EPN, CONTROL_0, MODMPQUESETFIRST, reg_control_0);
    control_0->prvlocalmpcppropagation = RU_FIELD_GET(0, EPN, CONTROL_0, PRVLOCALMPCPPROPAGATION, reg_control_0);
    control_0->prvtekmodeprefetch = RU_FIELD_GET(0, EPN, CONTROL_0, PRVTEKMODEPREFETCH, reg_control_0);
    control_0->prvincnonzeroaccum = RU_FIELD_GET(0, EPN, CONTROL_0, PRVINCNONZEROACCUM, reg_control_0);
    control_0->prvnounmapppedfcs = RU_FIELD_GET(0, EPN, CONTROL_0, PRVNOUNMAPPPEDFCS, reg_control_0);
    control_0->prvsupressdiscen = RU_FIELD_GET(0, EPN, CONTROL_0, PRVSUPRESSDISCEN, reg_control_0);
    control_0->cfgvlanmax = RU_FIELD_GET(0, EPN, CONTROL_0, CFGVLANMAX, reg_control_0);
    control_0->fcserronlydatafr = RU_FIELD_GET(0, EPN, CONTROL_0, FCSERRONLYDATAFR, reg_control_0);
    control_0->prvdropunmapppedllid = RU_FIELD_GET(0, EPN, CONTROL_0, PRVDROPUNMAPPPEDLLID, reg_control_0);
    control_0->prvsuppressllidmodebit = RU_FIELD_GET(0, EPN, CONTROL_0, PRVSUPPRESSLLIDMODEBIT, reg_control_0);
    control_0->moddiscoverydafilteren = RU_FIELD_GET(0, EPN, CONTROL_0, MODDISCOVERYDAFILTEREN, reg_control_0);
    control_0->rptselect = RU_FIELD_GET(0, EPN, CONTROL_0, RPTSELECT, reg_control_0);
    control_0->prvdisablesvaquestatusbp = RU_FIELD_GET(0, EPN, CONTROL_0, PRVDISABLESVAQUESTATUSBP, reg_control_0);
    control_0->utxloopback = RU_FIELD_GET(0, EPN, CONTROL_0, UTXLOOPBACK, reg_control_0);
    control_0->utxen = RU_FIELD_GET(0, EPN, CONTROL_0, UTXEN, reg_control_0);
    control_0->utxrst_pre_n = RU_FIELD_GET(0, EPN, CONTROL_0, UTXRST_PRE_N, reg_control_0);
    control_0->cfgdisabledns = RU_FIELD_GET(0, EPN, CONTROL_0, CFGDISABLEDNS, reg_control_0);
    control_0->drxloopback = RU_FIELD_GET(0, EPN, CONTROL_0, DRXLOOPBACK, reg_control_0);
    control_0->drxen = RU_FIELD_GET(0, EPN, CONTROL_0, DRXEN, reg_control_0);
    control_0->drxrst_pre_n = RU_FIELD_GET(0, EPN, CONTROL_0, DRXRST_PRE_N, reg_control_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_control_1_set(const epn_control_1 *control_1)
{
    uint32_t reg_control_1=0;

#ifdef VALIDATE_PARMS
    if(!control_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((control_1->prvoverlappedgntenable >= _1BITS_MAX_VAL_) ||
       (control_1->rstmisalignthr >= _1BITS_MAX_VAL_) ||
       (control_1->cfgtekrpt >= _1BITS_MAX_VAL_) ||
       (control_1->cfgstalegntchk >= _1BITS_MAX_VAL_) ||
       (control_1->fecrpten >= _1BITS_MAX_VAL_) ||
       (control_1->cfgl1l2truestrict >= _1BITS_MAX_VAL_) ||
       (control_1->cfgctcrpt >= _2BITS_MAX_VAL_) ||
       (control_1->cfgtscorrdis >= _1BITS_MAX_VAL_) ||
       (control_1->cfgnodiscrpt >= _1BITS_MAX_VAL_) ||
       (control_1->disablediscscale >= _1BITS_MAX_VAL_) ||
       (control_1->clronrd >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_control_1 = RU_FIELD_SET(0, EPN, CONTROL_1, PRVOVERLAPPEDGNTENABLE, reg_control_1, control_1->prvoverlappedgntenable);
    reg_control_1 = RU_FIELD_SET(0, EPN, CONTROL_1, RSTMISALIGNTHR, reg_control_1, control_1->rstmisalignthr);
    reg_control_1 = RU_FIELD_SET(0, EPN, CONTROL_1, CFGTEKRPT, reg_control_1, control_1->cfgtekrpt);
    reg_control_1 = RU_FIELD_SET(0, EPN, CONTROL_1, CFGSTALEGNTCHK, reg_control_1, control_1->cfgstalegntchk);
    reg_control_1 = RU_FIELD_SET(0, EPN, CONTROL_1, FECRPTEN, reg_control_1, control_1->fecrpten);
    reg_control_1 = RU_FIELD_SET(0, EPN, CONTROL_1, CFGL1L2TRUESTRICT, reg_control_1, control_1->cfgl1l2truestrict);
    reg_control_1 = RU_FIELD_SET(0, EPN, CONTROL_1, CFGCTCRPT, reg_control_1, control_1->cfgctcrpt);
    reg_control_1 = RU_FIELD_SET(0, EPN, CONTROL_1, CFGTSCORRDIS, reg_control_1, control_1->cfgtscorrdis);
    reg_control_1 = RU_FIELD_SET(0, EPN, CONTROL_1, CFGNODISCRPT, reg_control_1, control_1->cfgnodiscrpt);
    reg_control_1 = RU_FIELD_SET(0, EPN, CONTROL_1, DISABLEDISCSCALE, reg_control_1, control_1->disablediscscale);
    reg_control_1 = RU_FIELD_SET(0, EPN, CONTROL_1, CLRONRD, reg_control_1, control_1->clronrd);

    RU_REG_WRITE(0, EPN, CONTROL_1, reg_control_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_control_1_get(epn_control_1 *control_1)
{
    uint32_t reg_control_1=0;

#ifdef VALIDATE_PARMS
    if(!control_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, CONTROL_1, reg_control_1);

    control_1->prvoverlappedgntenable = RU_FIELD_GET(0, EPN, CONTROL_1, PRVOVERLAPPEDGNTENABLE, reg_control_1);
    control_1->rstmisalignthr = RU_FIELD_GET(0, EPN, CONTROL_1, RSTMISALIGNTHR, reg_control_1);
    control_1->cfgtekrpt = RU_FIELD_GET(0, EPN, CONTROL_1, CFGTEKRPT, reg_control_1);
    control_1->cfgstalegntchk = RU_FIELD_GET(0, EPN, CONTROL_1, CFGSTALEGNTCHK, reg_control_1);
    control_1->fecrpten = RU_FIELD_GET(0, EPN, CONTROL_1, FECRPTEN, reg_control_1);
    control_1->cfgl1l2truestrict = RU_FIELD_GET(0, EPN, CONTROL_1, CFGL1L2TRUESTRICT, reg_control_1);
    control_1->cfgctcrpt = RU_FIELD_GET(0, EPN, CONTROL_1, CFGCTCRPT, reg_control_1);
    control_1->cfgtscorrdis = RU_FIELD_GET(0, EPN, CONTROL_1, CFGTSCORRDIS, reg_control_1);
    control_1->cfgnodiscrpt = RU_FIELD_GET(0, EPN, CONTROL_1, CFGNODISCRPT, reg_control_1);
    control_1->disablediscscale = RU_FIELD_GET(0, EPN, CONTROL_1, DISABLEDISCSCALE, reg_control_1);
    control_1->clronrd = RU_FIELD_GET(0, EPN, CONTROL_1, CLRONRD, reg_control_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_enable_grants_set(uint8_t link_idx, bdmf_boolean grant_en)
{
    uint32_t reg_enable_grants=0;

#ifdef VALIDATE_PARMS
    if((grant_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, ENABLE_GRANTS, reg_enable_grants);

    FIELD_SET(reg_enable_grants, (link_idx % 32) *1, 0x1, grant_en);

    RU_REG_WRITE(0, EPN, ENABLE_GRANTS, reg_enable_grants);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_enable_grants_get(uint8_t link_idx, bdmf_boolean *grant_en)
{
    uint32_t reg_enable_grants=0;

#ifdef VALIDATE_PARMS
    if(!grant_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, ENABLE_GRANTS, reg_enable_grants);

    *grant_en = FIELD_GET(reg_enable_grants, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_drop_disc_gates_set(uint8_t link_idx, bdmf_boolean linkDiscGates_en)
{
    uint32_t reg_drop_disc_gates=0;

#ifdef VALIDATE_PARMS
    if((linkDiscGates_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, DROP_DISC_GATES, reg_drop_disc_gates);

    FIELD_SET(reg_drop_disc_gates, (link_idx % 32) *1, 0x1, linkDiscGates_en);

    RU_REG_WRITE(0, EPN, DROP_DISC_GATES, reg_drop_disc_gates);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_drop_disc_gates_get(uint8_t link_idx, bdmf_boolean *linkDiscGates_en)
{
    uint32_t reg_drop_disc_gates=0;

#ifdef VALIDATE_PARMS
    if(!linkDiscGates_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DROP_DISC_GATES, reg_drop_disc_gates);

    *linkDiscGates_en = FIELD_GET(reg_drop_disc_gates, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dis_fcs_chk_set(uint8_t link_idx, bdmf_boolean disableFcsChk)
{
    uint32_t reg_dis_fcs_chk=0;

#ifdef VALIDATE_PARMS
    if((disableFcsChk >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, DIS_FCS_CHK, reg_dis_fcs_chk);

    FIELD_SET(reg_dis_fcs_chk, (link_idx % 32) *1, 0x1, disableFcsChk);

    RU_REG_WRITE(0, EPN, DIS_FCS_CHK, reg_dis_fcs_chk);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dis_fcs_chk_get(uint8_t link_idx, bdmf_boolean *disableFcsChk)
{
    uint32_t reg_dis_fcs_chk=0;

#ifdef VALIDATE_PARMS
    if(!disableFcsChk)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DIS_FCS_CHK, reg_dis_fcs_chk);

    *disableFcsChk = FIELD_GET(reg_dis_fcs_chk, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_pass_gates_set(uint8_t link_idx, bdmf_boolean passGates)
{
    uint32_t reg_pass_gates=0;

#ifdef VALIDATE_PARMS
    if((passGates >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, PASS_GATES, reg_pass_gates);

    FIELD_SET(reg_pass_gates, (link_idx % 32) *1, 0x1, passGates);

    RU_REG_WRITE(0, EPN, PASS_GATES, reg_pass_gates);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_pass_gates_get(uint8_t link_idx, bdmf_boolean *passGates)
{
    uint32_t reg_pass_gates=0;

#ifdef VALIDATE_PARMS
    if(!passGates)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, PASS_GATES, reg_pass_gates);

    *passGates = FIELD_GET(reg_pass_gates, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_cfg_misalgn_fb_set(uint8_t link_idx, bdmf_boolean cfgMisalignFeedback)
{
    uint32_t reg_cfg_misalgn_fb=0;

#ifdef VALIDATE_PARMS
    if((cfgMisalignFeedback >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, CFG_MISALGN_FB, reg_cfg_misalgn_fb);

    FIELD_SET(reg_cfg_misalgn_fb, (link_idx % 32) *1, 0x1, cfgMisalignFeedback);

    RU_REG_WRITE(0, EPN, CFG_MISALGN_FB, reg_cfg_misalgn_fb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_cfg_misalgn_fb_get(uint8_t link_idx, bdmf_boolean *cfgMisalignFeedback)
{
    uint32_t reg_cfg_misalgn_fb=0;

#ifdef VALIDATE_PARMS
    if(!cfgMisalignFeedback)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, CFG_MISALGN_FB, reg_cfg_misalgn_fb);

    *cfgMisalignFeedback = FIELD_GET(reg_cfg_misalgn_fb, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_discovery_filter_set(uint16_t prvdiscinfomask, uint16_t prvdiscinfovalue)
{
    uint32_t reg_discovery_filter=0;

#ifdef VALIDATE_PARMS
#endif

    reg_discovery_filter = RU_FIELD_SET(0, EPN, DISCOVERY_FILTER, PRVDISCINFOMASK, reg_discovery_filter, prvdiscinfomask);
    reg_discovery_filter = RU_FIELD_SET(0, EPN, DISCOVERY_FILTER, PRVDISCINFOVALUE, reg_discovery_filter, prvdiscinfovalue);

    RU_REG_WRITE(0, EPN, DISCOVERY_FILTER, reg_discovery_filter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_discovery_filter_get(uint16_t *prvdiscinfomask, uint16_t *prvdiscinfovalue)
{
    uint32_t reg_discovery_filter=0;

#ifdef VALIDATE_PARMS
    if(!prvdiscinfomask || !prvdiscinfovalue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DISCOVERY_FILTER, reg_discovery_filter);

    *prvdiscinfomask = RU_FIELD_GET(0, EPN, DISCOVERY_FILTER, PRVDISCINFOMASK, reg_discovery_filter);
    *prvdiscinfovalue = RU_FIELD_GET(0, EPN, DISCOVERY_FILTER, PRVDISCINFOVALUE, reg_discovery_filter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_que_status_delay_set(uint8_t prvbbhquestatdelay)
{
    uint32_t reg_bbh_que_status_delay=0;

#ifdef VALIDATE_PARMS
#endif

    reg_bbh_que_status_delay = RU_FIELD_SET(0, EPN, BBH_QUE_STATUS_DELAY, PRVBBHQUESTATDELAY, reg_bbh_que_status_delay, prvbbhquestatdelay);

    RU_REG_WRITE(0, EPN, BBH_QUE_STATUS_DELAY, reg_bbh_que_status_delay);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_que_status_delay_get(uint8_t *prvbbhquestatdelay)
{
    uint32_t reg_bbh_que_status_delay=0;

#ifdef VALIDATE_PARMS
    if(!prvbbhquestatdelay)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BBH_QUE_STATUS_DELAY, reg_bbh_que_status_delay);

    *prvbbhquestatdelay = RU_FIELD_GET(0, EPN, BBH_QUE_STATUS_DELAY, PRVBBHQUESTATDELAY, reg_bbh_que_status_delay);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_minimum_grant_setup_set(uint16_t cfgmingrantsetup)
{
    uint32_t reg_minimum_grant_setup=0;

#ifdef VALIDATE_PARMS
#endif

    reg_minimum_grant_setup = RU_FIELD_SET(0, EPN, MINIMUM_GRANT_SETUP, CFGMINGRANTSETUP, reg_minimum_grant_setup, cfgmingrantsetup);

    RU_REG_WRITE(0, EPN, MINIMUM_GRANT_SETUP, reg_minimum_grant_setup);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_minimum_grant_setup_get(uint16_t *cfgmingrantsetup)
{
    uint32_t reg_minimum_grant_setup=0;

#ifdef VALIDATE_PARMS
    if(!cfgmingrantsetup)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, MINIMUM_GRANT_SETUP, reg_minimum_grant_setup);

    *cfgmingrantsetup = RU_FIELD_GET(0, EPN, MINIMUM_GRANT_SETUP, CFGMINGRANTSETUP, reg_minimum_grant_setup);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_reset_gnt_fifo_set(uint8_t link_idx, bdmf_boolean rstGntFifo)
{
    uint32_t reg_reset_gnt_fifo=0;

#ifdef VALIDATE_PARMS
    if((rstGntFifo >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, RESET_GNT_FIFO, reg_reset_gnt_fifo);

    FIELD_SET(reg_reset_gnt_fifo, (link_idx % 32) *1, 0x1, rstGntFifo);

    RU_REG_WRITE(0, EPN, RESET_GNT_FIFO, reg_reset_gnt_fifo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_reset_gnt_fifo_get(uint8_t link_idx, bdmf_boolean *rstGntFifo)
{
    uint32_t reg_reset_gnt_fifo=0;

#ifdef VALIDATE_PARMS
    if(!rstGntFifo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, RESET_GNT_FIFO, reg_reset_gnt_fifo);

    *rstGntFifo = FIELD_GET(reg_reset_gnt_fifo, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_reset_l1_accumulator_set(uint32_t cfgl1sclracum)
{
    uint32_t reg_reset_l1_accumulator=0;

#ifdef VALIDATE_PARMS
#endif

    reg_reset_l1_accumulator = RU_FIELD_SET(0, EPN, RESET_L1_ACCUMULATOR, CFGL1SCLRACUM, reg_reset_l1_accumulator, cfgl1sclracum);

    RU_REG_WRITE(0, EPN, RESET_L1_ACCUMULATOR, reg_reset_l1_accumulator);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_reset_l1_accumulator_get(uint32_t *cfgl1sclracum)
{
    uint32_t reg_reset_l1_accumulator=0;

#ifdef VALIDATE_PARMS
    if(!cfgl1sclracum)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, RESET_L1_ACCUMULATOR, reg_reset_l1_accumulator);

    *cfgl1sclracum = RU_FIELD_GET(0, EPN, RESET_L1_ACCUMULATOR, CFGL1SCLRACUM, reg_reset_l1_accumulator);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_l1_accumulator_sel_set(uint8_t cfgl1suvasizesel, uint8_t cfgl1ssvasizesel)
{
    uint32_t reg_l1_accumulator_sel=0;

#ifdef VALIDATE_PARMS
    if((cfgl1suvasizesel >= _5BITS_MAX_VAL_) ||
       (cfgl1ssvasizesel >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_l1_accumulator_sel = RU_FIELD_SET(0, EPN, L1_ACCUMULATOR_SEL, CFGL1SUVASIZESEL, reg_l1_accumulator_sel, cfgl1suvasizesel);
    reg_l1_accumulator_sel = RU_FIELD_SET(0, EPN, L1_ACCUMULATOR_SEL, CFGL1SSVASIZESEL, reg_l1_accumulator_sel, cfgl1ssvasizesel);

    RU_REG_WRITE(0, EPN, L1_ACCUMULATOR_SEL, reg_l1_accumulator_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_l1_accumulator_sel_get(uint8_t *cfgl1suvasizesel, uint8_t *cfgl1ssvasizesel)
{
    uint32_t reg_l1_accumulator_sel=0;

#ifdef VALIDATE_PARMS
    if(!cfgl1suvasizesel || !cfgl1ssvasizesel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, L1_ACCUMULATOR_SEL, reg_l1_accumulator_sel);

    *cfgl1suvasizesel = RU_FIELD_GET(0, EPN, L1_ACCUMULATOR_SEL, CFGL1SUVASIZESEL, reg_l1_accumulator_sel);
    *cfgl1ssvasizesel = RU_FIELD_GET(0, EPN, L1_ACCUMULATOR_SEL, CFGL1SSVASIZESEL, reg_l1_accumulator_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_l1_sva_bytes_get(uint32_t *l1ssvasize)
{
    uint32_t reg_l1_sva_bytes=0;

#ifdef VALIDATE_PARMS
    if(!l1ssvasize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, L1_SVA_BYTES, reg_l1_sva_bytes);

    *l1ssvasize = RU_FIELD_GET(0, EPN, L1_SVA_BYTES, L1SSVASIZE, reg_l1_sva_bytes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_l1_uva_bytes_get(uint32_t *l1suvasize)
{
    uint32_t reg_l1_uva_bytes=0;

#ifdef VALIDATE_PARMS
    if(!l1suvasize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, L1_UVA_BYTES, reg_l1_uva_bytes);

    *l1suvasize = RU_FIELD_GET(0, EPN, L1_UVA_BYTES, L1SUVASIZE, reg_l1_uva_bytes);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_l1_sva_overflow_get(uint32_t *l1ssvaoverflow)
{
    uint32_t reg_l1_sva_overflow=0;

#ifdef VALIDATE_PARMS
    if(!l1ssvaoverflow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, L1_SVA_OVERFLOW, reg_l1_sva_overflow);

    *l1ssvaoverflow = RU_FIELD_GET(0, EPN, L1_SVA_OVERFLOW, L1SSVAOVERFLOW, reg_l1_sva_overflow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_l1_uva_overflow_get(uint32_t *l1suvaoverflow)
{
    uint32_t reg_l1_uva_overflow=0;

#ifdef VALIDATE_PARMS
    if(!l1suvaoverflow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, L1_UVA_OVERFLOW, reg_l1_uva_overflow);

    *l1suvaoverflow = RU_FIELD_GET(0, EPN, L1_UVA_OVERFLOW, L1SUVAOVERFLOW, reg_l1_uva_overflow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_reset_rpt_pri_set(const epn_reset_rpt_pri *reset_rpt_pri)
{
    uint32_t reg_reset_rpt_pri=0;

#ifdef VALIDATE_PARMS
    if(!reset_rpt_pri)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((reset_rpt_pri->nullrptpri15 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri14 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri13 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri12 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri11 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri10 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri9 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri8 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri7 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri6 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri5 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri4 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri3 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri2 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri1 >= _1BITS_MAX_VAL_) ||
       (reset_rpt_pri->nullrptpri0 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI15, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri15);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI14, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri14);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI13, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri13);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI12, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri12);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI11, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri11);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI10, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri10);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI9, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri9);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI8, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri8);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI7, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri7);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI6, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri6);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI5, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri5);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI4, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri4);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI3, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri3);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI2, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri2);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI1, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri1);
    reg_reset_rpt_pri = RU_FIELD_SET(0, EPN, RESET_RPT_PRI, NULLRPTPRI0, reg_reset_rpt_pri, reset_rpt_pri->nullrptpri0);

    RU_REG_WRITE(0, EPN, RESET_RPT_PRI, reg_reset_rpt_pri);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_reset_rpt_pri_get(epn_reset_rpt_pri *reset_rpt_pri)
{
    uint32_t reg_reset_rpt_pri=0;

#ifdef VALIDATE_PARMS
    if(!reset_rpt_pri)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, RESET_RPT_PRI, reg_reset_rpt_pri);

    reset_rpt_pri->nullrptpri15 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI15, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri14 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI14, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri13 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI13, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri12 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI12, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri11 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI11, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri10 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI10, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri9 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI9, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri8 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI8, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri7 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI7, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri6 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI6, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri5 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI5, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri4 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI4, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri3 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI3, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri2 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI2, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri1 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI1, reg_reset_rpt_pri);
    reset_rpt_pri->nullrptpri0 = RU_FIELD_GET(0, EPN, RESET_RPT_PRI, NULLRPTPRI0, reg_reset_rpt_pri);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_reset_l2_rpt_fifo_set(uint8_t link_idx, bdmf_boolean cfgL2SClrQue)
{
    uint32_t reg_reset_l2_rpt_fifo=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, EPN, RESET_L2_RPT_FIFO, reg_reset_l2_rpt_fifo);

    FIELD_SET(reg_reset_l2_rpt_fifo, (link_idx % 32) *1, 0x1, cfgL2SClrQue);

    RU_REG_WRITE(0, EPN, RESET_L2_RPT_FIFO, reg_reset_l2_rpt_fifo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_reset_l2_rpt_fifo_get(uint8_t link_idx, bdmf_boolean *cfgL2SClrQue)
{
    uint32_t reg_reset_l2_rpt_fifo=0;

#ifdef VALIDATE_PARMS
    if(!cfgL2SClrQue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, RESET_L2_RPT_FIFO, reg_reset_l2_rpt_fifo);

    *cfgL2SClrQue = FIELD_GET(reg_reset_l2_rpt_fifo, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_enable_upstream_set(uint8_t link_idx, bdmf_boolean cfgEnableUpstream)
{
    uint32_t reg_enable_upstream=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, EPN, ENABLE_UPSTREAM, reg_enable_upstream);

    FIELD_SET(reg_enable_upstream, (link_idx % 32) *1, 0x1, cfgEnableUpstream);

    RU_REG_WRITE(0, EPN, ENABLE_UPSTREAM, reg_enable_upstream);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_enable_upstream_get(uint8_t link_idx, bdmf_boolean *cfgEnableUpstream)
{
    uint32_t reg_enable_upstream=0;

#ifdef VALIDATE_PARMS
    if(!cfgEnableUpstream)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, ENABLE_UPSTREAM, reg_enable_upstream);

    *cfgEnableUpstream = FIELD_GET(reg_enable_upstream, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_enable_upstream_fb_get(uint8_t link_idx, bdmf_boolean *cfgEnableUpstreamFeedBack)
{
    uint32_t reg_enable_upstream_fb=0;

#ifdef VALIDATE_PARMS
    if(!cfgEnableUpstreamFeedBack)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, ENABLE_UPSTREAM_FB, reg_enable_upstream_fb);

    *cfgEnableUpstreamFeedBack = FIELD_GET(reg_enable_upstream_fb, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_enable_upstream_fec_set(uint8_t link_idx, bdmf_boolean upstreamFecEn)
{
    uint32_t reg_enable_upstream_fec=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, EPN, ENABLE_UPSTREAM_FEC, reg_enable_upstream_fec);

    FIELD_SET(reg_enable_upstream_fec, (link_idx % 32) *1, 0x1, upstreamFecEn);

    RU_REG_WRITE(0, EPN, ENABLE_UPSTREAM_FEC, reg_enable_upstream_fec);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_enable_upstream_fec_get(uint8_t link_idx, bdmf_boolean *upstreamFecEn)
{
    uint32_t reg_enable_upstream_fec=0;

#ifdef VALIDATE_PARMS
    if(!upstreamFecEn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, ENABLE_UPSTREAM_FEC, reg_enable_upstream_fec);

    *upstreamFecEn = FIELD_GET(reg_enable_upstream_fec, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_report_byte_length_set(uint8_t prvrptbytelen)
{
    uint32_t reg_report_byte_length=0;

#ifdef VALIDATE_PARMS
#endif

    reg_report_byte_length = RU_FIELD_SET(0, EPN, REPORT_BYTE_LENGTH, PRVRPTBYTELEN, reg_report_byte_length, prvrptbytelen);

    RU_REG_WRITE(0, EPN, REPORT_BYTE_LENGTH, reg_report_byte_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_report_byte_length_get(uint8_t *prvrptbytelen)
{
    uint32_t reg_report_byte_length=0;

#ifdef VALIDATE_PARMS
    if(!prvrptbytelen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, REPORT_BYTE_LENGTH, reg_report_byte_length);

    *prvrptbytelen = RU_FIELD_GET(0, EPN, REPORT_BYTE_LENGTH, PRVRPTBYTELEN, reg_report_byte_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_main_int_status_set(const epn_main_int_status *main_int_status)
{
    uint32_t reg_main_int_status=0;

#ifdef VALIDATE_PARMS
    if(!main_int_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((main_int_status->intbbhupfrabort >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcol2sburstcapoverflowpres >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcoemptyrpt >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcodrxerrabortpres >= _1BITS_MAX_VAL_) ||
       (main_int_status->intl2sfifooverrun >= _1BITS_MAX_VAL_) ||
       (main_int_status->intco1588tsint >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcorptpres >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcogntpres >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcodelstalegnt >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcogntnonpoll >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcogntmisalign >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcognttoofar >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcogntinterval >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcogntdiscovery >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcogntmissabort >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcogntfullabort >= _1BITS_MAX_VAL_) ||
       (main_int_status->intbadupfrlen >= _1BITS_MAX_VAL_) ||
       (main_int_status->intuptardypacket >= _1BITS_MAX_VAL_) ||
       (main_int_status->intuprptfrxmt >= _1BITS_MAX_VAL_) ||
       (main_int_status->intbififooverrun >= _1BITS_MAX_VAL_) ||
       (main_int_status->intburstgnttoobig >= _1BITS_MAX_VAL_) ||
       (main_int_status->intwrgnttoobig >= _1BITS_MAX_VAL_) ||
       (main_int_status->intrcvgnttoobig >= _1BITS_MAX_VAL_) ||
       (main_int_status->intdnstatsoverrun >= _1BITS_MAX_VAL_) ||
       (main_int_status->intupstatsoverrun >= _1BITS_MAX_VAL_) ||
       (main_int_status->intdnoutoforder >= _1BITS_MAX_VAL_) ||
       (main_int_status->inttruantbbhhalt >= _1BITS_MAX_VAL_) ||
       (main_int_status->intupinvldgntlen >= _1BITS_MAX_VAL_) ||
       (main_int_status->intcobbhupsfault >= _1BITS_MAX_VAL_) ||
       (main_int_status->intdntimeinsync >= _1BITS_MAX_VAL_) ||
       (main_int_status->intdntimenotinsync >= _1BITS_MAX_VAL_) ||
       (main_int_status->intdportrdy >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTBBHUPFRABORT, reg_main_int_status, main_int_status->intbbhupfrabort);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCOL2SBURSTCAPOVERFLOWPRES, reg_main_int_status, main_int_status->intcol2sburstcapoverflowpres);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCOEMPTYRPT, reg_main_int_status, main_int_status->intcoemptyrpt);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCODRXERRABORTPRES, reg_main_int_status, main_int_status->intcodrxerrabortpres);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTL2SFIFOOVERRUN, reg_main_int_status, main_int_status->intl2sfifooverrun);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCO1588TSINT, reg_main_int_status, main_int_status->intco1588tsint);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCORPTPRES, reg_main_int_status, main_int_status->intcorptpres);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCOGNTPRES, reg_main_int_status, main_int_status->intcogntpres);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCODELSTALEGNT, reg_main_int_status, main_int_status->intcodelstalegnt);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCOGNTNONPOLL, reg_main_int_status, main_int_status->intcogntnonpoll);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCOGNTMISALIGN, reg_main_int_status, main_int_status->intcogntmisalign);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCOGNTTOOFAR, reg_main_int_status, main_int_status->intcognttoofar);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCOGNTINTERVAL, reg_main_int_status, main_int_status->intcogntinterval);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCOGNTDISCOVERY, reg_main_int_status, main_int_status->intcogntdiscovery);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCOGNTMISSABORT, reg_main_int_status, main_int_status->intcogntmissabort);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCOGNTFULLABORT, reg_main_int_status, main_int_status->intcogntfullabort);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTBADUPFRLEN, reg_main_int_status, main_int_status->intbadupfrlen);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTUPTARDYPACKET, reg_main_int_status, main_int_status->intuptardypacket);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTUPRPTFRXMT, reg_main_int_status, main_int_status->intuprptfrxmt);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTBIFIFOOVERRUN, reg_main_int_status, main_int_status->intbififooverrun);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTBURSTGNTTOOBIG, reg_main_int_status, main_int_status->intburstgnttoobig);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTWRGNTTOOBIG, reg_main_int_status, main_int_status->intwrgnttoobig);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTRCVGNTTOOBIG, reg_main_int_status, main_int_status->intrcvgnttoobig);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTDNSTATSOVERRUN, reg_main_int_status, main_int_status->intdnstatsoverrun);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTUPSTATSOVERRUN, reg_main_int_status, main_int_status->intupstatsoverrun);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTDNOUTOFORDER, reg_main_int_status, main_int_status->intdnoutoforder);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTTRUANTBBHHALT, reg_main_int_status, main_int_status->inttruantbbhhalt);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTUPINVLDGNTLEN, reg_main_int_status, main_int_status->intupinvldgntlen);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTCOBBHUPSFAULT, reg_main_int_status, main_int_status->intcobbhupsfault);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTDNTIMEINSYNC, reg_main_int_status, main_int_status->intdntimeinsync);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTDNTIMENOTINSYNC, reg_main_int_status, main_int_status->intdntimenotinsync);
    reg_main_int_status = RU_FIELD_SET(0, EPN, MAIN_INT_STATUS, INTDPORTRDY, reg_main_int_status, main_int_status->intdportrdy);

    RU_REG_WRITE(0, EPN, MAIN_INT_STATUS, reg_main_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_main_int_status_get(epn_main_int_status *main_int_status)
{
    uint32_t reg_main_int_status=0;

#ifdef VALIDATE_PARMS
    if(!main_int_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, MAIN_INT_STATUS, reg_main_int_status);

    main_int_status->intbbhupfrabort = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTBBHUPFRABORT, reg_main_int_status);
    main_int_status->intcol2sburstcapoverflowpres = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCOL2SBURSTCAPOVERFLOWPRES, reg_main_int_status);
    main_int_status->intcoemptyrpt = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCOEMPTYRPT, reg_main_int_status);
    main_int_status->intcodrxerrabortpres = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCODRXERRABORTPRES, reg_main_int_status);
    main_int_status->intl2sfifooverrun = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTL2SFIFOOVERRUN, reg_main_int_status);
    main_int_status->intco1588tsint = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCO1588TSINT, reg_main_int_status);
    main_int_status->intcorptpres = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCORPTPRES, reg_main_int_status);
    main_int_status->intcogntpres = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCOGNTPRES, reg_main_int_status);
    main_int_status->intcodelstalegnt = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCODELSTALEGNT, reg_main_int_status);
    main_int_status->intcogntnonpoll = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCOGNTNONPOLL, reg_main_int_status);
    main_int_status->intcogntmisalign = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCOGNTMISALIGN, reg_main_int_status);
    main_int_status->intcognttoofar = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCOGNTTOOFAR, reg_main_int_status);
    main_int_status->intcogntinterval = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCOGNTINTERVAL, reg_main_int_status);
    main_int_status->intcogntdiscovery = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCOGNTDISCOVERY, reg_main_int_status);
    main_int_status->intcogntmissabort = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCOGNTMISSABORT, reg_main_int_status);
    main_int_status->intcogntfullabort = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCOGNTFULLABORT, reg_main_int_status);
    main_int_status->intbadupfrlen = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTBADUPFRLEN, reg_main_int_status);
    main_int_status->intuptardypacket = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTUPTARDYPACKET, reg_main_int_status);
    main_int_status->intuprptfrxmt = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTUPRPTFRXMT, reg_main_int_status);
    main_int_status->intbififooverrun = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTBIFIFOOVERRUN, reg_main_int_status);
    main_int_status->intburstgnttoobig = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTBURSTGNTTOOBIG, reg_main_int_status);
    main_int_status->intwrgnttoobig = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTWRGNTTOOBIG, reg_main_int_status);
    main_int_status->intrcvgnttoobig = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTRCVGNTTOOBIG, reg_main_int_status);
    main_int_status->intdnstatsoverrun = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTDNSTATSOVERRUN, reg_main_int_status);
    main_int_status->intupstatsoverrun = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTUPSTATSOVERRUN, reg_main_int_status);
    main_int_status->intdnoutoforder = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTDNOUTOFORDER, reg_main_int_status);
    main_int_status->inttruantbbhhalt = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTTRUANTBBHHALT, reg_main_int_status);
    main_int_status->intupinvldgntlen = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTUPINVLDGNTLEN, reg_main_int_status);
    main_int_status->intcobbhupsfault = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTCOBBHUPSFAULT, reg_main_int_status);
    main_int_status->intdntimeinsync = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTDNTIMEINSYNC, reg_main_int_status);
    main_int_status->intdntimenotinsync = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTDNTIMENOTINSYNC, reg_main_int_status);
    main_int_status->intdportrdy = RU_FIELD_GET(0, EPN, MAIN_INT_STATUS, INTDPORTRDY, reg_main_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_full_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntFullAbort)
{
    uint32_t reg_gnt_full_int_status=0;

#ifdef VALIDATE_PARMS
    if((intDnGntFullAbort >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_gnt_full_int_status, (link_idx % 32) *1, 0x1, intDnGntFullAbort);

    RU_REG_WRITE(0, EPN, GNT_FULL_INT_STATUS, reg_gnt_full_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_full_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntFullAbort)
{
    uint32_t reg_gnt_full_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intDnGntFullAbort)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_FULL_INT_STATUS, reg_gnt_full_int_status);

    *intDnGntFullAbort = FIELD_GET(reg_gnt_full_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_full_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDnGntFullAbort)
{
    uint32_t reg_gnt_full_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskIntDnGntFullAbort >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, GNT_FULL_INT_MASK, reg_gnt_full_int_mask);

    FIELD_SET(reg_gnt_full_int_mask, (link_idx % 32) *1, 0x1, maskIntDnGntFullAbort);

    RU_REG_WRITE(0, EPN, GNT_FULL_INT_MASK, reg_gnt_full_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_full_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDnGntFullAbort)
{
    uint32_t reg_gnt_full_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskIntDnGntFullAbort)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_FULL_INT_MASK, reg_gnt_full_int_mask);

    *maskIntDnGntFullAbort = FIELD_GET(reg_gnt_full_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_miss_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntMissAbort)
{
    uint32_t reg_gnt_miss_int_status=0;

#ifdef VALIDATE_PARMS
    if((intDnGntMissAbort >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_gnt_miss_int_status, (link_idx % 32) *1, 0x1, intDnGntMissAbort);

    RU_REG_WRITE(0, EPN, GNT_MISS_INT_STATUS, reg_gnt_miss_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_miss_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntMissAbort)
{
    uint32_t reg_gnt_miss_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intDnGntMissAbort)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_MISS_INT_STATUS, reg_gnt_miss_int_status);

    *intDnGntMissAbort = FIELD_GET(reg_gnt_miss_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_miss_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDnGntMissAbort)
{
    uint32_t reg_gnt_miss_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskIntDnGntMissAbort >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, GNT_MISS_INT_MASK, reg_gnt_miss_int_mask);

    FIELD_SET(reg_gnt_miss_int_mask, (link_idx % 32) *1, 0x1, maskIntDnGntMissAbort);

    RU_REG_WRITE(0, EPN, GNT_MISS_INT_MASK, reg_gnt_miss_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_miss_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDnGntMissAbort)
{
    uint32_t reg_gnt_miss_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskIntDnGntMissAbort)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_MISS_INT_MASK, reg_gnt_miss_int_mask);

    *maskIntDnGntMissAbort = FIELD_GET(reg_gnt_miss_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_disc_rx_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntDiscovery)
{
    uint32_t reg_disc_rx_int_status=0;

#ifdef VALIDATE_PARMS
    if((intDnGntDiscovery >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_disc_rx_int_status, (link_idx % 32) *1, 0x1, intDnGntDiscovery);

    RU_REG_WRITE(0, EPN, DISC_RX_INT_STATUS, reg_disc_rx_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_disc_rx_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntDiscovery)
{
    uint32_t reg_disc_rx_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intDnGntDiscovery)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DISC_RX_INT_STATUS, reg_disc_rx_int_status);

    *intDnGntDiscovery = FIELD_GET(reg_disc_rx_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_disc_rx_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDnGntDiscovery)
{
    uint32_t reg_disc_rx_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskIntDnGntDiscovery >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, DISC_RX_INT_MASK, reg_disc_rx_int_mask);

    FIELD_SET(reg_disc_rx_int_mask, (link_idx % 32) *1, 0x1, maskIntDnGntDiscovery);

    RU_REG_WRITE(0, EPN, DISC_RX_INT_MASK, reg_disc_rx_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_disc_rx_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDnGntDiscovery)
{
    uint32_t reg_disc_rx_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskIntDnGntDiscovery)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DISC_RX_INT_MASK, reg_disc_rx_int_mask);

    *maskIntDnGntDiscovery = FIELD_GET(reg_disc_rx_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_intv_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntInterval)
{
    uint32_t reg_gnt_intv_int_status=0;

#ifdef VALIDATE_PARMS
    if((intDnGntInterval >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_gnt_intv_int_status, (link_idx % 32) *1, 0x1, intDnGntInterval);

    RU_REG_WRITE(0, EPN, GNT_INTV_INT_STATUS, reg_gnt_intv_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_intv_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntInterval)
{
    uint32_t reg_gnt_intv_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intDnGntInterval)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_INTV_INT_STATUS, reg_gnt_intv_int_status);

    *intDnGntInterval = FIELD_GET(reg_gnt_intv_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_intv_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDnGntInterval)
{
    uint32_t reg_gnt_intv_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskIntDnGntInterval >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, GNT_INTV_INT_MASK, reg_gnt_intv_int_mask);

    FIELD_SET(reg_gnt_intv_int_mask, (link_idx % 32) *1, 0x1, maskIntDnGntInterval);

    RU_REG_WRITE(0, EPN, GNT_INTV_INT_MASK, reg_gnt_intv_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_intv_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDnGntInterval)
{
    uint32_t reg_gnt_intv_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskIntDnGntInterval)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_INTV_INT_MASK, reg_gnt_intv_int_mask);

    *maskIntDnGntInterval = FIELD_GET(reg_gnt_intv_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_far_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntTooFar)
{
    uint32_t reg_gnt_far_int_status=0;

#ifdef VALIDATE_PARMS
    if((intDnGntTooFar >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_gnt_far_int_status, (link_idx % 32) *1, 0x1, intDnGntTooFar);

    RU_REG_WRITE(0, EPN, GNT_FAR_INT_STATUS, reg_gnt_far_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_far_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntTooFar)
{
    uint32_t reg_gnt_far_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intDnGntTooFar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_FAR_INT_STATUS, reg_gnt_far_int_status);

    *intDnGntTooFar = FIELD_GET(reg_gnt_far_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_far_int_mask_set(uint8_t link_idx, bdmf_boolean maskDnGntTooFar)
{
    uint32_t reg_gnt_far_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskDnGntTooFar >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, GNT_FAR_INT_MASK, reg_gnt_far_int_mask);

    FIELD_SET(reg_gnt_far_int_mask, (link_idx % 32) *1, 0x1, maskDnGntTooFar);

    RU_REG_WRITE(0, EPN, GNT_FAR_INT_MASK, reg_gnt_far_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_far_int_mask_get(uint8_t link_idx, bdmf_boolean *maskDnGntTooFar)
{
    uint32_t reg_gnt_far_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskDnGntTooFar)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_FAR_INT_MASK, reg_gnt_far_int_mask);

    *maskDnGntTooFar = FIELD_GET(reg_gnt_far_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_misalgn_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntMisalign)
{
    uint32_t reg_gnt_misalgn_int_status=0;

#ifdef VALIDATE_PARMS
    if((intDnGntMisalign >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_gnt_misalgn_int_status, (link_idx % 32) *1, 0x1, intDnGntMisalign);

    RU_REG_WRITE(0, EPN, GNT_MISALGN_INT_STATUS, reg_gnt_misalgn_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_misalgn_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntMisalign)
{
    uint32_t reg_gnt_misalgn_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intDnGntMisalign)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_MISALGN_INT_STATUS, reg_gnt_misalgn_int_status);

    *intDnGntMisalign = FIELD_GET(reg_gnt_misalgn_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_misalgn_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDnGntMisalign)
{
    uint32_t reg_gnt_misalgn_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskIntDnGntMisalign >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, GNT_MISALGN_INT_MASK, reg_gnt_misalgn_int_mask);

    FIELD_SET(reg_gnt_misalgn_int_mask, (link_idx % 32) *1, 0x1, maskIntDnGntMisalign);

    RU_REG_WRITE(0, EPN, GNT_MISALGN_INT_MASK, reg_gnt_misalgn_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_misalgn_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDnGntMisalign)
{
    uint32_t reg_gnt_misalgn_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskIntDnGntMisalign)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_MISALGN_INT_MASK, reg_gnt_misalgn_int_mask);

    *maskIntDnGntMisalign = FIELD_GET(reg_gnt_misalgn_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_np_gnt_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntNonPoll)
{
    uint32_t reg_np_gnt_int_status=0;

#ifdef VALIDATE_PARMS
    if((intDnGntNonPoll >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_np_gnt_int_status, (link_idx % 32) *1, 0x1, intDnGntNonPoll);

    RU_REG_WRITE(0, EPN, NP_GNT_INT_STATUS, reg_np_gnt_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_np_gnt_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntNonPoll)
{
    uint32_t reg_np_gnt_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intDnGntNonPoll)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, NP_GNT_INT_STATUS, reg_np_gnt_int_status);

    *intDnGntNonPoll = FIELD_GET(reg_np_gnt_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_np_gnt_int_mask_set(uint8_t link_idx, bdmf_boolean maskDnGntNonPoll)
{
    uint32_t reg_np_gnt_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskDnGntNonPoll >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, NP_GNT_INT_MASK, reg_np_gnt_int_mask);

    FIELD_SET(reg_np_gnt_int_mask, (link_idx % 32) *1, 0x1, maskDnGntNonPoll);

    RU_REG_WRITE(0, EPN, NP_GNT_INT_MASK, reg_np_gnt_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_np_gnt_int_mask_get(uint8_t link_idx, bdmf_boolean *maskDnGntNonPoll)
{
    uint32_t reg_np_gnt_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskDnGntNonPoll)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, NP_GNT_INT_MASK, reg_np_gnt_int_mask);

    *maskDnGntNonPoll = FIELD_GET(reg_np_gnt_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_del_stale_int_status_set(uint8_t link_idx, bdmf_boolean intDelStaleGnt)
{
    uint32_t reg_del_stale_int_status=0;

#ifdef VALIDATE_PARMS
    if((intDelStaleGnt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_del_stale_int_status, (link_idx % 32) *1, 0x1, intDelStaleGnt);

    RU_REG_WRITE(0, EPN, DEL_STALE_INT_STATUS, reg_del_stale_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_del_stale_int_status_get(uint8_t link_idx, bdmf_boolean *intDelStaleGnt)
{
    uint32_t reg_del_stale_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intDelStaleGnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DEL_STALE_INT_STATUS, reg_del_stale_int_status);

    *intDelStaleGnt = FIELD_GET(reg_del_stale_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_del_stale_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDelStaleGnt)
{
    uint32_t reg_del_stale_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskIntDelStaleGnt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, DEL_STALE_INT_MASK, reg_del_stale_int_mask);

    FIELD_SET(reg_del_stale_int_mask, (link_idx % 32) *1, 0x1, maskIntDelStaleGnt);

    RU_REG_WRITE(0, EPN, DEL_STALE_INT_MASK, reg_del_stale_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_del_stale_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDelStaleGnt)
{
    uint32_t reg_del_stale_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskIntDelStaleGnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DEL_STALE_INT_MASK, reg_del_stale_int_mask);

    *maskIntDelStaleGnt = FIELD_GET(reg_del_stale_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_pres_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntRdy)
{
    uint32_t reg_gnt_pres_int_status=0;

#ifdef VALIDATE_PARMS
    if((intDnGntRdy >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_gnt_pres_int_status, (link_idx % 32) *1, 0x1, intDnGntRdy);

    RU_REG_WRITE(0, EPN, GNT_PRES_INT_STATUS, reg_gnt_pres_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_pres_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntRdy)
{
    uint32_t reg_gnt_pres_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intDnGntRdy)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_PRES_INT_STATUS, reg_gnt_pres_int_status);

    *intDnGntRdy = FIELD_GET(reg_gnt_pres_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_pres_int_mask_set(uint8_t link_idx, bdmf_boolean maskDnGntRdy)
{
    uint32_t reg_gnt_pres_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskDnGntRdy >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, GNT_PRES_INT_MASK, reg_gnt_pres_int_mask);

    FIELD_SET(reg_gnt_pres_int_mask, (link_idx % 32) *1, 0x1, maskDnGntRdy);

    RU_REG_WRITE(0, EPN, GNT_PRES_INT_MASK, reg_gnt_pres_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_pres_int_mask_get(uint8_t link_idx, bdmf_boolean *maskDnGntRdy)
{
    uint32_t reg_gnt_pres_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskDnGntRdy)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_PRES_INT_MASK, reg_gnt_pres_int_mask);

    *maskDnGntRdy = FIELD_GET(reg_gnt_pres_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_rpt_pres_int_status_set(uint8_t link_idx, bdmf_boolean intUpRptFifo)
{
    uint32_t reg_rpt_pres_int_status=0;

#ifdef VALIDATE_PARMS
    if((intUpRptFifo >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_rpt_pres_int_status, (link_idx % 32) *1, 0x1, intUpRptFifo);

    RU_REG_WRITE(0, EPN, RPT_PRES_INT_STATUS, reg_rpt_pres_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_rpt_pres_int_status_get(uint8_t link_idx, bdmf_boolean *intUpRptFifo)
{
    uint32_t reg_rpt_pres_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intUpRptFifo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, RPT_PRES_INT_STATUS, reg_rpt_pres_int_status);

    *intUpRptFifo = FIELD_GET(reg_rpt_pres_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_rpt_pres_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntUpRptFifo)
{
    uint32_t reg_rpt_pres_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskIntUpRptFifo >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, RPT_PRES_INT_MASK, reg_rpt_pres_int_mask);

    FIELD_SET(reg_rpt_pres_int_mask, (link_idx % 32) *1, 0x1, maskIntUpRptFifo);

    RU_REG_WRITE(0, EPN, RPT_PRES_INT_MASK, reg_rpt_pres_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_rpt_pres_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntUpRptFifo)
{
    uint32_t reg_rpt_pres_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskIntUpRptFifo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, RPT_PRES_INT_MASK, reg_rpt_pres_int_mask);

    *maskIntUpRptFifo = FIELD_GET(reg_rpt_pres_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_drx_abort_int_status_set(uint8_t link_idx, bdmf_boolean intDrxErrAbort)
{
    uint32_t reg_drx_abort_int_status=0;

#ifdef VALIDATE_PARMS
#endif


    FIELD_SET(reg_drx_abort_int_status, (link_idx % 32) *1, 0x1, intDrxErrAbort);

    RU_REG_WRITE(0, EPN, DRX_ABORT_INT_STATUS, reg_drx_abort_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_drx_abort_int_status_get(uint8_t link_idx, bdmf_boolean *intDrxErrAbort)
{
    uint32_t reg_drx_abort_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intDrxErrAbort)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DRX_ABORT_INT_STATUS, reg_drx_abort_int_status);

    *intDrxErrAbort = FIELD_GET(reg_drx_abort_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_drx_abort_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDrxErrAbort)
{
    uint32_t reg_drx_abort_int_mask=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, EPN, DRX_ABORT_INT_MASK, reg_drx_abort_int_mask);

    FIELD_SET(reg_drx_abort_int_mask, (link_idx % 32) *1, 0x1, maskIntDrxErrAbort);

    RU_REG_WRITE(0, EPN, DRX_ABORT_INT_MASK, reg_drx_abort_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_drx_abort_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDrxErrAbort)
{
    uint32_t reg_drx_abort_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskIntDrxErrAbort)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DRX_ABORT_INT_MASK, reg_drx_abort_int_mask);

    *maskIntDrxErrAbort = FIELD_GET(reg_drx_abort_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_empty_rpt_int_status_set(uint8_t link_idx, bdmf_boolean intEmptyRpt)
{
    uint32_t reg_empty_rpt_int_status=0;

#ifdef VALIDATE_PARMS
    if((intEmptyRpt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_empty_rpt_int_status, (link_idx % 32) *1, 0x1, intEmptyRpt);

    RU_REG_WRITE(0, EPN, EMPTY_RPT_INT_STATUS, reg_empty_rpt_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_empty_rpt_int_status_get(uint8_t link_idx, bdmf_boolean *intEmptyRpt)
{
    uint32_t reg_empty_rpt_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intEmptyRpt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, EMPTY_RPT_INT_STATUS, reg_empty_rpt_int_status);

    *intEmptyRpt = FIELD_GET(reg_empty_rpt_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_empty_rpt_int_mask_set(uint8_t link_idx, bdmf_boolean  maskIntEmptyRpt)
{
    uint32_t reg_empty_rpt_int_mask=0;

#ifdef VALIDATE_PARMS
    if(( maskIntEmptyRpt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, EMPTY_RPT_INT_MASK, reg_empty_rpt_int_mask);

    FIELD_SET(reg_empty_rpt_int_mask, (link_idx % 32) *1, 0x1,  maskIntEmptyRpt);

    RU_REG_WRITE(0, EPN, EMPTY_RPT_INT_MASK, reg_empty_rpt_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_empty_rpt_int_mask_get(uint8_t link_idx, bdmf_boolean * maskIntEmptyRpt)
{
    uint32_t reg_empty_rpt_int_mask=0;

#ifdef VALIDATE_PARMS
    if(! maskIntEmptyRpt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, EMPTY_RPT_INT_MASK, reg_empty_rpt_int_mask);

    * maskIntEmptyRpt = FIELD_GET(reg_empty_rpt_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bcap_overflow_int_status_set(uint8_t l2_acc_idx, bdmf_boolean intl2sBurstCapOverFlow)
{
    uint32_t reg_bcap_overflow_int_status=0;

#ifdef VALIDATE_PARMS
#endif


    FIELD_SET(reg_bcap_overflow_int_status, (l2_acc_idx % 32) *1, 0x1, intl2sBurstCapOverFlow);

    RU_REG_WRITE(0, EPN, BCAP_OVERFLOW_INT_STATUS, reg_bcap_overflow_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bcap_overflow_int_status_get(uint8_t l2_acc_idx, bdmf_boolean *intl2sBurstCapOverFlow)
{
    uint32_t reg_bcap_overflow_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intl2sBurstCapOverFlow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BCAP_OVERFLOW_INT_STATUS, reg_bcap_overflow_int_status);

    *intl2sBurstCapOverFlow = FIELD_GET(reg_bcap_overflow_int_status, (l2_acc_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bcap_overflow_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntl2sBurstCapOverFlow)
{
    uint32_t reg_bcap_overflow_int_mask=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, EPN, BCAP_OVERFLOW_INT_MASK, reg_bcap_overflow_int_mask);

    FIELD_SET(reg_bcap_overflow_int_mask, (link_idx % 32) *1, 0x1, maskIntl2sBurstCapOverFlow);

    RU_REG_WRITE(0, EPN, BCAP_OVERFLOW_INT_MASK, reg_bcap_overflow_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bcap_overflow_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntl2sBurstCapOverFlow)
{
    uint32_t reg_bcap_overflow_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskIntl2sBurstCapOverFlow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BCAP_OVERFLOW_INT_MASK, reg_bcap_overflow_int_mask);

    *maskIntl2sBurstCapOverFlow = FIELD_GET(reg_bcap_overflow_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_dns_fault_int_status_set(bdmf_boolean intbbhdnsoverflow)
{
    uint32_t reg_bbh_dns_fault_int_status=0;

#ifdef VALIDATE_PARMS
    if((intbbhdnsoverflow >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_dns_fault_int_status = RU_FIELD_SET(0, EPN, BBH_DNS_FAULT_INT_STATUS, INTBBHDNSOVERFLOW, reg_bbh_dns_fault_int_status, intbbhdnsoverflow);

    RU_REG_WRITE(0, EPN, BBH_DNS_FAULT_INT_STATUS, reg_bbh_dns_fault_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_dns_fault_int_status_get(bdmf_boolean *intbbhdnsoverflow)
{
    uint32_t reg_bbh_dns_fault_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intbbhdnsoverflow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BBH_DNS_FAULT_INT_STATUS, reg_bbh_dns_fault_int_status);

    *intbbhdnsoverflow = RU_FIELD_GET(0, EPN, BBH_DNS_FAULT_INT_STATUS, INTBBHDNSOVERFLOW, reg_bbh_dns_fault_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_dns_fault_int_mask_set(bdmf_boolean maskintbbhdnsoverflow)
{
    uint32_t reg_bbh_dns_fault_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskintbbhdnsoverflow >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_dns_fault_int_mask = RU_FIELD_SET(0, EPN, BBH_DNS_FAULT_INT_MASK, MASKINTBBHDNSOVERFLOW, reg_bbh_dns_fault_int_mask, maskintbbhdnsoverflow);

    RU_REG_WRITE(0, EPN, BBH_DNS_FAULT_INT_MASK, reg_bbh_dns_fault_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_dns_fault_int_mask_get(bdmf_boolean *maskintbbhdnsoverflow)
{
    uint32_t reg_bbh_dns_fault_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskintbbhdnsoverflow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BBH_DNS_FAULT_INT_MASK, reg_bbh_dns_fault_int_mask);

    *maskintbbhdnsoverflow = RU_FIELD_GET(0, EPN, BBH_DNS_FAULT_INT_MASK, MASKINTBBHDNSOVERFLOW, reg_bbh_dns_fault_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_ups_fault_int_status_set(uint8_t link_idx, bdmf_boolean intBbhUpsFault)
{
    uint32_t reg_bbh_ups_fault_int_status=0;

#ifdef VALIDATE_PARMS
    if((intBbhUpsFault >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif


    FIELD_SET(reg_bbh_ups_fault_int_status, (link_idx % 32) *1, 0x1, intBbhUpsFault);

    RU_REG_WRITE(0, EPN, BBH_UPS_FAULT_INT_STATUS, reg_bbh_ups_fault_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_ups_fault_int_status_get(uint8_t link_idx, bdmf_boolean *intBbhUpsFault)
{
    uint32_t reg_bbh_ups_fault_int_status=0;

#ifdef VALIDATE_PARMS
    if(!intBbhUpsFault)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BBH_UPS_FAULT_INT_STATUS, reg_bbh_ups_fault_int_status);

    *intBbhUpsFault = FIELD_GET(reg_bbh_ups_fault_int_status, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_ups_fault_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntBbhUpsFault)
{
    uint32_t reg_bbh_ups_fault_int_mask=0;

#ifdef VALIDATE_PARMS
    if((maskIntBbhUpsFault >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, BBH_UPS_FAULT_INT_MASK, reg_bbh_ups_fault_int_mask);

    FIELD_SET(reg_bbh_ups_fault_int_mask, (link_idx % 32) *1, 0x1, maskIntBbhUpsFault);

    RU_REG_WRITE(0, EPN, BBH_UPS_FAULT_INT_MASK, reg_bbh_ups_fault_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_ups_fault_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntBbhUpsFault)
{
    uint32_t reg_bbh_ups_fault_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!maskIntBbhUpsFault)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BBH_UPS_FAULT_INT_MASK, reg_bbh_ups_fault_int_mask);

    *maskIntBbhUpsFault = FIELD_GET(reg_bbh_ups_fault_int_mask, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_ups_abort_int_status_set(bdmf_boolean tardybbhabort)
{
    uint32_t reg_bbh_ups_abort_int_status=0;

#ifdef VALIDATE_PARMS
    if((tardybbhabort >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_ups_abort_int_status = RU_FIELD_SET(0, EPN, BBH_UPS_ABORT_INT_STATUS, TARDYBBHABORT, reg_bbh_ups_abort_int_status, tardybbhabort);

    RU_REG_WRITE(0, EPN, BBH_UPS_ABORT_INT_STATUS, reg_bbh_ups_abort_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_ups_abort_int_status_get(bdmf_boolean *tardybbhabort)
{
    uint32_t reg_bbh_ups_abort_int_status=0;

#ifdef VALIDATE_PARMS
    if(!tardybbhabort)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BBH_UPS_ABORT_INT_STATUS, reg_bbh_ups_abort_int_status);

    *tardybbhabort = RU_FIELD_GET(0, EPN, BBH_UPS_ABORT_INT_STATUS, TARDYBBHABORT, reg_bbh_ups_abort_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_ups_abort_int_mask_set(bdmf_boolean masktardybbhabort)
{
    uint32_t reg_bbh_ups_abort_int_mask=0;

#ifdef VALIDATE_PARMS
    if((masktardybbhabort >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_ups_abort_int_mask = RU_FIELD_SET(0, EPN, BBH_UPS_ABORT_INT_MASK, MASKTARDYBBHABORT, reg_bbh_ups_abort_int_mask, masktardybbhabort);

    RU_REG_WRITE(0, EPN, BBH_UPS_ABORT_INT_MASK, reg_bbh_ups_abort_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_ups_abort_int_mask_get(bdmf_boolean *masktardybbhabort)
{
    uint32_t reg_bbh_ups_abort_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!masktardybbhabort)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BBH_UPS_ABORT_INT_MASK, reg_bbh_ups_abort_int_mask);

    *masktardybbhabort = RU_FIELD_GET(0, EPN, BBH_UPS_ABORT_INT_MASK, MASKTARDYBBHABORT, reg_bbh_ups_abort_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_main_int_mask_set(const epn_main_int_mask *main_int_mask)
{
    uint32_t reg_main_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!main_int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((main_int_mask->bbhupfrabortmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intl2sburstcapoverflowmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcoemptyrptmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intdrxerrabortmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intl2sfifooverrunmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intco1588tsmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcorptpresmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcogntpresmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcodelstalegntmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcogntnonpollmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcogntmisalignmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcognttoofarmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcogntintervalmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcogntdiscoverymask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcogntmissabortmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcogntfullabortmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->badupfrlenmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->uptardypacketmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->uprptfrxmtmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intbififooverrunmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->burstgnttoobigmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->wrgnttoobigmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->rcvgnttoobigmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->dnstatsoverrunmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intupstatsoverrunmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->dnoutofordermask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->truantbbhhaltmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->upinvldgntlenmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->intcobbhupsfaultmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->dntimeinsyncmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->dntimenotinsyncmask >= _1BITS_MAX_VAL_) ||
       (main_int_mask->dportrdymask >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, BBHUPFRABORTMASK, reg_main_int_mask, main_int_mask->bbhupfrabortmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTL2SBURSTCAPOVERFLOWMASK, reg_main_int_mask, main_int_mask->intl2sburstcapoverflowmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCOEMPTYRPTMASK, reg_main_int_mask, main_int_mask->intcoemptyrptmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTDRXERRABORTMASK, reg_main_int_mask, main_int_mask->intdrxerrabortmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTL2SFIFOOVERRUNMASK, reg_main_int_mask, main_int_mask->intl2sfifooverrunmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCO1588TSMASK, reg_main_int_mask, main_int_mask->intco1588tsmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCORPTPRESMASK, reg_main_int_mask, main_int_mask->intcorptpresmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCOGNTPRESMASK, reg_main_int_mask, main_int_mask->intcogntpresmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCODELSTALEGNTMASK, reg_main_int_mask, main_int_mask->intcodelstalegntmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCOGNTNONPOLLMASK, reg_main_int_mask, main_int_mask->intcogntnonpollmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCOGNTMISALIGNMASK, reg_main_int_mask, main_int_mask->intcogntmisalignmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCOGNTTOOFARMASK, reg_main_int_mask, main_int_mask->intcognttoofarmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCOGNTINTERVALMASK, reg_main_int_mask, main_int_mask->intcogntintervalmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCOGNTDISCOVERYMASK, reg_main_int_mask, main_int_mask->intcogntdiscoverymask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCOGNTMISSABORTMASK, reg_main_int_mask, main_int_mask->intcogntmissabortmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCOGNTFULLABORTMASK, reg_main_int_mask, main_int_mask->intcogntfullabortmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, BADUPFRLENMASK, reg_main_int_mask, main_int_mask->badupfrlenmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, UPTARDYPACKETMASK, reg_main_int_mask, main_int_mask->uptardypacketmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, UPRPTFRXMTMASK, reg_main_int_mask, main_int_mask->uprptfrxmtmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTBIFIFOOVERRUNMASK, reg_main_int_mask, main_int_mask->intbififooverrunmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, BURSTGNTTOOBIGMASK, reg_main_int_mask, main_int_mask->burstgnttoobigmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, WRGNTTOOBIGMASK, reg_main_int_mask, main_int_mask->wrgnttoobigmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, RCVGNTTOOBIGMASK, reg_main_int_mask, main_int_mask->rcvgnttoobigmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, DNSTATSOVERRUNMASK, reg_main_int_mask, main_int_mask->dnstatsoverrunmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTUPSTATSOVERRUNMASK, reg_main_int_mask, main_int_mask->intupstatsoverrunmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, DNOUTOFORDERMASK, reg_main_int_mask, main_int_mask->dnoutofordermask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, TRUANTBBHHALTMASK, reg_main_int_mask, main_int_mask->truantbbhhaltmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, UPINVLDGNTLENMASK, reg_main_int_mask, main_int_mask->upinvldgntlenmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, INTCOBBHUPSFAULTMASK, reg_main_int_mask, main_int_mask->intcobbhupsfaultmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, DNTIMEINSYNCMASK, reg_main_int_mask, main_int_mask->dntimeinsyncmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, DNTIMENOTINSYNCMASK, reg_main_int_mask, main_int_mask->dntimenotinsyncmask);
    reg_main_int_mask = RU_FIELD_SET(0, EPN, MAIN_INT_MASK, DPORTRDYMASK, reg_main_int_mask, main_int_mask->dportrdymask);

    RU_REG_WRITE(0, EPN, MAIN_INT_MASK, reg_main_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_main_int_mask_get(epn_main_int_mask *main_int_mask)
{
    uint32_t reg_main_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!main_int_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, MAIN_INT_MASK, reg_main_int_mask);

    main_int_mask->bbhupfrabortmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, BBHUPFRABORTMASK, reg_main_int_mask);
    main_int_mask->intl2sburstcapoverflowmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTL2SBURSTCAPOVERFLOWMASK, reg_main_int_mask);
    main_int_mask->intcoemptyrptmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCOEMPTYRPTMASK, reg_main_int_mask);
    main_int_mask->intdrxerrabortmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTDRXERRABORTMASK, reg_main_int_mask);
    main_int_mask->intl2sfifooverrunmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTL2SFIFOOVERRUNMASK, reg_main_int_mask);
    main_int_mask->intco1588tsmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCO1588TSMASK, reg_main_int_mask);
    main_int_mask->intcorptpresmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCORPTPRESMASK, reg_main_int_mask);
    main_int_mask->intcogntpresmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCOGNTPRESMASK, reg_main_int_mask);
    main_int_mask->intcodelstalegntmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCODELSTALEGNTMASK, reg_main_int_mask);
    main_int_mask->intcogntnonpollmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCOGNTNONPOLLMASK, reg_main_int_mask);
    main_int_mask->intcogntmisalignmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCOGNTMISALIGNMASK, reg_main_int_mask);
    main_int_mask->intcognttoofarmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCOGNTTOOFARMASK, reg_main_int_mask);
    main_int_mask->intcogntintervalmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCOGNTINTERVALMASK, reg_main_int_mask);
    main_int_mask->intcogntdiscoverymask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCOGNTDISCOVERYMASK, reg_main_int_mask);
    main_int_mask->intcogntmissabortmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCOGNTMISSABORTMASK, reg_main_int_mask);
    main_int_mask->intcogntfullabortmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCOGNTFULLABORTMASK, reg_main_int_mask);
    main_int_mask->badupfrlenmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, BADUPFRLENMASK, reg_main_int_mask);
    main_int_mask->uptardypacketmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, UPTARDYPACKETMASK, reg_main_int_mask);
    main_int_mask->uprptfrxmtmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, UPRPTFRXMTMASK, reg_main_int_mask);
    main_int_mask->intbififooverrunmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTBIFIFOOVERRUNMASK, reg_main_int_mask);
    main_int_mask->burstgnttoobigmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, BURSTGNTTOOBIGMASK, reg_main_int_mask);
    main_int_mask->wrgnttoobigmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, WRGNTTOOBIGMASK, reg_main_int_mask);
    main_int_mask->rcvgnttoobigmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, RCVGNTTOOBIGMASK, reg_main_int_mask);
    main_int_mask->dnstatsoverrunmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, DNSTATSOVERRUNMASK, reg_main_int_mask);
    main_int_mask->intupstatsoverrunmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTUPSTATSOVERRUNMASK, reg_main_int_mask);
    main_int_mask->dnoutofordermask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, DNOUTOFORDERMASK, reg_main_int_mask);
    main_int_mask->truantbbhhaltmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, TRUANTBBHHALTMASK, reg_main_int_mask);
    main_int_mask->upinvldgntlenmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, UPINVLDGNTLENMASK, reg_main_int_mask);
    main_int_mask->intcobbhupsfaultmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, INTCOBBHUPSFAULTMASK, reg_main_int_mask);
    main_int_mask->dntimeinsyncmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, DNTIMEINSYNCMASK, reg_main_int_mask);
    main_int_mask->dntimenotinsyncmask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, DNTIMENOTINSYNCMASK, reg_main_int_mask);
    main_int_mask->dportrdymask = RU_FIELD_GET(0, EPN, MAIN_INT_MASK, DPORTRDYMASK, reg_main_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_max_gnt_size_set(uint16_t maxgntsize)
{
    uint32_t reg_max_gnt_size=0;

#ifdef VALIDATE_PARMS
#endif

    reg_max_gnt_size = RU_FIELD_SET(0, EPN, MAX_GNT_SIZE, MAXGNTSIZE, reg_max_gnt_size, maxgntsize);

    RU_REG_WRITE(0, EPN, MAX_GNT_SIZE, reg_max_gnt_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_max_gnt_size_get(uint16_t *maxgntsize)
{
    uint32_t reg_max_gnt_size=0;

#ifdef VALIDATE_PARMS
    if(!maxgntsize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, MAX_GNT_SIZE, reg_max_gnt_size);

    *maxgntsize = RU_FIELD_GET(0, EPN, MAX_GNT_SIZE, MAXGNTSIZE, reg_max_gnt_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_max_frame_size_set(uint16_t cfgmaxframesize)
{
    uint32_t reg_max_frame_size=0;

#ifdef VALIDATE_PARMS
    if((cfgmaxframesize >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_max_frame_size = RU_FIELD_SET(0, EPN, MAX_FRAME_SIZE, CFGMAXFRAMESIZE, reg_max_frame_size, cfgmaxframesize);

    RU_REG_WRITE(0, EPN, MAX_FRAME_SIZE, reg_max_frame_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_max_frame_size_get(uint16_t *cfgmaxframesize)
{
    uint32_t reg_max_frame_size=0;

#ifdef VALIDATE_PARMS
    if(!cfgmaxframesize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, MAX_FRAME_SIZE, reg_max_frame_size);

    *cfgmaxframesize = RU_FIELD_GET(0, EPN, MAX_FRAME_SIZE, CFGMAXFRAMESIZE, reg_max_frame_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_grant_ovr_hd_set(uint16_t gntovrhdfec, uint16_t gntovrhd)
{
    uint32_t reg_grant_ovr_hd=0;

#ifdef VALIDATE_PARMS
#endif

    reg_grant_ovr_hd = RU_FIELD_SET(0, EPN, GRANT_OVR_HD, GNTOVRHDFEC, reg_grant_ovr_hd, gntovrhdfec);
    reg_grant_ovr_hd = RU_FIELD_SET(0, EPN, GRANT_OVR_HD, GNTOVRHD, reg_grant_ovr_hd, gntovrhd);

    RU_REG_WRITE(0, EPN, GRANT_OVR_HD, reg_grant_ovr_hd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_grant_ovr_hd_get(uint16_t *gntovrhdfec, uint16_t *gntovrhd)
{
    uint32_t reg_grant_ovr_hd=0;

#ifdef VALIDATE_PARMS
    if(!gntovrhdfec || !gntovrhd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GRANT_OVR_HD, reg_grant_ovr_hd);

    *gntovrhdfec = RU_FIELD_GET(0, EPN, GRANT_OVR_HD, GNTOVRHDFEC, reg_grant_ovr_hd);
    *gntovrhd = RU_FIELD_GET(0, EPN, GRANT_OVR_HD, GNTOVRHD, reg_grant_ovr_hd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_poll_size_set(uint16_t pollsizefec, uint16_t pollsize)
{
    uint32_t reg_poll_size=0;

#ifdef VALIDATE_PARMS
#endif

    reg_poll_size = RU_FIELD_SET(0, EPN, POLL_SIZE, POLLSIZEFEC, reg_poll_size, pollsizefec);
    reg_poll_size = RU_FIELD_SET(0, EPN, POLL_SIZE, POLLSIZE, reg_poll_size, pollsize);

    RU_REG_WRITE(0, EPN, POLL_SIZE, reg_poll_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_poll_size_get(uint16_t *pollsizefec, uint16_t *pollsize)
{
    uint32_t reg_poll_size=0;

#ifdef VALIDATE_PARMS
    if(!pollsizefec || !pollsize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, POLL_SIZE, reg_poll_size);

    *pollsizefec = RU_FIELD_GET(0, EPN, POLL_SIZE, POLLSIZEFEC, reg_poll_size);
    *pollsize = RU_FIELD_GET(0, EPN, POLL_SIZE, POLLSIZE, reg_poll_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_rd_gnt_margin_set(uint16_t rdgntstartmargin)
{
    uint32_t reg_dn_rd_gnt_margin=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dn_rd_gnt_margin = RU_FIELD_SET(0, EPN, DN_RD_GNT_MARGIN, RDGNTSTARTMARGIN, reg_dn_rd_gnt_margin, rdgntstartmargin);

    RU_REG_WRITE(0, EPN, DN_RD_GNT_MARGIN, reg_dn_rd_gnt_margin);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_rd_gnt_margin_get(uint16_t *rdgntstartmargin)
{
    uint32_t reg_dn_rd_gnt_margin=0;

#ifdef VALIDATE_PARMS
    if(!rdgntstartmargin)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DN_RD_GNT_MARGIN, reg_dn_rd_gnt_margin);

    *rdgntstartmargin = RU_FIELD_GET(0, EPN, DN_RD_GNT_MARGIN, RDGNTSTARTMARGIN, reg_dn_rd_gnt_margin);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_time_start_delta_set(uint16_t gntstarttimedelta)
{
    uint32_t reg_gnt_time_start_delta=0;

#ifdef VALIDATE_PARMS
#endif

    reg_gnt_time_start_delta = RU_FIELD_SET(0, EPN, GNT_TIME_START_DELTA, GNTSTARTTIMEDELTA, reg_gnt_time_start_delta, gntstarttimedelta);

    RU_REG_WRITE(0, EPN, GNT_TIME_START_DELTA, reg_gnt_time_start_delta);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_time_start_delta_get(uint16_t *gntstarttimedelta)
{
    uint32_t reg_gnt_time_start_delta=0;

#ifdef VALIDATE_PARMS
    if(!gntstarttimedelta)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_TIME_START_DELTA, reg_gnt_time_start_delta);

    *gntstarttimedelta = RU_FIELD_GET(0, EPN, GNT_TIME_START_DELTA, GNTSTARTTIMEDELTA, reg_gnt_time_start_delta);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_time_stamp_diff_set(uint16_t timestampdiffdelta)
{
    uint32_t reg_time_stamp_diff=0;

#ifdef VALIDATE_PARMS
#endif

    reg_time_stamp_diff = RU_FIELD_SET(0, EPN, TIME_STAMP_DIFF, TIMESTAMPDIFFDELTA, reg_time_stamp_diff, timestampdiffdelta);

    RU_REG_WRITE(0, EPN, TIME_STAMP_DIFF, reg_time_stamp_diff);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_time_stamp_diff_get(uint16_t *timestampdiffdelta)
{
    uint32_t reg_time_stamp_diff=0;

#ifdef VALIDATE_PARMS
    if(!timestampdiffdelta)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, TIME_STAMP_DIFF, reg_time_stamp_diff);

    *timestampdiffdelta = RU_FIELD_GET(0, EPN, TIME_STAMP_DIFF, TIMESTAMPDIFFDELTA, reg_time_stamp_diff);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_up_time_stamp_off_set(uint16_t timestampoffsetfec, uint16_t timestampoffset)
{
    uint32_t reg_up_time_stamp_off=0;

#ifdef VALIDATE_PARMS
#endif

    reg_up_time_stamp_off = RU_FIELD_SET(0, EPN, UP_TIME_STAMP_OFF, TIMESTAMPOFFSETFEC, reg_up_time_stamp_off, timestampoffsetfec);
    reg_up_time_stamp_off = RU_FIELD_SET(0, EPN, UP_TIME_STAMP_OFF, TIMESTAMPOFFSET, reg_up_time_stamp_off, timestampoffset);

    RU_REG_WRITE(0, EPN, UP_TIME_STAMP_OFF, reg_up_time_stamp_off);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_up_time_stamp_off_get(uint16_t *timestampoffsetfec, uint16_t *timestampoffset)
{
    uint32_t reg_up_time_stamp_off=0;

#ifdef VALIDATE_PARMS
    if(!timestampoffsetfec || !timestampoffset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, UP_TIME_STAMP_OFF, reg_up_time_stamp_off);

    *timestampoffsetfec = RU_FIELD_GET(0, EPN, UP_TIME_STAMP_OFF, TIMESTAMPOFFSETFEC, reg_up_time_stamp_off);
    *timestampoffset = RU_FIELD_GET(0, EPN, UP_TIME_STAMP_OFF, TIMESTAMPOFFSET, reg_up_time_stamp_off);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_interval_set(uint16_t gntinterval)
{
    uint32_t reg_gnt_interval=0;

#ifdef VALIDATE_PARMS
#endif

    reg_gnt_interval = RU_FIELD_SET(0, EPN, GNT_INTERVAL, GNTINTERVAL, reg_gnt_interval, gntinterval);

    RU_REG_WRITE(0, EPN, GNT_INTERVAL, reg_gnt_interval);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_gnt_interval_get(uint16_t *gntinterval)
{
    uint32_t reg_gnt_interval=0;

#ifdef VALIDATE_PARMS
    if(!gntinterval)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GNT_INTERVAL, reg_gnt_interval);

    *gntinterval = RU_FIELD_GET(0, EPN, GNT_INTERVAL, GNTINTERVAL, reg_gnt_interval);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_gnt_misalign_thr_set(uint16_t prvunusedgntthreshold, uint16_t gntmisalignthresh)
{
    uint32_t reg_dn_gnt_misalign_thr=0;

#ifdef VALIDATE_PARMS
    if((gntmisalignthresh >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dn_gnt_misalign_thr = RU_FIELD_SET(0, EPN, DN_GNT_MISALIGN_THR, PRVUNUSEDGNTTHRESHOLD, reg_dn_gnt_misalign_thr, prvunusedgntthreshold);
    reg_dn_gnt_misalign_thr = RU_FIELD_SET(0, EPN, DN_GNT_MISALIGN_THR, GNTMISALIGNTHRESH, reg_dn_gnt_misalign_thr, gntmisalignthresh);

    RU_REG_WRITE(0, EPN, DN_GNT_MISALIGN_THR, reg_dn_gnt_misalign_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_gnt_misalign_thr_get(uint16_t *prvunusedgntthreshold, uint16_t *gntmisalignthresh)
{
    uint32_t reg_dn_gnt_misalign_thr=0;

#ifdef VALIDATE_PARMS
    if(!prvunusedgntthreshold || !gntmisalignthresh)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DN_GNT_MISALIGN_THR, reg_dn_gnt_misalign_thr);

    *prvunusedgntthreshold = RU_FIELD_GET(0, EPN, DN_GNT_MISALIGN_THR, PRVUNUSEDGNTTHRESHOLD, reg_dn_gnt_misalign_thr);
    *gntmisalignthresh = RU_FIELD_GET(0, EPN, DN_GNT_MISALIGN_THR, GNTMISALIGNTHRESH, reg_dn_gnt_misalign_thr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_gnt_misalign_pause_set(uint16_t gntmisalignpause)
{
    uint32_t reg_dn_gnt_misalign_pause=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dn_gnt_misalign_pause = RU_FIELD_SET(0, EPN, DN_GNT_MISALIGN_PAUSE, GNTMISALIGNPAUSE, reg_dn_gnt_misalign_pause, gntmisalignpause);

    RU_REG_WRITE(0, EPN, DN_GNT_MISALIGN_PAUSE, reg_dn_gnt_misalign_pause);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_gnt_misalign_pause_get(uint16_t *gntmisalignpause)
{
    uint32_t reg_dn_gnt_misalign_pause=0;

#ifdef VALIDATE_PARMS
    if(!gntmisalignpause)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DN_GNT_MISALIGN_PAUSE, reg_dn_gnt_misalign_pause);

    *gntmisalignpause = RU_FIELD_GET(0, EPN, DN_GNT_MISALIGN_PAUSE, GNTMISALIGNPAUSE, reg_dn_gnt_misalign_pause);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_non_poll_intv_set(uint16_t nonpollgntintv)
{
    uint32_t reg_non_poll_intv=0;

#ifdef VALIDATE_PARMS
#endif

    reg_non_poll_intv = RU_FIELD_SET(0, EPN, NON_POLL_INTV, NONPOLLGNTINTV, reg_non_poll_intv, nonpollgntintv);

    RU_REG_WRITE(0, EPN, NON_POLL_INTV, reg_non_poll_intv);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_non_poll_intv_get(uint16_t *nonpollgntintv)
{
    uint32_t reg_non_poll_intv=0;

#ifdef VALIDATE_PARMS
    if(!nonpollgntintv)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, NON_POLL_INTV, reg_non_poll_intv);

    *nonpollgntintv = RU_FIELD_GET(0, EPN, NON_POLL_INTV, NONPOLLGNTINTV, reg_non_poll_intv);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_force_fcs_err_set(uint8_t link_idx, bdmf_boolean forceFcsErr)
{
    uint32_t reg_force_fcs_err=0;

#ifdef VALIDATE_PARMS
    if((forceFcsErr >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(0, EPN, FORCE_FCS_ERR, reg_force_fcs_err);

    FIELD_SET(reg_force_fcs_err, (link_idx % 32) *1, 0x1, forceFcsErr);

    RU_REG_WRITE(0, EPN, FORCE_FCS_ERR, reg_force_fcs_err);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_force_fcs_err_get(uint8_t link_idx, bdmf_boolean *forceFcsErr)
{
    uint32_t reg_force_fcs_err=0;

#ifdef VALIDATE_PARMS
    if(!forceFcsErr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, FORCE_FCS_ERR, reg_force_fcs_err);

    *forceFcsErr = FIELD_GET(reg_force_fcs_err, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_grant_overlap_limit_set(uint16_t prvgrantoverlaplimit)
{
    uint32_t reg_grant_overlap_limit=0;

#ifdef VALIDATE_PARMS
#endif

    reg_grant_overlap_limit = RU_FIELD_SET(0, EPN, GRANT_OVERLAP_LIMIT, PRVGRANTOVERLAPLIMIT, reg_grant_overlap_limit, prvgrantoverlaplimit);

    RU_REG_WRITE(0, EPN, GRANT_OVERLAP_LIMIT, reg_grant_overlap_limit);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_grant_overlap_limit_get(uint16_t *prvgrantoverlaplimit)
{
    uint32_t reg_grant_overlap_limit=0;

#ifdef VALIDATE_PARMS
    if(!prvgrantoverlaplimit)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, GRANT_OVERLAP_LIMIT, reg_grant_overlap_limit);

    *prvgrantoverlaplimit = RU_FIELD_GET(0, EPN, GRANT_OVERLAP_LIMIT, PRVGRANTOVERLAPLIMIT, reg_grant_overlap_limit);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_aes_configuration_0_set(uint8_t link_idx, uint8_t prvUpstreamAesMode_0)
{
    uint32_t reg_aes_configuration_0=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, EPN, AES_CONFIGURATION_0, reg_aes_configuration_0);

    FIELD_SET(reg_aes_configuration_0, (link_idx % 16) *2, 0x2, prvUpstreamAesMode_0);

    RU_REG_WRITE(0, EPN, AES_CONFIGURATION_0, reg_aes_configuration_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_aes_configuration_0_get(uint8_t link_idx, uint8_t *prvUpstreamAesMode_0)
{
    uint32_t reg_aes_configuration_0=0;

#ifdef VALIDATE_PARMS
    if(!prvUpstreamAesMode_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, AES_CONFIGURATION_0, reg_aes_configuration_0);

    *prvUpstreamAesMode_0 = FIELD_GET(reg_aes_configuration_0, (link_idx % 16) *2, 0x2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_aes_configuration_1_set(uint8_t link_idx, uint8_t prvUpstreamAesMode_1)
{
    uint32_t reg_aes_configuration_1=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, EPN, AES_CONFIGURATION_1, reg_aes_configuration_1);

    FIELD_SET(reg_aes_configuration_1, (link_idx % 16) *2, 0x2, prvUpstreamAesMode_1);

    RU_REG_WRITE(0, EPN, AES_CONFIGURATION_1, reg_aes_configuration_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_aes_configuration_1_get(uint8_t link_idx, uint8_t *prvUpstreamAesMode_1)
{
    uint32_t reg_aes_configuration_1=0;

#ifdef VALIDATE_PARMS
    if(!prvUpstreamAesMode_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, AES_CONFIGURATION_1, reg_aes_configuration_1);

    *prvUpstreamAesMode_1 = FIELD_GET(reg_aes_configuration_1, (link_idx % 16) *2, 0x2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_disc_grant_ovr_hd_set(uint16_t discgntovrhd)
{
    uint32_t reg_disc_grant_ovr_hd=0;

#ifdef VALIDATE_PARMS
#endif

    reg_disc_grant_ovr_hd = RU_FIELD_SET(0, EPN, DISC_GRANT_OVR_HD, DISCGNTOVRHD, reg_disc_grant_ovr_hd, discgntovrhd);

    RU_REG_WRITE(0, EPN, DISC_GRANT_OVR_HD, reg_disc_grant_ovr_hd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_disc_grant_ovr_hd_get(uint16_t *discgntovrhd)
{
    uint32_t reg_disc_grant_ovr_hd=0;

#ifdef VALIDATE_PARMS
    if(!discgntovrhd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DISC_GRANT_OVR_HD, reg_disc_grant_ovr_hd);

    *discgntovrhd = RU_FIELD_GET(0, EPN, DISC_GRANT_OVR_HD, DISCGNTOVRHD, reg_disc_grant_ovr_hd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_discovery_seed_set(uint16_t cfgdiscseed)
{
    uint32_t reg_dn_discovery_seed=0;

#ifdef VALIDATE_PARMS
    if((cfgdiscseed >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dn_discovery_seed = RU_FIELD_SET(0, EPN, DN_DISCOVERY_SEED, CFGDISCSEED, reg_dn_discovery_seed, cfgdiscseed);

    RU_REG_WRITE(0, EPN, DN_DISCOVERY_SEED, reg_dn_discovery_seed);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_discovery_seed_get(uint16_t *cfgdiscseed)
{
    uint32_t reg_dn_discovery_seed=0;

#ifdef VALIDATE_PARMS
    if(!cfgdiscseed)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DN_DISCOVERY_SEED, reg_dn_discovery_seed);

    *cfgdiscseed = RU_FIELD_GET(0, EPN, DN_DISCOVERY_SEED, CFGDISCSEED, reg_dn_discovery_seed);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_discovery_inc_set(uint16_t cfgdiscinc)
{
    uint32_t reg_dn_discovery_inc=0;

#ifdef VALIDATE_PARMS
    if((cfgdiscinc >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dn_discovery_inc = RU_FIELD_SET(0, EPN, DN_DISCOVERY_INC, CFGDISCINC, reg_dn_discovery_inc, cfgdiscinc);

    RU_REG_WRITE(0, EPN, DN_DISCOVERY_INC, reg_dn_discovery_inc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_discovery_inc_get(uint16_t *cfgdiscinc)
{
    uint32_t reg_dn_discovery_inc=0;

#ifdef VALIDATE_PARMS
    if(!cfgdiscinc)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DN_DISCOVERY_INC, reg_dn_discovery_inc);

    *cfgdiscinc = RU_FIELD_GET(0, EPN, DN_DISCOVERY_INC, CFGDISCINC, reg_dn_discovery_inc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_discovery_size_set(uint16_t cfgdiscsize)
{
    uint32_t reg_dn_discovery_size=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dn_discovery_size = RU_FIELD_SET(0, EPN, DN_DISCOVERY_SIZE, CFGDISCSIZE, reg_dn_discovery_size, cfgdiscsize);

    RU_REG_WRITE(0, EPN, DN_DISCOVERY_SIZE, reg_dn_discovery_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_discovery_size_get(uint16_t *cfgdiscsize)
{
    uint32_t reg_dn_discovery_size=0;

#ifdef VALIDATE_PARMS
    if(!cfgdiscsize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DN_DISCOVERY_SIZE, reg_dn_discovery_size);

    *cfgdiscsize = RU_FIELD_GET(0, EPN, DN_DISCOVERY_SIZE, CFGDISCSIZE, reg_dn_discovery_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_prog_rpt_vec_set(uint32_t progrptvec)
{
    uint32_t reg_prog_rpt_vec=0;

#ifdef VALIDATE_PARMS
#endif

    reg_prog_rpt_vec = RU_FIELD_SET(0, EPN, PROG_RPT_VEC, PROGRPTVEC, reg_prog_rpt_vec, progrptvec);

    RU_REG_WRITE(0, EPN, PROG_RPT_VEC, reg_prog_rpt_vec);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_prog_rpt_vec_get(uint32_t *progrptvec)
{
    uint32_t reg_prog_rpt_vec=0;

#ifdef VALIDATE_PARMS
    if(!progrptvec)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, PROG_RPT_VEC, reg_prog_rpt_vec);

    *progrptvec = RU_FIELD_GET(0, EPN, PROG_RPT_VEC, PROGRPTVEC, reg_prog_rpt_vec);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_fec_ipg_length_set(uint8_t modipgpreamblebytes, uint8_t cfgrptlen, uint8_t cfgfecrptlength, uint8_t cfgfecipglength)
{
    uint32_t reg_fec_ipg_length=0;

#ifdef VALIDATE_PARMS
    if((modipgpreamblebytes >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fec_ipg_length = RU_FIELD_SET(0, EPN, FEC_IPG_LENGTH, MODIPGPREAMBLEBYTES, reg_fec_ipg_length, modipgpreamblebytes);
    reg_fec_ipg_length = RU_FIELD_SET(0, EPN, FEC_IPG_LENGTH, CFGRPTLEN, reg_fec_ipg_length, cfgrptlen);
    reg_fec_ipg_length = RU_FIELD_SET(0, EPN, FEC_IPG_LENGTH, CFGFECRPTLENGTH, reg_fec_ipg_length, cfgfecrptlength);
    reg_fec_ipg_length = RU_FIELD_SET(0, EPN, FEC_IPG_LENGTH, CFGFECIPGLENGTH, reg_fec_ipg_length, cfgfecipglength);

    RU_REG_WRITE(0, EPN, FEC_IPG_LENGTH, reg_fec_ipg_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_fec_ipg_length_get(uint8_t *modipgpreamblebytes, uint8_t *cfgrptlen, uint8_t *cfgfecrptlength, uint8_t *cfgfecipglength)
{
    uint32_t reg_fec_ipg_length=0;

#ifdef VALIDATE_PARMS
    if(!modipgpreamblebytes || !cfgrptlen || !cfgfecrptlength || !cfgfecipglength)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, FEC_IPG_LENGTH, reg_fec_ipg_length);

    *modipgpreamblebytes = RU_FIELD_GET(0, EPN, FEC_IPG_LENGTH, MODIPGPREAMBLEBYTES, reg_fec_ipg_length);
    *cfgrptlen = RU_FIELD_GET(0, EPN, FEC_IPG_LENGTH, CFGRPTLEN, reg_fec_ipg_length);
    *cfgfecrptlength = RU_FIELD_GET(0, EPN, FEC_IPG_LENGTH, CFGFECRPTLENGTH, reg_fec_ipg_length);
    *cfgfecipglength = RU_FIELD_GET(0, EPN, FEC_IPG_LENGTH, CFGFECIPGLENGTH, reg_fec_ipg_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_fake_report_value_en_set(uint32_t fakereportvalueen)
{
    uint32_t reg_fake_report_value_en=0;

#ifdef VALIDATE_PARMS
#endif

    reg_fake_report_value_en = RU_FIELD_SET(0, EPN, FAKE_REPORT_VALUE_EN, FAKEREPORTVALUEEN, reg_fake_report_value_en, fakereportvalueen);

    RU_REG_WRITE(0, EPN, FAKE_REPORT_VALUE_EN, reg_fake_report_value_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_fake_report_value_en_get(uint32_t *fakereportvalueen)
{
    uint32_t reg_fake_report_value_en=0;

#ifdef VALIDATE_PARMS
    if(!fakereportvalueen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, FAKE_REPORT_VALUE_EN, reg_fake_report_value_en);

    *fakereportvalueen = RU_FIELD_GET(0, EPN, FAKE_REPORT_VALUE_EN, FAKEREPORTVALUEEN, reg_fake_report_value_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_fake_report_value_set(uint32_t fakereportvalue)
{
    uint32_t reg_fake_report_value=0;

#ifdef VALIDATE_PARMS
    if((fakereportvalue >= _21BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fake_report_value = RU_FIELD_SET(0, EPN, FAKE_REPORT_VALUE, FAKEREPORTVALUE, reg_fake_report_value, fakereportvalue);

    RU_REG_WRITE(0, EPN, FAKE_REPORT_VALUE, reg_fake_report_value);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_fake_report_value_get(uint32_t *fakereportvalue)
{
    uint32_t reg_fake_report_value=0;

#ifdef VALIDATE_PARMS
    if(!fakereportvalue)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, FAKE_REPORT_VALUE, reg_fake_report_value);

    *fakereportvalue = RU_FIELD_GET(0, EPN, FAKE_REPORT_VALUE, FAKEREPORTVALUE, reg_fake_report_value);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_burst_cap_set(uint8_t link_idx, uint32_t burstcap)
{
    uint32_t reg_burst_cap_=0;

#ifdef VALIDATE_PARMS
    if((link_idx >= 32) ||
       (burstcap >= _20BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_burst_cap_ = RU_FIELD_SET(0, EPN, BURST_CAP_, BURSTCAP, reg_burst_cap_, burstcap);

    RU_REG_RAM_WRITE(0, link_idx, EPN, BURST_CAP_, reg_burst_cap_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_burst_cap_get(uint8_t link_idx, uint32_t *burstcap)
{
    uint32_t reg_burst_cap_=0;

#ifdef VALIDATE_PARMS
    if(!burstcap)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((link_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, link_idx, EPN, BURST_CAP_, reg_burst_cap_);

    *burstcap = RU_FIELD_GET(0, EPN, BURST_CAP_, BURSTCAP, reg_burst_cap_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_queue_llid_map_set(uint8_t que_idx, uint8_t quellidmap)
{
    uint32_t reg_queue_llid_map_=0;

#ifdef VALIDATE_PARMS
    if((que_idx >= 32) ||
       (quellidmap >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_queue_llid_map_ = RU_FIELD_SET(0, EPN, QUEUE_LLID_MAP_, QUELLIDMAP, reg_queue_llid_map_, quellidmap);

    RU_REG_RAM_WRITE(0, que_idx, EPN, QUEUE_LLID_MAP_, reg_queue_llid_map_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_queue_llid_map_get(uint8_t que_idx, uint8_t *quellidmap)
{
    uint32_t reg_queue_llid_map_=0;

#ifdef VALIDATE_PARMS
    if(!quellidmap)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((que_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, que_idx, EPN, QUEUE_LLID_MAP_, reg_queue_llid_map_);

    *quellidmap = RU_FIELD_GET(0, EPN, QUEUE_LLID_MAP_, QUELLIDMAP, reg_queue_llid_map_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_valid_opcode_map_set(uint16_t prvvalidmpcpopcodes)
{
    uint32_t reg_valid_opcode_map=0;

#ifdef VALIDATE_PARMS
#endif

    reg_valid_opcode_map = RU_FIELD_SET(0, EPN, VALID_OPCODE_MAP, PRVVALIDMPCPOPCODES, reg_valid_opcode_map, prvvalidmpcpopcodes);

    RU_REG_WRITE(0, EPN, VALID_OPCODE_MAP, reg_valid_opcode_map);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_valid_opcode_map_get(uint16_t *prvvalidmpcpopcodes)
{
    uint32_t reg_valid_opcode_map=0;

#ifdef VALIDATE_PARMS
    if(!prvvalidmpcpopcodes)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, VALID_OPCODE_MAP, reg_valid_opcode_map);

    *prvvalidmpcpopcodes = RU_FIELD_GET(0, EPN, VALID_OPCODE_MAP, PRVVALIDMPCPOPCODES, reg_valid_opcode_map);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_up_packet_tx_margin_set(uint16_t uppackettxmargin)
{
    uint32_t reg_up_packet_tx_margin=0;

#ifdef VALIDATE_PARMS
#endif

    reg_up_packet_tx_margin = RU_FIELD_SET(0, EPN, UP_PACKET_TX_MARGIN, UPPACKETTXMARGIN, reg_up_packet_tx_margin, uppackettxmargin);

    RU_REG_WRITE(0, EPN, UP_PACKET_TX_MARGIN, reg_up_packet_tx_margin);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_up_packet_tx_margin_get(uint16_t *uppackettxmargin)
{
    uint32_t reg_up_packet_tx_margin=0;

#ifdef VALIDATE_PARMS
    if(!uppackettxmargin)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, UP_PACKET_TX_MARGIN, reg_up_packet_tx_margin);

    *uppackettxmargin = RU_FIELD_GET(0, EPN, UP_PACKET_TX_MARGIN, UPPACKETTXMARGIN, reg_up_packet_tx_margin);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_multi_pri_cfg_0_set(const epn_multi_pri_cfg_0 *multi_pri_cfg_0)
{
    uint32_t reg_multi_pri_cfg_0=0;

#ifdef VALIDATE_PARMS
    if(!multi_pri_cfg_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((multi_pri_cfg_0->cfgctcschdeficiten >= _1BITS_MAX_VAL_) ||
       (multi_pri_cfg_0->prvzeroburstcapoverridemode >= _2BITS_MAX_VAL_) ||
       (multi_pri_cfg_0->cfgsharedl2 >= _1BITS_MAX_VAL_) ||
       (multi_pri_cfg_0->cfgsharedburstcap >= _1BITS_MAX_VAL_) ||
       (multi_pri_cfg_0->cfgrptgntsoutst0 >= _1BITS_MAX_VAL_) ||
       (multi_pri_cfg_0->cfgrpthiprifirst0 >= _1BITS_MAX_VAL_) ||
       (multi_pri_cfg_0->cfgrptswapqs0 >= _1BITS_MAX_VAL_) ||
       (multi_pri_cfg_0->cfgrptmultipri0 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_multi_pri_cfg_0 = RU_FIELD_SET(0, EPN, MULTI_PRI_CFG_0, CFGCTCSCHDEFICITEN, reg_multi_pri_cfg_0, multi_pri_cfg_0->cfgctcschdeficiten);
    reg_multi_pri_cfg_0 = RU_FIELD_SET(0, EPN, MULTI_PRI_CFG_0, PRVZEROBURSTCAPOVERRIDEMODE, reg_multi_pri_cfg_0, multi_pri_cfg_0->prvzeroburstcapoverridemode);
    reg_multi_pri_cfg_0 = RU_FIELD_SET(0, EPN, MULTI_PRI_CFG_0, CFGSHAREDL2, reg_multi_pri_cfg_0, multi_pri_cfg_0->cfgsharedl2);
    reg_multi_pri_cfg_0 = RU_FIELD_SET(0, EPN, MULTI_PRI_CFG_0, CFGSHAREDBURSTCAP, reg_multi_pri_cfg_0, multi_pri_cfg_0->cfgsharedburstcap);
    reg_multi_pri_cfg_0 = RU_FIELD_SET(0, EPN, MULTI_PRI_CFG_0, CFGRPTGNTSOUTST0, reg_multi_pri_cfg_0, multi_pri_cfg_0->cfgrptgntsoutst0);
    reg_multi_pri_cfg_0 = RU_FIELD_SET(0, EPN, MULTI_PRI_CFG_0, CFGRPTHIPRIFIRST0, reg_multi_pri_cfg_0, multi_pri_cfg_0->cfgrpthiprifirst0);
    reg_multi_pri_cfg_0 = RU_FIELD_SET(0, EPN, MULTI_PRI_CFG_0, CFGRPTSWAPQS0, reg_multi_pri_cfg_0, multi_pri_cfg_0->cfgrptswapqs0);
    reg_multi_pri_cfg_0 = RU_FIELD_SET(0, EPN, MULTI_PRI_CFG_0, CFGRPTMULTIPRI0, reg_multi_pri_cfg_0, multi_pri_cfg_0->cfgrptmultipri0);

    RU_REG_WRITE(0, EPN, MULTI_PRI_CFG_0, reg_multi_pri_cfg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_multi_pri_cfg_0_get(epn_multi_pri_cfg_0 *multi_pri_cfg_0)
{
    uint32_t reg_multi_pri_cfg_0=0;

#ifdef VALIDATE_PARMS
    if(!multi_pri_cfg_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, MULTI_PRI_CFG_0, reg_multi_pri_cfg_0);

    multi_pri_cfg_0->cfgctcschdeficiten = RU_FIELD_GET(0, EPN, MULTI_PRI_CFG_0, CFGCTCSCHDEFICITEN, reg_multi_pri_cfg_0);
    multi_pri_cfg_0->prvzeroburstcapoverridemode = RU_FIELD_GET(0, EPN, MULTI_PRI_CFG_0, PRVZEROBURSTCAPOVERRIDEMODE, reg_multi_pri_cfg_0);
    multi_pri_cfg_0->cfgsharedl2 = RU_FIELD_GET(0, EPN, MULTI_PRI_CFG_0, CFGSHAREDL2, reg_multi_pri_cfg_0);
    multi_pri_cfg_0->cfgsharedburstcap = RU_FIELD_GET(0, EPN, MULTI_PRI_CFG_0, CFGSHAREDBURSTCAP, reg_multi_pri_cfg_0);
    multi_pri_cfg_0->cfgrptgntsoutst0 = RU_FIELD_GET(0, EPN, MULTI_PRI_CFG_0, CFGRPTGNTSOUTST0, reg_multi_pri_cfg_0);
    multi_pri_cfg_0->cfgrpthiprifirst0 = RU_FIELD_GET(0, EPN, MULTI_PRI_CFG_0, CFGRPTHIPRIFIRST0, reg_multi_pri_cfg_0);
    multi_pri_cfg_0->cfgrptswapqs0 = RU_FIELD_GET(0, EPN, MULTI_PRI_CFG_0, CFGRPTSWAPQS0, reg_multi_pri_cfg_0);
    multi_pri_cfg_0->cfgrptmultipri0 = RU_FIELD_GET(0, EPN, MULTI_PRI_CFG_0, CFGRPTMULTIPRI0, reg_multi_pri_cfg_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_shared_bcap_ovrflow_get(uint16_t *sharedburstcapoverflow)
{
    uint32_t reg_shared_bcap_ovrflow=0;

#ifdef VALIDATE_PARMS
    if(!sharedburstcapoverflow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, SHARED_BCAP_OVRFLOW, reg_shared_bcap_ovrflow);

    *sharedburstcapoverflow = RU_FIELD_GET(0, EPN, SHARED_BCAP_OVRFLOW, SHAREDBURSTCAPOVERFLOW, reg_shared_bcap_ovrflow);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_forced_report_en_set(uint8_t link_idx, bdmf_boolean cfgForceReportEn)
{
    uint32_t reg_forced_report_en=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, EPN, FORCED_REPORT_EN, reg_forced_report_en);

    FIELD_SET(reg_forced_report_en, (link_idx % 32) *1, 0x1, cfgForceReportEn);

    RU_REG_WRITE(0, EPN, FORCED_REPORT_EN, reg_forced_report_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_forced_report_en_get(uint8_t link_idx, bdmf_boolean *cfgForceReportEn)
{
    uint32_t reg_forced_report_en=0;

#ifdef VALIDATE_PARMS
    if(!cfgForceReportEn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, FORCED_REPORT_EN, reg_forced_report_en);

    *cfgForceReportEn = FIELD_GET(reg_forced_report_en, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_forced_report_max_interval_set(uint8_t cfgmaxreportinterval)
{
    uint32_t reg_forced_report_max_interval=0;

#ifdef VALIDATE_PARMS
    if((cfgmaxreportinterval >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_forced_report_max_interval = RU_FIELD_SET(0, EPN, FORCED_REPORT_MAX_INTERVAL, CFGMAXREPORTINTERVAL, reg_forced_report_max_interval, cfgmaxreportinterval);

    RU_REG_WRITE(0, EPN, FORCED_REPORT_MAX_INTERVAL, reg_forced_report_max_interval);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_forced_report_max_interval_get(uint8_t *cfgmaxreportinterval)
{
    uint32_t reg_forced_report_max_interval=0;

#ifdef VALIDATE_PARMS
    if(!cfgmaxreportinterval)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, FORCED_REPORT_MAX_INTERVAL, reg_forced_report_max_interval);

    *cfgmaxreportinterval = RU_FIELD_GET(0, EPN, FORCED_REPORT_MAX_INTERVAL, CFGMAXREPORTINTERVAL, reg_forced_report_max_interval);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_l2s_flush_config_set(bdmf_boolean cfgflushl2sen, bdmf_boolean flushl2sdone, uint8_t cfgflushl2ssel)
{
    uint32_t reg_l2s_flush_config=0;

#ifdef VALIDATE_PARMS
    if((cfgflushl2sen >= _1BITS_MAX_VAL_) ||
       (flushl2sdone >= _1BITS_MAX_VAL_) ||
       (cfgflushl2ssel >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_l2s_flush_config = RU_FIELD_SET(0, EPN, L2S_FLUSH_CONFIG, CFGFLUSHL2SEN, reg_l2s_flush_config, cfgflushl2sen);
    reg_l2s_flush_config = RU_FIELD_SET(0, EPN, L2S_FLUSH_CONFIG, FLUSHL2SDONE, reg_l2s_flush_config, flushl2sdone);
    reg_l2s_flush_config = RU_FIELD_SET(0, EPN, L2S_FLUSH_CONFIG, CFGFLUSHL2SSEL, reg_l2s_flush_config, cfgflushl2ssel);

    RU_REG_WRITE(0, EPN, L2S_FLUSH_CONFIG, reg_l2s_flush_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_l2s_flush_config_get(bdmf_boolean *cfgflushl2sen, bdmf_boolean *flushl2sdone, uint8_t *cfgflushl2ssel)
{
    uint32_t reg_l2s_flush_config=0;

#ifdef VALIDATE_PARMS
    if(!cfgflushl2sen || !flushl2sdone || !cfgflushl2ssel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, L2S_FLUSH_CONFIG, reg_l2s_flush_config);

    *cfgflushl2sen = RU_FIELD_GET(0, EPN, L2S_FLUSH_CONFIG, CFGFLUSHL2SEN, reg_l2s_flush_config);
    *flushl2sdone = RU_FIELD_GET(0, EPN, L2S_FLUSH_CONFIG, FLUSHL2SDONE, reg_l2s_flush_config);
    *cfgflushl2ssel = RU_FIELD_GET(0, EPN, L2S_FLUSH_CONFIG, CFGFLUSHL2SSEL, reg_l2s_flush_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_data_port_command_set(bdmf_boolean dportbusy, uint8_t dportselect, bdmf_boolean dportcontrol)
{
    uint32_t reg_data_port_command=0;

#ifdef VALIDATE_PARMS
    if((dportbusy >= _1BITS_MAX_VAL_) ||
       (dportselect >= _5BITS_MAX_VAL_) ||
       (dportcontrol >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_data_port_command = RU_FIELD_SET(0, EPN, DATA_PORT_COMMAND, DPORTBUSY, reg_data_port_command, dportbusy);
    reg_data_port_command = RU_FIELD_SET(0, EPN, DATA_PORT_COMMAND, DPORTSELECT, reg_data_port_command, dportselect);
    reg_data_port_command = RU_FIELD_SET(0, EPN, DATA_PORT_COMMAND, DPORTCONTROL, reg_data_port_command, dportcontrol);

    RU_REG_WRITE(0, EPN, DATA_PORT_COMMAND, reg_data_port_command);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_data_port_command_get(bdmf_boolean *dportbusy, uint8_t *dportselect, bdmf_boolean *dportcontrol)
{
    uint32_t reg_data_port_command=0;

#ifdef VALIDATE_PARMS
    if(!dportbusy || !dportselect || !dportcontrol)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DATA_PORT_COMMAND, reg_data_port_command);

    *dportbusy = RU_FIELD_GET(0, EPN, DATA_PORT_COMMAND, DPORTBUSY, reg_data_port_command);
    *dportselect = RU_FIELD_GET(0, EPN, DATA_PORT_COMMAND, DPORTSELECT, reg_data_port_command);
    *dportcontrol = RU_FIELD_GET(0, EPN, DATA_PORT_COMMAND, DPORTCONTROL, reg_data_port_command);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_data_port_address_set(uint16_t dportaddr)
{
    uint32_t reg_data_port_address=0;

#ifdef VALIDATE_PARMS
    if((dportaddr >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_data_port_address = RU_FIELD_SET(0, EPN, DATA_PORT_ADDRESS, DPORTADDR, reg_data_port_address, dportaddr);

    RU_REG_WRITE(0, EPN, DATA_PORT_ADDRESS, reg_data_port_address);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_data_port_address_get(uint16_t *dportaddr)
{
    uint32_t reg_data_port_address=0;

#ifdef VALIDATE_PARMS
    if(!dportaddr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DATA_PORT_ADDRESS, reg_data_port_address);

    *dportaddr = RU_FIELD_GET(0, EPN, DATA_PORT_ADDRESS, DPORTADDR, reg_data_port_address);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_data_port_data_0_set(uint32_t dportdata0)
{
    uint32_t reg_data_port_data_0=0;

#ifdef VALIDATE_PARMS
#endif

    reg_data_port_data_0 = RU_FIELD_SET(0, EPN, DATA_PORT_DATA_0, DPORTDATA0, reg_data_port_data_0, dportdata0);

    RU_REG_WRITE(0, EPN, DATA_PORT_DATA_0, reg_data_port_data_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_data_port_data_0_get(uint32_t *dportdata0)
{
    uint32_t reg_data_port_data_0=0;

#ifdef VALIDATE_PARMS
    if(!dportdata0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DATA_PORT_DATA_0, reg_data_port_data_0);

    *dportdata0 = RU_FIELD_GET(0, EPN, DATA_PORT_DATA_0, DPORTDATA0, reg_data_port_data_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_big_cnt_set(uint32_t unmapbigerrcnt)
{
    uint32_t reg_unmap_big_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_unmap_big_cnt = RU_FIELD_SET(0, EPN, UNMAP_BIG_CNT, UNMAPBIGERRCNT, reg_unmap_big_cnt, unmapbigerrcnt);

    RU_REG_WRITE(0, EPN, UNMAP_BIG_CNT, reg_unmap_big_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_big_cnt_get(uint32_t *unmapbigerrcnt)
{
    uint32_t reg_unmap_big_cnt=0;

#ifdef VALIDATE_PARMS
    if(!unmapbigerrcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, UNMAP_BIG_CNT, reg_unmap_big_cnt);

    *unmapbigerrcnt = RU_FIELD_GET(0, EPN, UNMAP_BIG_CNT, UNMAPBIGERRCNT, reg_unmap_big_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_frame_cnt_set(uint32_t unmapfrcnt)
{
    uint32_t reg_unmap_frame_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_unmap_frame_cnt = RU_FIELD_SET(0, EPN, UNMAP_FRAME_CNT, UNMAPFRCNT, reg_unmap_frame_cnt, unmapfrcnt);

    RU_REG_WRITE(0, EPN, UNMAP_FRAME_CNT, reg_unmap_frame_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_frame_cnt_get(uint32_t *unmapfrcnt)
{
    uint32_t reg_unmap_frame_cnt=0;

#ifdef VALIDATE_PARMS
    if(!unmapfrcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, UNMAP_FRAME_CNT, reg_unmap_frame_cnt);

    *unmapfrcnt = RU_FIELD_GET(0, EPN, UNMAP_FRAME_CNT, UNMAPFRCNT, reg_unmap_frame_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_fcs_cnt_set(uint32_t unmapfcserrcnt)
{
    uint32_t reg_unmap_fcs_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_unmap_fcs_cnt = RU_FIELD_SET(0, EPN, UNMAP_FCS_CNT, UNMAPFCSERRCNT, reg_unmap_fcs_cnt, unmapfcserrcnt);

    RU_REG_WRITE(0, EPN, UNMAP_FCS_CNT, reg_unmap_fcs_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_fcs_cnt_get(uint32_t *unmapfcserrcnt)
{
    uint32_t reg_unmap_fcs_cnt=0;

#ifdef VALIDATE_PARMS
    if(!unmapfcserrcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, UNMAP_FCS_CNT, reg_unmap_fcs_cnt);

    *unmapfcserrcnt = RU_FIELD_GET(0, EPN, UNMAP_FCS_CNT, UNMAPFCSERRCNT, reg_unmap_fcs_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_gate_cnt_set(uint32_t unmapgatecnt)
{
    uint32_t reg_unmap_gate_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_unmap_gate_cnt = RU_FIELD_SET(0, EPN, UNMAP_GATE_CNT, UNMAPGATECNT, reg_unmap_gate_cnt, unmapgatecnt);

    RU_REG_WRITE(0, EPN, UNMAP_GATE_CNT, reg_unmap_gate_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_gate_cnt_get(uint32_t *unmapgatecnt)
{
    uint32_t reg_unmap_gate_cnt=0;

#ifdef VALIDATE_PARMS
    if(!unmapgatecnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, UNMAP_GATE_CNT, reg_unmap_gate_cnt);

    *unmapgatecnt = RU_FIELD_GET(0, EPN, UNMAP_GATE_CNT, UNMAPGATECNT, reg_unmap_gate_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_oam_cnt_set(uint32_t unmapoamcnt)
{
    uint32_t reg_unmap_oam_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_unmap_oam_cnt = RU_FIELD_SET(0, EPN, UNMAP_OAM_CNT, UNMAPOAMCNT, reg_unmap_oam_cnt, unmapoamcnt);

    RU_REG_WRITE(0, EPN, UNMAP_OAM_CNT, reg_unmap_oam_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_oam_cnt_get(uint32_t *unmapoamcnt)
{
    uint32_t reg_unmap_oam_cnt=0;

#ifdef VALIDATE_PARMS
    if(!unmapoamcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, UNMAP_OAM_CNT, reg_unmap_oam_cnt);

    *unmapoamcnt = RU_FIELD_GET(0, EPN, UNMAP_OAM_CNT, UNMAPOAMCNT, reg_unmap_oam_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_small_cnt_set(uint32_t unmapsmallerrcnt)
{
    uint32_t reg_unmap_small_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_unmap_small_cnt = RU_FIELD_SET(0, EPN, UNMAP_SMALL_CNT, UNMAPSMALLERRCNT, reg_unmap_small_cnt, unmapsmallerrcnt);

    RU_REG_WRITE(0, EPN, UNMAP_SMALL_CNT, reg_unmap_small_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unmap_small_cnt_get(uint32_t *unmapsmallerrcnt)
{
    uint32_t reg_unmap_small_cnt=0;

#ifdef VALIDATE_PARMS
    if(!unmapsmallerrcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, UNMAP_SMALL_CNT, reg_unmap_small_cnt);

    *unmapsmallerrcnt = RU_FIELD_GET(0, EPN, UNMAP_SMALL_CNT, UNMAPSMALLERRCNT, reg_unmap_small_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_fif_dequeue_event_cnt_set(uint32_t fifdequeueeventcnt)
{
    uint32_t reg_fif_dequeue_event_cnt=0;

#ifdef VALIDATE_PARMS
#endif

    reg_fif_dequeue_event_cnt = RU_FIELD_SET(0, EPN, FIF_DEQUEUE_EVENT_CNT, FIFDEQUEUEEVENTCNT, reg_fif_dequeue_event_cnt, fifdequeueeventcnt);

    RU_REG_WRITE(0, EPN, FIF_DEQUEUE_EVENT_CNT, reg_fif_dequeue_event_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_fif_dequeue_event_cnt_get(uint32_t *fifdequeueeventcnt)
{
    uint32_t reg_fif_dequeue_event_cnt=0;

#ifdef VALIDATE_PARMS
    if(!fifdequeueeventcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, FIF_DEQUEUE_EVENT_CNT, reg_fif_dequeue_event_cnt);

    *fifdequeueeventcnt = RU_FIELD_GET(0, EPN, FIF_DEQUEUE_EVENT_CNT, FIFDEQUEUEEVENTCNT, reg_fif_dequeue_event_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unused_tq_cnt_set(uint8_t link_idx, uint32_t unusedtqcnt)
{
    uint32_t reg_unused_tq_cnt=0;

#ifdef VALIDATE_PARMS
    if((link_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unused_tq_cnt = RU_FIELD_SET(0, EPN, UNUSED_TQ_CNT, UNUSEDTQCNT, reg_unused_tq_cnt, unusedtqcnt);

    RU_REG_RAM_WRITE(0, link_idx, EPN, UNUSED_TQ_CNT, reg_unused_tq_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_unused_tq_cnt_get(uint8_t link_idx, uint32_t *unusedtqcnt)
{
    uint32_t reg_unused_tq_cnt=0;

#ifdef VALIDATE_PARMS
    if(!unusedtqcnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((link_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, link_idx, EPN, UNUSED_TQ_CNT, reg_unused_tq_cnt);

    *unusedtqcnt = RU_FIELD_GET(0, EPN, UNUSED_TQ_CNT, UNUSEDTQCNT, reg_unused_tq_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_up_fault_halt_en_set(uint8_t link_idx, bdmf_boolean bbhUpsFaultHaltEn)
{
    uint32_t reg_bbh_up_fault_halt_en=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, EPN, BBH_UP_FAULT_HALT_EN, reg_bbh_up_fault_halt_en);

    FIELD_SET(reg_bbh_up_fault_halt_en, (link_idx % 32) *1, 0x1, bbhUpsFaultHaltEn);

    RU_REG_WRITE(0, EPN, BBH_UP_FAULT_HALT_EN, reg_bbh_up_fault_halt_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_up_fault_halt_en_get(uint8_t link_idx, bdmf_boolean *bbhUpsFaultHaltEn)
{
    uint32_t reg_bbh_up_fault_halt_en=0;

#ifdef VALIDATE_PARMS
    if(!bbhUpsFaultHaltEn)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BBH_UP_FAULT_HALT_EN, reg_bbh_up_fault_halt_en);

    *bbhUpsFaultHaltEn = FIELD_GET(reg_bbh_up_fault_halt_en, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_up_tardy_halt_en_set(bdmf_boolean fataltardybbhaborten)
{
    uint32_t reg_bbh_up_tardy_halt_en=0;

#ifdef VALIDATE_PARMS
    if((fataltardybbhaborten >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_up_tardy_halt_en = RU_FIELD_SET(0, EPN, BBH_UP_TARDY_HALT_EN, FATALTARDYBBHABORTEN, reg_bbh_up_tardy_halt_en, fataltardybbhaborten);

    RU_REG_WRITE(0, EPN, BBH_UP_TARDY_HALT_EN, reg_bbh_up_tardy_halt_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_up_tardy_halt_en_get(bdmf_boolean *fataltardybbhaborten)
{
    uint32_t reg_bbh_up_tardy_halt_en=0;

#ifdef VALIDATE_PARMS
    if(!fataltardybbhaborten)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BBH_UP_TARDY_HALT_EN, reg_bbh_up_tardy_halt_en);

    *fataltardybbhaborten = RU_FIELD_GET(0, EPN, BBH_UP_TARDY_HALT_EN, FATALTARDYBBHABORTEN, reg_bbh_up_tardy_halt_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_debug_status_0_get(uint8_t *l2squefulldebug, bdmf_boolean *dndlufull, bdmf_boolean *dnsecfull, bdmf_boolean *epnliffifofull)
{
    uint32_t reg_debug_status_0=0;

#ifdef VALIDATE_PARMS
    if(!l2squefulldebug || !dndlufull || !dnsecfull || !epnliffifofull)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DEBUG_STATUS_0, reg_debug_status_0);

    *l2squefulldebug = RU_FIELD_GET(0, EPN, DEBUG_STATUS_0, L2SQUEFULLDEBUG, reg_debug_status_0);
    *dndlufull = RU_FIELD_GET(0, EPN, DEBUG_STATUS_0, DNDLUFULL, reg_debug_status_0);
    *dnsecfull = RU_FIELD_GET(0, EPN, DEBUG_STATUS_0, DNSECFULL, reg_debug_status_0);
    *epnliffifofull = RU_FIELD_GET(0, EPN, DEBUG_STATUS_0, EPNLIFFIFOFULL, reg_debug_status_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_debug_status_1_get(uint8_t link_idx, bdmf_boolean *gntRdy)
{
    uint32_t reg_debug_status_1=0;

#ifdef VALIDATE_PARMS
    if(!gntRdy)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DEBUG_STATUS_1, reg_debug_status_1);

    *gntRdy = FIELD_GET(reg_debug_status_1, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_debug_l2s_ptr_sel_set(uint8_t cfgl2sdebugptrsel, uint16_t l2sdebugptrstate)
{
    uint32_t reg_debug_l2s_ptr_sel=0;

#ifdef VALIDATE_PARMS
    if((cfgl2sdebugptrsel >= _3BITS_MAX_VAL_) ||
       (l2sdebugptrstate >= _15BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_l2s_ptr_sel = RU_FIELD_SET(0, EPN, DEBUG_L2S_PTR_SEL, CFGL2SDEBUGPTRSEL, reg_debug_l2s_ptr_sel, cfgl2sdebugptrsel);
    reg_debug_l2s_ptr_sel = RU_FIELD_SET(0, EPN, DEBUG_L2S_PTR_SEL, L2SDEBUGPTRSTATE, reg_debug_l2s_ptr_sel, l2sdebugptrstate);

    RU_REG_WRITE(0, EPN, DEBUG_L2S_PTR_SEL, reg_debug_l2s_ptr_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_debug_l2s_ptr_sel_get(uint8_t *cfgl2sdebugptrsel, uint16_t *l2sdebugptrstate)
{
    uint32_t reg_debug_l2s_ptr_sel=0;

#ifdef VALIDATE_PARMS
    if(!cfgl2sdebugptrsel || !l2sdebugptrstate)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DEBUG_L2S_PTR_SEL, reg_debug_l2s_ptr_sel);

    *cfgl2sdebugptrsel = RU_FIELD_GET(0, EPN, DEBUG_L2S_PTR_SEL, CFGL2SDEBUGPTRSEL, reg_debug_l2s_ptr_sel);
    *l2sdebugptrstate = RU_FIELD_GET(0, EPN, DEBUG_L2S_PTR_SEL, L2SDEBUGPTRSTATE, reg_debug_l2s_ptr_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_olt_mac_addr_lo_set(uint32_t oltaddrlo)
{
    uint32_t reg_olt_mac_addr_lo=0;

#ifdef VALIDATE_PARMS
#endif

    reg_olt_mac_addr_lo = RU_FIELD_SET(0, EPN, OLT_MAC_ADDR_LO, OLTADDRLO, reg_olt_mac_addr_lo, oltaddrlo);

    RU_REG_WRITE(0, EPN, OLT_MAC_ADDR_LO, reg_olt_mac_addr_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_olt_mac_addr_lo_get(uint32_t *oltaddrlo)
{
    uint32_t reg_olt_mac_addr_lo=0;

#ifdef VALIDATE_PARMS
    if(!oltaddrlo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, OLT_MAC_ADDR_LO, reg_olt_mac_addr_lo);

    *oltaddrlo = RU_FIELD_GET(0, EPN, OLT_MAC_ADDR_LO, OLTADDRLO, reg_olt_mac_addr_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_olt_mac_addr_hi_set(uint16_t oltaddrhi)
{
    uint32_t reg_olt_mac_addr_hi=0;

#ifdef VALIDATE_PARMS
#endif

    reg_olt_mac_addr_hi = RU_FIELD_SET(0, EPN, OLT_MAC_ADDR_HI, OLTADDRHI, reg_olt_mac_addr_hi, oltaddrhi);

    RU_REG_WRITE(0, EPN, OLT_MAC_ADDR_HI, reg_olt_mac_addr_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_olt_mac_addr_hi_get(uint16_t *oltaddrhi)
{
    uint32_t reg_olt_mac_addr_hi=0;

#ifdef VALIDATE_PARMS
    if(!oltaddrhi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, OLT_MAC_ADDR_HI, reg_olt_mac_addr_hi);

    *oltaddrhi = RU_FIELD_GET(0, EPN, OLT_MAC_ADDR_HI, OLTADDRHI, reg_olt_mac_addr_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l1s_shp_dqu_empty_get(uint8_t l1_acc_idx, bdmf_boolean *l1sDquQueEmpty)
{
    uint32_t reg_tx_l1s_shp_dqu_empty=0;

#ifdef VALIDATE_PARMS
    if(!l1sDquQueEmpty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, TX_L1S_SHP_DQU_EMPTY, reg_tx_l1s_shp_dqu_empty);

    *l1sDquQueEmpty = FIELD_GET(reg_tx_l1s_shp_dqu_empty, (l1_acc_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l1s_unshaped_empty_get(uint8_t l1_acc_idx, bdmf_boolean *l1sUnshapedQueEmpty)
{
    uint32_t reg_tx_l1s_unshaped_empty=0;

#ifdef VALIDATE_PARMS
    if(!l1sUnshapedQueEmpty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, TX_L1S_UNSHAPED_EMPTY, reg_tx_l1s_unshaped_empty);

    *l1sUnshapedQueEmpty = FIELD_GET(reg_tx_l1s_unshaped_empty, (l1_acc_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l1s_shp_que_mask_set(uint8_t shaper_idx, uint32_t cfgshpmask)
{
    uint32_t reg_tx_l1s_shp_que_mask_=0;

#ifdef VALIDATE_PARMS
    if((shaper_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_l1s_shp_que_mask_ = RU_FIELD_SET(0, EPN, TX_L1S_SHP_QUE_MASK_, CFGSHPMASK, reg_tx_l1s_shp_que_mask_, cfgshpmask);

    RU_REG_RAM_WRITE(0, shaper_idx, EPN, TX_L1S_SHP_QUE_MASK_, reg_tx_l1s_shp_que_mask_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l1s_shp_que_mask_get(uint8_t shaper_idx, uint32_t *cfgshpmask)
{
    uint32_t reg_tx_l1s_shp_que_mask_=0;

#ifdef VALIDATE_PARMS
    if(!cfgshpmask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((shaper_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, shaper_idx, EPN, TX_L1S_SHP_QUE_MASK_, reg_tx_l1s_shp_que_mask_);

    *cfgshpmask = RU_FIELD_GET(0, EPN, TX_L1S_SHP_QUE_MASK_, CFGSHPMASK, reg_tx_l1s_shp_que_mask_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l2s_queue_config_set(uint8_t que_idx, uint16_t cfgl2squeend, uint16_t cfgl2squestart)
{
    uint32_t reg_tx_l2s_que_config_=0;

#ifdef VALIDATE_PARMS
    if((que_idx >= 32) ||
       (cfgl2squeend >= _12BITS_MAX_VAL_) ||
       (cfgl2squestart >= _12BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_l2s_que_config_ = RU_FIELD_SET(0, EPN, TX_L2S_QUE_CONFIG_, CFGL2SQUEEND, reg_tx_l2s_que_config_, cfgl2squeend);
    reg_tx_l2s_que_config_ = RU_FIELD_SET(0, EPN, TX_L2S_QUE_CONFIG_, CFGL2SQUESTART, reg_tx_l2s_que_config_, cfgl2squestart);

    RU_REG_RAM_WRITE(0, que_idx, EPN, TX_L2S_QUE_CONFIG_, reg_tx_l2s_que_config_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l2s_queue_config_get(uint8_t que_idx, uint16_t *cfgl2squeend, uint16_t *cfgl2squestart)
{
    uint32_t reg_tx_l2s_que_config_=0;

#ifdef VALIDATE_PARMS
    if(!cfgl2squeend || !cfgl2squestart)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((que_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, que_idx, EPN, TX_L2S_QUE_CONFIG_, reg_tx_l2s_que_config_);

    *cfgl2squeend = RU_FIELD_GET(0, EPN, TX_L2S_QUE_CONFIG_, CFGL2SQUEEND, reg_tx_l2s_que_config_);
    *cfgl2squestart = RU_FIELD_GET(0, EPN, TX_L2S_QUE_CONFIG_, CFGL2SQUESTART, reg_tx_l2s_que_config_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l2s_que_empty_get(uint8_t l2_queue_idx, bdmf_boolean *l2sQueEmpty)
{
    uint32_t reg_tx_l2s_que_empty=0;

#ifdef VALIDATE_PARMS
    if(!l2sQueEmpty)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, TX_L2S_QUE_EMPTY, reg_tx_l2s_que_empty);

    *l2sQueEmpty = FIELD_GET(reg_tx_l2s_que_empty, (l2_queue_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l2s_que_full_get(uint8_t l2_queue_idx, bdmf_boolean *l2sQueFull)
{
    uint32_t reg_tx_l2s_que_full=0;

#ifdef VALIDATE_PARMS
    if(!l2sQueFull)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, TX_L2S_QUE_FULL, reg_tx_l2s_que_full);

    *l2sQueFull = FIELD_GET(reg_tx_l2s_que_full, (l2_queue_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l2s_que_stopped_get(uint8_t l2_queue_idx, bdmf_boolean *l2sStoppedQueues)
{
    uint32_t reg_tx_l2s_que_stopped=0;

#ifdef VALIDATE_PARMS
    if(!l2sStoppedQueues)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, TX_L2S_QUE_STOPPED, reg_tx_l2s_que_stopped);

    *l2sStoppedQueues = FIELD_GET(reg_tx_l2s_que_stopped, (l2_queue_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_ctc_burst_limit_set(uint8_t l2_queue_idx, uint32_t prvburstlimit)
{
    uint32_t reg_tx_ctc_burst_limit_=0;

#ifdef VALIDATE_PARMS
    if((l2_queue_idx >= 32) ||
       (prvburstlimit >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_ctc_burst_limit_ = RU_FIELD_SET(0, EPN, TX_CTC_BURST_LIMIT_, PRVBURSTLIMIT, reg_tx_ctc_burst_limit_, prvburstlimit);

    RU_REG_RAM_WRITE(0, l2_queue_idx, EPN, TX_CTC_BURST_LIMIT_, reg_tx_ctc_burst_limit_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_ctc_burst_limit_get(uint8_t l2_queue_idx, uint32_t *prvburstlimit)
{
    uint32_t reg_tx_ctc_burst_limit_=0;

#ifdef VALIDATE_PARMS
    if(!prvburstlimit)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((l2_queue_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, l2_queue_idx, EPN, TX_CTC_BURST_LIMIT_, reg_tx_ctc_burst_limit_);

    *prvburstlimit = RU_FIELD_GET(0, EPN, TX_CTC_BURST_LIMIT_, PRVBURSTLIMIT, reg_tx_ctc_burst_limit_);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_max_outstanding_tardy_packets_set(uint16_t cfgmaxoutstandingtardypackets)
{
    uint32_t reg_bbh_max_outstanding_tardy_packets=0;

#ifdef VALIDATE_PARMS
    if((cfgmaxoutstandingtardypackets >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_bbh_max_outstanding_tardy_packets = RU_FIELD_SET(0, EPN, BBH_MAX_OUTSTANDING_TARDY_PACKETS, CFGMAXOUTSTANDINGTARDYPACKETS, reg_bbh_max_outstanding_tardy_packets, cfgmaxoutstandingtardypackets);

    RU_REG_WRITE(0, EPN, BBH_MAX_OUTSTANDING_TARDY_PACKETS, reg_bbh_max_outstanding_tardy_packets);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_max_outstanding_tardy_packets_get(uint16_t *cfgmaxoutstandingtardypackets)
{
    uint32_t reg_bbh_max_outstanding_tardy_packets=0;

#ifdef VALIDATE_PARMS
    if(!cfgmaxoutstandingtardypackets)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BBH_MAX_OUTSTANDING_TARDY_PACKETS, reg_bbh_max_outstanding_tardy_packets);

    *cfgmaxoutstandingtardypackets = RU_FIELD_GET(0, EPN, BBH_MAX_OUTSTANDING_TARDY_PACKETS, CFGMAXOUTSTANDINGTARDYPACKETS, reg_bbh_max_outstanding_tardy_packets);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_min_report_value_difference_set(uint16_t prvminreportdiff)
{
    uint32_t reg_min_report_value_difference=0;

#ifdef VALIDATE_PARMS
    if((prvminreportdiff >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_min_report_value_difference = RU_FIELD_SET(0, EPN, MIN_REPORT_VALUE_DIFFERENCE, PRVMINREPORTDIFF, reg_min_report_value_difference, prvminreportdiff);

    RU_REG_WRITE(0, EPN, MIN_REPORT_VALUE_DIFFERENCE, reg_min_report_value_difference);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_min_report_value_difference_get(uint16_t *prvminreportdiff)
{
    uint32_t reg_min_report_value_difference=0;

#ifdef VALIDATE_PARMS
    if(!prvminreportdiff)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, MIN_REPORT_VALUE_DIFFERENCE, reg_min_report_value_difference);

    *prvminreportdiff = RU_FIELD_GET(0, EPN, MIN_REPORT_VALUE_DIFFERENCE, PRVMINREPORTDIFF, reg_min_report_value_difference);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_bbh_status_fifo_overflow_get(uint8_t bbh_queue_idx, bdmf_boolean *utxBbhStatusFifoOverflow)
{
    uint32_t reg_bbh_status_fifo_overflow=0;

#ifdef VALIDATE_PARMS
    if(!utxBbhStatusFifoOverflow)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, BBH_STATUS_FIFO_OVERFLOW, reg_bbh_status_fifo_overflow);

    *utxBbhStatusFifoOverflow = FIELD_GET(reg_bbh_status_fifo_overflow, (bbh_queue_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_spare_ctl_set(uint32_t cfgepnspare)
{
    uint32_t reg_spare_ctl=0;

#ifdef VALIDATE_PARMS
#endif

    reg_spare_ctl = RU_FIELD_SET(0, EPN, SPARE_CTL, CFGEPNSPARE, reg_spare_ctl, cfgepnspare);

    RU_REG_WRITE(0, EPN, SPARE_CTL, reg_spare_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_spare_ctl_get(uint32_t *cfgepnspare)
{
    uint32_t reg_spare_ctl=0;

#ifdef VALIDATE_PARMS
    if(!cfgepnspare)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, SPARE_CTL, reg_spare_ctl);

    *cfgepnspare = RU_FIELD_GET(0, EPN, SPARE_CTL, CFGEPNSPARE, reg_spare_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_ts_sync_offset_set(uint8_t cfgtssyncoffset)
{
    uint32_t reg_ts_sync_offset=0;

#ifdef VALIDATE_PARMS
    if((cfgtssyncoffset >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ts_sync_offset = RU_FIELD_SET(0, EPN, TS_SYNC_OFFSET, CFGTSSYNCOFFSET, reg_ts_sync_offset, cfgtssyncoffset);

    RU_REG_WRITE(0, EPN, TS_SYNC_OFFSET, reg_ts_sync_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_ts_sync_offset_get(uint8_t *cfgtssyncoffset)
{
    uint32_t reg_ts_sync_offset=0;

#ifdef VALIDATE_PARMS
    if(!cfgtssyncoffset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, TS_SYNC_OFFSET, reg_ts_sync_offset);

    *cfgtssyncoffset = RU_FIELD_GET(0, EPN, TS_SYNC_OFFSET, CFGTSSYNCOFFSET, reg_ts_sync_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_ts_offset_set(uint32_t cfgdntsoffset)
{
    uint32_t reg_dn_ts_offset=0;

#ifdef VALIDATE_PARMS
#endif

    reg_dn_ts_offset = RU_FIELD_SET(0, EPN, DN_TS_OFFSET, CFGDNTSOFFSET, reg_dn_ts_offset, cfgdntsoffset);

    RU_REG_WRITE(0, EPN, DN_TS_OFFSET, reg_dn_ts_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_ts_offset_get(uint32_t *cfgdntsoffset)
{
    uint32_t reg_dn_ts_offset=0;

#ifdef VALIDATE_PARMS
    if(!cfgdntsoffset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DN_TS_OFFSET, reg_dn_ts_offset);

    *cfgdntsoffset = RU_FIELD_GET(0, EPN, DN_TS_OFFSET, CFGDNTSOFFSET, reg_dn_ts_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_up_ts_offset_lo_set(uint32_t cfguptsoffset_lo)
{
    uint32_t reg_up_ts_offset_lo=0;

#ifdef VALIDATE_PARMS
#endif

    reg_up_ts_offset_lo = RU_FIELD_SET(0, EPN, UP_TS_OFFSET_LO, CFGUPTSOFFSET_LO, reg_up_ts_offset_lo, cfguptsoffset_lo);

    RU_REG_WRITE(0, EPN, UP_TS_OFFSET_LO, reg_up_ts_offset_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_up_ts_offset_lo_get(uint32_t *cfguptsoffset_lo)
{
    uint32_t reg_up_ts_offset_lo=0;

#ifdef VALIDATE_PARMS
    if(!cfguptsoffset_lo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, UP_TS_OFFSET_LO, reg_up_ts_offset_lo);

    *cfguptsoffset_lo = RU_FIELD_GET(0, EPN, UP_TS_OFFSET_LO, CFGUPTSOFFSET_LO, reg_up_ts_offset_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_up_ts_offset_hi_set(uint16_t cfguptsoffset_hi)
{
    uint32_t reg_up_ts_offset_hi=0;

#ifdef VALIDATE_PARMS
#endif

    reg_up_ts_offset_hi = RU_FIELD_SET(0, EPN, UP_TS_OFFSET_HI, CFGUPTSOFFSET_HI, reg_up_ts_offset_hi, cfguptsoffset_hi);

    RU_REG_WRITE(0, EPN, UP_TS_OFFSET_HI, reg_up_ts_offset_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_up_ts_offset_hi_get(uint16_t *cfguptsoffset_hi)
{
    uint32_t reg_up_ts_offset_hi=0;

#ifdef VALIDATE_PARMS
    if(!cfguptsoffset_hi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, UP_TS_OFFSET_HI, reg_up_ts_offset_hi);

    *cfguptsoffset_hi = RU_FIELD_GET(0, EPN, UP_TS_OFFSET_HI, CFGUPTSOFFSET_HI, reg_up_ts_offset_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_two_step_ts_ctl_set(bdmf_boolean twostepffrd, uint8_t twostepffentries)
{
    uint32_t reg_two_step_ts_ctl=0;

#ifdef VALIDATE_PARMS
    if((twostepffrd >= _1BITS_MAX_VAL_) ||
       (twostepffentries >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_two_step_ts_ctl = RU_FIELD_SET(0, EPN, TWO_STEP_TS_CTL, TWOSTEPFFRD, reg_two_step_ts_ctl, twostepffrd);
    reg_two_step_ts_ctl = RU_FIELD_SET(0, EPN, TWO_STEP_TS_CTL, TWOSTEPFFENTRIES, reg_two_step_ts_ctl, twostepffentries);

    RU_REG_WRITE(0, EPN, TWO_STEP_TS_CTL, reg_two_step_ts_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_two_step_ts_ctl_get(bdmf_boolean *twostepffrd, uint8_t *twostepffentries)
{
    uint32_t reg_two_step_ts_ctl=0;

#ifdef VALIDATE_PARMS
    if(!twostepffrd || !twostepffentries)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, TWO_STEP_TS_CTL, reg_two_step_ts_ctl);

    *twostepffrd = RU_FIELD_GET(0, EPN, TWO_STEP_TS_CTL, TWOSTEPFFRD, reg_two_step_ts_ctl);
    *twostepffentries = RU_FIELD_GET(0, EPN, TWO_STEP_TS_CTL, TWOSTEPFFENTRIES, reg_two_step_ts_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_two_step_ts_value_lo_get(uint32_t *twosteptimestamp_lo)
{
    uint32_t reg_two_step_ts_value_lo=0;

#ifdef VALIDATE_PARMS
    if(!twosteptimestamp_lo)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, TWO_STEP_TS_VALUE_LO, reg_two_step_ts_value_lo);

    *twosteptimestamp_lo = RU_FIELD_GET(0, EPN, TWO_STEP_TS_VALUE_LO, TWOSTEPTIMESTAMP_LO, reg_two_step_ts_value_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_two_step_ts_value_hi_get(uint16_t *twosteptimestamp_hi)
{
    uint32_t reg_two_step_ts_value_hi=0;

#ifdef VALIDATE_PARMS
    if(!twosteptimestamp_hi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, TWO_STEP_TS_VALUE_HI, reg_two_step_ts_value_hi);

    *twosteptimestamp_hi = RU_FIELD_GET(0, EPN, TWO_STEP_TS_VALUE_HI, TWOSTEPTIMESTAMP_HI, reg_two_step_ts_value_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_1588_timestamp_int_status_set(bdmf_boolean int1588pktabort, bdmf_boolean int1588twostepffint)
{
    uint32_t reg_1588_timestamp_int_status=0;

#ifdef VALIDATE_PARMS
    if((int1588pktabort >= _1BITS_MAX_VAL_) ||
       (int1588twostepffint >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_1588_timestamp_int_status = RU_FIELD_SET(0, EPN, 1588_TIMESTAMP_INT_STATUS, INT1588PKTABORT, reg_1588_timestamp_int_status, int1588pktabort);
    reg_1588_timestamp_int_status = RU_FIELD_SET(0, EPN, 1588_TIMESTAMP_INT_STATUS, INT1588TWOSTEPFFINT, reg_1588_timestamp_int_status, int1588twostepffint);

    RU_REG_WRITE(0, EPN, 1588_TIMESTAMP_INT_STATUS, reg_1588_timestamp_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_1588_timestamp_int_status_get(bdmf_boolean *int1588pktabort, bdmf_boolean *int1588twostepffint)
{
    uint32_t reg_1588_timestamp_int_status=0;

#ifdef VALIDATE_PARMS
    if(!int1588pktabort || !int1588twostepffint)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, 1588_TIMESTAMP_INT_STATUS, reg_1588_timestamp_int_status);

    *int1588pktabort = RU_FIELD_GET(0, EPN, 1588_TIMESTAMP_INT_STATUS, INT1588PKTABORT, reg_1588_timestamp_int_status);
    *int1588twostepffint = RU_FIELD_GET(0, EPN, 1588_TIMESTAMP_INT_STATUS, INT1588TWOSTEPFFINT, reg_1588_timestamp_int_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_1588_timestamp_int_mask_set(bdmf_boolean ts1588pktabort_mask, bdmf_boolean ts1588twostepff_mask)
{
    uint32_t reg_1588_timestamp_int_mask=0;

#ifdef VALIDATE_PARMS
    if((ts1588pktabort_mask >= _1BITS_MAX_VAL_) ||
       (ts1588twostepff_mask >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_1588_timestamp_int_mask = RU_FIELD_SET(0, EPN, 1588_TIMESTAMP_INT_MASK, TS1588PKTABORT_MASK, reg_1588_timestamp_int_mask, ts1588pktabort_mask);
    reg_1588_timestamp_int_mask = RU_FIELD_SET(0, EPN, 1588_TIMESTAMP_INT_MASK, TS1588TWOSTEPFF_MASK, reg_1588_timestamp_int_mask, ts1588twostepff_mask);

    RU_REG_WRITE(0, EPN, 1588_TIMESTAMP_INT_MASK, reg_1588_timestamp_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_1588_timestamp_int_mask_get(bdmf_boolean *ts1588pktabort_mask, bdmf_boolean *ts1588twostepff_mask)
{
    uint32_t reg_1588_timestamp_int_mask=0;

#ifdef VALIDATE_PARMS
    if(!ts1588pktabort_mask || !ts1588twostepff_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, 1588_TIMESTAMP_INT_MASK, reg_1588_timestamp_int_mask);

    *ts1588pktabort_mask = RU_FIELD_GET(0, EPN, 1588_TIMESTAMP_INT_MASK, TS1588PKTABORT_MASK, reg_1588_timestamp_int_mask);
    *ts1588twostepff_mask = RU_FIELD_GET(0, EPN, 1588_TIMESTAMP_INT_MASK, TS1588TWOSTEPFF_MASK, reg_1588_timestamp_int_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_up_packet_fetch_margin_set(uint16_t uppacketfetchmargin)
{
    uint32_t reg_up_packet_fetch_margin=0;

#ifdef VALIDATE_PARMS
#endif

    reg_up_packet_fetch_margin = RU_FIELD_SET(0, EPN, UP_PACKET_FETCH_MARGIN, UPPACKETFETCHMARGIN, reg_up_packet_fetch_margin, uppacketfetchmargin);

    RU_REG_WRITE(0, EPN, UP_PACKET_FETCH_MARGIN, reg_up_packet_fetch_margin);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_up_packet_fetch_margin_get(uint16_t *uppacketfetchmargin)
{
    uint32_t reg_up_packet_fetch_margin=0;

#ifdef VALIDATE_PARMS
    if(!uppacketfetchmargin)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, UP_PACKET_FETCH_MARGIN, reg_up_packet_fetch_margin);

    *uppacketfetchmargin = RU_FIELD_GET(0, EPN, UP_PACKET_FETCH_MARGIN, UPPACKETFETCHMARGIN, reg_up_packet_fetch_margin);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_dn_1588_timestamp_get(uint32_t *dn_1588_ts)
{
    uint32_t reg_dn_1588_timestamp=0;

#ifdef VALIDATE_PARMS
    if(!dn_1588_ts)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, DN_1588_TIMESTAMP, reg_dn_1588_timestamp);

    *dn_1588_ts = RU_FIELD_GET(0, EPN, DN_1588_TIMESTAMP, DN_1588_TS, reg_dn_1588_timestamp);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_persistent_report_cfg_set(uint16_t cfgpersrptduration, uint16_t cfgpersrptticksize)
{
    uint32_t reg_persistent_report_cfg=0;

#ifdef VALIDATE_PARMS
    if((cfgpersrptduration >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_persistent_report_cfg = RU_FIELD_SET(0, EPN, PERSISTENT_REPORT_CFG, CFGPERSRPTDURATION, reg_persistent_report_cfg, cfgpersrptduration);
    reg_persistent_report_cfg = RU_FIELD_SET(0, EPN, PERSISTENT_REPORT_CFG, CFGPERSRPTTICKSIZE, reg_persistent_report_cfg, cfgpersrptticksize);

    RU_REG_WRITE(0, EPN, PERSISTENT_REPORT_CFG, reg_persistent_report_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_persistent_report_cfg_get(uint16_t *cfgpersrptduration, uint16_t *cfgpersrptticksize)
{
    uint32_t reg_persistent_report_cfg=0;

#ifdef VALIDATE_PARMS
    if(!cfgpersrptduration || !cfgpersrptticksize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, PERSISTENT_REPORT_CFG, reg_persistent_report_cfg);

    *cfgpersrptduration = RU_FIELD_GET(0, EPN, PERSISTENT_REPORT_CFG, CFGPERSRPTDURATION, reg_persistent_report_cfg);
    *cfgpersrptticksize = RU_FIELD_GET(0, EPN, PERSISTENT_REPORT_CFG, CFGPERSRPTTICKSIZE, reg_persistent_report_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_persistent_report_enables_set(uint8_t link_idx, bdmf_boolean cfgPersRptEnable)
{
    uint32_t reg_persistent_report_enables=0;

#ifdef VALIDATE_PARMS
#endif

    RU_REG_READ(0, EPN, PERSISTENT_REPORT_ENABLES, reg_persistent_report_enables);

    FIELD_SET(reg_persistent_report_enables, (link_idx % 32) *1, 0x1, cfgPersRptEnable);

    RU_REG_WRITE(0, EPN, PERSISTENT_REPORT_ENABLES, reg_persistent_report_enables);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_persistent_report_enables_get(uint8_t link_idx, bdmf_boolean *cfgPersRptEnable)
{
    uint32_t reg_persistent_report_enables=0;

#ifdef VALIDATE_PARMS
    if(!cfgPersRptEnable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, PERSISTENT_REPORT_ENABLES, reg_persistent_report_enables);

    *cfgPersRptEnable = FIELD_GET(reg_persistent_report_enables, (link_idx % 32) *1, 0x1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_persistent_report_request_size_set(uint16_t cfgpersrptreqtq)
{
    uint32_t reg_persistent_report_request_size=0;

#ifdef VALIDATE_PARMS
#endif

    reg_persistent_report_request_size = RU_FIELD_SET(0, EPN, PERSISTENT_REPORT_REQUEST_SIZE, CFGPERSRPTREQTQ, reg_persistent_report_request_size, cfgpersrptreqtq);

    RU_REG_WRITE(0, EPN, PERSISTENT_REPORT_REQUEST_SIZE, reg_persistent_report_request_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_persistent_report_request_size_get(uint16_t *cfgpersrptreqtq)
{
    uint32_t reg_persistent_report_request_size=0;

#ifdef VALIDATE_PARMS
    if(!cfgpersrptreqtq)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EPN, PERSISTENT_REPORT_REQUEST_SIZE, reg_persistent_report_request_size);

    *cfgpersrptreqtq = RU_FIELD_GET(0, EPN, PERSISTENT_REPORT_REQUEST_SIZE, CFGPERSRPTREQTQ, reg_persistent_report_request_size);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_control_0,
    BDMF_control_1,
    BDMF_enable_grants,
    BDMF_drop_disc_gates,
    BDMF_dis_fcs_chk,
    BDMF_pass_gates,
    BDMF_cfg_misalgn_fb,
    BDMF_discovery_filter,
    BDMF_bbh_que_status_delay,
    BDMF_minimum_grant_setup,
    BDMF_reset_gnt_fifo,
    BDMF_reset_l1_accumulator,
    BDMF_l1_accumulator_sel,
    BDMF_l1_sva_bytes,
    BDMF_l1_uva_bytes,
    BDMF_l1_sva_overflow,
    BDMF_l1_uva_overflow,
    BDMF_reset_rpt_pri,
    BDMF_reset_l2_rpt_fifo,
    BDMF_enable_upstream,
    BDMF_enable_upstream_fb,
    BDMF_enable_upstream_fec,
    BDMF_report_byte_length,
    BDMF_main_int_status,
    BDMF_gnt_full_int_status,
    BDMF_gnt_full_int_mask,
    BDMF_gnt_miss_int_status,
    BDMF_gnt_miss_int_mask,
    BDMF_disc_rx_int_status,
    BDMF_disc_rx_int_mask,
    BDMF_gnt_intv_int_status,
    BDMF_gnt_intv_int_mask,
    BDMF_gnt_far_int_status,
    BDMF_gnt_far_int_mask,
    BDMF_gnt_misalgn_int_status,
    BDMF_gnt_misalgn_int_mask,
    BDMF_np_gnt_int_status,
    BDMF_np_gnt_int_mask,
    BDMF_del_stale_int_status,
    BDMF_del_stale_int_mask,
    BDMF_gnt_pres_int_status,
    BDMF_gnt_pres_int_mask,
    BDMF_rpt_pres_int_status,
    BDMF_rpt_pres_int_mask,
    BDMF_drx_abort_int_status,
    BDMF_drx_abort_int_mask,
    BDMF_empty_rpt_int_status,
    BDMF_empty_rpt_int_mask,
    BDMF_bcap_overflow_int_status,
    BDMF_bcap_overflow_int_mask,
    BDMF_bbh_dns_fault_int_status,
    BDMF_bbh_dns_fault_int_mask,
    BDMF_bbh_ups_fault_int_status,
    BDMF_bbh_ups_fault_int_mask,
    BDMF_bbh_ups_abort_int_status,
    BDMF_bbh_ups_abort_int_mask,
    BDMF_main_int_mask,
    BDMF_max_gnt_size,
    BDMF_max_frame_size,
    BDMF_grant_ovr_hd,
    BDMF_poll_size,
    BDMF_dn_rd_gnt_margin,
    BDMF_gnt_time_start_delta,
    BDMF_time_stamp_diff,
    BDMF_up_time_stamp_off,
    BDMF_gnt_interval,
    BDMF_dn_gnt_misalign_thr,
    BDMF_dn_gnt_misalign_pause,
    BDMF_non_poll_intv,
    BDMF_force_fcs_err,
    BDMF_grant_overlap_limit,
    BDMF_aes_configuration_0,
    BDMF_aes_configuration_1,
    BDMF_disc_grant_ovr_hd,
    BDMF_dn_discovery_seed,
    BDMF_dn_discovery_inc,
    BDMF_dn_discovery_size,
    BDMF_prog_rpt_vec,
    BDMF_fec_ipg_length,
    BDMF_fake_report_value_en,
    BDMF_fake_report_value,
    BDMF_burst_cap,
    BDMF_queue_llid_map,
    BDMF_valid_opcode_map,
    BDMF_up_packet_tx_margin,
    BDMF_multi_pri_cfg_0,
    BDMF_shared_bcap_ovrflow,
    BDMF_forced_report_en,
    BDMF_forced_report_max_interval,
    BDMF_l2s_flush_config,
    BDMF_data_port_command,
    BDMF_data_port_address,
    BDMF_data_port_data_0,
    BDMF_unmap_big_cnt,
    BDMF_unmap_frame_cnt,
    BDMF_unmap_fcs_cnt,
    BDMF_unmap_gate_cnt,
    BDMF_unmap_oam_cnt,
    BDMF_unmap_small_cnt,
    BDMF_fif_dequeue_event_cnt,
    BDMF_unused_tq_cnt,
    BDMF_bbh_up_fault_halt_en,
    BDMF_bbh_up_tardy_halt_en,
    BDMF_debug_status_0,
    BDMF_debug_status_1,
    BDMF_debug_l2s_ptr_sel,
    BDMF_olt_mac_addr_lo,
    BDMF_olt_mac_addr_hi,
    BDMF_tx_l1s_shp_dqu_empty,
    BDMF_tx_l1s_unshaped_empty,
    BDMF_tx_l1s_shp_que_mask,
    BDMF_tx_l2s_queue_config,
    BDMF_tx_l2s_que_empty,
    BDMF_tx_l2s_que_full,
    BDMF_tx_l2s_que_stopped,
    BDMF_tx_ctc_burst_limit,
    BDMF_bbh_max_outstanding_tardy_packets,
    BDMF_min_report_value_difference,
    BDMF_bbh_status_fifo_overflow,
    BDMF_spare_ctl,
    BDMF_ts_sync_offset,
    BDMF_dn_ts_offset,
    BDMF_up_ts_offset_lo,
    BDMF_up_ts_offset_hi,
    BDMF_two_step_ts_ctl,
    BDMF_two_step_ts_value_lo,
    BDMF_two_step_ts_value_hi,
    BDMF_1588_timestamp_int_status,
    BDMF_1588_timestamp_int_mask,
    BDMF_up_packet_fetch_margin,
    BDMF_dn_1588_timestamp,
    BDMF_persistent_report_cfg,
    BDMF_persistent_report_enables,
    BDMF_persistent_report_request_size,
};

typedef enum
{
    bdmf_address_control_0,
    bdmf_address_control_1,
    bdmf_address_enable_grants,
    bdmf_address_drop_disc_gates,
    bdmf_address_dis_fcs_chk,
    bdmf_address_pass_gates,
    bdmf_address_cfg_misalgn_fb,
    bdmf_address_discovery_filter,
    bdmf_address_bbh_que_status_delay,
    bdmf_address_minimum_grant_setup,
    bdmf_address_reset_gnt_fifo,
    bdmf_address_reset_l1_accumulator,
    bdmf_address_l1_accumulator_sel,
    bdmf_address_l1_sva_bytes,
    bdmf_address_l1_uva_bytes,
    bdmf_address_l1_sva_overflow,
    bdmf_address_l1_uva_overflow,
    bdmf_address_reset_rpt_pri,
    bdmf_address_reset_l2_rpt_fifo,
    bdmf_address_enable_upstream,
    bdmf_address_enable_upstream_fb,
    bdmf_address_enable_upstream_fec,
    bdmf_address_report_byte_length,
    bdmf_address_main_int_status,
    bdmf_address_gnt_full_int_status,
    bdmf_address_gnt_full_int_mask,
    bdmf_address_gnt_miss_int_status,
    bdmf_address_gnt_miss_int_mask,
    bdmf_address_disc_rx_int_status,
    bdmf_address_disc_rx_int_mask,
    bdmf_address_gnt_intv_int_status,
    bdmf_address_gnt_intv_int_mask,
    bdmf_address_gnt_far_int_status,
    bdmf_address_gnt_far_int_mask,
    bdmf_address_gnt_misalgn_int_status,
    bdmf_address_gnt_misalgn_int_mask,
    bdmf_address_np_gnt_int_status,
    bdmf_address_np_gnt_int_mask,
    bdmf_address_del_stale_int_status,
    bdmf_address_del_stale_int_mask,
    bdmf_address_gnt_pres_int_status,
    bdmf_address_gnt_pres_int_mask,
    bdmf_address_rpt_pres_int_status,
    bdmf_address_rpt_pres_int_mask,
    bdmf_address_drx_abort_int_status,
    bdmf_address_drx_abort_int_mask,
    bdmf_address_empty_rpt_int_status,
    bdmf_address_empty_rpt_int_mask,
    bdmf_address_bcap_overflow_int_status,
    bdmf_address_bcap_overflow_int_mask,
    bdmf_address_bbh_dns_fault_int_status,
    bdmf_address_bbh_dns_fault_int_mask,
    bdmf_address_bbh_ups_fault_int_status,
    bdmf_address_bbh_ups_fault_int_mask,
    bdmf_address_bbh_ups_abort_int_status,
    bdmf_address_bbh_ups_abort_int_mask,
    bdmf_address_main_int_mask,
    bdmf_address_max_gnt_size,
    bdmf_address_max_frame_size,
    bdmf_address_grant_ovr_hd,
    bdmf_address_poll_size,
    bdmf_address_dn_rd_gnt_margin,
    bdmf_address_gnt_time_start_delta,
    bdmf_address_time_stamp_diff,
    bdmf_address_up_time_stamp_off,
    bdmf_address_gnt_interval,
    bdmf_address_dn_gnt_misalign_thr,
    bdmf_address_dn_gnt_misalign_pause,
    bdmf_address_non_poll_intv,
    bdmf_address_force_fcs_err,
    bdmf_address_grant_overlap_limit,
    bdmf_address_aes_configuration_0,
    bdmf_address_aes_configuration_1,
    bdmf_address_disc_grant_ovr_hd,
    bdmf_address_dn_discovery_seed,
    bdmf_address_dn_discovery_inc,
    bdmf_address_dn_discovery_size,
    bdmf_address_prog_rpt_vec,
    bdmf_address_fec_ipg_length,
    bdmf_address_fake_report_value_en,
    bdmf_address_fake_report_value,
    bdmf_address_burst_cap_,
    bdmf_address_queue_llid_map_,
    bdmf_address_valid_opcode_map,
    bdmf_address_up_packet_tx_margin,
    bdmf_address_multi_pri_cfg_0,
    bdmf_address_shared_bcap_ovrflow,
    bdmf_address_forced_report_en,
    bdmf_address_forced_report_max_interval,
    bdmf_address_l2s_flush_config,
    bdmf_address_data_port_command,
    bdmf_address_data_port_address,
    bdmf_address_data_port_data_0,
    bdmf_address_unmap_big_cnt,
    bdmf_address_unmap_frame_cnt,
    bdmf_address_unmap_fcs_cnt,
    bdmf_address_unmap_gate_cnt,
    bdmf_address_unmap_oam_cnt,
    bdmf_address_unmap_small_cnt,
    bdmf_address_fif_dequeue_event_cnt,
    bdmf_address_unused_tq_cnt,
    bdmf_address_bbh_up_fault_halt_en,
    bdmf_address_bbh_up_tardy_halt_en,
    bdmf_address_debug_status_0,
    bdmf_address_debug_status_1,
    bdmf_address_debug_l2s_ptr_sel,
    bdmf_address_olt_mac_addr_lo,
    bdmf_address_olt_mac_addr_hi,
    bdmf_address_tx_l1s_shp_dqu_empty,
    bdmf_address_tx_l1s_unshaped_empty,
    bdmf_address_tx_l1s_shp_que_mask_,
    bdmf_address_tx_l2s_que_config_,
    bdmf_address_tx_l2s_que_empty,
    bdmf_address_tx_l2s_que_full,
    bdmf_address_tx_l2s_que_stopped,
    bdmf_address_tx_ctc_burst_limit_,
    bdmf_address_bbh_max_outstanding_tardy_packets,
    bdmf_address_min_report_value_difference,
    bdmf_address_bbh_status_fifo_overflow,
    bdmf_address_spare_ctl,
    bdmf_address_ts_sync_offset,
    bdmf_address_dn_ts_offset,
    bdmf_address_up_ts_offset_lo,
    bdmf_address_up_ts_offset_hi,
    bdmf_address_two_step_ts_ctl,
    bdmf_address_two_step_ts_value_lo,
    bdmf_address_two_step_ts_value_hi,
    bdmf_address_1588_timestamp_int_status,
    bdmf_address_1588_timestamp_int_mask,
    bdmf_address_up_packet_fetch_margin,
    bdmf_address_dn_1588_timestamp,
    bdmf_address_persistent_report_cfg,
    bdmf_address_persistent_report_enables,
    bdmf_address_persistent_report_request_size,
}
bdmf_address;

static int bcm_epn_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_control_0:
    {
        epn_control_0 control_0 = { .cfgen1588ts=parm[1].value.unumber, .cfgreplaceupfcs=parm[2].value.unumber, .cfgappendupfcs=parm[3].value.unumber, .cfgdropscb=parm[4].value.unumber, .moduncappedreportlimit=parm[5].value.unumber, .modmpquesetfirst=parm[6].value.unumber, .prvlocalmpcppropagation=parm[7].value.unumber, .prvtekmodeprefetch=parm[8].value.unumber, .prvincnonzeroaccum=parm[9].value.unumber, .prvnounmapppedfcs=parm[10].value.unumber, .prvsupressdiscen=parm[11].value.unumber, .cfgvlanmax=parm[12].value.unumber, .fcserronlydatafr=parm[13].value.unumber, .prvdropunmapppedllid=parm[14].value.unumber, .prvsuppressllidmodebit=parm[15].value.unumber, .moddiscoverydafilteren=parm[16].value.unumber, .rptselect=parm[17].value.unumber, .prvdisablesvaquestatusbp=parm[18].value.unumber, .utxloopback=parm[19].value.unumber, .utxen=parm[20].value.unumber, .utxrst_pre_n=parm[21].value.unumber, .cfgdisabledns=parm[22].value.unumber, .drxloopback=parm[23].value.unumber, .drxen=parm[24].value.unumber, .drxrst_pre_n=parm[25].value.unumber};
        err = ag_drv_epn_control_0_set(&control_0);
        break;
    }
    case BDMF_control_1:
    {
        epn_control_1 control_1 = { .prvoverlappedgntenable=parm[1].value.unumber, .rstmisalignthr=parm[2].value.unumber, .cfgtekrpt=parm[3].value.unumber, .cfgstalegntchk=parm[4].value.unumber, .fecrpten=parm[5].value.unumber, .cfgl1l2truestrict=parm[6].value.unumber, .cfgctcrpt=parm[7].value.unumber, .cfgtscorrdis=parm[8].value.unumber, .cfgnodiscrpt=parm[9].value.unumber, .disablediscscale=parm[10].value.unumber, .clronrd=parm[11].value.unumber};
        err = ag_drv_epn_control_1_set(&control_1);
        break;
    }
    case BDMF_enable_grants:
        err = ag_drv_epn_enable_grants_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_drop_disc_gates:
        err = ag_drv_epn_drop_disc_gates_set(parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_dis_fcs_chk:
        err = ag_drv_epn_dis_fcs_chk_set(parm[3].value.unumber, parm[4].value.unumber);
        break;
    case BDMF_pass_gates:
        err = ag_drv_epn_pass_gates_set(parm[4].value.unumber, parm[5].value.unumber);
        break;
    case BDMF_cfg_misalgn_fb:
        err = ag_drv_epn_cfg_misalgn_fb_set(parm[5].value.unumber, parm[6].value.unumber);
        break;
    case BDMF_discovery_filter:
        err = ag_drv_epn_discovery_filter_set(parm[6].value.unumber, parm[7].value.unumber);
        break;
    case BDMF_bbh_que_status_delay:
        err = ag_drv_epn_bbh_que_status_delay_set(parm[6].value.unumber);
        break;
    case BDMF_minimum_grant_setup:
        err = ag_drv_epn_minimum_grant_setup_set(parm[6].value.unumber);
        break;
    case BDMF_reset_gnt_fifo:
        err = ag_drv_epn_reset_gnt_fifo_set(parm[6].value.unumber, parm[7].value.unumber);
        break;
    case BDMF_reset_l1_accumulator:
        err = ag_drv_epn_reset_l1_accumulator_set(parm[7].value.unumber);
        break;
    case BDMF_l1_accumulator_sel:
        err = ag_drv_epn_l1_accumulator_sel_set(parm[7].value.unumber, parm[8].value.unumber);
        break;
    case BDMF_reset_rpt_pri:
    {
        epn_reset_rpt_pri reset_rpt_pri = { .nullrptpri15=parm[7].value.unumber, .nullrptpri14=parm[8].value.unumber, .nullrptpri13=parm[9].value.unumber, .nullrptpri12=parm[10].value.unumber, .nullrptpri11=parm[11].value.unumber, .nullrptpri10=parm[12].value.unumber, .nullrptpri9=parm[13].value.unumber, .nullrptpri8=parm[14].value.unumber, .nullrptpri7=parm[15].value.unumber, .nullrptpri6=parm[16].value.unumber, .nullrptpri5=parm[17].value.unumber, .nullrptpri4=parm[18].value.unumber, .nullrptpri3=parm[19].value.unumber, .nullrptpri2=parm[20].value.unumber, .nullrptpri1=parm[21].value.unumber, .nullrptpri0=parm[22].value.unumber};
        err = ag_drv_epn_reset_rpt_pri_set(&reset_rpt_pri);
        break;
    }
    case BDMF_reset_l2_rpt_fifo:
        err = ag_drv_epn_reset_l2_rpt_fifo_set(parm[7].value.unumber, parm[8].value.unumber);
        break;
    case BDMF_enable_upstream:
        err = ag_drv_epn_enable_upstream_set(parm[8].value.unumber, parm[9].value.unumber);
        break;
    case BDMF_enable_upstream_fec:
        err = ag_drv_epn_enable_upstream_fec_set(parm[9].value.unumber, parm[10].value.unumber);
        break;
    case BDMF_report_byte_length:
        err = ag_drv_epn_report_byte_length_set(parm[10].value.unumber);
        break;
    case BDMF_main_int_status:
    {
        epn_main_int_status main_int_status = { .intbbhupfrabort=parm[10].value.unumber, .intcol2sburstcapoverflowpres=parm[11].value.unumber, .intcoemptyrpt=parm[12].value.unumber, .intcodrxerrabortpres=parm[13].value.unumber, .intl2sfifooverrun=parm[14].value.unumber, .intco1588tsint=parm[15].value.unumber, .intcorptpres=parm[16].value.unumber, .intcogntpres=parm[17].value.unumber, .intcodelstalegnt=parm[18].value.unumber, .intcogntnonpoll=parm[19].value.unumber, .intcogntmisalign=parm[20].value.unumber, .intcognttoofar=parm[21].value.unumber, .intcogntinterval=parm[22].value.unumber, .intcogntdiscovery=parm[23].value.unumber, .intcogntmissabort=parm[24].value.unumber, .intcogntfullabort=parm[25].value.unumber, .intbadupfrlen=parm[26].value.unumber, .intuptardypacket=parm[27].value.unumber, .intuprptfrxmt=parm[28].value.unumber, .intbififooverrun=parm[29].value.unumber, .intburstgnttoobig=parm[30].value.unumber, .intwrgnttoobig=parm[31].value.unumber, .intrcvgnttoobig=parm[32].value.unumber, .intdnstatsoverrun=parm[33].value.unumber, .intupstatsoverrun=parm[34].value.unumber, .intdnoutoforder=parm[35].value.unumber, .inttruantbbhhalt=parm[36].value.unumber, .intupinvldgntlen=parm[37].value.unumber, .intcobbhupsfault=parm[38].value.unumber, .intdntimeinsync=parm[39].value.unumber, .intdntimenotinsync=parm[40].value.unumber, .intdportrdy=parm[41].value.unumber};
        err = ag_drv_epn_main_int_status_set(&main_int_status);
        break;
    }
    case BDMF_gnt_full_int_status:
        err = ag_drv_epn_gnt_full_int_status_set(parm[10].value.unumber, parm[11].value.unumber);
        break;
    case BDMF_gnt_full_int_mask:
        err = ag_drv_epn_gnt_full_int_mask_set(parm[11].value.unumber, parm[12].value.unumber);
        break;
    case BDMF_gnt_miss_int_status:
        err = ag_drv_epn_gnt_miss_int_status_set(parm[12].value.unumber, parm[13].value.unumber);
        break;
    case BDMF_gnt_miss_int_mask:
        err = ag_drv_epn_gnt_miss_int_mask_set(parm[13].value.unumber, parm[14].value.unumber);
        break;
    case BDMF_disc_rx_int_status:
        err = ag_drv_epn_disc_rx_int_status_set(parm[14].value.unumber, parm[15].value.unumber);
        break;
    case BDMF_disc_rx_int_mask:
        err = ag_drv_epn_disc_rx_int_mask_set(parm[15].value.unumber, parm[16].value.unumber);
        break;
    case BDMF_gnt_intv_int_status:
        err = ag_drv_epn_gnt_intv_int_status_set(parm[16].value.unumber, parm[17].value.unumber);
        break;
    case BDMF_gnt_intv_int_mask:
        err = ag_drv_epn_gnt_intv_int_mask_set(parm[17].value.unumber, parm[18].value.unumber);
        break;
    case BDMF_gnt_far_int_status:
        err = ag_drv_epn_gnt_far_int_status_set(parm[18].value.unumber, parm[19].value.unumber);
        break;
    case BDMF_gnt_far_int_mask:
        err = ag_drv_epn_gnt_far_int_mask_set(parm[19].value.unumber, parm[20].value.unumber);
        break;
    case BDMF_gnt_misalgn_int_status:
        err = ag_drv_epn_gnt_misalgn_int_status_set(parm[20].value.unumber, parm[21].value.unumber);
        break;
    case BDMF_gnt_misalgn_int_mask:
        err = ag_drv_epn_gnt_misalgn_int_mask_set(parm[21].value.unumber, parm[22].value.unumber);
        break;
    case BDMF_np_gnt_int_status:
        err = ag_drv_epn_np_gnt_int_status_set(parm[22].value.unumber, parm[23].value.unumber);
        break;
    case BDMF_np_gnt_int_mask:
        err = ag_drv_epn_np_gnt_int_mask_set(parm[23].value.unumber, parm[24].value.unumber);
        break;
    case BDMF_del_stale_int_status:
        err = ag_drv_epn_del_stale_int_status_set(parm[24].value.unumber, parm[25].value.unumber);
        break;
    case BDMF_del_stale_int_mask:
        err = ag_drv_epn_del_stale_int_mask_set(parm[25].value.unumber, parm[26].value.unumber);
        break;
    case BDMF_gnt_pres_int_status:
        err = ag_drv_epn_gnt_pres_int_status_set(parm[26].value.unumber, parm[27].value.unumber);
        break;
    case BDMF_gnt_pres_int_mask:
        err = ag_drv_epn_gnt_pres_int_mask_set(parm[27].value.unumber, parm[28].value.unumber);
        break;
    case BDMF_rpt_pres_int_status:
        err = ag_drv_epn_rpt_pres_int_status_set(parm[28].value.unumber, parm[29].value.unumber);
        break;
    case BDMF_rpt_pres_int_mask:
        err = ag_drv_epn_rpt_pres_int_mask_set(parm[29].value.unumber, parm[30].value.unumber);
        break;
    case BDMF_drx_abort_int_status:
        err = ag_drv_epn_drx_abort_int_status_set(parm[30].value.unumber, parm[31].value.unumber);
        break;
    case BDMF_drx_abort_int_mask:
        err = ag_drv_epn_drx_abort_int_mask_set(parm[31].value.unumber, parm[32].value.unumber);
        break;
    case BDMF_empty_rpt_int_status:
        err = ag_drv_epn_empty_rpt_int_status_set(parm[32].value.unumber, parm[33].value.unumber);
        break;
    case BDMF_empty_rpt_int_mask:
        err = ag_drv_epn_empty_rpt_int_mask_set(parm[33].value.unumber, parm[34].value.unumber);
        break;
    case BDMF_bcap_overflow_int_status:
        err = ag_drv_epn_bcap_overflow_int_status_set(parm[34].value.unumber, parm[35].value.unumber);
        break;
    case BDMF_bcap_overflow_int_mask:
        err = ag_drv_epn_bcap_overflow_int_mask_set(parm[35].value.unumber, parm[36].value.unumber);
        break;
    case BDMF_bbh_dns_fault_int_status:
        err = ag_drv_epn_bbh_dns_fault_int_status_set(parm[36].value.unumber);
        break;
    case BDMF_bbh_dns_fault_int_mask:
        err = ag_drv_epn_bbh_dns_fault_int_mask_set(parm[36].value.unumber);
        break;
    case BDMF_bbh_ups_fault_int_status:
        err = ag_drv_epn_bbh_ups_fault_int_status_set(parm[36].value.unumber, parm[37].value.unumber);
        break;
    case BDMF_bbh_ups_fault_int_mask:
        err = ag_drv_epn_bbh_ups_fault_int_mask_set(parm[37].value.unumber, parm[38].value.unumber);
        break;
    case BDMF_bbh_ups_abort_int_status:
        err = ag_drv_epn_bbh_ups_abort_int_status_set(parm[38].value.unumber);
        break;
    case BDMF_bbh_ups_abort_int_mask:
        err = ag_drv_epn_bbh_ups_abort_int_mask_set(parm[38].value.unumber);
        break;
    case BDMF_main_int_mask:
    {
        epn_main_int_mask main_int_mask = { .bbhupfrabortmask=parm[38].value.unumber, .intl2sburstcapoverflowmask=parm[39].value.unumber, .intcoemptyrptmask=parm[40].value.unumber, .intdrxerrabortmask=parm[41].value.unumber, .intl2sfifooverrunmask=parm[42].value.unumber, .intco1588tsmask=parm[43].value.unumber, .intcorptpresmask=parm[44].value.unumber, .intcogntpresmask=parm[45].value.unumber, .intcodelstalegntmask=parm[46].value.unumber, .intcogntnonpollmask=parm[47].value.unumber, .intcogntmisalignmask=parm[48].value.unumber, .intcognttoofarmask=parm[49].value.unumber, .intcogntintervalmask=parm[50].value.unumber, .intcogntdiscoverymask=parm[51].value.unumber, .intcogntmissabortmask=parm[52].value.unumber, .intcogntfullabortmask=parm[53].value.unumber, .badupfrlenmask=parm[54].value.unumber, .uptardypacketmask=parm[55].value.unumber, .uprptfrxmtmask=parm[56].value.unumber, .intbififooverrunmask=parm[57].value.unumber, .burstgnttoobigmask=parm[58].value.unumber, .wrgnttoobigmask=parm[59].value.unumber, .rcvgnttoobigmask=parm[60].value.unumber, .dnstatsoverrunmask=parm[61].value.unumber, .intupstatsoverrunmask=parm[62].value.unumber, .dnoutofordermask=parm[63].value.unumber, .truantbbhhaltmask=parm[64].value.unumber, .upinvldgntlenmask=parm[65].value.unumber, .intcobbhupsfaultmask=parm[66].value.unumber, .dntimeinsyncmask=parm[67].value.unumber, .dntimenotinsyncmask=parm[68].value.unumber, .dportrdymask=parm[69].value.unumber};
        err = ag_drv_epn_main_int_mask_set(&main_int_mask);
        break;
    }
    case BDMF_max_gnt_size:
        err = ag_drv_epn_max_gnt_size_set(parm[38].value.unumber);
        break;
    case BDMF_max_frame_size:
        err = ag_drv_epn_max_frame_size_set(parm[38].value.unumber);
        break;
    case BDMF_grant_ovr_hd:
        err = ag_drv_epn_grant_ovr_hd_set(parm[38].value.unumber, parm[39].value.unumber);
        break;
    case BDMF_poll_size:
        err = ag_drv_epn_poll_size_set(parm[38].value.unumber, parm[39].value.unumber);
        break;
    case BDMF_dn_rd_gnt_margin:
        err = ag_drv_epn_dn_rd_gnt_margin_set(parm[38].value.unumber);
        break;
    case BDMF_gnt_time_start_delta:
        err = ag_drv_epn_gnt_time_start_delta_set(parm[38].value.unumber);
        break;
    case BDMF_time_stamp_diff:
        err = ag_drv_epn_time_stamp_diff_set(parm[38].value.unumber);
        break;
    case BDMF_up_time_stamp_off:
        err = ag_drv_epn_up_time_stamp_off_set(parm[38].value.unumber, parm[39].value.unumber);
        break;
    case BDMF_gnt_interval:
        err = ag_drv_epn_gnt_interval_set(parm[38].value.unumber);
        break;
    case BDMF_dn_gnt_misalign_thr:
        err = ag_drv_epn_dn_gnt_misalign_thr_set(parm[38].value.unumber, parm[39].value.unumber);
        break;
    case BDMF_dn_gnt_misalign_pause:
        err = ag_drv_epn_dn_gnt_misalign_pause_set(parm[38].value.unumber);
        break;
    case BDMF_non_poll_intv:
        err = ag_drv_epn_non_poll_intv_set(parm[38].value.unumber);
        break;
    case BDMF_force_fcs_err:
        err = ag_drv_epn_force_fcs_err_set(parm[38].value.unumber, parm[39].value.unumber);
        break;
    case BDMF_grant_overlap_limit:
        err = ag_drv_epn_grant_overlap_limit_set(parm[39].value.unumber);
        break;
    case BDMF_aes_configuration_0:
        err = ag_drv_epn_aes_configuration_0_set(parm[39].value.unumber, parm[40].value.unumber);
        break;
    case BDMF_aes_configuration_1:
        err = ag_drv_epn_aes_configuration_1_set(parm[40].value.unumber, parm[41].value.unumber);
        break;
    case BDMF_disc_grant_ovr_hd:
        err = ag_drv_epn_disc_grant_ovr_hd_set(parm[41].value.unumber);
        break;
    case BDMF_dn_discovery_seed:
        err = ag_drv_epn_dn_discovery_seed_set(parm[41].value.unumber);
        break;
    case BDMF_dn_discovery_inc:
        err = ag_drv_epn_dn_discovery_inc_set(parm[41].value.unumber);
        break;
    case BDMF_dn_discovery_size:
        err = ag_drv_epn_dn_discovery_size_set(parm[41].value.unumber);
        break;
    case BDMF_prog_rpt_vec:
        err = ag_drv_epn_prog_rpt_vec_set(parm[41].value.unumber);
        break;
    case BDMF_fec_ipg_length:
        err = ag_drv_epn_fec_ipg_length_set(parm[41].value.unumber, parm[42].value.unumber, parm[43].value.unumber, parm[44].value.unumber);
        break;
    case BDMF_fake_report_value_en:
        err = ag_drv_epn_fake_report_value_en_set(parm[41].value.unumber);
        break;
    case BDMF_fake_report_value:
        err = ag_drv_epn_fake_report_value_set(parm[41].value.unumber);
        break;
    case BDMF_burst_cap:
        err = ag_drv_epn_burst_cap_set(parm[41].value.unumber, parm[42].value.unumber);
        break;
    case BDMF_queue_llid_map:
        err = ag_drv_epn_queue_llid_map_set(parm[41].value.unumber, parm[42].value.unumber);
        break;
    case BDMF_valid_opcode_map:
        err = ag_drv_epn_valid_opcode_map_set(parm[41].value.unumber);
        break;
    case BDMF_up_packet_tx_margin:
        err = ag_drv_epn_up_packet_tx_margin_set(parm[41].value.unumber);
        break;
    case BDMF_multi_pri_cfg_0:
    {
        epn_multi_pri_cfg_0 multi_pri_cfg_0 = { .cfgctcschdeficiten=parm[41].value.unumber, .prvzeroburstcapoverridemode=parm[42].value.unumber, .cfgsharedl2=parm[43].value.unumber, .cfgsharedburstcap=parm[44].value.unumber, .cfgrptgntsoutst0=parm[45].value.unumber, .cfgrpthiprifirst0=parm[46].value.unumber, .cfgrptswapqs0=parm[47].value.unumber, .cfgrptmultipri0=parm[48].value.unumber};
        err = ag_drv_epn_multi_pri_cfg_0_set(&multi_pri_cfg_0);
        break;
    }
    case BDMF_forced_report_en:
        err = ag_drv_epn_forced_report_en_set(parm[41].value.unumber, parm[42].value.unumber);
        break;
    case BDMF_forced_report_max_interval:
        err = ag_drv_epn_forced_report_max_interval_set(parm[42].value.unumber);
        break;
    case BDMF_l2s_flush_config:
        err = ag_drv_epn_l2s_flush_config_set(parm[42].value.unumber, parm[43].value.unumber, parm[44].value.unumber);
        break;
    case BDMF_data_port_command:
        err = ag_drv_epn_data_port_command_set(parm[42].value.unumber, parm[43].value.unumber, parm[44].value.unumber);
        break;
    case BDMF_data_port_address:
        err = ag_drv_epn_data_port_address_set(parm[42].value.unumber);
        break;
    case BDMF_data_port_data_0:
        err = ag_drv_epn_data_port_data_0_set(parm[42].value.unumber);
        break;
    case BDMF_unmap_big_cnt:
        err = ag_drv_epn_unmap_big_cnt_set(parm[42].value.unumber);
        break;
    case BDMF_unmap_frame_cnt:
        err = ag_drv_epn_unmap_frame_cnt_set(parm[42].value.unumber);
        break;
    case BDMF_unmap_fcs_cnt:
        err = ag_drv_epn_unmap_fcs_cnt_set(parm[42].value.unumber);
        break;
    case BDMF_unmap_gate_cnt:
        err = ag_drv_epn_unmap_gate_cnt_set(parm[42].value.unumber);
        break;
    case BDMF_unmap_oam_cnt:
        err = ag_drv_epn_unmap_oam_cnt_set(parm[42].value.unumber);
        break;
    case BDMF_unmap_small_cnt:
        err = ag_drv_epn_unmap_small_cnt_set(parm[42].value.unumber);
        break;
    case BDMF_fif_dequeue_event_cnt:
        err = ag_drv_epn_fif_dequeue_event_cnt_set(parm[42].value.unumber);
        break;
    case BDMF_unused_tq_cnt:
        err = ag_drv_epn_unused_tq_cnt_set(parm[42].value.unumber, parm[43].value.unumber);
        break;
    case BDMF_bbh_up_fault_halt_en:
        err = ag_drv_epn_bbh_up_fault_halt_en_set(parm[42].value.unumber, parm[43].value.unumber);
        break;
    case BDMF_bbh_up_tardy_halt_en:
        err = ag_drv_epn_bbh_up_tardy_halt_en_set(parm[43].value.unumber);
        break;
    case BDMF_debug_l2s_ptr_sel:
        err = ag_drv_epn_debug_l2s_ptr_sel_set(parm[43].value.unumber, parm[44].value.unumber);
        break;
    case BDMF_olt_mac_addr_lo:
        err = ag_drv_epn_olt_mac_addr_lo_set(parm[43].value.unumber);
        break;
    case BDMF_olt_mac_addr_hi:
        err = ag_drv_epn_olt_mac_addr_hi_set(parm[43].value.unumber);
        break;
    case BDMF_tx_l1s_shp_que_mask:
        err = ag_drv_epn_tx_l1s_shp_que_mask_set(parm[43].value.unumber, parm[44].value.unumber);
        break;
    case BDMF_tx_l2s_queue_config:
        err = ag_drv_epn_tx_l2s_queue_config_set(parm[43].value.unumber, parm[44].value.unumber, parm[45].value.unumber);
        break;
    case BDMF_tx_ctc_burst_limit:
        err = ag_drv_epn_tx_ctc_burst_limit_set(parm[43].value.unumber, parm[44].value.unumber);
        break;
    case BDMF_bbh_max_outstanding_tardy_packets:
        err = ag_drv_epn_bbh_max_outstanding_tardy_packets_set(parm[43].value.unumber);
        break;
    case BDMF_min_report_value_difference:
        err = ag_drv_epn_min_report_value_difference_set(parm[43].value.unumber);
        break;
    case BDMF_spare_ctl:
        err = ag_drv_epn_spare_ctl_set(parm[43].value.unumber);
        break;
    case BDMF_ts_sync_offset:
        err = ag_drv_epn_ts_sync_offset_set(parm[43].value.unumber);
        break;
    case BDMF_dn_ts_offset:
        err = ag_drv_epn_dn_ts_offset_set(parm[43].value.unumber);
        break;
    case BDMF_up_ts_offset_lo:
        err = ag_drv_epn_up_ts_offset_lo_set(parm[43].value.unumber);
        break;
    case BDMF_up_ts_offset_hi:
        err = ag_drv_epn_up_ts_offset_hi_set(parm[43].value.unumber);
        break;
    case BDMF_two_step_ts_ctl:
        err = ag_drv_epn_two_step_ts_ctl_set(parm[43].value.unumber, parm[44].value.unumber);
        break;
    case BDMF_1588_timestamp_int_status:
        err = ag_drv_epn_1588_timestamp_int_status_set(parm[43].value.unumber, parm[44].value.unumber);
        break;
    case BDMF_1588_timestamp_int_mask:
        err = ag_drv_epn_1588_timestamp_int_mask_set(parm[43].value.unumber, parm[44].value.unumber);
        break;
    case BDMF_up_packet_fetch_margin:
        err = ag_drv_epn_up_packet_fetch_margin_set(parm[43].value.unumber);
        break;
    case BDMF_persistent_report_cfg:
        err = ag_drv_epn_persistent_report_cfg_set(parm[43].value.unumber, parm[44].value.unumber);
        break;
    case BDMF_persistent_report_enables:
        err = ag_drv_epn_persistent_report_enables_set(parm[43].value.unumber, parm[44].value.unumber);
        break;
    case BDMF_persistent_report_request_size:
        err = ag_drv_epn_persistent_report_request_size_set(parm[44].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_epn_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_control_0:
    {
        epn_control_0 control_0;
        err = ag_drv_epn_control_0_get(&control_0);
        bdmf_session_print(session, "cfgen1588ts = %u = 0x%x\n", control_0.cfgen1588ts, control_0.cfgen1588ts);
        bdmf_session_print(session, "cfgreplaceupfcs = %u = 0x%x\n", control_0.cfgreplaceupfcs, control_0.cfgreplaceupfcs);
        bdmf_session_print(session, "cfgappendupfcs = %u = 0x%x\n", control_0.cfgappendupfcs, control_0.cfgappendupfcs);
        bdmf_session_print(session, "cfgdropscb = %u = 0x%x\n", control_0.cfgdropscb, control_0.cfgdropscb);
        bdmf_session_print(session, "moduncappedreportlimit = %u = 0x%x\n", control_0.moduncappedreportlimit, control_0.moduncappedreportlimit);
        bdmf_session_print(session, "modmpquesetfirst = %u = 0x%x\n", control_0.modmpquesetfirst, control_0.modmpquesetfirst);
        bdmf_session_print(session, "prvlocalmpcppropagation = %u = 0x%x\n", control_0.prvlocalmpcppropagation, control_0.prvlocalmpcppropagation);
        bdmf_session_print(session, "prvtekmodeprefetch = %u = 0x%x\n", control_0.prvtekmodeprefetch, control_0.prvtekmodeprefetch);
        bdmf_session_print(session, "prvincnonzeroaccum = %u = 0x%x\n", control_0.prvincnonzeroaccum, control_0.prvincnonzeroaccum);
        bdmf_session_print(session, "prvnounmapppedfcs = %u = 0x%x\n", control_0.prvnounmapppedfcs, control_0.prvnounmapppedfcs);
        bdmf_session_print(session, "prvsupressdiscen = %u = 0x%x\n", control_0.prvsupressdiscen, control_0.prvsupressdiscen);
        bdmf_session_print(session, "cfgvlanmax = %u = 0x%x\n", control_0.cfgvlanmax, control_0.cfgvlanmax);
        bdmf_session_print(session, "fcserronlydatafr = %u = 0x%x\n", control_0.fcserronlydatafr, control_0.fcserronlydatafr);
        bdmf_session_print(session, "prvdropunmapppedllid = %u = 0x%x\n", control_0.prvdropunmapppedllid, control_0.prvdropunmapppedllid);
        bdmf_session_print(session, "prvsuppressllidmodebit = %u = 0x%x\n", control_0.prvsuppressllidmodebit, control_0.prvsuppressllidmodebit);
        bdmf_session_print(session, "moddiscoverydafilteren = %u = 0x%x\n", control_0.moddiscoverydafilteren, control_0.moddiscoverydafilteren);
        bdmf_session_print(session, "rptselect = %u = 0x%x\n", control_0.rptselect, control_0.rptselect);
        bdmf_session_print(session, "prvdisablesvaquestatusbp = %u = 0x%x\n", control_0.prvdisablesvaquestatusbp, control_0.prvdisablesvaquestatusbp);
        bdmf_session_print(session, "utxloopback = %u = 0x%x\n", control_0.utxloopback, control_0.utxloopback);
        bdmf_session_print(session, "utxen = %u = 0x%x\n", control_0.utxen, control_0.utxen);
        bdmf_session_print(session, "utxrst_pre_n = %u = 0x%x\n", control_0.utxrst_pre_n, control_0.utxrst_pre_n);
        bdmf_session_print(session, "cfgdisabledns = %u = 0x%x\n", control_0.cfgdisabledns, control_0.cfgdisabledns);
        bdmf_session_print(session, "drxloopback = %u = 0x%x\n", control_0.drxloopback, control_0.drxloopback);
        bdmf_session_print(session, "drxen = %u = 0x%x\n", control_0.drxen, control_0.drxen);
        bdmf_session_print(session, "drxrst_pre_n = %u = 0x%x\n", control_0.drxrst_pre_n, control_0.drxrst_pre_n);
        break;
    }
    case BDMF_control_1:
    {
        epn_control_1 control_1;
        err = ag_drv_epn_control_1_get(&control_1);
        bdmf_session_print(session, "prvoverlappedgntenable = %u = 0x%x\n", control_1.prvoverlappedgntenable, control_1.prvoverlappedgntenable);
        bdmf_session_print(session, "rstmisalignthr = %u = 0x%x\n", control_1.rstmisalignthr, control_1.rstmisalignthr);
        bdmf_session_print(session, "cfgtekrpt = %u = 0x%x\n", control_1.cfgtekrpt, control_1.cfgtekrpt);
        bdmf_session_print(session, "cfgstalegntchk = %u = 0x%x\n", control_1.cfgstalegntchk, control_1.cfgstalegntchk);
        bdmf_session_print(session, "fecrpten = %u = 0x%x\n", control_1.fecrpten, control_1.fecrpten);
        bdmf_session_print(session, "cfgl1l2truestrict = %u = 0x%x\n", control_1.cfgl1l2truestrict, control_1.cfgl1l2truestrict);
        bdmf_session_print(session, "cfgctcrpt = %u = 0x%x\n", control_1.cfgctcrpt, control_1.cfgctcrpt);
        bdmf_session_print(session, "cfgtscorrdis = %u = 0x%x\n", control_1.cfgtscorrdis, control_1.cfgtscorrdis);
        bdmf_session_print(session, "cfgnodiscrpt = %u = 0x%x\n", control_1.cfgnodiscrpt, control_1.cfgnodiscrpt);
        bdmf_session_print(session, "disablediscscale = %u = 0x%x\n", control_1.disablediscscale, control_1.disablediscscale);
        bdmf_session_print(session, "clronrd = %u = 0x%x\n", control_1.clronrd, control_1.clronrd);
        break;
    }
    case BDMF_enable_grants:
    {
        bdmf_boolean grant_en;
        err = ag_drv_epn_enable_grants_get(parm[1].value.unumber, &grant_en);
        bdmf_session_print(session, "grant_en = %u = 0x%x\n", grant_en, grant_en);
        break;
    }
    case BDMF_drop_disc_gates:
    {
        bdmf_boolean linkDiscGates_en;
        err = ag_drv_epn_drop_disc_gates_get(parm[1].value.unumber, &linkDiscGates_en);
        bdmf_session_print(session, "linkDiscGates_en = %u = 0x%x\n", linkDiscGates_en, linkDiscGates_en);
        break;
    }
    case BDMF_dis_fcs_chk:
    {
        bdmf_boolean disableFcsChk;
        err = ag_drv_epn_dis_fcs_chk_get(parm[1].value.unumber, &disableFcsChk);
        bdmf_session_print(session, "disableFcsChk = %u = 0x%x\n", disableFcsChk, disableFcsChk);
        break;
    }
    case BDMF_pass_gates:
    {
        bdmf_boolean passGates;
        err = ag_drv_epn_pass_gates_get(parm[1].value.unumber, &passGates);
        bdmf_session_print(session, "passGates = %u = 0x%x\n", passGates, passGates);
        break;
    }
    case BDMF_cfg_misalgn_fb:
    {
        bdmf_boolean cfgMisalignFeedback;
        err = ag_drv_epn_cfg_misalgn_fb_get(parm[1].value.unumber, &cfgMisalignFeedback);
        bdmf_session_print(session, "cfgMisalignFeedback = %u = 0x%x\n", cfgMisalignFeedback, cfgMisalignFeedback);
        break;
    }
    case BDMF_discovery_filter:
    {
        uint16_t prvdiscinfomask;
        uint16_t prvdiscinfovalue;
        err = ag_drv_epn_discovery_filter_get(&prvdiscinfomask, &prvdiscinfovalue);
        bdmf_session_print(session, "prvdiscinfomask = %u = 0x%x\n", prvdiscinfomask, prvdiscinfomask);
        bdmf_session_print(session, "prvdiscinfovalue = %u = 0x%x\n", prvdiscinfovalue, prvdiscinfovalue);
        break;
    }
    case BDMF_bbh_que_status_delay:
    {
        uint8_t prvbbhquestatdelay;
        err = ag_drv_epn_bbh_que_status_delay_get(&prvbbhquestatdelay);
        bdmf_session_print(session, "prvbbhquestatdelay = %u = 0x%x\n", prvbbhquestatdelay, prvbbhquestatdelay);
        break;
    }
    case BDMF_minimum_grant_setup:
    {
        uint16_t cfgmingrantsetup;
        err = ag_drv_epn_minimum_grant_setup_get(&cfgmingrantsetup);
        bdmf_session_print(session, "cfgmingrantsetup = %u = 0x%x\n", cfgmingrantsetup, cfgmingrantsetup);
        break;
    }
    case BDMF_reset_gnt_fifo:
    {
        bdmf_boolean rstGntFifo;
        err = ag_drv_epn_reset_gnt_fifo_get(parm[1].value.unumber, &rstGntFifo);
        bdmf_session_print(session, "rstGntFifo = %u = 0x%x\n", rstGntFifo, rstGntFifo);
        break;
    }
    case BDMF_reset_l1_accumulator:
    {
        uint32_t cfgl1sclracum;
        err = ag_drv_epn_reset_l1_accumulator_get(&cfgl1sclracum);
        bdmf_session_print(session, "cfgl1sclracum = %u = 0x%x\n", cfgl1sclracum, cfgl1sclracum);
        break;
    }
    case BDMF_l1_accumulator_sel:
    {
        uint8_t cfgl1suvasizesel;
        uint8_t cfgl1ssvasizesel;
        err = ag_drv_epn_l1_accumulator_sel_get(&cfgl1suvasizesel, &cfgl1ssvasizesel);
        bdmf_session_print(session, "cfgl1suvasizesel = %u = 0x%x\n", cfgl1suvasizesel, cfgl1suvasizesel);
        bdmf_session_print(session, "cfgl1ssvasizesel = %u = 0x%x\n", cfgl1ssvasizesel, cfgl1ssvasizesel);
        break;
    }
    case BDMF_l1_sva_bytes:
    {
        uint32_t l1ssvasize;
        err = ag_drv_epn_l1_sva_bytes_get(&l1ssvasize);
        bdmf_session_print(session, "l1ssvasize = %u = 0x%x\n", l1ssvasize, l1ssvasize);
        break;
    }
    case BDMF_l1_uva_bytes:
    {
        uint32_t l1suvasize;
        err = ag_drv_epn_l1_uva_bytes_get(&l1suvasize);
        bdmf_session_print(session, "l1suvasize = %u = 0x%x\n", l1suvasize, l1suvasize);
        break;
    }
    case BDMF_l1_sva_overflow:
    {
        uint32_t l1ssvaoverflow;
        err = ag_drv_epn_l1_sva_overflow_get(&l1ssvaoverflow);
        bdmf_session_print(session, "l1ssvaoverflow = %u = 0x%x\n", l1ssvaoverflow, l1ssvaoverflow);
        break;
    }
    case BDMF_l1_uva_overflow:
    {
        uint32_t l1suvaoverflow;
        err = ag_drv_epn_l1_uva_overflow_get(&l1suvaoverflow);
        bdmf_session_print(session, "l1suvaoverflow = %u = 0x%x\n", l1suvaoverflow, l1suvaoverflow);
        break;
    }
    case BDMF_reset_rpt_pri:
    {
        epn_reset_rpt_pri reset_rpt_pri;
        err = ag_drv_epn_reset_rpt_pri_get(&reset_rpt_pri);
        bdmf_session_print(session, "nullrptpri15 = %u = 0x%x\n", reset_rpt_pri.nullrptpri15, reset_rpt_pri.nullrptpri15);
        bdmf_session_print(session, "nullrptpri14 = %u = 0x%x\n", reset_rpt_pri.nullrptpri14, reset_rpt_pri.nullrptpri14);
        bdmf_session_print(session, "nullrptpri13 = %u = 0x%x\n", reset_rpt_pri.nullrptpri13, reset_rpt_pri.nullrptpri13);
        bdmf_session_print(session, "nullrptpri12 = %u = 0x%x\n", reset_rpt_pri.nullrptpri12, reset_rpt_pri.nullrptpri12);
        bdmf_session_print(session, "nullrptpri11 = %u = 0x%x\n", reset_rpt_pri.nullrptpri11, reset_rpt_pri.nullrptpri11);
        bdmf_session_print(session, "nullrptpri10 = %u = 0x%x\n", reset_rpt_pri.nullrptpri10, reset_rpt_pri.nullrptpri10);
        bdmf_session_print(session, "nullrptpri9 = %u = 0x%x\n", reset_rpt_pri.nullrptpri9, reset_rpt_pri.nullrptpri9);
        bdmf_session_print(session, "nullrptpri8 = %u = 0x%x\n", reset_rpt_pri.nullrptpri8, reset_rpt_pri.nullrptpri8);
        bdmf_session_print(session, "nullrptpri7 = %u = 0x%x\n", reset_rpt_pri.nullrptpri7, reset_rpt_pri.nullrptpri7);
        bdmf_session_print(session, "nullrptpri6 = %u = 0x%x\n", reset_rpt_pri.nullrptpri6, reset_rpt_pri.nullrptpri6);
        bdmf_session_print(session, "nullrptpri5 = %u = 0x%x\n", reset_rpt_pri.nullrptpri5, reset_rpt_pri.nullrptpri5);
        bdmf_session_print(session, "nullrptpri4 = %u = 0x%x\n", reset_rpt_pri.nullrptpri4, reset_rpt_pri.nullrptpri4);
        bdmf_session_print(session, "nullrptpri3 = %u = 0x%x\n", reset_rpt_pri.nullrptpri3, reset_rpt_pri.nullrptpri3);
        bdmf_session_print(session, "nullrptpri2 = %u = 0x%x\n", reset_rpt_pri.nullrptpri2, reset_rpt_pri.nullrptpri2);
        bdmf_session_print(session, "nullrptpri1 = %u = 0x%x\n", reset_rpt_pri.nullrptpri1, reset_rpt_pri.nullrptpri1);
        bdmf_session_print(session, "nullrptpri0 = %u = 0x%x\n", reset_rpt_pri.nullrptpri0, reset_rpt_pri.nullrptpri0);
        break;
    }
    case BDMF_reset_l2_rpt_fifo:
    {
        bdmf_boolean cfgL2SClrQue;
        err = ag_drv_epn_reset_l2_rpt_fifo_get(parm[1].value.unumber, &cfgL2SClrQue);
        bdmf_session_print(session, "cfgL2SClrQue = %u = 0x%x\n", cfgL2SClrQue, cfgL2SClrQue);
        break;
    }
    case BDMF_enable_upstream:
    {
        bdmf_boolean cfgEnableUpstream;
        err = ag_drv_epn_enable_upstream_get(parm[1].value.unumber, &cfgEnableUpstream);
        bdmf_session_print(session, "cfgEnableUpstream = %u = 0x%x\n", cfgEnableUpstream, cfgEnableUpstream);
        break;
    }
    case BDMF_enable_upstream_fb:
    {
        bdmf_boolean cfgEnableUpstreamFeedBack;
        err = ag_drv_epn_enable_upstream_fb_get(parm[1].value.unumber, &cfgEnableUpstreamFeedBack);
        bdmf_session_print(session, "cfgEnableUpstreamFeedBack = %u = 0x%x\n", cfgEnableUpstreamFeedBack, cfgEnableUpstreamFeedBack);
        break;
    }
    case BDMF_enable_upstream_fec:
    {
        bdmf_boolean upstreamFecEn;
        err = ag_drv_epn_enable_upstream_fec_get(parm[1].value.unumber, &upstreamFecEn);
        bdmf_session_print(session, "upstreamFecEn = %u = 0x%x\n", upstreamFecEn, upstreamFecEn);
        break;
    }
    case BDMF_report_byte_length:
    {
        uint8_t prvrptbytelen;
        err = ag_drv_epn_report_byte_length_get(&prvrptbytelen);
        bdmf_session_print(session, "prvrptbytelen = %u = 0x%x\n", prvrptbytelen, prvrptbytelen);
        break;
    }
    case BDMF_main_int_status:
    {
        epn_main_int_status main_int_status;
        err = ag_drv_epn_main_int_status_get(&main_int_status);
        bdmf_session_print(session, "intbbhupfrabort = %u = 0x%x\n", main_int_status.intbbhupfrabort, main_int_status.intbbhupfrabort);
        bdmf_session_print(session, "intcol2sburstcapoverflowpres = %u = 0x%x\n", main_int_status.intcol2sburstcapoverflowpres, main_int_status.intcol2sburstcapoverflowpres);
        bdmf_session_print(session, "intcoemptyrpt = %u = 0x%x\n", main_int_status.intcoemptyrpt, main_int_status.intcoemptyrpt);
        bdmf_session_print(session, "intcodrxerrabortpres = %u = 0x%x\n", main_int_status.intcodrxerrabortpres, main_int_status.intcodrxerrabortpres);
        bdmf_session_print(session, "intl2sfifooverrun = %u = 0x%x\n", main_int_status.intl2sfifooverrun, main_int_status.intl2sfifooverrun);
        bdmf_session_print(session, "intco1588tsint = %u = 0x%x\n", main_int_status.intco1588tsint, main_int_status.intco1588tsint);
        bdmf_session_print(session, "intcorptpres = %u = 0x%x\n", main_int_status.intcorptpres, main_int_status.intcorptpres);
        bdmf_session_print(session, "intcogntpres = %u = 0x%x\n", main_int_status.intcogntpres, main_int_status.intcogntpres);
        bdmf_session_print(session, "intcodelstalegnt = %u = 0x%x\n", main_int_status.intcodelstalegnt, main_int_status.intcodelstalegnt);
        bdmf_session_print(session, "intcogntnonpoll = %u = 0x%x\n", main_int_status.intcogntnonpoll, main_int_status.intcogntnonpoll);
        bdmf_session_print(session, "intcogntmisalign = %u = 0x%x\n", main_int_status.intcogntmisalign, main_int_status.intcogntmisalign);
        bdmf_session_print(session, "intcognttoofar = %u = 0x%x\n", main_int_status.intcognttoofar, main_int_status.intcognttoofar);
        bdmf_session_print(session, "intcogntinterval = %u = 0x%x\n", main_int_status.intcogntinterval, main_int_status.intcogntinterval);
        bdmf_session_print(session, "intcogntdiscovery = %u = 0x%x\n", main_int_status.intcogntdiscovery, main_int_status.intcogntdiscovery);
        bdmf_session_print(session, "intcogntmissabort = %u = 0x%x\n", main_int_status.intcogntmissabort, main_int_status.intcogntmissabort);
        bdmf_session_print(session, "intcogntfullabort = %u = 0x%x\n", main_int_status.intcogntfullabort, main_int_status.intcogntfullabort);
        bdmf_session_print(session, "intbadupfrlen = %u = 0x%x\n", main_int_status.intbadupfrlen, main_int_status.intbadupfrlen);
        bdmf_session_print(session, "intuptardypacket = %u = 0x%x\n", main_int_status.intuptardypacket, main_int_status.intuptardypacket);
        bdmf_session_print(session, "intuprptfrxmt = %u = 0x%x\n", main_int_status.intuprptfrxmt, main_int_status.intuprptfrxmt);
        bdmf_session_print(session, "intbififooverrun = %u = 0x%x\n", main_int_status.intbififooverrun, main_int_status.intbififooverrun);
        bdmf_session_print(session, "intburstgnttoobig = %u = 0x%x\n", main_int_status.intburstgnttoobig, main_int_status.intburstgnttoobig);
        bdmf_session_print(session, "intwrgnttoobig = %u = 0x%x\n", main_int_status.intwrgnttoobig, main_int_status.intwrgnttoobig);
        bdmf_session_print(session, "intrcvgnttoobig = %u = 0x%x\n", main_int_status.intrcvgnttoobig, main_int_status.intrcvgnttoobig);
        bdmf_session_print(session, "intdnstatsoverrun = %u = 0x%x\n", main_int_status.intdnstatsoverrun, main_int_status.intdnstatsoverrun);
        bdmf_session_print(session, "intupstatsoverrun = %u = 0x%x\n", main_int_status.intupstatsoverrun, main_int_status.intupstatsoverrun);
        bdmf_session_print(session, "intdnoutoforder = %u = 0x%x\n", main_int_status.intdnoutoforder, main_int_status.intdnoutoforder);
        bdmf_session_print(session, "inttruantbbhhalt = %u = 0x%x\n", main_int_status.inttruantbbhhalt, main_int_status.inttruantbbhhalt);
        bdmf_session_print(session, "intupinvldgntlen = %u = 0x%x\n", main_int_status.intupinvldgntlen, main_int_status.intupinvldgntlen);
        bdmf_session_print(session, "intcobbhupsfault = %u = 0x%x\n", main_int_status.intcobbhupsfault, main_int_status.intcobbhupsfault);
        bdmf_session_print(session, "intdntimeinsync = %u = 0x%x\n", main_int_status.intdntimeinsync, main_int_status.intdntimeinsync);
        bdmf_session_print(session, "intdntimenotinsync = %u = 0x%x\n", main_int_status.intdntimenotinsync, main_int_status.intdntimenotinsync);
        bdmf_session_print(session, "intdportrdy = %u = 0x%x\n", main_int_status.intdportrdy, main_int_status.intdportrdy);
        break;
    }
    case BDMF_gnt_full_int_status:
    {
        bdmf_boolean intDnGntFullAbort;
        err = ag_drv_epn_gnt_full_int_status_get(parm[1].value.unumber, &intDnGntFullAbort);
        bdmf_session_print(session, "intDnGntFullAbort = %u = 0x%x\n", intDnGntFullAbort, intDnGntFullAbort);
        break;
    }
    case BDMF_gnt_full_int_mask:
    {
        bdmf_boolean maskIntDnGntFullAbort;
        err = ag_drv_epn_gnt_full_int_mask_get(parm[1].value.unumber, &maskIntDnGntFullAbort);
        bdmf_session_print(session, "maskIntDnGntFullAbort = %u = 0x%x\n", maskIntDnGntFullAbort, maskIntDnGntFullAbort);
        break;
    }
    case BDMF_gnt_miss_int_status:
    {
        bdmf_boolean intDnGntMissAbort;
        err = ag_drv_epn_gnt_miss_int_status_get(parm[1].value.unumber, &intDnGntMissAbort);
        bdmf_session_print(session, "intDnGntMissAbort = %u = 0x%x\n", intDnGntMissAbort, intDnGntMissAbort);
        break;
    }
    case BDMF_gnt_miss_int_mask:
    {
        bdmf_boolean maskIntDnGntMissAbort;
        err = ag_drv_epn_gnt_miss_int_mask_get(parm[1].value.unumber, &maskIntDnGntMissAbort);
        bdmf_session_print(session, "maskIntDnGntMissAbort = %u = 0x%x\n", maskIntDnGntMissAbort, maskIntDnGntMissAbort);
        break;
    }
    case BDMF_disc_rx_int_status:
    {
        bdmf_boolean intDnGntDiscovery;
        err = ag_drv_epn_disc_rx_int_status_get(parm[1].value.unumber, &intDnGntDiscovery);
        bdmf_session_print(session, "intDnGntDiscovery = %u = 0x%x\n", intDnGntDiscovery, intDnGntDiscovery);
        break;
    }
    case BDMF_disc_rx_int_mask:
    {
        bdmf_boolean maskIntDnGntDiscovery;
        err = ag_drv_epn_disc_rx_int_mask_get(parm[1].value.unumber, &maskIntDnGntDiscovery);
        bdmf_session_print(session, "maskIntDnGntDiscovery = %u = 0x%x\n", maskIntDnGntDiscovery, maskIntDnGntDiscovery);
        break;
    }
    case BDMF_gnt_intv_int_status:
    {
        bdmf_boolean intDnGntInterval;
        err = ag_drv_epn_gnt_intv_int_status_get(parm[1].value.unumber, &intDnGntInterval);
        bdmf_session_print(session, "intDnGntInterval = %u = 0x%x\n", intDnGntInterval, intDnGntInterval);
        break;
    }
    case BDMF_gnt_intv_int_mask:
    {
        bdmf_boolean maskIntDnGntInterval;
        err = ag_drv_epn_gnt_intv_int_mask_get(parm[1].value.unumber, &maskIntDnGntInterval);
        bdmf_session_print(session, "maskIntDnGntInterval = %u = 0x%x\n", maskIntDnGntInterval, maskIntDnGntInterval);
        break;
    }
    case BDMF_gnt_far_int_status:
    {
        bdmf_boolean intDnGntTooFar;
        err = ag_drv_epn_gnt_far_int_status_get(parm[1].value.unumber, &intDnGntTooFar);
        bdmf_session_print(session, "intDnGntTooFar = %u = 0x%x\n", intDnGntTooFar, intDnGntTooFar);
        break;
    }
    case BDMF_gnt_far_int_mask:
    {
        bdmf_boolean maskDnGntTooFar;
        err = ag_drv_epn_gnt_far_int_mask_get(parm[1].value.unumber, &maskDnGntTooFar);
        bdmf_session_print(session, "maskDnGntTooFar = %u = 0x%x\n", maskDnGntTooFar, maskDnGntTooFar);
        break;
    }
    case BDMF_gnt_misalgn_int_status:
    {
        bdmf_boolean intDnGntMisalign;
        err = ag_drv_epn_gnt_misalgn_int_status_get(parm[1].value.unumber, &intDnGntMisalign);
        bdmf_session_print(session, "intDnGntMisalign = %u = 0x%x\n", intDnGntMisalign, intDnGntMisalign);
        break;
    }
    case BDMF_gnt_misalgn_int_mask:
    {
        bdmf_boolean maskIntDnGntMisalign;
        err = ag_drv_epn_gnt_misalgn_int_mask_get(parm[1].value.unumber, &maskIntDnGntMisalign);
        bdmf_session_print(session, "maskIntDnGntMisalign = %u = 0x%x\n", maskIntDnGntMisalign, maskIntDnGntMisalign);
        break;
    }
    case BDMF_np_gnt_int_status:
    {
        bdmf_boolean intDnGntNonPoll;
        err = ag_drv_epn_np_gnt_int_status_get(parm[1].value.unumber, &intDnGntNonPoll);
        bdmf_session_print(session, "intDnGntNonPoll = %u = 0x%x\n", intDnGntNonPoll, intDnGntNonPoll);
        break;
    }
    case BDMF_np_gnt_int_mask:
    {
        bdmf_boolean maskDnGntNonPoll;
        err = ag_drv_epn_np_gnt_int_mask_get(parm[1].value.unumber, &maskDnGntNonPoll);
        bdmf_session_print(session, "maskDnGntNonPoll = %u = 0x%x\n", maskDnGntNonPoll, maskDnGntNonPoll);
        break;
    }
    case BDMF_del_stale_int_status:
    {
        bdmf_boolean intDelStaleGnt;
        err = ag_drv_epn_del_stale_int_status_get(parm[1].value.unumber, &intDelStaleGnt);
        bdmf_session_print(session, "intDelStaleGnt = %u = 0x%x\n", intDelStaleGnt, intDelStaleGnt);
        break;
    }
    case BDMF_del_stale_int_mask:
    {
        bdmf_boolean maskIntDelStaleGnt;
        err = ag_drv_epn_del_stale_int_mask_get(parm[1].value.unumber, &maskIntDelStaleGnt);
        bdmf_session_print(session, "maskIntDelStaleGnt = %u = 0x%x\n", maskIntDelStaleGnt, maskIntDelStaleGnt);
        break;
    }
    case BDMF_gnt_pres_int_status:
    {
        bdmf_boolean intDnGntRdy;
        err = ag_drv_epn_gnt_pres_int_status_get(parm[1].value.unumber, &intDnGntRdy);
        bdmf_session_print(session, "intDnGntRdy = %u = 0x%x\n", intDnGntRdy, intDnGntRdy);
        break;
    }
    case BDMF_gnt_pres_int_mask:
    {
        bdmf_boolean maskDnGntRdy;
        err = ag_drv_epn_gnt_pres_int_mask_get(parm[1].value.unumber, &maskDnGntRdy);
        bdmf_session_print(session, "maskDnGntRdy = %u = 0x%x\n", maskDnGntRdy, maskDnGntRdy);
        break;
    }
    case BDMF_rpt_pres_int_status:
    {
        bdmf_boolean intUpRptFifo;
        err = ag_drv_epn_rpt_pres_int_status_get(parm[1].value.unumber, &intUpRptFifo);
        bdmf_session_print(session, "intUpRptFifo = %u = 0x%x\n", intUpRptFifo, intUpRptFifo);
        break;
    }
    case BDMF_rpt_pres_int_mask:
    {
        bdmf_boolean maskIntUpRptFifo;
        err = ag_drv_epn_rpt_pres_int_mask_get(parm[1].value.unumber, &maskIntUpRptFifo);
        bdmf_session_print(session, "maskIntUpRptFifo = %u = 0x%x\n", maskIntUpRptFifo, maskIntUpRptFifo);
        break;
    }
    case BDMF_drx_abort_int_status:
    {
        bdmf_boolean intDrxErrAbort;
        err = ag_drv_epn_drx_abort_int_status_get(parm[1].value.unumber, &intDrxErrAbort);
        bdmf_session_print(session, "intDrxErrAbort = %u = 0x%x\n", intDrxErrAbort, intDrxErrAbort);
        break;
    }
    case BDMF_drx_abort_int_mask:
    {
        bdmf_boolean maskIntDrxErrAbort;
        err = ag_drv_epn_drx_abort_int_mask_get(parm[1].value.unumber, &maskIntDrxErrAbort);
        bdmf_session_print(session, "maskIntDrxErrAbort = %u = 0x%x\n", maskIntDrxErrAbort, maskIntDrxErrAbort);
        break;
    }
    case BDMF_empty_rpt_int_status:
    {
        bdmf_boolean intEmptyRpt;
        err = ag_drv_epn_empty_rpt_int_status_get(parm[1].value.unumber, &intEmptyRpt);
        bdmf_session_print(session, "intEmptyRpt = %u = 0x%x\n", intEmptyRpt, intEmptyRpt);
        break;
    }
    case BDMF_empty_rpt_int_mask:
    {
        bdmf_boolean  maskIntEmptyRpt;
        err = ag_drv_epn_empty_rpt_int_mask_get(parm[1].value.unumber, & maskIntEmptyRpt);
        bdmf_session_print(session, " maskIntEmptyRpt = %u = 0x%x\n",  maskIntEmptyRpt,  maskIntEmptyRpt);
        break;
    }
    case BDMF_bcap_overflow_int_status:
    {
        bdmf_boolean intl2sBurstCapOverFlow;
        err = ag_drv_epn_bcap_overflow_int_status_get(parm[1].value.unumber, &intl2sBurstCapOverFlow);
        bdmf_session_print(session, "intl2sBurstCapOverFlow = %u = 0x%x\n", intl2sBurstCapOverFlow, intl2sBurstCapOverFlow);
        break;
    }
    case BDMF_bcap_overflow_int_mask:
    {
        bdmf_boolean maskIntl2sBurstCapOverFlow;
        err = ag_drv_epn_bcap_overflow_int_mask_get(parm[1].value.unumber, &maskIntl2sBurstCapOverFlow);
        bdmf_session_print(session, "maskIntl2sBurstCapOverFlow = %u = 0x%x\n", maskIntl2sBurstCapOverFlow, maskIntl2sBurstCapOverFlow);
        break;
    }
    case BDMF_bbh_dns_fault_int_status:
    {
        bdmf_boolean intbbhdnsoverflow;
        err = ag_drv_epn_bbh_dns_fault_int_status_get(&intbbhdnsoverflow);
        bdmf_session_print(session, "intbbhdnsoverflow = %u = 0x%x\n", intbbhdnsoverflow, intbbhdnsoverflow);
        break;
    }
    case BDMF_bbh_dns_fault_int_mask:
    {
        bdmf_boolean maskintbbhdnsoverflow;
        err = ag_drv_epn_bbh_dns_fault_int_mask_get(&maskintbbhdnsoverflow);
        bdmf_session_print(session, "maskintbbhdnsoverflow = %u = 0x%x\n", maskintbbhdnsoverflow, maskintbbhdnsoverflow);
        break;
    }
    case BDMF_bbh_ups_fault_int_status:
    {
        bdmf_boolean intBbhUpsFault;
        err = ag_drv_epn_bbh_ups_fault_int_status_get(parm[1].value.unumber, &intBbhUpsFault);
        bdmf_session_print(session, "intBbhUpsFault = %u = 0x%x\n", intBbhUpsFault, intBbhUpsFault);
        break;
    }
    case BDMF_bbh_ups_fault_int_mask:
    {
        bdmf_boolean maskIntBbhUpsFault;
        err = ag_drv_epn_bbh_ups_fault_int_mask_get(parm[1].value.unumber, &maskIntBbhUpsFault);
        bdmf_session_print(session, "maskIntBbhUpsFault = %u = 0x%x\n", maskIntBbhUpsFault, maskIntBbhUpsFault);
        break;
    }
    case BDMF_bbh_ups_abort_int_status:
    {
        bdmf_boolean tardybbhabort;
        err = ag_drv_epn_bbh_ups_abort_int_status_get(&tardybbhabort);
        bdmf_session_print(session, "tardybbhabort = %u = 0x%x\n", tardybbhabort, tardybbhabort);
        break;
    }
    case BDMF_bbh_ups_abort_int_mask:
    {
        bdmf_boolean masktardybbhabort;
        err = ag_drv_epn_bbh_ups_abort_int_mask_get(&masktardybbhabort);
        bdmf_session_print(session, "masktardybbhabort = %u = 0x%x\n", masktardybbhabort, masktardybbhabort);
        break;
    }
    case BDMF_main_int_mask:
    {
        epn_main_int_mask main_int_mask;
        err = ag_drv_epn_main_int_mask_get(&main_int_mask);
        bdmf_session_print(session, "bbhupfrabortmask = %u = 0x%x\n", main_int_mask.bbhupfrabortmask, main_int_mask.bbhupfrabortmask);
        bdmf_session_print(session, "intl2sburstcapoverflowmask = %u = 0x%x\n", main_int_mask.intl2sburstcapoverflowmask, main_int_mask.intl2sburstcapoverflowmask);
        bdmf_session_print(session, "intcoemptyrptmask = %u = 0x%x\n", main_int_mask.intcoemptyrptmask, main_int_mask.intcoemptyrptmask);
        bdmf_session_print(session, "intdrxerrabortmask = %u = 0x%x\n", main_int_mask.intdrxerrabortmask, main_int_mask.intdrxerrabortmask);
        bdmf_session_print(session, "intl2sfifooverrunmask = %u = 0x%x\n", main_int_mask.intl2sfifooverrunmask, main_int_mask.intl2sfifooverrunmask);
        bdmf_session_print(session, "intco1588tsmask = %u = 0x%x\n", main_int_mask.intco1588tsmask, main_int_mask.intco1588tsmask);
        bdmf_session_print(session, "intcorptpresmask = %u = 0x%x\n", main_int_mask.intcorptpresmask, main_int_mask.intcorptpresmask);
        bdmf_session_print(session, "intcogntpresmask = %u = 0x%x\n", main_int_mask.intcogntpresmask, main_int_mask.intcogntpresmask);
        bdmf_session_print(session, "intcodelstalegntmask = %u = 0x%x\n", main_int_mask.intcodelstalegntmask, main_int_mask.intcodelstalegntmask);
        bdmf_session_print(session, "intcogntnonpollmask = %u = 0x%x\n", main_int_mask.intcogntnonpollmask, main_int_mask.intcogntnonpollmask);
        bdmf_session_print(session, "intcogntmisalignmask = %u = 0x%x\n", main_int_mask.intcogntmisalignmask, main_int_mask.intcogntmisalignmask);
        bdmf_session_print(session, "intcognttoofarmask = %u = 0x%x\n", main_int_mask.intcognttoofarmask, main_int_mask.intcognttoofarmask);
        bdmf_session_print(session, "intcogntintervalmask = %u = 0x%x\n", main_int_mask.intcogntintervalmask, main_int_mask.intcogntintervalmask);
        bdmf_session_print(session, "intcogntdiscoverymask = %u = 0x%x\n", main_int_mask.intcogntdiscoverymask, main_int_mask.intcogntdiscoverymask);
        bdmf_session_print(session, "intcogntmissabortmask = %u = 0x%x\n", main_int_mask.intcogntmissabortmask, main_int_mask.intcogntmissabortmask);
        bdmf_session_print(session, "intcogntfullabortmask = %u = 0x%x\n", main_int_mask.intcogntfullabortmask, main_int_mask.intcogntfullabortmask);
        bdmf_session_print(session, "badupfrlenmask = %u = 0x%x\n", main_int_mask.badupfrlenmask, main_int_mask.badupfrlenmask);
        bdmf_session_print(session, "uptardypacketmask = %u = 0x%x\n", main_int_mask.uptardypacketmask, main_int_mask.uptardypacketmask);
        bdmf_session_print(session, "uprptfrxmtmask = %u = 0x%x\n", main_int_mask.uprptfrxmtmask, main_int_mask.uprptfrxmtmask);
        bdmf_session_print(session, "intbififooverrunmask = %u = 0x%x\n", main_int_mask.intbififooverrunmask, main_int_mask.intbififooverrunmask);
        bdmf_session_print(session, "burstgnttoobigmask = %u = 0x%x\n", main_int_mask.burstgnttoobigmask, main_int_mask.burstgnttoobigmask);
        bdmf_session_print(session, "wrgnttoobigmask = %u = 0x%x\n", main_int_mask.wrgnttoobigmask, main_int_mask.wrgnttoobigmask);
        bdmf_session_print(session, "rcvgnttoobigmask = %u = 0x%x\n", main_int_mask.rcvgnttoobigmask, main_int_mask.rcvgnttoobigmask);
        bdmf_session_print(session, "dnstatsoverrunmask = %u = 0x%x\n", main_int_mask.dnstatsoverrunmask, main_int_mask.dnstatsoverrunmask);
        bdmf_session_print(session, "intupstatsoverrunmask = %u = 0x%x\n", main_int_mask.intupstatsoverrunmask, main_int_mask.intupstatsoverrunmask);
        bdmf_session_print(session, "dnoutofordermask = %u = 0x%x\n", main_int_mask.dnoutofordermask, main_int_mask.dnoutofordermask);
        bdmf_session_print(session, "truantbbhhaltmask = %u = 0x%x\n", main_int_mask.truantbbhhaltmask, main_int_mask.truantbbhhaltmask);
        bdmf_session_print(session, "upinvldgntlenmask = %u = 0x%x\n", main_int_mask.upinvldgntlenmask, main_int_mask.upinvldgntlenmask);
        bdmf_session_print(session, "intcobbhupsfaultmask = %u = 0x%x\n", main_int_mask.intcobbhupsfaultmask, main_int_mask.intcobbhupsfaultmask);
        bdmf_session_print(session, "dntimeinsyncmask = %u = 0x%x\n", main_int_mask.dntimeinsyncmask, main_int_mask.dntimeinsyncmask);
        bdmf_session_print(session, "dntimenotinsyncmask = %u = 0x%x\n", main_int_mask.dntimenotinsyncmask, main_int_mask.dntimenotinsyncmask);
        bdmf_session_print(session, "dportrdymask = %u = 0x%x\n", main_int_mask.dportrdymask, main_int_mask.dportrdymask);
        break;
    }
    case BDMF_max_gnt_size:
    {
        uint16_t maxgntsize;
        err = ag_drv_epn_max_gnt_size_get(&maxgntsize);
        bdmf_session_print(session, "maxgntsize = %u = 0x%x\n", maxgntsize, maxgntsize);
        break;
    }
    case BDMF_max_frame_size:
    {
        uint16_t cfgmaxframesize;
        err = ag_drv_epn_max_frame_size_get(&cfgmaxframesize);
        bdmf_session_print(session, "cfgmaxframesize = %u = 0x%x\n", cfgmaxframesize, cfgmaxframesize);
        break;
    }
    case BDMF_grant_ovr_hd:
    {
        uint16_t gntovrhdfec;
        uint16_t gntovrhd;
        err = ag_drv_epn_grant_ovr_hd_get(&gntovrhdfec, &gntovrhd);
        bdmf_session_print(session, "gntovrhdfec = %u = 0x%x\n", gntovrhdfec, gntovrhdfec);
        bdmf_session_print(session, "gntovrhd = %u = 0x%x\n", gntovrhd, gntovrhd);
        break;
    }
    case BDMF_poll_size:
    {
        uint16_t pollsizefec;
        uint16_t pollsize;
        err = ag_drv_epn_poll_size_get(&pollsizefec, &pollsize);
        bdmf_session_print(session, "pollsizefec = %u = 0x%x\n", pollsizefec, pollsizefec);
        bdmf_session_print(session, "pollsize = %u = 0x%x\n", pollsize, pollsize);
        break;
    }
    case BDMF_dn_rd_gnt_margin:
    {
        uint16_t rdgntstartmargin;
        err = ag_drv_epn_dn_rd_gnt_margin_get(&rdgntstartmargin);
        bdmf_session_print(session, "rdgntstartmargin = %u = 0x%x\n", rdgntstartmargin, rdgntstartmargin);
        break;
    }
    case BDMF_gnt_time_start_delta:
    {
        uint16_t gntstarttimedelta;
        err = ag_drv_epn_gnt_time_start_delta_get(&gntstarttimedelta);
        bdmf_session_print(session, "gntstarttimedelta = %u = 0x%x\n", gntstarttimedelta, gntstarttimedelta);
        break;
    }
    case BDMF_time_stamp_diff:
    {
        uint16_t timestampdiffdelta;
        err = ag_drv_epn_time_stamp_diff_get(&timestampdiffdelta);
        bdmf_session_print(session, "timestampdiffdelta = %u = 0x%x\n", timestampdiffdelta, timestampdiffdelta);
        break;
    }
    case BDMF_up_time_stamp_off:
    {
        uint16_t timestampoffsetfec;
        uint16_t timestampoffset;
        err = ag_drv_epn_up_time_stamp_off_get(&timestampoffsetfec, &timestampoffset);
        bdmf_session_print(session, "timestampoffsetfec = %u = 0x%x\n", timestampoffsetfec, timestampoffsetfec);
        bdmf_session_print(session, "timestampoffset = %u = 0x%x\n", timestampoffset, timestampoffset);
        break;
    }
    case BDMF_gnt_interval:
    {
        uint16_t gntinterval;
        err = ag_drv_epn_gnt_interval_get(&gntinterval);
        bdmf_session_print(session, "gntinterval = %u = 0x%x\n", gntinterval, gntinterval);
        break;
    }
    case BDMF_dn_gnt_misalign_thr:
    {
        uint16_t prvunusedgntthreshold;
        uint16_t gntmisalignthresh;
        err = ag_drv_epn_dn_gnt_misalign_thr_get(&prvunusedgntthreshold, &gntmisalignthresh);
        bdmf_session_print(session, "prvunusedgntthreshold = %u = 0x%x\n", prvunusedgntthreshold, prvunusedgntthreshold);
        bdmf_session_print(session, "gntmisalignthresh = %u = 0x%x\n", gntmisalignthresh, gntmisalignthresh);
        break;
    }
    case BDMF_dn_gnt_misalign_pause:
    {
        uint16_t gntmisalignpause;
        err = ag_drv_epn_dn_gnt_misalign_pause_get(&gntmisalignpause);
        bdmf_session_print(session, "gntmisalignpause = %u = 0x%x\n", gntmisalignpause, gntmisalignpause);
        break;
    }
    case BDMF_non_poll_intv:
    {
        uint16_t nonpollgntintv;
        err = ag_drv_epn_non_poll_intv_get(&nonpollgntintv);
        bdmf_session_print(session, "nonpollgntintv = %u = 0x%x\n", nonpollgntintv, nonpollgntintv);
        break;
    }
    case BDMF_force_fcs_err:
    {
        bdmf_boolean forceFcsErr;
        err = ag_drv_epn_force_fcs_err_get(parm[1].value.unumber, &forceFcsErr);
        bdmf_session_print(session, "forceFcsErr = %u = 0x%x\n", forceFcsErr, forceFcsErr);
        break;
    }
    case BDMF_grant_overlap_limit:
    {
        uint16_t prvgrantoverlaplimit;
        err = ag_drv_epn_grant_overlap_limit_get(&prvgrantoverlaplimit);
        bdmf_session_print(session, "prvgrantoverlaplimit = %u = 0x%x\n", prvgrantoverlaplimit, prvgrantoverlaplimit);
        break;
    }
    case BDMF_aes_configuration_0:
    {
        uint8_t prvUpstreamAesMode_0;
        err = ag_drv_epn_aes_configuration_0_get(parm[1].value.unumber, &prvUpstreamAesMode_0);
        bdmf_session_print(session, "prvUpstreamAesMode_0 = %u = 0x%x\n", prvUpstreamAesMode_0, prvUpstreamAesMode_0);
        break;
    }
    case BDMF_aes_configuration_1:
    {
        uint8_t prvUpstreamAesMode_1;
        err = ag_drv_epn_aes_configuration_1_get(parm[1].value.unumber, &prvUpstreamAesMode_1);
        bdmf_session_print(session, "prvUpstreamAesMode_1 = %u = 0x%x\n", prvUpstreamAesMode_1, prvUpstreamAesMode_1);
        break;
    }
    case BDMF_disc_grant_ovr_hd:
    {
        uint16_t discgntovrhd;
        err = ag_drv_epn_disc_grant_ovr_hd_get(&discgntovrhd);
        bdmf_session_print(session, "discgntovrhd = %u = 0x%x\n", discgntovrhd, discgntovrhd);
        break;
    }
    case BDMF_dn_discovery_seed:
    {
        uint16_t cfgdiscseed;
        err = ag_drv_epn_dn_discovery_seed_get(&cfgdiscseed);
        bdmf_session_print(session, "cfgdiscseed = %u = 0x%x\n", cfgdiscseed, cfgdiscseed);
        break;
    }
    case BDMF_dn_discovery_inc:
    {
        uint16_t cfgdiscinc;
        err = ag_drv_epn_dn_discovery_inc_get(&cfgdiscinc);
        bdmf_session_print(session, "cfgdiscinc = %u = 0x%x\n", cfgdiscinc, cfgdiscinc);
        break;
    }
    case BDMF_dn_discovery_size:
    {
        uint16_t cfgdiscsize;
        err = ag_drv_epn_dn_discovery_size_get(&cfgdiscsize);
        bdmf_session_print(session, "cfgdiscsize = %u = 0x%x\n", cfgdiscsize, cfgdiscsize);
        break;
    }
    case BDMF_prog_rpt_vec:
    {
        uint32_t progrptvec;
        err = ag_drv_epn_prog_rpt_vec_get(&progrptvec);
        bdmf_session_print(session, "progrptvec = %u = 0x%x\n", progrptvec, progrptvec);
        break;
    }
    case BDMF_fec_ipg_length:
    {
        uint8_t modipgpreamblebytes;
        uint8_t cfgrptlen;
        uint8_t cfgfecrptlength;
        uint8_t cfgfecipglength;
        err = ag_drv_epn_fec_ipg_length_get(&modipgpreamblebytes, &cfgrptlen, &cfgfecrptlength, &cfgfecipglength);
        bdmf_session_print(session, "modipgpreamblebytes = %u = 0x%x\n", modipgpreamblebytes, modipgpreamblebytes);
        bdmf_session_print(session, "cfgrptlen = %u = 0x%x\n", cfgrptlen, cfgrptlen);
        bdmf_session_print(session, "cfgfecrptlength = %u = 0x%x\n", cfgfecrptlength, cfgfecrptlength);
        bdmf_session_print(session, "cfgfecipglength = %u = 0x%x\n", cfgfecipglength, cfgfecipglength);
        break;
    }
    case BDMF_fake_report_value_en:
    {
        uint32_t fakereportvalueen;
        err = ag_drv_epn_fake_report_value_en_get(&fakereportvalueen);
        bdmf_session_print(session, "fakereportvalueen = %u = 0x%x\n", fakereportvalueen, fakereportvalueen);
        break;
    }
    case BDMF_fake_report_value:
    {
        uint32_t fakereportvalue;
        err = ag_drv_epn_fake_report_value_get(&fakereportvalue);
        bdmf_session_print(session, "fakereportvalue = %u = 0x%x\n", fakereportvalue, fakereportvalue);
        break;
    }
    case BDMF_burst_cap:
    {
        uint32_t burstcap;
        err = ag_drv_epn_burst_cap_get(parm[1].value.unumber, &burstcap);
        bdmf_session_print(session, "burstcap = %u = 0x%x\n", burstcap, burstcap);
        break;
    }
    case BDMF_queue_llid_map:
    {
        uint8_t quellidmap;
        err = ag_drv_epn_queue_llid_map_get(parm[1].value.unumber, &quellidmap);
        bdmf_session_print(session, "quellidmap = %u = 0x%x\n", quellidmap, quellidmap);
        break;
    }
    case BDMF_valid_opcode_map:
    {
        uint16_t prvvalidmpcpopcodes;
        err = ag_drv_epn_valid_opcode_map_get(&prvvalidmpcpopcodes);
        bdmf_session_print(session, "prvvalidmpcpopcodes = %u = 0x%x\n", prvvalidmpcpopcodes, prvvalidmpcpopcodes);
        break;
    }
    case BDMF_up_packet_tx_margin:
    {
        uint16_t uppackettxmargin;
        err = ag_drv_epn_up_packet_tx_margin_get(&uppackettxmargin);
        bdmf_session_print(session, "uppackettxmargin = %u = 0x%x\n", uppackettxmargin, uppackettxmargin);
        break;
    }
    case BDMF_multi_pri_cfg_0:
    {
        epn_multi_pri_cfg_0 multi_pri_cfg_0;
        err = ag_drv_epn_multi_pri_cfg_0_get(&multi_pri_cfg_0);
        bdmf_session_print(session, "cfgctcschdeficiten = %u = 0x%x\n", multi_pri_cfg_0.cfgctcschdeficiten, multi_pri_cfg_0.cfgctcschdeficiten);
        bdmf_session_print(session, "prvzeroburstcapoverridemode = %u = 0x%x\n", multi_pri_cfg_0.prvzeroburstcapoverridemode, multi_pri_cfg_0.prvzeroburstcapoverridemode);
        bdmf_session_print(session, "cfgsharedl2 = %u = 0x%x\n", multi_pri_cfg_0.cfgsharedl2, multi_pri_cfg_0.cfgsharedl2);
        bdmf_session_print(session, "cfgsharedburstcap = %u = 0x%x\n", multi_pri_cfg_0.cfgsharedburstcap, multi_pri_cfg_0.cfgsharedburstcap);
        bdmf_session_print(session, "cfgrptgntsoutst0 = %u = 0x%x\n", multi_pri_cfg_0.cfgrptgntsoutst0, multi_pri_cfg_0.cfgrptgntsoutst0);
        bdmf_session_print(session, "cfgrpthiprifirst0 = %u = 0x%x\n", multi_pri_cfg_0.cfgrpthiprifirst0, multi_pri_cfg_0.cfgrpthiprifirst0);
        bdmf_session_print(session, "cfgrptswapqs0 = %u = 0x%x\n", multi_pri_cfg_0.cfgrptswapqs0, multi_pri_cfg_0.cfgrptswapqs0);
        bdmf_session_print(session, "cfgrptmultipri0 = %u = 0x%x\n", multi_pri_cfg_0.cfgrptmultipri0, multi_pri_cfg_0.cfgrptmultipri0);
        break;
    }
    case BDMF_shared_bcap_ovrflow:
    {
        uint16_t sharedburstcapoverflow;
        err = ag_drv_epn_shared_bcap_ovrflow_get(&sharedburstcapoverflow);
        bdmf_session_print(session, "sharedburstcapoverflow = %u = 0x%x\n", sharedburstcapoverflow, sharedburstcapoverflow);
        break;
    }
    case BDMF_forced_report_en:
    {
        bdmf_boolean cfgForceReportEn;
        err = ag_drv_epn_forced_report_en_get(parm[1].value.unumber, &cfgForceReportEn);
        bdmf_session_print(session, "cfgForceReportEn = %u = 0x%x\n", cfgForceReportEn, cfgForceReportEn);
        break;
    }
    case BDMF_forced_report_max_interval:
    {
        uint8_t cfgmaxreportinterval;
        err = ag_drv_epn_forced_report_max_interval_get(&cfgmaxreportinterval);
        bdmf_session_print(session, "cfgmaxreportinterval = %u = 0x%x\n", cfgmaxreportinterval, cfgmaxreportinterval);
        break;
    }
    case BDMF_l2s_flush_config:
    {
        bdmf_boolean cfgflushl2sen;
        bdmf_boolean flushl2sdone;
        uint8_t cfgflushl2ssel;
        err = ag_drv_epn_l2s_flush_config_get(&cfgflushl2sen, &flushl2sdone, &cfgflushl2ssel);
        bdmf_session_print(session, "cfgflushl2sen = %u = 0x%x\n", cfgflushl2sen, cfgflushl2sen);
        bdmf_session_print(session, "flushl2sdone = %u = 0x%x\n", flushl2sdone, flushl2sdone);
        bdmf_session_print(session, "cfgflushl2ssel = %u = 0x%x\n", cfgflushl2ssel, cfgflushl2ssel);
        break;
    }
    case BDMF_data_port_command:
    {
        bdmf_boolean dportbusy;
        uint8_t dportselect;
        bdmf_boolean dportcontrol;
        err = ag_drv_epn_data_port_command_get(&dportbusy, &dportselect, &dportcontrol);
        bdmf_session_print(session, "dportbusy = %u = 0x%x\n", dportbusy, dportbusy);
        bdmf_session_print(session, "dportselect = %u = 0x%x\n", dportselect, dportselect);
        bdmf_session_print(session, "dportcontrol = %u = 0x%x\n", dportcontrol, dportcontrol);
        break;
    }
    case BDMF_data_port_address:
    {
        uint16_t dportaddr;
        err = ag_drv_epn_data_port_address_get(&dportaddr);
        bdmf_session_print(session, "dportaddr = %u = 0x%x\n", dportaddr, dportaddr);
        break;
    }
    case BDMF_data_port_data_0:
    {
        uint32_t dportdata0;
        err = ag_drv_epn_data_port_data_0_get(&dportdata0);
        bdmf_session_print(session, "dportdata0 = %u = 0x%x\n", dportdata0, dportdata0);
        break;
    }
    case BDMF_unmap_big_cnt:
    {
        uint32_t unmapbigerrcnt;
        err = ag_drv_epn_unmap_big_cnt_get(&unmapbigerrcnt);
        bdmf_session_print(session, "unmapbigerrcnt = %u = 0x%x\n", unmapbigerrcnt, unmapbigerrcnt);
        break;
    }
    case BDMF_unmap_frame_cnt:
    {
        uint32_t unmapfrcnt;
        err = ag_drv_epn_unmap_frame_cnt_get(&unmapfrcnt);
        bdmf_session_print(session, "unmapfrcnt = %u = 0x%x\n", unmapfrcnt, unmapfrcnt);
        break;
    }
    case BDMF_unmap_fcs_cnt:
    {
        uint32_t unmapfcserrcnt;
        err = ag_drv_epn_unmap_fcs_cnt_get(&unmapfcserrcnt);
        bdmf_session_print(session, "unmapfcserrcnt = %u = 0x%x\n", unmapfcserrcnt, unmapfcserrcnt);
        break;
    }
    case BDMF_unmap_gate_cnt:
    {
        uint32_t unmapgatecnt;
        err = ag_drv_epn_unmap_gate_cnt_get(&unmapgatecnt);
        bdmf_session_print(session, "unmapgatecnt = %u = 0x%x\n", unmapgatecnt, unmapgatecnt);
        break;
    }
    case BDMF_unmap_oam_cnt:
    {
        uint32_t unmapoamcnt;
        err = ag_drv_epn_unmap_oam_cnt_get(&unmapoamcnt);
        bdmf_session_print(session, "unmapoamcnt = %u = 0x%x\n", unmapoamcnt, unmapoamcnt);
        break;
    }
    case BDMF_unmap_small_cnt:
    {
        uint32_t unmapsmallerrcnt;
        err = ag_drv_epn_unmap_small_cnt_get(&unmapsmallerrcnt);
        bdmf_session_print(session, "unmapsmallerrcnt = %u = 0x%x\n", unmapsmallerrcnt, unmapsmallerrcnt);
        break;
    }
    case BDMF_fif_dequeue_event_cnt:
    {
        uint32_t fifdequeueeventcnt;
        err = ag_drv_epn_fif_dequeue_event_cnt_get(&fifdequeueeventcnt);
        bdmf_session_print(session, "fifdequeueeventcnt = %u = 0x%x\n", fifdequeueeventcnt, fifdequeueeventcnt);
        break;
    }
    case BDMF_unused_tq_cnt:
    {
        uint32_t unusedtqcnt;
        err = ag_drv_epn_unused_tq_cnt_get(parm[1].value.unumber, &unusedtqcnt);
        bdmf_session_print(session, "unusedtqcnt = %u = 0x%x\n", unusedtqcnt, unusedtqcnt);
        break;
    }
    case BDMF_bbh_up_fault_halt_en:
    {
        bdmf_boolean bbhUpsFaultHaltEn;
        err = ag_drv_epn_bbh_up_fault_halt_en_get(parm[1].value.unumber, &bbhUpsFaultHaltEn);
        bdmf_session_print(session, "bbhUpsFaultHaltEn = %u = 0x%x\n", bbhUpsFaultHaltEn, bbhUpsFaultHaltEn);
        break;
    }
    case BDMF_bbh_up_tardy_halt_en:
    {
        bdmf_boolean fataltardybbhaborten;
        err = ag_drv_epn_bbh_up_tardy_halt_en_get(&fataltardybbhaborten);
        bdmf_session_print(session, "fataltardybbhaborten = %u = 0x%x\n", fataltardybbhaborten, fataltardybbhaborten);
        break;
    }
    case BDMF_debug_status_0:
    {
        uint8_t l2squefulldebug;
        bdmf_boolean dndlufull;
        bdmf_boolean dnsecfull;
        bdmf_boolean epnliffifofull;
        err = ag_drv_epn_debug_status_0_get(&l2squefulldebug, &dndlufull, &dnsecfull, &epnliffifofull);
        bdmf_session_print(session, "l2squefulldebug = %u = 0x%x\n", l2squefulldebug, l2squefulldebug);
        bdmf_session_print(session, "dndlufull = %u = 0x%x\n", dndlufull, dndlufull);
        bdmf_session_print(session, "dnsecfull = %u = 0x%x\n", dnsecfull, dnsecfull);
        bdmf_session_print(session, "epnliffifofull = %u = 0x%x\n", epnliffifofull, epnliffifofull);
        break;
    }
    case BDMF_debug_status_1:
    {
        bdmf_boolean gntRdy;
        err = ag_drv_epn_debug_status_1_get(parm[1].value.unumber, &gntRdy);
        bdmf_session_print(session, "gntRdy = %u = 0x%x\n", gntRdy, gntRdy);
        break;
    }
    case BDMF_debug_l2s_ptr_sel:
    {
        uint8_t cfgl2sdebugptrsel;
        uint16_t l2sdebugptrstate;
        err = ag_drv_epn_debug_l2s_ptr_sel_get(&cfgl2sdebugptrsel, &l2sdebugptrstate);
        bdmf_session_print(session, "cfgl2sdebugptrsel = %u = 0x%x\n", cfgl2sdebugptrsel, cfgl2sdebugptrsel);
        bdmf_session_print(session, "l2sdebugptrstate = %u = 0x%x\n", l2sdebugptrstate, l2sdebugptrstate);
        break;
    }
    case BDMF_olt_mac_addr_lo:
    {
        uint32_t oltaddrlo;
        err = ag_drv_epn_olt_mac_addr_lo_get(&oltaddrlo);
        bdmf_session_print(session, "oltaddrlo = %u = 0x%x\n", oltaddrlo, oltaddrlo);
        break;
    }
    case BDMF_olt_mac_addr_hi:
    {
        uint16_t oltaddrhi;
        err = ag_drv_epn_olt_mac_addr_hi_get(&oltaddrhi);
        bdmf_session_print(session, "oltaddrhi = %u = 0x%x\n", oltaddrhi, oltaddrhi);
        break;
    }
    case BDMF_tx_l1s_shp_dqu_empty:
    {
        bdmf_boolean l1sDquQueEmpty;
        err = ag_drv_epn_tx_l1s_shp_dqu_empty_get(parm[1].value.unumber, &l1sDquQueEmpty);
        bdmf_session_print(session, "l1sDquQueEmpty = %u = 0x%x\n", l1sDquQueEmpty, l1sDquQueEmpty);
        break;
    }
    case BDMF_tx_l1s_unshaped_empty:
    {
        bdmf_boolean l1sUnshapedQueEmpty;
        err = ag_drv_epn_tx_l1s_unshaped_empty_get(parm[1].value.unumber, &l1sUnshapedQueEmpty);
        bdmf_session_print(session, "l1sUnshapedQueEmpty = %u = 0x%x\n", l1sUnshapedQueEmpty, l1sUnshapedQueEmpty);
        break;
    }
    case BDMF_tx_l1s_shp_que_mask:
    {
        uint32_t cfgshpmask;
        err = ag_drv_epn_tx_l1s_shp_que_mask_get(parm[1].value.unumber, &cfgshpmask);
        bdmf_session_print(session, "cfgshpmask = %u = 0x%x\n", cfgshpmask, cfgshpmask);
        break;
    }
    case BDMF_tx_l2s_queue_config:
    {
        uint16_t cfgl2squeend;
        uint16_t cfgl2squestart;
        err = ag_drv_epn_tx_l2s_queue_config_get(parm[1].value.unumber, &cfgl2squeend, &cfgl2squestart);
        bdmf_session_print(session, "cfgl2squeend = %u = 0x%x\n", cfgl2squeend, cfgl2squeend);
        bdmf_session_print(session, "cfgl2squestart = %u = 0x%x\n", cfgl2squestart, cfgl2squestart);
        break;
    }
    case BDMF_tx_l2s_que_empty:
    {
        bdmf_boolean l2sQueEmpty;
        err = ag_drv_epn_tx_l2s_que_empty_get(parm[1].value.unumber, &l2sQueEmpty);
        bdmf_session_print(session, "l2sQueEmpty = %u = 0x%x\n", l2sQueEmpty, l2sQueEmpty);
        break;
    }
    case BDMF_tx_l2s_que_full:
    {
        bdmf_boolean l2sQueFull;
        err = ag_drv_epn_tx_l2s_que_full_get(parm[1].value.unumber, &l2sQueFull);
        bdmf_session_print(session, "l2sQueFull = %u = 0x%x\n", l2sQueFull, l2sQueFull);
        break;
    }
    case BDMF_tx_l2s_que_stopped:
    {
        bdmf_boolean l2sStoppedQueues;
        err = ag_drv_epn_tx_l2s_que_stopped_get(parm[1].value.unumber, &l2sStoppedQueues);
        bdmf_session_print(session, "l2sStoppedQueues = %u = 0x%x\n", l2sStoppedQueues, l2sStoppedQueues);
        break;
    }
    case BDMF_tx_ctc_burst_limit:
    {
        uint32_t prvburstlimit;
        err = ag_drv_epn_tx_ctc_burst_limit_get(parm[1].value.unumber, &prvburstlimit);
        bdmf_session_print(session, "prvburstlimit = %u = 0x%x\n", prvburstlimit, prvburstlimit);
        break;
    }
    case BDMF_bbh_max_outstanding_tardy_packets:
    {
        uint16_t cfgmaxoutstandingtardypackets;
        err = ag_drv_epn_bbh_max_outstanding_tardy_packets_get(&cfgmaxoutstandingtardypackets);
        bdmf_session_print(session, "cfgmaxoutstandingtardypackets = %u = 0x%x\n", cfgmaxoutstandingtardypackets, cfgmaxoutstandingtardypackets);
        break;
    }
    case BDMF_min_report_value_difference:
    {
        uint16_t prvminreportdiff;
        err = ag_drv_epn_min_report_value_difference_get(&prvminreportdiff);
        bdmf_session_print(session, "prvminreportdiff = %u = 0x%x\n", prvminreportdiff, prvminreportdiff);
        break;
    }
    case BDMF_bbh_status_fifo_overflow:
    {
        bdmf_boolean utxBbhStatusFifoOverflow;
        err = ag_drv_epn_bbh_status_fifo_overflow_get(parm[1].value.unumber, &utxBbhStatusFifoOverflow);
        bdmf_session_print(session, "utxBbhStatusFifoOverflow = %u = 0x%x\n", utxBbhStatusFifoOverflow, utxBbhStatusFifoOverflow);
        break;
    }
    case BDMF_spare_ctl:
    {
        uint32_t cfgepnspare;
        err = ag_drv_epn_spare_ctl_get(&cfgepnspare);
        bdmf_session_print(session, "cfgepnspare = %u = 0x%x\n", cfgepnspare, cfgepnspare);
        break;
    }
    case BDMF_ts_sync_offset:
    {
        uint8_t cfgtssyncoffset;
        err = ag_drv_epn_ts_sync_offset_get(&cfgtssyncoffset);
        bdmf_session_print(session, "cfgtssyncoffset = %u = 0x%x\n", cfgtssyncoffset, cfgtssyncoffset);
        break;
    }
    case BDMF_dn_ts_offset:
    {
        uint32_t cfgdntsoffset;
        err = ag_drv_epn_dn_ts_offset_get(&cfgdntsoffset);
        bdmf_session_print(session, "cfgdntsoffset = %u = 0x%x\n", cfgdntsoffset, cfgdntsoffset);
        break;
    }
    case BDMF_up_ts_offset_lo:
    {
        uint32_t cfguptsoffset_lo;
        err = ag_drv_epn_up_ts_offset_lo_get(&cfguptsoffset_lo);
        bdmf_session_print(session, "cfguptsoffset_lo = %u = 0x%x\n", cfguptsoffset_lo, cfguptsoffset_lo);
        break;
    }
    case BDMF_up_ts_offset_hi:
    {
        uint16_t cfguptsoffset_hi;
        err = ag_drv_epn_up_ts_offset_hi_get(&cfguptsoffset_hi);
        bdmf_session_print(session, "cfguptsoffset_hi = %u = 0x%x\n", cfguptsoffset_hi, cfguptsoffset_hi);
        break;
    }
    case BDMF_two_step_ts_ctl:
    {
        bdmf_boolean twostepffrd;
        uint8_t twostepffentries;
        err = ag_drv_epn_two_step_ts_ctl_get(&twostepffrd, &twostepffentries);
        bdmf_session_print(session, "twostepffrd = %u = 0x%x\n", twostepffrd, twostepffrd);
        bdmf_session_print(session, "twostepffentries = %u = 0x%x\n", twostepffentries, twostepffentries);
        break;
    }
    case BDMF_two_step_ts_value_lo:
    {
        uint32_t twosteptimestamp_lo;
        err = ag_drv_epn_two_step_ts_value_lo_get(&twosteptimestamp_lo);
        bdmf_session_print(session, "twosteptimestamp_lo = %u = 0x%x\n", twosteptimestamp_lo, twosteptimestamp_lo);
        break;
    }
    case BDMF_two_step_ts_value_hi:
    {
        uint16_t twosteptimestamp_hi;
        err = ag_drv_epn_two_step_ts_value_hi_get(&twosteptimestamp_hi);
        bdmf_session_print(session, "twosteptimestamp_hi = %u = 0x%x\n", twosteptimestamp_hi, twosteptimestamp_hi);
        break;
    }
    case BDMF_1588_timestamp_int_status:
    {
        bdmf_boolean int1588pktabort;
        bdmf_boolean int1588twostepffint;
        err = ag_drv_epn_1588_timestamp_int_status_get(&int1588pktabort, &int1588twostepffint);
        bdmf_session_print(session, "int1588pktabort = %u = 0x%x\n", int1588pktabort, int1588pktabort);
        bdmf_session_print(session, "int1588twostepffint = %u = 0x%x\n", int1588twostepffint, int1588twostepffint);
        break;
    }
    case BDMF_1588_timestamp_int_mask:
    {
        bdmf_boolean ts1588pktabort_mask;
        bdmf_boolean ts1588twostepff_mask;
        err = ag_drv_epn_1588_timestamp_int_mask_get(&ts1588pktabort_mask, &ts1588twostepff_mask);
        bdmf_session_print(session, "ts1588pktabort_mask = %u = 0x%x\n", ts1588pktabort_mask, ts1588pktabort_mask);
        bdmf_session_print(session, "ts1588twostepff_mask = %u = 0x%x\n", ts1588twostepff_mask, ts1588twostepff_mask);
        break;
    }
    case BDMF_up_packet_fetch_margin:
    {
        uint16_t uppacketfetchmargin;
        err = ag_drv_epn_up_packet_fetch_margin_get(&uppacketfetchmargin);
        bdmf_session_print(session, "uppacketfetchmargin = %u = 0x%x\n", uppacketfetchmargin, uppacketfetchmargin);
        break;
    }
    case BDMF_dn_1588_timestamp:
    {
        uint32_t dn_1588_ts;
        err = ag_drv_epn_dn_1588_timestamp_get(&dn_1588_ts);
        bdmf_session_print(session, "dn_1588_ts = %u = 0x%x\n", dn_1588_ts, dn_1588_ts);
        break;
    }
    case BDMF_persistent_report_cfg:
    {
        uint16_t cfgpersrptduration;
        uint16_t cfgpersrptticksize;
        err = ag_drv_epn_persistent_report_cfg_get(&cfgpersrptduration, &cfgpersrptticksize);
        bdmf_session_print(session, "cfgpersrptduration = %u = 0x%x\n", cfgpersrptduration, cfgpersrptduration);
        bdmf_session_print(session, "cfgpersrptticksize = %u = 0x%x\n", cfgpersrptticksize, cfgpersrptticksize);
        break;
    }
    case BDMF_persistent_report_enables:
    {
        bdmf_boolean cfgPersRptEnable;
        err = ag_drv_epn_persistent_report_enables_get(parm[1].value.unumber, &cfgPersRptEnable);
        bdmf_session_print(session, "cfgPersRptEnable = %u = 0x%x\n", cfgPersRptEnable, cfgPersRptEnable);
        break;
    }
    case BDMF_persistent_report_request_size:
    {
        uint16_t cfgpersrptreqtq;
        err = ag_drv_epn_persistent_report_request_size_get(&cfgpersrptreqtq);
        bdmf_session_print(session, "cfgpersrptreqtq = %u = 0x%x\n", cfgpersrptreqtq, cfgpersrptreqtq);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_epn_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        epn_control_0 control_0 = {.cfgen1588ts=gtmv(m, 1), .cfgreplaceupfcs=gtmv(m, 1), .cfgappendupfcs=gtmv(m, 1), .cfgdropscb=gtmv(m, 1), .moduncappedreportlimit=gtmv(m, 1), .modmpquesetfirst=gtmv(m, 1), .prvlocalmpcppropagation=gtmv(m, 1), .prvtekmodeprefetch=gtmv(m, 1), .prvincnonzeroaccum=gtmv(m, 1), .prvnounmapppedfcs=gtmv(m, 1), .prvsupressdiscen=gtmv(m, 1), .cfgvlanmax=gtmv(m, 1), .fcserronlydatafr=gtmv(m, 1), .prvdropunmapppedllid=gtmv(m, 1), .prvsuppressllidmodebit=gtmv(m, 1), .moddiscoverydafilteren=gtmv(m, 1), .rptselect=gtmv(m, 2), .prvdisablesvaquestatusbp=gtmv(m, 1), .utxloopback=gtmv(m, 1), .utxen=gtmv(m, 1), .utxrst_pre_n=gtmv(m, 1), .cfgdisabledns=gtmv(m, 1), .drxloopback=gtmv(m, 1), .drxen=gtmv(m, 1), .drxrst_pre_n=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_epn_control_0_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", control_0.cfgen1588ts, control_0.cfgreplaceupfcs, control_0.cfgappendupfcs, control_0.cfgdropscb, control_0.moduncappedreportlimit, control_0.modmpquesetfirst, control_0.prvlocalmpcppropagation, control_0.prvtekmodeprefetch, control_0.prvincnonzeroaccum, control_0.prvnounmapppedfcs, control_0.prvsupressdiscen, control_0.cfgvlanmax, control_0.fcserronlydatafr, control_0.prvdropunmapppedllid, control_0.prvsuppressllidmodebit, control_0.moddiscoverydafilteren, control_0.rptselect, control_0.prvdisablesvaquestatusbp, control_0.utxloopback, control_0.utxen, control_0.utxrst_pre_n, control_0.cfgdisabledns, control_0.drxloopback, control_0.drxen, control_0.drxrst_pre_n);
        if(!err) ag_drv_epn_control_0_set(&control_0);
        if(!err) ag_drv_epn_control_0_get( &control_0);
        if(!err) bdmf_session_print(session, "ag_drv_epn_control_0_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", control_0.cfgen1588ts, control_0.cfgreplaceupfcs, control_0.cfgappendupfcs, control_0.cfgdropscb, control_0.moduncappedreportlimit, control_0.modmpquesetfirst, control_0.prvlocalmpcppropagation, control_0.prvtekmodeprefetch, control_0.prvincnonzeroaccum, control_0.prvnounmapppedfcs, control_0.prvsupressdiscen, control_0.cfgvlanmax, control_0.fcserronlydatafr, control_0.prvdropunmapppedllid, control_0.prvsuppressllidmodebit, control_0.moddiscoverydafilteren, control_0.rptselect, control_0.prvdisablesvaquestatusbp, control_0.utxloopback, control_0.utxen, control_0.utxrst_pre_n, control_0.cfgdisabledns, control_0.drxloopback, control_0.drxen, control_0.drxrst_pre_n);
        if(err || control_0.cfgen1588ts!=gtmv(m, 1) || control_0.cfgreplaceupfcs!=gtmv(m, 1) || control_0.cfgappendupfcs!=gtmv(m, 1) || control_0.cfgdropscb!=gtmv(m, 1) || control_0.moduncappedreportlimit!=gtmv(m, 1) || control_0.modmpquesetfirst!=gtmv(m, 1) || control_0.prvlocalmpcppropagation!=gtmv(m, 1) || control_0.prvtekmodeprefetch!=gtmv(m, 1) || control_0.prvincnonzeroaccum!=gtmv(m, 1) || control_0.prvnounmapppedfcs!=gtmv(m, 1) || control_0.prvsupressdiscen!=gtmv(m, 1) || control_0.cfgvlanmax!=gtmv(m, 1) || control_0.fcserronlydatafr!=gtmv(m, 1) || control_0.prvdropunmapppedllid!=gtmv(m, 1) || control_0.prvsuppressllidmodebit!=gtmv(m, 1) || control_0.moddiscoverydafilteren!=gtmv(m, 1) || control_0.rptselect!=gtmv(m, 2) || control_0.prvdisablesvaquestatusbp!=gtmv(m, 1) || control_0.utxloopback!=gtmv(m, 1) || control_0.utxen!=gtmv(m, 1) || control_0.utxrst_pre_n!=gtmv(m, 1) || control_0.cfgdisabledns!=gtmv(m, 1) || control_0.drxloopback!=gtmv(m, 1) || control_0.drxen!=gtmv(m, 1) || control_0.drxrst_pre_n!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        epn_control_1 control_1 = {.prvoverlappedgntenable=gtmv(m, 1), .rstmisalignthr=gtmv(m, 1), .cfgtekrpt=gtmv(m, 1), .cfgstalegntchk=gtmv(m, 1), .fecrpten=gtmv(m, 1), .cfgl1l2truestrict=gtmv(m, 1), .cfgctcrpt=gtmv(m, 2), .cfgtscorrdis=gtmv(m, 1), .cfgnodiscrpt=gtmv(m, 1), .disablediscscale=gtmv(m, 1), .clronrd=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_epn_control_1_set( %u %u %u %u %u %u %u %u %u %u %u)\n", control_1.prvoverlappedgntenable, control_1.rstmisalignthr, control_1.cfgtekrpt, control_1.cfgstalegntchk, control_1.fecrpten, control_1.cfgl1l2truestrict, control_1.cfgctcrpt, control_1.cfgtscorrdis, control_1.cfgnodiscrpt, control_1.disablediscscale, control_1.clronrd);
        if(!err) ag_drv_epn_control_1_set(&control_1);
        if(!err) ag_drv_epn_control_1_get( &control_1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_control_1_get( %u %u %u %u %u %u %u %u %u %u %u)\n", control_1.prvoverlappedgntenable, control_1.rstmisalignthr, control_1.cfgtekrpt, control_1.cfgstalegntchk, control_1.fecrpten, control_1.cfgl1l2truestrict, control_1.cfgctcrpt, control_1.cfgtscorrdis, control_1.cfgnodiscrpt, control_1.disablediscscale, control_1.clronrd);
        if(err || control_1.prvoverlappedgntenable!=gtmv(m, 1) || control_1.rstmisalignthr!=gtmv(m, 1) || control_1.cfgtekrpt!=gtmv(m, 1) || control_1.cfgstalegntchk!=gtmv(m, 1) || control_1.fecrpten!=gtmv(m, 1) || control_1.cfgl1l2truestrict!=gtmv(m, 1) || control_1.cfgctcrpt!=gtmv(m, 2) || control_1.cfgtscorrdis!=gtmv(m, 1) || control_1.cfgnodiscrpt!=gtmv(m, 1) || control_1.disablediscscale!=gtmv(m, 1) || control_1.clronrd!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean grant_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_enable_grants_set( %u %u)\n", link_idx, grant_en);
        if(!err) ag_drv_epn_enable_grants_set(link_idx, grant_en);
        if(!err) ag_drv_epn_enable_grants_get( link_idx, &grant_en);
        if(!err) bdmf_session_print(session, "ag_drv_epn_enable_grants_get( %u %u)\n", link_idx, grant_en);
        if(err || grant_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean linkDiscGates_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_drop_disc_gates_set( %u %u)\n", link_idx, linkDiscGates_en);
        if(!err) ag_drv_epn_drop_disc_gates_set(link_idx, linkDiscGates_en);
        if(!err) ag_drv_epn_drop_disc_gates_get( link_idx, &linkDiscGates_en);
        if(!err) bdmf_session_print(session, "ag_drv_epn_drop_disc_gates_get( %u %u)\n", link_idx, linkDiscGates_en);
        if(err || linkDiscGates_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean disableFcsChk=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dis_fcs_chk_set( %u %u)\n", link_idx, disableFcsChk);
        if(!err) ag_drv_epn_dis_fcs_chk_set(link_idx, disableFcsChk);
        if(!err) ag_drv_epn_dis_fcs_chk_get( link_idx, &disableFcsChk);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dis_fcs_chk_get( %u %u)\n", link_idx, disableFcsChk);
        if(err || disableFcsChk!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean passGates=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_pass_gates_set( %u %u)\n", link_idx, passGates);
        if(!err) ag_drv_epn_pass_gates_set(link_idx, passGates);
        if(!err) ag_drv_epn_pass_gates_get( link_idx, &passGates);
        if(!err) bdmf_session_print(session, "ag_drv_epn_pass_gates_get( %u %u)\n", link_idx, passGates);
        if(err || passGates!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean cfgMisalignFeedback=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_cfg_misalgn_fb_set( %u %u)\n", link_idx, cfgMisalignFeedback);
        if(!err) ag_drv_epn_cfg_misalgn_fb_set(link_idx, cfgMisalignFeedback);
        if(!err) ag_drv_epn_cfg_misalgn_fb_get( link_idx, &cfgMisalignFeedback);
        if(!err) bdmf_session_print(session, "ag_drv_epn_cfg_misalgn_fb_get( %u %u)\n", link_idx, cfgMisalignFeedback);
        if(err || cfgMisalignFeedback!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t prvdiscinfomask=gtmv(m, 16);
        uint16_t prvdiscinfovalue=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_discovery_filter_set( %u %u)\n", prvdiscinfomask, prvdiscinfovalue);
        if(!err) ag_drv_epn_discovery_filter_set(prvdiscinfomask, prvdiscinfovalue);
        if(!err) ag_drv_epn_discovery_filter_get( &prvdiscinfomask, &prvdiscinfovalue);
        if(!err) bdmf_session_print(session, "ag_drv_epn_discovery_filter_get( %u %u)\n", prvdiscinfomask, prvdiscinfovalue);
        if(err || prvdiscinfomask!=gtmv(m, 16) || prvdiscinfovalue!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t prvbbhquestatdelay=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_que_status_delay_set( %u)\n", prvbbhquestatdelay);
        if(!err) ag_drv_epn_bbh_que_status_delay_set(prvbbhquestatdelay);
        if(!err) ag_drv_epn_bbh_que_status_delay_get( &prvbbhquestatdelay);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_que_status_delay_get( %u)\n", prvbbhquestatdelay);
        if(err || prvbbhquestatdelay!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgmingrantsetup=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_minimum_grant_setup_set( %u)\n", cfgmingrantsetup);
        if(!err) ag_drv_epn_minimum_grant_setup_set(cfgmingrantsetup);
        if(!err) ag_drv_epn_minimum_grant_setup_get( &cfgmingrantsetup);
        if(!err) bdmf_session_print(session, "ag_drv_epn_minimum_grant_setup_get( %u)\n", cfgmingrantsetup);
        if(err || cfgmingrantsetup!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean rstGntFifo=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_reset_gnt_fifo_set( %u %u)\n", link_idx, rstGntFifo);
        if(!err) ag_drv_epn_reset_gnt_fifo_set(link_idx, rstGntFifo);
        if(!err) ag_drv_epn_reset_gnt_fifo_get( link_idx, &rstGntFifo);
        if(!err) bdmf_session_print(session, "ag_drv_epn_reset_gnt_fifo_get( %u %u)\n", link_idx, rstGntFifo);
        if(err || rstGntFifo!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgl1sclracum=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_reset_l1_accumulator_set( %u)\n", cfgl1sclracum);
        if(!err) ag_drv_epn_reset_l1_accumulator_set(cfgl1sclracum);
        if(!err) ag_drv_epn_reset_l1_accumulator_get( &cfgl1sclracum);
        if(!err) bdmf_session_print(session, "ag_drv_epn_reset_l1_accumulator_get( %u)\n", cfgl1sclracum);
        if(err || cfgl1sclracum!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cfgl1suvasizesel=gtmv(m, 5);
        uint8_t cfgl1ssvasizesel=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_epn_l1_accumulator_sel_set( %u %u)\n", cfgl1suvasizesel, cfgl1ssvasizesel);
        if(!err) ag_drv_epn_l1_accumulator_sel_set(cfgl1suvasizesel, cfgl1ssvasizesel);
        if(!err) ag_drv_epn_l1_accumulator_sel_get( &cfgl1suvasizesel, &cfgl1ssvasizesel);
        if(!err) bdmf_session_print(session, "ag_drv_epn_l1_accumulator_sel_get( %u %u)\n", cfgl1suvasizesel, cfgl1ssvasizesel);
        if(err || cfgl1suvasizesel!=gtmv(m, 5) || cfgl1ssvasizesel!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t l1ssvasize=gtmv(m, 30);
        if(!err) ag_drv_epn_l1_sva_bytes_get( &l1ssvasize);
        if(!err) bdmf_session_print(session, "ag_drv_epn_l1_sva_bytes_get( %u)\n", l1ssvasize);
    }
    {
        uint32_t l1suvasize=gtmv(m, 30);
        if(!err) ag_drv_epn_l1_uva_bytes_get( &l1suvasize);
        if(!err) bdmf_session_print(session, "ag_drv_epn_l1_uva_bytes_get( %u)\n", l1suvasize);
    }
    {
        uint32_t l1ssvaoverflow=gtmv(m, 32);
        if(!err) ag_drv_epn_l1_sva_overflow_get( &l1ssvaoverflow);
        if(!err) bdmf_session_print(session, "ag_drv_epn_l1_sva_overflow_get( %u)\n", l1ssvaoverflow);
    }
    {
        uint32_t l1suvaoverflow=gtmv(m, 32);
        if(!err) ag_drv_epn_l1_uva_overflow_get( &l1suvaoverflow);
        if(!err) bdmf_session_print(session, "ag_drv_epn_l1_uva_overflow_get( %u)\n", l1suvaoverflow);
    }
    {
        epn_reset_rpt_pri reset_rpt_pri = {.nullrptpri15=gtmv(m, 1), .nullrptpri14=gtmv(m, 1), .nullrptpri13=gtmv(m, 1), .nullrptpri12=gtmv(m, 1), .nullrptpri11=gtmv(m, 1), .nullrptpri10=gtmv(m, 1), .nullrptpri9=gtmv(m, 1), .nullrptpri8=gtmv(m, 1), .nullrptpri7=gtmv(m, 1), .nullrptpri6=gtmv(m, 1), .nullrptpri5=gtmv(m, 1), .nullrptpri4=gtmv(m, 1), .nullrptpri3=gtmv(m, 1), .nullrptpri2=gtmv(m, 1), .nullrptpri1=gtmv(m, 1), .nullrptpri0=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_epn_reset_rpt_pri_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", reset_rpt_pri.nullrptpri15, reset_rpt_pri.nullrptpri14, reset_rpt_pri.nullrptpri13, reset_rpt_pri.nullrptpri12, reset_rpt_pri.nullrptpri11, reset_rpt_pri.nullrptpri10, reset_rpt_pri.nullrptpri9, reset_rpt_pri.nullrptpri8, reset_rpt_pri.nullrptpri7, reset_rpt_pri.nullrptpri6, reset_rpt_pri.nullrptpri5, reset_rpt_pri.nullrptpri4, reset_rpt_pri.nullrptpri3, reset_rpt_pri.nullrptpri2, reset_rpt_pri.nullrptpri1, reset_rpt_pri.nullrptpri0);
        if(!err) ag_drv_epn_reset_rpt_pri_set(&reset_rpt_pri);
        if(!err) ag_drv_epn_reset_rpt_pri_get( &reset_rpt_pri);
        if(!err) bdmf_session_print(session, "ag_drv_epn_reset_rpt_pri_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", reset_rpt_pri.nullrptpri15, reset_rpt_pri.nullrptpri14, reset_rpt_pri.nullrptpri13, reset_rpt_pri.nullrptpri12, reset_rpt_pri.nullrptpri11, reset_rpt_pri.nullrptpri10, reset_rpt_pri.nullrptpri9, reset_rpt_pri.nullrptpri8, reset_rpt_pri.nullrptpri7, reset_rpt_pri.nullrptpri6, reset_rpt_pri.nullrptpri5, reset_rpt_pri.nullrptpri4, reset_rpt_pri.nullrptpri3, reset_rpt_pri.nullrptpri2, reset_rpt_pri.nullrptpri1, reset_rpt_pri.nullrptpri0);
        if(err || reset_rpt_pri.nullrptpri15!=gtmv(m, 1) || reset_rpt_pri.nullrptpri14!=gtmv(m, 1) || reset_rpt_pri.nullrptpri13!=gtmv(m, 1) || reset_rpt_pri.nullrptpri12!=gtmv(m, 1) || reset_rpt_pri.nullrptpri11!=gtmv(m, 1) || reset_rpt_pri.nullrptpri10!=gtmv(m, 1) || reset_rpt_pri.nullrptpri9!=gtmv(m, 1) || reset_rpt_pri.nullrptpri8!=gtmv(m, 1) || reset_rpt_pri.nullrptpri7!=gtmv(m, 1) || reset_rpt_pri.nullrptpri6!=gtmv(m, 1) || reset_rpt_pri.nullrptpri5!=gtmv(m, 1) || reset_rpt_pri.nullrptpri4!=gtmv(m, 1) || reset_rpt_pri.nullrptpri3!=gtmv(m, 1) || reset_rpt_pri.nullrptpri2!=gtmv(m, 1) || reset_rpt_pri.nullrptpri1!=gtmv(m, 1) || reset_rpt_pri.nullrptpri0!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean cfgL2SClrQue=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_reset_l2_rpt_fifo_set( %u %u)\n", link_idx, cfgL2SClrQue);
        if(!err) ag_drv_epn_reset_l2_rpt_fifo_set(link_idx, cfgL2SClrQue);
        if(!err) ag_drv_epn_reset_l2_rpt_fifo_get( link_idx, &cfgL2SClrQue);
        if(!err) bdmf_session_print(session, "ag_drv_epn_reset_l2_rpt_fifo_get( %u %u)\n", link_idx, cfgL2SClrQue);
        if(err || cfgL2SClrQue!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean cfgEnableUpstream=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_enable_upstream_set( %u %u)\n", link_idx, cfgEnableUpstream);
        if(!err) ag_drv_epn_enable_upstream_set(link_idx, cfgEnableUpstream);
        if(!err) ag_drv_epn_enable_upstream_get( link_idx, &cfgEnableUpstream);
        if(!err) bdmf_session_print(session, "ag_drv_epn_enable_upstream_get( %u %u)\n", link_idx, cfgEnableUpstream);
        if(err || cfgEnableUpstream!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean cfgEnableUpstreamFeedBack=gtmv(m, 1);
        if(!err) ag_drv_epn_enable_upstream_fb_get( link_idx, &cfgEnableUpstreamFeedBack);
        if(!err) bdmf_session_print(session, "ag_drv_epn_enable_upstream_fb_get( %u %u)\n", link_idx, cfgEnableUpstreamFeedBack);
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean upstreamFecEn=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_enable_upstream_fec_set( %u %u)\n", link_idx, upstreamFecEn);
        if(!err) ag_drv_epn_enable_upstream_fec_set(link_idx, upstreamFecEn);
        if(!err) ag_drv_epn_enable_upstream_fec_get( link_idx, &upstreamFecEn);
        if(!err) bdmf_session_print(session, "ag_drv_epn_enable_upstream_fec_get( %u %u)\n", link_idx, upstreamFecEn);
        if(err || upstreamFecEn!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t prvrptbytelen=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_epn_report_byte_length_set( %u)\n", prvrptbytelen);
        if(!err) ag_drv_epn_report_byte_length_set(prvrptbytelen);
        if(!err) ag_drv_epn_report_byte_length_get( &prvrptbytelen);
        if(!err) bdmf_session_print(session, "ag_drv_epn_report_byte_length_get( %u)\n", prvrptbytelen);
        if(err || prvrptbytelen!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        epn_main_int_status main_int_status = {.intbbhupfrabort=gtmv(m, 1), .intcol2sburstcapoverflowpres=gtmv(m, 1), .intcoemptyrpt=gtmv(m, 1), .intcodrxerrabortpres=gtmv(m, 1), .intl2sfifooverrun=gtmv(m, 1), .intco1588tsint=gtmv(m, 1), .intcorptpres=gtmv(m, 1), .intcogntpres=gtmv(m, 1), .intcodelstalegnt=gtmv(m, 1), .intcogntnonpoll=gtmv(m, 1), .intcogntmisalign=gtmv(m, 1), .intcognttoofar=gtmv(m, 1), .intcogntinterval=gtmv(m, 1), .intcogntdiscovery=gtmv(m, 1), .intcogntmissabort=gtmv(m, 1), .intcogntfullabort=gtmv(m, 1), .intbadupfrlen=gtmv(m, 1), .intuptardypacket=gtmv(m, 1), .intuprptfrxmt=gtmv(m, 1), .intbififooverrun=gtmv(m, 1), .intburstgnttoobig=gtmv(m, 1), .intwrgnttoobig=gtmv(m, 1), .intrcvgnttoobig=gtmv(m, 1), .intdnstatsoverrun=gtmv(m, 1), .intupstatsoverrun=gtmv(m, 1), .intdnoutoforder=gtmv(m, 1), .inttruantbbhhalt=gtmv(m, 1), .intupinvldgntlen=gtmv(m, 1), .intcobbhupsfault=gtmv(m, 1), .intdntimeinsync=gtmv(m, 1), .intdntimenotinsync=gtmv(m, 1), .intdportrdy=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_epn_main_int_status_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", main_int_status.intbbhupfrabort, main_int_status.intcol2sburstcapoverflowpres, main_int_status.intcoemptyrpt, main_int_status.intcodrxerrabortpres, main_int_status.intl2sfifooverrun, main_int_status.intco1588tsint, main_int_status.intcorptpres, main_int_status.intcogntpres, main_int_status.intcodelstalegnt, main_int_status.intcogntnonpoll, main_int_status.intcogntmisalign, main_int_status.intcognttoofar, main_int_status.intcogntinterval, main_int_status.intcogntdiscovery, main_int_status.intcogntmissabort, main_int_status.intcogntfullabort, main_int_status.intbadupfrlen, main_int_status.intuptardypacket, main_int_status.intuprptfrxmt, main_int_status.intbififooverrun, main_int_status.intburstgnttoobig, main_int_status.intwrgnttoobig, main_int_status.intrcvgnttoobig, main_int_status.intdnstatsoverrun, main_int_status.intupstatsoverrun, main_int_status.intdnoutoforder, main_int_status.inttruantbbhhalt, main_int_status.intupinvldgntlen, main_int_status.intcobbhupsfault, main_int_status.intdntimeinsync, main_int_status.intdntimenotinsync, main_int_status.intdportrdy);
        if(!err) ag_drv_epn_main_int_status_set(&main_int_status);
        if(!err) ag_drv_epn_main_int_status_get( &main_int_status);
        if(!err) bdmf_session_print(session, "ag_drv_epn_main_int_status_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", main_int_status.intbbhupfrabort, main_int_status.intcol2sburstcapoverflowpres, main_int_status.intcoemptyrpt, main_int_status.intcodrxerrabortpres, main_int_status.intl2sfifooverrun, main_int_status.intco1588tsint, main_int_status.intcorptpres, main_int_status.intcogntpres, main_int_status.intcodelstalegnt, main_int_status.intcogntnonpoll, main_int_status.intcogntmisalign, main_int_status.intcognttoofar, main_int_status.intcogntinterval, main_int_status.intcogntdiscovery, main_int_status.intcogntmissabort, main_int_status.intcogntfullabort, main_int_status.intbadupfrlen, main_int_status.intuptardypacket, main_int_status.intuprptfrxmt, main_int_status.intbififooverrun, main_int_status.intburstgnttoobig, main_int_status.intwrgnttoobig, main_int_status.intrcvgnttoobig, main_int_status.intdnstatsoverrun, main_int_status.intupstatsoverrun, main_int_status.intdnoutoforder, main_int_status.inttruantbbhhalt, main_int_status.intupinvldgntlen, main_int_status.intcobbhupsfault, main_int_status.intdntimeinsync, main_int_status.intdntimenotinsync, main_int_status.intdportrdy);
        if(err || main_int_status.intbbhupfrabort!=gtmv(m, 1) || main_int_status.intcol2sburstcapoverflowpres!=gtmv(m, 1) || main_int_status.intcoemptyrpt!=gtmv(m, 1) || main_int_status.intcodrxerrabortpres!=gtmv(m, 1) || main_int_status.intl2sfifooverrun!=gtmv(m, 1) || main_int_status.intco1588tsint!=gtmv(m, 1) || main_int_status.intcorptpres!=gtmv(m, 1) || main_int_status.intcogntpres!=gtmv(m, 1) || main_int_status.intcodelstalegnt!=gtmv(m, 1) || main_int_status.intcogntnonpoll!=gtmv(m, 1) || main_int_status.intcogntmisalign!=gtmv(m, 1) || main_int_status.intcognttoofar!=gtmv(m, 1) || main_int_status.intcogntinterval!=gtmv(m, 1) || main_int_status.intcogntdiscovery!=gtmv(m, 1) || main_int_status.intcogntmissabort!=gtmv(m, 1) || main_int_status.intcogntfullabort!=gtmv(m, 1) || main_int_status.intbadupfrlen!=gtmv(m, 1) || main_int_status.intuptardypacket!=gtmv(m, 1) || main_int_status.intuprptfrxmt!=gtmv(m, 1) || main_int_status.intbififooverrun!=gtmv(m, 1) || main_int_status.intburstgnttoobig!=gtmv(m, 1) || main_int_status.intwrgnttoobig!=gtmv(m, 1) || main_int_status.intrcvgnttoobig!=gtmv(m, 1) || main_int_status.intdnstatsoverrun!=gtmv(m, 1) || main_int_status.intupstatsoverrun!=gtmv(m, 1) || main_int_status.intdnoutoforder!=gtmv(m, 1) || main_int_status.inttruantbbhhalt!=gtmv(m, 1) || main_int_status.intupinvldgntlen!=gtmv(m, 1) || main_int_status.intcobbhupsfault!=gtmv(m, 1) || main_int_status.intdntimeinsync!=gtmv(m, 1) || main_int_status.intdntimenotinsync!=gtmv(m, 1) || main_int_status.intdportrdy!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intDnGntFullAbort=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_full_int_status_set( %u %u)\n", link_idx, intDnGntFullAbort);
        if(!err) ag_drv_epn_gnt_full_int_status_set(link_idx, intDnGntFullAbort);
        if(!err) ag_drv_epn_gnt_full_int_status_get( link_idx, &intDnGntFullAbort);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_full_int_status_get( %u %u)\n", link_idx, intDnGntFullAbort);
        if(err || intDnGntFullAbort!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskIntDnGntFullAbort=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_full_int_mask_set( %u %u)\n", link_idx, maskIntDnGntFullAbort);
        if(!err) ag_drv_epn_gnt_full_int_mask_set(link_idx, maskIntDnGntFullAbort);
        if(!err) ag_drv_epn_gnt_full_int_mask_get( link_idx, &maskIntDnGntFullAbort);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_full_int_mask_get( %u %u)\n", link_idx, maskIntDnGntFullAbort);
        if(err || maskIntDnGntFullAbort!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intDnGntMissAbort=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_miss_int_status_set( %u %u)\n", link_idx, intDnGntMissAbort);
        if(!err) ag_drv_epn_gnt_miss_int_status_set(link_idx, intDnGntMissAbort);
        if(!err) ag_drv_epn_gnt_miss_int_status_get( link_idx, &intDnGntMissAbort);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_miss_int_status_get( %u %u)\n", link_idx, intDnGntMissAbort);
        if(err || intDnGntMissAbort!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskIntDnGntMissAbort=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_miss_int_mask_set( %u %u)\n", link_idx, maskIntDnGntMissAbort);
        if(!err) ag_drv_epn_gnt_miss_int_mask_set(link_idx, maskIntDnGntMissAbort);
        if(!err) ag_drv_epn_gnt_miss_int_mask_get( link_idx, &maskIntDnGntMissAbort);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_miss_int_mask_get( %u %u)\n", link_idx, maskIntDnGntMissAbort);
        if(err || maskIntDnGntMissAbort!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intDnGntDiscovery=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_disc_rx_int_status_set( %u %u)\n", link_idx, intDnGntDiscovery);
        if(!err) ag_drv_epn_disc_rx_int_status_set(link_idx, intDnGntDiscovery);
        if(!err) ag_drv_epn_disc_rx_int_status_get( link_idx, &intDnGntDiscovery);
        if(!err) bdmf_session_print(session, "ag_drv_epn_disc_rx_int_status_get( %u %u)\n", link_idx, intDnGntDiscovery);
        if(err || intDnGntDiscovery!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskIntDnGntDiscovery=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_disc_rx_int_mask_set( %u %u)\n", link_idx, maskIntDnGntDiscovery);
        if(!err) ag_drv_epn_disc_rx_int_mask_set(link_idx, maskIntDnGntDiscovery);
        if(!err) ag_drv_epn_disc_rx_int_mask_get( link_idx, &maskIntDnGntDiscovery);
        if(!err) bdmf_session_print(session, "ag_drv_epn_disc_rx_int_mask_get( %u %u)\n", link_idx, maskIntDnGntDiscovery);
        if(err || maskIntDnGntDiscovery!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intDnGntInterval=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_intv_int_status_set( %u %u)\n", link_idx, intDnGntInterval);
        if(!err) ag_drv_epn_gnt_intv_int_status_set(link_idx, intDnGntInterval);
        if(!err) ag_drv_epn_gnt_intv_int_status_get( link_idx, &intDnGntInterval);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_intv_int_status_get( %u %u)\n", link_idx, intDnGntInterval);
        if(err || intDnGntInterval!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskIntDnGntInterval=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_intv_int_mask_set( %u %u)\n", link_idx, maskIntDnGntInterval);
        if(!err) ag_drv_epn_gnt_intv_int_mask_set(link_idx, maskIntDnGntInterval);
        if(!err) ag_drv_epn_gnt_intv_int_mask_get( link_idx, &maskIntDnGntInterval);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_intv_int_mask_get( %u %u)\n", link_idx, maskIntDnGntInterval);
        if(err || maskIntDnGntInterval!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intDnGntTooFar=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_far_int_status_set( %u %u)\n", link_idx, intDnGntTooFar);
        if(!err) ag_drv_epn_gnt_far_int_status_set(link_idx, intDnGntTooFar);
        if(!err) ag_drv_epn_gnt_far_int_status_get( link_idx, &intDnGntTooFar);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_far_int_status_get( %u %u)\n", link_idx, intDnGntTooFar);
        if(err || intDnGntTooFar!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskDnGntTooFar=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_far_int_mask_set( %u %u)\n", link_idx, maskDnGntTooFar);
        if(!err) ag_drv_epn_gnt_far_int_mask_set(link_idx, maskDnGntTooFar);
        if(!err) ag_drv_epn_gnt_far_int_mask_get( link_idx, &maskDnGntTooFar);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_far_int_mask_get( %u %u)\n", link_idx, maskDnGntTooFar);
        if(err || maskDnGntTooFar!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intDnGntMisalign=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_misalgn_int_status_set( %u %u)\n", link_idx, intDnGntMisalign);
        if(!err) ag_drv_epn_gnt_misalgn_int_status_set(link_idx, intDnGntMisalign);
        if(!err) ag_drv_epn_gnt_misalgn_int_status_get( link_idx, &intDnGntMisalign);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_misalgn_int_status_get( %u %u)\n", link_idx, intDnGntMisalign);
        if(err || intDnGntMisalign!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskIntDnGntMisalign=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_misalgn_int_mask_set( %u %u)\n", link_idx, maskIntDnGntMisalign);
        if(!err) ag_drv_epn_gnt_misalgn_int_mask_set(link_idx, maskIntDnGntMisalign);
        if(!err) ag_drv_epn_gnt_misalgn_int_mask_get( link_idx, &maskIntDnGntMisalign);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_misalgn_int_mask_get( %u %u)\n", link_idx, maskIntDnGntMisalign);
        if(err || maskIntDnGntMisalign!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intDnGntNonPoll=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_np_gnt_int_status_set( %u %u)\n", link_idx, intDnGntNonPoll);
        if(!err) ag_drv_epn_np_gnt_int_status_set(link_idx, intDnGntNonPoll);
        if(!err) ag_drv_epn_np_gnt_int_status_get( link_idx, &intDnGntNonPoll);
        if(!err) bdmf_session_print(session, "ag_drv_epn_np_gnt_int_status_get( %u %u)\n", link_idx, intDnGntNonPoll);
        if(err || intDnGntNonPoll!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskDnGntNonPoll=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_np_gnt_int_mask_set( %u %u)\n", link_idx, maskDnGntNonPoll);
        if(!err) ag_drv_epn_np_gnt_int_mask_set(link_idx, maskDnGntNonPoll);
        if(!err) ag_drv_epn_np_gnt_int_mask_get( link_idx, &maskDnGntNonPoll);
        if(!err) bdmf_session_print(session, "ag_drv_epn_np_gnt_int_mask_get( %u %u)\n", link_idx, maskDnGntNonPoll);
        if(err || maskDnGntNonPoll!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intDelStaleGnt=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_del_stale_int_status_set( %u %u)\n", link_idx, intDelStaleGnt);
        if(!err) ag_drv_epn_del_stale_int_status_set(link_idx, intDelStaleGnt);
        if(!err) ag_drv_epn_del_stale_int_status_get( link_idx, &intDelStaleGnt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_del_stale_int_status_get( %u %u)\n", link_idx, intDelStaleGnt);
        if(err || intDelStaleGnt!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskIntDelStaleGnt=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_del_stale_int_mask_set( %u %u)\n", link_idx, maskIntDelStaleGnt);
        if(!err) ag_drv_epn_del_stale_int_mask_set(link_idx, maskIntDelStaleGnt);
        if(!err) ag_drv_epn_del_stale_int_mask_get( link_idx, &maskIntDelStaleGnt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_del_stale_int_mask_get( %u %u)\n", link_idx, maskIntDelStaleGnt);
        if(err || maskIntDelStaleGnt!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intDnGntRdy=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_pres_int_status_set( %u %u)\n", link_idx, intDnGntRdy);
        if(!err) ag_drv_epn_gnt_pres_int_status_set(link_idx, intDnGntRdy);
        if(!err) ag_drv_epn_gnt_pres_int_status_get( link_idx, &intDnGntRdy);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_pres_int_status_get( %u %u)\n", link_idx, intDnGntRdy);
        if(err || intDnGntRdy!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskDnGntRdy=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_pres_int_mask_set( %u %u)\n", link_idx, maskDnGntRdy);
        if(!err) ag_drv_epn_gnt_pres_int_mask_set(link_idx, maskDnGntRdy);
        if(!err) ag_drv_epn_gnt_pres_int_mask_get( link_idx, &maskDnGntRdy);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_pres_int_mask_get( %u %u)\n", link_idx, maskDnGntRdy);
        if(err || maskDnGntRdy!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intUpRptFifo=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_rpt_pres_int_status_set( %u %u)\n", link_idx, intUpRptFifo);
        if(!err) ag_drv_epn_rpt_pres_int_status_set(link_idx, intUpRptFifo);
        if(!err) ag_drv_epn_rpt_pres_int_status_get( link_idx, &intUpRptFifo);
        if(!err) bdmf_session_print(session, "ag_drv_epn_rpt_pres_int_status_get( %u %u)\n", link_idx, intUpRptFifo);
        if(err || intUpRptFifo!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskIntUpRptFifo=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_rpt_pres_int_mask_set( %u %u)\n", link_idx, maskIntUpRptFifo);
        if(!err) ag_drv_epn_rpt_pres_int_mask_set(link_idx, maskIntUpRptFifo);
        if(!err) ag_drv_epn_rpt_pres_int_mask_get( link_idx, &maskIntUpRptFifo);
        if(!err) bdmf_session_print(session, "ag_drv_epn_rpt_pres_int_mask_get( %u %u)\n", link_idx, maskIntUpRptFifo);
        if(err || maskIntUpRptFifo!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intDrxErrAbort=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_drx_abort_int_status_set( %u %u)\n", link_idx, intDrxErrAbort);
        if(!err) ag_drv_epn_drx_abort_int_status_set(link_idx, intDrxErrAbort);
        if(!err) ag_drv_epn_drx_abort_int_status_get( link_idx, &intDrxErrAbort);
        if(!err) bdmf_session_print(session, "ag_drv_epn_drx_abort_int_status_get( %u %u)\n", link_idx, intDrxErrAbort);
        if(err || intDrxErrAbort!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskIntDrxErrAbort=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_drx_abort_int_mask_set( %u %u)\n", link_idx, maskIntDrxErrAbort);
        if(!err) ag_drv_epn_drx_abort_int_mask_set(link_idx, maskIntDrxErrAbort);
        if(!err) ag_drv_epn_drx_abort_int_mask_get( link_idx, &maskIntDrxErrAbort);
        if(!err) bdmf_session_print(session, "ag_drv_epn_drx_abort_int_mask_get( %u %u)\n", link_idx, maskIntDrxErrAbort);
        if(err || maskIntDrxErrAbort!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intEmptyRpt=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_empty_rpt_int_status_set( %u %u)\n", link_idx, intEmptyRpt);
        if(!err) ag_drv_epn_empty_rpt_int_status_set(link_idx, intEmptyRpt);
        if(!err) ag_drv_epn_empty_rpt_int_status_get( link_idx, &intEmptyRpt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_empty_rpt_int_status_get( %u %u)\n", link_idx, intEmptyRpt);
        if(err || intEmptyRpt!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean  maskIntEmptyRpt=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_empty_rpt_int_mask_set( %u %u)\n", link_idx,  maskIntEmptyRpt);
        if(!err) ag_drv_epn_empty_rpt_int_mask_set(link_idx,  maskIntEmptyRpt);
        if(!err) ag_drv_epn_empty_rpt_int_mask_get( link_idx, & maskIntEmptyRpt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_empty_rpt_int_mask_get( %u %u)\n", link_idx,  maskIntEmptyRpt);
        if(err ||  maskIntEmptyRpt!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t l2_acc_idx=gtmv(m, 5);
        bdmf_boolean intl2sBurstCapOverFlow=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bcap_overflow_int_status_set( %u %u)\n", l2_acc_idx, intl2sBurstCapOverFlow);
        if(!err) ag_drv_epn_bcap_overflow_int_status_set(l2_acc_idx, intl2sBurstCapOverFlow);
        if(!err) ag_drv_epn_bcap_overflow_int_status_get( l2_acc_idx, &intl2sBurstCapOverFlow);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bcap_overflow_int_status_get( %u %u)\n", l2_acc_idx, intl2sBurstCapOverFlow);
        if(err || intl2sBurstCapOverFlow!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskIntl2sBurstCapOverFlow=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bcap_overflow_int_mask_set( %u %u)\n", link_idx, maskIntl2sBurstCapOverFlow);
        if(!err) ag_drv_epn_bcap_overflow_int_mask_set(link_idx, maskIntl2sBurstCapOverFlow);
        if(!err) ag_drv_epn_bcap_overflow_int_mask_get( link_idx, &maskIntl2sBurstCapOverFlow);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bcap_overflow_int_mask_get( %u %u)\n", link_idx, maskIntl2sBurstCapOverFlow);
        if(err || maskIntl2sBurstCapOverFlow!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean intbbhdnsoverflow=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_dns_fault_int_status_set( %u)\n", intbbhdnsoverflow);
        if(!err) ag_drv_epn_bbh_dns_fault_int_status_set(intbbhdnsoverflow);
        if(!err) ag_drv_epn_bbh_dns_fault_int_status_get( &intbbhdnsoverflow);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_dns_fault_int_status_get( %u)\n", intbbhdnsoverflow);
        if(err || intbbhdnsoverflow!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean maskintbbhdnsoverflow=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_dns_fault_int_mask_set( %u)\n", maskintbbhdnsoverflow);
        if(!err) ag_drv_epn_bbh_dns_fault_int_mask_set(maskintbbhdnsoverflow);
        if(!err) ag_drv_epn_bbh_dns_fault_int_mask_get( &maskintbbhdnsoverflow);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_dns_fault_int_mask_get( %u)\n", maskintbbhdnsoverflow);
        if(err || maskintbbhdnsoverflow!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean intBbhUpsFault=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_ups_fault_int_status_set( %u %u)\n", link_idx, intBbhUpsFault);
        if(!err) ag_drv_epn_bbh_ups_fault_int_status_set(link_idx, intBbhUpsFault);
        if(!err) ag_drv_epn_bbh_ups_fault_int_status_get( link_idx, &intBbhUpsFault);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_ups_fault_int_status_get( %u %u)\n", link_idx, intBbhUpsFault);
        if(err || intBbhUpsFault!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean maskIntBbhUpsFault=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_ups_fault_int_mask_set( %u %u)\n", link_idx, maskIntBbhUpsFault);
        if(!err) ag_drv_epn_bbh_ups_fault_int_mask_set(link_idx, maskIntBbhUpsFault);
        if(!err) ag_drv_epn_bbh_ups_fault_int_mask_get( link_idx, &maskIntBbhUpsFault);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_ups_fault_int_mask_get( %u %u)\n", link_idx, maskIntBbhUpsFault);
        if(err || maskIntBbhUpsFault!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean tardybbhabort=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_ups_abort_int_status_set( %u)\n", tardybbhabort);
        if(!err) ag_drv_epn_bbh_ups_abort_int_status_set(tardybbhabort);
        if(!err) ag_drv_epn_bbh_ups_abort_int_status_get( &tardybbhabort);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_ups_abort_int_status_get( %u)\n", tardybbhabort);
        if(err || tardybbhabort!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean masktardybbhabort=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_ups_abort_int_mask_set( %u)\n", masktardybbhabort);
        if(!err) ag_drv_epn_bbh_ups_abort_int_mask_set(masktardybbhabort);
        if(!err) ag_drv_epn_bbh_ups_abort_int_mask_get( &masktardybbhabort);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_ups_abort_int_mask_get( %u)\n", masktardybbhabort);
        if(err || masktardybbhabort!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        epn_main_int_mask main_int_mask = {.bbhupfrabortmask=gtmv(m, 1), .intl2sburstcapoverflowmask=gtmv(m, 1), .intcoemptyrptmask=gtmv(m, 1), .intdrxerrabortmask=gtmv(m, 1), .intl2sfifooverrunmask=gtmv(m, 1), .intco1588tsmask=gtmv(m, 1), .intcorptpresmask=gtmv(m, 1), .intcogntpresmask=gtmv(m, 1), .intcodelstalegntmask=gtmv(m, 1), .intcogntnonpollmask=gtmv(m, 1), .intcogntmisalignmask=gtmv(m, 1), .intcognttoofarmask=gtmv(m, 1), .intcogntintervalmask=gtmv(m, 1), .intcogntdiscoverymask=gtmv(m, 1), .intcogntmissabortmask=gtmv(m, 1), .intcogntfullabortmask=gtmv(m, 1), .badupfrlenmask=gtmv(m, 1), .uptardypacketmask=gtmv(m, 1), .uprptfrxmtmask=gtmv(m, 1), .intbififooverrunmask=gtmv(m, 1), .burstgnttoobigmask=gtmv(m, 1), .wrgnttoobigmask=gtmv(m, 1), .rcvgnttoobigmask=gtmv(m, 1), .dnstatsoverrunmask=gtmv(m, 1), .intupstatsoverrunmask=gtmv(m, 1), .dnoutofordermask=gtmv(m, 1), .truantbbhhaltmask=gtmv(m, 1), .upinvldgntlenmask=gtmv(m, 1), .intcobbhupsfaultmask=gtmv(m, 1), .dntimeinsyncmask=gtmv(m, 1), .dntimenotinsyncmask=gtmv(m, 1), .dportrdymask=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_epn_main_int_mask_set( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", main_int_mask.bbhupfrabortmask, main_int_mask.intl2sburstcapoverflowmask, main_int_mask.intcoemptyrptmask, main_int_mask.intdrxerrabortmask, main_int_mask.intl2sfifooverrunmask, main_int_mask.intco1588tsmask, main_int_mask.intcorptpresmask, main_int_mask.intcogntpresmask, main_int_mask.intcodelstalegntmask, main_int_mask.intcogntnonpollmask, main_int_mask.intcogntmisalignmask, main_int_mask.intcognttoofarmask, main_int_mask.intcogntintervalmask, main_int_mask.intcogntdiscoverymask, main_int_mask.intcogntmissabortmask, main_int_mask.intcogntfullabortmask, main_int_mask.badupfrlenmask, main_int_mask.uptardypacketmask, main_int_mask.uprptfrxmtmask, main_int_mask.intbififooverrunmask, main_int_mask.burstgnttoobigmask, main_int_mask.wrgnttoobigmask, main_int_mask.rcvgnttoobigmask, main_int_mask.dnstatsoverrunmask, main_int_mask.intupstatsoverrunmask, main_int_mask.dnoutofordermask, main_int_mask.truantbbhhaltmask, main_int_mask.upinvldgntlenmask, main_int_mask.intcobbhupsfaultmask, main_int_mask.dntimeinsyncmask, main_int_mask.dntimenotinsyncmask, main_int_mask.dportrdymask);
        if(!err) ag_drv_epn_main_int_mask_set(&main_int_mask);
        if(!err) ag_drv_epn_main_int_mask_get( &main_int_mask);
        if(!err) bdmf_session_print(session, "ag_drv_epn_main_int_mask_get( %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", main_int_mask.bbhupfrabortmask, main_int_mask.intl2sburstcapoverflowmask, main_int_mask.intcoemptyrptmask, main_int_mask.intdrxerrabortmask, main_int_mask.intl2sfifooverrunmask, main_int_mask.intco1588tsmask, main_int_mask.intcorptpresmask, main_int_mask.intcogntpresmask, main_int_mask.intcodelstalegntmask, main_int_mask.intcogntnonpollmask, main_int_mask.intcogntmisalignmask, main_int_mask.intcognttoofarmask, main_int_mask.intcogntintervalmask, main_int_mask.intcogntdiscoverymask, main_int_mask.intcogntmissabortmask, main_int_mask.intcogntfullabortmask, main_int_mask.badupfrlenmask, main_int_mask.uptardypacketmask, main_int_mask.uprptfrxmtmask, main_int_mask.intbififooverrunmask, main_int_mask.burstgnttoobigmask, main_int_mask.wrgnttoobigmask, main_int_mask.rcvgnttoobigmask, main_int_mask.dnstatsoverrunmask, main_int_mask.intupstatsoverrunmask, main_int_mask.dnoutofordermask, main_int_mask.truantbbhhaltmask, main_int_mask.upinvldgntlenmask, main_int_mask.intcobbhupsfaultmask, main_int_mask.dntimeinsyncmask, main_int_mask.dntimenotinsyncmask, main_int_mask.dportrdymask);
        if(err || main_int_mask.bbhupfrabortmask!=gtmv(m, 1) || main_int_mask.intl2sburstcapoverflowmask!=gtmv(m, 1) || main_int_mask.intcoemptyrptmask!=gtmv(m, 1) || main_int_mask.intdrxerrabortmask!=gtmv(m, 1) || main_int_mask.intl2sfifooverrunmask!=gtmv(m, 1) || main_int_mask.intco1588tsmask!=gtmv(m, 1) || main_int_mask.intcorptpresmask!=gtmv(m, 1) || main_int_mask.intcogntpresmask!=gtmv(m, 1) || main_int_mask.intcodelstalegntmask!=gtmv(m, 1) || main_int_mask.intcogntnonpollmask!=gtmv(m, 1) || main_int_mask.intcogntmisalignmask!=gtmv(m, 1) || main_int_mask.intcognttoofarmask!=gtmv(m, 1) || main_int_mask.intcogntintervalmask!=gtmv(m, 1) || main_int_mask.intcogntdiscoverymask!=gtmv(m, 1) || main_int_mask.intcogntmissabortmask!=gtmv(m, 1) || main_int_mask.intcogntfullabortmask!=gtmv(m, 1) || main_int_mask.badupfrlenmask!=gtmv(m, 1) || main_int_mask.uptardypacketmask!=gtmv(m, 1) || main_int_mask.uprptfrxmtmask!=gtmv(m, 1) || main_int_mask.intbififooverrunmask!=gtmv(m, 1) || main_int_mask.burstgnttoobigmask!=gtmv(m, 1) || main_int_mask.wrgnttoobigmask!=gtmv(m, 1) || main_int_mask.rcvgnttoobigmask!=gtmv(m, 1) || main_int_mask.dnstatsoverrunmask!=gtmv(m, 1) || main_int_mask.intupstatsoverrunmask!=gtmv(m, 1) || main_int_mask.dnoutofordermask!=gtmv(m, 1) || main_int_mask.truantbbhhaltmask!=gtmv(m, 1) || main_int_mask.upinvldgntlenmask!=gtmv(m, 1) || main_int_mask.intcobbhupsfaultmask!=gtmv(m, 1) || main_int_mask.dntimeinsyncmask!=gtmv(m, 1) || main_int_mask.dntimenotinsyncmask!=gtmv(m, 1) || main_int_mask.dportrdymask!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t maxgntsize=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_max_gnt_size_set( %u)\n", maxgntsize);
        if(!err) ag_drv_epn_max_gnt_size_set(maxgntsize);
        if(!err) ag_drv_epn_max_gnt_size_get( &maxgntsize);
        if(!err) bdmf_session_print(session, "ag_drv_epn_max_gnt_size_get( %u)\n", maxgntsize);
        if(err || maxgntsize!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgmaxframesize=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_epn_max_frame_size_set( %u)\n", cfgmaxframesize);
        if(!err) ag_drv_epn_max_frame_size_set(cfgmaxframesize);
        if(!err) ag_drv_epn_max_frame_size_get( &cfgmaxframesize);
        if(!err) bdmf_session_print(session, "ag_drv_epn_max_frame_size_get( %u)\n", cfgmaxframesize);
        if(err || cfgmaxframesize!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t gntovrhdfec=gtmv(m, 16);
        uint16_t gntovrhd=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_grant_ovr_hd_set( %u %u)\n", gntovrhdfec, gntovrhd);
        if(!err) ag_drv_epn_grant_ovr_hd_set(gntovrhdfec, gntovrhd);
        if(!err) ag_drv_epn_grant_ovr_hd_get( &gntovrhdfec, &gntovrhd);
        if(!err) bdmf_session_print(session, "ag_drv_epn_grant_ovr_hd_get( %u %u)\n", gntovrhdfec, gntovrhd);
        if(err || gntovrhdfec!=gtmv(m, 16) || gntovrhd!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t pollsizefec=gtmv(m, 16);
        uint16_t pollsize=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_poll_size_set( %u %u)\n", pollsizefec, pollsize);
        if(!err) ag_drv_epn_poll_size_set(pollsizefec, pollsize);
        if(!err) ag_drv_epn_poll_size_get( &pollsizefec, &pollsize);
        if(!err) bdmf_session_print(session, "ag_drv_epn_poll_size_get( %u %u)\n", pollsizefec, pollsize);
        if(err || pollsizefec!=gtmv(m, 16) || pollsize!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t rdgntstartmargin=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_rd_gnt_margin_set( %u)\n", rdgntstartmargin);
        if(!err) ag_drv_epn_dn_rd_gnt_margin_set(rdgntstartmargin);
        if(!err) ag_drv_epn_dn_rd_gnt_margin_get( &rdgntstartmargin);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_rd_gnt_margin_get( %u)\n", rdgntstartmargin);
        if(err || rdgntstartmargin!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t gntstarttimedelta=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_time_start_delta_set( %u)\n", gntstarttimedelta);
        if(!err) ag_drv_epn_gnt_time_start_delta_set(gntstarttimedelta);
        if(!err) ag_drv_epn_gnt_time_start_delta_get( &gntstarttimedelta);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_time_start_delta_get( %u)\n", gntstarttimedelta);
        if(err || gntstarttimedelta!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t timestampdiffdelta=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_time_stamp_diff_set( %u)\n", timestampdiffdelta);
        if(!err) ag_drv_epn_time_stamp_diff_set(timestampdiffdelta);
        if(!err) ag_drv_epn_time_stamp_diff_get( &timestampdiffdelta);
        if(!err) bdmf_session_print(session, "ag_drv_epn_time_stamp_diff_get( %u)\n", timestampdiffdelta);
        if(err || timestampdiffdelta!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t timestampoffsetfec=gtmv(m, 16);
        uint16_t timestampoffset=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_up_time_stamp_off_set( %u %u)\n", timestampoffsetfec, timestampoffset);
        if(!err) ag_drv_epn_up_time_stamp_off_set(timestampoffsetfec, timestampoffset);
        if(!err) ag_drv_epn_up_time_stamp_off_get( &timestampoffsetfec, &timestampoffset);
        if(!err) bdmf_session_print(session, "ag_drv_epn_up_time_stamp_off_get( %u %u)\n", timestampoffsetfec, timestampoffset);
        if(err || timestampoffsetfec!=gtmv(m, 16) || timestampoffset!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t gntinterval=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_interval_set( %u)\n", gntinterval);
        if(!err) ag_drv_epn_gnt_interval_set(gntinterval);
        if(!err) ag_drv_epn_gnt_interval_get( &gntinterval);
        if(!err) bdmf_session_print(session, "ag_drv_epn_gnt_interval_get( %u)\n", gntinterval);
        if(err || gntinterval!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t prvunusedgntthreshold=gtmv(m, 16);
        uint16_t gntmisalignthresh=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_gnt_misalign_thr_set( %u %u)\n", prvunusedgntthreshold, gntmisalignthresh);
        if(!err) ag_drv_epn_dn_gnt_misalign_thr_set(prvunusedgntthreshold, gntmisalignthresh);
        if(!err) ag_drv_epn_dn_gnt_misalign_thr_get( &prvunusedgntthreshold, &gntmisalignthresh);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_gnt_misalign_thr_get( %u %u)\n", prvunusedgntthreshold, gntmisalignthresh);
        if(err || prvunusedgntthreshold!=gtmv(m, 16) || gntmisalignthresh!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t gntmisalignpause=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_gnt_misalign_pause_set( %u)\n", gntmisalignpause);
        if(!err) ag_drv_epn_dn_gnt_misalign_pause_set(gntmisalignpause);
        if(!err) ag_drv_epn_dn_gnt_misalign_pause_get( &gntmisalignpause);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_gnt_misalign_pause_get( %u)\n", gntmisalignpause);
        if(err || gntmisalignpause!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t nonpollgntintv=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_non_poll_intv_set( %u)\n", nonpollgntintv);
        if(!err) ag_drv_epn_non_poll_intv_set(nonpollgntintv);
        if(!err) ag_drv_epn_non_poll_intv_get( &nonpollgntintv);
        if(!err) bdmf_session_print(session, "ag_drv_epn_non_poll_intv_get( %u)\n", nonpollgntintv);
        if(err || nonpollgntintv!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean forceFcsErr=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_force_fcs_err_set( %u %u)\n", link_idx, forceFcsErr);
        if(!err) ag_drv_epn_force_fcs_err_set(link_idx, forceFcsErr);
        if(!err) ag_drv_epn_force_fcs_err_get( link_idx, &forceFcsErr);
        if(!err) bdmf_session_print(session, "ag_drv_epn_force_fcs_err_get( %u %u)\n", link_idx, forceFcsErr);
        if(err || forceFcsErr!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t prvgrantoverlaplimit=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_grant_overlap_limit_set( %u)\n", prvgrantoverlaplimit);
        if(!err) ag_drv_epn_grant_overlap_limit_set(prvgrantoverlaplimit);
        if(!err) ag_drv_epn_grant_overlap_limit_get( &prvgrantoverlaplimit);
        if(!err) bdmf_session_print(session, "ag_drv_epn_grant_overlap_limit_get( %u)\n", prvgrantoverlaplimit);
        if(err || prvgrantoverlaplimit!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 4);
        uint8_t prvUpstreamAesMode_0=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_epn_aes_configuration_0_set( %u %u)\n", link_idx, prvUpstreamAesMode_0);
        if(!err) ag_drv_epn_aes_configuration_0_set(link_idx, prvUpstreamAesMode_0);
        if(!err) ag_drv_epn_aes_configuration_0_get( link_idx, &prvUpstreamAesMode_0);
        if(!err) bdmf_session_print(session, "ag_drv_epn_aes_configuration_0_get( %u %u)\n", link_idx, prvUpstreamAesMode_0);
        if(err || prvUpstreamAesMode_0!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 4);
        uint8_t prvUpstreamAesMode_1=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_epn_aes_configuration_1_set( %u %u)\n", link_idx, prvUpstreamAesMode_1);
        if(!err) ag_drv_epn_aes_configuration_1_set(link_idx, prvUpstreamAesMode_1);
        if(!err) ag_drv_epn_aes_configuration_1_get( link_idx, &prvUpstreamAesMode_1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_aes_configuration_1_get( %u %u)\n", link_idx, prvUpstreamAesMode_1);
        if(err || prvUpstreamAesMode_1!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t discgntovrhd=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_disc_grant_ovr_hd_set( %u)\n", discgntovrhd);
        if(!err) ag_drv_epn_disc_grant_ovr_hd_set(discgntovrhd);
        if(!err) ag_drv_epn_disc_grant_ovr_hd_get( &discgntovrhd);
        if(!err) bdmf_session_print(session, "ag_drv_epn_disc_grant_ovr_hd_get( %u)\n", discgntovrhd);
        if(err || discgntovrhd!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgdiscseed=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_discovery_seed_set( %u)\n", cfgdiscseed);
        if(!err) ag_drv_epn_dn_discovery_seed_set(cfgdiscseed);
        if(!err) ag_drv_epn_dn_discovery_seed_get( &cfgdiscseed);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_discovery_seed_get( %u)\n", cfgdiscseed);
        if(err || cfgdiscseed!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgdiscinc=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_discovery_inc_set( %u)\n", cfgdiscinc);
        if(!err) ag_drv_epn_dn_discovery_inc_set(cfgdiscinc);
        if(!err) ag_drv_epn_dn_discovery_inc_get( &cfgdiscinc);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_discovery_inc_get( %u)\n", cfgdiscinc);
        if(err || cfgdiscinc!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgdiscsize=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_discovery_size_set( %u)\n", cfgdiscsize);
        if(!err) ag_drv_epn_dn_discovery_size_set(cfgdiscsize);
        if(!err) ag_drv_epn_dn_discovery_size_get( &cfgdiscsize);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_discovery_size_get( %u)\n", cfgdiscsize);
        if(err || cfgdiscsize!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t progrptvec=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_prog_rpt_vec_set( %u)\n", progrptvec);
        if(!err) ag_drv_epn_prog_rpt_vec_set(progrptvec);
        if(!err) ag_drv_epn_prog_rpt_vec_get( &progrptvec);
        if(!err) bdmf_session_print(session, "ag_drv_epn_prog_rpt_vec_get( %u)\n", progrptvec);
        if(err || progrptvec!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t modipgpreamblebytes=gtmv(m, 5);
        uint8_t cfgrptlen=gtmv(m, 8);
        uint8_t cfgfecrptlength=gtmv(m, 8);
        uint8_t cfgfecipglength=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_epn_fec_ipg_length_set( %u %u %u %u)\n", modipgpreamblebytes, cfgrptlen, cfgfecrptlength, cfgfecipglength);
        if(!err) ag_drv_epn_fec_ipg_length_set(modipgpreamblebytes, cfgrptlen, cfgfecrptlength, cfgfecipglength);
        if(!err) ag_drv_epn_fec_ipg_length_get( &modipgpreamblebytes, &cfgrptlen, &cfgfecrptlength, &cfgfecipglength);
        if(!err) bdmf_session_print(session, "ag_drv_epn_fec_ipg_length_get( %u %u %u %u)\n", modipgpreamblebytes, cfgrptlen, cfgfecrptlength, cfgfecipglength);
        if(err || modipgpreamblebytes!=gtmv(m, 5) || cfgrptlen!=gtmv(m, 8) || cfgfecrptlength!=gtmv(m, 8) || cfgfecipglength!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t fakereportvalueen=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_fake_report_value_en_set( %u)\n", fakereportvalueen);
        if(!err) ag_drv_epn_fake_report_value_en_set(fakereportvalueen);
        if(!err) ag_drv_epn_fake_report_value_en_get( &fakereportvalueen);
        if(!err) bdmf_session_print(session, "ag_drv_epn_fake_report_value_en_get( %u)\n", fakereportvalueen);
        if(err || fakereportvalueen!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t fakereportvalue=gtmv(m, 21);
        if(!err) bdmf_session_print(session, "ag_drv_epn_fake_report_value_set( %u)\n", fakereportvalue);
        if(!err) ag_drv_epn_fake_report_value_set(fakereportvalue);
        if(!err) ag_drv_epn_fake_report_value_get( &fakereportvalue);
        if(!err) bdmf_session_print(session, "ag_drv_epn_fake_report_value_get( %u)\n", fakereportvalue);
        if(err || fakereportvalue!=gtmv(m, 21))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        uint32_t burstcap=gtmv(m, 20);
        if(!err) bdmf_session_print(session, "ag_drv_epn_burst_cap_set( %u %u)\n", link_idx, burstcap);
        if(!err) ag_drv_epn_burst_cap_set(link_idx, burstcap);
        if(!err) ag_drv_epn_burst_cap_get( link_idx, &burstcap);
        if(!err) bdmf_session_print(session, "ag_drv_epn_burst_cap_get( %u %u)\n", link_idx, burstcap);
        if(err || burstcap!=gtmv(m, 20))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t que_idx=gtmv(m, 5);
        uint8_t quellidmap=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_epn_queue_llid_map_set( %u %u)\n", que_idx, quellidmap);
        if(!err) ag_drv_epn_queue_llid_map_set(que_idx, quellidmap);
        if(!err) ag_drv_epn_queue_llid_map_get( que_idx, &quellidmap);
        if(!err) bdmf_session_print(session, "ag_drv_epn_queue_llid_map_get( %u %u)\n", que_idx, quellidmap);
        if(err || quellidmap!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t prvvalidmpcpopcodes=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_valid_opcode_map_set( %u)\n", prvvalidmpcpopcodes);
        if(!err) ag_drv_epn_valid_opcode_map_set(prvvalidmpcpopcodes);
        if(!err) ag_drv_epn_valid_opcode_map_get( &prvvalidmpcpopcodes);
        if(!err) bdmf_session_print(session, "ag_drv_epn_valid_opcode_map_get( %u)\n", prvvalidmpcpopcodes);
        if(err || prvvalidmpcpopcodes!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t uppackettxmargin=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_up_packet_tx_margin_set( %u)\n", uppackettxmargin);
        if(!err) ag_drv_epn_up_packet_tx_margin_set(uppackettxmargin);
        if(!err) ag_drv_epn_up_packet_tx_margin_get( &uppackettxmargin);
        if(!err) bdmf_session_print(session, "ag_drv_epn_up_packet_tx_margin_get( %u)\n", uppackettxmargin);
        if(err || uppackettxmargin!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        epn_multi_pri_cfg_0 multi_pri_cfg_0 = {.cfgctcschdeficiten=gtmv(m, 1), .prvzeroburstcapoverridemode=gtmv(m, 2), .cfgsharedl2=gtmv(m, 1), .cfgsharedburstcap=gtmv(m, 1), .cfgrptgntsoutst0=gtmv(m, 1), .cfgrpthiprifirst0=gtmv(m, 1), .cfgrptswapqs0=gtmv(m, 1), .cfgrptmultipri0=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_epn_multi_pri_cfg_0_set( %u %u %u %u %u %u %u %u)\n", multi_pri_cfg_0.cfgctcschdeficiten, multi_pri_cfg_0.prvzeroburstcapoverridemode, multi_pri_cfg_0.cfgsharedl2, multi_pri_cfg_0.cfgsharedburstcap, multi_pri_cfg_0.cfgrptgntsoutst0, multi_pri_cfg_0.cfgrpthiprifirst0, multi_pri_cfg_0.cfgrptswapqs0, multi_pri_cfg_0.cfgrptmultipri0);
        if(!err) ag_drv_epn_multi_pri_cfg_0_set(&multi_pri_cfg_0);
        if(!err) ag_drv_epn_multi_pri_cfg_0_get( &multi_pri_cfg_0);
        if(!err) bdmf_session_print(session, "ag_drv_epn_multi_pri_cfg_0_get( %u %u %u %u %u %u %u %u)\n", multi_pri_cfg_0.cfgctcschdeficiten, multi_pri_cfg_0.prvzeroburstcapoverridemode, multi_pri_cfg_0.cfgsharedl2, multi_pri_cfg_0.cfgsharedburstcap, multi_pri_cfg_0.cfgrptgntsoutst0, multi_pri_cfg_0.cfgrpthiprifirst0, multi_pri_cfg_0.cfgrptswapqs0, multi_pri_cfg_0.cfgrptmultipri0);
        if(err || multi_pri_cfg_0.cfgctcschdeficiten!=gtmv(m, 1) || multi_pri_cfg_0.prvzeroburstcapoverridemode!=gtmv(m, 2) || multi_pri_cfg_0.cfgsharedl2!=gtmv(m, 1) || multi_pri_cfg_0.cfgsharedburstcap!=gtmv(m, 1) || multi_pri_cfg_0.cfgrptgntsoutst0!=gtmv(m, 1) || multi_pri_cfg_0.cfgrpthiprifirst0!=gtmv(m, 1) || multi_pri_cfg_0.cfgrptswapqs0!=gtmv(m, 1) || multi_pri_cfg_0.cfgrptmultipri0!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t sharedburstcapoverflow=gtmv(m, 11);
        if(!err) ag_drv_epn_shared_bcap_ovrflow_get( &sharedburstcapoverflow);
        if(!err) bdmf_session_print(session, "ag_drv_epn_shared_bcap_ovrflow_get( %u)\n", sharedburstcapoverflow);
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean cfgForceReportEn=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_forced_report_en_set( %u %u)\n", link_idx, cfgForceReportEn);
        if(!err) ag_drv_epn_forced_report_en_set(link_idx, cfgForceReportEn);
        if(!err) ag_drv_epn_forced_report_en_get( link_idx, &cfgForceReportEn);
        if(!err) bdmf_session_print(session, "ag_drv_epn_forced_report_en_get( %u %u)\n", link_idx, cfgForceReportEn);
        if(err || cfgForceReportEn!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cfgmaxreportinterval=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_epn_forced_report_max_interval_set( %u)\n", cfgmaxreportinterval);
        if(!err) ag_drv_epn_forced_report_max_interval_set(cfgmaxreportinterval);
        if(!err) ag_drv_epn_forced_report_max_interval_get( &cfgmaxreportinterval);
        if(!err) bdmf_session_print(session, "ag_drv_epn_forced_report_max_interval_get( %u)\n", cfgmaxreportinterval);
        if(err || cfgmaxreportinterval!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean cfgflushl2sen=gtmv(m, 1);
        bdmf_boolean flushl2sdone=gtmv(m, 1);
        uint8_t cfgflushl2ssel=gtmv(m, 5);
        if(!err) bdmf_session_print(session, "ag_drv_epn_l2s_flush_config_set( %u %u %u)\n", cfgflushl2sen, flushl2sdone, cfgflushl2ssel);
        if(!err) ag_drv_epn_l2s_flush_config_set(cfgflushl2sen, flushl2sdone, cfgflushl2ssel);
        if(!err) ag_drv_epn_l2s_flush_config_get( &cfgflushl2sen, &flushl2sdone, &cfgflushl2ssel);
        if(!err) bdmf_session_print(session, "ag_drv_epn_l2s_flush_config_get( %u %u %u)\n", cfgflushl2sen, flushl2sdone, cfgflushl2ssel);
        if(err || cfgflushl2sen!=gtmv(m, 1) || flushl2sdone!=gtmv(m, 1) || cfgflushl2ssel!=gtmv(m, 5))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean dportbusy=gtmv(m, 1);
        uint8_t dportselect=gtmv(m, 5);
        bdmf_boolean dportcontrol=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_data_port_command_set( %u %u %u)\n", dportbusy, dportselect, dportcontrol);
        if(!err) ag_drv_epn_data_port_command_set(dportbusy, dportselect, dportcontrol);
        if(!err) ag_drv_epn_data_port_command_get( &dportbusy, &dportselect, &dportcontrol);
        if(!err) bdmf_session_print(session, "ag_drv_epn_data_port_command_get( %u %u %u)\n", dportbusy, dportselect, dportcontrol);
        if(err || dportbusy!=gtmv(m, 1) || dportselect!=gtmv(m, 5) || dportcontrol!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t dportaddr=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_epn_data_port_address_set( %u)\n", dportaddr);
        if(!err) ag_drv_epn_data_port_address_set(dportaddr);
        if(!err) ag_drv_epn_data_port_address_get( &dportaddr);
        if(!err) bdmf_session_print(session, "ag_drv_epn_data_port_address_get( %u)\n", dportaddr);
        if(err || dportaddr!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t dportdata0=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_data_port_data_0_set( %u)\n", dportdata0);
        if(!err) ag_drv_epn_data_port_data_0_set(dportdata0);
        if(!err) ag_drv_epn_data_port_data_0_get( &dportdata0);
        if(!err) bdmf_session_print(session, "ag_drv_epn_data_port_data_0_get( %u)\n", dportdata0);
        if(err || dportdata0!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t unmapbigerrcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_big_cnt_set( %u)\n", unmapbigerrcnt);
        if(!err) ag_drv_epn_unmap_big_cnt_set(unmapbigerrcnt);
        if(!err) ag_drv_epn_unmap_big_cnt_get( &unmapbigerrcnt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_big_cnt_get( %u)\n", unmapbigerrcnt);
        if(err || unmapbigerrcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t unmapfrcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_frame_cnt_set( %u)\n", unmapfrcnt);
        if(!err) ag_drv_epn_unmap_frame_cnt_set(unmapfrcnt);
        if(!err) ag_drv_epn_unmap_frame_cnt_get( &unmapfrcnt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_frame_cnt_get( %u)\n", unmapfrcnt);
        if(err || unmapfrcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t unmapfcserrcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_fcs_cnt_set( %u)\n", unmapfcserrcnt);
        if(!err) ag_drv_epn_unmap_fcs_cnt_set(unmapfcserrcnt);
        if(!err) ag_drv_epn_unmap_fcs_cnt_get( &unmapfcserrcnt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_fcs_cnt_get( %u)\n", unmapfcserrcnt);
        if(err || unmapfcserrcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t unmapgatecnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_gate_cnt_set( %u)\n", unmapgatecnt);
        if(!err) ag_drv_epn_unmap_gate_cnt_set(unmapgatecnt);
        if(!err) ag_drv_epn_unmap_gate_cnt_get( &unmapgatecnt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_gate_cnt_get( %u)\n", unmapgatecnt);
        if(err || unmapgatecnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t unmapoamcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_oam_cnt_set( %u)\n", unmapoamcnt);
        if(!err) ag_drv_epn_unmap_oam_cnt_set(unmapoamcnt);
        if(!err) ag_drv_epn_unmap_oam_cnt_get( &unmapoamcnt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_oam_cnt_get( %u)\n", unmapoamcnt);
        if(err || unmapoamcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t unmapsmallerrcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_small_cnt_set( %u)\n", unmapsmallerrcnt);
        if(!err) ag_drv_epn_unmap_small_cnt_set(unmapsmallerrcnt);
        if(!err) ag_drv_epn_unmap_small_cnt_get( &unmapsmallerrcnt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unmap_small_cnt_get( %u)\n", unmapsmallerrcnt);
        if(err || unmapsmallerrcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t fifdequeueeventcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_fif_dequeue_event_cnt_set( %u)\n", fifdequeueeventcnt);
        if(!err) ag_drv_epn_fif_dequeue_event_cnt_set(fifdequeueeventcnt);
        if(!err) ag_drv_epn_fif_dequeue_event_cnt_get( &fifdequeueeventcnt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_fif_dequeue_event_cnt_get( %u)\n", fifdequeueeventcnt);
        if(err || fifdequeueeventcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        uint32_t unusedtqcnt=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unused_tq_cnt_set( %u %u)\n", link_idx, unusedtqcnt);
        if(!err) ag_drv_epn_unused_tq_cnt_set(link_idx, unusedtqcnt);
        if(!err) ag_drv_epn_unused_tq_cnt_get( link_idx, &unusedtqcnt);
        if(!err) bdmf_session_print(session, "ag_drv_epn_unused_tq_cnt_get( %u %u)\n", link_idx, unusedtqcnt);
        if(err || unusedtqcnt!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean bbhUpsFaultHaltEn=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_up_fault_halt_en_set( %u %u)\n", link_idx, bbhUpsFaultHaltEn);
        if(!err) ag_drv_epn_bbh_up_fault_halt_en_set(link_idx, bbhUpsFaultHaltEn);
        if(!err) ag_drv_epn_bbh_up_fault_halt_en_get( link_idx, &bbhUpsFaultHaltEn);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_up_fault_halt_en_get( %u %u)\n", link_idx, bbhUpsFaultHaltEn);
        if(err || bbhUpsFaultHaltEn!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean fataltardybbhaborten=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_up_tardy_halt_en_set( %u)\n", fataltardybbhaborten);
        if(!err) ag_drv_epn_bbh_up_tardy_halt_en_set(fataltardybbhaborten);
        if(!err) ag_drv_epn_bbh_up_tardy_halt_en_get( &fataltardybbhaborten);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_up_tardy_halt_en_get( %u)\n", fataltardybbhaborten);
        if(err || fataltardybbhaborten!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t l2squefulldebug=gtmv(m, 8);
        bdmf_boolean dndlufull=gtmv(m, 1);
        bdmf_boolean dnsecfull=gtmv(m, 1);
        bdmf_boolean epnliffifofull=gtmv(m, 1);
        if(!err) ag_drv_epn_debug_status_0_get( &l2squefulldebug, &dndlufull, &dnsecfull, &epnliffifofull);
        if(!err) bdmf_session_print(session, "ag_drv_epn_debug_status_0_get( %u %u %u %u)\n", l2squefulldebug, dndlufull, dnsecfull, epnliffifofull);
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean gntRdy=gtmv(m, 1);
        if(!err) ag_drv_epn_debug_status_1_get( link_idx, &gntRdy);
        if(!err) bdmf_session_print(session, "ag_drv_epn_debug_status_1_get( %u %u)\n", link_idx, gntRdy);
    }
    {
        uint8_t cfgl2sdebugptrsel=gtmv(m, 3);
        uint16_t l2sdebugptrstate=gtmv(m, 15);
        if(!err) bdmf_session_print(session, "ag_drv_epn_debug_l2s_ptr_sel_set( %u %u)\n", cfgl2sdebugptrsel, l2sdebugptrstate);
        if(!err) ag_drv_epn_debug_l2s_ptr_sel_set(cfgl2sdebugptrsel, l2sdebugptrstate);
        if(!err) ag_drv_epn_debug_l2s_ptr_sel_get( &cfgl2sdebugptrsel, &l2sdebugptrstate);
        if(!err) bdmf_session_print(session, "ag_drv_epn_debug_l2s_ptr_sel_get( %u %u)\n", cfgl2sdebugptrsel, l2sdebugptrstate);
        if(err || cfgl2sdebugptrsel!=gtmv(m, 3) || l2sdebugptrstate!=gtmv(m, 15))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t oltaddrlo=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_olt_mac_addr_lo_set( %u)\n", oltaddrlo);
        if(!err) ag_drv_epn_olt_mac_addr_lo_set(oltaddrlo);
        if(!err) ag_drv_epn_olt_mac_addr_lo_get( &oltaddrlo);
        if(!err) bdmf_session_print(session, "ag_drv_epn_olt_mac_addr_lo_get( %u)\n", oltaddrlo);
        if(err || oltaddrlo!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t oltaddrhi=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_olt_mac_addr_hi_set( %u)\n", oltaddrhi);
        if(!err) ag_drv_epn_olt_mac_addr_hi_set(oltaddrhi);
        if(!err) ag_drv_epn_olt_mac_addr_hi_get( &oltaddrhi);
        if(!err) bdmf_session_print(session, "ag_drv_epn_olt_mac_addr_hi_get( %u)\n", oltaddrhi);
        if(err || oltaddrhi!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t l1_acc_idx=gtmv(m, 5);
        bdmf_boolean l1sDquQueEmpty=gtmv(m, 1);
        if(!err) ag_drv_epn_tx_l1s_shp_dqu_empty_get( l1_acc_idx, &l1sDquQueEmpty);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l1s_shp_dqu_empty_get( %u %u)\n", l1_acc_idx, l1sDquQueEmpty);
    }
    {
        uint8_t l1_acc_idx=gtmv(m, 5);
        bdmf_boolean l1sUnshapedQueEmpty=gtmv(m, 1);
        if(!err) ag_drv_epn_tx_l1s_unshaped_empty_get( l1_acc_idx, &l1sUnshapedQueEmpty);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l1s_unshaped_empty_get( %u %u)\n", l1_acc_idx, l1sUnshapedQueEmpty);
    }
    {
        uint8_t shaper_idx=gtmv(m, 5);
        uint32_t cfgshpmask=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l1s_shp_que_mask_set( %u %u)\n", shaper_idx, cfgshpmask);
        if(!err) ag_drv_epn_tx_l1s_shp_que_mask_set(shaper_idx, cfgshpmask);
        if(!err) ag_drv_epn_tx_l1s_shp_que_mask_get( shaper_idx, &cfgshpmask);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l1s_shp_que_mask_get( %u %u)\n", shaper_idx, cfgshpmask);
        if(err || cfgshpmask!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t que_idx=gtmv(m, 5);
        uint16_t cfgl2squeend=gtmv(m, 12);
        uint16_t cfgl2squestart=gtmv(m, 12);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l2s_queue_config_set( %u %u %u)\n", que_idx, cfgl2squeend, cfgl2squestart);
        if(!err) ag_drv_epn_tx_l2s_queue_config_set(que_idx, cfgl2squeend, cfgl2squestart);
        if(!err) ag_drv_epn_tx_l2s_queue_config_get( que_idx, &cfgl2squeend, &cfgl2squestart);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l2s_queue_config_get( %u %u %u)\n", que_idx, cfgl2squeend, cfgl2squestart);
        if(err || cfgl2squeend!=gtmv(m, 12) || cfgl2squestart!=gtmv(m, 12))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t l2_queue_idx=gtmv(m, 5);
        bdmf_boolean l2sQueEmpty=gtmv(m, 1);
        if(!err) ag_drv_epn_tx_l2s_que_empty_get( l2_queue_idx, &l2sQueEmpty);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l2s_que_empty_get( %u %u)\n", l2_queue_idx, l2sQueEmpty);
    }
    {
        uint8_t l2_queue_idx=gtmv(m, 5);
        bdmf_boolean l2sQueFull=gtmv(m, 1);
        if(!err) ag_drv_epn_tx_l2s_que_full_get( l2_queue_idx, &l2sQueFull);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l2s_que_full_get( %u %u)\n", l2_queue_idx, l2sQueFull);
    }
    {
        uint8_t l2_queue_idx=gtmv(m, 5);
        bdmf_boolean l2sStoppedQueues=gtmv(m, 1);
        if(!err) ag_drv_epn_tx_l2s_que_stopped_get( l2_queue_idx, &l2sStoppedQueues);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l2s_que_stopped_get( %u %u)\n", l2_queue_idx, l2sStoppedQueues);
    }
    {
        uint8_t l2_queue_idx=gtmv(m, 5);
        uint32_t prvburstlimit=gtmv(m, 18);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_ctc_burst_limit_set( %u %u)\n", l2_queue_idx, prvburstlimit);
        if(!err) ag_drv_epn_tx_ctc_burst_limit_set(l2_queue_idx, prvburstlimit);
        if(!err) ag_drv_epn_tx_ctc_burst_limit_get( l2_queue_idx, &prvburstlimit);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_ctc_burst_limit_get( %u %u)\n", l2_queue_idx, prvburstlimit);
        if(err || prvburstlimit!=gtmv(m, 18))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgmaxoutstandingtardypackets=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_max_outstanding_tardy_packets_set( %u)\n", cfgmaxoutstandingtardypackets);
        if(!err) ag_drv_epn_bbh_max_outstanding_tardy_packets_set(cfgmaxoutstandingtardypackets);
        if(!err) ag_drv_epn_bbh_max_outstanding_tardy_packets_get( &cfgmaxoutstandingtardypackets);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_max_outstanding_tardy_packets_get( %u)\n", cfgmaxoutstandingtardypackets);
        if(err || cfgmaxoutstandingtardypackets!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t prvminreportdiff=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_epn_min_report_value_difference_set( %u)\n", prvminreportdiff);
        if(!err) ag_drv_epn_min_report_value_difference_set(prvminreportdiff);
        if(!err) ag_drv_epn_min_report_value_difference_get( &prvminreportdiff);
        if(!err) bdmf_session_print(session, "ag_drv_epn_min_report_value_difference_get( %u)\n", prvminreportdiff);
        if(err || prvminreportdiff!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t bbh_queue_idx=gtmv(m, 5);
        bdmf_boolean utxBbhStatusFifoOverflow=gtmv(m, 1);
        if(!err) ag_drv_epn_bbh_status_fifo_overflow_get( bbh_queue_idx, &utxBbhStatusFifoOverflow);
        if(!err) bdmf_session_print(session, "ag_drv_epn_bbh_status_fifo_overflow_get( %u %u)\n", bbh_queue_idx, utxBbhStatusFifoOverflow);
    }
    {
        uint32_t cfgepnspare=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_spare_ctl_set( %u)\n", cfgepnspare);
        if(!err) ag_drv_epn_spare_ctl_set(cfgepnspare);
        if(!err) ag_drv_epn_spare_ctl_get( &cfgepnspare);
        if(!err) bdmf_session_print(session, "ag_drv_epn_spare_ctl_get( %u)\n", cfgepnspare);
        if(err || cfgepnspare!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cfgtssyncoffset=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_epn_ts_sync_offset_set( %u)\n", cfgtssyncoffset);
        if(!err) ag_drv_epn_ts_sync_offset_set(cfgtssyncoffset);
        if(!err) ag_drv_epn_ts_sync_offset_get( &cfgtssyncoffset);
        if(!err) bdmf_session_print(session, "ag_drv_epn_ts_sync_offset_get( %u)\n", cfgtssyncoffset);
        if(err || cfgtssyncoffset!=gtmv(m, 6))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgdntsoffset=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_ts_offset_set( %u)\n", cfgdntsoffset);
        if(!err) ag_drv_epn_dn_ts_offset_set(cfgdntsoffset);
        if(!err) ag_drv_epn_dn_ts_offset_get( &cfgdntsoffset);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_ts_offset_get( %u)\n", cfgdntsoffset);
        if(err || cfgdntsoffset!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfguptsoffset_lo=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_up_ts_offset_lo_set( %u)\n", cfguptsoffset_lo);
        if(!err) ag_drv_epn_up_ts_offset_lo_set(cfguptsoffset_lo);
        if(!err) ag_drv_epn_up_ts_offset_lo_get( &cfguptsoffset_lo);
        if(!err) bdmf_session_print(session, "ag_drv_epn_up_ts_offset_lo_get( %u)\n", cfguptsoffset_lo);
        if(err || cfguptsoffset_lo!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfguptsoffset_hi=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_up_ts_offset_hi_set( %u)\n", cfguptsoffset_hi);
        if(!err) ag_drv_epn_up_ts_offset_hi_set(cfguptsoffset_hi);
        if(!err) ag_drv_epn_up_ts_offset_hi_get( &cfguptsoffset_hi);
        if(!err) bdmf_session_print(session, "ag_drv_epn_up_ts_offset_hi_get( %u)\n", cfguptsoffset_hi);
        if(err || cfguptsoffset_hi!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean twostepffrd=gtmv(m, 1);
        uint8_t twostepffentries=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_epn_two_step_ts_ctl_set( %u %u)\n", twostepffrd, twostepffentries);
        if(!err) ag_drv_epn_two_step_ts_ctl_set(twostepffrd, twostepffentries);
        if(!err) ag_drv_epn_two_step_ts_ctl_get( &twostepffrd, &twostepffentries);
        if(!err) bdmf_session_print(session, "ag_drv_epn_two_step_ts_ctl_get( %u %u)\n", twostepffrd, twostepffentries);
        if(err || twostepffrd!=gtmv(m, 1) || twostepffentries!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t twosteptimestamp_lo=gtmv(m, 32);
        if(!err) ag_drv_epn_two_step_ts_value_lo_get( &twosteptimestamp_lo);
        if(!err) bdmf_session_print(session, "ag_drv_epn_two_step_ts_value_lo_get( %u)\n", twosteptimestamp_lo);
    }
    {
        uint16_t twosteptimestamp_hi=gtmv(m, 16);
        if(!err) ag_drv_epn_two_step_ts_value_hi_get( &twosteptimestamp_hi);
        if(!err) bdmf_session_print(session, "ag_drv_epn_two_step_ts_value_hi_get( %u)\n", twosteptimestamp_hi);
    }
    {
        bdmf_boolean int1588pktabort=gtmv(m, 1);
        bdmf_boolean int1588twostepffint=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_1588_timestamp_int_status_set( %u %u)\n", int1588pktabort, int1588twostepffint);
        if(!err) ag_drv_epn_1588_timestamp_int_status_set(int1588pktabort, int1588twostepffint);
        if(!err) ag_drv_epn_1588_timestamp_int_status_get( &int1588pktabort, &int1588twostepffint);
        if(!err) bdmf_session_print(session, "ag_drv_epn_1588_timestamp_int_status_get( %u %u)\n", int1588pktabort, int1588twostepffint);
        if(err || int1588pktabort!=gtmv(m, 1) || int1588twostepffint!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean ts1588pktabort_mask=gtmv(m, 1);
        bdmf_boolean ts1588twostepff_mask=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_1588_timestamp_int_mask_set( %u %u)\n", ts1588pktabort_mask, ts1588twostepff_mask);
        if(!err) ag_drv_epn_1588_timestamp_int_mask_set(ts1588pktabort_mask, ts1588twostepff_mask);
        if(!err) ag_drv_epn_1588_timestamp_int_mask_get( &ts1588pktabort_mask, &ts1588twostepff_mask);
        if(!err) bdmf_session_print(session, "ag_drv_epn_1588_timestamp_int_mask_get( %u %u)\n", ts1588pktabort_mask, ts1588twostepff_mask);
        if(err || ts1588pktabort_mask!=gtmv(m, 1) || ts1588twostepff_mask!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t uppacketfetchmargin=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_up_packet_fetch_margin_set( %u)\n", uppacketfetchmargin);
        if(!err) ag_drv_epn_up_packet_fetch_margin_set(uppacketfetchmargin);
        if(!err) ag_drv_epn_up_packet_fetch_margin_get( &uppacketfetchmargin);
        if(!err) bdmf_session_print(session, "ag_drv_epn_up_packet_fetch_margin_get( %u)\n", uppacketfetchmargin);
        if(err || uppacketfetchmargin!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t dn_1588_ts=gtmv(m, 32);
        if(!err) ag_drv_epn_dn_1588_timestamp_get( &dn_1588_ts);
        if(!err) bdmf_session_print(session, "ag_drv_epn_dn_1588_timestamp_get( %u)\n", dn_1588_ts);
    }
    {
        uint16_t cfgpersrptduration=gtmv(m, 10);
        uint16_t cfgpersrptticksize=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_persistent_report_cfg_set( %u %u)\n", cfgpersrptduration, cfgpersrptticksize);
        if(!err) ag_drv_epn_persistent_report_cfg_set(cfgpersrptduration, cfgpersrptticksize);
        if(!err) ag_drv_epn_persistent_report_cfg_get( &cfgpersrptduration, &cfgpersrptticksize);
        if(!err) bdmf_session_print(session, "ag_drv_epn_persistent_report_cfg_get( %u %u)\n", cfgpersrptduration, cfgpersrptticksize);
        if(err || cfgpersrptduration!=gtmv(m, 10) || cfgpersrptticksize!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t link_idx=gtmv(m, 5);
        bdmf_boolean cfgPersRptEnable=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_epn_persistent_report_enables_set( %u %u)\n", link_idx, cfgPersRptEnable);
        if(!err) ag_drv_epn_persistent_report_enables_set(link_idx, cfgPersRptEnable);
        if(!err) ag_drv_epn_persistent_report_enables_get( link_idx, &cfgPersRptEnable);
        if(!err) bdmf_session_print(session, "ag_drv_epn_persistent_report_enables_get( %u %u)\n", link_idx, cfgPersRptEnable);
        if(err || cfgPersRptEnable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgpersrptreqtq=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_persistent_report_request_size_set( %u)\n", cfgpersrptreqtq);
        if(!err) ag_drv_epn_persistent_report_request_size_set(cfgpersrptreqtq);
        if(!err) ag_drv_epn_persistent_report_request_size_get( &cfgpersrptreqtq);
        if(!err) bdmf_session_print(session, "ag_drv_epn_persistent_report_request_size_get( %u)\n", cfgpersrptreqtq);
        if(err || cfgpersrptreqtq!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_epn_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_control_0 : reg = &RU_REG(EPN, CONTROL_0); blk = &RU_BLK(EPN); break;
    case bdmf_address_control_1 : reg = &RU_REG(EPN, CONTROL_1); blk = &RU_BLK(EPN); break;
    case bdmf_address_enable_grants : reg = &RU_REG(EPN, ENABLE_GRANTS); blk = &RU_BLK(EPN); break;
    case bdmf_address_drop_disc_gates : reg = &RU_REG(EPN, DROP_DISC_GATES); blk = &RU_BLK(EPN); break;
    case bdmf_address_dis_fcs_chk : reg = &RU_REG(EPN, DIS_FCS_CHK); blk = &RU_BLK(EPN); break;
    case bdmf_address_pass_gates : reg = &RU_REG(EPN, PASS_GATES); blk = &RU_BLK(EPN); break;
    case bdmf_address_cfg_misalgn_fb : reg = &RU_REG(EPN, CFG_MISALGN_FB); blk = &RU_BLK(EPN); break;
    case bdmf_address_discovery_filter : reg = &RU_REG(EPN, DISCOVERY_FILTER); blk = &RU_BLK(EPN); break;
    case bdmf_address_bbh_que_status_delay : reg = &RU_REG(EPN, BBH_QUE_STATUS_DELAY); blk = &RU_BLK(EPN); break;
    case bdmf_address_minimum_grant_setup : reg = &RU_REG(EPN, MINIMUM_GRANT_SETUP); blk = &RU_BLK(EPN); break;
    case bdmf_address_reset_gnt_fifo : reg = &RU_REG(EPN, RESET_GNT_FIFO); blk = &RU_BLK(EPN); break;
    case bdmf_address_reset_l1_accumulator : reg = &RU_REG(EPN, RESET_L1_ACCUMULATOR); blk = &RU_BLK(EPN); break;
    case bdmf_address_l1_accumulator_sel : reg = &RU_REG(EPN, L1_ACCUMULATOR_SEL); blk = &RU_BLK(EPN); break;
    case bdmf_address_l1_sva_bytes : reg = &RU_REG(EPN, L1_SVA_BYTES); blk = &RU_BLK(EPN); break;
    case bdmf_address_l1_uva_bytes : reg = &RU_REG(EPN, L1_UVA_BYTES); blk = &RU_BLK(EPN); break;
    case bdmf_address_l1_sva_overflow : reg = &RU_REG(EPN, L1_SVA_OVERFLOW); blk = &RU_BLK(EPN); break;
    case bdmf_address_l1_uva_overflow : reg = &RU_REG(EPN, L1_UVA_OVERFLOW); blk = &RU_BLK(EPN); break;
    case bdmf_address_reset_rpt_pri : reg = &RU_REG(EPN, RESET_RPT_PRI); blk = &RU_BLK(EPN); break;
    case bdmf_address_reset_l2_rpt_fifo : reg = &RU_REG(EPN, RESET_L2_RPT_FIFO); blk = &RU_BLK(EPN); break;
    case bdmf_address_enable_upstream : reg = &RU_REG(EPN, ENABLE_UPSTREAM); blk = &RU_BLK(EPN); break;
    case bdmf_address_enable_upstream_fb : reg = &RU_REG(EPN, ENABLE_UPSTREAM_FB); blk = &RU_BLK(EPN); break;
    case bdmf_address_enable_upstream_fec : reg = &RU_REG(EPN, ENABLE_UPSTREAM_FEC); blk = &RU_BLK(EPN); break;
    case bdmf_address_report_byte_length : reg = &RU_REG(EPN, REPORT_BYTE_LENGTH); blk = &RU_BLK(EPN); break;
    case bdmf_address_main_int_status : reg = &RU_REG(EPN, MAIN_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_full_int_status : reg = &RU_REG(EPN, GNT_FULL_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_full_int_mask : reg = &RU_REG(EPN, GNT_FULL_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_miss_int_status : reg = &RU_REG(EPN, GNT_MISS_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_miss_int_mask : reg = &RU_REG(EPN, GNT_MISS_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_disc_rx_int_status : reg = &RU_REG(EPN, DISC_RX_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_disc_rx_int_mask : reg = &RU_REG(EPN, DISC_RX_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_intv_int_status : reg = &RU_REG(EPN, GNT_INTV_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_intv_int_mask : reg = &RU_REG(EPN, GNT_INTV_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_far_int_status : reg = &RU_REG(EPN, GNT_FAR_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_far_int_mask : reg = &RU_REG(EPN, GNT_FAR_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_misalgn_int_status : reg = &RU_REG(EPN, GNT_MISALGN_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_misalgn_int_mask : reg = &RU_REG(EPN, GNT_MISALGN_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_np_gnt_int_status : reg = &RU_REG(EPN, NP_GNT_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_np_gnt_int_mask : reg = &RU_REG(EPN, NP_GNT_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_del_stale_int_status : reg = &RU_REG(EPN, DEL_STALE_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_del_stale_int_mask : reg = &RU_REG(EPN, DEL_STALE_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_pres_int_status : reg = &RU_REG(EPN, GNT_PRES_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_pres_int_mask : reg = &RU_REG(EPN, GNT_PRES_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_rpt_pres_int_status : reg = &RU_REG(EPN, RPT_PRES_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_rpt_pres_int_mask : reg = &RU_REG(EPN, RPT_PRES_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_drx_abort_int_status : reg = &RU_REG(EPN, DRX_ABORT_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_drx_abort_int_mask : reg = &RU_REG(EPN, DRX_ABORT_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_empty_rpt_int_status : reg = &RU_REG(EPN, EMPTY_RPT_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_empty_rpt_int_mask : reg = &RU_REG(EPN, EMPTY_RPT_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_bcap_overflow_int_status : reg = &RU_REG(EPN, BCAP_OVERFLOW_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_bcap_overflow_int_mask : reg = &RU_REG(EPN, BCAP_OVERFLOW_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_bbh_dns_fault_int_status : reg = &RU_REG(EPN, BBH_DNS_FAULT_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_bbh_dns_fault_int_mask : reg = &RU_REG(EPN, BBH_DNS_FAULT_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_bbh_ups_fault_int_status : reg = &RU_REG(EPN, BBH_UPS_FAULT_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_bbh_ups_fault_int_mask : reg = &RU_REG(EPN, BBH_UPS_FAULT_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_bbh_ups_abort_int_status : reg = &RU_REG(EPN, BBH_UPS_ABORT_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_bbh_ups_abort_int_mask : reg = &RU_REG(EPN, BBH_UPS_ABORT_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_main_int_mask : reg = &RU_REG(EPN, MAIN_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_max_gnt_size : reg = &RU_REG(EPN, MAX_GNT_SIZE); blk = &RU_BLK(EPN); break;
    case bdmf_address_max_frame_size : reg = &RU_REG(EPN, MAX_FRAME_SIZE); blk = &RU_BLK(EPN); break;
    case bdmf_address_grant_ovr_hd : reg = &RU_REG(EPN, GRANT_OVR_HD); blk = &RU_BLK(EPN); break;
    case bdmf_address_poll_size : reg = &RU_REG(EPN, POLL_SIZE); blk = &RU_BLK(EPN); break;
    case bdmf_address_dn_rd_gnt_margin : reg = &RU_REG(EPN, DN_RD_GNT_MARGIN); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_time_start_delta : reg = &RU_REG(EPN, GNT_TIME_START_DELTA); blk = &RU_BLK(EPN); break;
    case bdmf_address_time_stamp_diff : reg = &RU_REG(EPN, TIME_STAMP_DIFF); blk = &RU_BLK(EPN); break;
    case bdmf_address_up_time_stamp_off : reg = &RU_REG(EPN, UP_TIME_STAMP_OFF); blk = &RU_BLK(EPN); break;
    case bdmf_address_gnt_interval : reg = &RU_REG(EPN, GNT_INTERVAL); blk = &RU_BLK(EPN); break;
    case bdmf_address_dn_gnt_misalign_thr : reg = &RU_REG(EPN, DN_GNT_MISALIGN_THR); blk = &RU_BLK(EPN); break;
    case bdmf_address_dn_gnt_misalign_pause : reg = &RU_REG(EPN, DN_GNT_MISALIGN_PAUSE); blk = &RU_BLK(EPN); break;
    case bdmf_address_non_poll_intv : reg = &RU_REG(EPN, NON_POLL_INTV); blk = &RU_BLK(EPN); break;
    case bdmf_address_force_fcs_err : reg = &RU_REG(EPN, FORCE_FCS_ERR); blk = &RU_BLK(EPN); break;
    case bdmf_address_grant_overlap_limit : reg = &RU_REG(EPN, GRANT_OVERLAP_LIMIT); blk = &RU_BLK(EPN); break;
    case bdmf_address_aes_configuration_0 : reg = &RU_REG(EPN, AES_CONFIGURATION_0); blk = &RU_BLK(EPN); break;
    case bdmf_address_aes_configuration_1 : reg = &RU_REG(EPN, AES_CONFIGURATION_1); blk = &RU_BLK(EPN); break;
    case bdmf_address_disc_grant_ovr_hd : reg = &RU_REG(EPN, DISC_GRANT_OVR_HD); blk = &RU_BLK(EPN); break;
    case bdmf_address_dn_discovery_seed : reg = &RU_REG(EPN, DN_DISCOVERY_SEED); blk = &RU_BLK(EPN); break;
    case bdmf_address_dn_discovery_inc : reg = &RU_REG(EPN, DN_DISCOVERY_INC); blk = &RU_BLK(EPN); break;
    case bdmf_address_dn_discovery_size : reg = &RU_REG(EPN, DN_DISCOVERY_SIZE); blk = &RU_BLK(EPN); break;
    case bdmf_address_prog_rpt_vec : reg = &RU_REG(EPN, PROG_RPT_VEC); blk = &RU_BLK(EPN); break;
    case bdmf_address_fec_ipg_length : reg = &RU_REG(EPN, FEC_IPG_LENGTH); blk = &RU_BLK(EPN); break;
    case bdmf_address_fake_report_value_en : reg = &RU_REG(EPN, FAKE_REPORT_VALUE_EN); blk = &RU_BLK(EPN); break;
    case bdmf_address_fake_report_value : reg = &RU_REG(EPN, FAKE_REPORT_VALUE); blk = &RU_BLK(EPN); break;
    case bdmf_address_burst_cap_ : reg = &RU_REG(EPN, BURST_CAP_); blk = &RU_BLK(EPN); break;
    case bdmf_address_queue_llid_map_ : reg = &RU_REG(EPN, QUEUE_LLID_MAP_); blk = &RU_BLK(EPN); break;
    case bdmf_address_valid_opcode_map : reg = &RU_REG(EPN, VALID_OPCODE_MAP); blk = &RU_BLK(EPN); break;
    case bdmf_address_up_packet_tx_margin : reg = &RU_REG(EPN, UP_PACKET_TX_MARGIN); blk = &RU_BLK(EPN); break;
    case bdmf_address_multi_pri_cfg_0 : reg = &RU_REG(EPN, MULTI_PRI_CFG_0); blk = &RU_BLK(EPN); break;
    case bdmf_address_shared_bcap_ovrflow : reg = &RU_REG(EPN, SHARED_BCAP_OVRFLOW); blk = &RU_BLK(EPN); break;
    case bdmf_address_forced_report_en : reg = &RU_REG(EPN, FORCED_REPORT_EN); blk = &RU_BLK(EPN); break;
    case bdmf_address_forced_report_max_interval : reg = &RU_REG(EPN, FORCED_REPORT_MAX_INTERVAL); blk = &RU_BLK(EPN); break;
    case bdmf_address_l2s_flush_config : reg = &RU_REG(EPN, L2S_FLUSH_CONFIG); blk = &RU_BLK(EPN); break;
    case bdmf_address_data_port_command : reg = &RU_REG(EPN, DATA_PORT_COMMAND); blk = &RU_BLK(EPN); break;
    case bdmf_address_data_port_address : reg = &RU_REG(EPN, DATA_PORT_ADDRESS); blk = &RU_BLK(EPN); break;
    case bdmf_address_data_port_data_0 : reg = &RU_REG(EPN, DATA_PORT_DATA_0); blk = &RU_BLK(EPN); break;
    case bdmf_address_unmap_big_cnt : reg = &RU_REG(EPN, UNMAP_BIG_CNT); blk = &RU_BLK(EPN); break;
    case bdmf_address_unmap_frame_cnt : reg = &RU_REG(EPN, UNMAP_FRAME_CNT); blk = &RU_BLK(EPN); break;
    case bdmf_address_unmap_fcs_cnt : reg = &RU_REG(EPN, UNMAP_FCS_CNT); blk = &RU_BLK(EPN); break;
    case bdmf_address_unmap_gate_cnt : reg = &RU_REG(EPN, UNMAP_GATE_CNT); blk = &RU_BLK(EPN); break;
    case bdmf_address_unmap_oam_cnt : reg = &RU_REG(EPN, UNMAP_OAM_CNT); blk = &RU_BLK(EPN); break;
    case bdmf_address_unmap_small_cnt : reg = &RU_REG(EPN, UNMAP_SMALL_CNT); blk = &RU_BLK(EPN); break;
    case bdmf_address_fif_dequeue_event_cnt : reg = &RU_REG(EPN, FIF_DEQUEUE_EVENT_CNT); blk = &RU_BLK(EPN); break;
    case bdmf_address_unused_tq_cnt : reg = &RU_REG(EPN, UNUSED_TQ_CNT); blk = &RU_BLK(EPN); break;
    case bdmf_address_bbh_up_fault_halt_en : reg = &RU_REG(EPN, BBH_UP_FAULT_HALT_EN); blk = &RU_BLK(EPN); break;
    case bdmf_address_bbh_up_tardy_halt_en : reg = &RU_REG(EPN, BBH_UP_TARDY_HALT_EN); blk = &RU_BLK(EPN); break;
    case bdmf_address_debug_status_0 : reg = &RU_REG(EPN, DEBUG_STATUS_0); blk = &RU_BLK(EPN); break;
    case bdmf_address_debug_status_1 : reg = &RU_REG(EPN, DEBUG_STATUS_1); blk = &RU_BLK(EPN); break;
    case bdmf_address_debug_l2s_ptr_sel : reg = &RU_REG(EPN, DEBUG_L2S_PTR_SEL); blk = &RU_BLK(EPN); break;
    case bdmf_address_olt_mac_addr_lo : reg = &RU_REG(EPN, OLT_MAC_ADDR_LO); blk = &RU_BLK(EPN); break;
    case bdmf_address_olt_mac_addr_hi : reg = &RU_REG(EPN, OLT_MAC_ADDR_HI); blk = &RU_BLK(EPN); break;
    case bdmf_address_tx_l1s_shp_dqu_empty : reg = &RU_REG(EPN, TX_L1S_SHP_DQU_EMPTY); blk = &RU_BLK(EPN); break;
    case bdmf_address_tx_l1s_unshaped_empty : reg = &RU_REG(EPN, TX_L1S_UNSHAPED_EMPTY); blk = &RU_BLK(EPN); break;
    case bdmf_address_tx_l1s_shp_que_mask_ : reg = &RU_REG(EPN, TX_L1S_SHP_QUE_MASK_); blk = &RU_BLK(EPN); break;
    case bdmf_address_tx_l2s_que_config_ : reg = &RU_REG(EPN, TX_L2S_QUE_CONFIG_); blk = &RU_BLK(EPN); break;
    case bdmf_address_tx_l2s_que_empty : reg = &RU_REG(EPN, TX_L2S_QUE_EMPTY); blk = &RU_BLK(EPN); break;
    case bdmf_address_tx_l2s_que_full : reg = &RU_REG(EPN, TX_L2S_QUE_FULL); blk = &RU_BLK(EPN); break;
    case bdmf_address_tx_l2s_que_stopped : reg = &RU_REG(EPN, TX_L2S_QUE_STOPPED); blk = &RU_BLK(EPN); break;
    case bdmf_address_tx_ctc_burst_limit_ : reg = &RU_REG(EPN, TX_CTC_BURST_LIMIT_); blk = &RU_BLK(EPN); break;
    case bdmf_address_bbh_max_outstanding_tardy_packets : reg = &RU_REG(EPN, BBH_MAX_OUTSTANDING_TARDY_PACKETS); blk = &RU_BLK(EPN); break;
    case bdmf_address_min_report_value_difference : reg = &RU_REG(EPN, MIN_REPORT_VALUE_DIFFERENCE); blk = &RU_BLK(EPN); break;
    case bdmf_address_bbh_status_fifo_overflow : reg = &RU_REG(EPN, BBH_STATUS_FIFO_OVERFLOW); blk = &RU_BLK(EPN); break;
    case bdmf_address_spare_ctl : reg = &RU_REG(EPN, SPARE_CTL); blk = &RU_BLK(EPN); break;
    case bdmf_address_ts_sync_offset : reg = &RU_REG(EPN, TS_SYNC_OFFSET); blk = &RU_BLK(EPN); break;
    case bdmf_address_dn_ts_offset : reg = &RU_REG(EPN, DN_TS_OFFSET); blk = &RU_BLK(EPN); break;
    case bdmf_address_up_ts_offset_lo : reg = &RU_REG(EPN, UP_TS_OFFSET_LO); blk = &RU_BLK(EPN); break;
    case bdmf_address_up_ts_offset_hi : reg = &RU_REG(EPN, UP_TS_OFFSET_HI); blk = &RU_BLK(EPN); break;
    case bdmf_address_two_step_ts_ctl : reg = &RU_REG(EPN, TWO_STEP_TS_CTL); blk = &RU_BLK(EPN); break;
    case bdmf_address_two_step_ts_value_lo : reg = &RU_REG(EPN, TWO_STEP_TS_VALUE_LO); blk = &RU_BLK(EPN); break;
    case bdmf_address_two_step_ts_value_hi : reg = &RU_REG(EPN, TWO_STEP_TS_VALUE_HI); blk = &RU_BLK(EPN); break;
    case bdmf_address_1588_timestamp_int_status : reg = &RU_REG(EPN, 1588_TIMESTAMP_INT_STATUS); blk = &RU_BLK(EPN); break;
    case bdmf_address_1588_timestamp_int_mask : reg = &RU_REG(EPN, 1588_TIMESTAMP_INT_MASK); blk = &RU_BLK(EPN); break;
    case bdmf_address_up_packet_fetch_margin : reg = &RU_REG(EPN, UP_PACKET_FETCH_MARGIN); blk = &RU_BLK(EPN); break;
    case bdmf_address_dn_1588_timestamp : reg = &RU_REG(EPN, DN_1588_TIMESTAMP); blk = &RU_BLK(EPN); break;
    case bdmf_address_persistent_report_cfg : reg = &RU_REG(EPN, PERSISTENT_REPORT_CFG); blk = &RU_BLK(EPN); break;
    case bdmf_address_persistent_report_enables : reg = &RU_REG(EPN, PERSISTENT_REPORT_ENABLES); blk = &RU_BLK(EPN); break;
    case bdmf_address_persistent_report_request_size : reg = &RU_REG(EPN, PERSISTENT_REPORT_REQUEST_SIZE); blk = &RU_BLK(EPN); break;
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

bdmfmon_handle_t ag_drv_epn_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "epn"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "epn", "epn", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_control_0[]={
            BDMFMON_MAKE_PARM("cfgen1588ts", "cfgen1588ts", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgreplaceupfcs", "cfgreplaceupfcs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgappendupfcs", "cfgappendupfcs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgdropscb", "cfgdropscb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("moduncappedreportlimit", "moduncappedreportlimit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("modmpquesetfirst", "modmpquesetfirst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvlocalmpcppropagation", "prvlocalmpcppropagation", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvtekmodeprefetch", "prvtekmodeprefetch", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvincnonzeroaccum", "prvincnonzeroaccum", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvnounmapppedfcs", "prvnounmapppedfcs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvsupressdiscen", "prvsupressdiscen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgvlanmax", "cfgvlanmax", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fcserronlydatafr", "fcserronlydatafr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvdropunmapppedllid", "prvdropunmapppedllid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvsuppressllidmodebit", "prvsuppressllidmodebit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("moddiscoverydafilteren", "moddiscoverydafilteren", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rptselect", "rptselect", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvdisablesvaquestatusbp", "prvdisablesvaquestatusbp", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("utxloopback", "utxloopback", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("utxen", "utxen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("utxrst_pre_n", "utxrst_pre_n", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgdisabledns", "cfgdisabledns", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("drxloopback", "drxloopback", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("drxen", "drxen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("drxrst_pre_n", "drxrst_pre_n", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_control_1[]={
            BDMFMON_MAKE_PARM("prvoverlappedgntenable", "prvoverlappedgntenable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rstmisalignthr", "rstmisalignthr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgtekrpt", "cfgtekrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgstalegntchk", "cfgstalegntchk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fecrpten", "fecrpten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgl1l2truestrict", "cfgl1l2truestrict", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgctcrpt", "cfgctcrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgtscorrdis", "cfgtscorrdis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgnodiscrpt", "cfgnodiscrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("disablediscscale", "disablediscscale", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("clronrd", "clronrd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_enable_grants[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("grant_en", "grant_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_drop_disc_gates[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("linkdiscgates_en", "linkdiscgates_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dis_fcs_chk[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("disablefcschk", "disablefcschk", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pass_gates[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("passgates", "passgates", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_misalgn_fb[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgmisalignfeedback", "cfgmisalignfeedback", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_discovery_filter[]={
            BDMFMON_MAKE_PARM("prvdiscinfomask", "prvdiscinfomask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvdiscinfovalue", "prvdiscinfovalue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_que_status_delay[]={
            BDMFMON_MAKE_PARM("prvbbhquestatdelay", "prvbbhquestatdelay", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_minimum_grant_setup[]={
            BDMFMON_MAKE_PARM("cfgmingrantsetup", "cfgmingrantsetup", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reset_gnt_fifo[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rstgntfifo", "rstgntfifo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reset_l1_accumulator[]={
            BDMFMON_MAKE_PARM("cfgl1sclracum", "cfgl1sclracum", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_l1_accumulator_sel[]={
            BDMFMON_MAKE_PARM("cfgl1suvasizesel", "cfgl1suvasizesel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgl1ssvasizesel", "cfgl1ssvasizesel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reset_rpt_pri[]={
            BDMFMON_MAKE_PARM("nullrptpri15", "nullrptpri15", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri14", "nullrptpri14", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri13", "nullrptpri13", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri12", "nullrptpri12", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri11", "nullrptpri11", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri10", "nullrptpri10", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri9", "nullrptpri9", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri8", "nullrptpri8", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri7", "nullrptpri7", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri6", "nullrptpri6", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri5", "nullrptpri5", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri4", "nullrptpri4", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri3", "nullrptpri3", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri2", "nullrptpri2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri1", "nullrptpri1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("nullrptpri0", "nullrptpri0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_reset_l2_rpt_fifo[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgl2sclrque", "cfgl2sclrque", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_enable_upstream[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgenableupstream", "cfgenableupstream", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_enable_upstream_fec[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("upstreamfecen", "upstreamfecen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_report_byte_length[]={
            BDMFMON_MAKE_PARM("prvrptbytelen", "prvrptbytelen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_main_int_status[]={
            BDMFMON_MAKE_PARM("intbbhupfrabort", "intbbhupfrabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcol2sburstcapoverflowpres", "intcol2sburstcapoverflowpres", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcoemptyrpt", "intcoemptyrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcodrxerrabortpres", "intcodrxerrabortpres", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intl2sfifooverrun", "intl2sfifooverrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intco1588tsint", "intco1588tsint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcorptpres", "intcorptpres", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntpres", "intcogntpres", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcodelstalegnt", "intcodelstalegnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntnonpoll", "intcogntnonpoll", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntmisalign", "intcogntmisalign", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcognttoofar", "intcognttoofar", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntinterval", "intcogntinterval", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntdiscovery", "intcogntdiscovery", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntmissabort", "intcogntmissabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntfullabort", "intcogntfullabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intbadupfrlen", "intbadupfrlen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intuptardypacket", "intuptardypacket", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intuprptfrxmt", "intuprptfrxmt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intbififooverrun", "intbififooverrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intburstgnttoobig", "intburstgnttoobig", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intwrgnttoobig", "intwrgnttoobig", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intrcvgnttoobig", "intrcvgnttoobig", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdnstatsoverrun", "intdnstatsoverrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intupstatsoverrun", "intupstatsoverrun", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdnoutoforder", "intdnoutoforder", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("inttruantbbhhalt", "inttruantbbhhalt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intupinvldgntlen", "intupinvldgntlen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcobbhupsfault", "intcobbhupsfault", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdntimeinsync", "intdntimeinsync", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdntimenotinsync", "intdntimenotinsync", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdportrdy", "intdportrdy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_full_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdngntfullabort", "intdngntfullabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_full_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskintdngntfullabort", "maskintdngntfullabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_miss_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdngntmissabort", "intdngntmissabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_miss_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskintdngntmissabort", "maskintdngntmissabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_disc_rx_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdngntdiscovery", "intdngntdiscovery", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_disc_rx_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskintdngntdiscovery", "maskintdngntdiscovery", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_intv_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdngntinterval", "intdngntinterval", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_intv_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskintdngntinterval", "maskintdngntinterval", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_far_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdngnttoofar", "intdngnttoofar", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_far_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskdngnttoofar", "maskdngnttoofar", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_misalgn_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdngntmisalign", "intdngntmisalign", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_misalgn_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskintdngntmisalign", "maskintdngntmisalign", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_np_gnt_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdngntnonpoll", "intdngntnonpoll", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_np_gnt_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskdngntnonpoll", "maskdngntnonpoll", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_del_stale_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdelstalegnt", "intdelstalegnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_del_stale_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskintdelstalegnt", "maskintdelstalegnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_pres_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdngntrdy", "intdngntrdy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_pres_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskdngntrdy", "maskdngntrdy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rpt_pres_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intuprptfifo", "intuprptfifo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rpt_pres_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskintuprptfifo", "maskintuprptfifo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_drx_abort_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdrxerrabort", "intdrxerrabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_drx_abort_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskintdrxerrabort", "maskintdrxerrabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_empty_rpt_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intemptyrpt", "intemptyrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_empty_rpt_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM(" maskintemptyrpt", " maskintemptyrpt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bcap_overflow_int_status[]={
            BDMFMON_MAKE_PARM("l2_acc_idx", "l2_acc_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intl2sburstcapoverflow", "intl2sburstcapoverflow", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bcap_overflow_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskintl2sburstcapoverflow", "maskintl2sburstcapoverflow", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_dns_fault_int_status[]={
            BDMFMON_MAKE_PARM("intbbhdnsoverflow", "intbbhdnsoverflow", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_dns_fault_int_mask[]={
            BDMFMON_MAKE_PARM("maskintbbhdnsoverflow", "maskintbbhdnsoverflow", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_ups_fault_int_status[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intbbhupsfault", "intbbhupsfault", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_ups_fault_int_mask[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("maskintbbhupsfault", "maskintbbhupsfault", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_ups_abort_int_status[]={
            BDMFMON_MAKE_PARM("tardybbhabort", "tardybbhabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_ups_abort_int_mask[]={
            BDMFMON_MAKE_PARM("masktardybbhabort", "masktardybbhabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_main_int_mask[]={
            BDMFMON_MAKE_PARM("bbhupfrabortmask", "bbhupfrabortmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intl2sburstcapoverflowmask", "intl2sburstcapoverflowmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcoemptyrptmask", "intcoemptyrptmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intdrxerrabortmask", "intdrxerrabortmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intl2sfifooverrunmask", "intl2sfifooverrunmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intco1588tsmask", "intco1588tsmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcorptpresmask", "intcorptpresmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntpresmask", "intcogntpresmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcodelstalegntmask", "intcodelstalegntmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntnonpollmask", "intcogntnonpollmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntmisalignmask", "intcogntmisalignmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcognttoofarmask", "intcognttoofarmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntintervalmask", "intcogntintervalmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntdiscoverymask", "intcogntdiscoverymask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntmissabortmask", "intcogntmissabortmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcogntfullabortmask", "intcogntfullabortmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("badupfrlenmask", "badupfrlenmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("uptardypacketmask", "uptardypacketmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("uprptfrxmtmask", "uprptfrxmtmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intbififooverrunmask", "intbififooverrunmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("burstgnttoobigmask", "burstgnttoobigmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wrgnttoobigmask", "wrgnttoobigmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rcvgnttoobigmask", "rcvgnttoobigmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dnstatsoverrunmask", "dnstatsoverrunmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intupstatsoverrunmask", "intupstatsoverrunmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dnoutofordermask", "dnoutofordermask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("truantbbhhaltmask", "truantbbhhaltmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("upinvldgntlenmask", "upinvldgntlenmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("intcobbhupsfaultmask", "intcobbhupsfaultmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dntimeinsyncmask", "dntimeinsyncmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dntimenotinsyncmask", "dntimenotinsyncmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dportrdymask", "dportrdymask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_max_gnt_size[]={
            BDMFMON_MAKE_PARM("maxgntsize", "maxgntsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_max_frame_size[]={
            BDMFMON_MAKE_PARM("cfgmaxframesize", "cfgmaxframesize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grant_ovr_hd[]={
            BDMFMON_MAKE_PARM("gntovrhdfec", "gntovrhdfec", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gntovrhd", "gntovrhd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_poll_size[]={
            BDMFMON_MAKE_PARM("pollsizefec", "pollsizefec", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pollsize", "pollsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dn_rd_gnt_margin[]={
            BDMFMON_MAKE_PARM("rdgntstartmargin", "rdgntstartmargin", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_time_start_delta[]={
            BDMFMON_MAKE_PARM("gntstarttimedelta", "gntstarttimedelta", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_time_stamp_diff[]={
            BDMFMON_MAKE_PARM("timestampdiffdelta", "timestampdiffdelta", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_up_time_stamp_off[]={
            BDMFMON_MAKE_PARM("timestampoffsetfec", "timestampoffsetfec", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("timestampoffset", "timestampoffset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gnt_interval[]={
            BDMFMON_MAKE_PARM("gntinterval", "gntinterval", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dn_gnt_misalign_thr[]={
            BDMFMON_MAKE_PARM("prvunusedgntthreshold", "prvunusedgntthreshold", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gntmisalignthresh", "gntmisalignthresh", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dn_gnt_misalign_pause[]={
            BDMFMON_MAKE_PARM("gntmisalignpause", "gntmisalignpause", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_non_poll_intv[]={
            BDMFMON_MAKE_PARM("nonpollgntintv", "nonpollgntintv", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_force_fcs_err[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("forcefcserr", "forcefcserr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grant_overlap_limit[]={
            BDMFMON_MAKE_PARM("prvgrantoverlaplimit", "prvgrantoverlaplimit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_aes_configuration_0[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvupstreamaesmode_0", "prvupstreamaesmode_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_aes_configuration_1[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvupstreamaesmode_1", "prvupstreamaesmode_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_disc_grant_ovr_hd[]={
            BDMFMON_MAKE_PARM("discgntovrhd", "discgntovrhd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dn_discovery_seed[]={
            BDMFMON_MAKE_PARM("cfgdiscseed", "cfgdiscseed", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dn_discovery_inc[]={
            BDMFMON_MAKE_PARM("cfgdiscinc", "cfgdiscinc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dn_discovery_size[]={
            BDMFMON_MAKE_PARM("cfgdiscsize", "cfgdiscsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_prog_rpt_vec[]={
            BDMFMON_MAKE_PARM("progrptvec", "progrptvec", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fec_ipg_length[]={
            BDMFMON_MAKE_PARM("modipgpreamblebytes", "modipgpreamblebytes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgrptlen", "cfgrptlen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgfecrptlength", "cfgfecrptlength", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgfecipglength", "cfgfecipglength", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fake_report_value_en[]={
            BDMFMON_MAKE_PARM("fakereportvalueen", "fakereportvalueen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fake_report_value[]={
            BDMFMON_MAKE_PARM("fakereportvalue", "fakereportvalue", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_burst_cap[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("burstcap", "burstcap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_queue_llid_map[]={
            BDMFMON_MAKE_PARM("que_idx", "que_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("quellidmap", "quellidmap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_valid_opcode_map[]={
            BDMFMON_MAKE_PARM("prvvalidmpcpopcodes", "prvvalidmpcpopcodes", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_up_packet_tx_margin[]={
            BDMFMON_MAKE_PARM("uppackettxmargin", "uppackettxmargin", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_multi_pri_cfg_0[]={
            BDMFMON_MAKE_PARM("cfgctcschdeficiten", "cfgctcschdeficiten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvzeroburstcapoverridemode", "prvzeroburstcapoverridemode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgsharedl2", "cfgsharedl2", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgsharedburstcap", "cfgsharedburstcap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgrptgntsoutst0", "cfgrptgntsoutst0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgrpthiprifirst0", "cfgrpthiprifirst0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgrptswapqs0", "cfgrptswapqs0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgrptmultipri0", "cfgrptmultipri0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_forced_report_en[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgforcereporten", "cfgforcereporten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_forced_report_max_interval[]={
            BDMFMON_MAKE_PARM("cfgmaxreportinterval", "cfgmaxreportinterval", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_l2s_flush_config[]={
            BDMFMON_MAKE_PARM("cfgflushl2sen", "cfgflushl2sen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("flushl2sdone", "flushl2sdone", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgflushl2ssel", "cfgflushl2ssel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_data_port_command[]={
            BDMFMON_MAKE_PARM("dportbusy", "dportbusy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dportselect", "dportselect", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dportcontrol", "dportcontrol", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_data_port_address[]={
            BDMFMON_MAKE_PARM("dportaddr", "dportaddr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_data_port_data_0[]={
            BDMFMON_MAKE_PARM("dportdata0", "dportdata0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unmap_big_cnt[]={
            BDMFMON_MAKE_PARM("unmapbigerrcnt", "unmapbigerrcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unmap_frame_cnt[]={
            BDMFMON_MAKE_PARM("unmapfrcnt", "unmapfrcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unmap_fcs_cnt[]={
            BDMFMON_MAKE_PARM("unmapfcserrcnt", "unmapfcserrcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unmap_gate_cnt[]={
            BDMFMON_MAKE_PARM("unmapgatecnt", "unmapgatecnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unmap_oam_cnt[]={
            BDMFMON_MAKE_PARM("unmapoamcnt", "unmapoamcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unmap_small_cnt[]={
            BDMFMON_MAKE_PARM("unmapsmallerrcnt", "unmapsmallerrcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fif_dequeue_event_cnt[]={
            BDMFMON_MAKE_PARM("fifdequeueeventcnt", "fifdequeueeventcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unused_tq_cnt[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("unusedtqcnt", "unusedtqcnt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_up_fault_halt_en[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("bbhupsfaulthalten", "bbhupsfaulthalten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_up_tardy_halt_en[]={
            BDMFMON_MAKE_PARM("fataltardybbhaborten", "fataltardybbhaborten", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_l2s_ptr_sel[]={
            BDMFMON_MAKE_PARM("cfgl2sdebugptrsel", "cfgl2sdebugptrsel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("l2sdebugptrstate", "l2sdebugptrstate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_olt_mac_addr_lo[]={
            BDMFMON_MAKE_PARM("oltaddrlo", "oltaddrlo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_olt_mac_addr_hi[]={
            BDMFMON_MAKE_PARM("oltaddrhi", "oltaddrhi", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_l1s_shp_que_mask[]={
            BDMFMON_MAKE_PARM("shaper_idx", "shaper_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgshpmask", "cfgshpmask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_l2s_queue_config[]={
            BDMFMON_MAKE_PARM("que_idx", "que_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgl2squeend", "cfgl2squeend", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgl2squestart", "cfgl2squestart", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_ctc_burst_limit[]={
            BDMFMON_MAKE_PARM("l2_queue_idx", "l2_queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prvburstlimit", "prvburstlimit", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bbh_max_outstanding_tardy_packets[]={
            BDMFMON_MAKE_PARM("cfgmaxoutstandingtardypackets", "cfgmaxoutstandingtardypackets", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_min_report_value_difference[]={
            BDMFMON_MAKE_PARM("prvminreportdiff", "prvminreportdiff", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_spare_ctl[]={
            BDMFMON_MAKE_PARM("cfgepnspare", "cfgepnspare", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ts_sync_offset[]={
            BDMFMON_MAKE_PARM("cfgtssyncoffset", "cfgtssyncoffset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dn_ts_offset[]={
            BDMFMON_MAKE_PARM("cfgdntsoffset", "cfgdntsoffset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_up_ts_offset_lo[]={
            BDMFMON_MAKE_PARM("cfguptsoffset_lo", "cfguptsoffset_lo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_up_ts_offset_hi[]={
            BDMFMON_MAKE_PARM("cfguptsoffset_hi", "cfguptsoffset_hi", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_two_step_ts_ctl[]={
            BDMFMON_MAKE_PARM("twostepffrd", "twostepffrd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("twostepffentries", "twostepffentries", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_1588_timestamp_int_status[]={
            BDMFMON_MAKE_PARM("int1588pktabort", "int1588pktabort", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("int1588twostepffint", "int1588twostepffint", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_1588_timestamp_int_mask[]={
            BDMFMON_MAKE_PARM("ts1588pktabort_mask", "ts1588pktabort_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ts1588twostepff_mask", "ts1588twostepff_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_up_packet_fetch_margin[]={
            BDMFMON_MAKE_PARM("uppacketfetchmargin", "uppacketfetchmargin", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_persistent_report_cfg[]={
            BDMFMON_MAKE_PARM("cfgpersrptduration", "cfgpersrptduration", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgpersrptticksize", "cfgpersrptticksize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_persistent_report_enables[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgpersrptenable", "cfgpersrptenable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_persistent_report_request_size[]={
            BDMFMON_MAKE_PARM("cfgpersrptreqtq", "cfgpersrptreqtq", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="control_0", .val=BDMF_control_0, .parms=set_control_0 },
            { .name="control_1", .val=BDMF_control_1, .parms=set_control_1 },
            { .name="enable_grants", .val=BDMF_enable_grants, .parms=set_enable_grants },
            { .name="drop_disc_gates", .val=BDMF_drop_disc_gates, .parms=set_drop_disc_gates },
            { .name="dis_fcs_chk", .val=BDMF_dis_fcs_chk, .parms=set_dis_fcs_chk },
            { .name="pass_gates", .val=BDMF_pass_gates, .parms=set_pass_gates },
            { .name="cfg_misalgn_fb", .val=BDMF_cfg_misalgn_fb, .parms=set_cfg_misalgn_fb },
            { .name="discovery_filter", .val=BDMF_discovery_filter, .parms=set_discovery_filter },
            { .name="bbh_que_status_delay", .val=BDMF_bbh_que_status_delay, .parms=set_bbh_que_status_delay },
            { .name="minimum_grant_setup", .val=BDMF_minimum_grant_setup, .parms=set_minimum_grant_setup },
            { .name="reset_gnt_fifo", .val=BDMF_reset_gnt_fifo, .parms=set_reset_gnt_fifo },
            { .name="reset_l1_accumulator", .val=BDMF_reset_l1_accumulator, .parms=set_reset_l1_accumulator },
            { .name="l1_accumulator_sel", .val=BDMF_l1_accumulator_sel, .parms=set_l1_accumulator_sel },
            { .name="reset_rpt_pri", .val=BDMF_reset_rpt_pri, .parms=set_reset_rpt_pri },
            { .name="reset_l2_rpt_fifo", .val=BDMF_reset_l2_rpt_fifo, .parms=set_reset_l2_rpt_fifo },
            { .name="enable_upstream", .val=BDMF_enable_upstream, .parms=set_enable_upstream },
            { .name="enable_upstream_fec", .val=BDMF_enable_upstream_fec, .parms=set_enable_upstream_fec },
            { .name="report_byte_length", .val=BDMF_report_byte_length, .parms=set_report_byte_length },
            { .name="main_int_status", .val=BDMF_main_int_status, .parms=set_main_int_status },
            { .name="gnt_full_int_status", .val=BDMF_gnt_full_int_status, .parms=set_gnt_full_int_status },
            { .name="gnt_full_int_mask", .val=BDMF_gnt_full_int_mask, .parms=set_gnt_full_int_mask },
            { .name="gnt_miss_int_status", .val=BDMF_gnt_miss_int_status, .parms=set_gnt_miss_int_status },
            { .name="gnt_miss_int_mask", .val=BDMF_gnt_miss_int_mask, .parms=set_gnt_miss_int_mask },
            { .name="disc_rx_int_status", .val=BDMF_disc_rx_int_status, .parms=set_disc_rx_int_status },
            { .name="disc_rx_int_mask", .val=BDMF_disc_rx_int_mask, .parms=set_disc_rx_int_mask },
            { .name="gnt_intv_int_status", .val=BDMF_gnt_intv_int_status, .parms=set_gnt_intv_int_status },
            { .name="gnt_intv_int_mask", .val=BDMF_gnt_intv_int_mask, .parms=set_gnt_intv_int_mask },
            { .name="gnt_far_int_status", .val=BDMF_gnt_far_int_status, .parms=set_gnt_far_int_status },
            { .name="gnt_far_int_mask", .val=BDMF_gnt_far_int_mask, .parms=set_gnt_far_int_mask },
            { .name="gnt_misalgn_int_status", .val=BDMF_gnt_misalgn_int_status, .parms=set_gnt_misalgn_int_status },
            { .name="gnt_misalgn_int_mask", .val=BDMF_gnt_misalgn_int_mask, .parms=set_gnt_misalgn_int_mask },
            { .name="np_gnt_int_status", .val=BDMF_np_gnt_int_status, .parms=set_np_gnt_int_status },
            { .name="np_gnt_int_mask", .val=BDMF_np_gnt_int_mask, .parms=set_np_gnt_int_mask },
            { .name="del_stale_int_status", .val=BDMF_del_stale_int_status, .parms=set_del_stale_int_status },
            { .name="del_stale_int_mask", .val=BDMF_del_stale_int_mask, .parms=set_del_stale_int_mask },
            { .name="gnt_pres_int_status", .val=BDMF_gnt_pres_int_status, .parms=set_gnt_pres_int_status },
            { .name="gnt_pres_int_mask", .val=BDMF_gnt_pres_int_mask, .parms=set_gnt_pres_int_mask },
            { .name="rpt_pres_int_status", .val=BDMF_rpt_pres_int_status, .parms=set_rpt_pres_int_status },
            { .name="rpt_pres_int_mask", .val=BDMF_rpt_pres_int_mask, .parms=set_rpt_pres_int_mask },
            { .name="drx_abort_int_status", .val=BDMF_drx_abort_int_status, .parms=set_drx_abort_int_status },
            { .name="drx_abort_int_mask", .val=BDMF_drx_abort_int_mask, .parms=set_drx_abort_int_mask },
            { .name="empty_rpt_int_status", .val=BDMF_empty_rpt_int_status, .parms=set_empty_rpt_int_status },
            { .name="empty_rpt_int_mask", .val=BDMF_empty_rpt_int_mask, .parms=set_empty_rpt_int_mask },
            { .name="bcap_overflow_int_status", .val=BDMF_bcap_overflow_int_status, .parms=set_bcap_overflow_int_status },
            { .name="bcap_overflow_int_mask", .val=BDMF_bcap_overflow_int_mask, .parms=set_bcap_overflow_int_mask },
            { .name="bbh_dns_fault_int_status", .val=BDMF_bbh_dns_fault_int_status, .parms=set_bbh_dns_fault_int_status },
            { .name="bbh_dns_fault_int_mask", .val=BDMF_bbh_dns_fault_int_mask, .parms=set_bbh_dns_fault_int_mask },
            { .name="bbh_ups_fault_int_status", .val=BDMF_bbh_ups_fault_int_status, .parms=set_bbh_ups_fault_int_status },
            { .name="bbh_ups_fault_int_mask", .val=BDMF_bbh_ups_fault_int_mask, .parms=set_bbh_ups_fault_int_mask },
            { .name="bbh_ups_abort_int_status", .val=BDMF_bbh_ups_abort_int_status, .parms=set_bbh_ups_abort_int_status },
            { .name="bbh_ups_abort_int_mask", .val=BDMF_bbh_ups_abort_int_mask, .parms=set_bbh_ups_abort_int_mask },
            { .name="main_int_mask", .val=BDMF_main_int_mask, .parms=set_main_int_mask },
            { .name="max_gnt_size", .val=BDMF_max_gnt_size, .parms=set_max_gnt_size },
            { .name="max_frame_size", .val=BDMF_max_frame_size, .parms=set_max_frame_size },
            { .name="grant_ovr_hd", .val=BDMF_grant_ovr_hd, .parms=set_grant_ovr_hd },
            { .name="poll_size", .val=BDMF_poll_size, .parms=set_poll_size },
            { .name="dn_rd_gnt_margin", .val=BDMF_dn_rd_gnt_margin, .parms=set_dn_rd_gnt_margin },
            { .name="gnt_time_start_delta", .val=BDMF_gnt_time_start_delta, .parms=set_gnt_time_start_delta },
            { .name="time_stamp_diff", .val=BDMF_time_stamp_diff, .parms=set_time_stamp_diff },
            { .name="up_time_stamp_off", .val=BDMF_up_time_stamp_off, .parms=set_up_time_stamp_off },
            { .name="gnt_interval", .val=BDMF_gnt_interval, .parms=set_gnt_interval },
            { .name="dn_gnt_misalign_thr", .val=BDMF_dn_gnt_misalign_thr, .parms=set_dn_gnt_misalign_thr },
            { .name="dn_gnt_misalign_pause", .val=BDMF_dn_gnt_misalign_pause, .parms=set_dn_gnt_misalign_pause },
            { .name="non_poll_intv", .val=BDMF_non_poll_intv, .parms=set_non_poll_intv },
            { .name="force_fcs_err", .val=BDMF_force_fcs_err, .parms=set_force_fcs_err },
            { .name="grant_overlap_limit", .val=BDMF_grant_overlap_limit, .parms=set_grant_overlap_limit },
            { .name="aes_configuration_0", .val=BDMF_aes_configuration_0, .parms=set_aes_configuration_0 },
            { .name="aes_configuration_1", .val=BDMF_aes_configuration_1, .parms=set_aes_configuration_1 },
            { .name="disc_grant_ovr_hd", .val=BDMF_disc_grant_ovr_hd, .parms=set_disc_grant_ovr_hd },
            { .name="dn_discovery_seed", .val=BDMF_dn_discovery_seed, .parms=set_dn_discovery_seed },
            { .name="dn_discovery_inc", .val=BDMF_dn_discovery_inc, .parms=set_dn_discovery_inc },
            { .name="dn_discovery_size", .val=BDMF_dn_discovery_size, .parms=set_dn_discovery_size },
            { .name="prog_rpt_vec", .val=BDMF_prog_rpt_vec, .parms=set_prog_rpt_vec },
            { .name="fec_ipg_length", .val=BDMF_fec_ipg_length, .parms=set_fec_ipg_length },
            { .name="fake_report_value_en", .val=BDMF_fake_report_value_en, .parms=set_fake_report_value_en },
            { .name="fake_report_value", .val=BDMF_fake_report_value, .parms=set_fake_report_value },
            { .name="burst_cap", .val=BDMF_burst_cap, .parms=set_burst_cap },
            { .name="queue_llid_map", .val=BDMF_queue_llid_map, .parms=set_queue_llid_map },
            { .name="valid_opcode_map", .val=BDMF_valid_opcode_map, .parms=set_valid_opcode_map },
            { .name="up_packet_tx_margin", .val=BDMF_up_packet_tx_margin, .parms=set_up_packet_tx_margin },
            { .name="multi_pri_cfg_0", .val=BDMF_multi_pri_cfg_0, .parms=set_multi_pri_cfg_0 },
            { .name="forced_report_en", .val=BDMF_forced_report_en, .parms=set_forced_report_en },
            { .name="forced_report_max_interval", .val=BDMF_forced_report_max_interval, .parms=set_forced_report_max_interval },
            { .name="l2s_flush_config", .val=BDMF_l2s_flush_config, .parms=set_l2s_flush_config },
            { .name="data_port_command", .val=BDMF_data_port_command, .parms=set_data_port_command },
            { .name="data_port_address", .val=BDMF_data_port_address, .parms=set_data_port_address },
            { .name="data_port_data_0", .val=BDMF_data_port_data_0, .parms=set_data_port_data_0 },
            { .name="unmap_big_cnt", .val=BDMF_unmap_big_cnt, .parms=set_unmap_big_cnt },
            { .name="unmap_frame_cnt", .val=BDMF_unmap_frame_cnt, .parms=set_unmap_frame_cnt },
            { .name="unmap_fcs_cnt", .val=BDMF_unmap_fcs_cnt, .parms=set_unmap_fcs_cnt },
            { .name="unmap_gate_cnt", .val=BDMF_unmap_gate_cnt, .parms=set_unmap_gate_cnt },
            { .name="unmap_oam_cnt", .val=BDMF_unmap_oam_cnt, .parms=set_unmap_oam_cnt },
            { .name="unmap_small_cnt", .val=BDMF_unmap_small_cnt, .parms=set_unmap_small_cnt },
            { .name="fif_dequeue_event_cnt", .val=BDMF_fif_dequeue_event_cnt, .parms=set_fif_dequeue_event_cnt },
            { .name="unused_tq_cnt", .val=BDMF_unused_tq_cnt, .parms=set_unused_tq_cnt },
            { .name="bbh_up_fault_halt_en", .val=BDMF_bbh_up_fault_halt_en, .parms=set_bbh_up_fault_halt_en },
            { .name="bbh_up_tardy_halt_en", .val=BDMF_bbh_up_tardy_halt_en, .parms=set_bbh_up_tardy_halt_en },
            { .name="debug_l2s_ptr_sel", .val=BDMF_debug_l2s_ptr_sel, .parms=set_debug_l2s_ptr_sel },
            { .name="olt_mac_addr_lo", .val=BDMF_olt_mac_addr_lo, .parms=set_olt_mac_addr_lo },
            { .name="olt_mac_addr_hi", .val=BDMF_olt_mac_addr_hi, .parms=set_olt_mac_addr_hi },
            { .name="tx_l1s_shp_que_mask", .val=BDMF_tx_l1s_shp_que_mask, .parms=set_tx_l1s_shp_que_mask },
            { .name="tx_l2s_queue_config", .val=BDMF_tx_l2s_queue_config, .parms=set_tx_l2s_queue_config },
            { .name="tx_ctc_burst_limit", .val=BDMF_tx_ctc_burst_limit, .parms=set_tx_ctc_burst_limit },
            { .name="bbh_max_outstanding_tardy_packets", .val=BDMF_bbh_max_outstanding_tardy_packets, .parms=set_bbh_max_outstanding_tardy_packets },
            { .name="min_report_value_difference", .val=BDMF_min_report_value_difference, .parms=set_min_report_value_difference },
            { .name="spare_ctl", .val=BDMF_spare_ctl, .parms=set_spare_ctl },
            { .name="ts_sync_offset", .val=BDMF_ts_sync_offset, .parms=set_ts_sync_offset },
            { .name="dn_ts_offset", .val=BDMF_dn_ts_offset, .parms=set_dn_ts_offset },
            { .name="up_ts_offset_lo", .val=BDMF_up_ts_offset_lo, .parms=set_up_ts_offset_lo },
            { .name="up_ts_offset_hi", .val=BDMF_up_ts_offset_hi, .parms=set_up_ts_offset_hi },
            { .name="two_step_ts_ctl", .val=BDMF_two_step_ts_ctl, .parms=set_two_step_ts_ctl },
            { .name="1588_timestamp_int_status", .val=BDMF_1588_timestamp_int_status, .parms=set_1588_timestamp_int_status },
            { .name="1588_timestamp_int_mask", .val=BDMF_1588_timestamp_int_mask, .parms=set_1588_timestamp_int_mask },
            { .name="up_packet_fetch_margin", .val=BDMF_up_packet_fetch_margin, .parms=set_up_packet_fetch_margin },
            { .name="persistent_report_cfg", .val=BDMF_persistent_report_cfg, .parms=set_persistent_report_cfg },
            { .name="persistent_report_enables", .val=BDMF_persistent_report_enables, .parms=set_persistent_report_enables },
            { .name="persistent_report_request_size", .val=BDMF_persistent_report_request_size, .parms=set_persistent_report_request_size },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_epn_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_burst_cap[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_queue_llid_map[]={
            BDMFMON_MAKE_PARM("que_idx", "que_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unused_tq_cnt[]={
            BDMFMON_MAKE_PARM("link_idx", "link_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_l1s_shp_que_mask[]={
            BDMFMON_MAKE_PARM("shaper_idx", "shaper_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_l2s_queue_config[]={
            BDMFMON_MAKE_PARM("que_idx", "que_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_ctc_burst_limit[]={
            BDMFMON_MAKE_PARM("l2_queue_idx", "l2_queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="control_0", .val=BDMF_control_0, .parms=set_default },
            { .name="control_1", .val=BDMF_control_1, .parms=set_default },
            { .name="enable_grants", .val=BDMF_enable_grants, .parms=set_default },
            { .name="drop_disc_gates", .val=BDMF_drop_disc_gates, .parms=set_default },
            { .name="dis_fcs_chk", .val=BDMF_dis_fcs_chk, .parms=set_default },
            { .name="pass_gates", .val=BDMF_pass_gates, .parms=set_default },
            { .name="cfg_misalgn_fb", .val=BDMF_cfg_misalgn_fb, .parms=set_default },
            { .name="discovery_filter", .val=BDMF_discovery_filter, .parms=set_default },
            { .name="bbh_que_status_delay", .val=BDMF_bbh_que_status_delay, .parms=set_default },
            { .name="minimum_grant_setup", .val=BDMF_minimum_grant_setup, .parms=set_default },
            { .name="reset_gnt_fifo", .val=BDMF_reset_gnt_fifo, .parms=set_default },
            { .name="reset_l1_accumulator", .val=BDMF_reset_l1_accumulator, .parms=set_default },
            { .name="l1_accumulator_sel", .val=BDMF_l1_accumulator_sel, .parms=set_default },
            { .name="l1_sva_bytes", .val=BDMF_l1_sva_bytes, .parms=set_default },
            { .name="l1_uva_bytes", .val=BDMF_l1_uva_bytes, .parms=set_default },
            { .name="l1_sva_overflow", .val=BDMF_l1_sva_overflow, .parms=set_default },
            { .name="l1_uva_overflow", .val=BDMF_l1_uva_overflow, .parms=set_default },
            { .name="reset_rpt_pri", .val=BDMF_reset_rpt_pri, .parms=set_default },
            { .name="reset_l2_rpt_fifo", .val=BDMF_reset_l2_rpt_fifo, .parms=set_default },
            { .name="enable_upstream", .val=BDMF_enable_upstream, .parms=set_default },
            { .name="enable_upstream_fb", .val=BDMF_enable_upstream_fb, .parms=set_default },
            { .name="enable_upstream_fec", .val=BDMF_enable_upstream_fec, .parms=set_default },
            { .name="report_byte_length", .val=BDMF_report_byte_length, .parms=set_default },
            { .name="main_int_status", .val=BDMF_main_int_status, .parms=set_default },
            { .name="gnt_full_int_status", .val=BDMF_gnt_full_int_status, .parms=set_default },
            { .name="gnt_full_int_mask", .val=BDMF_gnt_full_int_mask, .parms=set_default },
            { .name="gnt_miss_int_status", .val=BDMF_gnt_miss_int_status, .parms=set_default },
            { .name="gnt_miss_int_mask", .val=BDMF_gnt_miss_int_mask, .parms=set_default },
            { .name="disc_rx_int_status", .val=BDMF_disc_rx_int_status, .parms=set_default },
            { .name="disc_rx_int_mask", .val=BDMF_disc_rx_int_mask, .parms=set_default },
            { .name="gnt_intv_int_status", .val=BDMF_gnt_intv_int_status, .parms=set_default },
            { .name="gnt_intv_int_mask", .val=BDMF_gnt_intv_int_mask, .parms=set_default },
            { .name="gnt_far_int_status", .val=BDMF_gnt_far_int_status, .parms=set_default },
            { .name="gnt_far_int_mask", .val=BDMF_gnt_far_int_mask, .parms=set_default },
            { .name="gnt_misalgn_int_status", .val=BDMF_gnt_misalgn_int_status, .parms=set_default },
            { .name="gnt_misalgn_int_mask", .val=BDMF_gnt_misalgn_int_mask, .parms=set_default },
            { .name="np_gnt_int_status", .val=BDMF_np_gnt_int_status, .parms=set_default },
            { .name="np_gnt_int_mask", .val=BDMF_np_gnt_int_mask, .parms=set_default },
            { .name="del_stale_int_status", .val=BDMF_del_stale_int_status, .parms=set_default },
            { .name="del_stale_int_mask", .val=BDMF_del_stale_int_mask, .parms=set_default },
            { .name="gnt_pres_int_status", .val=BDMF_gnt_pres_int_status, .parms=set_default },
            { .name="gnt_pres_int_mask", .val=BDMF_gnt_pres_int_mask, .parms=set_default },
            { .name="rpt_pres_int_status", .val=BDMF_rpt_pres_int_status, .parms=set_default },
            { .name="rpt_pres_int_mask", .val=BDMF_rpt_pres_int_mask, .parms=set_default },
            { .name="drx_abort_int_status", .val=BDMF_drx_abort_int_status, .parms=set_default },
            { .name="drx_abort_int_mask", .val=BDMF_drx_abort_int_mask, .parms=set_default },
            { .name="empty_rpt_int_status", .val=BDMF_empty_rpt_int_status, .parms=set_default },
            { .name="empty_rpt_int_mask", .val=BDMF_empty_rpt_int_mask, .parms=set_default },
            { .name="bcap_overflow_int_status", .val=BDMF_bcap_overflow_int_status, .parms=set_default },
            { .name="bcap_overflow_int_mask", .val=BDMF_bcap_overflow_int_mask, .parms=set_default },
            { .name="bbh_dns_fault_int_status", .val=BDMF_bbh_dns_fault_int_status, .parms=set_default },
            { .name="bbh_dns_fault_int_mask", .val=BDMF_bbh_dns_fault_int_mask, .parms=set_default },
            { .name="bbh_ups_fault_int_status", .val=BDMF_bbh_ups_fault_int_status, .parms=set_default },
            { .name="bbh_ups_fault_int_mask", .val=BDMF_bbh_ups_fault_int_mask, .parms=set_default },
            { .name="bbh_ups_abort_int_status", .val=BDMF_bbh_ups_abort_int_status, .parms=set_default },
            { .name="bbh_ups_abort_int_mask", .val=BDMF_bbh_ups_abort_int_mask, .parms=set_default },
            { .name="main_int_mask", .val=BDMF_main_int_mask, .parms=set_default },
            { .name="max_gnt_size", .val=BDMF_max_gnt_size, .parms=set_default },
            { .name="max_frame_size", .val=BDMF_max_frame_size, .parms=set_default },
            { .name="grant_ovr_hd", .val=BDMF_grant_ovr_hd, .parms=set_default },
            { .name="poll_size", .val=BDMF_poll_size, .parms=set_default },
            { .name="dn_rd_gnt_margin", .val=BDMF_dn_rd_gnt_margin, .parms=set_default },
            { .name="gnt_time_start_delta", .val=BDMF_gnt_time_start_delta, .parms=set_default },
            { .name="time_stamp_diff", .val=BDMF_time_stamp_diff, .parms=set_default },
            { .name="up_time_stamp_off", .val=BDMF_up_time_stamp_off, .parms=set_default },
            { .name="gnt_interval", .val=BDMF_gnt_interval, .parms=set_default },
            { .name="dn_gnt_misalign_thr", .val=BDMF_dn_gnt_misalign_thr, .parms=set_default },
            { .name="dn_gnt_misalign_pause", .val=BDMF_dn_gnt_misalign_pause, .parms=set_default },
            { .name="non_poll_intv", .val=BDMF_non_poll_intv, .parms=set_default },
            { .name="force_fcs_err", .val=BDMF_force_fcs_err, .parms=set_default },
            { .name="grant_overlap_limit", .val=BDMF_grant_overlap_limit, .parms=set_default },
            { .name="aes_configuration_0", .val=BDMF_aes_configuration_0, .parms=set_default },
            { .name="aes_configuration_1", .val=BDMF_aes_configuration_1, .parms=set_default },
            { .name="disc_grant_ovr_hd", .val=BDMF_disc_grant_ovr_hd, .parms=set_default },
            { .name="dn_discovery_seed", .val=BDMF_dn_discovery_seed, .parms=set_default },
            { .name="dn_discovery_inc", .val=BDMF_dn_discovery_inc, .parms=set_default },
            { .name="dn_discovery_size", .val=BDMF_dn_discovery_size, .parms=set_default },
            { .name="prog_rpt_vec", .val=BDMF_prog_rpt_vec, .parms=set_default },
            { .name="fec_ipg_length", .val=BDMF_fec_ipg_length, .parms=set_default },
            { .name="fake_report_value_en", .val=BDMF_fake_report_value_en, .parms=set_default },
            { .name="fake_report_value", .val=BDMF_fake_report_value, .parms=set_default },
            { .name="burst_cap", .val=BDMF_burst_cap, .parms=set_burst_cap },
            { .name="queue_llid_map", .val=BDMF_queue_llid_map, .parms=set_queue_llid_map },
            { .name="valid_opcode_map", .val=BDMF_valid_opcode_map, .parms=set_default },
            { .name="up_packet_tx_margin", .val=BDMF_up_packet_tx_margin, .parms=set_default },
            { .name="multi_pri_cfg_0", .val=BDMF_multi_pri_cfg_0, .parms=set_default },
            { .name="shared_bcap_ovrflow", .val=BDMF_shared_bcap_ovrflow, .parms=set_default },
            { .name="forced_report_en", .val=BDMF_forced_report_en, .parms=set_default },
            { .name="forced_report_max_interval", .val=BDMF_forced_report_max_interval, .parms=set_default },
            { .name="l2s_flush_config", .val=BDMF_l2s_flush_config, .parms=set_default },
            { .name="data_port_command", .val=BDMF_data_port_command, .parms=set_default },
            { .name="data_port_address", .val=BDMF_data_port_address, .parms=set_default },
            { .name="data_port_data_0", .val=BDMF_data_port_data_0, .parms=set_default },
            { .name="unmap_big_cnt", .val=BDMF_unmap_big_cnt, .parms=set_default },
            { .name="unmap_frame_cnt", .val=BDMF_unmap_frame_cnt, .parms=set_default },
            { .name="unmap_fcs_cnt", .val=BDMF_unmap_fcs_cnt, .parms=set_default },
            { .name="unmap_gate_cnt", .val=BDMF_unmap_gate_cnt, .parms=set_default },
            { .name="unmap_oam_cnt", .val=BDMF_unmap_oam_cnt, .parms=set_default },
            { .name="unmap_small_cnt", .val=BDMF_unmap_small_cnt, .parms=set_default },
            { .name="fif_dequeue_event_cnt", .val=BDMF_fif_dequeue_event_cnt, .parms=set_default },
            { .name="unused_tq_cnt", .val=BDMF_unused_tq_cnt, .parms=set_unused_tq_cnt },
            { .name="bbh_up_fault_halt_en", .val=BDMF_bbh_up_fault_halt_en, .parms=set_default },
            { .name="bbh_up_tardy_halt_en", .val=BDMF_bbh_up_tardy_halt_en, .parms=set_default },
            { .name="debug_status_0", .val=BDMF_debug_status_0, .parms=set_default },
            { .name="debug_status_1", .val=BDMF_debug_status_1, .parms=set_default },
            { .name="debug_l2s_ptr_sel", .val=BDMF_debug_l2s_ptr_sel, .parms=set_default },
            { .name="olt_mac_addr_lo", .val=BDMF_olt_mac_addr_lo, .parms=set_default },
            { .name="olt_mac_addr_hi", .val=BDMF_olt_mac_addr_hi, .parms=set_default },
            { .name="tx_l1s_shp_dqu_empty", .val=BDMF_tx_l1s_shp_dqu_empty, .parms=set_default },
            { .name="tx_l1s_unshaped_empty", .val=BDMF_tx_l1s_unshaped_empty, .parms=set_default },
            { .name="tx_l1s_shp_que_mask", .val=BDMF_tx_l1s_shp_que_mask, .parms=set_tx_l1s_shp_que_mask },
            { .name="tx_l2s_queue_config", .val=BDMF_tx_l2s_queue_config, .parms=set_tx_l2s_queue_config },
            { .name="tx_l2s_que_empty", .val=BDMF_tx_l2s_que_empty, .parms=set_default },
            { .name="tx_l2s_que_full", .val=BDMF_tx_l2s_que_full, .parms=set_default },
            { .name="tx_l2s_que_stopped", .val=BDMF_tx_l2s_que_stopped, .parms=set_default },
            { .name="tx_ctc_burst_limit", .val=BDMF_tx_ctc_burst_limit, .parms=set_tx_ctc_burst_limit },
            { .name="bbh_max_outstanding_tardy_packets", .val=BDMF_bbh_max_outstanding_tardy_packets, .parms=set_default },
            { .name="min_report_value_difference", .val=BDMF_min_report_value_difference, .parms=set_default },
            { .name="bbh_status_fifo_overflow", .val=BDMF_bbh_status_fifo_overflow, .parms=set_default },
            { .name="spare_ctl", .val=BDMF_spare_ctl, .parms=set_default },
            { .name="ts_sync_offset", .val=BDMF_ts_sync_offset, .parms=set_default },
            { .name="dn_ts_offset", .val=BDMF_dn_ts_offset, .parms=set_default },
            { .name="up_ts_offset_lo", .val=BDMF_up_ts_offset_lo, .parms=set_default },
            { .name="up_ts_offset_hi", .val=BDMF_up_ts_offset_hi, .parms=set_default },
            { .name="two_step_ts_ctl", .val=BDMF_two_step_ts_ctl, .parms=set_default },
            { .name="two_step_ts_value_lo", .val=BDMF_two_step_ts_value_lo, .parms=set_default },
            { .name="two_step_ts_value_hi", .val=BDMF_two_step_ts_value_hi, .parms=set_default },
            { .name="1588_timestamp_int_status", .val=BDMF_1588_timestamp_int_status, .parms=set_default },
            { .name="1588_timestamp_int_mask", .val=BDMF_1588_timestamp_int_mask, .parms=set_default },
            { .name="up_packet_fetch_margin", .val=BDMF_up_packet_fetch_margin, .parms=set_default },
            { .name="dn_1588_timestamp", .val=BDMF_dn_1588_timestamp, .parms=set_default },
            { .name="persistent_report_cfg", .val=BDMF_persistent_report_cfg, .parms=set_default },
            { .name="persistent_report_enables", .val=BDMF_persistent_report_enables, .parms=set_default },
            { .name="persistent_report_request_size", .val=BDMF_persistent_report_request_size, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_epn_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_epn_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="CONTROL_0" , .val=bdmf_address_control_0 },
            { .name="CONTROL_1" , .val=bdmf_address_control_1 },
            { .name="ENABLE_GRANTS" , .val=bdmf_address_enable_grants },
            { .name="DROP_DISC_GATES" , .val=bdmf_address_drop_disc_gates },
            { .name="DIS_FCS_CHK" , .val=bdmf_address_dis_fcs_chk },
            { .name="PASS_GATES" , .val=bdmf_address_pass_gates },
            { .name="CFG_MISALGN_FB" , .val=bdmf_address_cfg_misalgn_fb },
            { .name="DISCOVERY_FILTER" , .val=bdmf_address_discovery_filter },
            { .name="BBH_QUE_STATUS_DELAY" , .val=bdmf_address_bbh_que_status_delay },
            { .name="MINIMUM_GRANT_SETUP" , .val=bdmf_address_minimum_grant_setup },
            { .name="RESET_GNT_FIFO" , .val=bdmf_address_reset_gnt_fifo },
            { .name="RESET_L1_ACCUMULATOR" , .val=bdmf_address_reset_l1_accumulator },
            { .name="L1_ACCUMULATOR_SEL" , .val=bdmf_address_l1_accumulator_sel },
            { .name="L1_SVA_BYTES" , .val=bdmf_address_l1_sva_bytes },
            { .name="L1_UVA_BYTES" , .val=bdmf_address_l1_uva_bytes },
            { .name="L1_SVA_OVERFLOW" , .val=bdmf_address_l1_sva_overflow },
            { .name="L1_UVA_OVERFLOW" , .val=bdmf_address_l1_uva_overflow },
            { .name="RESET_RPT_PRI" , .val=bdmf_address_reset_rpt_pri },
            { .name="RESET_L2_RPT_FIFO" , .val=bdmf_address_reset_l2_rpt_fifo },
            { .name="ENABLE_UPSTREAM" , .val=bdmf_address_enable_upstream },
            { .name="ENABLE_UPSTREAM_FB" , .val=bdmf_address_enable_upstream_fb },
            { .name="ENABLE_UPSTREAM_FEC" , .val=bdmf_address_enable_upstream_fec },
            { .name="REPORT_BYTE_LENGTH" , .val=bdmf_address_report_byte_length },
            { .name="MAIN_INT_STATUS" , .val=bdmf_address_main_int_status },
            { .name="GNT_FULL_INT_STATUS" , .val=bdmf_address_gnt_full_int_status },
            { .name="GNT_FULL_INT_MASK" , .val=bdmf_address_gnt_full_int_mask },
            { .name="GNT_MISS_INT_STATUS" , .val=bdmf_address_gnt_miss_int_status },
            { .name="GNT_MISS_INT_MASK" , .val=bdmf_address_gnt_miss_int_mask },
            { .name="DISC_RX_INT_STATUS" , .val=bdmf_address_disc_rx_int_status },
            { .name="DISC_RX_INT_MASK" , .val=bdmf_address_disc_rx_int_mask },
            { .name="GNT_INTV_INT_STATUS" , .val=bdmf_address_gnt_intv_int_status },
            { .name="GNT_INTV_INT_MASK" , .val=bdmf_address_gnt_intv_int_mask },
            { .name="GNT_FAR_INT_STATUS" , .val=bdmf_address_gnt_far_int_status },
            { .name="GNT_FAR_INT_MASK" , .val=bdmf_address_gnt_far_int_mask },
            { .name="GNT_MISALGN_INT_STATUS" , .val=bdmf_address_gnt_misalgn_int_status },
            { .name="GNT_MISALGN_INT_MASK" , .val=bdmf_address_gnt_misalgn_int_mask },
            { .name="NP_GNT_INT_STATUS" , .val=bdmf_address_np_gnt_int_status },
            { .name="NP_GNT_INT_MASK" , .val=bdmf_address_np_gnt_int_mask },
            { .name="DEL_STALE_INT_STATUS" , .val=bdmf_address_del_stale_int_status },
            { .name="DEL_STALE_INT_MASK" , .val=bdmf_address_del_stale_int_mask },
            { .name="GNT_PRES_INT_STATUS" , .val=bdmf_address_gnt_pres_int_status },
            { .name="GNT_PRES_INT_MASK" , .val=bdmf_address_gnt_pres_int_mask },
            { .name="RPT_PRES_INT_STATUS" , .val=bdmf_address_rpt_pres_int_status },
            { .name="RPT_PRES_INT_MASK" , .val=bdmf_address_rpt_pres_int_mask },
            { .name="DRX_ABORT_INT_STATUS" , .val=bdmf_address_drx_abort_int_status },
            { .name="DRX_ABORT_INT_MASK" , .val=bdmf_address_drx_abort_int_mask },
            { .name="EMPTY_RPT_INT_STATUS" , .val=bdmf_address_empty_rpt_int_status },
            { .name="EMPTY_RPT_INT_MASK" , .val=bdmf_address_empty_rpt_int_mask },
            { .name="BCAP_OVERFLOW_INT_STATUS" , .val=bdmf_address_bcap_overflow_int_status },
            { .name="BCAP_OVERFLOW_INT_MASK" , .val=bdmf_address_bcap_overflow_int_mask },
            { .name="BBH_DNS_FAULT_INT_STATUS" , .val=bdmf_address_bbh_dns_fault_int_status },
            { .name="BBH_DNS_FAULT_INT_MASK" , .val=bdmf_address_bbh_dns_fault_int_mask },
            { .name="BBH_UPS_FAULT_INT_STATUS" , .val=bdmf_address_bbh_ups_fault_int_status },
            { .name="BBH_UPS_FAULT_INT_MASK" , .val=bdmf_address_bbh_ups_fault_int_mask },
            { .name="BBH_UPS_ABORT_INT_STATUS" , .val=bdmf_address_bbh_ups_abort_int_status },
            { .name="BBH_UPS_ABORT_INT_MASK" , .val=bdmf_address_bbh_ups_abort_int_mask },
            { .name="MAIN_INT_MASK" , .val=bdmf_address_main_int_mask },
            { .name="MAX_GNT_SIZE" , .val=bdmf_address_max_gnt_size },
            { .name="MAX_FRAME_SIZE" , .val=bdmf_address_max_frame_size },
            { .name="GRANT_OVR_HD" , .val=bdmf_address_grant_ovr_hd },
            { .name="POLL_SIZE" , .val=bdmf_address_poll_size },
            { .name="DN_RD_GNT_MARGIN" , .val=bdmf_address_dn_rd_gnt_margin },
            { .name="GNT_TIME_START_DELTA" , .val=bdmf_address_gnt_time_start_delta },
            { .name="TIME_STAMP_DIFF" , .val=bdmf_address_time_stamp_diff },
            { .name="UP_TIME_STAMP_OFF" , .val=bdmf_address_up_time_stamp_off },
            { .name="GNT_INTERVAL" , .val=bdmf_address_gnt_interval },
            { .name="DN_GNT_MISALIGN_THR" , .val=bdmf_address_dn_gnt_misalign_thr },
            { .name="DN_GNT_MISALIGN_PAUSE" , .val=bdmf_address_dn_gnt_misalign_pause },
            { .name="NON_POLL_INTV" , .val=bdmf_address_non_poll_intv },
            { .name="FORCE_FCS_ERR" , .val=bdmf_address_force_fcs_err },
            { .name="GRANT_OVERLAP_LIMIT" , .val=bdmf_address_grant_overlap_limit },
            { .name="AES_CONFIGURATION_0" , .val=bdmf_address_aes_configuration_0 },
            { .name="AES_CONFIGURATION_1" , .val=bdmf_address_aes_configuration_1 },
            { .name="DISC_GRANT_OVR_HD" , .val=bdmf_address_disc_grant_ovr_hd },
            { .name="DN_DISCOVERY_SEED" , .val=bdmf_address_dn_discovery_seed },
            { .name="DN_DISCOVERY_INC" , .val=bdmf_address_dn_discovery_inc },
            { .name="DN_DISCOVERY_SIZE" , .val=bdmf_address_dn_discovery_size },
            { .name="PROG_RPT_VEC" , .val=bdmf_address_prog_rpt_vec },
            { .name="FEC_IPG_LENGTH" , .val=bdmf_address_fec_ipg_length },
            { .name="FAKE_REPORT_VALUE_EN" , .val=bdmf_address_fake_report_value_en },
            { .name="FAKE_REPORT_VALUE" , .val=bdmf_address_fake_report_value },
            { .name="BURST_CAP_" , .val=bdmf_address_burst_cap_ },
            { .name="QUEUE_LLID_MAP_" , .val=bdmf_address_queue_llid_map_ },
            { .name="VALID_OPCODE_MAP" , .val=bdmf_address_valid_opcode_map },
            { .name="UP_PACKET_TX_MARGIN" , .val=bdmf_address_up_packet_tx_margin },
            { .name="MULTI_PRI_CFG_0" , .val=bdmf_address_multi_pri_cfg_0 },
            { .name="SHARED_BCAP_OVRFLOW" , .val=bdmf_address_shared_bcap_ovrflow },
            { .name="FORCED_REPORT_EN" , .val=bdmf_address_forced_report_en },
            { .name="FORCED_REPORT_MAX_INTERVAL" , .val=bdmf_address_forced_report_max_interval },
            { .name="L2S_FLUSH_CONFIG" , .val=bdmf_address_l2s_flush_config },
            { .name="DATA_PORT_COMMAND" , .val=bdmf_address_data_port_command },
            { .name="DATA_PORT_ADDRESS" , .val=bdmf_address_data_port_address },
            { .name="DATA_PORT_DATA_0" , .val=bdmf_address_data_port_data_0 },
            { .name="UNMAP_BIG_CNT" , .val=bdmf_address_unmap_big_cnt },
            { .name="UNMAP_FRAME_CNT" , .val=bdmf_address_unmap_frame_cnt },
            { .name="UNMAP_FCS_CNT" , .val=bdmf_address_unmap_fcs_cnt },
            { .name="UNMAP_GATE_CNT" , .val=bdmf_address_unmap_gate_cnt },
            { .name="UNMAP_OAM_CNT" , .val=bdmf_address_unmap_oam_cnt },
            { .name="UNMAP_SMALL_CNT" , .val=bdmf_address_unmap_small_cnt },
            { .name="FIF_DEQUEUE_EVENT_CNT" , .val=bdmf_address_fif_dequeue_event_cnt },
            { .name="UNUSED_TQ_CNT" , .val=bdmf_address_unused_tq_cnt },
            { .name="BBH_UP_FAULT_HALT_EN" , .val=bdmf_address_bbh_up_fault_halt_en },
            { .name="BBH_UP_TARDY_HALT_EN" , .val=bdmf_address_bbh_up_tardy_halt_en },
            { .name="DEBUG_STATUS_0" , .val=bdmf_address_debug_status_0 },
            { .name="DEBUG_STATUS_1" , .val=bdmf_address_debug_status_1 },
            { .name="DEBUG_L2S_PTR_SEL" , .val=bdmf_address_debug_l2s_ptr_sel },
            { .name="OLT_MAC_ADDR_LO" , .val=bdmf_address_olt_mac_addr_lo },
            { .name="OLT_MAC_ADDR_HI" , .val=bdmf_address_olt_mac_addr_hi },
            { .name="TX_L1S_SHP_DQU_EMPTY" , .val=bdmf_address_tx_l1s_shp_dqu_empty },
            { .name="TX_L1S_UNSHAPED_EMPTY" , .val=bdmf_address_tx_l1s_unshaped_empty },
            { .name="TX_L1S_SHP_QUE_MASK_" , .val=bdmf_address_tx_l1s_shp_que_mask_ },
            { .name="TX_L2S_QUE_CONFIG_" , .val=bdmf_address_tx_l2s_que_config_ },
            { .name="TX_L2S_QUE_EMPTY" , .val=bdmf_address_tx_l2s_que_empty },
            { .name="TX_L2S_QUE_FULL" , .val=bdmf_address_tx_l2s_que_full },
            { .name="TX_L2S_QUE_STOPPED" , .val=bdmf_address_tx_l2s_que_stopped },
            { .name="TX_CTC_BURST_LIMIT_" , .val=bdmf_address_tx_ctc_burst_limit_ },
            { .name="BBH_MAX_OUTSTANDING_TARDY_PACKETS" , .val=bdmf_address_bbh_max_outstanding_tardy_packets },
            { .name="MIN_REPORT_VALUE_DIFFERENCE" , .val=bdmf_address_min_report_value_difference },
            { .name="BBH_STATUS_FIFO_OVERFLOW" , .val=bdmf_address_bbh_status_fifo_overflow },
            { .name="SPARE_CTL" , .val=bdmf_address_spare_ctl },
            { .name="TS_SYNC_OFFSET" , .val=bdmf_address_ts_sync_offset },
            { .name="DN_TS_OFFSET" , .val=bdmf_address_dn_ts_offset },
            { .name="UP_TS_OFFSET_LO" , .val=bdmf_address_up_ts_offset_lo },
            { .name="UP_TS_OFFSET_HI" , .val=bdmf_address_up_ts_offset_hi },
            { .name="TWO_STEP_TS_CTL" , .val=bdmf_address_two_step_ts_ctl },
            { .name="TWO_STEP_TS_VALUE_LO" , .val=bdmf_address_two_step_ts_value_lo },
            { .name="TWO_STEP_TS_VALUE_HI" , .val=bdmf_address_two_step_ts_value_hi },
            { .name="1588_TIMESTAMP_INT_STATUS" , .val=bdmf_address_1588_timestamp_int_status },
            { .name="1588_TIMESTAMP_INT_MASK" , .val=bdmf_address_1588_timestamp_int_mask },
            { .name="UP_PACKET_FETCH_MARGIN" , .val=bdmf_address_up_packet_fetch_margin },
            { .name="DN_1588_TIMESTAMP" , .val=bdmf_address_dn_1588_timestamp },
            { .name="PERSISTENT_REPORT_CFG" , .val=bdmf_address_persistent_report_cfg },
            { .name="PERSISTENT_REPORT_ENABLES" , .val=bdmf_address_persistent_report_enables },
            { .name="PERSISTENT_REPORT_REQUEST_SIZE" , .val=bdmf_address_persistent_report_request_size },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_epn_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

