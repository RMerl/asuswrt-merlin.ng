#!/bin/bash
#
# Copyright (c) 2017, Sinodun Internet Technologies Ltd, NLnet Labs
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
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


# Stubby helper file to set DNS servers on macOS.
# Note - this script doesn't detect or handle network events, simply changes the 
# current resolvers
# Must run as root.

usage () {
    echo
    echo "Update the system DNS resolvers so that Stubby is used for all DNS"
    echo "queries on macOS. (Stubby must already be running)"
    echo "This must be run as root."
    echo
    echo "Usage: $0 options"
    echo
    echo "Supported options:"
    echo "  -r Reset DNS resolvers to the default ones (e.g. from DHCP)"
    echo "  -l List the current DNS settings for all interfaces"
    echo "  -h Show this help."
}

RESET=0
LIST=0
SERVERS="127.0.0.1 ::1"
OS_X=$(uname -a | grep -c 'Darwin')

while getopts ":rlh" opt; do
    case $opt in
        r  ) RESET=1 ;;
        l  ) LIST=1 ;;
        h  ) usage
             exit 1 ;;
        \? ) usage
             exit 1 ;;
    esac
done


if [[ $OS_X -eq 0 ]]; then
    echo "Sorry - This script only works on macOS and you are on a different OS."
    exit 1
fi

if [[ $LIST -eq 1 ]]; then
    echo "** Current DNS settings **"
    networksetup -listallnetworkservices 2>/dev/null | grep -v '\*' | while read -r x ; do
        RESULT=$(networksetup -getdnsservers "$x")
        RESULT=$(echo $RESULT)
        printf '%-30s %s\n' "$x:" "$RESULT"
    done
    exit 1
fi

if [ $EUID -ne 0 ]; then
    echo "Must be root to update system resolvers. Retry using 'sudo stubby-setdns'"
    exit 1
fi

if [[ $RESET -eq 1 ]]; then
    SERVERS="empty"
    echo "Setting DNS servers to $SERVERS - the system will use default DNS service."
else
    echo "Setting DNS servers to $SERVERS - the system will use Stubby if it is running."
fi

### Set the DNS settings via networksetup ###
networksetup -listallnetworkservices 2>/dev/null | grep -v '\*' | while read -r x ; do
    networksetup -setdnsservers "$x" $SERVERS
done