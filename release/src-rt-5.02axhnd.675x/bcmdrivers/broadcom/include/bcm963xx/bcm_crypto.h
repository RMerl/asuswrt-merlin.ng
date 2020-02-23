/*
  <:label-BRCM:2012:NONE:standard

  :>
*/

/**********************************************************************
 *  
 *  bcm_crypto.h       
 *
 *  Author:  Brian Nay (brian.nay@broadcom.com)
 *  
 *********************************************************************  
 *
 *  Copyright 2011
 *  Broadcom Corporation. All rights reserved.
 *  
 *  This software is furnished under license and may be used and 
 *  copied only in accordance with the following terms and 
 *  conditions.  Subject to these conditions, you may download, 
 *  copy, install, use, modify and distribute modified or unmodified 
 *  copies of this software in source and/or binary form.  No title 
 *  or ownership is transferred hereby.
 *  
 *  1) Any source code used, modified or distributed must reproduce 
 *     and retain this copyright notice and list of conditions 
 *     as they appear in the source file.
 *  
 *  2) No right is granted to use any trade name, trademark, or 
 *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
 *     name may not be used to endorse or promote products derived 
 *     from this software without the prior written permission of 
 *     Broadcom Corporation.
 *  
 *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
 *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
 *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
 *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
 *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
 *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
 *     THE POSSIBILITY OF SUCH DAMAGE.
 ********************************************************************* 
 */
#ifndef BCM_CRYPTO_H_
#define BCM_CRYPTO_H_

#define RK_LEN  		44
#define CIPHER_BLK_LEN   	16
#define CIPHER_KEY_LEN   	16
#define CIPHER_IV_LEN   	16
#define ENCRYPTED_EK_DATA_LEN 	32
#define ENCRYPTED_IV_DATA_LEN 	32

static const uint8_t nTe[256] =
{
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5,
    0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0,
    0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc,
    0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a,
    0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0,
    0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b,
    0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85,
    0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5,
    0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17,
    0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88,
    0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c,
    0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9,
    0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6,
    0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e,
    0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94,
    0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68,
    0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const uint32_t Td[256] =
{
    0xf4a75051, 0x4165537e, 0x17a4c31a, 0x275e963a, 0xab6bcb3b, 0x9d45f11f, 0xfa58abac, 0xe303934b,
    0x30fa5520, 0x766df6ad, 0xcc769188, 0x024c25f5, 0xe5d7fc4f, 0x2acbd7c5, 0x35448026, 0x62a38fb5,
    0xb15a49de, 0xba1b6725, 0xea0e9845, 0xfec0e15d, 0x2f7502c3, 0x4cf01281, 0x4697a38d, 0xd3f9c66b,
    0x8f5fe703, 0x929c9515, 0x6d7aebbf, 0x5259da95, 0xbe832dd4, 0x7421d358, 0xe0692949, 0xc9c8448e,
    0xc2896a75, 0x8e7978f4, 0x583e6b99, 0xb971dd27, 0xe14fb6be, 0x88ad17f0, 0x20ac66c9, 0xce3ab47d,
    0xdf4a1863, 0x1a3182e5, 0x51336097, 0x537f4562, 0x6477e0b1, 0x6bae84bb, 0x81a01cfe, 0x082b94f9,
    0x48685870, 0x45fd198f, 0xde6c8794, 0x7bf8b752, 0x73d323ab, 0x4b02e272, 0x1f8f57e3, 0x55ab2a66,
    0xeb2807b2, 0xb5c2032f, 0xc57b9a86, 0x3708a5d3, 0x2887f230, 0xbfa5b223, 0x036aba02, 0x16825ced,
    0xcf1c2b8a, 0x79b492a7, 0x07f2f0f3, 0x69e2a14e, 0xdaf4cd65, 0x05bed506, 0x34621fd1, 0xa6fe8ac4,
    0x2e539d34, 0xf355a0a2, 0x8ae13205, 0xf6eb75a4, 0x83ec390b, 0x60efaa40, 0x719f065e, 0x6e1051bd,
    0x218af93e, 0xdd063d96, 0x3e05aedd, 0xe6bd464d, 0x548db591, 0xc45d0571, 0x06d46f04, 0x5015ff60,
    0x98fb2419, 0xbde997d6, 0x4043cc89, 0xd99e7767, 0xe842bdb0, 0x898b8807, 0x195b38e7, 0xc8eedb79,
    0x7c0a47a1, 0x420fe97c, 0x841ec9f8, 0x00000000, 0x80868309, 0x2bed4832, 0x1170ac1e, 0x5a724e6c,
    0x0efffbfd, 0x8538560f, 0xaed51e3d, 0x2d392736, 0x0fd9640a, 0x5ca62168, 0x5b54d19b, 0x362e3a24,
    0x0a67b10c, 0x57e70f93, 0xee96d2b4, 0x9b919e1b, 0xc0c54f80, 0xdc20a261, 0x774b695a, 0x121a161c,
    0x93ba0ae2, 0xa02ae5c0, 0x22e0433c, 0x1b171d12, 0x090d0b0e, 0x8bc7adf2, 0xb6a8b92d, 0x1ea9c814,
    0xf1198557, 0x75074caf, 0x99ddbbee, 0x7f60fda3, 0x01269ff7, 0x72f5bc5c, 0x663bc544, 0xfb7e345b,
    0x4329768b, 0x23c6dccb, 0xedfc68b6, 0xe4f163b8, 0x31dccad7, 0x63851042, 0x97224013, 0xc6112084,
    0x4a247d85, 0xbb3df8d2, 0xf93211ae, 0x29a16dc7, 0x9e2f4b1d, 0xb230f3dc, 0x8652ec0d, 0xc1e3d077,
    0xb3166c2b, 0x70b999a9, 0x9448fa11, 0xe9642247, 0xfc8cc4a8, 0xf03f1aa0, 0x7d2cd856, 0x3390ef22,
    0x494ec787, 0x38d1c1d9, 0xcaa2fe8c, 0xd40b3698, 0xf581cfa6, 0x7ade28a5, 0xb78e26da, 0xadbfa43f,
    0x3a9de42c, 0x78920d50, 0x5fcc9b6a, 0x7e466254, 0x8d13c2f6, 0xd8b8e890, 0x39f75e2e, 0xc3aff582,
    0x5d80be9f, 0xd0937c69, 0xd52da96f, 0x2512b3cf, 0xac993bc8, 0x187da710, 0x9c636ee8, 0x3bbb7bdb,
    0x267809cd, 0x5918f46e, 0x9ab701ec, 0x4f9aa883, 0x956e65e6, 0xffe67eaa, 0xbccf0821, 0x15e8e6ef,
    0xe79bd9ba, 0x6f36ce4a, 0x9f09d4ea, 0xb07cd629, 0xa4b2af31, 0x3f23312a, 0xa59430c6, 0xa266c035,
    0x4ebc3774, 0x82caa6fc, 0x90d0b0e0, 0xa7d81533, 0x04984af1, 0xecdaf741, 0xcd500e7f, 0x91f62f17,
    0x4dd68d76, 0xefb04d43, 0xaa4d54cc, 0x9604dfe4, 0xd1b5e39e, 0x6a881b4c, 0x2c1fb8c1, 0x65517f46,
    0x5eea049d, 0x8c355d01, 0x877473fa, 0x0b412efb, 0x671d5ab3, 0xdbd25292, 0x105633e9, 0xd647136d,
    0xd7618c9a, 0xa10c7a37, 0xf8148e59, 0x133c89eb, 0xa927eece, 0x61c935b7, 0x1ce5ede1, 0x47b13c7a,
    0xd2df599c, 0xf2733f55, 0x14ce7918, 0xc737bf73, 0xf7cdea53, 0xfdaa5b5f, 0x3d6f14df, 0x44db8678,
    0xaff381ca, 0x68c43eb9, 0x24342c38, 0xa3405fc2, 0x1dc37216, 0xe2250cbc, 0x3c498b28, 0x0d9541ff,
    0xa8017139, 0x0cb3de08, 0xb4e49cd8, 0x56c19064, 0xcb84617b, 0x32b670d5, 0x6c5c7448, 0xb85742d0
};

static const uint8_t nTd[256] =
{
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38,
    0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87,
    0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d,
    0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2,
    0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16,
    0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda,
    0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a,
    0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02,
    0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea,
    0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85,
    0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89,
    0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20,
    0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31,
    0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d,
    0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0,
    0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26,
    0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};


/************************************************************************
 *
 *  Local functions
 *  
 ************************************************************************/

/**********************************************************************
 *  aesExpandCipherKey()
 *  
 *  Input parameters: 
 *        rk  - Pointer to where the expanded round key will be placed
 *      pKey  - Pointer to where the 16 byte key can be found
 *              (CIPHER_KEY_LEN in length)
 **********************************************************************/
static inline void aesExpandCipherKey(uint32_t *rk, const unsigned char *key)
{
    int i,j;
    uint32_t nTeElem, rcn;
    const unsigned char *pk = key;

    /* Load up aes key into the first four 32-bit spots of the expanded round key */
    for (i = 0; i < (CIPHER_KEY_LEN/sizeof(uint32_t)); i++)
    {
        rk[i] = 0;
        for (j = 0; j < sizeof(uint32_t); j++)
        {
            rk[i] <<= 8;
            rk[i] |= (uint32_t)pk[j];
        }
        pk += sizeof(uint32_t);
    }

    /* Calculate the last fourty 32-bit spots of the expanded round key doing it */
    /* four spots at a time and based on what the previous four spots are set to */
    for (i = (CIPHER_KEY_LEN/sizeof(uint32_t)); i < RK_LEN; i+= 4 )
    {
        rk[i] = rk[i-4];
        for (j = 0; j < 32; j+=8)
        {
            nTeElem = nTe[(rk[i-1] >> j) & 0xff];
            if (j != 24) 
                nTeElem <<= (j+8);

            rk[i] ^= nTeElem;
        }

        if (i == (RK_LEN-4))
            rcn = 0x36000000;
        else if (i == (RK_LEN-8))
            rcn = 0x1b000000;
        else
            rcn = 1 << ((i/4)+23);

        rk[i] ^= rcn;
     
        for (j = 1; j < sizeof(uint32_t); j++)
            rk[i+j] = rk[i+j-4] ^ rk[i+j-1];
    }
}

/**********************************************************************
 *  aesInvertRoundKeys()
 *  
 *  Input parameters: 
 *        rk  - Pointer to the expanded round key
 **********************************************************************/
static inline void aesInvertRoundKeys(uint32_t *rk)
{
    int i, j, k;
    uint32_t temp;

    for (i = 0, j = (RK_LEN-4); i < j; i += 4, j -= 4)
    {
        for (k = 0; k < 4; k++)
        {
            temp = rk[i + k];
            rk[i + k] = rk[j + k];
            rk[j + k] = temp;
        }
    }
}

/**********************************************************************
 *  aesInverseTransform()
 *  
 *  Input parameters: 
 *        rk  - Pointer to the expanded round key
 **********************************************************************/
static inline void aesInverseTransform(uint32_t *rk)
{
    int i, j;
    uint8_t nTeElem;
    uint32_t TdXor, TdTemp;

    /* Don't do first four or last four round keys */ 
    for (i = 4; i < (RK_LEN-4); i++)
    {
        TdXor = 0;
        for (j = 0; j < 32; j+=8)
        {
            nTeElem = nTe[(rk[i] >> j) & 0xff];
            TdTemp  = Td[nTeElem] >> (32-j);
            TdTemp |= Td[nTeElem] << j;
            TdXor ^= TdTemp;
        }
        rk[i] = TdXor;
    }   
}

/**********************************************************************
 *  aesInitDecrypt()
 *  
 *  Input parameters: 
 *        rk  - Pointer to the expanded round key
 *      pKey  - Pointer to where the 16 byte key can be found
 *              (CIPHER_KEY_LEN in length)
 **********************************************************************/
static inline void aesInitDecrypt(uint32_t *rk, const unsigned char *key)
{
    /* expand the cipher key */
    aesExpandCipherKey(rk, key);

    /* invert order of round keys */
    aesInvertRoundKeys(rk);

    /* 128 bit key so perform 10 rounds of inverse transform  */
    /* to the round keys except the first and last round keys */
    aesInverseTransform(rk);
}

/**********************************************************************
 *  aesCompleteDecrypt()
 *  
 *  Input parameters: 
 *        rk   - Pointer to the round keys
 *  ciphertext - Pointer to a block of encrypted data
 *  plaintext  - Pointer to where the decrypted data shall be placed
 *              
 **********************************************************************/
static inline void aesCompleteDecrypt(const uint32_t *rk,
                                      const unsigned char ciphertext[CIPHER_BLK_LEN], 
                                      unsigned char plaintext[CIPHER_BLK_LEN])
{
    const unsigned char *pc = ciphertext;
    unsigned char *pt = plaintext;

    int i, j, k, n;
    uint8_t sIdx[4][4] = {{0,3,2,1},{1,0,3,2},{2,1,0,3},{3,2,1,0}};
    uint32_t s[4],t[4], TdRaw, TdElem;

    for (i = 0; i < 4; i++)
    {
        s[i] = rk[i];
        for (j = 0, k = 3; j < 32; j+=8, k--)
            s[i] ^= ((uint32_t)pc[k] << j);

        pc+=4;
    }

    /* first nine rounds */
    for (i = 0; i < 5; i++)
    {
        rk += 4;
        for (j = 0; j < 4; j++)
        {
            t[j] = rk[j];
            for (k = 0, n = 3; k < 32; k+=8, n-- )
            {
                TdRaw = Td[(s[sIdx[j][n]] >> k) & 0xff];
                TdElem  = TdRaw >> (32-k);
                TdElem |= TdRaw << k;
                t[j] ^= TdElem;
            } 
        }

        rk += 4;
        if (i < 4)
        {
            for (j = 0; j < 4; j++)
            {
                s[j] = rk[j];
                for (k = 0, n = 3; k < 32; k+=8, n-- )
                {
                    TdRaw = Td[(t[sIdx[j][n]] >> k) & 0xff];
                    TdElem  = TdRaw >> (32-k);
                    TdElem |= TdRaw << k;
                    s[j] ^= TdElem;
                }
            }
        }
    }
  
    /* 10th final round */
    for (i = 0; i < 4; i++)
    {
        s[i] = rk[i];
        for (k = 0, n = 3; k < 32; k+=8, n-- )
            s[i] ^= ((uint32_t)(nTd[(t[sIdx[i][n]] >> k) & 0xff])) << k;

        for (k = 0, n = 3; k < 32; k+=8, n-- )
            pt[n] = (unsigned char)((s[i]) >> k);

        pt+=4;
    }
}


/************************************************************************
 *
 *  API
 *  
 ************************************************************************/

/************************************************************************
 *  bcm_aes_decrypt(void)
 *  
 *  Input parameters: 
 *  	   pDest - Pointer to where the decrypt content should be placed
 *  	   pSrc  - Pointer to where the encrypted content can be found
 *  	   pKey  - Pointer to where the 16 byte key can be found
 *  	           (CIPHER_KEY_LEN in length)
 *  	   len   - Length of encrypted content
 *  	   pIv   - Pointer to where the initialization vector is stored
 *  	           (CIPHER_BLK_LEN in length)
 *  	   
 *  Return value:
 *  	   none
 *
 ************************************************************************/
static inline void bcm_aes_decrypt(unsigned char *pDest, unsigned char *pSrc,
                                   unsigned char *pKey, uint32_t len,
                                   unsigned char *pIv)
{
    int			offset, i;
    uint32_t 	        rk[RK_LEN];
    unsigned char        temp[CIPHER_BLK_LEN];

    /* Populate rk structure */
    aesInitDecrypt(rk, pKey);

    for( offset = 0; offset < len; offset += CIPHER_BLK_LEN )
    {
        memcpy((void *)temp, (void *)(pSrc+offset), CIPHER_BLK_LEN );
        aesCompleteDecrypt(rk, temp, pDest);

        for( i = 0; i < CIPHER_BLK_LEN; i++ )
            pDest[i] = (unsigned char)( pDest[i] ^ pIv[i] );

        memcpy( pIv, pSrc+offset, CIPHER_BLK_LEN );
        pDest += CIPHER_BLK_LEN;
    }

    memset((void *)rk, 0 , RK_LEN);
}

#endif /* BCM_CRYPTO_H_ */
