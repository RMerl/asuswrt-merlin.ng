#ifndef _COMPAT_XTNU_H
#define _COMPAT_XTNU_H 1

#include <linux/netfilter/x_tables.h>

struct module;
struct sk_buff;

struct xtnu_match {
	/*
	 * Making it smaller by sizeof(void *) on purpose to catch
	 * lossy translation, if any.
	 */
	char name[sizeof(((struct xt_match *)NULL)->name) - 1 - sizeof(void *)];
	uint8_t revision;
	bool (*match)(const struct sk_buff *, struct xt_action_param *);
	int (*checkentry)(const struct xt_mtchk_param *);
	void (*destroy)(const struct xt_mtdtor_param *);
	struct module *me;
	const char *table;
	unsigned int matchsize, hooks;
	unsigned short proto, family;

	void *__compat_match;
};

struct xtnu_target {
	char name[sizeof(((struct xt_target *)NULL)->name) - 1 - sizeof(void *)];
	uint8_t revision;
	unsigned int (*target)(struct sk_buff **,
		const struct xt_action_param *);
	int (*checkentry)(const struct xt_tgchk_param *);
	void (*destroy)(const struct xt_tgdtor_param *);
	struct module *me;
	const char *table;
	unsigned int targetsize, hooks;
	unsigned short proto, family;

	void *__compat_target;
};

static inline struct xtnu_match *xtcompat_numatch(const struct xt_match *m)
{
	void *q;
	memcpy(&q, m->name + sizeof(m->name) - sizeof(void *), sizeof(void *));
	return q;
}

static inline struct xtnu_target *xtcompat_nutarget(const struct xt_target *t)
{
	void *q;
	memcpy(&q, t->name + sizeof(t->name) - sizeof(void *), sizeof(void *));
	return q;
}

extern int xtnu_register_match(struct xtnu_match *);
extern void xtnu_unregister_match(struct xtnu_match *);
extern int xtnu_register_matches(struct xtnu_match *, unsigned int);
extern void xtnu_unregister_matches(struct xtnu_match *, unsigned int);
extern int xtnu_register_target(struct xtnu_target *);
extern void xtnu_unregister_target(struct xtnu_target *);
extern int xtnu_register_targets(struct xtnu_target *, unsigned int);
extern void xtnu_unregister_targets(struct xtnu_target *, unsigned int);

extern void *HX_memmem(const void *, size_t, const void *, size_t);

#endif /* _COMPAT_XTNU_H */
