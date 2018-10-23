#ifndef _NF_SET_HASH_H_
#define _NF_SET_HASH_H_

#include <unistd.h>
#include "linux_list.h"

#include <stdint.h>

struct hashtable;
struct hashtable_node;

struct hashtable {
	uint32_t hashsize;
	uint32_t limit;
	uint32_t count;
	uint32_t initval;
	
	uint32_t (*hash)(const void *data, const struct hashtable *table);
	int	 (*compare)(const void *data1, const void *data2);

	struct list_head 	members[0];
};

struct hashtable_node {
	struct list_head head;
};

struct hashtable *
hashtable_create(int hashsize, int limit,
		 uint32_t (*hash)(const void *data,
		 		  const struct hashtable *table),
		 int (*compare)(const void *data1, const void *data2));
void hashtable_destroy(struct hashtable *h);
int hashtable_hash(const struct hashtable *table, const void *data);
struct hashtable_node *hashtable_find(const struct hashtable *table, const void *data, int id);
int hashtable_add(struct hashtable *table, struct hashtable_node *n, int id);
void hashtable_del(struct hashtable *table, struct hashtable_node *node);
int hashtable_flush(struct hashtable *table);
int hashtable_iterate(struct hashtable *table, void *data,
		      int (*iterate)(void *data, void *n));
int hashtable_iterate_limit(struct hashtable *table, void *data, uint32_t from, uint32_t steps, int (*iterate)(void *data1, void *n));
unsigned int hashtable_counter(const struct hashtable *table);

#endif
