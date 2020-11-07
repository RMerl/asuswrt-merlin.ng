/***********************************************************************
 *
 *  Copyright (c) 2010  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:proprietary:standard

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
 *
************************************************************************/

/***************************************************************************
 * File Name  : bpmctl_cmds.c
 * Description: Linux command line utility that controls the Broadcom
 *              BCM63xx BPM Driver.
 ***************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "bpmctl_common.h"
#include "bpmctl.h"
#include "bpmctl_api.h"

void Usage(void);

int  bpmctlStatusHandler(POPTION_INFO pOptions, int nNumOptions);
int  bpmctlThreshHandler(POPTION_INFO pOptions, int nNumOptions);
int  bpmctlThreshHandler(POPTION_INFO pOptions, int nNumOptions);
int  bpmctlBuffersHandler(POPTION_INFO pOptions, int nNumOptions);
int  bpmctlSkbuffsHandler(POPTION_INFO pOptions, int nNumOptions);
#if defined(CC_BPM_DEBUG)
int  bpmctlDebugHandler(POPTION_INFO pOptions, int nNumOptions);
#endif
#if defined(BPM_TRACK)
int  bpmctlTrackStatusHandler(POPTION_INFO pOptions, int nNumOptions);
int  bpmctlTrackEnableHandler(POPTION_INFO pOptions, int nNumOptions);
int  bpmctlTrackDisableHandler(POPTION_INFO pOptions, int nNumOptions);
int  bpmctlTrackDumpHandler(POPTION_INFO pOptions, int nNumOptions);
int  bpmctlTrackBuffersHandler(POPTION_INFO pOptions, int nNumOptions);
int  bpmctlTrackTrailsHandler(POPTION_INFO pOptions, int nNumOptions);
int  bpmctlTrackIncHandler(POPTION_INFO pOptions, int nNumOptions);
#endif


char g_PgmName[128] = {0};

/*** File Scoped Globals. ***/
COMMAND_INFO g_Cmds[] =
{
    {"status",      {""},                       bpmctlStatusHandler},
    {"thresh",      {""},                       bpmctlThreshHandler},
    {"buffers",     {""},                       bpmctlBuffersHandler},
    {"skbuffs",     {""},                       bpmctlSkbuffsHandler},
#if defined(CC_BPM_DEBUG)
    {"debug",       {"--drv", "--iq", ""},      bpmctlDebugHandler},
#endif
#if defined(BPM_TRACK)
    {"tstatus",     {""},                       bpmctlTrackStatusHandler},
    {"tenable",     {"--len", ""},              bpmctlTrackEnableHandler},
    {"tdisable",    {""},                       bpmctlTrackDisableHandler},
    {"dump",        {"--addr", ""},             bpmctlTrackDumpHandler},
    {"sbuffers",    {"--base", "--addr", "--drv", "--val", "--info", "--idle", "--idlemin", "--ref", "--refmin",""},
                                                bpmctlTrackBuffersHandler},
    {"trails",      {"--base", "--addr", "--drv", "--val", "--info", "--idle", "--idlemin", "--ref", "--refmin",""},
                                                bpmctlTrackTrailsHandler},
    {"inc",         {"--base", "--addr", "--drv", "--val", "--info", "--idle", "--idlemin", "--ref", "--refmin",""},
                                                bpmctlTrackIncHandler},
#endif
    {""}
};

/*
 *------------------------------------------------------------------------------
 * Function Name: Usage
 * Description  : Displays the iq usage
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
void Usage(void)
{
    printf( 
        "BPM Control Utility:\n\n"
        "::: Usage:\n\n"

        ":::::: BPM SW System :\n"
        "       bpm status\n"
        "       bpm thresh\n"
        "       bpm buffers\n"
        "       bpm skbuffs\n"
#if defined(CC_BPM_DEBUG)
        "       bpm debug\n"
        "                      [ --drv    <0..3> ]\n"
        "                    | [ --bpm     <0..3> ]\n\n"
#endif
#if defined(BPM_TRACK)
        "       bpm tstatus\n"
        "       bpm tenable\n"
        "                  [ --len     <length> ]\n"
        "          - Enable BPM tracking with len entries in ring buffer.\n"
        "            Ring buffer size is adjusted to be cache-aligned.\n"
        "       bpm tdisable\n"
        "       bpm dump --addr <address> [f] [buff | skb | fkb]\n"
        "          - Dump bytes from struct at <address>\n"
        "          - Only recognizes tracked nbuff.\n"
        "          - f flips endianness\n"
        "       bpm print --addr <address> [buff | skb | fkb]\n"
        "          - Print struct at <address>\n"
        "          - Only recognizes tracked nbuff.\n"
        "       bpm trails\n"
        "          - Dump all trails matching any combination of:\n"
        "                  [ --base    <base>   ]  - has base buffer address\n"
        "                | [ --addr    <addr>   ]  - has mark with addr\n"
        "                | [ --drv     <driver> ]  - has mark with driver\n"
        "                | [ --val     <value>  ]  - has mark with value\n"
        "                | [ --info    <info>   ]  - has mark with info\n"
        "                | [ --idle    <cnt>    ]  - has idle cnt == cnt\n"
        "                | [ --idlemin <min>    ]  - has idle cnt >= min\n"
        "                | [ --ref     <cnt>    ]  - has ref cnt == cnt\n"
        "                | [ --refmin  <min>    ]  - has ref cnt >= min\n\n"
        "           <driver> is one of the following:\n"
        "             bpm, eth, xtm, kern, bdmf\n"
        "           <mark> is one of the following:\n"
        "             nomrk, alloc, clone, recyl, free, rx, tx, enter, exit\n"
        "             info, init, cpsrc, cpdst, xlate\n"
        "       bpm sbuffers\n"
        "          - Dump static references to buffers, same options as \"bpm trails\"\n"
        "       bpm inc\n"
        "          - Increment idle cnt on trails, same options as \"bpm trails\"\n\n"
#endif
        );

    return;
} /* Usage */



/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlStatusHandler
 * Description  : Processes the BPM status command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int bpmctlStatusHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet = bcm_bpmctl_dump_status();

    if ( nRet == BPMCTL_ERROR )
        fprintf( stderr, "%s: failed to get BPM status\n", g_PgmName );
    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlThreshHandler
 * Description  : Processes the BPM status command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int bpmctlThreshHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet = bcm_bpmctl_dump_thresh();

    if ( nRet == BPMCTL_ERROR )
        fprintf( stderr, "%s: failed to get BPM thresh\n", g_PgmName );
    return nRet;
}



/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlBuffersHandler
 * Description  : Processes the BPM show buffers command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int bpmctlBuffersHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet = bcm_bpmctl_dump_buffers( );

    if ( nRet == BPMCTL_ERROR )
        fprintf( stderr, "%s: failed to get BPM buffers\n", g_PgmName );
    return nRet;
}


/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlSkbuffsHandler
 * Description  : Processes the BPM show skbuffsbuffers command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int bpmctlSkbuffsHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet = bcm_bpmctl_dump_skbuffs( );

    if ( nRet == BPMCTL_ERROR )
        fprintf( stderr, "%s: failed to get BPM skbuffs\n", g_PgmName );
    return nRet;
}


#if defined(CC_BPM_DEBUG)
/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlDebugHandler
 * Description  : Processes the BPM debug command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int  bpmctlDebugHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;

    while ( nNumOptions )
    {
        int level;

        if ( pOptions->nNumParms != 1 )
        {
           fprintf( stderr, "%s: did not specify debug level.\n", g_PgmName );
           return BPMCTL_INVALID_OPTION;
        }

        level = atoi( pOptions->pParms[0] );

        if ( !strcmp( pOptions->pOptName, "--drv") )
            nRet = bpmctlDebug( BPM_DBG_DRV_LAYER, level );
        else if ( !strcmp( pOptions->pOptName, "--bpm") )
            nRet = bpmctlDebug( BPM_DBG_FC_LAYER, level );
        else
        {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            return BPMCTL_INVALID_OPTION;
        }
        nNumOptions--;
        pOptions++;

        if ( nRet == BPMCTL_ERROR )
            fprintf( stderr, "%s: failed debug request\n", g_PgmName );
    }

    return nRet;
}
#endif

#if defined(BPM_TRACK)
static bpmctl_drv_t bpmctlTrackStrToDriverEnum( char * str )
{
    if ( !strcmp( str, "bpm") )
        return BPMCTL_DRV_BPM;
    else if ( !strcmp( str, "eth") )
        return BPMCTL_DRV_ETH;
    else if ( !strcmp( str, "xtm") )
        return BPMCTL_DRV_XTM;
    else if ( !strcmp( str, "kern") )
        return BPMCTL_DRV_KERN;
    else if ( !strcmp( str, "bdmf") )
        return BPMCTL_DRV_BDMF;
    return BPMCTL_INVALID_OPTION;
}

static bpmctl_val_t bpmctlTrackStrToMarkEnum( char * str )
{
    if ( !strcmp( str, "nomrk") )
        return BPMCTL_VAL_UNMARKED;
    else if ( !strcmp( str, "alloc") )
        return BPMCTL_VAL_ALLOC;
    else if ( !strcmp( str, "clone") )
        return BPMCTL_VAL_CLONE;
    else if ( !strcmp( str, "recyl") )
        return BPMCTL_VAL_RECYCLE;
    else if ( !strcmp( str, "free") )
        return BPMCTL_VAL_FREE;
    else if ( !strcmp( str, "rx") )
        return BPMCTL_VAL_RX;
    else if ( !strcmp( str, "tx") )
        return BPMCTL_VAL_TX;
    else if ( !strcmp( str, "enter") )
        return BPMCTL_VAL_ENTER;
    else if ( !strcmp( str, "exit") )
        return BPMCTL_VAL_EXIT;
    else if ( !strcmp( str, "info") )
        return BPMCTL_VAL_INFO;
    else if ( !strcmp( str, "init") )
        return BPMCTL_VAL_INIT;
    else if ( !strcmp( str, "cpsrc") )
        return BPMCTL_VAL_COPY_SRC;
    else if ( !strcmp( str, "cpdst") )
        return BPMCTL_VAL_COPY_DST;
    else if ( !strcmp( str, "xlate") )
        return BPMCTL_VAL_XLATE;
    return BPMCTL_INVALID_OPTION;
}

static int bpmctlTrackParseFilter(bpmctl_track_t * trk_p, POPTION_INFO pOption)
{
    unsigned long long parmVal;

    if ( pOption->nNumParms != 1 || pOption->pParms[0] == NULL )
    {
        fprintf( stderr, "%s: incorrect number of parameters\n", g_PgmName );
        return BPMCTL_INVALID_OPTION;
    }

    if ( !strcmp( pOption->pOptName, "--base") )
    {
        if ( sscanf(pOption->pParms[0], "%llx", &parmVal) == 0 )
            goto invalid_param;

        BPMCTL_TRK_SET(trk_p->filters, BPMCTL_TRK_BASE);
        trk_p->base = parmVal;
    }
    else if ( !strcmp( pOption->pOptName, "--addr") )
    {
        if ( sscanf(pOption->pParms[0], "%llx", &parmVal) == 0 )
            goto invalid_param;

        BPMCTL_TRK_SET(trk_p->filters, BPMCTL_TRK_ADDR);
        trk_p->addr = parmVal;
    }
    else if ( !strcmp( pOption->pOptName, "--drv") )
    {
        trk_p->driver = bpmctlTrackStrToDriverEnum(pOption->pParms[0]);

        if ( trk_p->driver == BPMCTL_INVALID_OPTION )
            goto invalid_driver;

        BPMCTL_TRK_SET(trk_p->filters, BPMCTL_TRK_DRIVER);
    }
    else if ( !strcmp( pOption->pOptName, "--val") )
    {
        trk_p->value = bpmctlTrackStrToMarkEnum(pOption->pParms[0]);

        if ( trk_p->value == BPMCTL_INVALID_OPTION )
            goto invalid_mark;

        BPMCTL_TRK_SET(trk_p->filters, BPMCTL_TRK_VALUE);
    }
    else if ( !strcmp( pOption->pOptName, "--info") )
    {
        if ( sscanf(pOption->pParms[0], "%llu", &parmVal) == 0 )
            goto invalid_param;

        BPMCTL_TRK_SET(trk_p->filters, BPMCTL_TRK_INFO);
        trk_p->info = parmVal;
    }
    else if ( !strcmp( pOption->pOptName, "--idle") )
    {
        if ( sscanf(pOption->pParms[0], "%llu", &parmVal) == 0 )
            goto invalid_param;

        BPMCTL_TRK_SET(trk_p->filters, BPMCTL_TRK_IDLE);
        trk_p->idle = (int)parmVal;
    }
    else if ( !strcmp( pOption->pOptName, "--idlemin") )
    {
        if ( sscanf(pOption->pParms[0], "%llu", &parmVal) == 0 )
            goto invalid_param;

        BPMCTL_TRK_SET(trk_p->filters, BPMCTL_TRK_IDLEMIN);
        trk_p->idle_min = parmVal;
    }
    else if ( !strcmp( pOption->pOptName, "--ref") )
    {
        if ( sscanf(pOption->pParms[0], "%llu", &parmVal) == 0 )
            goto invalid_param;

        BPMCTL_TRK_SET(trk_p->filters, BPMCTL_TRK_REF);
        trk_p->ref = (int)parmVal;
    }
    else if ( !strcmp( pOption->pOptName, "--refmin") )
    {
        if ( sscanf(pOption->pParms[0], "%llu", &parmVal) == 0 )
            goto invalid_param;

        BPMCTL_TRK_SET(trk_p->filters, BPMCTL_TRK_REFMIN);
        trk_p->ref_min = parmVal;
    }
    else
    {
        fprintf( stderr,
                 "%s: invalid option \"%s\", expected:\n"
                 "        --base, --addr, --drv, --val, --info"
                 "        --idle, --idlemin, --ref, --refmin\n",
             g_PgmName, pOption->pOptName);
        return BPMCTL_INVALID_OPTION;
    }

    return BPMCTL_SUCCESS;

invalid_driver:
    fprintf( stderr,
             "%s: invalid driver \"%s\", expected:\n"
             "        bpm, eth, xtm, kern, bdmf\n",
             g_PgmName, pOption->pParms[0]);
    return BPMCTL_INVALID_OPTION;

invalid_mark:
    fprintf( stderr,
             "%s: invalid mark \"%s\", expected:\n"
             "        nomrk, alloc, clone, recyl, free, rx, tx, enter, exit\n"
             "        info, init, cpsrc, cpdst, xlate\n",
             g_PgmName, pOption->pParms[0]);
    return BPMCTL_INVALID_OPTION;

invalid_param:
    fprintf( stderr, "%s: invalid param \"%s\"\n",
             g_PgmName, pOption->pParms[0]);
    return BPMCTL_INVALID_OPTION;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlTrackStatusHandler
 * Description  : Processes the BPM tstatus command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int bpmctlTrackStatusHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    bpmctl_track_t trk;

    trk.cmd = BPMCTL_TRK_STATUS;
    nRet = bcm_bpmctl_track( &trk );

    if ( nRet == BPMCTL_ERROR )
        fprintf( stderr, "%s: failed to get BPM tracking status\n", g_PgmName );
    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlTrackEnableHandler
 * Description  : Processes the BPM tenable command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int bpmctlTrackEnableHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    bpmctl_track_t trk;

    trk.cmd = BPMCTL_TRK_ENABLE;
    trk.len = 0;

    while ( nNumOptions )
    {
        if ( !strcmp( pOptions->pOptName, "--len") )
        {
            if ( sscanf(pOptions->pParms[0], "%u", &trk.len) == 0 )
            {
                fprintf( stderr, "%s: invalid param [%s]\n",
                         g_PgmName, pOptions->pParms[0] );
                return BPMCTL_INVALID_OPTION;
            }
        }
        else
        {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            return BPMCTL_INVALID_OPTION;
        }
        nNumOptions--;
        pOptions++;
    }
    nRet = bcm_bpmctl_track( &trk );

    if ( nRet == BPMCTL_ERROR )
        fprintf( stderr, "%s: failed to enable BPM tracking\n", g_PgmName );
    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlTrackDisableHandler
 * Description  : Processes the BPM tdisable command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int bpmctlTrackDisableHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    bpmctl_track_t trk;

    trk.cmd = BPMCTL_TRK_DISABLE;
    nRet = bcm_bpmctl_track( &trk );

    if ( nRet == BPMCTL_ERROR )
        fprintf( stderr, "%s: failed to disable BPM tracking\n", g_PgmName );
    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlTrackDumpHandler
 * Description  : Processes the BPM dump command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int bpmctlTrackDumpHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    int i, cnt = 0;
    bpmctl_track_t trk;

    trk.cmd = BPMCTL_TRK_DUMP;
    trk.flip_endian = 0;
    trk.reftype = BPMCTL_REF_ANY;
    trk.addr = 0;

    if ( nNumOptions != 1 ||
         strcmp( pOptions->pOptName, "--addr" ) ||
         pOptions->nNumParms == 0 ||
         sscanf(pOptions->pParms[0], "%llx", &trk.addr) == 0 )
    {
        fprintf( stderr, "%s: expect --addr <address>.\n", g_PgmName );
        return BPMCTL_INVALID_OPTION;
    }

    for ( i = 1; i < pOptions->nNumParms; i++ )
    {
        if ( !strcmp( pOptions->pParms[i], "f" ) )
        {
            trk.flip_endian = 1;
        }
        else if ( !strcmp( pOptions->pParms[i], "buff" ) )
        {
            trk.reftype = BPMCTL_REF_BUFF;
        }
        else if ( !strcmp( pOptions->pParms[i], "skb" ) )
        {
            trk.reftype = BPMCTL_REF_SKB;
        }
        else if ( !strcmp( pOptions->pParms[i], "fkb" ) )
        {
            trk.reftype = BPMCTL_REF_FKB;
        }
        else
        {
            fprintf( stderr, "%s: invalid parameter [%s]\n",
                     g_PgmName, pOptions->pParms[i] );
            return BPMCTL_INVALID_OPTION;
        }
    }

    nRet = bcm_bpmctl_track( &trk );

    if ( nRet == BPMCTL_ERROR )
        fprintf( stderr, "%s: failed to get BPM tracking dump\n", g_PgmName );
    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlTrackBuffersHandler
 * Description  : Processes the BPM sbuffers command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int bpmctlTrackBuffersHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    bpmctl_track_t trk;

    trk.cmd = BPMCTL_TRK_BUFFERS;
    trk.filters = 0;

    while ( nNumOptions )
    {
        nRet = bpmctlTrackParseFilter( &trk, pOptions );
        if ( nRet == BPMCTL_INVALID_OPTION )
        {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            return BPMCTL_INVALID_OPTION;
        }
        nNumOptions--;
        pOptions++;
    }
    nRet = bcm_bpmctl_track( &trk );

    if ( nRet == BPMCTL_ERROR )
        fprintf( stderr, "%s: failed to get BPM tracking buffers\n", g_PgmName );

    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlTrackTrailsHandler
 * Description  : Processes the BPM trails command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int bpmctlTrackTrailsHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    bpmctl_track_t trk;

    trk.cmd = BPMCTL_TRK_TRAILS;
    trk.filters = 0;

    while ( nNumOptions )
    {
        nRet = bpmctlTrackParseFilter( &trk, pOptions );
        if ( nRet == BPMCTL_INVALID_OPTION )
        {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            return BPMCTL_INVALID_OPTION;
        }
        nNumOptions--;
        pOptions++;
    }
    nRet = bcm_bpmctl_track( &trk );

    if ( nRet == BPMCTL_ERROR )
        fprintf( stderr, "%s: failed to get BPM tracking trails\n", g_PgmName );

    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: bpmctlTrackIncHandler
 * Description  : Processes the BPM inc command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int bpmctlTrackIncHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    bpmctl_track_t trk;

    trk.cmd = BPMCTL_TRK_INC;
    trk.filters = 0;

    while ( nNumOptions )
    {
        nRet = bpmctlTrackParseFilter( &trk, pOptions );
        if ( nRet == BPMCTL_INVALID_OPTION )
        {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            return BPMCTL_INVALID_OPTION;
        }
        nNumOptions--;
        pOptions++;
    }
    nRet = bcm_bpmctl_track( &trk );

    if ( nRet == BPMCTL_ERROR )
        fprintf( stderr, "%s: failed to increment BPM tracking trails\n", g_PgmName );

    return nRet;
}
#endif


