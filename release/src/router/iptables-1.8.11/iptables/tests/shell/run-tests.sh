#!/bin/bash

#configuration
TESTDIR="./$(dirname $0)/"
RETURNCODE_SEPARATOR="_"

usage() {
	cat <<EOF
Usage: $(basename $0) [-v|--verbose] [-H|--host] [-V|--valgrind]
		      [[-l|--legacy]|[-n|--nft]] [testscript ...]

-v | --verbose		Enable verbose mode (do not drop testscript output).
-H | --host		Run tests against installed binaries in \$PATH,
			not those built in this source tree.
-V | --valgrind		Enable leak checking via valgrind.
-l | --legacy		Test legacy variant only. Conflicts with --nft.
-n | --nft		Test nft variant only. Conflicts with --legacy.
testscript		Run only specific test(s). Implies --verbose.
EOF
}

msg_error() {
        echo "E: $1 ..." >&2
}

msg_warn() {
        echo "W: $1" >&2
}

msg_info() {
        echo "I: $1"
}

if [ "$(id -u)" != "0" ] ; then
        msg_error "this requires root!"
        exit 77
fi

if [ ! -d "$TESTDIR" ] ; then
        msg_error "missing testdir $TESTDIR"
        exit 99
fi

# support matching repeated pattern in SINGLE check below
shopt -s extglob

while [ -n "$1" ]; do
	case "$1" in
	-v|--verbose)
		VERBOSE=y
		shift
		;;
	-H|--host)
		HOST=y
		shift
		;;
	-l|--legacy)
		LEGACY_ONLY=y
		shift
		;;
	-n|--nft)
		NFT_ONLY=y
		shift
		;;
	-V|--valgrind)
		VALGRIND=y
		shift
		;;
	-h|--help)
		usage
		exit 0
		;;
	*${RETURNCODE_SEPARATOR}+([0-9]))
		SINGLE+=" $1"
		VERBOSE=y
		shift
		;;
	*)
		msg_error "unknown parameter '$1'"
		exit 99
		;;
	esac
done

if [ "$HOST" != "y" ]; then
	XTABLES_NFT_MULTI="$(dirname $0)/../../xtables-nft-multi"
	XTABLES_LEGACY_MULTI="$(dirname $0)/../../xtables-legacy-multi"

	export XTABLES_LIBDIR=${TESTDIR}/../../../extensions

	# maybe this is 'make distcheck' calling us from a build tree
	if [ ! -e "$XTABLES_NFT_MULTI" -a \
	     ! -e "$XTABLES_LEGACY_MULTI" -a \
	     -e "./iptables/xtables-nft-multi" -a \
	     -e "./iptables/xtables-legacy-multi" ]; then
		msg_warn "Running in separate build-tree, using binaries from $PWD/iptables"
		XTABLES_NFT_MULTI="$PWD/iptables/xtables-nft-multi"
		XTABLES_LEGACY_MULTI="$PWD/iptables/xtables-legacy-multi"
		export XTABLES_LIBDIR="$PWD/extensions"
	fi
else
	XTABLES_NFT_MULTI="xtables-nft-multi"
	XTABLES_LEGACY_MULTI="xtables-legacy-multi"
fi

printscript() { # (cmd, tmpd)
	cat <<EOF
#!/bin/bash

CMD="$1"

# note: valgrind man page warns about --log-file with --trace-children, the
# last child executed overwrites previous reports unless %p or %q is used.
# Since libtool wrapper calls exec but none of the iptables tools do, this is
# perfect for us as it effectively hides bash-related errors

valgrind --log-file=$2/valgrind.log --trace-children=yes \
	 --leak-check=full --show-leak-kinds=all \$CMD "\$@"
RC=\$?

# don't keep uninteresting logs
if grep -q 'no leaks are possible' $2/valgrind.log; then
	rm $2/valgrind.log
else
	mv $2/valgrind.log $2/valgrind_\$\$.log
fi

# drop logs for failing commands for now
[ \$RC -eq 0 ] || rm $2/valgrind_\$\$.log

exit \$RC
EOF
}

if [ "$VALGRIND" == "y" ]; then
	tmpd=$(mktemp -d)
	msg_info "writing valgrind logs to $tmpd"
	# let nobody write logs, too (././testcases/iptables/0008-unprivileged_0)
	chmod 777 $tmpd
	printscript "$XTABLES_NFT_MULTI" "$tmpd" >${tmpd}/xtables-nft-multi
	printscript "$XTABLES_LEGACY_MULTI" "$tmpd" >${tmpd}/xtables-legacy-multi
	trap "rm ${tmpd}/xtables-*-multi" EXIT
	chmod a+x ${tmpd}/xtables-nft-multi ${tmpd}/xtables-legacy-multi

	XTABLES_NFT_MULTI="${tmpd}/xtables-nft-multi"
	XTABLES_LEGACY_MULTI="${tmpd}/xtables-legacy-multi"

fi

find_tests() {
        if [ ! -z "$SINGLE" ] ; then
                echo $SINGLE
                return
        fi
        find ${TESTDIR} -executable -regex \
                .*${RETURNCODE_SEPARATOR}[0-9]+ | sort
}

ok=0
failed=0

do_test() {
	testfile="$1"
	xtables_multi="$2"

	rc_spec=`echo $(basename ${testfile}) | cut -d _ -f2-`

	[ -t 1 ] && msg_info "[EXECUTING]   $testfile"

	if [ "$VERBOSE" = "y" ]; then
		XT_MULTI=$xtables_multi unshare -n ${testfile}
		rc_got=$?
	else
		XT_MULTI=$xtables_multi unshare -n ${testfile} > /dev/null 2>&1
		rc_got=$?
		[ -t 1 ] && echo -en "\033[1A\033[K" # clean the [EXECUTING] foobar line
	fi

	if [ "$rc_got" == "$rc_spec" ] ; then
		msg_info "[OK]          $testfile"
		((ok++))
	else
		((failed++))
		msg_warn "[FAILED]      $testfile: expected $rc_spec but got $rc_got"
	fi
}

echo ""
if [ "$NFT_ONLY" != "y" ]; then
	for testfile in $(find_tests);do
		do_test "$testfile" "$XTABLES_LEGACY_MULTI"
	done
	msg_info "legacy results: [OK] $ok [FAILED] $failed [TOTAL] $((ok+failed))"

fi
legacy_ok=$ok
legacy_fail=$failed
ok=0
failed=0
if [ "$LEGACY_ONLY" != "y" ]; then
	for testfile in $(find_tests);do
		do_test "$testfile" "$XTABLES_NFT_MULTI"
	done
	msg_info "nft results: [OK] $ok [FAILED] $failed [TOTAL] $((ok+failed))"
fi

ok=$((legacy_ok+ok))
failed=$((legacy_fail+failed))

msg_info "combined results: [OK] $ok [FAILED] $failed [TOTAL] $((ok+failed))"

exit -$failed
