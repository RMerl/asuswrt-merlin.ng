#ifndef _SYNC_HOOKS_H_
#define _SYNC_HOOKS_H_

#include <sys/select.h>

struct nethdr;
struct cache_object;
struct fds;

struct sync_mode {
	int internal_cache_flags;
	int external_cache_flags;
	struct cache_extra *internal_cache_extra;
	struct cache_extra *external_cache_extra;

	int  (*init)(void);
	void (*kill)(void);
	int  (*local)(int fd, int type, void *data);
	int  (*recv)(const struct nethdr *net);
	void (*enqueue)(struct cache_object *obj, int type);
	void (*xmit)(void);
};

extern struct sync_mode sync_alarm;
extern struct sync_mode sync_ftfw;
extern struct sync_mode sync_notrack;

#endif
