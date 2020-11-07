/*
  <:copyright-BRCM:2015:proprietary:standard
  
     Copyright (c) 2015 Broadcom 
     All Rights Reserved
  
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
*/


#ifndef EAGLE_ONU10G_API_FUNCTIONS_H
#define EAGLE_ONU10G_API_FUNCTIONS_H

/* include all .h files, even though some are redundant */

#include "eagle_onu10g_usr_includes.h"

#include "eagle_onu10g_ipconfig.h"
#include "eagle_onu10g_dependencies.h"
#include "eagle_onu10g_interface.h"
#include "eagle_onu10g_debug_functions.h"
#include "eagle_onu10g_common.h"
#include "eagle_merlin_api_uc_common.h"
#include "common/srds_api_uc_common.h"

#include "eagle_onu10g_field_access.h"
#include "eagle_onu10g_enum.h"
#include "common/srds_api_err_code.h"
#include "eagle_onu10g_internal.h"




/**
 * Error-trapping macro.
 *
 * In other then SerDes-team post-silicon evaluation builds, simply yields
 * the error code supplied as an argument, without further action.
 */
#define _error(err_code) _print_err_msg(err_code)

/**@}*/



/************************************************************************//**
 * @name Error-Code Storage Addresses.
 *
 * Used by error-checking expression-wrapper macros.  Expands to the address
 * where the macros are meant to store the error codes on which they operate,
 * which depends on the target core and type of executable image being built.
 *
 * These are defined well ahead of the error-checking macros themselves to
 * facilitate definition of RAM- and register-access macros generally used
 * in their argyments.
 *//*************************************************************************/
/**@{*/


/**
 * Error-code storage address.
 *
 * This is a standard API build that directs error-checking expression-wrapper
 * macros to use a block-local error codes for efficient. local optimization.
 */
#define __ERR &__err

/**@}*/



/************************************************************************//**
 * @name Register Access Macro Inclusions
 *
 * All cores provide access to hardware control/status registers.
 *//*************************************************************************/
/**@{*/


/**
 * This build includes register accessor macros for the EAGLE/ONU10G core.
 */
#include "eagle_onu10g_fields.h"





/**@}*/



/************************************************************************//**
 * @name RAM Access Macro Inclusions
 *
 * Some cores also provide access to firmware control/status RAM variables.
 *//*************************************************************************/
/**@{*/

/**
 * This build includes macros to access Eagle, Merlin and/or Maple microcode
 * RAM variables.
 */
#include "eagle_merlin_api_uc_vars_rdwr_defns.h"

/**@}*/



/************************************************************************//**
 * @name Direct RAM Access
 *
 * Cores/chips with a built-in microcontroller afford direct, memory-mapped
 * access to the firmware control/status RAM variables.
 *//*************************************************************************/
/**@{*/

/** Base of core variable block, non-FALCON/FALCON2 variant. */
#define CORE_VAR_RAM_BASE (0x50)
/** Base of lane variable block, non-FALCON/FALCON2 variant. */
#define LANE_VAR_RAM_BASE (0x400)
/** Size of lane variable block, non-FALCON/FALCON2 variant. */
#define LANE_VAR_RAM_SIZE (0x100)
#define CORE_VAR_RAM_SIZE (0x40)

/**@}*/



/************************************************************************//**
 * @name Diagnostic Sampling
 *
 *//*************************************************************************/
/**@{*/

#ifdef STANDALONE_EVENT
#define DIAG_MAX_SAMPLES (64)
#else
/**
 * Diagnostic sample set size, non-FALCON/FALCON2 variant.
 *
 * Applies to collections of BER measurements, eye margins, etc.
 */
#define DIAG_MAX_SAMPLES (32)
#endif

/**@}*/



/************************************************************************//**
 * @name Error-Checking Expression Wrappers
 *
 * These macros simplify checking and forwarding of error codes returned
 * either directly or indirectly in the context of functions that themselves
 * return error codes directly or indirectly.
 *
 * All expand to unterminated statements and dereference `__ERR' (defined as
 * a macro in the same header) to access either private, block-internal error
 * codes (`__err') or a common error-code cache (e.g. `global_err_code' in
 * SerDes team post-silicon evaluation builds).
 *
 * Neither `__err' nor `__ERR' should be used directly outside the API; and
 * their names may change to comply with the C Language standard reservation
 * of identifiers beginning with `__' for use by compiler implementers.
 *
 * Great care is taken to ensure not only that error returns are checked but
 * that use of an error-code cache (as in SerDes team post-silicon evaluation
 * builds) does not cause "unused variable" warnings.
 *//*************************************************************************/
/**@{*/

/**
 * Error-check a function call, returning error codes returned.
 *
 * Evaluates an expression (typically function call), stores its value into
 * `*(__ERR)' and returns it from a containing function if it is unequal to
 * `ERR_CODE_NONE'.
 *
 * EFUN() is intended for use in functions returning error codes directly to
 * check calls to functions also returning error codes directly, e.g.:
 *
 *     err_code_t foo(...) // remaining arguments elided
 *     {
 *         // ...
 *         EFUN(wrc_core_s_rstb(0x0));
 *         // ...
 *         return ERR_CODE_NONE;
 *     }
 */

#define EFUN(expr) \
    do  { \
        err_code_t __err; \
        *(__ERR) = (expr); \
        if (*(__ERR) != ERR_CODE_NONE) \
            return _error(*(__ERR)); \
        (void)__err; \
    }   while(0)

/**
 * Error-check a statement, returning error codes forwarded.
 *
 * Evaluates an expression (typically unterminated statement) that may modify
 * `*(__ERR)' and returns it from a containing function if it is unequal to
 * `ERR_CODE_NONE'.
 *
 * ESTM() is intended for use in functions returning error codes directly to
 * check calls to functions returning error codes indirectly, e.g.:
 *
 *     err_code_t foo(...) // remaining arguments elided
 *     {
 *         uint8_t rst;
 *         // ...
 *         ESTM(rst = rdc_core_s_rstb());
 *         // ...
 *         return ERR_CODE_NONE;
 *     }
 */

#define ESTM(expr) \
    do  { \
        err_code_t __err; \
        *(__ERR) = ERR_CODE_NONE; \
        (expr); \
        if (*(__ERR) != ERR_CODE_NONE) \
            return  _error(*(__ERR)); \
        (void)__err; \
    }   while(0)

/**
 * Error-check a function call, defaulting when forwarding error codes
 * returned.
 *
 * In a function taking an argument `err_code_t *err_code_p' in lieu of
 * returning an error code directly, evaluates an expression (typically
 * function call), stores its value into `*(__ERR)', combines this (bitwise
 * inclusive ore) into `*(err_code_p)', and returns a default value if either
 * `*(__ERR)' or `*(err_code_p)' is not `ERR_CODE_NONE'.
 *
 * EPFUN2() is intended for use in functions returning error codes indirectly
 * to check calls to functions returning error codes directly, e.g.:
 *
 *     uint8_t foo(err_code_t *err_code_p, ...) // remaining arguments elided
 *     {
 *         uint8_t result = 0x0;
 *         // ...
 *         EPFUN2(wrc_core_s_rstb(0x0), 0x1);
 *         // ...
 *         return result;
 *     }
 */

#define EPFUN2(expr, on_err) \
    do  { \
        err_code_t __err; \
        *(__ERR) = (expr); \
        *(err_code_p) |= *(__ERR); \
        if ((*(err_code_p) != ERR_CODE_NONE) \
         || (*(__ERR)      != ERR_CODE_NONE)) \
             return (on_err); \
        (void)__err; \
    }   while(0)

/**
 * Error-check a statement, defaulting when forwarding error codes forwarded.
 *
 * In a function taking an argument `err_code_t *err_code_p' in lieu of
 * returning an error code directly, evaluates an expression (typically
 * unterminated statement), stores its value into `*(__ERR)', combines this
 * (bitwise inclusive ore) into `*(err_code_p)', and returns a default value
 * if either `*(__ERR)' or `*(err_code_p)' is not `ERR_CODE_NONE'.
 *
 * EPSTM2() is intended for use in functions returning error codes indirectly
 * to check calls to functions also returning error codes indirectly, e.g.:
 *
 *     uint8_t foo(err_code_t *err_code_p, ...) // remaining arguments elided
 *     {
 *         uint8_t result;
 *         // ...
 *         EPSTM(result = rdc_core_s_rstb(), 0x1);
 *         // ...
 *         return result;
 *     }
 */

#define EPSTM2(expr, on_err) \
    do  { \
        err_code_t __err; \
        *(__ERR) = ERR_CODE_NONE; \
        (expr); \
        *(err_code_p) |= *(__ERR); \
        if ((*(err_code_p )!= ERR_CODE_NONE) \
         || (*(__ERR)      != ERR_CODE_NONE)) \
             return (on_err); \
        (void)__err; \
    }   while(0)

/**
 * Error-check a function call, defaulting to zero when forwarding error codes
 * returned.
 *
 * Supplies a default value of zero to EPFUN2() to reduce clutter in the most
 * common case.
 *
 * EPFUN() is intended for use in functions returning error codes indirectly
 * to check calls to functions returning error codes directly, e.g.:
 *
 *     uint8_t foo(err_code_t *err_code_p, ...) // remaining arguments elided
 *     {
 *         uint8_t result;  // determined below, detail elided
 *         // ...
 *         EPFUN(wrc_core_s_rstb(0x0));
 *         // ...
 *         return result;
 *     }
 */

#define EPFUN(expr) EPFUN2((expr), 0)

/**
 * Error-check a statement, defaulting to zero when forwarding error codes
 * forwarded.
 *
 * Supplies a default value of zero to EPSTM2() to reduce clutter in the most
 * common case.
 *
 * EPSTM() is intended for use in functions returning error codes indirectly
 * to check calls to functions also returning error codes indirectly, e.g.:
 *
 *     uint8_t foo(err_code_t *err_code_p, ...) // remaining arguments elided
 *     {
 *         uint8_t result;
 *         // ...
 *         EPSTM(result = rdc_core_s_rstb());
 *         // ...
 *         return result;
 *     }
 */

#define EPSTM(expr) EPSTM2((expr), 0)

/**
 * Invoke a function with automatic return of error on NULL result.
 *
 * ENULL() is intended for use in functions returning error codes directly to
 * check calls to functions returning pointers, e.g.:
 *
 *     err_code_t foo(...) // remaining arguments elided
 *     {
 *         // ...
 *         ENULL(strchr("foo", 'q'));
 *         // ...
 *         return ERR_CODE_NONE;
 *     }
 */
#define ENULL(expr) \
  EFUN((((void *)0!=(expr))?ERR_CODE_NONE:ERR_CODE_BAD_PTR_OR_INVALID_INPUT))

/**
 * Invoke a function with automatic forward of error on NULL result.
 *
 * EPNULL() is intended for use in functions returning error codes indirectly
 * to check calls to functions returning pointers, e.g.:
 *
 *     uint8_t foo(err_code_t *err_code_p, ...) // remaining arguments elided
 *     {
 *         uint8_t result;  // determined below, detail elided
 *         // ...
 *         EPNULL(strchr(foo, 'q'));
 *         // ...
 *         return result;
 *     }
 */
#define EPNULL(expr) \
  EPFUN((((void *)0!=(expr))?ERR_CODE_NONE:ERR_CODE_BAD_PTR_OR_INVALID_INPUT))

/**
 * Invoke USR_PRINTF(()) with non-error-code-generating arguments.
 *
 * Note that the single argument is a parenthesized argument list to be
 * passed to USR_PRINTF(()).
 *
 * EFUN_PRINTF(()) is intended for use in functions returning error codes
 * directly, with an argument list the elements of which do not generate
 * error codes of any kind, e.g.:
 *
 *     err_code_t foo(...) // remaining arguments elided
 *     {
 *         // ...
 *         EFUN_PRINTF(("%u", 1));
 *         // ...
 *         return ERR_CODE_NONE;
 *     }
 */
#define EFUN_PRINTF(paren_arg_list) USR_PRINTF(paren_arg_list)

/**
 * Invoke USR_PRINTF(()) with error-code-generating arguments that would
 * otherwise be handled by ESTM().
 *
 * Note that the single argument is a parenthesized argument list to be
 * passed to USR_PRINTF(()).
 *
 * EFUN_PRINTF(()) is intended for use in functions returning error codes
 * directly, with an argument list the elements of which may generate error
 * codes indirectly, e.g.:
 *
 *     err_code_t foo(...) // remaining arguments elided
 *     {
 *         // ...
 *         ESTM_PRINTF(("%u", rdc_core_s_rstb()));
 *         // ...
 *         return ERR_CODE_NONE;
 *     }
 */
#define ESTM_PRINTF(paren_arg_list) \
    do  { \
        err_code_t __err; \
        *(__ERR) = ERR_CODE_NONE; \
        USR_PRINTF(paren_arg_list); \
        if (*(__ERR) != ERR_CODE_NONE) \
            return  _error(*(__ERR)); \
        (void)__err; \
    }   while(0)

/**
 * Invoke possibly-remapped 'memset()' and, if it returns NULL, force an
 * error return.
 *
 * Ordinarily, standard implementations of 'memset' will return NULL only if
 * passed a NULL destination address, and *may already* have overwritten an
 * inappropriate address range before returning:  nevertheless, a specialized
 * implementation could use a NULL return to indicate other failures.  In
 * either case, execution should not be allowed to proceed on NULL return.
 */
#define ENULL_MEMSET(mem, val, num) ENULL((USR_MEMSET((mem), (val), (num))))

/**
 * Invoke possibly-remapped 'memset()' and, if it returns NULL, force an
 * error to be forwarded.
 *
 * Ordinarily, standard implementations of 'memset' will return NULL only if
 * passed a NULL destination address, and *may already* have overwritten an
 * inappropriate address range before returning:  nevertheless, a specialized
 * implementation could use a NULL return to indicate other failures.  In
 * either case, execution should not be allowed to proceed on NULL return.
 */
#define EPNULL_MEMSET(mem, val, num) EPNULL((USR_MEMSET((mem), (val), (num))))

/**
 * Invoke possibly-remapped 'strcpy()' and, if it returns NULL, force an
 * error return.
 *
 * Ordinarily, standard implementations of 'strcpy' will return NULL only if
 * passed a NULL destination address, and *may already* have overwritten an
 * inappropriate address range before returning:  nevertheless, a specialized
 * implementation could use a NULL return to indicate other failures.  In
 * either case, execution should not be allowed to proceed on NULL return.
 */
#define ENULL_STRCPY(dst, src) ENULL((USR_STRCPY((dst), (src))))

/**
 * Invoke possibly-remapped 'strcpy()' and, if it returns NULL, force an
 * error to be forwarded.
 *
 * Ordinarily, standard implementations of 'strcpy' will return NULL only if
 * passed a NULL destination address, and *may already* have overwritten an
 * inappropriate address range before returning:  nevertheless, a specialized
 * implementation could use a NULL return to indicate other failures.  In
 * either case, execution should not be allowed to proceed on NULL return.
 */
#define EPNULL_STRCPY(dst, src) EPNULL((USR_STRCPY((dst), (src))))

/**
 * Invoke possibly-remapped 'strcpy()' and, if it returns NULL, force an
 * error return.
 *
 * Ordinarily, standard implementations of 'strncpy' will return NULL only if
 * passed a NULL destination address, and *may already* have overwritten an
 * inappropriate address range before returning:  nevertheless, a specialized
 * implementation could use a NULL return to indicate other failures.  In
 * either case, execution should not be allowed to proceed on NULL return.
 */
#define ENULL_STRNCAT(dst, src) ENULL((USR_STRNCAT((dst), (src))))

/**
 * Invoke possibly-remapped 'strcpy()' and, if it returns NULL, force an
 * error to be forwarded.
 *
 * Ordinarily, standard implementations of 'strncpy' will return NULL only if
 * passed a NULL destination address, and *may already* have overwritten an
 * inappropriate address range before returning:  nevertheless, a specialized
 * implementation could use a NULL return to indicate other failures.  In
 * either case, execution should not be allowed to proceed on NULL return.
 */
#define EPNULL_STRNCAT(dst, src) EPNULL((USR_STRNCAT((dst), (src))))

/**@}*/

/************************************************************************//**
 * @name Display Utility Macros
 *//*************************************************************************/
/**@{*/

/** Display a signed integer variable. */
#define DISP(x) ESTM_PRINTF(("%s = %d\n", #x, x))

/** Display an unsigned integer variable. */
#define DISPU(x) ESTM_PRINTF(("%s = %u\n", #x, x))

/** Display a floating point variable. */
#define DISPF(x) ESTM_PRINTF(("%s = %f\n", #x, x))

/** Display an integer variable in hex. */
#define DISPX(x) ESTM_PRINTF(("%s = %x\n", #x, x))

/** Read and display the value of a lane register field in decimal. */
#define DISP_REG(x) ESTM_PRINTF(("%s = %d\n", #x, rd_##x##()))

/** Read and display the value of a lane register field in hex. */
#define DISP_REGX(x) ESTM_PRINTF(("%s = %x\n", #x, rd_##x##()))

/** Read and display the value of a core register field in hex. */
#define DISP_REGC(x) ESTM_PRINTF(("%s = %x\n", #x, rdc_##x##()))

/** Display a single member of a lane struct. */
#define DISP_LN_VARS(name,param,format) \
    do { \
        ESTM_PRINTF(("%-16s\t",name)); \
        for(i=0;i<num_lanes;i++) { \
            ESTM_PRINTF((format,(lane_st[i].param))); \
        } \
        EFUN_PRINTF(("\n"));    \
    }   while (0)

/** Display two members of a lane struct. */
#define DISP_LNQ_VARS(name,param1,param2,format) \
    do { \
        ESTM_PRINTF(("%-16s\t",name)); \
        for(i=0;i<num_lanes;i++) { \
            ESTM_PRINTF((format,(lane_st[i].param1),(lane_st[i].param2))); \
        } \
        EFUN_PRINTF(("\n"));    \
    }   while (0)

/**@}*/

/************************************************************************//**
 * @name Arithmetic Utility Macros
 *//*************************************************************************/
/**@{*/

/**
 * Clockwise difference between phase counters.
 */
#define dist_cw(a,b) (((a)<=(b))?((b)-(a)):((uint16_t)256-(a)+(b)))

/**
 * Counter-clockwise difference between phase counters
 */
#define dist_ccw(a,b) (((a)>=(b))?((a)-(b)):((uint16_t)256+(a)-(b)))

/**
 * Lesser of two expressions.
 *
 * @warning
 *
 *    May evaluate the selected expression twice.
 */
#define _min(a,b) (((a)>(b)) ? (b) : (a))

/**
 * Greater of two expressions.
 *
 * @warning
 *
 *    May evaluate the selected expression twice.
 */
#define _max(a,b) (((a)>(b)) ? (a) : (b))

/**
 * Absolute value of an expression.
 *
 * @warning
 *
 *    May evaluate the given expression twice.
 */
#define _abs(a) (((a)>0) ? (a) : (-(a)))

/**@}*/
#endif
