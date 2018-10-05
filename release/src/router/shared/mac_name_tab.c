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

#include "mac_name_tab.h"
#include "shutils.h"

char host_name[33];

/*
	search_mnt will help to find the hostname from networkmap database
	input  : mac (Lower or Upper)
	output : if found, return hostname; else return NULL
*/
char *search_mnt(char *mac)
{
	FILE *fp_mnt;		//nmp_client_list FILE
	unsigned int size_ncl;	//size of nmp_client_list
	char *nmp_client_list;
	char *b, *dummy, *dummy2, *db_device_name;
	char lowerMac[13];

	strlcpy(lowerMac, mac, sizeof(lowerMac));
	toLowerCase(lowerMac);	

	if ((fp_mnt=fopen(NMP_CLIENT_LIST_FILENAME, "r"))) {
		fseek(fp_mnt, 0L, SEEK_END);
		size_ncl = ftell(fp_mnt);
		fseek(fp_mnt, 0L, SEEK_SET);
		if (size_ncl && size_ncl < NCL_LIMIT) {
			nmp_client_list = malloc(sizeof(char)*size_ncl + 1);
			if (fread(nmp_client_list, 1, size_ncl, fp_mnt) != size_ncl) {
				MNT_DEBUG("[Mac Name Table] Read ERR....Reset DB!\n");
				free(nmp_client_list);
				fclose(fp_mnt);
				return NULL;
			} 
			MNT_DEBUG("[Mac Name Table] Read: %s from %s\nsearch %s...\n", 
					nmp_client_list, NMP_CLIENT_LIST_FILENAME, lowerMac);
			b = strstr(nmp_client_list, lowerMac);
			if(b) {
				vstrsep(b, ">", &dummy, &dummy2, &db_device_name);
				if(db_device_name) {
					strlcpy(host_name, db_device_name, sizeof(host_name));
					free(nmp_client_list);
					fclose(fp_mnt);
					return host_name;
				}
			}
			free(nmp_client_list);
		}
		fclose(fp_mnt);
	}

	return NULL;
}
