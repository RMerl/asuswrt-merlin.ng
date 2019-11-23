#! /bin/bash
# source: test.sh
# Copyright Gerhard Rieger and contributors (see file CHANGES)
# Published under the GNU General Public License V.2, see file COPYING

# perform lots of tests on socat

# this script uses functions; you need a shell that supports them

# you can pass general options to socat: export OPTS="-d -d -d -d -lu"
# you can eg strace socat with: export TRACE="strace -v -tt -ff -D -x -s 1024 -o /tmp/$USER/socat.strace"
#set -vx

val_t=0.1
NUMCOND=true
#NUMCOND="test \$N -gt 70"
VERBOSE=
while [ "$1" ]; do
    case "X$1" in
	X-t?*) val_t="${1#-t}" ;;
	X-t)   shift; val_t="$1" ;;
	X-v)   VERBOSE=1 ;; 	# show commands
	X-n?*) NUMCOND="test \$N -eq ${1#-n}" ;;
	X-n)   shift; NUMCOND="test \$N -eq $1" ;;
	X-N?*) NUMCOND="test \$N -gt ${1#-N}" ;;
	X-N)   shift; NUMCOND="test \$N -ge $1" ;;
	*) break;
    esac
    shift
done

opt_t="-t $val_t"

UNAME=`uname`

#MICROS=100000
case "X$val_t" in
    X*.???????*) S="${val_t%.*}"; uS="${val_t#*.}"; uS="${uS:0:6}" ;;
    X*.*) S="${val_t%.*}"; uS="${val_t#*.}"; uS="${uS}000000"; uS="${uS:0:6}" ;;
    X*) S="${val_t}"; uS="000000" ;;
esac
MICROS=${S}${uS}
MICROS=${MICROS##0000}; MICROS=${MICROS##00}; MICROS=${MICROS##0}
#
_MICROS=$((MICROS+999999)); SECONDs="${_MICROS%??????}"
[ -z "$SECONDs" ] && SECONDs=0

withroot=0	# perform privileged tests even if not run by root
#PATH=$PATH:/opt/freeware/bin
#PATH=$PATH:/usr/local/ssl/bin
PATH=$PATH:/sbin 	# RHEL6:ip
case "$0" in
    */*) PATH="${0%/*}:$PATH"
esac
#OPENSSL_RAND="-rand /dev/egd-pool"
#SOCAT_EGD="egd=/dev/egd-pool"
MISCDELAY=1
[ -z "$SOCAT" ] && SOCAT="./socat"
if ! [ -x "$SOCAT" ] && ! type $SOCAT >/dev/null 2>&1; then
    echo "$SOCAT does not exist" >&2; exit 1;
fi
[ -z "$PROCAN" ] && PROCAN="./procan"
[ -z "$FILAN" ] && FILAN="./filan"
opts="$opt_t $OPTS"
export SOCAT_OPTS="$opts"
#debug="1"
debug=
TESTS="$@"; export TESTS
# for some tests we need a network interface
if type ip >/dev/null 2>&1; then
    INTERFACE=$(ip r get 8.8.8.8 |grep ' dev ' |head -n 1 |sed "s/.*dev[[:space:]][[:space:]]*\([^[:space:]][^[:space:]]*\).*/\1/")
else
    case "$UNAME" in
	Linux)   INTERFACE="$(netstat -rn |grep -e "^default" -e "^0\.0\.0\.0" |awk '{print($8);}')" ;;
	FreeBSD) INTERFACE="$(netstat -rn |grep -e "^default" -e "^0\.0\.0\.0" |awk '{print($4);}')" ;;
	*)       INTERFACE="$(netstat -rn |grep -e "^default" -e "^0\.0\.0\.0" |awk '{print($4);}')" ;;
    esac
fi
MCINTERFACE=lo	# !!! Linux only
#LOCALHOST=192.168.58.1
LOCALHOST=localhost
#LOCALHOST=127.0.0.1
LOCALHOST6=[::1]
#PROTO=$(awk '{print($2);}' /etc/protocols |sort -n |tail -n 1)
#PROTO=$(($PROTO+1))
PROTO=$((144+RANDOM/2048))
PORT=12002
SOURCEPORT=2002

# SSL certificate contents
TESTCERT_CONF=testcert.conf
TESTCERT6_CONF=testcert6.conf
#
TESTCERT_COMMONNAME="$LOCALHOST"
TESTCERT_COMMONNAME6="$LOCALHOST6"
TESTCERT_COUNTRYNAME="XY"
TESTCERT_LOCALITYNAME="Lunar Base"
TESTCERT_ORGANIZATIONALUNITNAME="socat"
TESTCERT_ORGANIZATIONNAME="dest-unreach"
TESTCERT_SUBJECT="C = $TESTCERT_COUNTRYNAME, CN = $TESTCERT_COMMONNAME, O = $TESTCERT_ORGANIZATIONNAME, OU = $TESTCERT_ORGANIZATIONALUNITNAME, L = $TESTCERT_LOCALITYNAME"
TESTCERT_ISSUER="C = $TESTCERT_COUNTRYNAME, CN = $TESTCERT_COMMONNAME, O = $TESTCERT_ORGANIZATIONNAME, OU = $TESTCERT_ORGANIZATIONALUNITNAME, L = $TESTCERT_LOCALITYNAME"
cat >$TESTCERT_CONF <<EOF
prompt=no

[ req ]
default_bits = 768
distinguished_name=Test

[ Test ]
countryName=$TESTCERT_COUNTRYNAME
commonName=$TESTCERT_COMMONNAME
O=$TESTCERT_ORGANIZATIONNAME
OU=$TESTCERT_ORGANIZATIONALUNITNAME
L=$TESTCERT_LOCALITYNAME
EOF

cat >$TESTCERT6_CONF <<EOF
prompt=no

[ req ]
default_bits = 768
distinguished_name=Test

[ Test ]
countryName=$TESTCERT_COUNTRYNAME
commonName=$TESTCERT_COMMONNAME6
O=$TESTCERT_ORGANIZATIONNAME
OU=$TESTCERT_ORGANIZATIONALUNITNAME
L=$TESTCERT_LOCALITYNAME
EOF

# clean up from previous runs
rm -f testcli.{crt,key,pem}
rm -f testsrv.{crt,key,pem}
rm -f testcli6.{crt,key,pem}
rm -f testsrv6.{crt,key,pem}


CAT=cat
OD_C="od -c"

# precision sleep; takes seconds with fractional part
psleep () {
    local T="$1"
    [ "$T" = 0 ] && T=0.000002
    $SOCAT -T "$T" pipe pipe
}
# time in microseconds to wait in some situations
if ! type usleep >/dev/null 2>&1; then
    usleep () {
	local n="$1"
	case "$n" in
	*???????) S="${n%??????}"; uS="${n:${#n}-6}" ;;
	*) S=0; uS="00000$n"; uS="${uS:${#uS}-6}" ;;
	esac
	$SOCAT -T $S.$uS pipe pipe
    }
fi
#USLEEP=usleep
F_n="%3d"	# format string for test numbers
LC_ALL=C	# for timestamps format...
LANG=C
LANGUAGE=C	# knoppix
case "$UNAME" in
HP-UX|OSF1)
    echo "$SOCAT -u stdin stdout" >cat.sh
    chmod a+x cat.sh
    CAT=./cat.sh
    ;;
SunOS)
    # /usr/bin/tr doesn't handle the a-z range syntax (needs [a-z]), use
    # /usr/xpg4/bin/tr instead
    alias tr=/usr/xpg4/bin/tr
    ;;
*)
    CAT=cat
    ;;
esac

case "$UNAME" in
#HP-UX)
#    # on HP-UX, the default options (below) hang some tests (former 14, 15)
#    PTYOPTS=
#    PTYOPTS2=
#    ;;
*)
    PTYOPTS="echo=0,opost=0"
    #PTYOPTS2="raw,echo=0"
    PTYOPTS2="cfmakeraw"
    #PTYOPTS2="rawer"
    ;;
esac

# for some tests we need an unprivileged user id to su to
if [ "$SUDO_USER" ]; then
    SUBSTUSER="$SUDO_USER"
else
    SUBSTUSER="$(grep -v '^[^:]*:^[^:]*:0:' /etc/passwd |tail -n 1 |cut -d: -f1)"
fi

# non-root users might miss ifconfig in their path
case "$UNAME" in
AIX)   IFCONFIG=/usr/sbin/ifconfig ;;
FreeBSD) IFCONFIG=/sbin/ifconfig ;;
HP-UX) IFCONFIG=/usr/sbin/ifconfig ;;
Linux) IFCONFIG=/sbin/ifconfig ;;
NetBSD)IFCONFIG=/sbin/ifconfig ;;
OpenBSD)IFCONFIG=/sbin/ifconfig ;;
OSF1)  IFCONFIG=/sbin/ifconfig ;;
SunOS) IFCONFIG=/sbin/ifconfig ;;
Darwin)IFCONFIG=/sbin/ifconfig ;;
DragonFly) IFCONFIG=/sbin/ifconfig ;;
*)     IFCONFIG=/sbin/ifconfig ;;
esac

# need output like "644"
case "$UNAME" in
    Linux) fileperms() { stat -L --print "%a\n" "$1" 2>/dev/null; } ;;
    FreeBSD) fileperms() { stat -L -x "$1" |grep ' Mode:' |sed 's/.* Mode:[[:space:]]*([0-9]\([0-7][0-7][0-7]\).*/\1/'; } ;;
    *) fileperms() {
	    local p s=0 c
	    p="$(ls -l -L "$1" |awk '{print($1);}')"
	    p="${p:1:9}"
	    while [ "$p" ]; do c=${p:0:1}; p=${p:1}; [ "x$c" == x- ]; let "s=2*s+$?"; done
	    printf "%03o\n" $s;
	} ;;
esac

# need user (owner) of filesystem entry
case "$UNAME" in
    Linux) fileuser() { stat -L --print "%U\n" "$tsock" 2>/dev/null; } ;;
    FreeBSD) fileuser() { ls -l test.sh |awk '{print($3);}'; } ;;
    *) fileuser() { ls -l test.sh |awk '{print($3);}'; } ;;
esac


# for some tests we need a second local IPv4 address
case "$UNAME" in
Linux)
    BROADCASTIF=$(ip r get 8.8.8.8 |grep ' dev ' |sed 's/.*\<dev[[:space:]][[:space:]]*\([a-z0-9][a-z0-9]*\).*/\1/')
    [ -z "$BROADCASTIF" ] && BROADCASTIF=eth0
    SECONDADDR=127.1.0.1
    SECONDMASK=255.255.0.0
    BCADDR=127.255.255.255
    BCIFADDR=$($IFCONFIG $BROADCASTIF |grep 'inet ' |awk '{print($2);}' |cut -d: -f2) ;;
FreeBSD|NetBSD|OpenBSD)
    MAINIF=$($IFCONFIG -a |grep '^[a-z]' |grep -v '^lo0: ' |head -1 |cut -d: -f1)
    BROADCASTIF="$MAINIF"
    SECONDADDR=$($IFCONFIG $BROADCASTIF |grep 'inet ' |awk '{print($2);}')
    BCIFADDR="$SECONDADDR"
    BCADDR=$($IFCONFIG $BROADCASTIF |grep 'broadcast ' |sed 's/.*broadcast/broadcast/' |awk '{print($2);}') ;;
HP-UX)
    MAINIF=lan0	# might use "netstat -ni" for this
    BROADCASTIF="$MAINIF"
    SECONDADDR=$($IFCONFIG $MAINIF |tail -n 1 |awk '{print($2);}')
    BCADDR=$($IFCONFIG $BROADCASTIF |grep 'broadcast ' |sed 's/.*broadcast/broadcast/' |awk '{print($2);}') ;;
SunOS)
    MAINIF=$($IFCONFIG -a |grep '^[a-z]' |grep -v '^lo0: ' |head -1 |cut -d: -f1)
    BROADCASTIF="$MAINIF"
    #BROADCASTIF=hme0
    #BROADCASTIF=eri0
    #SECONDADDR=$($IFCONFIG $BROADCASTIF |grep 'inet ' |awk '{print($2);}')
    SECONDADDR=$(expr "$($IFCONFIG -a |grep 'inet ' |fgrep -v ' 127.0.0.1 '| head -n 1)" : '.*inet \([0-9.]*\) .*') 
    #BCIFADDR="$SECONDADDR"
    #BCADDR=$($IFCONFIG $BROADCASTIF |grep 'broadcast ' |sed 's/.*broadcast/broadcast/' |awk '{print($2);}')
    ;;
DragonFly)
    MAINIF=$($IFCONFIG -a |grep -v ^lp |grep '^[a-z]' |grep -v '^lo0: ' |head -1 |cut -d: -f1)
    BROADCASTIF="$MAINIF"
    SECONDADDR=$($IFCONFIG $BROADCASTIF |grep 'inet ' |awk '{print($2);}')
    BCIFADDR="$SECONDADDR"
    BCADDR=$($IFCONFIG $BROADCASTIF |grep 'broadcast ' |sed 's/.*broadcast/broadcast/' |awk '{print($2);}') ;;
#AIX|FreeBSD|Solaris)
*)
    SECONDADDR=$(expr "$($IFCONFIG -a |grep 'inet ' |fgrep -v ' 127.0.0.1 ' |head -n 1)" : '.*inet \([0-9.]*\) .*') 
    ;;
esac
# for generic sockets we need this address in hex form
if [ "$SECONDADDR" ]; then
    SECONDADDRHEX="$(printf "%02x%02x%02x%02x\n" $(echo "$SECONDADDR" |tr '.' '
'))"
fi

# for some tests we need a second local IPv6 address
case "$UNAME" in
*)
    SECONDIP6ADDR=$(expr "$($IFCONFIG -a |grep 'inet6 ' |fgrep -v ' ::1/128 '| head -n 1)" : '.*inet \([0-9.]*\) .*') 
    ;;
esac
if [ -z "$SECONDIP6ADDR" ]; then
#    case "$TESTS" in
#	*%root2%*) $IFCONFIG eth0 ::2/128
#    esac
    SECONDIP6ADDR="$LOCALHOST6"
else 
    SECONDIP6ADDR="[$SECONDIP6ADDR]"
fi

TRUE=$(which true)
#E=-e	# Linux
if   [ $(echo "x\c") = "x" ]; then E=""
elif [ $(echo -e "x\c") = "x" ]; then E="-e"
else
    echo "cannot suppress trailing newline on echo" >&2
    exit 1
fi
ECHO="echo $E"
PRINTF="printf"

case "$TERM" in
vt100|vt320|linux|xterm|cons25|dtterm|aixterm|sun-color|xterm-color|xterm-256color)
	# there are different behaviours of printf (and echo)
	# on some systems, echo behaves different than printf...
	if [ $($PRINTF "\0101") = "A" ]; then
		RED="\0033[31m"
		GREEN="\0033[32m"
		YELLOW="\0033[33m"
#		if [ "$UNAME" = SunOS ]; then
#		    NORMAL="\0033[30m"
#		else
		    NORMAL="\0033[39m"
#		fi
	else
		RED="\033[31m"
		GREEN="\033[32m"
		YELLOW="\033[33m"
#		if [ "$UNAME" = SunOS ]; then
#		    NORMAL="\033[30m"
#		else
		    NORMAL="\033[39m"
#		fi
	fi
	OK="${GREEN}OK${NORMAL}"
	FAILED="${RED}FAILED${NORMAL}"
	NO_RESULT="${YELLOW}NO RESULT${NORMAL}"
	;;
*)	OK="OK"
	FAILED="FAILED"
	NO_RESULT="NO RESULT"
	;;
esac


if [ -x /usr/xpg4/bin/id ]; then
    # SunOS has rather useless tools in its default path
    PATH="/usr/xpg4/bin:$PATH"
fi

[ -z "$TESTS" ] && TESTS="consistency functions filan"
# use '%' as separation char
TESTS="%$(echo "$TESTS" |tr ' ' '%')%"

[ -z "$USER" ] && USER="$LOGNAME"	# HP-UX
if [ -z "$TMPDIR" ]; then
    if [ -z "$TMP" ]; then
	TMP=/tmp
    fi
    TMPDIR="$TMP"
fi
TD="$TMPDIR/$USER/$$"; td="$TD"
rm -rf "$TD" || (echo "cannot rm $TD" >&2; exit 1)
mkdir -p "$TD"
#trap "rm -r $TD" 0 3

echo "using temp directory $TD"

case "$TESTS" in
*%consistency%*)
# test if addresses are sorted alphabetically:
$ECHO "testing if address array is sorted...\c"
TF="$TD/socat-q"
IFS="$($ECHO ' \n\t')"
$SOCAT -? |sed '1,/address-head:/ d' |egrep 'groups=' |while IFS="$IFS:" read x y; do echo "$x"; done >"$TF"
$SOCAT -? |sed '1,/address-head:/ d' |egrep 'groups=' |while IFS="$IFS:" read x y; do echo "$x"; done |LC_ALL=C sort |diff "$TF" - >"$TF-diff"
if [ -s "$TF-diff" ]; then
    $ECHO "\n*** address array is not sorted. Wrong entries:" >&2
    cat "$TD/socat-q-diff" >&2
    exit 1
else
    echo " ok"
fi
#/bin/rm "$TF"
#/bin/rm "$TF-diff"
esac

case "$TESTS" in
*%consistency%*)
# test if address options array ("optionnames") is sorted alphabetically:
$ECHO "testing if address options are sorted...\c"
TF="$TD/socat-qq"
$SOCAT -??? |sed '1,/opt:/ d' |awk '{print($1);}' >"$TF"
LC_ALL=C sort "$TF" |diff "$TF" - >"$TF-diff"
if [ -s "$TF-diff" ]; then
    $ECHO "\n*** option array is not sorted. Wrong entries:" >&2
    cat "$TD/socat-qq-diff" >&2
    exit 1
else
    echo " ok"
fi
/bin/rm "$TF"
/bin/rm "$TF-diff"
esac

#==============================================================================
case "$TESTS" in
*%options%*)

# inquire which options are available
OPTS_ANY=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*ANY' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_BLK=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*BLK' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_CHILD=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*CHILD' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_CHR=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*CHR' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_DEVICE=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*DEVICE' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_EXEC=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*EXEC' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_FD=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*FD' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_FIFO=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*FIFO' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_FORK=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*FORK' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_LISTEN=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*LISTEN' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_NAMED=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*NAMED' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_OPEN=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*OPEN[^S]' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_PARENT=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*PARENT' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_READLINE=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*READLINE' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_RETRY=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*RETRY' |awk '{print($1);}' |grep -v forever|xargs echo |tr ' ' ',')
OPTS_RANGE=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*RANGE' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_FILE=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*REG' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_UNIX=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*UNIX' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_SOCKET=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*SOCKET' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_TERMIOS=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*TERMIOS' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_IP4=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*IP4' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_IP6=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*IP6' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_TCP=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*TCP' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_UDP=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*UDP' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_SOCKS4=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*SOCKS4' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_PROCESS=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*PROCESS' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_OPENSSL=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*OPENSSL' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_PTY=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*PTY' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_HTTP=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*HTTP' |awk '{print($1);}' |xargs echo |tr ' ' ',')
OPTS_APPL=$($SOCAT -?? |sed '1,/opt:/ d' |egrep 'groups=([A-Z]+,)*APPL' |awk '{print($1);}' |xargs echo |tr ' ' ',')

# find user ids to setown to; non-root only can setown to itself
if [ $(id -u) = 0 ]; then
  # up to now, it is not a big problem when these do not exist
  _UID=nobody
  _GID=staff
else
  _UID=$(id -u)
  _GID=$(id -g)
fi

# some options require values; here we try to replace these bare options with
#    valid forms.
filloptionvalues() {
    local OPTS=",$1,"
    #
    case "$OPTS" in
    *,umask,*) OPTS=$(echo "$OPTS" |sed "s/,umask,/,umask=0026,/g");;
    esac
    case "$OPTS" in
    *,user,*) OPTS=$(echo "$OPTS" |sed "s/,user,/,user=$_UID,/g");;
    esac
    case "$OPTS" in
    *,user-early,*) OPTS=$(echo "$OPTS" |sed "s/,user-early,/,user-early=$_UID,/g");;
    esac
    case "$OPTS" in
    *,user-late,*) OPTS=$(echo "$OPTS" |sed "s/,user-late,/,user-late=$_UID,/g");;
    esac
    case "$OPTS" in
    *,owner,*) OPTS=$(echo "$OPTS" |sed "s/,owner,/,owner=$_UID,/g");;
    esac
    case "$OPTS" in
    *,uid,*) OPTS=$(echo "$OPTS" |sed "s/,uid,/,uid=$_UID,/g");;
    esac
    case "$OPTS" in
    *,uid-l,*) OPTS=$(echo "$OPTS" |sed "s/,uid-l,/,uid-l=$_UID,/g");;
    esac
    case "$OPTS" in
    *,setuid,*) OPTS=$(echo "$OPTS" |sed "s/,setuid,/,setuid=$_UID,/g");;
    esac
    case "$OPTS" in
    *,group,*) OPTS=$(echo "$OPTS" |sed "s/,group,/,group=$_GID,/g");;
    esac
    case "$OPTS" in
    *,group-early,*) OPTS=$(echo "$OPTS" |sed "s/,group-early,/,group-early=$_GID,/g");;
    esac
    case "$OPTS" in
    *,group-late,*) OPTS=$(echo "$OPTS" |sed "s/,group-late,/,group-late=$_GID,/g");;
    esac
    case "$OPTS" in
    *,gid,*) OPTS=$(echo "$OPTS" |sed "s/,gid,/,gid=$_GID,/g");;
    esac
    case "$OPTS" in
    *,gid-l,*) OPTS=$(echo "$OPTS" |sed "s/,gid-l,/,gid-l=$_GID,/g");;
    esac
    case "$OPTS" in
    *,setgid,*) OPTS=$(echo "$OPTS" |sed "s/,setgid,/,setgid=$_GID,/g");;
    esac
    case "$OPTS" in
    *,mode,*) OPTS=$(echo "$OPTS" |sed "s/,mode,/,mode=0700,/g");;
    esac
    case "$OPTS" in
    *,perm,*) OPTS=$(echo "$OPTS" |sed "s/,perm,/,perm=0700,/g");;
    esac
    case "$OPTS" in
    *,perm-early,*) OPTS=$(echo "$OPTS" |sed "s/,perm-early,/,perm-early=0700,/g");;
    esac
    case "$OPTS" in
    *,perm-late,*) OPTS=$(echo "$OPTS" |sed "s/,perm-late,/,perm-late=0700,/g");;
    esac
    case "$OPTS" in
    *,path,*) OPTS=$(echo "$OPTS" |sed "s/,path,/,path=.,/g");;
    esac
    # SOCKET
    case "$OPTS" in
    *,bind,*) OPTS=$(echo "$OPTS" |sed "s/,bind,/,bind=:,/g");;
    esac
    case "$OPTS" in
    *,linger,*) OPTS=$(echo "$OPTS" |sed "s/,linger,/,linger=2,/g");;
    esac
    case "$OPTS" in
    *,connect-timeout,*) OPTS=$(echo "$OPTS" |sed "s/,connect-timeout,/,connect-timeout=1,/g");;
    esac
    # IP
    case "$OPTS" in
    *,ipoptions,*) OPTS=$(echo "$OPTS" |sed "s|,ipoptions,|,ipoptions=x01,|g");;
    esac
    case "$OPTS" in
    *,pf,*) OPTS=$(echo "$OPTS" |sed "s|,pf,|,pf=ip4,|g");;
    esac
    case "$OPTS" in
    *,range,*) OPTS=$(echo "$OPTS" |sed "s|,range,|,range=127.0.0.1/32,|g");;
    esac
    case "$OPTS" in
    *,if,*) OPTS=$(echo "$OPTS" |sed "s/,if,/,if=$INTERFACE,/g");;
    esac
    # PTY
    case "$OPTS" in
    *,pty-interval,*) OPTS=$(echo "$OPTS" |sed "s/,pty-interval,/,pty-interval=$INTERFACE,/g");;
    esac
    # RETRY
    case "$OPTS" in
    *,interval,*) OPTS=$(echo "$OPTS" |sed "s/,interval,/,interval=1,/g");;
    esac
    # READLINE
    case "$OPTS" in
    *,history,*) OPTS=$(echo "$OPTS" |sed "s/,history,/,history=.history,/g");;
    esac
    case "$OPTS" in
    *,noecho,*) OPTS=$(echo "$OPTS" |sed "s/,noecho,/,noecho=password,/g");;
    esac
    case "$OPTS" in
    *,prompt,*) OPTS=$(echo "$OPTS" |sed "s/,prompt,/,prompt=CMD,/g");;
    esac
    # IPAPP
    case "$OPTS" in
    *,sp,*) OPTS=$(echo "$OPTS" |sed "s/,sp,/,sp=$SOURCEPORT,/g");;
    esac
    # OPENSSL
    case "$OPTS" in
    *,ciphers,*) OPTS=$(echo "$OPTS" |sed "s/,ciphers,/,ciphers=NULL,/g");;
    esac
    case "$OPTS" in
    *,method,*) OPTS=$(echo "$OPTS" |sed "s/,method,/,method=SSLv3,/g");;
    esac
    case "$OPTS" in
    *,cafile,*) OPTS=$(echo "$OPTS" |sed "s/,cafile,/,cafile=/tmp/hugo,/g");;
    esac
    case "$OPTS" in
    *,capath,*) OPTS=$(echo "$OPTS" |sed "s/,capath,/,capath=/tmp/hugo,/g");;
    esac
    case "$OPTS" in
    *,cert,*) OPTS=$(echo "$OPTS" |sed "s/,cert,/,cert=/tmp/hugo,/g");;
    esac
    case "$OPTS" in
    *,key,*) OPTS=$(echo "$OPTS" |sed "s/,key,/,key=/tmp/hugo,/g");;
    esac
    case "$OPTS" in
    *,dh,*) OPTS=$(echo "$OPTS" |sed "s/,dh,/,dh=/tmp/hugo,/g");;
    esac
    case "$OPTS" in
    *,egd,*) OPTS=$(echo "$OPTS" |sed "s/,egd,/,egd=/tmp/hugo,/g");;
    esac
    case "$OPTS" in
    *,compress,*) OPTS=$(echo "$OPTS" |sed "s/,compress,/,compress=none,/g");;
    esac
    # PROXY
    case "$OPTS" in
    *,proxyauth,*) OPTS=$(echo "$OPTS" |sed "s/,proxyauth,/,proxyauth=user:pass,/g");;
    esac
    case "$OPTS" in
    *,proxyport,*) OPTS=$(echo "$OPTS" |sed "s/,proxyport,/,proxyport=3128,/g");;
    esac
    case "$OPTS" in
    *,link,*) OPTS=$(echo "$OPTS" |sed "s/,link,/,link=testlink,/g");;
    esac
    # TCP-WRAPPERS
    case "$OPTS" in
    *,allow-table,*) OPTS=$(echo "$OPTS" |sed "s|,allow-table,|,allow-table=/tmp/hugo,|g");;
    esac
    case "$OPTS" in
    *,deny-table,*) OPTS=$(echo "$OPTS" |sed "s|,deny-table,|,deny-table=/tmp/hugo,|g");;
    esac
    case "$OPTS" in
    *,tcpwrap-dir,*) OPTS=$(echo "$OPTS" |sed "s|,tcpwrap-dir,|,tcpwrap-dir=/tmp,|g");;
    esac
    echo $OPTS >&2
    expr "$OPTS" : ',\(.*\),'
}
# OPTS_FIFO: nothing yet

# OPTS_CHR: nothing yet

# OPTS_BLK: nothing yet

# OPTS_REG: nothing yet

OPTS_SOCKET=",$OPTS_SOCKET,"
OPTS_SOCKET=$(expr "$OPTS_SOCKET" : ',\(.*\),')

N=1
#------------------------------------------------------------------------------

#method=open
#METHOD=$(echo "$method" |tr a-z A-Z)
#TEST="$METHOD on file accepts all its options"
#    echo "### $TEST"
#TF=$TD/file$N
#DA="test$N $(date) $RANDOM"
#OPTGROUPS=$($SOCAT -? |fgrep " $method:" |sed 's/.*=//')
#for g in $(echo $OPTGROUPS |tr ',' ' '); do
#    eval "OPTG=\$OPTS_$(echo $g |tr a-z- A-Z_)";
#    OPTS="$OPTS,$OPTG";
#done
##echo $OPTS
#
#for o in $(filloptionvalues $OPTS|tr ',' ' '); do
#    echo testing if $METHOD accepts option $o
#    touch $TF
#    $SOCAT $opts -!!$method:$TF,$o /dev/null,ignoreof </dev/null
#    rm -f $TF
#done

#------------------------------------------------------------------------------

# test openssl connect

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
pid=$!
for addr in openssl; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
#    echo OPTGROUPS=$OPTGROUPS
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
	openssl s_server -www -accept $PORT || echo "cannot start s_server" >&2 &
	pid=$!
	sleep 1
	#waittcp4port $PORT
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
#	echo $SOCAT $opts /dev/null $addr:$LOCALHOST:$PORT,$o
	$SOCAT $opts /dev/null $addr:$LOCALHOST:$PORT,$o
    done
	kill $pid
done
kill $pid 2>/dev/null
opts=
	PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test proxy connect

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
pid=$!
for addr in proxy; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
#    echo OPTGROUPS=$OPTGROUPS
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    # prepare dummy server
    $SOCAT tcp-l:$PORT,reuseaddr,crlf exec:"/bin/bash proxyecho.sh" || echo "cannot start proxyecho.sh" >&2 &
	pid=$!
	sleep 1
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
#	echo $SOCAT $opts /dev/null $addr:$LOCALHOST:127.0.0.1:$PORT,$o
	$SOCAT $opts /dev/null $addr:$LOCALHOST:127.0.0.1:$((PORT+1)),proxyport=$PORT,$o
    done
	kill $pid 2>/dev/null
done
kill $pid 2>/dev/null
opts=
PORT=$((PORT+2))
fi

#------------------------------------------------------------------------------

# test tcp4

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
$SOCAT $opts tcp4-listen:$PORT,reuseaddr,fork,$o echo </dev/null &
pid=$!
for addr in tcp4; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
#    echo OPTGROUPS=$OPTGROUPS
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts /dev/null $addr:$LOCALHOST:$PORT,$o
    done
done
kill $pid 2>/dev/null
opts=
PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test udp4-connect

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
$SOCAT $opts udp4-listen:$PORT,fork,$o echo </dev/null &
pid=$!
for addr in udp4-connect; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
#    echo OPTGROUPS=$OPTGROUPS
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts /dev/null $addr:$LOCALHOST:$PORT,$o
    done
done
kill $pid 2>/dev/null
opts=
PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test tcp4-listen

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
for addr in tcp4-listen; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts $ADDR:$PORT,reuseaddr,$o echo </dev/null &
	pid=$!
	$SOCAT /dev/null tcp4:$LOCALHOST:$PORT 2>/dev/null
	kill $pid 2>/dev/null
    done
done
opts=
PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test udp4-listen

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
for addr in udp4-listen; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts $ADDR:$PORT,reuseaddr,$o echo </dev/null &
	pid=$!
	$SOCAT /dev/null udp4:$LOCALHOST:$PORT 2>/dev/null
	kill $pid 2>/dev/null
    done
done
opts=
PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test udp4-sendto

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
$SOCAT $opts udp4-recv:$PORT,fork,$o echo </dev/null &
pid=$!
for addr in udp4-sendto; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
#    echo OPTGROUPS=$OPTGROUPS
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts /dev/null $addr:$LOCALHOST:$PORT,$o
    done
done
kill $pid 2>/dev/null
opts=
PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test udp4-datagram

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
#$SOCAT $opts udp4-recvfrom:$PORT,fork,$o echo </dev/null &
#pid=$!
for addr in udp4-datagram; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
#    echo OPTGROUPS=$OPTGROUPS
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts /dev/null $addr:$LOCALHOST:$PORT,$o
    done
done
#kill $pid 2>/dev/null
opts=
PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test udp4-recv

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
for addr in udp4-recv; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts $ADDR:$PORT,reuseaddr,$o echo </dev/null &
	pid=$!
	$SOCAT /dev/null udp4:$LOCALHOST:$PORT 2>/dev/null
	kill $pid 2>/dev/null
    done
done
opts=
PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test udp4-recvfrom

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
for addr in udp4-recvfrom; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts $ADDR:$PORT,reuseaddr,$o echo </dev/null &
	pid=$!
	$SOCAT /dev/null udp4:$LOCALHOST:$PORT 2>/dev/null
	kill $pid 2>/dev/null
    done
done
opts=
PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test ip4-sendto

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
$SOCAT $opts ip4-recv:$PORT,fork,$o echo </dev/null &
pid=$!
for addr in ip4-sendto; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
#    echo OPTGROUPS=$OPTGROUPS
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts /dev/null $addr:$LOCALHOST:$PORT,$o
    done
done
kill $pid 2>/dev/null
opts=
PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test ip4-recv

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
for addr in ip4-recv; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts $ADDR:$PORT,reuseaddr,$o echo </dev/null &
	pid=$!
	$SOCAT /dev/null ip4-sendto:$LOCALHOST:$PORT 2>/dev/null
	kill $pid 2>/dev/null
    done
done
opts=
PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test ip4-recvfrom

#set -vx
if true; then
#if false; then
#opts="-s -d -d -d -d"
for addr in ip4-recvfrom; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts $ADDR:$PORT,reuseaddr,$o echo </dev/null &
	pid=$!
	$SOCAT /dev/null ip4-sendto:$LOCALHOST:$PORT 2>/dev/null
	kill $pid 2>/dev/null
    done
done
opts=
PORT=$((PORT+1))
fi

#------------------------------------------------------------------------------

# test READLINE

if true; then
#if false; then
#opts="-s -d -d -d -d"
for addr in readline; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    TS=$TD/script$N
    OPTGROUPS=$($SOCAT -? |fgrep " $addr	" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
#    for o in bs0; do
	echo "testing if $ADDR accepts option $o"
	echo "$SOCAT $opts readline,$o /dev/null" >$TS
	chmod u+x $TS
	$SOCAT /dev/null,ignoreeof exec:$TS,pty
	#stty sane
    done
    #reset 1>&0 2>&0
done
opts=
fi

#------------------------------------------------------------------------------

# unnamed pipe
#if false; then
if true; then
for addr in pipe; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="unnamed $ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |egrep " $addr[^:]" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS

    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if unnamed $ADDR accepts option $o
	$SOCAT $opts $addr,$o /dev/null </dev/null
    done
done
fi

#------------------------------------------------------------------------------

# test addresses on files

N=1
#if false; then
if true; then
for addr in create; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR on new file accepts all its options"
    echo "### $TEST"
    TF=$TD/file$N
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS

    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR accepts option $o
	rm -f $TF
	$SOCAT $opts -!!$addr:$TF,$o /dev/null,ignoreof </dev/null
	rm -f $TF
    done
done
fi
#------------------------------------------------------------------------------

#if false; then
if true; then
for addr in exec system; do
    ADDR=$(echo "$addr" |tr a-z A-Z)

    TEST="$ADDR with socketpair accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTGROUPS=$(echo $OPTGROUPS |sed -e 's/,FIFO,/,/g' -e 's/,TERMIOS,/,/g' -e 's/,PTY,/,/g')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    OPTS=$(echo $OPTS|sed -e 's/,pipes,/,/g' -e 's/,pty,/,/g' -e 's/,openpty,/,/g' -e 's/,ptmx,/,/g' -e 's/,nofork,/,/g')
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR with socketpair accepts option $o
	$SOCAT $opts $addr:$TRUE,$o /dev/null,ignoreof </dev/null
    done

    TEST="$ADDR with pipes accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTGROUPS=$(echo $OPTGROUPS |sed -e 's/,TERMIOS,/,/g' -e 's/,PTY,/,/g' -e 's/,SOCKET,/,/g' -e 's/,UNIX//g')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    # flock tends to hang, so dont test it
    OPTS=$(echo $OPTS|sed -e 's/,pipes,/,/g' -e 's/,pty,/,/g' -e 's/,openpty,/,/g' -e 's/,ptmx,/,/g' -e 's/,nofork,/,/g' -e 's/,flock,/,/g')
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR with pipes accepts option $o
	$SOCAT $opts $addr:$TRUE,pipes,$o /dev/null,ignoreof </dev/null
    done

    TEST="$ADDR with pty accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTGROUPS=$(echo $OPTGROUPS |sed -e 's/,FIFO,/,/g' -e 's/,SOCKET,/,/g' -e 's/,UNIX//g')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    OPTS=$(echo $OPTS|sed -e 's/,pipes,/,/g' -e 's/,pty,/,/g' -e 's/,openpty,/,/g' -e 's/,ptmx,/,/g' -e 's/,nofork,/,/g')
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR with pty accepts option $o
	$SOCAT $opts $addr:$TRUE,pty,$o /dev/null,ignoreof </dev/null
    done

    TEST="$ADDR with nofork accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTGROUPS=$(echo $OPTGROUPS |sed -e 's/,FIFO,/,/g' -e 's/,PTY,/,/g' -e 's/,TERMIOS,/,/g' -e 's/,SOCKET,/,/g' -e 's/,UNIX//g')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    OPTS=$(echo $OPTS|sed -e 's/,pipes,/,/g' -e 's/,pty,/,/g' -e 's/,openpty,/,/g' -e 's/,ptmx,/,/g' -e 's/,nofork,/,/g')
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR with nofork accepts option $o
	$SOCAT /dev/null $opts $addr:$TRUE,nofork,$o </dev/null
    done

done
fi

#------------------------------------------------------------------------------

#if false; then
if true; then
for addr in fd; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    TF=$TD/file$N
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR (to file) accepts option $o"
	rm -f $TF
	$SOCAT $opts -u /dev/null $addr:3,$o 3>$TF
    done
done
fi

#------------------------------------------------------------------------------

# test OPEN address

#! test it on pipe, device, new file

N=1
#if false; then
if true; then
for addr in open; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR on file accepts all its options"
    echo "### $TEST"
    TF=$TD/file$N
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR on file accepts option $o
	touch $TF
	$SOCAT $opts -!!$addr:$TF,$o /dev/null,ignoreof </dev/null
	rm -f $TF
    done
done
fi

#------------------------------------------------------------------------------

# test GOPEN address on files, sockets, pipes, devices

N=1
#if false; then
if true; then
for addr in gopen; do
    ADDR=$(echo "$addr" |tr a-z A-Z)

    TEST="$ADDR on new file accepts all its options"
    echo "### $TEST"
    TF=$TD/file$N
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTGROUPS=$(echo $OPTGROUPS |sed -e 's/,SOCKET,/,/g' -e 's/,UNIX//g')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR on new file accepts option $o
	rm -f $TF
	$SOCAT $opts -!!$addr:$TF,$o /dev/null,ignoreof </dev/null
	rm -f $TF
    done

    TEST="$ADDR on existing file accepts all its options"
    echo "### $TEST"
    TF=$TD/file$N
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTGROUPS=$(echo $OPTGROUPS |sed -e 's/,SOCKET,/,/g' -e 's/,UNIX//g')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR on existing file accepts option $o
	rm -f $TF; touch $TF
	$SOCAT $opts -!!$addr:$TF,$o /dev/null,ignoreof </dev/null
	rm -f $TF
    done

    TEST="$ADDR on existing pipe accepts all its options"
    echo "### $TEST"
    TF=$TD/pipe$N
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTGROUPS=$(echo $OPTGROUPS |sed -e 's/,REG,/,/g' -e 's/,SOCKET,/,/g' -e 's/,UNIX//g')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR on named pipe accepts option $o
	rm -f $TF; mkfifo $TF
	$SOCAT $opts $addr:$TF,$o,nonblock /dev/null </dev/null
	rm -f $TF
    done

    TEST="$ADDR on existing socket accepts all its options"
    echo "### $TEST"
    TF=$TD/sock$N
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTGROUPS=$(echo $OPTGROUPS |sed -e 's/,REG,/,/g' -e 's/,OPEN,/,/g')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR on socket accepts option $o
	rm -f $TF; $SOCAT - UNIX-L:$TF & pid=$!
	$SOCAT $opts -!!$addr:$TF,$o /dev/null,ignoreof </dev/null
	kill $pid 2>/dev/null
	rm -f $TF
    done

  if [ $(id -u) -eq 0 ]; then
    TEST="$ADDR on existing device accepts all its options"
    echo "### $TEST"
    TF=$TD/null
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTGROUPS=$(echo $OPTGROUPS |sed -e 's/,REG,/,/g' -e 's/,OPEN,/,/g')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR on existing device accepts option $o
	rm -f $TF; mknod $TF c 1 3
	$SOCAT $opts -!!$addr:$TF,$o /dev/null,ignoreof </dev/null
    done
  else
    TEST="$ADDR on existing device accepts all its options"
    echo "### $TEST"
    TF=/dev/null
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTGROUPS=$(echo $OPTGROUPS |sed -e 's/,REG,/,/g' -e 's/,OPEN,/,/g')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if $ADDR on existing device accepts option $o
	$SOCAT $opts -!!$addr:$TF,$o /dev/null,ignoreof </dev/null
    done
  fi

done
fi

#------------------------------------------------------------------------------

# test named pipe

N=1
#if false; then
if true; then
for addr in pipe; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR on file accepts all its options"
    echo "### $TEST"
    TF=$TD/pipe$N
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS

    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo testing if named $ADDR accepts option $o
	rm -f $TF
	# blocks with rdonly, wronly
	case "$o" in rdonly|wronly) o="$o,nonblock" ;; esac
	$SOCAT $opts $addr:$TF,$o /dev/null </dev/null
	rm -f $TF
    done
done
fi
#------------------------------------------------------------------------------

# test STDIO

#! test different stream types

#if false; then
if true; then
for addr in stdio; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS

    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR (/dev/null, stdout) accepts option $o"
	$SOCAT $opts $addr,$o /dev/null,ignoreof </dev/null
    done
done
fi

#------------------------------------------------------------------------------

# test STDIN

#if false; then
if true; then
for addr in stdin; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr	" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS

    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR (/dev/null) accepts option $o"
	$SOCAT $opts -u $addr,$o /dev/null </dev/null
    done
done
fi

#------------------------------------------------------------------------------

# test STDOUT, STDERR

if true; then
#if false; then
for addr in stdout stderr; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr	" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts -u /dev/null $addr,$o
    done
done
fi

#------------------------------------------------------------------------------
# REQUIRES ROOT

if [ "$withroot" ]; then
for addr in ip4; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts $addr:127.0.0.1:200 /dev/null,ignoreof </dev/null
    done
done
fi

#------------------------------------------------------------------------------
# REQUIRES ROOT

if [ "$withroot" ]; then
for addr in ip6; do
    ADDR=$(echo "$addr" |tr a-z A-Z)
    TEST="$ADDR accepts all its options"
    echo "### $TEST"
    OPTGROUPS=$($SOCAT -? |fgrep " $addr:" |sed 's/.*=//')
    OPTS=
    for g in $(echo $OPTGROUPS |tr ',' ' '); do
	eval "OPTG=\$OPTS_$(echo $g |tr a-z A-Z)";
	OPTS="$OPTS,$OPTG";
    done
    #echo $OPTS
    for o in $(filloptionvalues $OPTS|tr ',' ' '); do
	echo "testing if $ADDR accepts option $o"
	$SOCAT $opts $addr:[::1]:200 /dev/null,ignoreof </dev/null
    done
done
fi

#==============================================================================

#TEST="stdio accepts all options of GROUP_ANY"
#echo "### $TEST"
#CMD="$SOCAT $opts -,$OPTS_ANY /dev/null"
#$CMD
#if [ $? = 0 ]; then
#    echo "... test $N ($TEST) succeeded"
##    echo "CMD=$CMD"
#else
#    echo "*** test $N ($TEST) FAILED"
#    echo "CMD=$CMD"
#fi
#
#N=$((N+1))
##------------------------------------------------------------------------------
#
#TEST="exec accepts all options of GROUP_ANY and GROUP_SOCKET"
#echo "### $TEST"
#CMD="$SOCAT $opts exec:$TRUE,$OPTS_ANY,$OPTS_SOCKET /dev/null"
#$CMD
#if [ $? = 0 ]; then
#    echo "... test $N ($TEST) succeeded"
##    echo "CMD=$CMD"
#else
#    echo "*** test $N ($TEST) FAILED"
#    echo "CMD=$CMD"
#fi

#------------------------------------------------------------------------------

esac

#==============================================================================

N=1
numOK=0
numFAIL=0
numCANT=0
listFAIL=

#==============================================================================
# test if selected socat features work ("FUNCTIONS")

testecho () {
    local N="$1"
    local title="$2"
    local arg1="$3";	[ -z "$arg1" ] && arg1="-"
    local arg2="$4";	[ -z "$arg2" ] && arg2="echo"
    local opts="$5"
    local T="$6";	[ -z "$T" ] && T=0
    local tf="$td/test$N.stdout"
    local te="$td/test$N.stderr"
    local tdiff="$td/test$N.diff"
    local da="test$N $(date) $RANDOM"
    if ! eval $NUMCOND; then :; else
    #local cmd="$TRACE $SOCAT $opts $arg1 $arg2"
    #$ECHO "testing $title (test $N)... \c"
    $PRINTF "test $F_n %s... " $N "$title"
    #echo "$da" |$cmd >"$tf" 2>"$te"
    (psleep $T; echo "$da"; psleep $T) |($TRACE $SOCAT $opts "$arg1" "$arg2" >"$tf" 2>"$te"; echo $? >"$td/test$N.rc") &
    export rc1=$!
    #sleep 5 && kill $rc1 2>/dev/null &
#    rc2=$!
    wait $rc1
#    kill $rc2 2>/dev/null
    if [ "$(cat "$td/test$N.rc")" != 0 ]; then
	$PRINTF "$FAILED: $TRACE $SOCAT:\n"
	echo "$TRACE $SOCAT $opts $arg1 $arg2" >&2
	cat "$te" >&2
	numFAIL=$((numFAIL+1))
	listFAIL="$listFAIL $N"
    elif echo "$da" |diff - "$tf" >"$tdiff" 2>&1; then
	$PRINTF "$OK\n"
	if [ "$verbose" ]; then echo "$SOCAT $opts $arg1 $arg2" >&2; fi
	if [ -n "$debug" ]; then cat $te >&2; fi
	numOK=$((numOK+1))
    else
	$PRINTF "$FAILED:\n"
	echo "$TRACE $SOCAT $opts $arg1 $arg2" >&2
	cat "$te" >&2
	echo diff: >&2
	cat "$tdiff" >&2
	numFAIL=$((numFAIL+1))
	listFAIL="$listFAIL $N"
    fi
    fi # NUMCOND
}

# test if call to od and throughput of data works - with graceful shutdown and
# flush of od buffers
testod () {
    local num="$1"
    local title="$2"
    local arg1="$3";	[ -z "$arg1" ] && arg1="-"
    local arg2="$4";	[ -z "$arg2" ] && arg2="echo"
    local opts="$5"
    local T="$6";	[ -z "$T" ] && T=0
    local tf="$td/test$N.stdout"
    local te="$td/test$N.stderr"
    local tr="$td/test$N.ref"
    local tdiff="$td/test$N.diff"
    local dain="$(date) $RANDOM"
    if ! eval $NUMCOND; then :; else
    echo "$dain" |$OD_C >"$tr"
#    local daout="$(echo "$dain" |$OD_C)"
    $PRINTF "test $F_n %s... " $num "$title"
    (psleep $T; echo "$dain"; psleep $T) |$TRACE $SOCAT $opts "$arg1" "$arg2" >"$tf" 2>"$te"
    if [ "$?" != 0 ]; then
	$PRINTF "$FAILED: $TRACE $SOCAT:\n"
	echo "$TRACE $SOCAT $opts $arg1 $arg2"
	cat "$te"
	numFAIL=$((numFAIL+1))
	listFAIL="$listFAIL $num"
#    elif echo "$daout" |diff - "$tf" >"$tdiff" 2>&1; then
    elif diff "$tr" "$tf" >"$tdiff" 2>&1; then
	$PRINTF "$OK\n"
	if [ -n "$debug" ]; then cat $te; fi
	numOK=$((numOK+1))
    else
	$PRINTF "$FAILED: diff:\n"
	echo "$TRACE $SOCAT $opts $arg1 $arg2"
	cat "$te"
	cat "$tdiff"
	numFAIL=$((numFAIL+1))
	listFAIL="$listFAIL $num"
    fi
    fi # NUMCOND
}

# test if the socat executable has these address types compiled in
# print the first missing address type
testaddrs () {
    local a A;
    for a in $@; do
	A=$(echo "$a" |tr 'a-z-' 'A-Z_')
	if $TRACE $SOCAT -V |grep "#define WITH_$A 1\$" >/dev/null; then
	    shift
	    if [[ "$FEAT" =~ OPENSSL.* ]]; then
		gentestcert testsrv
		gentestcert testcli
	    fi
	    continue
	fi
	echo "$a"
	return 1
    done
    return 0
}

# test if the socat executable has these options compiled in
# print the first missing option
testoptions () {
    local a A;
    for a in $@; do
	A=$(echo "$a" |tr 'a-z' 'A-Z')
	if $SOCAT -??? |grep "[^a-z0-9-]$a[^a-z0-9-]" >/dev/null; then
	    shift
	    continue
	fi
	echo "$a"
	return 1
    done
    return 0
}

# check if the given pid exists and has child processes
# if yes: prints child process lines to stdout, returns 0
# if not: prints ev.message to stderr, returns 1
childprocess () {
    local l
    case "$UNAME" in
    AIX)     l="$(ps -fade |grep "^........ ...... $(printf %6u $1)")" ;;
    FreeBSD) l="$(ps -faje |grep "^........ ..... $(printf %5u $1)")" ;;
    HP-UX)   l="$(ps -fade |grep "^........ ..... $(printf %5u $1)")" ;;
    Linux)   l="$(ps -fade |grep "^........ ..... $(printf %5u $1)")" ;;
#    NetBSD)  l="$(ps -aj   |grep "^........ ..... $(printf %4u $1)")" ;;
    NetBSD)  l="$(ps -aj   |grep "^[^ ][^ ]*[ ][ ]*..... $(printf %5u $1)")" ;;
    OpenBSD) l="$(ps -aj   |grep "^........ ..... $(printf %5u $1)")" ;;
    SunOS)   l="$(ps -fade |grep "^........ ..... $(printf %5u $1)")" ;;
    DragonFly)l="$(ps -faje |grep "^[^ ][^ ]*[ ][ ]*..... $(printf %5u $1)")" ;;
    CYGWIN*)  l="$(ps -pafe |grep "^[^ ]*[ ][ ]*[^ ][^ ]*[ ][ ]*$1[ ]")" ;;
    *)       l="$(ps -fade |grep "^[^ ][^ ]*[ ][ ]*[0-9][0-9]**[ ][ ]*$(printf %5u $1) ")" ;;    esac
    if [ -z "$l" ]; then
	return 1;
    fi
    echo "$l"
    return 0
}

# return a list of child process pids
childpids () {
    case "$UNAME" in
    AIX)     l="$(ps -fade |grep "^........ ...... $(printf %6u $1)" |awk '{print($2);}')" ;;
    FreeBSD) l="$(ps -fl   |grep "^[^ ][^ ]*[ ][ ]*[0-9][0-9]*[ ][ ]*$(printf %5u $1)" |awk '{print($2);}')" ;;
    HP-UX)   l="$(ps -fade |grep "^........ ..... $(printf %5u $1)" |awk '{print($2);}')" ;;
    Linux)   l="$(ps -fade |grep "^........ ..... $(printf %5u $1)" |awk '{print($2);}')" ;;
#    NetBSD)  l="$(ps -aj   |grep "^........ ..... $(printf %4u $1)" |awk '{print($2);}')" ;;
    NetBSD)  l="$(ps -aj   |grep "^[^ ][^ ]*[ ][ ]*..... $(printf %5u $1)" |awk '{print($2);}')" ;;
    OpenBSD) l="$(ps -aj   |grep "^........ ..... $(printf %5u $1)" |awk '{print($2);}')" ;;
    SunOS)   l="$(ps -fade |grep "^........ ..... $(printf %5u $1)" |awk '{print($2);}')" ;;
    DragonFly)l="$(ps -faje |grep "^[^ ][^ ]*[ ][ ]*..... $(printf %5u $1)" |awk '{print($2);}')" ;;
    CYGWIN*)  l="$(ps -pafe |grep "^[^ ]*[ ][ ]*[^ ][^ ]*[ ][ ]*$1[ ]" |awk '{print($2)';})" ;;
    *)       l="$(ps -fade |grep "^[^ ][^ ]*[ ][ ]*[0-9][0-9]*[ ][ ]*$(printf %5u $1) " |awk '{print($2)';})" ;;    esac
    if [ -z "$l" ]; then
	return 1;
    fi
    echo "$l"
    return 0
}

# check if the given process line refers to a defunct (zombie) process
# yes: returns 0
# no: returns 1
isdefunct () {
    local l
    case "$UNAME" in
    AIX)     l="$(echo "$1" |grep ' <defunct>$')" ;;
    FreeBSD) l="$(echo "$1" |grep ' <defunct>$')" ;;
    HP-UX)   l="$(echo "$1" |grep ' <defunct>$')" ;;
    Linux)   l="$(echo "$1" |grep ' <defunct>$')" ;;
    SunOS)   l="$(echo "$1" |grep ' <defunct>$')" ;;
    DragonFly)l="$(echo "$1" |grep ' <defunct>$')" ;;
    *)       l="$(echo "$1" |grep ' <defunct>$')" ;;
    esac
    [ -n "$l" ];
}

# check if UNIX socket protocol is available on host
runsunix () {
    return 0;
    $TRACE $SOCAT /dev/null UNIX-LISTEN:"$td/unix.socket" 2>"$td/unix.stderr" &
    pid=$!
    usleep $MICROS
    kill "$pid" 2>/dev/null
    test ! -s "$td/unix.stderr"
}

unset HAVENOT_IP4
# check if an IP4 loopback interface exists
runsip4 () {
    [ -n "$HAVENOT_IP4" ] && return $HAVENOT_IP4
    local l
    case "$UNAME" in
    AIX)   l=$($IFCONFIG lo0 |fgrep 'inet 127.0.0.1 ') ;;
    FreeBSD) l=$($IFCONFIG lo0 |fgrep 'inet 127.0.0.1 ') ;;
    HP-UX) l=$($IFCONFIG lo0 |fgrep 'inet 127.0.0.1 ') ;;
    Linux) l=$($IFCONFIG |egrep 'inet (addr:)?127\.0\.0\.1 ') ;;
    NetBSD)l=$($IFCONFIG -a |fgrep 'inet 127.0.0.1 ');;
    OpenBSD)l=$($IFCONFIG -a |fgrep 'inet 127.0.0.1 ');;
    OSF1)  l=$($IFCONFIG -a |grep ' inet ') ;;
    SunOS) l=$($IFCONFIG -a |grep 'inet ') ;;
    Darwin)l=$($IFCONFIG lo0 |fgrep 'inet 127.0.0.1 ') ;;
    DragonFly)l=$($IFCONFIG -a |fgrep 'inet 127.0.0.1 ');;
    CYGWIN*) l=$(ipconfig |grep IPv4);;
    *)     l=$($IFCONFIG -a |grep ' ::1[^:0-9A-Fa-f]') ;;
    esac
    [ -z "$l" ] && return 1    
    # existence of interface might not suffice, check for routeability:
    case "$UNAME" in
    Darwin) ping -c 1 127.0.0.1 >/dev/null 2>&1; l="$?" ;;
    Linux)  ping -c 1 127.0.0.1 >/dev/null 2>&1; l="$?" ;;
    *) if [ -n "$l" ]; then l=0; else l=1; fi ;;
    esac
    HAVENOT_IP4=$l
    return $l;
}

unset HAVENOT_IP6
# check if an IP6 loopback interface exists
runsip6 () {
    [ -n "$HAVENOT_IP6" ] && return $HAVENOT_IP6
    local l
    case "$UNAME" in
    AIX)   l=$($IFCONFIG lo0 |grep 'inet6 ::1/0') ;;
    HP-UX) l=$($IFCONFIG lo0 |grep ' inet6 ') ;;
    Linux) l=$($IFCONFIG |egrep 'inet6 (addr: )?::1/?') ;;
    NetBSD)l=$($IFCONFIG -a |grep 'inet6 ::1 ');;
    OSF1)  l=$($IFCONFIG -a |grep ' inet6 ') ;;
    SunOS) l=$($IFCONFIG -a |grep 'inet6 ') ;;
    Darwin)l=$($IFCONFIG lo0 |grep 'inet6 ::1 ') ;;
    CYGWIN*) l=$(ipconfig |grep IPv6);;
    *)     l=$($IFCONFIG -a |grep ' ::1[^:0-9A-Fa-f]') ;;
    esac
    [ -z "$l" ] && return 1    
    # existence of interface might not suffice, check for routeability:
    case "$UNAME" in
    Darwin) ping6 -c 1 ::1 >/dev/null 2>&1; l="$?" ;;
    Linux)  ping6 -c 1 ::1 >/dev/null 2>&1; l="$?" ;;
    *) if [ -n "$l" ]; then l=0; else l=1; fi ;;
    esac
    HAVENOT_IP6=$l
    return $l;
}

# check if TCP on IPv4 is available on host
runstcp4 () {
    runsip4 || return 1
    $SOCAT -h |grep '\<tcp4-' >/dev/null || return 1
    return 0;
}

# check if TCP on IPv6 is available on host
runstcp6 () {
    runsip6 || return 1
    $SOCAT -h |grep '\<tcp6-' >/dev/null || return 1
    return 0;
}

# check if UDP on IPv4 is available on host
runsudp4 () {
    runsip4 || return 1
    $SOCAT -h |grep '\<udp4-' >/dev/null || return 1
    return 0;
}

# check if UDP on IPv6 is available on host
runsudp6 () {
    runsip6 || return 1
    $SOCAT -h |grep '\<udp6-' >/dev/null || return 1
    return 0;
}

# check if SCTP on IPv4 is available on host
runssctp4 () {
    runsip4 || return 1
    $SOCAT -h |grep '\<sctp4-' >/dev/null || return 1
    return 0;
}

# check if SCTP on IPv6 is available on host
runssctp6 () {
    runsip6 || return 1
    $SOCAT -h |grep '\<sctp6-' >/dev/null || return 1
    return 0;
}

# wait until an IP4 protocol is ready
waitip4proto () {
    local proto="$1"
    local logic="$2"	# 0..wait until free; 1..wait until listening
    local timeout="$3"
    local l
    [ "$logic" ] || logic=1
    [ "$timeout" ] || timeout=5
    while [ $timeout -gt 0 ]; do
	case "$UNAME" in
	Linux)   l=$(netstat -n -w -l |grep '^raw .* .*[0-9*]:'$proto' [ ]*0\.0\.0\.0:\*') ;;
#	FreeBSD) l=$(netstat -an |egrep '^raw46? .*[0-9*]\.'$proto' .* \*\.\*') ;;
#	NetBSD)  l=$(netstat -an |grep '^raw .*[0-9*]\.'$proto' [ ]* \*\.\*') ;;
#	OpenBSD) l=$(netstat -an |grep '^raw .*[0-9*]\.'$proto' [ ]* \*\.\*') ;;
#	Darwin) case "$(uname -r)" in
#		[1-5]*) l=$(netstat -an |grep '^raw.* .*[0-9*]\.'$proto' .* \*\.\*') ;;
#		*) l=$(netstat -an |grep '^raw4.* .*[0-9*]\.'$proto' .* \*\.\* .*') ;;
#		esac ;;
	AIX)	 # does not seem to show raw sockets in netstat
		 sleep 1;  return 0 ;;
#	SunOS)   l=$(netstat -an -f inet -P raw |grep '.*[1-9*]\.'$proto' [ ]*Idle') ;;
#	HP-UX)   l=$(netstat -an |grep '^raw        0      0  .*[0-9*]\.'$proto' .* \*\.\* ') ;;
#	OSF1)    l=$(/usr/sbin/netstat -an |grep '^raw        0      0  .*[0-9*]\.'$proto' [ ]*\*\.\*') ;;
 	*)       #l=$(netstat -an |grep -i 'raw .*[0-9*][:.]'$proto' ') ;;
		 sleep 1;  return 0 ;;
	esac
	[ \( \( $logic -ne 0 \) -a -n "$l" \) -o \
	  \( \( $logic -eq 0 \) -a -z "$l" \) ] && return 0
	sleep 1
	timeout=$((timeout-1))
    done

    $ECHO "!protocol $proto timed out! \c" >&2
    return 1
}

# we need this misleading function name for canonical reasons
waitip4port () {
    waitip4proto "$1" "$2" "$3"
}

# wait until an IP6 protocol is ready
waitip6proto () {
    local proto="$1"
    local logic="$2"	# 0..wait until free; 1..wait until listening
    local timeout="$3"
    local l
    [ "$logic" ] || logic=1
    [ "$timeout" ] || timeout=5
    while [ $timeout -gt 0 ]; do
	case "$UNAME" in
	Linux)   l=$(netstat -n -w -l |grep '^raw[6 ] .* .*:[0-9*]*:'$proto' [ ]*:::\*') ;;
#	FreeBSD) l=$(netstat -an |egrep '^raw46? .*[0-9*]\.'$proto' .* \*\.\*') ;;
#	NetBSD)  l=$(netstat -an |grep '^raw .*[0-9*]\.'$proto' [ ]* \*\.\*') ;;
#	OpenBSD) l=$(netstat -an |grep '^raw .*[0-9*]\.'$proto' [ ]* \*\.\*') ;;
#	Darwin) case "$(uname -r)" in
#		[1-5]*) l=$(netstat -an |grep '^raw.* .*[0-9*]\.'$proto' .* \*\.\*') ;;
#		*) l=$(netstat -an |grep '^raw4.* .*[0-9*]\.'$proto' .* \*\.\* .*') ;;
#		esac ;;
	AIX)	 # does not seem to show raw sockets in netstat
		 sleep 1;  return 0 ;;
#	SunOS)   l=$(netstat -an -f inet -P raw |grep '.*[1-9*]\.'$proto' [ ]*Idle') ;;
#	HP-UX)   l=$(netstat -an |grep '^raw        0      0  .*[0-9*]\.'$proto' .* \*\.\* ') ;;
#	OSF1)    l=$(/usr/sbin/netstat -an |grep '^raw        0      0  .*[0-9*]\.'$proto' [ ]*\*\.\*') ;;
 	*)       #l=$(netstat -an |egrep -i 'raw6? .*[0-9*][:.]'$proto' ') ;;
		 sleep 1;  return 0 ;;
	esac
	[ \( \( $logic -ne 0 \) -a -n "$l" \) -o \
	  \( \( $logic -eq 0 \) -a -z "$l" \) ] && return 0
	sleep 1
	timeout=$((timeout-1))
    done

    $ECHO "!protocol $proto timed out! \c" >&2
    return 1
}

# we need this misleading function name for canonical reasons
waitip6port () {
    waitip6proto "$1" "$2" "$3"
}

# check if a TCP4 port is in use
# exits with 0 when it is not used
checktcp4port () {
    local port="$1"
    local l
    case "$UNAME" in
    Linux)   l=$(netstat -a -n -t |grep '^tcp .* .*[0-9*]:'$port' .* LISTEN') ;;
    FreeBSD) l=$(netstat -an |grep '^tcp4.* .*[0-9*]\.'$port' .* \*\.\* .* LISTEN') ;;
    NetBSD)  l=$(netstat -an |grep '^tcp .* .*[0-9*]\.'$port' [ ]* \*\.\* [ ]* LISTEN.*') ;;
    Darwin) case "$(uname -r)" in
	[1-5]*) l=$(netstat -an |grep '^tcp.* .*[0-9*]\.'$port' .* \*\.\* .* LISTEN') ;;
	*) l=$(netstat -an |grep '^tcp4.* .*[0-9*]\.'$port' .* \*\.\* .* LISTEN') ;;
	esac ;;
    AIX)     l=$(netstat -an |grep '^tcp[^6]       0      0 .*[*0-9]\.'$port' .* LISTEN$') ;;
    SunOS)   l=$(netstat -an -f inet -P tcp |grep '.*[1-9*]\.'$port' .*\*                0 .* LISTEN') ;;
    HP-UX)   l=$(netstat -an |grep '^tcp        0      0  .*[0-9*]\.'$port' .* LISTEN$') ;;
    OSF1)    l=$(/usr/sbin/netstat -an |grep '^tcp        0      0  .*[0-9*]\.'$port' [ ]*\*\.\* [ ]*LISTEN') ;;
    CYGWIN*) l=$(netstat -an -p TCP |grep '^  TCP    [0-9.]*:'$port' .* LISTENING') ;;
    DragonFly)l=$(netstat -ant |grep '^tcp4 .* .*[0-9*]\.'$port' [ ]* \*\.\* [ ]* LISTEN.*') ;;
    *)       l=$(netstat -an |grep -i 'tcp .*[0-9*][:.]'$port' .* listen') ;;
    esac
    [ -z "$l" ] && return 0
    return 1
}

# wait until a TCP4 listen port is ready
waittcp4port () {
    local port="$1"
    local logic="$2"	# 0..wait until free; 1..wait until listening
    local timeout="$3"
    local l
    local vx=+; case $- in *vx*) set +vx; vx=-; esac	# no tracing here
    [ "$logic" ] || logic=1
    [ "$timeout" ] || timeout=5
    while [ $timeout -gt 0 ]; do
	case "$UNAME" in
	Linux)   l=$(netstat -a -n -t -l |grep '^tcp .* .*[0-9*]:'$port' .* LISTEN') ;;
	FreeBSD) l=$(netstat -an |grep '^tcp4.* .*[0-9*]\.'$port' .* \*\.\* .* LISTEN') ;;
	NetBSD)  l=$(netstat -an |grep '^tcp .* .*[0-9*]\.'$port' [ ]* \*\.\* [ ]* LISTEN.*') ;;
	Darwin) case "$(uname -r)" in
		[1-5]*) l=$(netstat -an |grep '^tcp.* .*[0-9*]\.'$port' .* \*\.\* .* LISTEN') ;;
		*) l=$(netstat -an |grep '^tcp4.* .*[0-9*]\.'$port' .* \*\.\* .* LISTEN') ;;
		esac ;;
	AIX)     l=$(netstat -an |grep '^tcp[^6]       0      0 .*[*0-9]\.'$port' .* LISTEN$') ;;
	SunOS)   l=$(netstat -an -f inet -P tcp |grep '.*[1-9*]\.'$port' .*\*                0 .* LISTEN') ;;
	HP-UX)   l=$(netstat -an |grep '^tcp        0      0  .*[0-9*]\.'$port' .* LISTEN$') ;;
	OSF1)    l=$(/usr/sbin/netstat -an |grep '^tcp        0      0  .*[0-9*]\.'$port' [ ]*\*\.\* [ ]*LISTEN') ;;
	CYGWIN*) l=$(netstat -an -p TCP |grep '^  TCP    [0-9.]*:'$port' .* LISTENING') ;;
	DragonFly)  l=$(netstat -ant |grep '^tcp4 .* .*[0-9*]\.'$port' [ ]* \*\.\* [ ]* LISTEN.*') ;;
 	*)       l=$(netstat -an |grep -i 'tcp .*[0-9*][:.]'$port' .* listen') ;;
	esac
	if [ \( \( $logic -ne 0 \) -a -n "$l" \) -o \
	    \( \( $logic -eq 0 \) -a -z "$l" \) ]; then
	    set ${vx}vx
	    return 0
	fi
	sleep 1
	timeout=$((timeout-1))
    done

    $ECHO "!port $port timed out! \c" >&2
    set ${vx}vx
    return 1
}

# wait until a UDP4 port is ready
waitudp4port () {
    local port="$1"
    local logic="$2"	# 0..wait until free; 1..wait until listening
    local timeout="$3"
    local l
    local vx=+; case $- in *vx*) set +vx; vx=-; esac	# no tracing here
     [ "$logic" ] || logic=1
    [ "$timeout" ] || timeout=5
    while [ $timeout -gt 0 ]; do
	case "$UNAME" in
	Linux)   l=$(netstat -a -n -u -l |grep '^udp .* .*[0-9*]:'$port' [ ]*0\.0\.0\.0:\*') ;;
	FreeBSD) l=$(netstat -an |egrep '^udp46? .*[0-9*]\.'$port' .* \*\.\*') ;;
	NetBSD)  l=$(netstat -an |grep '^udp .*[0-9*]\.'$port' [ ]* \*\.\*') ;;
	OpenBSD) l=$(netstat -an |grep '^udp .*[0-9*]\.'$port' [ ]* \*\.\*') ;;
	Darwin) case "$(uname -r)" in
		[1-5]*) l=$(netstat -an |grep '^udp.* .*[0-9*]\.'$port' .* \*\.\*') ;;
		*) l=$(netstat -an |grep '^udp4.* .*[0-9*]\.'$port' .* \*\.\* .*') ;;
		esac ;;
	AIX)	 l=$(netstat -an |grep '^udp[4 ]       0      0 .*[*0-9]\.'$port' .* \*\.\*[ ]*$') ;;
	SunOS)   l=$(netstat -an -f inet -P udp |grep '.*[1-9*]\.'$port' [ ]*Idle') ;;
	HP-UX)   l=$(netstat -an |grep '^udp        0      0  .*[0-9*]\.'$port' .* \*\.\* ') ;;
	OSF1)    l=$(/usr/sbin/netstat -an |grep '^udp        0      0  .*[0-9*]\.'$port' [ ]*\*\.\*') ;;
	DragonFly) l=$(netstat -an |grep '^udp4 .* .*[0-9*]\.'$port' [ ]* \*\.\* [ ]*') ;;
 	*)       l=$(netstat -an |grep -i 'udp .*[0-9*][:.]'$port' ') ;;
	esac
	if [ \( \( $logic -ne 0 \) -a -n "$l" \) -o \
	    \( \( $logic -eq 0 \) -a -z "$l" \) ]; then
	    set ${vx}vx
	    return 0
	fi
	sleep 1
	timeout=$((timeout-1))
    done

    $ECHO "!port $port timed out! \c" >&2
    set ${vx}vx
    return 1
}

# wait until an SCTP4 listen port is ready
waitsctp4port () {
    local port="$1"
    local logic="$2"	# 0..wait until free; 1..wait until listening
    local timeout="$3"
    local l
    local vx=+; case $- in *vx*) set +vx; vx=-; esac	# no tracing here
     [ "$logic" ] || logic=1
    [ "$timeout" ] || timeout=5
    while [ $timeout -gt 0 ]; do
	case "$UNAME" in
	Linux)   l=$(netstat -n -a |grep '^sctp .* .*[0-9*]:'$port' .* LISTEN') ;;
#	FreeBSD) l=$(netstat -an |grep '^tcp4.* .*[0-9*]\.'$port' .* \*\.\* .* LISTEN') ;;
#	NetBSD)  l=$(netstat -an |grep '^tcp .* .*[0-9*]\.'$port' [ ]* \*\.\* [ ]* LISTEN.*') ;;
#	Darwin) case "$(uname -r)" in
#		[1-5]*) l=$(netstat -an |grep '^tcp.* .*[0-9*]\.'$port' .* \*\.\* .* LISTEN') ;;
#		*) l=$(netstat -an |grep '^tcp4.* .*[0-9*]\.'$port' .* \*\.\* .* LISTEN') ;;
#		esac ;;
#	AIX)	 l=$(netstat -an |grep '^tcp[^6]       0      0 .*[*0-9]\.'$port' .* LISTEN$') ;;
	SunOS)   l=$(netstat -an -f inet -P sctp |grep '.*[1-9*]\.'$port' .*\*                0 .* LISTEN') ;;
#	HP-UX)   l=$(netstat -an |grep '^tcp        0      0  .*[0-9*]\.'$port' .* LISTEN$') ;;
#	OSF1)    l=$(/usr/sbin/netstat -an |grep '^tcp        0      0  .*[0-9*]\.'$port' [ ]*\*\.\* [ ]*LISTEN') ;;
#	CYGWIN*) l=$(netstat -an -p TCP |grep '^  TCP    [0-9.]*:'$port' .* LISTENING') ;;
 	*)       l=$(netstat -an |grep -i 'sctp .*[0-9*][:.]'$port' .* listen') ;;
	esac
	if [ \( \( $logic -ne 0 \) -a -n "$l" \) -o \
	    \( \( $logic -eq 0 \) -a -z "$l" \) ]; then
	    set ${vx}vx
	    return 0
	fi
	sleep 1
	timeout=$((timeout-1))
    done

    $ECHO "!port $port timed out! \c" >&2
    set ${vx}vx
    return 1
}

# wait until a tcp6 listen port is ready
waittcp6port () {
    local port="$1"
    local logic="$2"	# 0..wait until free; 1..wait until listening
    local timeout="$3"
    local l
    local vx=+; case $- in *vx*) set +vx; vx=-; esac	# no tracing here
    [ "$logic" ] || logic=1
    [ "$timeout" ] || timeout=5
    while [ $timeout -gt 0 ]; do
	case "$UNAME" in
	Linux)   l=$(netstat -an |grep -E '^tcp6? .* [0-9a-f:%]*:'$port' .* LISTEN') ;;
	FreeBSD) l=$(netstat -an |egrep -i 'tcp(6|46) .*[0-9*][:.]'$port' .* listen') ;;
	NetBSD)  l=$(netstat -an |grep '^tcp6 .*[0-9*]\.'$port' [ ]* \*\.\*') ;;
	OpenBSD) l=$(netstat -an |grep -i 'tcp6 .*[0-9*][:.]'$port' .* listen') ;;
	Darwin)  l=$(netstat -an |egrep '^tcp4?6 +[0-9]+ +[0-9]+ +[0-9a-z:%*]+\.'$port' +[0-9a-z:%*.]+ +LISTEN') ;;
	AIX)	 l=$(netstat -an |grep '^tcp[6 ]       0      0 .*[*0-9]\.'$port' .* LISTEN$') ;;
	SunOS)   l=$(netstat -an -f inet6 -P tcp |grep '.*[1-9*]\.'$port' .*\* [ ]* 0 .* LISTEN') ;;
	#OSF1)    l=$(/usr/sbin/netstat -an |grep '^tcp6       0      0  .*[0-9*]\.'$port' [ ]*\*\.\* [ ]*LISTEN') /*?*/;;
	DragonFly)  l=$(netstat -ant |grep '^tcp6 .* .*[0-9*]\.'$port' [ ]* \*\.\* [ ]* LISTEN.*') ;;
 	*)       l=$(netstat -an |grep -i 'tcp6 .*:'$port' .* listen') ;;
	esac
	if [ \( \( $logic -ne 0 \) -a -n "$l" \) -o \
	    \( \( $logic -eq 0 \) -a -z "$l" \) ]; then
	    set ${vx}vx
	    return 0
	fi
	sleep 1
	timeout=$((timeout-1))
    done

    $ECHO "!port $port timed out! \c" >&2
    #echo set ${vx}vx >&2
    set ${vx}vx
    return 1
}

# wait until a UDP6 port is ready
waitudp6port () {
    local port="$1"
    local logic="$2"	# 0..wait until free; 1..wait until listening
    local timeout="$3"
    local l
    local vx=+; case $- in *vx*) set +vx; vx=-; esac	# no tracing here
    [ "$logic" ] || logic=1
    [ "$timeout" ] || timeout=5
    while [ $timeout -gt 0 ]; do
	case "$UNAME" in
	Linux)   l=$(netstat -an |grep -E '^udp6? .* .*[0-9*:%]:'$port' [ ]*:::\*') ;;
	FreeBSD) l=$(netstat -an |egrep '^udp(6|46) .*[0-9*]\.'$port' .* \*\.\*') ;;
	NetBSD)  l=$(netstat -an |grep '^udp6 .* \*\.'$port' [ ]* \*\.\*') ;;
    	OpenBSD) l=$(netstat -an |grep '^udp6 .*[0-9*]\.'$port' [ ]* \*\.\*') ;;
	Darwin)  l=$(netstat -an |egrep '^udp4?6 +[0-9]+ +[0-9]+ +[0-9a-z:%*]+\.'$port' +[0-9a-z:%*.]+') ;;
	AIX)	 l=$(netstat -an |grep '^udp[6 ]       0      0 .*[*0-9]\.'$port' .* \*\.\*[ ]*$') ;;
	SunOS)   l=$(netstat -an -f inet6 -P udp |grep '.*[1-9*]\.'$port' [ ]*Idle') ;;
	#HP-UX)   l=$(netstat -an |grep '^udp        0      0  .*[0-9*]\.'$port' ') ;;
	#OSF1)    l=$(/usr/sbin/netstat -an |grep '^udp6       0      0  .*[0-9*]\.'$port' [ ]*\*\.\*') ;;
	DragonFly) l=$(netstat -ant |grep '^udp6 .* .*[0-9*]\.'$port' [ ]* \*\.\* [ ]*') ;;
 	*)       l=$(netstat -an |grep -i 'udp .*[0-9*][:.]'$port' ') ;;
	esac
	if [ \( \( $logic -ne 0 \) -a -n "$l" \) -o \
	    \( \( $logic -eq 0 \) -a -z "$l" \) ]; then
	    set ${vx}vx
	    return 0
	fi
	sleep 1
	timeout=$((timeout-1))
    done

    $ECHO "!port $port timed out! \c" >&2
    set ${vx}vx
    return 1
}

# wait until a sctp6 listen port is ready
# not all (Linux) variants show this in netstat
waitsctp6port () {
    local port="$1"
    local logic="$2"	# 0..wait until free; 1..wait until listening
    local timeout="$3"
    local l
    local vx=+; case $- in *vx*) set +vx; vx=-; esac	# no tracing here
    [ "$logic" ] || logic=1
    [ "$timeout" ] || timeout=5
    while [ $timeout -gt 0 ]; do
	case "$UNAME" in
	Linux)   l=$(netstat -an |grep '^sctp[6 ] .* [0-9a-f:]*:'$port' .* LISTEN') ;;
#	FreeBSD) l=$(netstat -an |grep -i 'tcp[46][6 ] .*[0-9*][:.]'$port' .* listen') ;;
#	NetBSD)  l=$(netstat -an |grep '^tcp6 .*[0-9*]\.'$port' [ ]* \*\.\*') ;;
#	OpenBSD) l=$(netstat -an |grep -i 'tcp6 .*[0-9*][:.]'$port' .* listen') ;;
#	AIX)	 l=$(netstat -an |grep '^tcp[6 ]       0      0 .*[*0-9]\.'$port' .* LISTEN$') ;;
	SunOS)   l=$(netstat -an -f inet6 -P sctp |grep '.*[1-9*]\.'$port' .*\* [ ]* 0 .* LISTEN') ;;
#	#OSF1)    l=$(/usr/sbin/netstat -an |grep '^tcp6       0      0  .*[0-9*]\.'$port' [ ]*\*\.\* [ ]*LISTEN') /*?*/;;
 	*)       l=$(netstat -an |grep -i 'stcp6 .*:'$port' .* listen') ;;
	esac
	if [ \( \( $logic -ne 0 \) -a -n "$l" \) -o \
	    \( \( $logic -eq 0 \) -a -z "$l" \) ]; then
	    set ${vx}vx
	    return 0
	fi
	sleep 1
	timeout=$((timeout-1))
    done

    $ECHO "!port $port timed out! \c" >&2
    set ${vx}vx
    return 1
}

# we need this misleading function name for canonical reasons
waitunixport () {
    waitfile "$1" "$2" "$3"
}

# wait until a filesystem entry exists
waitfile () {
    local crit=-e
    case "X$1" in X-*) crit="$1"; shift ;; esac
    local file="$1"
    local logic="$2"	# 0..wait until gone; 1..wait until exists (default);
			# 2..wait until not empty
    local timeout="$3"
    local vx=+; case $- in *vx*) set +vx; vx=-; esac	# no tracing here
    [ "$logic" ] || logic=1
    [ "$logic" -eq 2 ] && crit=-s
    [ "$timeout" ] || timeout=5
    while [ $timeout -gt 0 ]; do
	if [ \( \( $logic -ne 0 \) -a $crit "$file" \) -o \
	    \( \( $logic -eq 0 \) -a ! $crit "$file" \) ]; then
	    set ${vx}vx
	    return 0
	fi
	sleep 1
	timeout=$((timeout-1))
    done

    echo "file $file timed out" >&2
    set ${vx}vx
    return 1
}

# generate a test certificate and key
gentestcert () {
    local name="$1"
    if [ -s $name.key -a -s $name.crt -a -s $name.pem ]; then return; fi
    openssl genrsa $OPENSSL_RAND -out $name.key 768 >/dev/null 2>&1
    openssl req -new -config $TESTCERT_CONF -key $name.key -x509 -out $name.crt -days 3653 >/dev/null 2>&1
    cat $name.key $name.crt >$name.pem
}

# generate a test DSA key and certificate
gentestdsacert () {
    local name="$1"
    if [ -s $name.key -a -s $name.crt -a -s $name.pem ]; then return; fi
    openssl dsaparam -out $name-dsa.pem 1024 >/dev/null 2>&1
    openssl dhparam -dsaparam -out $name-dh.pem 1024 >/dev/null 2>&1
    openssl req -newkey dsa:$name-dsa.pem -keyout $name.key -nodes -x509 -config $TESTCERT_CONF -out $name.crt -days 3653 >/dev/null 2>&1
    cat $name-dsa.pem $name-dh.pem $name.key $name.crt >$name.pem
}

# generate a test EC key and certificate
gentesteccert () {
    local name="$1"
    if [ -s $name.key -a -s $name.crt -a -s $name.pem ]; then return; fi
    openssl ecparam -name secp521r1 -out $name-ec.pem >/dev/null 2>&1
    chmod 0400 $name-ec.pem
    openssl req -newkey ec:$name-ec.pem -keyout $name.key -nodes -x509 -config $TESTCERT_CONF -out $name.crt -days 3653 >/dev/null 2>&1
    cat $name-ec.pem $name.key $name.crt >$name.pem
}

gentestcert6 () {
    local name="$1"
    if [ -s $name.key -a -s $name.crt -a -s $name.pem ]; then return; fi
    cat $TESTCERT_CONF |
    { echo "# automatically generated by $0"; cat; } |
    sed 's/\(commonName\s*=\s*\).*/\1[::1]/' >$TESTCERT6_CONF
    openssl genrsa $OPENSSL_RAND -out $name.key 768 >/dev/null 2>&1
    openssl req -new -config $TESTCERT6_CONF -key $name.key -x509 -out $name.crt -days 3653 >/dev/null 2>&1
    cat $name.key $name.crt >$name.pem
}


NAME=UNISTDIO
case "$TESTS " in
*%$N%*|*%functions%*|*%stdio%*|*%$NAME%*)
TEST="$NAME: unidirectional throughput from stdin to stdout"
testecho "$N" "$TEST" "stdin" "stdout" "$opts -u"
esac
N=$((N+1))


NAME=UNPIPESTDIO
case "$TESTS" in
*%$N%*|*%functions%*|*%stdio%*|*%$NAME%*)
TEST="$NAME: stdio with simple echo via internal pipe"
testecho "$N" "$TEST" "stdio" "pipe" "$opts"
esac
N=$((N+1))


NAME=UNPIPESHORT
case "$TESTS" in
*%$N%*|*%functions%*|*%stdio%*|*%$NAME%*)
TEST="$NAME: short form of stdio ('-') with simple echo via internal pipe"
testecho "$N" "$TEST" "-" "pipe" "$opts"
esac
N=$((N+1))


NAME=DUALSTDIO
case "$TESTS" in
*%$N%*|*%functions%*|*%stdio%*|*%$NAME%*)
TEST="$NAME: splitted form of stdio ('stdin!!stdout') with simple echo via internal pipe"
testecho "$N" "$TEST" "stdin!!stdout" "pipe" "$opts"
esac
N=$((N+1))


NAME=DUALSHORTSTDIO
case "$TESTS" in
*%$N%*|*%functions%*|*%stdio%*|*%$NAME%*)
TEST="$NAME: short splitted form of stdio ('-!!-') with simple echo via internal pipe"
testecho "$N" "$TEST" "-!!-" "pipe" "$opts"
esac
N=$((N+1))


NAME=DUALFDS
case "$TESTS" in
*%$N%*|*%functions%*|*%fd%*|*%$NAME%*)
TEST="$NAME: file descriptors with simple echo via internal pipe"
testecho "$N" "$TEST" "0!!1" "pipe" "$opts"
esac
N=$((N+1))


NAME=NAMEDPIPE
case "$TESTS" in
*%$N%*|*%functions%*|*%pipe%*|*%$NAME%*)
TEST="$NAME: simple echo via named pipe"
# with MacOS, this test hangs if nonblock is not used. Is an OS bug.
tp="$td/pipe$N"
# note: the nonblock is required by MacOS 10.1(?), otherwise it hangs (OS bug?)
testecho "$N" "$TEST" "" "pipe:$tp,nonblock" "$opts"
esac
N=$((N+1))


NAME=DUALPIPE
case "$TESTS" in
*%$N%*|*%functions%*|*%pipe%*|*%$NAME%*)
TEST="$NAME: simple echo via named pipe, specified twice"
tp="$td/pipe$N"
testecho "$N" "$TEST" "" "pipe:$tp,nonblock!!pipe:$tp" "$opts"
esac
N=$((N+1))


NAME=FILE
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%file%*|*%ignoreeof%*|*%$NAME%*)
TEST="$NAME: simple echo via file"
tf="$td/file$N"
testecho "$N" "$TEST" "" "$tf,ignoreeof!!$tf" "$opts"
esac
N=$((N+1))


NAME=EXECSOCKET
case "$TESTS" in
*%$N%*|*%functions%*|*%exec%*|*%$NAME%*)
TEST="$NAME: simple echo via exec of cat with socketpair"
testecho "$N" "$TEST" "" "exec:$CAT" "$opts"
esac
N=$((N+1))


NAME=SYSTEMSOCKET
case "$TESTS" in
*%$N%*|*%functions%*|*%system%*|*%$NAME%*)
TEST="$NAME: simple echo via system() of cat with socketpair"
testecho "$N" "$TEST" "" "system:$CAT" "$opts" "$val_t"
esac
N=$((N+1))


NAME=EXECPIPES
case "$TESTS" in
*%$N%*|*%functions%*|*%pipe%*|*%$NAME%*)
TEST="$NAME: simple echo via exec of cat with pipes"
testecho "$N" "$TEST" "" "exec:$CAT,pipes" "$opts"
esac
N=$((N+1))


NAME=SYSTEMPIPES
case "$TESTS" in
*%$N%*|*%functions%*|*%pipes%*|*%$NAME%*)
TEST="$NAME: simple echo via system() of cat with pipes"
testecho "$N" "$TEST" "" "system:$CAT,pipes" "$opts"
esac
N=$((N+1))


NAME=EXECPTY
case "$TESTS" in
*%$N%*|*%functions%*|*%exec%*|*%pty%*|*%$NAME%*)
TEST="$NAME: simple echo via exec of cat with pseudo terminal"
if ! eval $NUMCOND; then :;
elif ! testaddrs pty >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}PTY not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testecho "$N" "$TEST" "" "exec:$CAT,pty,$PTYOPTS" "$opts"
fi
esac
N=$((N+1))


NAME=SYSTEMPTY
case "$TESTS" in
*%$N%*|*%functions%*|*%system%*|*%pty%*|*%$NAME%*)
TEST="$NAME: simple echo via system() of cat with pseudo terminal"
if ! eval $NUMCOND; then :;
elif ! testaddrs pty >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}PTY not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testecho "$N" "$TEST" "" "system:$CAT,pty,$PTYOPTS" "$opts"
fi
esac
N=$((N+1))


NAME=SYSTEMPIPESFDS
case "$TESTS" in
*%$N%*|*%functions%*|*%system%*|*%$NAME%*)
TEST="$NAME: simple echo via system() of cat with pipes, non stdio"
testecho "$N" "$TEST" "" "system:$CAT>&9 <&8,pipes,fdin=8,fdout=9" "$opts"
esac
N=$((N+1))


NAME=DUALSYSTEMFDS
case "$TESTS" in
*%$N%*|*%functions%*|*%system%*|*%$NAME%*)
TEST="$NAME: echo via dual system() of cat"
testecho "$N" "$TEST" "system:$CAT>&6,fdout=6!!system:$CAT<&7,fdin=7" "" "$opts" "$val_t"
esac
N=$((N+1))


# test: send EOF to exec'ed sub process, let it finish its operation, and 
# check if the sub process returns its data before terminating.
NAME=EXECSOCKETFLUSH
# idea: have socat exec'ing od; send data and EOF, and check if the od'ed data
# arrives.
case "$TESTS" in
*%$N%*|*%functions%*|*%exec%*|*%$NAME%*)
TEST="$NAME: call to od via exec with socketpair"
testod "$N" "$TEST" "" "exec:$OD_C" "$opts"
esac
N=$((N+1))


NAME=SYSTEMSOCKETFLUSH
case "$TESTS" in
*%$N%*|*%functions%*|*%system%*|*%$NAME%*)
TEST="$NAME: call to od via system() with socketpair"
testod "$N" "$TEST" "" "system:$OD_C" "$opts" $val_t
esac
N=$((N+1))


NAME=EXECPIPESFLUSH
case "$TESTS" in
*%$N%*|*%functions%*|*%exec%*|*%$NAME%*)
TEST="$NAME: call to od via exec with pipes"
testod "$N" "$TEST" "" "exec:$OD_C,pipes" "$opts"
esac
N=$((N+1))


NAME=SYSTEMPIPESFLUSH
case "$TESTS" in
*%$N%*|*%functions%*|*%system%*|*%$NAME%*)
TEST="$NAME: call to od via system() with pipes"
testod "$N" "$TEST" "" "system:$OD_C,pipes" "$opts" "$val_t"
esac
N=$((N+1))


## LATER:
#NAME=EXECPTYFLUSH
#case "$TESTS" in
#*%$N%*|*%functions%*|*%exec%*|*%pty%*|*%$NAME%*)
#TEST="$NAME: call to od via exec with pseudo terminal"
#if ! testaddrs pty >/dev/null; then
#    $PRINTF "test $F_n $TEST... ${YELLOW}PTY not available${NORMAL}\n" $N
#    numCANT=$((numCANT+1))
#else
#testod "$N" "$TEST" "" "exec:$OD_C,pty,$PTYOPTS" "$opts"
#fi
#esac
#N=$((N+1))


## LATER:
#NAME=SYSTEMPTYFLUSH
#case "$TESTS" in
#*%$N%*|*%functions%*|*%system%*|*%pty%*|*%$NAME%*)
#TEST="$NAME: call to od via system() with pseudo terminal"
#if ! testaddrs pty >/dev/null; then
#    $PRINTF "test $F_n $TEST... ${YELLOW}PTY not available${NORMAL}\n" $N
#    numCANT=$((numCANT+1))
#else
#testod "$N" "$TEST" "" "system:$OD_C,pty,$PTYOPTS" "$opts"
#fi
#esac
#N=$((N+1))


NAME=SYSTEMPIPESFDSFLUSH
case "$TESTS" in
*%$N%*|*%functions%*|*%system%*|*%$NAME%*)
TEST="$NAME: call to od via system() with pipes, non stdio"
testod "$N" "$TEST" "" "system:$OD_C>&9 <&8,pipes,fdin=8,fdout=9" "$opts" "$val_t"
esac
N=$((N+1))


NAME=DUALSYSTEMFDSFLUSH
case "$TESTS" in
*%$N%*|*%functions%*|*%system%*|*%$NAME%*)
TEST="$NAME: call to od via dual system()"
testod "$N" "$TEST" "system:$OD_C>&6,fdout=6!!system:$CAT<&7,fdin=7" "pipe" "$opts" "$val_t"
esac
N=$((N+1))


case "$UNAME" in
Linux)  IPPROTO=254 ;;
Darwin) IPPROTO=255 ;;
*)      IPPROTO=254 ;;	# just a guess
esac

NAME=RAWIP4SELF
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%rawip%*|*%root%*|*%$NAME%*)
TEST="$NAME: simple echo via self receiving raw IPv4 protocol"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testaddrs rawip) >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}RAWIP not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
    testecho "$N" "$TEST" "" "ip4:127.0.0.1:$IPPROTO" "$opts"
fi
esac
N=$((N+1))

NAME=RAWIPX4SELF
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%rawip%*|*%root%*|*%$NAME%*)
TEST="$NAME: simple echo via self receiving raw IP protocol, v4 by target"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testaddrs rawip) >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}RAWIP not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
    testecho "$N" "$TEST" "" "ip:127.0.0.1:$IPPROTO" "$opts"
fi ;; # NUMCOND, feats
esac
N=$((N+1))

NAME=RAWIP6SELF
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%rawip%*|*%root%*|*%$NAME%*)
TEST="$NAME: simple echo via self receiving raw IPv6 protocol"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testaddrs rawip) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}RAWIP not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
    testecho "$N" "$TEST" "" "ip6:[::1]:$IPPROTO" "$opts"
fi ;; # NUMCOND, feats
esac
N=$((N+1))

NAME=RAWIPX6SELF
case "$TESTS" in
*%$N%*|*%functions%*|*%ip%*|*%ip6%*|*%rawip%*|*%rawip6%*|*%root%*|*%$NAME%*)
TEST="$NAME: simple echo via self receiving raw IP protocol, v6 by target"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testaddrs rawip) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}RAWIP not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
    testecho "$N" "$TEST" "" "ip:[::1]:$IPPROTO" "$opts"
fi
esac
N=$((N+1))


NAME=TCPSELF
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: echo via self connection of TCP IPv4 socket"
if ! eval $NUMCOND; then :;
elif [ "$UNAME" != Linux ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}only on Linux$NORMAL\n" $N
    numCANT=$((numCANT+1))
else
    #ts="127.0.0.1:$tsl"
    testecho "$N" "$TEST" "" "tcp:$SECONDADDR:$PORT,sp=$PORT,bind=$SECONDADDR,reuseaddr" "$opts"
fi
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=UDPSELF
if ! eval $NUMCOND; then :; else
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: echo via self connection of UDP IPv4 socket"
if [ "$UNAME" != Linux ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}only on Linux$NORMAL\n" $N
    numCANT=$((numCANT+1))
else
    testecho "$N" "$TEST" "" "udp:$SECONDADDR:$PORT,sp=$PORT,bind=$SECONDADDR" "$opts"
fi
esac
fi # NUMCOND
PORT=$((PORT+1))
N=$((N+1))


NAME=UDP6SELF
case "$TESTS" in
*%$N%*|*%functions%*|*%udp%*|*%udp6%*|*%ip6%*|*%$NAME%*)
TEST="$NAME: echo via self connection of UDP IPv6 socket"
if ! eval $NUMCOND; then :;
elif [ "$UNAME" != Linux ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}only on Linux${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs udp ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
    tf="$td/file$N"
    testecho "$N" "$TEST" "" "udp6:[::1]:$PORT,sp=$PORT,bind=[::1]" "$opts"
fi
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=DUALUDPSELF
if ! eval $NUMCOND; then :; else
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: echo via two unidirectional UDP IPv4 sockets"
tf="$td/file$N"
p1=$PORT
p2=$((PORT+1))
testecho "$N" "$TEST" "" "udp:127.0.0.1:$p2,sp=$p1!!udp:127.0.0.1:$p1,sp=$p2" "$opts"
esac
fi # NUMCOND
PORT=$((PORT+2))
N=$((N+1))


#function testdual {
#    local
#}


NAME=UNIXSTREAM
if ! eval $NUMCOND; then :; else
case "$TESTS" in
*%$N%*|*%functions%*|*%unix%*|*%$NAME%*)
TEST="$NAME: echo via connection to UNIX domain socket"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
ts="$td/test$N.socket"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts UNIX-LISTEN:$ts PIPE"
CMD2="$TRACE $SOCAT $opts -!!- UNIX-CONNECT:$ts"
printf "test $F_n $TEST... " $N
$CMD1 </dev/null >$tf 2>"${te}1" &
bg=$!	# background process id
waitfile "$ts"
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2=$?
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   echo "rc=$rc2"
   cat "${te}1"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $bg 2>/dev/null
esac
fi # NUMCOND
N=$((N+1))


NAME=TCP4
if ! eval $NUMCOND; then :; else
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: echo via connection to TCP V4 socket"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="127.0.0.1:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP4-LISTEN:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP4:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1=$!
waittcp4port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   cat "${te}1"
   echo "$CMD2"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid1 2>/dev/null
wait ;;
esac
PORT=$((PORT+1))
fi # NUMCOND
N=$((N+1))


#et -xv
NAME=TCP6
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: echo via connection to TCP V6 socket"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="[::1]:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP6-listen:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP6:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp6port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "$te"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
fi
esac
PORT=$((PORT+1))
N=$((N+1))
#set +vx


NAME=TCPX4
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: echo via connection to TCP socket, v4 by target"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="127.0.0.1:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP-listen:$tsl,pf=ip4,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp4port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "$te"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
fi
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=TCPX6
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: echo via connection to TCP socket, v6 by target"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="[::1]:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP-listen:$tsl,pf=ip6,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp6port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "$te"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
fi
esac
PORT=$((PORT+1))
N=$((N+1))

# TCP6-LISTEN may also listen for IPv4 connections. Test if option
# ipv6-v6only=0 shows this behaviour.
NAME=IPV6ONLY0
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: option ipv6-v6only=0 listens on IPv4"
# create a listening TCP6 socket and try to connect to the port using TCP4
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testoptions ipv6-v6only); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="127.0.0.1:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP6-LISTEN:$tsl,ipv6-v6only=0,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts STDIN!!STDOUT TCP4:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp6port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1" "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
fi
esac
PORT=$((PORT+1))
N=$((N+1))

#set -vx
# TCP6-LISTEN may also listen for IPv4 connections. Test if option
# ipv6-v6only=1 turns off this behaviour.
NAME=IPV6ONLY1
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: option ipv6-v6only=1 does not listen on IPv4"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testoptions ipv6-v6only); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="127.0.0.1:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP6-listen:$tsl,ipv6-v6only=1,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP4:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp6port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -eq 0 ]; then
   $PRINTF "$FAILED:\n"
   cat "${te}1" "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED:\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   numOK=$((numOK+1))
fi
kill $pid; wait
wait
fi
esac
PORT=$((PORT+1))
N=$((N+1))
#set +vx

NAME=ENV_LISTEN_4
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: env SOCAT_DEFAULT_LISTEN_IP for IPv4 preference on listen"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testoptions ipv6-v6only); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="127.0.0.1:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP-listen:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP4:$ts"
printf "test $F_n $TEST... " $N
SOCAT_DEFAULT_LISTEN_IP=4 $CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp4port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1" "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null; wait
fi
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=ENV_LISTEN_6
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: env SOCAT_DEFAULT_LISTEN_IP for IPv6 preference on listen"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="[::1]:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP-listen:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP6:$ts"
printf "test $F_n $TEST... " $N
SOCAT_DEFAULT_LISTEN_IP=6 $CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp6port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1" "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null; wait
fi
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=LISTEN_OPTION_4
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: option -4 for IPv4 preference on listen"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testoptions ipv6-v6only); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="127.0.0.1:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -4 TCP-listen:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP4:$ts"
printf "test $F_n $TEST... " $N
SOCAT_DEFAULT_LISTEN_IP=6 $CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp4port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1" "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null; wait
fi
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=LISTEN_OPTION_6
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: option -6 for IPv6 preference on listen"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="[::1]:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -6 TCP-listen:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP6:$ts"
printf "test $F_n $TEST... " $N
SOCAT_DEFAULT_LISTEN_IP=4 $CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp6port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1" "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null; wait
wait
fi # feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=LISTEN_PF_IP4
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: pf=4 overrides option -6 on listen"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testoptions ipv6-v6only); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="127.0.0.1:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -6 TCP-listen:$tsl,pf=ip4,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP4:$ts"
printf "test $F_n $TEST... " $N
SOCAT_DEFAULT_LISTEN_IP=6 $CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp4port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1" "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null; wait
fi
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=LISTEN_PF_IP6
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: pf=6 overrides option -4 on listen"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="[::1]:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -4 TCP-listen:$tsl,pf=ip6,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP6:$ts"
printf "test $F_n $TEST... " $N
SOCAT_DEFAULT_LISTEN_IP=4 $CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp6port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1" "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null; wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=UDP4STREAM
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%ipapp%*|*%udp%*|*%$NAME%*)
TEST="$NAME: echo via connection to UDP V4 socket"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="$LOCALHOST:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts UDP4-LISTEN:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts - UDP4:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1=$!
waitudp4port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2=$?
kill $pid1 2>/dev/null; wait
if [ $rc2 -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1" "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   echo "$CMD1 &"
   cat "${te}1"
   echo "$CMD2"
   cat "${te}2"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=UDP6STREAM
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%udp%*|*%$NAME%*)
TEST="$NAME: echo via connection to UDP V6 socket"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="$LOCALHOST6:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts UDP6-LISTEN:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts - UDP6:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1=$!
waitudp6port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2=$?
kill $pid1 2>/dev/null; wait
if [ $rc2 -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1" "${te}2"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
fi ;; # ! testaddrs
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=GOPENFILE
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%gopen%*|*%file%*|*%ignoreeof%*|*%$NAME%*)
TEST="$NAME: file opening with gopen"
if ! eval $NUMCOND; then :; else
tf1="$td/test$N.1.stdout"
tf2="$td/test$N.2.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
echo "$da" >$tf1
CMD="$TRACE $SOCAT $opts $tf1!!/dev/null /dev/null,ignoreeof!!-"
printf "test $F_n $TEST... " $N
$CMD >"$tf2" 2>"$te"
if [ $? -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD"
    cat "$te"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! diff "$tf1" "$tf2" >"$tdiff"; then
    $PRINTF "$FAILED: diff:\n"
    cat "$tdiff"
   numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi # NUMCOND
esac
N=$((N+1))


NAME=GOPENPIPE
case "$TESTS" in
*%$N%*|*%functions%*|*%gopen%*|*%pipe%*|*%ignoreeof%*|*%$NAME%*)
TEST="$NAME: pipe opening with gopen for reading"
if ! eval $NUMCOND; then :; else
tp="$td/pipe$N"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD="$TRACE $SOCAT $opts $tp!!/dev/null /dev/null,ignoreeof!!$tf"
printf "test $F_n $TEST... " $N
#mknod $tp p	# no mknod p on FreeBSD
mkfifo $tp
$CMD >$tf 2>"$te" &
#($CMD >$tf 2>"$te" || rm -f "$tp") 2>/dev/null &
bg=$!	# background process id
usleep $MICROS
if [ ! -p "$tp" ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD"
    cat "$te"
    numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
#echo "$da" >"$tp"	# might hang forever
echo "$da" >"$tp" & export pid=$!; (sleep 1; kill $pid 2>/dev/null) &
# Solaris needs more time:
sleep 1
kill "$bg" 2>/dev/null; wait
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    if [ -s "$te" ]; then
	$PRINTF "$FAILED: $TRACE $SOCAT:\n"
	echo "$CMD"
	cat "$te"
    else
	$PRINTF "$FAILED: diff:\n"
	cat "$tdiff"
    fi
    numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi
wait
fi # NUMCOND
esac
N=$((N+1))


NAME=GOPENUNIXSTREAM
case "$TESTS" in
*%$N%*|*%functions%*|*%gopen%*|*%unix%*|*%listen%*|*%$NAME%*)
TEST="$NAME: GOPEN on UNIX stream socket"
if ! eval $NUMCOND; then :; else
ts="$td/test$N.socket"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da1="test$N $(date) $RANDOM"
#establish a listening unix socket in background
SRV="$TRACE $SOCAT $opts -lpserver UNIX-LISTEN:\"$ts\" PIPE"
#make a connection
CMD="$TRACE $SOCAT $opts - $ts"
$PRINTF "test $F_n $TEST... " $N
eval "$SRV 2>${te}s &"
pids=$!
waitfile "$ts"
echo "$da1" |eval "$CMD" >"${tf}1" 2>"${te}1"
if [ $? -ne 0 ]; then
    kill "$pids" 2>/dev/null
    $PRINTF "$FAILED:\n"
    echo "$SRV &"
    cat "${te}s"
    echo "$CMD"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
elif ! echo "$da1" |diff - "${tf}1" >"$tdiff"; then
    kill "$pids" 2>/dev/null
    $PRINTF "$FAILED:\n"
    echo "$SRV &"
    cat "${te}s"
    echo "$CMD"
    cat "${te}1"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
   listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi # !(rc -ne 0)
wait
fi # NUMCOND
esac
N=$((N+1))


NAME=GOPENUNIXDGRAM
case "$TESTS" in
*%$N%*|*%functions%*|*%gopen%*|*%unix%*|*%dgram%*|*%$NAME%*)
TEST="$NAME: GOPEN on UNIX datagram socket"
if ! eval $NUMCOND; then :; else
ts="$td/test$N.socket"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da1="test$N $(date) $RANDOM"
#establish a receiving unix socket in background
SRV="$TRACE $SOCAT $opts -u -lpserver UNIX-RECV:\"$ts\" file:\"$tf\",create"
#make a connection
CMD="$TRACE $SOCAT $opts -u - $ts"
$PRINTF "test $F_n $TEST... " $N
eval "$SRV 2>${te}s &"
pids=$!
waitfile "$ts"
echo "$da1" |eval "$CMD" 2>"${te}1"
waitfile -s "$tf"
if [ $? -ne 0 ]; then
    $PRINTF "$FAILED:\n"
    echo "$SRV &"
    cat "${te}s"
    echo "$CMD"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da1" |diff - "${tf}" >"$tdiff"; then
    $PRINTF "$FAILED:\n"
    echo "$SRV &"
    cat "${te}s"
    echo "$CMD"
    cat "${te}1"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi # !(rc -ne 0)
kill "$pids" 2>/dev/null; wait
fi ;; # NUMCOND
esac
N=$((N+1))


NAME=IGNOREEOF
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%ignoreeof%*|*%$NAME%*)
TEST="$NAME: ignoreeof on file"
if ! eval $NUMCOND; then :; else
ti="$td/test$N.file"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD="$TRACE $SOCAT $opts -u file:\"$ti\",ignoreeof -"
printf "test $F_n $TEST... " $N
touch "$ti"
$CMD >"$tf" 2>"$te" &
bg=$!
usleep 500000
echo "$da" >>"$ti"
sleep 1
kill $bg 2>/dev/null; wait
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: diff:\n"
    cat "$tdiff"
    listFAIL="$listFAIL $N"
    numFAIL=$((numFAIL+1))
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))

NAME=IGNOREEOF_REV
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%ignoreeof%*|*%$NAME%*)
TEST="$NAME: ignoreeof on file right-to-left"
if ! eval $NUMCOND; then :; else
ti="$td/test$N.file"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD="$SOCAT $opts -U - file:\"$ti\",ignoreeof"
printf "test $F_n $TEST... " $N
touch "$ti"
$CMD >"$tf" 2>"$te" &
bg=$!
usleep 500000
echo "$da" >>"$ti"
sleep 1
kill $bg 2>/dev/null
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: diff:\n"
    cat "$tdiff"
    listFAIL="$listFAIL $N"
    numFAIL=$((numFAIL+1))
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
wait
fi ;; # NUMCOND
esac
N=$((N+1))


NAME=EXECIGNOREEOF
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%ignoreeof%*|*%$NAME%*)
TEST="$NAME: exec against address with ignoreeof"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
CMD="$TRACE $SOCAT $opts -lf /dev/null EXEC:$TRUE /dev/null,ignoreeof"
printf "test $F_n $TEST... " $N
$CMD >"$tf" 2>"$te"
if [ -s "$te" ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))


NAME=FAKEPTY
case "$TESTS" in
*%$N%*|*%functions%*|*%pty%*|*%$NAME%*)
TEST="$NAME: generation of pty for other processes"
if ! eval $NUMCOND; then :;
elif ! testaddrs pty >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}PTY not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tt="$td/pty$N"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts pty,link=$tt pipe"
CMD2="$TRACE $SOCAT $opts - $tt,$PTYOPTS2"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
pid=$!	# background process id
waitfile "$tt"
# this hangs on HP-UX, so we use a timeout
(echo "$da"; sleep 1) |$CMD2 >$tf 2>"${te}2" &
rc2=$!
#sleep 5 && kill $rc2 2>/dev/null &
wait $rc2
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    sleep 1
    echo "$CMD2"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null; wait
fi ;; # NUMCOND, feats
esac
N=$((N+1))


NAME=O_TRUNC
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: option o-trunc"
if ! eval $NUMCOND; then :; else
ff="$td/test$N.file"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD="$TRACE $SOCAT -u $opts - open:$ff,append,o-trunc"
printf "test $F_n $TEST... " $N
rm -f $ff; $ECHO "prefix-\c" >$ff
echo "$da" |$CMD >$tf 2>"$te"
rc0=$?
if ! [ $rc0 = 0 ] ||
    ! echo "$da" |diff - $ff >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD"
    cat "$te"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))


NAME=FTRUNCATE
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: option ftruncate"
if ! eval $NUMCOND; then :; else
ff="$td/test$N.file"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD="$TRACE $SOCAT -u $opts - open:$ff,append,ftruncate=0"
printf "test $F_n $TEST... " $N
rm -f $ff; $ECHO "prefix-\c" >$ff
if ! echo "$da" |$CMD >$tf 2>"$te" ||
    ! echo "$da" |diff - $ff >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD"
    cat "$te"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))


NAME=RIGHTTOLEFT
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: unidirectional throughput from stdin to stdout, right to left"
testecho "$N" "$TEST" "stdout" "stdin" "$opts -U"
esac
N=$((N+1))


NAME=CHILDDEFAULT
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
if ! eval $NUMCOND; then :; else
TEST="$NAME: child process default properties"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
CMD="$TRACE $SOCAT $opts -u exec:$PROCAN -"
printf "test $F_n $TEST... " $N
$CMD >$tf 2>$te
MYPID=`expr "\`grep "process id =" $tf\`" : '[^0-9]*\([0-9]*\).*'`
MYPPID=`expr "\`grep "process parent id =" $tf\`" : '[^0-9]*\([0-9]*\).*'`
MYPGID=`expr "\`grep "process group id =" $tf\`" : '[^0-9]*\([0-9]*\).*'`
MYSID=`expr "\`grep "process session id =" $tf\`" : '[^0-9]*\([0-9]*\).*'`
#echo "PID=$MYPID, PPID=$MYPPID, PGID=$MYPGID, SID=$MYSID"
if [ "$MYPID" = "$MYPPID" -o "$MYPID" = "$MYPGID" -o "$MYPID" = "$MYSID" -o \
     "$MYPPID" = "$MYPGID" -o "$MYPPID" = "$MYSID" -o "$MYPGID" = "$MYSID" ];
then
    $PRINTF "$FAILED:\n"
    echo "$CMD"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))


NAME=CHILDSETSID
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: child process with setsid"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
CMD="$TRACE $SOCAT $opts -u exec:$PROCAN,setsid -"
printf "test $F_n $TEST... " $N
$CMD >$tf 2>$te
MYPID=`grep "process id =" $tf |(expr "\`cat\`" : '[^0-9]*\([0-9]*\).*')`
MYPPID=`grep "process parent id =" $tf |(expr "\`cat\`" : '[^0-9]*\([0-9]*\).*')`
MYPGID=`grep "process group id =" $tf |(expr "\`cat\`" : '[^0-9]*\([0-9]*\).*')`
MYSID=`grep "process session id =" $tf |(expr "\`cat\`" : '[^0-9]*\([0-9]*\).*')`
#$ECHO "\nPID=$MYPID, PPID=$MYPPID, PGID=$MYPGID, SID=$MYSID"
# PID, PGID, and  SID must be the same
if [ "$MYPID" = "$MYPPID" -o \
     "$MYPID" != "$MYPGID" -o "$MYPID" != "$MYSID" ];
then
    $PRINTF "$FAILED\n"
    echo "$CMD"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))


NAME=MAINSETSID
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: main process with setsid"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
CMD="$TRACE $SOCAT $opts -U -,setsid exec:$PROCAN"
printf "test $F_n $TEST... " $N
$CMD >$tf 2>$te
MYPID=`grep "process id =" $tf |(expr "\`cat\`" : '[^0-9]*\([0-9]*\).*')`
MYPPID=`grep "process parent id =" $tf |(expr "\`cat\`" : '[^0-9]*\([0-9]*\).*')`
MYPGID=`grep "process group id =" $tf |(expr "\`cat\`" : '[^0-9]*\([0-9]*\).*')`
MYSID=`grep "process session id =" $tf |(expr "\`cat\`" : '[^0-9]*\([0-9]*\).*')`
#$ECHO "\nPID=$MYPID, PPID=$MYPPID, PGID=$MYPGID, SID=$MYSID"
# PPID, PGID, and  SID must be the same
if [ "$MYPID" = "$MYPPID" -o \
     "$MYPPID" != "$MYPGID" -o "$MYPPID" != "$MYSID" ];
then
    $PRINTF "$FAILED\n"
    echo "$CMD"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))


NAME=OPENSSL_TCP4
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: openssl connect"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! type openssl >/dev/null 2>&1; then
    $PRINTF "test $F_n $TEST... ${YELLOW}openssl executable not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD2="$TRACE $SOCAT $opts exec:'openssl s_server -accept "$PORT" -quiet -cert testsrv.pem' pipe"
#! CMD="$TRACE $SOCAT $opts - openssl:$LOCALHOST:$PORT"
CMD="$TRACE $SOCAT $opts - openssl:$LOCALHOST:$PORT,pf=ip4,verify=0,$SOCAT_EGD"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
# this might timeout when openssl opens tcp46 port like " :::$PORT"
waittcp4port $PORT
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null; wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=OPENSSLLISTEN_TCP4
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: openssl listen"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD2="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,pf=ip4,reuseaddr,$SOCAT_EGD,cert=testsrv.crt,key=testsrv.key,verify=0 pipe"
CMD="$TRACE $SOCAT $opts - openssl:$LOCALHOST:$PORT,pf=ip4,verify=0,$SOCAT_EGD"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp4port $PORT
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=OPENSSLLISTEN_TCP6
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%$NAME%*)
TEST="$NAME: openssl listen"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD2="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,pf=ip6,reuseaddr,$SOCAT_EGD,cert=testsrv.crt,key=testsrv.key,verify=0 pipe"
CMD="$TRACE $SOCAT $opts - openssl:$LOCALHOST6:$PORT,verify=0,$SOCAT_EGD"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp6port $PORT
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


while read NAMEKEYW FEAT RUNS TESTTMPL PEERTMPL WAITTMPL; do
if [ -z "$NAMEKEYW" ] || [[ "$NAMEKEYW" == \#* ]]; then continue; fi

export ts="$td/test$N.socket"
WAITTMPL="$(echo "$WAITTMPL" |sed -e 's/\040/ /g')"
TESTADDR=$(eval echo $TESTTMPL)
PEERADDR=$(eval echo $PEERTMPL)
WAITCMD=$(eval echo $WAITTMPL)
TESTKEYW=${TESTADDR%%:*}

# does our address implementation support halfclose?
NAME=${NAMEKEYW}_HALFCLOSE
case "$TESTS" in
*%$N%*|*%functions%*|*%$FEAT%*|*%socket%*|*%halfclose%*|*%$NAME%*)
TEST="$NAME: $TESTKEYW half close"
# have a "peer" socat "peer" that executes "$OD_C" and see if EOF on the
# connecting socat  brings the result of od
if ! eval $NUMCOND; then :;
elif [ "$FEAT" != ',' ] && ! testaddrs "$FEAT" >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$FEAT not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! runs$RUNS; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$RUNS not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD2="$TRACE $SOCAT $opts \"$PEERADDR\" EXEC:'$OD_C'"
CMD="$TRACE $SOCAT -T1 $opts - $TESTADDR"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
$WAITCMD
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |$OD_C |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

done <<<"
UNIXCONNECT      ,       unix UNIX-CONNECT:\$ts UNIX-LISTEN:\$ts waitfile\040\$ts
UNIXCLIENT       ,       unix UNIX-CLIENT:\$ts UNIX-LISTEN:\$ts waitfile\040\$ts
GOPEN_UNIXSTREAM ,       unix GOPEN:\$ts UNIX-LISTEN:\$ts waitfile\040\$ts
UNIXLISTEN       ,       unix UNIX-LISTEN:\$ts UNIX-CONNECT:\$ts,retry=3 sleep\040\1
TCP4CONNECT      ,       tcp4 TCP4-CONNECT:\$LOCALHOST:\$PORT TCP4-LISTEN:\$PORT waittcp4port\040\$PORT
TCP4LISTEN       ,       tcp4 TCP4-LISTEN:\$PORT TCP4-CONNECT:\$LOCALHOST:\$PORT,retry=3
TCP6CONNECT      ,       tcp6 TCP6-CONNECT:\$LOCALHOST6:\$PORT TCP6-LISTEN:\$PORT waittcp6port\040\$PORT
TCP6LISTEN       ,       tcp6 TCP6-LISTEN:\$PORT TCP6-CONNECT:\$LOCALHOST6:\$PORT,retry=3
OPENSSL4CLIENT   OPENSSL tcp4 OPENSSL:\$LOCALHOST:\$PORT,verify=0 OPENSSL-LISTEN:\$PORT,$SOCAT_EGD,cert=testsrv.crt,key=testsrv.key,verify=0 waittcp4port\040\$PORT
OPENSSL4SERVER   OPENSSL tcp4 OPENSSL-LISTEN:\$PORT,$SOCAT_EGD,cert=testsrv.crt,key=testsrv.key,verify=0 OPENSSL:\$LOCALHOST:\$PORT,verify=0,retry=3
OPENSSL6CLIENT   OPENSSL tcp6 OPENSSL:\$LOCALHOST6:\$PORT,pf=ip6,verify=0 OPENSSL-LISTEN:\$PORT,pf=ip6,$SOCAT_EGD,cert=testsrv.crt,key=testsrv.key,verify=0 waittcp6port\040\$PORT
OPENSSL6SERVER   OPENSSL tcp6 OPENSSL-LISTEN:\$PORT,pf=ip6,$SOCAT_EGD,cert=testsrv.crt,key=testsrv.key,verify=0 OPENSSL:\$LOCALHOST6:\$PORT,pf=ip6,verify=0,retry=3
"


NAME=OPENSSL_SERVERAUTH
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: openssl server authentication"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
gentestcert testcli
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD2="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,reuseaddr,$SOCAT_EGD,cert=testsrv.crt,key=testsrv.key,verify=0 pipe"
CMD="$TRACE $SOCAT $opts - openssl:$LOCALHOST:$PORT,verify=1,cafile=testsrv.crt,$SOCAT_EGD"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp4port $PORT
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=OPENSSL_CLIENTAUTH
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: openssl client authentication"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
gentestcert testcli
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD2="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,reuseaddr,verify=1,cert=testsrv.crt,key=testsrv.key,cafile=testcli.crt,$SOCAT_EGD pipe"
CMD="$TRACE $SOCAT $opts - openssl:$LOCALHOST:$PORT,verify=0,cert=testcli.crt,key=testcli.key,$SOCAT_EGD"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp4port $PORT
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=OPENSSL_FIPS_BOTHAUTH
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%fips%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: OpenSSL+FIPS client and server authentication"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testoptions fips >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL/FIPS not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
OPENSSL_FIPS=1 gentestcert testsrvfips
OPENSSL_FIPS=1 gentestcert testclifips
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD2="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,reuseaddr,fips,$SOCAT_EGD,cert=testsrvfips.crt,key=testsrvfips.key,cafile=testclifips.crt pipe"
CMD="$TRACE $SOCAT $opts - openssl:$LOCALHOST:$PORT,fips,verify=1,cert=testclifips.crt,key=testclifips.key,cafile=testsrvfips.crt,$SOCAT_EGD"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp4port $PORT
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=OPENSSL_COMPRESS
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: OpenSSL compression"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testoptions openssl-compress >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL compression option not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
    gentestcert testsrv
    printf "test $F_n $TEST... " $N
    tf="$td/test$N.stdout"
    te="$td/test$N.stderr"
    tdiff="$td/test$N.diff"
    da="test$N $(date) $RANDOM"
    success=yes
    for srccompr in '' compress=auto compress=none; do
        for dstcompr in '' compress=auto compress=none; do
            CMD2="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,pf=ip4,reuseaddr,$SOCAT_EGD,cert=testsrv.crt,key=testsrv.key,verify=0,$dstcompr pipe"
            CMD="$TRACE $SOCAT $opts - openssl:$LOCALHOST:$PORT,pf=ip4,verify=0,$SOCAT_EGD,$srccompr"
            eval "$CMD2 2>\"${te}1\" &"
            pid=$! # background process id
            waittcp4port $PORT
            echo "$da" | $CMD >$tf 2>"${te}2"
            kill $pid 2>/dev/null
            if ! echo "$da" |diff - "$tf" >"$tdiff"; then
                success=
                break
            fi
        done
    done
    if test -z "$success"; then
        $PRINTF "$FAILED: $TRACE $SOCAT:\n"
        echo "$CMD2 &"
        echo "$CMD"
        cat "${te}1"
        cat "${te}2"
        cat "$tdiff"
        numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
    else
        $PRINTF "$OK\n"
        if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
        numOK=$((numOK+1))
    fi
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=SOCKS4CONNECT_TCP4
case "$TESTS" in
*%$N%*|*%functions%*|*%socks%*|*%socks4%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: socks4 connect over TCP/IPv4"
if ! eval $NUMCOND; then :;
elif ! testaddrs socks4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}SOCKS4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"; da="$da$($ECHO '\r')"
# we have a normal tcp echo listening - so the socks header must appear in answer
CMD2="$TRACE $SOCAT $opts tcp4-l:$PORT,reuseaddr exec:\"./socks4echo.sh\""
CMD="$TRACE $SOCAT $opts - socks4:$LOCALHOST:32.98.76.54:32109,pf=ip4,socksport=$PORT",socksuser="nobody"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp4port $PORT 1
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=SOCKS4CONNECT_TCP6
case "$TESTS" in
*%$N%*|*%functions%*|*%socks%*|*%socks4%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%$NAME%*)
TEST="$NAME: socks4 connect over TCP/IPv6"
if ! eval $NUMCOND; then :;
elif ! testaddrs socks4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}SOCKS4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"; da="$da$($ECHO '\r')"
# we have a normal tcp echo listening - so the socks header must appear in answer
CMD2="$TRACE $SOCAT $opts tcp6-l:$PORT,reuseaddr exec:\"./socks4echo.sh\""
CMD="$TRACE $SOCAT $opts - socks4:$LOCALHOST6:32.98.76.54:32109,socksport=$PORT",socksuser="nobody"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp6port $PORT 1
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=SOCKS4ACONNECT_TCP4
case "$TESTS" in
*%$N%*|*%functions%*|*%socks%*|*%socks4a%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: socks4a connect over TCP/IPv4"
if ! eval $NUMCOND; then :;
elif ! testaddrs socks4a >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}SOCKS4A not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"; da="$da$($ECHO '\r')"
# we have a normal tcp echo listening - so the socks header must appear in answer
CMD2="$TRACE $SOCAT $opts tcp4-l:$PORT,reuseaddr exec:\"./socks4a-echo.sh\""
CMD="$TRACE $SOCAT $opts - socks4a:$LOCALHOST:localhost:32109,pf=ip4,socksport=$PORT",socksuser="nobody"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp4port $PORT 1
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=SOCKS4ACONNECT_TCP6
case "$TESTS" in
*%$N%*|*%functions%*|*%socks%*|*%socks4a%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%$NAME%*)
TEST="$NAME: socks4a connect over TCP/IPv6"
if ! eval $NUMCOND; then :;
elif ! testaddrs socks4a >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}SOCKS4A not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"; da="$da$($ECHO '\r')"
# we have a normal tcp echo listening - so the socks header must appear in answer
CMD2="$TRACE $SOCAT $opts tcp6-l:$PORT,reuseaddr exec:\"./socks4a-echo.sh\""
CMD="$TRACE $SOCAT $opts - socks4a:$LOCALHOST6:localhost:32109,socksport=$PORT",socksuser="nobody"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp6port $PORT 1
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=PROXYCONNECT_TCP4
case "$TESTS" in
*%$N%*|*%functions%*|*%proxyconnect%*|*%proxy%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: proxy connect over TCP/IPv4"
if ! eval $NUMCOND; then :;
elif ! testaddrs proxy >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}PROXY not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ts="$td/test$N.sh"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"; da="$da$($ECHO '\r')"
#CMD2="$TRACE $SOCAT tcp4-l:$PORT,crlf SYSTEM:\"read; read; $ECHO \\\"HTTP/1.0 200 OK\n\\\"; cat\""
CMD2="$TRACE $SOCAT $opts tcp4-l:$PORT,reuseaddr,crlf exec:\"/bin/bash proxyecho.sh\""
CMD="$TRACE $SOCAT $opts - proxy:$LOCALHOST:127.0.0.1:1000,pf=ip4,proxyport=$PORT"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}2\" &"
pid=$!	# background process id
waittcp4port $PORT 1
echo "$da" |$CMD >"$tf" 2>"${te}1"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=PROXYCONNECT_TCP6
case "$TESTS" in
*%$N%*|*%functions%*|*%proxyconnect%*|*%proxy%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%$NAME%*)
TEST="$NAME: proxy connect over TCP/IPv6"
if ! eval $NUMCOND; then :;
elif ! testaddrs proxy >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}PROXY not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ts="$td/test$N.sh"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"; da="$da$($ECHO '\r')"
#CMD2="$TRACE $SOCAT $opts tcp6-l:$PORT,crlf SYSTEM:\"read; read; $ECHO \\\"HTTP/1.0 200 OK\n\\\"; cat\""
CMD2="$TRACE $SOCAT $opts tcp6-l:$PORT,reuseaddr,crlf exec:\"/bin/bash proxyecho.sh\""
CMD="$TRACE $SOCAT $opts - proxy:$LOCALHOST6:127.0.0.1:1000,proxyport=$PORT"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}2\" &"
pid=$!	# background process id
waittcp6port $PORT 1
echo "$da" |$CMD >"$tf" 2>"${te}1"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=TCP4NOFORK
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: echo via connection to TCP V4 socket with nofork'ed exec"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="127.0.0.1:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP4-LISTEN:$tsl,reuseaddr exec:$CAT,nofork"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP4:$ts"
printf "test $F_n $TEST... " $N
#$CMD1 >"$tf" 2>"${te}1" &
$CMD1 >/dev/null 2>"${te}1" &
waittcp4port $tsl
#usleep $MICROS
echo "$da" |$CMD2 >"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=EXECCATNOFORK
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: simple echo via exec of cat with nofork"
testecho "$N" "$TEST" "" "exec:$CAT,nofork" "$opts"
esac
N=$((N+1))


NAME=SYSTEMCATNOFORK
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: simple echo via system() of cat with nofork"
testecho "$N" "$TEST" "" "system:$CAT,nofork" "$opts"
esac
N=$((N+1))


NAME=NOFORKSETSID
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: simple echo via exec() of cat with nofork and setsid"
testecho "$N" "$TEST" "" "system:$CAT,nofork,setsid" "$opts"
esac
N=$((N+1))

#==============================================================================
#TEST="$NAME: echo via 'connection' to UDP V4 socket"
#if ! eval $NUMCOND; then :; else
#tf="$td/file$N"
#tsl=65534
#ts="127.0.0.1:$tsl"
#da="test$N $(date) $RANDOM"
#$TRACE $SOCAT UDP-listen:$tsl PIPE &
#sleep 2
#echo "$da" |$TRACE $SOCAT stdin!!stdout UDP:$ts >"$tf"
#if [ $? -eq 0 ] && echo "$da" |diff "$tf" -; then
#   $ECHO "... test $N succeeded"
#   numOK=$((numOK+1))
#else
#   $ECHO "*** test $N $FAILED"
#    numFAIL=$((numFAIL+1))
#    listFAIL="$listFAIL $N"
#fi
#fi ;; # NUMCOND
#N=$((N+1))
#==============================================================================
# TEST 4 - simple echo via new file
#if ! eval $NUMCOND; then :; else
#N=4
#tf="$td/file$N"
#tp="$td/pipe$N"
#da="test$N $(date) $RANDOM"
#rm -f "$tf.tmp"
#echo "$da" |$TRACE $SOCAT - FILE:$tf.tmp,ignoreeof >"$tf"
#if [ $? -eq 0 ] && echo "$da" |diff "$tf" -; then
#   $ECHO "... test $N succeeded"
#   numOK=$((numOK+1))
#else
#   $ECHO "*** test $N $FAILED"
#   numFAIL=$((numFAIL+1))
#    listFAIL="$listFAIL $N"
#fi
#fi ;; # NUMCOND

#==============================================================================

NAME=TOTALTIMEOUT
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%timeout%*|*%$NAME%*)
TEST="$NAME: socat inactivity timeout"
if ! eval $NUMCOND; then :; else
#set -vx
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"; da="$da$($ECHO '\r')"
CMD2="$TRACE $SOCAT $opts -T 1 tcp4-listen:$PORT,reuseaddr pipe"
CMD="$TRACE $SOCAT $opts - tcp4-connect:$LOCALHOST:$PORT"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>${te}1 &"
pid=$!	# background process id
waittcp4port $PORT 1
(echo "$da"; sleep 2; echo X) |$CMD >"$tf" 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
#set +vx
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=IGNOREEOF+TOTALTIMEOUT
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%timeout%*|*%ignoreeof%*|*%$NAME%*)
TEST="$NAME: ignoreeof and inactivity timeout"
if ! eval $NUMCOND; then :; else
#set -vx
ti="$td/test$N.file"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD="$TRACE $SOCAT $opts -T 2 -u file:\"$ti\",ignoreeof -"
printf "test $F_n $TEST... " $N
touch "$ti"
$CMD >"$tf" 2>"$te" &
bg=$!	# background process id
psleep 0.5
echo "$da" >>"$ti"
sleep 4
echo X >>"$ti"
sleep 1
kill $bg 2>/dev/null
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD &"
    cat "$te"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "$te"; fi
   numOK=$((numOK+1))
fi
wait
fi ;; # NUMCOND, feats
esac
N=$((N+1))


NAME=PROXY2SPACES
case "$TESTS" in
*%$N%*|*%functions%*|*%proxy%*|*%$NAME%*)
TEST="$NAME: proxy connect accepts status with multiple spaces"
if ! eval $NUMCOND; then :;
elif ! testaddrs proxy >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}PROXY not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ts="$td/test$N.sh"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"; da="$da$($ECHO '\r')"
#CMD2="$TRACE $SOCAT $opts tcp-l:$PORT,crlf SYSTEM:\"read; read; $ECHO \\\"HTTP/1.0 200 OK\n\\\"; cat\""
CMD2="$TRACE $SOCAT $opts tcp4-l:$PORT,reuseaddr,crlf exec:\"/bin/bash proxyecho.sh -w 2\""
CMD="$TRACE $SOCAT $opts - proxy:$LOCALHOST:127.0.0.1:1000,pf=ip4,proxyport=$PORT"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp4port $PORT 1
echo "$da" |$CMD >"$tf" 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=BUG-UNISTDIO
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: for bug with address options on both stdin/out in unidirectional mode"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
ff="$td/file$N"
printf "test $F_n $TEST... " $N
>"$ff"
#$TRACE $SOCAT $opts -u /dev/null -,setlk <"$ff"  2>"$te"
CMD="$TRACE $SOCAT $opts -u /dev/null -,setlk"
$CMD <"$ff"  2>"$te"
if [ "$?" -eq 0 ]; then
    $PRINTF "$OK\n"
   numOK=$((numOK+1))
else
    if [ "$UNAME" = "Linux" ]; then
	$PRINTF "$FAILED\n"
	echo "$CMD"
	cat "$te"
	numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
    else
	$PRINTF "${YELLOW}failed (don't care)${NORMAL}\n"
	numCANT=$((numCANT+1))
    fi
fi
fi ;; # NUMCOND
esac
N=$((N+1))


NAME=SINGLEEXECOUTSOCKETPAIR
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: inheritance of stdout to single exec with socketpair"
testecho "$N" "$TEST" "-!!exec:cat" "" "$opts" 1
esac
N=$((N+1))

NAME=SINGLEEXECOUTPIPE
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: inheritance of stdout to single exec with pipe"
testecho "$N" "$TEST" "-!!exec:cat,pipes" "" "$opts" 1
esac
N=$((N+1))

NAME=SINGLEEXECOUTPTY
case "$TESTS" in
*%$N%*|*%functions%*|*%pty%*|*%$NAME%*)
TEST="$NAME: inheritance of stdout to single exec with pty"
if ! eval $NUMCOND; then :;
elif ! testaddrs pty >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}PTY not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testecho "$N" "$TEST" "-!!exec:cat,pty,raw" "" "$opts" 1
fi ;; # NUMCOND, feats
esac
N=$((N+1))

NAME=SINGLEEXECINSOCKETPAIR
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: inheritance of stdin to single exec with socketpair"
testecho "$N" "$TEST" "exec:cat!!-" "" "$opts"
esac
N=$((N+1))

NAME=SINGLEEXECINPIPE
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: inheritance of stdin to single exec with pipe"
testecho "$N" "$TEST" "exec:cat,pipes!!-" "" "$opts"
esac
N=$((N+1))

NAME=SINGLEEXECINPTYDELAY
case "$TESTS" in
*%$N%*|*%functions%*|*%pty%*|*%$NAME%*)
TEST="$NAME: inheritance of stdin to single exec with pty, with delay"
if ! eval $NUMCOND; then :;
elif ! testaddrs pty >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}PTY not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testecho "$N" "$TEST" "exec:cat,pty,raw!!-" "" "$opts" $MISCDELAY
fi ;; # NUMCOND, feats
esac
N=$((N+1))

NAME=SINGLEEXECINPTY
case "$TESTS" in
*%$N%*|*%functions%*|*%pty%*|*%$NAME%*)
TEST="$NAME: inheritance of stdin to single exec with pty"
if ! eval $NUMCOND; then :;
elif ! testaddrs pty >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}PTY not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testecho "$N" "$TEST" "exec:cat,pty,raw!!-" "" "$opts"
fi ;; # NUMCOND, feats
esac
N=$((N+1))


NAME=READLINE
#set -vx
case "$TESTS" in
*%$N%*|*%functions%*|*%pty%*|*%readline%*|*%$NAME%*)
TEST="$NAME: readline with password and sigint"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs readline pty); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
SAVETERM="$TERM"; TERM=	# 'cause konsole might print controls even in raw
SAVEMICS=$MICROS
#MICROS=2000000
ts="$td/test$N.sh"
to="$td/test$N.stdout"
tpi="$td/test$N.inpipe"
tpo="$td/test$N.outpipe"
te="$td/test$N.stderr"
tr="$td/test$N.ref"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"; da="$da$($ECHO '\r')"
# the feature that we really want to test is in the readline.sh script:
CMD="$TRACE $SOCAT $opts -t1 open:$tpi,nonblock!!open:$tpo exec:\"./readline.sh -nh ./readline-test.sh\",pty,ctty,setsid,raw,echo=0,isig"
#echo "$CMD" >"$ts"
#chmod a+x "$ts"
printf "test $F_n $TEST... " $N
rm -f "$tpi" "$tpo"
mkfifo "$tpi"
touch "$tpo"
#
# during development of this test, the following command line succeeded:
# (sleep 1; $ECHO "user\n\c"; sleep 1; $ECHO "password\c"; sleep 1; $ECHO "\n\c"; sleep 1; $ECHO "test 1\n\c"; sleep 1; $ECHO "\003\c"; sleep 1; $ECHO "test 2\n\c"; sleep 1; $ECHO "exit\n\c"; sleep 1) |$TRACE $SOCAT -d -d -d -d -lf/tmp/gerhard/debug1 -v -x - exec:'./readline.sh ./readline-test.sh',pty,ctty,setsid,raw,echo=0,isig
#
PATH=${SOCAT%socat}:$PATH eval "$CMD 2>$te &"
pid=$!	# background process id
usleep $MICROS

(
usleep $((3*MICROS))
$ECHO "user\n\c"
usleep $MICROS
$ECHO "password\c"
usleep $MICROS
$ECHO "\n\c"
usleep $MICROS
$ECHO "test 1\n\c"
usleep $MICROS
$ECHO "\003\c"
usleep $MICROS
$ECHO "test 2\n\c"
usleep $MICROS
$ECHO "exit\n\c"
usleep $MICROS
) >"$tpi"

cat >$tr <<EOF
readline feature test program
Authentication required
Username: user
Password: 
prog> test 1
executing test 1
prog> ./readline-test.sh got SIGINT
test 2
executing test 2
prog> exit
EOF

#0 if ! sed 's/.*\r//g' "$tpo" |diff -q "$tr" - >/dev/null 2>&1; then
#0 if ! sed 's/.*'"$($ECHO '\r\c')"'/</g' "$tpo" |diff -q "$tr" - >/dev/null 2>&1; then
wait
if ! tr "$($ECHO '\r \c')" "% " <$tpo |sed 's/%$//g' |sed 's/.*%//g' |diff "$tr" - >"$tdiff" 2>&1; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD"
    cat "$te"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null	# necc on OpenBSD
wait
MICROS=$SAVEMICS
TERM="$SAVETERM"
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=GENDERCHANGER
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: TCP4 \"gender changer\""
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
# this is the server in the protected network that we want to reach
CMD1="$TRACE $SOCAT -lpserver $opts tcp4-l:$PORT,reuseaddr,bind=$LOCALHOST echo"
# this is the double client in the protected network
CMD2="$TRACE $SOCAT -lp2client $opts tcp4:$LOCALHOST:$((PORT+1)),retry=10,interval=1 tcp4:$LOCALHOST:$PORT"
# this is the double server in the outside network
CMD3="$TRACE $SOCAT -lp2server $opts tcp4-l:$((PORT+2)),reuseaddr,bind=$LOCALHOST tcp4-l:$((PORT+1)),reuseaddr,bind=$LOCALHOST"
# this is the outside client that wants to use the protected server
CMD4="$TRACE $SOCAT -lpclient $opts -t1 - tcp4:$LOCALHOST:$((PORT+2))"
printf "test $F_n $TEST... " $N
eval "$CMD1 2>${te}1 &"
pid1=$!
eval "$CMD2 2>${te}2 &"
pid2=$!
eval "$CMD3 2>${te}3 &"
pid3=$!
waittcp4port $PORT 1 &&
waittcp4port $((PORT+2)) 1
sleep 1
echo "$da" |$CMD4 >$tf 2>"${te}4"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    echo "$CMD2 &"
    echo "$CMD3 &"
    echo "$CMD4"
    cat "${te}1" "${te}2" "${te}3" "${te}4"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}1" "${te}2" "${te}3" "${te}4"; fi
   numOK=$((numOK+1))
fi
kill $pid1 $pid2 $pid3 $pid4 2>/dev/null
wait
fi ;; # NUMCOND
esac
PORT=$((PORT+3))
N=$((N+1))


#!
#PORT=10000
#!
NAME=OUTBOUNDIN
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%proxy%*|*%$NAME%*)
TEST="$NAME: gender changer via SSL through HTTP proxy, oneshot"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs openssl proxy); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat" |tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
gentestcert testcli
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
# this is the server in the protected network that we want to reach
CMD1="$TRACE $SOCAT $opts -lpserver tcp4-l:$PORT,reuseaddr,bind=$LOCALHOST echo"
# this is the proxy in the protected network that provides a way out
CMD2="$TRACE $SOCAT $opts -lpproxy tcp4-l:$((PORT+1)),reuseaddr,bind=$LOCALHOST,fork exec:./proxy.sh"
# this is our proxy connect wrapper in the protected network
CMD3="$TRACE $SOCAT $opts -lpwrapper tcp4-l:$((PORT+2)),reuseaddr,bind=$LOCALHOST,fork proxy:$LOCALHOST:$LOCALHOST:$((PORT+3)),pf=ip4,proxyport=$((PORT+1)),resolve"
# this is our double client in the protected network using SSL
#CMD4="$TRACE $SOCAT $opts -lp2client ssl:$LOCALHOST:$((PORT+2)),pf=ip4,retry=10,interval=1,cert=testcli.pem,cafile=testsrv.crt,$SOCAT_EGD tcp4:$LOCALHOST:$PORT"
CMD4="$TRACE $SOCAT $opts -lp2client ssl:$LOCALHOST:$((PORT+2)),pf=ip4,cert=testcli.pem,cafile=testsrv.crt,$SOCAT_EGD tcp4:$LOCALHOST:$PORT"
# this is the double server in the outside network
CMD5="$TRACE $SOCAT $opts -lp2server -t1 tcp4-l:$((PORT+4)),reuseaddr,bind=$LOCALHOST ssl-l:$((PORT+3)),pf=ip4,reuseaddr,bind=$LOCALHOST,$SOCAT_EGD,cert=testsrv.pem,cafile=testcli.crt"
# this is the outside client that wants to use the protected server
CMD6="$TRACE $SOCAT $opts -lpclient -t5 - tcp4:$LOCALHOST:$((PORT+4))"
printf "test $F_n $TEST... " $N
eval "$CMD1 2>${te}1 &"
pid1=$!
eval "$CMD2 2>${te}2 &"
pid2=$!
eval "$CMD3 2>${te}3 &"
pid3=$!
waittcp4port $PORT 1       || $PRINTF "$FAILED: port $PORT\n" >&2 </dev/null
waittcp4port $((PORT+1)) 1 || $PRINTF "$FAILED: port $((PORT+1))\n" >&2 </dev/null
waittcp4port $((PORT+2)) 1 || $PRINTF "$FAILED: port $((PORT+2))\n" >&2 </dev/null
eval "$CMD5 2>${te}5 &"
pid5=$!
waittcp4port $((PORT+4)) 1 || $PRINTF "$FAILED: port $((PORT+4))\n" >&2 </dev/null
echo "$da" |$CMD6 >$tf 2>"${te}6" &
pid6=$!
waittcp4port $((PORT+3)) 1 || $PRINTF "$FAILED: port $((PORT+3))\n" >&2 </dev/null
eval "$CMD4 2>${te}4 &"
pid4=$!
wait $pid6
if ! (echo "$da"; sleep 2) |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    cat "${te}1"
    echo "$CMD2 &"
    cat "${te}2"
    echo "$CMD3 &"
    cat "${te}3"
    echo "$CMD5 &"
    cat "${te}5"
    echo "$CMD6"
    cat "${te}6"
    echo "$CMD4 &"
    cat "${te}4"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}1" "${te}2" "${te}3" "${te}4" "${te}5" "${te}6"; fi
   numOK=$((numOK+1))
fi
kill $pid1 $pid2 $pid3 $pid4 $pid5 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+5))
N=$((N+1))


# test the TCP gender changer with almost production requirements: a double
# client repeatedly tries to connect to a double server via SSL through an HTTP
# proxy. the double servers SSL port becomes active for one connection only
# after a (real) client has connected to its TCP port. when the double client
# succeeded to establish an SSL connection, it connects with its second client
# side to the specified (protected) server. all three consecutive connections
# must function for full success of this test.
PORT=$((RANDOM+16184))
#!
NAME=INTRANETRIPPER
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%proxy%*|*%$NAME%*)
TEST="$NAME: gender changer via SSL through HTTP proxy, daemons"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs openssl proxy); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
gentestcert testcli
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da1="test$N.1 $(date) $RANDOM"
da2="test$N.2 $(date) $RANDOM"
da3="test$N.3 $(date) $RANDOM"
# this is the server in the protected network that we want to reach
CMD1="$TRACE $SOCAT $opts -lpserver -t1 tcp4-l:$PORT,reuseaddr,bind=$LOCALHOST,fork echo"
# this is the proxy in the protected network that provides a way out
# note: the proxy.sh script starts one or two more socat processes without
# setting the program name 
CMD2="$TRACE $SOCAT $opts -lpproxy -t1 tcp4-l:$((PORT+1)),reuseaddr,bind=$LOCALHOST,fork exec:./proxy.sh"
# this is our proxy connect wrapper in the protected network
CMD3="$TRACE $SOCAT $opts -lpwrapper -t3 tcp4-l:$((PORT+2)),reuseaddr,bind=$LOCALHOST,fork proxy:$LOCALHOST:$LOCALHOST:$((PORT+3)),pf=ip4,proxyport=$((PORT+1)),resolve"
# this is our double client in the protected network using SSL
CMD4="$TRACE $SOCAT $opts -lp2client -t3 ssl:$LOCALHOST:$((PORT+2)),retry=10,interval=1,cert=testcli.pem,cafile=testsrv.crt,verify,fork,$SOCAT_EGD tcp4:$LOCALHOST:$PORT,forever,interval=0.1"
# this is the double server in the outside network
CMD5="$TRACE $SOCAT $opts -lp2server -t4 tcp4-l:$((PORT+4)),reuseaddr,bind=$LOCALHOST,backlog=3,fork ssl-l:$((PORT+3)),pf=ip4,reuseaddr,bind=$LOCALHOST,$SOCAT_EGD,cert=testsrv.pem,cafile=testcli.crt,retry=20,interval=0.5"
# this is the outside client that wants to use the protected server
CMD6="$TRACE $SOCAT $opts -lpclient -t6 - tcp4:$LOCALHOST:$((PORT+4)),retry=3"
printf "test $F_n $TEST... " $N
# start the intranet infrastructure
eval "$CMD1 2>\"${te}1\" &"
pid1=$!
eval "$CMD2 2>\"${te}2\" &"
pid2=$!
waittcp4port $PORT 1       || $PRINTF "$FAILED: port $PORT\n" >&2 </dev/null
waittcp4port $((PORT+1)) 1 || $PRINTF "$FAILED: port $((PORT+1))\n" >&2 </dev/null
# initiate our internal measures
eval "$CMD3 2>\"${te}3\" &"
pid3=$!
eval "$CMD4 2>\"${te}4\" &"
pid4=$!
waittcp4port $((PORT+2)) 1 || $PRINTF "$FAILED: port $((PORT+2))\n" >&2 </dev/null
# now we start the external daemon
eval "$CMD5 2>\"${te}5\" &"
pid5=$!
waittcp4port $((PORT+4)) 1 || $PRINTF "$FAILED: port $((PORT+4))\n" >&2 </dev/null
# and this is the outside client:
echo "$da1" |$CMD6 >${tf}_1 2>"${te}6_1" &
pid6_1=$!
echo "$da2" |$CMD6 >${tf}_2 2>"${te}6_2" &
pid6_2=$!
echo "$da3" |$CMD6 >${tf}_3 2>"${te}6_3" &
pid6_3=$!
wait $pid6_1 $pid6_2 $pid6_3
#
(echo "$da1"; sleep 2) |diff - "${tf}_1" >"${tdiff}1"
(echo "$da2"; sleep 2) |diff - "${tf}_2" >"${tdiff}2"
(echo "$da3"; sleep 2) |diff - "${tf}_3" >"${tdiff}3"
if test -s "${tdiff}1" -o -s "${tdiff}2" -o -s "${tdiff}3"; then
  # FAILED only when none of the three transfers succeeded
  if test -s "${tdiff}1" -a -s "${tdiff}2" -a -s "${tdiff}3"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    cat "${te}1"
    echo "$CMD2 &"
    cat "${te}2"
    echo "$CMD3 &"
    cat "${te}3"
    echo "$CMD4 &"
    cat "${te}4"
    echo "$CMD5 &"
    cat "${te}5"
    echo "$CMD6 &"
    cat "${te}6_1"
    cat "${tdiff}1"
    echo "$CMD6 &"
    cat "${te}6_2"
    cat "${tdiff}2"
    echo "$CMD6 &"
    cat "${te}6_3"
    cat "${tdiff}3"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
  else
    $PRINTF "$OK ${YELLOW}(partial failure)${NORMAL}\n"
    if [ -n "$debug" ]; then cat "${te}1" "${te}2" "${te}3" "${te}4" "${te}5" ${te}6*; fi
    numOK=$((numOK+1))
  fi
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}1" "${te}2" "${te}3" "${te}4" "${te}5" ${te}6*; fi
   numOK=$((numOK+1))
fi
kill $pid1 $pid2 $pid3 $pid4 $pid5 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+5))
N=$((N+1))


# let us test the security features with -s, retry, and fork
# method: first test without security feature if it works
#   then try with security feature, must fail

# test the security features of a server address
testserversec () {
    local N="$1"
    local title="$2"
    local opts="$3"
    local arg1="$4"	# the server address
    local secopt0="$5"	# option without security for server, mostly empty
    local secopt1="$6"	# the security option for server, to be tested
    local arg2="$7"	# the client address
    local ipvers="$8"	# IP version, for check of listen port
    local proto="$9"	# protocol, for check of listen port
    local port="${10}"	# start client when this port is listening
    local expect="${11}"	# expected behaviour of client: 0..empty output; -1..error
    local T="${12}";	[ -z "$T" ] && T=0
    local tf="$td/test$N.stdout"
    local te="$td/test$N.stderr"
    local tdiff1="$td/test$N.diff1"
    local tdiff2="$td/test$N.diff2"
    local da="test$N.1 $(date) $RANDOM"
    local stat result

    $PRINTF "test $F_n %s... " $N "$title"
    # first: without security
    # start server
    $TRACE $SOCAT $opts "$arg1,$secopt0" echo 2>"${te}1" &
    spid=$!
    if [ "$port" ] && ! wait${proto}${ipvers}port $port 1; then
	kill $spid 2>/dev/null
	$PRINTF "$NO_RESULT (ph.1 server not working):\n"
	echo "$TRACE $SOCAT $opts \"$arg1,$secopt0\" echo &"
	cat "${te}1"
	numCANT=$((numCANT+1))
	wait; return
    fi
    # now use client
    (echo "$da"; sleep $T) |$TRACE $SOCAT $opts - "$arg2" >"$tf" 2>"${te}2"
    stat="$?"
    kill $spid 2>/dev/null
    #killall $TRACE $SOCAT 2>/dev/null
    if [ "$stat" != 0 ]; then
	$PRINTF "$NO_RESULT (ph.1 function fails): $TRACE $SOCAT:\n"
	echo "$TRACE $SOCAT $opts \"$arg1,$secopt0\" echo &"
	cat "${te}1"
	echo "$TRACE $SOCAT $opts - \"$arg2\""
	cat "${te}2"
	numCANT=$((numCANT+1))
	wait; return
    elif echo "$da" |diff - "$tf" >"$tdiff1" 2>&1; then
	:	# function without security is ok, go on
    else
	$PRINTF "$NO_RESULT (ph.1 function fails): diff:\n"
	echo "$TRACE $SOCAT $opts $arg1,$secopt0 echo &"
	cat "${te}1"
	echo "$TRACE $SOCAT $opts - $arg2"
	cat "${te}2"
	cat "$tdiff1"
	numCANT=$((numCANT+1))
	wait; return
    fi

    # then: with security
    if [ "$port" ] && ! wait${proto}${ipvers}port $port 0; then
	$PRINTF "$NO_RESULT (ph.1 port remains in use)\n"
	numCANT=$((numCANT+1))
	wait; return
    fi
    wait

#set -vx
    # assemble address w/ security option; on dual, take read part:
    case "$arg1" in
    *!!*) arg="${arg1%!!*},$secopt1!!${arg1#*!!}" ;;
    *)    arg="$arg1,$secopt1" ;;
    esac
    # start server
    CMD3="$TRACE $SOCAT $opts $arg echo"
    $CMD3 2>"${te}3" &
    spid=$!
    if [ "$port" ] && ! wait${proto}${ipvers}port $port 1; then
	kill $spid 2>/dev/null
	$PRINTF "$NO_RESULT (ph.2 server not working)\n"
	wait
	echo "$CMD3"
	cat "${te}3"
	numCANT=$((numCANT+1))
	return
    fi
    # now use client
    da="test$N.2 $(date) $RANDOM"
    (echo "$da"; sleep $T) |$TRACE $SOCAT $opts - "$arg2" >"$tf" 2>"${te}4"
    stat=$?
    kill $spid 2>/dev/null
#set +vx
    #killall $TRACE $SOCAT 2>/dev/null
    if [ "$stat" != 0 ]; then
	result=-1;	# socat had error
    elif [ ! -s "$tf" ]; then
	result=0;	# empty output
    elif echo "$da" |diff - "$tf" >"$tdiff2" 2>&1; then
	result=1;	# output is copy of input
    else
	result=2;	# output differs from input
    fi
    if [ X$result != X$expect ]; then
	case X$result in
	X-1) $PRINTF "$NO_RESULT (ph.2 client error): $TRACE $SOCAT:\n"
	    echo "$TRACE $SOCAT $opts $arg echo"
	    cat "${te}3"
	    echo "$TRACE $SOCAT $opts - $arg2"
	    cat "${te}4"
	    numCANT=$((numCANT+1)) ;;
	X0) $PRINTF "$NO_RESULT (ph.2 diff failed): diff:\n"
	    echo "$TRACE $SOCAT $opts $arg echo"
	    cat "${te}3"
	    echo "$TRACE $SOCAT $opts - $arg2"
	    cat "${te}4"
	    cat "$tdiff2"
	    numCANT=$((numCANT+1)) ;;
	X1) $PRINTF "$FAILED: SECURITY BROKEN\n"
	    echo "$TRACE $SOCAT $opts $arg echo"
	    cat "${te}3"
	    echo "$TRACE $SOCAT $opts - $arg2"
	    cat "${te}4"
	    cat "$tdiff2"
	    numFAIL=$((numFAIL+1))
	    listFAIL="$listFAIL $N" ;;
	X2) $PRINTF "$FAILED: diff:\n"
	    echo "$TRACE $SOCAT $opts $arg echo"
	    cat "${te}3"
	    echo "$TRACE $SOCAT $opts - $arg2"
	    cat "${te}4"
	    cat "$tdiff2"
	    numFAIL=$((numFAIL+1))
	    listFAIL="$listFAIL $N" ;;
	esac
    else
	$PRINTF "$OK\n"
	if [ -n "$debug" ]; then cat $te; fi
	numOK=$((numOK+1))
    fi
    wait
#set +vx
}


NAME=TCP4RANGEBITS
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of TCP4-L with RANGE option"
if ! eval $NUMCOND; then :;
elif [ -z "$SECONDADDR" ]; then
    # we need access to a second addresses
    $PRINTF "test $F_n $TEST... ${YELLOW}need a second IPv4 address${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testserversec "$N" "$TEST" "$opts -s" "tcp4-l:$PORT,reuseaddr,fork,retry=1" "" "range=$SECONDADDR/32" "tcp4:127.0.0.1:$PORT" 4 tcp $PORT 0
fi ;; # $SECONDADDR, NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=TCP4RANGEMASK
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of TCP4-L with RANGE option"
if ! eval $NUMCOND; then :;
elif [ -z "$SECONDADDR" ]; then
    # we need access to a second addresses
    $PRINTF "test $F_n $TEST... ${YELLOW}need a second IPv4 address${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testserversec "$N" "$TEST" "$opts -s" "tcp4-l:$PORT,reuseaddr,fork,retry=1" "" "range=$SECONDADDR:255.255.255.255" "tcp4:127.0.0.1:$PORT" 4 tcp $PORT 0
fi ;; # $SECONDADDR, NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

# like TCP4RANGEMASK, but the "bad" address is within the same class A network
NAME=TCP4RANGEMASKHAIRY
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of TCP4-L with RANGE option"
if ! eval $NUMCOND; then :; else
testserversec "$N" "$TEST" "$opts -s" "tcp4-l:$PORT,reuseaddr,fork,retry=1" "" "range=127.0.0.0:255.255.0.0" "tcp4:$SECONDADDR:$PORT,bind=$SECONDADDR" 4 tcp $PORT 0
fi ;; # Linux, NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=TCP4SOURCEPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%sourceport%*|*%$NAME%*)
TEST="$NAME: security of TCP4-L with SOURCEPORT option"
if ! eval $NUMCOND; then :; else
testserversec "$N" "$TEST" "$opts -s" "tcp4-l:$PORT,reuseaddr,fork,retry=1" "" "sp=$PORT" "tcp4:127.0.0.1:$PORT" 4 tcp $PORT 0
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=TCP4LOWPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%lowport%*|*%$NAME%*)
TEST="$NAME: security of TCP4-L with LOWPORT option"
if ! eval $NUMCOND; then :; else
testserversec "$N" "$TEST" "$opts -s" "tcp4-l:$PORT,reuseaddr,fork,retry=1" "" "lowport" "tcp4:127.0.0.1:$PORT" 4 tcp $PORT 0
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=TCP4WRAPPERS_ADDR
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: security of TCP4-L with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip4 libwrap) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: $SECONDADDR" >"$ha"
$ECHO "ALL: ALL" >"$hd"
testserversec "$N" "$TEST" "$opts -s" "tcp4-l:$PORT,reuseaddr,fork,retry=1" "" "hosts-allow=$ha,hosts-deny=$hd" "tcp4:127.0.0.1:$PORT" 4 tcp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=TCP4WRAPPERS_NAME
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: security of TCP4-L with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip4 libwrap) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: $LOCALHOST" >"$ha"
$ECHO "ALL: ALL" >"$hd"
testserversec "$N" "$TEST" "$opts -s" "tcp4-l:$PORT,reuseaddr,fork,retry=1" "" "hosts-allow=$ha,hosts-deny=$hd" "tcp4:$SECONDADDR:$PORT,bind=$SECONDADDR" 4 tcp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=TCP6RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of TCP6-L with RANGE option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testserversec "$N" "$TEST" "$opts -s" "tcp6-l:$PORT,reuseaddr,fork,retry=1" "" "range=[::2/128]" "tcp6:[::1]:$PORT" 6 tcp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=TCP6SOURCEPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%sourceport%*|*%$NAME%*)
TEST="$NAME: security of TCP6-L with SOURCEPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testserversec "$N" "$TEST" "$opts -s" "tcp6-l:$PORT,reuseaddr,fork,retry=1" "" "sp=$PORT" "tcp6:[::1]:$PORT" 6 tcp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=TCP6LOWPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%lowport%*|*%$NAME%*)
TEST="$NAME: security of TCP6-L with LOWPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testserversec "$N" "$TEST" "$opts -s" "tcp6-l:$PORT,reuseaddr,fork,retry=1" "" "lowport" "tcp6:[::1]:$PORT" 6 tcp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=TCP6TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: security of TCP6-L with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip6 libwrap) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: [::2]" >"$ha"
$ECHO "ALL: ALL" >"$hd"
testserversec "$N" "$TEST" "$opts -s" "tcp6-l:$PORT,reuseaddr,fork,retry=1" "" "hosts-allow=$ha,hosts-deny=$hd" "tcp6:[::1]:$PORT" 6 tcp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=UDP4RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of UDP4-L with RANGE option"
if ! eval $NUMCOND; then :; else
#testserversec "$N" "$TEST" "$opts -s" "udp4-l:$PORT,reuseaddr,fork" "" "range=$SECONDADDR/32" "udp4:127.0.0.1:$PORT" 4 udp $PORT 0
testserversec "$N" "$TEST" "$opts -s" "udp4-l:$PORT,reuseaddr" "" "range=$SECONDADDR/32" "udp4:127.0.0.1:$PORT" 4 udp $PORT 0
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP4SOURCEPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%sourceport%*|*%$NAME%*)
TEST="$NAME: security of UDP4-L with SOURCEPORT option"
if ! eval $NUMCOND; then :; else
testserversec "$N" "$TEST" "$opts -s" "udp4-l:$PORT,reuseaddr" "" "sp=$PORT" "udp4:127.0.0.1:$PORT" 4 udp $PORT 0
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP4LOWPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%lowport%*|*%$NAME%*)
TEST="$NAME: security of UDP4-L with LOWPORT option"
if ! eval $NUMCOND; then :; else
testserversec "$N" "$TEST" "$opts -s" "udp4-l:$PORT,reuseaddr" "" "lowport" "udp4:127.0.0.1:$PORT" 4 udp $PORT 0
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP4TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: security of UDP4-L with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip4 libwrap) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: $SECONDADDR" >"$ha"
$ECHO "ALL: ALL" >"$hd"
testserversec "$N" "$TEST" "$opts -s" "udp4-l:$PORT,reuseaddr,retry=1" "" "tcpwrap-etc=$td" "udp4:127.0.0.1:$PORT" 4 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=UDP6RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of UDP6-L with RANGE option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
#testserversec "$N" "$TEST" "$opts -s" "udp6-l:$PORT,reuseaddr,fork" "" "range=[::2/128]" "udp6:[::1]:$PORT" 6 udp $PORT 0
testserversec "$N" "$TEST" "$opts -s" "udp6-l:$PORT,reuseaddr" "" "range=[::2/128]" "udp6:[::1]:$PORT" 6 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP6SOURCEPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%sourceport%*|*%$NAME%*)
TEST="$NAME: security of UDP6-L with SOURCEPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testserversec "$N" "$TEST" "$opts -s" "udp6-l:$PORT,reuseaddr" "" "sp=$PORT" "udp6:[::1]:$PORT" 6 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP6LOWPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%lowport%*|*%$NAME%*)
TEST="$NAME: security of UDP6-L with LOWPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testserversec "$N" "$TEST" "$opts -s" "udp6-l:$PORT,reuseaddr" "" "lowport" "udp6:[::1]:$PORT" 6 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP6TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: security of UDP6-L with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip6 libwrap) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: [::2]" >"$ha"
$ECHO "ALL: ALL" >"$hd"
testserversec "$N" "$TEST" "$opts -s" "udp6-l:$PORT,reuseaddr" "" "lowport" "udp6:[::1]:$PORT" 6 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=OPENSSLTCP4_RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%openssl%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of SSL-L over TCP/IPv4 with RANGE option"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
testserversec "$N" "$TEST" "$opts -s" "SSL-L:$PORT,pf=ip4,reuseaddr,fork,retry=1,$SOCAT_EGD,verify=0,cert=testsrv.crt,key=testsrv.key" "" "range=$SECONDADDR/32" "SSL:$LOCALHOST:$PORT,cafile=testsrv.crt,$SOCAT_EGD" 4 tcp $PORT -1
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=OPENSSLTCP4_SOURCEPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%openssl%*|*%sourceport%*|*%$NAME%*)
TEST="$NAME: security of SSL-L with SOURCEPORT option"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
testserversec "$N" "$TEST" "$opts -s" "SSL-L:$PORT,pf=ip4,reuseaddr,fork,retry=1,$SOCAT_EGD,verify=0,cert=testsrv.crt,key=testsrv.key" "" "sp=$PORT" "SSL:$LOCALHOST:$PORT,cafile=testsrv.crt,$SOCAT_EGD" 4 tcp $PORT -1
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=OPENSSLTCP4_LOWPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%openssl%*|*%lowport%*|*%$NAME%*)
TEST="$NAME: security of SSL-L with LOWPORT option"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
testserversec "$N" "$TEST" "$opts -s" "SSL-L:$PORT,pf=ip4,reuseaddr,fork,retry=1,$SOCAT_EGD,verify=0,cert=testsrv.crt,key=testsrv.key" "" "lowport" "SSL:$LOCALHOST:$PORT,cafile=testsrv.crt,$SOCAT_EGD" 4 tcp $PORT -1
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=OPENSSLTCP4_TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%openssl%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: security of SSL-L with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 tcp libwrap openssl); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: $SECONDADDR" >"$ha"
$ECHO "ALL: ALL" >"$hd"
testserversec "$N" "$TEST" "$opts -s" "SSL-L:$PORT,pf=ip4,reuseaddr,fork,retry=1,$SOCAT_EGD,verify=0,cert=testsrv.crt,key=testsrv.key" "" "tcpwrap-etc=$td" "SSL:$LOCALHOST:$PORT,cafile=testsrv.crt,$SOCAT_EGD" 4 tcp $PORT -1
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=OPENSSLCERTSERVER
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%openssl%*|*%$NAME%*)
TEST="$NAME: security of SSL-L with client certificate"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
gentestcert testcli
testserversec "$N" "$TEST" "$opts -s" "SSL-L:$PORT,pf=ip4,reuseaddr,fork,retry=1,$SOCAT_EGD,verify,cert=testsrv.crt,key=testsrv.key" "cafile=testcli.crt" "cafile=testsrv.crt" "SSL:$LOCALHOST:$PORT,cafile=testsrv.crt,cert=testcli.pem,$SOCAT_EGD" 4 tcp $PORT -1
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=OPENSSLCERTCLIENT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%openssl%*|*%$NAME%*)
TEST="$NAME: security of SSL with server certificate"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
gentestcert testcli
testserversec "$N" "$TEST" "$opts -s -lu -d" "ssl:$LOCALHOST:$PORT,pf=ip4,fork,retry=2,verify,cert=testcli.pem,$SOCAT_EGD" "cafile=testsrv.crt" "cafile=testcli.crt" "ssl-l:$PORT,pf=ip4,reuseaddr,$SOCAT_EGD,cafile=testcli.crt,cert=testsrv.crt,key=testsrv.key" 4 tcp "" -1
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=OPENSSLTCP6_RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%openssl%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of SSL-L over TCP/IPv6 with RANGE option"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testaddrs tcp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert6 testsrv6
testserversec "$N" "$TEST" "$opts -s" "ssl-l:$PORT,pf=ip6,reuseaddr,fork,retry=1,$SOCAT_EGD,verify=0,cert=testsrv6.crt,key=testsrv6.key" "" "range=[::2/128]" "ssl:[::1]:$PORT,cafile=testsrv6.crt,$SOCAT_EGD" 6 tcp $PORT -1
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=OPENSSLTCP6_SOURCEPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%openssl%*|*%sourceport%*|*%$NAME%*)
TEST="$NAME: security of SSL-L over TCP/IPv6 with SOURCEPORT option"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testaddrs tcp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert6 testsrv6
testserversec "$N" "$TEST" "$opts -s" "ssl-l:$PORT,pf=ip6,reuseaddr,fork,retry=1,$SOCAT_EGD,verify=0,cert=testsrv6.crt,key=testsrv6.key" "" "sp=$PORT" "ssl:[::1]:$PORT,cafile=testsrv6.crt,$SOCAT_EGD" 6 tcp $PORT -1
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=OPENSSLTCP6_LOWPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%openssl%*|*%lowport%*|*%$NAME%*)
TEST="$NAME: security of SSL-L over TCP/IPv6 with LOWPORT option"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testaddrs tcp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert6 testsrv6
testserversec "$N" "$TEST" "$opts -s" "ssl-l:$PORT,pf=ip6,reuseaddr,fork,retry=1,$SOCAT_EGD,verify=0,cert=testsrv6.crt,key=testsrv6.key" "" "lowport" "ssl:[::1]:$PORT,cafile=testsrv6.crt,$SOCAT_EGD" 6 tcp $PORT -1
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=OPENSSLTCP6_TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%openssl%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: security of SSL-L over TCP/IPv6 with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip6 tcp libwrap openssl) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert6 testsrv6
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: [::2]" >"$ha"
$ECHO "ALL: ALL" >"$hd"
testserversec "$N" "$TEST" "$opts -s" "ssl-l:$PORT,pf=ip6,reuseaddr,fork,retry=1,$SOCAT_EGD,verify=0,cert=testsrv6.crt,key=testsrv6.key" "" "tcpwrap-etc=$td" "ssl:[::1]:$PORT,cafile=testsrv6.crt,$SOCAT_EGD" 6 tcp $PORT -1
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


# test security with the openssl-commonname option on client side
NAME=OPENSSL_CN_CLIENT_SECURITY
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%openssl%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: security of client openssl-commonname option"
# connect using non matching server name/address with commonname
# options, this should succeed. Then without this option, should fail
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
gentestcert testcli
testserversec "$N" "$TEST" "$opts" "SSL:127.0.0.1:$PORT,fork,retry=2,verify,cafile=testsrv.crt" "commonname=$LOCALHOST" "" "SSL-L:$PORT,pf=ip4,reuseaddr,cert=testsrv.crt,key=testsrv.key,verify=0" 4 tcp "" 0
fi ;; # testaddrs, NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

# test security with the openssl-commonname option on server side
NAME=OPENSSL_CN_SERVER_SECURITY
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%openssl%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: security of server openssl-commonname option"
# connect using with client certificate to server, this should succeed.
# Then use the server with a non matching openssl-commonname option,
# this must fail
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
gentestcert testcli
testserversec "$N" "$TEST" "$opts" "SSL-L:$PORT,pf=ip4,reuseaddr,cert=testsrv.crt,key=testsrv.key,cafile=testcli.crt" "" "commonname=onlyyou" "SSL:$LOCALHOST:$PORT,verify=0,cafile=testsrv.crt,cert=testcli.crt,key=testcli.key" 4 tcp "" 0
fi ;; # testaddrs, NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=OPENSSL_FIPS_SECURITY
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%openssl%*|*%fips%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: OpenSSL restrictions by FIPS"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testoptions fips >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL/FIPS not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
gentestcert testcli
# openssl client accepts a "normal" certificate only when not in fips mode
testserversec "$N" "$TEST" "$opts -s" "ssl:$LOCALHOST:$PORT,fork,retry=2,verify,cafile=testsrv.crt" "" "fips" "ssl-l:$PORT,pf=ip4,reuseaddr,cert=testsrv.crt,key=testsrv.key" 4 tcp "" -1
fi ;; # testaddrs, NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=UNIEXECEOF
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: give exec'd write-only process a chance to flush (-u)"
testod "$N" "$TEST" "" exec:"$OD_C" "$opts -u"
esac
N=$((N+1))


NAME=REVEXECEOF
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: give exec'd write-only process a chance to flush (-U)"
testod "$N" "$TEST" exec:"$OD_C" "-" "$opts -U"
esac
N=$((N+1))


NAME=FILANDIR
case "$TESTS" in
*%$N%*|*%filan%*|*%$NAME%*)
TEST="$NAME: check type printed for directories"
if ! eval $NUMCOND; then :; else
te="$td/test$N.stderr"
printf "test $F_n $TEST... " $N
type=$($FILAN -f . 2>$te |tail -n 1 |awk '{print($2);}')
if [ "$type" = "dir" ]; then
    $PRINTF "$OK\n"
   numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi ;; # NUMCOND
esac
N=$((N+1))


NAME=FILANSOCKET
case "$TESTS" in
*%$N%*|*%filan%*|*%$NAME%*)
TEST="$NAME: capability to analyze named unix socket"
if ! eval $NUMCOND; then :; else
ts="$td/test$N.socket"
te1="$td/test$N.stderr1"	# socat
te2="$td/test$N.stderr2"	# filan
printf "test $F_n $TEST... " $N
$TRACE $SOCAT $opts UNIX-LISTEN:"$ts" /dev/null </dev/null 2>"$te1" &
spid=$!
waitfile "$ts" 1
type=$($FILAN -f "$ts" 2>$te2 |tail -n 1 |awk '{print($2);}')
if [ "$type" = "socket" ]; then
    $PRINTF "$OK\n"
   numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    cat "$te1"
    cat "$te2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
kill $spid 2>/dev/null
wait
fi ;; # NUMCOND
esac
N=$((N+1))


testptywaitslave () {
    local N="$1"
    local TEST="$2"
    local PTYTYPE="$3"	# ptmx or openpty
    local opts="$4"

    local tp="$td/test$N.pty"
    local ts="$td/test$N.socket"
    local tf="$td/test$N.file"
    local tdiff="$td/test$N.diff"
    local te1="$td/test$N.stderr1"
    local te2="$td/test$N.stderr2"
    local te3="$td/test$N.stderr3"
    local te4="$td/test$N.stderr4"
    local da="test$N $(date) $RANDOM"
printf "test $F_n $TEST... " $N
# first generate a pty, then a socket
($TRACE $SOCAT $opts -lpsocat1 pty,$PTYTYPE,pty-wait-slave,link="$tp" unix-listen:"$ts" 2>"$te1"; rm -f "$tp") 2>/dev/null &
pid=$!
waitfile "$tp"
# if pty was non-blocking, the socket is active, and socat1 will term
$TRACE $SOCAT $opts -T 10 -lpsocat2 file:/dev/null unix-connect:"$ts" 2>"$te2"
# if pty is blocking, first socat is still active and we get a connection now
#((echo "$da"; sleep 2) |$TRACE $SOCAT -lpsocat3 $opts - file:"$tp",$PTYOPTS2 >"$tf" 2>"$te3") &
( (waitfile "$ts"; echo "$da"; sleep 1) |$TRACE $SOCAT -lpsocat3 $opts - file:"$tp",$PTYOPTS2 >"$tf" 2>"$te3") &
waitfile "$ts"
# but we need an echoer on the socket
$TRACE $SOCAT $opts -lpsocat4 unix:"$ts" echo 2>"$te4"
# now $tf file should contain $da
#kill $pid 2>/dev/null
wait
#
if echo "$da" |diff - "$tf"> "$tdiff"; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "${YELLOW}FAILED${NORMAL}\n"
    cat "$te1"
    #cat "$te2"	# not of interest
    cat "$te3"
    cat "$te4"
    cat "$tdiff"
    numCANT=$((numCANT+1))
fi
}

NAME=PTMXWAITSLAVE
PTYTYPE=ptmx
case "$TESTS" in
*%$N%*|*%functions%*|*%pty%*|*%$NAME%*)
TEST="$NAME: test if master pty ($PTYTYPE) waits for slave connection"
if ! eval $NUMCOND; then :; else
if ! feat=$(testaddrs pty); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testoptions "$PTYTYPE" pty-wait-slave); then
    $PRINTF "test $F_n $TEST... ${YELLOW}option $(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
   testptywaitslave "$N" "$TEST" "$PTYTYPE" "$opts"
fi
fi ;; # NUMCOND
esac
N=$((N+1))

NAME=OPENPTYWAITSLAVE
PTYTYPE=openpty
case "$TESTS" in
*%$N%*|*%functions%*|*%pty%*|*%$NAME%*)
TEST="$NAME: test if master pty ($PTYTYPE) waits for slave connection"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs pty); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testoptions "$PTYTYPE" pty-wait-slave); then
    $PRINTF "test $F_n $TEST... ${YELLOW}option $(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
   testptywaitslave "$N" "$TEST" "$PTYTYPE" "$opts"
fi ;; # NUMCOND, feats
esac
N=$((N+1))


NAME=CONNECTTIMEOUT
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%ipapp%*|*%tcp%*|*%timeout%*|*%$NAME%*)
TEST="$NAME: test the connect-timeout option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! feat=$(testoptions connect-timeout); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
# we need a hanging connection attempt, guess an address for this
case "$UNAME" in
Linux) HANGIP=1.0.0.1 ;;
*) HANGIP=255.255.255.254 ;;
esac
te1="$td/test$N.stderr1"
tk1="$td/test$N.kill1"
te2="$td/test$N.stderr2"
tk2="$td/test$N.kill2"
$PRINTF "test $F_n $TEST... " $N
# first, try to make socat hang and see if it can be killed
#$TRACE $SOCAT $opts - tcp:$HANGIP:1 >"$te1" 2>&1 </dev/null &
CMD="$TRACE $SOCAT $opts - tcp:$HANGIP:1"
$CMD >"$te1" 2>&1 </dev/null &
pid1=$!
sleep 2
if ! kill $pid1 2>"$tk1"; then
    $PRINTF "${YELLOW}does not hang${NORMAL}\n"
    numCANT=$((numCANT+1))
else
# second, set connect-timeout and see if socat exits before kill
$TRACE $SOCAT $opts - tcp:$HANGIP:1,connect-timeout=1.0 >"$te2" 2>&1 </dev/null &
pid2=$!
sleep 2
if kill $pid2 2>"$tk2"; then
    $PRINTF "$FAILED\n"
    echo "$CMD"
    cat "$te1"
    cat "$te2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
   numOK=$((numOK+1))
fi
fi
wait
fi ;; # testaddrs, NUMCOND
esac
N=$((N+1))


# version 1.7.0.0 had a bug with the connect-timeout option: while it correctly
# terminated a hanging connect attempt, it prevented a successful connection
# establishment from being recognized by socat, instead the timeout occurred
NAME=CONNECTTIMEOUT_CONN
if ! eval $NUMCOND; then :; else
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%ipapp%*|*%tcp%*|*%timeout%*|*%$NAME%*)
TEST="$NAME: TCP4 connect-timeout option when server replies"
# just try a connection that is expected to succeed with the usual data
# transfer; with the bug it will fail
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="127.0.0.1:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP4-LISTEN:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts STDIO TCP4:$ts,connect-timeout=1"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1=$!
waittcp4port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   cat "${te}1"
   echo "$CMD2"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid1 2>/dev/null
wait ;;
esac
PORT=$((PORT+1))
fi # NUMCOND
N=$((N+1))


NAME=OPENSSLLISTENDSA
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%$NAME%*)
TEST="$NAME: openssl listen with DSA certificate"
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
SRVCERT=testsrvdsa
gentestdsacert $SRVCERT
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD2="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,pf=ip4,reuseaddr,$SOCAT_EGD,cert=$SRVCERT.pem,key=$SRVCERT.key,verify=0 pipe"
CMD="$TRACE $SOCAT $opts - openssl:$LOCALHOST:$PORT,pf=ip4,verify=0,$SOCAT_EGD"
$PRINTF "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp4port $PORT
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat ${te}1 ${te}2; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # testaddrs, NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


# derive signal number from signal name
# kill -l should provide the info
signum () {
  if [ ! "$BASH_VERSION" -o -o posix ]; then
    # we expect:
    for i in $(POSIXLY_CORRECT=1 kill -l); do echo "$i"; done |grep -n -i "^$1$" |cut -d: -f1
  else
    # expect:
    # " 1) SIGHUP       2) SIGINT       3) SIGQUIT      4) SIGILL"
    signam="$1"
    kill -l </dev/null |
    while read l; do printf "%s %s\n%s %s\n%s %s\n%s %s\n" $l; done |
    grep -e "SIG$signam\$" |
    cut -d ')' -f 1
  fi
}

# problems with QUIT, INT (are blocked in system() )
for signam in TERM ILL; do
NAME=EXITCODESIG$signam
case "$TESTS" in
*%$N%*|*%functions%*|*%pty%*|*%signal%*|*%$NAME%*)
TEST="$NAME: exit status when dying on SIG$signam"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs pty); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat" |tr a-z A-Z) not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
SIG="$(signum $signam)"
te="$td/test$N.stderr"
tpp="$td/test$N.ppid"
tp="$td/test$N.pid"
$PRINTF "test $F_n $TEST... " $N
(sleep 1; kill -"$SIG" "$(cat "$tpp")") &
# a simple "system:echo $PPID..." does not work on NetBSD, OpenBSD
#$TRACE $SOCAT $opts echo SYSTEM:'exec /bin/bash -c "echo \$PPID '">$tpp"'; echo \$$ '">$tp; read x\"",nofork 2>"$te"; stat=$?
tsh="$td/test$N.sh"
cat <<EOF >"$tsh"
#! /bin/bash
echo \$PPID >"$tpp"
echo \$\$ >"$tp"
read x
EOF
chmod a+x "$tsh"
#$TRACE $SOCAT $opts echo SYSTEM:"exec \"$tsh\"",pty,setsid,nofork 2>"$te"; stat=$?
CMD="$TRACE $SOCAT $opts ECHO SYSTEM:\"exec\\\ \\\"$tsh\\\"\",pty,setsid,nofork"
$TRACE $SOCAT $opts ECHO SYSTEM:"exec \"$tsh\"",pty,setsid,nofork 2>"$te"
stat=$?
sleep 1; kill -INT $(cat $tp)
wait
if [ "$stat" -eq $((128+$SIG)) ]; then
    $PRINTF "$OK\n"
   numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
wait
fi ;; # NUMCOND, feats
esac
N=$((N+1))
done


NAME=READBYTES
#set -vx
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: restrict reading from file with bytes option"
if ! eval $NUMCOND; then :;
elif false; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tr="$td/test$N.ref"
ti="$td/test$N.in"
to="$td/test$N.out"
te="$td/test$N.err"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"; da="$da$($ECHO '\r')"
# the feature that we really want to test is in the readline.sh script:
CMD="$TRACE $SOCAT $opts -u open:$ti,readbytes=100 -"
printf "test $F_n $TEST... " $N
rm -f "$tf" "$ti" "$to"
#
echo "AAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAA" >"$tr"	# 100 bytes
cat "$tr" "$tr" >"$ti"			# 200 bytes
$CMD >"$to" 2>"$te"
if ! diff "$tr" "$to" >"$tdiff" 2>&1; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD"
    cat "$te"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
N=$((N+1))


NAME=UDPLISTENFORK
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%udp%*|*%listen%*|*%fork%*|*%$NAME%*)
TEST="$NAME: UDP socket rebinds after first connection"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da1="test$N $(date) $RANDOM"
da2="test$N $(date) $RANDOM"
#establish a listening and forking udp socket in background
#processes hang forever without -T
SRV="$TRACE $SOCAT -T 5 $opts -lpserver UDP4-LISTEN:$PORT,bind=$LOCALHOST,fork PIPE"
#make a first and a second connection
CLI="$TRACE $SOCAT $opts -lpclient - UDP4-CONNECT:$LOCALHOST:$PORT"
$PRINTF "test $F_n $TEST... " $N
eval "$SRV 2>${te}s &"
pids=$!
waitudp4port "$PORT"
echo "$da1" |eval "$CLI" >"${tf}1" 2>"${te}1"
if [ $? -ne 0 ]; then
    kill "$pids" 2>/dev/null
    $PRINTF "$NO_RESULT (first conn failed):\n"
    echo "$SRV &"
    echo "$CLI"
    cat "${te}s" "${te}1"
    numCANT=$((numCANT+1))
elif ! echo "$da1" |diff - "${tf}1" >"$tdiff"; then
    kill "$pids" 2>/dev/null
    $PRINTF "$NO_RESULT (first conn failed); diff:\n"
    cat "$tdiff"
    numCANT=$((numCANT+1))
else
sleep 2		# UDP-LISTEN sleeps 1s
echo "$da2" |eval "$CLI" >"${tf}2" 2>"${te}2"
rc="$?"; kill "$pids" 2>/dev/null
if [ $rc -ne 0 ]; then
    $PRINTF "$FAILED:\n"
    echo "$SRV &"
    echo "$CLI"
    cat "${te}s" "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da2" |diff - "${tf}2" >"$tdiff"; then
    $PRINTF "$FAILED: diff\n"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi # !( $? -ne 0)
fi # !(rc -ne 0)
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


# is a listen address capable of forking to child processes and have both
# active?
while read PROTOV MAJADDR MINADDR; do
if [ -z "$PROTOV" ] || [[ "$PROTOV" == \#* ]]; then continue; fi
protov="$(echo "$PROTOV" |tr A-Z a-z)"
proto="${protov%%[0-9]}"
NAME=${PROTOV}LISTENFORK
case "$TESTS" in
*%$N%*|*%functions%*|*%$protov%*|*%$proto%*|*%listen%*|*%fork%*|*%$NAME%*)
TEST="$NAME: $PROTOV listen handles 2 concurrent connections"
# have a listening address with fork option. connect with client1, send a piece
# of data, wait 1s, connect with client2, send another piece of data, wait 1s,
# and send another piece of data with client1. The server processes append all
# data to the same file. Check all data are written to the file in correct
# order.
if ! eval $NUMCOND; then :;
#elif ! feat=$(testaddrs $PROTOV); then
#    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$PROTOV" |tr a-z A-Z) not available${NORMAL}\n" $N
#    numCANT=$((numCANT+1))
elif ! runs$protov; then
     $PRINTF "test $F_n $TEST... ${YELLOW}$PROTOV not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ts="$td/test$N.sock"
tref="$td/test$N.ref"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da1a="test$N $(date) 1a $RANDOM"
da1b="test$N $(date) 1b $RANDOM"
da2="test$N $(date) 2 $RANDOM"
case "$MAJADDR" in
    "FILE")
	tla="$ts"
	tca="$ts"
	waitproto="file"
	waitfor="$ts" ;;
esac	
case "$MINADDR" in
    "PORT")
	tla="$PORT,bind=$MAJADDR"
	tca="$MAJADDR:$PORT"
	waitproto="${protov}port"
	waitfor="$PORT" ;;
esac	
#set -xv
echo -e "$da1a\n$da2\n$da1b" >"$tref"
# establish a listening and forking listen socket in background
# UDP processes hang forever without -T
CMD0="$TRACE $SOCAT -T 5 $opts -lpserver $PROTOV-LISTEN:$tla,fork PIPE"
# make a first and a second connection
CMD1="$TRACE $SOCAT $opts -lpclient - $PROTOV-CONNECT:$tca"
$PRINTF "test $F_n $TEST... " $N
eval "$CMD0 2>${te}0 &"
pid0=$!
wait$waitproto "$waitfor" 1 2
(echo "$da1a"; sleep 2; echo "$da1b") |eval "$CMD1" >>"${tf}" 2>"${te}1" &
sleep 1
# trailing sleep req for sctp because no half close
(echo "$da2"; sleep 1) |eval "$CMD1" >>"${tf}" 2>"${te}2" &
sleep 2
kill $pid0 2>/dev/null
wait
if ! diff "$tref" "$tf" >"$tdiff"; then
    $PRINTF "$FAILED\n"
    cat "${te}0" "${te}1" "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi # !(rc -ne 0)
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))
done <<<"
TCP4  $LOCALHOST PORT
TCP6  $LOCALHOST6 PORT
UDP4  $LOCALHOST PORT
UDP6  $LOCALHOST6 PORT
SCTP4 $LOCALHOST PORT
SCTP6 $LOCALHOST6 PORT
UNIX FILE ,
"


NAME=UNIXTOSTREAM
case "$TESTS" in
*%$N%*|*%functions%*|*%unix%*|*%listen%*|*%$NAME%*)
TEST="$NAME: generic UNIX client connects to stream socket"
if ! eval $NUMCOND; then :; else
ts="$td/test$N.socket"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da1="test$N $(date) $RANDOM"
#establish a listening unix socket in background
SRV="$TRACE $SOCAT $opts -lpserver UNIX-LISTEN:\"$ts\" PIPE"
#make a connection
CLI="$TRACE $SOCAT $opts -lpclient - UNIX:\"$ts\""
$PRINTF "test $F_n $TEST... " $N
eval "$SRV 2>${te}s &"
pids=$!
waitfile "$ts"
echo "$da1" |eval "$CLI" >"${tf}1" 2>"${te}1"
if [ $? -ne 0 ]; then
    kill "$pids" 2>/dev/null
    $PRINTF "$FAILED:\n"
    echo "$SRV &"
    echo "$CLI"
    cat "${te}s" "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da1" |diff - "${tf}1" >"$tdiff"; then
    kill "$pids" 2>/dev/null
    $PRINTF "$FAILED; diff:\n"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi # !(rc -ne 0)
wait
fi ;; # NUMCOND
esac
N=$((N+1))


NAME=UNIXTODGRAM
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%unix%*|*%recv%*|*%$NAME%*)
TEST="$NAME: generic UNIX client connects to datagram socket"
if ! eval $NUMCOND; then :; else
ts1="$td/test$N.socket1"
ts2="$td/test$N.socket2"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da1="test$N $(date) $RANDOM"
#establish a receiving unix datagram socket in background
SRV="$TRACE $SOCAT $opts -lpserver UNIX-RECVFROM:\"$ts1\" PIPE"
#make a connection
CLI="$TRACE $SOCAT $opts -lpclient - UNIX:\"$ts1\",bind=\"$ts2\""
#CLI="$TRACE $SOCAT $opts -lpclient - UNIX:\"$ts1\""
$PRINTF "test $F_n $TEST... " $N
eval "$SRV 2>${te}s &"
pids=$!
waitfile "$ts1"
echo "$da1" |eval "$CLI" >"${tf}1" 2>"${te}1"
rc=$?
wait
if [ $rc -ne 0 ]; then
    kill "$pids" 2>/dev/null
    $PRINTF "$FAILED:\n"
    echo "$SRV &"
    cat "${te}s"
    echo "$CLI"
    cat "${te}1" "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da1" |diff - "${tf}1" >"$tdiff"; then
    kill "$pids" 2>/dev/null
    $PRINTF "$FAILED:\n"
    echo "$SRV &"
    cat "${te}s"
    echo "$CLI"
    cat "${te}1"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi # !(rc -ne 0)
fi ;; # NUMCOND
esac
N=$((N+1))


# there was an error in address EXEC with options pipes,stderr
NAME=EXECPIPESSTDERR
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: simple echo via exec of cat with pipes,stderr"
# this test is known to fail when logging is enabled with OPTS/opts env var.
SAVE_opts="$opts"
opts="$(echo "$opts" |sed 's/-d//g')"
testecho "$N" "$TEST" "" "exec:$CAT,pipes,stderr" "$opts"
opts="$SAVE_opts"
esac
N=$((N+1))

# EXEC and SYSTEM with stderr injected socat messages into the data stream. 
NAME=EXECSTDERRLOG
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: simple echo via exec of cat with pipes,stderr"
SAVE_opts="$opts"
# make sure at least two -d are there
case "$opts" in
*-d*-d*) ;;
*-d*) opts="$opts -d" ;;
*) opts="-d -d" ;;
esac
testecho "$N" "$TEST" "" "exec:$CAT,pipes,stderr" "$opts"
opts="$SAVE_opts"
esac
N=$((N+1))


NAME=SIMPLEPARSE
case "$TESTS" in
*%$N%*|*%functions%*|*%PARSE%*|*%$NAME%*)
TEST="$NAME: invoke socat from socat"
testecho "$N" "$TEST" "" exec:"$SOCAT - exec\:$CAT,pipes" "$opts"
esac
N=$((N+1))


NAME=FULLPARSE
case "$TESTS" in
*%$N%*|*%functions%*|*%parse%*|*%$NAME%*)
TEST="$NAME: correctly parse special chars"
if ! eval $NUMCOND; then :; else
$PRINTF "test $F_n $TEST... " $N
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
# a string where commas are hidden in nesting lexical constructs
# if they are scanned incorrectly, socat will see an "unknown option"
dain='(,)[,]{,}","([),])hugo'
daout='(,)[,]{,},([),])hugo'
$TRACE "$SOCAT" $opts -u "exec:echo $dain" - >"$tf" 2>"$te"
rc=$?
echo "$daout" |diff "$tf" - >"$tdiff"
if [ "$rc" -ne 0 ]; then
    $PRINTF "$FAILED:\n"
    echo "$TRACE $SOCAT" -u "exec:echo $da" -
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif [ -s "$tdiff" ]; then
    $PRINTF "$FAILED:\n"
    echo diff:
    cat "$tdiff"
    if [ -n "$debug" ]; then cat $te; fi
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat $te; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))

NAME=NESTEDSOCATEXEC
case "$TESTS" in
*%parse%*|*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: does lexical analysis work sensibly (exec)"
testecho "$N" "$TEST" "" "exec:'$SOCAT - exec:$CAT,pipes'" "$opts" 1
esac
N=$((N+1))

NAME=NESTEDSOCATSYSTEM
case "$TESTS" in
*%parse%*|*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: does lexical analysis work sensibly (system)"
testecho "$N" "$TEST" "" "system:\"$SOCAT - exec:$CAT,pipes\"" "$opts" 1
esac
N=$((N+1))


NAME=TCP6BYTCP4
case "$TESTS" in
*%$N%*|*%functions%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%$NAME%*)
TEST="$NAME: TCP4 mapped into TCP6 address space"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="127.0.0.1:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP6-listen:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP6:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waittcp6port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null; wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


# test the UDP4-SENDTO and UDP4-RECVFROM addresses together
NAME=UDP4DGRAM
case "$TESTS" in
*%$N%*|*%functions%*|*%udp%*|*%udp4%*|*%ip4%*|*%dgram%*|*%$NAME%*)
TEST="$NAME: UDP/IPv4 sendto and recvfrom"
# start a UDP4-RECVFROM process that echoes data, and send test data using
# UDP4-SENDTO. The sent data should be returned.
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PORT; PORT=$((PORT+1))
ts1a="127.0.0.1"
ts1="$ts1a:$ts1p"
ts2p=$PORT; PORT=$((PORT+1))
ts2="127.0.0.1:$ts2p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts UDP4-RECVFROM:$ts1p,reuseaddr,bind=$ts1a PIPE"
CMD2="$TRACE $SOCAT $opts - UDP4-SENDTO:$ts1,bind=$ts2"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
pid1="$!"
waitudp4port $ts1p 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2="$?"
kill "$pid1" 2>/dev/null; wait;
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   cat "${te}1"
   echo "$CMD2"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   echo "$CMD1 &"
   cat "${te}1"
   echo "$CMD2"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=UDP6DGRAM
case "$TESTS" in
*%$N%*|*%functions%*|*%udp%*|*%udp6%*|*%ip6%*|*%dgram%*|*%$NAME%*)
TEST="$NAME: UDP/IPv6 datagram"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PORT; PORT=$((PORT+1))
tsa="[::1]"
ts1="$tsa:$ts1p"
ts2p=$PORT; PORT=$((PORT+1))
ts2="$tsa:$ts2p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts UDP6-RECVFROM:$ts1p,reuseaddr,bind=$tsa PIPE"
CMD2="$TRACE $SOCAT $opts - UDP6-SENDTO:$ts1,bind=$ts2"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
waitudp6port $ts1p 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat ${te}1 ${te}2; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=RAWIP4RECVFROM
case "$TESTS" in
*%$N%*|*%functions%*|*%ip%*|*%ip4%*|*%rawip%*|*%rawip4%*|*%dgram%*|*%root%*|*%$NAME%*)
TEST="$NAME: raw IPv4 datagram"
if ! eval $NUMCOND; then :;
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PROTO; PROTO=$((PROTO+1))
ts1a="127.0.0.1"
ts1="$ts1a:$ts1p"
ts2a="$SECONDADDR"
ts2="$ts2a:$ts2p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts IP4-RECVFROM:$ts1p,reuseaddr,bind=$ts1a PIPE"
CMD2="$TRACE $SOCAT $opts - IP4-SENDTO:$ts1,bind=$ts2a"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
pid1=$!
waitip4proto $ts1p 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2=$?
kill $pid1 2>/dev/null;  wait
if [ $rc2 -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # root, NUMCOND
esac
N=$((N+1))


if false; then
NAME=RAWIP6RECVFROM
case "$TESTS" in
*%$N%*|*%functions%*|*%ip%*|*%ip6%*|*%rawip%*|*%rawip6%*|*%dgram%*|*%root%*|*%$NAME%*)
TEST="$NAME: raw IPv6 datagram by self addressing"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip6 rawip) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PROTO; PROTO=$((PROTO+1))
tsa="[::1]"
ts1="$tsa:$ts1p"
ts2="$tsa"
da="test$N $(date) $RANDOM"
#CMD1="$TRACE $SOCAT $opts IP6-RECVFROM:$ts1p,reuseaddr,bind=$tsa PIPE"
CMD2="$TRACE $SOCAT $opts - IP6-SENDTO:$ts1,bind=$ts2"
printf "test $F_n $TEST... " $N
#$CMD1 2>"${te}1" &
waitip6proto $ts1p 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
#   echo "$CMD1 &"
#   cat "${te}1"
   echo "$CMD2"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "$te"; fi
   numOK=$((numOK+1))
fi
fi ;; # root, NUMCOND
esac
N=$((N+1))
fi #false


NAME=UNIXDGRAM
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%unix%*|*%dgram%*|*%$NAME%*)
TEST="$NAME: UNIX datagram"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1="$td/test$N.socket1"
ts2="$td/test$N.socket2"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts UNIX-RECVFROM:$ts1,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts - UNIX-SENDTO:$ts1,bind=$ts2"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
pid1="$!"
waitfile $ts1 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2=$?
kill "$pid1" 2>/dev/null; wait
if [ $rc2 -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   cat "${te}1"
   echo "$CMD2"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi # NUMCOND
 ;;
esac
N=$((N+1))


NAME=UDP4RECV
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%ip4%*|*%dgram%*|*%udp%*|*%udp4%*|*%recv%*|*%$NAME%*)
TEST="$NAME: UDP/IPv4 receive"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PORT; PORT=$((PORT+1))
ts1a="127.0.0.1"
ts1="$ts1a:$ts1p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -u UDP4-RECV:$ts1p,reuseaddr -"
CMD2="$TRACE $SOCAT $opts -u - UDP4-SENDTO:$ts1"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1="$!"
waitudp4port $ts1p 1
echo "$da" |$CMD2 2>>"${te}2"
rc2="$?"
#ls -l $tf
i=0; while [ ! -s "$tf" -a "$i" -lt 10 ]; do  usleep 100000; i=$((i+1));  done
kill "$pid1" 2>/dev/null; wait
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi # NUMCOND
 ;;
esac
N=$((N+1))


NAME=UDP6RECV
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%dgram%*|*%udp%*|*%udp6%*|*%recv%*|*%$NAME%*)
TEST="$NAME: UDP/IPv6 receive"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PORT; PORT=$((PORT+1))
ts1a="[::1]"
ts1="$ts1a:$ts1p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -u UDP6-RECV:$ts1p,reuseaddr -"
CMD2="$TRACE $SOCAT $opts -u - UDP6-SENDTO:$ts1"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1="$!"
waitudp6port $ts1p 1
echo "$da" |$CMD2 2>>"${te}2"
rc2="$?"
#ls -l $tf
i=0; while [ ! -s "$tf" -a "$i" -lt 10 ]; do  usleep 100000; i=$((i+1));  done
kill "$pid1" 2>/dev/null; wait
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
N=$((N+1))


NAME=RAWIP4RECV
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%dgram%*|*%rawip%*|*%rawip4%*|*%recv%*|*%root%*|*%$NAME%*)
TEST="$NAME: raw IPv4 receive"
if ! eval $NUMCOND; then :;
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PROTO; PROTO=$((PROTO+1))
ts1a="127.0.0.1"
ts1="$ts1a:$ts1p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -u IP4-RECV:$ts1p,reuseaddr -"
CMD2="$TRACE $SOCAT $opts -u - IP4-SENDTO:$ts1"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1="$!"
waitip4proto $ts1p 1
echo "$da" |$CMD2 2>>"${te}2"
rc2="$?"
#ls -l $tf
i=0; while [ ! -s "$tf" -a "$i" -lt 10 ]; do  usleep 100000; i=$((i+1));  done
kill "$pid1" 2>/dev/null; wait
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, root
esac
N=$((N+1))


NAME=RAWIP6RECV
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%dgram%*|*%rawip%*|*%rawip6%*|*%recv%*|*%root%*|*%$NAME%*)
TEST="$NAME: raw IPv6 receive"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip6 rawip) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PROTO; PROTO=$((PROTO+1))
ts1a="[::1]"
ts1="$ts1a:$ts1p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -u IP6-RECV:$ts1p,reuseaddr -"
CMD2="$TRACE $SOCAT $opts -u - IP6-SENDTO:$ts1"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1="$!"
waitip6proto $ts1p 1
echo "$da" |$CMD2 2>>"${te}2"
rc2="$?"
i=0; while [ ! -s "$tf" -a "$i" -lt 10 ]; do  usleep 100000; i=$((i+1));  done
kill "$pid1" 2>/dev/null; wait
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, root
esac
N=$((N+1))


NAME=UNIXRECV
case "$TESTS" in
*%$N%*|*%functions%*|*%unix%*|*%dgram%*|*%recv%*|*%$NAME%*)
TEST="$NAME: UNIX receive"
if ! eval $NUMCOND; then :; else
ts="$td/test$N.socket"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1="$ts"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -u UNIX-RECV:$ts1,reuseaddr -"
CMD2="$TRACE $SOCAT $opts -u - UNIX-SENDTO:$ts1"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1="$!"
waitfile $ts1 1
echo "$da" |$CMD2 2>>"${te}2"
rc2="$?"
i=0; while [ ! -s "$tf" -a "$i" -lt 10 ]; do  usleep 100000; i=$((i+1));  done
kill "$pid1" 2>/dev/null; wait
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi # NUMCOND
 ;;
esac
N=$((N+1))


NAME=UDP4RECVFROM_SOURCEPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%sourceport%*|*%$NAME%*)
TEST="$NAME: security of UDP4-RECVFROM with SOURCEPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip4) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testserversec "$N" "$TEST" "$opts -s" "udp4-recvfrom:$PORT,reuseaddr" "" "sp=$PORT" "udp4-sendto:127.0.0.1:$PORT" 4 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP4RECVFROM_LOWPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%lowport%*|*%$NAME%*)
TEST="$NAME: security of UDP4-RECVFROM with LOWPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip4) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testserversec "$N" "$TEST" "$opts -s" "udp4-recvfrom:$PORT,reuseaddr" "" "lowport" "udp4-sendto:127.0.0.1:$PORT" 4 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP4RECVFROM_RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of UDP4-RECVFROM with RANGE option"
#testserversec "$N" "$TEST" "$opts -s" "udp4-recvfrom:$PORT,reuseaddr,fork" "" "range=$SECONDADDR/32" "udp4-sendto:127.0.0.1:$PORT" 4 udp $PORT 0
if ! eval $NUMCOND; then :; else
testserversec "$N" "$TEST" "$opts -s" "udp4-recvfrom:$PORT,reuseaddr" "" "range=$SECONDADDR/32" "udp4-sendto:127.0.0.1:$PORT" 4 udp $PORT 0
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP4RECVFROM_TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: security of UDP4-RECVFROM with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 udp libwrap) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: $SECONDADDR" >"$ha"
$ECHO "ALL: ALL" >"$hd"
#testserversec "$N" "$TEST" "$opts -s" "udp4-recvfrom:$PORT,reuseaddr,fork" "" "tcpwrap=$d" "udp4-sendto:127.0.0.1:$PORT" 4 udp $PORT 0
testserversec "$N" "$TEST" "$opts -s" "udp4-recvfrom:$PORT,reuseaddr" "" "tcpwrap-etc=$td" "udp4-sendto:127.0.0.1:$PORT" 4 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=UDP4RECV_SOURCEPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%sourceport%*|*%$NAME%*)
TEST="$NAME: security of UDP4-RECV with SOURCEPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip4) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
PORT1=$PORT; PORT=$((PORT+1))
PORT2=$PORT; PORT=$((PORT+1))
PORT3=$PORT
# we use the forward channel (PORT1) for testing, and have a backward channel
# (PORT2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "udp4-recv:$PORT1,reuseaddr!!udp4-sendto:127.0.0.1:$PORT2" "" "sp=$PORT3" "udp4-recv:$PORT2!!udp4-sendto:127.0.0.1:$PORT1" 4 udp $PORT1 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP4RECV_LOWPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%lowport%*|*%$NAME%*)
TEST="$NAME: security of UDP4-RECV with LOWPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip4) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
PORT1=$PORT; PORT=$((PORT+1))
PORT2=$PORT
# we use the forward channel (PORT1) for testing, and have a backward channel
# (PORT2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "udp4-recv:$PORT1,reuseaddr!!udp4-sendto:127.0.0.1:$PORT2" "" "lowport" "udp4-recv:$PORT2!!udp4-sendto:127.0.0.1:$PORT1" 4 udp $PORT1 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP4RECV_RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of UDP4-RECV with RANGE option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip4) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
PORT1=$PORT; PORT=$((PORT+1))
PORT2=$PORT
# we use the forward channel (PORT1) for testing, and have a backward channel
# (PORT2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "udp4-recv:$PORT1,reuseaddr!!udp4-sendto:127.0.0.1:$PORT2" "" "range=$SECONDADDR/32" "udp4-recv:$PORT2!!udp4-sendto:127.0.0.1:$PORT1" 4 udp $PORT1 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP4RECV_TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: security of UDP4-RECV with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip4 libwrap) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
PORT1=$PORT; PORT=$((PORT+1))
PORT2=$PORT
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: $SECONDADDR" >"$ha"
$ECHO "ALL: ALL" >"$hd"
# we use the forward channel (PORT1) for testing, and have a backward channel
# (PORT2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "udp4-recv:$PORT1,reuseaddr!!udp4-sendto:127.0.0.1:$PORT2" "" "tcpwrap-etc=$td" "udp4-recv:$PORT2!!udp4-sendto:127.0.0.1:$PORT1" 4 udp $PORT1 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=UDP6RECVFROM_SOURCEPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%sourceport%*|*%$NAME%*)
TEST="$NAME: security of UDP6-RECVFROM with SOURCEPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testserversec "$N" "$TEST" "$opts -s" "udp6-recvfrom:$PORT,reuseaddr" "" "sp=$PORT" "udp6-sendto:[::1]:$PORT" 6 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP6RECVFROM_LOWPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%lowport%*|*%$NAME%*)
TEST="$NAME: security of UDP6-RECVFROM with LOWPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
testserversec "$N" "$TEST" "$opts -s" "udp6-recvfrom:$PORT,reuseaddr" "" "lowport" "udp6-sendto:[::1]:$PORT" 6 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP6RECVFROM_RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of UDP6-RECVFROM with RANGE option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
#testserversec "$N" "$TEST" "$opts -s" "udp6-recvfrom:$PORT,reuseaddr,fork" "" "range=[::2/128]" "udp6-sendto:[::1]:$PORT" 6 udp $PORT 0
testserversec "$N" "$TEST" "$opts -s" "udp6-recvfrom:$PORT,reuseaddr" "" "range=[::2/128]" "udp6-sendto:[::1]:$PORT" 6 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP6RECVFROM_TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: security of UDP6-RECVFROM with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip6 libwrap) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: [::2]" >"$ha"
$ECHO "ALL: ALL" >"$hd"
testserversec "$N" "$TEST" "$opts -s" "udp6-recvfrom:$PORT,reuseaddr" "" "tcpwrap-etc=$td" "udp6-sendto:[::1]:$PORT" 6 udp $PORT 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=UDP6RECV_SOURCEPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%sourceport%*|*%$NAME%*)
TEST="$NAME: security of UDP6-RECV with SOURCEPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
PORT1=$PORT; PORT=$((PORT+1))
PORT2=$PORT; PORT=$((PORT+1))
PORT3=$PORT
# we use the forward channel (PORT1) for testing, and have a backward channel
# (PORT2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "udp6-recv:$PORT1,reuseaddr!!udp6-sendto:[::1]:$PORT2" "" "sp=$PORT3" "udp6-recv:$PORT2!!udp6-sendto:[::1]:$PORT1" 6 udp $PORT1 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP6RECV_LOWPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%lowport%*|*%$NAME%*)
TEST="$NAME: security of UDP6-RECV with LOWPORT option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
PORT1=$PORT; PORT=$((PORT+1))
PORT2=$PORT
# we use the forward channel (PORT1) for testing, and have a backward channel
# (PORT2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "udp6-recv:$PORT1,reuseaddr!!udp6-sendto:[::1]:$PORT2" "" "lowport" "udp6-recv:$PORT2!!udp6-sendto:[::1]:$PORT1" 6 udp $PORT1 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP6RECV_RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of UDP6-RECV with RANGE option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
PORT1=$PORT; PORT=$((PORT+1))
PORT2=$PORT
# we use the forward channel (PORT1) for testing, and have a backward channel
# (PORT2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "udp6-recv:$PORT1,reuseaddr!!udp6-sendto:[::1]:$PORT2" "" "range=[::2/128]" "udp6-recv:$PORT2!!udp6-sendto:[::1]:$PORT1" 6 udp $PORT1 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=UDP6RECV_TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp6%*|*%ip6%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: security of UDP6-RECV with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip6 libwrap) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: [::2]" >"$ha"
$ECHO "ALL: ALL" >"$hd"
PORT1=$PORT; PORT=$((PORT+1))
PORT2=$PORT
# we use the forward channel (PORT1) for testing, and have a backward channel
# (PORT2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "udp6-recv:$PORT1,reuseaddr!!udp6-sendto:[::1]:$PORT2" "" "tcpwrap-etc=$td" "udp6-recv:$PORT2!!udp6-sendto:[::1]:$PORT1" 6 udp $PORT1 0
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=IP4RECVFROM_RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%ip%*|*%ip4%*|*%range%*|*%root%*|*%$NAME%*)
TEST="$NAME: security of IP4-RECVFROM with RANGE option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 rawip) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
#testserversec "$N" "$TEST" "$opts -s" "ip4-recvfrom:$PROTO,reuseaddr,fork" "" "range=$SECONDADDR/32" "ip4-sendto:127.0.0.1:$PROTO" 4 ip $PROTO 0
testserversec "$N" "$TEST" "$opts -s" "ip4-recvfrom:$PROTO,reuseaddr!!udp4-sendto:127.0.0.1:$PORT" "" "range=$SECONDADDR/32" "udp4-recv:$PORT!!ip4-sendto:127.0.0.1:$PROTO" 4 ip $PROTO 0
fi ;; # NUMCOND, feats, root
esac
PROTO=$((PROTO+1))
PORT=$((PORT+1))
N=$((N+1))

NAME=IP4RECVFROM_TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%ip%*|*%ip4%*|*%tcpwrap%*|*%root%*|*%$NAME%*)
TEST="$NAME: security of IP4-RECVFROM with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 rawip libwrap) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: $SECONDADDR" >"$ha"
$ECHO "ALL: ALL" >"$hd"
#testserversec "$N" "$TEST" "$opts -s" "ip4-recvfrom:$PROTO,reuseaddr,fork" "" "tcpwrap-etc=$td" "ip4-sendto:127.0.0.1:$PROTO" 4 ip $PROTO 0
testserversec "$N" "$TEST" "$opts -s" "ip4-recvfrom:$PROTO,reuseaddr!!udp4-sendto:127.0.0.1:$PORT" "" "tcpwrap-etc=$td" "udp4-recv:$PORT!!ip4-sendto:127.0.0.1:$PROTO" 4 ip $PROTO 0
fi # NUMCOND, feats, root
 ;;
esac
PROTO=$((PROTO+1))
PORT=$((PORT+1))
N=$((N+1))


NAME=IP4RECV_RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%ip%*|*%ip4%*|*%range%*|*%root%*|*%$NAME%*)
TEST="$NAME: security of IP4-RECV with RANGE option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 rawip) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
PROTO1=$PROTO; PROTO=$((PROTO+1))
PROTO2=$PROTO
# we use the forward channel (PROTO1) for testing, and have a backward channel
# (PROTO2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "ip4-recv:$PROTO1,reuseaddr!!ip4-sendto:127.0.0.1:$PROTO2" "" "range=$SECONDADDR/32" "ip4-recv:$PROTO2!!ip4-sendto:127.0.0.1:$PROTO1" 4 ip $PROTO1 0
fi ;; # NUMCOND, feats, root
esac
PROTO=$((PROTO+1))
N=$((N+1))



NAME=IP4RECV_TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%ip%*|*%ip4%*|*%tcpwrap%*|*%root%*|*%$NAME%*)
TEST="$NAME: security of IP4-RECV with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 rawip libwrap) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
PROTO1=$PROTO; PROTO=$((PROTO+1))
PROTO2=$PROTO
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: $SECONDADDR" >"$ha"
$ECHO "ALL: ALL" >"$hd"
# we use the forward channel (PROTO1) for testing, and have a backward channel
# (PROTO2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "ip4-recv:$PROTO1,reuseaddr!!ip4-sendto:127.0.0.1:$PROTO2" "" "tcpwrap-etc=$td" "ip4-recv:$PROTO2!!ip4-sendto:127.0.0.1:$PROTO1" 4 ip $PROTO1 0
fi ;; # NUMCOND, feats, root
esac
PROTO=$((PROTO+1))
N=$((N+1))


NAME=IP6RECVFROM_RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%ip%*|*%ip6%*|*%range%*|*%root%*|*%$NAME%*)
TEST="$NAME: security of IP6-RECVFROM with RANGE option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip6 rawip) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
#testserversec "$N" "$TEST" "$opts -s" "ip6-recvfrom:$PROTO,reuseaddr,fork" "" "range=[::2/128]" "ip6-sendto:[::1]:$PROTO" 6 ip $PROTO 0
testserversec "$N" "$TEST" "$opts -s" "ip6-recvfrom:$PROTO,reuseaddr!!udp6-sendto:[::1]:$PORT" "" "range=[::2/128]" "udp6-recv:$PORT!!ip6-sendto:[::1]:$PROTO" 6 ip $PROTO 0
fi ;; # NUMCOND, feats
esac
PROTO=$((PROTO+1))
PORT=$((PORT+1))
N=$((N+1))

NAME=IP6RECVFROM_TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%ip%*|*%ip6%*|*%tcpwrap%*|*%root%*|*%$NAME%*)
TEST="$NAME: security of IP6-RECVFROM with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip6 rawip libwrap) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: [::2]" >"$ha"
$ECHO "ALL: ALL" >"$hd"
#testserversec "$N" "$TEST" "$opts -s" "ip6-recvfrom:$PROTO,reuseaddr,fork" "" "tcpwrap-etc=$td" "ip6-sendto:[::1]:$PROTO" 6 ip $PROTO 0
testserversec "$N" "$TEST" "$opts -s" "ip6-recvfrom:$PROTO,reuseaddr!!udp6-sendto:[::1]:$PORT" "" "tcpwrap-etc=$td" "udp6-recv:$PORT!!ip6-sendto:[::1]:$PROTO" 6 ip $PROTO 0
fi ;; # NUMCOND, feats
esac
PROTO=$((PROTO+1))
PORT=$((PORT+1))
N=$((N+1))


NAME=IP6RECV_RANGE
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%ip%*|*%ip6%*|*%range%*|*%root%*|*%$NAME%*)
TEST="$NAME: security of IP6-RECV with RANGE option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip6 rawip) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}raw IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
PROTO1=$PROTO; PROTO=$((PROTO+1))
PROTO2=$PROTO
# we use the forward channel (PROTO1) for testing, and have a backward channel
# (PROTO2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "ip6-recv:$PROTO1,reuseaddr!!ip6-sendto:[::1]:$PROTO2" "" "range=[::2/128]" "ip6-recv:$PROTO2!!ip6-sendto:[::1]:$PROTO1" 6 ip $PROTO1 0
fi ;; # NUMCOND, feats
esac
PROTO=$((PROTO+1))
N=$((N+1))

NAME=IP6RECV_TCPWRAP
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%ip%*|*%ip6%*|*%tcpwrap%*|*%root%*|*%$NAME%*)
TEST="$NAME: security of IP6-RECV with TCPWRAP option"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip6 rawip libwrap) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
PROTO1=$PROTO; PROTO=$((PROTO+1))
PROTO2=$PROTO
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat: [::2]" >"$ha"
$ECHO "ALL: ALL" >"$hd"
# we use the forward channel (PROTO1) for testing, and have a backward channel
# (PROTO2) to get the data back, so we get the classical echo behaviour
testserversec "$N" "$TEST" "$opts -s" "ip6-recv:$PROTO1,reuseaddr!!ip6-sendto:[::1]:$PROTO2" "" "tcpwrap-etc=$td" "ip6-recv:$PROTO2!!ip6-sendto:[::1]:$PROTO1" 6 ip $PROTO1 0
fi ;; # NUMCOND, feats
esac
PROTO=$((PROTO+1))
N=$((N+1))


NAME=O_NOATIME_FILE
case "$TESTS" in
*%$N%*|*%functions%*|*%open%*|*%noatime%*|*%$NAME%*)
TEST="$NAME: option O_NOATIME on file"
# idea: create a file with o-noatime option; one second later create a file
# without this option (using touch); one second later read from the first file.
# Then we check which file has the later ATIME stamp. For this check we use
# "ls -ltu" because it is more portable than "test ... -nt ..."
if ! eval $NUMCOND; then :;
elif ! testoptions o-noatime >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}o-noatime not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.file"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
$PRINTF "test $F_n $TEST... " $N
CMD="$TRACE $SOCAT $opts -u open:\"${tf}1\",o-noatime /dev/null"
# generate a file
touch "${tf}1"
sleep 1
# generate a reference file
touch "${tf}2"
sleep 1
# read from the first file
$CMD 2>"$te"
if [ $? -ne 0 ]; then # command failed
    $PRINTF "${FAILED}:\n"
    echo "$CMD"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
# check which file has a later atime stamp
if [ $(ls -ltu "${tf}1" "${tf}2" |head -1 |sed 's/.* //') != "${tf}2" ];
then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD"
   cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "$te"; fi
   numOK=$((numOK+1))
fi # wrong time stamps
fi # command ok
fi ;; # NUMCOND, feats
esac
N=$((N+1))

NAME=O_NOATIME_FD
case "$TESTS" in
*%$N%*|*%functions%*|*%noatime%*|*%$NAME%*)
TEST="$NAME: option O_NOATIME on file descriptor"
# idea: use a fd of a file with o-noatime option; one second later create a file
# without this option (using touch); one second later read from the first file.
# Then we check which file has the later ATIME stamp. For this check we use
# "ls -ltu" because it is more portable than "test ... -nt ..."
if ! eval $NUMCOND; then :;
elif ! testoptions o-noatime >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}o-noatime not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.file"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
$PRINTF "test $F_n $TEST... " $N
touch ${tf}1
CMD="$TRACE $SOCAT $opts -u -,o-noatime /dev/null <${tf}1"
# generate a file, len >= 1
touch "${tf}1"
sleep 1
# generate a reference file
touch "${tf}2"
sleep 1
# read from the first file
sh -c "$CMD" 2>"$te"
if [ $? -ne 0 ]; then # command failed
    $PRINTF "${FAILED}:\n"
    echo "$CMD"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
# check which file has a later atime stamp
if [ $(ls -ltu "${tf}1" "${tf}2" |head -1 |sed 's/.* //') != "${tf}2" ];
then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD"
   cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "$te"; fi
   numOK=$((numOK+1))
fi # wrong time stamps
fi # command ok
fi ;; # NUMCOND, feats
esac
N=$((N+1))

NAME=EXT2_NOATIME
case "$TESTS" in
*%$N%*|*%functions%*|*%ext2%*|*%noatime%*|*%$NAME%*)
TEST="$NAME: extended file system options using ext2fs noatime option"
# idea: create a file with ext2-noatime option; one second later create a file
# without this option (using touch); one second later read from the first file.
# Then we check which file has the later ATIME stamp. For this check we use
# "ls -ltu" because it is more portable than "test ... -nt ..."
if ! eval $NUMCOND; then :;
elif ! testoptions ext2-noatime >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}ext2-noatime not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ts="$td/test$N.socket"
tf="$td/test$N.file"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1="$ts"
da="test$N $(date) $RANDOM"
$PRINTF "test $F_n $TEST... " $N
CMD0="$TRACE $SOCAT $opts -u /dev/null create:\"${tf}1\""
CMD="$TRACE $SOCAT $opts -u /dev/null create:\"${tf}1\",ext2-noatime"
# check if this is a capable FS; lsattr does other things on AIX, thus socat
$CMD0 2>"${te}0"
if [ $? -ne 0 ]; then
    $PRINTF "${YELLOW} cannot test${NORMAL}\n"
    numCANT=$((numCANT+1))
else
# generate a file with noatime, len >= 1
$CMD 2>"$te"
if [ $? -ne 0 ]; then # command failed
    $PRINTF "${YELLOW}impotent file system?${NORMAL}\n"
    echo "$CMD"
    cat "$te"
    numCANT=$((numCANT+1))
else
sleep 1
# generate a reference file
touch "${tf}2"
sleep 1
# read from the first file
cat "${tf}1" >/dev/null
# check which file has a later atime stamp
#if [ $(ls -ltu "${tf}1" "${tf}2" |head -n 1 |awk '{print($8);}') != "${tf}2" ];
if [ $(ls -ltu "${tf}1" "${tf}2" |head -n 1 |sed "s|.*\\($td.*\\)|\1|g") != "${tf}2" ];
then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD"
   cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "$te"; fi
   numOK=$((numOK+1))
fi
fi # not impotent
fi # can test
fi ;; # NUMCOND, feats
esac
N=$((N+1))


NAME=COOLWRITE
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%timeout%*|*%coolwrite%*|*%$NAME%*)
TEST="$NAME: option cool-write"
if ! eval $NUMCOND; then :;
elif ! testoptions cool-write >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}option cool-write not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
#set -vx
ti="$td/test$N.pipe"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
# a reader that will terminate after 1 byte
CMD1="$TRACE $SOCAT $opts -u pipe:\"$ti\",readbytes=1 /dev/null"
CMD="$TRACE $SOCAT $opts -u - file:\"$ti\",cool-write"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
bg=$!	# background process id
sleep 1
(echo .; sleep 1; echo) |$CMD 2>"$te"
rc=$?
kill $bg 2>/dev/null; wait
if [ $rc -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD &"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "$te"; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
N=$((N+1))


# test if option coolwrite can be applied to bidirectional address stdio
# this failed up to socat 1.6.0.0
NAME=COOLSTDIO
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%timeout%*|*%coolwrite%*|*%$NAME%*)
TEST="$NAME: option cool-write on bidirectional stdio"
# this test starts a socat reader that terminates after receiving one+ 
# bytes (option readbytes); and a test process that sends two bytes via
# named pipe to the receiving process and, a second later, sends another
# byte. The last write will fail with "broken pipe"; if option coolwrite
# has been applied successfully, socat will terminate with 0 (OK),
# otherwise with error.
if ! eval $NUMCOND; then :;
elif ! testoptions cool-write >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}option cool-write not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
#set -vx
ti="$td/test$N.pipe"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
# a reader that will terminate after 1 byte
CMD1="$TRACE $SOCAT $opts -u pipe:\"$ti\",readbytes=1 /dev/null"
CMD="$TRACE $SOCAT $opts -,cool-write pipe >\"$ti\""
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
bg=$!	# background process id
sleep 1
(echo .; sleep 1; echo) |eval "$CMD" 2>"$te"
rc=$?
kill $bg 2>/dev/null; wait
if [ $rc -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD &"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "$te"; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
N=$((N+1))


NAME=TCP4ENDCLOSE
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: end-close keeps TCP V4 socket open"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
p1=$PORT; PORT=$((PORT+1))
p2=$PORT
da1a="$(date) $RANDOM"
da1b="$(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -u - TCP4-CONNECT:$LOCALHOST:$p1"
CMD="$TRACE $SOCAT $opts -U TCP4:$LOCALHOST:$p2,end-close TCP4-LISTEN:$p1,bind=$LOCALHOST,reuseaddr,fork"
CMD3="$TRACE $SOCAT $opts -u TCP4-LISTEN:$p2,reuseaddr,bind=$LOCALHOST -"
printf "test $F_n $TEST... " $N
$CMD3 >"$tf" 2>"${te}3" &
pid3=$!
waittcp4port $p2 1
$CMD 2>"${te}2" &
pid2=$!
usleep $MICROS
waittcp4port $p1 1
echo "$da1a" |$CMD1 2>>"${te}1a"
echo "$da1b" |$CMD1 2>>"${te}1b"
sleep 1
kill "$pid3" "$pid2" 2>/dev/null
wait
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1a" "${te}1b" "${te}2" "${te}3"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! $ECHO "$da1a\n$da1b" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   cat "${te}1a" "${te}1b" "${te}2" "${te}3"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1a" "${te}1b" "${te}2" "${te}3"; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=EXECENDCLOSE
case "$TESTS" in
*%$N%*|*%functions%*|*%exec%*|*%$NAME%*)
TEST="$NAME: end-close keeps EXEC child running"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
ts="$td/test$N.sock"
tdiff="$td/test$N.diff"
da1a="$(date) $RANDOM"
da1b="$(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts - UNIX-CONNECT:$ts"
CMD="$TRACE $SOCAT $opts EXEC:"$CAT",end-close UNIX-LISTEN:$ts,fork"
printf "test $F_n $TEST... " $N
$CMD 2>"${te}2" &
pid2=$!
waitfile $ts 1
echo "$da1a" |$CMD1 2>>"${te}1a" >"$tf"
usleep $MICROS
echo "$da1b" |$CMD1 2>>"${te}1b" >>"$tf"
#usleep $MICROS
kill "$pid2" 2>/dev/null
wait
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1a" "${te}1b" "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! $ECHO "$da1a\n$da1b" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   cat "${te}1a" "${te}1b" "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1a" "${te}1b" "${te}2"; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))


# up to 1.7.0.0 option end-close led to an error with some address types due to
# bad internal handling. here we check it for address PTY
NAME=PTYENDCLOSE
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%pty%*|*%$NAME%*)
TEST="$NAME: PTY handles option end-close"
# with the bug, socat exits with error. we invoke socat in a no-op mode and
# check its return status.
if ! eval $NUMCOND; then :;
 else
tf="$td/test$N.stout"
te="$td/test$N.stderr"
CMD="$TRACE $SOCAT $opts /dev/null pty,end-close"
printf "test $F_n $TEST... " $N
$CMD 2>"${te}"
rc=$?
if [ "$rc" = 0 ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD"
    cat "${te}"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
N=$((N+1))


# test the shut-null and null-eof options
NAME=SHUTNULLEOF
case "$TESTS" in
*%$N%*|*%functions%*|*%socket%*|*%$NAME%*)
TEST="$NAME: options shut-null and null-eof"
# run a receiving background process with option null-eof. 
# start a sending process with option shut-null that sends a test record to the
# receiving process and then terminates.
# send another test record.
# whe the receiving process just got the first test record the test succeeded
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts -u UDP-RECV:$PORT,null-eof CREAT:$tf"
CMD1="$TRACE $SOCAT $opts -u - UDP-SENDTO:127.0.0.1:$PORT,shut-null"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
waitudp4port $PORT 1
echo "$da" |$CMD1 >"${tf}1" 2>"${te}1"
rc1=$?
echo "xyz" |$CMD1 >"${tf}2" 2>"${te}2"
rc2=$?
kill $pid0 2>/dev/null; wait
if [ $rc1 != 0 -o $rc2 != 0 ]; then
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif echo "$da" |diff - "${tf}" >"$tdiff"; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    cat "${tdiff}"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
N=$((N+1))


NAME=UDP6LISTENBIND
# this tests for a bug in (up to) 1.5.0.0:
#    with udp*-listen, the bind option supported only IPv4
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%ip6%*|*%ipapp%*|*%udp%*|*%udp6%*|*%$NAME%*)
TEST="$NAME: UDP6-LISTEN with bind"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs udp ip6) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}UDP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="$LOCALHOST6:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts UDP6-LISTEN:$tsl,reuseaddr,bind=$LOCALHOST6 PIPE"
CMD2="$TRACE $SOCAT $opts - UDP6:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1=$!
waitudp6port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2=$?
kill $pid1 2>/dev/null; wait
if [ $rc2 -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1" "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=TCPWRAPPERS_MULTIOPTS
# this tests for a bug in 1.5.0.0 that let socat fail when more than one 
# tcp-wrappers related option was specified in one address
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: use of multiple tcpwrapper enabling options"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip4 libwrap) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
ha="$td/hosts.allow"
$ECHO "test : ALL : allow" >"$ha"
CMD1="$TRACE $SOCAT $opts TCP4-LISTEN:$PORT,reuseaddr,hosts-allow=$ha,tcpwrap=test pipe"
CMD2="$TRACE $SOCAT $opts - TCP:$LOCALHOST:$PORT"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
waittcp4port $PORT
echo "$da" |$CMD2 >"$tf" 2>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=TCPWRAPPERS_TCP6ADDR
# this tests for a bug in 1.5.0.0 that brought false results with tcp-wrappers
# and IPv6 when 
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%tcp%*|*%tcp6%*|*%ip6%*|*%tcpwrap%*|*%$NAME%*)
TEST="$NAME: specification of TCP6 address in hosts.allow"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs tcp ip6 libwrap) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
ha="$td/hosts.allow"
hd="$td/hosts.deny"
$ECHO "socat : [::1] : allow" >"$ha"
$ECHO "ALL : ALL : deny" >"$hd"
CMD1="$TRACE $SOCAT $opts TCP6-LISTEN:$PORT,reuseaddr,tcpwrap-etc=$td,tcpwrappers=socat pipe"
CMD2="$TRACE $SOCAT $opts - TCP6:[::1]:$PORT"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
pid1=$!
waittcp6port $PORT
echo "$da" |$CMD2 >"$tf" 2>"${te}2"
kill $pid1 2>/dev/null; wait
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=UDP4BROADCAST
case "$TESTS" in
*%$N%*|*%functions%*|*%udp%*|*%udp4%*|*%ip4%*|*%dgram%*|*%broadcast%*|*%$NAME%*)
TEST="$NAME: UDP/IPv4 broadcast"
if ! eval $NUMCOND; then :;
elif [ -z "$BCADDR" ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}dont know a broadcast address${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PORT; PORT=$((PORT+1))
#ts1="$BCADDR/8:$ts1p"
ts1="$BCADDR:$ts1p"
ts2p=$PORT; PORT=$((PORT+1))
ts2="$BCIFADDR:$ts2p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts UDP4-RECVFROM:$ts1p,reuseaddr,broadcast PIPE"
#CMD2="$TRACE $SOCAT $opts - UDP4-BROADCAST:$ts1"
CMD2="$TRACE $SOCAT $opts - UDP4-DATAGRAM:$ts1,broadcast"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
pid1="$!"
waitudp4port $ts1p 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2="$?"
kill "$pid1" 2>/dev/null; wait;
if [ "$rc2" -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    echo "$CMD2"
    cat "${te}1"
    cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED\n"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$tut" ]; then
	echo "$CMD1 &"
	echo "$CMD2"
    fi
    if [ -n "$debug" ]; then cat $te; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
N=$((N+1))


NAME=IP4BROADCAST
# test a local broadcast of a raw IPv4 protocol.
# because we receive - in addition to the regular reply - our own broadcast,
# we use a token XXXX that is changed to YYYY in the regular reply packet.
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%rawip%*|*%rawip4%*|*%ip4%*|*%dgram%*|*%broadcast%*|*%root%*|*%$NAME%*)
TEST="$NAME: raw IPv4 broadcast"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 rawip) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}raw IP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ -z "$BCADDR" ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}dont know a broadcast address${NORMAL}\n" $N
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PROTO
#ts1="$BCADDR/8:$ts1p"
ts1="$BCADDR:$ts1p"
ts2p=$ts1p
ts2="$BCIFADDR"
da="test$N $(date) $RANDOM XXXX"
sh="$td/test$N-sed.sh"
echo 'sed s/XXXX/YYYY/' >"$sh"
chmod a+x "$sh"
CMD1="$TRACE $SOCAT $opts IP4-RECVFROM:$ts1p,reuseaddr,broadcast exec:$sh"
#CMD2="$TRACE $SOCAT $opts - IP4-BROADCAST:$ts1"
CMD2="$TRACE $SOCAT $opts - IP4-DATAGRAM:$ts1,broadcast"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
pid1="$!"
waitip4port $ts1p 1
echo "$da" |$CMD2 2>>"${te}2" |grep -v XXXX >>"$tf"
rc2="$?"
kill "$pid1" 2>/dev/null; wait;
if [ "$rc2" -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    echo "$CMD2"
    cat "${te}1"
    cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" | sed 's/XXXX/YYYY/'|diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED\n"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat $te; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
PROTO=$((PROTO+1))
N=$((N+1))


#NAME=UDP4BROADCAST_RANGE
#case "$TESTS" in
#*%$N%*|*%functions%*|*%security%*|*%udp%*|*%udp4%*|*%ip4%*|*%dgram%*|*%broadcast%*|*%range%*|*%$NAME%*)
#TEST="$NAME: security of UDP4-BROADCAST with RANGE option"
#if ! eval $NUMCOND; then :;
#elif [ -z "$BCADDR" ]; then
#    $PRINTF "test $F_n $TEST... ${YELLOW}dont know a broadcast address${NORMAL}\n" $N
#else
#testserversec "$N" "$TEST" "$opts -s" "UDP4-BROADCAST:$BCADDR/8:$PORT" "" "range=127.1.0.0:255.255.0.0" "udp4:127.1.0.0:$PORT" 4 udp $PORT 0
#fi ;; # NUMCOND, feats
#esac
#PORT=$((PORT+1))
#N=$((N+1))


NAME=UDP4MULTICAST_UNIDIR
case "$TESTS" in
*%$N%*|*%functions%*|*%udp%*|*%udp4%*|*%ip4%*|*%dgram%*|*%multicast%*|*%$NAME%*)
TEST="$NAME: UDP/IPv4 multicast, send only"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 udp) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PORT; PORT=$((PORT+1))
ts1a="$SECONDADDR"
ts1="$ts1a:$ts1p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT -u $opts UDP4-RECV:$ts1p,reuseaddr,ip-add-membership=224.255.255.254:$ts1a -"
CMD2="$TRACE $SOCAT -u $opts - UDP4-SENDTO:224.255.255.254:$ts1p,bind=$ts1a"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1"  >"${tf}" &
pid1="$!"
waitudp4port $ts1p 1
echo "$da" |$CMD2 2>>"${te}2"
rc2="$?"
usleep $MICROS
kill "$pid1" 2>/dev/null; wait;
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
N=$((N+1))

NAME=IP4MULTICAST_UNIDIR
case "$TESTS" in
*%$N%*|*%functions%*|*%rawip%*|*%ip4%*|*%dgram%*|*%multicast%*|*%root%*|*%$NAME%*)
TEST="$NAME: IPv4 multicast"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 rawip) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PROTO
ts1a="$SECONDADDR"
ts1="$ts1a:$ts1p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT -u $opts IP4-RECV:$ts1p,reuseaddr,ip-add-membership=224.255.255.254:$ts1a -"
CMD2="$TRACE $SOCAT -u $opts - IP4-SENDTO:224.255.255.254:$ts1p,bind=$ts1a"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1"  >"${tf}" &
pid1="$!"
waitip4proto $ts1p 1
usleep $MICROS
echo "$da" |$CMD2 2>>"${te}2"
rc2="$?"
#usleep $MICROS
sleep 1
kill "$pid1" 2>/dev/null; wait;
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
PROTO=$((PROTO+1))
N=$((N+1))

if false; then
NAME=UDP6MULTICAST_UNIDIR
case "$TESTS" in
*%$N%*|*%functions%*|*%udp%*|*%udp6%*|*%ip6%*|*%dgram%*|*%multicast%*|*%$NAME%*)
TEST="$NAME: UDP/IPv6 multicast"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip6 udp) || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PORT; PORT=$((PORT+1))
if1="$MCINTERFACE"
ts1a="[::1]"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT -u $opts UDP6-RECV:$ts1p,reuseaddr,ipv6-join-group=[ff02::2]:$if1 -"
CMD2="$TRACE $SOCAT -u $opts - UDP6-SENDTO:[ff02::2]:$ts1p,bind=$ts1a"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1"  >"${tf}" &
pid1="$!"
waitudp6port $ts1p 1
echo "$da" |$CMD2 2>>"${te}2"
rc2="$?"
usleep $MICROS
kill "$pid1" 2>/dev/null; wait;
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
N=$((N+1))
fi # false

NAME=UDP4MULTICAST_BIDIR
case "$TESTS" in
*%$N%*|*%functions%*|*%udp%*|*%udp4%*|*%ip4%*|*%dgram%*|*%multicast%*|*%$NAME%*)
TEST="$NAME: UDP/IPv4 multicast, with reply"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PORT; PORT=$((PORT+1))
ts1a="$SECONDADDR"
ts1="$ts1a:$ts1p"
ts2p=$PORT; PORT=$((PORT+1))
ts2="$BCIFADDR:$ts2p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts UDP4-RECVFROM:$ts1p,reuseaddr,ip-add-membership=224.255.255.254:$ts1a PIPE"
#CMD2="$TRACE $SOCAT $opts - UDP4-MULTICAST:224.255.255.254:$ts1p,bind=$ts1a"
CMD2="$TRACE $SOCAT $opts - UDP4-DATAGRAM:224.255.255.254:$ts1p,bind=$ts1a"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
pid1="$!"
waitudp4port $ts1p 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2="$?"
kill "$pid1" 2>/dev/null; wait;
if [ "$rc2" -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    echo "$CMD2"
    cat "${te}1"
    cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED\n"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$tut" ]; then
	echo "$CMD1 &"
	echo "$CMD2"
    fi
    if [ -n "$debug" ]; then cat $te; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))

NAME=IP4MULTICAST_BIDIR
case "$TESTS" in
*%$N%*|*%functions%*|*%rawip%*|*%ip4%*|*%dgram%*|*%multicast%*|*%root%*|*%$NAME%*)
TEST="$NAME: IPv4 multicast, with reply"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 rawip) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PROTO
ts1a="$SECONDADDR"
ts1="$ts1a:$ts1p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts IP4-RECVFROM:$ts1p,reuseaddr,ip-add-membership=224.255.255.254:$ts1a PIPE"
#CMD2="$TRACE $SOCAT $opts - IP4-MULTICAST:224.255.255.254:$ts1p,bind=$ts1a"
CMD2="$TRACE $SOCAT $opts - IP4-DATAGRAM:224.255.255.254:$ts1p,bind=$ts1a"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
pid1="$!"
waitip4port $ts1p 1
usleep 100000	# give process a chance to add multicast membership
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2="$?"
kill "$pid1" 2>/dev/null; wait;
if [ "$rc2" -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    echo "$CMD2"
    cat "${te}1"
    cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED\n"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$tut" ]; then
	echo "$CMD1 &"
	echo "$CMD2"
    fi
    if [ -n "$debug" ]; then cat $te; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
PROTO=$((PROTO+1))
N=$((N+1))


NAME=TUNREAD
case "$TESTS" in
*%$N%*|*%functions%*|*%tun%*|*%root%*|*%$NAME%*)
TEST="$NAME: reading data sent through tun interface"
#idea: create a TUN interface and send a datagram to one of the addresses of
# its virtual network. On the tunnel side, read the packet and compare its last
# bytes with the datagram payload
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 tun) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tl="$td/test$N.lock"
da="test$N $(date) $RANDOM"
dalen=$((${#da}+1))
TUNNET=10.255.255
CMD1="$TRACE $SOCAT $opts -u - UDP4-SENDTO:$TUNNET.2:$PORT"
#CMD="$TRACE $SOCAT $opts -u -L $tl TUN,ifaddr=$TUNNET.1,netmask=255.255.255.0,iff-up=1 -"
CMD="$TRACE $SOCAT $opts -u -L $tl TUN:$TUNNET.1/24,iff-up=1 -"
printf "test $F_n $TEST... " $N
$CMD 2>"${te}" |tail -c $dalen >"${tf}" &
sleep 1
echo "$da" |$CMD1 2>"${te}1"
sleep 1
kill "$(cat $tl 2>/dev/null)" 2>/dev/null
wait
if [ $? -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD &"
    echo "$CMD1"
    cat "${te}" "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED\n"
    echo "$CMD &"
    echo "$CMD1"
    cat "$tdiff"
    cat "${te}" "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}" "${te}1"; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


# use the INTERFACE address on a tun/tap device and transfer data fully
# transparent 
NAME=TUNINTERFACE
case "$TESTS" in
*%$N%*|*%functions%*|*%tun%*|*%interface%*|*%root%*|*%$NAME%*)
TEST="$NAME: pass data through tun interface using INTERFACE"
#idea: create a TUN interface and send a raw packet on the interface side.
# It should arrive unmodified on the tunnel side.
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs ip4 tun interface) || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tl="$td/test$N.lock"
da="$(date) $RANDOM"
dalen=$((${#da}+1))
TUNNET=10.255.255
TUNNAME=tun9
CMD1="$TRACE $SOCAT $opts -L $tl TUN:$TUNNET.1/24,iff-up=1,tun-type=tun,tun-name=$TUNNAME echo"
CMD="$TRACE $SOCAT $opts - INTERFACE:$TUNNAME"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}1" &
pid1="$!"
#waitinterface "$TUNNAME"
sleep 1
echo "$da" |$CMD 2>"${te}1" >"$tf" 2>"${te}"
kill $pid1 2>/dev/null
wait
if [ $? -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD &"
    echo "$CMD1"
    cat "${te}" "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED\n"
    echo "$CMD &"
    echo "$CMD1"
    cat "$tdiff"
    cat "${te}" "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}" "${te}1"; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=ABSTRACTSTREAM
case "$TESTS" in
*%$N%*|*%functions%*|*%unix%*|*%abstract%*|*%connect%*|*%listen%*|*%$NAME%*)
TEST="$NAME: abstract UNIX stream socket, listen and connect"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs abstract-unixsocket); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ts="$td/test$N.socket"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da1="test$N $(date) $RANDOM"
#establish a listening abstract unix socket
SRV="$TRACE $SOCAT $opts -lpserver ABSTRACT-LISTEN:\"$ts\" PIPE"
#make a connection
CMD="$TRACE $SOCAT $opts - ABSTRACT-CONNECT:$ts"
$PRINTF "test $F_n $TEST... " $N
touch "$ts"	# make a file with same name, so non-abstract fails
eval "$SRV 2>${te}s &"
pids=$!
#waitfile "$ts"
sleep 1
echo "$da1" |eval "$CMD" >"${tf}1" 2>"${te}1"
if [ $? -ne 0 ]; then
    kill "$pids" 2>/dev/null
    $PRINTF "$FAILED:\n"
    echo "$SRV &"
    cat "${te}s"
    echo "$CMD"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da1" |diff - "${tf}1" >"$tdiff"; then
    kill "$pids" 2>/dev/null
    $PRINTF "$FAILED:\n"
    echo "$SRV &"
    cat "${te}s"
    echo "$CMD"
    cat "${te}1"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi # !(rc -ne 0)
wait
fi ;; # NUMCOND, feats
esac
N=$((N+1))


NAME=ABSTRACTDGRAM
case "$TESTS" in
*%$N%*|*%functions%*|*%unix%*|*%abstract%*|*%dgram%*|*%$NAME%*)
TEST="$NAME: abstract UNIX datagram"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs abstract-unixsocket); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1="$td/test$N.socket1"
ts2="$td/test$N.socket2"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts ABSTRACT-RECVFROM:$ts1,reuseaddr PIPE"
#CMD2="$TRACE $SOCAT $opts - ABSTRACT-SENDTO:$ts1,bind=$ts2"
CMD2="$TRACE $SOCAT $opts - ABSTRACT-SENDTO:$ts1,bind=$ts2"
printf "test $F_n $TEST... " $N
touch "$ts1"	# make a file with same name, so non-abstract fails
$CMD1 2>"${te}1" &
pid1="$!"
sleep 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2=$?
kill "$pid1" 2>/dev/null; wait
if [ $rc2 -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   cat "${te}1"
   echo "$CMD2"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
N=$((N+1))


NAME=ABSTRACTRECV
case "$TESTS" in
*%$N%*|*%functions%*|*%unix%*|*%abstract%*|*%dgram%*|*%recv%*|*%$NAME%*)
TEST="$NAME: abstract UNIX datagram receive"
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs abstract-unixsocket); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ts="$td/test$N.socket"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1="$ts"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -u ABSTRACT-RECV:$ts1,reuseaddr -"
CMD2="$TRACE $SOCAT $opts -u - ABSTRACT-SENDTO:$ts1"
printf "test $F_n $TEST... " $N
touch "$ts1"	# make a file with same name, so non-abstract fails
$CMD1 >"$tf" 2>"${te}1" &
pid1="$!"
#waitfile $ts1 1
sleep 1
echo "$da" |$CMD2 2>>"${te}2"
rc2="$?"
i=0; while [ ! -s "$tf" -a "$i" -lt 10 ]; do  usleep 100000; i=$((i+1));  done
kill "$pid1" 2>/dev/null; wait
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
N=$((N+1))


# bind with Linux abstract UNIX domain addresses bound to filesystem socket
# instead of abstract namespace
NAME=ABSTRACT_BIND
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%socket%*|*%unix%*|*%abstract%*|*%$NAME%*)
TEST="$NAME: abstract bind"
# open an abstract client address with bind option, bind to the target socket.
# send a datagram. 
# when socat outputs the datagram it got the test succeeded
if ! eval $NUMCOND; then :; 
elif [ "$UNAME" != Linux ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}only on Linux${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1="$td/test$N.sock1"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts - ABSTRACT-SENDTO:$ts1,bind=$ts1"
printf "test $F_n $TEST... " $N
echo "$da" |$CMD1 >$tf 2>"${te}1"
rc1=$?
if [ $rc1 -ne 0 ]; then
    $PRINTF "$FAILED\n"
    echo "$CMD1" >&2
    echo "rc=$rc1" >&2
    cat "${te}1" >&2
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif echo "$da" |diff -q - $tf; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD1" >&2
    cat "${te}1" >&2
    echo "$da" |diff - "$tf" >&2
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


NAME=OPENSSLREAD
# socat determined availability of data using select(). With openssl, the
# following situation might occur:
# a SSL data block with more than 8192 bytes (socats default blocksize) 
# arrives; socat calls SSL_read, and the SSL routine reads the complete block.
# socat then reads 8192 bytes from the SSL layer, the rest remains buffered.
# If the TCP connection stays idle for some time, the data in the SSL layer
# keeps there and is not transferred by socat until the socket indicates more
# data or EOF.
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%$NAME%*)
TEST="$NAME: socat handles data buffered by openssl"
#idea: have a socat process (server) that gets an SSL block that is larger than
# socat transfer block size; keep the socket connection open and kill the
# server process after a short time; if not the whole data block has been
# transferred, the test has failed.
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs openssl) >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.out"
te="$td/test$N.err"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
SRVCERT=testsrv
gentestcert "$SRVCERT"
CMD1="$TRACE $SOCAT $opts -u -T 1 -b $($ECHO "$da\c" |wc -c) OPENSSL-LISTEN:$PORT,reuseaddr,cert=$SRVCERT.pem,verify=0 -"
CMD2="$TRACE $SOCAT $opts -u - OPENSSL-CONNECT:$LOCALHOST:$PORT,verify=0"
printf "test $F_n $TEST... " $N
#
$CMD1 2>"${te}1" >"$tf" &
pid=$!	# background process id
waittcp4port $PORT
(echo "$da"; sleep 2) |$CMD2 2>"${te}2"
kill "$pid" 2>/dev/null; wait
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1"
    cat "${te}1"
    echo "$CMD2"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
wait
fi # NUMCOND, featsesac
 ;;
esac
N=$((N+1))


# test: there is a bug with the readbytes option: when the socket delivered
# exacly that many bytes as specified with readbytes and the stays idle (no
# more data, no EOF), socat waits for more data instead of generating EOF on
# this in put stream.
NAME=READBYTES_EOF
#set -vx
case "$TESTS" in
*%$N%*|*%functions%*|*%$NAME%*)
TEST="$NAME: trigger EOF after that many bytes, even when socket idle"
#idea: we deliver that many bytes to socat; the process should terminate then.
# we try to transfer data in the other direction then; if transfer succeeds,
# the process did not terminate and the bug is still there.
if ! eval $NUMCOND; then :;
elif false; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tr="$td/test$N.ref"
ti="$td/test$N.in"
to="$td/test$N.out"
te="$td/test$N.err"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"; da="$da$($ECHO '\r')"
CMD="$TRACE $SOCAT $opts SYSTEM:\"echo A; sleep $((2*SECONDs))\",readbytes=2!!- -!!/dev/null"
printf "test $F_n $TEST... " $N
(usleep $((2*MICROS)); echo) |eval "$CMD" >"$to" 2>"$te"
if test -s "$to"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, feats
esac
N=$((N+1))


# test: there was a bug with exec:...,pty that did not kill the exec'd sub
# process under some circumstances.
NAME=EXECPTYKILL
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%exec%*|*%$NAME%*)
TEST="$NAME: exec:...,pty explicitely kills sub process"
# we want to check if the exec'd sub process is killed in time
# for this we have a shell script that generates a file after two seconds;
# it should be killed after one second, so if the file was generated the test
# has failed
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
ts="$td/test$N.sock"
tda="$td/test$N.data"
tsh="$td/test$N.sh"
tdiff="$td/test$N.diff"
cat >"$tsh" <<EOF
sleep $SECONDs; echo; sleep $SECONDs;  touch "$tda"; echo
EOF
chmod a+x "$tsh"
CMD1="$TRACE $SOCAT $opts -t $SECONDs -U UNIX-LISTEN:$ts,fork EXEC:$tsh,pty"
CMD="$TRACE $SOCAT $opts -t $SECONDs /dev/null UNIX-CONNECT:$ts"
printf "test $F_n $TEST... " $N
$CMD1 2>"${te}2" &
pid1=$!
sleep $SECONDs
waitfile $ts $SECONDs
$CMD 2>>"${te}1" >>"$tf"
sleep $((2*SECONDs))
kill "$pid1" 2>/dev/null
wait
if [ $? -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    echo "$CMD2"
    cat "${te}1" "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif [ -f "$tda" ]; then
    $PRINTF "$FAILED\n"
    cat "${te}1" "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


# test if service name resolution works; this was buggy in 1.5 and 1.6.0.0
NAME=TCP4SERVICE
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%ipapp%*|*%tcp%*|*%$NAME%*)
TEST="$NAME: echo via connection to TCP V4 socket"
# select a tcp entry from /etc/services, have a server listen on the port 
# number and connect using the service name; with the bug, connection will to a
# wrong port
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
# find a service entry we do not need root for (>=1024; here >=1100 for ease)
SERVENT="$(grep '^[a-z][a-z]*[^!-~][^!-~]*[1-9][1-9][0-9][0-9]/tcp' /etc/services |head -n 1)"
SERVICE="$(echo $SERVENT |cut -d' ' -f1)"
_PORT="$PORT"
PORT="$(echo $SERVENT |sed 's/.* \([1-9][0-9]*\).*/\1/')"
tsl="$PORT"
ts="127.0.0.1:$SERVICE"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts TCP4-LISTEN:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout TCP4:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1=$!
waittcp4port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   cat "${te}1"
   echo "$CMD2"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid1 2>/dev/null
wait
PORT="$_PORT"
fi ;; # NUMCOND
esac
N=$((N+1))


# test: up to socat 1.6.0.0, the highest file descriptor supported in socats
# transfer engine was FOPEN_MAX-1; this usually worked fine but would fail when
# socat was invoked with many file descriptors already opened. socat would 
# just hang in the select() call. Thanks to Daniel Lucq for reporting this
# problem. 
# FOPEN_MAX on different OS's:
#   OS			FOPEN_	ulimit	ulimit	FD_
#			MAX	-H -n	-S -n	SETSIZE
#   Linux 2.6:		16	1024	1024	1024
#   HP-UX 11.11:	60	2048	2048	2048
#   FreeBSD:		20	11095	11095	1024
#   Cygwin:		20	unlimit	256	64
#   AIX:		32767	65534		65534
#   SunOS 8:		20			1024
NAME=EXCEED_FOPEN_MAX
case "$TESTS" in
*%$N%*|*%functions%*|*%maxfds%*|*%$NAME%*)
TEST="$NAME: more than FOPEN_MAX FDs in use"
# this test opens a number of FDs before socat is invoked. socat will have to
# allocate higher FD numbers and thus hang if it cannot handle them.
if ! eval $NUMCOND; then :; else
REDIR=
#set -vx
FOPEN_MAX=$($PROCAN -c 2>/dev/null |grep '^#define[ ][ ]*FOPEN_MAX' |awk '{print($3);}')
if [ -z "$FOPEN_MAX" ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}could not determine FOPEN_MAX${NORMAL}\n" "$N"
    numCANT=$((numCANT+1))
else
OPEN_FILES=$FOPEN_MAX	# more than the highest FOPEN_MAX
i=3; while [ "$i" -lt "$OPEN_FILES" ]; do
    REDIR="$REDIR $i>&2"
    i=$((i+1))
done
#echo "$REDIR"
#testecho "$N" "$TEST" "" "pipe" "$opts -T 3" "" 1 
#set -vx
eval testecho "\"$N\"" "\"$TEST\"" "\"\"" "pipe" "\"$opts -T $((2*SECONDs))\"" 1 $REDIR
#set +vx
fi # could determine FOPEN_MAX
fi ;; # NUMCOND
esac
N=$((N+1))


# there was a bug with udp-listen and fork: terminating sub processes became
# zombies because the master process did not catch SIGCHLD
NAME=UDP4LISTEN_SIGCHLD
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%ipapp%*|*%udp%*|*%zombie%*|*%signal%*|*%$NAME%*)
TEST="$NAME: test if UDP4-LISTEN child becomes zombie"
# idea: run a udp-listen process with fork and -T. Connect once, so a sub
# process is forked off. Make some transfer and wait until the -T timeout is
# over. Now check for the child process: if it is zombie the test failed. 
# Correct is that child process terminated
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="$LOCALHOST:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -T 0.5 UDP4-LISTEN:$tsl,reuseaddr,fork PIPE"
CMD2="$TRACE $SOCAT $opts - UDP4:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1=$!
waitudp4port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2=$?
sleep 1
#read -p ">"
l="$(childprocess $pid1)"
kill $pid1 2>/dev/null; wait
if [ $rc2 -ne 0 ]; then
    $PRINTF "$NO_RESULT (client failed)\n"	# already handled in test UDP4STREAM
    numCANT=$((numCANT+1))
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$NO_RESULT (diff failed)\n"	# already handled in test UDP4STREAM
    numCANT=$((numCANT+1))
elif $(isdefunct "$l"); then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    echo "$CMD2"
    cat "${te}1" "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))
#set +vx

# there was a bug with udp-recvfrom and fork: terminating sub processes became
# zombies because the master process caught SIGCHLD but did not wait()
NAME=UDP4RECVFROM_SIGCHLD
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%udp%*|*%dgram%*|*%zombie%*|*%signal%*|*%$NAME%*)
TEST="$NAME: test if UDP4-RECVFROM child becomes zombie"
# idea: run a udp-recvfrom process with fork and -T. Send it one packet, so a
# sub process is forked off. Make some transfer and wait until the -T timeout
# is over. Now check for the child process: if it is zombie the test failed. 
# Correct is that child process terminated
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="$LOCALHOST:$tsl"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -T 0.5 UDP4-RECVFROM:$tsl,reuseaddr,fork PIPE"
CMD2="$TRACE $SOCAT $opts - UDP4-SENDTO:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1=$!
waitudp4port $tsl 1
echo "$da" |$CMD2 >>"$tf" 2>>"${te}2"
rc2=$?
sleep 1
#read -p ">"
l="$(childprocess $pid1)"
kill $pid1 2>/dev/null; wait
if [ $rc2 -ne 0 ]; then
    $PRINTF "$NO_RESULT\n"	# already handled in test UDP4DGRAM
    numCANT=$((numCANT+1))
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$NO_RESULT\n"	# already handled in test UDP4DGRAM
    numCANT=$((numCANT+1))
elif $(isdefunct "$l"); then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    echo "$CMD2"
    cat "${te}1" "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


# test: there was a bug with ip*-recv and bind option: it would not bind, and
# with the first received packet an error:
# socket_init(): unknown address family 0
# occurred
NAME=RAWIP4RECVBIND
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%dgram%*|*%rawip%*|*%rawip4%*|*%recv%*|*%root%*|*%$NAME%*)
TEST="$NAME: raw IPv4 receive with bind"
# idea: start a socat process with ip4-recv:...,bind=... and send it a packet
# if the packet passes the test succeeded
if ! eval $NUMCOND; then :;
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PROTO; PROTO=$((PROTO+1))
ts1a="127.0.0.1"
ts1="$ts1a:$ts1p"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -u IP4-RECV:$ts1p,bind=$ts1a,reuseaddr -"
CMD2="$TRACE $SOCAT $opts -u - IP4-SENDTO:$ts1"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1="$!"
waitip4proto $ts1p 1
echo "$da" |$CMD2 2>>"${te}2"
rc2="$?"
#ls -l $tf
i=0; while [ ! -s "$tf" -a "$i" -lt 10 ]; do  usleep 100000; i=$((i+1));  done
kill "$pid1" 2>/dev/null; wait
if [ "$rc2" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   echo "$CMD2"
   cat "${te}1"
   cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND, root
esac
PROTO=$((PROTO+1))
N=$((N+1))


# there was a bug in *-recvfrom with fork: due to an error in the appropriate
# signal handler the master process would hang after forking off the first
# child process.
NAME=UDP4RECVFROM_FORK
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%udp%*|*%dgram%*|*%$NAME%*)
TEST="$NAME: test if UDP4-RECVFROM handles more than one packet"
# idea: run a UDP4-RECVFROM process with fork and -T. Send it one packet;
# send it a second packet and check if this is processed properly. If yes, the
# test succeeded.
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsp=$PORT
ts="$LOCALHOST:$tsp"
da="test$N $(date) $RANDOM"
CMD1="$TRACE $SOCAT $opts -T 2 UDP4-RECVFROM:$tsp,reuseaddr,fork PIPE"
CMD2="$TRACE $SOCAT $opts -T 1 - UDP4-SENDTO:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >/dev/null 2>"${te}1" &
pid1=$!
waitudp4port $tsp 1
echo "$da" |$CMD2 >/dev/null 2>>"${te}2"	# this should always work
rc2a=$?
sleep 1
echo "$da" |$CMD2 >"$tf" 2>>"${te}3"		# this would fail when bug
rc2b=$?
kill $pid1 2>/dev/null; wait
if [ $rc2b -ne 0 ]; then
    $PRINTF "$NO_RESULT\n"
    numCANT=$((numCANT+1))
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD1 &"
    echo "$CMD2"
    cat "${te}1" "${te}2" "${te}3"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}1" "${te}2" "${te}3"; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


# there was a bug in parsing the arguments of exec: consecutive spaces resulted
# in additional empty arguments
NAME=EXECSPACES
case "$TESTS" in
*%$N%*|*%functions%*|*%exec%*|*%parse%*|*%$NAME%*)
TEST="$NAME: correctly parse exec with consecutive spaces"
if ! eval $NUMCOND; then :; else
$PRINTF "test $F_n $TEST... " $N
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
da="test$N $(date)  $RANDOM"	# with a double space
tdiff="$td/test$N.diff"
# put the test data as first argument after two spaces. expect the data in the
# first argument of the exec'd command.
$TRACE $SOCAT $opts -u "exec:\"bash -c \\\"echo \\\\\\\"\$1\\\\\\\"\\\"  \\\"\\\" \\\"$da\\\"\"" - >"$tf" 2>"$te"
rc=$?
echo "$da" |diff - "$tf" >"$tdiff"
if [ "$rc" -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif [ -s "$tdiff" ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo diff:
    cat "$tdiff"
    if [ -n "$debug" ]; then cat $te; fi
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat $te; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))


# a bug was found in the way UDP-LISTEN handles the listening socket:
# when UDP-LISTEN continued to listen after a packet had been dropped by, e.g.,
# range option, the old listen socket would not be closed but a new one created.
NAME=UDP4LISTENCONT
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%ip4%*|*%udp%*|*%$NAME%*)
TEST="$NAME: let range drop a packet and see if old socket is closed"
# idea: run a UDP4-LISTEN process with range option. Send it one packet from an
# address outside range and check if two listening sockets are open then
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
while [ "$(netstat -an |grep "^udp.*127.0.0.1:$PORT" |wc -l)" -ne 0 ]; do
    PORT=$((PORT+1))
done
tp=$PORT
da1="test$N $(date) $RANDOM"
a1="$LOCALHOST"
a2="$SECONDADDR"
#CMD0="$TRACE $SOCAT $opts UDP4-LISTEN:$tp,bind=$a1,range=$a2/32 PIPE"
CMD0="$TRACE $SOCAT $opts UDP4-LISTEN:$tp,range=$a2/32 PIPE"
CMD1="$TRACE $SOCAT $opts - UDP-CONNECT:$a1:$tp"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid1=$!
waitudp4port $tp 1
echo "$da1" |$CMD1 >"${tf}1" 2>"${te}1"	# this should fail
rc1=$?
waitudp4port $tp 1
nsocks="$(netstat -an |grep "^udp.*[:.]$PORT" |wc -l)"
kill $pid1 2>/dev/null; wait
if [ $rc1 -ne 0 ]; then
    $PRINTF "$NO_RESULT\n"
    numCANT=$((numCANT+1))
elif [ $nsocks -eq 0 ]; then
    $PRINTF "$NO_RESULT\n"
    numCANT=$((numCANT+1))
elif [ $nsocks -ne 1 ]; then
    $PRINTF "$FAILED ($nsocks listening sockets)\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0" "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}0" "${te}1" "${te}2"; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


# during wait for next poll time option ignoreeof blocked the data transfer in
# the reverse direction
NAME=IGNOREEOFNOBLOCK
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%socket%*|*%ignoreeof%*|*%$NAME%*)
TEST="$NAME: ignoreeof does not block other direction"
# have socat poll in ignoreeof mode. while it waits one second for next check,
# we send data in the reverse direction and then the total timeout fires.
# it the data has passed, the test succeeded.
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts /dev/null,ignoreeof!!- -!!/dev/null"
printf "test $F_n $TEST... " $N
(usleep 333333; echo "$da") |$CMD0 >"$tf" 2>"${te}0"
rc0=$?
if [ $rc0 != 0 ]; then
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif echo "$da" |diff - "$tf" >/dev/null; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi ;; # NUMCOND
esac
N=$((N+1))


# test the escape option
NAME=ESCAPE
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%escape%*|*%$NAME%*)
TEST="$NAME: escape character triggers EOF"
# idea: start socat just echoing input, but apply escape option. send a string
# containing the escape character and check if the output is truncated
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD="$TRACE $SOCAT $opts -,escape=27 pipe"
printf "test $F_n $TEST... " $N
$ECHO "$da\n\x1bXYZ" |$CMD >"$tf" 2>"$te"
if [ $? -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: diff:\n"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat $te; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))

# test the escape option combined with ignoreeof
NAME=ESCAPE_IGNOREEOF
case "$TESTS" in
*%$N%*|*%functions%*|*%engine%*|*%ignoreeof%*|*%escape%*|*%$NAME%*)
TEST="$NAME: escape character triggers EOF"
# idea: start socat just echoing input, but apply escape option. send a string
# containing the escape character and check if the output is truncated
if ! eval $NUMCOND; then :; else
ti="$td/test$N.file"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD="$TRACE $SOCAT -T 5 $opts file:$ti,ignoreeof,escape=27!!- pipe"
printf "test $F_n $TEST... " $N
>"$ti"
$CMD >"$tf" 2>"$te" &
$ECHO "$da\n\x1bXYZ" >>"$ti"
sleep 1
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: diff:\n"
    cat "$tdiff"
    cat "$te"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat $te; fi
    numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))


# test: logging of ancillary message
while read PF KEYW ADDR IPPORT SCM_ENABLE SCM_RECV SCM_TYPE SCM_NAME ROOT SCM_VALUE
do
if [ -z "$PF" ] || [[ "$PF" == \#* ]]; then continue; fi
#
pf="$(echo "$PF" |tr A-Z a-z)"
proto="$(echo "$KEYW" |tr A-Z a-z)"
NAME=${KEYW}SCM_$SCM_TYPE
case "$TESTS" in
*%$N%*|*%functions%*|*%socket%*|*%$pf%*|*%dgram%*|*%udp%*|*%$proto%*|*%recv%*|*%ancillary%*|*%$ROOT%*|*%$NAME%*)
TEST="$NAME: $KEYW log ancillary message $SCM_TYPE $SCM_NAME"
# idea: start a socat process with *-RECV:..,... , ev. with ancillary message
# enabling option and send it a packet, ev. with some option. check the info log
# for the appropriate output.
if ! eval $NUMCOND; then :;
#elif [[ "$PF" == "#*" ]]; then :
elif [ "$ROOT" = root -a $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ "$PF" = "IP6" ] && ( ! feat=$(testaddrs ip6) || ! runsip6 >/dev/null ); then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testoptions $SCM_RECV >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}option $SCM_RECV not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
case "X$IPPORT" in
    "XPORT")
    tra="$PORT"		# test recv address
    tsa="$ADDR:$PORT"	# test sendto address
    PORT=$((PORT+1)) ;;
    "XPROTO")
    tra="$PROTO"		# test recv address
    tsa="$ADDR:$PROTO"	# test sendto address
    PROTO=$((PROTO+1)) ;;
    *)
    tra="$(eval echo "$ADDR")"	# resolve $N
    tsa="$tra"
esac
CMD0="$TRACE $SOCAT $opts -d -d -d -u $KEYW-RECV:$tra,reuseaddr,$SCM_RECV -"
CMD1="$TRACE $SOCAT $opts -u - $KEYW-SENDTO:$tsa,$SCM_ENABLE"
printf "test $F_n $TEST... " $N
# is this option supported?
if $TRACE $SOCAT -hhh |grep "[[:space:]]$SCM_RECV[[:space:]]" >/dev/null; then
$CMD0 >"$tf" 2>"${te}0" &
pid0="$!"
wait${proto}port $tra 1
echo "XYZ" |$CMD1 2>"${te}1"
rc1="$?"
sleep 1
i=0; while [ ! -s "${te}0" -a "$i" -lt 10 ]; do  usleep 100000; i=$((i+1));  done
kill "$pid0" 2>/dev/null; wait
# do not show more messages than requested
case "$opts" in
*-d*-d*-d*-d*) LEVELS="[EWNID]" ;;
*-d*-d*-d*)    LEVELS="[EWNI]" ;;
*-d*-d*)       LEVELS="[EWN]" ;;
*-d*)          LEVELS="[EW]" ;;
*)             LEVELS="[E]" ;;
esac
if [ "$SCM_VALUE" = "timestamp" ]; then
    SCM_VALUE="$(date '+%a %b %e %H:%M:.. %Y'), ...... usecs"
fi
if [ "$rc1" -ne 0 ]; then
    $PRINTF "$NO_RESULT: $TRACE $SOCAT:\n"
    echo "$CMD0 &"
    echo "$CMD1"
    grep " $LEVELS " "${te}0"
    grep " $LEVELS " "${te}1"
    numCANT=$((numCANT+1))
elif ! grep "ancillary message: $SCM_TYPE: $SCM_NAME=" ${te}0 >/dev/null; then
    $PRINTF "$FAILED\n"
    echo "variable $SCM_TYPE: $SCM_NAME not set"
    echo "$CMD0 &"
    echo "$CMD1"
    grep " $LEVELS " "${te}0"
    grep " $LEVELS " "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! grep "ancillary message: $SCM_TYPE: $SCM_NAME=$SCM_VALUE\$" ${te}0 >/dev/null; then
    $PRINTF "$FAILED\n"
    badval="$(grep "ancillary message: $SCM_TYPE: $SCM_NAME" ${te}0 |sed 's/.*=//g')"
    echo "variable $SCM_TYPE: $SCM_NAME has value \"$badval\" instead of pattern \"$SCM_VALUE\"" >&2
    echo "$CMD0 &"
    echo "$CMD1"
    grep " $LEVELS " "${te}0"
    grep " $LEVELS " "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then
	grep " $LEVELS " "${te}0"; echo; grep " $LEVELS " "${te}1";
    fi
    numOK=$((numOK+1))
fi
else # option is not supported
    $PRINTF "${YELLOW}$SCM_RECV not available${NORMAL}\n"
    numCANT=$((numCANT+1))
fi # option is not supported
fi # NUMCOND, root, feats
 ;;
esac
N=$((N+1))
#
done <<<"
IP4  UDP4 127.0.0.1 PORT  ip-options=x01000000 ip-recvopts       IP_OPTIONS     options   user x01000000
IP4  UDP4 127.0.0.1 PORT  ,                    so-timestamp      SCM_TIMESTAMP  timestamp user timestamp
IP4  UDP4 127.0.0.1 PORT  ip-ttl=53            ip-recvttl        IP_TTL         ttl       user 53
IP4  UDP4 127.0.0.1 PORT  ip-tos=7             ip-recvtos        IP_TOS         tos       user 7
IP4  UDP4 127.0.0.1 PORT  ,                    ip-pktinfo        IP_PKTINFO     locaddr   user 127.0.0.1
IP4  UDP4 127.0.0.1 PORT  ,                    ip-pktinfo        IP_PKTINFO     dstaddr   user 127.0.0.1
IP4  UDP4 127.0.0.1 PORT  ,                    ip-pktinfo        IP_PKTINFO     if        user lo
IP4  UDP4 127.0.0.1 PORT  ,                    ip-recvif         IP_RECVIF      if        user lo0
IP4  UDP4 127.0.0.1 PORT  ,                    ip-recvdstaddr    IP_RECVDSTADDR dstaddr   user 127.0.0.1
IP4  IP4  127.0.0.1 PROTO ip-options=x01000000 ip-recvopts       IP_OPTIONS     options   root x01000000
IP4  IP4  127.0.0.1 PROTO ,                    so-timestamp      SCM_TIMESTAMP  timestamp root timestamp
IP4  IP4  127.0.0.1 PROTO ip-ttl=53            ip-recvttl        IP_TTL         ttl       root 53
IP4  IP4  127.0.0.1 PROTO ip-tos=7             ip-recvtos        IP_TOS         tos       root 7
IP4  IP4  127.0.0.1 PROTO ,                    ip-pktinfo        IP_PKTINFO     locaddr   root 127.0.0.1
IP4  IP4  127.0.0.1 PROTO ,                    ip-pktinfo        IP_PKTINFO     dstaddr   root 127.0.0.1
IP4  IP4  127.0.0.1 PROTO ,                    ip-pktinfo        IP_PKTINFO     if        root lo
IP4  IP4  127.0.0.1 PROTO ,                    ip-recvif         IP_RECVIF      if        root lo0
IP4  IP4  127.0.0.1 PROTO ,                    ip-recvdstaddr    IP_RECVDSTADDR dstaddr   root 127.0.0.1
IP6  UDP6 [::1]     PORT  ,                    so-timestamp      SCM_TIMESTAMP  timestamp user timestamp
IP6  UDP6 [::1]     PORT  ,                    ipv6-recvpktinfo  IPV6_PKTINFO   dstaddr   user [[]0000:0000:0000:0000:0000:0000:0000:0001[]]
IP6  UDP6 [::1]     PORT  ipv6-unicast-hops=35 ipv6-recvhoplimit IPV6_HOPLIMIT  hoplimit  user 35
IP6  UDP6 [::1]     PORT  ipv6-tclass=0xaa     ipv6-recvtclass   IPV6_TCLASS    tclass    user x000000aa
IP6  IP6  [::1]     PROTO ,                    so-timestamp      SCM_TIMESTAMP  timestamp root timestamp
IP6  IP6  [::1]     PROTO ,                    ipv6-recvpktinfo  IPV6_PKTINFO   dstaddr   root [[]0000:0000:0000:0000:0000:0000:0000:0001[]]
IP6  IP6  [::1]     PROTO ipv6-unicast-hops=35 ipv6-recvhoplimit IPV6_HOPLIMIT  hoplimit  root 35
IP6  IP6  [::1]     PROTO ipv6-tclass=0xaa     ipv6-recvtclass   IPV6_TCLASS    tclass    root x000000aa
#UNIX UNIX $td/test\$N.server - ,               so-timestamp      SCM_TIMESTAMP  timestamp user timestamp
"
# this one fails, appearently due to a Linux weakness:
# UNIX so-timestamp


# test: setting of environment variables that describe a stream socket
# connection: SOCAT_SOCKADDR, SOCAT_PEERADDR; and SOCAT_SOCKPORT,
# SOCAT_PEERPORT when applicable
while read KEYW FEAT TEST_SOCKADDR TEST_PEERADDR TEST_SOCKPORT TEST_PEERPORT; do
if [ -z "$KEYW" ] || [[ "$KEYW" == \#* ]]; then continue; fi
#
test_proto="$(echo "$KEYW" |tr A-Z a-z)"
NAME=${KEYW}LISTENENV
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%ipapp%*|*%tcp%*|*%$test_proto%*|*%envvar%*|*%$NAME%*)
TEST="$NAME: $KEYW-LISTEN sets environment variables with socket addresses"
# have a server accepting a connection and invoking some shell code. The shell
# code extracts and prints the SOCAT related environment vars.
# outside code then checks if the environment contains the variables correctly
# describing the peer and local sockets.
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs $FEAT); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat" |tr a-z A-Z) not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ "$KEYW" = "TCP6" -o "$KEYW" = "UDP6" -o "$KEYW" = "SCTP6" ] && \
    ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ "$KEYW" = "SCTP4" ] && ! runssctp4 "$((PORT))"; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$KEYW not available${NORMAL}\n" $N
elif [ "$KEYW" = "SCTP6" ] && ! runssctp6 "$((PORT))"; then
    #!!! branch not reached - caught above!
    $PRINTF "test $F_n $TEST... ${YELLOW}$KEYW not available${NORMAL}\n" $N
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
TEST_SOCKADDR="$(echo "$TEST_SOCKADDR" |sed "s/\$N/$N/g")"	# actual vars
tsa="$TEST_SOCKADDR"	# test server address
tsp="$TEST_SOCKPORT"	# test server port
if [ "$tsp" != ',' ]; then
    tsa1="$tsp"; tsa2="$tsa"; tsa="$tsa:$tsp"	# tsa2 used for server bind=
else
    tsa1="$tsa"; tsa2=				# tsa1 used for addr parameter
fi
TEST_PEERADDR="$(echo "$TEST_PEERADDR" |sed "s/\$N/$N/g")"	# actual vars
tca="$TEST_PEERADDR"	# test client address
tcp="$TEST_PEERPORT"	# test client port
if [ "$tcp" != ',' ]; then
    tca="$tca:$tcp"
fi
#CMD0="$TRACE $SOCAT $opts -u $KEYW-LISTEN:$tsa1 SYSTEM:\"export -p\""
CMD0="$TRACE $SOCAT $opts -u -lpsocat $KEYW-LISTEN:$tsa1 SYSTEM:\"echo SOCAT_SOCKADDR=\\\$SOCAT_SOCKADDR; echo SOCAT_PEERADDR=\\\$SOCAT_PEERADDR; echo SOCAT_SOCKPORT=\\\$SOCAT_SOCKPORT; echo SOCAT_PEERPORT=\\\$SOCAT_PEERPORT; sleep 1\""
CMD1="$TRACE $SOCAT $opts -u - $KEYW-CONNECT:$tsa,bind=$tca"
printf "test $F_n $TEST... " $N
eval "$CMD0 2>\"${te}0\" >\"$tf\" &"
pid0=$!
wait${test_proto}port $tsa1 1
echo |$CMD1 2>"${te}1"
rc1=$?
waitfile "$tf" 2
kill $pid0 2>/dev/null; wait
#set -vx
if [ $rc1 != 0 ]; then
    $PRINTF "$NO_RESULT (client failed):\n"
    echo "$CMD0 &"
    cat "${te}0"
    echo "$CMD1"
    cat "${te}1"
    numCANT=$((numCANT+1))
elif [ "$(grep SOCAT_SOCKADDR "${tf}" |sed -e 's/^[^=]*=//' |sed -e "s/[\"']//g")" = "$TEST_SOCKADDR" -a \
    "$(grep SOCAT_PEERADDR "${tf}" |sed -e 's/^[^=]*=//' -e "s/[\"']//g")" = "$TEST_PEERADDR" -a \
    \( "$TEST_SOCKPORT" = ',' -o "$(grep SOCAT_SOCKPORT "${tf}" |sed -e 's/^[^=]*=//' |sed -e 's/"//g')" = "$tsp" \) -a \
    \( "$TEST_PEERPORT" = ',' -o "$(grep SOCAT_PEERPORT "${tf}" |sed -e 's/^[^=]*=//' |sed -e 's/"//g')" = "$tcp" \) \
    ]; then
    $PRINTF "$OK\n"
    if [ "$debug" ]; then
	echo "$CMD0 &"
	cat "${te}0"
	echo "$CMD1"
	cat "${te}1"
    fi
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    cat "${te}0"
    echo "$CMD1"
    cat "${te}1"
    echo -e "SOCAT_SOCKADDR=$TEST_SOCKADDR\nSOCAT_PEERADDR=$TEST_PEERADDR\nSOCAT_SOCKPORT=$TEST_SOCKPORT\nSOCAT_PEERPORT=$TEST_PEERPORT" |
    diff - "${tf}"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND, feats
 ;;
esac
N=$((N+1))
#set +xv
#
done <<<"
TCP4  TCP  127.0.0.1                                 $SECONDADDR                               $PORT       $((PORT+1))
TCP6  IP6  [0000:0000:0000:0000:0000:0000:0000:0001] [0000:0000:0000:0000:0000:0000:0000:0001] $((PORT+2)) $((PORT+3))
UDP6  IP6  [0000:0000:0000:0000:0000:0000:0000:0001] [0000:0000:0000:0000:0000:0000:0000:0001] $((PORT+6)) $((PORT+7))
SCTP4 SCTP 127.0.0.1                                 $SECONDADDR                               $((PORT+8)) $((PORT+9))
SCTP6 SCTP [0000:0000:0000:0000:0000:0000:0000:0001] [0000:0000:0000:0000:0000:0000:0000:0001] $((PORT+10)) $((PORT+11))
UNIX  UNIX $td/test\$N.server                        $td/test\$N.client                        ,           ,
"
# this one fails due to weakness in socats UDP4-LISTEN implementation:
#UDP4 $LOCALHOST $SECONDADDR $((PORT+4)) $((PORT+5))


# test: environment variables from ancillary message
while read PF KEYW ADDR IPPORT SCM_ENABLE SCM_RECV SCM_ENVNAME ROOT SCM_VALUE
do
if [ -z "$PF" ] || [[ "$PF" == \#* ]]; then continue; fi
#
pf="$(echo "$PF" |tr A-Z a-z)"
proto="$(echo "$KEYW" |tr A-Z a-z)"
NAME=${KEYW}ENV_$SCM_ENVNAME
case "$TESTS" in
*%$N%*|*%functions%*|*%socket%*|*%$pf%*|*%dgram%*|*%udp%*|*%$proto%*|*%recv%*|*%ancillary%*|*%envvar%*|*%$ROOT%*|*%$NAME%*)
#set -vx
TEST="$NAME: $KEYW ancillary message sets env SOCAT_$SCM_ENVNAME"
# idea: start a socat process with *-RECVFROM:..,... , ev. with ancillary
# message  enabling option and send it a packet, ev. with some option. write
# the resulting environment to a file and check its contents for the
# appropriate variable.
if ! eval $NUMCOND; then :;
elif [ "$ROOT" = root -a $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ "$PF" = "IP6" ] && ( ! feat=$(testaddrs ip6) || ! runsip6 ) >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}IP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
case "X$IPPORT" in
    "XPORT")
    tra="$PORT"		# test recv address
    tsa="$ADDR:$PORT"	# test sendto address
    PORT=$((PORT+1)) ;;
    "XPROTO")
    tra="$PROTO"		# test recv address
    tsa="$ADDR:$PROTO"	# test sendto address
    PROTO=$((PROTO+1)) ;;
    *)
    tra="$(eval echo "$ADDR")"	# resolve $N
    tsa="$tra"
esac
#CMD0="$TRACE $SOCAT $opts -u $KEYW-RECVFROM:$tra,reuseaddr,$SCM_RECV SYSTEM:\"export -p\""
CMD0="$TRACE $SOCAT $opts -u -lpsocat $KEYW-RECVFROM:$tra,reuseaddr,$SCM_RECV SYSTEM:\"echo \\\$SOCAT_$SCM_ENVNAME\""
CMD1="$TRACE $SOCAT $opts -u - $KEYW-SENDTO:$tsa,$SCM_ENABLE"
printf "test $F_n $TEST... " $N
# is this option supported?
if $SOCAT -hhh |grep "[[:space:]]$SCM_RECV[[:space:]]" >/dev/null; then
eval "$CMD0 >\"$tf\" 2>\"${te}0\" &"
pid0="$!"
wait${proto}port $tra 1
echo "XYZ" |$CMD1 2>"${te}1"
rc1="$?"
waitfile "$tf" 2
#i=0; while [ ! -s "${te}0" -a "$i" -lt 10 ]; do  usleep 100000; i=$((i+1));  done
kill "$pid0" 2>/dev/null; wait
# do not show more messages than requested
if [ "$SCM_VALUE" = "timestamp" ]; then
    SCM_VALUE="$(date '+%a %b %e %H:%M:.. %Y'), ...... usecs"
fi
if [ "$rc1" -ne 0 ]; then
    $PRINTF "$NO_RESULT: $SOCAT:\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numCANT=$((numCANT+1))
#elif ! egrep "^export SOCAT_$SCM_ENVNAME=[\"']?$SCM_VALUE[\"']?\$" ${tf} >/dev/null; then
#elif ! eval echo "$TRACE $SOCAT_\$SCM_VALUE" |diff - "${tf}" >/dev/null; then
elif ! expr "$(cat "$tf")" : "$(eval echo "\$SCM_VALUE")\$" >/dev/null; then
    $PRINTF "$FAILED\n"
    echo "logged value \"$(cat "$tf")\" instead of \"$(eval echo "\$SCM_VALUE")\""
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then
	cat "${te}0"; echo; cat "${te}1";
    fi
    numOK=$((numOK+1))
fi
else # option is not supported
    $PRINTF "${YELLOW}$SCM_RECV not available${NORMAL}\n"
    numCANT=$((numCANT+1))
fi # option is not supported
fi ;; # NUMCOND, feats
esac
N=$((N+1))
#
done <<<"
IP4  UDP4 127.0.0.1 PORT  ip-options=x01000000 ip-recvopts       IP_OPTIONS     user x01000000
IP4  UDP4 127.0.0.1 PORT  ,                    so-timestamp      TIMESTAMP      user timestamp
IP4  UDP4 127.0.0.1 PORT  ip-ttl=53            ip-recvttl        IP_TTL         user 53
IP4  UDP4 127.0.0.1 PORT  ip-tos=7             ip-recvtos        IP_TOS         user 7
IP4  UDP4 127.0.0.1 PORT  ,                    ip-pktinfo        IP_LOCADDR     user 127.0.0.1
IP4  UDP4 127.0.0.1 PORT  ,                    ip-pktinfo        IP_DSTADDR     user 127.0.0.1
IP4  UDP4 127.0.0.1 PORT  ,                    ip-pktinfo        IP_IF          user lo
IP4  UDP4 127.0.0.1 PORT  ,                    ip-recvif         IP_IF          user lo0
IP4  UDP4 127.0.0.1 PORT  ,                    ip-recvdstaddr    IP_DSTADDR     user 127.0.0.1
IP4  IP4  127.0.0.1 PROTO ip-options=x01000000 ip-recvopts       IP_OPTIONS     root x01000000
IP4  IP4  127.0.0.1 PROTO ,                    so-timestamp      TIMESTAMP      root timestamp
IP4  IP4  127.0.0.1 PROTO ip-ttl=53            ip-recvttl        IP_TTL         root 53
IP4  IP4  127.0.0.1 PROTO ip-tos=7             ip-recvtos        IP_TOS         root 7
IP4  IP4  127.0.0.1 PROTO ,                    ip-pktinfo        IP_LOCADDR     root 127.0.0.1
IP4  IP4  127.0.0.1 PROTO ,                    ip-pktinfo        IP_DSTADDR     root 127.0.0.1
IP4  IP4  127.0.0.1 PROTO ,                    ip-pktinfo        IP_IF          root lo
IP4  IP4  127.0.0.1 PROTO ,                    ip-recvif         IP_IF          root lo0
IP4  IP4  127.0.0.1 PROTO ,                    ip-recvdstaddr    IP_DSTADDR     root 127.0.0.1
IP6  UDP6 [::1]     PORT  ,                    ipv6-recvpktinfo  IPV6_DSTADDR   user [[]0000:0000:0000:0000:0000:0000:0000:0001[]]
IP6  UDP6 [::1]     PORT  ipv6-unicast-hops=35 ipv6-recvhoplimit IPV6_HOPLIMIT  user 35
IP6  UDP6 [::1]     PORT  ipv6-tclass=0xaa     ipv6-recvtclass   IPV6_TCLASS    user x000000aa
IP6  IP6  [::1]     PROTO ,                    ipv6-recvpktinfo  IPV6_DSTADDR   root [[]0000:0000:0000:0000:0000:0000:0000:0001[]]
IP6  IP6  [::1]     PROTO ipv6-unicast-hops=35 ipv6-recvhoplimit IPV6_HOPLIMIT  root 35
IP6  IP6  [::1]     PROTO ipv6-tclass=0xaa     ipv6-recvtclass   IPV6_TCLASS    root x000000aa
#UNIX UNIX $td/test\$N.server - ,               so-timestamp      TIMESTAMP      user timestamp
"


# test the SOCKET-CONNECT address (against TCP4-LISTEN)
NAME=SOCKET_CONNECT_TCP4
case "$TESTS" in
*%$N%*|*%functions%*|*%generic%*|*%socket%*|*%$NAME%*)
TEST="$NAME: socket connect with TCP/IPv4"
# start a TCP4-LISTEN process that echoes data, and send test data using
# SOCKET-CONNECT, selecting TCP/IPv4. The sent data should be returned.
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts0p=$PORT; PORT=$((PORT+1))
ts0a="127.0.0.1"
ts1p=$(printf "%04x" $ts0p);
ts1a="7f000001" # "127.0.0.1"
ts1="x${ts1p}${ts1a}x0000000000000000"
ts1b=$(printf "%04x" $PORT); PORT=$((PORT+1))
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts TCP4-LISTEN:$ts0p,reuseaddr,bind=$ts0a PIPE"
CMD1="$TRACE $SOCAT $opts - SOCKET-CONNECT:2:6:$ts1,bind=x${ts1b}00000000x0000000000000000"
printf "test $F_n $TEST... " $N
$CMD0 2>"${te}0" &
pid0="$!"
waittcp4port $ts0p 1
echo "$da" |$CMD1 >>"$tf" 2>>"${te}1"
rc1="$?"
kill "$pid0" 2>/dev/null; wait;
if [ "$rc1" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))

PF_INET6="$($PROCAN -c |grep "^#define[[:space:]]*PF_INET6[[:space:]]" |cut -d' ' -f3)"

# test the SOCKET-CONNECT address (against TCP6-LISTEN)
NAME=SOCKET_CONNECT_TCP6
case "$TESTS" in
*%$N%*|*%functions%*|*%generic%*|*%tcp6%*|*%socket%*|*%$NAME%*)
TEST="$NAME: socket connect with TCP/IPv6"
if ! eval $NUMCOND; then :;
elif ! testaddrs tcp ip6 >/dev/null || ! runsip6 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
# start a TCP6-LISTEN process that echoes data, and send test data using
# SOCKET-CONNECT, selecting TCP/IPv6. The sent data should be returned.
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts0p=$PORT; PORT=$((PORT+1))
ts0a="[::1]"
ts1p=$(printf "%04x" $ts0p);
ts1a="00000000000000000000000000000001" # "[::1]"
ts1="x${ts1p}x00000000x${ts1a}x00000000"
ts1b=$(printf "%04x" $PORT); PORT=$((PORT+1))
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts TCP6-LISTEN:$ts0p,reuseaddr,bind=$ts0a PIPE"
CMD1="$TRACE $SOCAT $opts - SOCKET-CONNECT:$PF_INET6:6:$ts1,bind=x${ts1b}x00000000x00000000000000000000000000000000x00000000"
printf "test $F_n $TEST... " $N
$CMD0 2>"${te}0" &
pid0="$!"
waittcp6port $ts0p 1
echo "$da" |$CMD1 >>"$tf" 2>>"${te}1"
rc1="$?"
kill "$pid0" 2>/dev/null; wait;
if [ "$rc1" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

# test the SOCKET-CONNECT address (against UNIX-LISTEN)
NAME=SOCKET_CONNECT_UNIX
case "$TESTS" in
*%$N%*|*%functions%*|*%generic%*|*%unix%*|*%socket%*|*%$NAME%*)
TEST="$NAME: socket connect with UNIX domain"
# start a UNIX-LISTEN process that echoes data, and send test data using
# SOCKET-CONNECT, selecting UNIX socket. The sent data should be returned.
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts0="$td/test$N.server"
ts1="$td/test$N.client"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts UNIX-LISTEN:$ts0,reuseaddr PIPE"
CMD1="$TRACE $SOCAT $opts - SOCKET-CONNECT:1:0:\\\"$ts0\\\0\\\",bind=\\\"$ts1\\\0\\\""
printf "test $F_n $TEST... " $N
$CMD0 2>"${te}0" &
pid0="$!"
waitfile $ts0 1
echo "$da" |$CMD1 >>"$tf" 2>>"${te}1"
rc1="$?"
kill "$pid0" 2>/dev/null; wait;
if [ "$rc1" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
N=$((N+1))

# test the SOCKET-LISTEN address (with TCP4-CONNECT)
NAME=SOCKET_LISTEN
case "$TESTS" in
*%$N%*|*%functions%*|*%generic%*|*%socket%*|*%$NAME%*)
TEST="$NAME: socket recvfrom with TCP/IPv4"
# start a SOCKET-LISTEN process that uses TCP/IPv4 and echoes data, and
# send test data using TCP4-CONNECT. The sent data should be returned.
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PORT; PORT=$((PORT+1))
ts1a="127.0.0.1"
ts0p=$(printf "%04x" $ts1p);
ts0a="7f000001" # "127.0.0.1"
ts0="x${ts0p}${ts0a}x0000000000000000"
ts1b=$PORT; PORT=$((PORT+1))
ts1="$ts1a:$ts1p"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts SOCKET-LISTEN:2:6:$ts0,reuseaddr PIPE"
CMD1="$TRACE $SOCAT $opts - TCP4-CONNECT:$ts1,bind=:$ts1b"
printf "test $F_n $TEST... " $N
$CMD0 2>"${te}0" &
pid0="$!"
#sleep 1
waittcp4port $ts1p 1
echo "$da" |$CMD1 >>"$tf" 2>>"${te}1"
rc1="$?"
kill "$pid0" 2>/dev/null; wait;
if [ "$rc1" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

SOCK_DGRAM="$($PROCAN -c |grep "^#define[[:space:]]*SOCK_DGRAM[[:space:]]" |cut -d' ' -f3)"

# test the SOCKET-SENDTO address (against UDP4-RECVFROM)
NAME=SOCKET_SENDTO
case "$TESTS" in
*%$N%*|*%functions%*|*%generic%*|*%socket%*|*%ip4%*|*%udp%*|*%dgram%*|*%$NAME%*)
TEST="$NAME: socket sendto with UDP/IPv4"
# start a UDP4-RECVFROM process that echoes data, and send test data using
# SOCKET-SENDTO, selecting UDP/IPv4. The sent data should be returned.
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts0p=$PORT; PORT=$((PORT+1))
ts0a="127.0.0.1"
ts1p=$(printf "%04x" $ts0p);
ts1a="7f000001" # "127.0.0.1"
ts1="x${ts1p}${ts1a}x0000000000000000"
ts1b=$(printf "%04x" $PORT); PORT=$((PORT+1))
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts UDP4-RECVFROM:$ts0p,reuseaddr,bind=$ts0a PIPE"
CMD1="$TRACE $SOCAT $opts - SOCKET-SENDTO:2:$SOCK_DGRAM:17:$ts1,bind=x${ts1b}x00000000x0000000000000000"
printf "test $F_n $TEST... " $N
$CMD0 2>"${te}0" &
pid0="$!"
waitudp4port $ts0p 1
echo "$da" |$CMD1 >>"$tf" 2>>"${te}1"
rc1="$?"
kill "$pid0" 2>/dev/null; wait;
if [ "$rc1" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

# test the SOCKET-RECVFROM address (with UDP4-SENDTO)
NAME=SOCKET_RECVFROM
case "$TESTS" in
*%$N%*|*%functions%*|*%generic%*|*%socket%*|*%ip4%*|*%udp%*|*%dgram%*|*%$NAME%*)
TEST="$NAME: socket recvfrom with UDP/IPv4"
# start a SOCKET-RECVFROM process that uses UDP/IPv4 and echoes data, and
# send test data using UDP4-SENDTO. The sent data should be returned.
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PORT; PORT=$((PORT+1))
ts1a="127.0.0.1"
ts0p=$(printf "%04x" $ts1p);
ts0a="7f000001" # "127.0.0.1"
ts0="x${ts0p}${ts0a}x0000000000000000"
ts1b=$PORT; PORT=$((PORT+1))
ts1="$ts1a:$ts1p"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts SOCKET-RECVFROM:2:$SOCK_DGRAM:17:$ts0,reuseaddr PIPE"
CMD1="$TRACE $SOCAT $opts - UDP4-SENDTO:$ts1,bind=:$ts1b"
printf "test $F_n $TEST... " $N
$CMD0 2>"${te}0" &
pid0="$!"
sleep 1	# waitudp4port $ts1p 1
echo "$da" |$CMD1 >>"$tf" 2>>"${te}1"
rc1="$?"
kill "$pid0" 2>/dev/null; wait;
if [ "$rc1" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


# test the SOCKET-RECV address (with UDP4-SENDTO)
NAME=SOCKET_RECV
case "$TESTS" in
*%$N%*|*%functions%*|*%generic%*|*%socket%*|*%ip4%*|*%udp%*|*%dgram%*|*%$NAME%*)
TEST="$NAME: socket recv with UDP/IPv4"
# start a SOCKET-RECV process that uses UPD/IPv4 and writes received data to file, and
# send test data using UDP4-SENDTO.
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts1p=$PORT; PORT=$((PORT+1))
ts1a="127.0.0.1"
ts0p=$(printf "%04x" $ts1p);
ts0a="7f000001" # "127.0.0.1"
ts0="x${ts0p}${ts0a}x0000000000000000"
ts1b=$PORT; PORT=$((PORT+1))
ts1="$ts1a:$ts1p"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts -u SOCKET-RECV:2:$SOCK_DGRAM:17:$ts0,reuseaddr -"
CMD1="$TRACE $SOCAT $opts -u - UDP4-SENDTO:$ts1,bind=:$ts1b"
printf "test $F_n $TEST... " $N
$CMD0 2>"${te}0" >"$tf" &
pid0="$!"
sleep 1	# waitudp4port $ts1p 1
echo "$da" |$CMD1 2>>"${te}1"
rc1="$?"
sleep 1
kill "$pid0" 2>/dev/null; wait;
if [ "$rc1" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

# test SOCKET-DATAGRAM (with UDP4-DATAGRAM)
NAME=SOCKET_DATAGRAM
case "$TESTS" in
*%$N%*|*%functions%*|*%generic%*|*%socket%*|*%ip4%*|*%udp%*|*%dgram%*|*%$NAME%*)
TEST="$NAME: socket datagram via UDP/IPv4"
# start a UDP4-DATAGRAM process that echoes data, and send test data using
# SOCKET-DATAGRAM, selecting UDP/IPv4. The sent data should be returned.
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
ts0p=$PORT; PORT=$((PORT+1))
ts1p=$PORT; PORT=$((PORT+1))
ts0a="127.0.0.1"
ts1b=$(printf "%04x" $ts0p);
ts1a="7f000001" # "127.0.0.1"
ts0b=$(printf "%04x" $ts0p)
ts1b=$(printf "%04x" $ts1p)
ts1="x${ts0b}${ts1a}x0000000000000000"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts UDP4-DATAGRAM:$ts0a:$ts1p,bind=:$ts0p,reuseaddr PIPE"
CMD1="$TRACE $SOCAT $opts - SOCKET-DATAGRAM:2:$SOCK_DGRAM:17:$ts1,bind=x${ts1b}x00000000x0000000000000000"
printf "test $F_n $TEST... " $N
$CMD0 2>"${te}0" &
pid0="$!"
waitudp4port $ts0p 1
echo "$da" |$CMD1 2>>"${te}1" >"$tf"
rc1="$?"
kill "$pid0" 2>/dev/null; wait;
if [ "$rc1" -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   echo "$CMD0 &"
   cat "${te}0"
   echo "$CMD1"
   cat "${te}1"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat $te; fi
   numOK=$((numOK+1))
fi
fi ;; # NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=SOCKETRANGEMASK
case "$TESTS" in
*%$N%*|*%functions%*|*%security%*|*%generic%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%socket%*|*%range%*|*%$NAME%*)
TEST="$NAME: security of generic socket-listen with RANGE option"
if ! eval $NUMCOND; then :;
elif [ -z "$SECONDADDR" ]; then
    # we need access to more loopback addresses
    $PRINTF "test $F_n $TEST... ${YELLOW}need a second IPv4 address${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
ts1p=$(printf "%04x" $PORT);
testserversec "$N" "$TEST" "$opts -s" "SOCKET-LISTEN:2:6:x${ts1p}x00000000x0000000000000000,reuseaddr,fork,retry=1" "" "range=x0000x7f000000:x0000xffffffff" "SOCKET-CONNECT:2:6:x${ts1p}x${SECONDADDRHEX}x0000000000000000" 4 tcp $PORT 0
fi ;; # NUMCOND, $SECONDADDR
esac
PORT=$((PORT+1))
N=$((N+1))


TIOCEXCL="$($PROCAN -c |grep "^#define[[:space:]]*TIOCEXCL[[:space:]]" |cut -d' ' -f3)"

# test the generic ioctl-void option
NAME=IOCTL_VOID
case "$TESTS" in
*%$N%*|*%functions%*|*%pty%*|*%generic%*|*%$NAME%*)
TEST="$NAME: test the ioctl-void option"
# there are not many ioctls that apply to non global resources and do not
# require root. TIOCEXCL seems to fit:
# process 0 provides a pty;
# process 1 opens it with the TIOCEXCL ioctl; 
# process 2 opens it too and fails with "device or resource busy" only when the
# previous ioctl was successful
if ! eval $NUMCOND; then :;
elif [ -z "$TIOCEXCL" ]; then
    # we use the numeric value of TIOCEXL which is system dependent
    $PRINTF "test $F_n $TEST... ${YELLOW}no value of TIOCEXCL${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tp="$td/test$N.pty"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts PTY,LINK=$tp pipe"
CMD1="$TRACE $SOCAT $opts - file:$tp,ioctl-void=$TIOCEXCL,raw,echo=0"
CMD2="$TRACE $SOCAT $opts - file:$tp,raw,echo=0"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
waitfile $tp 1
(echo "$da"; sleep 2) |$CMD1 >"$tf" 2>"${te}1" &	# this should always work
pid1=$!
usleep 1000000
$CMD2 >/dev/null 2>"${te}2" </dev/null
rc2=$?
kill $pid0 $pid1 2>/dev/null; wait
if ! echo "$da" |diff - "$tf" >/dev/null; then
    $PRINTF "${YELLOW}phase 1 failed${NORMAL}\n"
    echo "$CMD0 &"
    echo "$CMD1"
    echo "$da" |diff - "$tf"
    numCANT=$((numCANT+1))
elif [ $rc2 -eq 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD0 &"
    echo "$CMD1"
    echo "$CMD2"
    cat "${te}0" "${te}1" "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}0" "${te}1" "${te}2"; fi
    numOK=$((numOK+1))
fi
fi # NUMCOND, TIOCEXCL
;;
esac
N=$((N+1))


SOL_SOCKET="$($PROCAN -c |grep "^#define[[:space:]]*SOL_SOCKET[[:space:]]" |cut -d' ' -f3)"
SO_REUSEADDR="$($PROCAN -c |grep "^#define[[:space:]]*SO_REUSEADDR[[:space:]]" |cut -d' ' -f3)"

# test the generic setsockopt-int option
if false; then
# this test no longer works due to fix for options on listening sockets
NAME=SETSOCKOPT_INT
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%tcp%*|*%generic%*|*%$NAME%*)
TEST="$NAME: test the setsockopt-int option"
# there are not many socket options that apply to non global resources, do not
# require root, do not require a network connection, and can easily be
# tested. SO_REUSEADDR seems to fit:
# process 0 provides a tcp listening socket with reuseaddr;
# process 1 connects to this port; thus the port is connected but no longer
# listening
# process 2 tries to listen on this port with SO_REUSEADDR, will fail if the
# (generically specified) SO_REUSEADDR socket options did not work
# process 3 connects to this port; only if it is successful the test is ok
if ! eval $NUMCOND; then :;
elif [ -z "$SO_REUSEADDR" ]; then
    # we use the numeric value of SO_REUSEADDR which might be system dependent
    $PRINTF "test $F_n $TEST... ${YELLOW}value of SO_REUSEADDR not known${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tp="$PORT"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts TCP4-L:$tp,setsockopt-int=$SOL_SOCKET:$SO_REUSEADDR:1 PIPE"
CMD1="$TRACE $SOCAT $opts - TCP:localhost:$tp"
CMD2="$CMD0"
CMD3="$CMD1"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
waittcp4port $tp 1
(echo "$da"; sleep 3) |$CMD1 >"$tf" 2>"${te}1" &	# this should always work
pid1=$!
usleep 1000000
$CMD2 >/dev/null 2>"${te}2" &
pid2=$!
waittcp4port $tp 1
(echo "$da") |$CMD3 >"${tf}3" 2>"${te}3"
rc3=$?
kill $pid0 $pid1 $pid2 2>/dev/null; wait
if ! echo "$da" |diff - "$tf"; then
    $PRINTF "${YELLOW}phase 1 failed${NORMAL}\n"
    echo "$CMD0 &"
    echo "$CMD1"
    numCANT=$((numCANT+1))
elif [ $rc3 -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD0 &"
    echo "$CMD1"
    echo "$CMD2 &"
    echo "$CMD3"
    cat "${te}2" "${te}3"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "${tf}3"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD0 &"
    echo "$CMD1"
    echo "$CMD2 &"
    echo "$CMD3"
    echo "$da" |diff - "${tf}3"
    numCANT=$((numCANT+1))
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}0" "${te}1" "${te}2" "${te}3"; fi
    numOK=$((numOK+1))
fi
fi # NUMCOND, SO_REUSEADDR
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))
#
fi


NAME=SCTP4STREAM
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%ipapp%*|*%sctp%*|*%$NAME%*)
TEST="$NAME: echo via connection to SCTP V4 socket"
PORT="$((PORT+1))"
if ! eval $NUMCOND; then :;
elif ! testaddrs sctp ip4 >/dev/null || ! runsip4 >/dev/null || ! runssctp4 "$((PORT-1))" >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}SCTP4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ "$UNAME" = Linux ] && ! grep ^sctp /proc/modules >/dev/null; then
    # RHEL5 based systems became unusable when an sctp socket was created but
    # module sctp not loaded
    $PRINTF "test $F_n $TEST...${YELLOW}load sctp module!${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="127.0.0.1:$tsl"
da=$(date)
CMD1="$TRACE $SOCAT $opts SCTP4-LISTEN:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout SCTP4:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid1=$!
waitsctp4port $tsl 1
# SCTP does not seem to support half close, so we give it 1s to finish
(echo "$da"; sleep 1) |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   cat "${te}1"
   echo "$CMD2"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid1 2>/dev/null
wait
fi # NUMCOND, feats
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))

NAME=SCTP6STREAM
case "$TESTS" in
*%$N%*|*%functions%*|*%ip6%*|*%ipapp%*|*%sctp%*|*%$NAME%*)
TEST="$NAME: echo via connection to SCTP V6 socket"
PORT="$((PORT+1))"
if ! eval $NUMCOND; then :;
elif ! testaddrs sctp ip6 >/dev/null || ! runsip6 >/dev/null || ! runssctp6 "$((PORT-1))" >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}SCTP6 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif [ "$UNAME" = Linux ] && ! grep ^sctp /proc/modules >/dev/null; then
    $PRINTF "test $F_n $TEST...${YELLOW}load sctp module!${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
tsl=$PORT
ts="[::1]:$tsl"
da=$(date)
CMD1="$TRACE $SOCAT $opts SCTP6-listen:$tsl,reuseaddr PIPE"
CMD2="$TRACE $SOCAT $opts stdin!!stdout SCTP6:$ts"
printf "test $F_n $TEST... " $N
$CMD1 >"$tf" 2>"${te}1" &
pid=$!	# background process id
waitsctp6port $tsl 1
# SCTP does not seem to support half close, so we let it 1s to finish
(echo "$da"; sleep 1) |$CMD2 >>"$tf" 2>>"${te}2"
if [ $? -ne 0 ]; then
   $PRINTF "$FAILED: $TRACE $SOCAT:\n"
   echo "$CMD1 &"
   cat "${te}1"
   echo "$CMD2"
   cat "${te}2"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
   $PRINTF "$FAILED: diff:\n"
   cat "$tdiff"
   numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
fi # NUMCOND, feats
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# socat up to 1.7.1.1 (and 2.0.0-b3) terminated with error when an openssl peer
# performed a renegotiation. Test if this is fixed.
NAME=OPENSSLRENEG1
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%openssl%*|*%socket%*|*%$NAME%*)
TEST="$NAME: OpenSSL connections survive renogotiation"
# connect with s_client to socat ssl-l; force a renog, then transfer data. When
# data is passed the test succeeded
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! type openssl >/dev/null 2>&1; then
    $PRINTF "test $F_n $TEST... ${YELLOW}openssl executable not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,reuseaddr,cert=testsrv.crt,key=testsrv.key,verify=0 PIPE"
CMD1="openssl s_client -port $PORT -verify 0"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
waittcp4port $PORT 1
(echo "R"; sleep 1; echo "$da"; sleep 1) |$CMD1 2>"${te}1" |fgrep "$da" >"${tf}1"
rc1=$?
kill $pid0 2>/dev/null; wait
if echo "$da" |diff - ${tf}1 >"$tdiff"; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
#    cat "${te}1"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# socat up to 1.7.1.1 (and 2.0.0-b3) terminated with error when an openssl peer
# performed a renegotiation. The first temporary fix to this problem might
# leave socat in a blocking ssl-read state. Test if this has been fixed.
NAME=OPENSSLRENEG2
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%openssl%*|*%socket%*|*%$NAME%*)
TEST="$NAME: OpenSSL connections do not block after renogotiation"
# connect with s_client to socat ssl-l; force a renog, then transfer data from
# socat to the peer. When data is passed this means that the former ssl read no
# longer blocks and the test succeeds
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! type openssl >/dev/null 2>&1; then
    $PRINTF "test $F_n $TEST... ${YELLOW}openssl executable not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
gentestcert testsrv
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,reuseaddr,cert=testsrv.crt,key=testsrv.key,verify=0 SYSTEM:\"sleep 1; echo \\\\\\\"\\\"$da\\\"\\\\\\\"; sleep 1\"!!STDIO"
CMD1="openssl s_client -port $PORT -verify 0"
printf "test $F_n $TEST... " $N
eval "$CMD0 >/dev/null 2>\"${te}0\" &"
pid0=$!
waittcp4port $PORT 1
(echo "R"; sleep 2) |$CMD1 2>"${te}1" |fgrep "$da" >"${tf}1"
rc1=$?
kill $pid0 2>/dev/null; wait
if echo "$da" |diff - ${tf}1 >"$tdiff"; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
#    cat "${te}1"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# socat up to 1.7.1.2 had a stack overflow vulnerability that occurred when
# command line arguments (whole addresses, host names, file names) were longer
# than 512 bytes.
NAME=HOSTNAMEOVFL
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%security%*|*%socket%*|*%$NAME%*)
TEST="$NAME: stack overflow on overly long host name"
# provide a long host name to TCP-CONNECT and check socats exit code
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
# prepare long data - perl might not be installed
rm -f "$td/test$N.dat"
i=0; while [ $i -lt 64 ]; do  echo -n "AAAAAAAAAAAAAAAA" >>"$td/test$N.dat"; i=$((i+1)); done
CMD0="$TRACE $SOCAT $opts TCP-CONNECT:$(cat "$td/test$N.dat"):$PORT STDIO"
printf "test $F_n $TEST... " $N
$CMD0 </dev/null 1>&0 2>"${te}0"
rc0=$?
if [ $rc0 -lt 128 ] || [ $rc0 -eq 255 ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))

# socat up to 1.7.1.2 had a stack overflow vulnerability that occurred when
# command line arguments (whole addresses, host names, file names) were longer
# than 512 bytes.
NAME=FILENAMEOVFL
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%security%*|*%openssl%*|*%$NAME%*)
TEST="$NAME: stack overflow on overly long file name"
# provide a 600 bytes long key file option to OPENSSL-CONNECT and check socats exit code
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
i=0; while [ $i -lt 64 ]; do  echo -n "AAAAAAAAAAAAAAAA" >>"$td/test$N.dat"; i=$((i+1)); done
CMD0="$TRACE $SOCAT $opts OPENSSL:localhost:$PORT,key=$(cat "$td/test$N.dat") STDIO"
printf "test $F_n $TEST... " $N
$CMD0 </dev/null 1>&0 2>"${te}0"
rc0=$?
if [ $rc0 -lt 128 ] || [ $rc0 -eq 255 ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))

# socat up to 1.7.3.0 had a stack overflow vulnerability that occurred when
# command line arguments (whole addresses, host names, file names) were longer
# than 512 bytes and specially crafted.
NAME=NESTEDOVFL
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%security%*|*%exec%*|*%$NAME%*)
TEST="$NAME: stack overflow on overly long nested arg"
# provide a long host name to TCP-CONNECT and check socats exit code
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
# prepare long data - perl might not be installed
rm -f "$td/test$N.dat"
i=0; while [ $i -lt 64 ]; do  echo -n "AAAAAAAAAAAAAAAA" >>"$td/test$N.dat"; i=$((i+1)); done
CMD0="$TRACE $SOCAT $opts EXEC:[$(cat "$td/test$N.dat")] STDIO"
printf "test $F_n $TEST... " $N
$CMD0 </dev/null 1>&0 2>"${te}0"
rc0=$?
if [ $rc0 -lt 128 ] || [ $rc0 -eq 255 ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# test for a bug in gopen that lead to crash or warning when opening a unix
# domain socket with GOPEN
NAME=GOPEN_UNIX_CRASH
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%gopen%*|*%unix%*|*%socket%*|*%$NAME%*)
TEST="$NAME: check crash when connecting to a unix domain socket using address GOPEN"
# a unix domain server is started in background. the check process connects to
# its socket. when this process crashes or issues a warning the bug is present.
# please note that a clean behaviour does not proof anything; behaviour of bug
# depends on the value of an uninitialized var
#set -vx
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
ts="$td/test$N.sock"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts UNIX-LISTEN:$ts PIPE"
CMD1="$TRACE $SOCAT $opts -d - GOPEN:$ts"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" </dev/null &
pid0=$!
waitunixport "$ts" 1
echo "$da" |$CMD1 >"${tf}1" 2>"${te}1"
rc1=$?
kill $pid0 2>/dev/null; wait
if [ $rc1 -ne 0 ]; then
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif grep -q ' W ' "${te}1"; then
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif  ! echo "$da" |diff - ${tf}1 >"$tdiff"; then
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi
fi # NUMCOND
 ;;
esac
N=$((N+1))


# test if socat keeps an existing file where it wanted to create a UNIX socket
NAME=UNIXLISTEN_KEEPFILE
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%unix%*|*%socket%*|*%$NAME%*)
TEST="$NAME: socat keeps an existing file where it wanted to create a UNIX socket"
# we create a file and start socat with UNIX-LISTEN on this file. expected
# behaviour: socat exits immediately with error, but keeps the file
# up to 1.7.1.3, it removed the file
if ! eval $NUMCOND; then :; else
tf="$td/test$N.file"
te="$td/test$N.stderr"
CMD0="$TRACE $SOCAT $opts -u UNIX-LISTEN:$tf /dev/null"
printf "test $F_n $TEST... " $N
rm -f "$tf"; touch "$tf"
$CMD0 >/dev/null 2>"${te}0"
rc0=$?
if [ $rc0 -ne 0 -a -f "$tf" ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
N=$((N+1))


# PTY address allowed to sepcify address parameters but ignored them
NAME=PTY_VOIDARG
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%pty%*|*%$NAME%*)
TEST="$NAME: check if address params of PTY produce error"
# invoke socat with address PTY and some param; expect an error
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts /dev/null PTY:/tmp/xyz"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0"
rc0=$?
if [ $rc0 -ne 0 ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
N=$((N+1))


# incomplete writes were reported but led to data loss
NAME=INCOMPLETE_WRITE
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%$NAME%*)
TEST="$NAME: check if incomplete writes are handled properly"
# write to a nonblocking fd a block that is too large for atomic write
# and check if all data arrives
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tp="$td/test$N.pipe"
tw="$td/test$N.wc-c"
# this is the size we write() in one call; data is never stored on disk, so
# make it large enough to exceed any atomic write size; but higher number might
# take much time
bytes=100000	# for Linux 2.6.? this must be >65536
CMD0="$TRACE $SOCAT $opts -u PIPE:$tp STDOUT"
CMD1="$TRACE $SOCAT $opts -u -b $bytes OPEN:/dev/zero,readbytes=$bytes FILE:$tp,o-nonblock"
printf "test $F_n $TEST... " $N
$CMD0 2>"${te}0" |wc -c >"$tw" &
pid=$!
waitfile "$tp"
$CMD1 2>"${te}1" >"${tf}1"
rc1=$?
wait
if [ $rc1 -ne 0 ]; then
    $PRINTF "$NO_RESULT\n"
    numCANT=$((numCANT+1))
elif [ ! -e "$tw" ]; then 
    $PRINTF "$NO_RESULT\n"
    numCANT=$((numCANT+1))
elif [ "$bytes" -eq $(cat "$tw") ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "transferred only $(cat $tw) of $bytes bytes" >&2
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
N=$((N+1))


NAME=OPENSSL_ANULL
case "$TESTS" in
*%$N%*|*%functions%*|*%openssl%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%$NAME%*)
TEST="$NAME: OpenSSL server with cipher aNULL "
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
elif ! testaddrs listen tcp ip4 >/dev/null || ! runsip4 >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}TCP/IPv4 not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD2="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,reuseaddr,$SOCAT_EGD,ciphers=aNULL,verify=0 pipe"
CMD="$TRACE $SOCAT $opts - openssl:$LOCALHOST:$PORT,ciphers=aNULL,verify=0,$SOCAT_EGD"
printf "test $F_n $TEST... " $N
eval "$CMD2 2>\"${te}1\" &"
pid=$!	# background process id
waittcp4port $PORT
echo "$da" |$CMD >$tf 2>"${te}2"
if ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD2 &"
    echo "$CMD"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
   $PRINTF "$OK\n"
   if [ -n "$debug" ]; then cat "${te}1" "${te}2"; fi
   numOK=$((numOK+1))
fi
kill $pid 2>/dev/null
wait
fi ;; # NUMCOND, feats
esac
PORT=$((PORT+1))
N=$((N+1))


while read KEYW FEAT ADDR IPPORT; do
if [ -z "$KEYW" ]|| [[ "$KEYW" == \#* ]]; then continue; fi
PROTO=$KEYW
proto="$(echo "$PROTO" |tr A-Z a-z)"
# test the max-children option on really connection oriented sockets
NAME=${KEYW}MAXCHILDREN
case "$TESTS" in
*%$N%*|*%functions%*|*%maxchildren%*|*%socket%*|*%$NAME%*)
TEST="$NAME: max-children option"
# start a listen process with max-children=1; connect with a client, let it
# sleep some time before sending data; connect with second client that sends
# data immediately. If max-children is working correctly the first data should
# arrive first because the second process has to wait.
if ! eval $NUMCOND; then :; else
case "X$IPPORT" in
    "XPORT")
    tsl=$PORT 		# test socket listen address
    tsc="$ADDR:$PORT"	# test socket connect address
    PORT=$((PORT+1)) ;;
    *)
    tsl="$(eval echo "$ADDR")"	# resolve $N
    tsc=$tsl
esac
#ts="$td/test$N.sock"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts -U FILE:$tf,o-trunc,o-creat,o-append $PROTO-LISTEN:$tsl,fork,max-children=1"
CMD1="$TRACE $SOCAT $opts -u - $PROTO-CONNECT:$tsc,shut-null"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
wait${proto}port $tsl 1
(echo "$da 1"; sleep 2) |$CMD1 >"${tf}1" 2>"${te}1" &
pid1=$!
sleep 1
echo "$da 2" |$CMD1 >"${tf}2" 2>"${te}2" &
pid2=$!
sleep 2
kill $pid1 $pid2 $pid0 2>/dev/null; wait
if echo -e "$da 1\n$da 2" |diff - $tf >$tdiff; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "(echo \"$da 1\"; sleep 2) |$CMD1"
    echo "echo \"$da 2\" |$CMD1"
    cat "${te}0"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
N=$((N+1))
done <<<"
TCP4  TCP  127.0.0.1 PORT
TCP6  TCP  [::1]     PORT
SCTP4 TCP  127.0.0.1 PORT
SCTP6 TCP  [::1]     PORT
UNIX  UNIX $td/test\$N.server -
"
# debugging this hanging test was difficult - following lessons learned:
# kill <parent> had no effect when child process existed
# strace -f (on Fedora-23) sometimes writes/pads? blocks with \0, overwriting client traces
# using the TRACE feature lets above kill command kill strace, not socat
# care for timing, understand what you want :-)


while read KEYW FEAT ADDR IPPORT SHUT; do
if [ -z "$KEYW" ]|| [[ "$KEYW" == \#* ]]; then continue; fi
PROTO=$KEYW
proto="$(echo "$PROTO" |tr A-Z a-z)"
# test the max-children option on pseudo connected sockets
NAME=${KEYW}MAXCHILDREN
case "$TESTS" in
*%$N%*|*%functions%*|*%maxchildren%*|*%socket%*|*%$NAME%*)
TEST="$NAME: max-children option"
# start a listen process with max-children=1; connect with a client, let it
# send data and then sleep; connect with second client that wants to send
# data immediately, but keep first client active until server terminates.
#If max-children is working correctly only the first data should
# arrive.
if ! eval $NUMCOND; then :; else
case "X$IPPORT" in
    "XPORT")
    tsl=$PORT 		# test socket listen address
    tsc="$ADDR:$PORT"	# test socket connect address
    PORT=$((PORT+1)) ;;
    *)
    tsl="$(eval echo "$ADDR")"	# resolve $N
    tsc=$tsl
esac
#ts="$td/test$N.sock"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts -U FILE:$tf,o-trunc,o-creat,o-append $PROTO-LISTEN:$tsl,fork,max-children=1"
CMD1="$TRACE $SOCAT $opts -u - $PROTO-CONNECT:$tsc,$SHUT"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
wait${proto}port $tsl 1
(echo "$da 1"; sleep 3) |$CMD1 >"${tf}1" 2>"${te}1" &
pid1=$!
sleep 1
echo "$da 2" |$CMD1 >"${tf}2" 2>"${te}2" &
pid2=$!
sleep 1
cpids="$(childpids $pid0)"
kill $pid1 $pid2 $pid0 $cpids 2>/dev/null; wait
if echo -e "$da 1" |diff - $tf >$tdiff; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "(echo \"$da 1\"; sleep 2) |$CMD1"
    echo "echo \"$da 2\" |$CMD1"
    cat "${te}0"
    cat "${te}1"
    cat "${te}2"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
N=$((N+1))
done <<<"
UDP4  UDP  127.0.0.1 PORT shut-null
UDP6  UDP  [::1]     PORT shut-null
"
# debugging this hanging test was difficult - following lessons learned:
# kill <parent> had no effect when child process existed
# strace -f (on Fedora-23) sometimes writes/pads? blocks with \0, overwriting client traces
# using the TRACE feature lets above kill command kill strace, not socat
# care for timing, understand what you want :-)


# socat up to 1.7.2.0 had a bug in xioscan_readline() that could be exploited
# to overflow a heap based buffer (socat security advisory 3)
# problem reported by Johan Thillemann
NAME=READLINE_OVFL
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%security%*|*%readline%*|*%$NAME%*)
TEST="$NAME: test for buffer overflow in readline prompt handling"
# address 1 is the readline where write data was handled erroneous
# address 2 provides data to trigger the buffer overflow
# when no SIGSEGV or so occurs the test succeeded (bug fixed)
if ! eval $NUMCOND; then :;
elif ! feat=$(testaddrs readline pty); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$(echo "$feat"| tr 'a-z' 'A-Z') not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
ti="$td/test$N.data"
CMD0="$SOCAT $opts READLINE $ti"
printf "test $F_n $TEST... " $N
# prepare long data - perl might not be installed
#perl -e 'print "\r","Z"x513' >"$ti"
echo $E -n "\rA" >"$ti"
i=0; while [ $i -lt 32 ]; do echo -n "AAAAAAAAAAAAAAAA" >>"$ti"; let i=i+1; done
$TRACE $SOCAT - SYSTEM:"$CMD0; echo rc=\$? >&2",pty >/dev/null 2>"${te}0"
rc=$?
rc0="$(grep ^rc= "${te}0" |sed 's/.*=//')"
if [ $rc -ne 0 ]; then
    $PRINTF "${YELLOW}framework failed${NORMAL}\n"
elif [ $rc0 -eq 0 ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0"
    grep -v ^rc= "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# socat up to 1.7.2.1 did only shutdown() but not close() an accept() socket 
# that was rejected due to range, tcpwrap, lowport, or sourceport option.
# This file descriptor leak could be used for a denial of service attack.
NAME=FDLEAK
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%security%*|*%socket%*|*%$NAME%*)
TEST="$NAME: file descriptor leak with range option"
# have a TCP-LISTEN with range option; connect with wrong source address until
# "open files" limit would exceed. When server continues operation the bug is
# not present.
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
RLIMIT_NOFILE="$(ulimit -n)"
if ! [[ "$RLIMIT_NOFILE" =~ ^[0-9][0-9]*$ ]]; then
    $PRINTF "${YELLOW}cannot determine ulimit -n"
else
CMD0="$TRACE $SOCAT $opts TCP-LISTEN:$PORT,reuseaddr,range=$LOCALHOST:255.255.255.255 PIPE"
CMD1="$TRACE $SOCAT $opts -t 0 /dev/null TCP:$SECONDADDR:$PORT,bind=$SECONDADDR"
CMD2="$TRACE $SOCAT $opts - TCP:$LOCALHOST:$PORT,bind=$LOCALHOST"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
waittcp4port $PORT 1
while [ $RLIMIT_NOFILE -gt 0 ]; do
    $CMD1 >/dev/null 2>>"${te}1"
    let RLIMIT_NOFILE=RLIMIT_NOFILE-1
done
echo "$da" |$CMD2 >"${tf}2" 2>"${te}2"
rc2=$?
kill $pid0 2>/dev/null; wait
echo -e "$da" |diff "${tf}2" - >$tdiff
if [ $rc2 -ne 0 ]; then
    $PRINTF "$FAILED\n"
    echo "$CMD2 &"
    cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif [ -f "$tdiff" -a ! -s "$tdiff" ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    echo "$CMD2"
    cat "${te}0"
    cat "${te}1"
    cat "${te}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # ulimit -n
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


if false; then	# this overflow is not reliably reproducable
# socat up to 2.0.0-b6 did not check the length of the PROXY-CONNECT command line paramters when copying them into the HTTP request buffer. This could lead to a buffer overflow.
NAME=PROXY_ADDR_OVFL
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%security%*|*%socket%*|*%$NAME%*)
TEST="$NAME: proxy address parameters overflow"
# invoke socat PROXY-CONNECT with long proxy server and target server names. If it terminates with exit code >= 128 it is vulnerable
# However, even if vulnerable it often does not crash. Therefore we try to use a boundary check program like ElectricFence; only with its help we can tell that clean run proofs absence of vulnerability
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
EF=; for p in ef; do
    if type ef >/dev/null 2>&1; then
	EF="ef "; break
    fi
done
CMD0="$TRACE $SOCAT $opts TCP-LISTEN:$PORT,reuseaddr FILE:/dev/null"
#CMD1="$EF $TRACE $SOCAT $opts FILE:/dev/null PROXY-CONNECT:$(perl -e "print 'A' x 256"):$(perl -e "print 'A' x 256"):80"
CMD1="$EF $TRACE $SOCAT $opts FILE:/dev/null PROXY-CONNECT:localhost:$(perl -e "print 'A' x 384"):80,proxyport=$PORT"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
waittcp4port $PORT 1
$CMD1 >/dev/null 2>"${te}1"
rc1=$?
if [ $rc1 -lt 128 ]; then
    if [ "$EF" ]; then
	$PRINTF "$OK\n"
	numOK=$((numOK+1))
    else
	$PRINTF "$UNKNOWN $RED(install ElectricFEnce!)$NORMAL\n"
	numCANT=$((num+1))
    fi
else
    $PRINTF "$FAILED\n"
    echo "$CMD1"
    cat "${te}"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))
fi	# false


# LISTEN addresses in socat up to 1.7.2.1 applied many file descriptor, socket,
# and TCP options only to the listening socket instead of the connection socket.
NAME=LISTEN_KEEPALIVE
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%listen%*|*%keepalive%*|*%socket%*|*%$NAME%*)
TEST="$NAME: keepalive option is applied to connection socket"
# instance 0 has TCP-LISTEN with option so-keepalive and invokes filan after 
# accept(). filan writes its output to the socket. instance 1 connects to 
# instance 0. The value of the sockets so-keepalive option is checked, it must
# be 1
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
#tdiff="$td/test$N.diff"
#da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts TCP4-LISTEN:$PORT,reuseaddr,so-keepalive EXEC:\"$FILAN -i 1\",nofork"
CMD1="$TRACE $SOCAT $opts - TCP4:$LOCALHOST:$PORT"
printf "test $F_n $TEST... " $N
eval $CMD0 >/dev/null 2>"${te}0" &
pid0=$!
waittcp4port $PORT 1
$CMD1 >"${tf}1" 2>"${te}1"
KEEPALIVE="$(cat "${tf}1" |tail -n +2 |sed -e "s/.*KEEPALIVE=//" -e "s/[[:space:]].*//")"
rc1=$?
kill $pid0 2>/dev/null; wait
if [ -z "$KEEPALIVE" ]; then
    $PRINTF "$NO_RESULT\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numWARN=$((numWARN+1))
elif [ "$KEEPALIVE" = "1" ]; then
    $PRINTF "$OK\n";
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# OPENSSL-CONNECT with bind option failed on some systems (eg.FreeBSD, but not
# Linux) with "Invalid argument".
NAME=OPENSSL_CONNECT_BIND
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%socket%*|*%openssl%*|*%$NAME%*)
TEST="$NAME: test OPENSSL-CONNECT with bind option"
# have a simple SSL server that just echoes data.
# connect with socat using OPENSSL-CONNECT with bind, send data and check if the
# reply is identical.
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf0="$td/test$N.0.stdout"
te0="$td/test$N.0.stderr"
tf1="$td/test$N.1.stdout"
te1="$td/test$N.1.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,reuseaddr,ciphers=aNULL,verify=0, PIPE"
CMD1="$TRACE $SOCAT $opts - OPENSSL-CONNECT:$LOCALHOST:$PORT,bind=$LOCALHOST,ciphers=aNULL,verify=0"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"$te0" &
pid0=$!
waittcp4port $PORT 1
echo "$da" |$CMD1 >"$tf1" 2>"$te1"
rc1=$?
kill $pid0 2>/dev/null; wait
if [ "$rc1" -ne 0 ]; then
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "$te0"
    cat "$te1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - $tf1 >"$tdiff"; then
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# socat up to version 1.7.2.3
# had a bug that converted a bit mask of 0 internally to 0xffffffff
NAME=TCP4RANGE_0BITS
case "$TESTS" in
*%$N%*|*%functions%*|*%tcp%*|*%tcp4%*|*%ip4%*|*%range%*|*%$NAME%*)
TEST="$NAME: correct evaluation of range mask 0"
if ! eval $NUMCOND; then :;
elif [ -z "$SECONDADDR" ]; then
    # we need access to a second addresses
    $PRINTF "test $F_n $TEST... ${YELLOW}need a second IPv4 address${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
#testserversec "$N" "$TEST" "$opts -s" "tcp4-l:$PORT,reuseaddr,fork,retry=1" "" "range=$SECONDADDR/32" "tcp4:127.0.0.1:$PORT" 4 tcp $PORT 0
CMD0="$TRACE $SOCAT $opts -u TCP4-LISTEN:$PORT,reuseaddr,range=127.0.0.1/0 CREATE:$tf"
CMD1="$TRACE $SOCAT $opts -u - TCP4-CONNECT:$SECONDADDR:$PORT,bind=$SECONDADDR"
printf "test $F_n $TEST... " $N
$CMD0 2>"${te}0" &
pid0=$!
waittcp4port $PORT 1
echo "$da" |$CMD1 >"${tf}1" 2>"${te}1"
rc1=$?
kill $pid0 2>/dev/null; wait
if [ $rc1 != 0 ]; then
    $PRINTF "${YELLOW}invocation failed${NORMAL}\n"
    numCANT=$((numCANT+1))
elif ! [ -f "$tf" ]; then
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "$tf" >"$tdiff"; then
    $PRINTF "${YELLOW}diff failed${NORMAL}\n"
    numCANT=$((numCANT+1))
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi

fi ;; # $SECONDADDR, NUMCOND
esac
PORT=$((PORT+1))
N=$((N+1))


# test: OPENSSL sets of environment variables with important values of peer certificate
while read ssldist MODE MODULE FIELD TESTADDRESS PEERADDRESS VALUE; do
if [ -z "$ssldist" ] || [[ "$ssldist" == \#* ]]; then continue; fi
#
SSLDIST=${ssldist^^*}
NAME="ENV_${SSLDIST}_${MODE}_${MODULE}_${FIELD}"
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%ipapp%*|*%tcp%*|*%$ssldist%*|*%envvar%*|*%$NAME%*)
TEST="$NAME: $SSLDIST sets env SOCAT_${SSLDIST}_${MODULE}_${FIELD}"
# have a server accepting a connection and invoking some shell code. The shell
# code extracts and prints the SOCAT related environment vars.
# outside code then checks if the environment contains the variables correctly
# describing the desired field.
FEAT=$(echo "$ssldist" |tr a-z A-Z)
if ! eval $NUMCOND; then :;
elif ! testaddrs $FEAT >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}$FEAT not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
gentestcert testsrv
gentestcert testcli
test_proto=tcp4
case "$MODE" in
    SERVER)
	CMD0="$SOCAT $opts -u $TESTADDRESS system:\"echo SOCAT_${SSLDIST}_${MODULE}_${FIELD}=\\\$SOCAT_${SSLDIST}_${MODULE}_${FIELD}; sleep 1\""
	CMD1="$SOCAT $opts -u /dev/null $PEERADDRESS"
	printf "test $F_n $TEST... " $N
	eval "$CMD0 2>\"${te}0\" >\"$tf\" &"
	pid0=$!
	wait${test_proto}port $PORT 1
	$CMD1 2>"${te}1"
	rc1=$?
	waitfile "$tf" 2
	kill $pid0 2>/dev/null; wait
	;;
    CLIENT)
	CMD0="$SOCAT $opts -u /dev/null $PEERADDRESS"
	CMD1="$SOCAT $opts -u $TESTADDRESS system:\"echo SOCAT_${SSLDIST}_${MODULE}_${FIELD}=\\\$SOCAT_${SSLDIST}_${MODULE}_${FIELD}; sleep 1\""
	printf "test $F_n $TEST... " $N
	$CMD0 2>"${te}0" &
	pid0=$!
	wait${test_proto}port $PORT 1
	eval "$CMD1 2>\"${te}1\" >\"$tf\""
	rc1=$?
	waitfile "$tf" 2
	kill $pid0 2>/dev/null; wait
	;;
esac
if [ $rc1 != 0 ]; then
    $PRINTF "$NO_RESULT (client failed):\n"
    echo "$CMD0 &"
    cat "${te}0"
    echo "$CMD1"
    cat "${te}1"
    numCANT=$((numCANT+1))
elif effval="$(grep SOCAT_${SSLDIST}_${MODULE}_${FIELD} "${tf}" |sed -e 's/^[^=]*=//' |sed -e "s/[\"']//g")";
    [ "$effval" = "$VALUE" ]; then
    $PRINTF "$OK\n"
    if [ "$debug" ]; then
	echo "$CMD0 &"
	cat "${te}0"
	echo "$CMD1"
	cat "${te}1"
    fi
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "expected \"$VALUE\", got \"$effval\"" >&2
    echo "$CMD0 &"
    cat "${te}0"
    echo "$CMD1"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND, feats
 ;;
esac
N=$((N+1))
#
done <<<"
openssl SERVER X509 ISSUER  OPENSSL-LISTEN:$PORT,so-reuseaddr,bind=$LOCALHOST,cert=testsrv.pem,cafile=testcli.crt,verify=1 OPENSSL-CONNECT:$LOCALHOST:$PORT,cert=testcli.pem,cafile=testsrv.crt,verify=1 $TESTCERT_ISSUER
openssl SERVER X509 SUBJECT OPENSSL-LISTEN:$PORT,so-reuseaddr,bind=$LOCALHOST,cert=testsrv.pem,cafile=testcli.crt,verify=1 OPENSSL-CONNECT:$LOCALHOST:$PORT,cert=testcli.pem,cafile=testsrv.crt,verify=1 $TESTCERT_SUBJECT
openssl SERVER X509 COMMONNAME             OPENSSL-LISTEN:$PORT,so-reuseaddr,bind=$LOCALHOST,cert=testsrv.pem,cafile=testcli.crt,verify=1 OPENSSL-CONNECT:$LOCALHOST:$PORT,cert=testcli.pem,cafile=testsrv.crt,verify=1 $TESTCERT_COMMONNAME
openssl SERVER X509 COUNTRYNAME            OPENSSL-LISTEN:$PORT,so-reuseaddr,bind=$LOCALHOST,cert=testsrv.pem,cafile=testcli.crt,verify=1 OPENSSL-CONNECT:$LOCALHOST:$PORT,cert=testcli.pem,cafile=testsrv.crt,verify=1 $TESTCERT_COUNTRYNAME
openssl SERVER X509 LOCALITYNAME           OPENSSL-LISTEN:$PORT,so-reuseaddr,bind=$LOCALHOST,cert=testsrv.pem,cafile=testcli.crt,verify=1 OPENSSL-CONNECT:$LOCALHOST:$PORT,cert=testcli.pem,cafile=testsrv.crt,verify=1 $TESTCERT_LOCALITYNAME
openssl SERVER X509 ORGANIZATIONALUNITNAME OPENSSL-LISTEN:$PORT,so-reuseaddr,bind=$LOCALHOST,cert=testsrv.pem,cafile=testcli.crt,verify=1 OPENSSL-CONNECT:$LOCALHOST:$PORT,cert=testcli.pem,cafile=testsrv.crt,verify=1 $TESTCERT_ORGANIZATIONALUNITNAME
openssl SERVER X509 ORGANIZATIONNAME       OPENSSL-LISTEN:$PORT,so-reuseaddr,bind=$LOCALHOST,cert=testsrv.pem,cafile=testcli.crt,verify=1 OPENSSL-CONNECT:$LOCALHOST:$PORT,cert=testcli.pem,cafile=testsrv.crt,verify=1 $TESTCERT_ORGANIZATIONNAME
openssl CLIENT X509 SUBJECT OPENSSL-CONNECT:$LOCALHOST:$PORT,cert=testcli.pem,cafile=testsrv.crt,verify=1 OPENSSL-LISTEN:$PORT,so-reuseaddr,bind=$LOCALHOST,cert=testsrv.pem,cafile=testcli.crt,verify=1 $TESTCERT_SUBJECT
openssl CLIENT X509 ISSUER  OPENSSL-CONNECT:$LOCALHOST:$PORT,cert=testcli.pem,cafile=testsrv.crt,verify=1 OPENSSL-LISTEN:$PORT,so-reuseaddr,bind=$LOCALHOST,cert=testsrv.pem,cafile=testcli.crt,verify=1 $TESTCERT_ISSUER
"


###############################################################################
# tests: option umask with "passive" NAMED group addresses
while read addr fileopt addropts proto diropt ADDR2; do
if [ -z "$addr" ] || [[ "$addr" == \#* ]]; then continue; fi
# some passive (listening...) filesystem based addresses did not implement the
# umask option
ADDR=${addr^^*}
ADDR_=${ADDR/-/_}
PROTO=${proto^^*}
if [ "$diropt" = "." ]; then diropt=; fi
if [ "$fileopt" = "." ]; then fileopt=; fi
if [ "$addropts" = "." ]; then addropts=; fi
NAME=${ADDR_}_UMASK
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%proto%*|*%socket%*|*%$proto%*|*%$NAME%*)
TEST="$NAME: $ADDR applies option umask"
# start a socat process with passive/listening file system entry. Check the
# permissions of the FS entry, then terminate the process.
# Test succeeds when FS entry exists and has expected permissions.
if ! eval $NUMCOND; then :; else
                            if [ $ADDR = PTY ]; then  set -xv; fi
tlog="$td/test$N.log"
te0="$td/test$N.0.stderr"
tsock="$td/test$N.sock"
if [ -z "$fileopt" ]; then
    CMD0="$TRACE $SOCAT $opts $diropt $ADDR:$tsock,$addropts,unlink-close=0,umask=177 $ADDR2"
else
    CMD0="$TRACE $SOCAT $opts $diropt $ADDR,$fileopt=$tsock,$addropts,unlink-close=0,umask=177 $ADDR2"
fi
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"$te0" &
pid0=$!
wait${proto} $tsock 1 2>"$tlog"
ERRNOENT=; if ! [ -e "$tsock" ]; then  ERRNOENT=1;  fi
perms=$(fileperms "$tsock")
kill $pid0 2>>"$tlog"
wait
if [ "$ERRNOENT" ]; then
    $PRINTF "${RED}no entry${NORMAL}\n"
    echo "$CMD0 &"
    cat "$te0"
    cat "$tlog"
    let numFAIL=numFAIL+1
    listFAIL="$listFAIL $N"
elif [ "$perms" != "600" ]; then
    $PRINTF "${RED}perms \"$perms\", expected \"600\" ${NORMAL}\n"
    echo "$CMD0 &"
    cat "$te0"
    let numFAIL=numFAIL+1
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    let numOK=numOK+1
fi
                               set +xv
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))
#
done <<<"
# address     fileopt addropts waitfor direction ADDR2
create        .       .        file     -U       FILE:/dev/null
open          .       creat    file     .        FILE:/dev/null
gopen         .       creat    file     .        FILE:/dev/null
unix-listen   .       .        unixport .        FILE:/dev/null
unix-recvfrom .       .        unixport .        FILE:/dev/null
unix-recv     .       .        unixport -u       FILE:/dev/null
pipe          .       .        file     -u       FILE:/dev/null
# pty does not seem to honor umask:
#pty           link    .        file     .        PIPE
"


# tests: option perm with "passive" NAMED group addresses
while read addr fileopt addropts proto diropt; do
if [ -z "$addr" ] || [[ "$addr" == \#* ]]; then continue; fi
# test if passive (listening...) filesystem based addresses implement option perm
ADDR=${addr^^*}
ADDR_=${ADDR/-/_}
PROTO=${proto^^*}
if [ "$diropt" = "." ]; then diropt=; fi
if [ "$fileopt" = "." ]; then fileopt=; fi
if [ "$addropts" = "." ]; then addropts=; fi
NAME=${ADDR_}_PERM
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%proto%*|*%socket%*|*%$proto%*|*%ignoreeof%*|*%$NAME%*)
TEST="$NAME: $ADDR applies option perm"
# start a socat process with passive/listening file system entry. Check the
# permissions of the FS entry, then terminate the process.
# Test succeeds when FS entry exists and has expected permissions.
if ! eval $NUMCOND; then :; else
tlog="$td/test$N.log"
te0="$td/test$N.0.stderr"
tsock="$td/test$N.sock"
#                                      set -vx
if [ -z "$fileopt" ]; then
    CMD0="$TRACE $SOCAT $opts $diropt $ADDR:$tsock,$addropts,perm=511 FILE:/dev/null,ignoreeof"
else
    CMD0="$TRACE $SOCAT $opts $diropt $ADDR,$fileopt=$tsock,$addropts,perm=511 FILE:/dev/null,ignoreeof"
fi
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"$te0" &
pid0=$!
wait${proto} $tsock 1 2>"$tlog"
ERRNOENT=; if ! [ -e "$tsock" ]; then  ERRNOENT=1;  fi
perms=$(fileperms "$tsock")
kill $pid0 2>>"$tlog"
wait
if [ "$ERRNOENT" ]; then
    $PRINTF "${RED}no entry${NORMAL}\n"
    echo "$CMD0 &"
    cat "$te0"
    cat "$tlog"
    let numFAIL=numFAIL+1
    listFAIL="$listFAIL $N"
elif [ "$perms" != "511" ]; then
    $PRINTF "${RED}perms \"$perms\", expected \"511\" ${NORMAL}\n"
    echo "$CMD0 &"
    cat "$te0"
    let numFAIL=numFAIL+1
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    let numOK=numOK+1
fi
                                      set +vx
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))
#
done <<<"
# address     fileopt addropts waitfor direction
create        .       .        file     -U
open          .       creat    file     .
gopen         .       creat    file     .
unix-listen   .       .        unixport .
unix-recvfrom .       .        unixport .
unix-recv     .       .        unixport -u
pipe          .       .        file     -u
pty           link    .        file     .
"


# tests: option user with "passive" NAMED group addresses
while read addr fileopt addropts proto diropt; do
if [ -z "$addr" ] || [[ "$addr" == \#* ]]; then continue; fi
# test if passive (listening...) filesystem based addresses implement option user
ADDR=${addr^^*}
ADDR_=${ADDR/-/_}
PROTO=${proto^^*}
if [ "$diropt" = "." ]; then diropt=; fi
if [ "$fileopt" = "." ]; then fileopt=; fi
if [ "$addropts" = "." ]; then addropts=; fi
NAME=${ADDR_}_USER
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%proto%*|*%socket%*|*%$proto%*|*%root%*|*%ignoreeof%*|*%$NAME%*)
TEST="$NAME: $ADDR applies option user"
# start a socat process with passive/listening file system entry with user option.
# Check the owner of the FS entry, then terminate the process.
# Test succeeds when FS entry exists and has expected owner.
if ! eval $NUMCOND; then :;
elif [ $(id -u) -ne 0 -a "$withroot" -eq 0 ]; then
    $PRINTF "test $F_n $TEST... ${YELLOW}must be root${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tlog="$td/test$N.log"
te0="$td/test$N.0.stderr"
tsock="$td/test$N.sock"
#                                      set -vx
if [ -z "$fileopt" ]; then
    CMD0="$TRACE $SOCAT $opts $diropt $ADDR:$tsock,$addropts,user=$SUBSTUSER FILE:/dev/null,ignoreeof"
else
    CMD0="$TRACE $SOCAT $opts $diropt $ADDR,$fileopt=$tsock,$addropts,user=$SUBSTUSER FILE:/dev/null,ignoreeof"
fi
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"$te0" &
pid0=$!
wait${proto} $tsock 1 2>"$tlog"
ERRNOENT=; if ! [ -e "$tsock" ]; then  ERRNOENT=1;  fi
user=$(fileuser "$tsock")
kill $pid0 2>>"$tlog"
wait
if [ "$ERRNOENT" ]; then
    $PRINTF "${RED}no entry${NORMAL}\n"
    echo "$CMD0 &"
    cat "$te0"
    cat "$tlog"
    let numFAIL=numFAIL+1
    listFAIL="$listFAIL $N"
elif [ "$user" != "$SUBSTUSER" ]; then
    $PRINTF "${RED}user \"$user\", expected \"$SUBSTUSER\" ${NORMAL}\n"
    echo "$CMD0 &"
    cat "$te0"
    let numFAIL=numFAIL+1
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    let numOK=numOK+1
fi
                                      set +vx
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))
#
done <<<"
# address     fileopt addropts waitfor direction
create        .       .        file     -U
open          .       creat    file     .
gopen         .       creat    file     .
unix-listen   .       .        unixport .
unix-recvfrom .       .        unixport .
unix-recv     .       .        unixport -u
pipe          .       .        file     -u
pty           link    .        file     .
"


# tests: is "passive" filesystem entry removed at the end? (without fork)
while read addr fileopt addropts proto diropt crit ADDR2; do
if [ -z "$addr" ] || [[ "$addr" == \#* ]]; then continue; fi
# some passive (listening...) filesystem based addresses did not remove the file
# system entry at the end
ADDR=${addr^^*}
ADDR_=${ADDR/-/_}
PROTO=${proto^^*}
if [ "$diropt" = "." ]; then diropt=; fi
if [ "$fileopt" = "." ]; then fileopt=; fi
if [ "$addropts" = "." ]; then addropts=; fi
# $ADDR removes the file system entry when the process is terminated
NAME=${ADDR_}_REMOVE
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%unix%*|*%socket%*|*%$NAME%*)
TEST="$NAME: $ADDR removes socket entry when terminated during accept"
# start a socat process with listening unix domain socket etc. Terminate the
# process and check if the file system socket entry still exists.
# Test succeeds when entry does not exist.
if ! eval $NUMCOND; then :; else
tlog="$td/test$N.log"
te0="$td/test$N.0.stderr"
tsock="$td/test$N.sock"
if [ -z "$fileopt" ]; then
    CMD0="$TRACE $SOCAT $opts $diropt $ADDR:$tsock,$addropts $ADDR2"
else
    CMD0="$TRACE $SOCAT $opts $diropt $ADDR,$fileopt=$tsock,$addropts $ADDR2"
fi
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"$te0" &
pid0=$!
wait${proto} "$crit" $tsock 1 2>"$tlog"
kill $pid0 2>>"$tlog"
rc1=$?
wait >>"$tlog"
if [ $rc1 != 0 ]; then
    $PRINTF "${YELLOW}setup failed${NORMAL}\n"
    echo "$CMD0 &"
    cat "$te0"
    cat "$tlog"
    let numCANT=numCANT+1
elif ! [ $crit $tsock ]; then
    $PRINTF "$OK\n"
    let numOK=numOK+1
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    cat "$te0"
    cat "$tlog"
    let numFAIL=numFAIL+1
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))
#
done <<<"
# address     fileopt addropts waitfor direction crit ADDR2
unix-listen   .       .        unixport .        -e   FILE:/dev/null
unix-recvfrom .       .        unixport .        -e   FILE:/dev/null
unix-recv     .       .        unixport -u       -e   FILE:/dev/null
pipe          .       .        file     -u       -e   FILE:/dev/null
pty           link    .        file     .        -L   PIPE
"


# tests: is "passive" filesystem entry removed at the end? (with fork)
while read addr fileopt addropts proto diropt crit ADDR2; do
if [ -z "$addr" ] || [[ "$addr" == \#* ]]; then continue; fi
# some passive (listening...) filesystem based addresses with fork did not remove
# the file system entry at the end
ADDR=${addr^^*}
ADDR_=${ADDR/-/_}
PROTO=${proto^^*}
if [ "$diropt" = "." ]; then diropt=; fi
if [ "$fileopt" = "." ]; then fileopt=; fi
if [ "$addropts" = "." ]; then addropts=; fi
# $ADDR with fork removes the file system entry when the process is terminated
NAME=${ADDR_}_REMOVE_FORK
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%unix%*|*%socket%*|*%$NAME%*)
TEST="$NAME: $ADDR with fork removes socket entry when terminated during accept"
# start a socat process with listening unix domain socket etc and option fork.
# Terminate the process and check if the file system socket entry still exists.
# Test succeeds when entry does not exist.
if ! eval $NUMCOND; then :; else
tlog="$td/test$N.log"
te0="$td/test$N.0.stderr"
tsock="$td/test$N.sock"
if [ -z "$fileopt" ]; then
    CMD0="$TRACE $SOCAT $opts $diropt $ADDR:$tsock,fork,$addropts $ADDR2"
else
    CMD0="$TRACE $SOCAT $opts $diropt $ADDR,fork,$fileopt=$tsock,$addropts $ADDR2"
fi
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"$te0" &
pid0=$!
wait${proto} "$crit" $tsock 1 2>"$tlog"
kill $pid0 2>>"$tlog"
rc1=$?
wait
if [ $rc1 != 0 ]; then
    $PRINTF "${YELLOW}setup failed${NORMAL}\n"
    echo "$CMD0 &"
    cat "$te0"
    cat "$tlog"
    let numCANT=numCANT+1
elif ! [ $crit $tsock ]; then
    $PRINTF "$OK\n"
    let numOK=numOK+1
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    cat "$te0"
    cat "$tlog"
    let numFAIL=numFAIL+1
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))
#
done <<<"
# address     fileopt addropts waitfor direction crit ADDR2
unix-listen   .       .        unixport .        -e   FILE:/dev/null
unix-recvfrom .       .        unixport .        -e   FILE:/dev/null
"


# bug fix: SYSTEM address child process shut down parents sockets including
# SSL connection under some circumstances.
NAME=SYSTEM_SHUTDOWN
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%system%*|*%openssl%*|*%socket%*|*%$NAME%*)
TEST="$NAME: SYSTEM address does not shutdown its parents addresses"
# start an OpenSSL echo server using SYSTEM:cat
# start an OpenSSL client that sends data
# when the client recieves its data and terminates without error the test succeeded
# in case of the bug the client issues an error like:
# SSL_connect(): error:1408F119:SSL routines:SSL3_GET_RECORD:decryption failed or bad record mac
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$SOCAT $opts OPENSSL-LISTEN:$PORT,reuseaddr,cipher=aNULL,verify=0 SYSTEM:cat"
CMD1="$SOCAT $opts - OPENSSL-CONNECT:$LOCALHOST:$PORT,cipher=aNULL,verify=0"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
waittcp4port $PORT 1
echo "$da" |$CMD1 >"${tf}1" 2>"${te}1"
rc1=$?
kill $pid0 2>/dev/null; wait
if [ $rc1 -ne 0 ]; then
    $PRINTF "$FAILED\n"
    echo "rc1=$rc1"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "${tf}1" >"$tdiff" 2>&1; then
    $PRINTF "$FAILED\n"
    echo "diff:"
    cat "$tdiff"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# test if TCP4-LISTEN with empty port arg terminates with error
NAME=TCP4_NOPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%socket%*|*%tcp%*|*%tcp4%*|*%$NAME%*)
TEST="$NAME: test if TCP4-LISTEN with empty port arg bails out"
# run socat with TCP4-LISTEN with empty port arg. Check if it terminates
# immediately with return code 1
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
t0rc="$td/test$N.rc"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$SOCAT $opts TCP4-LISTEN: /dev/null"
printf "test $F_n $TEST... " $N
{ $CMD0 >/dev/null 2>"${te}0"; echo $? >"$t0rc"; } & 2>/dev/null
pid0=$!
sleep 1
kill $pid0 2>/dev/null; wait
if [ ! -f "$t0rc" ]; then
    $PRINTF "$FAILED\n"
    echo "no return code of CMD0 stored" >&2
    echo "$CMD0 &"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo 1 |diff - "$t0rc" >"$tdiff"; then
    $PRINTF "$FAILED\n"
    echo "CMD0 exited with $(cat $t0rc), expected 1"
    echo "$CMD0 &"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))

# the OPENSSL_METHOD_DTLS1 test hangs sometimes, probably depending on the openssl version.
OPENSSL_VERSION="$(openssl version)"
OPENSSL_VERSION="${OPENSSL_VERSION#* }"
OPENSSL_VERSION="${OPENSSL_VERSION%%-*}"
OPENSSL_VERSION_GOOD=1.0.2 	# this is just a guess.
				# known bad:  1.0.1e
				# known good: 1.0.2j

# test if the various SSL methods can be used with OpenSSL
for method in SSL3 SSL23 TLS1 TLS1.1 TLS1.2 DTLS1; do

NAME=OPENSSL_METHOD_$method
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%socket%*|*%openssl%*|*%$NAME%*)
TEST="$NAME: test OpenSSL method $method"
# Start a socat process listening with OpenSSL and echoing data,
# using the selected method
# Start a second socat process connecting to the listener using
# the same method, send some data and catch the reply.
# If the reply is identical to the sent data the test succeeded.
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$SOCAT $opts OPENSSL-LISTEN:$PORT,reuseaddr,method=$method,cipher=aNULL,verify=0 PIPE"
CMD1="$SOCAT $opts - OPENSSL-CONNECT:$LOCALHOST:$PORT,method=$method,cipher=aNULL,verify=0"
printf "test $F_n $TEST... " $N
if [ "$method" = DTLS1 -a "$(echo -e "$OPENSSL_VERSION\n1.0.2" |sort -V |tail -n 1)" = "$OPENSSL_VERSION_GOOD" ]; then
    $PRINTF "${YELLOW}might hang, skipping${NORMAL}\n"
    numCANT=$((numCANT+1))
else
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
waittcp4port $PORT 1
echo "$da" |$CMD1 >"${tf}1" 2>"${te}1"
rc1=$?
kill $pid0 2>/dev/null; wait
if echo "$da" |diff - "${tf}1"; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
    if [ "$VERBOSE" ]; then
	echo "  $CMD0"
	echo "  echo \"$da\" |$CMD1"
    fi
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # !DTLS1 hang
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))

done


# Address options fdin and fdout were silently ignored when not applicable
# due to -u or -U option. Now these combinations are caught as errors.
NAME=FDOUT_ERROR
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%socket%*|*%$NAME%*)
TEST="$NAME: fdout bails out in write-only context"
# use EXEC in write-only context with option fdout. Expected behaviour: error
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD="$SOCAT $opts -u /dev/null EXEC:cat,fdout=1"
printf "test $F_n $TEST... " $N
$CMD >/dev/null 2>"${te}"
rc=$?
if [ $rc -eq 1 ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD"
    cat "${te}"
    echo "command did not terminate with error!"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# test if failure exit code of SYSTEM invocation causes socat to also exit
# with !=0
NAME=SYSTEM_RC
case "$TESTS" in
*%$N%*|*%functions%*|*%system%*|*%$NAME%*)
TEST="$NAME: promote failure of SYSTEM"
# run socat with SYSTEM:false and check if socat exits with !=0
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
# shut-none makes sure that the child is not killed by parent
CMD0="$TRACE $SOCAT $opts - SYSTEM:false,shut-none"
printf "test $F_n $TEST... " $N
sleep 1 |$CMD0 >/dev/null 2>"${te}0"
rc0=$?
if [ $rc0 -eq 0 ]; then
    $PRINTF "$FAILED\n"
    echo "$CMD0"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# test if failure exit code of EXEC invocation causes socat to also exit
# with !=0
NAME=EXEC_RC
case "$TESTS" in
*%$N%*|*%functions%*|*%exec%*|*%$NAME%*)
TEST="$NAME: promote failure of EXEC"
# run socat with EXEC:false and check if socat exits with !=0
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
# shut-none makes sure that the child is not killed by parent
CMD0="$TRACE $SOCAT $opts - EXEC:false,shut-none"
printf "test $F_n $TEST... " $N
sleep 1 |$CMD0 >/dev/null 2>"${te}0"
rc0=$?
if [ $rc0 -eq 0 ]; then
    $PRINTF "$FAILED\n"
    echo "$CMD0"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# test the so-reuseaddr option
NAME=SO_REUSEADDR
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%tcp%*|*%socket%*|*%$NAME%*)
TEST="$NAME: test the so-reuseaddr option"
# process 0 provides a tcp listening socket with so-reuseaddr;
# process 1 connects to this port; thus the port is connected but no longer
# listening
# process 2 tries to listen on this port with SO_REUSEADDR, will fail if the
# SO_REUSEADDR socket options did not work
# process 3 connects to this port; only if it is successful the test is ok
if ! eval $NUMCOND; then :;
elif ! feat=$(testoptions so-reuseaddr); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tp="$PORT"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts TCP4-L:$tp,so-reuseaddr PIPE"
CMD1="$TRACE $SOCAT $opts - TCP:localhost:$tp"
CMD2="$CMD0"
CMD3="$CMD1"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
waittcp4port $tp 1
(echo "$da"; sleep 3) |$CMD1 >"$tf" 2>"${te}1" &	# this should always work
pid1=$!
usleep 1000000
$CMD2 >/dev/null 2>"${te}2" &
pid2=$!
waittcp4port $tp 1
(echo "$da") |$CMD3 >"${tf}3" 2>"${te}3"
rc3=$?
kill $pid0 $pid1 $pid2 2>/dev/null; wait
if ! echo "$da" |diff - "$tf"; then
    $PRINTF "${YELLOW}phase 1 failed${NORMAL}\n"
    echo "$CMD0 &"
    echo "$CMD1"
    numCANT=$((numCANT+1))
elif [ $rc3 -ne 0 ]; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD0 &"
    echo "$CMD1"
    echo "$CMD2 &"
    echo "$CMD3"
    cat "${te}2" "${te}3"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da" |diff - "${tf}3"; then
    $PRINTF "$FAILED: $TRACE $SOCAT:\n"
    echo "$CMD0 &"
    echo "$CMD1"
    echo "$CMD2 &"
    echo "$CMD3"
    echo "$da" |diff - "${tf}3"
    numCANT=$((numCANT+1))
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}0" "${te}1" "${te}2" "${te}3"; fi
    numOK=$((numOK+1))
fi
fi # NUMCOND, SO_REUSEADDR
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# test the so-reuseport option
NAME=SO_REUSEPORT
case "$TESTS" in
*%$N%*|*%functions%*|*%ip4%*|*%tcp%*|*%socket%*|*%$NAME%*)
TEST="$NAME: test the so-reuseport option"
# process 0 provides a tcp listening socket with so-reuseport;
# process 1 provides an equivalent tcp listening socket with so-reuseport;
# process 2 connects to this port and transfers data
# process 3 connects to this port and transfers data
# test succeeds when both data transfers work
if ! eval $NUMCOND; then :;
elif ! feat=$(testoptions so-reuseport); then
    $PRINTF "test $F_n $TEST... ${YELLOW}$feat not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tp="$PORT"
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da2="test$N $(date) $RANDOM"
da3="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts TCP4-L:$tp,so-reuseport PIPE"
CMD1="$CMD0"
CMD2="$TRACE $SOCAT $opts - TCP:localhost:$tp"
CMD3="$CMD2"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
$CMD1 >/dev/null 2>"${te}1" &
pid1=$!
waittcp4port $tp 1
(echo "$da2") |$CMD2 >"${tf}2" 2>"${te}2"	# this should always work
rc2=$?
(echo "$da3") |$CMD3 >"${tf}3" 2>"${te}3"
rc3=$?
kill $pid0 $pid1 $pid2 2>/dev/null; wait
if ! echo "$da2" |diff - "${tf}2"; then
    $PRINTF "${YELLOW}phase 1 failed${NORMAL}\n"
    echo "$CMD0 &"
    echo "$CMD1 &"
    echo "$CMD2"
    cat "${te}0" "${te}1" "${te}2"
    numCANT=$((numCANT+1))
elif [ $rc3 -ne 0 ]; then
    $PRINTF "$FAILED:\n"
    echo "$CMD0 &"
    echo "$CMD1 &"
    echo "$CMD2"
    echo "$CMD3"
    cat "${te}0" "${te}1" "${te}2" "${te}3"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da2" |diff - "${tf}2"; then
    $PRINTF "$FAILED:\n"
    echo "$CMD0 &"
    echo "$CMD1"
    echo "$CMD2"
    echo "$CMD3"
    cat "${te}0" "${te}1" "${te}2" "${te}3"
    echo "$da2" |diff - "${tf}2"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif ! echo "$da3" |diff - "${tf}3"; then
    $PRINTF "$FAILED:\n"
    echo "$CMD0 &"
    echo "$CMD1"
    echo "$CMD2"
    echo "$CMD3"
    cat "${te}0" "${te}1" "${te}2" "${te}3"
    echo "$da3" |diff - "${tf}3"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
else
    $PRINTF "$OK\n"
    if [ -n "$debug" ]; then cat "${te}0" "${te}1" "${te}2" "${te}3"; fi
    numOK=$((numOK+1))
fi
fi # NUMCOND, SO_REUSEPORT
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# Programs invoked with EXEC, nofork, and -u or -U had stdin and stdout assignment swapped. 
NAME=EXEC_NOFORK_UNIDIR
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%exec%*|*%$NAME%*)
TEST="$NAME: Programs invoked with EXEC, nofork, and -u or -U had stdin and stdout assignment swapped"
# invoke a simple echo command with EXEC, nofork, and -u
# expected behaviour: output appears on stdout
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts -u /dev/null EXEC:\"echo \\\\\\\"\\\"$da\\\"\\\\\\\"\",nofork"
printf "test $F_n $TEST... " $N
eval "$CMD0" >"${tf}0" 2>"${te}0"
rc1=$?
if echo "$da" |diff - "${tf}0" >"$tdiff"; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0"
    cat "${te}0"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
#PORT=$((PORT+1))
N=$((N+1))


# OpenSSL ECDHE ciphers were introduced in socat 1.7.3.0 but in the same release
# they were broken by a porting effort. This test checks if OpenSSL ECDHE works
NAME=OPENSSL_ECDHE
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%openssl%*|*%socket%*|*%$NAME%*)
TEST="$NAME: test OpenSSL ECDHE"
# generate a ECDHE key, start an OpenSSL server, connect with a client and try to
# pass data
if ! eval $NUMCOND; then :;
elif ! testaddrs openssl >/dev/null; then
    $PRINTF "test $F_n $TEST... ${YELLOW}OPENSSL not available${NORMAL}\n" $N
    numCANT=$((numCANT+1))
else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
TESTSRV=./testsrvec
gentesteccert $TESTSRV
CMD0="$TRACE $SOCAT $opts OPENSSL-LISTEN:$PORT,reuseaddr,cert=testsrvec.crt,key=$TESTSRV.pem,verify=0 PIPE"
CMD1="$TRACE $SOCAT $opts - OPENSSL-CONNECT:$LOCALHOST:$PORT,cipher=ECDHE-ECDSA-AES256-GCM-SHA384,cafile=$TESTSRV.crt"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
waittcp4port $PORT 1
echo "$da" |$CMD1 >"${tf}1" 2>"${te}1"
rc1=$?
kill $pid0 2>/dev/null; wait
if [ $rc1 -ne 0 ]; then
    $PRINTF "$FAILED\n"
    echo "failure symptom: client error" >&2
    echo "server and stderr:" >&2
    echo "$CMD0 &"
    cat "${te}0"
    echo "client and stderr:" >&2
    echo "$CMD1"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif echo "$da" |diff - "${tf}1" >"$tdiff"; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "server and stderr:" >&2
    echo "$CMD1"
    cat "${te}1"
    echo "client and stderr:" >&2
    echo "$CMD0 &"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))


# option ipv6-join-group "could not be used"
# fixed in 1.7.3.2
NAME=USE_IPV6_JOIN_GROUP
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%socket%*|*%ip6%*|*%udp%*|*%udp6%*|*%dgram%*|*%$NAME%*)
TEST="$NAME: is option ipv6-join-group used"
# Invoke socat with option ipv6-join-group on UDP6 address.
# Terminate immediately, do not transfer data.
# If socat exits with 0 the test succeeds.
# Up to 1.7.3.1 it failed with "1 option(s) could not be used"
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts UDP6-RECV:$PORT,ipv6-join-group=[ff02::2]:$INTERFACE /dev/null"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0"
rc0=$?
if [ $rc0 -eq 0 ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0"
    cat "${te}0"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))

##################################################################################
#=================================================================================
# here come tests that might affect your systems integrity. Put normal tests
# before this paragraph.
# tests must be explicitely selected by roottough or name (not number)

NAME=PTYGROUPLATE
case "$TESTS" in
*%roottough%*|*%$NAME%*)
TEST="$NAME: pty with group-late works on pty"
# up to socat 1.7.1.1 address pty changed the ownership of /dev/ptmx instead of
# the pty with options user-late, group-late, or perm-late.
# here we check for correct behaviour. 
# ATTENTION: in case of failure of this test the
# group of /dev/ptmx might be changed!
if ! eval $NUMCOND; then :; else
# save current /dev/ptmx properties
F=
for f in /dev/ptmx /dev/ptc; do
    if [ -e $f ]; then
	F=$(echo "$f" |tr / ..)
	ls -l $f >"$td/test$N.$F.ls-l"
	break
    fi
done
printf "test $F_n $TEST... " $N
if [ -z "$F" ]; then
    echo -e "${YELLOW}no /dev/ptmx or /dev/ptc${NORMAL}"
else
GROUP=daemon
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tl="$td/test$N.pty"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts pty,link=$tl,group-late=$GROUP,escape=0x1a PIPE"
CMD1="$TRACE $SOCAT $opts - $tl,raw,echo=0"
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
(echo "$da"; usleep $MICROS; echo -e "\x1a") |$CMD1 >"${tf}1" 2>"${te}1" >"$tf"
rc1=$?
kill $pid0 2>/dev/null; wait
if [ $rc1 -ne 0 ]; then
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
elif echo "$da" |diff - "$tf" >$tdiff; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    cat "$tdiff"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
if ! ls -l $f |diff "$td/test$N.$F.ls-l" -; then
    $PRINTF "${RED}this test changed properties of $f!${NORMAL}\n"
fi
fi # no /dev/ptmx
fi # NUMCOND
 ;;
esac
N=$((N+1))


echo "summary: $((N-1)) tests, $((numOK+numFAIL+numCANT)) selected; $numOK ok, $numFAIL failed, $numCANT could not be performed"

if [ "$numFAIL" -gt 0 ]; then
    echo "FAILED: $listFAIL"
    exit 1
fi
exit 0

#==============================================================================

rm -f testsrv.* testcli.* testsrvdsa* testsrvfips* testclifips*

# end

# too dangerous - run as root and having a shell problem, it might purge your
# file systems 
#rm -r "$td"

# sometimes subprocesses hang; we want to see this
wait

exit

# test template

# give a description of what is tested (a bugfix, a new feature...)
NAME=SHORT_UNIQUE_TESTNAME
case "$TESTS" in
*%$N%*|*%functions%*|*%bugs%*|*%socket%*|*%$NAME%*)
TEST="$NAME: give a one line description of test"
# describe how the test is performed, and what's the success criteria
if ! eval $NUMCOND; then :; else
tf="$td/test$N.stdout"
te="$td/test$N.stderr"
tdiff="$td/test$N.diff"
da="test$N $(date) $RANDOM"
CMD0="$TRACE $SOCAT $opts server-address PIPE"
CMD1="$TRACE $SOCAT $opts - client-address"
printf "test $F_n $TEST... " $N
$CMD0 >/dev/null 2>"${te}0" &
pid0=$!
wait<something>port $PORT 1
echo "$da" |$CMD1 >"${tf}1" 2>"${te}1"
rc1=$?
kill $pid0 2>/dev/null; wait
if [ !!! ]; then
    $PRINTF "$OK\n"
    numOK=$((numOK+1))
else
    $PRINTF "$FAILED\n"
    echo "$CMD0 &"
    echo "$CMD1"
    cat "${te}0"
    cat "${te}1"
    numFAIL=$((numFAIL+1))
    listFAIL="$listFAIL $N"
fi
fi # NUMCOND
 ;;
esac
PORT=$((PORT+1))
N=$((N+1))
