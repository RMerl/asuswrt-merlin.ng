/*
 * Copyright (C) 2012-2018 Tobias Brunner
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

package org.strongswan.android.data;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.SQLException;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class VpnProfileDataSource
{
	private static final String TAG = VpnProfileDataSource.class.getSimpleName();
	public static final String KEY_ID = "_id";
	public static final String KEY_UUID = "_uuid";
	public static final String KEY_NAME = "name";
	public static final String KEY_GATEWAY = "gateway";
	public static final String KEY_VPN_TYPE = "vpn_type";
	public static final String KEY_USERNAME = "username";
	public static final String KEY_PASSWORD = "password";
	public static final String KEY_CERTIFICATE = "certificate";
	public static final String KEY_USER_CERTIFICATE = "user_certificate";
	public static final String KEY_MTU = "mtu";
	public static final String KEY_PORT = "port";
	public static final String KEY_SPLIT_TUNNELING = "split_tunneling";
	public static final String KEY_LOCAL_ID = "local_id";
	public static final String KEY_REMOTE_ID = "remote_id";
	public static final String KEY_EXCLUDED_SUBNETS = "excluded_subnets";
	public static final String KEY_INCLUDED_SUBNETS = "included_subnets";
	public static final String KEY_SELECTED_APPS = "selected_apps";
	public static final String KEY_SELECTED_APPS_LIST = "selected_apps_list";
	public static final String KEY_NAT_KEEPALIVE = "nat_keepalive";
	public static final String KEY_FLAGS = "flags";
	public static final String KEY_IKE_PROPOSAL = "ike_proposal";
	public static final String KEY_ESP_PROPOSAL = "esp_proposal";

	private DatabaseHelper mDbHelper;
	private SQLiteDatabase mDatabase;
	private final Context mContext;

	private static final String DATABASE_NAME = "strongswan.db";
	private static final String TABLE_VPNPROFILE = "vpnprofile";

	private static final int DATABASE_VERSION = 16;

	public static final DbColumn[] COLUMNS = new DbColumn[] {
								new DbColumn(KEY_ID, "INTEGER PRIMARY KEY AUTOINCREMENT", 1),
								new DbColumn(KEY_UUID, "TEXT UNIQUE", 9),
								new DbColumn(KEY_NAME, "TEXT NOT NULL", 1),
								new DbColumn(KEY_GATEWAY, "TEXT NOT NULL", 1),
								new DbColumn(KEY_VPN_TYPE, "TEXT NOT NULL", 3),
								new DbColumn(KEY_USERNAME, "TEXT", 1),
								new DbColumn(KEY_PASSWORD, "TEXT", 1),
								new DbColumn(KEY_CERTIFICATE, "TEXT", 1),
								new DbColumn(KEY_USER_CERTIFICATE, "TEXT", 2),
								new DbColumn(KEY_MTU, "INTEGER", 5),
								new DbColumn(KEY_PORT, "INTEGER", 5),
								new DbColumn(KEY_SPLIT_TUNNELING, "INTEGER", 7),
								new DbColumn(KEY_LOCAL_ID, "TEXT", 8),
								new DbColumn(KEY_REMOTE_ID, "TEXT", 8),
								new DbColumn(KEY_EXCLUDED_SUBNETS, "TEXT", 10),
								new DbColumn(KEY_INCLUDED_SUBNETS, "TEXT", 11),
								new DbColumn(KEY_SELECTED_APPS, "INTEGER", 12),
								new DbColumn(KEY_SELECTED_APPS_LIST, "TEXT", 12),
								new DbColumn(KEY_NAT_KEEPALIVE, "INTEGER", 13),
								new DbColumn(KEY_FLAGS, "INTEGER", 14),
								new DbColumn(KEY_IKE_PROPOSAL, "TEXT", 15),
								new DbColumn(KEY_ESP_PROPOSAL, "TEXT", 15),
							};

	private static final String[] ALL_COLUMNS = getColumns(DATABASE_VERSION);

	private static String getDatabaseCreate(int version)
	{
		boolean first = true;
		StringBuilder create = new StringBuilder("CREATE TABLE ");
		create.append(TABLE_VPNPROFILE);
		create.append(" (");
		for (DbColumn column : COLUMNS)
		{
			if (column.Since <= version)
			{
				if (!first)
				{
					create.append(",");
				}
				first = false;
				create.append(column.Name);
				create.append(" ");
				create.append(column.Type);
			}
		}
		create.append(");");
		return create.toString();
	}

	private static String[] getColumns(int version)
	{
		ArrayList<String> columns = new ArrayList<>();
		for (DbColumn column : COLUMNS)
		{
			if (column.Since <= version)
			{
				columns.add(column.Name);
			}
		}
		return columns.toArray(new String[0]);
	}

	private static class DatabaseHelper extends SQLiteOpenHelper
	{
		public DatabaseHelper(Context context)
		{
			super(context, DATABASE_NAME, null, DATABASE_VERSION);
		}

		@Override
		public void onCreate(SQLiteDatabase database)
		{
			database.execSQL(getDatabaseCreate(DATABASE_VERSION));
		}

		@Override
		public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion)
		{
			Log.w(TAG, "Upgrading database from version " + oldVersion +
				  " to " + newVersion);
			if (oldVersion < 2)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_USER_CERTIFICATE +
						   " TEXT;");
			}
			if (oldVersion < 3)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_VPN_TYPE +
						   " TEXT DEFAULT '';");
			}
			if (oldVersion < 4)
			{	/* remove NOT NULL constraint from username column */
				updateColumns(db, 4);
			}
			if (oldVersion < 5)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_MTU +
						   " INTEGER;");
			}
			if (oldVersion < 6)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_PORT +
						   " INTEGER;");
			}
			if (oldVersion < 7)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_SPLIT_TUNNELING +
						   " INTEGER;");
			}
			if (oldVersion < 8)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_LOCAL_ID +
						   " TEXT;");
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_REMOTE_ID +
						   " TEXT;");
			}
			if (oldVersion < 9)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_UUID +
						   " TEXT;");
				updateColumns(db, 9);
			}
			if (oldVersion < 10)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_EXCLUDED_SUBNETS +
						   " TEXT;");
			}
			if (oldVersion < 11)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_INCLUDED_SUBNETS +
						   " TEXT;");
			}
			if (oldVersion < 12)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_SELECTED_APPS +
						   " INTEGER;");
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_SELECTED_APPS_LIST +
						   " TEXT;");
			}
			if (oldVersion < 13)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_NAT_KEEPALIVE +
						   " INTEGER;");
			}
			if (oldVersion < 14)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_FLAGS +
						   " INTEGER;");
			}
			if (oldVersion < 15)
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_IKE_PROPOSAL +
						   " TEXT;");
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " ADD " + KEY_ESP_PROPOSAL +
						   " TEXT;");
			}
			if (oldVersion < 16)
			{	/* add a UUID to all entries that haven't one yet */
				db.beginTransaction();
				try
				{
					Cursor cursor = db.query(TABLE_VPNPROFILE, ALL_COLUMNS, KEY_UUID + " is NULL", null, null, null, null);
					for (cursor.moveToFirst(); !cursor.isAfterLast(); cursor.moveToNext())
					{
						ContentValues values = new ContentValues();
						values.put(KEY_UUID, UUID.randomUUID().toString());
						db.update(TABLE_VPNPROFILE, values, KEY_ID + " = " + cursor.getLong(cursor.getColumnIndex(KEY_ID)), null);
					}
					cursor.close();
					db.setTransactionSuccessful();
				}
				finally
				{
					db.endTransaction();
				}
			}
		}

		private void updateColumns(SQLiteDatabase db, int version)
		{
			db.beginTransaction();
			try
			{
				db.execSQL("ALTER TABLE " + TABLE_VPNPROFILE + " RENAME TO tmp_" + TABLE_VPNPROFILE + ";");
				db.execSQL(getDatabaseCreate(version));
				StringBuilder insert = new StringBuilder("INSERT INTO " + TABLE_VPNPROFILE + " SELECT ");
				SQLiteQueryBuilder.appendColumns(insert, getColumns(version));
				db.execSQL(insert.append(" FROM tmp_" + TABLE_VPNPROFILE + ";").toString());
				db.execSQL("DROP TABLE tmp_" + TABLE_VPNPROFILE + ";");
				db.setTransactionSuccessful();
			}
			finally
			{
				db.endTransaction();
			}
		}
	}

	/**
	 * Construct a new VPN profile data source. The context is used to
	 * open/create the database.
	 * @param context context used to access the database
	 */
	public VpnProfileDataSource(Context context)
	{
		this.mContext = context;
	}

	/**
	 * Open the VPN profile data source. The database is automatically created
	 * if it does not yet exist. If that fails an exception is thrown.
	 * @return itself (allows to chain initialization calls)
	 * @throws SQLException if the database could not be opened or created
	 */
	public VpnProfileDataSource open() throws SQLException
	{
		if (mDbHelper == null)
		{
			mDbHelper = new DatabaseHelper(mContext);
			mDatabase = mDbHelper.getWritableDatabase();
		}
		return this;
	}

	/**
	 * Close the data source.
	 */
	public void close()
	{
		if (mDbHelper != null)
		{
			mDbHelper.close();
			mDbHelper = null;
		}
	}

	/**
	 * Insert the given VPN profile into the database.  On success the Id of
	 * the object is updated and the object returned.
	 *
	 * @param profile the profile to add
	 * @return the added VPN profile or null, if failed
	 */
	public VpnProfile insertProfile(VpnProfile profile)
	{
		ContentValues values = ContentValuesFromVpnProfile(profile);
		long insertId = mDatabase.insert(TABLE_VPNPROFILE, null, values);
		if (insertId == -1)
		{
			return null;
		}
		profile.setId(insertId);
		return profile;
	}

	/**
	 * Updates the given VPN profile in the database.
	 * @param profile the profile to update
	 * @return true if update succeeded, false otherwise
	 */
	public boolean updateVpnProfile(VpnProfile profile)
	{
		long id = profile.getId();
		ContentValues values = ContentValuesFromVpnProfile(profile);
		return mDatabase.update(TABLE_VPNPROFILE, values, KEY_ID + " = " + id, null) > 0;
	}

	/**
	 * Delete the given VPN profile from the database.
	 * @param profile the profile to delete
	 * @return true if deleted, false otherwise
	 */
	public boolean deleteVpnProfile(VpnProfile profile)
	{
		long id = profile.getId();
		return mDatabase.delete(TABLE_VPNPROFILE, KEY_ID + " = " + id, null) > 0;
	}

	/**
	 * Get a single VPN profile from the database.
	 * @param id the ID of the VPN profile
	 * @return the profile or null, if not found
	 */
	public VpnProfile getVpnProfile(long id)
	{
		VpnProfile profile = null;
		Cursor cursor = mDatabase.query(TABLE_VPNPROFILE, ALL_COLUMNS,
										KEY_ID + "=" + id, null, null, null, null);
		if (cursor.moveToFirst())
		{
			profile = VpnProfileFromCursor(cursor);
		}
		cursor.close();
		return profile;
	}

	/**
	 * Get a single VPN profile from the database by its UUID.
	 * @param uuid the UUID of the VPN profile
	 * @return the profile or null, if not found
	 */
	public VpnProfile getVpnProfile(UUID uuid)
	{
		VpnProfile profile = null;
		Cursor cursor = mDatabase.query(TABLE_VPNPROFILE, ALL_COLUMNS,
										KEY_UUID + "='" + uuid.toString() + "'", null, null, null, null);
		if (cursor.moveToFirst())
		{
			profile = VpnProfileFromCursor(cursor);
		}
		cursor.close();
		return profile;
	}

	/**
	 * Get a single VPN profile from the database by its UUID as String.
	 * @param uuid the UUID of the VPN profile as String
	 * @return the profile or null, if not found
	 */
	public VpnProfile getVpnProfile(String uuid)
	{
		try
		{
			if (uuid != null)
			{
				return getVpnProfile(UUID.fromString(uuid));
			}
			return null;
		}
		catch (IllegalArgumentException e)
		{
			e.printStackTrace();
			return null;
		}
	}

	/**
	 * Get a list of all VPN profiles stored in the database.
	 * @return list of VPN profiles
	 */
	public List<VpnProfile> getAllVpnProfiles()
	{
		List<VpnProfile> vpnProfiles = new ArrayList<VpnProfile>();

		Cursor cursor = mDatabase.query(TABLE_VPNPROFILE, ALL_COLUMNS, null, null, null, null, null);
		cursor.moveToFirst();
		while (!cursor.isAfterLast())
		{
			VpnProfile vpnProfile = VpnProfileFromCursor(cursor);
			vpnProfiles.add(vpnProfile);
			cursor.moveToNext();
		}
		cursor.close();
		return vpnProfiles;
	}

	private VpnProfile VpnProfileFromCursor(Cursor cursor)
	{
		VpnProfile profile = new VpnProfile();
		profile.setId(cursor.getLong(cursor.getColumnIndex(KEY_ID)));
		profile.setUUID(UUID.fromString(cursor.getString(cursor.getColumnIndex(KEY_UUID))));
		profile.setName(cursor.getString(cursor.getColumnIndex(KEY_NAME)));
		profile.setGateway(cursor.getString(cursor.getColumnIndex(KEY_GATEWAY)));
		profile.setVpnType(VpnType.fromIdentifier(cursor.getString(cursor.getColumnIndex(KEY_VPN_TYPE))));
		profile.setUsername(cursor.getString(cursor.getColumnIndex(KEY_USERNAME)));
		profile.setPassword(cursor.getString(cursor.getColumnIndex(KEY_PASSWORD)));
		profile.setCertificateAlias(cursor.getString(cursor.getColumnIndex(KEY_CERTIFICATE)));
		profile.setUserCertificateAlias(cursor.getString(cursor.getColumnIndex(KEY_USER_CERTIFICATE)));
		profile.setMTU(getInt(cursor, cursor.getColumnIndex(KEY_MTU)));
		profile.setPort(getInt(cursor, cursor.getColumnIndex(KEY_PORT)));
		profile.setSplitTunneling(getInt(cursor, cursor.getColumnIndex(KEY_SPLIT_TUNNELING)));
		profile.setLocalId(cursor.getString(cursor.getColumnIndex(KEY_LOCAL_ID)));
		profile.setRemoteId(cursor.getString(cursor.getColumnIndex(KEY_REMOTE_ID)));
		profile.setExcludedSubnets(cursor.getString(cursor.getColumnIndex(KEY_EXCLUDED_SUBNETS)));
		profile.setIncludedSubnets(cursor.getString(cursor.getColumnIndex(KEY_INCLUDED_SUBNETS)));
		profile.setSelectedAppsHandling(getInt(cursor, cursor.getColumnIndex(KEY_SELECTED_APPS)));
		profile.setSelectedApps(cursor.getString(cursor.getColumnIndex(KEY_SELECTED_APPS_LIST)));
		profile.setNATKeepAlive(getInt(cursor, cursor.getColumnIndex(KEY_NAT_KEEPALIVE)));
		profile.setFlags(getInt(cursor, cursor.getColumnIndex(KEY_FLAGS)));
		profile.setIkeProposal(cursor.getString(cursor.getColumnIndex(KEY_IKE_PROPOSAL)));
		profile.setEspProposal(cursor.getString(cursor.getColumnIndex(KEY_ESP_PROPOSAL)));
		return profile;
	}

	private ContentValues ContentValuesFromVpnProfile(VpnProfile profile)
	{
		ContentValues values = new ContentValues();
		values.put(KEY_UUID, profile.getUUID().toString());
		values.put(KEY_NAME, profile.getName());
		values.put(KEY_GATEWAY, profile.getGateway());
		values.put(KEY_VPN_TYPE, profile.getVpnType().getIdentifier());
		values.put(KEY_USERNAME, profile.getUsername());
		values.put(KEY_PASSWORD, profile.getPassword());
		values.put(KEY_CERTIFICATE, profile.getCertificateAlias());
		values.put(KEY_USER_CERTIFICATE, profile.getUserCertificateAlias());
		values.put(KEY_MTU, profile.getMTU());
		values.put(KEY_PORT, profile.getPort());
		values.put(KEY_SPLIT_TUNNELING, profile.getSplitTunneling());
		values.put(KEY_LOCAL_ID, profile.getLocalId());
		values.put(KEY_REMOTE_ID, profile.getRemoteId());
		values.put(KEY_EXCLUDED_SUBNETS, profile.getExcludedSubnets());
		values.put(KEY_INCLUDED_SUBNETS, profile.getIncludedSubnets());
		values.put(KEY_SELECTED_APPS, profile.getSelectedAppsHandling().getValue());
		values.put(KEY_SELECTED_APPS_LIST, profile.getSelectedApps());
		values.put(KEY_NAT_KEEPALIVE, profile.getNATKeepAlive());
		values.put(KEY_FLAGS, profile.getFlags());
		values.put(KEY_IKE_PROPOSAL, profile.getIkeProposal());
		values.put(KEY_ESP_PROPOSAL, profile.getEspProposal());
		return values;
	}

	private Integer getInt(Cursor cursor, int columnIndex)
	{
		return cursor.isNull(columnIndex) ? null : cursor.getInt(columnIndex);
	}

	private static class DbColumn
	{
		public final String Name;
		public final String Type;
		public final Integer Since;

		public DbColumn(String name, String type, Integer since)
		{
			Name = name;
			Type = type;
			Since = since;
		}
	}
}
