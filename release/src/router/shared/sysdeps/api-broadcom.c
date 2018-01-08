#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bcmnvram.h>
#include <bcmdevs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <wlutils.h>
#include <linux_gpio.h>
#include <etioctl.h>
#include "utils.h"
#include "shutils.h"
#include "shared.h"
#include <trxhdr.h>
#include <bcmutils.h>
#include <bcmendian.h>

#ifdef RTCONFIG_QTN
#include "web-qtn.h"
#endif
#ifdef HND_ROUTER
#include <linux/mii.h>
//#include "bcmnet.h"
//#include "bcm/bcmswapitypes.h"
#include "ethctl.h"
#include "ethswctl.h"
#include "ethswctl_api.h"
#include "bcm/bcmswapistat.h"
#include "boardparms.h"
#include <asm/byteorder.h>
#include <board.h>
#endif

uint32_t gpio_dir(uint32_t gpio, int dir)
{
	/* FIXME
	return bcmgpio_connect(gpio, dir);
	 */

	return 0;
}

#define swapportstatus(x) \
{ \
    unsigned int data = *(unsigned int*)&(x); \
    data = ((data & 0x000c0000) >> 18) |    \
           ((data & 0x00030000) >> 14) |    \
           ((data & 0x0000c000) >> 10) |    \
           ((data & 0x00003000) >>  6) |    \
	   ((data & 0x00000c00) >>  2);     \
    *(unsigned int*)&(x) = data;            \
}

extern uint32_t gpio_read(void);
extern void gpio_write(uint32_t bitvalue, int en);

uint32_t get_gpio(uint32_t gpio)
{
#ifdef HND_ROUTER
	int board_fp = open("/dev/brcmboard", O_RDWR);
	int active_low = _gpio_active_low(gpio & 0xff);
	BOARD_IOCTL_PARMS ioctl_parms = {0};

	if (board_fp <= 0) {
		printf("Open /dev/brcmboard failed!\n");
		return -1;
	}
	if (active_low < 0) {
		printf("invalid gpionr!get(%d)\n", gpio);
		dump_ledtable();
		close(board_fp);
		return -1;
	}

	ioctl_parms.strLen = gpio | (active_low ? BP_ACTIVE_LOW : 0);

	if (ioctl(board_fp, BOARD_IOCTL_GET_GPIO, &ioctl_parms) < 0)
		printf("\nhnd iotcl fail!\n");
	//printf("\nhnd get_gpio: %04x\n", ioctl_parms.offset);

	close(board_fp);
	return ioctl_parms.offset;
#else
	uint32_t bit_value;
	uint32_t bit_mask;

	bit_mask = 1 << gpio;
	bit_value = gpio_read()&bit_mask;

	return bit_value == 0 ? 0 : 1;
#endif
}


uint32_t set_gpio(uint32_t gpio, uint32_t value)
{
#ifdef HND_ROUTER
#ifndef LEGACY_LED
	char ledpath[48];
	int active_low = _gpio_active_low(gpio & 0xff);
	int ledfd;

	if (active_low < 0) {
		printf("invalid gpionr!set(%d)\n", gpio);
		dump_ledtable();
		return -1;
	}
	sprintf(ledpath, "/sys/class/leds/%d/brightness", gpio);
	ledfd = open(ledpath, O_RDWR);
	if (ledfd <=0 ) {
		printf("\nopen ledpath %s failed !\n", ledpath);
		return -1;
	}

	write(ledfd, active_low?(!value?"255":"0"):(!value?"0":"255"), active_low?(!value?3:1):(!value?1:3));
	close(ledfd);
	return 0;
#else
	int board_fp = open("/dev/brcmboard", O_RDWR);
	int active_low = _gpio_active_low(gpio & 0xff);
	BOARD_IOCTL_PARMS ioctl_parms = {0};

	if (board_fp <= 0) {
		printf("Open /dev/brcmboard failed!\n");
		return -1;
	}
	if (active_low < 0) {
		printf("invalid gpionr!\n");
		dump_ledtable();
		close(board_fp);
		return -1;
	}

	ioctl_parms.strLen = gpio & 0xff | (active_low ? BP_ACTIVE_LOW : 0);
	ioctl_parms.offset = (active_low?!value:value) & 0x3;

	if (ioctl(board_fp, BOARD_IOCTL_SET_GPIO, &ioctl_parms) < 0)
		printf("\nhnd iotcl fail!\n");

	close(board_fp);
	return 0;
#endif
#else // HND_ROUTER
	gpio_write(gpio, value);
#endif
	return 0;
}

#ifdef RTCONFIG_BCMFA
int get_fa_rev(void)
{
	int fd, ret;
	unsigned int rev;
	struct ifreq ifr;
	et_var_t var;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) goto skip;

	rev = 0;
	var.set = 0;
	var.cmd = IOV_FA_REV;
	var.buf = &rev;
	var.len = sizeof(rev);

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (caddr_t) &var;

	ret = ioctl(fd, SIOCSETGETVAR, (caddr_t)&ifr);
	close(fd);
	if (ret < 0)
		goto skip;

	return rev;

skip:
	return 0;
}

int get_fa_dump(void)
{
	int fd, rev, ret;
	struct ifreq ifr;
	et_var_t var;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) goto skip;

	rev = 0;
	var.set = 0;
	var.cmd = IOV_FA_DUMP;
	var.buf = &rev;
	var.len = sizeof(rev);

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, "eth0");
	ifr.ifr_data = (caddr_t) &var;

	ret = ioctl(fd, SIOCSETGETVAR, (caddr_t)&ifr);
	close(fd);
	if (ret < 0)
		goto skip;

	return rev;

skip:
	return 0;
}

#endif

int get_switch_model(void)
{
#ifdef BCM5301X
	return SWITCH_BCM5301x;
#elif defined(HND_ROUTER)
	return SWITCH_BCM5301x_EX;
#else
	int fd, devid, ret;
	struct ifreq ifr;
	et_var_t var;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) goto skip;

	devid = 0;
	var.set = 0;
	var.cmd = IOV_ET_ROBO_DEVID;
	var.buf = &devid;
	var.len = sizeof(devid);

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, "eth0"); // is it always the same?
	ifr.ifr_data = (caddr_t) &var;

	ret = ioctl(fd, SIOCSETGETVAR, (caddr_t)&ifr);
	close(fd);
	if (ret < 0)
		goto skip;
	if (devid == 0x25)
		return SWITCH_BCM5325;
	else if (devid == 0x3115)
		return SWITCH_BCM53115;
	else if (devid == 0x3125)
		return SWITCH_BCM53125;
	else if ((devid & 0xfffffff0) == 0x53010)
		return SWITCH_BCM5301x;

skip:
	return SWITCH_UNKNOWN;
#endif
}

int robo_ioctl(int fd, int write, int page, int reg, uint32_t *value)
{
	static int __ioctl_args[2] = { SIOCGETCROBORD, SIOCSETCROBOWR };
	struct ifreq ifr;
	int ret, vecarg[4];

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, "eth0"); // is it always the same?
	ifr.ifr_data = (caddr_t) vecarg;

	vecarg[0] = (page << 16) | reg;
#if defined(BCM5301X) || defined(RTAC1200G) || defined(RTAC1200GP)
	vecarg[1] = 0;
	vecarg[2] = *value;
#else
	vecarg[1] = *value;
#endif
	ret = ioctl(fd, __ioctl_args[write], (caddr_t)&ifr);

#if defined(BCM5301X) || defined(RTAC1200G) || defined(RTAC1200GP)
	*value = vecarg[2];
#else
	*value = vecarg[1];
#endif

	return ret;
}

int phy_ioctl(int fd, int write, int phy, int reg, uint32_t *value)
{
#ifndef BCM5301X
#if defined(HND_ROUTER)
	return hnd_ethswctl(REGACCESS, 0x1000 | phy << 8, 2, 1, *value);
#endif
	static int __ioctl_args[2] = { SIOCGETCPHYRD2, SIOCSETCPHYWR2 };
	struct ifreq ifr;
	int ret, vecarg[2];

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, "eth0"); // is it always the same?
	ifr.ifr_data = (caddr_t) vecarg;

	vecarg[0] = (phy << 16) | reg;
	vecarg[1] = *value;
	ret = ioctl(fd, __ioctl_args[write], (caddr_t)&ifr);

	*value = vecarg[1];

	return ret;
#else
	return robo_ioctl(fd, write, 0x10 + phy, reg, value);
#endif
}

#ifdef HND_ROUTER
static inline int ethswctl_init(struct ifreq *p_ifr)
{
    int skfd;

    /* Open a basic socket */
    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket open error\n");
        return -1;
    }

    /* Get the name -> if_index mapping for ethswctl */
    strcpy(p_ifr->ifr_name, "bcmsw");
    if (ioctl(skfd, SIOCGIFINDEX, p_ifr) < 0 ) {
        strcpy(p_ifr->ifr_name, "eth0");
        if (ioctl(skfd, SIOCGIFINDEX, p_ifr) < 0 ) {
            close(skfd);
            printf("neither bcmsw nor eth0 exist\n");
            return -1;
        }
    }

    return skfd;
}

static int et_dev_subports_query(int skfd, struct ifreq *ifr)
{
	int port_list = 0;

	ifr->ifr_data = (char*)&port_list;
	if (ioctl(skfd, SIOCGQUERYNUMPORTS, ifr) < 0) {
		fprintf(stderr, "Error: Interface %s ioctl SIOCGQUERYNUMPORTS error!\n", ifr->ifr_name);
		return -1;
	}
	return port_list;;
}

static int get_bit_count(int i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static int et_get_phyid2(int skfd, struct ifreq *ifr, int sub_port)
{
	unsigned long phy_id;
	struct mii_ioctl_data *mii = (void *)&ifr->ifr_data;

	mii->val_in = sub_port;

	if (ioctl(skfd, SIOCGMIIPHY, ifr) < 0)
		return -1;

	phy_id = MII_IOCTL_2_PHYID(mii);
	/*
	* returned phy id carries mii->val_out flags if phy is
	* internal/external phy/phy on ext switch.
	* we save it in higher byte to pass to kernel when
	* phy is accessed.
	*/
	return phy_id;
}

static int et_get_phyid(int skfd, struct ifreq *ifr, int sub_port)
{
	int sub_port_map;
#define MAX_SUB_PORT_BITS (sizeof(sub_port_map)*8)

	if ((sub_port_map = et_dev_subports_query(skfd, ifr)) < 0) {
		return -1;
	}

	if (sub_port_map > 0) {
		if (sub_port == -1) {
			if (get_bit_count(sub_port_map) > 1) {
				fprintf(stderr, "Error: Interface %s has sub ports, please specified one of port map: 0x%x\n",
				ifr->ifr_name, sub_port_map);
				return -1;
			}
			else if (get_bit_count(sub_port_map) == 1) {
				// get bit position
				for(sub_port = 0; sub_port < MAX_SUB_PORT_BITS; sub_port++) {
					if ((sub_port_map & (1 << sub_port)))
					break;
				}
			}
		}

		if ((sub_port_map & (1 << sub_port)) == 0) {
			fprintf(stderr, "Specified SubPort %d is not interface %s's member port with map %x\n",
				sub_port, ifr->ifr_name, sub_port_map);
			return -1;
		}
	} else {
		if (sub_port != -1) {
			fprintf(stderr, "Interface %s has no sub port\n", ifr->ifr_name);
			return -1;
		}
	}

	return et_get_phyid2(skfd, ifr, sub_port);
}

int mdio_read(int skfd, struct ifreq *ifr, int phy_id, int location)
{
	struct mii_ioctl_data *mii = (void *)&ifr->ifr_data;

	PHYID_2_MII_IOCTL(phy_id, mii);
	mii->reg_num = location;
	if (ioctl(skfd, SIOCGMIIREG, ifr) < 0) {
		fprintf(stderr, "SIOCGMIIREG on %s failed: %s\n", ifr->ifr_name,
		strerror(errno));
		return 0;
	}
	return mii->val_out;
}

static void mdio_write(int skfd, struct ifreq *ifr, int phy_id, int location, int value)
{
	struct mii_ioctl_data *mii = (void *)&ifr->ifr_data;

	PHYID_2_MII_IOCTL(phy_id, mii);
	mii->reg_num = location;
	mii->val_in = value;

	if (ioctl(skfd, SIOCSMIIREG, ifr) < 0) {
		fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr->ifr_name,
			strerror(errno));
	}
}

static int ethctl_get_link_status(char *ifname)
{
	int skfd=0, err, bmsr;
	struct ethswctl_data ifdata;
	struct ifreq ifr;
	int phy_id = 0, sub_port = -1;

	if ( strstr(ifname, "eth") == ifname ||
	     strstr(ifname, "epon") == ifname) {
		strcpy(ifr.ifr_name, ifname);
	} else {
		fprintf(stderr, "invalid interface name %s\n", ifname);
		goto error;
	}

	/* Open a basic socket */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("ethctl: socket open error\n");
		return -1;
	}

	/* Get the name -> if_index mapping for ethctl */
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, SIOCGIFINDEX, &ifr) < 0 ) {
		fprintf(stderr, "No %s interface exist\n", ifr.ifr_name);
		goto error;
	}

	if ((phy_id = et_get_phyid(skfd, &ifr, sub_port)) == -1)
		goto error;

	if (ETHCTL_GET_FLAG_FROM_PHYID(phy_id) & ETHCTL_FLAG_ACCESS_SERDES) {
		ifr.ifr_data = (void*) &ifdata;
		ifdata.op = ETHSWPHYMODE;
		ifdata.type = TYPE_GET;
		ifdata.addressing_flag = ETHSW_ADDRESSING_DEV;
		if (sub_port != -1) {
			ifdata.sub_unit = -1; // Set sub_unit to -1 so that main unit of dev will be used
			ifdata.sub_port = sub_port;
			ifdata.addressing_flag |= ETHSW_ADDRESSING_SUBPORT;
		}

		if ((err = ioctl(skfd, SIOCETHSWCTLOPS, ifr))) {
			fprintf(stderr, "ioctl command return error %d!\n", err);
			goto error;
		}

		close(skfd);
		return (ifdata.speed == 0) ? 0 : 1;
	}

	bmsr = mdio_read(skfd, &ifr, phy_id, MII_BMSR);
	if (bmsr == 0x0000) {
		fprintf(stderr, "No MII transceiver present!.\n");
		goto error;
	}
	//printf("Link is %s\n", (bmsr & BMSR_LSTATUS) ? "up" : "down");

	close(skfd);
	return (bmsr & BMSR_LSTATUS) ? 1 : 0;
error:
	if (skfd) close(skfd);
	return -1;
}

#define _MB 0x1
#define _GB 0x2
static int ethctl_get_link_speed(char *ifname)
{
	int skfd=0, err;
	struct ethswctl_data ifdata;
	struct ifreq ifr;
	int phy_id = 0, sub_port = -1;
	int bmcr, bmsr, gig_ctrl, gig_status, v16;

	if ( strstr(ifname, "eth") == ifname ||
	     strstr(ifname, "epon") == ifname) {
		strcpy(ifr.ifr_name, ifname);
	} else {
		fprintf(stderr, "invalid interface name %s\n", ifname);
		goto error;
	}

	/* Open a basic socket */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("ethctl: socket open error\n");
		return -1;
	}

	/* Get the name -> if_index mapping for ethctl */
	strcpy(ifr.ifr_name, ifname);
	if (ioctl(skfd, SIOCGIFINDEX, &ifr) < 0 ) {
		printf("No %s interface exist\n", ifr.ifr_name);
		goto error;
	}

	if ((phy_id = et_get_phyid(skfd, &ifr, sub_port)) == -1)
		goto error;

	if (ETHCTL_GET_FLAG_FROM_PHYID(phy_id) & ETHCTL_FLAG_ACCESS_SERDES) {
		ifr.ifr_data = (void*) &ifdata;
		ifdata.op = ETHSWPHYMODE;
		ifdata.type = TYPE_GET;
		ifdata.addressing_flag = ETHSW_ADDRESSING_DEV;
		if (sub_port != -1) {
			ifdata.sub_unit = -1; // Set sub_unit to -1 so that main unit of dev will be used
			ifdata.sub_port = sub_port;
			ifdata.addressing_flag |= ETHSW_ADDRESSING_SUBPORT;
		}

		if ((err = ioctl(skfd, SIOCETHSWCTLOPS, ifr))) {
			fprintf(stderr, "ioctl command return error %d!\n", err);
			goto error;;
		}

		close(skfd);
		return (ifdata.speed >= 1000) ? _GB : _MB;
	}

	bmsr = mdio_read(skfd, &ifr, phy_id, MII_BMSR);
	bmcr = mdio_read(skfd, &ifr, phy_id, MII_BMCR);
	if (bmcr == 0xffff ||  bmsr == 0x0000) {
		fprintf(stderr, "No MII transceiver present!.\n");
		goto error;
	}

	if (!(bmsr & BMSR_LSTATUS)) {
		fprintf(stderr, "Link is down!.\n");
		goto error;
	}

	if (bmcr & BMCR_ANENABLE) {
		gig_ctrl = mdio_read(skfd, &ifr, phy_id, MII_CTRL1000);
		// check ethernet@wirspeed only for PHY support 1G
		if (gig_ctrl & ADVERTISE_1000FULL || gig_ctrl & ADVERTISE_1000HALF) {
			// check if ethernet@wirespeed is enabled, reg 0x18, shodow 0b'111, bit4
			mdio_write(skfd, &ifr, phy_id, 0x18, 0x7007);
			v16 = mdio_read(skfd, &ifr, phy_id, 0x18);
			if (v16 & 0x0010) {
				// get link speed from ASR if ethernet@wirespeed is enabled
				v16 = mdio_read(skfd, &ifr, phy_id, 0x19);
#define MII_ASR_1000(r) (((r & 0x0700) == 0x0700) || ((r & 0x0700) == 0x0600))
#define MII_ASR_100(r)  (((r & 0x0700) == 0x0500) || ((r & 0x0700) == 0x0300))
#define MII_ASR_10(r)   (((r & 0x0700) == 0x0200) || ((r & 0x0700) == 0x0100))
				close(skfd);
				return MII_ASR_1000(v16) ? _GB : (MII_ASR_100(v16) || MII_ASR_10(v16)) ? _MB : -1;
			}
		}

		gig_status = mdio_read(skfd, &ifr, phy_id, MII_STAT1000);
		close(skfd);
		if (((gig_ctrl & ADVERTISE_1000FULL) && (gig_status & LPA_1000FULL)) ||
		    ((gig_ctrl & ADVERTISE_1000HALF) && (gig_status & LPA_1000HALF))) {
			close(skfd);
			return _GB;
		}
		else {
			return _MB;
		}
	}
	else {
		close(skfd);
		return (bmcr & BMCR_SPEED1000) ? _GB : _MB;
	}

error:
	if (skfd) close(skfd);
	return -1;
}

int bcm_reg_read_X(int unit, unsigned int addr, char* data, int len)
{
    int skfd, err = 0;
    struct ifreq ifr;
    struct ethswctl_data ifdata;
    struct ethswctl_data *e = &ifdata;

    if ((skfd=ethswctl_init(&ifr)) < 0) {
        printf("ethswctl_init failed. \n");
        return skfd;
    }
    ifr.ifr_data = (char *)&ifdata;

    e->op = ETHSWREGACCESS;
    e->type = TYPE_GET;
    e->offset = addr;
    e->length = len;
    e->unit = unit;

    if ((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
        printf("ioctl command return error!\n");
        goto out;
    }

    memcpy(data, e->data, len);

out:
    close(skfd);
    return err;
}

int bcm_reg_write_X(int unit, unsigned int addr, char* data, int len)
{
    int skfd, err = 0;
    struct ifreq ifr;
    struct ethswctl_data ifdata;
    struct ethswctl_data *e = &ifdata;

    if ((skfd=ethswctl_init(&ifr)) < 0) {
        printf("ethswctl_init failed. \n");
        return skfd;
    }
    ifr.ifr_data = (char *)&ifdata;

    e->op = ETHSWREGACCESS;
    e->type = TYPE_SET;
    e->offset = addr;
    e->length = len;
    e->unit = unit;
    memcpy(e->data, data, len);

    if ((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
        printf("ioctl command return error!\n");
        goto out;
    }

out:
    close(skfd);
    return err;
}

int bcm_pseudo_mdio_read(unsigned int addr, char* data, int len)
{
    int skfd, err = 0;
    struct ifreq ifr;
    struct ethswctl_data ifdata;
    struct ethswctl_data *e = &ifdata;

    if ((skfd=ethswctl_init(&ifr)) < 0) {
        printf("ethswctl_init failed. \n");
        return skfd;
    }
    ifr.ifr_data = (char *)&ifdata;

    e->op = ETHSWPSEUDOMDIOACCESS;
    e->type = TYPE_GET;
    e->offset = addr;
    e->length = len;

    if ((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
        printf("ioctl command return error!\n");
        goto out;
    }

    memcpy(data, e->data, len);

out:
    close(skfd);
    return err;
}

int bcm_pseudo_mdio_write(unsigned int addr, char* data, int len)
{
    int skfd, err = 0;
    struct ifreq ifr;
    struct ethswctl_data ifdata;
    struct ethswctl_data *e = &ifdata;

    if ((skfd=ethswctl_init(&ifr)) < 0) {
        printf("ethswctl_init failed. \n");
        return skfd;
    }
    ifr.ifr_data = (char *)&ifdata;

    e->op = ETHSWPSEUDOMDIOACCESS;
    e->type = TYPE_SET;
    e->offset = addr;
    e->length = len;
    memcpy(e->data, data, sizeof(e->data));

    if ((err = ioctl(skfd, SIOCETHSWCTLOPS, &ifr))) {
        printf("ioctl command return error!\n");
        goto out;
    }

out:
    close(skfd);
    return err;
}

int hnd_ethswctl(ecmd_t act, unsigned int val, int len, int wr, unsigned long long regdata)
{
	unsigned long long data64 = 0;
	int ret_val = 0, i;
	unsigned char data[8];

	switch(act) {
		case REGACCESS:
			if (wr) {
				data64 = cpu_to_le64(regdata);
				//_dprintf("w Data: %08x %08x \n", (unsigned int)(data64 >> 32), (unsigned int)(data64) );
				ret_val = bcm_reg_write_X(1, val, (char *)&data64, len);
			} else {
				ret_val = bcm_reg_read_X(1, val, (char *)&data64, len);
				data64 = le64_to_cpu(data64);
				//_dprintf("Data: %08x %08x \n", (unsigned int)(data64 >> 32), (unsigned int)(data64) );
				return data64;
			}
			break;
		case PMDIOACCESS:
			for (i = 0; i < 8; i++) {
				data[i] = (unsigned char) (*( ((char *)&regdata) + 7-i));
			}
			if (wr) {
				//_dprintf("\npw data\n");
				ret_val = bcm_pseudo_mdio_write(val, (char*)data, len);
			} else {
				ret_val = bcm_pseudo_mdio_read(val, (char*)data, len);
				//_dprintf("pr Data: %02x%02x%02x%02x %02x%02x%02x%02x\n",
				//data[7], data[6], data[5], data[4], data[3], data[2], data[1], data[0]);
				memcpy(&data64, data, 8);
				return data64;
			}
			break;
	}

	return ret_val;
}

uint32_t hnd_get_phy_status(int port, int offs, unsigned int regv, unsigned int pmdv)
{
	if (port == 7 ) {			// wan port
		return ethctl_get_link_status("eth0");
	} else if (!offs || (port-offs < 0)) {	// main switch
		return regv & (1<<port) ? 1 : 0;
	} else {				// externai switch
		return pmdv & (1<<(port-offs)) ? 1 : 0;
	}
}

uint32_t hnd_get_phy_speed(int port, int offs, unsigned int regv, unsigned int pmdv)
{
	int val = 0;
	if (port == 7) {			// wan port
		return ethctl_get_link_speed("eth0");
	}
	else if (!offs || (port-offs < 0)) {	// main switch
		val = regv & (0x0003<<(port*2));
		return val>>(port*2);
	} else {				// externai switch
		val = pmdv & (0x0003<<((port-offs)*2));
		return val>>((port-offs)*2);
	}
}
#endif

// !0: connected
//  0: disconnected
uint32_t get_phy_status(uint32_t portmask)
{
	int fd, model;
	uint32_t value, mask = 0;
#ifndef BCM5301X
	int i;
#endif

	model = get_switch();
	if (model == SWITCH_UNKNOWN) return 0;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) return 0;

	switch (model) {
#ifndef BCM5301X
	case SWITCH_BCM53125:
#ifdef RTCONFIG_LANWAN_LED
		/* N15U can't read link status from phy sometimes */
		if (get_model() == MODEL_RTN15U)
			goto case_SWITCH_ROBORD;
		/* fall through */
#endif
	case SWITCH_BCM53115:
	case SWITCH_BCM5325:
		for (i = 0; i < 5 && (portmask >> i); i++) {
			if ((portmask & (1U << i)) == 0)
				continue;

			if (phy_ioctl(fd, 0, i, 0x01, &value) < 0)
				continue;
			/* link is down, but negotiation has started
			 * read register again, use previous value, if failed */
			if ((value & 0x22) == 0x20)
				phy_ioctl(fd, 0, i, 0x01, &value);

			if (value & (1U << 2))
				mask |= (1U << i);
		}
		break;
#ifdef RTCONFIG_LANWAN_LED
	case_SWITCH_ROBORD:
		/* fall through */
#endif
#endif
	case SWITCH_BCM5301x:
		if (robo_ioctl(fd, 0, 0x01, 0x00, &value) < 0)
			_dprintf("et ioctl SIOCGETCROBORD failed!\n");
		mask = value & portmask & 0x1f;
		break;
	}
	close(fd);

	//_dprintf("# get_phy_status %x %x\n", mask, portmask);

	return mask;
}

// 2bit per port (0-4(5)*2 shift)
// 0: 10 Mbps
// 1: 100 Mbps
// 2: 1000 Mbps
uint32_t get_phy_speed(uint32_t portmask)
{
	int fd, model;
	uint32_t value, mask = 0;

	model = get_switch();
	if (model == SWITCH_UNKNOWN) return 0;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) return 0;

	if (robo_ioctl(fd, 0, 0x01, 0x04, &value) < 0)
		value = 0;
	close(fd);

	switch (model) {
#ifndef BCM5301X
	case SWITCH_BCM5325:
		/* 5325E/535x, 1bit: 0=10 Mbps, 1=100Mbps */
		for (mask = 0; value & 0x1f; value >>= 1) {
			mask |= (value & 0x01);
			mask <<= 2;
		}
		swapportstatus(mask);
		break;
	case SWITCH_BCM53115:
	case SWITCH_BCM53125:
		/* fall through */
#endif
	case SWITCH_BCM5301x:
		/* 5301x/53115/53125, 2bit:00=10 Mbps,01=100Mbps,10=1000Mbps */
		mask = value & portmask & 0x3ff;
		break;
	}

	//_dprintf("get_phy_speed %x %x\n", vecarg[1], portmask);

	return mask;
}

#if defined(HND_ROUTER)
uint32_t set_ex53134_ctrl(uint32_t portmask, int ctrl)
{
	int i=0;
	uint32_t value;

	for (i = 0; i < 5 && (portmask >> i); i++) {
		if ((portmask & (1U << i)) == 0)
			continue;

		value = 0x1140;
		value |= ctrl ? 0 : 0x0800;

		hnd_ethswctl(PMDIOACCESS, 0x1000 | i << 8, 2, 1, value);
	}

	return 0;
}
#endif

uint32_t set_phy_ctrl(uint32_t portmask, int ctrl)
{
	int fd, i, model;
	uint32_t value;

	model = get_switch();
	if (model == SWITCH_UNKNOWN) return 0;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) return 0;

	for (i = 0; i < 5 && (portmask >> i); i++) {
		if ((portmask & (1U << i)) == 0)
			continue;

		switch (model) {
#ifndef BCM5301X
		case SWITCH_BCM5325:
			if (phy_ioctl(fd, 0, i, 0x1e, &value) < 0)
				value = 0;
			value &= 0x0007;
			value |= ctrl ? 0 : 0x0008;
			phy_ioctl(fd, 1, i, 0x1e, &value);
			value = 0x3300;
			break;
		case SWITCH_BCM53115:
		case SWITCH_BCM53125:
			/* fall through */
#endif
		case SWITCH_BCM5301x_EX:
			value = 0x1140;
			value |= ctrl ? 0 : 0x0800;
			break;
		case SWITCH_BCM5301x:
			value = 0x1340;
			value |= ctrl ? 0 : 0x0800;
			break;
		default:
			continue;
		}

		/* issue write */
		phy_ioctl(fd, 1, i, 0, &value);
	}

	close(fd);

	return 0;
}

#define IMAGE_HEADER "HDR0"
#define MAX_VERSION_LEN 64
#define MAX_PID_LEN 12
#define MAX_HW_COUNT 4

/*
 * 0: illegal image
 * 1: legal image
 */
int
check_crc(char *fname)
{
	FILE *fp;
	int ret = 1;
	int first_read = 1;
	unsigned int len, count;

	struct trx_header trx;
	uint32 crc;
	static uint32 buf[16*1024];

	fp = fopen(fname, "r");
	if (fp == NULL)
	{
		_dprintf("Open trx fail!!!\n");
		return 0;
	}

	/* Read header */
	ret = fread((unsigned char *) &trx, 1, sizeof(struct trx_header), fp);
	if (ret != sizeof(struct trx_header)) {
		ret = 0;
		_dprintf("read header error!!!\n");
		goto done;
	}

	/* Checksum over header */
	crc = hndcrc32((uint8 *) &trx.flag_version,
		       sizeof(struct trx_header) - OFFSETOF(struct trx_header, flag_version),
		       CRC32_INIT_VALUE);

	for (len = ltoh32(trx.len) - sizeof(struct trx_header); len; len -= count) {
		if (first_read) {
			count = MIN(len, sizeof(buf) - sizeof(struct trx_header));
			first_read = 0;
		} else
			count = MIN(len, sizeof(buf));

		/* Read data */
		ret = fread((unsigned char *) &buf, 1, count, fp);
		if (ret != count) {
			ret = 0;
			_dprintf("read error!\n");
			goto done;
		}

		/* Checksum over data */
		crc = hndcrc32((uint8 *) &buf, count, crc);
	}
	/* Verify checksum */
	//_dprintf("checksum: %u ? %u\n", ltoh32(trx.crc32), crc);
	if (ltoh32(trx.crc32) != crc) {
		ret = 0;
		goto done;
	}

done:
	fclose(fp);

	return ret;
}

/*
 * 0: illegal image
 * 1: legal image
 */

int check_imageheader(char *buf, long *filelen)
{
	long aligned;

#ifdef HND_ROUTER
	return 1;
#endif
	if (strncmp(buf, IMAGE_HEADER, sizeof(IMAGE_HEADER) - 1) == 0)
	{
		memcpy(&aligned, buf + sizeof(IMAGE_HEADER) - 1, sizeof(aligned));
		*filelen = aligned;
#ifdef RTCONFIG_DSL_TCLINUX
		*filelen+=0x790000;
#endif
		_dprintf("image len: %x\n", aligned);
		return 1;
	}
	else return 0;
}

#ifdef RTCONFIG_QTN
char *wl_vifname_qtn(int unit, int subunit)
{
	static char tmp[128];

	if ((subunit > 0) && (subunit < 4))
	{
		sprintf(tmp, "wifi%d", subunit);
		return strdup(tmp);
	}
	else
		return strdup("");
}
#endif

int get_radio(int unit, int subunit)
{
	int n = 0;

	//_dprintf("get radio %x %x %s\n", unit, subunit, nvram_safe_get(wl_nvname("ifname", unit, subunit)));

#ifdef RTCONFIG_QTN
	int ret;
	char interface_status = 0;

	if (unit)
	{
		if (!rpc_qtn_ready())
			return -1;

		if (subunit > 0)
		{
			ret = qcsapi_interface_get_status(wl_vifname_qtn(unit, subunit), &interface_status);
//			if (ret < 0)
//				dbG("Qcsapi qcsapi_interface_get_status %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);

			return interface_status;
		}
		else
		{
			ret = qcsapi_wifi_rfstatus((qcsapi_unsigned_int *) &n);
//			if (ret < 0)
//				dbG("Qcsapi qcsapi_wifi_rfstatus %s error, return: %d\n", wl_vifname_qtn(unit, subunit), ret);

			return n;
		}
	}
	else
#endif

	return (wl_ioctl(nvram_safe_get(wl_nvname("ifname", unit, subunit)), WLC_GET_RADIO, &n, sizeof(n)) == 0) &&
		!(n & (WL_RADIO_SW_DISABLE | WL_RADIO_HW_DISABLE));
}

void set_radio(int on, int unit, int subunit)
{
	uint32 n;
	char tmp[100], prefix[] = "wlXXXXXXXXXXXXXX";

#ifdef RTCONFIG_QTN
	if (unit) {
		if (!rpc_qtn_ready())
			return;

		rpc_set_radio(unit, subunit, on);

		return;
	}
#endif
	//_dprintf("set radio %x %x %x %s\n", on, unit, subunit, nvram_safe_get(wl_nvname("ifname", unit, subunit)));

	if (subunit > 0)
		snprintf(prefix, sizeof(prefix), "wl%d.%d_", unit, subunit);
	else
		snprintf(prefix, sizeof(prefix), "wl%d_", unit);

	//if (nvram_match(strcat_r(prefix, "radio", tmp), "0")) return;

#if defined(RTAC66U) || defined(BCM4352)
	if ((unit == 1) & (subunit < 1)) {
		if (on) {
#ifndef RTCONFIG_LED_BTN
			if (!(sw_mode()==SW_MODE_AP && nvram_get_int("wlc_psta")==1 && nvram_get_int("wlc_band")==0)) {
				nvram_set("led_5g", "1");
				led_control(LED_5G, LED_ON);
			}
#else
			nvram_set("led_5g", "1");
			if (nvram_get_int("AllLED"))
				led_control(LED_5G, LED_ON);
#endif
		}
		else {
			nvram_set("led_5g", "0");
			led_control(LED_5G, LED_OFF);
		}
	}
#endif

	if (subunit > 0) {
		sprintf(tmp, "%d", subunit);
		if (on) eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", unit, 0)), "bss", "-C", tmp, "up");
		else eval("wl", "-i", nvram_safe_get(wl_nvname("ifname", unit, 0)), "bss", "-C", tmp, "down");

		if (nvram_get_int("led_disable")==1) {
			led_control(LED_2G, LED_OFF);
			led_control(LED_5G, LED_OFF);
		}
		return;
	}

#ifndef WL_BSS_INFO_VERSION
#error WL_BSS_INFO_VERSION
#endif

#if WL_BSS_INFO_VERSION >= 108
	n = on ? (WL_RADIO_SW_DISABLE << 16) : ((WL_RADIO_SW_DISABLE << 16) | 1);
	wl_ioctl(nvram_safe_get(wl_nvname("ifname", unit, subunit)), WLC_SET_RADIO, &n, sizeof(n));
	if (!on) {
		//led(LED_WLAN, 0);
		//led(LED_DIAG, 0);
	}
#else
	n = on ? 0 : WL_RADIO_SW_DISABLE;
	wl_ioctl(nvram_safe_get(wl_nvname("ifname", unit, subunit)), WLC_SET_RADIO, &n, sizeof(n));
	if (!on) {
		//led(LED_DIAG, 0);
	}
#endif

	if (nvram_get_int("led_disable")==1) {
		led_control(LED_2G, LED_OFF);
		led_control(LED_5G, LED_OFF);
	}
}

/* Return nvram variable name, e.g. et0macaddr, which is used to repented as LAN MAC.
 * @return:
 */
char *get_lan_mac_name(void)
{
#ifdef RTCONFIG_BCMARM
#ifdef RTCONFIG_GMAC3
	char *et2macaddr;
	if (!nvram_match("stop_gmac3", "1") && (et2macaddr = nvram_get("et2macaddr")) &&
		*et2macaddr && strcmp(et2macaddr, "00:00:00:00:00:00") != 0) {
		return "et2macaddr";
	}
#endif

	switch(get_model()) {
		case MODEL_RTAC87U:
		case MODEL_RTAC88U:
		case MODEL_RTAC5300:
			return "et1macaddr";
	}
#endif
	return "et0macaddr";
}

/* Return nvram variable name, e.g. et1macaddr, which is used to repented as WAN MAC.
 * @return:
 */
char *get_wan_mac_name(void)
{
#ifdef RTCONFIG_BCMARM
#ifdef RTCONFIG_GMAC3
	char *et2macaddr;
	if (!nvram_match("stop_gmac3", "1") && (et2macaddr = nvram_get("et2macaddr")) &&
		*et2macaddr && strcmp(et2macaddr, "00:00:00:00:00:00") != 0) {
		return "et2macaddr";
	}
#endif
	switch(get_model()) {
		case MODEL_RTAC87U:
		case MODEL_RTAC88U:
		case MODEL_RTAC5300:
			return "et1macaddr";
	}
#endif
	return "et0macaddr";
}

char *get_label_mac()
{
	return get_2g_hwaddr();
}

char *get_lan_hwaddr(void)
{
	return nvram_safe_get(get_lan_mac_name());
}

char *get_2g_hwaddr(void)
{
	return nvram_safe_get(get_lan_mac_name());
}

char *get_wan_hwaddr(void)
{
	return nvram_safe_get(get_wan_mac_name());
}

char *get_wlifname(int unit, int subunit, int subunit_x, char *buf)
{
	sprintf(buf, "wl%d.%d", unit, subunit);

	return buf;
}

/**
 * Generate VAP interface name of wlX.Y for Guest network, Free Wi-Fi, and Facebook Wi-Fi
 * @x:		X of wlX.Y, aka unit
 * @y:		Y of wlX.Y
 * @buf:	Pointer to buffer of VAP interface name. Must greater than or equal to IFNAMSIZ
 * @return:
 */
char *get_wlxy_ifname(int x, int y, char *buf)
{
#ifdef RTAC87U
	if (get_model() == MODEL_RTAC87U && (x == 1)) {
		if(y == 1) strcpy(buf, "vlan4000");
		if(y == 2) strcpy(buf, "vlan4001");
		if(y == 3) strcpy(buf, "vlan4002");
		return buf;
	}
#endif
	return get_wlifname(x, y, y, buf);
}

#ifdef RTCONFIG_AMAS
void add_beacon_vsie(char *hexdata)
{
	char cmd[300] = {0};
	//Bit 0 - Beacons, Bit 1 - Probe Rsp, Bit 2 - Assoc/Reassoc Rsp 
	//Bit 3 - Auth Rsp, Bit 4 - Probe Req, Bit 5 - Assoc/Reassoc Req
	int pktflag = 0x3;
	int len = 0;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;

	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */

	if (is_router_mode() || access_point_mode())
		snprintf(prefix, sizeof(prefix), "wl0_");
	else
		snprintf(prefix, sizeof(prefix), "wl0.1_");

	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "wl -i %s add_ie %d %d %02X:%02X:%02X %s", 
			ifname, pktflag, len, OUI_ASUS[0], OUI_ASUS[1], OUI_ASUS[2], hexdata);
		system(cmd);
	}
}

void del_beacon_vsie(char *hexdata)
{
	char cmd[300] = {0};
	int pktflag = 0x3;
	int len = 0;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;

	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */

	if (is_router_mode() || access_point_mode())
		snprintf(prefix, sizeof(prefix), "wl0_");
	else
		snprintf(prefix, sizeof(prefix), "wl0.1_");

	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "wl -i %s del_ie %d %d %02X:%02X:%02X %s",
			ifname, pktflag, len, OUI_ASUS[0], OUI_ASUS[1], OUI_ASUS[2], hexdata);
		system(cmd);
	}
}

void add_obd_probe_req_vsie(char *hexdata)
{
	char cmd[300] = {0};
	//Bit 0 - Beacons, Bit 1 - Probe Rsp, Bit 2 - Assoc/Reassoc Rsp
	//Bit 3 - Auth Rsp, Bit 4 - Probe Req, Bit 5 - Assoc/Reassoc Req
	int pktflag = 0x10;
	int len = 0;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;

	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */

	if (is_router_mode() || access_point_mode())
		snprintf(prefix, sizeof(prefix), "wl0_");
	else
		snprintf(prefix, sizeof(prefix), "wl0.1_");

	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "wl -i %s add_ie %d %d %02X:%02X:%02X %s",
			ifname, pktflag, len, OUI_ASUS[0], OUI_ASUS[1], OUI_ASUS[2], hexdata);
		system(cmd);
	}
}

void del_obd_probe_req_vsie(char *hexdata)
{
	char cmd[300] = {0};
	int pktflag = 0x10;
	int len = 0;
	char tmp[128], prefix[] = "wlXXXXXXXXXX_";
	char *ifname = NULL;

	len = 3 + strlen(hexdata)/2;	/* 3 is oui's len */

	if (is_router_mode() || access_point_mode())
		snprintf(prefix, sizeof(prefix), "wl0_");
	else
		snprintf(prefix, sizeof(prefix), "wl0.1_");

	ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	if (ifname && strlen(ifname)) {
		snprintf(cmd, sizeof(cmd), "wl -i %s del_ie %d %d %02X:%02X:%02X %s",
			ifname, pktflag, len, OUI_ASUS[0], OUI_ASUS[1], OUI_ASUS[2], hexdata);
		system(cmd);
	}
}
#endif	/* RTCONFIG_AMAS */
