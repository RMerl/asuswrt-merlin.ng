#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdint.h>
#include "linux_list.h"

struct queue_node {
	struct list_head	head;
	uint32_t		type;
	struct queue		*owner;
	size_t 			size;
};

enum {
	Q_ELEM_OBJ = 0,
	Q_ELEM_CTL = 1,
	Q_ELEM_ERR = 2,
};

void queue_node_init(struct queue_node *n, int type);
void *queue_node_data(struct queue_node *n);

struct queue_object {
	struct queue_node	qnode;
	char			data[0];
};

struct queue_object *queue_object_new(int type, size_t size);
void queue_object_free(struct queue_object *obj);

struct evfd;

#define QUEUE_NAMELEN	16

struct queue {
	struct list_head	list;
	unsigned int		max_elems;
	unsigned int		num_elems;
	uint32_t		enospc_err;
	uint32_t		flags;
	struct list_head	head;
	struct evfd		*evfd;
	char			name[QUEUE_NAMELEN];
};

#define QUEUE_F_EVFD (1U << 0)

struct queue *queue_create(const char *name,
			   int max_objects, unsigned int flags);
void queue_destroy(struct queue *b);
void queue_stats_show(int fd);
unsigned int queue_len(const struct queue *b);
int queue_add(struct queue *b, struct queue_node *n);
int queue_del(struct queue_node *n);
struct queue_node *queue_del_head(struct queue *b);
int queue_in(struct queue *b, struct queue_node *n);
void queue_iterate(struct queue *b,
		   const void *data,
		   int (*iterate)(struct queue_node *n, const void *data2));
int queue_get_eventfd(struct queue *b);

#endif
