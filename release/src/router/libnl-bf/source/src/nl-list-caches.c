/*
 * nl-list-caches.c	List registered cache types
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink-local.h>
#include <netlink/cli/utils.h>

static void print_usage(void)
{
	fprintf(stderr, "Usage: nl-list-caches\n");
	exit(1);
}

static char *id_attr_list(struct nl_object_ops *ops, char *buf, size_t len)
{
	if (ops->oo_attrs2str != NULL)
		return ops->oo_attrs2str(ops->oo_id_attrs, buf, len);
	else {
		memset(buf, 0, len);
		return buf;
	}
}

static void print(struct nl_cache_ops *ops, void *arg)
{
	char buf[64];

	printf("%s:\n" \
	       "    hdrsize: %d bytes\n" \
	       "    protocol: %s\n" \
	       "    request-update: %s\n" \
	       "    msg-parser: %s\n",
	       ops->co_name, ops->co_hdrsize,
	       nl_nlfamily2str(ops->co_protocol, buf, sizeof(buf)),
	       ops->co_request_update ? "yes" : "no",
	       ops->co_msg_parser ? "yes" : "no");

	if (ops->co_obj_ops) {
		struct nl_object_ops *obj_ops = ops->co_obj_ops;
		const char *dump_names[NL_DUMP_MAX+1] = {
			"brief",
			"detailed",
			"stats",
		};
		int i;

		printf("    cacheable object:\n" \
		       "        name: %s:\n" \
		       "        size: %zu bytes\n" \
		       "        constructor: %s\n" \
		       "        free-data: %s\n" \
		       "        clone: %s\n" \
		       "        compare: %s\n" \
		       "        id attributes: %s\n" \
		       "        dump: ",
		       obj_ops->oo_name, obj_ops->oo_size,
		       obj_ops->oo_constructor ? "yes" : "no",
		       obj_ops->oo_free_data ? "yes" : "no",
		       obj_ops->oo_clone ? "yes" : "no",
		       obj_ops->oo_compare ? "yes" : "no",
		       id_attr_list(obj_ops, buf, sizeof(buf)));

		for (i = 0; i <= NL_DUMP_MAX; i++)
			if (obj_ops->oo_dump[i])
				printf("%s%s",
				i == 0 ? "" : ", ",
				dump_names[i]);

		printf("\n");
	}

	if (ops->co_genl) {
		struct genl_ops *genl_ops = ops->co_genl;

		printf("    genl:\n" \
		       "        name: %s\n" \
		       "        family: %d\n" \
		       "        id: %d\n",
		       genl_ops->o_name, genl_ops->o_family, genl_ops->o_id);

		if (genl_ops->o_ncmds) {
			int i;

			printf("        cmds:\n");

			for (i = 0; i < genl_ops->o_ncmds; i++) {
				struct genl_cmd *cmd = &genl_ops->o_cmds[i];

				printf("            %s:\n"
				       "                id: %d\n" \
				       "                maxattr: %d\n" \
				       "                msg-parser: %s\n" \
				       "                attr-policy: %s\n",
				       cmd->c_name, cmd->c_id, cmd->c_maxattr,
				       cmd->c_msg_parser ? "yes" : "no",
				       cmd->c_attr_policy ? "yes" : "no");
			}
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc > 1 && !strcasecmp(argv[1], "-h"))
		print_usage();

	nl_cache_ops_foreach(print, NULL);

	return 0;
}
