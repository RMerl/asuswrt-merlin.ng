#ifndef _CACHE_H_
#define _CACHE_H_

#include <stdint.h>
#include <stddef.h>
#include "hash.h"
#include "date.h"

/* cache features */
enum {
	NO_FEATURES = 0,

	TIMER_FEATURE = 0,
	TIMER = (1 << TIMER_FEATURE),

	__CACHE_MAX_FEATURE
};
#define CACHE_MAX_FEATURE __CACHE_MAX_FEATURE

enum {
	C_OBJ_NONE = 0,		/* not in the cache */
	C_OBJ_NEW,		/* just added to the cache */
	C_OBJ_ALIVE,		/* in the cache, alive */
	C_OBJ_DEAD,		/* still in the cache, but dead */
	C_OBJ_MAX
};

struct cache;
struct cache_object {
	struct	hashtable_node hashnode;
	void	*ptr;
	struct	cache *cache;
	int	status;
	int	refcnt;
	long	lifetime;
	long	lastupdate;
	char	data[0];
};

struct cache_feature {
	size_t size;
	void (*add)(struct cache_object *obj, void *data);
	void (*update)(struct cache_object *obj, void *data);
	void (*destroy)(struct cache_object *obj, void *data);
	int  (*dump)(struct cache_object *obj, void *data, char *buf, int type);
};

extern struct cache_feature timer_feature;

#define CACHE_MAX_NAMELEN 32

enum cache_type {
	CACHE_T_NONE = 0,
	CACHE_T_CT,
	CACHE_T_EXP,
	CACHE_T_MAX
};

struct cache {
	char name[CACHE_MAX_NAMELEN];
	enum cache_type type;
	struct hashtable *h;

	unsigned int num_features;
	struct cache_feature **features;
	unsigned int feature_type[CACHE_MAX_FEATURE];
	unsigned int *feature_offset;
	struct cache_ops *ops;
	struct cache_extra *extra;
	unsigned int extra_offset;
	size_t object_size;

        /* statistics */
	struct {
		uint32_t	active;
	
		uint32_t	add_ok;
		uint32_t	del_ok;
		uint32_t	upd_ok;
		
		uint32_t	add_fail;
		uint32_t	del_fail;
		uint32_t	upd_fail;

		uint32_t	add_fail_enomem;
		uint32_t	add_fail_enospc;
		uint32_t	del_fail_enoent;
		uint32_t	upd_fail_enoent;

		uint32_t	commit_ok;
		uint32_t	commit_fail;

		uint32_t	flush;

		uint32_t	objects;
	} stats;
};

struct cache_extra {
	unsigned int size;

	void (*add)(struct cache_object *obj, void *data);
	void (*update)(struct cache_object *obj, void *data);
	void (*destroy)(struct cache_object *obj, void *data);
};

struct nfct_handle;

/* cache options depends on the object type: conntrack or expectation. */
struct cache_ops {
	/* hashing and comparison of objects. */
	uint32_t (*hash)(const void *data, const struct hashtable *table);
	int (*cmp)(const void *data1, const void *data2);

	/* object allocation, copy and release. */
	void *(*alloc)(void);
	void (*copy)(void *dst, void *src, unsigned int flags);
	void (*free)(void *ptr);

	/* dump and commit. */
	int (*dump_step)(void *data1, void *n);
	int (*commit)(struct cache *c, struct nfct_handle *h, int clientfd);

	/* build network message from object. */
	struct nethdr *(*build_msg)(const struct cache_object *obj, int type);
};

/* templates to configure conntrack caching. */
extern struct cache_ops cache_sync_internal_ct_ops;
extern struct cache_ops cache_sync_external_ct_ops;
extern struct cache_ops cache_stats_ct_ops;
/* templates to configure expectation caching. */
extern struct cache_ops cache_sync_internal_exp_ops;
extern struct cache_ops cache_sync_external_exp_ops;

struct nf_conntrack;

struct cache *cache_create(const char *name, enum cache_type type, unsigned int features, struct cache_extra *extra, struct cache_ops *ops);
void cache_destroy(struct cache *e);

struct cache_object *cache_object_new(struct cache *c, void *ptr);
void cache_object_free(struct cache_object *obj);
void cache_object_get(struct cache_object *obj);
int cache_object_put(struct cache_object *obj);
void cache_object_set_status(struct cache_object *obj, int status);

int cache_add(struct cache *c, struct cache_object *obj, int id);
void cache_update(struct cache *c, struct cache_object *obj, int id, void *ptr);
struct cache_object *cache_update_force(struct cache *c, void *ptr);
void cache_del(struct cache *c, struct cache_object *obj);
struct cache_object *cache_find(struct cache *c, void *ptr, int *pos);
void cache_stats(const struct cache *c, int fd);
void cache_stats_extended(const struct cache *c, int fd);
void *cache_get_extra(struct cache_object *);
void cache_iterate(struct cache *c, void *data, int (*iterate)(void *data1, void *data2));
void cache_iterate_limit(struct cache *c, void *data, uint32_t from, uint32_t steps, int (*iterate)(void *data1, void *data2));

/* iterators */
struct nfct_handle;

struct __dump_container {
	int fd;
	int type;
};

void cache_dump(struct cache *c, int fd, int type);

struct __commit_container {
	struct nfct_handle	*h;
	struct cache		*c;
};

int cache_commit(struct cache *c, struct nfct_handle *h, int clientfd);
void cache_flush(struct cache *c);
void cache_bulk(struct cache *c);

#endif
