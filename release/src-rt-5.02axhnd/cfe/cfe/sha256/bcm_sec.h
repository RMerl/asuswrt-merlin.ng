#ifndef SEC_H
#define SEC_H

/*
* <:copyright-BRCM:2018:DUAL/GPL:standard
* 
*    Copyright (c) 2018 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :>
*/
#include "bcm_secPrivate.h"

// #include <stdint.h>

// Selectively publish some constants
#define SEC_S_DIGEST    SHA256_S_DIGEST8
#define SEC_S_MODULUS   RSA_S_MODULUS8
#define SEC_S_SIGNATURE RSA_S_MODULUS8

// Status codes returned by Sec_verify. These are only really of any use when 
// debugging. SEC_S_SUCCESS is the expected return value.
typedef enum
{
    SEC_S_SUCCESS,      // The function has completed successfully
    SEC_E_BADMESLEN,    // The expected message is larger than we can sign using SHA-256
    SEC_E_BADSIGLEN,    // The signature block provided is the wrong size
    SEC_E_BADDIGLEN,    // The SHA-256 digest we were passed was the wrong size
    SEC_E_BADPUBLEN,    // The public key modulus provided is the wrong size
    SEC_E_BADEMLEN,     // The encoded message is too small to hold the required information 
    SEC_E_BADSEEDLEN,   // The seed length is not the length of a SHA-256 digest
    SEC_E_DIVFAIL,      // An MP division operation has failed
    SEC_E_BADMASKLEN,   // We cannot generate a mask of the requested length 
    SEC_E_TSHORT,       // The message representative is too short to hold the required information
    SEC_E_TENDNOTBC,    // The message representative did not end with the required byte 0xbc
    SEC_E_BADLEADER,    // The recovered message MB' is not null (but should have been)
    SEC_E_MISMATCH      // The computed hash does not match the recovered hash
} SecStatus;


/** Compute a SHA-256 digest for a buffer 
 *
 * The SHA-256 digest of the data buffer provided is computed. The result is
 * stored in the space pointer to by the third argument. That space is expected
 * to be 32-bit aligned so that the result can be stored 32 bits at a time.
 * 
 * @param image_ptr     the start of the data to be processed 
 * @param bufferLength  the length of the data in bits
 * @param digest        location to hold digest (of size SEC_S_DIGEST)
 */
void Sec_sha256(uint8_t const* image_ptr, int bufferLength, uint32_t* digest);


/** Initialise a SHA-256 digest before use
 * 
 * Prepare a SHA-256 digest for use, setting it to the initial hash value H(0).
 * This function must be called before the digest can be passed to the
 * Sec_sha256Update and Sec_sha256Final functions. (It is not needed before a
 * call to Sec_sha256, however).
 * 
 * @param digest    the digest to be initialised
 */
void Sec_sha256Init(uint32_t* digest);


/** Update an existing SHA-256 digest using a new block of data
 * 
 * Given a SHA-256 digest which has already been initialised (and possibly 
 * already updated with data), hash into it as many 64-byte blocks of data as we 
 * can from the block of data provided. The function returns the number of bits
 * at the end of the block that were unused - this value will always be less 
 * that 512.
 *  
 * This restriction allows us to assume that on entry we are about to start a
 * new block of 512 bits which will speed up the hashing significantly.
 * 
 * This function can be called as many times as required to process the bulk of
 * a message. Sec_sha256Final must be called at the end to complete the 
 * operation, even if there are no bits left over.
 * 
 * @param digest        the digest to be updated
 * @param buffer        the data to be hashed into the digest
 * @param bufferLength  the length of the data in bits
 * @return              the number of bits left unused at the end
 */
int Sec_sha256Update(uint32_t* digest, uint8_t const* buffer, int bufferLength);


/** Conclude the calculation of a SHA-256 digest
 * 
 * Given a digest that has been initialised using Sec_sha256Init and possibly
 * updated with calls to Sec_sha256Update, conclude the calculation by
 * 
 * a) Hashing all of the data passed to this function
 * b) Adding the final padding and buffer length into the hash
 * 
 * This function processes all of the data passed to it. There is no requirement
 * that it consist of a multiple of 512 bits.
 * 
 * Once this function has been called, the digest cannot be updated again but it
 * can be reused by calling Sec_sha256Init to start a new digest.
 * 
 * @param digest        the digest to be updated
 * @param buffer        the data to be hashed into the digest
 * @param bufferLength  the total length of data that has been hashed (in all
 *                      previous calls to Sec_sha256Init and this call) in bits 
 * @param bitsRemaining the length of data in buffer to be hashed in bits
 */
void Sec_sha256Final(uint32_t* digest, uint8_t const* buffer, int bufferLength, int bitsRemaining);


/** Verify the RSASSA-PSS signature for an object
 * 
 * Given a signature, the modulus of the key alleged to have been used for 
 * signing and the object to which the signature is alleged to correspond, check
 * that the signature is indeed the result of signing the object with the key
 * using the RSASSA-PSS algorithm.
 * 
 * We assume that the public exponent is F4 and expect the signature and key
 * modulus to be SEC_S_SIGNATURE and SEC_S_MODULUS bits respectively.
 * 
 * @param signature     the signature
 * @param siglen        the length of the signature in bytes
 * @param publickey     the modulus of the key used to create the signature
 * @param publen        the length of the modulus in bytes
 * @param object        the non-recoverable (appended) object that was signed
 * @param objlen        the length of the signed object in bytes
 * @return              SEC_S_SUCCESS iff the signature was verified
 */
SecStatus Sec_verify(uint32_t const* signature, int siglen, 
                     uint32_t const* publickey, int publen, 
                     uint8_t const*  object,    int objlen);


/** Verify the RSASSA-PSS signature for a digest
 * 
 * Given a signature, the modulus of the key alleged to have been used for 
 * signing and the SHA-256 digest of an object to which the signature is alleged 
 * to correspond, check  that the signature is indeed the result of signing the 
 * object with the key using the RSASSA-PSS algorithm.
 * 
 * We assume that the public exponent is F4 and expect the signature and key
 * modulus to be SEC_S_SIGNATURE and SEC_S_MODULUS bits respectively.
 * 
 * 
 * @param signature     The signature encrypted by a private key
 * @param siglen        The length of the signature in octets
 * @param publickey     The modulus of the public key
 * @param publen        The length of the modulus
 * @param digest        The SHA-256 digest of the object
 * @param diglen        The length of the digest in octets
 * @return              true iff the signature signed the object
 */
SecStatus Sec_verifyDigest(uint32_t const* signature, int siglen, 
                           uint32_t const* publickey, int publen, 
                           uint8_t const*  digest,    int diglen);

/*
* @ buf to compute digest from 
* @ size of the buffer
* @ hash to compare to
* @ returns 0 on success
*/
int sec_verify_sha256(uint8_t const * buf, int size, uint8_t const * hash);

#endif
