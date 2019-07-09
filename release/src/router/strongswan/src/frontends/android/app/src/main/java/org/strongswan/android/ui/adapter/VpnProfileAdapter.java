/*
 * Copyright (C) 2012 Tobias Brunner
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

package org.strongswan.android.ui.adapter;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import org.strongswan.android.R;
import org.strongswan.android.data.VpnProfile;
import org.strongswan.android.data.VpnType.VpnTypeFeature;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class VpnProfileAdapter extends ArrayAdapter<VpnProfile>
{
	private final int resource;
	private final List<VpnProfile> items;

	public VpnProfileAdapter(Context context, int resource,
							 List<VpnProfile> items)
	{
		super(context, resource, items);
		this.resource = resource;
		this.items = items;
		sortItems();
	}

	@Override
	public View getView(int position, View convertView, ViewGroup parent)
	{
		View vpnProfileView;
		if (convertView != null)
		{
			vpnProfileView = convertView;
		}
		else
		{
			LayoutInflater inflater = LayoutInflater.from(getContext());
			vpnProfileView = inflater.inflate(resource, null);
		}
		VpnProfile profile = getItem(position);
		TextView tv = (TextView)vpnProfileView.findViewById(R.id.profile_item_name);
		tv.setText(profile.getName());
		tv = (TextView)vpnProfileView.findViewById(R.id.profile_item_gateway);
		tv.setText(getContext().getString(R.string.profile_gateway_label) + ": " + profile.getGateway());
		tv = (TextView)vpnProfileView.findViewById(R.id.profile_item_username);
		if (profile.getVpnType().has(VpnTypeFeature.USER_PASS))
		{	/* if the view is reused we make sure it is visible */
			tv.setVisibility(View.VISIBLE);
			tv.setText(getContext().getString(R.string.profile_username_label) + ": " + profile.getUsername());
		}
		else if (profile.getVpnType().has(VpnTypeFeature.CERTIFICATE) &&
				 profile.getLocalId() != null)
		{
			tv.setVisibility(View.VISIBLE);
			tv.setText(getContext().getString(R.string.profile_user_select_id_label) + ": " + profile.getLocalId());
		}
		else
		{
			tv.setVisibility(View.GONE);
		}
		tv = (TextView)vpnProfileView.findViewById(R.id.profile_item_certificate);
		if (profile.getVpnType().has(VpnTypeFeature.CERTIFICATE))
		{
			tv.setText(getContext().getString(R.string.profile_user_certificate_label) + ": " + profile.getUserCertificateAlias());
			tv.setVisibility(View.VISIBLE);
		}
		else
		{
			tv.setVisibility(View.GONE);
		}
		return vpnProfileView;
	}

	@Override
	public void notifyDataSetChanged()
	{
		sortItems();
		super.notifyDataSetChanged();
	}

	private void sortItems()
	{
		Collections.sort(this.items, new Comparator<VpnProfile>() {
			@Override
			public int compare(VpnProfile lhs, VpnProfile rhs)
			{
				return lhs.getName().compareToIgnoreCase(rhs.getName());
			}
		});
	}
}
