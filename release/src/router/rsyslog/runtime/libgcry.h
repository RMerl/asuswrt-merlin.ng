/* libgcry.h - rsyslog's guardtime support library
 *
 * Copyright 2013 Adiscon GmbH.
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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef INCLUDED_LIBGCRY_H
#define INCLUDED_LIBGCRY_H
#include <stdint.h>
#include <gcrypt.h>

struct gcryctx_s {
	uchar *key;
	size_t keyLen;
	int algo;
	int mode;
};
typedef struct gcryctx_s *gcryctx;
typedef struct gcryfile_s *gcryfile;

/* this describes a file, as far as libgcry is concerned */
struct gcryfile_s {
	gcry_cipher_hd_t chd; /* cypher handle */
	size_t blkLength; /* size of low-level crypto block */
	uchar *eiName; /* name of .encinfo file */
	int fd; /* descriptor of .encinfo file (-1 if not open) */
	char openMode; /* 'r': read, 'w': write */
	gcryctx ctx;
	uchar *readBuf;
	int16_t readBufIdx;
	int16_t readBufMaxIdx;
	int8_t bDeleteOnClose; /* for queue support, similar to stream subsys */
	ssize_t bytesToBlkEnd; /* number of bytes remaining in current crypto block
				-1 means -> no end (still being writen to, queue files),
				0 means -> end of block, new one must be started. */
};

int gcryGetKeyFromFile(const char *fn, char **key, unsigned *keylen);
int rsgcryInit(void);
void rsgcryExit(void);
int rsgcrySetKey(gcryctx ctx, unsigned char *key, uint16_t keyLen);
rsRetVal rsgcrySetMode(gcryctx ctx, uchar *algoname);
rsRetVal rsgcrySetAlgo(gcryctx ctx, uchar *modename);
gcryctx gcryCtxNew(void);
void rsgcryCtxDel(gcryctx ctx);
int gcryfileDestruct(gcryfile gf, off64_t offsLogfile);
rsRetVal rsgcryInitCrypt(gcryctx ctx, gcryfile *pgf, uchar *fname, char openMode);
rsRetVal rsgcryEncrypt(gcryfile pF, uchar *buf, size_t *len);
rsRetVal rsgcryDecrypt(gcryfile pF, uchar *buf, size_t *len);
int gcryGetKeyFromProg(char *cmd, char **key, unsigned *keylen);
rsRetVal gcryfileDeleteState(uchar *fn);
rsRetVal gcryfileGetBytesLeftInBlock(gcryfile gf, ssize_t *left);
int rsgcryModename2Mode(char *const __restrict__ modename);
int rsgcryAlgoname2Algo(char *const __restrict__ algoname);

/* error states */
#define RSGCRYE_EI_OPEN 1 	/* error opening .encinfo file */
#define RSGCRYE_OOM 4	/* ran out of memory */

#define EIF_MAX_RECTYPE_LEN 31 /* max length of record types */
#define EIF_MAX_VALUE_LEN 1023 /* max length of value types */
#define RSGCRY_FILETYPE_NAME "rsyslog-enrcyption-info"
#define ENCINFO_SUFFIX ".encinfo"

/* Note: gf may validly be NULL, e.g. if file has not yet been opened! */
static inline void __attribute__((unused))
gcryfileSetDeleteOnClose(gcryfile gf, const int val)
{
	if(gf != NULL)
		gf->bDeleteOnClose = val;
}

#endif  /* #ifndef INCLUDED_LIBGCRY_H */
