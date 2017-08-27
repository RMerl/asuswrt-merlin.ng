/*
 * Copyright (C) 2011 Sansar Choinyambuu
 * Copyright (C) 2012-2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup pts pts
 * @{ @ingroup libimcv
 */

#ifndef PTS_H_
#define PTS_H_

typedef struct pts_t pts_t;

#include "pts_error.h"
#include "pts_proto_caps.h"
#include "pts_meas_algo.h"
#include "pts_file_meas.h"
#include "pts_file_meta.h"
#include "pts_dh_group.h"
#include "pts_pcr.h"
#include "pts_req_func_comp_evid.h"
#include "pts_simple_evid_final.h"
#include "components/pts_comp_func_name.h"

#include <library.h>
#include <collections/linked_list.h>

/**
 * UTF-8 encoding of the character used to delimiter the filename
 */
#define SOLIDUS_UTF				0x2F
#define REVERSE_SOLIDUS_UTF		0x5C

/**
 * PCR indices used for measurements of various functional components
 */
#define PCR_BIOS						0
#define PCR_PLATFORM_EXT				1
#define PCR_MOTHERBOARD					1
#define PCR_OPTION_ROMS					2
#define PCR_IPL							4

#define PCR_TBOOT_POLICY				17
#define PCR_TBOOT_MLE					18

#define PCR_TGRUB_MBR_STAGE1			4
#define PCR_TGRUB_STAGE2_PART1			8
#define PCR_TGRUB_STAGE2_PART2			9
#define PCR_TGRUB_CMD_LINE_ARGS			12
#define PCR_TGRUB_CHECKFILE				13
#define PCR_TGRUB_LOADED_FILES			14

#define PCR_DEBUG						16

/**
 * Length of the generated nonce used for calculation of shared secret
 */
#define ASSESSMENT_SECRET_LEN	20

/**
 * Length of the TPM_QUOTE_INFO structure, TPM Spec 1.2
 */
#define TPM_QUOTE_INFO_LEN		48

/**
 * Hashing algorithm used by tboot and trustedGRUB
 */
#define TRUSTED_HASH_ALGO		PTS_MEAS_ALGO_SHA1

/**
 * Class implementing the TCG Platform Trust Service (PTS)
 *
 */
struct pts_t {

	/**
	 * Get PTS Protocol Capabilities
	 *
	 * @return					Protocol capabilities flags
	 */
	pts_proto_caps_flag_t (*get_proto_caps)(pts_t *this);

	/**
	 * Set PTS Protocol Capabilities
	 *
	 * @param flags				Protocol capabilities flags
	 */
	void (*set_proto_caps)(pts_t *this, pts_proto_caps_flag_t flags);

	/**
	 * Get PTS Measurement Algorithm
	 *
	 * @return					PTS measurement algorithm
	 */
	pts_meas_algorithms_t (*get_meas_algorithm)(pts_t *this);

	/**
	 * Set PTS Measurement Algorithm
	 *
	 * @param algorithm			PTS measurement algorithm
	 */
	void (*set_meas_algorithm)(pts_t *this, pts_meas_algorithms_t algorithm);

	/**
	 * Get DH Hash Algorithm
	 *
	 * @return					DH hash algorithm
	 */
	pts_meas_algorithms_t (*get_dh_hash_algorithm)(pts_t *this);

	/**
	 * Set DH Hash Algorithm
	 *
	 * @param algorithm			DH hash algorithm
	 */
	void (*set_dh_hash_algorithm)(pts_t *this, pts_meas_algorithms_t algorithm);

	/**
	 * Create PTS Diffie-Hellman object and nonce
	 *
	 * @param group				PTS DH group
	 * @param nonce_len			Nonce length
	 * @return					TRUE if creation was successful
	 *
	 */
	bool (*create_dh_nonce)(pts_t *this, pts_dh_group_t group, int nonce_len);

	/**
	 * Get my Diffie-Hellman public value
	 *
	 * @param value				My public DH value
	 * @param nonce				My DH nonce
	 */
	void (*get_my_public_value)(pts_t *this, chunk_t *value, chunk_t *nonce);

	/**
	 * Set peer Diffie.Hellman public value
	 *
	 * @param value				Peer public DH value
	 * @param nonce				Peer DH nonce
	 */
	void (*set_peer_public_value) (pts_t *this, chunk_t value, chunk_t nonce);

	/**
	 * Calculates assessment secret to be used for TPM Quote as ExternalData
	 *
	 * @return					TRUE unless both DH public values
	 *							and nonces are set
	 */
	bool (*calculate_secret) (pts_t *this);

	/**
	 * Get primary key of platform entry in database
	 *
	 * @return					Platform and OS info
	 */
	int (*get_platform_id)(pts_t *this);

	/**
	 * Set primary key of platform entry in database
	 *
	 * @param pid				Primary key of platform entry in database
	 */
	void (*set_platform_id)(pts_t *this, int pid);

	/**
	 * Get TPM 1.2 Version Info
	 *
	 * @param info				chunk containing a TPM_CAP_VERSION_INFO struct
	 * @return					TRUE if TPM Version Info available
	 */
	bool (*get_tpm_version_info)(pts_t *this, chunk_t *info);

	/**
	 * Set TPM 1.2 Version Info
	 *
	 * @param info				chunk containing a TPM_CAP_VERSION_INFO struct
	 */
	void (*set_tpm_version_info)(pts_t *this, chunk_t info);

	/**
	 * Get Attestation Identity Certificate or Public Key
	 *
	 * @return					AIK Certificate or Public Key
	 */
	certificate_t* (*get_aik)(pts_t *this);

	/**
	 * Set Attestation Identity Certificate or Public Key
	 *
	 * @param aik				AIK Certificate or Public Key
	 * @param aik_id			Primary key referencing AIK in database
	 */
	void (*set_aik)(pts_t *this, certificate_t *aik, int aik_id);

	/**
	 * Get primary key referencing AIK in database
	 *
	 * @return					Primary key referencing AIK in database
	 */
	int (*get_aik_id)(pts_t *this);

	/**
	 * Check whether path is valid file/directory on filesystem
	 *
	 * @param path				Absolute path
	 * @param error_code		Output variable for PTS error code
	 * @return					TRUE if path is valid or file/directory
	 *							doesn't exist or path is invalid
	 * 						FALSE if local error occurred within stat function
	 */
	bool (*is_path_valid)(pts_t *this, char *path, pts_error_code_t *error_code);

	/**
	 * Obtain file metadata
	 *
	 * @param pathname			Absolute pathname of file/directory
	 * @param is_dir			TRUE if directory contents are requested
	 * @return					PTS File Metadata or NULL if FAILED
	 */
	pts_file_meta_t* (*get_metadata)(pts_t *this, char *pathname, bool is_dir);

	/**
	 * Reads given PCR value and returns it
	 * Expects owner secret to be WELL_KNOWN_SECRET
	 *
	 * @param pcr_num			Number of PCR to read
	 * @param pcr_value			Chunk to save pcr read output
	 * @return					NULL in case of TSS error, PCR value otherwise
	 */
	bool (*read_pcr)(pts_t *this, u_int32_t pcr_num, chunk_t *pcr_value);

	/**
	 * Extends given PCR with given value
	 * Expects owner secret to be WELL_KNOWN_SECRET
	 *
	 * @param pcr_num			Number of PCR to extend
	 * @param input				Value to extend
	 * @param output			Chunk to save PCR value after extension
	 * @return					FALSE in case of TSS error, TRUE otherwise
	 */
	bool (*extend_pcr)(pts_t *this, u_int32_t pcr_num, chunk_t input,
					   chunk_t *output);

	/**
	 * Quote over PCR's
	 * Expects owner and SRK secret to be WELL_KNOWN_SECRET and no password set for AIK
	 *
	 * @param use_quote2		Version of the Quote function to be used
	 * @param pcr_comp			Chunk to save PCR composite structure
	 * @param quote_sig			Chunk to save quote operation output
	 *							without external data (anti-replay protection)
	 * @return					FALSE in case of TSS error, TRUE otherwise
	 */
	 bool (*quote_tpm)(pts_t *this, bool use_quote2, chunk_t *pcr_comp,
													 chunk_t *quote_sig);

	/**
	 * Get the shadow PCR set
	 *
	 * @return					shadow PCR set
	 */
	pts_pcr_t* (*get_pcrs)(pts_t *this);

	 /**
	 * Constructs and returns TPM Quote Info structure expected from IMC
	 *
	 * @param use_quote2		Version of the TPM_QUOTE_INFO to be constructed
	 * @param use_ver_info		Version info is concatenated to TPM_QUOTE_INFO2
	 * @param comp_hash_algo	Composite Hash Algorithm
	 * @param pcr_comp			Output variable to store PCR Composite
	 * @param quote_info		Output variable to store TPM Quote Info
	 * @return					FALSE in case of any error, TRUE otherwise
	 */
	 bool (*get_quote_info)(pts_t *this, bool use_quote2, bool ver_info_included,
							pts_meas_algorithms_t comp_hash_algo,
							chunk_t *pcr_comp, chunk_t *quote_info);

	 /**
	 * Constructs and returns PCR Quote Digest structure expected from IMC
	 *
	 * @param data				Calculated TPM Quote Digest
	 * @param signature			TPM Quote Signature received from IMC
	 * @return					FALSE if signature is not verified
	 */
	 bool (*verify_quote_signature)(pts_t *this, chunk_t data, chunk_t signature);

	/**
	 * Destroys a pts_t object.
	 */
	void (*destroy)(pts_t *this);

};

/**
 * Creates an pts_t object
 *
 * @param is_imc			TRUE if running on an IMC
 */
pts_t* pts_create(bool is_imc);

#endif /** PTS_H_ @}*/
