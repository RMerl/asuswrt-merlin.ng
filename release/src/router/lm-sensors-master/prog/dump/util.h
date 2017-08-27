/*
    util - helper functions
    Copyright (C) 2006-2011 Jean Delvare <jdelvare@suse.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
*/

#ifndef _UTIL_H
#define _UTIL_H

extern int user_ack(int def);
extern unsigned long inx(int addr, int width);
extern void outx(unsigned long value, int addr, int width);

#endif /* _UTIL_H */
