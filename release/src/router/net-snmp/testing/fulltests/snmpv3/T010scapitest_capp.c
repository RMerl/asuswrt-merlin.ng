/*
 * scapitest.c
 *
 * HEADER Testing SCAPI API
 *
 * Expected SUCCESSes:  2 + 10 + 1 for all tests.
 *
 * Returns:
 *      Number of FAILUREs.
 *
 *
 * ASSUMES  No key management functions return non-zero success codes.
 *
 * XXX  Split into individual modules?
 * XXX  Error/fringe conditions should be tested.
 *
 *
 * Test of sc_random.                                           SUCCESSes: 2.
 *      REQUIRES a human to spot check for obvious non-randomness...
 *
 * Test of sc_generate_keyed_hash and sc_check_keyed_hash.      SUCCESSes: 10.
 *
 * Test of sc_encrypt and sc_decrypt.                           SUCCESSes: 1.
 */

#include <net-snmp/net-snmp-config.h>

#include <stdio.h>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#include <net-snmp/library/asn1.h>
#include <net-snmp/library/snmp_api.h>
#include <net-snmp/library/keytools.h>
#include <net-snmp/library/tools.h>
#include <net-snmp/library/scapi.h>
#include <net-snmp/library/transform_oids.h>
#include <net-snmp/library/callback.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/library/snmpusm.h>
#include <net-snmp/library/getopt.h>

#include <stdlib.h>


/*
 * Globals, &c...
 */
char           *local_progname;
int             testcount=0;
int             failcount=0;

#define USAGE	"Usage: %s [-h][-acHr]"
#define OPTIONLIST	"achHr"

int             doalltests = 0, docrypt = 0, dokeyedhash = 0, dorandom = 0;

#define	ALLOPTIONS	(doalltests + docrypt + dokeyedhash + dorandom)



#define LOCAL_MAXBUF	(1024 * 8)
#define NL		"\n"

/* #define OUTPUT(o)	printf("# %s\n", o); */
int
output(const char *format, ...)
{
    va_list ap;
    char  *buffer = NULL;
    int   length;
    va_start(ap, format);
    length = vasprintf(&buffer, format, ap);
    va_end(ap);
    if (length < 0) {
        fprintf(stderr, "could not allocate output string\n");
        exit(127);
    }

    printf("# %s\n", buffer);
    free(buffer);
    return 0;
}

#define SUCCESS(s)					\
{							\
    printf("# Done with %s\n", s);			\
}

#define FAILED(e, f)                                                    \
{                                                                       \
    if (e != SNMPERR_SUCCESS) {                                         \
                printf("not ok: %d - %s\n", ++testcount, f);            \
		failcount += 1;                                         \
	} else {                                                        \
                printf("ok: %d - %s\n", ++testcount, f);                \
        }                                                               \
    fflush(stdout); \
}

#define BIGSTRING							\
    (const u_char *)                                                    \
    "   A port may be a pleasant retreat for any mind grown weary of"	\
    "the struggle for existence.  The vast expanse of sky, the"		\
    "mobile architecture of the clouds, the chameleon coloration"	\
    "of the sea, the beacons flashing on the shore, together make"	\
    "a prism which is marvellously calculated to entertain but not"	\
    "fatigue the eye.  The lofty ships with their complex webs of"	\
    "rigging, swayed to and fro by the swell in harmonious dance,"	\
    "all help to maintain a taste for rhythm and beauty in the"		\
    "mind.  And above all there is a mysterious, aristrocratic kind"	\
    "of pleasure to be had, for those who have lost all curiosity"	\
    "or ambition, as they strech on the belvedere or lean over the"	\
    "mole to watch the arrivals and departures of other men, those"	\
    "who still have sufficient strength of purpose in them, the"	\
    "urge to travel or enrich themselves."				\
    "	-- Baudelaire"							\
    "	   From _The_Poems_in_Prose_, \"The Port\" (XLI)."

#define BIGSECRET                                               \
    (const u_char *)                                            \
    "Shhhh... Don't tell *anyone* about this.  Not even a single soul."
#define BKWDSECRET                                              \
    (const u_char *)                                            \
    ".luos elgnis a neve toN  .siht tuoba *enoyna* llet t'noD ...hhhhS"


static void
usage(void)
{
    printf( USAGE
            "" NL
            "	-a		All tests." NL
            "	-c		Test of sc_encrypt()/sc_decrypt()."
            NL
            "	-h		Help."
            NL
            "	-H              Test sc_{generate,check}_keyed_hash()."
            NL
            "	-r              Test sc_random()."
            NL "" NL, local_progname);

}                               /* end usage() */



static int test_docrypt(void);
static int test_dokeyedhash(void);
static int test_dorandom(void);

int
main(int argc, char **argv)
{
    int             rval = SNMPERR_SUCCESS;
    int             ch;

    local_progname = argv[0];

    /*
     * Parse.
     */
    while ((ch = getopt(argc, argv, OPTIONLIST)) != EOF) {
        switch (ch) {
        case 'a':
            doalltests = 1;
            break;
        case 'c':
            docrypt = 1;
            break;
        case 'H':
            dokeyedhash = 1;
            break;
        case 'r':
            dorandom = 1;
            break;
        case 'h':
            rval = 0;
            /* fall through */
        default:
            usage();
            exit(rval);
        }

        argc -= 1;
        argv += 1;
        optind = 1;
    }                           /* endwhile getopt */

    if ((argc > 1)) {
        usage();
        exit(1000);

    } else if (ALLOPTIONS != 1) {
        doalltests = 1;
    }


    /*
     * Test stuff.
     */
    rval = sc_init();
    FAILED(rval, "sc_init() return code");


    if (docrypt || doalltests) {
        test_docrypt();
    }
    if (dokeyedhash || doalltests) {
        test_dokeyedhash();
    }
    if (dorandom || doalltests) {
        test_dorandom();
    }

    printf("1..%d\n", testcount);
    return 0;
}                               /* end main() */





/*******************************************************************-o-******
 * test_dorandom
 *
 * One large request, one set of short requests.
 *
 * Returns:
 *	Number of failures.
 *
 * XXX	probably should split up into individual options.
 */
int
test_dorandom(void)
{
    int             rval = SNMPERR_SUCCESS,
        origrequest = (1024 * 2),
        origrequest_short = 19, shortcount = 7, i;
    size_t          nbytes = origrequest;
    u_char          buf[LOCAL_MAXBUF];

    output("Random test -- large request:");

    rval = sc_random(buf, &nbytes);
    FAILED(rval, "sc_random() return code");

    if (nbytes != origrequest) {
        FAILED(SNMPERR_GENERR,
               "sc_random() returned different than requested.");
    }

    dump_chunk("scapitest", NULL, buf, nbytes);

    SUCCESS("Random test -- large request.");


    output("Random test -- short requests:");
    origrequest_short = 16;

    for (i = 0; i < shortcount; i++) {
        nbytes = origrequest_short;
        rval = sc_random(buf, &nbytes);
        FAILED(rval, "sc_random() return code");

        if (nbytes != origrequest_short) {
            FAILED(SNMPERR_GENERR,
                   "sc_random() returned different " "than requested.");
        }

        dump_chunk("scapitest", NULL, buf, nbytes);
    }                           /* endfor */

    SUCCESS("Random test -- short requests.");


    return failcount;

}                               /* end test_dorandom() */



/*******************************************************************-o-******
 * test_dokeyedhash
 *
 * Returns:
 *	Number of failures.
 *
 *
 * Test keyed hashes with a variety of MAC length requests.
 *
 *
 * NOTE Both tests intentionally use the same secret
 *
 * FIX	Get input or output from some other package which hashes...
 */
int
test_dokeyedhash(void)
{
    int rval = SNMPERR_SUCCESS,
        bigstring_len = strlen((const char *) BIGSTRING),
        secret_len = strlen((const char *) BIGSECRET),
        auth_idx = 0,
        mlcount = 0;        /* MAC Length count.   */
    size_t          hblen;      /* Hash Buffer length. */
    netsnmp_auth_alg_info *ai;
    u_int           hashbuf_len[] = {
        LOCAL_MAXBUF,
        USM_MD5_AND_SHA_AUTH_LEN,
        USM_HMAC128SHA224_AUTH_LEN,
        USM_HMAC192SHA256_AUTH_LEN,
        USM_HMAC256SHA384_AUTH_LEN,
        USM_HMAC384SHA512_AUTH_LEN,
        7,
        0,
    };

    u_char          hashbuf[LOCAL_MAXBUF];
    char           *s;

    while(1) {

        ai = sc_get_auth_alg_byindex(++auth_idx);
        if (NULL == ai)
            break;

        mlcount = 0;

      test_dokeyedhash_again:

        hblen = hashbuf_len[mlcount];

        if (ai->mac_length > hblen) {
            output("Skipping Keyed hash test (len %d, proplen %d maclen %d) using %s --", hblen, ai->proper_length, ai->mac_length, ai->name);
            goto skip;
        }

    memset(hashbuf, 0, LOCAL_MAXBUF);

    output("Starting Keyed hash test (len %d, proplen %d, maclen %d) using %s --", hblen, ai->proper_length, ai->mac_length, ai->name);

    rval =
        sc_generate_keyed_hash(ai->alg_oid, ai->oid_len, BIGSECRET,
                               secret_len, BIGSTRING,
                               bigstring_len,
                               hashbuf, &hblen);
    FAILED(rval, "sc_generate_keyed_hash() return code");

    if (hashbuf_len[mlcount] > ai->proper_length) {
        if (hblen != ai->proper_length) {
            FAILED(SNMPERR_GENERR, "Wrong hash length returned.  (1)");
        }
    } else if (hblen != hashbuf_len[mlcount]) {
        FAILED(SNMPERR_GENERR, "Wrong hash length returned.  (2)");
    }
    if (hblen > ai->mac_length) {
        printf("# TRUNCATING %d length hash to %s mac length %d\n", (int)hblen,
               ai->name, ai->mac_length);
        hblen = ai->mac_length;
    }

    rval =
        sc_check_keyed_hash(ai->alg_oid, ai->oid_len, BIGSECRET,
                            secret_len, BIGSTRING, bigstring_len, hashbuf,
                            hblen);
    FAILED(rval, "sc_check_keyed_hash() return code");

    binary_to_hex(hashbuf, hblen, &s);
    printf("# hash buffer (len=%" NETSNMP_PRIz "u, request=%d):   %s\n",
            hblen, hashbuf_len[mlcount], s);
    SNMP_FREE(s);

    /*
     * Run the basic hash tests but vary the size MAC requests.
     */
      skip:
    if (hashbuf_len[++mlcount] != 0) {
        goto test_dokeyedhash_again;
    }

    } /* whilte(1) */

    return failcount;

}                               /* end test_dokeyedhash() */





/*******************************************************************-o-******
 * test_docrypt
 *
 * Returns:
 *	Number of failures.
 */
int
test_docrypt(void)
{
    int             rval, index = 0, secret_len, iv_len,
        bigstring_len = strlen((const char *) BIGSTRING);
    netsnmp_priv_alg_info *pi;
    size_t          buf_len, cryptbuf_len;

    u_char            buf[LOCAL_MAXBUF],
        cryptbuf[LOCAL_MAXBUF], secret[LOCAL_MAXBUF], iv[LOCAL_MAXBUF];

    while(1) {
        pi = sc_get_priv_alg_byindex(++index);
        if (NULL == pi)
            break;

        buf_len = sizeof(buf);
        cryptbuf_len = sizeof(cryptbuf);
        secret_len = pi->proper_length;
        iv_len = pi->iv_length;

        output("Starting Test %s --", pi->name);


        memset(buf, 0, LOCAL_MAXBUF);

        memcpy(secret, BIGSECRET, secret_len);
        memcpy(iv, BKWDSECRET, iv_len);

        rval = sc_encrypt(pi->alg_oid, pi->oid_len,
                          secret, secret_len,
                          iv, iv_len,
                          BIGSTRING, bigstring_len, cryptbuf, &cryptbuf_len);
        FAILED(rval, "sc_encrypt() return code.");

        rval = sc_decrypt(pi->alg_oid, pi->oid_len,
                          secret, secret_len,
                          iv, iv_len, cryptbuf, cryptbuf_len, buf, &buf_len);
        FAILED(rval, "sc_decrypt() return code.");

        if (pi->pad_size > 0) {
            /* ignore the pad */
            buf_len -= buf[buf_len-1];
        }

        FAILED((buf_len != bigstring_len),
               "Decrypted buffer is the right length.");
        printf("# original length: %d\n", bigstring_len);
        printf("# output   length: %" NETSNMP_PRIz "u\n", buf_len);

        FAILED((memcmp(buf, BIGSTRING, bigstring_len) != 0),
               "Decrypted buffer is the same as the original plaintext.");

    } /* while(1) */

    return failcount;
}                               /* end test_docrypt() */
