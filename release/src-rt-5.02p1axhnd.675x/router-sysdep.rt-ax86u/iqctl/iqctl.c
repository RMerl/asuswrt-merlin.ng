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
 * File Name  : iqctl.c
 * Description: Linux command line utility that controls the Broadcom
 *              Ingress QoS Driver.
 *              BCM96368 and BCM6816 specific command line parsing is
 *              implemented in the iqCtl_6368.c and iqCtl_6816.c
 ***************************************************************************/

#include <ctype.h>
#include <iqctl_common.h>
#include <iqctl.h>

/*
 *------------------------------------------------------------------------------
 * Function Name: main
 * Description  : Main program function.
 * Returns      : 0 - success, non-0 - error.
 *------------------------------------------------------------------------------
 */
#ifdef BUILD_STATIC
int iq_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    int nExitCode = 0;
    PCOMMAND_INFO pCmd;
        /* Save the name that started this program into a global variable. */
    strcpy( g_PgmName, *argv );
    if ( argc == 1 )
    {
        Usage( );
        exit( nExitCode );
    }
    argc--, argv++;
    while ( argc && nExitCode == 0 )
    {
        if ( GetArgType( *argv, g_Cmds, NULL ) == ARG_TYPE_COMMAND )
        {
            int argnext = 0;
            pCmd = GetCommand( *argv, g_Cmds );
            argc--; argv++;
            nExitCode = ProcessCommand( pCmd, argc, argv, g_Cmds, &argnext );
            argc -= argnext;
            argv += argnext;
        }
        else
        {
            nExitCode = IQCTL_INVALID_COMMAND;
            fprintf( stderr, "%s: invalid command\n", g_PgmName );
        }
    }
    exit( nExitCode );
}


/*
 *------------------------------------------------------------------------------
 * Function Name: GetArgType
 * Description  : Determines if the specified command line argument is a
 *                command, option or option parameter.
 * Returns      : ARG_TYPE_COMMAND, ARG_TYPE_OPTION, ARG_TYPE_PARAMETER
 *------------------------------------------------------------------------------
 */
int GetArgType(char *pszArg, PCOMMAND_INFO pCmds, char **ppszOptions)
{
    int nArgType = ARG_TYPE_PARAMETER;

    if ( ppszOptions )  /* See if the argument is a option. */
    {
        do
        {
            if ( !strcmp( pszArg, *ppszOptions ) )
            {
                nArgType = ARG_TYPE_OPTION;
                break;
            }
        } while ( *++ppszOptions );
    }

        /* Next, see if the argument is an command. */
    if ( nArgType == ARG_TYPE_PARAMETER )
    {
        while ( pCmds->szCmdName[0] != '\0' )
        {
            if ( !strcmp( pszArg, pCmds->szCmdName ) )
            {
                nArgType = ARG_TYPE_COMMAND;
                break;
            }
            pCmds++;
        }
    }

    return nArgType;    /* Otherwise, assume that it is a parameter. */
} /* GetArgType */


/*
 *------------------------------------------------------------------------------
 * Function Name: GetCommand
 * Description  : Returns the COMMAND_INFO structure for the specified
 *                command name.
 * Returns      : COMMAND_INFOR structure pointer
 *------------------------------------------------------------------------------
 */
PCOMMAND_INFO GetCommand(char *pszArg, PCOMMAND_INFO pCmds)
{
    PCOMMAND_INFO pCmd = NULL;

    while ( pCmds->szCmdName[0] != '\0' )
    {
        if ( !strcmp( pszArg, pCmds->szCmdName ) )
        {
            pCmd = pCmds;
            break;
        }
        pCmds++;
    }

    return pCmd;
} /* GetCommand */


/*
 *------------------------------------------------------------------------------
 * Function Name: ProcessCommand
 * Description  : Gets the options and option paramters for a command and
 *                calls the command handler function to process the command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int ProcessCommand(PCOMMAND_INFO pCmd, int argc, char **argv,
                   PCOMMAND_INFO pCmds, int *pnArgNext)
{
    int nRet = IQCTL_SUCCESS, nNumOptInfo = 0, nArgType = 0;
    OPTION_INFO OptInfo[MAX_OPTS], *pCurrOpt = NULL;

    memset( OptInfo, 0x00, sizeof(OptInfo) );
    *pnArgNext = 0;

    do
    {
        if ( argc == 0 ) break;

        nArgType = GetArgType( *argv, pCmds, pCmd->pszOptionNames );
        switch ( nArgType )
        {
        case ARG_TYPE_OPTION:
            if ( nNumOptInfo < MAX_OPTS )
            {
                pCurrOpt = &OptInfo[nNumOptInfo++];
                pCurrOpt->pOptName = *argv;
            }
            else
            {
                nRet = IQCTL_INVALID_NUMBER_OF_OPTIONS;
                fprintf( stderr, "%s: too many options\n", g_PgmName );
            }
            (*pnArgNext)++;
            break;

        case ARG_TYPE_PARAMETER:
            if ( pCurrOpt && pCurrOpt->nNumParms < MAX_PARMS )
                pCurrOpt->pParms[pCurrOpt->nNumParms++] = *argv;
            else
            {
                if ( pCurrOpt )
                {
                    nRet = IQCTL_INVALID_OPTION;
                    fprintf( stderr, "%s: invalid option\n", g_PgmName );
                }
                else
                {
                    nRet = IQCTL_INVALID_NUMBER_OF_OPTIONS;
                    fprintf( stderr, "%s: too many parameters\n", g_PgmName );
                }
            }
            (*pnArgNext)++;
            break;

        case ARG_TYPE_COMMAND:
            /* The current command is done. */
            break;
        } /* switch ( nArgType ) */

        argc--, argv++;

    } while ( (nRet == IQCTL_SUCCESS) && (nArgType!=ARG_TYPE_COMMAND) );

    if ( nRet == IQCTL_SUCCESS )
        nRet = (*pCmd->pfnCmdHandler)( OptInfo, nNumOptInfo );

    return nRet;
} /* ProcessCommand */


/*
 *------------------------------------------------------------------------------
 * Function Name: DumpOption
 * Description  : Debug function that dumps the options and parameters
 *                for a particular command.
 * Returns      : None.
 *------------------------------------------------------------------------------
 */
void DumpOption(char *pszCmdName, POPTION_INFO pOptions, int nNumOptions)
{
    POPTION_INFO pOpt;
    int i, j;

    printf( "cmd=%s\n", pszCmdName );
    for ( i = 0; i < nNumOptions; i++ )
    {
        pOpt = pOptions + i;
        printf( "opt=%s, %d parms=", pOpt->pOptName, pOpt->nNumParms );
        for ( j = 0; j < pOpt->nNumParms; j++ )
        {
            printf( pOpt->pParms[j] );
            printf( " " );
        }
        printf( "\n" );
    }
    printf( "\n" );
}

/*
 *------------------------------------------------------------------------------
 * Function Name: htoi
 * Description  : Hex character string to integer converter
 * Returns      : integer corresponding to hex string
 *------------------------------------------------------------------------------
 */

int power(int base, int n)
{
    int i, p;
    if (n == 0) return 1;
    p = 1;
    for (i = 1; i <= n; ++i) p = p * base;
    return p;
}

int htoi(char s[])
{
    int len, value = 1, digit = 0,  total = 0;
    int c, x, y, i = 0;
    char hexchars[] = "abcdef";
    if (s[i] == '0') { i++; if (s[i] == 'x' || s[i] == 'X') i++; }
    len = strlen(s);
    for (x = i; x < len; x++)
    {
        c = tolower(s[x]);
        if (c >= '0' && c <= '9')
           digit = c - '0';
        else if (c >= 'a' && c <= 'f')
        {
            for (y = 0; hexchars[y] != '\0'; y++)
                if (c == hexchars[y])
                    digit = y + 10;
        }
        else return 0;
        value = power(16, len-x-1);
        total += value * digit;
    }
    return total;
}

