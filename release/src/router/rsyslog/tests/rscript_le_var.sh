#!/bin/bash
# added 2014-01-17 by rgerhards
# This file is part of the rsyslog project, released under ASL 2.0
echo ===============================================================================
echo \[rscript_le.sh\]: testing rainerscript LE statement for two JSON variables
. $srcdir/diag.sh init
generate_conf
add_conf '
template(name="outfmt" type="list") {
	property(name="$!usr!msgnum")
	constant(value="\n")
}

set $!var1 = "42";
set $!var2 = "42";
set $!var3 = "43";
if $!var1 <= $!var2 and $!var1 <= $!var3 then {
        if $!var3 <= $!var1 then {
                # Failure
                stop
        } else {
                unset $!var1;
                unset $!var2;
                unset $!var3;
        }
} else {
        # Failure
        stop
}
set $.var1 = "42";
set $.var2 = "42";
set $.var3 = "43";
if $.var1 <= $.var2 and $.var1 <= $.var3 then {
        if $.var3 <= $.var1 then {
                # Failure
                stop
        } else {
                unset $.var1;
                unset $.var2;
                unset $.var3;
        }
} else {
        # Failure
        stop
}
set $/var1 = "42";
set $/var2 = "42";
set $/var3 = "43";
if $/var1 <= $/var2 and $/var1 <= $/var3 then {
        if $/var3 <= $/var1 then {
                # Failure
                stop
        } else {
                unset $/var1;
                unset $/var2;
                unset $/var3;
        }
} else {
        # Failure
        stop
}

if $msg contains "msgnum" then {
	set $!usr!msgnum = field($msg, 58, 2);
	action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
}
'
startup
injectmsg  0 1
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown 
seq_check  0 0
exit_test
