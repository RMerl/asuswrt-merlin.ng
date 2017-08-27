/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

/*
 * XML output sample:
 *
 * <flow type="new">
 *	<layer3 protonum="2" protoname="IPv4">
 *		<expected>
 *			<src>192.168.0.2</src>
 *			<dst>192.168.1.2</dst>
 *		</expected>
 *		<mask>
 *			<src>255.255.255.255</src>
 *			<dst>255.255.255.255</dst>
 *		</mask>
 *		<master>
 *			<src>192.168.0.2</src>
 *			<dst>192.168.1.2</dst>
 *		</master>
 *	</layer3>
 *	<layer4 protonum="6" protoname="tcp">
 *		<expected>
 *			<sport>0</sport>
 *			<dport>41739</dport>
 *		</expected>
 *		<mask>
 *			<sport>0</sport>
 *			<dport>65535</dport>
 *		</mask>
 *		<master>
 *			<sport>36390</sport>
 *			<dport>21</dport>
 *		</master>
 *	</layer4>
 *	<meta>
 *		<helper-name>ftp</helper-name>
 *		<timeout>300</timeout>
 *		<zone>0</zone>
 *	</meta>
 * </flow>
 */

static int
snprintf_expect_meta_xml(char *buf, size_t len,
			 const struct nf_expect *exp, unsigned int flags)
{
	int ret;
	unsigned int size = 0, offset = 0;

	ret = snprintf(buf, len, "<meta>");
	BUFFER_SIZE(ret, size, len, offset);

	if (test_bit(ATTR_EXP_HELPER_NAME, exp->set)) {
		ret = snprintf(buf+offset, len,
				"<helper-name>%s</helper-name>",
				exp->helper_name);
		BUFFER_SIZE(ret, size, len, offset);
	}
	if (test_bit(ATTR_EXP_TIMEOUT, exp->set)) {
		ret = snprintf(buf+offset, len, "<timeout>%u</timeout>",
				exp->timeout);
		BUFFER_SIZE(ret, size, len, offset);
	}
	if (test_bit(ATTR_EXP_CLASS, exp->set)) {
		ret = snprintf(buf+offset, len, "<class>%u</class>",
				exp->class);
		BUFFER_SIZE(ret, size, len, offset);
	}
	if (test_bit(ATTR_EXP_ZONE, exp->set)) {
		ret = snprintf(buf+offset, len, "<zone>%u</zone>", exp->zone);
		BUFFER_SIZE(ret, size, len, offset);
	}
        if (flags & NFCT_OF_TIME) {
                time_t t;
                struct tm tm;

                t = time(NULL);
                if (localtime_r(&t, &tm) == NULL)
                        goto err_out;

                ret = snprintf(buf+offset, len, "<when>");
                BUFFER_SIZE(ret, size, len, offset);

                ret = __snprintf_localtime_xml(buf+offset, len, &tm);
                BUFFER_SIZE(ret, size, len, offset);

                ret = snprintf(buf+offset, len, "</when>");
                BUFFER_SIZE(ret, size, len, offset);
        }
err_out:
	if (exp->flags & NF_CT_EXPECT_PERMANENT) {
		ret = snprintf(buf+offset, len, "<permanent/>");
		BUFFER_SIZE(ret, size, len, offset);
	}
	if (exp->flags & NF_CT_EXPECT_INACTIVE) {
		ret = snprintf(buf+offset, len, "<inactive/>");
		BUFFER_SIZE(ret, size, len, offset);
	}
	if (exp->flags & NF_CT_EXPECT_USERSPACE) {
		ret = snprintf(buf+offset, len, "<userspace/>");
		BUFFER_SIZE(ret, size, len, offset);
	}

	ret = snprintf(buf+offset, len, "</meta>");
	BUFFER_SIZE(ret, size, len, offset);

	return size;
}

static int
snprintf_expect_layer3_xml(char *buf, size_t len, const struct nf_expect *exp)
{
	int ret;
	unsigned int size = 0, offset = 0;

        ret = snprintf(buf+offset, len,
                       "<layer3 protonum=\"%d\" protoname=\"%s\">",
			exp->expected.orig.l3protonum,
			__l3proto2str(exp->expected.orig.l3protonum));
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "<expected>");
        BUFFER_SIZE(ret, size, len, offset);

        ret = __snprintf_addr_xml(buf+offset, len, &exp->expected.orig,
				__ADDR_SRC);
        BUFFER_SIZE(ret, size, len, offset);

        ret = __snprintf_addr_xml(buf+offset, len, &exp->expected.orig,
				__ADDR_DST);
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "</expected>");
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "<mask>");
        BUFFER_SIZE(ret, size, len, offset);

        ret = __snprintf_addr_xml(buf+offset, len, &exp->mask.orig,
				__ADDR_SRC);
        BUFFER_SIZE(ret, size, len, offset);

        ret = __snprintf_addr_xml(buf+offset, len, &exp->mask.orig,
				__ADDR_DST);
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "</mask>");
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "<master>");
        BUFFER_SIZE(ret, size, len, offset);

        ret = __snprintf_addr_xml(buf+offset, len, &exp->master.orig,
				__ADDR_SRC);
        BUFFER_SIZE(ret, size, len, offset);

        ret = __snprintf_addr_xml(buf+offset, len, &exp->master.orig,
				__ADDR_DST);
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "</master>");
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "</layer3>");
        BUFFER_SIZE(ret, size, len, offset);

	return size;
}

static int
snprintf_expect_layer4_xml(char *buf, size_t len, const struct nf_expect *exp)
{
	int ret;
	unsigned int size = 0, offset = 0;

        ret = snprintf(buf+offset, len,
                       "<layer4 protonum=\"%d\" protoname=\"%s\">",
			exp->expected.orig.protonum,
			__proto2str(exp->expected.orig.protonum));
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "<expected>");
        BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_proto_xml(buf+offset, len, &exp->expected.orig,
				__ADDR_SRC);
        BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_proto_xml(buf+offset, len, &exp->expected.orig,
				__ADDR_DST);
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "</expected>");
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "<mask>");
        BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_proto_xml(buf+offset, len, &exp->mask.orig,
				__ADDR_SRC);
        BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_proto_xml(buf+offset, len, &exp->mask.orig,
				__ADDR_DST);
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "</mask>");
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "<master>");
        BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_proto_xml(buf+offset, len, &exp->master.orig,
				__ADDR_SRC);
        BUFFER_SIZE(ret, size, len, offset);

	ret = __snprintf_proto_xml(buf+offset, len, &exp->master.orig,
				__ADDR_DST);
        BUFFER_SIZE(ret, size, len, offset);

        ret = snprintf(buf+offset, len, "</master>");
        BUFFER_SIZE(ret, size, len, offset);

	ret = snprintf(buf+offset, len, "</layer4>");
        BUFFER_SIZE(ret, size, len, offset)

	return size;
}

int __snprintf_expect_xml(char *buf, unsigned int len,
			  const struct nf_expect *exp,
			  unsigned int msg_type, unsigned int flags)
{
	int ret = 0, size = 0, offset = 0;

	switch(msg_type) {
		case NFCT_T_NEW:
			ret = snprintf(buf, len, "<flow type=\"new\">");
			break;
		case NFCT_T_UPDATE:
			ret = snprintf(buf, len, "<flow type=\"update\">");
			break;
		case NFCT_T_DESTROY:
			ret = snprintf(buf, len, "<flow type=\"destroy\">");
			break;
		default:
			ret = snprintf(buf, len, "<flow>");
			break;
	}
	BUFFER_SIZE(ret, size, len, offset);

	ret = snprintf_expect_layer3_xml(buf+offset, len, exp);
	BUFFER_SIZE(ret, size, len, offset);

	ret = snprintf_expect_layer4_xml(buf+offset, len, exp);
	BUFFER_SIZE(ret, size, len, offset);

	ret = snprintf_expect_meta_xml(buf+offset, len, exp, flags);
	BUFFER_SIZE(ret, size, len, offset);

	ret = snprintf(buf+offset, len, "</flow>");
	BUFFER_SIZE(ret, size, len, offset);

	return size;
}
