/**
 * $Id$
 * @file decrypt.c
 * @brief Authentication for yubikey OTP tokens using the yubikey library.
 *
 * @author Arran Cudbard-Bell <a.cudbardb@networkradius.com>
 * @copyright 2013 The FreeRADIUS server project
 * @copyright 2013 Network RADIUS <info@networkradius.com>
 */
#include "rlm_yubikey.h"

#ifdef HAVE_YUBIKEY
/** Decrypt a Yubikey OTP AES block
 *
 * @param inst Module configuration.
 * @param otp string to decrypt.
 * @return one of the RLM_RCODE_* constants.
 */
rlm_rcode_t rlm_yubikey_decrypt(rlm_yubikey_t *inst, REQUEST *request, VALUE_PAIR *otp)
{
	uint32_t counter;
	yubikey_token_st token;

	DICT_ATTR const *da;

	char private_id[(YUBIKEY_UID_SIZE * 2) + 1];
	VALUE_PAIR *key, *vp;

	da = dict_attrbyname("Yubikey-Key");
	if (!da) {
		REDEBUG("Dictionary missing entry for 'Yubikey-Key'");
		return RLM_MODULE_FAIL;
	}

	key = pairfind(request->config_items, da->attr, da->vendor, TAG_ANY);
	if (!key) {
		REDEBUG("Yubikey-Key attribute not found in control list, can't decrypt OTP data");
		return RLM_MODULE_INVALID;
	}

	if (key->length != YUBIKEY_KEY_SIZE) {
		REDEBUG("Yubikey-Key length incorrect, expected %u got %zu", YUBIKEY_KEY_SIZE, key->length);
		return RLM_MODULE_INVALID;
	}

	yubikey_parse(otp->vp_octets + inst->id_len, key->vp_octets, &token);

	/*
	 *	Apparently this just uses byte offsets...
	 */
	if (!yubikey_crc_ok_p((uint8_t *) &token)) {
		REDEBUG("Decrypting OTP token data failed, rejecting");
		return RLM_MODULE_REJECT;
	}

	RDEBUG("Token data decrypted successfully");

	if (request->options && request->radlog) {
		(void) fr_bin2hex((char *) &private_id, (uint8_t*) &token.uid, YUBIKEY_UID_SIZE);
		RDEBUG2("Private ID	: 0x%s", private_id);
		RDEBUG2("Session counter   : %u", yubikey_counter(token.ctr));
		RDEBUG2("# used in session : %u", token.use);
		RDEBUG2("Token timestamp    : %u",
			(token.tstph << 16) | token.tstpl);
		RDEBUG2("Random data       : %u", token.rnd);
		RDEBUG2("CRC data          : 0x%x", token.crc);
	}

	/*
	 *	Private ID used for validation purposes
	 */
	vp = pairmake(request, &request->packet->vps, "Yubikey-Private-ID", NULL, T_OP_SET);
	if (!vp) {
		REDEBUG("Failed creating Yubikey-Private-ID");

		return RLM_MODULE_FAIL;
	}
	pairmemcpy(vp, token.uid, YUBIKEY_UID_SIZE);

	/*
	 *	Token timestamp
	 */
	vp = pairmake(request, &request->packet->vps, "Yubikey-Timestamp", NULL, T_OP_SET);
	if (!vp) {
		REDEBUG("Failed creating Yubikey-Timestamp");

		return RLM_MODULE_FAIL;
	}
	vp->vp_integer = (token.tstph << 16) | token.tstpl;
	vp->length = 4;

	/*
	 *	Token random
	 */
	vp = pairmake(request, &request->packet->vps, "Yubikey-Random", NULL, T_OP_SET);
	if (!vp) {
		REDEBUG("Failed creating Yubikey-Random");

		return RLM_MODULE_FAIL;
	}
	vp->vp_integer = token.rnd;
	vp->length = 4;

	/*
	 *	Combine the two counter fields together so we can do
	 *	replay attack checks.
	 */
	counter = (yubikey_counter(token.ctr) << 16) | token.use;

	vp = pairmake(request, &request->packet->vps, "Yubikey-Counter", NULL, T_OP_SET);
	if (!vp) {
		REDEBUG("Failed creating Yubikey-Counter");

		return RLM_MODULE_FAIL;
	}
	vp->vp_integer = counter;
	vp->length = 4;

	/*
	 *	Now we check for replay attacks
	 */
	vp = pairfind(request->config_items, vp->da->attr, vp->da->vendor, TAG_ANY);
	if (!vp) {
		RWDEBUG("Yubikey-Counter not found in control list, skipping replay attack checks");
		return RLM_MODULE_OK;
	}

	if (counter <= vp->vp_integer) {
		REDEBUG("Replay attack detected! Counter value %u, is lt or eq to last known counter value %u",
			counter, vp->vp_integer);
		return RLM_MODULE_REJECT;
	}

	return RLM_MODULE_OK;
}
#endif
