#include <stdio.h>
#include <stdlib.h>

#include <json.h>
#include "blobmsg.h"
#include "blobmsg_json.h"
#include "json_script.h"

struct json_script_ctx	jctx;
struct blob_buf 	b_vars;
struct blob_buf 	b_script;

static void handle_command(struct json_script_ctx *ctx, const char *name,
	struct blob_attr *data, struct blob_attr *vars)
{
	struct blob_attr *cur;
	int rem;

	fprintf(stdout, "%s", name);
	blobmsg_for_each_attr(cur, data, rem)
		fprintf(stdout, " %s", (char *) blobmsg_data(cur));
	fprintf(stdout, "\n");
}

static struct json_script_file *
handle_file(struct json_script_ctx *ctx, const char *filename)
{
	json_object *obj;

	obj = json_object_from_file(filename);
	if (!obj) {
		fprintf(stderr, "load JSON data from %s failed.\n", filename);
		return NULL;
	}

	blob_buf_init(&b_script, 0);
	blobmsg_add_json_element(&b_script, "", obj);
	json_object_put(obj);

	return json_script_file_from_blobmsg(filename,
		blob_data(b_script.head), blob_len(b_script.head));
}

static void usage(const char *prog, int exit_code)
{
	fprintf(stderr, "Usage: %s [VARNAME=value] <filename_json_script>\n", prog);
	exit(exit_code);
}

int main(int argc, char *argv[])
{
	int i;
	char *file = NULL;
	const char *prog = argv[0];

	blobmsg_buf_init(&b_vars);
	blobmsg_buf_init(&b_script);

	json_script_init(&jctx);
	jctx.handle_command = handle_command;
	jctx.handle_file = handle_file;

	for (i = 1; i < argc; i++) {
		char *sep = strchr(argv[i], '=');
		if (sep) {
			*sep = '\0';
			blobmsg_add_string(&b_vars, argv[i], sep + 1);
		} else if (!file) {
			file = argv[i];
		} else {
			usage(prog, -1);
		}
	}
	if (i < argc || !file)
		usage(prog, -2);

	json_script_run(&jctx, file, b_vars.head);

	json_script_free(&jctx);
	blob_buf_free(&b_script);
	blob_buf_free(&b_vars);

	return 0;
}
