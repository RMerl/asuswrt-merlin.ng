#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <shared.h>
#include "lp5523led.h"

/* step of pattern
 *		      .-> step(32) <-.
 *  	|	0  0  0  0  |  0  0  0  0	|	0
 * 	|	0  0  0  0  |  0  0  1  0	|	2
 * 	|	0  0  0  0  |  0  1  0  0	|	4
 * 	|	0  0  0  0  |  0  1  1  0	|	6
 * 	|	0  0  0  0  |  1  0  0  0	|	8
 * 	|	0  0  0  0  |  1  0  1  0	|	a
 * 	|	0  0  0  0  |  1  1  0  0	|	c
 * 	|	0  0  0  0  |  1  1  1  0	|	e
 * 0	|	0  0  0  0  |  0  0  0  0	|	
 * 1	|	0  0  0  1  |  0  0  0  0	|	
 * 2	|	0  0  1  0  |  0  0  0  0	|	
 * 3	|	0  0  1  1  |  0  0  0  0	|	
 *		   v			v
 * 		   unit-time		sign
 *
* step: total 32
 * unit-time: 0- 0.49(ms/step), 1- 15.6(ms/step)
 * sign: 0- increase, 1- discrease 
*/

#if defined(RTCONFIG_LP5523)

#define MAX_STEP 31

static const char lp55xx_path[] = "/sys/class/leds/blue1/device";
static int brightness = 100;
/*
Led array : [BLUE], [GREEN], [RED], NULL
*/
static char *led_arr[]  = { "blue", "green", "red", NULL };
static char *led_patten_arr[]  = {
#if defined(RTAC95U)
				"000001000",
				"000010000",
				"000000001",
#else
				"111000000",
				"000111000",
				"000000111",
#endif
				NULL
			};

// Voltage of LEDS
struct lp55xx_leds_pattern lp55xx_leds_col[] = {
//	{ COLOR_INDEX,				blue	green	red	}
	{ LP55XX_ALL_LEDS_ON,			"0xff", "0xff", "0xff" },
	{ LP55XX_ALL_LEDS_OFF,			"0x00", "0x00", "0x00" },
	{ LP55XX_ALL_BLEDS_ON,			"0xff", "0x00", "0x00" },
	{ LP55XX_ALL_GLEDS_ON,			"0x00", "0xff", "0x00" },
	{ LP55XX_ALL_RLEDS_ON,			"0x00", "0x00", "0xff" },
	{ LP55XX_ALL_BREATH_LEDS,		"0xfa", "0xfa", "0xfa" },
	{ LP55XX_BLACK_LEDS,			"0x00", "0x00", "0x00" },
	{ LP55XX_WHITE_LEDS,			"0xff", "0xff", "0xff" },
	{ LP55XX_RED_LEDS,			"0x00", "0x00", "0xf0" },
	{ LP55XX_LIGHT_CYAN_LEDS,		"0xff", "0xff", "0x1e" },
	{ LP55XX_PURPLE_LEDS,			"0xff", "0x00", "0x5a" },
	{ LP55XX_NIAGARA_BLUE_LEDS,		"0xff", "0x41", "0x0f" },
	{ LP55XX_PALE_DOGWOOD_LEDS,		"0x6e", "0xc3", "0xff" },
	{ LP55XX_PRIMROSE_YELLOW_LEDS,		"0x0a", "0xff", "0xff" },
	{ LP55XX_BLUEGREEN_BREATH,		"0xfa", "0xfa", "0x00" },
#if defined(RTAC95U)
	{ LP55XX_BTCOR_LEDS,			"0xff", "0x1e", "0x00" },	// BLUE
	{ LP55XX_LINKCOR_LEDS,			"0xff", "0xff", "0xaa" },	// WHITE
	{ LP55XX_DISCONNCOR_LDES,		"0x00", "0x00", "0xf0" },	// RED
	{ LP55XX_WPS_SYNC_LEDS,			"0xff", "0x1e", "0x00" },	// BLUE
	{ LP55XX_AMAS_ETH_LINK_LEDS,		"0xff", "0xff", "0xaa" },	// WHITE
	{ LP55XX_AMAS_RE_SYNC_LEDS,		"0xff", "0x1e", "0x00" },	// BLUE
	{ LP55XX_AMAS_CAPAP_LEDS,		"0xff", "0xff", "0xaa" },	// RED
	{ LP55XX_AMAS_REJOIN_LDES,		"0x00", "0xff", "0x1e" },	// GREEN
	{ LP55XX_ORANGE_LEDS,			"0x00", "0x82", "0xff" },
	{ LP55XX_GREENERY_LEDS,			"0x00", "0xff", "0x1e" },
#else
	{ LP55XX_BTCOR_LEDS,			"0xff", "0xff", "0xf0" },	// WHITE
	{ LP55XX_LINKCOR_LEDS,			"0xff", "0xff", "0x1e" },	// BLUE
	{ LP55XX_DISCONNCOR_LDES,		"0x00", "0x00", "0xf0" },	// RED
	{ LP55XX_WPS_SYNC_LEDS,			"0x41", "0xff", "0x32" },	// GREEN
	{ LP55XX_AMAS_ETH_LINK_LEDS,		"0x41", "0xff", "0x32" },	// GREEN
	{ LP55XX_AMAS_RE_SYNC_LEDS,		"0x41", "0xff", "0x32" },	// GREEN
	{ LP55XX_AMAS_CAPAP_LEDS,		"0x00", "0x99", "0xff" },	// ORANGE
	{ LP55XX_AMAS_REJOIN_LDES,		"0x41", "0xff", "0x32" },	// GREEN
	{ LP55XX_ORANGE_LEDS,			"0x00", "0x99", "0xff" },
	{ LP55XX_GREENERY_LEDS,			"0x41", "0xff", "0x32" },
#endif
	{ LP55XX_END_COLOR,			NULL,	NULL, 	NULL }
};

/* Behavior of LEDS
 * SBLINK = on 499.2ms + off 499.2ms
 * 3 ON 1 OFF = on 748.8ms + off 249.6ms
 *
 * */
struct lp55xx_leds_pattern lp55xx_leds_beh[] = {
	{ LP55XX_ACT_NONE,			"",				NULL,			NULL	}, // (300)
	{ LP55XX_ACT_SBLINK,			"7e00420040007e004200",		NULL,			NULL	}, // (301)
	{ LP55XX_ACT_3ON1OFF,			"7e00620040006000",		NULL,			NULL	}, // (302)
	{ LP55XX_ACT_BREATH_UP_00,		"44101ef042001ff04510",		"44101af042001bf04510",	"44101cf042001df04510"	}, // (303)
	{ LP55XX_ACT_BREATH_DOWN_00,		"1ff04510420044101ef0",		NULL,			NULL	}, // (304)
	{ LP55XX_ACT_BREATH_DOWN_01,		"15e6420014e6",			NULL,			NULL	}, // (305)
	{ LP55XX_ACT_BREATH_DOWN_02,		"11a523504400225010a5",		NULL,			NULL	}, // (306)
	{ LP55XX_ACT_BREATH_UP_01,		"44101ed006001fd04510",		"1fd04510060044101ed0",	""	}, // (307)
	{ LP55XX_END_BLINK,			NULL,				NULL, 			NULL 	}
};

void split(char **arr, char *str, char *del) {
	char *s = strtok(str, del);

	while(s != NULL) {
		*arr++ = s;
		s = strtok(NULL, del);
	}
}

void pattern_combine(char *initial, char *pwm, char *behavior, char *output)
{
	char result[CKN_STR256];
	int pwm_val = (int)strtol(pwm, NULL, 16);

	memset(result, '\0', sizeof(result));

	snprintf(result, sizeof(result), "%s%s%x%s", initial, pwm_val<16?"0":"", pwm_val, behavior);
	memcpy(output, result, strlen(result));

	return;
}


/* combine the string and save
 * @index: select engine 1, 2, 3
 * @engine_type: mode, load, leds
 * @value:
 * @return:
*/
void led_current_setting(int index, char *led, char *value) {
	char path[CKN_STR256];
	char result[CKN_STR8];
	int current = (int)strtol(value, NULL, 16);

	memset(path, '\0', sizeof(path));
	memset(result, '\0', sizeof(result));

	current = current*brightness/100;
	snprintf(path, sizeof(path), "%s/leds/%s%d/led_current", lp55xx_path, led, index);
	snprintf(result, sizeof(result), "0x%x", current);
//	_dprintf("\n\n path: %s, value:%s\n", path, value);
	f_write_string( path, result, 0, 0);

	return;
}

/* combine the string and save
 * @index: select engine 1, 2, 3
 * @engine_type: mode, load, leds
 * @value:
 * @return:
*/
void engine_setting(int index, char *action, char *value) {
	char path[CKN_STR256];

	memset(path, '\0', sizeof(path));

	snprintf(path, sizeof(path), "%s/engine%d_%s", lp55xx_path, index, action);
//	_dprintf("\n\n path: %s, value:%s\n", path, value);
	f_write_string( path, value, 0, 0);

	return;
}

void lp55xx_set_pattern_led(int col_mode, int beh_mode)
{
	char *initial[3] = {"9d8040", "9d8046", "9d804c"};
	struct lp55xx_leds_pattern *blnk_leds_col = lp55xx_leds_col;
	struct lp55xx_leds_pattern *blnk_leds_beh = lp55xx_leds_beh;
	char *current[3]={ 0 }, *pattern[3]={ 0 };
	char led_pattern[CKN_STR10];
	char tmp[CKN_STR256];
	int i=0, j=0, set=0;

	for (; blnk_leds_beh->ptn_mode!=LP55XX_END_BLINK; blnk_leds_beh++) {
// Get BLUE/GREEN/RED leds behavior pattern 
		if (beh_mode == blnk_leds_beh->ptn_mode){
			pattern[0] = blnk_leds_beh->ptn1;
			pattern[1] = blnk_leds_beh->ptn2?blnk_leds_beh->ptn2:blnk_leds_beh->ptn1;
			pattern[2] = blnk_leds_beh->ptn3?blnk_leds_beh->ptn3:blnk_leds_beh->ptn1;

// Get BLUE/GREEN/RED leds current from Manual 
			if (col_mode==LP55XX_MANUAL_COL) {
				char lp55xx_lp5523_manual[CKN_STR256];

				brightness = nvram_get_int("lp55xx_lp5523_user_brightness");
				memset(lp55xx_lp5523_manual, '\0', sizeof(lp55xx_lp5523_manual));
				strcpy(lp55xx_lp5523_manual, nvram_safe_get("lp55xx_lp5523_manual"));
				split(current, lp55xx_lp5523_manual, "_");
				set = 1;
			}
			else {
				for (; blnk_leds_col->ptn_mode!=LP55XX_END_COLOR; blnk_leds_col++) {
// Get BLUE/GREEN/RED leds current 
					if (blnk_leds_col->ptn_mode == col_mode){
						current[0] = blnk_leds_col->ptn1;
						current[1] = blnk_leds_col->ptn2;
						current[2] = blnk_leds_col->ptn3;
						set = 1;
						break;
					}
				}
			}
			break;
		}
	}

	if (set) {
		for (i = LP55XX_ENGINE_1; i <= LP55XX_ENGINE_3; i++) {
			memset(led_pattern, '\0', sizeof(led_pattern));
			strncpy(led_pattern, led_patten_arr[i-1], strlen(led_patten_arr[i-1]));

			for (j = 0; j <= strlen(led_pattern); j++) {
				if (led_pattern[j] == '1') {
/* Set BLUE/GREEN/RED leds current */
					led_current_setting((j%3)+1, led_arr[j/3], current[i-1]);
				}
			}
			memset(tmp, '\0', sizeof(tmp));
/* Set Engine pattern */
			if (col_mode == LP55XX_ALL_LEDS_OFF)
				pattern_combine(initial[0], "00", pattern[i-1], tmp);
			else if (col_mode==LP55XX_ALL_BREATH_LEDS)
				pattern_combine(initial[i-1], "00", pattern[i-1], tmp);
			else if (col_mode==LP55XX_BLUEGREEN_BREATH) {
				if (i == LP55XX_ENGINE_1)
					pattern_combine(initial[0], "05", pattern[i-1], tmp);
				else if (i == LP55XX_ENGINE_2)
					pattern_combine(initial[0], "e5", pattern[i-1], tmp);
				else
					pattern_combine(initial[0], "00", pattern[i-1], tmp);
			}
			else
				pattern_combine(initial[0], "fa", pattern[i-1], tmp);

			engine_setting(i, "load", tmp);
			engine_setting(i, "leds", led_pattern);
		}
	}
}

void lp55xx_blink_leds(int col_mode, int beh_mode)
{
	int i;

	for (i = LP55XX_ENGINE_1; i <= LP55XX_ENGINE_3; i++)
		engine_setting(i, "mode", "disabled");

	for (i = LP55XX_ENGINE_1; i<=LP55XX_ENGINE_3; i++)
		engine_setting(i, "mode", "load");

	lp55xx_set_pattern_led(col_mode, beh_mode);

	for (i = LP55XX_ENGINE_1; i<=LP55XX_ENGINE_3; i++)
		engine_setting(i, "mode", "run");
}

void lp55xx_leds_proc(int col_mode, int beh_mode)
{
	int beh_mode_tmp = beh_mode;
	int col_old = nvram_get_int("lp55xx_lp5523_col");
	int beh_old = nvram_get_int("lp55xx_lp5523_beh");

	brightness = 100;

//	_dprintf("\n\n col: %d, beh:%d\n", col_mode, beh_mode) ;
	// Behavior of ACT
	switch (beh_mode_tmp) {
		case LP55XX_PREVIOUS_STATE:
			col_mode = col_old;
			beh_mode_tmp = beh_old;
			break;
		case LP55XX_WPS_TRIG:
			col_mode = col_old;
			beh_mode_tmp = LP55XX_ACT_3ON1OFF;
			break;
		case LP55XX_WPS_SUCCESS:
			col_mode = col_old;
			beh_mode_tmp = LP55XX_ACT_SBLINK;
			break;
		case LP55XX_RESET_TRIG:
			beh_mode_tmp = LP55XX_ACT_NONE;
			break;
		case LP55XX_RESET_SUCCESS:
			beh_mode_tmp = LP55XX_ACT_SBLINK;
			break;
		case LP55XX_WIFI_PARAM_SYNC:
		case LP55XX_WPS_PARAM_SYNC:
			beh_mode_tmp = LP55XX_ACT_3ON1OFF;
			break;
		case LP55XX_SCH_ENABLE:
			col_mode = nvram_get_int("lp55xx_lp5523_sch_col");
			beh_mode_tmp = nvram_get_int("lp55xx_lp5523_sch_beh");
			brightness = nvram_get_int("lp55xx_lp5523_sch_brightness");
			break;
		case LP55XX_MANUAL_BREATH:
			col_mode = LP55XX_ALL_BREATH_LEDS;
			beh_mode_tmp = LP55XX_ACT_BREATH_UP_00;
			break;
		default:
			if (col_mode != LP55XX_MANUAL_COL) {
				nvram_set_int("lp55xx_lp5523_col", col_mode);
				nvram_set_int("lp55xx_lp5523_beh", beh_mode_tmp);
			}
			break;
	}

	// Color
	switch (col_mode) {
		case LP55XX_LINKCOR_LEDS:
		case LP55XX_GREENERY_LEDS:
		case LP55XX_ORANGE_LEDS:
			if (beh_mode_tmp == LP55XX_ACT_NONE ) {
				if (nvram_match("lp55xx_lp5523_user_enable", "1") && !nvram_match("lp55xx_lp5523_sch_enable", "2")) {
					col_mode = nvram_get_int("lp55xx_lp5523_user_col");
					beh_mode_tmp = nvram_get_int("lp55xx_lp5523_user_beh");
					brightness = nvram_get_int("lp55xx_lp5523_user_brightness");
				}
#if defined(RTCONFIG_SW_CTRL_ALLLED)
				else if (nvram_match("AllLED", "0")) {
					col_mode = LP55XX_ALL_LEDS_OFF;
					beh_mode_tmp = LP55XX_ACT_NONE;
				}
#endif
				else if (nvram_get_int("prelink_pap_status") > 5000) {
					brightness = 50;
				}
			}
#if defined(RTAC95U)
			else if (col_mode==LP55XX_GREENERY_LEDS && beh_mode_tmp==LP55XX_ACT_3ON1OFF) {
				beh_mode_tmp = LP55XX_ACT_BREATH_DOWN_00;
			}
#endif
			break;
#if defined(RTAC95U)
		case LP55XX_ALL_BREATH_LEDS: 
			if (beh_mode_tmp == LP55XX_ACT_BREATH_UP_00) {
				col_mode = LP55XX_GREENERY_LEDS; 
				beh_mode_tmp = LP55XX_ACT_BREATH_DOWN_02;
			}
			break;
#endif
		case LP55XX_ALL_LEDS_OFF:
#if defined(RTCONFIG_SW_CTRL_ALLLED)
			if (nvram_match("AllLED", "0")) {
				nvram_set_int("prelink_pap_status", 0);
				return;
			}
#endif
			break;
	}

	// Check schedule
	if (nvram_match("lp55xx_lp5523_sch_enable", "2")) {
		if (beh_mode!=LP55XX_SCH_ENABLE)
			return;
	}

	if (col_mode == LP55XX_ALL_BREATH_LEDS)
		lp55xx_blink_leds(LP55XX_ALL_LEDS_OFF, LP55XX_ACT_NONE);
	lp55xx_blink_leds(col_mode, beh_mode_tmp);

	return;
}

void lp55xx_leds_sch(int start, int end)
{
	char result[128];
	int flag=0;
	int start_day=0, end_day=0;
	int start_time=0, end_time=0;
	int week, hour;
	int len;

	memset(result, '\0', sizeof(result));
	len = 0;

	for (week=0;week<7;week++) {
		for (hour=0;hour<24;hour++) {
			if (start>end) {
				if ((hour>=0 && hour<end) || (hour>=start && hour<24)) {
					if (flag == 0) {
						flag = 1;
						start_day = week;
						start_time = hour;
					}
				}
				else {
					if (flag == 1) {
						flag = 0;
						end_day = week;
						end_time = hour;

						len += snprintf(result + len, sizeof(result)-len, "%s%d%d%s%d%s%d", len?"<":"", start_day, end_day, start_time<10?"0":"", start_time, end_time<10?"0":"", end_time);
					}
				}
			}
			else {
				if (hour>=start && hour<end) {
					if (flag == 0) {
						flag = 1;
						start_day = week;
						start_time = hour;
					}
				}
				else {
					if (flag == 1) {
						flag = 0;
						end_day = week;
						end_time = hour;

						len += snprintf(result + len, sizeof(result)-len, "%s%d%d%s%d%s%d", len?"<":"", start_day, end_day, start_time<10?"0":"", start_time, end_time<10?"0":"", end_time);
					}
				}
			}
		}
	}

	if (flag == 1)
		len += snprintf(result + len, sizeof(result)-len, "%s%d%s%s%d%s", len?"<":"", start_day, "0", start_time<10?"0":"", start_time, "00");

	nvram_set("lp55xx_lp5523_sch", result);

	return;
}

#endif	/* RTCONFIG_LP5523 */
