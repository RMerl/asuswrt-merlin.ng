/*
 * argv.c
 *
 * Copyright (c) 2001 Dug Song <dugsong@monkey.org>
 *
 * $Id$
 */

#include "argv.h"
#include "config.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
argv_create(char *p, int argc, char *argv[])
{
    int i;

    for (i = 0; i < argc - 1; i++) {
        while (*p != '\0' && isspace((int)*p))
            *p++ = '\0';

        if (*p == '\0')
            break;
        argv[i] = p;

        while (*p != '\0' && !isspace((int)*p))
            p++;
    }
    p[0] = '\0';
    argv[i] = NULL;

    return (i);
}

/* XXX - from tcpdump util.c. */
char *
argv_copy(char *argv[])
{
    char **p, *buf, *src, *dst;
    int len = 0;

    p = argv;
    if (*p == 0)
        return (NULL);

    while (*p)
        len += strlen(*p++) + 1;

    if ((buf = (char *)malloc(len)) == NULL)
        return (NULL);

    p = argv;
    dst = buf;

    while ((src = *p++) != NULL) {
        while ((*dst++ = *src++) != '\0')
            ;
        dst[-1] = ' ';
    }
    dst[-1] = '\0';

    return (buf);
}
