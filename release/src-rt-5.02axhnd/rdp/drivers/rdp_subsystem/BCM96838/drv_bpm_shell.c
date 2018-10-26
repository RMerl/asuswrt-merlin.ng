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

#define BDMF_SESSION_DISABLE_FORMAT_CHECK
#include "rdp_drv_bpm.h"
#include "bdmf_shell.h"


#ifdef USE_BDMF_SHELL

/*****************************************************************************/
/*                                                                           */
/* Static Functions declaration                                              */
/*                                                                           */
/*****************************************************************************/
char * f_drv_bpm_error_code_to_string ( DRV_BPM_ERROR xi_error_code ) ;
static char * f_user_group_status_enum_to_string ( E_DRV_BPM_ENABLE xi_enum_value );
static char * f_user_group_exclusive_status_enum_to_string ( E_DRV_BPM_ENABLE xi_enum_value );
static char * f_global_threshold_enum_to_string ( DRV_BPM_GLOBAL_THRESHOLD xi_enum_value );
static DRV_BPM_ERROR f_dump_bpm_database(bdmf_session_handle session);
char * f_source_port_enum_to_string ( DRV_BPM_SP_USR xi_sp );

static int p_bl_drv_bpm_initialize_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_GLOBAL_CONFIGURATION    global_configuration;
    DRV_BPM_USER_GROUPS_THRESHOLDS  ug_configuration;
    DRV_BPM_ERROR error ;

    uint8_t parameter_index ;

    memset ( & ug_configuration, 0, sizeof (DRV_BPM_USER_GROUPS_THRESHOLDS ) ) ;

    /* Get the parameters */
    parameter_index = 0 ;
    global_configuration.threshold = parm[parameter_index++].value.number ;
    global_configuration.hysteresis = parm[parameter_index++].value.unumber;
    switch ( parm[parameter_index++].value.unumber )
    {
    case 0:
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_0].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_2].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_3].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_4].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_5].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_6].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_7].hysteresis = 0x200;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_0].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_2].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_3].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_4].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_5].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_6].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_7].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_0].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_2].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_3].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_4].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_5].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_6].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_7].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_0].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_2].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_3].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_4].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_5].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_6].exclusive_hysteresis = 0x100;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_7].exclusive_hysteresis = 0x100;
            break;
    case 1:

            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_0].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_2].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_3].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_4].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_5].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_6].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_7].hysteresis = 0x700;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_0].threshold = 0x2400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].threshold = 0x2400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_2].threshold = 0x2400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_3].threshold = 0x2400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_4].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_5].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_6].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_7].threshold = 0x1400;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_0].exclusive_threshold = 0xFFFF;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].exclusive_threshold = 0x2000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_2].exclusive_threshold = 0x2000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_3].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_4].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_5].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_6].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_7].exclusive_threshold = 0x1000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_0].exclusive_hysteresis = 0x500;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].exclusive_hysteresis = 0x420;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_2].exclusive_hysteresis = 0x660;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_3].exclusive_hysteresis = 0x220;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_4].exclusive_hysteresis = 0x000;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_5].exclusive_hysteresis = 0x101;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_6].exclusive_hysteresis = 0xFFF;
            ug_configuration.ug_arr[DRV_BPM_USER_GROUP_7].exclusive_hysteresis = 0xFFFF;

            break;
    }

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_init'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_init ( &global_configuration, &ug_configuration, 0x0AA04, DRV_SPARE_BN_MESSAGE_FORMAT_14_bit_BN_WIDTH ) ;
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_sp_enable_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_SP_USR source_port ; /* input */
    E_DRV_BPM_ENABLE control;  /* input */
    DRV_BPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    source_port = parm[parameter_index++].value.number;
    control = parm[parameter_index++].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_sp_enable'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_sp_enable ( source_port, control ) ;
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_set_global_threshold_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint32_t hysteresis ;/* input */
    DRV_BPM_GLOBAL_THRESHOLD threshold  ;  /* input */
    DRV_BPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    threshold = parm[parameter_index++].value.number ;
    hysteresis = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_set_global_threshold'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error =  fi_bl_drv_bpm_set_global_threshold ( threshold, hysteresis ) ;
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_get_global_threshold_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint32_t hysteresis ;/* output */
    DRV_BPM_GLOBAL_THRESHOLD threshold  ;  /* output */

    DRV_BPM_ERROR error  ;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'pi_bl_drv_bpm_set_global_threshold'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_get_global_threshold ( &threshold, &hysteresis ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "Global Threshold Configuration:\n\r" ) ;
        bdmf_session_print(session, "Global Threshold :%s\n\r", f_global_threshold_enum_to_string(threshold) ) ;
        bdmf_session_print(session, "Global Threshold hysteresis: %d\n\r" ,hysteresis ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_set_user_group_thresholds_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_USER_GROUP_CONFIGURATION user_group_configuration ;/* input */
    DRV_BPM_USER_GROUP user_group_id  ;  /* input */
    DRV_BPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    user_group_id = parm[parameter_index++].value.unumber; ;
    user_group_configuration.hysteresis = parm[parameter_index++].value.unumber;
    user_group_configuration.threshold = parm[parameter_index++].value.unumber;
    user_group_configuration.exclusive_hysteresis = parm[parameter_index++].value.unumber;
    user_group_configuration.exclusive_threshold = parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'pi_bl_drv_bpm_set_user_group_thresholds'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_set_user_group_thresholds ( user_group_id, &user_group_configuration ) ;
    /* -------------------------------------------------------------------- */

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_get_user_group_thresholds_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_USER_GROUP user_group_id  ;  /* input */
    DRV_BPM_USER_GROUP_CONFIGURATION user_group_configuration ;/* output */
    DRV_BPM_ERROR error ;

    /* Get the parameters */
    user_group_id = parm[0].value.unumber; ;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'pi_bl_drv_bpm_get_user_group_thresholds'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_get_user_group_thresholds ( user_group_id, &user_group_configuration ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "User Group %d Configuration:\n\r", user_group_id ) ;
        bdmf_session_print(session, " Threshold :%d\n\r", user_group_configuration.threshold)  ;
        bdmf_session_print(session, " hysteresis: %d\n\r" ,user_group_configuration.hysteresis ) ;
        bdmf_session_print(session, " Exclusive Threshold :%d\n\r", user_group_configuration.exclusive_threshold ) ;
        bdmf_session_print(session, " Exclusive hysteresis: %d\n\r" ,user_group_configuration.exclusive_hysteresis ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_get_user_group_status_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_USER_GROUP user_group_id  ;  /* input */
    DRV_BPM_USER_GROUP_STATUS user_group_status ;/* output */
    DRV_BPM_ERROR error ;

    /* Get the parameters */
    user_group_id = parm[0].value.unumber; ;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_get_user_group_status'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_get_user_group_status ( user_group_id, &user_group_status ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "User Group %d Configuration:\n\r", user_group_id ) ;
        bdmf_session_print(session, " Ack/Nack status :%s\n\r", f_user_group_status_enum_to_string(user_group_status.ug_status) ) ;
        bdmf_session_print(session, " Exclusive/non-Exclusive status: %s\n\r" ,f_user_group_exclusive_status_enum_to_string(user_group_status.ug_exclusive_status) );
    }
    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_get_user_group_counter_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_USER_GROUP user_group_id  ;  /* input */
    uint16_t user_group_counter ;/* output */
    DRV_BPM_ERROR error ;

    /* Get the parameters */
    user_group_id = parm[0].value.unumber  ;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_get_user_group_counter'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_get_user_group_counter ( user_group_id, &user_group_counter ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "User Group %d counter = %lu \n\r", user_group_id ,user_group_counter ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_set_user_group_mapping_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_SP_USR  source_port;         /* input */
    DRV_BPM_USER_GROUP user_group_id  ;  /* input */
    DRV_BPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    source_port   = parm[parameter_index++].value.number;
    user_group_id = parm[parameter_index++].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_set_user_group_mapping'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_set_user_group_mapping ( source_port , user_group_id ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_get_user_group_mapping_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_SP_USR     source_port;   /* input */
    DRV_BPM_USER_GROUP user_group ;  /* output */
    DRV_BPM_ERROR error ;

    /* Get the parameters */
    source_port = parm[0].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_get_user_group_mapping'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_get_user_group_mapping ( source_port, &user_group ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "User Group mapping:\n\r" ) ;
        bdmf_session_print(session, " source port %s mapped to user group %lu\n", f_source_port_enum_to_string(source_port), user_group ) ;
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_request_buffer_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_SP_USR     source_port;   /* input */
    uint32_t                      buffer_ptr ;   /* output */
    DRV_BPM_ERROR error ;

    /* Get the parameters */
    source_port = parm[0].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_req_buffer'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_req_buffer ( source_port, &buffer_ptr ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, " BPM request buffer:\n\r" ) ;
        bdmf_session_print(session, " source port %s allocated buffer %lu\n", f_source_port_enum_to_string(source_port), buffer_ptr ) ;
    }
    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_free_buffer_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_SP_USR     source_port;   /* input */
    uint32_t                      buffer_ptr ;   /* input */
    DRV_BPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    source_port = parm[parameter_index++].value.number;
    buffer_ptr =  parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_free_buffer'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_free_buffer ( source_port, buffer_ptr ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    if ( error ==  DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, " BPM Free Buffer:\n\r" ) ;
        bdmf_session_print(session, " source port %s free buffer %lu\n", f_source_port_enum_to_string(source_port), buffer_ptr ) ;
    }
    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_update_multicast_counter_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint32_t multicast_counter;   /* input */
    uint32_t buffer_ptr ;         /* input */
    DRV_BPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    buffer_ptr =  parm[parameter_index++].value.unumber;
    multicast_counter =  parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_mcnt_update'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_mcnt_update ( buffer_ptr , multicast_counter ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_set_runner_message_control_parameters_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_RUNNER_MSG_CTRL_PARAMS runner_message_control_parameters;   /* input */
    DRV_BPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    runner_message_control_parameters.reply_wakeup_enable =  parm[parameter_index++].value.number;
    runner_message_control_parameters.transition_wakeup_enable =  parm[parameter_index++].value.number;
    runner_message_control_parameters.select_transition_msg =  parm[parameter_index++].value.number;
    runner_message_control_parameters.runner_a_reply_wakeup_task_number=  parm[parameter_index++].value.unumber;
    runner_message_control_parameters.runner_b_reply_wakeup_task_number =  parm[parameter_index++].value.unumber;
    runner_message_control_parameters.runner_transition_wakeup_task_number =  parm[parameter_index++].value.unumber;
    runner_message_control_parameters.runner_a_reply_target_address =  parm[parameter_index++].value.unumber;
    runner_message_control_parameters.runner_b_reply_target_address =  parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'pi_bl_drv_bpm_set_rnr_msg_ctl'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_set_runner_msg_ctrl ( &runner_message_control_parameters ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_get_runner_message_control_parameters_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{

    /* Define I/O variables */
    DRV_BPM_RUNNER_MSG_CTRL_PARAMS runner_message_control_parameters;   /* output */

    /* Define parameters handling variables */
    DRV_BPM_ERROR error ;
    char * str = "unknown";

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'pi_bl_drv_bpm_set_rnr_msg_ctl'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_get_runner_msg_ctrl ( &runner_message_control_parameters ) ;
    /* -------------------------------------------------------------------- */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    if ( error == DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "Runner Message Control Parameters:\n" ) ;
        bdmf_session_print(session, "Runner Reply WakeUp Enable: %s\n", str = runner_message_control_parameters.reply_wakeup_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, "Runner Transition WakeUp Enable: %s\n", str = runner_message_control_parameters.transition_wakeup_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, "Runner Select Transition Message : %s\n", str = runner_message_control_parameters.select_transition_msg ? "Runner A" : "Runner B" ) ;
        bdmf_session_print(session, "Runner A Reply Wakeup Task Number : %lu\n", runner_message_control_parameters.runner_a_reply_wakeup_task_number  ) ;
        bdmf_session_print(session, "Runner B Reply Wakeup Task Number : %lu\n", runner_message_control_parameters.runner_b_reply_wakeup_task_number  ) ;
        bdmf_session_print(session, "Runner Transition Wakeup Task Number : %lu\n", runner_message_control_parameters.runner_transition_wakeup_task_number  ) ;
        bdmf_session_print(session, "Runner A Reply Target Address : %lu\n", runner_message_control_parameters.runner_a_reply_target_address  ) ;
        bdmf_session_print(session, "Runner B Reply Target Address : %lu\n", runner_message_control_parameters.runner_b_reply_target_address  ) ;
    }
    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_set_mips_d_message_control_parameters_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_MIPS_D_MSG_CTRL_PARAMS mips_d_message_control_parameters;   /* input */
    DRV_BPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    mips_d_message_control_parameters.mips_d_reply_wakeup_enable =  parm[parameter_index++].value.number;
    mips_d_message_control_parameters.mips_d_reply_wakeup_task_number =  parm[parameter_index++].value.unumber;
    mips_d_message_control_parameters.mips_d_reply_target_address =  parm[parameter_index++].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_set_mips_d_msg_ctl'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_set_mips_d_msg_ctrl ( &mips_d_message_control_parameters ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_get_mips_d_message_control_parameters_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{

    /* Define I/O variables */
    DRV_BPM_MIPS_D_MSG_CTRL_PARAMS mips_d_message_control_parameters;   /* output */

    /* Define parameters handling variables */
    DRV_BPM_ERROR error ;
    char * str = "unknown";

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'pi_bl_drv_bpm_set_rnr_msg_ctl'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_get_mips_d_msg_ctrl ( &mips_d_message_control_parameters ) ;
    /* -------------------------------------------------------------------- */
    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    if ( error == DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "MIPS-D Message Control Parameters:\n" ) ;
        bdmf_session_print(session, "MIPS-D Reply WakeUp Enable: %s\n", str = mips_d_message_control_parameters.mips_d_reply_wakeup_enable ? "Enabled" : "Disabled" ) ;
        bdmf_session_print(session, "MIPS-D Reply Wakeup Task Number : %lu\n", mips_d_message_control_parameters.mips_d_reply_wakeup_task_number  ) ;
        bdmf_session_print(session, "MIPS-D Reply Target Address : %lu\n", mips_d_message_control_parameters.mips_d_reply_target_address  ) ;
    }
    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_get_interrupt_status_register_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_ISR bpm_isr;   /* input */
    DRV_BPM_ERROR error ;
    char * str = "unknown";

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'pi_bl_drv_bpm_get_interrupt_status_register'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_get_interrupt_status_register ( &bpm_isr ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    if (error == DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "BPM Interrupt status register:\n" ) ;
        bdmf_session_print(session, "free interupt status: %s\n" , str = ( bpm_isr.free_interrupt? "On" : "Off" ) ) ;
        bdmf_session_print(session, "multicast counter interrupt status: %s\n" , str = ( bpm_isr.multicast_counter_interrupt? "On" : "Off" ) ) ;
    }
    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_set_interrupt_enable_register_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_ISR bpm_ier;   /* input */
    DRV_BPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    bpm_ier.free_interrupt =  parm[parameter_index++].value.number;
    bpm_ier.multicast_counter_interrupt =  parm[parameter_index++].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'pi_bl_drv_bpm_set_interrupt_enable_register'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_set_interrupt_enable_register ( &bpm_ier ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_get_interrupt_enable_register_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_ISR bpm_ier;   /* input */
    DRV_BPM_ERROR error;
    char * str = "unknown";

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'pi_bl_drv_bpm_get_interrupt_enable_register'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_get_interrupt_enable_register ( &bpm_ier ) ;
    /* -------------------------------------------------------------------- */

    if ( error == DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "BPM Interrupt enable register:\n" ) ;
        bdmf_session_print(session, "free interupt status: %s\n" , str = ( bpm_ier.free_interrupt? "Enable" : "Disable" ) ) ;
        bdmf_session_print(session, "multicast counter interrupt status: %s\n" , str = ( bpm_ier.multicast_counter_interrupt? "Enable" : "Disable" ) ) ;
    }
    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_generate_interrupt_test_register_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    DRV_BPM_ISR bpm_itr;   /* input */
    DRV_BPM_ERROR error ;
    uint8_t parameter_index ;

    /* Get the parameters */
    parameter_index = 0 ;
    bpm_itr.free_interrupt =  parm[parameter_index++].value.number;
    bpm_itr.multicast_counter_interrupt =  parm[parameter_index++].value.number;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'pi_bl_drv_bpm_generate_interrupt_test_register'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_generate_interrupt_test_register ( &bpm_itr ) ;
    /* -------------------------------------------------------------------- */

    bdmf_session_print(session, "UT: Function returns '%s'\n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_get_buffer_number_status_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    /* Define I/O variables */
    uint16_t buffer_number;   /* input */
    uint8_t  bn_status ;      /* output */
    DRV_BPM_ERROR error ;

    /* Get the parameters */
    buffer_number = parm[0].value.unumber;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'fi_bl_drv_bpm_get_buffer_number_status'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = fi_bl_drv_bpm_get_buffer_number_status ( buffer_number , &bn_status ) ;
    /* -------------------------------------------------------------------- */

    if ( error == DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "Buffer Number %lu status:\n", buffer_number ) ;
        if ( bn_status == 0 )
        {
            bdmf_session_print(session, "Non-Occupied Buffer Number\n"   ) ;
        }
        else
        {
            bdmf_session_print(session, "BN is occupied for %lu ports\n" , bn_status ) ;
        }
    }

    return error ? BDMF_ERR_PARM : 0;
}

static int p_bl_drv_bpm_dump_all_configuration_command(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{

    DRV_BPM_ERROR error ;

    /* Call the under test routine */
    bdmf_session_print(session, "UT: Invoking function 'f_dump_bpm_database'\n\r" ) ;
    /* -------------------------------------------------------------------- */
    error = f_dump_bpm_database(session);
    /* -------------------------------------------------------------------- */

    /* Translate and print the error code */
    bdmf_session_print(session, "UT: Function returns '%s' \n\r", f_drv_bpm_error_code_to_string ( error ) ) ;

    return error ? BDMF_ERR_PARM : 0;
}


static DRV_BPM_ERROR f_dump_bpm_database(bdmf_session_handle session)
{

    DRV_BPM_GLOBAL_THRESHOLD global_threshold;
    DRV_BPM_SP_USR  source_port;
    DRV_BPM_USER_GROUP user_group_index;
    DRV_BPM_USER_GROUP_CONFIGURATION user_group_configuration  ;
    DRV_BPM_USER_GROUP_STATUS user_group_status;
    DRV_BPM_ISR bpm_isr;
    DRV_BPM_ERROR error;
    char * str = "unknown";
    uint32_t global_hysteresis ;
    uint16_t user_group_counter;


    bdmf_session_print(session, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r" ) ;
    bdmf_session_print(session, "\n\r                      ++++ BPM Configuration ++++                         \n\n\r" ) ;
    bdmf_session_print(session, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\n\n\r" ) ;



    bdmf_session_print(session, "\n\r                           Global Configuration                             \n\r" ) ;
    bdmf_session_print(session, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r" ) ;

    error = fi_bl_drv_bpm_get_global_threshold ( &global_threshold, &global_hysteresis );

    if ( error ==  DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "Global Threshold Configuration:\n\r" ) ;
        bdmf_session_print(session, "Global Threshold :%s\n\r", f_global_threshold_enum_to_string(global_threshold) ) ;
        bdmf_session_print(session, "Global Threshold hysteresis: %d\n\r" ,global_hysteresis ) ;
    }

    bdmf_session_print(session, "\n\r                          User Group Configuration                             \n\r" ) ;
    bdmf_session_print(session, "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r" ) ;

    for ( source_port = DRV_BPM_SP_GPON ; source_port <= DRV_BPM_SP_RNR_B ; source_port++ )
    {
            error = fi_bl_drv_bpm_get_user_group_mapping ( source_port, &user_group_index ) ;

            if ( error ==  DRV_BPM_ERROR_NO_ERROR )
            {
                bdmf_session_print(session, " source port %s mapped to user group %lu\n", f_source_port_enum_to_string(source_port), user_group_index ) ;
            }
            else
            {
                return ( error );
            }
    }

   bdmf_session_print(session, "\n\r" ) ;
   bdmf_session_print(session, "\n\r" ) ;

    for ( user_group_index = 0 ; user_group_index < DRV_BPM_NUMBER_OF_USER_GROUPS ; user_group_index++ )
    {
            bdmf_session_print(session, "User Group %d Configuration:\n\r", user_group_index ) ;
            bdmf_session_print(session, "---------------------------------\n\n\r" ) ;

            error = fi_bl_drv_bpm_get_user_group_thresholds ( user_group_index, &user_group_configuration ) ;

            if ( error ==  DRV_BPM_ERROR_NO_ERROR )
            {
                bdmf_session_print(session, " Threshold: %d\n\r", user_group_configuration.threshold ) ;
                bdmf_session_print(session, " hysteresis: %d\n\r" ,user_group_configuration.hysteresis ) ;
                bdmf_session_print(session, " Exclusive Threshold :%d\n\r", user_group_configuration.exclusive_threshold ) ;
                bdmf_session_print(session, " Exclusive hysteresis: %d\n\r" ,user_group_configuration.exclusive_hysteresis ) ;
            }
            else
            {
                return ( error );
            }

            bdmf_session_print(session, "\n\r" ) ;

            error = fi_bl_drv_bpm_get_user_group_status ( user_group_index, &user_group_status ) ;

            if ( error ==  DRV_BPM_ERROR_NO_ERROR )
            {
                bdmf_session_print(session, " Ack/Nack status: %s\n\r", f_user_group_status_enum_to_string(user_group_status.ug_status) ) ;
                bdmf_session_print(session, " Exclusive/non-Exclusive status: %s\n\r" ,f_user_group_exclusive_status_enum_to_string(user_group_status.ug_exclusive_status) );
            }
            else
            {
                return ( error );
            }

            bdmf_session_print(session, "\n\r" ) ;

            error = fi_bl_drv_bpm_get_user_group_counter ( user_group_index, &user_group_counter ) ;

            if ( error ==  DRV_BPM_ERROR_NO_ERROR )
            {
                bdmf_session_print(session, "User Group %d counter = %lu \n\r", user_group_index ,user_group_counter ) ;
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


    error = fi_bl_drv_bpm_get_interrupt_enable_register ( &bpm_isr ) ;

    if ( error == DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "BPM Interrupt enable register:\n" ) ;
        bdmf_session_print(session, "free interupt: %s\n" , str = ( bpm_isr.free_interrupt? "Enable" : "Disable" ) ) ;
        bdmf_session_print(session, "multicast counter interrupt: %s\n" , str = ( bpm_isr.multicast_counter_interrupt? "Enable" : "Disable" ) ) ;
    }
    else
    {
        return ( error );
    }

    error = fi_bl_drv_bpm_get_interrupt_status_register ( &bpm_isr ) ;
    if ( error == DRV_BPM_ERROR_NO_ERROR )
    {
        bdmf_session_print(session, "BPM Interrupt status register:\n" ) ;
        bdmf_session_print(session, "free interupt status: %s\n" , str = ( bpm_isr.free_interrupt? "On" : "Off" ) ) ;
        bdmf_session_print(session, "multicast counter interrupt status: %s\n" , str = ( bpm_isr.multicast_counter_interrupt? "On" : "Off" ) ) ;
    }
    else
    {
        return ( error );
    }

    return ( DRV_BPM_ERROR_NO_ERROR ) ;

}


static bdmfmon_handle_t bpm_dir;

void pi_bl_initialize_drv_bpm_shell(bdmfmon_handle_t driver_dir)
{
    static struct bdmfmon_enum_val global_threshold_enum_table[] = {
        {"2.5KB", 0},
        {"5KB", 1},
        {"7.5KB", 2},
        {"10KB", 3},
        {"12.5KB", 4},
        {"15KB", 5},
        {NULL, 0},
    };
    static struct bdmfmon_enum_val src_port_short_enum_table[] = {
        {"gpon",  DRV_BPM_SP_GPON},
        {"emac0", DRV_BPM_SP_EMAC0},
        {"emac1", DRV_BPM_SP_EMAC1},
        {"emac2", DRV_BPM_SP_EMAC2},
        {"emac3", DRV_BPM_SP_EMAC3},
        {"emac4", DRV_BPM_SP_EMAC4},
        {"rnr_a", DRV_BPM_SP_RNR_A},
        {"rnr_b", DRV_BPM_SP_RNR_B},
        {NULL, 0},
    };
    static struct bdmfmon_enum_val src_port_enum_table[] = {
        {"gpon",  DRV_BPM_SP_GPON},
        {"emac0", DRV_BPM_SP_EMAC0},
        {"emac1", DRV_BPM_SP_EMAC1},
        {"emac2", DRV_BPM_SP_EMAC2},
        {"emac3", DRV_BPM_SP_EMAC3},
        {"emac4", DRV_BPM_SP_EMAC4},
        {"cpu",   DRV_BPM_SP_MIPS_C},
        {"rnr_a", DRV_BPM_SP_RNR_A},
        {"rnr_b", DRV_BPM_SP_RNR_B},
        {"mips_d",DRV_BPM_SP_MIPS_D},
        {"pci_0", DRV_BPM_SP_PCI0},
        {"pci_1", DRV_BPM_SP_PCI1},
        {"usb_0", DRV_BPM_SP_USB0},
        {"usb_1", DRV_BPM_SP_USB1},
        {NULL, 0},
    };
    static struct bdmfmon_enum_val runner_enum_table[] = {
        {"A", 0},
        {"B", 1},
        { NULL, 0}
    };

    bpm_dir = bdmfmon_dir_add(driver_dir, "bpmd", "BPM Driver", BDMF_ACCESS_ADMIN, NULL );
    if (!bpm_dir)
    {
        bdmf_session_print(NULL, "Can't create bpmd directory\n");
        return;
    }

    BDMFMON_MAKE_CMD(bpm_dir, "bpmi", "BPM Initialize", p_bl_drv_bpm_initialize_command,
        BDMFMON_MAKE_PARM_ENUM("global_threshold", "global threshold", global_threshold_enum_table, 0),
        BDMFMON_MAKE_PARM("global_hysteresis", "global_hysteresis", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_RANGE("ug", "user group id", BDMFMON_PARM_NUMBER, 0, 0, 1));

    BDMFMON_MAKE_CMD(bpm_dir, "esp", "Enable source port", p_bl_drv_bpm_sp_enable_command,
        BDMFMON_MAKE_PARM_ENUM("port", "source_port", src_port_short_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("control", "control", bdmfmon_enum_bool_table, 0));

    BDMFMON_MAKE_CMD(bpm_dir, "sgt", "Set Global Threshold", p_bl_drv_bpm_set_global_threshold_command,
        BDMFMON_MAKE_PARM_ENUM("global_threshold", "global threshold", global_threshold_enum_table, 0),
        BDMFMON_MAKE_PARM("global_hysteresis", "global_hysteresis", BDMFMON_PARM_NUMBER, 0));

    BDMFMON_MAKE_CMD_NOPARM(bpm_dir, "ggt", "Get Global Threshold", p_bl_drv_bpm_get_global_threshold_command);

    BDMFMON_MAKE_CMD(bpm_dir, "sugt", "Set User Group Threshold", p_bl_drv_bpm_set_user_group_thresholds_command,
        BDMFMON_MAKE_PARM_RANGE("ug", "user group id", BDMFMON_PARM_NUMBER, 0, 0, 7),
        BDMFMON_MAKE_PARM("threshold", "user group threshold", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("hysteresis", "user group hysteresis", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("excl_threshold", "user group exclusive threshold", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("excl_hysteresis", "user group exclusive hysteresis", BDMFMON_PARM_NUMBER, 0));

    BDMFMON_MAKE_CMD(bpm_dir, "gugt", "Get User Group Threshold", p_bl_drv_bpm_get_user_group_thresholds_command,
        BDMFMON_MAKE_PARM_RANGE("ug", "user group id", BDMFMON_PARM_NUMBER, 0, 0, 7));

    BDMFMON_MAKE_CMD(bpm_dir, "gugs", "Get User Group Status", p_bl_drv_bpm_get_user_group_status_command,
        BDMFMON_MAKE_PARM_RANGE("ug", "user group id", BDMFMON_PARM_NUMBER, 0, 0, 7));

    BDMFMON_MAKE_CMD(bpm_dir, "gugc", "Get User Group Counter", p_bl_drv_bpm_get_user_group_counter_command,
        BDMFMON_MAKE_PARM_RANGE("ug", "user group id", BDMFMON_PARM_NUMBER, 0, 0, 7));

    BDMFMON_MAKE_CMD(bpm_dir, "sugm", "Set User Group Mapping", p_bl_drv_bpm_set_user_group_mapping_command,
        BDMFMON_MAKE_PARM_ENUM("port", "source_port", src_port_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("ug", "user group id", BDMFMON_PARM_NUMBER, 0, 0, 7));

    BDMFMON_MAKE_CMD(bpm_dir, "gugm", "Get User Group Mapping", p_bl_drv_bpm_get_user_group_mapping_command,
        BDMFMON_MAKE_PARM_ENUM("port", "source_port", src_port_enum_table, 0));

    BDMFMON_MAKE_CMD(bpm_dir, "rbuf", "Request Buffer", p_bl_drv_bpm_request_buffer_command,
        BDMFMON_MAKE_PARM_ENUM("port", "source_port", src_port_enum_table, 0));

    BDMFMON_MAKE_CMD(bpm_dir, "fbuf", "Free Buffer", p_bl_drv_bpm_free_buffer_command,
        BDMFMON_MAKE_PARM_ENUM("port", "source_port", src_port_enum_table, 0),
        BDMFMON_MAKE_PARM("ptr", "DDR buffer pointer value", BDMFMON_PARM_HEX, 0));

    BDMFMON_MAKE_CMD(bpm_dir, "umc",  "Update Multicast Counter", p_bl_drv_bpm_update_multicast_counter_command,
        BDMFMON_MAKE_PARM("ptr", "DDR buffer pointer value", BDMFMON_PARM_HEX, 0),
        BDMFMON_MAKE_PARM_RANGE("mc", "mulicast counter", BDMFMON_PARM_NUMBER, 0, 0, 7));

    BDMFMON_MAKE_CMD(bpm_dir, "srmcp", "Set Runner Control Parameters", p_bl_drv_bpm_set_runner_message_control_parameters_command,
        BDMFMON_MAKE_PARM_ENUM("rply_wkup_enable", "Enable wake-up message after each reply on Alloc request from Runner", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("trans_wkup_enable", "Enable wake-up message after each transition state of any peripheral", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("runner", "select the runner for sending transition message", runner_enum_table, 0),
        BDMFMON_MAKE_PARM_RANGE("rnr_a_task",  "Task number for Runner A Wake-Up as result of Reply on Alloc request", BDMFMON_PARM_NUMBER, 0, 0, 31),
        BDMFMON_MAKE_PARM_RANGE("rnr_b_task",  "Task number for Runner B Wake-Up as result of Reply on Alloc request", BDMFMON_PARM_NUMBER, 0, 0, 31),
        BDMFMON_MAKE_PARM_RANGE("trans_task",  "Task number for any selected Runner Wake-Up as result of Transition", BDMFMON_PARM_NUMBER, 0, 0, 31),
        BDMFMON_MAKE_PARM("rnr_a_ta",  "Target address for runner A", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("rnr_b_ta",  "Target address for runner B", BDMFMON_PARM_NUMBER, 0));

    BDMFMON_MAKE_CMD_NOPARM(bpm_dir, "grmcp", "Get Runner Control Parameters", p_bl_drv_bpm_get_runner_message_control_parameters_command) ;

    BDMFMON_MAKE_CMD(bpm_dir, "smdmcp", "Set MIPS-D Control Parameters", p_bl_drv_bpm_set_mips_d_message_control_parameters_command,
        BDMFMON_MAKE_PARM_ENUM("wkup_enable", "Enable wake-up message after each reply on Alloc request from Mips-D", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_RANGE("wkup_task",  "Task Number used on Wake-Up message after Reply response (if enabled) for MIPS-D", BDMFMON_PARM_NUMBER, 0, 0, 31),
        BDMFMON_MAKE_PARM("reply_ta",  "BPM will reply on Alloc request to MIPSD corresponding this Target address", BDMFMON_PARM_NUMBER, 0));

    BDMFMON_MAKE_CMD_NOPARM(bpm_dir, "gmdmcp", "Get MIPS-D Control Parameters", p_bl_drv_bpm_get_mips_d_message_control_parameters_command);

    BDMFMON_MAKE_CMD_NOPARM(bpm_dir, "gisr", "Get Interrupt Status Register", p_bl_drv_bpm_get_interrupt_status_register_command);

    BDMFMON_MAKE_CMD(bpm_dir, "sier", "Set Interrupt Enable Register", p_bl_drv_bpm_set_interrupt_enable_register_command,
        BDMFMON_MAKE_PARM_ENUM("free", "free_interupt", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("mcast", "multicast_counter_interrupt", bdmfmon_enum_bool_table, 0));


    BDMFMON_MAKE_CMD_NOPARM(bpm_dir, "gier", "Get Interrupt Enable Register", p_bl_drv_bpm_get_interrupt_enable_register_command);

    BDMFMON_MAKE_CMD(bpm_dir, "gitr", "Generate Interrupt Test Registe", p_bl_drv_bpm_generate_interrupt_test_register_command,
        BDMFMON_MAKE_PARM_ENUM("free", "free_interupt", bdmfmon_enum_bool_table, 0),
        BDMFMON_MAKE_PARM_ENUM("mcast", "multicast_counter_interrupt", bdmfmon_enum_bool_table, 0));

    BDMFMON_MAKE_CMD(bpm_dir, "gbns", "Get Buffer Number Status", p_bl_drv_bpm_get_buffer_number_status_command,
        BDMFMON_MAKE_PARM("bn",  "Buffer number", BDMFMON_PARM_NUMBER, 0));

    /* more advanced commands: */
    BDMFMON_MAKE_CMD_NOPARM(bpm_dir, "dac", "Dump BPM Configuration", p_bl_drv_bpm_dump_all_configuration_command);
}

void pi_bl_exit_drv_bpm_shell ( void )
{
    if (bpm_dir)
    {
        bdmfmon_token_destroy(bpm_dir);
        bpm_dir = NULL;
    }
}


char * f_drv_bpm_error_code_to_string ( DRV_BPM_ERROR xi_error_code )
{
    switch ( xi_error_code )
    {
    case DRV_BPM_ERROR_NO_ERROR:
        return "DRV_BPM_ERROR_NO_ERROR" ;
        break ;
    case DRV_BPM_ERROR_BPM_BUSY:
        return "DRV_BPM_ERROR_BPM_BUSY" ;
        break ;
    case DRV_BPM_ERROR_NO_FREE_BUFFER:
        return "DRV_BPM_ERROR_NO_FREE_BUFFER" ;
        break ;
    case DRV_BPM_ERROR_INVALID_SOURCE_PORT:
        return "DRV_BPM_ERROR_INVALID_SOURCE_PORT" ;
        break ;
    case DRV_BPM_ERROR_INVALID_USER_GROUP:
        return "DRV_BPM_ERROR_INVALID_USER_GROUP" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}

char * f_source_port_enum_to_string ( DRV_BPM_SP_USR xi_sp )
{
    switch ( xi_sp )
    {
    case DRV_BPM_SP_GPON:
        return "GPON" ;
        break ;
    case DRV_BPM_SP_EMAC0:
        return "Emac 0" ;
        break ;
    case DRV_BPM_SP_EMAC1:
        return "Emac 1" ;
        break ;
    case DRV_BPM_SP_EMAC2:
        return "Emac 2" ;
        break ;
    case DRV_BPM_SP_EMAC3:
        return "Emac 3" ;
        break ;
    case DRV_BPM_SP_EMAC4:
        return "Emac 4" ;
        break ;
    case DRV_BPM_SP_MIPS_C:
        return "CPU" ;
        break ;
    case DRV_BPM_SP_MIPS_D:
        return "MIPS D" ;
        break ;
    case DRV_BPM_SP_PCI0:
        return "PCI 0" ;
        break ;
    case DRV_BPM_SP_PCI1:
        return "PCI 1" ;
        break ;
    case DRV_BPM_SP_USB0:
        return "USB 0" ;
        break ;
    case DRV_BPM_SP_USB1:
        return "USB 1" ;
        break ;
    case DRV_BPM_SP_RNR_A:
        return "Runner A" ;
        break ;
    case DRV_BPM_SP_RNR_B:
        return "Runner B" ;
        break ;
    case DRV_BPM_SP_SPARE_0:
        return "Spare 0" ;
        break;
    case DRV_BPM_SP_SPARE_1:
        return "Spare 1" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}

static char * f_user_group_status_enum_to_string ( E_DRV_BPM_ENABLE xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_BPM_DISABLE:
        return " NACK" ;
        break ;
    case DRV_BPM_ENABLE:
        return " ACK" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}

static char * f_user_group_exclusive_status_enum_to_string ( E_DRV_BPM_ENABLE xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_BPM_DISABLE:
        return " Non-Exclusive" ;
        break ;
    case DRV_BPM_ENABLE:
        return " Exclusive" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}


static char * f_global_threshold_enum_to_string ( DRV_BPM_GLOBAL_THRESHOLD xi_enum_value )
{
    switch ( xi_enum_value )
    {
    case DRV_BPM_GLOBAL_THRESHOLD_2_5K:
        return "2.5K buffers" ;
        break ;
    case DRV_BPM_GLOBAL_THRESHOLD_5K:
        return "5K buffers" ;
        break ;
    case DRV_BPM_GLOBAL_THRESHOLD_7_5K:
        return "7.5K buffers" ;
        break ;
    case DRV_BPM_GLOBAL_THRESHOLD_10K:
        return "10K buffers" ;
        break ;
    case DRV_BPM_GLOBAL_THRESHOLD_12_5K:
        return "12.5 buffers" ;
        break ;
    case DRV_BPM_GLOBAL_THRESHOLD_15K:
        return "15K buffers" ;
        break ;
    default:
        return "unknown" ;
        break ;
    }
}



#endif



