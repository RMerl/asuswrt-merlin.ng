/*
 * Copyright (C) 2008-2009 Martin Willi
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
 * @defgroup eap_aka_3gpp2_functions eap_aka_3gpp2_functions
 * @{ @ingroup eap_aka_3gpp2
 */

#ifndef EAP_AKA_3GPP2_FUNCTIONS_H_
#define EAP_AKA_3GPP2_FUNCTIONS_H_

#include <simaka_manager.h>

#define AKA_SQN_LEN		 6
#define AKA_K_LEN		16
#define AKA_MAC_LEN 	 8
#define AKA_AK_LEN		 6
#define AKA_AMF_LEN		 2
#define AKA_FMK_LEN		 4

typedef struct eap_aka_3gpp2_functions_t eap_aka_3gpp2_functions_t;

/**
 * f1-f5(), f1*() and f5*() functions from the 3GPP2 (S.S0055) standard.
 */
struct eap_aka_3gpp2_functions_t {

	/**
	 * Calculate MAC from RAND, SQN, AMF using K.
	 *
	 * @param k		secret key K
	 * @param rand	random value rand
	 * @param sqn	sequence number
	 * @param amf	authentication management field
	 * @param mac	buffer receiving mac MAC
	 * @return		TRUE if calculations successful
	 */
	bool (*f1)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char sqn[AKA_SQN_LEN],
				u_char amf[AKA_AMF_LEN], u_char mac[AKA_MAC_LEN]);

	/**
	 * Calculate MACS from RAND, SQN, AMF using K
	 *
	 * @param k		secret key K
	 * @param rand	random value RAND
	 * @param sqn	sequence number
	 * @param amf	authentication management field
	 * @param macs	buffer receiving resynchronization mac MACS
	 * @return		TRUE if calculations successful
	 */
	bool (*f1star)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char sqn[AKA_SQN_LEN],
				u_char amf[AKA_AMF_LEN], u_char macs[AKA_MAC_LEN]);

	/**
	 * Calculate RES from RAND using K
	 *
	 * @param k		secret key K
	 * @param rand	random value RAND
	 * @param res	buffer receiving result RES, uses full 128 bit
	 * @return		TRUE if calculations successful
	 */
	bool (*f2)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char res[AKA_RES_MAX]);
	/**
	 * Calculate CK from RAND using K
	 *
	 * @param k		secret key K
	 * @param rand	random value RAND
	 * @param macs	buffer receiving encryption key CK
	 * @return		TRUE if calculations successful
	 */
	bool (*f3)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char ck[AKA_CK_LEN]);
	/**
	 * Calculate IK from RAND using K
	 *
	 * @param k		secret key K
	 * @param rand	random value RAND
	 * @param macs	buffer receiving integrity key IK
	 * @return		TRUE if calculations successful
	 */
	bool (*f4)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char ik[AKA_IK_LEN]);
	/**
	 * Calculate AK from a RAND using K
	 *
	 * @param k		secret key K
	 * @param rand	random value RAND
	 * @param macs	buffer receiving anonymity key AK
	 * @return		TRUE if calculations successful
	 */
	bool (*f5)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char ak[AKA_AK_LEN]);
	/**
	 * Calculate AKS from a RAND using K
	 *
	 * @param k		secret key K
	 * @param rand	random value RAND
	 * @param macs	buffer receiving resynchronization anonymity key AKS
	 * @return		TRUE if calculations successful
	 */
	bool (*f5star)(eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char aks[AKA_AK_LEN]);

	/**
	 * Destroy a eap_aka_3gpp2_functions_t.
	 */
	void (*destroy)(eap_aka_3gpp2_functions_t *this);
};

/**
 * Create a eap_aka_3gpp2_functions instance.
 *
 * @return			function set, NULL on error
 */
eap_aka_3gpp2_functions_t *eap_aka_3gpp2_functions_create();

#endif /** EAP_AKA_3GPP2_FUNCTIONS_H_ @}*/
