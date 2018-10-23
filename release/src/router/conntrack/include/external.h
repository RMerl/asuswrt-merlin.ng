#ifndef _EXTERNAL_H_
#define _EXTERNAL_H_

struct nf_conntrack;

struct external_handler {
	int	(*init)(void);
	void	(*close)(void);

	struct {
		void	(*new)(struct nf_conntrack *ct);
		void	(*upd)(struct nf_conntrack *ct);
		void	(*del)(struct nf_conntrack *ct);

		void	(*dump)(int fd, int type);
		void	(*flush)(void);
		int	(*commit)(struct nfct_handle *h, int fd);
		void	(*stats)(int fd);
		void	(*stats_ext)(int fd);
	} ct;
	struct {
		void	(*new)(struct nf_expect *exp);
		void	(*upd)(struct nf_expect *exp);
		void	(*del)(struct nf_expect *exp);

		void	(*dump)(int fd, int type);
		void	(*flush)(void);
		int	(*commit)(struct nfct_handle *h, int fd);
		void	(*stats)(int fd);
		void	(*stats_ext)(int fd);
	} exp;
};

extern struct external_handler external_cache;
extern struct external_handler external_inject;

#endif
