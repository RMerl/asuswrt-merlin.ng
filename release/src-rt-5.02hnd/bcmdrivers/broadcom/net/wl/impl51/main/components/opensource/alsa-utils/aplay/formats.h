#ifndef FORMATS_H
#define FORMATS_H		1

#include <endian.h>
#include <byteswap.h>

/* Definitions for .VOC files */

#define VOC_MAGIC_STRING	"Creative Voice File\x1A"
#define VOC_ACTUAL_VERSION	0x010A
#define VOC_SAMPLESIZE		8

#define VOC_MODE_MONO		0
#define VOC_MODE_STEREO		1

#define VOC_DATALEN(bp)		((u_long)(bp->datalen) | \
                         	((u_long)(bp->datalen_m) << 8) | \
                         	((u_long)(bp->datalen_h) << 16) )

typedef struct voc_header {
	u_char magic[20];	/* must be MAGIC_STRING */
	u_short headerlen;	/* Headerlength, should be 0x1A */
	u_short version;	/* VOC-file version */
	u_short coded_ver;	/* 0x1233-version */
} VocHeader;

typedef struct voc_blocktype {
	u_char type;
	u_char datalen;		/* low-byte    */
	u_char datalen_m;	/* medium-byte */
	u_char datalen_h;	/* high-byte   */
} VocBlockType;

typedef struct voc_voice_data {
	u_char tc;
	u_char pack;
} VocVoiceData;

typedef struct voc_ext_block {
	u_short tc;
	u_char pack;
	u_char mode;
} VocExtBlock;

/* Definitions for Microsoft WAVE format */

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define COMPOSE_ID(a,b,c,d)	((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#define LE_SHORT(v)		(v)
#define LE_INT(v)		(v)
#define BE_SHORT(v)		bswap_16(v)
#define BE_INT(v)		bswap_32(v)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define COMPOSE_ID(a,b,c,d)	((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#define LE_SHORT(v)		bswap_16(v)
#define LE_INT(v)		bswap_32(v)
#define BE_SHORT(v)		(v)
#define BE_INT(v)		(v)
#else
#error "Wrong endian"
#endif

#define WAV_RIFF		COMPOSE_ID('R','I','F','F')
#define WAV_WAVE		COMPOSE_ID('W','A','V','E')
#define WAV_FMT			COMPOSE_ID('f','m','t',' ')
#define WAV_DATA		COMPOSE_ID('d','a','t','a')

/* WAVE fmt block constants from Microsoft mmreg.h header */
#define WAV_FMT_PCM             0x0001
#define WAV_FMT_IEEE_FLOAT      0x0003
#define WAV_FMT_DOLBY_AC3_SPDIF 0x0092
#define WAV_FMT_EXTENSIBLE      0xfffe

/* Used with WAV_FMT_EXTENSIBLE format */
#define WAV_GUID_TAG		"\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71"

/* it's in chunks like .voc and AMIGA iff, but my source say there
   are in only in this combination, so I combined them in one header;
   it works on all WAVE-file I have
 */
typedef struct {
	u_int magic;		/* 'RIFF' */
	u_int length;		/* filelen */
	u_int type;		/* 'WAVE' */
} WaveHeader;

typedef struct {
	u_short format;		/* see WAV_FMT_* */
	u_short channels;
	u_int sample_fq;	/* frequence of sample */
	u_int byte_p_sec;
	u_short byte_p_spl;	/* samplesize; 1 or 2 bytes */
	u_short bit_p_spl;	/* 8, 12 or 16 bit */
} WaveFmtBody;

typedef struct {
	WaveFmtBody format;
	u_short ext_size;
	u_short bit_p_spl;
	u_int channel_mask;
	u_short guid_format;	/* WAV_FMT_* */
	u_char guid_tag[14];	/* WAV_GUID_TAG */
} WaveFmtExtensibleBody;

typedef struct {
	u_int type;		/* 'data' */
	u_int length;		/* samplecount */
} WaveChunkHeader;

/* Definitions for Sparc .au header */

#define AU_MAGIC		COMPOSE_ID('.','s','n','d')

#define AU_FMT_ULAW		1
#define AU_FMT_LIN8		2
#define AU_FMT_LIN16		3

typedef struct au_header {
	u_int magic;		/* '.snd' */
	u_int hdr_size;		/* size of header (min 24) */
	u_int data_size;	/* size of data */
	u_int encoding;		/* see to AU_FMT_XXXX */
	u_int sample_rate;	/* sample rate */
	u_int channels;		/* number of channels (voices) */
} AuHeader;

#endif				/* FORMATS */
