#!/bin/bash
#
# Run a series of 14 tests under KVM.  These are not particularly
# well-selected or well-tuned, but are the current set.  Run from the
# top level of the source tree.
#
# Edit the definitions below to set the locations of the various directories,
# as well as the test duration.
#
# Usage: kvm.sh [ options ]
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, you can access it online at
# http://www.gnu.org/licenses/gpl-2.0.html.
#
# Copyright (C) IBM Corporation, 2011
#
# Authors: Paul E. McKenney <paulmck@linux.vnet.ibm.com>

scriptname=$0
args="$*"

T=/tmp/kvm.sh.$$
trap 'rm -rf $T' 0
mkdir $T

dur=30
dryrun=""
KVM="`pwd`/tools/testing/selftests/rcutorture"; export KVM
PATH=${KVM}/bin:$PATH; export PATH
TORTURE_DEFCONFIG=defconfig
TORTURE_BOOT_IMAGE=""
TORTURE_INITRD="$KVM/initrd"; export TORTURE_INITRD
TORTURE_KMAKE_ARG=""
TORTURE_SUITE=rcu
resdir=""
configs=""
cpus=0
ds=`date +%Y.%m.%d-%H:%M:%S`

. functions.sh

usage () {
	echo "Usage: $scriptname optional arguments:"
	echo "       --bootargs kernel-boot-arguments"
	echo "       --bootimage relative-path-to-kernel-boot-image"
	echo "       --buildonly"
	echo "       --configs \"config-file list\""
	echo "       --cpus N"
	echo "       --datestamp string"
	echo "       --defconfig string"
	echo "       --dryrun sched|script"
	echo "       --duration minutes"
	echo "       --interactive"
	echo "       --kmake-arg kernel-make-arguments"
	echo "       --mac nn:nn:nn:nn:nn:nn"
	echo "       --no-initrd"
	echo "       --qemu-args qemu-system-..."
	echo "       --qemu-cmd qemu-system-..."
	echo "       --results absolute-pathname"
	echo "       --torture rcu"
	exit 1
}

while test $# -gt 0
do
	case "$1" in
	--bootargs)
		checkarg --bootargs "(list of kernel boot arguments)" "$#" "$2" '.*' '^--'
		TORTURE_BOOTARGS="$2"
		shift
		;;
	--bootimage)
		checkarg --bootimage "(relative path to kernel boot image)" "$#" "$2" '[a-zA-Z0-9][a-zA-Z0-9_]*' '^--'
		TORTURE_BOOT_IMAGE="$2"
		shift
		;;
	--buildonly)
		TORTURE_BUILDONLY=1
		;;
	--configs)
		checkarg --configs "(list of config files)" "$#" "$2" '^[^/]*$' '^--'
		configs="$2"
		shift
		;;
	--cpus)
		checkarg --cpus "(number)" "$#" "$2" '^[0-9]*$' '^--'
		cpus=$2
		shift
		;;
	--datestamp)
		checkarg --datestamp "(relative pathname)" "$#" "$2" '^[^/]*$' '^--'
		ds=$2
		shift
		;;
	--defconfig)
		checkarg --defconfig "defconfigtype" "$#" "$2" '^[^/][^/]*$' '^--'
		TORTURE_DEFCONFIG=$2
		shift
		;;
	--dryrun)
		checkarg --dryrun "sched|script" $# "$2" 'sched\|script' '^--'
		dryrun=$2
		shift
		;;
	--duration)
		checkarg --duration "(minutes)" $# "$2" '^[0-9]*$' '^error'
		dur=$2
		shift
		;;
	--interactive)
		TORTURE_QEMU_INTERACTIVE=1; export TORTURE_QEMU_INTERACTIVE
		;;
	--kmake-arg)
		checkarg --kmake-arg "(kernel make arguments)" $# "$2" '.*' '^error$'
		TORTURE_KMAKE_ARG="$2"
		shift
		;;
	--mac)
		checkarg --mac "(MAC address)" $# "$2" '^\([0-9a-fA-F]\{2\}:\)\{5\}[0-9a-fA-F]\{2\}$' error
		TORTURE_QEMU_MAC=$2
		shift
		;;
	--no-initrd)
		TORTURE_INITRD=""; export TORTURE_INITRD
		;;
	--qemu-args)
		checkarg --qemu-args "-qemu args" $# "$2" '^-' '^error'
		TORTURE_QEMU_ARG="$2"
		shift
		;;
	--qemu-cmd)
		checkarg --qemu-cmd "(qemu-system-...)" $# "$2" 'qemu-system-' '^--'
		TORTURE_QEMU_CMD="$2"
		shift
		;;
	--results)
		checkarg --results "(absolute pathname)" "$#" "$2" '^/' '^error'
		resdir=$2
		shift
		;;
	--torture)
		checkarg --torture "(suite name)" "$#" "$2" '^\(lock\|rcu\)$' '^--'
		TORTURE_SUITE=$2
		shift
		;;
	*)
		echo Unknown argument $1
		usage
		;;
	esac
	shift
done

CONFIGFRAG=${KVM}/configs/${TORTURE_SUITE}; export CONFIGFRAG

if test -z "$configs"
then
	configs="`cat $CONFIGFRAG/CFLIST`"
fi

if test -z "$resdir"
then
	resdir=$KVM/res
fi

# Create a file of test-name/#cpus pairs, sorted by decreasing #cpus.
touch $T/cfgcpu
for CF in $configs
do
	if test -f "$CONFIGFRAG/$CF"
	then
		cpu_count=`configNR_CPUS.sh $CONFIGFRAG/$CF`
		cpu_count=`configfrag_boot_cpus "$TORTURE_BOOTARGS" "$CONFIGFRAG/$CF" "$cpu_count"`
		echo $CF $cpu_count >> $T/cfgcpu
	else
		echo "The --configs file $CF does not exist, terminating."
		exit 1
	fi
done
sort -k2nr $T/cfgcpu > $T/cfgcpu.sort

# Use a greedy bin-packing algorithm, sorting the list accordingly.
awk < $T/cfgcpu.sort > $T/cfgcpu.pack -v ncpus=$cpus '
BEGIN {
	njobs = 0;
}

{
	# Read file of tests and corresponding required numbers of CPUs.
	cf[njobs] = $1;
	cpus[njobs] = $2;
	njobs++;
}

END {
	alldone = 0;
	batch = 0;
	nc = -1;

	# Each pass through the following loop creates on test batch
	# that can be executed concurrently given ncpus.  Note that a
	# given test that requires more than the available CPUs will run in
	# their own batch.  Such tests just have to make do with what
	# is available.
	while (nc != ncpus) {
		batch++;
		nc = ncpus;

		# Each pass through the following loop considers one
		# test for inclusion in the current batch.
		for (i = 0; i < njobs; i++) {
			if (done[i])
				continue; # Already part of a batch.
			if (nc >= cpus[i] || nc == ncpus) {

				# This test fits into the current batch.
				done[i] = batch;
				nc -= cpus[i];
				if (nc <= 0)
					break; # Too-big test in its own batch.
			}
		}
	}

	# Dump out the tests in batch order.
	for (b = 1; b <= batch; b++)
		for (i = 0; i < njobs; i++)
			if (done[i] == b)
				print cf[i], cpus[i];
}'

# Generate a script to execute the tests in appropriate batches.
cat << ___EOF___ > $T/script
CONFIGFRAG="$CONFIGFRAG"; export CONFIGFRAG
KVM="$KVM"; export KVM
PATH="$PATH"; export PATH
TORTURE_BOOT_IMAGE="$TORTURE_BOOT_IMAGE"; export TORTURE_BOOT_IMAGE
TORTURE_BUILDONLY="$TORTURE_BUILDONLY"; export TORTURE_BUILDONLY
TORTURE_DEFCONFIG="$TORTURE_DEFCONFIG"; export TORTURE_DEFCONFIG
TORTURE_INITRD="$TORTURE_INITRD"; export TORTURE_INITRD
TORTURE_KMAKE_ARG="$TORTURE_KMAKE_ARG"; export TORTURE_KMAKE_ARG
TORTURE_QEMU_CMD="$TORTURE_QEMU_CMD"; export TORTURE_QEMU_CMD
TORTURE_QEMU_INTERACTIVE="$TORTURE_QEMU_INTERACTIVE"; export TORTURE_QEMU_INTERACTIVE
TORTURE_QEMU_MAC="$TORTURE_QEMU_MAC"; export TORTURE_QEMU_MAC
TORTURE_SUITE="$TORTURE_SUITE"; export TORTURE_SUITE
if ! test -e $resdir
then
	mkdir -p "$resdir" || :
fi
mkdir $resdir/$ds
echo Results directory: $resdir/$ds
echo $scriptname $args
touch $resdir/$ds/log
echo $scriptname $args >> $resdir/$ds/log
echo ${TORTURE_SUITE} > $resdir/$ds/TORTURE_SUITE
pwd > $resdir/$ds/testid.txt
if test -d .git
then
	git status >> $resdir/$ds/testid.txt
	git rev-parse HEAD >> $resdir/$ds/testid.txt
	if ! git diff HEAD > $T/git-diff 2>&1
	then
		cp $T/git-diff $resdir/$ds
	fi
fi
___EOF___
awk < $T/cfgcpu.pack \
	-v CONFIGDIR="$CONFIGFRAG/" \
	-v KVM="$KVM" \
	-v ncpus=$cpus \
	-v rd=$resdir/$ds/ \
	-v dur=$dur \
	-v TORTURE_QEMU_ARG="$TORTURE_QEMU_ARG" \
	-v TORTURE_BOOTARGS="$TORTURE_BOOTARGS" \
'BEGIN {
	i = 0;
}

{
	cf[i] = $1;
	cpus[i] = $2;
	i++;
}

# Dump out the scripting required to run one test batch.
function dump(first, pastlast)
{
	print "echo ----Start batch: `date`";
	print "echo ----Start batch: `date` >> " rd "/log";
	jn=1
	for (j = first; j < pastlast; j++) {
		builddir=KVM "/b" jn
		cpusr[jn] = cpus[j];
		if (cfrep[cf[j]] == "") {
			cfr[jn] = cf[j];
			cfrep[cf[j]] = 1;
		} else {
			cfrep[cf[j]]++;
			cfr[jn] = cf[j] "." cfrep[cf[j]];
		}
		if (cpusr[jn] > ncpus && ncpus != 0)
			ovf = "-ovf";
		else
			ovf = "";
		print "echo ", cfr[jn], cpusr[jn] ovf ": Starting build. `date`";
		print "echo ", cfr[jn], cpusr[jn] ovf ": Starting build. `date` >> " rd "/log";
		print "rm -f " builddir ".*";
		print "touch " builddir ".wait";
		print "mkdir " builddir " > /dev/null 2>&1 || :";
		print "mkdir " rd cfr[jn] " || :";
		print "kvm-test-1-run.sh " CONFIGDIR cf[j], builddir, rd cfr[jn], dur " \"" TORTURE_QEMU_ARG "\" \"" TORTURE_BOOTARGS "\" > " rd cfr[jn]  "/kvm-test-1-run.sh.out 2>&1 &"
		print "echo ", cfr[jn], cpusr[jn] ovf ": Waiting for build to complete. `date`";
		print "echo ", cfr[jn], cpusr[jn] ovf ": Waiting for build to complete. `date` >> " rd "/log";
		print "while test -f " builddir ".wait"
		print "do"
		print "\tsleep 1"
		print "done"
		print "echo ", cfr[jn], cpusr[jn] ovf ": Build complete. `date`";
		print "echo ", cfr[jn], cpusr[jn] ovf ": Build complete. `date` >> " rd "/log";
		jn++;
	}
	for (j = 1; j < jn; j++) {
		builddir=KVM "/b" j
		print "rm -f " builddir ".ready"
		print "if test -z \"$TORTURE_BUILDONLY\""
		print "then"
		print "\techo ----", cfr[j], cpusr[j] ovf ": Starting kernel. `date`";
		print "\techo ----", cfr[j], cpusr[j] ovf ": Starting kernel. `date` >> " rd "/log";
		print "fi"
	}
	print "wait"
	print "if test -z \"$TORTURE_BUILDONLY\""
	print "then"
	print "\techo ---- All kernel runs complete. `date`";
	print "\techo ---- All kernel runs complete. `date` >> " rd "/log";
	print "fi"
	for (j = 1; j < jn; j++) {
		builddir=KVM "/b" j
		print "echo ----", cfr[j], cpusr[j] ovf ": Build/run results:";
		print "echo ----", cfr[j], cpusr[j] ovf ": Build/run results: >> " rd "/log";
		print "cat " rd cfr[j]  "/kvm-test-1-run.sh.out";
		print "cat " rd cfr[j]  "/kvm-test-1-run.sh.out >> " rd "/log";
	}
}

END {
	njobs = i;
	nc = ncpus;
	first = 0;

	# Each pass through the following loop considers one test.
	for (i = 0; i < njobs; i++) {
		if (ncpus == 0) {
			# Sequential test specified, each test its own batch.
			dump(i, i + 1);
			first = i;
		} else if (nc < cpus[i] && i != 0) {
			# Out of CPUs, dump out a batch.
			dump(first, i);
			first = i;
			nc = ncpus;
		}
		# Account for the CPUs needed by the current test.
		nc -= cpus[i];
	}
	# Dump the last batch.
	if (ncpus != 0)
		dump(first, i);
}' >> $T/script

cat << ___EOF___ >> $T/script
echo
echo
echo " --- `date` Test summary:"
echo Results directory: $resdir/$ds
kvm-recheck.sh $resdir/$ds
___EOF___

if test "$dryrun" = script
then
	cat $T/script
	exit 0
elif test "$dryrun" = sched
then
	# Extract the test run schedule from the script.
	egrep 'Start batch|Starting build\.' $T/script |
		grep -v ">>" |
		sed -e 's/:.*$//' -e 's/^echo //'
	exit 0
else
	# Not a dryrun, so run the script.
	sh $T/script
fi

# Tracing: trace_event=rcu:rcu_grace_period,rcu:rcu_future_grace_period,rcu:rcu_grace_period_init,rcu:rcu_nocb_wake,rcu:rcu_preempt_task,rcu:rcu_unlock_preempted_task,rcu:rcu_quiescent_state_report,rcu:rcu_fqs,rcu:rcu_callback,rcu:rcu_kfree_callback,rcu:rcu_batch_start,rcu:rcu_invoke_callback,rcu:rcu_invoke_kfree_callback,rcu:rcu_batch_end,rcu:rcu_torture_read,rcu:rcu_barrier
