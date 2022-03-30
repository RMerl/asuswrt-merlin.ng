struct list_head {
	struct list_head *next;
};

#define LIST_HEAD(name) \
	struct list_head name = { &(name) }

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	new->next = head->next;
	head->next = new;
}

static inline void list_del(struct list_head *entry, struct list_head *prev)
{
	prev->next = entry->next;
	entry->next = entry;
}

#define list_for_each_safe(pos, n, head) \
	for (n = (head), pos = (head)->next; pos != (head); \
	     n = pos, pos = n->next)

#undef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#ifdef DEBUG
#define pynl_dbg(fmt, ...) \
	fprintf(stderr, "%s: " fmt, __func__, __VA_ARGS__)
#else
#define pynl_dbg(fmt, ...)
#endif
