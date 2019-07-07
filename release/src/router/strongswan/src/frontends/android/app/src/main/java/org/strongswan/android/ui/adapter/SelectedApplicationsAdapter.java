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

import android.content.Context;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.Filter;
import android.widget.Filterable;
import android.widget.ImageView;
import android.widget.TextView;

import org.strongswan.android.R;
import org.strongswan.android.ui.widget.CheckableLinearLayout;

import java.util.ArrayList;
import java.util.List;

public class SelectedApplicationsAdapter extends BaseAdapter implements Filterable
{
	private Context mContext;
	private final Object mLock = new Object();
	private List<SelectedApplicationEntry> mData;
	private List<SelectedApplicationEntry> mDataFiltered;
	private SelectedApplicationsFilter mFilter;

	public SelectedApplicationsAdapter(Context context)
	{
		mContext = context;
		mData = mDataFiltered = new ArrayList<>();
	}

	/**
	 * Set new data for this adapter.
	 *
	 * @param data the new data (null to clear)
	 */
	public void setData(List<SelectedApplicationEntry> data)
	{
		synchronized (mLock)
		{
			mData.clear();
			mDataFiltered = mData;
			if (data != null)
			{
				mData.addAll(data);
			}
		}
		notifyDataSetChanged();
	}

	@Override
	public int getCount()
	{
		return mDataFiltered.size();
	}

	@Override
	public SelectedApplicationEntry getItem(int position)
	{
		return mDataFiltered.get(position);
	}

	@Override
	public long getItemId(int position)
	{
		return mDataFiltered.get(position).toString().hashCode();
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
			LayoutInflater inflater = LayoutInflater.from(mContext);
			view = inflater.inflate(R.layout.selected_application_item, parent, false);
		}
		SelectedApplicationEntry item = getItem(position);
		CheckableLinearLayout checkable = (CheckableLinearLayout)view;
		checkable.setChecked(item.isSelected());
		ImageView icon = (ImageView)view.findViewById(R.id.app_icon);
		icon.setImageDrawable(item.getIcon());
		TextView text = (TextView)view.findViewById(R.id.app_name);
		text.setText(item.toString());
		return view;
	}

	@Override
	public Filter getFilter()
	{
		if (mFilter == null)
		{
			mFilter = new SelectedApplicationsFilter();
		}
		return mFilter;
	}

	private class SelectedApplicationsFilter extends Filter
	{

		@Override
		protected FilterResults performFiltering(CharSequence constraint)
		{
			FilterResults results = new FilterResults();
			ArrayList<SelectedApplicationEntry> data, filtered;

			synchronized (mLock)
			{
				data = new ArrayList<>(mData);
			}

			if (TextUtils.isEmpty(constraint))
			{
				filtered = data;
			}
			else
			{
				String filter = constraint.toString().toLowerCase();
				filtered = new ArrayList<>();
				for (SelectedApplicationEntry entry : data)
				{
					if (entry.toString().toLowerCase().contains(filter))
					{
						filtered.add(entry);
					}
				}
			}
			results.values = filtered;
			results.count = filtered.size();
			return results;
		}

		@Override
		@SuppressWarnings("unchecked")
		protected void publishResults(CharSequence constraint, FilterResults results)
		{
			mDataFiltered = (List<SelectedApplicationEntry>)results.values;
			notifyDataSetChanged();
		}
	}
}
