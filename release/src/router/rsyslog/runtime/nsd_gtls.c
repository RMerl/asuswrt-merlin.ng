/* nsd_gtls.c
 *
 * An implementation of the nsd interface for GnuTLS.
 *
 * Copyright (C) 2007-2016 Rainer Gerhards and Adiscon GmbH.
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
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#if GNUTLS_VERSION_NUMBER <= 0x020b00
#	include <gcrypt.h>
#endif
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
#include "nsdsel_gtls.h"
#include "nsd_gtls.h"
#include "unicode-helper.h"

/* things to move to some better place/functionality - TODO */
#define CRLFILE "crl.pem"


#if GNUTLS_VERSION_NUMBER <= 0x020b00
GCRY_THREAD_OPTION_PTHREAD_IMPL;
#endif
MODULE_TYPE_LIB
MODULE_TYPE_KEEP

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(glbl)
DEFobjCurrIf(net)
DEFobjCurrIf(datetime)
DEFobjCurrIf(nsd_ptcp)


static int bGlblSrvrInitDone = 0;	/**< 0 - server global init not yet done, 1 - already done */

static pthread_mutex_t mutGtlsStrerror;
/*< a mutex protecting the potentially non-reentrant gtlStrerror() function */

/* a macro to abort if GnuTLS error is not acceptable. We split this off from
 * CHKgnutls() to avoid some Coverity report in cases where we know GnuTLS
 * failed. Note: gnuRet must already be set accordingly!
 */
#define ABORTgnutls { \
		uchar *pErr = gtlsStrerror(gnuRet); \
		LogError(0, RS_RET_GNUTLS_ERR, "unexpected GnuTLS error %d in %s:%d: %s\n", \
	gnuRet, __FILE__, __LINE__, pErr); \
		free(pErr); \
		ABORT_FINALIZE(RS_RET_GNUTLS_ERR); \
}
/* a macro to check GnuTLS calls against unexpected errors */
#define CHKgnutls(x) { \
	gnuRet = (x); \
	if(gnuRet == GNUTLS_E_FILE_ERROR) { \
		LogError(0, RS_RET_GNUTLS_ERR, "error reading file - a common cause is that the " \
			"file  does not exist"); \
		ABORT_FINALIZE(RS_RET_GNUTLS_ERR); \
	} else if(gnuRet != 0) { \
		ABORTgnutls; \
	} \
}


/* ------------------------------ GnuTLS specifics ------------------------------ */
static gnutls_certificate_credentials_t xcred;

/* This defines a log function to be provided to GnuTLS. It hopefully
 * helps us track down hard to find problems.
 * rgerhards, 2008-06-20
 */
static void logFunction(int level, const char *msg)
{
	dbgprintf("GnuTLS log msg, level %d: %s\n", level, msg);
}



/* read in the whole content of a file. The caller is responsible for
 * freeing the buffer. To prevent DOS, this function can NOT read
 * files larger than 1MB (which still is *very* large).
 * rgerhards, 2008-05-26
 */
static rsRetVal
readFile(uchar *pszFile, gnutls_datum_t *pBuf)
{
	int fd;
	struct stat stat_st;
	DEFiRet;

	assert(pszFile != NULL);
	assert(pBuf != NULL);

	pBuf->data = NULL;

	if((fd = open((char*)pszFile, O_RDONLY)) == -1) {
		LogError(errno, RS_RET_FILE_NOT_FOUND, "can not read file '%s'", pszFile);
		ABORT_FINALIZE(RS_RET_FILE_NOT_FOUND);
	}

	if(fstat(fd, &stat_st) == -1) {
		LogError(errno, RS_RET_FILE_NO_STAT, "can not stat file '%s'", pszFile);
		ABORT_FINALIZE(RS_RET_FILE_NO_STAT);
	}

	/* 1MB limit */
	if(stat_st.st_size > 1024 * 1024) {
		LogError(0, RS_RET_FILE_TOO_LARGE, "file '%s' too large, max 1MB", pszFile);
		ABORT_FINALIZE(RS_RET_FILE_TOO_LARGE);
	}

	CHKmalloc(pBuf->data = MALLOC(stat_st.st_size));
	pBuf->size = stat_st.st_size;
	if(read(fd,  pBuf->data, stat_st.st_size) != stat_st.st_size) {
		LogError(0, RS_RET_IO_ERROR, "error or incomplete read of file '%s'", pszFile);
		ABORT_FINALIZE(RS_RET_IO_ERROR);
	}

finalize_it:
	if(fd != -1)
		close(fd);
	if(iRet != RS_RET_OK) {
		if(pBuf->data != NULL) {
			free(pBuf->data);
			pBuf->data = NULL;
			pBuf->size = 0;
			}
	}
	RETiRet;
}


/* Load the certificate and the private key into our own store. We need to do
 * this in the client case, to support fingerprint authentication. In that case,
 * we may be presented no matching root certificate, but we must provide ours.
 * The only way to do that is via the cert callback interface, but for it we
 * need to load certificates into our private store.
 * rgerhards, 2008-05-26
 */
static rsRetVal
gtlsLoadOurCertKey(nsd_gtls_t *pThis)
{
	DEFiRet;
	int gnuRet;
	gnutls_datum_t data = { NULL, 0 };
	uchar *keyFile;
	uchar *certFile;

	ISOBJ_TYPE_assert(pThis, nsd_gtls);

	certFile = glbl.GetDfltNetstrmDrvrCertFile();
	keyFile = glbl.GetDfltNetstrmDrvrKeyFile();

	if(certFile == NULL || keyFile == NULL) {
		/* in this case, we can not set our certificate. If we are
		 * a client and the server is running in "anon" auth mode, this
		 * may be well acceptable. In other cases, we will see some
		 * more error messages down the road. -- rgerhards, 2008-07-02
		 */
		dbgprintf("our certificate is not set, file name values are cert: '%s', key: '%s'\n",
			  certFile, keyFile);
		ABORT_FINALIZE(RS_RET_CERTLESS);
	}

	/* try load certificate */
	CHKiRet(readFile(certFile, &data));
	pThis->nOurCerts = sizeof(pThis->pOurCerts) / sizeof(gnutls_x509_crt_t);
	gnuRet = gnutls_x509_crt_list_import(pThis->pOurCerts, &pThis->nOurCerts,
		&data, GNUTLS_X509_FMT_PEM,  GNUTLS_X509_CRT_LIST_IMPORT_FAIL_IF_EXCEED);
	if(gnuRet < 0) {
		ABORTgnutls;
	}
	pThis->bOurCertIsInit = 1;
	free(data.data);
	data.data = NULL;

	/* try load private key */
	CHKiRet(readFile(keyFile, &data));
	CHKgnutls(gnutls_x509_privkey_init(&pThis->ourKey));
	pThis->bOurKeyIsInit = 1;
	CHKgnutls(gnutls_x509_privkey_import(pThis->ourKey, &data, GNUTLS_X509_FMT_PEM));
	free(data.data);

finalize_it:
	if(iRet != RS_RET_OK) {
		if(data.data != NULL)
			free(data.data);
		if(pThis->bOurCertIsInit) {
			for(unsigned i=0; i<pThis->nOurCerts; ++i) {
				gnutls_x509_crt_deinit(pThis->pOurCerts[i]);
			}
			pThis->bOurCertIsInit = 0;
		}
		if(pThis->bOurKeyIsInit) {
			gnutls_x509_privkey_deinit(pThis->ourKey);
			pThis->bOurKeyIsInit = 0;
		}
	}
	RETiRet;
}


/* This callback must be associated with a session by calling
 * gnutls_certificate_client_set_retrieve_function(session, cert_callback),
 * before a handshake. We will always return the configured certificate,
 * even if it does not match the peer's trusted CAs. This is necessary
 * to use self-signed certs in fingerprint mode. And, yes, this usage
 * of the callback is quite a hack. But it seems the only way to
 * obey to the IETF -transport-tls I-D.
 * Note: GnuTLS requires the function to return 0 on success and
 * -1 on failure.
 * rgerhards, 2008-05-27
 */
static int
gtlsClientCertCallback(gnutls_session_t session,
	__attribute__((unused)) const gnutls_datum_t* req_ca_rdn,
	int __attribute__((unused)) nreqs,
	__attribute__((unused)) const gnutls_pk_algorithm_t* sign_algos,
	int __attribute__((unused)) sign_algos_length,
#if HAVE_GNUTLS_CERTIFICATE_SET_RETRIEVE_FUNCTION
	gnutls_retr2_st* st
#else
	gnutls_retr_st *st
#endif
	)
{
	nsd_gtls_t *pThis;

	pThis = (nsd_gtls_t*) gnutls_session_get_ptr(session);

#if HAVE_GNUTLS_CERTIFICATE_SET_RETRIEVE_FUNCTION
	st->cert_type = GNUTLS_CRT_X509;
#else
	st->type = GNUTLS_CRT_X509;
#endif
	st->ncerts = pThis->nOurCerts;
	st->cert.x509 = pThis->pOurCerts;
	st->key.x509 = pThis->ourKey;
	st->deinit_all = 0;

	return 0;
}


/* This function extracts some information about this session's peer
 * certificate. Works for X.509 certificates only. Adds all
 * of the info to a cstr_t, which is handed over to the caller.
 * Caller must destruct it when no longer needed.
 * rgerhards, 2008-05-21
 */
static rsRetVal
gtlsGetCertInfo(nsd_gtls_t *const pThis, cstr_t **ppStr)
{
	uchar szBufA[1024];
	uchar *szBuf = szBufA;
	size_t szBufLen = sizeof(szBufA), tmp;
	unsigned int algo, bits;
	time_t expiration_time, activation_time;
	const gnutls_datum_t *cert_list;
	unsigned cert_list_size = 0;
	gnutls_x509_crt_t cert;
	cstr_t *pStr = NULL;
	int gnuRet;
	DEFiRet;
	unsigned iAltName;

	assert(ppStr != NULL);
	ISOBJ_TYPE_assert(pThis, nsd_gtls);

	if(gnutls_certificate_type_get(pThis->sess) != GNUTLS_CRT_X509)
		return RS_RET_TLS_CERT_ERR;

	cert_list = gnutls_certificate_get_peers(pThis->sess, &cert_list_size);
	CHKiRet(rsCStrConstructFromszStrf(&pStr, "peer provided %d certificate(s). ", cert_list_size));

	if(cert_list_size > 0) {
		/* we only print information about the first certificate */
		CHKgnutls(gnutls_x509_crt_init(&cert));
		CHKgnutls(gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER));

		expiration_time = gnutls_x509_crt_get_expiration_time(cert);
		activation_time = gnutls_x509_crt_get_activation_time(cert);
		ctime_r(&activation_time, (char*)szBuf);
		szBuf[ustrlen(szBuf) - 1] = '\0'; /* strip linefeed */
		CHKiRet(rsCStrAppendStrf(pStr, "Certificate 1 info: "
			"certificate valid from %s ", szBuf));
		ctime_r(&expiration_time, (char*)szBuf);
		szBuf[ustrlen(szBuf) - 1] = '\0'; /* strip linefeed */
		CHKiRet(rsCStrAppendStrf(pStr, "to %s; ", szBuf));

		/* Extract some of the public key algorithm's parameters */
		algo = gnutls_x509_crt_get_pk_algorithm(cert, &bits);
		CHKiRet(rsCStrAppendStrf(pStr, "Certificate public key: %s; ",
			gnutls_pk_algorithm_get_name(algo)));

		/* names */
		tmp = szBufLen;
		if(gnutls_x509_crt_get_dn(cert, (char*)szBuf, &tmp)
		    == GNUTLS_E_SHORT_MEMORY_BUFFER) {
			szBufLen = tmp;
			szBuf = malloc(tmp);
			gnutls_x509_crt_get_dn(cert, (char*)szBuf, &tmp);
		}
		CHKiRet(rsCStrAppendStrf(pStr, "DN: %s; ", szBuf));

		tmp = szBufLen;
		if(gnutls_x509_crt_get_issuer_dn(cert, (char*)szBuf, &tmp)
		    == GNUTLS_E_SHORT_MEMORY_BUFFER) {
			szBufLen = tmp;
			szBuf = realloc((szBuf == szBufA) ? NULL : szBuf, tmp);
			gnutls_x509_crt_get_issuer_dn(cert, (char*)szBuf, &tmp);
		}
		CHKiRet(rsCStrAppendStrf(pStr, "Issuer DN: %s; ", szBuf));

		/* dNSName alt name */
		iAltName = 0;
		while(1) { /* loop broken below */
			tmp = szBufLen;
			gnuRet = gnutls_x509_crt_get_subject_alt_name(cert, iAltName,
					szBuf, &tmp, NULL);
			if(gnuRet == GNUTLS_E_SHORT_MEMORY_BUFFER) {
				szBufLen = tmp;
				szBuf = realloc((szBuf == szBufA) ? NULL : szBuf, tmp);
				continue;
			} else if(gnuRet < 0)
				break;
			else if(gnuRet == GNUTLS_SAN_DNSNAME) {
				/* we found it! */
				CHKiRet(rsCStrAppendStrf(pStr, "SAN:DNSname: %s; ", szBuf));
				/* do NOT break, because there may be multiple dNSName's! */
			}
			++iAltName;
		}

		gnutls_x509_crt_deinit(cert);
	}

	cstrFinalize(pStr);
	*ppStr = pStr;

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pStr != NULL)
			rsCStrDestruct(&pStr);
	}
	if(szBuf != szBufA)
		free(szBuf);

	RETiRet;
}



#if 0 /* we may need this in the future - code needs to be looked at then! */
/* This function will print some details of the
 * given pThis->sess.
 */
static rsRetVal
print_info(nsd_gtls_t *pThis)
{
	const char *tmp;
	gnutls_credentials_type cred;
	gnutls_kx_algorithm kx;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_gtls);
	/* print the key exchange's algorithm name
	*/
	kx = gnutls_kx_get(pThis->sess);
	tmp = gnutls_kx_get_name(kx);
	dbgprintf("- Key Exchange: %s\n", tmp);

	/* Check the authentication type used and switch
	* to the appropriate.
	*/
	cred = gnutls_auth_get_type(pThis->sess);
	switch (cred) {
	case GNUTLS_CRD_ANON:       /* anonymous authentication */
		dbgprintf("- Anonymous DH using prime of %d bits\n",
		gnutls_dh_get_prime_bits(pThis->sess));
		break;
	case GNUTLS_CRD_CERTIFICATE:        /* certificate authentication */
		/* Check if we have been using ephemeral Diffie Hellman.
		*/
		if (kx == GNUTLS_KX_DHE_RSA || kx == GNUTLS_KX_DHE_DSS) {
		 dbgprintf("\n- Ephemeral DH using prime of %d bits\n",
			gnutls_dh_get_prime_bits(pThis->sess));
		}

		/* if the certificate list is available, then
		* print some information about it.
		*/
		gtlsPrintCert(pThis);
		break;
	case GNUTLS_CRD_SRP:        /* certificate authentication */
		dbgprintf("GNUTLS_CRD_SRP/IA");
		break;
	case GNUTLS_CRD_PSK:        /* certificate authentication */
		dbgprintf("GNUTLS_CRD_PSK");
		break;
	case GNUTLS_CRD_IA:        /* certificate authentication */
		dbgprintf("GNUTLS_CRD_IA");
		break;
	} /* switch */

	/* print the protocol's name (ie TLS 1.0) */
	tmp = gnutls_protocol_get_name(gnutls_protocol_get_version(pThis->sess));
	dbgprintf("- Protocol: %s\n", tmp);

	/* print the certificate type of the peer.
	* ie X.509
	*/
	tmp = gnutls_certificate_type_get_name(
	gnutls_certificate_type_get(pThis->sess));

	dbgprintf("- Certificate Type: %s\n", tmp);

	/* print the compression algorithm (if any)
	*/
	tmp = gnutls_compression_get_name( gnutls_compression_get(pThis->sess));
	dbgprintf("- Compression: %s\n", tmp);

	/* print the name of the cipher used.
	* ie 3DES.
	*/
	tmp = gnutls_cipher_get_name(gnutls_cipher_get(pThis->sess));
	dbgprintf("- Cipher: %s\n", tmp);

	/* Print the MAC algorithms name.
	* ie SHA1
	*/
	tmp = gnutls_mac_get_name(gnutls_mac_get(pThis->sess));
	dbgprintf("- MAC: %s\n", tmp);

	RETiRet;
}
#endif


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


/* a thread-safe variant of gnutls_strerror
 * The caller must free the returned string.
 * rgerhards, 2008-04-30
 */
uchar *gtlsStrerror(int error)
{
	uchar *pErr;

	pthread_mutex_lock(&mutGtlsStrerror);
	pErr = (uchar*) strdup(gnutls_strerror(error));
	pthread_mutex_unlock(&mutGtlsStrerror);

	return pErr;
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
gtlsRecordRecv(nsd_gtls_t *pThis)
{
	ssize_t lenRcvd;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_gtls);
	lenRcvd = gnutls_record_recv(pThis->sess, pThis->pszRcvBuf, NSD_GTLS_MAX_RCVBUF);
	if(lenRcvd >= 0) {
		pThis->lenRcvBuf = lenRcvd;
		pThis->ptrRcvBuf = 0;
	} else if(lenRcvd == GNUTLS_E_AGAIN || lenRcvd == GNUTLS_E_INTERRUPTED) {
		pThis->rtryCall = gtlsRtry_recv;
		dbgprintf("GnuTLS receive requires a retry (this most probably is OK and no error condition)\n");
		ABORT_FINALIZE(RS_RET_RETRY);
	} else {
		int gnuRet = lenRcvd;
		ABORTgnutls;
	}

finalize_it:
	dbgprintf("gtlsRecordRecv return. nsd %p, iRet %d, lenRcvd %d, lenRcvBuf %d, ptrRcvBuf %d\n",
	pThis, iRet, (int) lenRcvd, pThis->lenRcvBuf, pThis->ptrRcvBuf);
	RETiRet;
}


/* add our own certificate to the certificate set, so that the peer
 * can identify us. Please note that we try to use mutual authentication,
 * so we always add a cert, even if we are in the client role (later,
 * this may be controlled by a config setting).
 * rgerhards, 2008-05-15
 */
static rsRetVal
gtlsAddOurCert(void)
{
	int gnuRet = 0;
	uchar *keyFile;
	uchar *certFile;
	uchar *pGnuErr; /* for GnuTLS error reporting */
	DEFiRet;

	certFile = glbl.GetDfltNetstrmDrvrCertFile();
	keyFile = glbl.GetDfltNetstrmDrvrKeyFile();
	dbgprintf("GTLS certificate file: '%s'\n", certFile);
	dbgprintf("GTLS key file: '%s'\n", keyFile);
	if(certFile == NULL) {
		LogError(0, RS_RET_CERT_MISSING, "error: certificate file is not set, cannot "
				"continue");
		ABORT_FINALIZE(RS_RET_CERT_MISSING);
	}
	if(keyFile == NULL) {
		LogError(0, RS_RET_CERTKEY_MISSING, "error: key file is not set, cannot "
				"continue");
		ABORT_FINALIZE(RS_RET_CERTKEY_MISSING);
	}
	CHKgnutls(gnutls_certificate_set_x509_key_file(xcred, (char*)certFile, (char*)keyFile, GNUTLS_X509_FMT_PEM));

finalize_it:
	if(iRet != RS_RET_OK && iRet != RS_RET_CERT_MISSING && iRet != RS_RET_CERTKEY_MISSING) {
		pGnuErr = gtlsStrerror(gnuRet);
		errno = 0;
		LogError(0, iRet, "error adding our certificate. GnuTLS error %d, message: '%s', "
				"key: '%s', cert: '%s'", gnuRet, pGnuErr, keyFile, certFile);
		free(pGnuErr);
	}
	RETiRet;
}


/* globally initialize GnuTLS */
static rsRetVal
gtlsGlblInit(void)
{
	int gnuRet;
	uchar *cafile;
	DEFiRet;

	/* gcry_control must be called first, so that the thread system is correctly set up */
	#if GNUTLS_VERSION_NUMBER <= 0x020b00
	gcry_control (GCRYCTL_SET_THREAD_CBS, &gcry_threads_pthread);
	#endif
	CHKgnutls(gnutls_global_init());

	/* X509 stuff */
	CHKgnutls(gnutls_certificate_allocate_credentials(&xcred));

	/* sets the trusted cas file */
	cafile = glbl.GetDfltNetstrmDrvrCAF();
	if(cafile == NULL) {
		LogError(0, RS_RET_CA_CERT_MISSING, "error: ca certificate is not set, cannot "
				"continue");
		ABORT_FINALIZE(RS_RET_CA_CERT_MISSING);
	}
	dbgprintf("GTLS CA file: '%s'\n", cafile);
	gnuRet = gnutls_certificate_set_x509_trust_file(xcred, (char*)cafile, GNUTLS_X509_FMT_PEM);
	if(gnuRet == GNUTLS_E_FILE_ERROR) {
		LogError(0, RS_RET_GNUTLS_ERR,
			"error reading certificate file '%s' - a common cause is that the "
			"file  does not exist", cafile);
		ABORT_FINALIZE(RS_RET_GNUTLS_ERR);
	} else if(gnuRet < 0) {
		/* TODO; a more generic error-tracking function (this one based on CHKgnutls()) */
		uchar *pErr = gtlsStrerror(gnuRet);
		LogError(0, RS_RET_GNUTLS_ERR, "unexpected GnuTLS error %d in %s:%d: %s\n",
		gnuRet, __FILE__, __LINE__, pErr);
		free(pErr);
		ABORT_FINALIZE(RS_RET_GNUTLS_ERR);
	}

	if(GetGnuTLSLoglevel() > 0){
		gnutls_global_set_log_function(logFunction);
		gnutls_global_set_log_level(GetGnuTLSLoglevel());
		/* 0 (no) to 9 (most), 10 everything */
	}

finalize_it:
	RETiRet;
}

static rsRetVal
gtlsInitSession(nsd_gtls_t *pThis)
{
	DEFiRet;
	int gnuRet;
	gnutls_session_t session;

	gnutls_init(&session, GNUTLS_SERVER);
	pThis->bHaveSess = 1;
	pThis->bIsInitiator = 0;

	/* avoid calling all the priority functions, since the defaults are adequate. */

	CHKgnutls(gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, xcred));

	/* request client certificate if any.  */
	gnutls_certificate_server_set_request( session, GNUTLS_CERT_REQUEST);

	pThis->sess = session;

#	if HAVE_GNUTLS_CERTIFICATE_SET_RETRIEVE_FUNCTION
	/* store a pointer to ourselfs (needed by callback) */
	gnutls_session_set_ptr(pThis->sess, (void*)pThis);
	iRet = gtlsLoadOurCertKey(pThis); /* first load .pem files */
	if(iRet == RS_RET_OK) {
		gnutls_certificate_set_retrieve_function(xcred, gtlsClientCertCallback);
	} else if(iRet != RS_RET_CERTLESS) {
		FINALIZE; /* we have an error case! */
	}
#	endif

finalize_it:
	RETiRet;
}


/* set up all global things that are needed for server operations
 * rgerhards, 2008-04-30
 */
static rsRetVal
gtlsGlblInitLstn(void)
{
	DEFiRet;

	if(bGlblSrvrInitDone == 0) {
		/* we do not use CRLs right now, and I doubt we'll ever do. This functionality is
		 * considered legacy. -- rgerhards, 2008-05-05
		 */
		/*CHKgnutls(gnutls_certificate_set_x509_crl_file(xcred, CRLFILE, GNUTLS_X509_FMT_PEM));*/
		bGlblSrvrInitDone = 1; /* we are all set now */

		/* now we need to add our certificate */
		CHKiRet(gtlsAddOurCert());
	}

finalize_it:
	RETiRet;
}


/* Obtain the CN from the DN field and hand it back to the caller
 * (which is responsible for destructing it). We try to follow
 * RFC2253 as far as it makes sense for our use-case. This function
 * is considered a compromise providing good-enough correctness while
 * limiting code size and complexity. If a problem occurs, we may enhance
 * this function. A (pointer to a) certificate must be caller-provided.
 * If no CN is contained in the cert, no string is returned
 * (*ppstrCN remains NULL). *ppstrCN MUST be NULL on entry!
 * rgerhards, 2008-05-22
 */
static rsRetVal
gtlsGetCN(gnutls_x509_crt_t *pCert, cstr_t **ppstrCN)
{
	DEFiRet;
	int gnuRet;
	int i;
	int bFound;
	cstr_t *pstrCN = NULL;
	size_t size;
	/* big var the last, so we hope to have all we usually neeed within one mem cache line */
	uchar szDN[1024]; /* this should really be large enough for any non-malicious case... */

	assert(pCert != NULL);
	assert(ppstrCN != NULL);
	assert(*ppstrCN == NULL);

	size = sizeof(szDN);
	CHKgnutls(gnutls_x509_crt_get_dn(*pCert, (char*)szDN, &size));

	/* now search for the CN part */
	i = 0;
	bFound = 0;
	while(!bFound && szDN[i] != '\0') {
		/* note that we do not overrun our string due to boolean shortcut
		 * operations. If we have '\0', the if does not match and evaluation
		 * stops. Order of checks is obviously important!
		 */
		if(szDN[i] == 'C' && szDN[i+1] == 'N' && szDN[i+2] == '=') {
			bFound = 1;
			i += 2;
		}
		i++;

	}

	if(!bFound) {
		FINALIZE; /* we are done */
	}

	/* we found a common name, now extract it */
	CHKiRet(cstrConstruct(&pstrCN));
	while(szDN[i] != '\0' && szDN[i] != ',') {
		if(szDN[i] == '\\') {
			/* hex escapes are not implemented */
			++i; /* escape char processed */
			if(szDN[i] == '\0')
				ABORT_FINALIZE(RS_RET_CERT_INVALID_DN);
			CHKiRet(cstrAppendChar(pstrCN, szDN[i]));
		} else {
			CHKiRet(cstrAppendChar(pstrCN, szDN[i]));
		}
		++i; /* char processed */
	}
	cstrFinalize(pstrCN);

	/* we got it - we ignore the rest of the DN string (if any). So we may
	 * not detect if it contains more than one CN
	 */

	*ppstrCN = pstrCN;

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pstrCN != NULL)
			cstrDestruct(&pstrCN);
	}

	RETiRet;
}


/* Check the peer's ID in fingerprint auth mode.
 * rgerhards, 2008-05-22
 */
static rsRetVal
gtlsChkPeerFingerprint(nsd_gtls_t *pThis, gnutls_x509_crt_t *pCert)
{
	uchar fingerprint[20];
	size_t size;
	cstr_t *pstrFingerprint = NULL;
	int bFoundPositiveMatch;
	permittedPeers_t *pPeer;
	int gnuRet;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_gtls);

	/* obtain the SHA1 fingerprint */
	size = sizeof(fingerprint);
	CHKgnutls(gnutls_x509_crt_get_fingerprint(*pCert, GNUTLS_DIG_SHA1, fingerprint, &size));
	CHKiRet(GenFingerprintStr(fingerprint, size, &pstrFingerprint));
	dbgprintf("peer's certificate SHA1 fingerprint: %s\n", cstrGetSzStrNoNULL(pstrFingerprint));

	/* now search through the permitted peers to see if we can find a permitted one */
	bFoundPositiveMatch = 0;
	pPeer = pThis->pPermPeers;
	while(pPeer != NULL && !bFoundPositiveMatch) {
		if(!rsCStrSzStrCmp(pstrFingerprint, pPeer->pszID, strlen((char*) pPeer->pszID))) {
			bFoundPositiveMatch = 1;
		} else {
			pPeer = pPeer->pNext;
		}
	}

	if(!bFoundPositiveMatch) {
		dbgprintf("invalid peer fingerprint, not permitted to talk to it\n");
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
gtlsChkOnePeerName(nsd_gtls_t *pThis, uchar *pszPeerID, int *pbFoundPositiveMatch)
{
	permittedPeers_t *pPeer;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_gtls);
	assert(pszPeerID != NULL);
	assert(pbFoundPositiveMatch != NULL);

	if(pThis->pPermPeers) { /* do we have configured peer IDs? */
		pPeer = pThis->pPermPeers;
		while(pPeer != NULL) {
			CHKiRet(net.PermittedPeerWildcardMatch(pPeer, pszPeerID, pbFoundPositiveMatch));
			if(*pbFoundPositiveMatch)
				break;
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
	RETiRet;
}


/* Check the peer's ID in name auth mode.
 * rgerhards, 2008-05-22
 */
static rsRetVal
gtlsChkPeerName(nsd_gtls_t *pThis, gnutls_x509_crt_t *pCert)
{
	uchar lnBuf[256];
	char szAltName[1024]; /* this is sufficient for the DNSNAME... */
	int iAltName;
	size_t szAltNameLen;
	int bFoundPositiveMatch;
	cstr_t *pStr = NULL;
	cstr_t *pstrCN = NULL;
	int gnuRet;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_gtls);

	bFoundPositiveMatch = 0;
	CHKiRet(rsCStrConstruct(&pStr));

	/* first search through the dNSName subject alt names */
	iAltName = 0;
	while(!bFoundPositiveMatch) { /* loop broken below */
		szAltNameLen = sizeof(szAltName);
		gnuRet = gnutls_x509_crt_get_subject_alt_name(*pCert, iAltName,
				szAltName, &szAltNameLen, NULL);
		if(gnuRet < 0)
			break;
		else if(gnuRet == GNUTLS_SAN_DNSNAME) {
			dbgprintf("subject alt dnsName: '%s'\n", szAltName);
			snprintf((char*)lnBuf, sizeof(lnBuf), "DNSname: %s; ", szAltName);
			CHKiRet(rsCStrAppendStr(pStr, lnBuf));
			CHKiRet(gtlsChkOnePeerName(pThis, (uchar*)szAltName, &bFoundPositiveMatch));
			/* do NOT break, because there may be multiple dNSName's! */
		}
		++iAltName;
	}

	if(!bFoundPositiveMatch) {
		/* if we did not succeed so far, we try the CN part of the DN... */
		CHKiRet(gtlsGetCN(pCert, &pstrCN));
		if(pstrCN != NULL) { /* NULL if there was no CN present */
			dbgprintf("gtls now checking auth for CN '%s'\n", cstrGetSzStrNoNULL(pstrCN));
			snprintf((char*)lnBuf, sizeof(lnBuf), "CN: %s; ", cstrGetSzStrNoNULL(pstrCN));
			CHKiRet(rsCStrAppendStr(pStr, lnBuf));
			CHKiRet(gtlsChkOnePeerName(pThis, cstrGetSzStrNoNULL(pstrCN), &bFoundPositiveMatch));
		}
	}

	if(!bFoundPositiveMatch) {
		dbgprintf("invalid peer name, not permitted to talk to it\n");
		if(pThis->bReportAuthErr == 1) {
			cstrFinalize(pStr);
			errno = 0;
			LogError(0, RS_RET_INVALID_FINGERPRINT, "error: peer name not authorized -  "
					"not permitted to talk to it. Names: %s",
					cstrGetSzStrNoNULL(pStr));
			pThis->bReportAuthErr = 0;
		}
		ABORT_FINALIZE(RS_RET_INVALID_FINGERPRINT);
	}

finalize_it:
	if(pStr != NULL)
		rsCStrDestruct(&pStr);
	if(pstrCN != NULL)
		rsCStrDestruct(&pstrCN);
	RETiRet;
}


/* check the ID of the remote peer - used for both fingerprint and
 * name authentication. This is common code. Will call into specific
 * drivers once the certificate has been obtained.
 * rgerhards, 2008-05-08
 */
static rsRetVal
gtlsChkPeerID(nsd_gtls_t *pThis)
{
	const gnutls_datum_t *cert_list;
	unsigned int list_size = 0;
	gnutls_x509_crt_t cert;
	int bMustDeinitCert = 0;
	int gnuRet;
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_gtls);

	/* This function only works for X.509 certificates.  */
	if(gnutls_certificate_type_get(pThis->sess) != GNUTLS_CRT_X509)
		return RS_RET_TLS_CERT_ERR;

	cert_list = gnutls_certificate_get_peers(pThis->sess, &list_size);

	if(list_size < 1) {
		if(pThis->bReportAuthErr == 1) {
			errno = 0;
			LogError(0, RS_RET_TLS_NO_CERT, "error: peer did not provide a certificate, "
					"not permitted to talk to it");
			pThis->bReportAuthErr = 0;
		}
		ABORT_FINALIZE(RS_RET_TLS_NO_CERT);
	}

	/* If we reach this point, we have at least one valid certificate.
	 * We always use only the first certificate. As of GnuTLS documentation, the
	 * first certificate always contains the remote peer's own certificate. All other
	 * certificates are issuer's certificates (up the chain). We are only interested
	 * in the first certificate, which is our peer. -- rgerhards, 2008-05-08
	 */
	CHKgnutls(gnutls_x509_crt_init(&cert));
	bMustDeinitCert = 1; /* indicate cert is initialized and must be freed on exit */
	CHKgnutls(gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER));

	/* Now we see which actual authentication code we must call.  */
	if(pThis->authMode == GTLS_AUTH_CERTFINGERPRINT) {
		CHKiRet(gtlsChkPeerFingerprint(pThis, &cert));
	} else {
		assert(pThis->authMode == GTLS_AUTH_CERTNAME);
		CHKiRet(gtlsChkPeerName(pThis, &cert));
	}

finalize_it:
	if(bMustDeinitCert)
		gnutls_x509_crt_deinit(cert);

	RETiRet;
}


/* Verify the validity of the remote peer's certificate.
 * rgerhards, 2008-05-21
 */
static rsRetVal
gtlsChkPeerCertValidity(nsd_gtls_t *pThis)
{
	DEFiRet;
	const char *pszErrCause;
	int gnuRet;
	cstr_t *pStr = NULL;
	unsigned stateCert;
	const gnutls_datum_t *cert_list;
	unsigned cert_list_size = 0;
	gnutls_x509_crt_t cert;
	unsigned i;
	time_t ttCert;
	time_t ttNow;

	ISOBJ_TYPE_assert(pThis, nsd_gtls);

	/* check if we have at least one cert */
	cert_list = gnutls_certificate_get_peers(pThis->sess, &cert_list_size);
	if(cert_list_size < 1) {
		errno = 0;
		LogError(0, RS_RET_TLS_NO_CERT,
			"peer did not provide a certificate, not permitted to talk to it");
		ABORT_FINALIZE(RS_RET_TLS_NO_CERT);
	}

	CHKgnutls(gnutls_certificate_verify_peers2(pThis->sess, &stateCert));

	if(stateCert & GNUTLS_CERT_INVALID) {
		/* provide error details if we have them */
		if(stateCert & GNUTLS_CERT_SIGNER_NOT_FOUND) {
			pszErrCause = "signer not found";
		} else if(stateCert & GNUTLS_CERT_SIGNER_NOT_CA) {
			pszErrCause = "signer is not a CA";
		} else if(stateCert & GNUTLS_CERT_INSECURE_ALGORITHM) {
			pszErrCause = "insecure algorithm";
		} else if(stateCert & GNUTLS_CERT_REVOKED) {
			pszErrCause = "certificate revoked";
		} else {
			pszErrCause = "GnuTLS returned no specific reason";
			dbgprintf("GnuTLS returned no specific reason for GNUTLS_CERT_INVALID, certificate "
				 "status is %d\n", stateCert);
		}
		LogError(0, NO_ERRCODE, "not permitted to talk to peer, certificate invalid: %s",
				pszErrCause);
		gtlsGetCertInfo(pThis, &pStr);
		LogError(0, NO_ERRCODE, "invalid cert info: %s", cstrGetSzStrNoNULL(pStr));
		cstrDestruct(&pStr);
		ABORT_FINALIZE(RS_RET_CERT_INVALID);
	}

	/* get current time for certificate validation */
	if(datetime.GetTime(&ttNow) == -1)
		ABORT_FINALIZE(RS_RET_SYS_ERR);

	/* as it looks, we need to validate the expiration dates ourselves...
	 * We need to loop through all certificates as we need to make sure the
	 * interim certificates are also not expired.
	 */
	for(i = 0 ; i < cert_list_size ; ++i) {
		CHKgnutls(gnutls_x509_crt_init(&cert));
		CHKgnutls(gnutls_x509_crt_import(cert, &cert_list[i], GNUTLS_X509_FMT_DER));
		ttCert = gnutls_x509_crt_get_activation_time(cert);
		if(ttCert == -1)
			ABORT_FINALIZE(RS_RET_TLS_CERT_ERR);
		else if(ttCert > ttNow) {
			LogError(0, RS_RET_CERT_NOT_YET_ACTIVE, "not permitted to talk to peer: "
					"certificate %d not yet active", i);
			gtlsGetCertInfo(pThis, &pStr);
			LogError(0, RS_RET_CERT_NOT_YET_ACTIVE,
				"invalid cert info: %s", cstrGetSzStrNoNULL(pStr));
			cstrDestruct(&pStr);
			ABORT_FINALIZE(RS_RET_CERT_NOT_YET_ACTIVE);
		}

		ttCert = gnutls_x509_crt_get_expiration_time(cert);
		if(ttCert == -1)
			ABORT_FINALIZE(RS_RET_TLS_CERT_ERR);
		else if(ttCert < ttNow) {
			LogError(0, RS_RET_CERT_EXPIRED, "not permitted to talk to peer: certificate"
				" %d expired", i);
			gtlsGetCertInfo(pThis, &pStr);
			LogError(0, RS_RET_CERT_EXPIRED, "invalid cert info: %s", cstrGetSzStrNoNULL(pStr));
			cstrDestruct(&pStr);
			ABORT_FINALIZE(RS_RET_CERT_EXPIRED);
		}
		gnutls_x509_crt_deinit(cert);
	}

finalize_it:
	RETiRet;
}


/* check if it is OK to talk to the remote peer
 * rgerhards, 2008-05-21
 */
rsRetVal
gtlsChkPeerAuth(nsd_gtls_t *pThis)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_gtls);

	/* call the actual function based on current auth mode */
	switch(pThis->authMode) {
		case GTLS_AUTH_CERTNAME:
			/* if we check the name, we must ensure the cert is valid */
			CHKiRet(gtlsChkPeerCertValidity(pThis));
			CHKiRet(gtlsChkPeerID(pThis));
			break;
		case GTLS_AUTH_CERTFINGERPRINT:
			CHKiRet(gtlsChkPeerID(pThis));
			break;
		case GTLS_AUTH_CERTVALID:
			CHKiRet(gtlsChkPeerCertValidity(pThis));
			break;
		case GTLS_AUTH_CERTANON:
			FINALIZE;
			break;
	}

finalize_it:
	RETiRet;
}


/* globally de-initialize GnuTLS */
static rsRetVal
gtlsGlblExit(void)
{
	DEFiRet;
	/* X509 stuff */
	gnutls_certificate_free_credentials(xcred);
	gnutls_global_deinit(); /* we are done... */
	RETiRet;
}


/* end a GnuTLS session
 * The function checks if we have a session and ends it only if so. So it can
 * always be called, even if there currently is no session.
 */
static rsRetVal
gtlsEndSess(nsd_gtls_t *pThis)
{
	int gnuRet;
	DEFiRet;

	if(pThis->bHaveSess) {
		if(pThis->bIsInitiator) {
			gnuRet = gnutls_bye(pThis->sess, GNUTLS_SHUT_RDWR);
			while(gnuRet == GNUTLS_E_INTERRUPTED || gnuRet == GNUTLS_E_AGAIN) {
				gnuRet = gnutls_bye(pThis->sess, GNUTLS_SHUT_RDWR);
			}
		}
		gnutls_deinit(pThis->sess);
		pThis->bHaveSess = 0;
	}
	RETiRet;
}


/* a small wrapper for gnutls_transport_set_ptr(). The main intension for
 * creating this wrapper is to get the annoying "cast to pointer from different
 * size" compiler warning just once. There seems to be no way around it, see:
 * http://lists.gnu.org/archive/html/help-gnutls/2008-05/msg00000.html
 * rgerhards, 2008.05-07
 */
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
static inline void
gtlsSetTransportPtr(nsd_gtls_t *pThis, int sock)
{
	/* Note: the compiler warning for the next line is OK - see header comment! */
	gnutls_transport_set_ptr(pThis->sess, (gnutls_transport_ptr_t) sock);
}
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"

/* ---------------------------- end GnuTLS specifics ---------------------------- */


/* Standard-Constructor */
BEGINobjConstruct(nsd_gtls) /* be sure to specify the object type also in END macro! */
	iRet = nsd_ptcp.Construct(&pThis->pTcp);
	pThis->bReportAuthErr = 1;
ENDobjConstruct(nsd_gtls)


/* destructor for the nsd_gtls object */
PROTOTYPEobjDestruct(nsd_gtls);
BEGINobjDestruct(nsd_gtls) /* be sure to specify the object type also in END and CODESTART macros! */
CODESTARTobjDestruct(nsd_gtls)
	if(pThis->iMode == 1) {
		gtlsEndSess(pThis);
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

	if(pThis->bOurCertIsInit)
		for(unsigned i=0; i<pThis->nOurCerts; ++i) {
			gnutls_x509_crt_deinit(pThis->pOurCerts[i]);
		}
	if(pThis->bOurKeyIsInit)
		gnutls_x509_privkey_deinit(pThis->ourKey);
	if(pThis->bHaveSess)
		gnutls_deinit(pThis->sess);
ENDobjDestruct(nsd_gtls)


/* Set the driver mode. For us, this has the following meaning:
 * 0 - work in plain tcp mode, without tls (e.g. before a STARTTLS)
 * 1 - work in TLS mode
 * rgerhards, 2008-04-28
 */
static rsRetVal
SetMode(nsd_t *pNsd, int mode)
{
	DEFiRet;
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_gtls);
	if(mode != 0 && mode != 1) {
		LogError(0, RS_RET_INVALID_DRVR_MODE, "error: driver mode %d not supported by "
				"gtls netstream driver", mode);
		ABORT_FINALIZE(RS_RET_INVALID_DRVR_MODE);
	}

	pThis->iMode = mode;

finalize_it:
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
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_gtls);
	if(mode == NULL || !strcasecmp((char*)mode, "x509/name")) {
		pThis->authMode = GTLS_AUTH_CERTNAME;
	} else if(!strcasecmp((char*) mode, "x509/fingerprint")) {
		pThis->authMode = GTLS_AUTH_CERTFINGERPRINT;
	} else if(!strcasecmp((char*) mode, "x509/certvalid")) {
		pThis->authMode = GTLS_AUTH_CERTVALID;
	} else if(!strcasecmp((char*) mode, "anon")) {
		pThis->authMode = GTLS_AUTH_CERTANON;
	} else {
		LogError(0, RS_RET_VALUE_NOT_SUPPORTED, "error: authentication mode '%s' not supported by "
				"gtls netstream driver", mode);
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
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_gtls);
	if(pPermPeers == NULL)
		FINALIZE;

	if(pThis->authMode != GTLS_AUTH_CERTFINGERPRINT && pThis->authMode != GTLS_AUTH_CERTNAME) {
		LogError(0, RS_RET_VALUE_NOT_IN_THIS_MODE, "authentication not supported by "
			"gtls netstream driver in the configured authentication mode - ignored");
		ABORT_FINALIZE(RS_RET_VALUE_NOT_IN_THIS_MODE);
	}

	pThis->pPermPeers = pPermPeers;

finalize_it:
	RETiRet;
}

/* gnutls priority string
 * PascalWithopf 2017-08-16
 */
static rsRetVal
SetGnutlsPriorityString(nsd_t *pNsd, uchar *gnutlsPriorityString)
{
	DEFiRet;
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_gtls);
	pThis->gnutlsPriorityString = gnutlsPriorityString;
	RETiRet;
}


/* Provide access to the underlying OS socket. This is primarily
 * useful for other drivers (like nsd_gtls) who utilize ourselfs
 * for some of their functionality. -- rgerhards, 2008-04-18
 */
static rsRetVal
SetSock(nsd_t *pNsd, int sock)
{
	DEFiRet;
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_gtls);
	assert(sock >= 0);

	nsd_ptcp.SetSock(pThis->pTcp, sock);

	RETiRet;
}


/* Keep Alive Options
 */
static rsRetVal
SetKeepAliveIntvl(nsd_t *pNsd, int keepAliveIntvl)
{
	DEFiRet;
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_gtls);
	assert(keepAliveIntvl >= 0);

	nsd_ptcp.SetKeepAliveIntvl(pThis->pTcp, keepAliveIntvl);

	RETiRet;
}


/* Keep Alive Options
 */
static rsRetVal
SetKeepAliveProbes(nsd_t *pNsd, int keepAliveProbes)
{
	DEFiRet;
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_gtls);
	assert(keepAliveProbes >= 0);

	nsd_ptcp.SetKeepAliveProbes(pThis->pTcp, keepAliveProbes);

	RETiRet;
}


/* Keep Alive Options
 */
static rsRetVal
SetKeepAliveTime(nsd_t *pNsd, int keepAliveTime)
{
	DEFiRet;
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;

	ISOBJ_TYPE_assert((pThis), nsd_gtls);
	assert(keepAliveTime >= 0);

	nsd_ptcp.SetKeepAliveTime(pThis->pTcp, keepAliveTime);

	RETiRet;
}


/* abort a connection. This is meant to be called immediately
 * before the Destruct call. -- rgerhards, 2008-03-24
 */
static rsRetVal
Abort(nsd_t *pNsd)
{
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;
	DEFiRet;

	ISOBJ_TYPE_assert((pThis), nsd_gtls);

	if(pThis->iMode == 0) {
		nsd_ptcp.Abort(pThis->pTcp);
	}

	RETiRet;
}



/* initialize the tcp socket for a listner
 * Here, we use the ptcp driver - because there is nothing special
 * at this point with GnuTLS. Things become special once we accept
 * a session, but not during listener setup.
 * gerhards, 2008-04-25
 */
static rsRetVal
LstnInit(netstrms_t *pNS, void *pUsr, rsRetVal(*fAddLstn)(void*,netstrm_t*),
	 uchar *pLstnPort, uchar *pLstnIP, int iSessMax,
	 uchar *pszLstnPortFileName)
{
	DEFiRet;
	CHKiRet(gtlsGlblInitLstn());
	iRet = nsd_ptcp.LstnInit(pNS, pUsr, fAddLstn, pLstnPort, pLstnIP, iSessMax, pszLstnPortFileName);
finalize_it:
	RETiRet;
}


/* This function checks if the connection is still alive - well, kind of...
 * This is a dummy here. For details, check function common in ptcp driver.
 * rgerhards, 2008-06-09
 */
static rsRetVal
CheckConnection(nsd_t __attribute__((unused)) *pNsd)
{
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_gtls);

	return nsd_ptcp.CheckConnection(pThis->pTcp);
}


/* get the remote hostname. The returned hostname must be freed by the caller.
 * rgerhards, 2008-04-25
 */
static rsRetVal
GetRemoteHName(nsd_t *pNsd, uchar **ppszHName)
{
	DEFiRet;
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_gtls);
	iRet = nsd_ptcp.GetRemoteHName(pThis->pTcp, ppszHName);
	RETiRet;
}


/* Provide access to the sockaddr_storage of the remote peer. This
 * is needed by the legacy ACL system. --- gerhards, 2008-12-01
 */
static rsRetVal
GetRemAddr(nsd_t *pNsd, struct sockaddr_storage **ppAddr)
{
	DEFiRet;
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_gtls);
	iRet = nsd_ptcp.GetRemAddr(pThis->pTcp, ppAddr);
	RETiRet;
}


/* get the remote host's IP address. Caller must Destruct the object. */
static rsRetVal
GetRemoteIP(nsd_t *pNsd, prop_t **ip)
{
	DEFiRet;
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_gtls);
	iRet = nsd_ptcp.GetRemoteIP(pThis->pTcp, ip);
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
	int gnuRet;
	nsd_gtls_t *pNew = NULL;
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;
	const char *error_position;

	ISOBJ_TYPE_assert((pThis), nsd_gtls);
	CHKiRet(nsd_gtlsConstruct(&pNew)); // TODO: prevent construct/destruct!
	CHKiRet(nsd_ptcp.Destruct(&pNew->pTcp));
	CHKiRet(nsd_ptcp.AcceptConnReq(pThis->pTcp, &pNew->pTcp));

	if(pThis->iMode == 0) {
		/* we are in non-TLS mode, so we are done */
		*ppNew = (nsd_t*) pNew;
		FINALIZE;
	}

	/* if we reach this point, we are in TLS mode */
	CHKiRet(gtlsInitSession(pNew));
	gtlsSetTransportPtr(pNew, ((nsd_ptcp_t*) (pNew->pTcp))->sock);
	pNew->authMode = pThis->authMode;
	pNew->pPermPeers = pThis->pPermPeers;
	pNew->gnutlsPriorityString = pThis->gnutlsPriorityString;
	/* here is the priorityString set */
	if(pNew->gnutlsPriorityString != NULL) {
		if(gnutls_priority_set_direct(pNew->sess,
					(const char*) pNew->gnutlsPriorityString,
					&error_position)==GNUTLS_E_INVALID_REQUEST) {
			LogError(0, RS_RET_GNUTLS_ERR, "Syntax Error in"
					" Priority String: \"%s\"\n", error_position);
		}
	} else {
		/* Use default priorities */
		CHKgnutls(gnutls_set_default_priority(pNew->sess));
	}

	/* we now do the handshake. This is a bit complicated, because we are
	 * on non-blocking sockets. Usually, the handshake will not complete
	 * immediately, so that we need to retry it some time later.
	 */
	gnuRet = gnutls_handshake(pNew->sess);
	if(gnuRet == GNUTLS_E_AGAIN || gnuRet == GNUTLS_E_INTERRUPTED) {
		pNew->rtryCall = gtlsRtry_handshake;
		dbgprintf("GnuTLS handshake does not complete immediately - "
			"setting to retry (this is OK and normal)\n");
	} else if(gnuRet == 0) {
		/* we got a handshake, now check authorization */
		CHKiRet(gtlsChkPeerAuth(pNew));
	} else {
		uchar *pGnuErr = gtlsStrerror(gnuRet);
		LogError(0, RS_RET_TLS_HANDSHAKE_ERR,
			"gnutls returned error on handshake: %s\n", pGnuErr);
		free(pGnuErr);
		ABORT_FINALIZE(RS_RET_TLS_HANDSHAKE_ERR);
	}

	pNew->iMode = 1; /* this session is now in TLS mode! */

	*ppNew = (nsd_t*) pNew;

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pNew != NULL)
			nsd_gtlsDestruct(&pNew);
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
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_gtls);

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
		CHKmalloc(pThis->pszRcvBuf = MALLOC(NSD_GTLS_MAX_RCVBUF));
		pThis->lenRcvBuf = -1;
	}

	/* now check if we have something in our buffer. If so, we satisfy
	 * the request from buffer contents.
	 */
	if(pThis->lenRcvBuf == -1) { /* no data present, must read */
		CHKiRet(gtlsRecordRecv(pThis));
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
	if (iRet != RS_RET_OK &&
		iRet != RS_RET_RETRY) {
		/* We need to free the receive buffer in error error case unless a retry is wanted. , if we
		 * allocated one. -- rgerhards, 2008-12-03 -- moved here by alorbach, 2015-12-01
		 */
		*pLenBuf = 0;
		free(pThis->pszRcvBuf);
		pThis->pszRcvBuf = NULL;
	}
	dbgprintf("gtlsRcv return. nsd %p, iRet %d, lenRcvBuf %d, ptrRcvBuf %d\n", pThis,
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
	int iSent;
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;
	DEFiRet;
	ISOBJ_TYPE_assert(pThis, nsd_gtls);

	if(pThis->bAbortConn)
		ABORT_FINALIZE(RS_RET_CONNECTION_ABORTREQ);

	if(pThis->iMode == 0) {
		CHKiRet(nsd_ptcp.Send(pThis->pTcp, pBuf, pLenBuf));
		FINALIZE;
	}

	/* in TLS mode now */
	while(1) { /* loop broken inside */
		iSent = gnutls_record_send(pThis->sess, pBuf, *pLenBuf);
		if(iSent >= 0) {
			*pLenBuf = iSent;
			break;
		}
		if(iSent != GNUTLS_E_INTERRUPTED && iSent != GNUTLS_E_AGAIN) {
			uchar *pErr = gtlsStrerror(iSent);
			LogError(0, RS_RET_GNUTLS_ERR, "unexpected GnuTLS error %d - this "
				"could be caused by a broken connection. GnuTLS reports: %s \n",
				iSent, pErr);
			free(pErr);
			gnutls_perror(iSent);
			ABORT_FINALIZE(RS_RET_GNUTLS_ERR);
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
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;
	ISOBJ_TYPE_assert(pThis, nsd_gtls);
	return nsd_ptcp.EnableKeepAlive(pThis->pTcp);
}


/*
 * SNI should not be used if the hostname is a bare IP address
 */
static int
SetServerNameIfPresent(nsd_gtls_t *pThis, uchar *host) {
	struct sockaddr_in sa;
	struct sockaddr_in6 sa6;

	int inet_pton_ret = inet_pton(AF_INET, CHAR_CONVERT(host), &(sa.sin_addr));

	if (inet_pton_ret == 0) { // host wasn't a bare IPv4 address: try IPv6
		inet_pton_ret = inet_pton(AF_INET6, CHAR_CONVERT(host), &(sa6.sin6_addr));
	}

	switch(inet_pton_ret) {
		case 1: // host is a valid IP address: don't use SNI
			return 0;
		case 0: // host isn't a valid IP address: assume it's a domain name, use SNI
			return gnutls_server_name_set(pThis->sess, GNUTLS_NAME_DNS, host, ustrlen(host));
		default: // unexpected error
			return -1;
	}

}

/* open a connection to a remote host (server). With GnuTLS, we always
 * open a plain tcp socket and then, if in TLS mode, do a handshake on it.
 * rgerhards, 2008-03-19
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations" /* TODO: FIX Warnings! */
static rsRetVal
Connect(nsd_t *pNsd, int family, uchar *port, uchar *host, char *device)
{
	nsd_gtls_t *pThis = (nsd_gtls_t*) pNsd;
	int sock;
	int gnuRet;
	const char *error_position;
#	ifdef HAVE_GNUTLS_CERTIFICATE_TYPE_SET_PRIORITY
	static const int cert_type_priority[2] = { GNUTLS_CRT_X509, 0 };
#	endif
	DEFiRet;

	ISOBJ_TYPE_assert(pThis, nsd_gtls);
	assert(port != NULL);
	assert(host != NULL);

	CHKiRet(nsd_ptcp.Connect(pThis->pTcp, family, port, host, device));

	if(pThis->iMode == 0)
		FINALIZE;

	/* we reach this point if in TLS mode */
	CHKgnutls(gnutls_init(&pThis->sess, GNUTLS_CLIENT));
	pThis->bHaveSess = 1;
	pThis->bIsInitiator = 1;

	CHKgnutls(SetServerNameIfPresent(pThis, host));

	/* in the client case, we need to set a callback that ensures our certificate
	 * will be presented to the server even if it is not signed by one of the server's
	 * trusted roots. This is necessary to support fingerprint authentication.
	 */
	/* store a pointer to ourselfs (needed by callback) */
	gnutls_session_set_ptr(pThis->sess, (void*)pThis);
	iRet = gtlsLoadOurCertKey(pThis); /* first load .pem files */
	if(iRet == RS_RET_OK) {
#		if HAVE_GNUTLS_CERTIFICATE_SET_RETRIEVE_FUNCTION
		gnutls_certificate_set_retrieve_function(xcred, gtlsClientCertCallback);
#		else
		gnutls_certificate_client_set_retrieve_function(xcred, gtlsClientCertCallback);
#		endif
	} else if(iRet != RS_RET_CERTLESS) {
		FINALIZE; /* we have an error case! */
	}

	/*priority string setzen*/
	if(pThis->gnutlsPriorityString != NULL) {
		if(gnutls_priority_set_direct(pThis->sess,
					(const char*) pThis->gnutlsPriorityString,
					&error_position)==GNUTLS_E_INVALID_REQUEST) {
			LogError(0, RS_RET_GNUTLS_ERR, "Syntax Error in"
					" Priority String: \"%s\"\n", error_position);
		}
	} else {
		/* Use default priorities */
		CHKgnutls(gnutls_set_default_priority(pThis->sess));
	}

#	ifdef HAVE_GNUTLS_CERTIFICATE_TYPE_SET_PRIORITY
	/* The gnutls_certificate_type_set_priority function is deprecated
	 * and not available in recent GnuTLS versions. However, there is no
	 * doc how to properly replace it with gnutls_priority_set_direct.
	 * A lot of folks have simply removed it, when they also called
	 * gnutls_set_default_priority. This is what we now also do. If
	 * this causes problems or someone has an idea of how to replace
	 * the deprecated function in a better way, please let us know!
	 * In any case, we use it as long as it is available and let
	 * not insult us by the deprecation warnings.
	 * 2015-05-18 rgerhards
	 */
	CHKgnutls(gnutls_certificate_type_set_priority(pThis->sess, cert_type_priority));
#	endif

	/* put the x509 credentials to the current session */
	CHKgnutls(gnutls_credentials_set(pThis->sess, GNUTLS_CRD_CERTIFICATE, xcred));

	/* assign the socket to GnuTls */
	CHKiRet(nsd_ptcp.GetSock(pThis->pTcp, &sock));
	gtlsSetTransportPtr(pThis, sock);

	/* we need to store the hostname as an alternate mean of authentication if no
	 * permitted peer names are given. Using the hostname is quite useful. It permits
	 * auto-configuration of security if a commen root cert is present. -- rgerhards, 2008-05-26
	 */
	CHKmalloc(pThis->pszConnectHost = (uchar*)strdup((char*)host));

	/* and perform the handshake */
	CHKgnutls(gnutls_handshake(pThis->sess));
	dbgprintf("GnuTLS handshake succeeded\n");

	/* now check if the remote peer is permitted to talk to us - ideally, we
	 * should do this during the handshake, but GnuTLS does not yet provide
	 * the necessary callbacks -- rgerhards, 2008-05-26
	 */
	CHKiRet(gtlsChkPeerAuth(pThis));

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pThis->bHaveSess) {
			gnutls_deinit(pThis->sess);
			pThis->bHaveSess = 0;
		}
	}

	RETiRet;
}
#pragma GCC diagnostic pop


/* queryInterface function */
BEGINobjQueryInterface(nsd_gtls)
CODESTARTobjQueryInterface(nsd_gtls)
	if(pIf->ifVersion != nsdCURR_IF_VERSION) {/* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->Construct = (rsRetVal(*)(nsd_t**)) nsd_gtlsConstruct;
	pIf->Destruct = (rsRetVal(*)(nsd_t**)) nsd_gtlsDestruct;
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
	pIf->SetGnutlsPriorityString = SetGnutlsPriorityString;
finalize_it:
ENDobjQueryInterface(nsd_gtls)


/* exit our class
 */
BEGINObjClassExit(nsd_gtls, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(nsd_gtls)
	gtlsGlblExit();	/* shut down GnuTLS */

	/* release objects we no longer need */
	objRelease(nsd_ptcp, LM_NSD_PTCP_FILENAME);
	objRelease(net, LM_NET_FILENAME);
	objRelease(glbl, CORE_COMPONENT);
	objRelease(datetime, CORE_COMPONENT);
ENDObjClassExit(nsd_gtls)


/* Initialize the nsd_gtls class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-02-19
 */
BEGINObjClassInit(nsd_gtls, 1, OBJ_IS_LOADABLE_MODULE) /* class, version */
	/* request objects we use */
	CHKiRet(objUse(datetime, CORE_COMPONENT));
	CHKiRet(objUse(glbl, CORE_COMPONENT));
	CHKiRet(objUse(net, LM_NET_FILENAME));
	CHKiRet(objUse(nsd_ptcp, LM_NSD_PTCP_FILENAME));

	/* now do global TLS init stuff */
	CHKiRet(gtlsGlblInit());
ENDObjClassInit(nsd_gtls)


/* --------------- here now comes the plumbing that makes as a library module --------------- */


BEGINmodExit
CODESTARTmodExit
	nsdsel_gtlsClassExit();
	nsd_gtlsClassExit();
	pthread_mutex_destroy(&mutGtlsStrerror);
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_LIB_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */

	/* Initialize all classes that are in our module - this includes ourselfs */
	CHKiRet(nsd_gtlsClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */
	CHKiRet(nsdsel_gtlsClassInit(pModInfo)); /* must be done after tcps_sess, as we use it */

	pthread_mutex_init(&mutGtlsStrerror, NULL);
ENDmodInit
/* vi:set ai:
 */
