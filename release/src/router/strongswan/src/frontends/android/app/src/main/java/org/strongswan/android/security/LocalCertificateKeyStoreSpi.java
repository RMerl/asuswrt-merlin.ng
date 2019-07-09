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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.Key;
import java.security.KeyStoreException;
import java.security.KeyStoreSpi;
import java.security.NoSuchAlgorithmException;
import java.security.UnrecoverableKeyException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.Collections;
import java.util.Date;
import java.util.Enumeration;

public class LocalCertificateKeyStoreSpi extends KeyStoreSpi
{
	private final LocalCertificateStore mStore = new LocalCertificateStore();

	@Override
	public Key engineGetKey(String alias, char[] password) throws NoSuchAlgorithmException, UnrecoverableKeyException
	{
		return null;
	}

	@Override
	public Certificate[] engineGetCertificateChain(String alias)
	{
		return null;
	}

	@Override
	public Certificate engineGetCertificate(String alias)
	{
		return mStore.getCertificate(alias);
	}

	@Override
	public Date engineGetCreationDate(String alias)
	{
		return mStore.getCreationDate(alias);
	}

	@Override
	public void engineSetKeyEntry(String alias, Key key, char[] password, Certificate[] chain) throws KeyStoreException
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public void engineSetKeyEntry(String alias, byte[] key, Certificate[] chain) throws KeyStoreException
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public void engineSetCertificateEntry(String alias, Certificate cert) throws KeyStoreException
	{
		/* we ignore the given alias as the store calculates it on its own,
		 * duplicates are replaced */
		if (!mStore.addCertificate(cert))
		{
			throw new KeyStoreException();
		}
	}

	@Override
	public void engineDeleteEntry(String alias) throws KeyStoreException
	{
		mStore.deleteCertificate(alias);
	}

	@Override
	public Enumeration<String> engineAliases()
	{
		return Collections.enumeration(mStore.aliases());
	}

	@Override
	public boolean engineContainsAlias(String alias)
	{
		return mStore.containsAlias(alias);
	}

	@Override
	public int engineSize()
	{
		return mStore.aliases().size();
	}

	@Override
	public boolean engineIsKeyEntry(String alias)
	{
		return false;
	}

	@Override
	public boolean engineIsCertificateEntry(String alias)
	{
		return engineContainsAlias(alias);
	}

	@Override
	public String engineGetCertificateAlias(Certificate cert)
	{
		return mStore.getCertificateAlias(cert);
	}

	@Override
	public void engineStore(OutputStream stream, char[] password) throws IOException, NoSuchAlgorithmException, CertificateException
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public void engineLoad(InputStream stream, char[] password) throws IOException, NoSuchAlgorithmException, CertificateException
	{
		if (stream != null)
		{
			throw new UnsupportedOperationException();
		}
	}
}
