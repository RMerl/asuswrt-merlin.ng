#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <shared.h>

int bit_count(in_addr_t i)
{
	int c = 0, b;
	unsigned int seen_one = 0;

	// Be sure to check all octets
	for (b = 0; i > 0 || b < 25; ++b, i >>= 1) {
		if (i & 1) {
			seen_one = 1;
			c++;
		} else {
			if (seen_one) {
				return -1;
			}
		}
	}

	return c;
}

int convert_subnet_mask_to_cidr(const char *mask)
{
	in_addr_t n;
	
	if(!mask)
		return -1;

	if(inet_pton(AF_INET, mask, &n) != 1)
		return -1;

	return bit_count(ntohl(n));
}


char *convert_cidr_to_subnet_mask(const unsigned long cidr, char *mask, const int mask_len)
{
	if(!mask || mask_len <= 0 || cidr > 32)
		return NULL;

	/*
		C Language Spec about << operator
		The behavior is undefined if the right operand is negative, orgreater than or
		equal to the length in bits of the promoted left operand.
	*/
	unsigned long submask = !cidr? cidr: (0xFFFFFFFF << (32 - cidr)) & 0xFFFFFFFF;

	snprintf(mask, mask_len, "%ld.%ld.%ld.%ld", (submask>>24)&0xff, (submask>>16)&0xff, (submask>>8)&0xff, submask&0xff);
	return mask;
}

/*--------------------------------------*/
/* Compute netmask address given prefix */
/*--------------------------------------*/
static in_addr_t _netmask( int prefix ) 
{
	in_addr_t addrt;
	if ( prefix == 0 )
	//return( ~((in_addr_t) -1) );
		addrt = ~((in_addr_t) -1);
	else
	//return( ~((1 << (32 - prefix)) - 1) );
		addrt = ~((1 << (32 - prefix)) - 1);

	return addrt;
} /* netmask() */

/*--------------------------------------------------*/
/* Compute network address given address and prefix */
/*--------------------------------------------------*/
static in_addr_t _network( in_addr_t addr, int prefix ) {

  return( addr & _netmask(prefix) );

} /* network() */

/*------------------------------------------------*/
/* Print out a 32-bit address in A.B.C.D/M format */
/*------------------------------------------------*/
static int _convert_addr( in_addr_t addr, int prefix, char *str, const int len ) {
  struct in_addr in;

  if(!str || len <= 0)
  	return -1;

  in.s_addr = htonl( addr );

  
  if ( prefix < 32 )
  	snprintf(str, len, "%s/%d", inet_ntoa(in), prefix);
  else
  	snprintf(str, len, "%s", inet_ntoa(in));

  return 0;
} /* print_addr() */


int get_network_addr_by_ip_prefix(const char *ip, const char *netmask, char *full_addr, const int len)
{
	struct in_addr in_addr;
	in_addr_t addrt;
	int prefix;

	if(!ip || !netmask || !full_addr || len <= 0)
		return -1;

	inet_aton(ip, &in_addr);
	addrt = ntohl(in_addr.s_addr);
	
	prefix = convert_subnet_mask_to_cidr(netmask);
	
	addrt = _network(addrt, prefix);

	return _convert_addr(addrt, prefix, full_addr, len);	
}

int validate_number(char *str)
{
	while(*str)
	{
		if(!isdigit(*str))  //if the character is not a number, return false
		{			
			return 0;
		}
		str++; //point to next character
	}
	return 1;
}

int validate_ip(char *ip)
{
	//check whether the IP is valid or not
	int i, num, dots = 0;
	char *ptr;
	char buf[16] = {0};

	if(ip == NULL)
	{
		return 0;
	}

	strlcpy(buf, ip, sizeof(buf));
	ptr = strtok(buf, "."); //cut the string using dor delimiter
	if(ptr == NULL)
	{
		return 0;
	}

	while(ptr)
	{
		if(!validate_number(ptr))  //check whether the sub string is holding only number or not
		{
			return 0;
		}
		num = atoi(ptr); //convert substring to number
		if(num >= 0 && num <= 255)
		{
			ptr = strtok(NULL, "."); //cut the next part of the string
			if(ptr != NULL)
			{
				dots++;    //increase the dot count
			}
		}
		else
		{
			return 0;
		}
	}
	if(dots != 3)  //if the number of dots are not 3, return false
	{
		return 0;
	}
	return 1;
}

int is_valid_ip(const char* addr)
{
	struct addrinfo hint, *res = NULL;
	int ret = -1;

	memset(&hint, 0, sizeof(hint));

	hint.ai_family = PF_UNSPEC;
	hint.ai_flags = AI_NUMERICHOST;

	if (getaddrinfo(addr, NULL, &hint, &res))
		ret = -1;
	else if (res->ai_family == AF_INET)
		ret = 1;
	else if (res->ai_family == AF_INET6)
		ret = 2;
	else
		ret = 0;

	freeaddrinfo(res);
	return (ret);
}

int is_valid_ip4(const char* addr)
{
	return (is_valid_ip(addr) == 1) ? 1 : 0;
}

int is_valid_ip6(const char* addr)
{
	return (is_valid_ip(addr) == 2) ? 1 : 0;
}

int is_ip4_in_use(const char* addr)
{
	struct in_addr ipaddr;
	int fd;
	struct ifreq *ifreq;
	struct ifconf ifconf;
	char buf[8192];

	if (inet_pton(AF_INET, addr, &ipaddr) <= 0) {
		perror("inet_pton()");
		return 0;
	}

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket()");
		return 0;
	}

	ifconf.ifc_len = sizeof(buf);
	ifconf.ifc_buf = buf;
	if (ioctl(fd, SIOCGIFCONF, &ifconf) != 0) {
		perror("ioctl(SIOCGIFCONF)");
		return 0;
	}

	ifreq = ifconf.ifc_req;
	while ( (char*)ifreq < buf + ifconf.ifc_len ) {
		if( ((struct sockaddr_in*)&ifreq->ifr_addr)->sin_addr.s_addr == ipaddr.s_addr ) {
			//char text[16];
			//cprintf("ifr %s: %s\n", ifreq->ifr_name, inet_ntop(AF_INET, &(((struct sockaddr_in*)&ifreq->ifr_addr)->sin_addr), text, sizeof(text)));
			return 1;
		}

		ifreq = (struct ifreq*)((char*)ifreq + sizeof(*ifreq));
	}

	return 0;
}
