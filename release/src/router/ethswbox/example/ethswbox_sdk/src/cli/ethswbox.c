/******************************************************************************

    Copyright 2022 Maxlinear

    SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0-only)

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.
******************************************************************************/

/**

   \file ethswbox.c

*/
#include "os_types.h"
#include "os_linux.h"

#include "ethswbox_version.h"

#include "cmds.h"

int ethswbox_main(int argc, char *argv[])
{
    int ret = OS_ERROR;
    int prmc;
    char **prmv;

    char *name = argv[0];

    if (strchr(name, '/') != NULL)
        do
        {
            /* move behind next / */
            name = strchr(name, '/') + 1;

            if (name == NULL)
                return ret;
        } while (strlen(name) > 0 && strchr(name, '/') != NULL);
    if (name == NULL || strcmp(name, "ethswbox") == 0)
    {
        printf("Ethernet SW Toolbox version %s.0 ... Updating symbolic links!\n", ETHSWBOX_VERSION_STR);
        ret |= cmds_symlink_set();

        return (int)ret;
    }

    /* remove the name parameter from the parameter count */
    prmc = argc;
    prmc--;
    prmv = &argv[1];
    /*
       name: command name
        dev: device number
       prmc: parameter counter without device number
       prmv: parameters
    */
#if 1
    {
        int i;
        printf("cmd: %s %d: ", name, prmc);
        for (i = 0; i < prmc; i++)
        {
            printf("%s ", prmv[i]);
        }
        printf("\n");
    }
#endif

    /* invoke CLI command parser */
    if (cmds(name, prmc, prmv, &ret) == OS_TRUE)
    {
        goto end;
    }

end:

    return ret;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    if (argv != NULL)
        ret = ethswbox_main(argc, argv);
    return ret;
}
