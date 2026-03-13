/* Auxiliary functions for the creation of subprocesses on Windows.
   Copyright (C) 2024 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2024.

   This file is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   This file is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef _WINDOWS_PATH_H
#define _WINDOWS_PATH_H

/* This file uses _GL_ATTRIBUTE_MALLOC.  */
#if !_GL_CONFIG_H_INCLUDED
 #error "Please include config.h first."
#endif

#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


/* Returns an augmented "PATH=..." string for the environment of a child process.
   dll_dirs is a NULL-terminated list of directories that contain DLLs needed to
   execute the program, or NULL if none is needed.
   Returns a freshly allocated string.  In case of memory allocation failure,
   NULL is returned, with errno set.  */
extern char * extended_PATH (const char * const *dll_dirs)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;

/* Returns an augmented environment for a child process.
   dll_dirs is a NULL-terminated list of directories that contain DLLs needed to
   execute the program, or NULL if none is needed.
   Returns a freshly allocated string array, with a freshly allocated first
   string.  In case of memory allocation failure, NULL is returned, with errno
   set.  */
extern char ** extended_environ (const char * const *dll_dirs)
  _GL_ATTRIBUTE_MALLOC _GL_ATTRIBUTE_DEALLOC_FREE;


#ifdef __cplusplus
}
#endif

#endif /* _WINDOWS_PATH_H */
