/*
 * Copyright (C) 2012-2018 Tobias Brunner
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
import android.os.FileObserver;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import org.strongswan.android.R;
import org.strongswan.android.logic.CharonVpnService;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.StringReader;
import java.util.ArrayList;

public class LogFragment extends Fragment
{
	private static String SCROLL_POSITION = "SCROLL_POSITION";
	private String mLogFilePath;
	private Handler mLogHandler;
	private ListView mLog;
	private LogAdapter mLogAdapter;
	private FileObserver mDirectoryObserver;
	private int mScrollPosition;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		mLogFilePath = getActivity().getFilesDir() + File.separator + CharonVpnService.LOG_FILE;

		mLogHandler = new Handler();

		mDirectoryObserver = new LogDirectoryObserver(getActivity().getFilesDir().getAbsolutePath());
	}

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState)
	{
		View view = inflater.inflate(R.layout.log_fragment, null);

		mLogAdapter = new LogAdapter(getActivity());
		mLog = view.findViewById(R.id.log);
		mLog.setAdapter(mLogAdapter);

		mScrollPosition = -1;
		if (savedInstanceState != null)
		{
			mScrollPosition = savedInstanceState.getInt(SCROLL_POSITION, mScrollPosition);
		}
		return view;
	}

	@Override
	public void onSaveInstanceState(Bundle outState)
	{
		super.onSaveInstanceState(outState);

		if (mLog.getLastVisiblePosition() == (mLogAdapter.getCount() - 1))
		{
			outState.putInt(SCROLL_POSITION, -1);
		}
		else
		{
			outState.putInt(SCROLL_POSITION, mLog.getFirstVisiblePosition());
		}
	}

	@Override
	public void onStart()
	{
		super.onStart();
		mLogAdapter.restart();
		mDirectoryObserver.startWatching();
	}

	@Override
	public void onStop()
	{
		super.onStop();
		mDirectoryObserver.stopWatching();
		mLogAdapter.stop();
	}

	private class LogAdapter extends ArrayAdapter<String> implements Runnable
	{
		private BufferedReader mReader;
		private Thread mThread;
		private volatile boolean mRunning;

		public LogAdapter(@NonNull Context context)
		{
			super(context, R.layout.log_list_item, R.id.log_line);
		}

		public void restart()
		{
			if (mRunning)
			{
				stop();
			}

			clear();

			try
			{
				mReader = new BufferedReader(new FileReader(mLogFilePath));
			}
			catch (FileNotFoundException e)
			{
				mReader = new BufferedReader(new StringReader(""));
			}
			mRunning = true;
			mThread = new Thread(this);
			mThread.start();
		}

		public void stop()
		{
			try
			{
				mRunning = false;
				mThread.interrupt();
				mThread.join();
			}
			catch (InterruptedException e)
			{
			}
		}

		private void logLines(final ArrayList<String> lines)
		{
			mLogHandler.post(() -> {
				boolean scroll = getCount() == 0;
				setNotifyOnChange(false);
				for (String line : lines)
				{
					if (getResources().getConfiguration().screenWidthDp < 600)
					{	/* strip off prefix (month=3, day=2, time=8, thread=2, spaces=3) */
						line = line.length() > 18 ? line.substring(18) : line;
					}
					add(line);
				}
				notifyDataSetChanged();
				if (scroll)
				{	/* scroll to the bottom or saved position after adding the first batch */
					mLogHandler.post(() -> mLog.setSelection(mScrollPosition == -1 ? getCount() - 1 : mScrollPosition));
				}
			});
		}

		@Override
		public void run()
		{
			ArrayList<String> lines = null;

			while (mRunning)
			{
				try
				{	/* this works as long as the file is not truncated */
					String line = mReader.readLine();
					if (line == null)
					{
						if (lines != null)
						{
							logLines(lines);
							lines = null;
						}
						/* wait until there is more to log */
						Thread.sleep(1000);
					}
					else
					{
						if (lines == null)
						{
							lines = new ArrayList<>();
						}
						lines.add(line);
					}
				}
				catch (Exception e)
				{
					break;
				}
			}
			if (lines != null)
			{
				logLines(lines);
			}
		}
	}

	/**
	 * FileObserver that checks for changes regarding the log file. Since charon
	 * truncates it (for which there is no explicit event) we check for any modification
	 * to the file, keep track of the file size and reopen it if it got smaller.
	 */
	private class LogDirectoryObserver extends FileObserver
	{
		private final File mFile;
		private long mSize;

		public LogDirectoryObserver(String path)
		{
			super(path, FileObserver.CREATE | FileObserver.MODIFY | FileObserver.DELETE);
			mFile = new File(mLogFilePath);
			mSize = mFile.length();
		}

		@Override
		public void onEvent(int event, String path)
		{
			if (path == null || !path.equals(CharonVpnService.LOG_FILE))
			{
				return;
			}
			switch (event)
			{	/* even though we only subscribed for these we check them,
				 * as strange events are sometimes received */
				case FileObserver.CREATE:
				case FileObserver.DELETE:
					restartLogReader();
					break;
				case FileObserver.MODIFY:
					/* if the size got smaller reopen the log file, as it was probably truncated */
					long size = mFile.length();
					if (size < mSize)
					{
						restartLogReader();
					}
					mSize = size;
					break;
			}
		}

		private void restartLogReader()
		{
			/* we are called from a separate thread, so we use the handler */
			mLogHandler.post(new Runnable() {
				@Override
				public void run()
				{
					mLogAdapter.restart();
				}
			});
		}
	}
}
