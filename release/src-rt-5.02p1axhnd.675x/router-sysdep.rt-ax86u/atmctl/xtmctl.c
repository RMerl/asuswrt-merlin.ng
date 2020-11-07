/***********************************************************************
 *
 *  Copyright (c) 2006-2010  Broadcom Corporation
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
 * File Name  : xtmctl.c
 *
 * Description: Linux command line utility that controls the Broadcom
 *              BCM6368 ATM/PTM driver. It does the following:
 *              - starts and stops the driver
 *              - restarts the XTM driver with its own pre-configuration
 *              & SAR reinitialization actions.
 *              - activates and deactivates an ATM/PTM interface (port)
 *              - adds and removes traffic descriptor table entries
 *              - adds and removes ATM/PTM connections
 *              - displays the configuration for traffic descriptor table
 *                entries, ATM/PTM interfaces and ATM/PTM connections
 *              - displays statistics for ATM/PTM interfaces
 *              - sends an ATM OAM F5 or OAM F4 cell
 *              - creates and deletes an ATM/PTM network device instance
 ***************************************************************************/

/** Includes. **/

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "board.h"
#include "bcm_map_part.h"
#include "cms.h"
#include "devctl_xtm.h"
#include "bcmxtmcfg.h"
#include "bcmnet.h"
#include <unistd.h>
#include <sys/mman.h>

#if defined (__mips__)
#define VIRT_TO_PHY(__virt)                (((unsigned long)(__virt)) & 0x1fffffff)
#endif

/** Defines. **/

#define XTMCTL_VERSION                      "2.7"

/* Default values. */

#define XTMCTL_DEFAULT_TDTE                 \
    {1, TDT_ATM_NO_TRAFFIC_DESCRIPTOR, 0, 0, 0, 0, SC_UBR}

/* Limit values */
#define CMD_NAME_LEN                        16
#define MAX_OPTS                            16
#define MAX_SUB_CMDS                        16
#define MAX_PARMS                           16

/* Argument type values. */
#define ARG_TYPE_COMMAND                    1
#define ARG_TYPE_OPTION                     2
#define ARG_TYPE_PARAMETER                  3

/** More Typedefs. **/

typedef struct
{
    char *pszOptName;
    char *pszParms[MAX_PARMS];
    int nNumParms;
} OPTION_INFO, *POPTION_INFO;

typedef CmsRet (*FN_COMMAND_HANDLER) (POPTION_INFO pOptions, int nNumOptions);

typedef struct
{
    char szCmdName[CMD_NAME_LEN];
    char *pszOptionNames[MAX_OPTS];
    FN_COMMAND_HANDLER pfnCmdHandler;
} COMMAND_INFO, *PCOMMAND_INFO;

/** Prototypes. **/

static int GetArgType( char *pszArg, PCOMMAND_INFO pCmds, char **ppszOptions );
static PCOMMAND_INFO GetCommand( char *pszArg, PCOMMAND_INFO pCmds );
static CmsRet ProcessCommand( PCOMMAND_INFO pCmd, int argc, char **argv,
    PCOMMAND_INFO pCmds, int *pnArgNext );
static CmsRet StartHandler( POPTION_INFO pOptions, int nNumOptions );
static CmsRet StopHandler( POPTION_INFO pOptions, int nNumOptions );
static CmsRet RestartHandler( POPTION_INFO pOptions, int nNumOptions );
static CmsRet OperateHandler( POPTION_INFO pOptions, int nNumOptions );
static CmsRet ConfigHandler( POPTION_INFO pOptions, int nNumOptions );
static CmsRet ThresholdHandler( POPTION_INFO pOptions, int nNumOptions );
static CmsRet TdteHandler( POPTION_INFO pOptions, int nNumOptions );
static CmsRet TdteHandlerAdd( POPTION_INFO pOpt, PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTdt, UINT32 ulTdtSize );
static CmsRet TdteHandlerDelete( POPTION_INFO pOpt, PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTdt, UINT32 ulTdtSize );
static CmsRet TdteHandlerShow( POPTION_INFO pOpt, PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTdt, UINT32 ulTdtSize );
static CmsRet IntfHandler( POPTION_INFO pOptions, int nNumOptions );
static CmsRet IntfHandlerState( POPTION_INFO pOpt );
static CmsRet IntfHandlerShow( POPTION_INFO pOpt );
static CmsRet IntfHandlerStats( POPTION_INFO pOpt );
static CmsRet ConnHandler( POPTION_INFO pOptions, int nNumOptions );
static CmsRet ConnHandlerAdd( POPTION_INFO pOpt );
static CmsRet ConnHandlerDelete( POPTION_INFO pOpt );
static CmsRet ConnHandlerAddQ( POPTION_INFO pOpt );
static CmsRet ConnHandlerDeleteQ( POPTION_INFO pOpt );
static CmsRet ConnHandlerState( POPTION_INFO pOpt );
static CmsRet ConnHandlerShow( POPTION_INFO pOpt );
static CmsRet ConnHandlerSendOam( POPTION_INFO pOpt );
static CmsRet ConnHandlerCreateDevice( POPTION_INFO pOpt );
static CmsRet ConnHandlerDeleteDevice( POPTION_INFO pOpt );
static CmsRet GetConnAddrsToUse( POPTION_INFO pOpt, PXTM_ADDR *ppAddrs,
    UINT32 *pulNumAddrs );
static CmsRet FindConnCfg( PXTM_ADDR pConnAddr, PXTM_ADDR pRetAddr,
    PXTM_CONN_CFG pCfg );
static CmsRet GetConnAddr( char *pszConnAddr, PXTM_ADDR pAddr );
static CmsRet SarHandler( POPTION_INFO pOptions, int nNumOptions ) ;
#if defined(CHIP_63158) || defined(CHIP_63178)
static CmsRet TxDbgHandler( POPTION_INFO pOptions, int nNumOptions );
#endif   //CHIP_63158
static CmsRet BondingHandler( POPTION_INFO pOptions, int nNumOptions ) ;
static CmsRet DbgHandler( POPTION_INFO pOptions, int nNumOptions ) ;
static CmsRet DbgSetInterfaceLinkInfo( POPTION_INFO pOpt);
static CmsRet DbgSARLoopBackConfig( POPTION_INFO pOpt);
static CmsRet BondingHandlerStatus( POPTION_INFO pOpt ) ;
static void GetTrafficTypeStr( UINT32 ulTrafficType, char *pcTrafficTypeStr) ;
static void GetBondingProtocolStr( UINT32 ulBondProto, char *pcBondingProtoStr) ;
static CmsRet VersionHandler( POPTION_INFO pOptions, int nNumOptions );
static CmsRet HelpHandler( POPTION_INFO pOptions, int nNumOptions );
static void DumpMem( int argc, char **argv );
static void SetMem( int argc, char **argv );


/** Globals. **/

static COMMAND_INFO g_Cmds[] =
    {{"start", {"--rq0", "--rq1", "--rq2", "--rq3", "--intf", "--bondingenable", ""}, StartHandler},
     {"stop", {""}, StopHandler},
     {"restart", {""}, RestartHandler},
     {"operate", {""}, OperateHandler},
     {"config", {"--trafficsense", "--singleline", ""}, ConfigHandler},
     {"threshold", {"--adsl", "--vdsl", "--vdslrtx", "--gfast", ""}, ThresholdHandler},
     {"tdte", {"--add", "--delete", "--show", ""}, TdteHandler},
     {"intf", {"--state", "--show", "--stats", ""}, IntfHandler},
     {"conn", {"--add", "--delete", "--addq", "--deleteq", "--state", "--show", 
      "--sendoam", "--createnetdev", "--deletenetdev", ""}, ConnHandler},
     {"sar", {"tx", "rx", "rxcam", "shaper", "rxpbuf", "mib", "rxpaf", "rxbond", "tmuext", "all", ""}, SarHandler},
#if defined(CHIP_63158) || defined(CHIP_63178) 
     {"txdbg",{"bbhfifo", "pktfifo", "utofifo", "all", ""}, TxDbgHandler},
#endif
     {"bonding", {"--status", ""}, BondingHandler},
     {"datapath", {"--status", ""}, BondingHandler},
     {"dbg", {"--SILI", "--SARLB", ""}, DbgHandler},
     {"--version", {""}, VersionHandler},
     {"--help", {""}, HelpHandler},
     {""}
    };

static char g_szPgmName[80] = {0};


/***************************************************************************
 * Function Name: main
 * Description  : Main program function.
 * Returns      : 0 - success, non-0 - error.
 ***************************************************************************/
#ifdef BUILD_STATIC
int xtmctl_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    int nExitCode = 0;
    PCOMMAND_INFO pCmd;

    /* Save the name that started this program into a global variable. */
    strcpy( g_szPgmName, *argv );

    if( !strcmp( argv[0], "dumpmem" ) )
    {
        DumpMem( argc, argv );
        exit( 0 );
    }

    if( !strcmp( argv[0], "setmem" ) )
    {
        SetMem( argc, argv );
        exit( 0 );
    }

    if( argc == 1 )
        HelpHandler( NULL, 0 );

    argc--, argv++;
    while( argc && nExitCode == 0 )
    {
        if( GetArgType( *argv, g_Cmds, NULL ) == ARG_TYPE_COMMAND )
        {
            int argnext = 0;
            pCmd = GetCommand( *argv, g_Cmds );
            nExitCode = ProcessCommand(pCmd, --argc, ++argv, g_Cmds, &argnext);
            argc -= argnext;
            argv += argnext;
        }
        else
        {
            nExitCode = CMSRET_INVALID_ARGUMENTS;
            fprintf( stderr, "%s: invalid command\n", g_szPgmName );
        }
    }

    exit( nExitCode );
}


/***************************************************************************
 * Function Name: GetArgType
 * Description  : Determines if the specified command line argument is a
 *                command, option or option parameter.
 * Returns      : ARG_TYPE_COMMAND, ARG_TYPE_OPTION, ARG_TYPE_PARAMETER
 ***************************************************************************/
static int GetArgType( char *pszArg, PCOMMAND_INFO pCmds, char **ppszOptions )
{
    int nArgType = ARG_TYPE_PARAMETER;

    /* See if the argument is a option. */
    if( ppszOptions )
    {
        do
        {
            if( !strcmp( pszArg, *ppszOptions) )
            {
                nArgType = ARG_TYPE_OPTION;
                break;
            }
        } while( *++ppszOptions );
    }

    /* Next, see if the argument is an command. */
    if( nArgType == ARG_TYPE_PARAMETER )
    {
        while( pCmds->szCmdName[0] != '\0' )
        {
            if( !strcmp( pszArg, pCmds->szCmdName ) )
            {
                nArgType = ARG_TYPE_COMMAND;
                break;
            }

            pCmds++;
        }
    }

    /* Otherwise, assume that it is a parameter. */

    return( nArgType );
} /* GetArgType */


/***************************************************************************
 * Function Name: GetCommand
 * Description  : Returns the COMMAND_INFO structure for the specified
 *                command name.
 * Returns      : COMMAND_INFOR structure pointer
 ***************************************************************************/
static PCOMMAND_INFO GetCommand( char *pszArg, PCOMMAND_INFO pCmds )
{
    PCOMMAND_INFO pCmd = NULL;

    while( pCmds->szCmdName[0] != '\0' )
    {
        if( !strcmp( pszArg, pCmds->szCmdName ) )
        {
            pCmd = pCmds;
            break;
        }

        pCmds++;
    }

    return( pCmd );
} /* GetCommand */


/***************************************************************************
 * Function Name: ProcessCommand
 * Description  : Gets the options and option paramters for a command and
 *                calls the command handler function to process the command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ProcessCommand( PCOMMAND_INFO pCmd, int argc, char **argv,
    PCOMMAND_INFO pCmds, int *pnArgNext )
{
    CmsRet nRet = CMSRET_SUCCESS;
    OPTION_INFO OptInfo[MAX_OPTS];
    OPTION_INFO *pCurrOpt = NULL;
    int nNumOptInfo = 0;
    int nArgType = 0;

    memset( OptInfo, 0x00, sizeof(OptInfo) );
    *pnArgNext = 0;

    do
    {
        if( argc == 0 )
            break;

        nArgType = GetArgType( *argv, pCmds, pCmd->pszOptionNames );
        switch( nArgType )
        {
        case ARG_TYPE_OPTION:
            if( nNumOptInfo < MAX_OPTS )
            {
                pCurrOpt = &OptInfo[nNumOptInfo++];
                pCurrOpt->pszOptName = *argv;
            }
            else
            {
                nRet = CMSRET_INVALID_ARGUMENTS;
                fprintf( stderr, "%s: too many options\n", g_szPgmName );
            }
            (*pnArgNext)++;
            break;

        case ARG_TYPE_PARAMETER:
            if( pCurrOpt && pCurrOpt->nNumParms < MAX_PARMS )
            {
                pCurrOpt->pszParms[pCurrOpt->nNumParms++] = *argv;
            }
            else
            {
                if( pCurrOpt )
                {
                    nRet = CMSRET_INVALID_ARGUMENTS;
                    fprintf( stderr, "%s: invalid option\n", g_szPgmName );
                }
                else
                {
                    nRet = CMSRET_INVALID_ARGUMENTS;
                    fprintf( stderr, "%s: too many options\n", g_szPgmName );
                }
            }
            (*pnArgNext)++;
            break;

        case ARG_TYPE_COMMAND:
            /* The current command is done. */
            break;
        }

        argc--, argv++;
    } while( nRet == CMSRET_SUCCESS && nArgType != ARG_TYPE_COMMAND );


    if( nRet == CMSRET_SUCCESS )
        nRet = (*pCmd->pfnCmdHandler) (OptInfo, nNumOptInfo);

    return( nRet );
} /* ProcessCommand */


/***************************************************************************
 * Function Name: DumpOpt
 * Description  : Debug function that dumps the options and parameters
 *                for a particular command.
 * Returns      : None.
 ***************************************************************************/
void DumpOption( char *pszCmdName, POPTION_INFO pOptions, int nNumOptions )
{
    POPTION_INFO pOpt;
    int i, j;

    fprintf( stderr, "cmd=%s\n", pszCmdName );
    for( i = 0; i < nNumOptions; i++ )
    {
        pOpt = pOptions + i;
        fprintf(stderr, "opt=%s, %d parms=",pOpt->pszOptName,pOpt->nNumParms);
        for( j = 0; j < pOpt->nNumParms; j++ )
        {
            fprintf( stderr, pOpt->pszParms[j] );
            fprintf( stderr, " " );
        }
        fprintf( stderr, "\n" );
    }
    fprintf( stderr, "\n" );
}


/***************************************************************************
 * Function Name: StartHandler
 * Description  : Processes the xtmctl start command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet StartHandler( POPTION_INFO pOptions, int nNumOptions )
{

    CmsRet nRet = CMSRET_SUCCESS;
    XTM_INITIALIZATION_PARMS InitParms;
    UINT32 *pulValue;

    memset(&InitParms, 0x00, sizeof(InitParms));

    while( nRet == CMSRET_SUCCESS && nNumOptions )
    {
        pulValue = NULL;

        if( !strcmp(pOptions->pszOptName, "--rq0") )
        {
            pulValue = &InitParms.ulReceiveQueueSizes[0];
        }
        else if( !strcmp(pOptions->pszOptName, "--rq1") )
        {
            pulValue = &InitParms.ulReceiveQueueSizes[1];
        }
        else if( !strcmp(pOptions->pszOptName, "--intf") )
        {
            if( pOptions->nNumParms == 1 || pOptions->nNumParms == 2 )
            {
                if( !strcmp( pOptions->pszParms[0], "allint" ) )
                    InitParms.ulPortConfig |= PC_ALL_INTERNAL;
                else if( !strcmp( pOptions->pszParms[0], "allext" ) )
                    InitParms.ulPortConfig |= PC_ALL_EXTERNAL;
                else if( !strcmp( pOptions->pszParms[0], "intext" ) )
                    InitParms.ulPortConfig |= PC_INTERNAL_EXTERNAL;
                else 
                    nRet = CMSRET_INVALID_ARGUMENTS;

                if( pOptions->nNumParms == 2 )
                {
                    if( !strcmp( pOptions->pszParms[1], "netedge" ) )
                        InitParms.ulPortConfig |= PC_NEG_EDGE;
                    else
                        nRet = CMSRET_INVALID_ARGUMENTS;
                }
            }
            else
                nRet = CMSRET_INVALID_ARGUMENTS;

            if( nRet != CMSRET_SUCCESS )
                fprintf( stderr, "%s: invalid parameter for option %s\n",
                    g_szPgmName, pOptions->pszOptName );
        }
        else if( !strcmp(pOptions->pszOptName, "--bondingenable") )
        {
            InitParms.bondConfig.sConfig.ptmBond = BC_PTM_BONDING_ENABLE ;
            InitParms.bondConfig.sConfig.atmBond = BC_ATM_BONDING_ENABLE ;
        }
        else
        {
            nRet = CMSRET_INVALID_ARGUMENTS;
            fprintf( stderr, "%s: invalid option %s\n", g_szPgmName,
                pOptions->pszOptName );
        }

        if( pulValue )
        {
            /* Convert the value for a particular option to an integer. */
            if( pOptions->nNumParms == 1 )
            {
                char *pszEnd = NULL;
                *pulValue = (long) strtol( pOptions->pszParms[0], &pszEnd, 10 );
                if( *pszEnd != '\0' )
                {
                    nRet = CMSRET_INVALID_ARGUMENTS;
                    fprintf( stderr, "%s: invalid parameter for option %s\n",
                        g_szPgmName, pOptions->pszOptName );
                }
            }
            else
            {
                nRet = CMSRET_INVALID_ARGUMENTS;
                fprintf( stderr, "%s: invalid number of parameters for option "
                    "%s\n", g_szPgmName, pOptions->pszOptName );
            }
        }

        nNumOptions--;
        pOptions++;
    }

    if( nRet == CMSRET_SUCCESS )
    {
#if defined(SUPPORT_DSL_BONDING)
        /* Set autosense depending on whether or not bonding is enabled. */
        if (InitParms.bondConfig.sConfig.atmBond == BC_ATM_BONDING_ENABLE)
           InitParms.bondConfig.sConfig.autoSenseAtm = BC_ATM_AUTO_SENSE_ENABLE ;
        else
           InitParms.bondConfig.sConfig.autoSenseAtm = BC_ATM_AUTO_SENSE_DISABLE ;  
		   
#endif

        nRet = devCtl_xtmInitialize( &InitParms );
        if( nRet == CMSRET_SUCCESS )
        {
            /* Set a default traffic descriptor table entry. */
            XTM_TRAFFIC_DESCR_PARM_ENTRY Entry = XTMCTL_DEFAULT_TDTE;
            devCtl_xtmSetTrafficDescrTable( &Entry, 1 );
        }
    }

    return( nRet );
} /* StartHandler */

/***************************************************************************
 * Function Name: StopHandler
 * Description  : Processes the xtmctl stop command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet StopHandler( POPTION_INFO pOptions, int nNumOptions )
{
    return( devCtl_xtmUninitialize() );
} /* StopHandler */

/***************************************************************************
 * Function Name: RestartHandler
 * Description  : Processes the xtmctl restart command. No configuration
 * needed.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet RestartHandler( POPTION_INFO pOptions, int nNumOptions )
{
   CmsRet ret ;
   ret = devCtl_xtmReInitialize() ;
   if (ret == CMSRET_SUCCESS) {
      system ("brctl addif br0 ptm0") ;
      system ("ifconfig ptm0 up") ;
   }
   return (ret) ;
} /* RestartHandler */

/***************************************************************************
 * Function Name: OperateHandler
 * Description  : Processes the xtmctl operate command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet OperateHandler( POPTION_INFO pOptions, int nNumOptions )
{
    /* This command does not do anything. Another command will follow.
     * It is included so that the command syntax uses verbs.
     */
    return( 0 );
} /* OperateHandler */





/***************************************************************************
 * Function Name: ConfigHandler
 * Description  : Processes the xtmctl config command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ConfigHandler( POPTION_INFO pOptions, int nNumOptions )
{

    CmsRet nRet = CMSRET_SUCCESS;
    XTM_CONFIGURATION_PARMS ConfigParms;

    /* Begin by setting up the parameter structure to pass to driver */
    memset(&ConfigParms, 0x00, sizeof(ConfigParms));

    /* Step through parameters */
    while( nRet == CMSRET_SUCCESS && nNumOptions )
    {
        int bTrafficSenseFlag = 0;
        int bSingleLineFlag = 0;


        /* Check parameters */
        if( !strcmp(pOptions->pszOptName, "--trafficsense") )
            bTrafficSenseFlag = 1;
        else if( !strcmp(pOptions->pszOptName, "--singleline") )
            bSingleLineFlag = 1;
        else
        {
            nRet = CMSRET_INVALID_ARGUMENTS;
            fprintf( stderr, "%s: invalid option %s\n", g_szPgmName,
                pOptions->pszOptName );
            continue;
        }

        /* We have a valid option.  See if it has a parameter. */
        {
            /* Convert string parameter to integer */
            if( pOptions->nNumParms == 1 )
            {
                char *pszEnd = NULL;
                unsigned long ulParamValue;

                /* Read in timeout */
                ulParamValue = (unsigned long) strtol( pOptions->pszParms[0], &pszEnd, 10 );

                /* Valid timeout? */
                if( *pszEnd != '\0' )
                {
                    /* Nope - flag error */
                    nRet = CMSRET_INVALID_ARGUMENTS;
                    fprintf( stderr, "%s: invalid parameter for option %s\n",
                        g_szPgmName, pOptions->pszOptName );
                }
                else
                {
                    /* Valid timeout.  Flag parameter is to be set */
                    if(bTrafficSenseFlag) {
                        /* Set flag for traffic parameter and timeout parameter value */
                        ConfigParms.sParamsSelected.trafficParam = XTM_CONFIGURATION_PARM_SET;
                        ConfigParms.ulBondingTrafficTimeoutSeconds = ulParamValue;
                    } else if(bSingleLineFlag) {
                        /* Set flag for single line parameter and timeout parameter value */
                        ConfigParms.sParamsSelected.singleLineParam = XTM_CONFIGURATION_PARM_SET;
                        ConfigParms.ulSingleLineTimeoutSeconds = ulParamValue;
                    } else {
                        /* ERROR - we should never get here */
                        fprintf( stderr, "%s: internal error while setting parameter '%s'.  Try again.\n",
                            g_szPgmName, pOptions->pszOptName );
                    }
                }
            }
            else
            {
                /* No timeout - the user just wants to dump the existing value.
                   Ignore the timeout value field. */
                if(bTrafficSenseFlag) {
                    ConfigParms.sParamsSelected.trafficParam = XTM_CONFIGURATION_PARM_DUMP;
                } else if(bSingleLineFlag) {
                    ConfigParms.sParamsSelected.singleLineParam = XTM_CONFIGURATION_PARM_DUMP;
                } else {
                    /* ERROR - we should never get here */
                    fprintf( stderr, "%s: internal error while setting parameter '%s'.  Try again.\n",
                        g_szPgmName, pOptions->pszOptName );
                }
            }

        }

        bTrafficSenseFlag = bSingleLineFlag = 0;  /* Clear flags */
        nNumOptions--;  /* Step to next option */
        pOptions++;
    }

    if( nRet == CMSRET_SUCCESS )
    {
        /* Pass config values to driver */
        nRet = devCtl_xtmConfig( &ConfigParms );
    }

    return( nRet );
} /* ConfigHandler */



/***************************************************************************
 * Function Name: ThresholdHandler
 * Description  : Processes the xtmctl threshold command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/

static CmsRet ThresholdHandler( POPTION_INFO pOptions, int nNumOptions )
{

    CmsRet nRet = CMSRET_SUCCESS;
    XTM_THRESHOLD_PARMS thresholdParms;

    /* Begin by setting up the parameter structure to pass to driver */
    memset(&thresholdParms, 0x00, sizeof(thresholdParms));

    /* Step through parameters */
    while( nRet == CMSRET_SUCCESS && nNumOptions )
    {
        int bAdslFlag, bVdslFlag, bVdslRtxFlag, bGfastFlag;

        bAdslFlag = bVdslFlag = bVdslRtxFlag = bGfastFlag = 0;

        /* Check parameters */
        if( !strcmp(pOptions->pszOptName, "--adsl") )
            bAdslFlag = 1;
        else if( !strcmp(pOptions->pszOptName, "--vdsl") )
            bVdslFlag = 1;
        else if( !strcmp(pOptions->pszOptName, "--vdslrtx") )
            bVdslRtxFlag = 1;
        else if( !strcmp(pOptions->pszOptName, "--gfast") )
            bGfastFlag = 1;
        else
        {
            nRet = CMSRET_INVALID_ARGUMENTS;
            fprintf( stderr, "%s: invalid option %s\n", g_szPgmName,
                pOptions->pszOptName );
            continue;
        }

        /* We have a valid option.  See if it has a parameter. */
        {
            /* Convert string parameter to integer */
            if( pOptions->nNumParms == 1 )
            {
                char *pszEnd = NULL;
                unsigned long ulParamValue;

                /* Read in the threshold value as number. */
                ulParamValue = (unsigned long) strtol( pOptions->pszParms[0], &pszEnd, 10 );

                /* Valid number */
                if( *pszEnd != '\0' )
                {
                    /* Nope - flag error */
                    nRet = CMSRET_INVALID_ARGUMENTS;
                    fprintf( stderr, "%s: invalid parameter for option %s\n",
                        g_szPgmName, pOptions->pszOptName );
                }
                else
                {
                    /* Valid threshold value.  Flag parameter is to be set */
                    if(bAdslFlag) {
                        /* Set flag for ADSL threshold */
                        thresholdParms.sParams.adslParam = XTM_THRESHOLD_PARM_SET;
                        thresholdParms.adslThreshold = ulParamValue;
                    } else if(bVdslFlag) {
                        /* Set flag for VDSL threshold */
                        thresholdParms.sParams.vdslParam = XTM_THRESHOLD_PARM_SET;
                        thresholdParms.vdslThreshold = ulParamValue;
                    } else if(bVdslRtxFlag) {
                        /* Set flag for VDSL-GINP threshold */
                        thresholdParms.sParams.vdslRtxParam = XTM_THRESHOLD_PARM_SET;
                        thresholdParms.vdslRtxThreshold = ulParamValue;
                    } else if(bGfastFlag) {
                        /* Set flag for GFAST threshold */
                        thresholdParms.sParams.gfastParam = XTM_THRESHOLD_PARM_SET;
                        thresholdParms.gfastThreshold = ulParamValue;
                    } else {
                        /* ERROR - we should never get here */
                        fprintf( stderr, "%s: internal error while setting parameter '%s'.  Try again.\n",
                            g_szPgmName, pOptions->pszOptName );
                    }
                } /* Valid threshold */
            }
            else
            {
                /* No valid threshold - the user just wants to dump the existing value.
                   Ignore the timeout value field. */
                if(bAdslFlag) {
                    thresholdParms.sParams.adslParam = XTM_THRESHOLD_PARM_GET;
                } else if(bVdslFlag) {
                    thresholdParms.sParams.vdslParam = XTM_THRESHOLD_PARM_GET;
                } else if(bVdslRtxFlag) {
                    thresholdParms.sParams.vdslRtxParam = XTM_THRESHOLD_PARM_GET;
                } else if(bGfastFlag) {
                    thresholdParms.sParams.gfastParam = XTM_THRESHOLD_PARM_GET;
                } else {
                    /* ERROR - we should never get here */
                    fprintf( stderr, "%s: internal error while setting parameter '%s'.  Try again.\n",
                        g_szPgmName, pOptions->pszOptName );
                }
            }
        }

        bAdslFlag = bVdslFlag = bVdslRtxFlag = bGfastFlag = 0;  /* Clear flags. */
        nNumOptions--;  /* Step to next option */
        pOptions++;
    }

    if( nRet == CMSRET_SUCCESS )
    {
        /* Pass threshold values to driver */
        nRet = devCtl_xtmManageThreshold( &thresholdParms );
    }

    return( nRet );
} /* ConfigHandler */


/***************************************************************************
 * Function Name: TdteHandler
 * Description  : Processes the xtmctl operate tdte command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet TdteHandler( POPTION_INFO pOptions, int nNumOptions )
{
    CmsRet nRet = CMSRET_SUCCESS;
    UINT32 ulTdtSize = 0;
    PXTM_TRAFFIC_DESCR_PARM_ENTRY pTdt = NULL;

    devCtl_xtmGetTrafficDescrTable( NULL, &ulTdtSize );

    /* Allocate memory for "nNumOptions" more traffice descriptor table entries
     * in case the options are all "--add" options.
     */
    pTdt = (PXTM_TRAFFIC_DESCR_PARM_ENTRY) malloc( (ulTdtSize + nNumOptions) *
        sizeof(XTM_TRAFFIC_DESCR_PARM_ENTRY) );

    if( pTdt )
    {
        UINT32 ulChanged = 0;

        if( ulTdtSize )
            nRet = devCtl_xtmGetTrafficDescrTable( pTdt, &ulTdtSize );

        if( nRet == CMSRET_SUCCESS )
        {
            while( nRet == CMSRET_SUCCESS && nNumOptions )
            {
                if( !strcmp( pOptions->pszOptName, "--add" ) )
                {
                    nRet = TdteHandlerAdd( pOptions, pTdt, ulTdtSize );
                    if( nRet == CMSRET_SUCCESS )
                        ulTdtSize++, ulChanged = 1;
                }
                else if( !strcmp( pOptions->pszOptName, "--delete" ) )
                {
                    nRet = TdteHandlerDelete( pOptions, pTdt, ulTdtSize );
                    if( nRet == CMSRET_SUCCESS )
                        ulTdtSize--, ulChanged = 1;
                }
                else if( !strcmp( pOptions->pszOptName, "--show" ) )
                {
                    nRet = TdteHandlerShow( pOptions, pTdt, ulTdtSize );
                }

                nNumOptions--;
                pOptions++;
            }

            if( nRet == CMSRET_SUCCESS && ulChanged == 1 )
                nRet = devCtl_xtmSetTrafficDescrTable( pTdt, ulTdtSize );
        }

        free( pTdt );
    }
    else
    {
        nRet = CMSRET_RESOURCE_EXCEEDED;
        fprintf( stderr, "%s: memory allocation error\n", g_szPgmName );
    }

    return( nRet );
} /* TdteHandler */


/***************************************************************************
 * Function Name: TdteHandlerAdd
 * Description  : Processes the xtmctl operate tdte --add command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet TdteHandlerAdd( POPTION_INFO pOpt, PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTdt, UINT32 ulTdtSize )
{
    CmsRet nRet = CMSRET_SUCCESS;

    if( pOpt->nNumParms > 0 )
    {
        PXTM_TRAFFIC_DESCR_PARM_ENTRY pTdte = NULL;
        UINT32 *pulParm = NULL;
        UINT32 ulNewIndex = 0;
        UINT32 i;
        UINT8 ucIndicies[256];
        int nNumParms;

        memset( ucIndicies, 0x00, sizeof(ucIndicies) ); 

        /* Mark all traffic descriptor table indicies that are in use. */
        for( i = 0, pTdte = pTdt; i < ulTdtSize; i++, pTdte++ )
            ucIndicies[pTdte->ulTrafficDescrIndex & 0xff] = 1;

        /* Find the lowest traffic descriptor table index that is not being
         * used.
         */
        for( i = 1; i < sizeof(ucIndicies); i++ )
            if( ucIndicies[i] == 0 )
            {
                ulNewIndex = i;
                break;
            }

        /* If necessary, move traffic descriptor table entries down by one
         * so the new entry is in ascending order by index.
         */
        for( i = 0, pTdte = pTdt; i < ulTdtSize; i++, pTdte++ )
            if( pTdte->ulTrafficDescrIndex > ulNewIndex )
            {
                memmove( pTdte + 1, pTdte, (ulTdtSize - i) *
                    sizeof(XTM_TRAFFIC_DESCR_PARM_ENTRY) );
                break;
            }

        /* Add the new traffic descriptor table entry. */
        pTdte = pTdt + i;
        memset(pTdte, 0x00, sizeof(XTM_TRAFFIC_DESCR_PARM_ENTRY));
        pTdte->ulTrafficDescrIndex = ulNewIndex;
        pulParm = &pTdte->ulPcr;
        if( !strcmp( pOpt->pszParms[0], "ubr" ) )
        {
            pTdte->ulTrafficDescrType = TDT_ATM_NO_TRAFFIC_DESCRIPTOR;
            pTdte->ulServiceCategory = SC_UBR;
            nNumParms = 1;
        }
        else if( !strcmp( pOpt->pszParms[0], "ubr_pcr" ) )
        {
            pTdte->ulTrafficDescrType = TDT_ATM_NO_CLP_NO_SCR;
            pTdte->ulServiceCategory = SC_UBR;
            nNumParms = 2;
        }
        else if( !strcmp( pOpt->pszParms[0], "cbr" ) )
        {
            pTdte->ulTrafficDescrType = TDT_ATM_NO_CLP_NO_SCR;
            pTdte->ulServiceCategory = SC_CBR;
            nNumParms = 2;
        }
        else if( !strcmp( pOpt->pszParms[0], "rtvbr" ) )
        {
            pTdte->ulTrafficDescrType = TDT_ATM_NO_CLP_SCR;
            pTdte->ulServiceCategory = SC_RT_VBR;
            nNumParms = 4;
        }
        else if( !strcmp( pOpt->pszParms[0], "nrtvbr" ) )
        {
            pTdte->ulTrafficDescrType = TDT_ATM_NO_CLP_SCR;
            pTdte->ulServiceCategory = SC_NRT_VBR;
            nNumParms = 4;
        }
        else if( !strcmp( pOpt->pszParms[0], "mbr" ) )
        {
            pTdte->ulTrafficDescrType = TDT_ATM_PTM_MAX_BIT_RATE_SCR;
            pTdte->ulServiceCategory = SC_MBR;
            nNumParms = 2;
            pulParm = &pTdte->ulScr;
        }
        else
        {
            nRet = CMSRET_INVALID_ARGUMENTS;
            fprintf( stderr, "%s: invalid parameter for option operate tdte "
                "%s\n", g_szPgmName, pOpt->pszOptName );
        }

        if( nRet == CMSRET_SUCCESS )
        {
            if( pOpt->nNumParms == nNumParms ||
                pOpt->nNumParms == (nNumParms + 1) )
            {
                int j;
                char *pszEnd = NULL;

                for( j = 1; j < nNumParms; j++, pulParm++ )
                {
                    pszEnd = NULL;
                    *pulParm = (UINT32) strtol( pOpt->pszParms[j], &pszEnd, 10 );
                    if( *pszEnd != '\0' )
                    {
                        nRet = CMSRET_INVALID_ARGUMENTS;
                        fprintf( stderr, "%s: invalid parameter for option "
                            "operate tdte %s %s\n", g_szPgmName,
                            pOpt->pszOptName, pOpt->pszParms[j] );
                        break;
                    }
                }

                if( pOpt->nNumParms == (nNumParms + 1) )
                {
                    pszEnd = NULL;
                    pTdte->ulMcr = (UINT32) strtol( pOpt->pszParms[nNumParms],
                        &pszEnd, 10 );
                }
            }
            else
            {
                nRet = CMSRET_INVALID_ARGUMENTS;
                fprintf( stderr, "%s: invalid number of parameters for option "
                    "operate tdte %s\n", g_szPgmName, pOpt->pszOptName );
            }
        }

        /* If there was an error, put the traffic descriptor table back to the
         * way it was.
         */
        if( nRet != 0 )
        {
            memmove( pTdte, pTdte + 1, (ulTdtSize - i) *
                sizeof(XTM_TRAFFIC_DESCR_PARM_ENTRY) );
        }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option operate"
            " tdte %s\n", g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* TdteHandlerAdd */


/***************************************************************************
 * Function Name: TdteHandlerDelete
 * Description  : Processes the xtmctl operate tdte --delete command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet TdteHandlerDelete( POPTION_INFO pOpt, PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTdt, UINT32 ulTdtSize )
{
    CmsRet nRet = CMSRET_SUCCESS;

    if( pOpt->nNumParms == 1 )
    {
        PXTM_TRAFFIC_DESCR_PARM_ENTRY pTdte = NULL;
        UINT32 i;
        UINT32 ulIndex = (UINT32) strtol( pOpt->pszParms[0], NULL, 10 ); 

        for( i = 0, pTdte = pTdt; i < ulTdtSize; i++, pTdte++ )
            if( pTdte->ulTrafficDescrIndex == ulIndex )
            {
                memmove( pTdte, pTdte + 1, (ulTdtSize - i - 1) *
                    sizeof(XTM_TRAFFIC_DESCR_PARM_ENTRY) );
                break;
            }

        if( i == ulTdtSize )
        {
            nRet = CMSRET_INVALID_ARGUMENTS;
            fprintf(stderr,"%s: invalid parameter for option operate tdte %s\n",
                g_szPgmName, pOpt->pszOptName );
        }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option operate "
            "tdte %s\n", g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* TdteHandlerDelete */


/***************************************************************************
 * Function Name: TdteHandlerShow
 * Description  : Processes the xtmctl operate tdte --show command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet TdteHandlerShow( POPTION_INFO pOpt, PXTM_TRAFFIC_DESCR_PARM_ENTRY
    pTdt, UINT32 ulTdtSize )
{
    CmsRet nRet = CMSRET_SUCCESS;

    if( pOpt->nNumParms == 0 || pOpt->nNumParms == 1 )
    {
        PXTM_TRAFFIC_DESCR_PARM_ENTRY pTdte = NULL;
        UINT32 i;
        char szType[16];
        UINT32 ulIndex = 0; 

        if( pOpt->nNumParms == 1 )
            ulIndex = (UINT32) strtol( pOpt->pszParms[0], NULL, 10 ); 

        printf( "index   type    pcr     scr     mbs     mcr\n" );
        for( i = 0, pTdte = pTdt; i < ulTdtSize; i++, pTdte++ )
        {
            switch( pTdte->ulServiceCategory )
            {
            case SC_UBR:
                if(pTdte->ulTrafficDescrType == TDT_ATM_NO_TRAFFIC_DESCRIPTOR)
                    strcpy( szType, "ubr" );
                else
                    strcpy( szType, "ubr_pcr" );
                break;

            case SC_CBR:
                strcpy( szType, "cbr" );
                break;

            case SC_RT_VBR:
                strcpy( szType, "rtvbr" );
                break;

            case SC_NRT_VBR:
                strcpy( szType, "nrtvbr" );
                break;

            case SC_MBR:
                strcpy( szType, "mbr" );
                break;

            default:
                sprintf( szType, "type_%d", pTdte->ulServiceCategory );
                break;
            }

            if( pOpt->nNumParms == 0 || pTdte->ulTrafficDescrIndex == ulIndex )
            {
                printf( "%-8u%-8s%-8u%-8u%-8u%-8u\n",
                    pTdte->ulTrafficDescrIndex, szType, pTdte->ulPcr,
                    pTdte->ulScr, pTdte->ulMbs, pTdte->ulMcr );
            }
        }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option operate "
            "tdte %s\n", g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* TdteHandlerShow */


/***************************************************************************
 * Function Name: IntfHandler
 * Description  : Processes the xtmctl operate intf command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet IntfHandler( POPTION_INFO pOptions, int nNumOptions )
{
    CmsRet nRet = CMSRET_SUCCESS;

    while( nRet == CMSRET_SUCCESS && nNumOptions )
    {
        if( !strcmp( pOptions->pszOptName, "--state" ) )
        {
            nRet = IntfHandlerState( pOptions );
        }
        else if( !strcmp( pOptions->pszOptName, "--show" ) )
        {
            nRet = IntfHandlerShow( pOptions );
        }
        else if( !strcmp( pOptions->pszOptName, "--stats" ) )
        {
            nRet = IntfHandlerStats( pOptions );
        }

        nNumOptions--;
        pOptions++;
    }

    return( nRet );
} /* IntfHandler */


/***************************************************************************
 * Function Name: IntfHandlerState
 * Description  : Processes the xtmctl operate intf --state command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet IntfHandlerState( POPTION_INFO pOpt )
{
#if defined(XTM_PORT_SHAPING)
    UINT32 ulPortShaping;
    UINT32 ulShapeRate;
    UINT16 usMbs;
#endif
    CmsRet nRet = CMSRET_SUCCESS;
#if defined(XTM_PORT_SHAPING)
    ulPortShaping = 0;
    ulShapeRate  = 0;
    usMbs        = 0;
    if( pOpt->nNumParms >= 2 )
#else
    if( pOpt->nNumParms == 2 )
#endif
    {
        UINT32 ulPortId = (UINT32) strtol( pOpt->pszParms[0], NULL, 0 ); 
        UINT32 ulPort = PORTID_TO_PORT(ulPortId);
        UINT32 ulAdminStatus = (UINT32) -1;

        if( !strcmp( pOpt->pszParms[1], "enable" ) )
            ulAdminStatus = ADMSTS_UP;
        else
            if( !strcmp( pOpt->pszParms[1], "disable" ) )
                ulAdminStatus = ADMSTS_DOWN;
#if defined(XTM_PORT_SHAPING)
        if( pOpt->pszParms[2] != NULL )
        {
           if( !strcmp(pOpt->pszParms[2], "ratelimit"))
           {
              if( pOpt->pszParms[3] != NULL ) {

                 if( !strcmp(pOpt->pszParms[3], "on" ) )
                 {
                    if( pOpt->nNumParms != 6 )
                       ulPort = MAX_PHY_PORTS ; // Error.

                    ulPortShaping  = PORT_Q_SHAPING_ON;
                    ulShapeRate  = (UINT32) strtol( pOpt->pszParms[4], NULL, 0);
                    usMbs        = (UINT32) strtol( pOpt->pszParms[5], NULL, 0);
                 }
                 else
                 {
                    ulPortShaping  = PORT_Q_SHAPING_OFF;
                    ulShapeRate  = 0;
                    usMbs        = 0;
                 }
              }
              else
                 ulPort = MAX_PHY_PORTS ; // Error.
           }
           else
              ulPort = MAX_PHY_PORTS ; // Error.
        }
#endif
        if( ulPort < MAX_PHY_PORTS && ulAdminStatus != (UINT32) -1 )
        {
            XTM_INTERFACE_CFG Cfg;

            memset( &Cfg, 0x00, sizeof(Cfg) );

            /* Read the current interface configuration. */
            nRet = devCtl_xtmGetInterfaceCfg( ulPortId, &Cfg );
            if( nRet == CMSRET_SUCCESS )
            {
                /* Change the state to the specified value. */
                Cfg.ulIfAdminStatus = ulAdminStatus;
#if defined(XTM_PORT_SHAPING)
                Cfg.ulPortShaping   = ulPortShaping;
                Cfg.ulShapeRate     = ulShapeRate;
                Cfg.usMbs           = usMbs;
#endif
                nRet = devCtl_xtmSetInterfaceCfg( ulPortId, &Cfg );
            }
        }
        else
        {
            nRet = CMSRET_INVALID_ARGUMENTS;
            fprintf(stderr,"%s: invalid parameter for option operate intf %s\n",
                g_szPgmName, pOpt->pszOptName );
        }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option operate "
            "intf %s\n", g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* IntfHandlerState */


/***************************************************************************
 * Function Name: IntfHandlerShow
 * Description  : Processes the xtmctl operate intf --show command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet IntfHandlerShow( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;
    UINT32 ulPortId;
    UINT32 ulPort = (UINT32) -1;

    if( pOpt->nNumParms == 1 )
    {
        ulPortId = (UINT32) strtol( pOpt->pszParms[0], NULL, 0 ); 
        ulPort = PORTID_TO_PORT(ulPortId);
    }
    
    if(pOpt->nNumParms <= 1 && (ulPort < MAX_PHY_PORTS || ulPort == (UINT32)-1))
    {
        XTM_INTERFACE_CFG Cfg;
        UINT32 i;

        printf( "portid  port    status      type\n" );
        for( i = 0; i < MAX_PHY_PORTS; i++ )
        {
            if((ulPort == i || ulPort == (UINT32) -1) &&
               devCtl_xtmGetInterfaceCfg(PORT_TO_PORTID(i),&Cfg) == CMSRET_SUCCESS)
            {
                char *pszAdminStatuses[] = {"", "enabled","disabled"};
                char *pszTrafficTypes[] = {"", "atm", "ptm"};
                char *pszAdminStatus = "";
                char *pszTrafficType = "";

                if( Cfg.ulIfAdminStatus <= ADMSTS_DOWN )
                    pszAdminStatus = pszAdminStatuses[Cfg.ulIfAdminStatus];
                else
                    pszAdminStatus = "";

                if( Cfg.usIfTrafficType <= TRAFFIC_TYPE_PTM )
                    pszTrafficType = pszTrafficTypes[Cfg.usIfTrafficType];
                else
                    pszTrafficType = "";

                printf("%-8d%-8d%-8s%-12s\n", PORT_TO_PORTID(i), i,
                    pszAdminStatus, pszTrafficType);
            }
        }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid parameter for option operate intf %s\n",
            g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* IntfHandlerShow */


/***************************************************************************
 * Function Name: IntfHandlerStats
 * Description  : Processes the xtmctl operate intf --stats command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet IntfHandlerStats( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;
    UINT32 ulPortId;
    UINT32 ulPort = (UINT32) -1;
    UINT32 ulReset = 0;

    if( pOpt->nNumParms == 1 )
    {
        if( !strcmp( pOpt->pszParms[0], "reset" ) )
        {
            ulReset = 1;
        }
        else
        { 
            ulPortId = (UINT32) strtol( pOpt->pszParms[0], NULL, 0 ); 
            ulPort = PORTID_TO_PORT(ulPortId);
        }
    }
    else if( pOpt->nNumParms == 2 ) 
    {
        ulPortId = (UINT32) strtol( pOpt->pszParms[0], NULL, 0 ); 
        ulPort = PORTID_TO_PORT(ulPortId);
        
        if( !strcmp( pOpt->pszParms[1], "reset" ) )
        {
            ulReset = 1;
        }
        else
        {
            nRet = CMSRET_INVALID_ARGUMENTS;
        }
    }

    if( nRet == CMSRET_SUCCESS &&
        pOpt->nNumParms <= 2 &&
        (ulPort < MAX_PHY_PORTS || ulPort == (UINT32)-1) )
    {
        XTM_INTERFACE_CFG Cfg;
        XTM_INTERFACE_STATS Stats;
        XTM_ERROR_STATS ErrStats;
        UINT32 i;

        for( i = 0; i < MAX_PHY_PORTS; i++ )
        {
           if ((i == ulPort) || (ulPort == (UINT32) -1))
           {
              if (devCtl_xtmGetInterfaceCfg( PORT_TO_PORTID(i), &Cfg ) != CMSRET_SUCCESS)
              {
                 printf("Unable to get port's configuration/status for port %d\n",i);
              }
              else if (Cfg.ulIfOperStatus != OPRSTS_UP)
              {
                 printf("Port %d is down\n",i);
              }
              else if (devCtl_xtmGetInterfaceStatistics(PORT_TO_PORTID(i), &Stats,ulReset) != CMSRET_SUCCESS )
              {
                 printf("Unable to get port's statistics for port %d\n",i);
              }
              else
              {
                 printf( "atm/ptm interface statistics for port %d\n"
                         "in octets                   %u\n"
                         "out octets                  %u\n"
                         "in packets                  %u\n"
                         "out packets                 %u\n"
                         "in OAM cells                %u\n"
                         "out OAM cells               %u\n"
                         "in ASM cells                %u\n"
                         "out ASM cells               %u\n"
                         "in packet errors            %u\n"
                         "in cell errors              %u\n\n",
                         i,
                         Stats.ulIfInOctets,
                         Stats.ulIfOutOctets,
                         Stats.ulIfInPackets,
                         Stats.ulIfOutPackets,
                         Stats.ulIfInOamRmCells,
                         Stats.ulIfOutOamRmCells,
                         Stats.ulIfInAsmCells,
                         Stats.ulIfOutAsmCells,
                         Stats.ulIfInPacketErrors,
                         Stats.ulIfInCellErrors );
              } /* got stats */
           } /* if ulPort == i */
        } /* for loop of each port */

        /* Print out error statistics for whole device */

        if (devCtl_xtmGetErrorStatistics(&ErrStats) != CMSRET_SUCCESS )
        {
           printf("Unable to get device's error statistics\n");
        }
        else
        {
           printf( "\nerror statistics\n"
                   "PAF errors                  %u\n"
                   "PAF lost fragments          %u\n"
                   "overflow errors             %u\n"
                   "frames dropped              %u\n\n",
                   ErrStats.ulPafErrs,
                   ErrStats.ulPafLostFragments,
                   ErrStats.ulOverflowErrorsRx,
                   ErrStats.ulFramesDropped );
        } /* got error stats */
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid parameter for option operate intf %s\n",
            g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* IntfHandlerStats */


/***************************************************************************
 * Function Name: ConnHandler
 * Description  : Processes the xtmctl operate an ATM/PTM connection command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ConnHandler( POPTION_INFO pOptions, int nNumOptions )
{
    CmsRet nRet = CMSRET_SUCCESS;

    while( nRet == CMSRET_SUCCESS && nNumOptions )
    {
        if( !strcmp( pOptions->pszOptName, "--add" ) )
        {
            nRet = ConnHandlerAdd( pOptions );
        }
        else if( !strcmp( pOptions->pszOptName, "--delete" ) )
        {
            nRet = ConnHandlerDelete( pOptions );
        }
        else if( !strcmp( pOptions->pszOptName, "--addq" ) )
        {
            nRet = ConnHandlerAddQ( pOptions );
        }
        else if( !strcmp( pOptions->pszOptName, "--deleteq" ) )
        {
            nRet = ConnHandlerDeleteQ( pOptions );
        }
        else if( !strcmp( pOptions->pszOptName, "--state" ) )
        {
            nRet = ConnHandlerState( pOptions );
        }
        else if( !strcmp( pOptions->pszOptName, "--show" ) )
        {
            nRet = ConnHandlerShow( pOptions );
        }
        else if( !strcmp( pOptions->pszOptName, "--sendoam" ) )
        {
            nRet = ConnHandlerSendOam( pOptions );
        }
        else if( !strcmp( pOptions->pszOptName, "--createnetdev" ) )
        {
            nRet = ConnHandlerCreateDevice( pOptions );
        }
        else if( !strcmp( pOptions->pszOptName, "--deletenetdev" ) )
        {
            nRet = ConnHandlerDeleteDevice( pOptions );
        }

        nNumOptions--;
        pOptions++;
    }

    return( nRet );
} /* ConnHandler */


/***************************************************************************
 * Function Name: ConnHandlerAdd
 * Description  : Processes the xtmctl operate conn --add command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ConnHandlerAdd( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;
    XTM_ADDR Addr;
    XTM_CONN_CFG Cfg, Cfg2;
    char *pszEnd = NULL;
    SINT32 minNumParms = 1;

    if( pOpt->nNumParms < minNumParms )
        goto missing_parms;

    nRet = GetConnAddr( pOpt->pszParms[0], &Addr );
    if( nRet != CMSRET_SUCCESS )
        goto out;

    memset( &Cfg, 0x00, sizeof(Cfg) );
    Cfg.ulHeaderType = HT_LLC_SNAP_ETHERNET;
    Cfg.ConnArbs[0][0].ulSubPriority = 0;
    Cfg.ConnArbs[0][0].ulWeightValue = 1;
    Cfg.ConnArbs[0][0].ulWeightAlg   = WA_CWRR;

    if( Addr.ulTrafficType == TRAFFIC_TYPE_ATM )
    {
        minNumParms = 2;
        if( pOpt->nNumParms < minNumParms )
            goto missing_parms;
         
        if( !strcmp( pOpt->pszParms[1], "aal5" ) )
        {
            Cfg.ulAtmAalType = AAL_5;

#if defined(CHIP_63268) || defined(CHIP_63138) || defined(CHIP_63381) || defined(CHIP_63148) || defined(CHIP_63158) || defined(CHIP_63178)
            minNumParms = 5;
#else
            minNumParms = 3;
#endif
            if( pOpt->nNumParms < minNumParms )
                goto missing_parms;

            if( !strcmp( pOpt->pszParms[2], "llcsnap_eth" ) )
                Cfg.ulHeaderType = HT_LLC_SNAP_ETHERNET;
            else if( !strcmp( pOpt->pszParms[2], "llcsnap_rtip" ) )
                Cfg.ulHeaderType = HT_LLC_SNAP_ROUTE_IP;
            else if( !strcmp( pOpt->pszParms[2], "llcencaps_ppp" ) )
                Cfg.ulHeaderType = HT_LLC_ENCAPS_PPP;
            else if( !strcmp( pOpt->pszParms[2], "vcmux_eth" ) )
                Cfg.ulHeaderType = HT_VC_MUX_ETHERNET;
            else if( !strcmp( pOpt->pszParms[2], "vcmux_ipoa" ) )
                Cfg.ulHeaderType = HT_VC_MUX_IPOA;
            else if( !strcmp( pOpt->pszParms[2], "vcmux_pppoa" ) )
                Cfg.ulHeaderType = HT_VC_MUX_PPPOA;
            else
            {
                fprintf( stderr, "%s: invalid parameter 3 for "
                    "option operate conn %s\n", g_szPgmName,
                    pOpt->pszOptName );
                goto errout;
            }
#if defined(CHIP_63268) || defined(CHIP_63138) || defined(CHIP_63381) || defined(CHIP_63148) || defined(CHIP_63158) || defined(CHIP_63178)
            Cfg.ConnArbs[0][0].ulSubPriority = (UINT32)
                    strtol( pOpt->pszParms[3], &pszEnd, 10 );
            if( *pszEnd != '\0' )
            {
                fprintf( stderr, "%s: invalid parameter 4 for option "
                    "operate conn %s\n",g_szPgmName,pOpt->pszOptName );
                goto errout;
            }
            Cfg.ConnArbs[0][0].ulWeightValue = (UINT32)
                    strtol( pOpt->pszParms[4], &pszEnd, 10 );
            if( *pszEnd != '\0' )
            {
                fprintf( stderr, "%s: invalid parameter 5 for option "
                    "operate conn %s\n",g_szPgmName,pOpt->pszOptName );
                goto errout;
            }
#endif
        }
        else if( !strcmp( pOpt->pszParms[1], "aal0pkt" ) )
        {
            Cfg.ulAtmAalType = AAL_0_PACKET;
        }
        else if( !strcmp( pOpt->pszParms[1], "aal0cell" ) )
        {
            Cfg.ulAtmAalType = AAL_0_CELL;
        }
        else if( !strcmp( pOpt->pszParms[1], "aaltransparent" ) )
        {
            Cfg.ulAtmAalType = AAL_TRANSPARENT;
        }
        else
        {
            fprintf( stderr, "%s: invalid parameter 2 for option "
                "operate conn %s\n", g_szPgmName, pOpt->pszOptName );
            goto errout;
        }
        
        if( pOpt->nNumParms > minNumParms )
        {
            /* the optional tdte index is specified */
            Cfg.ulTransmitTrafficDescrIndex = (UINT32)
                strtol( pOpt->pszParms[minNumParms], &pszEnd, 10 );
            if( *pszEnd != '\0' )
            {
                fprintf( stderr, "%s: invalid parameter %d for option "
                    "operate conn %s\n",g_szPgmName,minNumParms,pOpt->pszOptName );
                goto errout;
            }
        }
    }
    /* don't need other parameters for PTM mode */


    /* Read the current configuration. */
    nRet = devCtl_xtmGetConnCfg( &Addr, &Cfg2 );
    if( nRet == CMSRET_SUCCESS )
    {
        /* Copy queue information to new configuration. */
        Cfg.ulTransmitQParmsSize = Cfg2.ulTransmitQParmsSize;
        memcpy( Cfg.TransmitQParms, Cfg2.TransmitQParms,
            Cfg.ulTransmitQParmsSize *
            sizeof(XTM_TRANSMIT_QUEUE_PARMS) );
    }
    else
        Cfg.ulAdminStatus = ADMSTS_UP;

    nRet = devCtl_xtmSetConnCfg( &Addr, &Cfg );

    goto out;

missing_parms:
    fprintf( stderr, "%s: invalid number of parameters for "
        "option operate conn %s\n", g_szPgmName,
        pOpt->pszOptName );
errout:
    nRet = CMSRET_INVALID_ARGUMENTS;
out:                    
    return( nRet );
} /* ConnHandlerAdd */


/***************************************************************************
 * Function Name: ConnHandlerDelete
 * Description  : Processes the xtmctl operate conn --delete command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ConnHandlerDelete( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;

    if( pOpt->nNumParms == 1 )
    {
        XTM_ADDR Addr;

        nRet = GetConnAddr( pOpt->pszParms[0], &Addr );
        if( nRet == CMSRET_SUCCESS )
        {
            nRet = devCtl_xtmSetConnCfg( &Addr, NULL );
        }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option operate "
            "conn %s\n", g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* ConnHandlerDelete */


/***************************************************************************
 * Function Name: ConnHandlerAddQ
 * Description  : Processes the xtmctl operate conn --addq command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ConnHandlerAddQ( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;
    XTM_ADDR Addr;
    XTM_ADDR ConnAddr;
    XTM_CONN_CFG Cfg;
    UINT32 ulMaxQueues;
    PXTM_TRANSMIT_QUEUE_PARMS pTxQParms;
    UINT32 ulNewIndex  = 0;
    UINT32 i;
    UINT8  ucIndicies[MAX_TRANSMIT_QUEUES] = {0};

#if defined(CHIP_63268) || defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_63158) || defined(CHIP_63178)
    /* Basic parameters include:
     * <conn addr> <queue priority> <scheduler> <queue weight> <drop alg>
     */
    if( pOpt->nNumParms < 5)
#else
    /* Basic parameters include:
     * <conn addr> <queue priority> <scheduler> <queue weight>
     */
    if( pOpt->nNumParms < 4)
#endif
        goto missing_parms;

    nRet = GetConnAddr( pOpt->pszParms[0], &Addr );
    if( nRet != CMSRET_SUCCESS )
        goto out;

    memset( &Cfg, 0x00, sizeof(Cfg) );

    nRet = FindConnCfg( &Addr, &ConnAddr, &Cfg );
    if( nRet != CMSRET_SUCCESS )
        goto out;

    if( Addr.ulTrafficType == TRAFFIC_TYPE_ATM )
        ulMaxQueues = MAX_ATM_TRANSMIT_QUEUES;
    else
        ulMaxQueues = MAX_PTM_TRANSMIT_QUEUES;

    /* Add the new queue parameters entry. */
    if( Cfg.ulTransmitQParmsSize >= ulMaxQueues )
    {
        fprintf( stderr, "%s: Too many queues for this "
            "connection.\n", g_szPgmName );
        goto errout;
    }

    pTxQParms = &Cfg.TransmitQParms[0];

    /* Mark all queue ids that are in use. */
    for( i = 0; i < Cfg.ulTransmitQParmsSize; i++, pTxQParms++ )
        ucIndicies[pTxQParms->ucQosQId] = 1;

    /* Find the lowest queue id that is not being used. */
    for( i = 0; i < ulMaxQueues; i++ )
        if( ucIndicies[i] == 0 )
        {
            ulNewIndex = i;
            break;
        }

    if( i == ulMaxQueues )
    {
        fprintf( stderr, "%s: Could not allocate an index for this "
            "queue.\n", g_szPgmName );
        goto errout;
    }

    pTxQParms = &Cfg.TransmitQParms[Cfg.ulTransmitQParmsSize++];

    pTxQParms->usSize             = HOST_XTM_NR_TXBDS;
    pTxQParms->ucSubPriority      = (UINT8)strtol( pOpt->pszParms[1], NULL, 10 );
    pTxQParms->ucQosQId           = ulNewIndex;
    pTxQParms->ucWeightAlg        = WA_CWRR; /* default */
    pTxQParms->ulWeightValue      = 1; /* default */
    pTxQParms->ulMinBitRate       = 0; /* default is no shaping */
    pTxQParms->ulShapingRate      = 0; /* default is no shaping */
    pTxQParms->usShapingBurstSize = 0; /* default */
    pTxQParms->ucDropAlg          = WA_DT;   /* default */
    pTxQParms->ucLoMinThresh      = 0; /* default is DT */
    pTxQParms->ucLoMaxThresh      = 0; /* default is DT */
    pTxQParms->ucHiMinThresh      = 0; /* default is DT */
    pTxQParms->ucHiMaxThresh      = 0; /* default is DT */

    if( !strcmp( pOpt->pszParms[2], "rr" ) )
    {
        pTxQParms->ucWeightAlg   = WA_CWRR;
        pTxQParms->ulWeightValue = 1;
    }
    else if( !strcmp( pOpt->pszParms[2], "wrr" ) )
    {
        pTxQParms->ucWeightAlg   = WA_CWRR;
        pTxQParms->ulWeightValue = (UINT32)strtol( pOpt->pszParms[3], NULL, 10 );
    }
    else if( !strcmp( pOpt->pszParms[2], "wfq" ) )
    {
        pTxQParms->ucWeightAlg   = WA_WFQ;
        pTxQParms->ulWeightValue = (UINT32)strtol( pOpt->pszParms[3], NULL, 10 );
    }
    else
    {
        fprintf( stderr, "%s: invalid scheduling algorithm for "
            "option operate conn %s\n", g_szPgmName, pOpt->pszOptName );
        goto errout;
    }

#if defined(CHIP_63268) || defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_63158) || defined(CHIP_63178)
    if( !strcmp( pOpt->pszParms[4], "dt" ) )
    {
        if( Addr.ulTrafficType != TRAFFIC_TYPE_ATM )
        {
            /* For PTM mode, remaining parameters shall include:
             * <mbr_kbps>, <pbr_kbps> and <mbs_byte>
             */
            if( pOpt->nNumParms < 8 )
                goto missing_parms;

            pTxQParms->ulMinBitRate  = 1000 * (UINT32)strtol( pOpt->pszParms[5], NULL, 10 );
            pTxQParms->ulShapingRate = 1000 * (UINT32)strtol( pOpt->pszParms[6], NULL, 10 );
            pTxQParms->usShapingBurstSize = (UINT16)strtol( pOpt->pszParms[7], NULL, 10 );
        }
    }
    else if( !strcmp( pOpt->pszParms[4], "red" ) )
    {
        pTxQParms->ucDropAlg = WA_RED;

        /* Remaining parameters shall include:
         * <minThr>, <maxThr>
         */
        if( pOpt->nNumParms < 7 )
            goto missing_parms;

        pTxQParms->ucLoMinThresh = (UINT8)strtol( pOpt->pszParms[5], NULL, 10 );
        pTxQParms->ucLoMaxThresh = (UINT8)strtol( pOpt->pszParms[6], NULL, 10 );
        pTxQParms->ucHiMinThresh = pTxQParms->ucLoMinThresh;
        pTxQParms->ucHiMaxThresh = pTxQParms->ucLoMaxThresh;

        if( pTxQParms->ucLoMinThresh > pTxQParms->ucLoMaxThresh )
        {
            fprintf( stderr, "%s: minThresh > maxThresh for "
                "option operate conn %s\n", g_szPgmName, pOpt->pszOptName );
            goto errout;
        }
        
        if( Addr.ulTrafficType != TRAFFIC_TYPE_ATM )
        {
            /* For PTM mode, remaining parameters shall include:
             * <mbr_kbps>, <pbr_kbps> and <mbs_byte>
             */
            if( pOpt->nNumParms < 10 )
                goto missing_parms;

            pTxQParms->ulMinBitRate  = 1000 * (UINT32)strtol( pOpt->pszParms[7], NULL, 10 );
            pTxQParms->ulShapingRate = 1000 * (UINT32)strtol( pOpt->pszParms[8], NULL, 10 );
            pTxQParms->usShapingBurstSize = (UINT16)strtol( pOpt->pszParms[9], NULL, 10 );
        }
    }
    else if( !strcmp( pOpt->pszParms[4], "wred" ) )
    {
        pTxQParms->ucDropAlg = WA_WRED;

        /* Remaining parameters shall include:
         * <loMinThr>, <loMaxThr>, <hiMinThr>, <hiMaxThr>
         */
        if( pOpt->nNumParms < 9 )
            goto missing_parms;

        pTxQParms->ucLoMinThresh = (UINT8)strtol( pOpt->pszParms[5], NULL, 10 );
        pTxQParms->ucLoMaxThresh = (UINT8)strtol( pOpt->pszParms[6], NULL, 10 );
        pTxQParms->ucHiMinThresh = (UINT8)strtol( pOpt->pszParms[7], NULL, 10 );
        pTxQParms->ucHiMaxThresh = (UINT8)strtol( pOpt->pszParms[8], NULL, 10 );

        if( pTxQParms->ucLoMinThresh > pTxQParms->ucLoMaxThresh )
        {
            fprintf( stderr, "%s: loMinThresh > loMaxThresh for "
                "option operate conn %s\n", g_szPgmName, pOpt->pszOptName );
            goto errout;
        }
        if( pTxQParms->ucHiMinThresh > pTxQParms->ucHiMaxThresh )
        {
            fprintf( stderr, "%s: hiMinThresh > hiMaxThresh for "
                "option operate conn %s\n", g_szPgmName, pOpt->pszOptName );
            goto errout;
        }

        if( Addr.ulTrafficType != TRAFFIC_TYPE_ATM )
        {
            /* For PTM mode, remaining parameters shall include:
             * <mbr_kbps>, <pbr_kbps> and <mbs_byte>
             */
            if( pOpt->nNumParms < 12 )
                goto missing_parms;
            
            pTxQParms->ulMinBitRate  = 1000 * (UINT32)strtol( pOpt->pszParms[9], NULL, 10 );
            pTxQParms->ulShapingRate = 1000 * (UINT32)strtol( pOpt->pszParms[10], NULL, 10 );
            pTxQParms->usShapingBurstSize = (UINT16)strtol( pOpt->pszParms[11], NULL, 10 );
        }
    }
    else
    {
        fprintf( stderr, "%s: invalid drop algorithm for "
            "option operate conn %s\n", g_szPgmName, pOpt->pszOptName );
        goto errout;
    }
#elif defined(CHIP_63381)
    if( Addr.ulTrafficType != TRAFFIC_TYPE_ATM )
    {
        /* For PTM mode, remaining parameters shall include:
         * <mbr_kbps>, <pbr_kbps> and <mbs_byte>
         */
        if( pOpt->nNumParms < 7 )
            goto missing_parms;

        pTxQParms->ulMinBitRate  = 1000 * (UINT32)strtol( pOpt->pszParms[4], NULL, 10 );
        pTxQParms->ulShapingRate = 1000 * (UINT32)strtol( pOpt->pszParms[5], NULL, 10 );
        pTxQParms->usShapingBurstSize = (UINT16)strtol( pOpt->pszParms[6], NULL, 10 );
    }
#endif
    
    if( Addr.ulTrafficType == TRAFFIC_TYPE_ATM )
    {
        pTxQParms->ulPortId = Addr.u.Vcc.ulPortMask;
    }
    else
    {
        pTxQParms->ulPortId      = Addr.u.Flow.ulPortMask;
        pTxQParms->ulPtmPriority = (UINT32)Addr.u.Flow.ulPtmPriority;
    }

    /* Set the updated configuration. */
    nRet = devCtl_xtmSetConnCfg( &ConnAddr, &Cfg );

    goto out;

missing_parms:
    fprintf( stderr, "%s: invalid number of parameters for option operate "
            "conn %s\n", g_szPgmName, pOpt->pszOptName );
errout:
    nRet = CMSRET_INVALID_ARGUMENTS;
out:
    return( nRet );
} /* ConnHandlerAddQ */


/***************************************************************************
 * Function Name: ConnHandlerDeleteQ
 * Description  : Processes the xtmctl operate conn --deleteq command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ConnHandlerDeleteQ( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;

    if( pOpt->nNumParms >= 2 )
    {
        XTM_ADDR Addr;

        nRet = GetConnAddr( pOpt->pszParms[0], &Addr );
        if( nRet == CMSRET_SUCCESS )
        {
            XTM_ADDR ConnAddr;
            XTM_CONN_CFG Cfg;
            
            memset( &Cfg, 0x00, sizeof(Cfg) );

            /* Read the current configuration. */
            nRet = FindConnCfg( &Addr, &ConnAddr, &Cfg );
            if( nRet == CMSRET_SUCCESS )
            {
                UINT32 ulPortId = (Addr.ulTrafficType == TRAFFIC_TYPE_ATM) ?
                                       Addr.u.Vcc.ulPortMask : Addr.u.Flow.ulPortMask;
                UINT8  ucQosQId = (UINT8)strtol( pOpt->pszParms[1], NULL, 10 );

                UINT32 i, ulFound;
                PXTM_TRANSMIT_QUEUE_PARMS pTxQParms;

                /* Find the queue to delete. */
                for( i = 0, pTxQParms = Cfg.TransmitQParms, ulFound = 0;
                     i < Cfg.ulTransmitQParmsSize && ulFound == 0;
                     i++, pTxQParms++ )
                {
                    if( ulPortId == pTxQParms->ulPortId &&
                        ucQosQId == pTxQParms->ucQosQId )
                    {
                        if( Addr.ulTrafficType == TRAFFIC_TYPE_PTM &&
                            Addr.u.Flow.ulPtmPriority != pTxQParms->ulPtmPriority)
                            continue;

                        Cfg.ulTransmitQParmsSize--;
                        memmove( pTxQParms, pTxQParms + 1,
                            (Cfg.ulTransmitQParmsSize - i) *
                            sizeof(XTM_TRANSMIT_QUEUE_PARMS) );
                        ulFound = 1;
                    }
                }

                if( ulFound )
                {
                    /* Set the updated configuration. */
                    nRet = devCtl_xtmSetConnCfg( &ConnAddr, &Cfg );
                }
                else
                {
                    nRet = CMSRET_INVALID_ARGUMENTS;
                    fprintf(stderr, "%s: Queue was not found.\n", g_szPgmName);
                }
            }
            else
            {
                nRet = CMSRET_INVALID_ARGUMENTS;
                fprintf(stderr,"%s: The connection has not been configured.\n",
                    g_szPgmName );
            }
        }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option operate "
            "conn %s\n", g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* ConnHandlerDeleteQ */


/***************************************************************************
 * Function Name: ConnHandlerState
 * Description  : Processes the xtmctl operate conn --state command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ConnHandlerState( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;

    if( pOpt->nNumParms == 2 )
    {
        XTM_ADDR Addr;
        UINT32 ulAdminStatus = (UINT32) -1;

        if( !strcmp( pOpt->pszParms[1], "enable" ) )
            ulAdminStatus = ADMSTS_UP;
        else
            if( !strcmp( pOpt->pszParms[1], "disable" ) )
                ulAdminStatus = ADMSTS_DOWN;

        nRet = GetConnAddr( pOpt->pszParms[0], &Addr );

        if( nRet == CMSRET_SUCCESS && ulAdminStatus != (UINT32) -1 )
        {
            XTM_CONN_CFG Cfg;

            memset( &Cfg, 0x00, sizeof(Cfg) );

            /* Read the current connection configuration. */
            nRet = devCtl_xtmGetConnCfg( &Addr, &Cfg );
            if( nRet == CMSRET_SUCCESS )
            {
                /* Change the state to the specified value. */
                Cfg.ulAdminStatus = ulAdminStatus;
                nRet = devCtl_xtmSetConnCfg( &Addr, &Cfg );
            }
        }
        else
        {
            nRet = CMSRET_INVALID_ARGUMENTS;
            fprintf(stderr,"%s: invalid parameter for option operate conn %s\n",
                g_szPgmName, pOpt->pszOptName );
        }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option operate "
            "conn %s\n", g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* ConnHandlerState */


/***************************************************************************
 * Function Name: ConnHandlerShow
 * Description  : Processes the xtmctl operate conn --show command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ConnHandlerShow( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;
    PXTM_ADDR pAddrs = NULL;
    PXTM_ADDR pAddrsMem = NULL;
    UINT32 ulNumAddrs = 0;
    UINT32 i, j;
    PXTM_TRANSMIT_QUEUE_PARMS pTxQParms;

    nRet = GetConnAddrsToUse( pOpt, &pAddrsMem, &ulNumAddrs );

    if( nRet == CMSRET_SUCCESS )
    {
        XTM_CONN_CFG Cfg;
        char *pszAalTypes[] = {"","","aal0pkt","aal0cell","","","","aal5"};
        char *pszPtmPriorities[] = {"", "low", "high"};
        char *pszHeaderTypes[] = {"none","llcsnap_eth", "llcsnap_rtip",
            "llcencaps_ppp", "vcmux_eth", "vcmux_ipoa", "vcmux_pppoa"};
        char *pszAdminStatuses[] = {"", "enabled", "disabled"};
        char *pszSchAlg[] = {"", "wrr", "wrr", "wfq"};
        char *pszTrafficType = "";
        char *pszType = "";
        char *pszHeaderType = "";
        char *pszAdminStatus = "";
        char *pszAlg = "";
        char *p;
        char szAddr[12] = "";
        char szPorts[12] = "";
        UINT32 ulPortMask;

        printf("mode portids addr  type     tdte header        status   mpPri mpAlg mpWt "
               "txQ qId qPid qSiz qPri qAlg qWt dAlg loMinThr loMaxThr hiMinThr hiMaxThr mbr_kbps pbr_kbps mbs_byte\n");
        for( i = 0, pAddrs = pAddrsMem; i < ulNumAddrs; i++, pAddrs++ )
        {
            /* Display configuration. */
            memset( &Cfg, 0x00, sizeof(Cfg) );

            nRet = devCtl_xtmGetConnCfg( pAddrs, &Cfg );
            if( nRet == CMSRET_SUCCESS )
            {
                if( pAddrs->ulTrafficType == TRAFFIC_TYPE_ATM )
                {
                    pszTrafficType = "ATM";
                    ulPortMask = pAddrs->u.Vcc.ulPortMask;

                    sprintf(szAddr, "%u/%u", pAddrs->u.Vcc.usVpi,
                        pAddrs->u.Vcc.usVci);
                    if( Cfg.ulAtmAalType <= AAL_5 )
                        pszType = pszAalTypes[Cfg.ulAtmAalType];

                    if( HT_TYPE(Cfg.ulHeaderType) <= HT_VC_MUX_PPPOA )
                        pszHeaderType=pszHeaderTypes[HT_TYPE(Cfg.ulHeaderType)];
                }
                else
                {
                    pszTrafficType = "PTM";
                    ulPortMask = pAddrs->u.Flow.ulPortMask;

                    if( pAddrs->u.Flow.ulPtmPriority <=
                        (PTM_PRI_LOW | PTM_PRI_HIGH) )
                    {
                        pszType =
                          pszPtmPriorities[Cfg.TransmitQParms[0].ulPtmPriority];
                    }
                    else
                        pszType = "";

                    szAddr[0] = '\0';
                    pszHeaderType = "";
                }

                for( j = 0, p = szPorts; j < MAX_PHY_PORTS; j++ )
                {
                    if( (PORT_TO_PORTID(j) & ulPortMask) == PORT_TO_PORTID(j) )
                    {
                        if( p != szPorts )
                            *p++ = ',';
                        *p++ = (char) PORT_TO_PORTID(j) + '0';
                    }
                }
                *p = '\0';

                if( Cfg.ulAdminStatus <= ADMSTS_DOWN )
                    pszAdminStatus = pszAdminStatuses[Cfg.ulAdminStatus];
                else
                    pszAdminStatus = "";

                if( Cfg.ConnArbs[0][0].ulWeightAlg <= WA_WFQ )
                    pszAlg = pszSchAlg[Cfg.ConnArbs[0][0].ulWeightAlg];
                else
                    pszAlg = "";
#if defined(CHIP_63268) || defined(CHIP_63138) || defined(CHIP_63381) || defined(CHIP_63148) || defined(CHIP_63158) || defined(CHIP_63178)
                printf("%-5s%-8s%-6s%-9s%-5d%-14s%-9s%-6d%-6s%-5d",
                    pszTrafficType, szPorts, szAddr, pszType,
                    Cfg.ulTransmitTrafficDescrIndex, pszHeaderType, pszAdminStatus,
                    Cfg.ConnArbs[0][0].ulSubPriority,
                    pszAlg,
                    Cfg.ConnArbs[0][0].ulWeightValue);
#else
                printf("%-5s%-8s%-6s%-9s%-5d%-14s%-9s%-6s%-6s%-5d",
                    pszTrafficType, szPorts, szAddr, pszType,
                    Cfg.ulTransmitTrafficDescrIndex, pszHeaderType, pszAdminStatus,
                    "",
                    pszAlg,
                    Cfg.ConnArbs[0][0].ulWeightValue);
#endif
                if( Cfg.ulTransmitQParmsSize )
                {
                    if( Cfg.TransmitQParms[0].ucWeightAlg <= WA_WFQ )
                        pszAlg = pszSchAlg[Cfg.TransmitQParms[0].ucWeightAlg];
                    else
                        pszAlg = "";
                     
                    printf("%-4d%-4d%-5d%-5d%-5d%-5s%-4d",
                        Cfg.TransmitQParms[0].ulTxQueueIdx,
                        Cfg.TransmitQParms[0].ucQosQId,
                        Cfg.TransmitQParms[0].ulPortId,
                        Cfg.TransmitQParms[0].usSize,
                        Cfg.TransmitQParms[0].ucSubPriority,
                        pszAlg,
                        Cfg.TransmitQParms[0].ulWeightValue);

                    if( Cfg.TransmitQParms[0].ucDropAlg == WA_RED )
                        printf("%-5s%-9d%-9d%-9s%-9s", "red",
                            Cfg.TransmitQParms[0].ucLoMinThresh,
                            Cfg.TransmitQParms[0].ucLoMaxThresh,
                            "",
                            "");
                    else if( Cfg.TransmitQParms[0].ucDropAlg == WA_WRED )
                        printf("%-5s%-9d%-9d%-9d%-9d", "wred",
                            Cfg.TransmitQParms[0].ucLoMinThresh,
                            Cfg.TransmitQParms[0].ucLoMaxThresh,
                            Cfg.TransmitQParms[0].ucHiMinThresh,
                            Cfg.TransmitQParms[0].ucHiMaxThresh);
                    else
                        printf("%-5s%-9s%-9s%-9s%-9s", "dt", "", "", "", "");

                    if( pAddrs->ulTrafficType != TRAFFIC_TYPE_ATM )
                        printf("%-9d%-9d%-9d",
                            Cfg.TransmitQParms[0].ulMinBitRate,
                            Cfg.TransmitQParms[0].ulShapingRate,
                            Cfg.TransmitQParms[0].usShapingBurstSize);
                }
                printf("\n");
                if( pAddrs->ulTrafficType == TRAFFIC_TYPE_ATM )
                {
                    for( j = 1, pTxQParms = Cfg.TransmitQParms + j;
                        j < Cfg.ulTransmitQParmsSize; j++, pTxQParms++ )
                    {
                        if( pTxQParms->ucWeightAlg <= WA_WFQ )
                            pszAlg = pszSchAlg[pTxQParms->ucWeightAlg];
                        else
                            pszAlg = "";
                     
                        printf("                                                                         "
                               "%-4d%-4d%-5d%-5d%-5d%-5s%-4d",
                            pTxQParms->ulTxQueueIdx,
                            pTxQParms->ucQosQId,
                            pTxQParms->ulPortId,
                            pTxQParms->usSize,
                            pTxQParms->ucSubPriority,
                            pszAlg,
                            pTxQParms->ulWeightValue);

                        if( pTxQParms->ucDropAlg == WA_RED )
                            printf("%-5s%-9d%-9d%-9s%-9s\n", "red",
                                pTxQParms->ucLoMinThresh,
                                pTxQParms->ucLoMaxThresh,
                                "",
                                "");
                        else if( pTxQParms->ucDropAlg == WA_WRED )
                            printf("%-5s%-9d%-9d%-9d%-9d\n", "wred",
                                pTxQParms->ucLoMinThresh,
                                pTxQParms->ucLoMaxThresh,
                                pTxQParms->ucHiMinThresh,
                                pTxQParms->ucHiMaxThresh);
                        else
                            printf("%-5s%-9s%-9s%-9s%-9s\n", "dt", "", "", "", "");
                    }
                }
                else
                {
                    for( j = 1, pTxQParms = Cfg.TransmitQParms + j;
                        j < Cfg.ulTransmitQParmsSize; j++, pTxQParms++ )
                    {
                        if( pTxQParms->ucWeightAlg <= WA_WFQ )
                            pszAlg = pszSchAlg[pTxQParms->ucWeightAlg];
                        else
                            pszAlg = "";
                     
                        printf("                   %-9s                                             "
                               "%-4d%-4d%-5d%-5d%-5d%-5s%-4d",
                            pszPtmPriorities[pTxQParms->ulPtmPriority],
                            pTxQParms->ulTxQueueIdx,
                            pTxQParms->ucQosQId,
                            pTxQParms->ulPortId,
                            pTxQParms->usSize,
                            pTxQParms->ucSubPriority,
                            pszAlg,
                            pTxQParms->ulWeightValue);

                        if( pTxQParms->ucDropAlg == WA_RED )
                            printf("%-5s%-9d%-9d%-9s%-9s", "red",
                                pTxQParms->ucLoMinThresh,
                                pTxQParms->ucLoMaxThresh,
                                "",
                                "");
                        else if( pTxQParms->ucDropAlg == WA_WRED )
                            printf("%-5s%-9d%-9d%-9d%-9d", "wred",
                                pTxQParms->ucLoMinThresh,
                                pTxQParms->ucLoMaxThresh,
                                pTxQParms->ucHiMinThresh,
                                pTxQParms->ucHiMaxThresh);
                        else
                            printf("%-5s%-9s%-9s%-9s%-9s", "dt", "", "", "", "");

                        printf("%-9d%-9d%-9d\n",
                            pTxQParms->ulMinBitRate,
                            pTxQParms->ulShapingRate,
                            pTxQParms->usShapingBurstSize);
                    }
                }
            }
        }
    }

    if( pAddrsMem )
        free( pAddrsMem );

    return( nRet );
} /* ConnHandlerShow */


/***************************************************************************
 * Function Name: ConnHandlerSendOam
 * Description  : Processes the xtmctl operate conn --sendoam command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ConnHandlerSendOam( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;

    if( pOpt->nNumParms == 2 )
    {
        XTM_ADDR Addr;

        nRet = GetConnAddr( pOpt->pszParms[0], &Addr );
        if( nRet == CMSRET_SUCCESS )
        {
            PATM_ADDR pVccAddr = &Addr.u.Vcc;

            /* Only one port can be specified. */
            if( Addr.ulTrafficType == TRAFFIC_TYPE_ATM &&
                (pVccAddr->ulPortMask == PORT_PHY0_FAST ||
                 pVccAddr->ulPortMask == PORT_PHY0_INTERLEAVED ||
                 pVccAddr->ulPortMask == PORT_PHY1_FAST ||
                 pVccAddr->ulPortMask == PORT_PHY1_INTERLEAVED) )
            {
                const UINT16 usOamF4Segment  = 0x03;
                const UINT16 usOamF4EndToEnd = 0x04;

                UINT8 ucCt;

                if( !strcmp( pOpt->pszParms[1], "f5seg" ) )
                {
                    ucCt = CTYPE_OAM_F5_SEGMENT;
                }
                else if( !strcmp( pOpt->pszParms[1], "f5end" ) )
                {
                    ucCt = CTYPE_OAM_F5_END_TO_END;
                }
                else if( !strcmp( pOpt->pszParms[1], "f4seg" ) )
                {
                    ucCt = CTYPE_OAM_F4_SEGMENT;
                    pVccAddr->usVci = usOamF4Segment;
                }
                else if( !strcmp( pOpt->pszParms[1], "f4end" ) )
                {
                    ucCt = CTYPE_OAM_F4_END_TO_END;
                    pVccAddr->usVci = usOamF4EndToEnd;
                }
                else
                {
                    nRet = CMSRET_INVALID_ARGUMENTS;
                    fprintf( stderr, "%s: invalid parameter 2 for option"
                        "operate conn %s\n", g_szPgmName, pOpt->pszOptName );
                }

                if( nRet == CMSRET_SUCCESS )
                {
                    XTM_OAM_CELL_INFO OamCellInfo;

                    memset(&OamCellInfo, 0x00, sizeof(OamCellInfo));
                    OamCellInfo.ucCircuitType = ucCt;
                    OamCellInfo.ulTimeout = 5000;
                    OamCellInfo.ulRepetition = 1;
                    nRet = devCtl_xtmSendOamCell( &Addr, &OamCellInfo );
                    switch( (BCMXTM_STATUS)nRet )
                    {
                    case XTMSTS_SUCCESS:
                        printf("%s: OAM loopback response received on "
                            "PORT/VPI/VCI %u/%u/%u\n", g_szPgmName,
                            pVccAddr->ulPortMask, pVccAddr->usVpi,
                            pVccAddr->usVci);
                        break;

                    //KU_TBD: this is a bit odd -- XTMSTS_TIMEOUT does not matche
                    // up wtih any CMSRET_ values.  I think the compiler would 
                    // complain if devCtl_xtmSendOamCell tried to return it
                    case XTMSTS_TIMEOUT:
                        printf("%s: OAM loopback response not received on "
                            "PORT/VPI/VCI %u/%u/%u\n", g_szPgmName,
                            pVccAddr->ulPortMask, pVccAddr->usVpi,
                            pVccAddr->usVci);
                        break;

                    default:
                        break;
                    }
                }
            }
            else
            {
                nRet = CMSRET_INVALID_ARGUMENTS;
                fprintf( stderr, "%s: Invalid connection address.\n",
                    g_szPgmName );
            }
        }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option operate "
            "conn %s\n", g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* ConnHandlerSendOam */


/***************************************************************************
 * Function Name: ConnHandlerCreateDevice
 * Description  : Processes the xtmctl operate conn --createdevice command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ConnHandlerCreateDevice( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;

    if( pOpt->nNumParms == 2 )
    {
        XTM_ADDR Addr;

        nRet = GetConnAddr( pOpt->pszParms[0], &Addr );
        if( nRet == CMSRET_SUCCESS )
        {
            nRet = devCtl_xtmCreateNetworkDevice( &Addr, pOpt->pszParms[1] );
        }
        else
        {
            nRet = CMSRET_INVALID_ARGUMENTS;
            fprintf( stderr, "%s: Invalid connection address.\n",
                g_szPgmName );
        }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option operate "
            "conn %s\n", g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* ConnHandlerCreateDevice */


/***************************************************************************
 * Function Name: ConnHandlerDeleteDevice
 * Description  : Processes the xtmctl operate conn --deletedevice command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet ConnHandlerDeleteDevice( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;

    if( pOpt->nNumParms == 1 )
    {
        XTM_ADDR Addr;

        nRet = GetConnAddr( pOpt->pszParms[0], &Addr );
        if( nRet == CMSRET_SUCCESS )
        {
            nRet = devCtl_xtmDeleteNetworkDevice( &Addr );
        }
        else
        {
            nRet = CMSRET_INVALID_ARGUMENTS;
            fprintf( stderr, "%s: Invalid connection address.\n",
                g_szPgmName );
        }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option operate "
            "conn %s\n", g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* ConnHandlerDeleteDevice */


/***************************************************************************
 * Function Name: GetConnAddrsToUse
 * Description  : Returns an array XTM_ADDR structures that the caller should
 *                use.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet GetConnAddrsToUse( POPTION_INFO pOpt, PXTM_ADDR *ppAddrs,
    UINT32 *pulNumAddrs )
{
    CmsRet nRet = CMSRET_SUCCESS;
    PXTM_ADDR pAddrs = NULL;
    UINT32 ulNumAddrs = 0;

    *ppAddrs = NULL;
    *pulNumAddrs = 0;

    switch( pOpt->nNumParms )
    {
    case 1:
        /* A single connection address was passed on the command line. */
        ulNumAddrs = 1;
        pAddrs = (PXTM_ADDR) malloc(ulNumAddrs * (sizeof(XTM_ADDR)));
        if( pAddrs )
            nRet = GetConnAddr( pOpt->pszParms[0], pAddrs );
        else
        {
            nRet = CMSRET_RESOURCE_EXCEEDED;
            fprintf( stderr, "%s: memory allocation error\n", g_szPgmName );
        }
        break;

    case 0:
        /* No connection address was passed on the commnd line.  Use all
         * configured connections.
         */
        devCtl_xtmGetConnAddrs( NULL, &ulNumAddrs );

        if( ulNumAddrs )
        {
            pAddrs = (PXTM_ADDR) malloc(ulNumAddrs * sizeof(XTM_ADDR)); 

            /* Get the addresses of all configured connections. */
            if( pAddrs )
                devCtl_xtmGetConnAddrs( pAddrs, &ulNumAddrs );
            else
            {
                nRet = CMSRET_RESOURCE_EXCEEDED;
                fprintf(stderr, "%s: memory allocation error\n", g_szPgmName);
            }
        }
        break;

    default:
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option operate "
            "conn %s\n", g_szPgmName, pOpt->pszOptName );
        break;
    }

    if( nRet == CMSRET_SUCCESS )
    {
        *ppAddrs = pAddrs;
        *pulNumAddrs = ulNumAddrs;
    }

    return( nRet );
} /* GetConnAddrsToUse */


/***************************************************************************
 * Function Name: FindConnCfg
 * Description  : Find the connection connection configuration record for a
 *                connection address that has the specified VPI/VCI or PTM
 *                flow and the specified port id is part of the connection's
 *                port mask.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet FindConnCfg( PXTM_ADDR pConnAddr, PXTM_ADDR pRetAddr,
    PXTM_CONN_CFG pCfg )
{
    int nRet = CMSRET_OBJECT_NOT_FOUND;
    UINT32 ulNumAddrs = 0;
    PXTM_ADDR pAddrs = NULL;

    devCtl_xtmGetConnAddrs( NULL, &ulNumAddrs );

    if( (pAddrs = (PXTM_ADDR) malloc(ulNumAddrs * sizeof(XTM_ADDR))) != NULL )
    {
        UINT32 i, ulPortMask = 0, ulPtmPriMask = 0;
        PXTM_ADDR pAddr;

        devCtl_xtmGetConnAddrs( pAddrs, &ulNumAddrs );
        for( i = 0, pAddr = pAddrs; i < ulNumAddrs; i++, pAddr++ )
        {
            /* Mask the port bits. */
            if( pAddr->ulTrafficType == TRAFFIC_TYPE_ATM )
            {
                ulPortMask = pAddr->u.Vcc.ulPortMask;
                pAddr->u.Vcc.ulPortMask &= pConnAddr->u.Vcc.ulPortMask;
            }
            else
            {
                ulPortMask = pAddr->u.Flow.ulPortMask;
                pAddr->u.Flow.ulPortMask &= pConnAddr->u.Flow.ulPortMask;
                ulPtmPriMask = pAddr->u.Flow.ulPtmPriority;
                pAddr->u.Flow.ulPtmPriority &= pConnAddr->u.Flow.ulPtmPriority;
            }

            /* If the connection address matches after masking the port bits,
             * then the connection was found.  Get its configuration.
             */
            if( !memcmp(pAddr, pConnAddr, sizeof(XTM_ADDR)) )
            {
                /* Restore the port mask. */
                if( pAddr->ulTrafficType == TRAFFIC_TYPE_ATM )
                    pAddr->u.Vcc.ulPortMask = ulPortMask;
                else
                {
                    pAddr->u.Flow.ulPortMask = ulPortMask;
                    pAddr->u.Flow.ulPtmPriority = ulPtmPriMask;
                }

                memcpy( pRetAddr, pAddr, sizeof(XTM_ADDR));
                nRet = devCtl_xtmGetConnCfg(pAddr, pCfg);
                break;
            }
        }

        if( nRet == CMSRET_OBJECT_NOT_FOUND )
        {
            fprintf( stderr, "%s: connection configuration not found\n",
                g_szPgmName );
        }

        free(pAddrs);
    }
    else
    {
        nRet = CMSRET_RESOURCE_EXCEEDED;
        fprintf( stderr, "%s: memory allocation error\n", g_szPgmName );
    }

    return( nRet );
} /* FindConnCfg */


/***************************************************************************
 * Function Name: GetConnAddr
 * Description  : Converts a connection address string in the form of
 *                port_mask.vpi.vci or port_mask.priptm_mask to the XTM_ADDR
 *                structure.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet GetConnAddr( char *pszConnAddr, PXTM_ADDR pAddr )
{
    int nRet = CMSRET_INVALID_ARGUMENTS;
    char *pszPortMask = NULL;
    char *pszVpi = NULL;
    char *pszPtmPriMask = NULL;
    char *pszVci = NULL;

    pszPortMask = pszConnAddr;
    while( *++pszConnAddr )
    {
        if( *pszConnAddr == '.' )
        {
            *pszConnAddr = '\0';
            pszVpi = pszConnAddr + 1;
            pszPtmPriMask = pszConnAddr + 1;
            while( *++pszConnAddr )
            {
                if( *pszConnAddr == '.' && *(pszConnAddr + 1) != '\0' )
                {
                    *pszConnAddr = '\0';
                    pszVci = pszConnAddr + 1;
                    break;
                }
            }
            break;
        }
    }

    if( pszConnAddr && pszPortMask && pszVpi && pszVci )
    {
        /* ATM VCC address. */
        PATM_ADDR pVccAddr = &pAddr->u.Vcc;
        char *pszEnd = NULL;
        UINT32 ulPortMask = (UINT8) strtol( pszPortMask, &pszEnd, 0 );

        pAddr->ulTrafficType = TRAFFIC_TYPE_ATM;
        if( (ulPortMask == PORT_PHY0_FAST || ulPortMask == PORT_PHY0_INTERLEAVED) &&
             *pszEnd == '\0' )
        {
            pVccAddr->ulPortMask = ulPortMask;
            pszEnd = NULL;
            pVccAddr->usVpi = (UINT16) strtol( pszVpi, &pszEnd, 10 );
            if( pVccAddr->usVpi < 0xff && *pszEnd == '\0' )
            {
                pszEnd = NULL;
                pVccAddr->usVci = (UINT16) strtol( pszVci, &pszEnd, 10 );
                if( *pszEnd == '\0' )
                    nRet = 0;
            }
        }
    }
    else
        if( pszConnAddr && pszPortMask && pszPtmPriMask && pszVci == NULL )
        {
            /* PTM address. */
            PPTM_ADDR pPtmAddr = &pAddr->u.Flow;
            char *pszEnd = NULL;
            UINT32 ulPortMask = (UINT8) strtol( pszPortMask, &pszEnd, 0 );

            pAddr->ulTrafficType = TRAFFIC_TYPE_PTM;
            if( (ulPortMask == PORT_PHY0_FAST || ulPortMask == PORT_PHY0_INTERLEAVED) &&
                *pszEnd == '\0' )
            {
                pPtmAddr->ulPortMask = ulPortMask;
                pszEnd = NULL;
                pPtmAddr->ulPtmPriority = (UINT16) strtol( pszPtmPriMask, &pszEnd, 10 );
                if( *pszEnd=='\0' && (pPtmAddr->ulPtmPriority >= PTM_PRI_LOW ||
                    pPtmAddr->ulPtmPriority <= (PTM_PRI_LOW | PTM_PRI_HIGH)) )
                {
                    nRet = 0;
                }
            }
        }

    if( nRet != 0 )
    {
        if( pszVpi )
            *(pszVpi - 1) = '.';
        if( pszVci )
            *(pszVci - 1) = '.';
        fprintf(stderr, "%s: invalid ATM/PTM connection address '%s'\n",
            g_szPgmName, pszPortMask);
    }

    return( nRet );
} /* GetConnAddr */


/***************************************************************************
 * Function Name: BondingHandler
 * Description  : Processes the xtmctl bonding command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet BondingHandler( POPTION_INFO pOptions, int nNumOptions )
{
    CmsRet nRet = CMSRET_SUCCESS;


    /* With no options, just invoke default (--status) behavior */
    if(nNumOptions == 0)
    {
        nRet = BondingHandlerStatus( (POPTION_INFO) NULL );
    }

    /* Loop through options */
    while( nRet == CMSRET_SUCCESS && nNumOptions )
    {
        if( !strcmp( pOptions->pszOptName, "--status" ) )
        {
            nRet = BondingHandlerStatus( pOptions );
        }

        nNumOptions--;
        pOptions++;
    }

    return( nRet );
} /* BondingHandler */


/***************************************************************************
 * Function Name: DbgHandler
 * Description  : Processes the xtmctl dbg command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet DbgHandler( POPTION_INFO pOptions, int nNumOptions )
{
    CmsRet nRet = CMSRET_SUCCESS;


    /* With no options, just invoke default (--status) behavior */
    if(nNumOptions == 0)
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
    }

    /* Loop through options */
    while( nRet == CMSRET_SUCCESS && nNumOptions )
    {
        if( !strcmp( pOptions->pszOptName, "--SILI" ) )
        {
            nRet = DbgSetInterfaceLinkInfo( pOptions );
        }

        if( !strcmp(pOptions->pszOptName, "--SARLB"))
        {
           nRet = DbgSARLoopBackConfig( pOptions );
        }

        nNumOptions--;
        pOptions++;
    }

    return( nRet );
} /* DbgHandler */

static CmsRet DbgSetInterfaceLinkInfo( POPTION_INFO pOpt)
{
   CmsRet nRet = CMSRET_SUCCESS;

   if((pOpt->nNumParms < 1) || (pOpt->nNumParms > 2))
   {
      nRet = CMSRET_INVALID_ARGUMENTS;
   }

   if(nRet == CMSRET_SUCCESS)
   {
      char *pszEnd = NULL;
      unsigned long event_type;
      unsigned long traffic_type;
      XTM_INTERFACE_LINK_INFO linkInfo;

      /* Read in traffictype */
      event_type = (unsigned long) strtol( pOpt->pszParms[0], &pszEnd, 10 );

      /* Valid type? */
      if( *pszEnd != '\0' )
      {
          /* Nope - flag error */
          nRet = CMSRET_INVALID_ARGUMENTS;
          fprintf( stderr, "%s: invalid parameter for option %s\n",
              g_szPgmName, pOpt->pszOptName );
      }
      else if(event_type == 1)
      {
         linkInfo.ulLinkState = LINK_UP;
         traffic_type = (unsigned long) strtol( pOpt->pszParms[1], &pszEnd, 10 );
         if(traffic_type == 0)
         {
            linkInfo.ulLinkUsRate = 6000000;
            linkInfo.ulLinkDsRate = 20000000;
            linkInfo.ulLinkTrafficType = TRAFFIC_TYPE_ATM;
         }
         else
         {
            linkInfo.ulLinkUsRate = 60000000;
            linkInfo.ulLinkDsRate = 100000000;
            linkInfo.ulLinkTrafficType = TRAFFIC_TYPE_PTM;
         }
         nRet = devCtl_xtmSetInterfaceLinkInfo(1, &linkInfo);
      }
      else
      {
         linkInfo.ulLinkState = LINK_DOWN;
         linkInfo.ulLinkUsRate = 0;
         linkInfo.ulLinkDsRate = 0;
         linkInfo.ulLinkTrafficType = TRAFFIC_TYPE_NOT_CONNECTED; 
         nRet = devCtl_xtmSetInterfaceLinkInfo(1, &linkInfo);
      }
   }
   return (nRet);
}

static CmsRet DbgSARLoopBackConfig( POPTION_INFO pOpt)
{
   CmsRet nRet = CMSRET_SUCCESS;

   if((pOpt->nNumParms != 1) )
   {
      nRet = CMSRET_INVALID_ARGUMENTS;
   }

   if(nRet == CMSRET_SUCCESS)
   {
      char *pszEnd = NULL;
      unsigned long enable;
      /* Read in enable */
      enable = (unsigned long) strtol( pOpt->pszParms[0], &pszEnd, 10 );

      /* Valid type? */
      if( *pszEnd != '\0' )
      {
          /* Nope - flag error */
          nRet = CMSRET_INVALID_ARGUMENTS;
          fprintf( stderr, "%s: invalid parameter for option %s\n",
              g_szPgmName, pOpt->pszOptName );
      }
      else if(enable == 1)
      {
         //nRet = devCtl_xtmSetInterfaceLinkInfo(1, &linkInfo);
      }
      else
      {
         //nRet = devCtl_xtmSetInterfaceLinkInfo(1, &linkInfo);
      }
   }
   return (nRet);
}
/***************************************************************************
 * Function Name: BondingHandlerStatus
 * Description  : Processes the xtmctl bonding --status command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet BondingHandlerStatus( POPTION_INFO pOpt )
{
    CmsRet nRet = CMSRET_SUCCESS;

    if( (pOpt == (POPTION_INFO)NULL) || (pOpt->nNumParms == 0 ) )
    {
       XTM_BOND_INFO bondInfo ;
       char trafficTypeStr [25], bondProtoStr [50] ;
       memset( &bondInfo, 0x00, sizeof(XTM_BOND_INFO) );

       /* Read the current bonding status from inside the bonding
        * processes.
        * These parameters can also be used for TR-159 element retrieval
        * purposes.
        */
       nRet = devCtl_xtmGetBondingInfo ( &bondInfo );
       if( nRet == CMSRET_SUCCESS )
       {
          int i ;

          printf ("\nXTM Bonding Version   = %d.%d.%d \n", bondInfo.u8MajorVersion,
                bondInfo.u8MinorVersion, bondInfo.u8BuildVersion) ;
          GetTrafficTypeStr (bondInfo.ulTrafficType, &trafficTypeStr [0]) ;
          printf ("Traffic Type          = %s \n", trafficTypeStr) ;
          GetBondingProtocolStr (bondInfo.ulBondProto, &bondProtoStr [0]) ;
          printf ("Bonding Protocol      = %s \n", bondProtoStr) ;
          printf ("TxPAF Status          = %s \n", (bondInfo.ulTxPafEnabled==1) ? "Enabled" : "Disabled");
          printf ("Num of Bonding Groups = %d \n", bondInfo.ulNumGroups) ;
          for (i=0; i<bondInfo.ulNumGroups; i++) {
             int j ;
             printf ("Group %d information  :\n", i) ;
             printf ("******************************\n") ;
             printf ("Group Id              = %d \n", bondInfo.grpInfo[i].ulGroupId) ;
             printf ("Num of Ports          = %d \n\n", MAX_BOND_PORTS) ;
             for (j=0; j<MAX_BOND_PORTS; j++) {
                printf ("Port Id               = %d \n", bondInfo.grpInfo[i].portInfo[j].ulInterfaceId) ;
                printf ("Link State            = %s \n",
                      (bondInfo.grpInfo[i].portInfo[j].linkState == LINK_UP) ? "UP" : "DOWN") ;
                printf ("US Rate               = %d bps \n", bondInfo.grpInfo[i].portInfo[j].usRate) ;
                printf ("DS Rate               = %d bps \n", bondInfo.grpInfo[i].portInfo[j].dsRate) ;
                printf ("US Delay              = %d milli sec \n", bondInfo.grpInfo[i].portInfo[j].usDelay) ;
                printf ("DS Bonding Delay      = %d milli sec \n", bondInfo.grpInfo[i].portInfo[j].dsBondingDelay) ;
                if(bondInfo.ulBondProto == BC_BOND_PROTO_ASM)
                {
                   printf ("Received ASM Group Id = %d \n", bondInfo.grpInfo[i].portInfo[j].rcvdGrpId) ;
                   printf ("Local Tx State        = %d \n", bondInfo.grpInfo[i].portInfo[j].localTxLineSt) ;
                   printf ("Local Rx State        = %d \n", bondInfo.grpInfo[i].portInfo[j].localRxLineSt) ;
                }
                printf ("\n") ;
             } /* for (ports) */
             printf ("Aggr US Rate          = %d bps \n", bondInfo.grpInfo[i].aggrUSRate) ;
             printf ("Aggr DS Rate          = %d bps \n", bondInfo.grpInfo[i].aggrDSRate) ;
             printf ("Diff US Delay         = %d milli sec \n", bondInfo.grpInfo[i].diffUSDelay) ;
             printf ("Data Status           = %s \n", (bondInfo.grpInfo[i].dataStatus == DATA_STATUS_ENABLED)
                                                ? "Enabled" : "Disabled") ;
             printf ("******************************\n") ;
          } /* for (groups) */
       } /* nRet = Success) */
       else if (nRet == CMSRET_METHOD_NOT_SUPPORTED) {
          printf ("XTM Version   = %d.%d.%d \n", bondInfo.u8MajorVersion,
                bondInfo.u8MinorVersion, bondInfo.u8BuildVersion) ;
          GetTrafficTypeStr (bondInfo.ulTrafficType, &trafficTypeStr [0]) ;
          printf ("Traffic Type          = %s \n", trafficTypeStr) ;
          GetBondingProtocolStr (bondInfo.ulBondProto, &bondProtoStr [0]) ;
          printf ("Bonding Protocol      = %s \n", bondProtoStr) ;
          printf ("TxPAF Status          = %s \n", (bondInfo.ulTxPafEnabled==1) ? "Enabled" : "Disabled");
          printf ("Num of Bonding Groups = %d \n", bondInfo.ulNumGroups) ;
	  if((bondInfo.ulTrafficType == TRAFFIC_TYPE_ATM) || (bondInfo.ulTrafficType == TRAFFIC_TYPE_PTM)) {
             printf ("******************************\n") ;
             printf ("Port Id               = %d \n", bondInfo.grpInfo[0].portInfo[0].ulInterfaceId) ;
             printf ("Link State            = %s \n",
                      (bondInfo.grpInfo[0].portInfo[0].linkState == LINK_UP) ? "UP" : "DOWN") ;
             printf ("US Rate               = %d bps \n", bondInfo.grpInfo[0].portInfo[0].usRate) ;
             printf ("DS Rate               = %d bps \n", bondInfo.grpInfo[0].portInfo[0].dsRate) ;
             printf ("******************************\n") ;
	  }
       }
       else {
          printf ("XTM layer is not initialized. Can't retrieve any information. \n") ;
       }
    }
    else
    {
        nRet = CMSRET_INVALID_ARGUMENTS;
        fprintf( stderr, "%s: invalid number of parameters for option bonding "
            "%s\n", g_szPgmName, pOpt->pszOptName );
    }

    return( nRet );
} /* BondingHandlerState */



/***************************************************************************
 * Function Name: RegisterDataDump
 * Description  : Convenience function to dump single region of register memory.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet RegisterDataDump(unsigned long ulAddr, int nLen)
{

    CmsRet nRet = CMSRET_SUCCESS;           /* Return value */
    int _fdmem;                             /* File descriptor for memory map */
    const char memDevice[] = "/dev/mem";    /* Mem map device */
    int i;                                  /* Ubiquitous counter variable */

    unsigned int uiOffset;                  /* Offset of dump region within mapped page */
    unsigned long ulPagePhysAddr;           /* Address of page containing ulAddr */
    unsigned int uiMapRequestSize;          /* Size of mapped region to request */

    unsigned int uiNumRegsToDump = 
        (nLen / (sizeof(unsigned long)));   /* Number of registers to dump - derived from nLen */

    unsigned long *pulBaseMap = NULL;       /* Pointer for start of mapped region */
    unsigned long *pulDumpPtr = NULL;       /* Pointer within mapped region to dump */


    /* For MIPS, convert virtual address to physical address for use by /dev/mem */
#if defined (__mips__)
    ulAddr = VIRT_TO_PHY(ulAddr);
#endif

    /* Open /dev/mem and error check the results */
    if ((_fdmem = open( memDevice, O_RDWR | O_SYNC )) < 0){
        fprintf(stderr, "Failed to open the /dev/mem !\n");
    return -1;
    }

    /* Normalize physical address to dump for unsigned longs */
    if(ulAddr % (sizeof(unsigned long)) != 0) {
        /* Round back down to lowest ulong aligned address */
        ulAddr = ulAddr - (ulAddr % (sizeof(unsigned long)));
    }

    /* Normalize length of region to dump for unsigned longs */
    if(nLen % (sizeof(unsigned long)) != 0) {
        uiNumRegsToDump++;
    }

    /* The mmap() call requires mapping regions on page boundaries.  Calculate
       address and sizes of the requested piece of memory within the mapped page. */
    uiOffset = ulAddr % getpagesize();  /* Find the offset of the dump region within a physical page */
    ulPagePhysAddr = ulAddr - uiOffset; /* Find page starting address */
    uiMapRequestSize = (uiNumRegsToDump * sizeof(unsigned long)) + uiOffset; /* Size of region to map */  

    /* mmap() the opened /dev/mem */
    pulBaseMap = (unsigned long *)(mmap(0,uiMapRequestSize,PROT_READ,MAP_PRIVATE,_fdmem,ulPagePhysAddr));

    /* Did it work? */
    if(pulBaseMap == MAP_FAILED) {
        perror("Failed mmap() call");
        return(-1);
    }

    /* Figure out where in the mapped area we should dump.  Be sure to
       accomodate the fact that we're using an unsigned long pointer,
       not a byte pointer.  */
    pulDumpPtr = pulBaseMap + (uiOffset/sizeof(unsigned long));

    /* Use 'pulBaseMap' pointer to access the mapped area. */
    for (i=0;i<uiNumRegsToDump;i++)
    {
        if(i % 4 == 0)
            printf("\n%08lx:", ulAddr + (i * sizeof(unsigned long)));
        printf(" %08lx", pulDumpPtr[i]);
    }

    /* Print end the line */
    printf("\n");

    /* unmap the area & error checking */
    if (munmap(pulBaseMap, uiMapRequestSize)==-1){
        perror("Error un-mmapping the file");
        nRet = -1;
    }

    /* close the character device */
    close(_fdmem);

    return(nRet);
}

/***************************************************************************
 * Function Name: SarHandler
 * Description  : Processes the xtmctl sar command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet SarHandler( POPTION_INFO pOptions, int nNumOptions )
{
    CmsRet nRet = CMSRET_SUCCESS;   /* Return value */

    /* Array of SAR subcommands, their addresses, and the size of their dump regions */
    typedef struct { 
        char *pcCmd;
        unsigned long ulAddr;
        int nLen;
        char *pcComment;
    } SAR_CMD_DATA;


    SAR_CMD_DATA sSarCmdData[] = {
        {"tx",     SAR_TX_CTL_REGS,  SAR_TX_CTL_REGS_SZ,   "SAR Tx Control Registers"},
        {"rx",     SAR_RX_CTL_REGS,  SAR_RX_CTL_REGS_SZ,   "SAR Rx Control Registers"},
        {"rxcam",  SAR_RX_VCAM_REGS, SAR_RX_VCAM_REGS_SZ,  "SAR Rx ATM VPI_VCI CAM Table Reg Registers"},
        {"shaper", SAR_SHPR_REGS,    SAR_SHPR_REGS_SZ,     "SAR Atm Shaper Source Shaping Table Registers"},
        {"rxpbuf", SAR_RX_PBUF_REGS, SAR_RX_PBUF_REGS_SZ,  "SAR Rx Packet Buffer Control Registers"},
        {"mib",    SAR_MIB_REGS,     SAR_MIB_REGS_SZ,      "SAR Atm MIB Counters Registers"},
#ifdef SAR_RX_PAF_REGS
        {"rxpaf",  SAR_RX_PAF_REGS,  SAR_RX_PAF_REGS_SZ,   "SAR RxPaf Top Registers"},
#endif
#ifdef SAR_RX_BOND_REGS
        {"rxbond", SAR_RX_BOND_REGS, SAR_RX_BOND_REGS_SZ,  "SAR RxPaf Bonding Registers"},
#endif
#ifdef SAR_TMUEXT_REGS
        {"tmuext", SAR_TMUEXT_REGS,  SAR_TMUEXT_REGS_SZ,   "SAR Traffic Management Unit Extended Registers"},
#endif
        {NULL, 0, 0, NULL}
    };

    /* First, check to see if the "all" command is being invoked */
    if(nNumOptions >0 && strcasecmp(pOptions->pszOptName, "all") == 0)
    {
        SAR_CMD_DATA *psCmndPtr = sSarCmdData;  /* Pointer to check for valid commands */

        /* Yes.  User is invoking "all."  */ 
        printf("Dumping all SAR registers\n");
        printf("-------------------------\n");

        /* Loop and dump all commands. */
        while(psCmndPtr->pcCmd != NULL)
        {
            /* Yes.  Dump the data. */
            printf("%s", psCmndPtr->pcComment);  /* No newline - RegisterDataDump() will add one */

            /* Delay long enough for printf() to complete (200 mS). */
            usleep(200000);

            /* Dump the register region */
            RegisterDataDump(psCmndPtr->ulAddr, psCmndPtr->nLen);

            /* Go to the next command to dump */
            psCmndPtr++;
        }
        /* Add newline */
        printf("\n");
    }
    else
    {
        /* Find the command being used from the list of legal commands.  */
        while( nRet == CMSRET_SUCCESS && nNumOptions )
        {
            SAR_CMD_DATA *psCmndPtr = sSarCmdData;  /* Pointer to check for valid commands */

            /* Loop through valid commands, looking for a match */
            while(psCmndPtr->pcCmd != NULL)
            {
                /* Is it a match? */
                if(strcasecmp(psCmndPtr->pcCmd, pOptions->pszOptName) == 0)
                {
                    /* Yes.  Dump the data. */
                    printf("Data dump for %s:", psCmndPtr->pcComment);  /* No newline - RegisterDataDump() will add one */

                    /* Delay long enough for printf() to complete (200 mS). */
                    usleep(200000);

                    /* Dump the register region */
                    RegisterDataDump(psCmndPtr->ulAddr, psCmndPtr->nLen);

                    /* Add newline */
                    printf("\n");

                    /* Break out of loop to check commands */
                    break;
                }

                /* Go to the next command to check */
                psCmndPtr++;
            }

            /* Did we match a command? */
            if(psCmndPtr->pcCmd == NULL)
                fprintf(stderr, "%s: unrecognized SAR command '%s'.\n"
                            , g_szPgmName, pOptions->pszOptName);

            /* Go to next command/option */
            nNumOptions--;
            pOptions++;
        }
    }
    return( nRet );
} /* SarHandler */

#if defined(CHIP_63158) || defined(CHIP_63178)
/***************************************************************************
 * Function Name: TxDbgHandler
 * Description  : Processes the xtmctl txdbg command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet TxDbgHandler( POPTION_INFO pOptions, int nNumOptions )
{
    CmsRet nRet = CMSRET_SUCCESS;   /* Return value */
    char usr_input[10];
    /* Array of tx debug subcommands, their addresses, and the size of their dump regions */
    typedef struct { 
        char *pcCmd;
        unsigned long ulAddr;
        int nLen;
        char *pcComment;
    } TXDBG_CMD_DATA;
    TXDBG_CMD_DATA sTxDbgCmdData[] = {
        {"bbhfifo",     SAR_TXBBH_FIFO, SAR_TXBBH_FIFO_SZ, "SAR Tx BBH FiFo Data Registers"},
        {"pktfifo",     SAR_TXPKT_FIFO, SAR_TXPKT_FIFO_SZ, "SAR Tx PacketBuffer FiFo Data Registers"},
        {"utofifo",     SAR_TXUTO_FIFO, SAR_TXUTO_FIFO_SZ, "SAR Tx Utopia FiFo Data Registers"},
        {NULL, 0, 0, NULL}
    };

    memset(usr_input,0x0,sizeof(usr_input));
    printf("WARNING:Exectuting this debug command with any traffic going through XTM interface will\n" \
           "affect the future packet forwarding to completely halt and requires reboot to recover fully.\n");
    printf("Stop all traffic going through the XTM interface before issuing this debug command.\n");
    printf("Enter \"YES\" to continue with this command or anything else to abort this command.\n");
    printf("Do you want to Continue:");
    scanf("%9s",usr_input);
    if(strcmp(usr_input,"YES")!= 0)
    {
       printf("Debug command aborted\n");
       return CMSRET_SUCCESS;
    }

    /* First, check to see if the "all" command is being invoked */
    if(nNumOptions >0 && strcasecmp(pOptions->pszOptName, "all") == 0)
    {
        TXDBG_CMD_DATA *psCmndPtr = sTxDbgCmdData;  /* Pointer to check for valid commands */

        /* Yes.  User is invoking "all."  */ 
        printf("Dumping all SAR TX Dbg Counters and Fifobuffers\n");
        printf("-------------------------\n");

        /* Loop and dump all commands. */
        while(psCmndPtr->pcCmd != NULL)
        {
            /* Yes.  Dump the data. */
            printf("%s", psCmndPtr->pcComment);  /* No newline - RegisterDataDump() will add one */

            /* Delay long enough for printf() to complete (200 mS). */
            usleep(200000);

            /* Dump the register region */
            RegisterDataDump(psCmndPtr->ulAddr, psCmndPtr->nLen);

            /* Go to the next command to dump */
            psCmndPtr++;
        }
        /* Add newline */
        printf("\n");
    }
    else
    {
        TXDBG_CMD_DATA *psCmndPtr = sTxDbgCmdData;  /* Pointer to check for valid commands */
        /* Find the command being used from the list of legal commands.  */
        while( nRet == CMSRET_SUCCESS && nNumOptions )
        {

            /* Loop through valid commands, looking for a match */
            while(psCmndPtr->pcCmd != NULL)
            {
                /* Is it a match? */
                if(strcasecmp(psCmndPtr->pcCmd, pOptions->pszOptName) == 0)
                {
                    /* Yes.  Dump the data. */
                    printf("Data dump for %s:", psCmndPtr->pcComment);  /* No newline - RegisterDataDump() will add one */

                    /* Delay long enough for printf() to complete (200 mS). */
                    usleep(200000);
                    /* Dump the register region */
                    RegisterDataDump(psCmndPtr->ulAddr, psCmndPtr->nLen);

                    /* Add newline */
                    printf("\n");

                    /* Break out of loop to check commands */
                    break;
                }

                /* Go to the next command to check */
                psCmndPtr++;
            }

            /* Did we match a command? */
            if(psCmndPtr->pcCmd == NULL)
                fprintf(stderr, "%s: unrecognized SAR command '%s'.\n"
                            , g_szPgmName, pOptions->pszOptName);

            /* Go to next command/option */
            nNumOptions--;
            pOptions++;
        }
    }
    return( nRet );
} /* TxDbgHandler */
#endif

/***************************************************************************
 * Function Name: GetTrafficTypeStr
 * Description  : Returns the traffic type string information.
 * Returns      : string value in reference.
 ***************************************************************************/
static void GetTrafficTypeStr( UINT32 ulTrafficType, char *pcTrafficTypeStr)
{
   switch (ulTrafficType) {

      case TRAFFIC_TYPE_NOT_CONNECTED   :
         strcpy (pcTrafficTypeStr, "Not Connected") ;
         break ;
      case TRAFFIC_TYPE_ATM             :
         strcpy (pcTrafficTypeStr, "ATM") ;
         break ;
      case TRAFFIC_TYPE_ATM_BONDED      :
         strcpy (pcTrafficTypeStr, "ATM Bonded") ;
         break ;
      case TRAFFIC_TYPE_PTM             :
         strcpy (pcTrafficTypeStr, "PTM") ;
         break ;
      case TRAFFIC_TYPE_PTM_BONDED      :
         strcpy (pcTrafficTypeStr, "PTM Bonded") ;
         break ;
      case TRAFFIC_TYPE_PTM_RAW         :
         strcpy (pcTrafficTypeStr, "PTM Raw\n") ;
         break ;
      default                           :
         strcpy (pcTrafficTypeStr, "Unknown\n") ;
         break ;
   }
} /* GetTrafficTypeStr */

/***************************************************************************
 * Function Name: GetBondingProtocolStr
 * Description  : Returns the bonding protocol string information.
 * Returns      : string value in reference.
 ***************************************************************************/
static void GetBondingProtocolStr( UINT32 ulBondProto, char *pcBondingProtoStr)
{
   switch (ulBondProto) {

      case BC_BOND_PROTO_NONE       :
         strcpy (pcBondingProtoStr, "None") ;
         break ;
      case BC_BOND_PROTO_G994_AGGR  :
         strcpy (pcBondingProtoStr, "G994 Phy Aggregation & Discovery") ;
         break ;
      case BC_BOND_PROTO_ASM        :
         strcpy (pcBondingProtoStr, "ASM") ;
         break ;
      case BC_BOND_PROTO_BACP       :
         strcpy (pcBondingProtoStr, "BACP") ;
         break ;
      default                           :
         strcpy (pcBondingProtoStr, "Unknown") ;
         break ;
   }
} /* GetBondingProtocolStr */

/***************************************************************************
 * Function Name: VersionHandler
 * Description  : Processes the xtmctl version command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet VersionHandler( POPTION_INFO pOptions, int nNumOptions )
{
    fprintf(stderr, "%s version " XTMCTL_VERSION "\n", g_szPgmName );
    return( 0 );
} /* VersionHandler */


/***************************************************************************
 * Function Name: HelpHandler
 * Description  : Processes the xtmctl help command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static CmsRet HelpHandler( POPTION_INFO pOptions, int nNumOptions )
{
    fprintf( stderr,
    "Usage: %s start\n"
    "           [--rq0 <size>]\n"
    "           [--rq1 <size>]\n"
    "           [--intf allint|allext|intext [negedge]]\n"
    "           [--bondingenable]\n"
    "       %s stop\n"
    "       %s restart\n"
    "       %s bonding --status\n"
    "       %s datapath --status\n"
    "       %s sar [tx|rx|rxcam|shaper|rxpbuf|mib"
/* Only list supported SAR commands */
#ifdef SAR_RX_PAF_REGS
             "|rxpaf"
#endif
#ifdef SAR_RX_BOND_REGS
             "|rxbond"
#endif
#ifdef SAR_TMUEXT_REGS
             "|tmuext"
#endif
             "|all]\n"
#if defined(CHIP_63158) || defined(CHIP_63178)
    "       %s txdbg [bbhfifo|pktfifo|utofifo|all]\n"
    "           WARNING: Executing this debug command with bidrectional \n"
    "           traffic will affect packet forwarding to halt.\n"
    "           Requires reboot to recover fully.\n"
#endif
    "       %s config --trafficsense <traffic_timeout_seconds>\n"
    "       %s config --singleline <single_line_timeout_seconds>\n"
    "       %s threshold --adsl <thresh_val>\n"
    "       %s threshold --vdsl <thresh_val>\n"
    "       %s threshold --vdslrtx <thresh_val>\n"
    "       %s threshold --gfast <thresh_val>\n"
    "       %s operate tdte\n"
    "           [--add (ubr [<mcr>])|(ubr_pcr <pcr> [<mcr>])|(cbr <pcr>)|(rtvbr <pcr> <scr> <mbs_cell>)|(nrtvbr <pcr> <scr> <mbs_cell>)\n"
    "           [--delete <tdte_index>]\n"
    "           [--show [<tdte_index>]]\n"
    "       %s operate intf\n"
#if defined(XTM_PORT_SHAPING)
    "           [--state <port_id> enable|disable ratelimit on|off <pbr> <mbs_bytes>]\n"
#else
    "           [--state <port_id> enable|disable]\n"
#endif
    "           [--show [<port_id>]]\n"
    "           [--stats [<port_id>] [reset]]\n"
    "       %s operate conn\n"
    "           [--add <port_mask.vpi.vci> aal5\n"
    "                  llcsnap_eth|llcsnap_rtip|llcencaps_ppp|vcmux_eth|vcmux_ipoa|vcmux_pppoa\n"
#if defined(CHIP_63268) || defined(CHIP_63138) || defined(CHIP_63381) || defined(CHIP_63148) || defined(CHIP_63158) || defined(CHIP_63178)
    "                  <mp_prio> <mp_wght> "
#endif
    "[<tdte_index>]]\n"
    "           [--add <port_mask.vpi.vci> aal0pkt|aal0cell [<tdte_index>]]\n"
    "           [--add <port_mask.ptmpri_mask>]\n"
    "           [--delete <port_mask.vpi.vci>|<port_mask.ptmpri_mask>]\n"
#if defined(CHIP_63268) || defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_63158) || defined(CHIP_63178)
    "           [--addq <port_id.vpi.vci> <q_prio> wrr|wfq <q_wght> dt|(red <minThr> <maxThr>)|(wred <loMinThr> <loMaxThr> <hiMinThr> <hiMaxThr>)]\n"
    "           [--addq <port_id.ptmpri_id> <q_prio> wrr|wfq <q_wght> dt|(red <minThr> <maxThr>)|(wred <loMinThr> <loMaxThr> <hiMinThr> <hiMaxThr>) <mbr_kbps> <pbr_kbps> <mbs_byte>]\n"
#elif defined(CHIP_63381)
    "           [--addq <port_id.vpi.vci> <q_prio> wrr|wfq <q_wght>]\n"
    "           [--addq <port_id.ptmpri_id> <q_prio> wrr|wfq <q_wght> <mbr_kbps> <pbr_kbps> <mbs_byte>]\n"
#else    
    "           [--addq <port_id.vpi.vci>|<port_id.ptmpri_id> <q_prio> rr|wfq <q_wght>]\n"
#endif
    "           [--deleteq <port_id.vpi.vci>|<port_id.ptmpri_id> <qid>]\n"
    "           [--state <port_mask.vpi.vci>|<port_mask.ptmpri_mask> enable|disable]\n"
    "           [--show [<port_mask.vpi.vci>|<port_mask.ptmpri_mask>]]\n"
    "           [--sendoam <port_id.vpi.vci> f5seg|f5end|f4seg|f4end]\n"
    "           [--createnetdev <port_mask.vpi.vci>|<port_mask.ptmpri_mask> <netdevname>]\n"
    "           [--deletenetdev <port_mask.vpi.vci>|<port_mask.ptmpri_mask>]\n"
    "       %s dbg \n"
    "           [--SILI <event> up/down [<traffic_type>] atm/ptm]\n"
    "           [--SARLB <enable/disbale>]",
    g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName);

    fprintf( stderr, "\n"
    "       port_mask: bit mask of one or more port ids\n"
    "       port_id:\n"
    "           0x01 = PORT_PHY0_LATENCY0\n"
    "           0x02 = PORT_PHY0_LATENCY1\n"
    "           0x04 = PORT_PHY1_LATENCY0\n"
    "           0x08 = PORT_PHY1_LATENCY1\n"
    "       ptmpri_mask: bit mask of one or both PTM priority ids\n"
    "       ptmpri_id:\n"
    "           0x01 = PTM_PRI_LOW\n"
    "           0x02 = PTM_PRI_HIGH\n"
    "       mcr: minimum cell rate in cells/sec\n"
    "       pcr: peak cell rate in cells/sec\n"
    "       scr: sustainable cell rate in cells/sec\n"
    "       mbs_cell: maximum burst size in cells\n"
    "       mp_prio: only applicable to VC with mpaal arbiter. The priority value\n"
    "                to be used for VC arbitration. [0-7]\n"
    "       mp_wght: only applicable to VC with mpaal arbiter. The weight value\n"
    "                to be used for VC arbitration. [1-63]\n"
    "       q_prio: queue priority [0-7]\n"
    "       q_wght: queue weight [1-63]\n"
    "       mbr_kbps: PTM queue minimum bit rate in kbps. 0 = no shaping\n"
    "       pbr_kbps: PTM queue peak bit rate in kbps. 0 = no shaping\n"
    "       mbs_byte: PTM queue maximum burst size in bytes\n"
#if defined(CHIP_63268) || defined(CHIP_63138) || defined(CHIP_63148) || defined(CHIP_63158) || defined(CHIP_63178)
    "       minThr/maxThr: minimum/maximum threshold for RED drop algorithm. [1-100]%% of queue size\n"
    "       loMinThr/loMaxThr: low  class minimum/maximum threshold for WRED drop algorithm. [1-100]%% of queue size\n"
    "       hiMinThr/hiMaxThr: high class minimum/maximum threshold for WRED drop algorithm. [1-100]%% of queue size\n"
#endif
    "       SILI event: 0 LINK_DOWN, 1 LINK_UP\n"
    "            traffic_type: 0 ATM, 1 - PTM \n"
    "       SARLB enable: 0 disable, 1 - enable\n"
            );

    VersionHandler( NULL, 0 );
    return( 0 );
} /* HelpHandler */


/***************************************************************************
 * dumpmem and setmem functions
 ***************************************************************************/

#include <fcntl.h>
#include <sys/ioctl.h>
#include "board.h"
#include <errno.h>
#include <error.h>
#include <sys/mman.h>

typedef struct
{
    int mem_fd;
    char *mmap_addr;
    unsigned long addr;
    unsigned int size;
    unsigned int offset;
} MAP_INFO;

static int MapMem(unsigned long addr, int size, MAP_INFO *mapregs);
static void UnmapMem(MAP_INFO *mapregs);
static void DisplayMem( unsigned long ulRealAddr, int nLen,
    unsigned char *pMappedAddr );

/***************************************************************************
 * Function Name: MapMem
 * Description  : Maps kernel memory to user space.
 * Returns      : 0 = success, 1 = failed
 ***************************************************************************/
static int MapMem(unsigned long addr, int size, MAP_INFO *mapregs)
{
    int pagesize = getpagesize();

    memset(mapregs, 0, sizeof(MAP_INFO));
    mapregs->mem_fd = open("/dev/mem", O_RDWR+O_SYNC);
    if (mapregs->mem_fd < 0)
    {
        fprintf(stderr, "%s: device open on /dev/mem failed: %d\n",
            g_szPgmName, errno);
        return( 1 );
    }

    if( (addr & 0xfff00000) != 0xfff00000 )
        addr &= 0x1fffffff;

    /* handle page alignment */
    mapregs->addr = addr & ~(pagesize - 1);
    mapregs->offset = addr & (pagesize - 1);

    if( (mapregs->offset + size) > pagesize )
        mapregs->size = pagesize * 2;
    else
        mapregs->size = pagesize;

    mapregs->mmap_addr = mmap(0, mapregs->size, PROT_READ, MAP_SHARED,
        mapregs->mem_fd, mapregs->addr);

    if ((int)mapregs->mmap_addr == -1)
    {
        fprintf(stderr, "%s: mmap failed\n", g_szPgmName);
        close(mapregs->mem_fd);
        return( 1 );
    }

    mapregs->mmap_addr += mapregs->offset;

#if 0
    printf("MapMem: addr %x, size %x, mapped addr %x, offset %x\n",
        (int)addr, (int)mapregs->size,(int)mapregs->mmap_addr,
        (int)mapregs->offset);
#endif

    return( 0 );
}


/***************************************************************************
 * Function Name: UnmapMem
 * Description  : Unmaps kernel memory rrom user space.
 * Returns      : None.
 ***************************************************************************/
static void UnmapMem(MAP_INFO *mapregs)
{
    if (mapregs)
    {
        mapregs->mmap_addr -= mapregs->offset;
        if (munmap(mapregs->mmap_addr, mapregs->size) == -1 ||
            close(mapregs->mem_fd) == -1)
        {
            fprintf(stderr, "%s: munmap failed\n", g_szPgmName);
        }
    }
}


/***********************************************************************
 * Function Name: dumpaddr
 * Description  : Display a hex dump of the specified address.
 * Returns      : None.
 ***********************************************************************/
static void DisplayMem( unsigned long ulRealAddr, int nLen,
    unsigned char *pMappedAddr )
{
    static char szHexChars[] = "0123456789abcdef";
    char szLine[80];
    char *p = szLine;
    unsigned char ch, *q;
    int i, j;
    unsigned long ul;

    while( nLen > 0 )
    {
        sprintf( szLine, "%8.8lx: ", (unsigned long) ulRealAddr );
        p = szLine + strlen(szLine);

        for(i = 0; i < 16 && nLen > 0; i += sizeof(long), nLen -= sizeof(long))
        {
            ul = *(unsigned long *) &pMappedAddr[i];
            q = (unsigned char *) &ul;
            for( j = 0; j < sizeof(long); j++ )
            {
                *p++ = szHexChars[q[j] >> 4];
                *p++ = szHexChars[q[j] & 0x0f];
                *p++ = ' ';
            }
        }

        for( j = 0; j < 16 - i; j++ )
            *p++ = ' ', *p++ = ' ', *p++ = ' ';

        *p++ = ' ', *p++ = ' ', *p++ = ' ';

        for( j = 0; j < i; j++ )
        {
            ch = pMappedAddr[j];
            *p++ = (ch > ' ' && ch < '~') ? ch : '.';
        }

        *p++ = '\0';
        printf( "%s\r\n", szLine );

        pMappedAddr += i;
        ulRealAddr += i;
    }
    printf( "\r\n" );
} /* dumpmem */


//**************************************************************************
// Function Name: DumpMem
// Description  : Dump kernel registers or address.
// Returns      : None.
//**************************************************************************
static void DumpMem( int argc, char **argv )
{
    if( argc == 3 || (argc == 4 && argv[3][0] == 'b') )
    {
        unsigned long ulAddr = (unsigned long)strtoul(argv[1], (char **)NULL,16);
        int nLen= (int) strtol(argv[2], (char **)NULL, 10);

        if( nLen == 0 )
            fprintf( stderr, "%s: Length is 0\n\n", g_szPgmName );
        else
        {
            if( nLen > 2048 )
            {
                fprintf( stderr, "%s: Length must be less than 2048\n\n",
                    g_szPgmName );
            }
            else
            {
                if( argc == 3 || argv[3][0] != 'b' )
                {
                    /* display memory dump in user mode */
                    MAP_INFO MapInfo;

                    ulAddr &= ~0x03; /* force to 4 byte boundary */
                    if( MapMem( ulAddr, nLen, &MapInfo ) == 0 )
                    {
                        printf( "%s 0x%lx %d\n\n", g_szPgmName, ulAddr, nLen );
                        DisplayMem(ulAddr, nLen, (unsigned char *)
                            MapInfo.mmap_addr);
                        UnmapMem( &MapInfo );
                    }
                }
                else
                {
                    /* display memory dump in the kernel */
                    int f = open( "/dev/brcmboard", O_RDWR );

                    if( f != -1 )
                    {
                        BOARD_IOCTL_PARMS IoctlParms;
                        memset( &IoctlParms, 0x00, sizeof(IoctlParms) );
                        IoctlParms.string = (char *) ulAddr;
                        IoctlParms.strLen = nLen;
                        printf( "%s 0x%lx %d (from kernel)\n\n", g_szPgmName,
                            ulAddr, nLen );
                        ioctl( f, BOARD_IOCTL_DUMP_ADDR, &IoctlParms);
                        close(f);
                    }
                    else 
                    {
                        fprintf(stderr, "%s: Unable to open device "
                            "/dev/brcmboard\n", g_szPgmName);
                    }
                }
            }
        }
    }
    else
    {
        fprintf( stderr, "usage: %s <address_in_hex> <length_in_decimal>\n\n",
            g_szPgmName );
    }
} /* DumpMem */


//**************************************************************************
// Function Name: SetMem
// Description  : Set kernel address/register long/short/char.
// Returns      : None.
//**************************************************************************
static void SetMem( int argc, char **argv )
{
    if( argc == 4 )
    {
        unsigned long ulAddr = (unsigned long)strtoul(argv[1], (char **)NULL,16);
        unsigned long ulValue = (unsigned long)strtoul(argv[2],(char **)NULL,16);
        int nSize = (int) strtol( argv[3], (char **) NULL, 10 );

        if( nSize == 4 || nSize == 2 || nSize == 1 )
        {
            int f = open( "/dev/brcmboard", O_RDWR );

            if( f != -1 )
            {
                BOARD_IOCTL_PARMS IoctlParms;
                memset( &IoctlParms, 0x00, sizeof(IoctlParms) );
                IoctlParms.string = (char *) ulAddr;
                IoctlParms.strLen = nSize;
                IoctlParms.offset = ulValue;
                printf( "%s 0x%8.8lx 0x%8.8lx %d\n\n", g_szPgmName, ulAddr,
                    ulValue, nSize );
                ioctl( f, BOARD_IOCTL_SET_MEMORY, &IoctlParms);
                close(f);
            }
            else 
            {
                fprintf(stderr, "%s: Unable to open device /dev/brcmboard\n",
                    g_szPgmName);
            }
        }
        else
            fprintf( stderr, "%s: size must be 1, 2 or 4\n\n", g_szPgmName );
    }
    else
    {
        fprintf( stderr, "usage: %s <address_in_hex> <value_in_hex> "
            "<size_in_decimal:4, 2 or 1>\n\n", g_szPgmName );
    }
} /* SetMem */

