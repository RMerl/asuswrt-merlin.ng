/* -*- mode: c; c-basic-offset: 8 -*- */
#ifndef _LINUX_NETFILTER_XT_COOVA_H
#define _LINUX_NETFILTER_XT_COOVA_H 1

enum {
	XT_COOVA_SOURCE   = 0,
	XT_COOVA_DEST     = 1,

	XT_COOVA_NAME_LEN = 200,
};

struct xt_coova_mtinfo {
	u_int8_t invert;
	char name[XT_COOVA_NAME_LEN];
	u_int8_t side;
};

#endif /* _LINUX_NETFILTER_XT_COOVA_H */
