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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Date;
import java.util.regex.Pattern;

import org.strongswan.android.logic.StrongSwanApplication;
import org.strongswan.android.utils.Utils;

import android.content.Context;

public class LocalCertificateStore
{
	private static final String FILE_PREFIX = "certificate-";
	private static final String ALIAS_PREFIX = "local:";
	private static final Pattern ALIAS_PATTERN = Pattern.compile("^" + ALIAS_PREFIX + "[0-9a-f]{40}$");

	/**
	 * Add the given certificate to the store
	 * @param cert the certificate to add
	 * @return true if successful
	 */
	public boolean addCertificate(Certificate cert)
	{
		if (!(cert instanceof X509Certificate))
		{	/* only accept X.509 certificates */
			return false;
		}
		String keyid = getKeyId(cert);
		if (keyid == null)
		{
			return false;
		}
		FileOutputStream out;
		try
		{
			/* we replace any existing file with the same alias */
			out = StrongSwanApplication.getContext().openFileOutput(FILE_PREFIX + keyid, Context.MODE_PRIVATE);
			try
			{
				out.write(cert.getEncoded());
				return true;
			}
			catch (CertificateEncodingException e)
			{
				e.printStackTrace();
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}
			finally
			{
				try
				{
					out.close();
				}
				catch (IOException e)
				{
					e.printStackTrace();
				}
			}
		}
		catch (FileNotFoundException e)
		{
			e.printStackTrace();
		}
		return false;
	}

	/**
	 * Delete the certificate with the given alias
	 * @param alias a certificate's alias
	 */
	public void deleteCertificate(String alias)
	{
		if (ALIAS_PATTERN.matcher(alias).matches())
		{
			alias = alias.substring(ALIAS_PREFIX.length());
			StrongSwanApplication.getContext().deleteFile(FILE_PREFIX + alias);
		}
	}

	/**
	 * Retrieve the certificate with the given alias
	 * @param alias a certificate's alias
	 * @return certificate object or null
	 */
	public X509Certificate getCertificate(String alias)
	{
		if (!ALIAS_PATTERN.matcher(alias).matches())
		{
			return null;
		}
		alias = alias.substring(ALIAS_PREFIX.length());
		try
		{
			FileInputStream in = StrongSwanApplication.getContext().openFileInput(FILE_PREFIX + alias);
			try
			{
				CertificateFactory factory = CertificateFactory.getInstance("X.509");
				X509Certificate certificate = (X509Certificate)factory.generateCertificate(in);
				return certificate;
			}
			catch (CertificateException e)
			{
				e.printStackTrace();
			}
			finally
			{
				try
				{
					in.close();
				}
				catch (IOException e)
				{
					e.printStackTrace();
				}
			}
		}
		catch (FileNotFoundException e)
		{
			e.printStackTrace();
		}
		return null;
	}

	/**
	 * Returns the creation date of the certificate with the given alias
	 * @param alias certificate alias
	 * @return creation date or null if not found
	 */
	public Date getCreationDate(String alias)
	{
		if (!ALIAS_PATTERN.matcher(alias).matches())
		{
			return null;
		}
		alias = alias.substring(ALIAS_PREFIX.length());
		File file = StrongSwanApplication.getContext().getFileStreamPath(FILE_PREFIX + alias);
		return file.exists() ? new Date(file.lastModified()) : null;
	}

	/**
	 * Returns a list of all known certificate aliases
	 * @return list of aliases
	 */
	public ArrayList<String> aliases()
	{
		ArrayList<String> list = new ArrayList<String>();
		for (String file : StrongSwanApplication.getContext().fileList())
		{
			if (file.startsWith(FILE_PREFIX))
			{
				list.add(ALIAS_PREFIX + file.substring(FILE_PREFIX.length()));
			}
		}
		return list;
	}

	/**
	 * Check if the store contains a certificate with the given alias
	 * @param alias certificate alias
	 * @return true if the store contains the certificate
	 */
	public boolean containsAlias(String alias)
	{
		return getCreationDate(alias) != null;
	}

	/**
	 * Returns a certificate alias based on a SHA-1 hash of the public key.
	 *
	 * @param cert certificate to get an alias for
	 * @return hex encoded alias, or null if failed
	 */
	public String getCertificateAlias(Certificate cert)
	{
		String keyid = getKeyId(cert);
		return keyid != null ? ALIAS_PREFIX + keyid : null;
	}

	/**
	 * Calculates the SHA-1 hash of the public key of the given certificate.
	 * @param cert certificate to get the key ID from
	 * @return hex encoded SHA-1 hash of the public key or null if failed
	 */
	private String getKeyId(Certificate cert)
	{
		MessageDigest md;
		try
		{
			md = java.security.MessageDigest.getInstance("SHA1");
			byte[] hash = md.digest(cert.getPublicKey().getEncoded());
			return Utils.bytesToHex(hash);
		}
		catch (NoSuchAlgorithmException e)
		{
			e.printStackTrace();
		}
		return null;
	}
}
