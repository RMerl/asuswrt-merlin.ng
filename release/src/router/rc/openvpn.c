/*

	Copyright (C) 2008-2010 Keith Moyer, tomatovpn@keithmoyer.com
	Portions Copyright (C) 2012-2020 Eric Sauvageau

	No part of this file may be used without permission.

*/

#include <rc.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#include <openvpn_config.h>
#include <openvpn_control.h>


void start_ovpn_eas()
{
	char buffer[16], *cur;
	int nums[OVPN_CLIENT_MAX], i = 0;

	if (strlen(nvram_safe_get("vpn_serverx_start")) == 0 && strlen(nvram_safe_get("vpn_clientx_eas")) == 0) return;

	// Parse and start servers
	strlcpy(buffer, nvram_safe_get("vpn_serverx_start"), sizeof(buffer));
	for( cur = strtok(buffer,","); cur != NULL && i < OVPN_CLIENT_MAX; cur = strtok(NULL, ",")) { nums[i++] = atoi(cur); }
	if(i < OVPN_CLIENT_MAX) nums[i] = 0;
	for( i = 0; nums[i] > 0 && i < OVPN_CLIENT_MAX; i++ )
	{

		sprintf(buffer, "vpnserver%d", nums[i]);
		if ( pidof(buffer) >= 0 )
			stop_ovpn_server(nums[i]);

		start_ovpn_server(nums[i]);
	}

	// Setup client routing in case some are set to be blocked when tunnel is down
	for( i = 1; i <= OVPN_CLIENT_MAX; i++ ) {
		update_ovpn_routing(i);
	}

	// Parse and start clients
	strlcpy(buffer, nvram_safe_get("vpn_clientx_eas"), sizeof(buffer));
	i = 0;
	for( cur = strtok(buffer,","); cur != NULL && i < OVPN_CLIENT_MAX; cur = strtok(NULL, ",")) { nums[i++] = atoi(cur); }
	if(i < OVPN_CLIENT_MAX) nums[i] = 0;
	for( i = 0; nums[i] > 0 && i < OVPN_CLIENT_MAX; i++ )
	{
		sprintf(buffer, "vpnclient%d", nums[i]);
		if ( pidof(buffer) >= 0 )
			stop_ovpn_client(nums[i]);

		start_ovpn_client(nums[i]);

	}
}

void stop_ovpn_eas()
{
	char buffer[16], *cur;
	int nums[6], i;

	// Parse and stop servers
	strlcpy(buffer, nvram_safe_get("vpn_serverx_start"), sizeof(buffer));
	i = 0;
	for( cur = strtok(buffer,","); cur != NULL && i <= OVPN_SERVER_MAX; cur = strtok(NULL, ",")) { nums[i++] = atoi(cur); }
	nums[i] = 0;
	for( i = 0; nums[i] > 0; i++ )
	{
		sprintf(buffer, "vpnserver%d", nums[i]);
		if ( pidof(buffer) >= 0 )
			stop_ovpn_server(nums[i]);
	}

	// Parse and stop clients
	strlcpy(buffer, nvram_safe_get("vpn_clientx_eas"), sizeof(buffer));
	i = 0;
	for( cur = strtok(buffer,","); cur != NULL && i <= OVPN_CLIENT_MAX; cur = strtok(NULL, ",")) { nums[i++] = atoi(cur); }
	nums[i] = 0;
	for( i = 0; nums[i] > 0; i++ )
	{
		sprintf(buffer, "vpnclient%d", nums[i]);
		if ( pidof(buffer) >= 0 )
			stop_ovpn_client(nums[i]);
	}
}

void stop_ovpn_all()
{
	char buffer[16];
	int i;

	// stop servers
	for( i = 1; i <= OVPN_SERVER_MAX; i++ )
	{
		sprintf(buffer, "vpnserver%d", i);
		if ( pidof(buffer) >= 0 )
			stop_ovpn_server(i);
	}

	// stop clients
	for( i = 1; i <= OVPN_CLIENT_MAX; i++ )
	{
		sprintf(buffer, "vpnclient%d", i);
		if ( pidof(buffer) >= 0 )
			stop_ovpn_client(i);
	}

	// Remove tunnel interface module
	modprobe_r("tun");
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
