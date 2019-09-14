/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
/*
 * This is the implementation of a routine to notify the rc driver that it
 * should take some action.
 *
 * Copyright 2019, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of ASUSTeK Inc..
 */
#include <shared.h>

#ifdef RTCONFIG_ISP_CUSTOMIZE

#define PACKAGE_FOLDER "/jffs/.package"
#define PACKAGE_SETTING PACKAGE_FOLDER"/settings.ini"

/* returns number of strings replaced.
*/
static int replacestr(char *line, const char *end, const char *search, const char *replace)
{
        int count;
        char *sp; // start of pattern

        if ((sp = strstr(line, search)) == NULL) {
                return(0);
        }
        count = 1;
        int sLen = strlen(search);
        int rLen = strlen(replace);
 	int tLen = strlen(sp) - sLen;
        char *src = sp + sLen;
        char *dst = sp + rLen;

	if(dst+tLen > end-1)
		return 0;

	memmove(dst, src, tLen);
	*(dst+tLen) = '\0';
        memcpy(sp, replace, rLen);

        count += replacestr(sp + rLen, end, search, replace);

        return(count);
}

char *find_customize_setting_by_name(const char *name)
{
#define NVRAM_LINE_MAX (16384)
	char line_buffer[NVRAM_LINE_MAX];
	char multi_line_buffer[NVRAM_LINE_MAX];
	char *key_name,*value;
	int line_len, multi_buffer_left = 0;
	char *line_start, *line_current;
	int rep_count = 0;

#if 0
	// bypass username and passwd
	if (!strcmp(name, "http_username") || 
		!strcmp(name, "http_passwd"))
		return NULL;
#endif

    FILE *fp=fopen(PACKAGE_SETTING,"r");
	if(fp) {
		line_start = line_current = &multi_line_buffer[0];
		multi_buffer_left = sizeof(multi_line_buffer);
		while(fgets(line_buffer,NVRAM_LINE_MAX,fp) != NULL) {
			if(!strlen(line_buffer)) 
				continue;

			rep_count = replacestr(line_buffer, line_buffer+sizeof(line_buffer), "\\", "");
			if((line_len = strlen(line_buffer)) > 0) {
				if (multi_buffer_left > line_len) { // Check if buffer is enough.
					multi_buffer_left -= snprintf(line_current, multi_buffer_left, "%s", line_buffer);
					if (rep_count > 0) { // If '\\' occurs in line_buffer, read next line continuely.
						/*_dprintf("rep_count=[%d], line_len=[%d], line_current=[%s]\n", 
							rep_count, line_len, line_current);*/
						line_current = line_start + (sizeof(multi_line_buffer) - multi_buffer_left);
						continue;
					} else {
						// move cursor to start
						line_current = &multi_line_buffer[0];
						multi_buffer_left = sizeof(multi_line_buffer);
					}
				}
			}

			if(line_start[0]==';' || line_start[0]=='#')
				continue;

			value = line_start;
			key_name = strsep(&value,"=");
			if (!strcmp(key_name, name)) {
				_dprintf("name=%s. FOUND: value=%s\n", name, value);
				return trimNL(value);
			}
		}
		fclose(fp);
	}
	_dprintf("name=%s. NOT FOUND\n", name);
	return NULL;
}
#endif