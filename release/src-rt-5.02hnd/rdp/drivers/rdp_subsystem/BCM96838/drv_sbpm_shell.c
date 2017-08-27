/*
* <:copyright-BRCM:2012:proprietary:standard
* 
*    Copyright (c) 2012 Broadcom 
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

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation of UT shell commands for the Lilac   */
/* BPM driver.                                                                 */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdp_drv_sbpm.h"
#include "bdmf_shell.h"


#ifdef USE_BDMF_SHELL

/*****************************************************************************/
/*                                                                           */
/* Static Functions declaration                                              */
/*                                                                           */
/*****************************************************************************/
char * f_drv_sbpm_error_code_to_string ( DRV_SBPM_ERROR xi_error_code ) ;
static char * f_user_group_status_enum_to_string ( E_DRV_SBPM_ENABLE xi_enum_value );
static char * f_user_group_exclusive_status_enum_to_string ( E_DRV_SBPM_ENABLE xi_enum_value );
static DRV_SBPM_ERROR f_dump_sbpm_database ( bdmf_session_handle  session );
char * f_sbpm_source_port_enum_to_string ( DRV_SBPM_SP_USR xi_sp );



#define CS_MAXIMAL_DIRECTORY_NAME_LENGTH   ( 256 )

static int p_bl_drv_sbpm_initialize_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint16_t base_address;
    uint16_t list_size;
    DRV_SBPM_GLOBAL_CONFIGURATION    global_configuration;
    DRV_SBPM_USER_GROUPS_THRESHOLDS  ug_configuration;
    DRV_SBPM_ERROR error ;
    uint8_t parameter_index ;

    memset ( & ug_configuration, 0, sizeof (DRV_SBPM_USER_GROUPS_THRESHOLDS ) ) ;

    /* Get the parameters */
    parameter_index = 0;
    base_address =  parm[parameter_index++].value.unumber;
    list_size = parm[parameter_index++].value.unumber;
    global_configuration.threshold = parm[parameter_index++].value.unumber;
    global_configuration.hysteresis = parm[parameter_index++].value.unumber;
    switch (parm[parameter_index++].value.unumber)
    {
    case 0:
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_0].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_1].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_2].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_3].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_4].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_5].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_6].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_7].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_0].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_1].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_2].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_3].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_4].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_5].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_6].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_7].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_0].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_1].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_2].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_3].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_4].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_5].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_6].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_7].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_0].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_1].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_2].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_3].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_4].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_5].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_6].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_7].exclusive_hysteresis = 0x100;
            break;
    case 1:

            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_0].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_1].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_2].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_3].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_4].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_5].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_6].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_7].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_0].threshold = 0x2400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_1].threshold = 0x2400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_2].threshold = 0x2400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_3].threshold = 0x2400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_4].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_5].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_6].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_7].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_0].exclusive_threshold = 0xFFFF;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_1].exclusive_threshold = 0x2000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_2].exclusive_threshold = 0x2000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_3].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_4].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_5].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_6].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_7].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_0].exclusive_hysteresis = 0x500;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_1].exclusive_hysteresis = 0x420;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_2].exclusive_hysteresis = 0x660;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_3].exclusive_hysteresis = 0x220;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_4].exclusive_hysteresis = 0x000;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_5].exclusive_hysteresis = 0x101;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_6].exclusive_hysteresis = 0xFFF;
            ug_configuration.ug_arr[DRV_SBPM_USER_GROUP_7].exclusive_hysteresis = 0xFFFF;

            break;
    }

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_init'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_init ( base_address ,
                                  list_size ,
                                  0x08980 ,
                                  &global_configuration,
                                  &ug_configuration ) ;
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_sp_enable_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_SP_ENABLE sbpm_sp_enable ;  /* input */
    DRV_SBPM_ERROR error ;

    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    sbpm_sp_enable.eth0_sp_enable         = parm[parameter_index++].value.number;
    sbpm_sp_enable.eth1_sp_enable         = parm[parameter_index++].value.number;
    sbpm_sp_enable.eth2_sp_enable         = parm[parameter_index++].value.number;
    sbpm_sp_enable.eth3_sp_enable         = parm[parameter_index++].value.number;
    sbpm_sp_enable.eth4_sp_enable         = parm[parameter_index++].value.number;
    sbpm_sp_enable.gpon_or_eth5_sp_enable = parm[parameter_index++].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_sp_enable_'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_sp_enable ( & sbpm_sp_enable ) ;
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_set_global_threshold_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint32_t hysteresis ;/* input */
    uint32_t threshold  ;  /* input */
    DRV_SBPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    threshold = parm[parameter_index++].value.number;
    hysteresis = parm[parameter_index++].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_set_global_threshold'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error =  fi_bl_drv_sbpm_set_global_threshold ( threshold, hysteresis ) ;
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_global_threshold_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint16_t hysteresis ;/* output */
    uint16_t threshold  ;  /* output */

    DRV_SBPM_ERROR error  ;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_global_threshold'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_global_threshold ( &threshold, &hysteresis ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "Global Threshold Configuration:\n\r" ) ;
        bdmf_session_print(session, "Global Threshold :%d\n\r", threshold ) ;
        bdmf_session_print(session, "Global Threshold hysteresis: %d\n\r" ,hysteresis ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_set_user_group_thresholds_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_USER_GROUP_CONFIGURATION user_group_configuration ;/* input */
    DRV_SBPM_USER_GROUP user_group_id  ;  /* input */
    DRV_SBPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    user_group_id = parm[parameter_index++].value.unumber;
    user_group_configuration.hysteresis = parm[parameter_index++].value.unumber;
    user_group_configuration.threshold = parm[parameter_index++].value.unumber;
    user_group_configuration.exclusive_hysteresis = parm[parameter_index++].value.unumber;
    user_group_configuration.exclusive_threshold = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_set_user_group_thresholds'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_set_user_group_thresholds ( user_group_id, &user_group_configuration ) ;
    /* -------------------------------------------------------------------- */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_user_group_thresholds_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_USER_GROUP user_group_id  ;  /* input */
    DRV_SBPM_USER_GROUP_CONFIGURATION user_group_configuration ;/* output */
    DRV_SBPM_ERROR error ;
    uint8_t parameter_index = 0 ;

    /* Get the parameters */
    parameter_index = 0 ;
    user_group_id = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_user_group_thresholds'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_user_group_thresholds ( user_group_id, &user_group_configuration ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "User Group %d Configuration:\n\r", (int)user_group_id ) ;
        bdmf_session_print(session, " Threshold :%d\n\r", (int)user_group_configuration.threshold)  ;
        bdmf_session_print(session, " hysteresis: %d\n\r", (int)user_group_configuration.hysteresis ) ;
        bdmf_session_print(session, " Exclusive Threshold :%d\n\r", (int)user_group_configuration.exclusive_threshold ) ;
        bdmf_session_print(session, " Exclusive hysteresis: %d\n\r", (int)user_group_configuration.exclusive_hysteresis ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_user_group_status_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_USER_GROUP user_group_id  ;  /* input */
    DRV_SBPM_USER_GROUP_STATUS user_group_status ;/* output */
    DRV_SBPM_ERROR error ;

    /* Get the parameters */
    user_group_id = parm[0].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_user_group_status'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_user_group_status ( user_group_id, &user_group_status ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "User Group %d Configuration:\n\r", user_group_id ) ;
        bdmf_session_print(session, " Ack/Nack status :%s\n\r", f_user_group_status_enum_to_string(user_group_status.ug_status) ) ;
        bdmf_session_print(session, " Exclusive/non-Exclusive status: %s\n\r" ,f_user_group_exclusive_status_enum_to_string(user_group_status.ug_exclusive_status) );
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_user_group_counter_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_USER_GROUP user_group_id  ;  /* input */
    uint16_t user_group_counter ;/* output */
    DRV_SBPM_ERROR error ;

    /* Get the parameters */
    user_group_id = parm[0].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_user_group_counter'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_user_group_counter ( user_group_id, &user_group_counter ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
        bdmf_session_print(session, "User Group %d counter = %d\n\r", user_group_id ,user_group_counter ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_global_counter_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{

    /* Define I/O variables */
    uint16_t global_counter ;/* output */
    DRV_SBPM_ERROR error ;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_global_counter'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_global_counter (  &global_counter ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
        bdmf_session_print(session, "global BN counter = %d \n\r", global_counter ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_set_user_group_mapping_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_SP_USR  source_port;         /* input */
    DRV_SBPM_USER_GROUP user_group_id  ;  /* input */
    DRV_SBPM_ERROR error ;

    /* Get the parameters */
    source_port   = parm[0].value.number;
    user_group_id = parm[1].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_set_user_group_mapping'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_set_user_group_mapping ( source_port , user_group_id ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_user_group_mapping_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_SP_USR     source_port;   /* input */
    DRV_SBPM_USER_GROUP user_group ;  /* output */
    DRV_SBPM_ERROR error ;

    /* Get the parameters */
    source_port = parm[0].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_user_group_mapping'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_user_group_mapping ( source_port, &user_group ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "User Group mapping:\n\r" ) ;
        bdmf_session_print(session, " source port %s mapped to user group %d", f_sbpm_source_port_enum_to_string(source_port), user_group ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_request_buffer_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_SP_USR     source_port;   /* input */
    uint16_t                       buffer_number ;/* output */
    DRV_SBPM_USER_GROUP_STATUS status; /* output */
    DRV_SBPM_ERROR error ;

    /* Get the parameters */
    source_port = parm[0].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_req_buffer'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_request_buffer ( source_port, &buffer_number , &status ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, " SBPM request buffer:\n\r" ) ;
        bdmf_session_print(session, " source port %s allocated buffer %d", f_sbpm_source_port_enum_to_string(source_port), buffer_number ) ;
        bdmf_session_print(session, " status : ack state = %s , exclusive state = %s", f_user_group_status_enum_to_string(status.ug_status), f_user_group_exclusive_status_enum_to_string(status.ug_exclusive_status ) ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_free_buffer_without_context_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_SP_USR     source_port;    /* input */
    uint16_t                       buffer_number ; /* input */
    DRV_SBPM_USER_GROUP_STATUS status;  /* output */
    DRV_SBPM_ERROR error ;

    /* Get the parameters */
    source_port = parm[0].value.number;
    buffer_number =  parm[1].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_free_buffer_without_context'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_free_buffer_without_context ( source_port, buffer_number , &status ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, " BPM Free Buffer:\n\r" ) ;
        bdmf_session_print(session, " source port %s free buffer %d\n\r", f_sbpm_source_port_enum_to_string(source_port), buffer_number ) ;
        bdmf_session_print(session, " status : ack state = %s , exclusive state = %s\n\r", f_user_group_status_enum_to_string(status.ug_status), f_user_group_exclusive_status_enum_to_string(status.ug_exclusive_status ) ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_free_buffer_with_context_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_SP_USR     source_port;    /* input */
    uint32_t                       head_bn ;       /* input */
    uint32_t                       last_bn ;       /* input */
    uint32_t                       size ;       /* input */
    DRV_SBPM_USER_GROUP_STATUS status;  /* output */
    DRV_SBPM_ERROR error ;

    /* Get the parameters */
    source_port = parm[0].value.number;
    head_bn =  parm[1].value.unumber;
    last_bn =  parm[2].value.unumber;
    size =  parm[3].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_free_buffer_without_context'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_free_buffer_with_context ( source_port, head_bn , last_bn, size , &status ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, " BPM Free Buffer with context:\n\r" ) ;
        bdmf_session_print(session, " source port %s free buffer %lu last buffer = %lu, size = %lu\n", f_sbpm_source_port_enum_to_string(source_port), ( long unsigned int ) head_bn , ( long unsigned int ) last_bn, ( long unsigned int ) size  ) ;
        bdmf_session_print(session, " status : ack state = %s , exclusive state = %s", f_user_group_status_enum_to_string(status.ug_status), f_user_group_exclusive_status_enum_to_string(status.ug_exclusive_status ) ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_update_multicast_counter_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint8_t multicast_counter;   /* input */
    uint16_t buffer_ptr ;         /* input */
    DRV_SBPM_ERROR error ;

    /* Get the parameters */
    buffer_ptr =  parm[0].value.unumber;
    multicast_counter =  parm[1].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_mcnt_update'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_mcnt_update ( buffer_ptr , multicast_counter ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_next_bn_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint16_t current_bn ;         /* input */
    uint16_t next_bn ;         /* output */
    uint8_t multicast_value;
    DRV_SBPM_ERROR error ;

    /* Get the parameters */
    current_bn =  parm[0].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_next_bn'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_next_bn ( current_bn , &next_bn , &multicast_value ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, " Get Next BN:\n\r" ) ;
        bdmf_session_print(session, " current bn %d next bn= %d\n",  current_bn , next_bn ) ;
        bdmf_session_print(session, " multicast = %d\n", multicast_value ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_connect_bn_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint16_t current_bn ;         /* input */
    uint16_t pointed_bn ;         /* input */
    DRV_SBPM_ERROR error ;

    /* Get the parameters */
    current_bn =  parm[0].value.unumber;
    pointed_bn =  parm[1].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_connect_bn'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_connect_bn ( current_bn , pointed_bn ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_set_runner_message_control_parameters_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_RUNNER_MSG_CTRL_PARAMS runner_message_control_parameters;   /* input */
    DRV_SBPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    runner_message_control_parameters.status_wakeup_enable =  parm[parameter_index++].value.number;
    runner_message_control_parameters.status_message_enable =  parm[parameter_index++].value.number;
    runner_message_control_parameters.select_runner =  parm[parameter_index++].value.number;
    runner_message_control_parameters.runner_reply_wakeup_task_number=  parm[parameter_index++].value.unumber;
    runner_message_control_parameters.message_base_address =  parm[parameter_index++].value.unumber;
    runner_message_control_parameters.runner_reply_target_address =  parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_set_runner_msg_ctrl'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_set_runner_msg_ctrl ( &runner_message_control_parameters ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_runner_message_control_parameters_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{

    /* Define I/O variables */
    DRV_SBPM_RUNNER_MSG_CTRL_PARAMS runner_message_control_parameters;   /* output */

    /* Define parameters handling variables */
    DRV_SBPM_ERROR error ;
    char * str = "unknown";

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_runner_msg_ctrl'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_runner_msg_ctrl ( &runner_message_control_parameters ) ;
    /* -------------------------------------------------------------------- */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error == DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "Runner Message Control Parameters:\n" ) ;
        bdmf_session_print(session, "status wakeup enable: %s\n", str = runner_message_control_parameters.status_wakeup_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, "status message enable: %s\n", str = runner_message_control_parameters.status_message_enable  ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, "Runner Select recieve Message : %s\n", str = runner_message_control_parameters.select_runner ? "Runner A" : "Runner B" ) ;
        bdmf_session_print(session, "Runner  Reply Wakeup Task Number : %d\n", runner_message_control_parameters.runner_reply_wakeup_task_number  ) ;
        bdmf_session_print(session, "message base address : %d\n", runner_message_control_parameters.message_base_address  ) ;
        bdmf_session_print(session, "Runner  Reply Target Address : %d\n", runner_message_control_parameters.runner_reply_target_address ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_set_runner_wakeup_reply_set_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint8_t wakeup_reply_set_index ;
    DRV_SBPM_WAKEUP_REPLY_SET_PARAMS wakeup_reply_set_parameters;   /* input */
    DRV_SBPM_ERROR error ;
    uint8_t parameter_index ;


    /* Get the parameters */
    parameter_index = 0 ;
    wakeup_reply_set_index =  parm[parameter_index++].value.unumber;
    wakeup_reply_set_parameters.alloc_reply_wakeup_enable =  parm[parameter_index++].value.number;
    wakeup_reply_set_parameters.get_next_reply_wakeup_enable =  parm[parameter_index++].value.number;
    wakeup_reply_set_parameters.connect_reply_wakeup_enable=  parm[parameter_index++].value.number;
    wakeup_reply_set_parameters.mcnt_reply_wakeup_enable =  parm[parameter_index++].value.number;


    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_set_runner_wakeup_reply_set'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_set_runner_wakeup_reply_set ( wakeup_reply_set_index , &wakeup_reply_set_parameters ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_runner_wakeup_reply_set_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint8_t wakeup_reply_set_index ; /*input*/
    DRV_SBPM_WAKEUP_REPLY_SET_PARAMS wakeup_reply_set_parameters;   /*output */
    DRV_SBPM_ERROR error ;
    char * str = "unknown";

    /* Get the parameters */
    wakeup_reply_set_index =  parm[0].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_runner_wakeup_reply_set'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_runner_wakeup_reply_set ( wakeup_reply_set_index , &wakeup_reply_set_parameters ) ;
    /* -------------------------------------------------------------------- */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error == DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "  Wakeup reply set %d  Parameters:\n" ,wakeup_reply_set_index ) ;
        bdmf_session_print(session, "alloc Reply WakeUp Enable: %s\n", str = wakeup_reply_set_parameters.alloc_reply_wakeup_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, " get next Reply WakeUp Enable: %s\n", str = wakeup_reply_set_parameters.get_next_reply_wakeup_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, " connect Reply WakeUp Enable: %s\n", str = wakeup_reply_set_parameters.connect_reply_wakeup_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, " mcnt increament Reply WakeUp Enable: %s\n", str = wakeup_reply_set_parameters.mcnt_reply_wakeup_enable ? "Enabled" : "Disabled" ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_set_mips_d_message_control_parameters_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_MIPS_D_MSG_CTRL_PARAMS mips_d_message_control_parameters;   /* input */
    DRV_SBPM_ERROR error ;
    uint8_t parameter_index ;


    /* Get the parameters */
    parameter_index = 0 ;
    mips_d_message_control_parameters.mips_d_wakeup_reply_set.alloc_reply_wakeup_enable =  parm[parameter_index++].value.number;
    mips_d_message_control_parameters.mips_d_wakeup_reply_set.get_next_reply_wakeup_enable=  parm[parameter_index++].value.number;
    mips_d_message_control_parameters.mips_d_wakeup_reply_set.connect_reply_wakeup_enable =  parm[parameter_index++].value.number;
    mips_d_message_control_parameters.mips_d_wakeup_reply_set.mcnt_reply_wakeup_enable =  parm[parameter_index++].value.number;
    mips_d_message_control_parameters.mips_d_wakeup_reply_set.reply_wakeup_task_number =  parm[parameter_index++].value.unumber;
    mips_d_message_control_parameters.mips_d_reply_target_address =  parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_set_mips_d_msg_ctl'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_set_mips_d_msg_ctrl ( &mips_d_message_control_parameters ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_mips_d_message_control_parameters_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{

    /* Define I/O variables */
    DRV_SBPM_MIPS_D_MSG_CTRL_PARAMS mips_d_message_control_parameters;   /* output */

    /* Define parameters handling variables */
    DRV_SBPM_ERROR error ;
    char * str = "unknown";

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_mips_d_msg_ctrl'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_mips_d_msg_ctrl ( &mips_d_message_control_parameters ) ;
    /* -------------------------------------------------------------------- */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error == DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "MIPS-D Message Control Parameters:\n" ) ;
        bdmf_session_print(session, "MIPS-D alloc Reply WakeUp Enable: %s\n", str = mips_d_message_control_parameters.mips_d_wakeup_reply_set.alloc_reply_wakeup_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, "MIPS-D get next Reply WakeUp Enable: %s\n", str = mips_d_message_control_parameters.mips_d_wakeup_reply_set.get_next_reply_wakeup_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, "MIPS-D connect Reply WakeUp Enable: %s\n", str = mips_d_message_control_parameters.mips_d_wakeup_reply_set.connect_reply_wakeup_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, "MIPS-D mcnt increament Reply WakeUp Enable: %s\n", str = mips_d_message_control_parameters.mips_d_wakeup_reply_set.mcnt_reply_wakeup_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, "MIPS-D Reply Wakeup Task Number : %d\n", mips_d_message_control_parameters.mips_d_wakeup_reply_set.reply_wakeup_task_number  ) ;
        bdmf_session_print(session, "MIPS-D Reply Target Address : %d\n", mips_d_message_control_parameters.mips_d_reply_target_address  ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_set_error_handling_parameters_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_ERROR_HANDLING_PARAMETERS error_handling_parameters;   /* input */
    DRV_SBPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    error_handling_parameters.search_deapth =  parm[parameter_index++].value.unumber;
    error_handling_parameters.max_search_enable =  parm[parameter_index++].value.number;
    error_handling_parameters.check_last_enable =  parm[parameter_index++].value.number;
    error_handling_parameters.freeze_counters =  parm[parameter_index++].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_set_error_handling_parameters'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_set_error_handling_parameters ( &error_handling_parameters ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_error_handling_parameters_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{

    /* Define I/O variables */
    DRV_SBPM_ERROR_HANDLING_PARAMETERS error_handling_parameters;   /* output */

    /* Define parameters handling variables */
    DRV_SBPM_ERROR error ;
    char * str = "unknown";

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_error_handling_parameters'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_error_handling_parameters ( &error_handling_parameters ) ;
    /* -------------------------------------------------------------------- */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if ( error == DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "Error Handling Parameters:\n" ) ;
        bdmf_session_print(session, "Max search deapth: %d\n", error_handling_parameters.search_deapth  ) ;
        bdmf_session_print(session, "Max search Enable: %s\n", str = error_handling_parameters.max_search_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, "Check last error Enable: %s\n", str = error_handling_parameters.check_last_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, "Freeze Counters Enable: %s\n", str = error_handling_parameters.freeze_counters ? "Enabled" : "Disabled" ) ;
   }

   return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_interrupt_status_register_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_ISR sbpm_isr;   /* input */
    DRV_SBPM_ERROR error ;
    char * str = "unknown";

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_interrupt_status_register'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_interrupt_status_register ( &sbpm_isr ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if (error == DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "SBPM Interrupt status register:\n" ) ;
        bdmf_session_print(session, "bac_underrun interrupt status: %s\n" , str = ( sbpm_isr.bac_underrun? "On" : "Off" ) ) ;
        bdmf_session_print(session, "multicast_counter_overflow interrupt status: %s\n" , str = ( sbpm_isr.multicast_counter_overflow? "On" : "Off" ) ) ;
        bdmf_session_print(session, "check_last_error interrupt status: %s\n" , str = ( sbpm_isr.check_last_error? "On" : "Off" ) ) ;
        bdmf_session_print(session, "max_search_error interrupt status: %s\n" , str = ( sbpm_isr.max_search_error? "On" : "Off" ) ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_clear_interrupt_status_register_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    DRV_SBPM_ERROR error ;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_clear_interrupt_status_register'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_clear_interrupt_status_register (  ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_set_interrupt_enable_register_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_ISR sbpm_ier;   /* input */
    DRV_SBPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    sbpm_ier.bac_underrun =  parm[parameter_index++].value.number;
    sbpm_ier.multicast_counter_overflow =  parm[parameter_index++].value.number;
    sbpm_ier.check_last_error=  parm[parameter_index++].value.number;
    sbpm_ier.max_search_error =  parm[parameter_index++].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_set_interrupt_enable_register'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_set_interrupt_enable_register ( &sbpm_ier ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_interrupt_enable_register_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_ISR sbpm_ier;   /* input */
    DRV_SBPM_ERROR error;
    char * str = "unknown";

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_interrupt_enable_register'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_interrupt_enable_register ( &sbpm_ier ) ;
    /* -------------------------------------------------------------------- */

    if ( error == DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "BPM Interrupt enable register:\n" ) ;
        bdmf_session_print(session, "bac_underrun interupt status: %s\n" , str = ( sbpm_ier.bac_underrun? "Enable" : "Disable" ) ) ;
        bdmf_session_print(session, "check_last_error interrupt status: %s\n" , str = ( sbpm_ier.check_last_error? "Enable" : "Disable" ) ) ;
        bdmf_session_print(session, "max_search_error interupt status: %s\n" , str = ( sbpm_ier.max_search_error? "Enable" : "Disable" ) ) ;
        bdmf_session_print(session, "multicast counter interrupt status: %s\n" , str = ( sbpm_ier.multicast_counter_overflow? "Enable" : "Disable" ) ) ;
    }
    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_generate_interrupt_test_register_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_ISR sbpm_itr;   /* input */
    DRV_SBPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    sbpm_itr.bac_underrun =  parm[parameter_index++].value.number;
    sbpm_itr.multicast_counter_overflow =  parm[parameter_index++].value.number;
    sbpm_itr.check_last_error=  parm[parameter_index++].value.number;
    sbpm_itr.max_search_error =  parm[parameter_index++].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_generate_interrupt_test_register'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_generate_interrupt_test_register ( &sbpm_itr ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_get_interrupt_information_register_command(bdmf_session_handle  session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_SBPM_IIR sbpm_iir;   /* input */
    DRV_SBPM_ERROR error ;


    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_sbpm_get_interrupt_information_register'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_sbpm_get_interrupt_information_register ( &sbpm_iir ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    if (error == DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "SBPM Interrupt information register:\n" ) ;
        bdmf_session_print(session, "command SA =  %d\n" ,sbpm_iir.cmd_sa ) ;
        bdmf_session_print(session, "command type =  %d\n" ,sbpm_iir.cmd_type ) ;
        bdmf_session_print(session, "command data =  %x %x\n",
            (unsigned)sbpm_iir.cmd_data[1], (unsigned)sbpm_iir.cmd_data[0] ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_sbpm_dump_all_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{

    DRV_SBPM_ERROR error ;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'f_dump_sbpm_database'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = f_dump_sbpm_database ( session ) ;
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s' \n\r", f_drv_sbpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static DRV_SBPM_ERROR f_dump_sbpm_database (bdmf_session_handle  session)
{

    DRV_SBPM_SP_USR  source_port;
    DRV_SBPM_USER_GROUP user_group_index;
    DRV_SBPM_USER_GROUP_CONFIGURATION user_group_configuration  ;
    DRV_SBPM_USER_GROUP_STATUS user_group_status;
    DRV_SBPM_ERROR_HANDLING_PARAMETERS error_handling_parameters;
    DRV_SBPM_ISR sbpm_isr;
    DRV_SBPM_ERROR error;
    char * str = "unknown";
    uint16_t global_hysteresis ;
    uint16_t global_threshold ;
    uint16_t global_counter ;
    uint16_t user_group_counter;


    bdmf_session_print(session, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r" ) ;
    bdmf_session_print(session, "\n\r                      ++++ SBPM Configuration ++++                         \n\n\r" ) ;
    bdmf_session_print(session, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n\n\r" ) ;



    bdmf_session_print(session, "\n\r                           Global Configuration                             \n\r" ) ;
    bdmf_session_print(session, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r" ) ;

    error = fi_bl_drv_sbpm_get_global_threshold ( &global_threshold, &global_hysteresis );

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "Global Threshold Configuration:\n\r" ) ;
        bdmf_session_print(session, "Global Threshold :%d\n\r",global_threshold  ) ;
        bdmf_session_print(session, "Global Threshold hysteresis: %d\n\r" ,global_hysteresis ) ;
    }
    else
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_get_global_counter ( &global_counter ) ;

    if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, " Global allocated BN counter =  %d\n", global_counter  ) ;
    }
    else
    {
        return ( error );
    }


    bdmf_session_print(session, "\n\r                          User Group Configuration                             \n\r" ) ;
    bdmf_session_print(session, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r" ) ;

    for ( source_port = DRV_SBPM_SP_GPON ; source_port < DRV_SBPM_SP_NUMBER_OF_SOURCE_PORTS ; source_port++ )
    {
            error = fi_bl_drv_sbpm_get_user_group_mapping ( source_port, &user_group_index ) ;

            if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
            {
                bdmf_session_print(session, " source port %s mapped to user group %d\n", f_sbpm_source_port_enum_to_string(source_port), user_group_index ) ;
            }
            else
            {
                return ( error );
            }
    }

   bdmf_session_print(session, "\n\r" ) ;
   bdmf_session_print(session, "\n\r" ) ;

    for ( user_group_index = 0 ; user_group_index < DRV_SBPM_NUMBER_OF_USER_GROUPS ; user_group_index++ )
    {
            bdmf_session_print(session, "User Group %d Configuration:\n\r", user_group_index ) ;
            bdmf_session_print(session, "---------------------------------\n\n\r" ) ;

            error = fi_bl_drv_sbpm_get_user_group_thresholds ( user_group_index, &user_group_configuration ) ;

            if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
            {
                bdmf_session_print(session, " Threshold: %u\n\r", (unsigned)user_group_configuration.threshold ) ;
                bdmf_session_print(session, " hysteresis: %u\n\r" ,(unsigned)user_group_configuration.hysteresis ) ;
                bdmf_session_print(session, " Exclusive Threshold :%u\n\r", (unsigned)user_group_configuration.exclusive_threshold ) ;
                bdmf_session_print(session, " Exclusive hysteresis: %u\n\r", (unsigned)user_group_configuration.exclusive_hysteresis ) ;
            }
            else
            {
                return ( error );
            }

            bdmf_session_print(session, "\n\r" ) ;

            error = fi_bl_drv_sbpm_get_user_group_status ( user_group_index, &user_group_status ) ;

            if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
            {
                bdmf_session_print(session, " Ack/Nack status: %s\n\r", f_user_group_status_enum_to_string(user_group_status.ug_status) ) ;
                bdmf_session_print(session, " Exclusive/non-Exclusive status: %s\n\r" ,f_user_group_exclusive_status_enum_to_string(user_group_status.ug_exclusive_status) );
            }
            else
            {
                return ( error );
            }

            bdmf_session_print(session, "\n\r" ) ;

            error = fi_bl_drv_sbpm_get_user_group_counter ( user_group_index, &user_group_counter ) ;

            if ( error ==  DRV_SBPM_ERROR_NO_ERROR )
            {
                bdmf_session_print(session, "User Group %d counter = %d \n\r", user_group_index ,user_group_counter ) ;
            }
            else
            {
                return ( error );
            }

            bdmf_session_print(session, "\n\r" ) ;
            bdmf_session_print(session, "\n\r" ) ;
    }


    bdmf_session_print(session, "\n\r                           Interrupt Status                              \n\r" ) ;
    bdmf_session_print(session, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r" ) ;


    error = fi_bl_drv_sbpm_get_interrupt_enable_register ( &sbpm_isr ) ;

    if ( error == DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "SBPM Interrupt enable register:\n" ) ;
        bdmf_session_print(session, "bac_underrun interrupt: %s\n" , str = ( sbpm_isr.bac_underrun? "Enable" : "Disable" ) ) ;
        bdmf_session_print(session, "check_last_error  interrupt: %s\n" , str = ( sbpm_isr.check_last_error? "Enable" : "Disable" ) ) ;
        bdmf_session_print(session, "max_search_error interrupt: %s\n" , str = ( sbpm_isr.max_search_error? "Enable" : "Disable" ) ) ;
        bdmf_session_print(session, "multicast counter oberflow interrupt: %s\n" , str = ( sbpm_isr.multicast_counter_overflow? "Enable" : "Disable" ) ) ;

    }
    else
    {
        return ( error );
    }

    error = fi_bl_drv_sbpm_get_interrupt_status_register ( &sbpm_isr ) ;
    if ( error == DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "SBPM Interrupt status register:\n" ) ;
        bdmf_session_print(session, "SBPM Interrupt enable register:\n" ) ;
        bdmf_session_print(session, "bac_underrun interrupt: %s\n" , str = ( sbpm_isr.bac_underrun? "On" : "Off" ) ) ;
        bdmf_session_print(session, "check_last_error  interrupt: %s\n" , str = ( sbpm_isr.check_last_error? "On" : "Off" ) ) ;
        bdmf_session_print(session, "max_search_error interrupt: %s\n" , str = ( sbpm_isr.max_search_error? "On" : "Off" ) ) ;
        bdmf_session_print(session, "multicast counter oberflow interrupt: %s\n" , str = ( sbpm_isr.multicast_counter_overflow? "On" : "Off" ) ) ;

    }
    else
    {
        return ( error );
    }

    bdmf_session_print(session, "\n\r                Error Handling Parameters                               \n\r" ) ;
    bdmf_session_print(session, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r" ) ;


    error = fi_bl_drv_sbpm_get_error_handling_parameters ( &error_handling_parameters ) ;

    if ( error == DRV_SBPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "SBPM Error handling Parameters:\n" ) ;
        bdmf_session_print(session, "check_last_enable : %s\n" , str = (error_handling_parameters.check_last_enable? "Enable" : "Disable" ) ) ;
        bdmf_session_print(session, "max_search_enable  interrupt: %s\n" , str = ( error_handling_parameters.max_search_enable? "Enable" : "Disable" ) ) ;
        bdmf_session_print(session, "freeze_counters interrupt: %s\n" , str = ( error_handling_parameters.freeze_counters? "Enable" : "Disable" ) ) ;
        bdmf_session_print(session, "max search deapth = %d\n" , error_handling_parameters.search_deapth ) ;

    }
    else
    {
        return ( error );
    }

    return ( DRV_SBPM_ERROR_NO_ERROR ) ;

}

#define SBPM_MAX_BN     0x3fff      /* IT: verify */

static bdmfmon_handle_t sbpm_dir;

void pi_bl_initialize_drv_sbpm_shell(bdmfmon_handle_t driver_dir)
{
    static struct bdmfmon_enum_val src_port_enum_table[] = {
        {"gpon",  DRV_SBPM_SP_GPON},
        {"emac0", DRV_SBPM_SP_EMAC0},
        {"emac1", DRV_SBPM_SP_EMAC1},
        {"emac2", DRV_SBPM_SP_EMAC2},
        {"emac3", DRV_SBPM_SP_EMAC3},
        {"emac4", DRV_SBPM_SP_EMAC4},
        {"cpu",   DRV_SBPM_SP_MIPS_C},
        {"rnr_a", DRV_SBPM_SP_RNR_A},
        {"rnr_b", DRV_SBPM_SP_RNR_B},
        {"mips_d",DRV_SBPM_SP_MIPS_D},
        {"pci_0", DRV_SBPM_SP_PCI0},
        {"pci_1", DRV_SBPM_SP_PCI1},
        {"usb_0", DRV_SBPM_SP_USB0},
        {"usb_1", DRV_SBPM_SP_USB1},
        {NULL, 0},
    };
    static struct bdmfmon_enum_val runner_enum_table[] = {
        {"A", 0},
        {"B", 1},
        { NULL, 0}
    };

    sbpm_dir = bdmfmon_dir_add(driver_dir, "sbpm", "SBPM Driver", BDMF_ACCESS_ADMIN, NULL );
    if (!sbpm_dir)
    {
        bdmf_session_print(NULL, "Can't create sbpm directory\n");
        return;
    }

    BDMFMON_MAKE_CMD(sbpm_dir, "sbpmi", "SBPM Initialize", p_bl_drv_sbpm_initialize_command,
        BDMFMON_MAKE_PARM_RANGE("ba", "Base address (16 bit)", BDMFMON_PARM_NUMBER, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM_RANGE("ls", "List size (16 bit)", BDMFMON_PARM_NUMBER, 0, 0, 0xffff),
        BDMFMON_MAKE_PARM("thresh", "Global threshold", BDMFMON_PARM_NUMBER, 0 ),
        BDMFMON_MAKE_PARM("hyst", "Global hysteresis", BDMFMON_PARM_NUMBER, 0 ),
        BDMFMON_MAKE_PARM_RANGE("ug", "User group configuration", BDMFMON_PARM_NUMBER, 0, 0, 1 ) );

    BDMFMON_MAKE_CMD(sbpm_dir, "esp", "Enable source port", p_bl_drv_sbpm_sp_enable_command,
        BDMFMON_MAKE_PARM_ENUM("eth0_sp", "eth0 port", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth1_sp", "eth1 port", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth2_sp", "eth2 port", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth3_sp", "eth3 port", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("eth4_sp", "eth4 port", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("wan",   "gpon / eth5", bdmfmon_enum_bool_table, 0) );

    BDMFMON_MAKE_CMD(sbpm_dir, "sgt", "Set Global Threshold",p_bl_drv_sbpm_set_global_threshold_command,
        BDMFMON_MAKE_PARM("thresh", "Global threshold", BDMFMON_PARM_NUMBER, 0 ),
        BDMFMON_MAKE_PARM("hyst", "Global hysteresis", BDMFMON_PARM_NUMBER, 0 ));

    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "ggt", "Get Global Threshold", p_bl_drv_sbpm_get_global_threshold_command);

    BDMFMON_MAKE_CMD(sbpm_dir, "sugt", "Set User Group Threshold", p_bl_drv_sbpm_set_user_group_thresholds_command,
        BDMFMON_MAKE_PARM_RANGE("ug", "User group id", BDMFMON_PARM_NUMBER, 0, 0, 7),
        BDMFMON_MAKE_PARM("thresh", "User group threshold", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("hyst", "User group hysteresis", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("ex_thresh", "User group exclusive threshold", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("ex_hyst", "User group exclusive hysteresis", BDMFMON_PARM_NUMBER, 0));

    BDMFMON_MAKE_CMD(sbpm_dir, "gugt", "Get User Group Threshold", p_bl_drv_sbpm_get_user_group_thresholds_command,
        BDMFMON_MAKE_PARM_RANGE("ug", "User group id", BDMFMON_PARM_NUMBER, 0, 0, 7));

    BDMFMON_MAKE_CMD(sbpm_dir, "gugs", "Get User Group Status", p_bl_drv_sbpm_get_user_group_status_command,
        BDMFMON_MAKE_PARM_RANGE("ug", "User group id", BDMFMON_PARM_NUMBER, 0, 0, 7));

    BDMFMON_MAKE_CMD(sbpm_dir, "gugc", "Get User Group Counter", p_bl_drv_sbpm_get_user_group_counter_command,
        BDMFMON_MAKE_PARM_RANGE("ug", "User group id", BDMFMON_PARM_NUMBER, 0, 0, 7));

    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "ggbc", "Get Global Counter", p_bl_drv_sbpm_get_global_counter_command);

    BDMFMON_MAKE_CMD(sbpm_dir, "sugm", "Set User Group Mapping", p_bl_drv_sbpm_set_user_group_mapping_command,
        BDMFMON_MAKE_PARM_ENUM("port", "Source port", src_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("ug", "User group id", BDMFMON_PARM_NUMBER, 0, 0, 7));

    BDMFMON_MAKE_CMD(sbpm_dir, "gugm", "Get User Group Mapping", p_bl_drv_sbpm_get_user_group_mapping_command,
        BDMFMON_MAKE_PARM_ENUM("port", "Source port", src_port_enum_table, 0));

    BDMFMON_MAKE_CMD(sbpm_dir, "rbuf", "Request Buffer", p_bl_drv_sbpm_request_buffer_command,
        BDMFMON_MAKE_PARM_ENUM("port", "Source port", src_port_enum_table, 0));

    BDMFMON_MAKE_CMD(sbpm_dir, "fbuf", "Free Buffer without context", p_bl_drv_sbpm_free_buffer_without_context_command,
        BDMFMON_MAKE_PARM_ENUM("port", "Source port", src_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("bn",  "Buffer number", BDMFMON_PARM_NUMBER, 0, 0, SBPM_MAX_BN));

    BDMFMON_MAKE_CMD(sbpm_dir, "fbufc", "Free Buffer with context", p_bl_drv_sbpm_free_buffer_with_context_command,
        BDMFMON_MAKE_PARM_ENUM("port", "Source port", src_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("head_bn",  "Head BN of the packet to be freed", BDMFMON_PARM_NUMBER, 0, 0, SBPM_MAX_BN),
        BDMFMON_MAKE_PARM_RANGE("last_bn",  "Last BN of the packet to be freed", BDMFMON_PARM_NUMBER, 0, 0, SBPM_MAX_BN),
        BDMFMON_MAKE_PARM_RANGE("size",  "Number of BN of the packet to be freed", BDMFMON_PARM_NUMBER, 0, 0, SBPM_MAX_BN));

    BDMFMON_MAKE_CMD(sbpm_dir, "umc",  "Update Multicast Counter", p_bl_drv_sbpm_update_multicast_counter_command,
        BDMFMON_MAKE_PARM_RANGE("bn",  "Buffer number", BDMFMON_PARM_NUMBER, 0, 0, SBPM_MAX_BN),
        BDMFMON_MAKE_PARM_RANGE("mc",  "Multicast counter", BDMFMON_PARM_NUMBER, 0, 0, 7));

    BDMFMON_MAKE_CMD(sbpm_dir, "gnbn", "Get next BN", p_bl_drv_sbpm_get_next_bn_command,
        BDMFMON_MAKE_PARM_RANGE("bn",  "Current buffer", BDMFMON_PARM_NUMBER, 0, 0, SBPM_MAX_BN));

    BDMFMON_MAKE_CMD(sbpm_dir, "cbn",  "Connect BN", p_bl_drv_sbpm_connect_bn_command,
        BDMFMON_MAKE_PARM_RANGE("bn",  "Current buffer", BDMFMON_PARM_NUMBER, 0, 0, SBPM_MAX_BN),
        BDMFMON_MAKE_PARM_RANGE("bn",  "Pointer buffer", BDMFMON_PARM_NUMBER, 0, 0, SBPM_MAX_BN));

    BDMFMON_MAKE_CMD(sbpm_dir, "sehp", "Set Error handling Parameters", p_bl_drv_sbpm_set_error_handling_parameters_command,
        BDMFMON_MAKE_PARM("depth", "Search depth :  maximal threshold for search during Free with context", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_ENUM("max_search", "enable max search during Free with context", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("check_last", "enable checking last BN during Free with context", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("check_last", "enable freeze User Groups / global counters", bdmfmon_enum_bool_table, 0));

    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "gehp", "Get Error handling Parameters", p_bl_drv_sbpm_get_error_handling_parameters_command);

    BDMFMON_MAKE_CMD(sbpm_dir, "srmcp", "Set Runner Control Parameters", p_bl_drv_sbpm_set_runner_message_control_parameters_command,
        BDMFMON_MAKE_PARM_ENUM("status_wakeup_enable", "Enable wake-up message after each reply on request from Runner", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("status_message_enable", "Enable wstatus message", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("runner", "Select Runner", runner_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("task",  "Task number for runner used in SBPM Status Message Wake-Up", BDMFMON_PARM_NUMBER, 0, 0, 31),
        BDMFMON_MAKE_PARM("base",  "Message base address", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("ta",  "Target address for both Runners", BDMFMON_PARM_NUMBER, 0));

    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "grmcp", "Get Runner Control Parameters", p_bl_drv_sbpm_get_runner_message_control_parameters_command);

    BDMFMON_MAKE_CMD(sbpm_dir, "srwrs", "Set Runner Wakup reply set", p_bl_drv_sbpm_set_runner_wakeup_reply_set_command,
        BDMFMON_MAKE_PARM("set", "Set index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_ENUM("alloc", "Wake-up on Alloc reply", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("get_next", "Wake-up on Get_next reply", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("connect", "Wake-up on Connect reply", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("mcnt", "Wake-up on mcnt increment reply", bdmfmon_enum_bool_table, 0));

    BDMFMON_MAKE_CMD(sbpm_dir, "grwrs", "Get Runner Wakup reply set", p_bl_drv_sbpm_get_runner_wakeup_reply_set_command,
        BDMFMON_MAKE_PARM("set", "Set index", BDMFMON_PARM_NUMBER, 0));

    BDMFMON_MAKE_CMD(sbpm_dir, "smdmcp", "Set MIPS-D Control Parameters", p_bl_drv_sbpm_set_mips_d_message_control_parameters_command,
        BDMFMON_MAKE_PARM_ENUM("alloc", "Wake-up on Alloc reply", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("get_next", "Wake-up on Get_next reply", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("connect", "Wake-up on Connect reply", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("mcnt", "Wake-up on mcnt increment reply", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_RANGE("task",  "Runner reply wkup task number", BDMFMON_PARM_NUMBER, 0, 0, 31),
        BDMFMON_MAKE_PARM("ta",  "Alloc request reply target address", BDMFMON_PARM_NUMBER, 0));

    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "gmdmcp", "Get MIPS-D Control Parameters", p_bl_drv_sbpm_get_mips_d_message_control_parameters_command);

    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "gisr", "Get Interrupt Status Register", p_bl_drv_sbpm_get_interrupt_status_register_command);

    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "cisr", "Clear Interrupt Status Register", p_bl_drv_sbpm_clear_interrupt_status_register_command);

    BDMFMON_MAKE_CMD(sbpm_dir, "sier", "Set Interrupt Enable Register", p_bl_drv_sbpm_set_interrupt_enable_register_command,
        BDMFMON_MAKE_PARM_ENUM("bac_underrun", "bac_underrun interrupt", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("mcast", "multicast_counter_overflow interrupt", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("check_last_error", "check_last_error interrupt", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("max_search_error", "max_search_error interrupt", bdmfmon_enum_bool_table, 0));

    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "gier", "Get Interrupt Enable Register", p_bl_drv_sbpm_get_interrupt_enable_register_command);

    BDMFMON_MAKE_CMD(sbpm_dir, "gitr", "Generate Interrupt Test Register", p_bl_drv_sbpm_generate_interrupt_test_register_command,
        BDMFMON_MAKE_PARM_ENUM("bac_underrun", "bac_underrun interrupt", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("mcast", "multicast_counter_overflow interrupt", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("check_last_error", "check_last_error interrupt", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("max_search_error", "max_search_error interrupt", bdmfmon_enum_bool_table, 0));

    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "giir", "Get Interrupt Information Register", p_bl_drv_sbpm_get_interrupt_information_register_command);

    BDMFMON_MAKE_CMD_NOPARM(sbpm_dir, "dac", "Dump All SBPM Configuration", p_bl_drv_sbpm_dump_all_configuration_command);
}

void pi_bl_exit_drv_sbpm_shell ( void )
{
    if (sbpm_dir)
    {
        bdmfmon_token_destroy(sbpm_dir);
        sbpm_dir = NULL;
    }
}

char * f_drv_sbpm_error_code_to_string ( DRV_SBPM_ERROR xi_error_code )
{
    switch ( xi_error_code )
    {
    case DRV_SBPM_ERROR_NO_ERROR:
        return "DRV_SBPM_ERROR_NO_ERROR" ;
        break ;
    case DRV_SBPM_ERROR_SBPM_BUSY:
        return "DRV_SBPM_ERROR_SBPM_BUSY" ;
        break ;
    case DRV_SBPM_ERROR_NO_FREE_BUFFER:
        return "DRV_SBPM_ERROR_NO_FREE_BUFFER" ;
        break ;
    case DRV_SBPM_ERROR_INVALID_SOURCE_PORT:
        return "DRV_SBPM_ERROR_INVALID_SOURCE_PORT" ;
        break ;
    case DRV_SBPM_ERROR_INVALID_USER_GROUP:
        return "DRV_SBPM_ERROR_INVALID_USER_GROUP" ;
        break ;
    case DRV_SBPM_ERROR_BUFFER_NUMBER_EXCEEDS_MAXIMUM_VALUE :
        return "DRV_SBPM_ERROR_BUFFER_NUMBER_EXCEEDS_MAX_VALUE" ;
        break ;
    case DRV_SBPM_ERROR_TRYING_TO_CONNECT_TO_NULL_POINTER :
        return "DRV_SBPM_ERROR_TRYING_TO_CONNECT_TO_NULL_POINTER" ;
        break ;
    case DRV_SBPM_ERROR_TRYING_TO_FREE_NULL_POINTER :
        return "DRV_SBPM_ERROR_TRYING_TO_FREE_NULL_POINTER" ;
        break ;
    case DRV_SBPM_ERROR_MULTICAST_VALUE_EXCEEDS_MAXIMUM_VALUE :
        return "DRV_SBPM_ERROR_MULTICAST_VALUE_EXCEEDS_MAX_VALUE" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}

char * f_sbpm_source_port_enum_to_string ( DRV_SBPM_SP_USR xi_sp )
{
    switch ( xi_sp )
    {
    case DRV_SBPM_SP_GPON:
        return "GPON" ;
        break ;
    case DRV_SBPM_SP_EMAC0:
        return "Emac 0" ;
        break ;
    case DRV_SBPM_SP_EMAC1:
        return "Emac 1" ;
        break ;
    case DRV_SBPM_SP_EMAC2:
        return "Emac 2" ;
        break ;
    case DRV_SBPM_SP_EMAC3:
        return "Emac 3" ;
        break ;
    case DRV_SBPM_SP_EMAC4:
        return "Emac 4" ;
        break ;
    case DRV_SBPM_SP_MIPS_C:
        return "CPU" ;
        break ;
    case DRV_SBPM_SP_MIPS_D:
        return "MIPS D" ;
        break ;
    case DRV_SBPM_SP_PCI0:
        return "PCI 0" ;
        break ;
    case DRV_SBPM_SP_PCI1:
        return "PCI 1" ;
        break ;
    case DRV_SBPM_SP_USB0:
        return "USB 0" ;
        break ;
    case DRV_SBPM_SP_USB1:
        return "USB 1" ;
        break ;
    case DRV_SBPM_SP_RNR_A:
        return "Runner A" ;
        break ;
    case DRV_SBPM_SP_RNR_B:
        return "Runner B" ;
        break ;
    case DRV_SBPM_SP_SPARE_0:
        return "Spare 0" ;
        break;
    case DRV_SBPM_SP_SPARE_1:
        return "Spare 1" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}

static char * f_user_group_status_enum_to_string ( E_DRV_SBPM_ENABLE xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_SBPM_DISABLE:
        return " NACK" ;
        break ;
    case DRV_SBPM_ENABLE:
        return " ACK" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}

static char * f_user_group_exclusive_status_enum_to_string ( E_DRV_SBPM_ENABLE xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_SBPM_DISABLE:
        return " Non-Exclusive" ;
        break ;
    case DRV_SBPM_ENABLE:
        return " Exclusive" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}

#endif



