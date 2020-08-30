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

#ifndef __CMS_PARAMS_MODSW_H__
#define __CMS_PARAMS_MODSW_H__

/*!\file cms_params_modsw.h
 * \brief Header file containing customizable parameters related to
 *               Modular Software.
 */



/** Path to CMS data storage dir.
 */
#define CMS_DATA_STORAGE_DIR   "/local"

/** Path to preinstalled packages 
 */
#define MODSW_PREINSTALLED_DIR "/opt"
#define MODSW_PREINSTALLED_EE_DIR         MODSW_PREINSTALLED_DIR"/ee"
#define MODSW_PREINSTALLED_EE_BEE_DIR     MODSW_PREINSTALLED_EE_DIR"/BEE"
#define MODSW_PREINSTALLED_EE_BEE_DU_DIR  MODSW_PREINSTALLED_EE_BEE_DIR"/du"
#define MODSW_PREINSTALLED_SA_DIR         MODSW_PREINSTALLED_DIR"/sa"


#define MODSW_MAX_PREINSTALLED_EE      5
#define MODSW_MAX_PREINSTALLED_DU      32
#define MODSW_MAX_PREINSTALLED_SA      32




/** Path to Modular Software dir.
 */
#define CMS_MODSW_DIR   CMS_DATA_STORAGE_DIR"/modsw"


/** Path to tmp modsw dir.  Files are initially downloaded to tmp before
 *  they are put in their correct location.
 */
#define CMS_MODSW_TMP_DIR   CMS_DATA_STORAGE_DIR"/modsw/tmp"


/** Common path to both OSGI DU, BEEP DU and Linux DU.
 *
 */
#define CMS_MODSW_DU_DIR    CMS_DATA_STORAGE_DIR"/modsw/tr157du"


/** Path to TR157 Deployment Unit Directory for OSGI bundles.
 *  OSGI bundle deployment units are put in this directory based on their
 *  name, e.g. /local/modsw/tr157du/osgibundle/bundle_name...
 * So each DU has its own directory.
 */
#define CMS_MODSW_OSGIEE_DU_DIR    CMS_DATA_STORAGE_DIR"/modsw/tr157du/osgibundles"


/** Path to TR157 Deployment Unit Dir for Linux packages.
 * Linux deployment units are put in this directory based on their name,
 * e.g. /local/modsw/tr157du/linux/du_name...
 * So each DU has its own directory.
 */
#define CMS_MODSW_LINUXEE_DU_DIR    CMS_DATA_STORAGE_DIR"/modsw/tr157du/linuxpkgs"

/** Path to TR157 Deployment Unit Dir for OpenWRT opkg packages.
 * Linux deployment units are put in this directory based on their name,
 * e.g. /local/modsw/tr157du/openwrt/du_name...
 * So each DU has its own directory.
 */
#define CMS_MODSW_OPENWRTEE_DU_DIR    CMS_DATA_STORAGE_DIR"/modsw/tr157du/openwrt-opkgs"



/** Path to TR157 Primary Firmware Patch dir.  Patches to the primary firmware
 *  are put in this directory based on their name,
 *  e.g. /local/modsw/tr157pfp/pfpname/...
 *  So each Primary Firmware Patch has its own directory.
 */
#define CMS_MODSW_LINUXPFP_DIR   CMS_DATA_STORAGE_DIR"/modsw/tr157pfp"

#define CMS_MODSW_TMP_DU_DIR   CMS_DATA_STORAGE_DIR"/modsw/tmp/du"

#define CMS_MODSW_TMP_EE_DIR   CMS_DATA_STORAGE_DIR"/modsw/tmp/ee"


/** First 32 bytes of a Primary Firmware Patch consists of this string.
 *  This string must be 32 bytes, as specified by CMS_MODSW_LINUX_HEADER1_LEN
 *  below.
 */
#define CMS_MODSW_LINUXPFP_HEADER "BCMTR157:Linux:PrimFirmwrePatch;"


/** First 32 bytes of a Linux Deployment Unit (DU) consists of this string.
 *  This string must be 32 bytes, as specified by CMS_MODSW_LINUX_HEADER1_LEN
 *  below.
 */
#define CMS_MODSW_LINUXEE_DU_HEADER "BCMTR157:LinuxEE:DeploymentUnit;"


/** Length of first Header section.
 */
#define CMS_MODSW_LINUX_HEADER1_LEN    32


/** Length of second Header section, which is the name of the patch or DU.
 *  If the name is shorter than the length, the remaining bytes must
 *  contain 0.
 */
#define CMS_MODSW_LINUX_HEADER2_LEN    128


/** Text header in the outer tarball */
#define CMS_MODSW_OUTER_HEADER_NAME    "outer-header.txt"


/** Below are the keywords allowed in version 1 of the outer-header */
#define BCMPKG_VERSION                1

#define BCMPKG_KW_VERSION             "version"
#define BCMPKG_TK_VERSION             1001

#define BCMPKG_KW_INNERFILENAME       "innerFilename"
#define BCMPKG_TK_INNERFILENAME       1002
/* matches max length defined for TR181 Execution Unit name */
#define BCMPKG_INNERFILENAME_MAX_LEN  32

#define BCMPKG_KW_CHECKSUMALGO        "checksumAlgo"
#define BCMPKG_TK_CHECKSUMALGO        1003

#define BCMPKG_KW_CHECKSUM            "checksum"
#define BCMPKG_TK_CHECKSUM            1004

#define BCMPKG_SHA1_CHECKSUM_LEN      40


/** Text header in the inner tarball */
#define CMS_MODSW_INNER_HEADER_NAME    "inner-header.txt"

/** Below are the keywords allowed in version 1 of the inner-header */
#define BCMPKG_KW_PKGVERSION          "pkgVersion"
#define BCMPKG_TK_PKGVERSION          2001
#define TR157_VERSION_STR_LEN         32

#define BCMPKG_KW_PKGDESCRIPTION      "pkgDescription"
#define BCMPKG_TK_PKGDESCRIPTION      2002
#define TR157_DESCRIPTION_STR_LEN     256

#define BCMPKG_KW_INNERVERSION        "innerVersion"
#define BCMPKG_TK_INNERVERSION        2003

#define BCMPKG_KW_INNERDESCRIPTION    "innerDescription"
#define BCMPKG_TK_INNERDESCRIPTION    2004

#define TR157_NAME_STR_LEN            64
#define KEY_LEN                       128         /* max key length is 128 bytes for sha512 */
/** Debug log for storing various ModSw operations.  Use of this file is
 *  deprecated.  Was OSGID_LOG_FILE.
 */
#define CMS_MODSW_DEBUG_LOG           CMS_DATA_STORAGE_DIR"/modsw/tmp/DuLog"
#define CMS_MODSW_DEBUG_EU_LOG        CMS_DATA_STORAGE_DIR"/modsw/tmp/EuLog"
#define CMS_MODSW_DEBUG_EE_LOG        CMS_DATA_STORAGE_DIR"/modsw/tmp/EeLog"

/*
 * Defines specific to the Linux Execution Environment.
 */

#define LINUXEE_NAME     "Linux"
#define LINUXEE_TYPE     "Linux"
#define LINUXEE_VENDOR   "Linux"
#define LINUXEE_VERSION  "3.4"

/*
 * Defines specific to the Openwrt Execution Environment.
 */

#define OPENWRTEE_NAME     "Openwrt"
#define OPENWRTEE_TYPE     "Chaos Calmer"
#define OPENWRTEE_VENDOR   "OpenWRT Project"
#define OPENWRTEE_VERSION  "15.05"

/*
 * Defines specific to the Docker Execution Environment.
 */

#define DOCKEREE_NAME     "Docker"
#define DOCKEREE_TYPE     "Docker Daemon"
#define DOCKEREE_VENDOR   "Docker"
#define DOCKEREE_VERSION  "18.03.1-ce"

/* 
 * Folder where docker images are being stored.  
 * Note docker does not follow the same storage path as the other EE
 */
#define CMS_MODSW_DOCKEREE_DU_DIR  CMS_DATA_STORAGE_DIR"/docker_fs/storage"


/* supported digest methods */

#define DIGEST_SHA256                     "sha256"
#define DIGEST_SHA256_HEAD                "sha256:"

#define DIGEST_SHA512                     "sha512"
#define DIGEST_SHA512_HEAD                "sha512:"

#define DIGEST_HMAC_SHA1                  "hmac-sha1"
#define DIGEST_HMAC_SHA1_HEAD             "hmac-sha1:"

#define DIGEST_HMAC_SHA256                "hmac-sha256"
#define DIGEST_HMAC_SHA256_HEAD           "hmac-sha256:"

#define DIGEST_HMAC_SHA512                "hmac-sha512"
#define DIGEST_HMAC_SHA512_HEAD           "hmac-sha512:"
#define DIGEST_HEAD_MAX                   16

#define DIGEST_SHA1_KEYLEN                40          
#define DIGEST_SHA256_KEYLEN              64          /* hmac-sha256 and sha256 key length are same */
#define DIGEST_SHA512_KEYLEN              128         /* hmac-sha512 and sha512 key length are same */
#define BEEP_KEY_LEN_MAX                  DIGEST_SHA512_KEYLEN+DIGEST_HEAD_MAX

#endif  /* __CMS_PARAMS_MODSW_H__ */
