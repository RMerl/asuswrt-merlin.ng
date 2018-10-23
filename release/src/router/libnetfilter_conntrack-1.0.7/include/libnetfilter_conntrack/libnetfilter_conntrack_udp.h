#ifndef _LIBNETFILTER_CONNTRACK_UDP_H_
#define _LIBNETFILTER_CONNTRACK_UDP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* WARNING: do not use these flags in your new applications, they are obsolete
 * and we keep them here to avoid breaking backward compatibility. */
enum udp_flags {
	UDP_ORIG_SPORT_BIT = 0,
	UDP_ORIG_SPORT = (1 << UDP_ORIG_SPORT_BIT),

	UDP_ORIG_DPORT_BIT = 1,
	UDP_ORIG_DPORT = (1 << UDP_ORIG_DPORT_BIT),

	UDP_REPL_SPORT_BIT = 2,
	UDP_REPL_SPORT = (1 << UDP_REPL_SPORT_BIT),

	UDP_REPL_DPORT_BIT = 3,
	UDP_REPL_DPORT = (1 << UDP_REPL_DPORT_BIT),

	UDP_MASK_SPORT_BIT = 4,
	UDP_MASK_SPORT = (1 << UDP_MASK_SPORT_BIT),

	UDP_MASK_DPORT_BIT = 5,
	UDP_MASK_DPORT = (1 << UDP_MASK_DPORT_BIT),

	UDP_EXPTUPLE_SPORT_BIT = 6,
	UDP_EXPTUPLE_SPORT = (1 << UDP_EXPTUPLE_SPORT_BIT),

	UDP_EXPTUPLE_DPORT_BIT = 7,
	UDP_EXPTUPLE_DPORT = (1 << UDP_EXPTUPLE_DPORT_BIT)
};

#ifdef __cplusplus
}
#endif

#endif
