#ifndef __COLOR_H__
#define __COLOR_H__ 1

enum color_attr {
	COLOR_IFNAME,
	COLOR_MAC,
	COLOR_INET,
	COLOR_INET6,
	COLOR_OPERSTATE_UP,
	COLOR_OPERSTATE_DOWN
};

void enable_color(void);
int color_fprintf(FILE *fp, enum color_attr attr, const char *fmt, ...);

#endif
