/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
 *  All Rights Reserved
 *
<:label-BRCM:2012:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
 *
 ************************************************************************/

#ifndef __CMS_SECLOG_H__
#define __CMS_SECLOG_H__

/*!\enum CmsSecurityLogIDs
 * \brief Security Log identifiers.
 * 
 */
typedef enum
{
   LOG_SECURITY_CUSTOM             = 0, /**< Used for custom strings, blank default text */
   LOG_SECURITY_PWD_CHANGE_SUCCESS = 1, /**< Password change successful */
   LOG_SECURITY_PWD_CHANGE_FAIL,        /**< Password change failed */
   LOG_SECURITY_AUTH_LOGIN_PASS,        /**< An authorized user successfully logged in. */
   LOG_SECURITY_AUTH_LOGIN_FAIL,        /**< An authorized user failed login. */
   LOG_SECURITY_AUTH_LOGOUT,            /**< An authorized user has logged out. */
   LOG_SECURITY_LOCKOUT_START,          /**< Security lockout added */
   LOG_SECURITY_LOCKOUT_END,            /**< Security lockout removed. */
   LOG_SECURITY_AUTH_RESOURCES,         /**< Authorized attempt to access resources. */
   LOG_SECURITY_UNAUTH_RESOURCES,       /**< Unauthorized attempt to access resources. */
   LOG_SECURITY_SOFTWARE_MOD,           /**< Software update occurred. */
   LOG_SECURITY_MAX,                    /**< Used for range checking */
} CmsSecurityLogIDs;


/** Max number of ms to wait for a MDM read lock. */
#define CMSLOG_LOCK_TIMEOUT  (3 * MSECS_IN_SEC)


#define CMSLOG_SEC_COUNT_FLAG    (1 << 0)
#define CMSLOG_SEC_PORT_FLAG     (1 << 1)
#define CMSLOG_SEC_SRC_IP_FLAG   (1 << 2)
#define CMSLOG_SEC_USER_FLAG     (1 << 3)
#define CMSLOG_SEC_LEVEL_FLAG    (1 << 4)
#define CMSLOG_SEC_APP_NAME_FLAG (1 << 5)

#define CMSLOG_SEC_SET_COUNT(p, d)   {(p)->count = (d); (p)->data_flags |= CMSLOG_SEC_COUNT_FLAG;}
#define CMSLOG_SEC_SET_PORT(p, d)    {(p)->port = (d);  (p)->data_flags |= CMSLOG_SEC_PORT_FLAG;}
#define CMSLOG_SEC_SET_SRC_IP(p, d)  {(p)->ipAddr = (d); (p)->data_flags |= CMSLOG_SEC_SRC_IP_FLAG;}
#define CMSLOG_SEC_SET_APP_NAME(p, d) {(p)->appName = (d); (p)->data_flags |= CMSLOG_SEC_APP_NAME_FLAG;}
#define CMSLOG_SEC_SET_USER(p, d)    {(p)->user = (d);  (p)->data_flags |= CMSLOG_SEC_USER_FLAG;}
#define CMSLOG_SEC_SET_LEVEL(p, d)   {(p)->security_level = (d);  (p)->data_flags |= CMSLOG_SEC_LEVEL_FLAG;}

/** security log length */
#define SECURITY_LOG_RD_OFFSET   (0)
#define SECURITY_LOG_RD_SIZE     (sizeof(uint32_t))
#define SECURITY_LOG_WR_OFFSET   (4)
#define SECURITY_LOG_WR_SIZE     (sizeof(uint32_t))
#define SECURITY_LOG_DATA_OFFSET (8)
#define SECURITY_LOG_DATA_SIZE   (4 * 1024)
#define SECURITY_LOG_FILE_NAME  "/data/securitylog"

/** Structure to hold relevant data regarding security log events.
 *
 */
typedef struct 
{
   UINT32         count;
   UINT32         port;
   char *         appName;
   char *         ipAddr;
   char *         user;
   UINT32         security_level;
   UINT32         data_flags;
} CmsSecurityLogData;

#define EMPTY_CMS_SECURITY_LOG_DATA { 0, 0, NULL, NULL, NULL, 0, 0 }

/** Security log file structure 
 *
 */
typedef struct
{
   unsigned long read_offset;       /** offset of oldest log */
   unsigned long write_offset;      /** offset of newest log */
   char log[SECURITY_LOG_DATA_SIZE];     /** security log strings */   
} CmsSecurityLogFile;


/** Log a security log in the system.
 *
 * This function logs security logs into flash for non-volatile storage.
 * There are a specified list of logs that can be used, or the user may 
 * add their own string.
 *
 * @param id (IN) Predefined security log message string.
 * @param pdata (IN) Structure containing relevant log information. 
 * @param pFmt (IN) The message string.
 *
 * @return CMSRET_SUCCESS on success, otherwise an error occurred.
 */
CmsRet cmsLog_security(CmsSecurityLogIDs id, CmsSecurityLogData * pdata,
                           const char *pFmt, ... );


/** Retrieve the security log from the system.
 *
 * @param log (OUT) Pointer to CmsSecurityLogFile structure. 
 *                  Memory must be allocated by the caller.
 *
 * @return CMSRET_SUCCESS on success, otherwise an error occurred.
 */
CmsRet cmsLog_getSecurityLog(CmsSecurityLogFile * log);


/** Print the security log to stdout.
 *
 * @param log (OUT) Pointer to CmsSecurityLogFile structure. 
 *                  Memory must be allocated by the caller.
 *
 * @return None
 */
void cmsLog_printSecurityLog(CmsSecurityLogFile * log);


/** Reset and clear all entries in the security log in the system.
 *
 * @return CMSRET_SUCCESS on success, otherwise an error occurred.
 */
CmsRet cmsLog_resetSecurityLog(void);

#endif /* CMS_SECURITY_LOG */

