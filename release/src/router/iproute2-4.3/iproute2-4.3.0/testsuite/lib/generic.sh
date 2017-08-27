
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

ts_tc()
{
	SCRIPT=$1; shift
	DESC=$1; shift

	$TC $@ 2> $STD_ERR > $STD_OUT

	if [ -s $STD_ERR ]; then
		ts_err "${SCRIPT}: ${DESC} failed:"
		ts_err "command: $TC $@"
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

ts_ip()
{
	SCRIPT=$1; shift
	DESC=$1; shift

	$IP $@ 2> $STD_ERR > $STD_OUT
        RET=$?

	if [ -s $STD_ERR ] || [ "$RET" != "0" ]; then
		ts_err "${SCRIPT}: ${DESC} failed:"
		ts_err "command: $IP $@"
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
    echo "dev-$(tr -dc "[:alpha:]" < /dev/urandom | head -c 6)"
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
	if cat "$STD_OUT" | wc -l | grep -q "$1"
	then
		pr_success
	else
		pr_failed
	fi
}
