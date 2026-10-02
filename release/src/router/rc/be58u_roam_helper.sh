#!/bin/sh
# Open replacement for ASUS roamast: suggest a better AP to clients with a weak
# signal using standard 802.11v BSS Transition Management requests.
# Clients decide themselves whether to roam; nothing is disconnected.
THRESHOLD=${ROAM_RSSI:--75}   # dBm
STRIKES=3                     # consecutive weak samples before suggesting
INTERVAL=10                   # seconds between samples
COOLDOWN=300                  # seconds between suggestions to the same client
STATE=/tmp/roam-helper
PIDFILE=/var/run/be58u_roam_helper.pid
mkdir -p $STATE

# single instance: rc may call start_roamast several times during boot
old=$(cat $PIDFILE 2>/dev/null)
[ -n "$old" ] && [ "$old" != "$$" ] && kill -9 "$old" 2>/dev/null
echo $$ > $PIDFILE

while true; do
	for sock in /var/run/hostapd/*; do
		[ -S "$sock" ] || continue
		ifc=$(basename "$sock")
		for mac in $(wl -i "$ifc" assoclist 2>/dev/null | awk '{print $2}'); do
			rssi=$(wl -i "$ifc" rssi "$mac" 2>/dev/null)
			[ -n "$rssi" ] || continue
			f=$STATE/$(echo "$mac" | tr -d ':')
			if [ "$rssi" -lt "$THRESHOLD" ]; then
				n=$(( $(cat "$f.cnt" 2>/dev/null || echo 0) + 1 ))
				echo $n > "$f.cnt"
				last=$(cat "$f.ts" 2>/dev/null || echo 0)
				now=$(date +%s)
				if [ $n -ge $STRIKES ] && [ $((now - last)) -ge $COOLDOWN ]; then
					hostapd_cli -i "$ifc" bss_tm_req "$mac" pref=1 abridged=1 >/dev/null 2>&1
					logger -t roam-helper "$ifc: $mac rssi=$rssi dBm, suggested roaming (802.11v)"
					echo $now > "$f.ts"; echo 0 > "$f.cnt"
				fi
			else
				echo 0 > "$f.cnt"
			fi
		done
	done
	sleep $INTERVAL
done
