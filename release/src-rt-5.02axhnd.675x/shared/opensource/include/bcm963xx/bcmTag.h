/*
    Copyright 2000-2010 Broadcom Corporation

<:label-BRCM:2012:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/                       

//**************************************************************************************
// File Name  : bcmTag.h
//
// Description: add tag with validation system to the firmware image file to be uploaded
//              via http
//
// Created    : 02/28/2002  seanl
//**************************************************************************************

#ifndef _BCMTAG_H_
#define _BCMTAG_H_


#define BCM_SIG_1   "Broadcom Corporation"
#define BCM_SIG_2   "ver. 2.0"          // was "firmware version 2.0" now it is split 6 char out for chip id.

#define BCM_TAG_VER        "7"
#define BCM_TAG_VER_DTB    7           // image that support dtb starting from this version

// file tag (head) structure all is in clear text except validationTokens (crc, md5, sha1, etc). Total: 128 unsigned chars
#define TAG_LEN         256
#define TAG_VER_LEN     4
#define SIG_LEN         20
#define SIG_LEN_2       14   // Original second SIG = 20 is now devided into 14 for SIG_LEN_2 and 6 for CHIP_ID
#define CHIP_ID_LEN     6	
#define IMAGE_LEN       10
#define ADDRESS_LEN     12
#define FLAG_LEN        2
#define TOKEN_LEN       20
#define BOARD_ID_LEN    16
#define IMAGE_VER_LEN   32
#define RESERVED_LEN    (TAG_LEN - TAG_VER_LEN - SIG_LEN - SIG_LEN_2 - CHIP_ID_LEN - BOARD_ID_LEN - \
                        (5*IMAGE_LEN) - (4*ADDRESS_LEN) - (3*FLAG_LEN) - (2*TOKEN_LEN) - IMAGE_VER_LEN -\
                        sizeof(unsigned int))  /* RESERVED_LEN = 16 */


// TAG for downloadable image (kernel plus file system)
typedef struct _FILE_TAG
{
    char tagVersion[TAG_VER_LEN];       // tag version.  Will be 2 here.
    char signiture_1[SIG_LEN];          // text line for company info
    char signiture_2[SIG_LEN_2];        // additional info (can be version number)
    char chipId[CHIP_ID_LEN];           // chip id 
    char boardId[BOARD_ID_LEN];         // board id
    char bigEndian[FLAG_LEN];           // if = 1 - big, = 0 - little endia of the host
    char totalImageLen[IMAGE_LEN];      // the sum of all the following length
    char cfeAddress[ADDRESS_LEN];       // if non zero, cfe starting address
    char cfeLen[IMAGE_LEN];             // if non zero, cfe size in clear ASCII text.
    char rootfsAddress[ADDRESS_LEN];    // if non zero, filesystem starting address
    char rootfsLen[IMAGE_LEN];          // if non zero, filesystem size in clear ASCII text.
    union {
        char kernelAddress[ADDRESS_LEN];    // if non zero, kernel starting address
        char bootfsAddress[ADDRESS_LEN];    // if non zero, eMMC bootfs starting address
    };
    union {
        char kernelLen[IMAGE_LEN];          // if non zero, kernel size in clear ASCII text.
        char bootfsLen[IMAGE_LEN];          // if non zero, eMMC bootfs size in clear ASCII text.
    };
    char imageSequence[FLAG_LEN * 2];   // incrments everytime an image is flashed
    char imageVersion[IMAGE_VER_LEN];   // image version
    union {
        char dtbAddress[ADDRESS_LEN];       // if non zero, device tree blob starting address
        char mdataAddress[ADDRESS_LEN];     // if non zero, metadata blob starting address
    };
    union {
        char dtbLen[IMAGE_LEN];         // if non zero, device tree blob size in clear ASCII text.
        char mdataLen[IMAGE_LEN];       // if non zero, metadata blob size in clear ASCII text.
    };
    unsigned int imageFlags;            // image flags, same as wfiFlags. Use as 32 bit integer
    char reserved[RESERVED_LEN];        // reserved for later use
    char imageValidationToken[TOKEN_LEN];// image validation token - can be crc, md5, sha;  for
                                        // now will be 4 unsigned char crc
    char tagValidationToken[TOKEN_LEN]; // validation token for tag(from signiture_1 to end of // mageValidationToken)
} FILE_TAG, *PFILE_TAG;

/* Whole flash image TAG definitions. */
#define WFI_VERSION             0x00005732
#define WFI_ANY_VERS_MASK       0x0000ff00
#define WFI_ANY_VERS            0x00005700
#define WFI_VERSION_NAND_1MB_DATA 0x00005731
#define WFI_NOR_FLASH           1
#define WFI_NANDTYPE_FLASH_MIN  WFI_NAND16_FLASH
#define WFI_NAND16_FLASH        2
#define WFI_NAND128_FLASH       3
#define WFI_NAND256_FLASH       4
#define WFI_NAND512_FLASH       5
#define WFI_NAND1024_FLASH      6
#define WFI_NAND2048_FLASH      7
#define WFI_NANDTYPE_FLASH_MAX  WFI_NAND2048_FLASH

#define WFI_NANDTYPE_TO_BKSIZE(wfiFlashType)  (((wfiFlashType)==2) ? 16:128<<((wfiFlashType)-3))

/* WFI Flags bit definitions */
#define WFI_FLAG_HAS_PMC            0x1
#define WFI_FLAG_SUPPORTS_BTRM      0x2
#define WFI_FLAG_DDR_TYPE_SHIFT     2
#define WFI_FLAG_DDR_TYPE_MASK      (0x7<<WFI_FLAG_DDR_TYPE_SHIFT)
#define WFI_FLAG_DDR_TYPE_NONE      (0x0<<WFI_FLAG_DDR_TYPE_SHIFT)  /* DDR, DDR2 or don't care */
#define WFI_FLAG_DDR_TYPE_DDR3      (0x1<<WFI_FLAG_DDR_TYPE_SHIFT)
#define WFI_FLAG_DDR_TYPE_DDR4      (0x2<<WFI_FLAG_DDR_TYPE_SHIFT)

/* TAG at end of whole flash ".w" image.  Size must be TOKEN_LEN. */
typedef struct _WFI_TAG
{
    unsigned int wfiCrc;
    unsigned int wfiVersion;
    unsigned int wfiChipId;
    unsigned int wfiFlashType;
    unsigned int wfiFlags;
} WFI_TAG, *PWFI_TAG;

#define COMBOIMG_MAGIC_NUM_LEN 8 //Bytes
#define COMBOIMG_MAGIC_NUM "BRCMCMBO"
#define COMBOIMG_HEADER_VERSION 1

/* 
 * Combo image header definition.
 * This header is always stored big endian. A little endian target must byte
 * swap before parsing. This way, a combo image can be sent to mixed endian
 * targets.
 */
typedef struct 
{
    /* A unique magic number that identifies the combo image header. */
    unsigned char magic_num[COMBOIMG_MAGIC_NUM_LEN];
    /* 
     * Total combo header length. 
     * Note: it may be forced to an erase block size in future versions.
     */
    unsigned int header_len;
    /* Total combo header CRC. */
    unsigned int header_crc;
    unsigned int header_version;
    /*
     * The range of applicable image versions. If the recipient finds its
     * running image version is out of the range, the combo image is not
     * suitable for that recipient. Need further definition.
     */
    unsigned int version_min;
    unsigned int version_max;
    /* Unique per combo image. */
    unsigned int combo_image_revision;
    /* Number of images bundled. */
    unsigned int image_count;
    /* Bitwise flags place holder. */
    unsigned int header_flags;
    /* Jump over the extended header to the first individual image header. */
    unsigned int next_tag_offset;
    /* TLV extended fields. If the CPE parser does not know it - ignore. */
    unsigned int extended_combo_header;
} Comboimg_header_tag;

/* Individual image header definition. */
typedef struct
{
    /* Restrict image to specific board ID, empty string means 'any'. */
    char board_id[BOARD_ID_LEN];
    /* Example 0x00006848 for 6848. */
    unsigned int chip_id;
    /* Individual image size. */
    unsigned int image_len;
    /* From stream start. */
    unsigned int image_offset;
    /* Bitwise flags place holder. */
    unsigned int image_flags;
    /* Jump over the extended header to the next individual image header. */
    unsigned int next_tag_offset;
    /* TLV extended fields. If the CPE parser does not know it - ignore. */
    unsigned int extended_image_header;
} Comboimg_individual_img_tag; 


#define CRC32_INIT_VALUE 0xffffffff /* Initial CRC32 checksum value */
#define CRC_LEN 4

// only included if for bcmTag.exe program
#ifdef BCMTAG_EXE_USE

/*
Sources :
sourceware.org/svn/prelink/trunk/src/makecrc.c
sourceware.org/svn/prelink/trunk/src/crc32.c

Table computed with the following function "adler_init" by Mark Adler,
which had the following comment:

Generate a table for a byte-wise 32-bit CRC calculation on the polynomial:
x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1.

Polynomials over GF(2) are represented in binary, one bit per coefficient,
with the lowest powers in the most significant bit.  Then adding polynomials
is just exclusive-or, and multiplying a polynomial by x is a right shift by
one.  If we call the above polynomial p, and represent a byte as the
polynomial q, also with the lowest power in the most significant bit (so the
byte 0xb1 is the polynomial x^7+x^3+x+1), then the CRC is (q*x^32) mod p,
where a mod b means the remainder after dividing a by b.

This calculation is done using the shift-register method of multiplying and
taking the remainder.  The register is initialized to zero, and for each
incoming bit, x^32 is added mod p to the register if the bit is a one (where
x^32 mod p is p+x^32 = x^26+...+1), and the register is multiplied mod p by
x (which is shifting right by one and adding x^32 mod p if the bit shifted
out is a one).  We start with the highest power (least significant bit) of
q and repeat for all eight bits of q.

The table is simply the CRC of all possible eight bit values.  This is all
the information needed to generate CRC's on data a byte at a time for all
combinations of CRC register values and incoming bytes.

void adler_init()
{
  uint32_t c;      // crc shift register
  uint32_t e;      // polynomial exclusive-or pattern
  int i;           // counter for all possible eight bit values
  int k;           // byte being shifted into crc apparatus

  // terms of polynomial defining this crc (except x^32):
  static int p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};

  // Make exclusive-or pattern from polynomial (0xedb88320)
  e = 0;
  for (i = 0; i < sizeof(p)/sizeof(int); i++) {
    e |= 1L << (31 - p[i]);
  }

  // Compute table of CRC's
  for (i = 1; i < 256; i++) {
    c = i;
    // The idea to initialize the register with the byte instead of
    // zero was stolen from Haruhiko Okumura's ar002
    for (k = 8; k; k--) {
      c = c & 1 ? (c >> 1) ^ e : c >> 1;
    }
    crc32_table[i] = c;
  }
}
*/

static unsigned int Crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

#endif // BCMTAG_USE


#endif // _BCMTAG_H_

