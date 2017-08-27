#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "test.h"

static int configs_equal(snd_config_t *c1, snd_config_t *c2);

/* checks if all children of c1 also occur in c2 */
static int subset_of(snd_config_t *c1, snd_config_t *c2)
{
	snd_config_iterator_t i, next;
	snd_config_t *e1, *e2;
	const char *id;

	snd_config_for_each(i, next, c1) {
		e1 = snd_config_iterator_entry(i);
		if (snd_config_get_id(e1, &id) < 0 || !id)
			return 0;
		if (snd_config_search(c2, id, &e2) < 0)
			return 0;
		if (!configs_equal(e1, e2))
			return 0;
	}
	return 1;
}

/* checks if two configuration nodes are equal */
static int configs_equal(snd_config_t *c1, snd_config_t *c2)
{
	long i1, i2;
	long long i641, i642;
	const char *s1, *s2;

	if (snd_config_get_type(c1) != snd_config_get_type(c2))
		return 0;
	switch (snd_config_get_type(c1)) {
	case SND_CONFIG_TYPE_INTEGER:
		return snd_config_get_integer(c1, &i1) >= 0 &&
			snd_config_get_integer(c2, &i2) >= 0 &&
			i1 == i2;
	case SND_CONFIG_TYPE_INTEGER64:
		return snd_config_get_integer64(c1, &i641) >= 0 &&
			snd_config_get_integer64(c2, &i642) >= 0 &&
			i641 == i642;
	case SND_CONFIG_TYPE_STRING:
		return snd_config_get_string(c1, &s1) >= 0 &&
			snd_config_get_string(c2, &s2) >= 0 &&
			!s1 == !s2 &&
			(!s1 || !strcmp(s1, s2));
	case SND_CONFIG_TYPE_COMPOUND:
		return subset_of(c1, c2) && subset_of(c2, c1);
	default:
		fprintf(stderr, "unknown configuration node type %d\n",
			(int)snd_config_get_type(c1));
		return 0;
	}
}

static void test_top(void)
{
	snd_config_t *top;
	const char *id;

	if (ALSA_CHECK(snd_config_top(&top)) < 0)
		return;

	TEST_CHECK(snd_config_get_type(top) == SND_CONFIG_TYPE_COMPOUND);
	TEST_CHECK(snd_config_iterator_first(top) == snd_config_iterator_end(top));
	TEST_CHECK(snd_config_get_id(top, &id) >= 0 && id == NULL);

	ALSA_CHECK(snd_config_delete(top));
}

static void test_load(void)
{
	const char *config_text1 = "s='world';";
	const char *config_text2 = "c.elem 0";
	snd_config_t *loaded, *made, *c, *c2;
	snd_input_t *input;

	ALSA_CHECK(snd_config_top(&loaded));
	ALSA_CHECK(snd_config_imake_integer(&c, "i", 42));
	ALSA_CHECK(snd_config_add(loaded, c));
	ALSA_CHECK(snd_config_imake_string(&c, "s", "hello"));
	ALSA_CHECK(snd_config_add(loaded, c));

	ALSA_CHECK(snd_config_top(&made));
	ALSA_CHECK(snd_config_imake_string(&c, "s", "world"));
	ALSA_CHECK(snd_config_add(made, c));
	ALSA_CHECK(snd_config_imake_integer(&c, "i", 42));
	ALSA_CHECK(snd_config_add(made, c));

	ALSA_CHECK(snd_input_buffer_open(&input, config_text1, strlen(config_text1)));
	ALSA_CHECK(snd_config_load(loaded, input));
	ALSA_CHECK(snd_input_close(input));
	TEST_CHECK(configs_equal(loaded, made));

	ALSA_CHECK(snd_config_make_compound(&c, "c", 0));
	ALSA_CHECK(snd_config_add(made, c));
	ALSA_CHECK(snd_config_imake_integer(&c2, "elem", 0));
	ALSA_CHECK(snd_config_add(c, c2));

	ALSA_CHECK(snd_input_buffer_open(&input, config_text2, strlen(config_text2)));
	ALSA_CHECK(snd_config_load(loaded, input));
	ALSA_CHECK(snd_input_close(input));
	TEST_CHECK(configs_equal(loaded, made));

	ALSA_CHECK(snd_config_delete(loaded));
	ALSA_CHECK(snd_config_delete(made));
}

static void test_save(void)
{
	const char *text =
		"a.b.c 'x.y.z'\n"
		"xxx = yyy;\n"
		"q { qq=qqq }\n"
		"a [ 1 2 3 4 5 '...' ]\n";
	snd_config_t *orig, *saved;
	snd_input_t *input;
	snd_output_t *output;
	char *buf;
	size_t buf_size;

	ALSA_CHECK(snd_input_buffer_open(&input, text, strlen(text)));
	ALSA_CHECK(snd_config_top(&orig));
	ALSA_CHECK(snd_config_load(orig, input));
	ALSA_CHECK(snd_input_close(input));
	ALSA_CHECK(snd_output_buffer_open(&output));
	ALSA_CHECK(snd_config_save(orig, output));
	buf_size = snd_output_buffer_string(output, &buf);
	ALSA_CHECK(snd_input_buffer_open(&input, buf, buf_size));
	ALSA_CHECK(snd_config_top(&saved));
	ALSA_CHECK(snd_config_load(saved, input));
	ALSA_CHECK(snd_input_close(input));
	ALSA_CHECK(snd_output_close(output));
	TEST_CHECK(configs_equal(orig, saved));
	ALSA_CHECK(snd_config_delete(orig));
	ALSA_CHECK(snd_config_delete(saved));
}

static void test_update(void)
{
	ALSA_CHECK(snd_config_update_free_global());
	TEST_CHECK(snd_config == NULL);
	ALSA_CHECK(snd_config_update());
	TEST_CHECK(snd_config_get_type(snd_config) == SND_CONFIG_TYPE_COMPOUND);
	ALSA_CHECK(snd_config_update());
	TEST_CHECK(snd_config_get_type(snd_config) == SND_CONFIG_TYPE_COMPOUND);
	ALSA_CHECK(snd_config_update_free_global());
	TEST_CHECK(snd_config == NULL);
}

static void test_search(void)
{
	const char *text =
		"a 42\n"
		"b {\n"
		"    c cee\n"
		"    d {\n"
		"        e 2.71828\n"
		"    }\n"
		"}\n";
	snd_input_t *input;
	snd_config_t *top, *c;
	const char *id;

	ALSA_CHECK(snd_input_buffer_open(&input, text, strlen(text)));
	ALSA_CHECK(snd_config_top(&top));
	ALSA_CHECK(snd_config_load(top, input));
	ALSA_CHECK(snd_input_close(input));

	ALSA_CHECK(snd_config_search(top, "a", &c));
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "a"));
	ALSA_CHECK(snd_config_search(top, "b.d.e", &c));
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "e"));
	ALSA_CHECK(snd_config_search(top, "b.c", NULL));
	TEST_CHECK(snd_config_search(top, "x", NULL) == -ENOENT);
	TEST_CHECK(snd_config_search(top, "b.y", &c) == -ENOENT);
	TEST_CHECK(snd_config_search(top, "a.z", &c) == -ENOENT);

	ALSA_CHECK(snd_config_delete(top));
}

static void test_searchv(void)
{
	const char *text =
		"a 42\n"
		"b {\n"
		"    c cee\n"
		"    d {\n"
		"        e 2.71828\n"
		"    }\n"
		"}\n";
	snd_input_t *input;
	snd_config_t *top, *c;
	const char *id;

	ALSA_CHECK(snd_input_buffer_open(&input, text, strlen(text)));
	ALSA_CHECK(snd_config_top(&top));
	ALSA_CHECK(snd_config_load(top, input));
	ALSA_CHECK(snd_input_close(input));

	ALSA_CHECK(snd_config_searchv(top, &c, "a", NULL));
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "a"));
	ALSA_CHECK(snd_config_searchv(top, &c, "b", "d.e", NULL));
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "e"));
	ALSA_CHECK(snd_config_searchv(top, NULL, "b.c", NULL));
	TEST_CHECK(snd_config_searchv(top, NULL, "x", NULL) == -ENOENT);
	TEST_CHECK(snd_config_searchv(top, &c, "b.y", NULL) == -ENOENT);
	TEST_CHECK(snd_config_searchv(top, &c, "a", "z", NULL) == -ENOENT);

	ALSA_CHECK(snd_config_delete(top));
}

static void test_add(void)
{
	snd_config_t *c1, *c2, *c3, *c4, *c5;
	snd_config_iterator_t i;
	unsigned int count = 0;

	ALSA_CHECK(snd_config_top(&c1));
	ALSA_CHECK(snd_config_imake_integer(&c2, "c2", 0xc2));
	ALSA_CHECK(snd_config_add(c1, c2));
	ALSA_CHECK(snd_config_imake_string(&c3, "c3", "c3"));
	ALSA_CHECK(snd_config_add(c1, c3));
	for (i = snd_config_iterator_first(c1);
	     i != snd_config_iterator_end(c1);
	     i = snd_config_iterator_next(i))
		++count;
	TEST_CHECK(count == 2);
	ALSA_CHECK(snd_config_search(c1, "c2", &c2));
	ALSA_CHECK(snd_config_search(c1, "c3", &c3));
	ALSA_CHECK(snd_config_top(&c4));
	TEST_CHECK(snd_config_add(c1, c4) == -EINVAL);
	ALSA_CHECK(snd_config_imake_integer(&c5, "c5", 5));
	ALSA_CHECK(snd_config_add(c4, c5));
	TEST_CHECK(snd_config_add(c1, c5) == -EINVAL);
	ALSA_CHECK(snd_config_delete(c4));
	ALSA_CHECK(snd_config_imake_integer(&c3, "c3", 333));
	TEST_CHECK(snd_config_add(c1, c3) == -EEXIST);
	ALSA_CHECK(snd_config_delete(c3));
	ALSA_CHECK(snd_config_delete(c1));
}

static void test_delete(void)
{
	snd_config_t *c;

	ALSA_CHECK(snd_config_top(&c));
	ALSA_CHECK(snd_config_delete(c));
	ALSA_CHECK(snd_config_imake_string(&c, "s", "..."));
	ALSA_CHECK(snd_config_delete(c));
}

static void test_copy(void)
{
	snd_config_t *c1, *c2, *c3;
	long value;

	ALSA_CHECK(snd_config_imake_integer(&c1, "c1", 123));
	ALSA_CHECK(snd_config_copy(&c2, c1));
	ALSA_CHECK(snd_config_set_integer(c1, 456));
	TEST_CHECK(snd_config_get_type(c2) == SND_CONFIG_TYPE_INTEGER);
	ALSA_CHECK(snd_config_get_integer(c2, &value));
	TEST_CHECK(value == 123);
	ALSA_CHECK(snd_config_delete(c1));
	ALSA_CHECK(snd_config_delete(c2));
	ALSA_CHECK(snd_config_top(&c1));
	ALSA_CHECK(snd_config_imake_integer(&c2, "a", 1));
	ALSA_CHECK(snd_config_add(c1, c2));
	ALSA_CHECK(snd_config_copy(&c3, c1));
	ALSA_CHECK(snd_config_set_integer(c2, 2));
	TEST_CHECK(!configs_equal(c1, c3));
	ALSA_CHECK(snd_config_search(c3, "a", &c2));
	ALSA_CHECK(snd_config_set_integer(c2, 2));
	TEST_CHECK(configs_equal(c1, c3));
	ALSA_CHECK(snd_config_delete(c1));
	ALSA_CHECK(snd_config_delete(c3));
}

static void test_make_integer(void)
{
	snd_config_t *c;
	const char *id;
	long value;

	ALSA_CHECK(snd_config_make_integer(&c, "i"));
	TEST_CHECK(snd_config_get_type(c) == SND_CONFIG_TYPE_INTEGER);
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "i"));
	ALSA_CHECK(snd_config_get_integer(c, &value));
	TEST_CHECK(value == 0);
	ALSA_CHECK(snd_config_delete(c));
}

static void test_make_integer64(void)
{
	snd_config_t *c;
	const char *id;
	long long value;

	ALSA_CHECK(snd_config_make_integer64(&c, "i"));
	TEST_CHECK(snd_config_get_type(c) == SND_CONFIG_TYPE_INTEGER64);
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "i"));
	ALSA_CHECK(snd_config_get_integer64(c, &value));
	TEST_CHECK(value == 0);
	ALSA_CHECK(snd_config_delete(c));
}

static void test_make_string(void)
{
	snd_config_t *c;
	const char *id;
	const char *value;

	ALSA_CHECK(snd_config_make_string(&c, "s"));
	TEST_CHECK(snd_config_get_type(c) == SND_CONFIG_TYPE_STRING);
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "s"));
	ALSA_CHECK(snd_config_get_string(c, &value));
	TEST_CHECK(value == NULL);
	ALSA_CHECK(snd_config_delete(c));
}

static void test_make_compound(void)
{
	snd_config_t *c;
	const char *id;

	ALSA_CHECK(snd_config_make_compound(&c, "c", 0));
	TEST_CHECK(snd_config_get_type(c) == SND_CONFIG_TYPE_COMPOUND);
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "c"));
	TEST_CHECK(snd_config_iterator_first(c) == snd_config_iterator_end(c));
	ALSA_CHECK(snd_config_delete(c));
}

static void test_imake_integer(void)
{
	snd_config_t *c;
	const char *id;
	long value;

	ALSA_CHECK(snd_config_imake_integer(&c, "i", 123));
	TEST_CHECK(snd_config_get_type(c) == SND_CONFIG_TYPE_INTEGER);
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "i"));
	ALSA_CHECK(snd_config_get_integer(c, &value));
	TEST_CHECK(value == 123);
	ALSA_CHECK(snd_config_delete(c));
}

static void test_imake_integer64(void)
{
	snd_config_t *c;
	const char *id;
	long long value;

	ALSA_CHECK(snd_config_imake_integer64(&c, "i", 123456789012345LL));
	TEST_CHECK(snd_config_get_type(c) == SND_CONFIG_TYPE_INTEGER64);
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "i"));
	ALSA_CHECK(snd_config_get_integer64(c, &value));
	TEST_CHECK(value == 123456789012345LL);
	ALSA_CHECK(snd_config_delete(c));
}

static void test_imake_string(void)
{
	snd_config_t *c;
	const char *id;
	const char *value;

	ALSA_CHECK(snd_config_imake_string(&c, "s", "xyzzy"));
	TEST_CHECK(snd_config_get_type(c) == SND_CONFIG_TYPE_STRING);
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "s"));
	ALSA_CHECK(snd_config_get_string(c, &value));
	TEST_CHECK(!strcmp(value, "xyzzy"));
	ALSA_CHECK(snd_config_delete(c));
}

static void test_get_type(void)
{
	snd_config_t *c;

	ALSA_CHECK(snd_config_top(&c));
	TEST_CHECK(snd_config_get_type(c) == SND_CONFIG_TYPE_COMPOUND);
	ALSA_CHECK(snd_config_delete(c));
	ALSA_CHECK(snd_config_make_integer(&c, "i"));
	TEST_CHECK(snd_config_get_type(c) == SND_CONFIG_TYPE_INTEGER);
	ALSA_CHECK(snd_config_delete(c));
	ALSA_CHECK(snd_config_make_string(&c, "s"));
	TEST_CHECK(snd_config_get_type(c) == SND_CONFIG_TYPE_STRING);
	ALSA_CHECK(snd_config_delete(c));
}

static void test_set_integer(void)
{
	snd_config_t *c;
	long value;

	ALSA_CHECK(snd_config_make_integer(&c, "i"));
	ALSA_CHECK(snd_config_set_integer(c, 123));
	ALSA_CHECK(snd_config_get_integer(c, &value));
	TEST_CHECK(value == 123);
	ALSA_CHECK(snd_config_delete(c));
	ALSA_CHECK(snd_config_make_string(&c, "s"));
	TEST_CHECK(snd_config_set_integer(c, 123) == -EINVAL);
	ALSA_CHECK(snd_config_delete(c));
}

static void test_set_integer64(void)
{
	snd_config_t *c;
	long long value;

	ALSA_CHECK(snd_config_make_integer64(&c, "i"));
	ALSA_CHECK(snd_config_set_integer64(c, 123456789012345LL));
	ALSA_CHECK(snd_config_get_integer64(c, &value));
	TEST_CHECK(value == 123456789012345LL);
	ALSA_CHECK(snd_config_delete(c));
	ALSA_CHECK(snd_config_make_string(&c, "s"));
	TEST_CHECK(snd_config_set_integer64(c, 123) == -EINVAL);
	ALSA_CHECK(snd_config_delete(c));
}

static void test_set_string(void)
{
	snd_config_t *c;
	const char *value;

	ALSA_CHECK(snd_config_make_string(&c, "s"));
	ALSA_CHECK(snd_config_set_string(c, "string"));
	ALSA_CHECK(snd_config_get_string(c, &value));
	TEST_CHECK(!strcmp(value, "string"));
	ALSA_CHECK(snd_config_set_string(c, NULL));
	ALSA_CHECK(snd_config_get_string(c, &value));
	TEST_CHECK(value == NULL);
	ALSA_CHECK(snd_config_delete(c));
	ALSA_CHECK(snd_config_make_integer(&c, "i"));
	TEST_CHECK(snd_config_set_string(c, "") == -EINVAL);
	ALSA_CHECK(snd_config_delete(c));
}

static void test_set_ascii(void)
{
	snd_config_t *c;
	const char *s;
	long i;

	ALSA_CHECK(snd_config_make_string(&c, "s"));
	ALSA_CHECK(snd_config_set_ascii(c, "foo"));
	ALSA_CHECK(snd_config_get_string(c, &s));
	TEST_CHECK(!strcmp(s, "foo"));
	ALSA_CHECK(snd_config_delete(c));
	ALSA_CHECK(snd_config_make_integer(&c, "i"));
	ALSA_CHECK(snd_config_set_ascii(c, "23"));
	ALSA_CHECK(snd_config_get_integer(c, &i));
	TEST_CHECK(i == 23);
	TEST_CHECK(snd_config_set_ascii(c, "half blue") == -EINVAL);
	ALSA_CHECK(snd_config_delete(c));
	ALSA_CHECK(snd_config_top(&c));
	TEST_CHECK(snd_config_set_ascii(c, "0") == -EINVAL);
	ALSA_CHECK(snd_config_delete(c));
}

static void test_get_id(void)
{
	snd_config_t *c;
	const char *id;

	ALSA_CHECK(snd_config_make_integer(&c, "my_id"));
	ALSA_CHECK(snd_config_get_id(c, &id));
	TEST_CHECK(!strcmp(id, "my_id"));
	ALSA_CHECK(snd_config_delete(c));
}

#define test_get_integer test_set_integer
#define test_get_integer64 test_set_integer64
#define test_get_string test_set_string

static void test_get_ascii(void)
{
	snd_config_t *c;
	char *value;

	ALSA_CHECK(snd_config_imake_integer(&c, "i", 123));
	ALSA_CHECK(snd_config_get_ascii(c, &value));
	TEST_CHECK(!strcmp(value, "123"));
	free(value);
	ALSA_CHECK(snd_config_delete(c));
	ALSA_CHECK(snd_config_imake_string(&c, "s", "bar"));
	ALSA_CHECK(snd_config_get_ascii(c, &value));
	TEST_CHECK(!strcmp(value, "bar"));
	free(value);
	ALSA_CHECK(snd_config_delete(c));
	ALSA_CHECK(snd_config_top(&c));
	TEST_CHECK(snd_config_get_ascii(c, &value) == -EINVAL);
	ALSA_CHECK(snd_config_delete(c));
}

static void test_iterators(void)
{
	snd_config_t *c, *c2;
	snd_config_iterator_t i;
	long v;

	ALSA_CHECK(snd_config_top(&c));
	i = snd_config_iterator_first(c);
	TEST_CHECK(i == snd_config_iterator_end(c));
	ALSA_CHECK(snd_config_imake_integer(&c2, "one", 1));
	ALSA_CHECK(snd_config_add(c, c2));
	i = snd_config_iterator_first(c);
	TEST_CHECK(i != snd_config_iterator_end(c));
	c2 = snd_config_iterator_entry(i);
	ALSA_CHECK(snd_config_get_integer(c2, &v));
	TEST_CHECK(v == 1);
	i = snd_config_iterator_next(i);
	TEST_CHECK(i == snd_config_iterator_end(c));
	ALSA_CHECK(snd_config_delete(c));
}

static void test_for_each(void)
{
	snd_config_t *c, *c2;
	snd_config_iterator_t i, next;
	long v;
	unsigned int count = 0;

	ALSA_CHECK(snd_config_top(&c));
	ALSA_CHECK(snd_config_imake_integer(&c2, "one", 1));
	ALSA_CHECK(snd_config_add(c, c2));
	snd_config_for_each(i, next, c) {
		TEST_CHECK(i != snd_config_iterator_end(c));
		c2 = snd_config_iterator_entry(i);
		ALSA_CHECK(snd_config_get_integer(c2, &v));
		TEST_CHECK(v == 1);
		++count;
	}
	TEST_CHECK(count == 1);
	ALSA_CHECK(snd_config_delete(c));
}

int main(void)
{
	test_top();
	test_load();
	test_save();
	test_update();
	test_search();
	test_searchv();
	test_add();
	test_delete();
	test_copy();
	test_make_integer();
	test_make_integer64();
	test_make_string();
	test_make_compound();
	test_imake_integer();
	test_imake_integer64();
	test_imake_string();
	test_get_type();
	test_set_integer();
	test_set_integer64();
	test_set_string();
	test_set_ascii();
	test_get_id();
	test_get_integer();
	test_get_integer64();
	test_get_string();
	test_get_ascii();
	test_iterators();
	test_for_each();
	return TEST_EXIT_CODE();
}
