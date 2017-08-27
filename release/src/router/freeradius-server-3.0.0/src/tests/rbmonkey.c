#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>

#include <freeradius-devel/libradius.h>

/*
 *	We need knowlege of the internal structures.
 *	This needs to be kept in lockstep with rbtree.c
 */

/* red-black tree description */
typedef enum { Black, Red } NodeColor;

struct rbnode_t {
	rbnode_t    *Left;          /* left child */
	rbnode_t    *Right;         /* right child */
	rbnode_t    *Parent;        /* parent */
	NodeColor   Color;          /* node color (black, red) */
	void        *Data;          /* data stored in node */
};

struct rbtree_t {
#ifndef NDEBUG
	uint32_t magic;
#endif
	rbnode_t *Root;
	int     num_elements;
	int (*Compare)(void const *, void const *);
	int replace_flag;
	void (*freeNode)(void *);
#ifdef HAVE_PTHREAD_H
	int lock;
	pthread_mutex_t mutex;
#endif
};

/* Storage for the NIL pointer. */
rbnode_t *NIL;

static int comp(void const *a, void const *b)
{
	if (*(int const *)a > *(int const *)b) {
		return -1;
	}

	if (*(int const *)a < *(int const *)b) {
		return 1;
	}
	return 0;
}

#if 0
static int print_cb(UNUSED void *ctx, void *i)
{
	fprintf(stderr, "%i\n", *(int*)i);
	return 0;
}
#endif

#define MAXSIZE 1024

static int r = 0;
static int rvals[MAXSIZE];

static int store_cb(UNUSED void *ctx, void *i)
{
	rvals[r++] = *(int *)i;
	return 0;
}

static int mask;

static int filter_cb(UNUSED void *ctx, void *i)
{
	if ((*(int *)i & mask) == (*(int *)ctx & mask)) {
		return 2;
	}
	return 0;
}

/*
 * Returns the count of black nodes from root to child leaves, or a
 * negative number indicating which red-black rule was broken.
 */
static int rbcount(rbtree_t *t)
{
	rbnode_t *n;
	int count, count_expect;

	count_expect = -1;
	n = t->Root;
	if (!n || n == NIL) {
		return 0;
	}
	if (n->Color != Black) {
		return -2; /* Root not Black */
	}
	count = 0;
descend:
	while (n->Left != NIL) {
		if (n->Color == Red) {
			if (n->Left->Color != Black || n->Right->Color != Black) {
				return -4; /* Children of Red nodes must be Black */
			}
		}
		else {
			count++;
		}
		n = n->Left;
	}
	if (n->Right != NIL) {
		if (n->Color == Red) {
			if (n->Left->Color != Black || n->Right->Color != Black) {
				return -4; /* Children of Red nodes must be Black */
			}
		}
		else {
			count++;
		}
		n = n->Right;
	}
	if (n->Left != NIL || n->Right != NIL) {
		goto descend;
	}
	if (count_expect < 0) {
		count_expect = count + (n->Color == Black);
	}
	else {
		if (count_expect != count + (n->Color == Black)) {
			fprintf(stderr,"Expected %i got %i\n", count_expect, count);
			return -5; /* All paths must traverse the same number of Black nodes. */
		}
	}
ascend:
	if (!n->Parent) return count_expect;
	while (n->Parent->Right == n) {
		n = n->Parent;
		if (!n->Parent) return count_expect;
		if (n->Color == Black) {
			count--;
		}
	}
	if (n->Parent->Left == n) {
		if (n->Parent->Right != NIL) {
			n = n->Parent->Right;
			goto descend;
		}
		n = n->Parent;
		if (!n->Parent) return count_expect;
		if (n->Color == Black) {
			count--;
		}
	}
	goto ascend;
}

#define REPS 10

int main(UNUSED int argc, UNUSED char *argv[])
{
	rbtree_t *t;
	int i, j, thresh;
	int n, nextseed, rep;
	int vals[MAXSIZE];
	struct timeval now;
	gettimeofday(&now, NULL);

	/* TODO: make starting seed and repetitions a CLI option */
	nextseed = now.tv_usec;
	rep = REPS;

again:
	if (!--rep) return 0;

	srand(nextseed);
	thresh = rand();
	mask = 0xff >> (rand() & 7);
	thresh &= mask;
	n = (rand() % MAXSIZE) + 1;
	while (n < 0 || n > MAXSIZE) n >>= 1;
	fprintf(stderr, "seed = %i filter = %x mask = %x n= %i\n",
		nextseed, thresh, mask, n);
	nextseed = rand();

	t = rbtree_create(comp, free, RBTREE_FLAG_LOCK);
	/* Find out the value of the NIL node */
	NIL = t->Root->Left;

	for (i = 0; i < n; i++) {
		int *p;
		p = malloc(sizeof(int));
		*p = rand();
		vals[i] = *p;
		rbtree_insert(t, p);
	}

	i = rbcount(t);
	fprintf(stderr,"After insert rbcount is %i.\n", i);
	if (i < 0) { return i; }

	qsort(vals, n, sizeof(int), comp);

	/*
	 * For testing deletebydata instead

	 for (i = 0; i < n; i++) {
	 if (filter_cb(&vals[i], &thresh) == 2) {
	 rbtree_deletebydata(t, &vals[i]);
	 }
	 }

	 *
	 */
	rbtree_walk(t, DeleteOrder, filter_cb, &thresh);
	i = rbcount(t);
	fprintf(stderr,"After delete rbcount is %i.\n", i);
	if (i < 0) { return i; }

	r = 0;
	rbtree_walk(t, InOrder, &store_cb, NULL);

	for (j = i = 0; i < n; i++) {
		if (i && vals[i-1] == vals[i]) continue;
		if (!filter_cb(&thresh, &vals[i])) {
			if (vals[i] != rvals[j]) goto bad;
			j++;
		}
	}
	fprintf(stderr,"matched OK\n");
	rbtree_free(t);
	goto again;

bad:
	for (j = i = 0; i < n; i++) {
		if (i && vals[i-1] == vals[i]) continue;
		if (!filter_cb(&thresh, &vals[i])) {
			fprintf(stderr, "%i: %x %x\n", j, vals[i], rvals[j]);
			j++;
		} else {
			fprintf(stderr, "skipped %x\n", vals[i]);
		}
	}
	return -1;
}

