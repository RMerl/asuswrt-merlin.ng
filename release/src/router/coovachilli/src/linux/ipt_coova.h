/* -*- mode: c; c-basic-offset: 8 -*- */
#ifndef _LINUX_NETFILTER_IPT_COOVA_H
#define _LINUX_NETFILTER_IPT_COOVA_H 1

#include "xt_coova.h"

#define ipt_coova_info xt_coova_mtinfo

enum {
        IPT_COOVA_CHECK    = XT_COOVA_CHECK,
        IPT_COOVA_SET      = XT_COOVA_SET,
        IPT_COOVA_UPDATE   = XT_COOVA_UPDATE,
        IPT_COOVA_REMOVE   = XT_COOVA_REMOVE,
        IPT_COOVA_TTL      = XT_COOVA_TTL,

        IPT_COOVA_SOURCE   = XT_COOVA_SOURCE,
        IPT_COOVA_DEST     = XT_COOVA_DEST,

        IPT_COOVA_NAME_LEN = XT_COOVA_NAME_LEN,
};


#endif /* _LINUX_NETFILTER_XT_COOVA_H */
