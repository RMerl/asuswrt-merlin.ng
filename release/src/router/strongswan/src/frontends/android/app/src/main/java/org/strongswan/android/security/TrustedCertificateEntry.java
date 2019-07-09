/*
 * Copyright (C) 2012-2016 Tobias Brunner
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

import android.net.http.SslCertificate;

import java.security.cert.CertificateParsingException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

public class TrustedCertificateEntry implements Comparable<TrustedCertificateEntry>
{
	private final X509Certificate mCert;
	private final String mAlias;
	private String mSubjectPrimary;
	private String mSubjectSecondary = "";
	private String mString;

	/**
	 * Create an entry for certificate lists.
	 *
	 * @param alias alias of the certificate (as used in the KeyStore)
	 * @param cert certificate associated with that alias
	 */
	public TrustedCertificateEntry(String alias, X509Certificate cert)
	{
		mCert = cert;
		mAlias = alias;

		try
		{
			SslCertificate ssl = new SslCertificate(mCert);
			String o = ssl.getIssuedTo().getOName();
			String ou = ssl.getIssuedTo().getUName();
			String cn = ssl.getIssuedTo().getCName();
			if (!o.isEmpty())
			{
				mSubjectPrimary = o;
				if (!cn.isEmpty())
				{
					mSubjectSecondary = cn;
				}
				else if (!ou.isEmpty())
				{
					mSubjectSecondary = ou;
				}
			}
			else if (!cn.isEmpty())
			{
				mSubjectPrimary = cn;
			}
			else
			{
				mSubjectPrimary = ssl.getIssuedTo().getDName();
			}
		}
		catch (NullPointerException ex)
		{
			/* this has been seen in Play Console for certificates for which notBefore apparently
			 * can't be parsed (which SslCertificate() does) */
			mSubjectPrimary = cert.getSubjectDN().getName();
		}
	}

	/**
	 * The main subject of this certificate (O, CN or the complete DN, whatever
	 * is found first).
	 *
	 * @return the main subject
	 */
	public String getSubjectPrimary()
	{
		return mSubjectPrimary;
	}

	/**
	 * Get the secondary subject of this certificate (either CN or OU if primary
	 * subject is O, empty otherwise)
	 *
	 * @return the secondary subject
	 */
	public String getSubjectSecondary()
	{
		return mSubjectSecondary;
	}

	/**
	 * Get a sorted list of all rfc822Name, dnSName and iPAddress subjectAltNames
	 *
	 * @return sorted list of selected SANs
	 */
	public List<String> getSubjectAltNames()
	{
		List<String> list = new ArrayList<>();
		try
		{
			Collection<List<?>> sans = mCert.getSubjectAlternativeNames();
			if (sans != null)
			{
				for (List<?> san : sans)
				{
					switch ((Integer)san.get(0))
					{
						case 1: /* rfc822Name */
						case 2: /* dnSName */
						case 7: /* iPAddress */
							list.add((String)san.get(1));
							break;
					}
				}
			}
			Collections.sort(list);
		}
		catch(CertificateParsingException ex)
		{
			ex.printStackTrace();
		}
		return list;
	}

	/**
	 * The alias associated with this certificate.
	 *
	 * @return KeyStore alias of this certificate
	 */
	public String getAlias()
	{
		return mAlias;
	}

	/**
	 * The certificate.
	 *
	 * @return certificate
	 */
	public X509Certificate getCertificate()
	{
		return mCert;
	}

	@Override
	public String toString()
	{	/* combination of both subject lines, used for filtering lists */
		if (mString == null)
		{
			mString = mSubjectPrimary;
			if (!mSubjectSecondary.isEmpty())
			{
				mString += ", " + mSubjectSecondary;
			}
		}
		return mString;
	}

	@Override
	public int compareTo(TrustedCertificateEntry another)
	{
		int diff = mSubjectPrimary.compareToIgnoreCase(another.mSubjectPrimary);
		if (diff == 0)
		{
			diff = mSubjectSecondary.compareToIgnoreCase(another.mSubjectSecondary);
		}
		return diff;
	}
}
