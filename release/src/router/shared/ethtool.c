#include <linux/sockios.h>
#include <linux/ethtool.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <shared.h>
#include <shutils.h>

/**
 * Implement ETHTOOL_GLINK ioctl
 * @iface:	interface name
 * @return:
 *       1:	link-up
 * 	 0:	link-down
 * 	-1:	invalid parameter or @iface doesn't exist
 * 	-2:	open socket error
 * 	-3:	ioctl error
 */
int ethtool_glink(char *iface)
{
	int fd, err, ret = 0;
	struct ifreq ifr;
	struct ethtool_value eval;

	if (!iface_exist(iface))
		return -1;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return -2;

	memset(&eval, 0, sizeof(eval));
	memset(&ifr, 0, sizeof(ifr));
	eval.cmd = ETHTOOL_GLINK;
	ifr.ifr_data = (caddr_t) &eval;
	strcpy(ifr.ifr_name, iface);
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	close(fd);
	if (err) {
		dbg("%s: Can't link status of %s. errno %d (%s)\n",
			__func__, iface, errno, strerror(errno));
		return -3;
	}

	if (eval.data)
		ret = 1;

	return ret;
}

/**
 * Implement ETHTOOL_GSET ioctl
 * @iface:	interface name
 * @speed:	pointer to 32-bit unsigned integer
 * @duplex:	pointer to integer. (1: full-duplex, 0: half-duplex)
 * @return:
 * 	0:	success
 *     -1:	invalid parameter or @iface doesn't exist
 */
int ethtool_gset(char *iface, uint32_t *speed, int *duplex)
{
	int fd, err;
	struct ifreq ifr;
	struct ethtool_cmd ecmd;

	if (!iface_exist(iface))
		return -1;

	if (ethtool_glink(iface) <= 0) {
		if (speed)
			*speed = 0;
		if (duplex)
			*duplex = 0;
		return 0;
	}

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		return -2;

	memset(&ecmd, 0, sizeof(ecmd));
	memset(&ifr, 0, sizeof(ifr));
	ecmd.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t) &ecmd;
	strcpy(ifr.ifr_name, iface);
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	close(fd);
	if (err) {
		dbg("%s: Can't settings of %s. errno %d (%s)\n",
			__func__, iface, errno, strerror(errno));
		return -3;
	}

	if (speed)
		*speed = (ecmd.speed_hi << 16) | ecmd.speed;
	if (duplex)
		*duplex = (ecmd.duplex == DUPLEX_FULL)? 1 : 0;

	return 0;
}
