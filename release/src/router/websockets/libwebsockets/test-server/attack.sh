#!/bin/bash
#
# attack the test server and try to make it fall over
#
SERVER=127.0.0.1
PORT=7681
LOG=/tmp/lwslog

A=`which libwebsockets-test-server`
INSTALLED=`dirname $A`

CPID=
LEN=0

function check {
	kill -0 $CPID
	if [ $? -ne 0 ] ; then
		echo "(killed it) *******"
		exit 1
	fi
	dd if=$LOG bs=1 skip=$LEN 2>/dev/null

	if [ "$1" = "default" ] ; then
		diff /tmp/lwscap $INSTALLED/../share/libwebsockets-test-server/test.html > /dev/null
		if [ $? -ne 0 ] ; then
			echo "FAIL: got something other than test.html back"
			exit 1
		fi
	fi

	if [ "$1" = "forbidden" ] ; then
		if [ -z "`grep '<h1>403</h1>' /tmp/lwscap`" ] ; then
			echo "FAIL: should have told forbidden (test server has no dirs)"
			exit 1
		fi
	fi

	if [ "$1" == "1" ] ; then
		a="`dd if=$LOG bs=1 skip=$LEN 2>/dev/null |grep URI\ Arg\ 1\: | tr -s ' ' | cut -d' ' -f5-`"
		if [ "$a" != "$2" ] ; then
			echo "Arg 1 '$a' not $2"
			exit 1
		fi
	fi

	if [ "$1" == "2" ] ; then
		a="`dd if=$LOG bs=1 skip=$LEN 2>/dev/null |grep URI\ Arg\ 2\: | tr -s ' ' | cut -d' ' -f5-`"
		if [ "$a" != "$2" ] ; then
			echo "Arg 2 '$a' not $2"
			exit 1
		fi
	fi
	if [ "$1" == "3" ] ; then
		a="`dd if=$LOG bs=1 skip=$LEN 2>/dev/null |grep URI\ Arg\ 3\: | tr -s ' ' | cut -d' ' -f5-`"
		if [ "$a" != "$2" ] ; then
			echo "Arg 3 '$a' not $2"
			exit 1
		fi
	fi

	if [ -z "$1" ] ; then
		LEN=`stat $LOG -c %s`
	fi
}


rm -rf $LOG
killall libwebsockets-test-server 2>/dev/null
libwebsockets-test-server -d31 2>> $LOG &
CPID=$!

while [ -z "`grep Listening $LOG`" ] ; do
	sleep 0.5s
done
check

echo
echo "---- /cgi-bin/settingsjs?UPDATE_SETTINGS=1&Root_Channels_1_Channel_name_http_post=%3F&Root_Channels_1_Channel_location_http_post=%3F"
rm -f /tmp/lwscap
echo -e "GET /cgi-bin/settingsjs?UPDATE_SETTINGS=1&Root_Channels_1_Channel_name_http_post=%3F&Root_Channels_1_Channel_location_http_post=%3F HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check 1 "UPDATE_SETTINGS=1"
check 2 "Root_Channels_1_Channel_name_http_post=?"
check 3 "Root_Channels_1_Channel_location_http_post=?"
check

echo
echo "---- ? processing (/cgi-bin/settings.js?key1=value1)"
rm -f /tmp/lwscap
echo -e "GET /cgi-bin/settings.js?key1=value1 HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check 1 "key1=value1"
check

echo
echo "---- ? processing (/test?key1%3d2=value1)"
rm -f /tmp/lwscap
echo -e "GET /test?key1%3d2=value1 HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check 1 "key1_2=value1"
check

echo
echo "---- ? processing (%2f%2e%2e%2f%2e./test.html?arg=1)"
rm -f /tmp/lwscap
echo -e "GET %2f%2e%2e%2f%2e./test.html?arg=1 HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check 1 "arg=1"
check

echo
echo "---- ? processing (%2f%2e%2e%2f%2e./test.html?arg=/../.)"
rm -f /tmp/lwscap
echo -e "GET %2f%2e%2e%2f%2e./test.html?arg=/../. HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check 1 "arg=/../."
check

echo
echo "---- spam enough crap to not be GET"
echo "not GET" | nc $SERVER $PORT
check

echo
echo "---- spam more than the name buffer of crap"
dd if=/dev/urandom bs=1 count=80 2>/dev/null | nc -i1s $SERVER $PORT
check

echo
echo "---- spam 10MB of crap"
dd if=/dev/urandom bs=1 count=655360 | nc -i1s $SERVER $PORT
check

echo
echo "---- malformed URI"
echo "GET nonsense................................................................................................................" \
	| nc -i1s $SERVER $PORT
check

echo
echo "---- missing URI"
echo -e "GET HTTP/1.1\x0d\x0a\x0d\x0a" | nc -i1s $SERVER $PORT >/tmp/lwscap
check

echo
echo "---- repeated method"
echo -e "GET blah HTTP/1.1\x0d\x0aGET blah HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT >/tmp/lwscap 
check

echo
echo "---- crazy header name part"
echo -e "GET blah HTTP/1.1\x0d\x0a................................................................................................................" \
	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 | nc -i1s $SERVER $PORT
check

echo
echo "---- excessive uri content"
echo -e "GET ................................................................................................................" \
	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 | nc -i1s $SERVER $PORT
check

echo
echo "---- good request but http payload coming too (should be ignored and test.html served)"
echo -e "GET /test.html HTTP/1.1\x0d\x0a\x0d\x0aILLEGAL-PAYLOAD........................................" \
	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
 	"......................................................................................................................." \
	 | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check default
check

echo
echo "---- directory attack 1 (/../../../../etc/passwd should be /etc/passswd)"
rm -f /tmp/lwscap
echo -e "GET /../../../../etc/passwd HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check forbidden
check

echo
echo "---- directory attack 2 (/../ should be /)"
rm -f /tmp/lwscap
echo -e "GET /../ HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check default
check

echo
echo "---- directory attack 3 (/./ should be /)"
rm -f /tmp/lwscap
echo -e "GET /./ HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check default
check

echo
echo "---- directory attack 4 (/blah/.. should be /)"
rm -f /tmp/lwscap
echo -e "GET /blah/.. HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check default
check

echo
echo "---- directory attack 5 (/blah/../ should be /)"
rm -f /tmp/lwscap
echo -e "GET /blah/../ HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check default
check

echo
echo "---- directory attack 6 (/blah/../. should be /)"
rm -f /tmp/lwscap
echo -e "GET /blah/../. HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check default
check

echo
echo "---- directory attack 7 (/%2e%2e%2f../../../etc/passwd should be /etc/passswd)"
rm -f /tmp/lwscap
echo -e "GET /%2e%2e%2f../../../etc/passwd HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check forbidden
check

echo
echo "---- directory attack 7 (%2f%2e%2e%2f%2e./.%2e/.%2e%2fetc/passwd should be /etc/passswd)"
rm -f /tmp/lwscap
echo -e "GET %2f%2e%2e%2f%2e./.%2e/.%2e%2fetc/passwd HTTP/1.1\x0d\x0a\x0d\x0a" | nc $SERVER $PORT | sed '1,/^\r$/d'> /tmp/lwscap
check forbidden
check

echo
echo "--- survived OK ---"
kill -2 $CPID

