/*
 * Copyright (C) 2014 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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

package org.strongswan.android.security;

import java.security.Provider;

public class LocalCertificateKeyStoreProvider extends Provider
{
	private static final long serialVersionUID = 3515038332469843219L;

	public LocalCertificateKeyStoreProvider()
	{
		super("LocalCertificateKeyStoreProvider", 1.0, "KeyStore provider for local certificates");
		put("KeyStore.LocalCertificateStore", LocalCertificateKeyStoreSpi.class.getName());
	}
}
