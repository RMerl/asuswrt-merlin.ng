/*************************************************************************
 *
 * ivictl.c :
 *
 * MAP-T/MAP-E CPE Userspace Controller Utility
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn>
 * 	 Wentao Shang <wentaoshang@gmail.com>
 * 	 Guoliang Han <bupthgl@gmail.com>
 * 
 * Contributions:
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * For more versions, please send an email to <bupthgl@gmail.com> to
 * obtain an password to access the svn server.
 *
 * LIC: GPLv2
 * 
 ************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <getopt.h>

#include "./ivi_ioctl.h"

static const struct option longopts[] =
{
	{"rule", no_argument, NULL, 'r'},
	{"start", no_argument, NULL, 's'},
	{"stop", required_argument, NULL, 'q'},
	{"help", no_argument, NULL, 'h'},
	{"hgw", no_argument, NULL, 'H'},
	{"nat44", no_argument, NULL, 'N'},
	{"noeabits", no_argument, NULL, 'X'},
	{"default", no_argument, NULL, 'd'},
	{"prefix4", required_argument, NULL, 'p'},
	{"prefix6", required_argument, NULL, 'P'},
	{"ratio", required_argument, NULL, 'R'},
	{"psidoffset", required_argument, NULL, 'z'},
	{"encapsulate", required_argument, NULL, 'E'},
	{"translate", required_argument, NULL, 'T'},
	{"psid", required_argument, NULL, 'o'},
	{"address", required_argument, NULL, 'a'},
	{"publicaddr", required_argument, NULL, 'A'},
	{"dev4", required_argument, NULL, 'i'},
	{"dev6", required_argument, NULL, 'I'},
	{"mssclamping", required_argument, NULL, 'c'},
	{NULL, no_argument, NULL, 0}
};


static char hgw;
static char nat44;
static char xeabits;
static char transpt;
static char psidoff;
static char dev[IVI_IOCTL_LEN];
static __u16 gma[2];  // Store R and PSID, M is stored in 'rule.adjacent'
static __u16 mss_val;
static struct in_addr v4addr;
static struct rule_info rule;

int ffs(int x)
{
	int r = 1;

	if (!x)
		return 0;
	if (!(x & 0xffff)) {
		x >>= 16;
		r += 16;
	}
	if (!(x & 0xff)) {
		x >>= 8;
		r += 8;
	}
	if (!(x & 0xf)) {
		x >>= 4;
		r += 4;
	}
	if (!(x & 3)) {
		x >>= 2;
		r += 2;
	}
	if (!(x & 1)) {
		x >>= 1;
		r += 1;
	}
	return r;
}

int fls(int x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

void usage(int status) {
	if (status != EXIT_SUCCESS)
		printf("Try `ivictl --help' for more information.\n");
	else {
		printf("\
Usage:  ivictl -r [rule_options]\n\
	(used to insert a mapping rule)\n\
	ivictl -s [start_options]\n\
	(used to start MAP module)\n\
	ivictl -q\n\
	(used to stop MAP module)\n\
	ivictl -h\n\
	(used to display this help information)\n\
\n\
rule_options:\n\
	-p --prefix4 [PREFIX4/PLEN4]\n\
		specify the ipv4 prefix and length\n\
	-P --prefix6 [PREFIX6/PLEN6]\n\
		specify the ipv6 prefix and length\n\
	-z --psidoffset PSIDOFFSET\n\
		specify the psid offset parameter in GMA\n\
	-R --ratio RATIO\n\
		specify the address sharing ratio in GMA\n\
	-d --default\n\
		specify the ipv4 prefix is '0.0.0.0/0' instead of using '-p 0.0.0.0 -l 0'\n\
	-E --encapsulate\n\
		specify the mapping rule is used for MAP-E\n\
	-T --translate\n\
		specify the mapping rule is used for MAP-T\n\
\n\
start_options:\n\
	-i --dev4 DEV4\n\
		specify the name of ipv4 device\n\
	-I --dev6 DEV6\n\
		specify the name of ipv6 device\n\
	-c --mssclamping MSS\n\
		specify the reduced tcp mss value\n\
\n\
	HGW mode:\n\
		-H --hgw\n\
			specify that IVI is working as home gateway\n\
		-N --nat44\n\
			specify that IVI HGW is performing NAT44\n\
		-o --psid PSID\n\
			specify the local PSID of the HGW, default is 0\n\
		-a --address [ADDRRESS/PREFIXLENGTH]\n\
			specify the ipv4 address and mask used by the HGW\n\
		-A --publicaddr [PUBLICADDR/PUBLICPREFIXLENGTH]\n\
			specify the public ipv4 address and mask used by the HGW in NAT44 mode\n\
			always used with -N (--nat44)\n\
		-P --prefix6 [PREFIX6/PLEN6]\n\
			specify the local IVI prefix and length used by the HGW\n\
		-z --psidoffset PSIDOFFSET\n\
			specify the local psid offset parameter in GMA\n\
		-R --ratio RATIO\n\
			specify the local address sharing ratio in GMA\n\
		-X --noeabits\n\
			specify that the HGW doesn't use eabits to constitute the IPv6 address\n\
		-E --encapsulate\n\
			specify that the HGW supports MAP-E mode\n\
		-T --translate\n\
			specify that the HGW supports MAP-T\n\
\n");
	}
	exit(status);
} 

static inline void param_init(void) {
	hgw = 0;
	nat44 = 0;
	xeabits = 0;
	psidoff = 6;  // default PSID offset value
	transpt = MAP_T;
	gma[0] = gma[1] = 0;
	memset(&rule, 0, sizeof(rule));
	rule.ratio = 1;
	rule.adjacent = 1;
	rule.format = ADDR_FMT_MAPT;
	rule.transport = MAP_T;
}

int main(int argc, char *argv[]) {
	int retval, fd, temp, optc;
	char *token = NULL;
	
	printf("MAP netfilter device controller utility v1.0\n");
	
	if ((fd = open("/dev/ivi", 0)) < 0) {
		printf("\nError*****: cannot open virtual device for ioctl, code %d.\n\n", fd);
		exit(-1);
	}
	
	param_init();
	
	optc = getopt_long(argc, argv, "rsqh", longopts, NULL);
	switch (optc) 
	{
		case 'r':
			goto rule_opt;
			break;
		case 's':
			goto start_opt;
			break;
		case 'q':
			if ((retval = ioctl(fd, IVI_IOC_STOP, 0)) != 0) {
				printf("\nError*****: failed to stop MAP module, code %d.\n\n", retval);
			}
			else {
				printf("Info: successfully stopped MAP module.\n");
			}
			goto out;
			break;
		case 'h':
			close(fd);
			usage(EXIT_SUCCESS);
			break;
		default:
			close(fd);
			usage(EXIT_FAILURE);
			break;
	}
	
rule_opt:
	while ((optc = getopt_long(argc, argv, "p:P:R:z:fdET", longopts, NULL)) != -1)
	{
		switch(optc)
		{
			case 'd':
				rule.prefix4 = 0;
				rule.plen4 = 0;
				rule.format = ADDR_FMT_NONE;
				break;

			case 'p':
				// Extract IPv4 address
				token = strtok(optarg, "/");
				if (token == NULL) {
					retval = -1;
					goto out;
				}
				if ((retval = inet_pton(AF_INET, token, (void*)(&(rule.prefix4)))) != 1) {
					printf("\nError*****: failed to parse IPv4 address, code %d.\n\n", retval);
					retval = -1;
					goto out;
				}
				rule.prefix4 = ntohl(rule.prefix4);  // Convert to host byte order
				
				// Extract address prefix length
				token = strtok(NULL, "/");
				if (token == NULL) {
					retval = -1;
					goto out;
				}
				rule.plen4 = atoi(token);
				if (rule.plen4 > 32 || rule.plen4 < 0) {
					printf("\nError*****: IPv4 prefix length is out of scope.\n\n");
					usage(EXIT_FAILURE);
					retval = -1;
					goto out;
				}
				break;
			case 'P':
				// Extract IPv6 address
				token = strtok(optarg, "/");
				if (token == NULL) {
					retval = -1;
					goto out;
				}
				if ((retval = inet_pton(AF_INET6, token, (void*)(&(rule.prefix6)))) != 1) {
					printf("\nError*****: failed to parse IPv6 prefix, code %d.\n\n", retval);
					retval = -1;
					goto out;
				}
				
				// Extract address prefix length
				token = strtok(NULL, "/");
				if (token == NULL) {
					retval = -1;
					goto out;
				}
				rule.plen6 = atoi(token);
				if (rule.plen6 > 128 || rule.plen6 < 0) {
					printf("\nError*****: IPv6 prefix length is out of scope.\n\n");
					usage(EXIT_FAILURE);
					retval = -1;
					goto out;
				}
				break;
			case 'R':
				rule.ratio = atoi(optarg);
				if (fls(rule.ratio) != ffs(rule.ratio)) {
					printf("\nError*****: Ratio must be a power of two.\n\n");
					usage(EXIT_FAILURE);
					retval = -1;
					goto out;
				}
				break;
			case 'z':
				psidoff = atoi(optarg);
				if (psidoff > 16 || psidoff < 0) {
					printf("\nError*****: psid offset is out of scope.\n\n");
					usage(EXIT_FAILURE);
					retval = -1;
					goto out;
				}
				break;
			case 'E':
				rule.transport = MAP_E;
				break;
			case 'T':
				rule.transport = MAP_T;
				break;
			default:
				close(fd);
				usage(EXIT_FAILURE);
				break;
		}
	}
	
	if (rule.prefix4 > 0) {
		if (psidoff + fls(rule.ratio) - 1 > 16) {
			printf("\nError*****: PSID offset + PSID length must be no more than 16 bits.\n\n");
			usage(EXIT_FAILURE);
			retval = -1;
			goto out;
		}
	
		rule.adjacent = 1 << (16 - psidoff - (fls(rule.ratio) - 1));
	
		// Finalize
		temp = (rule.plen4 == 0) ? 0 : 0xffffffff << (32 - rule.plen4);  // Generate network mask
		rule.prefix4 = rule.prefix4 & temp;
	}
	
	// Insert rule
	if ((retval = ioctl(fd, IVI_IOC_ADD_RULE, &rule)) < 0) {
		printf("\nError*****: failed to add mapping rule, code %d.\n\n", retval);
		usage(EXIT_FAILURE);
	} else {
		printf("Info: successfully add mapping rule.\n");
	}
	
	goto out;

start_opt:
	while ((optc = getopt_long(argc, argv, "i:I:A:a:P:R:z:o:fc:HNXET", longopts, NULL)) != -1)
	{
		switch(optc)
		{
			case 'i':
				strncpy(dev, optarg, IVI_IOCTL_LEN);
				if ((retval = ioctl(fd, IVI_IOC_V4DEV, dev)) < 0) {
					printf("\nError*****: failed to assign IPv4 device, code %d.\n\n", retval);
					goto out;
				}
				break;
			case 'I':
				strncpy(dev, optarg, IVI_IOCTL_LEN);
				if ((retval = ioctl(fd, IVI_IOC_V6DEV, dev)) < 0) {
					printf("\nError*****: failed to assign IPv6 device, code %d.\n\n", retval);
					goto out;
				}
				break;
			case 'c':
				mss_val = atoi(optarg);
				if ((retval = ioctl(fd, IVI_IOC_MSS_LIMIT, (void*)(&mss_val))) < 0) {
					printf("\nError*****: failed to set mssclamping, code %d.\n\n", retval);
					goto out;
				}
				break;
			case 'H':
				hgw = 1;
				break;
			case 'a':
				// Extract IPv4 address
				token = strtok(optarg, "/");
				if (token == NULL) {
					retval = -1;
					goto out;
				}
				if ((retval = inet_pton(AF_INET, token, (void*)(&(rule.prefix4)))) != 1) {
					printf("\nError*****: failed to parse IPv4 address, code %d.\n\n", retval);
					retval = -1;
					goto out;
				}
				rule.prefix4 = ntohl(rule.prefix4);  // Convert to host byte order
				if ((retval = ioctl(fd, IVI_IOC_V4NET, &(rule.prefix4))) < 0) {
					printf("\nError*****: failed to assign IPv4 address, code %d.\n\n", retval);
					goto out;
				}
				
				// Extract address prefix length
				token = strtok(NULL, "/");
				if (token == NULL) {
					retval = -1;
					goto out;
				}
				rule.plen4 = atoi(token);
				if (rule.plen4 > 32 || rule.plen4 < 0) {
					printf("\nError*****: IPv4 prefix length is out of scope.\n\n");
					usage(EXIT_FAILURE);
					retval = -1;
					goto out;
				}
				temp = (rule.plen4 == 0) ? 0 : 0xffffffff << (32 - rule.plen4);  // Generate network mask
				if ((retval = ioctl(fd, IVI_IOC_V4MASK, &(temp))) < 0) {
					printf("\nError*****: failed to assign IPv4 address prefix length, code %d.\n\n", retval);
					goto out;
				}
						
				break;
			case 'N':
				nat44 = 1;
				break;
			case 'A':
				// Extract IPv4 address
				token = strtok(optarg, "/");
				if (token == NULL) {
					retval = -1;
					goto out;
				}
				if ((retval = inet_pton(AF_INET, token, (void*)(&v4addr))) != 1) {
					printf("\nError*****: failed to parse IPv4 public address, code %d.\n\n", retval);
					retval = -1;
					goto out;
				}
				v4addr.s_addr = ntohl(v4addr.s_addr);
				if ((retval = ioctl(fd, IVI_IOC_V4PUB, &(v4addr.s_addr))) < 0) {
					printf("\nError*****: failed to assign IPv4 public address, code %d.\n\n", retval);
					goto out;
				}
				
				// Extract address prefix length
				token = strtok(NULL, "/");
				if (token == NULL) {
					retval = -1;
					goto out;
				}
				rule.plen4 = atoi(token);
				if (rule.plen4 > 32 || rule.plen4 < 0) {
					printf("\nError*****: IPv4 prefix length is out of scope.\n\n");
					usage(EXIT_FAILURE);
					retval = -1;
					goto out;
				}
				temp = (rule.plen4 == 0) ? 0 : 0xffffffff << (32 - rule.plen4);  // Generate network mask
				if ((retval = ioctl(fd, IVI_IOC_V4PUBMASK, &(temp))) < 0) {
					printf("\nError*****: failed to assign IPv4 public address prefix length, code %d.\n\n", retval);
					goto out;
				}
				
				nat44 = 1;  // Done.:)
				break;
			case 'P':
				// Extract IPv6 address
				token = strtok(optarg, "/");
				if (token == NULL) {
					retval = -1;
					goto out;
				}
				if ((retval = inet_pton(AF_INET6, token, (void*)(&(rule.prefix6)))) != 1) {
					printf("\nError*****: failed to parse IPv6 prefix, code %d.\n\n", retval);
					retval = -1;
					goto out;
				}
				if ((retval = ioctl(fd, IVI_IOC_V6NET, &(rule.prefix6))) < 0) {
					printf("\nError*****: failed to assign IPv6 prefix, code %d.\n\n", retval);
					goto out;
				}
				
				// Extract address prefix length
				token = strtok(NULL, "/");
				if (token == NULL) {
					retval = -1;
					goto out;
				}
				rule.plen6 = atoi(token);
				if (rule.plen6 > 128 || rule.plen6 < 0) {
					printf("\nError*****: IPv6 prefix length is out of scope.\n\n");
					usage(EXIT_FAILURE);
					retval = -1;
					goto out;
				}
				temp = rule.plen6;  // counted in bits
				if ((retval = ioctl(fd, IVI_IOC_V6MASK, &(temp))) < 0) {
					printf("\nError*****: failed to assign IPv6 prefix length, code %d.\n\n", retval);
					goto out;
				}
				break;
				
			case 'X':
				xeabits = 1;
				break;
			case 'R':
				gma[0] = rule.ratio = atoi(optarg);
				if (fls(rule.ratio) != ffs(rule.ratio)) {
					printf("\nError*****: Ratio must be a power of two.\n\n");
					usage(EXIT_FAILURE);
					retval = -1;
					goto out;
				}
				break;
			case 'z':
				psidoff = atoi(optarg);
				if (psidoff > 16 || psidoff < 0) {
					printf("\nError*****: psid offset is out of scope.\n\n");
					usage(EXIT_FAILURE);
					retval = -1;
					goto out;
				}
				break;
			case 'o':
				gma[1] = atoi(optarg);
				break;
			case 'E':
				transpt = MAP_E;
				break;
			case 'T':
				transpt = MAP_T;
				break;
			default:
				close(fd);
				usage(EXIT_FAILURE);
				break;
		}
	}	
	
	if (psidoff + fls(rule.ratio) - 1 > 16) {
		printf("\nError*****: PSID offset + PSID length must be no more than 16 bits.\n\n");
		usage(EXIT_FAILURE);
		retval = -1;
		goto out;
	}
	
	rule.adjacent = 1 << (16 - psidoff - (fls(rule.ratio) - 1));
	
	if (gma[1] >= rule.ratio || gma[1] < 0) {
		printf("\nError*****: PSID must be less than ratio.\n\n");
		usage(EXIT_FAILURE);
		retval = -1;
		goto out;
	}
	
	// Set local addr format for HGW mode
	if (hgw) {
		if (rule.format == ADDR_FMT_MAPT) {
			if ((retval = ioctl(fd, IVI_IOC_MAPT, gma)) < 0) {
				printf("\nError*****: failed to set addr format, code %d.\n\n", retval);
				goto out;
			}
			if ((retval = ioctl(fd, IVI_IOC_ADJACENT, (void*)(&rule.adjacent))) < 0) {
				printf("\nError*****: failed to set adjacent parameter, code %d.\n\n", retval);
				goto out;
			}
		}
		
		if ((retval = ioctl(fd, IVI_IOC_TRANSPT, &transpt)) < 0) {
			printf("\nError*****: failed to set MAP transport, code %d.\n\n", retval);
			goto out;
		}
		
		printf("Info: successfully set HGW parameters.\n");
	}
	
	// Start IVI
	if (hgw && !nat44 && !xeabits) {
		if ((retval = ioctl(fd, IVI_IOC_NONAT, 0)) < 0) {
			printf("\nError*****: failed to disable nat44, code %d.\n\n", retval);
			goto out;
		}
		if ((retval = ioctl(fd, IVI_IOC_START, 0)) < 0) {
			printf("\nError*****: failed to start MAP module, code %d.\n\n", retval);
			goto out;
		}
	} else if (hgw && nat44 && !xeabits) {
		if ((retval = ioctl(fd, IVI_IOC_NAT, 0)) < 0) {
			printf("\nError*****: failed to enable nat44, code %d.\n\n", retval);
			goto out;
		}
		if ((retval = ioctl(fd, IVI_IOC_START, 0)) < 0) {
			printf("\nError*****: failed to start MAP module, code %d.\n\n", retval);
			goto out;
		}
	} else if (hgw && nat44 && xeabits) { // no eabits
		if ((retval = ioctl(fd, IVI_IOC_NAT, 0)) < 0) {
			printf("\nError*****: failed to enable nat44, code %d.\n\n", retval);
			goto out;
		}
		if ((retval = ioctl(fd, IVI_IOC_HGW_MAPX, 0)) < 0) {
			printf("\nError*****: failed to enable no eabits format, code %d.\n\n", retval);
			goto out;
		}
		if ((retval = ioctl(fd, IVI_IOC_START, 0)) < 0) {
			printf("\nError*****: failed to start MAP module, code %d.\n\n", retval);
			goto out;
		}
	} else if (hgw && !nat44 && xeabits) { // no eabits
		if ((retval = ioctl(fd, IVI_IOC_NONAT, 0)) < 0) {
			printf("\nError*****: failed to enable nat44, code %d.\n\n", retval);
			goto out;
		}
		if ((retval = ioctl(fd, IVI_IOC_HGW_MAPX, 0)) < 0) {
			printf("\nError*****: failed to enable no eabits format, code %d.\n\n", retval);
			goto out;
		}
		if ((retval = ioctl(fd, IVI_IOC_START, 0)) < 0) {
			printf("\nError*****: failed to start MAP module, code %d.\n\n", retval);
			goto out;
		}
	}
	else {
		close(fd);
		usage(EXIT_FAILURE);
	}
	
	printf("Info: successfully started MAP module.\n");

out:
	close(fd);
	return retval;
}
