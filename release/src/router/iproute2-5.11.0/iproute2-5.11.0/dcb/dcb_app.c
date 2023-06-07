// SPDX-License-Identifier: GPL-2.0+

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <libmnl/libmnl.h>
#include <linux/dcbnl.h>

#include "dcb.h"
#include "utils.h"
#include "rt_names.h"

static void dcb_app_help_add(void)
{
	fprintf(stderr,
		"Usage: dcb app { add | del | replace } dev STRING\n"
		"           [ default-prio PRIO ]\n"
		"           [ ethtype-prio ET:PRIO ]\n"
		"           [ stream-port-prio PORT:PRIO ]\n"
		"           [ dgram-port-prio PORT:PRIO ]\n"
		"           [ port-prio PORT:PRIO ]\n"
		"           [ dscp-prio INTEGER:PRIO ]\n"
		"\n"
		" where PRIO := { 0 .. 7 }\n"
		"       ET := { 0x600 .. 0xffff }\n"
		"       PORT := { 1 .. 65535 }\n"
		"       DSCP := { 0 .. 63 }\n"
		"\n"
	);
}

static void dcb_app_help_show_flush(void)
{
	fprintf(stderr,
		"Usage: dcb app { show | flush } dev STRING\n"
		"           [ default-prio ]\n"
		"           [ ethtype-prio ]\n"
		"           [ stream-port-prio ]\n"
		"           [ dgram-port-prio ]\n"
		"           [ port-prio ]\n"
		"           [ dscp-prio ]\n"
		"\n"
	);
}

static void dcb_app_help(void)
{
	fprintf(stderr,
		"Usage: dcb app help\n"
		"\n"
	);
	dcb_app_help_show_flush();
	dcb_app_help_add();
}

struct dcb_app_table {
	struct dcb_app *apps;
	size_t n_apps;
};

static void dcb_app_table_fini(struct dcb_app_table *tab)
{
	free(tab->apps);
}

static int dcb_app_table_push(struct dcb_app_table *tab, struct dcb_app *app)
{
	struct dcb_app *apps = reallocarray(tab->apps, tab->n_apps + 1,
					    sizeof(*tab->apps));

	if (apps == NULL) {
		perror("Cannot allocate APP table");
		return -ENOMEM;
	}

	tab->apps = apps;
	tab->apps[tab->n_apps++] = *app;
	return 0;
}

static void dcb_app_table_remove_existing(struct dcb_app_table *a,
					  const struct dcb_app_table *b)
{
	size_t ia, ja;
	size_t ib;

	for (ia = 0, ja = 0; ia < a->n_apps; ia++) {
		struct dcb_app *aa = &a->apps[ia];
		bool found = false;

		for (ib = 0; ib < b->n_apps; ib++) {
			const struct dcb_app *ab = &b->apps[ib];

			if (aa->selector == ab->selector &&
			    aa->protocol == ab->protocol &&
			    aa->priority == ab->priority) {
				found = true;
				break;
			}
		}

		if (!found)
			a->apps[ja++] = *aa;
	}

	a->n_apps = ja;
}

static void dcb_app_table_remove_replaced(struct dcb_app_table *a,
					  const struct dcb_app_table *b)
{
	size_t ia, ja;
	size_t ib;

	for (ia = 0, ja = 0; ia < a->n_apps; ia++) {
		struct dcb_app *aa = &a->apps[ia];
		bool present = false;
		bool found = false;

		for (ib = 0; ib < b->n_apps; ib++) {
			const struct dcb_app *ab = &b->apps[ib];

			if (aa->selector == ab->selector &&
			    aa->protocol == ab->protocol)
				present = true;
			else
				continue;

			if (aa->priority == ab->priority) {
				found = true;
				break;
			}
		}

		/* Entries that remain in A will be removed, so keep in the
		 * table only APP entries whose sel/pid is mentioned in B,
		 * but that do not have the full sel/pid/prio match.
		 */
		if (present && !found)
			a->apps[ja++] = *aa;
	}

	a->n_apps = ja;
}

static int dcb_app_table_copy(struct dcb_app_table *a,
			      const struct dcb_app_table *b)
{
	size_t i;
	int ret;

	for (i = 0; i < b->n_apps; i++) {
		ret = dcb_app_table_push(a, &b->apps[i]);
		if (ret != 0)
			return ret;
	}
	return 0;
}

static int dcb_app_cmp(const struct dcb_app *a, const struct dcb_app *b)
{
	if (a->protocol < b->protocol)
		return -1;
	if (a->protocol > b->protocol)
		return 1;
	return a->priority - b->priority;
}

static int dcb_app_cmp_cb(const void *a, const void *b)
{
	return dcb_app_cmp(a, b);
}

static void dcb_app_table_sort(struct dcb_app_table *tab)
{
	qsort(tab->apps, tab->n_apps, sizeof(*tab->apps), dcb_app_cmp_cb);
}

struct dcb_app_parse_mapping {
	__u8 selector;
	struct dcb_app_table *tab;
	int err;
};

static void dcb_app_parse_mapping_cb(__u32 key, __u64 value, void *data)
{
	struct dcb_app_parse_mapping *pm = data;
	struct dcb_app app = {
		.selector = pm->selector,
		.priority = value,
		.protocol = key,
	};

	if (pm->err)
		return;

	pm->err = dcb_app_table_push(pm->tab, &app);
}

static int dcb_app_parse_mapping_ethtype_prio(__u32 key, char *value, void *data)
{
	__u8 prio;

	if (key < 0x600) {
		fprintf(stderr, "Protocol IDs < 0x600 are reserved for EtherType\n");
		return -EINVAL;
	}

	if (get_u8(&prio, value, 0))
		return -EINVAL;

	return dcb_parse_mapping("ETHTYPE", key, 0xffff,
				 "PRIO", prio, IEEE_8021QAZ_MAX_TCS - 1,
				 dcb_app_parse_mapping_cb, data);
}

static int dcb_app_parse_dscp(__u32 *key, const char *arg)
{
	if (parse_mapping_num_all(key, arg) == 0)
		return 0;

	if (rtnl_dsfield_a2n(key, arg) != 0)
		return -1;

	if (*key & 0x03) {
		fprintf(stderr, "The values `%s' uses non-DSCP bits.\n", arg);
		return -1;
	}

	/* Unshift the value to convert it from dsfield to DSCP. */
	*key >>= 2;
	return 0;
}

static int dcb_app_parse_mapping_dscp_prio(__u32 key, char *value, void *data)
{
	__u8 prio;

	if (get_u8(&prio, value, 0))
		return -EINVAL;

	return dcb_parse_mapping("DSCP", key, 63,
				 "PRIO", prio, IEEE_8021QAZ_MAX_TCS - 1,
				 dcb_app_parse_mapping_cb, data);
}

static int dcb_app_parse_mapping_port_prio(__u32 key, char *value, void *data)
{
	__u8 prio;

	if (key == 0) {
		fprintf(stderr, "Port ID of 0 is invalid\n");
		return -EINVAL;
	}

	if (get_u8(&prio, value, 0))
		return -EINVAL;

	return dcb_parse_mapping("PORT", key, 0xffff,
				 "PRIO", prio, IEEE_8021QAZ_MAX_TCS - 1,
				 dcb_app_parse_mapping_cb, data);
}

static int dcb_app_parse_default_prio(int *argcp, char ***argvp, struct dcb_app_table *tab)
{
	int argc = *argcp;
	char **argv = *argvp;
	int ret = 0;

	while (argc > 0) {
		struct dcb_app app;
		__u8 prio;

		if (get_u8(&prio, *argv, 0)) {
			ret = 1;
			break;
		}

		app = (struct dcb_app){
			.selector = IEEE_8021QAZ_APP_SEL_ETHERTYPE,
			.protocol = 0,
			.priority = prio,
		};
		ret = dcb_app_table_push(tab, &app);
		if (ret != 0)
			break;

		argc--, argv++;
	}

	*argcp = argc;
	*argvp = argv;
	return ret;
}

static bool dcb_app_is_ethtype(const struct dcb_app *app)
{
	return app->selector == IEEE_8021QAZ_APP_SEL_ETHERTYPE &&
	       app->protocol != 0;
}

static bool dcb_app_is_default(const struct dcb_app *app)
{
	return app->selector == IEEE_8021QAZ_APP_SEL_ETHERTYPE &&
	       app->protocol == 0;
}

static bool dcb_app_is_dscp(const struct dcb_app *app)
{
	return app->selector == IEEE_8021QAZ_APP_SEL_DSCP;
}

static bool dcb_app_is_stream_port(const struct dcb_app *app)
{
	return app->selector == IEEE_8021QAZ_APP_SEL_STREAM;
}

static bool dcb_app_is_dgram_port(const struct dcb_app *app)
{
	return app->selector == IEEE_8021QAZ_APP_SEL_DGRAM;
}

static bool dcb_app_is_port(const struct dcb_app *app)
{
	return app->selector == IEEE_8021QAZ_APP_SEL_ANY;
}

static int dcb_app_print_key_dec(__u16 protocol)
{
	return print_uint(PRINT_ANY, NULL, "%d:", protocol);
}

static int dcb_app_print_key_hex(__u16 protocol)
{
	return print_uint(PRINT_ANY, NULL, "%x:", protocol);
}

static int dcb_app_print_key_dscp(__u16 protocol)
{
	const char *name = rtnl_dsfield_get_name(protocol << 2);


	if (!is_json_context() && name != NULL)
		return print_string(PRINT_FP, NULL, "%s:", name);
	return print_uint(PRINT_ANY, NULL, "%d:", protocol);
}

static void dcb_app_print_filtered(const struct dcb_app_table *tab,
				   bool (*filter)(const struct dcb_app *),
				   int (*print_key)(__u16 protocol),
				   const char *json_name,
				   const char *fp_name)
{
	bool first = true;
	size_t i;

	for (i = 0; i < tab->n_apps; i++) {
		struct dcb_app *app = &tab->apps[i];

		if (!filter(app))
			continue;
		if (first) {
			open_json_array(PRINT_JSON, json_name);
			print_string(PRINT_FP, NULL, "%s ", fp_name);
			first = false;
		}

		open_json_array(PRINT_JSON, NULL);
		print_key(app->protocol);
		print_uint(PRINT_ANY, NULL, "%d ", app->priority);
		close_json_array(PRINT_JSON, NULL);
	}

	if (!first) {
		close_json_array(PRINT_JSON, json_name);
		print_nl();
	}
}

static void dcb_app_print_ethtype_prio(const struct dcb_app_table *tab)
{
	dcb_app_print_filtered(tab, dcb_app_is_ethtype,  dcb_app_print_key_hex,
			       "ethtype_prio", "ethtype-prio");
}

static void dcb_app_print_dscp_prio(const struct dcb *dcb,
				    const struct dcb_app_table *tab)
{
	dcb_app_print_filtered(tab, dcb_app_is_dscp,
			       dcb->numeric ? dcb_app_print_key_dec
					    : dcb_app_print_key_dscp,
			       "dscp_prio", "dscp-prio");
}

static void dcb_app_print_stream_port_prio(const struct dcb_app_table *tab)
{
	dcb_app_print_filtered(tab, dcb_app_is_stream_port, dcb_app_print_key_dec,
			       "stream_port_prio", "stream-port-prio");
}

static void dcb_app_print_dgram_port_prio(const struct dcb_app_table *tab)
{
	dcb_app_print_filtered(tab, dcb_app_is_dgram_port, dcb_app_print_key_dec,
			       "dgram_port_prio", "dgram-port-prio");
}

static void dcb_app_print_port_prio(const struct dcb_app_table *tab)
{
	dcb_app_print_filtered(tab, dcb_app_is_port, dcb_app_print_key_dec,
			       "port_prio", "port-prio");
}

static void dcb_app_print_default_prio(const struct dcb_app_table *tab)
{
	bool first = true;
	size_t i;

	for (i = 0; i < tab->n_apps; i++) {
		if (!dcb_app_is_default(&tab->apps[i]))
			continue;
		if (first) {
			open_json_array(PRINT_JSON, "default_prio");
			print_string(PRINT_FP, NULL, "default-prio ", NULL);
			first = false;
		}
		print_uint(PRINT_ANY, NULL, "%d ", tab->apps[i].priority);
	}

	if (!first) {
		close_json_array(PRINT_JSON, "default_prio");
		print_nl();
	}
}

static void dcb_app_print(const struct dcb *dcb, const struct dcb_app_table *tab)
{
	dcb_app_print_ethtype_prio(tab);
	dcb_app_print_default_prio(tab);
	dcb_app_print_dscp_prio(dcb, tab);
	dcb_app_print_stream_port_prio(tab);
	dcb_app_print_dgram_port_prio(tab);
	dcb_app_print_port_prio(tab);
}

static int dcb_app_get_table_attr_cb(const struct nlattr *attr, void *data)
{
	struct dcb_app_table *tab = data;
	struct dcb_app *app;
	int ret;

	if (mnl_attr_get_type(attr) != DCB_ATTR_IEEE_APP) {
		fprintf(stderr, "Unknown attribute in DCB_ATTR_IEEE_APP_TABLE: %d\n",
			mnl_attr_get_type(attr));
		return MNL_CB_OK;
	}
	if (mnl_attr_get_payload_len(attr) < sizeof(struct dcb_app)) {
		fprintf(stderr, "DCB_ATTR_IEEE_APP payload expected to have size %zd, not %d\n",
			sizeof(struct dcb_app), mnl_attr_get_payload_len(attr));
		return MNL_CB_OK;
	}

	app = mnl_attr_get_payload(attr);
	ret = dcb_app_table_push(tab, app);
	if (ret != 0)
		return MNL_CB_ERROR;

	return MNL_CB_OK;
}

static int dcb_app_get(struct dcb *dcb, const char *dev, struct dcb_app_table *tab)
{
	uint16_t payload_len;
	void *payload;
	int ret;

	ret = dcb_get_attribute_va(dcb, dev, DCB_ATTR_IEEE_APP_TABLE, &payload, &payload_len);
	if (ret != 0)
		return ret;

	ret = mnl_attr_parse_payload(payload, payload_len, dcb_app_get_table_attr_cb, tab);
	if (ret != MNL_CB_OK)
		return -EINVAL;

	return 0;
}

struct dcb_app_add_del {
	const struct dcb_app_table *tab;
	bool (*filter)(const struct dcb_app *app);
};

static int dcb_app_add_del_cb(struct dcb *dcb, struct nlmsghdr *nlh, void *data)
{
	struct dcb_app_add_del *add_del = data;
	struct nlattr *nest;
	size_t i;

	nest = mnl_attr_nest_start(nlh, DCB_ATTR_IEEE_APP_TABLE);

	for (i = 0; i < add_del->tab->n_apps; i++) {
		const struct dcb_app *app = &add_del->tab->apps[i];

		if (add_del->filter == NULL || add_del->filter(app))
			mnl_attr_put(nlh, DCB_ATTR_IEEE_APP, sizeof(*app), app);
	}

	mnl_attr_nest_end(nlh, nest);
	return 0;
}

static int dcb_app_add_del(struct dcb *dcb, const char *dev, int command,
			   const struct dcb_app_table *tab,
			   bool (*filter)(const struct dcb_app *))
{
	struct dcb_app_add_del add_del = {
		.tab = tab,
		.filter = filter,
	};

	if (tab->n_apps == 0)
		return 0;

	return dcb_set_attribute_va(dcb, command, dev, dcb_app_add_del_cb, &add_del);
}

static int dcb_cmd_app_parse_add_del(struct dcb *dcb, const char *dev,
				     int argc, char **argv, struct dcb_app_table *tab)
{
	struct dcb_app_parse_mapping pm = {
		.tab = tab,
	};
	int ret;

	if (!argc) {
		dcb_app_help_add();
		return 0;
	}

	do {
		if (matches(*argv, "help") == 0) {
			dcb_app_help_add();
			return 0;
		} else if (matches(*argv, "ethtype-prio") == 0) {
			NEXT_ARG();
			pm.selector = IEEE_8021QAZ_APP_SEL_ETHERTYPE;
			ret = parse_mapping(&argc, &argv, false,
					    &dcb_app_parse_mapping_ethtype_prio,
					    &pm);
		} else if (matches(*argv, "default-prio") == 0) {
			NEXT_ARG();
			ret = dcb_app_parse_default_prio(&argc, &argv, pm.tab);
			if (ret != 0) {
				fprintf(stderr, "Invalid default priority %s\n", *argv);
				return ret;
			}
		} else if (matches(*argv, "dscp-prio") == 0) {
			NEXT_ARG();
			pm.selector = IEEE_8021QAZ_APP_SEL_DSCP;
			ret = parse_mapping_gen(&argc, &argv,
						&dcb_app_parse_dscp,
						&dcb_app_parse_mapping_dscp_prio,
						&pm);
		} else if (matches(*argv, "stream-port-prio") == 0) {
			NEXT_ARG();
			pm.selector = IEEE_8021QAZ_APP_SEL_STREAM;
			ret = parse_mapping(&argc, &argv, false,
					    &dcb_app_parse_mapping_port_prio,
					    &pm);
		} else if (matches(*argv, "dgram-port-prio") == 0) {
			NEXT_ARG();
			pm.selector = IEEE_8021QAZ_APP_SEL_DGRAM;
			ret = parse_mapping(&argc, &argv, false,
					    &dcb_app_parse_mapping_port_prio,
					    &pm);
		} else if (matches(*argv, "port-prio") == 0) {
			NEXT_ARG();
			pm.selector = IEEE_8021QAZ_APP_SEL_ANY;
			ret = parse_mapping(&argc, &argv, false,
					    &dcb_app_parse_mapping_port_prio,
					    &pm);
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_app_help_add();
			return -EINVAL;
		}

		if (ret != 0) {
			fprintf(stderr, "Invalid mapping %s\n", *argv);
			return ret;
		}
		if (pm.err)
			return pm.err;
	} while (argc > 0);

	return 0;
}

static int dcb_cmd_app_add(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct dcb_app_table tab = {};
	int ret;

	ret = dcb_cmd_app_parse_add_del(dcb, dev, argc, argv, &tab);
	if (ret != 0)
		return ret;

	ret = dcb_app_add_del(dcb, dev, DCB_CMD_IEEE_SET, &tab, NULL);
	dcb_app_table_fini(&tab);
	return ret;
}

static int dcb_cmd_app_del(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct dcb_app_table tab = {};
	int ret;

	ret = dcb_cmd_app_parse_add_del(dcb, dev, argc, argv, &tab);
	if (ret != 0)
		return ret;

	ret = dcb_app_add_del(dcb, dev, DCB_CMD_IEEE_DEL, &tab, NULL);
	dcb_app_table_fini(&tab);
	return ret;
}

static int dcb_cmd_app_show(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct dcb_app_table tab = {};
	int ret;

	ret = dcb_app_get(dcb, dev, &tab);
	if (ret != 0)
		return ret;

	dcb_app_table_sort(&tab);

	open_json_object(NULL);

	if (!argc) {
		dcb_app_print(dcb, &tab);
		goto out;
	}

	do {
		if (matches(*argv, "help") == 0) {
			dcb_app_help_show_flush();
			goto out;
		} else if (matches(*argv, "ethtype-prio") == 0) {
			dcb_app_print_ethtype_prio(&tab);
		} else if (matches(*argv, "dscp-prio") == 0) {
			dcb_app_print_dscp_prio(dcb, &tab);
		} else if (matches(*argv, "stream-port-prio") == 0) {
			dcb_app_print_stream_port_prio(&tab);
		} else if (matches(*argv, "dgram-port-prio") == 0) {
			dcb_app_print_dgram_port_prio(&tab);
		} else if (matches(*argv, "port-prio") == 0) {
			dcb_app_print_port_prio(&tab);
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_app_help_show_flush();
			ret = -EINVAL;
			goto out;
		}

		NEXT_ARG_FWD();
	} while (argc > 0);

out:
	close_json_object();
	dcb_app_table_fini(&tab);
	return 0;
}

static int dcb_cmd_app_flush(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct dcb_app_table tab = {};
	int ret;

	ret = dcb_app_get(dcb, dev, &tab);
	if (ret != 0)
		return ret;

	if (!argc) {
		ret = dcb_app_add_del(dcb, dev, DCB_CMD_IEEE_DEL, &tab, NULL);
		goto out;
	}

	do {
		if (matches(*argv, "help") == 0) {
			dcb_app_help_show_flush();
			goto out;
		} else if (matches(*argv, "ethtype-prio") == 0) {
			ret = dcb_app_add_del(dcb, dev, DCB_CMD_IEEE_DEL, &tab,
					      &dcb_app_is_ethtype);
			if (ret != 0)
				goto out;
		} else if (matches(*argv, "default-prio") == 0) {
			ret = dcb_app_add_del(dcb, dev, DCB_CMD_IEEE_DEL, &tab,
					      &dcb_app_is_default);
			if (ret != 0)
				goto out;
		} else if (matches(*argv, "dscp-prio") == 0) {
			ret = dcb_app_add_del(dcb, dev, DCB_CMD_IEEE_DEL, &tab,
					      &dcb_app_is_dscp);
			if (ret != 0)
				goto out;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			dcb_app_help_show_flush();
			ret = -EINVAL;
			goto out;
		}

		NEXT_ARG_FWD();
	} while (argc > 0);

out:
	dcb_app_table_fini(&tab);
	return ret;
}

static int dcb_cmd_app_replace(struct dcb *dcb, const char *dev, int argc, char **argv)
{
	struct dcb_app_table orig = {};
	struct dcb_app_table tab = {};
	struct dcb_app_table new = {};
	int ret;

	ret = dcb_app_get(dcb, dev, &orig);
	if (ret != 0)
		return ret;

	ret = dcb_cmd_app_parse_add_del(dcb, dev, argc, argv, &tab);
	if (ret != 0)
		goto out;

	/* Attempts to add an existing entry would be rejected, so drop
	 * these entries from tab.
	 */
	ret = dcb_app_table_copy(&new, &tab);
	if (ret != 0)
		goto out;
	dcb_app_table_remove_existing(&new, &orig);

	ret = dcb_app_add_del(dcb, dev, DCB_CMD_IEEE_SET, &new, NULL);
	if (ret != 0) {
		fprintf(stderr, "Could not add new APP entries\n");
		goto out;
	}

	/* Remove the obsolete entries. */
	dcb_app_table_remove_replaced(&orig, &tab);
	ret = dcb_app_add_del(dcb, dev, DCB_CMD_IEEE_DEL, &orig, NULL);
	if (ret != 0) {
		fprintf(stderr, "Could not remove replaced APP entries\n");
		goto out;
	}

out:
	dcb_app_table_fini(&new);
	dcb_app_table_fini(&tab);
	dcb_app_table_fini(&orig);
	return 0;
}

int dcb_cmd_app(struct dcb *dcb, int argc, char **argv)
{
	if (!argc || matches(*argv, "help") == 0) {
		dcb_app_help();
		return 0;
	} else if (matches(*argv, "show") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_app_show, dcb_app_help_show_flush);
	} else if (matches(*argv, "flush") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_app_flush, dcb_app_help_show_flush);
	} else if (matches(*argv, "add") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_app_add, dcb_app_help_add);
	} else if (matches(*argv, "del") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_app_del, dcb_app_help_add);
	} else if (matches(*argv, "replace") == 0) {
		NEXT_ARG_FWD();
		return dcb_cmd_parse_dev(dcb, argc, argv,
					 dcb_cmd_app_replace, dcb_app_help_add);
	} else {
		fprintf(stderr, "What is \"%s\"?\n", *argv);
		dcb_app_help();
		return -EINVAL;
	}
}
