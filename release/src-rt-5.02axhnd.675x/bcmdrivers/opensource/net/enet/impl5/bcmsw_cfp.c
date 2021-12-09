/*
    <:copyright-BRCM:2016:DUAL/GPL:standard    
    
       Copyright (c) 2016 Broadcom 
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

#define _BCMENET_LOCAL_

#include "bcm_OS_Deps.h"
#include "board.h"
#include "spidevices.h"
#include <bcm_map_part.h>
#include "bcm_intr.h"
#include "bcmmii.h"
#include "ethsw_phy.h"
#include "bcmswdefs.h"
#include "bcmenet.h"
#include "bcmsw.h"
/* Exports for other drivers */
#include "bcmsw_api.h"
#include "bcmswshared.h"
#include "bcmPktDma_defines.h"
#include "boardparms.h"
#if defined(CONFIG_BCM_GMAC)
#include "bcmgmac.h"
#endif
#include "bcmswaccess.h"
#include "eth_pwrmngt.h"
#include "bcmsw_cfp.h"

#define UDF_TOTAL_ENTRIES (CFP_L3_FRAME_TYPES*CFP_SLICES + 1)
udfCtl_t udfCtls[UDF_TOTAL_ENTRIES];    /* UDF Control Structrure */
cfpTcamCtl_t tcamCtls[MAX_CFP_ENTRIES];  /* CFP T-CAM Control Structure */
cfpCtl_t cfpCtl;
#define CFP_FIRST_TCAM_INDEX    0

static int cfp_find_empty_tcam(void)
{
    int i;

    for (i=CFP_FIRST_TCAM_INDEX; i<MAX_CFP_ENTRIES; i++)
    {
        if (tcamCtls[i].argFlag == 0) return i;
    }
    return -1;
}

static int cfp_find_empty_udf(udfCtl_t *curUdfCtl)
{
    int i;

    for (i=0; i<curUdfCtl->maxCnt; i++)
    {
        if (curUdfCtl->useMap & (1<<i)) continue;
        return i;
    }
    return -1;
}

/* Find the matched udf from udf control structure */
static int cfp_find_udf(udf_t *udf, udfCtl_t *curUdfCtl)
{
    int i;

    for (i=0; i<curUdfCtl->maxCnt; i++)
    {
        if ((curUdfCtl->useMap & (1<<i)) == 0) continue;
        if (curUdfCtl->udfDsc[i].udf.udf == udf->udf) return i;
    }
    return -1;
}

/*
    Get one pair UDF values and mask from the index of T-cam data and T-Cam mask 
    idx: UDF index inside *_tcam, *tcamMask; 
    *val, *mask: 2 byte UDF values to be returned from TCAM
*/
static int cfp_get_udf_val(int idx, void *_tcam, void *_tcamMask, u16 *val, u16 *mask)
{
    /* Use NoIP TCAM Structure for common UDF access among all different L3 types */
    cfpNoIpTcam_t *tcam = _tcam, *tcamMask = _tcamMask;


    switch (idx)
    {
        case 0: 
            *val = tcam->udf_n_x0;
            *mask = tcamMask->udf_n_x0;
            break;
        case 1:
            *val = (tcam->udf_n_x1_h << 8)|(tcam->udf_n_x1_l);
            *mask = (tcamMask->udf_n_x1_h << 8)|(tcamMask->udf_n_x1_l);
            break;
        case 2: 
            *val = tcam->udf_n_x2;
            *mask = tcamMask->udf_n_x2;
            break;
        case 3:
            *val = (tcam->udf_n_x3_h << 8)|(tcam->udf_n_x3_l);
            *mask = (tcamMask->udf_n_x3_h << 8)|(tcamMask->udf_n_x3_l);
            break;
        case 4: 
            *val = tcam->udf_n_x4;
            *mask = tcamMask->udf_n_x4;
            break;
        case 5:
            *val = (tcam->udf_n_x5_h << 8)|(tcam->udf_n_x5_l);
            *mask = (tcamMask->udf_n_x5_h << 8)|(tcamMask->udf_n_x5_l);
            break;
        case 6: 
            *val = tcam->udf_n_x6;
            *mask = tcamMask->udf_n_x6;
            break;
        case 7:
            *val = (tcam->udf_n_x7_h << 8)|tcam->udf_n_x7_l;
            *mask = (tcamMask->udf_n_x7_h << 8)|tcamMask->udf_n_x7_l;
            break;
        case 8: 
            *val = tcam->udf_n_x8;
            *mask = tcamMask->udf_n_x8;
            break;
    }

    return 1;
}

/* 
    Set UDF values of an index in T-CAM Data and T-CAM Mask
    idx: UDF index to be set
*/
static int cfp_set_udf_val(int idx, void *_tcam, void *_tcamMask, u16 val, u16 mask)
{
    cfpNoIpTcam_t *tcam = _tcam, *tcamMask = _tcamMask;

    if (mask == 0) return 1;
    switch (idx)
    {
        case 0:
            tcam->udf_n_x0 = val;
            tcamMask->udf_n_x0 = mask;
            break;
        case 1:
            tcam->udf_n_x1_l = val & 0xff;
            tcam->udf_n_x1_h = val >> 8;
            tcamMask->udf_n_x1_l = mask & 0xff;
            tcamMask->udf_n_x1_h = mask >> 8;
            break;
        case 2:
            tcam->udf_n_x2 = val;
            tcamMask->udf_n_x2 = mask;
            break;
        case 3:
            tcam->udf_n_x3_l = val & 0xff;
            tcam->udf_n_x3_h = val >> 8;
            tcamMask->udf_n_x3_l = mask & 0xff;
            tcamMask->udf_n_x3_h = mask >> 8;
            break;
        case 4:
            tcam->udf_n_x4 = val;
            tcamMask->udf_n_x4 = mask;
            break;
        case 5:
            tcam->udf_n_x5_l = val & 0xff;
            tcam->udf_n_x5_h = val >> 8;
            tcamMask->udf_n_x5_l = mask & 0xff;
            tcamMask->udf_n_x5_h = mask >> 8;
            break;
        case 6:
            tcam->udf_n_x6 = val;
            tcamMask->udf_n_x6 = mask;
            break;
        case 7:
            tcam->udf_n_x7_l = val & 0xff;
            tcam->udf_n_x7_h = val >> 8;
            tcamMask->udf_n_x7_l = mask & 0xff;
            tcamMask->udf_n_x7_h = mask >> 8;
            break;
        case 8:
            tcam->udf_n_x8 = val;
            tcamMask->udf_n_x8 = mask;
            break;
    }

    if (idx != 8)
    {
        tcam->udf_valid_0_7 |= (1<<idx);
        tcamMask->udf_valid_0_7 |= (1<<idx);
    }
    else
    {
        tcam->udf_valid_8 = 1;
        tcamMask->udf_valid_8 = 1;
    }
    return 1;
}

/*
    CFP processing structure collecting all pieces.
*/
typedef struct cfpParm_s {
    cfpTcam_t tcam, tcamMask; /* TCAM-Data and TCAM-Mask */
    cfpTcamCtl_t *tcamCtl;      /* TCAM Control structure */
    cfpArg_t *cfpArg;           /* Argument structure */
    udfCtl_t *curUdfCtl;        /* Current UDF control structure */
    udfCtl_t *udfPatLst[CFP_MAX_UDF_FIELDS_D]; /* UDF pattern list corresponding to the argment */
    cfpRateCtl_t rateCtl;
} cfpParm_t;

/* Use unsigned long to apply different size in 32 and 64 bit environment */
#define STU_OFF(type, member) (unsigned long)(&((type *)0)->member)  

/* UDF patterns for predefined packet fields */
udfCtl_t udfPats[] = 
{
    {{{{{0, UDF_START_OF_PACKET}}}, {{{1, UDF_START_OF_PACKET}}}, {{{2, UDF_START_OF_PACKET}}}}, {STU_OFF(cfpArg_t, da)}, {8}, 3}, 
    {{{{{3, UDF_START_OF_PACKET}}}, {{{4, UDF_START_OF_PACKET}}}, {{{5, UDF_START_OF_PACKET}}}}, {STU_OFF(cfpArg_t, sa)}, {8}, 3}, 
    {{{{{0, UDF_END_OF_L2_HEADER}},0,0xff}}, {STU_OFF(cfpArg_t, dscp)}, {4}, 1}, 
    {{{{{4, UDF_END_OF_L2_HEADER}},0,0xff}}, {STU_OFF(cfpArg_t, ip_protocol)}, {4}, 1}, 

    {{{{{6, UDF_END_OF_L2_HEADER}}}, {{{7, UDF_END_OF_L2_HEADER}}}}, {STU_OFF(cfpArg_t, ipsa)}, {4}, 2}, 
    {{{{{8, UDF_END_OF_L2_HEADER}}}, {{{9, UDF_END_OF_L2_HEADER}}}}, {STU_OFF(cfpArg_t, ipda)}, {4}, 2}, 
    {{{{{0, UDF_END_OF_L3_HEADER}}}}, {STU_OFF(cfpArg_t, tcpudp_sport)}, {4}, 1}, 
    {{{{{1, UDF_END_OF_L3_HEADER}}}}, {STU_OFF(cfpArg_t, tcpudp_dport)}, {4}, 1}, 
};

enum {
    UDF_PAT_DA, UDF_PAT_SA, UDF_PAT_DSCP, UDF_PAT_IP_PROTOCOL,
    UDF_PAT_IPSA, UDF_PAT_IPDA, UDF_PAT_TCPUDP_SPORT, UDF_PAT_TCPUDP_DPORT,
};

/*
    from UDF Pattern list, get UDF map from current UDF Control structure
    The patter list member should have been added to current UDF Control already.
*/
static int cfp_find_udf_map(udfCtl_t *udfPat, udfCtl_t *udfCtl)
{
    int i, j, udfMap = 0;

    for (i=0, j=0; i<udfPat->usedCnt; i++)
    {
        for (; j<CFP_MAX_UDF_FIELDS_D; j++) /* udfCtl could have non contious bit map */
        {
            if ((udfCtl->useMap & (1<<j)) == 0) continue;
            if (udfPat->udfDsc[i].udf.udf == udfCtl->udfDsc[j].udf.udf)
            {
                udfMap |= (1<<j);
                break;
            }
        }
    }
    return udfMap;
}

/*
    static int cfp_add_tcam_udf(cfpParm_t *cfpParm)
    Set UDF values in TCAM from UDF control structure
*/
static int cfp_add_tcam_udf(cfpParm_t *cfpParm)
{
    int i, j, k, dwIdx, udfMap;
    udfCtl_t *udfPat;
    u32 *v32;
    u64 *v64;

    for (k=0, udfPat = cfpParm->udfPatLst[k]; udfPat; udfPat=cfpParm->udfPatLst[++k])
    {
        udfMap = cfp_find_udf_map(udfPat, cfpParm->curUdfCtl);
        if (udfPat->memberSize == sizeof(u32))
        {
            v32 = (u32 *)((char *)cfpParm->cfpArg + udfPat->argOffset);
            for (j=CFP_MAX_UDF_FIELDS_D, i=0, dwIdx=0; j>=0; j--)
            {
                if ((udfMap & (1<<j)) == 0) continue;
                /* 
                    Note: The values are all held in host byte endian instead of Network endian,
                    so we need to shift word from high end to low end ie. in network byte order.
                */
                cfp_set_udf_val(j, &cfpParm->tcam, &cfpParm->tcamMask, (*v32>>(16*dwIdx))&0xffff, 
                        (*(v32+1)>>(16*dwIdx))&0xffff & udfPat->udfDsc[i++].mask);
                dwIdx++;
            }
        }
        else /* for u64 */
        {
            v64 = (u64 *)((char *)cfpParm->cfpArg + udfPat->argOffset);
            for (j=CFP_MAX_UDF_FIELDS_D, i=0, dwIdx=0; j>=0; j--)
            {
                if ((udfMap & (1<<j)) == 0) continue;
                /* 
                    Note: The values are all held in host byte endian instead of Network endian,
                    so we need to shift word from high end to low end ie. in network byte order.
                */
                cfp_set_udf_val(j, &cfpParm->tcam, &cfpParm->tcamMask, (*v64>>(16*dwIdx))&0xffff, 
                        (*(v64+1)>>(16*dwIdx))&0xffff & udfPat->udfDsc[i++].mask);
                dwIdx++;
            }
        }
    }

    return 1;
}

/*
    Allocate new UDF Fields from current UDF control Structure curUdfCtl
    udfPatLst[]: Temporary UDF list for new addition
    curUdfCtl: Current UDF control structure pointed by T-CAM
 */
static int cfp_alloc_udf(udfCtl_t *udfPatLst[], udfCtl_t *curUdfCtl)
{
    int i, j, k, regBase, update = 0;
    udfDsc_t *udfPatDsc, *curUdfDsc;
    udfCtl_t *udfPat;

    for (j=0, udfPat = udfPatLst[j]; udfPat; udfPat=udfPatLst[++j])
    {
        for (i=0; i<udfPat->usedCnt; i++)
        {
            udfPatDsc = &udfPat->udfDsc[i];
            if ((k = cfp_find_udf(&udfPatDsc->udf, curUdfCtl)) != -1)
            {
                /* This UDF exists in current UDF, increase the refcnt */
                curUdfDsc = &curUdfCtl->udfDsc[k];
                curUdfDsc->udf.udf = udfPatDsc->udf.udf;
                curUdfDsc->refCnt++;
            }
            else
            {
                /* Need to add a new UDF */
                k = cfp_find_empty_udf(curUdfCtl);
                curUdfDsc = &curUdfCtl->udfDsc[k];
                curUdfDsc->udf.udf = udfPatDsc->udf.udf;
                curUdfDsc->refCnt = 1;
                curUdfCtl->useMap |= (1<<k);
                curUdfCtl->usedCnt++;
                curUdfCtl->avaCnt--;
                update = 1;
            }
        }
    }

    if (update) /* Need to update UDF hardware */
    {
        regBase = CFP_UDF_REG + 0x10*(curUdfCtl->l3framing*3+curUdfCtl->sliceId); 

        for (i=0; i< CFP_MAX_UDF_FIELDS_D; i++)
        {
            if ((curUdfCtl->useMap & (1<<i)) == 0) continue;
            extsw_wreg_wrap(PAGE_CFP_CONFIG, regBase + i, &curUdfCtl->udfDsc[i].udf.udf, 1);
        }
    }
    return 1;
}

/*
    cfp_free_udf()
    freUdfCtl: Temporary UDF list to be freed.
    curUdfCtl: Current UDF pointed by T-CAM
*/
static int cfp_free_udf(udfCtl_t *udfPatLst[], udfCtl_t *curUdfCtl)
{
    int i, j, k;
    udfCtl_t *udfPat;
    udfDsc_t *freUdfDsc, *curUdfDsc;

    for (i=0, udfPat = udfPatLst[i]; udfPat; udfPat=udfPatLst[++i])
    {
        for (k=0; k<udfPat->usedCnt; k++)
        {
            freUdfDsc = &udfPat->udfDsc[k];
            if ((j = cfp_find_udf(&freUdfDsc->udf, curUdfCtl)) == -1)
            {
                printk("***** Error: Should not happend\n");
                continue;
            }

            curUdfDsc = &curUdfCtl->udfDsc[j];
            if (--curUdfDsc->refCnt == 0)
            {
                curUdfCtl->usedCnt--;
                curUdfCtl->useMap &= ~(1<<j);
                curUdfCtl->avaCnt++;
            }
        }
    }

    return 1;
}

/* 
   Get the new UDF fields needed based on new UDF list and current UDF control structure
 */
static int cfp_get_new_udf_fields(udfCtl_t *udfPatLst[], udfCtl_t *curUdfCtl)
{
    int i, j, n;
    udfCtl_t *udfPat;

    for (n=0, j=0, udfPat = udfPatLst[j]; udfPat; udfPat=udfPatLst[++j])
    {
        for (i=0; i<udfPat->usedCnt; i++)
        {
            if (cfp_find_udf(&udfPat->udfDsc[i].udf, curUdfCtl) != -1) continue;
            n++;
        }
    }
    return n;
}

static int cfp_init(void)
{
    int i, j;
    u32 v32;

    for (i = 0; i<ARRAY_SIZE(tcamCtls); i++)
    {
        tcamCtls[i].tidx = i;
    }

    for (i=0; i<UDF_TOTAL_ENTRIES; i++)
    {
        if (i==UDF_TOTAL_ENTRIES-1)
        {
            udfCtls[i].maxCnt = udfCtls[i].avaCnt = CFP_MAX_UDF_FIELDS_A_C;
        }
        else
        {
            udfCtls[i].maxCnt = udfCtls[i].avaCnt = CFP_MAX_UDF_FIELDS_D;
        }
        udfCtls[i].l3framing = (i/3);
        udfCtls[i].sliceId = (i%3);
    }

    for(i=0; i<ARRAY_SIZE(udfPats); i++)
    {
        udfPats[i].patIdx = i;
        for (j=0; j<udfPats[i].usedCnt; j++)
        {
            if(udfPats[i].udfDsc[j].mask == 0) udfPats[i].udfDsc[j].mask = 0xffff;
        }
    }

    /* Reset CFP */
    v32 = REG_CFPACC_TCAMRESET|CFPACC_RAM_CLEAR;
    extsw_wreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    do
    {
        extsw_rreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    } while(v32 & (REG_CFPACC_TCAMRESET|CFPACC_RAM_CLEAR));

    return 1;
}

/* 
   static int cfp_build_udf_list(udfCtl_t *udfPatLst, cfpArg_t *cfpArg)
   Build UDF List from command arguments
*/
static int cfp_build_udf_list(udfCtl_t *udfPatLst[], cfpArg_t *cfpArg)
{
    int i, idx, udfMask;

    udfMask = cfpArg->argFlag & CFP_UDF_FLAG;
    for (i=0, idx=0; i<sizeof(udfMask)*8; i++)
    {
        if (((1<<i) & udfMask) == 0) continue;
        switch (1<<i)
        {
            case CFP_ARG_DA_M:
                udfPatLst[idx++] = &udfPats[UDF_PAT_DA];
                break;
            case CFP_ARG_SA_M:
                udfPatLst[idx++] = &udfPats[UDF_PAT_SA];
                break;
            case CFP_ARG_DSCP_M:
                udfPatLst[idx++] = &udfPats[UDF_PAT_DSCP];
                break;
            case CFP_ARG_IP_PROTOCOL_M:
                udfPatLst[idx++] = &udfPats[UDF_PAT_IP_PROTOCOL];
                break;
            case CFP_ARG_IPSA_M:
                udfPatLst[idx++] = &udfPats[UDF_PAT_IPSA];
                break;
            case CFP_ARG_IPDA_M:
                udfPatLst[idx++] = &udfPats[UDF_PAT_IPDA];
                break;
            case CFP_ARG_TCPUDP_SPORT_M:
                udfPatLst[idx++] = &udfPats[UDF_PAT_TCPUDP_SPORT];
                break;
            case CFP_ARG_TCPUDP_DPORT_M:
                udfPatLst[idx++] = &udfPats[UDF_PAT_TCPUDP_DPORT];
                break;
        }
    }
    udfPatLst[idx] = 0;
    return 1;
}

enum {UDF_CHECK, UDF_ADD, UDF_DEL};

static int cfp_udf_op(cfpParm_t *cfpParm, int op)
{
    int rc = -1, newUdfFields;

    switch (op)
    {
        case UDF_CHECK:
            newUdfFields = cfp_get_new_udf_fields(cfpParm->udfPatLst, cfpParm->curUdfCtl); 
            if (newUdfFields > cfpParm->curUdfCtl->avaCnt) goto end;
            rc = 1;
            break;
        case UDF_ADD:
            cfp_alloc_udf(cfpParm->udfPatLst, cfpParm->curUdfCtl);
            rc = cfp_add_tcam_udf(cfpParm);
            break;

        case UDF_DEL:
            rc = cfp_free_udf(cfpParm->udfPatLst, cfpParm->curUdfCtl);
            break;
    }

    rc = 1;
end:
    return rc;
}

/* Write CFP Action RAM */
static int cfp_read_action_ram(u32 actData[], int tcIdx)
{
    u32 v32;

    do
    {
        extsw_rreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    } while((v32 & CFPACC_OP_START_DONE));

    v32 = (tcIdx<<REG_CFPACC_XCESS_ADDR_S)|CFPACC_ACTION_RAM_SEL|CFPACC_OP_READ|CFPACC_OP_START_DONE;
    extsw_wreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    do
    {
        extsw_rreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    } while((v32 & CFPACC_READ_ST_ACTION_RAM) == 0);

    extsw_rreg_wrap(PAGE_CFP, REG_CFP_ACT_DATA0, &actData[0], sizeof(actData[0]));
    extsw_rreg_wrap(PAGE_CFP, REG_CFP_ACT_DATA1, &actData[1], sizeof(actData[1]));

    return 1;
}

static int cfp_write_action_ram(u32 actData[], int tcIdx)
{
    int v32;

    do
    {
        extsw_rreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    } while((v32 & CFPACC_OP_START_DONE));

    extsw_wreg_wrap(PAGE_CFP, REG_CFP_ACT_DATA0, &actData[0], sizeof(actData[0]));
    extsw_wreg_wrap(PAGE_CFP, REG_CFP_ACT_DATA1, &actData[1], sizeof(actData[1]));

    v32 = (tcIdx<<REG_CFPACC_XCESS_ADDR_S)|CFPACC_ACTION_RAM_SEL|CFPACC_OP_WRITE|CFPACC_OP_START_DONE;
    extsw_wreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));

    return 1;
}

static int cfp_act_op (cfpArg_t *cfpArg, int tcIdx)
{
    uint32_t aramData[2];;

    aramData[0] = aramData[1] = 0;
    if (cfpArg->argFlag & CFP_ARG_NEW_DSCP_IB_M)
    {
        aramData[0] |= (cfpArg->new_dscp_ib << CFP_NEW_DSCP_IB_S);
        aramData[1] |= CFP_CHG_DSCP_IB;
    }

    if (cfpArg->argFlag & CFP_ARG_FPMAP_IB_M)
    {
        if ((cfpArg->argFlag & CFP_ARG_CHG_FPMAP_IB_M) == 0) cfpArg->chg_fpmap_ib = CFP_CHG_FPMAP_RPL_ARL;
        aramData[0] |= (SF2_LOG_TO_CHIP_PMAP(cfpArg->fpmap_ib) << CFP_FPMAP_IB_S) & CFP_FPMAP_IB_M;
        aramData[0] |= (cfpArg->chg_fpmap_ib << CFP_CHG_FPMAP_IB_S) & CFP_CHG_FPMAP_IB_M;
    }
    cfp_write_action_ram(aramData, tcIdx);
    return 1;
}

static int cfp_disable_tcam(int tcIdx)
{
    int v32 = 0;
    do
    {
        extsw_rreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    } while((v32 & CFPACC_OP_START_DONE));

    extsw_wreg_wrap(PAGE_CFP, REG_CFP_DATA, &v32, sizeof(v32));

    v32 = (tcIdx<<REG_CFPACC_XCESS_ADDR_S)|CFPACC_TCAM_SEL|CFPACC_OP_WRITE|CFPACC_OP_START_DONE;
    extsw_wreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    return 1;
}

/*
    Read TCAM registers values into memory TCAM structure
*/
static int cfp_read_tcam(cfpTcam_t *tcam, cfpTcam_t *tcamMask, int tcIdx)
{
    u32 i, dw, dwm, v32;

    do
    {
        extsw_rreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    } while((v32 & CFPACC_OP_START_DONE));

    v32 = (tcIdx<<REG_CFPACC_XCESS_ADDR_S)|CFPACC_TCAM_SEL|CFPACC_OP_READ|CFPACC_OP_START_DONE;
    extsw_wreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    do
    {
        extsw_rreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));

    } while((v32 & CFPACC_READ_ST_TCAM) == 0);

    for (i=0; i<sizeof(*tcam)/4; i++, dw++, dwm++)
    {
        extsw_rreg_wrap(PAGE_CFP, REG_CFP_DATA + (i*4), &dw, sizeof(dw));
        extsw_rreg_wrap(PAGE_CFP, REG_CFP_MASK + (i*4), &dwm, sizeof(dw));
        
        *((u32 *)tcam + i) = __cpu_to_le32(dw);
        *((u32 *)tcamMask + i) = __cpu_to_le32(dwm);
    }
    return 1;
}

/*
    Write memory TCAM structure to TCAM registers
*/
static int cfp_write_tcam(cfpTcam_t *tcam, cfpTcam_t *tcamMask, int tcIdx)
{
    u32 i, dw, dwm, v32;

    do
    {
        extsw_rreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    } while((v32 & CFPACC_OP_START_DONE));

    for (i=0; i<sizeof(*tcam)/4; i++, dw++, dwm++)
    {
        dw = __le32_to_cpu(*((u32 *)tcam + i));
        dwm = __le32_to_cpu(*((u32 *)tcamMask + i));
        extsw_wreg_wrap(PAGE_CFP, REG_CFP_DATA + (i*4), &dw, sizeof(dw));
        extsw_wreg_wrap(PAGE_CFP, REG_CFP_MASK + (i*4), &dwm, sizeof(dw));
    }

    v32 = (tcIdx<<REG_CFPACC_XCESS_ADDR_S)|CFPACC_TCAM_SEL|CFPACC_OP_WRITE|CFPACC_OP_START_DONE;
    extsw_wreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    return 1;
}

enum {
    CFP_IDX_OP_VIDX_TO_TCIDX, 
    CFP_IDX_OP_FIND_LAST,
    CFP_IDX_OP_FIND_FIRST,
    CFP_IDX_OP_FIND_NEXT,
    CFP_IDX_OP_MOVE_DOWN,
    CFP_IDX_OP_MOVE_UP,
};

static int cfp_index_op(int tcIdx, int sliceId, int vidx, int op)
{
    int i, j, rc = -1;
    cfpTcam_t tcam, tcamMask;
    u32 aramData[2];

    switch(op)
    {
        case CFP_IDX_OP_VIDX_TO_TCIDX:
            for (i=CFP_FIRST_TCAM_INDEX; i<ARRAY_SIZE(tcamCtls); i++)
            {
                if(tcamCtls[i].argFlag == 0 || tcamCtls[i].sliceId != sliceId) continue;
                if (tcamCtls[i].vidx == vidx)
                {
                    rc = i;
                    break;
                }
            }
            break;
        case CFP_IDX_OP_FIND_FIRST:
            /* Find the first TCAM of a certain sliceId group */
            for (i=CFP_FIRST_TCAM_INDEX; i<ARRAY_SIZE(tcamCtls); i++)
            {
                if(tcamCtls[i].argFlag == 0 || tcamCtls[i].sliceId != sliceId) continue;
                rc  = i;
                break;
            }
            break;
        case CFP_IDX_OP_FIND_NEXT:
            /* Find the next TCAM of a certain sliceId group */
            if (tcIdx >= (ARRAY_SIZE(tcamCtls) - 1)) break;
            for (i=tcIdx+1; tcamCtls[i].argFlag && i<ARRAY_SIZE(tcamCtls); i++)
            {
                if(tcamCtls[i].sliceId != sliceId) continue;
                rc  = i;
                break;
            }
            break;
        case CFP_IDX_OP_FIND_LAST:
            /* Find the last TCAM of a certain sliceId group */
            for (i=CFP_FIRST_TCAM_INDEX; tcamCtls[i].argFlag && i<ARRAY_SIZE(tcamCtls); i++)
            {
                if(tcamCtls[i].sliceId != sliceId) continue;
                rc  = i;
            }
            break;
        case CFP_IDX_OP_MOVE_DOWN:
            /* Move CFP rule down by one */
            j = cfp_find_empty_tcam(); /* Find the tail, Caller should has verified this valid */
            if (j == -1) goto end;
            for (i=j; i>tcIdx; i--)
            {
                cfp_read_action_ram(aramData, i-1);
                cfp_write_action_ram(aramData, i);
                cfp_read_tcam(&tcam, &tcamMask, i-1);
                cfp_write_tcam(&tcam, &tcamMask, i);
                tcamCtls[i] = tcamCtls[i-1];
                tcamCtls[i].tidx = i;
                if (tcamCtls[i].sliceId == sliceId) tcamCtls[i].vidx++;
            }
            cfp_disable_tcam(tcIdx);
            tcamCtls[tcIdx].argFlag = 0;
            rc = tcIdx;
            break;
        case CFP_IDX_OP_MOVE_UP:
            /* Move CFP rule up by one */
            j = cfp_find_empty_tcam(); /* Caller should has verified this valid */
            if (j == -1 || j == CFP_FIRST_TCAM_INDEX) goto end;
            for (i=tcIdx; i<j-1; i++)
            {
                cfp_read_action_ram(aramData, i+1);
                cfp_write_action_ram(aramData, i);
                cfp_read_tcam(&tcam, &tcamMask, i+1);
                cfp_write_tcam(&tcam, &tcamMask, i);
                tcamCtls[i] = tcamCtls[i+1];
                tcamCtls[i].tidx = i;
                if (tcamCtls[i].sliceId == sliceId) tcamCtls[i].vidx--;
            }
            cfp_disable_tcam(j-1);
            tcamCtls[j-1].argFlag = 0;
            rc = tcIdx;
            break;
    }
end:
    return rc;
}

static int cfp_write_policer(u32 rateReg[], int tcIdx)
{
    int i;
    u32 v32;
    
    for (i=0; i<REG_CFP_RATE_REGS; i++)
    {
        extsw_wreg_wrap(PAGE_CFP, REG_CFP_RATE_DATA0+(i*4), rateReg+i, sizeof(u32));
    }

    do
    {
        extsw_rreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    } while((v32 & CFPACC_OP_START_DONE));

    v32 = (tcIdx<<REG_CFPACC_XCESS_ADDR_S)|CFPACC_RATE_METER_RAM_SEL| CFPACC_OP_WRITE|CFPACC_OP_START_DONE;
    extsw_wreg_wrap(PAGE_CFP, REG_CFP_ACC, &v32, sizeof(v32));
    return 1;
}

/*
  Configure CFP RATE Policer
*/
static int cfp_plicer_op(cfpParm_t *cfpParm)
{
    u32 rateReg[REG_CFP_RATE_REGS];
    cfpRateCtl_t *rateCtl = &cfpParm->rateCtl;

    memset(rateReg, 0, sizeof(rateReg));
    switch(rateCtl->policer_mode)
    {
        case CFP_DISABLED_RATE_MODE:
            rateReg[0] = CFP_RATE_DISABLED_MODE;
            break;
    }
    cfp_write_policer(rateReg, cfpParm->tcamCtl->tidx);
    return 1;
}

/* 
   static int cfp_addition(cfpArg_t *cfpArg)
   Add to top, insert or append new rule into CFP
*/
static int cfp_addition(cfpParm_t *cfpParm)
{
    uint32_t udfMask;
    int i, j, tcIdx, ti, rc = 0;
    udfCtl_t *curUdfCtl = cfpParm->curUdfCtl;
    cfpIpv4Tcam_t *tcamIpv4 = &cfpParm->tcam.ipv4;
    cfpIpv4Tcam_t *tcamIpv4Mask = &cfpParm->tcamMask.ipv4;
    cfpNoIpTcam_t *tcamNoIp = &cfpParm->tcam.noIp;
    cfpNoIpTcam_t *tcamNoIpMask = &cfpParm->tcamMask.noIp;
    cfpTcamCtl_t *tcamCtl = 0;
    cfpArg_t *cfpArg = cfpParm->cfpArg;

    udfMask = cfpArg->argFlag & CFP_UDF_FLAG;
    if (udfMask)
    {
        if (cfp_udf_op(cfpParm, UDF_CHECK) == 0)
        {
            cfpArg->rc = CFP_RC_UDF_FULL;
            goto end;
        }
    }

    if (cfpArg->spmap == 0)
    {
        cfpArg->spmap = PBMAP_ALL;
        cfpArg->argFlag |= CFP_ARG_SPMAP_M;
    }

    if ((tcIdx = cfp_find_empty_tcam()) == -1) 
    {
        cfpArg->rc = CFP_RC_CFP_FULL;
        goto end;
    }

    switch (cfpArg->op)
    {
        case CFPOP_APPEND:
            tcamCtl = &tcamCtls[tcIdx];
            ti = cfp_index_op(tcIdx, cfpArg->priority, 0, CFP_IDX_OP_FIND_LAST);
            if (ti == -1)
            {
                tcamCtl->vidx = 0;
            }
            else
            {
                tcamCtl->vidx = tcamCtls[ti].vidx + 1;
            }
            break;
        case CFPOP_ADD:
            tcIdx = cfp_index_op(0, cfpArg->priority, 0, CFP_IDX_OP_FIND_FIRST);
            if (tcIdx == -1) tcIdx = cfp_find_empty_tcam();
            tcIdx = cfp_index_op(tcIdx, cfpArg->priority, 0, CFP_IDX_OP_MOVE_DOWN);
            tcamCtl = &tcamCtls[tcIdx];
            cfpArg->index = tcamCtl->vidx;
            tcIdx = cfp_index_op(tcIdx, cfpArg->priority, 0, CFP_IDX_OP_MOVE_DOWN);
            break;
        case CFPOP_INSERT:
            tcIdx = cfp_index_op(0, cfpArg->priority, cfpArg->index, CFP_IDX_OP_VIDX_TO_TCIDX);
            if (tcIdx == -1)
            {
                cfpArg->rc = CFP_RC_NON_EXISTING_INDEX;
                goto end;
            }
            tcIdx = cfp_index_op(tcIdx, cfpArg->priority, 0, CFP_IDX_OP_MOVE_DOWN);
            tcamCtl = &tcamCtls[tcIdx];
            break;
        default:
            printk("Error: Unknow op %d\n", cfpArg->op);
            rc = -1;
            goto end;
    }

    cfpArg->index = tcamCtl->vidx;
    cfpParm->tcamCtl = tcamCtl;
    tcamCtl->sliceId = cfpArg->priority;
    tcamCtl->argFlag = cfpArg->argFlag;
    cfp_udf_op(cfpParm, UDF_ADD);
    tcamCtl->udfCtl = curUdfCtl;


    if ((cfpArg->argFlag & CFP_ARG_L3_FRAMING_M) == 0)
    {
        tcamNoIp->l3_framing = CfpL3Ipv4;
        cfpArg->argFlag |= CFP_ARG_L3_FRAMING_M;
    }

    udfMask = cfpArg->argFlag & (~CFP_UDF_FLAG);
    for (i=0; i<sizeof(udfMask)*8; i++)
    {
        if ((udfMask & (1<<i)) == 0) continue;
        switch ((1<<i))
        {
            case CFP_ARG_SPMAP_M:
                /* IMP port(P8) and P7 are squeezed into b7 and b6 in HW */
                /* Source Port Map has only meaning on mask, Tcam Data read as zero always */
                tcamNoIpMask->spmap = SF2_LOG_TO_CHIP_PMAP(~cfpArg->spmap);
                break;
            case CFP_ARG_IP_PROTOCOL_M: 
                if (cfpArg->l3_framing > L3_FRAMING_IPv6) break;
                tcamIpv4->ip_protocol = cfpArg->ip_protocol;
                tcamIpv4Mask->ip_protocol = cfpArg->ip_protocol_mask;
                break;
            case CFP_ARG_ETYPE_SAP_M:
                tcamNoIp->etype_sap = cfpArg->etype_sap;
                tcamNoIpMask->etype_sap = cfpArg->etype_sap_mask;
                break;
            case CFP_ARG_SVLAN_TAG_M:
                tcamNoIp->svtag = cfpArg->svtag; 
                tcamNoIpMask->svtag = cfpArg->svtag_mask;
                if ((cfpArg->svtag & VLAN_VID_MASK) == 0)
                {
                    tcamNoIp->svtag_status = VTAG_ST_VID0;
                }
                else
                {
                    tcamNoIp->svtag_status = VTAG_ST_VIDN;
                }
                tcamNoIpMask->svtag_status = 3;
                break;
            case CFP_ARG_CVLAN_TAG_M:
                tcamNoIp->cvtag_l = cfpArg->cvtag & 0xff; 
                tcamNoIp->cvtag_h = (cfpArg->cvtag>>8) & 0xff; 
                tcamNoIpMask->cvtag_l = cfpArg->cvtag_mask & 0xff;
                tcamNoIpMask->cvtag_h = (cfpArg->cvtag_mask>>8) & 0xff;
                /*
                if ((cfpArg->cvtag & VLAN_VID_MASK) == 0)
                {
                    tcamNoIp->cvtag_status = VTAG_ST_VID0;
                }
                else
                {
                    tcamNoIp->cvtag_status = VTAG_ST_VIDN;
                }
                tcamNoIpMask->cvtag_status = 3;
                */
                break;
            case CFP_ARG_PPPOE_M:
                tcamNoIp->pppoe_session = 1;
                tcamNoIpMask->pppoe_session = 1;
                break;
            case CFP_ARG_L2_FRAMING_M:
                tcamNoIp->l2_framing = cfpArg->l2_framing;
                tcamNoIpMask->l2_framing = 3;
                break;
            case CFP_ARG_L3_FRAMING_M:
                tcamNoIp->l3_framing = (cfpArg->l3_framing == CfpL3NoIP?L3_FRAMING_NON_IP:cfpArg->l3_framing);
                tcamNoIpMask->l3_framing = 3;
                break;
        }
    }
    tcamNoIp->slice_id = tcamCtl->sliceId;
    tcamNoIpMask->slice_id = 3;
    tcamNoIp->slice_valid = 0x3;
    tcamNoIpMask->slice_valid = 0x3;

    for (j=0; j<TOTAL_SWITCH_PORTS; j++)
    {
        if ((cfpArg->spmap & (1<<j)) == 0) continue;
        cfpCtl.portRefCnt[j]++;
        cfpCtl.portEnbMap |= (1<<j);
    }

    cfp_plicer_op(cfpParm);
    cfp_act_op(cfpArg, tcIdx);
    cfp_write_tcam((void *)tcamNoIp, (void *)tcamNoIpMask, tcIdx);
    extsw_wreg_wrap(PAGE_CFP_CONFIG, CFP_CONTROL, &cfpCtl.portEnbMap, 2);
    rc = 1;

end:
    return rc;
}

/*
    Build argument data from CFP&UDF control strucure
*/
static int cfp_build_arg(cfpParm_t *cfpParm)
{
    u16 v16, v16m;
    u32 v32, v32m;
    u64 v64, v64m;
    int i, j, k, udfMap, patUdfIdx;
    cfpArg_t *cfpArg = cfpParm->cfpArg;
    udfCtl_t *udfPat;

    for (k=0, udfPat = cfpParm->udfPatLst[0]; udfPat; udfPat = cfpParm->udfPatLst[++k])
    {
        udfMap = cfp_find_udf_map(udfPat, cfpParm->curUdfCtl);

        v32 = v32m = 0;
        v64 = v64m = 0;
        if (udfPat->memberSize == 8)
        {
            for (i=CFP_MAX_UDF_FIELDS_D, j=0, patUdfIdx=0; i>=0; i--)
            {
                if ((udfMap & (1<<i)) == 0) continue;
                cfp_get_udf_val(i, &cfpParm->tcam, &cfpParm->tcamMask, &v16, &v16m);
                /* 
                    Note: The values are all held in host byte endian instead of Network endian,
                   so we need to shift word from high end to low end ie. in network byte order.  
                */
                v64 |= ((u64)(v16 & udfPat->udfDsc[patUdfIdx].mask)) << (patUdfIdx*16);
                v64m |= ((u64)(v16m & udfPat->udfDsc[patUdfIdx].mask)) << (patUdfIdx*16);
                patUdfIdx++;
            }
            *(u64*)((char*)cfpArg + udfPat->argOffset) = v64;
            *(u64*)((char*)cfpArg + udfPat->argOffset + 8) = v64m;
        }
        else    /* 32 bit */
        {
            for (i=CFP_MAX_UDF_FIELDS_D, j=0, patUdfIdx=0; i>=0; i--)
            {
                if ((udfMap & (1<<i)) == 0) continue;
                    cfp_get_udf_val(i, &cfpParm->tcam, &cfpParm->tcamMask, &v16, &v16m);
                v32 |= (v16 & udfPat->udfDsc[patUdfIdx].mask) << (patUdfIdx*16);
                v32m |= (v16m & udfPat->udfDsc[patUdfIdx].mask) << (patUdfIdx*16);
                patUdfIdx++;
            }
            *(u32*)((char*)cfpArg + udfPat->argOffset) = v32;
            *(u32*)((char*)cfpArg + udfPat->argOffset + 4) = v32m;
        }
    }
    return 1;
}

static int cfp_read(cfpParm_t *cfpParm)
{
    cfpArg_t *cfpArg = cfpParm->cfpArg;
    cfpNoIpTcam_t *tcamNoIp = &cfpParm->tcam.noIp; 
    cfpNoIpTcam_t *tcamNoIpMask = &cfpParm->tcamMask.noIp;
    //cfpIpv4Tcam_t *tcamIpv4 = &tcam.ipv4;
    //cfpIpv4Tcam_t *tcamIpv4Mask = &tcamMask.ipv4;
    u32 i, tcIdx, rc = -1, argFlag;
    cfpTcamCtl_t *tcamCtl;
    udfCtl_t *curUdfCtl, *udfPat;
    u32 aramData[2];

    if (cfpArg->op == CFPOP_READ)
    {
        if (cfpArg->index == -1)
        {
            tcIdx = cfp_index_op(0, cfpArg->priority, 0, CFP_IDX_OP_FIND_FIRST);
        }
        else
        {
            tcIdx = cfp_index_op(0, cfpArg->priority, cfpArg->index, CFP_IDX_OP_VIDX_TO_TCIDX);
        }
    }
    else /* READ_NEXT */
    {
        tcIdx = cfp_index_op(0, cfpArg->priority, cfpArg->index, CFP_IDX_OP_VIDX_TO_TCIDX);
        tcIdx = cfp_index_op(tcIdx, cfpArg->priority, 0, CFP_IDX_OP_FIND_NEXT);
    }

    if (tcIdx == -1)
    {
        cfpArg->rc = CFP_RC_NON_EXISTING_INDEX;
        goto end;
    }

    tcamCtl = &tcamCtls[tcIdx];
    cfpArg->index = tcamCtl->vidx;
    curUdfCtl = tcamCtl->udfCtl;
    cfpParm->tcamCtl = tcamCtl;
    cfpParm->curUdfCtl = curUdfCtl;
    cfpArg->argFlag = tcamCtl->argFlag;

    cfp_build_udf_list(cfpParm->udfPatLst, cfpArg);
    /* Read T-CAM data and mask */
    cfp_read_tcam(&cfpParm->tcam, &cfpParm->tcamMask, tcamCtl->tidx);

    for (i=0, udfPat = cfpParm->udfPatLst[0]; udfPat; udfPat = cfpParm->udfPatLst[++i])
    {
        cfp_build_arg(cfpParm);
    }


    cfp_read_action_ram(aramData, tcIdx);

    cfpArg->priority = tcamCtl->sliceId;
    argFlag = cfpArg->argFlag & (~CFP_UDF_FLAG);
    for (i=0; i<sizeof(argFlag)*8; i++)
    {
        if ((argFlag & (1<<i)) == 0) continue;
        switch(1<<i)
        {
            case CFP_ARG_SPMAP_M:
                /* IMP port(P8) and P7 are squeezed into b7 and b6 in HW */
                /* Source Port Map has only meaning on mask, data read as zero always */
                cfpArg->spmap = SF2_CHIP_TO_LOG_PMAP(~tcamNoIpMask->spmap);
                break;
            case CFP_ARG_L2_FRAMING_M:
                cfpArg->l2_framing = tcamNoIp->l2_framing;
                break;
            case CFP_ARG_L3_FRAMING_M:
                cfpArg->l3_framing = tcamNoIp->l3_framing == L3_FRAMING_NON_IP? CfpL3NoIP:tcamNoIp->l3_framing;
                break;
            case CFP_ARG_SVLAN_TAG_M:
                cfpArg->svtag = tcamNoIp->svtag;
                cfpArg->svtag_mask = tcamNoIpMask->svtag;
                break;
            case CFP_ARG_CVLAN_TAG_M:
                cfpArg->cvtag = (tcamNoIp->cvtag_h<<8)|tcamNoIp->cvtag_l;
                cfpArg->cvtag_mask = (tcamNoIpMask->cvtag_h<<8)|tcamNoIpMask->cvtag_l;
                break;
            case CFP_ARG_PPPOE_M:
                cfpArg->pppoe = 1;
                break;
            case CFP_ARG_ETYPE_SAP_M:
                cfpArg->etype_sap = tcamNoIp->etype_sap;
                cfpArg->etype_sap_mask = tcamNoIpMask->etype_sap;
                break;
            case CFP_ARG_NEW_DSCP_IB_M:
                cfpArg->new_dscp_ib = (aramData[0] & CFP_NEW_DSCP_IB_M);
                break;
            case CFP_ARG_FPMAP_IB_M:
                cfpArg->fpmap_ib = SF2_CHIP_TO_LOG_PMAP((aramData[0] & CFP_FPMAP_IB_M)>>CFP_FPMAP_IB_S);
                break;
            case CFP_ARG_CHG_FPMAP_IB_M:
                cfpArg->chg_fpmap_ib = (aramData[0] & CFP_CHG_FPMAP_IB_M) >> CFP_CHG_FPMAP_IB_S;
                break;
        }
    }

    if (cfpArg->da_mask == 0xffffffffffffLL) cfpArg->da_mask = 0;
    if (cfpArg->sa_mask == 0xffffffffffffLL) cfpArg->sa_mask = 0;
    if (cfpArg->ip_protocol_mask == 0xff) cfpArg->ip_protocol_mask = 0;
    if (cfpArg->ipsa_mask == 0xffffffff) cfpArg->ipsa_mask = 0;
    if (cfpArg->ipda_mask == 0xffffffff) cfpArg->ipda_mask = 0;
    if (cfpArg->tcpudp_sport_mask == 0xffff) cfpArg->tcpudp_sport_mask = 0;
    if (cfpArg->tcpudp_dport_mask == 0xffff) cfpArg->tcpudp_dport_mask = 0;
    if (cfpArg->dscp_mask == 0xff) cfpArg->dscp_mask = 0;
    if (cfpArg->etype_sap_mask == 0xffffffff) cfpArg->etype_sap_mask = 0;
    if (cfpArg->svtag_mask == 0xffff) cfpArg->svtag_mask = 0;
    if (cfpArg->cvtag_mask == 0xffff) cfpArg->cvtag_mask = 0;
    if (cfpArg->spmap == PBMAP_ALL)
    {
        cfpArg->spmap = 0;
        cfpArg->argFlag &= ~CFP_ARG_SPMAP_M;
    }
    rc = 0;
end:
    return rc;
}


static int cfp_del(cfpParm_t *cfpParm)
{
    int j, tcIdx, rc = -1;
    udfCtl_t *curUdfCtl;
    cfpTcamCtl_t *tcamCtl;
    u32 udfMask;
    cfpArg_t *cfpArg = cfpParm->cfpArg;

    tcIdx = cfp_index_op(0,  cfpArg->priority, cfpArg->index,  CFP_IDX_OP_VIDX_TO_TCIDX);
    if (tcIdx == -1)
    {
        cfpArg->rc = CFP_RC_NON_EXISTING_INDEX;
        goto end;
    }

    tcamCtl = cfpParm->tcamCtl = &tcamCtls[tcIdx];
    curUdfCtl = cfpParm->curUdfCtl = tcamCtl->udfCtl;
    cfpArg->argFlag = tcamCtl->argFlag;
    cfp_build_udf_list(cfpParm->udfPatLst, cfpArg);
    udfMask = cfpArg->argFlag & CFP_UDF_FLAG;
    if (udfMask)
    {
        if (cfp_udf_op(cfpParm, UDF_DEL) == 0) goto end;
    }

    if (cfpArg->spmap)
    {
        for (j=0; j<TOTAL_SWITCH_PORTS; j++)
    {
        if ((cfpArg->spmap & (1<<j)) == 0) continue;
        if (--cfpCtl.portRefCnt[j] == 0)
        {
            cfpCtl.portEnbMap &= ~(1<<j);
        }
    }
    }

    extsw_wreg_wrap(PAGE_CFP_CONFIG, CFP_CONTROL, &cfpCtl.portEnbMap, 2);
    cfp_disable_tcam(tcIdx);
    cfp_index_op(tcIdx, 0, 0, CFP_IDX_OP_MOVE_UP);

    rc = 0;

end:
    return rc;
}

static int cfp_del_all(cfpParm_t *cfpParm)
{
    int pr, prSt, prEnd;
    cfpArg_t *cfpArg = cfpParm->cfpArg;

    if ((cfpArg->argFlag & CFP_ARG_PRIORITY_M))
    {
        prSt = prEnd = cfpArg->priority;
    }
    else
    {
        prSt = 0;
        prEnd = 2;
    }

    for (pr = prSt; pr<=prEnd; pr++)
    {
        cfpArg->priority = pr;
        if ((cfpArg->argFlag & CFP_ARG_INDEX_M))
        {
            cfp_del(cfpParm);
        }
        else
        {
            for(cfpArg->index = 0, cfpArg->rc = CFP_RC_SUCCESS; 
                cfpArg->rc != CFP_RC_NON_EXISTING_INDEX;) 
            {
                cfp_del(cfpParm);
            }
        }
    }

    /* Override rc to be success for delete_all */
    cfpArg->rc = CFP_RC_SUCCESS;
    return 0;
}

int bcmeapi_ioctl_cfp(struct ethswctl_data *e)
{
    int rc = -1;
    cfpArg_t *cfpArg = &e->cfpArgs;
    static int inited = 0; 
    cfpParm_t cfpParm;

    memset(&cfpParm, 0, sizeof(cfpParm));
    cfpParm.cfpArg = cfpArg;
    cfp_build_udf_list(cfpParm.udfPatLst, cfpArg);
    cfpParm.curUdfCtl = &udfCtls[cfpArg->l3_framing* CFP_SLICES + cfpArg->priority];
    cfpArg->rc = CFP_RC_SUCCESS;

    if (inited++ == 0) cfp_init();

    if (cfpArg->da_mask == 0) cfpArg->da_mask = 0xffffffffffffffffLL;
    if (cfpArg->sa_mask == 0) cfpArg->sa_mask = 0xffffffffffffffffLL;
    if (cfpArg->ip_protocol_mask == 0) cfpArg->ip_protocol_mask = 0xffffffff;
    if (cfpArg->ipsa_mask == 0) cfpArg->ipsa_mask = 0xffffffff;
    if (cfpArg->ipda_mask == 0) cfpArg->ipda_mask = 0xffffffff;
    if (cfpArg->tcpudp_sport_mask == 0) cfpArg->tcpudp_sport_mask = 0xffffffff;
    if (cfpArg->tcpudp_dport_mask == 0) cfpArg->tcpudp_dport_mask = 0xffffffff;
    if (cfpArg->dscp_mask == 0) cfpArg->dscp_mask = 0xffffffff;
    if (cfpArg->etype_sap_mask == 0) cfpArg->etype_sap_mask = 0xffffffff;
    if (cfpArg->svtag_mask == 0) cfpArg->svtag_mask = 0xffffffff;
    if (cfpArg->cvtag_mask == 0) cfpArg->cvtag_mask = 0xffffffff;

    cfpParm.rateCtl.policer_mode = CFP_DISABLED_RATE_MODE;
    switch (cfpArg->op)
    {
        case CFPOP_ADD:
        case CFPOP_INSERT:
        case CFPOP_APPEND:
            rc = cfp_addition(&cfpParm);
            break;
        case CFPOP_READ:
        case CFPOP_READ_NEXT:
            rc = cfp_read(&cfpParm);
            break;
        case CFPOP_DELETE:
            rc = cfp_del(&cfpParm);
            break;
        case CFPOP_DELETE_ALL:
            rc = cfp_del_all(&cfpParm);
            break;
        default:
            break;
    }

    return rc;
}

