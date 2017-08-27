/*
    superio: Handle special I/O operations needed by most Super-I/O chips

    Copyright (C) 2005-2008  Jean Delvare <jdelvare@suse.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
*/

#ifndef _SUPERIO_H
#define _SUPERIO_H

#define SUPERIO_MAX_KEY	8

int superio_parse_key(unsigned char *key, const char *s);
void superio_write_key(int addrreg, unsigned char *key);
void superio_reset(int addrreg, int datareg);

#endif /* _SUPERIO_H */
