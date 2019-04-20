#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <libmnl/libmnl.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

static void print_label(struct nfct_labelmap *map)
{
	int b = nfct_labelmap_get_bit(map, "test label 1");
	assert(b == 1);

	b = nfct_labelmap_get_bit(map, "zero");
	assert(b == 0);

	b = nfct_labelmap_get_bit(map, "test label 2");
	assert(b == 2);

	b = nfct_labelmap_get_bit(map, "duplicate");
	assert(b < 0);

	b = nfct_labelmap_get_bit(map, "invalid label");
	assert(b < 0);

	b = nfct_labelmap_get_bit(map, "T");
	assert(b == 42);
}

static void dump_map(struct nfct_labelmap *map)
{
	unsigned int i = 0;

	for (;;) {
		const char *name = nfct_labelmap_get_name(map, i);
		if (!name)
			break;
		if (name[0])
			printf("\t\"%s\", bit %d\n", name, i);
		i++;
	}
}

int main(void)
{
	struct nfct_labelmap *l;

	l = nfct_labelmap_new("/");
	assert(l == NULL);

	l = nfct_labelmap_new(NULL);
	if (l) {
		puts("default connlabel.conf:");
		dump_map(l);
		nfct_labelmap_destroy(l);
	} else {
		puts("no default config found");
	}

	l = nfct_labelmap_new("qa-connlabel.conf");
	if (!l)
		l = nfct_labelmap_new("qa/qa-connlabel.conf");
	assert(l);
	puts("qa-connlabel.conf:");
	dump_map(l);
	print_label(l);
	nfct_labelmap_destroy(l);

	return 0;
}
