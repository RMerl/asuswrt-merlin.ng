/* pppoatm.c - pppd plugin to implement PPPoATM protocol.
 *
 * Copyright 2000 Mitchell Blank Jr.
 * Based in part on work from Jens Axboe and Paul Mackerras.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */
#include "pppd.h"
#include "pathnames.h"
#include "fsm.h" /* Needed for lcp.h to include cleanly */
#include "lcp.h"
#include <atm.h>

#include <bcm_local_kernel_include/linux/compiler.h>
#include <bcm_local_kernel_include/linux/atmdev.h> /* uses kernel include path */
#include <linux/atmppp.h>
#include <sys/stat.h>
#include <net/if.h>
#include <sys/ioctl.h>

#ifndef BRCM_CMS_BUILD
#include <unistd.h>
#include <string.h>
#endif

static struct sockaddr_atmpvc pvcaddr;
static char *qosstr = NULL;
static int pppoatm_accept = 0;
bool llc_encaps = 0;
bool vc_encaps = 0;
static int device_got_set = 0;
static int pppoatm_max_mtu, pppoatm_max_mru;

// brcm begin
struct channel pppoatm_channel;
static int fd_pppoa=0;

int setdevname_pppoatm(const char *cp);
//static int setdevname_pppoatm(const char *cp);
static void set_line_discipline_pppoatm(int fd);
// brcm end

static option_t pppoa_options[] = {
	{ "accept", o_bool, &pppoatm_accept,
	  "set PPPoATM socket to accept incoming connections", 1 },
	{ "llc-encaps", o_bool, &llc_encaps,
	  "use LLC encapsulation for PPPoATM", 1},
	{ "vc-encaps", o_bool, &vc_encaps,
	  "use VC multiplexing for PPPoATM (default)", 1},
	{ "qos", o_string, &qosstr,
	  "set QoS for PPPoATM connection", 1},
	{ NULL }
};

/* returns:
 *  -1 if there's a problem with setting the device
 *   0 if we can't parse "cp" as a valid name of a device
 *   1 if "cp" is a reasonable thing to name a device
 * Note that we don't actually open the device at this point
 * We do need to fill in:
 *   devnam: a string representation of the device
 *   devstat: a stat structure of the device.  In this case
 *     we're not opening a device, so we just make sure
 *     to set up S_ISCHR(devstat.st_mode) != 1, so we
 *     don't get confused that we're on stdin.
 */
// brcm begin
int setdevname_pppoatm(const char *cp)
//static int setdevname_pppoatm(const char *cp)
// brcm end
{
	struct sockaddr_atmpvc addr;
// brcm begin
   char *pvc;
//	extern struct stat devstat;
// brcm end

	if (device_got_set)
		return 0;
	info("PPPoATM setdevname_pppoatm");

// brcm begin
   if ((pvc = strchr(cp, '.')) == NULL)
      return 0;
// brcm end

	memset(&addr, 0, sizeof addr);
//	if (text2atm(cp, (struct sockaddr *) &addr, sizeof(addr),
//	    T2A_PVC | T2A_NAME) < 0)
//		return 0;

// brcm begin
	if (text2atm(pvc+1, (struct sockaddr *) &addr, sizeof(addr),
	    T2A_PVC | T2A_NAME) < 0)
		return 0;
   
	strncpy(devnam, cp, (pvc-cp));
   devnam[pvc-cp] = '\0';

//	if (!dev_set_ok())
//		return -1;
// brcm end

	memcpy(&pvcaddr, &addr, sizeof pvcaddr);
//	strlcpy(devnam, cp, sizeof devnam);

// brcm begin
//	devstat.st_mode = S_IFSOCK;
// brcm end
	// brcm begin
	the_channel = &pppoatm_channel;
	//modem = 0;
	lcp_wantoptions[0].neg_accompression = 0;
	lcp_allowoptions[0].neg_accompression = 0;
	lcp_wantoptions[0].neg_asyncmap = 0;
	lcp_allowoptions[0].neg_asyncmap = 0;
	lcp_wantoptions[0].neg_pcompression = 0;
	// brcm end

	info("PPPoATM setdevname_pppoatm - SUCCESS");
	device_got_set = 1;
	return 1;
}

#if 0 //brcm
static int setspeed_pppoatm(const char *cp)
{
	return 0;
}

static int options_for_pppoatm(void)
{
// brcm begin
//	return options_from_devfile(_PATH_ATMOPT, devnam);
// brcm end
}
#endif

#define pppoatm_overhead() (llc_encaps ? 6 : 2)

// brcm2
static void disconnect_pppoatm(int fd)
{
#if 1
   /* For 4.x xtm driver */
   struct atm_backend_ppp be;

   be.backend_num = ATM_BACKEND_PPP_BCM_DISCONN;
   if (ioctl(fd, ATM_SETBACKEND, &be) < 0)
      fatal("ioctl(ATM_SETBACKEND): %m");
#else
   /* For 3.x atm driver */
	unsigned char ppp_disconn[64];
        unsigned char * outp;

	outp = ppp_disconn;
	MAKEHEADER(outp, PPP_LCP);	// 2 bytes
	PUTCHAR(5, outp);		// TERMREQ==5 			// 1 byte
	PUTCHAR(2, outp);  		// id=02			// 1 byte
    PUTSHORT(4, outp);		// HEADERLEN==4 in fsm.h	// 2 byte
	
	write(fd, ppp_disconn+2, 6);
#endif
}

static void no_device_given_pppoatm(void)
{
	fatal("No vpi.vci specified");
}

static int open_device_pppoatm(void)
{
	int fd;
	struct atm_qos qos;
	int i = 0;

	if (!device_got_set)
		no_device_given_pppoatm();
	fd = socket(AF_ATMPVC, SOCK_DGRAM, 0);
	if (fd < 0)
		fatal("failed to create socket: %m");
	// brcm
	fd_pppoa = fd;
	memset(&qos, 0, sizeof qos);
	qos.txtp.traffic_class = qos.rxtp.traffic_class = ATM_UBR;
	/* TODO: support simplified QoS setting */
	if (qosstr != NULL)
		if (text2qos(qosstr, &qos, 0))
			fatal("Can't parse QoS: \"%s\"");
	qos.txtp.max_sdu = lcp_allowoptions[0].mru + pppoatm_overhead();
	qos.rxtp.max_sdu = lcp_wantoptions[0].mru + pppoatm_overhead();
	qos.aal = ATM_AAL5;
	if (setsockopt(fd, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0)
		fatal("setsockopt(SO_ATMQOS): %m");
	/* TODO: accept on SVCs... */
	if (connect(fd, (struct sockaddr *) &pvcaddr,
	    sizeof(struct sockaddr_atmpvc)))
		fatal("connect(%s): %m", devnam);
	pppoatm_max_mtu = lcp_allowoptions[0].mru;
	pppoatm_max_mru = lcp_wantoptions[0].mru;
// brcm begin
	set_line_discipline_pppoatm(fd);
// brcm end

	if (redisconn) {
    	    while (i++ < 5) {
    		usleep(300000);
    		disconnect_pppoatm(fd);
	    }
	    usleep(100000);
	}

	return fd;
}

#if 0 //brcm
static void post_open_setup_pppoatm(int fd)
{
	/* NOTHING */
}

static void pre_close_restore_pppoatm(int fd)
{
	/* NOTHING */
}
#endif

// brcm begin
static void close_device_pppoatm(void)
{
//    close(fd);
   if (fd_pppoa > 0) {
      struct atm_backend_ppp be;
      be.backend_num = ATM_BACKEND_PPP_BCM_CLOSE_DEV;
      if (ioctl(fd_pppoa, ATM_SETBACKEND, &be) < 0)
         fatal("ioctl(ATM_SETBACKEND): %m");
      close(fd_pppoa);
      fd_pppoa= -1;
   }
}
// brcm end

static void set_line_discipline_pppoatm(int fd)
{
	struct atm_backend_ppp be;
	be.backend_num = ATM_BACKEND_PPP_BCM;
	if (!llc_encaps)
		be.encaps = PPPOATM_ENCAPS_VC;
	else if (!vc_encaps)
		be.encaps = PPPOATM_ENCAPS_LLC;
	else
		be.encaps = PPPOATM_ENCAPS_AUTODETECT;
	if (ioctl(fd, ATM_SETBACKEND, &be) < 0)
		fatal("ioctl(ATM_SETBACKEND): %m");
}

#if 0 //brcm
static void reset_line_discipline_pppoatm(int fd)
{
	atm_backend_t be = ATM_BACKEND_RAW;
	/* 2.4 doesn't support this yet */
	(void) ioctl(fd, ATM_SETBACKEND, &be);
}
#endif

// brcm begin
static void send_config_pppoatm(int mtu, u_int32_t asyncmap,
	int pcomp, int accomp)
//static void send_config_pppoatm(int unit, int mtu, u_int32_t asyncmap,
//	int pcomp, int accomp)
// brcm end
{
	int sock;
	struct ifreq ifr;
	if (mtu > pppoatm_max_mtu)
		error("Couldn't increase MTU to %d", mtu);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		fatal("Couldn't create IP socket: %m");
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ifr.ifr_mtu = mtu;
	if (ioctl(sock, SIOCSIFMTU, (caddr_t) &ifr) < 0)
		fatal("ioctl(SIOCSIFMTU): %m");
	(void) close (sock);
}

// brcm begin
static void recv_config_pppoatm(int mru, u_int32_t asyncmap,
	int pcomp, int accomp)
//static void recv_config_pppoatm(int unit, int mru, u_int32_t asyncmap,
//	int pcomp, int accomp)
// brcm end
{
	if (mru > pppoatm_max_mru)
		error("Couldn't increase MRU to %d", mru);
}

#if 0 //brcm
static void set_xaccm_pppoatm(int unit, ext_accm accm)
{
	/* NOTHING */
}
#endif

// brcm begin
#if 0
void plugin_init(void)
{
#if _linux_
	extern int new_style_driver;	/* From sys-linux.c */
#endif
	static char *bad_options[] = {
		"noaccomp", "-ac",
		"default-asyncmap", "-am", "asyncmap", "-as", "escape",
		"receive-all",
		"crtscts", "-crtscts", "nocrtscts",
		"cdtrcts", "nocdtrcts",
		"xonxoff",
		"modem", "local", "sync",
		NULL };
#if _linux_
	if (!new_style_driver)
		fatal("Kernel doesn't support ppp_generic - "
		    "needed for PPPoATM");
#else
	fatal("No PPPoATM support on this OS");
#endif
	info("PPPoATM plugin_init");
	add_options(my_options);
	add_devname_class(setdevname_pppoatm);
	setspeed_hook = setspeed_pppoatm;
	options_for_device_hook = options_for_pppoatm;
	open_device_hook = open_device_pppoatm;
	post_open_setup_hook = post_open_setup_pppoatm;
	pre_close_restore_hook = pre_close_restore_pppoatm;
	no_device_given_hook = no_device_given_pppoatm;
	set_line_discipline_hook = set_line_discipline_pppoatm;
	reset_line_discipline_hook = reset_line_discipline_pppoatm;
	send_config_hook = send_config_pppoatm;
	recv_config_hook = recv_config_pppoatm;
	set_xaccm_hook = set_xaccm_pppoatm;
	{
		char **a;
		for (a = bad_options; *a != NULL; a++)
			remove_option(*a);
	}
	modem = 0;
	lcp_wantoptions[0].neg_accompression = 0;
	lcp_allowoptions[0].neg_accompression = 0;
	lcp_wantoptions[0].neg_asyncmap = 0;
	lcp_allowoptions[0].neg_asyncmap = 0;
	lcp_wantoptions[0].neg_pcompression = 0;
}
#endif
// brcm end

// brcm begin
struct channel pppoatm_channel = {
    options: pppoa_options,
    process_extra_options: NULL,
    check_options: NULL,
    connect: &open_device_pppoatm,
    disconnect: &close_device_pppoatm,
    establish_ppp: &generic_establish_ppp,
    disestablish_ppp: &generic_disestablish_ppp,
    send_config: &send_config_pppoatm,
    recv_config: &recv_config_pppoatm,
    close: NULL,
    cleanup: NULL
};
// brcm end
