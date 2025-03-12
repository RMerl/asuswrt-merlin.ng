#ifndef __ETHSWBOX_VERSION_H__
#define __ETHSWBOX_VERSION_H__
/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/



/** ETHSWBOX version, major number */
#define ETHSWBOX_VER_MAJOR 1
/** ETHSWBOX version, minor number */
#define ETHSWBOX_VER_MINOR 1
/** ETHSWBOX version, build number */
#define ETHSWBOX_VER_STEP 0

#define ETHSWBOX_VERSION_REG(a,b,c,d) (((a)<<24)|((b)<<16)|(c<<8)|(d))
#define ETHSWBOX_VERSION ETHSWBOX_VERSION_REG(VIT_VER_MAJOR, \
                                    ETHSWBOX_VER_MINOR, \
                                    ETHSWBOX_VER_STEP)

#ifndef _MKSTR_1
#define _MKSTR_1(x)    #x
#define _MKSTR(x)      _MKSTR_1(x)
#endif

/** driver version as string */
#define ETHSWBOX_VERSION_STR    _MKSTR(ETHSWBOX_VER_MAJOR) "." \
                                _MKSTR(ETHSWBOX_VER_MINOR) "." \
                                _MKSTR(ETHSWBOX_VER_STEP)

#endif /* __ETHSWBOX_VERSION_H__ */
