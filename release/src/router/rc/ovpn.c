/*
 * Copyright 2018, ASUSTeK Inc.
 * All Rights Reserved.
 *
 */

#include "rc.h"

int ovpn_route_up_main(int argc, char **argv)
{
//	ovpn_route_up_handler();

	return 0;
}

void create_ovpn_passwd()
{
	FILE *fps, *fpp;
	unsigned char s[512];
	char salt[32], *p;
	char *nv, *nvp, *b;
	char *username, *passwd;
#ifdef RTCONFIG_NVRAM_ENCRYPT
	char dec_passwd[256];
#endif
	int gid = 200; /* OpenVPN GID */
	int uid = 200;

	strcpy(salt, "$1$");
	f_read("/dev/urandom", s, 6);
	base64_encode(s, salt + 3, 6);
	salt[3 + 8] = 0;
	p = salt;
	while (*p) {
		if (*p == '+') *p = '.';
		++p;
	}

	fps = fopen("/etc/shadow.openvpn", "w");
	fpp = fopen("/etc/passwd.openvpn", "w");
	if (fps == NULL || fpp == NULL)
		goto error;

	nv = nvp = strdup(nvram_safe_get("vpn_serverx_clientlist"));

	if (nv) {
		while ((b = strsep(&nvp, "<")) != NULL) {
			if (vstrsep(b, ">", &username, &passwd) != 2)
				continue;
			if (*username == '\0' || *passwd == '\0')
				continue;
#ifdef RTCONFIG_NVRAM_ENCRYPT
			memset(dec_passwd, 0, sizeof(dec_passwd));
			pw_dec(passwd, dec_passwd, sizeof(dec_passwd));
			passwd = dec_passwd;
#endif
			p = crypt(passwd, salt);
			fprintf(fps, "%s:%s:0:0:99999:7:0:0:\n", username, p);
			fprintf(fpp, "%s:x:%d:%d::/dev/null:/dev/null\n", username, uid, gid);
			uid++;
		}
		free(nv);
	}

error:
	if (fps)
		fclose(fps);
	if (fpp)
		fclose(fpp);

	chmod("/etc/shadow.openvpn", 0600);
	chmod("/etc/passwd.openvpn", 0644);
}

