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

#ifndef __BCM_RETCODES_H__
#define __BCM_RETCODES_H__


/*!\enum BcmRet
 * \brief This file provides an alternative set of return codes similar to CmsRet.
 *
 * Initially, BcmRet and CmsRet will be exactly the same so they can be used
 * interchangably.  Over time, they will diverge, and that is OK.  New code should
 * not use them interchangably, but we will still keep new return codes in both
 * sets distinct from each other so there should be no confusion between the two.
 *
 * Reserved Ranges:
 *  0          : TR69 success return code.
 *  9000 - 9799: TR69 return values.
 *  9800 - 9899: vendor defined TR69 return codes.
 * 18000 -18999: CMS internal error codes, never sent out by TR69.
 * 23000 -23999: Broadcom (non-CMS) internal error codes, never sent out by
 *               TR69.  New codes defined in this file should be in this range.
 *
 * All other values are reserved.
 */
typedef enum
{
   BCMRET_SUCCESS              = 0,     /**<Success. */
   BCMRET_METHOD_NOT_SUPPORTED = 9000,  /**<Method not supported. */
   BCMRET_REQUEST_DENIED       = 9001,  /**< Request denied (no reason specified). */
   BCMRET_INTERNAL_ERROR       = 9002,  /**< Internal error. */
   BCMRET_INVALID_ARGUMENTS    = 9003,  /**< Invalid arguments. */
   BCMRET_RESOURCE_EXCEEDED    = 9004,  /**< Resource exceeded.
                                        *  (when used in association with
                                        *  setParameterValues, this MUST not be
                                        *  used to indicate parameters in error)
                                        */
   BCMRET_INVALID_PARAM_NAME   = 9005,  /**< Invalid parameter name.
                                        *  (associated with set/getParameterValues,
                                        *  getParameterNames,set/getParameterAtrributes)
                                        */
   BCMRET_INVALID_PARAM_TYPE   = 9006,  /**< Invalid parameter type.
                                        *  (associated with set/getParameterValues)
                                        */
   BCMRET_INVALID_PARAM_VALUE  = 9007,  /**< Invalid parameter value.
                                        *  (associated with set/getParameterValues)
                                        */
   BCMRET_SET_NON_WRITABLE_PARAM = 9008,/**< Attempt to set a non-writable parameter.
                                        *  (associated with setParameterValues)
                                        */
   BCMRET_NOTIFICATION_REQ_REJECTED = 9009, /**< Notification request rejected.
                                            *  (associated with setParameterAttributes)
                                            */
   BCMRET_DOWNLOAD_FAILURE     = 9010,  /**< Download failure.
                                         *  (associated with download or transferComplete)
                                         */
   BCMRET_UPLOAD_FAILURE       = 9011,  /**< Upload failure.
                                        *  (associated with upload or transferComplete)
                                        */
   BCMRET_FILE_TRANSFER_AUTH_FAILURE = 9012,  /**< File transfer server authentication
                                              *  failure.
                                              *  (associated with upload, download
                                              *  or transferComplete)
                                              */
   BCMRET_UNSUPPORTED_FILE_TRANSFER_PROTOCOL = 9013,/**< Unsupported protocol for file
                                                    *  transfer.
                                                    *  (associated with upload or
                                                    *  download)
                                                    */
   BCMRET_FILE_TRANSFER_UNABLE_JOIN_MULTICAST = 9014,/**< File transfer failure,
                                                    *  unable to join multicast
                                                    *  group.
                                                    */
   BCMRET_FILE_TRANSFER_UNABLE_CONTACT_FILE_SERVER = 9015,/**< File transfer failure,
                                                    *  unable to contact file server.
                                                    */
   BCMRET_FILE_TRANSFER_UNABLE_ACCESS_FILE = 9016,/**< File transfer failure,
                                                    *  unable to access file.
                                                    */
   BCMRET_FILE_TRANSFER_UNABLE_COMPLETE = 9017,/**< File transfer failure,
                                                    *  unable to complete download.
                                                    */
   BCMRET_FILE_TRANSFER_FILE_CORRUPTED = 9018,/**< File transfer failure,
                                                    *  file corrupted.
                                                    */
   BCMRET_FILE_TRANSFER_FILE_AUTHENTICATION_ERROR = 9019,/**< File transfer failure,
                                                    *  file authentication error.
                                                    */
   BCMRET_FILE_TRANSFER_FILE_TIMEOUT = 9020,/**< File transfer failure,
                                                    *  download timeout.
                                                    */
   BCMRET_FILE_TRANSFER_FILE_CANCELLATION_NOT_ALLOW = 9021,/**< File transfer failure,
                                                    *  cancellation not permitted.
                                                    */
   BCMRET_INVALID_UUID_FORMAT = 9022,/**< Invalid UUID Format
                                                    * (associated with ChangeDUState)
                                                    */
   BCMRET_UNKNOWN_EE = 9023,/**< Unknown Execution Environment
                                                    * (associated with ChangeDUState)
                                                    */

   BCMRET_EE_DISABLED = 9024,/**< Execution Environment disabled
                                                    * (associated with ChangeDUState)
                                                    */
   BCMRET_DU_EE_MISMATCH = 9025,/**< Execution Environment and Deployment Unit mismatch
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   BCMRET_DU_DUPLICATE = 9026,/**< Duplicate Deployment Unit
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   BCMRET_SW_MODULE_SYSTEM_RESOURCE_EXCEEDED = 9027,/**< System resources exceeded
                                                    * (associated with ChangeDUState:install/update)
                                                    */
   BCMRET_DU_UNKNOWN = 9028,/**< Unknown Deployment Unit
                                                    * (associated with ChangeDUState:update/uninstall)
                                                    */
   BCMRET_DU_STATE_INVALID = 9029,/**< Invalid Deployment Unit State
                                                    * (associated with ChangeDUState:update)
                                                    */
   BCMRET_DU_UPDATE_DOWNGRADE_NOT_ALLOWED = 9030,/**< Invalid Deployment Unit Update, downgrade not permitted
                                                    * (associated with ChangeDUState:update)
                                                    */
   BCMRET_DU_UPDATE_VERSION_NOT_SPECIFIED = 9031,/**< Invalid Deployment Unit Update, version not specified
                                                    * (associated with ChangeDUState:update)
                                                    */
   BCMRET_DU_UPDATE_VERSION_EXISTED= 9032,/**< Invalid Deployment Unit Update, version already exists
                                                    * (associated with ChangeDUState:update)
                                                    */
   
   BCMRET_SUCCESS_REBOOT_REQUIRED = 9800, /**< Config successful, but requires reboot to take effect. */
   BCMRET_SUCCESS_UNRECOGNIZED_DATA_IGNORED = 9801,  /**<Success, but some unrecognized data was ignored. */
   BCMRET_SUCCESS_OBJECT_UNCHANGED = 9802,  /**<Success, furthermore object has not changed, returned by STL handler functions. */
   BCMRET_SUCCESS_APPLY_NOT_COMPLETE = 9803, /**< Config validated/commited, but requires more action to take effect. */
   BCMRET_NO_MORE_INSTANCES = 9804,     /**<getnext operation cannot find any more instances to return. */
   BCMRET_MDM_TREE_ERROR = 9805,         /**<Error during MDM tree traversal */
   BCMRET_WOULD_DEADLOCK = 9806, /**< Caller is requesting a lock while holding the same lock or a different one. */
   BCMRET_LOCK_REQUIRED = 9807,  /**< The MDM lock is required for this operation. */
   BCMRET_OP_INTR = 9808,      /**<Operation was interrupted, most likely by a Linux signal. */
   BCMRET_TIMED_OUT = 9809,     /**<Operation timed out. */
   BCMRET_DISCONNECTED = 9810,  /**< Communications link is disconnected. */
   BCMRET_MSG_BOUNCED = 9811,   /**< Msg was sent to a process not running, and the
                                 *   bounceIfNotRunning flag was set on the header.  */
   BCMRET_OP_ABORTED_BY_USER = 9812,  /**< Operation was aborted/discontinued by the user */
   BCMRET_FAIL_REBOOT_REQUIRED = 9813,  /**<Config failed, and now system is in a bad state requiring reboot. */
   BCMRET_ACCESS_DENIED = 9814,  /**< Data model access denied (no reason specified). */
   BCMRET_OPERATION_NOT_PERMITTED= 9815,  /**< Operation not permitted (errno EPERM) */
   BCMRET_RECURSION_ERROR = 9817,     /**< too many levels of recursion */
   BCMRET_OPEN_FILE_ERROR = 9818,     /**< open file error */
   BCMRET_EAGAIN_ERROR = 9820,        /**< socket write EAGAIN error */
   BCMRET_SOCKET_ERROR = 9821,        /**< socket error */
   BCMRET_KEY_GENERATION_ERROR = 9830,     /** certificate key generation error */
   BCMRET_INVALID_CERT_REQ = 9831,     /** requested certificate does not match with issued certificate */
   BCMRET_INVALID_CERT_SUBJECT = 9832,     /** certificate has invalid subject information */
   BCMRET_OBJECT_NOT_FOUND = 9840,     /** failed to find object */
   BCMRET_OBJECT_SEQUENCE_ERROR = 9841, /**< Sequence number check failed. */

   BCMRET_INVALID_FILENAME = 9850,  /**< filename was not given for download */
   BCMRET_INVALID_IMAGE = 9851,     /**< bad image was given for download */
   BCMRET_INVALID_CONFIG_FILE = 9852,  /**< invalid config file was detected */
   BCMRET_CONFIG_PSI = 9853,         /**< old PSI/3.x config file was detected */
   BCMRET_IMAGE_FLASH_FAILED = 9854, /**< could not write the image to flash */
   BCMRET_RESOURCE_NOT_CONFIGURED = 9855, /**< requested resource is not configured/found */
   BCMRET_EE_UNPACK_ERROR = 9856,   /**< EE download unpack failure (associated with ChangeEEState) */
   BCMRET_EE_DUPLICATE = 9857,   /**< Duplicate Execution Environment (associated with ChangeEEState:install) */
   BCMRET_EE_UPDATE_DOWNGRADE_NOT_ALLOWED = 9858,/**< Invalid Execution Environment Update, downgrade not permitted
                                                    * (associated with ChangeEEState:update)
                                                    */
   BCMRET_EE_UPDATE_VERSION_NOT_SPECIFIED = 9859,/**< Invalid Execution Environment Update, version not specified
                                                    * (associated with ChangeEEState:update)
                                                    */
   BCMRET_EE_UPDATE_VERSION_EXISTED = 9860,/**< Invalid Execution Environment Update, version already exists
                                                    * (associated with ChangeEEState:update)
                                                    */
   BCMRET_PMD_CALIBRATION_FILE_SUCCESS = 9861, /**<Success. PMD Calibration file was uploaded.*/
   BCMRET_PMD_TEMP2APD_FILE_SUCCESS = 9862, /**<Success. PMD Temp2Apd file was uploaded.*/
   BCMRET_MANIFEST_PARSE_ERROR = 9863,   /**< Manifest parse error. */
   BCMRET_USERNAME_IN_USE = 9864,   /**< Username in use. */
   BCMRET_ADD_USER_ERROR = 9865,    /**< Add user error. */
   BCMRET_DELETE_USER_ERROR = 9866, /**< Delete user error. */
   BCMRET_USER_NOT_FOUND = 9867,    /**<User not found. */
   BCMRET_SET_BUSGATE_POLICY_ERROR = 9868,   /**<Set busgate policy error. */
   BCMRET_OPERATION_IN_PROCESS = 9869,    /**<Operation is still in process. */
   BCMRET_CONVERSION_ERROR = 9870, /**< Error during data conversion. */
   BCMRET_PARENTEE_EE_VERSION_MISMATCH = 9871, /**<EE version is not supported by platform version (BEEP Version)*/
   BCMRET_INVALID_URL_FORMAT = 9872,/**< Invalid URL Format
                                      * (associated with ChangeDUState)
                                      */
   BCMRET_UNKNOWN_ERROR = 9899,  /**< Can't figure out the real cause.  Use sparingly. */
} BcmRet;


#endif  /* __BCM_RETCODES_H__ */

