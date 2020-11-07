/***********************************************************************
 *
 * Copyright (c) 2006-2007  Broadcom Corporation
 * All Rights Reserved
 *
 * <:label-BRCM:2006-2007:DUAL/GPL:standard
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
 * :>
 *
 ************************************************************************/

#ifndef __CMS_RETCODES_H__
#define __CMS_RETCODES_H__    


/*!\enum CmsRet
 * \brief Return codes used throughout the system (CMS and non-CMS).
 *
 * Originally, these return codes were intended to be used by CMS code only,
 * but a lot of non-CMS code have been using them as well.
 * In the future, non-CMS code should use their own header file
 * to define their own return codes or use bcm_retcodes.h.
 * 
 * Reserved Ranges:
 *  0          : TR69 success return code.
 *  9000 - 9799: TR69 return values.
 *  9800 - 9899: vendor defined TR69 return codes.
 * 18000 -18999: CMS internal error codes, never sent out by TR69.
 * 23000 -23999: Broadcom (non-CMS) internal error codes, never sent out by
 *               TR69.  Define these in bcm_retcodes.h.
 *
 * All other values are reserved.
 */
typedef enum
{
   CMSRET_SUCCESS              = 0,     /**<Success. */
   CMSRET_METHOD_NOT_SUPPORTED = 9000,  /**<Method not supported. */
   CMSRET_REQUEST_DENIED       = 9001,  /**< Request denied (no reason specified). */
   CMSRET_INTERNAL_ERROR       = 9002,  /**< Internal error. */
   CMSRET_INVALID_ARGUMENTS    = 9003,  /**< Invalid arguments. */
   CMSRET_RESOURCE_EXCEEDED    = 9004,  /**< Resource exceeded.
                                        *  (when used in association with
                                        *  setParameterValues, this MUST not be
                                        *  used to indicate parameters in error)
                                        */
   CMSRET_INVALID_PARAM_NAME   = 9005,  /**< Invalid parameter name.
                                        *  (associated with set/getParameterValues,
                                        *  getParameterNames,set/getParameterAtrributes)
                                        */
   CMSRET_INVALID_PARAM_TYPE   = 9006,  /**< Invalid parameter type.
                                        *  (associated with set/getParameterValues)
                                        */
   CMSRET_INVALID_PARAM_VALUE  = 9007,  /**< Invalid parameter value.
                                        *  (associated with set/getParameterValues)
                                        */
   CMSRET_SET_NON_WRITABLE_PARAM = 9008,/**< Attempt to set a non-writable parameter.
                                        *  (associated with setParameterValues)
                                        */
   CMSRET_NOTIFICATION_REQ_REJECTED = 9009, /**< Notification request rejected.
                                            *  (associated with setParameterAttributes)
                                            */
   CMSRET_DOWNLOAD_FAILURE     = 9010,  /**< Download failure.
                                         *  (associated with download or transferComplete)
                                         */
   CMSRET_UPLOAD_FAILURE       = 9011,  /**< Upload failure.
                                        *  (associated with upload or transferComplete)
                                        */
   CMSRET_FILE_TRANSFER_AUTH_FAILURE = 9012,  /**< File transfer server authentication
                                              *  failure.
                                              *  (associated with upload, download
                                              *  or transferComplete)
                                              */
   CMSRET_UNSUPPORTED_FILE_TRANSFER_PROTOCOL = 9013,/**< Unsupported protocol for file
                                                    *  transfer.
                                                    *  (associated with upload or
                                                    *  download)
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_JOIN_MULTICAST = 9014,/**< File transfer failure,
                                                    *  unable to join multicast
                                                    *  group.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_CONTACT_FILE_SERVER = 9015,/**< File transfer failure,
                                                    *  unable to contact file server.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_ACCESS_FILE = 9016,/**< File transfer failure,
                                                    *  unable to access file.
                                                    */
   CMSRET_FILE_TRANSFER_UNABLE_COMPLETE = 9017,/**< File transfer failure,
                                                    *  unable to complete download.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_CORRUPTED = 9018,/**< File transfer failure,
                                                    *  file corrupted.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_AUTHENTICATION_ERROR = 9019,/**< File transfer failure,
                                                    *  file authentication error.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_TIMEOUT = 9020,/**< File transfer failure,
                                                    *  download timeout.
                                                    */
   CMSRET_FILE_TRANSFER_FILE_CANCELLATION_NOT_ALLOW = 9021,/**< File transfer failure,
                                                    *  cancellation not permitted.
                                                    */
   CMSRET_INVALID_UUID_FORMAT = 9022,/**< Invalid UUID Format
                                                    * (associated with ChangeDUState)
                                                    */
   CMSRET_UNKNOWN_EE = 9023,/**< Unknown Execution Environment
                                                    * (associated with ChangeDUState)
                                                    */

   CMSRET_EE_DISABLED = 9024,/**< Execution Environment disabled
                                                    * (associated with ChangeDUState)
                                                    */
   CMSRET_DU_EE_MISMATCH = 9025,/**< Execution Environment and Deployment Unit mismatch
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_DU_DUPLICATE = 9026,/**< Duplicate Deployment Unit
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_SW_MODULE_SYSTEM_RESOURCE_EXCEEDED = 9027,/**< System resources exceeded
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   CMSRET_DU_UNKNOWN = 9028,/**< Unknown Deployment Unit
                                                    * (associated with ChangeDUState:update/uninstall)
                                                    */
   CMSRET_DU_STATE_INVALID = 9029,/**< Invalid Deployment Unit State
                                                    * (associated with ChangeDUState:update)
                                                    */
   CMSRET_DU_UPDATE_DOWNGRADE_NOT_ALLOWED = 9030,/**< Invalid Deployment Unit Update, downgrade not permitted
                                                    * (associated with ChangeDUState:update)
                                                    */
   CMSRET_DU_UPDATE_VERSION_NOT_SPECIFIED = 9031,/**< Invalid Deployment Unit Update, version not specified
                                                    * (associated with ChangeDUState:update)
                                                    */
   CMSRET_DU_UPDATE_VERSION_EXISTED= 9032,/**< Invalid Deployment Unit Update, version already exists
                                                    * (associated with ChangeDUState:update)
                                                    */
   
   CMSRET_SUCCESS_REBOOT_REQUIRED = 9800, /**< Config successful, but requires reboot to take effect. */
   CMSRET_SUCCESS_UNRECOGNIZED_DATA_IGNORED = 9801,  /**<Success, but some unrecognized data was ignored. */
   CMSRET_SUCCESS_OBJECT_UNCHANGED = 9802,  /**<Success, furthermore object has not changed, returned by STL handler functions. */
   CMSRET_SUCCESS_APPLY_NOT_COMPLETE = 9803, /**< Config validated/commited, but requires more action to take effect. */
   CMSRET_NO_MORE_INSTANCES = 9804,     /**<getnext operation cannot find any more instances to return. */
   CMSRET_MDM_TREE_ERROR = 9805,         /**<Error during MDM tree traversal */
   CMSRET_WOULD_DEADLOCK = 9806, /**< Caller is requesting a lock while holding the same lock or a different one. */
   CMSRET_LOCK_REQUIRED = 9807,  /**< The MDM lock is required for this operation. */
   CMSRET_OP_INTR = 9808,      /**<Operation was interrupted, most likely by a Linux signal. */
   CMSRET_TIMED_OUT = 9809,     /**<Operation timed out. */
   CMSRET_DISCONNECTED = 9810,  /**< Communications link is disconnected. */
   CMSRET_MSG_BOUNCED = 9811,   /**< Msg was sent to a process not running, and the
                                 *   bounceIfNotRunning flag was set on the header.  */
   CMSRET_OP_ABORTED_BY_USER = 9812,  /**< Operation was aborted/discontinued by the user */
   CMSRET_FAIL_REBOOT_REQUIRED = 9813,  /**<Config failed, and now system is in a bad state requiring reboot. */
   CMSRET_ACCESS_DENIED = 9814,  /**< Data model access denied (no reason specified). */
   CMSRET_OPERATION_NOT_PERMITTED= 9815,  /**< Operation not permitted (errno EPERM) */
   CMSRET_RECURSION_ERROR = 9817,     /**< too many levels of recursion */
   CMSRET_OPEN_FILE_ERROR = 9818,     /**< open file error */
   CMSRET_EAGAIN_ERROR = 9820,        /**< socket write EAGAIN error */
   CMSRET_SOCKET_ERROR = 9821,        /**< socket error */
   CMSRET_KEY_GENERATION_ERROR = 9830,     /** certificate key generation error */
   CMSRET_INVALID_CERT_REQ = 9831,     /** requested certificate does not match with issued certificate */
   CMSRET_INVALID_CERT_SUBJECT = 9832,     /** certificate has invalid subject information */
   CMSRET_OBJECT_NOT_FOUND = 9840,     /** failed to find object */
   CMSRET_OBJECT_SEQUENCE_ERROR = 9841, /**< Sequence number check failed. */

   CMSRET_INVALID_FILENAME = 9850,  /**< filename was not given for download */
   CMSRET_INVALID_IMAGE = 9851,     /**< bad image was given for download */
   CMSRET_INVALID_CONFIG_FILE = 9852,  /**< invalid config file was detected */
   CMSRET_CONFIG_PSI = 9853,         /**< old PSI/3.x config file was detected */
   CMSRET_IMAGE_FLASH_FAILED = 9854, /**< could not write the image to flash */
   CMSRET_RESOURCE_NOT_CONFIGURED = 9855, /**< requested resource is not configured/found */
   CMSRET_EE_UNPACK_ERROR = 9856,   /**< EE download unpack failure (associated with ChangeEEState) */
   CMSRET_EE_DUPLICATE = 9857,   /**< Duplicate Execution Environment (associated with ChangeEEState:install) */
   CMSRET_EE_UPDATE_DOWNGRADE_NOT_ALLOWED = 9858,/**< Invalid Execution Environment Update, downgrade not permitted
                                                    * (associated with ChangeEEState:update)
                                                    */
   CMSRET_EE_UPDATE_VERSION_NOT_SPECIFIED = 9859,/**< Invalid Execution Environment Update, version not specified
                                                    * (associated with ChangeEEState:update)
                                                    */
   CMSRET_EE_UPDATE_VERSION_EXISTED = 9860,/**< Invalid Execution Environment Update, version already exists
                                                    * (associated with ChangeEEState:update)
                                                    */
   CMSRET_PMD_CALIBRATION_FILE_SUCCESS = 9861, /**<Success. PMD Calibration file was uploaded.*/
   CMSRET_PMD_TEMP2APD_FILE_SUCCESS = 9862, /**<Success. PMD Temp2Apd file was uploaded.*/
   CMSRET_MANIFEST_PARSE_ERROR = 9863,   /**< Manifest parse error. */
   CMSRET_USERNAME_IN_USE = 9864,   /**< Username in use. */
   CMSRET_ADD_USER_ERROR = 9865,    /**< Add user error. */
   CMSRET_DELETE_USER_ERROR = 9866, /**< Delete user error. */
   CMSRET_USER_NOT_FOUND = 9867,    /**<User not found. */
   CMSRET_SET_BUSGATE_POLICY_ERROR = 9868,   /**<Set busgate policy error. */
   CMSRET_OPERATION_IN_PROCESS = 9869,    /**<Operation is still in process. */
   CMSRET_CONVERSION_ERROR = 9870, /**< Error during data conversion. */
   CMSRET_PARENTEE_EE_VERSION_MISMATCH = 9871, /**<EE version is not supported by platform version (BEEP Version)*/
   CMSRET_INVALID_URL_FORMAT = 9872,/**< Invalid URL Format
                                      * (associated with ChangeDUState)
                                      */
   CMSRET_PMD_JSON_FILE_SUCCESS = 9873, /**<Success. PMD JSON file was uploaded.*/
   CMSRET_UNKNOWN_ERROR = 9899,  /**< Can't figure out the real cause.  Use sparingly. */
} CmsRet;


#endif /* __CMS_RETCODES_H__ */
