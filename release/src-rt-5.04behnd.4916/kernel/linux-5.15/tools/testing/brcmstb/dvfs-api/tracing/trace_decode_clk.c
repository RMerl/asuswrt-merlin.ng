#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "trace.h"

/*
 * NOTE:
 *   Brcmstb clock name strings may be a maximum of 16 chars and
 *   this includes the null-terminating zero.  In addition, the
 *   MAX_LINE_LENGTH is actually less than 128 and this number
 *   derives from the our SCMI size max char limit.
 */
#define MAX_LINE_SIZE 128

static struct clk_name_info_s {
	unsigned int num_clks;
	const char *all_names;
	uint16_t *offsets;
} clk_info;

void trace_decode_clk_name_uninit(void)
{
	free((void *)clk_info.all_names);
	free((void *)clk_info.offsets);
	clk_info.all_names = NULL;
	clk_info.offsets = NULL;
	clk_info.num_clks = 0;
}

int trace_decode_clk_name_init(const char *filename)
{

	FILE *fp = fopen(filename, "r");
	char line[MAX_LINE_SIZE];
	char name[16];
	unsigned int clk_id;
	unsigned int all_names_str_size = 0;
	/* 1 so any unassigned slots in offsets[] will point to null str */
	unsigned int offset = 1;

	/* Determine size of names and number of clocks */
	if (fp == NULL)
		goto err;
	while (!feof(fp)) {
		if (fgets(line, MAX_LINE_SIZE - 1, fp) == NULL)
			break;
		if (sscanf(line, " %u %*s %15[^(]", &clk_id, name) == 2) {
			clk_info.num_clks++;
			all_names_str_size += strnlen(name, 15) + 1;
		}
	}
	fclose(fp);

	/* Allocate space, 1 extra due to offset=1, 16 extra due to strncpy */
	clk_info.all_names = calloc(1, all_names_str_size + 16 + 1);
	if (!clk_info.all_names)
		goto err;
	clk_info.offsets = calloc(clk_info.num_clks, sizeof(uint16_t));
	if (!clk_info.offsets)
		goto err;

	/* Reread the names of the clocks */
	fp = fopen(filename, "r");
	if (fp == NULL)
		goto err;
	while (!feof(fp)) {
		if (fgets(line, MAX_LINE_SIZE - 1, fp) == NULL)
			break;
		if (sscanf(line, " %u %*s %15[^(]", &clk_id, name) == 2) {
			strncpy((char *)clk_info.all_names + offset, name, 16);
			clk_info.offsets[clk_id] = offset;
			offset += strnlen(name, 15) + 1;
		}
	}
	fclose(fp);
	return 0;
err:
	trace_decode_clk_name_uninit();
	return -1;
}

void trace_decode_clk_name(unsigned int clk_id, char *buf, size_t bufsize)
{
	if (!clk_info.all_names)
		snprintf(buf, bufsize, "clkid=%u", clk_id);
	else if (clk_id >= clk_info.num_clks)
		snprintf(buf, bufsize, "illegal");
	else
		snprintf(buf, bufsize, "%s", clk_info.all_names + clk_info.offsets[clk_id]);
}


#ifdef DEBUG_TEST
int main(int argc, const char **argv)
{
	const char *debugfs = "/sys/kernel/debug/brcm-scmi/clk_summary";
	const char *filename = argc > 1 ? argv[1] : debugfs;
	int ret, i;

	printf("=> Using %s\n", filename);
	ret = trace_decode_clk_name_init(filename);
	printf("=> clk_name_init(...) => %s\n", ret ? "Fail" : "Pass");
	if (ret == 0) {
		for (i = 0; i < (int)clk_info.num_clks; i++)
			printf("\t%3d %s\n", i, trace_decode_clk_name(i));
		trace_decode_clk_name_uninit();
	}
	return ret;
}
#endif
