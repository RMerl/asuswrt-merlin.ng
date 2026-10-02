#!/bin/sh
# Start one hostapd_cli event listener per hostapd control socket (RT-BE58U).
ACTION=/usr/sbin/be58u_wifi_events.sh
i=0
while [ $i -lt 60 ] && [ -z "$(ls /var/run/hostapd/ 2>/dev/null)" ]; do
	sleep 2; i=$((i+1))
done
killall -9 -q hostapd_cli 2>/dev/null
for sock in /var/run/hostapd/*; do
	[ -S "$sock" ] || continue
	hostapd_cli -B -p /var/run/hostapd -i "$(basename "$sock")" -a "$ACTION"
done

# The transplanted closed wlan LED code only knows WiFi_5/WiFi_6 (RT-BE92U);
# enable the 2.4 GHz activity LED ourselves when the band is up and LEDs are on.
if [ "$(nvram get AllLED)" = "1" ] && [ "$(nvram get wl0_radio)" = "1" ]; then
	echo 255 > /sys/class/leds/WiFi_24/brightness
else
	echo 0 > /sys/class/leds/WiFi_24/brightness
fi
