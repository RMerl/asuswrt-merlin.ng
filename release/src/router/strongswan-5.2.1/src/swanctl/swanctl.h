/*
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

/**
 * Configuration file for connections, etc.
 */
#define SWANCTL_CONF SWANCTLDIR "/swanctl.conf"

/**
 * Directory for X.509 end entity certs
 */
#define SWANCTL_X509DIR SWANCTLDIR "/x509"

/**
 * Directory for X.509 CA certs
 */
#define SWANCTL_X509CADIR SWANCTLDIR "/x509ca"

/**
 * Directory for X.509 Attribute Authority certs
 */
#define SWANCTL_X509AADIR SWANCTLDIR "/x509aa"

/**
 * Directory for X.509 CRLs
 */
#define SWANCTL_X509CRLDIR SWANCTLDIR "/x509crl"

/**
 * Directory for X.509 Attribute certificates
 */
#define SWANCTL_X509ACDIR SWANCTLDIR "/x509ac"

/**
 * Directory for RSA private keys
 */
#define SWANCTL_RSADIR SWANCTLDIR "/rsa"

/**
 * Directory for ECDSA private keys
 */
#define SWANCTL_ECDSADIR SWANCTLDIR "/ecdsa"

/**
 * Directory for PKCS#8 encoded private keys
 */
#define SWANCTL_PKCS8DIR SWANCTLDIR "/pkcs8"

#endif /** SWANCTL_H_ @}*/
