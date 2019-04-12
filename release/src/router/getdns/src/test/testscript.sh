#!/bin/sh
# Copyright (c) 2013, Verisign, Inc., NLNet Labs
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# * Neither the names of the copyright holders nor the
#   names of its contributors may be used to endorse or promote products
#   derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL Verisign, Inc. BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# run $1 > $2 and exit on failure to execute
runit () {
	echo -n "Test $1:"
	./$1 > $2
	if test $? -ne 0; then
		echo " failed (execution failed)"
		exit 1
	fi
}

# check output files $1 and $2, exit on failure
diffit () {
	if diff $1 $2; then
		echo " OK"
	else
		echo " failed (differences above)"
		exit 1
	fi
}

# check output of program $1, known_good must be in $1.good
checkoutput () {
	runit $1 output
	diffit output $1.good
}

# filter out TTL and bindata stuff from $1 to $2
filterout () {
	sed -e '/"ttl"/d' -e '/"ipv4_address"/d' -e '/"ipv6_address"/d' -e '/"rdata_raw"/d' -e '/<bindata/d' -e '/"serial"/d' <$1 >$2
}

# like checkoutput but removes addresses and TTLs and bindata
# this makes the test almost useless, but it tests runtime lookup
# and the structure of the answer format, against the live internet.
checkpacket () {
	runit $1 output
	cp $1.good output.good
	filterout output output2
	filterout output.good output2.good
	diffit output2 output2.good
}

echo "./check_getdns"
./check_getdns
if test $? -ne 0; then
	echo " failed (unit test execution failed)"
	exit 1
fi
checkoutput tests_dict
checkoutput tests_list 

# the packets are too different to compare for people
#checkpacket tests_stub_async
#checkpacket tests_stub_sync

runit tests_stub_async output
echo " exitcode-OK"
runit tests_stub_sync output
echo " exitcode-OK"
runit tests_dnssec output
echo " exitcode-OK"

rm -f output output.good output2 output2.good
exit 0
