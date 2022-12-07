#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcmnvram.h>
#include <bcmdevs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <ctype.h>
#include <wlutils.h>
#include <linux_gpio.h>
#include <etioctl.h>
#include "utils.h"
#include "shutils.h"
#include "shared.h"
#include <trxhdr.h>
#include <bcmutils.h>
#include <bcmendian.h>
#if defined(GT10) && !defined(RTCONFIG_BCM_MFG)
#include <signal.h>
#endif

#ifdef RTCONFIG_QTN
#include "web-qtn.h"
#endif
#ifdef HND_ROUTER
#include <board.h>
#endif

typedef enum cmds_e {
        REGACCESS,
        PMDIOACCESS,
} ecmd_t;

extern uint64_t hnd_ethswctl(ecmd_t act, unsigned int val, int len, int wr, unsigned long long regdata);

uint32_t gpio_dir(uint32_t gpio, int dir)
{
	/* FIXME
	return bcmgpio_connect(gpio, dir);
	 */

	return 0;
}

#define swapportstatus(x) \
{ \
    unsigned int data = *(unsigned int*)&(x); \
    data = ((data & 0x000c0000) >> 18) |    \
           ((data & 0x00030000) >> 14) |    \
           ((data & 0x0000c000) >> 10) |    \
           ((data & 0x00003000) >>  6) |    \
	   ((data & 0x00000c00) >>  2);     \
    *(unsigned int*)&(x) = data;            \
}

extern uint32_t gpio_read(void);
extern void gpio_write(uint32_t bitvalue, int en);

#if defined(RTCONFIG_BCM_CLED)

typedef struct {
	/* config[0]: path, config[1]: value */
	char config0_path[3][40];
	char config0_val[3][20];
	char config1_path[3][40];
	char config1_val[3][20];
	char config2_path[3][40];
	char config2_val[3][20];
	char config3_path[3][40];
	char config3_val[3][20];
}bcm_cled_rgb_led_s;

#if defined(RPAX56) || defined(RPAX58) || defined(ET12) || defined(XT12)
typedef struct {
	char config0_path[40];
	char config0_val[20];
	char config1_path[40];
	char config1_val[20];
	char config2_path[40];
	char config2_val[20];
	char config3_path[40];
	char config3_val[20];
}bcm_cled_x_led_s;
#endif

#if defined(ET12) || defined(XT12)
typedef struct {
	char RED[BCM_CLED_MODE_END][20];
	char GREEN[BCM_CLED_MODE_END][20];
	char BLUE[BCM_CLED_MODE_END][20];
}bcm_cled_color_blend;
#endif

int read_cled_value(bcm_cled_rgb_led_s *cur_led)
{
	int i = 0;
#if defined(BCM4912) || defined(RPAX58) || defined(GT10)
	FILE *fp = NULL;
	char *ptr = NULL, *saveptr, *val;
	char cmd[64], buf[128];
	int found;

	for(i = 0; i < 3; i++)	{
		snprintf(cmd, sizeof(cmd), "dw %s 4", cur_led->config0_path[i]);
		fp = popen(cmd, "r");
		if (fp) {
			memset(buf, 0, sizeof(buf));
			while(fgets(buf, sizeof(buf), fp) != NULL) {
				if((ptr = strchr(buf, ':')) != NULL) {
					found = 1;
					break;
				}
			}
			pclose(fp);
		}

		if (found) {
			ptr++;
			//config0
			if (!(val = strtok_r(ptr, " ", &saveptr)))
			    return -1;
			if(strlen(val)) {
				 snprintf(cur_led->config0_val[i], sizeof(cur_led->config0_val[i]), "%s", val);
			}
			//config1
			if (!(val = strtok_r(NULL, " ", &saveptr)))
				return -1;
			if(strlen(val)) {
				 snprintf(cur_led->config1_val[i], sizeof(cur_led->config1_val[i]), "%s", val);
			}
			//config2
			if (!(val = strtok_r(NULL, " ", &saveptr)))
				return -1;
			if(strlen(val)) {
				 snprintf(cur_led->config2_val[i], sizeof(cur_led->config2_val[i]), "%s", val);
			}
			//config3
			if (!(val = strtok_r(NULL, " ", &saveptr)))
				return -1;
			if(strlen(val)) {
				 snprintf(cur_led->config3_val[i], sizeof(cur_led->config3_val[i]), "%s", val);
			}
		}
	}
#else
	for(i = 0; i < 3; i++){
		f_read_string(cur_led->config0_path[i], cur_led->config0_val[i], sizeof(cur_led->config0_val[i]));
		f_read_string(cur_led->config1_path[i], cur_led->config1_val[i], sizeof(cur_led->config1_val[i]));
		f_read_string(cur_led->config2_path[i], cur_led->config2_val[i], sizeof(cur_led->config2_val[i]));
		f_read_string(cur_led->config3_path[i], cur_led->config3_val[i], sizeof(cur_led->config3_val[i]));
	}
#endif
	return 0;
}

#if defined(RPAX56) || defined(RPAX58) || defined(ET12) || defined(XT12)
int read_cled_value_x(bcm_cled_x_led_s *cur_led)
{
#if defined(BCM4912) || defined(RPAX58) || defined(GT10)
	FILE *fp = NULL;
	char *ptr, *saveptr, *val;
	char cmd[64], buf[128];
	int found;

	snprintf(cmd, sizeof(cmd), "dw %s 4", cur_led->config0_path);

	fp = popen(cmd, "r");
	if (fp) {
		memset(buf, 0, sizeof(buf));
		while(fgets(buf, sizeof(buf), fp) != NULL) {
			if((ptr = strchr(buf, ':')) != NULL) {
				found = 1;
				break;
			}
		}
		pclose(fp);
	}

	if (found) {
		ptr++;
		//config0
		if (!(val = strtok_r(ptr, " ", &saveptr)))
		    return -1;
		if(strlen(val)) {
			 snprintf(cur_led->config0_val, sizeof(cur_led->config0_val), "%s", val);
		}
		//config1
		if (!(val = strtok_r(NULL, " ", &saveptr)))
			return -1;
		if(strlen(val)) {
			 snprintf(cur_led->config1_val, sizeof(cur_led->config1_val), "%s", val);
		}
		//config2
		if (!(val = strtok_r(NULL, " ", &saveptr)))
			return -1;
		if(strlen(val)) {
			 snprintf(cur_led->config2_val, sizeof(cur_led->config2_val), "%s", val);
		}
		//config3
		if (!(val = strtok_r(NULL, " ", &saveptr)))
			return -1;
		if(strlen(val)) {
			 snprintf(cur_led->config3_val, sizeof(cur_led->config3_val), "%s", val);
		}
	}
#else
	f_read_string(cur_led->config0_path, cur_led->config0_val, sizeof(cur_led->config0_val));
	f_read_string(cur_led->config1_path, cur_led->config1_val, sizeof(cur_led->config1_val));
	f_read_string(cur_led->config2_path, cur_led->config2_val, sizeof(cur_led->config2_val));
	f_read_string(cur_led->config3_path, cur_led->config3_val, sizeof(cur_led->config3_val));
#endif
}

#endif


int is_cled_value_correct(bcm_cled_rgb_led_s *cur_led, int rgb_id, char *val0, char *val1, char *val2, char *val3)
{
#if defined(BCM4912) || defined(RPAX58) || defined(GT10)
	if(strtoul(cur_led->config0_val[rgb_id], NULL, 16) != strtoul(val0, NULL, 16) ||
	   strtoul(cur_led->config1_val[rgb_id], NULL, 16) != strtoul(val1, NULL, 16) ||
	   strtoul(cur_led->config2_val[rgb_id], NULL, 16) != strtoul(val2, NULL, 16) ||
	   strtoul(cur_led->config3_val[rgb_id], NULL, 16) != strtoul(val3, NULL, 16)) {
		return 0;
	}
	else {
		return 1;
	}
#else
	if(strcmp(cur_led->config0_val[rgb_id], val0) != 0 ||
		strcmp(cur_led->config1_val[rgb_id], val1) != 0 ||
		strcmp(cur_led->config2_val[rgb_id], val2) != 0 ||
		strcmp(cur_led->config3_val[rgb_id], val3) != 0){
		return 0;
	}else{
		return 1;
	}
#endif
}

int set_cled_value(bcm_cled_rgb_led_s *cur_led, int rgb_id, char *val0, char *val1, char *val2, char *val3)
{
#if defined(BCM4912) || defined(RPAX58) || defined(GT10)
	eval("sw", cur_led->config0_path[rgb_id], val0, val1, val2, val3);
#else
	f_write_string(cur_led->config0_path[rgb_id], val0, 0, 0);
	f_write_string(cur_led->config1_path[rgb_id], val1, 0, 0);
	f_write_string(cur_led->config2_path[rgb_id], val2, 0, 0);
	f_write_string(cur_led->config3_path[rgb_id], val3, 0, 0);
#endif
	return 0;
}

#if defined(RPAX56) || defined(RPAX58) || defined(ET12) || defined(XT12)
int is_cled_value_correct_x(bcm_cled_x_led_s *cur_led, char *val0, char *val1, char *val2, char *val3)
{
#if defined(BCM4912) || defined(RPAX58) || defined(GT10)
	if(strtoul(cur_led->config0_val, NULL, 16) != strtoul(val0, NULL, 16) ||
	   strtoul(cur_led->config1_val, NULL, 16) != strtoul(val1, NULL, 16) ||
	   strtoul(cur_led->config2_val, NULL, 16) != strtoul(val2, NULL, 16) ||
	   strtoul(cur_led->config3_val, NULL, 16) != strtoul(val3, NULL, 16)) {
		return 0;
	}
	else {
		return 1;
	}
#else
	if(strcmp(cur_led->config0_val, val0) != 0 ||
		strcmp(cur_led->config1_val, val1) != 0 ||
		strcmp(cur_led->config2_val, val2) != 0 ||
		strcmp(cur_led->config3_val, val3) != 0){
		return 0;
	}else{
		return 1;
	}
#endif
}

int set_cled_value_x(bcm_cled_x_led_s *cur_led, char *val0, char *val1, char *val2, char *val3)
{
#if defined(BCM4912) || defined(RPAX58) || defined(GT10)
#ifdef RPAX58
	strlcpy(cur_led->config0_val, val0, sizeof(cur_led->config0_val));
	strlcpy(cur_led->config1_val, val1, sizeof(cur_led->config0_val));
	strlcpy(cur_led->config2_val, val2, sizeof(cur_led->config0_val));
	strlcpy(cur_led->config3_val, val3, sizeof(cur_led->config0_val));
#endif
	eval("sw", cur_led->config0_path, val0, val1, val2, val3);
#else
	f_write_string(cur_led->config0_path, val0, 0, 0);
	f_write_string(cur_led->config1_path, val1, 0, 0);
	f_write_string(cur_led->config2_path, val2, 0, 0);
	f_write_string(cur_led->config3_path, val3, 0, 0);
#endif
}
#endif

#if defined(ET12) || defined(XT12)
#define CLED_RGB_NUM 3
#else
#define CLED_RGB_NUM 1
#endif

int _bcm_cled_ctrl(int rgb, int cled_mode)
{
	int index;
	int state_changed = 0;
#if defined(XT12) || defined(ET12)
	bcm_cled_color_blend LED_BEHAVIOR[BCM_CLED_END] = {
			//BCM_CLED_RED
			{{"0x0003ffc0", "0x0003d000", "0x0003c400", "0x0003ffd8", "0x0003ffc2", "0x0003fff8"},
			 {"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000"},
			 {"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000"}},
			//BCM_CLED_GREEN
			{{"0x0003c883", "0x0003c400", "0x0003c100", "0x0003c89b", "0x0003c883", "0x0003c8bb"},
			 {"0x0003ffc0", "0x0003efc0", "0x0003dfc0", "0x0003ffd8", "0x0003ffc2", "0x0003fff8"},
			 {"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000"}},
			//BCM_CLED_BLUE
			{{"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000"},
			 {"0x0003c800", "0x0003c400", "0x0003c100", "0x0003c818", "0x0003c802", "0x0003c838"},
			 {"0x0003ffc0", "0x0003efc0", "0x0003dfc0", "0x0003ffd8", "0x0003ffc2", "0x0003fff8"}},
			//BCM_CLED_YELLOW
			{{"0x0003ffc0", "0x0003efc0", "0x0003dfc0", "0x0003ffd8", "0x0003ffc2", "0x0003fff8"},
			 {"0x0003f400", "0x0003e400", "0x0003d400", "0x0003f418", "0x0003f402", "0x0003f438"},
			 {"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000"}},
			//BCM_CLED_WHITE
			{{"0x0003ee00", "0x0003de00", "0x0003ce00", "0x0003ee18", "0x0003ee02", "0x0003ee38"},
			 {"0x0003ffc0", "0x0003efc0", "0x0003dfc0", "0x0003ffd8", "0x0003ffc2", "0x0003fff8"},
			 {"0x0003ffc0", "0x0003efc0", "0x0003dfc0", "0x0003ffd8", "0x0003ffc2", "0x0003fff8"}},
			//BCM_CLED_OFF
			{{"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000"},
			 {"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000"},
			 {"0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000", "0x00000000"}}
	};
#else
	char LED_BEHAVIOR_WRITE[BCM_CLED_MODE_END][20] =
			{"0x0003e000", "0x0003d000", "0x0003c400", "0x0003e018", "0x0003e002", "0x0003e038"};
	char LED_BEHAVIOR_READ[BCM_CLED_MODE_END][20] =
			{"3e000\n", "3d000\n", "3c400\n", "3e018\n", "3e002\n", "3e038\n"};
#endif
	bcm_cled_rgb_led_s led[CLED_RGB_NUM] = {
#if defined(RTAX82_XD6) || defined(XD6_V2)
		{"/proc/bcm_cled/led7/config0", "/proc/bcm_cled/led8/config0", "/proc/bcm_cled/led9/config0",
		"0x00000000", "0x00000000", "0x00000000",
		"/proc/bcm_cled/led7/config1", "/proc/bcm_cled/led8/config1", "/proc/bcm_cled/led9/config1",
		"0x00000000", "0x00000000", "0x00000000",
		"/proc/bcm_cled/led7/config2", "/proc/bcm_cled/led8/config2", "/proc/bcm_cled/led9/config2",
		"0x00000000", "0x00000000", "0x00000000",
		"/proc/bcm_cled/led7/config3", "/proc/bcm_cled/led8/config3", "/proc/bcm_cled/led9/config3",
		"0x00000000", "0x00000000", "0x00000000"}
#elif defined(RTAX82_XD6S)
		{"/proc/bcm_cled/led12/config0", "/proc/bcm_cled/led13/config0", "/proc/bcm_cled/led20/config0",
		"0x00000000", "0x00000000", "0x00000000",
		"/proc/bcm_cled/led12/config1", "/proc/bcm_cled/led13/config1", "/proc/bcm_cled/led20/config1",
		"0x00000000", "0x00000000", "0x00000000",
		"/proc/bcm_cled/led12/config2", "/proc/bcm_cled/led13/config2", "/proc/bcm_cled/led20/config2",
		"0x00000000", "0x00000000", "0x00000000",
		"/proc/bcm_cled/led12/config3", "/proc/bcm_cled/led13/config3", "/proc/bcm_cled/led20/config3",
		"0x00000000", "0x00000000", "0x00000000"}
#elif defined(RPAX56)
                {"/proc/bcm_cled/led5/config0", "/proc/bcm_cled/led7/config0", "/proc/bcm_cled/led11/config0",
                "0x00000000", "0x00000000", "0x00000000",
                "/proc/bcm_cled/led5/config1", "/proc/bcm_cled/led7/config1", "/proc/bcm_cled/led11/config1",
                "0x00000000", "0x00000000", "0x00000000",
                "/proc/bcm_cled/led5/config2", "/proc/bcm_cled/led7/config2", "/proc/bcm_cled/led11/config2",
                "0x00000000", "0x00000000", "0x00000000",
                "/proc/bcm_cled/led5/config3", "/proc/bcm_cled/led7/config3", "/proc/bcm_cled/led11/config3",
                "0x00000000", "0x00000000", "0x00000000"}
#elif defined(RPAX58)
                {"0xff803070", "0xff803090", "0xff8030d0",
                "0x00000000", "0x00000000", "0x00000000",
                "0xff803074", "0xff803094", "0xff8030d4",
                "0x00000000", "0x00000000", "0x00000000",
                "0xff803078", "0xff803098", "0xff8030d8",
                "0x00000000", "0x00000000", "0x00000000",
                "0xff80307c", "0xff80309c", "0xff8030dc",
                "0x00000000", "0x00000000", "0x00000000"}
#elif defined(GT10)
		{{"0xff803020", "0xff803030", "0xff803040"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff803024", "0xff803034", "0xff803044"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff803028", "0xff803038", "0xff803048"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff80302c", "0xff80303c", "0xff80304c"},
		 {"0x00000000", "0x00000000", "0x00000000"}}
#elif defined(ET12) || defined(XT12)
		// LED1 [cled21 - cled17 - cled16]
		{{"0xff803170", "0xff803130", "0xff803120"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff803174", "0xff803134", "0xff803124"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff803178", "0xff803138", "0xff803128"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff80317c", "0xff80313c", "0xff80312c"},
		 {"0x00000000", "0x00000000", "0x00000000"}},
		// LED2 [cled18 - cled1 - cled13]
		{{"0xff803140", "0xff803030", "0xff8030f0"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff803144", "0xff803034", "0xff8030f4"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff803148", "0xff803038", "0xff8030f8"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff80314c", "0xff80303c", "0xff8030fc"},
		 {"0x00000000", "0x00000000", "0x00000000"}},
		// LED3 [cled2 - cled14 - cled15]
		{{"0xff803040", "0xff803100", "0xff803110"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff803044", "0xff803104", "0xff803114"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff803048", "0xff803108", "0xff803118"},
		 {"0x00000000", "0x00000000", "0x00000000"},
		 {"0xff80304c", "0xff80310c", "0xff80311c"},
		 {"0x00000000", "0x00000000", "0x00000000"}}
#else
		{"/proc/bcm_cled/led14/config0", "/proc/bcm_cled/led15/config0", "/proc/bcm_cled/led16/config0",
                "0x00000000", "0x00000000", "0x00000000",
		"/proc/bcm_cled/led14/config1", "/proc/bcm_cled/led15/config1", "/proc/bcm_cled/led16/config1",
                "0x00000000", "0x00000000", "0x00000000",
		"/proc/bcm_cled/led14/config2", "/proc/bcm_cled/led15/config2", "/proc/bcm_cled/led16/config2",
                "0x00000000", "0x00000000", "0x00000000",
		"/proc/bcm_cled/led14/config3", "/proc/bcm_cled/led15/config3", "/proc/bcm_cled/led16/config3",
                "0x00000000", "0x00000000", "0x00000000"}
#endif
	};

#if defined(RPAX56)
	bcm_cled_x_led_s led_w = {
                "/proc/bcm_cled/led12/config0",
                "0x00000000",
                "/proc/bcm_cled/led12/config1",
                "0x00000000",
                "/proc/bcm_cled/led12/config2",
                "0x00000000",
                "/proc/bcm_cled/led12/config3",
                "0x00000000"
	};
	read_cled_value_x(&led_w);

	bcm_cled_x_led_s led_y = {
                "/proc/bcm_cled/led14/config0",
                "0x00000000",
                "/proc/bcm_cled/led14/config1",
                "0x00000000",
                "/proc/bcm_cled/led14/config2",
                "0x00000000",
                "/proc/bcm_cled/led14/config3",
                "0x00000000"
	};
	read_cled_value_x(&led_y);
#elif defined(RPAX58)
	bcm_cled_x_led_s led_w = {
                "0xff8030e0",
                "0x00000000",
                "0xff8030e4",
                "0x00000000",
                "0xff8030e8",
                "0x00000000",
                "0xff8030ec",
                "0x00000000"
	};
	read_cled_value_x(&led_w);

	bcm_cled_x_led_s led_y = {
                "0xff803100",
                "0x00000000",
                "0xff803104",
                "0x00000000",
                "0xff803108",
                "0x00000000",
                "0xff80310c",
                "0x00000000"
	};
	read_cled_value_x(&led_y);

	bcm_cled_x_led_s led_p = {
                "0xff803110",
                "0x00000000",
                "0xff803114",
                "0x00000000",
                "0xff803118",
                "0x00000000",
                "0xff80311c",
                "0x00000000"
	};
	read_cled_value_x(&led_p);
#endif

	for(index = 0; index < CLED_RGB_NUM; index++) {
		read_cled_value(&led[index]);

#if defined(XT12) || defined(ET12)
		if(is_cled_value_correct(&led[index], BCM_CLED_RED, LED_BEHAVIOR[rgb].RED[cled_mode], "a34a32\n", "c34\n", "0\n") == 0 ||
		   is_cled_value_correct(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR[rgb].GREEN[cled_mode], "a34a32\n", "c34\n", "0\n") == 0 ||
		   is_cled_value_correct(&led[index], BCM_CLED_BLUE, LED_BEHAVIOR[rgb].BLUE[cled_mode], "a34a32\n", "c34\n", "0\n") == 0) {
				 set_cled_value(&led[index], BCM_CLED_RED, LED_BEHAVIOR[rgb].RED[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				 set_cled_value(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR[rgb].GREEN[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				 set_cled_value(&led[index], BCM_CLED_BLUE, LED_BEHAVIOR[rgb].BLUE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				 state_changed = 1;
		}
#else
		if(rgb == BCM_CLED_RED ){
			if(is_cled_value_correct(&led[index], BCM_CLED_RED, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0 ||
				strtoul(led[index].config0_val[BCM_CLED_GREEN], NULL, 16) != 0 ||
				strtoul(led[index].config0_val[BCM_CLED_BLUE], NULL, 16) != 0) {
				set_cled_value(&led[index], BCM_CLED_RED, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
#if defined(BCM4912) || defined(RPAX58) || defined(GT10)
				eval("sw", led[index].config0_path[BCM_CLED_GREEN], "0x00000000");
				eval("sw", led[index].config0_path[BCM_CLED_BLUE], "0x00000000");
#else
				f_write_string(led[index].config0_path[BCM_CLED_GREEN], "0x00000000", 0, 0);
				f_write_string(led[index].config0_path[BCM_CLED_BLUE], "0x00000000", 0, 0);
#endif
				state_changed = 1;
			}
#if defined(RPAX56) || defined(RPAX58)
			if(is_cled_value_correct_x(&led_w, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_w, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
#ifdef RPAX58
				eval("sw", led_w.config0_path, "0x00000000");
#endif
				state_changed = 1;
			}
			if(is_cled_value_correct_x(&led_y, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_y, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
#ifdef RPAX58
				eval("sw", led_y.config0_path, "0x00000000");
#endif
				state_changed = 1;
			}
#ifdef RPAX58
			if(is_cled_value_correct_x(&led_p, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_p, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				eval("sw", led_p.config0_path, "0x00000000");
				state_changed = 1;
			}
#endif
#endif
		}else if(rgb == BCM_CLED_GREEN){
			if(is_cled_value_correct(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0 ||
				strtoul(led[index].config0_val[BCM_CLED_RED], NULL, 16) != 0 ||
				strtoul(led[index].config0_val[BCM_CLED_BLUE], NULL, 16) != 0){

				set_cled_value(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
#if defined(BCM4912) || defined(RPAX58) || defined(GT10)
				eval("sw", led[index].config0_path[BCM_CLED_RED], "0x00000000");
				eval("sw", led[index].config0_path[BCM_CLED_BLUE], "0x00000000");
#else
				f_write_string(led[index].config0_path[BCM_CLED_RED], "0x00000000", 0, 0);
				f_write_string(led[index].config0_path[BCM_CLED_BLUE], "0x00000000", 0, 0);
#endif
				state_changed = 1;
			}
#if defined(RPAX56) || defined(RPAX58)
			if(is_cled_value_correct_x(&led_w, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_w, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
#ifdef RPAX58
				eval("sw", led_w.config0_path, "0x00000000");
#endif
				state_changed = 1;
			}
			if(is_cled_value_correct_x(&led_y, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_y, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
#ifdef RPAX58
				eval("sw", led_y.config0_path, "0x00000000");
#endif
				state_changed = 1;
			}
#ifdef RPAX58
			if(is_cled_value_correct_x(&led_p, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_p, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				eval("sw", led_p.config0_path, "0x00000000");
				state_changed = 1;
			}
#endif
#endif
		}else if(rgb == BCM_CLED_BLUE){
			if(is_cled_value_correct(&led[index], BCM_CLED_BLUE, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0 ||
				strtoul(led[index].config0_val[BCM_CLED_RED], NULL, 16) != 0 ||
				strtoul(led[index].config0_val[BCM_CLED_GREEN], NULL, 16) != 0){

				set_cled_value(&led[index], BCM_CLED_BLUE, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
#if defined(BCM4912) || defined(RPAX58) || defined(GT10)
				eval("sw", led[index].config0_path[BCM_CLED_RED], "0x00000000");
				eval("sw", led[index].config0_path[BCM_CLED_GREEN], "0x00000000");
#else
				f_write_string(led[index].config0_path[BCM_CLED_RED], "0x00000000", 0, 0);
				f_write_string(led[index].config0_path[BCM_CLED_GREEN], "0x00000000", 0, 0);
#endif
				state_changed = 1;
			}
#if defined(RPAX56) || defined(RPAX58)
			if(is_cled_value_correct_x(&led_w, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_w, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
#ifdef RPAX58
				eval("sw", led_w.config0_path, "0x00000000");
#endif
				state_changed = 1;
			}
			if(is_cled_value_correct_x(&led_y, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_y, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
#ifdef RPAX58
				eval("sw", led_y.config0_path, "0x00000000");
#endif
				state_changed = 1;
			}
#ifdef RPAX58
			if(is_cled_value_correct_x(&led_p, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_p, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				eval("sw", led_p.config0_path, "0x00000000");
				state_changed = 1;
			}
#endif
#endif
		}else if(rgb == BCM_CLED_YELLOW){
#if defined(RPAX56) || defined(RPAX58)
			if(is_cled_value_correct_x(&led_w, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_w, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
#ifdef RPAX58
				eval("sw", led_w.config0_path, "0x00000000");
#endif
				state_changed = 1;
			}
#ifdef RPAX58
			if(is_cled_value_correct_x(&led_p, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_p, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				eval("sw", led_p.config0_path, "0x00000000");
				state_changed = 1;
			}
#endif
			if(is_cled_value_correct_x(&led_y, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_y, LED_BEHAVIOR_READ[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
#ifdef RPAX58
				eval("sw", led_y.config0_path, led_y.config0_val);
#endif
				state_changed = 1;
			}
			if(is_cled_value_correct(&led[index], BCM_CLED_RED, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_RED, LED_BEHAVIOR_READ[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
#ifdef RPAX58
			int _cled_mode = cled_mode;
			if(_cled_mode == BCM_CLED_STEADY_NOBLINK)
				_cled_mode = BCM_CLED_STEADY_NOBLINK_DIM;
			if(is_cled_value_correct(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR_READ[_cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR_READ[_cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
#else
			if(is_cled_value_correct(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR_READ[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
#endif
#else
			if(is_cled_value_correct(&led[index], BCM_CLED_RED, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_RED, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
#endif
			if(is_cled_value_correct(&led[index], BCM_CLED_BLUE, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_BLUE, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
		}else if(rgb == BCM_CLED_WHITE){
#if defined(RPAX56) || defined(RPAX58)
			if(is_cled_value_correct(&led[index], BCM_CLED_RED, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_RED, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct(&led[index], BCM_CLED_GREEN, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_GREEN, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct(&led[index], BCM_CLED_BLUE, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_BLUE, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct_x(&led_w, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_w, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
#ifdef RPAX58
				eval("sw", led_w.config0_path, led_w.config0_val);
#endif
				state_changed = 1;
			}
			if(is_cled_value_correct_x(&led_y, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_y, "0\n", "0x00a34a32", "0x00000c34", "0x00000000");
#ifdef RPAX58
				eval("sw", led_y.config0_path, "0");
#endif
				state_changed = 1;
			}
#ifdef RPAX58
			if(is_cled_value_correct_x(&led_p, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_p, "0\n", "0x00a34a32", "0x00000c34", "0x00000000");
				eval("sw", led_p.config0_path, "0");
				state_changed = 1;
			}
#endif
#else
			if(is_cled_value_correct(&led[index], BCM_CLED_RED, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_RED, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_GREEN, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct(&led[index], BCM_CLED_BLUE, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_BLUE, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
#endif
#ifdef RPAX58
		}else if(rgb == BCM_CLED_PURPLE){
			if(is_cled_value_correct(&led[index], BCM_CLED_RED, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_RED, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct(&led[index], BCM_CLED_BLUE, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_BLUE, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct(&led[index], BCM_CLED_GREEN, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_GREEN, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct_x(&led_p, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_p, LED_BEHAVIOR_READ[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				eval("sw", led_p.config0_path, led_p.config0_val);
				state_changed = 1;
			}
			if(is_cled_value_correct_x(&led_y, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_y, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				eval("sw", led_y.config0_path, "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct_x(&led_w, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_w, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				eval("sw", led_w.config0_path, "0x00000000");
				state_changed = 1;
			}
#endif
		}else if(rgb == BCM_CLED_OFF){
			if(is_cled_value_correct(&led[index], BCM_CLED_RED, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_RED, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct(&led[index], BCM_CLED_GREEN, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_GREEN, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct(&led[index], BCM_CLED_BLUE, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value(&led[index], BCM_CLED_BLUE, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
#if defined(RPAX56) || defined(RPAX58)
			if(is_cled_value_correct_x(&led_w, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_w, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
			if(is_cled_value_correct_x(&led_y, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_y, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
#ifdef RPAX58
			if(is_cled_value_correct_x(&led_p, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_p, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
#endif
#endif
		}
#endif
	}

	return state_changed;
}

/* rgb: 0:red, 1:green, 2:blue, 3:white */
int bcm_cled_ctrl(int rgb, int cled_mode)
{
	int state_changed = 0;

#if defined(RPAX56) || defined(RPAX58)
	if(ate_brcm_factory_mode() || nvram_match("stop_watchdog", "1")) {
		//_dprintf("skip bcmcledctrl under atemode\n");
		return 0;
	}
#endif
#if defined(GT10) && !defined(RTCONFIG_BCM_MFG)
	if (nvram_get_int("asus_mfg") != 1) {
		led_control(LED_WHITE, (rgb == BCM_CLED_WHITE) ? LED_ON : LED_OFF);
		if (rgb == BCM_CLED_WHITE)
			rgb = BCM_CLED_OFF;
#ifdef RTCONFIG_AMAS
		else if (nvram_get_int("re_mode") && rgb == BCM_CLED_BLUE) {
			nvram_set_int("ledg_scheme_tmp", LEDG_SCHEME_BLINKING);
			kill_pidfile_s("/var/run/ledg.pid", SIGTSTP);
		}
#endif
	}
#endif
#if defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(CTAX56_XD4) || defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(RPAX56) || defined(RPAX58) || defined(ET12) || defined(XT12) || defined(GT10) || defined(XD6_V2)
	state_changed = _bcm_cled_ctrl(rgb, cled_mode);
	if(state_changed == 1){
#if defined(RTAX82_XD6) || defined(XD6_V2)
		f_write_string("/proc/bcm_cled/activate", "0x00000380", 0, 0);
#elif defined(RTAX82_XD6S)
		f_write_string("/proc/bcm_cled/activate", "0x00103000", 0, 0);
#elif defined(RPAX56)
                f_write_string("/proc/bcm_cled/activate", "0x000058a0", 0, 0);
#elif defined(RPAX58)
		eval("sw", "0xff80301c", "0x0000d8a0");
#elif defined(ET12) || defined(XT12)
		eval("sw", "0xff80301c", "0x0027E006");
#elif defined(GT10)
		eval("sw", "0xff80301c", "0x00000007");
#else
		f_write_string("/proc/bcm_cled/activate", "0x0001C000", 0, 0);
#endif
	}
#endif
	return state_changed;
}

int rc_bcm_cled_ctrl(int rgb, int cled_mode)
{
	int state_changed = 0;

	state_changed = _bcm_cled_ctrl(rgb, cled_mode);

	if(state_changed == 1){
#if defined(RPAX56)
                f_write_string("/proc/bcm_cled/activate", "0x000058a0", 0, 0);
#elif defined(RPAX58)
		eval("sw", "0xff80301c", "0x0000d8a0");
#endif
	}
	return state_changed;
}

#if defined(ET12) || defined(XT12)
#define CLED_W_NUM 3
int bcm_cled_ctrl_single_white(int rgb, int cled_mode) {

	int index = 0;
	int state_changed = 0;
	char LED_BEHAVIOR_WRITE[BCM_CLED_MODE_END][20] =
			{"0x0003e000", "0x0003d000", "0x0003e018", "0x0003e002", "0x0003e038", ""};
	char LED_BEHAVIOR_READ[BCM_CLED_MODE_END][20] =
			{"3e000\n", "3d000\n", "3e018\n", "3e002\n", "3e038\n", ""};

	bcm_cled_x_led_s led_w[CLED_W_NUM] = {
		// SIDE1 - cled23
		{"0xff803190", "0x00000000",
		 "0xff803394", "0x00000000",
		 "0xff803398", "0x00000000",
		 "0xff80339c", "0x00000000"},
		// SIDE2 - cled24
		{"0xff8031a0", "0x00000000",
		 "0xff8033a4", "0x00000000",
		 "0xff8033a8", "0x00000000",
		 "0xff8033ac", "0x00000000"},
		// SIDE3 - cled22
		{"0xff803180", "0x00000000",
		 "0xff803380", "0x00000000",
		 "0xff803380", "0x00000000",
		 "0xff803380", "0x00000000"}
	};

	if(rgb != BCM_CLED_WHITE && rgb != BCM_CLED_OFF)
		_dprintf("unsupport color [%d] for single while cled !!!\n", rgb);

	for(index = 0; index < CLED_W_NUM; index++) {
		read_cled_value_x(&led_w[index]);
		if(rgb == BCM_CLED_WHITE) {
			if(is_cled_value_correct_x(&led_w[index], LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_w[index], LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
		}
		else {
			if(is_cled_value_correct_x(&led_w[index], "0\n", "a34a32\n", "c34\n", "0\n") == 0){
				set_cled_value_x(&led_w[index], "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
				state_changed = 1;
			}
		}
	}

	if(state_changed)
#if defined(RPAX58)
		 eval("sw", "0xff80301c", "0x0000d8a0");
#else
		 eval("sw", "0xff80301c", "0x01C00000");
#endif
}
#endif
#endif

uint32_t get_gpio(uint32_t gpio)
{
#ifdef HND_ROUTER
	int board_fp = open("/dev/brcmboard", O_RDWR);
	int active_low = _gpio_active_low(gpio & 0xff);
	BOARD_IOCTL_PARMS ioctl_parms = {0};

	if (board_fp <= 0) {
		printf("Open /dev/brcmboard failed!\n");
		return -1;
	}
	if (active_low < 0) {
		printf("invalid gpionr!get(%d)\n", gpio);
		dump_ledtable();
		close(board_fp);
		return -1;
	}

	ioctl_parms.strLen = gpio | (active_low ? BP_ACTIVE_LOW : 0);

	if (ioctl(board_fp, BOARD_IOCTL_GET_GPIO, &ioctl_parms) < 0)
		printf("\nhnd iotcl fail!\n");
	//printf("\nhnd get_gpio: %04x\n", ioctl_parms.offset);

	close(board_fp);
	return ioctl_parms.offset;
#else
	uint32_t bit_value;
	uint32_t bit_mask;

	bit_mask = 1 << gpio;
	bit_value = gpio_read()&bit_mask;

	return bit_value == 0 ? 0 : 1;
#endif
}

#if defined(RTCONFIG_HND_ROUTER_AX_6710) || defined(BCM6750) || defined(BCM6756) || defined(GTAX6000) || defined(RTAX86U_PRO) || defined(BCM6855) || defined(RTAX88U_PRO)
uint32_t get_gpio2(uint32_t gpio)
{
	int board_fp = open("/dev/brcmboard", O_RDWR);
	BOARD_IOCTL_PARMS ioctl_parms = {0};

	if (board_fp <= 0) {
		printf("Open /dev/brcmboard failed!\n");
		return -1;
	}

	ioctl_parms.strLen = gpio;

	if (ioctl(board_fp, BOARD_IOCTL_GET_GPIO, &ioctl_parms) < 0)
		printf("\nhnd iotcl fail!\n");
	//printf("\nhnd get_gpio: %04x\n", ioctl_parms.offset);

	close(board_fp);
	return ioctl_parms.offset;
}
#endif

uint32_t set_gpio(uint32_t gpio, uint32_t value)
{
#ifdef HND_ROUTER
#ifndef LEGACY_LED
	char ledpath[48];
	int active_low = _gpio_active_low(gpio & 0xff);
	int ledfd;

	if (active_low < 0) {
		printf("invalid gpionr!set(%d)\n", gpio);
		dump_ledtable();
		return -1;
	}
#if defined(RTCONFIG_HND_ROUTER_AX_6756)
	sprintf(ledpath, "/sys/class/leds/sw_parallel_led_%d/brightness", gpio);
	if (!f_exists(ledpath))
		sprintf(ledpath, "/sys/class/leds/led_gpio_%d/brightness", gpio);
	else if (!f_exists(ledpath))
		sprintf(ledpath, "/sys/class/leds/sw_led_%d/brightness", gpio);
#else
	sprintf(ledpath, "/sys/class/leds/%d/brightness", gpio);
#endif
	ledfd = open(ledpath, O_RDWR);
	if (ledfd <=0 ) {
		printf("\nopen ledpath %s failed !\n", ledpath);
		return -1;
	}
#if defined(RTAX95Q) || defined(RTAXE95Q)
	write(ledfd, active_low?(!value?"0":"255"):(!value?"255":"0"), active_low?(!value?1:3):(!value?3:1));
#elif defined(RTCONFIG_HND_ROUTER_AX_6756) && !defined(BCM4912)
	write(ledfd, active_low?(!value?"255":"0"):(!value?"255":"0"), active_low?(!value?3:1):(!value?3:1));
#else
	write(ledfd, active_low?(!value?"255":"0"):(!value?"0":"255"), active_low?(!value?3:1):(!value?1:3));
#endif
	close(ledfd);
	return 0;
#else
	int board_fp = open("/dev/brcmboard", O_RDWR);
	int active_low = _gpio_active_low(gpio & 0xff);
	BOARD_IOCTL_PARMS ioctl_parms = {0};

	if (board_fp <= 0) {
		printf("Open /dev/brcmboard failed!\n");
		return -1;
	}
	if (active_low < 0) {
		printf("invalid gpionr!\n");
		dump_ledtable();
		close(board_fp);
		return -1;
	}

	ioctl_parms.strLen = gpio & 0xff | (active_low ? BP_ACTIVE_LOW : 0);
	ioctl_parms.offset = (active_low?!value:value) & 0x3;

	if (ioctl(board_fp, BOARD_IOCTL_SET_GPIO, &ioctl_parms) < 0)
		printf("\nhnd iotcl fail!\n");

	close(board_fp);
	return 0;
#endif
#else // HND_ROUTER
	gpio_write(gpio, value);
#endif
	return 0;
}

#ifdef RTCONFIG_BCMFA
int get_fa_rev(void)
{
	int fd, ret;
	unsigned int rev;
	struct ifreq ifr;
	et_var_t var;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) goto skip;

	rev = 0;
	var.set = 0;
	var.cmd = IOV_FA_REV;
	var.buf = &rev;
	var.len = sizeof(rev);

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, WAN_IF_ETH);
	ifr.ifr_data = (caddr_t) &var;

	ret = ioctl(fd, SIOCSETGETVAR, (caddr_t)&ifr);
	close(fd);
	if (ret < 0)
		goto skip;

	return rev;

skip:
	return 0;
}

int get_fa_dump(void)
{
	int fd, rev, ret;
	struct ifreq ifr;
	et_var_t var;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) goto skip;

	rev = 0;
	var.set = 0;
	var.cmd = IOV_FA_DUMP;
	var.buf = &rev;
	var.len = sizeof(rev);

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, WAN_IF_ETH);
	ifr.ifr_data = (caddr_t) &var;

	ret = ioctl(fd, SIOCSETGETVAR, (caddr_t)&ifr);
	close(fd);
	if (ret < 0)
		goto skip;

	return rev;

skip:
	return 0;
}

#endif

int get_switch_model(void)
{
#ifdef BCM5301X
	return SWITCH_BCM5301x;
#elif defined(HND_ROUTER)
	return SWITCH_BCM5301x_EX;
#else
	int fd, devid, ret;
	struct ifreq ifr;
	et_var_t var;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) goto skip;

	devid = 0;
	var.set = 0;
	var.cmd = IOV_ET_ROBO_DEVID;
	var.buf = &devid;
	var.len = sizeof(devid);

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, WAN_IF_ETH); // is it always the same?
	ifr.ifr_data = (caddr_t) &var;

	ret = ioctl(fd, SIOCSETGETVAR, (caddr_t)&ifr);
	close(fd);
	if (ret < 0)
		goto skip;
	if (devid == 0x25)
		return SWITCH_BCM5325;
	else if (devid == 0x3115)
		return SWITCH_BCM53115;
	else if (devid == 0x3125)
		return SWITCH_BCM53125;
	else if ((devid & 0xfffffff0) == 0x53010)
		return SWITCH_BCM5301x;

skip:
	return SWITCH_UNKNOWN;
#endif
}

int robo_ioctl(int fd, int write, int page, int reg, uint32_t *value)
{
	static int __ioctl_args[2] = { SIOCGETCROBORD, SIOCSETCROBOWR };
	struct ifreq ifr;
	int ret, vecarg[4];

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, WAN_IF_ETH); // is it always the same?
	ifr.ifr_data = (caddr_t) vecarg;

	vecarg[0] = (page << 16) | reg;
#if defined(BCM5301X) || defined(RTAC1200G) || defined(RTAC1200GP)
	vecarg[1] = 0;
	vecarg[2] = *value;
#else
	vecarg[1] = *value;
#endif
	ret = ioctl(fd, __ioctl_args[write], (caddr_t)&ifr);

#if defined(BCM5301X) || defined(RTAC1200G) || defined(RTAC1200GP)
	*value = vecarg[2];
#else
	*value = vecarg[1];
#endif

	return ret;
}

int robo_ioctl2(int fd, int write, int page, int reg, uint64_t *value, uint32_t len)
{
	static int __ioctl_args[2] = { SIOCGETCROBORD, SIOCSETCROBOWR };
	struct ifreq ifr;
	int ret;
	uint32_t vecarg[4] = {0};

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, WAN_IF_ETH); // is it always the same?
	ifr.ifr_data = (caddr_t) vecarg  ;

	vecarg[0] = (page << 16) | reg;
#if defined(BCM5301X) || defined(RTAC1200G) || defined(RTAC1200GP)
	vecarg[1] = len;
	vecarg[2] = *value;
#else
	vecarg[1] = *value;
#endif
	ret = ioctl(fd, __ioctl_args[write], (caddr_t)&ifr);

#if defined(BCM5301X) || defined(RTAC1200G) || defined(RTAC1200GP)
	*value = vecarg[3];
	*value = ((*value)<<32) | vecarg[2];
	//_dprintf("pr Data: %08x %08x %08x %08x\n", vecarg[3], vecarg[2], vecarg[1], vecarg[0]);
#else
	*value = vecarg[1];
#endif

	return ret;
}

int phy_ioctl(int fd, int write, int phy, int reg, uint32_t *value)
{
#ifndef BCM5301X
#if defined(HND_ROUTER)
	return hnd_ethswctl(REGACCESS, 0x1000 | phy << 8, 2, 1, *value);
#endif
	static int __ioctl_args[2] = { SIOCGETCPHYRD2, SIOCSETCPHYWR2 };
	struct ifreq ifr;
	int ret, vecarg[2];

#if defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56U) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(CTAX56_XD4) || defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63)
	return 1;
#endif
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, WAN_IF_ETH); // is it always the same?
	ifr.ifr_data = (caddr_t) vecarg;

	vecarg[0] = (phy << 16) | reg;
	vecarg[1] = *value;
	ret = ioctl(fd, __ioctl_args[write], (caddr_t)&ifr);

	*value = vecarg[1];

	return ret;
#else
	return robo_ioctl(fd, write, 0x10 + phy, reg, value);
#endif
}

// !0: connected
//  0: disconnected
uint32_t get_phy_status(uint32_t portmask)
{
	int fd, model;
	uint32_t value, mask = 0;
#ifndef BCM5301X
	int i;
#endif

	model = get_switch();
	if (model == SWITCH_UNKNOWN) return 0;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) return 0;

	switch (model) {
#ifndef BCM5301X
	case SWITCH_BCM53125:
#ifdef RTCONFIG_LANWAN_LED
		/* N15U can't read link status from phy sometimes */
		if (get_model() == MODEL_RTN15U)
			goto case_SWITCH_ROBORD;
		/* fall through */
#endif
	case SWITCH_BCM53115:
	case SWITCH_BCM5325:
		for (i = 0; i < 5 && (portmask >> i); i++) {
			if ((portmask & (1U << i)) == 0)
				continue;

			if (phy_ioctl(fd, 0, i, 0x01, &value) < 0)
				continue;
			/* link is down, but negotiation has started
			 * read register again, use previous value, if failed */
			if ((value & 0x22) == 0x20)
				phy_ioctl(fd, 0, i, 0x01, &value);

			if (value & (1U << 2))
				mask |= (1U << i);
		}
		break;
#ifdef RTCONFIG_LANWAN_LED
	case_SWITCH_ROBORD:
		/* fall through */
#endif
#endif
	case SWITCH_BCM5301x:
		if (robo_ioctl(fd, 0, 0x01, 0x00, &value) < 0)
			_dprintf("et ioctl SIOCGETCROBORD failed!\n");
		mask = value & portmask & 0x1f;
		break;
	}
	close(fd);

	//_dprintf("# get_phy_status %x %x\n", mask, portmask);

	return mask;
}

// 2bit per port (0-4(5)*2 shift)
// 0: 10 Mbps
// 1: 100 Mbps
// 2: 1000 Mbps
uint32_t get_phy_speed(uint32_t portmask)
{
	int fd, model;
	uint32_t value, mask = 0;

	model = get_switch();
	if (model == SWITCH_UNKNOWN) return 0;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) return 0;

	if (robo_ioctl(fd, 0, 0x01, 0x04, &value) < 0)
		value = 0;
	close(fd);

	switch (model) {
#ifndef BCM5301X
	case SWITCH_BCM5325:
		/* 5325E/535x, 1bit: 0=10 Mbps, 1=100Mbps */
		for (mask = 0; value & 0x1f; value >>= 1) {
			mask |= (value & 0x01);
			mask <<= 2;
		}
		swapportstatus(mask);
		break;
	case SWITCH_BCM53115:
	case SWITCH_BCM53125:
		/* fall through */
#endif
	case SWITCH_BCM5301x:
		/* 5301x/53115/53125, 2bit:00=10 Mbps,01=100Mbps,10=1000Mbps */
		mask = value & portmask & 0x3ff;
		break;
	}

	//_dprintf("get_phy_speed %x %x\n", vecarg[1], portmask);

	return mask;
}

// !0: full
//  0: half
uint32_t get_phy_duplex(uint32_t portmask)
{
	int fd, model;
	uint32_t value, mask = 0;

	model = get_switch();
	if (model == SWITCH_UNKNOWN) return 0;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) return 0;

	if (robo_ioctl(fd, 0, 0x01, 0x08, &value) < 0)
		value = 0;
	close(fd);

	switch (model) {
#ifndef BCM5301X
	case SWITCH_BCM5325:
		/* 5325E/535x, 1bit: 0=10 Mbps, 1=100Mbps */
		for (mask = 0; value & 0x1f; value >>= 1) {
			mask |= (value & 0x01);
			mask <<= 2;
		}
		swapportstatus(mask);
		break;
	case SWITCH_BCM53115:
	case SWITCH_BCM53125:
		/* fall through */
#endif
	case SWITCH_BCM5301x:
		/* 5301x/53115/53125, 2bit:00=10 Mbps,01=100Mbps,10=1000Mbps */
		mask = value & portmask & 0x3ff;
		break;
	}

	//_dprintf("get_phy_duplex portmask=%x value=%x\n", portmask, value);

	return mask;
}

// 2bit per port (0-4(5)*2 shift)
uint64_t get_phy_mib(int port, char *type)
{
	int fd, /*model, */addr_cnt = 0, i = 0;
	uint64_t buf = 0, value = 0;
	unsigned int addr[8] = {0};

#if 0
	model = get_switch();
	if (model == SWITCH_UNKNOWN) return 0;d
#endif

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) return 0;

	if (!strcmp(type, "tx_bytes")) {
		addr[addr_cnt++] = REG_OFFSET_TX_BYTES;
	}
	else if (!strcmp(type, "rx_bytes")) {
		addr[addr_cnt++] = REG_OFFSET_RX_BYTES;
	}
	else if (!strcmp(type, "tx_packets")) {
		addr[addr_cnt++] = REG_OFFSET_TX_BROADCAST_PACKETS;
		addr[addr_cnt++] = REG_OFFSET_TX_MULTICAST_PACKETS;
		addr[addr_cnt++] = REG_OFFSET_TX_UNICAST_PACKETS;
	}
	else if (!strcmp(type, "rx_packets")) {
		addr[addr_cnt++] = REG_OFFSET_RX_UNICAST_PACKETS;
		addr[addr_cnt++] = REG_OFFSET_RX_MULTICAST_PACKETS;
		addr[addr_cnt++] = REG_OFFSET_RX_BROADCAST_PACKETS;
	}
	else if (!strcmp(type, "rx_crc_errors")) {
		addr[addr_cnt++] = REG_OFFSET_RX_FCS_ERROR;
	}

	for (i = 0; i < addr_cnt; i++) {
		buf = 0;
		if (robo_ioctl2(fd, 0, PAGE_MIB_BASE+port, addr[i], &buf, 8) < 0)
			buf = 0;
		//_dprintf("buf=%llu, buf2=%llu\n", le64_to_cpu(buf));
		value += buf;
	}
	close(fd);
#if 0
	switch (model) {
#ifndef BCM5301X
	case SWITCH_BCM5325:
		/* 5325E/535x, 1bit: 0=10 Mbps, 1=100Mbps */
		for (mask = 0; value & 0x1f; value >>= 1) {
			mask |= (value & 0x01);
			mask <<= 2;
		}
		swapportstatus(mask);
		break;
	case SWITCH_BCM53115:
	case SWITCH_BCM53125:
		/* fall through */
#endif
	case SWITCH_BCM5301x:
		/* 5301x/53115/53125, 2bit:00=10 Mbps,01=100Mbps,10=1000Mbps */
		mask = value & portmask & 0x3ff;
		break;
	}

	//_dprintf("get_phy_speed %x %x\n", vecarg[1], portmask);
#endif
	return value;
}

#if defined(RTCONFIG_EXT_BCM53134)
uint32_t set_ex53134_ctrl(uint32_t portmask, int ctrl)
{
	int i=0;
	uint32_t value;

#if defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56U) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(CTAX56_XD4) || defined(TUFAX3000_V2) || defined(RTAXE7800)
	return 1;
#endif
	for (i = 0; i < 4 && (portmask >> i); i++) {
		if ((portmask & (1U << i)) == 0)
			continue;

		value = 0x1140;
		value |= ctrl ? 0 : 0x0800;

		hnd_ethswctl(PMDIOACCESS, 0x1000 | i << 8, 2, 1, value);
	}

	return 0;
}
#endif

uint32_t set_phy_ctrl(uint32_t portmask, int ctrl)
{
	int fd, i, model;
	uint32_t value;

#if defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56U) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(CTAX56_XD4) || defined(RTAX55) || defined(RTAX1800) || defined(RTAX58U_V2) || defined(RTAX3000N) || defined(BR63)
	return 1;
#endif
	model = get_switch();
	if (model == SWITCH_UNKNOWN) return 0;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) return 0;

	for (i = 0; i < 5 && (portmask >> i); i++) {
		if ((portmask & (1U << i)) == 0)
			continue;

		switch (model) {
#ifndef BCM5301X
		case SWITCH_BCM5325:
			if (phy_ioctl(fd, 0, i, 0x1e, &value) < 0)
				value = 0;
			value &= 0x0007;
			value |= ctrl ? 0 : 0x0008;
			phy_ioctl(fd, 1, i, 0x1e, &value);
			value = 0x3300;
			break;
		case SWITCH_BCM53115:
		case SWITCH_BCM53125:
			/* fall through */
#endif
		case SWITCH_BCM5301x_EX:
			value = 0x1140;
			value |= ctrl ? 0 : 0x0800;
			break;
		case SWITCH_BCM5301x:
			value = 0x1340;
			value |= ctrl ? 0 : 0x0800;
			break;
		default:
			continue;
		}

		/* issue write */
		phy_ioctl(fd, 1, i, 0, &value);
	}

	close(fd);

	return 0;
}

#define IMAGE_HEADER "HDR0"
#define MAX_VERSION_LEN 64
#define MAX_PID_LEN 12
#define MAX_HW_COUNT 4

/*
 * 0: illegal image
 * 1: legal image
 */
int
check_crc(char *fname)
{
	FILE *fp;
	int ret = 1;
	int first_read = 1;
	unsigned int len, count;

	struct trx_header trx;
	uint32 crc;
	static uint32 buf[16*1024];

	fp = fopen(fname, "r");
	if (fp == NULL)
	{
		_dprintf("Open trx fail!!!\n");
		return 0;
	}

	/* Read header */
	ret = fread((unsigned char *) &trx, 1, sizeof(struct trx_header), fp);
	if (ret != sizeof(struct trx_header)) {
		ret = 0;
		_dprintf("read header error!!!\n");
		goto done;
	}

	/* Checksum over header */
	crc = hndcrc32((uint8 *) &trx.flag_version,
		       sizeof(struct trx_header) - OFFSETOF(struct trx_header, flag_version),
		       CRC32_INIT_VALUE);

	for (len = ltoh32(trx.len) - sizeof(struct trx_header); len; len -= count) {
		if (first_read) {
			count = MIN(len, sizeof(buf) - sizeof(struct trx_header));
			first_read = 0;
		} else
			count = MIN(len, sizeof(buf));

		/* Read data */
		ret = fread((unsigned char *) &buf, 1, count, fp);
		if (ret != count) {
			ret = 0;
			_dprintf("read error!\n");
			goto done;
		}

		/* Checksum over data */
		crc = hndcrc32((uint8 *) &buf, count, crc);
	}
	/* Verify checksum */
	//_dprintf("checksum: %u ? %u\n", ltoh32(trx.crc32), crc);
	if (ltoh32(trx.crc32) != crc) {
		ret = 0;
		goto done;
	}

done:
	fclose(fp);

	return ret;
}

/*
 * 0: illegal image
 * 1: legal image
 */

int check_imageheader(char *buf, long *filelen)
{
	long aligned;

#ifdef HND_ROUTER
	return 1;
#endif
	if (strncmp(buf, IMAGE_HEADER, sizeof(IMAGE_HEADER) - 1) == 0)
	{
		memcpy(&aligned, buf + sizeof(IMAGE_HEADER) - 1, sizeof(aligned));
		*filelen = aligned;
#ifdef RTCONFIG_DSL_TCLINUX
		*filelen+=0x790000;
#endif
		_dprintf("image len: %x\n", aligned);
		return 1;
	}
	else return 0;
}

#ifdef RTCONFIG_QTN
char *wl_vifname_qtn(int unit, int subunit)
{
	static char tmp[128];

	if ((subunit > 0) && (subunit < 4))
	{
		sprintf(tmp, "wifi%d", subunit);
		return strdup(tmp);
	}
	else
		return strdup("");
}
#endif

int get_radio(int unit, int subunit)
{
	int n = 0;

	//_dprintf("get radio %x %x %s\n", unit, subunit, nvram_safe_get(wl_nvname("ifname", unit, subunit)));

#ifdef RTCONFIG_QTN
	int ret;
	char interface_status = 0;

	if (unit)
	{
		if (!rpc_qtn_ready())
			return -1;

		if (subunit > 0)
		{
			ret = qcsapi_interface_get_status(wl_vifname_qtn(unit, subunit), &interface_status);
//			if (ret < 0)
//				dbG("Qcsapi qcsapi_interface_get_status %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);

			return interface_status;
		}
		else
		{
			ret = qcsapi_wifi_rfstatus((qcsapi_unsigned_int *) &n);
//			if (ret < 0)
//				dbG("Qcsapi qcsapi_wifi_rfstatus %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);

			return n;
		}
	}
	else
#endif

	return (wl_ioctl(nvram_safe_get(wl_nvname("ifname", unit, subunit)), WLC_GET_RADIO, &n, sizeof(n)) == 0) &&
		!(n & (WL_RADIO_SW_DISABLE | WL_RADIO_HW_DISABLE));
}

void set_radio(int on, int unit, int subunit)
{
	uint32 n;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";

#ifdef RTCONFIG_QTN
	if (unit) {
		if (!rpc_qtn_ready())
			return;

		rpc_set_radio(unit, subunit, on);

		return;
	}
#endif
	//_dprintf("set radio %x %x %x %s\n", on, unit, subunit, nvram_safe_get(wl_nvname("ifname", unit, subunit)));

	if (subunit > 0)
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
	else
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	//if (nvram_match(strcat_r(prefix, "radio", tmp), "0")) return;

#if defined(RTAC66U) || defined(BCM4352) || defined(RTAX82U)
	if ((unit == 1) & (subunit < 1)) {
#if defined(RTAX82U) && !defined(RTCONFIG_BCM_MFG)
		if (!nvram_get_int("LED_order"))
			led_control(LED_5G, on ? LED_ON : LED_OFF);
#else
		if (on) {
#ifndef RTCONFIG_LED_BTN
			if (!(sw_mode()==SW_MODE_AP && nvram_get_int("wlc_psta")==1 && nvram_get_int("wlc_band")==0)) {
				nvram_set("led_5g", "1");
				led_control(LED_5G, LED_ON);
			}
#else
			nvram_set("led_5g", "1");
			if (nvram_get_int("AllLED"))
				led_control(LED_5G, LED_ON);
#endif
		}
		else {
			nvram_set("led_5g", "0");
			led_control(LED_5G, LED_OFF);
		}
#endif
	}
#endif

	if (subunit > 0) {
		sprintf(tmp, "%d", subunit);
		if (on) eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", unit, 0)), "bss", "-C", tmp, "up");
		else eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", unit, 0)), "bss", "-C", tmp, "down");

		return;
	}

#ifndef WL_BSS_INFO_VERSION
#error WL_BSS_INFO_VERSION
#endif

#if WL_BSS_INFO_VERSION >= 108
	n = on ? (WL_RADIO_SW_DISABLE << 16) : ((WL_RADIO_SW_DISABLE << 16) | 1);
	wl_ioctl(nvram_safe_get(wl_nvname("ifname", unit, subunit)), WLC_SET_RADIO, &n, sizeof(n));
	if (!on) {
		//led(LED_WLAN, 0);
		//led(LED_DIAG, 0);
	}
#else
	n = on ? 0 : WL_RADIO_SW_DISABLE;
	wl_ioctl(nvram_safe_get(wl_nvname("ifname", unit, subunit)), WLC_SET_RADIO, &n, sizeof(n));
	if (!on) {
		//led(LED_DIAG, 0);
	}
#endif
}

/* Return nvram variable name, e.g. et0macaddr, which is used to repented as LAN MAC.
 * @return:
 */
char *get_lan_mac_name(void)
{
#ifdef RTCONFIG_BCMARM
#ifdef RTCONFIG_GMAC3
	char *et2macaddr;
	if (nvram_get_int("gmac3_enable") && (et2macaddr = nvram_get("et2macaddr")) &&
		*et2macaddr && strcmp(et2macaddr, "00:00:00:00:00:00") != 0) {
		return "et2macaddr";
	}
#endif

	switch(get_model()) {
		case MODEL_RTAC87U:
		case MODEL_RTAC88U:
		case MODEL_RTAC5300:
			return "et1macaddr";
	}
#endif
	return "et0macaddr";
}

/* Return nvram variable name, e.g. et1macaddr, which is used to repented as WAN MAC.
 * @return:
 */
char *get_wan_mac_name(void)
{
#ifdef RTCONFIG_BCMARM
#ifdef RTCONFIG_GMAC3
	char *et2macaddr;
	if (nvram_get_int("gmac3_enable") && (et2macaddr = nvram_get("et2macaddr")) &&
		*et2macaddr && strcmp(et2macaddr, "00:00:00:00:00:00") != 0) {
		return "et2macaddr";
	}
#endif
	switch(get_model()) {
		case MODEL_RTAC87U:
		case MODEL_RTAC88U:
		case MODEL_RTAC5300:
			return "et1macaddr";
	}
#endif
	return "et0macaddr";
}

static char* mac_str_toupper(char *str)
{
	char *c;
	static char buf[18];

	strncpy(buf, str, sizeof(buf) - 1);
	for (c = buf; *c; ++c)
		*c = toupper(*c);

	return buf;
}

char *get_label_mac()
{
	return mac_str_toupper(get_2g_hwaddr());
}

char *get_lan_hwaddr(void)
{
	return mac_str_toupper(nvram_safe_get(get_lan_mac_name()));
}

char *get_2g_hwaddr(void)
{
	return mac_str_toupper(nvram_safe_get(get_lan_mac_name()));
}

char *get_wan_hwaddr(void)
{
	return mac_str_toupper(nvram_safe_get(get_wan_mac_name()));
}

char *get_wlifname(int unit, int subunit, int subunit_x, char *buf)
{
	char wifnv[12];

	if (!subunit)
	{
		snprintf(wifnv, sizeof(wifnv), "wl%d_ifname", unit);
		sprintf(buf, nvram_safe_get(wifnv));
	}
	else
		sprintf(buf, "wl%d.%d", unit, subunit);

	return buf;
}

char *wl_ifname(int unit, int subunit, char *buf)
{
	return get_wlifname(unit, subunit, -1, buf);
}

/**
 * Generate VAP interface name of wlX.Y for Guest network, Free Wi-Fi, and Facebook Wi-Fi
 * @x:		X of wlX.Y, aka unit
 * @y:		Y of wlX.Y
 * @buf:	Pointer to buffer of VAP interface name. Must greater than or equal to IFNAMSIZ
 * @return:
 */
char *get_wlxy_ifname(int x, int y, char *buf)
{
#ifdef RTAC87U
	if (get_model() == MODEL_RTAC87U && (x == 1)) {
		if(y == 1) strcpy(buf, "vlan4000");
		if(y == 2) strcpy(buf, "vlan4001");
		if(y == 3) strcpy(buf, "vlan4002");
		return buf;
	}
#endif
	return get_wlifname(x, y, y, buf);
}

#define	IW_MAX_FREQUENCIES	32
static bool g_swap = FALSE;
#define htod32(i) (g_swap?bcmswap32(i):(uint32)(i))
#define dtoh32(i) (g_swap?bcmswap32(i):(uint32)(i))
int get_channel_list_via_driver(int unit, char *buffer, int len)
{
	int channels[MAXCHANNEL+1];
	wl_uint32_list_t *list = (wl_uint32_list_t *) channels;
	char tmp[256], prefix[] = "wlXXXXXXXXXX_";
	char ifname[IFNAMSIZ] = { 0 };
	int i;
	uint ch;

	if (buffer == NULL)
		return -1;

	memset(buffer, 0, len);
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	strlcpy(ifname, nvram_safe_get(strcat_r(prefix, "ifname", tmp)), sizeof(ifname));

	memset(channels, 0, sizeof(channels));
	list->count = htod32(MAXCHANNEL);
	if (wl_ioctl(ifname, WLC_GET_VALID_CHANNELS , channels, sizeof(channels)) < 0) {
		dbg("error doing WLC_GET_VALID_CHANNELS\n");
		return -1;
	}

	if (dtoh32(list->count) == 0) {
		dbg("No valid channels\n");
		return 0;
	}

	for (i = 0; i < dtoh32(list->count) && i < IW_MAX_FREQUENCIES; i++) {
		ch = dtoh32(list->element[i]);
		if (i == 0) {
			snprintf(buffer, len, "%u", ch);
		}
		else {
			snprintf(tmp, sizeof(tmp), ",%u", ch);
			strlcat(buffer, tmp, len);
		}
	}

	return strlen(buffer);
}

int get_channel_list_via_country(int unit, const char *country_code, char *buffer, int len)
{
	//TODO:
	return 0;
}

#ifdef RTCONFIG_BONDING
#ifdef RTCONFIG_HND_ROUTER_AX
/*
	LAN bond_if = bond0
	WAN bond_if = bond1
*/
int get_bonding_speed(char *bond_if)
{
	char confbuf[64] = {0};
	char buf[32] = {0};
	int speed;

	snprintf(confbuf, sizeof(confbuf),
			"/sys/class/net/%s/speed", bond_if);
	f_read_string(confbuf, buf, sizeof(buf));
	speed = safe_atoi(buf);

	return (speed <= 0) ? 0 : speed;
}

/*
	WAN port: port = 0
	LAN4: port = 4
*/
int get_bonding_port_status(int port)
{
#ifdef RTCONFIG_BONDING_WAN
	int port_status = 0;
	int ret;
#if !defined(RTCONFIG_HND_ROUTER_AX_6710) && !defined(RTCONFIG_HND_ROUTER_AX_675X) && !defined(RTCONFIG_HND_ROUTER_AX_6756)
	int extra_p0=0;
	unsigned int regv=0, pmdv=0, regv2=0, pmdv2=0;
#endif
#if defined(RTCONFIG_EXT_BCM53134) && !defined(TUFAX3000_V2) && !defined(RTAXE7800) /* RT-AX88U */
	int lan_ports=4;
	int ports[lan_ports+1];
	ports[0]=7; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
#elif defined(RTAX92U)
	int lan_ports=4;
	int ports[lan_ports+1];
	/* 7 3 2 1 0	W0 L1 L2 L3 L4 */
	ports[0]=7; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
#elif defined(RTAX95Q) || defined(RTAXE95Q)
	int lan_ports=3;
	int ports[lan_ports+1];
	/* 7 3 2 1 0	W0 L1 L2 L3 L4 */
	ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3;
#elif defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(ET8PRO) || defined(ET8_V2)
	int lan_ports=3;
	int ports[lan_ports+1];
	/* 7 3 2 1 0	W0 L1 L2 L3 L4 */
	ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3;
#elif defined(RTAX56_XD4)
	int lan_ports=1;

	if(nvram_match("HwId", "A") || nvram_match("HwId", "C")){
		lan_ports = 1;
	} else {
		lan_ports = 0;
	}
	int ports[lan_ports+1];
	if(nvram_match("HwId", "A") || nvram_match("HwId", "C")){
		/* 7 3	W0 L1 */
		ports[0]=7; ports[1]=3;
	} else {
		/* 7 W0 */
		ports[0]=7;
	}
#elif defined(XD4PRO)
	int lan_ports=1;

	int ports[lan_ports+1];
	/* 7 3	W0 L1 */
	ports[0]=7; ports[1]=3;
#elif defined(CTAX56_XD4)
	int lan_ports=1;

	int ports[lan_ports+1];
	/* 7 3	W0 L1 */
	ports[0]=7; ports[1]=3;
#elif defined(RTAX58U) || defined(TUFAX3000) || defined(TUFAX5400) || defined(RTAX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(RTAX5400)
	int lan_ports=4;
	int ports[lan_ports+1];
	/* 4 3 2 1 0	W0 L1 L2 L3 L4 */
	ports[0]=4; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
#elif defined(TUFAX3000_V2)
	int lan_ports=4;
	int ports[lan_ports+1];
	/* 0 1 2 3    W0 L1 L2 L3 */
	ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3;
#elif defined(TUFAX3000_V2)
	int lan_ports=4;
	int ports[lan_ports+1];
	/* 0 1 2 3 4    W0 L1 L2 L3 L4 */
	ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3; ports[4]=4;
#elif defined(RTAXE7800)
	int lan_ports=4;
	int ports[lan_ports+1];
	if (!nvram_get_int("wans_extwan"))
	{
	/* 0 1 2 3 4    W0 L1 L2 L3 L4 */
	ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3; ports[4]=4;
	}
	else
	{
	/* 1 0 2 3 4    W0 L1 L2 L3 L4 */
	ports[0]=1; ports[1]=0; ports[2]=2; ports[3]=3; ports[4]=4;
	}
#elif defined(GT10)
	int lan_ports=3;
	int ports[lan_ports+1];
	if (!nvram_get_int("wans_extwan"))
	{
	/* 0 1 2 3    W0 L1 L2 L3 */
	ports[0]=0; ports[1]=1; ports[2]=2; ports[3]=3;
	}
	else
	{
	/* 1 0 2 3    W0 L1 L2 L3 */
	ports[0]=1; ports[1]=0; ports[2]=2; ports[3]=3;
	}
#elif defined(RTAX82_XD6) || defined(XD6_V2)
	int lan_ports=3;
	int ports[lan_ports+1];
	/* 4 2 1 0    W0 L1 L2 L3 */
	ports[0]=4; ports[1]=2; ports[2]=1; ports[3]=0;
#elif defined(RTAX82_XD6S)
	int lan_ports=1;
	int ports[lan_ports+1];
	/* 1 0    W0 L1 */
	ports[0]=1; ports[1]=0;;
#elif defined(RTAX56U)
	int lan_ports=4;
	int ports[lan_ports+1];
	/* 4 3 2 1 0	W0 L1 L2 L3 L4 */
	ports[0]=4; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
#elif defined(RTAX86U) || defined(GTAXE11000)
	int lan_ports = 5;
	char *ports[lan_ports+1];
	/* 7 4 3 2 1 0	L5(2.5G) W0 L1 L2 L3 L4 */
	/* eth5 eth0 eth4 eth3 eth2 eth1 */
	ports[0] = "eth0"; ports[1] = "eth4"; ports[2] = "eth3"; ports[3] = "eth2"; ports[4] = "eth1"; ports[5] = "eth5";
#elif defined(RTAX68U) || defined(RTAC68U_V4)
	int lan_ports = 4;
	char *ports[lan_ports+1];
	/* 4 3 2 1 0	W0 L1 L2 L3 L4 */
	/* eth0 eth4 eth3 eth2 eth1 */
	ports[0] = "eth0"; ports[1] = "eth4"; ports[2] = "eth3"; ports[3] = "eth2"; ports[4] = "eth1";
#elif defined(GTAX6000) || defined(RTAX86U_PRO) || defined(RTAX88U_PRO)
	/* 6 5 3 2 1 0  L5(2.5G) W0 L1 L2 L3 L4 */
	char *ports[6] = { "eth5", "eth0", "eth1", "eth2", "eth3", "eth4" };
#elif defined(GTAX11000_PRO)
	char *ports[6] = { "eth0", "eth1", "eth2", "eth3", "eth4", "eth5" };
#elif defined(GTAXE16000)
	char *ports[7] = { "eth0", "eth1", "eth2", "eth3", "eth4", "eth5", "eth6" };
#elif defined(ET12) || defined(XT12)
	char *ports[4] = { "eth0", "eth1", "eth2", "eth3" };
#elif defined(RTCONFIG_EXTPHY_BCM84880) /* GT-AX11000 */
	int lan_ports=5;
	int ports[lan_ports+1];
	/*
		7 4 3 2 1 0 	L5(2.5G) W0 L1 L2 L3 L4
	*/
	ports[0]=4; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
	ports[5]=7;
#endif

#if !defined(RTCONFIG_HND_ROUTER_AX_6710) && !defined(RTCONFIG_HND_ROUTER_AX_675X) && !defined(RTCONFIG_HND_ROUTER_AX_6756)
#ifdef RTCONFIG_EXT_BCM53134
	extra_p0 = S_53134;
#endif
	regv = hnd_ethswctl(REGACCESS, 0x0100, 2, 0, 0);
#ifdef RTCONFIG_EXT_BCM53134
	pmdv = hnd_ethswctl(PMDIOACCESS, 0x0100, 2, 0, 0);
#endif
	regv2 = hnd_ethswctl(REGACCESS, 0x0104, 4, 0, 0);
#ifdef RTCONFIG_EXT_BCM53134
	pmdv2 = hnd_ethswctl(PMDIOACCESS, 0x0104, 4, 0, 0);
#endif
#endif

	/* WAN port */
#if defined(RTCONFIG_HND_ROUTER_AX_6710) || defined(BCM4912) || defined(BCM6756)
	if (hnd_get_phy_status(ports[port]))				/*Disconnect*/
#elif defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(BCM6855) || defined(BCM6750)
	if (hnd_get_phy_status(ports[port]))				/*Disconnect*/
#else
	if (hnd_get_phy_status(ports[port], extra_p0, regv, pmdv))	/*Disconnect*/
#endif
	{
#if defined(RTCONFIG_HND_ROUTER_AX_6710) || defined(BCM4912) || defined(BCM6756)
		port_status = hnd_get_phy_speed(ports[port]);
#elif defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(BCM6855) || defined(BCM6750)
		port_status = hnd_get_phy_speed(ports[port]);
#else
		port_status = hnd_get_phy_speed(ports[port], extra_p0, regv2, pmdv2);
#endif
	}

/*
	port_status = 0 : disconnect
	port_status = 100 : connect and phy speed 100Mbps
	port_status = 1000 : connect and phy speed 1000Mbps
	port_status = 2500 : connect and phy speed 2500Mbps
*/
	return port_status;
#else // RTCONFIG_BONDING_WAN
	return 0;
#endif // RTCONFIG_BONDING_WAN
}
#endif // RTCONFIG_HND_ROUTER_AX
#endif // RTCONFIG_BONDING

int wl_max_no_vifs(int unit)
{
	char nv_interface[NVRAM_MAX_PARAM_LEN];
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_MEDLEN];
	char *name = NULL;
	char *next = NULL;
	int max_no_vifs = 0;
#ifdef RTCONFIG_PSR_GUEST
	char tmp[100], prefix[]="wlXXXXXXX_";
#endif
#ifdef RTCONFIG_AMAS
	int base_no_vifs = 5;
#ifdef RTCONFIG_FRONTHAUL_DWB
	base_no_vifs++;
#endif
#ifdef RTCONFIG_MSSID_PRELINK
	base_no_vifs++;
#endif
#ifdef RTCONFIG_VIF_ONBOARDING
	base_no_vifs++;
#endif
#ifdef RTCONFIG_FRONTHAUL_DBG
#ifdef GT10
	if(unit == 2)
#else
	if(!unit)
#endif
		base_no_vifs++;
#endif
#endif

#ifdef RTCONFIG_QTN
	if (unit == 1)
		return 4;
#endif

	snprintf(nv_interface, sizeof(nv_interface), "wl%d_ifname", unit);
#ifdef RTCONFIG_PSR_GUEST
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
#endif
	name = nvram_safe_get(nv_interface);
	if (!strlen(name))
	{
		sprintf(nv_interface, WL_IF_PREFIX, unit + 1);
		name = nv_interface;
	}

	if (!wl_iovar_get(name, "cap", (void *)caps, sizeof(caps))) {
		foreach(cap, caps, next) {
			if (!strcmp(cap, "mbss16"))
				max_no_vifs = 16;
			else if (!strcmp(cap, "mbss8"))
				max_no_vifs = 8;
			else if (!strcmp(cap, "mbss4"))
				max_no_vifs = 4;
		}
	}

#if defined(RTCONFIG_OWE_TRANS)
	if(wl_get_band(name) != WLC_BAND_6G)
		base_no_vifs++;
#endif

#if defined(RTCONFIG_AMAS) && (defined(RTCONFIG_FRONTHAUL_DWB) || defined(RTCONFIG_MSSID_PRELINK) || defined(RTCONFIG_FRONTHAUL_DBG))
	if (nvram_match("re_mode", "1"))
		return min(max_no_vifs, base_no_vifs);
#endif

#ifdef RTCONFIG_PSR_GUEST
#ifdef RTCONFIG_HND_ROUTER_AX
	if (is_psta(unit) || is_psr(unit)) {
#else
	if (!atoi(nvram_safe_get("ure_disable")) || is_psr(unit)) {
#endif
		if (is_psr(unit) && nvram_match(strcat_r(prefix, "psr_mbss", tmp), "1"))
			max_no_vifs = min(max_no_vifs, base_no_vifs);
		else
			max_no_vifs = 2;
	}
#endif

	return max_no_vifs;
}

int wl_get_band(char* wlif)
{
    int bandtype = WLC_BAND_AUTO;

    wl_ioctl(wlif, WLC_GET_BAND, &bandtype, sizeof(bandtype));

    return bandtype;
}

/* Check if interface is primary or not */
int wl_check_is_primary_ifce(const char *ifname)
{
	int unit = -1, subunit = -1;
	char nv_ifname[IFNAMSIZ] = {0};

	if (osifname_to_nvifname(ifname, nv_ifname, sizeof(nv_ifname)))
		return 0;

	if (get_ifname_unit(nv_ifname, &unit, &subunit) < 0)
		return 0;

	if (subunit > 0)
		return 0;

	return 1;
}

#ifdef RTCONFIG_HND_ROUTER_AX
/*
 * Set msched maxn
 */
int
wl_msched_iovar_setmaxn(char *ifname, char *iovar, char *subcmd, int bw, int val)
{
	int ret = BCME_OK;
	wl_musched_cmd_params_t *musched_params = NULL;
	uint musched_cmd_param_size = sizeof(wl_musched_cmd_params_t);

	if ((musched_params = (wl_musched_cmd_params_t *)malloc(musched_cmd_param_size)) == NULL) {
		fprintf(stderr, "Error allocating %d bytes for musched_cmd params\n",
			musched_cmd_param_size);
		ret = BCME_NOMEM;
		goto fail;
	}

	/* Init musched_params */
	memset(musched_params, 0, musched_cmd_param_size);

	WL_MUSCHED_FLAGS_BW_SET(musched_params);
	if (bw == 20)
		musched_params->bw = 0;
	else if (bw == 40)
		musched_params->bw = 1;
	else if (bw == 80)
		musched_params->bw = 2;
	else if (bw == 160)
		musched_params->bw = 3;
	else if (bw == -1)
		musched_params->bw = -1;

	musched_params->ac = -1;
	musched_params->row = -1;
	musched_params->col = -1;

	/* Init flags based on the one passed from caller */
	if (strcmp(iovar, "msched") == 0) {
		WL_MUSCHED_FLAGS_DL_SET(musched_params);
	} else if (strcmp(iovar, "umsched") == 0) {
		WL_MUSCHED_FLAGS_UL_SET(musched_params);
	}

	strncpy(musched_params->keystr, subcmd,
		MIN(strlen(subcmd), WL_MUSCHED_CMD_KEYSTR_MAXLEN));

	musched_params->vals[musched_params->num_vals++] = (int16)val;

	ret = wl_iovar_set(ifname, iovar, (void *)musched_params, musched_cmd_param_size);

fail:
	if (musched_params)
		free(musched_params);

	return ret;
}
#endif

#ifdef RTCONFIG_BCMBSD_V2

typedef struct {
	unsigned int sel_val;
	char steering[RULE_MAX][32];
	char sta_select[RULE_MAX][40];
	char if_qualify[RULE_MAX][20];
}bcmbsd_policy;

bcmbsd_policy bcmbsd_def_policy[BCMBSD_SELIF_MAX] = {
			// reserved 
			{0, {"","","",""},
			 {"","","",""},
			 {"","","",""}},
			// 2G_5G1_5G2_6G (1)
			{ 0xf, 
			 {"0 5 3 -62 0 0 0x22",
			  "0 5 3 -82 0 0 0x820",
			  "0 5 3 -82 0 0 0x420",
			  "0 5 3 -82 0 0 0x20"},
			 {"30 -62 0 0 0 1 1 0 0 0 0x122",
			  "30 -82 0 0 0 1 1 0 0 0 0x8020",
			  "30 -82 0 0 0 1 1 0 0 0 0x4020",
			  "30 -82 0 0 0 1 1 0 0 0 0x20"},
			 {"0 0x0 -100",
			  "0 0x400 -100",
			  "0 0x200 -100",
			  "0 0x0 -100"}},
			// 2G_5G1_6G (2)
			{ 0xb,
			 {"0 5 3 -62 0 0 0x22",
			  "0 5 3 -82 0 0 0x20",
			  "",
			  "0 5 3 -82 0 0 0x20"},
			 {"30 -62 0 0 0 1 1 0 0 0 0x122",
			  "30 -82 0 0 0 1 1 0 0 0 0x20",
			  "",
			  "30 -82 0 0 0 1 1 0 0 0 0x20"},
			 {"0 0x0 -100",
			  "0 0x0 -100",
			  "",
			  "0 0x0 -100"}},
			// 2G_5G1_5G2 (3:QIS)
			{ 0x7,
			 {"0 5 3 -62 0 0 0x22",
			  "0 5 3 -82 0 0 0x820",
			  "0 5 3 -82 0 0 0x420",
			  ""},
			 {"30 -62 0 0 0 1 1 0 0 0 0x122",
			  "30 -82 0 0 0 1 1 0 0 0 0x8020",
			  "30 -82 0 0 0 1 1 0 0 0 0x4020",
			  ""},
			 {"0 0x0 -100",
			  "0 0x400 -100",
			  "0 0x200 -100",
			  ""}},
			// 2G_5G1 (4)
			{ 0x3,
			 {"0 5 3 -62 0 0 0x22",
			  "0 5 3 -82 0 0 0x20",
			  "",
			  ""},
			 {"30 -62 0 0 0 1 1 0 0 0 0x122",
			  "30 -82 0 0 0 1 1 0 0 0 0x20",
			  "",
			  ""},
			 {"0 0x0 -100",
			  "0 0x0 -100",
			  "",
			  ""}},
			// 2G_5G2_6G (5)
			{ 0xd,
			 {"0 5 3 -62 0 0 0x22",
			  "",
			  "0 5 3 -82 0 0 0x20",
			  "0 5 3 -82 0 0 0x20"},
			 {"30 -62 0 0 0 1 1 0 0 0 0x122",
			  "",
			  "30 -82 0 0 0 1 1 0 0 0 0x20",
			  "30 -82 0 0 0 1 1 0 0 0 0x20"},
			 {"0 0x0 -100",
			  "",
			  "0 0x0 -100",
			  "0 0x0 -100"}},
			// 2G_6G (6) 
			{ 0x9, 
			 {"0 5 3 -62 0 0 0x22",
			  "",
			  "",
			  "0 5 3 -82 0 0 0x20"},
			 {"30 -62 0 0 0 1 1 0 0 0 0x122",
			  "",
			  "",
			  "30 -82 0 0 0 1 1 0 0 0 0x20"},
			 {"0 0x0 -100",
			  "",
			  "",
			  "0 0x0 -100"}},
			// 5G1_5G2_6G (7) 
			{ 0xe,
			 {"",
			  "0 5 3 -82 0 0 0x820",
			  "0 5 3 -82 0 0 0x420",
			  "0 5 3 -82 0 0 0x20"},
			 {"",
			  "30 -82 0 0 0 1 1 0 0 0 0x8020",
			  "30 -82 0 0 0 1 1 0 0 0 0x4020",
			  "30 -82 0 0 0 1 1 0 0 0 0x20"},
			 {"",
			  "0 0x400 -100",
			  "0 0x200 -100",
			  "0 0x0 -100"}},
			// 5G1_6G (8) 
			{ 0xa,
			 {"",
			  "0 5 3 -82 0 0 0x20",
			  "",
			  "0 5 3 -82 0 0 0x20"},
			 {"",
			  "30 -82 0 0 0 1 1 0 0 0 0x20",
			  "",
			  "30 -82 0 0 0 1 1 0 0 0 0x20"},
			 {"",
			  "0 0x0 -100",
			  "",
			  "0 0x0 -100"}},
			// 2G_5G2 (9) 
			{ 0x5,
			 {"0 5 3 -62 0 0 0x22",
			  "",
			  "0 5 3 -82 0 0 0x20",
			  ""},
			 {"30 -62 0 0 0 1 1 0 0 0 0x12",
			  "",
			  "30 -82 0 0 0 1 1 0 0 0 0x20",
			  ""},
			 {"0 0x0 -100",
			  "",
			  "0 0x0 -100",
			  ""}},
			// 5G1_5G2 (10)
			{ 0x6, 
			 {"",
			  "0 5 3 -82 0 0 0x820",
			  "0 5 3 -82 0 0 0x420",
			  ""},
			 {"",
			  "30 -82 0 0 0 1 1 0 0 0 0x8020",
			  "30 -82 0 0 0 1 1 0 0 0 0x4020",
			  ""},
			 {"",
			  "0 0x400 -100",
			  "0 0x200 -100",
			  ""}},
			// 5G2_6G (11)
			{ 0xc, 
			 {"",
			  "",
			  "0 5 3 -82 0 0 0x20",
			  "0 5 3 -82 0 0 0x20"},
			 {"",
			  "",
			  "30 -82 0 0 0 1 1 0 0 0 0x20",
			  "30 -82 0 0 0 1 1 0 0 0 0x20"},
			 {"",
			  "",
			  "0 0x0 -100",
			  "0 0x0 -100"}},
	};

int bcmbsd_tblidx(int selif_val)
{
	int i;

	for(i=1; i<BCMBSD_SELIF_MAX; ++i) {
		if(bcmbsd_def_policy[i].sel_val == selif_val)
			return i;
	}

	return -1;
}

// wl_seq: rule-seq-id; return value(unit_id): which wlunit to set the rule[wl_seq] 
int get_unitid(int wl_seq)
{
	char wlvar[16];
	int nband = -1;

	sprintf(wlvar, "wl%d_ifname", wl_seq);

	if(!*nvram_safe_get(wlvar))
		return -1;

	sprintf(wlvar, "wl%d_nband", wl_seq);
	nband = atoi(nvram_safe_get(wlvar));

	switch(wl_seq) {
		case RULE_2G:
			return WLIF_2G;	// wl-unit-of-2G_xxx set 2G-rule
		case RULE_5G1:
			return WLIF_5G1; // wl-unit-of-5G1_xxx set 5G1 rule
		case RULE_5G2:
#if !defined(GTAXE16000)
			if(nband == 4)
				return WLIF_6G; // * wl-unit-of-6G_xxx should set 6G rule
			else
#endif
				return WLIF_5G2; // wl-unit-of-5G2_xxx set 5G2 rule
		case RULE_6G:
#if !defined(GTAXE16000)
			if(nband != 4)
				return -1; // wl-unit-of-6G_xxx set 5G2 rule
			else
#endif			
				return WLIF_6G; // wl-unit-of-6G_xxx set 6G rule
	}

	return -1;
}

void gen_bcmbsd_def_policy(int sel)
{
	int i, tbi, unit_id, ruleid;
	char namebuf[32];
	int smart_connect_x = nvram_get_int("smart_connect_x");
	int o_sel = sel;
	
	if(!sel)
		sel = nvram_get_int("smart_connect_selif");

	tbi = bcmbsd_tblidx(sel);
	if(tbi < 0)
		return;
	
	_dprintf("%s: by selif val: %d(tbi_%d)(o_sel=%d)\n", __func__, sel, tbi, o_sel);

	for(i=0; i<RULE_MAX; ++i) {
		ruleid = i;
		unit_id = get_unitid(ruleid);

		if(unit_id < 0) {
			_dprintf("%s fin due no more valid wl unit\n", __func__);
			break;
		}
#if !defined(GTAXE16000)
		if(ruleid==RULE_5G2 && unit_id==WLIF_6G) {
			_dprintf("%s: triband-2G/5G/6G Dut adjust its ruleid for seq-%d\n", __func__, i);
			ruleid = RULE_6G;
		}
#endif
		_dprintf("%s, wl%d set rule_idx=%d\n", __func__, unit_id, ruleid);

		sprintf(namebuf, "wl%d_bsd_steering_policy_def", unit_id);
		nvram_set(namebuf, bcmbsd_def_policy[tbi].steering[ruleid]);
		sprintf(namebuf, "wl%d_bsd_steering_policy", unit_id);
		nvram_set(namebuf, bcmbsd_def_policy[tbi].steering[ruleid]);
		if(smart_connect_x == 2) {
			sprintf(namebuf, "wl%d_bsd_steering_policy_x", unit_id);
			nvram_set(namebuf, bcmbsd_def_policy[tbi].steering[ruleid]);
		}
		_dprintf("%s, set defnv [%s]=[%s]\n", __func__, namebuf, bcmbsd_def_policy[tbi].steering[ruleid]);

		sprintf(namebuf, "wl%d_bsd_sta_select_policy_def", unit_id);
		nvram_set(namebuf, bcmbsd_def_policy[tbi].sta_select[ruleid]);
		sprintf(namebuf, "wl%d_bsd_sta_select_policy", unit_id);
		nvram_set(namebuf, bcmbsd_def_policy[tbi].sta_select[ruleid]);
		if(smart_connect_x == 2) {
			sprintf(namebuf, "wl%d_bsd_sta_select_policy_x", unit_id);
			nvram_set(namebuf, bcmbsd_def_policy[tbi].sta_select[ruleid]);
		}
		_dprintf("%s, set defnv [%s]=[%s]\n", __func__, namebuf, bcmbsd_def_policy[tbi].sta_select[ruleid]);

		sprintf(namebuf, "wl%d_bsd_if_qualify_policy_def", unit_id);
		nvram_set(namebuf, bcmbsd_def_policy[tbi].if_qualify[ruleid]);
		sprintf(namebuf, "wl%d_bsd_if_qualify_policy", unit_id);
		nvram_set(namebuf, bcmbsd_def_policy[tbi].if_qualify[ruleid]);
		if(smart_connect_x == 2) {
			sprintf(namebuf, "wl%d_bsd_if_qualify_policy_x", unit_id);
			nvram_set(namebuf, bcmbsd_def_policy[tbi].if_qualify[ruleid]);
		}
		_dprintf("%s, set defnv [%s]=[%s]\n", __func__, namebuf, bcmbsd_def_policy[tbi].if_qualify[ruleid]);
	}	
}

#endif
