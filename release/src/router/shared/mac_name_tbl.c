 /*
 * Copyright 2016, ASUSTeK Inc.
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "mac_name_tbl.h"
#include "shutils.h"

char host_name[33];

extern void toUpperCase(char *str);
#if 0
static void toUpperCase(char *str) {
	char *p;

	for(p = str; *p != '\0'; p++)
		*p = toupper((unsigned char) *p);
}
#endif

/*
	search_mnt will help to find the hostname from networkmap database
	input  : mac (Lower or Upper)
	output : if found, return hostname; else return NULL
*/
char *search_mnt(char *mac)
{
	FILE *fp_mnt;
	unsigned int size_ncl;
	char line_buf_s[NCL_LIMIT];
	char *line_buf = NULL, upperMac[13], s_mac[18];
	char *t_str1, *t_str2;

	strlcpy(upperMac, mac, sizeof(upperMac));
	toUpperCase(upperMac);	
	sprintf(s_mac, "%C%C:%C%C:%C%C:%C%C:%C%C:%C%C%c", upperMac[0], upperMac[1], upperMac[2], upperMac[3], upperMac[4], upperMac[5], upperMac[6], upperMac[7], upperMac[8], upperMac[9], upperMac[10], upperMac[11], '\0');
	MNT_DEBUG("[MNT]: search MAC %s\n", s_mac);

	if ((fp_mnt=fopen(NMP_CL_JSON_FILE, "r"))) {
		fseek(fp_mnt, 0L, SEEK_END);
		size_ncl = ftell(fp_mnt);
		fseek(fp_mnt, 0L, SEEK_SET);
		if (size_ncl) {
			while ((line_buf = fgets(line_buf_s, sizeof(line_buf_s), fp_mnt)) != NULL) {
				MNT_DEBUG("[MNT]: DB %s\n", line_buf);
				if((t_str1 = strstr(line_buf, s_mac)) != NULL) {
					if((t_str1 = strstr(t_str1, "name")) != NULL) {
						if((t_str1 = strchr(t_str1, '"')) != NULL) {
							++t_str1;
							if((t_str1 = strchr(t_str1, '"')) != NULL) {
								++t_str1;
								if((t_str2 = strchr(t_str1, '"')) != NULL) {
									*t_str2 = '\0';
									if(strlen(t_str1) > 1) {
										strlcpy(host_name, t_str1, sizeof(host_name));
										fclose(fp_mnt);
										return host_name;
									}
								}
							}
						}
					}
				}
			}
		}
		fclose(fp_mnt);
	}

	return NULL;
}

