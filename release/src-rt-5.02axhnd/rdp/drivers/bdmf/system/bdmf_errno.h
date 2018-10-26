/*
* <:copyright-BRCM:2013-2015:GPL/GPL:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


/*******************************************************************
 * bdmf_errno.h
 *
 * BDMF framework - generic error codes
 *
 *******************************************************************/

#ifndef BDMF_ERRNO_H

#define BDMF_ERRNO_H

/** \defgroup bdmf_errno Broadlight Error Codes
 *
 * This header files includes declaration of generic error codes and also
 * support functions for adding additional error code ranges and
 * conversion of error code to error message.
 * @{
 */

/** Generic error codes
 */
typedef enum { 
    BDMF_ERR_OK               =  0,   /**< OK */
    BDMF_ERR_PARM             = -1,   /**< Error in parameters */
    BDMF_ERR_NOMEM            = -2,   /**< No memory */
    BDMF_ERR_NORES            = -3,   /**< No resources */
    BDMF_ERR_INTERNAL         = -4,   /**< Internal error */
    BDMF_ERR_NOENT            = -5,   /**< Entry doesn't exist */
    BDMF_ERR_NODEV            = -6,   /**< Device doesn't exist */
    BDMF_ERR_ALREADY          = -7,   /**< Entry already exists */
    BDMF_ERR_RANGE            = -8,   /**< Out of range */
    BDMF_ERR_PERM             = -9,   /**< No permission to perform an operation */
    BDMF_ERR_NOT_SUPPORTED    = -10,  /**< Operation is not supported */
    BDMF_ERR_PARSE            = -11,  /**< Parsing error */
    BDMF_ERR_INVALID_OP       = -12,  /**< Invalid operation */
    BDMF_ERR_IO               = -13,  /**< I/O error */
    BDMF_ERR_STATE            = -14,  /**< Object is in bad state */
    BDMF_ERR_DELETED          = -15,  /**< Object is deleted */
    BDMF_ERR_TOO_MANY         = -16,  /**< Too many objects */
    BDMF_ERR_NOT_LINKED       = -17,  /**< Objects are not linked */
    BDMF_ERR_NO_MORE          = -18,  /**< No more entries */
    BDMF_ERR_OVERFLOW         = -19,  /**< Buffer overflow */
    BDMF_ERR_COMM_FAIL        = -20,  /**< Communication failure */
    BDMF_ERR_NOT_CONNECTED    = -21,  /**< No connection with the target system */
    BDMF_ERR_SYSCALL_ERR      = -22,  /**< System call returned error */
    BDMF_ERR_MSG_ERROR        = -23,  /**< Received message is insane */
    BDMF_ERR_TOO_MANY_REQS    = -24,  /**< Too many outstanding requests */
    BDMF_ERR_NO_MSG_SERVER    = -25,  /**< Remote delivery error. No message server. */
    BDMF_ERR_NO_LOCAL_SUBS    = -26,  /**< Local subsystem is not set */
    BDMF_ERR_NO_SUBS          = -27,  /**< Subsystem is not recognised */
    BDMF_ERR_INTR             = -28,  /**< Operation interrupted */
    BDMF_ERR_HIST_RES_MISMATCH= -29,  /**< History result mismatch */
    BDMF_ERR_MORE             = -30,  /**< More work to do */
    BDMF_ERR_IGNORE           = -31,  /**< Ignore the error */
    BDMF_ERR_LAST         = -100,    /**< Last generic error */
} bdmf_error_t;

/** Register error code range
 *
 * \param[in]   from        From number. Must be negative
 * \param[in]   to          To number. Must be negative >from
 * \param[in]   p_strerr    Callback that returns error string
 *
 * \returns  BDMF_ERR_OK      - OK\n
 *           BDMF_ERR_PARM    - error in parameters\n
 *           BDMF_ERR_NOMEM   - no memory
 *           BDMF_ERR_ALREADY - ( from, to ) range overlaps with existing range
 */
bdmf_error_t bdmf_error_range_register(int from, int to,
                                   const char *(*p_strerr)(int err));


/** Unregister error code range
 *
 * \param[in]   from        From number. Must be negative
 * \param[in]   to          To number. Must be negative >from
 *
 * \returns  BDMF_ERR_OK      - OK\n
 *           BDMF_ERR_PARM    - error in parameters\n
 *           BDMF_ERR_NOENT   - ( from, to ) range is not registered
 */
bdmf_error_t bdmf_error_range_unregister(int from, int to);


/** Convert error code to error string
 *
 * \param[in]   err         Error code. One of bdmf_error_t constants or additional
 *                          codes registered using bdmf_error_register()
 * \returns Error string
 */
const char *bdmf_strerror(int err);

/** @} */

#endif /* #ifndef BDMF_ERRNO_H */

