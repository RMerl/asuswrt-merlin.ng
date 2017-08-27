%module capi
%{
#include <netlink/netlink.h>
#include <netlink/types.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/object.h>
#include <netlink/cache.h>
%}

%include <stdint.i>
%include <cstring.i>

%inline %{
        struct nl_dump_params *alloc_dump_params(void)
        {
                struct nl_dump_params *dp;
                if (!(dp = calloc(1, sizeof(*dp))))
                        return NULL;
                dp->dp_fd = stdout;
                return dp;
        }

        void free_dump_params(struct nl_dump_params *dp)
        {
                free(dp);
        }
%};

/* <netlink/types.h> */

enum nl_dump_type {
	NL_DUMP_LINE,		/**< Dump object briefly on one line */
	NL_DUMP_DETAILS,	/**< Dump all attributes but no statistics */
	NL_DUMP_STATS,		/**< Dump all attributes including statistics */
	__NL_DUMP_MAX,
};

struct nl_dump_params
{
	/**
	 * Specifies the type of dump that is requested.
	 */
	enum nl_dump_type	dp_type;

	/**
	 * Specifies the number of whitespaces to be put in front
	 * of every new line (indentation).
	 */
	int			dp_prefix;

	/**
	 * Causes the cache index to be printed for each element.
	 */
	int			dp_print_index;

	/**
	 * Causes each element to be prefixed with the message type.
	 */
	int			dp_dump_msgtype;

	/**
	 * A callback invoked for output
	 *
	 * Passed arguments are:
	 *  - dumping parameters
	 *  - string to append to the output
	 */
	void			(*dp_cb)(struct nl_dump_params *, char *);

	/**
	 * A callback invoked for every new line, can be used to
	 * customize the indentation.
	 *
	 * Passed arguments are:
	 *  - dumping parameters
	 *  - line number starting from 0
	 */
	void			(*dp_nl_cb)(struct nl_dump_params *, int);

	/**
	 * User data pointer, can be used to pass data to callbacks.
	 */
	void			*dp_data;

	/**
	 * File descriptor the dumping output should go to
	 */
	FILE *			dp_fd;

	/**
	 * Alternatively the output may be redirected into a buffer
	 */
	char *			dp_buf;

	/**
	 * Length of the buffer dp_buf
	 */
	size_t			dp_buflen;

	/**
	 * PRIVATE
	 * Set if a dump was performed prior to the actual dump handler.
	 */
	int			dp_pre_dump;

	/**
	 * PRIVATE
	 * Owned by the current caller
	 */
	int			dp_ivar;

	unsigned int		dp_line;
};

/* <netlink/errno.h> */
extern const char *nl_geterror(int);

/* <netlink/utils.h> */

extern double nl_cancel_down_bytes(unsigned long long, char **);
extern double nl_cancel_down_bits(unsigned long long, char **);
%cstring_output_maxsize(char *buf, size_t len)
extern int nl_rate2str(unsigned long long rate, int type, char *buf, size_t len);
extern double nl_cancel_down_us(uint32_t, char **);

extern long nl_size2int(const char *);
%cstring_output_maxsize(char *buf, const size_t len)
extern char *nl_size2str(const size_t, char *buf, const size_t len);
extern long nl_prob2int(const char *);

extern int nl_get_user_hz(void);
extern uint32_t nl_us2ticks(uint32_t);
extern uint32_t nl_ticks2us(uint32_t);
extern int nl_str2msec(const char *, uint64_t *);

%cstring_output_maxsize(char *buf, size_t len)
extern char *nl_msec2str(uint64_t, char *buf, size_t len);

%cstring_output_maxsize(char *buf, size_t len)
extern char *nl_llproto2str(int, char *buf, size_t len);
extern int nl_str2llproto(const char *);

%cstring_output_maxsize(char *buf, size_t len)
extern char *nl_ether_proto2str(int, char *buf, size_t len);
extern int nl_str2ether_proto(const char *);

%cstring_output_maxsize(char *buf, size_t len)
extern char *nl_ip_proto2str(int, char *buf, size_t len);
extern int nl_str2ip_proto(const char *);

extern void nl_new_line(struct nl_dump_params *);
extern void nl_dump(struct nl_dump_params *, const char *, ...);
extern void nl_dump_line(struct nl_dump_params *, const char *, ...);

/* <netlink/netlink.h> */
extern struct nl_dump_params *alloc_dump_params(void);
extern void free_dump_params(struct nl_dump_params *);

extern int nl_connect(struct nl_sock *, int);
extern void nl_close(struct nl_sock *);
extern int nl_pickup(struct nl_sock *, int (*parser)(struct nl_cache_ops *,
                                                struct sockaddr_nl *,
                                                struct nlmsghdr *,
                                                struct nl_parser_param *),
                                          struct nl_object **);

/* <netlink/socket.h> */
extern struct nl_sock *nl_socket_alloc(void);
extern struct nl_sock *nl_socket_alloc_cb(struct nl_cb *);
extern void nl_socket_free(struct nl_sock *);

extern uint32_t nl_socket_get_local_port(const struct nl_sock *);
extern void nl_socket_set_local_port(struct nl_sock *, uint32_t);

extern uint32_t nl_socket_get_peer_port(const struct nl_sock *);
extern void nl_socket_set_peer_port(struct nl_sock *, uint32_t);

extern uint32_t nl_socket_get_peer_groups(const struct nl_sock *sk);
extern void  nl_socket_set_peer_groups(struct nl_sock *sk, uint32_t groups);

extern int nl_socket_set_buffer_size(struct nl_sock *, int, int);

/* <netlink/msg.h> */
extern int			nlmsg_size(int);
extern int			nlmsg_total_size(int);
extern int			nlmsg_padlen(int);

extern void *			nlmsg_data(const struct nlmsghdr *);
extern int			nlmsg_datalen(const struct nlmsghdr *);
extern void *			nlmsg_tail(const struct nlmsghdr *);

/* attribute access */
extern struct nlattr *	  nlmsg_attrdata(const struct nlmsghdr *, int);
extern int		  nlmsg_attrlen(const struct nlmsghdr *, int);

/* message parsing */
extern int		  nlmsg_valid_hdr(const struct nlmsghdr *, int);
extern int		  nlmsg_ok(const struct nlmsghdr *, int);
extern struct nlmsghdr *  nlmsg_next(struct nlmsghdr *, int *);
extern int		  nlmsg_parse(struct nlmsghdr *, int, struct nlattr **,
				      int, struct nla_policy *);
extern struct nlattr *	  nlmsg_find_attr(struct nlmsghdr *, int, int);
extern int		  nlmsg_validate(struct nlmsghdr *, int, int,
					 struct nla_policy *);

extern struct nl_msg *	  nlmsg_alloc(void);
extern struct nl_msg *	  nlmsg_alloc_size(size_t);
extern struct nl_msg *	  nlmsg_alloc_simple(int, int);
extern void		  nlmsg_set_default_size(size_t);
extern struct nl_msg *	  nlmsg_inherit(struct nlmsghdr *);
extern struct nl_msg *	  nlmsg_convert(struct nlmsghdr *);
extern void *		  nlmsg_reserve(struct nl_msg *, size_t, int);
extern int		  nlmsg_append(struct nl_msg *, void *, size_t, int);
extern int		  nlmsg_expand(struct nl_msg *, size_t);

extern struct nlmsghdr *  nlmsg_put(struct nl_msg *, uint32_t, uint32_t,
				    int, int, int);
extern struct nlmsghdr *  nlmsg_hdr(struct nl_msg *);
extern void		  nlmsg_get(struct nl_msg *);
extern void		  nlmsg_free(struct nl_msg *);

/* attribute modification */
extern void		  nlmsg_set_proto(struct nl_msg *, int);
extern int		  nlmsg_get_proto(struct nl_msg *);
extern size_t		  nlmsg_get_max_size(struct nl_msg *);
extern void		  nlmsg_set_src(struct nl_msg *, struct sockaddr_nl *);
extern struct sockaddr_nl *nlmsg_get_src(struct nl_msg *);
extern void		  nlmsg_set_dst(struct nl_msg *, struct sockaddr_nl *);
extern struct sockaddr_nl *nlmsg_get_dst(struct nl_msg *);
extern void		  nlmsg_set_creds(struct nl_msg *, struct ucred *);
extern struct ucred *	  nlmsg_get_creds(struct nl_msg *);

extern char *		  nl_nlmsgtype2str(int, char *, size_t);
extern int		  nl_str2nlmsgtype(const char *);

extern char *		  nl_nlmsg_flags2str(int, char *, size_t);

extern int		  nl_msg_parse(struct nl_msg *,
				       void (*cb)(struct nl_object *, void *),
				       void *);

extern void		nl_msg_dump(struct nl_msg *, FILE *);

%inline %{
	struct nl_object *cast_obj(void *obj)
        {
                return (struct nl_object *) obj;
        }

        struct nl_object *object_alloc_name(const char *name)
        {
                struct nl_object *obj;

                if (nl_object_alloc_name(name, &obj) < 0)
                        return NULL;

                return obj;
        }
%};

extern struct nl_object *nl_object_alloc(struct nl_object_ops *);
extern void nl_object_free(struct nl_object *);
extern struct nl_object *nl_object_clone(struct nl_object *);
extern void nl_object_get(struct nl_object *);
extern void nl_object_put(struct nl_object *);
extern int nl_object_shared(struct nl_object *);

%cstring_output_maxsize(char *buf, size_t len)
extern void nl_object_dump_buf(struct nl_object *, char *buf, size_t len);

extern void nl_object_dump(struct nl_object *, struct nl_dump_params *);

extern int nl_object_identical(struct nl_object *, struct nl_object *);
extern uint32_t nl_object_diff(struct nl_object *, struct nl_object *);
extern int nl_object_match_filter(struct nl_object *, struct nl_object *);

%cstring_output_maxsize(char *buf, size_t len)
extern char *nl_object_attrs2str(struct nl_object *, uint32_t, char *buf, size_t len);

%cstring_output_maxsize(char *buf, size_t len)
extern char *nl_object_attr_list(struct nl_object *, char *buf, size_t len);

extern void nl_object_mark(struct nl_object *);
extern void nl_object_unmark(struct nl_object *);
extern int nl_object_is_marked(struct nl_object *);

extern int nl_object_get_refcnt(struct nl_object *);

/* <netlink/cache.h> */

typedef void (*change_func_t)(struct nl_cache *, struct nl_object *, int, void *);

%inline %{
        struct nl_cache *alloc_cache_name(const char *name)
        {
                struct nl_cache *c;
                if (nl_cache_alloc_name(name, &c) < 0)
                        return NULL;
                return c;
        }

        struct nl_cache_mngr *alloc_cache_mngr(struct nl_sock *sock,
                                               int protocol, int flags)
        {
                struct nl_cache_mngr *mngr;

                if (nl_cache_mngr_alloc(sock, protocol, flags, &mngr) < 0)
                        return NULL;

                return mngr;
        }

        struct nl_cache *cache_mngr_add(struct nl_cache_mngr *mngr,
                                        const char *name, change_func_t func,
                                        void *arg)
        {
                struct nl_cache *cache;

                if (nl_cache_mngr_add(mngr, name, func, arg, &cache) < 0)
                        return NULL;

                return cache;
        }
%}

/* Access Functions */
extern int			nl_cache_nitems(struct nl_cache *);
extern int			nl_cache_nitems_filter(struct nl_cache *,
						       struct nl_object *);
extern struct nl_cache_ops *	nl_cache_get_ops(struct nl_cache *);
extern struct nl_object *	nl_cache_get_first(struct nl_cache *);
extern struct nl_object *	nl_cache_get_last(struct nl_cache *);
extern struct nl_object *	nl_cache_get_next(struct nl_object *);
extern struct nl_object *	nl_cache_get_prev(struct nl_object *);

extern struct nl_cache *	nl_cache_alloc(struct nl_cache_ops *);
extern struct nl_cache *	nl_cache_subset(struct nl_cache *,
						struct nl_object *);
extern void			nl_cache_clear(struct nl_cache *);
extern void			nl_cache_free(struct nl_cache *);

/* Cache modification */
extern int			nl_cache_add(struct nl_cache *,
					     struct nl_object *);
extern int			nl_cache_parse_and_add(struct nl_cache *,
						       struct nl_msg *);
extern void			nl_cache_remove(struct nl_object *);
extern int			nl_cache_refill(struct nl_sock *,
						struct nl_cache *);
extern int			nl_cache_pickup(struct nl_sock *,
						struct nl_cache *);
extern int			nl_cache_resync(struct nl_sock *,
						struct nl_cache *,
						change_func_t,
						void *);
extern int			nl_cache_include(struct nl_cache *,
						 struct nl_object *,
						 change_func_t,
						 void *);
extern void			nl_cache_set_arg1(struct nl_cache *, int);
extern void			nl_cache_set_arg2(struct nl_cache *, int);

/* General */
extern int			nl_cache_is_empty(struct nl_cache *);
extern struct nl_object *	nl_cache_search(struct nl_cache *,
						struct nl_object *);
extern void			nl_cache_mark_all(struct nl_cache *);

/* Dumping */
extern void			nl_cache_dump(struct nl_cache *,
					      struct nl_dump_params *);
extern void			nl_cache_dump_filter(struct nl_cache *,
						     struct nl_dump_params *,
						     struct nl_object *);

/* Iterators */
extern void			nl_cache_foreach(struct nl_cache *,
						 void (*cb)(struct nl_object *,
							    void *),
						 void *arg);
extern void			nl_cache_foreach_filter(struct nl_cache *,
							struct nl_object *,
							void (*cb)(struct
								   nl_object *,
								   void *),
							void *arg);

/* --- cache management --- */

/* Cache type management */
extern struct nl_cache_ops *	nl_cache_ops_lookup(const char *);
extern struct nl_cache_ops *	nl_cache_ops_associate(int, int);
extern struct nl_msgtype *	nl_msgtype_lookup(struct nl_cache_ops *, int);
extern void			nl_cache_ops_foreach(void (*cb)(struct nl_cache_ops *, void *), void *);
extern int			nl_cache_mngt_register(struct nl_cache_ops *);
extern int			nl_cache_mngt_unregister(struct nl_cache_ops *);

/* Global cache provisioning/requiring */
extern void			nl_cache_mngt_provide(struct nl_cache *);
extern void			nl_cache_mngt_unprovide(struct nl_cache *);
extern struct nl_cache *	nl_cache_mngt_require(const char *);

struct nl_cache_mngr;

#define NL_AUTO_PROVIDE		1

extern int			nl_cache_mngr_get_fd(struct nl_cache_mngr *);
extern int			nl_cache_mngr_poll(struct nl_cache_mngr *,
						   int);
extern int			nl_cache_mngr_data_ready(struct nl_cache_mngr *);
extern void			nl_cache_mngr_free(struct nl_cache_mngr *);

/* <netlink/addr.h> */
%inline %{
        struct nl_addr *addr_parse(const char *addr, int guess)
        {
                struct nl_addr *result;

                if (nl_addr_parse(addr, guess, &result) < 0)
                        return NULL;

                return result;
        }
%};

extern struct nl_addr *nl_addr_alloc(size_t);
extern struct nl_addr *nl_addr_alloc_attr(struct nlattr *, int);
extern struct nl_addr *nl_addr_build(int, void *, size_t);
extern struct nl_addr *nl_addr_clone(struct nl_addr *);

extern struct nl_addr *nl_addr_get(struct nl_addr *);
extern void nl_addr_put(struct nl_addr *);
extern int nl_addr_shared(struct nl_addr *);

extern int nl_addr_cmp(struct nl_addr *, struct nl_addr *);
extern int nl_addr_cmp_prefix(struct nl_addr *, struct nl_addr *);
extern int nl_addr_iszero(struct nl_addr *);
extern int nl_addr_valid(char *, int);
extern int nl_addr_guess_family(struct nl_addr *);
extern int nl_addr_fill_sockaddr(struct nl_addr *, struct sockaddr *, socklen_t *);
extern int nl_addr_info(struct nl_addr *, struct addrinfo **);
extern int nl_addr_resolve(struct nl_addr *addr, char *host, size_t hostlen);

extern void nl_addr_set_family(struct nl_addr *, int);
extern int nl_addr_get_family(struct nl_addr *);
extern int nl_addr_set_binary_addr(struct nl_addr *, void *, size_t);

extern void *nl_addr_get_binary_addr(struct nl_addr *);
extern unsigned int nl_addr_get_len(struct nl_addr *);
extern void nl_addr_set_prefixlen(struct nl_addr *, int);
extern unsigned int nl_addr_get_prefixlen(struct nl_addr *);

%cstring_output_maxsize(char *buf, size_t len)
extern char *nl_af2str(int, char *buf, size_t len);
extern int nl_str2af(const char *);

%cstring_output_maxsize(char *buf, size_t len)
extern char *nl_addr2str(struct nl_addr *, char *buf, size_t len);
