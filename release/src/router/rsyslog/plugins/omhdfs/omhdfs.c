/* omhdfs.c
 * This is an output module to support Hadoop's HDFS.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * Copyright 2010-2014 Rainer Gerhards and Adiscon GmbH.
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

#include "config.h"
#include "rsyslog.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/file.h>
#include <pthread.h>
#ifdef HAVE_HDFS_H
#  include <hdfs.h>
#endif
#ifdef HAVE_HADOOP_HDFS_H
#  include <hadoop/hdfs.h>
#endif

#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "conf.h"
#include "cfsysline.h"
#include "module-template.h"
#include "unicode-helper.h"
#include "errmsg.h"
#include "hashtable.h"
#include "hashtable_itr.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
/* MODULE_CNFNAME("omhdfs") we need this only when we convert the module to v2 config system */

/* internal structures
 */
DEF_OMOD_STATIC_DATA

/* global data */
static struct hashtable *files;		/* holds all file objects that we know */
static pthread_mutex_t mutDoAct = PTHREAD_MUTEX_INITIALIZER;

typedef struct configSettings_s {
	uchar *fileName;
	uchar *hdfsHost;
	uchar *dfltTplName;	/* default template name to use */
	int hdfsPort;
} configSettings_t;
static configSettings_t cs;

typedef struct {
	uchar	*name;
	hdfsFS fs;
	hdfsFile fh;
	const char *hdfsHost;
	tPort hdfsPort;
	int nUsers;
	pthread_mutex_t mut;
} file_t;


typedef struct _instanceData {
	file_t *pFile;
	uchar ioBuf[64*1024];
	unsigned offsBuf;
} instanceData;

typedef struct wrkrInstanceData {
	instanceData *pData;
} wrkrInstanceData_t;

/* forward definitions (down here, need data types) */
static inline rsRetVal fileClose(file_t *pFile);

BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	printf("omhdfs: file:%s", pData->pFile->name);
ENDdbgPrintInstInfo


/* note that hdfsFileExists() does not work, so we did our
 * own function to see if a pathname exists. Returns 0 if the
 * file does not exists, something else otherwise. Note that
 * we can also check a directroy (if that matters...)
 */
static int
HDFSFileExists(hdfsFS fs, uchar *name)
{
	int r;
	hdfsFileInfo *info;

	info = hdfsGetPathInfo(fs, (char*)name);
	/* if things go wrong, we assume it is because the file
	 * does not exist. We do not get too much information...
	 */
	if(info == NULL) {
		r = 0;
	} else {
		r = 1;
		hdfsFreeFileInfo(info, 1);
	}
	return r;
}

static inline rsRetVal
HDFSmkdir(hdfsFS fs, uchar *name)
{
	DEFiRet;
	if(hdfsCreateDirectory(fs, (char*)name) == -1)
		ABORT_FINALIZE(RS_RET_ERR);

finalize_it:
	RETiRet;
}


/* ---BEGIN FILE OBJECT---------------------------------------------------- */
/* This code handles the "file object". This is split from the actual
 * instance data, because several instances may write into the same file.
 * If so, we need to use a single object, and also synchronize their writes.
 * So we keep the file object separately, and just stick a reference into
 * the instance data.
 */

static rsRetVal
fileObjConstruct(file_t **ppFile)
{
	file_t *pFile;
	DEFiRet;

	CHKmalloc(pFile = malloc(sizeof(file_t)));
	pFile->name = NULL;
	pFile->hdfsHost = NULL;
	pFile->fh = NULL;
	pFile->nUsers = 0;

	*ppFile = pFile;
finalize_it:
	RETiRet;
}

static void
fileObjAddUser(file_t *pFile)
{
	/* init mutex only when second user is added */
	++pFile->nUsers;
	if(pFile->nUsers == 2)
		pthread_mutex_init(&pFile->mut, NULL);
	DBGPRINTF("omhdfs: file %s now being used by %d actions\n", pFile->name, pFile->nUsers);
}

static rsRetVal
fileObjDestruct(file_t **ppFile)
{
	file_t *pFile = *ppFile;
	if(pFile->nUsers > 1)
		pthread_mutex_destroy(&pFile->mut);
	fileClose(pFile);
	free(pFile->name);
	free((char*)pFile->hdfsHost);
	free(pFile->fh);

	return RS_RET_OK;
}


/* check, and potentially create, all names inside a path */
static rsRetVal
filePrepare(file_t *pFile)
{
	uchar *p;
	uchar *pszWork;
	size_t len;
	DEFiRet;

	if(HDFSFileExists(pFile->fs, pFile->name))
		FINALIZE;

	/* file does not exist, create it (and eventually parent directories */
	if(1) { // check if bCreateDirs
		len = ustrlen(pFile->name) + 1;
		CHKmalloc(pszWork = MALLOC(len));
		memcpy(pszWork, pFile->name, len);
		for(p = pszWork+1 ; *p ; p++)
			if(*p == '/') {
				/* temporarily terminate string, create dir and go on */
				*p = '\0';
				if(!HDFSFileExists(pFile->fs, pszWork)) {
					CHKiRet(HDFSmkdir(pFile->fs, pszWork));
				}
				*p = '/';
			}
		free(pszWork);
		return 0;
	}

finalize_it:
	RETiRet;
}


/* this function is to be used as destructor for the
 * hash table code.
 */
static void
fileObjDestruct4Hashtable(void *ptr)
{
	file_t *pFile = (file_t*) ptr;
	fileObjDestruct(&pFile);
}


static rsRetVal
fileOpen(file_t *pFile)
{
	DEFiRet;

	assert(pFile->fh == NULL);
	if(pFile->nUsers > 1)
		d_pthread_mutex_lock(&pFile->mut);

	DBGPRINTF("omhdfs: try to connect to HDFS at host '%s', port %d\n",
		  pFile->hdfsHost, pFile->hdfsPort);
	pFile->fs = hdfsConnect(pFile->hdfsHost, pFile->hdfsPort);
	if(pFile->fs == NULL) {
		DBGPRINTF("omhdfs: error can not connect to hdfs\n");
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

	CHKiRet(filePrepare(pFile));

	pFile->fh = hdfsOpenFile(pFile->fs, (char*)pFile->name, O_WRONLY|O_APPEND, 0, 0, 0);
	if(pFile->fh == NULL) {
		/* maybe the file does not exist, so we try to create it now.
		 * Note that we can not use hdfsExists() because of a deficit in
		 * it: https://issues.apache.org/jira/browse/HDFS-1154
		 * As of my testing, libhdfs at least seems to return ENOENT if
		 * the file does not exist.
		 */
		if(errno == ENOENT) {
			DBGPRINTF("omhdfs: ENOENT trying to append to '%s', now trying create\n",
				  pFile->name);
		 	pFile->fh = hdfsOpenFile(pFile->fs,
						 (char*)pFile->name, O_WRONLY|O_CREAT, 0, 0, 0);
		}
	}
	if(pFile->fh == NULL) {
		DBGPRINTF("omhdfs: failed to open %s for writing!\n", pFile->name);
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}

finalize_it:
	if(pFile->nUsers > 1)
		d_pthread_mutex_unlock(&pFile->mut);
	RETiRet;
}


/* Note: lenWrite is reset to zero on successful write! */
static rsRetVal
fileWrite(file_t *pFile, uchar *buf, size_t *lenWrite)
{
	DEFiRet;

	if(*lenWrite == 0)
		FINALIZE;

	if(pFile->nUsers > 1)
		d_pthread_mutex_lock(&pFile->mut);

	/* open file if not open. This must be done *here* and while mutex-protected
	 * because of HUP handling (which is async to normal processing!).
	 */
	if(pFile->fh == NULL) {
		fileOpen(pFile);
		if(pFile->fh == NULL) {
			ABORT_FINALIZE(RS_RET_SUSPENDED);
		}
	}

	tSize num_written_bytes = hdfsWrite(pFile->fs, pFile->fh, buf, *lenWrite);
	if((unsigned) num_written_bytes != *lenWrite) {
		LogError(errno, RS_RET_ERR_HDFS_WRITE,
			        "omhdfs: failed to write %s, expected %lu bytes, "
			        "written %lu\n", pFile->name, (unsigned long) *lenWrite,
				(unsigned long) num_written_bytes);
		ABORT_FINALIZE(RS_RET_SUSPENDED);
	}
	*lenWrite = 0;

finalize_it:
	RETiRet;
}


static rsRetVal
fileClose(file_t *pFile)
{
	DEFiRet;

	if(pFile->fh == NULL)
		FINALIZE;

	if(pFile->nUsers > 1)
		d_pthread_mutex_lock(&pFile->mut);

	hdfsCloseFile(pFile->fs, pFile->fh);
	pFile->fh = NULL;

	if(pFile->nUsers > 1)
		d_pthread_mutex_unlock(&pFile->mut);

finalize_it:
	RETiRet;
}

/* ---END FILE OBJECT---------------------------------------------------- */

/* This adds data to the output buffer and performs an actual write
 * if the new data does not fit into the buffer. Note that we never write
 * partial data records. Other actions may write into the same file, and if
 * we would write partial records, data could become severely mixed up.
 * Note that we must check of some new data arrived is large than our
 * buffer. In that case, the new data will written with its own
 * write operation.
 */
static rsRetVal
addData(instanceData *pData, uchar *buf)
{
	unsigned len;
	DEFiRet;

	len = strlen((char*)buf);
	if(pData->offsBuf + len < sizeof(pData->ioBuf)) {
		/* new data fits into remaining buffer */
		memcpy((char*) pData->ioBuf + pData->offsBuf, buf, len);
		pData->offsBuf += len;
	} else {
		CHKiRet(fileWrite(pData->pFile, pData->ioBuf, &pData->offsBuf));
		if(len >= sizeof(pData->ioBuf)) {
			CHKiRet(fileWrite(pData->pFile, buf, &len));
		} else {
			memcpy((char*) pData->ioBuf + pData->offsBuf, buf, len);
			pData->offsBuf += len;
		}
	}

	iRet = RS_RET_DEFER_COMMIT;
finalize_it:
	RETiRet;
}

BEGINcreateInstance
CODESTARTcreateInstance
	pData->pFile = NULL;
ENDcreateInstance


BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
ENDcreateWrkrInstance


BEGINfreeInstance
CODESTARTfreeInstance
	if(pData->pFile != NULL)
		fileObjDestruct(&pData->pFile);
ENDfreeInstance


BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
ENDfreeWrkrInstance


BEGINtryResume
	instanceData *pData = pWrkrData->pData;
CODESTARTtryResume
	pthread_mutex_lock(&mutDoAct);
	fileClose(pData->pFile);
	fileOpen(pData->pFile);
	if(pData->pFile->fh == NULL){
		dbgprintf("omhdfs: tried to resume file %s, but still no luck...\n",
			  pData->pFile->name);
		iRet = RS_RET_SUSPENDED;
	}
	pthread_mutex_unlock(&mutDoAct);
ENDtryResume


BEGINbeginTransaction
CODESTARTbeginTransaction
	DBGPRINTF("omhdfs: beginTransaction\n");
ENDbeginTransaction


BEGINdoAction
	instanceData *pData = pWrkrData->pData;
CODESTARTdoAction
	DBGPRINTF("omhdfs: action to to write to %s\n", pData->pFile->name);
	pthread_mutex_lock(&mutDoAct);
	iRet = addData(pData, ppString[0]);
	DBGPRINTF("omhdfs: done doAction\n");
	pthread_mutex_unlock(&mutDoAct);
ENDdoAction


BEGINendTransaction
	instanceData *pData = pWrkrData->pData;
CODESTARTendTransaction
	DBGPRINTF("omhdfs: endTransaction\n");
	pthread_mutex_lock(&mutDoAct);
	if(pData->offsBuf != 0) {
		DBGPRINTF("omhdfs: data unwritten at end of transaction, persisting...\n");
		iRet = fileWrite(pData->pFile, pData->ioBuf, &pData->offsBuf);
	}
	pthread_mutex_unlock(&mutDoAct);
ENDendTransaction


BEGINparseSelectorAct
	file_t *pFile;
	int r;
	uchar *keybuf;
CODESTARTparseSelectorAct

	/* first check if this config line is actually for us */
	if(strncmp((char*) p, ":omhdfs:", sizeof(":omhdfs:") - 1)) {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	p += sizeof(":omhdfs:") - 1; /* eat indicator sequence  (-1 because of '\0'!) */
	CHKiRet(createInstance(&pData));
	CODE_STD_STRING_REQUESTparseSelectorAct(1)
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, 0,
				       (cs.dfltTplName == NULL) ? (uchar*)"RSYSLOG_FileFormat" : cs.dfltTplName));

	if(cs.fileName == NULL) {
		LogError(0, RS_RET_ERR_HDFS_OPEN, "omhdfs: no file name specified, can not continue");
		ABORT_FINALIZE(RS_RET_FILE_NOT_SPECIFIED);
	}

	pFile = hashtable_search(files, cs.fileName);
	if(pFile == NULL) {
		/* we need a new file object, this one not seen before */
		CHKiRet(fileObjConstruct(&pFile));
		CHKmalloc(pFile->name = cs.fileName);
		CHKmalloc(keybuf = ustrdup(cs.fileName));
		cs.fileName = NULL; /* re-set, data passed to file object */
		CHKmalloc(pFile->hdfsHost = strdup((cs.hdfsHost == NULL) ? "default" : (char*) cs.hdfsHost));
		pFile->hdfsPort = cs.hdfsPort;
		fileOpen(pFile);
		if(pFile->fh == NULL){
			LogError(0, RS_RET_ERR_HDFS_OPEN, "omhdfs: failed to open %s - "
				    	"retrying later", pFile->name);
			iRet = RS_RET_SUSPENDED;
		}
		r = hashtable_insert(files, keybuf, pFile);
		if(r == 0)
			ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);
	}
	fileObjAddUser(pFile);
	pData->pFile = pFile;
	pData->offsBuf = 0;

CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct


BEGINdoHUP
	file_t *pFile;
	struct hashtable_itr *itr;
CODESTARTdoHUP
	DBGPRINTF("omhdfs: HUP received (file count %d)\n", hashtable_count(files));
	/* Iterator constructor only returns a valid iterator if
	* the hashtable is not empty */
	itr = hashtable_iterator(files);
	if(hashtable_count(files) > 0)
	{
		do {
			pFile = (file_t *) hashtable_iterator_value(itr);
			fileClose(pFile);
			DBGPRINTF("omhdfs: HUP, closing file %s\n", pFile->name);
		} while (hashtable_iterator_advance(itr));
	}
ENDdoHUP


/* Reset config variables for this module to default values.
 * rgerhards, 2007-07-17
 */
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	cs.hdfsHost = NULL;
	cs.hdfsPort = 0;
	free(cs.fileName);
	cs.fileName = NULL;
	free(cs.dfltTplName);
	cs.dfltTplName = NULL;
	return RS_RET_OK;
}


BEGINmodExit
CODESTARTmodExit
	if(files != NULL)
		hashtable_destroy(files, 1); /* 1 => free all values automatically */
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
CODEqueryEtryPt_TXIF_OMOD_QUERIES /* we support the transactional interface! */
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_doHUP
ENDqueryEtryPt



BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION;
CODEmodInit_QueryRegCFSLineHdlr
	CHKmalloc(files = create_hashtable(20, hash_from_string, key_equals_string,
			                   fileObjDestruct4Hashtable));

	CHKiRet(regCfSysLineHdlr((uchar *)"omhdfsfilename", 0, eCmdHdlrGetWord, NULL, &cs.fileName, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"omhdfshost", 0, eCmdHdlrGetWord, NULL, &cs.hdfsHost, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"omhdfsport", 0, eCmdHdlrInt, NULL, &cs.hdfsPort, NULL));
	CHKiRet(regCfSysLineHdlr((uchar *)"omhdfsdefaulttemplate", 0, eCmdHdlrGetWord, NULL, &cs.dfltTplName, NULL));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler, resetConfigVariables,
	NULL, STD_LOADABLE_MODULE_ID));
	DBGPRINTF("omhdfs: module compiled with rsyslog version %s.\n", VERSION);
CODEmodInit_QueryRegCFSLineHdlr
ENDmodInit
