/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _ILA_COMMON_H_
#define _ILA_COMMON_H_

#include <linux/ila.h>
#include <string.h>

static inline char *ila_csum_mode2name(__u8 csum_mode)
{
	switch (csum_mode) {
	case ILA_CSUM_ADJUST_TRANSPORT:
		return "adj-transport";
	case ILA_CSUM_NEUTRAL_MAP:
		return "neutral-map";
	case ILA_CSUM_NO_ACTION:
		return "no-action";
	case ILA_CSUM_NEUTRAL_MAP_AUTO:
		return "neutral-map-auto";
	default:
		return "unknown";
	}
}

static inline int ila_csum_name2mode(char *name)
{
	if (strcmp(name, "adj-transport") == 0)
		return ILA_CSUM_ADJUST_TRANSPORT;
	else if (strcmp(name, "neutral-map") == 0)
		return ILA_CSUM_NEUTRAL_MAP;
	else if (strcmp(name, "neutral-map-auto") == 0)
		return ILA_CSUM_NEUTRAL_MAP_AUTO;
	else if (strcmp(name, "no-action") == 0)
		return ILA_CSUM_NO_ACTION;
	else if (strcmp(name, "neutral-map-auto") == 0)
		return ILA_CSUM_NEUTRAL_MAP_AUTO;
	else
		return -1;
}

static inline char *ila_ident_type2name(__u8 ident_type)
{
	switch (ident_type) {
	case ILA_ATYPE_IID:
		return "iid";
	case ILA_ATYPE_LUID:
		return "luid";
	case ILA_ATYPE_VIRT_V4:
		return "virt-v4";
	case ILA_ATYPE_VIRT_UNI_V6:
		return "virt-uni-v6";
	case ILA_ATYPE_VIRT_MULTI_V6:
		return "virt-multi-v6";
	case ILA_ATYPE_NONLOCAL_ADDR:
		return "nonlocal-addr";
	case ILA_ATYPE_USE_FORMAT:
		return "use-format";
	default:
		return "unknown";
	}
}

static inline int ila_ident_name2type(char *name)
{
	if (!strcmp(name, "luid"))
		return ILA_ATYPE_LUID;
	else if (!strcmp(name, "use-format"))
		return ILA_ATYPE_USE_FORMAT;
#if 0 /* No kernel support for configuring these yet */
	else if (!strcmp(name, "iid"))
		return ILA_ATYPE_IID;
	else if (!strcmp(name, "virt-v4"))
		return ILA_ATYPE_VIRT_V4;
	else if (!strcmp(name, "virt-uni-v6"))
		return ILA_ATYPE_VIRT_UNI_V6;
	else if (!strcmp(name, "virt-multi-v6"))
		return ILA_ATYPE_VIRT_MULTI_V6;
	else if (!strcmp(name, "nonlocal-addr"))
		return ILA_ATYPE_NONLOCAL_ADDR;
#endif
	else
		return -1;
}

static inline char *ila_hook_type2name(__u8 hook_type)
{
	switch (hook_type) {
	case ILA_HOOK_ROUTE_OUTPUT:
		return "output";
	case ILA_HOOK_ROUTE_INPUT:
		return "input";
	default:
		return "unknown";
	}
}

static inline int ila_hook_name2type(char *name)
{
	if (!strcmp(name, "output"))
		return ILA_HOOK_ROUTE_OUTPUT;
	else if (!strcmp(name, "input"))
		return ILA_HOOK_ROUTE_INPUT;
	else
		return -1;
}

#endif /* _ILA_COMMON_H_ */
