/* The serial stream class.
 *
 * A serial stream provides serial data access. In theory, serial streams
 * can be implemented via a number of methods (e.g. files or in-memory
 * streams). In practice, there currently only exist the file type (aka
 * "driver").
 *
 * File begun on 2008-01-09 by RGerhards
 * Large modifications in 2009-06 to support using it with omfile, including zip writer.
 * Note that this file obtains the zlib wrapper object is needed, but it never frees it
 * again. While this sounds like a leak (and one may argue it actually is), there is no
 * harm associated with that. The reason is that strm is a core object, so it is terminated
 * only when rsyslogd exists. As we could only release on termination (or else bear more
 * overhead for keeping track of how many users we have), not releasing zlibw is OK, because
 * it will be released when rsyslogd terminates. We may want to revisit this decision if
 * it turns out to be problematic. Then, we need to quasi-refcount the number of accesses
 * to the object.
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
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>	 /* required for HP UX */
#include <errno.h>
#include <pthread.h>
#include <poll.h>
#ifdef HAVE_SYS_PRCTL_H
#  include <sys/prctl.h>
#endif

#include "rsyslog.h"
#include "stringbuf.h"
#include "srUtils.h"
#include "obj.h"
#include "stream.h"
#include "unicode-helper.h"
#include "module-template.h"
#include "errmsg.h"
#include "cryprov.h"
#include "datetime.h"

/* some platforms do not have large file support :( */
#ifndef O_LARGEFILE
#  define O_LARGEFILE 0
#endif
#ifndef HAVE_LSEEK64
#  define lseek64(fd, offset, whence) lseek(fd, offset, whence)
#endif

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(zlibw)

/* forward definitions */
static rsRetVal strmFlushInternal(strm_t *pThis, int bFlushZip);
static rsRetVal strmWrite(strm_t *__restrict__ const pThis, const uchar *__restrict__ const pBuf,
	const size_t lenBuf);
static rsRetVal strmCloseFile(strm_t *pThis);
static void *asyncWriterThread(void *pPtr);
static rsRetVal doZipWrite(strm_t *pThis, uchar *pBuf, size_t lenBuf, int bFlush);
static rsRetVal doZipFinish(strm_t *pThis);
static rsRetVal strmPhysWrite(strm_t *pThis, uchar *pBuf, size_t lenBuf);
static rsRetVal strmSeekCurrOffs(strm_t *pThis);


/* methods */

/* note: this may return NULL if not line segment is currently set  */
// TODO: due to the cstrFinalize() this is not totally clean, albeit for our
// current use case it does not hurt -- refactor! rgerhards, 2018-03-27
const uchar * ATTR_NONNULL()
strmGetPrevLineSegment(strm_t *const pThis)
{
	const uchar *ret = NULL;
	if(pThis->prevLineSegment != NULL) {
		cstrFinalize(pThis->prevLineSegment);
		ret = rsCStrGetSzStrNoNULL(pThis->prevLineSegment);
	}
	return ret;
}
/* note: this may return NULL if not line segment is currently set  */
// TODO: due to the cstrFinalize() this is not totally clean, albeit for our
// current use case it does not hurt -- refactor! rgerhards, 2018-03-27
const uchar * ATTR_NONNULL()
strmGetPrevMsgSegment(strm_t *const pThis)
{
	const uchar *ret = NULL;
	if(pThis->prevMsgSegment != NULL) {
		cstrFinalize(pThis->prevMsgSegment);
		ret = rsCStrGetSzStrNoNULL(pThis->prevMsgSegment);
	}
	return ret;
}


int ATTR_NONNULL()
strmGetPrevWasNL(const strm_t *const pThis)
{
	return pThis->bPrevWasNL;
}


/* output (current) file name for debug log purposes. Falls back to various
 * levels of impreciseness if more precise name is not known.
 */
static const char *
getFileDebugName(const strm_t *const pThis)
{
	  return (pThis->pszCurrFName == NULL) ?
		  ((pThis->pszFName == NULL) ? "N/A" : (char*)pThis->pszFName)
		: (const char*) pThis->pszCurrFName;
}

/* Try to resolve a size limit situation. This is used to support custom-file size handlers
 * for omfile. It first runs the command, and then checks if we are still above the size
 * treshold. Note that this works only with single file names, NOT with circular names.
 * Note that pszCurrFName can NOT be taken from pThis, because the stream is closed when
 * we are called (and that destroys pszCurrFName, as there is NO CURRENT file name!). So
 * we need to receive the name as a parameter.
 * initially wirtten 2005-06-21, moved to this class & updates 2009-06-01, both rgerhards
 */
static rsRetVal
resolveFileSizeLimit(strm_t *pThis, uchar *pszCurrFName)
{
	uchar *pParams;
	uchar *pCmd;
	uchar *p;
	off_t actualFileSize;
	rsRetVal localRet;
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strm);
	assert(pszCurrFName != NULL);

	if(pThis->pszSizeLimitCmd == NULL) {
		ABORT_FINALIZE(RS_RET_NON_SIZELIMITCMD); /* nothing we can do in this case... */
	}
	
	/* we first check if we have command line parameters. We assume this,
	 * when we have a space in the program name. If we find it, everything after
	 * the space is treated as a single argument.
	 */
	CHKmalloc(pCmd = ustrdup(pThis->pszSizeLimitCmd));

	for(p = pCmd ; *p && *p != ' ' ; ++p) {
		/* JUST SKIP */
	}

	if(*p == ' ') {
		*p = '\0'; /* pretend string-end */
		pParams = p+1;
	} else
		pParams = NULL;

	/* the execProg() below is probably not great, but at least is is
	 * fairly secure now. Once we change the way file size limits are
	 * handled, we should also revisit how this command is run (and
	 * with which parameters).   rgerhards, 2007-07-20
	 */
	execProg(pCmd, 1, pParams);

	free(pCmd);

	localRet = getFileSize(pszCurrFName, &actualFileSize);

	if(localRet == RS_RET_OK && actualFileSize >= pThis->iSizeLimit) {
		ABORT_FINALIZE(RS_RET_SIZELIMITCMD_DIDNT_RESOLVE); /* OK, it didn't work out... */
	} else if(localRet != RS_RET_FILE_NOT_FOUND) {
		/* file not found is OK, the command may have moved away the file */
		ABORT_FINALIZE(localRet);
	}

finalize_it:
	if(iRet != RS_RET_OK) {
		if(iRet == RS_RET_SIZELIMITCMD_DIDNT_RESOLVE) {
			LogError(0, RS_RET_ERR, "file size limit cmd for file '%s' "
				"did no resolve situation\n", pszCurrFName);
		} else {
			LogError(0, RS_RET_ERR, "file size limit cmd for file '%s' "
				"failed with code %d.\n", pszCurrFName, iRet);
		}
		pThis->bDisabled = 1;
	}

	RETiRet;
}


/* Check if the file has grown beyond the configured omfile iSizeLimit
 * and, if so, initiate processing.
 */
static rsRetVal
doSizeLimitProcessing(strm_t *pThis)
{
	uchar *pszCurrFName = NULL;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strm);
	ASSERT(pThis->iSizeLimit != 0);
	ASSERT(pThis->fd != -1);

	if(pThis->iCurrOffs >= pThis->iSizeLimit) {
		/* strmCloseFile() destroys the current file name, so we
		 * need to preserve it.
		 */
		CHKmalloc(pszCurrFName = ustrdup(pThis->pszCurrFName));
		CHKiRet(strmCloseFile(pThis));
		CHKiRet(resolveFileSizeLimit(pThis, pszCurrFName));
	}

finalize_it:
	free(pszCurrFName);
	RETiRet;
}


/* now, we define type-specific handlers. The provide a generic functionality,
 * but for this specific type of strm. The mapping to these handlers happens during
 * strm construction. Later on, handlers are called by pointers present in the
 * strm instance object.
 */

/* do the physical open() call on a file.
 */
static rsRetVal
doPhysOpen(strm_t *pThis)
{
	int iFlags = 0;
	struct stat statOpen;
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strm);

	/* compute which flags we need to provide to open */
	switch(pThis->tOperationsMode) {
		case STREAMMODE_READ:
			iFlags = O_CLOEXEC | O_NOCTTY | O_RDONLY;
			break;
		case STREAMMODE_WRITE:	/* legacy mode used inside queue engine */
			iFlags = O_CLOEXEC | O_NOCTTY | O_WRONLY | O_CREAT;
			break;
		case STREAMMODE_WRITE_TRUNC:
			iFlags = O_CLOEXEC | O_NOCTTY | O_WRONLY | O_CREAT | O_TRUNC;
			break;
		case STREAMMODE_WRITE_APPEND:
			iFlags = O_CLOEXEC | O_NOCTTY | O_WRONLY | O_CREAT | O_APPEND;
			break;
		case STREAMMMODE_INVALID:
		default:assert(0);
			break;
	}
	if(pThis->sType == STREAMTYPE_NAMED_PIPE) {
		DBGPRINTF("Note: stream '%s' is a named pipe, open with O_NONBLOCK\n", pThis->pszCurrFName);
		iFlags |= O_NONBLOCK;
	}

	pThis->fd = open((char*)pThis->pszCurrFName, iFlags | O_LARGEFILE, pThis->tOpenMode);
	const int errno_save = errno; /* dbgprintf can mangle it! */
	DBGPRINTF("file '%s' opened as #%d with mode %d\n", pThis->pszCurrFName,
		  pThis->fd, (int) pThis->tOpenMode);
	if(pThis->fd == -1) {
		const rsRetVal errcode = (errno_save == ENOENT)
			? RS_RET_FILE_NOT_FOUND : RS_RET_FILE_OPEN_ERROR;
		if(pThis->fileNotFoundError) {
			if(pThis->noRepeatedErrorOutput == 0) {
				LogError(errno_save, errcode, "file '%s': open error", pThis->pszCurrFName);
				pThis->noRepeatedErrorOutput = 1;
			}
		} else {
			DBGPRINTF("file '%s': open error", pThis->pszCurrFName);
		}
		ABORT_FINALIZE(errcode);
	} else {
		pThis->noRepeatedErrorOutput = 0;
	}

	if(pThis->tOperationsMode == STREAMMODE_READ) {
		if(fstat(pThis->fd, &statOpen) == -1) {
			DBGPRINTF("Error: cannot obtain inode# for file %s\n", pThis->pszCurrFName);
			ABORT_FINALIZE(RS_RET_IO_ERROR);
		}
		pThis->inode = statOpen.st_ino;
	}

	if(!ustrcmp(pThis->pszCurrFName, UCHAR_CONSTANT(_PATH_CONSOLE)) || isatty(pThis->fd)) {
		DBGPRINTF("file %d is a tty-type file\n", pThis->fd);
		pThis->bIsTTY = 1;
	} else {
		pThis->bIsTTY = 0;
	}

	if(pThis->cryprov != NULL) {
		CHKiRet(pThis->cryprov->OnFileOpen(pThis->cryprovData,
		 	pThis->pszCurrFName, &pThis->cryprovFileData,
			(pThis->tOperationsMode == STREAMMODE_READ) ? 'r' : 'w'));
		pThis->cryprov->SetDeleteOnClose(pThis->cryprovFileData, pThis->bDeleteOnClose);
	}

finalize_it:
	RETiRet;
}


static rsRetVal
strmSetCurrFName(strm_t *pThis)
{
	DEFiRet;

	if(pThis->sType == STREAMTYPE_FILE_CIRCULAR) {
		CHKiRet(genFileName(&pThis->pszCurrFName, pThis->pszDir, pThis->lenDir,
				    pThis->pszFName, pThis->lenFName, pThis->iCurrFNum, pThis->iFileNumDigits));
	} else {
		if(pThis->pszDir == NULL) {
			if((pThis->pszCurrFName = ustrdup(pThis->pszFName)) == NULL)
				ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
		} else {
			CHKiRet(genFileName(&pThis->pszCurrFName, pThis->pszDir, pThis->lenDir,
					    pThis->pszFName, pThis->lenFName, -1, 0));
		}
	}
finalize_it:
	RETiRet;
}
	
/* This function checks if the actual file has changed and, if so, resets the
 * offset. This is support for monitoring files. It should be called after
 * deserializing the strm object and before doing any other operation on it
 * (most importantly not an open or seek!).
 */
static rsRetVal
CheckFileChange(strm_t *pThis)
{
	struct stat statName;
	DEFiRet;

	CHKiRet(strmSetCurrFName(pThis));
	if(stat((char*) pThis->pszCurrFName, &statName) == -1)
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	DBGPRINTF("stream/after deserialize checking for file change on '%s', "
		"inode %u/%u, size/currOffs %llu/%llu\n",
		pThis->pszCurrFName, (unsigned) pThis->inode,
		(unsigned) statName.st_ino,
		(long long unsigned) statName.st_size,
		(long long unsigned) pThis->iCurrOffs);
	if(pThis->inode != statName.st_ino || statName.st_size < pThis->iCurrOffs) {
		DBGPRINTF("stream: file %s has changed\n", pThis->pszCurrFName);
		pThis->iCurrOffs = 0;
	}
finalize_it:
	RETiRet;
}


/* open a strm file
 * It is OK to call this function when the stream is already open. In that
 * case, it returns immediately with RS_RET_OK
 */
static rsRetVal strmOpenFile(strm_t *pThis)
{
	DEFiRet;
	off_t offset;

	ASSERT(pThis != NULL);

	if(pThis->fd != -1)
		ABORT_FINALIZE(RS_RET_OK);

	free(pThis->pszCurrFName);
	pThis->pszCurrFName = NULL; /* used to prevent mem leak in case of error */

	if(pThis->pszFName == NULL)
		ABORT_FINALIZE(RS_RET_FILE_PREFIX_MISSING);

	CHKiRet(strmSetCurrFName(pThis));
	
	CHKiRet(doPhysOpen(pThis));

	pThis->iCurrOffs = 0;
	CHKiRet(getFileSize(pThis->pszCurrFName, &offset));
	if(pThis->tOperationsMode == STREAMMODE_WRITE_APPEND) {
		pThis->iCurrOffs = offset;
	} else if(pThis->tOperationsMode == STREAMMODE_WRITE_TRUNC) {
		if(offset != 0) {
			LogError(0, 0, "file '%s' opened for truncate write, but "
				"already contains %zd bytes\n",
				pThis->pszCurrFName, (ssize_t) offset);
		}
	}

	DBGOPRINT((obj_t*) pThis, "opened file '%s' for %s as %d\n", pThis->pszCurrFName,
		  (pThis->tOperationsMode == STREAMMODE_READ) ? "READ" : "WRITE", pThis->fd);

finalize_it:
	if(iRet == RS_RET_OK) {
		assert(pThis->fd != -1);
	} else {
		if(pThis->pszCurrFName != NULL) {
			free(pThis->pszCurrFName);
			pThis->pszCurrFName = NULL; /* just to prevent mis-adressing down the road... */
		}
		if(pThis->fd != -1) {
			close(pThis->fd);
			pThis->fd = -1;
		}
	}
	RETiRet;
}


/* wait for the output writer thread to be done. This must be called before actions
 * that require data to be persisted. May be called in non-async mode and is a null
 * operation than. Must be called with the mutex locked.
 */
static void
strmWaitAsyncWriterDone(strm_t *pThis)
{
	BEGINfunc
	if(pThis->bAsyncWrite) {
		/* awake writer thread and make it write out everything */
		while(pThis->iCnt > 0) {
			pthread_cond_signal(&pThis->notEmpty);
			d_pthread_cond_wait(&pThis->isEmpty, &pThis->mut);
		}
	}
	ENDfunc
}


/* close a strm file
 * Note that the bDeleteOnClose flag is honored. If it is set, the file will be
 * deleted after close. This is in support for the qRead thread.
 * Note: it is valid to call this function when the physical file is closed. If so,
 * strmCloseFile() will still check if there is any unwritten data inside buffers
 * (this may be the case) and, if so, will open the file, write the data, and then
 * close it again (this is done via strmFlushInternal and friends).
 */
static rsRetVal strmCloseFile(strm_t *pThis)
{
	off64_t currOffs;
	DEFiRet;

	ASSERT(pThis != NULL);
	DBGOPRINT((obj_t*) pThis, "file %d(%s) closing, bDeleteOnClose %d\n", pThis->fd,
		getFileDebugName(pThis), pThis->bDeleteOnClose);

	if(pThis->tOperationsMode != STREAMMODE_READ) {
		strmFlushInternal(pThis, 0);
		if(pThis->iZipLevel) {
			doZipFinish(pThis);
		}
		if(pThis->bAsyncWrite) {
			strmWaitAsyncWriterDone(pThis);
		}
	}

	/* if we have a signature provider, we must make sure that the crypto
	 * state files are opened and proper close processing happens. */
	if(pThis->cryprov != NULL && pThis->fd == -1) {
		const rsRetVal localRet = strmOpenFile(pThis);
		if(localRet != RS_RET_OK) {
			LogError(0, localRet, "could not open file %s, this "
				"may result in problems with encryption - "
				"unfortunately, we cannot do anything against "
				"this.", pThis->pszCurrFName);
		}
	}

	/* the file may already be closed (or never have opened), so guard
	 * against this. -- rgerhards, 2010-03-19
	 */
	if(pThis->fd != -1) {
		currOffs = lseek64(pThis->fd, 0, SEEK_CUR);
		close(pThis->fd);
		pThis->fd = -1;
		pThis->inode = 0;
		if(pThis->cryprov != NULL) {
			pThis->cryprov->OnFileClose(pThis->cryprovFileData, currOffs);
			pThis->cryprovFileData = NULL;
		}
	}

	if(pThis->fdDir != -1) {
		/* close associated directory handle, if it is open */
		close(pThis->fdDir);
		pThis->fdDir = -1;
	}

	if(pThis->bDeleteOnClose) {
		if(pThis->pszCurrFName == NULL) {
			CHKiRet(genFileName(&pThis->pszCurrFName, pThis->pszDir, pThis->lenDir,
					    pThis->pszFName, pThis->lenFName, pThis->iCurrFNum,
					    pThis->iFileNumDigits));
		}
		DBGPRINTF("strmCloseFile: deleting '%s'\n", pThis->pszCurrFName);
		if(unlink((char*) pThis->pszCurrFName) == -1) {
			char errStr[1024];
			int err = errno;
			rs_strerror_r(err, errStr, sizeof(errStr));
			DBGPRINTF("error %d unlinking '%s' - ignored: %s\n",
				   errno, pThis->pszCurrFName, errStr);
		}
	}

	pThis->iCurrOffs = 0;	/* we are back at begin of file */

finalize_it:
	free(pThis->pszCurrFName);
	pThis->pszCurrFName = NULL;
	RETiRet;
}


/* switch to next strm file
 * This method must only be called if we are in a multi-file mode!
 */
static rsRetVal
strmNextFile(strm_t *pThis)
{
	DEFiRet;

	assert(pThis != NULL);
	assert(pThis->sType == STREAMTYPE_FILE_CIRCULAR);
	assert(pThis->iMaxFiles != 0);
	assert(pThis->fd != -1);

	CHKiRet(strmCloseFile(pThis));

	/* we do modulo operation to ensure we obey the iMaxFile property. This will always
	 * result in a file number lower than iMaxFile, so it if wraps, the name is back to
	 * 0, which results in the first file being overwritten. Not desired for queues, so
	 * make sure their iMaxFiles is large enough. But it is well-desired for other
	 * use cases, e.g. a circular output log file. -- rgerhards, 2008-01-10
	 */
	pThis->iCurrFNum = (pThis->iCurrFNum + 1) % pThis->iMaxFiles;

finalize_it:
	RETiRet;
}


/* handle the eof case for monitored files.
 * If we are monitoring a file, someone may have rotated it. In this case, we
 * also need to close it and reopen it under the same name.
 * rgerhards, 2008-02-13
 * The previous code also did a check for file truncation, in which case the
 * file was considered rewritten. However, this potential border case turned
 * out to be a big trouble spot on busy systems. It caused massive message
 * duplication (I guess stat() can return a too-low number under some
 * circumstances). So starting as of now, we only check the inode number and
 * a file change is detected only if the inode changes. -- rgerhards, 2011-01-10
 */
static rsRetVal
strmHandleEOFMonitor(strm_t *pThis)
{
	DEFiRet;
	struct stat statName;

	ISOBJ_TYPE_assert(pThis, strm);
	if(stat((char*) pThis->pszCurrFName, &statName) == -1)
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	DBGPRINTF("stream checking for file change on '%s', inode %u/%u\n",
	  pThis->pszCurrFName, (unsigned) pThis->inode,
	  (unsigned) statName.st_ino);

	/* Inode unchanged but file size on disk is less than current offset
	 * means file was truncated, we also reopen if 'reopenOnTruncate' is on
	 */
	if (pThis->inode != statName.st_ino
		  || (pThis->bReopenOnTruncate && statName.st_size < pThis->iCurrOffs)) {
		DBGPRINTF("we had a file change on '%s'\n", pThis->pszCurrFName);
		CHKiRet(strmCloseFile(pThis));
		CHKiRet(strmOpenFile(pThis));
	} else {
		ABORT_FINALIZE(RS_RET_EOF);
	}

finalize_it:
	RETiRet;
}


/* handle the EOF case of a stream
 * The EOF case is somewhat complicated, as the proper action depends on the
 * mode the stream is in. If there are multiple files (circular logs, most
 * important use case is queue files!), we need to close the current file and
 * try to open the next one.
 * rgerhards, 2008-02-13
 */
static rsRetVal
strmHandleEOF(strm_t *pThis)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strm);
	switch(pThis->sType) {
		case STREAMTYPE_FILE_SINGLE:
		case STREAMTYPE_NAMED_PIPE:
			ABORT_FINALIZE(RS_RET_EOF);
			break;
		case STREAMTYPE_FILE_CIRCULAR:
			/* we have multiple files and need to switch to the next one */
			/* TODO: think about emulating EOF in this case (not yet needed) */
			DBGOPRINT((obj_t*) pThis, "file %d EOF\n", pThis->fd);
			CHKiRet(strmNextFile(pThis));
			break;
		case STREAMTYPE_FILE_MONITOR:
			CHKiRet(strmHandleEOFMonitor(pThis));
			break;
	}

finalize_it:
	RETiRet;
}

/* read the next buffer from disk
 * rgerhards, 2008-02-13
 */
static rsRetVal
strmReadBuf(strm_t *pThis, int *padBytes)
{
	DEFiRet;
	int bRun;
	long iLenRead;
	size_t actualDataLen;
	size_t toRead;
	ssize_t bytesLeft;

	ISOBJ_TYPE_assert(pThis, strm);
	/* We need to try read at least twice because we may run into EOF and need to switch files. */
	bRun = 1;
	while(bRun) {
		/* first check if we need to (re)open the file. We may have switched to a new one in
		 * circular mode or it may have been rewritten (rotated) if we monitor a file
		 * rgerhards, 2008-02-13
		 */
		CHKiRet(strmOpenFile(pThis));
		if(pThis->cryprov == NULL) {
			toRead = pThis->sIOBufSize;
		} else {
			CHKiRet(pThis->cryprov->GetBytesLeftInBlock(pThis->cryprovFileData, &bytesLeft));
			if(bytesLeft == -1 || bytesLeft > (ssize_t) pThis->sIOBufSize)  {
				toRead = pThis->sIOBufSize;
			} else {
				toRead = (size_t) bytesLeft;
			}
		}
		iLenRead = read(pThis->fd, pThis->pIOBuf, toRead);
		DBGOPRINT((obj_t*) pThis, "file %d read %ld bytes\n", pThis->fd, iLenRead);
		/* end crypto */
		if(iLenRead == 0) {
			CHKiRet(strmHandleEOF(pThis));
		} else if(iLenRead < 0)
			ABORT_FINALIZE(RS_RET_IO_ERROR);
		else { /* good read */
			/* here we place our crypto interface */
			if(pThis->cryprov != NULL) {
				actualDataLen = iLenRead;
				pThis->cryprov->Decrypt(pThis->cryprovFileData, pThis->pIOBuf, &actualDataLen);
				*padBytes = iLenRead - actualDataLen;
				iLenRead = actualDataLen;
				DBGOPRINT((obj_t*) pThis, "encrypted file %d pad bytes %d, actual "
					"data %ld\n", pThis->fd, *padBytes, iLenRead);
			} else {
				*padBytes = 0;
			}
			pThis->iBufPtrMax = iLenRead;
			bRun = 0;	/* exit loop */
		}
	}
	/* if we reach this point, we had a good read */
	pThis->iBufPtr = 0;

finalize_it:
	RETiRet;
}


/* debug output of current buffer */
void
strmDebugOutBuf(const strm_t *const pThis)
{
	int strtIdx = pThis->iBufPtr - 50;
	if(strtIdx < 0)
		strtIdx = 0;
	DBGOPRINT((obj_t*) pThis, "strmRead ungetc %d, index %zd, max %zd, buf '%.*s', CURR: '%.*s'\n",
		pThis->iUngetC, pThis->iBufPtr, pThis->iBufPtrMax, (int) pThis->iBufPtrMax - strtIdx,
		pThis->pIOBuf+strtIdx, (int) (pThis->iBufPtrMax - pThis->iBufPtr), pThis->pIOBuf+pThis->iBufPtr);
}

/* logically "read" a character from a file. What actually happens is that
 * data is taken from the buffer. Only if the buffer is full, data is read
 * directly from file. In that case, a read is performed blockwise.
 * rgerhards, 2008-01-07
 * NOTE: needs to be enhanced to support sticking with a strm entry (if not
 * deleted).
 */
static rsRetVal strmReadChar(strm_t *pThis, uchar *pC)
{
	int padBytes = 0; /* in crypto mode, we may have some padding (non-data) bytes */
	DEFiRet;
	
	ASSERT(pThis != NULL);
	ASSERT(pC != NULL);

	/* DEV debug only: DBGOPRINT((obj_t*) pThis, "strmRead index %zd, max %zd\n", pThis->iBufPtr,
	pThis->iBufPtrMax); */
	if(pThis->iUngetC != -1) {	/* do we have an "unread" char that we need to provide? */
		*pC = pThis->iUngetC;
		++pThis->iCurrOffs; /* one more octet read */
		pThis->iUngetC = -1;
		ABORT_FINALIZE(RS_RET_OK);
	}
	
	/* do we need to obtain a new buffer? */
	if(pThis->iBufPtr >= pThis->iBufPtrMax) {
		CHKiRet(strmReadBuf(pThis, &padBytes));
	}
	pThis->iCurrOffs += padBytes;

	/* if we reach this point, we have data available in the buffer */

	*pC = pThis->pIOBuf[pThis->iBufPtr++];
	++pThis->iCurrOffs; /* one more octet read */

finalize_it:
	RETiRet;
}


/* unget a single character just like ungetc(). As with that call, there is only a single
 * character buffering capability.
 * rgerhards, 2008-01-07
 */
static rsRetVal strmUnreadChar(strm_t *pThis, uchar c)
{
	ASSERT(pThis != NULL);
	ASSERT(pThis->iUngetC == -1);
	pThis->iUngetC = c;
	--pThis->iCurrOffs; /* one less octet read - NOTE: this can cause problems if we got a file change
	and immediately do an unread and the file is on a buffer boundary and the stream is then persisted.
	With the queue, this can not happen as an Unread is only done on record begin, which is never split
	accross files. For other cases we accept the very remote risk. -- rgerhards, 2008-01-12 */

	return RS_RET_OK;
}

/* read a 'paragraph' from a strm file.
 * A paragraph may be terminated by a LF, by a LFLF, or by LF<not whitespace> depending on the option set.
 * The termination LF characters are read, but are
 * not returned in the buffer (it is discared). The caller is responsible for
 * destruction of the returned CStr object! -- dlang 2010-12-13
 *
 * Parameter mode controls legacy multi-line processing:
 * mode = 0 single line mode (equivalent to ReadLine)
 * mode = 1 LFLF mode (paragraph, blank line between entries)
 * mode = 2 LF <not whitespace> mode, a log line starts at the beginning of
 * a line, but following lines that are indented are part of the same log entry
 */
static rsRetVal
strmReadLine(strm_t *pThis, cstr_t **ppCStr, uint8_t mode, sbool bEscapeLF,
	uint32_t trimLineOverBytes, int64 *const strtOffs)
{
	uchar c;
	uchar finished;
	DEFiRet;

	ASSERT(pThis != NULL);
	ASSERT(ppCStr != NULL);

	CHKiRet(cstrConstruct(ppCStr));
	CHKiRet(strmReadChar(pThis, &c));

	/* append previous message to current message if necessary */
	if(pThis->prevLineSegment != NULL) {
		cstrFinalize(pThis->prevLineSegment);
		dbgprintf("readLine: have previous line segment: '%s'\n",
			rsCStrGetSzStrNoNULL(pThis->prevLineSegment));
		CHKiRet(cstrAppendCStr(*ppCStr, pThis->prevLineSegment));
		cstrDestruct(&pThis->prevLineSegment);
	}
	if(mode == 0) {
		while(c != '\n') {
			CHKiRet(cstrAppendChar(*ppCStr, c));
			CHKiRet(strmReadChar(pThis, &c));
		}
		if (trimLineOverBytes > 0 && (uint32_t) cstrLen(*ppCStr) > trimLineOverBytes) {
			/* Truncate long line at trimLineOverBytes position */
			dbgprintf("Truncate long line at %u, mode %d\n", trimLineOverBytes, mode);
			rsCStrTruncate(*ppCStr, cstrLen(*ppCStr) - trimLineOverBytes);
			cstrAppendChar(*ppCStr, '\n');
		}
		cstrFinalize(*ppCStr);
	} else if(mode == 1) {
		finished=0;
		while(finished == 0){
			if(c != '\n') {
				CHKiRet(cstrAppendChar(*ppCStr, c));
				CHKiRet(strmReadChar(pThis, &c));
				pThis->bPrevWasNL = 0;
			} else {
				if ((((*ppCStr)->iStrLen) > 0) ){
					if(pThis->bPrevWasNL) {
						rsCStrTruncate(*ppCStr, (bEscapeLF) ? 4 : 1);
						/* remove the prior newline */
						finished=1;
					} else {
						if(bEscapeLF) {
							CHKiRet(rsCStrAppendStrWithLen(*ppCStr, (uchar*)"#012",
							sizeof("#012")-1));
						} else {
							CHKiRet(cstrAppendChar(*ppCStr, c));
						}
						CHKiRet(strmReadChar(pThis, &c));
						pThis->bPrevWasNL = 1;
					}
				} else {
					finished=1;  /* this is a blank line, a \n with nothing since
							the last complete record */
				}
			}
		}
		cstrFinalize(*ppCStr);
		pThis->bPrevWasNL = 0;
	} else if(mode == 2) {
		/* indented follow-up lines */
		finished=0;
		while(finished == 0){
			if ((*ppCStr)->iStrLen == 0){
				if(c != '\n') {
				/* nothing in the buffer, and it's not a newline, add it to the buffer */
					CHKiRet(cstrAppendChar(*ppCStr, c));
					CHKiRet(strmReadChar(pThis, &c));
				} else {
					finished=1;  /* this is a blank line, a \n with nothing since the
							last complete record */
				}
			} else {
				if(pThis->bPrevWasNL) {
					if ((c == ' ') || (c == '\t')){
						CHKiRet(cstrAppendChar(*ppCStr, c));
						CHKiRet(strmReadChar(pThis, &c));
						pThis->bPrevWasNL = 0;
					} else {
						/* clean things up by putting the character we just read back into
						 * the input buffer and removing the LF character that is
						 * currently at the
						 * end of the output string */
						CHKiRet(strmUnreadChar(pThis, c));
						rsCStrTruncate(*ppCStr, (bEscapeLF) ? 4 : 1);
						finished=1;
					}
				} else { /* not the first character after a newline, add it to the buffer */
					if(c == '\n') {
						pThis->bPrevWasNL = 1;
						if(bEscapeLF) {
							CHKiRet(rsCStrAppendStrWithLen(*ppCStr, (uchar*)"#012",
							sizeof("#012")-1));
						} else {
							CHKiRet(cstrAppendChar(*ppCStr, c));
						}
					} else {
						CHKiRet(cstrAppendChar(*ppCStr, c));
					}
					CHKiRet(strmReadChar(pThis, &c));
				}
			}
		}
		if (trimLineOverBytes > 0 && (uint32_t) cstrLen(*ppCStr) > trimLineOverBytes) {
			/* Truncate long line at trimLineOverBytes position */
			dbgprintf("Truncate long line at %u, mode %d\n", trimLineOverBytes, mode);
			rsCStrTruncate(*ppCStr, cstrLen(*ppCStr) - trimLineOverBytes);
			cstrAppendChar(*ppCStr, '\n');
		}
		cstrFinalize(*ppCStr);
		pThis->bPrevWasNL = 0;
	}

finalize_it:
	if(iRet == RS_RET_OK) {
		if(strtOffs != NULL) {
			*strtOffs = pThis->strtOffs;
		}
		pThis->strtOffs = pThis->iCurrOffs; /* we are at begin of next line */
	} else {
		if(*ppCStr != NULL) {
			if(cstrLen(*ppCStr) > 0) {
			/* we may have an empty string in an unsuccesfull poll or after restart! */
				if(rsCStrConstructFromCStr(&pThis->prevLineSegment, *ppCStr) != RS_RET_OK) {
					/* we cannot do anything against this, but we can at least
					 * ensure we do not have any follow-on errors.
					 */
					 pThis->prevLineSegment = NULL;
				}
			}
			cstrDestruct(ppCStr);
		}
	}

	RETiRet;
}

/* check if the current multi line read is timed out
 * @return 0 - no timeout, something else - timeout
 */
int
strmReadMultiLine_isTimedOut(const strm_t *const __restrict__ pThis)
{
	/* note: order of evaluation is choosen so that the most inexpensive
	 * processing flow is used.
	 */
	DBGPRINTF("strmReadMultiline_isTimedOut: prevMsgSeg %p, readTimeout %d, "
		"lastRead %lld\n", pThis->prevMsgSegment, pThis->readTimeout,
		(long long) pThis->lastRead);
	return(   (pThis->readTimeout)
	       && (pThis->prevMsgSegment != NULL)
	       && (getTime(NULL) > pThis->lastRead + pThis->readTimeout) );
}

/* read a multi-line message from a strm file.
 * The multi-line message is terminated based on the user-provided
 * startRegex or endRegex (Posix ERE). For performance reasons, the regex
 * must already have been compiled by the user.
 * added 2015-05-12 rgerhards
 */
rsRetVal
strmReadMultiLine(strm_t *pThis, cstr_t **ppCStr, regex_t *start_preg, regex_t *end_preg, const sbool bEscapeLF,
	const sbool discardTruncatedMsg, const sbool msgDiscardingError, int64 *const strtOffs)
{
	uchar c;
	uchar finished = 0;
	cstr_t *thisLine = NULL;
	rsRetVal readCharRet;
	const time_t tCurr = pThis->readTimeout ? getTime(NULL) : 0;
	int maxMsgSize = glblGetMaxLine();
	DEFiRet;

	do {
		CHKiRet(strmReadChar(pThis, &c)); /* immediately exit on EOF */
		pThis->lastRead = tCurr;
		CHKiRet(cstrConstruct(&thisLine));
		/* append previous message to current message if necessary */
		if(pThis->prevLineSegment != NULL) {
			CHKiRet(cstrAppendCStr(thisLine, pThis->prevLineSegment));
			cstrDestruct(&pThis->prevLineSegment);
		}

		while(c != '\n') {
			CHKiRet(cstrAppendChar(thisLine, c));
			readCharRet = strmReadChar(pThis, &c);
			if(readCharRet == RS_RET_EOF) {/* end of file reached without \n? */
				CHKiRet(rsCStrConstructFromCStr(&pThis->prevLineSegment, thisLine));
			}
			CHKiRet(readCharRet);
		}
		cstrFinalize(thisLine);

		/* we have a line, now let's assemble the message */
		const int isStartMatch = start_preg ?
				!regexec(start_preg, (char*)rsCStrGetSzStrNoNULL(thisLine), 0, NULL, 0) :
				0;
		const int isEndMatch = end_preg ?
				!regexec(end_preg, (char*)rsCStrGetSzStrNoNULL(thisLine), 0, NULL, 0) :
				0;

		if(isStartMatch) {
			/* in this case, the *previous* message is complete and we are
			 * at the start of a new one.
			 */
			if(pThis->ignoringMsg == 0) {
				if(pThis->prevMsgSegment != NULL) {
					/* may be NULL in initial poll! */
					finished = 1;
					*ppCStr = pThis->prevMsgSegment;
				}
			}
			CHKiRet(rsCStrConstructFromCStr(&pThis->prevMsgSegment, thisLine));
			pThis->ignoringMsg = 0;
		} else {
			if(pThis->ignoringMsg == 0) {
				if(pThis->prevMsgSegment == NULL) {
					/* may be NULL in initial poll or after timeout! */
					CHKiRet(rsCStrConstructFromCStr(&pThis->prevMsgSegment, thisLine));
				} else {
					if(bEscapeLF) {
						rsCStrAppendStrWithLen(pThis->prevMsgSegment, (uchar*)"\\n", 2);
					} else {
						cstrAppendChar(pThis->prevMsgSegment, '\n');
					}


					int currLineLen = cstrLen(thisLine);
					if(currLineLen > 0) {
						int len;
						if((len = cstrLen(pThis->prevMsgSegment) + currLineLen) <
						maxMsgSize) {
							CHKiRet(cstrAppendCStr(pThis->prevMsgSegment, thisLine));
							/* we could do this faster, but for now keep it simple */
						} else {
							len = currLineLen-(len-maxMsgSize);
							for(int z=0; z<len; z++) {
								cstrAppendChar(pThis->prevMsgSegment,
								thisLine->pBuf[z]);
							}
							finished = 1;
							*ppCStr = pThis->prevMsgSegment;
							CHKiRet(rsCStrConstructFromszStr(&pThis->prevMsgSegment,
								thisLine->pBuf+len));
							if(discardTruncatedMsg == 1) {
								pThis->ignoringMsg = 1;
							}
							if(msgDiscardingError == 1) {
								if(discardTruncatedMsg == 1) {
									LogError(0, RS_RET_ERR,
									"imfile error: message received is "
									"larger than max msg size; "
									"rest of message will not be "
									"processed");
								} else {
									LogError(0, RS_RET_ERR,
									"imfile error: message received is "
									"larger than max msg size; message "
									"will be split and processed as "
									"another message");
								}
							}
						}
					}
				}
			}
		}
		if(isEndMatch) {
			/* in this case, the *current* message is complete and we are
			 * at the end of it.
			 */
			if(pThis->ignoringMsg == 0) {
				if(pThis->prevMsgSegment != NULL) {
					finished = 1;
					*ppCStr = pThis->prevMsgSegment;
					pThis->prevMsgSegment= NULL;
				}
			}
			pThis->ignoringMsg = 0;
		}
		cstrDestruct(&thisLine);
	} while(finished == 0);

finalize_it:
	*strtOffs = pThis->strtOffs;
	if(thisLine != NULL) {
		cstrDestruct(&thisLine);
	}
	if(iRet == RS_RET_OK) {
		pThis->strtOffs = pThis->iCurrOffs; /* we are at begin of next line */
		cstrFinalize(*ppCStr);
	} else {
		if(   pThis->readTimeout
		   && (pThis->prevMsgSegment != NULL)
		   && (tCurr > pThis->lastRead + pThis->readTimeout)) {
			if(rsCStrConstructFromCStr(ppCStr, pThis->prevMsgSegment) == RS_RET_OK) {
				cstrFinalize(*ppCStr);
				cstrDestruct(&pThis->prevMsgSegment);
				pThis->lastRead = tCurr;
				pThis->strtOffs = pThis->iCurrOffs; /* we are at begin of next line */
				dbgprintf("stream: generated msg based on timeout: %s\n",
					cstrGetSzStrNoNULL(*ppCStr));
				iRet = RS_RET_OK;
			}
		}
	}
	RETiRet;
}

/* Standard-Constructor for the strm object
 */
BEGINobjConstruct(strm) /* be sure to specify the object type also in END macro! */
	pThis->iCurrFNum = 1;
	pThis->fd = -1;
	pThis->fdDir = -1;
	pThis->iUngetC = -1;
	pThis->bVeryReliableZip = 0;
	pThis->sType = STREAMTYPE_FILE_SINGLE;
	pThis->sIOBufSize = glblGetIOBufSize();
	pThis->tOpenMode = 0600;
	pThis->pszSizeLimitCmd = NULL;
	pThis->prevLineSegment = NULL;
	pThis->prevMsgSegment = NULL;
	pThis->strtOffs = 0;
	pThis->ignoringMsg = 0;
	pThis->bPrevWasNL = 0;
	pThis->fileNotFoundError = 1;
	pThis->noRepeatedErrorOutput = 0;
	pThis->lastRead = getTime(NULL);
ENDobjConstruct(strm)


/* ConstructionFinalizer
 * rgerhards, 2008-01-09
 */
static rsRetVal strmConstructFinalize(strm_t *pThis)
{
	rsRetVal localRet;
	int i;
	DEFiRet;

	ASSERT(pThis != NULL);

	pThis->iBufPtrMax = 0; /* results in immediate read request */
	if(pThis->iZipLevel) { /* do we need a zip buf? */
		localRet = objUse(zlibw, LM_ZLIBW_FILENAME);
		if(localRet != RS_RET_OK) {
			pThis->iZipLevel = 0;
			DBGPRINTF("stream was requested with zip mode, but zlibw module unavailable (%d) - using "
				  "without zip\n", localRet);
		} else {
			/* we use the same size as the original buf, as we would like
			 * to make sure we can write out everything with a SINGLE api call!
			 * We add another 128 bytes to take care of the gzip header and "all eventualities".
			 */
			CHKmalloc(pThis->pZipBuf = (Bytef*) MALLOC(pThis->sIOBufSize + 128));
		}
	}

	/* if we are set to sync, we must obtain a file handle to the directory for fsync() purposes */
	if(pThis->bSync && !pThis->bIsTTY && pThis->pszDir != NULL) {
		pThis->fdDir = open((char*)pThis->pszDir, O_RDONLY | O_CLOEXEC | O_NOCTTY);
		if(pThis->fdDir == -1) {
			char errStr[1024];
			int err = errno;
			rs_strerror_r(err, errStr, sizeof(errStr));
			DBGPRINTF("error %d opening directory file for fsync() use - fsync for directory "
				"disabled: %s\n", errno, errStr);
		}
	}

	/* if we have a flush interval, we need to do async writes in any case */
	if(pThis->iFlushInterval != 0) {
		pThis->bAsyncWrite = 1;
	}

	DBGPRINTF("file stream %s params: flush interval %d, async write %d\n",
		  getFileDebugName(pThis),
		  pThis->iFlushInterval, pThis->bAsyncWrite);

	/* if we work asynchronously, we need a couple of synchronization objects */
	if(pThis->bAsyncWrite) {
		pthread_mutex_init(&pThis->mut, 0);
		pthread_cond_init(&pThis->notFull, 0);
		pthread_cond_init(&pThis->notEmpty, 0);
		pthread_cond_init(&pThis->isEmpty, 0);
		pThis->iCnt = pThis->iEnq = pThis->iDeq = 0;
		for(i = 0 ; i < STREAM_ASYNC_NUMBUFS ; ++i) {
			CHKmalloc(pThis->asyncBuf[i].pBuf = (uchar*) MALLOC(pThis->sIOBufSize));
		}
		pThis->pIOBuf = pThis->asyncBuf[0].pBuf;
		pThis->bStopWriter = 0;
		if(pthread_create(&pThis->writerThreadID,
			    	  &default_thread_attr,
				  asyncWriterThread, pThis) != 0)
			DBGPRINTF("ERROR: stream %p cold not create writer thread\n", pThis);
	} else {
		/* we work synchronously, so we need to alloc a fixed pIOBuf */
		CHKmalloc(pThis->pIOBuf = (uchar*) MALLOC(pThis->sIOBufSize));
	}

finalize_it:
	RETiRet;
}


/* stop the writer thread (we MUST be runnnig asynchronously when this method
 * is called!). Note that the mutex must be locked! -- rgerhards, 2009-07-06
 */
static void
stopWriter(strm_t *pThis)
{
	BEGINfunc
	pThis->bStopWriter = 1;
	pthread_cond_signal(&pThis->notEmpty);
	d_pthread_mutex_unlock(&pThis->mut);
	pthread_join(pThis->writerThreadID, NULL);
	ENDfunc
}


/* destructor for the strm object */
BEGINobjDestruct(strm) /* be sure to specify the object type also in END and CODESTART macros! */
	int i;
CODESTARTobjDestruct(strm)
	/* we need to stop the ZIP writer */
	if(pThis->bAsyncWrite)
		/* Note: mutex will be unlocked in stopWriter! */
		d_pthread_mutex_lock(&pThis->mut);

	/* strmClose() will handle read-only files as well as need to open
	 * files that have unwritten buffers. -- rgerhards, 2010-03-09
	 */
	strmCloseFile(pThis);

	if(pThis->bAsyncWrite) {
		stopWriter(pThis);
		pthread_mutex_destroy(&pThis->mut);
		pthread_cond_destroy(&pThis->notFull);
		pthread_cond_destroy(&pThis->notEmpty);
		pthread_cond_destroy(&pThis->isEmpty);
		for(i = 0 ; i < STREAM_ASYNC_NUMBUFS ; ++i) {
			free(pThis->asyncBuf[i].pBuf);
		}
	} else {
		free(pThis->pIOBuf);
	}

	/* Finally, we can free the resources.
	 * IMPORTANT: we MUST free this only AFTER the ansyncWriter has been stopped, else
	 * we get random errors...
	 */
	if(pThis->prevLineSegment)
		cstrDestruct(&pThis->prevLineSegment);
	if(pThis->prevMsgSegment)
		cstrDestruct(&pThis->prevMsgSegment);
	free(pThis->pszDir);
	free(pThis->pZipBuf);
	free(pThis->pszCurrFName);
	free(pThis->pszFName);
	free(pThis->pszSizeLimitCmd);
	pThis->bStopWriter = 2; /* RG: use as flag for destruction */
ENDobjDestruct(strm)


/* check if we need to open a new file (in output mode only).
 * The decision is based on file size AND record delimition state.
 * This method may also be called on a closed file, in which case
 * it immediately returns.
 */
static rsRetVal strmCheckNextOutputFile(strm_t *pThis)
{
	DEFiRet;

	if(pThis->fd == -1 || pThis->sType != STREAMTYPE_FILE_CIRCULAR)
		FINALIZE;

	/* wait for output to be empty, so that our counts are correct */
	strmWaitAsyncWriterDone(pThis);

	if(pThis->iCurrOffs >= pThis->iMaxFileSize) {
		DBGOPRINT((obj_t*) pThis, "max file size %ld reached for %d, now %ld - starting new file\n",
			  (long) pThis->iMaxFileSize, pThis->fd, (long) pThis->iCurrOffs);
		CHKiRet(strmNextFile(pThis));
	}

finalize_it:
	RETiRet;
}


/* try to recover a tty after a write error. This may have happend
 * due to vhangup(), and, if so, we can simply re-open it.
 */
#ifdef linux
#	define ERR_TTYHUP EIO
#else
#	define ERR_TTYHUP EBADF
#endif
static rsRetVal
tryTTYRecover(strm_t *pThis, int err)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strm);
#ifndef __FreeBSD__
	if(err == ERR_TTYHUP) {
#else
	/* Try to reopen our file descriptor even on errno 6, FreeBSD bug 200429
	 * Also try on errno 5, FreeBSD bug 211033
	 */
	if(err == ERR_TTYHUP || err == ENXIO || err == EIO) {
#endif /* __FreeBSD__ */
		close(pThis->fd);
		CHKiRet(doPhysOpen(pThis));
	}

finalize_it:
	RETiRet;
}
#undef ER_TTYHUP


/* issue write() api calls until either the buffer is completely
 * written or an error occured (it may happen that multiple writes
 * are required, what is perfectly legal. On exit, *pLenBuf contains
 * the number of bytes actually written.
 * rgerhards, 2009-06-08
 */
static rsRetVal
doWriteCall(strm_t *pThis, uchar *pBuf, size_t *pLenBuf)
{
	ssize_t lenBuf;
	ssize_t iTotalWritten;
	ssize_t iWritten;
	char *pWriteBuf;
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strm);
#ifdef __FreeBSD__
	sbool crnlNow = 0;
#endif /* __FreeBSD__ */

	lenBuf = *pLenBuf;
	pWriteBuf = (char*) pBuf;
	iTotalWritten = 0;
	do {
#ifdef __FreeBSD__
		if (pThis->bIsTTY && !pThis->iZipLevel && !pThis->cryprov) {
			char *pNl = NULL;
			if (crnlNow == 0) pNl = strchr(pWriteBuf, '\n');
			else crnlNow = 0;
			if (pNl == pWriteBuf) {
				iWritten = write(pThis->fd, "\r", 1);
				if (iWritten > 0) {
					crnlNow = 1;
					iWritten = 0;
				}
			} else iWritten = write(pThis->fd, pWriteBuf, pNl ? pNl - pWriteBuf : lenBuf);
		} else
#endif /* __FreeBSD__ */
		iWritten = write(pThis->fd, pWriteBuf, lenBuf);
		if(iWritten < 0) {
			const int err = errno;
			iWritten = 0; /* we have written NO bytes! */
			if(err != EINTR) {
				LogError(err, RS_RET_IO_ERROR, "file '%d' write error", pThis->fd);
			}
			if(err == EINTR) {
				/*NO ERROR, just continue */;
			} else if( !pThis->bIsTTY && ( err == ENOTCONN  || err == EIO )) {
				/* Failure for network file system, thus file needs to be closed and reopened. */
				close(pThis->fd);
				CHKiRet(doPhysOpen(pThis));
			} else {
				if(pThis->bIsTTY) {
					CHKiRet(tryTTYRecover(pThis, err));
				} else {
					ABORT_FINALIZE(RS_RET_IO_ERROR);
					/* Would it make sense to cover more error cases? So far, I
					 * do not see good reason to do so.
					 */
				}
			}
	 	}
		/* advance buffer to next write position */
		iTotalWritten += iWritten;
		lenBuf -= iWritten;
		pWriteBuf += iWritten;
	} while(lenBuf > 0);	/* Warning: do..while()! */

	DBGOPRINT((obj_t*) pThis, "file %d write wrote %d bytes\n", pThis->fd, (int) iWritten);

finalize_it:
	*pLenBuf = iTotalWritten;
	RETiRet;
}



/* write memory buffer to a stream object.
 */
static rsRetVal
doWriteInternal(strm_t *pThis, uchar *pBuf, const size_t lenBuf, const int bFlush)
{
	DEFiRet;

	DBGOPRINT((obj_t*) pThis, "file %d(%s) doWriteInternal: bFlush %d\n",
		pThis->fd, getFileDebugName(pThis), bFlush);

	if(pThis->iZipLevel) {
		CHKiRet(doZipWrite(pThis, pBuf, lenBuf, bFlush));
	} else {
		/* write without zipping */
		CHKiRet(strmPhysWrite(pThis, pBuf, lenBuf));
	}

finalize_it:
	RETiRet;
}


/* This function is called to "do" an async write call, what primarily means that
 * the data is handed over to the writer thread (which will then do the actual write
 * in parallel). Note that the stream mutex has already been locked by the
 * strmWrite...() calls. Also note that we always have only a single producer,
 * so we can simply serially assign the next free buffer to it and be sure that
 * the very some producer comes back in sequence to submit the then-filled buffers.
 * This also enables us to timout on partially written buffers. -- rgerhards, 2009-07-06
 */
static rsRetVal
doAsyncWriteInternal(strm_t *pThis, size_t lenBuf, const int bFlushZip)
{
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strm);

	DBGOPRINT((obj_t*) pThis, "file %d(%s) doAsyncWriteInternal at begin: "
		"iCnt %d, iEnq %d, bFlushZip %d\n",
		pThis->fd, getFileDebugName(pThis),
		pThis->iCnt, pThis->iEnq, bFlushZip);
	/* the -1 below is important, because we need one buffer for the main thread! */
	while(pThis->iCnt >= STREAM_ASYNC_NUMBUFS - 1)
		d_pthread_cond_wait(&pThis->notFull, &pThis->mut);

	pThis->asyncBuf[pThis->iEnq % STREAM_ASYNC_NUMBUFS].lenBuf = lenBuf;
	pThis->pIOBuf = pThis->asyncBuf[++pThis->iEnq % STREAM_ASYNC_NUMBUFS].pBuf;
	if(!pThis->bFlushNow) /* if we already need to flush, do not overwrite */
		pThis->bFlushNow = bFlushZip;

	pThis->bDoTimedWait = 0; /* everything written, no need to timeout partial buffer writes */
	if(++pThis->iCnt == 1) {
		pthread_cond_signal(&pThis->notEmpty);
		DBGOPRINT((obj_t*) pThis, "doAsyncWriteInternal signaled notEmpty\n");
	}
	DBGOPRINT((obj_t*) pThis, "file %d(%s) doAsyncWriteInternal at exit: "
		"iCnt %d, iEnq %d, bFlushZip %d\n",
		pThis->fd, getFileDebugName(pThis),
		pThis->iCnt, pThis->iEnq, bFlushZip);

	RETiRet;
}


/* schedule writing to the stream. Depending on our concurrency settings,
 * this either directly writes to the stream or schedules writing via
 * the background thread. -- rgerhards, 2009-07-07
 */
static rsRetVal
strmSchedWrite(strm_t *pThis, uchar *pBuf, size_t lenBuf, const int bFlushZip)
{
	DEFiRet;

	ASSERT(pThis != NULL);

	/* we need to reset the buffer pointer BEFORE calling the actual write
	 * function. Otherwise, in circular mode, the write function will
	 * potentially close the file, then close will flush and as the
	 * buffer pointer is nonzero, will re-call into this code here. In
	 * the end result, we than have a problem (and things are screwed
	 * up). So we reset the buffer pointer first, and all this can
	 * not happen. It is safe to do so, because that pointer is NOT
	 * used inside the write functions. -- rgerhads, 2010-03-10
	 */
	pThis->iBufPtr = 0; /* we are at the begin of a new buffer */
	if(pThis->bAsyncWrite) {
		CHKiRet(doAsyncWriteInternal(pThis, lenBuf, bFlushZip));
	} else {
		CHKiRet(doWriteInternal(pThis, pBuf, lenBuf, bFlushZip));
	}


finalize_it:
	RETiRet;
}



/* This is the writer thread for asynchronous mode.
 * -- rgerhards, 2009-07-06
 */
static void*
asyncWriterThread(void *pPtr)
{
	int iDeq;
	struct timespec t;
	sbool bTimedOut = 0;
	strm_t *pThis = (strm_t*) pPtr;
	int err;
	uchar thrdName[256] = "rs:";
	ISOBJ_TYPE_assert(pThis, strm);

	BEGINfunc
	ustrncpy(thrdName+3, pThis->pszFName, sizeof(thrdName)-4);
	dbgOutputTID((char*)thrdName);
#	if defined(HAVE_PRCTL) && defined(PR_SET_NAME)
	if(prctl(PR_SET_NAME, (char*)thrdName, 0, 0, 0) != 0) {
		DBGPRINTF("prctl failed, not setting thread name for '%s'\n", "stream writer");
	}
#	endif

	d_pthread_mutex_lock(&pThis->mut);
	while(1) { /* loop broken inside */
		while(pThis->iCnt == 0) {
			DBGOPRINT((obj_t*) pThis, "file %d(%s) asyncWriterThread new iteration, "
				  "iCnt %d, bTimedOut %d, iFlushInterval %d\n", pThis->fd,
				  getFileDebugName(pThis),
				  pThis->iCnt, bTimedOut, pThis->iFlushInterval);
			if(pThis->bStopWriter) {
				pthread_cond_broadcast(&pThis->isEmpty);
				d_pthread_mutex_unlock(&pThis->mut);
				goto finalize_it; /* break main loop */
			}
			if(bTimedOut && pThis->iBufPtr > 0) {
				/* if we timed out, we need to flush pending data */
				strmFlushInternal(pThis, 1);
				bTimedOut = 0;
				continue;
			}
			bTimedOut = 0;
			if(pThis->bDoTimedWait) {
				timeoutComp(&t, pThis->iFlushInterval * 1000); /* 1000 *millisconds* */
				if((err = pthread_cond_timedwait(&pThis->notEmpty, &pThis->mut, &t)) != 0) {
					DBGOPRINT((obj_t*) pThis, "file %d(%s) asyncWriterThread timed out\n",
						  pThis->fd, getFileDebugName(pThis));
					bTimedOut = 1; /* simulate in any case */
					if(err != ETIMEDOUT) {
						char errStr[1024];
						rs_strerror_r(err, errStr, sizeof(errStr));
						DBGPRINTF("stream async writer timeout with error (%d): %s - "
							"ignoring\n", err, errStr);
					}
				}
			} else {
				d_pthread_cond_wait(&pThis->notEmpty, &pThis->mut);
			}
		}

		DBGOPRINT((obj_t*) pThis, "file %d(%s) asyncWriterThread awoken, "
			  "iCnt %d, bTimedOut %d\n", pThis->fd, getFileDebugName(pThis),
			  pThis->iCnt, bTimedOut);
		bTimedOut = 0; /* we may have timed out, but there *is* work to do... */

		iDeq = pThis->iDeq++ % STREAM_ASYNC_NUMBUFS;
		const int bFlush = (pThis->bFlushNow || bTimedOut) ? 1 : 0;
		pThis->bFlushNow = 0;

		/* now we can do the actual write in parallel */
		d_pthread_mutex_unlock(&pThis->mut);
		doWriteInternal(pThis, pThis->asyncBuf[iDeq].pBuf, pThis->asyncBuf[iDeq].lenBuf, bFlush);
		// TODO: error check????? 2009-07-06
		d_pthread_mutex_lock(&pThis->mut);

		--pThis->iCnt;
		if(pThis->iCnt < STREAM_ASYNC_NUMBUFS) {
			pthread_cond_signal(&pThis->notFull);
			if(pThis->iCnt == 0)
				pthread_cond_broadcast(&pThis->isEmpty);
		}
	}
	/* Not reached */

finalize_it:
	ENDfunc
	return NULL; /* to keep pthreads happy */
}


/* sync the file to disk, so that any unwritten data is persisted. This
 * also syncs the directory and thus makes sure that the file survives
 * fatal failure. Note that we do NOT return an error status if the
 * sync fails. Doing so would probably cause more trouble than it
 * is worth (read: data loss may occur where we otherwise might not
 * have it). -- rgerhards, 2009-06-08
 */
#undef SYNCCALL
#if defined(HAVE_FDATASYNC) && !defined(__APPLE__)
#	define SYNCCALL(x) fdatasync(x)
#else
#	define SYNCCALL(x) fsync(x)
#endif
static rsRetVal
syncFile(strm_t *pThis)
{
	int ret;
	DEFiRet;

	if(pThis->bIsTTY)
		FINALIZE; /* TTYs can not be synced */

	DBGPRINTF("syncing file %d\n", pThis->fd);
	ret = SYNCCALL(pThis->fd);
	if(ret != 0) {
		char errStr[1024];
		int err = errno;
		rs_strerror_r(err, errStr, sizeof(errStr));
		DBGPRINTF("sync failed for file %d with error (%d): %s - ignoring\n",
			   pThis->fd, err, errStr);
	}
	
	if(pThis->fdDir != -1) {
		if(fsync(pThis->fdDir) != 0)
			DBGPRINTF("stream/syncFile: fsync returned error, ignoring\n");
	}

finalize_it:
	RETiRet;
}
#undef SYNCCALL

/* physically write to the output file. the provided data is ready for
 * writing (e.g. zipped if we are requested to do that).
 * Note that if the write() API fails, we do not reset any pointers, but return
 * an error code. That means we may redo work in the next iteration.
 * rgerhards, 2009-06-04
 */
static rsRetVal
strmPhysWrite(strm_t *pThis, uchar *pBuf, size_t lenBuf)
{
	size_t iWritten;
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, strm);

	DBGPRINTF("strmPhysWrite, stream %p, len %u\n", pThis, (unsigned)lenBuf);
	if(pThis->fd == -1)
		CHKiRet(strmOpenFile(pThis));

	/* here we place our crypto interface */
	if(pThis->cryprov != NULL) {
		pThis->cryprov->Encrypt(pThis->cryprovFileData, pBuf, &lenBuf);
	}
	/* end crypto */

	iWritten = lenBuf;
	CHKiRet(doWriteCall(pThis, pBuf, &iWritten));

	pThis->iCurrOffs += iWritten;
	/* update user counter, if provided */
	if(pThis->pUsrWCntr != NULL)
		*pThis->pUsrWCntr += iWritten;

	if(pThis->bSync) {
		CHKiRet(syncFile(pThis));
	}

	if(pThis->sType == STREAMTYPE_FILE_CIRCULAR) {
		CHKiRet(strmCheckNextOutputFile(pThis));
	} else if(pThis->iSizeLimit != 0) {
		CHKiRet(doSizeLimitProcessing(pThis));
	}

finalize_it:
	RETiRet;
}


/* write the output buffer in zip mode
 * This means we compress it first and then do a physical write.
 * Note that we always do a full deflateInit ... deflate ... deflateEnd
 * sequence. While this is not optimal, we need to do it because we need
 * to ensure that the file is readable even when we are aborted. Doing the
 * full sequence brings us as far towards this goal as possible (and not
 * doing it would be a total failure). It may be worth considering to
 * add a config switch so that the user can decide the risk he is ready
 * to take, but so far this is not yet implemented (not even requested ;)).
 * rgerhards, 2009-06-04
 */
static rsRetVal
doZipWrite(strm_t *pThis, uchar *pBuf, size_t lenBuf, const int bFlush)
{
	int zRet;	/* zlib return state */
	DEFiRet;
	unsigned outavail = 0;
	assert(pThis != NULL);
	assert(pBuf != NULL);

	if(!pThis->bzInitDone) {
		/* allocate deflate state */
		pThis->zstrm.zalloc = Z_NULL;
		pThis->zstrm.zfree = Z_NULL;
		pThis->zstrm.opaque = Z_NULL;
		/* see note in file header for the params we use with deflateInit2() */
		zRet = zlibw.DeflateInit2(&pThis->zstrm, pThis->iZipLevel, Z_DEFLATED, 31, 9, Z_DEFAULT_STRATEGY);
		if(zRet != Z_OK) {
			LogError(0, RS_RET_ZLIB_ERR, "error %d returned from zlib/deflateInit2()", zRet);
			ABORT_FINALIZE(RS_RET_ZLIB_ERR);
		}
		pThis->bzInitDone = RSTRUE;
	}

	/* now doing the compression */
	pThis->zstrm.next_in = (Bytef*) pBuf;
	pThis->zstrm.avail_in = lenBuf;
	/* run deflate() on buffer until everything has been compressed */
	do {
		DBGPRINTF("in deflate() loop, avail_in %d, total_in %ld, bFlush %d\n",
			pThis->zstrm.avail_in, pThis->zstrm.total_in, bFlush);
		pThis->zstrm.avail_out = pThis->sIOBufSize;
		pThis->zstrm.next_out = pThis->pZipBuf;
		zRet = zlibw.Deflate(&pThis->zstrm, bFlush ? Z_SYNC_FLUSH : Z_NO_FLUSH);    /* no bad return value */
		DBGPRINTF("after deflate, ret %d, avail_out %d, to write %d\n",
			zRet, pThis->zstrm.avail_out, outavail);
		if(zRet != Z_OK) {
			LogError(0, RS_RET_ZLIB_ERR, "error %d returned from zlib/Deflate()", zRet);
			ABORT_FINALIZE(RS_RET_ZLIB_ERR);
		}
		outavail = pThis->sIOBufSize - pThis->zstrm.avail_out;
		if(outavail != 0) {
			CHKiRet(strmPhysWrite(pThis, (uchar*)pThis->pZipBuf, outavail));
		}
	} while (pThis->zstrm.avail_out == 0);

finalize_it:
	if(pThis->bzInitDone && pThis->bVeryReliableZip) {
		doZipFinish(pThis);
	}
	RETiRet;
}



/* finish zlib buffer, to be called before closing the ZIP file (if
 * running in stream mode).
 */
static rsRetVal
doZipFinish(strm_t *pThis)
{
	int zRet;	/* zlib return state */
	DEFiRet;
	unsigned outavail;
	assert(pThis != NULL);

	if(!pThis->bzInitDone)
		goto done;

	pThis->zstrm.avail_in = 0;
	/* run deflate() on buffer until everything has been compressed */
	do {
		DBGPRINTF("in deflate() loop, avail_in %d, total_in %ld\n", pThis->zstrm.avail_in,
			pThis->zstrm.total_in);
		pThis->zstrm.avail_out = pThis->sIOBufSize;
		pThis->zstrm.next_out = pThis->pZipBuf;
		zRet = zlibw.Deflate(&pThis->zstrm, Z_FINISH);    /* no bad return value */
		DBGPRINTF("after deflate, ret %d, avail_out %d\n", zRet, pThis->zstrm.avail_out);
		outavail = pThis->sIOBufSize - pThis->zstrm.avail_out;
		if(outavail != 0) {
			CHKiRet(strmPhysWrite(pThis, (uchar*)pThis->pZipBuf, outavail));
		}
	} while (pThis->zstrm.avail_out == 0);

finalize_it:
	zRet = zlibw.DeflateEnd(&pThis->zstrm);
	if(zRet != Z_OK) {
		LogError(0, RS_RET_ZLIB_ERR, "error %d returned from zlib/deflateEnd()", zRet);
	}

	pThis->bzInitDone = 0;
done:	RETiRet;
}

/* flush stream output buffer to persistent storage. This can be called at any time
 * and is automatically called when the output buffer is full.
 * rgerhards, 2008-01-10
 */
static rsRetVal
strmFlushInternal(strm_t *pThis, int bFlushZip)
{
	DEFiRet;

	ASSERT(pThis != NULL);
	DBGOPRINT((obj_t*) pThis, "strmFlushinternal: file %d(%s) flush, buflen %ld%s\n", pThis->fd,
		  getFileDebugName(pThis),
		  (long) pThis->iBufPtr, (pThis->iBufPtr == 0) ? " (no need to flush)" : "");

	if(pThis->tOperationsMode != STREAMMODE_READ && pThis->iBufPtr > 0) {
		iRet = strmSchedWrite(pThis, pThis->pIOBuf, pThis->iBufPtr, bFlushZip);
	}

	RETiRet;
}


/* flush stream output buffer to persistent storage. This can be called at any time
 * and is automatically called when the output buffer is full. This function is for
 * use by EXTERNAL callers. Do NOT use it internally. It locks the async writer
 * mutex if ther is need to do so.
 * rgerhards, 2010-03-18
 */
static rsRetVal
strmFlush(strm_t *pThis)
{
	DEFiRet;

	ASSERT(pThis != NULL);

	if(pThis->bAsyncWrite)
		d_pthread_mutex_lock(&pThis->mut);
	CHKiRet(strmFlushInternal(pThis, 1));

finalize_it:
	if(pThis->bAsyncWrite)
		d_pthread_mutex_unlock(&pThis->mut);

	RETiRet;
}


/* seek a stream to a specific location. Pending writes are flushed, read data
 * is invalidated.
 * rgerhards, 2008-01-12
 */
static rsRetVal strmSeek(strm_t *pThis, off64_t offs)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strm);

	if(pThis->fd == -1) {
		CHKiRet(strmOpenFile(pThis));
	} else {
		CHKiRet(strmFlushInternal(pThis, 0));
	}
	long long i;
	DBGOPRINT((obj_t*) pThis, "file %d seek, pos %llu\n", pThis->fd, (long long unsigned) offs);
	i = lseek64(pThis->fd, offs, SEEK_SET);
	if(i != offs) {
		DBGPRINTF("strmSeek: error %lld seeking to offset %lld\n", i, (long long) offs);
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}
	pThis->strtOffs = pThis->iCurrOffs = offs; /* we are now at *this* offset */
	pThis->iBufPtr = 0; /* buffer invalidated */

finalize_it:
	RETiRet;
}

/* multi-file seek, seeks to file number & offset within file. This
 * is a support function for the queue, in circular mode. DO NOT USE
 * IT FOR OTHER NEEDS - it may not work as expected. It will
 * seek to the new position and delete interim files, as it skips them.
 * Note: this code can be removed when the queue gets a new disk store
 * handler (if and when it does ;)).
 * The output parameter bytesDel receives the number of bytes that have
 * been deleted (if a file is deleted) or 0 if nothing was deleted.
 * rgerhards, 2012-11-07
 */
rsRetVal
strmMultiFileSeek(strm_t *pThis, unsigned int FNum, off64_t offs, off64_t *bytesDel)
{
	struct stat statBuf;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strm);

	if(FNum == 0 && offs == 0) { /* happens during queue init */
		*bytesDel = 0;
		FINALIZE;
	}

	if(pThis->iCurrFNum != FNum) {
		/* Note: we assume that no more than one file is skipped - an
		 * assumption that is being used also by the whole rest of the
		 * code and most notably the queue subsystem.
		 */
		CHKiRet(genFileName(&pThis->pszCurrFName, pThis->pszDir, pThis->lenDir,
				    pThis->pszFName, pThis->lenFName, pThis->iCurrFNum,
				    pThis->iFileNumDigits));
		if(stat((char*)pThis->pszCurrFName, &statBuf) != 0) {
			LogError(errno, RS_RET_IO_ERROR, "unexpected error doing a stat() "
				"on file %s - further malfunctions may happen",
				pThis->pszCurrFName);
			ABORT_FINALIZE(RS_RET_IO_ERROR);
		}
		*bytesDel = statBuf.st_size;
		DBGPRINTF("strmMultiFileSeek: detected new filenum, was %u, new %u, "
			  "deleting '%s' (%lld bytes)\n", pThis->iCurrFNum, FNum,
			  pThis->pszCurrFName, (long long) *bytesDel);
		unlink((char*)pThis->pszCurrFName);
		if(pThis->cryprov != NULL)
			pThis->cryprov->DeleteStateFiles(pThis->pszCurrFName);
		free(pThis->pszCurrFName);
		pThis->pszCurrFName = NULL;
		pThis->iCurrFNum = FNum;
	} else {
		*bytesDel = 0;
	}
	pThis->strtOffs = pThis->iCurrOffs = offs;

finalize_it:
	RETiRet;
}


/* seek to current offset. This is primarily a helper to readjust the OS file
 * pointer after a strm object has been deserialized.
 */
static rsRetVal strmSeekCurrOffs(strm_t *pThis)
{
	off64_t targetOffs;
	uchar c;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strm);

	if(pThis->cryprov == NULL || pThis->tOperationsMode != STREAMMODE_READ) {
		iRet = strmSeek(pThis, pThis->iCurrOffs);
		FINALIZE;
	}

	/* As the cryprov may use CBC or similiar things, we need to read skip data */
	targetOffs = pThis->iCurrOffs;
	pThis->strtOffs = pThis->iCurrOffs = 0;
	DBGOPRINT((obj_t*) pThis, "encrypted, doing skip read of %lld bytes\n",
		(long long) targetOffs);
	while(targetOffs != pThis->iCurrOffs) {
		CHKiRet(strmReadChar(pThis, &c));
	}
finalize_it:
	RETiRet;
}


/* write a *single* character to a stream object -- rgerhards, 2008-01-10
 */
static rsRetVal strmWriteChar(strm_t *__restrict__ const pThis, const uchar c)
{
	DEFiRet;

	ASSERT(pThis != NULL);

	if(pThis->bAsyncWrite)
		d_pthread_mutex_lock(&pThis->mut);

	if(pThis->bDisabled)
		ABORT_FINALIZE(RS_RET_STREAM_DISABLED);

	/* if the buffer is full, we need to flush before we can write */
	if(pThis->iBufPtr == pThis->sIOBufSize) {
		CHKiRet(strmFlushInternal(pThis, 0));
	}
	/* we now always have space for one character, so we simply copy it */
	*(pThis->pIOBuf + pThis->iBufPtr) = c;
	pThis->iBufPtr++;

finalize_it:
	if(pThis->bAsyncWrite)
		d_pthread_mutex_unlock(&pThis->mut);

	RETiRet;
}


/* write an integer value (actually a long) to a stream object
 * Note that we do not need to lock the mutex here, because we call
 * strmWrite(), which does the lock (aka: we must not lock it, else we
 * would run into a recursive lock, resulting in a deadlock!)
 */
static rsRetVal strmWriteLong(strm_t *__restrict__ const pThis, const long i)
{
	DEFiRet;
	uchar szBuf[32];

	ASSERT(pThis != NULL);

	CHKiRet(srUtilItoA((char*)szBuf, sizeof(szBuf), i));
	CHKiRet(strmWrite(pThis, szBuf, strlen((char*)szBuf)));

finalize_it:
	RETiRet;
}


/* write memory buffer to a stream object.
 * process the data in chunks and copy it over to our buffer. The caller-provided data
 * may theoritically be larger than our buffer. In that case, we do multiple copies. One
 * may argue if it were more efficient to write out the caller-provided buffer in that case
 * and earlier versions of rsyslog did this. However, this introduces a lot of complexity
 * inside the buffered writer and potential performance bottlenecks when trying to solve
 * it. Now keep in mind that we actually do (almost?) never have a case where the
 * caller-provided buffer is larger than our one. So instead of optimizing a case
 * which normally does not exist, we expect some degradation in its case but make us
 * perform better in the regular cases. -- rgerhards, 2009-07-07
 * Note: the pThis->iBufPtr == pThis->sIOBufSize logic below looks a bit like an
 * on-off error. In fact, it is not, because iBufPtr always points to the next
 * *free* byte in the buffer. So if it is sIOBufSize - 1, there actually is one
 * free byte left. This came up during a code walkthrough and was considered
 * worth nothing. -- rgerhards, 2010-03-10
 */
static rsRetVal
strmWrite(strm_t *__restrict__ const pThis, const uchar *__restrict__ const pBuf, size_t lenBuf)
{
	DEFiRet;
	size_t iWrite;
	size_t iOffset;

	ASSERT(pThis != NULL);
	ASSERT(pBuf != NULL);

/* DEV DEBUG ONLY DBGPRINTF("strmWrite(%p[%s], '%65.65s', %ld);,
disabled %d, sizelim %ld, size %lld\n", pThis, pThis->pszCurrFName, pBuf,(long) lenBuf,
pThis->bDisabled, (long) pThis->iSizeLimit, (long long) pThis->iCurrOffs); */
	if(pThis->bDisabled)
		ABORT_FINALIZE(RS_RET_STREAM_DISABLED);

	if(pThis->bAsyncWrite)
		d_pthread_mutex_lock(&pThis->mut);

	iOffset = 0;
	do {
		if(pThis->iBufPtr == pThis->sIOBufSize) {
			CHKiRet(strmFlushInternal(pThis, 0)); /* get a new buffer for rest of data */
		}
		iWrite = pThis->sIOBufSize - pThis->iBufPtr; /* this fits in current buf */
		if(iWrite > lenBuf)
			iWrite = lenBuf;
		memcpy(pThis->pIOBuf + pThis->iBufPtr, pBuf + iOffset, iWrite);
		pThis->iBufPtr += iWrite;
		iOffset += iWrite;
		lenBuf -= iWrite;
	} while(lenBuf > 0);

	/* now check if the buffer right at the end of the write is full and, if so,
	 * write it. This seems more natural than waiting (hours?) for the next message...
	 */
	if(pThis->iBufPtr == pThis->sIOBufSize) {
		CHKiRet(strmFlushInternal(pThis, 0)); /* get a new buffer for rest of data */
	}

finalize_it:
	if(pThis->bAsyncWrite) {
		if(pThis->bDoTimedWait == 0) {
			/* we potentially have a partial buffer, so re-activate the
			 * writer thread that it can set and pick up timeouts.
			 */
			pThis->bDoTimedWait = 1;
			pthread_cond_signal(&pThis->notEmpty);
		}
		d_pthread_mutex_unlock(&pThis->mut);
	}

	RETiRet;
}


/* property set methods */
/* simple ones first */
DEFpropSetMeth(strm, iMaxFileSize, int64)
DEFpropSetMeth(strm, iFileNumDigits, int)
DEFpropSetMeth(strm, tOperationsMode, int)
DEFpropSetMeth(strm, tOpenMode, mode_t)
DEFpropSetMeth(strm, sType, strmType_t)
DEFpropSetMeth(strm, iZipLevel, int)
DEFpropSetMeth(strm, bVeryReliableZip, int)
DEFpropSetMeth(strm, bSync, int)
DEFpropSetMeth(strm, bReopenOnTruncate, int)
DEFpropSetMeth(strm, sIOBufSize, size_t)
DEFpropSetMeth(strm, iSizeLimit, off_t)
DEFpropSetMeth(strm, iFlushInterval, int)
DEFpropSetMeth(strm, pszSizeLimitCmd, uchar*)
DEFpropSetMeth(strm, cryprov, cryprov_if_t*)
DEFpropSetMeth(strm, cryprovData, void*)

/* sets timeout in seconds */
void
strmSetReadTimeout(strm_t *const __restrict__ pThis, const int val)
{
	pThis->readTimeout = val;
}

static rsRetVal strmSetbDeleteOnClose(strm_t *pThis, int val)
{
	pThis->bDeleteOnClose = val;
	if(pThis->cryprov != NULL) {
		pThis->cryprov->SetDeleteOnClose(pThis->cryprovFileData, pThis->bDeleteOnClose);
	}
	return RS_RET_OK;
}

static rsRetVal strmSetiMaxFiles(strm_t *pThis, int iNewVal)
{
	pThis->iMaxFiles = iNewVal;
	pThis->iFileNumDigits = getNumberDigits(iNewVal);
	return RS_RET_OK;
}

static rsRetVal strmSetFileNotFoundError(strm_t *pThis, int pFileNotFoundError)
{
	pThis->fileNotFoundError = pFileNotFoundError;
	return RS_RET_OK;
}


/* set the stream's file prefix
 * The passed-in string is duplicated. So if the caller does not need
 * it any longer, it must free it.
 * rgerhards, 2008-01-09
 */
static rsRetVal
strmSetFName(strm_t *pThis, uchar *pszName, size_t iLenName)
{
	DEFiRet;

	ASSERT(pThis != NULL);
	ASSERT(pszName != NULL);
	
	if(iLenName < 1)
		ABORT_FINALIZE(RS_RET_FILE_PREFIX_MISSING);

	if(pThis->pszFName != NULL)
		free(pThis->pszFName);

	if((pThis->pszFName = MALLOC(iLenName + 1)) == NULL)
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);

	memcpy(pThis->pszFName, pszName, iLenName + 1); /* always think about the \0! */
	pThis->lenFName = iLenName;

finalize_it:
	RETiRet;
}


/* set the stream's directory
 * The passed-in string is duplicated. So if the caller does not need
 * it any longer, it must free it.
 * rgerhards, 2008-01-09
 */
static rsRetVal
strmSetDir(strm_t *pThis, uchar *pszDir, size_t iLenDir)
{
	DEFiRet;

	ASSERT(pThis != NULL);
	ASSERT(pszDir != NULL);
	
	if(iLenDir < 1)
		ABORT_FINALIZE(RS_RET_FILE_PREFIX_MISSING);

	CHKmalloc(pThis->pszDir = MALLOC(iLenDir + 1));

	memcpy(pThis->pszDir, pszDir, iLenDir + 1); /* always think about the \0! */
	pThis->lenDir = iLenDir;

finalize_it:
	RETiRet;
}


/* support for data records
 * The stream class is able to write to multiple files. However, there are
 * situation (actually quite common), where a single data record should not
 * be split across files. This may be problematic if multiple stream write
 * calls are used to create the record. To support that, we provide the
 * bInRecord status variable. If it is set, no file spliting occurs. Once
 * it is set to 0, a check is done if a split is necessary and it then
 * happens. For a record-oriented caller, the proper sequence is:
 *
 * strmRecordBegin()
 * strmWrite...()
 * strmRecordEnd()
 *
 * Please note that records do not affect the writing of output buffers. They
 * are always written when full. The only thing affected is circular files
 * creation. So it is safe to write large records.
 *
 * IMPORTANT: RecordBegin() can not be nested! It is a programming error
 * if RecordBegin() is called while already in a record!
 *
 * rgerhards, 2008-01-10
 */
static rsRetVal strmRecordBegin(strm_t *pThis)
{
	ASSERT(pThis != NULL);
	ASSERT(pThis->bInRecord == 0);
	pThis->bInRecord = 1;
	return RS_RET_OK;
}

static rsRetVal strmRecordEnd(strm_t *pThis)
{
	DEFiRet;
	ASSERT(pThis != NULL);
	ASSERT(pThis->bInRecord == 1);

	pThis->bInRecord = 0;
	iRet = strmCheckNextOutputFile(pThis); /* check if we need to switch files */

	RETiRet;
}
/* end stream record support functions */


/* This method serializes a stream object. That means the whole
 * object is modified into text form. That text form is suitable for
 * later reconstruction of the object.
 * The most common use case for this method is the creation of an
 * on-disk representation of the message object.
 * We do not serialize the dynamic properties.
 * rgerhards, 2008-01-10
 */
static rsRetVal strmSerialize(strm_t *pThis, strm_t *pStrm)
{
	DEFiRet;
	int i;
	int64 l;

	ISOBJ_TYPE_assert(pThis, strm);
	ISOBJ_TYPE_assert(pStrm, strm);

	strmFlushInternal(pThis, 0);
	CHKiRet(obj.BeginSerialize(pStrm, (obj_t*) pThis));

	objSerializeSCALAR(pStrm, iCurrFNum, INT); /* implicit cast is OK for persistance */
	objSerializePTR(pStrm, pszFName, PSZ);
	objSerializeSCALAR(pStrm, iMaxFiles, INT);
	objSerializeSCALAR(pStrm, bDeleteOnClose, INT);

	i = pThis->sType;
	objSerializeSCALAR_VAR(pStrm, sType, INT, i);

	i = pThis->tOperationsMode;
	objSerializeSCALAR_VAR(pStrm, tOperationsMode, INT, i);

	i = pThis->tOpenMode;
	objSerializeSCALAR_VAR(pStrm, tOpenMode, INT, i);

	l = pThis->iCurrOffs;
	objSerializeSCALAR_VAR(pStrm, iCurrOffs, INT64, l);

	l = pThis->inode;
	objSerializeSCALAR_VAR(pStrm, inode, INT64, l);

	l = pThis->strtOffs;
	objSerializeSCALAR_VAR(pStrm, strtOffs, INT64, l);

	dbgprintf("strmSerialize: pThis->prevLineSegment %p\n", pThis->prevLineSegment);
	if(pThis->prevLineSegment != NULL) {
		cstrFinalize(pThis->prevLineSegment);
		objSerializePTR(pStrm, prevLineSegment, CSTR);
	}

	if(pThis->prevMsgSegment != NULL) {
		cstrFinalize(pThis->prevMsgSegment);
		objSerializePTR(pStrm, prevMsgSegment, CSTR);
	}

	i = pThis->bPrevWasNL;
	objSerializeSCALAR_VAR(pStrm, bPrevWasNL, INT, i);

	CHKiRet(obj.EndSerialize(pStrm));

finalize_it:
	RETiRet;
}


/* duplicate a stream object excluding dynamic properties. This function is
 * primarily meant to provide a duplicate that later on can be used to access
 * the data. This is needed, for example, for a restart of the disk queue.
 * Note that ConstructFinalize() is NOT called. So our caller may change some
 * properties before finalizing things.
 * rgerhards, 2009-05-26
 */
static rsRetVal
strmDup(strm_t *const pThis, strm_t **ppNew)
{
	strm_t *pNew = NULL;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strm);
	assert(ppNew != NULL);

	CHKiRet(strmConstruct(&pNew));
	pNew->sType = pThis->sType;
	pNew->iCurrFNum = pThis->iCurrFNum;
	CHKmalloc(pNew->pszFName = ustrdup(pThis->pszFName));
	pNew->lenFName = pThis->lenFName;
	CHKmalloc(pNew->pszDir = ustrdup(pThis->pszDir));
	pNew->lenDir = pThis->lenDir;
	pNew->tOperationsMode = pThis->tOperationsMode;
	pNew->tOpenMode = pThis->tOpenMode;
	pNew->iMaxFileSize = pThis->iMaxFileSize;
	pNew->iMaxFiles = pThis->iMaxFiles;
	pNew->iFileNumDigits = pThis->iFileNumDigits;
	pNew->bDeleteOnClose = pThis->bDeleteOnClose;
	pNew->iCurrOffs = pThis->iCurrOffs;
	
	*ppNew = pNew;
	pNew = NULL;

finalize_it:
	if(pNew != NULL)
		strmDestruct(&pNew);

	RETiRet;
}

/* set a user write-counter. This counter is initialized to zero and
 * receives the number of bytes written. It is accurate only after a
 * flush(). This hook is provided as a means to control disk size usage.
 * The pointer must be valid at all times (so if it is on the stack, be sure
 * to remove it when you exit the function). Pointers are removed by
 * calling strmSetWCntr() with a NULL param. Only one pointer is settable,
 * any new set overwrites the previous one.
 * rgerhards, 2008-02-27
 */
static rsRetVal
strmSetWCntr(strm_t *pThis, number_t *pWCnt)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strm);

	if(pWCnt != NULL)
		*pWCnt = 0;
	pThis->pUsrWCntr = pWCnt;

	RETiRet;
}


#include "stringbuf.h"

/* This function can be used as a generic way to set properties.
 * rgerhards, 2008-01-11
 */
#define isProp(name) !rsCStrSzStrCmp(pProp->pcsName, UCHAR_CONSTANT(name), sizeof(name) - 1)
static rsRetVal strmSetProperty(strm_t *pThis, var_t *pProp)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strm);
	ASSERT(pProp != NULL);

	if(isProp("sType")) {
		CHKiRet(strmSetsType(pThis, (strmType_t) pProp->val.num));
	} else if(isProp("iCurrFNum")) {
		pThis->iCurrFNum = (unsigned) pProp->val.num;
	} else if(isProp("pszFName")) {
		CHKiRet(strmSetFName(pThis, rsCStrGetSzStrNoNULL(pProp->val.pStr), rsCStrLen(pProp->val.pStr)));
	} else if(isProp("tOperationsMode")) {
		CHKiRet(strmSettOperationsMode(pThis, pProp->val.num));
	} else if(isProp("tOpenMode")) {
		CHKiRet(strmSettOpenMode(pThis, pProp->val.num));
	} else if(isProp("iCurrOffs")) {
		pThis->iCurrOffs = pProp->val.num;
	} else if(isProp("inode")) {
		pThis->inode = (ino_t) pProp->val.num;
	} else if(isProp("strtOffs")) {
		pThis->strtOffs = pProp->val.num;
	} else if(isProp("iMaxFileSize")) {
		CHKiRet(strmSetiMaxFileSize(pThis, pProp->val.num));
	} else if(isProp("fileNotFoundError")) {
		CHKiRet(strmSetFileNotFoundError(pThis, pProp->val.num));
	} else if(isProp("iMaxFiles")) {
		CHKiRet(strmSetiMaxFiles(pThis, pProp->val.num));
	} else if(isProp("iFileNumDigits")) {
		CHKiRet(strmSetiFileNumDigits(pThis, pProp->val.num));
	} else if(isProp("bDeleteOnClose")) {
		CHKiRet(strmSetbDeleteOnClose(pThis, pProp->val.num));
	} else if(isProp("prevLineSegment")) {
		CHKiRet(rsCStrConstructFromCStr(&pThis->prevLineSegment, pProp->val.pStr));
	} else if(isProp("prevMsgSegment")) {
		CHKiRet(rsCStrConstructFromCStr(&pThis->prevMsgSegment, pProp->val.pStr));
	} else if(isProp("bPrevWasNL")) {
		pThis->bPrevWasNL = (sbool) pProp->val.num;
	}

finalize_it:
	RETiRet;
}
#undef	isProp


/* return the current offset inside the stream. Note that on two consequtive calls, the offset
 * reported on the second call may actually be lower than on the first call. This is due to
 * file circulation. A caller must deal with that. -- rgerhards, 2008-01-30
 */
static rsRetVal
strmGetCurrOffset(strm_t *pThis, int64 *pOffs)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, strm);
	ASSERT(pOffs != NULL);

	*pOffs = pThis->iCurrOffs;

	RETiRet;
}


/* queryInterface function
 * rgerhards, 2008-02-29
 */
BEGINobjQueryInterface(strm)
CODESTARTobjQueryInterface(strm)
	if(pIf->ifVersion != strmCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = strmConstruct;
	pIf->ConstructFinalize = strmConstructFinalize;
	pIf->Destruct = strmDestruct;
	pIf->ReadChar = strmReadChar;
	pIf->UnreadChar = strmUnreadChar;
	pIf->ReadLine = strmReadLine;
	pIf->SeekCurrOffs = strmSeekCurrOffs;
	pIf->Write = strmWrite;
	pIf->WriteChar = strmWriteChar;
	pIf->WriteLong = strmWriteLong;
	pIf->SetFName = strmSetFName;
	pIf->SetFileNotFoundError = strmSetFileNotFoundError;
	pIf->SetDir = strmSetDir;
	pIf->Flush = strmFlush;
	pIf->RecordBegin = strmRecordBegin;
	pIf->RecordEnd = strmRecordEnd;
	pIf->Serialize = strmSerialize;
	pIf->GetCurrOffset = strmGetCurrOffset;
	pIf->Dup = strmDup;
	pIf->SetWCntr = strmSetWCntr;
	pIf->CheckFileChange = CheckFileChange;
	/* set methods */
	pIf->SetbDeleteOnClose = strmSetbDeleteOnClose;
	pIf->SetiMaxFileSize = strmSetiMaxFileSize;
	pIf->SetiMaxFiles = strmSetiMaxFiles;
	pIf->SetiFileNumDigits = strmSetiFileNumDigits;
	pIf->SettOperationsMode = strmSettOperationsMode;
	pIf->SettOpenMode = strmSettOpenMode;
	pIf->SetsType = strmSetsType;
	pIf->SetiZipLevel = strmSetiZipLevel;
	pIf->SetbVeryReliableZip = strmSetbVeryReliableZip;
	pIf->SetbSync = strmSetbSync;
	pIf->SetbReopenOnTruncate = strmSetbReopenOnTruncate;
	pIf->SetsIOBufSize = strmSetsIOBufSize;
	pIf->SetiSizeLimit = strmSetiSizeLimit;
	pIf->SetiFlushInterval = strmSetiFlushInterval;
	pIf->SetpszSizeLimitCmd = strmSetpszSizeLimitCmd;
	pIf->Setcryprov = strmSetcryprov;
	pIf->SetcryprovData = strmSetcryprovData;
finalize_it:
ENDobjQueryInterface(strm)


/* Initialize the stream class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-01-09
 */
BEGINObjClassInit(strm, 1, OBJ_IS_CORE_MODULE)
	/* request objects we use */

	OBJSetMethodHandler(objMethod_SERIALIZE, strmSerialize);
	OBJSetMethodHandler(objMethod_SETPROPERTY, strmSetProperty);
	OBJSetMethodHandler(objMethod_CONSTRUCTION_FINALIZER, strmConstructFinalize);
ENDObjClassInit(strm)

/* vi:set ai:
 */
