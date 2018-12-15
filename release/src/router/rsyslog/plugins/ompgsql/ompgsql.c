/* ompgsql.c
 * This is the implementation of the build-in output module for PgSQL.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * File begun on 2007-10-18 by sur5r (converted from ommysql.c)
 *
 * Copyright 2007-2018 Rainer Gerhards and Adiscon GmbH.
 *
 * The following link my be useful for the not-so-postgres literate
 * when setting up a test environment (on Fedora):
 * http://www.jboss.org/community/wiki/InstallPostgreSQLonFedora
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */
#include "config.h"
#include "rsyslog.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <libpq-fe.h>
#include "conf.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "module-template.h"
#include "errmsg.h"
#include "parserif.h"

MODULE_TYPE_OUTPUT
MODULE_TYPE_NOKEEP
MODULE_CNFNAME("ompgsql")


/* internal structures
 */
DEF_OMOD_STATIC_DATA

typedef struct _instanceData {
	char            srv[MAXHOSTNAMELEN+1];   /* IP or hostname of DB server*/
	char            dbname[_DB_MAXDBLEN+1];  /* DB name */
	char            user[_DB_MAXUNAMELEN+1]; /* DB user */
	char            pass[_DB_MAXPWDLEN+1];   /* DB user's password */
	unsigned int    trans_age;
	unsigned int    trans_commit;
	unsigned short  multi_row;
	int             port;
	uchar          *tpl;                      /* format template to use */
} instanceData;

typedef struct wrkrInstanceData {
	instanceData   *pData;
	PGconn         *f_hpgsql;                /* handle to PgSQL */
	ConnStatusType  eLastPgSQLStatus;        /* last status from postgres */
} wrkrInstanceData_t;

/* action (instance) parameters */
static struct cnfparamdescr actpdescr[] = {
	{ "server",     eCmdHdlrGetWord, 1 },
	{ "db",         eCmdHdlrGetWord, 1 },
	{ "user",       eCmdHdlrGetWord, 0 },
	{ "uid",        eCmdHdlrGetWord, 0 },
	{ "pass",       eCmdHdlrGetWord, 0 },
	{ "pwd",        eCmdHdlrGetWord, 0 },
	{ "multirows",  eCmdHdlrInt,     0 },
	{ "trans_size", eCmdHdlrInt,     0 },
	{ "trans_age",  eCmdHdlrInt,     0 },
	{ "serverport", eCmdHdlrInt,     0 },
	{ "port",       eCmdHdlrInt,     0 },
	{ "template",   eCmdHdlrGetWord, 0 }
};


static struct cnfparamblk actpblk =
	{ CNFPARAMBLK_VERSION,
	  sizeof(actpdescr)/sizeof(struct cnfparamdescr),
	  actpdescr
	};


BEGINinitConfVars     /* (re)set config variables to default values */
CODESTARTinitConfVars
ENDinitConfVars


static rsRetVal writePgSQL(uchar *psz, wrkrInstanceData_t *pData);

BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance

BEGINcreateWrkrInstance
CODESTARTcreateWrkrInstance
	pWrkrData->f_hpgsql = NULL;
ENDcreateWrkrInstance


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if (eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


/* The following function is responsible for closing a
 * PgSQL connection.
 */
static void closePgSQL(wrkrInstanceData_t *pWrkrData)
{
	assert(pWrkrData != NULL);

	if (pWrkrData->f_hpgsql != NULL) {  /* just to be on the safe side... */
		PQfinish(pWrkrData->f_hpgsql);
		pWrkrData->f_hpgsql = NULL;
	}
}

BEGINfreeInstance
CODESTARTfreeInstance
	free(pData->tpl);
ENDfreeInstance

BEGINfreeWrkrInstance
CODESTARTfreeWrkrInstance
	closePgSQL(pWrkrData);
ENDfreeWrkrInstance

BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	/* nothing special here */
ENDdbgPrintInstInfo


/* log a database error with descriptive message.
 * We check if we have a valid handle. If not, we simply
 * report an error, but can not be specific. RGerhards, 2007-01-30
 */
static void reportDBError(wrkrInstanceData_t *pWrkrData, int bSilent)
{
	char errMsg[512];
	ConnStatusType ePgSQLStatus;

	assert(pWrkrData != NULL);
	bSilent = 0;

	/* output log message */
	errno = 0;
	if (pWrkrData->f_hpgsql == NULL) {
		LogError(0, NO_ERRCODE, "unknown DB error occured - could not obtain PgSQL handle");
	} else { /* we can ask pgsql for the error description... */
		ePgSQLStatus = PQstatus(pWrkrData->f_hpgsql);
		snprintf(errMsg, sizeof(errMsg), "db error (%d): %s\n", ePgSQLStatus,
				PQerrorMessage(pWrkrData->f_hpgsql));
		if (bSilent || ePgSQLStatus == pWrkrData->eLastPgSQLStatus)
			dbgprintf("pgsql, DBError(silent): %s\n", errMsg);
		else {
			pWrkrData->eLastPgSQLStatus = ePgSQLStatus;
			LogError(0, NO_ERRCODE, "%s", errMsg);
		}
	}

	return;
}


/* The following function is responsible for initializing a
 * PgSQL connection.
 */
static rsRetVal initPgSQL(wrkrInstanceData_t *pWrkrData, int bSilent)
{
	instanceData *pData;
	DEFiRet;

	pData = pWrkrData->pData;
	assert(pData != NULL);
	assert(pWrkrData->f_hpgsql == NULL);

	dbgprintf("host=%s port=%d dbname=%s uid=%s\n",pData->srv, pData->port, pData->dbname, pData->user);

	/* Force PostgreSQL to use ANSI-SQL conforming strings, otherwise we may
	 * get all sorts of side effects (e.g.: backslash escapes) and warnings
	 */
	const char *PgConnectionOptions = "-c standard_conforming_strings=on";

	/* Connect to database */
	char port[6];
	snprintf(port, sizeof(port), "%d", pData->port);
	if ((pWrkrData->f_hpgsql=PQsetdbLogin(pData->srv, port, PgConnectionOptions, NULL,
				pData->dbname, pData->user, pData->pass)) == NULL) {
		reportDBError(pWrkrData, bSilent);
		closePgSQL(pWrkrData); /* ignore any error we may get */
		iRet = RS_RET_SUSPENDED;
	}

	RETiRet;
}


/* try the insert into postgres and return if that failed or not
 * (1 = had error, 0=ok). We do not use the standard IRET calling convention
 * rgerhards, 2009-04-17
 */
static int
tryExec(uchar *pszCmd, wrkrInstanceData_t *pWrkrData)
{
	PGresult *pgRet;
	ExecStatusType execState;
	int bHadError = 0;

	/* try insert */
	pgRet = PQexec(pWrkrData->f_hpgsql, (char*)pszCmd);
	execState = PQresultStatus(pgRet);
	if (execState != PGRES_COMMAND_OK && execState != PGRES_TUPLES_OK) {
		dbgprintf("postgres query execution failed: %s\n", PQresStatus(PQresultStatus(pgRet)));
		bHadError = 1;
	}
	PQclear(pgRet);

	return(bHadError);
}


/* The following function writes the current log entry
 * to an established PgSQL session.
 * Enhanced function to take care of the returned error
 * value (if there is such). Note that this may happen due to
 * a sql format error - connection aborts were properly handled
 * before my patch. -- rgerhards, 2009-04-17
 */
static rsRetVal
writePgSQL(uchar *psz, wrkrInstanceData_t *pWrkrData)
{
	int bHadError = 0;
	DEFiRet;

	assert(psz != NULL);
	assert(pWrkrData != NULL);

	dbgprintf("writePgSQL: %s\n", psz);

	bHadError = tryExec(psz, pWrkrData); /* try insert */

	if (bHadError || (PQstatus(pWrkrData->f_hpgsql) != CONNECTION_OK)) {
#if 0		/* re-enable once we have transaction support */
		/* error occured, try to re-init connection and retry */
		int inTransaction = 0;
		if(pData->f_hpgsql != NULL) {
			PGTransactionStatusType xactStatus = PQtransactionStatus(pData->f_hpgsql);
			if((xactStatus == PQTRANS_INTRANS) || (xactStatus == PQTRANS_ACTIVE)) {
				inTransaction = 1;
			}
		}
		if ( inTransaction == 0 )
#endif
		{
			closePgSQL(pWrkrData); /* close the current handle */
			CHKiRet(initPgSQL(pWrkrData, 0)); /* try to re-open */
			bHadError = tryExec(psz, pWrkrData); /* retry */
		}
		if(bHadError || (PQstatus(pWrkrData->f_hpgsql) != CONNECTION_OK)) {
			/* we failed, giving up for now */
			reportDBError(pWrkrData, 0);
			closePgSQL(pWrkrData); /* free ressources */
			ABORT_FINALIZE(RS_RET_SUSPENDED);
		}
	}

finalize_it:
	if (iRet == RS_RET_OK) {
		pWrkrData->eLastPgSQLStatus = CONNECTION_OK; /* reset error for error supression */
	}

	RETiRet;
}


BEGINtryResume
CODESTARTtryResume
	if (pWrkrData->f_hpgsql == NULL) {
		iRet = initPgSQL(pWrkrData, 1);
		if (iRet == RS_RET_OK) {
			/* the code above seems not to actually connect to the database. As such, we do a
			 * dummy statement (a pointless select...) to verify the connection and return
			 * success only when that statemetn succeeds. Note that I am far from being a
			 * PostgreSQL expert, so any patch that does the desired result in a more
			 * intelligent way is highly welcome. -- rgerhards, 2009-12-16
			 */
			iRet = writePgSQL((uchar*)"select 'a' as a", pWrkrData);
		}
	}
ENDtryResume


BEGINbeginTransaction
CODESTARTbeginTransaction
ENDbeginTransaction


BEGINcommitTransaction
CODESTARTcommitTransaction
	dbgprintf("ompgsql: beginTransaction\n");
	if (pWrkrData->f_hpgsql == NULL)
		initPgSQL(pWrkrData, 0);
	CHKiRet(writePgSQL((uchar*) "BEGIN", pWrkrData)); /* TODO: make user-configurable */

	for (unsigned i = 0 ; i < nParams ; ++i) {
		iRet = writePgSQL(actParam(pParams, 1, i, 0).param, pWrkrData);
		if (iRet != RS_RET_OK
			&& iRet != RS_RET_DEFER_COMMIT
			&& iRet != RS_RET_PREVIOUS_COMMITTED) {
			/*if(mysql_rollback(pWrkrData->hmysql) != 0) {
				DBGPRINTF("ommysql: server error: transaction could not be rolled back\n");
			}*/
			// closeMySQL(pWrkrData);
			// FINALIZE;
		}
	}

	CHKiRet(writePgSQL((uchar*) "COMMIT", pWrkrData)); /* TODO: make user-configurable */

finalize_it:
	if (iRet == RS_RET_OK) {
		pWrkrData->eLastPgSQLStatus = CONNECTION_OK; /* reset error for error supression */
	}

ENDcommitTransaction


static inline void
setInstParamDefaults(instanceData *pData)
{
	pData->tpl           = NULL;
	pData->multi_row     = 100;
	pData->trans_commit  = 100;
	pData->trans_age     = 60;
	pData->port          = 5432;
	strcpy(pData->user, "postgres");
	strcpy(pData->pass, "postgres");
}

BEGINnewActInst
	struct cnfparamvals *pvals;
	int i;
	char *cstr;
	size_t len;
CODESTARTnewActInst
	if ((pvals = nvlstGetParams(lst, &actpblk, NULL)) == NULL) {
		ABORT_FINALIZE(RS_RET_MISSING_CNFPARAMS);
	}

	CHKiRet(createInstance(&pData));
	setInstParamDefaults(pData);

	CODE_STD_STRING_REQUESTparseSelectorAct(1)
	for (i = 0 ; i < actpblk.nParams ; ++i) {
		if (!pvals[i].bUsed)
			continue;
		if (!strcmp(actpblk.descr[i].name, "server")) {
			cstr = es_str2cstr(pvals[i].val.d.estr, NULL);
			len = es_strlen(pvals[i].val.d.estr);
			if(len >= sizeof(pData->srv)-1) {
				parser_errmsg("ompgsql: srv parameter longer than supported "
					"maximum of %d characters", (int)sizeof(pData->srv)-1);
				ABORT_FINALIZE(RS_RET_PARAM_ERROR);
			}
			memcpy(pData->srv, cstr, len+1);
			free(cstr);
		} else if (!strcmp(actpblk.descr[i].name, "port")) {
			pData->port = (int) pvals[i].val.d.n;
		} else if (!strcmp(actpblk.descr[i].name, "serverport")) {
			pData->port = (int) pvals[i].val.d.n;
		} else if (!strcmp(actpblk.descr[i].name, "multirows")) {
			pData->multi_row = (int) pvals[i].val.d.n;
		} else if (!strcmp(actpblk.descr[i].name, "trans_size")) {
			pData->trans_commit = (int) pvals[i].val.d.n;
		} else if (!strcmp(actpblk.descr[i].name, "trans_age")) {
			pData->trans_age = (int) pvals[i].val.d.n;
		} else if (!strcmp(actpblk.descr[i].name, "db")) {
			cstr = es_str2cstr(pvals[i].val.d.estr, NULL);
			len = es_strlen(pvals[i].val.d.estr);
			if(len >= sizeof(pData->dbname)-1) {
				parser_errmsg("ompgsql: db parameter longer than supported "
					"maximum of %d characters", (int)sizeof(pData->dbname)-1);
				ABORT_FINALIZE(RS_RET_PARAM_ERROR);
			}
			memcpy(pData->dbname, cstr, len+1);
			free(cstr);
		} else if (   !strcmp(actpblk.descr[i].name, "user")
		           || !strcmp(actpblk.descr[i].name, "uid")) {
			cstr = es_str2cstr(pvals[i].val.d.estr, NULL);
			len = es_strlen(pvals[i].val.d.estr);
			if(len >= sizeof(pData->user)-1) {
				parser_errmsg("ompgsql: user/uid parameter longer than supported "
					"maximum of %d characters", (int)sizeof(pData->user)-1);
				ABORT_FINALIZE(RS_RET_PARAM_ERROR);
			}
			memcpy(pData->user, cstr, len+1);
			free(cstr);
		} else if (   !strcmp(actpblk.descr[i].name, "pass")
		           || !strcmp(actpblk.descr[i].name, "pwd")) {
			cstr = es_str2cstr(pvals[i].val.d.estr, NULL);
			len = es_strlen(pvals[i].val.d.estr);
			if(len >= sizeof(pData->pass)-1) {
				parser_errmsg("ompgsql: pass/pwd parameter longer than supported "
					"maximum of %d characters", (int)sizeof(pData->pass)-1);
				ABORT_FINALIZE(RS_RET_PARAM_ERROR);
			}
			memcpy(pData->pass, cstr, len+1);
			free(cstr);
		} else if (!strcmp(actpblk.descr[i].name, "template")) {
			pData->tpl = (uchar*)es_str2cstr(pvals[i].val.d.estr, NULL);
		} else {
			dbgprintf("ompgsql: program error, non-handled "
				"param '%s'\n", actpblk.descr[i].name);
		}
	}

	if (pData->tpl == NULL) {
		CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*) strdup(" StdPgSQLFmt"),     OMSR_RQD_TPL_OPT_SQL));
	} else {
		CHKiRet(OMSRsetEntry(*ppOMSR, 0, (uchar*) strdup((char*) pData->tpl), OMSR_RQD_TPL_OPT_SQL));
	}

CODE_STD_FINALIZERnewActInst
	cnfparamvalsDestruct(pvals, &actpblk);
ENDnewActInst

BEGINparseSelectorAct
	int iPgSQLPropErr = 0;
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	/* first check if this config line is actually for us
	 * The first test [*p == '>'] can be skipped if a module shall only
	 * support the newer slection syntax [:modname:]. This is in fact
	 * recommended for new modules. Please note that over time this part
	 * will be handled by rsyslogd itself, but for the time being it is
	 * a good compromise to do it at the module level.
	 * rgerhards, 2007-10-15
	 */

	if (!strncmp((char*) p, ":ompgsql:", sizeof(":ompgsql:") - 1))
		p += sizeof(":ompgsql:") - 1; /* eat indicator sequence (-1 because of '\0'!) */
	else
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);

	/* ok, if we reach this point, we have something for us */
	if ((iRet = createInstance(&pData)) != RS_RET_OK)
		goto finalize_it;
	setInstParamDefaults(pData);

	/* sur5r 2007-10-18: added support for PgSQL
	 * :ompgsql:server,dbname,userid,password
	 * Now we read the PgSQL connection properties
	 * and verify that the properties are valid.
	 */
	if (getSubString(&p, pData->srv, MAXHOSTNAMELEN+1, ','))
		iPgSQLPropErr++;
	dbgprintf("%p:%s\n",p,p);
	if (*pData->srv == '\0')
		iPgSQLPropErr++;
	if (getSubString(&p, pData->dbname, _DB_MAXDBLEN+1, ','))
		iPgSQLPropErr++;
	if (*pData->dbname == '\0')
		iPgSQLPropErr++;
	if (getSubString(&p, pData->user, _DB_MAXUNAMELEN+1, ','))
		iPgSQLPropErr++;
	if (*pData->user == '\0')
		iPgSQLPropErr++;
	if (getSubString(&p, pData->pass, _DB_MAXPWDLEN+1, ';'))
		iPgSQLPropErr++;
	/* now check for template
	 * We specify that the SQL option must be present in the template.
	 * This is for your own protection (prevent sql injection).
	 */
	if (*(p - 1) == ';') {
		p--;
		CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_RQD_TPL_OPT_SQL, (uchar*) pData->tpl));
	} else {
		CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_RQD_TPL_OPT_SQL, (uchar*)" StdPgSQLFmt"));
	}

	/* If we detect invalid properties, we disable logging,
	 * because right properties are vital at this place.
	 * Retries make no sense.
	 */
	if (iPgSQLPropErr) {
		LogError(0, RS_RET_INVALID_PARAMS, "Trouble with PgSQL connection properties. "
				"-PgSQL logging disabled");
		ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
	}

CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct

BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMODTX_QUERIES
CODEqueryEtryPt_STD_OMOD8_QUERIES
CODEqueryEtryPt_STD_CONF2_OMOD_QUERIES
/* CODEqueryEtryPt_TXIF_OMOD_QUERIES currently no TX support! */ /* we support the transactional interface! */
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
INITLegCnfVars
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	INITChkCoreFeature(bCoreSupportsBatching, CORE_FEATURE_BATCHING);
	if (!bCoreSupportsBatching) {
		LogError(0, NO_ERRCODE, "ompgsql: rsyslog core too old");
		ABORT_FINALIZE(RS_RET_ERR);
	}
ENDmodInit

/* vi:set ai: */
