/***********************************************************************
 *
 *  Copyright (c) 2011  Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2011:DUAL/GPL:standard
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

