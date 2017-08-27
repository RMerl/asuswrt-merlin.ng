#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <shared.h>
#include <rc.h>
#include <time.h>
#include <sys/time.h>

#define NORMAL_PERIOD           1               /* second */
#define DELAY_0			0		/* No flash */
#define DELAY_1S		1000000		/* 1 second */
#define DELAY_0_5S		500000		/* 0.5 second */
#define DELAY_0_25S		250000		/* 0.25 second */
#define DELAY_0_2S		200000		/* 0.2 second */
#define DELAY_0_1S		100000		/* 0.2 second */

#ifdef RPAC87
#define link_quality_level1  0
#define link_quality_level2  20
#define link_quality_level3  40
#define link_quality_level4  70
#endif

#ifdef RPAC66
#define link_quality_level1  0
#define link_quality_level2  70
#define link_quality_level3  80
#endif

#ifdef RPAC53
#define link_quality_level1  0
#define link_quality_level2  30
#define link_quality_level3  60
#endif

#if defined(RPAC51) || defined(RPAC55)
#define link_quality_level1  0
#define link_quality_level2  35 // RSSI: -65
#define link_quality_level3  80 // RSSI: -20
#endif

static enum LED_STATUS status;
static enum LED_STATUS pre_status;

#if defined(RPAC53) /* RP-AC53 */
#define LEDS_COUNT 4
led_state_t leds[LEDS_COUNT] = {
	[0] = {
		.id = LED_POWER
	},
	[1] = {
		.id = LED_2G
	},
	[2] = {
		.id = LED_5G
	},
	[3] = {
		.id = LED_LAN
	},
};
#elif defined(RPAC55)
#define LEDS_COUNT 4
led_state_t leds[LEDS_COUNT] = {
	[0] = {
		.id = LED_POWER
	},
	[1] = {
		.id = LED_WIFI
	},
	[2] = {
		.id = LED_SIG1
	},
	[3] = {
		.id = LED_SIG2
	},
};
#elif defined(RPAC51)
#define LEDS_COUNT 4
led_state_t leds[LEDS_COUNT] = {
	[0] = {
		.id = LED_POWER
	},
	[1] = {
		.id = LED_SINGLE
	},
	[2] = {
		.id = LED_FAR
	},
	[3] = {
		.id = LED_NEAR
	},
};
#else /* RP-AC66 */
#define LEDS_COUNT 3
led_state_t leds[LEDS_COUNT] = {
	[0] = {
		.id = LED_POWER
	},
	[1] = {
		.id = LED_2G
	},
	[2] = {
		.id = LED_5G
	},
};
#endif

static void update_led(led_state_t *led, int color, unsigned long flash_interval);

static struct itimerval itv;

static int sw_mode;

static void alarmtimer(unsigned long sec, unsigned long usec)
{
	itv.it_value.tv_sec  = sec;
        itv.it_value.tv_usec = usec;
        itv.it_interval = itv.it_value;
        setitimer(ITIMER_REAL, &itv, NULL);
}

static unsigned long get_us_timer()
{
	return itv.it_value.tv_usec + itv.it_value.tv_sec * 1000000;
}

static void update_interval_timer()
{
	int i = 0;
	int update_flag = 0;
	int no_flash_flag = 1;

	while (i < LEDS_COUNT){
		if (leds[i].flash_interval > 0 && leds[i].flash_interval < get_us_timer()) {
			itv.it_value.tv_sec  = leds[i].flash_interval / 1000000;
		        itv.it_value.tv_usec = leds[i].flash_interval % 1000000;
			update_flag = 1;
		}
		else if (leds[i].flash_interval > 0)
			no_flash_flag = 0;
		i++;
	}

	if (no_flash_flag) {
		alarmtimer(NORMAL_PERIOD, 0);
	}
	else if (update_flag) {
		itv.it_interval = itv.it_value;
		setitimer(ITIMER_REAL, &itv, NULL);
	}
}

static void led_flash(led_state_t *led)
{
	if (led->next_switch_time <= get_us_timer()) {
		if (led->state == LED_ON) { // ON
			set_off_led(led);
		}
		else {	//OFF
			set_on_led(led);
		}
		led->next_switch_time = led->flash_interval;
	}
	else
		led->next_switch_time -= get_us_timer();
}

enum WiFi_QUALITY {
	WIFI_2G_HIGH_QUALITY,
	WIFI_2G_MEDIUM_QUALITY,
	WIFI_2G_MEDIUM2_QUALITY,
	WIFI_2G_LOW_QUALITY,
	WIFI_2G_DOWN,
	WIFI_5G_HIGH_QUALITY,
	WIFI_5G_MEDIUM_QUALITY,
	WIFI_5G_MEDIUM2_QUALITY,
	WIFI_5G_LOW_QUALITY,
	WIFI_5G_DOWN
};

enum {
	GW_LINK_DOWN = 0,
	GW_LINK_UP
};

static void detect_eth_link()
{
	static int p_gw_status = -1;
	static int skip_time = 3000000;
	/* Stop detect conn link */
	if (sw_mode != SW_MODE_AP
#ifdef RTCONFIG_REALTEK
		|| mediabridge_mode()
#endif
	) // Only for AP mode
		return;

	if (nvram_get_int("wps_cli_state") == 1) // WPS processing...
		return;

	if (status == LED_BOOTING || status == LED_WPS_PROCESSING || status == LED_WPS_RESTART_WL)
		return;

	if (status == LED_BOOTED_APMODE && skip_time > 0) { // Special case. Wait for eth ready.
		skip_time -= get_us_timer();
		return;
	}
	/* Stop detect coon link end */

	int gw_status = GW_LINK_DOWN;
	if (nvram_get_int("dnsqmode") == 1)
		gw_status = GW_LINK_UP;

	if (gw_status != p_gw_status) {
		if (gw_status == GW_LINK_DOWN) {
#if defined(RPAC53)
			update_led(&leds[3], LED_GREEN, DELAY_0_5S);
			update_gpiomode(14, 1);
#elif defined(RPAC51) || defined(RPAC55)
			update_led(&leds[1], LED_NONE, DELAY_0);
#else
			update_led(&leds[0], LED_GREEN, DELAY_0_5S);
#endif
		}
		else {
#if defined(RPAC53)
			update_led(&leds[3], LED_GREEN, DELAY_0);
			update_gpiomode(14, 0);
#elif defined(RPAC51) || defined(RPAC55)
			update_led(&leds[1], LED_BLUE, DELAY_0);
#else
			update_led(&leds[0], LED_GREEN, DELAY_0);
#endif
		}
		p_gw_status = gw_status;
	}
}

static void detect_conn_link_quality()
{
	/* Stop detect conn link */
	if (sw_mode != SW_MODE_REPEATER
#ifdef RTCONFIG_REALTEK
		&& !mediabridge_mode()
#endif
	   ) // Only for repeater mode
		return;

	static int link_quality_2g = WIFI_2G_DOWN, link_quality_5g = WIFI_5G_DOWN;

	if (nvram_get_int("wps_cli_state") == 1 || nvram_get_int("restore_defaults") == 1) {// WPS processing and Restore defaults
		link_quality_2g = WIFI_2G_DOWN;
		link_quality_5g = WIFI_5G_DOWN;
		return;
	}

	if (status == LED_BOOTING || status == LED_WPS_START || status == LED_WPS_PROCESSING || status == LED_WPS_RESTART_WL || status == LED_RESTART_WL || status == LED_FIRMWARE_UPGRADE) {
		link_quality_2g = WIFI_2G_DOWN;
		link_quality_5g = WIFI_5G_DOWN;
		return;
	}
	/* Stop detect coon link end */

	int link_quality = 0;

	if (nvram_get_int("wlc0_state") == WLC_STATE_CONNECTED) {
		link_quality = get_conn_link_quality(0);
#ifdef RPAC87
		if (link_quality >= link_quality_level4) {
			if (link_quality_2g != WIFI_2G_HIGH_QUALITY) {
				update_led(&leds[1], LED_SL4, DELAY_0);
				link_quality_2g = WIFI_2G_HIGH_QUALITY;
			}
		}
		else if	(link_quality >= link_quality_level3)
#else
		if (link_quality >= link_quality_level3)
#endif
		{
			if (link_quality_2g != WIFI_2G_MEDIUM2_QUALITY) {
#ifdef RPAC87
				update_led(&leds[1], LED_SL3, DELAY_0);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[1], LED_GREEN, DELAY_0);
#endif

				link_quality_2g = WIFI_2G_MEDIUM2_QUALITY;

			}
		}
		else if (link_quality >= link_quality_level2) {
			if (link_quality_2g != WIFI_2G_MEDIUM_QUALITY) {
#ifdef RPAC87
				update_led(&leds[1], LED_SL2, DELAY_0);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[1], LED_ORANGE, DELAY_0);
#endif
				link_quality_2g = WIFI_2G_MEDIUM_QUALITY;
			}
		}
		else if (link_quality > link_quality_level1) {
			if (link_quality_2g != WIFI_2G_LOW_QUALITY) {
#ifdef RPAC87
				update_led(&leds[1], LED_SL1, DELAY_0);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[1], LED_RED, DELAY_0);
#endif
				link_quality_2g = WIFI_2G_LOW_QUALITY;
			}
		}
		else {
			if (link_quality_2g != WIFI_2G_DOWN) {
				update_led(&leds[1], LED_NONE, DELAY_0);
				link_quality_2g = WIFI_2G_DOWN;
			}
		}
	}
	else {
		if (link_quality_2g != WIFI_2G_DOWN) {
			update_led(&leds[1], LED_NONE, DELAY_0);
			link_quality_2g = WIFI_2G_DOWN;
		}
	}

	if (nvram_get_int("wlc1_state") == WLC_STATE_CONNECTED) {
		link_quality = get_conn_link_quality(1);
#ifdef RPAC87
		if (link_quality >= link_quality_level4) {
			if (link_quality_5g != WIFI_5G_HIGH_QUALITY) {
				update_led(&leds[2], LED_SL4, DELAY_0);
				link_quality_5g = WIFI_5G_HIGH_QUALITY;
			}
		}
		else if (link_quality >= link_quality_level3)
#else
		if (link_quality >= link_quality_level3)
#endif
		{
			if (link_quality_5g != WIFI_5G_MEDIUM2_QUALITY) {
#if defined(RPAC87) || defined(RPAC53)
				update_led(&leds[2], LED_SL3, DELAY_0);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[2], LED_GREEN, DELAY_0);
#endif
				link_quality_5g = WIFI_5G_MEDIUM2_QUALITY;
			}
		}
		else if (link_quality >= link_quality_level2) {
			if (link_quality_5g != WIFI_5G_MEDIUM_QUALITY) {
#ifdef RPAC87
				update_led(&leds[2], LED_SL2, DELAY_0);

#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[2], LED_ORANGE, DELAY_0);
#endif
				link_quality_5g = WIFI_5G_MEDIUM_QUALITY;
			}
		}
		else if (link_quality > link_quality_level1) {
			if (link_quality_5g != WIFI_5G_LOW_QUALITY) {
#ifdef RPAC87
				update_led(&leds[2], LED_SL1, DELAY_0);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[2], LED_RED, DELAY_0);
#endif
				link_quality_5g = WIFI_5G_LOW_QUALITY;
			}
		}
		else {
			if (link_quality_5g != WIFI_5G_DOWN) {
				update_led(&leds[2], LED_NONE, DELAY_0);
				link_quality_5g = WIFI_5G_DOWN;
			}
		}
	}
	else {
		if (link_quality_5g != WIFI_5G_DOWN) {
			update_led(&leds[2], LED_NONE, DELAY_0);
			link_quality_5g = WIFI_5G_DOWN;
		}
	}
}

#if (defined(RPAC51) || defined(RPAC55)) && defined(RTCONFIG_WLCSCAN_RSSI)
#define DISTANCE_NORMAL 0
#define DISTANCE_TOO_CLOSE 1
#define DISTANCE_TOO_FAR 2

/**
 * A clock.
 * @param  start [1: timer start. Others: decide time stop or not]
 * @param  secs  [Judgment time value]
 * @return       [0: non time out. 1: time out.]
 */
static int time_stop(int start, int secs)
{
	static time_t start_time = 0;
	if (start) {
		start_time = time(NULL);
	}
	else {
		if (start_time == 0)
			return 1; /* Not be here.*/

		time_t now = time(NULL);
		if (difftime(now, start_time) >= secs)
		{
			start_time = 0;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

/**
 * Detect does repeater connect to P-AP.
 */
static int detect_conn_link_internet()
{
	static int pre_link_internet = 0;

	if (pre_link_internet != nvram_get_int("link_internet")) {
		pre_link_internet = nvram_get_int("link_internet");

		if (pre_link_internet == 2) { // Connected.
			update_led(&leds[1], LED_BLUE, DELAY_0);
			return 1;
		}
		else
			update_led(&leds[1], LED_NONE, DELAY_0);
	}
	else {
		if (pre_link_internet == 2)
			return 1;
	}
	return 0;
}

/**
 * Select a band for detecting signal by signal.
 * @return The band is be selected. 0: 2.4G. 1: 5G. -1: No result.
 */
static int select_detect_band()
{

	static int signal_2g = 0;
	static int signal_5g = 0;
	static int done_2g = 0, done_5g = 0;
	char tmp[64] = {0}, tmp2[16] = {0};

	if (nvram_get_int("wlcscan_ssid_rssi_state") == 0) {
		if (!done_2g) {
			snprintf(tmp2, sizeof(tmp2), "wlc0_ssid");
			snprintf(tmp, sizeof(tmp), "wlcscan_ssid_rssi 0 %s &", nvram_safe_get(tmp2));
			system(tmp);
		}
		else {
			snprintf(tmp2, sizeof(tmp2), "wlc1_ssid");
			snprintf(tmp, sizeof(tmp), "wlcscan_ssid_rssi 1 %s &", nvram_safe_get(tmp2));
			system(tmp);
		}
	}
	else if (nvram_get_int("wlcscan_ssid_rssi_state") == 2) { // Finished
		nvram_set_int("wlcscan_ssid_rssi_state", 0);
		if (!done_2g) {
			snprintf(tmp, sizeof(tmp), "wlc0_scan_rssi");
			signal_2g = nvram_get_int(tmp);
			done_2g = 1;
		}
		else {
			snprintf(tmp, sizeof(tmp), "wlc1_scan_rssi");
			signal_5g = nvram_get_int(tmp);
			done_5g = 1;
		}
	}
	else if (nvram_get_int("wlcscan_ssid_rssi_state") == -1) { // Finished, But error.
		nvram_set_int("wlcscan_ssid_rssi_state", 0);
		if (!done_2g) {
			signal_2g = 0;
			done_2g = 1;
		}
		else {
			signal_5g = 0;
			done_5g = 1;
		}
	}

	if (done_2g && done_5g) {
		if (signal_5g >= signal_2g)
			return 1;
		else
			return 0;
	}
	return -1;
}

/**
 * Detcet P-AP and router distance. Turn on/off indicator light.
 */
static void detect_conn_link_zone()
{

	if (nvram_get_int("wps_cli_state") == 1 || nvram_get_int("restore_defaults") == 1) // WPS processing and Restore defaults
		return;

	if (status == LED_BOOTING || status == LED_WPS_PROCESSING || status == LED_WPS_START || status == LED_WPS_SCANNING
	 || status == LED_WPS_2G_SCANNING || status == LED_WPS_5G_SCANNING
	 || status == LED_WPS_RESTART_WL || status == LED_RESTART_WL || status == LED_FIRMWARE_UPGRADE)
		return;

	/* Stop detect conn link */
	if (sw_mode != SW_MODE_REPEATER
#ifdef RTCONFIG_REALTEK
	&& !mediabridge_mode()
#endif
	   ) {// Only for repeater mode
		return;
	}

	static int pre_link_status = -1;
	int link_quality = 0;
	static int band = -1;

	char tmp[64] = {0}, tmp2[16] = {0};

	if (detect_conn_link_internet()) {

		if (nvram_get_int("wlc_band") == 0) // 2.4G connected
			link_quality = get_conn_link_quality(0);
		else if (nvram_get_int("wlc_band") == 1) // 5G connected
			link_quality = get_conn_link_quality(1);
		else
			return;
			/* Do nothing... */
	}
	else {
		if (nvram_get_int("wlc_express") == 0) { // Mediabridge mode. Repeater mode.
			if (band == -1) {
				band = select_detect_band();
				return;				
			}
		}
		else if (nvram_get_int("wlc_express") == 1) { // Express 2G
			band = 0;
		}
		else { // Express 5G
			band = 1;
		}
		if (nvram_get_int("wlcscan_ssid_rssi_state") == 0) {
			snprintf(tmp2, sizeof(tmp2), "wlc%d_ssid", band);
			snprintf(tmp, sizeof(tmp), "wlcscan_ssid_rssi %d %s &", band, nvram_safe_get(tmp2));
			system(tmp);
		}
		else if (nvram_get_int("wlcscan_ssid_rssi_state") == 2) { // Finished
			nvram_set_int("wlcscan_ssid_rssi_state", 0);
		}
		else if (nvram_get_int("wlcscan_ssid_rssi_state") == -1) { // Finished, But error.
			nvram_set_int("wlcscan_ssid_rssi_state", 0);
		}
		snprintf(tmp, sizeof(tmp), "wlc%d_scan_rssi", band);
		link_quality = nvram_get_int(tmp);
	}
	if (link_quality >= link_quality_level3) { // Too close.
		if (pre_link_status == DISTANCE_TOO_CLOSE) {
			if (time_stop(0, 30)) {
				update_led(&leds[2], LED_NONE, DELAY_0);
				update_led(&leds[3], LED_BLUE, DELAY_0);
				return;
			}
			else
				return;
		}
		else {
			time_stop(1, 30);
			pre_link_status = DISTANCE_TOO_CLOSE;
		}

		update_led(&leds[2], LED_NONE, DELAY_0);
		update_led(&leds[3], LED_BLUE, DELAY_0_5S);
		update_interval_timer();
	}
	else if (link_quality >= link_quality_level2) { // Normal
		if (pre_link_status == DISTANCE_NORMAL)
			return; // Don't do anything.
		else
			pre_link_status = DISTANCE_NORMAL;

		update_led(&leds[2], LED_NONE, DELAY_0);
		update_led(&leds[3], LED_NONE, DELAY_0);
	}
	else if (link_quality > link_quality_level1) { // Too far.
		if (pre_link_status == DISTANCE_TOO_FAR) {
			if (time_stop(0, 30)) {
				update_led(&leds[2], LED_BLUE, DELAY_0);
				update_led(&leds[3], LED_NONE, DELAY_0);
				return;
			}
			else
				return;
		}
		else {
			time_stop(1, 30);
			pre_link_status = DISTANCE_TOO_FAR;
		}

		update_led(&leds[2], LED_BLUE, DELAY_0);
		update_led(&leds[3], LED_NONE, DELAY_0_5S);
		update_interval_timer();
	}
	else { // Disconnected.
		if (pre_link_status == -1)
			return; // Don't do anything.
		else
			pre_link_status = -1;

		update_led(&leds[2], LED_NONE, DELAY_0);
		update_led(&leds[3], LED_NONE, DELAY_0);
	}
}
#endif

static void process_leds()
{
	int count = 0;
	while (count < LEDS_COUNT) {

		if (leds[count].changed) {

			/* Set off the LED's GPIO */
			set_off_led(&leds[count]);

			/* Set On the LED's GPIO */

			if (leds[count].flash_interval == 0 && leds[count].color != LED_NONE) //ON
				set_on_led(&leds[count]);

			leds[count].changed = 0;
		}

		if (leds[count].flash_interval > 0)
			led_flash(&leds[count]);
		count++;
	}
}

static void process_status(int sig)
{

	status = nvram_get_int("led_status");

	if (status != pre_status) {
		switch (status) {

			case LED_BOOTING:
#ifdef RPAC87
				update_led(&leds[0], LED_GREEN, DELAY_1S);
				update_led(&leds[1], ALL_LED, DELAY_1S);
				update_led(&leds[2], ALL_LED, DELAY_1S);
#endif
#ifdef RPAC66
				update_led(&leds[0], LED_GREEN, DELAY_1S);
				update_led(&leds[1], LED_GREEN, DELAY_1S);
				update_led(&leds[2], LED_GREEN, DELAY_1S);
#endif
#ifdef RPAC53
				update_led(&leds[0], LED_GREEN, DELAY_1S);
				update_led(&leds[0], LED_GREEN, DELAY_1S);
				update_led(&leds[1], LED_GREEN, DELAY_1S);
				update_led(&leds[2], LED_GREEN, DELAY_1S);
				update_led(&leds[3], LED_GREEN, DELAY_1S);
#endif
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_RED, DELAY_0);
#endif	
				break;

			case LED_BOOTED:
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_BLUE, DELAY_0);
#else
				update_led(&leds[0], LED_GREEN, DELAY_0);
#endif
				update_led(&leds[1], LED_NONE, DELAY_0);
				update_led(&leds[2], LED_NONE, DELAY_0);
#ifdef RPAC53
				update_led(&leds[3], LED_NONE, DELAY_0);
				update_gpiomode(14, 0);
#endif
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[3], LED_NONE, DELAY_0);
#endif				
				break;

			case LED_BOOTED_APMODE:

#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_BLUE, DELAY_0);
				update_led(&leds[1], LED_NONE, DELAY_0);
				update_led(&leds[2], LED_NONE, DELAY_0);
				update_led(&leds[3], LED_NONE, DELAY_0);
#else
				update_led(&leds[0], LED_GREEN, DELAY_0);
#endif			
#if defined(RPAC53)
				update_led(&leds[3], LED_GREEN, DELAY_0);
#endif
				/* check the status of radio for 2G */
#ifdef RPAC87
				update_led(&leds[1], (nvram_get_int("wl0_radio") ? ALL_LED : LED_NONE), DELAY_0);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[1], (nvram_get_int("wl0_radio") ? LED_GREEN : LED_NONE), DELAY_0);
#endif
#ifdef RTCONFIG_HAS_5G
#ifdef RPAC87
				update_led(&leds[2], (nvram_get_int("wl1_radio") ? ALL_LED : LED_NONE), DELAY_0);
#endif
#if defined(RPAC66) || defined(RPAC53)
				/* check the status of radio for 5G */
				update_led(&leds[2], (nvram_get_int("wl1_radio") ? LED_GREEN : LED_NONE), DELAY_0);
#endif
#endif

				break;
			case LED_WIFI_2G_DOWN:
#if defined(RPAC51) || defined(RPAC55)
				// TBD.
#else
				update_led(&leds[1], LED_NONE, DELAY_0);
#endif				
				break;
			case LED_WIFI_5G_DOWN:
#if defined(RPAC51) || defined(RPAC55)
				// TBD.
#else
				update_led(&leds[2], LED_NONE, DELAY_0);
#endif
				break;
			case LED_WPS_START:
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_BLUE, DELAY_0_5S);
				update_led(&leds[1], LED_NONE, DELAY_0);
#else			
				update_led(&leds[0], LED_GREEN, DELAY_0_5S);
#endif			
#ifdef RPAC87			
				update_led(&leds[1], ALL_LED, DELAY_0_5S);
				update_led(&leds[2], ALL_LED, DELAY_0_5S);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[1], LED_GREEN, DELAY_0_5S);
				update_led(&leds[2], LED_GREEN, DELAY_0_5S);
#endif
				break;
			case LED_WPS_SCANNING:
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_BLUE, DELAY_0_5S);
				update_led(&leds[1], LED_NONE, DELAY_0);			
#else			
				update_led(&leds[0], LED_GREEN, DELAY_0_5S);			
#endif			
#ifdef RPAC87			
				update_led(&leds[1], ALL_LED, DELAY_0_5S);
				update_led(&leds[2], ALL_LED, DELAY_0_5S);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[1], LED_GREEN, DELAY_0_5S);
				update_led(&leds[2], LED_GREEN, DELAY_0_5S);
#endif
				break;
			case LED_WPS_2G_SCANNING:
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_BLUE, DELAY_0_5S);
				update_led(&leds[1], LED_NONE, DELAY_0);
#else			
				update_led(&leds[0], LED_GREEN, DELAY_0_5S);
#endif				
#ifdef RPAC87			
				update_led(&leds[1], ALL_LED, DELAY_0_5S);
				update_led(&leds[2], ALL_LED, DELAY_0_5S);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[1], LED_GREEN, DELAY_0_5S);
				update_led(&leds[2], LED_GREEN, DELAY_0_5S);
#endif
				break;
			case LED_WPS_5G_SCANNING:
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_BLUE, DELAY_0_5S);
				update_led(&leds[1], LED_NONE, DELAY_0);
#else			
				update_led(&leds[0], LED_GREEN, DELAY_0_5S);
#endif					
#ifdef RPAC87			
				update_led(&leds[1], ALL_LED, DELAY_0_5S);
				update_led(&leds[2], ALL_LED, DELAY_0_5S);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[1], LED_GREEN, DELAY_0_5S);
				update_led(&leds[2], LED_GREEN, DELAY_0_5S);
#endif
				break;
			case LED_WPS_FAIL:

				if (sw_mode == SW_MODE_AP) {
					/* Check gw_status */
					if (nvram_get_int("dnsqmode") == 1) {
#if defined(RPAC53)
						update_led(&leds[3], LED_GREEN, DELAY_0);
						update_gpiomode(14, 0);
#elif defined(RPAC51) || defined(RPAC55)
						update_led(&leds[0], LED_BLUE, DELAY_0);
#else						
						update_led(&leds[0], LED_GREEN, DELAY_0);
#endif
					}
					else {
#if defined(RPAC53)
						update_led(&leds[3], LED_GREEN, DELAY_0_5S);
						update_gpiomode(14, 1);
#elif defined(RPAC51) || defined(RPAC55)
						update_led(&leds[0], LED_BLUE, DELAY_0);
#else						
						update_led(&leds[0], LED_GREEN, DELAY_0_5S);
#endif
					}
#ifdef RPAC87
					update_led(&leds[1], (nvram_get_int("wl0_radio") ? ALL_LED : LED_NONE), DELAY_0);
#endif
#if defined(RPAC66) || defined(RPAC53)
					update_led(&leds[1], LED_GREEN, DELAY_0);
#endif
#ifdef RTCONFIG_HAS_5G
#ifdef RPAC87
					update_led(&leds[2], (nvram_get_int("wl1_radio") ? ALL_LED : LED_NONE), DELAY_0);
#endif
#if defined(RPAC66) || defined(RPAC53)

					update_led(&leds[2], LED_GREEN, DELAY_0);
#endif
#endif
				}
				else {
#if defined(RPAC51) || defined(RPAC55)
					update_led(&leds[0], LED_BLUE, DELAY_0);
#else					
					update_led(&leds[0], LED_GREEN, DELAY_0);
					update_led(&leds[1], LED_NONE, DELAY_0);
					update_led(&leds[2], LED_NONE, DELAY_0);
#endif				
				}

				break;
			case LED_WPS_PROCESSING:
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_BLUE, DELAY_0_5S);
				update_led(&leds[1], LED_NONE, DELAY_0);
#else			
				update_led(&leds[0], LED_GREEN, DELAY_0_2S);
#endif				
#ifdef RPAC87			
				update_led(&leds[1], ALL_LED, DELAY_0_2S);
				update_led(&leds[2], ALL_LED, DELAY_0_2S);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[1], LED_GREEN, DELAY_0_2S);
				update_led(&leds[2], LED_GREEN, DELAY_0_2S);
#endif
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[1], LED_NONE, DELAY_0);
				update_led(&leds[2], LED_NONE, DELAY_0);
				update_led(&leds[3], LED_NONE, DELAY_0);
#endif
				break;
			case LED_WPS_RESTART_WL:
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_BLUE, DELAY_0_5S);
#else			
				update_led(&leds[0], LED_GREEN, DELAY_0_5S);
#endif				
#ifdef RPAC87			
				update_led(&leds[1], ALL_LED, DELAY_0_5S);
				update_led(&leds[2], ALL_LED, DELAY_0_5S);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[1], LED_GREEN, DELAY_0_5S);
				update_led(&leds[2], LED_GREEN, DELAY_0_5S);
#endif
				break;
			case LED_RESTART_WL:
#if defined(RPAC51) || defined(RPAC55)
				// TBD.
#else			
				update_led(&leds[0], LED_GREEN, DELAY_0_5S);
#endif				
#ifdef RPAC87			
				update_led(&leds[1], ALL_LED, DELAY_0_5S);
				update_led(&leds[2], ALL_LED, DELAY_0_5S);
#endif
#if defined(RPAC66) || defined(RPAC53)
				update_led(&leds[1], LED_GREEN, DELAY_0_5S);
				update_led(&leds[2], LED_GREEN, DELAY_0_5S);
#endif
				break;
			case LED_RESTART_WL_DONE:

				if (access_point_mode()) {
					/* Check gw_status */
#if defined(RPAC51) || defined(RPAC55)				
					update_led(&leds[0], LED_BLUE, DELAY_0);
#else					
					if (nvram_get_int("dnsqmode") == 1)
						update_led(&leds[0], LED_GREEN, DELAY_0);
					else
						update_led(&leds[0], LED_GREEN, DELAY_0_5S);
#endif					

					/* check the status of radio for 2G */
#ifdef RPAC87
					update_led(&leds[1], (nvram_get_int("wl0_radio") ? ALL_LED : LED_NONE), DELAY_0);
#endif
#if defined(RPAC66) || defined(RPAC53)
					update_led(&leds[1], (nvram_get_int("wl0_radio") ? LED_GREEN : LED_NONE), DELAY_0);
#endif
#ifdef RTCONFIG_HAS_5G
					/* check the status of radio for 5G */
#ifdef RPAC87
					update_led(&leds[2], (nvram_get_int("wl1_radio") ? ALL_LED : LED_NONE), DELAY_0);
#endif
#if defined(RPAC66) || defined(RPAC53)
					update_led(&leds[2], (nvram_get_int("wl1_radio") ? LED_GREEN : LED_NONE), DELAY_0);
#endif
#endif
				}
				else {
#if defined(RPAC51) || defined(RPAC55)
					update_led(&leds[0], LED_BLUE, DELAY_0);
#else					
					update_led(&leds[0], LED_GREEN, DELAY_0);
					update_led(&leds[1], LED_NONE, DELAY_0);
					update_led(&leds[2], LED_NONE, DELAY_0);
#endif					
				}

				break;
			case LED_FIRMWARE_UPGRADE:

#ifdef RPAC87
				update_led(&leds[0], LED_GREEN, DELAY_1S);
				update_led(&leds[1], ALL_LED, DELAY_1S);
				update_led(&leds[2], ALL_LED, DELAY_1S);
#endif
#ifdef RPAC66
				update_led(&leds[0], LED_GREEN, DELAY_1S);
				update_led(&leds[1], LED_GREEN, DELAY_1S);
				update_led(&leds[2], LED_GREEN, DELAY_1S);
#endif
#if defined(RPAC53)
				update_led(&leds[0], LED_RED, DELAY_1S);
				update_led(&leds[1], LED_NONE, DELAY_0);
				update_led(&leds[2], LED_NONE, DELAY_0);
				update_led(&leds[3], LED_NONE, DELAY_0);
				update_gpiomode(14, 1);
#endif
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_RED, DELAY_0);				
				update_led(&leds[1], LED_NONE, DELAY_0);
				update_led(&leds[2], LED_NONE, DELAY_0);
				update_led(&leds[3], LED_NONE, DELAY_0);
#endif				
				break;
			case LED_FACTORY_RESET:

#ifdef RPAC87
				update_led(&leds[0], LED_GREEN, DELAY_1S);
				update_led(&leds[1], ALL_LED, DELAY_1S);
				update_led(&leds[2], ALL_LED, DELAY_1S);
#endif
#ifdef RPAC66
				update_led(&leds[0], LED_GREEN, DELAY_1S);
				update_led(&leds[1], LED_GREEN, DELAY_1S);
				update_led(&leds[2], LED_GREEN, DELAY_1S);
#endif
#if defined(RPAC53)
				update_led(&leds[0], LED_GREEN, DELAY_1S);
				update_led(&leds[1], LED_GREEN, DELAY_1S);
				update_led(&leds[2], LED_GREEN, DELAY_1S);
				update_led(&leds[3], LED_GREEN, DELAY_1S);
				update_gpiomode(14, 1);
#endif
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_BLUE, DELAY_1S);			
				update_led(&leds[1], LED_NONE, DELAY_0);
				update_led(&leds[2], LED_NONE, DELAY_0);
				update_led(&leds[3], LED_NONE, DELAY_0);
#endif				
				break;
			case LED_AP_WPS_START:
#ifdef RPAC87
				update_led(&leds[0], LED_GREEN, DELAY_0);
				update_led(&leds[1], ALL_LED, DELAY_1S);
				update_led(&leds[2], ALL_LED, DELAY_1S);
#endif
#ifdef RPAC53
				update_led(&leds[0], LED_GREEN, DELAY_0);
				update_led(&leds[1], LED_GREEN, DELAY_0_2S);
				update_led(&leds[2], LED_GREEN, DELAY_0_2S);

#endif
#if defined(RPAC51) || defined(RPAC55)
				update_led(&leds[0], LED_BLUE, DELAY_0_5S);			
#endif
				break;
			default:
				TRACE_PT("status error.\n");
		}
		pre_status = status;

		update_interval_timer();
	}
	process_leds();
#if (defined(RPAC51) || defined(RPAC55)) && defined(RTCONFIG_WLCSCAN_RSSI)
	detect_conn_link_zone();
#else
	detect_conn_link_quality(); // Check and change WiFi LED in Real time mode.
#endif
	detect_eth_link(); // Check and change Power LED in Real time mode.
}

static void led_monitor_exit(int sig)
{
	remove("/var/run/led_monitor.pid");
	exit(0);
}

static void update_led(led_state_t *led, int color, unsigned long flash_interval)
{
	led->color = color;
	led->flash_interval = flash_interval;
	led->next_switch_time = 0;
	led->changed = 1;
}

int
led_monitor_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;

	sw_mode = sw_mode();

	if ((fp=fopen("/var/run/led_monitor.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigaddset(&sigs_to_catch, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);
	signal(SIGTERM, led_monitor_exit);
	signal(SIGALRM, process_status);

	pre_status = -1;

#if (defined(RPAC51) || defined(RPAC55)) && defined(RTCONFIG_WLCSCAN_RSSI)
	nvram_set_int("wlcscan_ssid_rssi_state", 0); // Init wlcscan_ssid_rssi_state
	nvram_set_int("wlc0_scan_rssi", 0); // Init wlc0_scan_rssi
	nvram_set_int("wlc1_scan_rssi", 0); // Init wlc1_scan_rssi
#endif
	alarmtimer(NORMAL_PERIOD, 0);

	while (1)
	{
		pause();
	}

	return 0;
}
