/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2021:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
:>
 *
************************************************************************/

#ifndef __BCM_STRUTILS_H__
#define __BCM_STRUTILS_H__

#include <string.h>



/*!\file bcm_strutils.h
 * \brief Broadcom string helper functions.  Similar to CMS strconv but
 *        not tied to CMS.
 *
 */

#if defined __cplusplus
extern "C" {
#endif

/** By default, this is just a wrapper around strsep().
 *
 * In case strsep is not available, there is a Broadccom implementation which
 * behaves just like standard strtok_r with 3 exceptions:
 *
 * 1. contiguous delimitors are NOT treated like a single delimitor.  Two
 *    contiguous delimitors is treated as if there is an empty string value
 *    between the two delimitors.  Returned char *ptr == '\0'.
 * 2. Delimitors must be only a single character.  Multiple delimitors not
 *    supported.
 * 3. If there is no more data to parse, *savePtr will be NULL on exit.
 *
 * See man page for strtok_r for behavior.
 *
 * @return ptr to token, or NULL if no more tokens or error.
 */
char *bcmUtl_strtok_r(char *str, const char *delim, char **saveptr);


#if defined __cplusplus
};
#endif
#endif  /* __BCM_STRUTILS_H__ */
