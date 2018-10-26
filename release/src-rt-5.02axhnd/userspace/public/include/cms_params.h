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

#include "cms.h"


/** Config file version.
 *
 */
#define CMS_CONFIG_FILE_VERSION "3.0"


/** Number of spaces to indent each line in the config file.
 *
 */
#define CMS_CONFIG_FILE_INDENT 2


/** Address where the shared memory region is attached.
 *
 * Every process must attach to the shared memory at the same address
 * because the data structures inside the shared memory region contain
 * pointers to other areas in the shared memory.
 */
#define MDM_SHM_ATTACH_ADDR  0x58800000


/** Base amount of shared memory to allocate.
 *
 */
#define MDM_SHM_BASE_SIZE         (256 * 1024)


/** Amount of shared memory to allocate if WLAN feature is enabled.
 *
 */
#if defined(CONFIG_BCM94908) || defined(BCM_XRDP) || defined(CONFIG_BCM947189)
#define MDM_SHM_WLAN_EXTRA        (328 * 1024)
#else
#define MDM_SHM_WLAN_EXTRA        (128 * 1024)
#endif


/** Amount of shared memory to allocate if VOIP feature is enabled.
 *
 */
#define MDM_SHM_VOIP_EXTRA        (128 * 1024)


/** Amount of shared memory to allocate if GPON feature is enabled.
 *
 */
#define MDM_SHM_GPON_EXTRA        (128 * 1024)

/** Amount of shared memory to allocate if extra large DSL data is enabled.
 *
 */
#define MDM_SHM_DSL_BIG_DATA_EXTRA  (256 * 1024)

/** Amount of shared memory to allocate if HOMEPLUG is enabled.
 *
 */
#define MDM_SHM_TR069_HOMEPLUG_EXTRA  (256 * 1024)

/** The "key" to use when requesting a semaphore from the Linux OS.
 *
 * This is used to implement low level MDM transation locks.
 * The only time this will need to be modified is when other code
 * is using the same key.
 */
#define MDM_LOCK_SEMAPHORE_KEY 0x5ed7


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
*/
#define TR64C_HTTP_CONN_PORT     49431


/** This is the port tr69c listens on for connection requests from the ACS.
 * 
 */
#define TR69C_CONN_REQ_PORT      30005


/** This is the path part of the URL for tr69c connection requests from the ACS.
 * 
 */
#define TR69C_CONN_REQ_PATH      "/"


/** The amount of idle time, in seconds, before tr69c exits.
 *
 * This value does not need to be very large because the ACS is usually running
 * a script so it will do all the actions it needs back-to-back and then be
 * completely done.  So if we see no more requests from the ACS for 30 seconds,
 * that probably means the ACS is completely done.  However, if response time
 * is very important, and you do not want the tr69c client to exit, then you
 * can set this to a very large value (e.g. 2160356, which is one year).
 */
#define TR69C_EXIT_ON_IDLE_TIMEOUT       30 


/** Maximum number of Layer 2 bridges supported.
 * 
 * If this value is changed, be sure to also modify the default value in
 * the data model.
 */
#define MAX_LAYER2_BRIDGES                16


/** Maximum depth of objects in the Data Model that we can support.
 *  If the data model has a greater actual depth than what is defined here,
 *  cmsMdm_init() will fail.
 */
#define MAX_MDM_INSTANCE_DEPTH    6


/** Maximum length of a parameter name in the Data Model that we can support.
 *  If the data model has a greater actual param name length than what is defined here,
 *  cmsMdm_init() will fail.
 */
#define MAX_MDM_PARAM_NAME_LENGTH   55


/** Maximum MDM instance number.  Instance numbers are signed 32 bit integers.
 *
 */
#define MAX_MDM_INSTANCE_NUMBER    MAX_SINT32


/** In Data Model Detect mode, our current data model selection is stored
 *  in the Persistent Scratch Pad (PSP) under this key.
 *  4 bytes are written into the PSP, but only the first byte is used.
 *  The other 3 bytes are reserved for future expansion.
 *  0 : root=InternetGatewayDevice (for Legacy98 and Hybrid)
 *  1 : root=Device (for Pure181)
 */
#define CMS_DATA_MODEL_PSP_KEY "CmsDMSelect"

#define CMS_DATA_MODEL_PSP_VALUE_LEN   4



/** DNS Probing parameters for both dnsprobe and dproxy. They probe every
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

#endif  /* __CMS_PARAMS_H__ */
