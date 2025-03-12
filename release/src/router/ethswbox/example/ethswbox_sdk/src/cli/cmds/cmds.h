#ifndef _CMDS_H
#define _CMDS_H
/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

/**

   \file cmds.h

*/

#include "ethswbox_version.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */

extern OS_boolean_t cmds(char *name, int prmc, char *prmv[], int *err);
    extern int cmds_symlink_set(void);

#define ARG_COUNT 50
    /* Global command arguments */
    typedef struct
    {
        char *name;
        int prmc;
        int prmvi[ARG_COUNT];
        char *prmvs[ARG_COUNT];
    } CmdArgs_t;

#ifdef __cplusplus
}
#endif

#endif /* _CMDS_H */
