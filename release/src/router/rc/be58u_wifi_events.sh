#!/bin/sh
# Open replacement for ASUS wlceventd: log Wi-Fi client (dis)associations to syslog.
# Invoked by hostapd_cli -a: $1 = interface, $2 = event, $3 = client MAC
IFACE="$1"
EVENT="$2"
MAC="$3"

case "$EVENT" in
	AP-STA-CONNECTED)    logger -t wifi-events "$IFACE: Assoc $MAC" ;;
	AP-STA-DISCONNECTED) logger -t wifi-events "$IFACE: Disassoc $MAC" ;;
esac
exit 0
