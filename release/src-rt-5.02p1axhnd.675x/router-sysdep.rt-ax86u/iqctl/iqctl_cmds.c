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
 * File Name  : iqctl_cmds.c
 * Description: Linux command line utility that controls the Broadcom
 *              BCM63xx Ingress QoS Driver.
 ***************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>  // for getaddrinfo
#include "iqctl.h"
#include "iqctl_common.h"
#include "iqctl_api.h"

void Usage(void);

int iqctlStatusHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlEnableHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlDisableHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlFlushHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlHWAccelCongCtrlEnableHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlHWAccelCongCtrlDisableHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlAddportHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlRemportHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlGetportHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlAddkeymaskHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlRemkeymaskHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlDumpkeymasktblHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlAddkeyHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlRemkeyHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlGetkeyHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlDumpkeytblHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlDumpporttblHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlSetDefaultPrioHandler(POPTION_INFO pOptions, int nNumOptions);
int iqctlRemDefaultPrioHandler(POPTION_INFO pOptions, int nNumOptions);
#if defined(CC_IQ_DEBUG)
int iqctlDebugHandler(POPTION_INFO pOptions, int nNumOptions);
#endif


char g_PgmName[128] = {0};

/*** File Scoped Globals. ***/
#if defined(SUPPORT_RDPA)
#define IQCTL_KEY_FIELD_OPTON \
    /* below are classification fields */ \
    {"--srcmac", "--dstmac", "--ethtype", "--outervid", \
    "--outerpbit", "--innervid", "--innerpbit", \
    "--l3proto", "--ipproto", "--srcip", "--dstip", "--dscp", \
    "--ipv6flowlabel", "--srcport", "--dstport", \
    /* below are classification fields that are yet supported */ \
    /* "--l2proto", "--indev", "--offset0", "--offset1", */ \
    /* below are attributes/types */ \
    "--maskprio", "--ent", \
    /* below are actions */ \
    "--prio", "--drop", "--trap", \
    ""}
#elif defined(SUPPORT_ARCHERCTL)
#define IQCTL_KEY_FIELD_OPTON \
    /* below are classification fields */ \
   { "--ipproto", "--dscp", "--dstport", \
     "--ethtype", "--l3proto", \
    /* below are attributes/types */ \
    "--maskprio", "--ent", \
    /* below are actions */ \
    "--prio", \
    ""}
#endif

COMMAND_INFO g_Cmds[] =
{
    {"status",      {"-u"},                     iqctlStatusHandler},
    {"enable",      {""},                       iqctlEnableHandler},
    {"disable",     {""},                       iqctlDisableHandler},
#if defined(SUPPORT_RDPA)
    {"enablehwaccelcongctrl",  {""},            iqctlHWAccelCongCtrlEnableHandler},
    {"disablehwaccelcongctrl", {""},            iqctlHWAccelCongCtrlDisableHandler},
#endif
    {"flush",       {""},                       iqctlFlushHandler},
    {"addport",     {"--proto", "--dport", "--ent", "--prio", ""},
                                                iqctlAddportHandler},
    {"remport",     {"--proto", "--dport", "--ent", ""},
                                                iqctlRemportHandler},
    {"getport",     {"--proto", "--dport", ""},
                                                iqctlGetportHandler},
    {"porttbl",     {"-k"},                     iqctlDumpporttblHandler},
#if (defined(SUPPORT_RDPA) || defined(SUPPORT_ARCHERCTL))
    {"addkeymask",  IQCTL_KEY_FIELD_OPTON,      iqctlAddkeymaskHandler},
    {"remkeymask",  IQCTL_KEY_FIELD_OPTON,      iqctlRemkeymaskHandler},
    {"keymasktbl",  IQCTL_KEY_FIELD_OPTON,      iqctlDumpkeymasktblHandler},
    {"addkey",      IQCTL_KEY_FIELD_OPTON,      iqctlAddkeyHandler},
    {"remkey",      IQCTL_KEY_FIELD_OPTON,      iqctlRemkeyHandler},
    {"getkey",      IQCTL_KEY_FIELD_OPTON,      iqctlGetkeyHandler},
    {"keytbl",      IQCTL_KEY_FIELD_OPTON,      iqctlDumpkeytblHandler},
#else
    {"setdefaultprio", {"--prototype", "--protoval", "--prio", ""},   
                                                iqctlSetDefaultPrioHandler},
    {"remdefaultprio", {"--prototype", "--protoval", ""},   
                                                iqctlRemDefaultPrioHandler},
#endif
#if defined(CC_IQ_DEBUG)
    {"debug",       {"--drv", "--iq", ""},      iqctlDebugHandler},
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
        "Ingress QoS Control Utility:\n\n"
        "::: Usage:\n\n"

        ":::::: Ingress QoS SW System :\n"
        "       iq status [-u]\n"
        "          -u: new userspace status dump.\n"
        "       iq enable\n"
        "       iq disable\n"
#if defined(SUPPORT_RDPA)
        "       iq enablehwaccelcongctrl\n"
        "       iq disablehwaccelcongctrl\n"
#endif
        "       iq flush\n"
        "       iq addport\n"
        "          --proto <0|1> --dport <1..65534> --ent <0|1> --prio <0|1>\n"
        "          proto: 0 = TCP, 1 = UDP\n"
        "          ent: 0 = dynamic, 1 = static\n"
        "          prio: 0 = low, 1 = high\n"
        "       iq remport --proto <0|1> --dport <1..65534> --ent <0|1>\n"
        "          proto: 0 = TCP, 1 = UDP\n"
        "          ent: 0 = dynamic, 1 = static\n"
        "       iq getport --proto <0|1> --dport <1..65534> \n"
        "          proto: 0 = TCP, 1 = UDP\n"
        "       iq porttbl [-k]\n"
        "          -k: dump from kernel\n"
#if (defined(SUPPORT_RDPA) || defined(SUPPORT_ARCHERCTL))
        "       iq addkeymask [field] --maskprio <0..15>\n"
        "          maskprio: priority value, larger value = higher priority\n"
        "       iq remkeymask [field]\n"
        "       iq keymasktbl\n"
        "       iq addkey [field | action | attribute]\n"
        "       iq remkey [field | action]\n"
        "       iq getkey [field]\n"
#if defined(SUPPORT_RDPA)
        "       iq keytbl\n"
        "          field:\n"
/*        "              indev: [TBD]\n" unsupported at this moment FIXME*/
        "              --srcmac: MAC address in x:x:x:x:x:x format\n"
        "              --dstmac: MAC address in x:x:x:x:x:x format\n"
        "              --ethtype: ether type\n"
        "              --outervid: Outer VLAN ID\n"
        "              --outerpbit: Outer VLAN PBit\n"
        "              --innervid: Inner VLAN ID\n"
        "              --innerpbit: Inner VLAN PBit\n"
/*        "              --l2proto: [TBD]\n" unsupported at this moment FIXME*/
        "              --l3proto: e.g., IPv4 (0x0800) or IPv6 (0x86DD)\n"
        "              --ipproto: such as TCP (6) or UDP (17)\n"
        "              --srcip: Source IPv4/6 Address\n"
        "              --dstip: Destination IPv4/6 Address\n"
        "              --dscp: 6-bit IP->DSCP value\n"
        "              --ipv6flowlabel: 20-bit IPv6->flowlabel value\n"
        "              --srcport: 16-bit L4 SRC Port\n"
        "              --dstport: 16-bit L4 DST Port\n"
/*        "              --offset0: [TBD]\n" FIXME*/
/*        "              --offset1: [TBD]\n" FIXME*/
        "          action:\n"
        "              --prio: 0 = low, 1 = high\n"
        "              --drop: drop packet\n"
        "              --trap: trap packet before flow lookup\n"
#else // SUPPORT_ARCHERCTL
        "       iq keytbl\n"
        "          field:\n"
        "              --ethtype: ether type\n"
        "              --ipproto: such as TCP (6) or UDP (17)\n"
        "              --dscp: 6-bit IP->DSCP value\n"
        "              --dstport: 16-bit L4 DST Port\n"
        "              --l3proto: 16-bit L3 protocol, applicable to PPPoE session only\n"
        "          action:\n"
        "              --prio: 0 = low, 1 = high\n"
#endif
        "          attribute:\n"
        "              --ent: 0 = dynamic, 1 = static\n"
#else
        "       iq setdefaultprio --prototype <0> --protoval <0..255> --prio <0/1>\n"
        "          prototype: 0 = ipproto\n"
        "          protoval: protocol value (0 to 255)\n"
        "          prio: 0 = low, 1 = high\n"
        "       iq remdefaultprio --prototype <0> --protoval <0..255>\n"
        "          prototype: 0 = ipproto\n"
        "          protoval: protocol value (0 to 255)\n"
#endif
#if defined(CC_IQ_DEBUG)
        "       iq debug\n"
        "                      [ --drv    <0..5> ]\n"
        "                    | [ --iq     <0..5> ]\n\n"
#endif

        );

    return;
} /* Usage */



/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlStatusHandler
 * Description  : Processes the ingress QoS status command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlStatusHandler(POPTION_INFO pOptions, int nNumOptions)
{
    iqctl_status_t status;
    int nRet;

    if ((nNumOptions == 1) && (!strcmp(pOptions->pOptName, "-u")))
    {
        nRet = bcm_iqctl_get_status( &status );
    
        if ( nRet )
            fprintf( stderr, "%s: failed to get Ingress QoS status\n", g_PgmName );

        fprintf(stdout, "BCM Ingress QOS ");
        fprintf(stdout, "Status: %s\n", (status != 0)? "enabled" : "disabled");

        nRet = bcm_iqctl_get_hw_accel_cong_ctrl( &status );

        if ( nRet )
            fprintf( stderr, "%s: failed to get Ingress QoS Congestion Control State\n", g_PgmName );

        fprintf(stdout, "BCM Ingress QOS HW Accel Congestion Control ");
        fprintf(stdout, "Status: %s\n", (status != 0)? "enabled" : "disabled");

    }
    else
        nRet = bcm_iqctl_dump_status_kernel_mode();
    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlEnableHandler
 * Description  : Processes the ingress QoS enable command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlEnableHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet = bcm_iqctl_set_status( IQCTL_STATUS_ENABLE );
    if ( nRet )
        fprintf( stderr, "%s: failed to enable Ingress QoS\n", g_PgmName );
    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: DisableHandler
 * Description  : Processes the ingress QoS disable command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlDisableHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet = bcm_iqctl_set_status( IQCTL_STATUS_DISABLE );
    if ( nRet )
        fprintf( stderr, "%s: failed to disable Ingress QoS\n", g_PgmName );
    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: FlushHandler
 * Description  : Processes the ingress QoS flush command, to remove
 *                dynamic entries.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlFlushHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;

    if ( nNumOptions > 0 )
    {
        fprintf( stderr, "%s: Too many options specified\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }

    nRet = bcm_iqctl_flush();
    if ( nRet )
        fprintf( stderr, "%s: failed to flush Ingress QoS table\n", g_PgmName );
    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlHWAccelCongCtrlEnableHandler
 * Description  : Processes the ingress QoS HW Accel Congestion Control
 *                enable command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlHWAccelCongCtrlEnableHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet = bcm_iqctl_set_hw_accel_cong_ctrl( IQCTL_STATUS_ENABLE );
    if ( nRet )
        fprintf( stderr, "%s: failed to enable Ingress QoS Congestion Control\n", g_PgmName );
    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: DisableHandler
 * Description  : Processes the ingress QoS HW Accel Congestion Control
 *                disable command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlHWAccelCongCtrlDisableHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet = bcm_iqctl_set_hw_accel_cong_ctrl( IQCTL_STATUS_DISABLE );
    if ( nRet )
        fprintf( stderr, "%s: failed to disable Ingress QoS Congestion Control\n", g_PgmName );
    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlAddportHandler
 * Description  : Processes the ingress QoS add L4 port command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlAddportHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    iqctl_data_t iqdata;

    if ( nNumOptions == 0 )
    {
        fprintf( stderr, "%s: No options specified\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }
    else if ( nNumOptions != 4 )
    {
        fprintf( stderr, "%s: incorrect number of options\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    iqdata.ent = IQCTL_ENT_STAT;
    iqdata.action = IQCTL_ACTION_PRIO;

    while ( nNumOptions )
    {
        if ( pOptions->nNumParms != 1 )
        {
            fprintf( stderr, "%s: specify a value\n", g_PgmName );
            return IQCTL_INVALID_OPTION;
        }

        if ( !strcmp( pOptions->pOptName, "--proto") )
        {
            iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
            iqdata.key_data.ip_proto = atoi( pOptions->pParms[0] );
            if (iqdata.key_data.ip_proto == IQCTL_PROTO_TCP)
                iqdata.key_data.ip_proto = IPPROTO_TCP; //6;
            else if (iqdata.key_data.ip_proto == IQCTL_PROTO_UDP)
                iqdata.key_data.ip_proto = IPPROTO_UDP; //17;
            else
            {
                fprintf(stderr, "%s: invalid option and param [%s %s]\n",
                        g_PgmName, pOptions->pOptName,
                        pOptions->pParms[0]);
                return IQCTL_INVALID_OPTION;
            }
        }
        else if ( !strcmp( pOptions->pOptName, "--dport") )
        {
            iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_PORT;
            iqdata.key_data.l4_dst_port = atoi( pOptions->pParms[0] );
        }
        else if ( !strcmp( pOptions->pOptName, "--ent") )
        {
            iqdata.ent = atoi( pOptions->pParms[0] );
        }
        else if ( !strcmp( pOptions->pOptName, "--prio") )
        {
            iqdata.action_value = atoi( pOptions->pParms[0] );
        }
        else
        {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            return IQCTL_INVALID_OPTION;
        }

        nNumOptions--;
        pOptions++;
    }

    nRet = bcm_iqctl_add_key(&iqdata);

    if ( nRet )
        fprintf( stderr, "%s: failed to configure Ingress QoS \n", g_PgmName );

    return nRet;
}


/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlRemportHandler
 * Description  : Processes the ingress QoS remove L4 port command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlRemportHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    iqctl_data_t iqdata;

    if ( nNumOptions == 0 )
    {
        fprintf( stderr, "%s: No options specified\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }
    else if ( nNumOptions != 3 )
    {
        fprintf( stderr, "%s: incorrect number of options\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    iqdata.ent = IQCTL_ENT_STAT;

    while ( nNumOptions )
    {
        if ( pOptions->nNumParms != 1 )
        {
            fprintf( stderr, "%s: specify a value\n", g_PgmName );
            return IQCTL_INVALID_OPTION;
        }

        if ( !strcmp( pOptions->pOptName, "--proto") )
        {
            iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
            iqdata.key_data.ip_proto = atoi( pOptions->pParms[0] );
            if (iqdata.key_data.ip_proto == IQCTL_PROTO_TCP)
                iqdata.key_data.ip_proto = IPPROTO_TCP; //6;
            else if (iqdata.key_data.ip_proto == IQCTL_PROTO_UDP)
                iqdata.key_data.ip_proto = IPPROTO_UDP; //17;
            else
            {
                fprintf(stderr, "%s: invalid option and param [%s %s]\n",
                        g_PgmName, pOptions->pOptName,
                        pOptions->pParms[0]);
                return IQCTL_INVALID_OPTION;
            }
        }
        else if ( !strcmp( pOptions->pOptName, "--dport") )
        {
            iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_PORT;
            iqdata.key_data.l4_dst_port = atoi( pOptions->pParms[0] );
        }
        else if ( !strcmp( pOptions->pOptName, "--ent") )
        {
            iqdata.ent = atoi( pOptions->pParms[0] );
        }
        else
        {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            return IQCTL_INVALID_OPTION;
        }

        nNumOptions--;
        pOptions++;
    }

    nRet = bcm_iqctl_rem_key(&iqdata);

    if ( nRet )
        fprintf( stderr, "%s: failed to configure Ingress QoS \n", g_PgmName );

    return nRet;
}


/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlGetportHandler
 * Description  : Processes the ingress QoS get L4 port config command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlGetportHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    iqctl_data_t iqdata;
    memset(&iqdata, 0x0, sizeof(iqctl_data_t));

    if ( nNumOptions == 0 )
    {
        fprintf( stderr, "%s: No options specified\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }
    else if ( nNumOptions != 2 )
    {
        fprintf( stderr, "%s: incorrect number of options\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }

    while ( nNumOptions )
    {
        if ( pOptions->nNumParms != 1 )
        {
            fprintf( stderr, "%s: specify a value\n", g_PgmName );
            return IQCTL_INVALID_OPTION;
        }

        if ( !strcmp( pOptions->pOptName, "--proto") )
        {
            iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
            iqdata.key_data.ip_proto = atoi( pOptions->pParms[0] );
            if (iqdata.key_data.ip_proto == IQCTL_PROTO_TCP)
                iqdata.key_data.ip_proto = IPPROTO_TCP; //6;
            else if (iqdata.key_data.ip_proto == IQCTL_PROTO_UDP)
                iqdata.key_data.ip_proto = IPPROTO_UDP; //17;
            else
            {
                fprintf(stderr, "%s: invalid option and param [%s %s]\n",
                        g_PgmName, pOptions->pOptName,
                        pOptions->pParms[0]);
                return IQCTL_INVALID_OPTION;
            }
        }
        else if ( !strcmp( pOptions->pOptName, "--dport") )
        {
            iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_PORT;
            iqdata.key_data.l4_dst_port = atoi( pOptions->pParms[0] );
        }
        else
        {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            return IQCTL_INVALID_OPTION;
        }

        nNumOptions--;
        pOptions++;
    }

    nRet = bcm_iqctl_get_key(&iqdata);

    if ( nRet )
    {
        fprintf( stderr, "%s: failed to find the Ingress QoS entry\n", g_PgmName );
        return nRet;
    }

    if (iqdata.action != IQCTL_ACTION_PRIO)
    {
        fprintf( stderr, "%s: failed to find the Ingress QoS entry\n", g_PgmName );
        return -ENOENT;
    }

    fprintf(stdout, "entry is %s, with priority set to %d\n",
            (iqdata.ent == IQCTL_ENT_STAT)? "static" : "dynamic",
            iqdata.action_value);

    return nRet;
}


/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlDumpporttblHandler
 * Description  : Processes the ingress QoS dump L4 port table config command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlDumpporttblHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;

    if ((nNumOptions == 1) && (!strcmp(pOptions->pOptName, "-k")))
    {
        nRet = bcm_iqctl_dump_porttbl_kernel_mode();
    }
    else
    {
        nRet = bcm_iqctl_dump_all();
    }


    if ( nRet )
        fprintf( stderr, "%s: failed to dump Ingress QoS \n", g_PgmName );

    return nRet;
}

#if defined(SUPPORT_RDPA)
static int macAddressParser(char *macaddrStr, char *resultValue)
{
    if (sscanf(macaddrStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%*c", &resultValue[0], &resultValue[1],
               &resultValue[2], &resultValue[3], &resultValue[4], &resultValue[5]) != 6)
    {
        fprintf(stderr, "invalid mac address '%s'\n", macaddrStr);
        return -1;
    }

    return 0;
}

static int ipAddressParser(char *ipaddrStr, int *resultValue, int *is_ipv6)
{
    struct addrinfo hints = {};
    struct addrinfo *res;
    int rc = 0;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    rc = getaddrinfo(ipaddrStr, NULL, &hints, &res);
    if (rc != 0)
    {
        fprintf(stderr, "Failure to parse address '%s' : %s (%d)\n", ipaddrStr,
                gai_strerror(rc), rc);
        return rc;
    }

    if (res == NULL)
    {
        fprintf(stderr, "No host found for '%s'\n", ipaddrStr);
        return -1;
    }

    if (res->ai_family == AF_INET6)
    {
        struct sockaddr_in6 *sa6 = res->ai_addr;
        struct in6_addr *in6 = &sa6->sin6_addr;

        *is_ipv6 = 1;
        memcpy(resultValue, in6->s6_addr, 16);
        resultValue[0] = ntohl(resultValue[0]);
        resultValue[1] = ntohl(resultValue[1]);
        resultValue[2] = ntohl(resultValue[2]);
        resultValue[3] = ntohl(resultValue[3]);
    }
    else if (res->ai_family == AF_INET)
    {
        struct sockaddr_in *sa4 = res->ai_addr;
        struct in_addr *in4 = &sa4->sin_addr;
        *is_ipv6 = 0;
        resultValue[0] = ntohl(in4->s_addr);
    }
    else
    {
        fprintf(stderr, "address not IPv4/6 '%s'\n", ipaddrStr);
        rc = -1;
    }

    freeaddrinfo(res);
    return rc;
}
#endif

#if (defined(SUPPORT_RDPA) || defined(SUPPORT_ARCHERCTL))
static int iqctlParseKeyField(POPTION_INFO pOptions, int nNumOptions,
                              iqctl_data_t *iq, int fCheckParms)
{
    char *end_ptr;
    int rc;

    if ( nNumOptions == 0 )
    {
        fprintf( stderr, "%s: No options specified\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }

    while ( nNumOptions )
    {
#if 0
        if ( !strcmp( pOptions->pOptName, "--indev") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_INGRESS_DEVICE;
            // FIXME!! implement me
        }
        else
#endif
        if ( !strcmp( pOptions->pOptName, "--ipproto") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                iq->key_data.ip_proto = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
        else if ( !strcmp( pOptions->pOptName, "--ethtype") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_ETHER_TYPE;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                iq->key_data.eth_type = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
#if defined(SUPPORT_RDPA)
        else if ( !strcmp( pOptions->pOptName, "--srcmac") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_SRC_MAC;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                rc = macAddressParser(pOptions->pParms[0], iq->key_data.src_mac);
                if (rc)
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
        else if ( !strcmp( pOptions->pOptName, "--dstmac") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_MAC;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                rc = macAddressParser(pOptions->pParms[0], iq->key_data.dst_mac);
                if (rc)
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
        else if ( !strcmp( pOptions->pOptName, "--outervid") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_OUTER_VID;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                iq->key_data.outer_vid = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
        else if ( !strcmp( pOptions->pOptName, "--outerpbit") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_OUTER_PBIT;
            if (( pOptions->nNumParms == 1 ) &&  ( fCheckParms != 0 ))
            {
                iq->key_data.outer_pbit = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
        else if ( !strcmp( pOptions->pOptName, "--innervid") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_INNER_VID;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                iq->key_data.inner_vid = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
        else if ( !strcmp( pOptions->pOptName, "--innerpbit") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_INNER_PBIT;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                iq->key_data.inner_pbit = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
#if 0
        else if ( !strcmp( pOptions->pOptName, "--l2proto") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_L2_PROTO;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                iq->key_data.l2_proto = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
#endif
        else if ( !strcmp( pOptions->pOptName, "--srcip") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_SRC_IP;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                rc = ipAddressParser(pOptions->pParms[0], iq->key_data.src_ip,
                                     &iq->key_data.is_ipv6);
                if (rc)
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
        else if ( !strcmp( pOptions->pOptName, "--dstip") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_IP;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                rc = ipAddressParser(pOptions->pParms[0], iq->key_data.dst_ip,
                                     &iq->key_data.is_ipv6);
                if (rc)
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
        else if ( !strcmp( pOptions->pOptName, "--ipv6flowlabel") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IPV6_FLOW_LABEL;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                iq->key_data.flow_label = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
                if (iq->key_data.flow_label >= (0x1 << 20))
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
        else if ( !strcmp( pOptions->pOptName, "--srcport") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_SRC_PORT;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                iq->key_data.l4_src_port = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
            }
            else if  ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
#endif  // defined(SUPPORT_RDPA)
        else if ( !strcmp( pOptions->pOptName, "--dscp") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_DSCP;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                iq->key_data.dscp = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
                if (iq->key_data.dscp > 63)
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
        else if ( !strcmp( pOptions->pOptName, "--dstport") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_DST_PORT;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                iq->key_data.l4_dst_port = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
#if 0
        else if ( !strcmp( pOptions->pOptName, "--offset0") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_OFFSET_0;
            // FIXME!! implement me
        }
        else if ( !strcmp( pOptions->pOptName, "--offset1") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_OFFSET_1;
            // FIXME!! implement me
        }
#endif
        else if ( !strcmp( pOptions->pOptName, "--l3proto") )
        {
            iq->key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_L3_PROTO;
            if (( pOptions->nNumParms == 1 ) && ( fCheckParms != 0 ))
            {
                iq->key_data.l3_proto = strtol(pOptions->pParms[0], &end_ptr, 0);
                if ((errno != 0) || (end_ptr == pOptions->pParms[0]))
                    return IQCTL_INVALID_OPTION;
            }
            else if ( fCheckParms != 0 )
                return IQCTL_INVALID_OPTION;
        }
        else if ( !strcmp( pOptions->pOptName, "--ent") )
        {
            if ( pOptions->nNumParms != 1 )
            {
                fprintf(stderr, "%s: invalid option [%s]\n",
                        g_PgmName, pOptions->pOptName);
                return IQCTL_INVALID_OPTION;
            }
            iq->ent = atoi( pOptions->pParms[0] );
        }
        else if ( !strcmp( pOptions->pOptName, "--maskprio") )
        {
            if ( pOptions->nNumParms != 1 )
            {
                fprintf(stderr, "%s: invalid option [%s]\n",
                        g_PgmName, pOptions->pOptName);
                return IQCTL_INVALID_OPTION;
            }
            iq->prio = atoi( pOptions->pParms[0] );
            if ((iq->prio < 0) || (iq->prio > 15))
            {
                fprintf(stderr, "%s: value ouf of range [%d]\n",
                        g_PgmName, iq->prio);
                return IQCTL_INVALID_OPTION;
            }
        }
        else if ( !strcmp( pOptions->pOptName, "--prio") )
        {
            if (iq->action != IQCTL_ACTION_NOP)
            {
                fprintf(stderr, "%s: multiple actions are provided [%s]\n",
                        g_PgmName, pOptions->pOptName);
                return IQCTL_INVALID_OPTION;
            }
            if ( pOptions->nNumParms != 1 )
            {
                fprintf(stderr, "%s: invalid option [%s]\n",
                        g_PgmName, pOptions->pOptName);
                return IQCTL_INVALID_OPTION;
            }
            iq->action = IQCTL_ACTION_PRIO;
            iq->action_value = atoi( pOptions->pParms[0] );
        }
#if defined(SUPPORT_RDPA)
        else if ( !strcmp( pOptions->pOptName, "--drop") )
        {
            if (iq->action != IQCTL_ACTION_NOP)
            {
                fprintf(stderr, "%s: multiple actions are provided [%s]\n",
                        g_PgmName, pOptions->pOptName);
                return IQCTL_INVALID_OPTION;
            }
            if ( pOptions->nNumParms != 0 )
            {
                fprintf(stderr, "%s: invalid option [%s]\n",
                        g_PgmName, pOptions->pOptName);
                return IQCTL_INVALID_OPTION;
            }
            iq->action = IQCTL_ACTION_DROP;
        }
        else if ( !strcmp( pOptions->pOptName, "--trap") )
        {
            if (iq->action != IQCTL_ACTION_NOP)
            {
                fprintf(stderr, "%s: multiple actions are provided [%s]\n",
                        g_PgmName, pOptions->pOptName);
                return IQCTL_INVALID_OPTION;
            }
            if ( pOptions->nNumParms != 0 )
            {
                fprintf(stderr, "%s: invalid option [%s]\n",
                        g_PgmName, pOptions->pOptName);
                return IQCTL_INVALID_OPTION;
            }
            iq->action = IQCTL_ACTION_TRAP;
        }
#endif //defined(SUPPORT_RDPA)
        else
        {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            return IQCTL_INVALID_OPTION;
        }

        nNumOptions--;
        pOptions++;
    }

    return IQCTL_SUCCESS;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlAddkeymaskHandler
 * Description  : Processes the ingress QoS adding keymask command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlAddkeymaskHandler(POPTION_INFO pOptions, int nNumOptions)
{
    iqctl_data_t iqdata;
    int nRet;

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    iqdata.prio = -1;
    nRet = iqctlParseKeyField(pOptions, nNumOptions, &iqdata, 0);
    if (nRet != 0)
    {
        fprintf(stderr, "%s: failed to parse parameter\n", g_PgmName);
        return nRet;
    }

    if (iqdata.prio == -1)
    {
        fprintf(stderr, "%s: --maskprio is required paramemter for adding keymask\n", g_PgmName);
        return IQCTL_INVALID_OPTION;
    }

    if (iqdata.key_data.key_field_mask == 0)
    {
        fprintf(stderr, "%s: invalid key field\n", g_PgmName);
        return IQCTL_INVALID_OPTION;
    }

    nRet = bcm_iqctl_add_keymask(&iqdata);

    if (nRet)
        fprintf(stderr, "%s: failed to add keymask\n", g_PgmName);

    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlRemkeymaskHandler
 * Description  : Processes the ingress QoS removing keymask command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlRemkeymaskHandler(POPTION_INFO pOptions, int nNumOptions)
{
    iqctl_data_t iqdata;
    int nRet;

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    nRet = iqctlParseKeyField(pOptions, nNumOptions, &iqdata, 0);
    if (nRet != 0)
    {
        fprintf(stderr, "%s: failed to parse parameter \n", g_PgmName);
        return nRet;
    }

    if (iqdata.key_data.key_field_mask == 0)
    {
        fprintf(stderr, "%s: invalid key field\n", g_PgmName);
        return IQCTL_INVALID_OPTION;
    }

    nRet = bcm_iqctl_rem_keymask(&iqdata);

    if (nRet)
        fprintf(stderr, "%s: failed to remove keymask\n", g_PgmName);

    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlDumpkeymasktblHandler 
 * Description  : Processes the ingress QoS dumping keymask table command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlDumpkeymasktblHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;

    nRet = bcm_iqctl_dump_keymasktbl();

    if ( nRet )
        fprintf( stderr, "%s: failed to dump Ingress QoS \n", g_PgmName );

    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlAddkeyHandler
 * Description  : Processes the ingress QoS adding key command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlAddkeyHandler(POPTION_INFO pOptions, int nNumOptions)
{
    iqctl_data_t iqdata;
    int nRet;

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    nRet = iqctlParseKeyField(pOptions, nNumOptions, &iqdata, 1);
    if ( nRet )
    {
        fprintf( stderr, "%s: failed to parse parameter \n", g_PgmName );
        return nRet;
    }

    if (iqdata.action == IQCTL_ACTION_NOP)
    {
        fprintf( stderr, "%s: failed to add key, please define the action\n", g_PgmName );
        return -1;
    }

    nRet = bcm_iqctl_add_key(&iqdata);

    if ( nRet )
        fprintf( stderr, "%s: failed to add key\n", g_PgmName );

    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlRemkeyHandler
 * Description  : Processes the ingress QoS adding key command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlRemkeyHandler(POPTION_INFO pOptions, int nNumOptions)
{
    iqctl_data_t iqdata;
    int nRet;

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    nRet = iqctlParseKeyField(pOptions, nNumOptions, &iqdata, 1);
    if ( nRet )
    {
        fprintf( stderr, "%s: failed to parse parameter \n", g_PgmName );
        return nRet;
    }

    nRet = bcm_iqctl_rem_key(&iqdata);

    if ( nRet )
        fprintf( stderr, "%s: failed to remove key\n", g_PgmName );

    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlGetkeyHandler
 * Description  : Processes the ingress QoS adding key command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlGetkeyHandler(POPTION_INFO pOptions, int nNumOptions)
{
    iqctl_data_t iqdata;
    int nRet;

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    nRet = iqctlParseKeyField(pOptions, nNumOptions, &iqdata, 1);
    if ( nRet )
    {
        fprintf( stderr, "%s: failed to parse parameter \n", g_PgmName );
        return nRet;
    }

    nRet = bcm_iqctl_get_key(&iqdata);

    if ( nRet )
    {
        fprintf( stderr, "%s: failed to get key\n", g_PgmName );
        return nRet;
    }

    bcm_iqctl_print_key(&iqdata);

    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlDumpkeytblHandler
 * Description  : Processes the ingress QoS dumping key table command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlDumpkeytblHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;

    nRet = bcm_iqctl_dump_keytbl();

    if ( nRet )
        fprintf( stderr, "%s: failed to dump Ingress QoS \n", g_PgmName );

    return nRet;
}
#endif /* defined(SUPPORT_RDPA) || defined(SUPPORT_ARCHERCTL) */

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlSetDefaultPrioHandler
 * Description  : Processes the ingress QoS set default protocol prio command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlSetDefaultPrioHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    iqctl_data_t iqdata;
    iqctl_prototype_t prototype;

    if ( nNumOptions == 0 )
    {
        fprintf( stderr, "%s: No options specified\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }
    else if ( nNumOptions != 3 )
    {
        fprintf( stderr, "%s: incorrect number of options\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    iqdata.ent = IQCTL_ENT_STAT;
    iqdata.action = IQCTL_ACTION_PRIO;

    while ( nNumOptions )
    {
        if ( pOptions->nNumParms != 1 )
        {
            fprintf( stderr, "%s: specify a value\n", g_PgmName );
            return IQCTL_INVALID_OPTION;
        }

        if ( !strcmp( pOptions->pOptName, "--prototype") )
        {
            prototype = atoi( pOptions->pParms[0] );
            if (prototype == IQCTL_PROTOTYPE_IP)
            {
                iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
            }
            else
            {
                fprintf(stderr, "%s: invalid prototype value %d\n",
                        g_PgmName, prototype);
                return IQCTL_INVALID_OPTION;
            }
        }
        else if ( !strcmp( pOptions->pOptName, "--protoval") )
        {
            /* we only support IP Proto with this feature */
            iqdata.key_data.ip_proto = atoi( pOptions->pParms[0] );
        }
        else if ( !strcmp( pOptions->pOptName, "--prio") )
        {
            iqdata.action_value = atoi( pOptions->pParms[0] );
        }
        else
        {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            return IQCTL_INVALID_OPTION;
        }

        nNumOptions--;
        pOptions++;
    }

    nRet = bcm_iqctl_add_key(&iqdata);

    if ( nRet )
        fprintf( stderr, "%s: failed to configure Ingress QoS \n", g_PgmName );

    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlRemDefaultPrioHandler
 * Description  : Processes the ingress QoS set default protocol prio command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int iqctlRemDefaultPrioHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;
    iqctl_data_t iqdata;
    iqctl_prototype_t prototype;

    if ( nNumOptions == 0 )
    {
        fprintf( stderr, "%s: No options specified\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }
    else if ( nNumOptions != 2 )
    {
        fprintf( stderr, "%s: incorrect number of options\n", g_PgmName );
        return IQCTL_INVALID_OPTION;
    }

    memset(&iqdata, 0x0, sizeof(iqctl_data_t));
    iqdata.ent = IQCTL_ENT_STAT;
    iqdata.action = IQCTL_ACTION_PRIO;

    while ( nNumOptions )
    {
        if ( pOptions->nNumParms != 1 )
        {
            fprintf( stderr, "%s: specify a value\n", g_PgmName );
            return IQCTL_INVALID_OPTION;
        }

        if ( !strcmp( pOptions->pOptName, "--prototype") )
        {
            prototype = atoi( pOptions->pParms[0] );
            if (prototype == IQCTL_PROTOTYPE_IP)
            {
                iqdata.key_data.key_field_mask |= 0x1 << IQCTL_KEY_FIELD_IP_PROTO;
            }
            else
            {
                fprintf(stderr, "%s: invalid prototype value %d\n",
                        g_PgmName, prototype);
                return IQCTL_INVALID_OPTION;
            }
        }
        else if ( !strcmp( pOptions->pOptName, "--protoval") )
        {
            /* we only support IP Proto with this feature */
            iqdata.key_data.ip_proto = atoi( pOptions->pParms[0] );
        }
        else
        {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            return IQCTL_INVALID_OPTION;
        }

        nNumOptions--;
        pOptions++;
    }

    nRet = bcm_iqctl_rem_key(&iqdata);

    if ( nRet )
        fprintf( stderr, "%s: failed to configure Ingress QoS \n", g_PgmName );

    return nRet;
}

#if defined(CC_IQ_DEBUG)
/*
 *------------------------------------------------------------------------------
 * Function Name: iqctlDebugHandler
 * Description  : Processes the ingress QoS debug command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int  iqctlDebugHandler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;

    while ( nNumOptions )
    {
        int level;

        if ( pOptions->nNumParms != 1 )
        {
           fprintf( stderr, "%s: did not specify debug level.\n", g_PgmName );
           return IQCTL_INVALID_OPTION;
        }

        level = atoi( pOptions->pParms[0] );

        if ( !strcmp( pOptions->pOptName, "--drv") )
            nRet = iqctlDebug( IQ_DBG_DRV_LAYER, level );
        else if ( !strcmp( pOptions->pOptName, "--iq") )
            nRet = iqctlDebug( IQ_DBG_FC_LAYER, level );
        else
        {
            fprintf( stderr, "%s: invalid option\n", g_PgmName );
            return IQCTL_INVALID_OPTION;
        }
        nNumOptions--;
        pOptions++;

        if ( nRet )
            fprintf( stderr, "%s: failed debug request\n", g_PgmName );
    }

    return nRet;
}
#endif


