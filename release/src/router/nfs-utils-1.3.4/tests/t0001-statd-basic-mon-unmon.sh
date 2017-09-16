#!/bin/bash
#
# statd_basic_mon_unmon -- test basic mon/unmon functionality with statd
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

. ./test-lib.sh

# This test needs root privileges
check_root

start_statd
if [ $? -ne 0 ]; then
	echo "FAIL: problem starting statd"
	exit 1
fi

COOKIE=`echo $$ | md5sum | cut -d' ' -f1`
MON_NAME=`hostname`

nsm_client mon $MON_NAME $COOKIE
if [ $? -ne 0 ]; then
	echo "FAIL: mon failed"
	kill_statd
	exit 1
fi

statdb_dump | grep $MON_NAME | grep -q $COOKIE
if [ $? -ne 0 ]; then
	echo "FAIL: monitor DB doesn't seem to contain entry"
	kill_statd
	exit 1
fi

nsm_client unmon $MON_NAME
if [ $? -ne 0 ]; then
	echo "FAIL: unmon failed"
	kill_statd
	exit 1
fi

kill_statd

