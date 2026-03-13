/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2001-2007 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software available under the same license
 *   as the "OpenBSD" operating system, distributed at
 *   http://www.openbsd.org/.
 *
 * ----------------------------------------------------------------------- */

/*
 * remap.h
 *
 * Prototypes for regular-expression based filename remapping.
 */

#ifndef TFTPD_REMAP_H
#define TFTPD_REMAP_H

/* Opaque type */
struct rule;

#ifdef WITH_REGEX

/* This is called when we encounter a substitution like \i.  The
   macro character is passed as the first argument; the output buffer,
   if any, is passed as the second argument.  The function should return
   the number of characters output, or -1 on failure. */
typedef int (*match_pattern_callback) (char, char *);

/* Read a rule file */
struct rule *parserulefile(FILE *);

/* Destroy a rule file data structure */
void freerules(struct rule *);

/* Execute a rule set on a string; returns a malloc'd new string. */
char *rewrite_string(const char *, const struct rule *, char,
                     match_pattern_callback, const char **);

#endif                          /* WITH_REGEX */
#endif                          /* TFTPD_REMAP_H */
