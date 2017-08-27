/*
 *  Control Symbols
 *  Copyright (c) 2001 by Jaroslav Kysela <perex@perex.cz>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#ifndef PIC

extern const char *_snd_module_control_hw;
extern const char *_snd_module_control_shm;
extern const char *_snd_module_control_ext;

static const char **snd_control_open_objects[] = {
	&_snd_module_control_hw,
#include "ctl_symbols_list.c"
};
	
void *snd_control_open_symbols(void)
{
	return snd_control_open_objects;
}

#endif /* !PIC */
