/***********************************************************************
 *
 *  Copyright (c) 2007-2010  Broadcom Corporation
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
 * File Name  : blogctl_cmds.c
 * Description: Linux command line utility that controls the Broadcom
 *              BCM6368 Flow Cache Driver.
 ***************************************************************************/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <blog_ioctl.h>
#include <blogctl_api.h>
#include <blogctl.h>


void Usage(void);

int  blogctl_config_handler(POPTION_INFO pOptions, int nNumOptions);
int  blogctl_get_stats_handler(POPTION_INFO pOptions, int nNumOptions);
int  blogctl_reset_stats_handler(POPTION_INFO pOptions, int nNumOptions);

char g_PgmName[128] = {0};

/*** File Scoped Globals. ***/
COMMAND_INFO g_Cmds[] =
{
    {"stats",       {""},                       blogctl_get_stats_handler},
    {"resetstats",  {""},                       blogctl_reset_stats_handler},
    {"config",      {"--blog-dump", ""},        blogctl_config_handler},
    {""}
};

/*
 *------------------------------------------------------------------------------
 * Function Name: Usage
 * Description  : Displays the fc usage
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
void Usage(void)
{
    printf( 
        "Blog Control Utility:\n\n"
        "::: Usage:\n\n"
        "       blog stats\n"
        "       blog resetstats\n"
        "       blog config [ --blog-dump <0|1|2|3> ]\n\n"
        );

    return;
} /* Usage */


/*
 *------------------------------------------------------------------------------
 * Function Name: blogctl_config_handler
 * Description  : Processes the flow cache config command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int blogctl_config_handler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet;

    if ( nNumOptions == 0 )
    {
        fprintf( stderr, "%s: No options specified\n", g_PgmName );
        return BLOGCTL_INVALID_OPTION;
    }
    else if ( nNumOptions > 1 )
    {
        fprintf( stderr, "%s: Too many options specified\n", g_PgmName );
        return BLOGCTL_INVALID_OPTION;
    }
    else if ( !strcmp( pOptions->pOptName, "--blog-dump") )
    {
        int dump_val;
        if ( pOptions->nNumParms != 1 )
        {
            fprintf( stderr, "%s: must specify blog-dump action 0 (=disable), 1 (=RX), 2 (=TX), or 3 (=RX & TX)\n",
                     g_PgmName );
            return BLOGCTL_INVALID_OPTION;
        }
        dump_val = atoi( pOptions->pParms[0] );
        nRet = blogctl_config( BLOG_CONFIG_OPT_DUMP_BLOG, dump_val );
		
        if(nRet == BLOG_ERROR)
        	fprintf( stderr, "%s: failed to config dump-blog \n", g_PgmName );
        else
        	printf("config --blog-dump RX<%s> TX<%s>\n", 
        		(dump_val & 0x01)? "ENABLED" : "DISABLED", 
        		(dump_val & 0x02)? "ENABLED" : "DISABLED" );
    }
    else
    {
            fprintf( stderr, "%s: invalid option [%s]\n",
                     g_PgmName, pOptions->pOptName );
            /* Bad parameters - show usage */
            Usage();
            return BLOGCTL_INVALID_OPTION;
    }

    if ( nRet == BLOG_ERROR )
        fprintf( stderr, "%s: failed to configure Flow Cache \n", g_PgmName );

    return nRet;
}

/*
 *------------------------------------------------------------------------------
 * Function Name: blogctl_get_stats_handler
 * Description  : Processes the flow cache get stats command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int blogctl_get_stats_handler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet = blogctl_get_stats();
    if ( nRet == BLOG_ERROR )
        fprintf( stderr, "%s: failed to get blog stats\n", g_PgmName );
    else
        printf("get blog stats.\n");

    return nRet;
}


/*
 *------------------------------------------------------------------------------
 * Function Name: blogctl_reset_stats_handler
 * Description  : Processes the flow cache reset stats command.
 * Returns      : 0 - success, non-0 - error
 *------------------------------------------------------------------------------
 */
int blogctl_reset_stats_handler(POPTION_INFO pOptions, int nNumOptions)
{
    int nRet = blogctl_reset_stats();
    if ( nRet == BLOG_ERROR )
        fprintf( stderr, "%s: failed to reset blog stats\n", g_PgmName );
    else
        printf("reset blog stats.\n");

    return nRet;
}



