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

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.content.ContextCompat;
import android.view.GestureDetector;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.view.ViewConfiguration;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.strongswan.android.R;
import org.strongswan.android.logic.VpnStateService;
import org.strongswan.android.logic.VpnStateService.VpnStateListener;
import org.strongswan.android.logic.imc.ImcState;
import org.strongswan.android.logic.imc.RemediationInstruction;

import java.util.ArrayList;

public class ImcStateFragment extends Fragment implements VpnStateListener
{
	private int mColorIsolate;
	private int mColorBlock;
	private boolean mVisible;
	private TextView mStateView;
	private TextView mAction;
	private LinearLayout mButton;
	private VpnStateService mService;
	private final ServiceConnection mServiceConnection = new ServiceConnection()
	{
		@Override
		public void onServiceDisconnected(ComponentName name)
		{
			mService = null;
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service)
		{
			mService = ((VpnStateService.LocalBinder)service).getService();
			if (mVisible)
			{
				mService.registerListener(ImcStateFragment.this);
				updateView();
			}
		}
	};

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		mColorIsolate = ContextCompat.getColor(getActivity(), R.color.warning_text);
		mColorBlock = ContextCompat.getColor(getActivity(), R.color.error_text);

		/* bind to the service only seems to work from the ApplicationContext */
		Context context = getActivity().getApplicationContext();
		context.bindService(new Intent(context, VpnStateService.class),
							mServiceConnection, Service.BIND_AUTO_CREATE);
		/* hide it initially */
		getFragmentManager().beginTransaction().hide(this).commit();
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container,
							 Bundle savedInstanceState)
	{
		View view = inflater.inflate(R.layout.imc_state_fragment, container, false);

		mButton = (LinearLayout)view.findViewById(R.id.imc_state_button);
		mButton.setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View v)
			{
				Intent intent;
				if (mService != null && !mService.getRemediationInstructions().isEmpty())
				{
					intent = new Intent(getActivity(), RemediationInstructionsActivity.class);
					intent.putParcelableArrayListExtra(RemediationInstructionsFragment.EXTRA_REMEDIATION_INSTRUCTIONS,
													   new ArrayList<RemediationInstruction>(mService.getRemediationInstructions()));
				}
				else
				{
					intent = new Intent(getActivity(), LogActivity.class);
				}
				startActivity(intent);
			}
		});
		final GestureDetector gestures = new GestureDetector(getActivity(), new GestureDetector.SimpleOnGestureListener()
		{
			/* a better value would be getScaledTouchExplorationTapSlop() but that is hidden */
			private final int mMinDistance = ViewConfiguration.get(getActivity()).getScaledTouchSlop() * 4;

			@Override
			public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY)
			{
				if (Math.abs(e1.getX() - e2.getX()) >= mMinDistance)
				{	/* only if the user swiped a minimum horizontal distance */
					if (mService != null)
					{
						mService.setImcState(ImcState.UNKNOWN);
					}
					return true;
				}
				return false;
			}
		});
		mButton.setOnTouchListener(new OnTouchListener()
		{
			@Override
			public boolean onTouch(View v, MotionEvent event)
			{
				return gestures.onTouchEvent(event);
			}
		});

		mStateView = (TextView)view.findViewById(R.id.imc_state);
		mAction = (TextView)view.findViewById(R.id.action);

		return view;
	}

	@Override
	public void onResume()
	{
		super.onResume();
		mVisible = true;
		if (mService != null)
		{
			mService.registerListener(this);
			updateView();
		}
	}

	@Override
	public void onPause()
	{
		super.onPause();
		mVisible = false;
		if (mService != null)
		{
			mService.unregisterListener(this);
		}
	}

	@Override
	public void onDestroy()
	{
		super.onDestroy();
		if (mService != null)
		{
			getActivity().getApplicationContext().unbindService(mServiceConnection);
		}
	}

	@Override
	public void stateChanged()
	{
		updateView();
	}

	public void updateView()
	{
		FragmentManager fm = getFragmentManager();
		if (fm == null)
		{
			return;
		}
		FragmentTransaction ft = fm.beginTransaction();

		switch (mService.getImcState())
		{
			case UNKNOWN:
			case ALLOW:
				ft.hide(this);
				break;
			case ISOLATE:
				mStateView.setText(R.string.imc_state_isolate);
				mStateView.setTextColor(mColorIsolate);
				ft.show(this);
				break;
			case BLOCK:
				mStateView.setText(R.string.imc_state_block);
				mStateView.setTextColor(mColorBlock);
				ft.show(this);
				break;
		}
		ft.commit();

		mAction.setText(mService.getRemediationInstructions().isEmpty() ? R.string.show_log
																		: R.string.show_remediation_instructions);
	}
}
