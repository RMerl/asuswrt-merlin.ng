#!/bin/bash
# Run basic tests with all HW feature combinations
# Copyright 2017 Jussi Kivilinna <jussi.kivilinna@iki.fi>
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This file is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

# Use BINEXT to set executable extension
#  For example for Windows executables: BINEXT=.exe
if [ "x$BINEXT" != "x" ] && [ -e "tests/version$BINEXT" ]; then
	binext="$BINEXT"
else
	binext=""
fi

# Use BINPRE to set executable prefix
#  For example to run Windows executable with WINE: BINPRE="wine "
if [ "x$BINPRE" != "x" ]; then
	binpre="$BINPRE"
else
	binpre=""
fi

# Use NJOBS to define number of parallel tasks
if [ "x$NJOBS" != "x" ]; then
	njobs="$NJOBS"
else
	# default to cpu count
	ncpus=$(nproc --all)
	if [ "x@cpus" != "x" ]; then
		njobs=$ncpus
	else
		# could not get cpu count, use 4 parallel tasks instead
		njobs=4
	fi
fi

get_supported_hwfeatures() {
	$binpre "tests/version$binext" 2>&1 | \
		grep "hwflist" | \
		sed -e 's/hwflist://' -e 's/:/ /g' -e 's/\x0d/\x0a/g'
}

hwfs=($(get_supported_hwfeatures))
retcodes=()
optslist=()
echo "Total HW-feature combinations: $((1<<${#hwfs[@]}))"
for ((cbits=0; cbits < (1<<${#hwfs[@]}); cbits++)); do
  for ((mask=0; mask < ${#hwfs[@]}; mask++)); do
    match=$(((1<<mask) & cbits))
    if [ "x$match" != "x0" ]; then
      echo -n "--disable-hwf ${hwfs[$mask]} "
    fi
  done
  echo ""
done | sort | (
  # Run all combinations
  nbasic=0
  nwait=0
  failed=0
  output=""
  while read opts; do
    currn=$nbasic
    curr_jobs=($(jobs -p))
    while [ "${#curr_jobs[@]}" -ge "$njobs" ]; do
      # Wait for one job to complete
      wait ${retcodes[$nwait]}
      retval=$?
      if [ "x$retval" != "x0" ]; then
        output="$output\nERROR: HWF disable failed: [${optslist[$nwait]}]"
        failed=1
      else
        output="$output\nSUCCESS: HWF disable OK: [${optslist[$nwait]}]"
      fi
      nwait=$((nwait+1))
      curr_jobs=($(jobs -p))
      if [ $failed != 0 ]; then
        break
      fi
    done
    if [ $failed != 0 ]; then
      break
    fi
    nbasic=$((nbasic+1))
    echo "[$nbasic/$((1<<${#hwfs[@]}))] Basic test with '$opts'"
    optslist[$currn]="$opts"
    nice nice $binpre "tests/basic$binext" $opts & retcodes[$currn]=$!
  done

  # Fetch remaining return codes
  for ((; nwait < nbasic; nwait++)); do
    # Wait for one job to complete
    wait ${retcodes[$nwait]}
    retval=$?
    if [ "x$retval" != "x0" ]; then
      output="$output\nERROR: HWF disable failed: [${optslist[$nwait]}]"
      failed=1
    else
      output="$output\nSUCCESS: HWF disable OK: [${optslist[$nwait]}]"
    fi
  done

  echo -e "$output"
  exit $failed
)
