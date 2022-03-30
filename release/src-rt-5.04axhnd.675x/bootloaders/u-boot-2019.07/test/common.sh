#!/bin/sh

OUTPUT_DIR=sandbox

fail() {
	echo "Test failed: $1"
	if [ -n ${tmp} ]; then
		rm ${tmp}
	fi
	exit 1
}

build_uboot() {
	echo "Build sandbox"
	OPTS="O=${OUTPUT_DIR} $1"
	NUM_CPUS=$(grep -c processor /proc/cpuinfo)
	echo ${OPTS}
	make ${OPTS} sandbox_config
	make ${OPTS} -s -j${NUM_CPUS}
}
