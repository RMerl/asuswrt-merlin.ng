#!/bin/sh

#scipt to create redsocks config file

if [ $# -ne 3 ]; then

echo "Wrong Syntax"
echo "#### Usage#### :"
echo "redsocks.sh path_to_create_conf_file proxy_server_ip proxy_server_port"
echo " ex: redsocks.sh /var/redsocks.conf 4.4.4.4 1080"
exit
fi

CONF_FILE=$1
PORXY_SERVER_IP=$2
PORXY_SERVER_PORT=$3

echo "creating redsocks config file  $CONF_FILE"

cat > $CONF_FILE << EOF
base {
	// debug: connection progress & client list on SIGUSR1
	log_debug = off;

	// info: start and end of client session
	log_info = off;

	/* possible log values are:
	 *   stderr
	 *   "file:/path/to/file"
	 *   syslog:FACILITY  facility is any of "daemon", "local0"..."local7"
	 */
	//log = "syslog:daemon";
	log = "file:/var/redsocks.log";

	daemon = on;
	redirector = iptables;

	redsocks_conn_max = 512;
}

redsocks {
	/* 'local_ip' defaults to 127.0.0.1 for security reasons,
	 * use 0.0.0.0 if you want to listen on every interface.
	 * 'local_*' are used as port to redirect to.
	 */
	local_ip = 0.0.0.0;
	local_port = 12345;

	// 'ip' and 'port' are IP and tcp-port of proxy-server
	// You can also use hostname instead of IP, only one (random)
	// address of multihomed host will be used.
	ip = $PORXY_SERVER_IP;
	port = $PORXY_SERVER_PORT;

	// known types: socks4, socks5, http-connect, http-relay
	type = socks5;
}

EOF

killall -9 redsocks
redsocks -c $CONF_FILE
