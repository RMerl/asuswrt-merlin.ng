#ifndef _CMDS_APPS_SSB_H
#define _CMDS_APPS_SSB_H
/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

/**

   \file cmd_apps_ssb.h

*/

#ifdef __cplusplus
extern "C"
{
#endif

    /* ========================================================================== */
    /*                           Function prototypes                              */
    /* ========================================================================== */
    extern OS_boolean_t cmds_ssb(CmdArgs_t *pArgs, int *err);
    extern int cmds_ssb_symlink_set(void);

#ifdef __cplusplus
}
#endif

#endif
