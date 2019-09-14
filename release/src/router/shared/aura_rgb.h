#ifndef __AURA_SYNC_H__
#define __AURA_SYNC_H__

#if 0
#include <shared.h>

#define _FILE			"/tmp/ASS_DEBUG"

#define ASS_DEBUG(fmt, args...) \
	if(f_exists(MNT_FILE)) { \
		_dprintf("[AURA_SYNC][%s]"fmt, __FUNCTION__, __LINE__, ## args); \
	}

#define AURA_SW_REQ	0x00
#define AURA_SW_SET	0x01
#define ROUTER_AURA_SET 0x02

//--------------------------------------------------------------------
//Structure, Group base address define
//--------------------------------------------------------------------
enum
{
	RGBLED_USER_RED			= 0x10,
	RGBLED_USER_BLUE		= 0x11,
	RGBLED_USER_GREEN		= 0x12,
	RGBLED_CONTROL_STOP		= 0x20,
	RGBLED_MODE			= 0x21,
	RGBLED_SPEED			= 0x22,
	RGBLED_DIRECTION		= 0x23,
	RGBLED_LED_OFF			= 0x24,
	RGBLED_SYSTEM_OFF_EFF		= 0x25,
	RGBLED_SYSTEM_OFF_SPEED		= 0x26,
	RGBLED_SYSTEM_OFF_DIRECTION	= 0x27,
	RGBLED_LED_APPLY		= 0x2f,
};
#endif
#define AURA_CAP	    "014f"
#define EXTRA_RGB_MAX_GROUP 5
#define RGB_MAX_GROUP       1

#define AURA_SW_REQ	0x00
#define AURA_SW_SET	0x01
#define ROUTER_AURA_SET 0x02

typedef struct rgb_led_status_t {
#if 0
	char 	red[EXTRA_RGB_MAX_GROUP];
	char 	blue[EXTRA_RGB_MAX_GROUP];
	char	green[EXTRA_RGB_MAX_GROUP];
	char 	mode[EXTRA_RGB_MAX_GROUP];
	char 	speed[EXTRA_RGB_MAX_GROUP];
	char 	direction[EXTRA_RGB_MAX_GROUP];
	char	mode_support_h[EXTRA_RGB_MAX_GROUP];
	char	mode_support_l[EXTRA_RGB_MAX_GROUP];
	char	cur_frame[EXTRA_RGB_MAX_GROUP];
#endif
	unsigned char	red, green, blue;
	unsigned char	mode;
	signed char	speed; /* -2, -1, 0, 1, 2 */
	unsigned char	direction;
	unsigned char	mode_support_h, mode_support_l;
	unsigned char	cur_frame;
} RGB_LED_STATUS_T;

extern int aura_rgb_led(int type, RGB_LED_STATUS_T *status, int group, int from_server);
extern int __nv_to_rgb(char *aurargb_val, RGB_LED_STATUS_T *out_rgb);
extern int nv_to_rgb(char *nv_name, RGB_LED_STATUS_T *out_rgb);
extern int switch_rgb_mode(char *nv_name, RGB_LED_STATUS_T *out_rgb, int led_onoff);
#if defined(RTCONFIG_RGBLED) && defined(GTAC2900)
extern int send_aura_event(const char *event_name);
extern int check_aura_rgb_reg(void);
#endif
#endif	/* ! __AURA_SYNC_H__ */
