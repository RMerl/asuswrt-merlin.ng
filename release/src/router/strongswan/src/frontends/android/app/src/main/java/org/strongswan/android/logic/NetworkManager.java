/*
 * Copyright (C) 2012-2015 Tobias Brunner
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

package org.strongswan.android.logic;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

import java.util.LinkedList;

public class NetworkManager extends BroadcastReceiver implements Runnable
{
	private final Context mContext;
	private volatile boolean mRegistered;
	private Thread mEventNotifier;
	private LinkedList<Boolean> mEvents = new LinkedList<>();

	public NetworkManager(Context context)
	{
		mContext = context;
	}

	public void Register()
	{
		mEvents.clear();
		mRegistered = true;
		mEventNotifier = new Thread(this);
		mEventNotifier.start();
		mContext.registerReceiver(this, new IntentFilter(ConnectivityManager.CONNECTIVITY_ACTION));
	}

	public void Unregister()
	{
		mContext.unregisterReceiver(this);
		mRegistered = false;
		synchronized (this)
		{
			notifyAll();
		}
		try
		{
			mEventNotifier.join();
			mEventNotifier = null;
		}
		catch (InterruptedException e)
		{
			e.printStackTrace();
		}
	}

	public boolean isConnected()
	{
		ConnectivityManager cm = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
		NetworkInfo info = null;
		if (cm != null)
		{
			info = cm.getActiveNetworkInfo();
		}
		return info != null && info.isConnected();
	}

	@Override
	public void onReceive(Context context, Intent intent)
	{
		synchronized (this)
		{
			mEvents.addLast(isConnected());
			notifyAll();
		}
	}

	@Override
	public void run()
	{
		while (mRegistered)
		{
			boolean connected;

			synchronized (this)
			{
				try
				{
					while (mRegistered && mEvents.isEmpty())
					{
						wait();
					}
				}
				catch (InterruptedException ex)
				{
					break;
				}
				if (!mRegistered)
				{
					break;
				}
				connected = mEvents.removeFirst();
			}
			/* call the native parts without holding the lock */
			networkChanged(!connected);
		}
	}

	/**
	 * Notify the native parts about a network change
	 *
	 * @param disconnected true if no connection is available at the moment
	 */
	public native void networkChanged(boolean disconnected);
}
