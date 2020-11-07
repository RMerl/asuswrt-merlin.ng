/*
 * bcmiov.c
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id$
 */

/* Common iovar handling/parsing support - batching, parsing, sub-cmd dispatch etc.
 * To be used in firmware and host apps or dhd - reducing code size,
 * duplication, and maintenance overhead.
 */
#include <bcmendian.h>
#include <bcmiov.h>

/*
 * Make it tunable. TBD
 */
#define BCM_IOV_GET_BUF_SZ	1024
#define BCM_IOV_SUB_CMDS_OPT_SZ (sizeof(uint32))
#define BCM_IOV_INVALID_SUBCMD 0x0000

#ifdef BCMDBG
#define BCM_IOV_DBG(x) printf x
#else
#define BCM_IOV_DBG(x)
#endif // endif

/*
 * Dispatch info. entry
 */
typedef struct bcm_iov_disp_info {
	uint16 max_cmds; /* Max opaque cmd table size */
	uint16 max_tlvs; /* Max TLV table size */
	const bcm_iov_cmd_info_t *iovt_cmd_ent; /* opaque cmd disp entry */
	const bcm_iov_cmd_tlv_info_t *iovt_tlv_ent; /* TLV cmd disp entry */
	void *cmd_ctx;
} bcm_iov_disp_info_t;

/*
 * Parse context entry
 */
struct bcm_iov_parse_context {
	parse_context_opts_t options; /* to handle different ver lengths */
	bcm_iov_malloc_t alloc_fn;
	bcm_iov_free_t free_fn;
	bcm_iov_get_digest_t dig_fn;
	uint16 disp_cnt;
	uint16 disp_max; /* dispatch table limit */
	bcm_iov_disp_info_t **disp_info; /* cmd tables info */
	void *ctx;
	bcm_iov_cmd_digest_t *dig;
};

/*
 * TLV pack/unpack context
 */
typedef struct bcm_iov_batch_cmd_context {
	bcm_iov_parse_context_t *parse_ctx;
	bcm_iov_cmd_digest_t *dig;
	bcm_iov_batch_buf_t *p_cmd;
	const uint8 *cmd_data; /* ptr to sub-command data */
	uint16 cmd_len; /* sub-command length */
	void *result; /* ptr to the sub-commands results */
	int avail;
	uint32 actionid;
	uint32 status;
	bool is_batch;
	uint8 ncmds;
} bcm_iov_batch_cmd_context_t;

/*
 * Local functions
 */

/*
 * Lookup command table entries for Get/Set handler
 */
static int
bcm_iov_lookup_cmd_handler(bcm_iov_parse_context_t *p_ctx, uint16 disp_cnt,
	uint16 cmd_id, bcm_iov_cmd_digest_t *dig, const bcm_iov_cmd_info_t **cmd)
{
	int err = BCME_ERROR;
	uint16 i;
	bcm_iov_disp_info_t *disp;
	const bcm_iov_cmd_info_t *p_cmd = NULL;

	for (i = 0; i < disp_cnt; i++) {
		int idx;
		disp = p_ctx->disp_info[i];
		if (disp == NULL) {
			continue;
		}

		/*
		 * Search the commands table
		 */
		for (idx = 0; idx < disp->max_cmds; idx++) {
			p_cmd = &(disp->iovt_cmd_ent[idx]);
			if (p_cmd->cmd == cmd_id) {
				*cmd = p_cmd;
				dig->cmd_ctx = disp->cmd_ctx;
				err = BCME_OK;
				break;
			}
		}
	}

	return err;
}

/*
 * Attach function
 */
/*
 * Create parsing context.
 */
int
BCMATTACHFN(bcm_iov_create_parse_context)(const bcm_iov_parse_config_t *parse_cfg,
	bcm_iov_parse_context_t **parse_ctx)
{
	int err = BCME_OK;
	size_t sz_disp_info;
	bcm_iov_parse_context_t *ctx_p = NULL;

	ASSERT(parse_cfg != NULL);
	ASSERT(parse_cfg->alloc_fn != NULL);
	ASSERT(parse_cfg->free_fn != NULL);
	ASSERT(parse_cfg->dig_fn != NULL);

	/*
	 * Allocate parse context
	 */
	ctx_p = (bcm_iov_parse_context_t *)(parse_cfg->alloc_fn)(parse_cfg->alloc_ctx,
		sizeof(bcm_iov_parse_context_t));
	if (ctx_p == NULL) {
		err = BCME_NOMEM;
		goto done;
	}

	/*
	 * Zero the memory just in case if the module did
	 * not allocate using MALLOCZ
	 */
	memset(ctx_p, 0, sizeof(*ctx_p));

	sz_disp_info = (size_t)(sizeof(bcm_iov_disp_info_t *) * parse_cfg->max_regs);
	ctx_p->disp_info = (parse_cfg->alloc_fn)(parse_cfg->alloc_ctx, sz_disp_info);

	if (ctx_p->disp_info == NULL) {
		err = BCME_NOMEM;
		goto done;
	}

	ctx_p->alloc_fn = parse_cfg->alloc_fn;
	ctx_p->free_fn = parse_cfg->free_fn;
	ctx_p->ctx = parse_cfg->alloc_ctx;
	ctx_p->disp_max = (uint16)parse_cfg->max_regs;
	ctx_p->options = parse_cfg->options;
	ctx_p->dig_fn = parse_cfg->dig_fn;

	err = ctx_p->dig_fn(ctx_p->ctx, &ctx_p->dig);

done:
	if (err != BCME_OK) {
		bcm_iov_free_parse_context(&ctx_p, parse_cfg->free_fn);
	}
	*parse_ctx = ctx_p;

	return err;
}

/*
 * free the parsing context; ctx is set to NULL on exit
 */
int
BCMATTACHFN(bcm_iov_free_parse_context)(bcm_iov_parse_context_t **ctx, bcm_iov_free_t free_fn)
{
	int err = BCME_OK;
	int i;
	bcm_iov_parse_context_t *cp;

	if (!free_fn || !ctx || !*ctx) {
		goto done;
	}

	cp = *ctx;
	*ctx = NULL;

	if (cp->dig) {
		(*free_fn)(cp->ctx, cp->dig, sizeof(bcm_iov_cmd_digest_t));
	}
	if (cp->disp_info) {
		for (i = 0; i < cp->disp_cnt; i++) {
			(*free_fn)(cp->ctx, cp->disp_info[i], sizeof(*cp->disp_info[i]));
		}
		(*free_fn)(cp->ctx, cp->disp_info,  (sizeof(bcm_iov_disp_info_t *) * cp->disp_max));
	}
	(*free_fn)(cp->ctx, cp, sizeof(*cp));

done:
	return err;
}

void *
bcm_iov_get_cmd_ctx_info(bcm_iov_parse_context_t *cp)
{
	ASSERT(cp != NULL);
	return cp->ctx;
}

/*
 * return length of allocation for 'num_cmds' commands. data_len
 * including length of data for all the commands excluding the headers
 */
size_t
bcm_iov_get_alloc_len(int num_cmds, size_t data_len)
{
	size_t tlen = 0;

	return tlen;
}

/*
 * register a command info vector along with supported tlvs. Each command
 * may support a subset of tlvs
 */
int
bcm_iov_register_commands(bcm_iov_parse_context_t *parse_ctx, void *cmd_ctx,
	const bcm_iov_cmd_info_t *info, size_t num_cmds,
	const bcm_iov_cmd_tlv_info_t *tlv_info, size_t num_tlvs)
{
	int err = BCME_OK;
	uint16 idx;

	ASSERT(parse_ctx != NULL);
	idx = parse_ctx->disp_cnt;

	/*
	 * Allow if current count is less than max allowed
	 */
	if (idx < parse_ctx->disp_max) {
		bcm_iov_disp_info_t *p_disp;
		p_disp = (bcm_iov_disp_info_t *)(parse_ctx->alloc_fn)(parse_ctx->ctx,
			sizeof(*p_disp));
		if (p_disp == NULL) {
			err = BCME_NOMEM;
			goto done;
		}
		p_disp->max_cmds = (uint16)num_cmds;
		p_disp->max_tlvs = (uint16)num_tlvs;
		p_disp->cmd_ctx = cmd_ctx;
		p_disp->iovt_cmd_ent = info;
		p_disp->iovt_tlv_ent = tlv_info;
		parse_ctx->disp_info[idx] = p_disp;
		parse_ctx->disp_cnt++;
	} else {
		err = BCME_ERROR;
	}
done:
	return err;
}

/*
 * pack the xtlvs provided in the digest. may returns BCME_BUFTOOSHORT, but the
 * out_len is set to required length in that case.
 */
int
bcm_iov_pack_xtlvs(const bcm_iov_cmd_digest_t *dig,  bcm_xtlv_opts_t xtlv_opts,
	uint8 *out_buf, size_t out_size, size_t *out_len)
{
	int err = BCME_OK;
	/* TBD */
	return err;
}

/*
 * Return TRUE if input/output buffers overlap each other.
 */
static bool
bcm_iov_buf_overlap(void *p, void *a, uint p_len, uint a_len)
{
	bool ovlp = FALSE;

	if ((((uint8 *)p <= (uint8 *)a) && ((uint8 *)p + p_len) > (uint8 *)a) ||
		(((uint8 *)a <= (uint8 *)p) && ((uint8 *)a + a_len) > (uint8 *)p)) {
		ovlp = TRUE;
	}

	return ovlp;
}

/*
 * Dispatch Get/Set commands
 */
static int
bcm_iov_dispatch_sub_cmd(bcm_iov_batch_cmd_context_t *ctx, uint16 cmd_id)
{
	int err = BCME_OK;
	bcm_iov_parse_context_t *p_ctx = NULL;
	const bcm_iov_cmd_info_t *cmd = NULL;
	bcm_iov_cmd_digest_t *dig = ctx->dig;
	uint8 *res = NULL;
	int res_len;
	uint16 min_len;
	uint16 max_len;

	p_ctx = ctx->parse_ctx;

	/*
	 * Lookup sub-command handler
	 */
	if ((err = bcm_iov_lookup_cmd_handler(p_ctx, p_ctx->disp_cnt,
		cmd_id, dig, &cmd)) != BCME_OK) {
		goto have_result;
	}

	ASSERT(cmd != NULL);

	dig->cmd_info = cmd;

	/*
	 * Set up resp buffer and its max len and ensure space for resp hdr
	 */
	res = (uint8 *)ctx->result + ((ctx->is_batch) ? OFFSETOF(bcm_iov_batch_subcmd_t, data) :
		OFFSETOF(bcm_iov_buf_t, data));
	if (ctx->avail < (res - (uint8 *)ctx->result)) {
		err = BCME_BUFTOOSHORT;
		goto done; /* note: no space to return command status */
	}
	res_len = ctx->avail - (int)(res - (uint8 *)ctx->result);

	/*
	 * Validate lengths.
	 */
	if (IOV_ISSET(ctx->actionid)) {
		min_len = dig->cmd_info->min_len_set;
		max_len = dig->cmd_info->max_len_set;
	} else {
		min_len = dig->cmd_info->min_len_get;
		max_len = dig->cmd_info->max_len_get;
	}

	if (ctx->cmd_len < min_len) {
		err = BCME_BADLEN;
		goto have_result;
	}

	/* process only supported len and allow future extension */
	ctx->cmd_len = MIN(ctx->cmd_len, max_len);

	/*
	 * Validate command
	 */
	if (cmd->validate_h) {
		err = (cmd->validate_h)(ctx->dig, ctx->actionid, ctx->cmd_data,
			(size_t)ctx->cmd_len, res, (size_t *)&res_len);
	}

	if (err != BCME_OK) {
		goto have_result;
	}

	/*
	 * Dispatch get/set
	 */
	if (IOV_ISSET(ctx->actionid)) {
		if (cmd->set_h) {
			err = (cmd->set_h)(dig, ctx->cmd_data,
				(size_t)ctx->cmd_len, res, (size_t *)&res_len);
		} else {
			err = BCME_UNSUPPORTED;
		}
	} else {
		if (cmd->get_h) {
			err = (cmd->get_h)(dig, ctx->cmd_data,
				(size_t)ctx->cmd_len, res, (size_t *)&res_len);
		} else {
			err = BCME_UNSUPPORTED;
		}
	}

have_result:

	/* upon error, return only status - return values/buffer not deterministic */
	if (err != BCME_OK) {
		res_len = 0;
	}

	/*
	 * Save the status and len for this response. Data already in place
	 */
	if (ctx->is_batch) {
		bcm_iov_batch_subcmd_t *p_subcmd = (bcm_iov_batch_subcmd_t *)ctx->result;
		bcm_xtlv_t *ptlv = (bcm_xtlv_t *)ctx->result;

		/*
		 * Pack the response. Add response status len to length of the result
		 */
		bcm_xtlv_pack_xtlv(ptlv, cmd_id, res_len + BCM_IOV_STATUS_LEN,
			NULL, BCM_XTLV_OPTION_ALIGN32);
		/* status is part of tlv data */
		p_subcmd->u.status = htol32(err);

		/*
		 * Align response length to word boundary
		 */
		res_len = bcm_xtlv_size(ptlv, BCM_XTLV_OPTION_ALIGN32);

		/*
		 * Advance the response buffer for next response.
		 */
		ctx->result = ((uint8 *)ctx->result + res_len);
		ctx->avail -= res_len;
	} else {
		/*
		 * Done with non-batched command. handlers fill the data
		 * framework fills the version, id and len. Non batched
		 * commands get return status in bcm_iovar_t
		 */
		bcm_iov_buf_t *p_subcmd = (bcm_iov_buf_t *)ctx->result;
		p_subcmd->version = htol16((uint16)dig->version);
		p_subcmd->id = htol16(cmd_id);
		if (cmd && cmd->flags & BCM_IOV_CMD_FLAG_HDR_IN_LEN) {
			res_len += OFFSETOF(bcm_iov_buf_t, data);
		}
		p_subcmd->len = htol16((uint16)res_len);
	}

done:

	return err;
}

/*
 * TLV callback for sub-commands
 */
static int
bcm_iov_unpack_sub_cmd(void *ctx, const uint8 *buf, uint16 cmd_id, uint16 cmd_len)
{
	int err = BCME_OK;
	bcm_iov_batch_cmd_context_t *bc_ctx = (bcm_iov_batch_cmd_context_t *)ctx;
	bcm_iov_batch_buf_t *p_cmd;
	uint16 id;

	ASSERT(bc_ctx != NULL);
	ASSERT(buf != NULL);
	p_cmd = bc_ctx->p_cmd;
	ASSERT(p_cmd != NULL);

	/*
	 * range error if input buffer extends past the commands - XXX ignore
	 */
	if (bc_ctx->ncmds >= p_cmd->count) {
		err = BCME_RANGE;
		goto done;
	}
	if (cmd_len < BCM_IOV_SUB_CMDS_OPT_SZ) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	/* we have a (input) command */
	bc_ctx->ncmds += 1;

	/*
	 * TBD: Process options
	 */

	/*
	 * Skip over options
	 */
	buf += BCM_IOV_SUB_CMDS_OPT_SZ;
	cmd_len -= BCM_IOV_SUB_CMDS_OPT_SZ;
	bc_ctx->cmd_data = buf;
	id = ltoh16(cmd_id);
	bc_ctx->cmd_len = cmd_len;

	/*
	 * Dispatch sub-command
	 */
	if ((err = bcm_iov_dispatch_sub_cmd(bc_ctx, id)) != BCME_OK) {
		goto done;
	}

done:

	return err;
}

/*
 * wlc modules register their iovar(s) using the parsing context w/ wlc layer
 * during attach.
 */
#ifdef BCMDRIVER
int
bcm_iov_doiovar(void *ctx, uint32 id, void *params, uint params_len,
    void *arg, uint arg_len, uint vsize, struct wlc_if *wl_if)
{
	int err = BCME_OK;
	uint16 version;
	bcm_iov_batch_cmd_context_t bc_ctx_s;
	bcm_iov_batch_cmd_context_t *bc_ctx = NULL;
	bcm_iov_parse_context_t *p_ctx = NULL;
	uint8 *p_buf = NULL;

	/*
	 * Check NULL
	 */
	ASSERT(ctx != NULL);
	ASSERT(params != NULL);
	ASSERT(arg != NULL);

	p_ctx = (bcm_iov_parse_context_t *)ctx;
	if (!p_ctx) {
		err = BCME_BADARG;
		goto done;
	}

	ASSERT(p_ctx->alloc_fn != NULL);
	ASSERT(p_ctx->free_fn != NULL);

	if ((params_len < OFFSETOF(bcm_iov_batch_buf_t, count)) ||
		(params_len < OFFSETOF(bcm_iov_buf_t, len))) {
		err = BCME_BUFTOOSHORT;
		goto done;
	}

	/*
	 * Get version
	 */
	memcpy(&version, params, sizeof(version));
	version = ltoh16_ua(&version);

	bc_ctx = &bc_ctx_s;
	memset(bc_ctx, 0, sizeof(*bc_ctx));

	bc_ctx->parse_ctx = (bcm_iov_parse_context_t *)ctx;
	bc_ctx->actionid = id;

	/*
	 * Assign digest
	 */
	bc_ctx->dig = ((bcm_iov_parse_context_t *)ctx)->dig;

	/*
	 * Fill in some digest info.
	 */
	bc_ctx->dig->version = version;
	/*
	 * bsscfg from wlc_if requires wlc??
	 */
	bc_ctx->dig->bsscfg = bcm_iov_bsscfg_find_from_wlcif(NULL, wl_if);
	if (bc_ctx->dig->bsscfg == NULL) {
		err = BCME_NOTFOUND;
		goto done;
	}

	/*
	 * Check if input/output buffers overlap
	 */
	if (bcm_iov_buf_overlap(params, arg, params_len, arg_len)) {
		/*
		 * Alloc input copy buffer.
		 */
		p_buf = (p_ctx->alloc_fn)(p_ctx->ctx, params_len);
		if (p_buf == NULL) {
			err = BCME_NOMEM;
			goto done;
		}
		/*
		 * Make copy of input commands. The response is overwritten
		 * in the original params buffer passed.
		 */
		memcpy(p_buf, params, params_len);
	} else {
		/*
		 * Input buffer does not overlap with output. Just point
		 * p_buf to input buffer.
		 */
		p_buf = params;
	}

	/*
	 * If sub-commands are batched
	 */
	if (version & BCM_IOV_BATCH_MASK) {
		uint16 tlvs_len;
		bcm_iov_batch_buf_t *p_resp = (bcm_iov_batch_buf_t *)arg;
		bcm_iov_batch_buf_t *p_cmd = (bcm_iov_batch_buf_t *)p_buf;
		bc_ctx->p_cmd = p_cmd;
		bc_ctx->is_batch = TRUE;

		tlvs_len = (params_len - OFFSETOF(bcm_iov_batch_buf_t, cmds[0]));

		p_resp->version = htol16(version);
		p_resp->count = p_cmd->count;
		bc_ctx->avail = (arg_len - OFFSETOF(bcm_iov_batch_buf_t, cmds[0]));

		/*
		 * result tracks current response pointer address.
		 */
		bc_ctx->result = (void *)(((uint8 *)p_resp) +
				OFFSETOF(bcm_iov_batch_buf_t, cmds[0]));
		/* ensure we have space for status from at least one command */
		if (bc_ctx->avail < OFFSETOF(bcm_iov_batch_subcmd_t, data)) {
			err = BCME_BUFTOOSHORT;
			goto done;
		}

		/*
		 * Unpack and dispatch sub-commands. Results will be packed by
		 * sub-commands dispatcher. The command status is packed by command
		 * handling. Batch command always succeeds once dispatched.
		 */
		(void)bcm_unpack_xtlv_buf((void *)bc_ctx,
			(const uint8 *)&p_cmd->cmds[0],
			tlvs_len, BCM_IOV_CMD_OPT_ALIGN32,
			bcm_iov_unpack_sub_cmd);
		p_resp->count = bc_ctx->ncmds;
		if (bc_ctx->ncmds < 1) {
			err = BCME_BUFTOOSHORT; /* nothing executed */
			goto done;
		}
	} else {
		bcm_iov_buf_t *p_cmd = (bcm_iov_buf_t *)p_buf;
		bcm_iov_buf_t *p_resp = (bcm_iov_buf_t *)arg;
		uint16 cmd_id = ltoh16(p_cmd->id);

		bc_ctx->cmd_data = ((uint8 *)p_cmd + OFFSETOF(bcm_iov_buf_t, data));
		bc_ctx->cmd_len = ltoh16(p_cmd->len);
		bc_ctx->result = arg;
		bc_ctx->avail = (int)arg_len;

		err = bcm_iov_dispatch_sub_cmd(bc_ctx, cmd_id);
		p_resp->version = htol16(version);
	}

done:
	/*
	 * If input/output buffers overlap, then we need to free the input
	 * copy buffer.
	 */
	if (p_ctx != NULL && p_buf != NULL && params != p_buf) {
			(p_ctx->free_fn)(p_ctx->ctx, p_buf, params_len);
	}

	return err;
}
#endif /* BCMDRIVER */

/*
 * get the maximum number of tlvs - can be used to allocate digest for all
 * commands. the digest can be shared. Negative values are BCM_*, >=0, the
 * number of tlvs
 */
int
bcm_iov_parse_get_max_tlvs(const bcm_iov_parse_context_t *ctx)
{
	int err = BCME_OK;
	/* TBD */
	return err;
}

/*
 * pack a buffer of uint8s - memcpy wrapper
 */
int
bcm_iov_pack_buf(const bcm_iov_cmd_digest_t *dig, uint8 *buf,
	const uint8 *data, size_t len)
{
	int err = BCME_OK;
	/* TBD */
	return err;
}
/*
 * pack a buffer with uint16s - serialized in LE order, data points to uint16
 */
int
bcm_iov_packv_u16(const bcm_iov_cmd_digest_t *dig, uint8 *buf,
	const uint16 *data, int n)
{
	int err = BCME_OK;
	/* TBD */
	return err;
}

/*
 * pack a buffer with uint32s - serialized in LE order - data points to uint32
 */
int
bcm_iov_packv_u32(const bcm_iov_cmd_digest_t *dig, uint8 *buf,
	const uint32 *data, int n)
{
	int err = BCME_OK;

	/* TBD */
	return err;
}
