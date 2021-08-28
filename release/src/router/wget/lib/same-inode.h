/* Determine whether two stat buffers are known to refer to the same file.

   Copyright (C) 2006, 2009-2021 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef SAME_INODE_H
# define SAME_INODE_H 1

# include <sys/types.h>

# if defined __VMS && __CRTL_VER < 80200000
#  define SAME_INODE(a, b)             \
    ((a).st_ino[0] == (b).st_ino[0]    \
     && (a).st_ino[1] == (b).st_ino[1] \
     && (a).st_ino[2] == (b).st_ino[2] \
     && (a).st_dev == (b).st_dev)
# elif defined _WIN32 && ! defined __CYGWIN__
   /* Native Windows.  */
#  if _GL_WINDOWS_STAT_INODES
    /* stat() and fstat() set st_dev and st_ino to 0 if information about
       the inode is not available.  */
#   define SAME_INODE(a, b) \
     (!((a).st_ino == 0 && (a).st_dev == 0) \
      && (a).st_ino == (b).st_ino && (a).st_dev == (b).st_dev)
#  else
    /* stat() and fstat() set st_ino to 0 always.  */
#   define SAME_INODE(a, b) 0
#  endif
# else
#  define SAME_INODE(a, b)    \
    ((a).st_ino == (b).st_ino \
     && (a).st_dev == (b).st_dev)
# endif

#endif
