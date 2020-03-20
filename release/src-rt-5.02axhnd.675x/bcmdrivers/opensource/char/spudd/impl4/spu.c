/*
<:copyright-BRCM:2015:GPL/GPL:spu

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#include <linux/kernel.h>
#include <linux/string.h>
#include <crypto/sha.h>
#include <crypto/md5.h>

#include "util.h"
#include "spu.h"

void dump_spu_msg_hdr(u8 *buf, unsigned buf_len)
{
	u8 *ptr = buf;
	struct SPUHEADER *spuh = (struct SPUHEADER *)buf;
	struct SPUHEADER tmp;
	unsigned hashKeyLength = 0;
	unsigned ICV_length = 0;
	unsigned hashStateLength = 0;

	packet_log("\n");
	packet_log("SPU Message header %p len: %u\n", buf, buf_len);

	/* ========== Process the MH & EMH ========== */
	packet_log("  MH 0x%08x\n", htonl(*((u32 *) ptr)));
	tmp.mh.headers.bits = ntohl(spuh->mh.headers.bits);
	if (tmp.mh.headers.flags.SCTX_PR)
		packet_log("    SCTX  present\n");
	if (tmp.mh.headers.flags.BDESC_PR)
		packet_log("    BDESC present\n");
	if (tmp.mh.headers.flags.MFM_PR)
		packet_log("    MFM   present\n");
	if (tmp.mh.headers.flags.BD_PR)
		packet_log("    BD    present\n");
	if (tmp.mh.headers.flags.HASH_PR)
		packet_log("    HASH  present\n");
	if (tmp.mh.headers.flags.SUPDT_PR)
		packet_log("    SUPDT present\n");
	packet_log("    Opcode 0x%02x\n", tmp.mh.headers.flags.opCode);

	/* skip MH and EMH */
	ptr += 2 * 4;

	/* ========== Process the SCTX ========== */
	if (tmp.mh.headers.flags.SCTX_PR) {
		tmp.sa.protocol.bits = ntohl(spuh->sa.protocol.bits);
		packet_log("  SCTX[0] 0x%08x\n", tmp.sa.protocol.bits);
		packet_log("    Size %u words\n",
			   tmp.sa.protocol.flags.SCTX_size);

		tmp.sa.cipher.bits = ntohl(spuh->sa.cipher.bits);
		packet_log("  SCTX[1] 0x%08x\n", tmp.sa.cipher.bits);
		packet_log("    Inbound:%u (1:decrypt/verify 0:encrypt/auth)\n",
			   tmp.sa.cipher.flags.inbound);
		packet_log("    Order:%u (1:AuthFirst 0:EncFirst)\n",
			   tmp.sa.cipher.flags.order);
		packet_log("    Crypto Alg:%u Mode:%u Type:%u\n",
			   tmp.sa.cipher.flags.cryptoAlg,
			   tmp.sa.cipher.flags.cryptoMode,
			   tmp.sa.cipher.flags.cryptoType);
		packet_log("    Hash   Alg:%u Mode:%u Type:%u, is_512 %u\n",
			   tmp.sa.cipher.flags.hashAlg,
			   tmp.sa.cipher.flags.hashMode,
			   tmp.sa.cipher.flags.hashType,
			   tmp.sa.cipher.flags.icv_is_512);
		packet_log("    UPDT_Offset:%u\n",
			   tmp.sa.cipher.flags.UPDT_ofst);

		tmp.sa.ecf.bits = ntohl(spuh->sa.ecf.bits);
		packet_log("  SCTX[2] 0x%08x\n", tmp.sa.ecf.bits);
		packet_log("    WriteICV:%u CheckICV:%u ICV_SIZE:%u\n",
			   tmp.sa.ecf.flags.insert_icv,
			   tmp.sa.ecf.flags.check_icv,
			   tmp.sa.ecf.flags.ICV_size);
		packet_log("    BD_SUPPRESS:%u\n", tmp.sa.ecf.flags.bd_suppress);
		packet_log("    SCTX_IV:%u ExplicitIV:%u GenIV:%u\n",
			   tmp.sa.ecf.flags.SCTX_IV,
			   tmp.sa.ecf.flags.explicit_IV,
			   tmp.sa.ecf.flags.gen_IV);
		packet_log("    EXP_IV_SIZE:%u\n",
			   tmp.sa.ecf.flags.explicit_IV_size);

		ICV_length = tmp.sa.ecf.flags.ICV_size * 4;

		ptr += sizeof(struct SCTX);

		if (tmp.sa.cipher.flags.hashAlg)
		{
			if (tmp.sa.cipher.flags.hashMode ||
			    (tmp.sa.cipher.flags.hashAlg == HASH_ALG_AES)) 
			{
				char *name = "NONE";
				switch (tmp.sa.cipher.flags.hashAlg) {
				case HASH_ALG_MD5:
					hashKeyLength = MD5_DIGEST_SIZE;
					name = "MD5";
					break;
				case HASH_ALG_SHA1:
					hashKeyLength = SHA1_DIGEST_SIZE;
					name = "SHA1";
					break;
				case HASH_ALG_SHA224:
					hashKeyLength = SHA224_DIGEST_SIZE;
					name = "SHA224";
					break;
				case HASH_ALG_SHA256:
					hashKeyLength = SHA256_DIGEST_SIZE;
					name = "SHA256";
					break;
				case HASH_ALG_SHA384:
					hashKeyLength = SHA384_DIGEST_SIZE;
					name = "SHA384";
					break;
				case HASH_ALG_SHA512:
					hashKeyLength = SHA512_DIGEST_SIZE;
					name = "SHA512";
					break;
				case HASH_ALG_AES:
					hashKeyLength = 0;
					name = "AES";
					break;
				case HASH_ALG_NONE:
					break;
				}

				packet_log("    Auth Type:%s Length:%u Bytes\n",
					   name, hashKeyLength);
				packet_dump("    Key: ", ptr, hashKeyLength);
				ptr += hashKeyLength;
			}
			else if ((tmp.sa.cipher.flags.hashType == HASH_TYPE_UPDT) ||
				 (tmp.sa.cipher.flags.hashType == HASH_TYPE_FIN)) 
			{
				char *name = "NONE";
				switch (tmp.sa.cipher.flags.hashAlg) {
				case HASH_ALG_MD5:
					hashStateLength = MD5_DIGEST_SIZE;
					name = "MD5";
					break;
				case HASH_ALG_SHA1:
					hashStateLength = SHA1_DIGEST_SIZE;
					name = "SHA1";
					break;
				case HASH_ALG_SHA224:
					/* state size is same as SHA256 */
					hashStateLength = SHA256_DIGEST_SIZE;
					name = "SHA224";
					break;
				case HASH_ALG_SHA256:
					hashStateLength = SHA256_DIGEST_SIZE;
					name = "SHA256";
					break;
				case HASH_ALG_SHA384:
					/* state size is same as SHA512 */
					hashStateLength = SHA512_DIGEST_SIZE;
					name = "SHA384";
					break;
				case HASH_ALG_SHA512:
					hashStateLength = SHA512_DIGEST_SIZE;
					name = "SHA512";
					break;
				case HASH_ALG_AES:
					hashStateLength = 0;
					name = "AES";
					break;
				case HASH_ALG_NONE:
					break;
				}

				packet_log("    Auth State Type:%s Length:%u Bytes\n",
					   name, hashStateLength);
				packet_dump("    State: ", ptr, hashStateLength);
				ptr += hashStateLength;
			}
		}

		if (tmp.sa.cipher.flags.cryptoAlg) {
			unsigned length = 0;
			char *name = "NONE";

			switch (tmp.sa.cipher.flags.cryptoAlg) {
			case CIPHER_ALG_DES:
				length = 8;
				name = "DES";
				break;
			case CIPHER_ALG_3DES:
				length = 24;
				name = "3DES";
				break;
			case CIPHER_ALG_RC4:
				length = 260;
				name = "ARC4";
				break;
			case CIPHER_ALG_AES:
				switch (tmp.sa.cipher.flags.cryptoType) {
				case CIPHER_TYPE_AES128:
					length = 16;
					name = "AES128";
					break;
				case CIPHER_TYPE_AES192:
					length = 24;
					name = "AES192";
					break;
				case CIPHER_TYPE_AES256:
					length = 32;
					name = "AES256";
					break;
				}
				break;
			case CIPHER_ALG_NONE:
				break;
			}

			packet_log("    Cipher Key Type:%s Length:%u Bytes\n",
			           name, length);
			/* XTS has two keys */
			if (tmp.sa.cipher.flags.cryptoMode == CIPHER_MODE_XTS) {
				packet_dump("    KEY2: ", ptr, length);
				ptr += length;
				packet_dump("    KEY1: ", ptr, length);
				ptr += length;
				length *= 2;
			}
			else {
				packet_dump("    KEY: ", ptr, length);
				ptr += length;
			}

			if (tmp.sa.ecf.flags.SCTX_IV) {
				unsigned iv_len = ((tmp.sa.protocol.flags.SCTX_size - 3) * 4)
				                  - (hashKeyLength + hashStateLength + length);
				packet_log("    IV Length:%u Bytes\n", iv_len);
				packet_dump("    IV: ", ptr, iv_len);
				ptr += iv_len;
			}
		}
	}

	/* ========== Process the BDESC ========== */
	if (tmp.mh.headers.flags.BDESC_PR) {
#ifdef DEBUG
		struct BDESC_HEADER *bdesc = (struct BDESC_HEADER *)ptr;
#endif
		packet_log("  BDESC[0] 0x%08x\n", htonl(*((u32 *)ptr)));
		packet_log("    OffsetMAC:%u LengthMAC:%u\n",
			   htons(bdesc->offsetMAC), htons(bdesc->lengthMAC));
		ptr += 4;

		packet_log("  BDESC[1] 0x%08x\n", htonl(*((u32 *) ptr)));
		packet_log("    OffsetCrypto:%u LengthCrypto:%u\n",
			   htons(bdesc->offsetCrypto),
			   htons(bdesc->lengthCrypto));
		ptr += 4;

		packet_log("  BDESC[2] 0x%08x\n", htonl(*((u32 *) ptr)));
		packet_log("    OffsetICV:%u OffsetIV:%u\n",
			   htons(bdesc->offsetICV), htons(bdesc->offsetIV));
		ptr += 4;
	}

	/* ========== Process the MFM ========== */
	if (tmp.mh.headers.flags.MFM_PR)
		packet_log("    MFM currently unparsed!\n");

	/* ========== Process the BD ========== */
	if (tmp.mh.headers.flags.BD_PR) {
#ifdef DEBUG
		struct BD_HEADER *bd = (struct BD_HEADER *) ptr;
#endif
		packet_log("  BD[0] 0x%08x\n", htonl(*((u32 *) ptr)));
		packet_log("    Size:%ubytes PrevLength:%u\n",
			   htons(bd->size), htons(bd->PrevLength));
		ptr += 4;
	}

	/* Double check sanity */
	if (buf + buf_len != ptr) {
		packet_log(" Packet parsed incorrectly. ");
		packet_log("buf:%p buf_len:%u buf+buf_len:%p ptr:%p\n",
			   buf, buf_len, buf + buf_len, ptr);
	}

	packet_log("\n");
}

/* Given a SPU-M message header, extract the payload length.
 * Assumes just MH, EMH, BD (no SCTX, BDESC. Works for response frames.
 */
u32 spu_payload_length(u8 *spu_hdr)
{
	struct BD_HEADER *bd;
	u32 pl_len;

	/* Find BD header.  skip MH, EMH */
	bd = (struct BD_HEADER *) (spu_hdr + 8);
	pl_len = be16_to_cpu(bd->size);

	return pl_len;
}

/* Determine the size of the receive buffer required to catch associated data.
 */
u32 spu_assoc_resp_len(enum spu_cipher_mode cipher_mode, bool dtls_hmac,
			unsigned int assoc_len, unsigned int iv_len)
{
	u32 buflen = 0;
	u32 pad;

	if (assoc_len || !dtls_hmac) {
		buflen = assoc_len;
		if ((cipher_mode != CIPHER_MODE_GCM) &&
		    (cipher_mode != CIPHER_MODE_CCM) && !dtls_hmac)
			buflen += iv_len;
	}

	if (cipher_mode == CIPHER_MODE_GCM) {
		/* AAD needs to be padded in responses too */
		pad = spu_gcm_ccm_padlen(cipher_mode, buflen);
		buflen += pad;
	}
	else if (cipher_mode == CIPHER_MODE_CCM) {
		/* AAD needs to be padded in responses
		 * for CCM, len + 2 needs to be 128-bit aligned.
		 */
		pad = spu_gcm_ccm_padlen(cipher_mode, buflen + 2);
		buflen += pad;
	}

	return buflen;
}

/* Return true if SPU request message should include the ICV as a separate
 * buffer
 */
bool spu_req_incl_icv(enum spu_cipher_mode cipher_mode, bool is_encrypt)
{
	if (((cipher_mode == CIPHER_MODE_GCM) || 
	     (cipher_mode == CIPHER_MODE_CCM)) &&
	     !is_encrypt) {
		return true;
	}
	else
	{
		return false;
	}
}

/*
 * Build a SPU request message header, up to and including the BD header.
 * Construct the message starting at spu_hdr. Caller should allocate this
 * buffer in DMA-able memory at least SPU_HEADER_ALLOC_LEN bytes long.
 * Returns the length of the SPU header in bytes. 0 if an error occurs.
 */
u32 spu_aead_req_create(u8 *spu_hdr,
		       u32 isInbound, u32 authFirst,
		       enum spu_cipher_alg cipher_alg,
		       enum spu_cipher_mode cipher_mode,
		       enum spu_cipher_type cipher_type,
		       u8 *cipher_key_buf,
		       u32 cipher_key_len,
		       u8 *cipher_iv_buf,
		       unsigned cipher_iv_len, enum hash_alg auth_alg,
		       enum hash_mode auth_mode, enum hash_type auth_type,
		       u8 digestsize, u8 *auth_key_buf,
		       u32 auth_key_len,
		       u32 assoc_size,
		       u32 data_size,
		       u32 dtls_aead,
		       u32 hmac_offset,
		       u32 aead_iv_buf_len,
		       u32 aad_pad_len,
		       u32 data_pad_len,
		       bool is_rfc4543)
{
	struct SPUHEADER *spuh;
	struct BDESC_HEADER *bdesc;
	struct BD_HEADER *bd;
	u8 *ptr;
	u32 protocol_bits = 0;
	u32 cipher_bits = 0;
	u32 ecf_bits = 0;
	u32 mh_bits = 0;
	u8 sctx_words = 0;
	u32 buf_len = 0;
	u32 cipher_len;
	u32 cipher_offset;
	u32 real_db_size;
	u32 auth_offset = 0;
	u32 offset_iv = 0;
	u32 auth_len;

	cipher_len = data_size;
	if ( dtls_aead) {
		cipher_len += aead_iv_buf_len;
	}
	if (isInbound && !dtls_aead) {
		cipher_len -= digestsize;
	}

	cipher_offset = assoc_size + aad_pad_len;
	if (!dtls_aead) {
		cipher_offset += aead_iv_buf_len;
	}
	real_db_size = spu_real_db_size(assoc_size, aead_iv_buf_len, 0,
	                                data_size, aad_pad_len, data_pad_len, 0);
	auth_len = real_db_size;
	if (dtls_aead) {
		auth_len = assoc_size + aead_iv_buf_len + hmac_offset;
	}
	else if (isInbound) {
		auth_len -= digestsize;
	}

	flow_log("%s()\n", __func__);
	flow_log("  in:%u authFirst:%u\n", isInbound, authFirst);
	flow_log("  cipher alg:%u mode:%u type %u\n", cipher_alg, cipher_mode,
	                                              cipher_type);
	flow_log("    key: %d\n", cipher_key_len);
	flow_dump("    key: ", cipher_key_buf, cipher_key_len);
	flow_log("    iv: %d\n", cipher_iv_len);
	flow_dump("    iv: ", cipher_iv_buf, cipher_iv_len);
	flow_log("  auth alg:%u mode:%u type %u\n", auth_alg, auth_mode,
	                                            auth_type);
	flow_log("  digestsize: %u\n", digestsize);
	flow_log("  authkey: %d\n", auth_key_len);
	flow_dump("  authkey: ", auth_key_buf, auth_key_len);
	flow_log("  assoc_size:%u\n", assoc_size);
	flow_log("  data_size:%u\n", data_size);
	flow_log
	    ("  dtls_aead:%u real_db_size:%u\n", dtls_aead, real_db_size);
	flow_log("  auth_offset:%u auth_len:%u cipher_offset:%u cipher_len:%u\n",
	         auth_offset, auth_len, cipher_offset, cipher_len);
	flow_log("  hmac_offset:%u\n", hmac_offset);
	flow_log("  aead_iv: %u\n", aead_iv_buf_len);

	/* starting out: zero the header (plus some) */
	ptr = spu_hdr;

	/* format master header word */
	/* Do not set the next bit even though the datasheet says to */
	spuh = (struct SPUHEADER *) ptr;
	ptr += sizeof(struct SPUHEADER);
	buf_len += sizeof(struct SPUHEADER);

	mh_bits = (SPU_CRYPTO_OPERATION_GENERIC << MH_OPCODE_SHIFT);
	mh_bits |= (1 << MH_SCTX_PRES_SHIFT);
	mh_bits |= (1 << MH_BDESC_PRES_SHIFT);
	mh_bits |= (1 << MH_BD_PRES_SHIFT);
	spuh->mh.headers.bits = htonl(mh_bits);

	spuh->emh = 0;

	/* Format sctx word 0 (protocol_bits) */
	sctx_words = 3;		/* size so far in words, update later */

	/* Format sctx word 1 (cipher_bits) */
	if (isInbound)
		cipher_bits |= 1 << CIPHER_INBOUND_SHIFT;
	if (authFirst)
		cipher_bits |= 1 << CIPHER_ORDER_SHIFT;

	/* Set the crypto parameters in the cipher.flags */
	cipher_bits |= cipher_alg << CIPHER_ALG_SHIFT;
	cipher_bits |= cipher_mode << CIPHER_MODE_SHIFT;
	cipher_bits |= cipher_type << CIPHER_TYPE_SHIFT;

	/* Set the auth parameters in the cipher.flags */
	cipher_bits |= auth_alg << HASH_ALG_SHIFT;
	cipher_bits |= auth_mode << HASH_MODE_SHIFT;
	cipher_bits |= auth_type << HASH_TYPE_SHIFT;

	/* Format sctx extensions if required, and
	   update main fields if required) */
	if (auth_alg) {
		/* Write the authentication key material if present */
		if (auth_key_len) {
			memcpy(ptr, auth_key_buf, auth_key_len);
			ptr += auth_key_len;
			buf_len += auth_key_len;
			sctx_words += auth_key_len / 4;
		}

		if ((cipher_mode == CIPHER_MODE_GCM) ||
		    (cipher_mode == CIPHER_MODE_CCM)) {
			offset_iv = assoc_size;  /* unpadded length */
		}

		/* in DTLS or GCM/CCM we need to write ICV into the payload */
		if (!isInbound) {
			if (dtls_aead || 
			   (cipher_mode == CIPHER_MODE_GCM) ||
			   (cipher_mode == CIPHER_MODE_CCM) ||
			   (cipher_alg == CIPHER_ALG_NONE)) {
				ecf_bits |= 1 << INSERT_ICV_SHIFT;
			}
		}
		else {
			ecf_bits |= 1 << CHECK_ICV_SHIFT;
		}

		/* Inform the SPU of the ICV size (in words) */
		if (digestsize == SHA512_DIGEST_SIZE)
		{
			cipher_bits |= (1 << ICV_IS_512_SHIFT);
		}
		else
		{
			ecf_bits |= (digestsize >> 2) << ICV_SIZE_SHIFT;
		}
	}

	/* copy the encryption keys in the SAD entry */
	if (cipher_alg) {
		if (cipher_key_len) {
			memcpy(ptr, cipher_key_buf, cipher_key_len);
			ptr += cipher_key_len;
			buf_len += cipher_key_len;
			sctx_words += cipher_key_len / 4;
		}

		/* if encrypting then set IV size, use
		   SCTX IV unless no IV given here */
		if (cipher_iv_buf && cipher_iv_len) {
			/* Use SCTX IV */
			ecf_bits |= 1 << SCTX_IV_SHIFT;

			/* cipher iv provided so put it in here */
			memcpy(ptr, cipher_iv_buf, cipher_iv_len);

			ptr += cipher_iv_len;
			buf_len += cipher_iv_len;
			sctx_words += cipher_iv_len / 4;
		}
	}

	/* write in the total sctx length now that we know it */
	protocol_bits |= sctx_words;

	/* Endian adjust the SCTX */
	spuh->sa.protocol.bits = htonl(protocol_bits);
	spuh->sa.cipher.bits = htonl(cipher_bits);
	spuh->sa.ecf.bits = htonl(ecf_bits);

	/* RFC4543 (GMAC/ESP) requires data to be sent as part of AAD
	 * so we need to override some BDESC parameters.
	 */
	if (is_rfc4543) {
		offset_iv = auth_len - data_pad_len;
		cipher_len = 0;
		cipher_offset = auth_len;
	}

	/* === create the BDESC section === */
	bdesc = (struct BDESC_HEADER *) ptr;

	/* in the case of DTLS we'll fill in the lengthMAC later */
	bdesc->offsetMAC = htons(auth_offset);
	bdesc->lengthMAC = htons(auth_len);
	bdesc->offsetCrypto = htons(cipher_offset);
	bdesc->lengthCrypto = htons(cipher_len);

	/* CCM ICV cannot be in same 32-bit word as data or
	 * padding. So add padding if necessary. */
	if (cipher_mode == CIPHER_MODE_CCM) {
		auth_len += spu_wordalign_padlen(auth_len);
	}

	bdesc->offsetICV = htons(auth_len);
	bdesc->offsetIV = htons(offset_iv);

	ptr += sizeof(struct BDESC_HEADER);
	buf_len += sizeof(struct BDESC_HEADER);

	/* === no MFM section === */

	/* === create the BD section === */

	/* add the BD header */
	bd = (struct BD_HEADER *) ptr;
	bd->size = htons(real_db_size);
	bd->PrevLength = 0;

	ptr += sizeof(struct BD_HEADER);
	buf_len += sizeof(struct BD_HEADER);

	packet_dump("  SPU request header: ", spu_hdr, buf_len);

	return buf_len;
}

/*
 * Build a SPU request message header, up to and including the BD header.
 * Construct the message starting at spu_hdr. Caller should allocate this
 * buffer in DMA-able memory at least SPU_HEADER_ALLOC_LEN bytes long.
 * Returns the length of the SPU header in bytes. 0 if an error occurs.
 */
u32 spu_hash_req_create(u8 *spu_hdr,
                        enum hash_alg auth_alg,
                        enum hash_mode auth_mode, 
                        enum hash_type auth_type,
                        u8 digestsize,
                        u8 *auth_key_buf,
                        unsigned auth_key_len,
                        u32 auth_len,
                        u32 prev_length_blocks,
                        u32 data_len)
{
	struct SPUHEADER *spuh;
	struct BDESC_HEADER *bdesc;
	struct BD_HEADER *bd;
	u32 mh_bits;
	u32 cipher_bits;
	u32 ecf_bits;
	u8 sctx_words;
	u8 *phdr = spu_hdr;

	flow_log("%s()\n", __func__);
	flow_log("  auth alg:%u mode:%u type:%u\n", auth_alg, auth_mode, auth_type);
	flow_log("  digestsize: %u\n", digestsize);
	flow_log("  spu_hdr: %p\n", spu_hdr);
	flow_log("  authkey: %d\n", auth_key_len);
	flow_dump("  authkey: ", auth_key_buf, auth_key_len);

	spuh = (struct SPUHEADER *)phdr;
	phdr += sizeof(struct SPUHEADER);

	mh_bits = (SPU_CRYPTO_OPERATION_GENERIC << MH_OPCODE_SHIFT);
	mh_bits |= (1 << MH_SCTX_PRES_SHIFT);
	mh_bits |= (1 << MH_BDESC_PRES_SHIFT);
	mh_bits |= (1 << MH_BD_PRES_SHIFT);
	spuh->mh.headers.bits = htonl(mh_bits);

	spuh->emh = 0;

	/* Format sctx word 1 (cipher_bits) */
	sctx_words = (sizeof(struct SCTX) >> 2);
	/* Set the auth parameters in the cipher.flags */
	cipher_bits = auth_alg << HASH_ALG_SHIFT;
	cipher_bits |= auth_mode << HASH_MODE_SHIFT;
	cipher_bits |= auth_type << HASH_TYPE_SHIFT;

	/* Format sctx word 2, add auth key if required */
	ecf_bits = (1 << BD_SUPPRESS);
	if (auth_alg) {
		/* Write the authentication key material if present */
		if (auth_key_len) {
			memcpy(phdr, auth_key_buf, auth_key_len);
			phdr += auth_key_len;
			sctx_words += (auth_key_len >> 2);
		}
		/* Inform the SPU of the ICV size (in words) */
		if (digestsize == SHA512_DIGEST_SIZE)
		{
			cipher_bits |= (1 << ICV_IS_512_SHIFT);
		}
		else
		{
			ecf_bits |= (digestsize >> 2) << ICV_SIZE_SHIFT;
		}
	}

	spuh->sa.protocol.bits = htonl(sctx_words);
	spuh->sa.cipher.bits = htonl(cipher_bits);
	spuh->sa.ecf.bits = htonl(ecf_bits);

	bdesc = (struct BDESC_HEADER *)phdr;
	bdesc->offsetMAC = 0;
	bdesc->lengthMAC = htons(auth_len);
	bdesc->offsetCrypto = 0;
	bdesc->lengthCrypto = 0;
	bdesc->offsetICV = 0;
	bdesc->offsetIV = 0;
	phdr += sizeof(struct BDESC_HEADER);

	/* add the BD header */
	bd = (struct BD_HEADER *)phdr;
	bd->size = htons(data_len);
	bd->PrevLength = htons(prev_length_blocks);
	phdr += sizeof(struct BD_HEADER);

	packet_dump("  SPU request header: ", spu_hdr, (phdr - spu_hdr));
	return (phdr - spu_hdr);
}

/*
 * Build an SPU request message header, up to and including the BD header.
 * Construct the message starting at spu_hdr. Caller should allocate this
 * buffer in DMA-able memory at least SPU_HEADER_ALLOC_LEN bytes long.
 * Returns the length of the SPU header in bytes. 0 if an error occurs.
 */
u16 spu_cipher_req_init(u8 *spu_hdr,
			enum spu_cipher_alg cipher_alg,
			enum spu_cipher_mode cipher_mode,
			enum spu_cipher_type cipher_type,
			u8 *cipher_key_buf,
			unsigned cipher_key_len,
			u32 cipher_iv_len)
{

	struct SPUHEADER *spuh;
	u32 protocol_bits = 0;
	u32 cipher_bits = 0;
	u32 mh_bits = 0;
	u32 ecf_bits = 0;
	u8 sctx_words = 0;
	u8 *ptr = spu_hdr;

	flow_log("%s()\n", __func__);
	flow_log("  cipher alg:%u mode:%u type %u\n", cipher_alg, cipher_mode,
		 cipher_type);
	flow_log("  cipher_iv_len: %u\n", cipher_iv_len);
	flow_log("    key: %d\n", cipher_key_len);
	flow_dump("    key: ", cipher_key_buf, cipher_key_len);

	/* format master header word */
	/* Do not set the next bit even though the datasheet says to */
	spuh = (struct SPUHEADER *)ptr;
	ptr += sizeof(struct SPUHEADER);

	mh_bits = (SPU_CRYPTO_OPERATION_GENERIC << MH_OPCODE_SHIFT);
	mh_bits |= (1 << MH_SCTX_PRES_SHIFT);
	mh_bits |= (1 << MH_BDESC_PRES_SHIFT);
	mh_bits |= (1 << MH_BD_PRES_SHIFT);
	spuh->mh.headers.bits = htonl(mh_bits);

	spuh->emh = 0;

	/* Format sctx word 0 (protocol_bits) */
	sctx_words = 3;		/* size so far in words, update later */

	/* copy the encryption keys in the SAD entry */
	if (cipher_alg) {
		if (cipher_key_len) {
			ptr += cipher_key_len;
			sctx_words += cipher_key_len / 4;
		}

		/* if encrypting then set IV size, use
		   SCTX IV unless no IV given here */
		if (cipher_iv_len) {
			/* Use SCTX IV */
			ecf_bits |= 1 << SCTX_IV_SHIFT;
			ptr += cipher_iv_len;
			sctx_words += cipher_iv_len / 4;
		}
	}

	/* Set the crypto parameters in the cipher.flags */
	cipher_bits |= cipher_alg << CIPHER_ALG_SHIFT;
	cipher_bits |= cipher_mode << CIPHER_MODE_SHIFT;
	cipher_bits |= cipher_type << CIPHER_TYPE_SHIFT;

	/* copy the encryption keys in the SAD entry */
	if (cipher_alg && cipher_key_len)
		memcpy(spuh + 1, cipher_key_buf, cipher_key_len);

	/* write in the total sctx length now that we know it */
	protocol_bits |= sctx_words;

	/* Endian adjust the SCTX */
	spuh->sa.protocol.bits = htonl(protocol_bits);

	/* Endian adjust the SCTX */
	spuh->sa.cipher.bits = htonl(cipher_bits);
	spuh->sa.ecf.bits = htonl(ecf_bits);

	packet_dump("  SPU request header: ", spu_hdr,
		    sizeof(struct SPUHEADER));

	return sizeof(struct SPUHEADER) + cipher_key_len + cipher_iv_len +
			sizeof(struct BDESC_HEADER) + sizeof(struct BD_HEADER);
}

/*
 * Finish building a SPU request message header for a block cipher request.
 * Assumes much of the header was already filled in at setkey() time in
 * spu_cipher_req_init().
 *
 * Inputs:
 *   spu_hdr         - Start of the request message header (MH field)
 *   spu_req_hdr_len - Length in bytes of the SPU request header
 *   isInbound       - 0 encrypt, 1 decrypt
 *   cipher_alg      - the encryption algorithm
 *   cipher_type     - for RC4, whether INIT or UPDT
 *   cipher_key_buf  - init() fills in the encryption key. For RC4, when
 *                     submitting a request for a non-first chunk, we use
 *                     the 260-byte SUPDT field from the previous response
 *                     as the key. update_key is true for this case. Unused
 *                     in all other cases.
 *   cipher_key_len  - length of key in cipher_key_buf, in bytes
 *   update_key      - if true, rewrite the cipher key in SCTX
 *   cipher_iv_buf   - cipher IV to write to SCTX
 *   cipher_iv_len   - length of IV in bytes
 *   data_size       - length of the data in the BD field
 *
 */
void spu_cipher_req_finish(u8 *spu_hdr,
			   u16 spu_req_hdr_len,
			   unsigned isInbound,
			   enum spu_cipher_alg cipher_alg,
			   enum spu_cipher_type cipher_type,
			   u8 *cipher_key_buf,
			   unsigned cipher_key_len,
			   bool update_key,
			   u8 *cipher_iv_buf,
			   unsigned cipher_iv_len,
			   unsigned data_size)
{
	struct SPUHEADER *spuh;
	struct BDESC_HEADER *bdesc;
	struct BD_HEADER *bd;
	u8 *bdesc_ptr = spu_hdr + spu_req_hdr_len -
		(sizeof(struct BD_HEADER) + sizeof(struct BDESC_HEADER));

	u32 cipher_bits;

	flow_log("%s()\n", __func__);
	flow_log(" in: %u\n", isInbound);
	flow_log(" cipher alg: %u, cipher_type: %u\n", cipher_alg, cipher_type);
	if (update_key) {
		flow_log(" cipher key len: %u\n", cipher_key_len);
		flow_dump("  key: ", cipher_key_buf, cipher_key_len);
	}
	flow_log(" iv len: %d\n", cipher_iv_len);
	flow_dump("    iv: ", cipher_iv_buf, cipher_iv_len);
	flow_log(" data_size: %u\n", data_size);

	/* format master header word */
	/* Do not set the next bit even though the datasheet says to */
	spuh = (struct SPUHEADER *) spu_hdr;

	/* cipher_bits was initialized at setkey time */
	cipher_bits = ntohl(spuh->sa.cipher.bits);

	/* Format sctx word 1 (cipher_bits) */
	if (isInbound)
		cipher_bits |= 1 << CIPHER_INBOUND_SHIFT;
	else
		cipher_bits &= ~(1 << CIPHER_INBOUND_SHIFT);

	/* update encryption key for RC4 on non-first chunk */
	if (update_key) {
		spuh->sa.cipher.flags.cryptoType = cipher_type;
		memcpy(spuh + 1, cipher_key_buf, cipher_key_len);
	}

	if (cipher_alg && cipher_iv_buf && cipher_iv_len)
		/* cipher iv provided so put it in here */
		memcpy(bdesc_ptr - cipher_iv_len, cipher_iv_buf, cipher_iv_len);

	spuh->sa.cipher.bits = htonl(cipher_bits);

	/* === create the BDESC section === */
	bdesc = (struct BDESC_HEADER *) bdesc_ptr;
	bdesc->offsetMAC = 0;
	bdesc->lengthMAC = 0;
	bdesc->offsetCrypto = 0;
	bdesc->lengthCrypto = htons(data_size);
	bdesc->offsetICV = 0;
	bdesc->offsetIV = 0;

	/* === no MFM section === */

	/* === create the BD section === */
	/* add the BD header */
	bd = (struct BD_HEADER *) (bdesc_ptr + sizeof(struct BDESC_HEADER));
	bd->size = htons(data_size);
	bd->PrevLength = 0;

	packet_dump("  SPU request header: ", spu_hdr, spu_req_hdr_len);
}

/*
 * Create pad bytes at the end of the data. There may be three forms of
 * pad:
 *  1. GCM pad - for GCM mode ciphers, pad to 16-byte alignment
 *  2. hash pad - pad to a block length, with 0x80 data terminator and
 *                size at the end
 *  3. STAT pad - to ensure the STAT field is 4-byte aligned
 *
 * Inputs:
 *   pad_start      - Start of buffer where pad bytes are to be written
 *   gcm_padding    - length of GCM padding, in bytes
 *   hash_pad_len   - Number of bytes of padding extend data to full block
 *   auth_alg       - authentication algorithm
 *   total_sent     - length inserted at end of hash pad
 *   status_padding - Number of bytes of padding to align STATUS word
 */
void spu_request_pad(u8 *pad_start,
		     u32 gcm_ccm_padding,
		     u32 hash_pad_len,
		     enum hash_alg auth_alg,
		     unsigned total_sent,
		     u32 status_padding)
{
	u8 *ptr = pad_start;

	/* fix data alignent for GCM/CCM */
	if (gcm_ccm_padding > 0) {
		flow_log("  GCM: padding to 16 byte alignment: %u bytes\n",
			 gcm_ccm_padding);
		memset(ptr, 0, gcm_ccm_padding);
		ptr += gcm_ccm_padding;
	}

	if (hash_pad_len > 0) {
		/* clear the padding section */
		memset(ptr, 0, hash_pad_len);

		/* terminate the data */
		*ptr = 0x80;
		ptr += (hash_pad_len - sizeof(uint64_t));

		/* add the size at the end as required per alg */
		if (auth_alg == HASH_ALG_MD5)
			*(uint64_t *) ptr = cpu_to_le64((u64) total_sent * 8);
		else		/* SHA1, SHA2-224, SHA2-256 */
			*(uint64_t *) ptr = cpu_to_be64((u64) total_sent * 8);
		ptr += sizeof(uint64_t);
	}

	/* pad to a 4byte alignment for STAT */
	if (status_padding > 0) {
		flow_log("  STAT: padding to 4 byte alignment: %u bytes\n",
			 status_padding);

		memset(ptr, 0, status_padding);
		ptr += status_padding;
	}
}

/**
 * spu_ccm_update_iv() - Update the IV as per the requirements for CCM mode.
 *
 * @digestsize: Digest size of this request
 * @iv_len: lenght of iv
 * @iv_buf: (pointer to) IV buffer
 * @assoclen: Length of AAD data
 * @chunksize: length of input data to be sent in this req
 * @is_encrypt: true if this is an output/encrypt operation
 * @is_esp: true if this is an ESP / RFC4309 operation
 *
 * Note that both SPU-M and SPU2 require similar IV changes, so no need for
 * separate functions for the two variants.  (Only difference between the two
 * is the printing of a warning message.)
 */
void spu_ccm_update_iv(unsigned int digestsize,
		       int iv_len,
		       char *iv_buf,
		       unsigned int assoclen,
		       unsigned int chunksize,
		       bool is_encrypt,
		       bool is_esp)
{
	u8 L;		/* L from CCM algorithm, length of plaintext data */
	u8 mprime;	/* M' from CCM algo, (M - 2) / 2, where M=authsize */
	u8 adata;

	if (iv_len != CCM_AES_IV_SIZE) {
		pr_err("%s(): Invalid IV len %d for CCM mode, should be %d\n",
			__func__, iv_len, CCM_AES_IV_SIZE);
		return;
	}

	/* IV needs to be formatted as follows:
	 *
	 * |          Byte 0               | Bytes 1 - N | Bytes (N+1) - 15 |
	 * | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 | Bits 7 - 0  |    Bits 7 - 0    |
	 * | 0 |Ad?|(M - 2) / 2|   L - 1   |    Nonce    | Plaintext Length |
	 *
	 * Ad? = 1 if AAD present, 0 if not present
	 * M = size of auth field, 8, 12, or 16 bytes
	 * L = Size of Plaintext Length field; Nonce size = 15 - L
	 *
	 * It appears that the crypto API already expects the L-1 portion
	 * to be set in the first byte of the IV, which implicitly determines
	 * the nonce size, and also fills in the nonce.  But the other bits
	 * in byte 0 as well as the plaintext length need to be filled in.
	 *
	 * In rfc4309/esp mode, L is not already in the supplied IV and
	 * we need to fill it in, as well as move the IV data to be after
	 * the salt
	 */
	if (is_esp) {
		L = CCM_ESP_L_VALUE;  /* RFC4309 has fixed L */
	}
	else {
		/* L' = plaintext length - 1 so Plaintext length is L' + 1 */
		L = ((iv_buf[0] & CCM_B0_L_PRIME) >> CCM_B0_L_PRIME_SHIFT) + 1;
	}

	mprime = (digestsize - 2) >> 1;  /* M' = (M - 2) / 2 */
	adata = (assoclen > 0);  /* adata = 1 if any associated data */

	iv_buf[0] = (adata << CCM_B0_ADATA_SHIFT) |
	            (mprime << CCM_B0_M_PRIME_SHIFT) |
	            ((L - 1) << CCM_B0_L_PRIME_SHIFT);

	/* Nonce is already filled in by crypto API, as is 15 - L bytes */

	/* Don't include digest in plaintext size when decrypting */
	if (!is_encrypt) {
		chunksize -= digestsize;
	}

	/* Fill in length of plaintext, formatted to be L bytes long */
	format_value_ccm(chunksize, &iv_buf[15 - L + 1], L);
	flow_dump("ccm update: ", iv_buf, CCM_AES_IV_SIZE);
}

