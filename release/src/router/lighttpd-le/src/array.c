#include "first.h"

#include "array.h"
#include "buffer.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <errno.h>
#include <assert.h>

#define ARRAY_NOT_FOUND ((size_t)(-1))

array *array_init(void) {
	array *a;

	a = calloc(1, sizeof(*a));
	force_assert(a);

	return a;
}

array *array_init_array(array *src) {
	size_t i;
	array *a = array_init();

	if (0 == src->size) return a;

	a->used = src->used;
	a->size = src->size;
	a->unique_ndx = src->unique_ndx;

	a->data = malloc(sizeof(*src->data) * src->size);
	force_assert(NULL != a->data);
	for (i = 0; i < src->size; i++) {
		if (src->data[i]) a->data[i] = src->data[i]->copy(src->data[i]);
		else a->data[i] = NULL;
	}

	a->sorted = malloc(sizeof(*src->sorted) * src->size);
	force_assert(NULL != a->sorted);
	memcpy(a->sorted, src->sorted, sizeof(*src->sorted) * src->size);
	return a;
}

void array_free(array *a) {
	size_t i;
	if (!a) return;

	for (i = 0; i < a->size; i++) {
		if (a->data[i]) a->data[i]->free(a->data[i]);
	}

	if (a->data) free(a->data);
	if (a->sorted) free(a->sorted);

	free(a);
}

void array_reset(array *a) {
	size_t i;
	if (!a) return;

	for (i = 0; i < a->used; i++) {
		a->data[i]->reset(a->data[i]);
	}

	a->used = 0;
	a->unique_ndx = 0;
}

data_unset *array_pop(array *a) {
	data_unset *du;

	force_assert(a->used != 0);

	a->used --;
	du = a->data[a->used];
	force_assert(a->sorted[a->used] == a->used); /* only works on "simple" lists */
	a->data[a->used] = NULL;

	return du;
}

/* returns index of element or ARRAY_NOT_FOUND
 * if rndx != NULL it stores the position in a->sorted[] where the key needs
 * to be inserted
 */
static size_t array_get_index(array *a, const char *key, size_t keylen, size_t *rndx) {
	/* invariant: [lower-1] < key < [upper]
	 * "virtual elements": [-1] = -INFTY, [a->used] = +INFTY
	 * also an invariant: 0 <= lower <= upper <= a->used
	 */
	size_t lower = 0, upper = a->used;
	force_assert(upper <= SSIZE_MAX); /* (lower + upper) can't overflow */

	while (lower != upper) {
		size_t probe = (lower + upper) / 2;
		int cmp = buffer_caseless_compare(key, keylen, CONST_BUF_LEN(a->data[a->sorted[probe]]->key));
		assert(lower < upper); /* from loop invariant (lower <= upper) + (lower != upper) */
		assert((lower <= probe) && (probe < upper)); /* follows from lower < upper */

		if (cmp == 0) {
			/* found */
			if (rndx) *rndx = probe;
			return a->sorted[probe];
		} else if (cmp < 0) {
			/* key < [probe] */
			upper = probe; /* still: lower <= upper */
		} else {
			/* key > [probe] */
			lower = probe + 1; /* still: lower <= upper */
		}
	}

	/* not found: [lower-1] < key < [upper] = [lower] ==> insert at [lower] */
	if (rndx) *rndx = lower;
	return ARRAY_NOT_FOUND;
}

data_unset *array_get_element(array *a, const char *key) {
	size_t ndx;
	force_assert(NULL != key);

	if (ARRAY_NOT_FOUND != (ndx = array_get_index(a, key, strlen(key), NULL))) {
		/* found, return it */
		return a->data[ndx];
	}

	return NULL;
}

data_unset *array_extract_element(array *a, const char *key) {
	size_t ndx, pos;
	force_assert(NULL != key);

	if (ARRAY_NOT_FOUND != (ndx = array_get_index(a, key, strlen(key), &pos))) {
		/* found */
		const size_t last_ndx = a->used - 1;
		data_unset *entry = a->data[ndx];

		/* now we need to swap it with the last element (if it isn't already the last element) */
		if (ndx != last_ndx) {
			/* to swap we also need to modify the index in a->sorted - find pos of last_elem there */
			size_t last_elem_pos;
			/* last element must be present at the expected position */
			force_assert(last_ndx == array_get_index(a, CONST_BUF_LEN(a->data[last_ndx]->key), &last_elem_pos));

			/* move entry from last_ndx to ndx */
			a->data[ndx] = a->data[last_ndx];
			a->data[last_ndx] = NULL;

			/* fix index entry for moved entry */
			a->sorted[last_elem_pos] = ndx;
		} else {
			a->data[ndx] = NULL;
		}

		/* remove entry in a->sorted: move everything after pos one step to the left */
		if (pos != last_ndx) {
			memmove(a->sorted + pos, a->sorted + pos + 1, (last_ndx - pos) * sizeof(*a->sorted));
		}
		a->sorted[last_ndx] = ARRAY_NOT_FOUND;
		--a->used;

		return entry;
	}

	return NULL;
}

data_unset *array_get_unused_element(array *a, data_type_t t) {
	data_unset *ds = NULL;
	unsigned int i;

	for (i = a->used; i < a->size; i++) {
		if (a->data[i] && a->data[i]->type == t) {
			ds = a->data[i];

			/* make empty slot at a->used for next insert */
			a->data[i] = a->data[a->used];
			a->data[a->used] = NULL;

			return ds;
		}
	}

	return NULL;
}

void array_set_key_value(array *hdrs, const char *key, size_t key_len, const char *value, size_t val_len) {
	data_string *ds_dst;

	if (NULL != (ds_dst = (data_string *)array_get_element(hdrs, key))) {
		buffer_copy_string_len(ds_dst->value, value, val_len);
		return;
	}

	if (NULL == (ds_dst = (data_string *)array_get_unused_element(hdrs, TYPE_STRING))) {
		ds_dst = data_string_init();
	}

	buffer_copy_string_len(ds_dst->key, key, key_len);
	buffer_copy_string_len(ds_dst->value, value, val_len);
	array_insert_unique(hdrs, (data_unset *)ds_dst);
}

/* if entry already exists return pointer to existing entry, otherwise insert entry and return NULL */
static data_unset **array_find_or_insert(array *a, data_unset *entry) {
	size_t ndx, pos, j;

	/* generate unique index if neccesary */
	if (buffer_is_empty(entry->key) || entry->is_index_key) {
		buffer_copy_int(entry->key, a->unique_ndx++);
		entry->is_index_key = 1;
		force_assert(0 != a->unique_ndx); /* must not wrap or we'll get problems */
	}

	/* try to find the entry */
	if (ARRAY_NOT_FOUND != (ndx = array_get_index(a, CONST_BUF_LEN(entry->key), &pos))) {
		/* found collision, return it */
		return &a->data[ndx];
	}

	/* insert */

	/* there couldn't possibly be enough memory to store so many entries */
	force_assert(a->used + 1 <= SSIZE_MAX);

	if (a->size == 0) {
		a->size   = 16;
		a->data   = malloc(sizeof(*a->data)     * a->size);
		a->sorted = malloc(sizeof(*a->sorted)   * a->size);
		force_assert(a->data);
		force_assert(a->sorted);
		for (j = a->used; j < a->size; j++) a->data[j] = NULL;
	} else if (a->size == a->used) {
		a->size  += 16;
		a->data   = realloc(a->data,   sizeof(*a->data)   * a->size);
		a->sorted = realloc(a->sorted, sizeof(*a->sorted) * a->size);
		force_assert(a->data);
		force_assert(a->sorted);
		for (j = a->used; j < a->size; j++) a->data[j] = NULL;
	}

	ndx = a->used;

	/* make sure there is nothing here */
	if (a->data[ndx]) a->data[ndx]->free(a->data[ndx]);

	a->data[a->used++] = entry;

	/* move everything one step to the right */
	if (pos != ndx) {
		memmove(a->sorted + (pos + 1), a->sorted + (pos), (ndx - pos) * sizeof(*a->sorted));
	}

	/* insert */
	a->sorted[pos] = ndx;

	return NULL;
}

/* replace or insert data (free existing entry) */
void array_replace(array *a, data_unset *entry) {
	data_unset **old;

	force_assert(NULL != entry);
	if (NULL != (old = array_find_or_insert(a, entry))) {
		force_assert(*old != entry);
		(*old)->free(*old);
		*old = entry;
	}
}

void array_insert_unique(array *a, data_unset *entry) {
	data_unset **old;

	force_assert(NULL != entry);
	if (NULL != (old = array_find_or_insert(a, entry))) {
		force_assert((*old)->type == entry->type);
		entry->insert_dup(*old, entry);
	}
}

void array_print_indent(int depth) {
	int i;
	for (i = 0; i < depth; i ++) {
		fprintf(stdout, "    ");
	}
}

size_t array_get_max_key_length(array *a) {
	size_t maxlen, i;

	maxlen = 0;
	for (i = 0; i < a->used; i ++) {
		data_unset *du = a->data[i];
		size_t len = strlen(du->key->ptr);

		if (len > maxlen) {
			maxlen = len;
		}
	}
	return maxlen;
}

int array_print(array *a, int depth) {
	size_t i;
	size_t maxlen;
	int oneline = 1;

	if (a->used > 5) {
		oneline = 0;
	}
	for (i = 0; i < a->used && oneline; i++) {
		data_unset *du = a->data[i];
		if (!du->is_index_key) {
			oneline = 0;
			break;
		}
		switch (du->type) {
			case TYPE_INTEGER:
			case TYPE_STRING:
				break;
			default:
				oneline = 0;
				break;
		}
	}
	if (oneline) {
		fprintf(stdout, "(");
		for (i = 0; i < a->used; i++) {
			data_unset *du = a->data[i];
			if (i != 0) {
				fprintf(stdout, ", ");
			}
			du->print(du, depth + 1);
		}
		fprintf(stdout, ")");
		return 0;
	}

	maxlen = array_get_max_key_length(a);
	fprintf(stdout, "(\n");
	for (i = 0; i < a->used; i++) {
		data_unset *du = a->data[i];
		array_print_indent(depth + 1);
		if (!du->is_index_key) {
			int j;

			if (i && (i % 5) == 0) {
				fprintf(stdout, "# %zu\n", i);
				array_print_indent(depth + 1);
			}
			fprintf(stdout, "\"%s\"", du->key->ptr);
			for (j = maxlen - strlen(du->key->ptr); j > 0; j --) {
				fprintf(stdout, " ");
			}
			fprintf(stdout, " => ");
		}
		du->print(du, depth + 1);
		fprintf(stdout, ",\n");
	}
	if (!(i && (i - 1 % 5) == 0)) {
		array_print_indent(depth + 1);
		fprintf(stdout, "# %zu\n", i);
	}
	array_print_indent(depth);
	fprintf(stdout, ")");

	return 0;
}

#ifdef DEBUG_ARRAY
int main (int argc, char **argv) {
	array *a;
	data_string *ds;

	UNUSED(argc);
	UNUSED(argv);

	a = array_init();

	ds = data_string_init();
	buffer_copy_string_len(ds->key, CONST_STR_LEN("abc"));
	buffer_copy_string_len(ds->value, CONST_STR_LEN("alfrag"));

	array_insert_unique(a, (data_unset *)ds);

	ds = data_string_init();
	buffer_copy_string_len(ds->key, CONST_STR_LEN("abc"));
	buffer_copy_string_len(ds->value, CONST_STR_LEN("hameplman"));

	array_insert_unique(a, (data_unset *)ds);

	ds = data_string_init();
	buffer_copy_string_len(ds->key, CONST_STR_LEN("123"));
	buffer_copy_string_len(ds->value, CONST_STR_LEN("alfrag"));

	array_insert_unique(a, (data_unset *)ds);

	array_print(a, 0);

	array_free(a);

	fprintf(stderr, "%d\n",
	       buffer_caseless_compare(CONST_STR_LEN("Content-Type"), CONST_STR_LEN("Content-type")));

	return 0;
}
#endif
