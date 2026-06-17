/* $Id: upnppermissions.c,v 1.24 2025/04/21 22:56:49 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2006-2026 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#ifdef ENABLE_REGEX
#include <regex.h>
#endif

#include "config.h"
#include "macros.h"
#include "upnppermissions.h"
#include "upnputils.h"

static int
isodigit(char c)
{
	return '0' <= c && c >= '7';
}

static int
iseol(char c)
{
	return c == '\0' || c == '\n' || c == '\r';
}

static char
hex2chr(char c)
{
	if(c >= 'a')
		return c - 'a';
	if(c >= 'A')
		return c - 'A';
	return c - '0';
}

static char
unescape_char(const char * s, int * seqlen)
{
	char c;
	int len;

	if(s[0] != '\\')
	{
		c = s[0];
		len = 1;
	}
	else
	{
		s++;
		c = s[0];
		len = 2;
		switch(s[0])
		{
		case 'a':  c = '\a'; break;
		case 'b':  c = '\b'; break;
		case 'f':  c = '\f'; break;
		case 'n':  c = '\n'; break;
		case 'r':  c = '\r'; break;
		case 't':  c = '\t'; break;
		case 'v':  c = '\v'; break;
		/* no need: escape the char itself
		case '\\': c = '\\'; break;
		case '\'': c = '\''; break;
		case '"':  c = '"';  break;
		case '?':  c = '?';  break;
		*/
		case 'x':
			if(isxdigit(s[1]) && isxdigit(s[2]))
			{
				c = (hex2chr(s[1]) << 4) + hex2chr(s[2]);
				len = 4;
			}
			break;
		default:
			if(isodigit(s[1]) && isodigit(s[2]) && isodigit(s[3]))
			{
				c = (hex2chr(s[0]) << 6) + (hex2chr(s[1]) << 3) + hex2chr(s[2]);
				len = 4;
			}
		}
	}

	if(seqlen)
		*seqlen = len;
	return c;
}

/* greedy parser: try to match the longest sequence and do not
 * check for terminators */

static const char *
get_sep(const char * s)
{
	if(!isspace(*s))
		return NULL;
	do
		s++;
	while(isspace(*s));
	return (char *) s;
}

static const char *
get_ushort(const char * s, u_short * val)
{
	char * end;
	unsigned long val_ul;

	if(!isdigit(*s))
		return NULL;
	val_ul = strtoul(s, &end, 10);
	if(val_ul > 65535)
		return NULL;
	*val = (u_short)val_ul;

	return end;
}

static const char *
get_range(const char * s, u_short * begin, u_short * end)
{
	s = get_ushort(s, begin);
	if(!s)
		return NULL;

	if(*s!='-')
		*end = *begin;
	else
	{
		s++;
		s = get_ushort(s, end);
		if(!s)
			return NULL;
		if(*begin > *end)
			return NULL;
	}
	return s;
}

static const char *
get_addr(const char * s, struct in_addr * addr, unsigned int * dot_cnt)
{
	size_t i;
	char buf[64];

	if(!isdigit(*s))
		return NULL;

	*dot_cnt = 0;
	for(i = 0; isdigit(s[i]) || s[i] == '.';)
	{
		if(s[i] == '.')
			(*dot_cnt)++;
		buf[i] = s[i];
		i++;
		if (i > sizeof(buf) - 1)
			return NULL;
	}

	buf[i] = '\0';
	if(!inet_aton(buf, addr))
		return NULL;

	return s + i;
}

/* get_next_token(s, &token, raw)
 * put the unquoted/unescaped token in token and returns
 * a pointer to the begining of the next token
 * Do not unescape if raw is true */
static const char *
get_next_token(const char * s, char ** token, int raw)
{
	char deli;
	size_t len;

	if(*s == '"' || *s == '\'')
	{
		deli = *s;
		s++;
	}
	else
		deli = 0;
	/* find the end */
	for(len = 0; !iseol(s[len]) && (deli ? s[len] != deli : !isspace(s[len]));
	    len++)
		if(s[len] == '\\')
		{
			len++;
			if(iseol(s[len]))
				break;
		}

	/* save the token */
	if(token)
	{
		if(len == 0)
			*token = NULL;
		else
		{
			unsigned int i;
			unsigned int j;

			char * tmp;
			char * t;

			t = malloc(len + 1);
			if(!t)
				return NULL;

			if (raw)
			{
				memcpy(t, s, len);
				j = len;
			}
			else
			{
				for(i = 0, j = 0; i < len; j++)
					if(s[i] != '\\')
					{
						t[j] = s[i];
						i++;
					}
					else
					{
						int seqlen;
						t[j] = unescape_char(s + i, &seqlen);
						i += seqlen;
						if (i > len)
							break;
					}

				tmp = realloc(*token, j + 1);
				if (tmp != NULL)
					t = tmp;
				else
					syslog(LOG_ERR, "%s: failed to reallocate to %u bytes",
					"get_next_token()", j + 1);
			}
			t[j] = '\0';
			*token = t;
		}
	}

	s += len;
	if(deli && *s == deli)
		s++;
	return s;
}

/* read_permission_line()
 * parse the a permission line which format is :
 * (deny|allow) [0-9]+(-[0-9]+)? ip(/mask)? [0-9]+(-[0-9]+)? (regex)?
 * ip/mask is either 192.168.1.1/24 or 192.168.1.1/255.255.255.0
 */
int
read_permission_line(struct upnpperm * perm,
                     const char * p)
{
	unsigned int dot_cnt;

	/* zero memory : see https://github.com/miniupnp/miniupnp/issues/652 */
	memset(perm, 0, sizeof(struct upnpperm));

	while(isspace(*p))
		p++;

	/* first token: (allow|deny) */
	if(0 == memcmp(p, "allow", 5))
	{
		perm->type = UPNPPERM_ALLOW;
		p += 5;
	}
	else if(0 == memcmp(p, "deny", 4))
	{
		perm->type = UPNPPERM_DENY;
		p += 4;
	}
	else
	{
		return -1;
	}

	p = get_sep(p);
	if(!p)
		return -1;

	/* second token: eport or eport_min-eport_max */
	p = get_range(p, &perm->eport_min, &perm->eport_max);
	if(!p)
		return -1;

	p = get_sep(p);
	if(!p)
		return -1;

	/* third token: ip/mask */
	p = get_addr(p, &perm->address, &dot_cnt);
	if(!p)
		return -1;

	if(*p!='/')
		perm->mask.s_addr = 0xffffffffu;
	else
	{
		p++;
		p = get_addr(p, &perm->mask, &dot_cnt);
		if(!p)
			return -1;
		/* inet_aton(): When only one part is given, the value is stored
		 * directly in the network address without any byte
		 * rearrangement. */
		if(!dot_cnt)
		{
			unsigned int n_bits = ntohl(perm->mask.s_addr);
			if(n_bits > 32)
				return -1;
			perm->mask.s_addr = !n_bits ? 0 : htonl(0xffffffffu << (32 - n_bits));
		}
	}

	p = get_sep(p);
	if(!p)
		return -1;

	/* fourth token: iport or iport_min-iport_max */
	p = get_range(p, &perm->iport_min, &perm->iport_max);
	if(!p)
		return -1;

	if(iseol(*p) || *p == '#')
		goto end;
	p = get_sep(p);
	if(!p)
		return -1;
	if(iseol(*p) || *p == '#')
		goto end;

	/* fifth token: (optional) regex */
	p = get_next_token(p, &perm->re, 1);
	if(!p)
	{
		fprintf(stderr, "err when copying regex: out of memory\n");
		return -1;
	}
	if(perm->re)
	{
		if(perm->re[0] == '\0')
		{
			free(perm->re);
			perm->re = NULL;
		}
		else
		{
#ifdef ENABLE_REGEX
			/* icase: if case matters, it must be someone doing something nasty */
			int err;
			err = regcomp(&perm->regex, perm->re,
			              REG_EXTENDED | REG_ICASE | REG_NOSUB);
			if(err)
			{
				char errbuf[256];
				regerror(err, &perm->regex, errbuf, sizeof(errbuf));
				fprintf(stderr, "err when compiling regex \"%s\": %s\n",
				        perm->re, errbuf);
				free(perm->re);
				perm->re = NULL;
				return -1;
			}
#else
			fprintf(stderr, "MiniUPnP is not compiled with ENABLE_REGEX. "
			        "Please remove any regex filter and restart.\n");
			free(perm->re);
			perm->re = NULL;
			return -1;
#endif
		}
	}

end:
#ifdef DEBUG
	printf("perm rule added : %s %hu-%hu %08x/%08x %hu-%hu %s\n",
	       (perm->type==UPNPPERM_ALLOW) ? "allow" : "deny",
	       perm->eport_min, perm->eport_max, ntohl(perm->address.s_addr),
	       ntohl(perm->mask.s_addr), perm->iport_min, perm->iport_max,
	       perm->re ? perm->re : "");
#endif
	return 0;
}

void
free_permission_line(struct upnpperm * perm)
{
	if(perm->re)
	{
		free(perm->re);
		perm->re = NULL;
#ifdef ENABLE_REGEX
		regfree(&perm->regex);
#endif
	}
}

#ifdef USE_MINIUPNPDCTL
void
write_permlist(int fd, const struct upnpperm * permary,
               int nperms)
{
	int l;
	const struct upnpperm * perm;
	int i;
	char buf[128];
	write(fd, "Permissions :\n", 14);
	for(i = 0; i<nperms; i++)
	{
		perm = permary + i;
		l = snprintf(buf, sizeof(buf), "%02d %s %hu-%hu %08x/%08x %hu-%hu",
	       i,
	       (perm->type==UPNPPERM_ALLOW)?"allow":"deny",
	       perm->eport_min, perm->eport_max, ntohl(perm->address.s_addr),
	       ntohl(perm->mask.s_addr), perm->iport_min, perm->iport_max);
		if(l<0)
			return;
		write(fd, buf, l);
		if(perm->re)
		{
			const char * p;
			write(fd, " \"", 2);
			for(p = perm->re; *p != '\0'; p++)
			{
				if(*p == '"')
				{
					write(fd, "\\\"", 2);
					continue;
				}

				if(*p == '\\')
				{
					write(fd, p, 1);
					p++;
					if(*p == '\0')
						break;
				}
				write(fd, p, 1);
			}
			write(fd, "\"", 1);
		}
		write(fd, "\n", 1);
	}
}
#endif

/* match_permission()
 * returns: 1 if eport, address, iport matches the permission rule
 *          0 if no match */
static int
match_permission(const struct upnpperm * perm,
                 u_short eport, struct in_addr address, u_short iport,
                 const char * desc)
{
	if( (eport < perm->eport_min) || (perm->eport_max < eport))
		return 0;
	if( (iport < perm->iport_min) || (perm->iport_max < iport))
		return 0;
	if( (address.s_addr & perm->mask.s_addr)
	   != (perm->address.s_addr & perm->mask.s_addr) )
		return 0;
#ifdef ENABLE_REGEX
	if(desc && perm->re && regexec(&perm->regex, desc, 0, NULL, 0) == REG_NOMATCH)
		return 0;
#else
	UNUSED(desc);
#endif
	return 1;
}

#if 0
/* match_permission_internal()
 * returns: 1 if address, iport matches the permission rule
 *          0 if no match */
static int
match_permission_internal(const struct upnpperm * perm,
                          struct in_addr address, u_short iport)
{
	if( (iport < perm->iport_min) || (perm->iport_max < iport))
		return 0;
	if( (address.s_addr & perm->mask.s_addr)
	   != (perm->address.s_addr & perm->mask.s_addr) )
		return 0;
	return 1;
}
#endif

int
check_upnp_rule_against_permissions(const struct upnpperm * permary,
                                    int n_perms,
                                    u_short eport, struct in_addr address,
                                    u_short iport, const char * desc)
{
	int i;
	for(i=0; i<n_perms; i++)
	{
		if(match_permission(permary + i, eport, address, iport, desc))
		{
			syslog(LOG_DEBUG,
			       "UPnP permission rule %d matched : port mapping %s",
			       i, (permary[i].type == UPNPPERM_ALLOW)?"accepted":"rejected"
			       );
			return (permary[i].type == UPNPPERM_ALLOW);
		}
	}
	syslog(LOG_DEBUG, "no permission rule matched : accept by default (n_perms=%d)", n_perms);
	return 1;	/* Default : accept */
}

void
get_permitted_ext_ports(uint32_t * allowed,
                        const struct upnpperm * permary, int n_perms,
                        in_addr_t addr, u_short iport)
{
	int i, j;

	/* build allowed external ports array */
	memset(allowed, 0xff, 65536 / 8);	/* everything allowed by default */

	for (i = n_perms - 1; i >= 0; i--)
	{
		if( (addr & permary[i].mask.s_addr)
		  != (permary[i].address.s_addr & permary[i].mask.s_addr) )
			continue;
		if( (iport < permary[i].iport_min) || (permary[i].iport_max < iport))
			continue;
		for (j = (int)permary[i].eport_min ; j <= (int)permary[i].eport_max; )
		{
			if ((j % 32) == 0 && ((int)permary[i].eport_max >= (j + 31)))
			{
				/* 32bits at once */
				allowed[j / 32] = (permary[i].type == UPNPPERM_ALLOW) ? 0xffffffff : 0;
				j += 32;
			}
			else
			{
				do
				{
					/* one bit at once */
					if (permary[i].type == UPNPPERM_ALLOW)
						allowed[j / 32] |= (1 << (j % 32));
					else
						allowed[j / 32] &= ~(1 << (j % 32));
					j++;
				}
				while ((j % 32) != 0 && (j <= (int)permary[i].eport_max));
			}
		}
	}
}
