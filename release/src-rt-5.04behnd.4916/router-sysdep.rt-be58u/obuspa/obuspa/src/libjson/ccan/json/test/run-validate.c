#include "common.h"

int main(void)
{
	const char *strings_file = "test/test-strings";
	FILE *f;
	char buffer[1024];
	
	plan_tests(224);
	
	f = fopen(strings_file, "rb");
	if (f == NULL) {
		diag("Could not open %s: %s", strings_file, strerror(errno));
		return 1;
	}
	
	while (fgets(buffer, sizeof(buffer), f)) {
		const char *s = chomp(buffer);
		bool valid;
		
		if (expect_literal(&s, "valid ")) {
			valid = true;
		} else if (expect_literal(&s, "invalid ")) {
			valid = false;
		} else {
			fail("Invalid line in test-strings: %s", buffer);
			continue;
		}
		
		if (strcmp(s, "\"1\\u2\"") == 0)
			puts("here");
		
		if (json_validate(s) == valid) {
			pass("%s %s", valid ? "valid" : "invalid", s);
		} else {
			fail("%s is %s, but json_validate returned %s",
				 s,
				 valid ? "valid" : "invalid",
				 valid ? "false" : "true");
		}
	}
	
	if (ferror(f) || fclose(f) != 0) {
		diag("I/O error reading test strings.");
		return 1;
	}
	
	return exit_status();
}
