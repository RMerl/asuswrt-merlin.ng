// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/**********************************************************************************
 **********************************************************************************
 *                                                                                *
 *  Revision    :   *
 *                                                                                *
 *  Description :  Internal API error functions                                   *
 *                                                                                *
 **********************************************************************************
 **********************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MERLIN8_PHY1_API_INTERNAL_ERROR_H
#define MERLIN8_PHY1_API_INTERNAL_ERROR_H

#include "merlin8_phy1_usr_includes.h"
#include "common/srds_api_err_code.h"
#include "common/srds_api_types.h"

/*! @file
 *  @brief Internal API error functions
 */

/*! @addtogroup APITag
 * @{
 */

/*! @defgroup SerdesAPIInternalErrorTag Internal Error Functions
 * Internal Serdes API functions and maros for error handling which get called by other modules and
 * should not be called directly.
 */

/*! @addtogroup SerdesAPIInternalErrorTag
 * @{
 */


/**
 * Error-trapping macro.
 *
 * In other then SerDes-team post-silicon evaluation builds, simply yields
 * the error code supplied as an argument, without further action.
 */
#ifndef SMALL_FOOTPRINT
#define merlin8_phy1_error_report(sa__, err_code) \
        merlin8_phy1_INTERNAL_print_err_msg_and_triage_info(sa__, (err_code), __FILE__, API_FUNCTION_NAME, (uint16_t)__LINE__)
#else
#define merlin8_phy1_error_report(sa__, err_code) (err_code)
#endif
/* Prints error code, containing function, file and line number */
#ifndef SMALL_FOOTPRINT
#define merlin8_phy1_error(sa__, err_code) \
        merlin8_phy1_INTERNAL_print_err_msg(sa__, (err_code), __FILE__, API_FUNCTION_NAME, (uint16_t)__LINE__)
#else
#define merlin8_phy1_error(sa__, err_code) (err_code)
#endif
#ifndef SMALL_FOOTPRINT
/** Print Error messages to screen before returning.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] err_code Error Code input which is returned as well.
 * @param[in] filename filename containing the function from which error is reported.
 * @param[in] func_name function in which error is reported.
 * @param[in] line Line number of where the error occurred.
 * @return Error Code.
 */
err_code_t merlin8_phy1_INTERNAL_print_err_msg(srds_access_t *sa__, uint16_t err_code, const char *filename, const char *func_name, uint16_t line);

/** Print Error messages to screen and collects and prints Triage info before returning.
 * @param[in] sa__ is an opaque state vector passed through to device access functions.
 * @param[in] err_code Error Code input which is returned as well.
 * @param[in] filename filename containing the function from which error is reported.
 * @param[in] func_name function in which error is reported.
 * @param[in] line Line number of where the error occurred.
 * @return Error Code.
 */
err_code_t merlin8_phy1_INTERNAL_print_err_msg_and_triage_info(srds_access_t *sa__, uint16_t err_code, const char *filename, const char *func_name, uint16_t line);

/** Print Convert Error code to String.
 * @param[in] err_code Error Code input which is converted to string.
 * @return String containing Error code information.
 */
const char* merlin8_phy1_INTERNAL_e2s_err_code(err_code_t err_code);
#endif /* !SMALL_FOOTPRINT */

/*! @} SerdesAPIInternalErrorTag */
/*! @} APITag */
#endif
#ifdef __cplusplus
}
#endif
