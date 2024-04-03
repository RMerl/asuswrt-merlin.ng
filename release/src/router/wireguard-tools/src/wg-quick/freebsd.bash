#!/usr/local/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
#

set -e -o pipefail
shopt -s extglob
export LC_ALL=C

exec 3>&2
SELF="$(readlink -f "${BASH_SOURCE[0]}")"
export PATH="${SELF%/*}:$PATH"

WG_CONFIG=""
INTERFACE=""
ADDRESSES=( )
MTU=""
DNS=( )
DNS_SEARCH=( )
TABLE=""
PRE_UP=( )
POST_UP=( )
PRE_DOWN=( )
POST_DOWN=( )
SAVE_CONFIG=0
CONFIG_FILE=""
PROGRAM="${0##*/}"
ARGS=( "$@" )

cmd() {
	echo "[#] $*" >&3
	"$@"
}

die() {
	echo "$PROGRAM: $*" >&2
	exit 1
}

CONFIG_SEARCH_PATHS=( /etc/wireguard /usr/local/etc/wireguard )

unset ORIGINAL_TMPDIR
make_temp() {
	local old_umask

	[[ -v ORIGINAL_TMPDIR ]] && export TMPDIR="$ORIGINAL_TMPDIR"
	ORIGINAL_TMPDIR="$TMPDIR"
	[[ -z $TMPDIR ]] && unset TMPDIR

	old_umask="$(umask)"
	umask 077
	export TMPDIR="$(mktemp -d)"
	umask "$old_umask"

	[[ -d $TMPDIR ]] || die "Unable to create safe temporary directory"
	CLEANUP_TMPDIR="$TMPDIR"
}

clean_temp() {
	[[ -n $CLEANUP_TMPDIR ]] && rm -rf "$CLEANUP_TMPDIR"
}

parse_options() {
	local interface_section=0 line key value stripped path v
	CONFIG_FILE="$1"
	if [[ $CONFIG_FILE =~ ^[a-zA-Z0-9_=+.-]{1,15}$ ]]; then
		for path in "${CONFIG_SEARCH_PATHS[@]}"; do
			CONFIG_FILE="$path/$1.conf"
			[[ -e $CONFIG_FILE ]] && break
		done
	fi
	[[ -e $CONFIG_FILE ]] || die "\`$CONFIG_FILE' does not exist"
	[[ $CONFIG_FILE =~ (^|/)([a-zA-Z0-9_=+.-]{1,15})\.conf$ ]] || die "The config file must be a valid interface name, followed by .conf"
	CONFIG_FILE="$(readlink -f "$CONFIG_FILE")"
	((($(stat -f '0%#p' "$CONFIG_FILE") & $(stat -f '0%#p' "${CONFIG_FILE%/*}") & 0007) == 0)) || echo "Warning: \`$CONFIG_FILE' is world accessible" >&2
	INTERFACE="${BASH_REMATCH[2]}"
	shopt -s nocasematch
	while read -r line || [[ -n $line ]]; do
		stripped="${line%%\#*}"
		key="${stripped%%=*}"; key="${key##*([[:space:]])}"; key="${key%%*([[:space:]])}"
		value="${stripped#*=}"; value="${value##*([[:space:]])}"; value="${value%%*([[:space:]])}"
		[[ $key == "["* ]] && interface_section=0
		[[ $key == "[Interface]" ]] && interface_section=1
		if [[ $interface_section -eq 1 ]]; then
			case "$key" in
			Address) ADDRESSES+=( ${value//,/ } ); continue ;;
			MTU) MTU="$value"; continue ;;
			DNS) for v in ${value//,/ }; do
				[[ $v =~ (^[0-9.]+$)|(^.*:.*$) ]] && DNS+=( $v ) || DNS_SEARCH+=( $v )
			done; continue ;;
			Table) TABLE="$value"; continue ;;
			PreUp) PRE_UP+=( "$value" ); continue ;;
			PreDown) PRE_DOWN+=( "$value" ); continue ;;
			PostUp) POST_UP+=( "$value" ); continue ;;
			PostDown) POST_DOWN+=( "$value" ); continue ;;
			SaveConfig) read_bool SAVE_CONFIG "$value"; continue ;;
			esac
		fi
		WG_CONFIG+="$line"$'\n'
	done < "$CONFIG_FILE"
	shopt -u nocasematch
}

read_bool() {
	case "$2" in
	true) printf -v "$1" 1 ;;
	false) printf -v "$1" 0 ;;
	*) die "\`$2' is neither true nor false"
	esac
}

auto_su() {
	[[ $UID == 0 ]] || exec sudo -p "$PROGRAM must be run as root. Please enter the password for %u to continue: " -- "$BASH" -- "$SELF" "${ARGS[@]}"
}

add_if() {
	local ret rc
	if ret="$(cmd ifconfig wg create name "$INTERFACE" 2>&1 >/dev/null)"; then
		return 0
	fi
	rc=$?
	if [[ $ret == *"ifconfig: ioctl SIOCSIFNAME (set name): File exists"* ]]; then
		echo "$ret" >&3
		return $rc
	fi
	echo "[!] Missing WireGuard kernel support ($ret). Falling back to slow userspace implementation." >&3
	cmd "${WG_QUICK_USERSPACE_IMPLEMENTATION:-wireguard-go}" "$INTERFACE"
}

del_routes() {
	local todelete=( ) destination gateway netif
	while read -r destination _ _ _ _ netif _; do
		[[ $netif == "$INTERFACE" ]] && todelete+=( "$destination" )
	done < <(netstat -nr -f inet)
	for destination in "${todelete[@]}"; do
		cmd route -q -n delete -inet "$destination" || true
	done
	todelete=( )
	while read -r destination gateway _ netif; do
		[[ $netif == "$INTERFACE" || ( $netif == lo* && $gateway == "$INTERFACE" ) ]] && todelete+=( "$destination" )
	done < <(netstat -nr -f inet6)
	for destination in "${todelete[@]}"; do
		cmd route -q -n delete -inet6 "$destination" || true
	done
	for destination in "${ENDPOINTS[@]}"; do
		if [[ $destination == *:* ]]; then
			cmd route -q -n delete -inet6 "$destination" || true
		else
			cmd route -q -n delete -inet "$destination" || true
		fi
	done
}

del_if() {
	[[ $HAVE_SET_DNS -eq 0 ]] || unset_dns
	if [[ -S /var/run/wireguard/$INTERFACE.sock ]]; then
		cmd rm -f "/var/run/wireguard/$INTERFACE.sock"
	else
		cmd ifconfig "$INTERFACE" destroy
	fi
	while ifconfig "$INTERFACE" >/dev/null 2>&1; do
		# HACK: it would be nice to `route monitor` here and wait for RTM_IFANNOUNCE
		# but it turns out that the announcement is made before the interface
		# disappears so we sometimes get a hang. So, we're instead left with polling
		# in a sleep loop like this.
		sleep 0.1
	done
}

up_if() {
	cmd ifconfig "$INTERFACE" up
}

add_addr() {
	if [[ $1 == *:* ]]; then
		cmd ifconfig "$INTERFACE" inet6 "$1" alias
	else
		cmd ifconfig "$INTERFACE" inet "$1" alias
	fi
}

set_mtu() {
	local mtu=0 endpoint output family
	if [[ -n $MTU ]]; then
		cmd ifconfig "$INTERFACE" mtu "$MTU"
		return
	fi
	while read -r _ endpoint; do
		[[ $endpoint =~ ^\[?([a-z0-9:.]+)\]?:[0-9]+$ ]] || continue
		family=inet
		[[ ${BASH_REMATCH[1]} == *:* ]] && family=inet6
		output="$(route -n get "-$family" "${BASH_REMATCH[1]}" || true)"
		[[ $output =~ interface:\ ([^ ]+)$'\n' && $(ifconfig "${BASH_REMATCH[1]}") =~ mtu\ ([0-9]+) && ${BASH_REMATCH[1]} -gt $mtu ]] && mtu="${BASH_REMATCH[1]}"
	done < <(wg show "$INTERFACE" endpoints)
	if [[ $mtu -eq 0 ]]; then
		read -r output < <(route -n get default || true) || true
		[[ $output =~ interface:\ ([^ ]+)$'\n' && $(ifconfig "${BASH_REMATCH[1]}") =~ mtu\ ([0-9]+) && ${BASH_REMATCH[1]} -gt $mtu ]] && mtu="${BASH_REMATCH[1]}"
	fi
	[[ $mtu -gt 0 ]] || mtu=1500
	cmd ifconfig "$INTERFACE" mtu $(( mtu - 80 ))
}


collect_gateways() {
	local destination gateway

	GATEWAY4=""
	while read -r destination gateway _; do
		[[ $destination == default ]] || continue
		GATEWAY4="$gateway"
		break
	done < <(netstat -nr -f inet)

	GATEWAY6=""
	while read -r destination gateway _; do
		[[ $destination == default ]] || continue
		GATEWAY6="$gateway"
		break
	done < <(netstat -nr -f inet6)
}

collect_endpoints() {
	ENDPOINTS=( )
	while read -r _ endpoint; do
		[[ $endpoint =~ ^\[?([a-z0-9:.]+)\]?:[0-9]+$ ]] || continue
		ENDPOINTS+=( "${BASH_REMATCH[1]}" )
	done < <(wg show "$INTERFACE" endpoints)
}

set_endpoint_direct_route() {
	local old_endpoints endpoint old_gateway4 old_gateway6 remove_all_old=0 added=( )
	old_endpoints=( "${ENDPOINTS[@]}" )
	old_gateway4="$GATEWAY4"
	old_gateway6="$GATEWAY6"
	collect_gateways
	collect_endpoints

	[[ $old_gateway4 != "$GATEWAY4" || $old_gateway6 != "$GATEWAY6" ]] && remove_all_old=1

	if [[ $remove_all_old -eq 1 ]]; then
		for endpoint in "${ENDPOINTS[@]}"; do
			[[ " ${old_endpoints[*]} " == *" $endpoint "* ]] || old_endpoints+=( "$endpoint" )
		done
	fi

	for endpoint in "${old_endpoints[@]}"; do
		[[ $remove_all_old -eq 0 && " ${ENDPOINTS[*]} " == *" $endpoint "* ]] && continue
		if [[ $endpoint == *:* && $AUTO_ROUTE6 -eq 1 ]]; then
			cmd route -q -n delete -inet6 "$endpoint" 2>/dev/null || true
		elif [[ $AUTO_ROUTE4 -eq 1 ]]; then
			cmd route -q -n delete -inet "$endpoint" 2>/dev/null || true
		fi
	done

	for endpoint in "${ENDPOINTS[@]}"; do
		if [[ $remove_all_old -eq 0 && " ${old_endpoints[*]} " == *" $endpoint "* ]]; then
			added+=( "$endpoint" )
			continue
		fi
		if [[ $endpoint == *:* && $AUTO_ROUTE6 -eq 1 ]]; then
			if [[ -n $GATEWAY6 ]]; then
				cmd route -q -n add -inet6 "$endpoint" -gateway "$GATEWAY6" || true
			else
				# Prevent routing loop
				cmd route -q -n add -inet6 "$endpoint" ::1 -blackhole || true
			fi
			added+=( "$endpoint" )
		elif [[ $AUTO_ROUTE4 -eq 1 ]]; then
			if [[ -n $GATEWAY4 ]]; then
				cmd route -q -n add -inet "$endpoint" -gateway "$GATEWAY4" || true
			else
				# Prevent routing loop
				cmd route -q -n add -inet "$endpoint" 127.0.0.1 -blackhole || true
			fi
			added+=( "$endpoint" )
		fi
	done
	ENDPOINTS=( "${added[@]}" )
}

monitor_daemon() {
	echo "[+] Backgrounding route monitor" >&2
	(make_temp
	trap 'del_routes; clean_temp; exit 0' INT TERM EXIT
	exec >/dev/null 2>&1
	exec 19< <(exec route -n monitor)
	local event pid=$!
	# TODO: this should also check to see if the endpoint actually changes
	# in response to incoming packets, and then call set_endpoint_direct_route
	# then too. That function should be able to gracefully cleanup if the
	# endpoints change.
	while read -u 19 -r event; do
		[[ $event == RTM_* ]] || continue
		ifconfig "$INTERFACE" >/dev/null 2>&1 || break
		[[ $AUTO_ROUTE4 -eq 1 || $AUTO_ROUTE6 -eq 1 ]] && set_endpoint_direct_route
		# TODO: set the mtu as well, but only if up
	done
	kill $pid) & disown
}

HAVE_SET_DNS=0
set_dns() {
	[[ ${#DNS[@]} -gt 0 ]] || return 0
	{ printf 'nameserver %s\n' "${DNS[@]}"
	  [[ ${#DNS_SEARCH[@]} -eq 0 ]] || printf 'search %s\n' "${DNS_SEARCH[*]}"
	} | cmd resolvconf -a "$INTERFACE" -x
	HAVE_SET_DNS=1
}

unset_dns() {
	[[ ${#DNS[@]} -gt 0 ]] || return 0
	cmd resolvconf -d "$INTERFACE"
}

add_route() {
	[[ $TABLE != off ]] || return 0

	local family=inet
	[[ $1 == *:* ]] && family=inet6

	if [[ -n $TABLE && $TABLE != auto ]]; then
		cmd route -q -n add "-$family" -fib "$TABLE" "$1" -interface "$INTERFACE"
	elif [[ $1 == */0 ]]; then
		if [[ $1 == *:* ]]; then
			AUTO_ROUTE6=1
			cmd route -q -n add -inet6 ::/1 -interface "$INTERFACE"
			cmd route -q -n add -inet6 8000::/1 -interface "$INTERFACE"
		else
			AUTO_ROUTE4=1
			cmd route -q -n add -inet 0.0.0.0/1 -interface "$INTERFACE"
			cmd route -q -n add -inet 128.0.0.0/1 -interface "$INTERFACE"
		fi
	else
		[[ $(route -n get "-$family" "$1" 2>/dev/null) =~ interface:\ $INTERFACE$'\n' ]] || cmd route -q -n add "-$family" "$1" -interface "$INTERFACE"
	fi
}

set_config() {
	echo "$WG_CONFIG" | cmd wg setconf "$INTERFACE" /dev/stdin
}

save_config() {
	local old_umask new_config current_config address cmd
	new_config=$'[Interface]\n'
	{ read -r _; while read -r _ _ _ address _; do
		new_config+="Address = $address"$'\n'
	done } < <(netstat -I "$INTERFACE" -n -W -f inet)
	{ read -r _; while read -r _ _ _ address _; do
		new_config+="Address = $address"$'\n'
	done } < <(netstat -I "$INTERFACE" -n -W -f inet6)
	while read -r address; do
		[[ $address =~ ^nameserver\ ([a-zA-Z0-9_=+:%.-]+)$ ]] && new_config+="DNS = ${BASH_REMATCH[1]}"$'\n'
	done < <(resolvconf -l "$INTERFACE" 2>/dev/null)
	[[ -n $MTU ]] && new_config+="MTU = $MTU"$'\n'
	[[ -n $TABLE ]] && new_config+="Table = $TABLE"$'\n'
	[[ $SAVE_CONFIG -eq 0 ]] || new_config+=$'SaveConfig = true\n'
	for cmd in "${PRE_UP[@]}"; do
		new_config+="PreUp = $cmd"$'\n'
	done
	for cmd in "${POST_UP[@]}"; do
		new_config+="PostUp = $cmd"$'\n'
	done
	for cmd in "${PRE_DOWN[@]}"; do
		new_config+="PreDown = $cmd"$'\n'
	done
	for cmd in "${POST_DOWN[@]}"; do
		new_config+="PostDown = $cmd"$'\n'
	done
	old_umask="$(umask)"
	umask 077
	current_config="$(cmd wg showconf "$INTERFACE")"
	trap 'rm -f "$CONFIG_FILE.tmp"; clean_temp; exit' INT TERM EXIT
	echo "${current_config/\[Interface\]$'\n'/$new_config}" > "$CONFIG_FILE.tmp" || die "Could not write configuration file"
	sync "$CONFIG_FILE.tmp"
	mv "$CONFIG_FILE.tmp" "$CONFIG_FILE" || die "Could not move configuration file"
	trap 'clean_temp; exit' INT TERM EXIT
	umask "$old_umask"
}

execute_hooks() {
	local hook
	for hook in "$@"; do
		hook="${hook//%i/$INTERFACE}"
		echo "[#] $hook" >&2
		(eval "$hook")
	done
}

cmd_usage() {
	cat >&2 <<-_EOF
	Usage: $PROGRAM [ up | down | save | strip ] [ CONFIG_FILE | INTERFACE ]

	  CONFIG_FILE is a configuration file, whose filename is the interface name
	  followed by \`.conf'. Otherwise, INTERFACE is an interface name, with
	  configuration found at:
	  ${CONFIG_SEARCH_PATHS[@]/%//INTERFACE.conf}.
	  It is to be readable by wg(8)'s \`setconf' sub-command, with the exception
	  of the following additions to the [Interface] section, which are handled
	  by $PROGRAM:

	  - Address: may be specified one or more times and contains one or more
	    IP addresses (with an optional CIDR mask) to be set for the interface.
	  - DNS: an optional DNS server to use while the device is up.
	  - MTU: an optional MTU for the interface; if unspecified, auto-calculated.
	  - Table: an optional routing table to which routes will be added; if
	    unspecified or \`auto', the default table is used. If \`off', no routes
	    are added.
	  - PreUp, PostUp, PreDown, PostDown: script snippets which will be executed
	    by bash(1) at the corresponding phases of the link, most commonly used
	    to configure DNS. The string \`%i' is expanded to INTERFACE.
	  - SaveConfig: if set to \`true', the configuration is saved from the current
	    state of the interface upon shutdown.

	See wg-quick(8) for more info and examples.
	_EOF
}

cmd_up() {
	local i
	[[ -z $(ifconfig "$INTERFACE" 2>/dev/null) ]] || die "\`$INTERFACE' already exists"
	trap 'del_if; del_routes; clean_temp; exit' INT TERM EXIT
	add_if
	execute_hooks "${PRE_UP[@]}"
	set_config
	for i in "${ADDRESSES[@]}"; do
		add_addr "$i"
	done
	set_mtu
	up_if
	set_dns
	for i in $(while read -r _ i; do for i in $i; do [[ $i =~ ^[0-9a-z:.]+/[0-9]+$ ]] && echo "$i"; done; done < <(wg show "$INTERFACE" allowed-ips) | sort -nr -k 2 -t /); do
		add_route "$i"
	done
	[[ $AUTO_ROUTE4 -eq 1 || $AUTO_ROUTE6 -eq 1 ]] && set_endpoint_direct_route
	monitor_daemon
	execute_hooks "${POST_UP[@]}"
	trap 'clean_temp; exit' INT TERM EXIT
}

cmd_down() {
	[[ " $(wg show interfaces) " == *" $INTERFACE "* ]] || die "\`$INTERFACE' is not a WireGuard interface"
	execute_hooks "${PRE_DOWN[@]}"
	[[ $SAVE_CONFIG -eq 0 ]] || save_config
	del_if
	unset_dns
	execute_hooks "${POST_DOWN[@]}"
}

cmd_save() {
	[[ " $(wg show interfaces) " == *" $INTERFACE "* ]] || die "\`$INTERFACE' is not a WireGuard interface"
	save_config
}

cmd_strip() {
	echo "$WG_CONFIG"
}

# ~~ function override insertion point ~~

make_temp
trap 'clean_temp; exit' INT TERM EXIT

if [[ $# -eq 1 && ( $1 == --help || $1 == -h || $1 == help ) ]]; then
	cmd_usage
elif [[ $# -eq 2 && $1 == up ]]; then
	auto_su
	parse_options "$2"
	cmd_up
elif [[ $# -eq 2 && $1 == down ]]; then
	auto_su
	parse_options "$2"
	cmd_down
elif [[ $# -eq 2 && $1 == save ]]; then
	auto_su
	parse_options "$2"
	cmd_save
elif [[ $# -eq 2 && $1 == strip ]]; then
	auto_su
	parse_options "$2"
	cmd_strip
else
	cmd_usage
	exit 1
fi

exit 0
