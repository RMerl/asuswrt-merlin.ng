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

package org.strongswan.android.ui;

import android.content.Context;
import android.os.Bundle;
import android.support.v4.app.ListFragment;
import android.view.View;
import android.widget.ListView;

import org.strongswan.android.R;
import org.strongswan.android.logic.imc.RemediationInstruction;
import org.strongswan.android.ui.adapter.RemediationInstructionAdapter;

import java.util.ArrayList;

public class RemediationInstructionsFragment extends ListFragment
{
	public static final String EXTRA_REMEDIATION_INSTRUCTIONS = "instructions";
	private static final String KEY_POSITION = "position";
	private ArrayList<RemediationInstruction> mInstructions = null;
	private OnRemediationInstructionSelectedListener mListener;
	private RemediationInstructionAdapter mAdapter;
	private int mCurrentPosition = -1;

	/**
	 * The activity containing this fragment should implement this interface
	 */
	public interface OnRemediationInstructionSelectedListener
	{
		public void onRemediationInstructionSelected(RemediationInstruction instruction);
	}

	@Override
	public void onActivityCreated(Bundle savedInstanceState)
	{
		super.onActivityCreated(savedInstanceState);

		if (savedInstanceState != null)
		{
			mInstructions = savedInstanceState.getParcelableArrayList(EXTRA_REMEDIATION_INSTRUCTIONS);
			mCurrentPosition = savedInstanceState.getInt(KEY_POSITION);
		}
	}

	@Override
	public void onSaveInstanceState(Bundle outState)
	{
		super.onSaveInstanceState(outState);
		outState.putParcelableArrayList(RemediationInstructionsFragment.EXTRA_REMEDIATION_INSTRUCTIONS, mInstructions);
		outState.putInt(KEY_POSITION, mCurrentPosition);
	}

	@Override
	public void onAttach(Context context)
	{
		super.onAttach(context);

		if (context instanceof OnRemediationInstructionSelectedListener)
		{
			mListener = (OnRemediationInstructionSelectedListener)context;
		}
	}

	@Override
	public void onStart()
	{
		super.onStart();

		boolean two_pane = getFragmentManager().findFragmentById(R.id.remediation_instruction_fragment) != null;
		if (two_pane)
		{	/* two-pane layout, make list items selectable */
			getListView().setChoiceMode(ListView.CHOICE_MODE_SINGLE);
		}

		Bundle args = getArguments();
		if (mInstructions == null && args != null)
		{
			mInstructions = args.getParcelableArrayList(EXTRA_REMEDIATION_INSTRUCTIONS);
		}
		updateView(mInstructions);

		if (two_pane && mCurrentPosition == -1 && mInstructions.size() > 0)
		{	/* two-pane layout, select first instruction */
			mCurrentPosition = 0;
			mListener.onRemediationInstructionSelected(mInstructions.get(0));
		}
		getListView().setItemChecked(mCurrentPosition, true);
	}

	@Override
	public void onListItemClick(ListView l, View v, int position, long id)
	{
		mCurrentPosition = position;
		mListener.onRemediationInstructionSelected(mInstructions.get(position));
		getListView().setItemChecked(position, true);
	}

	public void updateView(ArrayList<RemediationInstruction> instructions)
	{
		if (mAdapter == null)
		{
			mAdapter = new RemediationInstructionAdapter(getActivity());
			setListAdapter(mAdapter);
		}
		mInstructions = instructions;
		mAdapter.setData(mInstructions);
	}
}
