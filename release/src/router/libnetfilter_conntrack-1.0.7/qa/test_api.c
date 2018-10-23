/*
 * Run this after adding a new attribute to the nf_conntrack object
 */

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

/*
 * this file contains a test to check the set/get/copy/cmp APIs.
 */

static void eval_sigterm(int status)
{
	switch(WTERMSIG(status)) {
	case SIGSEGV:
		printf("received SIGSEV\n");
		break;
	case 0:
		printf("OK\n");
		break;
	default:
		printf("exited with signal: %d\n", WTERMSIG(status));
		break;
	}
}

static void test_nfct_bitmask(void)
{
	struct nfct_bitmask *a, *b;
	unsigned short int maxb, i;
	struct nf_conntrack *ct1, *ct2;

	printf("== test nfct_bitmask_* API ==\n");

	maxb = rand() & 0xffff;

	a = nfct_bitmask_new(maxb);

	assert(!nfct_bitmask_test_bit(a, maxb + 32));
	nfct_bitmask_set_bit(a, maxb + 32);
	assert(!nfct_bitmask_test_bit(a, maxb + 32));

	for (i = 0; i <= maxb; i++)
		assert(!nfct_bitmask_test_bit(a, i));

	for (i = 0; i <= maxb; i++) {
		if (rand() & 1) {
			assert(!nfct_bitmask_test_bit(a, i));
			continue;
		}
		nfct_bitmask_set_bit(a, i);
		assert(nfct_bitmask_test_bit(a, i));
	}

	b = nfct_bitmask_clone(a);
	assert(b);

	for (i = 0; i <= maxb; i++) {
		if (nfct_bitmask_test_bit(a, i))
			assert(nfct_bitmask_test_bit(b, i));
		else
			assert(!nfct_bitmask_test_bit(b, i));
	}

	nfct_bitmask_destroy(a);

	for (i = 0; i <= maxb; i++) {
		if (rand() & 1)
			continue;
		nfct_bitmask_unset_bit(b, i);
		assert(!nfct_bitmask_test_bit(b, i));
	}

	/* nfct_bitmask_clear() */
	for (i = 0; i < maxb; i++) {
		nfct_bitmask_set_bit(b, i);
		assert(nfct_bitmask_test_bit(b, i));
		nfct_bitmask_clear(b);
		assert(!nfct_bitmask_test_bit(b, i));
	}

	for (i = 0; i < maxb; i++)
		nfct_bitmask_set_bit(b, i);
	nfct_bitmask_clear(b);
	for (i = 0; i < maxb; i++)
		assert(!nfct_bitmask_test_bit(b, i));

	/* nfct_bitmask_equal() */
	for (i = 0; i < maxb / 32 * 32; i += 32) {
		a = nfct_bitmask_new(i);
		assert(!nfct_bitmask_equal(a, b));
		nfct_bitmask_destroy(a);
	}

	a = nfct_bitmask_clone(b);
	assert(nfct_bitmask_equal(a, b));
	for (i = 0; i < maxb; i++) {
		if (nfct_bitmask_test_bit(a, i)) {
			nfct_bitmask_unset_bit(a, i);
			assert(!nfct_bitmask_equal(a, b));
			nfct_bitmask_set_bit(a, i);
		} else {
			nfct_bitmask_set_bit(a, i);
			assert(!nfct_bitmask_equal(a, b));
			nfct_bitmask_unset_bit(a, i);
		}
		assert(nfct_bitmask_equal(a, b));
	}

	nfct_bitmask_destroy(a);
	nfct_bitmask_destroy(b);

	ct1 = nfct_new();
	ct2 = nfct_new();

	maxb = rand() & 0xff;
	maxb += 128;
	maxb /= 2;
	a = nfct_bitmask_new(maxb * 2);
	b = nfct_bitmask_new(maxb);
	nfct_set_attr(ct1, ATTR_CONNLABELS, a);
	nfct_set_attr(ct2, ATTR_CONNLABELS, b);

	assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL) == 1);

	nfct_bitmask_set_bit(a, maxb);
	nfct_bitmask_set_bit(b, maxb);
	assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL) == 1);

	nfct_bitmask_set_bit(a, maxb * 2);
	assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL) == 0);
	nfct_destroy(ct1);
	nfct_destroy(ct2);
	printf("OK\n");
}

/* These attributes cannot be set, ignore them. */
static int attr_is_readonly(int attr)
{
	switch (attr) {
	case ATTR_ORIG_COUNTER_PACKETS:
	case ATTR_REPL_COUNTER_PACKETS:
	case ATTR_ORIG_COUNTER_BYTES:
	case ATTR_REPL_COUNTER_BYTES:
	case ATTR_USE:
	case ATTR_SECCTX:
	case ATTR_TIMESTAMP_START:
	case ATTR_TIMESTAMP_STOP:
		return 1;
	}
	return 0;
}


static int test_nfct_cmp_api_single(struct nf_conntrack *ct1,
				struct nf_conntrack *ct2, int attr)
{
	char data[256];
	struct nfct_bitmask *b;
	int bit;

	if (attr_is_readonly(attr))
		return 0;

	switch (attr) {
	case ATTR_SECMARK: /* obsolete */
		return 0;

	/* FIXME: not implemented comparators: */
	case ATTR_SNAT_IPV4:
	case ATTR_DNAT_IPV4:
	case ATTR_SNAT_IPV6:
	case ATTR_DNAT_IPV6:
	case ATTR_SNAT_PORT:
	case ATTR_DNAT_PORT:

	case ATTR_TCP_FLAGS_ORIG:
	case ATTR_TCP_FLAGS_REPL:
	case ATTR_TCP_MASK_ORIG:
	case ATTR_TCP_MASK_REPL:

	case ATTR_MASTER_IPV4_SRC:
	case ATTR_MASTER_IPV4_DST:
	case ATTR_MASTER_IPV6_SRC:
	case ATTR_MASTER_IPV6_DST:
	case ATTR_MASTER_PORT_SRC:
	case ATTR_MASTER_PORT_DST:
	case ATTR_MASTER_L3PROTO:
	case ATTR_MASTER_L4PROTO:

	case ATTR_ORIG_NAT_SEQ_CORRECTION_POS:
	case ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE:
	case ATTR_ORIG_NAT_SEQ_OFFSET_AFTER:
	case ATTR_REPL_NAT_SEQ_CORRECTION_POS:
	case ATTR_REPL_NAT_SEQ_OFFSET_BEFORE:
	case ATTR_REPL_NAT_SEQ_OFFSET_AFTER:

	case ATTR_SCTP_VTAG_ORIG:
	case ATTR_SCTP_VTAG_REPL:

	case ATTR_HELPER_NAME:

	case ATTR_DCCP_ROLE:
	case ATTR_DCCP_HANDSHAKE_SEQ:

	case ATTR_TCP_WSCALE_ORIG:
	case ATTR_TCP_WSCALE_REPL:

	case ATTR_HELPER_INFO:
		return 0; /* XXX */

	default:
		break;
	}

	if (attr >= ATTR_SCTP_STATE) {
		nfct_set_attr_u8(ct1, ATTR_REPL_L4PROTO, IPPROTO_SCTP);
		nfct_set_attr_u8(ct1, ATTR_L4PROTO, IPPROTO_SCTP);
	} else if (attr >= ATTR_TCP_FLAGS_ORIG) {
		nfct_set_attr_u8(ct1, ATTR_REPL_L4PROTO, IPPROTO_TCP);
		nfct_set_attr_u8(ct1, ATTR_L4PROTO, IPPROTO_TCP);
	} else if (attr >= ATTR_ICMP_CODE) {
		nfct_set_attr_u8(ct1, ATTR_REPL_L4PROTO, IPPROTO_ICMP);
		nfct_set_attr_u8(ct1, ATTR_L4PROTO, IPPROTO_ICMP);
	} else if (attr >= ATTR_ORIG_PORT_SRC) {
		nfct_set_attr_u8(ct1, ATTR_REPL_L4PROTO, IPPROTO_TCP);
		nfct_set_attr_u8(ct1, ATTR_L4PROTO, IPPROTO_TCP);
	}

	nfct_copy(ct2, ct1, NFCT_CP_OVERRIDE);
	memset(data, 42, sizeof(data));

	assert(nfct_attr_is_set(ct1, attr));
	assert(nfct_attr_is_set(ct2, attr));

	switch (attr) {
	case ATTR_CONNLABELS:
	case ATTR_CONNLABELS_MASK:
		b = (void *) nfct_get_attr(ct1, attr);
		assert(b);
		b = nfct_bitmask_clone(b);
		assert(b);
		bit = nfct_bitmask_maxbit(b);
		if (nfct_bitmask_test_bit(b, bit)) {
			nfct_bitmask_unset_bit(b, bit);
			assert(!nfct_bitmask_test_bit(b, bit));
		} else {
			nfct_bitmask_set_bit(b, bit);
			assert(nfct_bitmask_test_bit(b, bit));
		}
		assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL) == 1);
		nfct_set_attr(ct2, attr, b);
		break;
	case ATTR_HELPER_INFO:
		nfct_set_attr_l(ct2, attr, "test", 4);
		break;
	default:
		nfct_set_attr(ct2, attr, data);
		break;
	}

	if (nfct_cmp(ct1, ct2, NFCT_CMP_ALL) != 0) {
		fprintf(stderr, "nfct_cmp assert failure for attr %d\n", attr);
		fprintf(stderr, "%p, %p, %x, %x\n", nfct_get_attr(ct1, attr),
				nfct_get_attr(ct2, attr),
				nfct_get_attr_u32(ct1, attr), nfct_get_attr_u32(ct2, attr));
		return -1;
	}
	if (nfct_cmp(ct1, ct2, NFCT_CMP_ALL|NFCT_CMP_STRICT) != 0) {
		fprintf(stderr, "nfct_cmp strict assert failure for attr %d\n", attr);
		return -1;
	}
	return 0;
}

static int test_cmp_attr32(int attr, bool at1, bool at2,
			   uint32_t v1, uint32_t v2, unsigned int flags)
{
	struct nf_conntrack *ct1 = nfct_new();
	struct nf_conntrack *ct2 = nfct_new();
	int ret;

	if (at1)
		nfct_set_attr_u32(ct1, attr, v1);
	if (at2)
		nfct_set_attr_u32(ct2, attr, v2);

	ret = nfct_cmp(ct1, ct2, NFCT_CMP_ALL | flags);

	nfct_destroy(ct1);
	nfct_destroy(ct2);

	return ret;
}

static void test_nfct_cmp_attr(int attr)
{
	unsigned int flags = 0;

	/* 0000, 1000, 1100, 0010, 1010... */
	/*		       attr       at1    at2    v1	v2 */
	assert(test_cmp_attr32(attr, false, false,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, true,  false,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, false, true,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, true,  true,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, false, false,	1,	0,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	1,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, false, true,	1,	0,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  true,	1,	0,	flags) == 0);
	assert(test_cmp_attr32(attr, false, false,	0,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	0,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, false, true,	0,	1,	flags) == 1);
	assert(test_cmp_attr32(attr, true,  true,	0,	1,	flags) == 0);
	assert(test_cmp_attr32(attr, false, false,	1,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	1,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, false, true,	1,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  true,	1,	1,	flags) == 1);

	flags = NFCT_CMP_STRICT;
	assert(test_cmp_attr32(attr, false, false,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, true,  false,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, false, true,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, true,  true,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, false, false,	1,	0,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	1,	0,	flags) == 0);
	assert(test_cmp_attr32(attr, false, true,	1,	0,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  true,	1,	0,	flags) == 0);
	assert(test_cmp_attr32(attr, false, false,	0,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	0,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, false, true,	0,	1,	flags) == 0);
	assert(test_cmp_attr32(attr, true,  true,	0,	1,	flags) == 0);
	assert(test_cmp_attr32(attr, false, false,	1,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	1,	1,	flags) == 0); /* verbose */
	assert(test_cmp_attr32(attr, false, true,	1,	1,	flags) == 0); /* verbose */
	assert(test_cmp_attr32(attr, true,  true,	1,	1,	flags) == 1);

	flags = NFCT_CMP_MASK;
	assert(test_cmp_attr32(attr, false, false,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, true,  false,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, false, true,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, true,  true,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, false, false,	1,	0,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	1,	0,	flags) == 0);
	assert(test_cmp_attr32(attr, false, true,	1,	0,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  true,	1,	0,	flags) == 0);
	assert(test_cmp_attr32(attr, false, false,	0,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	0,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, false, true,	0,	1,	flags) == 1);
	assert(test_cmp_attr32(attr, true,  true,	0,	1,	flags) == 0);
	assert(test_cmp_attr32(attr, false, false,	1,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	1,	1,	flags) == 0); /* verbose */
	assert(test_cmp_attr32(attr, false, true,	1,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  true,	1,	1,	flags) == 1);

	flags = NFCT_CMP_STRICT|NFCT_CMP_MASK;
	assert(test_cmp_attr32(attr, false, false,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, true,  false,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, false, true,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, true,  true,	0,	0,	flags) == 1);
	assert(test_cmp_attr32(attr, false, false,	1,	0,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	1,	0,	flags) == 0);
	assert(test_cmp_attr32(attr, false, true,	1,	0,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  true,	1,	0,	flags) == 0);
	assert(test_cmp_attr32(attr, false, false,	0,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	0,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, false, true,	0,	1,	flags) == 0);
	assert(test_cmp_attr32(attr, true,  true,	0,	1,	flags) == 0);
	assert(test_cmp_attr32(attr, false, false,	1,	1,	flags) == 1); /* verbose */
	assert(test_cmp_attr32(attr, true,  false,	1,	1,	flags) == 0); /* verbose */
	assert(test_cmp_attr32(attr, false, true,	1,	1,	flags) == 0); /* verbose */
	assert(test_cmp_attr32(attr, true,  true,	1,	1,	flags) == 1);
}

static void test_nfct_cmp_api(struct nf_conntrack *ct1, struct nf_conntrack *ct2)
{
	int i;

	printf("== test cmp API ==\n");

	test_nfct_cmp_attr(ATTR_ZONE);
	test_nfct_cmp_attr(ATTR_ORIG_ZONE);
	test_nfct_cmp_attr(ATTR_REPL_ZONE);
	test_nfct_cmp_attr(ATTR_MARK);

	assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL) == 1);
	assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL|NFCT_CMP_STRICT) == 0);

	nfct_copy(ct1, ct2, NFCT_CP_OVERRIDE);

	assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL) == 1);
	assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL|NFCT_CMP_STRICT) == 1);

	for (i=0; i < ATTR_MAX ; i++) {
		nfct_attr_unset(ct1, i);

		assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL) == 1);
		assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL|NFCT_CMP_STRICT) == 0);
		assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL|NFCT_CMP_MASK) == 1);
	}
	nfct_copy(ct1, ct2, NFCT_CP_OVERRIDE);
	for (i=0; i < ATTR_MAX ; i++) {
		nfct_attr_unset(ct2, i);

		assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL) == 1);
		assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL|NFCT_CMP_STRICT) == 0);
		assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL|NFCT_CMP_MASK) == 0);
	}

	for (i=0; i < ATTR_MAX ; i++)
		assert(test_nfct_cmp_api_single(ct1, ct2, i) == 0);

	nfct_copy(ct2, ct1, NFCT_CP_OVERRIDE);
	for (i=0; i < ATTR_MAX ; i++) {
		nfct_attr_unset(ct1, i);
		nfct_attr_unset(ct2, i);

		assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL) == 1);
		assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL|NFCT_CMP_STRICT) == 1);
		assert(nfct_cmp(ct1, ct2, NFCT_CMP_ALL|NFCT_CMP_MASK) == 1);
	}
	nfct_destroy(ct1);
	nfct_destroy(ct2);
}

static void test_nfexp_cmp_api(struct nf_expect *ex1, struct nf_expect *ex2)
{
	int i;

	printf("== test expect cmp API ==\n");

	/* XXX: missing nfexp_copy API. */
	memcpy(ex1, ex2, nfexp_maxsize());

	assert(nfexp_cmp(ex1, ex2, 0) == 1);
	assert(nfexp_cmp(ex1, ex2, NFCT_CMP_STRICT) == 1);

	assert(nfexp_attr_is_set(ex1, 0) == 1);
	nfexp_attr_unset(ex1, 0);
	assert(nfexp_attr_is_set(ex1, 0) == 0);

	memcpy(ex1, ex2, nfexp_maxsize());
	for (i=0; i < ATTR_EXP_MAX; i++) {
		nfexp_attr_unset(ex1, i);

		assert(nfexp_cmp(ex1, ex2, 0) == 1);
		assert(nfexp_cmp(ex1, ex2, NFCT_CMP_STRICT) == 0);
		assert(nfexp_cmp(ex1, ex2, NFCT_CMP_MASK) == 1);
	}
	memcpy(ex1, ex2, nfexp_maxsize());
	for (i=0; i < ATTR_EXP_MAX; i++) {
		nfexp_attr_unset(ex2, i);

		assert(nfexp_cmp(ex1, ex2, 0) == 1);
		assert(nfexp_cmp(ex1, ex2, NFCT_CMP_MASK) == 0);
	}
	memcpy(ex1, ex2, nfexp_maxsize());
	for (i=0; i < ATTR_EXP_MAX; i++) {
		nfexp_attr_unset(ex1, i);
		nfexp_attr_unset(ex2, i);

		assert(nfexp_cmp(ex1, ex2, 0) == 1);
		assert(nfexp_cmp(ex1, ex2, NFCT_CMP_STRICT) == 1);
		assert(nfexp_cmp(ex1, ex2, NFCT_CMP_MASK) == 1);
	}
	nfexp_destroy(ex1);
	nfexp_destroy(ex2);
}

int main(void)
{
	int ret, i;
	struct nf_conntrack *ct, *ct2, *tmp;
	struct nf_expect *exp, *tmp_exp;
	char data[256];
	const char *val;
	int status;
	struct nfct_bitmask *b, *b2;

	srand(time(NULL));

	/* initialize fake data for testing purposes */
	for (i=0; i<sizeof(data); i++)
		data[i] = 0x01;

	ct = nfct_new();
	if (!ct) {
		perror("nfct_new");
		return 0;
	}
	tmp = nfct_new();
	if (!tmp) {
		perror("nfct_new");
		return 0;
	}

	printf("== test set API ==\n");
	ret = fork();
	if (ret == 0) {
		for (i=0; i<ATTR_MAX; i++)
			nfct_set_attr(ct, i, data);
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	b = nfct_bitmask_new(rand() & 0xffff);
	assert(b);
	b2 = nfct_bitmask_new(rand() & 0xffff);
	assert(b2);

	for (i=0; i<ATTR_MAX; i++) {
		switch (i) {
		case ATTR_CONNLABELS:
			nfct_set_attr(ct, i, b);
			break;
		case ATTR_CONNLABELS_MASK:
			nfct_set_attr(ct, i, b2);
			break;
		default:
			nfct_set_attr(ct, i, data);
			break;
		}
	}

	printf("== test get API ==\n");
	ret = fork();
	if (ret == 0) {
		for (i=0; i<ATTR_MAX; i++)
			nfct_get_attr(ct, i);
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	printf("== validate set API ==\n");
	ret = fork();
	if (ret == 0) {
		for (i=0; i<ATTR_MAX; i++) {
			if (attr_is_readonly(i))
				continue;
			switch(i) {
			/* These attributes require special handling */
			case ATTR_HELPER_INFO:
				nfct_set_attr_l(ct, i, data, sizeof(data));
				break;
			case ATTR_CONNLABELS:
			case ATTR_CONNLABELS_MASK:
				/* already set above */
				break;
			default:
				data[0] = (uint8_t) i;
				nfct_set_attr(ct, i, data);
			}
			val = nfct_get_attr(ct, i);
			switch (i) {
			case ATTR_CONNLABELS:
				assert((void *) val == b);
				continue;
			case ATTR_CONNLABELS_MASK:
				assert((void *) val == b2);
				continue;
			}

			if (val[0] != data[0]) {
				printf("ERROR: set/get operations don't match "
				       "for attribute %d (%x != %x)\n",
					i, val[0], data[0]);
			}
		}
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	printf("== test copy API ==\n");
	ret = fork();
	if (ret == 0) {
		for (i=0; i<ATTR_MAX; i++)
			nfct_copy_attr(tmp, ct, i);
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	ret = fork();
	if (ret == 0) {
		test_nfct_cmp_api(tmp, ct);
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	exp = nfexp_new();
	if (!exp) {
		perror("nfexp_new");
		return 0;
	}
	tmp_exp = nfexp_new();
	if (!tmp_exp) {
		perror("nfexp_new");
		return 0;
	}

	printf("== test expect set API ==\n");
	ret = fork();
	if (ret == 0) {
		for (i=0; i<ATTR_EXP_MAX; i++)
			nfexp_set_attr(exp, i, data);
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	for (i=0; i<ATTR_EXP_MAX; i++)
		nfexp_set_attr(exp, i, data);

	printf("== test expect get API ==\n");
	ret = fork();
	if (ret == 0) {
		for (i=0; i<ATTR_EXP_MAX; i++)
			nfexp_get_attr(exp, i);
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	printf("== validate expect set API ==\n");
	ret = fork();
	if (ret == 0) {
		for (i=0; i<ATTR_EXP_MAX; i++) {
			data[0] = (uint8_t) i;
			nfexp_set_attr(exp, i, data);
			val = nfexp_get_attr(exp, i);
			if (val[0] != data[0]) {
				printf("ERROR: set/get operations don't match "
				       "for attribute %d (%x != %x)\n",
					i, val[0], data[0]);
			}
		}
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	ret = fork();
	if (ret == 0) {
		test_nfexp_cmp_api(tmp_exp, exp);
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	ct2 = nfct_new();
	if (!ct2) {
		perror("nfct_new");
		return 0;
	}

	printf("== test set grp API ==\n");
	ret = fork();
	if (ret == 0) {
		for (i=0; i<ATTR_GRP_MAX; i++)
			nfct_set_attr_grp(ct2, i, data);
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	for (i=0; i<ATTR_GRP_MAX; i++)
		nfct_set_attr_grp(ct2, i, data);

	printf("== test get grp API ==\n");
	ret = fork();
	if (ret == 0) {
		char buf[32]; /* IPv6 group address is 16 bytes * 2 */

		for (i=0; i<ATTR_GRP_MAX; i++)
			nfct_get_attr_grp(ct2, i, buf);
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	printf("== validate set grp API ==\n");
	ret = fork();
	if (ret == 0) {
		for (i=0; i<ATTR_GRP_MAX; i++) {
			char buf[32]; /* IPv6 group address is 16 bytes */

			data[0] = (uint8_t) i;
			nfct_set_attr_grp(ct2, i, data);
			nfct_get_attr_grp(ct2, i, buf);
			/* These attributes cannot be set, ignore them. */
			switch(i) {
			case ATTR_GRP_ORIG_COUNTERS:
			case ATTR_GRP_REPL_COUNTERS:
			case ATTR_GRP_ORIG_ADDR_SRC:
			case ATTR_GRP_ORIG_ADDR_DST:
			case ATTR_GRP_REPL_ADDR_SRC:
			case ATTR_GRP_REPL_ADDR_DST:
				continue;
			}
			if (buf[0] != data[0]) {
				printf("ERROR: set/get operations don't match "
				       "for attribute %d (%x != %x)\n",
					i, buf[0], data[0]);
			}
		}
		exit(0);
	} else {
		wait(&status);
		eval_sigterm(status);
	}

	nfct_destroy(ct2);
	printf("== destroy cloned ct entry ==\n");
	nfct_destroy(ct);
	nfct_destroy(tmp);
	nfexp_destroy(exp);
	nfexp_destroy(tmp_exp);

	printf("OK\n");

	test_nfct_bitmask();

	return EXIT_SUCCESS;
}
