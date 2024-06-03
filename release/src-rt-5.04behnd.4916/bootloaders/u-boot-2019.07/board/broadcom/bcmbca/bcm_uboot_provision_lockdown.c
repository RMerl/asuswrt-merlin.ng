// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2020 Broadcom Corporation
 * Joel Peshkin, Broadcom Corporation, joel.peshkin@broadcom.com
 */

#define DEBUG
#define USE_FIXED_PASSWORD 1

#include <common.h>
#include <command.h>
#include <environment.h>
#include <hexdump.h>
#include <linux/ctype.h>
#include <linux/stddef.h>
#include <u-boot/sha256.h>
#include <errno.h>
#include <bca_sdk.h>
#include "bcm_secure.h"

DECLARE_GLOBAL_DATA_PTR;

static const char *null_env = "\0\0\0\0";

#define BCM_LOCKDOWN_MAX_ENV_NAME 64
#define BCM_LOCKDOWN_MAX_ENV_VALUE 520
/* also limit with CONFIG_ENV_SIZE */

static void lock_env_add_safe(char *name, char *val, char *safe, int max);

/* 
 * lock_env_add_safe:  Add environment variable only if its value is strictly alphanumeric or
 *                     alphanumeric plus a specific set of characters
 */
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
	for (i = 0; i < l; i++) {
		v[0] = val[i];
		if (!(isalnum(val[i]) || (NULL != strstr(safe, v)))) {
			// printf("rejected '%s'='%s'\n",name,val); 
			return;
		}
	}
	// printf("accepted '%s'='%s'\n",name,val); 
	env_set(name, val);
}

int env_override_import(void *ep)
{
	char sha_env_str[SHA256_SUM_LEN * 4 + 1]; /* enough room for salt */
	char password[32];
	char salt[32] = {0};
	char dnum[16] = {0};
	u8 sha_env[SHA256_SUM_LEN];
	char *cp = ((char *)ep) + 4;	/* skip length field */
	char *delim;
	int size = 0;
	int i,j;
	int node,len;
#if defined(CONFIG_BCMBCA_RELAX_CHECKS_WHEN_NONSECURE)
	bcm_sec_state_t sec_state;
	(void)bcm_sec_boot_state(&sec_state);
	if (sec_state == SEC_STATE_UNSEC) {
		printf("Board not in secure state -- (RELAX) allow standard environment\n");
		return(0);
	}
#endif
	env_import((void *)null_env, 0);
	env_set("overridden", "true");
	/* Iterate over stored environment variables and only import them if they are trusted */
	while (*cp != '\0') {
		delim = strstr(cp, "=");
		if (delim && ((unsigned char *)delim <= (unsigned char *)(cp + BCM_LOCKDOWN_MAX_ENV_NAME))
		    && ((uintptr_t)delim <= (uintptr_t)(ep + CONFIG_ENV_SIZE))) {
			*delim = '\0';
			delim++;
			if (strlen(delim) > BCM_LOCKDOWN_MAX_ENV_VALUE) {
				// printf("TOO LONG '%s'\n",cp); 
				break;
			}
			/* if we made it this far, *cp and *delim point to name and value */

			if ((strcmp(cp, "IMAGE") == 0)
			    || (strcmp(cp, "MCB") == 0)
			    || (strcmp(cp, "serial") == 0)
			    || (strcmp(cp, "ipaddr") == 0)
			    || (strcmp(cp, "ethaddr") == 0)) {
				/* These are OK as long as they contain only alphanumeric plus : */
				lock_env_add_safe(cp, delim, ":,.", 32);
			} else if (strcmp(cp, "env_boot_magic") == 0) {
				/* env_boot_magic is safe but can have '@' */
				lock_env_add_safe(cp, delim, "@,", 64);
			} else if (strcmp(cp,"env_boot_magic_updated") == 0) {
				lock_env_add_safe(cp,delim,"",32);
			} else if (strcmp(cp, "once") == 0) {
				/* "once" can be stored as "once=true" 
				 * otherwise, we will need to set it to a fixed script later
				 */
				if (strcmp(delim, "true") == 0) {
					env_set(cp, delim);
				}
			} else if (strncmp(cp, "rdp", 3) == 0) {
				lock_env_add_safe(cp, delim, "", 6);
			} else if (strcmp(cp, "nummacaddrs") == 0) {
				lock_env_add_safe(cp, delim, "_", 32);
			} else if (strcmp(cp, "boardid") == 0) {
				lock_env_add_safe(cp, delim, "_", 32);
			} else if (strncasecmp(cp, "wl", 2) == 0) {
				lock_env_add_safe(cp, delim, "", 6);
			} else {
				// printf("skipped '%s'='%s'\n",cp,delim); 
			}

			/* done with conditional import */
			cp = delim + strlen(delim) + 1;
		} else {
			// printf("bailed out\n");
			break;
		}
	}

	/* Boot command can only be set from this (signed) code and will not accept arbitrary values */
	env_set("bootdelay", "5");
	env_set("bootcmd", "sdk load_img ; fdt rm serial0 interrupts ; bootm go");

	// gd->flags |= GD_FLG_DISABLE_CONSOLE; 

	// WARNING -- uboot password will be needed before serial number is
	// set, so may be better to use a function of the device key
	// like, perhaps, crc32(sha256(some_device_key))
	node = fdt_path_offset(gd->fdt_blob, "/trust/serial_num");
	if (node < 0) {
		printf("no serial number node in uboot DTB\n");
	} else {
		unsigned long sn = 0;
		len = 0;
		cp = NULL;
		cp = (char *)fdt_getprop(gd->fdt_blob, node, "value", &len);
		printf("full serial number: ");
		for (i = 0; i < len ; i++) {
			printf("%02x",cp[i]);
		}
		printf("\n");
		if (len) {
			for (i = 0 ; i < 6 ; i++) {
				sn = (sn << 8) | (long)(cp[len-6+i]);
			}
			printf("decimal serial number: %ld\n",sn);
			sprintf(dnum,"%ld",sn);
			env_set("decimal_serial_num", dnum);
		}
	}

	/* Now set bootstopkeysha256 */

	/* In this example, password is being set to a fixed value.   More likely, you will want to set it to
	 * something like the base64 of the last 6 bytes of the sha256 of a secret concatenated with the serial 
	 * number 
	 */

	size = SHA256_SUM_LEN;
	j = 0;

	node = fdt_path_offset(gd->fdt_blob, "/trust/key_cli_seed");

#ifndef USE_FIXED_PASSWORD
	if (node < 0) {
		printf("no cli seed node in uboot DTB\n");
	} else {
		char combined[256];
		long long pass;
		char cset[] = "abcdefghijk#mnopqrstuvwxyzABCDEFGHIJKLMN-PQRSTUVWXYZ@!23456789_*";
		len = 0;
		cp = NULL;
		cp = (char *)fdt_getprop(gd->fdt_blob, node, "value", &len);
		strncpy(combined,cp,256);
		strncat(combined,dnum,256);
		hash_block("sha256", (const void *)combined,
		   strlen(combined), sha_env, &size);
		memcpy(&pass, sha_env, sizeof(pass));
		for (i = 0 ; i < 8 ; i++) {
			password[i] = cset[ (pass >> (6 * i) ) & 0x3f ];
		}
		password[8] = 0;
		printf("OBVIOUSLY -- REMOVE THIS PRINT\npassword set to %s\n",password);
	}
#endif
		

#ifdef USE_FIXED_PASSWORD
	strcpy(salt, "S4lt3d@");
	strcpy(password, salt);
	strcat(password, "test54321");
#endif

	hash_block("sha256", (const void *)password,
		   strlen(password), sha_env, &size);
	j = sprintf(sha_env_str,"%s:",salt);
	for (i = 0; i < SHA256_SUM_LEN; i++) {
		j += sprintf(&sha_env_str[j], "%02x", sha_env[i]);
	}

	env_set("bootstopkeysha256", sha_env_str);
	return 1;
}
