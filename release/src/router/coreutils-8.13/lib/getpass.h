/* -*- buffer-read-only: t -*- vi: set ro: */
/* DO NOT EDIT! GENERATED AUTOMATICALLY! */
/* getpass.h -- Read a password of arbitrary length from /dev/tty or stdin.
   Copyright (C) 2004, 2009-2011 Free Software Foundation, Inc.
   Contributed by Simon Josefsson <jas@extundo.com>, 2004.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef GETPASS_H
# define GETPASS_H

/* Get getpass declaration, if available.  */
# include <unistd.h>

# if defined HAVE_DECL_GETPASS && !HAVE_DECL_GETPASS
/* Read a password of arbitrary length from /dev/tty or stdin.  */
char *getpass (const char *prompt);

# endif

#endif /* GETPASS_H */
