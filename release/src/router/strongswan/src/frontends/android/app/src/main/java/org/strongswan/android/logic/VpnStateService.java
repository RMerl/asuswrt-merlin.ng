/*
 * Copyright (C) 2012-2017 Tobias Brunner
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

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.SystemClock;
import android.support.v4.content.ContextCompat;

import org.strongswan.android.R;
import org.strongswan.android.data.VpnProfile;
import org.strongswan.android.data.VpnProfileDataSource;
import org.strongswan.android.data.VpnType;
import org.strongswan.android.logic.imc.ImcState;
import org.strongswan.android.logic.imc.RemediationInstruction;
import org.strongswan.android.ui.VpnProfileControlActivity;

import java.lang.ref.WeakReference;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.concurrent.Callable;

public class VpnStateService extends Service
{
	private final HashSet<VpnStateListener> mListeners = new HashSet<VpnStateListener>();
	private final IBinder mBinder = new LocalBinder();
	private long mConnectionID = 0;
	private Handler mHandler;
	private VpnProfile mProfile;
	private State mState = State.DISABLED;
	private ErrorState mError = ErrorState.NO_ERROR;
	private ImcState mImcState = ImcState.UNKNOWN;
	private final LinkedList<RemediationInstruction> mRemediationInstructions = new LinkedList<RemediationInstruction>();
	private static long RETRY_INTERVAL = 1000;
	/* cap the retry interval at 2 minutes */
	private static long MAX_RETRY_INTERVAL = 120000;
	private static int RETRY_MSG = 1;
	private RetryTimeoutProvider mTimeoutProvider = new RetryTimeoutProvider();
	private long mRetryTimeout;
	private long mRetryIn;

	public enum State
	{
		DISABLED,
		CONNECTING,
		CONNECTED,
		DISCONNECTING,
	}

	public enum ErrorState
	{
		NO_ERROR,
		AUTH_FAILED,
		PEER_AUTH_FAILED,
		LOOKUP_FAILED,
		UNREACHABLE,
		GENERIC_ERROR,
		PASSWORD_MISSING,
		CERTIFICATE_UNAVAILABLE,
	}

	/**
	 * Listener interface for bound clients that are interested in changes to
	 * this Service.
	 */
	public interface VpnStateListener
	{
		public void stateChanged();
	}

	/**
	 * Simple Binder that allows to directly access this Service class itself
	 * after binding to it.
	 */
	public class LocalBinder extends Binder
	{
		public VpnStateService getService()
		{
			return VpnStateService.this;
		}
	}

	@Override
	public void onCreate()
	{
		/* this handler allows us to notify listeners from the UI thread and
		 * not from the threads that actually report any state changes */
		mHandler = new RetryHandler(this);
	}

	@Override
	public IBinder onBind(Intent intent)
	{
		return mBinder;
	}

	@Override
	public void onDestroy()
	{
	}

	/**
	 * Register a listener with this Service. We assume this is called from
	 * the main thread so no synchronization is happening.
	 *
	 * @param listener listener to register
	 */
	public void registerListener(VpnStateListener listener)
	{
		mListeners.add(listener);
	}

	/**
	 * Unregister a listener from this Service.
	 *
	 * @param listener listener to unregister
	 */
	public void unregisterListener(VpnStateListener listener)
	{
		mListeners.remove(listener);
	}

	/**
	 * Get the current VPN profile.
	 *
	 * @return profile
	 */
	public VpnProfile getProfile()
	{	/* only updated from the main thread so no synchronization needed */
		return mProfile;
	}

	/**
	 * Get the current connection ID.  May be used to track which state
	 * changes have already been handled.
	 *
	 * Is increased when startConnection() is called.
	 *
	 * @return connection ID
	 */
	public long getConnectionID()
	{	/* only updated from the main thread so no synchronization needed */
		return mConnectionID;
	}

	/**
	 * Get the total number of seconds until there is an automatic retry to reconnect.
	 * @return total number of seconds until the retry
	 */
	public int getRetryTimeout()
	{
		return (int)(mRetryTimeout / 1000);
	}

	/**
	 * Get the number of seconds until there is an automatic retry to reconnect.
	 * @return number of seconds until the retry
	 */
	public int getRetryIn()
	{
		return (int)(mRetryIn / 1000);
	}

	/**
	 * Get the current state.
	 *
	 * @return state
	 */
	public State getState()
	{	/* only updated from the main thread so no synchronization needed */
		return mState;
	}

	/**
	 * Get the current error, if any.
	 *
	 * @return error
	 */
	public ErrorState getErrorState()
	{	/* only updated from the main thread so no synchronization needed */
		return mError;
	}

	/**
	 * Get a description of the current error, if any.
	 *
	 * @return error description text id
	 */
	public int getErrorText()
	{
		switch (mError)
		{
			case AUTH_FAILED:
				if (mImcState == ImcState.BLOCK)
				{
					return R.string.error_assessment_failed;
				}
				else
				{
					return R.string.error_auth_failed;
				}
			case PEER_AUTH_FAILED:
				return R.string.error_peer_auth_failed;
			case LOOKUP_FAILED:
				return R.string.error_lookup_failed;
			case UNREACHABLE:
				return R.string.error_unreachable;
			case PASSWORD_MISSING:
				return R.string.error_password_missing;
			case CERTIFICATE_UNAVAILABLE:
				return R.string.error_certificate_unavailable;
			default:
				return R.string.error_generic;
		}
	}

	/**
	 * Get the current IMC state, if any.
	 *
	 * @return imc state
	 */
	public ImcState getImcState()
	{	/* only updated from the main thread so no synchronization needed */
		return mImcState;
	}

	/**
	 * Get the remediation instructions, if any.
	 *
	 * @return read-only list of instructions
	 */
	public List<RemediationInstruction> getRemediationInstructions()
	{	/* only updated from the main thread so no synchronization needed */
		return Collections.unmodifiableList(mRemediationInstructions);
	}

	/**
	 * Disconnect any existing connection and shutdown the daemon, the
	 * VpnService is not stopped but it is reset so new connections can be
	 * started.
	 */
	public void disconnect()
	{
		/* reset any potential retry timer and error state */
		resetRetryTimer();
		setError(ErrorState.NO_ERROR);

		/* as soon as the TUN device is created by calling establish() on the
		 * VpnService.Builder object the system binds to the service and keeps
		 * bound until the file descriptor of the TUN device is closed.  thus
		 * calling stopService() here would not stop (destroy) the service yet,
		 * instead we call startService() with a specific action which shuts down
		 * the daemon (and closes the TUN device, if any) */
		Context context = getApplicationContext();
		Intent intent = new Intent(context, CharonVpnService.class);
		intent.setAction(CharonVpnService.DISCONNECT_ACTION);
		context.startService(intent);
	}

	/**
	 * Connect (or reconnect) a profile
	 * @param profileInfo optional profile info (basically the UUID and password), taken from the
	 *                    previous profile if null
	 * @param fromScratch true if this is a manual retry/reconnect or a completely new connection
	 */
	public void connect(Bundle profileInfo, boolean fromScratch)
	{
		/* we assume we have the necessary permission */
		Context context = getApplicationContext();
		Intent intent = new Intent(context, CharonVpnService.class);
		if (profileInfo == null)
		{
			profileInfo = new Bundle();
			profileInfo.putString(VpnProfileDataSource.KEY_UUID, mProfile.getUUID().toString());
			/* pass the previous password along */
			profileInfo.putString(VpnProfileDataSource.KEY_PASSWORD, mProfile.getPassword());
		}
		if (fromScratch)
		{
			/* reset if this is a manual retry or a new connection */
			mTimeoutProvider.reset();
		}
		else
		{	/* mark this as an automatic retry */
			profileInfo.putBoolean(CharonVpnService.KEY_IS_RETRY, true);
		}
		intent.putExtras(profileInfo);
		ContextCompat.startForegroundService(context, intent);
	}

	/**
	 * Reconnect to the previous profile.
	 */
	public void reconnect()
	{
		if (mProfile == null)
		{
			return;
		}
		if (mProfile.getVpnType().has(VpnType.VpnTypeFeature.USER_PASS))
		{
			if (mProfile.getPassword() == null ||
				mError == ErrorState.AUTH_FAILED)
			{	/* show a dialog if we either don't have the password or if it might be the wrong
				 * one (which is or isn't stored with the profile, let the activity decide)  */
				Intent intent = new Intent(this, VpnProfileControlActivity.class);
				intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
				intent.setAction(VpnProfileControlActivity.START_PROFILE);
				intent.putExtra(VpnProfileControlActivity.EXTRA_VPN_PROFILE_ID, mProfile.getUUID().toString());
				startActivity(intent);
				/* reset the retry timer immediately in case the user needs more time to enter the password */
				notifyListeners(() -> {
					resetRetryTimer();
					return true;
				});
				return;
			}
		}
		connect(null, true);
	}

	/**
	 * Update state and notify all listeners about the change. By using a Handler
	 * this is done from the main UI thread and not the initial reporter thread.
	 * Also, in doing the actual state change from the main thread, listeners
	 * see all changes and none are skipped.
	 *
	 * @param change the state update to perform before notifying listeners, returns true if state changed
	 */
	private void notifyListeners(final Callable<Boolean> change)
	{
		mHandler.post(new Runnable() {
			@Override
			public void run()
			{
				try
				{
					if (change.call())
					{	/* otherwise there is no need to notify the listeners */
						for (VpnStateListener listener : mListeners)
						{
							listener.stateChanged();
						}
					}
				}
				catch (Exception e)
				{
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Called when a connection is started.  Sets the currently active VPN
	 * profile, resets IMC and Error state variables, sets the State to
	 * CONNECTING, increases the connection ID, and notifies all listeners.
	 *
	 * May be called from threads other than the main thread.
	 *
	 * @param profile current profile
	 */
	public void startConnection(final VpnProfile profile)
	{
		notifyListeners(new Callable<Boolean>() {
			@Override
			public Boolean call() throws Exception
			{
				resetRetryTimer();
				VpnStateService.this.mConnectionID++;
				VpnStateService.this.mProfile = profile;
				VpnStateService.this.mState = State.CONNECTING;
				VpnStateService.this.mError = ErrorState.NO_ERROR;
				VpnStateService.this.mImcState = ImcState.UNKNOWN;
				VpnStateService.this.mRemediationInstructions.clear();
				return true;
			}
		});
	}

	/**
	 * Update the state and notify all listeners, if changed.
	 *
	 * May be called from threads other than the main thread.
	 *
	 * @param state new state
	 */
	public void setState(final State state)
	{
		notifyListeners(new Callable<Boolean>() {
			@Override
			public Boolean call() throws Exception
			{
				if (state == State.CONNECTED)
				{	/* reset counter in case there is an error later on */
					mTimeoutProvider.reset();
				}
				if (VpnStateService.this.mState != state)
				{
					VpnStateService.this.mState = state;
					return true;
				}
				return false;
			}
		});
	}

	/**
	 * Set the current error state and notify all listeners, if changed.
	 *
	 * May be called from threads other than the main thread.
	 *
	 * @param error error state
	 */
	public void setError(final ErrorState error)
	{
		notifyListeners(new Callable<Boolean>() {
			@Override
			public Boolean call() throws Exception
			{
				if (VpnStateService.this.mError != error)
				{
					if (VpnStateService.this.mError == ErrorState.NO_ERROR)
					{
						setRetryTimer(error);
					}
					else if (error == ErrorState.NO_ERROR)
					{
						resetRetryTimer();
					}
					VpnStateService.this.mError = error;
					return true;
				}
				return false;
			}
		});
	}

	/**
	 * Set the current IMC state and notify all listeners, if changed.
	 *
	 * Setting the state to UNKNOWN clears all remediation instructions.
	 *
	 * May be called from threads other than the main thread.
	 *
	 * @param state IMC state
	 */
	public void setImcState(final ImcState state)
	{
		notifyListeners(new Callable<Boolean>() {
			@Override
			public Boolean call() throws Exception
			{
				if (state == ImcState.UNKNOWN)
				{
					VpnStateService.this.mRemediationInstructions.clear();
				}
				if (VpnStateService.this.mImcState != state)
				{
					VpnStateService.this.mImcState = state;
					return true;
				}
				return false;
			}
		});
	}

	/**
	 * Add the given remediation instruction to the internal list.  Listeners
	 * are not notified.
	 *
	 * Instructions are cleared if the IMC state is set to UNKNOWN.
	 *
	 * May be called from threads other than the main thread.
	 *
	 * @param instruction remediation instruction
	 */
	public void addRemediationInstruction(final RemediationInstruction instruction)
	{
		mHandler.post(new Runnable() {
			@Override
			public void run()
			{
				VpnStateService.this.mRemediationInstructions.add(instruction);
			}
		});
	}

	/**
	 * Sets the retry timer
	 */
	private void setRetryTimer(ErrorState error)
	{
		mRetryTimeout = mRetryIn = mTimeoutProvider.getTimeout(error);
		if (mRetryTimeout <= 0)
		{
			return;
		}
		mHandler.sendMessageAtTime(mHandler.obtainMessage(RETRY_MSG), SystemClock.uptimeMillis() + RETRY_INTERVAL);
	}

	/**
	 * Reset the retry timer
	 */
	private void resetRetryTimer()
	{
		mRetryTimeout = 0;
		mRetryIn = 0;
	}

	/**
	 * Special Handler subclass that handles the retry countdown (more accurate than CountDownTimer)
	 */
	private static class RetryHandler extends Handler {
		WeakReference<VpnStateService> mService;

		public RetryHandler(VpnStateService service)
		{
			mService = new WeakReference<>(service);
		}

		@Override
		public void handleMessage(Message msg)
		{
			/* handle retry countdown */
			if (mService.get().mRetryTimeout <= 0)
			{
				return;
			}
			mService.get().mRetryIn -= RETRY_INTERVAL;
			if (mService.get().mRetryIn > 0)
			{
				/* calculate next interval before notifying listeners */
				long next = SystemClock.uptimeMillis() + RETRY_INTERVAL;

				for (VpnStateListener listener : mService.get().mListeners)
				{
					listener.stateChanged();
				}
				sendMessageAtTime(obtainMessage(RETRY_MSG), next);
			}
			else
			{
				mService.get().connect(null, false);
			}
		}
	}

	/**
	 * Class that handles an exponential backoff for retry timeouts
	 */
	private static class RetryTimeoutProvider
	{
		private long mRetry;

		private long getBaseTimeout(ErrorState error)
		{
			switch (error)
			{
				case AUTH_FAILED:
					return 10000;
				case PEER_AUTH_FAILED:
					return 5000;
				case LOOKUP_FAILED:
					return 5000;
				case UNREACHABLE:
					return 5000;
				case PASSWORD_MISSING:
					/* this needs user intervention (entering the password) */
					return 0;
				case CERTIFICATE_UNAVAILABLE:
					/* if this is because the device has to be unlocked we might be able to reconnect */
					return 5000;
				default:
					return 10000;
			}
		}

		/**
		 * Called each time a new retry timeout is started. The timeout increases until reset() is
		 * called and the base timeout is returned again.
		 * @param error Error state
		 */
		public long getTimeout(ErrorState error)
		{
			long timeout = (long)(getBaseTimeout(error) * Math.pow(2, mRetry++));
			/* return the result rounded to seconds */
			return Math.min((timeout / 1000) * 1000, MAX_RETRY_INTERVAL);
		}

		/**
		 * Reset the retry counter.
		 */
		public void reset()
		{
			mRetry = 0;
		}
	}
}
