/* imfile.c
 *
 * This is the input module for reading text file data. A text file is a
 * non-binary file who's lines are delemited by the \n character.
 *
 * Work originally begun on 2008-02-01 by Rainer Gerhards
 *
 * Copyright 2008-2018 Adiscon GmbH.
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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <glob.h>
#include <poll.h>
#include <json.h>
#include <fnmatch.h>
#ifdef HAVE_SYS_INOTIFY_H
#include <sys/inotify.h>
#include <linux/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#	include <sys/stat.h>
#endif
#if defined(OS_SOLARIS) && defined (HAVE_PORT_SOURCE_FILE)
#include <port.h>
#include <sys/port.h>
#endif
#include "rsyslog.h"		/* error codes etc... */
#include "dirty.h"
#include "cfsysline.h"		/* access to config file objects */
#include "module-template.h"	/* generic module interface code - very important, read it! */
#include "srUtils.h"		/* some utility functions */
#include "msg.h"
#include "stream.h"
#include "errmsg.h"
#include "glbl.h"
#include "unicode-helper.h"
#include "prop.h"
#include "stringbuf.h"
#include "ruleset.h"
#include "ratelimit.h"
#include "srUtils.h"
#include "parserif.h"

#include <regex.h>

MODULE_TYPE_INPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("imfile")

/* defines */
#define FILE_ID_HASH_SIZE 20	/* max size of a file_id hash */
#define FILE_ID_SIZE	512	/* how many bytes are used for file-id? */

/* Module static data */
DEF_IMOD_STATIC_DATA	/* must be present, starts static data */
DEFobjCurrIf(glbl)
DEFobjCurrIf(strm)
DEFobjCurrIf(prop)
DEFobjCurrIf(ruleset)

extern int rs_siphash(const uint8_t *in, const size_t inlen, const uint8_t *k,
	uint8_t *out, const size_t outlen); /* see siphash.c */

static int bLegacyCnfModGlobalsPermitted;/* are legacy module-global config parameters permitted? */

#define NUM_MULTISUB 1024 /* default max number of submits */
#define DFLT_PollInterval 10
#define INIT_WDMAP_TAB_SIZE 1 /* default wdMap table size - is extended as needed, use 2^x value */
#define ADD_METADATA_UNSPECIFIED -1

/* If set to 1, fileTableDisplay will be compiled and used for debugging */
#define ULTRA_DEBUG 0

/* Setting GLOB_BRACE to ZERO which disables support for GLOB_BRACE if not available on current platform */
#ifndef GLOB_BRACE
	#define GLOB_BRACE 0
#endif


static struct configSettings_s {
	uchar *pszFileName;
	uchar *pszFileTag;
	uchar *pszStateFile;
	uchar *pszBindRuleset;
	int iPollInterval;
	int iPersistStateInterval;	/* how often if state file to be persisted? (default 0->never) */
	int iFacility; /* local0 */
	int iSeverity;  /* notice, as of rfc 3164 */
	int readMode;  /* mode to use for ReadMultiLine call */
	int64 maxLinesAtOnce;	/* how many lines to process in a row? */
	uint32_t trimLineOverBytes;  /* 0: never trim line, positive number: trim line if over bytes */
} cs;

struct instanceConf_s {
	uchar *pszFileName;
	uchar *pszFileName_forOldStateFile; /* we unfortunately needs this to read old state files */
	uchar *pszDirName;
	uchar *pszFileBaseName;
	uchar *pszTag;
	size_t lenTag;
	uchar *pszStateFile;
	uchar *pszBindRuleset;
	int nMultiSub;
	int iPersistStateInterval;
	int iFacility;
	int iSeverity;
	int readTimeout;
	unsigned delay_perMsg;
	sbool bRMStateOnDel;
	uint8_t readMode;
	uchar *startRegex;
	uchar *endRegex;
	regex_t start_preg;	/* compiled version of startRegex */
	regex_t end_preg;	/* compiled version of endRegex */
	sbool discardTruncatedMsg;
	sbool msgDiscardingError;
	sbool escapeLF;
	sbool reopenOnTruncate;
	sbool addCeeTag;
	sbool addMetadata;
	sbool freshStartTail;
	sbool fileNotFoundError;
	int maxLinesAtOnce;
	uint32_t trimLineOverBytes;
	ruleset_t *pBindRuleset;	/* ruleset to bind listener to (use system default if unspecified) */
	struct instanceConf_s *next;
};


/* file system objects */
typedef struct fs_edge_s fs_edge_t;
typedef struct fs_node_s fs_node_t;
typedef struct act_obj_s act_obj_t;
struct act_obj_s {
	act_obj_t *prev;
	act_obj_t *next;
	fs_edge_t *edge;	/* edge which this object belongs to */
	char *name;		/* full path name of active object */
	char *basename;		/* only basename */ //TODO: remove when refactoring rename support
	char *source_name;	/* if this object is target of a symlink, source_name is its name (else NULL) */
	//char *statefile;	/* base name of state file (for move operations) */
	int wd;
#if defined(OS_SOLARIS) && defined (HAVE_PORT_SOURCE_FILE)
	struct fileinfo *pfinf;
	sbool bPortAssociated;
	int is_deleted;	/* debugging: entry deleted? */
#endif
	time_t timeoutBase; /* what time to calculate the timeout against? */
	/* file dynamic data */
	int in_move;	/* workaround for inotify move: if set, state file must not be deleted */
	ino_t ino;	/* current inode nbr */
	strm_t *pStrm;	/* its stream (NULL if not assigned) */
	int nRecords; /**< How many records did we process before persisting the stream? */
	ratelimit_t *ratelimiter;
	multi_submit_t multiSub;
	int is_symlink;
};
struct fs_edge_s {
	fs_node_t *parent;	/* node pointing to this edge */
	fs_node_t *node;	/* node this edge points to */
	fs_edge_t *next;
	uchar *name;
	uchar *path;
	act_obj_t *active;
	int is_file;
	int ninst;		/* nbr of instances in instarr */
	instanceConf_t **instarr;
};
struct fs_node_s {
	fs_edge_t *edges;	/* NULL in leaf nodes */
	fs_node_t *root;	/* node one level up (NULL for file system root) */
};


/* forward definitions */
static rsRetVal persistStrmState(act_obj_t *);
static rsRetVal resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal);
static rsRetVal ATTR_NONNULL(1) pollFile(act_obj_t *act);
static int ATTR_NONNULL() getBasename(uchar *const __restrict__ basen, uchar *const __restrict__ path);
static void ATTR_NONNULL() act_obj_unlink(act_obj_t *act);
static uchar * ATTR_NONNULL(1, 2) getStateFileName(const act_obj_t *, uchar *, const size_t);
static int ATTR_NONNULL() getFullStateFileName(const uchar *const, const char *const,
	uchar *const pszout, const size_t ilenout);


#define OPMODE_POLLING 0
#define OPMODE_INOTIFY 1
#define OPMODE_FEN 2

/* config variables */
struct modConfData_s {
	rsconf_t *pConf;	/* our overall config object */
	int iPollInterval;	/* number of seconds to sleep when there was no file activity */
	int readTimeout;
	int timeoutGranularity;		/* value in ms */
	instanceConf_t *root, *tail;
	fs_node_t *conf_tree;
	uint8_t opMode;
	sbool configSetViaV2Method;
	sbool sortFiles;
	sbool normalizePath;	/* normalize file system pathes (all start with root dir) */
	sbool haveReadTimeouts;	/* use special processing if read timeouts exist */
	sbool bHadFileData;	/* actually a global variable:
				   1 - last call to pollFile() had data
				   0 - last call to pollFile() had NO data
				   Must be manually reset to 0 if desired. Helper for
				   polling mode.
				 */
};
static modConfData_t *loadModConf = NULL;/* modConf ptr to use for the current load process */
static modConfData_t *runModConf = NULL;/* modConf ptr to use for the current load process */


#ifdef HAVE_INOTIFY_INIT
/* We need to map watch descriptors to our actual objects. Unfortunately, the
 * inotify API does not provide us with any cookie, so a simple O(1) algorithm
 * cannot be done (what a shame...). We assume that maintaining the array is much
 * less often done than looking it up, so we keep the array sorted by watch descriptor
 * and do a binary search on the wd we get back. This is at least O(log n), which
 * is not too bad for the anticipated use case.
 */
struct wd_map_s {
	int wd;		/* ascending sort key */
	act_obj_t *act; /* point to related active object */
};
typedef struct wd_map_s wd_map_t;
static wd_map_t *wdmap = NULL;
static int nWdmap;
static int allocMaxWdmap;
static int ino_fd;	/* fd for inotify calls */
#endif /* #if HAVE_INOTIFY_INIT -------------------------------------------------- */

#if defined(OS_SOLARIS) && defined (HAVE_PORT_SOURCE_FILE)
struct fileinfo {
	struct file_obj fobj;
	int events;
	int port;
};

static int glport; /* Static port handle for FEN api*/
#endif /* #if OS_SOLARIS -------------------------------------------------- */

static prop_t *pInputName = NULL;
/* there is only one global inputName for all messages generated by this input */

/* module-global parameters */
static struct cnfparamdescr modpdescr[] = {
	{ "pollinginterval", eCmdHdlrPositiveInt, 0 },
	{ "readtimeout", eCmdHdlrPositiveInt, 0 },
	{ "timeoutgranularity", eCmdHdlrPositiveInt, 0 },
	{ "sortfiles", eCmdHdlrBinary, 0 },
	{ "normalizepath", eCmdHdlrBinary, 0 },
	{ "mode", eCmdHdlrGetWord, 0 }
};
static struct cnfparamblk modpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(modpdescr)/sizeof(struct cnfparamdescr),
	  modpdescr
	};

/* input instance parameters */
static struct cnfparamdescr inppdescr[] = {
	{ "file", eCmdHdlrString, CNFPARAM_REQUIRED },
	{ "tag", eCmdHdlrString, CNFPARAM_REQUIRED },
	{ "severity", eCmdHdlrSeverity, 0 },
	{ "facility", eCmdHdlrFacility, 0 },
	{ "ruleset", eCmdHdlrString, 0 },
	{ "readmode", eCmdHdlrInt, 0 },
	{ "startmsg.regex", eCmdHdlrString, 0 },
	{ "endmsg.regex", eCmdHdlrString, 0 },
	{ "discardtruncatedmsg", eCmdHdlrBinary, 0 },
	{ "msgdiscardingerror", eCmdHdlrBinary, 0 },
	{ "escapelf", eCmdHdlrBinary, 0 },
	{ "reopenontruncate", eCmdHdlrBinary, 0 },
	{ "maxlinesatonce", eCmdHdlrInt, 0 },
	{ "trimlineoverbytes", eCmdHdlrInt, 0 },
	{ "maxsubmitatonce", eCmdHdlrInt, 0 },
	{ "removestateondelete", eCmdHdlrBinary, 0 },
	{ "persiststateinterval", eCmdHdlrInt, 0 },
	{ "deletestateonfiledelete", eCmdHdlrBinary, 0 },
	{ "delay.message", eCmdHdlrPositiveInt, 0 },
	{ "addmetadata", eCmdHdlrBinary, 0 },
	{ "addceetag", eCmdHdlrBinary, 0 },
	{ "statefile", eCmdHdlrString, CNFPARAM_DEPRECATED },
	{ "readtimeout", eCmdHdlrPositiveInt, 0 },
	{ "freshstarttail", eCmdHdlrBinary, 0},
	{ "filenotfounderror", eCmdHdlrBinary, 0}
};
static struct cnfparamblk inppblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(inppdescr)/sizeof(struct cnfparamdescr),
	  inppdescr
	};

#include "im-helper.h" /* must be included AFTER the type definitions! */


/* Support for "old cruft" state files will potentially become optional in the
 * future (hopefully). To prepare so, we use conditional compilation with a
 * fixed-true condition ;-) -- rgerhards, 2018-03-28
 * reason: https://github.com/rsyslog/rsyslog/issues/2231#issuecomment-376862280
 */
#define ENABLE_V1_STATE_FILE_FORMAT_SUPPORT 1
#ifdef ENABLE_V1_STATE_FILE_FORMAT_SUPPORT
static uchar * ATTR_NONNULL(1, 2)
OLD_getStateFileName(const instanceConf_t *const inst,
	 uchar *const __restrict__ buf,
	 const size_t lenbuf)
{
	DBGPRINTF("OLD_getStateFileName trying '%s'\n", inst->pszFileName_forOldStateFile);
	snprintf((char*)buf, lenbuf - 1, "imfile-state:%s", inst->pszFileName_forOldStateFile);
	buf[lenbuf-1] = '\0'; /* be on the safe side... */
	uchar *p = buf;
	for( ; *p ; ++p) {
		if(*p == '/')
			*p = '-';
	}
	return buf;
}

/* try to open an old-style state file for given file. If the state file does not
 * exist or cannot be read, an error is returned.
 */
static rsRetVal ATTR_NONNULL(1)
OLD_openFileWithStateFile(act_obj_t *const act)
{
	DEFiRet;
	strm_t *psSF = NULL;
	uchar pszSFNam[MAXFNAME];
	size_t lenSFNam;
	struct stat stat_buf;
	uchar statefile[MAXFNAME];
	const instanceConf_t *const inst = act->edge->instarr[0];// TODO: same file, multiple instances?

	uchar *const statefn = OLD_getStateFileName(inst, statefile, sizeof(statefile));
	DBGPRINTF("OLD_openFileWithStateFile: trying to open state for '%s', state file '%s'\n",
		  act->name, statefn);

	/* Get full path and file name */
	lenSFNam = getFullStateFileName(statefn, "", pszSFNam, sizeof(pszSFNam));

	/* check if the file exists */
	if(stat((char*) pszSFNam, &stat_buf) == -1) {
		if(errno == ENOENT) {
			DBGPRINTF("OLD_openFileWithStateFile: NO state file (%s) exists for '%s'\n",
				pszSFNam, act->name);
			ABORT_FINALIZE(RS_RET_FILE_NOT_FOUND);
		} else {
			char errStr[1024];
			rs_strerror_r(errno, errStr, sizeof(errStr));
			DBGPRINTF("OLD_openFileWithStateFile: error trying to access state "
				"file for '%s':%s\n", act->name, errStr);
			ABORT_FINALIZE(RS_RET_IO_ERROR);
		}
	}

	/* If we reach this point, we have a state file */

	DBGPRINTF("old state file found - instantiating from it\n");
	CHKiRet(strm.Construct(&psSF));
	CHKiRet(strm.SettOperationsMode(psSF, STREAMMODE_READ));
	CHKiRet(strm.SetsType(psSF, STREAMTYPE_FILE_SINGLE));
	CHKiRet(strm.SetFName(psSF, pszSFNam, lenSFNam));
	CHKiRet(strm.SetFileNotFoundError(psSF, inst->fileNotFoundError));
	CHKiRet(strm.ConstructFinalize(psSF));

	/* read back in the object */
	CHKiRet(obj.Deserialize(&act->pStrm, (uchar*) "strm", psSF, NULL, act));
	free(act->pStrm->pszFName);
	CHKmalloc(act->pStrm->pszFName = ustrdup(act->name));

	strm.CheckFileChange(act->pStrm);
	CHKiRet(strm.SeekCurrOffs(act->pStrm));

	/* we now persist the new state file and delete the old one, so we will
	 * never have to deal with the old one. */
	persistStrmState(act);
	unlink((char*)pszSFNam);

finalize_it:
	if(psSF != NULL)
		strm.Destruct(&psSF);
	RETiRet;
}
#endif /* #ifdef ENABLE_V1_STATE_FILE_FORMAT_SUPPORT */



#if 0 // Code we can potentially use for new functionality // TODO: use or remove
//TODO add a kind of portable asprintf:
static const char * ATTR_NONNULL()
gen_full_name(const char *const dirname, const char *const name)
{
	const size_t len_full_name = strlen(dirname) + 1 + strlen(name) + 1;
	char *const full_name = malloc(len_full_name);
	if(full_name == NULL)
		return NULL;

	snprintf(full_name, len_full_name, "%s/%s", dirname, name);
	return full_name;
}
#endif


#ifdef HAVE_INOTIFY_INIT
#if ULTRA_DEBUG == 1
static void
dbg_wdmapPrint(const char *msg)
{
	int i;
	DBGPRINTF("%s\n", msg);
	for(i = 0 ; i < nWdmap ; ++i)
		DBGPRINTF("wdmap[%d]: wd: %d, act %p, name: %s\n",
			i, wdmap[i].wd, wdmap[i].act, wdmap[i].act->name);
}
#endif

static rsRetVal
wdmapInit(void)
{
	DEFiRet;
	free(wdmap);
	CHKmalloc(wdmap = malloc(sizeof(wd_map_t) * INIT_WDMAP_TAB_SIZE));
	allocMaxWdmap = INIT_WDMAP_TAB_SIZE;
	nWdmap = 0;
finalize_it:
	RETiRet;
}


/* note: we search backwards, as inotify tends to return increasing wd's */
static rsRetVal
wdmapAdd(int wd, act_obj_t *const act)
{
	wd_map_t *newmap;
	int newmapsize;
	int i;
	DEFiRet;

	for(i = nWdmap-1 ; i >= 0 && wdmap[i].wd > wd ; --i)
		; 	/* just scan */
	if(i >= 0 && wdmap[i].wd == wd) {
		LogError(0, RS_RET_INTERNAL_ERROR, "imfile: wd %d already in wdmap!", wd);
		ABORT_FINALIZE(RS_RET_FILE_ALREADY_IN_TABLE);
	}
	++i;
	/* i now points to the entry that is to be moved upwards (or end of map) */
	if(nWdmap == allocMaxWdmap) {
		newmapsize = 2 * allocMaxWdmap;
		CHKmalloc(newmap = realloc(wdmap, sizeof(wd_map_t) * newmapsize));
		// TODO: handle the error more intelligently? At all possible? -- 2013-10-15
		wdmap = newmap;
		allocMaxWdmap = newmapsize;
	}
	if(i < nWdmap) {
		/* we need to shift to make room for new entry */
		memmove(wdmap + i + 1, wdmap + i, sizeof(wd_map_t) * (nWdmap - i));
	}
	wdmap[i].wd = wd;
	wdmap[i].act = act;
	++nWdmap;
	DBGPRINTF("add wdmap[%d]: wd %d, act obj %p, path %s\n", i, wd, act, act->name);

finalize_it:
	RETiRet;
}

/* return wd or -1 on error */
static int
in_setupWatch(act_obj_t *const act, const int is_file)
{
	int wd = -1;
	if(runModConf->opMode != OPMODE_INOTIFY)
		goto done;

	wd = inotify_add_watch(ino_fd, act->name,
		(is_file) ? IN_MODIFY|IN_DONT_FOLLOW : IN_CREATE|IN_DELETE|IN_MOVED_FROM|IN_MOVED_TO);
	if(wd < 0) {
		if (errno == EACCES) { /* There is high probability of selinux denial on top-level paths */
			DBGPRINTF("imfile: permission denied when adding watch for '%s'\n", act->name);
		} else {
			LogError(errno, RS_RET_IO_ERROR, "imfile: cannot watch object '%s'", act->name);
		}
		goto done;
	}
	wdmapAdd(wd, act);
	DBGPRINTF("in_setupWatch: watch %d added for %s(object %p)\n", wd, act->name, act);
done:	return wd;
}

/* compare function for bsearch() */
static int
wdmap_cmp(const void *k, const void *a)
{
	int key = *((int*) k);
	wd_map_t *etry = (wd_map_t*) a;
	if(key < etry->wd)
		return -1;
	else if(key > etry->wd)
		return 1;
	else
		return 0;
}
/* looks up a wdmap entry and returns it's index if found
 * or -1 if not found.
 */
static wd_map_t *
wdmapLookup(int wd)
{
	return bsearch(&wd, wdmap, nWdmap, sizeof(wd_map_t), wdmap_cmp);
}


static rsRetVal
wdmapDel(const int wd)
{
	int i;
	DEFiRet;

	for(i = 0 ; i < nWdmap && wdmap[i].wd < wd ; ++i)
		; 	/* just scan */
	if(i == nWdmap ||  wdmap[i].wd != wd) {
		DBGPRINTF("wd %d shall be deleted but not in wdmap!\n", wd);
		FINALIZE;
	}

	if(i < nWdmap-1) {
		/* we need to shift to delete it (see comment at wdmap definition) */
		memmove(wdmap + i, wdmap + i + 1, sizeof(wd_map_t) * (nWdmap - i - 1));
	}
	--nWdmap;
	DBGPRINTF("wd %d deleted, was idx %d\n", wd, i);

finalize_it:
	RETiRet;
}

#endif // #ifdef HAVE_INOTIFY_INIT

#if defined(OS_SOLARIS) && defined (HAVE_PORT_SOURCE_FILE)
static void ATTR_NONNULL()
fen_setupWatch(act_obj_t *const act)
{
	DBGPRINTF("fen_setupWatch: enter, opMode %d\n", runModConf->opMode);
	if(runModConf->opMode != OPMODE_FEN)
		goto done;

	DBGPRINTF("fen_setupWatch: %s\n", act->name);
	if(act->pfinf == NULL) {
		act->pfinf = malloc(sizeof(struct fileinfo));
		if (act->pfinf == NULL) {
			LogError(errno, RS_RET_OUT_OF_MEMORY, "imfile: fen_setupWatch alloc memory "
				"for fileinfo failed ");
			goto done;
		}
		if ((act->pfinf->fobj.fo_name = strdup(act->name)) == NULL) {
			LogError(errno, RS_RET_OUT_OF_MEMORY, "imfile: fen_setupWatch alloc memory "
				"for strdup failed ");
			free(act->pfinf);
			act->pfinf = NULL;
			goto done;
		}
		act->pfinf->events = FILE_MODIFIED;
		act->pfinf->port = glport;
		act->bPortAssociated = 0;
	}

	DBGPRINTF("fen_setupWatch: bPortAssociated %d\n", act->bPortAssociated);
	if(act->bPortAssociated) {
		goto done;
	}

	struct stat fileInfo;
	const int r = stat(act->name, &fileInfo);
	if(r == -1) { /* object gone away? */
		DBGPRINTF("fen_setupWatch: file gone away, no watch: '%s'\n", act->name);
		goto done;
	}

	/* note: FEN watch must be re-registered each time - this is what we do now */
	act->pfinf->fobj.fo_atime = fileInfo.st_atim;
	act->pfinf->fobj.fo_mtime = fileInfo.st_mtim;
	act->pfinf->fobj.fo_ctime = fileInfo.st_ctim;
	if(port_associate(glport, PORT_SOURCE_FILE, (uintptr_t)&(act->pfinf->fobj),
				act->pfinf->events, (void *)act) == -1) {
		LogError(errno, RS_RET_SYS_ERR, "fen_setupWatch: Failed to associate port for file "
			": %s\n", act->pfinf->fobj.fo_name);
		goto done;
	} else {
		/* Port successfull listening now*/
		DBGPRINTF("fen_setupWatch: associated port for file %s\n", act->name);
		act->bPortAssociated = 1;
	}

	DBGPRINTF("in_setupWatch: fen association added for %s\n", act->name);
done:	return;
}
#else
static void ATTR_NONNULL()
fen_setupWatch(act_obj_t *const act __attribute__((unused)))
{
	DBGPRINTF("fen_setupWatch: DUMMY CALLED - not on Solaris?\n");
}
#endif /* FEN */

static void
fs_node_print(const fs_node_t *const node, const int level)
{
	fs_edge_t *chld;
	act_obj_t *act;
	dbgprintf("node print[%2.2d]: %p edges:\n", level, node);

	for(chld = node->edges ; chld != NULL ; chld = chld->next) {
		dbgprintf("node print[%2.2d]:     child %p '%s' isFile %d, path: '%s'\n",
			level, chld->node, chld->name, chld->is_file, chld->path);
		for(int i = 0 ; i < chld->ninst ; ++i) {
			dbgprintf("\tinst: %p\n", chld->instarr[i]);
		}
		for(act = chld->active ; act != NULL ; act = act->next) {
			dbgprintf("\tact : %p\n", act);
			dbgprintf("\tact : %p: name '%s', wd: %d\n",
				act, act->name, act->wd);
		}
	}
	for(chld = node->edges ; chld != NULL ; chld = chld->next) {
		fs_node_print(chld->node, level+1);
	}
}

/* add a new file system object if it not yet exists, ignore call
 * if it already does.
 */
static rsRetVal ATTR_NONNULL(1,2)
act_obj_add(fs_edge_t *const edge, const char *const name, const int is_file,
	const ino_t ino, const int is_symlink, const char *const source)
{
	act_obj_t *act;
	char basename[MAXFNAME];
	DEFiRet;
	
	DBGPRINTF("act_obj_add: edge %p, name '%s' (source '%s')\n", edge, name, source? source : "---");
	for(act = edge->active ; act != NULL ; act = act->next) {
		if(!strcmp(act->name, name)) {
			if (!source || !act->source_name || !strcmp(act->source_name, source)) {
				DBGPRINTF("active object '%s' already exists in '%s' - no need to add\n",
					name, edge->path);
				FINALIZE;
			}
		}
	}
	DBGPRINTF("add new active object '%s' in '%s'\n", name, edge->path);
	CHKmalloc(act = calloc(sizeof(act_obj_t), 1));
	CHKmalloc(act->name = strdup(name));
	if (-1 == getBasename((uchar*)basename, (uchar*)name)) {
		CHKmalloc(act->basename = strdup(name)); /* assume basename is same as name */
	} else {
		CHKmalloc(act->basename = strdup(basename));
	}
	act->edge = edge;
	act->ino = ino;
	act->is_symlink = is_symlink;
	if (source) { /* we are target of symlink */
		CHKmalloc(act->source_name = strdup(source));
	} else {
		act->source_name = NULL;
	}
	#ifdef HAVE_INOTIFY_INIT
	act->wd = in_setupWatch(act, is_file);
	#endif
	fen_setupWatch(act);
	if(is_file && !is_symlink) {
		const instanceConf_t *const inst = edge->instarr[0];// TODO: same file, multiple instances?
		CHKiRet(ratelimitNew(&act->ratelimiter, "imfile", name));
		CHKmalloc(act->multiSub.ppMsgs = MALLOC(inst->nMultiSub * sizeof(smsg_t *)));
		act->multiSub.maxElem = inst->nMultiSub;
		act->multiSub.nElem = 0;
		pollFile(act);
	}

	/* all well, add to active list */
	if(edge->active != NULL) {
		edge->active->prev = act;
	}
	act->next = edge->active;
	edge->active = act;
//dbgprintf("printout of fs tree after act_obj_add for '%s'\n", name);
//fs_node_print(runModConf->conf_tree, 0);
//dbg_wdmapPrint("wdmap after act_obj_add");
finalize_it:
	if(iRet != RS_RET_OK) {
		if(act != NULL) {
			free(act->name);
			free(act);
		}
	}
	RETiRet;
}


/* this walks an edges active list and detects and acts on any changes
 * seen there. It does NOT detect newly appeared files, as they are not
 * inside the active list!
 */
static void
detect_updates(fs_edge_t *const edge)
{
	act_obj_t *act;
	struct stat fileInfo;
	int restart = 0;

	for(act = edge->active ; act != NULL ; act = act->next) {
		DBGPRINTF("detect_updates checking active obj '%s'\n", act->name);
		const int r = lstat(act->name, &fileInfo);
		if(r == -1) { /* object gone away? */
			DBGPRINTF("object gone away, unlinking: '%s'\n", act->name);
			act_obj_unlink(act);
			restart = 1;
			break;
		}
		// TODO: add inode check for change notification!

	}

	if (restart) {
		detect_updates(edge);
	}
}


/* check if active files need to be processed. This is only needed in
 * polling mode.
 */
static void ATTR_NONNULL()
poll_active_files(fs_edge_t *const edge)
{
	if(   runModConf->opMode != OPMODE_POLLING
	   || !edge->is_file
	   || glbl.GetGlobalInputTermState() != 0) {
		return;
	}

	act_obj_t *act;
	for(act = edge->active ; act != NULL ; act = act->next) {
		fen_setupWatch(act);
		DBGPRINTF("poll_active_files: polling '%s'\n", act->name);
		pollFile(act);
	}
}

static rsRetVal ATTR_NONNULL()
process_symlink(fs_edge_t *const chld, const char *symlink)
{
	DEFiRet;
	char *target;
	CHKmalloc(target = realpath(symlink, NULL));
	struct stat fileInfo;
	if(lstat(target, &fileInfo) != 0) {
		LogError(errno, RS_RET_ERR,	"imfile: process_symlink: cannot stat file '%s' - ignored", target);
		FINALIZE;
	}
	const int is_file = (S_ISREG(fileInfo.st_mode));
	DBGPRINTF("process_symlink:  found '%s', File: %d (config file: %d), symlink: %d\n",
		target, is_file, chld->is_file, 0);
	if (act_obj_add(chld, target, is_file, fileInfo.st_ino, 0, symlink) == RS_RET_OK) {
		/* need to watch parent target as well for proper rotation support */
		uint idx = ustrlen(chld->active->name) - ustrlen(chld->active->basename);
		if (idx) { /* basename is different from name */
			char parent[MAXFNAME];
			idx--; /* move past trailing slash */
			memcpy(parent, chld->active->name, idx);
			parent[idx] = '\0';
			if(lstat(parent, &fileInfo) != 0) {
				LogError(errno, RS_RET_ERR,
					"imfile: process_symlink: cannot stat directory '%s' - ignored", parent);
				FINALIZE;
			}
			if (chld->parent->root->edges) {
				DBGPRINTF("process_symlink: adding parent '%s' of target '%s'\n", parent, target);
				act_obj_add(chld->parent->root->edges, parent, 0, fileInfo.st_ino, 0, NULL);
			}
		}
	}

finalize_it:
	free(target);
	RETiRet;
}

static void ATTR_NONNULL()
poll_tree(fs_edge_t *const chld)
{
	struct stat fileInfo;
	glob_t files;
	int need_globfree = 0;
	int issymlink;
	DBGPRINTF("poll_tree: chld %p, name '%s', path: %s\n", chld, chld->name, chld->path);
	detect_updates(chld);
	const int ret = glob((char*)chld->path, runModConf->sortFiles|GLOB_BRACE, NULL, &files);
	need_globfree = 1;
	DBGPRINTF("poll_tree: glob returned %d\n", ret);
	if(ret == 0) {
		DBGPRINTF("poll_tree: processing %d files\n", (int) files.gl_pathc);
		for(unsigned i = 0 ; i < files.gl_pathc ; i++) {
			if(glbl.GetGlobalInputTermState() != 0) {
				goto done;
			}
			char *const file = files.gl_pathv[i];
			if(lstat(file, &fileInfo) != 0) {
				LogError(errno, RS_RET_ERR,
					"imfile: poll_tree cannot stat file '%s' - ignored", file);
				continue;
			}

			if (S_ISLNK(fileInfo.st_mode)) {
				rsRetVal slink_ret = process_symlink(chld, file);
				if (slink_ret != RS_RET_OK) {
					continue;
				}
				issymlink = 1;
			} else {
				issymlink = 0;
			}
			const int is_file = (S_ISREG(fileInfo.st_mode) || issymlink);
			DBGPRINTF("poll_tree:  found '%s', File: %d (config file: %d), symlink: %d\n",
				file, is_file, chld->is_file, issymlink);
			if(!is_file && S_ISREG(fileInfo.st_mode)) {
				LogMsg(0, RS_RET_ERR, LOG_WARNING,
					"imfile: '%s' is neither a regular file, symlink, nor a "
					"directory - ignored", file);
				continue;
			}
			if(chld->is_file != is_file) {
				LogMsg(0, RS_RET_ERR, LOG_WARNING,
					"imfile: '%s' is %s but %s expected - ignored",
					file, (is_file) ? "FILE" : "DIRECTORY",
					(chld->is_file) ? "FILE" : "DIRECTORY");
				continue;
			}
			act_obj_add(chld, file, is_file, fileInfo.st_ino, issymlink, NULL);
		}
	}

	poll_active_files(chld);

done:
	if(need_globfree) {
		globfree(&files);
	}
	return;
}

#ifdef HAVE_INOTIFY_INIT // TODO: shouldn't we use that in polling as well?
static void ATTR_NONNULL()
poll_timeouts(fs_edge_t *const edge)
{
	if(edge->is_file) {
		act_obj_t *act;
		for(act = edge->active ; act != NULL ; act = act->next) {
			if(strmReadMultiLine_isTimedOut(act->pStrm)) {
				DBGPRINTF("timeout occured on %s\n", act->name);
				pollFile(act);
			}
		}
	}
}
#endif


/* destruct a single act_obj object */
static void
act_obj_destroy(act_obj_t *const act, const int is_deleted)
{
	uchar *statefn;
	uchar statefile[MAXFNAME];
	uchar toDel[MAXFNAME];

	if(act == NULL)
		return;

	DBGPRINTF("act_obj_destroy: act %p '%s' (source '%s'), wd %d, pStrm %p, is_deleted %d, in_move %d\n",
		act, act->name, act->source_name? act->source_name : "---", act->wd, act->pStrm, is_deleted,
		act->in_move);
	if(act->is_symlink && is_deleted) {
		act_obj_t *target_act;
		for(target_act = act->edge->active ; target_act != NULL ; target_act = target_act->next) {
			if(target_act->source_name && !strcmp(target_act->source_name, act->name)) {
				DBGPRINTF("act_obj_destroy: unlinking slink target %s of %s "
						"symlink\n", target_act->name, act->name);
				act_obj_unlink(target_act);
				break;
			}
		}
	}
	if(act->ratelimiter != NULL) {
		ratelimitDestruct(act->ratelimiter);
	}
	if(act->pStrm != NULL) {
		const instanceConf_t *const inst = act->edge->instarr[0];// TODO: same file, multiple instances?
		pollFile(act); /* get any left-over data */
		if(inst->bRMStateOnDel) {
			statefn = getStateFileName(act, statefile, sizeof(statefile));
			getFullStateFileName(statefn, "", toDel, sizeof(toDel)); // TODO: check!
			statefn = toDel;
		}
		persistStrmState(act);
		strm.Destruct(&act->pStrm);
		/* we delete state file after destruct in case strm obj initiated a write */
		if(is_deleted && !act->in_move && inst->bRMStateOnDel) {
			DBGPRINTF("act_obj_destroy: deleting state file %s\n", statefn);
			unlink((char*)statefn);
		}
	}
	#ifdef HAVE_INOTIFY_INIT
	if(act->wd != -1) {
		wdmapDel(act->wd);
	}
	#endif
	#if defined(OS_SOLARIS) && defined (HAVE_PORT_SOURCE_FILE)
	if(act->pfinf != NULL) {
		free(act->pfinf->fobj.fo_name);
		free(act->pfinf);
	}
	#endif
	free(act->basename);
	free(act->source_name);
	//free(act->statefile);
	free(act->multiSub.ppMsgs);
	#if defined(OS_SOLARIS) && defined (HAVE_PORT_SOURCE_FILE)
		act->is_deleted = 1;
	#else
		free(act->name);
		free(act);
	#endif
}


/* destroy complete act list starting at given node */
static void
act_obj_destroy_all(act_obj_t *act)
{
	if(act == NULL)
		return;

	DBGPRINTF("act_obj_destroy_all: act %p '%s', wd %d, pStrm %p\n", act, act->name, act->wd, act->pStrm);
	while(act != NULL) {
		act_obj_t *const toDel = act;
		act = act->next;
		act_obj_destroy(toDel, 0);
	}
}

#if 0
/* debug: find if ptr is still present in list */
static void
chk_active(const act_obj_t *act, const act_obj_t *const deleted)
{
	while(act != NULL) {
		DBGPRINTF("chk_active %p vs %p\n", act, deleted);
		if(act->prev == deleted)
			DBGPRINTF("chk_active %p prev points to %p\n", act, deleted);
		if(act->next == deleted)
			DBGPRINTF("chk_active %p next points to %p\n", act, deleted);
		act = act->next;
		DBGPRINTF("chk_active next %p\n", act);
	}
}
#endif

/* unlink act object from linked list and then
 * destruct it.
 */
static void //ATTR_NONNULL()
act_obj_unlink(act_obj_t *act)
{
	DBGPRINTF("act_obj_unlink %p: %s\n", act, act->name);
	if(act->prev == NULL) {
		act->edge->active = act->next;
	} else {
		act->prev->next = act->next;
	}
	if(act->next != NULL) {
		act->next->prev = act->prev;
	}
	act_obj_destroy(act, 1);
	act = NULL;
//dbgprintf("printout of fs tree post unlink\n");
//fs_node_print(runModConf->conf_tree, 0);
//dbg_wdmapPrint("wdmap after");
}

static void
fs_node_destroy(fs_node_t *const node)
{
	fs_edge_t *edge;
	DBGPRINTF("node destroy: %p edges:\n", node);

	for(edge = node->edges ; edge != NULL ; ) {
		fs_node_destroy(edge->node);
		fs_edge_t *const toDel = edge;
		edge = edge->next;
		act_obj_destroy_all(toDel->active);
		free(toDel->name);
		free(toDel->path);
		free(toDel->instarr);
		free(toDel);
	}
	free(node);
}

static void ATTR_NONNULL(1, 2)
fs_node_walk(fs_node_t *const node,
	void (*f_usr)(fs_edge_t*const))
{
	DBGPRINTF("node walk: %p edges:\n", node);

	fs_edge_t *edge;
	for(edge = node->edges ; edge != NULL ; edge = edge->next) {
		DBGPRINTF("node walk: child %p '%s'\n", edge->node, edge->name);
		f_usr(edge);
		fs_node_walk(edge->node, f_usr);
	}
}



/* add a file system object to config tree (or update existing node with new monitor)
 */
static rsRetVal
fs_node_add(fs_node_t *const node,
	fs_node_t *const source,
	const uchar *const toFind,
	const size_t pathIdx,
	instanceConf_t *const inst)
{
	DEFiRet;
	fs_edge_t *newchld = NULL;
	int i;

	DBGPRINTF("fs_node_add(%p, '%s') enter, idx %zd\n",
		node, toFind+pathIdx, pathIdx);
	assert(toFind[0] != '\0');
	for(i = pathIdx ; (toFind[i] != '\0') && (toFind[i] != '/') ; ++i)
		/*JUST SKIP*/;
	const int isFile = (toFind[i] == '\0') ? 1 : 0;
	uchar ourPath[PATH_MAX];
	if(i == 0) {
		ourPath[0] = '/';
		ourPath[1] = '\0';
	} else {
		memcpy(ourPath, toFind, i);
		ourPath[i] = '\0';
	}
	const size_t nextPathIdx = i+1;
	const size_t len = i - pathIdx;
	uchar name[PATH_MAX];
	memcpy(name, toFind+pathIdx, len);
	name[len] = '\0';
	DBGPRINTF("fs_node_add: name '%s'\n", name);
	node->root = source;

	fs_edge_t *chld;
	for(chld = node->edges ; chld != NULL ; chld = chld->next) {
		if(!ustrcmp(chld->name, name)) {
			DBGPRINTF("fs_node_add(%p, '%s') found '%s'\n", chld->node, toFind, name);
			/* add new instance */
			chld->ninst++;
			CHKmalloc(chld->instarr = realloc(chld->instarr, sizeof(instanceConf_t*) * chld->ninst));
			chld->instarr[chld->ninst-1] = inst;
			/* recurse */
			if(!isFile) {
				CHKiRet(fs_node_add(chld->node, node, toFind, nextPathIdx, inst));
			}
			FINALIZE;
		}
	}

	/* could not find node --> add it */
	DBGPRINTF("fs_node_add(%p, '%s') did not find '%s' - adding it\n",
		node, toFind, name);
	CHKmalloc(newchld = calloc(sizeof(fs_edge_t), 1));
	CHKmalloc(newchld->name = ustrdup(name));
	CHKmalloc(newchld->node = calloc(sizeof(fs_node_t), 1));
	CHKmalloc(newchld->path = ustrdup(ourPath));
	CHKmalloc(newchld->instarr = calloc(sizeof(instanceConf_t*), 1));
	newchld->instarr[0] = inst;
	newchld->is_file = isFile;
	newchld->ninst = 1;
	newchld->parent = node;

	DBGPRINTF("fs_node_add(%p, '%s') returns %p\n", node, toFind, newchld->node);

	if(!isFile) {
		CHKiRet(fs_node_add(newchld->node, node, toFind, nextPathIdx, inst));
	}

	/* link to list */
	newchld->next = node->edges;
	node->edges = newchld;
finalize_it:
	if(iRet != RS_RET_OK) {
		if(newchld != NULL) {
		free(newchld->name);
		free(newchld->node);
		free(newchld->path);
		free(newchld->instarr);
		free(newchld);
		}
	}
	RETiRet;
}

#if 0 //TODO: check if we need (specialised?) versions of this?
/* we receive a notification that a new object is found *beneath*
 * act. This function now finds the right spot to place it and the
 * activate the monitor.
 * TODO: think if it is worth optimizing this based on the inotify-provided
 * name. But it's complex in any case...
 */
static rsRetVal ATTR_NONNULL(1, 2)
fs_node_notify_new_obj(act_obj_t *const act, const char *const name)
{
	DBGPRINTF("fs_node_notify_new_obj: act->name '%s', name '%s'\n",
		act->name, name);
#if 0
	char fullname[MAXFNAME];
	snprintf(fullname, MAXFNAME, "%s/%s", act->name, name);
//	act_obj_add(act->edge->node, fullname, 0);
#endif
	fs_node_walk(act->edge->node, poll_tree);
	return RS_RET_OK;
}

static rsRetVal ATTR_NONNULL(1, 2)
fs_node_notify_file_del(act_obj_t *const act, const char *const name)
{
	DBGPRINTF("fs_node_notify_file_del: act->name '%s', name '%s'\n",
		act->name, name);
	fs_node_walk(act->edge->parent, poll_tree);
	// TODO: 1. impl: walk tree, 2. impl: use inotify name
	return RS_RET_OK;
}
#endif


/* Helper function to combine statefile and workdir
 * This function is guranteed to work only on config data and DOES NOT
 * open or otherwise modify disk file state.
 */
static int ATTR_NONNULL()
getFullStateFileName(const uchar *const pszstatefile,
	const char *const file_id,
	uchar *const pszout,
	const size_t ilenout)
{
	int lenout;
	const uchar* pszworkdir;

	/* Get Raw Workdir, if it is NULL we need to propper handle it */
	pszworkdir = glblGetWorkDirRaw();

	/* Construct file name */
	lenout = snprintf((char*)pszout, ilenout, "%s/%s%s%s",
		(char*) (pszworkdir == NULL ? "." : (char*) pszworkdir), (char*)pszstatefile,
		(*file_id == '\0') ? "" : ":", file_id);

	/* return out length */
	return lenout;
}


/* hash function for file-id
 * Takes a block of data and returns a string with the hash value.
 *
 * Currently one provided by Aaaron Wiebe based on perl's hashing algorithm
 * (so probably pretty generic). Not for excessively large strings!
 * TODO: re-think the hash function!
 */
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wunknown-attributes"
#endif
static void __attribute__((nonnull(1,3)))
#if defined(__clang__)
__attribute__((no_sanitize("unsigned-integer-overflow")))
#endif
get_file_id_hash(const char *data, size_t lendata,
	char *const hash_str, const size_t len_hash_str)
{
	assert(len_hash_str >= 17); /* we always generate 8-byte strings */

	size_t i;
	uint8_t out[8], k[16];
	for (i = 0; i < 16; ++i)
		k[i] = i;
	memset(out, 0, sizeof(out));
	rs_siphash((const uint8_t *)data, lendata, k, out, 8);

	for(i = 0 ; i < 8 ; ++i) {
		if(2 * i+1 >= len_hash_str)
			break;
		snprintf(hash_str+(2*i), 3, "%2.2x", out[i]);
	}
}


/* this returns the file-id for a given file
 */
static void ATTR_NONNULL(1, 2)
getFileID(const act_obj_t *const act, char *const buf, const size_t lenbuf)
{
	*buf = '\0'; /* default: empty hash, only set if file has sufficient data */
	const int fd = open(act->name, O_RDONLY | O_CLOEXEC);
	if(fd >= 0) {
		char filedata[FILE_ID_SIZE];
		const int r = read(fd, filedata, FILE_ID_SIZE);
		if(r == FILE_ID_SIZE) {
			get_file_id_hash(filedata, sizeof(filedata), buf, lenbuf);
		} else {
			DBGPRINTF("getFileID partial or error read, ret %d\n", r);
		}
		close(fd);
	} else {
		DBGPRINTF("getFileID open %s failed\n", act->name);
	}
	DBGPRINTF("getFileID for '%s', file_id_hash '%s'\n", act->name, buf);
}

/* this generates a state file name suitable for the given file. To avoid
 * malloc calls, it must be passed a buffer which should be MAXFNAME large.
 * Note: the buffer is not necessarily populated ... always ONLY use the
 * RETURN VALUE!
 * This function is guranteed to work only on config data and DOES NOT
 * open or otherwise modify disk file state.
 */
static uchar * ATTR_NONNULL(1, 2)
getStateFileName(const act_obj_t *const act,
	 	 uchar *const __restrict__ buf,
		 const size_t lenbuf)
{
	DBGPRINTF("getStateFileName for '%s'\n", act->name);
	snprintf((char*)buf, lenbuf - 1, "imfile-state:%lld", (long long) act->ino);
	DBGPRINTF("getStateFileName:  stat file name now is %s\n", buf);
	return buf;
}


/* enqueue the read file line as a message. The provided string is
 * not freed - this must be done by the caller.
 */
#define MAX_OFFSET_REPRESENTATION_NUM_BYTES 20
static rsRetVal ATTR_NONNULL(1,2)
enqLine(act_obj_t *const act,
	cstr_t *const __restrict__ cstrLine,
	const int64 strtOffs)
{
	DEFiRet;
	const instanceConf_t *const inst = act->edge->instarr[0];// TODO: same file, multiple instances?
	smsg_t *pMsg;
	uchar file_offset[MAX_OFFSET_REPRESENTATION_NUM_BYTES+1];
	const uchar *metadata_names[2] = {(uchar *)"filename",(uchar *)"fileoffset"} ;
	const uchar *metadata_values[2] ;
	const size_t msgLen = cstrLen(cstrLine);

	if(msgLen == 0) {
		/* we do not process empty lines */
		FINALIZE;
	}

	CHKiRet(msgConstruct(&pMsg));
	MsgSetFlowControlType(pMsg, eFLOWCTL_FULL_DELAY);
	MsgSetInputName(pMsg, pInputName);
	if(inst->addCeeTag) {
		/* Make sure we account for terminating null byte */
		size_t ceeMsgSize = msgLen + CONST_LEN_CEE_COOKIE + 1;
		char *ceeMsg;
		CHKmalloc(ceeMsg = MALLOC(ceeMsgSize));
		strcpy(ceeMsg, CONST_CEE_COOKIE);
		strcat(ceeMsg, (char*)rsCStrGetSzStrNoNULL(cstrLine));
		MsgSetRawMsg(pMsg, ceeMsg, ceeMsgSize);
		free(ceeMsg);
	} else {
		MsgSetRawMsg(pMsg, (char*)rsCStrGetSzStrNoNULL(cstrLine), msgLen);
	}
	MsgSetMSGoffs(pMsg, 0);	/* we do not have a header... */
	MsgSetHOSTNAME(pMsg, glbl.GetLocalHostName(), ustrlen(glbl.GetLocalHostName()));
	MsgSetTAG(pMsg, inst->pszTag, inst->lenTag);
	msgSetPRI(pMsg, inst->iFacility | inst->iSeverity);
	MsgSetRuleset(pMsg, inst->pBindRuleset);
	if(inst->addMetadata) {
		if (act->source_name) {
			metadata_values[0] = (const uchar*)act->source_name;
		} else {
			metadata_values[0] = (const uchar*)act->name;
		}
		snprintf((char *)file_offset, MAX_OFFSET_REPRESENTATION_NUM_BYTES+1, "%lld", strtOffs);
		metadata_values[1] = file_offset;
		msgAddMultiMetadata(pMsg, metadata_names, metadata_values, 2);
	}

	if(inst->delay_perMsg) {
		srSleep(inst->delay_perMsg % 1000000, inst->delay_perMsg / 1000000);
	}

	ratelimitAddMsg(act->ratelimiter, &act->multiSub, pMsg);
finalize_it:
	RETiRet;
}
/* try to open a file which has a state file. If the state file does not
 * exist or cannot be read, an error is returned.
 */
static rsRetVal ATTR_NONNULL(1)
openFileWithStateFile(act_obj_t *const act)
{
	DEFiRet;
	uchar pszSFNam[MAXFNAME];
	uchar statefile[MAXFNAME];
	char file_id[FILE_ID_HASH_SIZE];
	int fd = -1;
	const instanceConf_t *const inst = act->edge->instarr[0];// TODO: same file, multiple instances?

	uchar *const statefn = getStateFileName(act, statefile, sizeof(statefile));
	getFileID(act, file_id, sizeof(file_id));

	getFullStateFileName(statefn, file_id, pszSFNam, sizeof(pszSFNam));
	DBGPRINTF("trying to open state for '%s', state file '%s'\n", act->name, pszSFNam);

	/* check if the file exists */
	fd = open((char*)pszSFNam, O_CLOEXEC | O_NOCTTY | O_RDONLY, 0600);
	if(fd < 0) {
		if(errno == ENOENT) {
			if(file_id[0] != '\0') {
				const char *pszSFNamHash = strdup((const char*)pszSFNam);
				CHKmalloc(pszSFNamHash);
				DBGPRINTF("state file %s for %s does not exist - trying to see if "
					"inode-only file exists\n", pszSFNam, act->name);
				getFullStateFileName(statefn, "", pszSFNam, sizeof(pszSFNam));
				fd = open((char*)pszSFNam, O_CLOEXEC | O_NOCTTY | O_RDONLY, 0600);
				if(fd >= 0) {
					/* we now can use identify the file, so let's rename it */
					if(rename((const char*)pszSFNam, pszSFNamHash) != 0) {
						LogError(errno, RS_RET_IO_ERROR,
							"imfile error trying to rename state file for '%s' - "
							"ignoring this error, usually this means a file no "
							"longer file is left over, but this may also cause "
							"some real trouble. Still the best we can do ",
							act->name);
						free((void*) pszSFNamHash);
						ABORT_FINALIZE(RS_RET_IO_ERROR);
					}
				}
				free((void*) pszSFNamHash);
			}
			if(fd < 0) {
				DBGPRINTF("state file %s for %s does not exist - trying to see if "
					"old-style file exists\n", pszSFNam, act->name);
				CHKiRet(OLD_openFileWithStateFile(act));
				FINALIZE;
			}
		} else {
			LogError(errno, RS_RET_IO_ERROR,
				"imfile error trying to access state file for '%s'",
			        act->name);
			ABORT_FINALIZE(RS_RET_IO_ERROR);
		}
	}

	DBGPRINTF("opened state file %s for %s\n", pszSFNam, act->name);
	CHKiRet(strm.Construct(&act->pStrm));

	struct json_object *jval;
	struct json_object *json = fjson_object_from_fd(fd);
	if(json == NULL) {
		LogError(0, RS_RET_ERR, "imfile: error reading state file for '%s'", act->name);
	}

	/* we access some data items a bit dirty, as we need to refactor the whole
	 * thing in any case - TODO
	 */
	/* Note: we ignore filname property - it is just an aid to the user. Most
	 * importantly it *is wrong* after a file move!
	 */
	fjson_object_object_get_ex(json, "prev_was_nl", &jval);
	act->pStrm->bPrevWasNL = fjson_object_get_int(jval);

	fjson_object_object_get_ex(json, "curr_offs", &jval);
	act->pStrm->iCurrOffs = fjson_object_get_int64(jval);

	fjson_object_object_get_ex(json, "strt_offs", &jval);
	act->pStrm->strtOffs = fjson_object_get_int64(jval);

	fjson_object_object_get_ex(json, "prev_line_segment", &jval);
	const uchar *const prev_line_segment = (const uchar*)fjson_object_get_string(jval);
	if(jval != NULL) {
		CHKiRet(rsCStrConstructFromszStr(&act->pStrm->prevLineSegment, prev_line_segment));
		cstrFinalize(act->pStrm->prevLineSegment);
		uchar *ret = rsCStrGetSzStrNoNULL(act->pStrm->prevLineSegment);
		DBGPRINTF("prev_line_segment present in state file 2, is: %s\n", ret);
	}

	fjson_object_object_get_ex(json, "prev_msg_segment", &jval);
	const uchar *const prev_msg_segment = (const uchar*)fjson_object_get_string(jval);
	if(jval != NULL) {
		CHKiRet(rsCStrConstructFromszStr(&act->pStrm->prevMsgSegment, prev_msg_segment));
		cstrFinalize(act->pStrm->prevMsgSegment);
		uchar *ret = rsCStrGetSzStrNoNULL(act->pStrm->prevMsgSegment);
		DBGPRINTF("prev_msg_segment present in state file 2, is: %s\n", ret);
	}
	fjson_object_put(json);

	CHKiRet(strm.SetFName(act->pStrm, (uchar*)act->name, strlen(act->name)));
	CHKiRet(strm.SettOperationsMode(act->pStrm, STREAMMODE_READ));
	CHKiRet(strm.SetsType(act->pStrm, STREAMTYPE_FILE_MONITOR));
	CHKiRet(strm.SetFileNotFoundError(act->pStrm, inst->fileNotFoundError));
	CHKiRet(strm.ConstructFinalize(act->pStrm));

	CHKiRet(strm.SeekCurrOffs(act->pStrm));

finalize_it:
	if(fd >= 0) {
		close(fd);
	}
	RETiRet;
}

/* try to open a file for which no state file exists. This function does NOT
 * check if a state file actually exists or not -- this must have been
 * checked before calling it.
 */
static rsRetVal
openFileWithoutStateFile(act_obj_t *const act)
{
	DEFiRet;
	struct stat stat_buf;

	const instanceConf_t *const inst = act->edge->instarr[0];// TODO: same file, multiple instances?

	DBGPRINTF("clean startup withOUT state file for '%s'\n", act->name);
	if(act->pStrm != NULL)
		strm.Destruct(&act->pStrm);
	CHKiRet(strm.Construct(&act->pStrm));
	CHKiRet(strm.SettOperationsMode(act->pStrm, STREAMMODE_READ));
	CHKiRet(strm.SetsType(act->pStrm, STREAMTYPE_FILE_MONITOR));
	CHKiRet(strm.SetFName(act->pStrm, (uchar*)act->name, strlen(act->name)));
	CHKiRet(strm.SetFileNotFoundError(act->pStrm, inst->fileNotFoundError));
	CHKiRet(strm.ConstructFinalize(act->pStrm));

	/* As a state file not exist, this is a fresh start. seek to file end
	 * when freshStartTail is on.
	 */
	if(inst->freshStartTail){
		if(stat((char*) act->name, &stat_buf) != -1) {
			act->pStrm->iCurrOffs = stat_buf.st_size;
			CHKiRet(strm.SeekCurrOffs(act->pStrm));
		}
	}

finalize_it:
	RETiRet;
}

/* try to open a file. This involves checking if there is a status file and,
 * if so, reading it in. Processing continues from the last known location.
 */
static rsRetVal
openFile(act_obj_t *const act)
{
	DEFiRet;
	const instanceConf_t *const inst = act->edge->instarr[0];// TODO: same file, multiple instances?

	CHKiRet_Hdlr(openFileWithStateFile(act)) {
		CHKiRet(openFileWithoutStateFile(act));
	}

	DBGPRINTF("breopenOnTruncate %d for '%s'\n", inst->reopenOnTruncate, act->name);
	CHKiRet(strm.SetbReopenOnTruncate(act->pStrm, inst->reopenOnTruncate));
	strmSetReadTimeout(act->pStrm, inst->readTimeout);

finalize_it:
	RETiRet;
}


/* The following is a cancel cleanup handler for strmReadLine(). It is necessary in case
 * strmReadLine() is cancelled while processing the stream. -- rgerhards, 2008-03-27
 */
static void pollFileCancelCleanup(void *pArg)
{
	BEGINfunc;
	cstr_t **ppCStr = (cstr_t**) pArg;
	if(*ppCStr != NULL)
		rsCStrDestruct(ppCStr);
	ENDfunc;
}


/* pollFile needs to be split due to the unfortunate pthread_cancel_push() macros. */
static rsRetVal ATTR_NONNULL()
pollFileReal(act_obj_t *act, cstr_t **pCStr)
{
	int64 strtOffs;
	DEFiRet;
	int nProcessed = 0;
	regex_t *start_preg = NULL, *end_preg = NULL;

	DBGPRINTF("pollFileReal enter, pStrm %p, name '%s'\n", act->pStrm, act->name);
	DBGPRINTF("pollFileReal enter, edge %p\n", act->edge);
	DBGPRINTF("pollFileReal enter, edge->instarr %p\n", act->edge->instarr);

	instanceConf_t *const inst = act->edge->instarr[0];// TODO: same file, multiple instances?

	if(act->pStrm == NULL) {
		CHKiRet(openFile(act)); /* open file */
	}

	start_preg = (inst->startRegex == NULL) ? NULL : &inst->start_preg;
	end_preg = (inst->endRegex == NULL) ? NULL : &inst->end_preg;

	/* loop below will be exited when strmReadLine() returns EOF */
	while(glbl.GetGlobalInputTermState() == 0) {
		if(inst->maxLinesAtOnce != 0 && nProcessed >= inst->maxLinesAtOnce)
			break;
		if((start_preg == NULL) && (end_preg == NULL)) {
			CHKiRet(strm.ReadLine(act->pStrm, pCStr, inst->readMode, inst->escapeLF,
				inst->trimLineOverBytes, &strtOffs));
		} else {
			CHKiRet(strmReadMultiLine(act->pStrm, pCStr, start_preg, end_preg,
				inst->escapeLF, inst->discardTruncatedMsg, inst->msgDiscardingError, &strtOffs));
		}
		++nProcessed;
		runModConf->bHadFileData = 1; /* this is just a flag, so set it and forget it */
		CHKiRet(enqLine(act, *pCStr, strtOffs)); /* process line */
		rsCStrDestruct(pCStr); /* discard string (must be done by us!) */
		if(inst->iPersistStateInterval > 0 && ++act->nRecords >= inst->iPersistStateInterval) {
			persistStrmState(act);
			act->nRecords = 0;
		}
	}

finalize_it:
	multiSubmitFlush(&act->multiSub);

	if(*pCStr != NULL) {
		rsCStrDestruct(pCStr);
	}

	RETiRet;
}

/* poll a file, need to check file rollover etc. open file if not open */
static rsRetVal ATTR_NONNULL(1)
pollFile(act_obj_t *const act)
{
	cstr_t *pCStr = NULL;
	DEFiRet;
	if (act->is_symlink) {
		FINALIZE;    /* no reason to poll symlink file */
	}
	/* Note: we must do pthread_cleanup_push() immediately, because the POSIX macros
	 * otherwise do not work if I include the _cleanup_pop() inside an if... -- rgerhards, 2008-08-14
	 */
	pthread_cleanup_push(pollFileCancelCleanup, &pCStr);
	iRet = pollFileReal(act, &pCStr);
	pthread_cleanup_pop(0);
finalize_it: RETiRet;
}


/* create input instance, set default parameters, and
 * add it to the list of instances.
 */
static rsRetVal ATTR_NONNULL(1)
createInstance(instanceConf_t **const pinst)
{
	instanceConf_t *inst;
	DEFiRet;
	CHKmalloc(inst = MALLOC(sizeof(instanceConf_t)));
	inst->next = NULL;
	inst->pBindRuleset = NULL;

	inst->pszBindRuleset = NULL;
	inst->pszFileName = NULL;
	inst->pszTag = NULL;
	inst->pszStateFile = NULL;
	inst->nMultiSub = NUM_MULTISUB;
	inst->iSeverity = 5;
	inst->iFacility = 128;
	inst->maxLinesAtOnce = 0;
	inst->trimLineOverBytes = 0;
	inst->iPersistStateInterval = 0;
	inst->readMode = 0;
	inst->startRegex = NULL;
	inst->endRegex = NULL;
	inst->discardTruncatedMsg = 0;
	inst->msgDiscardingError = 1;
	inst->bRMStateOnDel = 1;
	inst->escapeLF = 1;
	inst->reopenOnTruncate = 0;
	inst->addMetadata = ADD_METADATA_UNSPECIFIED;
	inst->addCeeTag = 0;
	inst->freshStartTail = 0;
	inst->fileNotFoundError = 1;
	inst->readTimeout = loadModConf->readTimeout;
	inst->delay_perMsg = 0;

	/* node created, let's add to config */
	if(loadModConf->tail == NULL) {
		loadModConf->tail = loadModConf->root = inst;
	} else {
		loadModConf->tail->next = inst;
		loadModConf->tail = inst;
	}

	*pinst = inst;
finalize_it:
	RETiRet;
}


/* the basen(ame) buffer must be of size MAXFNAME
 * returns the index of the slash in front of basename
 */
static int ATTR_NONNULL()
getBasename(uchar *const __restrict__ basen, uchar *const __restrict__ path)
{
	int i;
	int found = 0;
	const int lenName = ustrlen(path);
	for(i = lenName ; i >= 0 ; --i) {
		if(path[i] == '/') {
			/* found basename component */
			found = 1;
			if(i == lenName)
				basen[0] = '\0';
			else {
				memcpy(basen, path+i+1, lenName-i);
			}
			break;
		}
	}
	if (found == 1)
		return i;
	else {
		return -1;
	}
}

/* this function checks instance parameters and does some required pre-processing
 */
static rsRetVal ATTR_NONNULL()
checkInstance(instanceConf_t *const inst)
{
	uchar curr_wd[MAXFNAME];
	DEFiRet;

	/* this is primarily for the clang static analyzer, but also
	 * guards against logic errors in the config handler.
	 */
	if(inst->pszFileName == NULL)
		ABORT_FINALIZE(RS_RET_INTERNAL_ERROR);

	CHKmalloc(inst->pszFileName_forOldStateFile = ustrdup(inst->pszFileName));
	if(loadModConf->normalizePath) {
		if(inst->pszFileName[0] == '.' && inst->pszFileName[1] == '/') {
			DBGPRINTF("imfile: removing heading './' from name '%s'\n", inst->pszFileName);
			memmove(inst->pszFileName, inst->pszFileName+2, ustrlen(inst->pszFileName) - 1);
		}

		if(inst->pszFileName[0] != '/') {
			if(getcwd((char*)curr_wd, MAXFNAME) == NULL || curr_wd[0] != '/') {
				LogError(errno, RS_RET_ERR, "imfile: error querying current working "
					"directory - can not continue with %s", inst->pszFileName);
				ABORT_FINALIZE(RS_RET_ERR);
			}
			const size_t len_curr_wd = ustrlen(curr_wd);
			if(len_curr_wd + ustrlen(inst->pszFileName) + 1 >= MAXFNAME) {
				LogError(0, RS_RET_ERR, "imfile: length of configured file and current "
					"working directory exceeds permitted size - ignoring %s",
					inst->pszFileName);
				ABORT_FINALIZE(RS_RET_ERR);
			}
			curr_wd[len_curr_wd] = '/';
			strcpy((char*)curr_wd+len_curr_wd+1, (char*)inst->pszFileName);
			free(inst->pszFileName);
			CHKmalloc(inst->pszFileName = ustrdup(curr_wd));
		}
	}
	dbgprintf("imfile: adding file monitor for '%s'\n", inst->pszFileName);

	if(inst->pszTag != NULL) {
		inst->lenTag = ustrlen(inst->pszTag);
	}
finalize_it:
	RETiRet;
}


/* add a new monitor */
static rsRetVal
addInstance(void __attribute__((unused)) *pVal, uchar *pNewVal)
{
	instanceConf_t *inst;
	DEFiRet;

	if(cs.pszFileName == NULL) {
		LogError(0, RS_RET_CONFIG_ERROR, "imfile error: no file name given, file monitor can "
					"not be created");
		ABORT_FINALIZE(RS_RET_CONFIG_ERROR);
	}
	if(cs.pszFileTag == NULL) {
		LogError(0, RS_RET_CONFIG_ERROR, "imfile error: no tag value given, file monitor can "
					"not be created");
		ABORT_FINALIZE(RS_RET_CONFIG_ERROR);
	}

	CHKiRet(createInstance(&inst));
	if((cs.pszBindRuleset == NULL) || (cs.pszBindRuleset[0] == '\0')) {
		inst->pszBindRuleset = NULL;
	} else {
		CHKmalloc(inst->pszBindRuleset = ustrdup(cs.pszBindRuleset));
	}
	CHKmalloc(inst->pszFileName = ustrdup((char*) cs.pszFileName));
	CHKmalloc(inst->pszTag = ustrdup((char*) cs.pszFileTag));
	if(cs.pszStateFile == NULL) {
		inst->pszStateFile = NULL;
	} else {
		CHKmalloc(inst->pszStateFile = ustrdup(cs.pszStateFile));
	}
	inst->iSeverity = cs.iSeverity;
	inst->iFacility = cs.iFacility;
	if(cs.maxLinesAtOnce) {
		if(loadModConf->opMode == OPMODE_INOTIFY) {
			LogError(0, RS_RET_PARAM_NOT_PERMITTED,
				"parameter \"maxLinesAtOnce\" not "
				"permited in inotify mode - ignored");
		} else {
			inst->maxLinesAtOnce = cs.maxLinesAtOnce;
		}
	}
	inst->trimLineOverBytes = cs.trimLineOverBytes;
	inst->iPersistStateInterval = cs.iPersistStateInterval;
	inst->readMode = cs.readMode;
	inst->escapeLF = 0;
	inst->reopenOnTruncate = 0;
	inst->addMetadata = 0;
	inst->addCeeTag = 0;
	inst->bRMStateOnDel = 0;
	inst->readTimeout = loadModConf->readTimeout;

	CHKiRet(checkInstance(inst));

	/* reset legacy system */
	cs.iPersistStateInterval = 0;
	resetConfigVariables(NULL, NULL); /* values are both dummies */

finalize_it:
	free(pNewVal); /* we do not need it, but we must free it! */
	RETiRet;
}


BEGINnewInpInst
	struct cnfparamvals *pvals;
	instanceConf_t *inst;
	int i;
CODESTARTnewInpInst
	DBGPRINTF("newInpInst (imfile)\n");

	pvals = nvlstGetParams(lst, &inppblk, NULL);
	if(pvals == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		DBGPRINTF("input param blk in imfile:\n");
		cnfparamsPrint(&inppblk, pvals);
	}

	CHKiRet(createInstance(&inst));

	for(i = 0 ; i < inppblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(inppblk.descr[i].name, "file")) {
			inst->pszFileName = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "statefile")) {
			inst->pszStateFile = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "removestateondelete")) {
			inst->bRMStateOnDel = (uint8_t) pvals[i].val.d.n; // TODO: duplicate!
		} else if(!strcmp(inppblk.descr[i].name, "tag")) {
			inst->pszTag = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "ruleset")) {
			inst->pszBindRuleset = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "severity")) {
			inst->iSeverity = pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "facility")) {
			inst->iFacility = pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "readmode")) {
			inst->readMode = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "startmsg.regex")) {
			inst->startRegex = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "endmsg.regex")) {
			inst->endRegex = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else if(!strcmp(inppblk.descr[i].name, "discardtruncatedmsg")) {
			inst->discardTruncatedMsg = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "msgdiscardingerror")) {
			inst->msgDiscardingError = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "deletestateonfiledelete")) {
			inst->bRMStateOnDel = (sbool) pvals[i].val.d.n; // TODO: duplicate!
		} else if(!strcmp(inppblk.descr[i].name, "addmetadata")) {
			inst->addMetadata = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "delay.message")) {
			inst->delay_perMsg = (unsigned) pvals[i].val.d.n;
		} else if (!strcmp(inppblk.descr[i].name, "addceetag")) {
			inst->addCeeTag = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "freshstarttail")) {
			inst->freshStartTail = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "filenotfounderror")) {
			inst->fileNotFoundError = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "escapelf")) {
			inst->escapeLF = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "reopenontruncate")) {
			inst->reopenOnTruncate = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "maxlinesatonce")) {
			if(   loadModConf->opMode == OPMODE_INOTIFY
			   && pvals[i].val.d.n > 0) {
				LogError(0, RS_RET_PARAM_NOT_PERMITTED,
					"parameter \"maxLinesAtOnce\" not "
					"permited in inotify mode - ignored");
			} else {
				inst->maxLinesAtOnce = pvals[i].val.d.n;
			}
		} else if(!strcmp(inppblk.descr[i].name, "trimlineoverbytes")) {
			inst->trimLineOverBytes = pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "persiststateinterval")) {
			inst->iPersistStateInterval = pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "maxsubmitatonce")) {
			inst->nMultiSub = pvals[i].val.d.n;
		} else if(!strcmp(inppblk.descr[i].name, "readtimeout")) {
			inst->readTimeout = pvals[i].val.d.n;
		} else {
			DBGPRINTF("program error, non-handled "
			  "param '%s'\n", inppblk.descr[i].name);
		}
	}
	i = (inst->readMode > 0) ? 1 : 0;
	i = (NULL != inst->startRegex) ? (i+1) : i;
	i = (NULL != inst->endRegex) ? (i+1) : i;
	if(i > 1) {
		LogError(0, RS_RET_PARAM_NOT_PERMITTED,
			"only one of readMode or startmsg.regex or endmsg.regex can be set "
			"at the same time");
			ABORT_FINALIZE(RS_RET_PARAM_NOT_PERMITTED);
	}

	if(inst->startRegex != NULL) {
		const int errcode = regcomp(&inst->start_preg, (char*)inst->startRegex, REG_EXTENDED);
		if(errcode != 0) {
			char errbuff[512];
			regerror(errcode, &inst->start_preg, errbuff, sizeof(errbuff));
			parser_errmsg("imfile: error in startmsg.regex expansion: %s", errbuff);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}
	if(inst->endRegex != NULL) {
		const int errcode = regcomp(&inst->end_preg, (char*)inst->endRegex, REG_EXTENDED);
		if(errcode != 0) {
			char errbuff[512];
			regerror(errcode, &inst->end_preg, errbuff, sizeof(errbuff));
			parser_errmsg("imfile: error in endmsg.regex expansion: %s", errbuff);
			ABORT_FINALIZE(RS_RET_ERR);
		}
	}
	if(inst->readTimeout != 0)
		loadModConf->haveReadTimeouts = 1;
	iRet = checkInstance(inst);
finalize_it:
CODE_STD_FINALIZERnewInpInst
	cnfparamvalsDestruct(pvals, &inppblk);
ENDnewInpInst

BEGINbeginCnfLoad
CODESTARTbeginCnfLoad
	loadModConf = pModConf;
	pModConf->pConf = pConf;
	/* init our settings */
	loadModConf->opMode = OPMODE_POLLING;
	loadModConf->iPollInterval = DFLT_PollInterval;
	loadModConf->configSetViaV2Method = 0;
	loadModConf->readTimeout = 0; /* default: no timeout */
	loadModConf->timeoutGranularity = 1000; /* default: 1 second */
	loadModConf->haveReadTimeouts = 0; /* default: no timeout */
	loadModConf->normalizePath = 1;
	loadModConf->sortFiles = GLOB_NOSORT;
	loadModConf->conf_tree = calloc(sizeof(fs_node_t), 1);
	loadModConf->conf_tree->edges = NULL;
	bLegacyCnfModGlobalsPermitted = 1;
	/* init legacy config vars */
	cs.pszFileName = NULL;
	cs.pszFileTag = NULL;
	cs.pszStateFile = NULL;
	cs.iPollInterval = DFLT_PollInterval;
	cs.iPersistStateInterval = 0;
	cs.iFacility = 128;
	cs.iSeverity = 5;
	cs.readMode = 0;
	cs.maxLinesAtOnce = 10240;
	cs.trimLineOverBytes = 0;
ENDbeginCnfLoad


BEGINsetModCnf
	struct cnfparamvals *pvals = NULL;
	int i;
CODESTARTsetModCnf
	/* new style config has different default! */
#if defined(OS_SOLARIS)
	#if defined (HAVE_PORT_SOURCE_FILE) /* use FEN on Solaris if available */
		loadModConf->opMode = OPMODE_FEN;
	#else
		loadModConf->opMode = OPMODE_POLLING;
	#endif
#else
	loadModConf->opMode = OPMODE_INOTIFY;
#endif
	pvals = nvlstGetParams(lst, &modpblk, NULL);
	if(pvals == NULL) {
		LogError(0, RS_RET_MISSING_CNFPARAMS, "imfile: error processing module "
				"config parameters [module(...)]");
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	if(Debug) {
		DBGPRINTF("module (global) param blk for imfile:\n");
		cnfparamsPrint(&modpblk, pvals);
	}

	for(i = 0 ; i < modpblk.nParams ; ++i) {
		if(!pvals[i].bUsed)
			continue;
		if(!strcmp(modpblk.descr[i].name, "pollinginterval")) {
			loadModConf->iPollInterval = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "readtimeout")) {
			loadModConf->readTimeout = (int) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "timeoutgranularity")) {
			/* note: we need ms, thus "* 1000" */
			loadModConf->timeoutGranularity = (int) pvals[i].val.d.n * 1000;
		} else if(!strcmp(modpblk.descr[i].name, "sortfiles")) {
			loadModConf->sortFiles = ((sbool) pvals[i].val.d.n) ? 0 : GLOB_NOSORT;
		} else if(!strcmp(modpblk.descr[i].name, "normalizepath")) {
			loadModConf->normalizePath = (sbool) pvals[i].val.d.n;
		} else if(!strcmp(modpblk.descr[i].name, "mode")) {
			if(!es_strconstcmp(pvals[i].val.d.estr, "polling"))
				loadModConf->opMode = OPMODE_POLLING;
			else if(!es_strconstcmp(pvals[i].val.d.estr, "inotify")) {
#if defined(OS_SOLARIS) && defined (HAVE_PORT_SOURCE_FILE) /* use FEN on Solaris! */
				loadModConf->opMode = OPMODE_FEN;
				DBGPRINTF("inotify mode configured, but only FEN "
					"is available on OS SOLARIS. Switching to FEN "
					"Mode automatically\n");
#else
				#if defined(HAVE_INOTIFY_INIT)
					loadModConf->opMode = OPMODE_INOTIFY;
				#else
					loadModConf->opMode = OPMODE_POLLING;
				#endif
#endif
			} else if(!es_strconstcmp(pvals[i].val.d.estr, "fen"))
				loadModConf->opMode = OPMODE_FEN;
			else {
				char *cstr = es_str2cstr(pvals[i].val.d.estr, NULL);
				LogError(0, RS_RET_PARAM_ERROR, "imfile: unknown "
					"mode '%s'", cstr);
				free(cstr);
			}
		} else {
			DBGPRINTF("program error, non-handled "
			  "param '%s' in beginCnfLoad\n", modpblk.descr[i].name);
		}
	}

	/* remove all of our legacy handlers, as they can not used in addition
	 * the the new-style config method.
	 */
	bLegacyCnfModGlobalsPermitted = 0;
	loadModConf->configSetViaV2Method = 1;

finalize_it:
	if(pvals != NULL)
		cnfparamvalsDestruct(pvals, &modpblk);
ENDsetModCnf


BEGINendCnfLoad
CODESTARTendCnfLoad
	if(!loadModConf->configSetViaV2Method) {
		/* persist module-specific settings from legacy config system */
		loadModConf->iPollInterval = cs.iPollInterval;
	}
	DBGPRINTF("opmode is %d, polling interval is %d\n",
		  loadModConf->opMode,
		  loadModConf->iPollInterval);

	loadModConf = NULL; /* done loading */
	/* free legacy config vars */
	free(cs.pszFileName);
	free(cs.pszFileTag);
	free(cs.pszStateFile);
ENDendCnfLoad


BEGINcheckCnf
	instanceConf_t *inst;
CODESTARTcheckCnf
	for(inst = pModConf->root ; inst != NULL ; inst = inst->next) {
		std_checkRuleset(pModConf, inst);
	}
	if(pModConf->root == NULL) {
		LogError(0, RS_RET_NO_LISTNERS,
				"imfile: no files configured to be monitored - "
				"no input will be gathered");
		iRet = RS_RET_NO_LISTNERS;
	}
ENDcheckCnf


/* note: we do access files AFTER we have dropped privileges. This is
 * intentional, user must make sure the files have the right permissions.
 */
BEGINactivateCnf
	instanceConf_t *inst;
CODESTARTactivateCnf
	runModConf = pModConf;
	if(runModConf->root == NULL) {
		LogError(0, NO_ERRCODE, "imfile: no file monitors configured, "
				"input not activated.\n");
		ABORT_FINALIZE(RS_RET_NO_RUN);
	}

	for(inst = runModConf->root ; inst != NULL ; inst = inst->next) {
		// TODO: provide switch to turn off this warning?
		if(!containsGlobWildcard((char*)inst->pszFileName)) {
			if(access((char*)inst->pszFileName, R_OK) != 0) {
				LogError(errno, RS_RET_ERR,
					"imfile: on startup file '%s' does not exist "
					"but is configured in static file monitor - this "
					"may indicate a misconfiguration. If the file "
					"appears at a later time, it will automatically "
					"be processed. Reason", inst->pszFileName);
			}
		}
		fs_node_add(runModConf->conf_tree, NULL, inst->pszFileName, 0, inst);
	}

	if(Debug) {
		fs_node_print(runModConf->conf_tree, 0);
	}

finalize_it:
ENDactivateCnf


BEGINfreeCnf
	instanceConf_t *inst, *del;
CODESTARTfreeCnf
	fs_node_destroy(pModConf->conf_tree);
	//move_list_destruct(pModConf);
	for(inst = pModConf->root ; inst != NULL ; ) {
		free(inst->pszBindRuleset);
		free(inst->pszFileName);
		free(inst->pszTag);
		free(inst->pszStateFile);
		free(inst->pszFileName_forOldStateFile);
		if(inst->startRegex != NULL) {
			regfree(&inst->start_preg);
			free(inst->startRegex);
		}
		if(inst->endRegex != NULL) {
			regfree(&inst->end_preg);
			free(inst->endRegex);
		}
		del = inst;
		inst = inst->next;
		free(del);
	}
ENDfreeCnf


/* initial poll run, to be used for all modes. Depending on mode, it does some
 * further initializations (e.g. watches in inotify mode). Most importantly,
 * it processes already-existing files, which would not otherwise be picked
 * up in notifcation modes (inotfiy, FEN). Also, when freshStartTail is set,
 * this run assumes that all previous existing data exists and needs not
 * to be considered.
 * Note: there is a race on files created *during* the run, but that race is
 * inevitable (and thus freshStartTail is actually broken, but users still seem
 * to want it...).
 * rgerhards, 2018-05-17
 */
static void
do_initial_poll_run(void)
{
	fs_node_walk(runModConf->conf_tree, poll_tree);

	/* fresh start done, so disable freshStartTail for files that now will be created */
	for(instanceConf_t *inst = runModConf->root ; inst != NULL ; inst = inst->next) {
		inst->freshStartTail = 0;
	}
}


/* Monitor files in polling mode. */
static rsRetVal
doPolling(void)
{
	DEFiRet;
	do_initial_poll_run();
	while(glbl.GetGlobalInputTermState() == 0) {
		DBGPRINTF("doPolling: new poll run\n");
		do {
			runModConf->bHadFileData = 0;
			fs_node_walk(runModConf->conf_tree, poll_tree);
			DBGPRINTF("doPolling: end poll walk, hadData %d\n", runModConf->bHadFileData);
		} while(runModConf->bHadFileData); /* warning: do...while()! */

		/* Note: the additional 10ns wait is vitally important. It guards rsyslog
		 * against totally hogging the CPU if the users selects a polling interval
		 * of 0 seconds. It doesn't hurt any other valid scenario. So do not remove.
		 * rgerhards, 2008-02-14
		 */
		DBGPRINTF("doPolling: poll going to sleep\n");
		if(glbl.GetGlobalInputTermState() == 0)
			srSleep(runModConf->iPollInterval, 10);
	}

	RETiRet;
}

#if defined(HAVE_INOTIFY_INIT)

static void ATTR_NONNULL(1)
in_dbg_showEv(const struct inotify_event *ev)
{
	if(!Debug)
		return;
	if(ev->mask & IN_IGNORED) {
		dbgprintf("INOTIFY event: watch was REMOVED\n");
	}
	if(ev->mask & IN_MODIFY) {
		dbgprintf("INOTIFY event: watch was MODIFID\n");
	}
	if(ev->mask & IN_ACCESS) {
		dbgprintf("INOTIFY event: watch IN_ACCESS\n");
	}
	if(ev->mask & IN_ATTRIB) {
		dbgprintf("INOTIFY event: watch IN_ATTRIB\n");
	}
	if(ev->mask & IN_CLOSE_WRITE) {
		dbgprintf("INOTIFY event: watch IN_CLOSE_WRITE\n");
	}
	if(ev->mask & IN_CLOSE_NOWRITE) {
		dbgprintf("INOTIFY event: watch IN_CLOSE_NOWRITE\n");
	}
	if(ev->mask & IN_CREATE) {
		dbgprintf("INOTIFY event: file was CREATED: %s\n", ev->name);
	}
	if(ev->mask & IN_DELETE) {
		dbgprintf("INOTIFY event: watch IN_DELETE\n");
	}
	if(ev->mask & IN_DELETE_SELF) {
		dbgprintf("INOTIFY event: watch IN_DELETE_SELF\n");
	}
	if(ev->mask & IN_MOVE_SELF) {
		dbgprintf("INOTIFY event: watch IN_MOVE_SELF\n");
	}
	if(ev->mask & IN_MOVED_FROM) {
		dbgprintf("INOTIFY event: watch IN_MOVED_FROM, cookie %u, name '%s'\n", ev->cookie, ev->name);
	}
	if(ev->mask & IN_MOVED_TO) {
		dbgprintf("INOTIFY event: watch IN_MOVED_TO, cookie %u, name '%s'\n", ev->cookie, ev->name);
	}
	if(ev->mask & IN_OPEN) {
		dbgprintf("INOTIFY event: watch IN_OPEN\n");
	}
	if(ev->mask & IN_ISDIR) {
		dbgprintf("INOTIFY event: watch IN_ISDIR\n");
	}
}


static void ATTR_NONNULL(1, 2)
in_handleFileEvent(struct inotify_event *ev, const wd_map_t *const etry)
{
	if(ev->mask & IN_MODIFY) {
		DBGPRINTF("fs_node_notify_file_update: act->name '%s'\n", etry->act->name);
		pollFile(etry->act);
	} else {
		DBGPRINTF("got non-expected inotify event:\n");
		in_dbg_showEv(ev);
	}
}


/* workaround for IN_MOVED: walk active list and prevent state file deletion of
 * IN_MOVED_IN active object
 * TODO: replace by a more generic solution.
 */
static void
flag_in_move(fs_edge_t *const edge, const char *name_moved)
{
	act_obj_t *act;

	for(act = edge->active ; act != NULL ; act = act->next) {
		DBGPRINTF("checking active object %s\n", act->basename);
		if(!strcmp(act->basename, name_moved)){
			DBGPRINTF("found file\n");
			act->in_move = 1;
			break;
		} else {
			DBGPRINTF("name check fails, '%s' != '%s'\n", act->basename, name_moved);
		}
	}
	if (!act && edge->next) {
		flag_in_move(edge->next, name_moved);
	}
}

static void ATTR_NONNULL(1)
in_processEvent(struct inotify_event *ev)
{
	if(ev->mask & IN_IGNORED) {
		DBGPRINTF("imfile: got IN_IGNORED event\n");
		goto done;
	}

	DBGPRINTF("in_processEvent process Event %x for %s\n", ev->mask, ev->name);
	const wd_map_t *const etry =  wdmapLookup(ev->wd);
	if(etry == NULL) {
		LogMsg(0, RS_RET_INTERNAL_ERROR, LOG_WARNING, "imfile: internal error? "
			"inotify provided watch descriptor %d which we could not find "
			"in our tables - ignored", ev->wd);
		goto done;
	}
	DBGPRINTF("in_processEvent process Event %x is_file %d, act->name '%s'\n",
		ev->mask, etry->act->edge->is_file, etry->act->name);

	if((ev->mask & IN_MOVED_FROM)) {
		flag_in_move(etry->act->edge->node->edges, ev->name);
	}
	if(ev->mask & (IN_MOVED_FROM | IN_MOVED_TO))  {
		fs_node_walk(etry->act->edge->node, poll_tree);
	} else if(etry->act->edge->is_file && !(etry->act->is_symlink)) {
		in_handleFileEvent(ev, etry); // esentially poll_file()!
	} else {
		fs_node_walk(etry->act->edge->node, poll_tree);
	}
done:	return;
}


/* Monitor files in inotify mode */
#if !defined(_AIX)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align" /* TODO: how can we fix these warnings? */
#endif
/* Problem with the warnings: they seem to stem back from the way the API is structured */
static rsRetVal
do_inotify(void)
{
	char iobuf[8192];
	struct inotify_event *ev;
	int rd;
	int currev;
	DEFiRet;

	CHKiRet(wdmapInit());
	ino_fd = inotify_init();
	if(ino_fd < 0) {
		LogError(errno, RS_RET_INOTIFY_INIT_FAILED, "imfile: Init inotify "
			"instance failed ");
		return RS_RET_INOTIFY_INIT_FAILED;
	}
	DBGPRINTF("inotify fd %d\n", ino_fd);

	do_initial_poll_run();

	while(glbl.GetGlobalInputTermState() == 0) {
		if(runModConf->haveReadTimeouts) {
			int r;
			struct pollfd pollfd;
			pollfd.fd = ino_fd;
			pollfd.events = POLLIN;
			do {
				r = poll(&pollfd, 1, runModConf->timeoutGranularity);
			} while(r  == -1 && errno == EINTR);
			if(r == 0) {
				DBGPRINTF("readTimeouts are configured, checking if some apply\n");
				fs_node_walk(runModConf->conf_tree, poll_timeouts);
				continue;
			} else if (r == -1) {
				LogError(errno, RS_RET_INTERNAL_ERROR,
					"%s:%d: unexpected error during poll timeout wait",
					__FILE__, __LINE__);
				/* we do not abort, as this would render the whole input defunct */
				continue;
			} else if(r != 1) {
				LogError(errno, RS_RET_INTERNAL_ERROR,
					"%s:%d: ERROR: poll returned more fds (%d) than given to it (1)",
					__FILE__, __LINE__, r);
				/* we do not abort, as this would render the whole input defunct */
				continue;
			}
		}
		rd = read(ino_fd, iobuf, sizeof(iobuf));
		if(rd == -1 && errno == EINTR) {
			/* This might have been our termination signal! */
			DBGPRINTF("EINTR received during inotify, restarting poll\n");
			continue;
		}
		if(rd < 0) {
			LogError(errno, RS_RET_IO_ERROR, "imfile: error during inotify - ignored");
			continue;
		}
		currev = 0;
		while(currev < rd) {
			ev = (struct inotify_event*) (iobuf+currev);
			in_dbg_showEv(ev);
			in_processEvent(ev);
			currev += sizeof(struct inotify_event) + ev->len;
		}
	}

finalize_it:
	close(ino_fd);
	RETiRet;
}
#pragma GCC diagnostic pop

#else /* #if HAVE_INOTIFY_INIT */
static rsRetVal
do_inotify(void)
{
	LogError(0, RS_RET_NOT_IMPLEMENTED, "imfile: mode set to inotify, but the "
			"platform does not support inotify");
	return RS_RET_NOT_IMPLEMENTED;
}
#endif /* #if HAVE_INOTIFY_INIT */


/* --- Monitor files in FEN mode (OS_SOLARIS)*/
#if defined(OS_SOLARIS) && defined (HAVE_PORT_SOURCE_FILE) /* use FEN on Solaris! */
static void
fen_printevent(int event)
{
	if (event & FILE_ACCESS) {
		DBGPRINTF(" FILE_ACCESS");
	}
	if (event & FILE_MODIFIED) {
		DBGPRINTF(" FILE_MODIFIED");
	}
	if (event & FILE_ATTRIB) {
		DBGPRINTF(" FILE_ATTRIB");
	}
	if (event & FILE_DELETE) {
		DBGPRINTF(" FILE_DELETE");
	}
	if (event & FILE_RENAME_TO) {
		DBGPRINTF(" FILE_RENAME_TO");
	}
	if (event & FILE_RENAME_FROM) {
		DBGPRINTF(" FILE_RENAME_FROM");
	}
	if (event & UNMOUNTED) {
		DBGPRINTF(" UNMOUNTED");
	}
	if (event & MOUNTEDOVER) {
		DBGPRINTF(" MOUNTEDOVER");
	}
}


/* https://docs.oracle.com/cd/E19253-01/816-5168/port-get-3c/index.html */
static rsRetVal
do_fen(void)
{
	port_event_t portEvent;
	struct timespec timeout;
	DEFiRet;
	//rsRetVal iRetTmp = RS_RET_OK;

	/* Set port timeout to 1 second. We need to check for unmonitored files during meantime */
	// TODO: do we need this timeout at all for equality to old code?
	// TODO: do we need it to support the timeout feature!
	timeout.tv_sec = 300;
	timeout.tv_nsec = 0;

	/* create port instance */
	if((glport = port_create()) == -1) {
		LogError(errno, RS_RET_FEN_INIT_FAILED, "do_fen INIT Port failed ");
		return RS_RET_FEN_INIT_FAILED;
	}

	do_initial_poll_run();

	DBGPRINTF("do_fen ENTER monitoring loop \n");
	while(glbl.GetGlobalInputTermState() == 0) {
		DBGPRINTF("do_fen loop begin... \n");
		/* Loop through events, if there are any */
		while (!port_get(glport, &portEvent, &timeout)) { // wie inotify-wait
			DBGPRINTF("do_fen: received port event with ");
			fen_printevent((int) portEvent.portev_events);
			DBGPRINTF("\n");
			if(portEvent.portev_source != PORT_SOURCE_FILE) {
				LogError(errno, RS_RET_SYS_ERR, "do_fen: Event from unexpected source "
					": %d\n", portEvent.portev_source);
				continue;
			}
			act_obj_t *const act = (act_obj_t*) portEvent.portev_user;
			DBGPRINTF("do_fen event received: deleted %d, is_file %d, name '%s' foname '%s'\n",
				act->is_deleted, act->edge->is_file, act->name,
				((struct file_obj*)portEvent.portev_object)->fo_name);
			if(act->is_deleted) {
				free(act->name);
				free(act);
				continue;
			}

			/* we need to re-associate the object */
			act->bPortAssociated = 0;
			fen_setupWatch(act);

			if(act->edge->is_file) {
				pollFile(act);
			} else {
				// curr: fs_node_walk(act->edge->parent, poll_tree);
				fs_node_walk(act->edge->node, poll_tree);
			}
		}
	}

	/* close port, will de-activate all file events watches associated
	 * with the port.
	 */
	close(glport);
	RETiRet;
}
#else /* #if OS_SOLARIS */
static rsRetVal
do_fen(void)
{
	LogError(0, RS_RET_NOT_IMPLEMENTED, "do_fen: mode set to fen, but the "
			"platform does not support fen");
	return RS_RET_NOT_IMPLEMENTED;
}
#endif /* #if OS_SOLARIS */


/* This function is called by the framework to gather the input. The module stays
 * most of its lifetime inside this function. It MUST NEVER exit this function. Doing
 * so would end module processing and rsyslog would NOT reschedule the module. If
 * you exit from this function, you violate the interface specification!
 */
BEGINrunInput
CODESTARTrunInput
	#if defined(OS_SOLARIS) && defined (HAVE_PORT_SOURCE_FILE) /* use FEN on Solaris! */
	if(runModConf->opMode == OPMODE_INOTIFY) {
		DBGPRINTF("auto-adjusting 'inotify' mode to 'fen' on Solaris\n");
		runModConf->opMode = OPMODE_FEN;
	}
	#endif
	DBGPRINTF("working in %s mode\n",
		 (runModConf->opMode == OPMODE_POLLING) ? "polling" :
			((runModConf->opMode == OPMODE_INOTIFY) ?"inotify" : "fen"));
	if(runModConf->opMode == OPMODE_POLLING)
		iRet = doPolling();
	else if(runModConf->opMode == OPMODE_INOTIFY)
		iRet = do_inotify();
	else if(runModConf->opMode == OPMODE_FEN)
		iRet = do_fen();
	else {
		LogError(0, RS_RET_NOT_IMPLEMENTED, "imfile: unknown mode %d set",
			runModConf->opMode);
		return RS_RET_NOT_IMPLEMENTED;
	}
	DBGPRINTF("terminating upon request of rsyslog core\n");
ENDrunInput


/* The function is called by rsyslog before runInput() is called. It is a last chance
 * to set up anything specific. Most importantly, it can be used to tell rsyslog if the
 * input shall run or not. The idea is that if some config settings (or similiar things)
 * are not OK, the input can tell rsyslog it will not execute. To do so, return
 * RS_RET_NO_RUN or a specific error code. If RS_RET_OK is returned, rsyslog will
 * proceed and call the runInput() entry point.
 */
BEGINwillRun
CODESTARTwillRun
	/* we need to create the inputName property (only once during our lifetime) */
	CHKiRet(prop.Construct(&pInputName));
	CHKiRet(prop.SetString(pInputName, UCHAR_CONSTANT("imfile"), sizeof("imfile") - 1));
	CHKiRet(prop.ConstructFinalize(pInputName));
finalize_it:
ENDwillRun

// TODO: refactor this into a generically-usable "atomic file creation" utility for
// all kinds of "state files"
static rsRetVal ATTR_NONNULL()
atomicWriteStateFile(const char *fn, const char *content)
{
	DEFiRet;
	const int fd = open(fn, O_CLOEXEC | O_NOCTTY | O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if(fd < 0) {
		LogError(errno, RS_RET_IO_ERROR, "imfile: cannot open state file '%s' for "
			"persisting file state - some data will probably be duplicated "
			"on next startup", fn);
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}

	const size_t toWrite = strlen(content);
	const ssize_t w = write(fd, content, toWrite);
	if(w != (ssize_t) toWrite) {
		LogError(errno, RS_RET_IO_ERROR, "imfile: partial write to state file '%s' "
			"this may cause trouble in the future. We will try to delete the "
			"state file, as this provides most consistent state", fn);
		unlink(fn);
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}

finalize_it:
	if(fd >= 0) {
		close(fd);
	}
	RETiRet;
}


/* This function persists information for a specific file being monitored.
 * To do so, it simply persists the stream object. We do NOT abort on error
 * iRet as that makes matters worse (at least we can try persisting the others...).
 * rgerhards, 2008-02-13
 */
static rsRetVal ATTR_NONNULL()
persistStrmState(act_obj_t *const act)
{
	DEFiRet;
	char file_id[FILE_ID_HASH_SIZE];
	uchar statefile[MAXFNAME];
	uchar statefname[MAXFNAME];

	uchar *const statefn = getStateFileName(act, statefile, sizeof(statefile));
	getFileID(act, file_id, sizeof(file_id));
	getFullStateFileName(statefn, file_id, statefname, sizeof(statefname));
	DBGPRINTF("persisting state for '%s', state file '%s'\n", act->name, statefname);

	struct json_object *jval = NULL;
	struct json_object *json = NULL;
	CHKmalloc(json = json_object_new_object());
	jval = json_object_new_string((char*) act->name);
	json_object_object_add(json, "filename", jval);
	jval = json_object_new_int(strmGetPrevWasNL(act->pStrm));
	json_object_object_add(json, "prev_was_nl", jval);

	/* we access some data items a bit dirty, as we need to refactor the whole
	 * thing in any case - TODO
	 */
	jval = json_object_new_int64(act->pStrm->iCurrOffs);
	json_object_object_add(json, "curr_offs", jval);
	jval = json_object_new_int64(act->pStrm->strtOffs);
	json_object_object_add(json, "strt_offs", jval);

	const uchar *const prevLineSegment = strmGetPrevLineSegment(act->pStrm);
	if(prevLineSegment != NULL) {
		jval = json_object_new_string((const char*) prevLineSegment);
		json_object_object_add(json, "prev_line_segment", jval);
	}

	const uchar *const prevMsgSegment = strmGetPrevMsgSegment(act->pStrm);
	if(prevMsgSegment != NULL) {
		jval = json_object_new_string((const char*) prevMsgSegment);
		json_object_object_add(json, "prev_msg_segment", jval);
	}

	const char *jstr =  json_object_to_json_string_ext(json, JSON_C_TO_STRING_SPACED);

	CHKiRet(atomicWriteStateFile((const char*)statefname, jstr));
	json_object_put(json);

finalize_it:
	if(iRet != RS_RET_OK) {
		LogError(0, iRet, "imfile: could not persist state "
				"file %s - data may be repeated on next "
				"startup. Is WorkDirectory set?",
				statefname);
	}

	RETiRet;
}

/* This function is called by the framework after runInput() has been terminated. It
 * shall free any resources and prepare the module for unload.
 */
BEGINafterRun
CODESTARTafterRun
	if(pInputName != NULL)
		prop.Destruct(&pInputName);
ENDafterRun


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURENonCancelInputTermination)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


/* The following entry points are defined in module-template.h.
 * In general, they need to be present, but you do NOT need to provide
 * any code here.
 */
BEGINmodExit
CODESTARTmodExit
	/* release objects we used */
	objRelease(strm, CORE_COMPONENT);
	objRelease(glbl, CORE_COMPONENT);
	objRelease(prop, CORE_COMPONENT);
	objRelease(ruleset, CORE_COMPONENT);

	#ifdef HAVE_INOTIFY_INIT
	free(wdmap);
	#endif
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_IMOD_QUERIES
CODEqueryEtryPt_STD_CONF2_QUERIES
CODEqueryEtryPt_STD_CONF2_setModCnf_QUERIES
CODEqueryEtryPt_STD_CONF2_IMOD_QUERIES
CODEqueryEtryPt_IsCompatibleWithFeature_IF_OMOD_QUERIES
ENDqueryEtryPt


/* The following function shall reset all configuration variables to their
 * default values. The code provided in modInit() below registers it to be
 * called on "$ResetConfigVariables". You may also call it from other places,
 * but in general this is not necessary. Once runInput() has been called, this
 * function here is never again called.
 */
static rsRetVal
resetConfigVariables(uchar __attribute__((unused)) *pp, void __attribute__((unused)) *pVal)
{
	DEFiRet;

	free(cs.pszFileName);
	cs.pszFileName = NULL;
	free(cs.pszFileTag);
	cs.pszFileTag = NULL;
	free(cs.pszStateFile);
	cs.pszStateFile = NULL;

	/* set defaults... */
	cs.iPollInterval = DFLT_PollInterval;
	cs.iFacility = 128; /* local0 */
	cs.iSeverity = 5;  /* notice, as of rfc 3164 */
	cs.readMode = 0;
	cs.maxLinesAtOnce = 10240;
	cs.trimLineOverBytes = 0;

	RETiRet;
}

static inline void
std_checkRuleset_genErrMsg(__attribute__((unused)) modConfData_t *modConf, instanceConf_t *inst)
{
	LogError(0, NO_ERRCODE, "imfile: ruleset '%s' for %s not found - "
			"using default ruleset instead", inst->pszBindRuleset,
			inst->pszFileName);
}

/* modInit() is called once the module is loaded. It must perform all module-wide
 * initialization tasks. There are also a number of housekeeping tasks that the
 * framework requires. These are handled by the macros. Please note that the
 * complexity of processing is depending on the actual module. However, only
 * thing absolutely necessary should be done here. Actual app-level processing
 * is to be performed in runInput(). A good sample of what to do here may be to
 * set some variable defaults.
 */
BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(strm, CORE_COMPONENT));
	CHKiRet(objUse(ruleset, CORE_COMPONENT));
	CHKiRet(objUse(prop, CORE_COMPONENT));

	DBGPRINTF("version %s initializing\n", VERSION);
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputfilename", 0, eCmdHdlrGetWord,
	  	NULL, &cs.pszFileName, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputfiletag", 0, eCmdHdlrGetWord,
	  	NULL, &cs.pszFileTag, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputfilestatefile", 0, eCmdHdlrGetWord,
	  	NULL, &cs.pszStateFile, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputfileseverity", 0, eCmdHdlrSeverity,
	  	NULL, &cs.iSeverity, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputfilefacility", 0, eCmdHdlrFacility,
	  	NULL, &cs.iFacility, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputfilereadmode", 0, eCmdHdlrInt,
	  	NULL, &cs.readMode, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputfilemaxlinesatonce", 0, eCmdHdlrSize,
	  	NULL, &cs.maxLinesAtOnce, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputfiletrimlineoverbytes", 0, eCmdHdlrSize,
	  	NULL, &cs.trimLineOverBytes, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputfilepersiststateinterval", 0, eCmdHdlrInt,
	  	NULL, &cs.iPersistStateInterval, STD_LOADABLE_MODULE_ID));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputfilebindruleset", 0, eCmdHdlrGetWord,
		NULL, &cs.pszBindRuleset, STD_LOADABLE_MODULE_ID));
	/* that command ads a new file! */
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"inputrunfilemonitor", 0, eCmdHdlrGetWord,
		addInstance, NULL, STD_LOADABLE_MODULE_ID));
	/* module-global config params - will be disabled in configs that are loaded
	 * via module(...).
	 */
	CHKiRet(regCfSysLineHdlr2((uchar *)"inputfilepollinterval", 0, eCmdHdlrInt,
	  	NULL, &cs.iPollInterval, STD_LOADABLE_MODULE_ID, &bLegacyCnfModGlobalsPermitted));
	CHKiRet(omsdRegCFSLineHdlr((uchar *)"resetconfigvariables", 1, eCmdHdlrCustomHandler,
		resetConfigVariables, NULL, STD_LOADABLE_MODULE_ID));
ENDmodInit
