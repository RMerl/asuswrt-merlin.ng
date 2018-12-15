#!/bin/bash
# added 2015-11-24 by portant
# This file is part of the rsyslog project, released under ASL 2.0
echo =============================================================================================
echo \[json_var_case.sh\]: test for referencing local and global variables properly in comparisons
. $srcdir/diag.sh init
generate_conf
add_conf '
module(load="../plugins/mmjsonparse/.libs/mmjsonparse")
module(load="../plugins/imtcp/.libs/imtcp")
input(type="imtcp" port="'$TCPFLOOD_PORT'")

# we must make sure the template contains references to the variables
template(name="outfmt" type="string" string="json prop:%$!val%  local prop:%$.val%  global prop:%$/val%\n")

action(type="mmjsonparse")

set $.val = "123";
set $.rval = "123";
if ($.val == $.rval) then {
	set $.val = "def";
}
set $/val = "123";
set $/rval = "123";
if ($/val == $/rval) then {
	set $/val = "ghi";
}

action(type="omfile" file=`echo $RSYSLOG_OUT_LOG` template="outfmt")
'
startup
tcpflood -m 1 -M "\"<167>Nov  6 12:34:56 172.0.0.1 test: @cee: { \\\"val\\\": \\\"abc\\\" }\""
echo doing shutdown
shutdown_when_empty
echo wait on shutdown
wait_shutdown
content_check  "json prop:abc  local prop:def  global prop:ghi"
exit_test
