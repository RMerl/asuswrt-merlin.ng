#ifndef MAINLOOP_H_INCLUDED
#define MAINLOOP_H_INCLUDED

#include CURSESINC

void initialize_curses(bool use_color);
void mainloop(void);
void shutdown(void);

#endif
