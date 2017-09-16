#!/bin/bash
#
# test-lib.sh -- library of functions for nfs-utils tests
#
# Copyright (C) 2010  Red Hat, Jeff Layton <jlayton@redhat.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 0211-1301 USA
#

# make sure $srcdir is set and sanity check it
srcdir=${srcdir-.}
if [ ! -d ${srcdir} ]; then
	echo "***ERROR***: bad installation -- \$srcdir=${srcdir}"
	exit 1
fi

export PATH=$PATH:${srcdir}:${srcdir}/nsm_client

# Some tests require root privileges. Check for them and skip the test (exit 77)
# if the caller doesn't have them.
check_root() {
	if [ $EUID -ne 0 ]; then
		echo "*** Skipping this test as it requires root privs ***"
		exit 77
	fi
}

# is lockd registered as a service?
lockd_registered() {
	rpcinfo -p | grep -q nlockmgr
	return $?
}

# start up statd
start_statd() {
	rpcinfo -u 127.0.0.1 status 1 &> /dev/null
	if [ $? -eq 0 ]; then
		echo "***ERROR***: statd is already running and should "
		echo "             be down when starting this test"
		return 1
	fi
	$srcdir/../utils/statd/statd --no-notify
}

# shut down statd
kill_statd() {
	kill `cat /var/run/rpc.statd.pid`
}
