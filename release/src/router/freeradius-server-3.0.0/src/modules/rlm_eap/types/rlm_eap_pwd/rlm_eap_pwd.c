/*
 * Copyright (c) Dan Harkins, 2012
 *
 *  Copyright holder grants permission for redistribution and use in source
 *  and binary forms, with or without modification, provided that the
 *  following conditions are met:
 *     1. Redistribution of source code must retain the above copyright
 *	notice, this list of conditions, and the following disclaimer
 *	in all source files.
 *     2. Redistribution in binary form must retain the above copyright
 *	notice, this list of conditions, and the following disclaimer
 *	in the documentation and/or other materials provided with the
 *	distribution.
 *
 *  "DISCLAIMER OF LIABILITY
 *
 *  THIS SOFTWARE IS PROVIDED BY DAN HARKINS ``AS IS'' AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INDUSTRIAL LOUNGE BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE."
 *
 * This license and distribution terms cannot be changed. In other words,
 * this code cannot simply be copied and put under a different distribution
 * license (including the GNU public license).
 */

RCSID("$Id$")
USES_APPLE_DEPRECATED_API	/* OpenSSL API has been deprecated by Apple */

#include "rlm_eap_pwd.h"

#include "eap_pwd.h"

#define MSK_EMSK_LEN    64
#define MPPE_KEY_LEN    32

static CONF_PARSER pwd_module_config[] = {
    { "group", PW_TYPE_INTEGER,
      offsetof(EAP_PWD_CONF, group), NULL, "19"},
    { "fragment_size", PW_TYPE_INTEGER,
      offsetof(EAP_PWD_CONF, fragment_size), NULL, "1020"},
    { "server_id", PW_TYPE_STRING_PTR,
      offsetof(EAP_PWD_CONF, server_id), NULL, NULL },
    { "virtual_server", PW_TYPE_STRING_PTR,
      offsetof(EAP_PWD_CONF, virtual_server), NULL, NULL },
    { NULL, -1, 0, NULL, NULL }
};

static int
mod_detach (void *arg)
{
    eap_pwd_t *inst;

    inst = (eap_pwd_t *) arg;

    if (inst->bnctx) {
	BN_CTX_free(inst->bnctx);
    }

    return 0;
}

static int
eap_pwd_attach (CONF_SECTION *cs, void **instance)
{
    eap_pwd_t *inst;

    *instance = inst = talloc_zero(cs, eap_pwd_t);
    if (!inst) return -1;

    inst->conf = talloc_zero(inst, EAP_PWD_CONF);
    if (!inst->conf) return -1;

    if (cf_section_parse(cs, inst->conf, pwd_module_config) < 0) {
	return -1;
    }

    if ((inst->bnctx = BN_CTX_new()) == NULL) {
	ERROR("rlm_eap_pwd: failed to get BN context!");
	return -1;
    }

    return 0;
}

static void
free_session (void *data)
{
    pwd_session_t *session = (pwd_session_t *)data;

    if (!session) {
	ERROR("rlm_eap_pwd: freeing a NULL session...naughty, naughty");
	return;
    }
    BN_free(session->private_value);
    BN_free(session->peer_scalar);
    BN_free(session->my_scalar);
    BN_free(session->k);
    EC_POINT_free(session->my_element);
    EC_POINT_free(session->peer_element);
    EC_GROUP_free(session->group);
    EC_POINT_free(session->pwe);
    BN_free(session->order);
    BN_free(session->prime);
}

static int
send_pwd_request (pwd_session_t *sess, EAP_DS *eap_ds)
{
    int len;
    uint16_t totlen;
    pwd_hdr *hdr;

    len = (sess->out_buf_len - sess->out_buf_pos) + sizeof(pwd_hdr);
    rad_assert(len > 0);
    eap_ds->request->code = PW_EAP_REQUEST;
    eap_ds->request->type.num = PW_EAP_PWD;
    eap_ds->request->type.length = (len > sess->mtu) ? sess->mtu : len;
    eap_ds->request->type.data = talloc_zero_array(eap_ds->request, uint8_t,
						   eap_ds->request->type.length);
    hdr = (pwd_hdr *)eap_ds->request->type.data;
    switch (sess->state) {
	case PWD_STATE_ID_REQ:
	    EAP_PWD_SET_EXCHANGE(hdr, EAP_PWD_EXCH_ID);
	    break;
	case PWD_STATE_COMMIT:
	    EAP_PWD_SET_EXCHANGE(hdr, EAP_PWD_EXCH_COMMIT);
	    break;
	case PWD_STATE_CONFIRM:
	    EAP_PWD_SET_EXCHANGE(hdr, EAP_PWD_EXCH_CONFIRM);
	    break;
	default:
	    DEBUG2("pwd state is messed up! Cannot send request");
	    return 0;
    }
    /*
     * are we fragmenting?
     */
    if ((int)((sess->out_buf_len - sess->out_buf_pos) + sizeof(pwd_hdr)) > sess->mtu) {
	EAP_PWD_SET_MORE_BIT(hdr);
	if (sess->out_buf_pos == 0) {
	    /*
	     * the first fragment, add the total length
	     */
	    EAP_PWD_SET_LENGTH_BIT(hdr);
	    totlen = ntohs(sess->out_buf_len);
	    memcpy(hdr->data, (char *)&totlen, sizeof(totlen));
	    memcpy(hdr->data + sizeof(uint16_t),
		   sess->out_buf,
		   sess->mtu - sizeof(pwd_hdr) - sizeof(uint16_t));
	    sess->out_buf_pos += (sess->mtu - sizeof(pwd_hdr) - sizeof(uint16_t));
	} else {
	    /*
	     * an intermediate fragment
	     */
	    memcpy(hdr->data, sess->out_buf + sess->out_buf_pos, (sess->mtu - sizeof(pwd_hdr)));
	    sess->out_buf_pos += (sess->mtu - sizeof(pwd_hdr));
	}
    } else {
	/*
	 * either it's not a fragment or it's the last fragment.
	 * The out buffer isn't needed anymore though so get rid of it.
	 */
	memcpy(hdr->data, sess->out_buf + sess->out_buf_pos,
	       (sess->out_buf_len - sess->out_buf_pos));
	talloc_free(sess->out_buf);
	sess->out_buf = NULL;
	sess->out_buf_pos = sess->out_buf_len = 0;
    }
    return 1;
}

static int
eap_pwd_initiate (void *instance, eap_handler_t *handler)
{
    pwd_session_t *pwd_session;
    eap_pwd_t *inst = (eap_pwd_t *)instance;
    VALUE_PAIR *vp;
    pwd_id_packet *pack;

    if (!inst || !handler) {
	ERROR("rlm_eap_pwd: initiate, NULL data provided");
	return -1;
    }

    /*
     * make sure the server's been configured properly
     */
    if (!inst->conf->server_id) {
	ERROR("rlm_eap_pwd: server ID is not configured!");
	return -1;
    }
    switch (inst->conf->group) {
	case 19:
	case 20:
	case 21:
	case 25:
	case 26:
	    break;
	default:
	    ERROR("rlm_eap_pwd: group is not supported!");
	    return -1;
    }

    if ((pwd_session = talloc_zero(handler, pwd_session_t)) == NULL) {
	return -1;
    }
    /*
     * set things up so they can be free'd reliably
     */
    pwd_session->group_num = inst->conf->group;
    pwd_session->private_value = NULL;
    pwd_session->peer_scalar = NULL;
    pwd_session->my_scalar = NULL;
    pwd_session->k = NULL;
    pwd_session->my_element = NULL;
    pwd_session->peer_element = NULL;
    pwd_session->group = NULL;
    pwd_session->pwe = NULL;
    pwd_session->order = NULL;
    pwd_session->prime = NULL;

    /*
     * figure out the MTU (basically do what eap-tls does)
     */
    pwd_session->mtu = inst->conf->fragment_size;
    vp = pairfind(handler->request->packet->vps, PW_FRAMED_MTU, 0, TAG_ANY);
    if (vp && ((int)(vp->vp_integer - 9) < pwd_session->mtu)) {
	/*
	 * 9 = 4 (EAPOL header) + 4 (EAP header) + 1 (EAP type)
	 *
	 * the fragmentation code deals with the included length
	 * so we don't need to subtract that here.
	 */
	pwd_session->mtu = vp->vp_integer - 9;
    }

    pwd_session->state = PWD_STATE_ID_REQ;
    pwd_session->in_buf = NULL;
    pwd_session->out_buf_pos = 0;
    handler->opaque = pwd_session;
    handler->free_opaque = free_session;

    /*
     * construct an EAP-pwd-ID/Request
     */
    pwd_session->out_buf_len = sizeof(pwd_id_packet) + strlen(inst->conf->server_id);
    if ((pwd_session->out_buf = talloc_zero_array(pwd_session, uint8_t,
						  pwd_session->out_buf_len)) == NULL) {
	return -1;
    }

    pack = (pwd_id_packet *)pwd_session->out_buf;
    pack->group_num = htons(pwd_session->group_num);
    pack->random_function = EAP_PWD_DEF_RAND_FUN;
    pack->prf = EAP_PWD_DEF_PRF;
    pwd_session->token = random();
    memcpy(pack->token, (char *)&pwd_session->token, 4);
    pack->prep = EAP_PWD_PREP_NONE;
    strcpy(pack->identity, inst->conf->server_id);

    handler->stage = AUTHENTICATE;

    return send_pwd_request(pwd_session, handler->eap_ds);
}

static int
mod_authenticate (void *arg, eap_handler_t *handler)
{
    pwd_session_t *pwd_session;
    pwd_hdr *hdr;
    pwd_id_packet *id;
    eap_packet_t *response;
    REQUEST *request, *fake;
    VALUE_PAIR *pw, *vp;
    EAP_DS *eap_ds;
    int len, ret = 0;
    eap_pwd_t *inst = (eap_pwd_t *)arg;
    uint16_t offset;
    uint8_t exch, *buf, *ptr, msk[MSK_EMSK_LEN], emsk[MSK_EMSK_LEN];
    uint8_t peer_confirm[SHA256_DIGEST_LENGTH];
    BIGNUM *x = NULL, *y = NULL;
    char *p;

    if ((!handler) ||
	((eap_ds = handler->eap_ds) == NULL) ||
	(!inst)) {
	return 0;
    }
    pwd_session = (pwd_session_t *)handler->opaque;
    request = handler->request;
    response = handler->eap_ds->response;
    hdr = (pwd_hdr *)response->type.data;

    buf = hdr->data;
    len = response->type.length - sizeof(pwd_hdr);

    /*
     * see if we're fragmenting, if so continue until we're done
     */
    if (pwd_session->out_buf_pos) {
	if (len) {
	    RDEBUG2("pwd got something more than an ACK for a fragment");
	}
	return send_pwd_request(pwd_session, eap_ds);
    }

    /*
     * the first fragment will have a total length, make a
     * buffer to hold all the fragments
     */
    if (EAP_PWD_GET_LENGTH_BIT(hdr)) {
	if (pwd_session->in_buf) {
	    RDEBUG2("pwd already alloced buffer for fragments");
	    return 0;
	}
	pwd_session->in_buf_len = ntohs(buf[0] * 256 | buf[1]);
	if ((pwd_session->in_buf = talloc_zero_array(pwd_session, uint8_t,
						     pwd_session->in_buf_len)) == NULL) {
	    RDEBUG2("pwd cannot allocate %d buffer to hold fragments",
		    pwd_session->in_buf_len);
	    return 0;
	}
	memset(pwd_session->in_buf, 0, pwd_session->in_buf_len);
	pwd_session->in_buf_pos = 0;
	buf += sizeof(uint16_t);
	len -= sizeof(uint16_t);
    }

    /*
     * all fragments, including the 1st will have the M(ore) bit set,
     * buffer those fragments!
     */
    if (EAP_PWD_GET_MORE_BIT(hdr)) {
	rad_assert(pwd_session->in_buf != NULL);
	if ((pwd_session->in_buf_pos + len) > pwd_session->in_buf_len) {
	    RDEBUG2("pwd will not overflow a fragment buffer. Nope, not prudent.");
	    return 0;
	}
	memcpy(pwd_session->in_buf + pwd_session->in_buf_pos, buf, len);
	pwd_session->in_buf_pos += len;

	/*
	 * send back an ACK for this fragment
	 */
	exch = EAP_PWD_GET_EXCHANGE(hdr);
	eap_ds->request->code = PW_EAP_REQUEST;
	eap_ds->request->type.num = PW_EAP_PWD;
	eap_ds->request->type.length = sizeof(pwd_hdr);
	if ((eap_ds->request->type.data = talloc_array(eap_ds->request,
						       uint8_t, sizeof(pwd_hdr))) == NULL) {
	    return 0;
	}
	hdr = (pwd_hdr *)eap_ds->request->type.data;
	EAP_PWD_SET_EXCHANGE(hdr, exch);
	return 1;

    }

    if (pwd_session->in_buf) {
	/*
	 * the last fragment...
	 */
	if ((pwd_session->in_buf_pos + len) > pwd_session->in_buf_len) {
	    RDEBUG2("pwd will not overflow a fragment buffer. Nope, not prudent.");
	    return 0;
	}
	memcpy(pwd_session->in_buf + pwd_session->in_buf_pos, buf, len);
	buf = pwd_session->in_buf;
	len = pwd_session->in_buf_len;
    }

    switch (pwd_session->state) {
	case PWD_STATE_ID_REQ:
	    if (EAP_PWD_GET_EXCHANGE(hdr) != EAP_PWD_EXCH_ID) {
		RDEBUG2("pwd exchange is incorrect: not ID");
		return 0;
	    }
	    id = (pwd_id_packet *)buf;
	    if ((id->prf != EAP_PWD_DEF_PRF) ||
		(id->random_function != EAP_PWD_DEF_RAND_FUN) ||
		(id->prep != EAP_PWD_PREP_NONE) ||
		(memcmp(id->token, (char *)&pwd_session->token, 4)) ||
		(id->group_num != ntohs(pwd_session->group_num))) {
		RDEBUG2("pwd id response is invalid");
		return 0;
	    }
	    /*
	     * we've agreed on the ciphersuite, record it...
	     */
	    ptr = (uint8_t *)&pwd_session->ciphersuite;
	    memcpy(ptr, (char *)&id->group_num, sizeof(uint16_t));
	    ptr += sizeof(uint16_t);
	    *ptr = EAP_PWD_DEF_RAND_FUN;
	    ptr += sizeof(uint8_t);
	    *ptr = EAP_PWD_DEF_PRF;

	    pwd_session->peer_id_len = len - sizeof(pwd_id_packet);
	    if (pwd_session->peer_id_len >= sizeof(pwd_session->peer_id)) {
		RDEBUG2("pwd id response is malformed");
		return 0;
	    }
	    memcpy(pwd_session->peer_id, id->identity,
		    pwd_session->peer_id_len);
	    pwd_session->peer_id[pwd_session->peer_id_len] = '\0';

	    /*
	     * make fake request to get the password for the usable ID
	     */
	    if ((fake = request_alloc_fake(handler->request)) == NULL) {
		RDEBUG("pwd unable to create fake request!");
		return 0;
	    }
	    fake->username = pairmake_packet("User-Name", NULL, T_OP_EQ);
	    if (!fake->username) {
		RDEBUG("pwd unanable to create value pair for username!");
		request_free(&fake);
		return 0;
	    }
	    fake->username->length = pwd_session->peer_id_len;
	    fake->username->vp_strvalue = p = talloc_array(fake->username, char, fake->username->length + 1);

	    memcpy(p, pwd_session->peer_id,
		   pwd_session->peer_id_len);
	    p[fake->username->length] = 0;

	    if ((vp = pairfind(request->config_items, PW_VIRTUAL_SERVER, 0, TAG_ANY)) != NULL) {
		    fake->server = vp->vp_strvalue;

	    } else if (inst->conf->virtual_server) {
		    fake->server = inst->conf->virtual_server;

	    } /* else fake->server == request->server */

	    if ((debug_flag > 0) && fr_log_fp) {
		    RDEBUG("Sending tunneled request");

		    debug_pair_list(fake->packet->vps);

		    fprintf(fr_log_fp, "server %s {\n",
			    (!fake->server) ? "" : fake->server);
	    }

	    /*
	     *	Call authorization recursively, which will
	     *	get the password.
	     */
	    process_authorize(0, fake);

	    /*
	     *	Note that we don't do *anything* with the reply
	     *	attributes.
	     */
	    if ((debug_flag > 0) && fr_log_fp) {
		    fprintf(fr_log_fp, "} # server %s\n",
			    (!fake->server) ? "" : fake->server);

		    RDEBUG("Got tunneled reply code %d", fake->reply->code);

		    debug_pair_list(fake->reply->vps);
	    }

	    if ((pw = pairfind(fake->config_items, PW_CLEARTEXT_PASSWORD, 0, TAG_ANY)) == NULL) {
		DEBUG2("failed to find password for %s to do pwd authentication",
		       pwd_session->peer_id);
		request_free(&fake);
		return 0;
	    }

	    if (compute_password_element(pwd_session, pwd_session->group_num,
					 pw->data.strvalue, strlen(pw->data.strvalue),
					 inst->conf->server_id, strlen(inst->conf->server_id),
					 pwd_session->peer_id, strlen(pwd_session->peer_id),
					 &pwd_session->token)) {
		DEBUG2("failed to obtain password element :-(");
		request_free(&fake);
		return 0;
	    }
	    request_free(&fake);

	    /*
	     * compute our scalar and element
	     */
	    if (compute_scalar_element(pwd_session, inst->bnctx)) {
		DEBUG2("failed to compute server's scalar and element");
		return 0;
	    }
	    if (((x = BN_new()) == NULL) ||
		((y = BN_new()) == NULL)) {
		DEBUG2("server point allocation failed");
		return 0;
	    }
	    /*
	     * element is a point, get both coordinates: x and y
	     */
	    if (!EC_POINT_get_affine_coordinates_GFp(pwd_session->group,
						     pwd_session->my_element, x, y,
						     inst->bnctx)) {
		DEBUG2("server point assignment failed");
		BN_free(x);
		BN_free(y);
		return 0;
	    }
	    /*
	     * construct request
	     */
	    pwd_session->out_buf_len = BN_num_bytes(pwd_session->order) +
		(2 * BN_num_bytes(pwd_session->prime));
	    if ((pwd_session->out_buf = talloc_array(pwd_session, uint8_t,
						     pwd_session->out_buf_len)) == NULL) {
		return 0;
	    }
	    memset(pwd_session->out_buf, 0, pwd_session->out_buf_len);

	    ptr = pwd_session->out_buf;
	    offset = BN_num_bytes(pwd_session->prime) - BN_num_bytes(x);
	    BN_bn2bin(x, ptr + offset);

	    ptr += BN_num_bytes(pwd_session->prime);
	    offset = BN_num_bytes(pwd_session->prime) - BN_num_bytes(y);
	    BN_bn2bin(y, ptr + offset);

	    ptr += BN_num_bytes(pwd_session->prime);
	    offset = BN_num_bytes(pwd_session->order) - BN_num_bytes(pwd_session->my_scalar);
	    BN_bn2bin(pwd_session->my_scalar, ptr + offset);

	    pwd_session->state = PWD_STATE_COMMIT;
	    ret = send_pwd_request(pwd_session, eap_ds);
	    break;
	case PWD_STATE_COMMIT:
	    if (EAP_PWD_GET_EXCHANGE(hdr) != EAP_PWD_EXCH_COMMIT) {
		RDEBUG2("pwd exchange is incorrect: not commit!");
		return 0;
	    }
	    /*
	     * process the peer's commit and generate the shared key, k
	     */
	    if (process_peer_commit(pwd_session, buf, inst->bnctx)) {
		RDEBUG2("failed to process peer's commit");
		return 0;
	    }

	    /*
	     * compute our confirm blob
	     */
	    if (compute_server_confirm(pwd_session, pwd_session->my_confirm, inst->bnctx)) {
		ERROR("rlm_eap_pwd: failed to compute confirm!");
		return 0;
	    }
	    /*
	     * construct a response...which is just our confirm blob
	     */
	    pwd_session->out_buf_len = SHA256_DIGEST_LENGTH;
	    if ((pwd_session->out_buf = talloc_array(pwd_session, uint8_t,
						     pwd_session->out_buf_len)) == NULL) {
		return 0;
	    }
	    memset(pwd_session->out_buf, 0, pwd_session->out_buf_len);
	    memcpy(pwd_session->out_buf, pwd_session->my_confirm, SHA256_DIGEST_LENGTH);

	    pwd_session->state = PWD_STATE_CONFIRM;
	    ret = send_pwd_request(pwd_session, eap_ds);
	    break;
	case PWD_STATE_CONFIRM:
	    if (EAP_PWD_GET_EXCHANGE(hdr) != EAP_PWD_EXCH_CONFIRM) {
		RDEBUG2("pwd exchange is incorrect: not commit!");
		return 0;
	    }
	    if (compute_peer_confirm(pwd_session, peer_confirm, inst->bnctx)) {
		RDEBUG2("pwd exchange cannot compute peer's confirm");
		return 0;
	    }
	    if (memcmp(peer_confirm, buf, SHA256_DIGEST_LENGTH)) {
		RDEBUG2("pwd exchange fails: peer confirm is incorrect!");
		return 0;
	    }
	    if (compute_keys(pwd_session, peer_confirm, msk, emsk)) {
		RDEBUG2("pwd exchange cannot generate (E)MSK!");
		return 0;
	    }
	    eap_ds->request->code = PW_EAP_SUCCESS;
	    /*
	     * return the MSK (in halves)
	     */
	    eap_add_reply(handler->request,
			  "MS-MPPE-Recv-Key", msk, MPPE_KEY_LEN);
	    eap_add_reply(handler->request,
			  "MS-MPPE-Send-Key", msk+MPPE_KEY_LEN, MPPE_KEY_LEN);
	    ret = 1;
	    break;
	default:
	    RDEBUG2("unknown PWD state");
	    return 0;
    }

    /*
     * we processed the buffered fragments, get rid of them
     */
    if (pwd_session->in_buf) {
	    talloc_free(pwd_session->in_buf);
	    pwd_session->in_buf = NULL;
    }

    return ret;
}


rlm_eap_module_t rlm_eap_pwd = {
    "eap_pwd",
    eap_pwd_attach,		     /* attach */
    eap_pwd_initiate,		   /* initiate to a client */
    NULL,			       /* no authorization */
    mod_authenticate,	       /* pwd authentication */
    mod_detach		      /* detach */
};

