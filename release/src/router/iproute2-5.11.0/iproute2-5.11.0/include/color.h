/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __COLOR_H__
#define __COLOR_H__ 1

#include <stdbool.h>

enum color_attr {
	COLOR_IFNAME,
	COLOR_MAC,
	COLOR_INET,
	COLOR_INET6,
	COLOR_OPERSTATE_UP,
	COLOR_OPERSTATE_DOWN,
	COLOR_NONE
};

enum color_opt {
	COLOR_OPT_NEVER = 0,
	COLOR_OPT_AUTO = 1,
	COLOR_OPT_ALWAYS = 2
};

bool check_enable_color(int color, int json);
bool matches_color(const char *arg, int *val);
int color_fprintf(FILE *fp, enum color_attr attr, const char *fmt, ...);
enum color_attr ifa_family_color(__u8 ifa_family);
enum color_attr oper_state_color(__u8 state);

#endif
