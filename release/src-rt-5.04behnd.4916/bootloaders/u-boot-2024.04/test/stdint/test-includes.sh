#!/bin/bash

# Test script to check uintptr_t and 64-bit types for warnings
#
# It builds a few boards with different toolchains. If there are no warnings
# then all is well.
#
# Usage:
#
# Make sure that your toolchains are correct at the bottom of this file
#
# Then:
#	./test/stdint/test-includes.sh

out=/tmp/test-includes.tmp

try_test() {
	local board=$1
	local arch=$2
	local soc=$3
	local gcc=$4
	local flags="$5"

	echo $@
	if ! which ${gcc} >/dev/null 2>&1; then
		echo "Not found: ${gcc}"
		return
	fi

	rm -rf ${out}
	mkdir -p ${out}
	touch ${out}/config.h
	mkdir -p ${out}/generated
	touch ${out}/generated/generic-asm-offsets.h
	mkdir -p ${out}/include/asm
	ln -s $(pwd)/arch/${arch}/include/asm/arch-${soc} \
			${out}/include/asm/arch

	cmd="${gcc} -c -D__KERNEL__ ${flags} \
		-fno-builtin -ffreestanding \
		-Iarch/${arch}/include \
		-Iinclude \
		-I${out} -I${out}/include \
		-include configs/${board}.h test/stdint/int-types.c \
		-o /dev/null"
	$cmd
}

try_both() {
	try_test $@
}

# board arch soc path-to-gcc
try_both sandbox sandbox - gcc
try_both coreboot x86 - x86_64-linux-gnu-gcc
try_both seaboard arm tegra20 /opt/linaro/gcc-linaro-arm-linux-gnueabihf-4.8-2013.08_linux/bin/arm-linux-gnueabihf-gcc
