/* source: xio-tun.c */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

/* this file contains the source for opening addresses of tun/tap type */

#include "xiosysincludes.h"
#if WITH_TUN
#include "xioopen.h"

#include "xio-named.h"
#include "xio-socket.h"
#include "xio-ip.h"

#include "xio-tun.h"


static int xioopen_tun(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *fd, unsigned groups, int dummy1, int dummy2, int dummy3);

/****** TUN addresses ******/
const struct optdesc opt_tun_device    = { "tun-device",     NULL,      OPT_TUN_DEVICE,      GROUP_TUN,       PH_OPEN, TYPE_FILENAME, OFUNC_SPEC };
const struct optdesc opt_tun_name      = { "tun-name",       NULL,      OPT_TUN_NAME,        GROUP_INTERFACE, PH_FD,   TYPE_STRING,   OFUNC_SPEC };
const struct optdesc opt_tun_type      = { "tun-type",       NULL,      OPT_TUN_TYPE,        GROUP_INTERFACE, PH_FD,   TYPE_STRING,   OFUNC_SPEC };
const struct optdesc opt_iff_no_pi     = { "iff-no-pi",       "no-pi",       OPT_IFF_NO_PI,         GROUP_TUN,       PH_FD,   TYPE_BOOL,   OFUNC_SPEC };
/*0 const struct optdesc opt_interface_addr    = { "interface-addr",    "address", OPT_INTERFACE_ADDR,    GROUP_INTERFACE, PH_FD, TYPE_STRING,   OFUNC_SPEC };*/
/*0 const struct optdesc opt_interface_netmask = { "interface-netmask", "netmask", OPT_INTERFACE_NETMASK, GROUP_INTERFACE, PH_FD, TYPE_STRING,   OFUNC_SPEC };*/
const struct optdesc opt_iff_up          = { "iff-up",          "up",          OPT_IFF_UP,          GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_UP };
const struct optdesc opt_iff_broadcast   = { "iff-broadcast",   NULL,          OPT_IFF_BROADCAST,   GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_BROADCAST };
const struct optdesc opt_iff_debug       = { "iff-debug"    ,   NULL,          OPT_IFF_DEBUG,       GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_DEBUG };
const struct optdesc opt_iff_loopback    = { "iff-loopback" ,   "loopback",    OPT_IFF_LOOPBACK,    GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_LOOPBACK };
const struct optdesc opt_iff_pointopoint = { "iff-pointopoint", "pointopoint",OPT_IFF_POINTOPOINT, GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_POINTOPOINT };
const struct optdesc opt_iff_notrailers  = { "iff-notrailers",  "notrailers",  OPT_IFF_NOTRAILERS,  GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_NOTRAILERS };
const struct optdesc opt_iff_running     = { "iff-running",     "running",     OPT_IFF_RUNNING,     GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_RUNNING };
const struct optdesc opt_iff_noarp       = { "iff-noarp",       "noarp",       OPT_IFF_NOARP,       GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_NOARP };
const struct optdesc opt_iff_promisc     = { "iff-promisc",     "promisc",     OPT_IFF_PROMISC,     GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_PROMISC };
const struct optdesc opt_iff_allmulti    = { "iff-allmulti",    "allmulti",    OPT_IFF_ALLMULTI,    GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_ALLMULTI };
const struct optdesc opt_iff_master      = { "iff-master",      "master",      OPT_IFF_MASTER,      GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_MASTER };
const struct optdesc opt_iff_slave       = { "iff-slave",       "slave",       OPT_IFF_SLAVE,       GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_SLAVE };
const struct optdesc opt_iff_multicast   = { "iff-multicast",   NULL,          OPT_IFF_MULTICAST,   GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_MULTICAST };
const struct optdesc opt_iff_portsel     = { "iff-portsel",     "portsel",     OPT_IFF_PORTSEL,     GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_PORTSEL };
const struct optdesc opt_iff_automedia   = { "iff-automedia",   "automedia",   OPT_IFF_AUTOMEDIA,   GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(para.tun.iff_opts), IFF_AUTOMEDIA };
/*const struct optdesc opt_iff_dynamic   = { "iff-dynamic",     "dynamic",     OPT_IFF_DYNAMIC,     GROUP_INTERFACE, PH_FD,   TYPE_BOOL,     OFUNC_OFFSET_MASKS, XIO_OFFSETOF(para.tun.iff_opts), XIO_SIZEOF(short), IFF_DYNAMIC };*/
#if LATER
const struct optdesc opt_route           = { "route",           NULL,          OPT_ROUTE,           GROUP_INTERFACE, PH_INIT, TYPE_STRING,   OFUNC_SPEC };
#endif

const struct addrdesc xioaddr_tun    = { "tun",    3, xioopen_tun, GROUP_FD|GROUP_CHR|GROUP_NAMED|GROUP_OPEN|GROUP_TUN, 0, 0, 0 HELP("[:<ip-addr>/<bits>]") };
/* "if-name"=tun3
// "route"=address/netmask
// "ip6-route"=address/netmask
// "iff-broadcast"
// "iff-debug"
// "iff-promisc"
// see .../linux/if.h
*/


#if LATER
/* sub options for route option */
#define IFOPT_ROUTE 1
static const struct optdesc opt_route_tos = { "route", NULL, IFOPT_ROUTE, };
static const struct optname xio_route_options[] = {
   {"tos", &xio_route_tos }
} ;
#endif

static int xioopen_tun(int argc, const char *argv[], struct opt *opts, int xioflags, xiofile_t *xfd, unsigned groups, int dummy1, int dummy2, int dummy3) {
   char *tundevice = NULL;
   char *tunname = NULL, *tuntype = NULL;
   int pf = /*! PF_UNSPEC*/ PF_INET;
   struct xiorange network;
   bool no_pi = false;
   const char *namedargv[] = { "tun", NULL, NULL };
   int rw = (xioflags & XIO_ACCMODE);
   bool exists;
   struct ifreq ifr;
   int sockfd;
   char *ifaddr;
   int result;

   if (argc > 2 || argc < 0) {
      Error2("%s: wrong number of parameters (%d instead of 0 or 1)",
	     argv[0], argc-1);
   }

   if (retropt_string(opts, OPT_TUN_DEVICE, &tundevice) != 0) {
      tundevice = strdup("/dev/net/tun");
   }

   /*! socket option here? */
   retropt_socket_pf(opts, &pf);

   namedargv[1] = tundevice;
   /* open the tun cloning device */
   if ((result = _xioopen_named_early(2, namedargv, xfd, groups, &exists, opts)) < 0) {
      return result;
   }

   /*========================= the tunnel interface =========================*/
   Notice("creating tunnel network interface");
   if ((result = _xioopen_open(tundevice, rw, opts)) < 0)
      return result;
   xfd->stream.fd = result;

   /* prepare configuration of the new network interface */
   memset(&ifr, 0,sizeof(ifr));

   if (retropt_string(opts, OPT_TUN_NAME, &tunname) == 0) {
      strncpy(ifr.ifr_name, tunname, IFNAMSIZ);	/* ok */
      free(tunname);
   } else {
      ifr.ifr_name[0] = '\0';
   }

   ifr.ifr_flags = IFF_TUN;
   if (retropt_string(opts, OPT_TUN_TYPE, &tuntype) == 0) {
      if (!strcmp(tuntype, "tap")) {
	 ifr.ifr_flags = IFF_TAP;
      } else if (strcmp(tuntype, "tun")) {
	 Error1("unknown tun-type \"%s\"", tuntype);
      }
   }

   if (retropt_bool(opts, OPT_IFF_NO_PI, &no_pi) == 0) {
      if (no_pi) {
	 ifr.ifr_flags |= IFF_NO_PI;
#if 0 /* not neccessary for now */
      } else {
	 ifr.ifr_flags &= ~IFF_NO_PI;
#endif
      }
   }

   if (Ioctl(xfd->stream.fd, TUNSETIFF, &ifr) < 0) {
      Error3("ioctl(%d, TUNSETIFF, {\"%s\"}: %s",
	     xfd->stream.fd, ifr.ifr_name, strerror(errno));
      Close(xfd->stream.fd);
   }

   /*===================== setting interface properties =====================*/

   /* we seem to need a socket for manipulating the interface */
   if ((sockfd = Socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
      Error1("socket(PF_INET, SOCK_DGRAM, 0): %s", strerror(errno));
      sockfd = xfd->stream.fd;	/* desparate fallback attempt */
   }

   /*--------------------- setting interface address and netmask ------------*/
   if (argc == 2) {
       if ((ifaddr = strdup(argv[1])) == NULL) {
          Error1("strdup(\"%s\"): out of memory", argv[1]);
          return STAT_RETRYLATER;
       }
       if ((result = xioparsenetwork(ifaddr, pf, &network)) != STAT_OK) {
          /*! recover */
          return result;
       }
       socket_init(pf, (union sockaddr_union *)&ifr.ifr_addr);
       ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr =
          network.netaddr.ip4.sin_addr;
       if (Ioctl(sockfd, SIOCSIFADDR, &ifr) < 0) {
          Error4("ioctl(%d, SIOCSIFADDR, {\"%s\", \"%s\"}: %s",
             sockfd, ifr.ifr_name, ifaddr, strerror(errno));
       }
       ((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr =
          network.netmask.ip4.sin_addr;
       if (Ioctl(sockfd, SIOCSIFNETMASK, &ifr) < 0) {
          Error4("ioctl(%d, SIOCSIFNETMASK, {\"0x%08u\", \"%s\"}, %s",
             sockfd, ((struct sockaddr_in *)&ifr.ifr_netmask)->sin_addr.s_addr,
             ifaddr, strerror(errno));
       }
       free(ifaddr);
   }
   /*--------------------- setting interface flags --------------------------*/
   applyopts_single(&xfd->stream, opts, PH_FD);

   if (Ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
      Error3("ioctl(%d, SIOCGIFFLAGS, {\"%s\"}: %s",
	     sockfd, ifr.ifr_name, strerror(errno));
   }
   Debug2("\"%s\": system set flags: 0x%hx", ifr.ifr_name, ifr.ifr_flags);
   ifr.ifr_flags |= xfd->stream.para.tun.iff_opts[0];
   ifr.ifr_flags &= ~xfd->stream.para.tun.iff_opts[1];
   Debug2("\"%s\": xio merged flags: 0x%hx", ifr.ifr_name, ifr.ifr_flags);
   if (Ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0) {
      Error4("ioctl(%d, SIOCSIFFLAGS, {\"%s\", %hd}: %s",
	     sockfd, ifr.ifr_name, ifr.ifr_flags, strerror(errno));
   }
   ifr.ifr_flags = 0;
   if (Ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
      Error3("ioctl(%d, SIOCGIFFLAGS, {\"%s\"}: %s",
	     sockfd, ifr.ifr_name, strerror(errno));
   }
   Debug2("\"%s\": resulting flags: 0x%hx", ifr.ifr_name, ifr.ifr_flags);


#if LATER
   applyopts_named(tundevice, opts, PH_FD);
#endif
   applyopts(xfd->stream.fd, opts, PH_FD);
   applyopts_cloexec(xfd->stream.fd, opts);

   applyopts_fchown(xfd->stream.fd, opts);

   if ((result = _xio_openlate(&xfd->stream, opts)) < 0)
      return result;

   return 0;
}

#endif /* WITH_TUN */
