#include "common.h"

int main(void)
{
	const char *strings_file = "test/test-strings";
	const char *strings_reencoded_file = "test/test-strings-reencoded";
	FILE *f, *f2;
	char buffer[1024], buffer2[1024];
	
	plan_tests(90);
	
	f = fopen(strings_file, "rb");
	if (f == NULL) {
		diag("Could not open %s: %s", strings_file, strerror(errno));
		return 1;
	}
	f2 = fopen(strings_reencoded_file, "rb");
	if (f2 == NULL) {
		diag("Could not open %s: %s", strings_reencoded_file, strerror(errno));
		return 1;
	}
	
	while (fgets(buffer, sizeof(buffer), f)) {
		const char *s = chomp(buffer);
		bool valid;
		JsonNode *node;
		
		if (expect_literal(&s, "valid ")) {
			valid = true;
		} else if (expect_literal(&s, "invalid ")) {
			valid = false;
		} else {
			fail("Invalid line in test-strings: %s", buffer);
			continue;
		}
		
		node = json_decode(s);
		
		if (valid) {
			char *reencoded;
			char errmsg[256];
			
			if (node == NULL) {
				fail("%s is valid, but json_decode returned NULL", s);
				continue;
			}
			
			if (!json_check(node, errmsg)) {
				fail("Corrupt tree produced by json_decode: %s", errmsg);
				continue;
			}
			
			reencoded = json_encode(node);
			
			if (!fgets(buffer2, sizeof(buffer2), f2)) {
				fail("test-strings-reencoded is missing this line: %s", reencoded);
				continue;
			}
			chomp(buffer2);
			
			ok(strcmp(reencoded, buffer2) == 0, "re-encode %s -> %s", s, reencoded);
			
			free(reencoded);
			json_delete(node);
		} else if (node != NULL) {
			fail("%s is invalid, but json_decode returned non-NULL", s);
			continue;
		}
	}
	
	if (ferror(f) || fclose(f) != 0 || ferror(f2) || fclose(f2) != 0) {
		diag("I/O error reading test data.");
		return 1;
	}
	
	return exit_status();
}
