/* Definition of serial stream class (strm).
 *
 * A serial stream provides serial data access. In theory, serial streams
 * can be implemented via a number of methods (e.g. files or in-memory
 * streams). In practice, there currently only exist the file type (aka
 * "driver").
 *
 * In practice, many stream features are bound to files. I have not yet made
 * any serious effort, except for the naming of this class, to try to make
 * the interfaces very generic. However, I assume that we could work much
 * like in the strm class, where some properties are simply ignored when
 * the wrong strm mode is selected (which would translate here to the wrong
 * stream mode).
 *
 * Most importantly, this class provides generic input and output functions
 * which can directly be used to work with the strms and file output. It
 * provides such useful things like a circular file buffer and, hopefully
 * at a later stage, a lazy writer. The object is also seriazable and thus
 * can easily be persistet. The bottom line is that it makes much sense to
 * use this class whereever possible as its features may grow in the future.
 *
 * An important note on writing gzip format via zlib (kept anonymous
 * by request):
 *
 * --------------------------------------------------------------------------
 * We'd like to make sure the output file is in full gzip format
 * (compatible with gzip -d/zcat etc).  There is a flag in how the output
 * is initialized within zlib to properly add the gzip wrappers to the
 * output.  (gzip is effectively a small metadata wrapper around raw
 * zstream output.)
 *
 * I had written an old bit of code to do this - the documentation on
 * deflatInit2() was pretty tricky to nail down on this specific feature:
 *
 * int deflateInit2 (z_streamp strm, int level, int method, int windowBits,
 * int memLevel, int strategy);
 *
 * I believe "31" would be the value for the "windowBits" field that you'd
 * want to try:
 *
 * deflateInit2(zstrmptr, 6, Z_DEFLATED, 31, 9, Z_DEFAULT_STRATEGY);
 * --------------------------------------------------------------------------
 *
 * Copyright 2008-2018 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * The rsyslog runtime library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The rsyslog runtime library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the rsyslog runtime library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 * A copy of the LGPL can be found in the file "COPYING.LESSER" in this distribution.
 */

#ifndef STREAM_H_INCLUDED
#define STREAM_H_INCLUDED

#include <regex.h> // TODO: fix via own module
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include "obj-types.h"
#include "glbl.h"
#include "stream.h"
#include "zlibw.h"
#include "cryprov.h"

/* stream types */
typedef enum {
	STREAMTYPE_FILE_SINGLE = 0,	/**< read a single file */
	STREAMTYPE_FILE_CIRCULAR = 1,	/**< circular files */
	STREAMTYPE_FILE_MONITOR = 2,	/**< monitor a (third-party) file */
	STREAMTYPE_NAMED_PIPE = 3	/**< file is a named pipe (so far, tested for output only) */
} strmType_t;

typedef enum {				/* when extending, do NOT change existing modes! */
	STREAMMMODE_INVALID = 0,
	STREAMMODE_READ = 1,
	STREAMMODE_WRITE = 2,
	STREAMMODE_WRITE_TRUNC = 3,
	STREAMMODE_WRITE_APPEND = 4
} strmMode_t;

#define STREAM_ASYNC_NUMBUFS 2 /* must be a power of 2 -- TODO: make configurable */
/* The strm_t data structure */
typedef struct strm_s {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	strmType_t sType;
	/* descriptive properties */
	unsigned int iCurrFNum;/* current file number (NOT descriptor, but the number in the file name!) */
	uchar *pszFName; /* prefix for generated filenames */
	int lenFName;
	strmMode_t tOperationsMode;
	mode_t tOpenMode;
	int64 iMaxFileSize;/* maximum size a file may grow to */
	unsigned int iMaxFiles;	/* maximum number of files if a circular mode is in use */
	int iFileNumDigits;/* min number of digits to use in file number (only in circular mode) */
	sbool bDeleteOnClose; /* set to 1 to auto-delete on close -- be careful with that setting! */
	int64 iCurrOffs;/* current offset */
	int64 *pUsrWCntr; /* NULL or a user-provided counter that receives the nbr of bytes written since
	the last CntrSet() */
	sbool bPrevWasNL; /* used for readLine() when reading multi-line messages */
	/* dynamic properties, valid only during file open, not to be persistet */
	sbool bDisabled; /* should file no longer be written to? (currently set only if omfile file size limit fails) */
	sbool bSync;	/* sync this file after every write? */
	sbool bReopenOnTruncate;
	size_t sIOBufSize;/* size of IO buffer */
	uchar *pszDir; /* Directory */
	int lenDir;
	int fd;		/* the file descriptor, -1 if closed */
	int fdDir;	/* the directory's descriptor, in case bSync is requested (-1 if closed) */
	int readTimeout;/* 0: do not timeout */
	time_t lastRead;/* for timeout processing */
	ino_t inode;	/* current inode for files being monitored (undefined else) */
	uchar *pszCurrFName; /* name of current file (if open) */
	uchar *pIOBuf;	/* the iobuffer currently in use to gather data */
	size_t iBufPtrMax;	/* current max Ptr in Buffer (if partial read!) */
	size_t iBufPtr;	/* pointer into current buffer */
	int iUngetC;	/* char set via UngetChar() call or -1 if none set */
	sbool bInRecord;	/* if 1, indicates that we are currently writing a not-yet complete record */
	int iZipLevel;	/* zip level (0..9). If 0, zip is completely disabled */
	Bytef *pZipBuf;
	/* support for async flush procesing */
	sbool bAsyncWrite;	/* do asynchronous writes (always if a flush interval is given) */
	sbool bStopWriter;	/* shall writer thread terminate? */
	sbool bDoTimedWait;	/* instruct writer thread to do a times wait to support flush timeouts */
	sbool bzInitDone;	/* did we do an init of zstrm already? */
	sbool bFlushNow;	/* shall we flush with the next async write? */
	sbool bVeryReliableZip; /* shall we write interim headers to create a very reliable ZIP file? */
	int iFlushInterval; /* flush in which interval - 0, no flushing */
	pthread_mutex_t mut;/* mutex for flush in async mode */
	pthread_cond_t notFull;
	pthread_cond_t notEmpty;
	pthread_cond_t isEmpty;
	unsigned short iEnq;	/* this MUST be unsigned as we use module arithmetic (else invalid indexing happens!) */
	unsigned short iDeq;	/* this MUST be unsigned as we use module arithmetic (else invalid indexing happens!) */
	cryprov_if_t *cryprov;  /* ptr to crypto provider; NULL = do not encrypt */
	void	*cryprovData;	/* opaque data ptr for provider use */
	void 	*cryprovFileData;/* opaque data ptr for file instance */
	short iCnt;	/* current nbr of elements in buffer */
	z_stream zstrm;	/* zip stream to use */
	struct {
		uchar *pBuf;
		size_t lenBuf;
	} asyncBuf[STREAM_ASYNC_NUMBUFS];
	pthread_t writerThreadID;
	/* support for omfile size-limiting commands, special counters, NOT persisted! */
	off_t	iSizeLimit;	/* file size limit, 0 = no limit */
	uchar	*pszSizeLimitCmd;	/* command to carry out when size limit is reached */
	sbool	bIsTTY;		/* is this a tty file? */
	cstr_t *prevLineSegment; /* for ReadLine, previous, unprocessed part of file */
	cstr_t *prevMsgSegment; /* for ReadMultiLine, previous, yet unprocessed part of msg */
	int64 strtOffs;		/* start offset in file for current line/msg */
	int fileNotFoundError;
	int noRepeatedErrorOutput; /* if a file is missing the Error is only given once */
	int ignoringMsg;
} strm_t;


/* interfaces */
BEGINinterface(strm) /* name must also be changed in ENDinterface macro! */
	rsRetVal (*Construct)(strm_t **ppThis);
	rsRetVal (*ConstructFinalize)(strm_t *pThis);
	rsRetVal (*Destruct)(strm_t **ppThis);
	rsRetVal (*SetFileName)(strm_t *pThis, uchar *pszName, size_t iLenName);
	rsRetVal (*ReadChar)(strm_t *pThis, uchar *pC);
	rsRetVal (*UnreadChar)(strm_t *pThis, uchar c);
	rsRetVal (*SeekCurrOffs)(strm_t *pThis);
	rsRetVal (*Write)(strm_t *const pThis, const uchar *const pBuf, size_t lenBuf);
	rsRetVal (*WriteChar)(strm_t *pThis, uchar c);
	rsRetVal (*WriteLong)(strm_t *pThis, long i);
	rsRetVal (*SetFileNotFoundError)(strm_t *pThis, int pFileNotFoundError);
	rsRetVal (*SetFName)(strm_t *pThis, uchar *pszPrefix, size_t iLenPrefix);
	rsRetVal (*SetDir)(strm_t *pThis, uchar *pszDir, size_t iLenDir);
	rsRetVal (*Flush)(strm_t *pThis);
	rsRetVal (*RecordBegin)(strm_t *pThis);
	rsRetVal (*RecordEnd)(strm_t *pThis);
	rsRetVal (*Serialize)(strm_t *pThis, strm_t *pStrm);
	rsRetVal (*GetCurrOffset)(strm_t *pThis, int64 *pOffs);
	rsRetVal (*SetWCntr)(strm_t *pThis, number_t *pWCnt);
	rsRetVal (*Dup)(strm_t *pThis, strm_t **ppNew);
	INTERFACEpropSetMeth(strm, bDeleteOnClose, int);
	INTERFACEpropSetMeth(strm, iMaxFileSize, int64);
	INTERFACEpropSetMeth(strm, iMaxFiles, int);
	INTERFACEpropSetMeth(strm, iFileNumDigits, int);
	INTERFACEpropSetMeth(strm, tOperationsMode, int);
	INTERFACEpropSetMeth(strm, tOpenMode, mode_t);
	INTERFACEpropSetMeth(strm, sType, strmType_t);
	INTERFACEpropSetMeth(strm, iZipLevel, int);
	INTERFACEpropSetMeth(strm, bSync, int);
	INTERFACEpropSetMeth(strm, bReopenOnTruncate, int);
	INTERFACEpropSetMeth(strm, sIOBufSize, size_t);
	INTERFACEpropSetMeth(strm, iSizeLimit, off_t);
	INTERFACEpropSetMeth(strm, iFlushInterval, int);
	INTERFACEpropSetMeth(strm, pszSizeLimitCmd, uchar*);
	/* v6 added */
	rsRetVal (*ReadLine)(strm_t *pThis, cstr_t **ppCStr, uint8_t mode, sbool bEscapeLF,
		uint32_t trimLineOverBytes, int64 *const strtOffs);
	/* v7 added  2012-09-14 */
	INTERFACEpropSetMeth(strm, bVeryReliableZip, int);
	/* v8 added  2013-03-21 */
	rsRetVal (*CheckFileChange)(strm_t *pThis);
	/* v9 added  2013-04-04 */
	INTERFACEpropSetMeth(strm, cryprov, cryprov_if_t*);
	INTERFACEpropSetMeth(strm, cryprovData, void*);
ENDinterface(strm)
#define strmCURR_IF_VERSION 13 /* increment whenever you change the interface structure! */
/* V10, 2013-09-10: added new parameter bEscapeLF, changed mode to uint8_t (rgerhards) */
/* V11, 2015-12-03: added new parameter bReopenOnTruncate */
/* V12, 2015-12-11: added new parameter trimLineOverBytes, changed mode to uint32_t */
/* V13, 2017-09-06: added new parameter strtoffs to ReadLine() */

#define strmGetCurrFileNum(pStrm) ((pStrm)->iCurrFNum)

/* prototypes */
PROTOTYPEObjClassInit(strm);
rsRetVal strmMultiFileSeek(strm_t *pThis, unsigned int fileNum, off64_t offs, off64_t *bytesDel);
rsRetVal strmReadMultiLine(strm_t *pThis, cstr_t **ppCStr, regex_t *start_preg, regex_t *end_preg,
	sbool bEscapeLF, sbool discardTruncatedMsg, sbool msgDiscardingError, int64 *const strtOffs);
int strmReadMultiLine_isTimedOut(const strm_t *const __restrict__ pThis);
void strmDebugOutBuf(const strm_t *const pThis);
void strmSetReadTimeout(strm_t *const __restrict__ pThis, const int val);
const uchar * ATTR_NONNULL() strmGetPrevLineSegment(strm_t *const pThis);
const uchar * ATTR_NONNULL() strmGetPrevMsgSegment(strm_t *const pThis);
int ATTR_NONNULL() strmGetPrevWasNL(const strm_t *const pThis);

#endif /* #ifndef STREAM_H_INCLUDED */
