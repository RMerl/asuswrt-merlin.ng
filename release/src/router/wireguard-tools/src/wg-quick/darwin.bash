#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (C) 2015-2020 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
#

set -e -o pipefail
shopt -s extglob
export LC_ALL=C

SELF="${BASH_SOURCE[0]}"
[[ $SELF == */* ]] || SELF="./$SELF"
SELF="$(cd "${SELF%/*}" && pwd -P)/${SELF##*/}"
export PATH="/usr/bin:/bin:/usr/sbin:/sbin:${SELF%/*}:$PATH"

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
	echo "[#] $*" >&2
	"$@"
}

die() {
	echo "$PROGRAM: $*" >&2
	exit 1
}

[[ ${BASH_VERSINFO[0]} -ge 4 ]] || die "Version mismatch: bash ${BASH_VERSINFO[0]} detected, when bash 4+ required"

CONFIG_SEARCH_PATHS=( /etc/wireguard /usr/local/etc/wireguard )

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
	CONFIG_FILE="$(cd "${CONFIG_FILE%/*}" && pwd -P)/${CONFIG_FILE##*/}"
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

detect_launchd() {
	unset LAUNCHED_BY_LAUNCHD
	local line
	while read -r line; do
		if [[ $line =~ ^\s*domain\ =\  ]]; then
			LAUNCHED_BY_LAUNCHD=1
			break
		fi
	done < <(launchctl procinfo $$ 2>/dev/null)
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

get_real_interface() {
	local interface diff
	wg show interfaces >/dev/null
	[[ -f "/var/run/wireguard/$INTERFACE.name" ]] || return 1
	interface="$(< "/var/run/wireguard/$INTERFACE.name")"
	[[ -n $interface && -S "/var/run/wireguard/$interface.sock" ]] || return 1
	diff=$(( $(stat -f %m "/var/run/wireguard/$interface.sock" 2>/dev/null || echo 200) - $(stat -f %m "/var/run/wireguard/$INTERFACE.name" 2>/dev/null || echo 100) ))
	[[ $diff -ge 2 || $diff -le -2 ]] && return 1
	REAL_INTERFACE="$interface"
	echo "[+] Interface for $INTERFACE is $REAL_INTERFACE" >&2
	return 0
}

add_if() {
	export WG_TUN_NAME_FILE="/var/run/wireguard/$INTERFACE.name"
	mkdir -p "/var/run/wireguard/"
	cmd "${WG_QUICK_USERSPACE_IMPLEMENTATION:-wireguard-go}" utun
	get_real_interface
}

del_routes() {
	[[ -n $REAL_INTERFACE ]] || return 0
	local todelete=( ) destination gateway netif
	while read -r destination _ _ _ _ netif _; do
		[[ $netif == "$REAL_INTERFACE" ]] && todelete+=( "$destination" )
	done < <(netstat -nr -f inet)
	for destination in "${todelete[@]}"; do
		cmd route -q -n delete -inet "$destination" >/dev/null || true
	done
	todelete=( )
	while read -r destination gateway _ netif; do
		[[ $netif == "$REAL_INTERFACE" || ( $netif == lo* && $gateway == "$REAL_INTERFACE" ) ]] && todelete+=( "$destination" )
	done < <(netstat -nr -f inet6)
	for destination in "${todelete[@]}"; do
		cmd route -q -n delete -inet6 "$destination" >/dev/null || true
	done
	for destination in "${ENDPOINTS[@]}"; do
		if [[ $destination == *:* ]]; then
			cmd route -q -n delete -inet6 "$destination" >/dev/null || true
		else
			cmd route -q -n delete -inet "$destination" >/dev/null || true
		fi
	done
}

del_if() {
	[[ -z $REAL_INTERFACE ]] || cmd rm -f "/var/run/wireguard/$REAL_INTERFACE.sock"
	cmd rm -f "/var/run/wireguard/$INTERFACE.name"
}

up_if() {
	cmd ifconfig "$REAL_INTERFACE" up
}

add_addr() {
	if [[ $1 == *:* ]]; then
		cmd ifconfig "$REAL_INTERFACE" inet6 "$1" alias
	else
		cmd ifconfig "$REAL_INTERFACE" inet "$1" "${1%%/*}" alias
	fi
}

set_mtu() {
	# TODO: use better set_mtu algorithm from freebsd.bash
	local mtu=0 current_mtu=-1 destination netif defaultif
	if [[ -n $MTU ]]; then
		cmd ifconfig "$REAL_INTERFACE" mtu "$MTU"
		return
	fi
	while read -r destination _ _ _ _ netif _; do
		if [[ $destination == default ]]; then
			defaultif="$netif"
			break
		fi
	done < <(netstat -nr -f inet)
	[[ -n $defaultif && $(ifconfig "$defaultif") =~ mtu\ ([0-9]+) ]] && mtu="${BASH_REMATCH[1]}"
	[[ $mtu -gt 0 ]] || mtu=1500
	mtu=$(( mtu - 80 ))
	[[ $(ifconfig "$REAL_INTERFACE") =~ mtu\ ([0-9]+) ]] && current_mtu="${BASH_REMATCH[1]}"
	[[ $mtu -eq $current_mtu ]] || cmd ifconfig "$REAL_INTERFACE" mtu "$mtu"
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
	done < <(wg show "$REAL_INTERFACE" endpoints)
}

declare -A SERVICE_DNS
declare -A SERVICE_DNS_SEARCH
collect_new_service_dns() {
	local service get_response
	local -A found_services
	{ read -r _ && while read -r service; do
		[[ $service == "*"* ]] && service="${service:1}"
		found_services["$service"]=1
		[[ -n ${SERVICE_DNS["$service"]} ]] && continue
		get_response="$(cmd networksetup -getdnsservers "$service")"
		[[ $get_response == *" "* ]] && get_response="Empty"
		[[ -n $get_response ]] && SERVICE_DNS["$service"]="$get_response"
		get_response="$(cmd networksetup -getsearchdomains "$service")"
		[[ $get_response == *" "* ]] && get_response="Empty"
		[[ -n $get_response ]] && SERVICE_DNS_SEARCH["$service"]="$get_response"
	done; } < <(networksetup -listallnetworkservices)

	for service in "${!SERVICE_DNS[@]}"; do
		if ! [[ -n ${found_services["$service"]} ]]; then
			unset SERVICE_DNS["$service"]
			unset SERVICE_DNS_SEARCH["$service"]
		fi
	done
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
			cmd route -q -n delete -inet6 "$endpoint" >/dev/null 2>&1 || true
		elif [[ $AUTO_ROUTE4 -eq 1 ]]; then
			cmd route -q -n delete -inet "$endpoint" >/dev/null 2>&1 || true
		fi
	done

	for endpoint in "${ENDPOINTS[@]}"; do
		if [[ $remove_all_old -eq 0 && " ${old_endpoints[*]} " == *" $endpoint "* ]]; then
			added+=( "$endpoint" )
			continue
		fi
		if [[ $endpoint == *:* && $AUTO_ROUTE6 -eq 1 ]]; then
			if [[ -n $GATEWAY6 ]]; then
				cmd route -q -n add -inet6 "$endpoint" -gateway "$GATEWAY6" >/dev/null || true
			else
				# Prevent routing loop
				cmd route -q -n add -inet6 "$endpoint" ::1 -blackhole >/dev/null || true
			fi
			added+=( "$endpoint" )
		elif [[ $AUTO_ROUTE4 -eq 1 ]]; then
			if [[ -n $GATEWAY4 ]]; then
				cmd route -q -n add -inet "$endpoint" -gateway "$GATEWAY4" >/dev/null || true
			else
				# Prevent routing loop
				cmd route -q -n add -inet "$endpoint" 127.0.0.1 -blackhole >/dev/null || true
			fi
			added+=( "$endpoint" )
		fi
	done
	ENDPOINTS=( "${added[@]}" )
}

set_dns() {
	collect_new_service_dns
	local service response
	for service in "${!SERVICE_DNS[@]}"; do
		while read -r response; do
			[[ $response == *Error* ]] && echo "$response" >&2
		done < <(
			cmd networksetup -setdnsservers "$service" "${DNS[@]}"
			if [[ ${#DNS_SEARCH[@]} -eq 0 ]]; then
				cmd networksetup -setsearchdomains "$service" Empty
			else
				cmd networksetup -setsearchdomains "$service" "${DNS_SEARCH[@]}"
			fi
		)
	done
}

del_dns() {
	local service response
	for service in "${!SERVICE_DNS[@]}"; do
		while read -r response; do
			[[ $response == *Error* ]] && echo "$response" >&2
		done < <(
			cmd networksetup -setdnsservers "$service" ${SERVICE_DNS["$service"]} || true
			cmd networksetup -setsearchdomains "$service" ${SERVICE_DNS_SEARCH["$service"]} || true
		)
	done
}

monitor_daemon() {
	echo "[+] Backgrounding route monitor" >&2
	(trap 'del_routes; del_dns; exit 0' INT TERM EXIT
	exec >/dev/null 2>&1
	local event pid=$BASHPID
	[[ ${#DNS[@]} -gt 0 ]] && trap set_dns ALRM
	# TODO: this should also check to see if the endpoint actually changes
	# in response to incoming packets, and then call set_endpoint_direct_route
	# then too. That function should be able to gracefully cleanup if the
	# endpoints change.
	while read -r event; do
		[[ $event == RTM_* ]] || continue
		ifconfig "$REAL_INTERFACE" >/dev/null 2>&1 || break
		[[ $AUTO_ROUTE4 -eq 1 || $AUTO_ROUTE6 -eq 1 ]] && set_endpoint_direct_route
		[[ -z $MTU ]] && set_mtu
		if [[ ${#DNS[@]} -gt 0 ]]; then
			set_dns
			sleep 2 && kill -ALRM $pid 2>/dev/null &
		fi
	done < <(route -n monitor)) &
	[[ -n $LAUNCHED_BY_LAUNCHD ]] || disown
}

add_route() {
	[[ $TABLE != off ]] || return 0

	local family=inet
	[[ $1 == *:* ]] && family=inet6

	if [[ $1 == */0 && ( -z $TABLE || $TABLE == auto ) ]]; then
		if [[ $1 == *:* ]]; then
			AUTO_ROUTE6=1
			cmd route -q -n add -inet6 ::/1 -interface "$REAL_INTERFACE" >/dev/null
			cmd route -q -n add -inet6 8000::/1 -interface "$REAL_INTERFACE" >/dev/null
		else
			AUTO_ROUTE4=1
			cmd route -q -n add -inet 0.0.0.0/1 -interface "$REAL_INTERFACE" >/dev/null
			cmd route -q -n add -inet 128.0.0.0/1 -interface "$REAL_INTERFACE" >/dev/null
		fi
	else
		[[ $TABLE == main || $TABLE == auto || -z $TABLE ]] || die "Darwin only supports TABLE=auto|main|off"
		[[ $(route -n get "-$family" "$1" 2>/dev/null) =~ interface:\ $REAL_INTERFACE$'\n' ]] || cmd route -q -n add -$family "$1" -interface "$REAL_INTERFACE" >/dev/null

	fi
}

set_config() {
	cmd wg setconf "$REAL_INTERFACE" <(echo "$WG_CONFIG")
}

save_config() {
	local old_umask new_config current_config address cmd
	new_config=$'[Interface]\n'
	while read -r address; do
		[[ $address =~ inet6?\ ([^ ]+) ]] && new_config+="Address = ${BASH_REMATCH[1]}"$'\n'
	done < <(ifconfig "$REAL_INTERFACE")
	# TODO: actually determine current DNS for interface
	for address in "${DNS[@]}"; do
		new_config+="DNS = $address"$'\n'
	done
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
	current_config="$(cmd wg showconf "$REAL_INTERFACE")"
	trap 'rm -f "$CONFIG_FILE.tmp"; exit' INT TERM EXIT
	echo "${current_config/\[Interface\]$'\n'/$new_config}" > "$CONFIG_FILE.tmp" || die "Could not write configuration file"
	sync "$CONFIG_FILE.tmp"
	mv "$CONFIG_FILE.tmp" "$CONFIG_FILE" || die "Could not move configuration file"
	trap - INT TERM EXIT
	umask "$old_umask"
}

execute_hooks() {
	local hook
	for hook in "$@"; do
		hook="${hook//%i/$REAL_INTERFACE}"
		hook="${hook//%I/$INTERFACE}"
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
	    are added. Besides \`auto' and \`off', only \`main' is supported on
	    this platform.
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
	get_real_interface && die "\`$INTERFACE' already exists as \`$REAL_INTERFACE'"
	trap 'del_if; del_routes; exit' INT TERM EXIT
	execute_hooks "${PRE_UP[@]}"
	add_if
	set_config
	for i in "${ADDRESSES[@]}"; do
		add_addr "$i"
	done
	set_mtu
	up_if
	for i in $(while read -r _ i; do for i in $i; do [[ $i =~ ^[0-9a-z:.]+/[0-9]+$ ]] && echo "$i"; done; done < <(wg show "$REAL_INTERFACE" allowed-ips) | sort -nr -k 2 -t /); do
		add_route "$i"
	done
	[[ $AUTO_ROUTE4 -eq 1 || $AUTO_ROUTE6 -eq 1 ]] && set_endpoint_direct_route
	[[ ${#DNS[@]} -gt 0 ]] && set_dns
	monitor_daemon
	execute_hooks "${POST_UP[@]}"
	trap - INT TERM EXIT
}

cmd_down() {
	if ! get_real_interface || [[ " $(wg show interfaces) " != *" $REAL_INTERFACE "* ]]; then
		die "\`$INTERFACE' is not a WireGuard interface"
	fi
	execute_hooks "${PRE_DOWN[@]}"
	[[ $SAVE_CONFIG -eq 0 ]] || save_config
	del_if
	execute_hooks "${POST_DOWN[@]}"
}

cmd_save() {
	if ! get_real_interface || [[ " $(wg show interfaces) " != *" $REAL_INTERFACE "* ]]; then
		die "\`$INTERFACE' is not a WireGuard interface"
	fi
	save_config
}

cmd_strip() {
	echo "$WG_CONFIG"
}

# ~~ function override insertion point ~~

if [[ $# -eq 1 && ( $1 == --help || $1 == -h || $1 == help ) ]]; then
	cmd_usage
elif [[ $# -eq 2 && $1 == up ]]; then
	auto_su
	detect_launchd
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

[[ -n $LAUNCHED_BY_LAUNCHD ]] && wait

exit 0
