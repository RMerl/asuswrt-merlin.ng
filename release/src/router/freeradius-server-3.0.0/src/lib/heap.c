RCSID("$Id$")

#include <freeradius-devel/libradius.h>
#include <freeradius-devel/heap.h>

/*
 *	A heap entry is made of a pointer to the object, which
 *	contains the key.  The heap itself is an array of pointers.
 *
 *	Heaps normally support only ordered insert, and extraction
 *	of the minimum element.  The heap entry can contain an "int"
 *	field that holds the entries position in the heap.  The offset
 *	of the field is held inside of the heap structure.
 */

struct fr_heap_t {
	int size;
	int num_elements;
	size_t offset;
	fr_heap_cmp_t cmp;
	void **p;
};

/*
 *	First node in a heap is element 0. Children of i are 2i+1 and
 *	2i+2.  These macros wrap the logic, so the code is more
 *	descriptive.
 */
#define HEAP_PARENT(x) ( ( (x) - 1 ) / 2 )
#define HEAP_LEFT(x) ( 2*(x) + 1 )
#define HEAP_RIGHT(x) ( 2*(x) + 2 )
#define	HEAP_SWAP(a, b) { void *_tmp = a; a = b; b = _tmp; }

static int fr_heap_bubble(fr_heap_t *hp, int child);

void fr_heap_delete(fr_heap_t *hp)
{
	if (!hp) return;

	free(hp->p);
	free(hp);
}

fr_heap_t *fr_heap_create(fr_heap_cmp_t cmp, size_t offset)
{
	fr_heap_t *fh;

	if (!cmp) return NULL;

	fh = malloc(sizeof(*fh));
	if (!fh) return NULL;

	memset(fh, 0, sizeof(*fh));

	fh->size = 2048;
	fh->p = malloc(sizeof(*(fh->p)) * fh->size);
	if (!fh->p) {
		free(fh);
		return NULL;
	}

	fh->cmp = cmp;
	fh->offset = offset;

	return fh;
}

/*
 *	Insert element in heap. Normally, p != NULL, we insert p in a
 *	new position and bubble up. If p == NULL, then the element is
 *	already in place, and key is the position where to start the
 *	bubble-up.
 *
 *	Returns 1 on failure (cannot allocate new heap entry)
 *
 *	If offset > 0 the position (index, int) of the element in the
 *	heap is also stored in the element itself at the given offset
 *	in bytes.
 */
#define SET_OFFSET(heap, node) \
    if (heap->offset) \
	    *((int *)(((uint8_t *)heap->p[node]) + heap->offset)) = node

/*
 *	RESET_OFFSET is used for sanity checks. It sets offset to an
 *	invalid value.
 */
#define RESET_OFFSET(heap, node) \
    if (heap->offset) \
	    *((int *)(((uint8_t *)heap->p[node]) + heap->offset)) = -1

int fr_heap_insert(fr_heap_t *hp, void *data)
{
	int child = hp->num_elements;

	/*
	 *	Heap is full.  Double it's size.
	 */
	if (child == hp->size) {
		void **p;

		p = malloc(2 * hp->size * sizeof(*p));
		if (!p) return 0;

		memcpy(p, hp->p, sizeof(*p) * hp->size);
		free(hp->p);
		hp->p = p;
		hp->size *= 2;
	}

	hp->p[child] = data;
	hp->num_elements++;

	return fr_heap_bubble(hp, child);
}


static int fr_heap_bubble(fr_heap_t *hp, int child)
{
	/*
	 *	Bubble up the element.
	 */
	while (child > 0) {
		int parent = HEAP_PARENT(child);

		/*
		 *	Parent is smaller than the child.  We're done.
		 */
		if (hp->cmp(hp->p[parent], hp->p[child]) < 0) break;

		/*
		 *	Child is smaller than the parent, repeat.
		 */
		HEAP_SWAP(hp->p[child], hp->p[parent]);
		SET_OFFSET(hp, child);
		child = parent;
	}
	SET_OFFSET(hp, child);

	return 1;
}


/*
 *	Remove the top element, or object.
 */
int fr_heap_extract(fr_heap_t *hp, void *data)
{
	int child, parent;
	int max;

	if (!hp || (hp->num_elements == 0)) return 0;

	max = hp->num_elements - 1;

	/*
	 *	Extract element.  Default is the first one.
	 */
	if (!data) {
		parent = 0;

	} else {		/* extract from the middle */
		if (!hp->offset) return 0;

		parent = *((int *)(((uint8_t *)data) + hp->offset));

		/*
		 *	Out of bounds.
		 */
		if (parent < 0 || parent >= hp->num_elements) return 0;
	}

	RESET_OFFSET(hp, parent);
	child = HEAP_LEFT(parent);
	while (child <= max) {
		/*
		 *	Maybe take the right child.
		 */
		if ((child != max) &&
		    (hp->cmp(hp->p[child + 1], hp->p[child]) < 0)) {
			child = child + 1;
		}
		hp->p[parent] = hp->p[child];
		SET_OFFSET(hp, parent);
		parent = child;
		child = HEAP_LEFT(child);
	}
	hp->num_elements--;

	/*
	 *	We didn't end up at the last element in the heap.
	 *	This element has to be re-inserted.
	 */
	if (parent != max) {
		/*
		 *	Fill hole with last entry and bubble up,
		 *	reusing the insert code
		 */
		hp->p[parent] = hp->p[max];
		return fr_heap_bubble(hp, parent);
	}

	return 1;
}


void *fr_heap_peek(fr_heap_t *hp)
{
	if (!hp || (hp->num_elements == 0)) return NULL;

	/*
	 *	If this is NULL, we have a problem.
	 */
	return hp->p[0];
}

int fr_heap_num_elements(fr_heap_t *hp)
{
	if (!hp) return 0;

	return hp->num_elements;
}


#ifdef TESTING
/*
 *  cc -g -DTESTING -I .. heap.c -o heap
 *
 *  ./heap
 */
static int heap_cmp(void const *a, void const *b)
{
	return *(int *)a - *(int *) b;

}


int main(int argc, char **arg)
{
	fr_heap_t *hp;
	int i, array[1024];

	hp = fr_heap_create(heap_cmp, 0);
	if (!hp) {
		fprintf(stderr, "Failed creating heap!\n");
		fr_exit(1);
	}

	for (i = 0; i < 1024; i++) {
		array[i] = (i * 257) % 65537;
		if (!fr_heap_insert(hp, &array[i])) {
			fprintf(stderr, "Failed inserting %d\n", i);
			fr_exit(1);
		}
	}

	for (i = 0; i < 1024; i++) {
		int *p = fr_heap_peek(hp);

		if (!p) {
			fprintf(stderr, "Failed peeking %d\n", i);
			fr_exit(1);
		}

		printf("%d\t%d\n", i, *p);

		if (!fr_heap_extract(hp, NULL)) {
			fprintf(stderr, "Failed extracting %d\n", i);
			fr_exit(1);
		}
	}

	fr_heap_delete(hp);

	return 0;
}
#endif
