// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

*/
#ifndef MERLIN16_SHORTFIN_API_INTERNAL_ERROR_H
#define MERLIN16_SHORTFIN_API_INTERNAL_ERROR_H
#include "merlin16_shortfin_usr_includes.h"
#include "common/srds_api_types.h"
#include "common/srds_api_err_code.h"


/**
 * Error-trapping macro.
 *
 * In other then SerDes-team post-silicon evaluation builds, simply yields
 * the error code supplied as an argument, without further action.
 */
#define merlin16_shortfin_error_report(sa__, err_code) \
        merlin16_shortfin_INTERNAL_print_err_msg_and_triage_info(sa__, (err_code), __FILE__, API_FUNCTION_NAME, __LINE__)
/**@}*/

/* Prints error code, containing function, file and line number */
#define merlin16_shortfin_error(sa__, err_code) \
        merlin16_shortfin_INTERNAL_print_err_msg(sa__, (err_code), __FILE__, API_FUNCTION_NAME, __LINE__)

/** Print Error messages to screen before returning.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param err_code Error Code input which is returned as well
 * @param filename filename containing the function from which error is reported.
 * @param func_name function in which error is reported.
 * @param line Line number.
 * @return Error Code
 */
err_code_t merlin16_shortfin_INTERNAL_print_err_msg(srds_access_t *sa__, uint16_t err_code, const char *filename, const char *func_name, uint16_t line);

/** Print Error messages to screen and collects and prints Triage info before returning.
 * @param sa__ is an opaque state vector passed through to device access functions.
 * @param err_code Error Code input which is returned as well
 * @param filename filename containing the function from which error is reported.
 * @param func_name function in which error is reported.
 * @param line Line number.
 * @return Error Code
 */
err_code_t merlin16_shortfin_INTERNAL_print_err_msg_and_triage_info(srds_access_t *sa__, uint16_t err_code, const char *filename, const char *func_name, uint16_t line);
/** Print Convert Error code to String.
 * @param err_code Error Code input which is converted to string
 * @return String containing Error code information.
 */
const char* merlin16_shortfin_INTERNAL_e2s_err_code(err_code_t err_code);

#endif
