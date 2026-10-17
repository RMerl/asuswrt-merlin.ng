// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#define LOG_CATEGORY	UCLASS_ETH

/*
 * Boot support
 */
#include <common.h>
#include <bootstage.h>
#include <command.h>
#include <dm.h>
#include <dm/devres.h>
#include <env.h>
#include <image.h>
#include <log.h>
#include <net.h>
#include <net6.h>
#include <net/udp.h>
#include <net/sntp.h>
#include <net/ncsi.h>

static int netboot_common(enum proto_t, struct cmd_tbl *, int, char * const []);

#ifdef CONFIG_CMD_BOOTP
static int do_bootp(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
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
int do_tftpb(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int ret;

	bootstage_mark_name(BOOTSTAGE_KERNELREAD_START, "tftp_start");
	ret = netboot_common(TFTPGET, cmdtp, argc, argv);
	bootstage_mark_name(BOOTSTAGE_KERNELREAD_STOP, "tftp_done");
	return ret;
}

#if IS_ENABLED(CONFIG_IPV6)
U_BOOT_CMD(
	tftpboot,	4,	1,	do_tftpb,
	"boot image via network using TFTP protocol\n"
	"To use IPv6 add -ipv6 parameter or use IPv6 hostIPaddr framed "
	"with [] brackets",
	"[loadAddress] [[hostIPaddr:]bootfilename] [" USE_IP6_CMD_PARAM "]"
);
#else
U_BOOT_CMD(
	tftpboot,	3,	1,	do_tftpb,
	"load file via network using TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif
#endif

#ifdef CONFIG_CMD_TFTPPUT
static int do_tftpput(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
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
static int do_tftpsrv(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
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
int do_rarpb(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return netboot_common(RARP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	rarpboot,	3,	1,	do_rarpb,
	"boot image via network using RARP/TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

#if defined(CONFIG_CMD_DHCP6)
static int do_dhcp6(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	int i;
	int dhcp_argc;
	char *dhcp_argv[] = {NULL, NULL, NULL, NULL};

	/* Add -ipv6 flag for autoload */
	for (i = 0; i < argc; i++)
		dhcp_argv[i] = argv[i];
	dhcp_argc = argc + 1;
	dhcp_argv[dhcp_argc - 1] =  USE_IP6_CMD_PARAM;

	return netboot_common(DHCP6, cmdtp, dhcp_argc, dhcp_argv);
}

U_BOOT_CMD(dhcp6,	3,	1,	do_dhcp6,
	   "boot image via network using DHCPv6/TFTP protocol.\n"
	   "Use IPv6 hostIPaddr framed with [] brackets",
	   "[loadAddress] [[hostIPaddr:]bootfilename]");
#endif

#if defined(CONFIG_CMD_DHCP)
static int do_dhcp(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	return netboot_common(DHCP, cmdtp, argc, argv);
}

U_BOOT_CMD(
	dhcp,	3,	1,	do_dhcp,
	"boot image via network using DHCP/TFTP protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);

int dhcp_run(ulong addr, const char *fname, bool autoload)
{
	char *dhcp_argv[] = {"dhcp", NULL, (char *)fname, NULL};
	struct cmd_tbl cmdtp = {};	/* dummy */
	char file_addr[17];
	int old_autoload;
	int ret, result;

	log_debug("addr=%lx, fname=%s, autoload=%d\n", addr, fname, autoload);
	old_autoload = env_get_yesno("autoload");
	ret = env_set("autoload", autoload ? "y" : "n");
	if (ret)
		return log_msg_ret("en1", -EINVAL);

	if (autoload) {
		sprintf(file_addr, "%lx", addr);
		dhcp_argv[1] = file_addr;
	}

	result = do_dhcp(&cmdtp, 0, !autoload ? 1 : fname ? 3 : 2, dhcp_argv);

	ret = env_set("autoload", old_autoload == -1 ? NULL :
		      old_autoload ? "y" : "n");
	if (ret)
		return log_msg_ret("en2", -EINVAL);

	if (result)
		return log_msg_ret("res", -ENOENT);

	return 0;
}
#endif

#if defined(CONFIG_CMD_NFS)
static int do_nfs(struct cmd_tbl *cmdtp, int flag, int argc,
		  char *const argv[])
{
	return netboot_common(NFS, cmdtp, argc, argv);
}

U_BOOT_CMD(
	nfs,	3,	1,	do_nfs,
	"boot image via network using NFS protocol",
	"[loadAddress] [[hostIPaddr:]bootfilename]"
);
#endif

#if defined(CONFIG_CMD_WGET)
static int do_wget(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	return netboot_common(WGET, cmdtp, argc, argv);
}

U_BOOT_CMD(
	wget,   3,      1,      do_wget,
	"boot image via network using HTTP protocol",
	"[loadAddress] [[hostIPaddr:]path and image name]"
);
#endif

static void netboot_update_env(void)
{
	char tmp[46];

	if (net_gateway.s_addr) {
		ip_to_string(net_gateway, tmp);
		env_set("gatewayip", tmp);
	}

	if (net_netmask.s_addr) {
		ip_to_string(net_netmask, tmp);
		env_set("netmask", tmp);
	}

#ifdef CONFIG_CMD_BOOTP
	if (net_hostname[0])
		env_set("hostname", net_hostname);
#endif

#ifdef CONFIG_CMD_BOOTP
	if (net_root_path[0])
		env_set("rootpath", net_root_path);
#endif

	if (net_ip.s_addr) {
		ip_to_string(net_ip, tmp);
		env_set("ipaddr", tmp);
	}
	/*
	 * Only attempt to change serverip if net/bootp.c:store_net_params()
	 * could have set it
	 */
	if (!IS_ENABLED(CONFIG_BOOTP_SERVERIP) && net_server_ip.s_addr) {
		ip_to_string(net_server_ip, tmp);
		env_set("serverip", tmp);
	}
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
#ifdef CONFIG_CMD_BOOTP
	if (net_nis_domain[0])
		env_set("domain", net_nis_domain);
#endif

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

	if (IS_ENABLED(CONFIG_IPV6)) {
		if (!ip6_is_unspecified_addr(&net_ip6) ||
		    net_prefix_length != 0) {
			if (net_prefix_length != 0)
				snprintf(tmp, sizeof(tmp), "%pI6c/%d", &net_ip6, net_prefix_length);
			else
				snprintf(tmp, sizeof(tmp), "%pI6c", &net_ip6);
			env_set("ip6addr", tmp);
		}

		if (!ip6_is_unspecified_addr(&net_server_ip6)) {
			snprintf(tmp, sizeof(tmp), "%pI6c", &net_server_ip6);
			env_set("serverip6", tmp);
		}

		if (!ip6_is_unspecified_addr(&net_gateway6)) {
			snprintf(tmp, sizeof(tmp), "%pI6c", &net_gateway6);
			env_set("gatewayip6", tmp);
		}
	}
}

/**
 * parse_addr_size() - parse address and size arguments for tftpput
 *
 * @argv:	command line arguments
 * Return:	0 on success
 */
static int parse_addr_size(char * const argv[])
{
	if (strict_strtoul(argv[1], 16, &image_save_addr) < 0 ||
	    strict_strtoul(argv[2], 16, &image_save_size) < 0) {
		printf("Invalid address/size\n");
		return CMD_RET_USAGE;
	}
	return 0;
}

/**
 * parse_args() - parse command line arguments
 *
 * @proto:	command prototype
 * @argc:	number of arguments
 * @argv:	command line arguments
 * Return:	0 on success
 */
static int parse_args(enum proto_t proto, int argc, char *const argv[])
{
	ulong addr;
	char *end;

	switch (argc) {
	case 1:
		if (IS_ENABLED(CONFIG_CMD_TFTPPUT) && proto == TFTPPUT)
			return 1;

		/* refresh bootfile name from env */
		copy_filename(net_boot_file_name, env_get("bootfile"),
			      sizeof(net_boot_file_name));
		break;

	case 2:
		if (IS_ENABLED(CONFIG_CMD_TFTPPUT) && proto == TFTPPUT)
			return 1;
		/*
		 * Only one arg - accept two forms:
		 * Just load address, or just boot file name. The latter
		 * form must be written in a format which can not be
		 * mis-interpreted as a valid number.
		 */
		addr = hextoul(argv[1], &end);
		if (end == (argv[1] + strlen(argv[1]))) {
			image_load_addr = addr;
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
		if (IS_ENABLED(CONFIG_CMD_TFTPPUT) && proto == TFTPPUT) {
			if (parse_addr_size(argv))
				return 1;
		} else {
			image_load_addr = hextoul(argv[1], NULL);
			net_boot_file_name_explicit = true;
			copy_filename(net_boot_file_name, argv[2],
				      sizeof(net_boot_file_name));
		}
		break;

#ifdef CONFIG_CMD_TFTPPUT
	case 4:
		if (parse_addr_size(argv))
			return 1;
		net_boot_file_name_explicit = true;
		copy_filename(net_boot_file_name, argv[3],
			      sizeof(net_boot_file_name));
		break;
#endif
	default:
		return 1;
	}
	return 0;
}

static int netboot_common(enum proto_t proto, struct cmd_tbl *cmdtp, int argc,
			  char *const argv[])
{
	char *s;
	int   rcode = 0;
	int   size;

	net_boot_file_name_explicit = false;
	*net_boot_file_name = '\0';

	/* pre-set image_load_addr */
	s = env_get("loadaddr");
	if (s != NULL)
		image_load_addr = hextoul(s, NULL);

	if (IS_ENABLED(CONFIG_IPV6)) {
		use_ip6 = false;

		/* IPv6 parameter has to be always *last* */
		if (!strcmp(argv[argc - 1], USE_IP6_CMD_PARAM)) {
			use_ip6 = true;
			/* It is a hack not to break switch/case code */
			--argc;
		}
	}

	if (parse_args(proto, argc, argv)) {
		bootstage_error(BOOTSTAGE_ID_NET_START);
		return CMD_RET_USAGE;
	}

	bootstage_mark(BOOTSTAGE_ID_NET_START);

	if (IS_ENABLED(CONFIG_IPV6) && !use_ip6) {
		char *s, *e;
		size_t len;

		s = strchr(net_boot_file_name, '[');
		e = strchr(net_boot_file_name, ']');
		if (s && e) {
			len = e - s;
			if (!string_to_ip6(s + 1, len - 1, &net_server_ip6))
				use_ip6 = true;
		}
	}

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
static int do_ping(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
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

#if IS_ENABLED(CONFIG_CMD_PING6)
int do_ping6(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	if (string_to_ip6(argv[1], strlen(argv[1]), &net_ping_ip6))
		return CMD_RET_USAGE;

	use_ip6 = true;
	if (net_loop(PING6) < 0) {
		use_ip6 = false;
		printf("ping6 failed; host %pI6c is not alive\n",
		       &net_ping_ip6);
		return 1;
	}

	use_ip6 = false;
	printf("host %pI6c is alive\n", &net_ping_ip6);
	return 0;
}

U_BOOT_CMD(
	ping6,  2,      1,      do_ping6,
	"send ICMPv6 ECHO_REQUEST to network host",
	"pingAddress"
);
#endif /* CONFIG_CMD_PING6 */

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

int do_cdp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
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
static struct udp_ops sntp_ops = {
	.prereq = sntp_prereq,
	.start = sntp_start,
	.data = NULL,
};

int do_sntp(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
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

	if (udp_loop(&sntp_ops) < 0) {
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
int do_dns(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
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
static int do_link_local(struct cmd_tbl *cmdtp, int flag, int argc,
			 char *const argv[])
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

static int do_net_list(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	const struct udevice *current = eth_get_dev();
	unsigned char env_enetaddr[ARP_HLEN];
	const struct udevice *dev;
	struct uclass *uc;

	uclass_id_foreach_dev(UCLASS_ETH, dev, uc) {
		eth_env_get_enetaddr_by_index("eth", dev_seq(dev), env_enetaddr);
		printf("eth%d : %s %pM %s\n", dev_seq(dev), dev->name, env_enetaddr,
		       current == dev ? "active" : "");
	}
	return CMD_RET_SUCCESS;
}

static int do_net_stats(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	int nstats, err, i, off;
	struct udevice *dev;
	u64 *values;
	u8 *strings;

	if (argc < 2)
		return CMD_RET_USAGE;

	err = uclass_get_device_by_name(UCLASS_ETH, argv[1], &dev);
	if (err) {
		printf("Could not find device %s\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	if (!eth_get_ops(dev)->get_sset_count ||
	    !eth_get_ops(dev)->get_strings ||
	    !eth_get_ops(dev)->get_stats) {
		printf("Driver does not implement stats dump!\n");
		return CMD_RET_FAILURE;
	}

	nstats = eth_get_ops(dev)->get_sset_count(dev);
	strings = kcalloc(nstats, ETH_GSTRING_LEN, GFP_KERNEL);
	if (!strings)
		return CMD_RET_FAILURE;

	values = kcalloc(nstats, sizeof(u64), GFP_KERNEL);
	if (!values)
		goto err_free_strings;

	eth_get_ops(dev)->get_strings(dev, strings);
	eth_get_ops(dev)->get_stats(dev, values);

	off = 0;
	for (i = 0; i < nstats; i++) {
		printf("  %s: %llu\n", &strings[off], values[i]);
		off += ETH_GSTRING_LEN;
	};

	return CMD_RET_SUCCESS;

err_free_strings:
	kfree(strings);

	return CMD_RET_FAILURE;
}

static struct cmd_tbl cmd_net[] = {
	U_BOOT_CMD_MKENT(list, 1, 0, do_net_list, "", ""),
	U_BOOT_CMD_MKENT(stats, 2, 0, do_net_stats, "", ""),
};

static int do_net(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	struct cmd_tbl *cp;

	cp = find_cmd_tbl(argv[1], cmd_net, ARRAY_SIZE(cmd_net));

	/* Drop the net command */
	argc--;
	argv++;

	if (!cp || argc > cp->maxargs)
		return CMD_RET_USAGE;
	if (flag == CMD_FLAG_REPEAT && !cmd_is_repeatable(cp))
		return CMD_RET_SUCCESS;

	return cp->cmd(cmdtp, flag, argc, argv);
}

U_BOOT_CMD(
	net, 3, 1, do_net,
	"NET sub-system",
	"list - list available devices\n"
	"stats <device> - dump statistics for specified device\n"
);

#if defined(CONFIG_CMD_NCSI)
static int do_ncsi(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	if (!phy_interface_is_ncsi() || !ncsi_active()) {
		printf("Device not configured for NC-SI\n");
		return CMD_RET_FAILURE;
	}

	if (net_loop(NCSI) < 0)
		return CMD_RET_FAILURE;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	ncsi,	1,	1,	do_ncsi,
	"Configure attached NIC via NC-SI",
	""
);
#endif  /* CONFIG_CMD_NCSI */
