#ifndef WIDGET_H_INCLUDED
#define WIDGET_H_INCLUDED

#include <panel.h>

#define WIDGET_BORDER		0x1
#define WIDGET_SUBWINDOW	0x2
#define WIDGET_CURSOR_VISIBLE	0x4

#define SCREEN_CENTER	-1

struct widget {
	WINDOW *window;
	WINDOW *subwindow; /* optional: contents without border */
	PANEL *panel;
	int cursor_visibility;

	void (*handle_key)(int key);
	void (*window_size_changed)(void);
	void (*close)(void);
};

extern int screen_lines;
extern int screen_cols;

void widget_init(struct widget *widget,
		 int lines_, int cols, int y, int x,
		 chtype bkgd, unsigned int flags);
void widget_free(struct widget *widget);
struct widget *get_active_widget(void);
void window_size_changed(void);

#endif
