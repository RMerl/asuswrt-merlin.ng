/* 
* <:copyright-BRCM:2006:proprietary:standard
* 
*    Copyright (c) 2006 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/

/** Includes. **/
#include <stdio.h>
#include <errno.h>
#include <linux/version.h>

#if defined(__linux__)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
#include "devctl_adsl.h"
#else
typedef unsigned char   UINT8;
typedef unsigned short  UINT16, ushort;
typedef unsigned int    UINT32;
#endif
#elif defined(__ECOS)
#include "devctl_adsl.h"
#include <bcmtypes.h>
#endif /* !__linux__ && !__ECOS */


#ifdef XDSL_CTL_API
#include "adslctlapi.h"
#endif
#include "DiagDef.h"
#include "AdslMibDef.h"

/** Defines. **/
#define ADSLCTL_VERSION                         "1.2"

#ifdef DMP_VDSL2WAN_1
#define  CONFIG_VDSL_SUPPORTED
#endif


/* Limit values */
#define CMD_NAME_LEN                            16
#define MAX_OPTS                                28
#define MAX_PARMS                               19

/* Argument type values. */
#define ARG_TYPE_COMMAND                        1
#define ARG_TYPE_OPTION                         2
#define ARG_TYPE_PARAMETER                      3

/* Return codes. */
#define ADSLCTL_GENERAL_ERROR                   100
#define ADSLCTL_ALLOC_ERROR                     101
#define ADSLCTL_INVALID_COMMAND                 102
#define ADSLCTL_INVALID_OPTION                  103
#define ADSLCTL_INVALID_PARAMETER               104
#define ADSLCTL_INVALID_NUMBER_OF_OPTIONS       105
#define ADSLCTL_INVALID_NUMBER_OF_PARAMETERS    106

/* adsl info types */
#define  INFO_TYPE_NONE                         0
#define  INFO_TYPE_STATE                        1
#define  INFO_TYPE_SHOW                         2
#define  INFO_TYPE_STATS                        3
#define  INFO_TYPE_SNR                          4
#define  INFO_TYPE_QLN                          5
#define  INFO_TYPE_Hlog                         6
#define  INFO_TYPE_DIAGMODE                     7
#define  INFO_TYPE_HLIN_RAW                     8
#define  INFO_TYPE_HLIN_BITALLOC                9
#define  INFO_TYPE_HLIN_SCALED                  10
#define  INFO_TYPE_VENDOR                       11
#define  INFO_TYPE_CONFIG                       12
#define  INFO_TYPE_DIAGMODE_1                   13
#define  INFO_TYPE_VDSL_PER_BAND_PARAMS         14
#define  INFO_TYPE_VECTORING_STATE              16
#define  INFO_TYPE_24HR_STATS                   17
#define  INFO_TYPE_UER                          18
#define  INFO_TYPE_ECHO_VARIANCE                19
#define  INFO_TYPE_RNC_QLN                      20
#define  INFO_TYPE_ALN                          21
#define  INFO_TYPE_DOI_BITALLOC                 22
#define  INFO_TYPE_TOD                          23

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif

#define ATTRIBUTE_UNUSED   __attribute__((unused))

/** Macros for Linux Release 3 **/

#if defined(__linux__)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21))
typedef enum
{
   CMSRET_SUCCESS        = 0,     /**BCMADSL_STATUS_SUCCESS */
   CMSRET_INTERNAL_ERROR          /** BCMADSL_STATUS_ERROR */
} CmsRet;
#define xdslCtl_DiagProcessCommandFrame(lineId,...)	BcmAdsl_DiagProcessCommandFrame(__VA_ARGS__)
#define xdslCtl_DiagProcessDbgCommand(lineId,...)		BcmAdsl_DiagProcessDbgCommand(__VA_ARGS__)
#define xdslCtl_Initialize(lineId,...)					BcmAdsl_Initialize(__VA_ARGS__)
#define xdslCtl_ConnectionStart(lineId)				BcmAdsl_ConnectionStart()
#define xdslCtl_ConnectionStop(lineId)				BcmAdsl_ConnectionStop()
#define xdslCtl_Configure(lineId,...)					BcmAdsl_Configure(__VA_ARGS__)
#define xdslCtl_SetTestMode(lineId,...)				BcmAdsl_SetTestMode(__VA_ARGS__)
#define xdslCtl_SelectTones(lineId,...)					BcmAdsl_SelectTones(__VA_ARGS__)
#define xdslCtl_Uninitialize(lineId)					BcmAdsl_Uninitialize()
#define xdslCtl_BertStartEx(lineId,...)					BcmAdsl_BertStartEx(__VA_ARGS__)
#define xdslCtl_BertStopEx(lineId)					BcmAdsl_BertStopEx()
#define xdslCtl_GetObjectValue(lineId,...)				BcmAdsl_GetObjectValue(__VA_ARGS__)
#define xdslCtl_SetObjectValue(lineId,...)				BcmAdsl_SetObjectValue(__VA_ARGS__)
#define xdslCtl_ResetStatCounters(lineId)				BcmAdsl_ResetStatCounters()
#define xdslCtl_GetVersion(lineId,...)					BcmAdsl_GetVersion(__VA_ARGS__)
#elif !defined(XDSL_CTL_API)
#define xdslCtl_DiagProcessCommandFrame(lineId,...)	devCtl_adslDiagProcessCommandFrame(__VA_ARGS__)
#define xdslCtl_DiagProcessDbgCommand(lineId,...)		BcmAdsl_DiagProcessDbgCommand(__VA_ARGS__)
#define xdslCtl_Initialize(lineId,...)					devCtl_adslInitialize(__VA_ARGS__)
#define xdslCtl_ConnectionStart(lineId)				devCtl_adslConnectionStart()
#define xdslCtl_ConnectionStop(lineId)				devCtl_adslConnectionStop()
#define xdslCtl_Configure(lineId,...)					devCtl_adslConfigure(__VA_ARGS__)
#define xdslCtl_SetTestMode(lineId,...)				devCtl_adslSetTestMode(__VA_ARGS__)
#define xdslCtl_SelectTones(lineId,...)					devCtl_adslSelectTones(__VA_ARGS__)
#define xdslCtl_Uninitialize(lineId)					devCtl_adslUninitialize()
#define xdslCtl_BertStartEx(lineId,...)					devCtl_adslBertStartEx(__VA_ARGS__)
#define xdslCtl_BertStopEx(lineId)					devCtl_adslBertStopEx()
#define xdslCtl_GetObjectValue(lineId,...)				devCtl_adslGetObjectValue(__VA_ARGS__)
#define xdslCtl_SetObjectValue(lineId,...)				devCtl_adslSetObjectValue(__VA_ARGS__)
#define xdslCtl_ResetStatCounters(lineId)				devCtl_adslResetStatCounters()
#define xdslCtl_GetVersion(lineId,...)					devCtl_adslGetVersion(__VA_ARGS__)
#endif
#elif defined(__ECOS)
#endif

/** More Typedefs. **/

typedef struct
{
    char *pszOptName;
    char *pszParms[MAX_PARMS];
    int nNumParms;
} OPTION_INFO, *POPTION_INFO;

typedef int (*FN_COMMAND_HANDLER) (unsigned char lineId, POPTION_INFO pOptions, int nNumOptions );

typedef struct
{
    char szCmdName[CMD_NAME_LEN];
    char *pszOptionNames[MAX_OPTS];
    FN_COMMAND_HANDLER pfnCmdHandler;
} COMMAND_INFO, *PCOMMAND_INFO;

#ifdef ANNEX_C
#define CFG_CMD_ANNEX_AC  "--bm", "--ccw"
#else /* Allow for AnnexA/AnnexB builds */
#define CFG_CMD_ANNEX_AC  "--forceJ43", "--toggleJ43B43"
#endif

#ifdef CONFIG_VDSL_SUPPORTED
#define CFG_CMD_VDSL	"--profile","--us0", "--dynamicD", "--dynamicF", "--SOS", "--maxDataRate",
#ifdef SUPPORT_DSL_GFAST
#define HELP_CMD_OPTION_MOD	"       %s configure/configure1 [--mod <a|d|l|t|2|p|e|m|M3|M5|v|r|f>] [--lpair <(i)nner|(o)uter>]\n"
#else
#define HELP_CMD_OPTION_MOD	"       %s configure/configure1 [--mod <a|d|l|t|2|p|e|m|M3|M5|v|r>] [--lpair <(i)nner|(o)uter>]\n"
#endif
#ifdef CONFIG_VDSLBRCMPRIV1_SUPPORT
#define HELP_CMD_OPTION_PROFILE	"           [--profile <0x00 - 0x1FF>|<\"8a |8b |8c |8d |12a |12b |17a |30a |35b\">] [--us0 <on|off>]\n"
#else
#define HELP_CMD_OPTION_PROFILE	"           [--profile <0x00 - 0xFF>|<\"8a |8b |8c |8d |12a |12b |17a |30a\">] [--us0 <on|off>]\n"
#endif
#else	/* !CONFIG_VDSL_SUPPORTED */
#define CFG_CMD_VDSL
#define HELP_CMD_OPTION_MOD	"       %s configure/configure1 [--mod <a|d|l|t|2|p|e|m|M3|M5>] [--lpair <(i)nner|(o)uter>]\n"
#endif

#define CFG_CMD_OPTIONS { \
	"--up", /* start command only */    \
	"--mod", "--lpair", "--trellis", "--snr", "--bitswap","--sesdrop", "--sra","--CoMinMgn", "--minINP", "--maxDelay", "--phycfg", "--mcfg", "--lcfg", \
	CFG_CMD_ANNEX_AC, \
    "--i24k", "--phyReXmt", "--Ginp", "--TpsTc", "--monitorTone", \
    CFG_CMD_VDSL \
    NULL \
}

/** Prototypes. **/

static int GetArgType( char *pszArg, PCOMMAND_INFO pCmds, char **ppszOptions );
static PCOMMAND_INFO GetCommand( char *pszArg, PCOMMAND_INFO pCmds );
static int ProcessCommand(unsigned char lineId, PCOMMAND_INFO pCmd, int argc, char **argv,
							PCOMMAND_INFO pCmds, int *pnArgNext );
static int StartHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
static int ConnectionHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
static int ConfigureHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
static int ConfigureHandler1(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
static int InfoHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
static int BertHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
static int DiagHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
static int AfelbHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
static int QLNHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
static int INMHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
static int SnrClampHandler( unsigned char lineId, POPTION_INFO pOptions, int nNumOptions );
static int NonLinCmdHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
static int StopHandler(unsigned char lineId ATTRIBUTE_UNUSED, POPTION_INFO pOptions ATTRIBUTE_UNUSED, int nNumOptions ATTRIBUTE_UNUSED);
static int VersionHandler(unsigned char lineId ATTRIBUTE_UNUSED, POPTION_INFO pOptions ATTRIBUTE_UNUSED, int nNumOptions ATTRIBUTE_UNUSED);
static int HelpHandler(unsigned char lineId ATTRIBUTE_UNUSED, POPTION_INFO pOptions ATTRIBUTE_UNUSED, int nNumOptions ATTRIBUTE_UNUSED);

#define  SIGN_MASK   0x80000000
static int GetInteger(char **sPtrPtr, int *outValue, int base);
static int GetToneMap(char *ppStr, char *pToneMap, int toneMapSize);
static int GetSnrClampShape(char *ppStr, char *snrClampShape, int snrClampSize);

static int ProfileHandler( unsigned char lineId, POPTION_INFO pOptions, int nNumOptions );
static void ProfileShow(unsigned char lineId);
#ifdef SUPPORT_SELT
static int SeltHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
#endif
#ifdef NTR_SUPPORT
static int NTRHandler(unsigned char lineId,  POPTION_INFO pOptions, int nNumOptions );
#endif

/** Globals. **/

static COMMAND_INFO g_Cmds[] = {
     {"start", CFG_CMD_OPTIONS, StartHandler},
     {"stop", {NULL}, StopHandler},
     {"connection", {"--up", "--down", "--loopback", "--reverb", "--medley","--noretrain","--tones", "--L3", "--diagmode", "--L0", "--normal", "--freezeReverb", "--freezeMedley", NULL}, ConnectionHandler},
     {"configure", CFG_CMD_OPTIONS, ConfigureHandler},
     {"configure1", CFG_CMD_OPTIONS, ConfigureHandler1},
     {"bert", {"--start", "--stop", "--show", NULL}, BertHandler},
#ifdef CONFIG_VDSL_SUPPORTED
     {"info", {"--state", "--show", "--sho1", "--stats", "--SNR", "--QLN", "--Hlog", "--Hlin", "--HlinS", "--Bits", "--pbParams", "--linediag", "--linediag1", "--reset", "--vendor", "--cfg", "--toneGroupObjects",
#ifdef SUPPORT_VECTORING
            "--vectoring",
#endif
#ifdef SUPPORT_24HR_CNT_STAT
            "--24hrhiststat",
#endif
#if defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
         "--UER",
#ifdef SUPPORT_SELT
         "--EchoVariance",
#endif
#endif
#if defined(CONFIG_RNC_SUPPORT)
         "--RNC_QLN",
#endif
#if defined(SUPPORT_DSL_GFAST)
         "--ALN", "--BitsDOI",
#endif
#ifdef CONFIG_TOD_SUPPORTED
         "--TOD",
#endif
            NULL}, InfoHandler},
#else /* CONFIG_VDSL_SUPPORTED */
     {"info", {"--state", "--show", "--sho1", "--stats", "--SNR", "--QLN", "--Hlog", "--Hlin", "--HlinS", "--Bits", "--linediag", "--reset", "--vendor", "--cfg", NULL}, InfoHandler},
#endif
     {"nlnm", {"--show", "--setThld", NULL}, NonLinCmdHandler},
     {"diag", {"--cmd", "--logstart", "--logpause", "--logstop", "--loguntilbufferfull", "--loguntilretrain", "--dumpBuf", "--dbgcmd",
#ifdef SUPPORT_MULTI_PHY
            "--mediaSearchCfg",
#endif
#if defined(SUPPORT_DSL_GFAST)
            "--phyTypeCfg",
#endif
            NULL}, DiagHandler},
     {"afelb", {"--time","--tones", "--signal", NULL}, AfelbHandler},
     {"qlnmntr", {"--time", "--freq", NULL}, QLNHandler},
     {"inm", {"--start", "--stop","--show", NULL}, INMHandler},
     {"snrclamp", {"--shape","--bpshape", NULL}, SnrClampHandler},
     {"profile", {"--show","--save","--restore", NULL}, ProfileHandler},
#ifdef SUPPORT_SELT
     {"selt", {"--start", "--stop", "--status", "--steps", "--cfg", NULL}, SeltHandler},
#endif
#ifdef NTR_SUPPORT
     {"ntr", {"--start", "--stop", NULL}, NTRHandler},
#endif
     {"--version", {NULL}, VersionHandler},
     {"--help", {NULL}, HelpHandler},
     {"", {NULL}, NULL}
};

static char g_szPgmName[80] = {0};
#ifndef CONFIG_VDSL_SUPPORTED
static short scratchBuf[kAdslMibAnnexAToneNum*2*2];
#else

#if defined(CONFIG_VDSLBRCMPRIV2_SUPPORT)
#define SCRATCH_SIZE (kVdslMibToneNum*2*2*4)
#elif defined(CONFIG_VDSLBRCMPRIV1_SUPPORT)
#define SCRATCH_SIZE (kVdslMibToneNum*2*4)
#elif defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
#define SCRATCH_SIZE (kVdslMibToneNum*4)
#else
#define SCRATCH_SIZE (kVdslMibToneNum*2)
#endif
static short scratchBuf[SCRATCH_SIZE];

#endif


/***************************************************************************
 * Function Name: main
 * Description  : Main program function.
 * Returns      : 0 - success, non-0 - error.
 ***************************************************************************/
#ifdef BUILD_STATIC
int adslctl_main(int argc, char **argv)
#elif defined(__ECOS)
int xdslctl_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    int nExitCode = 0;
    PCOMMAND_INFO pCmd;
    unsigned char lineId = 0;

    /* Save the name that started this program into a global variable. */
    strcpy( g_szPgmName, *argv );

    if( argc == 1 )
        HelpHandler (0, NULL, 0);
#ifdef SUPPORT_DSL_BONDING
    if( !strcmp( argv[0], "xdslctl1" ) )
        lineId = 1;
#endif

    argc--, argv++;
    while( argc && nExitCode == 0 )
    {
        if( GetArgType( *argv, g_Cmds, NULL ) == ARG_TYPE_COMMAND )
        {
            int argnext = 0;
            pCmd = GetCommand( *argv, g_Cmds );
            nExitCode = ProcessCommand(lineId, pCmd, --argc, ++argv, g_Cmds, &argnext);
            argc -= argnext;
            argv += argnext;
        }
        else
        {
            nExitCode = ADSLCTL_INVALID_COMMAND;
            fprintf( stderr, "%s: invalid command\n", g_szPgmName );
        }
    }

    return ( nExitCode );
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

static SINT32 _f2DecI(SINT32 val, int q)
{
   return (val/q);
}

static SINT32 _f2DecF(SINT32 val, int q)
{
   int      sn = val >> 31;
   return (((val ^ sn) - sn) % q);
}


#if !defined(XDSL_CTL_API)
static SINT32 Qn2DecF(SINT32 qnVal, int q)
{
   int      sn = qnVal >> 31;
   return (((qnVal ^ sn) - sn) & ((1 << q) - 1)) * (10000 >> q);
}

static char* QnToString(SINT32 val, int q)
{
   static   char  str1[32];
   SINT32        iPart;

   if (val < 0) {
      val = -val;
      iPart = -(val >> q);
      if (0 == iPart) {
         sprintf(str1, "-0.%04u", Qn2DecF(val,q));
         return str1;
      }
   }
   else
      iPart = val >> q;
   sprintf( str1, "%d.%04d", iPart, Qn2DecF(val,q));
   return str1;
}
#endif

#ifdef CONFIG_VDSL_SUPPORTED
static char* QnToString1(int val, int q, int ipartlen)
{
   static   char  str1[32];
   int        iPart;

   if (val < 0) {
      val = -val;
      iPart = -(val >> q);
      if (0 == iPart) {
         if(ipartlen==5)
         	sprintf(str1," ");
         sprintf(str1,"  -0.%1d", (int)((Qn2DecF(val,q)+500)/1000));
         return str1;
      }
   }
   else
      iPart = val >> q;
   if(ipartlen==5)
      sprintf( str1, "%5d.%1d", iPart, (int)((Qn2DecF(val,q)+500)/1000));
   else sprintf( str1, "%4d.%1d", iPart, (int)((Qn2DecF(val,q)+500)/1000));
   return str1;
}
#endif
/***************************************************************************
 * Function Name: ProcessCommand
 * Description  : Gets the options and option paramters for a command and
 *                calls the command handler function to process the command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int ProcessCommand(unsigned char lineId, PCOMMAND_INFO pCmd, int argc, char **argv,
    PCOMMAND_INFO pCmds, int *pnArgNext )
{
    int nRet = 0;
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
                nRet = ADSLCTL_INVALID_NUMBER_OF_OPTIONS;
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
                    nRet = ADSLCTL_INVALID_PARAMETER;
                    fprintf( stderr, "%s: invalid parameter\n", g_szPgmName );
                }
                else
                {
                    nRet = ADSLCTL_INVALID_NUMBER_OF_PARAMETERS;
                    fprintf( stderr, "%s: too many parameters\n", g_szPgmName );
                }
            }
            (*pnArgNext)++;
            break;

        case ARG_TYPE_COMMAND:
            /* The current command is done. */
            break;
        }

        argc--, argv++;
    } while( nRet == 0 && nArgType != ARG_TYPE_COMMAND );

    if( nRet == 0 )
        nRet = (*pCmd->pfnCmdHandler) (lineId, OptInfo, nNumOptInfo);

    return( nRet );
} /* ProcessCommand */


SINT32 GetHexOrDec(char *str)
{
    SINT32  n;
    if ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X'))) {
        n = strtoul(str+2, NULL, 16);
    } else {
        n = strtoul(str, NULL, 10);
    }
    return n;
}

#define GetMaskValue(cfgName,sMask,sVal) do {  \
   SINT32 mask, val;                             \
                                               \
   mask = GetHexOrDec(sMask);                  \
   val  = GetHexOrDec(sVal);                   \
                                               \
   cfgName##Mask |= mask;                      \
   cfgName##Value |= (mask & val);             \
   cfgName##Value &= ~(mask & ~val);           \
} while (0)

/***************************************************************************
 * Function Name: ParseCfgParam
 * Description  : Processes the adslctl start command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int ParseCfgParam (POPTION_INFO pOptions, int nNumOptions, int bInit, adslCfgProfile *pAdslCfg, int *pUp)
{
   int     i, nRet = 0;
   SINT32  adslCfgParam;
   char  ch;
   adslVersionInfo adslVer;
#ifdef CONFIG_VDSL_SUPPORTED
   SINT32 vdslProfileMask;
#ifdef SUPPORT_DSL_GFAST
   SINT32 gfastProfileMask;
#endif
#ifdef CONFIG_VDSLBRCMPRIV2_SUPPORT
   vdslProfileMask = kVdslProfileMask3;
#elif defined(CONFIG_VDSLBRCMPRIV1_SUPPORT)
   vdslProfileMask = kVdslProfileMask2;
#else
   vdslProfileMask = kVdslProfileMask1;
#endif
#ifdef SUPPORT_DSL_GFAST
#ifdef CONFIG_BCM963138
   gfastProfileMask = kGfastProfileMask;
#else
   gfastProfileMask = kGfastProfileMask1;
#endif
   vdslProfileMask |= gfastProfileMask;
#endif
#endif  /* CONFIG_VDSL_SUPPORTED */

   memset((void*)&adslVer, 0, sizeof(adslVer));
   xdslCtl_GetVersion(0, &adslVer);

   if (bInit) {
   adslCfgParam = kAdslCfgModAny | kAdslCfgLineInnerPair |  kAdslCfgTrellisOn | 
      kAdslCfgLOSMonitoringOn | kAdslCfgDemodCapOn;
   pAdslCfg->adslTrainingMarginQ4 = kAdslCfgDefaultTrainingMargin;
   pAdslCfg->adslShowtimeMarginQ4 = kAdslCfgDefaultShowtimeMargin;
   pAdslCfg->adslLOMTimeThldSec   = kAdslCfgDefaultLOMTimeThld;
   pAdslCfg->adslDemodCapMask  = 0;
   pAdslCfg->adslDemodCapValue = 0;
   pAdslCfg->adslDemodCap2Mask  = 0;
   pAdslCfg->adslDemodCap2Value = 0;
#ifdef CONFIG_VDSL_SUPPORTED
   pAdslCfg->vdslParam = vdslProfileMask | kVdslUS0Mask;
#ifdef SUPPORT_DSL_GFAST
   pAdslCfg->vdslParam &= ~gfastProfileMask;  /* Enable G.fast profiles by default */
#endif
   pAdslCfg->vdslParam1 = 0;
#endif /* CONFIG_VDSL_SUPPORTED */
   }
   else {
#ifdef ANNEX_C
     adslCfgParam = pAdslCfg->adslAnnexCParam;
#else
     adslCfgParam = pAdslCfg->adslAnnexAParam;
#endif
   }

   *pUp = 0;
   pOptions--;

   while ((0 == nRet) && (nNumOptions-- > 0)) {
      pOptions++;
      if (0 == strcmp(pOptions->pszOptName, "--mod")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         adslCfgParam &= ~kAdslCfgModMask;
         pAdslCfg->adsl2Param = 0;
         i = 0;
        while ((ch = pOptions->pszParms[0][i]) != 0) {
            if ('a' == ch) {
               adslCfgParam |= kAdslCfgModAny;
               /* pAdslCfg->adsl2Param |= kAdsl2CfgReachExOn; */
               break;
            }
            else if ('d' == ch)
               adslCfgParam |= kAdslCfgModGdmtOnly;
            else if ('l' == ch)
               adslCfgParam |= kAdslCfgModGliteOnly;
            else if ('t' == ch)
               adslCfgParam |= kAdslCfgModT1413Only;
            else if ('2' == ch)
               adslCfgParam |= kAdslCfgModAdsl2Only;
            else if ('p' == ch)
               adslCfgParam |= kAdslCfgModAdsl2pOnly;
            else if ('e' == ch)
               pAdslCfg->adsl2Param |= kAdsl2CfgReachExOn;
            else if ('m' == ch)
               pAdslCfg->adsl2Param |= kAdsl2CfgAnnexMEnabled | kAdsl2CfgAnnexMp3 | kAdsl2CfgAnnexMp5;
            else if ('M' == ch) {
               i++;
               ch = pOptions->pszParms[0][i];
               if(('3' == ch) || ('5' == ch)) {
                  pAdslCfg->adsl2Param |= kAdsl2CfgAnnexMEnabled;
                  if('3' == ch)
                     pAdslCfg->adsl2Param |= kAdsl2CfgAnnexMp3;
                  else
                     pAdslCfg->adsl2Param |= kAdsl2CfgAnnexMp5;
               }
               else {
                  printf("Unknown param: M%c\n", ch);
                  if( 0 == ch )
                     break;
               }
            }
#ifdef CONFIG_VDSL_SUPPORTED
            else if ('v' == ch)
                adslCfgParam |= kDslCfgModVdsl2Only;
            else if ('r' == ch)
                adslCfgParam |= kDslCfgModVdsl2LROnly;
#ifdef SUPPORT_DSL_GFAST
            else if ('f' == ch)
                adslCfgParam |= kDslCfgModGfastOnly;
#endif
#endif
            else {
               nRet = ADSLCTL_INVALID_PARAMETER;
               break;
            }
            i++;
         }
         if((pAdslCfg->adsl2Param & kAdsl2CfgAnnexMEnabled) &&
            (kAdslCfgModAny == (adslCfgParam & kAdslCfgModMask)))
            pAdslCfg->adsl2Param |= kAdsl2CfgAnnexMOnly;
         if(pAdslCfg->adsl2Param & kAdsl2CfgReachExOn)
            adslCfgParam |= kAdslCfgModAdsl2Only;
      }
#ifdef CONFIG_VDSL_SUPPORTED
      else if (0 == strcmp(pOptions->pszOptName, "--profile")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         pAdslCfg->vdslParam &= ~vdslProfileMask;
         if ((pOptions->pszParms[0][0] == '0') &&
            ((pOptions->pszParms[0][1] == 'x') || (pOptions->pszParms[0][1] == 'X'))) {
            pAdslCfg->vdslParam |= strtoul(pOptions->pszParms[0]+2, NULL, 16) & vdslProfileMask;
         }
         else {
            char *pLast=NULL;
            char *profileName = strtok_r(pOptions->pszParms[0], " ", &pLast);
            while(profileName) {
               if (0 == strcmp (profileName, "8a"))
                    pAdslCfg->vdslParam |= kVdslProfile8a;
               else if (0 == strcmp (profileName, "8b"))
                    pAdslCfg->vdslParam |= kVdslProfile8b;
                else if (0 == strcmp (profileName, "8c"))
                    pAdslCfg->vdslParam |= kVdslProfile8c;
                else if (0 == strcmp (profileName, "8d"))
                    pAdslCfg->vdslParam |= kVdslProfile8d;
                else if (0 == strcmp (profileName, "12a"))
                    pAdslCfg->vdslParam |= kVdslProfile12a;
                else if (0 == strcmp (profileName, "12b"))
                    pAdslCfg->vdslParam |= kVdslProfile12b;
                else if (0 == strcmp (profileName, "17a"))
                    pAdslCfg->vdslParam |= kVdslProfile17a;
                else if (0 == strcmp (profileName, "30a"))
                    pAdslCfg->vdslParam |= kVdslProfile30a;
#ifdef CONFIG_VDSLBRCMPRIV1_SUPPORT
                else if (0 == strcmp (profileName, "35b"))
                    pAdslCfg->vdslParam |= kVdslProfile35b;
#ifdef CONFIG_VDSLBRCMPRIV2_SUPPORT
                else if (0 == strcmp (profileName, "BrcmPriv2"))
                    pAdslCfg->vdslParam |= kVdslProfileBrcmPriv2;
#endif
#endif
#ifdef SUPPORT_DSL_GFAST
                else if (0 == strcmp (profileName, "106a"))
                    pAdslCfg->vdslParam |= kGfastProfile106aDisable;
                else if (0 == strcmp (profileName, "106b"))
                    pAdslCfg->vdslParam |= kGfastProfile106bDisable;
#ifndef CONFIG_BCM963138
                else if (0 == strcmp (profileName, "212a"))
                    pAdslCfg->vdslParam |= kGfastProfile212aDisable;
                else if (0 == strcmp (profileName, "106c"))
                    pAdslCfg->vdslParam |= kGfastProfile106cDisable;
                else if (0 == strcmp (profileName, "212c"))
                    pAdslCfg->vdslParam |= kGfastProfile212cDisable;
#endif
#endif /* SUPPORT_DSL_GFAST */
                else
                    printf("Ignore unknown profilename:%s\n",  profileName);
                profileName = strtok_r(NULL, " ", &pLast);
            }
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--us0")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         if (0 == strcmp (pOptions->pszParms[0], "on"))
            pAdslCfg->vdslParam |= kVdslUS0Mask;
         else if (0 == strcmp (pOptions->pszParms[0], "off"))
            pAdslCfg->vdslParam &= ~kVdslUS0Mask;
         else
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--dynamicD")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         pAdslCfg->vdslCfgFlagsMask |= kVdslDynamicDDisable;    /* CfgFlagsDynamicDFeatureDisable */
         if (0 == strcmp (pOptions->pszParms[0], "off"))
            pAdslCfg->vdslCfgFlagsValue |= kVdslDynamicDDisable;
         else if (0 == strcmp (pOptions->pszParms[0], "on"))
            pAdslCfg->vdslCfgFlagsValue &= ~kVdslDynamicDDisable;
         else
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--dynamicF")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         pAdslCfg->vdslCfgFlagsMask |= kVdslDynamicFDisable;    /* CfgFlagsDynamicFFeatureDisable */
         if (0 == strcmp (pOptions->pszParms[0], "off"))
            pAdslCfg->vdslCfgFlagsValue |= kVdslDynamicFDisable;
         else if (0 == strcmp (pOptions->pszParms[0], "on"))
            pAdslCfg->vdslCfgFlagsValue &= ~kVdslDynamicFDisable;
         else
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--SOS")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         pAdslCfg->vdslCfgFlagsMask |= kVdslSOSDisable;    /* CfgFlagsSOSFeatureDisable */
         pAdslCfg->xdslAuxFeaturesMask |= kVdslSOSEnableAux | kVdslROCEnableAux;

         if (0 == strcmp (pOptions->pszParms[0], "off")) {
            pAdslCfg->vdslCfgFlagsValue |= kVdslSOSDisable;
            pAdslCfg->xdslAuxFeaturesValue &= ~(kVdslSOSEnableAux | kVdslROCEnableAux);
         }
         else if (0 == strcmp (pOptions->pszParms[0], "on")) {
            pAdslCfg->vdslCfgFlagsValue &= ~kVdslSOSDisable;
            pAdslCfg->xdslAuxFeaturesValue |= kVdslSOSEnableAux | kVdslROCEnableAux;
         }
         else
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--maxDataRate")) {
         if (3 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         if ((pOptions->pszParms[0][0] == '0') && 
             ((pOptions->pszParms[0][1] == 'x') || (pOptions->pszParms[0][1] == 'X'))) {
            pAdslCfg->maxDsDataRateKbps|= strtoul(pOptions->pszParms[0]+2, NULL, 16);
         }
         else
            pAdslCfg->maxDsDataRateKbps |= strtoul(pOptions->pszParms[0], NULL, 10);
         
         if ((pOptions->pszParms[1][0] == '0') && 
             ((pOptions->pszParms[1][1] == 'x') || (pOptions->pszParms[1][1] == 'X'))) {
            pAdslCfg->maxUsDataRateKbps|= strtoul(pOptions->pszParms[1]+2, NULL, 16);
         }
         else
            pAdslCfg->maxUsDataRateKbps |= strtoul(pOptions->pszParms[1], NULL, 10);
         
         if ((pOptions->pszParms[2][0] == '0') && 
             ((pOptions->pszParms[2][1] == 'x') || (pOptions->pszParms[2][1] == 'X'))) {
            pAdslCfg->maxAggrDataRateKbps|= strtoul(pOptions->pszParms[2]+2, NULL, 16);
         }
         else
            pAdslCfg->maxAggrDataRateKbps |= strtoul(pOptions->pszParms[2], NULL, 10);
         if(((UINT32)pAdslCfg->maxDsDataRateKbps > 0x3FFFC) ||
            ((UINT32)pAdslCfg->maxUsDataRateKbps > 0x3FFFC) ||
            ((UINT32)pAdslCfg->maxAggrDataRateKbps > 0x7FFF8)) {
            printf("Parameter value out of range.  Valid value:\n0 <= max[D/U]sDataRateKbps <= 0x3FFFC and 0 <= maxAggrDataRateKbps <= 0x7FFF8\n");
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
      }
#endif
      else if (0 == strcmp(pOptions->pszOptName, "--lpair")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         adslCfgParam &= ~kAdslCfgLinePairMask;
         /* ch = tolower(pOptions->pszParms[0][0]); */
         ch = pOptions->pszParms[0][0];
         if ('i' == ch)
            adslCfgParam |= kAdslCfgLineInnerPair;
         else if ('o' == ch)
            adslCfgParam |= kAdslCfgLineOuterPair;
         else 
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--trellis")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         adslCfgParam &= ~kAdslCfgTrellisMask;
         if (0 == strcmp (pOptions->pszParms[0], "on"))
            adslCfgParam |= kAdslCfgTrellisOn;
         else if (0 == strcmp (pOptions->pszParms[0], "off"))
            adslCfgParam |= kAdslCfgTrellisOff;
         else
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--snr")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         if ((pOptions->pszParms[0][0] == '0') && 
             ((pOptions->pszParms[0][1] == 'x') || (pOptions->pszParms[0][1] == 'X'))) {
            pAdslCfg->adslTrainingMarginQ4 = strtoul(pOptions->pszParms[0]+2, NULL, 16);
         }
         else
            pAdslCfg->adslTrainingMarginQ4 = strtol(pOptions->pszParms[0], NULL, 10);
         if (pAdslCfg->adslTrainingMarginQ4 <= 0)
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--bitswap")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         pAdslCfg->adslDemodCapMask |= kXdslBitSwapEnabled;
         if (0 == strcmp (pOptions->pszParms[0], "on"))
            pAdslCfg->adslDemodCapValue |= kXdslBitSwapEnabled;
         else if (0 == strcmp (pOptions->pszParms[0], "off"))
            pAdslCfg->adslDemodCapValue &= ~kXdslBitSwapEnabled;
         else
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--sesdrop")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         adslCfgParam |= kAdslCfgDemodCap2On;
         pAdslCfg->adslDemodCap2Mask |= kXdslRetrainOnSesEnabled;
         if (0 == strcmp (pOptions->pszParms[0], "on"))
            pAdslCfg->adslDemodCap2Value |= kXdslRetrainOnSesEnabled;
         else if (0 == strcmp (pOptions->pszParms[0], "off"))
            pAdslCfg->adslDemodCap2Value &= ~kXdslRetrainOnSesEnabled;
         else
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--CoMinMgn")) {
          if (1 != pOptions->nNumParms) {
              nRet = ADSLCTL_INVALID_PARAMETER;
              break;
          }
          adslCfgParam |= kAdslCfgDemodCap2On;
          pAdslCfg->adslDemodCap2Mask |= kXdslRetrainOnDslamMinMargin;
          if (0 == strcmp (pOptions->pszParms[0], "on"))
              pAdslCfg->adslDemodCap2Value |= kXdslRetrainOnDslamMinMargin;
          else if (0 == strcmp (pOptions->pszParms[0], "off"))
              pAdslCfg->adslDemodCap2Value &= ~kXdslRetrainOnDslamMinMargin;
          else
              nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--minINP")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         pAdslCfg->minINP = strtol(pOptions->pszParms[0], NULL, 10);
         if (pAdslCfg->minINP < 0)
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--maxDelay")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         pAdslCfg->maxDelay = strtol(pOptions->pszParms[0], NULL, 10);
         if (pAdslCfg->maxDelay == -1)
            pAdslCfg->maxDelay = 0;
         else if (pAdslCfg->maxDelay < 0)
            nRet = ADSLCTL_INVALID_PARAMETER;
         else
            pAdslCfg->maxDelay |= 0x80000000;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--sra")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         pAdslCfg->adslDemodCapMask |= kXdslSRAEnabled;
         if (0 == strcmp (pOptions->pszParms[0], "on"))
            pAdslCfg->adslDemodCapValue |= kXdslSRAEnabled;
         else if (0 == strcmp (pOptions->pszParms[0], "off"))
            pAdslCfg->adslDemodCapValue &= ~kXdslSRAEnabled;
         else
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--phycfg")) {
         if (pOptions->nNumParms < 2) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         /* parse demodulation capability  mask */
		 GetMaskValue(pAdslCfg->adslDemodCap, pOptions->pszParms[0], pOptions->pszParms[1]);
         /* parse handshaking switch timeout */
         if (pOptions->nNumParms >= 3)
            pAdslCfg->adslHsModeSwitchTime = strtoul(pOptions->pszParms[2], NULL, 10);
         /* parse demodulation capability 2  mask/value */
         if (pOptions->nNumParms >= 5) {
             adslCfgParam |= kAdslCfgDemodCap2On;
  			 GetMaskValue(pAdslCfg->adslDemodCap2, pOptions->pszParms[3], pOptions->pszParms[4]);
         }
         /* parse auxFeatures mask/value */
         if (pOptions->nNumParms >= 7)
  			GetMaskValue(pAdslCfg->xdslAuxFeatures, pOptions->pszParms[5], pOptions->pszParms[6]);
#ifdef CONFIG_VDSL_SUPPORTED
         /* parse vdslCfgFlags mask/value */
         if (pOptions->nNumParms >= 9)
  			GetMaskValue(pAdslCfg->vdslCfgFlags, pOptions->pszParms[7], pOptions->pszParms[8]);
#endif
         /* parse xdslCfg1 mask/value */
         if (pOptions->nNumParms >= 11)
  			GetMaskValue(pAdslCfg->xdslCfg1, pOptions->pszParms[9], pOptions->pszParms[10]);
         /* parse xdslCfg2 mask/value */
         if (pOptions->nNumParms >= 13)
  			GetMaskValue(pAdslCfg->xdslCfg2, pOptions->pszParms[11], pOptions->pszParms[12]);
         /* parse xdslCfg3 mask/value */
         if (pOptions->nNumParms >= 15)
  			GetMaskValue(pAdslCfg->xdslCfg3, pOptions->pszParms[13], pOptions->pszParms[14]);
         /* parse xdslCfg4 mask/value */
         if (pOptions->nNumParms >= 17)
  			GetMaskValue(pAdslCfg->xdslCfg4, pOptions->pszParms[15], pOptions->pszParms[16]);
      }
      else if (0 == strcmp(pOptions->pszOptName, "--mcfg")) {
         if (pOptions->nNumParms < 1) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         /* parse AnnexM PSD mask */
         if ((pOptions->pszParms[0][0] == '0') && 
             ((pOptions->pszParms[0][1] == 'x') || (pOptions->pszParms[0][1] == 'X'))) {
            i = strtoul(pOptions->pszParms[0]+2, NULL, 16);
         }
         else
            i = strtoul(pOptions->pszParms[0], NULL, 10);
         pAdslCfg->adsl2Param &= ~kAdsl2CfgAnnexMPsdMask;
         pAdslCfg->adsl2Param |= (i << kAdsl2CfgAnnexMPsdShift) & kAdsl2CfgAnnexMPsdMask;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--lcfg")) {
         if (pOptions->nNumParms < 1) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         /* parse AnnexL mask */
         if ((pOptions->pszParms[0][0] == '0') && 
             ((pOptions->pszParms[0][1] == 'x') || (pOptions->pszParms[0][1] == 'X'))) {
            i = strtoul(pOptions->pszParms[0]+2, NULL, 16);
         }
         else
            i = strtoul(pOptions->pszParms[0], NULL, 10);
         pAdslCfg->adsl2Param &= ~kAdsl2CfgAnnexLMask;
         pAdslCfg->adsl2Param |= (i << kAdsl2CfgAnnexLShift) & kAdsl2CfgAnnexLMask;
      }
#ifdef ANNEX_C
      else if (0 == strcmp(pOptions->pszOptName, "--bm")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         adslCfgParam &= ~kAdslCfgBitmapMask;
         ch = tolower(pOptions->pszParms[0][0]);
         if ('d' == ch)
            adslCfgParam |= kAdslCfgDBM;
         else if ('f' == ch)
            adslCfgParam |= kAdslCfgFBM;
         else 
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--ccw")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         adslCfgParam |= kAdslCfgCentilliumCRCWorkAroundEnabled;
      }
#endif
      else if (0 == strcmp(pOptions->pszOptName, "--up")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         *pUp = 1;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--i24k")) {
          if (1 != pOptions->nNumParms) {
              nRet = ADSLCTL_INVALID_PARAMETER;
              break;
          }
          adslCfgParam |= kAdslCfgDemodCap2On;
          pAdslCfg->adslDemodCap2Mask |= 0x00100000;
          if (0 == strcmp (pOptions->pszParms[0], "on"))
              pAdslCfg->adslDemodCap2Value |= 0x00100000;
          else if (0 == strcmp (pOptions->pszParms[0], "off"))
              pAdslCfg->adslDemodCap2Value &= ~0x00100000;
          else
              nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--phyReXmt")) {
          if (1 != pOptions->nNumParms) {
              nRet = ADSLCTL_INVALID_PARAMETER;
              break;
          }
          if ((pOptions->pszParms[0][0] == '0') &&
              ((pOptions->pszParms[0][1] == 'x') || (pOptions->pszParms[0][1] == 'X')))
              i = strtoul(pOptions->pszParms[0]+2, NULL, 16);
          else
             i = strtoul(pOptions->pszParms[0], NULL, 10);
          if (i > 0x3) {
             nRet = ADSLCTL_INVALID_PARAMETER;
             break;
          }
          adslCfgParam |= kAdslCfgDemodCap2On;
          pAdslCfg->adslDemodCap2Mask |= (kXdslFireUsSupported|kXdslFireDsSupported);
          if (i & 0x1)
              pAdslCfg->adslDemodCap2Value |= kXdslFireDsSupported;
          else
              pAdslCfg->adslDemodCap2Value &= ~kXdslFireDsSupported;
          if (i & 0x2)
              pAdslCfg->adslDemodCap2Value |= kXdslFireUsSupported;
          else
            pAdslCfg->adslDemodCap2Value &= ~kXdslFireUsSupported;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--Ginp")) {
          if (1 != pOptions->nNumParms) {
              nRet = ADSLCTL_INVALID_PARAMETER;
              break;
          }
          if ((pOptions->pszParms[0][0] == '0') &&
              ((pOptions->pszParms[0][1] == 'x') || (pOptions->pszParms[0][1] == 'X')))
              i = strtoul(pOptions->pszParms[0]+2, NULL, 16);
          else
             i = strtoul(pOptions->pszParms[0], NULL, 10);
          if (i > 0x3) {
             nRet = ADSLCTL_INVALID_PARAMETER;
             break;
          }
          pAdslCfg->xdslAuxFeaturesMask |= (kXdslGinpDsSupported | kXdslGinpUsSupported );
          if (i & 0x1)
              pAdslCfg->xdslAuxFeaturesValue |= kXdslGinpDsSupported;
          else
              pAdslCfg->xdslAuxFeaturesValue &= ~kXdslGinpDsSupported;
          if (i & 0x2)
              pAdslCfg->xdslAuxFeaturesValue |= kXdslGinpUsSupported;
          else
              pAdslCfg->xdslAuxFeaturesValue &= ~kXdslGinpUsSupported;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--TpsTc")) {
          if (1 != pOptions->nNumParms) {
              nRet = ADSLCTL_INVALID_PARAMETER;
              break;
          }
         if ((pOptions->pszParms[0][0] == '0') && 
            ((pOptions->pszParms[0][1] == 'x') || (pOptions->pszParms[0][1] == 'X')))
            i = strtoul(pOptions->pszParms[0]+2, NULL, 16);
         else
            i = strtoul(pOptions->pszParms[0], NULL, 10);
         
         if (i < 0x20) {
            adslCfgParam &= ~(0xF << kAdslCfgTpsTcShift);
            adslCfgParam |= (i << kAdslCfgTpsTcShift);
         }
         else
            nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if (0 == strcmp(pOptions->pszOptName, "--monitorTone")) {
          if (1 != pOptions->nNumParms) {
              nRet = ADSLCTL_INVALID_PARAMETER;
              break;
          }
          pAdslCfg->xdslAuxFeaturesMask |= kXdslMonitorToneDisable;       /* kDslMonitorToneFeatureDisable */
          if (0 == strcmp (pOptions->pszParms[0], "on"))
              pAdslCfg->xdslAuxFeaturesValue &= ~kXdslMonitorToneDisable;
          else if (0 == strcmp (pOptions->pszParms[0], "off"))
              pAdslCfg->xdslAuxFeaturesValue |= kXdslMonitorToneDisable;
          else
              nRet = ADSLCTL_INVALID_PARAMETER;
      }
      else if(kAdslTypeAnnexB == adslVer.phyType) {   /* Options for AnnexB PHY */
         if (0 == strcmp(pOptions->pszOptName, "--forceJ43")) {
             if (1 != pOptions->nNumParms) {
                 nRet = ADSLCTL_INVALID_PARAMETER;
                 break;
             }
             adslCfgParam |= kAdslCfgDemodCap2On;
             pAdslCfg->adslDemodCap2Mask |= (1 << 9);       /* Re-use kDslAnnexMcustomMode */
             if (0 == strcmp (pOptions->pszParms[0], "on"))
                 pAdslCfg->adslDemodCap2Value |= (1 << 9);
             else if (0 == strcmp (pOptions->pszParms[0], "off"))
                 pAdslCfg->adslDemodCap2Value &= ~(1 << 9);
             else
                 nRet = ADSLCTL_INVALID_PARAMETER;
         }
         else if (0 == strcmp(pOptions->pszOptName, "--toggleJ43B43")) {
             if (1 != pOptions->nNumParms) {
                 nRet = ADSLCTL_INVALID_PARAMETER;
                 break;
             }
             pAdslCfg->xdslAuxFeaturesMask |= 0x00000400;       /* kDslAnnexJhandshakeB43J43Toggle */
             if (0 == strcmp (pOptions->pszParms[0], "on"))
                 pAdslCfg->xdslAuxFeaturesValue |= 0x00000400;
             else if (0 == strcmp (pOptions->pszParms[0], "off"))
                 pAdslCfg->xdslAuxFeaturesValue &= ~0x00000400;
             else
                 nRet = ADSLCTL_INVALID_PARAMETER;
         }
         else
            nRet = ADSLCTL_INVALID_OPTION;
      }
      else
          nRet = ADSLCTL_INVALID_OPTION;
   }

   if (ADSLCTL_INVALID_PARAMETER == nRet) {
      fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName, pOptions->pszOptName);
      return nRet;
   }
   if (ADSLCTL_INVALID_OPTION == nRet) {
      fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);
      return nRet;
   }

#ifdef ANNEX_C
   pAdslCfg->adslAnnexCParam = adslCfgParam;
#else
   pAdslCfg->adslAnnexAParam = adslCfgParam;
#endif

   return nRet;
}

static int GetAdslCfg(unsigned char lineId, adslCfgProfile *pAdslCfg)
{
	int	nRet;
	long	dataLen;
	char	oidStr[] = { 95 };		/* kOidAdslPhyCfg */

	dataLen = sizeof(adslCfgProfile);
	nRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)pAdslCfg, &dataLen);
	
	if( nRet != BCMADSL_STATUS_SUCCESS) {
		fprintf( stderr, "%s: xdslCtl_GetObjectValue error\n", g_szPgmName );
	}

	return nRet;
}

/***************************************************************************
 * Function Name: StartHandler
 * Description  : Processes the adslctl start command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int StartHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
#if defined(PLATFORM6352)
    int linkUp = 1;
    int nRet = (int) BcmAdsl_Check();
    cmsRet cmsRet;
    if( nRet == BCMADSL_STATUS_SUCCESS )
        nRet = (int) BcmAdsl_Initialize( NULL, 0 );
    cmsRet=nRet;
#else
   int xdslMiscCfgParam, tmpMask, tmpValue;
   adslCfgProfile adslCfg;
   int linkUp, nRet = BCMADSL_STATUS_SUCCESS;
   CmsRet cmsRet = CMSRET_INTERNAL_ERROR;
   
   nRet = GetAdslCfg(lineId, &adslCfg);
   if (nRet == BCMADSL_STATUS_SUCCESS) {
      xdslMiscCfgParam = adslCfg.xdslMiscCfgParam;
      tmpMask = adslCfg.xdslCfg3Mask & (1 << 15);
      tmpValue = adslCfg.xdslCfg3Value & (1 << 15);
      memset((void *)&adslCfg, 0, sizeof(adslCfg));
      adslCfg.xdslMiscCfgParam = xdslMiscCfgParam;
      adslCfg.xdslCfg3Mask = tmpMask;
      adslCfg.xdslCfg3Value = tmpValue;
   }
   else
      memset((void *)&adslCfg, 0, sizeof(adslCfg));
   
   nRet = ParseCfgParam (pOptions, nNumOptions, 1, &adslCfg, &linkUp);
   if (nRet == BCMADSL_STATUS_SUCCESS) 
   {
      cmsRet = xdslCtl_Initialize(lineId, NULL, 0, &adslCfg );
   }
#endif
   if(cmsRet == CMSRET_SUCCESS)
   {
      if (linkUp) 
      {
         cmsRet = xdslCtl_ConnectionStart(lineId);
      }
      if(cmsRet != CMSRET_SUCCESS)
      {
         fprintf( stderr, "%s->%s: devCtl_adslConnectionStart error code %d\n", g_szPgmName, __FUNCTION__, cmsRet);
      }
    }
    else
        fprintf( stderr, "%s: devCtl_adslInitialize error\n", g_szPgmName );

   if (cmsRet != CMSRET_SUCCESS)
      nRet = ADSLCTL_GENERAL_ERROR;

   return( nRet );
} /* StartHandler */

/***************************************************************************
 * Function Name: ConfigureHandler
 * Description  : Processes the adslctl configure command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int ConfigureHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
   adslCfgProfile adslCfg;
   SINT32 xdslMiscCfgParam, tmpMask, tmpValue;
   int linkUp, nRet = BCMADSL_STATUS_SUCCESS;
   CmsRet cmsRet = CMSRET_INTERNAL_ERROR;

   nRet = GetAdslCfg(lineId, &adslCfg);
   if (nRet != BCMADSL_STATUS_SUCCESS)
      return nRet;
   
   xdslMiscCfgParam = adslCfg.xdslMiscCfgParam;
   tmpMask = adslCfg.xdslCfg3Mask & (1 << 15);
   tmpValue = adslCfg.xdslCfg3Value & (1 << 15);
   memset((void *)&adslCfg, 0, sizeof(adslCfg));
   adslCfg.xdslMiscCfgParam = xdslMiscCfgParam;
   adslCfg.xdslCfg3Mask = tmpMask;
   adslCfg.xdslCfg3Value = tmpValue;
   
   nRet = ParseCfgParam (pOptions, nNumOptions, 1, &adslCfg, &linkUp);
   if (nRet == BCMADSL_STATUS_SUCCESS) {
      cmsRet = xdslCtl_Configure(lineId, &adslCfg);
      if (cmsRet != CMSRET_SUCCESS) {
         fprintf (stderr, "%s: devCtl_adslConfigure error\n", g_szPgmName);
         nRet = ADSLCTL_GENERAL_ERROR;
      }
   }
   
   return( nRet );
} /* ConfigureHandler */

/***************************************************************************
 * Function Name: ConfigureHandler1
 * Description  : Processes the adslctl configure command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int ConfigureHandler1(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
   adslCfgProfile adslCfg;
   int linkUp, nRet = BCMADSL_STATUS_SUCCESS;
   CmsRet cmsRet = CMSRET_INTERNAL_ERROR;
   
   nRet = GetAdslCfg(lineId, &adslCfg);
   if (nRet != BCMADSL_STATUS_SUCCESS)
      return nRet;

   nRet = ParseCfgParam (pOptions, nNumOptions, 0, &adslCfg, &linkUp);
   if (nRet == BCMADSL_STATUS_SUCCESS) {
      cmsRet = xdslCtl_Configure(lineId, &adslCfg);
      if (cmsRet != CMSRET_SUCCESS) {
         fprintf (stderr, "%s: devCtl_adslConfigure error\n", g_szPgmName);
         nRet = ADSLCTL_GENERAL_ERROR;
      }
   }
   
   return( nRet );
} /* ConfigureHandler1 */

/***************************************************************************
 * Function Name: ParseToneMap
 * Description  : Parses xmt or rcv tone maps of --tones command
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/

static int GetMapNibble(char *p)
{
   int n = *p;

   if ((n >= '0') && (n <= '9'))
      return n - '0';
   if ((n >= 'A') && (n <= 'F'))
      return n - 'A' + 10;
   if ((n >= 'a') && (n <= 'f'))
      return n - 'a' + 10;
   return -1;
}

static int GetMapByte(char *p)
{
   int n1, n2;

   n1 = GetMapNibble(p);
   if (n1 < 0)
      return ADSLCTL_INVALID_PARAMETER;
   n2 = GetMapNibble(p+1);
   if (n2 < 0)
      return ADSLCTL_INVALID_PARAMETER;

   return (n1 << 4) + n2;
}


/***************************************************************************
 * Function Name: ConnectionHandler
 * Description  : Processes the adslctl connection command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int ConnectionHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
   int nRet = 0;
   CmsRet cmsRet = CMSRET_INTERNAL_ERROR;

   pOptions--;
   while ((0 == nRet) && (nNumOptions-- > 0)) {
      pOptions++;
      if (0 == strcmp(pOptions->pszOptName, "--up")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_ConnectionStart(lineId);
         if( cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s->%s: devCtl_adslConnectionStart error\n", g_szPgmName, __FUNCTION__ );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--down")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_ConnectionStop(lineId);
         if( cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslConnectionStop error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--loopback")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         cmsRet = xdslCtl_ConnectionStop(lineId);
         if( cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslConnectionStop error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break; 
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--reverb")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_SetTestMode(lineId, ADSL_TEST_REVERB);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslSetTestMode error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break; 
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--medley")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_SetTestMode(lineId, ADSL_TEST_MEDLEY);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslSetTestMode error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--noretrain")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         
         cmsRet = xdslCtl_SetTestMode(lineId, ADSL_TEST_NO_AUTO_RETRAIN);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslSetTestMode error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--tones")) {
		int		xmtStart=0, xmtNum=32, rcvStart=32,rcvNum=480;
		char    ToneMap[512/8];
		if (1 != pOptions->nNumParms) {
			nRet = ADSLCTL_INVALID_PARAMETER;
			break;
		}
		GetToneMap(pOptions->pszParms[0], ToneMap, 512/8);

		cmsRet = xdslCtl_SelectTones(lineId, xmtStart,xmtNum,rcvStart,rcvNum, ToneMap, ToneMap+4);
		if( cmsRet != CMSRET_SUCCESS) {
			fprintf( stderr, "%s: BcmAdsl_DiagProcessCommandFrame error\n",g_szPgmName );
			return ADSLCTL_GENERAL_ERROR;
		}
		cmsRet = xdslCtl_SetTestMode(lineId, ADSL_TEST_NORMAL);
		if (cmsRet != CMSRET_SUCCESS) {
			fprintf( stderr, "%s: devCtl_adslSetTestMode error\n", g_szPgmName );
			nRet = ADSLCTL_GENERAL_ERROR;
			break;
		}
      }
      else if (0 == strcmp(pOptions->pszOptName, "--L3")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_SetTestMode(lineId, ADSL_TEST_L3);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslSetTestMode error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--L0")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_SetTestMode(lineId, ADSL_TEST_L0);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslSetTestMode error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--normal")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_SetTestMode(lineId,ADSL_TEST_NORMAL);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslSetTestMode error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--freezeReverb")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_SetTestMode(lineId,ADSL_TEST_FREEZE_REVERB);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslSetTestMode error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--freezeMedley")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_SetTestMode(lineId,ADSL_TEST_FREEZE_MEDLEY);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslSetTestMode error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--diagmode")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_SetTestMode(lineId, ADSL_TEST_DIAGMODE);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslSetTestMode error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else
         nRet = ADSLCTL_INVALID_OPTION;

   } /* while loop */

   if (ADSLCTL_INVALID_PARAMETER == nRet) {
      fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName,
              pOptions->pszOptName);
      return nRet;
   }
   if (ADSLCTL_INVALID_OPTION == nRet) {
      fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);
      return nRet;
   }
   
   return nRet;
} /* ConnectionHandler */

/***************************************************************************
 * Function Name: StopHandler
 * Description  : Processes the adslctl stop command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int StopHandler(unsigned char lineId ATTRIBUTE_UNUSED, POPTION_INFO pOptions ATTRIBUTE_UNUSED, int nNumOptions ATTRIBUTE_UNUSED)
{
   CmsRet cmsRet = CMSRET_INTERNAL_ERROR;
   int nRet = BCMADSL_STATUS_SUCCESS;

   cmsRet = xdslCtl_ConnectionStop(0);

   if (cmsRet != CMSRET_SUCCESS)
   {
      fprintf( stderr, "%s: devCtl_adslConnectionStop error\n", g_szPgmName );
      nRet = ADSLCTL_GENERAL_ERROR;
   }
#ifdef SUPPORT_DSL_BONDING
   cmsRet = xdslCtl_ConnectionStop(1);
   if (cmsRet != CMSRET_SUCCESS)
   {
      fprintf( stderr, "%s: xdslCtl_ConnectionStop(1) error\n", g_szPgmName );
      nRet = ADSLCTL_GENERAL_ERROR;
   }
#endif

   cmsRet = xdslCtl_Uninitialize(0);
   if (cmsRet != CMSRET_SUCCESS) 
   {
      fprintf( stderr, "%s: devCtl_adslUninitialize error\n", g_szPgmName );
      nRet = ADSLCTL_GENERAL_ERROR;
   }
   return( nRet );
} /* StopHandler */

/***************************************************************************
 * Function Name: BertHandler
 * Description  : Processes the adslctl bert command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int BertHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
   int nRet = BCMADSL_STATUS_SUCCESS;;
   CmsRet cmsRet;

   pOptions--;
   while ((BCMADSL_STATUS_SUCCESS == nRet) && (nNumOptions-- > 0)) {
      pOptions++;
      if (0 == strcmp(pOptions->pszOptName, "--start")) {
         SINT32  nSec;

         if ((1 != pOptions->nNumParms) ||
             ((nSec = strtol (pOptions->pszParms[0], NULL, 10)) <= 0)) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_BertStartEx(lineId, nSec);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslBertStartEx error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--stop")) {
         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_BertStopEx(lineId);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslBertStopEx error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }
      }
      else if (0 == strcmp(pOptions->pszOptName, "--show")) {
         adslMibInfo    adslMib;
         long size = sizeof(adslMib);

         if (0 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }

         cmsRet = xdslCtl_GetObjectValue(lineId, NULL, 0, (char *)&adslMib, &size);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName );
            nRet = ADSLCTL_GENERAL_ERROR;
            break;
         }

         printf ("%s: BERT results:\n"
                 "BERT Status = %s\n"
                 "BERT Total Time   = %u sec\n"
                 "BERT Elapsed Time = %u sec\n" 
                 "BERT Bits Tested = 0x%08X%08X bits\n"
                 "BERT Err Bits = 0x%08X%08X bits\n", g_szPgmName,
                 adslMib.adslBertStatus.bertSecCur != 0 ? "RUNNING" : "NOT RUNNING",
                 adslMib.adslBertStatus.bertSecTotal, adslMib.adslBertStatus.bertSecElapsed,
                 adslMib.adslBertStatus.bertTotalBits.cntHi, adslMib.adslBertStatus.bertTotalBits.cntLo,
                 adslMib.adslBertStatus.bertErrBits.cntHi, adslMib.adslBertStatus.bertErrBits.cntLo);
      }
      else
         nRet = ADSLCTL_INVALID_OPTION;
   }
   
   if (ADSLCTL_INVALID_PARAMETER == nRet) {
      fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName,
              pOptions->pszOptName);
      return nRet;
   }
   if (ADSLCTL_INVALID_OPTION == nRet) {
      fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);
      return nRet;
   }

   return nRet;
}

/***************************************************************************
 * 
 *    Functions for  InfoHandler
 * 
 ***************************************************************************/

#define abs(n)    ( ((n) ^ ((n) >> 31)) - ((n) >> 31) )
#define DEC_POINT(n)    (n) < 0 ? '-' : ' ', (int)(abs(n) / 10), (int)(abs(n) % 10)

#define	kTwoThirdQ15			((SINT32)21845)		/* 2/3 in Q15 */
#define	kFourFifthQ15			((SINT32)26214)		/* 4/5 in Q15 */
#define	kTenLog10TwoQ12			((SINT32)12330)		/* 10*log10(2) in Q12 */	
#define	kTenLog10EQ12			((SINT32)17789)		/* 10*log10(e) in Q12 */

#define	kMaxQ4dBInput			915
#define	kLog2TenDivTenQ16		((SINT32)21771)		/* log2(10)/10 in Q16 */
#define	kLn2Q15					((SINT32)22713)		/* ln(2) in Q15 */
#define	kOneThirdQ15			((SINT32)10923)		/* 1/3 in Q15 */
#define	kOneQ31					((UINT32)1 << 31)	/* 1.0 in Q31 */
#define klog4096Q4				((short)579)			/*16*log10(2^12)*/


static char * GetStateStatusStr(int status)
{
   char *pBuf = (char *)&scratchBuf[0];
   pBuf[0] = '\0';
   if(kAdslPhysStatusNoDefect == status)
      strcpy(pBuf, "No Defect");
   else {
      if (kAdslPhysStatusLOM & status)
        strcat(pBuf, "Loss Of Margin  ");
      if (kAdslPhysStatusLOF & status)
        strcat(pBuf, "Loss Of Frame  ");
      if (kAdslPhysStatusLOS & status)
        strcat(pBuf, "Loss Of Signal  ");
      if (kAdslPhysStatusLOSQ & status)
        strcat(pBuf, "Loss Of Signal Quality  ");
      if (kAdslPhysStatusLPR & status)
        strcat(pBuf, "Loss Of Power  ");
      if(0 == strlen(pBuf))
         strcpy(pBuf, "Unknown");
   }
   return pBuf;
}

static char * GetTrainingStatusStr(adslMibInfo *pAdslMib)
{
   switch (pAdslMib->adslTrainingState)
   {
   case kAdslTrainingIdle:
      return "Idle";
   case kAdslTrainingG994:
      return "G.994 Training";
   case kAdslTrainingG992Started:
      return "G.992 Started";
   case kAdslTrainingG992ChanAnalysis:
      return "G.992 Channel Analysis";
   case kAdslTrainingG992Exchange:
      return "G.992 Message Exchange";
   case kAdslTrainingG993Started:
      return "G.993 Started";
   case kAdslTrainingG993ChanAnalysis:
      return "G.993 Channel Analysis";
   case kAdslTrainingG993Exchange:
      return "G.993 Message Exchange";
   case kAdslTrainingConnected:
      if(!pAdslMib->fastRetrainActive)
          return "Showtime";
      else
          return "Showtime - fastRetrainActive";
   default:
      return "Unknown";
   }
}

static char * GetModulationStr(int modType)
{
   switch (modType)
   {
   case kAdslModGdmt:
      return "G.DMT";
   case kAdslModT1413:
      return "T1.413";
   case kAdslModGlite:
      return "G.lite";
   case kAdslModAnnexI:
      return "AnnexI";
   case kAdslModAdsl2:
      return "ADSL2";
   case kAdslModAdsl2p:
      return "ADSL2+";
   case kAdslModReAdsl2:
      return "RE-ADSL2";
   case kVdslModVdsl2:
      return "VDSL2";
   case kXdslModGfast:
      return "G.fast";
   default:
      return "Unknown";
   }
}

static unsigned char IsXdsl2(int modType)
{
	unsigned char res = 0;
	
	switch (modType) {
		case kAdslModAdsl2:
		case kAdslModAdsl2p:
		case kAdslModReAdsl2:
		case kVdslModVdsl2:
		case kXdslModGfast:
			res = 1;
			break;
		default:
			break;
	}
	return res;
}

static unsigned char IsVdsl2OrGfastConnection(int modType)
{
	return ((kVdslModVdsl2 == modType) || (kXdslModGfast == modType));
}

static char *GetAnnexTypeStr(int annexType)
{
	char *res = "";
	
	switch(annexType) {
		case kAdslTypeAnnexA:
			res ="Annex A";
			break;
		case kAdslTypeAnnexB:
			res ="Annex B";
			break;
		case kAdslTypeAnnexC:
			res ="Annex C";
			break;
		case kAdslTypeSADSL:
			res ="Annex SADSL";
			break;
		case kAdslTypeAnnexI:
			res ="Annex I";
			break;
		case kAdslTypeAnnexAB:
			res ="Annex AB";
			break;
		case kAdslTypeAnnexL:
			res ="Annex L";
			break;
	}
	
	return res;
}

#ifdef CONFIG_VDSL_SUPPORTED
static char * GetSelectedProfileStr(unsigned short vdsl2Profile)
{
	char *res = "Unknown";
	
	switch(vdsl2Profile) {
		case kVdslProfile8a:
			res = "Profile 8a";
			break;
		case kVdslProfile8b:
			res = "Profile 8b";
			break;
		case kVdslProfile8c:
			res = "Profile 8c";
			break;
		case kVdslProfile8d:
			res = "Profile 8d";
			break;
		case kVdslProfile12a:
			res = "Profile 12a";
			break;
		case kVdslProfile12b:
			res = "Profile 12b";
			break;
		case kVdslProfile17a:
			res = "Profile 17a";
			break;
		case kVdslProfile30a:
			res = "Profile 30a";
			break;
#ifdef CONFIG_VDSLBRCMPRIV1_SUPPORT
		case kVdslProfile35b:
			res = "Profile 35b";
			break;
#ifdef CONFIG_VDSLBRCMPRIV2_SUPPORT
		case kVdslProfileBrcmPriv2:
			res = "Profile BrcmPriv2";
			break;
#endif
#endif
#ifdef SUPPORT_DSL_GFAST
		case kGfastProfile106a:
			res = "Profile Gfast 106a";
			break;
		case kGfastProfile212a:
			res = "Profile Gfast 212a";
			break;
		case kGfastProfile106b:
			res = "Profile Gfast 106b";
			break;
		case kGfastProfile106c:
			res = "Profile Gfast 106c";
			break;
		case kGfastProfile212c:
			res = "Profile Gfast 212c";
			break;
#endif
	}
	return res;
}

static char * GetVdsl2LrStr(unsigned char vdsl2LR)
{
	char *res = "Off";
	
	switch(vdsl2LR) {
		case kVdslLRShortLoop:
			res = "short loop";
			break;
		case kVdslLRMediumLoop:
			res = "medium loop";
			break;
		case kVdslLRLongLoop:
			res = "long loop";
			break;
	}
	return res;
}

#endif

static char *GetConnectionTypeStr(unsigned char tmType)
{
	return ( kXdslDataPtm == tmType ) ? "PTM Mode":
			((kXdslDataAtm == tmType) || (kXdslDataNitro == tmType)) ? "ATM Mode":
			(kXdslDataRaw == tmType) ? "RAW Data Mode": "Not connected";
}

static char * GetModulationStr2(int modType, int xdslType)
{
   static char annexMStr[] = "AnnexM EU-  ";
   CmsRet cmsRet;
   
#ifdef CONFIG_VDSL_SUPPORTED
    if(kVdslModVdsl2 == modType)
        return GetAnnexTypeStr(xdslType >> kXdslModeAnnexShift);
#endif

   if (xdslType & kAdsl2ModeAnnexMask)
   {
      adslVersionInfo adslVer;
      int mVal = 32 + (((xdslType & kAdsl2ModeAnnexMask) - 1) << 2);
      cmsRet = xdslCtl_GetVersion(0, &adslVer);
      if( cmsRet == CMSRET_SUCCESS)
         annexMStr[5] = (kAdslTypeAnnexB == adslVer.phyType) ? 'J': 'M';
      annexMStr[sizeof(annexMStr)-3] = '0' + mVal/10;
      annexMStr[sizeof(annexMStr)-2] = '0' + mVal%10;
      return annexMStr;
   }
   else
      return GetAnnexTypeStr(xdslType >> kXdslModeAnnexShift);
}

static char * PerfTime2String(char *tmStr, UINT32 tm)
{
   int n;
   char *p = tmStr;

   *p = 0;
   if (tm > (3600*24)) {
      n = tm / (3600*24);
      p += sprintf (p, "%d days ", n);
      tm -= n * (3600*24);
   }
   if (tm > 3600) {
      n = tm / 3600;
      p += sprintf (p, "%d hours ", n);
      tm -= n * 3600;
   }
   if (tm > 60) {
      n = tm / 60;
      p += sprintf (p, "%d min ", n);
      tm -= n * 60;
   }
   p += sprintf (p, "%d sec", tm);
   return tmStr;
}

static void PrintPerformanceData (
	char *hdr, 
	UINT32 tm, 
	adslPerfCounters *pPerfCnt, 
	adslPerfCounters *pTxPerfCnt, 
	adslChanCounters *pChanPerfCnt,
	adslFailureCounters *pFailureCounters)
{
   char  tmStr[80];

   printf ("%s time = %s\n", hdr, PerfTime2String(tmStr, tm));
   printf ("FEC:\t\t%u\t\t%u\n", pChanPerfCnt->adslChanCorrectedBlks, pChanPerfCnt->adslChanTxFEC);
   printf ("CRC:\t\t%u\t\t%u\n", pChanPerfCnt->adslChanUncorrectBlks, pChanPerfCnt->adslChanTxCRC);
   printf ("ES:\t\t%u\t\t%u\n",  pPerfCnt->adslESs, pTxPerfCnt->adslESs);
   printf ("SES:\t\t%u\t\t%u\n", pPerfCnt->adslSES, pTxPerfCnt->adslSES);
   printf ("UAS:\t\t%u\t\t%u\n", pPerfCnt->adslUAS, pTxPerfCnt->adslUAS);
   printf ("LOS:\t\t%u\t\t%u\n", pPerfCnt->adslLoss, pTxPerfCnt->adslLOSS);
   printf ("LOF:\t\t%u\t\t%u\n", pPerfCnt->adslLofs, pTxPerfCnt->adslLOSS);
   printf ("LOM:\t\t%u\t\t%u\n", pPerfCnt->xdslLoms, pTxPerfCnt->adslLOMS);
   if(NULL != pFailureCounters) {
      printf ("Retr:\t\t%u\n", pFailureCounters->adslRetr);
#ifdef SUPPORT_DSL_GFAST
      printf ("FastRetr:\t%u\n", pFailureCounters->xdslFastRetr);
#endif
      printf ("FailedRetr:\t%u\n", pFailureCounters->adslInitErr);
      printf ("FailedFastRetr:\t%u\n", pFailureCounters->xdslFastInitErr);
   }
   else {
      printf ("Retr:\t\tN/A\n");
      printf ("HostInitRetr:\tN/A\n");
#ifdef SUPPORT_DSL_GFAST
      printf ("FastRetr:\tN/A\n");
#endif
      printf ("FailedRetr:\tN/A\n");
#ifdef SUPPORT_DSL_GFAST
      printf ("FailedFastRetr:\tN/A\n");
#endif
   }
}

static void PrintToneData(unsigned char lineId, char *hdr, int id)
{
   int i;
   CmsRet cmsRet;
   char  oidStr[] = { kOidAdslPrivate, 0 };
   short *snr = &scratchBuf[0];
   long  snrLen = sizeof(scratchBuf);

   oidStr[1] = id;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)snr, &snrLen);
   if (cmsRet != CMSRET_SUCCESS) {
      fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName );
      return;
   }
   snrLen >>= 1;
   printf("Tone number      %s\n", hdr);
   for (i = 0; i < snrLen; i++)
      printf("   %d\t\t%s\n", i, QnToString(snr[i],4));
}        

#ifdef SUPPORT_SELT
static void PrintToneData4(unsigned char lineId, char *hdr, int id)
{
   int i;
   CmsRet cmsRet;
   char  oidStr[] = { kOidAdslPrivate, 0 };
   unsigned short *val = (unsigned short *)&scratchBuf[0];
   long  snrLen = sizeof(scratchBuf);

   oidStr[1] = id;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)val, &snrLen);
   if (cmsRet != CMSRET_SUCCESS) {
      fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName );
      return;
   }
   snrLen >>= 1;
   printf("Tone number      %s\n", hdr);
   for (i = 0; i < snrLen; i++)
      printf("   %d\t\t%d\n", i, val[i]);
}
#endif

static void PrintToneData1(unsigned char lineId, char *hdr, int id)
{
   int i;
   CmsRet cmsRet;
   char  oidStr[] = { kOidAdslPrivate, 0 };
   char  *data = (char *)&scratchBuf[0];
   long  dataLen = sizeof(scratchBuf);

   oidStr[1] = id;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
   if (cmsRet != CMSRET_SUCCESS) {
      fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName );
      return;
   }
   printf("Tone number      %s\n", hdr);
   for (i = 0; i < dataLen; i++)
      printf("   %d\t\t%u\n", i, data[i]);
}

#ifdef CONFIG_VDSL_SUPPORTED
static void PrintToneGroupObjects(unsigned char lineId)
{
   int i;
   CmsRet cmsRet;
   char  oidStr[] = { kOidAdslPrivate, 0 };
   char oidStr1[] = { kOidAdslPrivate, kOidAdslPrivExtraInfo, kOidAdslPrivSetFlagActualGFactor };

   long  dataLen = kVdslMibToneNum;
   unsigned char dataBuf1[2];
   char  *data = (char *)&scratchBuf[0];
   dataBuf1[0]=1;
   dataLen=1;

   oidStr[1]=kOidAdslPrivSATNdsperband;
   dataLen=5*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
   printf("SATNds dataLen %ld \n",dataLen);
   for (i=0;i<5*2; i++)
    printf("SATNds[%d] %x \n",i,data[i]);
    
   oidStr[1]=kOidAdslPrivSATNusperband;
   dataLen=5*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
   printf("SATNus dataLen %ld \n",dataLen);
   for (i=0;i<5*2; i++)
    printf("SATNus[%d] %x \n",i,data[i]);
    
   oidStr[1]=kOidAdslPrivLATNdsperband;
   dataLen=5*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
   printf("LATNds dataLen %ld \n",dataLen);
   for (i=0;i<5*2 ;i++)
    printf("LATNds[%d] %x\n",i,data[i]);
    
   oidStr[1]=kOidAdslPrivLATNusperband;
   dataLen=5*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
   printf("LATNus dataLen %ld \n",dataLen);
   for (i=0;i<5*2 ;i++)
   printf("LATNus[%d] %x\n",i,data[i]);
   
   printf("Setting FlagActualGFactor dataBuf1 %x\n",dataBuf1[0]);
   oidStr[1]=kOidAdslPrivQuietLineNoiseDsPerToneGroup;
   dataLen=1;
   cmsRet=xdslCtl_SetObjectValue(lineId, oidStr1,3,(char*)dataBuf1, &dataLen);
   dataLen=512*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);

   printf("Tone number QLNds dataLen=%ld\n",dataLen);
   for (i = 0; i < dataLen; i+=8)
      printf("   %d\t%x %x %x %x %x %x %x %x\n", i, data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7]);

      
   oidStr[1]=kOidAdslPrivSNRDsPerToneGroup;
   dataLen=1;
   cmsRet=xdslCtl_SetObjectValue(lineId, oidStr1,3,(char*)dataBuf1, &dataLen);
   dataLen=512*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);

   printf("Tone number SNRds dataLen=%ld\n",dataLen);
   for (i = 0; i < dataLen; i+=8)
      printf("   %d\t%x %x %x %x %x %x %x %x\n", i, data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7]);

   dataLen=1;
   cmsRet=xdslCtl_SetObjectValue(lineId, oidStr1,3,(char*)dataBuf1, &dataLen);
   oidStr[1]=kOidAdslPrivChanCharLogDsPerToneGroup;
   dataLen=512*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);

   printf("Tone number HlogDS dataLen=%ld\n",dataLen);
   for (i = 0; i < dataLen; i+=8)
      printf("   %d\t%x %x %x %x %x %x %x %x\n", i, data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7]);

   oidStr[1]=kOidAdslPrivBitAllocDsPerToneGroup;
   dataLen=1;
   cmsRet=xdslCtl_SetObjectValue(lineId, oidStr1,3,(char*)dataBuf1,&dataLen);
   dataLen=512*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);

   printf("Tone number Bit Alloc DS dataLen=%ld\n",dataLen);
   for (i = 0; i < dataLen; i+=8)
      printf("   %d\t%x %x %x %x %x %x %x %x\n", i, data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7]);
   dataLen=1;
   cmsRet=xdslCtl_SetObjectValue(lineId, oidStr1,3,(char*)dataBuf1,&dataLen);
   oidStr[1]=kOidAdslPrivGainDsPerToneGroup;
   dataLen=512*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
   
   printf("Tone number Gain Alloc DS dataLEn=%ld\n",dataLen);
   for (i = 0; i < dataLen; i+=8)
       printf("   %d\t%x %x %x %x %x %x %x %x\n", i, data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7]);

   oidStr[1]=kOidAdslPrivQuietLineNoiseUsPerToneGroup;
   dataLen=1;
   cmsRet=xdslCtl_SetObjectValue(lineId, oidStr1,3,(char*)dataBuf1, &dataLen);
   dataLen=512*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);

   printf("Tone number QLNus dataLen=%ld\n",dataLen);
   for (i = 0; i < dataLen; i+=8)
      printf("   %d\t%x %x %x %x %x %x %x %x\n", i, data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7]);

      
      
  oidStr[1]=kOidAdslPrivSNRUsPerToneGroup;
   dataLen=1;
   cmsRet=xdslCtl_SetObjectValue(lineId, oidStr1,3,(char*)dataBuf1, &dataLen);
   dataLen=512*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);

   printf("Tone number SNRus dataLen=%ld\n",dataLen);
   for (i = 0; i < dataLen; i+=8)
      printf("   %d\t%x %x %x %x %x %x %x %x\n", i, data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7]);

   oidStr[1]=kOidAdslPrivBitAllocUsPerToneGroup;
   dataLen=1;
   cmsRet=xdslCtl_SetObjectValue(lineId, oidStr1,3,(char*)dataBuf1, &dataLen);
   dataLen=512*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);

   printf("Tone number Bit Alloc US dataLen=%ld\n",dataLen);
   for (i = 0; i < dataLen; i+=8)
      printf("   %d\t%x %x %x %x %x %x %x %x\n", i, data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7]);
   oidStr[1]=kOidAdslPrivGainUsPerToneGroup;
   dataLen=1;
   cmsRet=xdslCtl_SetObjectValue(lineId, oidStr1,3,(char*)dataBuf1, &dataLen);
   dataLen=512*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
   
   printf("Tone number Gain Alloc US dataLen=%ld\n",dataLen);
   for (i = 0; i < dataLen; i+=8)
      printf("   %d\t%x %x %x %x %x %x %x %x\n", i, data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7]);


   dataLen=1;
   cmsRet=xdslCtl_SetObjectValue(lineId, oidStr1,3,(char*)dataBuf1, &dataLen);
   oidStr[1]=kOidAdslPrivChanCharLogUsPerToneGroup;
   dataLen=512*2;
   cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);

   printf("Tone number Hlog US dataLen=%ld\n",dataLen);
   for (i = 0; i < dataLen; i+=8)
      printf("   %d\t%x %x %x %x %x %x %x %x\n", i, data[i],data[i+1],data[i+2],data[i+3],data[i+4],data[i+5],data[i+6],data[i+7]);

   if (cmsRet != CMSRET_SUCCESS) {
      fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName );
      return;
   }
}

static void PrintPMDVectorFormat8tones(unsigned char lineId, bandPlanDescriptor* bandPlan,int id, int ValUnused)
{
	char  oidStr[] = { kOidAdslPrivate, 0 };
	CmsRet cmsRet;
	short *data = &scratchBuf[0];
	int n,toneNum,grpNum,ipartlen;
	long dataLen = sizeof(scratchBuf);
	
	oidStr[1] = id;
	cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
	if (cmsRet != CMSRET_SUCCESS) {
		fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName );
		return;
	}
	if(id==kOidAdslPrivSNR)
		ipartlen=4;
	else ipartlen=5;
	toneNum=0;
	grpNum=0;
	printf("%5d :",grpNum);
	for(n=0;n<bandPlan->noOfToneGroups;n++)
	{
		for(;toneNum<bandPlan->toneGroups[n].startTone;toneNum+=8)
		{
			printf("%s",QnToString1(ValUnused,0,ipartlen));
			grpNum++;
			if(grpNum%10==0)
				printf("\n%5d :",grpNum);
		}
		for(;toneNum<=bandPlan->toneGroups[n].endTone;toneNum+=8)
		{
			printf("%s",QnToString1(data[toneNum],4,ipartlen));
			grpNum++;
			if(grpNum%10==0)
				printf("\n%5d :",grpNum);
		}
	}
	while(grpNum<512)
	{
		printf("%s",QnToString1(ValUnused,0,ipartlen));
		grpNum++;
		if(grpNum%10==0)
			printf("\n%5d :",grpNum);
	}
	printf("\n");
}
static void PrintPMDHlinFormat8tones(unsigned char lineId,bandPlanDescriptor32* bandPlan, int ValUnused)
{
	char  oidStr[] = { kOidAdslPrivate, kOidAdslPrivChanCharLin };
	CmsRet cmsRet;
	short *data = &scratchBuf[0];
	int n,toneNum,grpNum;
	long dataLen = sizeof(scratchBuf);
	
	cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
	if (cmsRet != CMSRET_SUCCESS) {
		fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName );
		return;
	}
	printf(" real part:\n");
	toneNum=0;
	grpNum=0;
	printf("%5d :",grpNum);
	for(n=0;n<bandPlan->noOfToneGroups;n++)
	{
		for(;toneNum<bandPlan->toneGroups[n].startTone;toneNum+=8)
		{
			printf("%7d",ValUnused);
			grpNum++;
			if(grpNum%10==0)
				printf("\n%5d :",grpNum);
		}
		for(;toneNum<=bandPlan->toneGroups[n].endTone;toneNum+=8)
		{
			printf("%7d",data[2*toneNum]);
			grpNum++;
			if(grpNum%10==0)
				printf("\n%5d :",grpNum);
		}
	}
	while(grpNum<512)
	{
		printf("%7d",ValUnused);
		grpNum++;
		if(grpNum%10==0)
			printf("\n%5d :",grpNum);
	}
	printf("\n\n\n");
	toneNum=0;
	grpNum=0;
	printf(" imaginary part:\n");
	printf("%5d :",grpNum);
	for(n=0;n<bandPlan->noOfToneGroups;n++)
	{
		for(;toneNum<bandPlan->toneGroups[n].startTone;toneNum+=8)
		{
			printf("%7d",ValUnused);
			grpNum++;
			if(grpNum%10==0)
				printf("\n%5d :",grpNum);
		}
		for(;toneNum<=bandPlan->toneGroups[n].endTone;toneNum+=8)
		{
			printf("%7d",data[2*toneNum+1]);
			grpNum++;
			if(grpNum%10==0)
				printf("\n%5d :",grpNum);
		}
	}
	while(grpNum<512)
	{
		printf("%7d",ValUnused);
		grpNum++;
		if(grpNum%10==0)
			printf("\n%5d :",grpNum);
	}
	printf("\n");
}
#endif
static void PrintToneData2(unsigned char lineId, char *hdr, int id)
{
	int i;
	CmsRet cmsRet;
	char  oidStr[] = { kOidAdslPrivate, 0 };
	short *data = &scratchBuf[0];
	long	dataLen = sizeof(scratchBuf);

	oidStr[1] = id;
	cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
	if (cmsRet != CMSRET_SUCCESS) {
    	fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName );
    	return;
	}
	dataLen >>= 1;
	printf("Tone number      %s\n", hdr);
	for (i = 0; i < dataLen; i+=2)
    	printf("   %d\t\t%d\t%d\n", i >> 1, data[i], data[i+1]);
}        

#if defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
static void PrintToneData3(unsigned char lineId, char *hdr, int id)
{
	unsigned int i;
	CmsRet cmsRet;
	char  oidStr[] = { kOidAdslPrivate, 0 };
	int *data = (int *)&scratchBuf[0];
	long	dataLen = sizeof(scratchBuf);
	
	oidStr[1] = id;
	cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
	if (cmsRet != CMSRET_SUCCESS) {
		fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName );
		return;
	}
	
	printf("Tone number      %s\n", hdr);
	for (i = 0; i < dataLen/sizeof(int); i+=2)
		printf("   %d\t\t%d\t%d\n", i >> 1, data[i], data[i+1]);
}
#endif

static void PrintToneData2Scaled(unsigned char lineId, char *hdr __attribute__((unused)), int id, int dsScale, int usScale)
{
	int i;
	CmsRet cmsRet;
	long  dataLen,size;
	char  oidStr[] = { kOidAdslPrivate, 0 };
	adslMibInfo	*adslMib = NULL;
	short *data = &scratchBuf[0];
	dataLen = sizeof(scratchBuf);

#ifdef CONFIG_VDSL_SUPPORTED
	double   sc=0;
	int usB=0,dsB=0;
#endif

	double   dsc = dsScale;
	double   usc = usScale;
	size=sizeof(adslMibInfo);
	if(NULL == (adslMib = (void*)calloc(1, size))) {
		fprintf( stderr, "%s: %s calloc failed\n", g_szPgmName, __FUNCTION__);
		return;
	}
	
	cmsRet = xdslCtl_GetObjectValue(lineId, NULL, 0, (char *)adslMib, &size);
	if( cmsRet != CMSRET_SUCCESS) {
		fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName );
		goto free_exit;
	}
	
	oidStr[1] = id;
	cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)data, &dataLen);
	if (cmsRet != CMSRET_SUCCESS) {
		fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName);
		goto free_exit;
	}
	dataLen >>= 1;
#ifdef CONFIG_VDSL_SUPPORTED
	if( kVdslModVdsl2 == adslMib->adslConnection.modType)
	{
		for(i=0;i<dataLen;i+=2)
		{
			if(i>>1==adslMib->usNegBandPlan.toneGroups[usB].startTone)
			{
				sc=usc;
				usB++;
			}
			if(i>>1==adslMib->dsNegBandPlan.toneGroups[dsB].startTone)
			{
				sc=dsc;
				dsB++;
			}
			printf("   %d\t\t%e\t%e\n", i >> 1, sc*data[i]/(1 << 30), sc*data[i+1]/(1 << 30));
		}
		goto free_exit;
	}
	
#endif
	for (i = 0; i < 32*2; i+=2)
		printf("   %d\t\t%e\t%e\n", i >> 1, usc*data[i]/(1 << 30), usc*data[i+1]/(1 << 30));
	for (i = 32*2; i < dataLen; i+=2)
		printf("   %d\t\t%e\t%e\n", i >> 1, dsc*data[i]/(1 << 30), dsc*data[i+1]/(1 << 30));
	
free_exit:
	free(adslMib);
}

static void ProfileShow(unsigned char lineId)
{
	adslCfgProfile adslCfg;
	adslCfgProfile *pAdslCfg = &adslCfg;
	int nRet;
	UINT32 dslCfgParam;
#ifndef ANNEX_C
	adslVersionInfo adslVer;
	memset((void*)&adslVer, 0, sizeof(adslVer));
	xdslCtl_GetVersion(0, &adslVer);
#endif
	nRet = GetAdslCfg(lineId, &adslCfg);
	if (nRet != BCMADSL_STATUS_SUCCESS)
		return;

#ifdef ANNEX_C
	dslCfgParam = pAdslCfg->adslAnnexCParam;
#else
	dslCfgParam = pAdslCfg->adslAnnexAParam;
#endif
	printf("\nModulations:\n");
	if(kAdslCfgModAny == (dslCfgParam & kAdslCfgModMask)) {
		if(pAdslCfg->adsl2Param & kAdsl2CfgAnnexMOnly) {
			printf("\tG.Dmt\tDisabled\n");
			printf("\tG.lite\tDisabled\n");
			printf("\tT1.413\tDisabled\n");
			printf("\tADSL2\tDisabled\n");
			printf("\tAnnexL\tDisabled\n");
			printf("\tADSL2+\tDisabled\n");
			printf("\tAnnexM\tEnabled\n");
#ifdef CONFIG_VDSL_SUPPORTED
			printf("\tVDSL2\tDisabled\n");
			printf("\tVDSL2LR\tDisabled\n");
#endif
#ifdef SUPPORT_DSL_GFAST
			printf("\tGfast\tDisabled\n");
#endif
		}
		else {
			dslCfgParam |= kAdslCfgModMask;
			printf("\tG.Dmt\tEnabled\n");
			printf("\tG.lite\tEnabled\n");
			printf("\tT1.413\tEnabled\n");
			printf("\tADSL2\tEnabled\n");
			printf("\tAnnexL\tEnabled\n");
			printf("\tADSL2+\tEnabled\n");
			if(pAdslCfg->adsl2Param & kAdsl2CfgAnnexMEnabled)
				printf("\tAnnexM\tEnabled\n");
			else
				printf("\tAnnexM\tDisabled\n");
#ifdef CONFIG_VDSL_SUPPORTED
			printf("\tVDSL2\tEnabled\n");
			printf("\tVDSL2LR\tEnabled\n");
#endif
#ifdef SUPPORT_DSL_GFAST
			printf("\tGfast\tEnabled\n");
#endif
		}
	}
	else {
		if(dslCfgParam & kAdslCfgModGdmtOnly)
			printf("\tG.Dmt\tEnabled\n");
		else
			printf("\tG.Dmt\tDisabled\n");
		if(dslCfgParam & kAdslCfgModGliteOnly)
			printf("\tG.lite\tEnabled\n");
		else
			printf("\tG.lite\tDisabled\n");
		if(dslCfgParam & kAdslCfgModT1413Only)
			printf("\tT1.413\tEnabled\n");
		else
			printf("\tT1.413\tDisabled\n");
		if(dslCfgParam & kAdslCfgModAdsl2Only)
			printf("\tADSL2\tEnabled\n");
		else
			printf("\tADSL2\tDisabled\n");
		if(pAdslCfg->adsl2Param & kAdsl2CfgReachExOn)
			printf("\tAnnexL\tEnabled\n");
		else
			printf("\tAnnexL\tDisabled\n");
		if(dslCfgParam & kAdslCfgModAdsl2pOnly)
			printf("\tADSL2+\tEnabled\n");
		else
			printf("\tADSL2+\tDisabled\n");
		if(pAdslCfg->adsl2Param & kAdsl2CfgAnnexMEnabled)
			printf("\tAnnexM\tEnabled\n");
		else
			printf("\tAnnexM\tDisabled\n");
#ifdef CONFIG_VDSL_SUPPORTED
		if(dslCfgParam & kDslCfgModVdsl2Only)
			printf("\tVDSL2\tEnabled\n");
		else
			printf("\tVDSL2\tDisabled\n");
		if(dslCfgParam & kDslCfgModVdsl2LROnly)
			printf("\tVDSL2LR\tEnabled\n");
		else
			printf("\tVDSL2LR\tDisabled\n");
#endif
#ifdef SUPPORT_DSL_GFAST
		if(dslCfgParam & kDslCfgModGfastOnly)
			printf("\tGfast\tEnabled\n");
		else
			printf("\tGfast\tDisabled\n");
#endif
	}
	
#ifdef CONFIG_VDSL_SUPPORTED
	if(dslCfgParam & (kDslCfgModVdsl2Only|kDslCfgModGfastOnly)) {
		printf("VDSL2 profiles:\n");
		if(pAdslCfg->vdslParam & kVdslProfile8a)
			printf("\t8a\tEnabled\n");
		else
			printf("\t8a\tDisabled\n");
		if(pAdslCfg->vdslParam & kVdslProfile8b)
			printf("\t8b\tEnabled\n");
		else
			printf("\t8b\tDisabled\n");
		if(pAdslCfg->vdslParam & kVdslProfile8c)
			printf("\t8c\tEnabled\n");
		else
			printf("\t8c\tDisabled\n");
		if(pAdslCfg->vdslParam & kVdslProfile8d)
			printf("\t8d\tEnabled\n");
		else
			printf("\t8d\tDisabled\n");
		if(pAdslCfg->vdslParam & kVdslProfile12a)
			printf("\t12a\tEnabled\n");
		else
			printf("\t12a\tDisabled\n");
		if(pAdslCfg->vdslParam & kVdslProfile12b)
			printf("\t12b\tEnabled\n");
		else
			printf("\t12b\tDisabled\n");
		if(pAdslCfg->vdslParam & kVdslProfile17a)
			printf("\t17a\tEnabled\n");
		else
			printf("\t17a\tDisabled\n");
		if(pAdslCfg->vdslParam & kVdslProfile30a)
			printf("\t30a\tEnabled\n");
		else
			printf("\t30a\tDisabled\n");
#ifdef CONFIG_VDSLBRCMPRIV1_SUPPORT
		if(pAdslCfg->vdslParam & kVdslProfile35b)
			printf("\t35b\tEnabled\n");
		else
			printf("\t35b\tDisabled\n");
#endif
#ifdef CONFIG_VDSLBRCMPRIV2_SUPPORT
		if(pAdslCfg->vdslParam & kVdslProfileBrcmPriv2)
			printf("\tBrcmPriv2\tEnabled\n");
		else
			printf("\tBrcmPriv2\tDisabled\n");
#endif
	if(pAdslCfg->vdslParam & kVdslUS0Mask)
		printf("\tUS0\tEnabled\n");
	else
		printf("\tUS0\tDisabled\n");
#ifdef SUPPORT_DSL_GFAST
		printf("GFAST profiles:\n");
		if(0 == (pAdslCfg->vdslParam & kGfastProfile106aDisable))
			printf("\t106a\tEnabled\n");
		else
			printf("\t106a\tDisabled\n");
		if(0 == (pAdslCfg->vdslParam & kGfastProfile106bDisable))
			printf("\t106b\tEnabled\n");
		else
			printf("\t106b\tDisabled\n");
#ifndef CONFIG_BCM963138
		if(0 == (pAdslCfg->vdslParam & kGfastProfile212aDisable))
			printf("\t212a\tEnabled\n");
		else
			printf("\t212a\tDisabled\n");
		if(0 == (pAdslCfg->vdslParam & kGfastProfile106cDisable))
			printf("\t106c\tEnabled\n");
		else
			printf("\t106c\tDisabled\n");
		if(0 == (pAdslCfg->vdslParam & kGfastProfile212cDisable))
			printf("\t212c\tEnabled\n");
		else
			printf("\t212c\tDisabled\n");
#endif
#endif /* SUPPORT_DSL_GFAST */
	}
#endif

	printf("Phone line pair:\n");
	if(kAdslCfgLineOuterPair == (dslCfgParam & kAdslCfgLinePairMask))
		printf("\tOuter pair\n");
	else
		printf("\tInner pair\n");
	
	printf("Capability:\n");
	printf("\tbitswap\t\t%s\n", (pAdslCfg->adslDemodCapValue&kXdslBitSwapEnabled)? "On": "Off");
	printf("\tsra\t\t%s\n", (pAdslCfg->adslDemodCapValue&kXdslSRAEnabled)? "On": "Off");
	printf("\ttrellis\t\t%s\n", (pAdslCfg->adslDemodCapValue&kXdslTrellisEnabled)? "On": "Off");
	printf("\tsesdrop\t\t%s\n", (pAdslCfg->adslDemodCap2Value&kXdslRetrainOnSesEnabled)? "On": "Off");
	printf("\tCoMinMgn\t%s\n", (pAdslCfg->adslDemodCap2Value&kXdslRetrainOnDslamMinMargin)? "On": "Off");
	printf("\t24k\t\t%s\n", (pAdslCfg->adslDemodCap2Value&kXdsl24kbyteInterleavingEnabled)? "On": "Off");
	printf("\tphyReXmt(Us/Ds)\t%s/%s\n",
		(pAdslCfg->adslDemodCap2Value & kXdslFireUsSupported)? "On": "Off",
		(pAdslCfg->adslDemodCap2Value & kXdslFireDsSupported)? "On": "Off");
	printf("\tGinp(Us/Ds)\t%s/%s\n",
		(pAdslCfg->xdslAuxFeaturesValue & kXdslGinpUsSupported)? "On": "Off",
		(pAdslCfg->xdslAuxFeaturesValue & kXdslGinpDsSupported)? "On": "Off");
	if (0 == (dslCfgParam & kAdslCfgTpsTcMask)) {
#ifdef CONFIG_VDSL_SUPPORTED
		printf("\tTpsTc\t\tAvPvAa\n");
#else
		printf("\tTpsTc\t\tAa\n");
#endif
	}
	else {
		printf("\tTpsTc\t\t%s%s%s%s\n",
			#ifdef CONFIG_VDSL_SUPPORTED
			(dslCfgParam & kAdslCfgTpsTcAtmVdsl)?"Av":"", (dslCfgParam & kAdslCfgTpsTcPtmVdsl)?"Pv":"",
			#else
			"", "",
			#endif
			(dslCfgParam & kAdslCfgTpsTcAtmAdsl)?"Aa":"", (dslCfgParam & kAdslCfgTpsTcPtmAdsl)?"Pa":"");
	}
	
	printf("\tmonitorTone:\t%s\n", (pAdslCfg->xdslAuxFeaturesValue & kXdslMonitorToneDisable)? "Off": "On");
#ifdef CONFIG_VDSL_SUPPORTED
	printf("\tdynamicD:\t%s\n", (pAdslCfg->vdslCfgFlagsValue & kVdslDynamicDDisable)? "Off": "On");
	printf("\tdynamicF:\t%s\n", (pAdslCfg->vdslCfgFlagsValue & kVdslDynamicFDisable)? "Off": "On");
	printf("\tSOS:\t\t%s\n", (pAdslCfg->xdslAuxFeaturesValue & kVdslSOSEnableAux)? "On": "Off");
#endif
#ifndef ANNEX_C
	if(kAdslTypeAnnexB == adslVer.phyType) {
		printf("\tforceJ43:\t%s\n", (pAdslCfg->adslDemodCap2Value & (1 << 9))? "On": "Off");	/* kDslAnnexMcustomMode (1 << 9) */
		printf("\ttoggleJ43B43:\t%s\n", (pAdslCfg->xdslAuxFeaturesValue & 0x00000400)? "On": "Off");	/* kDslAnnexJhandshakeB43J43Toggle 0x00000400 */
	}
#endif
	printf("\tTraining Margin(Q4 in dB):\t%d%s\n",
		(int)pAdslCfg->adslTrainingMarginQ4,
		(-1==pAdslCfg->adslTrainingMarginQ4)? "(DEFAULT)":"");
#ifdef SUPPORT_MULTI_PHY
	printf("Media search CFG:\n");
	printf("\tPHY switch\t%s\n", (pAdslCfg->xdslMiscCfgParam & BCM_SWITCHPHY_DISABLED)? "Disabled": "Enabled");
	printf("\tMedia search\t%s\n", (pAdslCfg->xdslMiscCfgParam & BCM_MEDIASEARCH_DISABLED)? "Disabled": "Enabled");
	printf("\tSingle line only\t%s\n", (pAdslCfg->xdslMiscCfgParam & BCM_IMAGETYPE_SINGLELINE)? "Enabled": "Disabled");
	printf("\tDefault AFE\t%s\n",(pAdslCfg->xdslMiscCfgParam & BCM_MEDIATYPE_EXTERNALAFE)? "External": "Internal");
#elif defined(SUPPORT_DSL_GFAST)
	printf("PHY TYPE CFG:\n");
	printf("\tPHY switch\t%s\n", (pAdslCfg->xdslMiscCfgParam & BCM_PHYSWITCH_DISABLED)? "Disabled": "Enabled");
	if(!(pAdslCfg->xdslMiscCfgParam & BCM_SAVEPREFERPHY_DISABLED))
		printf("\tBootup PHY\t%s\n",
			((pAdslCfg->xdslMiscCfgParam & BCM_PREFERREDTYPE_FOUND) || !(dslCfgParam & kDslCfgModGfastOnly))? "Non Gfast": "Gfast");
#endif
}

static void PrintAdslCfg(unsigned char lineId)
{
	int	nRet;
	long	dataLen;
	adslCfgProfile	adsCfg;
	adslMibInfo		adslMib;

    nRet = GetAdslCfg(lineId, &adsCfg);
    if (nRet != BCMADSL_STATUS_SUCCESS)
		return;

	dataLen = sizeof(adslMib);
	nRet = xdslCtl_GetObjectValue(lineId, NULL, 0, (char *)&adslMib, &dataLen);

	printf("\n"
#if defined(ANNEX_A)
		"adslAnnexAParam:\t%08x\n"
#elif defined(ANNEX_C)
		"adslAnnexCParam:\t%08x\n"
#endif
		"adslTrainingMarginQ4:\t%d\n"
		"adslShowtimeMarginQ4:\t%d\n"
		"adslLOMTimeThldSec:\t%d\n"
		"adslDemodCapMask:\t%08x\n"
		"adslDemodCapValue:\t%08x\n"
		"adsl2Param:\t\t%08x\n"
		"adslPwmSyncClockFreq:\t%d\n"
		"adslHsModeSwitchTime:\t%d\n"
		"adslDemodCap2Mask:\t%08x\n"
		"adslDemodCap2Value:\t%08x\n"
#ifdef CONFIG_VDSL_SUPPORTED
		"vdslParam:\t\t%08x\n"
		"vdslParam1:\t\t%08x\n"
#endif
		"xdslAuxFeaturesMask:\t%08x\n"
		"xdslAuxFeaturesValue:\t%08x\n"
#ifdef CONFIG_VDSL_SUPPORTED
		"vdslCfgFlagsMask:\t%08x\n"
		"vdslCfgFlagsValue:\t%08x\n"
#endif
		"xdslCfg1Mask:\t%08x\n"
		"xdslCfg1Value:\t%08x\n"
		"xdslCfg2Mask:\t%08x\n"
		"xdslCfg2Value:\t%08x\n"
		"xdslCfg3Mask:\t%08x\n"
		"xdslCfg3Value:\t%08x\n"
		"xdslCfg4Mask:\t%08x\n"
		"xdslCfg4Value:\t%08x\n"
		"minINP:\t%d\n"
		"maxDelay:\t%d\n"
#ifdef CONFIG_VDSL_SUPPORTED
		"maxDsDataRateKbps:\t%d\n"
		"maxUsDataRateKbps:\t%d\n"
		"maxAggrDataRateKbps:\t%d\n"
#endif
		"xdslMiscCfgParam:\t%08x\n"
		"AFE_ID:\t\t\t%08x %08x\n"
#ifdef SECONDARY_AFEID_FN
		"SECONDARY_AFE_ID:\t%08x %08x\n"
#endif
		,
#if defined(ANNEX_A)
		(unsigned int)adsCfg.adslAnnexAParam,
#elif defined(ANNEX_C)
		(unsigned int)adsCfg.adslAnnexCParam,
#endif
		(int) adsCfg.adslTrainingMarginQ4,
		(int) adsCfg.adslShowtimeMarginQ4,
		(int) adsCfg.adslLOMTimeThldSec,
		(unsigned int) adsCfg.adslDemodCapMask,
		(unsigned int) adsCfg.adslDemodCapValue,
		(unsigned int) adsCfg.adsl2Param,
		(int) adsCfg.adslPwmSyncClockFreq,
		(int) adsCfg.adslHsModeSwitchTime,
		(unsigned int) adsCfg.adslDemodCap2Mask,
		(unsigned int) adsCfg.adslDemodCap2Value,
#ifdef CONFIG_VDSL_SUPPORTED
		(unsigned int) adsCfg.vdslParam,
		(unsigned int) adsCfg.vdslParam1,
#endif
		(unsigned int) adsCfg.xdslAuxFeaturesMask,
		(unsigned int) adsCfg.xdslAuxFeaturesValue,
#ifdef CONFIG_VDSL_SUPPORTED
		(unsigned int) adsCfg.vdslCfgFlagsMask,
		(unsigned int) adsCfg.vdslCfgFlagsValue,
#endif
		(unsigned int) adsCfg.xdslCfg1Mask,
		(unsigned int) adsCfg.xdslCfg1Value,
		(unsigned int) adsCfg.xdslCfg2Mask,
		(unsigned int) adsCfg.xdslCfg2Value,
		(unsigned int) adsCfg.xdslCfg3Mask,
		(unsigned int) adsCfg.xdslCfg3Value,
		(unsigned int) adsCfg.xdslCfg4Mask,
		(unsigned int) adsCfg.xdslCfg4Value,
		(unsigned int) adsCfg.minINP,
		adsCfg.maxDelay ? adsCfg.maxDelay & 0x7FFFFFFF : -1,
#ifdef CONFIG_VDSL_SUPPORTED
		(unsigned int) adsCfg.maxDsDataRateKbps,
		(unsigned int) adsCfg.maxUsDataRateKbps,
		(unsigned int) adsCfg.maxAggrDataRateKbps,
#endif
		(unsigned int) adsCfg.xdslMiscCfgParam,
		(unsigned int) adslMib.afeId[0], (unsigned int) adslMib.afeId[1]
#ifdef SECONDARY_AFEID_FN
		,adslMib.xdslSecondaryAfeId[0], (unsigned int) adslMib.xdslSecondaryAfeId[1]
#endif
		);
}

/* These functions are the same as those defined in xdslctl lib in adsl_api.c.
 * This file adslctl.c is part of a DSL driver release, and not adsl_api.c.   So we need to keep these
 * functions here so new DSL driver can still compile with older Linux SDK
 */
static int _GetRcvRate(adslMibInfo *pMib, int pathId)
{
	return pMib->xdslInfo.dirInfo[0].lpInfo[pathId].dataRate;
}

static int _GetXmtRate(adslMibInfo *pMib, int pathId)
{
	return pMib->xdslInfo.dirInfo[1].lpInfo[pathId].dataRate;
}


static int GetAdsl2Perq(xdslFramingInfo *p2, int q)
{
	return (0 == p2->M*p2->L) ? -1 : (int) ((2*q*p2->T*(p2->M*(p2->B[0]+1)+p2->R)*(p2->U*p2->G))/(p2->M*p2->L));
}

static int GetAdsl2Orq(xdslFramingInfo *p2, int q)
{
	int den = p2->T*(p2->M*(p2->B[0]+1)+p2->R);
	return (0 == den) ? -1 : (int) (4*q*p2->M*p2->L/den);
}

static int GetAdsl2AggrRate(xdslFramingInfo *p, int q)
{
	int       phyrAdj = (1 == p->rtxMode) ? 1 : 0;
	long long num = (long long) 1024*q*p->L*(p->N - p->R - phyrAdj);
    return (0 == p->N) ? -1 : num/(p->N*257);
}

#ifdef CONFIG_VDSL_SUPPORTED
static int GetVdsl2PERp(int vdslProf, xdslFramingInfo *p, int q)
{
	long long num = (long long) (257*q*p->N*p->T*p->U);
	if (kVdslProfile30a == vdslProf)
		num >>= 1;  /* Fs = 8Khz */
    return (0 == p->M*p->L) ? -1 : num/(4*32*p->M*p->L);
}

static int GetVdsl2ORp(int vdslProf, xdslFramingInfo *p, int q)
{
	long long num = (long long) 1024*q*p->M*p->L*p->G;
    int den = p->T*p->N;

	if (kVdslProfile30a == vdslProf)
		num <<= 1;  /* Fs = 8Khz */
    return (0 == den) ? -1 : num/(den*257);
}

static int GetVdsl2AggrRate(int vdslProf, xdslFramingInfo *p, int q)
{
	int       phyrAdj = (1 == p->rtxMode) ? 1 : 0;
	long long num = (long long) 1024*q*p->L*(p->N - p->R - phyrAdj);

	if (kVdslProfile30a == vdslProf)
		num <<= 1;  /* Fs = 8Khz */
    return (0 == p->N) ? -1 : num/(p->N*257);
}
#endif

static int GetXdsl2PERp(int modType, int vdslProf, xdslFramingInfo *p, int q)
{
#ifdef CONFIG_VDSL_SUPPORTED
	if(kVdslModVdsl2 == modType)
		return GetVdsl2PERp(vdslProf, p, q);
	else
#endif
	return GetAdsl2Perq(p, q);
}

static int GetXdsl2ORp(int modType, int vdslProf, xdslFramingInfo *p, int q)
{
#ifdef CONFIG_VDSL_SUPPORTED
	if(kVdslModVdsl2 == modType)
		return GetVdsl2ORp(vdslProf, p, q);
	else
#endif
	return GetAdsl2Orq(p, q);
}

static int GetXdsl2AgR(int modType, int vdslProf, xdslFramingInfo *p, int q)
{
#ifdef CONFIG_VDSL_SUPPORTED
	if(kVdslModVdsl2 == modType)
		return GetVdsl2AggrRate(vdslProf, p, q);
	else
#endif
	return GetAdsl2AggrRate(p, q);
}

char * GetVendorIdStr(char *id)
{
    static char returnedString[20];
    unsigned short version=((unsigned char)id[6]<<8)+(unsigned char)id[7];

    sprintf(returnedString,"%c%c%c%c:0x%04x",id[2],id[3],id[4],id[5],version);
    return returnedString; 
}

adsl2DelayInp *GetDelayInpPtr(adslMibInfo *pMib, int pathId, int rxTx)
{
  adsl2DelayInp  *pDelayInp;

#ifdef CONFIG_VDSL_SUPPORTED
  pDelayInp = (kVdslModVdsl2 == pMib->adslConnection.modType) ?  &pMib->vdslInfo[pathId].rcv2DelayInp : &pMib->adsl2Info.rcv2DelayInp;
#else
  pDelayInp = &pMib->adsl2Info.rcv2DelayInp;
#endif
  return (pDelayInp + rxTx);
}

#ifdef SUPPORT_24HR_CNT_STAT
void PrintStatHist24Hr(adslMibInfo *pAdslMib)
{
	int	i, curHrIndex;
	StatHistHrCounters	statHist24HrTotal;
	
	printf("Time elapsed for the current Hour: %02d:%02d(min:sec)\n",
		(int)pAdslMib->statHist24HrCounters.cur1HourTimeElapsed/60, (int)pAdslMib->statHist24HrCounters.cur1HourTimeElapsed%60);
	memset ((void *)&statHist24HrTotal, 0, sizeof(StatHistHrCounters));
	
	for(i=0; i < 24; i++) {
		curHrIndex = pAdslMib->statHist24HrCounters.curHourIndex-i;
		if(curHrIndex < 0)
			curHrIndex += 24;
		printf("Hour[%d].Syncs:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].Syncs);
		printf("Hour[%d].ReceiveBlocks:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].ReceiveBlocks);
		printf("Hour[%d].TransmitBlocks:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].TransmitBlocks);
		printf("Hour[%d].CellDelin:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].CellDelin);
		printf("Hour[%d].LinkRetrain:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].LinkRetrain);
		printf("Hour[%d].InitErrors:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].InitErrors);
		printf("Hour[%d].InitTimeouts:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].InitTimeouts);
		printf("Hour[%d].LossOfFraming:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].LossOfFraming);
		printf("Hour[%d].ErroredSecs:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].ErroredSecs);
		printf("Hour[%d].SeverelyErroredSecs:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].SeverelyErroredSecs);
		printf("Hour[%d].FECErrors:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].FECErrors);
		printf("Hour[%d].ATUCFECErrors:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].ATUCFECErrors);
		printf("Hour[%d].HECErrors:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].HECErrors);
		printf("Hour[%d].ATUCHECErrors:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].ATUCHECErrors);
		printf("Hour[%d].CRCErrors:	%u\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].CRCErrors);
		printf("Hour[%d].ATUCCRCErrors:	%u\n\n", i, pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].ATUCCRCErrors);

		/* Calc last 24 hours statistics */
		statHist24HrTotal.Syncs += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].Syncs;
		statHist24HrTotal.ReceiveBlocks += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].ReceiveBlocks;
		statHist24HrTotal.TransmitBlocks += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].TransmitBlocks;
		statHist24HrTotal.CellDelin += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].CellDelin;
		statHist24HrTotal.LinkRetrain += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].LinkRetrain;
		statHist24HrTotal.InitErrors += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].InitErrors;
		statHist24HrTotal.InitTimeouts += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].InitTimeouts;
		statHist24HrTotal.LossOfFraming += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].LossOfFraming;
		statHist24HrTotal.ErroredSecs += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].ErroredSecs;
		statHist24HrTotal.SeverelyErroredSecs += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].SeverelyErroredSecs;
		statHist24HrTotal.FECErrors += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].FECErrors;
		statHist24HrTotal.ATUCFECErrors += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].ATUCFECErrors;
		statHist24HrTotal.HECErrors += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].HECErrors;
		statHist24HrTotal.ATUCHECErrors += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].ATUCHECErrors;
		statHist24HrTotal.CRCErrors += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].CRCErrors;
		statHist24HrTotal.ATUCCRCErrors += pAdslMib->statHist24HrCounters.statHistHrCounters[curHrIndex].ATUCCRCErrors;
	}
	
	/* Print total 24 Hour statistics */
	printf("Last24Hours.Syncs:	%u\n", statHist24HrTotal.Syncs);
	printf("Last24Hours.ReceiveBlocks:	%u\n", statHist24HrTotal.ReceiveBlocks);
	printf("Last24Hours.TransmitBlocks:	%u\n", statHist24HrTotal.TransmitBlocks);
	printf("Last24Hours.CellDelin:	%u\n", statHist24HrTotal.CellDelin);
	printf("Last24Hours.LinkRetrain:	%u\n", statHist24HrTotal.LinkRetrain);
	printf("Last24Hours.InitErrors:	%u\n", statHist24HrTotal.InitErrors);
	printf("Last24Hours.InitTimeouts:	%u\n", statHist24HrTotal.InitTimeouts);
	printf("Last24Hours.LossOfFraming:	%u\n", statHist24HrTotal.LossOfFraming);
	printf("Last24Hours.ErroredSecs:	%u\n", statHist24HrTotal.ErroredSecs);
	printf("Last24Hours.SeverelyErroredSecs:	%u\n", statHist24HrTotal.SeverelyErroredSecs);
	printf("Last24Hours.FECErrors:	%u\n", statHist24HrTotal.FECErrors);
	printf("Last24Hours.ATUCFECErrors:	%u\n", statHist24HrTotal.ATUCFECErrors);
	printf("Last24Hours.HECErrors:	%u\n", statHist24HrTotal.HECErrors);
	printf("Last24Hours.ATUCHECErrors:	%u\n", statHist24HrTotal.ATUCHECErrors);
	printf("Last24Hours.CRCErrors:	%u\n", statHist24HrTotal.CRCErrors);
	printf("Last24Hours.ATUCCRCErrors:	%u\n\n", statHist24HrTotal.ATUCCRCErrors);
}
#endif

/***************************************************************************
 * Function Name: InfoHandler
 * Description  : Processes the adslctl info command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int InfoHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
	int nRet = 0, infoType = 1, infoNoConnect = 0;
	CmsRet		cmsRet;
	adslMibInfo	adslMib;
	long			size = sizeof(adslMib);
	int				pathId=0;
	int				modVdsl2=0;
	xdslFramingInfo	*pRxFramingParam, *pTxFramingParam;

	pOptions--;
	while ((0 == nRet) && (nNumOptions-- > 0)) {
		pOptions++;
		if (0 == strcmp(pOptions->pszOptName, "--state")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 1;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--show")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 2;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--sho1")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 2;
			infoNoConnect = 1;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--stats")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 3;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--SNR")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 4;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--QLN")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 5;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--Hlog")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 6;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--linediag")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 7;
		}
#ifdef CONFIG_VDSL_SUPPORTED
		else if (0 == strcmp(pOptions->pszOptName, "--linediag1")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 13;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--pbParams")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 14;
		}
#endif
		else if (0 == strcmp(pOptions->pszOptName, "--Hlin")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = INFO_TYPE_HLIN_RAW;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--Bits")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 9;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--HlinS")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = INFO_TYPE_HLIN_SCALED;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--reset")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			cmsRet = xdslCtl_ResetStatCounters(lineId);
			if (cmsRet != CMSRET_SUCCESS)
				fprintf( stderr, "%s: devCtl_adslResetStatCounters error\n", g_szPgmName );
			infoType = 0;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--vendor")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 11;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--cfg")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 12;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--toneGroupObjects")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = 15;
		}
#ifdef SUPPORT_VECTORING
		else if (0 == strcmp(pOptions->pszOptName, "--vectoring")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = INFO_TYPE_VECTORING_STATE;
		}
#endif
#ifdef SUPPORT_24HR_CNT_STAT
		else if (0 == strcmp(pOptions->pszOptName, "--24hrhiststat")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = INFO_TYPE_24HR_STATS;
		}
#endif
#if defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
		else if (0 == strcmp(pOptions->pszOptName, "--UER")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = INFO_TYPE_UER;
		}
#ifdef SUPPORT_SELT
		else if (0 == strcmp(pOptions->pszOptName, "--EchoVariance")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = INFO_TYPE_ECHO_VARIANCE;
		}
#endif
#endif
#if defined(CONFIG_RNC_SUPPORT)
		else if (0 == strcmp(pOptions->pszOptName, "--RNC_QLN")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = INFO_TYPE_RNC_QLN;
		}
#endif
#if defined(SUPPORT_DSL_GFAST)
		else if (0 == strcmp(pOptions->pszOptName, "--ALN")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = INFO_TYPE_ALN;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--BitsDOI")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = INFO_TYPE_DOI_BITALLOC;
		}
#endif
#ifdef CONFIG_TOD_SUPPORTED
		else if (0 == strcmp(pOptions->pszOptName, "--TOD")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			infoType = INFO_TYPE_TOD;
		}
#endif
		else
			nRet = ADSLCTL_INVALID_OPTION;
	}

	if (ADSLCTL_INVALID_PARAMETER == nRet) {
		fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName, pOptions->pszOptName);
		return nRet;
	}
	
	if (ADSLCTL_INVALID_OPTION == nRet) {
		fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);
		return nRet;
	}

	if (0 == infoType)
		return nRet;

	cmsRet = xdslCtl_GetObjectValue(lineId, NULL, 0, (char *)&adslMib, &size);
	if( cmsRet != CMSRET_SUCCESS) {
		fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n", g_szPgmName );
		return (ADSLCTL_GENERAL_ERROR);
	}

	printf("%s: ADSL driver and PHY status\nStatus: %s\n", g_szPgmName,
		GetTrainingStatusStr(&adslMib));
	
	printf("Last Retrain Reason:\t%x\n",(unsigned int)adslMib.adslPerfData.lastRetrainReason);
	
	printf("Last initialization procedure status:\t%x\n", adslMib.xdslInitializationCause);
	
	if(adslMib.adslPhys.adslLDCompleted) {
		if(kAdslTrainingConnected == adslMib.adslTrainingState)
			adslMib.adslPhys.adslLDCompleted = 0;
		if(1 == adslMib.adslPhys.adslLDCompleted)
			printf("Loop Diagnostic is in progress\n");
		else if(2 == adslMib.adslPhys.adslLDCompleted)
			printf("Loop Diagnostic is successfully completed!\n");
		else if(-1 == adslMib.adslPhys.adslLDCompleted)
			printf("Loop Diagnostic failed!\n");
		else if(-2 == adslMib.adslPhys.adslLDCompleted)
			printf("Last Loop Diagnostic session failed! A new session is in progress\n");
	}
	
	if (kAdslTrainingConnected == adslMib.adslTrainingState) {
		printf("Max:\tUpstream rate = %d Kbps, Downstream rate = %d Kbps\n",
			(int)(adslMib.adslAtucPhys.adslCurrAttainableRate / 1000),
			(int)(adslMib.adslPhys.adslCurrAttainableRate / 1000));
		printf ("%s:\t%s, Upstream rate = %d Kbps, Downstream rate = %d Kbps\n",
			(IsXdsl2(adslMib.adslConnection.modType)) ? "Bearer" : "Channel",
			(IsXdsl2(adslMib.adslConnection.modType)) ? "0": (kAdslIntlChannel == adslMib.adslConnection.chType) ? "INTR" : "FAST",
			_GetXmtRate(&adslMib, 0), _GetRcvRate(&adslMib, 0));
		if(IsXdsl2(adslMib.adslConnection.modType) && (adslMib.lp2Active || adslMib.lp2TxActive))
			printf ("Bearer:\t1, Upstream rate = %d Kbps, Downstream rate = %d Kbps", _GetXmtRate(&adslMib, 1), _GetRcvRate(&adslMib, 1));
	}
	printf ("\n");

	if (1 == infoType)
		return nRet;
	
	if (4 == infoType) {
		PrintToneData(lineId, "SNR", kOidAdslPrivSNR);
		return nRet;
	}
	
	if (5 == infoType) {
		PrintToneData(lineId, "QLN", kOidAdslPrivQuietLineNoise);
		return nRet;
	}
	
	if (6 == infoType) {
		PrintToneData(lineId, "Hlog", kOidAdslPrivChanCharLog);
		return nRet;
	}
#if defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
	if (INFO_TYPE_UER == infoType) {
		PrintToneData3(lineId, "UER", kOidAdslPrivUER);
		return nRet;
	}
#ifdef SUPPORT_SELT
	if (INFO_TYPE_ECHO_VARIANCE == infoType) {
		PrintToneData4(lineId, "EchoVariance", kOidAdslPrivEchoVariance);
		return nRet;
	}
#endif
#endif
#if defined(CONFIG_RNC_SUPPORT)
	if (INFO_TYPE_RNC_QLN == infoType) {
		PrintToneData(lineId, "RNC QLN", kOidAdslPrivQuietLineNoiseRnc);
		return nRet;
	}
#endif
#if defined(SUPPORT_DSL_GFAST)
	if (INFO_TYPE_ALN == infoType) {
		PrintToneData(lineId, "ALN", kOidAdslPrivActiveLineNoise);
		return nRet;
	}
	
	if (INFO_TYPE_DOI_BITALLOC == infoType) {
		PrintToneData1(lineId, "DOI Bit Allocation", kOidAdslPrivDoiBitAlloc);
		return nRet;
	}
#endif
	if (INFO_TYPE_HLIN_RAW == infoType) {
		printf("Hlin scale factor: DS = %u  US = %u\n", (unsigned int)adslMib.adslPhys.adslHlinScaleFactor,
			(unsigned int)adslMib.adslAtucPhys.adslHlinScaleFactor);
		PrintToneData2(lineId, "Hlin", kOidAdslPrivChanCharLin);
		return nRet;
	}
	
	if (INFO_TYPE_HLIN_SCALED == infoType) {
		PrintToneData2Scaled(lineId, "Hlin", kOidAdslPrivChanCharLin, adslMib.adslPhys.adslHlinScaleFactor, adslMib.adslAtucPhys.adslHlinScaleFactor);
		return nRet;
	}
	
	if (9 == infoType) {
		PrintToneData1(lineId, "Bit Allocation", kOidAdslPrivBitAlloc);
		return nRet;
	}
	
	if (11==infoType) {
#ifdef CONFIG_VDSL_SUPPORTED
		printf ("ChipSet Vendor Id:\t%s\n",GetVendorIdStr(adslMib.xdslAtucPhys.adslVendorID));
		printf ("ChipSet VersionNumber:\t%s\n",adslMib.xdslAtucPhys.adslVersionNumber);
		printf ("ChipSet SerialNumber:\t%s\n",adslMib.xdslAtucPhys.adslSerialNumber);
#else
		printf ("ChipSet Vendor Id:\t%s\n",GetVendorIdStr(adslMib.adslAtucPhys.adslVendorID));
		printf ("ChipSet VersionNumber:\t%s\n",adslMib.adslAtucPhys.adslVersionNumber);
		printf ("ChipSet SerialNumber:\t%s\n",adslMib.adslAtucPhys.adslSerialNumber);
#endif
#ifdef CO_G994_NSIF
		{
		int i;
		printf ("CO G994 NSIF:len=%d\n", adslMib.adslAtucPhys.nsifLen);
		for (i = 0; i < adslMib.adslAtucPhys.nsifLen; i++) {
		  printf("0x%02X ", adslMib.adslAtucPhys.adslNsif[i]);
		  if (7 == (i & 7))
			printf ("\n");
		}
		}
#endif
		return nRet;
	}

	if (12==infoType) {
		PrintAdslCfg(lineId);
		return nRet;
	}
	
	if (7 == infoType) {
		printf(
			"\t\tDown\t\tUp\n"
			"SNRM(dB):\t%c%d.%d\t\t%c%d.%d\n"
			"LATN(dB):\t%c%d.%d\t\t%c%d.%d\n"
			"SATN(dB):\t%c%d.%d\t\t%c%d.%d\n"
			"TxPwr(dBm):\t%c%d.%d\t\t%c%d.%d\n"
			"ATTNDR(Kbps):\t%d\t\t%d\n",
			DEC_POINT(adslMib.adslPhys.adslCurrSnrMgn), DEC_POINT(adslMib.adslAtucPhys.adslCurrSnrMgn),
			DEC_POINT(adslMib.adslPhys.adslCurrAtn), DEC_POINT(adslMib.adslAtucPhys.adslCurrAtn),
			DEC_POINT(adslMib.adslPhys.adslSignalAttn), DEC_POINT(adslMib.adslAtucPhys.adslSignalAttn),
			DEC_POINT(adslMib.adslAtucPhys.adslCurrOutputPwr), DEC_POINT(adslMib.adslPhys.adslCurrOutputPwr),
			(int)(adslMib.adslPhys.adslCurrAttainableRate / 1000),
			(int)(adslMib.adslAtucPhys.adslCurrAttainableRate / 1000));

		PrintToneData(lineId, "SNR", kOidAdslPrivSNR);
		printf ("\n");
		PrintToneData(lineId, "QLN", kOidAdslPrivQuietLineNoise);
		printf ("\n");
		PrintToneData(lineId, "Hlog", kOidAdslPrivChanCharLog);
		printf ("\n");
		printf("Hlin scale factor: DS = %u  US = %u\n", (unsigned int)adslMib.adslPhys.adslHlinScaleFactor,
			(unsigned int)adslMib.adslAtucPhys.adslHlinScaleFactor);
		PrintToneData2(lineId, "Hlin", kOidAdslPrivChanCharLin);
		return nRet;
	}
#ifdef CONFIG_VDSL_SUPPORTED
	if (13 == infoType) {
		int n,numDS;
		short data[5];
		long dataLen;
		char strPrint1[200],strPrint2[200];
		bandPlanDescriptor32	usNegBandPlanDiscPresentation,dsNegBandPlanDiscPresentation;
		char  oidStr[]={kOidAdslPrivate,kOidAdslPrivExtraInfo,kOidAdslPrivBandPlanDSNegDiscoveryPresentation};
		char  oidStr2[]={kOidAdslPrivate,kOidAdslPrivLATNusperband};
		
		if(kVdslModVdsl2 != adslMib.adslConnection.modType)
		{
			printf("Currently not in VDSL modulation --linediag1 is only for VDSL mode\n");
			return nRet;
		}
		dataLen=sizeof(bandPlanDescriptor32);
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)&dsNegBandPlanDiscPresentation, &dataLen);	
		if(dsNegBandPlanDiscPresentation.noOfToneGroups==4)
		{
			numDS=4;
			strcpy(strPrint1,"============================================================================================\n");
			strcpy(strPrint2,"	VDSL Band Status		U0		U1		U2		U3		U4		D1		D2		D3		D4\n");
		}
		else
		{
			numDS=3;
			strcpy(strPrint1,"====================================================================================\n");
			strcpy(strPrint2,"	VDSL Band Status		U0		U1		U2		U3		U4		D1		D2		D3\n");
		}
		printf(
			"		VDSL Port Details		Upstream		Downstream\n"
			"Attainable Net Data Rate:	   %6d kbps		   %6d kbps\n"
			"Actual Aggregate Tx Power:	   %c%4d.%d dBm		  %c%4d.%d dBm\n%s%s",
			(int)(adslMib.adslAtucPhys.adslCurrAttainableRate / 1000),
			(int)(adslMib.adslPhys.adslCurrAttainableRate / 1000),
			DEC_POINT(adslMib.adslPhys.adslCurrOutputPwr),DEC_POINT(adslMib.adslAtucPhys.adslCurrOutputPwr),strPrint1,strPrint2);
		printf("  Line Attenuation(dB):\t");
		dataLen=sizeof(bandPlanDescriptor32);
		oidStr[2]=kOidAdslPrivBandPlanUSNegDiscoveryPresentation;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)&usNegBandPlanDiscPresentation, &dataLen);
		dataLen = 5*sizeof(short);
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);
		for(n=0;n<=4;n++) {
			if(n<usNegBandPlanDiscPresentation.noOfToneGroups){
				if(data[n]==1023)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}
		
	
		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivLATNdsperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);
		
		for(n=0;n<numDS;n++) {
			if(n<dsNegBandPlanDiscPresentation.noOfToneGroups){
				if(data[n]==1023)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}
		
		printf("\n");
		printf("Signal Attenuation(dB):\t");
		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivSATNusperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);

		for(n=0;n<=4;n++) {
			if(n<usNegBandPlanDiscPresentation.noOfToneGroups){
				if(data[n]==1023)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}
		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivSATNdsperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);

		for(n=0;n<numDS;n++) {
			if(n<dsNegBandPlanDiscPresentation.noOfToneGroups){
				if(data[n]==1023)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}
		printf("\n");
		printf("		SNR Margin(dB):\t");
		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivSNRMusperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);

		for(n=0;n<=4;n++) {
			if(n<usNegBandPlanDiscPresentation.noOfToneGroups){
				if(data[n]<-511 || data[n] >511)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}

		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivSNRMdsperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);

		for(n=0;n<numDS;n++) {
			if(n<dsNegBandPlanDiscPresentation.noOfToneGroups){
				if(data[n]<-511 || data[n] >511)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}

		printf("\n\n\n Line 0 DS HLOG (dB) (grouped by 8 tones):\n");
		PrintPMDVectorFormat8tones(lineId, (void *)&adslMib.dsNegBandPlanDiscovery32,kOidAdslPrivChanCharLog,-96);
		printf("\n\n Line 0 US HLOG (dB) (grouped by 8 tones):\n");
		PrintPMDVectorFormat8tones(lineId, (void *)&adslMib.usNegBandPlanDiscovery32,kOidAdslPrivChanCharLog,-96);
		printf("\n\n Line 0 DS QLN (dBm/Hz) (grouped by 8 tones):\n");
		PrintPMDVectorFormat8tones(lineId, (void *)&adslMib.dsNegBandPlanDiscovery32,kOidAdslPrivQuietLineNoise,-160);
		printf("\n\n Line 0 US QLN (dBm/Hz) (grouped by 8 tones):\n");
		PrintPMDVectorFormat8tones(lineId, (void *)&adslMib.usNegBandPlanDiscovery32,kOidAdslPrivQuietLineNoise,-160);
		printf("\n\n Line 0 DS SNR (dB) (grouped by 8 tones):\n");
		PrintPMDVectorFormat8tones(lineId, (void *)&adslMib.dsNegBandPlan32,kOidAdslPrivSNR,0);
		printf("\n\n Line 0 US SNR (dB) (grouped by 8 tones):\n");
		PrintPMDVectorFormat8tones(lineId, (void *)&adslMib.usNegBandPlan32,kOidAdslPrivSNR,0);
		printf("\n\n Line 0 DS HLIN (grouped by 8 tones):\n");
		PrintPMDHlinFormat8tones(lineId, (void *)&adslMib.dsNegBandPlan32,-1);
		printf(" scaled by %u/2^30\n",(unsigned int)adslMib.adslPhys.adslHlinScaleFactor);
		printf("\n\n Line 0 US HLIN (grouped by 8 tones):\n");
		PrintPMDHlinFormat8tones(lineId, (void *)&adslMib.usNegBandPlan32,-1);
		printf(" scaled by %u/2^30\n",(unsigned int)adslMib.adslAtucPhys.adslHlinScaleFactor);
		return nRet;
	}
	if (14 == infoType) {
		int n,numDS, numUS;
		short data[5];
		long dataLen;
		char strPrint1[200],strPrint2[200];
		bandPlanDescriptor32	usNegBandPlanDiscPresentation,usNegBandPlanPresentation,dsNegBandPlanDiscPresentation,dsNegBandPlanPresentation;
		char  oidStr[]={kOidAdslPrivate,kOidAdslPrivExtraInfo,kOidAdslPrivBandPlanDSNegDiscoveryPresentation};
		char  oidStr2[]={kOidAdslPrivate,kOidAdslPrivLATNusperband};
		if((kVdslModVdsl2 != adslMib.adslConnection.modType) && (kXdslModGfast != adslMib.adslConnection.modType))
		{
			printf("Currently not in VDSL modulation --pbParams is only for VDSL mode\n");
			return nRet;
		}
		printf("Discovery Phase (Initial) Band Plan\nUS: ");
		for(n=0;n<adslMib.usNegBandPlanDiscovery.noOfToneGroups;n++)
		{
			printf("(%d,%d) ",adslMib.usNegBandPlanDiscovery.toneGroups[n].startTone,adslMib.usNegBandPlanDiscovery.toneGroups[n].endTone);
		}
		printf("Gfactor=%d",adslMib.gFactors.Gfactor_SUPPORTERCARRIERSus);
		printf("\nDS: ");
		for(n=0;n<adslMib.dsNegBandPlanDiscovery.noOfToneGroups;n++)
		{
			printf("(%d,%d) ",adslMib.dsNegBandPlanDiscovery.toneGroups[n].startTone,adslMib.dsNegBandPlanDiscovery.toneGroups[n].endTone);
		}
		printf("Gfactor=%d",adslMib.gFactors.Gfactor_SUPPORTERCARRIERSds);
		printf("\nMedley Phase (Final) Band Plan\nUS: ");
		for(n=0;n<adslMib.usNegBandPlan.noOfToneGroups;n++)
		{
			printf("(%d,%d) ",adslMib.usNegBandPlan.toneGroups[n].startTone,adslMib.usNegBandPlan.toneGroups[n].endTone);
		}
		printf("Gfactor=%d",adslMib.gFactors.Gfactor_MEDLEYSETus);
		printf("\nDS: ");
		for(n=0;n<adslMib.dsNegBandPlan.noOfToneGroups;n++)
		{
			printf("(%d,%d) ",adslMib.dsNegBandPlan.toneGroups[n].startTone,adslMib.dsNegBandPlan.toneGroups[n].endTone);
		}
		printf("Gfactor=%d\n",adslMib.gFactors.Gfactor_MEDLEYSETds);
		dataLen=sizeof(bandPlanDescriptor32);
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)&dsNegBandPlanDiscPresentation, &dataLen);
		if(dsNegBandPlanDiscPresentation.noOfToneGroups==4)
		{
			numDS=4;
			strcpy(strPrint1,"============================================================================================\n");
			strcpy(strPrint2,"VDSL Band Status	U0	U1	U2	U3	U4	D1	D2	D3	D4\n");
		}
		else
		{
			numDS=3;
			strcpy(strPrint1,"====================================================================================\n");
			strcpy(strPrint2,"VDSL Band Status	U0	U1	U2	U3	U4	D1	D2	D3\n");
		}
		printf(
			"\nVDSL Port Details		Upstream		Downstream\n"
			"Attainable Net Data Rate:	%6d kbps		%6d kbps\n"
			"Actual Aggregate Tx Power:	%c%4d.%d dBm		%c%4d.%d dBm\n%s%s",
			(int)(adslMib.adslAtucPhys.adslCurrAttainableRate / 1000),
			(int)(adslMib.adslPhys.adslCurrAttainableRate / 1000),
			DEC_POINT(adslMib.adslPhys.adslCurrOutputPwr),DEC_POINT(adslMib.adslAtucPhys.adslCurrOutputPwr),strPrint1,strPrint2);
		printf("  Line Attenuation(dB):\t");
		dataLen=sizeof(bandPlanDescriptor32);
		oidStr[2]=kOidAdslPrivBandPlanUSNegDiscoveryPresentation;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)&usNegBandPlanDiscPresentation, &dataLen);
		dataLen = 5*sizeof(short);
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);
		for(n=0;n<=4;n++) {
			if(n<usNegBandPlanDiscPresentation.noOfToneGroups){
				if(data[n]==1023)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}
		
		
		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivLATNdsperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);
		
		for(n=0;n<numDS;n++) {
			if(n<dsNegBandPlanDiscPresentation.noOfToneGroups){
				if(data[n]==1023)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}
		
		printf("\n");
		printf("Signal Attenuation(dB):\t");
	
		dataLen=sizeof(bandPlanDescriptor32);
		oidStr[2]=kOidAdslPrivBandPlanUSNegPresentation;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)&usNegBandPlanPresentation, &dataLen);
		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivSATNusperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);

		for(n=0;n<=4;n++) {
			if(n<usNegBandPlanPresentation.noOfToneGroups&&usNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF){
				if(data[n]==1023)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}
		

		dataLen=sizeof(bandPlanDescriptor32);
		oidStr[2]=kOidAdslPrivBandPlanDSNegPresentation;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)&dsNegBandPlanPresentation, &dataLen);
		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivSATNdsperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);

		for(n=0;n<numDS;n++) {
		    if(n<dsNegBandPlanPresentation.noOfToneGroups&&dsNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF){
				if(data[n]==1023)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}
		

		printf("\n");
		printf("	SNR Margin(dB):\t");

		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivSNRMusperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);

		for(n=0;n<=4;n++) {
			if(n<usNegBandPlanPresentation.noOfToneGroups&&usNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF){
				if(data[n]<-511 || data[n] >511)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}

		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivSNRMdsperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);

		for(n=0;n<numDS;n++) {
			if(n<dsNegBandPlanPresentation.noOfToneGroups&&dsNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF){
				if(data[n]<-511 || data[n] >511)
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}
		
		printf("\n");
		printf("	TX Power(dBm):\t");


		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivTxPwrusperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);


		for(n=0;n<=4;n++) {
			if(n<usNegBandPlanPresentation.noOfToneGroups&&usNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF){
				if((data[n] < -1280) || (data[n] > 1280))
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}

		dataLen = 5*sizeof(short);
		oidStr2[1]=kOidAdslPrivTxPwrdsperband;
		cmsRet=xdslCtl_GetObjectValue(lineId, oidStr2, sizeof(oidStr2), (char *)&data[0], &dataLen);
		for(n=0;n<numDS;n++) {
			if(n<dsNegBandPlanPresentation.noOfToneGroups&&dsNegBandPlanPresentation.toneGroups[n].startTone!=0xFFFF){
				if((data[n] < -1280) || (data[n] > 1280))
					printf("  N/A\t");
				else
					printf("%c%d.%d\t",DEC_POINT(data[n]));
			}
			else
				printf("  N/A\t");
		}
		
		printf("\n");
		printf("	kl0(dBx100):\t");
		if(adslMib.usNegBandPlan.toneGroups[0].endTone < 256)
			numUS = 4;
		else {
			printf("  N/A\t");
			numUS = 3;
		}
		for(n = 0; n <= numUS; n++) {
			if(n < adslMib.xdslAtucPhys.numKl0BandReported)
				printf("%d\t", adslMib.xdslAtucPhys.kl0PerBand[n]);
			else
				printf("  N/A\t");
		}
		for(n = 0; n < numDS; n++) {
			if(n < adslMib.xdslPhys.numKl0BandReported)
				printf("%d\t", adslMib.xdslPhys.kl0PerBand[n]);
			else
				printf("  N/A\t");
		}
		printf("\n");
		
		return nRet;
	}
	if (15 == infoType) {
		PrintToneGroupObjects(lineId);
		return nRet;
	}
#ifdef SUPPORT_VECTORING
	if (INFO_TYPE_VECTORING_STATE == infoType) {
		long len;
		char vectState, oidStr[] = { kOidAdslPrivate, kOIdAdslPrivGetVectState };
		
		if(kVdslModVdsl2 != adslMib.adslConnection.modType) {
			printf("Currently not in VDSL modulation --vectoring is only for VDSL mode\n");
			return nRet;
		}
		
		len = sizeof(vectState);
		cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), &vectState, &len);
		if( cmsRet != CMSRET_SUCCESS) {
			fprintf( stderr, "%s: xdslCtl_GetObjectValue() failed getting vectSM.state\n", g_szPgmName );
			return (ADSLCTL_GENERAL_ERROR);
		}
		if (VECT_DISABLED == vectState) {
			printf("Vectoring is disabled\n");
			return nRet;
		}
		else {
			unsigned int extraSkb;

			len = sizeof(extraSkb);
			oidStr[1] = kOidAdslPrivGetExtraSkbCnt;
			cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)&extraSkb, &len);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: xdslCtl_GetObjectValue() failed getting extraSkb.count\n", g_szPgmName );
				return (ADSLCTL_GENERAL_ERROR);
			}
			
			printf("Vectoring state: %d\n", vectState);
			if (0 == adslMib.vectData.macAddress.addressType)
				printf("VCE MAC Address: %x:%x:%x:%x:%x:%x\n", adslMib.vectData.macAddress.macAddress[0], adslMib.vectData.macAddress.macAddress[1], adslMib.vectData.macAddress.macAddress[2], adslMib.vectData.macAddress.macAddress[3], adslMib.vectData.macAddress.macAddress[4], adslMib.vectData.macAddress.macAddress[5]);
			else
				printf("VCE IP Address: %d.%d.%d.%d:%d\n",adslMib.vectData.macAddress.macAddress[0], adslMib.vectData.macAddress.macAddress[1], adslMib.vectData.macAddress.macAddress[2], adslMib.vectData.macAddress.macAddress[3], (adslMib.vectData.macAddress.macAddress[4]+(adslMib.vectData.macAddress.macAddress[5]<<8)));

			printf("Total error samples Ethernet pkts sent: %u\n", adslMib.vectData.vectStat.cntESPktSend);
			printf("Total error samples Ethernet pkts discarded: %u\n", adslMib.vectData.vectStat.cntESPktDrop);
			printf("Total error samples statuses sent: %u\n", adslMib.vectData.vectStat.cntESStatSend);
			printf("Total error samples statuses discarded: %u\n", adslMib.vectData.vectStat.cntESStatDrop);
			printf("Total extra SKB buffers allocated: %u\n", extraSkb);
			return nRet;
		}
	}
#endif
#ifdef SUPPORT_24HR_CNT_STAT
	if (INFO_TYPE_24HR_STATS == infoType) {
		PrintStatHist24Hr(&adslMib);
		return nRet;
	}
#endif
#endif

#ifdef CONFIG_TOD_SUPPORTED
	if (INFO_TYPE_TOD == infoType) {
		TodInfo todInfo;
		char oidStr[] = { kOidAdslPrivate, kOidAdslPrivGetTodInfo };
		size = sizeof(TodInfo);
		cmsRet = xdslCtl_GetObjectValue(lineId, oidStr, sizeof(oidStr), (char *)&todInfo, &size);
		if( cmsRet != CMSRET_SUCCESS) {
			fprintf( stderr, "%s: xdslCtl_GetObjectValue() failed getting todInfo\n", g_szPgmName );
			return (ADSLCTL_GENERAL_ERROR);
		}
		if(todInfo.todStatus) {
#ifdef SUPPORT_DSL_BONDING
			printf("ToD is active on line%d\n",todInfo.todStatus - 1);
#else
			printf("ToD is active\n");
#endif
			printf("secondsSinceEpoch 0x%llX nanoSecondsSinceEpoch 0x%X\n",
				todInfo.todTimeStampRpt.secondsSinceEpoch,
				todInfo.todTimeStampRpt.nanoSecondsSinceEpoch);
		}
		else
			printf("ToD is not active\n");
		return nRet;
	}
#endif

	if (infoType >= 2) {
		int modType = adslMib.adslConnection.modType;
#ifdef CONFIG_VDSL_SUPPORTED
		int vdslProf = adslMib.xdslInfo.vdsl2Profile;
		if(kVdslModVdsl2 == modType)
			modVdsl2 = 1;
#else
		int vdslProf = 0;
#endif
		printf ("Link Power State:\tL%d\n", adslMib.xdslInfo.pwrState);

		if (infoNoConnect || (kAdslTrainingConnected == adslMib.adslTrainingState)) {
			int   rK = adslMib.adslConnection.rcvInfo.K;
			int   xK = adslMib.adslConnection.xmtInfo.K;
			char trellisString[80];
			if(adslMib.adslConnection.modType<kAdslModAdsl2 ) {
				if(adslMib.adslConnection.trellisCoding ==kAdslTrellisOn)
					strcpy(trellisString,"ON");
				else strcpy(trellisString,"OFF");
			}
			else {
				if(0 == (adslMib.adslConnection.trellisCoding2 & kAdsl2TrellisTxEnabled))
					strcpy(trellisString,"U:OFF");
				else
					strcpy(trellisString,"U:ON");
				if(0 == (adslMib.adslConnection.trellisCoding2 & kAdsl2TrellisRxEnabled))
					strcat(trellisString," /D:OFF");
				else
					strcat(trellisString," /D:ON");
			}

			printf ("Mode:\t\t\t%s %s\n",
				GetModulationStr(adslMib.adslConnection.modType),
				GetModulationStr2(adslMib.adslConnection.modType, adslMib.xdslInfo.xdslMode));
#ifdef CONFIG_VDSL_SUPPORTED
			if(modVdsl2) {
				printf (	"VDSL2 Profile:\t\t%s\n", GetSelectedProfileStr(vdslProf));
				if (adslMib.xdslInfo.vdslLRmode != 0)
					printf (	"VDSL2-LR:\t\t%s\n", GetVdsl2LrStr(adslMib.xdslInfo.vdslLRmode));
			}
#endif
			printf ("TPS-TC:\t\t\t%s(0x%X)\n", GetConnectionTypeStr(adslMib.xdslInfo.dirInfo[1].lpInfo[0].tmType[0]), adslMib.xdslInfo.dirInfo[1].lpInfo[0].tpsTcOptions);
			printf (
				"Trellis:\t\t%s\n"
				"Line Status:\t\t%s\n"
				"Training Status:\t%s\n"
				"\t\tDown\t\tUp\n"
				"SNR (dB):\t%c%d.%d\t\t%c%d.%d\n"
				"Attn(dB):\t%c%d.%d\t\t%c%d.%d\n"
				"Pwr(dBm):\t%c%d.%d\t\t%c%d.%d\n",

				trellisString,
				GetStateStatusStr(adslMib.adslPhys.adslCurrStatus),
				GetTrainingStatusStr(&adslMib),

				DEC_POINT(adslMib.adslPhys.adslCurrSnrMgn), DEC_POINT(adslMib.adslAtucPhys.adslCurrSnrMgn),
				DEC_POINT(adslMib.adslPhys.adslCurrAtn), DEC_POINT(adslMib.adslAtucPhys.adslCurrAtn),
				DEC_POINT(adslMib.adslAtucPhys.adslCurrOutputPwr), DEC_POINT(adslMib.adslPhys.adslCurrOutputPwr));

			if(!IsXdsl2(adslMib.adslConnection.modType)) {
				pRxFramingParam = &adslMib.xdslInfo.dirInfo[0].lpInfo[0];
				pTxFramingParam = &adslMib.xdslInfo.dirInfo[1].lpInfo[0];
				printf (
					"\t\t\tG.dmt framing\n"
					"K:\t\t%d(%d)\t\t%d\n"
					"R:\t\t%d\t\t%d\n"
					"S:\t\t%5.4f\t\t%5.4f\n"
					"D:\t\t%d\t\t%d\n",
					rK, adslMib.adslRxNonStdFramingAdjustK, xK,
					pRxFramingParam->R, pTxFramingParam->R,
					(pRxFramingParam->S.denom) ? (float)pRxFramingParam->S.num/(float)pRxFramingParam->S.denom : 0,
					(pTxFramingParam->S.denom) ? (float)pTxFramingParam->S.num/(float)pTxFramingParam->S.denom : 0,
					pRxFramingParam->D, pTxFramingParam->D);
			}
			else {
				printf ("\n\t\t\t%s framing\n",(kXdslModGfast==adslMib.adslConnection.modType)? "G.fast" :
					(kVdslModVdsl2==adslMib.adslConnection.modType)? "VDSL2": "ADSL2");
#ifdef SUPPORT_DSL_GFAST
				if(kXdslModGfast == adslMib.adslConnection.modType) {
					pRxFramingParam = &adslMib.xdslInfo.dirInfo[0].lpInfo[0];
					pTxFramingParam = &adslMib.xdslInfo.dirInfo[1].lpInfo[0];
					printf (
						"\t\t\tBearer 0\n"
						"R:\t\t%d\t\t%d\n"
						"N:\t\t%d\t\t%d\n"
						"Q:\t\t%d\t\t%d\n\n"
						
						"L:\t\t%d\t\t%d\n"
						"Lrmc:\t\t%d\t\t%d\n"
						"Ldoi:\t\t%d\t\t%d\n"
						"Rrmc:\t\t%d\t\t%d\n"
						"Drmc:\t\t%d\t\t%d\n\n"
						
						"Mf:\t\t%d\t\t%d\n"
						"M(ds/us):\t%d\t\t%d\n\n"
						
						"MNDSNOI:\t%d\t\t%d\n"
						"ackWindowShift:\t%d\t\t%d\n"
						"Ldr:\t\t%d\t\t%d\n"
						"etru:\t\t%d\t\t%d\n"
						"ETRminEoc:\t%d\t\t%d\n",
						pRxFramingParam->R, pTxFramingParam->R,
						pRxFramingParam->N, pTxFramingParam->N,
						pRxFramingParam->Q, pTxFramingParam->Q,
						
						pRxFramingParam->L, pTxFramingParam->L,
						pRxFramingParam->Lrmc, pTxFramingParam->Lrmc,
						pRxFramingParam->Ldoi, pTxFramingParam->Ldoi,
						pRxFramingParam->Rrmc, pTxFramingParam->Rrmc,
						pRxFramingParam->Drmc, pTxFramingParam->Drmc,
						
						pRxFramingParam->Mf, pTxFramingParam->Mf,
						pRxFramingParam->M, pTxFramingParam->M,
						
						pRxFramingParam->MNDSNOI, pTxFramingParam->MNDSNOI,
						pRxFramingParam->ackWindowShift, pTxFramingParam->ackWindowShift,
						pRxFramingParam->Ldr, pTxFramingParam->Ldr,
						pRxFramingParam->etru, pTxFramingParam->etru,
						pRxFramingParam->ETRminEoc, pTxFramingParam->ETRminEoc);
				}
				else
#endif
				for(pathId = 0; pathId < MAX_LP_NUM; pathId++) {
					if((0==pathId) || (adslMib.lp2Active || adslMib.lp2TxActive) ) {
						pRxFramingParam = &adslMib.xdslInfo.dirInfo[0].lpInfo[pathId];
						pTxFramingParam = &adslMib.xdslInfo.dirInfo[1].lpInfo[pathId];
						printf (
						"\t\t\tBearer %d\n"
						"MSGc:\t\t%d\t\t%d\n"
						"B:\t\t%d\t\t%d\n"
						"M:\t\t%d\t\t%d\n"
						"T:\t\t%d\t\t%d\n"
						"R:\t\t%d\t\t%d\n"
						"S:\t\t%5.4f\t\t%5.4f\n"
						"L:\t\t%d\t\t%d\n"
						"D:\t\t%d\t\t%d\n",
						pathId,
						pRxFramingParam->U*pRxFramingParam->G-6, pTxFramingParam->U*pTxFramingParam->G-6,
						pRxFramingParam->B[0], pTxFramingParam->B[0],
						pRxFramingParam->M, pTxFramingParam->M,
						pRxFramingParam->T, pTxFramingParam->T,
						pRxFramingParam->R, pTxFramingParam->R,
						(pRxFramingParam->S.denom) ? (float)pRxFramingParam->S.num/(float)pRxFramingParam->S.denom : 0,
						(pTxFramingParam->S.denom) ? (float)pTxFramingParam->S.num/(float)pTxFramingParam->S.denom : 0,
						pRxFramingParam->L, pTxFramingParam->L,
						pRxFramingParam->D, pTxFramingParam->D);

						if( modVdsl2 ) {
							printf (
							"I:\t\t%d\t\t%d\n"
							"N:\t\t%d\t\t%d\n",
							pRxFramingParam->I, pTxFramingParam->I,
							pRxFramingParam->N, pTxFramingParam->N);
						}
						if(adslMib.xdslStat[0].ginpStat.status & 0xC) {
							printf (
							"Q:\t\t%d\t\t%d\n"
							"V:\t\t%d\t\t%d\n"
							"RxQueue:\t\t%d\t\t%d\n"
							"TxQueue:\t\t%d\t\t%d\n"
							"G.INP Framing:\t\t%d\t\t%d\n"
							"G.INP lookback:\t\t%d\t\t%d\n"
							"RRC bits:\t\t%d\t\t%d\n",
							pRxFramingParam->Q, pTxFramingParam->Q,
							pRxFramingParam->V, pTxFramingParam->V,
							pRxFramingParam->rxQueue, pTxFramingParam->rxQueue,
							pRxFramingParam->txQueue, pTxFramingParam->txQueue,
							pRxFramingParam->rtxMode, pTxFramingParam->rtxMode,
							pRxFramingParam->ginpLookBack, pTxFramingParam->ginpLookBack,
							pRxFramingParam->rrcBits, pTxFramingParam->rrcBits);
						}
					}
				}
			}

			printf ("\n\t\t\tCounters\n");
			for(pathId = 0; pathId < MAX_LP_NUM; pathId++) {
				if((0==pathId) || (adslMib.lp2Active || adslMib.lp2TxActive) ) {
					pRxFramingParam = &adslMib.xdslInfo.dirInfo[0].lpInfo[pathId];
					pTxFramingParam = &adslMib.xdslInfo.dirInfo[1].lpInfo[pathId];
					printf (
					"\t\t\tBearer %d\n"
					"%s:\t\t%u\t\t%u\n"
					"%s:\t\t%u\t\t%u\n",
					pathId,
					(IsVdsl2OrGfastConnection(adslMib.adslConnection.modType))? "OHF" : "SF",
					(unsigned int)adslMib.xdslStat[pathId].rcvStat.cntSF,(unsigned int)adslMib.xdslStat[pathId].xmtStat.cntSF,
					(IsVdsl2OrGfastConnection(adslMib.adslConnection.modType))? "OHFErr" : "SFErr",
					(unsigned int)adslMib.xdslStat[pathId].rcvStat.cntSFErr, (unsigned int)adslMib.xdslStat[pathId].xmtStat.cntSFErr);
					
					printf(
						"RS:\t\t%u\t\t%u\n"
						"RSCorr:\t\t%u\t\t%u\n"
						"RSUnCorr:\t%u\t\t%u\n",
						(unsigned int)adslMib.xdslStat[pathId].rcvStat.cntRS, (unsigned int)adslMib.xdslStat[pathId].xmtStat.cntRS,
						(unsigned int) adslMib.xdslStat[pathId].rcvStat.cntRSCor,(unsigned int) adslMib.xdslStat[pathId].xmtStat.cntRSCor,
						(unsigned int)adslMib.xdslStat[pathId].rcvStat.cntRSUncor, (unsigned int)adslMib.xdslStat[pathId].xmtStat.cntRSUncor);
				}
			}

			if((adslMib.xdslStat[0].ginpStat.status & 0xC) || (kXdslModGfast==adslMib.adslConnection.modType)) {
				printf(
					"\n\t\t\tRetransmit Counters\n"
					"rtx_tx:\t\t%u\t\t%u\n"
					"rtx_c:\t\t%u\t\t%u\n"
					"rtx_uc:\t\t%u\t\t%u\n",
					adslMib.adslStat.ginpStat.cntDS.rtx_tx, adslMib.adslStat.ginpStat.cntUS.rtx_tx,
					adslMib.adslStat.ginpStat.cntDS.rtx_c, adslMib.adslStat.ginpStat.cntUS.rtx_c,
					adslMib.adslStat.ginpStat.cntDS.rtx_uc, adslMib.adslStat.ginpStat.cntUS.rtx_uc);
				if(adslMib.xdslStat[0].ginpStat.status & 0xC)
					printf(
						"\n\t\t\tG.INP Counters\n"
						"LEFTRS:\t\t%u\t\t%u\n",
						adslMib.adslStat.ginpStat.cntDS.LEFTRS, adslMib.adslStat.ginpStat.cntUS.LEFTRS);
				else
					printf("\n\t\t\tG.fast Counters\n");
				printf(
					"minEFTR:\t%u\t\t%u\n"
					"errFreeBits:\t%u\t\t%u\n",
					adslMib.adslStat.ginpStat.cntDS.minEFTR, adslMib.adslStat.ginpStat.cntUS.minEFTR,
					adslMib.adslStat.ginpStat.cntDS.errFreeBits, adslMib.adslStat.ginpStat.cntUS.errFreeBits);
#ifdef SUPPORT_DSL_GFAST
				if(kXdslModGfast == adslMib.adslConnection.modType) {
					gfastOlrCounters *pRxCnt = &adslMib.gfastOlrXoiCounterData[0].cntDS.perfSinceShowTime;
					gfastOlrCounters *pTxCnt = &adslMib.gfastOlrXoiCounterData[0].cntUS.perfSinceShowTime;
					gfastOlrCounters *pDoiRxCnt = &adslMib.gfastOlrXoiCounterData[1].cntDS.perfSinceShowTime;
					gfastOlrCounters *pDoiTxCnt = &adslMib.gfastOlrXoiCounterData[1].cntUS.perfSinceShowTime;
					if (adslMib.gfastDta.dtaFlags != 0)
					  printf(
					    "dtaActRate:\t%u\t\t%u\n"
						"MaxM(ds/us):\t%u\t\t%u\n", adslMib.gfastDta.dsCurRate, adslMib.gfastDta.usCurRate, 
						adslMib.gfastDta.maxMds, adslMib.gfastDta.maxMds);
					printf(
						"\n"
						"NOI\n"
						"BSW:\t\t%u/%u\t\t%u/%u\n"
						"SRA:\t\t%u/%u\t\t%u/%u\n"
						"FRA:\t\t%u/%u\t\t%u/%u\n"
						"RPA:\t\t%u/%u\t\t%u/%u\n"
						"TIGA:\t\t%u/%u\t\t%u/%u\n"
						"DOI\n"
						"BSW:\t\t%u/%u\t\t%u/%u\n"
						"SRA:\t\t%u/%u\t\t%u/%u\n"
						"FRA:\t\t%u/%u\t\t%u/%u\n"
						"RPA:\t\t%u/%u\t\t%u/%u\n"
						"TIGA:\t\t%u/%u\t\t%u/%u\n\n"
						"eocBytes:\t%u\t\t%u\n"
						"eocPkts:\t%u\t\t%u\n"
						"eocMsgs:\t%u\t\t%u\n\n"
						"ANDEFTRmin:\t%u\t\t%u\n"
						"ANDEFTRmax:\t%u\t\t%u\n"
						"ANDEFTRsum:\t%u\t\t%u\n"
						"ANDEFTRDS:\t%u\t\t%u\n"
						"LANDEFTRRS:\t%u\t\t%u\n",
						pRxCnt->bswCompleted, pRxCnt->bswStarted, pTxCnt->bswCompleted, pTxCnt->bswStarted,
						pRxCnt->sraCompleted, pRxCnt->sraStarted, pTxCnt->sraCompleted, pTxCnt->sraStarted,
						pRxCnt->fraCompleted, pRxCnt->fraStarted, pTxCnt->fraCompleted, pTxCnt->fraStarted,
						pRxCnt->rpaCompleted, pRxCnt->rpaStarted, pTxCnt->rpaCompleted, pTxCnt->rpaStarted,
						pRxCnt->tigaCompleted, pRxCnt->tigaStarted, pTxCnt->tigaCompleted, pTxCnt->tigaStarted,
						pDoiRxCnt->bswCompleted, pDoiRxCnt->bswStarted, pDoiTxCnt->bswCompleted, pDoiTxCnt->bswStarted,
						pDoiRxCnt->sraCompleted, pDoiRxCnt->sraStarted, pDoiTxCnt->sraCompleted, pDoiTxCnt->sraStarted,
						pDoiRxCnt->fraCompleted, pDoiRxCnt->fraStarted, pDoiTxCnt->fraCompleted, pDoiTxCnt->fraStarted,
						pDoiRxCnt->rpaCompleted, pDoiRxCnt->rpaStarted, pDoiTxCnt->rpaCompleted, pDoiTxCnt->rpaStarted,
						pDoiRxCnt->tigaCompleted, pDoiRxCnt->tigaStarted, pDoiTxCnt->tigaCompleted, pDoiTxCnt->tigaStarted,
						adslMib.adslStat.eocStat.bytesReceived, adslMib.adslStat.eocStat.bytesSent,
						adslMib.adslStat.eocStat.packetsReceived, adslMib.adslStat.eocStat.packetsSent,
						adslMib.adslStat.eocStat.messagesReceived, adslMib.adslStat.eocStat.messagesSent,
						adslMib.adslStat.gfastStat.rxANDEFTRmin, adslMib.adslStat.gfastStat.txANDEFTRmin/1000,
						adslMib.adslStat.gfastStat.rxANDEFTRmax, (adslMib.adslStat.gfastStat.txANDEFTRmax+999)/1000,
						adslMib.adslStat.gfastStat.rxANDEFTRsum, adslMib.adslStat.gfastStat.txANDEFTRsum,
						adslMib.adslStat.gfastStat.rxANDEFTRDS, adslMib.adslStat.gfastStat.txANDEFTRDS,
						adslMib.adslStat.gfastStat.rxLANDEFTRS, adslMib.adslStat.gfastStat.txLANDEFTRS);
				}
#endif
			}
			else
			/* 1 - kFireDsEnabled, 2 -kFireUsEnabled */
			if(adslMib.xdslStat[0].fireStat.status & 0x3) {
				printf (
					"\n"
					"ReXmt:\t\t%u\t\t%u\n"
					"ReXmtCorr:\t%u\t\t%u\n"
					"ReXmtUnCorr:\t%u\t\t%u\n",
					(unsigned int)adslMib.adslStat.fireStat.reXmtRSCodewordsRcved, (unsigned int)adslMib.adslStat.fireStat.reXmtRSCodewordsRcvedUS,
					(unsigned int)adslMib.adslStat.fireStat.reXmtCorrectedRSCodewords, (unsigned int)adslMib.adslStat.fireStat.reXmtCorrectedRSCodewordsUS,
					(unsigned int)adslMib.adslStat.fireStat.reXmtUncorrectedRSCodewords, (unsigned int)adslMib.adslStat.fireStat.reXmtUncorrectedRSCodewordsUS);
			}

			for(pathId = 0; pathId < MAX_LP_NUM; pathId++) {
				if((0==pathId) || (adslMib.lp2Active || adslMib.lp2TxActive) ) {
					printf (
					"\n"
					"\t\t\tBearer %d\n"
					"HEC:\t\t%u\t\t%u\n"
					"OCD:\t\t%u\t\t%u\n"
					"LCD:\t\t%u\t\t%u\n"
					"Total Cells:\t%u\t\t%u\n"
					"Data Cells:\t%u\t\t%u\n"
					"Drop Cells:\t%u\n"
					"Bit Errors:\t%u\t\t%u\n",
					pathId,
					(unsigned int)adslMib.atmStat2lp[pathId].rcvStat.cntHEC, (unsigned int)adslMib.atmStat2lp[pathId].xmtStat.cntHEC,
					(unsigned int)adslMib.atmStat2lp[pathId].rcvStat.cntOCD, (unsigned int)adslMib.atmStat2lp[pathId].xmtStat.cntOCD,
					(unsigned int)adslMib.atmStat2lp[pathId].rcvStat.cntLCD, (unsigned int)adslMib.atmStat2lp[pathId].xmtStat.cntLCD,
					(unsigned int)adslMib.atmStat2lp[pathId].rcvStat.cntCellTotal, (unsigned int)adslMib.atmStat2lp[pathId].xmtStat.cntCellTotal,
					(unsigned int)adslMib.atmStat2lp[pathId].rcvStat.cntCellData, (unsigned int)adslMib.atmStat2lp[pathId].xmtStat.cntCellData,
					(unsigned int)adslMib.atmStat2lp[pathId].rcvStat.cntCellDrop,
					(unsigned int)adslMib.atmStat2lp[pathId].rcvStat.cntBitErrs, (unsigned int)adslMib.atmStat2lp[pathId].xmtStat.cntBitErrs);
				}
			}
#ifdef SUPPORT_DSL_GFAST
			if(kXdslModGfast == adslMib.adslConnection.modType)
				printf (
					"\nLORS:\t\t%u\t\t%u"
					"\nLOSS:\t\t%u\t\t%u",
					(unsigned int)adslMib.adslPerfData.perfTotal.xdslLORS,(unsigned int) adslMib.adslTxPerfTotal.xdslLORS,
					(unsigned int)adslMib.adslPerfData.perfTotal.adslLOSS,(unsigned int) adslMib.adslTxPerfTotal.adslLOSS);
#endif
			printf (
				"\n"
				"ES:\t\t%u\t\t%u\n"
				"SES:\t\t%u\t\t%u\n"
				"UAS:\t\t%u\t\t%u\n"
				"AS:\t\t%u\n"
				"\n",
				(unsigned int)adslMib.adslPerfData.perfTotal.adslESs, (unsigned int)adslMib.adslTxPerfTotal.adslESs,
				(unsigned int)adslMib.adslPerfData.perfTotal.adslSES,(unsigned int) adslMib.adslTxPerfTotal.adslSES,
				(unsigned int)adslMib.adslPerfData.perfTotal.adslUAS, (unsigned int)adslMib.adslTxPerfTotal.adslUAS,
				(unsigned int)adslMib.adslPerfData.perfSinceShowTime.adslAS);
			
			for(pathId = 0; pathId < MAX_LP_NUM; pathId++) {
				if((0==pathId) || (adslMib.lp2Active || adslMib.lp2TxActive) ) {
					pRxFramingParam = &adslMib.xdslInfo.dirInfo[0].lpInfo[pathId];
					pTxFramingParam = &adslMib.xdslInfo.dirInfo[1].lpInfo[pathId];
					printf(
						"\t\t\tBearer %d\n"
						"INP:\t\t%4.2f\t\t%4.2f\n"
						"INPRein:\t%4.2f\t\t%4.2f\n"
						"delay:\t\t%d\t\t%d\n"
						"PER:\t\t%d.%02d\t\t%d.%02d\n"
						"OR:\t\t%d.%02d\t\t%d.%02d\n"
						"AgR:\t\t%d.%02d\t%d.%02d\n"
						"\n",
						pathId,
						(float)pRxFramingParam->INP/2, (float)pTxFramingParam->INP/2,
						(float)pRxFramingParam->INPrein/2, (float)pTxFramingParam->INPrein/2,
						pRxFramingParam->delay, pTxFramingParam->delay,
						_f2DecI(GetXdsl2PERp(modType, vdslProf, pRxFramingParam,100),100), _f2DecF(GetXdsl2PERp(modType, vdslProf, pRxFramingParam,100),100),
						_f2DecI(GetXdsl2PERp(modType, vdslProf, pTxFramingParam,100),100), _f2DecF(GetXdsl2PERp(modType, vdslProf, pTxFramingParam,100),100),
						_f2DecI(GetXdsl2ORp(modType, vdslProf, pRxFramingParam,100),100), _f2DecF(GetXdsl2ORp(modType, vdslProf, pRxFramingParam,100),100),
						_f2DecI(GetXdsl2ORp(modType, vdslProf, pTxFramingParam,100),100), _f2DecF(GetXdsl2ORp(modType, vdslProf, pTxFramingParam,100),100),
						_f2DecI(GetXdsl2AgR(modType, vdslProf, pRxFramingParam,100),100), _f2DecF(GetXdsl2AgR(modType, vdslProf, pRxFramingParam,100),100),
						_f2DecI(GetXdsl2AgR(modType, vdslProf, pTxFramingParam,100),100), _f2DecF(GetXdsl2AgR(modType, vdslProf, pTxFramingParam,100),100)
						);
				}
			}
#ifdef SUPPORT_DSL_GFAST
			if(kXdslModGfast != adslMib.adslConnection.modType)
#endif
			printf("Bitswap:\t%u/%u\t\t%u/%u\n\n", (unsigned int)adslMib.adslStat.bitswapStat.rcvCnt, (unsigned int)adslMib.adslStat.bitswapStat.rcvCntReq,
				(unsigned int)adslMib.adslStat.bitswapStat.xmtCnt, (unsigned int)adslMib.adslStat.bitswapStat.xmtCntReq);
		}
	}

	if (infoType >= 3) {
		adslChanPerfDataEntry	*pChanPerfData;
		adslChanCounters		*pChanPerfIntervals;
		if(!adslMib.lp2Active && !adslMib.lp2TxActive) {
			if (kAdslIntlChannel == adslMib.adslConnection.chType) {
				pChanPerfData = &adslMib.adslChanIntlPerfData;
				pChanPerfIntervals = adslMib.adslChanIntlPerfIntervals;
			}
			else {
				pChanPerfData = &adslMib.adslChanFastPerfData;
				pChanPerfIntervals = adslMib.adslChanFastPerfIntervals;
			}
		}
		else {
			/* path/bear 0 for G.inp case */
			pChanPerfData = &adslMib.xdslChanPerfData[0];
			pChanPerfIntervals = &adslMib.xdslChanPerfIntervals[0][0];
		}
		PrintPerformanceData ("Total",
			adslMib.adslPerfData.adslSinceDrvStartedTimeElapsed,
			&adslMib.adslPerfData.perfTotal, &adslMib.adslTxPerfTotal,
			&pChanPerfData->perfTotal, &adslMib.adslPerfData.failTotal);
		PrintPerformanceData ("Latest 15 minutes",
			adslMib.adslPerfData.adslPerfCurr15MinTimeElapsed,
			&adslMib.adslPerfData.perfCurr15Min, &adslMib.adslTxPerfCur15Min,
			&pChanPerfData->perfCurr15Min, &adslMib.adslPerfData.failCur15Min);
		if (adslMib.adslPerfData.adslPerfValidIntervals > 0)
			PrintPerformanceData ("Previous 15 minutes", 15 * 60, &adslMib.adslPerfIntervals[0], &adslMib.adslTxPerfLast15Min, &pChanPerfIntervals[0], NULL);
		else /* print zeros */
			PrintPerformanceData ("Previous 15 minutes", 0, &adslMib.adslPerfData.perfPrev1Day, &adslMib.adslTxPerfLast1Day, &pChanPerfData->perfPrev1Day, NULL);

		PrintPerformanceData ("Latest 1 day",
			adslMib.adslPerfData.adslPerfCurr1DayTimeElapsed, 
			&adslMib.adslPerfData.perfCurr1Day, &adslMib.adslTxPerfCur1Day,
			&pChanPerfData->perfCurr1Day, &adslMib.adslPerfData.failCurDay);
		PrintPerformanceData ("Previous 1 day",
			adslMib.adslPerfData.adslAturPerfPrev1DayMoniSecs, 
			&adslMib.adslPerfData.perfPrev1Day, &adslMib.adslTxPerfLast1Day,
			&pChanPerfData->perfPrev1Day, &adslMib.adslPerfData.failPrevDay);

		{
		adslChanCounters	linkupCnt;

		linkupCnt.adslChanCorrectedBlks = adslMib.adslStat.rcvStat.cntRSCor;
		linkupCnt.adslChanUncorrectBlks = adslMib.adslStat.rcvStat.cntSFErr;
		linkupCnt.adslChanTxFEC         = adslMib.adslStat.xmtStat.cntRSCor;
		linkupCnt.adslChanTxCRC         = adslMib.adslStat.xmtStat.cntSFErr;

		PrintPerformanceData ("Since Link",
			adslMib.adslPerfData.adslSinceLinkTimeElapsed,
			&adslMib.adslPerfData.perfSinceShowTime, &adslMib.adslTxPerfSinceShowTime,
			&linkupCnt, &adslMib.adslPerfData.failSinceShowTime);
		}
		
#ifdef NTR_SUPPORT
		printf ("NTR: mipsCntAtNtr=%u ncoCntAtNtr=%u\n", adslMib.ntrCnt.mipsCntAtNtr, adslMib.ntrCnt.ncoCntAtNtr);
#endif
	}
	return nRet;
}

/***************************************************************************
 * Function Name: DiagHandler
 * Description  : Processes the adslctl diag command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int DiagHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
   int nRet = 0;
   CmsRet cmsRet = CMSRET_INTERNAL_ERROR;
   char *pCmd;
   int i, n;

   pOptions--;
   while ((0 == nRet) && (nNumOptions-- > 0)) {
      pOptions++;
      if (0 == strcmp(pOptions->pszOptName, "--cmd")) {
         unsigned char  diagCmd[1000], *pDiagCmd;
         
         if (0 == pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         diagCmd[0] = 0;
         diagCmd[1] = 0;
         diagCmd[2] = (DIAG_DSL_CLIENT << DIAG_PARTY_TYPE_SEND_SHIFT) & 0xFF;
         for (i = 0; i < pOptions->nNumParms; i++) {
            pCmd = pOptions->pszParms[i];
            if ((pCmd[0] == '0') && ((pCmd[1] == 'x') || (pCmd[1] == 'X')))
               pCmd += 2;
            pDiagCmd = diagCmd+3;
            while (*pCmd != 0) {
               n = GetMapByte(pCmd);
               if (n < 0)
                  return ADSLCTL_INVALID_PARAMETER;
               *pDiagCmd++ = n & 0xFF;
               pCmd += 2;
            }
            cmsRet = xdslCtl_DiagProcessCommandFrame(lineId, diagCmd, pDiagCmd - diagCmd);
            if (cmsRet != CMSRET_SUCCESS) {
               fprintf( stderr, "%s: xdslCtl_DiagProcessCommandFrame error\n", g_szPgmName );
               return (ADSLCTL_GENERAL_ERROR);
            }
         }
      }
#ifdef SUPPORT_MULTI_PHY
      else if(0 == strcmp(pOptions->pszOptName, "--mediaSearchCfg")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         n = strtol(pOptions->pszParms[0], NULL, 0);
         cmsRet = xdslCtl_DiagProcessDbgCommand(
                                               lineId,
                                                38,   /* DIAG_DEBUG_CMD_MEDIASEARCH_CFG */
                                                0,
                                                n,
                                                0);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: xdslCtl_DiagProcessDbgCommand --mediaSearchCfg Error\n", g_szPgmName );
            return (ADSLCTL_GENERAL_ERROR);
         }
      }
#endif
#if defined(SUPPORT_DSL_GFAST)
      else if(0 == strcmp(pOptions->pszOptName, "--phyTypeCfg")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         n = strtol(pOptions->pszParms[0], NULL, 0);
         cmsRet = xdslCtl_DiagProcessDbgCommand(
                                               lineId,
                                                48,   /* DIAG_DEBUG_CMD_PHY_TYPE_CFG */
                                                0,
                                                n,
                                                0);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: xdslCtl_DiagProcessDbgCommand --phyTypeCfg Error\n", g_szPgmName );
            return (ADSLCTL_GENERAL_ERROR);
         }
      }
#endif
      else if(0 == strcmp(pOptions->pszOptName, "--logstart")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         n = strtol(pOptions->pszParms[0], NULL, 0);
         cmsRet = xdslCtl_DiagProcessDbgCommand(
                                               lineId,
                                                25,   /* DIAG_DEBUG_CMD_STAT_SAVE_LOCAL */
                                                0,
                                                6,    /* log continously */
                                                n);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: xdslCtl_DiagProcessDbgCommand --logstart Error\n", g_szPgmName );
            return (ADSLCTL_GENERAL_ERROR);
         }
      }
      else if(0 == strcmp(pOptions->pszOptName, "--logstop")) {
         cmsRet = xdslCtl_DiagProcessDbgCommand(
                                               lineId,
                                                25,   /* DIAG_DEBUG_CMD_STAT_SAVE_LOCAL */
                                                0,
                                                7,    /* log stop */
                                                0);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: xdslCtl_DiagProcessDbgCommand --logstop Error\n", g_szPgmName );
            return (ADSLCTL_GENERAL_ERROR);
         }
      }
      else if(0 == strcmp(pOptions->pszOptName, "--loguntilbufferfull")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         n = strtol(pOptions->pszParms[0], NULL, 0);
         cmsRet = xdslCtl_DiagProcessDbgCommand(
                                               lineId,
                                                25,   /* DIAG_DEBUG_CMD_STAT_SAVE_LOCAL */
                                                0,
                                                8,    /* log until buffer is full */
                                                n);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: xdslCtl_DiagProcessDbgCommand --loguntilbufferfull Error\n", g_szPgmName );
            return (ADSLCTL_GENERAL_ERROR);
         }
      }
      else if(0 == strcmp(pOptions->pszOptName, "--loguntilretrain")) {
         if (1 != pOptions->nNumParms) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         n = strtol(pOptions->pszParms[0], NULL, 0);
         cmsRet = xdslCtl_DiagProcessDbgCommand(
                                               lineId,
                                                25,   /* DIAG_DEBUG_CMD_STAT_SAVE_LOCAL */
                                                0,
                                                9,    /* log until retrain */
                                                n);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: xdslCtl_DiagProcessDbgCommand --loguntilretrain Error\n", g_szPgmName );
            return (ADSLCTL_GENERAL_ERROR);
         }
      }
      else if(0 == strcmp(pOptions->pszOptName, "--logpause")) {
         cmsRet = xdslCtl_DiagProcessDbgCommand(
                                                lineId,
                                                DIAG_DEBUG_CMD_STAT_SAVE_LOCAL,
                                                0,
                                                10,    /* pause log */
                                                0);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: xdslCtl_DiagProcessDbgCommand --logpause Error\n", g_szPgmName );
            return (ADSLCTL_GENERAL_ERROR);
         }
      }
      else if(0 == strcmp(pOptions->pszOptName, "--dumpBuf")) {
		 int  mode = 0;
         if ((pOptions->nNumParms < 1) || (pOptions->nNumParms > 2)) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         n = strtol(pOptions->pszParms[0], NULL, 0);  /* dump buffer size in Kb */

         if (pOptions->nNumParms >= 2) {
			mode = strtol(pOptions->pszParms[1], NULL, 0);  /* dumpBuf mode */
         }

         cmsRet = xdslCtl_DiagProcessDbgCommand(
                                               lineId,
                                                DIAG_DEBUG_CMD_DUMPBUF_CFG,
                                                0,
                                                n,    /* log continously */
                                                mode);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: xdslCtl_DiagProcessDbgCommand --dumpBuf Error\n", g_szPgmName );
            return (ADSLCTL_GENERAL_ERROR);
         }
      }
      else if(0 == strcmp(pOptions->pszOptName, "--dbgcmd")) {
         UINT32 param[4] = {0,0,0,0};
         if ((pOptions->nNumParms < 1) || (pOptions->nNumParms > 4)) {
            nRet = ADSLCTL_INVALID_PARAMETER;
            break;
         }
         for(i = 0; i < pOptions->nNumParms; i++) {
            if(i < 2)
               param[i] = strtoul(pOptions->pszParms[i], NULL, 0);
            else
               param[i] = strtol(pOptions->pszParms[i], NULL, 0);
         }
         cmsRet = xdslCtl_DiagProcessDbgCommand(lineId, (unsigned short)param[0], (unsigned short)param[1], param[2], param[3]);
         if (cmsRet != CMSRET_SUCCESS) {
            fprintf( stderr, "%s: xdslCtl_DiagProcessDbgCommand --dbgcmd Error\n", g_szPgmName );
            return (ADSLCTL_GENERAL_ERROR);
         }
      }
      else
         nRet = ADSLCTL_INVALID_OPTION;
   }

   if (ADSLCTL_INVALID_PARAMETER == nRet) {
      fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName,
            pOptions->pszOptName);
      return nRet;
   }
   if (ADSLCTL_INVALID_OPTION == nRet) {
      fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);
      return nRet;
   }

   return nRet;
}
#define	SIGN_MASK			0x80000000
int GetInteger(char **sPtrPtr, int *outValue, int base)
{
	char	*sPtr = *sPtrPtr;
	
	while ((*sPtr == ' ') || (*sPtr == '\t')) 
		sPtr++;
	
	*sPtrPtr = sPtr;

	if (base & SIGN_MASK) {
		base &= ~SIGN_MASK;
		*outValue = strtoul (sPtr, sPtrPtr, base);
	}
	else
		*outValue = strtol (sPtr, sPtrPtr, base);

	if (sPtr == *sPtrPtr)	return 0;
	return 1;
}
void SetTones(char *pToneMap, int tone1, int tone2, int maxTone)
{
	int		i;

	if ((tone1 > maxTone) || (tone1 > tone2))
		return;
	if (tone2 > maxTone)
		tone2 = maxTone;

	for (i = tone1; i < (tone2+1); i++)
		pToneMap[i>>3] |= (1 << (i & 7));
}
char *SkipWhitespace(char *s)
{
	while ((' ' == *s) || ('\t' == *s))
		s++;
	return s;
}
static int GetSnrClampShape(char *ppStr, char *snrClampShape, int snrClampSize)
{
    char *p = ppStr;
    int i0=0, lastBpIndex = 0, lastBpLevel = -1;
    int		n1, n2;

    fprintf( stderr, "\t\tGenerating SNR Clamp from BP list, expectedSize=%d\n", snrClampSize);
    p = SkipWhitespace(p);
    do {

        while (',' == *p) p = SkipWhitespace(p+1);

        fprintf( stderr, "\t\tskipped some spaces, now at 0x%x, offset=%td\n", *p, p-ppStr);

        if (!GetInteger(&p, &n1,0))
        {
                fprintf( stderr, "\t\tcannot get an integer from 0x%x, n1=%d, offset=%td\n", *p, n1,p-ppStr);
                break;
        }
        p = SkipWhitespace(p);
		if ('-' == *p) {
			p++;
			p = SkipWhitespace(p);
			if (!GetInteger(&p, &n2,0))
				break;
			p = SkipWhitespace(p+1);
            if (lastBpLevel==-1){
                    lastBpLevel=n2<<1;
            }
            fprintf( stderr, "\t\tBP %d - %d, lastBpIndex=%d, offset=%td\n", n1, n2, lastBpIndex,p-ppStr);
            if (n1>lastBpIndex) {
                while (i0<=MIN(n1,snrClampSize-1)) {
                    snrClampShape[i0] = lastBpLevel + (i0-lastBpIndex)*((n2<<1)-lastBpLevel)/(n1-lastBpIndex);
                    i0++;
                }
            }
            else if (n1>snrClampSize)
            {
                fprintf( stderr, "\t\tn1=%d > clampSize=%d\n", n1, snrClampSize );
                    break;
            }
		   }
		else if (('\0' == *p) || ('\n' == *p) || (';' == *p)){
                fprintf( stderr, "\t\tline end, i0=%d\n", i0);
                if (i0>0) {
                    fprintf( stderr, "\t\tfinishing shape from %d to %d\n", i0, snrClampSize );
                    while(i0<snrClampSize) {
                        snrClampShape[i0++] = lastBpLevel;
                    }
                break; 
                }
            }
		else
        {
            fprintf( stderr, "\t\tunexpected char: [%c]\n", *p );
            break;
        }
    } while (1);

    fprintf( stderr, "\t\treturning %d elements after processing %td char\n", i0, p-ppStr );

    return i0;
}
static int GetToneMap(char *ppStr, char *pToneMap, int toneMapSize)
{
	char	*p = ppStr;
	int		n1, n2;
	int     bToneSet;

	
	p = SkipWhitespace(p);
	bToneSet = 1;
	memset (pToneMap, 0, toneMapSize);
	toneMapSize = (toneMapSize << 3) - 1;
	do {
		
		while (',' == *p) p = SkipWhitespace(p+1);
		 
		
		if (!GetInteger(&p, &n1,0))
			break;
		p = SkipWhitespace(p);
		if ('-' == *p) {
			p++;
			p = SkipWhitespace(p);
			if (!GetInteger(&p, &n2,0))
				break;
			p = SkipWhitespace(p);
			SetTones(pToneMap, n1, n2, toneMapSize);
		}
		else if ((',' == *p))
			SetTones(pToneMap, n1, n1, toneMapSize);
		else if (('\0' == *p) || ('\n' == *p)){
			SetTones(pToneMap, n1, n1, toneMapSize);
			break;}
		else
			break;

	} while (1);
	return bToneSet;
}
/***************************************************************************
 * Function Name: AfelbHandler
 * Description  : Processes the adslctl afelb command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int AfelbHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
	CmsRet cmsRet = CMSRET_SUCCESS;
	int nRet=0;
	unsigned char	diagCmd[1000], *pDiagCmd;
	SINT32		n;
	
	diagCmd[0] = 0;
	diagCmd[1] = 0;
	diagCmd[2] = (DIAG_DSL_CLIENT << DIAG_PARTY_TYPE_SEND_SHIFT) & 0xFF;
	pOptions--;
	while ((0 == nRet) && (nNumOptions-- > 0)) {
		pOptions++;
		if (0 == strcmp(pOptions->pszOptName, "--time")) {
			
			if (1 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			pDiagCmd=diagCmd+3;
			*pDiagCmd++=50;
			n=strtol(pOptions->pszParms[0], NULL, 10);
			memcpy(pDiagCmd,&n,sizeof(SINT32));
			pDiagCmd+=sizeof(SINT32);
			cmsRet = xdslCtl_DiagProcessCommandFrame(lineId, diagCmd, pDiagCmd - diagCmd);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: BcmAdsl_DiagProcessCommandFrame error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
		}
		else if (0 == strcmp(pOptions->pszOptName, "--tones")) {
			int		xmtStart=0, xmtNum=32, rcvStart=32,rcvNum=4;
			char	*ToneMap = (char *)&diagCmd[0];
			
			if (1 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			GetToneMap(pOptions->pszParms[0], ToneMap, 512/8);
			cmsRet = xdslCtl_SelectTones(lineId, xmtStart,xmtNum,rcvStart,rcvNum,ToneMap, ToneMap+4);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: BcmAdsl_DiagProcessCommandFrame error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
		}
		else if (0 == strcmp(pOptions->pszOptName, "--signal")) {
			
			if (1 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			pDiagCmd=diagCmd+3;
			*pDiagCmd++=23;
			n=strtol(pOptions->pszParms[0], NULL, 10);
			memcpy(pDiagCmd,&n,sizeof(SINT32));
			pDiagCmd+=sizeof(SINT32);
			cmsRet = xdslCtl_DiagProcessCommandFrame(lineId, diagCmd, pDiagCmd - diagCmd);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslDiagProcessCommandFrame error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
		}
		else
			nRet = ADSLCTL_INVALID_OPTION;
	}

	if (ADSLCTL_INVALID_PARAMETER == nRet) {
		fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName, 	pOptions->pszOptName);
		return nRet;
	}
	
	if (ADSLCTL_INVALID_OPTION == nRet) {
		fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);
		return nRet;
	}

	return nRet;
}

/***************************************************************************
 * Function Name: NonLinCmdHandler
 * Description  : Processes the adslctl nonlin command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int NonLinCmdHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
	long		dataLen;
	unsigned	short n;
	CmsRet	cmsRet=CMSRET_SUCCESS;
	int		nRet = 0;
	pOptions--;
	while ((0 == nRet) && (nNumOptions-- > 0)) {
		pOptions++;
		if (0 == strcmp(pOptions->pszOptName, "--setThld")) {
			char  oidStr[] = { kOidAdslPrivate, kOidAdslPrivExtraInfo, kOidAdslPrivNonLinThldNumBins };
			unsigned char	dataBuf[2];
			
			if (1 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			n=(unsigned short)strtol(pOptions->pszParms[0], NULL, 10);
			if (n>480)
			{
				fprintf( stderr, "%s: Invalid Threshold value not in [0 to 480]\n",g_szPgmName);
				return ADSLCTL_GENERAL_ERROR;
			}
			memcpy(&dataBuf[0],&n,sizeof(unsigned short));
			dataLen=2;
			cmsRet=xdslCtl_SetObjectValue(lineId, oidStr,3,(char*)dataBuf,&dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslSetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
		}
		else if (0 == strcmp(pOptions->pszOptName, "--show")) {
			char  oidStr[] = { kOidAdslPrivate, kOidAdslPrivExtraInfo, kOidAdslExtraNLInfo};
			adslNonLinearityData	adslNonLinData;
			dataLen=sizeof(adslNonLinData);
			cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, 3, (char *)(&adslNonLinData), &dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			fprintf(stderr,"NonLinearityFlag %d \nNumberOfAffectedBins %d \nThresholdNumberOfBins %d\nEcho-to-Noise Ratio (ENR) %d \n",adslNonLinData.NonLinearityFlag, adslNonLinData.NonLinNumAffectedBins, adslNonLinData.NonLinThldNumAffectedBins, adslNonLinData.NonLinDbEcho);
		}
		else
			nRet = ADSLCTL_INVALID_OPTION;
	}
	if (ADSLCTL_INVALID_PARAMETER == nRet)
		fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName,pOptions->pszOptName);
	else if (ADSLCTL_INVALID_OPTION == nRet)
		fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);
	return nRet;
}

/***************************************************************************
 * Function Name: INMHandler
 * Description  : Processes the adslctl inm command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
 static int INMHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
	long		dataLen;
	CmsRet	cmsRet=CMSRET_SUCCESS;
	int		nRet = 0;
	pOptions--;
	while ((0 == nRet) && (nNumOptions-- > 0)) {
		pOptions++;
		if (0 == strcmp(pOptions->pszOptName, "--start")) {
			char  oidStr[] = { kOidAdslPrivate, kOidAdslPrivExtraInfo, kOidAdslExtraPLNData, 0 };
			unsigned char	dataBuf[1000];
			ushort INMIATO,INMIATS,INMCC,INM_INPEQ_MODE,INM_INPEQ_FORMAT,INMIATS_actual; int i,j,numParams;short n;
			int inpBinIntervalLogFormat[17]={1,2,3,4,6,8,12,16,22,29,39,53,71,94,126,168,169};
			adslMibInfo adslMib;
			dataLen=sizeof(adslMibInfo);
			cmsRet = xdslCtl_GetObjectValue(lineId, NULL, 0, (char *)&adslMib, &dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			if(adslMib.adslTrainingState!=kAdslTrainingConnected)
			{
				fprintf( stderr, "%s: Cannot start inm in current state - line not in Showtime\n",g_szPgmName);
				return ADSLCTL_GENERAL_ERROR;
			}
			if(adslMib.adslConnection.modType != kAdslModAdsl2 && adslMib.adslConnection.modType != kAdslModAdsl2p && adslMib.adslConnection.modType != kAdslModReAdsl2)
			{
				fprintf( stderr, "%s: Cannot start inm in current state - modulation is not ADSL2/2p\n",g_szPgmName);
				return ADSLCTL_GENERAL_ERROR;
			}
			j=0;
			numParams=5;
			if (numParams != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			INMIATO=(short)strtol(pOptions->pszParms[j++], NULL, 10);
			INMIATS=(short)strtol(pOptions->pszParms[j++], NULL, 10);
			INMCC=(short)strtol(pOptions->pszParms[j++], NULL, 10);
			INM_INPEQ_MODE=(short)strtol(pOptions->pszParms[j++], NULL, 10);
			INM_INPEQ_FORMAT=(short)strtol(pOptions->pszParms[j++], NULL, 10);
			if (INMIATO <3 || INMIATO >511)
			{
				fprintf( stderr, "%s: Invalid INMIATO value not in [3-511]\n",g_szPgmName);
				return ADSLCTL_GENERAL_ERROR;
			}
			
			if ( INMIATS > 7 )
			{
				fprintf( stderr, "%s: Invalid INMIATS value not in [0-7]\n",g_szPgmName);
				return ADSLCTL_GENERAL_ERROR;
			}
			if (INMCC>64)
			{
				fprintf( stderr, "%s: Invalid INMCC value not in [0-64]\n",g_szPgmName);
				return ADSLCTL_GENERAL_ERROR;
			}			    
			if (INM_INPEQ_MODE>3)
			{
				fprintf( stderr, "%s: Invalid INM_INPEQ_MODE value not in [0-3]\n",g_szPgmName);
				return ADSLCTL_GENERAL_ERROR;
			}			    
			if (INM_INPEQ_MODE==0 && INMCC!=0)
			{
				fprintf( stderr, "%s: Invalid INMCC value not 0 for INM_INPEQ_MODE=0\n",g_szPgmName);
				return ADSLCTL_GENERAL_ERROR;
			}
			if (INM_INPEQ_FORMAT!=0 && INM_INPEQ_FORMAT!=1)
			{
				fprintf( stderr, "%s: Invalid INM_INPEQ_FORMAT not in [0-1]\n",g_szPgmName);
				return ADSLCTL_GENERAL_ERROR;
			}
			oidStr[3]=kOidAdslExtraPLNDataNbDurBins;
			dataBuf[0]=17;
			dataLen=1;
			cmsRet=xdslCtl_SetObjectValue(lineId, oidStr,4,(char*)dataBuf,&dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslSetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			for (i=0;i<17;i++)
			{
				dataBuf[2*i]=0;
				if(INM_INPEQ_FORMAT==1)
					dataBuf[2*i+1]=inpBinIntervalLogFormat[i];
				else
					dataBuf[2*i+1]=i+1;
			}
			dataLen=34;
			oidStr[2]=kOidAdslPrivPLNDurationBins;
			xdslCtl_SetObjectValue(lineId, oidStr,3,(char*)dataBuf,&dataLen);
			oidStr[2]=kOidAdslExtraPLNData;
			oidStr[3]=kOidAdslExtraPLNDataNbIntArrBins;
			dataBuf[0]=8;
			dataLen=1;
			cmsRet=xdslCtl_SetObjectValue(lineId, oidStr,4,(char*)dataBuf,&dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslSetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			n=2;
			memcpy(&dataBuf[0],&n,sizeof(short));
			n=1;
			INMIATS_actual=(n<<INMIATS);
			for (i=0;i<7;i++)
			{
				n=INMIATO+i*INMIATS_actual;
				memcpy(&dataBuf[2*i+2],&n,sizeof(short));
			}
			dataLen=16;
			oidStr[2]=kOidAdslPrivPLNIntrArvlBins;
			cmsRet=xdslCtl_SetObjectValue(lineId, oidStr,3,(char*)dataBuf,&dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslSetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}

			dataBuf[0]=INM_INPEQ_FORMAT;
			dataLen=1;
			oidStr[2]=kOidAdslPrivINMConfigFormat;
			cmsRet=xdslCtl_SetObjectValue(lineId, oidStr,3,(char*)dataBuf,&dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslSetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			dataBuf[0]=INMCC;
			dataBuf[1]=INM_INPEQ_MODE;
			dataBuf[2]=INMIATO;
			dataBuf[3]=INMIATS;
			dataLen=4;
			oidStr[2]=kOidAdslPrivINMControlParams;
			cmsRet=xdslCtl_SetObjectValue(lineId, oidStr,3,(char*)dataBuf,&dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslSetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			}
		else if (0 == strcmp(pOptions->pszOptName, "--stop")) {
			char  oidStr[] = { kOidAdslPrivate, kOidAdslPrivExtraInfo,kOidAdslExtraPLNInfo,kOidAdslExtraPLNDataPLNState };
			unsigned char	dataBuf[1];
			adslPLNDataEntry adslPLNData;
			dataLen=sizeof(adslPLNDataEntry);
			cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, 3, (char *)(&adslPLNData), &dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			if(adslPLNData.PLNState!=1)
			{
				fprintf( stderr, "%s: INM currently not running\n",g_szPgmName);
				return ADSLCTL_GENERAL_ERROR;
			}
			dataBuf[0]=3;
			oidStr[2]=kOidAdslExtraPLNData;
			dataLen=1;
			cmsRet=xdslCtl_SetObjectValue(lineId, oidStr,4,(char*)dataBuf,&dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslSetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			}
		else if (0 == strcmp(pOptions->pszOptName, "--show")) {
			adslPLNDataEntry adslPLNData;unsigned char	dataBuf[1];
			ushort INMIATO,INMIATS;
			int i=0;
			char  oidStr[] = { kOidAdslPrivate, kOidAdslPrivExtraInfo, kOidAdslExtraPLNData,kOidAdslExtraPLNDataUpdate };
			short  PLNIntrArvlBins[kPlnNumberOfInterArrivalBins];
			UINT32  PLNDurationHist[kPlnNumberOfDurationBins];
			UINT32  PLNIntrArvlHist[kPlnNumberOfInterArrivalBins];
			adslINMConfiguration    inmConfig;
			dataBuf[0]=1;
			dataLen=1;
			cmsRet=xdslCtl_SetObjectValue(lineId, oidStr,4,(char*)dataBuf,&dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslSetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			oidStr[2]=kOidAdslExtraPLNInfo;
			dataLen=sizeof(adslPLNData);
			sleep(1);
			cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, 3, (char *)(&adslPLNData), &dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			if (adslPLNData.PLNUpdateData==0)
			{
				fprintf(stderr, "%s: Results update request failed\n",g_szPgmName);
				return ADSLCTL_GENERAL_ERROR;
			}
			oidStr[1]=kOidAdslPrivPLNIntrArvlBins;
			dataLen=sizeof(PLNIntrArvlBins);
			cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, 3, (char *)(&PLNIntrArvlBins), &dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			oidStr[1]=kOidAdslPrivPLNDurationHist;
			dataLen=sizeof(PLNDurationHist);
			cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, 3, (char *)(&PLNDurationHist), &dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			oidStr[1]=kOidAdslPrivPLNIntrArvlHist;
			dataLen=sizeof(PLNIntrArvlHist);
			cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, 3, (char *)(&PLNIntrArvlHist), &dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			oidStr[1]=kOidAdslPrivExtraInfo;
			oidStr[2]=kOidAdslPrivINMConfigParameters;
			dataLen=sizeof(inmConfig);
			cmsRet=xdslCtl_GetObjectValue(lineId, oidStr, 3, (char *)(&inmConfig), &dataLen);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslGetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			INMIATO=inmConfig.INMIATO;
			INMIATS=inmConfig.INMIATS;
			fprintf(stderr,"INM_INPEQ_MODE=%d INMCC=%d INMAITO=%d INMIATS=%d INM_INPEQ_FORMAT=%d \nINMAME (BB Counter)= %u \n",
				inmConfig.INM_INPEQ_MODE,inmConfig.INMCC, INMIATO,INMIATS,inmConfig.INM_INPEQ_FORMAT,adslPLNData.PLNBBCounter);
			for(i=0;i<17;i++)
				fprintf(stderr,"INPEQ%d:     %u\n",(i+1),PLNDurationHist[i]);
			fprintf(stderr,"Inter Arrival Histogram: \n");
			for(i=0;i<7;i++)
				fprintf(stderr,"[%d-%d]:    %u\n",PLNIntrArvlBins[i],PLNIntrArvlBins[i+1]-1, PLNIntrArvlHist[i]);
			fprintf(stderr,"[%d-INF]:   %u\n",PLNIntrArvlBins[7], PLNIntrArvlHist[7]);
			}
		else
			nRet = ADSLCTL_INVALID_OPTION;
	}

	if (ADSLCTL_INVALID_PARAMETER == nRet) {
		fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName, pOptions->pszOptName);
		return nRet;
	}
	if (ADSLCTL_INVALID_OPTION == nRet) {
		fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);
		return nRet;
	}

	return nRet;
}

#ifdef SUPPORT_SELT
/***************************************************************************
 * Function Name: SeltHandler
 * Description  : Processes the adslctl selt command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int SeltHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
	CmsRet   cmsRet=CMSRET_SUCCESS;
	int		 nRet = 0, startSelt=0;
	long	 len=sizeof(SeltData);
    char	 oid[] = { kOidAdslPrivate, kOidAdslPrivExtraInfo, kOidAdslPrivGetSeltData};
    SeltData seltData;

    cmsRet = xdslCtl_GetObjectValue(lineId, oid, sizeof(oid), (char*)&seltData, &len);
	if( cmsRet != CMSRET_SUCCESS) {
		fprintf( stderr, "%s: xdslCtl_GetObjectValue error: %d\n",g_szPgmName, cmsRet);
		return ADSLCTL_GENERAL_ERROR;
	}

	pOptions--;
	while ((0 == nRet) && (nNumOptions-- > 0)) {
		pOptions++;
		if (0 == strcmp(pOptions->pszOptName, "--start")) {
			if (0!=pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}

            startSelt = 1;
		}
		else if (0 == strcmp(pOptions->pszOptName, "--steps")) {
            unsigned char newSteps=0x0;
			if (1!=pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
            if ((pOptions->pszParms[0][0] == '0') &&
                ((pOptions->pszParms[0][1] == 'x') || (pOptions->pszParms[0][1] == 'X'))) {
                newSteps = strtoul(pOptions->pszParms[0]+2, NULL, 16);
            }
            else
                newSteps = strtoul(pOptions->pszParms[0], NULL, 10);

            if (newSteps&(~SELT_STATE_ALL_STEPS)) {
                fprintf(stderr, "%s: invalid steps: 0x%x\n",g_szPgmName, newSteps);
                nRet = ADSLCTL_INVALID_OPTION;
                break;                
            }
            else
            {
                seltData.seltSteps = newSteps;
		}
		}
		else if (0 == strcmp(pOptions->pszOptName, "--cfg")) {
			if ((1!=pOptions->nNumParms) && startSelt) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
            if (startSelt) {
            if ((pOptions->pszParms[0][0] == '0') &&
                ((pOptions->pszParms[0][1] == 'x') || (pOptions->pszParms[0][1] == 'X'))) {
                    seltData.seltCfg = strtoul(pOptions->pszParms[0]+2, NULL, 16);
            }
            else
                    seltData.seltCfg= strtoul(pOptions->pszParms[0], NULL, 10);
            }
            else if ((0==pOptions->nNumParms) && (startSelt==0))
                printf("SELT config: 0x%x   steps: 0x%x\n",(unsigned int)seltData.seltCfg, seltData.seltSteps);
            else
            {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
		}
		else if (0 == strcmp(pOptions->pszOptName, "--status")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
            printf("SELT status: 0x%x\n", (unsigned int)seltData.seltState);
            if ((seltData.seltState&SELT_STATE_MASK)==SELT_STATE_IDLE)
                printf("\tSELT is IDLE\n");
            else if ((seltData.seltState&SELT_STATE_MASK)==SELT_STATE_POSTPROCESSING)
                printf("\tSELT is post-processing\n");
            else if ((seltData.seltState&SELT_STATE_MASK)==SELT_STATE_COMPLETE)
                printf("\tSELT is complete\n");
            else if ((seltData.seltState&SELT_STATE_MASK)==SELT_STATE_MEASURING)
            {
                switch((seltData.seltState>>SELT_STATE_MEASUREMENT_SHIFT)&SELT_STATE_MASK)
                {
                    case (SELT_STATE_WAITING>>SELT_STATE_MEASUREMENT_SHIFT):
                        printf("\tSELT is waiting\n);");
                        break;
                    case (SELT_STATE_MEASURING_QLN>>SELT_STATE_MEASUREMENT_SHIFT):
                        printf("\tSELT is measuring QLN\n);");
                        break;
                    case (SELT_STATE_MEASURING_ENR>>SELT_STATE_MEASUREMENT_SHIFT):
                        printf("\tSELT is measuring ENR\n);");
                        break;
                    case (SELT_STATE_MEASURING_SELT>>SELT_STATE_MEASUREMENT_SHIFT):
                        printf("\tSELT is measuring SELT echo\n);");
                        break;
            }
        }
        }
		else if (0 == strcmp(pOptions->pszOptName, "--stop")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}

            xdslCtl_StopSeltMeasurement(lineId);

		}
		else
			nRet = ADSLCTL_INVALID_OPTION;
	}

	if (ADSLCTL_INVALID_PARAMETER == nRet)
		fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName, pOptions->pszOptName);
	else if (ADSLCTL_INVALID_OPTION == nRet)
		fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);

    if (startSelt)
    {
        fprintf(stderr, "%s->%s: calling xdslCtl_StartSeltTest with steps=0x%x\n", g_szPgmName, __FUNCTION__, seltData.seltSteps );
        nRet = xdslCtl_StartSeltTest(lineId, &seltData.seltSteps, &seltData.seltCfg);
        fprintf(stderr, "%s->%s: calling xdslCtl_StartSeltTest returned %d\n", g_szPgmName, __FUNCTION__, nRet);
    }

	return nRet;
}
#endif /* SUPPORT_SELT */



#ifdef NTR_SUPPORT

/***************************************************************************
 * Function Name: NTRHandler
 * Description  : Processes the adslctl ntr command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int NTRHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
	CmsRet cmsRet=CMSRET_SUCCESS;
	int		nRet = 0;
	
	pOptions--;
	while ((0 == nRet) && (nNumOptions-- > 0)) {
		pOptions++;
		if (0 == strcmp(pOptions->pszOptName, "--start")) {
			long	len;
			dslNtrCfg	ntrCfg;
			char	oid[] = { kOidAdslPrivate, kOidAdslPrivExtraInfo, kOidAdslPrivSetNtrCfg};
			
			memset((void *)&ntrCfg, 0, sizeof(dslNtrCfg));
			ntrCfg.operMode = kNtrOperModeInt;
			ntrCfg.intModeDivRatio = 8000;	/* default output frequency(8kHz) */
			if (1 < pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			else if(1 == pOptions->nNumParms)
				ntrCfg.intModeDivRatio = strtol(pOptions->pszParms[0], NULL, 10);
			len = sizeof(dslNtrCfg);
			cmsRet = xdslCtl_SetObjectValue(lineId, oid, sizeof(oid), (char*)&ntrCfg, &len);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: xdslCtl_SetObjectValue error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			cmsRet = xdslCtl_SetTestMode(lineId, kNtrStart);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: xdslCtl_SetTestMode error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
		}
		else if (0 == strcmp(pOptions->pszOptName, "--stop")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			cmsRet = xdslCtl_SetTestMode(lineId, kNtrStop);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: xdslCtl_SetTestMode error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
		}
		else
			nRet = ADSLCTL_INVALID_OPTION;
	}

	if (ADSLCTL_INVALID_PARAMETER == nRet)
		fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName, pOptions->pszOptName);
	else if (ADSLCTL_INVALID_OPTION == nRet)
		fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);
	
	return nRet;
}

#endif

/***************************************************************************
 * Function Name: QLNHandler
 * Description  : Processes the adslctl qlnmntr command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int QLNHandler(unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
	CmsRet cmsRet=CMSRET_SUCCESS;
	int		nRet = 0;
	unsigned char	diagCmd[1000], *pDiagCmd;
	SINT32		n;
	
	diagCmd[0] = 0;
	diagCmd[1] = 0;
	diagCmd[2] = (DIAG_DSL_CLIENT << DIAG_PARTY_TYPE_SEND_SHIFT) & 0xFF;
	pOptions--;
	while ((0 == nRet) && (nNumOptions-- > 0)) {
		pOptions++;
		if (0 == strcmp(pOptions->pszOptName, "--time")) {
			
			if (1 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			pDiagCmd=diagCmd+3;
			*pDiagCmd++=52;
			n=strtol(pOptions->pszParms[0], NULL, 10);
			memcpy(pDiagCmd,&n,sizeof(SINT32));
			pDiagCmd+=sizeof(SINT32);
			cmsRet = xdslCtl_DiagProcessCommandFrame(lineId, diagCmd, pDiagCmd - diagCmd);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslDiagProcessCommandFrame error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
		}
		else if (0 == strcmp(pOptions->pszOptName, "--freq")) {
			
			if (1 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			pDiagCmd=diagCmd+3;
			*pDiagCmd++=51;
			n=strtol(pOptions->pszParms[0], NULL, 10);
			memcpy(pDiagCmd,&n,sizeof(SINT32));
			pDiagCmd+=sizeof(SINT32);
			cmsRet = xdslCtl_DiagProcessCommandFrame(lineId, diagCmd, pDiagCmd - diagCmd);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslDiagProcessCommandFrame error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
			}
			pDiagCmd=diagCmd+3;
			*pDiagCmd++=23;
			n=13;
			memcpy(pDiagCmd,&n,sizeof(SINT32));
			pDiagCmd+=sizeof(SINT32);
			cmsRet = xdslCtl_DiagProcessCommandFrame(lineId, diagCmd, pDiagCmd - diagCmd);
			if( cmsRet != CMSRET_SUCCESS) {
				fprintf( stderr, "%s: devCtl_adslDiagProcessCommandFrame error\n",g_szPgmName );
				return ADSLCTL_GENERAL_ERROR;
				}
			}
		else
			nRet = ADSLCTL_INVALID_OPTION;
	}

	if (ADSLCTL_INVALID_PARAMETER == nRet) {
		fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName, pOptions->pszOptName);
		return nRet;
	}
	if (ADSLCTL_INVALID_OPTION == nRet) {
		fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);
		return nRet;
	}

	return nRet;
}

/***************************************************************************
 * Function Name: SnrClampHandler
 * Description  : Processes the adslctl snr clamping control command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int SnrClampHandler( unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
		CmsRet cmsRet=CMSRET_SUCCESS;
		int  nRet = 0;
		SINT32 n;
		char oidStr[] = { kOidAdslPrivate, kOidAdslPrivExtraInfo, kOidAdslPrivSetSnrClampShape };
		char data[512];
		pOptions--;
		while ((0 == nRet) && (nNumOptions-- > 0)) {
			pOptions++;
			if (0 == strcmp(pOptions->pszOptName, "--shape")) {
				long dataLen=512;
				if (1 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
				}
				n=strtol(pOptions->pszParms[0], NULL, 10);
				if ((n>2) || (n<0))
				{
					nRet = ADSLCTL_INVALID_PARAMETER;
					break;
				}
				else if (n==0) {
					memset(data, 80<<1, 512);
					data[0] = 0;                         /* Tone zero used for PHY internal ID: 0 means reset*/
				}
				else if (n==1) {
					memset(data, 80<<1, 200);
					memset(&data[200], 40<<1, 100);
					memset(&data[300], 80<<1, 512-300);
					data[0] = n;                         /* Tone zero used for PHY internal ID*/
				}
				else if (n==2) {
					memset(data, 50<<1, 512);
					data[0] = n;                         /* Tone zero used for PHY internal ID*/
				}
				cmsRet=xdslCtl_SetObjectValue(lineId,oidStr,3,data,&dataLen);
				if( cmsRet != CMSRET_SUCCESS) {
					fprintf( stderr, "%s: xdslCtl_SetObjectValue error\n",g_szPgmName );
					return ADSLCTL_GENERAL_ERROR;
				}
			}
			else if (0 == strcmp(pOptions->pszOptName, "--bpshape")) {
				long n = (long)GetSnrClampShape(pOptions->pszParms[0], data, 512);
				if (n!=512)
				{
					nRet = ADSLCTL_INVALID_PARAMETER;
					break;
				}
				cmsRet=xdslCtl_SetObjectValue(lineId,oidStr,3,data,&n);
				if( cmsRet != CMSRET_SUCCESS) {
					fprintf( stderr, "%s: xdslCtl_SetObjectValue error\n",g_szPgmName );
					return ADSLCTL_GENERAL_ERROR;
				}
				fprintf( stderr, "%s: xdslCtl_SetObjectValue Done\n",g_szPgmName);
			}
		else
			nRet = ADSLCTL_INVALID_OPTION;
		}

		if (ADSLCTL_INVALID_PARAMETER == nRet) {
			fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName,
			pOptions->pszOptName);
			return nRet;
		}
		if (ADSLCTL_INVALID_OPTION == nRet) {
			fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);
			return nRet;
		}
		return nRet;
}


/***************************************************************************
 * Function Name: ProfileHandler
 * Description	: Processes the adslctl profile control command.
 * Returns		: 0 - success, non-0 - error
 ***************************************************************************/
static int ProfileHandler( unsigned char lineId, POPTION_INFO pOptions, int nNumOptions )
{
	int	nRet = 0;
	pOptions--;
	while ((0 == nRet) && (nNumOptions-- > 0)) {
		pOptions++;
		if (0 == strcmp(pOptions->pszOptName, "--show")) {
			if (0 != pOptions->nNumParms) {
				nRet = ADSLCTL_INVALID_PARAMETER;
				break;
			}
			ProfileShow(lineId);
		}
		else if (0 == strcmp(pOptions->pszOptName, "--save")) {
			printf("%s %s is only supported from Linux404 on ward\n",g_szPgmName, pOptions->pszOptName);
		}
		else if (0 == strcmp(pOptions->pszOptName, "--restore")) {
			printf("%s %s is only supported from Linux404 on ward\n",g_szPgmName, pOptions->pszOptName);
		}
		else
			nRet = ADSLCTL_INVALID_OPTION;
	}

	if (ADSLCTL_INVALID_PARAMETER == nRet)
		fprintf(stderr, "%s: invalid parameter for option %s\n", g_szPgmName, pOptions->pszOptName);
	else if (ADSLCTL_INVALID_OPTION == nRet)
		fprintf(stderr, "%s: invalid option %s\n", g_szPgmName, pOptions->pszOptName);

	return nRet;
}


/***************************************************************************
 * Function Name: VersionHandler
 * Description  : Processes the adslctl version command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int VersionHandler(unsigned char lineId ATTRIBUTE_UNUSED, POPTION_INFO pOptions ATTRIBUTE_UNUSED, int nNumOptions ATTRIBUTE_UNUSED)
{
   adslVersionInfo adslVer;
   int nRet = BCMADSL_STATUS_SUCCESS;
   char *s = "Unknown";
   CmsRet cmsRet;

   fprintf(stderr, "%s version " ADSLCTL_VERSION "\n", g_szPgmName );
   cmsRet = xdslCtl_GetVersion(0, &adslVer);
   if (cmsRet != CMSRET_SUCCESS) 
   {
      fprintf( stderr, "%s: devCtl_adslGetVersion error\n", g_szPgmName );
      nRet = ADSLCTL_GENERAL_ERROR;
   }
   else
   {
      switch (adslVer.phyType)
      {
      case kAdslTypeAnnexA:
         s = "AnnexA";
         break;
      case kAdslTypeAnnexB:
         s = "AnnexB";
         break;
      case kAdslTypeAnnexC:
         s = "AnnexC";
         break;
      case kAdslTypeSADSL:
         s = "SADSL";
         break;
      case kAdslTypeUnknown:
      default:
         s = "Unknown";
         break;
      }
   }
   fprintf(stderr, "ADSL PHY: %s version - %s\n", s, adslVer.phyVerStr);
   return( nRet );
} /* VersionHandler */

/***************************************************************************
 * Function Name: HelpHandler
 * Description  : Processes the adslctl help command.
 * Returns      : 0 - success, non-0 - error
 ***************************************************************************/
static int HelpHandler(unsigned char lineId ATTRIBUTE_UNUSED, POPTION_INFO pOptions ATTRIBUTE_UNUSED, int nNumOptions ATTRIBUTE_UNUSED)
{
   fprintf( stderr,
   "Usage: %s start [--up] <configure command options>\n" 
   "       %s stop\n"
   "       %s connection [--up] [--down] [--loopback] [--reverb]\n"
   "           [--medley] [--noretrain] [--L3] [--diagmode] [--L0]\n"
   "           [--tones <r1-r2,r3-r4,...>] [--normal] [--freezeReverb] [--freezeMedley]\n"
   HELP_CMD_OPTION_MOD
   "           [--trellis <on|off>] [--snr <snrQ4>] [--bitswap <on|off>] [--sesdrop <on|off>]\n"
   "           [--sra <on|off>] [--CoMinMgn <on|off>] [--minINP <sym>] [--maxDelay <ms>] [--i24k <on|off>] [--phyReXmt <0xBitMap-UsDs>]\n"
   "           [--Ginp <0xBitMap-UsDs>] [--TpsTc <0xBitMap-AvPvAaPa>] [--monitorTone <on|off>]\n"
#ifdef CONFIG_VDSL_SUPPORTED
   HELP_CMD_OPTION_PROFILE
   "           [--dynamicD <on|off>] [--dynamicF <on|off>] [--SOS <on|off>] [--maxDataRate <maxDsDataRateKbps maxUsDataRateKbps maxAggrDataRateKbps>]\n"
#endif
#ifdef ANNEX_C
   "           [--bm <(D)BM|(F)BM>] [--ccw]\n"
#else /* Allow for AnnexA/AnnexB builds */
   "           [--forceJ43 <on|off>] [--toggleJ43B43 <on|off>]\n"
#endif
   "       %s bert [--start <#seconds>] [--stop] [--show]\n"
   "       %s afelb [--time <sec>] [--tones] [--signal <1/2/8>] \n"
   "       %s qlnmntr [--time <sec>] [--freq <msec>]\n"
   "       %s inm [--start <INMIATO> <INMIATS><INMCC><INM_INPEQ_MODE><INM_INPEQ_FORMAT>] [--show]\n"
   "       %s snrclamp [--shape <shapeId>] [--bpshape [bpIndex-bpLevel,]]\n"
   "       %s nlnm [--show ] [--setThld <Thld_Num_Tones>]\n"
   "       %s diag [--logstart <nBytes>] [--logpause] [--logstop] [--loguntilbufferfull <nBytes>] [--loguntilretrain <nBytes>] [--dumpBuf <sizeKb>]\n"
#ifdef SUPPORT_MULTI_PHY
   "           [--mediaSearchCfg <0xBitMap >]\n"
   "               Bits\n"
   "                [0] PHY switch:            1=Disabled, 0=Enabled\n"
   "                [1] Media search:          1=Disabled, 0=Enabled\n"
#if !defined(SUPPORT_DSL_GFAST) || defined(SUPPORT_DSL_GFASTCOMBO)
   "                [2] Force new line configuration as defined in bits 3, 4 and 5\n"
#else
   "                [2] Force new PHY/line configuration as defined in bits 3, 4, 5 and 6\n"
#endif
   "                [3] Single line only:      1=Enabled,  0=Disabled\n"
   "                [4-5] AFE:                 1=external, 0=internal\n"
#if defined(SUPPORT_DSL_GFAST) && !defined(SUPPORT_DSL_GFASTCOMBO)
   "                [6] PHY Type:              1=Gfast,    0=Non-Gfast\n"
#endif
#endif
#if defined(SUPPORT_DSL_GFAST)
#if !defined(SUPPORT_DSL_GFASTCOMBO)
   "           [--phyTypeCfg <0xBitMap >]\n"
   "               Bits\n"
   "                [0] PHY switch:            1=Disabled, 0=Enabled\n"
   "                [1] PHY type:              1=Gfast,    0=Non-Gfast\n"
#else /* SUPPORT_DSL_GFASTCOMBO */
   "           [--phyTypeCfg <0xBitMap >]\n"
   "               Bits\n"
   "                [1] AFE relay mode:        1=Gfast,    0=Non-Gfast\n"
#endif /* SUPPORT_DSL_GFASTCOMBO */
#endif /* SUPPORT_DSL_GFAST */
#ifdef NTR_SUPPORT
   "       %s ntr [--start [output freq(default is 8000)]] [--stop]\n"
#endif
#ifdef SUPPORT_SELT
   "       %s selt [--start] [--stop] [--status] [--steps <steps_bitmap>] [--cfg <selt_cfg>]\n"
#endif
   "       %s info [--state] [--show] [--stats] [--SNR] [--QLN] [--Hlog] [--Hlin] [--HlinS] [--Bits]\n"
   "           "
#ifdef SUPPORT_24HR_CNT_STAT
   "[--24hrhiststat] "
#endif
#ifdef CONFIG_VDSL_SUPPORTED
   "[--pbParams] "
#endif
   "[--linediag] "
#ifdef CONFIG_VDSL_SUPPORTED
   "[--linediag1] "
#endif
#if defined(SUPPORT_SELT) || defined(CONFIG_VDSL_SUPPORTED)
   "[--UER] "
#ifdef SUPPORT_SELT
   "[--EchoVariance] "
#endif
#endif
#if defined(CONFIG_RNC_SUPPORT)
   "[--RNC_QLN] "
#endif
#if defined(SUPPORT_DSL_GFAST)
   "[--ALN] [--BitsDOI] "
#endif
#ifdef CONFIG_TOD_SUPPORTED
   "\n           [--TOD] "
#endif
   "[--reset] [--vendor] [--cfg]\n"
   "       %s profile [--show] [--save] [--restore]\n"
   "       %s --version\n"
   "       %s --help\n",
   g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName,
   g_szPgmName, g_szPgmName, g_szPgmName, g_szPgmName,
   g_szPgmName,g_szPgmName, g_szPgmName, g_szPgmName,
#ifdef NTR_SUPPORT
   g_szPgmName,
#endif
#ifdef SUPPORT_SELT
   g_szPgmName,
#endif
   g_szPgmName, g_szPgmName, g_szPgmName);
   return( 0 );
} /* HelpHandler */
