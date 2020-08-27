#include <stdio.h>	
#include <sys/types.h>
#include <string.h>		
#include <stdlib.h>
#include <openssl/hmac.h>
#ifndef HAVE_TYPE_FLOAT
#include <math.h>
#endif
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <pthread.h>
#include <utime.h>
#include <unistd.h>
#include "function.h"
//#include "data.h"
#include "api.h"
#include "list.h"
#include <ctype.h>
#include "mem_pool.h"



#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define WORDS_BIGENDIAN 1
#endif

#ifndef IPKG
//#include <bcmnvram.h>
#ifndef USE_TCAPI
    //#include <bcmnvram.h>  //markcool
#else
    #define WANDUCK "Wanduck_Common"
    #define AICLOUD "AiCloud_Entry"
    //#include "libtcapi.h"   // markcool
    //#include "tcapi.h"	//markcool
#endif
//#include <shutils.h> //markcool
#endif

#define WAIT_LOCAL_TIME 1000*10

int sync_local_add_file(char *parentfolder,Local *local,int i,int entryID);
int mySync(char *username,int parentid,char *localpath,NodeStack **head);


extern char username[256];
extern char password[256];
extern int server_modify;
extern char sync_path[256];
extern struct sync_item *from_server_sync_head;
extern struct sync_item *down_head;
extern struct sync_item *up_head;
extern sync_item_t up_excep_fail;
extern int pre_seq;
extern pthread_mutex_t mutex;
extern my_mutex_t my_mutex;
extern my_mutex_t wait_sleep_mutex;
extern int exit_loop;
extern int upload_only;
extern int download_only;
extern pthread_mutex_t mutex_socket;
extern sync_item_t download_only_socket_head;
extern s_tree *s_link;
extern Hb_TreeNode *DirRootNode;
extern int init_fail_item;
extern int server_sync;
extern int local_sync;
extern int has_local_socket;
extern struct asus_config cfg;

extern int server_space_full;
extern int local_space_full;
extern int IsSyncError;
int loop_max;
int usleep_time;
extern int IsNetworkUnlink;
extern Hb_SubNode *SyncNode;
extern my_mutex_t wait_server_mutex;

#ifdef IPKG
extern int disk_change;
extern int sync_disk_removed;
extern struct mounts_info_tag mounts_info;
extern struct asus_config cfg;
extern char record_token_file[256];
extern char token_filename[256];
#endif

// start : modify by markcool
extern struct asuswebstorage_conf asus_conf;
#include "uploader_config.h"
#define AICAM 1
// end : modify by markcool

struct find_number
{
    int item_num;
    int find_num;
};
char *case_conflict_name = "case conflict";


char *my_str_malloc(size_t len){
    char *s;
    s = (char *)malloc(sizeof(char)*len);
    if(s == NULL)
    {
        printf("Out of memory.\n");
        exit(1);
    }

    memset(s,'\0',sizeof(char)*len);
    return s;
}

static void *xmalloc_fatal(size_t size) {
    if (size==0) return NULL;
    fprintf(stderr, "Out of memory.");
    //exit(1);
    return NULL;
}

void *xmalloc (size_t size) {
    void *ptr = malloc (size);
    if (ptr == NULL) return xmalloc_fatal(size);
    return ptr;
}

void *xcalloc (size_t nmemb, size_t size) {
    void *ptr = calloc (nmemb, size);
    if (ptr == NULL) return xmalloc_fatal(nmemb*size);
    return ptr;
}

void *xrealloc (void *ptr, size_t size) {
    void *p = realloc (ptr, size);
    if (p == NULL) return xmalloc_fatal(size);
    return p;
}

char *xstrdup (const char *s) {
    void *ptr = xmalloc(strlen(s)+1);
    strcpy (ptr, s);
    return (char*) ptr;
}

/**
 * Base64 encode one byte
 */
char oauth_b64_encode(unsigned char u) {
    if(u < 26)  return 'A'+u;
    if(u < 52)  return 'a'+(u-26);
    if(u < 62)  return '0'+(u-52);
    if(u == 62) return '+';
    return '/';
}

/**
 * Decode a single base64 character.
 */
unsigned char oauth_b64_decode(char c) {
    if(c >= 'A' && c <= 'Z') return(c - 'A');
    if(c >= 'a' && c <= 'z') return(c - 'a' + 26);
    if(c >= '0' && c <= '9') return(c - '0' + 52);
    if(c == '+')             return 62;
    return 63;
}

/**
 * Return TRUE if 'c' is a valid base64 character, otherwise FALSE
 */
int oauth_b64_is_base64(char c) {
    if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
       (c >= '0' && c <= '9') || (c == '+')             ||
       (c == '/')             || (c == '=')) {
        return 1;
    }
    return 0;
}

/**
 * Base64 encode and return size data in 'src'. The caller must free the
 * returned string.
 *
 * @param size The size of the data in src
 * @param src The data to be base64 encode
 * @return encoded string otherwise NULL
 */
char *oauth_encode_base64(int size, const unsigned char *src) {
    int i;
    char *out, *p;

    if(!src) return NULL;
    if(!size) size= strlen((char *)src);
    out= (char*) xcalloc(sizeof(char), size*4/3+4);
    p= out;

    for(i=0; i<size; i+=3) {
        unsigned char b1=0, b2=0, b3=0, b4=0, b5=0, b6=0, b7=0;
        b1= src[i];
        if(i+1<size) b2= src[i+1];
        if(i+2<size) b3= src[i+2];

        b4= b1>>2;
        b5= ((b1&0x3)<<4)|(b2>>4);
        b6= ((b2&0xf)<<2)|(b3>>6);
        b7= b3&0x3f;

        *p++= oauth_b64_encode(b4);
        *p++= oauth_b64_encode(b5);

        if(i+1<size) *p++= oauth_b64_encode(b6);
        else *p++= '=';

        if(i+2<size) *p++= oauth_b64_encode(b7);
        else *p++= '=';
    }
    return out;
}

/**
 * Decode the base64 encoded string 'src' into the memory pointed to by
 * 'dest'.
 *
 * @param dest Pointer to memory for holding the decoded string.
 * Must be large enough to receive the decoded string.
 * @param src A base64 encoded string.
 * @return the length of the decoded string if decode
 * succeeded otherwise 0.
 */
int oauth_decode_base64(unsigned char *dest, const char *src) {
    if(src && *src) {
        unsigned char *p= dest;
        int k, l= strlen(src)+1;
        unsigned char *buf= (unsigned char*) xcalloc(sizeof(unsigned char), l);

        /* Ignore non base64 chars as per the POSIX standard */
        for(k=0, l=0; src[k]; k++) {
            if(oauth_b64_is_base64(src[k])) {
                buf[l++]= src[k];
            }
        }

        for(k=0; k<l; k+=4) {
            char c1='A', c2='A', c3='A', c4='A';
            unsigned char b1=0, b2=0, b3=0, b4=0;
            c1= buf[k];

            if(k+1<l) c2= buf[k+1];
            if(k+2<l) c3= buf[k+2];
            if(k+3<l) c4= buf[k+3];

            b1= oauth_b64_decode(c1);
            b2= oauth_b64_decode(c2);
            b3= oauth_b64_decode(c3);
            b4= oauth_b64_decode(c4);

            *p++=((b1<<2)|(b2>>4) );

            if(c3 != '=') *p++=(((b2&0xf)<<4)|(b3>>2) );
            if(c4 != '=') *p++=(((b3&0x3)<<6)|b4 );
        }
        free(buf);
        dest[p-dest]='\0';
        return(p-dest);
    }
    return 0;
}




/**
 * Escape 'string' according to RFC3986 and
 * http://oauth.net/core/1.0/#encoding_parameters.
 *
 * @param string The data to be encoded
 * @return encoded string otherwise NULL
 * The caller must free the returned string.
 */
char *oauth_url_escape(const char *string) {
    size_t alloc, newlen;
    char *ns = NULL, *testing_ptr = NULL;
    unsigned char in;
    size_t strindex=0;
    size_t length;

    if (!string) return xstrdup("");

    alloc = strlen(string)+1;
    newlen = alloc;

    ns = (char*) xmalloc(alloc);

    length = alloc-1;
    while(length--) {
        in = *string;

        switch(in){
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        case 'a': case 'b': case 'c': case 'd': case 'e':
        case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o':
        case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E':
        case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O':
        case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '_': case '~': case '.': case '-':
            ns[strindex++]=in;
            break;
        default:
            newlen += 2; /* this'll become a %XX */
            if(newlen > alloc) {
                alloc *= 2;
                testing_ptr = (char*) xrealloc(ns, alloc);
                ns = testing_ptr;
            }
            snprintf(&ns[strindex], 4, "%%%02X", in);
            strindex+=3;
            break;
        }
        string++;
    }
    ns[strindex]=0;
    return ns;
}

#ifndef ISXDIGIT
# define ISXDIGIT(x) (isxdigit((int) ((unsigned char)x)))
#endif

/**
 * Parse RFC3986 encoded 'string' back to  unescaped version.
 *
 * @param string The data to be unescaped
 * @param olen unless NULL the length of the returned string is stored there.
 * @return decoded string or NULL
 * The caller must free the returned string.
 */
char *oauth_url_unescape(const char *string, size_t *olen) {
    size_t alloc, strindex=0;
    char *ns = NULL;
    unsigned char in;
    long hex;

    if (!string) return NULL;
    alloc = strlen(string)+1;
    ns = (char*) xmalloc(alloc);

    while(--alloc > 0) {
        in = *string;
        if(('%' == in) && ISXDIGIT(string[1]) && ISXDIGIT(string[2])) {
            char hexstr[3]; // '%XX'
            hexstr[0] = string[1];
            hexstr[1] = string[2];
            hexstr[2] = 0;
            hex = strtol(hexstr, NULL, 16);
            in = (unsigned char)hex; /* hex is always < 256 */
            string+=2;
            alloc-=2;
        }
        ns[strindex++] = in;
        string++;
    }
    ns[strindex]=0;
    if(olen) *olen = strindex;
    return ns;
}

/*   HMAC-SHA1 */
char *oauth_sign_hmac_sha1_raw (const char *m, const size_t ml, const char *k, const size_t kl) {
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int resultlen = 0;

    HMAC(EVP_sha1(), k, kl,
         (unsigned char*) m, ml,
         result, &resultlen);

    return(oauth_encode_base64(resultlen, result));
}

char *oauth_sign_hmac_sha1 (const char *m, const char *k) {
    return(oauth_sign_hmac_sha1_raw (m, strlen(m), k, strlen(k)));
}

//general nonce
char *oauth_gen_nonce() {
    char *nc;
    static int rndinit = 1;
    const char *chars = "abcdefghijklmnopqrstuvwxyz"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789_";
    unsigned int max = strlen( chars );
    int i, len;

    if(rndinit) {srand(time(NULL)
#ifndef WIN32 // quick windows check.
        * getpid()
#endif
        ); rndinit=0;} // seed random number generator - FIXME: we can do better ;)

        len=15+floor(rand()*16.0/(double)RAND_MAX);
        nc = (char*) xmalloc((len+1)*sizeof(char));
        for(i=0;i<len; i++) {
            nc[i] = chars[ rand() % max ];
        }
        nc[i]='\0';
        return (nc);
    }

/****** MD5 Code *******/
typedef unsigned int u32;
//typedef unsigned long u32;

/* original code from header - function names have changed */

struct MD5Context {
    u32 buf[4];
    u32 bits[2];
    unsigned char in[64];
    unsigned char digest[16];
};

static void MD5Init(struct MD5Context *context);
static void MD5Update(struct MD5Context *context,
                      const unsigned char *buf,
                      unsigned len);
static void MD5Final(struct MD5Context *context);
static void MD5Transform(u32 buf[4], u32 const in[16]);


/* original code from C file - GNU configurised */

#ifndef WORDS_BIGENDIAN
#define byteReverse(buf, len)	/* Nothing */
#else
static void byteReverse(unsigned char *buf, unsigned longs);

/* ASM ifdef removed */

/*
 * Note: this code is harmless on little-endian machines.
 */
static void byteReverse(unsigned char *buf, unsigned longs)
{
    u32 t;
    do {
        t = (u32) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
            ((unsigned) buf[1] << 8 | buf[0]);
        *(u32 *) buf = t;
        buf += 4;
    } while (--longs);
}
#endif /* WORDS_BIGENDIAN */


/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
static void MD5Init(struct MD5Context *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
static void MD5Update(struct MD5Context *ctx,
                      const unsigned char* buf,
                      unsigned len)
{
    u32 t;

    /* Update bitcount */

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((u32) len << 3)) < t)
        ctx->bits[1]++;		/* Carry from low to high */
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if (t) {
        unsigned char *p = (unsigned char *) ctx->in + t;

        t = 64 - t;
        if (len < t) {
            memcpy(p, buf, len);
            return;
        }
        memcpy(p, buf, t);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (u32 *) ctx->in);
        buf += t;
        len -= t;
    }

    /* Process data in 64-byte chunks */

    while (len >= 64) {
        memcpy(ctx->in, buf, 64);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (u32 *) ctx->in);
        buf += 64;
        len -= 64;
    }

    /* Handle any remaining bytes of data. */

    memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */

/* Interface altered by DJB to write digest into pre-allocated context */
static void MD5Final(struct MD5Context *ctx)
{
    unsigned count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = (ctx->bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
     always at least one byte free */
    p = ctx->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if (count < 8) {
        /* Two lots of padding:  Pad the first block to 64 bytes */
        memset(p, 0, count);
        byteReverse(ctx->in, 16);
        MD5Transform(ctx->buf, (u32 *) ctx->in);

        /* Now fill the next block with 56 bytes */
        memset(ctx->in, 0, 56);
    } else {
        /* Pad block to 56 bytes */
        memset(p, 0, count - 8);
    }
    byteReverse(ctx->in, 14);

    /* Append length in bits and transform */
    ((u32 *) ctx->in)[14] = ctx->bits[0];
    ((u32 *) ctx->in)[15] = ctx->bits[1];

    MD5Transform(ctx->buf, (u32 *) ctx->in);
    byteReverse((unsigned char *) ctx->buf, 4);
    memcpy(ctx->digest, ctx->buf, 16);
    memset(ctx, 0, sizeof(ctx));	/* In case it's sensitive */
}


/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

        /*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
        static void MD5Transform(u32 buf[4], u32 const in[16])
{
    register u32 a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}


/* my code from here */

char*
        MD5_string(char *string)
{
    struct MD5Context md5;
    size_t len=strlen(string);
    char* b=NULL;
    int i;

    MD5Init(&md5);
    MD5Update(&md5, (const unsigned char*)string, len);
    MD5Final(&md5);

#define MD5_LEN 16
    b=(char*)malloc(1+(MD5_LEN<<1));
    if(!b)
        return NULL;

    for(i=0; i < MD5_LEN; i++)
        sprintf(b+(i<<1), "%02x", (unsigned int)md5.digest[i]);
    b[i<<1]='\0';

    return b;
}

/*
int get_all_folder_in_mount_path(const char *const mount_path, int *sh_num, char ***folder_list){
    DIR *pool_to_open;
    struct dirent *dp;
    char *testdir;
    char **tmp_folder_list, **tmp_folder;
    int len, i;

    pool_to_open = opendir(mount_path);
    if(pool_to_open == NULL){
        //csprintf("Can't opendir \"%s\".\n", mount_path);
        return -1;
    }

    *sh_num = 0;
    while((dp = readdir(pool_to_open)) != NULL){
        //if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
        if(dp->d_name[0] == '.')
            continue;

        if(test_if_System_folder(dp->d_name) == 1)
            continue;

        len = strlen(mount_path)+strlen("/")+strlen(dp->d_name);
        testdir = (char *)malloc(sizeof(char)*(len+1));
        if(testdir == NULL){
            closedir(pool_to_open);
            return -1;
        }
        sprintf(testdir, "%s/%s", mount_path, dp->d_name);
        testdir[len] = 0;
        if(!test_if_dir(testdir)){
            free(testdir);
            continue;
        }
        free(testdir);

        tmp_folder = (char **)malloc(sizeof(char *)*(*sh_num+1));
        if(tmp_folder == NULL){
            //csprintf("Can't malloc \"tmp_folder\".\n");

            return -1;
        }

        len = strlen(dp->d_name);
        tmp_folder[*sh_num] = (char *)malloc(sizeof(char)*(len+1));
        if(tmp_folder[*sh_num] == NULL){
            //csprintf("Can't malloc \"tmp_folder[%d]\".\n", *sh_num);
            free(tmp_folder);

            return -1;
        }
        strcpy(tmp_folder[*sh_num], dp->d_name);
        if(*sh_num != 0){
            for(i = 0; i < *sh_num; ++i)
                tmp_folder[i] = tmp_folder_list[i];

            free(tmp_folder_list);
            tmp_folder_list = tmp_folder;
        }
        else
            tmp_folder_list = tmp_folder;

        ++(*sh_num);
    }
    closedir(pool_to_open);

    *folder_list = tmp_folder_list;

    return 0;
}

int test_if_System_folder(const char *const dirname){
    char *MS_System_folder[] = {"SYSTEM VOLUME INFORMATION", "RECYCLER", "RECYCLED", NULL};
    char *Linux_System_folder[] = {"lost+found", NULL};
    int i;

    for(i = 0; MS_System_folder[i] != NULL; ++i){
        if(!upper_strcmp(dirname, MS_System_folder[i]))
            return 1;
    }

    for(i = 0; Linux_System_folder[i] != NULL; ++i){
        if(!upper_strcmp(dirname, Linux_System_folder[i]))
            return 1;
    }

    return 0;
}*/

int test_if_dir(const char *dir){
    DIR *dp = opendir(dir);

    if(dp == NULL)
        return 0;

    closedir(dp);
    return 1;
}

int upper_strcmp(const char *const str1, const char *const str2){
    int len1, len2, i;

    len1 = strlen(str1);
    len2 = strlen(str2);
    if(len1 != len2)
        return len1-len2;

    for(i = 0; i < len1; ++i){
        if(toupper(str1[i]) != toupper(str2[i]))
            return i+1;
    }

    return 0;
}

int UploadOnlyFindItem(char *display,Local *local,Browse *browse,int isfolder,int num)
{
    int i;
    char temp[NORMALSIZE];
    //printf("find display is %s,num is %d\n",display,num);

    for( i = 0 ;i < num ;i++)
    {
        memset(temp,0,sizeof(temp));
        if(isfolder) //uppdate folder
        {
            oauth_decode_base64((unsigned char *)temp,(browse->folderlist)[i]->display);

        }
        else        //update files
        {
            oauth_decode_base64((unsigned char *)temp,(browse->filelist)[i]->display);
        }

        if(strlen(temp) == 0)
        {
            handle_error(S_DECODE_BASE64_FAIL,"findItem");
            continue;
        }

        //printf(" single file is %s,temp file is %s \n",display,temp);
        if( !strcmp(display,temp))
        {
            return i;
        }

    }

    return -1;
}

int findItem(char *display,Local *local,Browse *browse,int isfolder,int isupload,int num)
{
    int i;
    //int num;
    char temp[NORMALSIZE];
    char *encode = NULL;

#if 0

    if(isfolder ==1 )      //update folder
    {
        if(isupload == 1)  //for localfile in serverfiles
            num = browse->foldernumber;
        else if(isupload == 0)
            num = local->foldernum;
    }
    else                   //update files
    {
        if(isupload == 1)  //for localfile in serverfiles
            num = browse->filenumber;
        else if(isupload == 0)
            num = local->filenum;
    }
#endif

    //printf("find display is %s,num is %d\n",display,num);

    for( i = 0 ;i < num ;i++)
    {
        if(exit_loop ==1 )
            return -2;

        memset(temp,0,sizeof(temp));
        if(isfolder) //uppdate folder
        {   if(isupload == 1)
            oauth_decode_base64((unsigned char *)temp,(browse->folderlist)[i]->display);
            else if(isupload == 0)
                //sprintf(temp,"%s",oauth_encode_base64(0,(local->folderlist)[i]->name));
                encode = oauth_encode_base64(0,(unsigned char *)(local->folderlist)[i]->name);
        }
        else        //update files
        {   if(isupload == 1)
            oauth_decode_base64((unsigned char *)temp,(browse->filelist)[i]->display);
            else if(isupload == 0)
                //sprintf(temp,"%s",oauth_encode_base64(0,(local->filelist)[i]->name));
                encode = oauth_encode_base64(0,(unsigned char *)(local->filelist)[i]->name);
        }

        if(isupload == 1)
        {
            if(strlen(temp) == 0)
            {
                handle_error(S_DECODE_BASE64_FAIL,"findItem");
                continue;
            }

            //printf(" single file is %s,temp file is %s \n",display,temp);
            if( !strcmp(display,temp))
            {
                return i;
            }
        }
        else if(isupload == 0)
        {
            if(encode == NULL)
            {
                handle_error(S_ENCODE_BASE64_FAIL,"findItem");
                continue;
            }

            if( !strcmp(display,encode))
            {
                free(encode);
                return i;
            }
            else
            {
                free(encode);
            }
        }

    }

    return -1;
}

void add_server_action_list(char *action,char *fullname,struct sync_item *head)
{
    //printf("add_server_action_list fullname = %s\n",fullname);
    //pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&my_mutex.mutex);

    add_sync_item(action,fullname,head);
    //my_mutex.ready = 1;
    /*if(download_only == 1)
    {   //my_mutex.ready ==1 ;
        pthread_cond_signal(&my_mutex.cond);    //del by alan
    }*/
    pthread_mutex_unlock(&my_mutex.mutex);
    //pthread_mutex_unlock(&mutex);

}


int my_remove_dir(const char *dir,Browse *br,int pid,int rec)
{
    printf("enter my_remove_dir,dir=%s,pid=%d,rec=%d\n",dir,pid,rec);
    int i=0,id=-1;

    if(remove(dir) ==  -1)
        return -1;

    if(!rec)
        return 0;

    Hb_SubNode *p1 = NULL,*pnode = NULL;
    pnode = get_parent_node(pid,SyncNode);
    if(pnode == NULL)
    {
        handle_error(99,"get_parent_node fail in my_remove_dir\n");
        return 0;
     }

    p1 = pnode->Child;
    if(p1 == NULL)
        return 0;
    if(br->foldernumber == 0)
    {
        id = p1->id;
    }
    else
    {
        while(p1 != NULL)
        {
            for(i=0;i<br->foldernumber;i++)
            {
                if(p1->id == br->folderlist[i]->id)
                    break;
            }
            if(i == br->foldernumber) //not find
            {
                id = p1->id;
                break;
            }
            p1 = p1->NextBrother;
        }
    }
    printf("id=%d\n",id);

    if(id >0)
        remove_node(pnode,id);

   // del_node()
}

int del_all_items(char *dir,int pid,Browse *br,int rec)
{
    if(test_if_file_up_excep_fail(dir))
        return 0;

    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(dir);
    int fail_flag = 0;
    char error_message[NORMALSIZE];
    char fullname[512];

    if(NULL == pDir)
    {
        snprintf(error_message,NORMALSIZE,"open %s fail \n",dir);
        handle_error(S_OPENDIR_FAIL,error_message);
        return -1;
    }

    while (NULL != (ent=readdir(pDir)))
    {
        //if(ent->d_name[0] == '.')
            //continue;

        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        memset(fullname,0,sizeof(fullname));
        memset(error_message,0,sizeof(error_message));

        snprintf(fullname,512,"%s/%s",dir,ent->d_name);

        if( test_if_dir(fullname) == 1 )
        {
            del_all_items(fullname,pid,br,0);
        }
        else
        {
            if(remove(fullname) == -1)
            {
                snprintf(error_message,NORMALSIZE,"remove %s fail \n",fullname);
                handle_error(S_REMOVE_LOCAL_FAIL,error_message);
                fail_flag = 1;
                continue;
            }
            else
            {
                add_server_action_list("remove",fullname,from_server_sync_head);
#if SYSTEM_LOG
                write_system_log("remove",ent->d_name);
#endif
            }

        }
    }
    closedir(pDir);

    //if(remove(dir) == -1)
    if(my_remove_dir(dir,br,pid,rec) == -1)
    {
        snprintf(error_message,NORMALSIZE,"remove %s fail \n",dir);
        handle_error(S_REMOVE_LOCAL_FAIL,error_message);
        fail_flag = 1;
    }
    else
    {
        add_server_action_list("remove",dir,from_server_sync_head);

#if SYSTEM_LOG
        write_system_log("remove",dir);
#endif

#if TREE_NODE_ENABLE
        modify_tree_node(dir,DirRootNode,DEL_TREE_NODE);
#endif
    }

    return (fail_flag == 1) ? -1 : 0 ;
}

#if 0
int test_file_if_new(char *filename)
{
    struct stat buf1,buf2;
    unsigned int c_time1,c_time2;

    if( stat(temp_file,&buf1) == -1)
    {
        printf(" %s file does not exist,not need compare\n",temp_file);
        return -1;
    }

    if( stat(filename,&buf2) == -1)
    {
        printf(" %s file does not exist",filename);
        return -1;
    }

    c_time1 = buf1.st_atime;
    c_time2 = buf2.st_atime;

    printf("%s c_time is %d,%s c_time is %d\n",temp_file,c_time1,filename,c_time2);

    return  ( c_time2 > c_time1 )  ?  1 : 0;

    /*
    if( c_time2 > c_time1 )
        return 1;
    else
        return 0;
   */
}


int test_file_if_exist(char *filename)
{
    int fd;
    int i=0;
    char ch;
    char tmp[NORMALSIZE];
    struct stat buf;

    if( stat(all_local_item_file,&buf) == -1)
    {
        printf(" %s file does not exist\n",all_local_item_file);
        return -1;
    }

    if(buf.st_size == 0)
    {
        //printf(" %s is blank\n",all_local_item_file);
        return -1;
    }

    memset(tmp, 0, sizeof(tmp));

    //printf("path  is %s\n",filename);

    if((fd = open(all_local_item_file, O_RDONLY | O_NONBLOCK)) < 0)
    {
        printf("\nread log error!\n");
        return -1;
    }
    else
    {

        while( read(fd, &ch, 1)  > 0)
        {

            if(ch == '\n')
            {
                //printf("file  is %s\n",tmp);
                if( !strcmp(tmp,filename) )
                {
                    close(fd);
                    return 1;
                }

                memset(tmp, 0, sizeof(tmp));
                i = 0;

                continue;
            }
            memcpy(tmp+i, &ch, 1);
            i++;
        }
        close(fd);
    }

    return 0;
}
#endif

unsigned int GetFile_modtime(char *localpath){
    struct stat buf;
    if( stat(localpath,&buf) == -1)
    {
        //perror("stat:");
        printf("GetFile_modtime stat error:%s file not exist\n",localpath);
        return 0;
    }
    unsigned int msec = buf.st_mtime;
    return msec;
}

int upload_only_find_diff_item(char *parentfolder,Browse *browse,Local *local,int isfolder)
{
    if(exit_loop == 1)
        return 0;

    int item_num = 0;
    int find_num = 0;
    int i = 0;
    int find = 0;
    char temp[NORMALSIZE];
    char filename[NORMALSIZE];
    char fullname[NORMALSIZE];
    int  fail_flag = 0;
    int parent_ID;
    //int entry_ID;
    char error_message[NORMALSIZE];
    //char *confilicted_name;
    //char *confilicted_filename;
    //Operateentry *oe = NULL;
    //Propfind *finds = NULL;
    int status = -10;
    int res = -10;

    memset(filename,0,sizeof(filename));
    memset(fullname,0,sizeof(fullname));

    //printf("is folder is %d\n",isfolder);

    if(isfolder)    /*sync folder*/
    {
        item_num = local->foldernum;
        find_num = browse->foldernumber;
    }
    else    /*sync file*/
    {
        item_num = local->filenum;
        find_num = browse->filenumber;
    }

    for( i= 0; i<item_num ;i++)
    {
        if(exit_loop ==1 )
            return 0;

        memset(temp,0,sizeof(temp));
        memset(error_message,0,sizeof(error_message));

        if(isfolder)
        {
            snprintf(temp,NORMALSIZE,"%s",(local->folderlist)[i]->name);
        }
        else
        {
            snprintf(temp,NORMALSIZE,"%s",(local->filelist)[i]->name);
        }

        if(strlen(temp) == 0)
        {
            handle_error(S_NAME_BLANK,"find diff item");
            continue;
        }

        find = UploadOnlyFindItem(temp,local,browse,isfolder,find_num);

        //printf("find is %d\n",find);

        //struct utimbuf tbuf;
        unsigned int local_mtime;
        unsigned int server_mtime;

        if(find == -1) //can't find item
        {
            //server_modify = 1;
            int parentID = -10;
            int entry_ID = -10;
            Createfolder *createfolder = NULL;
            //int res = -1;

            //printf("server mot exists [%s] files \n",local->filelist[i].name);
            if(isfolder)
            {
                snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,(local->folderlist)[i]->name);
#ifdef DEBUG
                printf("%s folder is new add \n",fullname);
#endif
                parentID = getParentID(parentfolder);

                if(parentID < 0 )
                {
                    snprintf(error_message,NORMALSIZE,"get parentID %s fail",fullname);
                    handle_error(S_GET_PARENTID_FAIL,error_message);
                    fail_flag = 1;
                }
                else if(parentID > 0)
                {
                    if(receve_socket)
                        return -1;

                    createfolder = createFolder(username,parentID,0,fullname);

                    if(NULL == createfolder)
                    {
                        snprintf(error_message,NORMALSIZE,"createfolder %s fail",(local->folderlist)[i]->name);
                        handle_error(S_CREATEFOLDER_FAIL,error_message);
                        fail_flag = 1;
                    }
                    else if( createfolder->status != 0 )
                    {
                        status = createfolder->status;
                        handle_error(status,"create folder");
                        my_free(createfolder);
                        res = handle_createfolder_fail_code(status,parentID,parentfolder,fullname);
                        if(res != 0)
                            fail_flag = 1;
                    }
                    else if( createfolder->status == 0 )
                    {
                        entry_ID = createfolder->id;
                        my_free(createfolder);
#ifdef DEBUG
                        //printf("entry ID is %d\n",entry_ID);
#endif
                        if( sync_all_item_uploadonly(fullname,entry_ID) == -1)
                        {
                            fail_flag = 1;
                        }
                    }
                }
            }
            else /* sync file */
            {
                snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,(local->filelist)[i]->name);
                if(test_if_file_up_excep_fail(fullname) != 1)
                {
                    printf("%s file is new add \n",fullname);
                    parentID = getParentID(parentfolder);

                    printf("parentID is %d\n",parentID);

                    if(parentID < 0)
                    {
                        snprintf(error_message,NORMALSIZE,"get %s parentID fail",fullname);
                        handle_error(S_GET_PARENTID_FAIL,error_message);
                        fail_flag = 1;
                    }
                    else if(parentID > 0)
                    {
                        if(receve_socket)
                            return -1;

                        status = uploadFile(fullname,parentID,NULL,0);
			printf("status is %d\n",status);

                        if( status != 0)
                        {

                            snprintf(error_message,NORMALSIZE,"uploadfile %s fail",fullname);
                            handle_error(S_UPLOADFILE_FAIL,error_message);
                            res = handle_upload_fail_code(status,parentID,fullname,parentfolder);
                            if(res != 0)
                                fail_flag = 1;
                        } else {
                            // start : modify by markcool
                            #ifdef AICAM
			    if( remove(fullname) != 0 ) {
			      printf("Error deleting file : %s\n",fullname);
			    } else {
			      printf("File successfully deleted : %s\n",fullname);
			    }
                            #endif
			    // start : modify by markcool
			}
                    }
                }

            }
        }
        else //find item
        {
            //printf("find same start\n");
            /*
            if(isfolder)
            {

            }
            else
            */
            if(!isfolder)
            {
                snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,(local->filelist)[i]->name);

                server_mtime = atoi((browse->filelist)[find]->attribute.lastwritetime);
                local_mtime = GetFile_modtime(fullname);

                if(server_mtime != local_mtime)
                {
                    //printf("enter confilicted name\n");
                    //printf("local time is %d,server_time is %d\n",local_mtime,server_mtime);
                    if(server_mtime - local_mtime == 1)
                    {
                        printf("local file one second\n");
                        res = update_local_file_attr(&(browse->filelist[find]->attribute),fullname);
                    }
                    else
                    {           
                        parent_ID = getParentID(parentfolder);

                        if(parent_ID == -2)
                        {
                            printf("%s has no parent!\n",parentfolder);
                            return 0;
                        }

                        /*
                        finds = checkEntryExisted(username,parent_ID,(local->filelist)[i]->name,"system.unknown");

                        if(NULL == finds)
                        {
                            printf("find prop failed\n");
                            return -1;
                        }

                        if( finds->status != 0 )
                        {
                            handle_error(finds->status,"propfind");
                            my_free(finds);
                            return -1;
                        }

                        entry_ID = finds->id;

                        confilicted_name = get_confilicted_name(fullname,isfolder);
                        confilicted_filename = confilicted_name+strlen(parentfolder)+1;

                        //printf("confilicted_filename = %s\n",confilicted_filename);
                        //printf("finds->status = %d\n",finds->status);
                        //printf("entry_ID = %d\n",entry_ID);

                        oe = renameEntry(username,entry_ID,0,confilicted_filename,isfolder);

                        my_free(finds);

                        if(NULL == oe)
                        {
                            printf("operate rename failed\n");
                            return -1;
                        }

                        if( oe->status != 0 )
                        {
                            handle_error(oe->status,"rename");
                            res = handle_rename_fail_code(oe->status,parent_ID,fullname,parentfolder,0);
                            my_free(oe);
                            return res;
                            //return -1;
                        }

                        my_free(oe);

                        write_confilicted_log(fullname,confilicted_name);
                        */

                        if(test_if_file_up_excep_fail(fullname) != 1)
                        {
                            status = uploadFile(fullname,parent_ID,NULL,browse->filelist[find]->id);
                            if( status != 0)
                            {
                                snprintf(error_message,NORMALSIZE,"uploadfile %s fail",fullname);
                                handle_error(S_UPLOADFILE_FAIL,error_message);
                                res = handle_upload_fail_code(status,parent_ID,fullname,parentfolder);
                                if(res != 0)
                                    fail_flag = 1;
                            }
                            else
                            {
#if SYSTEM_LOG
                                write_system_log("createfile",confilicted_name);
#endif
                            }
                        }
                        //my_free(confilicted_name);
                    }
               }

            }
            //printf("file is same end\n");
        }
    }
    return (fail_flag == 1) ? -1 : 0;
}

int sync_server_del_local_file(char *fullname)
{
    char error_message[NORMALSIZE];

    //printf("before del %s start fullname\n",fullname);
    if(wait_handle_socket() == -1 )
        return -1;
    if(test_if_file_up_excep_fail(fullname) != 1)
    {
        //printf("del %s start fullname\n",fullname);

        /*
        if(receve_socket == 1)
        {
            printf("error:receve_socket\n");
            return -1;
        }
        */

        /*
        while(receve_socket)
        {
            usleep(WAIT_LOCAL_TIME);
        }
        */
        //if(wait_handle_socket() == -1)
           //return -1;

        printf("%s file has del from server \n",fullname);
        if(remove(fullname) == -1)
        {
            snprintf(error_message,NORMALSIZE,"remove %s fail \n",fullname);
            handle_error(S_REMOVE_LOCAL_FAIL,error_message);
            return -1;
        }
        else
        {
            add_server_action_list("remove",fullname,from_server_sync_head);
#if TREE_NODE_ENABLE
            modify_tree_node(fullname,DirRootNode,DEL_TREE_NODE)
#endif
        }
    }

    return 0;
}

int get_find_num(Browse *browse,Local *local,int isupload,int isfolder,struct find_number *find)
{
    if(isfolder)    /*sync folder*/
    {
        if(isupload == 1)
        {
            find->item_num = local->foldernum;
            find->find_num = browse->foldernumber;
        }
        else
        {
            find->item_num = browse->foldernumber;
            //find_num = browse->foldernumber;
            find->find_num = local->foldernum;
        }
    }
    else    /*sync file*/
    {
        if(isupload == 1)
        {
            find->item_num = local->filenum;
            find->find_num = browse->filenumber;
        }
        else
        {
            find->item_num = browse->filenumber;
            find->find_num = local->filenum;
        }
    }

    return 0;
}

int get_find_name(Browse *browse,Local *local,int isupload,int isfolder,int i,char *temp)
{
    if(isfolder)
    {
        if(isupload == 1)              //server del
        {
            snprintf(temp,NORMALSIZE,"%s",(local->folderlist)[i]->name);
        }
        else if(isupload == 0)          //server add
        {
            snprintf(temp,NORMALSIZE,"%s",(browse->folderlist)[i]->display);
        }
    }
    else
    {
        if(isupload == 1)              //server del
        {
            snprintf(temp,NORMALSIZE,"%s",(local->filelist)[i]->name);
        }
        else if(isupload == 0)          //server add
        {
            //printf("broese file name is %s \n",browse->filelist[i].display);
            snprintf(temp,NORMALSIZE,"%s",(browse->filelist)[i]->display);
        }
    }

    return 0;
}

int sync_server_downloadfile(int id,char *fullname,long long int size,int ismodify,Fileattribute *attr)
{
    char error_message[NORMALSIZE];
    memset(error_message,0,sizeof(error_message));

    if( downloadFile(id,fullname,size,ismodify,attr) == -1)
    {
        snprintf(error_message,NORMALSIZE,"download %s fail \n",fullname);
        handle_error(S_DOWNLOADFILE_FAIL,error_message);
        return -1;
    }
    else
    {
        //add_server_action_list("createfile",fullname,from_server_sync_head);
#if SYSTEM_LOG
        write_system_log("createfile",fullname);
#endif
#if TREE_NODE_ENABLE
        modify_tree_node(fullname,DirRootNode,ADD_TREE_NODE);
#endif
    }

    return 0;
}



int handle_local_confilict_file(char *fullname,int isfolder)
{
    //printf("handle_local_confilict_file start\n");
    char *confilicted_name = NULL;
    char *con_name = NULL;
    char error_message[NORMALSIZE] = {0};
    //int have_same = 0;
    char tmp_name[NORMALSIZE] = {0};
    snprintf(tmp_name,NORMALSIZE,"%s",fullname);

    while(!exit_loop)
    {

        confilicted_name = get_confilicted_name(tmp_name,isfolder);

        if(confilicted_name ==  NULL)
        {
            printf("handle_local_confilict_file fail\n");
            return -1;
        }
       //printf("confilicted_name=%s\n",confilicted_name);
       if(access(confilicted_name,F_OK) == 0)
       {
           memset(tmp_name,0,sizeof(tmp_name));
           snprintf(tmp_name,NORMALSIZE,"%s",confilicted_name);
           my_free(confilicted_name);
           //have_same = 1;
       }
       else
           break;
           //have_same = 0;
      //sleep(5);
      }

    con_name = parse_name_from_path(confilicted_name);

    //printf("confilicted_name=%s,con=%s\n",confilicted_name,con_name);
    if(rename(fullname,confilicted_name) == -1)
    {
        snprintf(error_message,NORMALSIZE,"rename %s fail",fullname);
        handle_error(S_RENAME_FAIL,error_message);
        my_free(confilicted_name);
        my_free(con_name);
        return -1;
    }
    //else
    //{
        //printf("confilicted name is end\n");
#if TREE_NODE_ENABLE
        if(con_name != NULL)
            rename_update_tree(filename,con_name);
        else
        {
            printf("get con_name from %s fail\n",confilicted_name);
            continue;
        }
#endif
        //add_server_action_list("rename",fullname,from_server_sync_head); //add for Server create and modify File
        write_confilicted_log(fullname,confilicted_name);

        my_free(confilicted_name);
        my_free(con_name);

        return 0;
    //}
}

int downloadonly_server_modify(const char *parentfolder,Browse *browse,
                               int i,char *filename,int isfolder)
{
    char fullname[NORMALSIZE] = {0};
    unsigned int local_mtime;
    unsigned int server_mtime;
    //char *con_name = NULL;
    File *pfile = NULL;
    pfile = browse->filelist[i];
    sync_item_t item;
    int res;
    int fail_flag;

    snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,filename);

    server_mtime = atoi(pfile->attribute.lastwritetime);
    local_mtime = GetFile_modtime(fullname);

    if(pre_seq == -10)
    {
        //printf("server_mitem=%d,local_mtime=%d\n",server_mtime,local_mtime);


        if(server_mtime != local_mtime)
        {
            if(server_mtime - local_mtime == 1) // handle reboot router file mtime decrease 1 second on FAT32
            {
#ifdef DEBUG
                printf("local file one second\n");
#endif
                res = update_local_file_attr(&(pfile->attribute),fullname);
            }
            else
            {
                    res = handle_local_confilict_file(fullname,isfolder);
                    //printf("res=%d\n");
                    if(res != -1)
                    {
                        if(sync_server_downloadfile(pfile->id,fullname,
                                                     pfile->size,0,&(pfile->attribute)) == -1)
                            fail_flag = 1;
                    }
            }
        }
    }
    else if(pre_seq != -10)
    {
        item = get_sync_item("find",fullname,download_only_socket_head);
        if(item)
        {
            //printf("find item from download_only_socket_head\n");
           res = handle_local_confilict_file(fullname,isfolder);
           if(res != -1)
           {
               if(isfolder)
               {
                   my_mkdir(fullname);
                   add_server_action_list("createfolder",fullname,from_server_sync_head);
   #if SYSTEM_LOG
                   write_system_log("createfolder",filename);
   #endif
   #if TREE_NODE_ENABLE
               modify_tree_node(fullname,DirRootNode,ADD_TREE_NODE);
   #endif
               }
               else
               {
                   if(sync_server_downloadfile(pfile->id,fullname,pfile->size,
                                               0,&(pfile->attribute))== -1)
                       fail_flag = 1;
               }
           }

        }
        else
        {
            //printf("can't find item from download_only_socket_head\n");
            //if(isfolder)
                //continue;
            //else
            if(!isfolder)
            {
                if(server_mtime != local_mtime)
                {
                   item = get_sync_item("find",fullname,download_only_socket_head);
                   if(item == NULL)
                   {
                       if(server_mtime>local_mtime)
                       {
                           if(sync_server_downloadfile(pfile->id,fullname,pfile->size,
                                                       0,&(pfile->attribute))== -1)
                               fail_flag = 1;
                       }
                   }
                   else
                   {
                       if(server_mtime>local_mtime)
                       {
                           res = handle_local_confilict_file(fullname,isfolder);
                           if(res != -1)
                           {
                               if(sync_server_downloadfile(pfile->id,fullname,pfile->size,
                                                                                      0,&(pfile->attribute))== -1)
                                                              fail_flag = 1;
                           }
                       }
                   }
                }

            }
        }
    }
    return 0;
}

int sync_server_modify(char *parentfolder,Browse *browse,Local *local,int isupload,int i,int find)
{
    int local_pos;
    int server_pos;
    int fileID;
    long long size;
    char fullname[NORMALSIZE];
    struct utimbuf tbuf;
    unsigned int local_mtime;
    unsigned int server_mtime;
    char error_message[NORMALSIZE];
    Fileattribute *attr;
    int res;

    memset(error_message,0,sizeof(error_message));
    memset(fullname,0,sizeof(fullname));

    if(isupload)
    {
        local_pos = i;
        server_pos = find;
    }
    else
    {
        local_pos = find;
        server_pos = i;
    }

    snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,(local->filelist)[local_pos]->name);
    server_mtime = atoi((browse->filelist)[server_pos]->attribute.lastwritetime);
    local_mtime = GetFile_modtime(fullname);
    //int l_mtime = atoi((local->filelist)[local_pos]->attribute.creationtime);

    //printf("@@@@@ local time is %s @@@@@\n",(local->filelist)[local_pos]->attribute.lastwritetime);
    //printf("[%s] server mtime is %d,local mtime is %d\n",fullname,server_mtime,local_mtime);

    if(pre_seq == -10)
    {

        if(server_mtime != local_mtime)
        {
#ifdef DEBUG
            printf("[%s] server mtime is %d,local mtime is %d\n",fullname,server_mtime,local_mtime);
#endif
            if(server_mtime - local_mtime == 1) // handle reboot router file mtime decrease 1 second on FAT32
            {
#ifdef DEBUG
                printf("local file one second\n");
#endif
                res = update_local_file_attr(&(browse->filelist[server_pos]->attribute),fullname);
            }
            else
            {
                res = sync_local_add_file(parentfolder,local,local_pos,browse->filelist[server_pos]->id);
                sprintf(browse->filelist[server_pos]->attribute.lastwritetime,"%d",local_mtime);
            }

            return res;
        }
    }
    else
    {
        if(server_mtime > local_mtime)
        {
#ifdef DEBUG
            //printf("server mtime is %d,local mtime is %d\n",server_mtime,local_mtime);
#endif
            fileID = (browse->filelist)[server_pos]->id;
            size = (browse->filelist)[server_pos]->size;
            attr = &((browse->filelist)[server_pos]->attribute);
#ifdef DEBUG
            printf("server %s has motify\n",fullname);
#endif
            if(download_only != 1 && download_only != 1)
            {
                if(wait_handle_socket()== -1)
                    return -1;
            }

            if( downloadFile(fileID,fullname,size,1,attr) == -1)
            {
                snprintf(error_message,NORMALSIZE,"download %s fail",fullname);
                handle_error(S_DOWNLOADFILE_FAIL,error_message);
                return -1;
            }
            else
            {
                tbuf.actime = (time_t)server_mtime;
                tbuf.modtime = (time_t)server_mtime;
                if( utime(fullname,&tbuf) == -1)
                {
                    snprintf(error_message,NORMALSIZE,"utime %s fail",fullname);
                    handle_error(S_UPDATE_ATTR_FAIL,error_message);
                    return -1;
                }

                //add_server_action_list("createfile",fullname,from_server_sync_head);
#if TREE_NODE_ENABLE
                modify_tree_node(fullname,DirRootNode,DEL_TREE_NODE);
                modify_tree_node(fullname,DirRootNode,ADD_TREE_NODE);
#endif
            }
        }
    }


    return 0;
}

int sync_local_add_folder(char *parentfolder,Local *local,int i)
{
    int parentID = -10;
    int entry_ID = -10;
    Createfolder *createfolder = NULL;
    char error_message[NORMALSIZE];
    char fullname[NORMALSIZE];

    memset(error_message,0,sizeof(error_message));
    memset(fullname,0,sizeof(fullname));
    snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,(local->folderlist)[i]->name);


    parentID = getParentID(parentfolder);

    if(parentID < 0 )
    {
        snprintf(error_message,NORMALSIZE,"get parentID %s fail",fullname);
        handle_error(S_GET_PARENTID_FAIL,error_message);
        return -1;
    }
    else if(parentID > 0)
    {
        /*
        if(receve_socket == 1 && download_only != 1)
        {
            printf("error:receve_socket\n");
            return -1;
        }
        */
        /*
        while(receve_socket)
        {
            usleep(WAIT_LOCAL_TIME);
        }
        */
        if(wait_handle_socket() == -1)
        {
            //printf("wait_handle_socket() return -1\n");
            return -1;
        }
        createfolder = createFolder(username,parentID,0,fullname);

        if(NULL == createfolder)
        {
            snprintf(error_message,NORMALSIZE,"createfolder %s fail",(local->folderlist)[i]->name);
            handle_error(S_CREATEFOLDER_FAIL,error_message);
            return -1;
        }
        else if( createfolder->status != 0 )
        {
            handle_error(createfolder->status,"propfind");
            my_free(createfolder);
            return -1;
        }
        else if( createfolder->status == 0 )
        {
            entry_ID = createfolder->id;

            my_free(createfolder);
#ifdef DEBUG
            //printf("entry ID is %d\n",entry_ID);
#endif
            if( sync_all_item(fullname,entry_ID) == -1)
            {
                return -1;
            }
#if TREE_NODE_ENABLE
            //modify_tree_node(fullname,DirRootNode,ADD_TREE_NODE);
#endif
        }
    }
    return 0;
}

int sync_local_add_file(char *parentfolder,Local *local,int i,int entryID)
{
    char fullname[NORMALSIZE];
    int parentID = -10;
    char error_message[NORMALSIZE];
    int status;

    memset(error_message,0,sizeof(error_message));
    memset(fullname,0,sizeof(fullname));

    snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,(local->filelist)[i]->name);

    printf("fullname is %s\n",fullname);

    parentID = getParentID(parentfolder);

    printf("parentID is %d\n",parentID);
    printf("entryID is %d\n",entryID);

    if(parentID < 0)
    {
        snprintf(error_message,NORMALSIZE,"get %s parentID fail",fullname);
        handle_error(S_GET_PARENTID_FAIL,error_message);
        return -1;
    }
    else if(parentID > 0)
    {
        /*
        if(receve_socket == 1 && download_only != 1)
        {
            printf("error:receve_socket\n");
            return -1;
        }
        */

        /*
        while(download_only != 1 && receve_socket)
        {
            usleep(WAIT_LOCAL_TIME);
        }
        */
        if(wait_handle_socket() == -1)
            return -1;

        status = uploadFile(fullname,parentID,NULL,entryID);
        if( status != 0)
        {
            snprintf(error_message,NORMALSIZE,"uploadfile %s fail",fullname);
            handle_error(S_UPLOADFILE_FAIL,error_message);
            status = handle_upload_fail_code(status,parentID,fullname,parentfolder);
            return status;
        }
    }
    return 0;
}

int find_diff_item(char *parentfolder,Browse *browse,Local *local,int isupload,int isfolder,int pid)
{
    if(exit_loop == 1)
        return 0;

    int item_num = 0;
    int find_num = 0;
    int i = 0;
    int find = 0;
    //int parent_ID;
    char temp[NORMALSIZE];
    char filename[NORMALSIZE];
    char fullname[NORMALSIZE];
    int  fail_flag = 0;
    char error_message[NORMALSIZE];
    //char *confilicted_name;
    struct find_number find_n;
    int status = 0;
    int count = 0;

    memset(filename,0,sizeof(filename));
    memset(fullname,0,sizeof(fullname));

    //printf("isupload is %d,is folder is %d\n",isupload,isfolder);
    get_find_num(browse,local,isupload,isfolder,&find_n);
    item_num = find_n.item_num;
    find_num = find_n.find_num;

    for( i= 0; i<item_num ;i++,count++)
    {
        if(exit_loop ==1 )
            return 0;

//        if(count > loop_max)
//        {
//            printf("count=%d\n",count);
//            count = 0;
//            usleep(usleep_time);
//        }

        memset(temp,0,sizeof(temp));
        memset(error_message,0,sizeof(error_message));

        get_find_name(browse,local,isupload,isfolder,i,temp);

        if(strlen(temp) == 0)
        {
            handle_error(S_NAME_BLANK,"find diff item");
            continue;
        }

        find = findItem(temp,local,browse,isfolder,isupload,find_num);

        //struct utimbuf tbuf;
        //unsigned int local_mtime;
        //unsigned int server_mtime;

        //printf("find name=%s,index=%d\n",temp,find);
        //printf("find is %d\n",find);

        if(find == -2)
            return -1;

        if(find >= 0)
        {
            if( download_only == 1 )
            {
                if(isupload == 0) // local item find from server list
                {

                    if(!isfolder)
                    {
                        oauth_decode_base64((unsigned char *)filename,temp);
                        snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,filename);
                        //printf("fullname=%s\n",fullname);
                        downloadonly_server_modify(parentfolder,browse,i,filename,isfolder);
                        //printf("donwload_server_modify end\n");
                    }

#if 0
                    if(pre_seq == -10 && (!isfolder))
                    {
                    	server_mtime = atoi((browse->filelist)[i]->attribute.lastwritetime);
                    	local_mtime = GetFile_modtime(fullname);

                        //printf("server_mitem=%d,local_mtime=%d\n",server_mtime,local_mtime);

                    	if(server_mtime != local_mtime)
                    	{
                            //printf("confilicted %s is start\n",fullname);
                            confilicted_name = get_confilicted_name(fullname,isfolder);
                            //printf("confilicted name is %s\n",confilicted_name);
                            char *con_name = NULL;
                            con_name = parse_name_from_path(confilicted_name);
                            if(rename(fullname,confilicted_name) == -1)
                            {
                                snprintf(error_message,NORMALSIZE,"rename %s fail",fullname);
                                handle_error(S_RENAME_FAIL,error_message);
                                continue;
                            }
                            else
                            {
                                //printf("confilicted name is end\n");
    #if TREE_NODE_ENABLE
                                if(con_name != NULL)
                                    rename_update_tree(filename,con_name);
                                else
                                {
                                    printf("get con_name from %s fail\n",confilicted_name);
                                    continue;
                                }
    #endif
                                //add_server_action_list("rename",fullname,from_server_sync_head); //add for Server create and modify File
                                write_confilicted_log(fullname,confilicted_name);

                                if(sync_server_downloadfile((browse->filelist)[i]->id,fullname,(browse->filelist)[i]->size,0,
                                                            &((browse->filelist)[i]->attribute)) == -1)
                                    fail_flag = 1;

                                //printf("confilicted name and downloadfile is end\n");

                                my_free(confilicted_name);
                                my_free(con_name);
                            }



                    	}
                        //else
                        //{
                            //add_server_action_list("createfile",fullname,from_server_sync_head);
                        //}

                    }
                    //else if(pre_seq == 0 && isfolder)
                    //{
                        //add_server_action_list("createfolder",fullname,from_server_sync_head);
                    //}
                    else if(pre_seq != -10)
                    {
                    	sync_item_t item = get_sync_item("find",fullname,download_only_socket_head);
                        if(item)
                        {
                            //printf("find item from download_only_socket_head\n");
                            confilicted_name = get_confilicted_name(fullname,isfolder);
                            char *con_name = NULL;
                            con_name = parse_name_from_path(confilicted_name);

                            if(rename(fullname,confilicted_name) == -1)
                            {
                                printf("rename %s from to fail!!!!!\n",fullname,confilicted_name);
                            }
#if TREE_NODE_ENABLE
                            if(con_name != NULL)
                                rename_update_tree(filename,con_name);
                            else
                                printf("get con_name from %s fail\n",confilicted_name);
#endif
                            write_confilicted_log(fullname,confilicted_name);

                            //printf("find and rename from %s to %s\n",fullname,confilicted_name);

                            if(isfolder)
                            {
                                status = IsEntryDeletedFromServer(browse->folderlist[i]->id,1);
                                if(status == -1)
                                    fail_flag = 1;
                                else if(status == 0)
                                {
                                    my_mkdir(fullname);
                                    add_server_action_list("createfolder",fullname,from_server_sync_head);
    #if SYSTEM_LOG
                                    write_system_log("createfolder",filename);
    #endif
    #if TREE_NODE_ENABLE
                                   modify_tree_node(fullname,DirRootNode,ADD_TREE_NODE);
   #endif
                                }
                                else if(status == 1)
                                {
                                    browse->folderlist[i]->isdeleted = 1;
                                }


                            }
                            else
                            {
                                if(sync_server_downloadfile((browse->filelist)[i]->id,fullname,(browse->filelist)[i]->size,0,
                                                            &((browse->filelist)[i]->attribute)) == -1)
                                    fail_flag = 1;
                            }
                            my_free(confilicted_name);
                            my_free(con_name);
                        }
                        else
                        {
                            //printf("can't find item from download_only_socket_head\n");
                            if(isfolder)
                                continue;
                            else
                            {
                                status = sync_server_modify(parentfolder,browse,local,isupload,i,find);
                                if(status == -1)
                                    fail_flag = 1;
                            }
                        }
                    }
#endif
                }
            }
            else
            {
                if(isfolder)
                    continue;
                else
                {
                    status = sync_server_modify(parentfolder,browse,local,isupload,i,find);
                    if(status == -1)
                        fail_flag = 1;
                }

            }
        }
        else   // can't find item
        {
            //server_modify = 1;
            //printf("can't find,find is %d\n",find);

            //int parentID = -10;
            //int entry_ID = -10;
            //Createfolder *createfolder = NULL;
            //int res = -1;

            if(isupload == 1) // server have del or remove this file,remove local file
            {
                //printf("server mot exists [%s] files \n",local->filelist[i].name);
                if(isfolder)
                {
                    snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,(local->folderlist)[i]->name);

                    if(pre_seq == -10)
                    {
                        //res = test_file_if_exist(local->folderlist[i].name);
                    	if(download_only == 0)
                        {
#ifdef DEBUG
                            printf("%s folder is new add \n",fullname);
#endif
                            status = sync_local_add_folder(parentfolder,local,i);
                            if(status == -1)
                                fail_flag = 1;
                    	}
                    	else   //add for download only,add action_list for folders in the folder first
                    	{
                            char info[512];
                            memset(info,0,sizeof(info));
                            snprintf(info,512,"%s/%s",parentfolder,(local->folderlist)[i]->name);
                            add_sync_item("download_only",info,download_only_socket_head);
                    	}
                    }
                    else // pre_seq != 0
                    {
                        /*
                        if(receve_socket == 1)
                    	{
                            printf("error:receve_socket\n");
                            return -1;
                    	}
                        */
                        /*
                        while(receve_socket)
                        {
                            usleep(WAIT_LOCAL_TIME);
                        }
                        */
                        if(wait_handle_socket() == -1)
                        {
                            return -1;
                        }

                        if(download_only == 1)
                        {
                            /*check file if new add by local on download to disk rule*/
                            pthread_mutex_lock(&my_mutex.mutex);
                            sync_item_t p1 = get_sync_item("download_only",fullname,download_only_socket_head);
                            pthread_mutex_unlock(&my_mutex.mutex);
                            if(p1 != NULL)
                            {
                                continue;
                            }
                            else
                            {
                                printf("remove folder %s \n",fullname);

                                if(del_all_items(fullname,pid,browse,1) == -1)
                                {
                                    fail_flag = 1;
                                }
                            }
                        }
                        else
                        {
                            printf("remove folder %s \n",fullname);

#if SYSTEM_LOG
                            write_system_log("remove folder",fullname);
#endif
                            if(del_all_items(fullname,pid,browse,1) == -1)
                            {
                                fail_flag = 1;
                            }
                        }

                    }
                }
                else /* sync file */
                {
                    snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,(local->filelist)[i]->name);
#if 1
                    if( pre_seq == -10 )
                    {
                        if(download_only == 1)//add for download only,add action_list for files in the folder first
                        {
                            char info[512];
                            memset(info,0,sizeof(info));
                            snprintf(info,512,"%s/%s",parentfolder,(local->filelist)[i]->name);
                            add_sync_item("download_only",info,download_only_socket_head);

                    	}
                        else
                    	{
                            if(test_if_file_up_excep_fail(fullname) != 1)
                            {
                                printf("%s file is new add \n",fullname);
                                status = sync_local_add_file(parentfolder,local,i,0);
printf("status is %d \n",status);
                                if(status == -1)
                                    fail_flag = 1;
                            }
                    	}
                    }
                    else   // pre_seq != 0
                    {
                        if(download_only == 1)
                        {
                            /*check file if new add by local on download to disk rule*/
                            //printf("get socket item when del file start\n");
                            pthread_mutex_lock(&my_mutex.mutex);
                            sync_item_t p1 = get_sync_item("download_only",fullname,download_only_socket_head);
                            pthread_mutex_unlock(&my_mutex.mutex);
                            //printf("get socket item when del file end\n");
                            if(p1 != NULL)
                            {
                                continue;
                            }
                        }

                        if(sync_server_del_local_file(fullname) == -1)
                            fail_flag = 1;

                        //printf(" del server file end\n");
                    }
#endif
                }

            }
            else if(isupload == 0)
            {
                //printf("local mot exists [%s] files \n",temp);
                if(isfolder)
                {
                    /*
                    if(receve_socket == 1 && download_only != 1)
                    {
                        printf("error:receve_socket\n");
                        return -1;
                    }
                    */
                    /*
                    while(download_only != 1 && receve_socket)
                    {
                        usleep(WAIT_LOCAL_TIME);
                    }*/
                    if(wait_handle_socket() == -1)
                    {
                        return -1;
                    }
                    oauth_decode_base64((unsigned char *)filename,(browse->folderlist)[i]->display);
                    snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,filename);
#ifdef DEBUG
                    printf("create folder is %s\n",fullname);
#endif
                    status = IsEntryDeletedFromServer(browse->folderlist[i]->id,1);
#ifdef DEBUG
                    printf("#######folder status=%d ####\n",status);
#endif
                    if(status == -1)
                        fail_flag = 1;
                    else if(status == 0)
                    {
                        if(mkdir(fullname,0777) == -1)
                        {
                            snprintf(error_message,NORMALSIZE,"mkdir %s fail \n",fullname);
                            handle_error(S_MKDIR_FAIL,error_message);
                            fail_flag = 1;
                        }
                        else
                        {
                            add_server_action_list("createfolder",fullname,from_server_sync_head);

                            if(pre_seq > 0)
                            {
                                browse->folderlist[i]->ischangeseq = 1;
                                insert_node(pid,browse->folderlist[i]->id,0);
                            }
    #if SYSTEM_LOG
                            write_system_log("createfolder",filename);
    #endif
    #if TREE_NODE_ENABLE
                            modify_tree_node(fullname,DirRootNode,ADD_TREE_NODE)
    #endif
                        }
                    }
                    else if(status == 1)
                    {
                        browse->folderlist[i]->isdeleted = 1;
                        del_node(pid,browse->folderlist[i]->id);
                    }

                }
                else
                {
                    /*
                    if(receve_socket == 1 && download_only != 1)
                    {
                        printf("error:receve_socket\n");
                        return -1;
                    }
                    */
                    /*
                    while(download_only != 1 && receve_socket)
                    {
                        usleep(WAIT_LOCAL_TIME);
                    }*/
                    if(wait_handle_socket() == -1)
                    {
                        return -1;
                    }

                    oauth_decode_base64((unsigned char *)filename,(browse->filelist)[i]->display);
                    snprintf(fullname,NORMALSIZE,"%s/%s",parentfolder,filename);
                    //printf("download file is %s\n",fullname);
                    if(sync_server_downloadfile((browse->filelist)[i]->id,fullname,(browse->filelist)[i]->size,0,
                                                &((browse->filelist)[i]->attribute)) == -1)
                        fail_flag = 1;
                }
            }
        }
    }
    return (fail_flag == 1) ? -1 : 0;
}

int UploadOnlySyncItem(char *parentfolder,int parentid,Browse *browse,Local *local,char *username)
{
    int file_res = -10;
    int folder_res = -10;

    /*sync files*/
    file_res = upload_only_find_diff_item(parentfolder,browse,local,0);
    /*sync folder*/
    folder_res = upload_only_find_diff_item(parentfolder,browse,local,1);


    if(file_res == -1 || folder_res == -1)
    {
        printf("UploadOnlySyncItem fail\n");
        return -1;
    }

    return 0;
}

int syncItem(char *parentfolder,int parentid,Browse *browse,Local *local,char *username)
{
    int isupload;
    int isfolder;
    int file_status = -10;
    int folder_status = -10;
    int file_res = -10;
    int folder_res = -10;

    /*sync files*/
    if(download_only == 1 || pre_seq == -10)
    {
    	isfolder = 0;

        file_status = find_diff_item(parentfolder,browse,local,0,isfolder,parentid);

        file_res = find_diff_item(parentfolder,browse,local,1,isfolder,parentid);

    	isfolder = 1;

        folder_status = find_diff_item(parentfolder,browse,local,0,isfolder,parentid);

        folder_res = find_diff_item(parentfolder,browse,local,1,isfolder,parentid);
    }
    else
    {
    	isfolder = 0;
        if(browse->filenumber < local->filenum ) //server del files
        {
            isupload = 1;
            file_status = find_diff_item(parentfolder,browse,local,isupload,isfolder,parentid);
        }
        else if(browse->filenumber > local->filenum) //server upload files
        {
            isupload = 0;
            file_status = find_diff_item(parentfolder,browse,local,isupload,isfolder,parentid);
        }
        else                                        //server rename files
        {
#if 0
            int i;
            for(i = 0; i < local->filenum;i++)
            {
                char t_name[256];
                memset(t_name,0,sizeof(t_name));

                oauth_decode_base64(t_name,browse->filelist[i].display);

                printf("### %d ###\n",i);
                printf("local [%s] : createtime is %s \n",local->filelist[i].name,local->filelist[i].attribute.creationtime);
                printf("server [%s] ,decode name is %s: createtime is %s \n", \
                       browse->filelist[i].display,t_name,browse->filelist[i].attribute.creationtime);
            }

            return 0;
#endif
            file_status = find_diff_item(parentfolder,browse,local,0,isfolder,parentid);

            file_res = find_diff_item(parentfolder,browse,local,1,isfolder,parentid);
        }

        /*sync folder*/
        isfolder = 1;
        if(browse->foldernumber < local->foldernum ) //upload files to server
        {
            isupload = 1;
            folder_status = find_diff_item(parentfolder,browse,local,isupload,isfolder,parentid);
        }
        else if(browse->foldernumber > local->foldernum)
        {
            isupload = 0;
            folder_status = find_diff_item(parentfolder,browse,local,isupload,isfolder,parentid);
        }
        else
        {
            folder_status = find_diff_item(parentfolder,browse,local,0,isfolder,parentid);
            folder_res = find_diff_item(parentfolder,browse,local,1,isfolder,parentid);
        }
    }


    if(file_status == -1 || file_res == -1 || folder_status == -1 || folder_res == -1)
    {
        return -1;
    }

    return 0;

}

void print_all_local_item(char *dir,char *sync_item_filename,int init)
{
    //printf("dis is %s,filename is %s\n",dir,sync_item_filename);

    struct dirent* ent = NULL;
    DIR *pDir;
    pDir = opendir(dir);
    FILE *fp;

    if(NULL == pDir)
    {
        printf("open %s fail \n",dir);
        return;
    }

    fp = fopen(sync_item_filename,"a");

    if(NULL == fp)
    {
        printf("create %s file error\n",sync_item_filename);
        return;
    }

    //printf("dis is %s 2\n",dir);

    while (NULL != (ent=readdir(pDir)))
    {
        //if(ent->d_name[0] == '.')
            //continue;
        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        char fullname[NORMALSIZE];
        memset(fullname,0,sizeof(fullname));
        snprintf(fullname,NORMALSIZE,"%s/%s",dir,ent->d_name);

        fprintf(fp,"%s\n",fullname);
        //printf("%s\n",fullname);

        if( test_if_dir(fullname) == 1)
        {
            //printf("aa\n");
            print_all_local_item(fullname,sync_item_filename,0);
        }
    }

    fclose(fp);

    //printf("bb\n");
    closedir(pDir);
    //fclose(fp);
}

int get_item_size(char *dir, int *size)
{
    DIR *pDir;
    pDir=opendir(dir);
    int folder_size = 0;
    int file_size = 0;
    struct dirent* ent = NULL;

    if(NULL == pDir)
    {
        handle_error(S_OPENDIR_FAIL,"opendir");
        return -1;
    }

    while (NULL != (ent=readdir(pDir)))
    {

        //if(ent->d_name[0] == '.')
            //continue;
        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,"..") || !strcmp(ent->d_name,".smartsync"))
            continue;

        char fullname[512];

        //printf("get_item_size ent->d_name is %s\n",ent->d_name);
        memset(fullname,0,sizeof(fullname));
        snprintf(fullname,512,"%s/%s",dir,ent->d_name);

        if( test_if_dir(fullname) == 1)
        {
            folder_size++;
        }
        else
        {
            file_size++;
        }

    }
    closedir(pDir);

    size[0] = file_size;
    size[1] = folder_size;

    return 0;
}

int myFindDir(char *dir,int level,Local *local)
{
    int size[2];
    int len = 0;
    memset(size,0,sizeof(size));
    if(get_item_size(dir,size) == -1)
    {
        return -1;
    }
    struct dirent* ent = NULL;
    DIR *pDir;
    pDir=opendir(dir);
    if(size[0] > 0)
    {
        local->filelist = (Localfile **)malloc(sizeof(Localfile *)*size[0]);
        if(local->filelist == NULL)
        {
            handle_error(S_MEMORY_FAIL,"myFindDIr");
            return -1;
        }
    }
    if(size[1]>0)
    {
        local->folderlist = (Localfolder **)malloc(sizeof(Localfolder *)*size[1]);
        if(local->folderlist == NULL)
        {
            handle_error(S_MEMORY_FAIL,"myFindDIr");
            return -1;
        }
    }



    if(NULL == pDir)
    {
        //printf("dir is %s\n",dir);
        handle_error(S_OPENDIR_FAIL,"opendir");
        return -1;
    }

    while (NULL != (ent=readdir(pDir)))
    {

        //if(ent->d_name[0] == '.')
            //continue;

        if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
            continue;

        if(!strcmp(dir,mount_path) && !strcmp(ent->d_name,".smartsync"))
        {
            continue;
        }

        if(test_if_download_temp_file(ent->d_name))
        {
            continue;
        }

        char fullname[512];

        memset(fullname,0,sizeof(fullname));
        snprintf(fullname,512,"%s/%s",dir,ent->d_name);


        //printf("name is %s,d_type is %u\n",ent->d_name,ent->d_type);
        //printf("name=%s\n",ent->d_name);
        len = strlen(ent->d_name) + 1;
        if( test_if_dir(fullname) == 1)
        {
            local->foldernum++;
            int foldernum = local->foldernum -1;
            //printf("folder name is %s,folder num is %d\n",ent->d_name,local->foldernum);
            //strcpy(local->folderlist[foldernum].name,ent->d_name);
            (local->folderlist)[foldernum] = (Localfolder *)malloc(sizeof(Localfolder));
            //strcpy((local->folderlist)[foldernum]->name,ent->d_name);
            //sprintf((local->folderlist)[foldernum].name,"%s",ent->d_name);

            (local->folderlist)[foldernum]->name = calloc(len,sizeof(char));
            snprintf((local->folderlist)[foldernum]->name,NORMALSIZE,"%s",ent->d_name);
        }
        else
        {
#if 0
            if(upload_only == 1)
            {
                if(is_copying_finished(fullname) > 0)
                    continue;
            }
#endif
            struct stat buf;

            if( stat(fullname,&buf) == -1)
            {
                //perror("stat:");
                printf("myFindDir stat error:%s file not exist\n",fullname);
                continue;
            }

            local->filenum++;
            int filenum = local->filenum -1;

//            unsigned long asec = buf.st_atime;
            unsigned long msec = buf.st_mtime;
//            unsigned long csec = buf.st_ctime;

            (local->filelist)[filenum] = (Localfile *)malloc(sizeof(Localfile));


//            snprintf(((local->filelist)[filenum])->attribute.lastaccesstime,MINSIZE,"%lu",asec);
//            snprintf(((local->filelist)[filenum])->attribute.creationtime,MINSIZE,"%lu",csec);
            snprintf(((local->filelist)[filenum])->attribute.lastwritetime,MINSIZE,"%lu",msec);

            //strcpy(local->filelist[filenum].name,ent->d_name);
            //printf("@@@@@@@@ file is %s @@@@@@@\n ",ent->d_name);
            //len = strlen(ent->d_name) + 1;
            ((local->filelist)[filenum])->name = calloc(len,sizeof(char));
            snprintf(((local->filelist)[filenum])->name,NORMALSIZE,"%s",ent->d_name);
            //strcpy(local->filelist[filenum]->name,ent->d_name);
            ((local->filelist)[filenum])->size = buf.st_size;
        }

        //printf("per end\n");

    }
    closedir(pDir);

    return 0;
}

#if 0
int initMyLocalFolder(char *username,int parentid,char *localpath,char *xmlfilename)
{

    //printf("#########initMyLocalFolder starts########\n");
#if 0
    Browse browse;
    memset(&browse,0,sizeof(Browse));
    browseFolder(username,parentid,0);
#else
    Browse *browse;
    char filename[NORMALSIZE];
    char path[NORMALSIZE];
    char foldername[NORMALSIZE];
    int i;
    int k;
    //memset(browse,0,sizeof(Browse));

    //printf("#########browse start########\n");

    browse = browseFolder(username,parentid,0);

    //printf("#########browse end########\n");
    if(NULL == browse)
        return -1;

    /*
    if(browse->status == S_AUTH_FAIL)
    {
        //free(browse);
        //return 0;
        free_server_list(browse);
        my_free(browse);
        if(getToken(username,password) == -1)
            return -1;
        initMyLocalFolder(username,parentid,localpath,xmlfilename);
        return 0;
    }
    */
#endif
    //parseDoc1(xmlfilename,&browse);


    for( i= 0 ; i <browse->filenumber; i++)
    {
#ifdef IPKG
            if(disk_change)
            {
                pthread_mutex_lock(&mutex_socket);
                disk_change = 0;
                pthread_mutex_unlock(&mutex_socket);
                check_disk_change();
            }
#endif
        if(exit_loop == 1)
        {
            free_server_list(browse);
            my_free(browse);
            return 0;
        }

        memset(filename,0,sizeof(filename));
        memset(path,0,sizeof(path));

        oauth_decode_base64(filename,(browse->filelist)[i]->display);

        if(strlen(filename) == 0)
        {
            handle_error(S_NAME_BLANK,"init local syncfolder");
            continue;
        }
        snprintf(path,NORMALSIZE,"%s/%s",localpath,filename);
#ifdef DEBUG
        //printf("download filename is %s \n",path);
#endif

#if SYSTEM_LOG
        write_system_log("download",path);
#endif
        downloadFile((browse->filelist)[i]->id,path,(browse->filelist)[i]->size,0,
                     &(browse->filelist[i]->attribute));
    }



    for( k= 0; k < browse->foldernumber;k++)
    {
#ifdef IPKG
            if(disk_change)
            {
                pthread_mutex_lock(&mutex_socket);
                disk_change = 0;
                pthread_mutex_unlock(&mutex_socket);
                check_disk_change();
            }
#endif
        if(exit_loop == 1)
        {
            free_server_list(browse);
            my_free(browse);
            return 0;
        }

        memset(foldername,0,sizeof(foldername));
        memset(path,0,sizeof(path));

        oauth_decode_base64(foldername,(browse->folderlist)[k]->display);

        if(strlen(foldername) == 0)
        {
            handle_error(S_NAME_BLANK,"init local syncfolder");
            continue;
        }

        snprintf(path,NORMALSIZE,"%s/%s",localpath,foldername);
#ifdef DEBUG
        //printf("foldername is %s \n",foldername);
        printf("create path is %s \n",path);
#endif

#if SYSTEM_LOG
        write_system_log("createfolder",path);
#endif

        mkdir(path,0777);

        initMyLocalFolder(username,(browse->folderlist)[k]->id,path,xmlfilename);
    }

    free_server_list(browse);
    my_free(browse);

    return 0;
}
#endif

int get_local_folder_id(const char *filename,Browse *br)
{
    int i;
    char temp[NORMALSIZE];
    for(i = 0;i<br->foldernumber;i++)
    {
        memset(temp,0,sizeof(temp));
        oauth_decode_base64((unsigned char *)temp,(br->folderlist)[i]->display);
        if(!strcmp(filename,temp))
            return br->folderlist[i]->id;
    }

    return -1;
}

int syncServerAllItem(char *username,int parentid,char *localpath)
{
   //mySync(username,parentid,localpath);
   int res = -1;
   int fail_flag = 0;
   NodeStack *node_stack_link = NULL;
   res = mySync(username,parentid,localpath,&node_stack_link);
   if(res == -1)
       fail_flag = 1;
   //printf("sub folder start\n");
   while(node_stack_link != NULL)
   {
       FolderNode *node = pop_node(&node_stack_link);
       if(node == NULL)
           break;
       //printf("node->path=%s\n",node->path);
       res = mySync(username,node->id,node->path,&node_stack_link);
       if(res == -1)
           fail_flag = 1;
		 else
        {
           if(pre_seq >0 && node->seq != -10 )
               update_seq(node->id,node->seq,SyncNode);
        }
       my_free(node->path);
       my_free(node);
   }

   return (fail_flag == 1) ? -1 : 0 ;
}
static int sum2 = 0;
int print_server_struct_size(Browse *br,Local *local)
{
   int i,size,sum=0,sum1=0;
   size = sizeof(Browse);
   sum += size;
   printf("browse_struct_size=%d\n",size);
   size = 0;
   for(i=0;i<br->filenumber;i++)
   {
       size += sizeof(File);
       size += strlen(br->filelist[i]->display)+1;
   }
   printf("browse_total_file_size=%d\n",size);
   sum += size;
   size = 0;
   for(i=0;i<br->foldernumber;i++)
   {
       size += sizeof(Folder);
       size += strlen(br->folderlist[i]->display)+1;
   }
   sum += size;
   printf("browse_total_folder_size=%d\n",size);

   printf("########browse_total_size=%d#####\n",sum);
   sum1 += sum;

   sum = 0; size = 0;
   size = sizeof(Local);
   sum += size;
   printf("local_struct_size=%d\n",size);
   size = 0;
   for(i=0;i<local->filenum;i++)
   {
       size += sizeof(Localfile);
       size += strlen(local->filelist[i]->name)+1;
   }
   printf("local_total_file_size=%d\n",size);
   sum += size;
   size = 0;
   for(i=0;i<local->foldernum;i++)
   {
       size += sizeof(Localfolder);
       size += strlen(local->folderlist[i]->name)+1;
   }
   sum += size;
   printf("local_total_folder_size=%d\n",size);
   printf("########local_total_size=%d#####\n",sum);

   sum1 += sum;
   sum2 += sum1;
   printf("*************  totoal_size=%d,all_size=%d  ***********\n",sum1,sum2);

   return 0;
}

int is_folder_change(int id,Hb_SubNode **pnode,int *current_seq)
{
    //printf("enter check_change\n");
    Changeseq *cs = NULL;
    Hb_SubNode *fnode = NULL;
    int seq =0;

    //printf("name=%s,id=%d\n",name,id);

    while( (cs = getChangeSeq(id)) == NULL)
    {
       enter_sleep_time(5,&wait_server_mutex);
    }

    if(cs->status != 0)
    {
        my_free(cs);
        return 1;
    }
    else
    {
       seq = cs->changeseq;
       my_free(cs);
    }
    //printf("name=%s,id=%d,seq=%d\n",name,id,cs->changeseq);

    fnode = get_parent_node(id,SyncNode);
    if(fnode == NULL)
        return 1;
    printf("cur_seq=%d,pre_seq=%d\n",seq,fnode->seq);
    if(fnode->seq == seq)
        return 0;
    else
    {
        //fnode->seq = seq;
        *pnode = fnode;
        *current_seq = seq;
        return 1;
    }
}


int mySync(char *username,int parentid,char *localpath,NodeStack **head)
{
    printf("mySync function start,localpath=%s\n",localpath);

    if(upload_only != 1)
    {
        if(exit_loop)
            return 0;
    }

    Browse *br = NULL;
    Local local;
    int id = -10;
    int fail_flag = 0;
    char path[MAXSIZE];
    char foldername[256];
    int loop;
    int i;
    Hb_SubNode *subnode = NULL;
    int seq;
    int change_status = 0;

    memset(&local,0,sizeof(Local));

    printf("browse function start\n");

    br = browseFolder(username,parentid,0,1);

    if(NULL == br)
    {
        printf("browse folder failed\n");
        check_network_state();
        return -1;
    }

    if( myFindDir(localpath,0,&local) == -1)
    {
        printf("myFindDir() fail\n");
        free_server_list(br);
        my_free(br);
        return -1;
    }

    if(upload_only == 1)
    {
        if(UploadOnlySyncItem(localpath,parentid,br,&local,username) == -1)
        {
            printf("UploadOnlySyncItem\n");
            fail_flag = 1;
        }
    }
    else
    {
        if(syncItem(localpath,parentid,br,&local,username) == -1)
        {
            printf("syncItem() fail\n");
            fail_flag = 1;
        }
    }

    if(upload_only ==1)
        loop = local.foldernum;
    else
        loop = br->foldernumber;
    printf("browse folder num is %d\n",loop);

    for( i= 0; i <loop;i++)
    {
        if(exit_loop == 1)
        {
            break;
        }

        if(upload_only && receve_socket)
        {
                fail_flag = 1;
                break;
        }

        printf("brfolder list start\n");

        if(!upload_only)
        {
            if(br->folderlist[i]->isdeleted)
                continue;
        }

        memset(foldername,0,sizeof(foldername));
        memset(path,0,sizeof(path));

        if(upload_only == 1)
            strncpy(foldername,(local.folderlist)[i]->name,256);
        else
            oauth_decode_base64((unsigned char *)foldername,(br->folderlist)[i]->display);

        if(strlen(foldername) == 0)
        {
            handle_error(S_NAME_BLANK,"my sync");
            continue;
        }

        snprintf(path,1024,"%s/%s",localpath,foldername);


        if(upload_only == 1)
        {
            id = get_local_folder_id(foldername,br);

            if(id < 0)
            {
                continue;
            }
        }
        else
        {
            id = (br->folderlist)[i]->id;
        }
		  
       if(pre_seq > 0)
       {
            subnode = NULL;
            seq = -10;
            change_status = is_folder_change(id,&subnode,&seq);
            if( change_status== 0 && br->folderlist[i]->ischangeseq != 1)
            {
                printf("no change\n");
                continue;
            }
        }

        FolderNode *node = (FolderNode *)calloc(1,sizeof(FolderNode));
        node->path = (char *)calloc(1,strlen(path)+1);
        //strcpy(node->name,foldername);
        strcpy(node->path,path);
        printf("node_path is %s\n",node->path);
        node->id = id;
        node->seq = seq;
        push_node(node,head);
    }

    free_server_list(br);
    my_free(br);
    free_local_list(&local);

    printf("end mySync function\n");

    return (fail_flag == 1) ? -1 : 0 ;

}

int get_all_folders(const char *dirname,Folders *allfolderlist)
{
    struct dirent* ent = NULL;
    DIR *pDir;
    char temp_dir[1024];
    int num ;

    pDir=opendir(dirname);
    if(pDir != NULL )
    {
        while (NULL != (ent=readdir(pDir)))
        {
            //if(ent->d_name[0] == '.')
                //continue;

            if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
                continue;

            if(ent->d_type == DT_DIR)
            {
                num = allfolderlist->number;
                memset(temp_dir,0,sizeof(temp_dir));
                snprintf(temp_dir,1024,"%s/%s",dirname,ent->d_name);

                strcpy(allfolderlist->folderlist[num].name,temp_dir);
                //printf("folder name is %s,num is %d\n",temp_dir,num);

                allfolderlist->number++;
                get_all_folders(temp_dir,allfolderlist);
            }

        }
        closedir(pDir);
    }
    else
        printf(" %s fail \n",dirname);

    return 0;
}

long long int check_disk_space(char *path)
{
    struct statvfs diskdata;
    long long int free_disk_space;
    if (!statvfs(path, &diskdata))
    {
        free_disk_space = (long long)diskdata.f_bsize * (long long)diskdata.f_bavail;
        //printf("free disk space is %lld \n",free_disk_space);
        return free_disk_space;
    }
    else
    {
        printf("obtain disk space is failed ,path is %s \n",path);
        return -1;
    }
}

#if 0
/*sync queue function*/
int sync_queue_empty (void *q,int type)
{
    switch (type)
    {
    case LOCAL_FILE_QUEUE:
        q = (queue_lfile_t)q;
        break;
    case LOCAL_FOLDER_QUEUE:
        q = (queue_lfolder_t)q;
        break;
    default:
        break;
    }

    return q->head == NULL;
}

void *sync_queue_create (int type)
{
    void * q;
    switch (type)
    {
    case LOCAL_FILE_QUEUE:
        q = (queue_lfile_t)q;
        break;
    case LOCAL_FOLDER_QUEUE:
        q = (queue_lfolder_t)q;
        break;
    default:
        break;
    }
}

void sync_queue_destroy (void *q,int type);
void sync_queue_enqueue (void *d, void *q,int type);
void *queue_dequeue (void *q,int type);
#endif

/*queue function*/
int queue_empty (queue_t q)
{
    return q->head == NULL;
}

queue_t queue_create ()
{
    queue_t q;
    q = malloc (sizeof (struct queue_struct));
    if (q == NULL)
        //exit (-1);
        return NULL;

    q->head = q->tail = NULL;
    return q;
}

void queue_destroy (queue_t q)
{
    if (q != NULL)
    {
        while (q->head != NULL)
        {
            queue_entry_t next = q->head;
            q->head = next->next_ptr;
            next->next_ptr = NULL;
#if MEM_POOL_ENABLE
            mem_free(next->cmd_name);
            mem_free (next);
#else
            free(next->cmd_name);
            free (next);
#endif
        }
        q->head = q->tail = NULL;
        free (q);
    }
}

void queue_enqueue (queue_entry_t d, queue_t q)
{
    d->next_ptr = NULL;
    if (q->tail)
    {
        q->tail->next_ptr = d;
        q->tail = d;
    }
    else
    {
        q->head = q->tail = d;
    }
}

queue_entry_t  queue_dequeue (queue_t q)
{
    queue_entry_t first = q->head;

    if (first)
    {
        q->head = first->next_ptr;
        if (q->head == NULL)
        {
            q->tail = NULL;
        }
        first->next_ptr = NULL;
    }
    return first;
}

/*sync item*/

int item_empty(sync_item_t head)
{
    if(head == NULL)
        return 1;
    else
        return (head->next == NULL) ? 1 : 0 ;
}

sync_item_t create_head()
{
    {
        sync_item_t head;
        head=(sync_item_t)malloc(sizeof (struct sync_item) );
        if(head == NULL)
        {
            printf("create memory error!\n");
            return NULL;
        }
        head->next = NULL;
        return head;
    }
}

sync_item_t create_sync_item(const char *action, const char *name)
{
    int len;
    sync_item_t q = (sync_item_t)malloc(sizeof(struct sync_item));
    if(q == NULL)
        return NULL;
    memset(q, 0, sizeof(struct sync_item));

    len = strlen(action)+1;
    q->action = (char *)calloc(len,sizeof(char));
    if(q->action == NULL)
        return NULL;

    len = strlen(name)+1;
    q->name = (char *)calloc(len,sizeof(char));
    if(q->name == NULL)
        return NULL;

    strcpy(q->action,action);
    strcpy(q->name,name);
    return q;
}

int insert_sync_item(sync_item_t item, sync_item_t head)
{
    sync_item_t p1, p2;
    p2 = head;
    p1 = head->next;

    while(p1 != NULL)
    {
        p2 = p1;
        p1 = p1->next;
    }

    p2->next = item;
    item->next = p1;
    return 0;
}



int add_sync_item(const char *action,const char *name, struct sync_item *phead)
{
    printf("add action=%s,name=%s\n",action,name);

    struct sync_item *p1, *p2;
		
		printf("add action 0000001 \n");
		
    p2 = phead;
		
		printf("add action 0000002 \n");
    p1 = phead->next;
		printf("add action 0000003 \n");
		
		printf("add action 000000 \n");

    while(p1 != NULL)
    {

				printf("add action 111111 \n");
        char *p = strstr(action,"uploadfile");

        printf("p is %s\n",p);

        if(p)
        {
            printf("p1->name is %s,name is %s\n",p1->name,name);
            if( strcmp(p1->name,name) == 0 )
            {
                printf("add sync item fail,upload item has exist\n");
                return -1;
            }
        }
        else
        {
            printf("add action=%s,name=%s,p1->action is %s,p1->name is %s\n",action,name,p1->action,p1->name);

            if( strcmp(p1->action,action) == 0 && strcmp(p1->name,name) == 0)
            {
                printf("add sync item fail,item has exist\n");
                return -1;
            }

        }

        p2 = p1;
        p1 = p1->next;
    }

		printf("add action 222222 \n");
    struct sync_item *item = create_sync_item(action,name);

    p2->next = item;
    item->next = p1;


    printf("add sync item ok\n");

    return 0;
}



void del_sync_item(char *action,char *name, struct sync_item *phead)
{
    //printf("@@@@@@@@@@@@@del_sync_item action=%s,name=%s\n",action,name);
    struct sync_item *p1, *p2;
    p1 = phead->next;
    p2 = phead;
    while(p1 != NULL)
    {
        char *p = strstr(action,"uploadfile");

        if(p)
        {
            if( strcmp(p1->name,name) == 0 )
            {
                p2->next = p1->next;
                my_free(p1->action);
                my_free(p1->name);
                my_free(p1);
                //printf("del sync item ok\n");
                break;
            }
        }
        else
        {
            if( strcmp(p1->action,action) == 0 && strcmp(p1->name,name) == 0)
            {
                p2->next = p1->next;
                my_free(p1->action);
                my_free(p1->name);
                my_free(p1);
                //printf("del sync item ok\n");
                break;
            }
        }

        p2 = p1;
        p1 = p1->next;
    }

    //printf("del sync item fail\n");
}

void del_download_only_sync_item(char *action,char *name, struct sync_item *phead)
{
    //printf("del_sync_item action=%s,name=%s\n",action,name);
    struct sync_item *p1, *p2;
    char cmp_name[512];
    char p1_cmp_name[512];
    p1 = phead->next;
    p2 = phead;

    memset(cmp_name,'\0',512);
    snprintf(cmp_name,512,"%s/",name);    //add for delete folder and subfolder in download only socket list

    while(p1 != NULL)
    {
        memset(p1_cmp_name,'\0',512);
        snprintf(p1_cmp_name,512,"%s/",p1->name);      //add for delete folder and subfolder in download only socket list
        //printf("del_download_only_sync_item  p1->name = %s\n",p1->name);
        //printf("del_download_only_sync_item  cmp_name = %s\n",cmp_name);
        if(strstr(p1_cmp_name,cmp_name) != NULL)
        {
            p2->next = p1->next;
            free(p1);
            //printf("del sync item ok\n");
            //break;
            p1 = p2->next;
        }
    	else
    	{
            p2 = p1;
            p1 = p1->next;
    	}
    }

    //printf("del sync item fail\n");
}

struct sync_item* get_sync_item(char *action,char *name, struct sync_item *phead)
{
    //printf("find  [action=%s,len is %d],[name=%s,len is %d]\n",action,strlen(action),name,strlen(name));
    struct sync_item *p1 = phead->next;
    while(p1 != NULL)
    {
        //printf("find  [p1->action=%s,len is %d],[p1->name=%s,len is %d]\n",p1->action,strlen(p1->action),p1->name,strlen(p1->name));

        if(download_only == 1)
        {
            if(!strcmp(p1->name,name))
            {
                return p1;
            }
        }
        else
        {
            if( !strcmp(p1->action,action) && !strcmp(p1->name,name))
            {
                return p1;
            }
        }
        p1 = p1->next;
    }

    //printf("can not find item\n");

    return NULL;
}

//int check_excep_item(struct sync_item *phead)
//{
//    struct sync_item *p1 = phead->next;
//    struct stat buf;
//    char name[1024] = {0};
//
//    while(p1 != NULL)
//    {
//         if( stat(p1->name,&buf) == -1)
//        {
//             strncpy(name,p1->name,1024);
//             del_sync_item("up_excep_fail",name,phead);
//         }
//         else
//
//        p1 = p1->next;
//    }
//
//    return NULL;
//}

void free_sync_item(sync_item_t head)
{
    sync_item_t p,p1;
    p = head->next;
    while(p != NULL)
    {
        //head = head->next;
        p1 = p->next;
        free(p->action);
        free(p->name);
        free(p);
        p = p1;
    }
    free(head);

    printf("free list ok\n");
}

void print_all_sync_item(struct sync_item *head)
{
    sync_item_t p = head->next;

    while(p != NULL)
    {
        printf("action = %s\n", p->action);
        printf("name = %s\n", p->name);
        p = p->next;
    }
}

void print_sync_item(struct sync_item *head,int type)
{

    FILE *fp;
    char filename[NORMALSIZE];

    memset(filename,0,sizeof(filename));

    if(type == UPLOAD) //upload
        strcpy(filename,up_item_file);
    else if(type == DOWNLOAD)
        strcpy(filename,down_item_file);
    else
        strcpy(filename,up_excep_fail_file);

    fp = fopen(filename,"w");

    if(NULL == fp)
        return;

    sync_item_t p = head->next;

    while(p != NULL)
    {
        printf("action = %s\n", p->action);
        printf("name = %s\n", p->name);
        fprintf(fp,"%s,%s\n",p->action,p->name);
        p = p->next;
    }

    fclose(fp);
}


char *parse_name_from_path(const char *path)
{
    char *name;
    char *p;

    name = (char *)malloc(sizeof(char)*512);
    memset(name,0,sizeof(char)*512);

    p = strrchr(path,'/');

    if( p == NULL)
    {
        my_free(name);
        return NULL;
    }

    p++;

    strcpy(name,p);

    return name;
}

Transitem *parse_trans_item_from_buffer(char *buffer,int type)
{
    printf("parse content is %s\n",buffer);

    Transitem *item;
    item = (Transitem *)malloc(sizeof(Transitem));
    if(item == NULL)
    {
        handle_error(S_MEMORY_FAIL,"memory");
        return NULL;
    }

    memset(item,0,sizeof(Transitem));

    const char *split = ",";
    char *p;

    p=strtok(buffer,split);
    int i=0;
    while(p!=NULL)
    {
        switch (i)
        {
        case 0 :
            break;
        case 1:
            item->id = atoi(p);
            break;
        case 2:
            if(type == UPLOAD) //upload
                strcpy(item->transid,p);
            else
                item->size = atoll(p);
            break;
        case 3:
            strcpy(item->name,p);
            break;

        default:
            break;
        }

        i++;
        p=strtok(NULL,split);
    }

    

    if( item->name[ strlen(item->name)-1 ] == '\n' )
        item->name[ strlen(item->name)-1 ] = '\0';



    return item;

//printf("type is %d,status is %d,username is %s,password is %s,rule is %d,base_path is %s\n", type,status,username,password,rule,base_path);


}


int parse_trans_item(char *path,int type)
{
    //char filename[256];
    //int id;
    //long long int size;

    //init_fail_item = 1;

    Transitem item;
    //Propfind *find;
    //char *filename;
    char user[256];
    //int parentID = -10;
    char check_path[NORMALSIZE];

    memset(&item,0,sizeof(Transitem));
    memset(user,0,sizeof(user));
    memset(check_path,0,sizeof(check_path));
    //memset(filename,0,sizeof(filename));

    FILE *fp;

    char buffer[NORMALSIZE];
    const char *split = ",";
    char *p;

    memset(buffer, '\0', sizeof(buffer));

    if (access(path,0) != 0)
        return -1;

    if(( fp = fopen(path,"rb"))==NULL)
    {
        fprintf(stderr,"read Cloud error");
    }
    while(fgets(buffer,NORMALSIZE,fp)!=NULL)
    {
        p=strtok(buffer,split);
        int i=0;
        while(p!=NULL)
        {
            switch (i)
            {
                //case 0:
                //strcpy(user,p);
                //if(strcmp(user,username) != 0) // is not same user
                //return;
                //break;
            case 0 :
                break;
            case 1:
                item.id = atoi(p);
                break;
            case 2:
                if(type == UPLOAD) //upload
                    strcpy(item.transid,p);
                else
                    item.size = atoll(p);
                break;
            case 3:
                strcpy(item.name,p);

            default:
                break;
            }

            i++;
            p=strtok(NULL,split);
        }

        if( item.name[ strlen(item.name)-1 ] == '\n' )
            item.name[ strlen(item.name)-1 ] = '\0';

        if(item.id > 0)
        {

            if(type == UPLOAD)
            {
                printf("init upload file is %s\n",item.name);
                uploadFile(item.name,item.id,item.transid,0);
            }
#if 0
            else
            {
                //printf("download file id is %d ,size is %lld,name is %s\n",item.id,item.size,item.name);

                /*
                 if(remove(item.name) != -1name[])
                     printf("remove ok\n");
                 else
                     printf("remove fail\n");
                  */

                filename = parse_name_from_path(item.name);

                strncpy(check_path,item.name,strlen(item.name)-strlen(filename)-1);

                parentID = getParentID(check_path);

                if(parentID <= 0)
                {
                    printf("obtain %s parent ID is fai\n",item.name);
                    my_free(filename);
                    fclose(fp);
                    return -1;
                }

                if(filename != NULL)
                {
                    //printf("filename is %s\n",filename);
                    //init_fail_item = 1;
                    find = checkEntryExisted(username,parentID,filename,"system.file");
                    if(find != NULL)
                    {
                        if(find->status == 0 && strcmp(find->type,"system.file") == 0)
                            downloadFile(item.id,item.name,item.size,0);
                        my_free(find);
                    }

                    my_free(filename);


                }


            }
#endif
        }


    }

    fclose(fp);


    return 0;

    //printf("type is %d,status is %d,username is %s,password is %s,rule is %d,base_path is %s\n",
    //type,status,username,password,rule,base_path);
}

void init_up_excep_fail(char *path)
{
    FILE *fp;

    char buffer[NORMALSIZE];
    const char *split = ",";
    char *p;
    char name[NORMALSIZE];
    char action[256];

    memset(buffer, '\0', sizeof(buffer));

    if(( fp = fopen(path,"rb"))==NULL)
    {
        fprintf(stderr,"read Cloud error");
    }
    while(fgets(buffer,NORMALSIZE,fp)!=NULL)
    {
        if(strlen(buffer) <= 0)
            continue;

        memset(name,0,sizeof(name));

        p=strtok(buffer,split);
        int i=0;
        while(p!=NULL)
        {
            switch (i)
            {
            case 0 :
                strcpy(action,p);
                break;
            case 1:
                strcpy(name,p);

            default:
                break;
            }

            i++;
            p=strtok(NULL,split);
        }

        if(name[strlen(name)-1] == '\n')
            name[strlen(name)-1] = '\0';

        if(strlen(action) && strlen(name))
            add_sync_item(action,name,up_excep_fail);

    }
    fclose(fp);
    //print_all_sync_item(up_excep_fail);
}

int test_if_dir_empty(char *dir)
{
    struct dirent* ent = NULL;
    DIR *pDir;
    int i = 0;
    pDir=opendir(dir);

    if(pDir != NULL )
    {
        while (NULL != (ent=readdir(pDir)))
        {

            //if(ent->d_name[0] == '.')
                //continue;

            if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
                continue;
            i++;

        }
        closedir(pDir);
    }

    return  (i == 0) ? 1 : 0;
}

int test_if_file_up_excep_fail(char *name)
{
    if(item_empty(up_excep_fail))
    {
        //printf("up_excep_fail is NULL\n");
    	return 0;
    }


    //printf("test_if_file_up_excep_fail name = %s\n",name);

    sync_item_t p = up_excep_fail->next;

    while(p != NULL)
    {
        //printf("name = %s,p->name= %s\n",p->name);
        if( !strcmp(p->name,name) )
        {
#ifdef DEBUG
            //printf("%s exist in up excep fail\n",name);
#endif
            return 1;
        }
        p = p->next;
    }

    return 0;
}

void free_local_fileslist(Local *local)
{

    int i;
    if(local->filenum > 0)
    {
        for( i = 0; i<local->filenum; i++ )
        {
            my_free(local->filelist[i]->name);
            my_free((local->filelist)[i]);
        }
        my_free(local->filelist);

    }
}

void free_local_folderslist(Local *local)
{
    int i;
    if(local->foldernum > 0)
    {
        for( i = 0; i<local->foldernum; i++ )
        {
            my_free(local->folderlist[i]->name);
            my_free((local->folderlist)[i]);
        }
        my_free(local->folderlist);
    }
}

void free_local_list(Local *local)
{
    //my_free(local->filelist);
    //my_free(local->folderlist);

    int i;
    //printf("folder num is %d,file num is %d\n",local->foldernum,local->filenum);
    if(local->filenum > 0)
    {
        for( i = 0; i<local->filenum; i++ )
        {
            //printf("free file is %d\n",i);
            my_free(local->filelist[i]->name);
            my_free((local->filelist)[i]);
        }
        my_free(local->filelist);

    }

    if(local->foldernum > 0)
    {
        for( i = 0; i<local->foldernum; i++ )
        {
            my_free(local->folderlist[i]->name);
            my_free((local->folderlist)[i]);
        }
        my_free(local->folderlist);
    }

}

void free_server_fileslist(Browse *br)
{
    //my_free(br->filelist);
    //my_free(br->folderlist);

    //printf("folder num is %d,file num is %d\n",br->foldernumber,br->filenumber);
    int i;
    if(br->filenumber > 0)
    {
        for( i = 0; i<br->filenumber; i++ )
        {   //printf("free file is %d\n",i);
            my_free(br->filelist[i]->display);
            my_free((br->filelist)[i]);
        }
        my_free(br->filelist);
        br->filenumber = 0;
    }
}

void free_server_folderslist(Browse *br)
{
    int i;
    if(br->foldernumber > 0)
       {
           for( i = 0; i<br->foldernumber; i++ )
           {
               my_free(br->folderlist[i]->display);
               my_free((br->folderlist)[i]);
           }
           my_free(br->folderlist);
           br->foldernumber = 0;
       }
}

void free_server_list(Browse *br)
{
    //my_free(br->filelist);
    //my_free(br->folderlist);

    //printf("folder num is %d,file num is %d\n",br->foldernumber,br->filenumber);
    int i;
    if(br->filenumber > 0)
    {
        for( i = 0; i<br->filenumber; i++ )
        {   //printf("free file is %d\n",i);
            my_free(br->filelist[i]->display);
            my_free((br->filelist)[i]);
        }
        my_free(br->filelist);
    }

    if(br->foldernumber > 0)
    {
        for( i = 0; i<br->foldernumber; i++ )
        {
            my_free(br->folderlist[i]->display);
            my_free((br->folderlist)[i]);
        }
        my_free(br->folderlist);
    }

}


/*
int is_copying_finished(char *filename)
{
    unsigned long size1,size2;
    size1 = stat_file(filename);
    sleep(1);
    size2 = stat_file(filename);

    //printf("size1 is %ld,size2 is %ld\n",size1,size2);

    return (size2 - size1);
}*/

int my_mkdir(char *path)
{
    char error_message[NORMALSIZE];
    DIR *pDir;
    pDir=opendir(path);
    if(NULL == pDir)
    {
        if(-1 == mkdir(path,0777))
        {
            snprintf(error_message,NORMALSIZE,"mkdir %s fail\n",path);
            handle_error(S_MKDIR_FAIL,error_message);
            return -1;
        }
    }
    else
        closedir(pDir);

    return 0;
}

int my_mkdir_r(char *path)
{
    int i,len;
    char str[512];
    char fullname[512];


    memset(str,0,sizeof(str));

    len = strlen(mount_path);
    strcpy(str,path+len);

    //strncpy(str,path,512);
    len = strlen(str);
    for(i=0; i < len ; i++)
    {
        if(str[i] == '/' && i != 0)
        {
            str[i] = '\0';
            memset(fullname,0,sizeof(fullname));
            sprintf(fullname,"%s%s",mount_path,str);
            //printf("fullname=%s\n",fullname);
            if(access(fullname,F_OK) != 0)
            {
                if( my_mkdir(fullname) == -1)
                {
                    printf("mkdir %s fail\n",fullname);
                    return -1;
                }
            }
            str[i] = '/';
        }
    }


    memset(fullname,0,sizeof(fullname));
    sprintf(fullname,"%s%s",mount_path,str);

    if(len > 0 && access(fullname,F_OK) != 0)
    {
        if(my_mkdir(fullname) == -1)
            return -1;
    }

    return 0;

}

int is_number(char *str)
{
    if(str == NULL)
        return -1;

    if(!strcmp(str,"0"))
        return 0;

    int i;
    int len = strlen(str);
    for(i=0;i<len;i++)
    {
        if(!isdigit(str[i]))
            return 0;
    }

    if(i == len && i > 0)
        return 1;

    return 0;
}

int get_conflict_seq(char *name,int *num,int *count)
{
    char *p,*p1;
    char seq[64];
    int n = 0;

    p = strrchr(name,'(');

    if(p)
    {
        p1 = strchr(p,')');
        if(p1)
        {
            p++;
            memset(seq,0,sizeof(seq));
            strncpy(seq,p,strlen(p)-strlen(p1));
            //printf("seq=%s\n",seq);
            if(is_number(seq))
            {
                *num = atoi(seq);
                (*num)++;

                n = *num;
                while((n=(n/10)))
                {
                    count++;
                }
            }
        }
    }

    return 0;
}

char *get_confilicted_name_case(const char *fullname,const char *path,const char *pre_name,
                                const char *raw_name)
{
    char *confilicted_name = NULL;
    char prefix_name[NORMALSIZE];
    char suffix_name[256];
    char parse_name[NORMALSIZE];
    char *p = NULL;
    int  num = 0;
    char *filename = NULL;
    //char path[512];
    //int n = 0,j=0;
    char con_filename[256];
    int count = 0;
    char cmp_name[128] = {0};
    int isfolder = 0;
    char new_prefix_name[256] = {0};

    memset(prefix_name,0,sizeof(prefix_name));
    memset(suffix_name,0,sizeof(suffix_name));
    memset(parse_name,0,sizeof(parse_name));
    //memset(path,0,sizeof(path));
    memset(con_filename,0,sizeof(con_filename));

    isfolder = test_if_dir(fullname);

    filename = parse_name_from_path(fullname);
    if(NULL == filename)
    {
        handle_error(S_MEMORY_FAIL,"get_confilicted_name()");
        return NULL;
    }
//    strncpy(path,fullname,strlen(fullname)-strlen(filename)-1);

    confilicted_name = (char *)malloc(sizeof(char)*NORMALSIZE);
    if(NULL == confilicted_name)
    {
        handle_error(S_MEMORY_FAIL,"get_confilicted_name()");
        my_free(filename);
        return NULL;
    }

    p = strrchr(filename,'.');

    if(!isfolder && p && filename[0] != '.')
    {
        strncpy(parse_name,filename,strlen(filename)-strlen(p));
        strcpy(suffix_name,p);
        strncpy(prefix_name,pre_name,strlen(pre_name)-strlen(suffix_name));
    }
    else
    {
        strcpy(parse_name,filename);
        strcpy(prefix_name,pre_name);
    }

    //printf("filename=%s,pre_name=%s,parse_name=%s,prefix_name=%s\n",filename,pre_name,parse_name,prefix_name);

    get_conflict_seq(parse_name,&num,&count);

    //printf("num=%d,count=%d\n",num,count);

    if(num == 0)
    {
        sprintf(cmp_name,"(%s)",case_conflict_name);

        if(strstr(parse_name,cmp_name) && !strstr(raw_name,cmp_name))
        {
            memset(cmp_name,0,sizeof(cmp_name));
            sprintf(cmp_name,"(%s(1))",case_conflict_name);
        }
    }
    else
    {
       sprintf(cmp_name,"(%s(%d))",case_conflict_name,num);
    }

    //printf("cmp_name=%s\n",cmp_name);

    snprintf(new_prefix_name,252-strlen(cmp_name)-strlen(suffix_name),"%s",prefix_name);

    snprintf(con_filename,256,"%s%s%s",new_prefix_name,cmp_name,suffix_name);
    snprintf(confilicted_name,NORMALSIZE,"%s/%s",path,con_filename);

    //printf("------ prefix name is %s,num is %d,suffix name is %s -----\n",prefix_name,num,suffix_name);

    my_free(filename);

    return confilicted_name;
}

char *get_confilicted_name(const char *fullname,int isfolder)
{
    char *confilicted_name = NULL;
    char prefix_name[NORMALSIZE];
    char suffix_name[256];
    char parse_name[NORMALSIZE];
    char *p = NULL;
    //char *p1 = NULL;
    //char *p2 = NULL;
    //char seq[8];
    int  num = 0;
    //int have_suf = 0;
    char *filename = NULL;
    char path[512];
    int j=0;
    //char seq_num[8];
    char con_filename[256];
    int count = 0;
    char new_prefix_name[256] = {0};

    memset(prefix_name,0,sizeof(prefix_name));
    memset(suffix_name,0,sizeof(suffix_name));
    memset(parse_name,0,sizeof(parse_name));
    memset(path,0,sizeof(path));
    memset(con_filename,0,sizeof(con_filename));

    filename = parse_name_from_path(fullname);
    if(NULL == filename)
    {
        handle_error(S_MEMORY_FAIL,"get_confilicted_name()");
        return NULL;
    }
    strncpy(path,fullname,strlen(fullname)-strlen(filename)-1);

    confilicted_name = (char *)malloc(sizeof(char)*NORMALSIZE);
    if(NULL == confilicted_name)
    {
        handle_error(S_MEMORY_FAIL,"get_confilicted_name()");
        my_free(filename);
        return NULL;
    }

    p = strrchr(filename,'.');

    if(!isfolder && p && filename[0] != '.')
    {
        strncpy(parse_name,filename,strlen(filename)-strlen(p));
        strcpy(suffix_name,p);
    }
    else
        strcpy(parse_name,filename);

    get_conflict_seq(parse_name,&num,&count);

    printf("filename=%s,path=%s,num=%d,count=%d\n",filename,path,num,count);

    /*if(isfolder)
    {
        strcpy(parse_name,filename);
        get_conflict_seq(parse_name,&num,&count);
    }
    else
    {
        p = strrchr(filename,'.');

        //printf("p=%s\n",p);

        if(p && filename[0] != '.')
        {
            strncpy(parse_name,filename,strlen(filename)-strlen(p));
            strcpy(suffix_name,p);
            //have_suf = 1;

            p = NULL;

            p = strrchr(parse_name,'(');

            if(p)
            {
                p1 = strchr(p,')');
                if(p1)
                {
                    p++;
                    memset(seq,0,sizeof(seq));
                    strncpy(seq,p,strlen(p)-strlen(p1));
                    if(is_number(seq))
                    {
                        num = atoi(seq);
                        num++;
                        //printf("seq is %s,num is %d\n",seq,num);
                        n = num;
                        while((n=(n/10)))
                        {
                            j++;
                        }


                        strncpy(prefix_name,parse_name,strlen(parse_name)-strlen(p)-1);
                    }
                }
            }
        }
        else
        {
            strcpy(parse_name,filename);
            get_conflict_seq(parse_name,&num,&count);
        }
    }*/

    printf("parse_name=%s,suffix_name=%s\n",parse_name,suffix_name);

    if(num == 0)
    {
        num = 1;
        strcpy(prefix_name,parse_name);
    }
    else
    {
        p = strrchr(parse_name,'(');
        strncpy(prefix_name,parse_name,strlen(parse_name)-strlen(p));
    }

    snprintf(new_prefix_name,252-j-strlen(suffix_name),"%s",prefix_name);
    snprintf(con_filename,256,"%s(%d)%s",new_prefix_name,num,suffix_name);
    snprintf(confilicted_name,NORMALSIZE,"%s/%s",path,con_filename);

    //printf("------ prefix name is %s,num is %d,suffix name is %s -----\n",prefix_name,num,suffix_name);

    my_free(filename);

    return confilicted_name;
}


int test_if_download_temp_file(char *filename)
{
    char file_suffix[9];
    char *temp_suffix = ".asus.td";
    memset(file_suffix,0,sizeof(file_suffix));
    char *p = filename;

    if(strstr(filename,temp_suffix))
    {
        strcpy(file_suffix,p+(strlen(filename)-strlen(temp_suffix)));

        //printf(" %s file_suffix is %s\n",filename,file_suffix);

        if(!strcmp(file_suffix,temp_suffix))
            return 1;
    }

    return 0;

}

void enter_sleep_time(int num,my_mutex_t *mutex)
{
    struct timespec timeout;
    if(num > 1000)
    {
        usleep(num);
    }
    else
    {
        if(!exit_loop)
        {
            timeout.tv_sec = time(NULL) + num;
            timeout.tv_nsec = 0;
            //int status = -10;
            //pthread_mutex_lock(&wait_sleep_mutex.mutex);
            pthread_mutex_lock(&(mutex->mutex));
            //pthread_cond_timedwait(&wait_sleep_mutex.cond,&wait_sleep_mutex.mutex,&timeout);
            pthread_cond_timedwait(&(mutex->cond),&(mutex->mutex),&timeout);
            //pthread_mutex_unlock(&wait_sleep_mutex.mutex);
            pthread_mutex_unlock(&(mutex->mutex));
        }
    }
}

/* disk info function*/
//#ifdef IPKG
//#if 1
#ifdef IPKG
int check_disk_change()
{
    int status = -1;
    disk_change = 0;
    //status = check_sync_disk_removed();
    status = check_token_file(&cfg);
#ifdef DEBUG
    printf("check disk change status=%d \n",status);
#endif
    if(status == 2 || status ==1)
    {
        exit_loop = 1;
        //sync_up = 0;
        //sync_down = 0;
        //sync_disk_removed = status;
    }

    return 0;
}

int get_mounts_info(struct mounts_info_tag *info)
{
    int len = 0;
    FILE *fp;
    int i = 0;
    int num = 0;
    //char *mount_path;
    char **tmp_mount_list, **tmp_mount;

    char buf[len+1];
    memset(buf,'\0',sizeof(buf));
    char a[1024];
    char *p,*q;
    fp = fopen("/proc/mounts","r");
    if(fp == NULL)
    {
        printf("open /proc/mounts fail \n");
        return -1;
    }
    if(fp)
    {
        while(!feof(fp))
        {
            memset(a,'\0',sizeof(a));
            fscanf(fp,"%[^\n]%*c",a);
            if((strlen(a) != 0)&&((p=strstr(a,"/dev/sd")) != NULL))
            {
                if((q=strstr(p,"/tmp")) != NULL)
                {
                    if((p=strstr(q," ")) != NULL)
                    {
                        len = strlen(q) - strlen(p);
                        tmp_mount = (char **)malloc(sizeof(char *)*(num+1));
                        if(tmp_mount == NULL)
                        {
                            fclose(fp);
                            return -1;
                        }

                        tmp_mount[num] = (char *)malloc(sizeof(char)*(len+1));
                        memset(tmp_mount[num],0,sizeof(char)*(len+1));
                        if(tmp_mount[num] == NULL)
                        {
                            free(tmp_mount);
                            fclose(fp);
                            return -1;
                        }
                        strncpy(tmp_mount[num],q,len);
#ifdef DEBUG
                        printf("tmp_mount[%d]=%s###\n",num,tmp_mount[num]);
#endif
                        if(num != 0)
                        {
                            for(i = 0; i < num; ++i)
                                tmp_mount[i] = tmp_mount_list[i];

                            free(tmp_mount_list);
                            tmp_mount_list = tmp_mount;
                        }
                        else
                            tmp_mount_list = tmp_mount;

                        ++num;
                   }
                }
            }
        }
    }
    fclose(fp);

    info->paths = tmp_mount_list;
    info->num = num;
#ifdef DEBUG
    for(i=0;i<info->num;i++)
        printf("path[%d]=%s###\n",i,info->paths[i]);
#endif
}

int check_token_file(struct asus_config *cfg)
{
    int i;
    //char filename[256];
    char fullname[256];
    char cmp_mount_path[256];
    int removed = 0;
    int find = 0;
    char new_sync_path[256];
    char new_mount_path[128];
    char tmp_path[256];
    struct mounts_info_tag *info = NULL;
    int status = 0;

    info = (struct mounts_info_tag *)malloc(sizeof(struct mounts_info_tag));
    if(info == NULL)
    {
        printf("obtain memory space fail\n");
        return -1;
    }

    memset(info,0,sizeof(struct mounts_info_tag));

    if(get_mounts_info(info) == -1)
    {
        printf("get mounts info fail\n");
        my_free(info->paths);
        return -1;
    }


    //memset(filename,0,sizeof(filename));
    memset(fullname,0,sizeof(fullname));

    //sprintf(filename,".__smartsync_0_%s_%s",cfg->user,cfg->sync_path);
    //get_token_filename(filename,cfg->sync_path);
    for(i=0;i<info->num;++i)
    {
       sprintf(fullname,"%s/%s",info->paths[i],token_filename);
#ifdef DEBUG
       printf("token_fullname=%s\n",fullname);
#endif
       if(access(fullname,F_OK) == 0)
       {
           find = 1;
           memset(cmp_mount_path,0,sizeof(cmp_mount_path));
           if(!strncmp(mount_path,"/tmp",4))
           {
              strcpy(cmp_mount_path,mount_path);
              strcpy(new_mount_path,info->paths[i]);
              //has_tmp = 1;
           }
           else
           {
              sprintf(cmp_mount_path,"/tmp%s",mount_path);
              strncpy(new_mount_path,info->paths[i]+4,strlen(info->paths[i])-4);
           }
#ifdef DEBUG
           printf("cmp_mount_path=%s,new_mount_path=%s\n",cmp_mount_path,new_mount_path);
           printf("info->paths[%d] = %s \n",i,info->paths[i]);
#endif

           if(strcmp(cmp_mount_path,info->paths[i]))
           {
               //memset(mount_path,0,sizeof(mount_path));
               //strcpy(mount_path,new_mount_path);
               removed = 2;
               //del old token file

               //memset(fullname,0,sizeof(fullname));
               //sprintf(filename,"%s/%s",mount_path,)
               //get_token_filename(filename,sync_path);

               //sprintf(fullname,"%s/%s",info->paths[i],filename);
#if 0
#ifdef IPKG

#ifdef DEBUG
               printf("remove %s \n",fullname);
#endif
               if(remove(fullname) == -1)
               {
                   printf("remove %s fail\n",fullname);
               }
#else
               del_old_token_file(info->paths[i]);
#endif
#endif

               memset(new_sync_path,0,sizeof(new_sync_path));
               memset(tmp_path,0,sizeof(tmp_path));
               //len = strlen(mount_path);
               strcpy(tmp_path,sync_path+strlen(mount_path));
               sprintf(new_sync_path,"%s%s",new_mount_path,tmp_path);
#ifdef DEBUG
               printf("new_sync_path=%s\n",new_sync_path);
#endif
               //rewrite_config(CONFIG_PATH,new_sync_path);
               memset(sync_path,0,sizeof(sync_path));
               strcpy(sync_path,new_sync_path);
               //m_path = get_mount_path(sync_path,4);
               memset(mount_path,0,sizeof(mount_path));
               strcpy(mount_path,new_mount_path);

               memset(cfg->sync_path,0,sizeof(cfg->sync_path));
               strcpy(cfg->sync_path,new_sync_path);
               //my_free(m_path);

               //rewrite_config(CONFIG_PATH,new_sync_path);
               //write_token_file(new_mount_path,cfg);
               //memset(filename,0,sizeof(filename));
               //memset(token_filename,0,sizeof(token_filename));
              // get_token_filename(token_filename,new_sync_path);

#if 0               //write_token_file(new_mount_path,new_sync_path);
#ifdef IPKG
               remove(record_token_file);
               record_token_to_file(record_token_file,cfg);
#else
               write_to_nvram(token_filename,NVRAM_TOKENFILE);
#endif
#endif               //memset(fullname,0,sizeof(fullname));
               //sprintf(fullname,"%s/%s",new_mount_path,)

           }
           break;

       }
       //else
       //{

       //}
    }

    for(i=0;i<info->num;++i)
    {
        my_free(info->paths[i]);
    }
    my_free(info->paths);

    if(!find)
    {
#ifdef IPKG
       status = check_record_token_file(record_token_file,token_filename);
#else
       status = check_nvram_token_file(token_filename);
#endif
       if(status == 0)  // first start
       {
           cfg->sync_disk_exist = 1;
#ifdef IPKG
          memset(record_token_file,0,sizeof(record_token_file));
          sprintf(record_token_file,"/opt/etc/.smartsync/asuswebstorage_%s",cfg->user);
          my_mkdir("/opt/etc/.smartsync");
          record_token_to_file(record_token_file,token_filename);
#else
          //del_old_token_file(mount_path);
          write_to_nvram(token_filename,NVRAM_TOKENFILE);
#endif
          write_token_file(mount_path,token_filename);
           return 1;
       }
       else if(status == 1) // new disk
           return 2;
    }
    else
    {
        cfg->sync_disk_exist = 1;
    }
       //removed = 1 ;
    //return removed;
    return 0;
}

int check_record_token_file(char *record_filename,char *token_filename)
{
    FILE *fp;
    char buffer[256];
    int find = 0;
    //char token_filename[256];

    memset(buffer,0,sizeof(buffer));
    //memset(token_filename,0,sizeof(token_filename));

    if(access(record_filename,F_OK) == 0)
    {
       fp = fopen(record_filename,"r");
       if(fp == NULL)
       {
           printf("open %s fail \n",record_filename);
           return -1;
       }
       //sprintf(token_filename,".__smartsync_0_%s_%s",cfg->user,cfg->sync_path);
       //get_token_filename(token_filename,cfg->sync_path);
       while(fgets(buffer,256,fp)!=NULL)
       {
          if(buffer[strlen(buffer)-1] == '\n')
              buffer[strlen(buffer)-1] = '\0';
          if(!strcmp(buffer,token_filename))
          {
              find = 1;
              break;
          }
       }
       fclose(fp);
    }

    return find;
}


int write_token_file(char *path,char *filename)
{
   FILE *fp;
   //char filename[256];
   char fullname[512] = {0};

   //memset(filename,0,sizeof(filename));
   //memset(fullname,0,sizeof(fullname));
   //sprintf(filename,".__smartsync_0_%s_%s",cfg->user,cfg->sync_path);
   //get_token_filename(filename,cfg->sync_path);
   sprintf(fullname,"%s/%s",path,filename);

   fp = fopen(fullname,"w");
   if(fp == NULL)
   {
     printf("open %s fail \n",fullname);
     return -1;
   }

   fclose(fp);

   return 0;
}

int record_token_to_file(char *fullname,char *token_filename)
{
    FILE *fp;
    //char filename[256];
    //char fullname[512];
    //memset(filename,0,sizeof(filename));
    //memset(fullname,0,sizeof(fullname));
    //sprintf(filename,".__smartsync_0_%s_%s",cfg->user,cfg->sync_path);
    //get_token_filename(filename,cfg->sync_path);
    //sprintf(fullname,"%s/%s",path,filename);

    fp = fopen(fullname,"w");
    if(fp == NULL)
    {
      printf("open %s fail \n",fullname);
      return -1;
    }
    fprintf(fp,"%s",token_filename);

    fclose(fp);

    return 0;
}

#if 0
int get_token_filename(char *filename,char *sync_path)
{
   char encode_sync_path[256];
   int i;
   int len;
   memset(encode_sync_path,0,sizeof(encode_sync_path));
   strcpy(encode_sync_path,sync_path);
   len = strlen(encode_sync_path);
   for(i=0;i<len;i++)
   {
       if(encode_sync_path[i] == '/')
           encode_sync_path[i] = '@';
   }

   //strcpy(filename,encode_sync_path);
   sprintf(filename,".__smartsync_0_%s_%s",username,encode_sync_path);

#ifdef DEBUG
   printf("token_filename=%s\n",filename);
#endif

   return 0;

}
#endif

#if 0
int rewrite_config(char *config_path,char *new_sync_path)
{
    //struct asus_config cfg;
    FILE *fp;
    //memset(&cfg,0,sizeof(struct asus_config));
    //parse_config_new(config_path,&cfg);
    //printf("rewrite config read end\n");
    fp = fopen(config_path,"w");
    if(NULL==fp)
    {
       printf("open %s config file fail\n",config_path);
       return -1;
    }
    fprintf(fp,"%d\n%d\n%s\n%s\n%s\n%d\n%s\n",cfg.type,cfg.enable,cfg.user,cfg.pwd,
                                              cfg.url,cfg.rule,new_sync_path);
    fclose(fp);
    return 0;
}
#endif

#endif

#ifdef IPKG
int write_get_nvram_script(char *nvram_name,char *nvram_path,char *script_path)
{

printf("contents %s , , , \n",nvram_name);

    FILE *fp;
    char contents[512];
    memset(contents,0,512);

    printf("contents %s , , , \n",nvram_name);

    printf(contents,"#! /bin/sh\nNV=`nvram get %s`\nif [ ! -f \"%s\" ]; then\n   touch %s\nfi\necho \"$NV\" >%s",nvram_name,nvram_path,nvram_path,nvram_path);

    sprintf(contents,"#! /bin/sh\nNV=`nvram get %s`\nif [ ! -f \"%s\" ]; then\n   touch %s\nfi\necho \"$NV\" >%s",nvram_name,nvram_path,nvram_path,nvram_path);

    if(( fp = fopen(script_path,"w"))==NULL)
    {
        fprintf(stderr,"create webdav_get_nvram file error\n");
        return -1;
    }

    fprintf(fp,"%s",contents);
    fclose(fp);

    return 0;
}

int create_asuswebstorage_conf_file(char *config_path)
{

#ifdef DEBUG
    printf("enter convert_nvram_to_file_mutidir function\n");
#endif
    FILE *fp;
    char *nv, *nvp, *b;
    int i;
    //int j = 0;
    char *p;
    char *buffer;
    char *buf;

    fp = fopen("/tmp/smartsync/asuswebstorage/config/asuswebstorage_tmpconfig","r");
    if (fp==NULL) return -1;

    nvp = malloc(1024);
    memset(nvp,0,1024);
    fgets(nvp,1024,fp);
    fclose(fp);

    nv = nvp;

    fp=fopen(config_path, "w");
    if (fp==NULL) return -1;

    if(nvp[strlen(nvp)-1] == '\n')
        nvp[strlen(nvp)-1] = '\0';

    if(nv)
    {
        while ((b = strsep(&nvp, "<")) != NULL)
        {
            i = 0;
            buf = buffer = strdup(b);
#ifdef DEBUG
            printf("buffer = %s\n",buffer);
#endif
            while((p = strsep(&buffer,">")) != NULL)
            {
                //printf("p = %s\n",p);

                if (*p == 0)
                {
                    fprintf(fp,"\n");
                    i++;
                    continue;
                }
                if(i == 0)
                {
                    if(atoi(p) != 0)
                        break;

                    fprintf(fp,"%s",p);
                }
                else
                {
                    fprintf(fp,"\n%s",p);
                }
                i++;
            }
            free(buf);

        }

        free(nv);
    }
#ifdef DEBUG
    else
        printf("get nvram fail\n");
#endif
    fclose(fp);
    return 0;

}
#else
/*Type>Desc>URL>Rule>capacha>LocalFolder*/
int convert_nvram_to_file(char *file)
{
    //printf("enter convert_nvram_to_file_mutidir function\n");

    FILE *fp;
    char *nv, *nvp, *b;
    //struct asus_config config;
    int i;
    //int j = 0;
    //int status;
    char *p;
    char *buffer;
    char *buf;

    fp=fopen(file, "w");

    if (fp==NULL) return -1;

    //nv = nvp = strdup(nvram_safe_get("cloud_sync"));
#ifndef USE_TCAPI
    //nv = nvp = strdup(nvram_safe_get("cloud_sync")); // markcool modify
#else
    char tmp[MAXLEN_TCAPI_MSG] = {0};
    tcapi_get(AICLOUD, "cloud_sync", tmp);
    nv = nvp = my_str_malloc(strlen(tmp)+1);
    sprintf(nv,"%s",tmp);
#endif

    //printf("otain nvram end\n");

    if(nv == NULL)
    {
       printf("nvram is NULL\n");
       return -1;
    }

    if(nv)
    {
        while ((b = strsep(&nvp, "<")) != NULL)
        {
            i = 0;
            buf = buffer = strdup(b);

            //printf("buffer = %s\n",buffer);

            while((p = strsep(&buffer,">")) != NULL)
            {
                //printf("p = %s\n",p);

                if (*p == 0)
                {
                    fprintf(fp,"\n");
                    i++;
                    continue;
                }
                if(i == 0)
                {
                    if(atoi(p) != 0)
                        break;

                     fprintf(fp,"%s",p);
                }
                else
                {
                    fprintf(fp,"\n%s",p);
                }
                i++;
            }
            free(buf);

        }
        free(nv);
    }
    //else

    fclose(fp);
    //printf("end convert_nvram_to_file_mutidir function\n");
    return 0;
}
/*
int convert_nvram_to_file(char *file)
{
    FILE *fp;
    char *nv = NULL, *nvp =NULL, *b = NULL;
    char *type, *url ,*user,*pwd,*s_path,*rule, *enable;
    int len;

    fp=fopen(file, "w");

    if (fp==NULL)
    {
        printf("open %s fail\n",file);
        return -1;
    }

    nv = nvp = strdup(nvram_safe_get("cloud_sync"));

    if(nv == NULL)
    {
        printf("cloud_sync is null\n");
        return -2;
    }

    if(nv) {
        while ((b = strsep(&nvp, "<")) != NULL) {
            if((vstrsep(b, ">", &type, &user,&pwd,&url,&rule,&s_path,&enable)!=7)) continue;
            if(strlen(user)==0||strlen(pwd)==0) continue;
#ifdef DEBUG
            printf("%s,%s,%s,%s,%s,%s,%s\n",type,enable,user,pwd,url,rule,s_path);
#endif
            len = strlen(s_path);
            if(s_path[len-1] == '/')
                s_path[len-1] = '\0';
            //fprintf(fp, "%s,%s,%s,%s,%s,%s,%s\n",type,enable,user,pwd,url,rule,s_path);
            fprintf(fp, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n",type,enable,user,pwd,url,rule,s_path);
        }
        free(nv);
    }
    fclose(fp);

    return 0;

}*/

#if 0
int create_shell_file()
{
    FILE *fp;
    char contents[256];
    memset(contents,0,256);
    strcpy(contents,"#! /bin/sh\nnvram set $2=\"$1\"\nnvram commit");

    if(( fp = fopen(SHELL_FILE,"w"))==NULL)
    {
        fprintf(stderr,"create shell file error");
        return -1;
    }

    fprintf(fp,"%s",contents);
    fclose(fp);
    return 0;

}

int write_to_nvram(char *contents,char *nv_name)
{
    char *command = NULL;
    command = malloc(strlen(contents)+strlen(SHELL_FILE)+strlen(nv_name)+8);
    if(command == NULL)
    {
        printf("create memory fail\n");
        return -1;
    }
    sprintf(command,"sh %s \"%s\" %s",SHELL_FILE,contents,nv_name);
#ifdef DEBUG
    printf("command : [%s]\n",command);
#endif
    system(command);
    free(command);

    return 0;
}

int del_old_token_file(char *path)
{
    char *nv = NULL;
    char fullname[512] = {0};
    nv = strdup(nvram_safe_get(NVRAM_TOKENFILE));
    if(nv == NULL)
         return 0;
    sprintf(fullname,"%s/%s",path,nv);
    if(remove(fullname) == -1)
         return 0;
    return 1;
}

int check_nvram_token_file(char *token_filename)
{
    char *nv = NULL;
    nv = strdup(nvram_safe_get(NVRAM_TOKENFILE));
    if(nv == NULL)
         return 0;

    /*if(strlen(nv) == 0)
    {
        my_free(nv);
        return 0;
    }*/

    if(!strcmp(token_filename,nv))
    {
        my_free(nv);
        return 1;
    }

   my_free(nv);
   return 0;
}
#endif


#endif

//#ifdef IPKG
int check_accout_status()
{
#ifdef DEBUG
    printf("enter check_accout_status \n");
#endif
    struct asus_config stop_cfg;

    memset(&stop_cfg,0,sizeof(struct asus_config));
    int status = 0;
    int clean_token = 0;

#ifdef IPKG
    char fullname[512] = {0};
    sprintf(fullname,"%s/%s",mount_path,token_filename);
#ifdef DEBUG
    printf("token_fullname=%s\n",fullname);
#endif
    system("sh /tmp/smartsync/asuswebstorage/script/asuswebstorage_get_nvram");
    if(create_asuswebstorage_conf_file(CONFIG_PATH) == -1)
    {
        printf("create_asuswebstorage_conf_file fail\n");
        return;
    }
    parse_config_onexit(CONFIG_PATH,&stop_cfg);

    if(strlen(stop_cfg.user) == 0)
    {
#ifdef DEBUG
        printf("remove %s \n",fullname);
#endif
        clean_token = 1;
        remove(fullname);
        remove(record_token_file);
    }

#else
    status = convert_nvram_to_file(CONFIG_PATH);
    if(status == -2 || status == -1) // -2---nvram is null ; -1------open config fail
        clean_token = 1;
    else
    {
        status = parse_config_onexit(CONFIG_PATH,&stop_cfg);
        if(status == -1)
        {
           clean_token = 1;
        }
        else if(strlen(stop_cfg.user) == 0)
        {
           clean_token = 1;
        }
    }
    printf("clean_token=%d\n",clean_token);
    /*if(clean_token)
    {
        remove(fullname);
        write_to_nvram("",NVRAM_TOKENFILE);
    }*/
#endif

    if(clean_token)
    {
        clean_download_temp_file(down_head);
        remove(system_token);
    }

    return 0;

}
//#endif

void parse_otp_and_captcha(char *str,struct asus_config *cfg)
{
    char *p = NULL;
    if(!strcmp(str,"none"))
        return;

    p = strchr(str,'#');
    if(p == NULL)
        return;

    if(str[0] == '#')                  //only captcha
        strcpy(cfg->captcha,p+1);
    else if(strlen(p) == 1)            //only otp_key
        strncpy(cfg->otp_key,str,strlen(str)-1);
    else                               //otp and captcha
    {
        strncpy(cfg->otp_key,str,strlen(str)-strlen(p));
        strcpy(cfg->captcha,p+1);
    }

    printf("otp_key=%s,captcha=%s\n",cfg->otp_key,cfg->captcha);

}


int parse_config_new(char *path,struct asus_config *cfg)
{
    int type;
    int status;
    int rule;
    FILE *fp;

    char buffer[256];
    int i=0;
    int len = 0;
    //const char *split = ",";
    //char *p;

    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(sync_path, 0, sizeof(sync_path));

    memset(buffer, '\0', sizeof(buffer));


    if (access(path,0) == 0)
    {
        if(( fp = fopen(path,"rb"))==NULL)
        {
            fprintf(stderr,"read Cloud error");
            return -1;
        }

        while(fgets(buffer,256,fp)!=NULL)
        {

            len = strlen(buffer);
            if(buffer[len-1] == '\n')
                buffer[len-1] = '\0';
            //printf("buffer is %s\n",buffer);
            //p=strtok(buffer,split);

            //while(p!=NULL)
            //{

            switch (i)
            {
            case 0 :
                type = atoi(buffer);
                printf("type is %d\n",type);
                if( type != 0)
                    return 1;
                if(cfg!=NULL)
                    cfg->type = type;
                break;
            case 1:
                printf("buffer username is %s\n",buffer);
                strcpy(username,buffer);
                if(cfg!=NULL)
                    strcpy(cfg->user,username);
                break;
            case 2:
                strcpy(password,buffer);
				printf("buffer password is %s\n",password);
					
                if(cfg!=NULL)
                    strcpy(cfg->pwd,password);
                break;
            case 3:
                printf("p4 is %s\n",buffer);

                parse_otp_and_captcha(buffer,cfg);
                if(cfg!=NULL)
                    strcpy(cfg->url,buffer);
                break;
            case 4:
                //printf("p5 is %s\n",p);
                rule = atoi(buffer);
				printf("p5 is rule : %d\n",rule);
                if(cfg!=NULL)
                    cfg->rule = rule;

                //if( rule == 0 )
                //{
                //sync_up = 1;
                //sync_down = 1;
                //}
                if( rule == 1)
                {
                    //sync_up = 1;
                    //sync_down = 0;
                    download_only = 1;
                }
                else if( rule == 2)
                {
                    //sync_up = 0;
                    //sync_down = 1;
                    upload_only = 1;
                }

                break;
          case 5:
                //printf("p6 is sync_path : %s\n",p);
                strcpy(sync_path,buffer);

				printf("p6 is sync_path : %s\n",sync_path);
                if( sync_path[ strlen(sync_path)-1 ] == '\n' )
                    sync_path[ strlen(sync_path)-1 ] = '\0';
                if(cfg!=NULL)
                    strcpy(cfg->sync_path,sync_path);
                break;
            case 6:
                status = atoi(buffer);
                printf("status is %d\n",status);
                if(status != 1)
                    return 1;
                if(cfg!=NULL)
                    cfg->enable = status;
                break;
           default:
                break;
                //}


                //p=strtok(NULL,split);
            }
            i++;
            memset(buffer, '\0', sizeof(buffer));

        }

        fclose(fp);
    }
    else
        return -1;

    return 0;

    //printf("type is %d,status is %d,username is %s,password is %s,rule is %d,base_path is %s\n",
    //type,status,username,password,rule,base_path);
}


// modify by markcool
int parse_config_copy(struct asuswebstorage_conf *asus_conf,struct asus_config *cfg)
{
   

    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(sync_path, 0, sizeof(sync_path));


	//strcpy(&cfg.type , 0);
	cfg->type = 0;
	strcpy(cfg->user,asus_conf->username);
	strcpy(cfg->pwd,asus_conf->password);
  parse_otp_and_captcha("none",cfg);
  if(cfg!=NULL)
    strcpy(cfg->url,"none");

	if(cfg!=NULL)
    cfg->rule = 2;
	
	int rule = 2;
	if( rule == 1) {
		download_only = 1;
	} else if( rule == 2) {
		upload_only = 1;
	}
	
	strcpy(sync_path,asus_conf->uploader_path);

	if( sync_path[ strlen(sync_path)-1 ] == '\n' )
		sync_path[ strlen(sync_path)-1 ] = '\0';
	if(cfg!=NULL)
		strcpy(cfg->sync_path,sync_path);



	
	cfg->enable = 1;
	
	strcpy(username,asus_conf->username);
	strcpy(password,asus_conf->password);
	strcpy(sync_path,asus_conf->uploader_path);

  return 0;
}

//#ifdef IPKG
int parse_config_onexit(char *path, struct asus_config *cfg)
{
    //int type;
    //int status;
    FILE *fp;

    char buffer[256];
    int i=0;
    int len = 0;

    memset(buffer, '\0', sizeof(buffer));

    if (access(path,0) == 0)
    {
        if(( fp = fopen(path,"rb"))==NULL)
        {
            fprintf(stderr,"read Cloud error");
            return -1;
        }

        while(fgets(buffer,256,fp)!=NULL)
        {
            len = strlen(buffer);
            if(buffer[len-1] == '\n')
                buffer[len-1] = '\0';

            switch (i)
            {
            case 2:
                    strcpy(cfg->user,buffer);
                break;
            case 3:
                    strcpy(cfg->pwd,buffer);
                break;
            default:
                break;
            }
            i++;
            memset(buffer, '\0', sizeof(buffer));

        }

        fclose(fp);
    }
    else
        return -1;

    return 0;
}

#ifdef IPKG
/*process running,return 1;else return 0*/
/*int detect_process(char * process_name)
{
    FILE *ptr;
    char buff[512];
    char ps[128];
    sprintf(ps,"ps | grep -c %s",process_name);
    strcpy(buff,"ABNORMAL");
    if((ptr=popen(ps, "r")) != NULL)
    {
        while (fgets(buff, 512, ptr) != NULL)
        {
            if(atoi(buff)>2)
            {
                pclose(ptr);
                return 1;
            }
        }
    }
    if(strcmp(buff,"ABNORMAL")==0)
    {
        return 0;
    }
    pclose(ptr);
    return 0;
}*/

int detect_process_file()
{
    struct dirent *ent = NULL;
    DIR *pdir;
    int num = 0;

    unlink("/tmp/smartsync_app/asuswebstorage_start");

    pdir = opendir("/tmp/smartsync_app");

    if(pdir != NULL)
    {
        while (NULL != (ent=readdir(pdir)))
        {
            //printf("%s is ent->d_name\n",ent->d_name);
            if(!strcmp(ent->d_name,".") || !strcmp(ent->d_name,".."))
                continue;
            num++;
        }
        closedir(pdir);
    }
    else
        return 0;

    if(num)
        return 1;

    return 0;
}

void create_start_file()
{
    my_mkdir("/tmp/smartsync_app");
    FILE *fp;
    fp = fopen("/tmp/smartsync_app/asuswebstorage_start","w");
    fclose(fp);
}

#endif

int  parse_sync_path(struct asus_config *cfg,char *path)
{
#ifdef DEBUG
    printf("path is %s\n",path);
#endif
    int i;
    int n;
    char temp_path[512];
    char *new_path = NULL;

    memset(temp_path,0,sizeof(temp_path));
    memset(mount_path,0,sizeof(mount_path));
    strcpy(temp_path,path);
    if(!strncmp(temp_path,"/tmp",4))
        n = 4;
    else
        n = 3 ;

    new_path = temp_path;

    for(i= 0;i< n ;i++)
    {
        new_path = strchr(new_path,'/');
        if(new_path == NULL)
            break;
        new_path++;
    }

    if( i > 3)
        strncpy(mount_path,temp_path,strlen(temp_path)-strlen(new_path)-1);
    else
        strcpy(mount_path,temp_path);

    strcpy(cfg->dir_name,temp_path+strlen(mount_path)+1);

#ifdef DEBUG
    printf("mount_path=%s,dir_name=%s\n",mount_path,cfg->dir_name);
#endif

    return 0;
}

int wait_handle_socket()
{
    if(receve_socket)
    {
        server_sync = 0;
        while(receve_socket || local_sync) /* 13/04/17 fix write log conflict to programm dead*/
        {
            //usleep(1000*100);
            enter_sleep_time(1000*100,NULL);
        }
        server_sync = 1;

        if(has_local_socket)
        {
            has_local_socket = 0;
            return -1;
        }

        return 0 ;
    }

    return 0;
}

int check_network_state()
{
    int link_flag = 0;
    //int i;

    //struct timeval now;
    //struct timespec outtime;

    while(!link_flag && !exit_loop)
    {
#ifdef IPKG
        system(SH_GET_NVRAM_SCRIPT_2);
        sleep(2);

        char nv[64] = {0};
        FILE *fp;
        fp = fopen(NVRAM_PATH_2,"r");
        if(fp == NULL)
            return -1;
        fgets(nv,sizeof(nv),fp);
        //DEBUG("nv=%s\n",nv);
        fclose(fp);

        link_flag = atoi(nv);

#else
        //char *link_internet = strdup(nvram_safe_get("link_internet"));
        char *link_internet = NULL;
#ifndef USE_TCAPI
        // link_internet = strdup(nvram_safe_get("link_internet")); // markcool modify
#else
        char tmp[MAXLEN_TCAPI_MSG] = {0};
        tcapi_get(WANDUCK, "link_internet", tmp);
        link_internet = my_str_malloc(strlen(tmp)+1);
        sprintf(link_internet,"%s",tmp);
#endif
        link_flag = atoi(link_internet);
        free(link_internet);
#endif
        //printf("will sleep 20 seconds\n");
        if(!link_flag)
        {
            IsNetworkUnlink = 1;
            write_log(S_ERROR,"Network Connection Failed","");
            enter_sleep_time(20,&my_mutex);
        }
    }

    if(!local_space_full && !server_space_full && !exit_loop && IsNetworkUnlink)
    {
        write_log(S_SYNC,"","");
        IsNetworkUnlink = 0;
    }

    IsSyncError = 0;

    return 0;
}

int clean_download_temp_file(struct sync_item *head)
{
#ifdef DEBUG
    printf("clean download temp file\n");
#endif
    sync_item_t p = head->next;

    while(p != NULL)
    {
        //printf("action = %s\n", p->action);
        //printf("name = %s\n", p->name);
#ifdef DEBUG
        printf("remove=%s\n",p->name);
#endif
        remove(p->name);
        p = p->next;

    }
    return 0;
}

void push_node(FolderNode *node,NodeStack **head)
{
    NodeStack *new_item = NULL;
    NodeStack *s_link = NULL;
    s_link = *head;
    new_item = (NodeStack *)malloc(sizeof(NodeStack));

    memset(new_item,0,sizeof(NodeStack));

    if(new_item == NULL)
    {
        printf("obtain memory fail\n");
        return;
    }

    new_item->point = node;
    new_item->next = s_link;
    s_link = new_item;
    *head = s_link;
}

FolderNode *pop_node(NodeStack **head)
{
    FolderNode *node = NULL;
    NodeStack *top = NULL;
    NodeStack *s_link = NULL;
    s_link = *head;
    if(s_link != NULL)
    {
        top = s_link;
        s_link = top->next;
        node = top->point;
        free(top);
        *head = s_link;
    }

    return node;
}

long long FileSize(const char* szFilename)
{
#ifdef HAVE_STAT64
        struct stat64 buffer;
        stat64(szFilename, &buffer);
#else
        struct stat buffer;
        stat(szFilename, &buffer);
#endif
        return buffer.st_size;
}

int LoadFileIntoBuffer(const char* szFileName, char** pBuffer, int* pBufferLength)
{
    FILE* pFile = fopen(szFileName, "rb");
    if (!pFile)
    {
        return -1;
    }

    // obtain file size.
    fseek(pFile , 0 , SEEK_END);
    int iSize  = ftell(pFile);
    rewind(pFile);

    // allocate memory to contain the whole file.
    *pBuffer = (char*) malloc(iSize + 1);
    if (!*pBuffer)
    {
        return -1;
    }

    // copy the file into the buffer.
    fread(*pBuffer, 1, iSize, pFile);

    fclose(pFile);

    (*pBuffer)[iSize] = 0;

    *pBufferLength = iSize + 1;

    return 0;
}

/*save cloud all folders id and seqnum*/
Hb_SubNode *create_node(int id,int seq)
{
    Hb_SubNode *node = NULL;
    node = (Hb_SubNode *)malloc(sizeof (Hb_SubNode));
    if(node == NULL)
    {
        printf("create memory error!\n");
        return NULL;
    }

    node->Child = NULL;
    node->NextBrother = NULL;
    node->id = id;
    node->seq = seq;

    return node;
}

Hb_SubNode *get_parent_node(int pid,Hb_SubNode *node)
{
    //printf("enter get_parent_node,node_id=%d\n",node->id);
    Hb_SubNode *p1 = NULL ,*p2 = NULL;

    if(node == NULL)
        return NULL;

    //printf("node_id=%d,id=%d\n",node->id,pid);
    if(node->id == pid)
    {
        //printf("find id=%d\n",id);
        return node;
    }
    else
    {
        if(node->Child!= NULL)
            p1 = get_parent_node(pid,node->Child);

        if(node->NextBrother != NULL)
            p2 = get_parent_node(pid,node->NextBrother);

        if(p1 != NULL)
            return p1;
        else
            if(p2 != NULL)
                return p2;
        else
            return NULL;
    }
}

Hb_SubNode *find_node(Hb_SubNode *pnode,int id)
{
    Hb_SubNode *p1 = NULL;

    p1 = pnode->Child;
    while(p1 != NULL)
    {
        printf("p1->id=%d,id=%d\n",p1->id,id);
        if(p1->id == id)
        {
            return p1;
        }
        p1 = p1->NextBrother;
    }
    return NULL;
}

int update_seq(int id,int seq,Hb_SubNode *node)
{
    Hb_SubNode *p = NULL;
    p = get_parent_node(id,node);
    if(p == NULL)
        return -1;
    p->seq = seq;
    return 0;
}

int remove_node(Hb_SubNode *pnode,int id)
{
    Hb_SubNode *p1 = NULL,*p2 = NULL;

    if(pnode->Child != NULL)
    {

        p1 = pnode->Child;
        printf("remove_node:node->id=%d,id=%d\n",p1->id,id);
        while(p1 != NULL)
        {
            if(p1->id == id)
                break;
            p2 = p1;
            p1 = p1->NextBrother;
        }

        if(p1 != NULL) // find
        {
            if(p1->id == pnode->Child->id)
                pnode->Child = p1->NextBrother;
            else
            {
                p2->NextBrother = p1->NextBrother;
            }
            p1->NextBrother = NULL;
            free_node(p1);
        }
    }

   print_all_nodes(SyncNode);
    return 0;
}

int del_node(int pid,int id)
{
    Hb_SubNode *pnode = NULL;
    pnode = get_parent_node(pid,SyncNode);
    if(pnode == NULL)
        return -1;

    remove_node(pnode,id);

    return 0;
}

Hb_SubNode *move_from_node(int pid,int id)
{
    Hb_SubNode *p1 = NULL,*p2 = NULL,*pnode = NULL;
    pnode = get_parent_node(pid,SyncNode);
    if(pnode == NULL)
    {
        handle_error(99,"get node fail in move_from_node");
        return NULL;
     }

    if(pnode->Child == NULL)
    {
        handle_error(99,"no child in move_from_node");
        return NULL;
    }
    else
    {
        p1 = pnode->Child;
        while(p1 != NULL)
        {
            if(p1->id == id)
                break;
            p2 = p1;
            p1 = p1->NextBrother;
        }

        if(p1 != NULL) // find
        {
            if(p1 == pnode->Child)
                pnode->Child = p1->NextBrother;
            else
            {
                p2->NextBrother = p1->NextBrother;
            }
        }
    }

    return p1;
}

int move_to_node(int pid,Hb_SubNode *node)
{
    Hb_SubNode *pnode = NULL,*p1 = NULL,*p2;
    pnode = get_parent_node(pid,SyncNode);
    if(pnode == NULL)
        return -1;

    p1 = find_node(pnode,node->id);
    if(p1 != NULL)
    {
        free_node(node);
    }
    else
    {
        p2 = pnode->Child;
        pnode->Child = node;
        node->NextBrother = p2;
    }   

    return 0;
}

int move_node(int move_from_pid,int id,int move_to_pid)
{
    Hb_SubNode *pnode = NULL;
    pnode = move_from_node(move_from_pid,id);
    if(pnode == NULL)
    {
        handle_error(99,"move from node fail");
        return -1;
    }

    move_to_node(move_to_pid,pnode);
}

int add_node(int id,int seq,Hb_SubNode *pnode)
{
    printf("enter add_node,id=%d,seq=%d\n",id,seq);
    Hb_SubNode *p1 = NULL,*tempnode = NULL;

    tempnode = create_node(id,seq);
    if(tempnode == NULL)
    {
        handle_error(99,"malloc fail in add_node");
        return -1;
     }

    if(find_node(pnode,id) == NULL)
    {
        printf("not find\n");
        p1 = pnode->Child;
        pnode->Child = tempnode;
        tempnode->NextBrother = p1;
    }

    //printf("add_node end\n");
    //if(pre_seq >0)
        print_all_nodes(SyncNode);
    return 0;
}

int insert_node(int pid,int id,int seq)
{
    Hb_SubNode *pnode = NULL;
    pnode = get_parent_node(pid,SyncNode);
    if(pnode == NULL)
    {
        handle_error(99,"get node fail in insert_node");
        return -1;
     }
    add_node(id,seq,pnode);
}

void free_node(Hb_SubNode *node)
{
    if(node != NULL)
    {
        //printf("free tree node\n");
        if(node->NextBrother != NULL)
            free_node(node->NextBrother);
        if(node->Child != NULL)
            free_node(node->Child);
        free(node);
    }
}

void print_all_nodes(Hb_SubNode *node)
{
    if(node != NULL)
    {
        printf("id=%d,seq=%d\n",node->id,node->seq);

        if(node->NextBrother != NULL)
            print_all_nodes(node->NextBrother);

        if(node->Child != NULL)
            print_all_nodes(node->Child);
    }
}





/* string_concat [string + string] 
* str1 + str2 , return new string
* */  
char *string_concat(char *str1, char *str2) {  
   // count array len
   int length=strlen(str1)+strlen(str2)+1;  
     
   // create new string memory
   char *result = (char*)malloc(sizeof(char) * length);  
     
   // copy str1 to result
   strcpy(result, str1);  
   // result + str2
   strcat(result, str2);  
     
   return result;  
}  