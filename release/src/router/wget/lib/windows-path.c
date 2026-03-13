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

#include <config.h>

/* Specification.  */
#include "windows-path.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *
extended_PATH (const char * const *dll_dirs)
{
  /* Create a string PATH=(dll_dirs[0];dll_dirs[1];...;)old_PATH  */
  const char *old_PATH = getenv ("PATH");
  if (old_PATH == NULL)
    old_PATH = "";
  size_t size;
  {
    size = 5;
    {
      size_t i;
      for (i = 0; dll_dirs[i] != NULL; i++)
        size += strlen (dll_dirs[i]) + 1;
    }
    size += strlen (old_PATH) + 1;
  }
  char *new_PATH = (char *) malloc (size);
  if (new_PATH == NULL)
    return NULL;
  {
    char *p = new_PATH;
    {
      memcpy (p, "PATH=", 5);
      p += 5;
    }
    {
      size_t i;
      for (i = 0; dll_dirs[i] != NULL; i++)
        {
          size_t l = strlen (dll_dirs[i]);
          memcpy (p, dll_dirs[i], l);
          p += l;
#if defined _WIN32 && !defined __CYGWIN__
          *p++ = ';';
#else
          *p++ = ':';
#endif
        }
    }
    {
      size_t l = strlen (old_PATH);
      memcpy (p, old_PATH, l);
      p += l;
      *p = '\0';
    }
  }
  return new_PATH;
}

char **
extended_environ (const char * const *dll_dirs)
{
  char *child_PATH = extended_PATH (dll_dirs);
  if (child_PATH == NULL)
    return NULL;

  /* Create a shallow copy of environ, adding the child_PATH and removing
     the original "PATH=..." string.
     This is a bit hairy, because we don't have a lock that would prevent
     other threads from making modifications in ENVP.  So, just make sure
     we don't crash; but if other threads are making modifications, part
     of the result may be wrong.  */
  char **envp;

#if defined _WIN32 && !defined __CYGWIN__
  envp = _environ;
#else
  envp = environ;
#endif

 retry:
  {
    /* Guess the size of the needed block of memory.
       The guess will be exact if other threads don't make modifications.  */
    size_t size = 0;
    {
      char **ep;
      char *p;
      for (ep = envp; (p = *ep) != NULL; ep++)
        if (strncmp (p, "PATH=", 5) != 0)
          size += 1;
    }
    char **new_environ = (char **) malloc ((1 + size + 1) * sizeof (char *));
    if (new_environ == NULL)
      {
        free (child_PATH);
        errno = ENOMEM;
        return NULL;
      }
    char **nep = new_environ;
    *nep++ = child_PATH;
    {
      size_t i = 0;
      char **ep;
      char *p;
      for (ep = envp; (p = *ep) != NULL; ep++)
        if (strncmp (p, "PATH=", 5) != 0)
          {
            if (i == size)
              {
                /* Other threads did modifications.  Restart.  */
                free (new_environ);
                goto retry;
              }
            *nep++ = p;
            i += 1;
          }
      if (i < size)
        {
          /* Other threads did modifications.  Restart.  */
          free (new_environ);
          goto retry;
        }
    }
    *nep = NULL;
    return new_environ;
  }
}
