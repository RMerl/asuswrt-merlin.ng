#!/bin/sh

# Customize these parameters according to your environment
ANDROID_NDK="${HOME}/bin/android-ndk-r6b"

# Check for parameters
if [ ! -d "${ANDROID_NDK}" ]; then
 echo "Android NDK not found in ${ANDROID_NDK}, please edit $0 to fix it."
 exit 1
fi

if [ ! -e "${ANDROID_NDK}/build/tools/make-standalone-toolchain.sh" ]; then
 echo "Your Android NDK is not compatible (make-standalone-toolchain.sh not found)."
 echo "Android NDK r6b is known to work."
 exit 1
fi

# Extract the Android toolchain from NDK
ANDROID_PLATFORM="android-3"
ROOT="`pwd`"
OUT="${ROOT}/out"
${ANDROID_NDK}/build/tools/make-standalone-toolchain.sh \
 --ndk-dir="${ANDROID_NDK}" \
 --platform="${ANDROID_PLATFORM}" \
 --install-dir="${OUT}/toolchain" \
 || exit 1
# Remove resolv.h because it is quite unusable as is
rm ${OUT}/toolchain/sysroot/usr/include/resolv.h

# Create configure script
cd ${ROOT}
autoconf || exit 1

# Create config.h and Makefile
cd ${OUT}
${ROOT}/configure \
 --host \
 --disable-openssl \
 --disable-unix \
 CC="${OUT}/toolchain/bin/arm-linux-androideabi-gcc" \
 || exit 1

# Replace misconfigured values in config.h and enable PTY functions
mv config.h config.old
cat config.old \
 | sed 's/CRDLY_SHIFT.*/CRDLY_SHIFT 9/' \
 | sed 's/TABDLY_SHIFT.*/TABDLY_SHIFT 11/' \
 | sed 's/CSIZE_SHIFT.*/CSIZE_SHIFT 4/' \
 | sed 's/\/\* #undef HAVE_OPENPTY \*\//#define HAVE_OPENPTY 1/' \
 | sed 's/\/\* #undef HAVE_GRANTPT \*\//#define HAVE_GRANTPT 1/' \
 > config.h

# Enable openpty() in Makefile
mv Makefile Makefile.old
cat Makefile.old | sed 's/error.c/error.c openpty.c/' > Makefile

# Provide openpty.c
cat >openpty.c <<EOF
/* Copyright (C) 1998, 1999, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Zack Weinberg <zack@rabi.phys.columbia.edu>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#define _PATH_DEVPTMX "/dev/ptmx"

int openpty (int *amaster, int *aslave, char *name, struct termios *termp,
		struct winsize *winp)
{
	char buf[PATH_MAX];
	int master, slave;

	master = open(_PATH_DEVPTMX, O_RDWR);
	if (master == -1)
		return -1;

	if (grantpt(master))
		goto fail;

	if (unlockpt(master))
		goto fail;

	if (ptsname_r(master, buf, sizeof buf))
		goto fail;

	slave = open(buf, O_RDWR | O_NOCTTY);
	if (slave == -1)
		goto fail;

	/* XXX Should we ignore errors here?  */
	if (termp)
		tcsetattr(slave, TCSAFLUSH, termp);
	if (winp)
		ioctl(slave, TIOCSWINSZ, winp);

	*amaster = master;
	*aslave = slave;
	if (name != NULL)
		strcpy(name, buf);

	return 0;

fail:
	close(master);
	return -1;
}
EOF

# Compile
make socat || exit 1

# Done
echo "Build finished, socat has been generated successfuly in out/socat"

