// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Murray Jensen <Murray.Jensen@csiro.au>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "error.h"

char *pname;

void
Warning(char *fmt, ...)
{
    va_list args;

    fprintf(stderr, "%s: WARNING: ", pname);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
}

void
Error(char *fmt, ...)
{
    va_list args;

    fprintf(stderr, "%s: ERROR: ", pname);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");

    exit(1);
}

void
Perror(char *fmt, ...)
{
    va_list args;
    int e = errno;
    char *p;

    fprintf(stderr, "%s: ERROR: ", pname);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    if ((p = strerror(e)) == NULL || *p == '\0')
	fprintf(stderr, ": Unknown Error (%d)\n", e);
    else
	fprintf(stderr, ": %s\n", p);

    exit(1);
}
