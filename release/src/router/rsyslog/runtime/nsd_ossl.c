/* nsd_ossl.c
 *
 * An implementation of the nsd interface for OpenSSL.
 *
 * Copyright 2018-2018 Adiscon GmbH.
 * Author: Andre Lorbach
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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "rsyslog.h"
#include "syslogd-types.h"
#include "module-template.h"
#include "cfsysline.h"
#include "obj.h"
#include "stringbuf.h"
#include "errmsg.h"
#include "net.h"
#include "datetime.h"
#include "nsd_ptcp.h"
#include "nsdsel_ossl.h"
#include "nsd_ossl.h"
#include "unicode-helper.h"

/* things to move to some better place/functionality - TODO */
// #define CRLFILE "crl.pem"


MODULE_TYPE_LIB
MODULE_TYPE_KEEP

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)
DEFobjCurrIf(net)
DEFobjCurrIf(datetime)
DEFobjCurrIf(nsd_ptcp)

/* OpenSSL API differences */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
	#define RSYSLOG_X509_NAME_oneline(X509CERT) X509_get_subject_name(X509CERT)
	#define RSYSLOG_BIO_method_name(SSLBIO) BIO_method_name(SSLBIO)
	#define RSYSLOG_BIO_number_read(SSLBIO) BIO_number_read(SSLBIO)
	#define RSYSLOG_BIO_number_written(SSLBIO) BIO_number_written(SSLBIO)
#else
	#define RSYSLOG_X509_NAME_oneline(X509CERT) (X509CERT != NULL ? X509CERT->cert_info->subject : NULL)
	#define RSYSLOG_BIO_method_name(SSLBIO) SSLBIO->method->name
	#define RSYSLOG_BIO_number_read(SSLBIO) SSLBIO->num
	#define RSYSLOG_BIO_number_written(SSLBIO) SSLBIO->num
#endif


static int bGlblSrvrInitDone = 0;	/**< 0 - server global init not yet done, 1 - already done */

/* Main OpenSSL CTX pointer */
static SSL_CTX *ctx;

/*--------------------------------------MT OpenSSL helpers ------------------------------------------*/
static MUTEX_TYPE *mutex_buf = NULL;

void locking_function(int mode, int n,
	__attribute__((unused)) const char * file, __attribute__((unused)) int line)
{
	if (mode & CRYPTO_LOCK)
		MUTEX_LOCK(mutex_buf[n]);
	else
		MUTEX_UNLOCK(mutex_buf[n]);
}

unsigned long id_function(void)
{
	return ((unsigned long)THREAD_ID);
}


struct CRYPTO_dynlock_value * dyn_create_function(
	__attribute__((unused)) const char *file, __attribute__((unused)) int line)
{
	struct CRYPTO_dynlock_value *value;
	value = (struct CRYPTO_dynlock_value *)malloc(sizeof(struct CRYPTO_dynlock_value));
	if (!value)
		return NULL;

	MUTEX_SETUP(value->mutex);
	return value;
}

void dyn_lock_function(int mode, struct CRYPTO_dynlock_value *l,
	__attribute__((unused)) const char *file, __attribute__((unused)) int line)
{
	if (mode & CRYPTO_LOCK)
		MUTEX_LOCK(l->mutex);
	else
		MUTEX_UNLOCK(l->mutex);
}

void dyn_destroy_function(struct CRYPTO_dynlock_value *l,
	__attribute__((unused)) const char *file, __attribute__((unused)) int line)
{
	MUTEX_CLEANUP(l->mutex);
	free(l);
}

/* set up support functions for openssl multi-threading. This must
 * be done at library initialisation. If the function fails,
 * processing can not continue normally. On failure, 0 is
 * returned, on success 1.
 */
int opensslh_THREAD_setup(void)
{
	int i;
	mutex_buf = (MUTEX_TYPE *)malloc(CRYPTO_num_locks( ) * sizeof(MUTEX_TYPE));
	if (mutex_buf == NULL)
		return 0;
	for (i = 0; i < CRYPTO_num_locks( ); i++)
		MUTEX_SETUP(mutex_buf[i]);

	CRYPTO_set_id_callback(id_function);
	CRYPTO_set_locking_callback(locking_function);
	/* The following three CRYPTO_... functions are the OpenSSL functions
	for registering the callbacks we implemented above */
	CRYPTO_set_dynlock_create_callback(dyn_create_function);
	CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
	CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);

	DBGPRINTF("openssl: multithread setup finished\n");
	return 1;
}

/* shut down openssl - do this only when you are totally done
 * with openssl.
 */
int opensslh_THREAD_cleanup(void)
{
	int i;
	if (!mutex_buf)
		return 0;

	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);
	CRYPTO_set_dynlock_create_callback(NULL);
	CRYPTO_set_dynlock_lock_callback(NULL);
	CRYPTO_set_dynlock_destroy_callback(NULL);

	for (i = 0; i < CRYPTO_num_locks( ); i++)
		MUTEX_CLEANUP(mutex_buf[i]);

	free(mutex_buf);
	mutex_buf = NULL;

	DBGPRINTF("openssl: multithread cleanup finished\n");
	return 1;
}
/*--------------------------------------MT OpenSSL helpers ------------------------------------------*/

/*--------------------------------------OpenSSL helpers ------------------------------------------*/
void osslLastSSLErrorMsg(int ret, SSL *ssl, int severity, const char* pszCallSource)
{
	unsigned long un_error = 0;
//	char psz[256];
	int iSSLErr = SSL_get_error(ssl, ret);

	/* Check which kind of error we have */
	dbgprintf("OpenSSL Error '%s(%d)' in '%s' with ret=%d\n",
		ERR_error_string(iSSLErr, NULL), iSSLErr, pszCallSource, ret);
	if(iSSLErr == SSL_ERROR_SSL) {
		LogMsg(0, RS_RET_NO_ERRCODE, severity, "SSL_ERROR_SSL in '%s'", pszCallSource);
	} else if(iSSLErr == SSL_ERROR_SYSCALL){
		LogMsg(0, RS_RET_NO_ERRCODE, severity, "SSL_ERROR_SYSCALL in '%s'", pszCallSource);
/*
		if(ret == 0) {
			// iSSLErr = ERR_get_error();
			// iSSLErr = SSL_get_error(ssl, iSSLErr);
			if(iSSLErr == 0) {
				*psz = '\0';
			} else {
				ERR_error_string_n(ERR_get_error(), psz, sizeof(psz));
			}
		}
		LogMsg(0, RS_RET_NO_ERRCODE, "SSL_ERROR_SYSCALL in '%s': %s",
			pszCallSource, psz);
*/
	} else {
		LogMsg(0, RS_RET_NO_ERRCODE, severity, "SSL_ERROR_UNKNOWN in '%s', SSL_get_error: '%s(%d)'",
			pszCallSource, ERR_error_string(iSSLErr, NULL), iSSLErr);
	}

	/* Loop through ERR_get_error */
	while ((un_error = ERR_get_error()) > 0){
		LogMsg(0, RS_RET_NO_ERRCODE, severity, "Error Stack: %s", ERR_error_string(un_error, NULL) );
		dbgprintf("OpenSSL Error Stack: %s\n", ERR_error_string(un_error, NULL) );
	}

	/* Loop through ERR_peek_last_error */
	while ((un_error = ERR_peek_last_error()) != 0){
		LogMsg(0, RS_RET_NO_ERRCODE, severity, "Error Stack: %s", ERR_error_string(un_error, NULL) );
		dbgprintf("OpenSSL Error Stack: %s\n", ERR_error_string(un_error, NULL) );
	}
}

int verify_callback(int status, X509_STORE_CTX *store)
{
	char szdbgdata1[256];
	char szdbgdata2[256];

	if(status == 0) {
		dbgprintf("verify_callback: certificate validation failed!\n");

		X509 *cert = X509_STORE_CTX_get_current_cert(store);
		int depth = X509_STORE_CTX_get_error_depth(store);
		int err = X509_STORE_CTX_get_error(store);
		X509_NAME_oneline(X509_get_issuer_name(cert), szdbgdata1, sizeof(szdbgdata1));
		X509_NAME_oneline(RSYSLOG_X509_NAME_oneline(cert), szdbgdata2, sizeof(szdbgdata2));

		/* Log Warning only on EXPIRED */
		if (err == X509_V_OK || err == X509_V_ERR_CERT_HAS_EXPIRED) {
			LogMsg(0, RS_RET_NO_ERRCODE, LOG_WARNING,
				"Certificate warning at depth: %d \n\t"
				"issuer  = %s\n\t"
				"subject = %s\n\t"
				"err %d:%s",
				depth, szdbgdata1, szdbgdata2, err, X509_verify_cert_error_string(err));

			/* Set Status to OK*/
			status = 1;
		} else {
			LogError(0, RS_RET_NO_ERRCODE,
				"Certificate error at depth: %d \n\t"
				"issuer  = %s\n\t"
				"subject = %s\n\t"
				"err %d:%s",
				depth, szdbgdata1, szdbgdata2, err, X509_verify_cert_error_string(err));
		}

	}

	return status;
}

long BIO_debug_callback(BIO *bio, int cmd, const char __attribute__((unused)) *argp,
			int argi, long __attribute__((unused)) argl, long ret)
{
	long r = 1;

	if (BIO_CB_RETURN & cmd)
		r = ret;

	dbgprintf("openssl debugmsg: BIO[%p]: ", (void *)bio);

	switch (cmd) {
	case BIO_CB_FREE:
		dbgprintf("Free - %s\n", RSYSLOG_BIO_method_name(bio));
		break;
/* Disabled due API changes for OpenSSL 1.1.0+ */
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	case BIO_CB_READ:
		if (bio->method->type & BIO_TYPE_DESCRIPTOR)
			dbgprintf("read(%d,%lu) - %s fd=%d\n",
				RSYSLOG_BIO_number_read(bio), (unsigned long)argi,
				RSYSLOG_BIO_method_name(bio), RSYSLOG_BIO_number_read(bio));
		else
			dbgprintf("read(%d,%lu) - %s\n", RSYSLOG_BIO_number_read(bio),
					(unsigned long)argi, RSYSLOG_BIO_method_name(bio));
		break;
	case BIO_CB_WRITE:
		if (bio->method->type & BIO_TYPE_DESCRIPTOR)
			dbgprintf("write(%d,%lu) - %s fd=%d\n",
				RSYSLOG_BIO_number_written(bio), (unsigned long)argi,
				RSYSLOG_BIO_method_name(bio), RSYSLOG_BIO_number_written(bio));
		else
			dbgprintf("write(%d,%lu) - %s\n",
					RSYSLOG_BIO_number_written(bio),
					(unsigned long)argi,
					RSYSLOG_BIO_method_name(bio));
		break;
#else
	case BIO_CB_READ:
		dbgprintf("read %s\n", RSYSLOG_BIO_method_name(bio));
		break;
	case BIO_CB_WRITE:
		dbgprintf("write %s\n", RSYSLOG_BIO_method_name(bio));
		break;
#endif
	case BIO_CB_PUTS:
		dbgprintf("puts() - %s\n", RSYSLOG_BIO_method_name(bio));
		break;
	case BIO_CB_GETS:
		dbgprintf("gets(%lu) - %s\n", (unsigned long)argi,
			RSYSLOG_BIO_method_name(bio));
		break;
	case BIO_CB_CTRL:
		dbgprintf("ctrl(%lu) - %s\n", (unsigned long)argi,
			RSYSLOG_BIO_method_name(bio));
		break;
	case BIO_CB_RETURN | BIO_CB_READ:
		dbgprintf("read return %ld\n", ret);
		break;
	case BIO_CB_RETURN | BIO_CB_WRITE:
		dbgprintf("write return %ld\n", ret);
		break;
	case BIO_CB_RETURN | BIO_CB_GETS:
		dbgprintf("gets return %ld\n", ret);
		break;
	case BIO_CB_RETURN | BIO_CB_PUTS:
		dbgprintf("puts return %ld\n", ret);
		break;
	case BIO_CB_RETURN | BIO_CB_CTRL:
		dbgprintf("ctrl return %ld\n", ret);
		break;
	default:
		dbgprintf("bio callback - unknown type (%d)\n", cmd);
		break;
	}

	return (r);
}


/* Convert a fingerprint to printable data. The  conversion is carried out
 * according IETF I-D syslog-transport-tls-12. The fingerprint string is
 * returned in a new cstr object. It is the caller's responsibility to
 * destruct that object.
 * rgerhards, 2008-05-08
 */
static rsRetVal
GenFingerprintStr(uchar *pFingerprint, size_t sizeFingerprint, cstr_t **ppStr)
{
	cstr_t *pStr = NULL;
	uchar buf[4];
	size_t i;
	DEFiRet;

	CHKiRet(rsCStrConstruct(&pStr));
	CHKiRet(rsCStrAppendStrWithLen(pStr, (uchar*)"SHA1", 4));
	for(i = 0 ; i < sizeFingerprint ; ++i) {
		snprintf((char*)buf, sizeof(buf), ":%2.2X", pFingerprint[i]);
		CHKiRet(rsCStrAppendStrWithLen(pStr, buf, 3));
	}
	cstrFinalize(pStr);

	*ppStr = pStr;

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pStr != NULL)
			rsCStrDestruct(&pStr);
	}
	RETiRet;
}


/* globally initialize OpenSSL  */
static rsRetVal
osslGlblInit(void)
{
	DEFiRet;
	DBGPRINTF("openssl: entering osslGlblInit\n");
	const char *caFile, *certFile, *keyFile;

	/* Setup OpenSSL library */
	if((opensslh_THREAD_setup() == 0) || !SSL_library_init()) {
		LogError(0, RS_RET_NO_ERRCODE, "Error: OpenSSL initialization failed!");
	}

	/* Load readable error strings */
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	ERR_load_crypto_strings();

	/* Setup certificates */
	caFile = (const char *) glbl.GetDfltNetstrmDrvrCAF();
	if(caFile == NULL) {
		LogError(0, RS_RET_CA_CERT_MISSING, "Error: CA certificate is not set, cannot continue");
		ABORT_FINALIZE(RS_RET_CA_CERT_MISSING);
	}
	certFile = (const char *) glbl.GetDfltNetstrmDrvrCertFile();
	if(certFile == NULL) {
		LogError(0, RS_RET_CERT_MISSING, "Error: Certificate file is not set, cannot continue");
		ABORT_FINALIZE(RS_RET_CERT_MISSING);

	}
	keyFile = (const char *) glbl.GetDfltNetstrmDrvrKeyFile();
	if(keyFile == NULL) {
		LogError(0, RS_RET_CERTKEY_MISSING, "Error: Key file is not set, cannot continue");
		ABORT_FINALIZE(RS_RET_CERTKEY_MISSING);

	}

	/* Create main CTX Object */
	ctx = SSL_CTX_new(SSLv23_method());
	if(SSL_CTX_load_verify_locations(ctx, caFile, NULL) != 1) {
		LogError(0, RS_RET_NO_ERRCODE, "Error: CA certificate could not be accessed."
				" Is the file at the right path? And do we have the permissions?");
		ABORT_FINALIZE(RS_RET_NO_ERRCODE);
	}
	if(SSL_CTX_use_certificate_file(ctx, certFile, SSL_FILETYPE_PEM) != 1) {
		LogError(0, RS_RET_NO_ERRCODE, "Error: Certificate file could not be "
				"accessed. Is the file at the right path? And do we have the "
				"permissions?");
		ABORT_FINALIZE(RS_RET_NO_ERRCODE);
	}
	if(SSL_CTX_use_PrivateKey_file(ctx, keyFile, SSL_FILETYPE_PEM) != 1) {
		LogError(0, RS_RET_NO_ERRCODE, "Error: Key file could not be accessed. "
				"Is the file at the right path? And do we have the permissions?");
		ABORT_FINALIZE(RS_RET_NO_ERRCODE);
	}

	/* Set CTX Options */
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);		/* Disable insecure SSLv2 Protocol */
	SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv3);		/* Disable insecure SSLv3 Protocol */
	SSL_CTX_sess_set_cache_size(ctx,1024);			/* TODO: make configurable? */

	/* TODO: DO ONLY SUPPORT DEFAULT CIPHERS YET
	SSL_CTX_set_cipher_list(ctx,"ALL");			Support all ciphers */
// TODO MORE NEEDED 	SSL_CTX_set_ecdh_auto(ctx, 1);
	/* Enable Support for automatic EC temporary key parameter selection. */

	/* Set default VERIFY Options for OpenSSL CTX - and CALLBACK */
	SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, verify_callback);

	SSL_CTX_set_timeout(ctx, 30);	/* Default Session Timeout, TODO: Make configureable */
	SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

	bGlblSrvrInitDone = 1;

finalize_it:
	RETiRet;
}

/* try to receive a record from the remote peer. This works with
 * our own abstraction and handles local buffering and EAGAIN.
 * See details on local buffering in Rcv(9 header-comment.
 * This function MUST only be called when the local buffer is
 * empty. Calling it otherwise will cause losss of current buffer
 * data.
 * rgerhards, 2008-06-24
 */
rsRetVal
osslRecordRecv(nsd_ossl_t *pThis)
{
	ssize_t lenRcvd;
	DEFiRet;
	int err;

	ISOBJ_TYPE_assert(pThis, nsd_ossl);
	DBGPRINTF("osslRecordRecv: start\n");

	lenRcvd = SSL_read(pThis->ssl, pThis->pszRcvBuf, NSD_OSSL_MAX_RCVBUF);
	if(lenRcvd > 0) {
		pThis->lenRcvBuf = lenRcvd;
		pThis->ptrRcvBuf = 0;
	} else {
		err = SSL_get_error(pThis->ssl, lenRcvd);
		if(	err == SSL_ERROR_ZERO_RETURN ) {
			DBGPRINTF("osslRecordRecv: SSL_ERROR_ZERO_RETURN received, connection may closed already\n");
			ABORT_FINALIZE(RS_RET_RETRY);
		}
		else if(err != SSL_ERROR_WANT_READ &&
			err != SSL_ERROR_WANT_WRITE) {
			/* Output error and abort */
			osslLastSSLErrorMsg(lenRcvd, pThis->ssl, LOG_ERR, "osslRecordRecv");
			ABORT_FINALIZE(RS_RET_NO_ERRCODE);
		} else {
			DBGPRINTF("osslRecordRecv: SSL_get_error = %d\n", err);
			pThis->rtryCall =  osslRtry_recv;
			pThis->rtryOsslErr = err; /* Store SSL ErrorCode into*/
			ABORT_FINALIZE(RS_RET_RETRY);
		}
	}

// TODO: Check if MORE retry logic needed?

finalize_it:
	dbgprintf("osslRecordRecv return. nsd %p, iRet %d, lenRcvd %d, lenRcvBuf %d, ptrRcvBuf %d\n",
	pThis, iRet, (int) lenRcvd, pThis->lenRcvBuf, pThis->ptrRcvBuf);
	RETiRet;
}

static rsRetVal
osslInitSession(nsd_ossl_t *pThis) /* , nsd_ossl_t *pServer) */
{
	DEFiRet;
	BIO *client;

	nsd_ptcp_t *pPtcp = (nsd_ptcp_t*) pThis->pTcp;

	if(!(pThis->ssl = SSL_new(ctx))) {
		osslLastSSLErrorMsg(0, pThis->ssl, LOG_ERR, "osslInitSession");
	}

	if (pThis->authMode != OSSL_AUTH_CERTANON) {
		dbgprintf("osslInitSession: enable certificate checking\n");
		/* Enable certificate valid checking */
		SSL_set_verify(pThis->ssl, SSL_VERIFY_PEER|SSL_VERIFY_FAIL_IF_NO_PEER_CERT, verify_callback);
		SSL_set_verify_depth(pThis->ssl, 4);
	}

	/* Create BIO from ptcp socket! */
	client = BIO_new_socket(pPtcp->sock, BIO_CLOSE /*BIO_NOCLOSE*/);
	dbgprintf("osslInitSession: Init client BIO[%p] done\n", (void *)client);

	/* Set debug Callback for client BIO as well! */
	BIO_set_callback(client, BIO_debug_callback);

/* TODO: still needed? Set to NON blocking ! */
BIO_set_nbio( client, 1 );

	SSL_set_bio(pThis->ssl, client, client);
	SSL_set_accept_state(pThis->ssl); /* sets ssl to work in server mode. */

	pThis->bHaveSess = 1;
	pThis->sslState = osslServer; /*set Server state */

	/* we are done */
	FINALIZE;

finalize_it:
	RETiRet;
}


/* Check the peer's ID in fingerprint auth mode.
 * rgerhards, 2008-05-22
 */
static rsRetVal
osslChkPeerFingerprint(nsd_ossl_t *pThis, X509 *pCert)
{
	unsigned int n;
	uchar fingerprint[20 /*EVP_MAX_MD_SIZE**/];
	size_t size;
	cstr_t *pstrFingerprint = NULL;
	int bFoundPositiveMatch;
	permittedPeers_t *pPeer;
	const EVP_MD *fdig = EVP_sha1();
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_ossl);

	/* obtain the SHA1 fingerprint */
	size = sizeof(fingerprint);
	if (!X509_digest(pCert,fdig,fingerprint,&n)) {
		dbgprintf("osslChkPeerFingerprint: error X509cert is not valid!\n");
		ABORT_FINALIZE(RS_RET_INVALID_FINGERPRINT);
	}

	CHKiRet(GenFingerprintStr(fingerprint, size, &pstrFingerprint));
	dbgprintf("osslChkPeerFingerprint: peer's certificate SHA1 fingerprint: %s\n",
		cstrGetSzStrNoNULL(pstrFingerprint));

	/* now search through the permitted peers to see if we can find a permitted one */
	bFoundPositiveMatch = 0;
	pPeer = pThis->pPermPeers;
	while(pPeer != NULL && !bFoundPositiveMatch) {
/* // VERBOSE
dbgprintf("osslChkPeerFingerprint: Compare '%s'  against Peerfingerprint '%s'\n",
	pPeer->pszID, cstrGetSzStrNoNULL(pstrFingerprint));
*/
		if(!rsCStrSzStrCmp(pstrFingerprint, pPeer->pszID, strlen((char*) pPeer->pszID))) {
// VERBOSE
dbgprintf("osslChkPeerFingerprint: peer's certificate MATCH found: %s\n", pPeer->pszID);

			bFoundPositiveMatch = 1;
		} else {
			pPeer = pPeer->pNext;
		}
	}

	if(!bFoundPositiveMatch) {
		dbgprintf("osslChkPeerFingerprint: invalid peer fingerprint, not permitted to talk to it\n");
		if(pThis->bReportAuthErr == 1) {
			errno = 0;
			LogError(0, RS_RET_INVALID_FINGERPRINT, "error: peer fingerprint '%s' unknown - we are "
					"not permitted to talk to it", cstrGetSzStrNoNULL(pstrFingerprint));
			pThis->bReportAuthErr = 0;
		}
		ABORT_FINALIZE(RS_RET_INVALID_FINGERPRINT);
	}

finalize_it:
	if(pstrFingerprint != NULL)
		cstrDestruct(&pstrFingerprint);
	RETiRet;
}


/* Perform a match on ONE peer name obtained from the certificate. This name
 * is checked against the set of configured credentials. *pbFoundPositiveMatch is
 * set to 1 if the ID matches. *pbFoundPositiveMatch must have been initialized
 * to 0 by the caller (this is a performance enhancement as we expect to be
 * called multiple times).
 * TODO: implemet wildcards?
 * rgerhards, 2008-05-26
 */
static rsRetVal
osslChkOnePeerName(nsd_ossl_t *pThis, X509 *pCert, uchar *pszPeerID, int *pbFoundPositiveMatch)
{
	permittedPeers_t *pPeer;
	#if OPENSSL_VERSION_NUMBER >= 0x10002000L
	int osslRet;
	#endif
	char *x509name = NULL;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_ossl);
	assert(pszPeerID != NULL);
	assert(pbFoundPositiveMatch != NULL);

	/* Obtain Namex509 name from subject */
	x509name = X509_NAME_oneline(RSYSLOG_X509_NAME_oneline(pCert), NULL, 0);

	if(pThis->pPermPeers) { /* do we have configured peer IDs? */
		pPeer = pThis->pPermPeers;
		while(pPeer != NULL) {
/* // VERBOSE
dbgprintf("osslChkOnePeerName: Compare '%s'  against Peercert '%s'\n",
	pPeer->pszID, x509name);
*/
			CHKiRet(net.PermittedPeerWildcardMatch(pPeer, pszPeerID, pbFoundPositiveMatch));
			if(*pbFoundPositiveMatch)
				break;

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
			/* if we did not succeed so far, try ossl X509_check_host
			* ( Includes check against SubjectAlternativeName )
			*/
			osslRet = X509_check_host(	pCert, (const char*)pPeer->pszID,
							strlen((const char*)pPeer->pszID), 0, NULL);
			if ( osslRet == 1 ) {
				/* Found Peer cert in allowed Peerslist */
				dbgprintf("osslChkOnePeerName: Client ('%s') is allowed (X509_check_host)\n",
					x509name);
				*pbFoundPositiveMatch = 1;
				break;
			} else if ( osslRet < 0 ) {
				osslLastSSLErrorMsg(osslRet, pThis->ssl, LOG_ERR, "osslChkOnePeerName");
				ABORT_FINALIZE(RS_RET_NO_ERRCODE);
			}
#endif
			/* Check next peer */
			pPeer = pPeer->pNext;
		}
	} else {
		/* we do not have configured peer IDs, so we use defaults */
		if(   pThis->pszConnectHost
		   && !strcmp((char*)pszPeerID, (char*)pThis->pszConnectHost)) {
			*pbFoundPositiveMatch = 1;
		}
	}
finalize_it:
	if (x509name != NULL){
		OPENSSL_free(x509name);
	}

	RETiRet;
}


/* Check the peer's ID in name auth mode.
 */
static rsRetVal
osslChkPeerName(nsd_ossl_t *pThis, X509 *pCert)
{
	uchar lnBuf[256];
	int bFoundPositiveMatch;
	cstr_t *pStr = NULL;
	char *x509name = NULL;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_ossl);

	bFoundPositiveMatch = 0;
	CHKiRet(rsCStrConstruct(&pStr));

	/* Obtain Namex509 name from subject */
	x509name = X509_NAME_oneline(RSYSLOG_X509_NAME_oneline(pCert), NULL, 0);

	dbgprintf("osslChkPeerName: checking - peername '%s'\n", x509name);
	snprintf((char*)lnBuf, sizeof(lnBuf), "name: %s; ", x509name);
	CHKiRet(rsCStrAppendStr(pStr, lnBuf));
	CHKiRet(osslChkOnePeerName(pThis, pCert, (uchar*)x509name, &bFoundPositiveMatch));

	if(!bFoundPositiveMatch) {
		dbgprintf("osslChkPeerName: invalid peername, not permitted to talk to it\n");
		if(pThis->bReportAuthErr == 1) {
			cstrFinalize(pStr);
			errno = 0;
			LogError(0, RS_RET_INVALID_FINGERPRINT, "error: peer name not authorized -  "
					"not permitted to talk to it. Names: %s",
					cstrGetSzStrNoNULL(pStr));
			pThis->bReportAuthErr = 0;
		}
		ABORT_FINALIZE(RS_RET_INVALID_FINGERPRINT);
	} else {
		dbgprintf("osslChkPeerName: permitted to talk!\n");
	}

finalize_it:
	if (x509name != NULL){
		OPENSSL_free(x509name);
	}

	if(pStr != NULL)
		rsCStrDestruct(&pStr);
	RETiRet;
}


/* check the ID of the remote peer - used for both fingerprint and
 * name authentication.
 */
static rsRetVal
osslChkPeerID(nsd_ossl_t *pThis)
{
	X509* certpeer;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_ossl);

	/* Get peer certificate from SSL */
	certpeer = SSL_get_peer_certificate(pThis->ssl);
	if ( certpeer == NULL ) {
		if(pThis->bReportAuthErr == 1) {
			errno = 0;
			LogError(0, RS_RET_TLS_NO_CERT, "error: peer did not provide a certificate, "
					"not permitted to talk to it");
			pThis->bReportAuthErr = 0;
		}
		ABORT_FINALIZE(RS_RET_TLS_NO_CERT);
	}

	/* Now we see which actual authentication code we must call.  */
	if(pThis->authMode == OSSL_AUTH_CERTFINGERPRINT) {
		CHKiRet(osslChkPeerFingerprint(pThis, certpeer));
	} else {
		assert(pThis->authMode == OSSL_AUTH_CERTNAME);
		CHKiRet(osslChkPeerName(pThis, certpeer));
	}

finalize_it:
	RETiRet;
}


/* Verify the validity of the remote peer's certificate.
 */
static rsRetVal
osslChkPeerCertValidity(nsd_ossl_t *pThis)
{
	DEFiRet;
	int iVerErr = X509_V_OK;

	ISOBJ_TYPE_assert(pThis, nsd_ossl);

	iVerErr = SSL_get_verify_result(pThis->ssl);
	if (iVerErr != X509_V_OK) {
		if (iVerErr == X509_V_ERR_CERT_HAS_EXPIRED) {
			LogError(0, RS_RET_CERT_EXPIRED, "not permitted to talk to peer: certificate expired: %s",
				X509_verify_cert_error_string(iVerErr));
			ABORT_FINALIZE(RS_RET_CERT_EXPIRED);
		} else {
			LogError(0, RS_RET_CERT_INVALID, "not permitted to talk to peer: "
				"certificate validation failed: %s", X509_verify_cert_error_string(iVerErr));
			ABORT_FINALIZE(RS_RET_CERT_INVALID);
		}
	}

finalize_it:
	RETiRet;
}


/* check if it is OK to talk to the remote peer
 * rgerhards, 2008-05-21
 */
rsRetVal
osslChkPeerAuth(nsd_ossl_t *pThis)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_ossl);

	/* call the actual function based on current auth mode */
	switch(pThis->authMode) {
		case OSSL_AUTH_CERTNAME:
			/* if we check the name, we must ensure the cert is valid */
			dbgprintf("osslChkPeerAuth: Check peer certname[%p]\n", (void *)pThis->ssl);
			CHKiRet(osslChkPeerCertValidity(pThis));
			CHKiRet(osslChkPeerID(pThis));
			break;
		case OSSL_AUTH_CERTFINGERPRINT:
			dbgprintf("osslChkPeerAuth: Check peer fingerprint[%p]\n", (void *)pThis->ssl);
			CHKiRet(osslChkPeerID(pThis));
			break;
		case OSSL_AUTH_CERTVALID:
			dbgprintf("osslChkPeerAuth: Check peer valid[%p]\n", (void *)pThis->ssl);
			CHKiRet(osslChkPeerCertValidity(pThis));
			break;
		case OSSL_AUTH_CERTANON:
			FINALIZE;
			break;
	}

finalize_it:
	RETiRet;
}


/* globally de-initialize OpenSSL */
static rsRetVal
osslGlblExit(void)
{
	DEFiRet;
	DBGPRINTF("openssl: entering osslGlblExit\n");
	ENGINE_cleanup();
	ERR_free_strings();
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	RETiRet;
}


/* end a OpenSSL session
 * The function checks if we have a session and ends it only if so. So it can
 * always be called, even if there currently is no session.
 */
static rsRetVal
osslEndSess(nsd_ossl_t *pThis)
{
	DEFiRet;
	int ret;
	int err;

	/* try closing SSL Connection */
	if(pThis->bHaveSess) {
		DBGPRINTF("osslEndSess: closing SSL Session ...\n");
		ret = SSL_shutdown(pThis->ssl);
		if (ret <= 0) {
			err = SSL_get_error(pThis->ssl, ret);
			DBGPRINTF("osslEndSess: shutdown failed with err = %d, forcing ssl shutdown!\n", err);

			/* ignore those SSL Errors on shutdown */
			if(	err != SSL_ERROR_SYSCALL &&
					err != SSL_ERROR_ZERO_RETURN &&
					err != SSL_ERROR_WANT_READ &&
					err != SSL_ERROR_WANT_WRITE) {
				/* Output Warning only */
				osslLastSSLErrorMsg(ret, pThis->ssl, LOG_WARNING, "osslEndSess");
			}

			DBGPRINTF("osslEndSess: session closed (un)successfully \n");
		} else {
			DBGPRINTF("osslEndSess: session closed successfully \n");
		}

		/* Session closed */
		pThis->bHaveSess = 0;
		SSL_free(pThis->ssl);
		pThis->ssl = NULL;
	}

	RETiRet;
}
/* ---------------------------- end OpenSSL specifics ---------------------------- */


/* Standard-Constructor */
BEGINobjConstruct(nsd_ossl) /* be sure to specify the object type also in END macro! */
	iRet = nsd_ptcp.Construct(&pThis->pTcp);
	pThis->bReportAuthErr = 1;
ENDobjConstruct(nsd_ossl)


/* destructor for the nsd_ossl object */
PROTOTYPEobjDestruct(nsd_ossl);
BEGINobjDestruct(nsd_ossl) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(nsd_ossl)
	if(pThis->iMode == 1) {
		osslEndSess(pThis);
	}

	if(pThis->pTcp != NULL) {
		nsd_ptcp.Destruct(&pThis->pTcp);
	}

	if(pThis->pszConnectHost != NULL) {
		free(pThis->pszConnectHost);
	}

	if(pThis->pszRcvBuf == NULL) {
		free(pThis->pszRcvBuf);
	}
ENDobjDestruct(nsd_ossl)


/* Set the driver mode. For us, this has the following meaning:
 * 0 - work in plain tcp mode, without tls (e.g. before a STARTTLS)
 * 1 - work in TLS mode
 * rgerhards, 2008-04-28
 */
static rsRetVal
SetMode(nsd_t *pNsd, int mode)
{
	DEFiRet;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_ossl);
	if(mode != 0 && mode != 1) {
		LogError(0, RS_RET_INVALID_DRVR_MODE, "error: driver mode %d not supported by"
				" ossl netstream driver", mode);
	}
	pThis->iMode = mode;

	RETiRet;
}

/* Set the authentication mode. For us, the following is supported:
 * anon - no certificate checks whatsoever (discouraged, but supported)
 * x509/certvalid - (just) check certificate validity
 * x509/fingerprint - certificate fingerprint
 * x509/name - cerfificate name check
 * mode == NULL is valid and defaults to x509/name
 * rgerhards, 2008-05-16
 */
static rsRetVal
SetAuthMode(nsd_t *pNsd, uchar *mode)
{
	DEFiRet;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_ossl);
	if(mode == NULL || !strcasecmp((char*)mode, "x509/name")) {
		pThis->authMode = OSSL_AUTH_CERTNAME;
	} else if(!strcasecmp((char*) mode, "x509/fingerprint")) {
		pThis->authMode = OSSL_AUTH_CERTFINGERPRINT;
	} else if(!strcasecmp((char*) mode, "x509/certvalid")) {
		pThis->authMode = OSSL_AUTH_CERTVALID;
	} else if(!strcasecmp((char*) mode, "anon")) {
		pThis->authMode = OSSL_AUTH_CERTANON;
	} else {
		LogError(0, RS_RET_VALUE_NOT_SUPPORTED, "error: authentication mode '%s' not supported by "
				"ossl netstream driver", mode);
		ABORT_FINALIZE(RS_RET_VALUE_NOT_SUPPORTED);
	}

/* TODO: clear stored IDs! */

finalize_it:
	RETiRet;
}


/* Set permitted peers. It is depending on the auth mode if this are
 * fingerprints or names. -- rgerhards, 2008-05-19
 */
static rsRetVal
SetPermPeers(nsd_t *pNsd, permittedPeers_t *pPermPeers)
{
	DEFiRet;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_ossl);
	if(pPermPeers == NULL)
		FINALIZE;

	if(pThis->authMode != OSSL_AUTH_CERTFINGERPRINT && pThis->authMode != OSSL_AUTH_CERTNAME) {
		LogError(0, RS_RET_VALUE_NOT_IN_THIS_MODE, "authentication not supported by "
				"ossl netstream driver in the configured authentication mode - ignored");
		ABORT_FINALIZE(RS_RET_VALUE_NOT_IN_THIS_MODE);
	}
	pThis->pPermPeers = pPermPeers;

finalize_it:
	RETiRet;
}


/* Provide access to the underlying OS socket. This is primarily
 * useful for other drivers (like nsd_ossl) who utilize ourselfs
 * for some of their functionality. -- rgerhards, 2008-04-18
 */
static rsRetVal
SetSock(nsd_t *pNsd, int sock)
{
	DEFiRet;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;
	ISOBJ_TYPE_assert((pThis), nsd_ossl);
	assert(sock >= 0);

	DBGPRINTF("SetSock for [%p]: Setting sock %d\n", pNsd, sock);
	nsd_ptcp.SetSock(pThis->pTcp, sock);

	RETiRet;
}


/* Keep Alive Options
 */
static rsRetVal
SetKeepAliveIntvl(nsd_t *pNsd, int keepAliveIntvl)
{
	DEFiRet;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_ossl);
	assert(keepAliveIntvl >= 0);

	dbgprintf("SetKeepAliveIntvl: keepAliveIntvl=%d\n", keepAliveIntvl);
	nsd_ptcp.SetKeepAliveIntvl(pThis->pTcp, keepAliveIntvl);

	RETiRet;
}


/* Keep Alive Options
 */
static rsRetVal
SetKeepAliveProbes(nsd_t *pNsd, int keepAliveProbes)
{
	DEFiRet;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_ossl);
	assert(keepAliveProbes >= 0);

	dbgprintf("SetKeepAliveProbes: keepAliveProbes=%d\n", keepAliveProbes);
	nsd_ptcp.SetKeepAliveProbes(pThis->pTcp, keepAliveProbes);

	RETiRet;
}


/* Keep Alive Options
 */
static rsRetVal
SetKeepAliveTime(nsd_t *pNsd, int keepAliveTime)
{
	DEFiRet;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_ossl);
	assert(keepAliveTime >= 0);

	dbgprintf("SetKeepAliveTime: keepAliveTime=%d\n", keepAliveTime);
	nsd_ptcp.SetKeepAliveTime(pThis->pTcp, keepAliveTime);

	RETiRet;
}


/* abort a connection. This is meant to be called immediately
 * before the Destruct call. -- rgerhards, 2008-03-24
 */
static rsRetVal
Abort(nsd_t *pNsd)
{
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;
	DEFiRet;

	ISOBJ_TYPE_assert((pThis), nsd_ossl);

	if(pThis->iMode == 0) {
		nsd_ptcp.Abort(pThis->pTcp);
	}

	RETiRet;
}



/* initialize the tcp socket for a listner
 * Here, we use the ptcp driver - because there is nothing special
 * at this point with OpenSSL. Things become special once we accept
 * a session, but not during listener setup.
 */
static rsRetVal
LstnInit(netstrms_t *pNS, void *pUsr, rsRetVal(*fAddLstn)(void*,netstrm_t*),
	 uchar *pLstnPort, uchar *pLstnIP, int iSessMax, uchar *pszLstnPortFileName)
{
	DEFiRet;

	dbgprintf("LstnInit for openssl: entering LstnInit (%p) for %s:%s SessMax=%d\n",
		fAddLstn, pLstnIP, pLstnPort, iSessMax);

	/* Init TCP Listener using base ptcp class */
	iRet = nsd_ptcp.LstnInit(pNS, pUsr, fAddLstn, pLstnPort, pLstnIP,
			iSessMax, pszLstnPortFileName);
	RETiRet;
}


/* This function checks if the connection is still alive - well, kind of...
 * This is a dummy here. For details, check function common in ptcp driver.
 * rgerhards, 2008-06-09
 */
static rsRetVal
CheckConnection(nsd_t __attribute__((unused)) *pNsd)
{
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_ossl);

	dbgprintf("CheckConnection for %p\n", pNsd);
	return nsd_ptcp.CheckConnection(pThis->pTcp);
}


/* get the remote hostname. The returned hostname must be freed by the caller.
 * rgerhards, 2008-04-25
 */
static rsRetVal
GetRemoteHName(nsd_t *pNsd, uchar **ppszHName)
{
	DEFiRet;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_ossl);
	iRet = nsd_ptcp.GetRemoteHName(pThis->pTcp, ppszHName);
	dbgprintf("GetRemoteHName for %p = %s\n", pNsd, *ppszHName);
	RETiRet;
}


/* Provide access to the sockaddr_storage of the remote peer. This
 * is needed by the legacy ACL system. --- gerhards, 2008-12-01
 */
static rsRetVal
GetRemAddr(nsd_t *pNsd, struct sockaddr_storage **ppAddr)
{
	DEFiRet;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_ossl);
	iRet = nsd_ptcp.GetRemAddr(pThis->pTcp, ppAddr);
	dbgprintf("GetRemAddr for %p\n", pNsd);
	RETiRet;
}


/* get the remote host's IP address. Caller must Destruct the object. */
static rsRetVal
GetRemoteIP(nsd_t *pNsd, prop_t **ip)
{
	DEFiRet;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_ossl);
	iRet = nsd_ptcp.GetRemoteIP(pThis->pTcp, ip);
	dbgprintf("GetRemoteIP for %p\n", pNsd);
	RETiRet;
}


/* Perform all necessary checks after Handshake
 */
rsRetVal
osslPostHandshakeCheck(nsd_ossl_t *pNsd)
{
	DEFiRet;
	char szDbg[255];
	const SSL_CIPHER* sslCipher;

	/* Some extra output for debugging openssl */
	if (SSL_get_shared_ciphers(pNsd->ssl,szDbg, sizeof szDbg) != NULL)
		dbgprintf("osslPostHandshakeCheck: Debug Shared ciphers = %s\n", szDbg);
	sslCipher = (const SSL_CIPHER*) SSL_get_current_cipher(pNsd->ssl);
	if (sslCipher != NULL)
		dbgprintf("osslPostHandshakeCheck: Debug Version: %s Name: %s\n",
			SSL_CIPHER_get_version(sslCipher), SSL_CIPHER_get_name(sslCipher));

	FINALIZE;

finalize_it:
	RETiRet;
}


/* Perform all necessary actions for Handshake
 */
rsRetVal
osslHandshakeCheck(nsd_ossl_t *pNsd)
{
	DEFiRet;
	int res, resErr;
	dbgprintf("osslHandshakeCheck: Starting TLS Handshake for ssl[%p]\n", (void *)pNsd->ssl);

	if (pNsd->sslState == osslServer) {
		/* Handle Server SSL Object */
		if((res = SSL_accept(pNsd->ssl)) <= 0) {
			/* Obtain SSL Error code */
			resErr = SSL_get_error(pNsd->ssl, res);
			if(	resErr == SSL_ERROR_WANT_READ ||
				resErr == SSL_ERROR_WANT_WRITE) {
				pNsd->rtryCall = osslRtry_handshake;
				pNsd->rtryOsslErr = resErr; /* Store SSL ErrorCode into*/
				dbgprintf("osslHandshakeCheck: OpenSSL Server handshake does not complete "
					"immediately - setting to retry (this is OK and normal)\n");
				FINALIZE;
			} else if(resErr == SSL_ERROR_SYSCALL) {
				dbgprintf("osslHandshakeCheck: OpenSSL Server handshake failed with SSL_ERROR_SYSCALL "
					"- Aborting handshake.\n");
				osslLastSSLErrorMsg(res, pNsd->ssl, LOG_WARNING, "osslHandshakeCheck Server");
				ABORT_FINALIZE(RS_RET_NO_ERRCODE);
			} else {
				osslLastSSLErrorMsg(res, pNsd->ssl, LOG_ERR, "osslHandshakeCheck Server");
				ABORT_FINALIZE(RS_RET_NO_ERRCODE);
			}
		}
	} else {
		/* Handle Client SSL Object */
		if((res = SSL_do_handshake(pNsd->ssl)) <= 0) {
			/* Obtain SSL Error code */
			resErr = SSL_get_error(pNsd->ssl, res);
			if(	resErr == SSL_ERROR_WANT_READ ||
				resErr == SSL_ERROR_WANT_WRITE) {
				pNsd->rtryCall = osslRtry_handshake;
				pNsd->rtryOsslErr = resErr; /* Store SSL ErrorCode into*/
				dbgprintf("osslHandshakeCheck: OpenSSL Client handshake does not complete "
					"immediately - setting to retry (this is OK and normal)\n");
				FINALIZE;
			} else if(resErr == SSL_ERROR_SYSCALL) {
				dbgprintf("osslHandshakeCheck: OpenSSL Client handshake failed with SSL_ERROR_SYSCALL "
					"- Aborting handshake.\n");
				osslLastSSLErrorMsg(res, pNsd->ssl, LOG_WARNING, "osslHandshakeCheck Client");
				ABORT_FINALIZE(RS_RET_NO_ERRCODE /*RS_RET_RETRY*/);
			} else {
				osslLastSSLErrorMsg(res, pNsd->ssl, LOG_ERR, "osslHandshakeCheck Client");
				ABORT_FINALIZE(RS_RET_NO_ERRCODE);
			}
		}
	}

	/* Do post handshake stuff */
	CHKiRet(osslPostHandshakeCheck(pNsd));

	/* Now check authorization */
	CHKiRet(osslChkPeerAuth(pNsd));
finalize_it:
	if(iRet == RS_RET_OK) {
		/* If no error occured, set socket to SSL mode */
		pNsd->iMode = 1;
	}

	RETiRet;
}

/* accept an incoming connection request - here, we do the usual accept
 * handling. TLS specific handling is done thereafter (and if we run in TLS
 * mode at this time).
 * rgerhards, 2008-04-25
 */
static rsRetVal
AcceptConnReq(nsd_t *pNsd, nsd_t **ppNew)
{
	DEFiRet;
	nsd_ossl_t *pNew = NULL;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_ossl);
	CHKiRet(nsd_osslConstruct(&pNew));
	CHKiRet(nsd_ptcp.Destruct(&pNew->pTcp));
	dbgprintf("AcceptConnReq for [%p]: Accepting connection ... \n", (void *)pThis);
	CHKiRet(nsd_ptcp.AcceptConnReq(pThis->pTcp, &pNew->pTcp));

	if(pThis->iMode == 0) {
		/*we are in non-TLS mode, so we are done */
		DBGPRINTF("AcceptConnReq: NOT in TLS mode!\n");
		*ppNew = (nsd_t*) pNew;
		FINALIZE;
	}

	/* If we reach this point, we are in TLS mode */
	CHKiRet(osslInitSession(pNew)); /*, pThis));*/
	pNew->authMode = pThis->authMode;
	pNew->pPermPeers = pThis->pPermPeers;

	/* We now do the handshake */
	CHKiRet(osslHandshakeCheck(pNew));

	*ppNew = (nsd_t*) pNew;
finalize_it:
	/* Accept appears to be done here */
	if(pNew != NULL) {
		DBGPRINTF("AcceptConnReq: END iRet = %d, pNew=[%p], pNsd->rtryCall=%d\n",
			iRet, pNew, pNew->rtryCall);
	}
	if(iRet != RS_RET_OK) {
		if(pNew != NULL) {
			nsd_osslDestruct(&pNew);
		}
	}
	RETiRet;
}


/* receive data from a tcp socket
 * The lenBuf parameter must contain the max buffer size on entry and contains
 * the number of octets read on exit. This function
 * never blocks, not even when called on a blocking socket. That is important
 * for client sockets, which are set to block during send, but should not
 * block when trying to read data. -- rgerhards, 2008-03-17
 * The function now follows the usual iRet calling sequence.
 * With GnuTLS, we may need to restart a recv() system call. If so, we need
 * to supply the SAME buffer on the retry. We can not assure this, as the
 * caller is free to call us with any buffer location (and in current
 * implementation, it is on the stack and extremely likely to change). To
 * work-around this problem, we allocate a buffer ourselfs and always receive
 * into that buffer. We pass data on to the caller only after we have received it.
 * To save some space, we allocate that internal buffer only when it is actually
 * needed, which means when we reach this function for the first time. To keep
 * the algorithm simple, we always supply data only from the internal buffer,
 * even if it is a single byte. As we have a stream, the caller must be prepared
 * to accept messages in any order, so we do not need to take care about this.
 * Please note that the logic also forces us to do some "faking" in select(), as
 * we must provide a fake "is ready for readign" status if we have data inside our
 * buffer. -- rgerhards, 2008-06-23
 */
static rsRetVal
Rcv(nsd_t *pNsd, uchar *pBuf, ssize_t *pLenBuf, int *const oserr)
{
	DEFiRet;
	ssize_t iBytesCopy; /* how many bytes are to be copied to the client buffer? */
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_ossl);
	DBGPRINTF("Rcv for %p\n", pNsd);

	if(pThis->bAbortConn)
		ABORT_FINALIZE(RS_RET_CONNECTION_ABORTREQ);

	if(pThis->iMode == 0) {
		CHKiRet(nsd_ptcp.Rcv(pThis->pTcp, pBuf, pLenBuf, oserr));
		FINALIZE;
	}

	/* --- in TLS mode now --- */

	/* Buffer logic applies only if we are in TLS mode. Here we
	 * assume that we will switch from plain to TLS, but never back. This
	 * assumption may be unsafe, but it is the model for the time being and I
	 * do not see any valid reason why we should switch back to plain TCP after
	 * we were in TLS mode. However, in that case we may lose something that
	 * is already in the receive buffer ... risk accepted. -- rgerhards, 2008-06-23
	 */

	if(pThis->pszRcvBuf == NULL) {
		/* we have no buffer, so we need to malloc one */
		CHKmalloc(pThis->pszRcvBuf = MALLOC(NSD_OSSL_MAX_RCVBUF));
		pThis->lenRcvBuf = -1;
	}

	/* now check if we have something in our buffer. If so, we satisfy
	 * the request from buffer contents.
	 */
	if(pThis->lenRcvBuf == -1) { /* no data present, must read */
		CHKiRet(osslRecordRecv(pThis));
	}

	if(pThis->lenRcvBuf == 0) { /* EOS */
		*oserr = errno;
		ABORT_FINALIZE(RS_RET_CLOSED);
	}

	/* if we reach this point, data is present in the buffer and must be copied */
	iBytesCopy = pThis->lenRcvBuf - pThis->ptrRcvBuf;
	if(iBytesCopy > *pLenBuf) {
		iBytesCopy = *pLenBuf;
	} else {
		pThis->lenRcvBuf = -1; /* buffer will be emptied below */
	}

	memcpy(pBuf, pThis->pszRcvBuf + pThis->ptrRcvBuf, iBytesCopy);
	pThis->ptrRcvBuf += iBytesCopy;
	*pLenBuf = iBytesCopy;

finalize_it:
	if (iRet != RS_RET_OK) {
		if (iRet != RS_RET_RETRY) {
			/* We need to free the receive buffer in error error case unless a retry is wanted. , if we
			 * allocated one. -- rgerhards, 2008-12-03 -- moved here by alorbach, 2015-12-01
			 */
			*pLenBuf = 0;
			free(pThis->pszRcvBuf);
			pThis->pszRcvBuf = NULL;
		} else {
			/* Check for SSL Shutdown */
			if (SSL_get_shutdown(pThis->ssl) == SSL_RECEIVED_SHUTDOWN) {
				dbgprintf("osslRcv received SSL_RECEIVED_SHUTDOWN!\n");
				iRet = RS_RET_CLOSED;

				/* Send Shutdown message back */
				SSL_shutdown(pThis->ssl);
			}
		}
	}
	dbgprintf("osslRcv return. nsd %p, iRet %d, lenRcvBuf %d, ptrRcvBuf %d\n", pThis,
	iRet, pThis->lenRcvBuf, pThis->ptrRcvBuf);
	RETiRet;
}


/* send a buffer. On entry, pLenBuf contains the number of octets to
 * write. On exit, it contains the number of octets actually written.
 * If this number is lower than on entry, only a partial buffer has
 * been written.
 * rgerhards, 2008-03-19
 */
static rsRetVal
Send(nsd_t *pNsd, uchar *pBuf, ssize_t *pLenBuf)
{
	DEFiRet;
	int iSent;
	int err;
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;
	DBGPRINTF("Send for %p\n", pNsd);

	ISOBJ_TYPE_assert(pThis, nsd_ossl);

	if(pThis->bAbortConn)
		ABORT_FINALIZE(RS_RET_CONNECTION_ABORTREQ);

	if(pThis->iMode == 0) {
		CHKiRet(nsd_ptcp.Send(pThis->pTcp, pBuf, pLenBuf));
		FINALIZE;
	}

	while(1) {
		iSent = SSL_write(pThis->ssl, pBuf, *pLenBuf);
		if(iSent > 0) {
			*pLenBuf = iSent;
			break;
		} else {
			err = SSL_get_error(pThis->ssl, iSent);
			if(	err == SSL_ERROR_ZERO_RETURN ) {
				DBGPRINTF("Send: SSL_ERROR_ZERO_RETURN received, retry next time\n");
				ABORT_FINALIZE(RS_RET_RETRY);
			}
			else if(err != SSL_ERROR_WANT_READ &&
				err != SSL_ERROR_WANT_WRITE) {
				/* Output error and abort */
				osslLastSSLErrorMsg(iSent, pThis->ssl, LOG_ERR, "Send");
/*
				LogError(0, RS_RET_NO_ERRCODE, "Error while sending data: "
						"[%d] %s", err, ERR_error_string(err, NULL));
				LogError(0, RS_RET_NO_ERRCODE, "Error is: %s",
						ERR_reason_error_string(err));
*/
				ABORT_FINALIZE(RS_RET_NO_ERRCODE);
			} else {
				/* Check for SSL Shutdown */
				if (SSL_get_shutdown(pThis->ssl) == SSL_RECEIVED_SHUTDOWN) {
					dbgprintf("osslRcv received SSL_RECEIVED_SHUTDOWN!\n");
					ABORT_FINALIZE(RS_RET_CLOSED);
				}
			}
		}
	}
finalize_it:
	RETiRet;
}


/* Enable KEEPALIVE handling on the socket.
 * rgerhards, 2009-06-02
 */
static rsRetVal
EnableKeepAlive(nsd_t *pNsd)
{
	nsd_ossl_t *pThis = (nsd_ossl_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_ossl);
	return nsd_ptcp.EnableKeepAlive(pThis->pTcp);
}


/* open a connection to a remote host (server). With OpenSSL, we always
 * open a plain tcp socket and then, if in TLS mode, do a handshake on it.
 */
static rsRetVal
Connect(nsd_t *pNsd, int family, uchar *port, uchar *host, char *device)
{
	DEFiRet;
	DBGPRINTF("openssl: entering Connect family=%d, device=%s\n", family, device);
	nsd_ossl_t*pThis = (nsd_ossl_t*) pNsd;
	nsd_ptcp_t *pPtcp = (nsd_ptcp_t*) pThis->pTcp;

	BIO *conn;

	ISOBJ_TYPE_assert(pThis, nsd_ossl);
	assert(port != NULL);
	assert(host != NULL);

	CHKiRet(nsd_ptcp.Connect(pThis->pTcp, family, port, host, device));

	if(pThis->iMode == 0) {
		/*we are in non-TLS mode, so we are done */
		DBGPRINTF("Connect: NOT in TLS mode!\n");
		FINALIZE;
	}

	/* Create BIO from ptcp socket! */
	conn = BIO_new_socket(pPtcp->sock, BIO_CLOSE /*BIO_NOCLOSE*/);
	dbgprintf("Connect: Init conn BIO[%p] done\n", (void *)conn);


	/* Set debug Callback for client BIO as well! */
	BIO_set_callback(conn, BIO_debug_callback);

/* TODO: still needed? Set to NON blocking ! */
BIO_set_nbio( conn, 1 );

	/*if we reach this point we are in tls mode */
	DBGPRINTF("Connect: TLS Mode\n");
	if(!(pThis->ssl = SSL_new(ctx))) {
		osslLastSSLErrorMsg(0, pThis->ssl, LOG_ERR, "Connect");
/*		LogError(0, RS_RET_NO_ERRCODE, "Error creating an SSL context"); */
		ABORT_FINALIZE(RS_RET_NO_ERRCODE);
	}
	SSL_set_bio(pThis->ssl, conn, conn);
	SSL_set_connect_state(pThis->ssl); /*sets ssl to work in client mode.*/
	pThis->sslState = osslClient; /*set Client state */
	pThis->bHaveSess = 1;

	/* We now do the handshake */
	iRet = osslHandshakeCheck(pThis);
finalize_it:
	/* Connect appears to be done here */
	dbgprintf("Connect: END iRet = %d, pThis=[%p], pNsd->rtryCall=%d\n",
		iRet, pThis, pThis->rtryCall);
	if(iRet != RS_RET_OK) {
		if(pThis->bHaveSess) {
			pThis->bHaveSess = 0;
			SSL_free(pThis->ssl);
			pThis->ssl = NULL;
		}
	}
	RETiRet;
}


/* Empty wrapper for GNUTLS helper function
 * TODO: implement a similar capability
 */
static rsRetVal
SetGnutlsPriorityString(__attribute__((unused)) nsd_t *pNsd, __attribute__((unused)) uchar *gnutlsPriorityString)
{
	DEFiRet;
	RETiRet;
}

/* queryInterface function */
BEGINobjQueryInterface(nsd_ossl)
CODESTARTobjQueryInterface(nsd_ossl)
	if(pIf->ifVersion != nsdCURR_IF_VERSION) {/* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = (rsRetVal(*)(nsd_t**)) nsd_osslConstruct;
	pIf->Destruct = (rsRetVal(*)(nsd_t**)) nsd_osslDestruct;
	pIf->Abort = Abort;
	pIf->LstnInit = LstnInit;
	pIf->AcceptConnReq = AcceptConnReq;
	pIf->Rcv = Rcv;
	pIf->Send = Send;
	pIf->Connect = Connect;
	pIf->SetSock = SetSock;
	pIf->SetMode = SetMode;
	pIf->SetAuthMode = SetAuthMode;
	pIf->SetPermPeers =SetPermPeers;
	pIf->CheckConnection = CheckConnection;
	pIf->GetRemoteHName = GetRemoteHName;
	pIf->GetRemoteIP = GetRemoteIP;
	pIf->GetRemAddr = GetRemAddr;
	pIf->EnableKeepAlive = EnableKeepAlive;
	pIf->SetKeepAliveIntvl = SetKeepAliveIntvl;
	pIf->SetKeepAliveProbes = SetKeepAliveProbes;
	pIf->SetKeepAliveTime = SetKeepAliveTime;
	pIf->SetGnutlsPriorityString = SetGnutlsPriorityString; /* we don't NEED this interface! */

finalize_it:
ENDobjQueryInterface(nsd_ossl)


/* exit our class
 */
BEGINObjClassExit(nsd_ossl, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(nsd_ossl)
	osslGlblExit();	/* shut down OpenSSL */

	/* release objects we no longer need */
	objRelease(nsd_ptcp, LM_NSD_PTCP_FILENAME);
	objRelease(net, LM_NET_FILENAME);
	objRelease(glbl, CORE_COMPONENT);
	objRelease(datetime, CORE_COMPONENT);
ENDObjClassExit(nsd_ossl)


/* Initialize the nsd_ossl class. Must be called as the very first method
 * before anything else is called inside this class.
 */
BEGINObjClassInit(nsd_ossl, 1, OBJ_IS_LOADABLE_MODULE) /* class, version */
	/* request objects we use */
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(net, LM_NET_FILENAME));
	CHKiRet(objUse(nsd_ptcp, LM_NSD_PTCP_FILENAME));

	/* now do global TLS init stuff */
	CHKiRet(osslGlblInit());
ENDObjClassInit(nsd_ossl)


/* --------------- here now comes the plumbing that makes as a library module --------------- */


BEGINmodExit
CODESTARTmodExit
	nsdsel_osslClassExit();
	nsd_osslClassExit();
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_LIB_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */

	/* Initialize all classes that are in our module - this includes ourselfs */
	CHKiRet(nsd_osslClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */
	CHKiRet(nsdsel_osslClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */
ENDmodInit
/* vi:set ai:
 */
