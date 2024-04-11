/*
 * Dropbear - a SSH2 server
 * 
 * Copyright (c) 2002,2003 Matt Johnston
 * All rights reserved.
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. */

/* The format of the keyfiles is basically a raw dump of the buffer. Data types
 * are specified in the transport rfc 4253 - string is a 32-bit len then the
 * non-null-terminated string, mp_int is a 32-bit len then the bignum data.
 * The actual functions are buf_put_rsa_priv_key() and buf_put_dss_priv_key()

 * RSA:
 * string	"ssh-rsa"
 * mp_int	e
 * mp_int	n
 * mp_int	d
 * mp_int	p (newer versions only)
 * mp_int	q (newer versions only) 
 *
 * DSS:
 * string	"ssh-dss"
 * mp_int	p
 * mp_int	q
 * mp_int	g
 * mp_int	y
 * mp_int	x
 *
 * Ed25519:
 * string	"ssh-ed25519"
 * string	k (32 bytes) + A (32 bytes)
 *
 */
#include "includes.h"
#include "signkey.h"
#include "buffer.h"
#include "dbutil.h"

#include "genrsa.h"
#include "gendss.h"
#include "gened25519.h"
#include "ecdsa.h"
#include "crypto_desc.h"
#include "dbrandom.h"
#include "gensignkey.h"

#if DROPBEAR_ED25519
#define DEFAULT_KEY_TYPE_NAME "ed25519"
#elif DROPBEAR_RSA
/* Different to the sigalgs list because negotiated hostkeys have fallbacks for compatibility,
 * whereas a generated authkey doesn't, so RSA needs to be higher than ECDSA */
#define DEFAULT_KEY_TYPE_NAME "rsa"
#elif DROPBEAR_ECDSA
#define DEFAULT_KEY_TYPE_NAME "ecdsa"
#elif DROPBEAR_DSS
#define DEFAULT_KEY_TYPE_NAME "dss"
#endif

static void printhelp(char * progname);

static void printpubkey(sign_key * key, int keytype, const char * comment, int create_pub_file, const char * filename);
/* Print a public key and fingerprint to stdout.
 * Used for "dropbearkey -y" command but also after generation of a new key.
 * For the new key pair the create_pub_file will be TRUE and the pub key will be saved to a .pub file.
*/
static int printpubfile(const char* filename, const char * comment, int create_pub_file);

/* Print a help message */
static void printhelp(char * progname) {

	fprintf(stderr, "Usage: %s -t <type> -f <filename> [-s bits]\n"
					"-t type	Type of key to generate. One of:\n"
#if DROPBEAR_RSA
					"		rsa\n"
#endif
#if DROPBEAR_DSS
					"		dss\n"
#endif
#if DROPBEAR_ECDSA
					"		ecdsa\n"
#endif
#if DROPBEAR_ED25519
					"		ed25519\n"
#endif
					"-f filename    Use filename for the secret key.\n"
					"               ~/.ssh/id_dropbear is recommended for client keys.\n"
					"-s bits	Key size in bits, should be a multiple of 8 (optional)\n"
#if DROPBEAR_DSS
					"           DSS has a fixed size of 1024 bits\n"
#endif
#if DROPBEAR_ECDSA
					"           ECDSA has sizes "
#if DROPBEAR_ECC_256
					"256 "
#endif
#if DROPBEAR_ECC_384
					"384 "
#endif
#if DROPBEAR_ECC_521
					"521 "
#endif
					"\n"
#endif
#if DROPBEAR_ED25519
					"           Ed25519 has a fixed size of 256 bits\n"
#endif
					"-y		Just print the publickey and fingerprint for the\n		private key in <filename>.\n"
					"-C		Specify the key comment (email).\n"
#if DEBUG_TRACE
					"-v		verbose\n"
#endif
					,progname);
}

/* fails fatally */
static void check_signkey_bits(enum signkey_type type, int bits)
{
	switch (type) {
#if DROPBEAR_ED25519
		case DROPBEAR_SIGNKEY_ED25519:
			if (bits != 256) {
				dropbear_exit("Ed25519 keys have a fixed size of 256 bits\n");
				exit(EXIT_FAILURE);
			}
			break;
#endif
#if DROPBEAR_RSA
		case DROPBEAR_SIGNKEY_RSA:
			if (bits < 1024 || bits > 4096 || (bits % 8 != 0)) {
				dropbear_exit("Bits must satisfy 1024 <= bits <= 4096, and be a"
				              " multiple of 8\n");
			}
			break;
#endif
#if DROPBEAR_DSS
		case DROPBEAR_SIGNKEY_DSS:
			if (bits != 1024) {
				dropbear_exit("DSS keys have a fixed size of 1024 bits\n");
				exit(EXIT_FAILURE);
			}
			break;
#endif
		default:
			(void)0; /* quiet, compiler. ecdsa handles checks itself */
	}
}

#if defined(DBMULTI_dropbearkey) || !DROPBEAR_MULTI
#if defined(DBMULTI_dropbearkey) && DROPBEAR_MULTI
int dropbearkey_main(int argc, char ** argv) {
#else
int main(int argc, char ** argv) {
#endif

	int i;
	char ** next = NULL;
	char * filename = NULL;
	enum signkey_type keytype = DROPBEAR_SIGNKEY_NONE;
	char * typetext = DEFAULT_KEY_TYPE_NAME;
	char * sizetext = NULL;
	char * passphrase = NULL;
	char * comment = NULL;
	unsigned int bits = 0, genbits;
	int printpub = 0;

	crypto_init();
	seedrandom();

	/* get the commandline options */
	for (i = 1; i < argc; i++) {
		if (argv[i] == NULL) {
			continue; /* Whack */
		} 
		if (next) {
			*next = argv[i];
			next = NULL;
			continue;
		}

		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'f':
					next = &filename;
					break;
				case 't':
					next = &typetext;
					break;
				case 's':
					next = &sizetext;
					break;
				case 'C':
					next = &comment;
					break;
				case 'y':
					printpub = 1;
					break;
				case 'h':
					printhelp(argv[0]);
					exit(EXIT_SUCCESS);
					break;
				case 'v':
#if DEBUG_TRACE
					debug_trace = DROPBEAR_VERBOSE_LEVEL;
#endif
					break;
				case 'q':
					break;  /* quiet is default */
				case 'N':
					next = &passphrase;
					break;
				default:
					fprintf(stderr, "Unknown argument %s\n", argv[i]);
					printhelp(argv[0]);
					exit(EXIT_FAILURE);
					break;
			}
		}
	}

	if (!filename) {
		fprintf(stderr, "Must specify a key filename\n");
		printhelp(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (printpub) {
		int ret = printpubfile(filename, NULL, 0);
		exit(ret);
	}

#if DROPBEAR_RSA
	if (strcmp(typetext, "rsa") == 0)
	{
		keytype = DROPBEAR_SIGNKEY_RSA;
	}
#endif
#if DROPBEAR_DSS
	if (strcmp(typetext, "dss") == 0)
	{
		keytype = DROPBEAR_SIGNKEY_DSS;
	}
#endif
#if DROPBEAR_ECDSA
	if (strcmp(typetext, "ecdsa") == 0)
	{
		keytype = DROPBEAR_SIGNKEY_ECDSA_KEYGEN;
	}
#endif
#if DROPBEAR_ED25519
	if (strcmp(typetext, "ed25519") == 0)
	{
		keytype = DROPBEAR_SIGNKEY_ED25519;
	}
#endif

	if (keytype == DROPBEAR_SIGNKEY_NONE) {
		fprintf(stderr, "Unknown key type '%s'\n", typetext);
		printhelp(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (sizetext) {
		if (sscanf(sizetext, "%u", &bits) != 1) {
			fprintf(stderr, "Bits must be an integer\n");
			exit(EXIT_FAILURE);
		}
		
		check_signkey_bits(keytype, bits);;
	}

	if (passphrase && *passphrase != '\0') {
		fprintf(stderr, "Only empty passphrase is supported\n");
		exit(EXIT_FAILURE);
	}

	genbits = signkey_generate_get_bits(keytype, bits);
	fprintf(stderr, "Generating %u bit %s key, this may take a while...\n", genbits, typetext);
	if (signkey_generate(keytype, bits, filename, 0) == DROPBEAR_FAILURE)
	{
		dropbear_exit("Failed to generate key.\n");
	}

	printpubfile(filename, comment, 1);

	return EXIT_SUCCESS;
}
#endif

static int printpubfile(const char* filename, const char* comment, int create_pub_file) {

	buffer *buf = NULL;
	sign_key *key = NULL;
	enum signkey_type keytype;
	int ret;
	int err = DROPBEAR_FAILURE;

	buf = buf_new(MAX_PRIVKEY_SIZE);
	ret = buf_readfile(buf, filename);

	if (ret != DROPBEAR_SUCCESS) {
		fprintf(stderr, "Failed reading '%s'\n", filename);
		goto out;
	}

	key = new_sign_key();
	keytype = DROPBEAR_SIGNKEY_ANY;

	buf_setpos(buf, 0);
	ret = buf_get_priv_key(buf, key, &keytype);
	if (ret == DROPBEAR_FAILURE) {
		fprintf(stderr, "Bad key in '%s'\n", filename);
		goto out;
	}

	printpubkey(key, keytype, comment, create_pub_file, filename);

	err = DROPBEAR_SUCCESS;

out:
	buf_burn_free(buf);
	buf = NULL;
	if (key) {
		sign_key_free(key);
		key = NULL;
	}
	return err;
}

static void printpubkey(sign_key * key, int keytype, const char * comment, int create_pub_file, const char * filename) {

	buffer * buf = NULL;
	unsigned char base64key[MAX_PUBKEY_SIZE*2];
	unsigned long base64len;
	int err;
	const char * typestring = NULL;
	char *fp = NULL;
	int len;
	struct passwd * pw = NULL;
	char * username = NULL;
	char hostname[100];
	char * filename_pub = NULL;
	FILE *pubkey_file = NULL;

	if (create_pub_file) {
		int pubkey_fd = -1;
		int filename_pub_len = 0;
		filename_pub_len = strlen(filename) + 5;
		filename_pub = m_malloc(filename_pub_len);
		snprintf(filename_pub, filename_pub_len, "%s.pub", filename);

		/* open() to use O_EXCL, then use a FILE* for fprintf().
		 * dprintf() is only posix2008 onwards */
		pubkey_fd = open(filename_pub, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
		if (pubkey_fd >= 0) {
			/* Convert the fd to a FILE*. The underlying FD is closed
			 * by later fclose() */
			pubkey_file = fdopen(pubkey_fd, "w");
			if (!pubkey_file) {
				m_close(pubkey_fd);
			}
		}
		if (!pubkey_file) {
			dropbear_log(LOG_ERR, "Save public key to %s failed: %s", filename_pub, strerror(errno));
		}
	}

	buf = buf_new(MAX_PUBKEY_SIZE);
	buf_put_pub_key(buf, key, keytype);
	buf_setpos(buf, 4);

	len = buf->len - buf->pos;

	base64len = sizeof(base64key);
	err = base64_encode(buf_getptr(buf, len), len, base64key, &base64len);

	if (err != CRYPT_OK) {
		dropbear_exit("base64 failed");
	}

	typestring = signkey_name_from_type(keytype, NULL);

	printf("Public key portion is:\n");

	if (comment) {
		printf("%s %s %s\n",
				typestring, base64key, comment);
		if (pubkey_file) {
			fprintf(pubkey_file, "%s %s %s\n",
					typestring, base64key, comment);
		}
	} else {
		/* a user@host comment is informative */
		username = "";
		pw = getpwuid(getuid());
		if (pw) {
			username = pw->pw_name;
		}

		gethostname(hostname, sizeof(hostname));
		hostname[sizeof(hostname) - 1] = '\0';

		printf("%s %s %s@%s\n",
				typestring, base64key, username, hostname);
		if (pubkey_file) {
			fprintf(pubkey_file, "%s %s %s@%s\n",
					typestring, base64key, username, hostname);
		}
	}

	fp = sign_key_fingerprint(buf_getptr(buf, len), len);
	printf("Fingerprint: %s\n", fp);

	m_free(fp);
	buf_free(buf);

	if (pubkey_file) {
		if (fsync(fileno(pubkey_file)) != 0) {
			dropbear_log(LOG_ERR, "fsync of %s failed: %s", filename_pub, strerror(errno));
		}
		fclose(pubkey_file);
	}
	m_free(filename_pub);
}
