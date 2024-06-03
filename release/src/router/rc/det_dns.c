#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>

#include <network_utility.h>
#include <shared.h>
#include <multi_wan.h>
#include <det_dns.h>

#define DEFAULT_TIMEOUT	10
#define MAX_BUF_SZ	65536
#define NAME_SZ	256

#define STATE_PARAMETER_ERR	"Parameters Error"
#define STATE_INITIAL	"Initial"
#define STATE_SENDING	"Sending"
#define STATE_SEND_FAIL	"Sending Fail"
#define STATE_RECVING	"Recving"
#define STATE_RECV_FAIL	"Revcving Fail"
#define STATE_FAIL	"Fail"
#define STATE_TIMEOUT	"Timeout"


extern void mtwan_set_detect_routing_rule(const char *dest, const char *ifname, int add);

static void _convert_hostname_format(unsigned char *dns, unsigned char *host);
static unsigned char *_read_name(unsigned char *, unsigned char *, int *);

// DNS header structure
struct DNS_HEADER
{
	unsigned short id; // identification number

	unsigned char rd : 1;	  // recursion desired
	unsigned char tc : 1;	  // truncated message
	unsigned char aa : 1;	  // authoritive answer
	unsigned char opcode : 4; // purpose of message
	unsigned char qr : 1;	  // query/response flag

	unsigned char rcode : 4; // response code
	unsigned char cd : 1;	 // checking disabled
	unsigned char ad : 1;	 // authenticated data
	unsigned char z : 1;	 // its z! reserved
	unsigned char ra : 1;	 // recursion available

	unsigned short q_count;	   // number of question entries
	unsigned short ans_count;  // number of answer entries
	unsigned short auth_count; // number of authority entries
	unsigned short add_count;  // number of resource entries
};

// Constant sized fields of query structure
struct QUESTION
{
	unsigned short qtype;
	unsigned short qclass;
};

// Constant sized fields of the resource record structure
#pragma pack(push, 1)
struct R_DATA
{
	unsigned short type;
	unsigned short _class;
	unsigned int ttl;
	unsigned short data_len;
};
#pragma pack(pop)

// Pointers to resource record contents
struct RES_RECORD
{
	unsigned char *name;
	struct R_DATA *resource;
	unsigned char *rdata;
};

// Structure of a Query
typedef struct
{
	unsigned char *name;
	struct QUESTION *ques;
} QUERY;

static void _print_help(const char* cmd)
{
	if(cmd)
	{
		printf("usage: %s -i <ifname> -s <dns server> -h <hostname> [options]\n", cmd);
		printf(" optional:\n\t-t <timeout seconds>\n");
	}
	exit(-1);
}

int det_dns_main(int argc, char *argv[])
{
	char *ifname = NULL;
	char *dns_server = NULL;
	int timeout = DEFAULT_TIMEOUT;
	char *hostname = NULL;
	int cmd_opt;

	//parsing command options
	while((cmd_opt = getopt(argc, argv, "i:s:t:h:")) != -1)
	{
		switch(cmd_opt)
		{
			case 'i':
				ifname = strdup(optarg); 
			break;
			case 's':
				dns_server = strdup(optarg);
			break;
			case 't':
				timeout = atoi(optarg);
			break;
			case 'h':
				hostname = strdup(optarg);
			break;
			case '?':
			default:
				printf("Unknown option.\n");
				_print_help(argv[0]);
			break;
		}
	}

	//check options
	if(!dns_server || !ifname || !hostname)
	{
			_print_help(argv[0]);
		goto ERROR;
	}

	printf("Interface: %s\n", ifname);
	printf("DNS server: %s\n", dns_server);
	printf("Hostname: %s\n", hostname);
	printf("Timeout:%d\n", timeout);

	// Now get the ip of this hostname , A record
	mtwan_set_detect_routing_rule(dns_server, ifname, 1);
	get_host_by_name((unsigned char*)hostname, T_A, ifname, dns_server, timeout, NULL, 0);
	mtwan_set_detect_routing_rule(dns_server, ifname, 0);

ERROR:
	SAFE_FREE(ifname);
	SAFE_FREE(dns_server);
	SAFE_FREE(hostname);
	return 0;
}

/*
 * Perform a DNS query by sending a packet
 * */
int get_host_by_name(unsigned char *host, int query_type, const char *ifname, const char *dns_server, const int timeout, char *ip, const size_t ip_sz)
{
	unsigned char buf[MAX_BUF_SZ], *qname, *reader;
	int i, j, stop, s;
	struct sockaddr_in a;
	struct RES_RECORD answers[20], auth[20], addit[20]; // the replies from the DNS server
	struct sockaddr_in dest;
	struct DNS_HEADER *dns = NULL;
	struct QUESTION *qinfo = NULL;
	struct timeval tv;
	int ret = -1;

	if(!host || !dns_server)
	{
		printf("%s\n", STATE_PARAMETER_ERR);
		return ret;
	}
	//_dprintf("[%s, %d]host:%s, ifname:%s, dns_server:%s\n", __FUNCTION__, __LINE__, host, ifname? ifname: "", dns_server);

	if(ip && ip_sz > 0)
		ip[0] = '\0';

	printf("%s\n", STATE_INITIAL);

	tv.tv_sec = timeout > 0? timeout: DEFAULT_TIMEOUT;
	tv.tv_usec = 0;

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // UDP packet for DNS queries

	if (s < 0)
	{
		printf("%s\n", STATE_FAIL);
		return ret;
	}

	dest.sin_family = AF_INET;
	dest.sin_port = htons(53);
	dest.sin_addr.s_addr = inet_addr(dns_server); // dns servers

	//bind interface
	if(ifname)
	{
		setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, ifname, strlen(ifname));
	}

	//set timeout
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) ;
	setsockopt(s, SOL_SOCKET, SO_SNDTIMEO,&tv,sizeof(tv)) ;

	memset(answers, 0, sizeof(struct RES_RECORD) * 20);
	memset(auth, 0, sizeof(struct RES_RECORD) * 20);
	memset(addit, 0, sizeof(struct RES_RECORD) * 20);

	// Set the DNS structure to standard queries
	memset(buf, 0, sizeof(buf));
	dns = (struct DNS_HEADER *)&buf;

	dns->id = (unsigned short)htons(getpid());
	dns->qr = 0;	 // This is a query
	dns->opcode = 0; // This is a standard query
	dns->aa = 0;	 // Not Authoritative
	dns->tc = 0;	 // This message is not truncated
	dns->rd = 1;	 // Recursion Desired
	dns->ra = 0;	 // Recursion not available! hey we dont have it (lol)
	dns->z = 0;
	dns->ad = 0;
	dns->cd = 0;
	dns->rcode = 0;
	dns->q_count = htons(1); // we have only 1 question
	dns->ans_count = 0;
	dns->auth_count = 0;
	dns->add_count = 0;

	// point to the query portion
	qname = (unsigned char *)&buf[sizeof(struct DNS_HEADER)];

	_convert_hostname_format(qname, host);
	qinfo = (struct QUESTION *)&buf[sizeof(struct DNS_HEADER) + (strlen((const char *)qname) + 1)]; // fill it

	qinfo->qtype = htons(query_type); // type of the query , A , MX , CNAME , NS etc
	qinfo->qclass = htons(1);		  // its internet (lol)

	printf("%s\n", STATE_SENDING);
	if (sendto(s, (char *)buf, sizeof(struct DNS_HEADER) + (strlen((const char *)qname) + 1) + sizeof(struct QUESTION), 0, (struct sockaddr *)&dest, sizeof(dest)) < 0)
	{
		printf("%s\n", STATE_SEND_FAIL);
		close(s);
		return ret;
	}

	// Receive the answer
	i = sizeof dest;
	printf("%s\n", STATE_RECVING);
	if (recvfrom(s, (char *)buf, sizeof(buf), 0, (struct sockaddr *)&dest, (socklen_t *)&i) < 0)
	{
		printf("%s\n", STATE_RECV_FAIL);
		close(s);
		return ret;
	}

	close(s);

//	dns = (struct DNS_HEADER *)buf;

	// move ahead of the dns header and the query field
	reader = &buf[sizeof(struct DNS_HEADER) + (strlen((const char *)qname) + 1) + sizeof(struct QUESTION)];

	printf("\nThe response contains : ");
	printf("\n %d Questions.", ntohs(dns->q_count));
	printf("\n %d Answers.", ntohs(dns->ans_count));
	printf("\n %d Authoritative Servers.", ntohs(dns->auth_count));
	printf("\n %d Additional records.\n\n", ntohs(dns->add_count));

	// Start reading answers
	stop = 0;

	for (i = 0; i < ntohs(dns->ans_count); i++)
	{
		answers[i].name = _read_name(reader, buf, &stop);
		reader = reader + stop;

		answers[i].resource = (struct R_DATA *)(reader);
		reader = reader + sizeof(struct R_DATA);

		if (ntohs(answers[i].resource->type) == 1) // if its an ipv4 address
		{
			answers[i].rdata = (unsigned char *)malloc(ntohs(answers[i].resource->data_len));

			for (j = 0; j < ntohs(answers[i].resource->data_len); j++)
			{
				answers[i].rdata[j] = reader[j];
			}

			answers[i].rdata[ntohs(answers[i].resource->data_len)] = '\0';

			reader = reader + ntohs(answers[i].resource->data_len);
		}
		else
		{
			answers[i].rdata = _read_name(reader, buf, &stop);
			reader = reader + stop;
		}
	}

	// read authorities
	for (i = 0; i < ntohs(dns->auth_count); i++)
	{
		auth[i].name = _read_name(reader, buf, &stop);
		reader += stop;

		auth[i].resource = (struct R_DATA *)(reader);
		reader += sizeof(struct R_DATA);

		auth[i].rdata = _read_name(reader, buf, &stop);
		reader += stop;
	}

	// read additional
	for (i = 0; i < ntohs(dns->add_count); i++)
	{
		addit[i].name = _read_name(reader, buf, &stop);
		reader += stop;

		addit[i].resource = (struct R_DATA *)(reader);
		reader += sizeof(struct R_DATA);

		if (ntohs(addit[i].resource->type) == 1)
		{
			addit[i].rdata = (unsigned char *)malloc(ntohs(addit[i].resource->data_len));
			for (j = 0; j < ntohs(addit[i].resource->data_len); j++)
				addit[i].rdata[j] = reader[j];

			addit[i].rdata[ntohs(addit[i].resource->data_len)] = '\0';
			reader += ntohs(addit[i].resource->data_len);
		}
		else
		{
			addit[i].rdata = _read_name(reader, buf, &stop);
			reader += stop;
		}
	}

	// print answers
	printf("\nAnswer Records : %d \n", ntohs(dns->ans_count));
	for (i = 0; i < ntohs(dns->ans_count); i++)
	{
		printf("Name : %s ", answers[i].name);

		if (ntohs(answers[i].resource->type) == T_A) // IPv4 address
		{
			long *p;
			p = (long *)answers[i].rdata;
			a.sin_addr.s_addr = (*p); // working without ntohl
			printf("has IPv4 address : %s", inet_ntoa(a.sin_addr));
			if(ip && ip[0] == '\0')
				strlcpy(ip, inet_ntoa(a.sin_addr), ip_sz);
			ret = 0;
		}

		if (ntohs(answers[i].resource->type) == 5)
		{
			// Canonical name for an alias
			printf("has alias name : %s", answers[i].rdata);
		}
		printf("\n");
		//free data
		SAFE_FREE(answers[i].name);
		SAFE_FREE(answers[i].rdata);
	}

	// print authorities
	printf("\nAuthoritive Records : %d \n", ntohs(dns->auth_count));
	for (i = 0; i < ntohs(dns->auth_count); i++)
	{
		printf("Name : %s ", auth[i].name);
		if (ntohs(auth[i].resource->type) == 2)
		{
			printf("has nameserver : %s", auth[i].rdata);
		}
		printf("\n");
		//free data
		SAFE_FREE(auth[i].name);
		SAFE_FREE(auth[i].rdata);
	}

	// print additional resource records
	printf("\nAdditional Records : %d \n", ntohs(dns->add_count));
	for (i = 0; i < ntohs(dns->add_count); i++)
	{
		printf("Name : %s ", addit[i].name);
		if (ntohs(addit[i].resource->type) == 1)
		{
			long *p;
			p = (long *)addit[i].rdata;
			a.sin_addr.s_addr = (*p);
			printf("has IPv4 address : %s", inet_ntoa(a.sin_addr));
			if(ip && ip[0] == '\0')
				strlcpy(ip, inet_ntoa(a.sin_addr), ip_sz);
			ret = 0;
		}
		printf("\n");
		//free data
		SAFE_FREE(addit[i].name);
		SAFE_FREE(addit[i].rdata);
}
	return ret;
}

/*
 *
 * */
static u_char *_read_name(unsigned char *reader, unsigned char *buffer, int *count)
{
	unsigned char *name;
	unsigned int p = 0, jumped = 0, offset;
	int i, j;

	*count = 1;
	name = (unsigned char *)malloc(NAME_SZ);

	name[0] = '\0';

	// read the names in 3www6google3com format
	while (*reader != 0)
	{
		if (*reader >= 192)
		{
			offset = (*reader) * NAME_SZ + *(reader + 1) - 49152; // 49152 = 11000000 00000000 ;)
			reader = buffer + offset - 1;
			jumped = 1; // we have jumped to another location so counting wont go up!
		}
		else
		{
			name[p++] = *reader;
		}

		reader = reader + 1;

		if (jumped == 0)
		{
			*count = *count + 1; // if we havent jumped to another location then we can count up
		}
	}

	name[p] = '\0'; // string complete
	if (jumped == 1)
	{
		*count = *count + 1; // number of steps we actually moved forward in the packet
	}

	// now convert 3www6google3com0 to www.google.com
	for (i = 0; i < (int)strlen((const char *)name); i++)
	{
		p = name[i];
		for (j = 0; j < (int)p; j++)
		{
			name[i] = name[i + 1];
			i = i + 1;
		}
		name[i] = '.';
	}
	name[i - 1] = '\0'; // remove the last dot
	return name;
}

/*
 * This will convert www.google.com to 3www6google3com
 * got it :)
 * */
static void _convert_hostname_format(unsigned char *dns, unsigned char *host)
{
	int lock = 0, i;
	strcat((char *)host, ".");

	for (i = 0; i < strlen((char *)host); i++)
	{
		if (host[i] == '.')
		{
			*dns++ = i - lock;
			for (; lock < i; lock++)
			{
				*dns++ = host[lock];
			}
			lock++; // or lock=i+1;
		}
	}
	*dns++ = '\0';
}
