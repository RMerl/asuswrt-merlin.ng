/* Save the list of files in a directory in a string.

   Copyright (C) 1997, 1999, 2001, 2003, 2005, 2009-2011 Free Software
   Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by David MacKenzie <djm@gnu.ai.mit.edu>. */

#ifndef _GL_SAVEDIR_H
#define _GL_SAVEDIR_H

#include <dirent.h>
char *streamsavedir (DIR *dirp);
char *savedir (char const *dir);
char *fdsavedir (int fd); /* deprecated */

#endif
