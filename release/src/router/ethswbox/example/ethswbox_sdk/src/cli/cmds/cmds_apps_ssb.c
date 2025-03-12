/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

/**
   \file cmds_lif.c
    Implements CLI commands for lif mdio

*/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "os_types.h"
#include "os_linux.h"

#include "cmds.h"
#include "cmds_apps_ssb.h"
#include "host_smdio_ssb.h"

/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */
static void cmds_ssb_help(void);

OS_boolean_t cmds_ssb(CmdArgs_t *pArgs, int *err)
{
    OS_boolean_t api_executed;
    int32_t ret;

    if (pArgs == NULL)
    {
        *err = OS_ERROR;
        return OS_TRUE;
    }

    ret = OS_SUCCESS;
    api_executed = OS_TRUE;

    /******************************************
     *  ssb CLI cmds                          *
     ******************************************/

    if ((strcmp(pArgs->name, "cmds-lif-help") == 0) || (strcmp(pArgs->name, "cmds-lif-?") == 0))
    {
        cmds_ssb_help();
    }

    /***************************************
     * cmds_ssb:                       		*
     *   - ssb-read-file              		*
     * ************************************/

    else if (strcmp(pArgs->name, "ssb_smdio_download") == 0)
    {
        char *file_to_read;
        unsigned char *pdata;

        if (pArgs->prmc < 1)
        {
            printf("Usage: ssb_smdio_download\n");
            printf("file_path: Path to File\n");

            goto goto_end_help;
        }
        file_to_read = pArgs->prmvs[0];

        ssb_load(file_to_read);
    }

    /***************
     *  No command  *
     ***************/
    else
    {
        api_executed = OS_FALSE;
    }

goto_end_help:
    *err = (int)ret;
    return api_executed;
}

int cmds_ssb_symlink_set(void)
{
    system("ln -sf ./ethswbox ssb_smdio_download");
    return OS_SUCCESS;
}

static void cmds_ssb_help(void)
{
    printf("+------------------------------------------------------------------+\n");
    printf("|                           HELP !                                 |\n");
    printf("|                                                                  |\n");
    printf("+------------------------------------------------------------------+\n");
    printf("| ssb_smdio_download     : Load FW                                 |\n");
    printf("\n");
}
