#! /bin/sh
# source: ftp.sh
# Copyright Gerhard Rieger and contributors (see file CHANGES)
# Published under the GNU General Public License V.2, see file COPYING

# example how to write a shell script that communicates with stdio on the front
# end and with a socat address on the back end

# usage:
# ftp.sh [opts] server directory/	# show directory contents on stdout
# ftp.sh [opts] server file		# print file contents to stdout
# opts:
#	-socks socksserver	# use given socks server, port 1080
#	-proxy proxyserver	# use given proxy server, port 8080
#				# must be http proxy that accepts CONNECT
#				# method to ports 21 and >=1024
#	-user username		# default: "ftp"
#	-passwd password	# default: "anonymous@domain.org"
#	-t	# shell script trace+debug
#	-d	# debug on control connection (use up to 4 times)
#	-D	# debug on data connection (use up to 4 times)
#	-b	# block size for data connection
#	-v	# verbose
#	-l*	# socat logging options
# example:
# ftp.sh -v -d -d -D -D -D -b 65536 -proxy proxy ftp.ftp.org /README >README

user="ftp"
passwd="anonymous@domain.org"
#method="socks4:socks"	# socks4 is address spec, socks is socks server name
method=tcp
addropts=

# socat options
SO1=
SO2=

while :; do
    case "$1" in
    -socks|-socks4) shift;
	case "$1" in
	*:*) method="socks4:${1%%:*}"; addropts="socksport=${1#*:}" ;;
	*) method="socks4:$1" ;;
	esac ;;
    -socks4a) shift;
	case "$1" in
	*:*) method="socks4a:${1%%:*}"; addropts="socksport=${1#*:}" ;;
	*) method="socks4a:$1" ;;
	esac ;;
    -proxy) shift;
	case "$1" in
	*:*) method="proxy:${1%%:*}"; addropts="proxyport=${1#*:}" ;;
	*) method="proxy:$1" ;;
	esac ;;
    -user) shift; user="$1" ;;
    -passwd) shift; passwd="$1" ;;
    -t) set -vx ;;
    -d) SO1="$SO1 -d" ;;
    -D) SO2="$SO2 -d" ;;
    -b) SO2="$SO2 -b $2"; shift ;;
    -v) SO1="$SO1 -v" ;;
    -l*) SO1="$SO1 $1"; SO2="$SO2 $1" ;;
    -*) echo "unknown option \"$1\"" >&2; exit 1;;
    *) break ;;
    esac
    shift
done
export SO2

server="$1"
dir="$2"

echo "addr=$method:$server:21,$addropts"; exit

### this is the central part to establish communication with socat ###
### copy these lines to make new communication shell scripts 
TMPDIR=$(if [ -x /bin/mktemp ]; then
	    /bin/mktemp -d /tmp/$USER/FTPSH.XXXXXX
	 else
	    (umask 077; d=/tmp/$USER/FTPSH.$$; mkdir $d; echo $d)
	 fi)
TO="$TMPDIR/to"; FROM="$TMPDIR/from"
socat $SO1 fifo:$TO,nonblock,ignoreeof!!fifo:$FROM $method:$server:21,$addropts &
S1=$!
while ! [ -p "$TO" -a -p "$FROM" ]; do sleep 1; done
exec 4>$TMPDIR/to 3<$TMPDIR/from
trap "S1=" 17
#trap "echo cleaning up...>&2; rm -r $TMPDIR; [ -n "$S1" ] && kill $S1" 0 3
trap "rm -r $TMPDIR" 0 3
### here the central part ends


# this function waits for a complete server message, checks if its status
# is in the permitted range (terminates session if not), and returns.
ftp_chat () {
    local cmd="$1"
    local errlevel="$2";  [ -z "$errlevel" ] && errlevel=300
    if [ -n "$cmd" ]; then echo "$cmd" >&4; fi
    while read status message <&3;
       ( case "$status" in [0-9][0-9][0-9]-*) exit 0;; [0-9][0-9][0-9]*) exit 1;; *) exit 1;; esac )
    do :; done
    #echo "got \"$status $message\"" >&2
    if [ -z "$status" ]; then  echo ftp data connection failed >&2; exit;  fi
    if [ "$status" -ge "$errlevel" ]; then
	echo $message >&2
	echo "QUIT" >&4; exit 1
    fi
set +vx
}


# wait for server greeting
ftp_chat

ftp_chat "USER $user" 400

ftp_chat "PASS $passwd"

#ftp_chat "CWD $dir"

case "$dir" in
*/) ftp_chat "TYPE A" ;;
*) ftp_chat "TYPE I" ;;
esac
      
echo "PASV" >&4; read status message <&3
info=$(expr "$message" : '.*[^0-9]\([0-9]*,[0-9]*,[0-9]*,[0-9]*,[0-9]*,[0-9]*\).*')
echo $info |tr ',' ' ' |(read i1 i2 i3 i4 p1 p2 

    addr=$i1.$i2.$i3.$i4
    port=$(echo "256*$p1+$p2" |bc)
    #echo $addr:$port

    trap : 20
    # open data connection and transfer data
    socat -u $SO2 $method:$server:$port,$addropts -
) &
S2=$!

case "$dir" in
*/) ftp_chat "NLST $dir" ;;
#*/) ftp_chat "LIST $dir" ;;
*) ftp_chat "RETR $dir" ;;
esac
case "$status" in
    [45]*) kill $S2;;
esac

#echo "waiting for process $S2 to terminate" >&2
wait $S2

ftp_chat

ftp_chat "QUIT"

#echo "waiting for process $S1 to terminate" >&2
wait $S1
exit
