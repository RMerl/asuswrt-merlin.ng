/******************************************************************************
 *
 *  Copyright (c) 2009  Broadcom Corporation
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
#ifndef _MOCALIB_CLI_H_
#define _MOCALIB_CLI_H_

/* Limit values */
#define CMD_NAME_LEN                        64
#define MAX_OPTS                            280
#define MAX_PARMS                           9216
#define MAX_IFNAME_INPUTS                   16
#define MAX_STRING_LENGTH                   500

/* Argument type values. */
#define ARG_TYPE_COMMAND                    1
#define ARG_TYPE_OPTION                     2
#define ARG_TYPE_PARAMETER                  3

extern uint32_t gCheckinit;

typedef struct
{
   char *pszOptName;
   char *pszParms[MAX_PARMS];
   int nNumParms;
} OPTION_INFO, *POPTION_INFO;

#ifdef MOCACTL_C
// mocactl uses a different style command handler for now
typedef int (*FN_COMMAND_HANDLER) (void * handle, POPTION_INFO pOptions, int nNumOptions);
#else
typedef int (*FN_COMMAND_HANDLER) (void * handle, int argc, char **argv);
#endif


typedef int (*mocacli_fn)(void * handle, char ** pp_parms, int num_parms);

struct handler_entry {
   char *      string;
   char *      alias;
   mocacli_fn  get_fn;
   mocacli_fn  set_fn;
   mocacli_fn  do_fn;
};

struct handler_info {
   char *      string;
   mocacli_fn  fn;
   unsigned int  binit:1;
   unsigned int  bconfig:1;
   char *grpname;
}  __attribute__((packed,aligned(4)));

typedef struct
{
   char szCmdName[CMD_NAME_LEN];
   char *pszOptionNames[MAX_OPTS];
   FN_COMMAND_HANDLER pfnCmdHandler;
} COMMAND_INFO, *PCOMMAND_INFO;

int mocacli_get_nvram_handler( void * handle, char ** pp_parms, int num_parms );
int mocacli_do_pqos_update_flow_handler( void * handle, char ** pp_parms, int num_parms );
int mocacli_get_gen_node_ext_status_handler( void * handle, char ** pp_parms, int num_parms );
MOCALIB_CLI_GET int mocacli_get_rx_gain_params_handler( void * handle, char ** pp_parms, int num_parms );
MOCALIB_CLI_GET int mocacli_get_tx_power_params_handler( void * handle, char ** pp_parms, int num_parms );
MOCALIB_CLI_GET int mocacli_get_rx_gain_agc_table_handler( void * handle, char ** pp_parms, int num_parms );
int mocacli_get_network_handler( void * handle, char ** pp_parms, int num_parms);
void mocacli_register_help_handler(FN_COMMAND_HANDLER pFn);
COMMAND_INFO * mocacli_get_cmds(int *ncmds);
void mocacli_print_bit_loading(uint32_t * p_bit_loading1, uint32_t * p_bit_loading2, uint32_t num_carriers);
uint8_t mocacli_get_subcarrier(uint32_t * p_bit_loading, uint32_t sub_carrier);
int mocacli_get_macaddr(char * string, macaddr_t * out);
MOCALIB_CLI int  divByPrecisionFractional(int a_val, int a_precision);

#endif
