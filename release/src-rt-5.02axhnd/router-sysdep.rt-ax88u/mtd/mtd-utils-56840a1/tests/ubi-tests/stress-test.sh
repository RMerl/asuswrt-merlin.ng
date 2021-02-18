#!/bin/sh -euf

srcdir="$(readlink -ev -- ${0%/*})"
PATH="$srcdir:$srcdir/../..:$PATH"

fatal()
{
	echo "Error: $1" 1>&2
	exit 1
}

usage()
{
	cat 1>&2 <<EOF
Stress-test an UBI device. This test is basically built on top of
'runtests.sh' and runs it several times for different configurations.

The nandsim and mtdram drivers have to be compiled as kernel modules.

Usage:
  ${0##*/} run
EOF
}

cleanup_handler()
{
	local ret="$1"
	rmmod ubi >/dev/null 2>&1 ||:
	rmmod nandsim >/dev/null 2>&1 ||:
	rmmod mtdram >/dev/null 2>&1  ||:

	# Below is magic to exit with correct exit code
	if [ "$ret" != "0" ]; then
		trap false EXIT
	else
		trap true EXIT
	fi
}
trap 'cleanup_handler $?' EXIT
trap 'cleanup_handler 1' HUP PIPE INT QUIT TERM

# Find MTD device number by pattern in /proc/mtd
# Usage: find_mtd_device <pattern>
find_mtd_device()
{
	printf "%s" "$(grep "$1" /proc/mtd | sed -e "s/^mtd\([0-9]\+\):.*$/\1/")"
}

# Just print parameters of the 'run_test' funcion in a user-friendly form.
print_params()
{
	local module="$1";    shift
	local size="$1";      shift
	local peb_size="$1";  shift
	local page_size="$1"; shift
	local vid_offs="$1";  shift
	local fastmap="$1";   shift

	printf "%s" "$module: ${size}MiB, PEB size ${peb_size}KiB, "
	if [ "$module" = "nandsim" ]; then
		printf "%s" "page size ${page_size}KiB, VID offset $vid_offs, "
	fi
	printf "%s\n" "fastmap $fastmap" 
}

# Load mtdram with specified size and PEB size
# Usage: load_mtdram <flash size> <PEB size>
# 1. Flash size is specified in MiB 
# 2. PEB size is specified in KiB
load_mtdram()
{
	local size="$1";     shift
	local peb_size="$1"; shift

	size="$(($size * 1024))"
	modprobe mtdram total_size="$size" erase_size="$peb_size" ||
		echo "Error: cannot load $size MiB mtdram"
}

print_separator()
{
	echo "======================================================================"
}

# Run a test on nandsim or mtdram with certain geometry.
# Usage: run_test <nandsim|mtdram> <flash size> <PEB size> \
#                 <Page size> <VID hdr offs> <enable fastmap>
# 1. Simulator type (nandsim or mtdram)
# 2. Flash size is specified in MiB 
# 3. PEB size is specified in KiB
# 4. Page size is specified in bytes (mtdram ingores this)
# 5. VID header offset (mtdram ingores this)
# 6. Whether fast-map should be enabled (pass "enabled" or "disabled")
run_test()
{
	local module="$1";
	local size="$2";
	local peb_size="$3";
	local page_size="$4";
	local vid_offs="$5"
	local fastmap="$6";  
	local fm_supported fm_param mtdnum

	print_separator

	# Check if fastmap is supported by UBI
	if modinfo ubi | grep -q fm_auto; then
		fm_supported="yes"
	else
		fm_supported="no"
	fi

	if [ "$fastmap" = "enabled" ]; then
		fm_param=
	elif [ "$fm_supported" = "yes" ]; then
		fastmap="disabled"
		fm_param="fm_auto"
	else
		echo "Fastmap is not supported, skip"
		return 0
	fi

	if [ "$module" = "nandsim" ]; then
		print_params "$@"

		load_nandsim.sh "$size" "$peb_size" "$page_size" ||
			echo "Cannot load nandsim, test skipped"

		mtdnum="$(find_mtd_device "$nandsim_patt")"
	elif [ "$module" = "mtdram" ]; then
		print_params "$@"

		load_mtdram "$size" "$peb_size"

		mtdnum="$(find_mtd_device "$mtdram_patt")"
	else
		fatal "$module is not supported" ||
			echo "Cannot load nandsim, test skipped"
	fi

	modprobe ubi mtd="$mtdnum,$vid_offs" $fm_param
	runtests.sh /dev/ubi0 ||:

	sudo rmmod ubi
	sudo rmmod "$module"
}

if [ "$#" -lt 1 ] || [ "$1" != "run" ]; then
	usage
	exit 1
fi

# Make sure neither mtdram nor nandsim are used
nandsim_patt="NAND simulator"
mtdram_patt="mtdram test device"
if [ -e /proc/mtd ]; then
	! grep -q "$nandsim_patt" /proc/mtd ||
		fatal "the nandsim driver is already used"
	! grep -q "$mtdram_patt" /proc/mtd ||
		fatal "the mtdram driver is already used"
fi

rmmod ubi >/dev/null 2>&1 ||:

for module in "mtdram" "nandsim"; do
	for fm in "enabled" "disabled"; do
		for vid_factor in 1 0; do
			print_separator
			print_separator
			print_separator
			echo "Test on $module, fastmap $fm, VID header offset factor $vid_factor"
			print_separator
			print_separator

			pg_size="512"
			vid_offs="$(($pg_size * $vid_factor))"

			run_test "$module" "16"  "16" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "32"  "16" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "64"  "16" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "128" "16" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "256" "16" "$pg_size" "$vid_offs" "$fm"

			pg_size="2048"
			vid_offs="$(($pg_size * $vid_factor))"

			run_test "$module" "64"   "64" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "128"  "64" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "256"  "64" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "512"  "64" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "1024" "64" "$pg_size" "$vid_offs" "$fm"

			run_test "$module" "64"   "128" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "128"  "128" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "256"  "128" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "512"  "128" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "1024" "128" "$pg_size" "$vid_offs" "$fm"

			run_test "$module" "64"   "256" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "128"  "256" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "256"  "256" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "512"  "256" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "1024" "256" "$pg_size" "$vid_offs" "$fm"

			run_test "$module" "64"   "512" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "128"  "512" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "256"  "512" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "512"  "512" "$pg_size" "$vid_offs" "$fm"
			run_test "$module" "1024" "512" "$pg_size" "$vid_offs" "$fm"
		done
	done
done
