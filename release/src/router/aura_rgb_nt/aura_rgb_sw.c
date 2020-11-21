 /*
 * Copyright 2019, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include <aura_rgb.h>
#include <shared.h>
#include <shutils.h>
#include <aura_rgb_nt.h>

#if defined(GTAXY16000) || defined(GTAX11000) || defined(GTAC2900) || defined(GTAXE11000)
#define	I2C_SLAVE_ADDR	0x4E
#endif

#define	COLOR_NUM	60
#define	COLOR_SET	6
enum
{
	RGBLED_SW_RED		= 0x00,
	RGBLED_SW_GREEN		= 0x02,
	RGBLED_SW_BLUE		= 0x01,
	RGBLED_CONTROL_STOP	= 0x20,
	RGBLED_LED_OFF		= 0x24,
	RGBLED_LED_APPLY		= 0x2F,
};

static int target_rainbow_idx = 0;
static int last_rainbow_idx = 0;

static char *rainbow[] = {
			"255,0,255,0,0,0", "240,0,255,0,0,0", "224,0,248,0,0,0", "208,0,248,0,0,0", "192,0,240,0,0,0", "176,0,240,0,0,0", "160,0,232,0,0,0", "144,0,232,0,0,0", "128,0,224,0,0,0", "112,0,224,0,0,0", //purple
			"0,0,255,0,0,0", "0,16,255,0,0,0", "32,0,248,0,0,0", "0,48,248,0,0,0", "0,64,240,0,0,0", "0,80,240,0,0,0", "0,96,232,0,0,0", "0,112,232,0,0,0", "0,128,224,0,0,0", "0,144,224,0,0,0", //blue
			"0,255,255,0,0,0", "0,255,240,0,0,0", "0,248,224,0,0,0", "0,248,208,0,0,0", "0,240,192,0,0,0", "0,240,176,0,0,0", "0,232,160,0,0,0", "0,232,144,0,0,0", "0,224,128,0,0,0", "0,224,112,0,0,0", //Cyan
			"0,255,0,0,0,0", "16,255,0,0,0,0", "32,248,0,0,0,0", "48,248,0,0,0,0", "64,240,0,0,0,0", "80,240,0,0,0,0", "96,232,0,0,0,0", "112,232,0,0,0,0", "128,224,0,0,0,0", "144,224,0,0,0,0", //green
			"255,255,0,0,0,0", "255,240,0,0,0,0", "248,224,0,0,0,0", "248,208,0,0,0,0", "240,192,0,0,0,0", "240,176,0,0,0,0", "232,160,0,0,0,0", "232,144,0,0,0,0", "224,128,0,0,0,0", "224,112,0,0,0,0", //yellow
			"224,0,32,0,0,0", "224,0,32,0,0,0", "232,0,24,0,0,0", "232,0,24,0,0,0", "240,0,16,0,0,0", "240,0,16,0,0,0", "248,0,8,0,0,0", "248,0,8,0,0,0", "255,0,0,0,0,0", "255,0,0,0,0,0", //red
			 NULL};

int get_aurargb_mode(void)
{
	RGB_LED_STATUS_T rgb_cfg = { 0 };
	if(nv_to_rgb("aurargb_val", &rgb_cfg) == 0)
		return rgb_cfg.mode;
	else
		return -1;
}

static void set_LED_SW_CONTROL_DUTY(int y)
{
	int rainbow_idx_tmp;
	char rgb_cmd[255] = {0};
	RGB_LED_STATUS_T rgb_cfg = { 0 };

	if(y<2)
		rainbow_idx_tmp = last_rainbow_idx;
	else
		rainbow_idx_tmp = (last_rainbow_idx + ((y-1)*((COLOR_NUM)/COLOR_SET)))%(COLOR_NUM);

	memset(&rgb_cfg, 0x00, sizeof(rgb_cfg));
	__nv_to_rgb(rainbow[rainbow_idx_tmp], &rgb_cfg);

	//set LED_SW_CONTROL_DUTY
	snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_SW_RED + (y * 3));
	system(rgb_cmd);
	snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, rgb_cfg.red);
	system(rgb_cmd);

	snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_SW_GREEN + (y * 3));
	system(rgb_cmd);
	snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, rgb_cfg.green);
	system(rgb_cmd);

	snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_SW_BLUE + (y * 3));
	system(rgb_cmd);
	snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, rgb_cfg.blue);
	system(rgb_cmd);

	if(y!=1){
		if(rainbow_idx_tmp%(COLOR_NUM/COLOR_SET) == 0 || rainbow_idx_tmp%(COLOR_NUM/COLOR_SET) == 1){
			usleep(100000);
		}
		else
			usleep(30000);
	}
}

static void set_LED_SW_off(void)
{
	int y=0;
	char rgb_cmd[255] = {0};
	RGB_LED_STATUS_T rgb_cfg = { 0 };

	memset(&rgb_cfg, 0x00, sizeof(rgb_cfg));

	for( y = 4; y >= 0; y--)
	{
		//set LED_SW_CONTROL_DUTY
		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_SW_RED + (y * 3));
		system(rgb_cmd);
		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, rgb_cfg.red);
		system(rgb_cmd);

		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_SW_GREEN + (y * 3));
		system(rgb_cmd);
		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, rgb_cfg.green);
		system(rgb_cmd);

		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_SW_BLUE + (y * 3));
		system(rgb_cmd);
		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x%02x i", I2C_SLAVE_ADDR, rgb_cfg.blue);
		system(rgb_cmd);
	}
}

static void calculate_meter(void)
{
	int traffic_meter=0, rainbow_idx_t = 0;

	traffic_meter = nvram_get_int("aura_traffic_meter");
	AURA_NT_DBG("traffic_meter = %d\n", traffic_meter);
	rainbow_idx_t = traffic_meter/(10240/COLOR_NUM);

	if(rainbow_idx_t >= COLOR_NUM)
		target_rainbow_idx = COLOR_NUM-1;
	else
		target_rainbow_idx = rainbow_idx_t;
	AURA_NT_DBG("target_rainbow_idx = %d\n", target_rainbow_idx);
	return;
}

int main(int argc, char **argv)
{
	int i=0, y=0;
	int direct=0, peroid=0;
	int first_pause=1;
	char rgb_cmd[255] = {0};
	char i2c_addr[12];
	char *detect;

	snprintf(i2c_addr, sizeof(i2c_addr), "%2x", I2C_SLAVE_ADDR);
	// check RGB LED exist

	system("i2cdetect -y -r 0 > /tmp/i2coutput.txt");
	detect = read_whole_file("/tmp/i2coutput.txt");
	if(detect)
	{
		if(!strstr(detect, i2c_addr))
		{
			unlink("/tmp/i2coutput.txt");
			return -1;
		}
		free(detect);
	}
	else
	{
		unlink("/tmp/i2coutput.txt");
		return -1;
	}
	unlink("/tmp/i2coutput.txt");

	while(1){
		if (!nvram_get_int("aurargb_enable")) {
			sleep(3);
			continue;
		}

		AURA_NT_DBG("peroid = %d\n", peroid);
		if(!peroid)
			calculate_meter();

		peroid = (peroid+1)%8;

		if(target_rainbow_idx > last_rainbow_idx){
			direct = 1;
			last_rainbow_idx = (last_rainbow_idx+1)%COLOR_NUM;
		}
		else if(target_rainbow_idx < last_rainbow_idx){
			direct = 0;
			last_rainbow_idx = last_rainbow_idx-1;
		}

		AURA_NT_DBG("last_rainbow_idx = %d\n", last_rainbow_idx);

		if(nvram_get_int("pause_aura_rgb_sw") == 1 || nvram_get_int("pause_aura_rgb_nt") == 1 || get_aurargb_mode() != 0){
			if(first_pause){
				set_LED_SW_off();
				first_pause = 0;
			}
			AURA_NT_DBG("aura_rgb_sw pause\n");
			sleep(3);
			continue;
		}else
			first_pause = 1;

		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_CONTROL_STOP + (i*16));
		system(rgb_cmd);
		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x03 0x01 0x01 i", I2C_SLAVE_ADDR);
		system(rgb_cmd);

		if(direct ==1)
		{
			for( y = 4; y >= 0; y--)
				set_LED_SW_CONTROL_DUTY(y);
		}
		else
		{
			for( y = 0; y < 5; y++)
				set_LED_SW_CONTROL_DUTY(y);
		}
		usleep(60000);
		//apply
		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0 0x80 0x%02x i", I2C_SLAVE_ADDR, RGBLED_LED_APPLY);
		system(rgb_cmd);
		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x%02x 0x01 0x01 b", I2C_SLAVE_ADDR);
		system(rgb_cmd);
	}

	return 0;
}
