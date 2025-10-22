/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2001 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software available under the same license
 *   as the "OpenBSD" operating system, distributed at
 *   http://www.openbsd.org/.
 *
 * ----------------------------------------------------------------------- */

/*
 * tftpd.h
 *
 * Prototypes for various functions that are part of the tftpd server.
 */

#ifndef TFTPD_TFTPD_H
#define TFTPD_TFTPD_H

void set_signal(int, void (*)(int), int);
void *tfmalloc(size_t);
char *tfstrdup(const char *);

extern int verbosity;

#endif
