/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "keychain_creds.h"

#include <utils/debug.h>
#include <credentials/sets/mem_cred.h>
#include <processing/jobs/callback_job.h>

#include <Security/Security.h>

/**
 * System Roots keychain
 */
#define SYSTEM_ROOTS "/System/Library/Keychains/SystemRootCertificates.keychain"

/**
 * System keychain
 */
#define SYSTEM "/Library/Keychains/System.keychain"

typedef struct private_keychain_creds_t private_keychain_creds_t;

/**
 * Private data of an keychain_creds_t object.
 */
struct private_keychain_creds_t {

	/**
	 * Public keychain_creds_t interface.
	 */
	keychain_creds_t public;

	/**
	 * Active in-memory credential set
	 */
	mem_cred_t *set;

	/**
	 * System roots credential set
	 */
	mem_cred_t *roots;

	/**
	 * Run loop of event monitoring thread
	 */
	CFRunLoopRef loop;
};

/**
 * Load a credential sets with certificates from a keychain path
 */
static mem_cred_t* load_certs(private_keychain_creds_t *this, char *path)
{
	SecKeychainRef keychain;
	SecKeychainSearchRef search;
	SecKeychainItemRef item;
	mem_cred_t *set;
	OSStatus status;
	int loaded = 0;

	set = mem_cred_create();

	DBG2(DBG_CFG, "loading certificates from %s:", path);
	status = SecKeychainOpen(path, &keychain);
	if (status == errSecSuccess)
	{
		status = SecKeychainSearchCreateFromAttributes(keychain,
									kSecCertificateItemClass, NULL, &search);
		if (status == errSecSuccess)
		{
			while (SecKeychainSearchCopyNext(search, &item) == errSecSuccess)
			{
				certificate_t *cert;
				UInt32 len;
				void *data;

				if (SecKeychainItemCopyAttributesAndData(item, NULL, NULL, NULL,
												&len, &data) == errSecSuccess)
				{
					cert = lib->creds->create(lib->creds,
								CRED_CERTIFICATE, CERT_X509,
								BUILD_BLOB_ASN1_DER, chunk_create(data, len),
								BUILD_END);
					if (cert)
					{
						DBG2(DBG_CFG, "  loaded '%Y'", cert->get_subject(cert));
						set->add_cert(set, TRUE, cert);
						loaded++;
					}
					SecKeychainItemFreeAttributesAndData(NULL, data);
				}
				CFRelease(item);
			}
			CFRelease(search);
		}
		CFRelease(keychain);
	}
	DBG1(DBG_CFG, "loaded %d certificates from %s", loaded, path);
	return set;
}

/**
 * Callback function reloading keychain on changes
 */
static OSStatus keychain_cb(SecKeychainEvent keychainEvent,
							SecKeychainCallbackInfo *info,
							private_keychain_creds_t *this)
{
	mem_cred_t *new;

	DBG1(DBG_CFG, "received keychain event, reloading credentials");

	/* register new before removing old */
	new = load_certs(this, SYSTEM);
	lib->credmgr->add_set(lib->credmgr, &new->set);
	lib->credmgr->remove_set(lib->credmgr, &this->set->set);

	lib->credmgr->flush_cache(lib->credmgr, CERT_X509);

	this->set->destroy(this->set);
	this->set = new;

	return errSecSuccess;
}

/**
 * Wait for changes in the keychain and handle them
 */
static job_requeue_t monitor_changes(private_keychain_creds_t *this)
{
	if (SecKeychainAddCallback((SecKeychainCallback)keychain_cb,
						kSecAddEventMask | kSecDeleteEventMask |
						kSecUpdateEventMask | kSecTrustSettingsChangedEventMask,
						this) == errSecSuccess)
	{
		this->loop = CFRunLoopGetCurrent();

		/* does not return until cancelled */
		CFRunLoopRun();

		this->loop = NULL;
		SecKeychainRemoveCallback((SecKeychainCallback)keychain_cb);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * Cancel the monitoring thread in its RunLoop
 */
static bool cancel_monitor(private_keychain_creds_t *this)
{
	if (this->loop)
	{
		CFRunLoopStop(this->loop);
	}
	return TRUE;
}

METHOD(keychain_creds_t, destroy, void,
	private_keychain_creds_t *this)
{
	lib->credmgr->remove_set(lib->credmgr, &this->set->set);
	lib->credmgr->remove_set(lib->credmgr, &this->roots->set);
	this->set->destroy(this->set);
	this->roots->destroy(this->roots);
	free(this);
}

/**
 * See header
 */
keychain_creds_t *keychain_creds_create()
{
	private_keychain_creds_t *this;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
	);

	this->roots = load_certs(this, SYSTEM_ROOTS);
	this->set = load_certs(this, SYSTEM);

	lib->credmgr->add_set(lib->credmgr, &this->roots->set);
	lib->credmgr->add_set(lib->credmgr, &this->set->set);

	lib->processor->queue_job(lib->processor,
			(job_t*)callback_job_create_with_prio((void*)monitor_changes,
					this, NULL, (void*)cancel_monitor, JOB_PRIO_CRITICAL));

	return &this->public;
}
