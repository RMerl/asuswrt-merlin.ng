/***********************************************************************
 *
 *  Copyright (c) 2021  Broadcom Ltd
 *  All Rights Reserved
 *
 * <:label-BRCM:2021:DUAL/GPL:standard
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
