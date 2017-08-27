#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>

int bit_count(uint32_t i)
{
	int c = 0;
	unsigned int seen_one = 0;

	while (i > 0) {
		if (i & 1) {
			seen_one = 1;
			c++;
		} else {
			if (seen_one) {
				return -1;
			}
		}
		i >>= 1;
	}

	return c;
}

int convert_subnet_mask_to_cidr(const char *mask)
{
	unsigned long n;
	
	if(!mask)
		return -1;

	if(inet_pton(AF_INET, mask, &n) != 1)
		return -1;

	return bit_count(n);
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

