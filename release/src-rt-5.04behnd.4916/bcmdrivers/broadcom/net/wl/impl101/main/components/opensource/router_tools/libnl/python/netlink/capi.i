%module capi
%{
#include <netlink/netlink.h>
#include <netlink/types.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/object.h>
#include <netlink/cache.h>
#include <netlink/attr.h>
#include <net/if.h>

#define DEBUG
#include "utils.h"
%}

%include <stdint.i>
%include <cstring.i>
%include <cpointer.i>

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

/* <net/if.h> */
extern unsigned int if_nametoindex(const char *ifname);

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
extern void nl_socket_set_cb(struct nl_sock *, struct nl_cb *);

extern int nl_send_auto_complete(struct nl_sock *, struct nl_msg *);
extern int nl_recvmsgs(struct nl_sock *, struct nl_cb *);

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

/* Message Handlers <netlink/handlers.h> */
/**
 * Callback actions
 * @ingroup cb
 */
enum nl_cb_action {
	/** Proceed with wathever would come next */
	NL_OK,
	/** Skip this message */
	NL_SKIP,
	/** Stop parsing altogether and discard remaining messages */
	NL_STOP,
};

/**
 * Callback kinds
 * @ingroup cb
 */
enum nl_cb_kind {
	/** Default handlers (quiet) */
	NL_CB_DEFAULT,
	/** Verbose default handlers (error messages printed) */
	NL_CB_VERBOSE,
	/** Debug handlers for debugging */
	NL_CB_DEBUG,
	/** Customized handler specified by the user */
	NL_CB_CUSTOM,
	__NL_CB_KIND_MAX,
};

#define NL_CB_KIND_MAX (__NL_CB_KIND_MAX - 1)

/**
 * Callback types
 * @ingroup cb
 */
enum nl_cb_type {
	/** Message is valid */
	NL_CB_VALID,
	/** Last message in a series of multi part messages received */
	NL_CB_FINISH,
	/** Report received that data was lost */
	NL_CB_OVERRUN,
	/** Message wants to be skipped */
	NL_CB_SKIPPED,
	/** Message is an acknowledge */
	NL_CB_ACK,
	/** Called for every message received */
	NL_CB_MSG_IN,
	/** Called for every message sent out except for nl_sendto() */
	NL_CB_MSG_OUT,
	/** Message is malformed and invalid */
	NL_CB_INVALID,
	/** Called instead of internal sequence number checking */
	NL_CB_SEQ_CHECK,
	/** Sending of an acknowledge message has been requested */
	NL_CB_SEND_ACK,
	/** Flag NLM_F_DUMP_INTR is set in message */
	NL_CB_DUMP_INTR,
	__NL_CB_TYPE_MAX,
};

#define NL_CB_TYPE_MAX (__NL_CB_TYPE_MAX - 1)

extern struct nl_cb *nl_cb_alloc(enum nl_cb_kind);
extern struct nl_cb *nl_cb_clone(struct nl_cb *);

struct nlmsgerr {
	int error;
};

%{

struct pynl_callback {
	PyObject *cbf;
	PyObject *cba;
};

struct pynl_cbinfo {
	struct nl_cb *cb;
	struct pynl_callback cbtype[NL_CB_TYPE_MAX+1];
	struct pynl_callback cberr;
	struct list_head list;
};

LIST_HEAD(callback_list);

static struct pynl_cbinfo *pynl_find_cbinfo(struct nl_cb *cb, int unlink)
{
	struct list_head *pos, *prev;
	struct pynl_cbinfo *info;

	list_for_each_safe(pos, prev, &callback_list) {
		info = container_of(pos, struct pynl_cbinfo, list);
		if (info->cb == cb) {
			if (unlink)
				list_del(pos, prev);
			pynl_dbg("cb=%p: found=%p\n", cb, info);
			return info;
		}
	}
	pynl_dbg("cb=%p: not found\n", cb);
	return NULL;
}

static struct pynl_cbinfo *pynl_get_cbinfo(struct nl_cb *cb, int unlink)
{
	struct pynl_cbinfo *info;

	info = pynl_find_cbinfo(cb, unlink);

	if (info || unlink) {
		/* found or no need to allocate a new one */
		pynl_dbg("cb=%p: done\n", cb);
		return info;
	}

	info = calloc(1, sizeof(*info));
	info->cb = cb;
	list_add(&info->list, &callback_list);
	pynl_dbg("cb=%p: added %p\n", cb, info);
	return info;
}

static int nl_recv_msg_handler(struct nl_msg *msg, void *arg)
{
	struct pynl_callback *cbd = arg;
	PyObject *msgobj;
	PyObject *cbparobj;
	PyObject *resobj;
	PyObject *funcobj;
	int result;

	if (!cbd) {
		result = NL_STOP;
		goto done;
	}
	msgobj = SWIG_NewPointerObj(SWIG_as_voidptr(msg),
				    SWIGTYPE_p_nl_msg, 0 |  0 );
	/* add selfobj if callback is a method */
	if (cbd->cbf && PyMethod_Check(cbd->cbf)) {
		PyObject *selfobj = PyMethod_Self(cbd->cbf);
		cbparobj = Py_BuildValue("(OOO)", selfobj ? selfobj : cbd->cba,
					 msgobj, cbd->cba);
		funcobj = PyMethod_Function(cbd->cbf);
		pynl_dbg("callback %sbounded instance method %p\n",
			 selfobj ? "" : "un", funcobj);
	} else {
		cbparobj = Py_BuildValue("(OO)", msgobj, cbd->cba);
		funcobj = cbd->cbf;
		pynl_dbg("callback function %p\n", funcobj);
	}
	resobj = PyObject_CallObject(funcobj, cbparobj);
	Py_DECREF(cbparobj);
	if (resobj && PyInt_Check(resobj))
		result = (int)PyInt_AsLong(resobj);
	else
		result = NL_STOP;
	Py_XDECREF(resobj);
done:
	pynl_dbg("result=%d\n", result);
	return result;
}

static int nl_recv_err_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
			       void *arg)
{
	struct pynl_callback *cbd = arg;
	PyObject *errobj;
	PyObject *cbparobj;
	PyObject *resobj;
	PyObject *funcobj;
	int result;

	if (!cbd)
		return NL_STOP;
	errobj = SWIG_NewPointerObj(SWIG_as_voidptr(err),
				    SWIGTYPE_p_nlmsgerr, 0 |  0 );
	/* add selfobj if callback is a method */
	if (cbd->cbf && PyMethod_Check(cbd->cbf)) {
		PyObject *selfobj = PyMethod_Self(cbd->cbf);
		cbparobj = Py_BuildValue("(OOO)", selfobj ? selfobj : cbd->cba,
					 errobj, cbd->cba);
		funcobj = PyMethod_Function(cbd->cbf);
	} else {
		cbparobj = Py_BuildValue("(OO)", errobj, cbd->cba);
		funcobj = cbd->cbf;
	}
	resobj = PyObject_CallObject(funcobj, cbparobj);
	Py_DECREF(cbparobj);
	if (resobj && PyInt_Check(resobj))
		result = (int)PyInt_AsLong(resobj);
	else
		result = NL_STOP;
	Py_XDECREF(resobj);
	pynl_dbg("error: err=%d ret=%d\n", err->error, result);
	return result;
}

%}
%inline %{
struct nl_cb *py_nl_cb_clone(struct nl_cb *cb)
{
	struct pynl_cbinfo *info, *clone_info;
	struct nl_cb *clone;
	int i;

	clone = nl_cb_clone(cb);
	info = pynl_find_cbinfo(cb, 0);
	if (info) {
		clone_info = pynl_get_cbinfo(clone, 0);
		/* increase refcnt to callback parameters and copy them */
		for (i = 0; info && i <= NL_CB_TYPE_MAX; i++) {
			Py_XINCREF(info->cbtype[i].cbf);
			Py_XINCREF(info->cbtype[i].cba);
			clone_info->cbtype[i].cbf = info->cbtype[i].cbf;
			clone_info->cbtype[i].cba = info->cbtype[i].cba;
		}
		Py_XINCREF(info->cberr.cbf);
		Py_XINCREF(info->cberr.cba);
		clone_info->cberr.cbf = info->cberr.cbf;
		clone_info->cberr.cba = info->cberr.cba;
	}
	return clone;
}

void py_nl_cb_put(struct nl_cb *cb)
{
	struct pynl_cbinfo *info;
	int i;

	/* obtain callback info (and unlink) */
	info = pynl_get_cbinfo(cb, 1);
	pynl_dbg("cb=%p, info=%p\n", cb, info);
	/* decrease refcnt for callback type handlers */
	for (i = 0; info && i <= NL_CB_TYPE_MAX; i++) {
		Py_XDECREF(info->cbtype[i].cbf);
		Py_XDECREF(info->cbtype[i].cba);
	}
	/* decrease refcnt for error handler and free callback info */
	if (info) {
		Py_XDECREF(info->cberr.cbf);
		Py_XDECREF(info->cberr.cba);
		free(info);
	}
	nl_cb_put(cb);
}

int py_nl_cb_set(struct nl_cb *cb, enum nl_cb_type t, enum nl_cb_kind k,
		PyObject *func, PyObject *a)
{
	struct pynl_cbinfo *info;

	/* obtain callback info */
	info = pynl_get_cbinfo(cb, 0);

	/* clear existing handlers (if any) */
	Py_XDECREF(info->cbtype[t].cbf);
	Py_XDECREF(info->cbtype[t].cba);
	info->cbtype[t].cbf = NULL;
	info->cbtype[t].cba = NULL;
	pynl_dbg("cb=%p, info=%p, type=%d, kind=%d\n", cb, info, t, k);
	/* handle custom callback */
	if (k == NL_CB_CUSTOM) {
		Py_XINCREF(func);
		Py_XINCREF(a);
		info->cbtype[t].cbf = func;
		info->cbtype[t].cba = a;
		return nl_cb_set(cb, t, k,
				 nl_recv_msg_handler, &info->cbtype[t]);
	}
	return nl_cb_set(cb, t, k,  NULL, NULL);
}

int py_nl_cb_set_all(struct nl_cb *cb, enum nl_cb_kind k,
		    PyObject *func , PyObject *a)
{
	struct pynl_cbinfo *info;
	int t;

	info = pynl_get_cbinfo(cb, 0);
	pynl_dbg("cb=%p, info=%p, kind=%d\n", cb, info, k);
	for (t = 0; t <= NL_CB_TYPE_MAX; t++) {
		/* (possibly) free existing handler */
		Py_XDECREF(info->cbtype[t].cbf);
		Py_XDECREF(info->cbtype[t].cba);
		info->cbtype[t].cbf = NULL;
		info->cbtype[t].cba = NULL;
		if (k == NL_CB_CUSTOM) {
			Py_XINCREF(func);
			Py_XINCREF(a);
			info->cbtype[t].cbf = func;
			info->cbtype[t].cba = a;
		}
	}
	if (k == NL_CB_CUSTOM)
		/* callback argument is same for all so using idx 0 here */
		return nl_cb_set_all(cb, k, nl_recv_msg_handler,
				     &info->cbtype[0]);
	else
		return nl_cb_set_all(cb, k, NULL, NULL);
}

int py_nl_cb_err(struct nl_cb *cb, enum nl_cb_kind k,
		PyObject *func, PyObject *a)
{
	struct pynl_cbinfo *info;

	/* obtain callback info */
	info = pynl_get_cbinfo(cb, 0);
	pynl_dbg("cb=%p, info=%p, kind=%d\n", cb, info, k);
	/* clear existing handlers (if any) */
	Py_XDECREF(info->cberr.cbf);
	Py_XDECREF(info->cberr.cba);
	info->cberr.cbf = NULL;
	info->cberr.cba = NULL;

	/* handle custom callback */
	if (k == NL_CB_CUSTOM) {
		Py_XINCREF(func);
		Py_XINCREF(a);
		info->cberr.cbf = func;
		info->cberr.cba = a;
		return nl_cb_err(cb, k,
				 nl_recv_err_handler, &info->cberr);
	}
	return nl_cb_err(cb, k,  NULL, NULL);
}
%}

/* Attributes <netlink/attr.h> */
/*
 * This typemap is a bit tricky as it uses arg1, which is knowledge about
 * the SWIGged wrapper output.
 */
%typemap(out) void * {
	$result = PyByteArray_FromStringAndSize($1, nla_len(arg1));
}
extern void *nla_data(struct nlattr *);
%typemap(out) void *;
extern int		nla_type(const struct nlattr *);

/* Integer attribute */
extern uint8_t		nla_get_u8(struct nlattr *);
extern int		nla_put_u8(struct nl_msg *, int, uint8_t);
extern uint16_t		nla_get_u16(struct nlattr *);
extern int		nla_put_u16(struct nl_msg *, int, uint16_t);
extern uint32_t		nla_get_u32(struct nlattr *);
extern int		nla_put_u32(struct nl_msg *, int, uint32_t);
extern uint64_t		nla_get_u64(struct nlattr *);
extern int		nla_put_u64(struct nl_msg *, int, uint64_t);

/* String attribute */
extern char *		nla_get_string(struct nlattr *);
extern char *		nla_strdup(struct nlattr *);
extern int		nla_put_string(struct nl_msg *, int, const char *);

/* Flag attribute */
extern int		nla_get_flag(struct nlattr *);
extern int		nla_put_flag(struct nl_msg *, int);

/* Msec attribute */
extern unsigned long	nla_get_msecs(struct nlattr *);
extern int		nla_put_msecs(struct nl_msg *, int, unsigned long);

/* Attribute nesting */
extern int		nla_put_nested(struct nl_msg *, int, struct nl_msg *);
extern struct nlattr *	nla_nest_start(struct nl_msg *, int);
extern int		nla_nest_end(struct nl_msg *, struct nlattr *);
%inline %{
PyObject *py_nla_parse_nested(int max, struct nlattr *nest_attr, PyObject *p)
{
	struct nlattr *tb_msg[max + 1];
	struct nla_policy *policy = NULL;
	void *pol;
	PyObject *attrs = Py_None;
	PyObject *k;
	PyObject *v;
	PyObject *resobj;
	int err;
	int i;

	if (p != Py_None) {
		PyObject *pobj;

		if (!PyList_Check(p)) {
			fprintf(stderr, "expected list object\n");
			err = -1;
			goto fail;
		}
		pobj = PyList_GetItem(p, 0);
		err = SWIG_ConvertPtr(pobj, &pol, SWIGTYPE_p_nla_policy, 0 |  0 );
		if (!SWIG_IsOK(err))
			goto fail;
		policy = pol;
	}
	err = nla_parse_nested(tb_msg, max, nest_attr, policy);
	if (err < 0) {
		fprintf(stderr, "Failed to parse response message\n");
	} else {
		attrs = PyDict_New();
		for (i = 0; i <= max; i++)
			if (tb_msg[i]) {
				k = PyInt_FromLong((long)i);
				v = SWIG_NewPointerObj(SWIG_as_voidptr(tb_msg[i]), SWIGTYPE_p_nlattr, 0 |  0 );
				PyDict_SetItem(attrs, k, v);
			}
	}
fail:
	if (attrs == Py_None)
		Py_INCREF(attrs);
	resobj = Py_BuildValue("(iO)", err, attrs);
	return resobj;
}

/*
 * nla_get_nested() - get list of nested attributes.
 *
 * nla_for_each_<nested|attr>() is a macro construct that needs another approach
 * for Python. Create and return list of nested attributes.
 */
PyObject *nla_get_nested(struct nlattr *nest_attr)
{
	PyObject *listobj;
	PyObject *nestattrobj;
	struct nlattr *pos;
	int rem;

	listobj = PyList_New(0);
	nla_for_each_nested(pos, nest_attr, rem) {
		nestattrobj = SWIG_NewPointerObj(SWIG_as_voidptr(pos),
						 SWIGTYPE_p_nlattr, 0 |  0 );
		PyList_Append(listobj, nestattrobj);
	}
	return listobj;
}
%}

 /**
  * @ingroup attr
  * Basic attribute data types
  *
  * See \ref attr_datatypes for more details.
  */
enum {
	NLA_UNSPEC,	/**< Unspecified type, binary data chunk */
	NLA_U8,		/**< 8 bit integer */
	NLA_U16,	/**< 16 bit integer */
	NLA_U32,	/**< 32 bit integer */
	NLA_U64,	/**< 64 bit integer */
	NLA_STRING,	/**< NUL terminated character string */
	NLA_FLAG,	/**< Flag */
	NLA_MSECS,	/**< Micro seconds (64bit) */
	NLA_NESTED,	/**< Nested attributes */
	__NLA_TYPE_MAX,
};

#define NLA_TYPE_MAX (__NLA_TYPE_MAX - 1)

/** @} */

/**
 * @ingroup attr
 * Attribute validation policy.
 *
 * See \ref attr_datatypes for more details.
 */
struct nla_policy {
	/** Type of attribute or NLA_UNSPEC */
	uint16_t	type;

	/** Minimal length of payload required */
	uint16_t	minlen;

	/** Maximal length of payload allowed */
	uint16_t	maxlen;
};

%inline %{
PyObject *nla_policy_array(int n_items)
{
	struct nla_policy *policies;
	PyObject *listobj;
	PyObject *polobj;
	int i;

	policies = calloc(n_items, sizeof(*policies));
	listobj = PyList_New(n_items);
	for (i = 0; i < n_items; i++) {
		polobj = SWIG_NewPointerObj(SWIG_as_voidptr(&policies[i]),
					    SWIGTYPE_p_nla_policy, 0 |  0 );
		PyList_SetItem(listobj, i, polobj);
	}
	return listobj;
}
%}
