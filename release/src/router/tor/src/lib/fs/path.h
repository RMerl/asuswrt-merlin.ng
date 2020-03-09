/* Copyright (c) 2003-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2019, The Tor Project, Inc. */
/* See LICENSE for licensing information */

/**
 * \file path.h
 *
 * \brief Header for path.c
 **/

#ifndef TOR_PATH_H
#define TOR_PATH_H

#include "lib/cc/compat_compiler.h"

#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

char *get_unquoted_path(const char *path);
char *expand_filename(const char *filename);
int path_is_relative(const char *filename);
void clean_fname_for_stat(char *name);
int get_parent_directory(char *fname);
char *make_path_absolute(char *fname);

#endif /* !defined(TOR_PATH_H) */
