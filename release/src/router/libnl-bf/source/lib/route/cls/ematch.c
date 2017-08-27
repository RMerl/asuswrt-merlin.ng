/*
 * lib/route/cls/ematch.c	Extended Matches
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008-2010 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cls
 * @defgroup ematch Extended Match
 *
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/route/classifier.h>
#include <netlink/route/cls/ematch.h>
#include <netlink/route/cls/ematch/cmp.h>

#include "ematch_syntax.h"
#include "ematch_grammar.h"

/**
 * @name Module API
 * @{
 */

static NL_LIST_HEAD(ematch_ops_list);

/**
 * Register ematch module
 * @arg ops		Module operations.
 *
 * This function must be called by each ematch module at initialization
 * time. It registers the calling module as available module.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_ematch_register(struct rtnl_ematch_ops *ops)
{
	if (rtnl_ematch_lookup_ops(ops->eo_kind))
		return -NLE_EXIST;

	NL_DBG(1, "ematch module \"%s\" registered\n", ops->eo_name);

	nl_list_add_tail(&ops->eo_list, &ematch_ops_list);

	return 0;
}

/**
 * Lookup ematch module by identification number.
 * @arg kind		Module kind.
 *
 * Searches the list of registered ematch modules for match and returns it.
 *
 * @return Module operations or NULL if not found.
 */
struct rtnl_ematch_ops *rtnl_ematch_lookup_ops(int kind)
{
	struct rtnl_ematch_ops *ops;

	nl_list_for_each_entry(ops, &ematch_ops_list, eo_list)
		if (ops->eo_kind == kind)
			return ops;

	return NULL;
}

/**
 * Lookup ematch module by name
 * @arg name		Name of ematch module.
 *
 * Searches the list of registered ematch modules for a match and returns it.
 *
 * @return Module operations or NULL if not fuond.
 */
struct rtnl_ematch_ops *rtnl_ematch_lookup_ops_by_name(const char *name)
{
	struct rtnl_ematch_ops *ops;

	nl_list_for_each_entry(ops, &ematch_ops_list, eo_list)
		if (!strcasecmp(ops->eo_name, name))
			return ops;

	return NULL;
}

/** @} */

/**
 * @name Match
 */

/**
 * Allocate ematch object.
 *
 * Allocates and initializes an ematch object.
 *
 * @return New ematch object or NULL.
 */
struct rtnl_ematch *rtnl_ematch_alloc(void)
{
	struct rtnl_ematch *e;

	if (!(e = calloc(1, sizeof(*e))))
		return NULL;

	NL_DBG(2, "allocated ematch %p\n", e);

	NL_INIT_LIST_HEAD(&e->e_list);
	NL_INIT_LIST_HEAD(&e->e_childs);

	return e;
}

/**
 * Add ematch to the end of the parent's list of children.
 * @arg parent		parent ematch object
 * @arg child		ematch object to be added to parent
 *
 * The parent must be a container ematch.
 */
int rtnl_ematch_add_child(struct rtnl_ematch *parent,
			   struct rtnl_ematch *child)
{
	if (parent->e_kind != TCF_EM_CONTAINER)
		return -NLE_OPNOTSUPP;

	NL_DBG(2, "added ematch %p \"%s\" to container %p\n",
		  child, child->e_ops->eo_name, parent);

	nl_list_add_tail(&child->e_list, &parent->e_childs);

	return 0;
}

/**
 * Remove ematch from the list of ematches it is linked to.
 * @arg ematch		ematch object
 */
void rtnl_ematch_unlink(struct rtnl_ematch *ematch)
{
	NL_DBG(2, "unlinked ematch %p from any lists\n", ematch);

	if (!nl_list_empty(&ematch->e_childs))
		NL_DBG(1, "warning: ematch %p with childs was unlinked\n",
			  ematch);

	nl_list_del(&ematch->e_list);
	nl_init_list_head(&ematch->e_list);
}

void rtnl_ematch_free(struct rtnl_ematch *ematch)
{
	NL_DBG(2, "freed ematch %p\n", ematch);
	rtnl_ematch_unlink(ematch);
	free(ematch->e_data);
	free(ematch);
}

int rtnl_ematch_set_ops(struct rtnl_ematch *ematch, struct rtnl_ematch_ops *ops)
{
	if (ematch->e_ops)
		return -NLE_EXIST;

	ematch->e_ops = ops;
	ematch->e_kind = ops->eo_kind;

	if (ops->eo_datalen) {
		ematch->e_data = calloc(1, ops->eo_datalen);
		if (!ematch->e_data)
			return -NLE_NOMEM;

		ematch->e_datalen = ops->eo_datalen;
	}

	return 0;
}

int rtnl_ematch_set_kind(struct rtnl_ematch *ematch, uint16_t kind)
{
	struct rtnl_ematch_ops *ops;

	if (ematch->e_kind)
		return -NLE_EXIST;

	ematch->e_kind = kind;

	if ((ops = rtnl_ematch_lookup_ops(kind)))
		rtnl_ematch_set_ops(ematch, ops);

	return 0;
}

int rtnl_ematch_set_name(struct rtnl_ematch *ematch, const char *name)
{
	struct rtnl_ematch_ops *ops;

	if (ematch->e_kind)
		return -NLE_EXIST;

	if (!(ops = rtnl_ematch_lookup_ops_by_name(name)))
		return -NLE_OPNOTSUPP;

	rtnl_ematch_set_ops(ematch, ops);

	return 0;
}

void rtnl_ematch_set_flags(struct rtnl_ematch *ematch, uint16_t flags)
{
	ematch->e_flags |= flags;
}

void rtnl_ematch_unset_flags(struct rtnl_ematch *ematch, uint16_t flags)
{
	ematch->e_flags &= ~flags;
}

uint16_t rtnl_ematch_get_flags(struct rtnl_ematch *ematch)
{
	return ematch->e_flags;
}

void *rtnl_ematch_data(struct rtnl_ematch *ematch)
{
	return ematch->e_data;
}

/** @} */

/**
 * @name Tree
 */

/**
 * Allocate ematch tree object
 * @arg progid		program id
 */
struct rtnl_ematch_tree *rtnl_ematch_tree_alloc(uint16_t progid)
{
	struct rtnl_ematch_tree *tree;

	if (!(tree = calloc(1, sizeof(*tree))))
		return NULL;
	
	NL_INIT_LIST_HEAD(&tree->et_list);
	tree->et_progid = progid;

	NL_DBG(2, "allocated new ematch tree %p, progid=%u\n", tree, progid);

	return tree;
}

static void free_ematch_list(struct nl_list_head *head)
{
	struct rtnl_ematch *pos, *next;

	nl_list_for_each_entry_safe(pos, next, head, e_list) {
		if (!nl_list_empty(&pos->e_childs))
			free_ematch_list(&pos->e_childs);
		rtnl_ematch_free(pos);
	}
}

/**
 * Free ematch tree object
 * @arg tree		ematch tree object
 *
 * This function frees the ematch tree and all ematches attached to it.
 */
void rtnl_ematch_tree_free(struct rtnl_ematch_tree *tree)
{
	if (!tree)
		return;

	free_ematch_list(&tree->et_list);
	free(tree);

	NL_DBG(2, "Freed ematch tree %p\n", tree);
}

/**
 * Add ematch object to the end of the ematch tree
 * @arg tree		ematch tree object
 * @arg ematch		ematch object to add
 */
void rtnl_ematch_tree_add(struct rtnl_ematch_tree *tree,
			  struct rtnl_ematch *ematch)
{
	nl_list_add_tail(&ematch->e_list, &tree->et_list);
}

static inline uint32_t container_ref(struct rtnl_ematch *ematch)
{
	return *((uint32_t *) rtnl_ematch_data(ematch));
}

static int link_tree(struct rtnl_ematch *index[], int nmatches, int pos,
		     struct nl_list_head *root)
{
	struct rtnl_ematch *ematch;
	int i;

	for (i = pos; i < nmatches; i++) {
		ematch = index[i];

		nl_list_add_tail(&ematch->e_list, root);

		if (ematch->e_kind == TCF_EM_CONTAINER)
			link_tree(index, nmatches, container_ref(ematch),
				  &ematch->e_childs);

		if (!(ematch->e_flags & TCF_EM_REL_MASK))
			return 0;
	}

	/* Last entry in chain can't possibly have no relation */
	return -NLE_INVAL;
}

static struct nla_policy tree_policy[TCA_EMATCH_TREE_MAX+1] = {
	[TCA_EMATCH_TREE_HDR]  = { .minlen=sizeof(struct tcf_ematch_tree_hdr) },
	[TCA_EMATCH_TREE_LIST] = { .type = NLA_NESTED },
};

/**
 * Parse ematch netlink attributes
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_ematch_parse_attr(struct nlattr *attr, struct rtnl_ematch_tree **result)
{
	struct nlattr *a, *tb[TCA_EMATCH_TREE_MAX+1];
	struct tcf_ematch_tree_hdr *thdr;
	struct rtnl_ematch_tree *tree;
	struct rtnl_ematch **index;
	int nmatches = 0, err, remaining;

	NL_DBG(2, "Parsing attribute %p as ematch tree\n", attr);

	err = nla_parse_nested(tb, TCA_EMATCH_TREE_MAX, attr, tree_policy);
	if (err < 0)
		return err;

	if (!tb[TCA_EMATCH_TREE_HDR])
		return -NLE_MISSING_ATTR;

	thdr = nla_data(tb[TCA_EMATCH_TREE_HDR]);

	/* Ignore empty trees */
	if (thdr->nmatches == 0) {
		NL_DBG(2, "Ignoring empty ematch configuration\n");
		return 0;
	}

	if (!tb[TCA_EMATCH_TREE_LIST])
		return -NLE_MISSING_ATTR;

	NL_DBG(2, "ematch tree found with nmatches=%u, progid=%u\n",
		  thdr->nmatches, thdr->progid);

	/*
	 * Do some basic sanity checking since we will allocate
	 * index[thdr->nmatches]. Calculate how many ematch headers fit into
	 * the provided data and make sure nmatches does not exceed it.
	 */
	if (thdr->nmatches > (nla_len(tb[TCA_EMATCH_TREE_LIST]) /
			      nla_total_size(sizeof(struct tcf_ematch_hdr))))
		return -NLE_INVAL;

	if (!(index = calloc(thdr->nmatches, sizeof(struct rtnl_ematch *))))
		return -NLE_NOMEM;

	if (!(tree = rtnl_ematch_tree_alloc(thdr->progid))) {
		err = -NLE_NOMEM;
		goto errout;
	}

	nla_for_each_nested(a, tb[TCA_EMATCH_TREE_LIST], remaining) {
		struct rtnl_ematch_ops *ops;
		struct tcf_ematch_hdr *hdr;
		struct rtnl_ematch *ematch;
		void *data;
		size_t len;

		NL_DBG(3, "parsing ematch attribute %d, len=%u\n",
			  nmatches+1, nla_len(a));

		if (nla_len(a) < sizeof(*hdr)) {
			err = -NLE_INVAL;
			goto errout;
		}

		/* Quit as soon as we've parsed more matches than expected */
		if (nmatches >= thdr->nmatches) {
			err = -NLE_RANGE;
			goto errout;
		}

		hdr = nla_data(a);
		data = nla_data(a) + NLA_ALIGN(sizeof(*hdr));
		len = nla_len(a) - NLA_ALIGN(sizeof(*hdr));

		NL_DBG(3, "ematch attribute matchid=%u, kind=%u, flags=%u\n",
			  hdr->matchid, hdr->kind, hdr->flags);

		/*
		 * Container matches contain a reference to another sequence
		 * of matches. Ensure that the reference is within boundries.
		 */
		if (hdr->kind == TCF_EM_CONTAINER &&
		    *((uint32_t *) data) >= thdr->nmatches) {
			err = -NLE_INVAL;
			goto errout;
		}

		if (!(ematch = rtnl_ematch_alloc())) {
			err = -NLE_NOMEM;
			goto errout;
		}

		ematch->e_id = hdr->matchid;
		ematch->e_kind = hdr->kind;
		ematch->e_flags = hdr->flags;

		if ((ops = rtnl_ematch_lookup_ops(hdr->kind))) {
			if (ops->eo_minlen && len < ops->eo_minlen) {
				rtnl_ematch_free(ematch);
				err = -NLE_INVAL;
				goto errout;
			}

			rtnl_ematch_set_ops(ematch, ops);

			if (ops->eo_parse &&
			    (err = ops->eo_parse(ematch, data, len)) < 0) {
				rtnl_ematch_free(ematch);
				goto errout;
			}
		}

		NL_DBG(3, "index[%d] = %p\n", nmatches, ematch);
		index[nmatches++] = ematch;
	}

	if (nmatches != thdr->nmatches) {
		err = -NLE_INVAL;
		goto errout;
	}

	err = link_tree(index, nmatches, 0, &tree->et_list);
	if (err < 0)
		goto errout;

	free(index);
	*result = tree;

	return 0;

errout:
	rtnl_ematch_tree_free(tree);
	free(index);
	return err;
}

static void dump_ematch_sequence(struct nl_list_head *head,
				 struct nl_dump_params *p)
{
	struct rtnl_ematch *match;

	nl_list_for_each_entry(match, head, e_list) {
		if (match->e_flags & TCF_EM_INVERT)
			nl_dump(p, "!");

		if (match->e_kind == TCF_EM_CONTAINER) {
			nl_dump(p, "(");
			dump_ematch_sequence(&match->e_childs, p);
			nl_dump(p, ")");
		} else if (!match->e_ops) {
			nl_dump(p, "[unknown ematch %d]", match->e_kind);
		} else {
			if (match->e_ops->eo_dump)
				match->e_ops->eo_dump(match, p);
			else
				nl_dump(p, "[data]");
		}

		switch (match->e_flags & TCF_EM_REL_MASK) {
		case TCF_EM_REL_AND:
			nl_dump(p, " AND ");
			break;
		case TCF_EM_REL_OR:
			nl_dump(p, " OR ");
			break;
		default:
			/* end of first level ematch sequence */
			return;
		}
	}
}

void rtnl_ematch_tree_dump(struct rtnl_ematch_tree *tree,
			   struct nl_dump_params *p)
{
	if (!tree)
		BUG();

	dump_ematch_sequence(&tree->et_list, p);
	nl_dump(p, "\n");
}

static int update_container_index(struct nl_list_head *list, int *index)
{
	struct rtnl_ematch *e;

	nl_list_for_each_entry(e, list, e_list)
		e->e_index = (*index)++;

	nl_list_for_each_entry(e, list, e_list) {
		if (e->e_kind == TCF_EM_CONTAINER) {
			int err;

			if (nl_list_empty(&e->e_childs))
				return -NLE_OBJ_NOTFOUND;

			*((uint32_t *) e->e_data) = *index;

			err = update_container_index(&e->e_childs, index);
			if (err < 0)
				return err;
		}
	}

	return 0;
}

static int fill_ematch_sequence(struct nl_msg *msg, struct nl_list_head *list)
{
	struct rtnl_ematch *e;

	nl_list_for_each_entry(e, list, e_list) {
		struct tcf_ematch_hdr match = {
			.matchid = e->e_id,
			.kind = e->e_kind,
			.flags = e->e_flags,
		};
		struct nlattr *attr;
		int err = 0;

		if (!(attr = nla_nest_start(msg, e->e_index + 1)))
			return -NLE_NOMEM;

		if (nlmsg_append(msg, &match, sizeof(match), 0) < 0)
			return -NLE_NOMEM;

		if (e->e_ops->eo_fill)
			err = e->e_ops->eo_fill(e, msg);
		else if (e->e_flags & TCF_EM_SIMPLE)
			err = nlmsg_append(msg, e->e_data, 4, 0);
		else if (e->e_datalen > 0)
			err = nlmsg_append(msg, e->e_data, e->e_datalen, 0);

		NL_DBG(3, "msg %p: added ematch [%d] id=%d kind=%d flags=%d\n",
			  msg, e->e_index, match.matchid, match.kind, match.flags);

		if (err < 0)
			return -NLE_NOMEM;

		nla_nest_end(msg, attr);
	}

	nl_list_for_each_entry(e, list, e_list) {
		if (e->e_kind == TCF_EM_CONTAINER &&
		    fill_ematch_sequence(msg, &e->e_childs) < 0)
			return -NLE_NOMEM;
	}

	return 0;
}

int rtnl_ematch_fill_attr(struct nl_msg *msg, int attrid,
			  struct rtnl_ematch_tree *tree)
{
	struct tcf_ematch_tree_hdr thdr = {
		.progid = tree->et_progid,
	};
	struct nlattr *list, *topattr;
	int err, index = 0;

	/* Assign index number to each ematch to allow for references
	 * to be made while constructing the sequence of matches. */
	err = update_container_index(&tree->et_list, &index);
	if (err < 0)
		return err;

	if (!(topattr = nla_nest_start(msg, attrid)))
		goto nla_put_failure;

	thdr.nmatches = index;
	NLA_PUT(msg, TCA_EMATCH_TREE_HDR, sizeof(thdr), &thdr);

	if (!(list = nla_nest_start(msg, TCA_EMATCH_TREE_LIST)))
		goto nla_put_failure;

	if (fill_ematch_sequence(msg, &tree->et_list) < 0)
		goto nla_put_failure;

	nla_nest_end(msg, list);

	nla_nest_end(msg, topattr);

	return 0;

nla_put_failure:
	return -NLE_NOMEM;
}

/** @} */

extern int ematch_parse(void *, char **, struct nl_list_head *);

int rtnl_ematch_parse_expr(const char *expr, char **errp,
			   struct rtnl_ematch_tree **result)
{
	struct rtnl_ematch_tree *tree;
	YY_BUFFER_STATE buf = NULL;
	yyscan_t scanner = NULL;
	int err;

	NL_DBG(2, "Parsing ematch expression \"%s\"\n", expr);

	if (!(tree = rtnl_ematch_tree_alloc(RTNL_EMATCH_PROGID)))
		return -NLE_FAILURE;

	if ((err = ematch_lex_init(&scanner)) < 0) {
		err = -NLE_FAILURE;
		goto errout;
	}

	buf = ematch__scan_string(expr, scanner);

	if ((err = ematch_parse(scanner, errp, &tree->et_list)) != 0) {
		ematch__delete_buffer(buf, scanner);
		err = -NLE_PARSE_ERR;
		goto errout;
	}

	if (scanner)
		ematch_lex_destroy(scanner);

	*result = tree;

	return 0;

errout:
	if (scanner)
		ematch_lex_destroy(scanner);

	rtnl_ematch_tree_free(tree);

	return err;
}

static const char *layer_txt[] = {
	[TCF_LAYER_LINK]	= "eth",
	[TCF_LAYER_NETWORK]	= "ip",
	[TCF_LAYER_TRANSPORT]	= "tcp",
};

char *rtnl_ematch_offset2txt(uint8_t layer, uint16_t offset, char *buf, size_t len)
{
	snprintf(buf, len, "%s+%u",
		 (layer <= TCF_LAYER_MAX) ? layer_txt[layer] : "?",
		 offset);

	return buf;
}

static const char *operand_txt[] = {
	[TCF_EM_OPND_EQ] = "=",
	[TCF_EM_OPND_LT] = "<",
	[TCF_EM_OPND_GT] = ">",
};

char *rtnl_ematch_opnd2txt(uint8_t opnd, char *buf, size_t len)
{
	snprintf(buf, len, "%s",
		opnd <= ARRAY_SIZE(operand_txt) ? operand_txt[opnd] : "?");

	return buf;
}

/** @} */
