/*
 * sensord
 *
 * A daemon that periodically logs sensor information to syslog.
 *
 * Copyright (c) 1999-2002 Merlin Hughes <merlin@merlin.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

#include "sensord.h"
#include "lib/error.h"

static int loadConfig(const char *cfgPath, int reload)
{
	int ret;
 	FILE *fp;

 	/* Load default configuration. */
 	if (!cfgPath) {
 		if (reload) {
			sensorLog(LOG_INFO, "configuration reloading");
			sensors_cleanup();
		}

 		ret = sensors_init(NULL);
 		if (ret) {
 			sensorLog(LOG_ERR, "Error loading default"
 				  " configuration file: %s",
 				  sensors_strerror(ret));
 			return -1;
  		}
 		return 0;
 	}

 	fp = fopen(cfgPath, "r");
 	if (!fp) {
 		sensorLog(LOG_ERR, "Error opening config file %s: %s",
 			  strerror(errno));
 		return -1;
 	}

	if (reload) {
		sensorLog(LOG_INFO, "configuration reloading");
		sensors_cleanup();
	}
 	ret = sensors_init(fp);
 	if (ret) {
 		sensorLog(LOG_ERR, "Error loading sensors configuration file"
			  " %s: %s", cfgPath, sensors_strerror(ret));
 		fclose(fp);
 		return -1;
 	}
 	fclose(fp);

 	return 0;
}

int loadLib(const char *cfgPath)
{
	int ret;
	ret = loadConfig(cfgPath, 0);
	if (!ret)
		ret = initKnownChips();
	return ret;
}

int reloadLib(const char *cfgPath)
{
	int ret;
	freeKnownChips();
	ret = loadConfig(cfgPath, 1);
	if (!ret)
		ret = initKnownChips();
	return ret;
}

int unloadLib(void)
{
	freeKnownChips();
	sensors_cleanup();
	return 0;
}
