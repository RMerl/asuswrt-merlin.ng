/* I/O block size definitions for coreutils
   Copyright (C) 1989, 1991-2011 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Include this file _after_ system headers if possible.  */

/* sys/stat.h will already have been included by system.h. */
#include "stat-size.h"


/* As of Mar 2009, 32KiB is determined to be the minimium
   blksize to best minimize system call overhead.
   This can be tested with this script with the results
   shown for a 1.7GHz pentium-m with 2GB of 400MHz DDR2 RAM:

   for i in $(seq 0 10); do
     size=$((8*1024**3)) #ensure this is big enough
     bs=$((1024*2**$i))
     printf "%7s=" $bs
     dd bs=$bs if=/dev/zero of=/dev/null count=$(($size/$bs)) 2>&1 |
     sed -n 's/.* \([0-9.]* [GM]B\/s\)/\1/p'
   done

      1024=734 MB/s
      2048=1.3 GB/s
      4096=2.4 GB/s
      8192=3.5 GB/s
     16384=3.9 GB/s
     32768=5.2 GB/s
     65536=5.3 GB/s
    131072=5.5 GB/s
    262144=5.7 GB/s
    524288=5.7 GB/s
   1048576=5.8 GB/s

   Note that this is to minimize system call overhead.
   Other values may be appropriate to minimize file system
   or disk overhead.  For example on my current GNU/Linux system
   the readahead setting is 128KiB which was read using:

   file="."
   device=$(df -P --local "$file" | tail -n1 | cut -d' ' -f1)
   echo $(( $(blockdev --getra $device) * 512 ))

   However there isn't a portable way to get the above.
   In the future we could use the above method if available
   and default to io_blksize() if not.
 */
enum { IO_BUFSIZE = 32*1024 };
static inline size_t
io_blksize (struct stat sb)
{
  return MAX (IO_BUFSIZE, ST_BLKSIZE (sb));
}
