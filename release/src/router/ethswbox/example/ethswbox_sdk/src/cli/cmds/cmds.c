/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

/**
   \file cmds.c
*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "os_types.h"
#include "os_linux.h"

#include "cmds.h"
#include "cmds_fapi.h"
#include "cmds_apps_ssb.h"

/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */
static CmdArgs_t *cmds_update_args(int prmc, char *name, char *prmv[]);
static unsigned long int string2val(char *s);
static void cmds_help(void);

/* command arguments */
static CmdArgs_t cmds_Args;

static CmdArgs_t *cmds_update_args(int prmc, char *name, char *prmv[])
{
    int i;
    memset(&cmds_Args, 0, sizeof(cmds_Args));

    for (i = 0; i < ARG_COUNT; i++)
    {
        if (prmc > i)
        {
            cmds_Args.prmvi[i] = (int)string2val(prmv[i]);
            cmds_Args.prmvs[i] = prmv[i];
        }
        else
            break;
    }

    cmds_Args.name = name;
    cmds_Args.prmc = prmc;

    return &cmds_Args;
}

/*
 * string2val:
 * hexadecimal -> 0x[0-9 AaBbCcDdEeFf]
 * decimal -> [0-9]
 * illegal: return 0;
 */
static unsigned long int string2val(char *s)
{
    int base = 10;
    char *p = s;

    /* hexadecimal value start with 0x */
    if (s[0] == '0' && s[1] == 'x')
    {
        base = 16;
        p += 2;

        for (; *p; p++)
        {
            if ((*p < '0' || *p > 'f') || (*p > '9' && *p < 'A') || (*p > 'F' && *p < 'a'))
            {
                printf("string2val: Illegal hexadecimal value!\n");
                return 0;
            }
        }
    }
    else
    {
        /* otherwise decimal value */
        base = 10;
        for (; *p; p++)
        {
            if ((*p < '0' || *p > '9'))
            {
                return 0;
            }
        }
    }

    /* return the value according to base 10 or 16 */
    return strtoul(s, &p, base);
}

OS_boolean_t cmds(char *name, int prmc, char *prmv[],
                  int *err)
{
    OS_boolean_t api_executed;
    int ret;
    CmdArgs_t *pCmdArgs;

    pCmdArgs = cmds_update_args(prmc, name, prmv);

    if (cmds_fapi(pCmdArgs, &ret) == OS_TRUE)
    {
        return ret;
    }
    if (cmds_ssb(pCmdArgs, &ret) == OS_TRUE)
    {
        return ret;
    }

    ret = OS_SUCCESS;
    api_executed = OS_TRUE;

    /*****************************************
     *  CLI CMD                               *
     *****************************************/
    if ((strcmp(name, "cmds-help") == 0) || (strcmp(name, "cmds-?") == 0))
    {
        cmds_help();
    }

    /***************
     *  No command  *
     ***************/
    else
    {
        api_executed = OS_FALSE;
    }

    *err = ret;
    return api_executed;
}

int cmds_symlink_set(void)
{
    int ret = 0;

    /* Create cmds symbolic links */
    ret |= cmds_fapi_symlink_set();
    ret |= cmds_ssb_symlink_set();
    return ret;
}

static void cmds_help(void)
{
    printf("+------------------------------------------------------------------+\n");
    printf("|                           HELP !                                 |\n");
    printf("|                 Ethernet Software Toolbox (%s)                |\n", ETHSWBOX_VERSION_STR);
    printf("|                                                                  |\n");
    printf("|                           CMDS                                    |\n");
    printf("+------------------------------------------------------------------+\n");
    printf("|                                                                  |\n");
    printf("+------------------------------------------------------------------+\n");
    printf("\n");
}