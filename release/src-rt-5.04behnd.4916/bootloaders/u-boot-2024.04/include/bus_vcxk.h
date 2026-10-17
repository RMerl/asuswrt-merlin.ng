/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2005-2009
 * Jens Scharsig @ BuS Elektronik GmbH & Co. KG, <esw@bus-elektronik.de>
 */

#ifndef __BUS_VCXK_H_
#define __BUS_VCXK_H_

extern int vcxk_init(unsigned long width, unsigned long height);
extern void vcxk_setpixel(int x, int y, unsigned long color);
extern int vcxk_acknowledge_wait(void);
extern int vcxk_request(void);
extern void vcxk_loadimage(ulong source);
extern int vcxk_display_bitmap(ulong addr, int x, int y);
extern void vcxk_setbrightness(unsigned int side, short brightness);
extern int video_display_bitmap(ulong addr, int x, int y);

#endif
