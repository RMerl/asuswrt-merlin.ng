/* dnsmasq is Copyright (c) 2000-2024 Simon Kelley

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 dated June, 1991, or
   (at your option) version 3 dated 29 June, 2007.
 
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
      
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* The SURF random number generator was taken from djbdns-1.05, by 
   Daniel J Bernstein, which is public domain. */


#include "dnsmasq.h"

#ifdef HAVE_BROKEN_RTC
#include <sys/times.h>
#endif

#if defined(HAVE_LIBIDN2)
#include <idn2.h>
#elif defined(HAVE_IDN)
#include <idna.h>
#endif

#ifdef HAVE_LINUX_NETWORK
#include <sys/utsname.h>
#endif

/* SURF random number generator */

static u32 seed[32];
static u32 in[12];
static u32 out[8];
static int outleft = 0;

void rand_init()
{
  int fd = open(RANDFILE, O_RDONLY);
  
  if (fd == -1 ||
      !read_write(fd, (unsigned char *)&seed, sizeof(seed), 1) ||
      !read_write(fd, (unsigned char *)&in, sizeof(in), 1))
    die(_("failed to seed the random number generator: %s"), NULL, EC_MISC);
  
  close(fd);
}

#define ROTATE(x,b) (((x) << (b)) | ((x) >> (32 - (b))))
#define MUSH(i,b) x = t[i] += (((x ^ seed[i]) + sum) ^ ROTATE(x,b));

static void surf(void)
{
  u32 t[12]; u32 x; u32 sum = 0;
  int r; int i; int loop;

  for (i = 0;i < 12;++i) t[i] = in[i] ^ seed[12 + i];
  for (i = 0;i < 8;++i) out[i] = seed[24 + i];
  x = t[11];
  for (loop = 0;loop < 2;++loop) {
    for (r = 0;r < 16;++r) {
      sum += 0x9e3779b9;
      MUSH(0,5) MUSH(1,7) MUSH(2,9) MUSH(3,13)
      MUSH(4,5) MUSH(5,7) MUSH(6,9) MUSH(7,13)
      MUSH(8,5) MUSH(9,7) MUSH(10,9) MUSH(11,13)
    }
    for (i = 0;i < 8;++i) out[i] ^= t[i + 4];
  }
}

unsigned short rand16(void)
{
  if (!outleft) 
    {
      if (!++in[0]) if (!++in[1]) if (!++in[2]) ++in[3];
      surf();
      outleft = 8;
    }
  
  return (unsigned short) out[--outleft];
}

u32 rand32(void)
{
 if (!outleft) 
    {
      if (!++in[0]) if (!++in[1]) if (!++in[2]) ++in[3];
      surf();
      outleft = 8;
    }
  
  return out[--outleft]; 
}

u64 rand64(void)
{
  static int outleft = 0;

  if (outleft < 2)
    {
      if (!++in[0]) if (!++in[1]) if (!++in[2]) ++in[3];
      surf();
      outleft = 8;
    }
  
  outleft -= 2;

  return (u64)out[outleft+1] + (((u64)out[outleft]) << 32);
}

int rr_on_list(struct rrlist *list, unsigned short rr)
{
  while (list)
    {
      if (list->rr == rr)
	return 1;

      list = list->next;
    }

  return 0;
}

/* returns 1 if name is OK and ascii printable
 * returns 2 if name should be processed by IDN */
static int check_name(char *in)
{
  /* remove trailing . 
     also fail empty string and label > 63 chars */
  size_t dotgap = 0, l = strlen(in);
  char c;
  int nowhite = 0;
  int idn_encode = 0;
  int hasuscore = 0;
  int hasucase = 0;
  
  if (l == 0 || l > MAXDNAME) return 0;
  
  if (in[l-1] == '.')
    {
      in[l-1] = 0;
      nowhite = 1;
    }

  for (; (c = *in); in++)
    {
      if (c == '.')
        dotgap = 0;
      else if (++dotgap > MAXLABEL)
        return 0;
      else if (isascii((unsigned char)c) && iscntrl((unsigned char)c)) 
        /* iscntrl only gives expected results for ascii */
        return 0;
      else if (!isascii((unsigned char)c))
#if !defined(HAVE_IDN) && !defined(HAVE_LIBIDN2)
        return 0;
#else
        idn_encode = 1;
#endif
      else if (c != ' ')
        {
          nowhite = 1;
#if defined(HAVE_LIBIDN2) && (!defined(IDN2_VERSION_NUMBER) || IDN2_VERSION_NUMBER < 0x02000003)
          if (c == '_')
            hasuscore = 1;
#else
          (void)hasuscore;
#endif

#if defined(HAVE_IDN) || defined(HAVE_LIBIDN2)
          if (c >= 'A' && c <= 'Z')
            hasucase = 1;
#else
          (void)hasucase;
#endif
        }
    }

  if (!nowhite)
    return 0;

#if defined(HAVE_LIBIDN2) && (!defined(IDN2_VERSION_NUMBER) || IDN2_VERSION_NUMBER < 0x02000003)
  /* Older libidn2 strips underscores, so don't do IDN processing
     if the name has an underscore unless it also has non-ascii characters. */
  idn_encode = idn_encode || (hasucase && !hasuscore);
#else
  idn_encode = idn_encode || hasucase;
#endif

  return (idn_encode) ? 2 : 1;
}

/* Hostnames have a more limited valid charset than domain names
   so check for legal char a-z A-Z 0-9 - _ 
   Note that this may receive a FQDN, so only check the first label 
   for the tighter criteria. */
static int check_hostname(char *name)
{
  char c, *at, *src = name;
  int first;

  if (!check_name(name))
    return -1;

  at = strchr(name, '@');

  for (first = 1; (c = *name); name++, first = 0)
    /* check for legal char a-z A-Z 0-9 - _ . */
    {
      if ((c >= 'A' && c <= 'Z') ||
	  (c >= 'a' && c <= 'z') ||
	  (c >= '0' && c <= '9'))
	continue;

      if (!first && (c == '-' || c == '_'))
	continue;

      /* relax name part */
      if (at && (name <= at) && (c >= 33) && (c < 127))
	continue;
      
      /* end of hostname part */
      if (c == '.')
	break;
      
      return -1;
    }
  
  return name - src;
}

int legal_hostname(char *name)
{
  return check_hostname(name) >= 0;
}

int valid_hostname(char *name)
{
  static const char *reserved[] = {
    "localhost",
    "ip6-localhost",
    "ip6-loopback",
    "wpad",
    "isatap",
    NULL
  };
  const char **next;
  int len;

  len = check_hostname(name);
  if (len < 0)
    return 0;

  for (next = reserved; *next; next++)
    {
      if (strncasecmp(name, *next, len) == 0 && len == strlen(*next))
      return 0;
    }
  
  return 1;
}
  
char *canonicalise(char *in, int *nomem)
{
  char *ret = NULL;
  int rc;
  
  if (nomem)
    *nomem = 0;
  
  if (!(rc = check_name(in)))
    return NULL;
  
#if defined(HAVE_IDN) || defined(HAVE_LIBIDN2)
  if (rc == 2)
    {
#  ifdef HAVE_LIBIDN2
      rc = idn2_to_ascii_lz(in, &ret, IDN2_NONTRANSITIONAL);
#  else
      rc = idna_to_ascii_lz(in, &ret, 0);
#  endif
      if (rc != IDNA_SUCCESS)
	{
	  if (ret)
	    free(ret);
	  
	  if (nomem && (rc == IDNA_MALLOC_ERROR || rc == IDNA_DLOPEN_ERROR))
	    {
	      my_syslog(LOG_ERR, _("failed to allocate memory"));
	      *nomem = 1;
	    }
	  
	  return NULL;
	}
      
      return ret;
    }
#else
  (void)rc;
#endif
  
  if ((ret = whine_malloc(strlen(in)+1)))
    strcpy(ret, in);
  else if (nomem)
    *nomem = 1;

  return ret;
}

unsigned char *do_rfc1035_name(unsigned char *p, char *sval, char *limit)
{
  int j;
  
  while (sval && *sval)
    {
      unsigned char *cp = p++;

      if (limit && p > (unsigned char*)limit)
        return NULL;

      for (j = 0; *sval && (*sval != '.'); sval++, j++)
	{
          if (limit && p + 1 > (unsigned char*)limit)
            return NULL;

	  if (*sval == NAME_ESCAPE)
	    *p++ = (*(++sval))-1;
	  else
	    *p++ = *sval;
	}
      
      *cp  = j;
      if (*sval)
	sval++;
    }
  
  return p;
}

/* for use during startup */
void *safe_malloc(size_t size)
{
  void *ret = calloc(1, size);
  
  if (!ret)
    die(_("could not get memory"), NULL, EC_NOMEM);
      
  return ret;
}

/* Ensure limited size string is always terminated.
 * Can be replaced by (void)strlcpy() on some platforms */
void safe_strncpy(char *dest, const char *src, size_t size)
{
  if (size != 0)
    {
      dest[size-1] = '\0';
      strncpy(dest, src, size-1);
    }
}

void safe_pipe(int *fd, int read_noblock)
{
  if (pipe(fd) == -1 || 
      !fix_fd(fd[1]) ||
      (read_noblock && !fix_fd(fd[0])))
    die(_("cannot create pipe: %s"), NULL, EC_MISC);
}

void *whine_malloc(size_t size)
{
  void *ret = calloc(1, size);

  if (!ret)
    my_syslog(LOG_ERR, _("failed to allocate %d bytes"), (int) size);
  
  return ret;
}

void *whine_realloc(void *ptr, size_t size)
{
  void *ret = realloc(ptr, size);

  if (!ret)
    my_syslog(LOG_ERR, _("failed to reallocate %d bytes"), (int) size);

  return ret;
}

int sockaddr_isequal(const union mysockaddr *s1, const union mysockaddr *s2)
{
  if (s1->sa.sa_family == s2->sa.sa_family)
    { 
      if (s1->sa.sa_family == AF_INET &&
	  s1->in.sin_port == s2->in.sin_port &&
	  s1->in.sin_addr.s_addr == s2->in.sin_addr.s_addr)
	return 1;
      
      if (s1->sa.sa_family == AF_INET6 &&
	  s1->in6.sin6_port == s2->in6.sin6_port &&
	  s1->in6.sin6_scope_id == s2->in6.sin6_scope_id &&
	  IN6_ARE_ADDR_EQUAL(&s1->in6.sin6_addr, &s2->in6.sin6_addr))
	return 1;
    }
  return 0;
}

int sockaddr_isnull(const union mysockaddr *s)
{
  if (s->sa.sa_family == AF_INET &&
      s->in.sin_addr.s_addr == 0)
    return 1;
  
  if (s->sa.sa_family == AF_INET6 &&
      IN6_IS_ADDR_UNSPECIFIED(&s->in6.sin6_addr))
    return 1;
  
  return 0;
}

int sa_len(union mysockaddr *addr)
{
#ifdef HAVE_SOCKADDR_SA_LEN
  return addr->sa.sa_len;
#else
  if (addr->sa.sa_family == AF_INET6)
    return sizeof(addr->in6);
  else
    return sizeof(addr->in); 
#endif
}

/* don't use strcasecmp and friends here - they may be messed up by LOCALE */
int hostname_order(const char *a, const char *b)
{
  unsigned int c1, c2;
  
  do {
    c1 = (unsigned char) *a++;
    c2 = (unsigned char) *b++;
    
    if (c1 >= 'A' && c1 <= 'Z')
      c1 += 'a' - 'A';
    if (c2 >= 'A' && c2 <= 'Z')
      c2 += 'a' - 'A';
    
    if (c1 < c2)
      return -1;
    else if (c1 > c2)
      return 1;
    
  } while (c1);
  
  return 0;
}

int hostname_isequal(const char *a, const char *b)
{
  return hostname_order(a, b) == 0;
}

/* is b equal to or a subdomain of a return 2 for equal, 1 for subdomain */
int hostname_issubdomain(char *a, char *b)
{
  char *ap, *bp;
  unsigned int c1, c2;
  
  /* move to the end */
  for (ap = a; *ap; ap++); 
  for (bp = b; *bp; bp++);

  /* a shorter than b or a empty. */
  if ((bp - b) < (ap - a) || ap == a)
    return 0;

  do
    {
      c1 = (unsigned char) *(--ap);
      c2 = (unsigned char) *(--bp);
  
       if (c1 >= 'A' && c1 <= 'Z')
	 c1 += 'a' - 'A';
       if (c2 >= 'A' && c2 <= 'Z')
	 c2 += 'a' - 'A';

       if (c1 != c2)
	 return 0;
    } while (ap != a);

  if (bp == b)
    return 2;

  if (*(--bp) == '.')
    return 1;

  return 0;
}
 
  
time_t dnsmasq_time(void)
{
#ifdef HAVE_BROKEN_RTC
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
    die(_("cannot read monotonic clock: %s"), NULL, EC_MISC);

  return ts.tv_sec;
#else
  return time(NULL);
#endif
}

u32 dnsmasq_milliseconds(void)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);

  return (tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}

int netmask_length(struct in_addr mask)
{
  int zero_count = 0;

  while (0x0 == (mask.s_addr & 0x1) && zero_count < 32) 
    {
      mask.s_addr >>= 1;
      zero_count++;
    }
  
  return 32 - zero_count;
}

int is_same_net(struct in_addr a, struct in_addr b, struct in_addr mask)
{
  return (a.s_addr & mask.s_addr) == (b.s_addr & mask.s_addr);
}

int is_same_net_prefix(struct in_addr a, struct in_addr b, int prefix)
{
  struct in_addr mask;

  mask.s_addr = htonl(~((1 << (32 - prefix)) - 1));

  return is_same_net(a, b, mask);
}


int is_same_net6(struct in6_addr *a, struct in6_addr *b, int prefixlen)
{
  int pfbytes = prefixlen >> 3;
  int pfbits = prefixlen & 7;

  if (memcmp(&a->s6_addr, &b->s6_addr, pfbytes) != 0)
    return 0;

  if (pfbits == 0 ||
      (a->s6_addr[pfbytes] >> (8 - pfbits) == b->s6_addr[pfbytes] >> (8 - pfbits)))
    return 1;

  return 0;
}

/* return least significant 64 bits if IPv6 address */
u64 addr6part(struct in6_addr *addr)
{
  int i;
  u64 ret = 0;

  for (i = 8; i < 16; i++)
    ret = (ret << 8) + addr->s6_addr[i];

  return ret;
}

void setaddr6part(struct in6_addr *addr, u64 host)
{
  int i;

  for (i = 15; i >= 8; i--)
    {
      addr->s6_addr[i] = host;
      host = host >> 8;
    }
}


/* returns port number from address */
int prettyprint_addr(union mysockaddr *addr, char *buf)
{
  int port = 0;
  
  if (addr->sa.sa_family == AF_INET)
    {
      inet_ntop(AF_INET, &addr->in.sin_addr, buf, ADDRSTRLEN);
      port = ntohs(addr->in.sin_port);
    }
  else if (addr->sa.sa_family == AF_INET6)
    {
      char name[IF_NAMESIZE];
      inet_ntop(AF_INET6, &addr->in6.sin6_addr, buf, ADDRSTRLEN);
      if (addr->in6.sin6_scope_id != 0 &&
	  if_indextoname(addr->in6.sin6_scope_id, name) &&
	  strlen(buf) + strlen(name) + 2 <= ADDRSTRLEN)
	{
	  strcat(buf, "%");
	  strcat(buf, name);
	}
      port = ntohs(addr->in6.sin6_port);
    }
  
  return port;
}

void prettyprint_time(char *buf, unsigned int t)
{
  if (t == 0xffffffff)
    sprintf(buf, _("infinite"));
  else
    {
      unsigned int x, p = 0;
       if ((x = t/86400))
	p += sprintf(&buf[p], "%ud", x);
       if ((x = (t/3600)%24))
	p += sprintf(&buf[p], "%uh", x);
      if ((x = (t/60)%60))
	p += sprintf(&buf[p], "%um", x);
      if ((x = t%60))
	sprintf(&buf[p], "%us", x);
    }
}


/* in may equal out, when maxlen may be -1 (No max len). 
   Return -1 for extraneous no-hex chars found. */
int parse_hex(char *in, unsigned char *out, int maxlen, 
	      unsigned int *wildcard_mask, int *mac_type)
{
  int done = 0, mask = 0, i = 0;
  char *r;
    
  if (mac_type)
    *mac_type = 0;
  
  while (!done && (maxlen == -1 || i < maxlen))
    {
      for (r = in; *r != 0 && *r != ':' && *r != '-' && *r != ' '; r++)
	if (*r != '*' && !isxdigit((unsigned char)*r))
	  return -1;
      
      if (*r == 0)
	done = 1;
      
      if (r != in )
	{
	  if (*r == '-' && i == 0 && mac_type)
	   {
	      *r = 0;
	      *mac_type = strtol(in, NULL, 16);
	      mac_type = NULL;
	   }
	  else
	    {
	      *r = 0;
	      if (strcmp(in, "*") == 0)
		{
		  mask = (mask << 1) | 1;
		  i++;
		}
	      else
		{
		  int j, bytes = (1 + (r - in))/2;
		  for (j = 0; j < bytes; j++)
		    { 
		      char sav;
		      if (j < bytes - 1)
			{
			  sav = in[(j+1)*2];
			  in[(j+1)*2] = 0;
			}
		      /* checks above allow mix of hexdigit and *, which
			 is illegal. */
		      if (strchr(&in[j*2], '*'))
			return -1;
		      out[i] = strtol(&in[j*2], NULL, 16);
		      mask = mask << 1;
		      if (++i == maxlen)
			break; 
		      if (j < bytes - 1)
			in[(j+1)*2] = sav;
		    }
		}
	    }
	}
      in = r+1;
    }
  
  if (wildcard_mask)
    *wildcard_mask = mask;

  return i;
}

/* return 0 for no match, or (no matched octets) + 1 */
int memcmp_masked(unsigned char *a, unsigned char *b, int len, unsigned int mask)
{
  int i, count;
  for (count = 1, i = len - 1; i >= 0; i--, mask = mask >> 1)
    if (!(mask & 1))
      {
	if (a[i] == b[i])
	  count++;
	else
	  return 0;
      }
  return count;
}

/* _note_ may copy buffer */
int expand_buf(struct iovec *iov, size_t size)
{
  void *new;

  if (size <= (size_t)iov->iov_len)
    return 1;

  if (!(new = whine_malloc(size)))
    {
      errno = ENOMEM;
      return 0;
    }

  if (iov->iov_base)
    {
      memcpy(new, iov->iov_base, iov->iov_len);
      free(iov->iov_base);
    }

  iov->iov_base = new;
  iov->iov_len = size;

  return 1;
}

char *print_mac(char *buff, unsigned char *mac, int len)
{
  char *p = buff;
  int i;
   
  if (len == 0)
    sprintf(p, "<null>");
  else
    for (i = 0; i < len; i++)
      p += sprintf(p, "%.2x%s", mac[i], (i == len - 1) ? "" : ":");
  
  return buff;
}

/* rc is return from sendto and friends.
   Return 1 if we should retry.
   Set errno to zero if we succeeded. */
int retry_send(ssize_t rc)
{
  static int retries = 0;
  struct timespec waiter;
  
  if (rc != -1)
    {
      retries = 0;
      errno = 0;
      return 0;
    }
  
  /* Linux kernels can return EAGAIN in perpetuity when calling
     sendmsg() and the relevant interface has gone. Here we loop
     retrying in EAGAIN for 1 second max, to avoid this hanging 
     dnsmasq. */

  if (errno == EAGAIN || errno == EWOULDBLOCK)
     {
       waiter.tv_sec = 0;
       waiter.tv_nsec = 10000;
       nanosleep(&waiter, NULL);
       if (retries++ < 1000)
	 return 1;
     }
  
  retries = 0;
  
  if (errno == EINTR)
    return 1;
  
  return 0;
}

int read_write(int fd, unsigned char *packet, int size, int rw)
{
  ssize_t n, done;
  
  for (done = 0; done < size; done += n)
    {
      do { 
	if (rw)
	  n = read(fd, &packet[done], (size_t)(size - done));
	else
	  n = write(fd, &packet[done], (size_t)(size - done));
	
	if (n == 0)
	  return 0;
	
      } while (retry_send(n) || errno == ENOMEM || errno == ENOBUFS);

      if (errno != 0)
	return 0;
    }
     
  return 1;
}

/* close all fds except STDIN, STDOUT and STDERR, spare1, spare2 and spare3 */
void close_fds(long max_fd, int spare1, int spare2, int spare3) 
{
  /* On Linux, use the /proc/ filesystem to find which files
     are actually open, rather than iterate over the whole space,
     for efficiency reasons. If this fails we drop back to the dumb code. */
#ifdef HAVE_LINUX_NETWORK 
  DIR *d;
  
  if ((d = opendir("/proc/self/fd")))
    {
      struct dirent *de;

      while ((de = readdir(d)))
	{
	  long fd;
	  char *e = NULL;
	  
	  errno = 0;
	  fd = strtol(de->d_name, &e, 10);
	  	  
      	  if (errno != 0 || !e || *e || fd == dirfd(d) ||
	      fd == STDOUT_FILENO || fd == STDERR_FILENO || fd == STDIN_FILENO ||
	      fd == spare1 || fd == spare2 || fd == spare3)
	    continue;
	  
	  close(fd);
	}
      
      closedir(d);
      return;
  }
#endif

  /* fallback, dumb code. */
  for (max_fd--; max_fd >= 0; max_fd--)
    if (max_fd != STDOUT_FILENO && max_fd != STDERR_FILENO && max_fd != STDIN_FILENO &&
	max_fd != spare1 && max_fd != spare2 && max_fd != spare3)
      close(max_fd);
}

/* Basically match a string value against a wildcard pattern.  */
int wildcard_match(const char* wildcard, const char* match)
{
  while (*wildcard && *match)
    {
      if (*wildcard == '*')
        return 1;

      if (*wildcard != *match)
        return 0; 

      ++wildcard;
      ++match;
    }

  return *wildcard == *match;
}

/* The same but comparing a maximum of NUM characters, like strncmp.  */
int wildcard_matchn(const char* wildcard, const char* match, int num)
{
  while (*wildcard && *match && num)
    {
      if (*wildcard == '*')
        return 1;

      if (*wildcard != *match)
        return 0; 

      ++wildcard;
      ++match;
      --num;
    }

  return (!num) || (*wildcard == *match);
}

#ifdef HAVE_LINUX_NETWORK
int kernel_version(void)
{
  struct utsname utsname;
  int version;
  char *split;
  
  if (uname(&utsname) < 0)
    die(_("failed to find kernel version: %s"), NULL, EC_MISC);
  
  split = strtok(utsname.release, ".");
  version = (split ? atoi(split) : 0);
  split = strtok(NULL, ".");
  version = version * 256 + (split ? atoi(split) : 0);
  split = strtok(NULL, ".");
  return version * 256 + (split ? atoi(split) : 0);
}
#endif
