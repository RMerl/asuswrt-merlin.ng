/*
 * Copyright (C) 2012-2015 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
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

package org.strongswan.android.logic;

import android.util.Log;

import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Observable;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class TrustedCertificateManager extends Observable
{
	private static final String TAG = TrustedCertificateManager.class.getSimpleName();
	private final ReentrantReadWriteLock mLock = new ReentrantReadWriteLock();
	private Hashtable<String, X509Certificate> mCACerts = new Hashtable<String, X509Certificate>();
	private volatile boolean mReload;
	private boolean mLoaded;
	private final ArrayList<KeyStore> mKeyStores = new ArrayList<KeyStore>();

	public enum TrustedCertificateSource
	{
		SYSTEM("system:"),
		USER("user:"),
		LOCAL("local:");

		private final String mPrefix;

		private TrustedCertificateSource(String prefix)
		{
			mPrefix = prefix;
		}

		private String getPrefix()
		{
			return mPrefix;
		}
	}

	/**
	 * Private constructor to prevent instantiation from other classes.
	 */
	private TrustedCertificateManager()
	{
		for (String name : new String[]{"LocalCertificateStore", "AndroidCAStore"})
		{
			KeyStore store;
			try
			{
				store = KeyStore.getInstance(name);
				store.load(null, null);
				mKeyStores.add(store);
			}
			catch (Exception e)
			{
				Log.e(TAG, "Unable to load KeyStore: " + name);
				e.printStackTrace();
			}
		}
	}

	/**
	 * This is not instantiated until the first call to getInstance()
	 */
	private static class Singleton
	{
		public static final TrustedCertificateManager mInstance = new TrustedCertificateManager();
	}

	/**
	 * Get the single instance of the CA certificate manager.
	 *
	 * @return CA certificate manager
	 */
	public static TrustedCertificateManager getInstance()
	{
		return Singleton.mInstance;
	}

	/**
	 * Invalidates the current load state so that the next call to load()
	 * will force a reload of the cached CA certificates.
	 *
	 * Observers are notified when this method is called.
	 *
	 * @return reference to itself
	 */
	public TrustedCertificateManager reset()
	{
		Log.d(TAG, "Force reload of cached CA certificates on next load");
		this.mReload = true;
		this.setChanged();
		this.notifyObservers();
		return this;
	}

	/**
	 * Ensures that the certificates are loaded but does not force a reload.
	 * As this takes a while if the certificates are not loaded yet it should
	 * be called asynchronously.
	 *
	 * Observers are only notified when the certificates are initially loaded, not when reloaded.
	 *
	 * @return reference to itself
	 */
	public TrustedCertificateManager load()
	{
		Log.d(TAG, "Ensure cached CA certificates are loaded");
		this.mLock.writeLock().lock();
		if (!this.mLoaded || this.mReload)
		{
			this.mReload = false;
			loadCertificates();
		}
		this.mLock.writeLock().unlock();
		return this;
	}

	/**
	 * Opens the CA certificate KeyStore and loads the cached certificates.
	 * The lock must be locked when calling this method.
	 */
	private void loadCertificates()
	{
		Log.d(TAG, "Load cached CA certificates");
		Hashtable<String, X509Certificate> certs = new Hashtable<String, X509Certificate>();
		for (KeyStore store : this.mKeyStores)
		{
			fetchCertificates(certs, store);
		}
		this.mCACerts = certs;
		if (!this.mLoaded)
		{
			this.setChanged();
			this.notifyObservers();
			this.mLoaded = true;
		}
		Log.d(TAG, "Cached CA certificates loaded");
	}

	/**
	 * Load all X.509 certificates from the given KeyStore.
	 *
	 * @param certs Hashtable to store certificates in
	 * @param store KeyStore to load certificates from
	 */
	private void fetchCertificates(Hashtable<String, X509Certificate> certs, KeyStore store)
	{
		try
		{
			Enumeration<String> aliases = store.aliases();
			while (aliases.hasMoreElements())
			{
				String alias = aliases.nextElement();
				Certificate cert;
				cert = store.getCertificate(alias);
				if (cert != null && cert instanceof X509Certificate)
				{
					certs.put(alias, (X509Certificate)cert);
				}
			}
		}
		catch (KeyStoreException ex)
		{
			ex.printStackTrace();
		}
	}

	/**
	 * Retrieve the CA certificate with the given alias.
	 *
	 * @param alias alias of the certificate to get
	 * @return the certificate, null if not found
	 */
	public X509Certificate getCACertificateFromAlias(String alias)
	{
		X509Certificate certificate = null;

		if (this.mLock.readLock().tryLock())
		{
			certificate = this.mCACerts.get(alias);
			this.mLock.readLock().unlock();
		}
		else
		{	/* if we cannot get the lock load it directly from the KeyStore,
			 * should be fast for a single certificate */
			for (KeyStore store : this.mKeyStores)
			{
				try
				{
					Certificate cert = store.getCertificate(alias);
					if (cert != null && cert instanceof X509Certificate)
					{
						certificate = (X509Certificate)cert;
						break;
					}
				}
				catch (KeyStoreException e)
				{
					e.printStackTrace();
				}
			}
		}
		return certificate;
	}

	/**
	 * Get all CA certificates (from all keystores).
	 *
	 * @return Hashtable mapping aliases to certificates
	 */
	@SuppressWarnings("unchecked")
	public Hashtable<String, X509Certificate> getAllCACertificates()
	{
		Hashtable<String, X509Certificate> certs;
		this.mLock.readLock().lock();
		certs = (Hashtable<String, X509Certificate>)this.mCACerts.clone();
		this.mLock.readLock().unlock();
		return certs;
	}

	/**
	 * Get all certificates from the given source.
	 *
	 * @param source type to filter certificates
	 * @return Hashtable mapping aliases to certificates
	 */
	public Hashtable<String, X509Certificate> getCACertificates(TrustedCertificateSource source)
	{
		Hashtable<String, X509Certificate> certs = new Hashtable<String, X509Certificate>();
		this.mLock.readLock().lock();
		for (String alias : this.mCACerts.keySet())
		{
			if (alias.startsWith(source.getPrefix()))
			{
				certs.put(alias, this.mCACerts.get(alias));
			}
		}
		this.mLock.readLock().unlock();
		return certs;
	}
}
