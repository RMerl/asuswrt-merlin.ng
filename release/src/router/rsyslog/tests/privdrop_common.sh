#!/bin/bash
# added 2016-04-15 by Thomas D., released under ASL 2.0
# Several tests need another user/group to test impersonation.
# This script can be sourced to prevent duplicated code.

# To support <bash-4.2 which don't support "declare -g" we declare
# the array outside of the function
declare -A TESTBENCH_TESTUSER

rsyslog_testbench_setup_testuser() {
	local has_testuser=
	local testusername=
	local testgroupname=

	if [ -z "${EUID}" ]; then
		# Should never happen
		echo "FATAL ERROR: \$EUID not set!"
		exit 1
	fi

	if [ ${EUID} -eq 0 ]; then
		# Only root is able to become a different user

		local testusers=("rsyslog" "syslog" "daemon")

		if [ -n "${RSYSLOG_TESTUSER}" ]; then
			# User has specified an username/uid we should use in testbench
			testusers=("${RSYSLOG_TESTUSER}" ${testusers[@]})
		fi

		local testuser=
		for testuser in "${testusers[@]}"; do
			testusername=$(id --user --name ${testuser} 2>/dev/null)
			if [ -z "${testusername}" ]; then
				echo "'id' did not find user \"${testuser}\" ... skipping, trying next user!"
				continue
			fi

			testgroupname=$(id --group --name ${testuser} 2>/dev/null)
			if [ -z "${testgroupname}" ]; then
				echo "'id' did not find a primary group for \"${testuser}\" ... skipping, trying next user!"
				continue
			fi

			has_testuser="${testuser}"
			break
		done
	fi

	if [ -z "${has_testuser}" ]; then
		testgroupname=$(id --group --name ${EUID} 2>/dev/null)
		if [ -z "${testgroupname}" ]; then
			echo "Skipping ... please set RSYSLOG_TESTUSER or make sure the user running the testbench has a primary group!"
			exit_test
			exit 0
		else
			has_testuser="${EUID}"
		fi
	fi

	_rsyslog_testbench_declare_testuser ${has_testuser}
}

_rsyslog_testbench_declare_testuser() {
	local testuser=$1

	local testusername=$(id --user --name ${testuser} 2>/dev/null)
	if [ -z "${testusername}" ]; then
		# Should never happen
		echo "FATAL ERROR: Could not get username for user \"${testuser}\"!"
		exit 1
	fi

	local testuid=$(id --user ${testuser} 2>/dev/null)
	if [ -z "${testuid}" ]; then
		# Should never happen
		echo "FATAL ERROR: Could not get uid for user \"${testuser}\"!"
		exit 1
	fi

	local testgroupname=$(id --group --name ${testuser} 2>/dev/null)
	if [ -z "${testgroupname}" ]; then
		# Should never happen
		echo "FATAL ERROR: Could not get uid of user \"${testuser}\"!"
		exit 1
	fi

	local testgid=$(id --group ${testuser} 2>/dev/null)
	if [ -z "${testgid}" ]; then
		# Should never happen
		echo "FATAL ERROR: Could not get primary gid of user \"${testuser}\"!"
		exit 1
	fi

	echo "Will use user \"${testusername}\" (#${testuid}) and group \"${testgroupname}\" (#${testgid})"

	TESTBENCH_TESTUSER[username]=${testusername}
	TESTBENCH_TESTUSER[uid]=${testuid}
	TESTBENCH_TESTUSER[groupname]=${testgroupname}
	TESTBENCH_TESTUSER[gid]=${testgid}
}
