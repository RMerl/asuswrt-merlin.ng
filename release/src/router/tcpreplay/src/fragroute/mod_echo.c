/*
 * mod_echo.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "config.h"
#include "argv.h"
#include "mod.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *
echo_open(int argc, char *argv[])
{
    char *p;

    if (argc < 2)
        return (NULL);

    if ((p = argv_copy(argv + 1)) == NULL)
        return (NULL);

    return (p);
}

int
echo_apply(void *d, _U_ struct pktq *pktq)
{
    char *p = (char *)d;

    printf("%s\n", p);
    return (0);
}

void *
echo_close(void *d)
{
    if (d != NULL)
        free(d);
    return (NULL);
}

struct mod mod_echo = {
        "echo",              /* name */
        "echo <string> ...", /* usage */
        echo_open,           /* open */
        echo_apply,          /* apply */
        echo_close           /* close */
};
