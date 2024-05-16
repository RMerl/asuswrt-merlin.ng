#include "common.h"

static char buf1[256], buf2[256];

/* Used for pass and fail messages */
static char *quote_string(const char *str, char buf[256])
{
	char *out = buf;
	
	*out++ = '"';
	for (; *str != 0; str++) {
		if (out - buf > 256 - 5) {
			/* String is too long.  End it with `...' */
			out = buf + 256 - 5;
			*out++ = '.';
			*out++ = '.';
			*out++ = '.';
			break;
		}
		switch (*str) {
			case '\t':
				*out++ = '\\';
				*out++ = 't';
				break;
			case '\n':
				*out++ = '\\';
				*out++ = 'n';
				break;
			case '"':
				*out++ = '\\';
				*out++ = '"';
				break;
			case '\\':
				*out++ = '\\';
				*out++ = '\\';
				break;
			default:
				*out++ = *str;
				break;
		}
	}
	*out++ = '"';
	
	*out = 0;
	return buf;
}

static void test_stringify(const char *input, const char *expected)
{
	JsonNode *node = NULL;
	char *enc = NULL;
	char *strn = NULL;
	char *str = NULL;
	
	node = json_decode(input);
	if (node == NULL) {
		fail("Failed to decode %s", input);
		goto end;
	}
	
	enc = json_encode(node);
	if (strcmp(enc, input) != 0) {
		fail("%s re-encodes to %s.  Either encode/decode is broken, or the input string needs to be normalized", input, enc);
		goto end;
	}
	
	strn = json_stringify(node, NULL);
	if (strcmp(strn, enc) != 0) {
		fail("json_stringify with NULL space produced a different string than json_encode");
		goto end;
	}
	
	str = json_stringify(node, "\t");
	if (strcmp(str, expected) != 0) {
		fail("Expected %s, but json_stringify produced %s",
			 quote_string(expected, buf1), quote_string(str, buf2));
		goto end;
	}
	
	pass("stringify %s", input);
	
end:
	json_delete(node);
	free(enc);
	free(strn);
	free(str);
}

int main(void)
{
	(void) chomp;
	
	plan_tests(9);
	
	test_stringify("[]", "[]");
	test_stringify("[1]", "[\n\t1\n]");
	test_stringify("[1,2,3]", "[\n\t1,\n\t2,\n\t3\n]");
	test_stringify("[[]]", "[\n\t[]\n]");
	test_stringify("[[1,2],[3,4]]", "[\n\t[\n\t\t1,\n\t\t2\n\t],\n\t[\n\t\t3,\n\t\t4\n\t]\n]");
	
	test_stringify("{}", "{}");
	test_stringify("{\"one\":1}", "{\n\t\"one\": 1\n}");
	test_stringify("{\"one\":1,\"t*\":[2,3,10]}", "{\n\t\"one\": 1,\n\t\"t*\": [\n\t\t2,\n\t\t3,\n\t\t10\n\t]\n}");
	test_stringify("{\"a\":{\"1\":1,\"2\":2},\"b\":{\"3\":[null,false,true,\"\\f\"]}}",
				   "{\n\t\"a\": {\n\t\t\"1\": 1,\n\t\t\"2\": 2\n\t},\n\t\"b\": {\n\t\t\"3\": [\n\t\t\tnull,\n\t\t\tfalse,\n\t\t\ttrue,\n\t\t\t\"\\f\"\n\t\t]\n\t}\n}");
	
	return exit_status();
}
