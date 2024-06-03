// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 Broadcom Corporation
 * Joel Peshkin, Broadcom Corporation, joel.peshkin@broadcom.com
 */

#define DEBUG

#include <common.h>
#include <command.h>
#include <environment.h>
#include <hexdump.h>
#include <linux/ctype.h>
#include <linux/stddef.h>
#include <errno.h>
#include <bca_sdk.h>

DECLARE_GLOBAL_DATA_PTR;

static const char *null_env = "\0\0\0\0";

#define BCM_LOCKDOWN_MAX_ENV_NAME 64
#define BCM_LOCKDOWN_MAX_ENV_VALUE 520
/* also limit with CONFIG_ENV_SIZE */

static void lock_env_add_safe(char *name, char *val, char * safe, int max);

static void lock_env_add_safe(char *name, char *val, char *safe, int max)
{
	int l;
	int i;
	char v[2];
	v[1] = '\0';
	l = strlen(val);
	if (l > max) {
		return;
	}
	for (i = 0 ; i < l ; i++) {
		v[0] = val[i];
		if (!(isalnum(val[i]) || (NULL != strstr(safe,v)))) {
			/* printf("rejected '%s'='%s'\n",name,val); */
			return;
		}
	}
	env_set(name, val);
}

int env_override_import(void *ep)
{
	char *cp = ((char *)ep) + 4; /* skip length field */
	char *delim;
	env_import ((void *) null_env, 0);
	env_set("overridden", "true");
	while (*cp != '\0') {
		delim = strstr(cp,"=");
		if (delim && (delim <= cp + BCM_LOCKDOWN_MAX_ENV_NAME) && (delim <= ep + CONFIG_ENV_SIZE)) {
			*delim = '\0';
			delim++;
			if (strlen(delim) > BCM_LOCKDOWN_MAX_ENV_VALUE) {
				break;
			}
			/* if we made it this far, *cp and *delim point to name and value */

			if ((strcmp(cp,"IMAGE") == 0) || (strcmp(cp,"MCB") == 0) || (strcmp(cp,"ethaddr") == 0)) {
				lock_env_add_safe(cp,delim,":,",32);
			} else if (strncmp(cp,"demo_",5) == 0) {
				env_set(cp,delim);
			} else if (strcmp(cp,"env_boot_magic") == 0) {
				lock_env_add_safe(cp,delim,"@,",32);
			} else if (strcmp(cp,"env_boot_magic_updated") == 0) {
				lock_env_add_safe(cp,delim,"",32);
			} else if (strncmp(cp,"rdp",3) == 0) {
				lock_env_add_safe(cp,delim,"",6);
			} else if (strcmp(cp,"nummacaddrs") == 0) {
				lock_env_add_safe(cp,delim,"_",32);
			} else if (strcmp(cp,"boardid") == 0) {
				lock_env_add_safe(cp,delim,"_",32);
			} else {
				/* printf("skipped '%s'='%s'\n",cp,delim); */
			}


			/* done with conditional import */
			cp = delim + strlen(delim) + 1;
		} else {
			break;
		}
	}
	env_set("bootdelay", "1");
	env_set("bootcmd", "printenv;sdk boot_img");
	/* gd->flags |= GD_FLG_SILENT | GD_FLG_DISABLE_CONSOLE; */
	gd->flags |= GD_FLG_DISABLE_CONSOLE; 
	return 1;
}

