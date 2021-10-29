/*
    Copyright 2000-2016 Broadcom Limited

<:label-BRCM:2016:DUAL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

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
