#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/if_ether.h>		//have to in front of <linux/ethtool.h> and <linux/mii.h> to avoid redefinition of 'struct ethhdr'
#include <linux/mii.h>
#include <bcmnvram.h>

#include "utils.h"
#include "shutils.h"

#include "shared.h"

#if defined(RTCONFIG_BLINK_LED)
#include <bled_defs.h>
#endif

#ifdef RTCONFIG_RALINK
// TODO: make it switch model dependent, not product dependent
#include "rtkswitch.h"
#endif

#if !defined(RTCONFIG_ALPINE)
#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
#include <rtk_switch.h>
#endif
#endif
#ifdef HND_ROUTER
#include "ethswctl.h"
#include "ethctl.h"
#ifdef RTCONFIG_HND_ROUTER_AX_675X
#include "bcmnet.h"
#endif
#endif

int led_control(int which, int mode);

static int gpio_values_loaded = 0;

static int btn_gpio_table[BTN_ID_MAX];
int led_gpio_table[LED_ID_MAX] = {0};

int wan_port = 0xff;
int fan_gpio = 0xff;
#ifdef RTCONFIG_QTN
int reset_qtn_gpio = 0xff;
#endif

static const struct led_btn_table_s {
	char *nv;
	int *p_val;
} led_btn_table[] = {
	/* button */
	{ "btn_rst_gpio",	&btn_gpio_table[BTN_RESET] },
	{ "btn_wps_gpio",	&btn_gpio_table[BTN_WPS] },
#ifdef RTCONFIG_SWMODE_SWITCH
#if defined(PLAC66U)
	{ "btn_swmode1_gpio",	&btn_gpio_table[BTN_SWMODE_SW_ROUTER] },
#else
	{ "btn_swmode1_gpio",	&btn_gpio_table[BTN_SWMODE_SW_ROUTER] },
	{ "btn_swmode2_gpio",	&btn_gpio_table[BTN_SWMODE_SW_REPEATER] },
	{ "btn_swmode3_gpio",	&btn_gpio_table[BTN_SWMODE_SW_AP] },
#endif	/* Model */
#endif	/* RTCONFIG_SWMODE_SWITCH */

#ifdef RTCONFIG_WIRELESS_SWITCH
	{ "btn_wifi_gpio",	&btn_gpio_table[BTN_WIFI_SW] },
#endif
#ifdef RTCONFIG_WIFI_TOG_BTN
	{ "btn_wltog_gpio",	&btn_gpio_table[BTN_WIFI_TOG] },
#endif
#ifdef RTCONFIG_TURBO_BTN
	{ "btn_turbo_gpio",     &btn_gpio_table[BTN_TURBO] },
	{ "led_turbo_gpio",	&led_gpio_table[LED_TURBO] },
#endif
#ifdef RTCONFIG_LED_BTN
	{ "btn_led_gpio",	&btn_gpio_table[BTN_LED] },
#endif
#ifdef RT4GAC55U
	{ "btn_lte_gpio",	&btn_gpio_table[BTN_LTE] },
#endif
#ifdef RTCONFIG_EJUSB_BTN
	{ "btn_ejusb1_gpio",	&btn_gpio_table[BTN_EJUSB1] },
	{ "btn_ejusb2_gpio",	&btn_gpio_table[BTN_EJUSB2] },
#endif
	/* LED */
	{ "led_pwr_gpio",	&led_gpio_table[LED_POWER] },
#if defined(RTCONFIG_PWRRED_LED)
	{ "led_pwr_red_gpio",	&led_gpio_table[LED_POWER_RED] },
#endif
	{ "led_wps_gpio",	&led_gpio_table[LED_WPS] },
	{ "led_2g_gpio",	&led_gpio_table[LED_2G] },
	{ "led_5g_gpio",	&led_gpio_table[LED_5G] },
	{ "led_5g2_gpio",	&led_gpio_table[LED_5G2] },
	{ "led_60g_gpio",	&led_gpio_table[LED_60G] },
#ifdef RTCONFIG_LAN4WAN_LED
	{ "led_lan1_gpio",	&led_gpio_table[LED_LAN1] },
	{ "led_lan2_gpio",	&led_gpio_table[LED_LAN2] },
	{ "led_lan3_gpio",	&led_gpio_table[LED_LAN3] },
	{ "led_lan4_gpio",	&led_gpio_table[LED_LAN4] },
#else
	{ "led_lan_gpio",	&led_gpio_table[LED_LAN] },
#endif
#if defined(RTAX86U) || defined(RTAX5700)
	{ "led_lan_gpio",	&led_gpio_table[LED_LAN] },
#endif
	{ "led_wan_gpio",	&led_gpio_table[LED_WAN] },
#ifdef HND_ROUTER
	{ "led_wan_normal_gpio",&led_gpio_table[LED_WAN_NORMAL] },
#endif
#ifdef RTCONFIG_EXTPHY_BCM84880
	{ "led_extphy_gpio",&led_gpio_table[LED_EXTPHY] },
#endif
#if defined(RTCONFIG_WANLEDX2)
	{ "led_wan2_gpio",	&led_gpio_table[LED_WAN2] },
#endif
#if defined(RTCONFIG_R10G_LED)
	{ "led_r10g_gpio",	&led_gpio_table[LED_R10G] },
#endif
#if defined(RTCONFIG_SFPP_LED)
	{ "led_sfpp_gpio",	&led_gpio_table[LED_SFPP] },
#endif
#if defined(RTCONFIG_FAILOVER_LED)
	{ "led_failover_gpio",	&led_gpio_table[LED_FAILOVER] },
#endif
#if defined(RTCONFIG_M2_SSD)
	{ "led_sata_gpio",	&led_gpio_table[LED_SATA] },
#endif
	{ "led_usb_gpio",	&led_gpio_table[LED_USB] },
	{ "led_usb3_gpio",	&led_gpio_table[LED_USB3] },
#ifdef RTCONFIG_MMC_LED
	{ "led_mmc_gpio",	&led_gpio_table[LED_MMC] },
#endif
#ifdef RTCONFIG_RESET_SWITCH
	{ "reset_switch_gpio",	&led_gpio_table[LED_RESET_SWITCH] },
#endif
#ifdef RTCONFIG_LED_ALL
	{ "led_all_gpio",	&led_gpio_table[LED_ALL] },
#endif
#ifdef RTCONFIG_LOGO_LED
	{ "led_logo_gpio",	&led_gpio_table[LED_LOGO] },
#endif
	{ "led_wan_red_gpio",	&led_gpio_table[LED_WAN_RED] },
#if defined(RTCONFIG_WANLEDX2) && defined(RTCONFIG_WANRED_LED)
	{ "led_wan2_red_gpio",	&led_gpio_table[LED_WAN2_RED] },
#endif
#ifdef RTCONFIG_QTN
	{ "reset_qtn_gpio",	&led_gpio_table[BTN_QTN_RESET] },
#endif
#ifdef RTCONFIG_INTERNAL_GOBI
#if defined(RT4GAC53U)
	{ "led_lteoff_gpio",	&led_gpio_table[LED_LTE_OFF] },
	{ "led_pwr_red_gpio",	&led_gpio_table[LED_POWER_RED] },
#else
	{ "led_3g_gpio",	&led_gpio_table[LED_3G] },
	{ "led_lte_gpio",	&led_gpio_table[LED_LTE] },
#endif
	{ "led_sig1_gpio",	&led_gpio_table[LED_SIG1] },
	{ "led_sig2_gpio",	&led_gpio_table[LED_SIG2] },
	{ "led_sig3_gpio",	&led_gpio_table[LED_SIG3] },
#if defined(RT4GAC53U)
	{ "led_sig4_gpio",	&led_gpio_table[LED_SIG4] },
#endif
#endif

#ifdef BLUECAVE
	{ "led_ctl_sig1_gpio",	&led_gpio_table[LED_CENTRAL_SIG1] },
	{ "led_ctl_sig2_gpio",  &led_gpio_table[LED_CENTRAL_SIG2] },
	{ "led_ctl_sig3_gpio",  &led_gpio_table[LED_CENTRAL_SIG3] },
	{ "led_idr_sig1_gpio",  &led_gpio_table[LED_INDICATOR_SIG1] },
	{ "led_idr_sig2_gpio",  &led_gpio_table[LED_INDICATOR_SIG2] },
#endif

#if defined(RTAC5300) || defined(GTAC5300)
	{ "rpm_fan_gpio",	&led_gpio_table[RPM_FAN] },
#endif
#ifdef RTCONFIG_USB
	{ "pwr_usb_gpio",	&led_gpio_table[PWR_USB] },
#if defined(RTAX89U) || defined(GTAXY16000)
	{ "pwr_usb_gpio2",	&led_gpio_table[PWR_USB2] },
#endif
#endif

#if defined(PLN12) || defined(PLAC56)
	{ "plc_wake_gpio",	&led_gpio_table[PLC_WAKE] },
	{ "led_pwr_red_gpio",	&led_gpio_table[LED_POWER_RED] },
	{ "led_2g_green_gpio",	&led_gpio_table[LED_2G_GREEN] },
	{ "led_2g_orange_gpio",	&led_gpio_table[LED_2G_ORANGE] },
	{ "led_2g_red_gpio",	&led_gpio_table[LED_2G_RED] },
	{ "led_5g_green_gpio",	&led_gpio_table[LED_5G_GREEN] },
	{ "led_5g_orange_gpio",	&led_gpio_table[LED_5G_ORANGE] },
	{ "led_5g_red_gpio",	&led_gpio_table[LED_5G_RED] },
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
	{ "led_blue_gpio",	&led_gpio_table[LED_BLUE] },
	{ "led_green_gpio",	&led_gpio_table[LED_GREEN] },
	{ "led_red_gpio",	&led_gpio_table[LED_RED] },
#if defined(RTAC59_CD6R) || defined(RTAC59_CD6N) || defined(PLAX56_XP4)
	{ "led_white_gpio",	&led_gpio_table[LED_WHITE] },
#endif
#endif
#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400)
	{ "led_group1_red_gpio",	&led_gpio_table[LED_GROUP1_RED] },
	{ "led_group1_green_gpio",	&led_gpio_table[LED_GROUP1_GREEN] },
	{ "led_group1_blue_gpio",	&led_gpio_table[LED_GROUP1_BLUE] },
	{ "led_group2_red_gpio",	&led_gpio_table[LED_GROUP2_RED] },
	{ "led_group2_green_gpio",	&led_gpio_table[LED_GROUP2_GREEN] },
	{ "led_group2_blue_gpio",	&led_gpio_table[LED_GROUP2_BLUE] },
	{ "led_group3_red_gpio",	&led_gpio_table[LED_GROUP3_RED] },
	{ "led_group3_green_gpio",	&led_gpio_table[LED_GROUP3_GREEN] },
	{ "led_group3_blue_gpio",	&led_gpio_table[LED_GROUP3_BLUE] },
	{ "led_group4_red_gpio",	&led_gpio_table[LED_GROUP4_RED] },
	{ "led_group4_green_gpio",	&led_gpio_table[LED_GROUP4_GREEN] },
	{ "led_group4_blue_gpio",	&led_gpio_table[LED_GROUP4_BLUE] },
#endif
#if defined(GSAX3000) || defined(GSAX5400)
	{ "led_group5_red_gpio",	&led_gpio_table[LED_GROUP5_RED] },
	{ "led_group5_green_gpio",	&led_gpio_table[LED_GROUP5_GREEN] },
	{ "led_group5_blue_gpio",	&led_gpio_table[LED_GROUP5_BLUE] },
#endif

#ifdef RPAC53
	{ "led_pwr_red_gpio",	&led_gpio_table[LED_POWER_RED] },
	{ "led_2g_orange_gpio",	&led_gpio_table[LED_2G_ORANGE] },
	{ "led_2g_green_gpio",	&led_gpio_table[LED_2G_GREEN] },
	{ "led_2g_red_gpio",	&led_gpio_table[LED_2G_RED] },
	{ "led_5g_orange_gpio",	&led_gpio_table[LED_5G_ORANGE] },
	{ "led_5g_green_gpio",	&led_gpio_table[LED_5G_GREEN] },
	{ "led_5g_red_gpio",	&led_gpio_table[LED_5G_RED] },
#endif
#ifdef RPAC66
	{ "led_pwr_orange_gpio",&led_gpio_table[LED_ORANGE_POWER] },
	{ "led_2g_blue_gpio",	&led_gpio_table[LED_2G_BLUE] },
	{ "led_2g_green_gpio",	&led_gpio_table[LED_2G_GREEN] },
	{ "led_2g_red_gpio",	&led_gpio_table[LED_2G_RED] },
	{ "led_5g_blue_gpio",	&led_gpio_table[LED_5G_BLUE] },
	{ "led_5g_green_gpio",	&led_gpio_table[LED_5G_GREEN] },
	{ "led_5g_red_gpio",	&led_gpio_table[LED_5G_RED] },
#endif
#if defined(RPAC51)
	{ "led_pwr_red_gpio",&led_gpio_table[LED_RED_POWER] },
	{ "led_single_gpio",	&led_gpio_table[LED_SINGLE] },
	{ "led_near_gpio",	&led_gpio_table[LED_NEAR] },
	{ "led_far_gpio",	&led_gpio_table[LED_FAR] },
#endif
#ifdef RPAC87
	{ "led_2g_green_gpio1",	&led_gpio_table[LED_2G_GREEN1] },
	{ "led_2g_green_gpio2",	&led_gpio_table[LED_2G_GREEN2] },
	{ "led_2g_green_gpio3",	&led_gpio_table[LED_2G_GREEN3] },
	{ "led_2g_green_gpio4",	&led_gpio_table[LED_2G_GREEN4] },

	{ "led_5g_green_gpio1",	&led_gpio_table[LED_5G_GREEN1] },
	{ "led_5g_green_gpio2",	&led_gpio_table[LED_5G_GREEN2] },
	{ "led_5g_green_gpio3",	&led_gpio_table[LED_5G_GREEN3] },
	{ "led_5g_green_gpio4",	&led_gpio_table[LED_5G_GREEN4] },
#endif
#ifdef RPAC55
	{ "led_pwr_red_gpio",	&led_gpio_table[LED_POWER_RED] },
	{ "led_wifi_gpio",		&led_gpio_table[LED_WIFI] },
	{ "led_sig1_gpio",		&led_gpio_table[LED_SIG1] },
	{ "led_sig2_gpio",		&led_gpio_table[LED_SIG2] },
#endif
#ifdef RPAC92
	{ "led_pwr_red_gpio",	&led_gpio_table[LED_POWER_RED] },
	{ "led_wifi_gpio",		&led_gpio_table[LED_WIFI] },
	{ "led_sig1_gpio",		&led_gpio_table[LED_SIG1] },
	{ "led_sig2_gpio",		&led_gpio_table[LED_SIG2] },
	{ "led_purple_gpio",		&led_gpio_table[LED_PURPLE] },
#endif		
#ifdef RPAX56
	{ "led_red_gpio",       &led_gpio_table[LED_RED_GPIO] },
	{ "led_green_gpio",     &led_gpio_table[LED_GREEN_GPIO] },
	{ "led_blue_gpio",      &led_gpio_table[LED_BLUE_GPIO] },
	{ "led_white_gpio",     &led_gpio_table[LED_WHITE_GPIO] },
	{ "led_yellow_gpio",    &led_gpio_table[LED_YELLOW_GPIO] },
	{ "led_purple_gpio",    &led_gpio_table[LED_PURPLE_GPIO] },
#endif
#if defined(RTAX95Q) || defined(RTAX56_XD4) || defined(RTAX82_XD6)
	{ "bt_rst_gpio",        &led_gpio_table[BT_RESET] },
	{ "bt_disable_gpio",    &led_gpio_table[BT_DISABLE] },
	{ "led_rgb1_red_gpio",  &led_gpio_table[LED_RGB1_RED] },
	{ "led_rgb1_green_gpio",        &led_gpio_table[LED_RGB1_GREEN] },
	{ "led_rgb1_blue_gpio", &led_gpio_table[LED_RGB1_BLUE] },
#endif
#if defined(CTAX56_XD4)
	{ "led_rgb1_red_gpio",  &led_gpio_table[LED_RGB1_RED] },
	{ "led_rgb1_green_gpio",        &led_gpio_table[LED_RGB1_GREEN] },
	{ "led_rgb1_blue_gpio", &led_gpio_table[LED_RGB1_BLUE] },
#endif
#if defined(RTAX56_XD4)
	{ "btn_bt_indicator_gpio",        &led_gpio_table[IND_BT] },
	{ "btn_pa_indicator_gpio",    &led_gpio_table[IND_PA] },
#endif
#if defined(DSL_AX82U)
	{ "led_wifi_gpio",		&led_gpio_table[LED_WIFI] },
#endif

	{ NULL, NULL },
};

int extract_gpio_pin(const char *gpio)
{
	char *p;

	if (!gpio || !(p = nvram_get(gpio)))
		return -1;

	return (atoi(p) & GPIO_PIN_MASK);
}

int init_gpio(void)
{
	char *btn_list[] = { "btn_rst_gpio", "btn_wps_gpio", "fan_gpio", "have_fan_gpio"
#ifdef RTCONFIG_WIRELESS_SWITCH
		, "btn_wifi_gpio"
#endif
#ifdef RTCONFIG_WIFI_TOG_BTN
		, "btn_wltog_gpio"
#endif
#ifdef RTCONFIG_SWMODE_SWITCH
#if defined(PLAC66U)
		, "btn_swmode1_gpio"
#else
		, "btn_swmode1_gpio", "btn_swmode2_gpio", "btn_swmode3_gpio"
#endif	/* Mode */
#endif	/* RTCONFIG_SWMODE_SWITCH */
#ifdef RTCONFIG_TURBO_BTN
		, "btn_turbo_gpio"
#endif
#ifdef RTCONFIG_LED_BTN
		, "btn_led_gpio"
#endif
#ifdef RT4GAC55U
		, "btn_lte_gpio"
#endif
#ifdef RTCONFIG_EJUSB_BTN
		, "btn_ejusb1_gpio", "btn_ejusb2_gpio"
#endif
	};
	char *led_list[] = { "led_pwr_gpio", "led_usb_gpio", "led_wps_gpio", "fan_gpio", "have_fan_gpio", "led_wan_gpio", "led_usb3_gpio", "led_2g_gpio", "led_5g_gpio"
#if defined(RTCONFIG_HAS_5G_2)
		, "led_5g2_gpio"
#endif
#if defined(RTCONFIG_WIGIG)
		, "led_60g_gpio"
#endif
#ifdef HND_ROUTER
		, "led_wan_normal_gpio"
#endif
#ifdef RTCONFIG_EXTPHY_BCM84880
		, "led_extphy_gpio"
#endif
#if defined(RTCONFIG_PWRRED_LED)
		, "led_pwr_red_gpio"
#endif
#ifdef RTCONFIG_TURBO_BTN
		, "led_turbo_gpio"
#endif
#ifdef RTCONFIG_LOGO_LED
		, "led_logo_gpio"
#endif
#ifdef RTCONFIG_LAN4WAN_LED
		, "led_lan1_gpio", "led_lan2_gpio", "led_lan3_gpio", "led_lan4_gpio"
#else
		, "led_lan_gpio"
#endif
#ifdef RTCONFIG_LED_ALL
		, "led_all_gpio"
#endif
#if defined(RTCONFIG_WANLEDX2)
		, "led_wan2_gpio"
#endif
#if defined(RTCONFIG_WANRED_LED)
		, "led_wan_red_gpio"
#if defined(RTCONFIG_WANLEDX2)
		, "led_wan2_red_gpio"
#endif
#endif
#if defined(RTCONFIG_R10G_LED)
		, "led_r10g_gpio"
#endif
#if defined(RTCONFIG_SFPP_LED)
		, "led_sfpp_gpio"
#endif
#ifdef RTCONFIG_QTN
		, "reset_qtn_gpio"
#endif
		, "pwr_usb_gpio"
#if defined(RTCONFIG_USBRESET) \
 || defined(RTAX89U) || defined(GTAXY16000)
		, "pwr_usb_gpio2"
#endif
#ifdef RTCONFIG_WIFIPWR
		, "pwr_2g_gpio"
		, "pwr_5g_gpio"
#endif
#ifdef RTCONFIG_INTERNAL_GOBI
		, "led_3g_gpio", "led_lte_gpio", "led_sig1_gpio", "led_sig2_gpio", "led_sig3_gpio"
#endif
#if defined(RTCONFIG_FAILOVER_LED)
		, "led_failover_gpio"
#endif
#if defined(RTCONFIG_M2_SSD)
		, "led_sata_gpio"
#endif
#if defined(RTCONFIG_FAILOVER_LED)
		, "led_failover_gpio"
#endif
#if defined(RTCONFIG_M2_SSD)
		, "led_sata_gpio"
#endif
#if (defined(PLN12) || defined(PLAC56))
		, "plc_wake_gpio"
		, "led_pwr_red_gpio"
		, "led_2g_green_gpio", "led_2g_orange_gpio", "led_2g_red_gpio"
		, "led_5g_green_gpio", "led_5g_orange_gpio", "led_5g_red_gpio"
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
		, "led_blue_gpio", "led_green_gpio", "led_red_gpio"
#if defined(RTAC59_CD6R) || defined(RTAC59_CD6N) || defined(PLAX56_XP4)
		, "led_white_gpio"
#endif
#endif
#ifdef RPAC53
		, "led_pwr_red_gpio"
		, "led_2g_green_gpio", "led_2g_orange_gpio", "led_2g_red_gpio"
		, "led_5g_green_gpio", "led_5g_orange_gpio", "led_5g_red_gpio"
#endif
#if defined(RPAC66)
		, "led_pwr_orange_gpio" , "led_2g_blue_gpio", "led_2g_green_gpio", "led_2g_red_gpio"
		, "led_5g_blue_gpio", "led_5g_green_gpio", "led_5g_red_gpio"
#endif
#if defined(RPAC51)
		, "led_pwr_red_gpio" , "led_single_gpio", "led_far_gpio", "led_near_gpio"
#endif
#ifdef RPAC87
		, "led_2g_green_gpio1" , "led_2g_green_gpio2", "led_2g_green_gpio3", "led_2g_green_gpio4"
		, "led_5g_green_gpio1", "led_5g_green_gpio2", "led_5g_green_gpio3", "led_5g_green_gpio4"
#endif
#ifdef RPAC55
		, "led_pwr_red_gpio"
		, "led_wifi_gpio", "led_sig1_gpio", "led_sig2_gpio"
#endif
#ifdef RPAC92
		, "led_pwr_red_gpio"
		, "led_wifi_gpio", "led_sig1_gpio", "led_sig2_gpio", "led_purple_gpio"
#endif
#ifdef RPAX56
		, "led_red_gpio", "led_green_gpio", "led_blue_gpio", "led_white_gpio", "led_yellow_gpio", "led_purple_gpio"
#endif
#ifdef BLUECAVE
		, "led_ctl_sig1_gpio", "led_ctl_sig2_gpio", "led_ctl_sig3_gpio"
		, "led_idr_sig1_gpio", "led_idr_sig2_gpio"
#endif			
#ifdef RTCONFIG_MMC_LED
		, "led_mmc_gpio"
#endif
#if defined(RTAC5300) || defined(GTAC5300)
		, "rpm_fan_gpio"
#endif
#ifdef RTCONFIG_RESET_SWITCH
		, "reset_switch_gpio"
#endif
			   };
	int use_gpio, gpio_pin;
	int enable, disable;
	int i;

#ifdef RT4GAC55U
	void get_gpio_values_once(int);
	get_gpio_values_once(0);		// for filling data to led_gpio_table[]
#endif

#ifdef BLUECAVE
	_dprintf("BLUECAVE: skip init_gpio()\n");
	return 0;
#endif

	/* btn input */
	for(i = 0; i < ASIZE(btn_list); i++)
	{
		if (!nvram_get(btn_list[i]))
			continue;
		use_gpio = nvram_get_int(btn_list[i]);
		if((gpio_pin = use_gpio & 0xff) == 0xff)
			continue;
		gpio_dir(gpio_pin, GPIO_DIR_IN);
	}

	/* led output */
	for(i = 0; i < ASIZE(led_list); i++)
	{
		if (!nvram_get(led_list[i]))
			continue;
#if defined(RTCONFIG_ETRON_XHCI_USB3_LED)
		if (!strcmp(led_list[i], "led_usb3_gpio") && nvram_match("led_usb3_gpio", "etron")) {
			led_control(LED_USB3, LED_OFF);
			continue;
		}
#endif
#if defined(BRTAC828)
		if ((!strcmp(led_list[i], "led_wan_gpio") && nvram_match("led_wan_gpio", "qca8033")) ||
		    (!strcmp(led_list[i], "led_wan2_gpio") && nvram_match("led_wan2_gpio", "qca8033"))) {
			continue;
		}
#endif
		use_gpio = nvram_get_int(led_list[i]);

		if((gpio_pin = use_gpio & 0xff) == 0xff)
			continue;
#if defined(RTCONFIG_RALINK_MT7620)
		if(gpio_pin == 72)	//skip 2g wifi led NOT to be gpio LED
			continue;
#endif

		disable = (use_gpio&GPIO_ACTIVE_LOW)==0 ? 0: 1;
#ifndef RTCONFIG_LEDS_CLASS
		gpio_dir(gpio_pin, GPIO_DIR_OUT);
#endif

#if defined(RTCONFIG_WANLEDX2)
		/* Turn on WAN RED LED at system start-up if and only if coresponding WAN unit is enabled. */
		if (is_router_mode()) {
			if ((!strcmp(led_list[i], "led_wan_red_gpio") && get_dualwan_by_unit(0) != WANS_DUALWAN_IF_NONE) ||
			    (!strcmp(led_list[i], "led_wan2_red_gpio") && get_dualwan_by_unit(1) != WANS_DUALWAN_IF_NONE))
				disable = !disable;
		}
#else
#if defined(RTCONFIG_WANRED_LED)
		/* If WAN RED LED is defined, keep it on until Internet connection ready in router mode. */
		if (!strcmp(led_list[i], "led_wan_red_gpio") && is_router_mode())
		{
			disable = !disable;
#if defined(RTCONFIG_DSL_HOST) && defined(RTCONFIG_DUALWAN)
			if (get_wans_dualwan() & WANSCAP_DSL)
				disable = 1;
#endif
		}
#endif
#endif

#if !defined(RTCONFIG_CONCURRENTREPEATER)
		set_gpio(gpio_pin, disable);
#endif

#ifdef RT4GAC55U	// save setting value
		{ int i; char led[16]; for(i=0; i<LED_ID_MAX; i++) if(gpio_pin == (led_gpio_table[i]&0xff)){snprintf(led, sizeof(led), "led%02d", i); nvram_set_int(led, LED_OFF); break;}}
#endif
	}

#if (defined(PLN12) || defined(PLAC56))
	if((gpio_pin = (use_gpio = nvram_get_int("led_pwr_red_gpio")) & 0xff) != 0xff)
#elif defined(MAPAC1750)
	if((gpio_pin = (use_gpio = nvram_get_int("led_blue_gpio")) & 0xff) != 0xff)
#elif defined(RTAC59_CD6R) || defined(RTAC59_CD6N) || defined(PLAX56_XP4)
	if((gpio_pin = (use_gpio = nvram_get_int("led_green_gpio")) & 0xff) != 0xff)
#else
	if((gpio_pin = (use_gpio = nvram_get_int("led_pwr_gpio")) & 0xff) != 0xff)
#endif
	{
		enable = (use_gpio&GPIO_ACTIVE_LOW)==0 ? 1 : 0;
#if !defined(RTCONFIG_CONCURRENTREPEATER)
#ifdef RTCONFIG_SW_CTRL_ALLLED
		if (nvram_match("AllLED", "1"))
#endif
			set_gpio(gpio_pin, enable);
#endif
#ifdef RT4GAC55U	// save setting value
		{ int i; char led[16]; for(i=0; i<LED_ID_MAX; i++) if(gpio_pin == (led_gpio_table[i]&0xff)){snprintf(led, sizeof(led), "led%02d", i); nvram_set_int(led, LED_ON); break;}}
#endif
	}

	// Power of USB.
	if((gpio_pin = (use_gpio = nvram_get_int("pwr_usb_gpio")) & 0xff) != 0xff){
		enable = (use_gpio&GPIO_ACTIVE_LOW)==0 ? 1 : 0;
		set_gpio(gpio_pin, enable);
	}
	if((gpio_pin = (use_gpio = nvram_get_int("pwr_usb_gpio2")) & 0xff) != 0xff){
		enable = (use_gpio&GPIO_ACTIVE_LOW)==0 ? 1 : 0;
		set_gpio(gpio_pin, enable);
	}

#if defined(RTAC5300) || defined(GTAC5300)
	// RPM of FAN
	if((gpio_pin = (use_gpio = nvram_get_int("rpm_fan_gpio")) & 0xff) != 0xff){
	enable = (use_gpio&GPIO_ACTIVE_LOW)==0 ? 1 : 0;
	set_gpio(gpio_pin, enable);
	}
#endif

#ifdef PLAC56
	if((gpio_pin = (use_gpio = nvram_get_int("plc_wake_gpio")) & 0xff) != 0xff){
		enable = (use_gpio&GPIO_ACTIVE_LOW)==0 ? 1 : 0;
		set_gpio(gpio_pin, enable);
	}
#endif

	// TODO: system dependent initialization
	return 0;
}
#ifdef HND_ROUTER
int _gpio_active_low(int gpionr)
{
	int i;

	if(led_gpio_table[LED_POWER]<=0)
		get_gpio_values_once(0);

	for(i=0; i<LED_ID_MAX; i++)
		if(gpionr == (led_gpio_table[i] & 0xff))
			return led_gpio_table[i] & GPIO_ACTIVE_LOW;

	for(i=0; i<BTN_ID_MAX; i++)
		if(gpionr == (btn_gpio_table[i] & 0xff))
			return btn_gpio_table[i] & GPIO_ACTIVE_LOW;

	return -1;
}

void
dump_ledtable()
{
	int i;

	if(!nvram_match("dbg", "1"))
		return;

	for(i=0; i<LED_ID_MAX; i++)
		printf("tb[%d]=%04x\n", i, led_gpio_table[i]);

	for(i=0; i<BTN_ID_MAX; i++)
		printf("tb_btn[%d]=%04x\n", i, btn_gpio_table[i]);
}
#endif

#ifdef RTCONFIG_BCMARM
int set_pwr_usb(int boolOn) {
	int use_gpio, gpio_pin;

#ifdef RTAC68U
	switch(get_model()) {
		case MODEL_RTAC68U:
			if (!hw_usb_cap())
				return 0;
			if ((nvram_get_int("HW_ver") != 170) &&
			    (nvram_get_int("HW_ver") != 180) &&
			    (nvram_get_double("HW_ver") != 1.10) &&
			    (nvram_get_double("HW_ver") != 1.85) &&
			    (nvram_get_double("HW_ver") != 1.90) &&
			    (nvram_get_double("HW_ver") != 1.95) &&
			    (nvram_get_double("HW_ver") != 2.10) &&
			    (nvram_get_double("HW_ver") != 2.20) &&
			    (nvram_get_double("HW_ver") != 3.00) &&
			    !is_ac66u_v2_series() &&
			    !nvram_match("cpurev", "c0"))
				return 0;
			break;
	}
#endif

	if ((gpio_pin = (use_gpio = nvram_get_int("pwr_usb_gpio"))&0xff) != 0xff) {
		if (boolOn)
			set_gpio(gpio_pin, 1);
		else
			set_gpio(gpio_pin, 0);
	}

	if ((gpio_pin = (use_gpio = nvram_get_int("pwr_usb_gpio2"))&0xff) != 0xff) {
		if (boolOn)
			set_gpio(gpio_pin, 1);
		else
			set_gpio(gpio_pin, 0);
	}

	return 0;
}
#else
int set_pwr_usb(int boolOn) {
	return 0;
}
#endif

#ifdef RT4GAC68U
int set_pwr_modem(int boolOn){
	int gpio_pin = 6;

	if(boolOn)
		set_gpio(gpio_pin, 1);
	else
		set_gpio(gpio_pin, 0);

	return 0;
}
#endif

/* Return GPIO# of specified nvram variable.
 * @return:
 * 	-1:	nvram variables doesn't not exist.
 */
static int __get_gpio(char *name)
{
	if (!nvram_get(name))
		return -1;

	return nvram_get_int(name);
}

// this is shared by every process, so, need to get nvram for first time it called per process
void get_gpio_values_once(int force)
{
	int i;
	const struct led_btn_table_s *p;

	if (gpio_values_loaded && !force) return;

	gpio_values_loaded = 1;

	// TODO : add other models
	for (i = 0; i < ARRAY_SIZE(led_gpio_table); ++i) {
		led_gpio_table[i] = -1;
	}

	for (p = &led_btn_table[0]; p->p_val; ++p)
		*(p->p_val) = __get_gpio(p->nv);

}

int button_pressed(int which)
{
	int use_gpio;
	int gpio_value;

	if (which < 0 || which >= BTN_ID_MAX)
		return -1;

	get_gpio_values_once(0);
	use_gpio = btn_gpio_table[which];
	if((use_gpio&0xff)!=0x0ff)
	{
		gpio_value = get_gpio(use_gpio&0xff);

		//_dprintf("use_gpio: %x gpio_value: %x\n", use_gpio, gpio_value);

		if((use_gpio&GPIO_ACTIVE_LOW)==0) // active high case
			return gpio_value==0 ? 0 : 1;
		else
			return gpio_value==0 ? 1 : 0;
	}
	else return 0;
}

#if defined(RTAX86U) || defined(RTAX5700)
void config_ext_wan_led(int onoff) {
	unsigned int val = 0;
	unsigned int mask = 0x200000; // bit20
	char buf1[32], buf2[32], *ptr;
	char *reg1 = "0xFF800528";
	char *rd_argv[] = {"dw", reg1, NULL};
	char *wr_argv[] = {"sw", reg1, buf1, NULL};

	if(onoff != 0 && onoff != 1)
		return;

	_eval(rd_argv, ">/tmp/output_reg", 0, NULL);

	memset(buf1, 0, sizeof(buf1));
	f_read("/tmp/output_reg", buf1, sizeof(buf1));

	if((ptr = strstr(buf1, reg1)) == NULL)
		return;
	snprintf(buf2, sizeof(buf2), "%s", ptr);
	ptr = buf2+strlen(reg1)+3;

	val = strtoul(ptr, NULL, 16);

	if(onoff)
		val &= ~mask;
	else
		val |= mask;

	snprintf(buf1, sizeof(buf1), "0x%x", val);

	_eval(wr_argv, NULL, 0, NULL);

	return;
}
#endif

int led_control(int which, int mode)
#ifdef RT4GAC55U
{ //save value
	char name[16];

	snprintf(name, sizeof(name), "led%02d", which);
	if(nvram_get_int(name) != mode)
		nvram_set_int(name, mode);

	if (nvram_match(LED_CTRL_HIPRIO, "1"))
		return 0;

	int do_led_control(int which, int mode);
	return do_led_control(which, mode);
}

int do_led_control(int which, int mode)
#endif
{
	int use_gpio, gpio_nr;
	int v = (mode == LED_OFF)? 0:1;

	// Did the user disable the leds?
	if ((mode == LED_ON) && (nvram_get_int("led_disable") == 1)
#ifdef RTCONFIG_QTN
		&& (which != BTN_QTN_RESET)
#endif
	)
	{
		return 0;
	}

	if (which < 0 || which >= LED_ID_MAX || mode < 0 || mode >= LED_FAN_MODE_MAX)
		return -1;

#if defined(RTAX86U) || defined(RTAX5700)
	if(which == LED_LAN){
		config_ext_wan_led(mode);
		return 0;
	}
#endif

	get_gpio_values_once(0);
	use_gpio = led_gpio_table[which];
	gpio_nr = use_gpio & 0xFF;

#if defined(RTCONFIG_ETRON_XHCI_USB3_LED)
	if (which == LED_USB3 && nvram_match("led_usb3_gpio", "etron")) {
		char *onoff = "2";	/* LED OFF */

		if (mode == LED_ON || mode == FAN_ON || mode == HAVE_FAN_ON)
			onoff = "3";	/* LED ON */

		f_write_string("/sys/bus/pci/drivers/xhci_hcd/port_led", onoff, 0, 0);
		return 0;
	}
#endif

#if defined(RTAC56U) || defined(RTAC56S)
	if (which == LED_5G && nvram_match("5g_fail", "1"))
		return -1;
#endif

	if (use_gpio < 0 || gpio_nr == 0xFF)
		return -1;

	if (use_gpio & GPIO_ACTIVE_LOW)
		v ^= 1;

#ifndef HND_ROUTER
	if (mode == LED_OFF) {
		stop_bled(use_gpio);
	}
#endif
	set_gpio(gpio_nr, v);
#ifndef HND_ROUTER
	if (mode == LED_ON) {
		start_bled(use_gpio);
	}
#endif
	return 0;
}

#ifdef RT4GAC55U
void led_control_lte(int percent)
{
	if(percent >= 0)
	{ //high priority led for LTE
		int LTE_LED[] = { LED_SIG1, LED_SIG2, LED_SIG3, LED_USB, LED_LAN, LED_2G, LED_5G, LED_WAN, LED_LTE, LED_POWER };
		int i;
		nvram_set(LED_CTRL_HIPRIO, "1");
		for(i = 0; i < ARRAY_SIZE(LTE_LED); i++)
		{
			if((percent/9) > i)
				do_led_control(LTE_LED[i], LED_ON);
			else
				do_led_control(LTE_LED[i], LED_OFF);
		}
	}
	else if(nvram_match(LED_CTRL_HIPRIO, "1"))
	{ //restore
		int which, mode;
		char name[16];
		char *p;

		nvram_unset(LED_CTRL_HIPRIO);

		for(which = 0; which < LED_ID_MAX; which++)
		{
			sprintf(name, "led%02d", which);
			if ((p = nvram_get(name)) != NULL)
			{
				mode = atoi(p);
				do_led_control(which, mode);
			}
		}
	}
}
#endif	/* RT4GAC55U */

#ifdef RTCONFIG_AMAS
int get_port_status(int unit)
{
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK)
	return rtkswitch_wanPort_phyStatus(unit);
#elif defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
	return get_phy_status(unit);
#else
	int mask = 0;

#ifdef HND_ROUTER
	int i, ret = 0;
#ifndef RTCONFIG_HND_ROUTER_AX_675X
	int extra_p0 = 0;
	unsigned int regv=0, pmdv=0;

#ifdef RTCONFIG_EXT_BCM53134
	regv = hnd_ethswctl(REGACCESS, 0x0100, 2, 0, 0);
	pmdv = hnd_ethswctl(PMDIOACCESS, 0x0100, 2, 0, 0);
#else
	regv = hnd_ethswctl(REGACCESS, 0x0100, 2, 0, 0) & 0xf;
#endif

#ifdef RTCONFIG_EXT_BCM53134
	switch(get_model()) {
		case MODEL_GTAC5300:
			extra_p0 = S_53134;
			break;
	}
#endif
#endif

#if defined(RTCONFIG_HND_ROUTER_AX_6710)
	char word[100], *next;

	foreach(word, nvram_safe_get("wan_ifnames"), next)
		ret |= hnd_get_phy_status(word);

	foreach(word, nvram_safe_get("lan_ifnames"), next)
		ret |= hnd_get_phy_status(word);
#else
	for(i = 0; i < 9; ++i){
		if(mask & 1<<i) {
#ifdef RTCONFIG_HND_ROUTER_AX_675X
			ret |= hnd_get_phy_status(i);
#else
			ret |= hnd_get_phy_status(i, extra_p0, regv, pmdv);
#endif
		}
	}
#endif
	return ret;
#else
	mask |= (0x0001<<unit);

	return get_phy_status(mask);
#endif
#endif
}
#endif

int wanport_status(int wan_unit)
{
#if defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA) || defined(RTCONFIG_REALTEK)
	return rtkswitch_wanPort_phyStatus(wan_unit);
#elif defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
#ifdef RTCONFIG_AMAS
	if(sw_mode() == SW_MODE_AP && nvram_get_int("re_mode") == 1)
		return get_amas_eth_phy_status(wan_unit);
#endif	
	return get_phy_status(wan_unit);
#else // Broadcom
#if defined(RTCONFIG_HND_ROUTER_AX_6710)
	if(!is_router_mode())
		return hnd_get_phy_status(nvram_safe_get("eth_ifnames"));
	else
		return hnd_get_phy_status(get_wanx_ifname(wan_unit));
#elif defined(RTCONFIG_HND_ROUTER_AX_675X)
	int i = 0;
	char word[100], *next;

	foreach(word, nvram_safe_get("wanports"), next) {
		if (i == wan_unit)
			return hnd_get_phy_status(atoi(word));

		i++;
	}

	return 0;
#else // RTCONFIG_HND_ROUTER_AX_675X
	char word[100], *next;
	int mask;
	char wan_ports[16];
#ifdef HND_ROUTER
	int i, ret = 0, extra_p0 = 0;
	unsigned int regv=0, pmdv=0;
	char word2[100], *next2;
#endif

	memset(wan_ports, 0, 16);
	mask = 0;

#ifdef HND_ROUTER
	if(sw_mode() == SW_MODE_AP && nvram_get_int("re_mode") == 0){
		strcpy(wan_ports, "lanports");

		foreach(word, nvram_safe_get(wan_ports), next) {
			mask |= (0x0001<<atoi(word));
			if(sw_mode() == SW_MODE_AP)
				break;
		}
	}
	else{
		strcpy(wan_ports, "wanports");
		i = 0;
		foreach(word2, nvram_safe_get(wan_ports), next2){
			if(i == wan_unit)
				break;

			++i;
		}

		mask |= (0x0001<<atoi(word2));
#ifdef RTCONFIG_BONDING_WAN
		if (nvram_match("bond_wan", "1")){
			memset(wan_ports, 0, 16);
			mask = 0;
			strcpy(wan_ports, "wanports_bond");
			foreach(word2, nvram_safe_get(wan_ports), next2){
				mask |= (0x0001<<atoi(word2));
				// _dprintf("wan_bond: port mask[%08X]\n", mask);
			}
		}
#endif
	}
#else // HND_ROUTER
#ifndef RTN53
	if(sw_mode() == SW_MODE_AP && nvram_get_int("re_mode") == 0)
		strcpy(wan_ports, "lanports");
	else
#endif
	if(wan_unit == 1)
		strcpy(wan_ports, "wan1ports");
	else
		strcpy(wan_ports, "wanports");

	foreach(word, nvram_safe_get(wan_ports), next) {
		mask |= (0x0001<<atoi(word));
		if(sw_mode() == SW_MODE_AP)
			break;
	}
#endif // HND_ROUTER

#ifdef RTCONFIG_WIRELESSWAN
	// to do for report wireless connection status
	if(is_wirelesswan_enabled())
		return 1;
#endif

#ifdef HND_ROUTER
#ifdef RTCONFIG_EXT_BCM53134
	regv = hnd_ethswctl(REGACCESS, 0x0100, 2, 0, 0);
	pmdv = hnd_ethswctl(PMDIOACCESS, 0x0100, 2, 0, 0);
#else
	regv = hnd_ethswctl(REGACCESS, 0x0100, 2, 0, 0) & 0xf;
#endif

#ifdef RTCONFIG_EXT_BCM53134
	switch(get_model()) {
		case MODEL_GTAC5300:
		case MODEL_RTAX88U:
		case MODEL_RTAX95Q:
		case MODEL_GTAX11000:
		case MODEL_GTAXE11000:
			extra_p0 = S_53134;
			break;
	}
#endif

	for(i = 0; i < 9; ++i){
		if(mask & 1<<i) {
			ret |= hnd_get_phy_status(i, extra_p0, regv, pmdv);
		}
	}
	return ret;
#else // HND_ROUTER
	return get_phy_status(mask);
#endif // HND_ROUTER
#endif	/* RTCONFIG_HND_ROUTER_AX_675X */
#endif
}

int wanport_speed(void)
{
	char word[100], *next;
	int mask;

	mask = 0;

	foreach(word, nvram_safe_get("wanports"), next) {
		mask |= (0x0003<<(atoi(word)*2));
	}

#ifdef RTCONFIG_WIRELESSWAN
	if(is_wirelesswan_enabled())
		return 0x01;
#endif
	return get_phy_speed(mask);
}

#ifdef RTCONFIG_HND_ROUTER_AX_675X
int ethctl_set_phy(char *ifname, int ctrl)
{
	struct ifreq ifr;
	struct ethctl_data ethctl;
	int skfd;
	int ret = 0;

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		fprintf(stderr, "socket open error\n");
		return -1;
	}

	strcpy(ifr.ifr_name, ifname);

	memset(&ethctl, 0, sizeof(ethctl));
	ifr.ifr_data = &ethctl;
	ethctl.op = ctrl==1?ETHSETPHYPWRON:ctrl==0?ETHSETPHYPWROFF:ETHGETPHYPWR;
	ethctl.sub_port = -1;   // when no port specified
	ethctl.phy_addr = 0;

	ret = ioctl(skfd, SIOCETHCTLOPS, &ifr);
	if (ret) {
		fprintf(stderr, "command error, op=%d ret=%d\n", ethctl.op, ret);
		return ret;
	}

	return 0;
}
#endif

int wanport_ctrl(int ctrl)
{
#ifdef RPAX56
	return ethctl_set_phy("eth0", ctrl);
#endif
#ifdef RTCONFIG_RALINK

#ifdef RTCONFIG_DSL
	/* FIXME: Not implemented yet. */
	return 1;
#else
	if(ctrl) rtkswitch_WanPort_linkUp();
	else rtkswitch_WanPort_linkDown();
	return 1;
#endif
	return 1;
#elif defined(RTCONFIG_QCA)
	if(ctrl)
		rtkswitch_WanPort_linkUp();
	else
		rtkswitch_WanPort_linkDown();
	return 1;
#else
	char word[100], *next;
	int mask;

	mask = 0;

	foreach(word, nvram_safe_get("wanports"), next) {
		mask |= (0x0001<<atoi(word));
	}
#ifdef RTCONFIG_WIRELESSWAN
	// TODO for enable/disable wireless radio
	if(is_wirelesswan_enabled())
		return 0;
#endif
	return set_phy_ctrl(mask, ctrl);
#endif
}

int lanport_status(void)
{
// turn into a general way?
#ifdef RTCONFIG_RALINK

#ifdef RTCONFIG_DSL
	//DSL has no software controlled LAN LED
	return 0;
#else
	return rtkswitch_lanPorts_phyStatus();
#endif

#elif defined(RTCONFIG_QCA)
	return rtkswitch_lanPorts_phyStatus();
#elif defined(RTAX55) || defined(RTAX1800)
	return rtkswitch_lanPorts_phyStatus();
#elif defined(RTCONFIG_HND_ROUTER_AX_675X)
	int status = 0;
	char word[16] = {0};
	char *next = NULL;
	foreach(word, nvram_safe_get("lanports"), next) {
		status |= hnd_get_phy_status(atoi(word));
	}
	return status;
#else
	char word[100], *next;
	int mask;

	mask = 0;

	foreach(word, nvram_safe_get("lanports"), next) {
		mask |= (0x0001<<atoi(word));
	}
	return get_phy_status(mask);
#endif
}

int lanport_speed(void)
{
#ifdef RTCONFIG_RALINK
	return get_phy_speed(0);	/* FIXME */
#elif defined(RTCONFIG_QCA)
	return get_phy_speed(0);	/* FIXME */
#else
	char word[100], *next;
	int mask;

	mask = 0;

	foreach(word, nvram_safe_get("lanports"), next) {
		mask |= (0x0003<<(atoi(word)*2));
	}
	return get_phy_speed(mask);
#endif
}

int lanport_ctrl(int ctrl)
{
#ifdef RPAX56
	return ethctl_set_phy("eth0", ctrl);
#endif
	// no general way for ralink platform, so individual api for each kind of switch are used
#ifdef RTCONFIG_RALINK

	if(ctrl) rtkswitch_LanPort_linkUp();
	else rtkswitch_LanPort_linkDown();
	return 1;

#elif defined(RTCONFIG_QCA)

	if(ctrl)
		rtkswitch_LanPort_linkUp();
	else
		rtkswitch_LanPort_linkDown();
	return 1;
#elif defined(RTCONFIG_REALTEK)
	char word[100], *next;
	int mask = 0;
	char cmd[64];
	foreach(word, nvram_safe_get("lan_ifnames"), next) {
		if (!strcmp(word, "eth0")) // Port 0
			mask += (1 << 0);
		else if (!strcmp(word, "eth1")) // Port 4
			mask += (1 << 4);
		else if (!strcmp(word, "eth2")) // Port 1
			mask += (1 << 1);
		else if (!strcmp(word, "eth3")) // Port 2
			mask += (1 << 2);
		else if (!strcmp(word, "eth4")) // Port 3
			mask += (1 << 3);
	}

	if (ctrl)
		sprintf(cmd, "echo 0x%x 1 > /proc/phyPower", mask);
	else
		sprintf(cmd, "echo 0x%x 0 > /proc/phyPower", mask);
	doSystem(cmd);
	return 1;
#elif defined(RTCONFIG_ALPINE)
	if(ctrl)
		rtkswitch_LanPort_linkUp();
	else
		rtkswitch_LanPort_linkDown();
	return 1;

#elif defined(RTCONFIG_LANTIQ)
	if(ctrl){
		fprintf(stderr, "start_lan_port: power on the LAN ports...\n");
		system("/usr/bin/switch_cli GSW_MDIO_DATA_WRITE nAddressDev=2 nAddressReg=0 nData=0x1040");
		system("/usr/bin/switch_cli GSW_MDIO_DATA_WRITE nAddressDev=3 nAddressReg=0 nData=0x1040");
		system("/usr/bin/switch_cli GSW_MDIO_DATA_WRITE nAddressDev=4 nAddressReg=0 nData=0x1040");
		system("/usr/bin/switch_cli GSW_MDIO_DATA_WRITE nAddressDev=5 nAddressReg=0 nData=0x1040");
	}
	else{
		fprintf(stderr, "stop_lan_port: power off the LAN ports...\n");
		system("/usr/bin/switch_cli GSW_MDIO_DATA_WRITE nAddressDev=2 nAddressReg=0 nData=0x1c00");
		system("/usr/bin/switch_cli GSW_MDIO_DATA_WRITE nAddressDev=3 nAddressReg=0 nData=0x1c00");
		system("/usr/bin/switch_cli GSW_MDIO_DATA_WRITE nAddressDev=4 nAddressReg=0 nData=0x1c00");
		system("/usr/bin/switch_cli GSW_MDIO_DATA_WRITE nAddressDev=5 nAddressReg=0 nData=0x1c00");
	}
	return 1;
#elif defined(RTAX55) || defined(RTAX1800)
	if (ctrl)
		rtkswitch_LanPort_linkUp();
	else
		rtkswitch_LanPort_linkDown();
	return 1;
#elif defined(DSL_AX82U)
	char word[32] = {0};
	char *next = NULL;
	char cmd[64];
	foreach(word, nvram_safe_get("wired_ifnames"), next) {
		snprintf(cmd, sizeof(cmd), "ethctl %s phy-power %s"
			, word, ctrl ? "up" : "down");
		system(cmd);
	}
#else
	char word[100], *next;
	int mask = 0;

#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
	if(ctrl)
		rtkswitch_ioctl(POWERUP_LANPORTS, -1, -1);
	else
		rtkswitch_ioctl(POWERDOWN_LANPORTS, -1, -1);
#endif
#if defined(RTCONFIG_EXT_BCM53134)
	set_ex53134_ctrl(0xf, ctrl);
#endif

	foreach(word, nvram_safe_get("lanports"), next) {
		mask |= (0x0001<<atoi(word));
	}
	return set_phy_ctrl(mask, ctrl);
#endif
}
