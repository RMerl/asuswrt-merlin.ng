#include <rc.h>
#include <shared.h>
#include <shutils.h>
#ifdef RTCONFIG_RALINK
#include <ralink.h>
#if defined(RTN14U) || defined(RTAC52U) || defined(RTAC51U) || defined(RTN54U) || defined(RTAC1200HP) || defined(RTN56UB1) || defined(RTN56UB2) || defined(RTAC54U) || defined(RTAC1200GA1) || defined(RTAC1200GU) || defined(RTAC51UP) || defined(RT4GAC86U) || defined(RTAC53)
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif
#endif
#ifdef RTCONFIG_QCA
#include <qca.h>
#endif
#include "ate.h"
#ifdef RTCONFIG_INTERNAL_GOBI
#include <at_cmd.h>
#endif
#ifdef RTCONFIG_QCA_PLC_UTILS
#include <plc_utils.h>
#endif
#if defined(RTCONFIG_BCMWL6) && (defined(RTCONFIG_HAS_5G_2) || defined(RTCONFIG_HAS_6G_2) || defined(RTCONFIG_HAS_6G))
#include <wlioctl.h>
#endif

#define MULTICAST_BIT  0x0001
#define UNIQUE_OUI_BIT 0x0002

#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(GTBE96) || defined(GTBE19000) || defined(GTBE19000AI) || defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO) || defined(GT7) || defined(GTBE96_AI)
extern int cled_gpio[];
#endif

static int setAllSpecificColorLedOn(enum ate_led_color color)
{
	int i, model = get_model();
#if defined(TUFAX4200) || defined(TUFAX6000)
	enum ate_led_color switch_led_color = LED_COLOR_MAX;
#endif
#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	enum ate_led_color rtk_switch_led_color = LED_COLOR_MAX;
#endif
	enum led_id *all_led[LED_COLOR_MAX], no_led = LED_ID_MAX, *p;
	enum led_fan_mode_id v;

	if (color < 0 || color >= LED_COLOR_MAX) {
		puts("0");
		return EINVAL;
	}

#if defined(RTCONFIG_LP5523)
	{
		switch (color) {
			case LED_COLOR_WHITE: 
				lp55xx_leds_proc(LP55XX_ALL_LEDS_ON, LP55XX_ACT_NONE);
				break;
			case LED_COLOR_BLUE:
				lp55xx_leds_proc(LP55XX_ALL_BLEDS_ON, LP55XX_ACT_NONE);
				break;
			case LED_COLOR_RED:
				lp55xx_leds_proc(LP55XX_ALL_RLEDS_ON, LP55XX_ACT_NONE);
				break;
			case LED_COLOR_GREEN:
				lp55xx_leds_proc(LP55XX_ALL_GLEDS_ON, LP55XX_ACT_NONE);
				break;
			case LED_COLOR_ORANGE:
				lp55xx_leds_proc(LP55XX_ORANGE_LEDS, LP55XX_ACT_NONE);
				break;
			default: //Default Color : White color
				lp55xx_leds_proc(LP55XX_ALL_LEDS_ON, LP55XX_ACT_NONE);
				break;
		}

		puts("1");
		return 0;
	}
#endif

	for (i = 0; i < LED_COLOR_MAX; ++i)
		all_led[i] = &no_led;

	switch (model) {
#ifdef RT4GAC68U
	case MODEL_RTAC68U:
		{
			static enum led_id blue_led[] = {
				LED_POWER, LED_USB3,
				LED_LAN, LED_WPS,
#ifdef RTCONFIG_INTERNAL_GOBI
				LED_LTE, LED_SIG1, LED_SIG2, LED_SIG3,
#endif
				LED_ID_MAX
			};
			all_led[LED_COLOR_BLUE] = blue_led;
		}

		eval("et", "-i", "eth0", "robowr", "0", "0x18", "0x01ff"); // WAN
		eval("et", "-i", "eth0", "robowr", "0", "0x1a", "0x01e0");

		eval("wl", "ledbh", "10", "1"); // wl 2.4G
		eval("wl", "-i", "eth2", "ledbh", "10", "1"); // wl 5G

		break;
#endif
#if defined(BRTAC828)
	case MODEL_BRTAC828:
		{
			static enum led_id white_led[] = {
				LED_POWER, LED_WAN, LED_WAN2,
				LED_USB, LED_USB3, LED_2G, LED_5G,
				LED_FAILOVER, LED_SATA,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_POWER_RED, LED_WAN_RED,
				LED_WAN2_RED,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
			rtk_switch_led_color = LED_COLOR_WHITE;
		}
		break;
#endif
#if defined(RT4GAC53U)
	case MODEL_RT4GAC53U:
		{
			static enum led_id blue_led[] = {
				LED_POWER, LED_2G, LED_5G, LED_LAN, LED_USB,
				LED_SIG1, LED_SIG2, LED_SIG3, LED_SIG4,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_LTE_OFF, LED_POWER_RED,
				LED_ID_MAX
			};
			all_led[LED_COLOR_BLUE] = blue_led;
			all_led[LED_COLOR_RED] = red_led;
		}
		break;
#endif
#if defined(RT4GAC56)
	case MODEL_RT4GAC56:
		{
			static enum led_id white_led[] = {
				LED_POWER, LED_2G, LED_5G, LED_LAN,
				LED_SIM_DET, LED_LTE, LED_SIG1, LED_SIG2, LED_SIG3, LED_SIG4,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_SIM_UNDET,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
		}
		break;
#endif
#if defined(GTAXY16000) || defined(RTAX89U)
	case MODEL_GTAXY16000:
	case MODEL_RTAX89U:
		{
			static enum led_id white_led[] = {
				LED_POWER, LED_WAN, LED_LAN, LED_2G,
				LED_5G, LED_SFPP,
				LED_ID_MAX, /* LED_R10G @ GT-AXY16000 / LED_USB @ RT-AC89U */
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN_RED,
#if defined(RTCONFIG_BOOST)
				LED_TURBO,
#endif
				LED_ID_MAX
			};

			if (is_aqr_phy_exist())
				white_led[ARRAY_SIZE(white_led) - 2] = LED_R10G;
			else
				white_led[ARRAY_SIZE(white_led) - 2] = LED_USB;

			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			if (is_aqr_phy_exist()) {
				int aqr_addr = aqr_phy_addr();

				if (aqr_addr >= 0) {
					/* AQR107 LED0/2: GREEN, LED1: ORANGE */
					write_phy_reg(aqr_addr, 0x401EC430, (color == LED_COLOR_GREEN)? 0x100 : 0);
					write_phy_reg(aqr_addr, 0x401EC432, (color == LED_COLOR_GREEN)? 0x100 : 0);
					write_phy_reg(aqr_addr, 0x401EC431, (color == LED_COLOR_ORANGE)? 0x100 : 0);
				}
			}
		}
		break;
#endif
#if defined(TUFAX4200) || defined(TUFAX6000)
	case MODEL_TUFAX4200:
	case MODEL_TUFAX6000:
		{
			static enum led_id blue_led[] = {
#if defined(RTCONFIG_PWMX2_GPIOX1_RGBLED)
				LED_BLUE,
#endif
				LED_ID_MAX
			};
			static enum led_id green_led[] = {
#if defined(RTCONFIG_PWMX2_GPIOX1_RGBLED)
				LED_GREEN,
#endif
				LED_ID_MAX
			};
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN, 	/* GPY211 */
				LED_LAN, 	/* LAN1~4: MT7531, configed together; LAN5: GPY211 */
				LED_2G, LED_5G,	/* MT7986 WiFi LED */
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN_RED,
#if defined(RTCONFIG_PWMX2_GPIOX1_RGBLED)
				LED_RED,
#endif
				LED_ID_MAX
			};

			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
			all_led[LED_COLOR_BLUE] = blue_led;
			all_led[LED_COLOR_GREEN] = green_led;
			switch_led_color = LED_COLOR_WHITE;
		}
		break;
#endif	/* TUFAX4200, TUFAX6000 */
#if defined(RTAC82U)
	case MODEL_RTAC82U:
		{
			static enum led_id blue_led[] = {
				LED_POWER, LED_WAN, 
				LED_2G, LED_5G,
				LED_LAN1,LED_LAN2,
				LED_LAN3,LED_LAN4,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN_RED,
				LED_ID_MAX
			};
			all_led[LED_COLOR_BLUE] = blue_led;
			all_led[LED_COLOR_RED] = red_led;
		}
		break;
#endif
#if defined(RTAD7200)
	case MODEL_RTAD7200:
		{
			static enum led_id white_led[] = {
				LED_POWER, LED_WAN, LED_WAN2,
				LED_USB, LED_USB3, LED_2G, LED_5G,
				LED_FAILOVER,
#if defined(RTCONFIG_SATA_LED)
				LED_SATA,
#else
				LED_60G,
#endif
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_POWER_RED, LED_WAN_RED,
				LED_WAN2_RED,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
			rtk_switch_led_color = LED_COLOR_WHITE;
		}
		break;
#endif
#if defined(RTAX88U)
	case MODEL_RTAX88U:
		{
			static enum led_id white_led[] = {
				LED_POWER, LED_WAN_NORMAL,
				LED_LAN, LED_WPS, LED_USB, LED_USB3,
                                LED_ID_MAX
                        };
			static enum led_id red_led[] = {
				LED_WAN,
                                LED_ID_MAX
                        };
                        all_led[LED_COLOR_WHITE] = white_led;
                        all_led[LED_COLOR_RED] = red_led;

			if(color == LED_COLOR_WHITE) {
				eval("wl", "ledbh", "15", "1"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "15", "1"); // wl 5G
				eval("sw", "0x800c00b8", "0x4008f"); // LAN
			}
			else {
				eval("wl", "ledbh", "15", "0"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "15", "0"); // wl 5G
				eval("sw", "0x800c00b8", "0x40000"); //LAN
			}
		}
		break;
#endif
#if 0
#if defined(BC109)
	case MODEL_BC109:
		{
			static enum led_id white_led[] = {
				LED_POWER, LED_WAN_NORMAL, LED_LAN, 
                                LED_ID_MAX
                        };
			static enum led_id red_led[] = {
				LED_WAN,
                                LED_ID_MAX
                        };
                        all_led[LED_COLOR_WHITE] = white_led;
                        all_led[LED_COLOR_RED] = red_led;

			if(color == LED_COLOR_WHITE) {
				//eval("wl", "ledbh", "15", "1"); // wl 2.4G
				//eval("wl", "-i", "eth7", "ledbh", "15", "1"); // wl 5G
				eval("sw", "0x800c00b8", "0x4008f"); // LAN
			}
			else {
				//eval("wl", "ledbh", "15", "0"); // wl 2.4G
				//eval("wl", "-i", "eth7", "ledbh", "15", "0"); // wl 5G
				eval("sw", "0x800c00b8", "0x40000"); //LAN
			}
		}
		break;
#endif
#endif
#if defined(GTAX11000)
	case MODEL_GTAX11000:
		{
			static enum led_id white_led[] = {
				LED_POWER, LED_WAN_NORMAL,
				LED_LAN, LED_WPS,
                                LED_ID_MAX
                        };
			static enum led_id red_led[] = {
				LED_WAN,
#if defined(RTCONFIG_LOGO_LED)
				LED_LOGO,
#endif
                                LED_ID_MAX
                        };
                        all_led[LED_COLOR_WHITE] = white_led;
                        all_led[LED_COLOR_RED] = red_led;
#ifdef RTCONFIG_EXTPHY_BCM84880
			int ext_phy_model = nvram_get_int("ext_phy_model"); // 0: BCM54991, 2: GPY211
#endif
			if(color == LED_COLOR_WHITE) {
				eval("wl", "ledbh", "15", "1"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "15", "1"); // wl 5G
				eval("wl", "-i", "eth8", "ledbh", "15", "1"); // wl 5G-2
#ifdef RTCONFIG_EXTPHY_BCM84880
				if(ext_phy_model == EXT_PHY_GPY211){
					eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0x3f0");
				}
				else {
					eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x7fff0", "0x0011");	// 2.5G LED (1000M/100M)
					eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a832", "0x21");        // 2.5G LED (2500M)
					eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a83b", "0xa490");
				}
#endif
			}
			else {
				eval("wl", "ledbh", "15", "0"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "15", "0"); // wl 5G
				eval("wl", "-i", "eth8", "ledbh", "15", "0"); // wl 5G-2
#ifdef RTCONFIG_EXTPHY_BCM84880
				if(ext_phy_model == EXT_PHY_GPY211){
					eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0x0");
				}
				eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x7fff0", "0x0009");
#endif
			}
		}
		break;
#endif

#if defined(GTAXE11000)
	case MODEL_GTAXE11000:
		{
			static enum led_id white_led[] = {
				LED_POWER, LED_WAN_NORMAL,
				LED_LAN, LED_WPS,
                                LED_ID_MAX
                        };
			static enum led_id red_led[] = {
				LED_WAN,
#if defined(RTCONFIG_LOGO_LED)
				LED_LOGO,
#endif
                                LED_ID_MAX
                        };
                        all_led[LED_COLOR_WHITE] = white_led;
                        all_led[LED_COLOR_RED] = red_led;

			if(color == LED_COLOR_WHITE) {
				eval("wl", "ledbh", "9", "1"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "9", "1"); // wl 5G
				eval("wl", "-i", "eth8", "ledbh", "9", "1"); // wl 5G-2
#ifdef RTCONFIG_EXTPHY_BCM84880
				eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a832", "0x0");        // CTL LED3 MASK LOW
				eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a835", "0xffff");     // CTL LED4 MASK LOW
#endif
			}
			else {
				eval("wl", "ledbh", "9", "0"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "9", "0"); // wl 5G
				eval("wl", "-i", "eth8", "ledbh", "9", "0"); // wl 5G-2
#ifdef RTCONFIG_EXTPHY_BCM84880
				eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a835", "0x0");
#endif
			}
		}
		break;
#endif

#if defined(GT10)
	case MODEL_GT10:
		{
			if (color == LED_COLOR_RED) {
				setAllRedLedOn();
			} else if (color == LED_COLOR_GREEN) {
				setAllGreenLedOn();
			} else if (color == LED_COLOR_BLUE) {
				setAllBlueLedOn();
			} else
				bcm_cled_ctrl(BCM_CLED_OFF, BCM_CLED_STEADY_NOBLINK);

			static enum led_id white_led[] = {
				LED_WHITE,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_GROUP1_RED,
				LED_GROUP2_RED,
				LED_GROUP3_RED,
				LED_ID_MAX
			};
			static enum led_id green_led[] = {
				LED_GROUP1_GREEN,
				LED_GROUP2_GREEN,
				LED_GROUP3_GREEN,
				LED_ID_MAX
			};
			static enum led_id blue_led[] = {
				LED_GROUP1_BLUE,
				LED_GROUP2_BLUE,
				LED_GROUP3_BLUE,
				LED_ID_MAX
			};

			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
			all_led[LED_COLOR_GREEN] = green_led;
			all_led[LED_COLOR_BLUE] = blue_led;
		}
		break;
#endif

#ifdef RTAX9000
	case MODEL_RTAX9000:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WPS,
				LED_WAN_NORMAL,
				LED_LAN,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			if (color == LED_COLOR_WHITE)
			{
				eval("wl", "-i", "eth2", "ledbh", "0", "1");	// wl 2.4G
				eval("wl", "-i", "eth3", "ledbh", "0", "1");	// wl 5G
			}
			else
			{
				eval("wl", "-i", "eth2", "ledbh", "0", "21");	// wl 2.4G
				eval("wl", "-i", "eth3", "ledbh", "0", "21");	// wl 5G
			}
		}

#endif

#if defined(GTAX6000)
	case MODEL_GTAX6000:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_LAN, LED_WPS,
				LED_WAN_NORMAL,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_GROUP1_RED,
				LED_GROUP2_RED,
				LED_GROUP3_RED,
				LED_ID_MAX
			};
			static enum led_id green_led[] = {
				LED_GROUP1_GREEN,
				LED_GROUP2_GREEN,
				LED_GROUP3_GREEN,
				LED_ID_MAX
			};
			static enum led_id blue_led[] = {
				LED_GROUP1_BLUE,
				LED_GROUP2_BLUE,
				LED_GROUP3_BLUE,
				LED_ID_MAX
			};

			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
			all_led[LED_COLOR_GREEN] = green_led;
			all_led[LED_COLOR_BLUE] = blue_led;
			LEDGroupReset(LED_ON);
			wan_phy_led_pinmux(1);

			if (color == LED_COLOR_WHITE) {
				eval("wl", "-i", "eth6", "ledbh", "13", "1"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "13", "1"); // wl 5G
#ifdef RTCONFIG_EXTPHY_BCM84880
				if (nvram_get_int("ext_phy_model") == EXT_PHY_GPY211)
					eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0xf0");
				else
				{
					eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a832", "0x0");	// CTL LED3 MASK LOW
					eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a835", "0xffff");	// CTL LED4 MASK LOW
				}
#endif
			} else {
				eval("wl", "-i", "eth6", "ledbh", "13", "0"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "13", "0"); // wl 5G
#ifdef RTCONFIG_EXTPHY_BCM84880
				if (nvram_get_int("ext_phy_model") == EXT_PHY_GPY211)
					eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0x0");
				else
					eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a835", "0x0");
#endif
			}

			AntennaGroupReset(LED_OFF);
		}
		break;
#endif

#if defined(GTAX11000_PRO)
	case MODEL_GTAX11000_PRO:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_LAN,
				LED_10G_WHITE,
				LED_WAN_NORMAL,
				LED_WPS,
                                LED_ID_MAX
                        };
			static enum led_id red_led[] = {
				LED_WAN,
				LED_10G_RGB_RED,
				LED_ID_MAX
			};
			static enum led_id green_led[] = {
				LED_WAN_RGB_GREEN,
				LED_10G_RGB_GREEN,
				LED_ID_MAX
			};
			static enum led_id blue_led[] = {
				LED_WAN_RGB_BLUE,
				LED_10G_RGB_BLUE,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
			all_led[LED_COLOR_GREEN] = green_led;
			all_led[LED_COLOR_BLUE] = blue_led;

			if(color == LED_COLOR_WHITE) {
				LEDGroupReset(LED_OFF);
				eval("wl", "-i", "eth6", "ledbh", "13", "1"); // wl 5GL
				eval("wl", "-i", "eth7", "ledbh", "13", "1"); // wl 5GH
				eval("wl", "-i", "eth8", "ledbh", "13", "1"); // wl 6G
			}
			else {
				LEDGroupColor(color);
				eval("wl", "-i", "eth6", "ledbh", "13", "0"); // wl 5GL
				eval("wl", "-i", "eth7", "ledbh", "13", "0"); // wl 5GH
				eval("wl", "-i", "eth8", "ledbh", "13", "0"); // wl 6G
			}
		}
		break;
#endif

#if defined(GTAXE16000)
	case MODEL_GTAXE16000:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_LAN,
				LED_10G_WHITE,
				LED_WAN_NORMAL,
                                LED_ID_MAX
                        };
			static enum led_id red_led[] = {
				LED_WAN,
				LED_10G_RGB_RED,
				LED_ID_MAX
			};
			static enum led_id green_led[] = {
				LED_WAN_RGB_GREEN,
				LED_10G_RGB_GREEN,
				LED_ID_MAX
			};
			static enum led_id blue_led[] = {
				LED_WAN_RGB_BLUE,
				LED_10G_RGB_BLUE,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
			all_led[LED_COLOR_GREEN] = green_led;
			all_led[LED_COLOR_BLUE] = blue_led;

			if(color == LED_COLOR_WHITE) {
				LEDGroupReset(LED_OFF);
				eval("wl", "-i", "eth7", "ledbh", "13", "1"); // wl 5GL
				eval("wl", "-i", "eth8", "ledbh", "13", "1"); // wl 5GH
				eval("wl", "-i", "eth9", "ledbh", "13", "1"); // wl 6G
				eval("wl", "-i", "eth10", "ledbh", "13", "1"); // wl 2.4G
			}
			else {
				LEDGroupColor(color);
				eval("wl", "-i", "eth7", "ledbh", "13", "0"); // wl 5GL
				eval("wl", "-i", "eth8", "ledbh", "13", "0"); // wl 5GH
				eval("wl", "-i", "eth9", "ledbh", "13", "0"); // wl 6G
				eval("wl", "-i", "eth10", "ledbh", "13", "0"); // wl 2.4G
			}
		}
		break;
#endif

#if defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE19000) || defined(GTBE19000AI) || defined(GTBE96_AI)
	case MODEL_GTBE98:
	case MODEL_GTBE98_PRO:
	case MODEL_GTBE19000:
	case MODEL_GTBE19000AI:
	case MODEL_GTBE96_AI:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_LAN,
				LED_10G_WHITE,
				LED_WAN_NORMAL,
				LED_WPS,
                                LED_ID_MAX
                        };
			static enum led_id red_led[] = {
				LED_WAN,
				LED_10G_RGB_RED,
				LED_ID_MAX
			};
			static enum led_id green_led[] = {
				LED_WAN_RGB_GREEN,
				LED_10G_RGB_GREEN,
				LED_ID_MAX
			};
			static enum led_id blue_led[] = {
				LED_WAN_RGB_BLUE,
				LED_10G_RGB_BLUE,
				LED_AFC,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
			all_led[LED_COLOR_GREEN] = green_led;
			all_led[LED_COLOR_BLUE] = blue_led;

			if(color == LED_COLOR_WHITE) {
				LEDGroupReset(LED_OFF);
#if defined(GTBE19000AI) || defined(GTBE96_AI)
				bcm_cled_ctrl_single_led(BCM_CLED_OFF, BCM_CLED_STEADY_NOBLINK, 128);
#endif
				eval("wl", "-i", nvram_safe_get("wl0_ifname"), "ledbh", "12", "7"); // wl 5GL
				eval("wl", "-i", nvram_safe_get("wl1_ifname"), "ledbh", "12", "7"); // wl 5GH
				eval("wl", "-i", nvram_safe_get("wl2_ifname"), "ledbh", "12", "7"); // wl 6G
#if !defined(GTBE19000) && !defined(GTBE19000AI) && !defined(GTBE96_AI)
				eval("wl", "-i", nvram_safe_get("wl3_ifname"), "ledbh", "12", "7"); // wl 2.4G
#endif
			}
			else {
				LEDGroupColor(color);
#if defined(GTBE19000AI) || defined(GTBE96_AI)
				if(color == LED_COLOR_RED)
					bcm_cled_ctrl_single_led(BCM_CLED_RED, BCM_CLED_STEADY_NOBLINK, 128);
				else if(color == LED_COLOR_GREEN)
					bcm_cled_ctrl_single_led(BCM_CLED_GREEN, BCM_CLED_STEADY_NOBLINK, 128);
				else if(color == LED_COLOR_BLUE)
					bcm_cled_ctrl_single_led(BCM_CLED_BLUE, BCM_CLED_STEADY_NOBLINK, 128);
#endif
				eval("wl", "-i", nvram_safe_get("wl0_ifname"), "ledbh", "12", "0"); // wl 5GL
				eval("wl", "-i", nvram_safe_get("wl1_ifname"), "ledbh", "12", "0"); // wl 5GH
				eval("wl", "-i", nvram_safe_get("wl2_ifname"), "ledbh", "12", "0"); // wl 6G
#if !defined(GTBE19000) && !defined(GTBE19000AI) && !defined(GTBE96_AI)
				eval("wl", "-i", nvram_safe_get("wl3_ifname"), "ledbh", "12", "0"); // wl 2.4G
#endif
			}

			if(color == LED_COLOR_BLUE)
				f_write_string("/sys/class/leds/led_gpio_15/brightness", "255", 0, 0);
			else
				f_write_string("/sys/class/leds/led_gpio_15/brightness", "0", 0, 0);

#ifdef RTCONFIG_CC3220
			ti_set_led_on(color);
#endif
		}
		break;
#endif

#if defined(GTBE96)
	case MODEL_GTBE96:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_LAN,
				LED_10G_WHITE,
				LED_WAN_NORMAL,
				LED_WPS,
                                LED_ID_MAX
                        };
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};

			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			if (pidof("sw_devled") > 0)
				killall_tk("sw_devled");

			if(color == LED_COLOR_WHITE) {
				LEDGroupReset(LED_OFF);
				eval("wl", "-i", nvram_safe_get("wl0_ifname"), "ledbh", "12", "7"); // wl 2.4G
				eval("wl", "-i", nvram_safe_get("wl1_ifname"), "ledbh", "12", "7"); // wl 5GL
				eval("wl", "-i", nvram_safe_get("wl2_ifname"), "ledbh", "12", "7"); // wl 5GH
			}
			else {
				LEDGroupColor(color);
				eval("wl", "-i", nvram_safe_get("wl0_ifname"), "ledbh", "12", "0"); // wl 2.4G
				eval("wl", "-i", nvram_safe_get("wl1_ifname"), "ledbh", "12", "0"); // wl 5GL
				eval("wl", "-i", nvram_safe_get("wl2_ifname"), "ledbh", "12", "0"); // wl 5GH
			}
		}
		break;
#endif

#if defined(RTAX88U_PRO)
	case MODEL_RTAX88U_PRO:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN_NORMAL,
				LED_LAN,
#ifdef RTCONFIG_EXTPHY_BCM84880
				LED_EXTPHY,
#endif
				LED_USB,
				LED_WPS,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};

			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			wan_phy_led_pinmux(1);
			lan_phy_led_pinmux(1);

			if(color == LED_COLOR_WHITE) {
				eval("wl", "-i", "eth6", "ledbh", "13", "1"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "13", "1"); // wl 5G
			}
			else {
				eval("wl", "-i", "eth6", "ledbh", "13", "0"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "13", "0"); // wl 5G
			}
		}
		break;
#endif

#if defined(RTBE96U)
	case MODEL_RTBE96U:
	{

		static enum led_id white_led[] = {
			LED_POWER,
			LED_WAN_NORMAL,
			LED_LAN,
			LED_10G_WHITE,
			LED_WPS,
			LED_ID_MAX
		};
		static enum led_id red_led[] = {
			LED_WAN,
			LED_ID_MAX
		};
#ifdef RTCONFIG_PRESSURE_SENSOR
		static enum led_id blue_led[] = {
			LED_AFC,
			LED_ID_MAX
		};
#endif

		all_led[LED_COLOR_WHITE] = white_led;
		all_led[LED_COLOR_RED] = red_led;

		if(color == LED_COLOR_WHITE) {
			eval("wl", "-i", "wl0", "ledbh", "12", "1"); // wl 2.4G
			eval("wl", "-i", "wl1", "ledbh", "12", "1"); // wl 5G
			eval("wl", "-i", "wl2", "ledbh", "12", "1"); // wl 6G
		}
		else {
			eval("wl", "-i", "wl0", "ledbh", "12", "0"); // wl 2.4G
			eval("wl", "-i", "wl1", "ledbh", "12", "0"); // wl 5G
			eval("wl", "-i", "wl2", "ledbh", "12", "0"); // wl 6G
		}

	}
	break;
#endif

#if defined(RTBE88U)
	case MODEL_RTBE88U:
	{

		static enum led_id white_led[] = {
			LED_POWER,
			LED_WAN_NORMAL,
			LED_LAN,
			LED_10G_WHITE,
			LED_USB,
			LED_SFPP,
			LED_ID_MAX
		};
		static enum led_id red_led[] = {
			LED_WAN,
			LED_ID_MAX
		};

		all_led[LED_COLOR_WHITE] = white_led;
		all_led[LED_COLOR_RED] = red_led;

		if(color == LED_COLOR_WHITE) {
			eval("wl", "-i", "wl0", "ledbh", "12", "1"); // wl 2.4G
			eval("wl", "-i", "wl1", "ledbh", "12", "1"); // wl 5G
		}
		else {
			eval("wl", "-i", "wl0", "ledbh", "12", "0"); // wl 2.4G
			eval("wl", "-i", "wl1", "ledbh", "12", "0"); // wl 5G
		}

	}
	break;
#endif

#if defined(RTBE86U)
	case MODEL_RTBE86U:
	{

		static enum led_id white_led[] = {
			LED_POWER,
			LED_WAN_NORMAL,
			LED_10G_WHITE,
#ifdef RTCONFIG_LAN4WAN_LED
			LED_LAN1, LED_LAN2, LED_LAN3, LED_LAN4,
#endif
			LED_WPS,
			LED_USB,
			LED_ID_MAX
		};
		static enum led_id red_led[] = {
			LED_WAN,
			LED_ID_MAX
		};

		all_led[LED_COLOR_WHITE] = white_led;
		all_led[LED_COLOR_RED] = red_led;

		lan_phy_led_pinmux(1);
		wan_phy_led_pinmux(1);

		if (color == LED_COLOR_WHITE) {
			eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "7", "1");	// wl 2.4G
			eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "12", "1");// wl 5G
		} else {
			eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "7", "0");	// wl 2.4G
			eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "12", "0");// wl 5G
		}
	}
	break;
#endif

#if defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE58U_V2) || defined(TUFBE3600_V2) || defined(RTBE55)
	case MODEL_RTBE58U:
	case MODEL_RTBE58U_V2:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN_NORMAL,
				LED_LAN,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			wan_phy_led_pinmux(1);
			if (color == LED_COLOR_WHITE)
			{
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "0", "1");	// wl 2.4G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "0", "1");	// wl 5G
			}
			else
			{
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "0", "21");// wl 2.4G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "0", "21");// wl 5G
			}
		}
		break;
#endif

#if defined(RTBE92U)
	case MODEL_RTBE92U:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN_NORMAL,
				LED_LAN,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			wan_phy_led_pinmux(1);
			if (color == LED_COLOR_WHITE)
			{
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "7", "1");	// wl 2.4G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "0", "1");	// wl 5G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 2, 0)), "ledbh", "0", "1");	// wl 6G
			}
			else
			{
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "7", "0");	// wl 2.4G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "0", "21");// wl 5G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 2, 0)), "ledbh", "0", "21");// wl 6G
			}
		}
		break;
#endif

#if defined(RTBE95U)
	case MODEL_RTBE95U:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN_NORMAL,
				LED_LAN,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			wan_phy_led_pinmux(1);
			if (color == LED_COLOR_WHITE)
			{
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "12", "1");// wl 5G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "0", "1");	// wl 6G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 2, 0)), "ledbh", "0", "1");	// wl 2.4G
			}
			else
			{
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "12", "0");// wl 5G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "0", "21");// wl 6G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 2, 0)), "ledbh", "0", "21");// wl 2.4G
			}
		}
		break;
#endif

#if defined(RTBE82U) || defined(TUFBE82) || defined(RTBE82M)
	case MODEL_RTBE82U:
	case MODEL_RTBE82M:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN_NORMAL,
				LED_LAN,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			if (color == LED_COLOR_WHITE)
			{
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "0", "1");	// wl 2.4G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "0", "1");	// wl 5G
			}
			else
			{
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "0", "21");// wl 2.4G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "0", "21");// wl 5G
			}
		}
		break;
#endif

#if defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO)
	case MODEL_GSBE18000:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN_NORMAL,
				LED_WIFI,
				LED_LAN,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_GROUP1_RED,
				LED_GROUP2_RED,
				LED_GROUP3_RED,
				LED_ID_MAX
			};
			static enum led_id green_led[] = {
				LED_GROUP1_GREEN,
				LED_GROUP2_GREEN,
				LED_GROUP3_GREEN,
				LED_ID_MAX
			};
			static enum led_id blue_led[] = {
				LED_GROUP1_BLUE,
				LED_GROUP2_BLUE,
				LED_GROUP3_BLUE,
				LED_ID_MAX
			};

			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
			all_led[LED_COLOR_GREEN] = green_led;
			all_led[LED_COLOR_BLUE] = blue_led;
			LEDGroupReset(LED_ON);
		}
		break;
#endif

#if defined(GT7)
	case MODEL_GT7:
		{
			if (color == LED_COLOR_RED) {
				setAllRedLedOn();
			} else if (color == LED_COLOR_GREEN) {
				setAllGreenLedOn();
			} else if (color == LED_COLOR_BLUE) {
				setAllBlueLedOn();
			} else
				bcm_cled_ctrl(BCM_CLED_OFF, BCM_CLED_STEADY_NOBLINK);

			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN_NORMAL,
				LED_WIFI,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_GROUP1_RED,
				LED_GROUP2_RED,
				LED_GROUP3_RED,
				LED_ID_MAX
			};
			static enum led_id green_led[] = {
				LED_GROUP1_GREEN,
				LED_GROUP2_GREEN,
				LED_GROUP3_GREEN,
				LED_ID_MAX
			};
			static enum led_id blue_led[] = {
				LED_GROUP1_BLUE,
				LED_GROUP2_BLUE,
				LED_GROUP3_BLUE,
				LED_ID_MAX
			};

			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
			all_led[LED_COLOR_GREEN] = green_led;
			all_led[LED_COLOR_BLUE] = blue_led;
			LEDGroupReset(LED_ON);
		}
		break;
#endif

#if defined(RTBE58U_PRO)
	case MODEL_RTBE58U_PRO:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN_NORMAL,
				LED_LAN,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			wan_phy_led_pinmux(1);
			if (color == LED_COLOR_WHITE)
			{
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "0", "1");	// wl 2.4G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "0", "1");	// wl 5G
			}
			else
			{
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 0, 0)), "ledbh", "0", "21");// wl 2.4G
				eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", 1, 0)), "ledbh", "0", "21");// wl 5G
			}
		}
		break;
#endif

#if defined(RTAX86U_PRO)
	case MODEL_RTAX86U_PRO:
		{
			static enum led_id white_led[] = {
				LED_POWER,
#if 0
				LED_WAN_NORMAL,
#ifdef RTCONFIG_LAN4WAN_LED
				LED_LAN1, LED_LAN2, LED_LAN3, LED_LAN4,
#else
				LED_LAN,
#endif
#endif
				LED_WPS,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
#ifdef RTCONFIG_EXTPHY_BCM84880
				LED_EXTPHY,
#endif
				LED_ID_MAX
			};

			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			wan_phy_led_pinmux(1);

			if(color == LED_COLOR_WHITE) {
				eval("sw", "0xff800520", "0xfdc1fc4a"); // orig value: 0xfdcbfdda or 0xfdcbfddb, let GPIO 4/5/7/8/17/19 be LOW
				eval("wl", "-i", "eth6", "ledbh", "7", "1"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "13", "1"); // wl 5G
			}
			else {
				eval("sw", "0xff800520", "0xfdcbfdfe"); // orig value: 0xfdcbfdda or 0xfdcbfddb, let GPIO 4/5/7/8/17/19 be HIGH. let GPIO2 be HIGH to avoid POWER LED is on abnormally
				eval("wl", "-i", "eth6", "ledbh", "7", "0"); // wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "13", "0"); // wl 5G
			}
		}
		break;
#endif

#if defined(RTAX92U)
	case MODEL_RTAX92U:
		{
			static enum led_id white_led[] = {
				LED_POWER, LED_WAN_NORMAL,
				LED_LAN,
                                LED_ID_MAX
                        };
			static enum led_id red_led[] = {
				LED_WAN,
                                LED_ID_MAX
                        };
                        all_led[LED_COLOR_WHITE] = white_led;
                        all_led[LED_COLOR_RED] = red_led;

			if(color == LED_COLOR_WHITE) {
	                        eval("wl", "-i", "eth5", "ledbh", "10", "1");    // wl 2.4G
		                eval("wl", "-i", "eth6", "ledbh", "10", "1");    // wl 5G low
			        eval("wl", "-i", "eth7", "ledbh", "15", "1");    // wl 5G high
			}
			else {
	                        eval("wl", "-i", "eth5", "ledbh", "10", "0");    // wl 2.4G
		                eval("wl", "-i", "eth6", "ledbh", "10", "0");    // wl 5G low
			        eval("wl", "-i", "eth7", "ledbh", "15", "0");    // wl 5G high
			}
		}
		break;
#endif
#if defined(ET12) || defined(XT12)
	case MODEL_ET12:
	case MODEL_XT12:
		{
			if(color == LED_COLOR_RED) {
				setAllRedLedOn();
			}else if (color == LED_COLOR_GREEN) {
				setAllGreenLedOn();
			}else if (color == LED_COLOR_BLUE) {
				setAllBlueLedOn();
			}else if(color == LED_COLOR_WHITE) {
				setAllWhiteLedOn();
			}
		}
		return;
#endif
#if defined(RTAX95Q) || defined(XT8PRO) || defined(BT12) || defined(BT10) || defined(BQ16) || defined(BQ16_PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(CTAX56_XD4) || defined(XC5) || defined(EBA63)
	case MODEL_RTAX95Q:
	case MODEL_XT8PRO:
	case MODEL_BT12:
	case MODEL_BT10:
	case MODEL_BQ16:
	case MODEL_BQ16_PRO:
	case MODEL_BM68:
	case MODEL_XT8_V2:
	case MODEL_RTAXE95Q:
	case MODEL_ET8PRO:
	case MODEL_ET8_V2:
	case MODEL_RTAX56_XD4:
	case MODEL_XD4PRO:
	case MODEL_XC5:
	case MODEL_CTAX56_XD4:
	case MODEL_EBA63:
		{
			if(color == LED_COLOR_RED) {
				setAllRedLedOn();
			}else if (color == LED_COLOR_GREEN) {
				setAllGreenLedOn();
			}else if (color == LED_COLOR_BLUE) {
				setAllBlueLedOn();
			}
#if defined(EBA63)
			else if (color == LED_COLOR_WHITE) {
				bcm_cled_ctrl(BCM_CLED_WHITE, BCM_CLED_STEADY_NOBLINK);
			}
#endif
		}
		break;
#endif

#ifdef BCM6750
	case MODEL_RTAX58U:
	case MODEL_RTAX82U_V2:
	case MODEL_TUFAX5400_V2:
	case MODEL_RTAX5400:
	case MODEL_XD6_V2:
		{
#if defined(RTAX82_XD6) || defined(XD6_V2)
			if (color == LED_COLOR_RED) {
				setAllRedLedOn();
			} else if (color == LED_COLOR_GREEN) {
				setAllGreenLedOn();
			} else if (color == LED_COLOR_BLUE) {
				setAllBlueLedOn();
			} else
				bcm_cled_ctrl(BCM_CLED_OFF, BCM_CLED_STEADY_NOBLINK);
#else
			static enum led_id white_led[] = {
				LED_POWER,
#ifdef RTCONFIG_LAN4WAN_LED
				LED_LAN1, LED_LAN2, LED_LAN3, LED_LAN4,
#endif
				LED_WAN_NORMAL,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
#if defined(RTAX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(RTAX82U_V2) || defined(TUFAX5400_V2)
				LED_GROUP1_RED,
#if !defined(TUFAX5400) && !defined(TUFAX5400_V2)
				LED_GROUP2_RED,
				LED_GROUP3_RED,
				LED_GROUP4_RED,
#if defined(GSAX3000) || defined(GSAX5400)
				LED_GROUP5_RED,
#endif
#endif
#endif
				LED_ID_MAX
			};
#if defined(RTAX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(RTAX82U_V2) || defined(TUFAX5400_V2)
			static enum led_id green_led[] = {
				LED_GROUP1_GREEN,
#if !defined(TUFAX5400) && !defined(TUFAX5400_V2)
				LED_GROUP2_GREEN,
				LED_GROUP3_GREEN,
				LED_GROUP4_GREEN,
#if defined(GSAX3000) || defined(GSAX5400)
				LED_GROUP5_GREEN,
#endif
#endif
				LED_ID_MAX
			};
			static enum led_id blue_led[] = {
				LED_GROUP1_BLUE,
#if !defined(TUFAX5400) && !defined(TUFAX5400_V2)
				LED_GROUP2_BLUE,
				LED_GROUP3_BLUE,
				LED_GROUP4_BLUE,
#if defined(GSAX3000) || defined(GSAX5400)
				LED_GROUP5_BLUE,
#endif
#endif
				LED_ID_MAX
			};
#endif
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
#if defined(RTAX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(RTAX82U_V2) || defined(TUFAX5400_V2)
			all_led[LED_COLOR_GREEN] = green_led;
			all_led[LED_COLOR_BLUE] = blue_led;
			LEDGroupReset(LED_ON);
#endif
#if defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(RTAX5400)
#if defined(TUFAX5400_V2) || defined(RTAX5400)
			lan_phy_led_pinmux(1);
#endif
			wan_phy_led_pinmux(1);
#endif
#endif
			if (color == LED_COLOR_WHITE) {
#if defined(RTAX82U_V2) || defined(TUFAX5400_V2)
				LEDGroupReset(LED_OFF);
#endif
				eval("wl", "-i", "eth5", "ledbh", "0", "1");	// wl 2.4G
#if defined(RTAX82U) && !defined(RTCONFIG_BCM_MFG)
				if (!nvram_get_int("LED_order")) {
					led_control(LED_5G, LED_ON);
					eval("wl", "-i", "eth6", "ledbh", "15", "0");	// fake WAN
				} else
#endif
				eval("wl", "-i", "eth6", "ledbh", "15", "1");	// wl 5G
			}
			else {
#if defined(RTAX82U_V2) || defined(TUFAX5400_V2)
				LEDGroupColor(color);
#endif
				eval("wl", "-i", "eth5", "ledbh", "0", "21");	// wl 2.4G
#if defined(RTAX82U) && !defined(RTCONFIG_BCM_MFG)
				if (!nvram_get_int("LED_order")) {
					led_control(LED_5G, LED_OFF);
					eval("wl", "-i", "eth6", "ledbh", "15", "1");	// fake WAN
				} else
#endif
				eval("wl", "-i", "eth6", "ledbh", "15", "0");	// wl 5G
			}
		}
		break;
#endif

#ifdef RTAX82_XD6S
	case MODEL_RTAX82_XD6S:
		{
			if (color == LED_COLOR_RED) {
				setAllRedLedOn();
			} else if (color == LED_COLOR_GREEN) {
				setAllGreenLedOn();
			} else if (color == LED_COLOR_BLUE) {
				setAllBlueLedOn();
			} else
				bcm_cled_ctrl(BCM_CLED_OFF, BCM_CLED_STEADY_NOBLINK);

			if (color == LED_COLOR_WHITE) {
				eval("wl", "-i", "eth5", "ledbh", "0", "1");	// wl 2.4G
				eval("wl", "-i", "eth6", "ledbh", "15", "1");	// wl 5G
			}
			else {
				eval("wl", "-i", "eth5", "ledbh", "0", "21");	// wl 2.4G
				eval("wl", "-i", "eth6", "ledbh", "15", "0");	// wl 5G
			}
		}
		break;
#endif

#if defined(RPAX58) || defined(RPBE58)
	case MODEL_RPBE58:
		{
			eval("sw", "0xff803010", "0x00110039");

                        eval("sw", "0xff803020", "0");	// W
                        eval("sw", "0xff803050", "0");		// R
                        eval("sw", "0xff803060", "0");		// G
                        eval("sw", "0xff803070", "0");		// Y
                        eval("sw", "0xff803160", "0");		// B
                        eval("sw", "0xFF80301c", "0x00110039");

			switch(color) {
				case LED_COLOR_RED:
					eval("sw", "0xff803050", "0x3e000");
					break;		
				case LED_COLOR_GREEN:
					eval("sw", "0xff803060", "0x3e000");
					break;		
				case LED_COLOR_BLUE:
					eval("sw", "0xff803160", "0x3e000");
					break;		
				case LED_COLOR_WHITE:
					eval("sw", "0xff803020", "0x3e000");	// W
					break;		
				case LED_COLOR_ORANGE:
					eval("sw", "0xff803070", "0x3e000");
					break;		
				case LED_COLOR_PURPLE:
					eval("sw", "0xff803050", "0x3e000");
					eval("sw", "0xff803160", "0x3e000");
					break;		
			} 
			eval("sw", "0xff80301c", "0x00110039");
		}
		break;
	case MODEL_RPAX58:
		{
			eval("sw", "0xff803010", "0xd8a0");

                        eval("sw", "0xff803070", "0");
                        eval("sw", "0xff803090", "0");
                        eval("sw", "0xff8030d0", "0");
                        eval("sw", "0xff8030e0", "0");
                        eval("sw", "0xff803100", "0");
                        eval("sw", "0xff803110", "0");
                        eval("sw", "0xFF80301c", "0xd8a0");

			switch(color) {
				case LED_COLOR_RED:
					eval("sw", "0xff803070", "0x3e000");
					break;		
				case LED_COLOR_GREEN:
					eval("sw", "0xff803090", "0x3e000");
					break;		
				case LED_COLOR_BLUE:
					eval("sw", "0xff8030d0", "0x3e000");
					break;		
				case LED_COLOR_WHITE:
					eval("sw", "0xff8030e0", "0x3e000");
					break;		
				case LED_COLOR_ORANGE:
					eval("sw", "0xff803100", "0x3e000");
					break;		
				case LED_COLOR_PURPLE:
					//eval("sw", "0xff803070", "0x3e000");
					//eval("sw", "0xff8030d0", "0x3e000");
					eval("sw", "0xff803110", "0x3e000");
					break;		
			} 
			eval("sw", "0xff80301c", "0xd8a0");
		}
		break;
#endif
#if defined(RTBE58_GO)
	case MODEL_RTBE58_GO:
		{
			eval("sw", "0xff803010", "0x40c00008");

                        eval("sw", "0xff803050", "0");	// W
                        eval("sw", "0xff803180", "0");	// R
                        eval("sw", "0xff803190", "0");	// G
                        eval("sw", "0xff803200", "0");	// B
                        eval("sw", "0xFF80301c", "0x40c00008");

			switch(color) {
				case LED_COLOR_RED:
					eval("sw", "0xff803180", "0x3e000");
					break;		
				case LED_COLOR_GREEN:
					eval("sw", "0xff803190", "0x3e000");
					break;		
				case LED_COLOR_BLUE:
					eval("sw", "0xff803200", "0x3e000");
					break;		
				case LED_COLOR_WHITE:
					eval("sw", "0xff803050", "0x3e000");	// W
					break;		
			} 
			eval("sw", "0xff80301c", "0x40c00008");
		}
		break;
#endif
#ifdef RTAX58U_V2
	case MODEL_RTAX58U_V2:
		{

			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN_NORMAL,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};


			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			wan_phy_led_pinmux(1);

			if (color == LED_COLOR_WHITE)
			{
				system("rtkswitch 42");
				eval("wl", "-i", "eth2", "ledbh", "0", "1");	// wl 2.4G
				eval("wl", "-i", "eth3", "ledbh", "0", "1");	// wl 5G
			}
			else
			{
				system("rtkswitch 41");
				eval("wl", "-i", "eth2", "ledbh", "0", "21");	// wl 2.4G
				eval("wl", "-i", "eth3", "ledbh", "0", "21");	// wl 5G
			}
		}
		break;
#endif

#if defined(RTAX3000N) || defined(BR63)
	case MODEL_RTAX3000N:
	case MODEL_BR63:
		{
#ifdef BR63
			static enum led_id white_led[] = {
#else
			static enum led_id blue_led[] = {
#endif
				LED_POWER,
				LED_WAN_NORMAL,
#ifdef RTAX3000N
				LED_LAN,
#endif
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};
#ifdef BR63
			all_led[LED_COLOR_WHITE] = white_led;
#else
			all_led[LED_COLOR_BLUE] = blue_led;
#endif
			all_led[LED_COLOR_RED] = red_led;

			wan_phy_led_pinmux(1);
#ifdef BR63
			if (color == LED_COLOR_WHITE)
#else
			if (color == LED_COLOR_BLUE)
#endif
			{
				system("rtkswitch 42");
				eval("wl", "-i", "eth2", "ledbh", "0", "1");	// wl 2.4G
				eval("wl", "-i", "eth3", "ledbh", "0", "1");	// wl 5G
			}
			else
			{
				system("rtkswitch 41");
				eval("wl", "-i", "eth2", "ledbh", "0", "21");	// wl 2.4G
				eval("wl", "-i", "eth3", "ledbh", "0", "21");	// wl 5G
			}
		}
		break;
#endif

#if defined(RTAXE7800)
	case MODEL_RTAXE7800:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WPS,
				LED_LAN,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_WAN2,
				LED_ID_MAX
			};

			nvram_set_int("led_lan_gpio", 3|GPIO_ACTIVE_LOW);
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			lan_phy_led_pinmux(1);

			if (color == LED_COLOR_WHITE) {
				eval("wl", "-i", "eth5", "ledbh", "0", "1");	// wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "15", "1");	// wl 5G
				eval("wl", "-i", "eth6", "ledbh", "0", "1");	// wl 6G
				bcm53134_led_control(1);
#ifdef RTCONFIG_EXTPHY_BCM84880
				eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0xf0");
#endif
			} else {
				eval("wl", "-i", "eth5", "ledbh", "0", "0");	// wl 2.4G
				eval("wl", "-i", "eth7", "ledbh", "15", "0");	// wl 5G
				eval("wl", "-i", "eth6", "ledbh", "0", "0");	// wl 6G
				bcm53134_led_control(0);
#ifdef RTCONFIG_EXTPHY_BCM84880
				eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0x0");
#endif
			}
		}
		break;
#endif
#if defined(TUFAX3000_V2)
	case MODEL_TUFAX3000_V2:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_LAN, LED_WPS,
				LED_WAN_NORMAL,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};

			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			lan_phy_led_pinmux(1);

			if (color == LED_COLOR_WHITE) {
				eval("wl", "-i", "eth5", "ledbh", "0", "1"); // wl 2.4G
				eval("wl", "-i", "eth6", "ledbh", "0", "1"); // wl 5G
				bcm53134_led_control(1);
#ifdef RTCONFIG_EXTPHY_BCM84880
				eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0xf0");
#endif
			} else {
				eval("wl", "-i", "eth5", "ledbh", "0", "0"); // wl 2.4G
				eval("wl", "-i", "eth6", "ledbh", "0", "0"); // wl 5G
				bcm53134_led_control(0);
#ifdef RTCONFIG_EXTPHY_BCM84880
				eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0x0");
#endif
			}
		}
		break;
#endif
#if defined(RTAX55) || defined(RTAX1800)
	case MODEL_RTAX55:
		{
			static enum led_id red_led[] = {
				LED_WAN,
				LED_ID_MAX
			};

			static enum led_id blue_led[] = {
				LED_POWER,
				LED_WAN_NORMAL,
				LED_LAN,
				LED_ID_MAX
			};

			all_led[LED_COLOR_RED] = red_led;
			all_led[LED_COLOR_BLUE] = blue_led;

			if (color == LED_COLOR_BLUE)
			{
				eval("wl", "-i", "eth2", "ledbh", "0", "1");	// wl 2.4G
				eval("wl", "-i", "eth3", "ledbh", "0", "1");	// wl 5G
			}
			else
			{
				eval("wl", "-i", "eth2", "ledbh", "0", "21");	// wl 2.4G
				eval("wl", "-i", "eth3", "ledbh", "0", "21");	// wl 5G
			}
		}
		break;
#endif
#if 0
//#if defined(RPAX56)
	case MODEL_RPAX56:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN_NORMAL,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			if(color == LED_COLOR_WHITE) {
				eval("wl", "-i", "eth1", "ledbh", "0", "1");	// wl 2.4G
				eval("wl", "-i", "eth2", "ledbh", "0", "1");	// wl 5G low
			}
			else {
				eval("wl", "-i", "eth1", "ledbh", "0", "21");	// wl 2.4G
				eval("wl", "-i", "eth2", "ledbh", "0", "21");	// wl 5G low
			}
		}
#endif
#if defined(RTAX56U)
	case MODEL_RTAX56U:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_WAN,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_WAN_NORMAL,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;

			if(color == LED_COLOR_WHITE) {
				eval("wl", "-i", "eth5", "ledbh", "0", "1");	// wl 2.4G
				eval("wl", "-i", "eth6", "ledbh", "0", "1");	// wl 5G low
			}
			else {
				eval("wl", "-i", "eth5", "ledbh", "0", "21");	// wl 2.4G
				eval("wl", "-i", "eth6", "ledbh", "0", "21");	// wl 5G low
			}
		}
		break;
#endif
#if defined(DSL_AX82U)
//TODO:
	case MODEL_DSLAX82U:
		{
			static enum led_id white_led[] = {
				LED_POWER,
				LED_LAN,
#ifdef RTCONFIG_WANRED_LED
				LED_WAN,
#else
				LED_WAN_NORMAL,
#endif
				LED_WIFI,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_POWER_RED,
#ifdef RTCONFIG_WANRED_LED
				LED_WAN_RED,
#else
				LED_WAN,
#endif
				LED_GROUP1_RED,
				LED_GROUP2_RED,
				LED_GROUP3_RED,
				LED_GROUP4_RED,
				LED_ID_MAX
			};
			static enum led_id green_led[] = {
				LED_GROUP1_GREEN,
				LED_GROUP2_GREEN,
				LED_GROUP3_GREEN,
				LED_GROUP4_GREEN,
				LED_ID_MAX
			};
			static enum led_id blue_led[] = {
				LED_GROUP1_BLUE,
				LED_GROUP2_BLUE,
				LED_GROUP3_BLUE,
				LED_GROUP4_BLUE,
				LED_ID_MAX
			};

			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_RED] = red_led;
			all_led[LED_COLOR_GREEN] = green_led;
			all_led[LED_COLOR_BLUE] = blue_led;
			LEDGroupReset(LED_ON);
		}
		break;
#endif
#if defined(RTAX86U) || defined(RTAX68U) || defined(RTAC68U_V4)
	case MODEL_RTAX86U:
	case MODEL_RTAX68U:
	case MODEL_RTAC68U_V4:
		{
#ifdef RTAC68U_V4
			static enum led_id blue_led[] = {
#else
			static enum led_id white_led[] = {
#endif
					LED_POWER, LED_WAN_NORMAL,
#ifdef RTCONFIG_LAN4WAN_LED
					LED_LAN1, LED_LAN2, LED_LAN3, LED_LAN4,
#else
					LED_LAN,
#endif
#if defined(RTAX86U)
					LED_WPS,
					LED_USB, // RT-AX86S
#endif
#ifdef RTAC68U_V4
					LED_USB, LED_USB3,
#endif
					LED_ID_MAX
					};
			static enum led_id red_led[] = {
					LED_WAN,
#ifdef RTCONFIG_EXTPHY_BCM84880
					LED_EXTPHY,
#endif
					LED_ID_MAX
					};
#ifdef RTCONFIG_EXTPHY_BCM84880
			int ext_phy_model = nvram_get_int("ext_phy_model"); // 0: BCM54991, 1: RTL8226, 2: GPY211
#endif
#if defined(RTAX86U) || defined(RTAX68U)
			char productid[16], *wifi_2g, *wifi_5g;
			snprintf(productid, sizeof(productid), "%s", get_productid());
			if(!strcmp(productid, "RT-AX86S") || !strcmp(productid, "RT-AX68U")){
				wifi_2g = "eth5";
				wifi_5g = "eth6";
			} else {
				wifi_2g = "eth6";
				wifi_5g = "eth7";
			}
#endif

#ifdef RTAC68U_V4
			all_led[LED_COLOR_BLUE] = blue_led;
#else
			all_led[LED_COLOR_WHITE] = white_led;
#endif
			all_led[LED_COLOR_RED] = red_led;

#ifdef RTAC68U_V4
			if(color == LED_COLOR_BLUE) {
#else
			if(color == LED_COLOR_WHITE) {
#endif
#ifdef RTAC68U_V4
				eval("wl", "-i", "eth5", "ledbh", "10", "1"); // wl 2.4G
				eval("wl", "-i", "eth6", "ledbh", "10", "1"); // wl 5G
#else
				eval("wl", "-i", wifi_2g, "ledbh", "7", "1"); // wl 2.4G
				eval("wl", "-i", wifi_5g, "ledbh", "15", "1"); // wl 5G
#endif
#ifdef RTCONFIG_EXTPHY_BCM84880
				if(!strcmp(productid, "RT-AX86S")) ;
				else if(ext_phy_model == EXT_PHY_GPY211){
					eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0xf0");
				}
				else if(ext_phy_model == EXT_PHY_RTL8226){
					eval("ethctl", "phy", "ext", EXTPHY_RTL_ADDR_STR, "0x1fd032", "0x0027");	// RTL LCR2 LED Control Reg
				}
				else if(ext_phy_model == EXT_PHY_BCM54991){
					eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a832", "0x0");	// CTL LED3 MASK LOW
					eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a835", "0xffff");	// CTL LED4 MASK LOW
				}
#endif
			}
			else {
#ifdef RTAC68U_V4
				eval("wl", "-i", "eth5", "ledbh", "10", "0"); // wl 2.4G
				eval("wl", "-i", "eth6", "ledbh", "10", "0"); // wl 5G
#else
				eval("wl", "-i", wifi_2g, "ledbh", "7", "0"); // wl 2.4G
				eval("wl", "-i", wifi_5g, "ledbh", "15", "0"); // wl 5G
#endif
#ifdef RTCONFIG_EXTPHY_BCM84880
				if(!strcmp(productid, "RT-AX86S")) ;
				else if(ext_phy_model == EXT_PHY_GPY211){
					eval("ethctl", "phy", "ext", EXTPHY_GPY_ADDR_STR, "0x1e0001", "0x0");
				}
				else if(ext_phy_model == EXT_PHY_RTL8226){
					eval("ethctl", "phy", "ext", EXTPHY_RTL_ADDR_STR, "0x1fd032", "0x0000");
				}
				else if(ext_phy_model == EXT_PHY_BCM54991){
					eval("ethctl", "phy", "ext", EXTPHY_ADDR_STR, "0x1a835", "0x0");
				}
#endif
			}
		}
		break;
#endif
#if defined(RTCONFIG_GPIOX3_RGBLED) \
 || defined(RTCONFIG_PWMX1_GPIOX3_RGBLED) \
 || defined(RTCONFIG_PWMX3_RGBLED)
	case MODEL_MAPAC1750:
	case MODEL_RTAC59CD6R:
	case MODEL_RTAC59CD6N:
	case MODEL_PLAX56XP4:
	case MODEL_XD4S:
	case MODEL_RTAX59U:
	case MODEL_PRTAX57GO:
		{
			static enum led_id blue_led[] = {
				LED_BLUE,
				LED_ID_MAX
			};
			static enum led_id green_led[] = {
				LED_GREEN,
				LED_ID_MAX
			};
			static enum led_id red_led[] = {
				LED_RED,
				LED_ID_MAX
			};
			static enum led_id white_led[] = {
				LED_WHITE,
				LED_ID_MAX
			};
			all_led[LED_COLOR_WHITE] = white_led;
			all_led[LED_COLOR_BLUE] = blue_led;
			all_led[LED_COLOR_GREEN] = green_led;
			all_led[LED_COLOR_RED] = red_led;
		}
		break;
#endif
	}

	for (i = 0; i < LED_COLOR_MAX; ++i) {
		p = all_led[i];
		v = (i == color)? LED_ON : LED_OFF;
		while (*p >= 0 && *p < LED_ID_MAX) {
			led_control(*p++, v);
		}
	}

#if defined(TUFAX4200) || defined(TUFAX6000)
	if (switch_led_color >= 0 && switch_led_color < LED_COLOR_MAX) {
		if (color == switch_led_color) {
			force_gpy211_led_onoff(5, 1);	/* 2.5G LAN LED */
			force_gpy211_led_onoff(6, 1);	/* 2.5G WAN LED, active-low */
			force_mt7531_led_onoff(1);	/* LAN1~LAN4 LED */
		} else {
			force_gpy211_led_onoff(5, 0);	/* 2.5G LAN LED */
			force_gpy211_led_onoff(6, 0);	/* 2.5G WAN LED, active-low */
			force_mt7531_led_onoff(0);	/* LAN1~LAN4 LED */
		}
	}
#endif

#if defined(RTCONFIG_SWITCH_RTL8370M_PHY_QCA8033_X2) || \
    defined(RTCONFIG_SWITCH_RTL8370MB_PHY_QCA8033_X2)
	if (rtk_switch_led_color >= 0 && rtk_switch_led_color < LED_COLOR_MAX) {
		if (color == rtk_switch_led_color) {
			eval("rtkswitch", "100", "3");	/* Turn on GROUP0 LEDs */
		} else {
			eval("rtkswitch", "100", "2");	/* Turn off GROUP0 LEDs */
		}
	}
#endif

	puts("1");
	return 0;
}

void AllRedLedOn(void)
{
	setAllSpecificColorLedOn(LED_COLOR_RED);
}

int isValidMacAddr(const char* mac)
{
	int sec_byte;
	int i = 0, s = 0;

	if (strlen(mac) != 17 || !strcmp("00:00:00:00:00:00", mac))
		return 0;

	while (*mac && i < 12) {
		if (isxdigit(*mac)) {
			if (i == 1) {
				sec_byte= strtol(mac, NULL, 16);
				if ((sec_byte & MULTICAST_BIT)||(sec_byte & UNIQUE_OUI_BIT))
					break;
			}
			i++;
		}
		else if (*mac == ':') {
			if (i == 0 || i/2-1 != s)
				break;
			++s;
		}
		++mac;
	}
	return (i == 12 && s == 5);
}

int
isValidCountryCode(const char *Ccode)
{
	const char *c = Ccode;
	int i = 0;

	if (strlen(Ccode) == 2) {
		while (i < 2) { //0~9, A~F
			if ((*c > 0x2F && *c < 0x3A) || (*c > 0x40 && *c < 0x5B)) {
				i++;
				c++;
			}
			else
				break;
		}
	}
	if (i == 2)
		return 1;
	else
		return 0;
}

int
isNumber(const char *num)
{
	const char *c = num;
	int i = 0, len = 0;

	len = strlen(num);
	while (i < len) { //0~9
		if (*c >= '0' && *c <= '9') {
			i++;
			c++;
		}
		else
			break;
	}
	if (i == len)
		return 1;
	else
		return 0;
}

int
isValidRegrev(const char *regrev) {
	char *c = (char *) regrev;
	int len, i = 0, ret = 0;

	len = strlen(regrev);

	if (len == 1 || len == 2 || len == 3) {
		while (i < len) { //0~9
			if (*c > 0x2F && *c < 0x3A) {
				i++;
				c++;
				ret = 1;
			}
			else {
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

int
isValidChannel(int is_2G, char *channel)
{
	char *c = channel;
	int len, i = 0, ret=0;

	len = strlen(channel);

	if ((is_2G && (len == 1 || len == 2))
	||  (!is_2G && (len == 2 || len == 3))) {
		while (i < len) { //0~9
			if ((*c > 0x2F && *c < 0x3A)) {
				i++;
				c++;
				ret = 1;
			}
			else {
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

int
pincheck(const char *a)
{
	unsigned char *c = (unsigned char *) a;
	unsigned long int uiPINtemp = atoi(a);
	unsigned long int uiAccum = 0;
	int i = 0;

	for (;;) {
		if (*c > 0x39 || *c < 0x30)
			break;
		else
			i++;
		if (!*c++ || i == 8)
			break;
	}
	if (i == 8) {
		uiAccum += 3 * ((uiPINtemp / 10000000) % 10);
		uiAccum += 1 * ((uiPINtemp / 1000000) % 10);
		uiAccum += 3 * ((uiPINtemp / 100000) % 10);
		uiAccum += 1 * ((uiPINtemp / 10000) % 10);
		uiAccum += 3 * ((uiPINtemp / 1000) % 10);
		uiAccum += 1 * ((uiPINtemp / 100) % 10);
		uiAccum += 3 * ((uiPINtemp / 10) % 10);
		uiAccum += 1 * ((uiPINtemp / 1) % 10);
		if (0 != (uiAccum % 10)) {
			return 0;
		}
		return 1;
	}
	else
		return 0;
}

int is0to9AtoZ(const char *str)
{
	int i = 0;
	unsigned char *c = (unsigned char *) str;

	while (i < strlen(str)) {
		/*  0~9 & A~Z */
		if (!((*c > 0x2F && *c < 0x3A) || (*c > 0x40 && *c < 0x5B)))
			return 0;

		c++;
		i++;
	}

	return 1;
}

int isValidSN(const char *sn)
{
	if ( (strlen(sn) != SERIAL_NUMBER_LENGTH)
		&& (strlen(sn) < (SERIAL_NUMBER_LENGTH+3) || strlen(sn) > SERIAL_NUMBER_LENGTH32 )
	   )
		return 0;

	return is0to9AtoZ(sn);
}

int isValidEISN(const char *eisn)
{
	if (strlen(eisn) < 6 || strlen(eisn) > SERIAL_NUMBER_LENGTH32)
		return 0;

	return is0to9AtoZ(eisn);
}

#if defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK)
int isResetFactory(const char *str)
{
	char reset[] = "NONE";

	if (strlen(str)==strlen(reset) && !strncmp(str, reset, strlen(reset)))
		return 1;

	return 0;
}

/* find the first 0xff/0x0 byte of FRead buffer */
int lenFRead(const unsigned char *buf, const int len)
{
	int i = 0;
	unsigned char *c = (unsigned char *) buf;

	while (i < len) {
		if (*c == 0xFF || *c == 0xff || *c == 0x00)
			break;

		c++;
		i++;
	}

	return i;
}
#endif

#define USB_HUB_PORT_NUM_MAX 8
int
Get_USB_Port_Info(const char *port_x)
{
	char output_buf[12*USB_HUB_PORT_NUM_MAX];
	char usb_pid[14];
	char usb_vid[14];
	char usb_pid_x[16];
	char usb_vid_x[16];
	char tmp[12];
	int sub_port = 0;
	int detect = 0;

	snprintf(usb_pid, sizeof(usb_pid), "usb_path%s_pid", port_x);
	snprintf(usb_vid, sizeof(usb_vid), "usb_path%s_vid", port_x);

	if (strcmp(nvram_safe_get(usb_pid),"") && strcmp(nvram_safe_get(usb_vid),"")) {
		snprintf(output_buf, sizeof(output_buf), "%s/%s",nvram_safe_get(usb_pid),nvram_safe_get(usb_vid));
		puts(output_buf);
	}
	else {
		for( sub_port = 1; sub_port <= USB_HUB_PORT_NUM_MAX; sub_port++) {
			snprintf(usb_pid_x, sizeof(usb_pid_x), "usb_path%s.%d_pid", port_x, sub_port);
			snprintf(usb_vid_x, sizeof(usb_vid_x), "usb_path%s.%d_vid", port_x, sub_port);
			if (strcmp(nvram_safe_get(usb_pid_x),"") && strcmp(nvram_safe_get(usb_vid_x),"")) {

				snprintf(tmp, sizeof(tmp), "%s%s/%s", detect ? ";" : "", nvram_safe_get(usb_pid_x),nvram_safe_get(usb_vid_x));
				if(detect)
					strncat(output_buf, tmp, sizeof(output_buf) - strlen(output_buf) - 1);
				else
					snprintf(output_buf, sizeof(output_buf), "%s", tmp);
				detect++;
			}	
		}
		if(detect)
			puts(output_buf);
		else
			puts("N/A");
	}

	return 1;
}

int
Get_USB_Port_Folder(const char *port_x)
{
	char output_buf[6 * USB_HUB_PORT_NUM_MAX];
	char usb_folder[19];
	char usb_folder_x[21];
	char tmp[8];
        int sub_port = 0;
        int detect = 0;

	snprintf(usb_folder, sizeof(usb_folder), "usb_path%s_fs_path0", port_x);

	if (strcmp(nvram_safe_get(usb_folder),""))
		puts(nvram_safe_get(usb_folder));
	else {
		for( sub_port = 1; sub_port <= USB_HUB_PORT_NUM_MAX; sub_port++) {
                        snprintf(usb_folder_x, sizeof(usb_folder_x), "usb_path%s.%d_fs_path0", port_x, sub_port);
                        if (strcmp(nvram_safe_get(usb_folder_x),"")) {

                                snprintf(tmp, sizeof(tmp), "%s%s", detect ? ";" : "", nvram_safe_get(usb_folder_x));
                                if(detect)
                                        strncat(output_buf, tmp, sizeof(output_buf) - strlen(output_buf) - 1);
                                else
                                        snprintf(output_buf, sizeof(output_buf), "%s", tmp);
                                detect++;
                        }
                }
                if(detect)
                        puts(output_buf);
                else
                        puts("N/A");

	}

	return 1;
}

int
Get_USB_Port_DataRate(const char *port_x)
{
	char output_buf[12 * USB_HUB_PORT_NUM_MAX];
	char usb_speed[19];
	char usb_speed_x[19];
	char tmp[12];
	int sub_port = 0;
	int detect = 0;

	snprintf(usb_speed, sizeof(usb_speed), "usb_path%s_speed", port_x);

	if (strcmp(nvram_safe_get(usb_speed),"")) {
		sprintf(output_buf, "%sMbps", nvram_safe_get(usb_speed));
		puts(output_buf);
	}
	else {
		for( sub_port = 1; sub_port <= USB_HUB_PORT_NUM_MAX; sub_port++) {
                        snprintf(usb_speed_x, sizeof(usb_speed_x), "usb_path%s.%d_speed", port_x, sub_port);
                        if (strcmp(nvram_safe_get(usb_speed_x),"")) {

                                snprintf(tmp, sizeof(tmp), "%s%sMbps", detect ? ";" : "", nvram_safe_get(usb_speed_x));
                                if(detect)
                                        strncat(output_buf, tmp, sizeof(output_buf) - strlen(output_buf) - 1);
                                else
                                        snprintf(output_buf, sizeof(output_buf), "%s", tmp);
                                detect++;
                        }
                }
                if(detect)
                        puts(output_buf);
                else
                        puts("N/A");

	}

	return 1;
}

int
Get_SD_Card_Info(void)
{
	char check_cmd[48];
	char sd_info_buf[128];
	int get_sd_card = 1;
	FILE *fp;

	if (!strcmp(nvram_safe_get("usb_path3_fs_path0"), "")) {
		puts("0");
		return 1;
	}

	sprintf(check_cmd, "test_disk2 %s &> /var/sd_info.txt", nvram_safe_get("usb_path3_fs_path0"));
	system(check_cmd);

	if ((fp = fopen("/var/sd_info.txt", "r")) != NULL) {
		while (fgets(sd_info_buf, 128, fp) != NULL) {
			if (strstr(sd_info_buf, "No partition")||strstr(sd_info_buf, "No disk"))
				get_sd_card=0;
		}
		if (get_sd_card)
			puts("1");
		else
			puts("0");
		fclose(fp);
		eval("rm", "-rf", "/var/sd_info.txt");
	}
	else
		puts("ATE_ERROR");

	return 1;
}

int
Get_SD_Card_Folder(void)
{
	if (strcmp(nvram_safe_get("usb_path3_fs_path0"),""))
		puts(nvram_safe_get("usb_path3_fs_path0"));
	else
		puts("N/A");

	return 1;
}

int Ej_device(const char *dev_no)
{
	if (dev_no == NULL || *dev_no < '1' || *dev_no > '9')
		return 0;

	eval("ejusb", (char*)dev_no);
	puts("1");

	return 1;
}

#if defined(RTCONFIG_EJUSB_BTN)
#define MAX_NR_EJBTN	2
void get_usb_port_eject_button(unsigned int port)
{
	int i, ejbtn, found = 0;
	char *v, nv[sizeof("btn_ejusb1_gpioXXX")];
	char nv2[sizeof("btn_ejusb_btn1XXX")];

	if (!port || port > MAX_NR_EJBTN) {
		puts("0");
		return;
	}

	for (i = 1; i <= MAX_NR_EJBTN; ++i) {
		snprintf(nv, sizeof(nv), "btn_ejusb%d_gpio", i);
		v = nvram_get(nv);
		if (!v)
			continue;
		ejbtn = (atoi(v) & GPIO_EJUSB_MASK) >> GPIO_EJUSB_SHIFT;
		if (ejbtn && ejbtn != (1 << (port - 1)))
			continue;

		found = 1;
		snprintf(nv2, sizeof(nv2), "btn_ejusb_btn%d", i);
		puts(nvram_safe_get(nv2));
		break;
	}

	if (!found)
		puts("0");
}
#endif

#if defined(RTCONFIG_RGBLED) && defined(RTCONFIG_I2CTOOLS) && !defined(HND_ROUTER)
#define NUM_OF_GROUPS	1
#ifdef GTAC2900
#define NUM_OF_SETS	5
#elif defined(GTAXE11000)
#define NUM_OF_SETS    3
#else
#define NUM_OF_SETS	1
#endif
static int setRogRGBLedTest(int RGB)
{
	char baseAddr = 0x00;
	char groupBaseAddr = 0x20;
	char sw_mode = 0x01;
	int i;
	char rgb[3] ={0};
	char rgb_cmd[255] = {0};

	for(i = 0; i < 3; ++i)
	{
		if(i == RGB)
			rgb[i] = 0xff;
	}

	//auto mode check
	if(RGB == 3)
		sw_mode = 0x00;

	//set all group(max 1) RED LED
	for(i = 0; i < 1; ++i)
	{
		if(sw_mode == 0x01)
		{
			//set to sw mode
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", groupBaseAddr);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x%02x i", sw_mode);
			system(rgb_cmd);
			//apply
			groupBaseAddr += 0x0f;
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", groupBaseAddr);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x01 i");
			system(rgb_cmd);

			//set Red
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", baseAddr);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x%02x i", rgb[0]);
			system(rgb_cmd);
			//set Blue
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", ++baseAddr);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x%02x i", rgb[1]);
			system(rgb_cmd);
			//set Green
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", ++baseAddr);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x%02x i", rgb[2]);
			system(rgb_cmd);
			++baseAddr;
			++groupBaseAddr;
		}
		else if(sw_mode == 0x00)
		{
			//set to auto mode
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", groupBaseAddr);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x%02x i", sw_mode);
			system(rgb_cmd);
			++groupBaseAddr;

			//set auto mode effect (glowing yoyo effect)
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", groupBaseAddr);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x0c i");
			system(rgb_cmd);

			//apply
			groupBaseAddr += 0x0e;
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", groupBaseAddr);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x01 i");
			system(rgb_cmd);
			++groupBaseAddr;
		}
	}

	puts("1");
	return 0;
}

int setRogRGBLedSetTest(int SET)
{
	char baseAddr = 0x00;
	char groupBaseAddr = 0x20;
	int i, y;
	char rgb_cmd[255] = {0};

	//set all group RED LED
	for(i = 0; i < NUM_OF_GROUPS; ++i)
	{
		//set to sw mode
		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", groupBaseAddr);
		system(rgb_cmd);
		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x%02x i", 0x01);
		system(rgb_cmd);
		//apply
		groupBaseAddr += 0x0f;
		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", groupBaseAddr);
		system(rgb_cmd);
		snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x01 i");
		system(rgb_cmd);

		for(y = 0; y < NUM_OF_SETS; ++y)
		{
			//set Red
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", baseAddr);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x%02x i", (y == SET ? 0xff : 0x00));
			system(rgb_cmd);
			//set Blue
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", ++baseAddr);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x%02x i", (y == SET ? 0xff : 0x00));
			system(rgb_cmd);
			//set Green
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0 0x80 0x%02x i", ++baseAddr);
			system(rgb_cmd);
			snprintf(rgb_cmd, sizeof(rgb_cmd), "i2cset -y 0 0x4e 0x03 0x01 0x%02x i", (y == SET ? 0xff : 0x00));
			system(rgb_cmd);
			++baseAddr;
		}
		++groupBaseAddr;
	}

	puts("1");
	return 0;
}
#endif

void asus_ate_StartATEMode(void)
{
	nvram_set("asus_mfg", "1");
#if defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
	nvram_set(ATE_QCA_FACTORY_MODE_STR(), "1");
#endif
#ifdef RTCONFIG_QSR10G
	start_ate_mode_qsr10g();
#endif
	stop_services_mfg();
}

#if defined(RTCONFIG_AI_SERVICE)
int GetAIMacAddr(){
	int ret = -1;
	char tmp[32] = {0};
	
	ret = system("ping -c 1 169.254.0.2 > /dev/null 2>&1");
	if(ret) return ret;

	ret = system("nvram set ate_ai_macaddr=$(arp 169.254.0.2 | awk '{print toupper($4)}')");
	if(ret) return ret;

	strlcpy(tmp, nvram_safe_get("ate_ai_macaddr"), sizeof(tmp));
	puts(tmp);
	
	return ret;	
}
#endif

int asus_ate_command(const char *command, const char *value, const char *value2)
{
	//_dprintf("===[ATE %s %s]===\n", command, value);
#ifdef RTCONFIG_QTN
	if (!nvram_match("qtn_ready", "1")) {
		_dprintf("ATE Error: wireless 5G not ready\n");
		return 0;
	}
#endif
	/*** ATE Set function ***/
	if (!strcmp(command, "Set_StartATEMode")) {
		if (!nvram_match("noconsole", "0")) {
			pid_t pid, pid1;
			char *nv_commit[] = { "nvram", "commit", NULL };
			char *console[] = { "console", NULL };

			nvram_set("noconsole", "0");
			_eval(nv_commit, NULL, 0, &pid);
			killall_tk("console");
			_eval(console, NULL, 0, &pid1);
		}
		asus_ate_StartATEMode();
		stop_wanduck();
#if defined(RTCONFIG_GPIOX3_RGBLED) \
 || defined(RTCONFIG_PWMX1_GPIOX3_RGBLED) \
 || defined(RTCONFIG_PWMX2_GPIOX1_RGBLED) \
 || defined(RTCONFIG_PWMX3_RGBLED)
		set_rgbled(RGBLED_ATE_MODE);
#endif
		puts("1");
		return 0;
	}
#if defined(RTCONFIG_AI_SERVICE)
	else if (!strcmp(command, "Get_MacAddr_AI")) {
		int ai_ret = -1;
		ai_ret = GetAIMacAddr();
		if(ai_ret) puts("Get_MacAddr FAIL");
		return 0;
	}
#endif
#ifdef BLUECAVE
	else if (!strcmp(command, "Set_AllRedLedOn")) {
		return setAllRedLedOn();
	}
	else if (!strcmp(command, "Set_AllBlueLedOn")) {
		return setAllBlueLedOn();
	}
	else if (!strcmp(command, "Set_AllBlueLedLightOn")) {
		return setAllBlueLedLight_level1();
	}
	else if (!strcmp(command, "Set_CentralLedLv")) {
		return setCentralLedLv(atoi(value));
	}
#endif
#if defined(RTCONFIG_RGBLED) && defined(RTCONFIG_I2CTOOLS)
	else if (!strcmp(command, "Set_RogRGBRedLedOn")) {
		return setRogRGBLedTest(0);
	}
	else if (!strcmp(command, "Set_RogRGBBlueLedOn")) {
		return setRogRGBLedTest(1);
	}
	else if (!strcmp(command, "Set_RogRGBGreenLedOn")) {
		return setRogRGBLedTest(2);
	}
	else if (!strcmp(command, "Set_RogRGBLedModeOn")) {
		return setRogRGBLedTest(3);
	}
	else if (!strcmp(command, "Set_AllRogRGBLedOff")) {
		return setRogRGBLedTest(4);
	}
	else if (!strcmp(command, "Set_AllRogRGBLedOn")) {
		return setRogRGBLedTest(5);
	}
	else if (!strcmp(command, "Set_RogRGBSet_LedOn")) {
		return setRogRGBLedSetTest(atoi(value));
	}
#endif
#ifdef RTCONFIG_ALPINE
	else if (!strcmp(command, "Set_FanRead")) {
		return fanCtrl(5);
	}
	else if (!strcmp(command, "Set_FanMax")) {
		return fanCtrl(4);
	}
	else if (!strcmp(command, "Set_FanMidwell")) {
		return fanCtrl(3);
	}
	else if (!strcmp(command, "Set_FanMid")) {
		return fanCtrl(2);
	}
	else if (!strcmp(command, "Set_FanLow")) {
		return fanCtrl(1);
	}
	else if (!strcmp(command, "Set_FanStop")) {
		return fanCtrl(0);
	}
#endif
	else if (!strcmp(command, "Set_AllLedOn")) {
#if defined(RTCONFIG_LP5523)
		lp55xx_leds_proc(LP55XX_ALL_LEDS_ON, LP55XX_ACT_NONE);
		puts("1");
		return 0;
#else
#ifdef RTCONFIG_QCA_PLC_UTILS
		set_plc_all_led_onoff(1);
#endif
		return setAllLedOn();
#endif
	}
#ifdef CONFIG_BCMWL5
	else if (!strcmp(command, "Set_NetLed")) {
		return setNetLed();

	}
#endif
	else if (!strcmp(command, "Set_AllLedOn2")) {
		return setAllLedOn2();
	}
	else if (!strcmp(command, "Set_AllLedOff")) {
#if defined(RTCONFIG_LP5523)
		lp55xx_leds_proc(LP55XX_ALL_LEDS_OFF, LP55XX_ACT_NONE);
		puts("1");
		return 0;
#else
#ifdef RTCONFIG_QCA_PLC_UTILS
		set_plc_all_led_onoff(0);
#endif
		return setAllLedOff();
#endif
	}
#if defined(RPAC53) || defined(RT4GAC68U) || defined(RPAC66)
	else if (!strcmp(command, "Set_AllOrangeLedOn")) {
		return setAllOrangeLedOn();
	}
#endif
#if defined(RPAC51) || defined(RPAC55) || defined(RPAC66) || defined(RPAC92) || defined(RT4GAC86U) || defined(RT4GAX56)
	else if (!strcmp(command, "Set_AllBlueLedOn"))  {
		return setAllBlueLedOn();
	}
#endif
#if defined(RPAC53) || defined(RPAC66) || defined(RPAC92)
	else if (!strcmp(command, "Set_AllGreenLedOn"))  {
		return setAllGreenLedOn();
	}
#endif
#if defined(EBA63)
	else if (!strcmp(command, "Set_AllYellowLedOn"))  {
		return setAllYellowLedOn();
	}
#endif
#if defined(RPAC92) || defined(RT4GAC86U) || defined(RT4GAX56)
	else if (!strcmp(command, "Set_AllYellowLedOn"))  {
		return setAllYellowLedOn();
	}
	else if (!strcmp(command, "Set_AllWhiteLedOn"))  {
		return setAllWhiteLedOn();
	}
#if defined(RPAC92)
	else if (!strcmp(command, "Set_AllPurpleLedOn"))  {
		return setAllPurpleLedOn();
	}
#endif
#endif
#if defined(RPAC53) || defined(RPAC51) || defined(RPAC55) || defined(RPAC66) || defined(RPAC92) || defined(RT4GAX56)
	else if (!strcmp(command, "Set_AllRedLedOn"))  {
		return setAllRedLedOn();
	}
#endif
	else if (!strcmp(command, "Set_AllLedOn_Half")) {
		puts("ATE_ERROR"); //Need to implement for EA-N66U
		return EINVAL;
	}
	else if (!strcmp(command, "Set_AllWhiteLedOn")) {
#if defined(BQ16) || defined(BQ16_PRO) || defined(BT10)
		return setAllWhiteLedOn();
#else
		return setAllSpecificColorLedOn(LED_COLOR_WHITE);
#endif
	}
	else if (!strcmp(command, "Set_AllBlueLedOn")) {
		return setAllSpecificColorLedOn(LED_COLOR_BLUE);
	}
	else if (!strcmp(command, "Set_AllRedLedOn")) {
		return setAllSpecificColorLedOn(LED_COLOR_RED);
	}
	else if (!strcmp(command, "Set_AllGreenLedOn")) {
		return setAllSpecificColorLedOn(LED_COLOR_GREEN);
	}
	else if (!strcmp(command, "Set_AllOrangeLedOn")) {
		return setAllSpecificColorLedOn(LED_COLOR_ORANGE);
	}
#if defined(RPAX58) || defined(RPBE58)
	else if (!strcmp(command, "Set_AllYellowLedOn"))  {
		return setAllSpecificColorLedOn(LED_COLOR_ORANGE);
	}
	else if (!strcmp(command, "Set_AllPurpleLedOn"))  {
		return setAllSpecificColorLedOn(LED_COLOR_PURPLE);
	}
#endif
#if defined(RTAX82U) || defined(DSL_AX82U) || defined(GSAX3000) || defined(GSAX5400) || defined(TUFAX5400) || defined(GTAX11000_PRO) || defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTAX6000) || defined(GT10) || defined(RTAX82U_V2) || defined(TUFAX5400_V2) || defined(GTBE96) || defined(GTBE19000) || defined(GTBE19000AI) || defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO) || defined(GT7) || defined(GTBE96_AI)
	else if (!strcmp(command, "Set_Red1LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[0], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP1_RED, LED_ON);
		puts("1");
		return 0;
#if !defined(TUFAX5400) && !defined(TUFAX5400_V2)
	} else if (!strcmp(command, "Set_Red2LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[3], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP2_RED, LED_ON);
		puts("1");
		return 0;
	} else if (!strcmp(command, "Set_Red3LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[6], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP3_RED, LED_ON);
		puts("1");
		return 0;
#if defined(GTAX6000)
	} else if (!strcmp(command, "Set_Red4LedOn")) {
		setAllLedOff();
		AntennaGroupReset(LED_ON);
		setAntennaGroupOn();
		puts("1");
		return 0;
#elif !defined(GTAX11000_PRO) && !defined(GTAXE16000) && !defined(GTBE98) && !defined(GT10) && !defined(GTBE98_PRO) && !defined(GTBE96) && !defined(GSBE18000) && !defined(GSBE12000) && !defined(GS7_PRO)
	} else if (!strcmp(command, "Set_Red4LedOn")) {
#ifdef GT7
		setAllRedLedOn();
#else
		setAllLedOff();
		cled_set(cled_gpio[9], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP4_RED, LED_ON);
#endif
		puts("1");
		return 0;
#if defined(GSAX3000) || defined(GSAX5400)
	} else if (!strcmp(command, "Set_Red5LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[12], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP5_RED, LED_ON);
		puts("1");
		return 0;
#endif
#endif
#endif
	} else if (!strcmp(command, "Set_Green1LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[1], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP1_GREEN, LED_ON);
		puts("1");
		return 0;
#if !defined(TUFAX5400) && !defined(TUFAX5400_V2)
	} else if (!strcmp(command, "Set_Green2LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[4], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP2_GREEN, LED_ON);
		puts("1");
		return 0;
	} else if (!strcmp(command, "Set_Green3LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[7], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP3_GREEN, LED_ON);
		puts("1");
		return 0;
#if !defined(GTAX11000_PRO) && !defined(GTAXE16000) && !defined(GTBE98) && !defined(GTAX6000) && !defined(GT10) && !defined(GTBE98_PRO) && !defined(GTBE96) && !defined(GSBE18000) && !defined(GSBE12000) && !defined(GS7_PRO)
	} else if (!strcmp(command, "Set_Green4LedOn")) {
#ifdef GT7
		setAllGreenLedOn();
#else
		setAllLedOff();
		cled_set(cled_gpio[10], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP4_GREEN, LED_ON);
#endif
		puts("1");
		return 0;
#if defined(GSAX3000) || defined(GSAX5400)
	} else if (!strcmp(command, "Set_Green5LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[13], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP5_GREEN, LED_ON);
		puts("1");
		return 0;
#endif
#endif
#endif
	} else if (!strcmp(command, "Set_Blue1LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[2], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP1_BLUE, LED_ON);
		puts("1");
		return 0;
#if !defined(TUFAX5400) && !defined(TUFAX5400_V2)
	} else if (!strcmp(command, "Set_Blue2LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[5], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP2_BLUE, LED_ON);
		puts("1");
		return 0;
	} else if (!strcmp(command, "Set_Blue3LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[8], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP3_BLUE, LED_ON);
		puts("1");
		return 0;
#if !defined(GTAX11000_PRO) && !defined(GTAXE16000) && !defined(GTBE98) && !defined(GTAX6000) && !defined(GT10) && !defined(GTBE98_PRO) && !defined(GTBE96) && !defined(GSBE18000) && !defined(GSBE12000) && !defined(GS7_PRO)
	} else if (!strcmp(command, "Set_Blue4LedOn")) {
#ifdef GT7
		setAllBlueLedOn();
#else
		setAllLedOff();
		cled_set(cled_gpio[11], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP4_BLUE, LED_ON);
#endif
		puts("1");
		return 0;
#if defined(GSAX3000) || defined(GSAX5400)
	} else if (!strcmp(command, "Set_Blue5LedOn")) {
		setAllLedOff();
		cled_set(cled_gpio[14], CLED_BRIGHTNESS_ON, 0x0, 0x0, 0x0);
		led_control(LED_GROUP5_BLUE, LED_ON);
		puts("1");
		return 0;
#endif
#endif
#endif
#ifdef GTAX6000
	} else if (!strcmp(command, "Set_AntLedOn")) {
		setAllLedOff();
		AntennaGroupReset(LED_ON);
		setAntennaGroupOn();
		puts("1");
		return 0;
#endif
	}
#endif
#ifdef RTCONFIG_BCMARM
	else if (!strcmp(command, "Set_WanLedMode1")) {
		return setWanLedMode1();
	}
	else if (!strcmp(command, "Set_WanLedMode2")) {
		return setWanLedMode2();
	}
	else if (!strcmp(command, "Set_AteModeLedOn")) {
		return setATEModeLedOn();
	}
#endif
	else if (!strcmp(command, "Set_MacAddr_2G") || !strcmp(command, "Set_MacAddr")) {
		if (checkPASS("") < 0) {
			puts("ATE_ERROR_CLEAR_PASS_FIRST");
			return EINVAL;
		}
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		//Andy Chiu, 2016/02/04.
		char *p = (char *) value;
		char UpperMac[20] = {0};
		int i;
		for(i = 0; p[i]; ++i)
		{
			UpperMac[i] = toupper(p[i]);
		}
#if 0
		char tmp[256];
		snprintf(tmp, sizeof(tmp), "<%s;%s>", value, UpperMac);
		puts(tmp);
#endif
		if (!setMAC_2G(UpperMac))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(command, "Set_MacAddr_5G")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		//Andy Chiu, 2016/02/04.
		char *p = (char *) value;
		char UpperMac[20] = {0};
		int i;
		for(i = 0; p[i]; ++i)
		{
			UpperMac[i] = toupper(p[i]);
		}

#ifdef RTCONFIG_QTN
		if (!setMAC_5G_qtn(UpperMac))
#else
		if (!setMAC_5G(UpperMac))
#endif
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#ifdef RTCONFIG_HAS_5G_2
	else if (!strcmp(command, "Set_MacAddr_5G_2")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		//Andy Chiu, 2016/02/04.
		char *p = (char *) value;
		char UpperMac[20] = {0};
		int i;
		for(i = 0; p[i]; ++i)
		{
			UpperMac[i] = toupper(p[i]);
		}

		if (!setMAC_5G_2(UpperMac))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif
#endif	/* RTCONFIG_HAS_5G */

#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
	else if (!strcmp(command, "Set_MacAddr_6G")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		char *p = (char *) value;
		char UpperMac[20] = {0};
		int i;
		for(i = 0; p[i]; ++i)
		{
			UpperMac[i] = toupper(p[i]);
		}

		if (!setMAC_6G(UpperMac))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif

#ifdef RTCONFIG_HAS_6G_2
	else if (!strcmp(command, "Set_MacAddr_6G_2")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		char *p = (char *) value;
		char UpperMac[20] = {0};
		int i;
		for(i = 0; p[i]; ++i)
		{
			UpperMac[i] = toupper(p[i]);
		}

		if (!setMAC_6G_2(UpperMac))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif


#ifdef RPAC55
	else if (!strcmp(command, "Set_MacAddr_BT")) {
		const char *p = (char *) value;
		char UpperMac[20] = {0};
		int i;
		for (i = 0; p[i]; ++i)
		{
			UpperMac[i] = toupper(p[i]);
		}
		if ( !setMAC_BT(UpperMac) )
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif
#if defined(RTN14U)
	else if (!strcmp(command, "eeprom")) {
		if (!eeprom_upgrade(value, 1))
			return EINVAL;
		return 0;
	}
	else if (!strcmp(command, "eeover")) {
		if (!eeprom_upgrade(value, 0))
			return EINVAL;
		return 0;
	}
#endif
#if defined(RTCONFIG_NEW_REGULATION_DOMAIN)
	else if (!strcmp(command, "Set_RegSpec")) {
		if (setRegSpec(value, 1) < 0)
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		getRegSpec();
		return 0;
	}
	else if (!strcmp(command, "Set_RegulationDomain_2G")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		if (setRegDomain_2G(value) == -1) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		getRegDomain_2G();
		return 0;
	}
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(command, "Set_RegulationDomain_5G")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		if (setRegDomain_5G(value) == -1) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		getRegDomain_5G();
		return 0;
	}
#endif	/* RTCONFIG_HAS_5G */
#else	/* ! RTCONFIG_NEW_REGULATION_DOMAIN */
#if (defined(RTCONFIG_REALTEK) && defined(RTCONFIG_HAS_5G)) || defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
/* [MUST]: Why can't use the common API ??? */
	else if (!strcmp(command, "Set_RegulationDomain_5G")) {
		if (!setCountryCode_5G(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif
	else if (!strcmp(command, "Set_RegulationDomain_2G")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		if (!setCountryCode_2G(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
#ifdef RTCONFIG_QCA
		if ((value2 == NULL) || strcmp(value2,"noctl"))
			setCTL(value);
#endif
		return 0;
	}
#endif /* RTCONFIG_NEW_REGULATION_DOMAIN */
#ifdef CONFIG_BCMWL5
	else if (!strcmp(command, "Set_RegulationDomain_5G")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif

#ifdef RTCONFIG_QTN
		if (!setCountryCode_5G_qtn(value))
#else
		if (!setCountryCode_5G(value))
#endif
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#if defined(RTCONFIG_BCMARM) && defined(RTCONFIG_HAS_5G_2)
	else if (!strcmp(command, "Set_RegulationDomain_5G_2")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		if (!setCountryCode_5G_2(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif

#if defined(RTCONFIG_BCMARM) && (defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7))
	else if (!strcmp(command, "Set_RegulationDomain_6G")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		if (!setCountryCode_6G(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif
#ifdef RTCONFIG_HAS_6G_2
	else if (!strcmp(command, "Set_RegulationDomain_6G_2")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		if (!setCountryCode_6G_2(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif
	else if (!strcmp(command, "Set_Regrev_2G")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif

		if (!setRegrev_2G(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Set_Regrev_5G")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif

#ifdef RTCONFIG_QTN
		if (!setRegrev_5G_qtn(value))
#else
		if (!setRegrev_5G(value))
#endif
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#if defined(RTCONFIG_BCMARM) && defined(RTCONFIG_HAS_5G_2)
	else if (!strcmp(command, "Set_Regrev_5G_2")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		if (!setRegrev_5G_2(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif

#if defined(RTCONFIG_BCMARM) && (defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7))
	else if (!strcmp(command, "Set_Regrev_6G")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		if (!setRegrev_6G(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif
#ifdef RTCONFIG_HAS_6G_2
	else if (!strcmp(command, "Set_Regrev_6G_2")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		if (!setRegrev_6G_2(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif

	else if (!strcmp(command, "Set_Commit")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		setCommit();
		return 0;
	}
#endif
#if defined(RTN14U)
	else if (!strcmp(command, "pkt_flood")) {
		if (nvram_invmatch("asus_mfg", "0"))
		{
#if 0 // TBD
			struct sockaddr_ll dev,dev2;
			int fd,fd2,do_flag = 3;
			unsigned char buffer[1514];
			fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
			dev.sll_family = AF_PACKET;
			dev.sll_protocol = htons(ETH_P_ALL);
			dev.sll_ifindex = 4; // LAN
			bind(fd, (struct sockaddr *) &dev, sizeof(dev));

			fd2 = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
			dev2.sll_family = AF_PACKET;
			dev2.sll_protocol = htons(ETH_P_ALL);
			dev2.sll_ifindex = 5; // WAN
			bind(fd2, (struct sockaddr *) &dev2, sizeof(dev2));

			if (value) {
				if (strcmp(value,"WAN") == 0)
					do_flag = 2;
				else if (strcmp(value,"LAN") == 0)
					do_flag = 1;
			}
			memset(buffer,0xff,6);
			FRead(buffer+6, OFFSET_MAC_ADDR_2G, 6);
			memset(buffer+12,0x55,1502);
			while (1)
			{
				if (do_flag & 1)
					send(fd, buffer, 1514, 0);
				if (do_flag & 2)
					send(fd2, buffer, 1514, 0);
			}
#endif
		}
		return 0;
 	}
#endif
	else if (!strcmp(command, "Set_SerialNumber")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif

		if (!setSN(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
 	}
	else if (!strcmp(command, "Set_EmsInternalSerialNumber")) {
		if (!setEISN(value)) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#ifdef RTCONFIG_ODMPID
	else if (!strcmp(command, "Set_ModelName")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif

		if (!setMN(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif
	else if (!strcmp(command, "Set_PINCode")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif

		if (!setPIN(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Set_40M_Channel_2G")) {
		if (!set40M_Channel_2G((char*)value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(command, "Set_40M_Channel_5G")) {
		if (!set40M_Channel_5G((char*)value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif	/* RTCONFIG_HAS_5G */
	else if (!strcmp(command, "Set_RestoreDefault")) {
		int ret_reset;
#if defined(RTCONFIG_JFFS2) || defined(RTCONFIG_BRCM_NAND_JFFS2) || defined(RTCONFIG_UBIFS) || defined(RTCONFIG_JFFS_PARTITION)
		eval("touch", "/jffs/remove_hidden_flag");
#endif
#if defined(RTCONFIG_NVSW_IN_JFFS)
                nvsw_destroy();
#endif
#if defined(RTCONFIG_DHDAP) && defined(RTCONFIG_BCM_MFG)
	system("rm -f /data/debug-*.tgz");
#endif
#ifndef HND_ROUTER
		nvram_set("restore_defaults", "1");
		nvram_set(ASUS_STOP_COMMIT, "1");
#endif
#ifdef RTAC87U
		ret_reset = ResetDefault();
		if (ret_reset == 0) {
			logmessage("ATE", "Set_RestoreDefault OK");
			sleep(3);
			puts("1");
		} else {
			logmessage("ATE", "Set_RestoreDefault failed");
			sleep(3);
			puts("0");
		}
#else
		ret_reset = ResetDefault();
#endif
		return ret_reset;
	}
	else if (!strcmp(command, "Set_Eject")) {
		if (!Ej_device(value)) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#ifdef RTCONFIG_FANCTRL
	else if (!strcmp(command, "Set_FanOn") || !strcmp(command, "Set_FanMax")) {
		setFanOn();
		return 0;
	}
	else if (!strcmp(command, "Set_FanOff") || !strcmp(command, "Set_FanStop")) {
		setFanOff();
		return 0;
	}
#if defined(RTCONFIG_QCA)
	else if (!strcmp(command, "Set_FanRead")) {
		getFanSpeed();
		return 0;
	}
#endif
#endif
#ifdef CONFIG_BCMWL5
	else if (!strcmp(command, "Set_WaitTime")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif

		if (!setWaitTime(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Set_WiFi_2G")) {
		if (!setWiFi2G(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(command, "Set_WiFi_5G")) {
		if (!setWiFi5G(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif	/* RTCONFIG_HAS_5G */
#endif
#ifdef RTCONFIG_RALINK
	else if (!strcmp(command, "Set_DevFlags")) {
		if (Set_Device_Flags(value) < 0)
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Set_WanToLan")) {
	   	set_wantolan();
		unregister_hnat_wlifaces();
		modprobe_r(MTK_HNAT_MOD);
		modprobe(MTK_HNAT_MOD);
		register_hnat_wlifaces();
		stop_wanduck();
		stop_udhcpc(-1);
		return 0;
	}
#if defined(RTAC1200HP) || defined(RTN56UB1) || defined(RTN56UB2)
	else if (!strcmp(command, "Set_FixChannel")) {
		FWrite("1", OFFSET_FIX_CHANNEL, 1);
		puts("1");
		return 0;
	}
	else if (!strcmp(command, "Set_FreeChannel")) {
		FWrite("0", OFFSET_FIX_CHANNEL, 1);
		nvram_set("wl0_channel","0");
		nvram_set("wl1_channel","0");
		nvram_set("lan_stp","1");
		nvram_commit();
		puts("1");
		return 0;
	}
#endif
#if defined(RTAC85U) || defined(RTAC85P) || defined(RTN800HP) || defined(RTACRH26) || defined(TUFAC1750)
	else if (!strcmp(command, "Set_DisableStp")) {
		FWrite("1", OFFSET_BR_STP, 1);
		puts("1");
		return 0;
	}
	else if (!strcmp(command, "Set_EnableStp")) {
		FWrite("0", OFFSET_BR_STP, 1);
		nvram_set("lan_stp","1");
		nvram_commit();
		puts("1");
		return 0;
	}
#endif
#endif
	else if (!strcmp(command, "Set_XSetting")) {
		if (value == NULL || strcmp(value, "1")) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		else {
			nvram_set("x_Setting", "1");
			puts(nvram_get("x_Setting"));
		}
		return 0;
	}
#if defined(CONFIG_BCMWL5) || defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RPAC92) 
	else if (!strcmp(command, "Set_HwId")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
                if (set_HwId(value) < 0)
                {
                        puts("ATE_ERROR_INCORRECT_PARAMETER");
                        return EINVAL;
                }
                return 0;
	}
	else if (!strcmp(command, "Set_HwVersion")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
                if (set_HwVersion(value) < 0)
                {
                        puts("ATE_ERROR_INCORRECT_PARAMETER");
                        return EINVAL;
                }
                return 0;
	}
	else if (!strcmp(command, "Set_HwBom")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
                if (set_HwBom(value) < 0)
                {
                        puts("ATE_ERROR_INCORRECT_PARAMETER");
                        return EINVAL;
                }
                return 0;
	}
	else if (!strcmp(command, "Set_DateCode")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
                if (set_DateCode(value) < 0)
                {
                        puts("ATE_ERROR_INCORRECT_PARAMETER");
                        return EINVAL;
                }
                return 0;
	}
#endif
#ifdef RTCONFIG_ISP_CUSTOMIZE
	else if (!strcmp(command, "Set_DeleteFile")) {
		int ret;
		if (value == NULL || strlen(value) <= 0) {
			puts("ATE_ERROR");
			return EINVAL;
		}
		if ((ret = delete_file(value)) == 0)
			puts("ATE_SUCCESS");
		else {
			if (ret == -2)
				puts("ATE_ERROR_FILE_NOT_FOUND");
			else
				puts("ATE_ERROR");
		}
		return 0;
	}
	else if (!strcmp(command, "Set_IspDeleteFile")) {
		int ret;
		if ((ret = delete_package()) == 0)
			puts("ATE_SUCCESS");
		else {
			if (ret == -2)
				puts("ATE_ERROR_FILE_NOT_FOUND");
			else
				puts("ATE_ERROR");
		}
		return 0;
	}
#endif
	/*** ATE Get functions ***/
	else if (!strcmp(command, "Get_FWVersion")) {
		char fwver[16];
		snprintf(fwver, sizeof(fwver), "%s.%s", nvram_safe_get("firmver"), nvram_safe_get("buildno"));
		puts(fwver);
		return 0;
	}
	else if (!strcmp(command, "Get_BootLoaderVersion")) {
		getBootVer();
		return 0;
	}
	else if (!strcmp(command, "Get_ResetButtonStatus")) {
		puts(nvram_safe_get("btn_rst"));
		return 0;
	}
	else if (!strcmp(command, "Get_WpsButtonStatus") || !strcmp(command, "Get_PairingButtonStatus")) {
		puts(nvram_safe_get("btn_ez"));
		return 0;
	}
#if defined(RTCONFIG_EJUSB_BTN)
	else if (!strcmp(command, "Get_EjectUsbButton1Status")) {
		puts(nvram_safe_get("btn_ejusb_btn1"));
		return 0;
	}
	else if (!strcmp(command, "Get_EjectUsbButton2Status")) {
		puts(nvram_safe_get("btn_ejusb_btn2"));
		return 0;
	}
	else if (!strcmp(command, "Get_UsbPort1EjectButtonStatus")) {
		get_usb_port_eject_button(1);
		return 0;
	}
	else if (!strcmp(command, "Get_UsbPort2EjectButtonStatus")) {
		get_usb_port_eject_button(2);
		return 0;
	}
#endif
#ifdef RTCONFIG_WIFI_TOG_BTN
	else if (!strcmp(command, "Get_WirelessButtonStatus")) {
		puts(nvram_safe_get("btn_wifi_toggle"));
		return 0;
	}
#endif
#ifdef RTCONFIG_TURBO_BTN
	else if (!strcmp(command, "Get_TurboButtonStatus")) {
		puts(nvram_safe_get("btn_turbo"));
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_SSID_2G")) {
#if defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO)
		puts(nvram_safe_get("wl3_ssid"));
#elif defined(BT10) || defined(BQ16) || defined(BQ16_PRO)
		getSSID(WLIF_2G);
#elif defined(GT10) || defined(RTAX9000)
		puts(nvram_safe_get("wl2_ssid"));
#elif defined(RPAX56) || defined(RPAX58) || defined(RPBE58)
		if(nvram_match("wl0_mode", "wet"))
			puts(nvram_safe_get("wl0.1_ssid"));
		else
			puts(nvram_safe_get("wl0_ssid"));
#else
		puts(nvram_safe_get("wl0_ssid"));
#endif
		return 0;
	}
	else if (!strcmp(command, "Get_SSID_5G")) {
#if defined(RPAX56) || defined(RPAX58) || defined(RPBE58)
		if(nvram_match("wl1_mode", "wet"))
			puts(nvram_safe_get("wl1.1_ssid"));
		else
			puts(nvram_safe_get("wl1_ssid"));
#else
#if defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO)
		puts(nvram_safe_get("wl0_ssid"));
#elif defined(BT10)
		getSSID(WLIF_5G1);
#elif defined(GT10) || defined(RTAX9000)
		puts(nvram_safe_get("wl0_ssid"));
#else
		puts(nvram_safe_get("wl1_ssid"));
#endif
#endif
		return 0;
	}
	else if (!strcmp(command, "Get_SSID_5G_2")) {
#if defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(BQ16)
		puts(nvram_safe_get("wl1_ssid"));
#elif defined(BT10)
		getSSID(WLIF_5G2);
#elif defined(GT10) || defined(RTAX9000)
		puts(nvram_safe_get("wl1_ssid"));
#else
		puts(nvram_safe_get("wl2_ssid"));
#endif
		return 0;
	}
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
	else if (!strcmp(command, "Get_SSID_6G")) {
#if defined(RTCONFIG_BCMARM) && defined(RTCONFIG_HAS_6G_2)
		puts(nvram_safe_get("wl1_ssid"));
#elif defined(BT10)
		getSSID(WLIF_6G);
#else
		puts(nvram_safe_get("wl2_ssid"));
#endif
		return 0;
	}
#endif
#ifdef RTCONFIG_HAS_6G_2
	else if (!strcmp(command, "Get_SSID_6G_2")) {
		puts(nvram_safe_get("wl2_ssid"));
		return 0;
	}
#endif
#ifdef RTCONFIG_SWMODE_SWITCH
#if defined(PLAC66U)
	else if (!strcmp(command, "Get_SwitchStatus")) {
		puts(nvram_safe_get("switch_mode"));
		return 0;
	}
#endif  /* Model */
#endif  /* RTCONFIG_SWMODE_SWITCH */
#ifdef RTCONFIG_SW_BTN
	else if (!strcmp(command, "Get_SwitchStatus")) {
#if defined(RTBE58_GO)
		get_btn_switch();
#endif
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_SWMode")) {
		puts(nvram_safe_get("sw_mode"));
		return 0;
	}
	else if (!strcmp(command, "Get_MacAddr_2G") || !strcmp(command, "Get_MacAddr")) {
		getMAC_2G();
		return 0;
	}
#ifdef RTCONFIG_RALINK
	else if (!strcmp(command, "Get_SSID_2G")) {
		getSSID_2G();
		return 0;
	}
#endif
#if defined(RTCONFIG_HAS_5G)
#if defined(RTCONFIG_RALINK)	
	else if (!strcmp(command, "Get_SSID_5G")) {
		getSSID_5G();
		return 0;
	}	
#endif
	else if (!strcmp(command, "Get_MacAddr_5G")) {
#ifdef RTCONFIG_QTN
		getMAC_5G_qtn();
#else
		getMAC_5G();
#endif
		return 0;
	}
#ifdef RTCONFIG_HAS_5G_2
	else if (!strcmp(command, "Get_MacAddr_5G_2")) {
		getMAC_5G_2();
		return 0;
	}
#endif
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
	else if (!strcmp(command, "Get_MacAddr_6G")) {
		getMAC_6G();
		return 0;
	}
#endif
#ifdef RTCONFIG_HAS_6G_2
	else if (!strcmp(command, "Get_MacAddr_6G_2")) {
		getMAC_6G_2();
		return 0;
	}
#endif

#ifdef RTCONFIG_RALINK
	else if (!strcmp(command, "Get_SSID_5G")) {
		getSSID_5G();
		return 0;
	}
#endif
#endif	/* RTCONFIG_HAS_5G */
#ifdef RPAC55
	else if (!strcmp(command, "Get_MacAddr_BT")) {
		getMAC_BT();
		return 0;
	}
#endif
#if defined(RTCONFIG_NEW_REGULATION_DOMAIN)
	else if (!strcmp(command, "Get_RegSpec")) {
		getRegSpec();
		return 0;
	}
	else if (!strcmp(command, "Get_RegulationDomain_2G")) {
		getRegDomain_2G();
		return 0;
	}
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(command, "Get_RegulationDomain_5G")) {
		getRegDomain_5G();
		return 0;
	}
#endif	/* RTCONFIG_HAS_5G */
#else	/* ! RTCONFIG_NEW_REGULATION_DOMAIN */
#if (defined(RTCONFIG_REALTEK) && defined(RTCONFIG_HAS_5G)) || defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
/* [MUST] : Why can't use the same API to get country code : getCountryCode_5G() */
	else if (!strcmp(command, "Get_RegulationDomain_5G")) {
		getCountryCode_5G();
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_RegulationDomain_2G")) {
		getCountryCode_2G();
		return 0;
	}
#endif	/* ! RTCONFIG_NEW_REGULATION_DOMAIN */
#ifdef CONFIG_BCMWL5
	else if (!strcmp(command, "Get_RegulationDomain_5G")) {
#ifdef RTCONFIG_QTN
		getCountryCode_5G_qtn();
#else
	   	getCountryCode_5G();
#endif
		return 0;
	}
#if defined(RTCONFIG_BCMARM) && defined(RTCONFIG_HAS_5G_2)
	else if (!strcmp(command, "Get_RegulationDomain_5G_2")) {
		getCountryCode_5G_2();
		return 0;
	}
#endif
#if defined(RTCONFIG_BCMARM) && (defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7))
	else if (!strcmp(command, "Get_RegulationDomain_6G")) {
		getCountryCode_6G();
		return 0;
	}
#endif
#ifdef RTCONFIG_HAS_6G_2
	else if (!strcmp(command, "Get_RegulationDomain_6G_2")) {
		getCountryCode_6G_2();
		return 0;
	}
#endif

	else if (!strcmp(command, "Get_Regrev_2G")) {
		getRegrev_2G();
		return 0;
	}
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(command, "Get_Regrev_5G")) {
#ifdef RTCONFIG_QTN
		getRegrev_5G_qtn();
#else
		getRegrev_5G();
#endif
		return 0;
	}
#if defined(RTCONFIG_BCMARM) && defined(RTCONFIG_HAS_5G_2)
	else if (!strcmp(command, "Get_Regrev_5G_2")) {
		getRegrev_5G_2();
		return 0;
	}
#endif

#if defined(RTCONFIG_BCMARM) && (defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7))
	else if (!strcmp(command, "Get_Regrev_6G")) {
		getRegrev_6G();
		return 0;
	}
#endif
#ifdef RTCONFIG_HAS_6G_2
	else if (!strcmp(command, "Get_Regrev_6G_2")) {
		getRegrev_6G_2();
		return 0;
	}
#endif

#endif	/* RTCONFIG_HAS_5G */
#endif
	else if (!strcmp(command, "Get_SerialNumber")) {
		getSN();
		return 0;
	}
	else if (!strcmp(command, "Get_EmsInternalSerialNumber")) {
		getEISN();
		return 0;
	}
#ifdef RTCONFIG_ODMPID
	else if (!strcmp(command, "Get_ModelName")) {
		getMN();
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_PINCode")) {
		getPIN();
		return 0;
	}
	else if (!strcmp(command, "Get_WanLanStatus")) {
#if defined(RTCONFIG_EXT_RTL8365MB) || defined(RTCONFIG_EXT_RTL8370MB)
		GetPhyStatus(1, NULL);
#else
		if (!GetPhyStatus(1, NULL) && nvram_match(ATE_FACTORY_MODE_STR(), "1")) {
			puts("ATE_ERROR");
		}
#endif

		return 0;
	}
#if defined(HND_ROUTER)
	else if (!strcmp(command, "Get_SecureBoot")) {
		getSB();
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_FwReadyStatus")) {
		puts(nvram_safe_get("success_start_service"));
		return 0;
	}
	else if (!strcmp(command, "Get_Build_Info")) {
		puts(nvram_safe_get("buildinfo"));
		return 0;
	}
#ifdef RTCONFIG_RALINK
	else if (!strcmp(command, "Get_RSSI_2G")) {
		getrssi(0);
		return 0;
	}
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(command, "Get_RSSI_5G")) {
		getrssi(1);
		return 0;
	}
#endif	/* RTCONFIG_HAS_5G */
#endif
	else if (!strcmp(command, "Get_ChannelList_2G")) {
		if (!Get_ChannelList_2G())
			puts("ATE_ERROR");
		return 0;
	}
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(command, "Get_ChannelList_5G")) {
#ifdef RTCONFIG_QTN
		if (!Get_ChannelList_5G_qtn())
#else
		if (!Get_ChannelList_5G())
#endif
			puts("ATE_ERROR");
		return 0;
	}
#ifdef RTCONFIG_HAS_5G_2
	else if (!strcmp(command, "Get_ChannelList_5G_2")) {
		if (!Get_ChannelList_5G_2())
			puts("ATE_ERROR");
		return 0;
	}
#endif
#endif	/* RTCONFIG_HAS_5G */
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
	else if (!strcmp(command, "Get_ChannelList_6G")) {
		if (!Get_ChannelList_6G())
			puts("ATE_ERROR");
		return 0;
	}
#endif
#ifdef RTCONFIG_HAS_6G_2
	else if (!strcmp(command, "Get_ChannelList_6G_2")) {
		if (!Get_ChannelList_6G_2())
			puts("ATE_ERROR");
		return 0;
	}
#endif

#if defined(RTCONFIG_WIGIG)
	else if (!strcmp(command, "Get_ChannelList_60G")) {
		if (!Get_ChannelList_60G())
			puts("ATE_ERROR");
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_fail_ret")) {
		Get_fail_ret();
		return 0;
	}
	else if (!strcmp(command, "Get_fail_reboot_log")) {
		Get_fail_reboot_log();
		return 0;
	}
	else if (!strcmp(command, "Get_fail_dev_log")) {
		Get_fail_dev_log();
		return 0;
	}
#if defined(RTCONFIG_RALINK) && !defined(RTCONFIG_MT798X) // obsoleted error code?
#if !defined(RTN14U) && !defined(RTAC52U) && !defined(RTAC51U) && !defined(RTN11P) && !defined(RTN300) && !defined(RTN54U) && !defined(RTAC1200HP) && !defined(RTN56UB1) && !defined(RTAC54U) && !defined(RTN56UB2) && !defined(RTAC54U) && !defined(RTAC1200) && !defined(RTAC1200V2) && !defined(RTAC1200GA1) && !defined(RTAC1200GU) && !defined(RTN11P_B1) && !defined(RTN10P_V3) && !defined(RTAC51UP) && !defined(RTAC53) && !defined(RPAC87) && !defined(RTAC85U) && !defined(RTAC85P) && !defined(RTAC65U) && !defined(RTN800HP) && !defined(RTACRH26) && !defined(TUFAC1750) && !defined(RTACRH18) && !defined(RT4GAC86U) && !defined(RTCONFIG_WLMODULE_MT7915D_AP)
		return FWRITE(value, value2);
	}
	else if (!strcmp(command, "Ra_Asuscfe_2G")) {
		return asuscfe(value, WIF_2G);
	}
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(command, "Ra_Asuscfe_5G")) {
		return asuscfe(value, WIF_5G);
	}
#endif	/* RTCONFIG_HAS_5G */
	else if (!strcmp(command, "Set_SwitchPort_LEDs")) {
		if (Set_SwitchPort_LEDs(value, value2) < 0)
		{
			puts("ATE_ERROR");
			return EINVAL;
		}
		return 0;
	}
#endif
#endif
#ifdef RTCONFIG_WIRELESS_SWITCH
	else if (!strcmp(command, "Get_WifiSwStatus")) {
		puts(nvram_safe_get("btn_wifi_sw"));
		return 0;
	}
#endif

#ifdef RTCONFIG_WIFI_TOG_BTN
	else if (!strcmp(command, "Get_WifiButtonStatus")) {
		puts(nvram_safe_get("btn_wifi_toggle"));
		return 0;
	}
#endif
#ifdef RTCONFIG_LED_BTN
	else if (!strcmp(command, "Get_LedButtonStatus")) {
		puts(nvram_safe_get("btn_led"));
		return 0;
	}
#endif
#ifdef RTCONFIG_QTN
	else if (!strcmp(command, "Enable_Qtn_TelnetSrv")) {
		enable_qtn_telnetsrv(1);
		puts("1");
		return 0;
	}
	else if (!strcmp(command, "Disable_Qtn_TelnetSrv")) {
		enable_qtn_telnetsrv(0);
		puts("1");
		return 0;
	}
	else if (!strcmp(command, "Get_Qtn_TelnetSrv_Status")) {
		getstatus_qtn_telnetsrv();
		return 0;
	}
	else if (!strcmp(command, "Del_Qtn_Cal_Files")) {
		del_qtn_cal_files();
		puts("1");
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_WiFiStatus_2G")) {
		if (get_radio(0, 0))
			puts("1");
		else
			puts("0");
		return 0;
	}
#if defined(CONFIG_BCMWL5) || defined(RTCONFIG_HAS_5G)
	else if (!strcmp(command, "Get_WiFiStatus_5G")) {
		if (get_radio(1, 0))
			puts("1");
		else
			puts("0");
		return 0;
	}
#endif
	else if (!strcmp(command, "Set_WiFiStatus_2G")) {
		int act = !strcmp(value, "on");

		if (!strcmp(value, "on") && !strcmp(value, "off"))
			puts("ATE_UNSUPPORT");

		set_radio(act, 0, 0);

		if (get_radio(0, 0)) {
			if (act)
				puts("success=on");
			else
				puts("ATE_ERROR_INCORRECT_PARAMETER");
		} else {
			if (!act)
				puts("success=off");
			else
				puts("ATE_ERROR_INCORRECT_PARAMETER");
		}
		return 0;
	}
#if defined(RTCONFIG_HAS_5G)
	else if (!strcmp(command, "Set_WiFiStatus_5G")) {
		int act = !strcmp(value, "on");

		if (!strcmp(value, "on") && !strcmp(value, "off"))
			puts("ATE_UNSUPPORT");

		set_radio(act, 1, 0);

		if (get_radio(1, 0)) {
			if (act)
				puts("success=on");
			else
				puts("ATE_ERROR_INCORRECT_PARAMETER");
		} else {
			if (!act)
				puts("success=off");
			else
				puts("ATE_ERROR_INCORRECT_PARAMETER");
		}
		return 0;
	}
#endif	/* RTCONFIG_HAS_5G */
	else if (!strcmp(command, "Get_ATEVersion")) {
		puts(nvram_safe_get("Ate_version"));
		return 0;
	}
	else if (!strcmp(command, "Get_XSetting")) {
		puts(nvram_safe_get("x_Setting"));
		return 0;
	}
	else if (!strcmp(command, "Get_WaitTime")) {
		puts(nvram_safe_get("wait_time"));
		return 0;
	}
	else if (!strcmp(command, "Get_ExtendNo")) {
		puts(nvram_safe_get("extendno"));
		return 0;
	}
#ifdef RTCONFIG_RALINK
	else if (!strcmp(command, "Get_DevFlags")) {
		if (Get_Device_Flags() < 0)
		{
			puts("ATE_ERROR");
			return EINVAL;
		}
		return 0;
	}
#endif
#ifdef RTCONFIG_QCA
#if defined(RTCONFIG_WIFI_QCA9557_QCA9882) || defined(RTCONFIG_QCA953X) || defined(RTCONFIG_QCA956X) || defined(RTCONFIG_QCN550X)
	else if (!strcmp(command, "Set_ART2")) {
#ifdef RTCONFIG_ART2_BUILDIN
		Set_ART2();
#else
		if (value == NULL || strlen(value) <= 0) {
			printf("ATE_ERROR_INCORRECT_PARAMETER\n");
			return EINVAL;
		}
		Set_ART2(value);
#endif
		return 0;
	}
	else if (!strncmp(command, "Get_EEPROM_", 11)) {
		Get_EEPROM_X(command);
		return 0;
	}
	else if (!strcmp(command, "Get_CalCompare")) {
		Get_CalCompare();
		return 0;
	}
#endif
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
      defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
      defined(RTCONFIG_WIFI_QCN5024_QCN5054) || \
      defined(RTCONFIG_PCIE_AR9888) || defined(RTCONFIG_PCIE_QCA9888) || \
      defined(RTCONFIG_SOC_IPQ40XX)
	else if (!strcmp(command, "Set_Qcmbr")) {
#if defined(RTCONFIG_QCA) && defined(RTCONFIG_SOC_IPQ40XX)
		nvram_set_int("restwifi_qis", 1);
#endif
#if !defined(RPAC66)
		Set_Qcmbr(value);
#endif
		return 0;
	}
#endif
#if defined(RTCONFIG_WIFI_QCN5024_QCN5054) || defined(RTCONFIG_QCA_AXCHIP)
	else if (!strcmp(command, "Set_Ftm")) {
		Set_Ftm(value);
		return 0;
	}
#endif
#if defined(RTCONFIG_WIFI_QCA9990_QCA9990) || \
    defined(RTCONFIG_WIFI_QCA9994_QCA9994) || \
    defined(RTCONFIG_WIFI_QCN5024_QCN5054) || \
    defined(RTCONFIG_QCA_AXCHIP) || \
    defined(RTCONFIG_PCIE_QCA9888) || \
    defined(RTCONFIG_SOC_IPQ40XX)
	/* ATE Get_BData_2G / ATE Get_BData_5G
	 * To prevent whole ATE command strings exposed in rc binary,
	 * compare these commands in 3 steps instead.
	 */
	else if (!strncmp(command, "Get_", 4) && !strncmp(command + 4, "BData", 5) &&
		 *(command + 9) == '_') {
		Get_BData_X(command);
		return 0;
	}
#endif
#if defined(RTCONFIG_SOC_IPQ8074)
	else if (!strcmp(command, "Get_VoltUp")) {
		Get_VoltUp();
		return 0;
	}
	else if (!strcmp(command, "Set_VoltUp")) {
		Set_VoltUp(value);
		return 0;
	}
	else if (!strcmp(command, "Get_L2Ceiling")) {
		Get_L2Ceiling();
		return 0;
	}
	else if (!strcmp(command, "Set_L2Ceiling")) {
		Set_L2Ceiling(value);
		return 0;
	}
	else if (!strcmp(command, "Get_PwrCycleCnt")) {
		Get_PwrCycleCnt();
		return 0;
	}
	else if (!strcmp(command, "Set_PwrCycleCnt")) {
		Set_PwrCycleCnt(value);
		return 0;
	}
	else if (!strcmp(command, "Get_AvgUptime")) {
		Get_AvgUptime();
		return 0;
	}
	else if (!strcmp(command, "Set_AvgUptime")) {
		Set_AvgUptime(value);
		return 0;
	}
#endif
#if defined(RTCONFIG_WIFI_DRV_DISABLE) /* for IPQ40XX */
	else if (!strcmp(command, "Set_DisableWifiDrv")) {
		if (setDisableWifiDrv(value))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Get_DisableWifiDrv")) {
		if (getDisableWifiDrv())
		{
			puts("Invalid content!");
			return EINVAL;
		}
		return 0;
	}
#endif
#endif	/* RTCONFIG_QCA */
#if defined(RTCONFIG_TCODE)
	else if (!strcmp(command, "Set_TerritoryCode")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif

		if (setTerritoryCode(value) < 0)
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
#ifndef CONFIG_BCMWL5
		getTerritoryCode();
#endif
		return 0;
	}
	else if (!strcmp(command, "Get_TerritoryCode")) {
		getTerritoryCode();
		return 0;
	}
#if defined(CONFIG_BCMWL5) || defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_REALTEK)
	else if (!strcmp(command, "Set_PSK")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif

		if (setPSK(value) < 0)
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		puts(value);
		return 0;
	}
	else if (!strcmp(command, "Check_PSK")) {
		if (checkPSK(value) < 0)
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		puts("ATE_OK");
		return 0;
	}
#endif
#endif
#if defined(RTCONFIG_BCMARM) || defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
	else if (!strcmp(command, "Set_PASS")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif

		if (setPASS(value) < 0)
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		puts(value);
		return 0;
	}
	else if (!strcmp(command, "Check_PASS")) {
		if (checkPASS(value) < 0)
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		puts("ATE_OK");
		return 0;
	}
#endif
#ifdef RTCONFIG_QCA_PLC_UTILS
	else if (!strcmp(command, "Set_MacAddr_Plc")) {
		if (!setPLC_para(value, OFFSET_PLC_MAC))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Get_MacAddr_Plc")) {
		getPLC_para(OFFSET_PLC_MAC);
		return 0;
	}
	else if (!strcmp(command, "Set_NMK_Plc")) {
		if (!setPLC_para(value, OFFSET_PLC_NMK))
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Get_NMK_Plc")) {
		getPLC_para(OFFSET_PLC_NMK);
		return 0;
	}
	else if (!strcmp(command, "Get_PWD_Plc")) {
		if (!getPLC_PWD()) {
			puts("ATE_ERROR");
			return EINVAL;
		}
		return 0;
	}
#elif defined(RTCONFIG_QCA_PLC2)
	else if (!strcmp(command, "Get_MacAddr_Plc")) {
		puts(nvram_safe_get("plc_macaddr"));
		return 0;
	}
#endif
#ifdef RTCONFIG_DEFAULT_AP_MODE
#if defined(RTCONFIG_QCA) || defined(RTCONFIG_ALPINE) || defined(RTCONFIG_LANTIQ)
	else if (!strcmp(command, "Set_ForceDisableDHCP")) {
		FWrite("1", OFFSET_FORCE_DISABLE_DHCP, 1);
		puts("1");
		return 0;
	}
	else if (!strcmp(command, "Set_FreeDisableDHCP")) {
		char buf[2];
		FWrite("0", OFFSET_FORCE_DISABLE_DHCP, 1);
		if (FRead(buf, OFFSET_FORCE_DISABLE_DHCP, 1) < 0)
			puts("ATE_ERROR");
		else {
			buf[1] = '\0';
			puts(buf);
		}
		return 0;
	}
#endif
#endif
#ifdef RTCONFIG_AMAS
	else if (!strcmp(command, "Set_AB")) {
		int flag;
		if (value)
			flag = atoi(value);
		else
			flag = 1; /* old flag */
		if ((flag <= AB_FLAG_NONE) || (flag >= AB_FLAG_MAX))
			puts("ATE_ERROR");
		else
			set_amas_bdl(flag);
		return 0;
	}
	else if (!strcmp(command, "Unset_AB")) {
		unset_amas_bdl();
		return 0;
	}
	else if (!strcmp(command, "Get_AB")) {
		get_amas_bdl();
		return 0;
	}
	else if (!strcmp(command, "Set_ABK")) {
		if (!value || strlen(value)!=CFGSYNC_GROUPID_LEN || !is_valid_group_id(value))
			puts("ATE_ERROR");
		else
			set_amas_bdlkey(value);
		return 0;
	}
	else if (!strcmp(command, "Unset_ABK")) {
		unset_amas_bdlkey();
		return 0;
	}
	else if (!strcmp(command, "Get_ABK")) {
		get_amas_bdlkey();
		return 0;
	}
#endif

#ifdef RTCONFIG_PRESSURE_SENSOR
	else if (!strcmp(command, "Get_PressureOffset")) {
		get_pressure_offset();
		return 0;
	}
	else if (!strcmp(command, "Set_PressureOffset")) {
		char *endptr;
		errno = 0;

		if (!value || strlen(value) > PRESSURE_LEN){
			puts("ATE_ERROR");
			return EINVAL;
		}

		strtod(value, &endptr);
		if (*endptr != '\0' || errno != 0) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		} 

		if ( !set_pressure_offset(value) ){
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		
		return 0;
	}
	else if (!strcmp(command, "Get_CurPressure")) {
		get_cur_pressure();
		return 0;
	}	
#endif

#ifdef CONFIG_BCMWL5
	else if (!strcmp(command, "Get_SSID_2G")) {
#if defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO)
		getSSID(WLIF_2G);
#elif defined(GT10) || defined(RTAX9000)
		getSSID(2);
#elif defined(RPAX56) || defined(RPAX58) || defined(RPBE58)
		if(nvram_match("wl0_mode", "wet"))
			puts(nvram_safe_get("wl0.1_ssid"));
		else
			puts(nvram_safe_get("wl0_ssid"));
#else
		getSSID(0);
#endif
		return 0;
	}
	else if (!strcmp(command, "Get_SSID_5G")) {
#if defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO)
		getSSID(0);
#elif defined(BT10)
		getSSID(WLIF_5G1);
#elif defined(GT10) || defined(RTAX9000)
		getSSID(0);
#elif defined(RPAX56) || defined(RPAX58) || defined(RPBE58)
		if(nvram_match("wl1_mode", "wet"))
			puts(nvram_safe_get("wl1.1_ssid"));
		else
			puts(nvram_safe_get("wl1_ssid"));
#else
		getSSID(1);
#endif
		return 0;
	}
	else if (!strcmp(command, "Get_SSID_5G_2")) {
#if defined(GTAXE16000) || defined(GTBE98) || defined(BQ16)
		getSSID(1);
#elif defined(BT10)
		getSSID(WLIF_5G2);
#elif defined(GT10) || defined(RTAX9000)
		getSSID(1);
#else
		getSSID(2);
#endif
		return 0;
	}
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
	else if (!strcmp(command, "Get_SSID_6G")) {
#if defined(GTBE98_PRO) || defined(BQ16_PRO)
		getSSID(1);
#elif defined(BT10)
		getSSID(WLIF_6G);
#else
		getSSID(2);
#endif
		return 0;
	}
#endif
#ifdef RTCONFIG_HAS_6G_2
	else if (!strcmp(command, "Get_SSID_6G_2")) {
#if defined(GTBE98_PRO) || defined(BQ16_PRO)
		getSSID(2);
#endif
		return 0;
	}
#endif
#endif
#ifdef RTCONFIG_USB
	else if (!strcmp(command, "Get_Usb2p0_Port1_Infor") || !strcmp(command, "Get_Usb_Port1_Infor")) {
		Get_USB_Port_Info("1");
		return 0;
	}
	else if (!strcmp(command, "Get_Usb2p0_Port1_Folder") || !strcmp(command, "Get_Usb_Port1_Folder")) {
		Get_USB_Port_Folder("1");
		return 0;
	}
	else if (!strcmp(command, "Get_Usb2p0_Port2_Infor") || !strcmp(command, "Get_Usb_Port2_Infor")) {
		Get_USB_Port_Info("2");
		return 0;
	}
	else if (!strcmp(command, "Get_Usb2p0_Port2_Folder") || !strcmp(command, "Get_Usb_Port2_Folder")) {
		Get_USB_Port_Folder("2");
		return 0;
	}
	else if (!strcmp(command, "Get_Usb_Port1_DataRate")) {
		if (!Get_USB_Port_DataRate("1"))
			puts("ATE_ERROR");
		return 0;
	}
	else if (!strcmp(command, "Get_Usb_Port2_DataRate")) {
		if (!Get_USB_Port_DataRate("2"))
			puts("ATE_ERROR");
		return 0;
	}
#ifdef RTCONFIG_M2_SSD
	/* Because M.2 SSD is assigned to port 3 and BRT-AC828 doesn't have SD card.
	 * It's safe to call functions for SD card here.
	 */
	else if (!strcmp(command, "Get_M2Ssd_Infor")) {
		Get_SD_Card_Info();
		return 0;
	}
	else if (!strcmp(command, "Get_M2Ssd_Folder")) {
		Get_SD_Card_Folder();
		return 0;
	}
	else if (!strcmp(command, "Get_M2Ssd_DataRate")) {
		if (!Get_USB_Port_DataRate("3"))
			puts("ATE_ERROR");
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_SD_Infor")) {
		Get_SD_Card_Info();
		return 0;
	}
	else if (!strcmp(command, "Get_SD_Folder")) {
		Get_SD_Card_Folder();
		return 0;
	}
#ifdef RTCONFIG_USB_XHCI
	else if (!strcmp(command, "Get_Usb3p0_Port1_Infor")) {
		if (!Get_USB3_Port_Info("1"))
			puts("ATE_ERROR");
		return 0;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port2_Infor")) {
		if (!Get_USB3_Port_Info("2"))
			puts("ATE_ERROR");
		return 0;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port3_Infor")) {
		puts("ATE_ERROR"); //Need to implement
		return 0;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port1_Folder")) {
		if (!Get_USB3_Port_Folder("1"))
			puts("ATE_ERROR");
		return 0;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port2_Folder")) {
		if (!Get_USB3_Port_Folder("2"))
			puts("ATE_ERROR");
		return 0;
	}
 	else if (!strcmp(command, "Get_Usb3p0_Port3_Folder")) {
		puts("ATE_ERROR"); //Need to implement
		return EINVAL;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port1_DataRate")) {
		if (!Get_USB3_Port_DataRate("1"))
			puts("ATE_ERROR");
		return 0;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port2_DataRate")) {
		if (!Get_USB3_Port_DataRate("2"))
			puts("ATE_ERROR");
		return 0;
	}
	else if (!strcmp(command, "Get_Usb3p0_Port3_DataRate")) {
		puts("ATE_ERROR"); //Need to implement
		return EINVAL;
	}
#endif	/* RTCONFIG_USB_XHCI */
#ifdef RTCONFIG_ATEUSB3_FORCE
	else if (!strcmp(command, "Set_ForceUSB3")) {
		if (setForceU3(value) < 0)
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		getForceU3();
		return 0;
	}
	else if (!strcmp(command, "Get_ForceUSB3")) {
		getForceU3();
		return 0;
	}
#endif
#endif  /* RTCONFIG_QCA */
#ifdef RT4GAC55U
	else if(!strcmp(command, "Get_LteButtonStatus")) {
		puts(nvram_safe_get("btn_lte"));
		return 0;
	}
#endif
#ifdef RTCONFIG_INTERNAL_GOBI
	else if (!strcmp(command, "Get_GobiSimCard")) {
		char line[128];
		if (!Gobi_SimCardReady(Gobi_SimCard(MODEM_UNIT_FIRST, line, sizeof(line))))
		{
			puts("FAIL");
			return EINVAL;
		}
		puts("PASS");
	}
	else if (!strcmp(command, "Get_GobiIMEI")) {
		char line[128];
		const char *IMEI = Gobi_IMEI(MODEM_UNIT_FIRST, line, sizeof(line));
#if defined(RT4GAC86U)
		unsigned char IMEI_old[16];
#endif
		if (IMEI == NULL)
		{
			puts("FAIL");
			return EINVAL;
		}

#if defined(RT4GAC86U)
		memset(IMEI_old, 0, sizeof(IMEI_old));
		FRead(IMEI_old, OFFSET_GOBIIMEI, 15);

		printf("%s,%s\n", IMEI, (IMEI_old[0] != 0 && IMEI_old[0] != 255)?IMEI_old:"NONE");
#else
		printf("%s\n", IMEI);
#endif
	}
#if defined(RT4GAC86U)
	else if (!strcmp(command, "Set_GobiIMEI")) {
		char line[128];
		const char *IMEI = Gobi_IMEI(MODEM_UNIT_FIRST, line, sizeof(line));
		char IMEI_old[16];

		if(IMEI == NULL){
			puts("FAIL");
			return EINVAL;
		}

		if(value == NULL || strlen(value) <= 0){
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}

		memset(IMEI_old, 0, sizeof(IMEI_old));
		FRead(IMEI_old, OFFSET_GOBIIMEI, 15);

		if(!strcmp(value, "NONE")){ // Clean the backup IMEI
			if(setgobi_imei(value) < 0){
				puts("ATE_ERROR_INCORRECT_PARAMETER");
				return EINVAL;
			}
		}
		else{
			if(IMEI_old[0] == 0 || IMEI_old[0] == 255){ // Backup the original IMEI
				if(setgobi_imei(IMEI) < 0){
					puts("ATE_ERROR_INCORRECT_PARAMETER");
					return EINVAL;
				}
			}

			const char *ret = Gobi_setIMEI(MODEM_UNIT_FIRST, line, sizeof(line), value);
			if(ret == NULL){
				puts("ATE_ERROR_INCORRECT_PARAMETER");
				return EINVAL;
			}
		}

		puts(value);
	}
#endif
	else if (!strcmp(command, "Get_GobiConnectISP")) {
		char line[128];
		const char *ISP = Gobi_ConnectISP(MODEM_UNIT_FIRST, line, sizeof(line));
		if (ISP == NULL)
		{
			puts("FAIL");
			return EINVAL;
		}
		puts(ISP);
	}
	else if (!strcmp(command, "Get_GobiConnectStatus")) {
		const char *status = Gobi_ConnectStatus_Str(Gobi_ConnectStatus_Int(MODEM_UNIT_FIRST));
		if (status == NULL)
		{
			puts("FAIL");
			return EINVAL;
		}
		puts(status);
	}
	else if (!strcmp(command, "Get_GobiSignal_Percent")) {
		int percent = Gobi_SignalQuality_Percent(Gobi_SignalQuality_Int(MODEM_UNIT_FIRST));
		if (percent < 0)
		{
			puts("FAIL");
			return EINVAL;
		}
		printf("%d\n", percent);
	}
	else if (!strcmp(command, "Get_GobiSignal_dbm")) {
		int dbm = Gobi_SignalLevel_Int(MODEM_UNIT_FIRST);
		if (dbm >= 0)
		{
			puts("FAIL");
			return EINVAL;
		}
		printf("%d dBm\n", dbm);
	}
	else if (!strcmp(command, "Get_GobiVersion")) {
		char line[128];
		if (Gobi_FwVersion(MODEM_UNIT_FIRST, line, sizeof(line)) == NULL)
		{
			puts("FAIL");
			return EINVAL;
		}
		printf("%s\n", line);
	}
	else if (!strcmp(command, "Get_GobiQcnVersion")) {
		char line[128];
		if (Gobi_QcnVersion(MODEM_UNIT_FIRST, line, sizeof(line)) == NULL)
		{
			puts("FAIL");
			return EINVAL;
		}
		printf("%s\n", line);
	}
	else if (!strcmp(command, "Set_GobiBand")) {
		char line[128];
		if (Gobi_SelectBand(MODEM_UNIT_FIRST, value, line, sizeof(line)) == NULL)
		{
			puts("FAIL");
			return EINVAL;
		}
		printf("FINISH\n");
	}
	else if (!strcmp(command, "Get_GobiBand")) {
		char line[128];
		char *band;
		if ((band = Gobi_BandChannel(MODEM_UNIT_FIRST, line, sizeof(line))) == NULL)
		{
			puts("FAIL");
			return EINVAL;
		}
		cprintf("line(%p) band(%p)\n", line, band);
		printf("%s\n", band);
	}
#endif	/* RTCONFIG_INTERNAL_GOBI */
#ifdef RTCONFIG_REALTEK
	else if (!strcmp(command, "Set_Usb3p0_Enable")) {
		setUsb3p0Enable();
		return 0;
	}
	else if (!strcmp(command, "Set_Usb3p0_Disable")) {
		setUsb3p0Disable();
		return 0;
	}
#endif
#ifdef RTCONFIG_OUTFOX
	else if (!strcmp(command, "Set_OutfoxCode")) {
#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
		if (!chk_envrams_proc())
			return EINVAL;
#endif
		if (setOutfoxCode(value) < 0)
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}

		return 0;
	}
	else if (!strcmp(command, "Get_OutfoxCode")) {
		getOutfoxCode();
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_FwUpgradeState")) {
		ate_get_fw_upgrade_state();
		return 0;
	}
	else if (!strcmp(command, "Get_LabelMacAddr")) {
		puts(get_label_mac());
		return 0;
	}
	else if (!strcmp(command, "Set_IpAddr_Lan")) {
		if(value == NULL || strlen(value) <= 0){
			printf("ATE_ERROR_INCORRECT_PARAMETER\n");
			return EINVAL;
		}

		set_IpAddr_Lan(value);
	}
	else if (!strcmp(command, "Get_IpAddr_Lan")) {
		get_IpAddr_Lan();
	}
	else if (!strcmp(command, "Set_MRFLAG")) {
		if(value == NULL || strlen(value) <= 0){
			printf("ATE_ERROR_INCORRECT_PARAMETER\n");
			return EINVAL;
		}

		set_MRFLAG(value);
	}
	else if (!strcmp(command, "Get_MRFLAG")) {
		get_MRFLAG();
	}
	else if (!strcmp(command, "Get_Default")) {
		char *p = NULL;

		if (value == NULL)
			return 0;

		if (!(p = nvram_default_get(value)))
			puts("NULL");
		else
			puts(p);

		return 0;
	}
	else if (!strcmp(command, "Get_txBurst")) {
#ifdef RTCONFIG_LANTIQ
				update_txburst_status();
#endif
		if (nvram_match("wl1_frameburst", "on"))
#ifdef RTCONFIG_RALINK
		if(nvram_match("reg_spec", "CE"))
			puts("0");
		else
			puts("1");
#else
		puts("1");
#endif

		else if (nvram_match("wl1_frameburst", "off"))
			puts("0");
		return 0;
        }
#if defined(CONFIG_BCMWL5) || defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK) || defined(RPAC92) 
	else if (!strcmp(command, "Get_HwId")) {
		get_HwId();
		return 0;
	}
	else if (!strcmp(command, "Get_HwVersion")) {
		get_HwVersion();
		return 0;
	}
	else if (!strcmp(command, "Get_HwBom")) {
		get_HwBom();
		return 0;
	}
	else if (!strcmp(command, "Get_DateCode")) {
		get_DateCode();
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_RDG")) {
#ifdef RTCONFIG_RALINK
		if(nvram_match("reg_spec", "CE"))
			puts("0");
		else
			puts(nvram_safe_get("wl_HT_RDG"));
#else
		puts("NA");
#endif
		return 0;
	}
#ifdef RTCONFIG_ISP_CUSTOMIZE
	else if (!strcmp(command, "Get_MD5")) {
		int ret;
		char md5[36];
		int md5_len = sizeof(md5);
		if (value == NULL || strlen(value) <= 0) {
			puts("ATE_ERROR");
			return EINVAL;
		}
		if ((ret = get_file_hash(value, md5, &md5_len)) == 0) {
			puts(md5);
		}
		else {
			if (ret == -2)
				puts("ATE_ERROR_FILE_NOT_FOUND");
			else
				puts("ATE_ERROR");
		}
		return 0;
	}
	else if (!strcmp(command, "Get_IspMD5")) {
		int ret;
		char md5[36];
		int md5_len = sizeof(md5);
		if ((ret = get_package_hash(md5, &md5_len)) == 0) {
			puts(md5);
		}
		else {
			if (ret == -2)
				puts("ATE_ERROR_FILE_NOT_FOUND");
			else
				puts("ATE_ERROR");
		}
		return 0;
	}
	else if (!strcmp(command, "Get_IspVersion")) {
		int ret;
		char ver[100];
		int ver_len = sizeof(ver);
		if ((ret = get_package_version(ver, &ver_len)) == 0) {
			puts(ver);
		}
		else {
			if (ret == -2)
				puts("ATE_ERROR_FILE_NOT_FOUND");
			else
				puts("ATE_ERROR");
		}
		return 0;
	}
	else if (!strcmp(command, "Get_IspVerification")) {
		int ret;
		if ((ret = verify_package(NULL, NULL)) == 1)
			puts("ATE_SUCCESS");
		else
			puts("ATE_ERROR");
		return 0;
	}
	else if (!strcmp(command, "Get_IspVerificationDetail")) {
		int ret;
		char result[1024];
		int result_len = sizeof(result);
		if ((ret = verify_package(result, &result_len)) >= 0 && result_len)
			puts(result);
		else
			puts("ATE_ERROR");
		return 0;
	}
#endif
#ifdef RTCONFIG_DSL_HOST
	else if (!strcmp(command, "Set_Annex")) {
		if(set_Annex(value, value2)) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		puts("1");
		return 0;
	}
	else if (!strcmp(command, "Get_Annex")) {
		if(get_Annex()) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
#endif
#if defined(CONFIG_BCMWL5) || defined(RTCONFIG_COBRAND)
	else if (!strcmp(command, "Set_CoBrand")) {
		int n ;
		if (value && (n=atoi(value)) && ((n >= 0) && (n <= 100))) {
			if(set_cb(n) < 0)
				puts("ATE_ERROR_INCORRECT_PARAMETER");
		} else
			puts("ATE_ERROR");
		return 0;
	}
	else if (!strcmp(command, "Unset_CoBrand")) {
		unset_cb();
		return 0;
	}
	else if (!strcmp(command, "Get_CoBrand")) {
		get_cb();
		return 0;
	}
#endif
#if defined(RTCONFIG_CSR8811)
	else if (!strcmp(command, "Get_MacAddr_BT")) {
		char mac[6];
		getMAC_BT(mac, sizeof(mac));
		return 0;
	}
	else if (!strcmp(command, "Get_Cal_BT")) {
		unsigned char cal;
		getCal_BT(&cal);
		return 0;
	}
	else if (!strcmp(command, "Set_MacAddr_BT")) {
		const char *p = (char *) value;
		char UpperMac[20] = {0};
		int i;
		for (i = 0; p[i]; ++i)
		{
			UpperMac[i] = toupper(p[i]);
		}
		if ( !setMAC_BT(UpperMac) )
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if(!strcmp(command, "Set_Cal_BT"))
	{
		if ( !setCal_BT(value) )
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if(!strcmp(command, "Set_StartBTDiag"))
	{
		extern void setStartBTDiag(void);
		setStartBTDiag();
		puts("1");
		return 0;
	}
#endif
	else if (!strcmp(command, "Get_ModelDesc")) {
		char ispctrl_desc[128];
		snprintf(ispctrl_desc, sizeof(ispctrl_desc), "%s", nvram_safe_get("ispctrl_desc"));
		if (strlen(ispctrl_desc))
			puts(ispctrl_desc);
		else {
			if (rt_modeldesc && strlen(rt_modeldesc))
				puts(rt_modeldesc);
			else
				puts("NONE");
		}
		return 0;
	}
#if defined(RTCONFIG_ASUSCTRL) && (defined(RTCONFIG_QCA) || defined(RTCONFIG_RALINK))
	else if (!strcmp(command, "Set_asusctrl"))
	{
		if (!IS_ATE_FACTORY_MODE())
			return -1;

		if (asus_ctrl_write(value)<0)
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Get_asusctrl"))
	{
		asus_ctrl_get();
		return 0;
	}
	else if (!strcmp(command, "Set_asusctrl_sku"))
	{
		if (!IS_ATE_FACTORY_MODE())
			return -1;

		if (asus_ctrl_sku_write(value)<0)
		{
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Get_asusctrl_sku"))
	{
		asus_ctrl_sku_get();
		return 0;
	}
#endif
#if defined(RTCONFIG_CC3220)
	else if (!strcmp(command, "Set_TiPowerIndex")) {
		if (setTiPowerIndex(value)) {
			puts("ATE_ERROR_INCORRECT_PARAMETER");
			return EINVAL;
		}
		return 0;
	}
	else if (!strcmp(command, "Get_TiPowerIndex"))
	{
		char power[10];
		if (getTiPowerIndex(power, sizeof(power))) {
			return -1;
		}
		puts(power);
		return 0;
	}
	else if (!strcmp(command, "Get_TiFWVersion"))
	{
		char fwver[16];
		if (ti_get_fwver(fwver, sizeof(fwver))) {
			puts("Not ASUS firmware");
			return -1;
		}
		puts(fwver);
		return 0;
	}
	else if (!strcmp(command, "Get_TiMacAddr_2G"))
	{
		char mac[18];
		if (ti_get_mac_2g(mac, sizeof(mac))) {
			return -1;
		}
		puts(mac);
		return 0;
	}
#endif
	else
	{
		puts("ATE_UNSUPPORT");
		return EINVAL;
	}

	return 0;
}

int ate_dev_status(void)
{
	int ret = 1, ate_wl_band = 1;
	char wl_dev_name[4 * IFNAMSIZ], dev_chk_buf[64], word[256], *next;
	int len, remain;
	char result;
	char *p;
#ifdef RTCONFIG_BT_CONN
	int have_bt_device = 1;
#endif
#if defined(RTCONFIG_BCMWL6) && (defined(RTCONFIG_HAS_5G_2) || defined(RTCONFIG_HAS_6G_2) || defined(RTCONFIG_HAS_6G))
	int count_5g = 0;
	int count_6g = 0;
#ifdef RTCONFIG_QUADBAND
#ifdef defined(RTCONFIG_HAS_6G_2)
	char wlif_name[4][4] = {"2G", "5G", "6G", "6G2"};
#else
	char wlif_name[4][4] = {"2G", "5G", "5G2", "6G"};
#endif
#else // !RTCONFIG_QUADBAND
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
#if defined(BT10)
	char wlif_name[3][4] = {"6G", "5G", "2G"};
#else
	char wlif_name[3][4] = {"2G", "5G", "6G"};
#endif
#else
	char wlif_name[3][4] = {"2G", "5G", "5G2"};
#endif
#endif //RTCONFIG_QUADBAND
#endif

	memset(dev_chk_buf, 0, sizeof(dev_chk_buf));
	snprintf(wl_dev_name, sizeof(wl_dev_name), nvram_safe_get("wl_ifnames"));
	if(switch_exist())
		snprintf(dev_chk_buf, sizeof(dev_chk_buf), "switch=O");
	else{
		snprintf(dev_chk_buf, sizeof(dev_chk_buf), "switch=X");
#ifdef CONFIG_BCMWL5	//broadcom platform need to shift the interface name
		snprintf(wl_dev_name, sizeof(wl_dev_name), "eth0 eth1");
#endif
		ret = 0;
	}

	len = strlen(dev_chk_buf);
	p = dev_chk_buf + len;
	remain = sizeof(dev_chk_buf) - len;

	foreach(word, wl_dev_name, next){
		if (absent_band(ate_wl_band - 1)) {
			ate_wl_band++;
			continue;
		}
#if defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(BT10) || defined(GT10) || defined(RTAX9000) || defined(GSBE18000) || defined(GSBE12000) || defined(GS7_PRO) || defined(GT7)
		// override ate_wl_band since wifi radio sequence is not habitual
		if (wl_get_band(word) == WLC_BAND_2G)
			ate_wl_band = 1;
		else 
			ate_wl_band = 2;
#endif
#if defined(EBG15) || defined(EBG19)
		result = 'O';
#else
		if(wl_exist(word, ate_wl_band)){
			result = 'O';
		}
		else{
			result = 'X';
			ret = 0;
		}
#endif
#if defined(RTCONFIG_BCMWL6) && (defined(RTCONFIG_HAS_5G_2) || defined(RTCONFIG_HAS_6G_2) || defined(RTCONFIG_HAS_6G))
		switch(wl_get_band(word)) {
			case WLC_BAND_2G:
			    	len = snprintf(p, remain, ",2G=%c", result);
		    		break;
			case WLC_BAND_5G:
				if(!count_5g) {
					len = snprintf(p, remain, ",5G=%c", result);
					count_5g++;
				}
				else
					len = snprintf(p, remain, ",5G2=%c", result);
				break;
#if defined(RTCONFIG_WIFI6E) || defined(RTCONFIG_WIFI7)
		    	case WLC_BAND_6G:
				if(!count_6g) {
					len = snprintf(p, remain, ",6G=%c", result);
					count_6g++;
				}
				else
					len = snprintf(p, remain, ",6G2=%c", result);
				break;
#endif
			default:
				len = snprintf(p, remain, ",%s=%c", wlif_name[ate_wl_band-1], result);
				break;
		}
#else
		if(ate_wl_band == 1)
			len = snprintf(p, remain, ",2G=%c", result);
		else if(ate_wl_band == 2)
			len = snprintf(p, remain, ",5G=%c", result);
		else if (ate_wl_band == 3)
#if !defined(RTCONFIG_WIFI6E) && !defined(RTCONFIG_WIFI7)
			len = snprintf(p, remain, ",5G2=%c", result);
#else
			len = snprintf(p, remain, ",6G=%c", result);
#endif
		else
			len = snprintf(p, remain, ",60G=%c", result);
#endif
		p += len;
		remain -= len;
		ate_wl_band++;
	}

#ifndef RTCONFIG_ALPINE
#ifdef RTCONFIG_BT_CONN
	{
#define RETRY_MAX 100
		int retry;
#if defined(RTAX56_XD4) || defined(PLAX56_XP4)
		if(nvram_match("HwId", "A") || nvram_match("HwId", "C")){
			have_bt_device = 1;
		}else{
			have_bt_device = 0;
		}
#endif
#if defined(RTAX56_XD4)
		if((nvram_match("HwId", "A") && nvram_get_int("BLE_BT") == 99) ||
			(nvram_match("HwId", "C") && nvram_get_int("BLE_BT") == 99)){
			/* Master without BT */
				have_bt_device = 0;
		}
#endif
#if defined(RTCONFIG_LANTIQ) || defined(RTAX95Q) || defined(XT8PRO) || defined(BT12) || defined(BT10) || defined(BQ16) || defined(BQ16_PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(RTAX82_XD6) || defined(RTAX82_XD6S) || defined(ET12) || defined(XT12) || defined(XD6_V2) || defined(XC5) || defined(EBG15) || defined(EBG19)
		if(have_bt_device == 1){
			system("killall bluetoothd");
			system("hciconfig hci0 down");
			system("hciconfig hci0 reset");
			system("hciconfig hci0 up");
			system("hciconfig hci0 leadv 0");
			system("bluetoothd &");
		}
#endif
		if(have_bt_device == 1){
			for(retry = 0; retry < RETRY_MAX; retry++){
				extern int check_bluetooth_device(const char *bt_dev);
				if(check_bluetooth_device("hci0") == 0)
					break;
				sleep(1);
			}
			if(retry < RETRY_MAX)
			{
				result = 'O';
			}
			else
			{
				result = 'X';
				ret = 0;
			}
		}
	}
	if(have_bt_device == 1){
		len = snprintf(p, remain, ",hci0=%c", result);
		p += len;
		remain -= len;
	}
#endif
#endif

#if defined(RTCONFIG_EXTPHY_BCM84880)
#if defined(RTAX86U)
	if(strcmp(get_productid(), "RT-AX86S"))
#endif
	{
	/* Get extend 2.5G phy bcm84880 status */
		if(
#if defined(ET12) || defined(XT12)
			ethctl_get_link_status("eth3") == -1
#elif defined(GTAX6000) || defined(GTAX11000_PRO) || defined(RTAX86U_PRO) || defined(RTAX88U_PRO) || defined(RTBE96U)
			ethctl_get_link_status("eth5") == -1
#elif defined(GTAXE16000)
			ethctl_get_link_status("eth5") == -1 || ethctl_get_link_status("eth6") == -1
#elif defined(GTBE98) || defined(GTBE98_PRO) || defined(GTBE96) || defined(GTBE19000) || defined(GTBE19000AI) || defined(GTBE96_AI)
			(is_rtl8372_boardid() && ethctl_get_link_status("eth3") == -1) ||
			(!is_rtl8372_boardid() && ethctl_get_link_status("eth6") == -1)
#elif defined(TUFAX3000_V2) || defined(RTAXE7800) || defined(GT10) || defined(RTBE58U) || defined(TUFBE3600) || defined(RTBE58U_V2) || defined(TUFBE3600_V2) || defined(RTBE55) || defined(RTBE92U) || defined(RTBE95U) || defined(RTBE58U_PRO)
			ethctl_get_link_status("eth0") == -1
#elif defined(RTAX9000)
			ethctl_get_link_status("eth0") == -1 || ethctl_get_link_status("eth5") == -1
#elif defined(RTBE88U)
			ethctl_get_link_status("eth1") == -1 || ethctl_get_link_status("eth2") == -1 || ethctl_get_link_status("eth3") == -1 || ethctl_get_link_status("eth4") == -1 ||
			ethctl_get_link_status("eth9") == -1
#elif defined(RTBE86U)
			ethctl_get_link_status("eth1") == -1 || ethctl_get_link_status("eth2") == -1 || ethctl_get_link_status("eth3") == -1 || ethctl_get_link_status("eth4") == -1
#elif defined(RTBE58_GO)
			ethctl_get_link_status("eth1") == -1
#else // RT-AX86U
			ethctl_get_link_status("eth5") == -1 || (nvram_get_int("ext_phy_model") == EXT_PHY_BCM54991 && ethctl_phy_op("ext", EXTPHY_ADDR, 0x1e4037, 0, 0) == -1)
#endif
		)
			result = 'X';
		else
			result = 'O';
		len = snprintf(p, remain, ",EXTPHY=%c", result);
		p += len;
		remain -= len;
	}
#endif
#if defined(GTBE19000AI) || defined(GTBE96_AI)
	if (ethctl_get_link_status("eth.ai") != 1)
		result = 'X';
	else
		result = 'O';
	len = snprintf(p, remain, ",AIBOARD=%c", result);
	p += len;
	remain -= len;
#endif
#ifdef RTCONFIG_CC3220
	result = (ti_check_alive()) ? 'X' : 'O';
	snprintf(dev_chk_buf + strlen(dev_chk_buf), sizeof(dev_chk_buf) - strlen(dev_chk_buf), ",TI=%c", result);
	len = strlen(dev_chk_buf);
	p = dev_chk_buf + len;
	remain = sizeof(dev_chk_buf) - len;
#endif
	nvram_set("Ate_dev_status", dev_chk_buf);

	return ret;
}

#if defined(RTCONFIG_CFEZ) && defined(RTCONFIG_BCMARM)
int chk_envrams_proc(void)
{
	if (!pids("envrams"))
	{
		puts("ATE_ERROR_NOT_ALLOWED");
		return 0;
	}

	return 1;
}

int start_envrams(void) {
	int ret;

	ret = 0;
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
	if (!pids("envrams")){
		dbg("[%s][%d] start envrams\n", __func__, __LINE__);
#if defined(XT8PRO) || defined(BT12) || defined(BT10) || defined(BQ16) || defined(BQ16_PRO) || defined(BM68) || defined(XT8_V2) || defined(ET8PRO) || defined(ET8_V2) || defined(XD4PRO) || defined(GTAXE16000) || defined(GTBE98) || defined(GTBE98_PRO) || defined(GTAX11000_PRO) || \
	defined(ET12) || defined(XT12) || defined(RTAX88U_PRO) || defined(EBG19) || defined(EBG15) || defined(RTBE96U)  || defined(XC5) || defined(EBA63) || defined(GTBE96) || defined(RTBE88U) || defined(RTCONFIG_HND_ROUTER_BE_4916) && defined(RTCONFIG_USB)
		dbg("[%s][%d] start envrams\n", __func__, __LINE__);
		system("mkdir -p /tmp/mnt/defaults");
		system("umount /tmp/mnt/defaults");
#if defined (RTCONFIG_FLASH_TYPE_EMMC)
		system("mount -t ext4 /dev/defaults /tmp/mnt/defaults");
#else
		system("mount -t ubifs ubi:defaults /tmp/mnt/defaults");
#endif
		system("/usr/sbin/envrams >/dev/null");
		sleep(1);
		system("sleep 1; umount /mnt/defaults; sleep 1");
#else
		system("/usr/sbin/envrams &> /dev/null");
#endif
		ret = 1;
	}
#else
	if (!pids("envrams")){
		dbg("[%s][%d] start envrams\n", __func__, __LINE__);
		system("/usr/sbin/envrams &> /dev/null");
		ret = 1;
	}
#endif
	return ret;
}

void stop_envrams(void) {
	killall_tk("envrams");
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
#ifdef RTCONFIG_USB
	system("umount /tmp/mnt/defaults &> /dev/null");
#else
	system("umount /mnt/defaults &> /dev/null");
#endif
#endif
}
#endif

int ate_run_arpstrom(void) {

	int ate_arpstorm = 0;
	ate_arpstorm = nvram_get_int("ate_arpstorm");
	while(ate_arpstorm) {
		ate_arpstorm--;
		doSystem("arpstorm &");
		sleep(1);
	}

	return 1;
}

int ate_get_fw_upgrade_state(void) {

	FILE *fp;
	char buf[64];

#ifdef CONFIG_BCMWL5
		if (!factory_debug() && !nvram_match(ATE_UPGRADE_MODE_STR(), "1"))
#else
		if (!IS_ATE_FACTORY_MODE() && !nvram_match(ATE_UPGRADE_MODE_STR(), "1"))
#endif
		{
			puts("ATEMODE ONLY");
			return 0;
		}
#if defined(RTCONFIG_HND_ROUTER_AX_6756) || defined(RTCONFIG_HND_ROUTER_BE_4916)
		switch(nvram_get_int("ate_upgrade_state")){
		    case _ATE_FW_START:
			puts("START"); return 0;
		    case _ATE_FW_WRITING:
			puts("WRITING"); return 0;
		    case _ATE_FW_UNEXPECT_ERROR:
			puts("UNEXPECT_ERROR"); return 0;
		    case _ATE_FW_FAILURE:
			puts("FAILURE"); return 0;
		    case _ATE_FW_COMPLETE:
			puts("COMPLETE"); return 1;
		    default:
			puts("UNKNOWN STATE"); return 0;
		}
#else
		if (!(fp=fopen("/tmp/ate_upgrade_state", "r"))) {
			puts("ERROR TO CHECK STATE");
			return 0;
		}
		fgets(buf, sizeof(buf), fp);
		fclose(fp);

		if(strstr(buf, "Upgarde Complete")) {
			puts("COMPLETE");
			return 1;
		}
		else if(strstr(buf, "stop_upgarde_ate"))
			puts("UPGRADING(1)");
		else if(strstr(buf, "start_upgarde_ate"))
			puts("UPGRADING(2)");
		else
			puts("UNKNOWN STATE");
#endif
		return 0;
}

/* For init default password and need to be built in prebuild */
/* copy password from memory to nvram */
int init_pass_nvram(void)
{
#ifdef RTCONFIG_NVRAM_ENCRYPT
	char dec_passwd[128]={0};
#endif
#if defined(RTCONFIG_BCMARM)
	if (!nvram_get_int("x_Setting")) {
		char *forget_it = cfe_nvram_safe_get_raw("forget_it");
#ifdef RTCONFIG_NVRAM_ENCRYPT
		if(pw_dec(forget_it, dec_passwd, sizeof(dec_passwd), 0))
			forget_it = dec_passwd;
#endif
		nvram_set("forget_it", forget_it);
	}
#elif defined(RTCONFIG_RALINK) || defined(RTCONFIG_QCA)
	/* PASS */
	{
		char pass[PASS_LEN + 1];
		memset(pass, 0, sizeof(pass));
		if (FRead(pass, PASS_OFFSET, PASS_LEN) < 0) {
			_dprintf("READ ASUS PASS: Out of scope\n");
			nvram_set("forget_it", "");
		 } else {
			int len = strlen(pass);
			int i;
			if (pass[0] == 0xff)
				nvram_set("forget_it", "");
			else
			{
				for(i = 0; i < PASS_LEN && pass[i] != '\0'; i++) {
					if ((unsigned char)pass[i] == 0xff)
					{
						pass[i] = '\0';
						break;
					}
				}
#if defined(RTCONFIG_NVRAM_ENCRYPT) && defined(RTCONFIG_PASS_V2)
				if(pw_dec(pass, dec_passwd, sizeof(dec_passwd), 0))
					strlcpy(pass, dec_passwd, sizeof(pass));
#endif
				nvram_set("forget_it", pass);
			}
		}
	}
#endif
	if (strcmp(nvram_safe_get("forget_it"), "") && !nvram_contains_word("rc_support", "defpass"))
		add_rc_support("defpass");

#ifdef RTCONFIG_TRUSTZONE
	if (!atee_check_admin_def_pw_exist() && !nvram_is_empty("forget_it")) {
		atee_set_admin_def_pw(nvram_safe_get("forget_it"));
		sync();
	}
#endif

	return 0;
}
