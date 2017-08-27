/* Determine whether two stat buffers refer to the same file.

   Copyright (C) 2006, 2009-2011 Free Software Foundation, Inc.

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

#ifndef SAME_INODE_H
# define SAME_INODE_H 1

# define SAME_INODE(Stat_buf_1, Stat_buf_2) \
   ((Stat_buf_1).st_ino == (Stat_buf_2).st_ino \
    && (Stat_buf_1).st_dev == (Stat_buf_2).st_dev)

#endif
