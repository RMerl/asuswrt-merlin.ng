#!/usr/bin/env bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
SERVER_IP="8.8.8.8"
SERVER_IPv6="2001:4860:4860::8888"
TLS_SERVER_IP="185.49.141.38~getdnsapi.net"
TLS_SERVER_IPv6="2a04:b900:0:100::38~getdnsapi.net"
TLS_SERVER_SS_IP="184.105.193.78~tls-dns-u.odvr.dns-oarc.net"  #Self signed cert
TLS_SERVER_KEY="foxZRnIh9gZpWnl+zEiKa0EJ2rdCGroMWm02gaxSc9S="
TLS_SERVER_SS_KEY="pOXrpUt9kgPgbWxBFFcBTbRH2heo2wHwXp1fd4AEVXI="
TLS_SERVER_WRONG_KEY="foxZRnIh9gZpWnl+zEiKa0EJ2rdCGroMWm02gaxSc1S="
GOOD_RESULT_SYNC="Status was: At least one response was returned"
GOOD_RESULT_ASYNC="successful"
BAD_RESULT_SYNC="1 'Generic error'"
BAD_RESULT_ASYNC="callback_type of 703"
NUM_ARGS=3
GOOD_COUNT=0
FAIL_COUNT=0


check_auth () {
	local my_auth_ok=0;
	auth_result=`echo $1 | sed 's/.*tls_auth_status\": <bindata of "//' | sed 's/\">.*//'`
	if [[ $2 == "-" ]] ; then
		my_auth_ok=1;
	fi
	if [[ $2 == "N" ]] && [[ $auth_result == "None" ]]; then
		my_auth_ok=1;
	fi
	if [[ $2 == "F" ]] && [[ $auth_result == "Failed" ]]; then
		my_auth_ok=1;
	fi
	if [[ $2 == "S" ]] && [[ $auth_result == "Success" ]]; then
		my_auth_ok=1;
	fi
	echo $my_auth_ok;
}

check_trans () {
	local my_trans_ok=0;
	trans_result=`echo $1 | sed "s/.*\"transport\": GETDNS_TRANSPORT_//" | sed 's/ }.*//' | sed 's/,.*//'`
	if [[ $2 == "U" ]] && [[ $trans_result == "UDP" ]]; then
		my_trans_ok=1;
	fi
	if [[ $2 == "T" ]] && [[ $trans_result == "TCP" ]]; then
		my_trans_ok=1;
	fi
	if [[ $2 == "L" ]] && [[ $trans_result == "TLS" ]]; then
		my_trans_ok=1;
	fi
	echo $my_trans_ok;
}

check_good () {
	auth_ok=0;
	result_ok=0;
	trans_ok=0;
	result=`echo $1 | sed 's/ All done.'// | sed 's/.*Response code was: GOOD. '//`
	async_success=`echo $result | grep -c "$GOOD_RESULT_ASYNC"`
	if [[ $result =~ $GOOD_RESULT_SYNC ]] || [[ $async_success =~ 1 ]]; then
		result_ok=1;
	fi
	if [[ $result_ok == 1 ]] ; then
		trans_ok=$(check_trans "$1" "$2")
		auth_ok=$(check_auth "$1" "$3")
	fi
	if [[ $result_ok == 1 ]] && [[ $auth_ok == 1 ]] && [[ $trans_ok == 1 ]]; then
		(( GOOD_COUNT++ ))
		echo -n "PASS: "
	else
		(( FAIL_COUNT++ ))
		echo "FAIL (RESULT): Result: $result  Auth: $auth_ok  Trans: $trans_ok"
		echo -n "FAIL: "
	fi
}

check_bad () {
	result=`echo $1 | grep "An error occurred:" | tail -1 | sed 's/ All done.'//`
	error=` echo $result | sed 's/An error occurred: //'`
	if [[ ! -z $result ]]; then
		if [[ $error =~ $BAD_RESULT_SYNC ]] || [[ $error =~ $BAD_RESULT_ASYNC ]]; then
				(( GOOD_COUNT++ ))
				echo -n "PASS:"
			else
				(( FAIL_COUNT++ ))
				echo "FAIL (RESULT): " $error
				echo -n "FAIL: "
		fi
	else
		(( FAIL_COUNT++ ))
		echo "FAIL (RESULT): " $1
		echo -n "FAIL: "
	fi
}

usage () {
	echo "This is a basic and temporary testing script for the transport list"
	echo "functionality that utilises getdns_query to perform multiple queries."
	echo "It will be replaced by an automated test harness in future, but"
	echo "it can be used to check the basic functionality for now. It is recommended that"
	echo "local or known test servers are used, but it should work with the default servers:"
	echo " - Google Open DNS for TCP and UDP only "
	echo  "- the getdnsapi.net test server Open Resolver for TLS, TCP and UDP"
	echo "NOTE: By default this script assumes it is located in the same directory"
	echo "as the getdns_query binary. If it is not, then the location of the binary"
	echo "can be specified via the command line option."
	echo
	echo "usage: test_transport.sh"
	echo "         -p   path to getdns_query binary"
	echo "         -s   server configured for only TCP and UDP"
	echo "         -t   server configured for TLS, TCP and UDP"
	echo "              (This must include the hostname e.g. 185.49.141.38~getdnsapi.net)"
	echo "         -k   SPKI pin for server configured for TLS, TCP and UDP"
	echo "         -i   Use IPv6 addresses (when using default servers)"
}

while getopts ":p:s:t:k:idh" opt; do
	case $opt in
		d ) set -x ; echo "DEBUG mode set" ;;
		p ) DIR=$OPTARG ;;
		s ) SERVER_IP=$OPTARG ; echo "Setting server to $OPTARG" ;;
		t ) TLS_SERVER_IP=$OPTARG ; echo "Setting TLS server to $OPTARG" ;;
		k ) TLS_SERVER_KEY=$OPTARG ; echo "Setting TLS server key to $OPTARG" ;;
		i ) SERVER_IP=$SERVER_IPv6; TLS_SERVER_IP=$TLS_SERVER_IPv6 ; echo "Using IPv6" ;;
		h ) usage ; exit ;;
	esac
done

TLS_SERVER_IP_NO_NAME=`echo ${TLS_SERVER_IP%~*}`
TLS_SERVER_SS_IP_NO_NAME=`echo ${TLS_SERVER_SS_IP%~*}`
TLS_SERVER_IP_WRONG_NAME=`echo ${TLS_SERVER_IP::${#TLS_SERVER_IP}-1}`

NUM_GOOD_QUERIES=7
GOOD_QUERIES=(
"-s -A  getdnsapi.net -l U        @${SERVER_IP}"              "U" "-"
"-s -A  getdnsapi.net -l T        @${SERVER_IP}"              "T" "-"
"-s -A  getdnsapi.net -l L        @${TLS_SERVER_IP_NO_NAME}"  "L" "N"
"-s -A  getdnsapi.net -l L -m     @${TLS_SERVER_IP}"          "L" "S"
"-s -A  getdnsapi.net -l L -m     @${TLS_SERVER_IP_NO_NAME} -K pin-sha256=\"${TLS_SERVER_KEY}\"" "L" "S"
"-s -A  getdnsapi.net -l L -m     @${TLS_SERVER_IP} -K pin-sha256=\"${TLS_SERVER_KEY}\"" "L" "S"
"-s -A  getdnsapi.net -l L -m     @${TLS_SERVER_SS_IP_NO_NAME} -K pin-sha256=\"${TLS_SERVER_SS_KEY}\"" "L" "S"
"-s -G DNSKEY getdnsapi.net -l U  @${SERVER_IP} -b 512 -D" "U" "-")

NUM_GOOD_FB_QUERIES=6
GOOD_FALLBACK_QUERIES=(
"-s -A getdnsapi.net -l LU     @${SERVER_IP}" "U" "-"
"-s -A getdnsapi.net -l LT     @${SERVER_IP}" "T" "-" 
"-s -A getdnsapi.net -l LT     @${TLS_SERVER_IP_NO_NAME}" "L" "N"
"-s -A getdnsapi.net -l LT -m  @${TLS_SERVER_IP_NO_NAME}" "L" "N"
"-s -A getdnsapi.net -l L      @${SERVER_IP} @${TLS_SERVER_IP_NO_NAME}" "L" "-"
"-s -G DNSKEY getdnsapi.net -l UT  @${SERVER_IP} -b 512 -D" "T" "-")

NOT_AVAILABLE_QUERIES=(
"-s -A getdnsapi.net -l L      @${SERVER_IP}"
"-s -A getdnsapi.net -l L -m   @${TLS_SERVER_IP_WRONG_NAME}"
"-s -A getdnsapi.net -l L -m   @${TLS_SERVER_IP_NO_NAME}"
"-s -A getdnsapi.net -l L -m   @${TLS_SERVER_IP_NO_NAME}    -K pin-sha256=\"${TLS_SERVER_WRONG_KEY}\""
"-s -A getdnsapi.net -l L -m   @${TLS_SERVER_IP}            -K pin-sha256=\"${TLS_SERVER_WRONG_KEY}\""
"-s -A getdnsapi.net -l L -m   @${TLS_SERVER_IP_WRONG_NAME} -K pin-sha256=\"${TLS_SERVER_KEY}\""
"-s -A getdnsapi.net -l L -m   @${TLS_SERVER_IP_WRONG_NAME} -K pin-sha256=\"${TLS_SERVER_WRONG_KEY}\""
"-s -A getdnsapi.net -l L -m   @${TLS_SERVER_SS_IP}         -K pin-sha256=\"${TLS_SERVER_SS_KEY}\"")


echo "Starting transport test"
echo
for (( i = 0; i < 2; i+=1 )); do
	if [[ i -eq 0 ]]; then
		echo "**SYNC Mode**"
	else
		echo
		echo "**ASYNC Mode**"
		SYNC_MODE=" -a "
	fi

	echo "*Success cases:"
	for (( j = 0; j < $NUM_GOOD_QUERIES; j+=1 )); do
	  check_good "`$DIR/getdns_query -V +return_call_reporting $SYNC_MODE ${GOOD_QUERIES[$j*$NUM_ARGS]} 2>/dev/null`" ${GOOD_QUERIES[$((j*NUM_ARGS))+1]} ${GOOD_QUERIES[$((j*NUM_ARGS))+2]}
	  echo "getdns_query $SYNC_MODE ${GOOD_QUERIES[$j*$NUM_ARGS]}"
	  (( COUNT++ ))
	done
	
	echo "*Success fallback cases:"
	for (( j = 0; j < $NUM_GOOD_FB_QUERIES; j+=1 )); do
	    check_good "`$DIR/getdns_query -V +return_call_reporting $SYNC_MODE ${GOOD_FALLBACK_QUERIES[$j*$NUM_ARGS]} 2>/dev/null`" ${GOOD_FALLBACK_QUERIES[$((j*NUM_ARGS))+1]} ${GOOD_FALLBACK_QUERIES[$((j*NUM_ARGS))+2]}
	    echo "getdns_query $SYNC_MODE ${GOOD_FALLBACK_QUERIES[$j*$NUM_ARGS]}  TESTS: ${GOOD_FALLBACK_QUERIES[$((j*NUM_ARGS))+1]} ${GOOD_FALLBACK_QUERIES[$((j*NUM_ARGS))+2]}"
	    (( COUNT++ ))
	done
	
	echo "*Transport not available cases:"
	for (( j = 0; j < ${#NOT_AVAILABLE_QUERIES[@]}; j+=1 )); do
		check_bad "`$DIR/getdns_query -V $SYNC_MODE ${NOT_AVAILABLE_QUERIES[${j}]} 2>&1`"
		echo "getdns_query $SYNC_MODE ${NOT_AVAILABLE_QUERIES[${j}]}"
		(( COUNT++ ))
	done
done

echo
echo "Finished transport test: did $COUNT queries, $GOOD_COUNT passes, $FAIL_COUNT failures"
echo
