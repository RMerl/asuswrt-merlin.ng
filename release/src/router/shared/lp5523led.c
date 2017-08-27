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

static const char lp55xx_path[] = "/sys/class/leds/blue1/device/";
static int brightness = 100;
static int max_unit = 0;

//	Voltage of LEDS
struct lp55xx_leds_pattern lp55xx_leds_col[] = {
//	{ COLOR_INDEX,		blue  green red	}
	{ LP55XX_ALL_LEDS_ON,	"ff", "ff", "ff" },
	{ LP55XX_ALL_LEDS_OFF,	"00", "00", "00" },
	{ LP55XX_ALL_BLEDS_ON,	"ff", "00", "00" },
	{ LP55XX_ALL_GLEDS_ON,	"00", "ff", "00" },
	{ LP55XX_ALL_RLEDS_ON,	"00", "00", "ff" },
	{ LP55XX_ALL_BREATH_LEDS,	"00", "00", "00" },	//100
	{ LP55XX_ALL_DOWN_LEDS,	"00", "00", "00" },		//101
	{ LP55XX_WHITE_LEDS,	"ff", "ff", "8c" },		//102
	{ LP55XX_RED_LEDS,	"00", "00", "c8" },		//103
	{ LP55XX_LIGHT_CYAN_LEDS,	"ff", "ff", "1e" },	//104
	{ LP55XX_ORANGE_LEDS,	"00", "99", "ff" },		//105
	{ LP55XX_PURPLE_LEDS,	"ff", "00", "5a" },		//106
	{ LP55XX_NIAGARA_BLUE_LEDS,	"ff", "41", "0f" },	//107
	{ LP55XX_PALE_DOGWOOD_LEDS,	"6e", "c3", "ff" },	//108
	{ LP55XX_GREENERY_LEDS,	"41", "ff", "32" },		//109
	{ LP55XX_PRIMROSE_YELLOW_LEDS,	"0a", "ff", "ff" },	//110
	{ LP55XX_END_COLOR,	"", "", "" }
};

/* Behavior of LEDS
 * SBLINK = on 499.2ms + off 499.2ms
 * 3 ON 1 OFF = on 748.8ms + off 249.6ms
 *
 * */
struct lp55xx_leds_pattern lp55xx_leds_beh[] = {
//	{ PATTERN_INDEX,		blue  				green 				red	}
	{ LP55XX_ACT_NONE,		"",				"", 				"" },
	{ LP55XX_ACT_SBLINK,		"7e00420040007e004200",		"7e00420040007e004200", 	"7e00420040007e004200" },
	{ LP55XX_ACT_3ON1OFF,		"7e00620040006000",		"7e00620040006000", 		"7e00620040006000" },
	{ LP55XX_ACT_BREATH,		"44101ef042001ff04510",		"44101af042001bf04510", 	"44101cf042001df04510" },
	{ LP55XX_ACT_BREATH_DOWN_011,	"",				"43584508420044084258", 	"43584508420044084258" },
	{ LP55XX_ACT_BREATH_EXCEPTION,	"BREATH_EXCEPTION",		"BREATH_EXCEPTION",	 	"BREATH_EXCEPTION" },
	{ LP55XX_END_BLINK,		"", 				"", 				"" }
};

void split(char **arr, char *str, char *del) {
	char *s = strtok(str, del);

	while(s != NULL) {
		*arr++ = s;
		s = strtok(NULL, del);
	}
}

int get_unit(int value)
{
	return value *9 / 10 / 2;
}

void max_col_cur(char *b, char *g, char *r)
{
	int blue = (int)strtol(b, NULL, 16);
	int green = (int)strtol(g, NULL, 16);
	int red = (int)strtol(r, NULL, 16);
	int max, min;

	max = min = blue;

	if (max<green)
		max = green;

	if (max<red)
		max = red;

	max_unit = get_unit(max) * brightness / 100;

	return;
}

void pattern_breath_ex(int value, char *output)
{
	char *wait = "4200";
	char result[CKN_STR256], increase[CKN_STR256], decrease[CKN_STR256], tmp[CKN_STR256]; 
	int val_unit = get_unit(value);
	int mul_unit = max_unit / val_unit;
	int wait_unit = max_unit - (mul_unit*val_unit);
	int fnl_stp1, fnl_stp2, fnl_stp3;

	memset(result, '\0', sizeof(result));
	memset(increase, '\0', sizeof(increase));
	memset(decrease, '\0', sizeof(decrease));

	if (mul_unit < MAX_STEP) {
		fnl_stp1 = 4 + mul_unit/8;
		fnl_stp2 = (mul_unit%8)*2;
		fnl_stp3 = fnl_stp2 + 1;
	}
	else
		return;

	snprintf(increase, sizeof(increase), "%x%x%s%x", fnl_stp1, fnl_stp2, val_unit<16?"0":"", val_unit);
	snprintf(decrease, sizeof(decrease), "%x%x%s%x", fnl_stp1, fnl_stp3, val_unit<16?"0":"", val_unit);

	while (wait_unit) {
		if (wait_unit < MAX_STEP)
			mul_unit = wait_unit;
		else
			mul_unit = MAX_STEP;

		fnl_stp1 = 4 + mul_unit/8;
		fnl_stp2 = (mul_unit%8)*2;

		memset(tmp, '\0', sizeof(tmp));
		strncpy(tmp, increase, sizeof(increase));
		snprintf(increase, sizeof(increase), "%x%x%s%s", fnl_stp1, fnl_stp2, "00", tmp);

		memset(tmp, '\0', sizeof(tmp));
		strncpy(tmp, decrease, sizeof(decrease));
		snprintf(decrease, sizeof(decrease), "%s%x%x%s", tmp, fnl_stp1, fnl_stp2, "00");

		if (wait_unit < MAX_STEP)
			break;
		else
			wait_unit -= MAX_STEP;
	}

	snprintf(result, sizeof(result), "%s%s%s%s%s", decrease, decrease, wait, increase, increase);

	printf("result: %s\n", result);
	memcpy(output, result, strlen(result));

	return;
}

void pattern_combine(char *initial, char *color, char *behavior, char *output)
{
	char result[CKN_STR256];
	char tmp1[CKN_STR256];
	int color_val = (int)strtol(color, NULL, 16);

	memset(result, '\0', CKN_STR256);
	memset(tmp1, '\0', CKN_STR256);

	color_val = color_val * brightness / 100;
	if (!strncmp(behavior, "BREATH_EXCEPTION", 15))
	{
		if (color_val==0)
			snprintf(result, sizeof(result), "%s%s%x", initial, color_val<16?"0":"", color_val);
		else
		{
			pattern_breath_ex(color_val, tmp1);
			snprintf(result, sizeof(result), "%s%s%x%s%s%s%x", initial, color_val<16?"0":"", color_val, tmp1, "40", color_val<16?"0":"", color_val);
		}
	}
	else 
		snprintf(result, sizeof(result), "%s%s%x%s", initial, color_val<16?"0":"", color_val, behavior);

	memcpy(output, result, strlen(result));

	return;
}

/* combine the str and save
 * @engine_index: select engine 1, 2, 3
 * @str_type: mode, load, leds
 * @value:
 * @return:
*/
void engine_save(int engine_index, char *str_type, char *value) {
	char result[CKN_STR256];
	memset(result, '\0', CKN_STR256);

	snprintf(result, sizeof(result), "%sengine%d_%s", lp55xx_path, engine_index, str_type);

	f_write_string( result, value, 0, 0);

	return;
}

void lp55xx_set_pattern_led(int col_mode, int beh_mode)
{
	char *initial[3] = {"9d8040", "9d8044", "9d8048"};
	struct lp55xx_leds_pattern *blnk_leds_col = lp55xx_leds_col;
	struct lp55xx_leds_pattern *blnk_leds_beh = lp55xx_leds_beh;
	char tmp1[CKN_STR256];
	char tmp2[CKN_STR256];
	char tmp3[CKN_STR256];
	int i=0, set=0;

	memset(tmp1, '\0', CKN_STR256);
	memset(tmp2, '\0', CKN_STR256);
	memset(tmp3, '\0', CKN_STR256);


	for (; blnk_leds_beh->ptn_mode!=LP55XX_END_BLINK; blnk_leds_beh++) {
		if (beh_mode == blnk_leds_beh->ptn_mode){
			if (col_mode==LP55XX_MANUAL_COL)
			{
				char lp55xx_lp5523_manual[CKN_STR256];
				char *arr[3];

				memset(lp55xx_lp5523_manual, '\0', CKN_STR256);
				strcpy(lp55xx_lp5523_manual, nvram_safe_get("lp55xx_lp5523_manual"));
				split(arr, lp55xx_lp5523_manual, "_");
				max_col_cur(arr[0], arr[1], arr[2]);

				pattern_combine(initial[i], arr[0], blnk_leds_beh->ptn1, tmp1);
				if (col_mode==LP55XX_ALL_BREATH_LEDS) i++;
				pattern_combine(initial[i], arr[1], blnk_leds_beh->ptn2, tmp2);
				if (col_mode==LP55XX_ALL_BREATH_LEDS) i++;
				pattern_combine(initial[i], arr[2], blnk_leds_beh->ptn3, tmp3);
			}
			else
			{
				for (; blnk_leds_col->ptn_mode!=LP55XX_END_COLOR; blnk_leds_col++) {
					if (col_mode == blnk_leds_col->ptn_mode){
						max_col_cur(blnk_leds_col->ptn1, blnk_leds_col->ptn2, blnk_leds_col->ptn3);

						pattern_combine(initial[i], blnk_leds_col->ptn1, blnk_leds_beh->ptn1, tmp1);
						if (col_mode==LP55XX_ALL_BREATH_LEDS) i++;
						pattern_combine(initial[i], blnk_leds_col->ptn2, blnk_leds_beh->ptn2, tmp2);
						if (col_mode==LP55XX_ALL_BREATH_LEDS) i++;
						pattern_combine(initial[i], blnk_leds_col->ptn3, blnk_leds_beh->ptn3, tmp3);
					}
				}
			}

			set=1;
			break;
		}
	}

	if (set)
	{
		for (i = LP55XX_ENGINE_1; i <= LP55XX_ENGINE_3; i++) {
			if (i==LP55XX_ENGINE_1)
			{
				engine_save(i, "load", tmp1);
				engine_save(i, "leds", "111000000");
			}
			else if (i==LP55XX_ENGINE_2)
			{
				engine_save(i, "load", tmp2);
				engine_save(i, "leds", "000111000");
			}
			else
			{
				engine_save(i, "load", tmp3);
				engine_save(i, "leds", "000000111");
			}
		}
	}
}

void lp55xx_blink_leds(int col_mode, int beh_mode)
{
	int i;

	for (i = LP55XX_ENGINE_1; i <= LP55XX_ENGINE_3; i++)
		engine_save(i, "mode", "disabled");

	for (i = LP55XX_ENGINE_1; i <= LP55XX_ENGINE_3; i++)
		engine_save(i, "mode", "load");

	lp55xx_set_pattern_led(col_mode, beh_mode);

	for (i = LP55XX_ENGINE_1; i <= LP55XX_ENGINE_3; i++)
		engine_save(i, "mode", "run");
}

void lp55xx_leds_proc(int col_mode, int beh_mode)
{
	int beh_mode_tmp = beh_mode;
	int col_old = nvram_get_int("lp55xx_lp5523_col");
	int beh_old = nvram_get_int("lp55xx_lp5523_beh");

	brightness = 100;

	if (col_mode==col_old && beh_mode_tmp==beh_old)
		return;

	// Behavior of ACT
	switch (beh_mode_tmp)
	{
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
		default:
			nvram_set_int("lp55xx_lp5523_col", col_mode);
			nvram_set_int("lp55xx_lp5523_beh", beh_mode_tmp);
			nvram_commit();
			break;
	}

	// Color
	switch (col_mode)
	{
		case LP55XX_LIGHT_CYAN_LEDS:
			if (nvram_match("lp55xx_lp5523_user_enable", "1")
				&& !nvram_match("lp55xx_lp5523_sch_enable", "2"))
			{
				col_mode = nvram_get_int("lp55xx_lp5523_user_col");
				beh_mode_tmp = nvram_get_int("lp55xx_lp5523_user_beh");
				brightness = nvram_get_int("lp55xx_lp5523_user_brightness");
			}
			break;
		case LP55XX_ALL_BREATH_LEDS:
			beh_mode_tmp=LP55XX_ACT_BREATH;
			break;
	}

	// Behavior of Blink Mode
	if (beh_mode_tmp==LP55XX_ACT_BREATH && col_mode!=LP55XX_ALL_BREATH_LEDS)
		 beh_mode_tmp = LP55XX_ACT_BREATH_EXCEPTION;

	// Check schedule
	if (nvram_match("lp55xx_lp5523_sch_enable", "2"))
	{
		if (beh_mode!=LP55XX_SCH_ENABLE)
			return;
	}

	lp55xx_blink_leds(LP55XX_ALL_LEDS_OFF, LP55XX_ACT_NONE);
	if (col_mode != LP55XX_ALL_LEDS_OFF)
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

	memset(result, '\0', 128);

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

						snprintf(result, sizeof(result), "%s%s%d%d%s%d%s%d", result, strlen(result)?"<":"", start_day, end_day, start_time<10?"0":"", start_time, end_time<10?"0":"", end_time);
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

						snprintf(result, sizeof(result), "%s%s%d%d%s%d%s%d", result, strlen(result)?"<":"", start_day, end_day, start_time<10?"0":"", start_time, end_time<10?"0":"", end_time);
					}
				}
			}
		}
	}

	if (flag == 1)
		snprintf(result, sizeof(result), "%s%s%d%s%s%d%s", result, strlen(result)?"<":"", start_day, "0", start_time<10?"0":"", start_time, "00");

	nvram_set("lp55xx_lp5523_sch", result);

	return;
}

#endif	/* RTCONFIG_LP5523 */
