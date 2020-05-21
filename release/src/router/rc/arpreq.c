#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <bcmnvram.h>
#include <rc.h>
#include <shared.h>

#pragma pack (1)
struct globals {
	struct in_addr src;
	struct in_addr dst;
	struct sockaddr_ll me;
	struct sockaddr_ll he;
	int sock_fd;

	int count;
	unsigned last;
	unsigned timeout_us;
	unsigned start;

	unsigned sent;
	unsigned brd_sent;
	unsigned received;
	unsigned brd_recv;
	unsigned req_recv;
};
#pragma pack()

static char gbuf[sizeof(struct globals)];

#define G (*(struct globals*)&gbuf)
#define src        (G.src       )
#define dst        (G.dst       )
#define me         (G.me        )
#define he         (G.he        )
#define sock_fd    (G.sock_fd   )
#define count      (G.count     )
#define last       (G.last      )
#define timeout_us (G.timeout_us)
#define start      (G.start     )
#define sent       (G.sent      )
#define brd_sent   (G.brd_sent  )
#define received   (G.received  )
#define brd_recv   (G.brd_recv  )
#define req_recv   (G.req_recv  )
#define INIT_G() do { \
        count = -1; \
} while (0)

static int send_pack(struct in_addr *src_addr, struct in_addr *dst_addr, struct sockaddr_ll *ME, struct sockaddr_ll *HE)
{
	int err;
	unsigned char buf[256];
	struct arphdr *ah = (struct arphdr*) buf;
	unsigned char *p = (unsigned char*) (ah + 1);
#ifdef DEBUG
	int i;
#endif

	ah->ar_hrd = htons(ARPHRD_ETHER);
	ah->ar_pro = htons(ETH_P_IP);
	ah->ar_hln = ME->sll_halen;
	ah->ar_pln = 4;
	ah->ar_op = htons(ARPOP_REQUEST);
#ifdef DEBUG
	dbg("[%s] me ha_addr:(%d): ", __FUNCTION__, ah->ar_hln);
	for (i = 0; i < ah->ar_hln; ++i)
		dbg("[%x]", *((unsigned char*)&HE->sll_addr + i));
	dbg("\n");
#endif

	p = mempcpy(p, &ME->sll_addr, ah->ar_hln);
	p = mempcpy(p, src_addr, 4);
	p = mempcpy(p, &HE->sll_addr, ah->ar_hln);
	p = mempcpy(p, dst_addr, 4);

	err = sendto(sock_fd, buf, p - buf, 0, (struct sockaddr*)HE, sizeof(*HE));
#ifdef DEBUG
	dbg("[%s] send %d (%d)bytes:\n", __FUNCTION__, err, p-buf);
#endif

	return err;
}

int send_arpreq(void)
{
	char *device, *source, *target;
	struct ifreq ifr;

	source = nvram_safe_get("lan_ipaddr");
	target = nvram_safe_get("lan_gateway");
	device = nvram_safe_get("lan_ifname");

#ifdef DEBUG
	dbg("[%s]: source:[%s] taget:[%s], dev:[%s]\n", __FUNCTION__, source, target, device);
#endif

	sock_fd = socket(AF_PACKET, SOCK_DGRAM, 0);
	if (sock_fd < 0) {
		perror("sockfd");
		return -1;
	}
	else if (sock_fd < 3) {
		dbg("wierd sockfd(%d) !\n", sock_fd);
		close(sock_fd);
		return -1;
	}

	memset(&ifr, 0x0, sizeof(ifr));
	strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name) - 1);
	ioctl(sock_fd, SIOCGIFINDEX, (char*)&ifr);
	me.sll_ifindex = ifr.ifr_ifindex;
	ioctl(sock_fd, SIOCGIFFLAGS, (char*)&ifr);
	if (!(ifr.ifr_flags & IFF_UP)) {
#ifdef DEBUG
		dbg("[%s] %s is down\n", __FUNCTION__, device);
#endif
		close(sock_fd);
		return -1;
	}

	inet_aton(target, &dst);
	inet_aton(source, &src);

	if (src.s_addr) {
		struct sockaddr_in saddr;
		int probe_fd = socket(AF_INET, SOCK_DGRAM, 0);

		if (setsockopt(probe_fd, SOL_SOCKET, SO_BINDTODEVICE, device, strlen(device)+1) == -1)
			perror("BindToDevice");
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin_family = AF_INET;
		saddr.sin_addr = src;
		bind(probe_fd, (struct sockaddr*) &saddr, sizeof(saddr));
		close(probe_fd);
	}
	else
		dbg("no saddr !\n");

	me.sll_family = AF_PACKET;
	me.sll_protocol = htons(ETH_P_ARP);
	bind(sock_fd, (struct sockaddr*)&me, sizeof(me));

	socklen_t alen = sizeof(me);
	/* get hwaddr here */
	if (getsockname(sock_fd, (struct sockaddr*)&me, &alen) == -1) {
		perror("getsockname");
		close(sock_fd);
		return -1;
	}
	he = me;
	memset(he.sll_addr, -1, he.sll_halen);
#ifdef DEBUG
	dbg("[%s] ARPING to %s", __FUNCTION__, inet_ntoa(dst));
	dbg(" from %s via %s\n", inet_ntoa(src), device);
#endif

	send_pack(&src, &dst, &me, &he);

	close(sock_fd);
	return 0;
}
