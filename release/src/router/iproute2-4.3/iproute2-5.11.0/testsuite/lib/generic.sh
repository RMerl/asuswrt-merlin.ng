export DEST="127.0.0.1"

ts_log()
{
	echo "$@"
}

ts_err()
{
	ts_log "$@" | tee >> $ERRF
}

ts_cat()
{
	cat "$@"
}

ts_err_cat()
{
	ts_cat "$@" | tee >> $ERRF
}

ts_skip()
{
    exit 127
}

__ts_cmd()
{
	CMD=$1; shift
	SCRIPT=$1; shift
	DESC=$1; shift

	$CMD $@ 2> $STD_ERR > $STD_OUT

	if [ -s $STD_ERR ]; then
		ts_err "${SCRIPT}: ${DESC} failed:"
		ts_err "command: $CMD $@"
		ts_err "stderr output:"
		ts_err_cat $STD_ERR
		if [ -s $STD_OUT ]; then
			ts_err "stdout output:"
			ts_err_cat $STD_OUT
		fi
	elif [ -s $STD_OUT ]; then
		echo "${SCRIPT}: ${DESC} succeeded with output:"
		cat $STD_OUT
	else
		echo "${SCRIPT}: ${DESC} succeeded"
	fi
}

ts_tc()
{
	__ts_cmd "$TC" "$@"
}

ts_ip()
{
	__ts_cmd "$IP" "$@"
}

ts_ss()
{
	__ts_cmd "$SS" "$@"
}

ts_bridge()
{
	__ts_cmd "$BRIDGE" "$@"
}

ts_qdisc_available()
{
	HELPOUT=`$TC qdisc add $1 help 2>&1`
	if [ "`echo $HELPOUT | grep \"^Unknown qdisc\"`" ]; then
		return 0;
	else
		return 1;
	fi
}

rand_dev()
{
    rnd=""
    while [ ${#rnd} -ne 6 ]; do
        rnd="$(head -c 250 /dev/urandom | tr -dc '[:alpha:]' | head -c 6)"
    done
    echo "dev-$rnd"
}

pr_failed()
{
	echo " [FAILED]"
	ts_err "matching failed"
}

pr_success()
{
	echo " [SUCCESS]"
}

test_on()
{
	echo -n "test on: \"$1\""
	if cat "$STD_OUT" | grep -qE "$1"
	then
		pr_success
	else
		pr_failed
	fi
}

test_on_not()
{
	echo -n "test on: \"$1\""
	if cat "$STD_OUT" | grep -vqE "$1"
	then
		pr_success
	else
		pr_failed
	fi
}

test_lines_count()
{
	echo -n "test on lines count ($1): "
	if [ $(cat "$STD_OUT" | wc -l) -eq "$1" ]
	then
		pr_success
	else
		pr_failed
	fi
}
