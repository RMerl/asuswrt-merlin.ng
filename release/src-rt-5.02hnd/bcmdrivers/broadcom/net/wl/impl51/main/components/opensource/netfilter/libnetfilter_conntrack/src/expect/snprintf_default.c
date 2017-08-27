/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

static int
__snprintf_expect_timeout(char *buf, unsigned int len,
			  const struct nf_expect *exp)
{
	if (test_bit(ATTR_EXP_TIMEOUT, exp->set))
		return snprintf(buf, len, "%u ", exp->timeout);

	return 0;
}

static int
__snprintf_expect_class(char *buf, unsigned int len,
			  const struct nf_expect *exp)
{
	if (test_bit(ATTR_EXP_CLASS, exp->set))
		return snprintf(buf, len, "class=%u ", exp->class);

	return 0;
}

static int __snprintf_expect_proto(char *buf, 
				   unsigned int len,
				   const struct nf_expect *exp)
{
	 return(snprintf(buf, len, "proto=%d ",
			 exp->expected.orig.protonum));
}

int __snprintf_expect_default(char *buf, 
			      unsigned int len,
			      const struct nf_expect *exp,
			      unsigned int msg_type,
			      unsigned int flags) 
{
	int ret = 0, size = 0, offset = 0;
	const char *delim = "";

	switch(msg_type) {
		case NFCT_T_NEW:
			ret = snprintf(buf, len, "%9s ", "[NEW]");
			break;
		case NFCT_T_UPDATE:
			ret = snprintf(buf, len, "%9s ", "[UPDATE]");
			break;
		case NFCT_T_DESTROY:
			ret = snprintf(buf, len, "%9s ", "[DESTROY]");
			break;
		default:
			break;
	}

	BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_expect_timeout(buf+offset, len, exp);
	BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_expect_proto(buf+offset, len, exp);
	BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_address(buf+offset, len, &exp->expected.orig,
				 "src", "dst");
	BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_proto(buf+offset, len, &exp->expected.orig);
	BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_address(buf+offset, len, &exp->mask.orig,
				 "mask-src", "mask-dst");
	BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_proto(buf+offset, len,
				&exp->mask.orig);
	BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_address(buf+offset, len, &exp->master.orig,
				 "master-src", "master-dst");
	BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_proto(buf+offset, len,
				&exp->master.orig);
	BUFFER_SIZE(ret, size, len, offset);

	if (test_bit(ATTR_EXP_ZONE, exp->set)) {
		ret = snprintf(buf+offset, len, "zone=%u ", exp->zone);
		BUFFER_SIZE(ret, size, len, offset);
	}

	if (exp->flags & NF_CT_EXPECT_PERMANENT) {
		ret = snprintf(buf+offset, len, "PERMANENT");
		BUFFER_SIZE(ret, size, len, offset);
		delim = ",";
	}
	if (exp->flags & NF_CT_EXPECT_INACTIVE) {
		ret = snprintf(buf+offset, len, "%sINACTIVE", delim);
		BUFFER_SIZE(ret, size, len, offset);
		delim = ",";
	}
	if (exp->flags & NF_CT_EXPECT_USERSPACE) {
		ret = snprintf(buf+offset, len, "%sUSERSPACE", delim);
		BUFFER_SIZE(ret, size, len, offset);
	}
	/* extra space not to stick to next field. */
	if (exp->flags) {
		ret = snprintf(buf+offset, len, " ");
		BUFFER_SIZE(ret, size, len, offset);
	}
	ret = __snprintf_expect_class(buf+offset, len, exp);
	BUFFER_SIZE(ret, size, len, offset);

	if (test_bit(ATTR_EXP_HELPER_NAME, exp->set)) {
		ret = snprintf(buf+offset, len, "helper=%s", exp->helper_name);
		BUFFER_SIZE(ret, size, len, offset);
	}

	/* Delete the last blank space if needed */
	if (len > 0 && buf[size-1] == ' ')
		size--;

	return size;
}
