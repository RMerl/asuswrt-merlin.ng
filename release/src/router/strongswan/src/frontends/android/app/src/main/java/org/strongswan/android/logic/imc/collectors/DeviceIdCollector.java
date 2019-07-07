/*
 * Copyright (C) 2013 Tobias Brunner
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

package org.strongswan.android.logic.imc.collectors;

import org.strongswan.android.logic.imc.attributes.Attribute;
import org.strongswan.android.logic.imc.attributes.DeviceIdAttribute;

import android.content.ContentResolver;
import android.content.Context;

public class DeviceIdCollector implements Collector
{
	private final ContentResolver mContentResolver;

	public DeviceIdCollector(Context context)
	{
		mContentResolver = context.getContentResolver();
	}

	@Override
	public Attribute getMeasurement()
	{
		String id = android.provider.Settings.Secure.getString(mContentResolver, "android_id");
		if (id != null)
		{
			DeviceIdAttribute attribute = new DeviceIdAttribute();
			attribute.setDeviceId(id);
			return attribute;
		}
		return null;
	}
}
