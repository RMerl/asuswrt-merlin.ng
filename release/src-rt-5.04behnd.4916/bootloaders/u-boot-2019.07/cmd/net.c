// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * Boot support
 */
#include <common.h>
#include <command.h>
#include <net.h>

static int netboot_common(enum proto_t, cmd_tbl_t *, int, char * const []);

#ifdef CONFIG_CMD_BOOTP
static int do_bootp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return netboot_common(BOOTP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	bootp,	3,	1,	do_bootp,
	"boot image via network using BOOTP/TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

#ifdef CONFIG_CMD_TFTPBOOT
int do_tftpb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;

	bootstage_mark_name(BOOTSTAGE_KERNELREAD_START, "tftp_start");
	ret = netboot_common(TFTPGET, cmdtp, argc, argv);
	bootstage_mark_name(BOOTSTAGE_KERNELREAD_STOP, "tftp_done");
	return ret;
}

U_BOOT_CMD(
	tftpboot,	3,	1,	do_tftpb,
	"boot image via network using TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

#ifdef CONFIG_CMD_TFTPPUT
static int do_tftpput(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return netboot_common(TFTPPUT, cmdtp, argc, argv);
}

U_BOOT_CMD(
	tftpput,	4,	1,	do_tftpput,
	"TFTP put command, for uploading files to a server",
	"Address Size [[hostIPaddr:]filename]"
);
#endif

#ifdef CONFIG_CMD_TFTPSRV
static int do_tftpsrv(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	return netboot_common(TFTPSRV, cmdtp, argc, argv);
}

U_BOOT_CMD(
	tftpsrv,	2,	1,	do_tftpsrv,
	"act as a TFTP server and boot the first received file",
	"[loadAddress]\n"
	"Listen for an incoming TFTP transfer, receive a file and boot it.\n"
	"The transfer is aborted if a transfer has not been started after\n"
	"about 50 seconds or if Ctrl-C is pressed."
);
#endif


#ifdef CONFIG_CMD_RARP
int do_rarpb(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return netboot_common(RARP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	rarpboot,	3,	1,	do_rarpb,
	"boot image via network using RARP/TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

#if defined(CONFIG_CMD_DHCP)
static int do_dhcp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return netboot_common(DHCP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	dhcp,	3,	1,	do_dhcp,
	"boot image via network using DHCP/TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

#if defined(CONFIG_CMD_NFS)
static int do_nfs(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return netboot_common(NFS, cmdtp, argc, argv);
}

U_BOOT_CMD(
	nfs,	3,	1,	do_nfs,
	"boot image via network using NFS protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

static void netboot_update_env(void)
{
	char tmp[22];

	if (net_gateway.s_addr) {
		ip_to_string(net_gateway, tmp);
		env_set("gatewayip", tmp);
	}

	if (net_netmask.s_addr) {
		ip_to_string(net_netmask, tmp);
		env_set("netmask", tmp);
	}

	if (net_hostname[0])
		env_set("hostname", net_hostname);

	if (net_root_path[0])
		env_set("rootpath", net_root_path);

	if (net_ip.s_addr) {
		ip_to_string(net_ip, tmp);
		env_set("ipaddr", tmp);
	}
#if !defined(CONFIG_BOOTP_SERVERIP)
	/*
	 * Only attempt to change serverip if net/bootp.c:store_net_params()
	 * could have set it
	 */
	if (net_server_ip.s_addr) {
		ip_to_string(net_server_ip, tmp);
		env_set("serverip", tmp);
	}
#endif
	if (net_dns_server.s_addr) {
		ip_to_string(net_dns_server, tmp);
		env_set("dnsip", tmp);
	}
#if defined(CONFIG_BOOTP_DNS2)
	if (net_dns_server2.s_addr) {
		ip_to_string(net_dns_server2, tmp);
		env_set("dnsip2", tmp);
	}
#endif
	if (net_nis_domain[0])
		env_set("domain", net_nis_domain);

#if defined(CONFIG_CMD_SNTP) && defined(CONFIG_BOOTP_TIMEOFFSET)
	if (net_ntp_time_offset) {
		sprintf(tmp, "%d", net_ntp_time_offset);
		env_set("timeoffset", tmp);
	}
#endif
#if defined(CONFIG_CMD_SNTP) && defined(CONFIG_BOOTP_NTPSERVER)
	if (net_ntp_server.s_addr) {
		ip_to_string(net_ntp_server, tmp);
		env_set("ntpserverip", tmp);
	}
#endif
}

static int netboot_common(enum proto_t proto, cmd_tbl_t *cmdtp, int argc,
		char * const argv[])
{
	char *s;
	char *end;
	int   rcode = 0;
	int   size;
	ulong addr;

	net_boot_file_name_explicit = false;

	/* pre-set load_addr */
	s = env_get("loadaddr");
	if (s != NULL)
		load_addr = simple_strtoul(s, NULL, 16);

	switch (argc) {
	case 1:
		/* refresh bootfile name from env */
		copy_filename(net_boot_file_name, env_get("bootfile"),
			      sizeof(net_boot_file_name));
		break;

	case 2:	/*
		 * Only one arg - accept two forms:
		 * Just load address, or just boot file name. The latter
		 * form must be written in a format which can not be
		 * mis-interpreted as a valid number.
		 */
		addr = simple_strtoul(argv[1], &end, 16);
		if (end == (argv[1] + strlen(argv[1]))) {
			load_addr = addr;
			/* refresh bootfile name from env */
			copy_filename(net_boot_file_name, env_get("bootfile"),
				      sizeof(net_boot_file_name));
		} else {
			net_boot_file_name_explicit = true;
			copy_filename(net_boot_file_name, argv[1],
				      sizeof(net_boot_file_name));
		}
		break;

	case 3:
		load_addr = simple_strtoul(argv[1], NULL, 16);
		net_boot_file_name_explicit = true;
		copy_filename(net_boot_file_name, argv[2],
			      sizeof(net_boot_file_name));

		break;

#ifdef CONFIG_CMD_TFTPPUT
	case 4:
		if (strict_strtoul(argv[1], 16, &save_addr) < 0 ||
		    strict_strtoul(argv[2], 16, &save_size) < 0) {
			printf("Invalid address/size\n");
			return CMD_RET_USAGE;
		}
		net_boot_file_name_explicit = true;
		copy_filename(net_boot_file_name, argv[3],
			      sizeof(net_boot_file_name));
		break;
#endif
	default:
		bootstage_error(BOOTSTAGE_ID_NET_START);
		return CMD_RET_USAGE;
	}
	bootstage_mark(BOOTSTAGE_ID_NET_START);

	size = net_loop(proto);
	if (size < 0) {
		bootstage_error(BOOTSTAGE_ID_NET_NETLOOP_OK);
		return CMD_RET_FAILURE;
	}
	bootstage_mark(BOOTSTAGE_ID_NET_NETLOOP_OK);

	/* net_loop ok, update environment */
	netboot_update_env();

	/* done if no file was loaded (no errors though) */
	if (size == 0) {
		bootstage_error(BOOTSTAGE_ID_NET_LOADED);
		return CMD_RET_SUCCESS;
	}

	bootstage_mark(BOOTSTAGE_ID_NET_LOADED);

	rcode = bootm_maybe_autostart(cmdtp, argv[0]);

	if (rcode == CMD_RET_SUCCESS)
		bootstage_mark(BOOTSTAGE_ID_NET_DONE);
	else
		bootstage_error(BOOTSTAGE_ID_NET_DONE_ERR);
	return rcode;
}

#if defined(CONFIG_CMD_PING)
static int do_ping(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	net_ping_ip = string_to_ip(argv[1]);
	if (net_ping_ip.s_addr == 0)
		return CMD_RET_USAGE;

	if (net_loop(PING) < 0) {
		printf("ping failed; host %s is not alive\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	printf("host %s is alive\n", argv[1]);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	ping,	2,	1,	do_ping,
	"send ICMP ECHO_REQUEST to network host",
	"pingAddress"
);
#endif

#if defined(CONFIG_CMD_CDP)

static void cdp_update_env(void)
{
	char tmp[16];

	if (cdp_appliance_vlan != htons(-1)) {
		printf("CDP offered appliance VLAN %d\n",
		       ntohs(cdp_appliance_vlan));
		vlan_to_string(cdp_appliance_vlan, tmp);
		env_set("vlan", tmp);
		net_our_vlan = cdp_appliance_vlan;
	}

	if (cdp_native_vlan != htons(-1)) {
		printf("CDP offered native VLAN %d\n", ntohs(cdp_native_vlan));
		vlan_to_string(cdp_native_vlan, tmp);
		env_set("nvlan", tmp);
		net_native_vlan = cdp_native_vlan;
	}
}

int do_cdp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int r;

	r = net_loop(CDP);
	if (r < 0) {
		printf("cdp failed; perhaps not a CISCO switch?\n");
		return CMD_RET_FAILURE;
	}

	cdp_update_env();

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	cdp,	1,	1,	do_cdp,
	"Perform CDP network configuration",
	"\n"
);
#endif

#if defined(CONFIG_CMD_SNTP)
int do_sntp(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *toff;

	if (argc < 2) {
		net_ntp_server = env_get_ip("ntpserverip");
		if (net_ntp_server.s_addr == 0) {
			printf("ntpserverip not set\n");
			return CMD_RET_FAILURE;
		}
	} else {
		net_ntp_server = string_to_ip(argv[1]);
		if (net_ntp_server.s_addr == 0) {
			printf("Bad NTP server IP address\n");
			return CMD_RET_FAILURE;
		}
	}

	toff = env_get("timeoffset");
	if (toff == NULL)
		net_ntp_time_offset = 0;
	else
		net_ntp_time_offset = simple_strtol(toff, NULL, 10);

	if (net_loop(SNTP) < 0) {
		printf("SNTP failed: host %pI4 not responding\n",
		       &net_ntp_server);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	sntp,	2,	1,	do_sntp,
	"synchronize RTC via network",
	"[NTP server IP]\n"
);
#endif

#if defined(CONFIG_CMD_DNS)
int do_dns(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc == 1)
		return CMD_RET_USAGE;

	/*
	 * We should check for a valid hostname:
	 * - Each label must be between 1 and 63 characters long
	 * - the entire hostname has a maximum of 255 characters
	 * - only the ASCII letters 'a' through 'z' (case-insensitive),
	 *   the digits '0' through '9', and the hyphen
	 * - cannot begin or end with a hyphen
	 * - no other symbols, punctuation characters, or blank spaces are
	 *   permitted
	 * but hey - this is a minimalist implmentation, so only check length
	 * and let the name server deal with things.
	 */
	if (strlen(argv[1]) >= 255) {
		printf("dns error: hostname too long\n");
		return CMD_RET_FAILURE;
	}

	net_dns_resolve = argv[1];

	if (argc == 3)
		net_dns_env_var = argv[2];
	else
		net_dns_env_var = NULL;

	if (net_loop(DNS) < 0) {
		printf("dns lookup of %s failed, check setup\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	dns,	3,	1,	do_dns,
	"lookup the IP of a hostname",
	"hostname [envvar]"
);

#endif	/* CONFIG_CMD_DNS */

#if defined(CONFIG_CMD_LINK_LOCAL)
static int do_link_local(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	char tmp[22];

	if (net_loop(LINKLOCAL) < 0)
		return CMD_RET_FAILURE;

	net_gateway.s_addr = 0;
	ip_to_string(net_gateway, tmp);
	env_set("gatewayip", tmp);

	ip_to_string(net_netmask, tmp);
	env_set("netmask", tmp);

	ip_to_string(net_ip, tmp);
	env_set("ipaddr", tmp);
	env_set("llipaddr", tmp); /* store this for next time */

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	linklocal,	1,	1,	do_link_local,
	"acquire a network IP address using the link-local protocol",
	""
);

#endif  /* CONFIG_CMD_LINK_LOCAL */
