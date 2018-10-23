#include <stdbool.h>
#include <stdint.h>

#include "internal/internal.h"

#define MAX_BITS 1024

#define CONNLABEL_CFG "/etc/xtables/connlabel.conf"
#define HASH_SIZE 64

struct labelmap_bucket {
	char *name;
	unsigned int bit;
	struct labelmap_bucket *next;
};

struct nfct_labelmap {
	struct labelmap_bucket *map_name[HASH_SIZE];
	unsigned int namecount;
	char **bit_to_name;
};

static struct labelmap_bucket* label_map_bucket_alloc(const char *n, unsigned int b)
{
	struct labelmap_bucket *bucket;
	char *name = strdup(n);

	if (!name)
		return NULL;

	bucket = malloc(sizeof(*bucket));
	if (!bucket) {
		free(name);
		return NULL;
	}
	bucket->name = name;
	bucket->bit = b;
	return bucket;
}

static unsigned int hash_name(const char *name)
{
	unsigned int hash = 0;

	while (*name) {
		hash = (hash << 5) - hash + *name;
		name++;
	}
	return hash & (HASH_SIZE - 1);
}

int __labelmap_get_bit(struct nfct_labelmap *m, const char *name)
{
	struct labelmap_bucket *list;
	unsigned int i;

	if (!m)
		return -1;

	i = hash_name(name);
	list = m->map_name[i];

	while (list) {
		if (strcmp(name, list->name) == 0)
			return list->bit;
		list = list->next;
	}
	return -1;
}

const char *__labelmap_get_name(struct nfct_labelmap *m, unsigned int bit)
{
	if (bit < m->namecount)
		return m->bit_to_name[bit] ? m->bit_to_name[bit] : "";
	return NULL;
}

static int map_insert(struct nfct_labelmap *m, const char *n, unsigned int b)
{
	unsigned int i = hash_name(n);
	struct labelmap_bucket *list = m->map_name[i];

	while (list) {
		if (strcmp(list->name, n) == 0)
			return -1;
		list = list->next;
	}

	list = label_map_bucket_alloc(n, b);
	if (!list)
		return -1;

	if (m->map_name[i])
		list->next = m->map_name[i];
	else
		list->next = NULL;
	m->map_name[i] = list;
	return 0;
}

static int is_space_posix(int c)
{
	return c == ' ' || c == '\f' || c == '\r' || c == '\t' || c == '\v';
}

static char *trim_label(char *label)
{
	char *end;

	while (is_space_posix(*label))
		label++;
	end = strchr(label, '\n');
	if (end)
		*end = 0;
	else
		end = strchr(label, '\0');
	end--;

	while (end > label && is_space_posix(*end)) {
		*end = 0;
		end--;
	}

	return *label ? label : NULL;
}

static int
xtables_parse_connlabel_numerical(const char *s, char **end)
{
	unsigned long value;

	value = strtoul(s, end, 0);
	if (value == 0 && s == *end)
		return -1;
	if (value >= MAX_BITS)
		return -1;
	return value;
}

static void free_list(struct labelmap_bucket *b)
{
	struct labelmap_bucket *tmp;

	while (b) {
		free(b->name);

		tmp = b;
		b = b->next;

		free(tmp);
	}
}

void __labelmap_destroy(struct nfct_labelmap *map)
{
	unsigned int i;
	struct labelmap_bucket *b;

	for (i = 0; i < HASH_SIZE; i++) {
		b = map->map_name[i];
		free_list(b);
	}

	free(map->bit_to_name);
	free(map);
}

static void make_name_table(struct nfct_labelmap *m)
{
	struct labelmap_bucket *b;
	unsigned int i;

	for (i = 0; i < HASH_SIZE; i++) {
		b = m->map_name[i];
		while (b) {
			m->bit_to_name[b->bit] = b->name;
			b = b->next;
		}
	}
}

static struct nfct_labelmap *map_alloc(void)
{
	struct nfct_labelmap *map = malloc(sizeof(*map));
	if (map) {
		unsigned int i;
		for (i = 0; i < HASH_SIZE; i++)
			map->map_name[i] = NULL;
		map->bit_to_name = NULL;
	}
	return map;
}

/*
 * We will only accept alpha numerical labels; else
 * parses might choke on output when label named
 * "foo;<&bar" exists.  ASCII machines only.
 *
 * Avoids libc isalnum() etc. to avoid issues with locale
 * settings.
 */
static bool label_is_sane(const char *label)
{
	for (;*label; label++) {
		if (*label >= 'a' && *label <= 'z')
			continue;
		if (*label >= 'A' && *label <= 'Z')
			continue;
		if (*label >= '0' && *label <= '9')
			continue;
		if (*label == ' ' || *label == '-')
			continue;
		return false;
	}
	return true;
}

const char *__labels_get_path(void)
{
	return CONNLABEL_CFG;
}

struct nfct_labelmap *__labelmap_new(const char *name)
{
	struct nfct_labelmap *map;
	char label[1024];
	char *end;
	FILE *fp;
	int added = 0;
	unsigned int maxbit = 0;
	uint32_t bits_seen[MAX_BITS/32];

	fp = fopen(name ? name : CONNLABEL_CFG, "re");
	if (!fp)
		return NULL;

	memset(bits_seen, 0, sizeof(bits_seen));

	map = map_alloc();
	if (!map) {
		fclose(fp);
		return NULL;
	}

	while (fgets(label, sizeof(label), fp)) {
		int bit;

		if (label[0] == '#')
			continue;

		bit = xtables_parse_connlabel_numerical(label, &end);
		if (bit < 0 || test_bit(bit, bits_seen))
			continue;

		end = trim_label(end);
		if (!end)
			continue;

		if (label_is_sane(end) && map_insert(map, end, bit) == 0) {
			added++;
			if (maxbit < bit)
				maxbit = bit;
			set_bit(bit, bits_seen);
		}
	}

	fclose(fp);

	if (added) {
		map->namecount = maxbit + 1;
		map->bit_to_name = calloc(sizeof(char *), map->namecount);
		if (!map->bit_to_name)
			goto err;
		make_name_table(map);
		return map;
	} else {
		errno = 0;
	}
 err:
	__labelmap_destroy(map);
	return NULL;
}
