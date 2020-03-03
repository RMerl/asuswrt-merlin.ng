#include "symbol/kallsyms.h"
#include <stdio.h>
#include <stdlib.h>

int kallsyms__parse(const char *filename, void *arg,
		    int (*process_symbol)(void *arg, const char *name,
					  char type, u64 start))
{
	char *line = NULL;
	size_t n;
	int err = -1;
	FILE *file = fopen(filename, "r");

	if (file == NULL)
		goto out_failure;

	err = 0;

	while (!feof(file)) {
		u64 start;
		int line_len, len;
		char symbol_type;
		char *symbol_name;

		line_len = getline(&line, &n, file);
		if (line_len < 0 || !line)
			break;

		line[--line_len] = '\0'; /* \n */

		len = hex2u64(line, &start);

		len++;
		if (len + 2 >= line_len)
			continue;

		symbol_type = line[len];
		len += 2;
		symbol_name = line + len;
		len = line_len - len;

		if (len >= KSYM_NAME_LEN) {
			err = -1;
			break;
		}

		err = process_symbol(arg, symbol_name, symbol_type, start);
		if (err)
			break;
	}

	free(line);
	fclose(file);
	return err;

out_failure:
	return -1;
}
