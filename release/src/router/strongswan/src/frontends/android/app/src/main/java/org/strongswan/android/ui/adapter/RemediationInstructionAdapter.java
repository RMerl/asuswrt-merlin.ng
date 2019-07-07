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

package org.strongswan.android.ui.adapter;

import java.util.List;

import org.strongswan.android.R;
import org.strongswan.android.logic.imc.RemediationInstruction;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class RemediationInstructionAdapter extends ArrayAdapter<RemediationInstruction>
{
	public RemediationInstructionAdapter(Context context)
	{
		super(context, 0);
	}

	/**
	 * Set new data for this adapter.
	 *
	 * @param data the new data (null to clear)
	 */
	public void setData(List<RemediationInstruction> data)
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
			view = inflater.inflate(R.layout.remediation_instruction_item, parent, false);
		}
		RemediationInstruction item = getItem(position);
		TextView text = (TextView)view.findViewById(android.R.id.text1);
		text.setText(item.getTitle());
		text = (TextView)view.findViewById(android.R.id.text2);
		text.setText(item.getDescription());
		return view;
	}
}
