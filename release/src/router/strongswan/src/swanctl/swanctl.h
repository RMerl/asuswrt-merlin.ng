/*
 * Copyright (C) 2016-2018 Tobias Brunner
 * Copyright (C) 2015 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

/**
 * @defgroup swanctl swanctl
 * @{
 */

#ifndef SWANCTL_H_
#define SWANCTL_H_

#include <settings/settings.h>

/**
 * Base directory for credentials and config
 */
char *swanctl_dir;

/**
 * Configuration file for connections, etc.
 */
#define SWANCTL_CONF "swanctl.conf"

/**
 * Directory for X.509 end entity certs
 */
#define SWANCTL_X509DIR "x509"

/**
 * Directory for X.509 CA certs
 */
#define SWANCTL_X509CADIR "x509ca"

/**
 * Directory for X.509 Attribute Authority certs
 */
#define SWANCTL_X509AADIR "x509aa"

/**
 * Directory for X.509 OCSP Signer certs
 */
#define SWANCTL_X509OCSPDIR "x509ocsp"

/**
 * Directory for X.509 CRLs
 */
#define SWANCTL_X509CRLDIR "x509crl"

/**
 * Directory for X.509 Attribute certificates
 */
#define SWANCTL_X509ACDIR "x509ac"

/**
 * Directory for raw public keys
 */
#define SWANCTL_PUBKEYDIR "pubkey"

/**
 * Directory for private keys
 */
#define SWANCTL_PRIVATEDIR "private"

/**
 * Directory for RSA private keys
 */
#define SWANCTL_RSADIR "rsa"

/**
 * Directory for ECDSA private keys
 */
#define SWANCTL_ECDSADIR "ecdsa"

/**
 * Directory for BLISS private keys
 */
#define SWANCTL_BLISSDIR "bliss"

/**
 * Directory for PKCS#8 encoded private keys
 */
#define SWANCTL_PKCS8DIR "pkcs8"

/**
 * Directory for PKCS#12 containers
 */
#define SWANCTL_PKCS12DIR "pkcs12"

/**
 * Load swanctl.conf, optionally from a custom path. Sets the base dir relative
 * to that file.
 *
 * @param file		optional custom path to swanctl.conf, NULL to use default
 * @return			settings, or NULL if loading failed
 */
settings_t *load_swanctl_conf(char *file);

#endif /** SWANCTL_H_ @}*/
