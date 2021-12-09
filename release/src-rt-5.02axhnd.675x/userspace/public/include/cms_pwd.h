/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
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

#ifndef __CMS_PWD_H__
#define __CMS_PWD_H__

/*!\file cms_pwd.h
 * \brief Header file for password utilities
 */

#include "cms.h"

/** create salt for use with cmsUtil_pwCrypt
 *
 * @return pointer to salt
 */
char *cmsUtil_cryptMakeSalt(void);


/** create hashed password
 *
 * @param clear (IN) clear text password
 * @param salt  (IN) salt for password hashing
 *
 * @return pointer to hashed password
 */
char *cmsUtil_pwEncrypt(const char *clear, const char *salt);

#endif /* __CMS_PWD_H__ */

