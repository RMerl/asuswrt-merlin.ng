/* source: xio-tun.h */
/* Copyright Gerhard Rieger and contributors (see file CHANGES) */
/* Published under the GNU General Public License V.2, see file COPYING */

#ifndef __xio_tun_h_included
#define __xio_tun_h_included 1

extern const struct optdesc opt_tun_device;
extern const struct optdesc opt_tun_name;
extern const struct optdesc opt_tun_type;
extern const struct optdesc opt_iff_no_pi;
extern const struct optdesc opt_interface_addr;
extern const struct optdesc opt_interface_netmask;
extern const struct optdesc opt_iff_up;
extern const struct optdesc opt_iff_broadcast;
extern const struct optdesc opt_iff_debug;
extern const struct optdesc opt_iff_loopback;
extern const struct optdesc opt_iff_pointopoint;
extern const struct optdesc opt_iff_notrailers;
extern const struct optdesc opt_iff_running;
extern const struct optdesc opt_iff_noarp;
extern const struct optdesc opt_iff_promisc;
extern const struct optdesc opt_iff_allmulti;
extern const struct optdesc opt_iff_master;
extern const struct optdesc opt_iff_slave;
extern const struct optdesc opt_iff_multicast;
extern const struct optdesc opt_iff_portsel;
extern const struct optdesc opt_iff_automedia;
/*extern const struct optdesc opt_iff_dynamic;*/

extern const struct addrdesc xioaddr_tun;

#endif /* !defined(__xio_tun_h_included) */
