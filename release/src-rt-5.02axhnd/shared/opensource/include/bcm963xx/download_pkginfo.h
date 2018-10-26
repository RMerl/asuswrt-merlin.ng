/*
    Copyright 2000-2016 Broadcom Limited

<:label-BRCM:2016:DUAL/GPL:standard

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
*/

#ifndef _DOWNLOAD_PKGINFO_H_
#define _DOWNLOAD_PKGINFO_H_


#define PACKAGE_BEEP_FILE_FREFIX    "pkg_beep_"
#define PACKAGE_BEEP_FILE_SUFFIX    ".tar.gz"
#define PACKAGE_BEEP_MANI_SUFFIX    ".manifest"
#define PACKAGE_EE_FILE_SUFFIX      ".tar.gz"
#define PACKAGE_MANIFESTFILE        "manifest"
#define PACKAGE_BEEP_BUILD_DIR      "pkg_beep_build"
#define APP_DIR_PREFIX              "/app_"            /* app AKA  EU in tr157 */
#define LIB_DIR_PREFIX              "/lib"


#define EE_NAME_FIELD_TAG           "eeName"
#define EE_VERSION_FIELD_TAG        "eeVersion"
#define EE_HostMsgType_FIELD_TAG    "eeToHostMsgType"


#define PACKAGE_FIELD_TAG           "pkgName"
#define VENDOR_FIELD_TAG            "vendor"
#define DESCRIPTION_FIELD_TAG       "description"
#define VERSION_FIELD_TAG           "version"
#define PKG_DEPENDENCY_FIELD_TAG    "pkg-dependency"
#define APP_FIELD_APP_LIST          "app-list"
#define APP_FIELD_NAME              "name"
#define APP_FIELD_MEDIA_TYPE        "app-mediaType"
#define APP_FIELD_APP_DIGEST        "app-digest"
#define APP_FIELD_MANI_DIGEST       "mani-digest"
#define APP_FIELD_TARBALL_MNGR_EXE  "app-tarballMngrExecutable"

#define MEDIA_TYPE_EXECUTABLE       "executable"
#define MEDIA_TYPE_TARBALL          "tarball"
#define MEDIA_TYPE_EE_EXECUTABLE    "ee_executable"
#define MEDIA_TYPE_EE_TARBALL       "ee_tarball"

typedef enum
{
   ALG_HMAC_SHA1        = 0,   
   ALG_SHA256           = 1,
   ALG_HMAC_SHA256      = 2, 
   ALG_SHA512           = 3,
   ALG_HMAC_SHA512      = 4,
} algType;


#endif // _DOWNLOAD_PKGINFO_H_
