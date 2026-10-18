/*
 *
 * Copyright (C) 2019-2024, Broadband Forum
 * Copyright (C) 2017-2024  CommScope, Inc
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file device_security.c
 *
 * Implements the Device.LocalAgent.Certificate and the Device.Security data model object
 *
 */
#include "vendor_defs.h"

#ifndef REMOVE_DEVICE_SECURITY
#include <time.h>
#include <string.h>

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/safestack.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "common_defs.h"
#include "stomp.h"
#include "data_model.h"
#include "usp_api.h"
#include "device.h"
#include "dm_access.h"
#include "vendor_api.h"
#include "iso8601.h"
#include "text_utils.h"


//-----------------------------------------------------------------------------------------
// Typedef for hash of certificate
typedef unsigned cert_hash_t;

//------------------------------------------------------------------------------
// String, set by '-a' command line option to specify a file containing the client cert and private key to use when authenticating this device
char *auth_cert_file = NULL;

//------------------------------------------------------------------------------
// String, set by '-t' command line option to specify a file containing trust store certificates
char *usp_trust_store_file = NULL;

//------------------------------------------------------------------------------
// Location of the Device.Security.Certificate table within the data model
#define DEVICE_CERT_ROOT "Device.Security.Certificate"
static const char device_cert_root[] = DEVICE_CERT_ROOT;

//------------------------------------------------------------------------------
// Location of the Device.LocalAgent.Certificate table within the data model
#define DEVICE_LA_CERT_ROOT "Device.LocalAgent.Certificate"
static const char device_la_cert_root[] = DEVICE_LA_CERT_ROOT;

//------------------------------------------------------------------------------
// Client certificate and associated private key obtained either from the get_agent_cert vendor hook, or from a file (specified by the '--authcert' option)
// NOTE: If the client cert is obtained via the load_agent_cert vendor hook, then the client cert will not be cached here.
static X509 *agent_cert = NULL;
static EVP_PKEY *agent_pkey = NULL;

//------------------------------------------------------------------------------
// Structure containing parsed information about the agent certificate
typedef struct
{
    bool is_loaded;
    bool is_san_equal_endpoint_id;
    char *serial_number;
    char *issuer;
} client_cert_t;

static client_cert_t client_cert;

//------------------------------------------------------------------------------
// Enumeration defining what a certificate is used for
typedef enum
{
    kCertUsage_TrustCert,
    kCertUsage_ClientCert,
    kCertUsage_SystemCert,
} cert_usage_t;

//------------------------------------------------------------------------------
// Table used to convert from an enumeration of certificate usage to a textual representation
const enum_entry_t cert_usages[] =
{
    { kCertUsage_TrustCert,     "TrustCerts" },
    { kCertUsage_ClientCert,    "ClientCert" },
    { kCertUsage_SystemCert,    "SystemCerts" },
};

//------------------------------------------------------------------------------
// Vector holding information about each certificate in Device.Security.Certificate
// Some certificates in this array are also present in Device.LocalAgent.Certificate
typedef struct
{
    int instance;               // Instance number of this certificate in Device.Security.Certificate
    int la_instance;            // Instance number of this certificate in Device.LocalAgent.Certificate (if applicable)

    cert_usage_t cert_usage;    // Defines what the cert is used for
    X509 *cert;                 // Points to a copy of the certificate (only if this is a trust store cert, NULL otherwise)
                                // This is used to seed curl's trust store in DEVICE_SECURITY_SetCurlTrustStore()

    char *subject;              // Free with OPENSSL_free()
    char *issuer;               // Free with OPENSSL_free()
    char *serial_number;        // Free with OPENSSL_free()
    char not_before[MAX_ISO8601_LEN];
    char not_after[MAX_ISO8601_LEN];
    time_t last_modif;
    char *subject_alt;          // Free with USP_FREE()
    char *signature_algorithm;  // Free with USP_FREE()
    cert_hash_t hash;           // Hash of the DER (binary) form of the certificate
} cert_t;

static cert_t *all_certs = NULL;
static int num_all_certs = 0;
static int num_trust_certs = 0;

//------------------------------------------------------------------------------------
// Enumeration for FingerprintAlgorithm input argument of GetFingerprint() command
typedef enum
{
    kFpAlg_None,
    kFpAlg_SHA1,
    kFpAlg_SHA224,
    kFpAlg_SHA256,
    kFpAlg_SHA384,
    kFpAlg_SHA512
} fp_alg_t;

//------------------------------------------------------------------------------
// Table used to convert from an enumeration of an MTP status to a textual representation
const enum_entry_t fp_algs[] =
{
    { kFpAlg_SHA1,      "SHA-1"   },
    { kFpAlg_SHA224,    "SHA-224" },
    { kFpAlg_SHA256,    "SHA-256" },
    { kFpAlg_SHA384,    "SHA-384" },
    { kFpAlg_SHA512,    "SHA-512" },
};

//------------------------------------------------------------------------------------
// Array of valid input arguments for GetFingerprint()
static char *fp_input_args[] =
{
    "FingerprintAlgorithm"
};

//------------------------------------------------------------------------------------
// Array of valid output arguments for GetFingerprint()
static char *fp_output_args[] =
{
    "Fingerprint"
};

//------------------------------------------------------------------------------
// Forward declarations. Note these are not static, because we need them in the symbol table for USP_LOG_Callstack() to show them
void LoadCerts_FromPath(char *path, cert_usage_t cert_usage, int role_instance);
void LoadCerts_FromFile(char *file_path, cert_usage_t cert_usage, int role_instance);
void LoadCerts_FromDir(char *dir_path, cert_usage_t cert_usage, int role_instance);
int Get_NumCerts(dm_req_t *req, char *buf, int len);
int Get_NumTrustCerts(dm_req_t *req, char *buf, int len);
int GetCert_LastModif(dm_req_t *req, char *buf, int len);
int GetCert_SerialNumber(dm_req_t *req, char *buf, int len);
int GetCert_Issuer(dm_req_t *req, char *buf, int len);
int GetCert_NotBefore(dm_req_t *req, char *buf, int len);
int GetCert_NotAfter(dm_req_t *req, char *buf, int len);
int GetCert_Subject(dm_req_t *req, char *buf, int len);
int GetCert_SubjectAlt(dm_req_t *req, char *buf, int len);
int GetCert_SignatureAlgorithm(dm_req_t *req, char *buf, int len);
int GetLaCert_SerialNumber(dm_req_t *req, char *buf, int len);
int GetLaCert_Issuer(dm_req_t *req, char *buf, int len);
cert_t *Find_SecurityCertByReq(dm_req_t *req);
cert_t *Find_LocalAgentCertByReq(dm_req_t *req);
cert_t *Find_LocalAgentCertByHash(cert_hash_t hash);
cert_t *Find_CertByHash(cert_hash_t hash);
int LoadTrustStore(void);
int LoadClientCert(SSL_CTX *ctx);
int GetClientCert(X509 **p_cert, EVP_PKEY **p_pkey);
int GetCertFromFile(char *cert_file, X509 **p_cert, EVP_PKEY **p_pkey);
int GetClientCertFromMemory(X509 **p_cert, EVP_PKEY **p_pkey);
int AddClientCert(SSL_CTX *ctx);
int AddCert(X509 *cert, cert_usage_t cert_usage, int role_instance);
void DestroyCert(cert_t *ct);
X509 *Cert_FromDER(const unsigned char *cert_data, int cert_len);
int ParseCert_Subject(X509 *cert, char **p_subject);
int ParseCert_Issuer(X509 *cert, char **p_issuer);
int ParseCert_LastModif(X509 *cert, time_t *last_modif);
int ParseCert_SerialNumber(X509 *cert, char **p_serial_number);
int ParseCert_NotBefore(X509 *cert, char *buf, int len);
int ParseCert_NotAfter(X509 *cert, char *buf, int len);
int Asn1Time_To_ISO8601(ASN1_TIME *cert_time, char *buf, int buflen);
int ParseCert_SubjectAlt(X509 *cert, char **p_subject_alt);
int ParseCert_SignatureAlg(X509 *cert, char **p_sig_alg);
int CalcCertHash(X509 *cert, cert_hash_t *p_hash);
bool IsSystemTimeReliable(void);
void LogCertChain(STACK_OF(X509) *cert_chain);
void LogTrustCerts(void);
void LogCert_DER(X509 *cert);
int Operate_GetFingerprint(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args);

/*********************************************************************//**
**
** DEVICE_SECURITY_Init
**
** Initialises this component, and registers all parameters which it implements
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_SECURITY_Init(void)
{
    int err = USP_ERR_OK;

    // Register Device.Security.Certificate parameters
    err |= USP_REGISTER_VendorParam_ReadOnly("Device.Security.CertificateNumberOfEntries", Get_NumCerts, DM_UINT);
    err |= USP_REGISTER_Object(DEVICE_CERT_ROOT ".{i}", USP_HOOK_DenyAddInstance, NULL, NULL,   // This table is read only
                                                        USP_HOOK_DenyDeleteInstance, NULL, NULL);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CERT_ROOT ".{i}.LastModif", GetCert_LastModif, DM_DATETIME);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CERT_ROOT ".{i}.SerialNumber", GetCert_SerialNumber, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CERT_ROOT ".{i}.Issuer", GetCert_Issuer, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CERT_ROOT ".{i}.NotBefore", GetCert_NotBefore, DM_DATETIME);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CERT_ROOT ".{i}.NotAfter", GetCert_NotAfter, DM_DATETIME);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CERT_ROOT ".{i}.Subject", GetCert_Subject, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CERT_ROOT ".{i}.SubjectAlt", GetCert_SubjectAlt, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_CERT_ROOT ".{i}.SignatureAlgorithm", GetCert_SignatureAlgorithm, DM_STRING);

    // Register Device.LocalAgent.Certificate parameters
    err |= USP_REGISTER_Object(DEVICE_LA_CERT_ROOT ".{i}", USP_HOOK_DenyAddInstance, NULL, NULL,   // This table is read only
                                                        USP_HOOK_DenyDeleteInstance, NULL, NULL);
    err |= USP_REGISTER_VendorParam_ReadOnly("Device.LocalAgent.CertificateNumberOfEntries", Get_NumTrustCerts, DM_UINT);

    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_LA_CERT_ROOT ".{i}.Alias", DM_ACCESS_PopulateAliasParam, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_LA_CERT_ROOT ".{i}.SerialNumber", GetLaCert_SerialNumber, DM_STRING);
    err |= USP_REGISTER_VendorParam_ReadOnly(DEVICE_LA_CERT_ROOT ".{i}.Issuer", GetLaCert_Issuer, DM_STRING);

    // Register Device.LocalAgent.Certificate.{i}.GetFingerprint operation
    err |= USP_REGISTER_SyncOperation(DEVICE_LA_CERT_ROOT ".{i}.GetFingerprint()", Operate_GetFingerprint);
    err |= USP_REGISTER_OperationArguments(DEVICE_LA_CERT_ROOT ".{i}.GetFingerprint()", fp_input_args, NUM_ELEM(fp_input_args),
                                                                                        fp_output_args, NUM_ELEM(fp_output_args));
    err |= USP_REGISTER_Param_SupportedList("Device.LocalAgent.SupportedFingerprintAlgorithms", fp_algs, NUM_ELEM(fp_algs));

    // Register unique keys for tables
    char *unique_keys[] = { "SerialNumber", "Issuer" };
    char *alias_unique_key[] = { "Alias" };
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_CERT_ROOT ".{i}", unique_keys, NUM_ELEM(unique_keys));
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_LA_CERT_ROOT ".{i}", unique_keys, NUM_ELEM(unique_keys));
    err |= USP_REGISTER_Object_UniqueKey(DEVICE_LA_CERT_ROOT ".{i}", alias_unique_key, NUM_ELEM(alias_unique_key));

    // Exit if any errors occurred
    if (err != USP_ERR_OK)
    {
        return USP_ERR_INTERNAL_ERROR;
    }

    // Initialise SSL
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
#else
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS, NULL);
#endif

    // Initialise client certificate structure
    memset(&client_cert, 0, sizeof(client_cert));
    client_cert.is_san_equal_endpoint_id = false;
    client_cert.is_loaded = false;

    // If the code gets here, then registration was successful
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_SECURITY_Start
**
** Loads all trust store and client certificates into memory
** NOTE: This function must not create any permanent SSL contests, as
**       libwebsockets re-initialises LibSSL, after this function is called
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_SECURITY_Start(void)
{
    int err;
    SSL_CTX *temp_ssl_ctx = NULL;   // Temporary SSL context: required because the load_agent_cert vendor hook only loads into an SSL context
    load_agent_cert_cb_t load_agent_cert_cb;

    // Exit if failed to load certificate trust store provided by vendor hook
    err = LoadTrustStore();
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // Add trust store certificates specified by '-t' option
    // NOTE: The string compare test is present in order to allow an invocation of USP Agent to specify no trust store file using -t "null". Useful, if the -t option is always used in all invocations.
    if ((usp_trust_store_file != NULL) && (strcmp(usp_trust_store_file, "null") != 0))
    {
        LoadCerts_FromPath(usp_trust_store_file, kCertUsage_TrustCert, ROLE_TRUST_STORE_DEFAULT);
    }

    // Exit if unable to create a temporary SSL context.
    // This is necessary because the load_agent_cert vendor hook only loads into an SSL context
    temp_ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    if (temp_ssl_ctx == NULL)
    {
        USP_ERR_SetMessage("%s: SSL_CTX_new failed", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Determine which function to call to load the client cert
    load_agent_cert_cb = vendor_hook_callbacks.load_agent_cert_cb;
    if (load_agent_cert_cb == NULL)
    {
        load_agent_cert_cb = LoadClientCert;  // Fallback to a function which calls the get_agent_cert vendor hook
    }
    else
    {
        USP_LOG_Info("%s: Obtaining a device certificate from load_agent_cert vendor hook", __FUNCTION__);
    }

    // Exit if failed whilst loading a client certificate for this Agent
    err = load_agent_cert_cb(temp_ssl_ctx);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: load_agent_cert_cb() failed", __FUNCTION__);
        goto exit;
    }

    // Exit if unable to add the client cert into the data model (if one has been loaded)
    err = AddClientCert(temp_ssl_ctx);
    if (err != USP_ERR_OK)
    {
        goto exit;
    }

    // The trust store and client cert has been successfully cached in this module
    err = USP_ERR_OK;

exit:
    // Free the temporary SSL Context
    // NOTE: This does not free the X509 certificate pointed to by the agent_cert, as the
    // X509 object contains a reference counter, and it still has a count for the instance created in LoadClientCert()
    if (temp_ssl_ctx != NULL)
    {
        SSL_CTX_free(temp_ssl_ctx);
    }

    // Load the certificates contained in the system cert directory
    LoadCerts_FromPath(SYSTEM_CERT_PATH, kCertUsage_SystemCert, INVALID);

    return err;
}

/*********************************************************************//**
**
** DEVICE_SECURITY_Stop
**
** Frees all memory used by this component
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
void DEVICE_SECURITY_Stop(void)
{
    int i;
    cert_t *ct;

    // Iterate over all certs, freeing all memory
    for (i=0; i<num_all_certs; i++)
    {
        ct = &all_certs[i];
        DestroyCert(ct);
    }
    USP_SAFE_FREE(all_certs);

    // Free the client certificate
    if (agent_cert != NULL)
    {
        X509_free(agent_cert);
    }

    if (agent_pkey != NULL)
    {
        EVP_PKEY_free(agent_pkey);
    }

    if (client_cert.is_loaded)
    {
        OPENSSL_free(client_cert.serial_number);
        OPENSSL_free(client_cert.issuer);
    }

    // No explicit cleanup of OpenSSL is required.
    // Cleanup routines are now NoOps which have been deprecated. See OpenSSL Changes between 1.0.2h and 1.1.0  [25 Aug 2016]
}

/*********************************************************************//**
**
** DEVICE_SECURITY_GetControllerTrust
**
** Obtains the controller trust role to use for controllers attached to this connection
** NOTE: This function is called from the MTP thread, so it should only log errors (not call USP_ERR_SetMessage)
** NOTE: The DM thread owned variables accessed by this function are seeded at startup and are immutable afterwards,
**       therefore this function may safely be called from the MTP thread even though it accesses variables
**       which are owned by the DM thread
**
** \param   cert_chain - pointer to verified certificate chain for this connection
** \param   role_instance - pointer to variable in which to return instance in Device.LocalAgent.ControllerTrust.Role.{i} permitted by CA cert
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
#define STACK_OF_X509  STACK_OF(X509)  // Define so that ctags works for this function
int DEVICE_SECURITY_GetControllerTrust(STACK_OF_X509 *cert_chain, int *role_instance)
{
    int err;
    unsigned num_certs;
    X509 *ca_cert;
    X509 *broker_cert;
    cert_hash_t hash;
    cert_t *ct;

    // The cert at position[0] will be the STOMP broker cert
    // The cert at position[1] will be the CA cert that validates the broker cert
    // The certs at higher positions are higher level CA certs, all the way up to one in our trust store

    // Exit if the certificate chain does not contain at least 2 certificates
    num_certs = sk_X509_num(cert_chain);
    if (num_certs < 2)
    {
        USP_LOG_Error("%s: Expected 2 or more certificates in the certificate chain", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to get broker cert
    broker_cert = (X509*) sk_X509_value(cert_chain, 0);
    if (broker_cert == NULL)
    {
        USP_LOG_Error("%s: Unable to get broker cert with sk_X509_value()", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to get trust store cert
    ca_cert = (X509*) sk_X509_value(cert_chain, num_certs-1);
    if (ca_cert == NULL)
    {
        USP_LOG_Error("%s: Unable to get trust store cert with sk_X509_value()", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to calculate the hash of the trust store cert
    err = CalcCertHash(ca_cert, &hash);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if unable to find the entry in Device.LocalAgent.Certificate.{i} that matches the trust store cert in our SSL chain of trust
    // NOTE: This should never fail, as we load the trust certs that Open SSL uses
    ct = Find_LocalAgentCertByHash(hash);
    if (ct == NULL)
    {
        USP_LOG_Error("%s: CA cert in chain of trust, not found in Device.LocalAgent.Certificate", __FUNCTION__);
        LogCertChain(cert_chain);
        LogTrustCerts();
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to get a role associated with the certificate
    *role_instance = DEVICE_CTRUST_GetCertInheritedRole(ct->la_instance);
    if (*role_instance == INVALID)
    {
        USP_LOG_Error("%s: CA cert in chain of trust (Device.LocalAgent.Certificate.%d) did not have an associated role in Device.LocalAgent.ControllerTrust.Credential.{i}", __FUNCTION__, ct->la_instance);
        LogCertChain(cert_chain);
        LogTrustCerts();
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  DEVICE_SECURITY_CreateSSLContext
**
**  Function to create an SSL context capable of performing the given method
**  The SSL context will be loaded with the trust store certs, client cert and verify callback
**
** \param   method - Type of SSL to use (eg TLS or DTLS)
** \param   verify_mode - whether SSL should verify the peer (SSL_VERIFY_PEER), and whether to only perform verification once (SSL_VERIFY_CLIENT_ONCE - for DTLS servers)
** \param   verify_callback - Function to call when verifying certificates from the server
**
** \return  pointer to created SSL context, or NULL if an error occurred
**
**************************************************************************/
SSL_CTX *DEVICE_SECURITY_CreateSSLContext(const SSL_METHOD *method, int verify_mode, ssl_verify_callback_t verify_callback)
{
    int err;
    SSL_CTX *ssl_ctx;

    // Exit if unable to create an SSL context
    ssl_ctx = SSL_CTX_new(method);
    if (ssl_ctx == NULL)
    {
        USP_ERR_SetMessage("%s: SSL_CTX_new failed", __FUNCTION__);
        goto exit;
    }

    // Explicitly disallow SSLv2, as it is insecure. See https://arxiv.org/pdf/1407.2168.pdf
    // NOTE: Even without this, SSLv2 ciphers don't seem to appear in the cipher list. Just added in case someone is using an older version of OpenSSL.
    SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv2);
    // SSL_CTX_set_options(ssl_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION);

    // Exit if unable to load our trust store and client cert into the SSL context's trust store
    err = DEVICE_SECURITY_LoadTrustStore(ssl_ctx, verify_mode, verify_callback);
    if (err != USP_ERR_OK)
    {
        SSL_CTX_free(ssl_ctx);
        ssl_ctx = NULL;
        goto exit;
    }

exit:
    return ssl_ctx;
}

/*********************************************************************//**
**
**  DEVICE_SECURITY_LoadTrustStore
**
**  Loads the trust store certificates and client cert into the specified SSL context
**
** \param   ssl_ctx - pointer to SSL context to add trust store certs to
** \param   verify_mode - whether SSL should verify the peer (SSL_VERIFY_PEER), and whether to only perform verification once (SSL_VERIFY_CLIENT_ONCE - for DTLS servers)
** \param   verify_callback - Function to call when verifying certificates from the server or NULL if none should be set
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_SECURITY_LoadTrustStore(SSL_CTX *ssl_ctx, int verify_mode, ssl_verify_callback_t verify_callback)
{
    X509_STORE *trust_store;
    load_agent_cert_cb_t load_agent_cert_cb;
    cert_t *ct;
    int i;
    int err;

    // Exit if unable to obtain the SSL context's trust store object
    trust_store = SSL_CTX_get_cert_store(ssl_ctx);
    if (trust_store == NULL)
    {
        USP_LOG_Error("%s: SSL_CTX_get_cert_store() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Add all certificates in our trust store to the SSL context's trust store
    // NOTE: X509 objects have a reference counter, calling X509_STORE_add_cert() just increases the reference count
    for (i=0; i<num_all_certs; i++)
    {
        ct = &all_certs[i];
        if (ct->cert_usage == kCertUsage_TrustCert)
        {
            USP_ASSERT(ct->cert != NULL);
            err = X509_STORE_add_cert(trust_store, ct->cert);
            if (err == 0)
            {
                USP_LOG_Error("%s: X509_STORE_add_cert() failed", __FUNCTION__);
                return USP_ERR_INTERNAL_ERROR;
            }
        }
    }

    // Set the verify callback to use for each certificate
    if ((verify_callback != NULL) && (verify_mode != 0))
    {
        SSL_CTX_set_verify(ssl_ctx, verify_mode, verify_callback);
    }

    // Load the client cert using the load_agent_cert vendor hook (if registered)
    load_agent_cert_cb = vendor_hook_callbacks.load_agent_cert_cb;
    if (load_agent_cert_cb != NULL)
    {
        err = load_agent_cert_cb(ssl_ctx);
        if (err != USP_ERR_OK)
        {
            USP_LOG_Error("%s: load_agent_cert_cb() failed", __FUNCTION__);
            return USP_ERR_INTERNAL_ERROR;
        }
        goto exit;
    }

    // Otherwise load the cached client cert
    if ((agent_cert != NULL) && (agent_pkey != NULL))
    {
        // Exit if unable to add this agent's certificate
        // NOTE: X509 objects have a reference counter, calling SSL_CTX_use_certificate() just increases the reference count
        err = SSL_CTX_use_certificate(ssl_ctx, agent_cert);
        if (err != 1)
        {
            USP_LOG_Error("%s: SSL_CTX_use_certificate() failed", __FUNCTION__);
            return USP_ERR_INTERNAL_ERROR;
        }

        // Exit if unable to add the private key
        // NOTE: Private key objects have a reference counter, calling SSL_CTX_use_PrivateKey() just increases the reference count
        err = SSL_CTX_use_PrivateKey(ssl_ctx, agent_pkey);
        if (err != 1)
        {
            USP_LOG_Error("%s: SSL_CTX_use_PrivateKey() failed", __FUNCTION__);
            return USP_ERR_INTERNAL_ERROR;
        }
    }

exit:
    return USP_ERR_OK;
}

/*********************************************************************//**
**
**  DEVICE_SECURITY_CanClientCertAuthenticate
**
**  Called from STOMP MTP to determine whether a client certificate is loaded
**  and contains authentication information (SAN=EndpointID)
**  This function is only called by STOMP MTP in order to log the correct debug message
**  This function is threadsafe as the variables accessed are immutable since startup
**
** \param   available - pointer to variable in which to return whether a client certificate has been loaded
** \param   matches_endpoint - pointer to variable in which to return whether the SubjectAltName field in the client cert matches the EndpointID of the device
**                             i.e. whether the client cert is suitable for authentication purposes
**
** \return  true if a client cert has been loaded and SAN=EndpointID
**
**************************************************************************/
void DEVICE_SECURITY_GetClientCertStatus(bool *available, bool *matches_endpoint)
{
    *available = client_cert.is_loaded;
    *matches_endpoint = client_cert.is_san_equal_endpoint_id;
}


/*********************************************************************//**
**
** DEVICE_SECURITY_TrustCertVerifyCallbackWithCertChain
**
** Called back from OpenSSL for each certificate in the received server certificate chain of trust
** This function allows the caller to provide the certificate chain, instead of deriving it from
** the parent ssl itself. This is also called by the normal TrustCertVerifyCallback once it retrieves
** the cert chain.
** This function is used to ignore certificate validation errors caused by system time being incorrect
**
** \param   preverify_ok - set to 1, if the current certificate passed, set to 0 if it did not
** \param   x509_ctx - pointer to context for certificate chain verification
** \param   p_cert_chain - pointer to variable in which to return a pointer to a saved cert chain
**                         NOTE: If a cert chain is already saved, then the cert chain is not updated
**
** \return  1 if certificate chain should be trusted
**          0 if certificate chain should not be trusted, and connection dropped
**
**************************************************************************/
int DEVICE_SECURITY_TrustCertVerifyCallbackWithCertChain(int preverify_ok, X509_STORE_CTX *x509_ctx, STACK_OF_X509 **p_cert_chain)
{
    int cert_err;
    bool is_reliable;
    is_system_time_reliable_cb_t   is_system_time_reliable_cb;
    int err_depth;        // A depth of 0 indicates the server cert, 1=intermediate cert (CA cert) etc
    char *err_string;
    STACK_OF(X509) *cert_chain;
    char buf[MAX_ISO8601_LEN];

    // Save the certificate chain back into the STOMP connection if not done so already
    // (This function is called for each certificate in the chain, so it might have been done already)
    if ((p_cert_chain != NULL) && (*p_cert_chain == NULL))
    {
        cert_chain = X509_STORE_CTX_get1_chain(x509_ctx);
        if (cert_chain == NULL)
        {
            USP_LOG_Error("%s: X509_STORE_CTX_get1_chain() failed", __FUNCTION__);
            return 0;
        }

        *p_cert_chain = cert_chain;
    }

    // Exit if OpenSSL validation has passed
    if (preverify_ok == 1)
    {
        return 1;
    }

    // From this point on, OpenSSL had determined that the certificate could not be trusted
    // Fail validation if the reason the certificate could not be trusted was not one related to validity time
    cert_err = X509_STORE_CTX_get_error(x509_ctx);
    if ((cert_err != X509_V_ERR_CERT_NOT_YET_VALID) &&
        (cert_err != X509_V_ERR_CERT_HAS_EXPIRED) &&
        (cert_err != X509_V_ERR_CRL_NOT_YET_VALID) &&
        (cert_err != X509_V_ERR_CRL_HAS_EXPIRED) )
    {
        err_string = (char *) X509_verify_cert_error_string(cert_err);
        err_depth = X509_STORE_CTX_get_error_depth(x509_ctx);
        USP_LOG_Error("%s: OpenSSL error: %s (err_code=%d) at depth=%d", __FUNCTION__, err_string, cert_err, err_depth);

        if (p_cert_chain != NULL)
        {
            LogCertChain(*p_cert_chain);
        }
        LogTrustCerts();
        return 0;
    }

    // Determine function to call to get whether system time is reliable yet
    is_system_time_reliable_cb = vendor_hook_callbacks.is_system_time_reliable_cb;
    if (is_system_time_reliable_cb == NULL)
    {
        is_system_time_reliable_cb = IsSystemTimeReliable;
    }

    // Pass validation if the certificate validity errors are due to system time not being reliable
    is_reliable = is_system_time_reliable_cb();
    if (is_reliable == false)
    {
        X509_STORE_CTX_set_error(x509_ctx, X509_V_OK); // Ensure that SSL_get_verify_result() returns X509_V_OK
        return 1;
    }

    // If the code gets here, then the cert validity time check failed whilst system time was reliable, so fail validation
    USP_LOG_Error("%s: Cert validity time check failed whilst system time was reliable (current system time=%s)", __FUNCTION__, iso8601_cur_time(buf, sizeof(buf)));
    return 0;
}

/*********************************************************************//**
**
** DEVICE_SECURITY_TrustCertVerifyCallback
**
** Called back from OpenSSL for each certificate in the received server certificate chain of trust
** This function saves the certificate chain into the STOMP connection structure
** This function is used to ignore certificate validation errors caused by system time being incorrect
**
** \param   preverify_ok - set to 1, if the current certificate passed, set to 0 if it did not
** \param   x509_ctx - pointer to context for certificate chain verification
**
** \return  1 if certificate chain should be trusted
**          0 if certificate chain should not be trusted, and connection dropped
**
**************************************************************************/
int DEVICE_SECURITY_TrustCertVerifyCallback(int preverify_ok, X509_STORE_CTX *x509_ctx)
{
    STACK_OF(X509) **p_cert_chain;
    SSL *ssl;

    // Get the parent SSL context
    ssl = X509_STORE_CTX_get_ex_data(x509_ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
    USP_ASSERT(ssl != NULL);

    // Get the pointer to variable in which to save the certificate chain
    p_cert_chain = (STACK_OF(X509) **)SSL_get_app_data(ssl);

    // Simplification, allow this to be functionally the same with origin of p_cert_chain to differ
    return DEVICE_SECURITY_TrustCertVerifyCallbackWithCertChain(preverify_ok, x509_ctx, p_cert_chain);
}

/*********************************************************************//**
**
** DEVICE_SECURITY_NoSaveTrustCertVerifyCallback
**
** Called back from OpenSSL for each certificate in the received server certificate chain of trust
** This function is used to ignore certificate validation errors caused by system time being incorrect
** NOTE: This code is different from DEVICE_SECURITY_TrustCertVerifyCallback() in that it does not
**       save the certificate chain for use by ControllerTrust
**
** \param   preverify_ok - set to 1, if the current certificate passed, set to 0 if it did not
** \param   x509_ctx - pointer to context for certificate chain verification
**
** \return  1 if certificate chain should be trusted
**          0 if certificate chain should not be trusted, and connection dropped
**
**************************************************************************/
int DEVICE_SECURITY_NoSaveTrustCertVerifyCallback(int preverify_ok, X509_STORE_CTX *x509_ctx)
{
    int cert_err;
    bool is_reliable;
    is_system_time_reliable_cb_t   is_system_time_reliable_cb;
    int err_depth;        // A depth of 0 indicates the server cert, 1=intermediate cert (CA cert) etc
    char *err_string;
    STACK_OF(X509) *cert_chain;
    SSL *ssl;
    char buf[MAX_ISO8601_LEN];

    // Get the parent SSL context
    ssl = X509_STORE_CTX_get_ex_data(x509_ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
    USP_ASSERT(ssl != NULL);

    // Exit if OpenSSL validation has passed
    if (preverify_ok == 1)
    {
        return 1;
    }

    // From this point on, OpenSSL had determined that the certificate could not be trusted
    // Fail validation if the reason the certificate could not be trusted was not one related to validity time
    cert_err = X509_STORE_CTX_get_error(x509_ctx);
    if ((cert_err != X509_V_ERR_CERT_NOT_YET_VALID) &&
        (cert_err != X509_V_ERR_CERT_HAS_EXPIRED) &&
        (cert_err != X509_V_ERR_CRL_NOT_YET_VALID) &&
        (cert_err != X509_V_ERR_CRL_HAS_EXPIRED) )
    {
        err_string = (char *) X509_verify_cert_error_string(cert_err);
        err_depth = X509_STORE_CTX_get_error_depth(x509_ctx);
        USP_LOG_Error("%s: OpenSSL error: %s (err_code=%d) at depth=%d", __FUNCTION__, err_string, cert_err, err_depth);

        cert_chain = X509_STORE_CTX_get1_chain(x509_ctx);
        if (cert_chain == NULL)
        {
            USP_LOG_Error("%s: X509_STORE_CTX_get1_chain() failed", __FUNCTION__);
            return 0;
        }

        LogCertChain(cert_chain);
        LogTrustCerts();
        sk_X509_pop_free(cert_chain, X509_free);
        return 0;
    }

    // Determine function to call to get whether system time is reliable yet
    is_system_time_reliable_cb = vendor_hook_callbacks.is_system_time_reliable_cb;
    if (is_system_time_reliable_cb == NULL)
    {
        is_system_time_reliable_cb = IsSystemTimeReliable;
    }

    // Pass validation if the certificate validity errors are due to system time not being reliable
    is_reliable = is_system_time_reliable_cb();
    if (is_reliable == false)
    {
        X509_STORE_CTX_set_error(x509_ctx, X509_V_OK); // Ensure that SSL_get_verify_result() returns X509_V_OK
        return 1;
    }

    // If the code gets here, then the cert validity time check failed whilst system time was reliable, so fail validation
    USP_LOG_Error("%s: Cert validity time check failed whilst system time was reliable (current system time=%s)", __FUNCTION__, iso8601_cur_time(buf, sizeof(buf)));
    return 0;
}

/*********************************************************************//**
**
** DEVICE_SECURITY_AddCertHostnameValidation
**
**
** Called to add automatic hostname validation in later versions of OpenSSL.
** Will silently return success if unsupported.
** Both this, and the function DEVICE_SECURITY_AddCertHostnameValidationCtx are
** required as STOMP uses a single ctx for all connections.
**
** \param   ssl - an SSL object
** \param   name - string of the (host)name
** \param   length - name length
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_SECURITY_AddCertHostnameValidation(SSL* ssl, const char* name, size_t length)
{
#if OPENSSL_VERSION_NUMBER >= 0x1000200FL // SSL version 1.0.2
{
    // Enable automatic hostname validation in later versions of OpenSSL
    // Exit if unable to get the verify parameter object, which we are going to set properties on
    X509_VERIFY_PARAM *verify_object;
    verify_object = SSL_get0_param(ssl);
    if (verify_object == NULL)
    {
        USP_LOG_Error("%s: SSL_get0_param() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Set the properties on the verify object
    // These fail the cert if the server hostname doesn't match the SubjectAltName (SAN) in the cert
    // If SAN is not present, the cert is failed if hostname doesn't match the CommonName (CN) in the cert
    // If neither SAN, nor CN are present in the cert, then the cert will automatically be failed by the verify object set
    // For wildcarded hostnames in the cert, the '*' must be a subdomain (eg *.foo.com, *.foo.bar.com, but not *.com)
    // and Partial Wildcards eg f*oo.bar.com are not allowed ('*' must represent a full subdomain)
    X509_VERIFY_PARAM_set_hostflags(verify_object, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    X509_VERIFY_PARAM_set1_host(verify_object, name, length);
}
#endif

#if OPENSSL_VERSION_NUMBER >= 0x0090806f // SSL version 0.9.8f
    SSL_set_tlsext_host_name(ssl, name);
#endif

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_SECURITY_AddCertHostnameValidationCtx
**
** Called to add automatic hostname validation in later versions of OpenSSL.
** Will silently return success if unsupported.
** This version takes an SSL CTX object, instead of SSL.
**
** \param   ssl_ctx - an SSL CTX object
** \param   name - string of the (host)name
** \param   length - name length
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_SECURITY_AddCertHostnameValidationCtx(SSL_CTX* ssl_ctx, const char* name, size_t length)
{
#if OPENSSL_VERSION_NUMBER >= 0x1000200FL // SSL version 1.0.2
    if (ssl_ctx == NULL)
    {
        USP_LOG_Error("%s: SSL_CTX is NULL", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    X509_VERIFY_PARAM* verify_object;
    verify_object = SSL_CTX_get0_param(ssl_ctx);
    if (verify_object == NULL)
    {
        USP_LOG_Error("%s: SSL_CTX_get0_param() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Set the properties on the verify object
    // These fail the cert if the server hostname doesn't match the SubjectAltName (SAN) in the cert
    // If SAN is not present, the cert is failed if hostname doesn't match the CommonName (CN) in the cert
    // If neither SAN, nor CN are present in the cert, then the cert will automatically be failed by the verify object set
    // For wildcarded hostnames in the cert, the '*' must be a subdomain (eg *.foo.com, *.foo.bar.com, but not *.com)
    // and Partial Wildcards eg f*oo.bar.com are not allowed ('*' must represent a full subdomain)
    X509_VERIFY_PARAM_set_hostflags(verify_object, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    X509_VERIFY_PARAM_set1_host(verify_object, name, length);
#endif

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DEVICE_SECURITY_ValidateALPN
**
** Determines whether the specified ALPN options are valid
**
** \param   req - pointer to structure identifying the parameter (unused)
** \param   value - value that the controller would like to set the parameter to
**
** \return  USP_ERR_OK if options are valid
**
**************************************************************************/
int DEVICE_SECURITY_ValidateALPN(dm_req_t *req, char *value)
{
    int len;
    str_vector_t sv;
    char *option;
    int i;
    int err = USP_ERR_OK;

    (void)req;      // Keep compiler happy about unused argument

    // Exit if no ALPN options to validate
    if (*value == '\0')
    {
        return USP_ERR_OK;
    }

    // Form a vector of each of the options to set
    TEXT_UTILS_SplitString(value, &sv, ",");

    for (i=0; i < sv.num_entries; i++)
    {
        option = sv.vector[i];
        len = strlen(option);
        USP_ASSERT(len != 0);       // TEXT_UTILS_SplitString() should not have populated the string vector with zero length strings
        if (len > 255)
        {
            option[20] = '\0';      // Truncate the option, in order to log it
            USP_ERR_SetMessage("%s: ALPN option too long ('%s...')", __FUNCTION__, option);
            err = USP_ERR_INVALID_ARGUMENTS;
            goto exit;
        }
    }

exit:
    STR_VECTOR_Destroy(&sv);
    return err;
}

/*********************************************************************//**
**
** DEVICE_SECURITY_SetALPN
**
** Sets the Application Layer Protocol Negotiation options to put in all subsequent SSL ClientHello messages
**
** \param   ssl_ctx - pointer to SSL context to set the ALPN options up in
** \param   alpn - string containing comma separated list of ALPN options
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int DEVICE_SECURITY_SetALPN(SSL_CTX *ssl_ctx, char *alpn)
{
    int len;
    str_vector_t sv;
    char *option;
    unsigned char *buf;
    unsigned char *p;
    int err;
    int i;

    // Exit if no ALPN options to set
    if ((alpn == NULL) || (*alpn == '\0'))
    {
        // Turn off any ALPN options set previously.
        SSL_CTX_set_alpn_protos(ssl_ctx, NULL, 0);   // NOTE: This call returns an error, but does work in clearing any previously set ALPN options
        return USP_ERR_OK;
    }

    // Form a vector of each of the options to set
    TEXT_UTILS_SplitString(alpn, &sv, ",");

    // Allocate a buffer to place the ALPN options in the format required by OpenSSL library
    // The buffer contains one length byte followed by the option, for each option
    len = strlen(alpn) + 1;     // Plus 1 to include a length byte for the first option. The length byte of subsequent options are covered by the comma character
    buf = USP_MALLOC(len);

    p = buf;
    for (i=0; i < sv.num_entries; i++)
    {
        option = sv.vector[i];
        len = strlen(option);
        USP_ASSERT((len > 0) && (len <= 255));

        WRITE_BYTE(p, len);
        WRITE_N_BYTES(p, option, len);
    }

    len = p - buf;
    err = SSL_CTX_set_alpn_protos(ssl_ctx, (const unsigned char *) buf, len);
    if (err != 0)
    {
        USP_LOG_Error("%s: Failed to set ALPN in the SSL CTX", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Clean up
    USP_FREE(buf);
    STR_VECTOR_Destroy(&sv);

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** LoadCerts_FromPath
**
** Called to load all certificates in the specified file or directory into the Device.Security.Certificate table
**
** \param   path - file or directory containing system certs
** \param   cert_usage - type of certificates to add
** \param   role_instance - if the certificates are trust certs, then this is the role that the certs permit to a broker cert
**
** \return  None - errors are ignored
**
**************************************************************************/
void LoadCerts_FromPath(char *path, cert_usage_t cert_usage, int role_instance)
{
    int err;
    struct stat info;
    mode_t type;

    // Exit if no path specified
    if (*path == '\0')
    {
        return;
    }

    // Exit if unable to determine whether the path is for a file or a directory
    err = stat(path, &info);
    if (err != 0)
    {
        char buf[256];
        USP_SNPRINTF(buf, sizeof(buf), "Unable to access %s : stat", path);
        USP_ERR_ERRNO(buf, errno);
        return;
    }

    // Process the certs contained in the path, based on whether the path is a file or directory
    type = info.st_mode & S_IFMT;
    if ((type == S_IFREG) || (type == S_IFLNK))
    {
        LoadCerts_FromFile(path, cert_usage, role_instance);
    }
    else if (type == S_IFDIR)
    {
        LoadCerts_FromDir(path, cert_usage, role_instance);
    }
    else
    {
        USP_LOG_Warning("%s: %s is not a file or directory. Ignoring path for %s", __FUNCTION__, path, TEXT_UTILS_EnumToString(cert_usage, cert_usages, NUM_ELEM(cert_usages)) );
    }
}

/*********************************************************************//**
**
** LoadCerts_FromFile
**
** Called to load all certificates contained in the specified file (in PEM format) into the Device.Security.Certificate table
**
** \param   file_path - file containing certs
** \param   cert_usage - type of certificates to add
** \param   role_instance - if the certificates are trust certs, then this is the role that the certs permit to a broker cert
**
** \return  None - errors are ignored
**
**************************************************************************/
void LoadCerts_FromFile(char *file_path, cert_usage_t cert_usage, int role_instance)
{
    FILE *fp;
    X509 *cert;
    int err;

    // Exit if unable to open the file containing the certs in PEM format
    fp = fopen(file_path, "r");
    if (fp == NULL)
    {
        USP_LOG_Error("%s: Unable to open %s", __FUNCTION__, usp_trust_store_file);
        return;
    }

    // Iterate over all certs in the file
    cert = PEM_read_X509(fp, NULL, NULL, NULL);
    while (cert != NULL)
    {
        // Skip this file if unable to add the certificate
        // NOTE: Ownership of the X509 cert passes to AddCert(), so no need to free it in this function
        err = AddCert(cert, cert_usage, role_instance);
        if (err != USP_ERR_OK)
        {
            continue;
        }

        // Read the next cert. If no more certs in the file, then the pointer returned will be NULL
        cert = PEM_read_X509(fp, NULL, NULL, NULL);
    }

    // Close the file
    fclose(fp);
}

/*********************************************************************//**
**
** LoadCerts_FromDir
**
** Called to load all certificates in the specified directory into the Device.Security.Certificate table
**
** \param   dir_path - directory containing certs
** \param   cert_usage - type of certificates to add
** \param   role_instance - if the certificates are trust certs, then this is the role that the certs permit to a broker cert
**
** \return  None - errors are ignored
**
**************************************************************************/
void LoadCerts_FromDir(char *dir_path, cert_usage_t cert_usage, int role_instance)
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    int err;
    char file_path[PATH_MAX];
    mode_t type;
    X509 *cert;

    // Exit if unable to open the specified directory to read
    dir = opendir(dir_path);
    if (dir == NULL)
    {
        USP_ERR_ERRNO("opendir", errno);
        return;
    }

    // Iterate over all entries in the directory
    while (FOREVER)
    {
        // Exit loop if unable to retrieve the next entry in the directory
        errno = 0;
        entry = readdir(dir);
        if (errno != 0)
        {
            USP_ERR_ERRNO("readdir", errno);
            break;
        }

        // Exit loop if iterated over all entries in the directory
        if (entry == NULL)
        {
            break;
        }

        // Skip this entry, if unable to determine whether this entry is a file
        USP_SNPRINTF(file_path, sizeof(file_path), "%s/%s", dir_path, entry->d_name);
        err = stat(file_path, &info);
        if (err != 0)
        {
            USP_ERR_ERRNO(file_path, err);
            continue;
        }

        // Skip this entry if it's not a file or symbolic link
        type = info.st_mode & S_IFMT;
        if ((type != S_IFREG) && (type != S_IFLNK))
        {
            continue;
        }

        // Skip this file if unable to retrieve the certificate (file must be in PEM format)
        err = GetCertFromFile(file_path, &cert, NULL);
        if (err != USP_ERR_OK)
        {
            continue;
        }

        // Skip this file if unable to add the certificate
        // NOTE: Ownership of the X509 cert passes to AddCert(), so no need to free it in this function
        err = AddCert(cert, cert_usage, role_instance);
        if (err != USP_ERR_OK)
        {
            continue;
        }
    }

    // Close the directory
    closedir(dir);
}

/*********************************************************************//**
**
** LoadClientCert
**
** Called to load the client certificate authenticating this agent
**
** \param   ctx - pointer to SSL context to load the certificate into
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int LoadClientCert(SSL_CTX *ctx)
{
    int err;
    X509 *cert = NULL;
    EVP_PKEY *pkey = NULL;

    // Exit if an error occurred whilst trying to get the client cert
    err = GetClientCert(&cert, &pkey);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if no cert was obtained. NOTE: This is not an error
    if ((cert == NULL) || (pkey == NULL))
    {
        USP_LOG_Info("%s: Not using a device certificate for connections", __FUNCTION__);
        return USP_ERR_OK;
    }

    // Exit if unable to add this agent's certificate
    // NOTE: X509 objects have a reference counter, calling SSL_CTX_use_certificate() just increases the reference count
    err = SSL_CTX_use_certificate(ctx, cert);
    if (err != 1)
    {
        USP_ERR_SetMessage("%s: SSL_CTX_use_certificate() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to add the private key
    // NOTE: Private key objects have a reference counter, calling SSL_CTX_use_PrivateKey() just increases the reference count
    err = SSL_CTX_use_PrivateKey(ctx, pkey);
    if (err != 1)
    {
        USP_ERR_SetMessage("%s: SSL_CTX_use_PrivateKey() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Cache the client certificate and private key, to be used by DEVICE_SECURITY_LoadTrustStore()
    agent_cert = cert;
    agent_pkey = pkey;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetClientCert
**
** Called to get the client certificate authenticating this agent
**
** \param   p_cert - pointer to variable in which to return a pointer to the client cert
** \param   p_pkey - pointer to variable in which to return a pointer to the client cert's private key
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetClientCert(X509 **p_cert, EVP_PKEY **p_pkey)
{
    int err;

    // If the client cert file to use is specified on the command line, this overrides all other methods of configuring the client cert
    // NOTE: The string compare test is present in order to allow an invocation of USP Agent to specify no auth cert file using -a "null". Useful, if the -a option is always used in all invocations.
    if ((auth_cert_file != NULL) && (*auth_cert_file != '\0') && (strcmp(auth_cert_file, "null") != 0))
    {
        err = GetCertFromFile(auth_cert_file, p_cert, p_pkey);
        return err;
    }

    // Otherwise, attempt to read the client cert from an in-memory buffer provided by the get_agent_cert vendor hook
    err = GetClientCertFromMemory(p_cert, p_pkey);
    return err;
}

/*********************************************************************//**
**
** GetCertFromFile
**
** Gets a certificate and optionally associated private key from a file containing both in PEM format
**
** \param   cert_file - filesystem path to the file containing the PEM formatted cert data concatenated with the PEM formatted private key
** \param   p_cert - pointer to variable in which to return a pointer to the client cert
** \param   p_pkey - pointer to variable in which to return a pointer to the client cert's private key or NULL if private key not required
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetCertFromFile(char *cert_file, X509 **p_cert, EVP_PKEY **p_pkey)
{
    int result;
    BIO *bio = NULL;
    X509 *cert = NULL;
    EVP_PKEY *pkey = NULL;
    int err = USP_ERR_INTERNAL_ERROR;

    // Exit if unable to create a bio to read from the cert file
    bio = BIO_new(BIO_s_file());
    if (bio == NULL)
    {
        USP_ERR_SetMessage("%s: BIO_new() failed", __FUNCTION__);
        goto exit;
    }

    // Exit if unable to set the file to read from
    result = BIO_read_filename(bio, cert_file);
    if (result <= 0)
    {
        USP_ERR_SetMessage("%s: BIO_read_filename(%s) failed", __FUNCTION__, cert_file);
        goto exit;
    }

    // Exit if unable to parse an X509 structure from the file
    cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);
    if (cert == NULL)
    {
        USP_ERR_SetMessage("%s: PEM_read_bio_X509(%s) failed", __FUNCTION__, cert_file);
        goto exit;
    }

    // Public cert retrieved successfully
    *p_cert = cert;

    // Optionally retrieve private key
    // NOTE: This is only performed for client certs, in which a private key is expected to be present.
    // Trust store CA certs do not contain private keys
    if (p_pkey != NULL)
    {
        // Exit if unable to reset the bio, to go back to the beginning of the file
        result = BIO_reset(bio);
        if (result != 0)
        {
            USP_ERR_SetMessage("%s: BIO_reset() failed", __FUNCTION__);
            goto exit;
        }

        // Exit if unable to parse a EVP_PKEY (private key) structure from the file
        pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
        if (pkey == NULL)
        {
            USP_ERR_SetMessage("%s: PEM_read_bio_PrivateKey(%s) failed", __FUNCTION__, cert_file);
            goto exit;
        }

        // Private key retrieved successfully
        *p_pkey = pkey;
    }

    // If the code gets here, then it was successful
    err = USP_ERR_OK;

exit:
    // Clean up, if an error occurred
    if (err != USP_ERR_OK)
    {
        if (cert != NULL)
        {
            X509_free(cert);
        }

        if (pkey != NULL)
        {
            EVP_PKEY_free(pkey);
        }
    }

    if (bio != NULL)
    {
        BIO_free(bio);
    }

    return err;
}

/*********************************************************************//**
**
** GetClientCertFromMemory
**
** Gets a client certificate and associated private key from a buffer supplied by the get_agent_cert vendor hook
** The buffer containing the cert is in binary DER format
**
** \param   p_cert - pointer to variable in which to return a pointer to the client cert
** \param   p_pkey - pointer to variable in which to return a pointer to the client cert's private key
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetClientCertFromMemory(X509 **p_cert, EVP_PKEY **p_pkey)
{
    int err;
    const unsigned char *in;
    X509 *cert;
    EVP_PKEY *pkey;
    agent_cert_info_t info = {0};
    get_agent_cert_cb_t get_agent_cert_cb;

    // Setup default return values
    *p_cert = NULL;
    *p_pkey = NULL;

    // Determine function to call to get the client cert
    get_agent_cert_cb = vendor_hook_callbacks.get_agent_cert_cb;
    if (get_agent_cert_cb == NULL)
    {
        return USP_ERR_OK;
    }

    // Obtain the agent certificate and key from the vendor
    err = get_agent_cert_cb(&info);
    if (err != USP_ERR_OK)
    {
        USP_ERR_SetMessage("%s: get_agent_cert_cb() failed", __FUNCTION__);
        return err;
    }

    // Exit if no client cert was provided. In this case no client cert will be presented to the broker
    // NOTE: This condition is handled gracefully by the caller.
    if ((info.cert_data == NULL) || (info.cert_len == 0) || (info.key_data == NULL) || (info.key_len == 0))
    {
        return USP_ERR_OK;
    }

    // Exit if unable to convert the buffer into an X509 structure
    const unsigned char *cert_data = info.cert_data;
    cert = d2i_X509(NULL, &cert_data, (long)info.cert_len);
    if (cert == NULL)
    {
        USP_ERR_SetMessage("%s: d2i_X509() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to read the private key into an EVP_PKEY internal data structure
    // d2i_AutoPrivateKey() determines whether the supplied key is RSA or DSA.
    in = info.key_data;
    pkey = d2i_AutoPrivateKey(NULL, &in, info.key_len);
    if (pkey == NULL)
    {
        USP_ERR_SetMessage("%s: d2i_AutoPrivateKey() failed", __FUNCTION__);
        X509_free(cert);
        return USP_ERR_INTERNAL_ERROR;
    }

    // If the code gets here, the client certificate was extracted successfully
    *p_cert = cert;
    *p_pkey = pkey;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** LogCertChain
**
** Logs the Subject and Issuer of each certificate in a certificate chain
**
** \param   cert_chain - pointer to chain of certificates
**                       The cert at position[0] will be the STOMP broker cert
**                       The cert at position[1] will be the CA cert that validates the broker cert
**                       The certs at higher positions are higher level CA certs, all the way up to one in our trust store
**
** \return  None
**
**************************************************************************/
void LogCertChain(STACK_OF_X509 *cert_chain)
{
    int i;
    X509 *cert;
    unsigned num_certs;
    char subject[257];
    char issuer[257];

    // Iterate over all certs in the chain, printing their subject and issuer
    num_certs = sk_X509_num(cert_chain);
    USP_LOG_Info("\nCertificate Chain: Peer cert at position [0], Root cert at position [%d]", num_certs-1);
    for (i=0; i<num_certs; i++)
    {
        cert = (X509*) sk_X509_value(cert_chain, i);
        if (cert == NULL)
        {
            char *fail_str = "Unable to get cert";
            USP_STRNCPY(subject, fail_str, sizeof(subject));
            USP_STRNCPY(issuer, fail_str, sizeof(issuer));
        }
        else
        {
            X509_NAME_oneline(X509_get_subject_name(cert), subject, sizeof(subject));
            X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof(issuer));
        }
        USP_LOG_Info("[%d] Subject: %s", i, subject);
        USP_LOG_Info("[%d]      (Issuer: %s)", i, issuer);
    }
}

/*********************************************************************//**
**
** LogTrustCerts
**
** Logs the Subject and Issuer of each certificate in the trust store
**
** \param   None
**
** \return  None
**
**************************************************************************/
void LogTrustCerts(void)
{
    int i;
    cert_t *ct;

    USP_LOG_Info("\nTrust Store certificates:");
    for (i=0; i<num_all_certs; i++)
    {
        ct = &all_certs[i];
        if (ct->cert_usage == kCertUsage_TrustCert)
        {
            USP_LOG_Info("[%d] Subject: %s", i, ct->subject);
            USP_LOG_Info("[%d]      (Issuer: %s)", i, ct->issuer);
        }
    }
}

/*********************************************************************//**
**
** LogCert_DER
**
** Logs the specified certificate in DER format
**
** \param   cert - pointer to SSL certificate to log
**
** \return  None
**
**************************************************************************/
void LogCert_DER(X509 *cert)
{
    int len;
    unsigned char *buf = NULL;

    len = i2d_X509(cert, &buf);
    USP_LOG_HexBuffer("cert", buf, len);
    OPENSSL_free(buf);

}

/*********************************************************************//**
**
** AddClientCert
**
** Determines if a client cert has been loaded, and if so parses it
**
** \param   ctx - pointer to SSL context in which the client cert might have been loaded
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int AddClientCert(SSL_CTX *ctx)
{
    int err;
    X509 *cert;
    char *subject_alt = NULL;
    char *endpoint_id;

    // Exit if no client cert has been installed (this is not an error).
    // NOTE: We have to get it from the SSL context, because it could have been installed using the load_agent_cert vendor hook
    cert = SSL_CTX_get0_certificate(ctx);
    if (cert == NULL)
    {
        return USP_ERR_OK;
    }

    // Parse the certificate
    err = USP_ERR_OK;
    err |= ParseCert_SerialNumber(cert, &client_cert.serial_number);
    err |= ParseCert_Issuer(cert, &client_cert.issuer);
    err |= ParseCert_SubjectAlt(cert, &subject_alt);
    if (err != USP_ERR_OK)
    {
        USP_LOG_Error("%s: Failed to parse client certificate", __FUNCTION__);
        USP_SAFE_FREE(subject_alt);
        return USP_ERR_INTERNAL_ERROR;
    }
    USP_ASSERT(subject_alt != NULL);

    // Exit if unable to add the client certificate into Device.Security.Certificate
    // NOTE: Ownership of the X509 cert stays with SSL ctx and agent_cert pointer
    err = AddCert(cert, kCertUsage_ClientCert, INVALID);
    if (err != USP_ERR_OK)
    {
        USP_SAFE_FREE(subject_alt);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Determine whether the SubjectAltName field in the certificate matches the URN form of the agent's endpoint_id
    #define URN_PREFIX "urn:bbf:usp:id:"
    #define URN_PREFIX_LEN (sizeof(URN_PREFIX)-1)    // Minus 1 to not include the NULL terminator
    endpoint_id = DEVICE_LOCAL_AGENT_GetEndpointID();
    if ((strlen(subject_alt) > URN_PREFIX_LEN) &&
        (strncmp(subject_alt, URN_PREFIX, URN_PREFIX_LEN)==0) &&
        (strcmp(&subject_alt[URN_PREFIX_LEN], endpoint_id)==0))
    {
        client_cert.is_san_equal_endpoint_id = true;
    }

    USP_LOG_Info("%s: Using a device certificate for connections (SubjectAltName=%s)", __FUNCTION__, subject_alt);
    USP_SAFE_FREE(subject_alt);

    // Mark the client certificate as loaded
    client_cert.is_loaded = true;

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_NumCerts
**
** Gets the value of Device.Security.CertificateNumberOfEntries
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_NumCerts(dm_req_t *req, char *buf, int len)
{
    val_uint = num_all_certs;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetCert_LastModif
**
** Gets the value of Device.Security.Certificate.{i}.LastModif
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetCert_LastModif(dm_req_t *req, char *buf, int len)
{
    cert_t *ct;

    // Write the value into the return buffer
    ct = Find_SecurityCertByReq(req);
    USP_ASSERT(ct != NULL);
    val_datetime = ct->last_modif;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetCert_SerialNumber
**
** Gets the value of Device.Security.Certificate.{i}.SerialNumber
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetCert_SerialNumber(dm_req_t *req, char *buf, int len)
{
    cert_t *ct;

    // Write the value into the return buffer
    ct = Find_SecurityCertByReq(req);
    USP_ASSERT(ct != NULL);
    USP_SNPRINTF(buf, len, "%s", ct->serial_number);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetCert_Issuer
**
** Gets the value of Device.Security.Certificate.{i}.Issuer
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetCert_Issuer(dm_req_t *req, char *buf, int len)
{
    cert_t *ct;

    // Write the value into the return buffer
    ct = Find_SecurityCertByReq(req);
    USP_ASSERT(ct != NULL);
    USP_SNPRINTF(buf, len, "%s", ct->issuer);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetCert_NotBefore
**
** Gets the value of Device.Security.Certificate.{i}.NotBefore
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetCert_NotBefore(dm_req_t *req, char *buf, int len)
{
    cert_t *ct;

    // Write the value into the return buffer
    ct = Find_SecurityCertByReq(req);
    USP_ASSERT(ct != NULL);
    USP_STRNCPY(buf, ct->not_before, len);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetCert_NotAfter
**
** Gets the value of Device.Security.Certificate.{i}.NotAfter
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetCert_NotAfter(dm_req_t *req, char *buf, int len)
{
    cert_t *ct;

    // Write the value into the return buffer
    ct = Find_SecurityCertByReq(req);
    USP_ASSERT(ct != NULL);
    USP_STRNCPY(buf, ct->not_after, len);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetCert_Subject
**
** Gets the value of Device.Security.Certificate.{i}.Subject
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetCert_Subject(dm_req_t *req, char *buf, int len)
{
    cert_t *ct;

    // Write the value into the return buffer
    ct = Find_SecurityCertByReq(req);
    USP_ASSERT(ct != NULL);
    USP_SNPRINTF(buf, len, "%s", ct->subject);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetCert_SubjectAlt
**
** Gets the value of Device.Security.Certificate.{i}.SubjectAlt
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetCert_SubjectAlt(dm_req_t *req, char *buf, int len)
{
    cert_t *ct;

    // Write the value into the return buffer
    ct = Find_SecurityCertByReq(req);
    USP_ASSERT(ct != NULL);
    USP_SNPRINTF(buf, len, "%s", ct->subject_alt);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetCert_SignatureAlgorithm
**
** Gets the value of Device.Security.Certificate.{i}.SignatureAlgorithm
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetCert_SignatureAlgorithm(dm_req_t *req, char *buf, int len)
{
    cert_t *ct;

    // Write the value into the return buffer
    ct = Find_SecurityCertByReq(req);
    USP_ASSERT(ct != NULL);
    USP_SNPRINTF(buf, len, "%s", ct->signature_algorithm);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Get_NumTrustCerts
**
** Gets the value of Device.LocalAgent.CertificateNumberOfEntries
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Get_NumTrustCerts(dm_req_t *req, char *buf, int len)
{
    val_uint = num_trust_certs;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetLaCert_SerialNumber
**
** Gets the value of Device.LocalAgent.Certificate.{i}.SerialNumber
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetLaCert_SerialNumber(dm_req_t *req, char *buf, int len)
{
    cert_t *ct;

    // Write the value into the return buffer
    ct = Find_LocalAgentCertByReq(req);
    USP_ASSERT(ct != NULL);
    USP_SNPRINTF(buf, len, "%s", ct->serial_number);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** GetLaCert_Issuer
**
** Gets the value of Device.LocalAgent.Certificate.{i}.Issuer
**
** \param   req - pointer to structure identifying the parameter
** \param   buf - pointer to buffer into which to return the value of the parameter (as a textual string)
** \param   len - length of buffer in which to return the value of the parameter
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int GetLaCert_Issuer(dm_req_t *req, char *buf, int len)
{
    cert_t *ct;

    // Write the value into the return buffer
    ct = Find_LocalAgentCertByReq(req);
    USP_ASSERT(ct != NULL);
    USP_SNPRINTF(buf, len, "%s", ct->issuer);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Find_SecurityCertByReq
**
** Returns a pointer to the certificate with the specified Device.Security.Certificate instance number
**
** \param   req - pointer to structure identifying the instance
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
cert_t *Find_SecurityCertByReq(dm_req_t *req)
{
    int i;
    cert_t *ct;
    int index;

    // Normally the certificates are arranged in instance order in the vector
    // Exit, if this is the case. NOTE: This is a speedup to avoid having to iterate over all certs (as later on in this function)
    index = inst1 - 1;
    USP_ASSERT((index >= 0) && (index < num_all_certs));
    ct = &all_certs[index];
    if (ct->instance == inst1)
    {
        return ct;
    }

    // If the certificates are not arranged in instance order, then iterate over all certs,
    // finding the one with matching instance number
    for (i=0; i<num_all_certs; i++)
    {
        ct = &all_certs[i];
        if (ct->instance == inst1)
        {
            return ct;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** Find_LocalAgentCertByReq
**
** Returns a pointer to the certificate with the specified Device.LocalAgent.Certificate instance number
**
** \param   req - pointer to structure identifying the instance
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
cert_t *Find_LocalAgentCertByReq(dm_req_t *req)
{
    int i;
    cert_t *ct;
    int index;

    // Normally the certificates are arranged in instance order in the vector
    // Exit, if this is the case. NOTE: This is a speedup to avoid having to iterate over all certs (as later on in this function)
    index = inst1 - 1;
    USP_ASSERT((index >= 0) && (index < num_trust_certs));
    ct = &all_certs[index];
    if (ct->la_instance == inst1)
    {
        return ct;
    }

    // If the certificates are not arranged in instance order, then iterate over all certs,
    // finding the one with matching instance number
    for (i=0; i<num_all_certs; i++)
    {
        ct = &all_certs[i];
        if (ct->la_instance == inst1)
        {
            return ct;
        }
    }

    return NULL;
}

/*********************************************************************//**
**
** Find_LocalAgentCertByHash
**
** Finds the certificate in our trust store that matches the given hash
**
** \param   hash - hash of the trusted cert we want to find
**
** \return  pointer to certificate, or INVALID if not found
**
**************************************************************************/
cert_t *Find_LocalAgentCertByHash(cert_hash_t hash)
{
    int i;
    cert_t *ct;

    // Iterate over all certificates in our trust store
    for (i=0; i<num_all_certs; i++)
    {
        // Exit if we've found a matching certificate
        ct = &all_certs[i];
        if ((ct->cert_usage == kCertUsage_TrustCert) && (ct->hash == hash))
        {
            return ct;
        }
    }

    // If the code gets here, then no match was found
    return NULL;
}

/*********************************************************************//**
**
** Find_CertByHash
**
** Finds the certificate in our vector that matches the given hash
**
** \param   hash - hash of the cert we want to find
**
** \return  pointer to certificate, or INVALID if not found
**
**************************************************************************/
cert_t *Find_CertByHash(cert_hash_t hash)
{
    int i;
    cert_t *ct;

    // Iterate over all certificates in our trust store
    for (i=0; i<num_all_certs; i++)
    {
        // Exit if we've found a matching certificate
        ct = &all_certs[i];
        if (ct->hash == hash)
        {
            return ct;
        }
    }

    // If the code gets here, then no match was found
    return NULL;
}

/*********************************************************************//**
**
** AddCert
**
** Adds the specified certificate into a vector, along with its parsed details
**
** \param   cert - pointer to the certificate to parse.
**                 NOTE: If the certificate is a trust cert, then ownership of the cert passes to the vector
**                 NOTE: If the certificate is a client cert, then ownership of the cert stays with the caller
**                 NOTE: If the certificate is a system cert, then it will be freed by this function
** \param   cert_usage - type of certificate to add
** \param   role_instance - if the certificate is a trust cert, then this is the role that this cert permits to a broker cert,
**                          if the certificate is a trust cert, then this is set to INVALID
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int AddCert(X509 *cert, cert_usage_t cert_usage, int role_instance)
{
    int new_num_entries;
    cert_hash_t hash;
    cert_t *ct;
    int err;
    char path[MAX_DM_PATH];

    // Exit if unable to calculate the hash of the certificate
    err = CalcCertHash(cert, &hash);
    if (err != USP_ERR_OK)
    {
        X509_free(cert);
        return err;
    }

    // Exit if the certificate is already present in our vector
    // NOTE: This is likely to be the case if the directory contains symbolic links created by SSL's c_rehash utility
    ct = Find_CertByHash(hash);
    if (ct != NULL)
    {
        X509_free(cert);
        return USP_ERR_OK;
    }

    // Certificate is not in the vector, so add it
    // Increase the size of the vector, and initialise the new entry to default values
    new_num_entries = num_all_certs + 1;
    all_certs = USP_REALLOC(all_certs, new_num_entries*sizeof(cert_t));

    ct = &all_certs[ num_all_certs ];
    memset(ct, 0, sizeof(cert_t));

    // Extract the details of the specified certificate
    err = USP_ERR_OK;
    err |= ParseCert_Subject(cert, &ct->subject);
    err |= ParseCert_Issuer(cert, &ct->issuer);
    err |= ParseCert_LastModif(cert, &ct->last_modif);
    err |= ParseCert_SerialNumber(cert, &ct->serial_number);
    err |= ParseCert_NotBefore(cert, ct->not_before, sizeof(ct->not_before));
    err |= ParseCert_NotAfter(cert, ct->not_after, sizeof(ct->not_after));
    err |= ParseCert_SubjectAlt(cert, &ct->subject_alt);
    err |= ParseCert_SignatureAlg(cert, &ct->signature_algorithm);
    ct->hash = hash;

    // Exit if any error occurred when parsing
    if (err != USP_ERR_OK)
    {
        DestroyCert(ct);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Fill in the other member variables of the certificate structure
    num_all_certs = new_num_entries;
    ct->cert_usage = cert_usage;
    ct->instance = num_all_certs;

    switch(cert_usage)
    {
        case kCertUsage_TrustCert:
            // Save X509 trust certs into the vector
            num_trust_certs++;
            ct->cert = cert;
            ct->la_instance = num_trust_certs;
            break;

        case kCertUsage_ClientCert:
            // Client cert is not saved into the vector, but is not freed. Ownership stays with the caller. It is saved in agent_cert pointer.
            ct->cert = NULL;
            ct->la_instance = INVALID;
            break;

        case kCertUsage_SystemCert:
            // System certs are not saved into the vector, but instead freed (consumed) here
            X509_free(cert);
            ct->cert = NULL;
            ct->la_instance = INVALID;
            break;

        default:
            TERMINATE_BAD_CASE(cert_usage);
            break;
    }

    // Exit if unable to add the instance into the Device.Security.Certificate table
    USP_SNPRINTF(path, sizeof(path), "%s.%d", device_cert_root, ct->instance);
    err = DATA_MODEL_InformInstance(path);
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Trust store certs additionally populate the Device.LocalAgent.Certificate and Device.LocalAgent.ControllerTrust.Credential tables
    if (cert_usage == kCertUsage_TrustCert)
    {
        // Exit if unable to add the instance into the Device.LocalAgent.Certificate table
        USP_SNPRINTF(path, sizeof(path), "%s.%d", device_la_cert_root, ct->la_instance);
        err = DATA_MODEL_InformInstance(path);
        if (err != USP_ERR_OK)
        {
            return err;
        }

        // Exit if unable to add the certificate to the Device.LocalAgent.ControllerTrust.Credential table
        err = DEVICE_CTRUST_AddCertRole(ct->la_instance, role_instance, false);
        if (err != USP_ERR_OK)
        {
            return err;
        }
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** DestroyCert
**
** Frees all memory allocated to the specified certificate
**
** \param   ct - pointer to certificate to free memeber variables of
**
** \return  None
**
**************************************************************************/
void DestroyCert(cert_t *ct)
{
    if (ct->cert != NULL)
    {
        X509_free(ct->cert);
    }

    #define OPENSSL_SAFE_FREE(x)  if (x != NULL) { OPENSSL_free(x); }
    OPENSSL_SAFE_FREE(ct->subject);
    OPENSSL_SAFE_FREE(ct->issuer);
    OPENSSL_SAFE_FREE(ct->serial_number);

    USP_SAFE_FREE(ct->subject_alt);
    USP_SAFE_FREE(ct->signature_algorithm);

    // Since all member variables have been freed, zero out their pointers
    memset(ct, 0, sizeof(cert_t));
}

/*********************************************************************//**
**
** ParseCert_Subject
**
** Extracts the Subject field of the specified cert
**
** \param   cert - pointer to the certificate structure to extract the details of
** \param   p_subject - pointer to variable in which to return the pointer to a
**                      dynamically allocated string containing the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ParseCert_Subject(X509 *cert, char **p_subject)
{
    *p_subject = X509_NAME_oneline(X509_get_subject_name(cert), NULL, 0);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ParseCert_Issuer
**
** Extracts the Issuer field of the specified cert
**
** \param   cert - pointer to the certificate structure to extract the details of
** \param   p_issuer - pointer to variable in which to return the pointer to a
**                     dynamically allocated string containing the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ParseCert_Issuer(X509 *cert, char **p_issuer)
{
    *p_issuer = X509_NAME_oneline(X509_get_issuer_name(cert), NULL, 0);
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ParseCert_LastModif
**
** Extracts the LastModif field of the specified cert
**
** \param   cert - pointer to the certificate structure to extract the details of
** \param   last_modif - pointer to variable in which to return the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ParseCert_LastModif(X509 *cert, time_t *last_modif)
{
    *last_modif = 0;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ParseCert_SerialNumber
**
** Extracts the SerialNumber field of the specified cert
**
** \param   cert - pointer to the certificate structure to extract the details of
** \param   p_serial_number - pointer to variable in which to return a pointer to a dynamically allocated string
**                            The string may be freed by OPENSSL_free()
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ParseCert_SerialNumber(X509 *cert, char **p_serial_number)
{
    ASN1_INTEGER *asn1_number;
    BIGNUM *big_num;
    char *serial_number;
    int num_nibbles;
    int num_octets;
    char *p;
    char *q;
    int i;
    char octet_buf[128];    // This should be plenty. Serial numbers are only supposed to be upto 20 octets in length, which would be 60 characters in this buffer
    int leading_zeros;
    char *final_serial_number;

    // Exit if unable to get the serial number (in static ASN1 form)
    asn1_number = X509_get_serialNumber(cert);
    if (asn1_number == NULL)
    {
        USP_ERR_SetMessage("%s: X509_get_serialNumber() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to convert the ANS1 form to a (dynamically allocated) big number
    big_num = ASN1_INTEGER_to_BN(asn1_number, NULL);
    if (big_num == NULL)
    {
        USP_ERR_SetMessage("%s: ASN1_INTEGER_to_BN() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if unable to convert the big number to a hexadecimal string
    serial_number = BN_bn2hex(big_num);
    if (serial_number == NULL)
    {
        USP_ERR_SetMessage("%s: BN_bn2hec() failed", __FUNCTION__);
        BN_free(big_num);
        return USP_ERR_OK;
    }

    // Determine leading zero padding to make the serial number an even number of full octets, with 2 or more octets
    num_nibbles = strlen(serial_number);
    leading_zeros = 0;
    if (num_nibbles < 4)
    {
        // Ensure that the serial number contains at least 2 octets
        leading_zeros = 4 - num_nibbles;
    }
    else if ((num_nibbles %2) == 1)
    {
        // Ensure that it contains full octets rather than leading with a least significant nibble
        leading_zeros = 1;
    }

    // Add leading zeros to the serial number to make it at least 2 full octets in length, and an even number of nibbles
    memset(octet_buf, '0', 4);
    strncpy(&octet_buf[leading_zeros], serial_number, sizeof(octet_buf)-leading_zeros);

    // Free OpenSSL allocated data
    BN_free(big_num);
    OPENSSL_free(serial_number);

    // Allocate a buffer to store the final format Serial Number
    num_octets = (num_nibbles + leading_zeros) / 2;
    final_serial_number = OPENSSL_malloc(num_octets*3);  // Note: this includes trailing NULL terminator, as we have one less colon than the number of octets
    if (final_serial_number == NULL)
    {
        USP_ERR_SetMessage("%s: BN_bn2hec() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Copy the serial number into the return buffer, inserting colons between octets
    p = final_serial_number;
    q = octet_buf;
    for (i=0; i<num_octets; i++)
    {
        if (i != 0)
        {
            *p++ = ':';
        }
        *p++ = *q++;
        *p++ = *q++;
    }
    *p = '\0';

    // Serial number successfully extracted
    *p_serial_number = final_serial_number;
    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ParseCert_NotBefore
**
** Extracts the NotBefore time field of the specified cert
**
** \param   cert - pointer to the certificate structure to extract the details of
** \param   buf - pointer to buffer in which to return ISO8601 format string
** \param   len - length of buffer in which to return ISO8601 format string
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ParseCert_NotBefore(X509 *cert, char *buf, int len)
{
    ASN1_TIME *cert_time;

    // Exit if unable to get a not before time
    cert_time = X509_get_notBefore(cert);
    if (cert_time == NULL)
    {
        USP_ERR_SetMessage("%s: X509_get_notBefore() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    return Asn1Time_To_ISO8601(cert_time, buf, len);
}

/*********************************************************************//**
**
** ParseCert_NotAfter
**
** Extracts the NotBefore time field of the specified cert
**
** \param   cert - pointer to the certificate structure to extract the details of
** \param   buf - pointer to buffer in which to return ISO8601 format string
** \param   len - length of buffer in which to return ISO8601 format string
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ParseCert_NotAfter(X509 *cert, char *buf, int len)
{
    ASN1_TIME *cert_time;

    // Exit if unable to get a not after time
    cert_time = X509_get_notAfter(cert);
    if (cert_time == NULL)
    {
        USP_ERR_SetMessage("%s: X509_get_notAfter() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    return Asn1Time_To_ISO8601(cert_time, buf, len);
}

/*********************************************************************//**
**
** Asn1Time_To_ISO8601
**
** Converts a time specified in SSL ASN1 to am ISO8601 time
**
** \param   cert_time - pointer to SSL ANS1 time to convert
** \param   buf - pointer to buffer in which to return ISO8601 format string
** \param   buflen - length of buffer in which to return ISO8601 format string
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Asn1Time_To_ISO8601(ASN1_TIME *cert_time, char *buf, int buflen)
{
    char *s;
    struct tm tm;
    int i;
    int len;

    // Exit if the string does not match one of the lengths we are expecting
    s = (char *) cert_time->data;
    len = strlen(s);
    if ((len != 13) && (len != 15))
    {
        USP_ERR_SetMessage("%s: ASN1 string ('%s') does not match expected format (wrong length)", __FUNCTION__, s);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if the string is not terminated by 'Z'
    if (s[len-1] != 'Z')
    {
        USP_ERR_SetMessage("%s: ASN1 string ('%s') does not match expected format (not terminated in 'Z')", __FUNCTION__, s);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Exit if one of the digits is not numeric
    for (i=0; i < len-1; i++)
    {
        if ((s[i] < '0') || (s[i] > '9'))
        {
            USP_ERR_SetMessage("%s: ASN1 string ('%s') contains invalid digit ('%c')", __FUNCTION__, s, s[i]);
            return USP_ERR_INTERNAL_ERROR;
        }
    }

    // Calculate year since 1900, correcting for years after the millenium
    memset(&tm, 0, sizeof(tm));
    #define TO_DIGIT(x) (x - '0')
    if (len == 13)
    {
        // ASN1 string is of the format "YYMMDDHHMMSSZ"
        tm.tm_year = 10*TO_DIGIT(s[0]) + TO_DIGIT(s[1]);
        if (tm.tm_year < 70)
        {
            tm.tm_year += 100;
        }
        s += 2; // Skip to month characters
    }
    else
    {
        // ASN1 string is of the format "YYYYMMDDHHMMSSZ"
        tm.tm_year = 1000*TO_DIGIT(s[0]) + 100*TO_DIGIT(s[1]) + 10*TO_DIGIT(s[2]) + TO_DIGIT(s[3]) - 1900;
        if (tm.tm_year < 70)
        {
            USP_ERR_SetMessage("%s: ASN1 string ('%s') contains invalid year", __FUNCTION__, s);
            return USP_ERR_INTERNAL_ERROR;
        }

        s += 4; // Skip to month characters
    }

    // Fill in other fields
    tm.tm_mon  = 10*TO_DIGIT(s[0]) + TO_DIGIT(s[1]) - 1; // Month 0-11
    tm.tm_mday = 10*TO_DIGIT(s[2]) + TO_DIGIT(s[3]);     // Day of month 1-31
    tm.tm_hour = 10*TO_DIGIT(s[4]) + TO_DIGIT(s[5]);     // Hour 0-23
    tm.tm_min  = 10*TO_DIGIT(s[6]) + TO_DIGIT(s[7]);     // Minute 0-59
    tm.tm_sec  = 10*TO_DIGIT(s[8]) + TO_DIGIT(s[9]);     // Second 0-59

    // Exit if  unable to Form the ISO8601 string in the return buffer
    buf[0] = '\0';
    len = iso8601_strftime(buf, buflen, &tm);
    if (len == 0)
    {
        USP_ERR_SetMessage("%s: iso8601_strftime() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    return USP_ERR_OK;
}

/*********************************************************************//**
**
** ParseCert_SubjectAlt
**
** Extracts the SubjectAltName field of the specified cert
** NOTE: There may be more than one SubjectAlt field. This code extracts only the first
**
** \param   cert - pointer to the certificate structure to extract the details of
** \param   p_subject_alt - pointer to variable in which to return the pointer to a
**                          dynamically allocated string containing the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ParseCert_SubjectAlt(X509 *cert, char **p_subject_alt)
{
    GENERAL_NAMES *subj_alt_names = NULL;
    GENERAL_NAME *gname;
    int count;
    char *str = NULL;
    int str_len = 0;
    char *subject_alt;
    char buf[257];
    int err;

    // Exit if unable to get the list of subject alt names
    // NOTE: This is not an error, as subject alt name is an optional field
    subj_alt_names = X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    if (subj_alt_names == NULL)
    {
        *p_subject_alt = USP_STRDUP("");
        err = USP_ERR_OK;
        goto exit;
    }

    // Log a warning if the certificate contains more than one subject alt name
    count = sk_GENERAL_NAME_num(subj_alt_names);
    if (count > 1)
    {
        USP_LOG_Warning("%s: WARNING: Certificate has more than one SubjectAltName defined. Using only first.", __FUNCTION__);
    }

    // Exit if unable to get the first subject alt name
    gname = sk_GENERAL_NAME_value(subj_alt_names, 0);
    if (gname == NULL)
    {
        USP_ERR_SetMessage("%s: sk_GENERAL_NAME_value() failed", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Make code compatible with older versions of OpenSSL
    #if OPENSSL_VERSION_NUMBER < 0x10100000L  // SSL version 1.1.0
    #define ASN1_STRING_get0_data  ASN1_STRING_data
    #endif

    // Get a pointer to the internal string, depending on the type of SubjectAlt Name
    switch(gname->type)
    {
        case GEN_OTHERNAME:
            str = "Othername not supported";
            str_len = strlen(str);
            break;

        case GEN_EMAIL:
            str = (char *) ASN1_STRING_get0_data(gname->d.rfc822Name);
            str_len = ASN1_STRING_length(gname->d.rfc822Name); // This len does not include NULL terminator

            break;

        case GEN_DNS:
            str = (char *) ASN1_STRING_get0_data(gname->d.dNSName);
            str_len = ASN1_STRING_length(gname->d.dNSName); // This len does not include NULL terminator
            break;

        case GEN_X400:
            str = "x400Address not supported";
            str_len = strlen(str);
            break;

        case GEN_DIRNAME:
            buf[0] = '\0';
            str = X509_NAME_oneline(gname->d.directoryName, buf, sizeof(buf));
            buf[sizeof(buf)-1] = '\0';
            str_len = strlen(str);
            break;

        case GEN_EDIPARTY:
            str = "ediPartyName not supported";
            str_len = strlen(str);
            break;

        case GEN_URI:
            str = (char *) ASN1_STRING_get0_data(gname->d.uniformResourceIdentifier);
            str_len = ASN1_STRING_length(gname->d.uniformResourceIdentifier); // This len does not include NULL terminator
            break;

        case GEN_IPADD:
            str = "IPAddress not supported";
            str_len = strlen(str);
            break;

        case GEN_RID:
            str = "RegisteredID not supported";
            str_len = strlen(str);
            break;

        default:
            str = "Unknown SubjectAlt type";
            str_len = strlen(str);
            break;
    }

    // Exit if the extracted string is still NULL. Note this should not occur.
    if (str == NULL)
    {
        USP_ERR_SetMessage("%s: Unable to extract SubjectAltName for type=%d", __FUNCTION__, gname->type);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Dynamically allocate memory to hold the string, and copy it in
    subject_alt = USP_MALLOC(str_len+1);
    memcpy(subject_alt, str, str_len);
    subject_alt[str_len] = '\0';

    *p_subject_alt = subject_alt;
    err = USP_ERR_OK;

exit:
    if (subj_alt_names != NULL)
    {
        GENERAL_NAMES_free(subj_alt_names);
    }
    return err;
}

/*********************************************************************//**
**
** ParseCert_SignatureAlg
**
** Extracts the SignatureAlg field of the specified cert
**
** \param   cert - pointer to the certificate structure to extract the details of
** \param   p_sig_alg - pointer to variable in which to return the pointer to a
**                      dynamically allocated string containing the value
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int ParseCert_SignatureAlg(X509 *cert, char **p_sig_alg)
{
#if OPENSSL_VERSION_NUMBER >= 0x1010000FL // SSL version 1.1.0
    #define SSL_X509_ALGOR    const X509_ALGOR
#else
    #define SSL_X509_ALGOR    X509_ALGOR
#endif


#if OPENSSL_VERSION_NUMBER >= 0x1000200FL // SSL version 1.0.2
    SSL_X509_ALGOR *sig_alg_obj;
    int err;
    int result;
    BIO *bp = NULL;
    BUF_MEM *bm;
    char *buf;
    char *p;

    // Exit if unable to create an in-memory BIO
    bp = BIO_new( BIO_s_mem());
    if (bp == NULL)
    {
        USP_ERR_SetMessage("%s: BIO_new() failed", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Get the signature algorithm ASN1 object
    X509_get0_signature(NULL, (SSL_X509_ALGOR **)&sig_alg_obj, cert);

    // Print the signature algorithm to an in-memory BIO
    result = X509_signature_print(bp, (SSL_X509_ALGOR *)sig_alg_obj, NULL);
    if (result <= 0)
    {
        USP_ERR_SetMessage("%s: X509_signature_print() failed", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // X509_signature_print() should create a NON NULL terminated string of the form "  Signature Algorithm: XXX \n"
    // We want to extract the 'XXX' from this string

    // Ensure that the string is terminated by at least one '\n' (NOTE: should not be necessary, but added for extra safety)
    BIO_puts(bp, "\n");

    // Exit if unable to get a pointer to the string written by the BIO
    BIO_get_mem_ptr(bp, &bm);
    if (bm == NULL)
    {
        USP_ERR_SetMessage("%s: BIO_get_mem_ptr() failed", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }
    buf = bm->data;

    // Exit if unable to find the '\n' terminating the string
    p = strchr(buf, '\n');
    if (p == NULL)
    {
        USP_ERR_SetMessage("%s: strchr() failed to find '\n'", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // NULL terminate the string by replacing the first '\n' with '\0'
    *p = '\0';

    // Exit if unable to find the ':', just before the signature algorithm
    p = strchr(buf, ':');
    if (p == NULL)
    {
        USP_ERR_SetMessage("%s: strchr() failed to find ':'", __FUNCTION__);
        err = USP_ERR_INTERNAL_ERROR;
        goto exit;
    }

    // Skip ':' character
    p++;

    // Skip space characters before signature algorithm
    while ((*p == ' ') && (*p != '\0'))
    {
        p++;
    }

    // Copy the algorithmn name into a dynamically allcated buffer controlled by us
    *p_sig_alg = USP_STRDUP(p);
    err = USP_ERR_OK;

exit:
    // Free the BIO (if it was successfully created)
    if (bp != NULL)
    {
        BIO_free(bp);
    }

    return err;

#else
    // Following code is used by versions of OpenSSL prior to 1.0.2
    int alg;
    char *sig_alg;

    // Determine algorithm
    #if OPENSSL_VERSION_NUMBER < 0x10100000L  // SSL version 1.1.0
        alg = OBJ_obj2nid(cert->sig_alg->algorithm);
    #else
        const X509_ALGOR *algor;
        algor = X509_get0_tbs_sigalg(cert);
        alg = OBJ_obj2nid(algor->algorithm);
    #endif

    sig_alg = (char *) OBJ_nid2ln(alg);
    if (sig_alg == NULL)
    {
        sig_alg = "Unknown";
    }

    // Copy the algorithmn name into a dynamically allcated buffer controlled by us
    *p_sig_alg = USP_STRDUP(sig_alg);
    return USP_ERR_OK;
#endif
}

/*********************************************************************//**
**
** CalcCertHash
**
** Implements a 32 bit hash of the DER (binary) form of the specified certificate
** Implemented using the FNV1a algorithm
** NOTE: This function is called from the MTP thread, so it should only log errors (not call USP_ERR_SetMessage)
**
** \param   cert - pointer to the certificate structure to calculate a hash of
** \param   p_hash - pointer to variable in which to return the hash
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int CalcCertHash(X509 *cert, cert_hash_t *p_hash)
{
    #define OFFSET_BASIS (0x811C9DC5)
    #define FNV_PRIME (0x1000193)
    int i;
    cert_hash_t hash = OFFSET_BASIS;
    int len;
    unsigned char *buf = NULL;
    unsigned char *p;

    // Exit if unable to convert the X509 structure to DER form
    // NOTE: OpenSSL allocates memory for the DER form, and stores the pointer to this memory in 'buf'
    len = i2d_X509(cert, &buf);
    if ((len < 0) || (buf == NULL))
    {
        USP_LOG_Error("%s: i2d_X509() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Calculate a hash of the certificate
    p = buf;
    for (i=0; i < len; i++)
    {
        hash = hash * FNV_PRIME;
        hash = hash ^ (*p++);
    }

    // Free the memory allocated by OpenSSL to store the DER form of the cert
    OPENSSL_free(buf);

    *p_hash = hash;
    return USP_ERR_OK;
}



/*********************************************************************//**
**
** LoadTrustStore
**
** Called to load the trusted root certs provided by the get_trust_store vendor hook
**
** \param   None
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int LoadTrustStore(void)
{
    int i;
    int err;
    const trust_store_t *trusted_certs;
    const trust_store_t *tct;
    int num_trusted_certs;
    get_trust_store_cb_t get_trust_store_cb;
    X509 *cert;

    // Determine vendor hook function to call to get the trust store from in-memory array
    if (vendor_hook_callbacks.get_trust_store_cb != NULL)
    {
        get_trust_store_cb = vendor_hook_callbacks.get_trust_store_cb;
    }
    else
    {
        return USP_ERR_OK;
    }

    // Obtain the list of trusted certificates from the vendor
    trusted_certs = get_trust_store_cb(&num_trusted_certs);
    if (trusted_certs == NULL)
    {
        USP_ERR_SetMessage("%s: get_trust_store_cb() failed", __FUNCTION__);
        return USP_ERR_INTERNAL_ERROR;
    }

    // Iterate over all trusted certificates, adding them to our trust store
    for (i=0; i<num_trusted_certs; i++)
    {
        tct = &trusted_certs[i];
        cert = Cert_FromDER(tct->cert_data, tct->cert_len);
        if (cert != NULL)
        {
            // NOTE: Ownership of the X509 cert passes to AddCert(), so no need to free it in this function
            err = AddCert(cert, kCertUsage_TrustCert, tct->role_instance);
            if (err != USP_ERR_OK)
            {
                return err;
            }
        }
    }


    return USP_ERR_OK;
}

/*********************************************************************//**
**
** Cert_FromDER
**
** Returns an X509 object by reading it from DER format (binary) certificate array
**
** \param   cert_data - pointer to binary DER format certificate data
** \param   cert_len - number of bytes in the DER format certificate data
**
** \return  pointer to dynamically allocated X509 object, or NULL if failed to convert
**
**************************************************************************/
X509 *Cert_FromDER(const unsigned char *cert_data, int cert_len)
{
    const unsigned char *in;
    X509 *cert;

    // Exit if unable to convert the DER format byte array into an internal X509 format (DER to internal - d2i)
    in = cert_data;
    cert = d2i_X509(NULL, &in, cert_len);
    if (cert == NULL)
    {
        USP_ERR_SetMessage("%s: d2i_X509() failed. Error in trusted root cert array", __FUNCTION__);
        return NULL;
    }

    return cert;
}


/*********************************************************************//**
**
**  IsSystemTimeReliable
**
**  Function to determine whether the system (unix) time has been set on this CPE yet
**  It is expected that system time will only ever transition from not available to available (not back again)
**  This function is called from various places in USP Agent code and should not block for long periods of time
**  Typically, this function is expected to query the value of a global variable, rather than performing any more complex processing
**  If system time has not been set, then USP Agent will disregard system time when processing.
**
** \param   None
**
** \return  true if system time has been set, false otherwise
**
**************************************************************************/
bool IsSystemTimeReliable(void)
{
    return true;
}


/*********************************************************************//**
**
** Operate_GetFingerprint
**
** Sync Operation handler for the GetFingerprint() operation
**
** \param   req - pointer to structure identifying the operation in the data model
** \param   command_key - pointer to string containing the command key for this operation
** \param   input_args - vector containing input arguments and their values (unused)
** \param   output_args - vector to return output arguments in
**
** \return  USP_ERR_OK if successful
**
**************************************************************************/
int Operate_GetFingerprint(dm_req_t *req, char *command_key, kv_vector_t *input_args, kv_vector_t *output_args)
{
    int err;
    int result;
    cert_t *ct;
    fp_alg_t alg;
    int expected_size;
    unsigned char buf[MAX_DM_VALUE_LEN];
    unsigned len;
    const EVP_MD *type;

    // Exit if input argument could not be converted
    err = KV_VECTOR_GetEnum(input_args, "FingerprintAlgorithm", &alg, kFpAlg_None, fp_algs, NUM_ELEM(fp_algs));
    if (err != USP_ERR_OK)
    {
        return err;
    }

    // Exit if no fingerprint algorithm was specified
    if (alg == kFpAlg_None)
    {
        USP_ERR_SetMessage("%s: FingerprintAlgorithm input argument not specified", __FUNCTION__);
        return USP_ERR_INVALID_ARGUMENTS;
    }

    // Assign SSL object to perform fingerprinting
    // NOTE: The SSL object is static, so must not be freed
    switch(alg)
    {
        case kFpAlg_SHA1:
            type = EVP_sha1();
            expected_size = 160/8;
            break;

        case kFpAlg_SHA224:
            type = EVP_sha224();
            expected_size = 224/8;
            break;

        case kFpAlg_SHA256:
            type = EVP_sha256();
            expected_size = 256/8;
            break;

        case kFpAlg_SHA384:
            type = EVP_sha384();
            expected_size = 384/8;
            break;

        case kFpAlg_SHA512:
            type = EVP_sha512();
            expected_size = 512/8;
            break;

        default:
        case kFpAlg_None:
            TERMINATE_BAD_CASE(alg);
            return USP_ERR_INVALID_ARGUMENTS;   // NOTE: This statement is never reached due to exit() call in TERMINATE_BAD_CASE. It is here to stop the compiler complaining about type possible being uninitialised further on in the function
            break;
    }

    // Determine which cert
    ct = Find_LocalAgentCertByReq(req);
    USP_ASSERT(ct != NULL);             // Data model caller should have ensured instance number exists in data model
    USP_ASSERT(ct->cert != NULL);       // Trust store certs are always saved in the vector

    // Exit if unable to calculate digest
    result = X509_digest(ct->cert, type, buf, &len);
    if (result == 0)
    {
        USP_ERR_SetMessage("%s: X509_digest(%s) failed", __FUNCTION__, TEXT_UTILS_EnumToString(alg, fp_algs, NUM_ELEM(fp_algs)) );
        return USP_ERR_COMMAND_FAILURE;
    }

    // Exit if digest is of unexpected size
    if (len != expected_size)
    {
        USP_ERR_SetMessage("%s: X509_digest(%s) returned digest of unexpected length (got %d bytes, expected %d bytes)", __FUNCTION__, TEXT_UTILS_EnumToString(alg, fp_algs, NUM_ELEM(fp_algs)), len, expected_size);
        return USP_ERR_COMMAND_FAILURE;
    }

    // Convert the binary digest into hexadecimal characters
    KV_VECTOR_AddHexNumber(output_args, "Fingerprint", buf, len);

    return USP_ERR_OK;
}

#endif // REMOVE_DEVICE_SECURITY

