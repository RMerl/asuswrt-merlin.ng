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

#ifdef RTCONFIG_QTN
#include "web-qtn.h"
#endif
#ifdef HND_ROUTER
#include <linux/mii.h>
//#include "bcmnet.h"
//#include "bcm/bcmswapitypes.h"
#include "ethctl.h"
#include "ethswctl.h"
#include "ethswctl_api.h"
#include "bcm/bcmswapistat.h"
#include "boardparms.h"
#include <asm/byteorder.h>
#include <board.h>
#endif

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

int read_cled_value(bcm_cled_rgb_led_s *cur_led)
{
	int i = 0;

	for(i = 0; i < 3; i++){
		f_read_string(cur_led->config0_path[i], cur_led->config0_val[i], sizeof(cur_led->config0_val[i]));
		f_read_string(cur_led->config1_path[i], cur_led->config1_val[i], sizeof(cur_led->config1_val[i]));
		f_read_string(cur_led->config2_path[i], cur_led->config2_val[i], sizeof(cur_led->config2_val[i]));
		f_read_string(cur_led->config3_path[i], cur_led->config3_val[i], sizeof(cur_led->config3_val[i]));
	}
}

int is_cled_value_correct(bcm_cled_rgb_led_s *cur_led, int rgb_id, char *val0, char *val1, char *val2, char *val3)
{
	if(strcmp(cur_led->config0_val[rgb_id], val0) != 0 ||
		strcmp(cur_led->config1_val[rgb_id], val1) != 0 ||
		strcmp(cur_led->config2_val[rgb_id], val2) != 0 ||
		strcmp(cur_led->config3_val[rgb_id], val3) != 0){
		return 0;
	}else{
		return 1;
	}
}

int set_cled_value(bcm_cled_rgb_led_s *cur_led, int rgb_id, char *val0, char *val1, char *val2, char *val3)
{
	f_write_string(cur_led->config0_path[rgb_id], val0, 0, 0);
	f_write_string(cur_led->config1_path[rgb_id], val1, 0, 0);
	f_write_string(cur_led->config2_path[rgb_id], val2, 0, 0);
	f_write_string(cur_led->config3_path[rgb_id], val3, 0, 0);
}

int _bcm_cled_ctrl(int rgb, int cled_mode)
{
	int state_changed = 0;
	char LED_BEHAVIOR_WRITE[BCM_CLED_MODE_END][20] =
			{"0x0003e000", "0x0003d000", "0x0003e018", "0x0003e002", "0x0003e038", ""};
	char LED_BEHAVIOR_READ[BCM_CLED_MODE_END][20] =
			{"3e000\n", "3d000\n", "3e018\n", "3e002\n", "3e038\n", ""};

	bcm_cled_rgb_led_s led1 = {
#ifdef RTAX82_XD6
		{"/proc/bcm_cled/led7/config0", "/proc/bcm_cled/led8/config0", "/proc/bcm_cled/led9/config0"},
		{"0x00000000", "0x00000000", "0x00000000"},
		{"/proc/bcm_cled/led7/config1", "/proc/bcm_cled/led8/config1", "/proc/bcm_cled/led9/config1"},
		{"0x00000000", "0x00000000", "0x00000000"},
		{"/proc/bcm_cled/led7/config2", "/proc/bcm_cled/led8/config2", "/proc/bcm_cled/led9/config2"},
		{"0x00000000", "0x00000000", "0x00000000"},
		{"/proc/bcm_cled/led7/config3", "/proc/bcm_cled/led8/config3", "/proc/bcm_cled/led9/config3"},
		{"0x00000000", "0x00000000", "0x00000000"}
#elif defined(RPAX56)
                {"/proc/bcm_cled/led5/config0", "/proc/bcm_cled/led7/config0", "/proc/bcm_cled/led11/config0"},
                {"0x00000000", "0x00000000", "0x00000000"},
                {"/proc/bcm_cled/led5/config1", "/proc/bcm_cled/led7/config1", "/proc/bcm_cled/led11/config1"},
                {"0x00000000", "0x00000000", "0x00000000"},
                {"/proc/bcm_cled/led5/config2", "/proc/bcm_cled/led7/config2", "/proc/bcm_cled/led11/config2"},
                {"0x00000000", "0x00000000", "0x00000000"},
                {"/proc/bcm_cled/led5/config3", "/proc/bcm_cled/led7/config3", "/proc/bcm_cled/led11/config3"},
                {"0x00000000", "0x00000000", "0x00000000"}
#else
		{"/proc/bcm_cled/led14/config0", "/proc/bcm_cled/led15/config0", "/proc/bcm_cled/led16/config0"},
                {"0x00000000", "0x00000000", "0x00000000"},
		{"/proc/bcm_cled/led14/config1", "/proc/bcm_cled/led15/config1", "/proc/bcm_cled/led16/config1"},
                {"0x00000000", "0x00000000", "0x00000000"},
		{"/proc/bcm_cled/led14/config2", "/proc/bcm_cled/led15/config2", "/proc/bcm_cled/led16/config2"},
                {"0x00000000", "0x00000000", "0x00000000"},
		{"/proc/bcm_cled/led14/config3", "/proc/bcm_cled/led15/config3", "/proc/bcm_cled/led16/config3"},
                {"0x00000000", "0x00000000", "0x00000000"}
#endif
	};

	read_cled_value(&led1);

	if(rgb == BCM_CLED_RED ){
		if(is_cled_value_correct(&led1, BCM_CLED_RED, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0 ||
			strcmp(led1.config0_val[BCM_CLED_GREEN], "0\n") != 0 ||
			strcmp(led1.config0_val[BCM_CLED_BLUE], "0\n") != 0){

			set_cled_value(&led1, BCM_CLED_RED, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");

			f_write_string(led1.config0_path[BCM_CLED_GREEN], "0x00000000", 0, 0);
			f_write_string(led1.config0_path[BCM_CLED_BLUE], "0x00000000", 0, 0);

			state_changed = 1;
		}
	}else if(rgb == BCM_CLED_GREEN){
		if(is_cled_value_correct(&led1, BCM_CLED_GREEN, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0 ||
			strcmp(led1.config0_val[BCM_CLED_RED], "0\n") != 0 ||
			strcmp(led1.config0_val[BCM_CLED_BLUE], "0\n") != 0){

			set_cled_value(&led1, BCM_CLED_GREEN, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");

			f_write_string(led1.config0_path[BCM_CLED_RED], "0x00000000", 0, 0);
			f_write_string(led1.config0_path[BCM_CLED_BLUE], "0x00000000", 0, 0);

			state_changed = 1;
		}
	}else if(rgb == BCM_CLED_BLUE){
		if(is_cled_value_correct(&led1, BCM_CLED_BLUE, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0 ||
			strcmp(led1.config0_val[BCM_CLED_RED], "0\n") != 0 ||
			strcmp(led1.config0_val[BCM_CLED_GREEN], "0\n") != 0){

			set_cled_value(&led1, BCM_CLED_BLUE, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");

			f_write_string(led1.config0_path[BCM_CLED_RED], "0x00000000", 0, 0);
			f_write_string(led1.config0_path[BCM_CLED_GREEN], "0x00000000", 0, 0);

			state_changed = 1;
		}
	}else if(rgb == BCM_CLED_YELLOW){
		if(is_cled_value_correct(&led1, BCM_CLED_RED, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
			set_cled_value(&led1, BCM_CLED_RED, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
			state_changed = 1;
		}
		if(is_cled_value_correct(&led1, BCM_CLED_GREEN, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
			set_cled_value(&led1, BCM_CLED_GREEN, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
			state_changed = 1;
		}
		if(is_cled_value_correct(&led1, BCM_CLED_BLUE, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
			set_cled_value(&led1, BCM_CLED_BLUE, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
			state_changed = 1;
		}
	}else if(rgb == BCM_CLED_WHITE){
		if(is_cled_value_correct(&led1, BCM_CLED_RED, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
			set_cled_value(&led1, BCM_CLED_RED, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
			state_changed = 1;
		}
		if(is_cled_value_correct(&led1, BCM_CLED_GREEN, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
			set_cled_value(&led1, BCM_CLED_GREEN, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
			state_changed = 1;
		}
		if(is_cled_value_correct(&led1, BCM_CLED_BLUE, LED_BEHAVIOR_READ[cled_mode], "a34a32\n", "c34\n", "0\n") == 0){
			set_cled_value(&led1, BCM_CLED_BLUE, LED_BEHAVIOR_WRITE[cled_mode], "0x00a34a32", "0x00000c34", "0x00000000");
			state_changed = 1;
		}
	}else if(rgb == BCM_CLED_OFF){
		if(is_cled_value_correct(&led1, BCM_CLED_RED, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
			set_cled_value(&led1, BCM_CLED_RED, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
			state_changed = 1;
		}
		if(is_cled_value_correct(&led1, BCM_CLED_GREEN, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
			set_cled_value(&led1, BCM_CLED_GREEN, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
			state_changed = 1;
		}
		if(is_cled_value_correct(&led1, BCM_CLED_BLUE, "0\n", "a34a32\n", "c34\n", "0\n") == 0){
			set_cled_value(&led1, BCM_CLED_BLUE, "0x00000000", "0x00a34a32", "0x00000c34", "0x00000000");
			state_changed = 1;
		}
	}

	return state_changed;
}

#ifdef RPAX56
static int _reset12 = 0;
#endif
/* rgb: 0:red, 1:green, 2:blue, 3:white */
int bcm_cled_ctrl(int rgb, int cled_mode)
{
	int state_changed = 0;

#ifdef RPAX56
	if(ate_brcm_factory_mode()) {
		//_dprintf("skip bcmcledctrl under atemode\n");
		return 0;
	}
#endif
#if defined(RTAX95Q) || defined(RTAX56_XD4) || defined(CTAX56_XD4) || defined(RTAX82_XD6) || defined(RPAX56)
	state_changed = _bcm_cled_ctrl(rgb, cled_mode);
	if(state_changed == 1){
#ifdef RTAX82_XD6
		f_write_string("/proc/bcm_cled/activate", "0x00000380", 0, 0);
#elif defined(RPAX56)
                f_write_string("/proc/bcm_cled/activate", "0x000008a0", 0, 0);
                if(!_reset12) {
                        _dprintf("\n rc: Reset(2) led12 ah/al\n");
                        eval("sw", "0xff803014", "0xfffff75f");
                        eval("sw", "0xff803018", "0x00001000");
                        _reset12 = 1;
                }
#else
		f_write_string("/proc/bcm_cled/activate", "0x0001C000", 0, 0);
#endif
	}
#endif
	return state_changed;
}
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

#if defined(RTCONFIG_HND_ROUTER_AX_6710) || defined(BCM6750)
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
	sprintf(ledpath, "/sys/class/leds/%d/brightness", gpio);
	ledfd = open(ledpath, O_RDWR);
	if (ledfd <=0 ) {
		printf("\nopen ledpath %s failed !\n", ledpath);
		return -1;
	}

#if defined(RTAX95Q)
	write(ledfd, active_low?(!value?"0":"255"):(!value?"255":"0"), active_low?(!value?1:3):(!value?3:1));
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

#if defined(RTAX95Q) || defined(RTAX56_XD4) || defined(CTAX56_XD4) || defined(RTAX55) || defined(RTAX1800)
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

#ifdef HND_ROUTER
static inline int ethswctl_init(struct ifreq *p_ifr)
{
    int skfd;

#if defined(RTAX95Q) || defined(RTAX56_XD4) || defined(CTAX56_XD4) || defined(RTAX55) || defined(RTAX1800)
	return 1;
#endif
    /* Open a basic socket */
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket open error\n");
        return -1;
    }

    /* Get the name -> if_index mapping for ethswctl */
    strcpy(p_ifr->ifr_name, "bcmsw");
    if (ioctl(skfd, SIOCGIFINDEX, p_ifr) < 0 ) {
        strcpy(p_ifr->ifr_name, WAN_IF_ETH);
        if (ioctl(skfd, SIOCGIFINDEX, p_ifr) < 0 ) {
            close(skfd);
            printf("neither bcmsw nor %s exist\n", WAN_IF_ETH);
            return -1;
        }
    }

    return skfd;
}

#if !defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_EXTPHY_BCM84880)
static int et_dev_subports_query(int skfd, struct ifreq *ifr)
{
	int port_list = 0;

	ifr->ifr_data = (char*)&port_list;
	if (ioctl(skfd, SIOCGQUERYNUMPORTS, ifr) < 0) {
		fprintf(stderr, "Error: Interface %s ioctl SIOCGQUERYNUMPORTS error!\n", ifr->ifr_name);
		return -1;
	}
	return port_list;;
}

static int get_bit_count(int i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static int et_get_phyid2(int skfd, struct ifreq *ifr, int sub_port)
{
	unsigned long phy_id;
	struct mii_ioctl_data *mii = (void *)&ifr->ifr_data;

	mii->val_in = sub_port;

	if (ioctl(skfd, SIOCGMIIPHY, ifr) < 0)
		return -1;

	phy_id = MII_IOCTL_2_PHYID(mii);
	/*
	* returned phy id carries mii->val_out flags if phy is
	* internal/external phy/phy on ext switch.
	* we save it in higher byte to pass to kernel when
	* phy is accessed.
	*/
	return phy_id;
}

static int et_get_phyid(int skfd, struct ifreq *ifr, int sub_port)
{
	int sub_port_map;
#ifdef RTCONFIG_HND_ROUTER_AX
#define MAX_SUB_PORT_BITS (sizeof(int)*8)
#else
#define MAX_SUB_PORT_BITS (sizeof(sub_port_map)*8)
#endif
	if ((sub_port_map = et_dev_subports_query(skfd, ifr)) < 0) {
		return -1;
	}

	if (sub_port_map > 0) {
		if (sub_port == -1) {
			if (get_bit_count(sub_port_map) > 1) {
				fprintf(stderr, "Error: Interface %s has sub ports, please specified one of port map: 0x%x\n",
				ifr->ifr_name, sub_port_map);
				return -1;
			}
			else if (get_bit_count(sub_port_map) == 1) {
				// get bit position
				for(sub_port = 0; sub_port < MAX_SUB_PORT_BITS; sub_port++) {
					if ((sub_port_map & (1 << sub_port)))
					break;
				}
			}
		}

		if ((sub_port_map & (1 << sub_port)) == 0) {
			fprintf(stderr, "Specified SubPort %d is not interface %s's member port with map %x\n",
				sub_port, ifr->ifr_name, sub_port_map);
			return -1;
		}
	} else {
		if (sub_port != -1) {
			fprintf(stderr, "Interface %s has no sub port\n", ifr->ifr_name);
			return -1;
		}
	}

	return et_get_phyid2(skfd, ifr, sub_port);
}

int mdio_read(int skfd, struct ifreq *ifr, int phy_id, int location)
{
	struct mii_ioctl_data *mii = (void *)&ifr->ifr_data;

	PHYID_2_MII_IOCTL(phy_id, mii);
	mii->reg_num = location;
	if (ioctl(skfd, SIOCGMIIREG, ifr) < 0) {
		fprintf(stderr, "SIOCGMIIREG on %s failed: %s\n", ifr->ifr_name,
		strerror(errno));
		return 0;
	}
	return mii->val_out;
}

static void mdio_write(int skfd, struct ifreq *ifr, int phy_id, int location, int value)
{
	struct mii_ioctl_data *mii = (void *)&ifr->ifr_data;

	PHYID_2_MII_IOCTL(phy_id, mii);
	mii->reg_num = location;
	mii->val_in = value;

	if (ioctl(skfd, SIOCSMIIREG, ifr) < 0) {
		fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr->ifr_name,
			strerror(errno));
	}
}

int ethctl_get_link_status(char *ifname)
{
#ifdef RTCONFIG_HND_ROUTER_AX_6710
	char *cmd[] = {"ethctl", ifname, "media-type", NULL};
	char *output = "/tmp/ethctl_get_link_status.txt";
	char *str;
	int ret;
	int lock;

	lock = file_lock("ethctl_link");

	unlink(output);
	_eval(cmd, output, 0, NULL);

	str = file2str(output);
	//_dprintf("%s", str);
	if(!strstr(str, "Enabled"))
		ret = -1;
	else{
		if(strstr(str, "Up"))
			ret = 1;
		else
			ret = 0;
	}

	free(str);
	unlink(output);

	file_unlock(lock);

	return ret;
#else
	int skfd=0, err, bmsr;
	struct ethswctl_data ifdata;
	struct ifreq ifr;
	int phy_id = 0, sub_port = -1;

	if ( strstr(ifname, "eth") == ifname ||
	     strstr(ifname, "epon") == ifname) {
		strcpy(ifr.ifr_name, ifname);
	} else {
		fprintf(stderr, "invalid interface name %s\n", ifname);
		goto error;
	}

	/* Open a basic socket */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("ethctl: socket open error\n");
		return -1;
	}

	/* Get the name -> if_index mapping for ethctl */
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, SIOCGIFINDEX, &ifr) < 0 ) {
		fprintf(stderr, "No %s interface exist\n", ifr.ifr_name);
		goto error;
	}

	if ((phy_id = et_get_phyid(skfd, &ifr, sub_port)) == -1)
		goto error;

	if (ETHCTL_GET_FLAG_FROM_PHYID(phy_id) & ETHCTL_FLAG_ACCESS_SERDES) {
		ifr.ifr_data = (void*) &ifdata;
		ifdata.op = ETHSWPHYMODE;
		ifdata.type = TYPE_GET;
		ifdata.addressing_flag = ETHSW_ADDRESSING_DEV;
		if (sub_port != -1) {
			ifdata.sub_unit = -1; // Set sub_unit to -1 so that main unit of dev will be used
			ifdata.sub_port = sub_port;
			ifdata.addressing_flag |= ETHSW_ADDRESSING_SUBPORT;
		}

		if ((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
			fprintf(stderr, "ioctl command return error %d!\n", err);
			goto error;
		}

		close(skfd);

		return (ifdata.speed == 0) ? 0 : 1;
	}

	bmsr = mdio_read(skfd, &ifr, phy_id, MII_BMSR);
	if (bmsr == 0x0000) {
		fprintf(stderr, "No MII transceiver present!.\n");
		goto error;
	}
	//printf("Link is %s\n", (bmsr & BMSR_LSTATUS) ? "up" : "down");

	close(skfd);
	return (bmsr & BMSR_LSTATUS) ? 1 : 0;
error:
	if (skfd) close(skfd);
	return -1;
#endif
}

#define _MB 0x1
#define _GB 0x2
#define _2GB 0x4
static int ethctl_get_link_speed(char *ifname)
{
	int skfd=0, err;
	struct ethswctl_data ifdata;
	struct ifreq ifr;
	int phy_id = 0, sub_port = -1;
	int bmcr, bmsr, gig_ctrl, gig_status, v16;

	if ( strstr(ifname, "eth") == ifname ||
	     strstr(ifname, "epon") == ifname) {
		strcpy(ifr.ifr_name, ifname);
	} else {
		fprintf(stderr, "invalid interface name %s\n", ifname);
		goto error;
	}

	/* Open a basic socket */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("ethctl: socket open error\n");
		return -1;
	}

	/* Get the name -> if_index mapping for ethctl */
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, SIOCGIFINDEX, &ifr) < 0 ) {
		printf("No %s interface exist\n", ifr.ifr_name);
		goto error;
	}

	if ((phy_id = et_get_phyid(skfd, &ifr, sub_port)) == -1)
		goto error;

	if (ETHCTL_GET_FLAG_FROM_PHYID(phy_id) & ETHCTL_FLAG_ACCESS_SERDES) {
		ifr.ifr_data = (void*) &ifdata;
		ifdata.op = ETHSWPHYMODE;
		ifdata.type = TYPE_GET;
		ifdata.addressing_flag = ETHSW_ADDRESSING_DEV;
		if (sub_port != -1) {
			ifdata.sub_unit = -1; // Set sub_unit to -1 so that main unit of dev will be used
			ifdata.sub_port = sub_port;
			ifdata.addressing_flag |= ETHSW_ADDRESSING_SUBPORT;
		}

		if ((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
			fprintf(stderr, "ioctl command return error %d!\n", err);
			goto error;;
		}

		close(skfd);
		return (ifdata.speed >= 2000 ? _2GB : (ifdata.speed >= 1000 ? _GB : _MB));
	}

	bmsr = mdio_read(skfd, &ifr, phy_id, MII_BMSR);
	bmcr = mdio_read(skfd, &ifr, phy_id, MII_BMCR);
	if (bmcr == 0xffff ||  bmsr == 0x0000) {
		fprintf(stderr, "No MII transceiver present!.\n");
		goto error;
	}

	if (!(bmsr & BMSR_LSTATUS)) {
		fprintf(stderr, "Link is down!.\n");
		goto error;
	}

	if (bmcr & BMCR_ANENABLE) {
		gig_ctrl = mdio_read(skfd, &ifr, phy_id, MII_CTRL1000);
		// check ethernet@wirspeed only for PHY support 1G
		if (gig_ctrl & ADVERTISE_1000FULL || gig_ctrl & ADVERTISE_1000HALF) {
			// check if ethernet@wirespeed is enabled, reg 0x18, shodow 0b'111, bit4
			mdio_write(skfd, &ifr, phy_id, 0x18, 0x7007);
			v16 = mdio_read(skfd, &ifr, phy_id, 0x18);
			if (v16 & 0x0010) {
				// get link speed from ASR if ethernet@wirespeed is enabled
				v16 = mdio_read(skfd, &ifr, phy_id, 0x19);
#define MII_ASR_1000(r) (((r & 0x0700) == 0x0700) || ((r & 0x0700) == 0x0600))
#define MII_ASR_100(r)  (((r & 0x0700) == 0x0500) || ((r & 0x0700) == 0x0300))
#define MII_ASR_10(r)   (((r & 0x0700) == 0x0200) || ((r & 0x0700) == 0x0100))
				close(skfd);
				return MII_ASR_1000(v16) ? _GB : (MII_ASR_100(v16) || MII_ASR_10(v16)) ? _MB : -1;
			}
		}

		gig_status = mdio_read(skfd, &ifr, phy_id, MII_STAT1000);
		close(skfd);
		if (((gig_ctrl & ADVERTISE_1000FULL) && (gig_status & LPA_1000FULL)) ||
		    ((gig_ctrl & ADVERTISE_1000HALF) && (gig_status & LPA_1000HALF))) {
			close(skfd);
			return _GB;
		}
		else {
			return _MB;
		}
	}
	else {
		close(skfd);
		return (bmcr & BMCR_SPEED1000) ? _GB : _MB;
	}

error:
	if (skfd) close(skfd);
	return -1;
}

static int ethctl_get_link_duplex(char *ifname)
{
	int skfd=0, err;
	struct ethswctl_data ifdata;
	struct ifreq ifr;
	int phy_id = 0, sub_port = -1;
	int bmcr, bmsr, gig_ctrl, gig_status, v16;

	if ( strstr(ifname, "eth") == ifname ||
	     strstr(ifname, "epon") == ifname) {
		strcpy(ifr.ifr_name, ifname);
	} else {
		fprintf(stderr, "invalid interface name %s\n", ifname);
		goto error;
	}

	/* Open a basic socket */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("ethctl: socket open error\n");
		return -1;
	}

	/* Get the name -> if_index mapping for ethctl */
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, SIOCGIFINDEX, &ifr) < 0 ) {
		printf("No %s interface exist\n", ifr.ifr_name);
		goto error;
	}

	if ((phy_id = et_get_phyid(skfd, &ifr, sub_port)) == -1)
		goto error;

	if (ETHCTL_GET_FLAG_FROM_PHYID(phy_id) & ETHCTL_FLAG_ACCESS_SERDES) {
		ifr.ifr_data = (void*) &ifdata;
		ifdata.op = ETHSWPHYMODE;
		ifdata.type = TYPE_GET;
		ifdata.addressing_flag = ETHSW_ADDRESSING_DEV;
		if (sub_port != -1) {
			ifdata.sub_unit = -1; // Set sub_unit to -1 so that main unit of dev will be used
			ifdata.sub_port = sub_port;
			ifdata.addressing_flag |= ETHSW_ADDRESSING_SUBPORT;
		}

		if ((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
			fprintf(stderr, "ioctl command return error %d!\n", err);
			goto error;;
		}

		close(skfd);
		return (ifdata.speed >= 2000 ? _2GB : (ifdata.speed >= 1000 ? _GB : _MB));
	}

	bmsr = mdio_read(skfd, &ifr, phy_id, MII_BMSR);
	bmcr = mdio_read(skfd, &ifr, phy_id, MII_BMCR);
	if (bmcr == 0xffff ||  bmsr == 0x0000) {
		fprintf(stderr, "No MII transceiver present!.\n");
		goto error;
	}

	if (!(bmsr & BMSR_LSTATUS)) {
		fprintf(stderr, "Link is down!.\n");
		goto error;
	}

	if (bmcr & BMCR_ANENABLE) { // auto nego
		gig_ctrl = mdio_read(skfd, &ifr, phy_id, MII_CTRL1000);
		// check ethernet@wirspeed only for PHY support 1G
		if (gig_ctrl & ADVERTISE_1000FULL || gig_ctrl & ADVERTISE_1000HALF) {
			// check if ethernet@wirespeed is enabled, reg 0x18, shodow 0b'111, bit4
			mdio_write(skfd, &ifr, phy_id, 0x18, 0x7007);
			v16 = mdio_read(skfd, &ifr, phy_id, 0x18);
			if (v16 & 0x0010) {
				// get link speed from ASR if ethernet@wirespeed is enabled
				v16 = mdio_read(skfd, &ifr, phy_id, 0x19);
#define MII_ASR_FDX(r)  (((r & 0x0700) == 0x0700) || ((r & 0x0700) == 0x0500) || ((r & 0x0700) == 0x0200))
				close(skfd);
				return MII_ASR_FDX(v16);
			}
		}

		gig_status = mdio_read(skfd, &ifr, phy_id, MII_STAT1000);
		close(skfd);
		if (((gig_ctrl & ADVERTISE_1000FULL) && (gig_status & LPA_1000FULL)) ||
		    (gig_ctrl & ADVERTISE_100FULL) || 
		    (gig_ctrl & ADVERTISE_10FULL)) {
			close(skfd);
			return 1;
		}
		else {
			return 0;
		}
	}
	else {
		close(skfd);
		return (bmcr & BMCR_FULLDPLX);
	}

error:
	if (skfd) close(skfd);
	return -1;
}
#else
int ethctl_get_link_status(char *ifname)
{
#if defined(RTCONFIG_HND_ROUTER_AX_675X)
	char tmp[100], buf[32];
	int ret = 0;

	snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/operstate", ifname);
	f_read_string(tmp, buf, sizeof(buf));

	if(strcmp(buf, "up\n")==0) ret = 1;
	else ret = 0;
#else
	char *cmd[] = {"ethctl", ifname, "media-type", NULL};
	char *output = "/tmp/ethctl_get_link_status.txt";
	char *str;
	int ret;
	int lock;

	lock = file_lock("ethctl_link");

	unlink(output);
	_eval(cmd, output, 0, NULL);

	str = file2str(output);
	if(!strstr(str, "Enabled"))
		ret = -1;
	else{
		if(strstr(str, "Up"))
			ret = 1;
		else
			ret = 0;
	}

	free(str);
	unlink(output);

	file_unlock(lock);

#endif
	return ret;
}
#endif //!defined(RTCONFIG_HND_ROUTER_AX_675X) || defined(RTCONFIG_EXTPHY_BCM84880)

struct ethctl_data ethctl;
int ethctl_phy_op(char* phy_type, int addr, unsigned int reg, unsigned int value, int wr)
{
	struct ifreq ifr;
    	int skfd;
	int err, phy_id = 0, phy_flag = 0;
	unsigned int phy_reg = 0;

	strcpy(ifr.ifr_name, "bcmsw");
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "socket open error\n");
		return -1;
	}

	if (ioctl(skfd, SIOCGIFINDEX, &ifr) < 0 ) {
		fprintf(stderr, "ioctl failed. check if %s exists\n", ifr.ifr_name);
		close(skfd);
		return -1;
	}


	if (strcmp(phy_type, "ext") == 0) {
		phy_flag = ETHCTL_FLAG_ACCESS_EXT_PHY;
	} else if (strcmp(phy_type, "int") == 0) {
		phy_flag = ETHCTL_FLAG_ACCESS_INT_PHY;
	} else if (strcmp(phy_type, "extsw") == 0) { // phy connected to external switch
		phy_flag = ETHCTL_FLAG_ACCESS_EXTSW_PHY;
	} else if (strcmp(phy_type, "i2c") == 0) { // phy connected through I2C bus
		phy_flag = ETHCTL_FLAG_ACCESS_I2C_PHY;
#ifdef RTCONFIG_EXTPHY_BCM84880
	} else if (strcmp(phy_type, "10gserdes") == 0) { // phy connected through I2C bus
		phy_flag = ETHCTL_FLAG_ACCESS_10GSERDES;
	} else if (strcmp(phy_type, "10gpcs") == 0) { // phy connected through I2C bus
		phy_flag = ETHCTL_FLAG_ACCESS_10GPCS;
#endif
	} else if (strcmp(phy_type, "serdespower") == 0) { // Serdes power saving mode
		phy_flag = ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE;
	} else if (strcmp(phy_type, "ext32") == 0) { // Extended 32bit register access.
		phy_flag = ETHCTL_FLAG_ACCESS_32BIT|ETHCTL_FLAG_ACCESS_EXT_PHY;
	} else {
		fprintf(stderr, "Unknown phy type!\n");
		close(skfd);
		return -1;
	}

	phy_id = addr;
	phy_reg = reg;

	if ((phy_id < 0) || (phy_id > 31)) {
		fprintf(stderr, "Invalid Phy Address 0x%02x\n", phy_id);
		close(skfd);
		return -1;
        }

	if(phy_flag == ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE && (reg < 0 || reg > 2))
        {
		fprintf(stderr, "Invalid Serdes Power Mode%02x\n", reg);
		close(skfd);
		return -1;
	}

	ethctl.phy_addr = phy_id;
	ethctl.phy_reg = phy_reg;
	ethctl.flags = phy_flag;

	if(wr) { // Write
		ethctl.op = ETHSETMIIREG;
		ethctl.val = value;
		ifr.ifr_data = (void *)&ethctl;
		err = ioctl(skfd, SIOCETHCTLOPS, &ifr);
		if (ethctl.ret_val || err) {
			_dprintf("SET ERROR!!!\n");
            		fprintf(stderr, "command return error!\n");
			close(skfd);
			return -1;
		}

//		_dprintf("[SET] %08x = %08x\n", phy_reg, value);
	}
	else { // Read
		ethctl.op = ETHGETMIIREG;
		ifr.ifr_data = (void *)&ethctl;
		err = ioctl(skfd, SIOCETHCTLOPS, &ifr);
		if (ethctl.ret_val || err) {
			_dprintf("GET ERROR!!!\n");
			fprintf(stderr, "command return error!\n");
			close(skfd);
			return -1;
		}
		else {
//			_dprintf("[GET] %08x = %04x\n", phy_reg, ethctl.val);
			close(skfd);
			return ethctl.val;
		}
	}

	return 0;
}

#ifdef RTCONFIG_EXTPHY_BCM84880
int extphy_bit_op(unsigned int reg, unsigned int val, int wr, unsigned int start_bit, unsigned int end_bit, unsigned int wait_ms){
#define MIN_BIT 0
#define MAX_BIT 15
	struct ifreq ifr;
	int skfd, err;
	int orig_val, val_mask, val_reverse_mask;
	int bit;

	if((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "socket open error\n");
		return -1;
	}

	strcpy(ifr.ifr_name, "bcmsw");
	if(ioctl(skfd, SIOCGIFINDEX, &ifr) < 0 ){
		fprintf(stderr, "ioctl failed. check if %s exists\n", ifr.ifr_name);
		close(skfd);
		return -1;
	}

	if(nvram_get_int("ext_phy_model") == 1)
		ethctl.phy_addr = EXTPHY_RTL_ADDR;
	else
		ethctl.phy_addr = EXTPHY_ADDR;
	ethctl.phy_reg = reg;
	ethctl.flags = ETHCTL_FLAG_ACCESS_EXT_PHY;

	// Read
	ethctl.op = ETHGETMIIREG;
	ifr.ifr_data = (void *)&ethctl;
	err = ioctl(skfd, SIOCETHCTLOPS, &ifr);
	if(ethctl.ret_val || err){
		_dprintf("GET ERROR!!!\n");
		fprintf(stderr, "command return error!\n");
		close(skfd);
		return -1;
	}

	if(start_bit < MIN_BIT || start_bit > MAX_BIT)
		start_bit = MIN_BIT;

	if(end_bit < MIN_BIT || end_bit > MAX_BIT)
		end_bit = MAX_BIT;

	val_mask = 0;
	for(bit = start_bit; bit <= end_bit; ++bit)
		val_mask |= 0x1<<bit;
	//_dprintf("[val_mask] [%u:%u], 0x%04x\n", end_bit, start_bit, val_mask);

	val_reverse_mask = val_mask^0xffff;
	//_dprintf("[val_reverse_mask] [%u:%u], 0x%04x\n", end_bit, start_bit, val_reverse_mask);

	orig_val = ethctl.val;
	if(!wr){
		ethctl.val &= val_mask;
		ethctl.val >>= start_bit;
		_dprintf("[GET] 0x%08x [%u:%u] = 0x%04x, full 0x%04x\n", reg, end_bit, start_bit, ethctl.val, orig_val);
		close(skfd);
		return ethctl.val;
	}
	else
		_dprintf("[Ori] 0x%08x [%u:%u] = 0x%04x\n", reg, MAX_BIT, MIN_BIT, orig_val);

	// Write
	ethctl.op = ETHSETMIIREG;
	ethctl.val = (orig_val&val_reverse_mask) + (val<<start_bit);
	ifr.ifr_data = (void *)&ethctl;
	err = ioctl(skfd, SIOCETHCTLOPS, &ifr);
	if (ethctl.ret_val || err) {
		_dprintf("SET ERROR!!!\n");
		fprintf(stderr, "command return error!\n");
		close(skfd);
		return -1;
	}

	_dprintf("[SET] 0x%08x [%u:%u] = 0x%04x, full 0x%04x\n", reg, end_bit, start_bit, val, ethctl.val);
	close(skfd);

	if(wait_ms > 0){
		//_dprintf("Sleeping %u mini seconds...\n", wait_ms);
		usleep(wait_ms*1000);
	}

	//_dprintf("done\n");

	return ethctl.val;
}
#endif

int bcm_reg_read_X(int unit, unsigned int addr, char* data, int len)
{
    int skfd, err = 0;
    struct ifreq ifr;
    struct ethswctl_data ifdata;
    struct ethswctl_data *e = &ifdata;

    if ((skfd=ethswctl_init(&ifr)) < 0) {
        printf("ethswctl_init failed. \n");
        return skfd;
    }
    ifr.ifr_data = (char *)&ifdata;

    e->op = ETHSWREGACCESS;
    e->type = TYPE_GET;
    e->offset = addr;
    e->length = len;
    e->unit = unit;

    if ((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
        printf("ioctl command return error!\n");
        goto out;
    }

    memcpy(data, e->data, len);

out:
    close(skfd);
    return err;
}

int bcm_reg_write_X(int unit, unsigned int addr, char* data, int len)
{
    int skfd, err = 0;
    struct ifreq ifr;
    struct ethswctl_data ifdata;
    struct ethswctl_data *e = &ifdata;

    if ((skfd=ethswctl_init(&ifr)) < 0) {
        printf("ethswctl_init failed. \n");
        return skfd;
    }
    ifr.ifr_data = (char *)&ifdata;

    e->op = ETHSWREGACCESS;
    e->type = TYPE_SET;
    e->offset = addr;
    e->length = len;
    e->unit = unit;
    memcpy(e->data, data, len);

    if ((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
        printf("ioctl command return error!\n");
        goto out;
    }

out:
    close(skfd);
    return err;
}

int bcm_pseudo_mdio_read(unsigned int addr, char* data, int len)
{
    int skfd, err = 0;
    struct ifreq ifr;
    struct ethswctl_data ifdata;
    struct ethswctl_data *e = &ifdata;

    if ((skfd=ethswctl_init(&ifr)) < 0) {
        printf("ethswctl_init failed. \n");
        return skfd;
    }
    ifr.ifr_data = (char *)&ifdata;

    e->op = ETHSWPSEUDOMDIOACCESS;
    e->type = TYPE_GET;
    e->offset = addr;
    e->length = len;

    if ((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
        printf("ioctl command return error!\n");
        goto out;
    }

    memcpy(data, e->data, len);

out:
    close(skfd);
    return err;
}

int bcm_pseudo_mdio_write(unsigned int addr, char* data, int len)
{
    int skfd, err = 0;
    struct ifreq ifr;
    struct ethswctl_data ifdata;
    struct ethswctl_data *e = &ifdata;

    if ((skfd=ethswctl_init(&ifr)) < 0) {
        printf("ethswctl_init failed. \n");
        return skfd;
    }
    ifr.ifr_data = (char *)&ifdata;

    e->op = ETHSWPSEUDOMDIOACCESS;
    e->type = TYPE_SET;
    e->offset = addr;
    e->length = len;
    memcpy(e->data, data, sizeof(e->data));

    if ((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
        printf("ioctl command return error!\n");
        goto out;
    }

out:
    close(skfd);
    return err;
}

uint64_t hnd_ethswctl(ecmd_t act, unsigned int val, int len, int wr, unsigned long long regdata)
{
	unsigned long long data64 = 0;
	int ret_val = 0, i;
	unsigned char data[8];

	switch(act) {
		case REGACCESS:
			if (wr) {
				data64 = cpu_to_le64(regdata);
				//_dprintf("w Data: %08x %08x \n", (unsigned int)(data64 >> 32), (unsigned int)(data64) );
				ret_val = bcm_reg_write_X(1, val, (char *)&data64, len);
			} else {
				ret_val = bcm_reg_read_X(1, val, (char *)&data64, len);
				data64 = le64_to_cpu(data64);
				//_dprintf("Data: %08x %08x \n", (unsigned int)(data64 >> 32), (unsigned int)(data64) );
				return data64;
			}
			break;
		case PMDIOACCESS:
#ifdef RTCONFIG_HND_ROUTER_AX
			data64 = cpu_to_le64(regdata);
			for(i = 0; i < 8; i++)
			{
				data[i] = (unsigned char) (*( ((char *)&data64) + i));
			}
#else
			for(i = 0; i < 8; i++)
			{
 				data[i] = (unsigned char) (*( ((char *)&regdata) + 7-i));
			}
#endif
			if (wr) {
				//_dprintf("\npw data\n");
				ret_val = bcm_pseudo_mdio_write(val, (char*)data, len);
			} else {
				memset(data, 0, sizeof(data));
				ret_val = bcm_pseudo_mdio_read(val, (char*)data, len);
				//_dprintf("pr Data: %02x%02x%02x%02x %02x%02x%02x%02x\n",
				//data[7], data[6], data[5], data[4], data[3], data[2], data[1], data[0]);
				memcpy(&data64, data, 8);
				return data64;
			}
			break;
	}

	return ret_val;
}

typedef struct {
	unsigned int link[4];
	unsigned int speed[4];
	unsigned int duplex[4];
} phyState;

#if defined(RTCONFIG_HND_ROUTER_AX_6710)
uint32_t hnd_get_phy_status(char *ifname)
{
	char tmp[100], buf[32];

	snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/operstate", ifname);

	f_read_string(tmp, buf, sizeof(buf));
	if(!strncmp(buf, "up", 2))
		return 1;
	else
		return 0;
}
#elif defined(RTCONFIG_HND_ROUTER_AX_675X)
uint32_t hnd_get_phy_status(int port)
{
	char ifname[16], tmp[100], buf[32];

#if defined(RTAX55) || defined(RTAX1800)
	int fd;
	phyState pS;

	if (port)
	{
		fd = open("/dev/rtkswitch", O_RDONLY);
		if (fd < 0) {
			perror("/dev/rtkswitch");
		} else {
			memset(&pS, 0, sizeof(pS));
			if (ioctl(fd, 0, &pS) < 0) {
				perror("rtkswitch ioctl");
				close(fd);
			}

			close(fd);
		}

		return pS.link[port - 1];
	}
	else
#endif
	{
		snprintf(ifname, sizeof(ifname), "eth%d", port);
		snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/operstate", ifname);

		f_read_string(tmp, buf, sizeof(buf));
		if(!strncmp(buf, "up", 2))
			return 1;
		else
			return 0;
	}
}
#else
uint32_t hnd_get_phy_status(int port, int offs, unsigned int regv, unsigned int pmdv)
{
	if (port == 7
#ifdef RTCONFIG_EXTPHY_BCM84880
	    || port == 4
#endif 
	) {			// wan port
#ifdef RTCONFIG_EXTPHY_BCM84880
		// port4(eth0)->1G WAN, port7(eth5)->2.5G LAN
		return ethctl_get_link_status(port == 4 ? WAN_IF_ETH : "eth5");
#else
		return ethctl_get_link_status(WAN_IF_ETH);
#endif
	} else if (!offs || (port-offs < 0)) {	// main switch
		return regv & (1<<port) ? 1 : 0;
	} else {				// externai switch
		return pmdv & (1<<(port-offs)) ? 1 : 0;
	}
}
#endif

#if defined(RTCONFIG_HND_ROUTER_AX_6710)
uint32_t hnd_get_phy_speed(char *ifname)
{
	char tmp[100], buf[32];

	snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/speed", ifname);

	f_read_string(tmp, buf, sizeof(buf));
	return strtoul(buf, NULL, 10);
}
#elif defined(RTCONFIG_HND_ROUTER_AX_675X)
uint32_t hnd_get_phy_speed(int port)
{
	char ifname[16], tmp[100], buf[32];

#if defined(RTAX55) || defined(RTAX1800)
	int fd;
	phyState pS;

	if (port)
	{
		fd = open("/dev/rtkswitch", O_RDONLY);
		if (fd < 0) {
			perror("/dev/rtkswitch");
		} else {
			memset(&pS, 0, sizeof(pS));
			if (ioctl(fd, 0, &pS) < 0) {
				perror("rtkswitch ioctl");
				close(fd);
			}

			close(fd);
		}

		if (pS.link[port - 1])
			return ((pS.speed[port - 1] == 2) ? 1000 : 100);
		else
			return 0;
	}
	else
#endif
	{
		snprintf(ifname, sizeof(ifname), "eth%d", port);
		snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/speed", ifname);

		f_read_string(tmp, buf, sizeof(buf));
		return strtoul(buf, NULL, 10);
	}
}
#else
uint32_t hnd_get_phy_speed(int port, int offs, unsigned int regv, unsigned int pmdv)
{
	int val = 0;
	if (port == 7
#ifdef RTCONFIG_EXTPHY_BCM84880
            || port == 4
#endif
	) {			// wan port
#ifdef RTCONFIG_EXTPHY_BCM84880
                // port4(eth0)->1G WAN, port7(eth5)->2.5G LAN
		return ethctl_get_link_speed(port == 4 ? WAN_IF_ETH : "eth5");
#else
		return ethctl_get_link_speed(WAN_IF_ETH);
#endif
	}
	else if (!offs || (port-offs < 0)) {	// main switch
		val = regv & (0x0003<<(port*2));
		return val>>(port*2);
	} else {				// externai switch
		val = pmdv & (0x0003<<((port-offs)*2));
		return val>>((port-offs)*2);
	}
}
#endif

#if defined(RTCONFIG_HND_ROUTER_AX_6710)
uint32_t hnd_get_phy_duplex(char *ifname)
{
	char tmp[100], buf[32];

	snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/duplex", ifname);

	f_read_string(tmp, buf, sizeof(buf));
	if(!strncmp(buf, "full", 4))
		return 1;
	else
		return 0;
}
#elif defined(RTCONFIG_HND_ROUTER_AX_675X)
uint32_t hnd_get_phy_duplex(int port)
{
	char ifname[16], tmp[100], buf[32];

#if defined(RTAX55) || defined(RTAX1800)
	int fd;
	phyState pS;

	if (port)
	{
		fd = open("/dev/rtkswitch", O_RDONLY);
		if (fd < 0) {
			perror("/dev/rtkswitch");
		} else {
			memset(&pS, 0, sizeof(pS));
			if (ioctl(fd, 0, &pS) < 0) {
				perror("rtkswitch ioctl");
				close(fd);
			}

			close(fd);
		}

		return pS.duplex[port - 1];
	}
	else
#endif
	{
		snprintf(ifname, sizeof(ifname), "eth%d", port);
		snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/duplex", ifname);

		f_read_string(tmp, buf, sizeof(buf));
		if(!strncmp(buf, "full", 4))
			return 1;
		else
			return 0;
	}
}
#else
uint32_t hnd_get_phy_duplex(int port, int offs, unsigned int regv, unsigned int pmdv)
{
	if (port == 7
#ifdef RTCONFIG_EXTPHY_BCM84880
	    || port == 4
#endif 
	) {			// wan port
#ifdef RTCONFIG_EXTPHY_BCM84880
		// port4(eth0)->1G WAN, port7(eth5)->2.5G LAN
		return ethctl_get_link_duplex(port == 4 ? WAN_IF_ETH : "eth5");
#else
		return ethctl_get_link_duplex(WAN_IF_ETH);
#endif
	} else if (!offs || (port-offs < 0)) {	// main switch
		return regv & (1<<port) ? 1 : 0;
	} else {				// externai switch
		return pmdv & (1<<(port-offs)) ? 1 : 0;
	}
}
#endif

static uint64_t hnd_get_phy_mib_by_ifname(char *ifname, char *type)
{
	char tmp[100], buf[32];
	int result = 0;

	if (!ifname || !type)
		return result;

	snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/statistics/%s", ifname, type);

	f_read_string(tmp, buf, sizeof(buf));
	return strtoull(buf, NULL, 10);
}

#if defined(RTCONFIG_HND_ROUTER_AX_6710)
uint64_t hnd_get_phy_mib(char *ifname, char *type)
{
	return hnd_get_phy_mib_by_ifname(ifname, type);
}
#elif defined(RTCONFIG_HND_ROUTER_AX_675X)
uint64_t hnd_get_phy_mib(int port, char *type)
{
	char ifname[16], tmp[100], buf[32];
	int result = 0;

	if (!type)
		return result;

#if defined(RTAX55) || defined(RTAX1800)
	int fd;
	int *p = NULL;
	rtk_stat_port_cntr_t Port_cntrs;

	if (port)
	{
		fd = open("/dev/rtkswitch", O_RDONLY);
		if (fd < 0) {
			perror("/dev/rtkswitch");
		} else {
			memset(&Port_cntrs, 0, sizeof(Port_cntrs));
			p = (int *) &Port_cntrs;
			*p = port - 1;
			if (ioctl(fd, 1, &Port_cntrs) < 0) {
				perror("rtkswitch ioctl");
				close(fd);
			} else {
				if (!strcmp(type, "tx_bytes"))
					result = Port_cntrs.ifOutOctets;
				else if (!strcmp(type, "rx_bytes"))
					result = Port_cntrs.ifInOctets;
				else if (!strcmp(type, "tx_packets"))
					result = Port_cntrs.ifOutUcastPkts + Port_cntrs.ifOutMulticastPkts + Port_cntrs.ifOutBrocastPkts;
				else if (!strcmp(type, "rx_packets"))
					result = Port_cntrs.ifInUcastPkts + Port_cntrs.ifInMulticastPkts;
				else if (!strcmp(type, "rx_crc_errors"))
					result = Port_cntrs.dot3StatsFCSErrors;
			}

			close(fd);
		}
		return result;
	}
	else
#endif
	{
		snprintf(ifname, sizeof(ifname), "eth%d", port);
		snprintf(tmp, sizeof(tmp), "/sys/class/net/%s/statistics/%s", ifname, type);

		f_read_string(tmp, buf, sizeof(buf));
		return strtoull(buf, NULL, 10);
	}
}
#else
static uint64_t hnd_get_phy_mib_by_ethswctl(int port, int offs, char *type)
{
	uint64_t val = 0;
	int addr_cnt = 0, i = 0;
	unsigned int addr[8] = {0};
	unsigned long long data = 0;
	unsigned int port_id = (!offs || (port-offs < 0)) ? port : port-offs;
	ecmd_t act = (!offs || (port-offs < 0)) ? REGACCESS : PMDIOACCESS;
	if (!strcmp(type, "tx_bytes")) {
		addr[addr_cnt++] = ((PAGE_MIB_BASE+port_id)<<8) + REG_OFFSET_TX_BYTES;
	}
	else if (!strcmp(type, "rx_bytes")) {
		addr[addr_cnt++] = ((PAGE_MIB_BASE+port_id)<<8) + REG_OFFSET_RX_BYTES;
	}
	else if (!strcmp(type, "tx_packets")) {
		addr[addr_cnt++] = ((PAGE_MIB_BASE+port_id)<<8) + REG_OFFSET_TX_BROADCAST_PACKETS;
		addr[addr_cnt++] = ((PAGE_MIB_BASE+port_id)<<8) + REG_OFFSET_TX_MULTICAST_PACKETS;
		addr[addr_cnt++] = ((PAGE_MIB_BASE+port_id)<<8) + REG_OFFSET_TX_UNICAST_PACKETS;
	}
	else if (!strcmp(type, "rx_packets")) {
		addr[addr_cnt++] = ((PAGE_MIB_BASE+port_id)<<8) + REG_OFFSET_RX_UNICAST_PACKETS;
		addr[addr_cnt++] = ((PAGE_MIB_BASE+port_id)<<8) + REG_OFFSET_RX_MULTICAST_PACKETS;
		addr[addr_cnt++] = ((PAGE_MIB_BASE+port_id)<<8) + REG_OFFSET_RX_BROADCAST_PACKETS;
	}
	else if (!strcmp(type, "rx_crc_errors")) {
		addr[addr_cnt++] = ((PAGE_MIB_BASE+port_id)<<8) + REG_OFFSET_RX_FCS_ERROR;
	}

	for (i = 0; i < addr_cnt; i++) {
		data = 0;
		data = hnd_ethswctl(act, addr[i], 8, 0, 0);
		//fprintf(stderr, "addr=%x, data=%llu\n", addr[i], data);
		val += data;
	}
	return val;
}

uint64_t hnd_get_phy_mib(int port, int offs, char *type)
{
	if (port == 7
#ifdef RTCONFIG_EXTPHY_BCM84880
            || port == 4
#endif
	) {			// wan port
#ifdef RTCONFIG_EXTPHY_BCM84880
                // port4(eth0)->1G WAN, port7(eth5)->2.5G LAN
		return hnd_get_phy_mib_by_ifname(port == 4 ? WAN_IF_ETH : "eth5", type);
#else
		return hnd_get_phy_mib_by_ifname(WAN_IF_ETH, type);
#endif
	} else {
		return hnd_get_phy_mib_by_ethswctl(port, offs, type);
	}
}
#endif /* RTCONFIG_HND_ROUTER_AX_6710 */

#endif /* HND_ROUTER */

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

#if defined(RTAX95Q) || defined(RTAX56_XD4) || defined(CTAX56_XD4)
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

#if defined(RTAX95Q) || defined(RTAX56_XD4) || defined(CTAX56_XD4) || defined(RTAX55) || defined(RTAX1800)
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

		if (nvram_get_int("led_disable")==1) {
			led_control(LED_2G, LED_OFF);
			led_control(LED_5G, LED_OFF);
		}
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

	if (nvram_get_int("led_disable")==1) {
		led_control(LED_2G, LED_OFF);
		led_control(LED_5G, LED_OFF);
	}
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
	char buf[32];

	snprintf(confbuf, sizeof(confbuf),
			"/sys/class/net/%s/speed", bond_if);
	f_read_string(confbuf, buf, sizeof(buf));

	if(strcmp(buf, "2000\n")==0) return 2000;
	else if(strcmp(buf, "1000\n") == 0) return 1000;
	else return 0;
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
	int extra_p0=0;
	unsigned int regv=0, pmdv=0, regv2=0, pmdv2=0;
#ifdef RTCONFIG_EXT_BCM53134 /* RT-AX88U */
	int lan_ports=4;
	int ports[lan_ports+1];
	ports[0]=7; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
#elif defined(RTAX92U)
	int lan_ports=4;
	int ports[lan_ports+1];
	/* 7 3 2 1 0	W0 L1 L2 L3 L4 */
	ports[0]=7; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
#elif defined(RTAX95Q)
	int lan_ports=4;
	int ports[lan_ports+1];
	/* 7 3 2 1 0	W0 L1 L2 L3 L4 */
	ports[0]=7; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
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
#elif defined(CTAX56_XD4)
	int lan_ports=1;

	int ports[lan_ports+1];
	/* 7 3	W0 L1 */
	ports[0]=7; ports[1]=3;
#elif defined(RTAX58U) || defined(TUFAX3000) || defined(RTAX82U) || defined(GSAX3000) || defined(GSAX5400)
	int lan_ports=4;
	int ports[lan_ports+1];
	/* 4 3 2 1 0	W0 L1 L2 L3 L4 */
	ports[0]=4; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
#elif defined(RTAX82_XD6)
        int lan_ports=3;
        int ports[lan_ports+1];
        /* 4 2 1 0    W0 L1 L2 L3 */
        ports[0]=4; ports[1]=2; ports[2]=1; ports[3]=0;
#elif defined(RTAX56U)
	int lan_ports=4;
	int ports[lan_ports+1];
	/* 4 3 2 1 0	W0 L1 L2 L3 L4 */
	ports[0]=4; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
#elif defined(RTAX86U) || defined(RTAX5700) || defined(GTAXE11000)
	int lan_ports = 5;
	char *ports[lan_ports+1];
	/* 7 4 3 2 1 0	L5(2.5G) W0 L1 L2 L3 L4 */
	/* eth5 eth0 eth4 eth3 eth2 eth1 */
	ports[0] = "eth0"; ports[1] = "eth4"; ports[2] = "eth3"; ports[3] = "eth2"; ports[4] = "eth1"; ports[5] = "eth5";
#elif defined(RTAX68U)
	int lan_ports = 4;
	char *ports[lan_ports+1];
	/* 4 3 2 1 0	W0 L1 L2 L3 L4 */
	/* eth0 eth4 eth3 eth2 eth1 */
	ports[0] = "eth0"; ports[1] = "eth4"; ports[2] = "eth3"; ports[3] = "eth2"; ports[4] = "eth1";
#elif defined(RTCONFIG_EXTPHY_BCM84880) /* GT-AX11000 */
	int lan_ports=5;
	int ports[lan_ports+1];
	/*
		7 4 3 2 1 0 	L5(2.5G) W0 L1 L2 L3 L4
	*/
	ports[0]=4; ports[1]=3; ports[2]=2; ports[3]=1; ports[4]=0;
	ports[5]=7;
#endif

#ifdef RTCONFIG_EXT_BCM53134
	extra_p0 = S_53134;
#endif

#ifdef HND_ROUTER
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
#ifdef RTCONFIG_HND_ROUTER_AX_6710
	if (!hnd_get_phy_status(ports[port]))				/*Disconnect*/
#elif defined(RTCONFIG_HND_ROUTER_AX_675X)
	if (!hnd_get_phy_status(ports[port]))				/*Disconnect*/
#else
	if (!hnd_get_phy_status(ports[port], extra_p0, regv, pmdv))	/*Disconnect*/
#endif
	{
		port_status = 0;
	}else{
#ifdef RTCONFIG_HND_ROUTER_AX_6710
		ret = hnd_get_phy_speed(ports[port]);
#elif defined(RTCONFIG_HND_ROUTER_AX_675X)
		ret = hnd_get_phy_speed(ports[port]);
#else
		ret = hnd_get_phy_speed(ports[port], extra_p0, regv2, pmdv2);
#endif
		port_status =
#ifdef RTCONFIG_EXTPHY_BCM84880
                 (ret & 4)? 2500 :
#endif
					(ret & 2)? 1000:100;
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
	char caps[WLC_IOCTL_SMLEN * 2];
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
