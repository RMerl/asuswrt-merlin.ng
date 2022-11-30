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
	int nojffs = 0;

	if (!d_exists("/jffs/.ssh")) {
		if (mkdir("/jffs/.ssh", 0700)) {
			logmessage("dropbear", "JFFS unwritable - creating temporary keys in RAM");
			eval("dropbearkey", "-t", "rsa", "-f", "/etc/dropbear/dropbear_rsa_host_key");
			eval("dropbearkey", "-t", "ecdsa", "-f", "/etc/dropbear/dropbear_ecdsa_host_key");
			eval("dropbearkey", "-t", "ed25519", "-f", "/etc/dropbear/dropbear_ed25519_host_key");
			nojffs = 1;
		}
	}

	if (!nojffs) {
		if (!f_exists("/jffs/.ssh/dropbear_rsa_host_key"))
			eval("dropbearkey", "-t", "rsa", "-f", "/jffs/.ssh/dropbear_rsa_host_key");
		if (!f_exists("/jffs/.ssh/dropbear_ecdsa_host_key"))
			eval("dropbearkey", "-t", "ecdsa", "-f", "/jffs/.ssh/dropbear_ecdsa_host_key");
		if (!f_exists("/jffs/.ssh/dropbear_ed25519_host_key"))
			eval("dropbearkey", "-t", "ed25519", "-f", "/jffs/.ssh/dropbear_ed25519_host_key");

		eval("ln", "-s", "/jffs/.ssh/dropbear_rsa_host_key", "/etc/dropbear/dropbear_rsa_host_key");
		eval("ln", "-s", "/jffs/.ssh/dropbear_ecdsa_host_key", "/etc/dropbear/dropbear_ecdsa_host_key");
		eval("ln", "-s", "/jffs/.ssh/dropbear_ed25519_host_key", "/etc/dropbear/dropbear_ed25519_host_key");
	}
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
