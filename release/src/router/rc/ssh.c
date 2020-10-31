/*

	Tomato Firmware
	Copyright (C) 2006-2009 Jonathan Zarate

*/

#include <shared.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "rc.h"

static void check_host_keys()
{
	int nvram_change = 0;

	if (!f_exists("/jffs/.ssh/dropbear_rsa_host_key") ||
	    !f_exists("/jffs/.ssh/dropbear_rsa_host_key") ||
	    !f_exists("/jffs/.ssh/dropbear_ecdsa_host_key") ||
	    !f_exists("/jffs/.ssh/dropbear_ed25519_host_key")) {
		mkdir("/jffs/.ssh", 0700);

		if (nvram_get_file("sshd_hostkey", "/jffs/.ssh/dropbear_rsa_host_key", 2048)) {
			nvram_unset("sshd_hostkey");
			nvram_change = 1;
		} else
			eval("dropbearkey", "-t", "rsa", "-f", "/jffs/.ssh/dropbear_rsa_host_key");

		if (nvram_get_file("sshd_dsskey", "/jffs/.ssh/dropbear_dss_host_key", 2048)) {
			nvram_unset("sshd_dsskey");
			nvram_change = 1;
		} else
			eval("dropbearkey", "-t", "dss", "-f", "/jffs/.ssh/dropbear_dss_host_key");

		if (nvram_get_file("sshd_ecdsakey", "/jffs/.ssh/dropbear_ecdsa_host_key", 2048)) {
			nvram_unset("sshd_ecdsakey");
			nvram_change = 1;
		} else
			eval("dropbearkey", "-t", "ecdsa", "-f", "/jffs/.ssh/dropbear_ecdsa_host_key");

		eval("dropbearkey", "-t", "ed25519", "-f", "/jffs/.ssh/dropbear_ed25519_host_key");
	} else {
		if (nvram_get("sshd_hostkey")) {
			nvram_unset("sshd_hostkey");
			nvram_change = 1;
		}
		if (nvram_get("sshd_dsskey")) {
			nvram_unset("sshd_dsskey");
			nvram_change = 1;
		}
		if (nvram_get("sshd_ecdsakey")) {
			nvram_unset("sshd_ecdsakey");
			nvram_change = 1;
		}
	}

	if (nvram_change)
		nvram_commit();

	eval("ln", "-s", "/jffs/.ssh/dropbear_rsa_host_key", "/etc/dropbear/dropbear_rsa_host_key");
	eval("ln", "-s", "/jffs/.ssh/dropbear_dss_host_key", "/etc/dropbear/dropbear_dss_host_key");
	eval("ln", "-s", "/jffs/.ssh/dropbear_ecdsa_host_key", "/etc/dropbear/dropbear_ecdsa_host_key");
	eval("ln", "-s", "/jffs/.ssh/dropbear_ed25519_host_key", "/etc/dropbear/dropbear_ed25519_host_key");
}

int start_sshd(void)
{
	char buf[3500], *port;
	char *dropbear_argv[] = { "dropbear",
		"-p", buf,	/* -p [address:]port */
		NULL,		/* -s */
		NULL, NULL,	/* -W receive_window_buffer */
		NULL, NULL,	/* -a or -j -k */
		NULL };
	int index = 3;

	if (!nvram_get_int("sshd_enable"))
		return 0;

	if (getpid() != 1) {
		notify_rc("start_sshd");
		return 0;
	}

	mkdir("/etc/dropbear", 0700);
	mkdir("/root/.ssh", 0700);

	strlcpy(buf, nvram_safe_get("sshd_authkeys"), sizeof(buf));
	replace_char(buf, '>', '\n');

	f_write_string("/root/.ssh/authorized_keys", buf, 0, 0700);

	check_host_keys();

	port = buf;
	if (is_routing_enabled() && nvram_get_int("sshd_enable") != 1)
		port += snprintf(buf, sizeof(buf), "%s:", nvram_safe_get("lan_ipaddr"));
	snprintf(port, sizeof(buf) - (port - buf), "%d", nvram_get_int("sshd_port") ? : 22);

	if (!nvram_get_int("sshd_pass"))
		dropbear_argv[index++] = "-s";

	if (nvram_get_int("sshd_rwb")) {
		dropbear_argv[index++] = "-W";
		dropbear_argv[index++] = nvram_safe_get("sshd_rwb");
	}

	if (nvram_get_int("sshd_forwarding")) {
		dropbear_argv[index++] = "-a";
	} else {
		dropbear_argv[index++] = "-j";
		dropbear_argv[index++] = "-k";
	}

	return _eval(dropbear_argv, NULL, 0, NULL);
}

void stop_sshd(void)
{
	if (getpid() != 1) {
		notify_rc("stop_sshd");
		return;
	}

	if (pids("dropbear"))
		killall_tk("dropbear");
}
