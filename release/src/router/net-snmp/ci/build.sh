#!/usr/bin/env bash

scriptdir="$(dirname "$0")"
export NOAUTODEPS=1
export SNMP_VERBOSE=1
case "$(uname -a)" in
    MSYS*|MINGW*)
	pacman --noconfirm --remove mingw-w64-x86_64-gcc-ada
	pacman --noconfirm --remove mingw-w64-x86_64-gcc-fortran
	pacman --noconfirm --remove mingw-w64-x86_64-gcc-libgfortran
	pacman --noconfirm --remove mingw-w64-x86_64-gcc-objc
	pacman --noconfirm --sync --refresh
	pacman --noconfirm --sync --needed diffutils
	pacman --noconfirm --sync --needed make
	pacman --noconfirm --sync --needed perl-ExtUtils-MakeMaker
	pacman --noconfirm --sync --needed perl-Test-Harness
	;;
esac
case "$(uname -a)" in
    MSYS*x86_64*)
	pacman --noconfirm --sync --needed openssl-devel
	;;
    MINGW64*)
	pacman --noconfirm --sync --needed mingw-w64-x86_64-libmariadbclient
	pacman --noconfirm --sync --needed mingw-w64-x86_64-gcc
	pacman --noconfirm --sync --needed mingw-w64-x86_64-openssl
	export PATH="/mingw64/bin:$PATH"
	;;
esac
echo "compiler path: $(type -p gcc)"
"${scriptdir}"/net-snmp-configure V5-8-patches || exit $?
case "$MODE" in
    mini*)
	# Net-SNMP uses static dependencies, the Makefile.depend files have
	# been generated for MODE=regular, net-snmp-features.h includes
	# <net-snmp/library/features.h> in minimalist mode and that file is
	# generated dynamically and is not in Makefile.depend. Hence disable
	# parallel compilation for minimalist mode.
	nproc=1;;
    *)
	if type nproc >&/dev/null; then
	    nproc=$(nproc)
	else
	    nproc=1
	fi;;
esac
make -s -j${nproc}                       || exit $?
case "$MODE" in
    disable-set|mini*|read-only)
        exit 0;;
esac
[ -n "$APPVEYOR" ]			 && exit 0
"${scriptdir}"/net-snmp-run-tests        || exit $?
