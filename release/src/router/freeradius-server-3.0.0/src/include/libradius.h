/*
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */
#ifndef LIBRADIUS_H
#define LIBRADIUS_H
/*
 * $Id$
 *
 * @file libradius.h
 * @brief Structures and prototypes for the radius library.
 *
 * @copyright 1999-2008 The FreeRADIUS server project
 */
RCSIDH(libradius_h, "$Id$")

#include <freeradius-devel/missing.h>

#include <talloc.h>

/*
 *  Let any external program building against the library know what
 *  features the library was built with.
 */
#include <freeradius-devel/features.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include <freeradius-devel/radius.h>
#include <freeradius-devel/token.h>
#include <freeradius-devel/hash.h>

#ifdef SIZEOF_UNSIGNED_INT
#if SIZEOF_UNSIGNED_INT != 4
#error FATAL: sizeof(unsigned int) != 4
#endif
#endif

/*
 *  Include for modules.
 */
#include <freeradius-devel/sha1.h>
#include <freeradius-devel/md4.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WITH_VERIFY_PTR)
/*
 *	Requires typeof(), which is in most modern C compilers.
 */

/*
#define VERIFY_VP(_x) do { (void) talloc_get_type_abort(_x, VALUE_PAIR); \
			if (_x->da) { \
			   	(void) talloc_get_type_abort(_x->da, DICT_ATTR); \
			} \
		      } while (0)
*/
#define FREE_MAGIC (0xF4EEF4EE)

#define VERIFY_VP(_x) (void) talloc_get_type_abort(_x, VALUE_PAIR)
#define VERIFY_PACKET(_x) (void) talloc_get_type_abort(_x, RADIUS_PACKET)
#else
#define VERIFY_VP(_x)
#define VERIFY_PACKET(_x)
#endif

#define AUTH_VECTOR_LEN		16
#define CHAP_VALUE_LENGTH       16
#define MAX_STRING_LEN		254	/* RFC2138: string 0-253 octets */
#define FR_MAX_VENDOR		(1 << 24) /* RFC limitations */

#ifdef _LIBRADIUS
#  define AUTH_HDR_LEN		20
#  define VENDORPEC_USR		429
#define VENDORPEC_LUCENT	4846
#define VENDORPEC_STARENT	8164
#  define DEBUG			if (fr_debug_flag && fr_log_fp) fr_printf_log
#  define debug_pair(vp)	do { if (fr_debug_flag && fr_log_fp) { \
					vp_print(fr_log_fp, vp); \
				     } \
				} while(0)
#endif

#define TAG_VALID(x)		((x) > 0 && (x) < 0x20)
#define TAG_VALID_ZERO(x)	((x) < 0x20)
#define TAG_ANY			-128	/* minimum signed char */
#define TAG_UNUSED		0

#if defined(__GNUC__)
# define PRINTF_LIKE(n) __attribute__ ((format(printf, n, n+1)))
# define NEVER_RETURNS __attribute__ ((noreturn))
# define UNUSED __attribute__ ((unused))
# define BLANK_FORMAT " "	/* GCC_LINT whines about empty formats */
#else
# define PRINTF_LIKE(n)	/* ignore */
# define NEVER_RETURNS /* ignore */
# define UNUSED /* ignore */
# define BLANK_FORMAT ""
#endif

typedef struct attr_flags {
	unsigned int 	is_unknown : 1;				//!< Attribute number or vendor is unknown.
	unsigned int	is_tlv : 1;				//!< Is a sub attribute.
	unsigned int	vp_free : 1;				//!< Should be freed when VALUE_PAIR is freed.

	unsigned int	has_tag : 1;				//!< Tagged attribute.
	unsigned int	array : 1; 				//!< Pack multiples into 1 attr.
	unsigned int	has_value : 1;				//!< Has a value.
	unsigned int	has_value_alias : 1; 			//!< Has a value alias.
	unsigned int	has_tlv : 1; 				//!< Has sub attributes.

	unsigned int	extended : 1; 				//!< Extended attribute.
	unsigned int	long_extended : 1; 			//!< Long format.
	unsigned int	evs : 1;				//!< Extended VSA.
	unsigned int	wimax: 1;				//!< WiMAX format=1,1,c.

	unsigned int	concat : 1;				//!< concatenate multiple instances
	unsigned int	is_pointer : 1;				//!< data is a pointer

	uint8_t		encrypt;      				//!< Ecryption method.
	uint8_t		length;
} ATTR_FLAGS;

/*
 *  Values of the encryption flags.
 */
#define FLAG_ENCRYPT_NONE	    (0)
#define FLAG_ENCRYPT_USER_PASSWORD   (1)
#define FLAG_ENCRYPT_TUNNEL_PASSWORD (2)
#define FLAG_ENCRYPT_ASCEND_SECRET   (3)

extern const FR_NAME_NUMBER dict_attr_types[];
extern const size_t dict_attr_sizes[PW_TYPE_MAX][2];

/** dictionary attribute
 *
 */
typedef struct dict_attr {
	unsigned int		attr;
	PW_TYPE			type;
	unsigned int		vendor;
	ATTR_FLAGS		flags;
	char			name[1];
} DICT_ATTR;

/** value of an enumerated attribute
 *
 */
typedef struct dict_value {
	unsigned int		attr;
	unsigned int		vendor;
	int			value;
	char			name[1];
} DICT_VALUE;

/** dictionary vendor
 *
 */
typedef struct dict_vendor {
	unsigned int		vendorpec;
	size_t			type; 				//!< Length of type data
	size_t			length;				//!< Length of length data
	size_t			flags;
	char			name[1];
} DICT_VENDOR;

/** Union containing all data types supported by the server
 *
 * This union contains all data types that can be represented with VALUE_PAIRs. It may also be used in other parts
 * of the server where values of different types need to be stored.
 *
 * PW_TYPE should be an enumeration of the values in this union.
 */
typedef union value_data {
	char const	        *strvalue;			//!< Pointer to UTF-8 string.
	uint8_t const		*octets;			//!< Pointer to binary string.
	struct in_addr		ipaddr;				//!< IPv4 Address.
	struct in6_addr		ipv6addr;			//!< IPv6 Address.
	uint32_t		date;				//!< Date (32bit Unix timestamp).
	uint32_t		integer;			//!< 32bit unsigned integer.
	int32_t			sinteger;			//!< 32bit signed integer.
	uint64_t		integer64;			//!< 64bit unsigned integer.
	size_t			filter[32/sizeof(size_t)];	//!< 64bit signed integer.
	uint8_t			ifid[8]; /* struct? */		//!< IPv6 interface ID.
	uint8_t			ipv6prefix[18]; /* struct? */	//!< IPv6 prefix.
	uint8_t			ipv4prefix[6]; /* struct? */	//!< IPv4 prefix.
     	uint8_t			ether[6];			//!< Ethernet (MAC) address.
	uint8_t			*tlv;				//!< Nested TLV (should go away).
	void const		*ptr;				//!< generic pointer.
} value_data_t;

/** The type of value a VALUE_PAIR contains
 *
 * This is used to add structure to nested VALUE_PAIRs and specifies what type of node it is (set, list, data).
 *
 * xlat is another type of data node which must first be expanded before use.
 */
typedef enum value_type {
	VT_NONE = 0,						//!< VALUE_PAIR has no value.
	VT_SET,							//!< VALUE_PAIR has children.
	VT_LIST,						//!< VALUE_PAIR has multiple values.
	VT_DATA,						//!< VALUE_PAIR has a single value.
	VT_XLAT							//!< valuepair value must be xlat expanded when it's
								//!< added to VALUE_PAIR tree.
} value_type_t;

/** Stores an attribute, a value and various bits of other data
 *
 * VALUE_PAIRs are the main data structure used in the server, they specify an attribute, it's children and
 * it's siblings.
 *
 * They also specify what behaviour should be used when the attribute is merged into a new list/tree.
 */
typedef struct value_pair {
	DICT_ATTR const		*da;				//!< Dictionary attribute defines the attribute
								//!< number, vendor and type of the attribute.

	struct value_pair	*next;

	FR_TOKEN		op;				//!< Operator to use when moving or inserting
								//!< valuepair into a list.

	int8_t			tag;				//!< Tag value used to group valuepairs.

	union {
	//	VALUE_SET	*set;				//!< Set of child attributes.
	//	VALUE_LIST	*list;				//!< List of values for
								//!< multivalued attribute.
	//	value_data_t	*data;				//!< Value data for this attribute.

		char const 	*xlat;				//!< Source string for xlat expansion.
	} value;

	value_type_t		type;				//!< Type of pointer in value union.

	size_t			length;				//!< of Data field.
	value_data_t		data;
} VALUE_PAIR;

/** Abstraction to allow iterating over different configurations of VALUE_PAIRs
 *
 * This allows functions which do not care about the structure of collections of VALUE_PAIRs
 * to iterate over all members in a collection.
 *
 * Field within a vp_cursor should not be accessed directly, and vp_cursors should only be
 * manipulated with the pair* functions.
 */
typedef struct vp_cursor {
	VALUE_PAIR	**first;
	VALUE_PAIR	*found;					//!< pairfind marker.
	VALUE_PAIR	*last;					//!< Temporary only used for pairinsert
	VALUE_PAIR	*current;				//!< The current attribute.
	VALUE_PAIR	*next;					//!< Next attribute to process.
} vp_cursor_t;

/** A VALUE_PAIR in string format.
 *
 * Used to represent pairs in the legacy 'users' file format.
 */
typedef struct value_pair_raw {
	char l_opand[64];					//!< Left hand side of the pair.
	char r_opand[1024];					//!< Right hand side of the pair.

	FR_TOKEN quote;						//!< Type of quoting around the r_opand.

	FR_TOKEN op;						//!< Operator.
} VALUE_PAIR_RAW;

#define vp_strvalue   data.strvalue
#define vp_octets     data.octets
#define vp_ipv6addr   data.ipv6addr
#define vp_ifid       data.ifid
#define vp_ipv6prefix data.ipv6prefix
#define vp_ipv4prefix data.ipv4prefix
#define vp_filter     data.filter
#define vp_ether      data.ether
#define vp_signed     data.sinteger
#define vp_tlv	      data.tlv
#define vp_integer64  data.integer64
#define vp_ipaddr     data.ipaddr.s_addr
#define vp_date       data.date
#define vp_integer    data.integer

typedef struct fr_ipaddr_t {
	int		af;	/* address family */
	union {
		struct in_addr	ip4addr;
		struct in6_addr ip6addr; /* maybe defined in missing.h */
	} ipaddr;
	uint32_t	scope;	/* for IPv6 */
} fr_ipaddr_t;

/*
 *	vector:		Request authenticator from access-request packet
 *			Put in there by rad_decode, and must be put in the
 *			response RADIUS_PACKET as well before calling rad_send
 *
 *	verified:	Filled in by rad_decode for accounting-request packets
 *
 *	data,data_len:	Used between rad_recv and rad_decode.
 */
typedef struct radius_packet {
	int			sockfd;
	fr_ipaddr_t		src_ipaddr;
	fr_ipaddr_t		dst_ipaddr;
	uint16_t		src_port;
	uint16_t		dst_port;
	int			id;
	unsigned int		code;
	uint8_t			vector[AUTH_VECTOR_LEN];
	struct timeval		timestamp;
	uint8_t			*data;
	size_t			data_len;
	VALUE_PAIR		*vps;
	ssize_t			offset;
#ifdef WITH_TCP
	size_t			partial;
#endif
} RADIUS_PACKET;

/*
 *	Printing functions.
 */
int		fr_utf8_char(uint8_t const *str);
size_t		fr_print_string(char const *in, size_t inlen,
				 char *out, size_t outlen);
size_t   	vp_prints_value(char *out, size_t outlen, VALUE_PAIR const *vp, int8_t quote);
char		*vp_aprinttype(TALLOC_CTX *ctx, PW_TYPE type);
char     	*vp_aprint(TALLOC_CTX *ctx, VALUE_PAIR const *vp);
size_t    	vp_prints_value_json(char *out, size_t outlen, VALUE_PAIR const *vp);
size_t		vp_print_name(char *out, size_t outlen, unsigned int attr, unsigned int vendor);
size_t		vp_prints(char *out, size_t outlen, VALUE_PAIR const *vp);
void		vp_print(FILE *, VALUE_PAIR const *);
void		vp_printlist(FILE *, VALUE_PAIR const *);
#define		fprint_attr_val vp_print

/*
 *	Dictionary functions.
 */
extern const int dict_attr_allowed_chars[256];
int		str2argv(char *str, char **argv, int max_argc);
int		dict_str2oid(char const *ptr, unsigned int *pattr,
			     unsigned int *pvendor, int tlv_depth);
int		dict_addvendor(char const *name, unsigned int value);
int		dict_addattr(char const *name, int attr, unsigned int vendor, int type, ATTR_FLAGS flags);
int		dict_addvalue(char const *namestr, char const *attrstr, int value);
int		dict_init(char const *dir, char const *fn);
void		dict_free(void);
int		dict_read(char const *dir, char const *filename);
void 		dict_attr_free(DICT_ATTR const **da);
DICT_ATTR const	*dict_attr_copy(DICT_ATTR const *da, int vp_free);
DICT_ATTR const	*dict_attrunknown(unsigned int attr, unsigned int vendor, int vp_free);
DICT_ATTR const	*dict_attrunknownbyname(char const *attribute, int vp_free);
DICT_ATTR const	*dict_attrbyvalue(unsigned int attr, unsigned int vendor);
DICT_ATTR const	*dict_attrbyname(char const *attr);
DICT_ATTR const	*dict_attrbytype(unsigned int attr, unsigned int vendor,
				 PW_TYPE type);
DICT_ATTR const	*dict_attrbyparent(DICT_ATTR const *parent, unsigned int attr,
					   unsigned int vendor);
int		dict_attr_child(DICT_ATTR const *parent,
				unsigned int *pattr, unsigned int *pvendor);
DICT_VALUE	*dict_valbyattr(unsigned int attr, unsigned int vendor, int val);
DICT_VALUE	*dict_valbyname(unsigned int attr, unsigned int vendor, char const *val);
char const	*dict_valnamebyattr(unsigned int attr, unsigned int vendor, int value);
int		dict_vendorbyname(char const *name);
DICT_VENDOR	*dict_vendorbyvalue(int vendor);

#if 1 /* FIXME: compat */
#define dict_attrget	dict_attrbyvalue
#define dict_attrfind	dict_attrbyname
#define dict_valfind	dict_valbyname
/*#define dict_valget	dict_valbyattr almost but not quite*/
#endif

/* md5.c */

void		fr_md5_calc(uint8_t *, uint8_t const *, unsigned int);

/* hmac.c */

void fr_hmac_md5(uint8_t const *text, size_t text_len, uint8_t const *key, size_t key_len, unsigned char *digest);

/* hmacsha1.c */

void fr_hmac_sha1(uint8_t const *text, size_t text_len, uint8_t const *key, size_t key_len, uint8_t *digest);

/* radius.c */
int		rad_send(RADIUS_PACKET *, RADIUS_PACKET const *, char const *secret);
int		rad_packet_ok(RADIUS_PACKET *packet, int flags);
RADIUS_PACKET	*rad_recv(int fd, int flags);
ssize_t rad_recv_header(int sockfd, fr_ipaddr_t *src_ipaddr, int *src_port,
			int *code);
void		rad_recv_discard(int sockfd);
int		rad_verify(RADIUS_PACKET *packet, RADIUS_PACKET *original,
			   char const *secret);
int		rad_decode(RADIUS_PACKET *packet, RADIUS_PACKET *original, char const *secret);
int		rad_encode(RADIUS_PACKET *packet, RADIUS_PACKET const *original,
			   char const *secret);
int		rad_sign(RADIUS_PACKET *packet, RADIUS_PACKET const *original,
			 char const *secret);

int rad_digest_cmp(uint8_t const *a, uint8_t const *b, size_t length);
RADIUS_PACKET	*rad_alloc(TALLOC_CTX *ctx, int newvector);
RADIUS_PACKET	*rad_alloc_reply(TALLOC_CTX *ctx, RADIUS_PACKET *);
void		rad_free(RADIUS_PACKET **);
int		rad_pwencode(char *encpw, size_t *len, char const *secret,
			     uint8_t const *vector);
int		rad_pwdecode(char *encpw, size_t len, char const *secret,
			     uint8_t const *vector);
int		rad_tunnel_pwencode(char *encpw, size_t *len, char const *secret,
				    uint8_t const *vector);
int		rad_tunnel_pwdecode(uint8_t *encpw, size_t *len,
				    char const *secret, uint8_t const *vector);
int		rad_chap_encode(RADIUS_PACKET *packet, uint8_t *output,
				int id, VALUE_PAIR *password);

int rad_attr_ok(RADIUS_PACKET const *packet, RADIUS_PACKET const *original,
		DICT_ATTR *da,
		uint8_t const *data, size_t length);
int rad_tlv_ok(uint8_t const *data, size_t length,
	       size_t dv_type, size_t dv_length);

ssize_t	rad_attr2vp(RADIUS_PACKET *packet, RADIUS_PACKET const *original,
		    char const *secret,
		    uint8_t const *data, size_t length,
		    VALUE_PAIR **pvp);

ssize_t  rad_data2vp(unsigned int attribute, unsigned int vendor,
		     uint8_t const *data, size_t length,
		     VALUE_PAIR **pvp);

ssize_t rad_vp2data(VALUE_PAIR const *vp, uint8_t *out, size_t outlen);

int rad_vp2extended(RADIUS_PACKET const *packet,
		    RADIUS_PACKET const *original,
		    char const *secret, VALUE_PAIR const **pvp,
		    uint8_t *ptr, size_t room);
int rad_vp2wimax(RADIUS_PACKET const *packet,
		 RADIUS_PACKET const *original,
		 char const *secret, VALUE_PAIR const **pvp,
		 uint8_t *ptr, size_t room);
int rad_vp2vsa(RADIUS_PACKET const *packet, RADIUS_PACKET const *original,
	       char const *secret, VALUE_PAIR const **pvp, uint8_t *start,
	       size_t room);
int rad_vp2rfc(RADIUS_PACKET const *packet,
	       RADIUS_PACKET const *original,
	       char const *secret, VALUE_PAIR const **pvp,
	       uint8_t *ptr, size_t room);

int rad_vp2attr(RADIUS_PACKET const *packet,
		RADIUS_PACKET const *original, char const *secret,
		VALUE_PAIR const **pvp, uint8_t *ptr, size_t room);

/* valuepair.c */
VALUE_PAIR	*pairalloc(TALLOC_CTX *ctx, DICT_ATTR const *da);
VALUE_PAIR	*paircreate(TALLOC_CTX *ctx, unsigned int attr, unsigned int vendor);
int		pair2unknown(VALUE_PAIR *vp);
void		pairfree(VALUE_PAIR **);
VALUE_PAIR	*pairfind(VALUE_PAIR *, unsigned int attr, unsigned int vendor, int8_t tag);

#define		paircursor(_x, _y)	paircursorc(_x,(VALUE_PAIR const * const *) _y)
VALUE_PAIR	*paircursorc(vp_cursor_t *cursor, VALUE_PAIR const * const *node);
VALUE_PAIR	*pairfirst(vp_cursor_t *cursor);
VALUE_PAIR	*pairfindnext(vp_cursor_t *cursor, unsigned int attr, unsigned int vendor, int8_t tag);
VALUE_PAIR	*pairnext(vp_cursor_t *cursor);
VALUE_PAIR	*pairlast(vp_cursor_t *cursor);
VALUE_PAIR	*paircurrent(vp_cursor_t *cursor);
void		pairinsert(vp_cursor_t *cursor, VALUE_PAIR *vp);
VALUE_PAIR	*pairremove(vp_cursor_t *cursor);
void		pairdelete(VALUE_PAIR **, unsigned int attr, unsigned int vendor, int8_t tag);
void		pairadd(VALUE_PAIR **, VALUE_PAIR *);
void		pairreplace(VALUE_PAIR **first, VALUE_PAIR *add);
int		paircmp(VALUE_PAIR *check, VALUE_PAIR *data);
int		paircmp_op(VALUE_PAIR const *one, FR_TOKEN op, VALUE_PAIR const *two);
void		pairsort(VALUE_PAIR **vps, bool with_tag);
bool		pairvalidate(VALUE_PAIR *filter, VALUE_PAIR *list);
VALUE_PAIR	*paircopyvp(TALLOC_CTX *ctx, VALUE_PAIR const *vp);
VALUE_PAIR	*paircopyvpdata(TALLOC_CTX *ctx, DICT_ATTR const *da, VALUE_PAIR const *vp);
VALUE_PAIR	*paircopy(TALLOC_CTX *ctx, VALUE_PAIR *from);
VALUE_PAIR	*paircopy2(TALLOC_CTX *ctx, VALUE_PAIR *from, unsigned int attr, unsigned int vendor, int8_t tag);
void		pairmemcpy(VALUE_PAIR *vp, uint8_t const * src, size_t len);
void		pairmemsteal(VALUE_PAIR *vp, uint8_t *src);
void		pairstrsteal(VALUE_PAIR *vp, char *src);
void		pairstrcpy(VALUE_PAIR *vp, char const * src);
void		pairsprintf(VALUE_PAIR *vp, char const * fmt, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 2, 3)))
#endif
;
void		pairmove(TALLOC_CTX *ctx, VALUE_PAIR **to, VALUE_PAIR **from);
void		pairfilter(TALLOC_CTX *ctx, VALUE_PAIR **to, VALUE_PAIR **from,
					   unsigned int attr, unsigned int vendor, int8_t tag);
bool		pairparsevalue(VALUE_PAIR *vp, char const *value);
VALUE_PAIR	*pairmake(TALLOC_CTX *ctx, VALUE_PAIR **vps, char const *attribute, char const *value, FR_TOKEN op);
int 		pairmark_xlat(VALUE_PAIR *vp, char const *value);
FR_TOKEN 	pairread(char const **ptr, VALUE_PAIR_RAW *raw);
FR_TOKEN	userparse(TALLOC_CTX *ctx, char const *buffer, VALUE_PAIR **head);
VALUE_PAIR	*readvp2(TALLOC_CTX *ctx, FILE *fp, int *pfiledone, char const *errprefix);

/*
 *	Error functions.
 */
#ifdef _LIBRADIUS
void		fr_strerror_printf(char const *, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 1, 2)))
#endif
;
#endif
void		fr_perror(char const *, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 1, 2)))
#endif
;
extern bool fr_assert_cond(char const *file, int line, char const *expr, bool cond);
#define fr_assert(_x) fr_assert_cond(__FILE__,  __LINE__, #_x, (_x))

extern void _fr_exit(char const *file, int line, int status);
#define fr_exit(_x) _fr_exit(__FILE__,  __LINE__, (_x))

extern void _fr_exit_now(char const *file, int line, int status);
#define fr_exit_now(_x) _fr_exit_now(__FILE__,  __LINE__, (_x))

extern char const *fr_strerror(void);
extern int	fr_dns_lookups;	/* 0 = no dns lookups */
extern int	fr_debug_flag;	/* 0 = no debugging information */
extern int	fr_max_attributes; /* per incoming packet */
#define	FR_MAX_PACKET_CODE (52)
extern char const *fr_packet_codes[FR_MAX_PACKET_CODE];
extern FILE	*fr_log_fp;
extern void rad_print_hex(RADIUS_PACKET *packet);
void		fr_printf_log(char const *, ...)
#ifdef __GNUC__
		__attribute__ ((format (printf, 1, 2)))
#endif
;

/*
 *	Several handy miscellaneous functions.
 */
void		fr_debug_break(void);
char const 	*ip_ntoa(char *, uint32_t);
char		*ifid_ntoa(char *buffer, size_t size, uint8_t const *ifid);
uint8_t		*ifid_aton(char const *ifid_str, uint8_t *ifid);
int		rad_lockfd(int fd, int lock_len);
int		rad_lockfd_nonblock(int fd, int lock_len);
int		rad_unlockfd(int fd, int lock_len);
size_t		fr_bin2hex(char *hex, uint8_t const *bin, size_t inlen);
size_t		fr_hex2bin(uint8_t *bin, char const *hex, size_t outlen);
int fr_ipaddr_cmp(fr_ipaddr_t const *a, fr_ipaddr_t const *b);

int		ip_hton(char const *src, int af, fr_ipaddr_t *dst);
char const	*ip_ntoh(fr_ipaddr_t const *src, char *dst, size_t cnt);
int fr_ipaddr2sockaddr(fr_ipaddr_t const *ipaddr, int port,
		       struct sockaddr_storage *sa, socklen_t *salen);
int fr_sockaddr2ipaddr(struct sockaddr_storage const *sa, socklen_t salen,
		       fr_ipaddr_t *ipaddr, int * port);
ssize_t fr_utf8_to_ucs2(uint8_t *out, size_t outlen, char const *in, size_t inlen);


#ifdef WITH_ASCEND_BINARY
/* filters.c */
int		ascend_parse_filter(VALUE_PAIR *pair);
void		print_abinary(VALUE_PAIR const *vp, char *buffer, size_t len, int8_t quote);
#endif /*WITH_ASCEND_BINARY*/

/* random numbers in isaac.c */
/* context of random number generator */
typedef struct fr_randctx {
	uint32_t randcnt;
	uint32_t randrsl[256];
  	uint32_t randmem[256];
	uint32_t randa;
	uint32_t randb;
	uint32_t randc;
} fr_randctx;

void fr_isaac(fr_randctx *ctx);
void fr_randinit(fr_randctx *ctx, int flag);
uint32_t fr_rand(void);	/* like rand(), but better. */
void fr_rand_seed(void const *, size_t ); /* seed the random pool */


/* crypt wrapper from crypt.c */
int fr_crypt_check(char const *key, char const *salt);

/* rbtree.c */
typedef struct rbtree_t rbtree_t;
typedef struct rbnode_t rbnode_t;

#define RBTREE_FLAG_NONE    (0)
#define RBTREE_FLAG_REPLACE (1 << 0)
#define RBTREE_FLAG_LOCK    (1 << 1)
rbtree_t       *rbtree_create(int (*Compare)(void const *, void const *),
			      void (*freeNode)(void *),
			      int flags);
void		rbtree_free(rbtree_t *tree);
bool		rbtree_insert(rbtree_t *tree, void *Data);
rbnode_t	*rbtree_insertnode(rbtree_t *tree, void *Data);
void		rbtree_delete(rbtree_t *tree, rbnode_t *Z);
bool		rbtree_deletebydata(rbtree_t *tree, void const *data);
rbnode_t       *rbtree_find(rbtree_t *tree, void const *Data);
void	       *rbtree_finddata(rbtree_t *tree, void const *Data);
int		rbtree_num_elements(rbtree_t *tree);
void	       *rbtree_min(rbtree_t *tree);
void	       *rbtree_node2data(rbtree_t *tree, rbnode_t *node);

/* callback order for walking  */
typedef enum { PreOrder, InOrder, PostOrder, DeleteOrder } RBTREE_ORDER;

/*
 *	The callback should be declared as:
 *	int callback(void *context, void *data)
 *
 *	The "context" is some user-defined context.
 *	The "data" is the pointer to the user data in the node,
 *	  NOT the node itself.
 *
 *	It should return 0 if all is OK, and !0 for any error.
 *	The walking will stop on any error.
 *
 *	Except with DeleteOrder, where the callback should return <0 for
 *	errors, and may return 1 to delete the current node and halt,
 *	or 2 to delete the current node and continue.  This may be
 *	used to batch-delete select nodes from a locked rbtree.
 */
int rbtree_walk(rbtree_t *tree, RBTREE_ORDER order, int (*callback)(void *, void *), void *context);

/*
 *	Find a matching data item in an rbtree and, if one is found,
 *	perform a callback on it.
 *
 *	The callback is similar to rbtree_walk above, except that a
 *	positive return code from the callback will cause the found node
 *	to be deleted from the tree.  If the tree was created with
 *	RBTREE_FLAG_LOCK, then the entire find/callback/delete/rebalance
 *	sequence happens while the lock is held.
 *
 *	Note that the callback MUST NOT alter any of the data which
 *	is used as the rbtree key, nor attempt to alter the rest of
 *	the rbtree in any way.
 *
 *	Returns a pointer to the user data in the found node, or NULL if the
 *	item was not found, or NULL if the item was deleted and the tree was
 *	created with a freeNode garbage collection routine.
 */
void *rbtree_callbydata(rbtree_t *tree, void const *Data, int (*callback)(void *, void *), void *context);

/*
 *	FIFOs
 */
typedef struct fr_fifo_t fr_fifo_t;
typedef void (*fr_fifo_free_t)(void *);
fr_fifo_t *fr_fifo_create(int max_entries, fr_fifo_free_t freeNode);
void fr_fifo_free(fr_fifo_t *fi);
int fr_fifo_push(fr_fifo_t *fi, void *data);
void *fr_fifo_pop(fr_fifo_t *fi);
void *fr_fifo_peek(fr_fifo_t *fi);
int fr_fifo_num_elements(fr_fifo_t *fi);

#ifdef __cplusplus
}
#endif

#include <freeradius-devel/packet.h>

#ifdef WITH_TCP
#include <freeradius-devel/tcp.h>
#endif

#endif /*LIBRADIUS_H*/
