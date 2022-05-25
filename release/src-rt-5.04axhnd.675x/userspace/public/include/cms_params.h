/***********************************************************************
 *
 *  Copyright (c) 2006-2007  Broadcom Corporation
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

#ifndef __CMS_PARAMS_H__
#define __CMS_PARAMS_H__

/*!\file cms_params.h
 * \brief Header file containing customizable or board/hardware dependent
 *        parameters for the the CPE Management System (CMS).  Note that
 *        other customizable parameters are modified via make menuconfig.
 */


/** Config file version.
 *
 */
#define CMS_CONFIG_FILE_VERSION "3.0"


/** Number of spaces to indent each line in the config file.
 *
 */
#define CMS_CONFIG_FILE_INDENT 2


/* The shared memory attach addr and size have been moved to cms_mdm.h */


/** This is the Unix Domain Socket address for communications with smd used
 *  by the messaging library.
 *
 * Note two different addresses are defined, one for modem and one for DESKTOP_LINUX testing.
 *  It is highly unlikely that this needs to be changed.
 */
#if defined(DESKTOP_LINUX) && !defined(BUILD_DESKTOP_BEEP)
#define SMD_MESSAGE_ADDR  "/var/tmp/smd_messaging_server_addr"
#else
#define SMD_MESSAGE_ADDR  "/var/smd_messaging_server_addr"
#endif


/** This is the number of fully connected connections that can be queued
 *  up at the SMD message server socket.
 *
 *  It is highly unlikely that this needs to be changed.
 */
#define SMD_MESSAGE_BACKLOG  3


/** Special hack for the smd dynamic launch service, when it launches a server app, the
 *  server app will find its server fd at this number.
 *
 * It is highly unlikely that this needs to be changed.
 */
#define CMS_DYNAMIC_LAUNCH_SERVER_FD  3

#define CMS_DYNAMIC_LAUNCH_SERVER_FD_2 4


/** If running on desktop, server fd's under 1024 are offset by this amount
 *
 */
#define CMS_DESKTOP_SERVER_PORT_OFFSET  44400


/** This is the port ftpd listens on.
 */
#define FTPD_PORT       21


/** This is the port tftpd listens on.
 */
#define TFTPD_PORT      69


/** This is the port sshd listens on.
 */
#define SSHD_PORT       22


/** The amount of idle time, in seconds, before sshd exits.
 *
 * Make this relatively long because the user might be configuring something,
 * then gets confused and have to look up some manual.
 * If 0, then no timeout.
 */
#define SSHD_EXIT_ON_IDLE_TIMEOUT  600


/** This is the port telnetd listens on.
 */
#define TELNETD_PORT    23


/** The amount of idle time, in seconds, before telnetd exits.
 *
 * Make this relatively long because the user might be configuring something,
 * then gets confused and have to look up some manual.
 * If 0, then no timeout.
 */
#define TELNETD_EXIT_ON_IDLE_TIMEOUT  600


/** This is the port httpd listens on.
 */
#define HTTPD_PORT_SSL          443
#define HTTPD_PORT              80



/** The amount of idle time, in seconds, before httpd exits.
 *
 * Make this relatively long because the user might be configuring something,
 * then gets confused and have to look up some manual.
 */
#define HTTPD_EXIT_ON_IDLE_TIMEOUT  600


/** The amount of idle time, in seconds, before consoled exits.
 *
 * Make this relatively long because the user might be configuring something,
 * then gets confused and have to look up some manual.
 * If 0, then no timeout.
 */
#define CONSOLED_EXIT_ON_IDLE_TIMEOUT  0


/** This is the port snmpd listens on.
 */
#define SNMPD_PORT      161


/** This is the port tr64c listens on.
* LGD_TODO: Due to the time limit, it still have one DESKTOP_LINUX version TR64C, 
* in the future will add it.
* // TODO: TR64 is no longer supported, so this define should be removed.
*/
#define TR64C_HTTP_CONN_PORT     49431


/** This is the port tr69c listens on for connection requests from the ACS.
 * 
 */
#define TR69C_CONN_REQ_PORT      30005


/** This is the port tr69c_2 listens on for connection requests from the ACS.
 * 
 */
#define TR69C_2_CONN_REQ_PORT      30006


/** This is the path part of the URL for tr69c connection requests from the ACS.
 * 
 */
#define TR69C_CONN_REQ_PATH      "/"


/** TCP backlog for the connection request socket.
 */
#define TR69C_CONN_REQ_BACKLOG      32



/** Maximum number of Layer 2 bridges supported.
 * 
 * If this value is changed, be sure to also modify the default value in
 * the data model.
 */
#define MAX_LAYER2_BRIDGES                16


/* MAX_MDM_INSTANCE_DEPTH defined in os_defs.h */


/** Maximum length of a parameter name in the Data Model that we can support.
 *  If the data model has a greater actual param name length than what is defined here,
 *  cmsMdm_init() will fail.
 */
#define MAX_MDM_PARAM_NAME_LENGTH   55


/** Maximum MDM instance number.  Instance numbers are signed 32 bit integers.
 *
 */
#define MAX_MDM_INSTANCE_NUMBER    MAX_SINT32



/** the invalid maximum MDM OID.  due to MDM_MAX_OID == last valid max OID, so +1 here.
 *
 */
#define INVALIDE_MDM_MAX_OID (MDM_MAX_OID + 1)



/** In Data Model Detect mode, our current data model selection is stored
 *  in the Persistent Scratch Pad (PSP) under this key.
 *  4 bytes are written into the PSP, but only the first byte is used.
 *  The other 3 bytes are reserved for future expansion.
 *  0 : root=InternetGatewayDevice (for Legacy98 and Hybrid)
 *  1 : root=Device (for Pure181)
 */
#define CMS_DATA_MODEL_PSP_KEY "CmsDMSelect"

#define CMS_DATA_MODEL_PSP_VALUE_LEN   4



/** DNS Probing parameters for the dproxy. They probe every
 * 30 seconds. Timeout is 3 seconds and only retry 2 more times. */
#define DNS_PROBE_INTERVAL 30
#define DNS_PROBE_TIMEOUT 3 
#define DNS_PROBE_MAX_TRY 3


/** Path to additional CMS Entity Info directory
 *
 * This path must be kept in sync with userspace/public/libs/cms_util/Makefile
 */
#define CMS_EID_DIR  "/etc/cms_entity_info.d"


/** Max length of a line in a file in the CMS_EID_DIR
 */
#define MAX_EID_LINE_LENGTH      512


/** Path to the cgroups cpu subsystem hierarchy.
 *
 * This path must be kept in sync with userspace/public/apps/cgroupctl/Makefile
 * and userspace/public/apps/cgroupctl/scripts/cgroup.conf
 */
#define CGROUP_CPUTREEDIR  "/cgroups/cputree"


/** Path to a magic cookie file to indicate CMS smd is shutting down.
 */
#define SMD_SHUTDOWN_IN_PROGRESS "/tmp/smd_shutdown_in_progress"


/** Max length of a full path name.
 *
 * Linux and various filesystems is usually 4096, but we limit it to save
 * some memory.
 */
#define CMS_MAX_FULLPATH_LENGTH     1024


/** Max length of a file name.
 *
 * Linux and various filesystems is around 1024, but we limit it to save
 * some memory.
 */
#define CMS_MAX_FILENAME_LENGTH     257


/** Max length of CMS password (actual max password length is this value -1
 *  to account for the null terminating char).
 */
#define CMS_MAX_PASSWORD_LENGTH     64

#endif  /* __CMS_PARAMS_H__ */
