/***********************************************************************
 *
 *  Copyright (c) 2011  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
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

#ifndef __CMS_RAND_H__
#define __CMS_RAND_H__


/*!\file cms_rand.h
 * \brief Header file for various random number utility funcs in CMS.
 *
 */

#include "cms.h"


#define MAX_RANDSTRING_LEN   127

/** Get a random string of specified length.
 *
 * Before calling this function, the app should call srand() to
 * seed the random number generator.
 *
 * Note this function is not multi-thread safe because it uses
 * an internal static buffer.  A multi-thread safe version of this
 * function (_r) may be implemented if there is a need for it.
 *
 * @param minchars (IN)  Minimum number of chars in the string.  Must be
 *                       greater than 0.
 * @param maxchars (IN)  Maximum number of chars in the string.  Must be
 *                       equal to or greater than min chars and not more
 *                       than MAX_RANDSTRING_LEN
 *
 * @return pointer to random string or NULL on error.
 */
char *cmsRand_getRandString(UINT32 minChars, UINT32 maxChars);

#endif  /* __CMS_RAND_H__ */

