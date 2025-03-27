/* Build a list of numbers with various appends and prepends, verify them by testing against their encoded value, do pointer consistency checks each time, do element lookups, and remove items as well. */

#include "common.h"

#define should_be(var, expected) should_be_(var, #var, expected)

static void should_be_(const JsonNode *node, const char *name, const char *expected)
{
	char errmsg[256];
	char *encoded;
	
	if (!json_check(node, errmsg)) {
		fail("Invariants check failed: %s", errmsg);
		return;
	}
	
	encoded = json_encode(node);
	
	if (strcmp(encoded, expected) == 0)
		pass("%s is %s", name, expected);
	else
		fail("%s should be %s, but is actually %s", name, expected, encoded);
	
	free(encoded);
}

static void test_string(void)
{
	JsonNode *str;
	
	str = json_mkstring("Hello\tworld!\n\001");
	should_be(str, "\"Hello\\tworld!\\n\\u0001\"");
	json_delete(str);
	
	str = json_mkstring("\"\\\b\f\n\r\t");
	should_be(str, "\"\\\"\\\\\\b\\f\\n\\r\\t\"");
	json_delete(str);
}

static void test_number(void)
{
	JsonNode *num;
	
	num = json_mknumber(5678901234.0);
	should_be(num, "5678901234");
	json_delete(num);
	
	num = json_mknumber(-5678901234.0);
	should_be(num, "-5678901234");
	json_delete(num);
	
	num = json_mknumber(0.0 / 0.0);
	should_be(num, "null");
	json_delete(num);
}

static void test_array(void)
{
	JsonNode *array;
	JsonNode *children[5 + 1];
	
	array = json_mkarray();
	should_be(array, "[]");
	
	children[1] = json_mknumber(1);
	children[2] = json_mknumber(2);
	children[3] = json_mknumber(3);
	children[4] = json_mknumber(4);
	children[5] = json_mknumber(5);
	
	json_append_element(array, children[3]);
	should_be(array, "[3]");
	
	json_remove_from_parent(children[3]);
	should_be(array, "[]");
	
	json_prepend_element(array, children[3]);
	should_be(array, "[3]");
	
	json_prepend_element(array, children[2]);
	should_be(array, "[2,3]");
	
	json_append_element(array, children[4]);
	should_be(array, "[2,3,4]");
	
	json_delete(children[3]);
	should_be(array, "[2,4]");
	
	json_prepend_element(array, children[1]);
	should_be(array, "[1,2,4]");
	
	json_delete(children[1]);
	should_be(array, "[2,4]");
	
	json_delete(children[4]);
	should_be(array, "[2]");
	
	ok1(json_find_element(array, 0) == children[2]);
	ok1(json_find_element(array, -1) == NULL);
	ok1(json_find_element(array, 1) == NULL);
	
	json_append_element(array, children[5]);
	should_be(array, "[2,5]");
	
	ok1(json_find_element(array, 0) == children[2]);
	ok1(json_find_element(array, 1) == children[5]);
	ok1(json_find_element(array, -1) == NULL);
	ok1(json_find_element(array, 2) == NULL);
	
	json_delete(children[2]);
	json_delete(children[5]);
	should_be(array, "[]");
	
	ok1(json_find_element(array, -1) == NULL);
	ok1(json_find_element(array, 0) == NULL);
	ok1(json_find_element(array, 1) == NULL);
	
	json_delete(array);
}

static void test_object(void)
{
	JsonNode *object;
	JsonNode *children[5 + 1];
	
	object = json_mkobject();
	should_be(object, "{}");
	
	children[1] = json_mknumber(1);
	children[2] = json_mknumber(2);
	children[3] = json_mknumber(3);
	
	ok1(json_find_member(object, "one") == NULL);
	ok1(json_find_member(object, "two") == NULL);
	ok1(json_find_member(object, "three") == NULL);
	
	json_append_member(object, "one", children[1]);
	should_be(object, "{\"one\":1}");
	
	ok1(json_find_member(object, "one") == children[1]);
	ok1(json_find_member(object, "two") == NULL);
	ok1(json_find_member(object, "three") == NULL);
	
	json_prepend_member(object, "two", children[2]);
	should_be(object, "{\"two\":2,\"one\":1}");
	
	ok1(json_find_member(object, "one") == children[1]);
	ok1(json_find_member(object, "two") == children[2]);
	ok1(json_find_member(object, "three") == NULL);
	
	json_append_member(object, "three", children[3]);
	should_be(object, "{\"two\":2,\"one\":1,\"three\":3}");
	
	ok1(json_find_member(object, "one") == children[1]);
	ok1(json_find_member(object, "two") == children[2]);
	ok1(json_find_member(object, "three") == children[3]);
	
	json_delete(object);
}

int main(void)
{
	JsonNode *node;
	
	(void) chomp;
	
	plan_tests(49);
	
	ok1(json_find_element(NULL, 0) == NULL);
	ok1(json_find_member(NULL, "") == NULL);
	ok1(json_first_child(NULL) == NULL);
	
	node = json_mknull();
	should_be(node, "null");
	json_delete(node);
	
	node = json_mkbool(false);
	should_be(node, "false");
	json_delete(node);
	
	node = json_mkbool(true);
	should_be(node, "true");
	json_delete(node);
	
	test_string();
	test_number();
	test_array();
	test_object();
	
	return exit_status();
}
