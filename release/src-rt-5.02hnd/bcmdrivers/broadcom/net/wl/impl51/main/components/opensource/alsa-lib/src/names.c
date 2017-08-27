/**
 * \file names.c
 * \ingroup Configuration
 * \brief Configuration helper functions - device names
 * \author Jaroslav Kysela <perex@perex.cz>
 * \date 2005
 *
 * Provide a list of device names for applications.
 *
 * See the \ref conf page for more details.
 */
/*
 *  Configuration helper functions - device names
 *  Copyright (c) 2005 by Jaroslav Kysela <perex@perex.cz>
 *
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

#include <stdarg.h>
#include <limits.h>
#include <sys/stat.h>
#include "local.h"

/** 
 * \brief Give a list of device names and associated comments for selected interface
 * \param iface a string identifying interface ("pcm", "ctl", "seq", "rawmidi")
 * \param list result - a pointer to list
 * \return A non-negative value if successful, otherwise a negative error code.
 * \deprecated Since 1.0.14
 *
 * The global configuration files are specified in the environment variable
 * \c ALSA_NAMES_FILE.
 */
int snd_names_list(const char *iface ATTRIBUTE_UNUSED,
		   snd_devname_t **list ATTRIBUTE_UNUSED)
{
	return -ENXIO;
}
link_warning(snd_names_list, "Warning: snd_names_list is deprecated, use snd_device_name_hint");

/**
 * \brief Release the list of device names
 * \param list the name list to release
 * \deprecated Since 1.0.14
 *
 * Releases the list of device names allocated via #snd_names_list().
 */
void snd_names_list_free(snd_devname_t *list ATTRIBUTE_UNUSED)
{
}
link_warning(snd_names_list_free, "Warning: snd_names_list_free is deprecated, use snd_device_name_free_hint");
