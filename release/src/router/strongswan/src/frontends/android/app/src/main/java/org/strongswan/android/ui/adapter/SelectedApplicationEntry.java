/*
 * Copyright (C) 2017 Tobias Brunner
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

package org.strongswan.android.ui.adapter;

import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.support.annotation.NonNull;

import java.text.Collator;

public class SelectedApplicationEntry implements Comparable<SelectedApplicationEntry>
{
	private final ApplicationInfo mInfo;
	private final Drawable mIcon;
	private final String mName;
	private boolean mSelected;

	public SelectedApplicationEntry(PackageManager packageManager, ApplicationInfo info)
	{
		mInfo = info;
		CharSequence name = info.loadLabel(packageManager);
		mName = name == null ? info.packageName : name.toString();
		mIcon = info.loadIcon(packageManager);
	}

	public void setSelected(boolean selected)
	{
		mSelected = selected;
	}

	public boolean isSelected()
	{
		return mSelected;
	}

	public ApplicationInfo getInfo()
	{
		return mInfo;
	}

	public Drawable getIcon()
	{
		return mIcon;
	}

	@Override
	public String toString()
	{
		return mName;
	}

	@Override
	public int compareTo(@NonNull SelectedApplicationEntry another)
	{
		return Collator.getInstance().compare(toString(), another.toString());
	}
}
