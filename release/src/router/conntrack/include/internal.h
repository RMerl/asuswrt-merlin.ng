#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

struct nf_conntrack;

enum {
	INTERNAL_F_POPULATE	= (1 << 0),
	INTERNAL_F_RESYNC	= (1 << 1),
	INTERNAL_F_MAX		= (1 << 2)
};

struct internal_handler {
	unsigned int flags;

	int	(*init)(void);
	void	(*close)(void);

	struct {
		void	*data;

		void	(*new)(struct nf_conntrack *ct, int origin_type);
		void	(*upd)(struct nf_conntrack *ct, int origin_type);
		int	(*del)(struct nf_conntrack *ct, int origin_type);

		void	(*dump)(int fd, int type);
		void	(*populate)(struct nf_conntrack *ct);
		void	(*purge)(void);
		int	(*resync)(enum nf_conntrack_msg_type type,
				  struct nf_conntrack *ct, void *data);
		void	(*flush)(void);

		void	(*stats)(int fd);
		void	(*stats_ext)(int fd);
	} ct;
	struct {
		void	*data;

		void	(*new)(struct nf_expect *exp, int origin_type);
		void	(*upd)(struct nf_expect *exp, int origin_type);
		int	(*del)(struct nf_expect *exp, int origin_type);
		int	(*find)(const struct nf_conntrack *master);

		void	(*dump)(int fd, int type);
		void	(*populate)(struct nf_expect *exp);
		void	(*purge)(void);
		int	(*resync)(enum nf_conntrack_msg_type type,
				  struct nf_expect *exp, void *data);
		void	(*flush)(void);

		void	(*stats)(int fd);
		void	(*stats_ext)(int fd);
	} exp;
};

extern struct internal_handler internal_cache;
extern struct internal_handler internal_bypass;

#endif
