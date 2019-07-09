/*
 * Copyright (C) 2012 Tobias Brunner
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

package org.strongswan.android.data;

import java.io.File;
import java.io.FileNotFoundException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.concurrent.ConcurrentHashMap;

import org.strongswan.android.logic.CharonVpnService;

import android.content.ContentProvider;
import android.content.ContentValues;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.os.SystemClock;
import android.provider.OpenableColumns;

public class LogContentProvider extends ContentProvider
{
	private static final String AUTHORITY = "org.strongswan.android.content.log";
	/* an Uri is valid for 30 minutes */
	private static final long URI_VALIDITY = 30 * 60 * 1000;
	private static ConcurrentHashMap<Uri, Long> mUris = new ConcurrentHashMap<Uri, Long>();
	private File mLogFile;

	public LogContentProvider()
	{
	}

	@Override
	public boolean onCreate()
	{
		mLogFile = new File(getContext().getFilesDir(), CharonVpnService.LOG_FILE);
		return true;
	}

	/**
	 * The log file can only be accessed by Uris created with this method
	 * @return null if failed to create the Uri
	 */
	public static Uri createContentUri()
	{
		SecureRandom random;
		try
		{
			random = SecureRandom.getInstance("SHA1PRNG");
		}
		catch (NoSuchAlgorithmException e)
		{
			return null;
		}
		Uri uri = Uri.parse("content://" + AUTHORITY + "/" + random.nextLong());
		mUris.put(uri, SystemClock.uptimeMillis());
		return uri;
	}

	@Override
	public String getType(Uri uri)
	{
		/* MIME type for our log file */
		return "text/plain";
	}

	@Override
	public Cursor query(Uri uri, String[] projection, String selection,
						String[] selectionArgs, String sortOrder)
	{
		/* this is called by apps to find out the name and size of the file.
		 * since we only provide a single file this is simple to implement */
		if (projection == null || projection.length < 1)
		{
			return null;
		}
		Long timestamp = mUris.get(uri);
		if (timestamp == null)
		{	/* don't check the validity as this information is not really private */
			return null;
		}
		MatrixCursor cursor = new MatrixCursor(projection, 1);
		if (OpenableColumns.DISPLAY_NAME.equals(cursor.getColumnName(0)))
		{
			cursor.newRow().add(CharonVpnService.LOG_FILE);
		}
		else if (OpenableColumns.SIZE.equals(cursor.getColumnName(0)))
		{
			cursor.newRow().add(mLogFile.length());
		}
		else
		{
			return null;
		}
		return cursor;
	}

	@Override
	public ParcelFileDescriptor openFile(Uri uri, String mode) throws FileNotFoundException
	{
		Long timestamp = mUris.get(uri);
		if (timestamp != null)
		{
			long elapsed = SystemClock.uptimeMillis() - timestamp;
			if (elapsed > 0 && elapsed < URI_VALIDITY)
			{	/* we fail if clock wrapped, should happen rarely though */
				return ParcelFileDescriptor.open(mLogFile, ParcelFileDescriptor.MODE_CREATE | ParcelFileDescriptor.MODE_READ_ONLY);
			}
			mUris.remove(uri);
		}
		return super.openFile(uri, mode);
	}

	@Override
	public Uri insert(Uri uri, ContentValues values)
	{
		/* not supported */
		return null;
	}

	@Override
	public int delete(Uri uri, String selection, String[] selectionArgs)
	{
		/* not supported */
		return 0;
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection,
					  String[] selectionArgs)
	{
		/* not supported */
		return 0;
	}
}
