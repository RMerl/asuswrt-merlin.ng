/* lib_ksils12.h - rsyslog's KSI-LS12 support library
 *
 * Copyright 2013-2017 Adiscon GmbH and Guardtime, Inc.
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
#ifndef INCLUDED_KSILS12_H
#define INCLUDED_KSILS12_H
#include <ksi/ksi.h>

#include "lib_ksi_queue.h"

#define MAX_ROOTS 64

/* Flags and record types for TLV handling */
#define RSGT_FLAG_NONCRIT 0x20
#define RSGT_FLAG_FORWARD 0x40
#define RSGT_TYPE_MASK 0x1f
#define RSGT_FLAG_TLV16 0x80

/* check return state of operation and abort, if non-OK */
#define CHKr(code) if((r = code) != 0) goto done

/* check the return value of a ksi api call and log a message in case of error */
#define CHECK_KSI_API(code, context, msg) if((res = code) != 0) do { \
	reportKSIAPIErr(context, NULL, msg, res); \
	goto cleanup; \
	} while (0)


typedef enum LOGSIG_SyncMode_en {
	/** The block hashes and ksi signatures in one file */
	LOGSIG_ASYNCHRONOUS = 0x00,
	/** The block hashes and ksi signatures split into separate files */
	LOGSIG_SYNCHRONOUS = 0x01
} LOGSIG_SyncMode;


/* Max number of roots inside the forest. This permits blocks of up to
 * 2^MAX_ROOTS records. We assume that 64 is sufficient for all use
 * cases ;) [and 64 is not really a waste of memory, so we do not even
 * try to work with reallocs and such...]
 */

typedef struct rsksictx_s *rsksictx;
typedef struct ksifile_s *ksifile;
typedef struct ksierrctx_s ksierrctx_t;


/* context for gt calls. This primarily serves as a container for the
 * config settings. The actual file-specific data is kept in ksifile.
 */
struct rsksictx_s {
	KSI_CTX *ksi_ctx;	/* libksi's context object */
	KSI_DataHasher *hasher;
	KSI_HashAlgorithm hashAlg;
	KSI_HashAlgorithm hmacAlg;
	uint8_t bKeepRecordHashes;
	uint8_t bKeepTreeHashes;
	uint64_t blockLevelLimit;
	uint32_t blockTimeLimit;
	uint8_t syncMode;
	uid_t	fileUID;	/* IDs for creation */
	uid_t	dirUID;
	gid_t	fileGID;
	gid_t	dirGID;
	int fCreateMode; /* mode to use when creating files */
	int fDirCreateMode; /* mode to use when creating files */
	char* aggregatorUri;
	char* aggregatorId;
	char* aggregatorKey;
	char* random_source;
	pthread_mutex_t module_lock;
	pthread_t signer_thread;
	ProtectedQueue *signer_queue;
	bool thread_started;
	uint8_t disabled; /* permits to disable the plugin --> set to 1 */
	ksifile ksi;
	bool debug;
	uint64_t max_requests;
	void (*errFunc)(void *, unsigned char*);
	void (*logFunc)(void *, unsigned char*);
	void *usrptr; /* for error function */
};

/* this describes a file, as far as librsksi is concerned */
struct ksifile_s {
	/* the following data items are mirrored from rsksictx to
	 * increase cache hit ratio (they are frequently accesed).
	 */
	KSI_HashAlgorithm hashAlg;
	uint8_t bKeepRecordHashes;
	uint8_t bKeepTreeHashes;
	uint64_t blockSizeLimit;
	uint32_t blockTimeLimit;
	/* end mirrored properties */
	uint8_t disabled; /* permits to disable this file --> set to 1 */
	uint8_t *IV; /* initial value for blinding masks */
	unsigned char lastLeaf[KSI_MAX_IMPRINT_LEN]; /* last leaf hash (maybe of previous block)
							--> preserve on term */
	unsigned char *blockfilename;
	unsigned char *ksifilename;
	unsigned char *statefilename;
	uint64_t nRecords;  /* current number of records in current block */
	uint64_t bInBlk;    /* are we currently inside a blk --> need to finish on close */
	time_t blockStarted;
	int8_t nRoots;
	/* algo engineering: roots structure is split into two arrays
	 * in order to improve cache hits.
	 */
	KSI_DataHash *roots[MAX_ROOTS];
	/* data members for the associated TLV file */
	FILE *blockFile;
	rsksictx ctx;
};

/* the following defines the ksistate file record. Currently, this record
 * is fixed, we may change that over time.
 */
struct rsksistatefile {
	char hdr[9];	/* must be "KSISTAT10" */
	uint8_t hashID;
	uint8_t lenHash;
	/* after that, the hash value is contained within the file */
};

/* error states */
#define RSGTE_SUCCESS 0 /* Success state */
#define RSGTE_IO 1 	/* any kind of io error */
#define RSGTE_FMT 2	/* data fromat error */
#define RSGTE_INVLTYP 3	/* invalid TLV type record (unexcpected at this point) */
#define RSGTE_OOM 4	/* ran out of memory */
#define RSGTE_LEN 5	/* error related to length records */
#define RSGTE_SIG_EXTEND 6/* error extending signature */
#define RSGTE_INVLD_RECCNT 7/* mismatch between actual records and records
				given in block-sig record */
#define RSGTE_INVLHDR 8/* invalid file header */
#define RSGTE_EOF 9 	/* specific EOF */
#define RSGTE_MISS_REC_HASH 10 /* record hash missing when expected */
#define RSGTE_MISS_TREE_HASH 11 /* tree hash missing when expected */
#define RSGTE_INVLD_REC_HASH 12 /* invalid record hash (failed verification) */
#define RSGTE_INVLD_TREE_HASH 13 /* invalid tree hash (failed verification) */
#define RSGTE_INVLD_REC_HASHID 14 /* invalid record hash ID (failed verification) */
#define RSGTE_INVLD_TREE_HASHID 15 /* invalid tree hash ID (failed verification) */
#define RSGTE_MISS_BLOCKSIG 16 /* block signature record missing when expected */
#define RSGTE_INVLD_SIGNATURE 17 /* Signature is invalid (KSI_Signature_verifyDataHash)*/
#define RSGTE_TS_CREATEHASH 18 /* error creating HASH (KSI_DataHash_create) */
#define RSGTE_TS_DERENCODE 19 /* error DER-Encoding a timestamp */
#define RSGTE_HASH_CREATE 20 /* error creating a hash */
#define RSGTE_END_OF_SIG 21 /* unexpected end of signature - more log line exist */
#define RSGTE_END_OF_LOG 22 /* unexpected end of log file - more signatures exist */
#define RSGTE_EXTRACT_HASH 23 /* error extracting hashes for record */
#define RSGTE_CONFIG_ERROR 24 /* Configuration error */
#define RSGTE_NETWORK_ERROR 25 /* Network error */
#define RSGTE_MISS_KSISIG 26 /* KSI signature missing */
#define RSGTE_INTERNAL 27 /* Internal error */

#define getIVLenKSI(bh) (hashOutputLengthOctetsKSI((bh)->hashID))
#define rsksiSetBlockLevelLimit(ctx, limit) ((ctx)->blockLevelLimit = limit)
#define rsksiSetBlockTimeLimit(ctx, limit) ((ctx)->blockTimeLimit = limit)
#define rsksiSetKeepRecordHashes(ctx, val) ((ctx)->bKeepRecordHashes = val)
#define rsksiSetKeepTreeHashes(ctx, val) ((ctx)->bKeepTreeHashes = val)
#define rsksiSetFileFormat(ctx, val) ((ctx)->fileFormat = val)
#define rsksiSetSyncMode(ctx, val) ((ctx)->syncMode = val)
#define rsksiSetRandomSource(ctx, val) ((ctx)->random_source = strdup(val))
#define rsksiSetFileUID(ctx, val) ((ctx)->fileUID = val)	/* IDs for creation */
#define rsksiSetDirUID(ctx, val) ((ctx)->dirUID = val)
#define rsksiSetFileGID(ctx, val) ((ctx)->fileGID= val)
#define rsksiSetDirGID(ctx, val) ((ctx)->dirGID = val)
#define rsksiSetCreateMode(ctx, val) ((ctx)->fCreateMode= val)
#define rsksiSetDirCreateMode(ctx, val) ((ctx)->fDirCreateMode = val)
#define rsksiSetDebug(ctx, val) ((ctx)->debug = val)

int rsksiSetAggregator(rsksictx ctx, char *uri, char *loginid, char *key);
int rsksiSetHashFunction(rsksictx ctx, char *algName);
int rsksiSetHmacFunction(rsksictx ctx, char *algName);
int rsksiInitModule(rsksictx ctx);
rsksictx rsksiCtxNew(void);
void rsksisetErrFunc(rsksictx ctx, void (*func)(void*, unsigned char *), void *usrptr);
void rsksisetLogFunc(rsksictx ctx, void (*func)(void*, unsigned char *), void *usrptr);
void reportKSIAPIErr(rsksictx ctx, ksifile ksi, const char *apiname, int ecode);
ksifile rsksiCtxOpenFile(rsksictx ctx, unsigned char *logfn);
int rsksifileDestruct(ksifile ksi);
void rsksiCtxDel(rsksictx ctx);
void sigblkInitKSI(ksifile ksi);
int sigblkAddRecordKSI(ksifile ksi, const unsigned char *rec, const size_t len);
int sigblkAddLeaf(ksifile ksi, const unsigned char *rec, const size_t len, bool metadata);
unsigned sigblkCalcLevel(unsigned leaves);
int sigblkFinishKSI(ksifile ksi);
int sigblkAddMetadata(ksifile ksi, const char *key, const char *value);
int sigblkCreateMask(ksifile ksi, KSI_DataHash **m);
int sigblkCreateHash(ksifile ksi, KSI_DataHash **r, const unsigned char *rec, const size_t len);
int sigblkHashTwoNodes(ksifile ksi, KSI_DataHash **node, KSI_DataHash *m, KSI_DataHash *r, uint8_t level);

#endif  /* #ifndef INCLUDED_KSILS12_H */
