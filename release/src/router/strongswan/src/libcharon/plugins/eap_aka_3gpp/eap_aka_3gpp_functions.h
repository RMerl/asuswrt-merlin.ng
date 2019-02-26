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
/*
 * Copyright (C) 2015 Thomas Strangert
 * Polystar System AB, Sweden
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup eap_aka_3gpp_functions eap_aka_3gpp_functions
 * @{ @ingroup eap_aka_3gpp
 */

#ifndef EAP_AKA_3GPP_FUNCTIONS_H_
#define EAP_AKA_3GPP_FUNCTIONS_H_

#include <credentials/keys/shared_key.h>
#include <simaka_manager.h>
#include "eap_aka_3gpp_plugin.h"

#define AKA_SQN_LEN		 6
#define AKA_K_LEN		16
#define AKA_OPC_LEN		16
#define AKA_MAC_LEN		 8
#define AKA_AK_LEN		 6
#define AKA_AMF_LEN		 2
#define AKA_RES_LEN		 8

typedef struct eap_aka_3gpp_functions_t eap_aka_3gpp_functions_t;

/**
 * Get a shared key K and OPc of a particular user from the credential database.
 *
 * @param id			user identity
 * @param[out] k		(16 byte) scratchpad to receive secret key K
 * @param[out] opc		(16 byte) scratchpad to receive operator variant key
 *						derivate OPc
 */
bool eap_aka_3gpp_get_k_opc(identification_t *id, uint8_t k[AKA_K_LEN],
							uint8_t opc[AKA_OPC_LEN]);

/**
 * Get SQN using current time. Only used when creating/initializing
 * an eap_aka_3gpp_card_t or eap_aka_3gpp_provider_t object.
 *
 * @param offset		time offset to add to current time to avoid initial
 *						SQN resync
 * @param[out] sqn		(6 byte) scratchpad to receive generated SQN
 */
void eap_aka_3gpp_get_sqn(uint8_t sqn[AKA_SQN_LEN], int offset);

/**
 * f1, f1*(), f2345() and f5*() functions from 3GPP as specified
 * in the TS 35.205, .206, .207, .208 standards.
 */
struct eap_aka_3gpp_functions_t {

	/**
	 * f1 : Calculate MAC-A from RAND, SQN, AMF using K and OPc
	 *
	 * @param k			(128 bit) secret key K
	 * @param opc		(128 bit) operator variant key derivate OPc
	 * @param rand		(128 bit) random value RAND
	 * @param sqn		 (48 bit) sequence number SQN
	 * @param amf		 (16 bit) authentication management field AMF
	 * @param[out] maca	 (64 bit) scratchpad to receive network auth code MAC-A
	 * @return				TRUE if calculations successful
	 */
	bool (*f1)(eap_aka_3gpp_functions_t *this,
			const uint8_t k[AKA_K_LEN], const uint8_t opc[AKA_OPC_LEN],
			const uint8_t rand[AKA_RAND_LEN], const uint8_t sqn[AKA_SQN_LEN],
			const uint8_t amf[AKA_AMF_LEN],
			uint8_t maca[AKA_MAC_LEN]);


	/**
	 * f1* : Calculate MAC-S from RAND, SQN, AMF using K and OPc
	 *
	 * @param k			(128 bit) secret key K
	 * @param opc		(128 bit) operator variant key derivate OPc
	 * @param rand		(128 bit) random value RAND
	 * @param sqn		 (48 bit) sequence number SQN
	 * @param amf		 (16 bit) authentication management field AMF
	 * @param[out] macs	 (64 bit) scratchpad to receive resync auth code MAC-S
	 * @return				TRUE if calculations successful
	 */
	bool (*f1star)(eap_aka_3gpp_functions_t *this,
			const uint8_t k[AKA_K_LEN], const uint8_t opc[AKA_OPC_LEN],
			const uint8_t rand[AKA_RAND_LEN], const uint8_t sqn[AKA_SQN_LEN],
			const uint8_t amf[AKA_AMF_LEN],
			uint8_t macs[AKA_MAC_LEN]);

	/**
	 * f2345 : Do f2, f3, f4 and f5 in a single scoop, where:
	 * f2 : Calculates RES from RAND using K and OPc
	 * f3 : Calculates CK  from RAND using K and OPc
	 * f4 : Calculates IK  from RAND using K and OPc
	 * f5 : Calculates AK  from RAND using K and OPc
	 *
	 * @param k			(128 bit) secret key K
	 * @param opc		(128 bit) operator variant key derivate OPc
	 * @param rand		(128 bit) random value RAND
	 * @param[out] res	 (64 bit) scratchpad to receive signed response RES
	 * @param[out] ck	(128 bit) scratchpad to receive encryption key CK
	 * @param[out] ik	(128 bit) scratchpad to receive integrity key IK
	 * @param[out] ak	 (48 bit) scratchpad to receive anonymity key AK
	 * @return				TRUE if calculations successful
	 */
	bool (*f2345)(eap_aka_3gpp_functions_t *this,
				  const uint8_t k[AKA_K_LEN], const uint8_t opc[AKA_OPC_LEN],
				  const uint8_t rand[AKA_RAND_LEN],
				  uint8_t res[AKA_RES_LEN], uint8_t ck[AKA_CK_LEN],
				  uint8_t ik[AKA_IK_LEN], uint8_t ak[AKA_AK_LEN]);


	/**
	 * f5* : Calculates resync AKS from RAND using K and OPc
	 *
	 * @param k			(128 bit) secret key K
	 * @param opc		(128 bit) operator variant key derivate OPc
	 * @param rand		(128 bit) random value RAND
	 * @param[out] aks	 (48 bit) scratchpad to receive resync anonymity key AKS
	 * @return				TRUE if calculations successful
	 */
	bool (*f5star)(eap_aka_3gpp_functions_t *this,
				   const uint8_t k[AKA_K_LEN], const uint8_t opc[AKA_OPC_LEN],
				   const uint8_t rand[AKA_RAND_LEN],
				   uint8_t aks[AKA_AK_LEN]);

	/**
	 * Destroy a eap_aka_3gpp_functions_t.
	 */
	void (*destroy)(eap_aka_3gpp_functions_t *this);
};

/**
 * Create a eap_aka_3gpp_functions instance.
 *
 * @return	function set, NULL on error
 */
eap_aka_3gpp_functions_t *eap_aka_3gpp_functions_create();

#endif /** EAP_AKA_3GPP_FUNCTIONS_H_ @}*/
