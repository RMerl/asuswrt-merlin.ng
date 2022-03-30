/***********************************************************************
 *
 *  Copyright (c) 2007  Broadcom Corporation
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


/** Get default passwords from /proc/environment (UBoot env var) if it exists.
 *  /proc/environment/default_admin_password
 *  /proc/environment/default_support_password
 *  /proc/environment/default_user_password
 * 
 * @param buf     (OUT) should be at least CMS_MAX_PASSWORD_LENGTH bytes.
 * @param bufSize (IN)  should be CMS_MAX_PASSWORD_LENGTH-1 to ensure a null
 *                      terminator at the end.
 *
 * @return CMSRET_SUCCESS if the default password was found and read in.
 */
CmsRet cmsUtil_getDefaultAdminPassword(char *buf, UINT32 bufSize);
CmsRet cmsUtil_getDefaultSupportPassword(char *buf, UINT32 bufSize);
CmsRet cmsUtil_getDefaultUserPassword(char *buf, UINT32 bufSize);


#endif /* __CMS_PWD_H__ */

