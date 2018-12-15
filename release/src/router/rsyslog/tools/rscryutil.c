/* This is a tool for processing rsyslog encrypted log files.
 *
 * Copyright 2013-2016 Adiscon GmbH
 *
 * This file is part of rsyslog.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either exprs or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gcrypt.h>

#include "rsyslog.h"
#include "libgcry.h"


static enum { MD_DECRYPT, MD_WRITE_KEYFILE
} mode = MD_DECRYPT;
static int verbose = 0;
static gcry_cipher_hd_t gcry_chd;
static size_t blkLength;

static char *keyfile = NULL;
static char *keyprog = NULL;
static int randomKeyLen = -1;
static char *cry_key = NULL;
static unsigned cry_keylen = 0;
static int cry_algo = GCRY_CIPHER_AES128;
static int cry_mode = GCRY_CIPHER_MODE_CBC;
static int optionForce = 0;

/* We use some common code which expects rsyslog runtime to be
 * present, most importantly for debug output. As a stand-alone
 * tool, we do not really have this. So we do some dummy defines
 * in order to satisfy the needs of the common code.
 */
int Debug = 0;
#ifndef DEBUGLESS
void r_dbgprintf(const char *srcname __attribute__((unused)), const char *fmt __attribute__((unused)), ...) {};
#endif
void srSleep(int a __attribute__((unused)), int b __attribute__((unused)));
/* prototype (avoid compiler warning) */
void srSleep(int a __attribute__((unused)), int b __attribute__((unused))) {}
/* this is not really needed by any of our code */
long randomNumber(void);
/* prototype (avoid compiler warning) */
long randomNumber(void) {return 0l;}
/* this is not really needed by any of our code */

/* rectype/value must be EIF_MAX_*_LEN+1 long!
 * returns 0 on success or something else on error/EOF
 */
static int
eiGetRecord(FILE *eifp, char *rectype, char *value)
{
	int r;
	unsigned short i, j;
	char buf[EIF_MAX_RECTYPE_LEN+EIF_MAX_VALUE_LEN+128];
	     /* large enough for any valid record */

	if(fgets(buf, sizeof(buf), eifp) == NULL) {
		r = 1; goto done;
	}

	for(i = 0 ; i < EIF_MAX_RECTYPE_LEN && buf[i] != ':' ; ++i)
		if(buf[i] == '\0') {
			r = 2; goto done;
		} else
			rectype[i] = buf[i];
	rectype[i] = '\0';
	j = 0;
	for(++i ; i < EIF_MAX_VALUE_LEN && buf[i] != '\n' ; ++i, ++j)
		if(buf[i] == '\0') {
			r = 3; goto done;
		} else
			value[j] = buf[i];
	value[j] = '\0';
	r = 0;
done:	return r;
}

static int
eiCheckFiletype(FILE *eifp)
{
	char rectype[EIF_MAX_RECTYPE_LEN+1];
	char value[EIF_MAX_VALUE_LEN+1];
	int r;

	if((r = eiGetRecord(eifp, rectype, value)) != 0) goto done;
	if(strcmp(rectype, "FILETYPE") || strcmp(value, RSGCRY_FILETYPE_NAME)) {
		fprintf(stderr, "invalid filetype \"cookie\" in encryption "
			"info file\n");
		fprintf(stderr, "\trectype: '%s', value: '%s'\n", rectype, value);
		r = 1; goto done;
	}
	r = 0;
done:	return r;
}

static int
eiGetIV(FILE *eifp, char *iv, size_t leniv)
{
	char rectype[EIF_MAX_RECTYPE_LEN+1];
	char value[EIF_MAX_VALUE_LEN+1];
	size_t valueLen;
	unsigned short i, j;
	int r;
	unsigned char nibble;

	if((r = eiGetRecord(eifp, rectype, value)) != 0) goto done;
	if(strcmp(rectype, "IV")) {
		fprintf(stderr, "no IV record found when expected, record type "
			"seen is '%s'\n", rectype);
		r = 1; goto done;
	}
	valueLen = strlen(value);
	if(valueLen/2 != leniv) {
		fprintf(stderr, "length of IV is %lld, expected %lld\n",
			(long long) valueLen/2, (long long) leniv);
		r = 1; goto done;
	}

	for(i = j = 0 ; i < valueLen ; ++i) {
		if(value[i] >= '0' && value[i] <= '9')
			nibble = value[i] - '0';
		else if(value[i] >= 'a' && value[i] <= 'f')
			nibble = value[i] - 'a' + 10;
		else {
			fprintf(stderr, "invalid IV '%s'\n", value);
			r = 1; goto done;
		}
		if(i % 2 == 0)
			iv[j] = nibble << 4;
		else
			iv[j++] |= nibble;
	}
	r = 0;
done:	return r;
}

static int
eiGetEND(FILE *eifp, off64_t *offs)
{
	char rectype[EIF_MAX_RECTYPE_LEN+1] = "";
	char value[EIF_MAX_VALUE_LEN+1];
	int r;

	if((r = eiGetRecord(eifp, rectype, value)) != 0) goto done;
	if(strcmp(rectype, "END")) {
		fprintf(stderr, "no END record found when expected, record type "
			"seen is '%s'\n", rectype);
		r = 1; goto done;
	}
	*offs = atoll(value);
	r = 0;
done:	return r;
}

static int
initCrypt(FILE *eifp)
{
	int r = 0;
	gcry_error_t gcryError;
	char iv[4096];

	blkLength = gcry_cipher_get_algo_blklen(cry_algo);
	if(blkLength > sizeof(iv)) {
		fprintf(stderr, "internal error[%s:%d]: block length %lld too large for "
			"iv buffer\n", __FILE__, __LINE__, (long long) blkLength);
		r = 1; goto done;
	}
	if((r = eiGetIV(eifp, iv, blkLength)) != 0) goto done;

	size_t keyLength = gcry_cipher_get_algo_keylen(cry_algo);
	if(strlen(cry_key) != keyLength) {
		fprintf(stderr, "invalid key length; key is %u characters, but "
			"exactly %llu characters are required\n", cry_keylen,
			(long long unsigned) keyLength);
		r = 1; goto done;
	}

	gcryError = gcry_cipher_open(&gcry_chd, cry_algo, cry_mode, 0);
	if (gcryError) {
		printf("gcry_cipher_open failed:  %s/%s\n",
			gcry_strsource(gcryError),
			gcry_strerror(gcryError));
		r = 1; goto done;
	}

	gcryError = gcry_cipher_setkey(gcry_chd, cry_key, keyLength);
	if (gcryError) {
		printf("gcry_cipher_setkey failed:  %s/%s\n",
			gcry_strsource(gcryError),
			gcry_strerror(gcryError));
		r = 1; goto done;
	}

	gcryError = gcry_cipher_setiv(gcry_chd, iv, blkLength);
	if (gcryError) {
		printf("gcry_cipher_setiv failed:  %s/%s\n",
			gcry_strsource(gcryError),
			gcry_strerror(gcryError));
		r = 1; goto done;
	}
done: return r;
}

static void
removePadding(char *buf, size_t *plen)
{
	unsigned len = (unsigned) *plen;
	unsigned iSrc, iDst;
	char *frstNUL;

	frstNUL = memchr(buf, 0x00, *plen);
	if(frstNUL == NULL)
		goto done;
	iDst = iSrc = frstNUL - buf;

	while(iSrc < len) {
		if(buf[iSrc] != 0x00)
			buf[iDst++] = buf[iSrc];
		++iSrc;
	}

	*plen = iDst;
done:	return;
}

static void
decryptBlock(FILE *fpin, FILE *fpout, off64_t blkEnd, off64_t *pCurrOffs)
{
	gcry_error_t gcryError;
	size_t nRead, nWritten;
	size_t toRead;
	size_t leftTillBlkEnd;
	char buf[64*1024];
	
	leftTillBlkEnd = blkEnd - *pCurrOffs;
	while(1) {
		toRead = sizeof(buf) <= leftTillBlkEnd ? sizeof(buf) : leftTillBlkEnd;
		toRead = toRead - toRead % blkLength;
		nRead = fread(buf, 1, toRead, fpin);
		if(nRead == 0)
			break;
		leftTillBlkEnd -= nRead, *pCurrOffs += nRead;
		gcryError = gcry_cipher_decrypt(
				gcry_chd, // gcry_cipher_hd_t
				buf,    // void *
				nRead,    // size_t
				NULL,    // const void *
				0);   // size_t
		if (gcryError) {
			fprintf(stderr, "gcry_cipher_decrypt failed:  %s/%s\n",
			gcry_strsource(gcryError),
			gcry_strerror(gcryError));
			return;
		}
		removePadding(buf, &nRead);
		nWritten = fwrite(buf, 1, nRead, fpout);
		if(nWritten != nRead) {
			perror("fpout");
			return;
		}
	}
}


static int
doDecrypt(FILE *logfp, FILE *eifp, FILE *outfp)
{
	off64_t blkEnd;
	off64_t currOffs = 0;
	int r = 1;
	int fd;
	struct stat buf;

	while(1) {
		/* process block */
		if(initCrypt(eifp) != 0)
			goto done;
		/* set blkEnd to size of logfp and proceed. */
		if((fd = fileno(logfp)) == -1) {
			r = -1;
			goto done;
		}
		if((r = fstat(fd, &buf)) != 0) goto done;
		blkEnd = buf.st_size;
		r = eiGetEND(eifp, &blkEnd);
		if(r != 0 && r != 1) goto done;
		decryptBlock(logfp, outfp, blkEnd, &currOffs);
		gcry_cipher_close(gcry_chd);
	}
	r = 0;
done:	return r;
}

static void
decrypt(const char *name)
{
	FILE *logfp = NULL, *eifp = NULL;
	int r = 0;
	char eifname[4096];
	
	if(!strcmp(name, "-")) {
		fprintf(stderr, "decrypt mode cannot work on stdin\n");
		goto err;
	} else {
		if((logfp = fopen(name, "r")) == NULL) {
			perror(name);
			goto err;
		}
		snprintf(eifname, sizeof(eifname), "%s%s", name, ENCINFO_SUFFIX);
		eifname[sizeof(eifname)-1] = '\0';
		if((eifp = fopen(eifname, "r")) == NULL) {
			perror(eifname);
			goto err;
		}
		if(eiCheckFiletype(eifp) != 0)
			goto err;
	}

	if((r = doDecrypt(logfp, eifp, stdout)) != 0)
		goto err;

	fclose(logfp); logfp = NULL;
	fclose(eifp); eifp = NULL;
	return;

err:
	fprintf(stderr, "error %d processing file %s\n", r, name);
	if(eifp != NULL)
		fclose(eifp);
	if(logfp != NULL)
		fclose(logfp);
}

static void
write_keyfile(char *fn)
{
	int fd;
	int r;
	mode_t fmode;

	fmode = O_WRONLY|O_CREAT;
	if(!optionForce)
		fmode |= O_EXCL;
	if(fn == NULL) {
		fprintf(stderr, "program error: keyfile is NULL");
		exit(1);
	}
	if((fd = open(fn, fmode, S_IRUSR)) == -1) {
		fprintf(stderr, "error opening keyfile ");
		perror(fn);
		exit(1);
	}
	if((r = write(fd, cry_key, cry_keylen)) != (ssize_t)cry_keylen) {
		fprintf(stderr, "error writing keyfile (ret=%d) ", r);
		perror(fn);
		exit(1);
	}
	close(fd);
}

static void
getKeyFromFile(const char *fn)
{
	const int r = gcryGetKeyFromFile(fn, &cry_key, &cry_keylen);
	if(r != 0) {
		perror(fn);
		exit(1);
	}
}

static void
getRandomKey(void)
{
	int fd;
	cry_keylen = randomKeyLen;
	cry_key = malloc(randomKeyLen); /* do NOT zero-out! */
	/* if we cannot obtain data from /dev/urandom, we use whatever
	 * is present at the current memory location as random data. Of
	 * course, this is very weak and we should consider a different
	 * option, especially when not running under Linux (for Linux,
	 * unavailability of /dev/urandom is just a theoretic thing, it
	 * will always work...).  -- TODO -- rgerhards, 2013-03-06
	 */
	if((fd = open("/dev/urandom", O_RDONLY)) >= 0) {
		if(read(fd, cry_key, randomKeyLen) != randomKeyLen) {
			fprintf(stderr, "warning: could not read sufficient data "
				"from /dev/urandom - key may be weak\n");
		};
		close(fd);
	}
}


static void
setKey(void)
{
	if(randomKeyLen != -1)
		getRandomKey();
	else if(keyfile != NULL)
		getKeyFromFile(keyfile);
	else if(keyprog != NULL)
		gcryGetKeyFromProg(keyprog, &cry_key, &cry_keylen);
	if(cry_key == NULL) {
		fprintf(stderr, "ERROR: key must be set via some method\n");
		exit(1);
	}
}

static struct option long_options[] =
{
	{"verbose", no_argument, NULL, 'v'},
	{"version", no_argument, NULL, 'V'},
	{"decrypt", no_argument, NULL, 'd'},
	{"force", no_argument, NULL, 'f'},
	{"write-keyfile", required_argument, NULL, 'W'},
	{"key", required_argument, NULL, 'K'},
	{"generate-random-key", required_argument, NULL, 'r'},
	{"keyfile", required_argument, NULL, 'k'},
	{"key-program", required_argument, NULL, 'p'},
	{"algo", required_argument, NULL, 'a'},
	{"mode", required_argument, NULL, 'm'},
	{NULL, 0, NULL, 0}
};

int
main(int argc, char *argv[])
{
	int i;
	int opt;
	int temp;
	char *newKeyFile = NULL;

	while(1) {
		opt = getopt_long(argc, argv, "a:dfk:K:m:p:r:vVW:", long_options, NULL);
		if(opt == -1)
			break;
		switch(opt) {
		case 'd':
			mode = MD_DECRYPT;
			break;
		case 'W':
			mode = MD_WRITE_KEYFILE;
			newKeyFile = optarg;
			break;
		case 'k':
			keyfile = optarg;
			break;
		case 'p':
			keyprog = optarg;
			break;
		case 'f':
			optionForce = 1;
			break;
		case 'r':
			randomKeyLen = atoi(optarg);
			if(randomKeyLen > 64*1024) {
				fprintf(stderr, "ERROR: keys larger than 64KiB are "
					"not supported\n");
				exit(1);
			}
			break;
		case 'K':
			fprintf(stderr, "WARNING: specifying the actual key "
				"via the command line is highly insecure\n"
				"Do NOT use this for PRODUCTION use.\n");
			cry_key = optarg;
			cry_keylen = strlen(cry_key);
			break;
		case 'a':
			temp = rsgcryAlgoname2Algo(optarg);
			if(temp == GCRY_CIPHER_NONE) {
				fprintf(stderr, "ERROR: algorithm \"%s\" is not "
					"kown/supported\n", optarg);
				exit(1);
			}
			cry_algo = temp;
			break;
		case 'm':
			temp = rsgcryModename2Mode(optarg);
			if(temp == GCRY_CIPHER_MODE_NONE) {
				fprintf(stderr, "ERROR: cipher mode \"%s\" is not "
					"kown/supported\n", optarg);
				exit(1);
			}
			cry_mode = temp;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'V':
			fprintf(stderr, "rsgtutil " VERSION "\n");
			exit(0);
			break;
		case '?':
			break;
		default:fprintf(stderr, "getopt_long() returns unknown value %d\n", opt);
			return 1;
		}
	}

	setKey();

	if(mode == MD_WRITE_KEYFILE) {
		if(optind != argc) {
			fprintf(stderr, "ERROR: no file parameters permitted in "
				"--write-keyfile mode\n");
			exit(1);
		}
		write_keyfile(newKeyFile);
	} else {
		if(optind == argc)
			decrypt("-");
		else {
			for(i = optind ; i < argc ; ++i)
				decrypt(argv[i]);
		}
	}

	memset(cry_key, 0, cry_keylen); /* zero-out key store */
	cry_keylen = 0;
	return 0;
}
