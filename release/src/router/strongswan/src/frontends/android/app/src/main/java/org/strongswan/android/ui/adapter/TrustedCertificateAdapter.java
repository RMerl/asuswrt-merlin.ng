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

package org.strongswan.android.ui.adapter;

import java.util.List;

import org.strongswan.android.R;
import org.strongswan.android.security.TrustedCertificateEntry;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class TrustedCertificateAdapter extends ArrayAdapter<TrustedCertificateEntry>
{
	public TrustedCertificateAdapter(Context context)
	{
		super(context, R.layout.trusted_certificates_item);
	}

	/**
	 * Set new data for this adapter.
	 *
	 * @param data the new data (null to clear)
	 */
	public void setData(List<TrustedCertificateEntry> data)
	{
		clear();
		if (data != null)
		{
			addAll(data);
		}
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent)
	{
		View view;
		if (convertView != null)
		{
			view = convertView;
		}
		else
		{
			LayoutInflater inflater = LayoutInflater.from(getContext());
			view = inflater.inflate(R.layout.trusted_certificates_item, parent, false);
		}
		TrustedCertificateEntry item = getItem(position);
		TextView text = (TextView)view.findViewById(R.id.subject_primary);
		text.setText(item.getSubjectPrimary());
		text = (TextView)view.findViewById(R.id.subject_secondary);
		text.setText(item.getSubjectSecondary());
		return view;
	}
}
