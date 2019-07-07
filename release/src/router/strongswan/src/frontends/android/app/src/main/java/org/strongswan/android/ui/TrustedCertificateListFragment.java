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

package org.strongswan.android.ui;

import android.content.Context;
import android.os.Bundle;
import android.support.v4.app.ListFragment;
import android.support.v4.app.LoaderManager.LoaderCallbacks;
import android.support.v4.content.AsyncTaskLoader;
import android.support.v4.content.Loader;
import android.text.TextUtils;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.ListView;
import android.widget.SearchView;
import android.widget.SearchView.OnQueryTextListener;

import org.strongswan.android.R;
import org.strongswan.android.logic.TrustedCertificateManager;
import org.strongswan.android.logic.TrustedCertificateManager.TrustedCertificateSource;
import org.strongswan.android.security.TrustedCertificateEntry;
import org.strongswan.android.ui.adapter.TrustedCertificateAdapter;

import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Hashtable;
import java.util.List;
import java.util.Map.Entry;
import java.util.Observable;
import java.util.Observer;

public class TrustedCertificateListFragment extends ListFragment implements LoaderCallbacks<List<TrustedCertificateEntry>>, OnQueryTextListener
{
	public static final String EXTRA_CERTIFICATE_SOURCE = "certificate_source";
	private OnTrustedCertificateSelectedListener mListener;
	private TrustedCertificateAdapter mAdapter;
	private TrustedCertificateSource mSource = TrustedCertificateSource.SYSTEM;

	/**
	 * The activity containing this fragment should implement this interface
	 */
	public interface OnTrustedCertificateSelectedListener
	{
		public void onTrustedCertificateSelected(TrustedCertificateEntry selected);
	}

	@Override
	public void onActivityCreated(Bundle savedInstanceState)
	{
		super.onActivityCreated(savedInstanceState);
		setHasOptionsMenu(true);

		setEmptyText(getString(R.string.no_certificates));

		mAdapter = new TrustedCertificateAdapter(getActivity());
		setListAdapter(mAdapter);

		setListShown(false);

		Bundle arguments = getArguments();
		if (arguments != null)
		{
			mSource = (TrustedCertificateSource)arguments.getSerializable(EXTRA_CERTIFICATE_SOURCE);
		}

		getLoaderManager().initLoader(0, null, this);
	}

	@Override
	public void onDestroy()
	{
		super.onDestroy();
	}

	@Override
	public void onAttach(Context context)
	{
		super.onAttach(context);

		if (context instanceof OnTrustedCertificateSelectedListener)
		{
			mListener = (OnTrustedCertificateSelectedListener)context;
		}
	}

	@Override
	public void onCreateOptionsMenu(Menu menu, MenuInflater inflater)
	{
		MenuItem item = menu.add(R.string.search);
		item.setIcon(android.R.drawable.ic_menu_search);
		item.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);

		SearchView sv = new SearchView(getActivity());
		sv.setOnQueryTextListener(this);
		item.setActionView(sv);
	}

	@Override
	public boolean onQueryTextSubmit(String query)
	{	/* already handled when the text changes */
		return true;
	}

	@Override
	public boolean onQueryTextChange(String newText)
	{
		String search = TextUtils.isEmpty(newText) ? null : newText;
		mAdapter.getFilter().filter(search);
		return true;
	}

	@Override
	public Loader<List<TrustedCertificateEntry>> onCreateLoader(int id, Bundle args)
	{	/* we don't need the id as we have only one loader */
		return new CertificateListLoader(getActivity(), mSource);
	}

	@Override
	public void onLoadFinished(Loader<List<TrustedCertificateEntry>> loader, List<TrustedCertificateEntry> data)
	{
		mAdapter.setData(data);

		if (isResumed())
		{
			setListShown(true);
		}
		else
		{
			setListShownNoAnimation(true);
		}
	}

	@Override
	public void onLoaderReset(Loader<List<TrustedCertificateEntry>> loader)
	{
		mAdapter.setData(null);
	}

	@Override
	public void onListItemClick(ListView l, View v, int position, long id)
	{
		if (mListener != null)
		{
			mListener.onTrustedCertificateSelected(mAdapter.getItem(position));
		}
	}

	public static class CertificateListLoader extends AsyncTaskLoader<List<TrustedCertificateEntry>>
	{
		private List<TrustedCertificateEntry> mData;
		private final TrustedCertificateSource mSource;
		private TrustedCertificateManagerObserver mObserver;

		public CertificateListLoader(Context context, TrustedCertificateSource source)
		{
			super(context);
			mSource = source;
		}

		@Override
		public List<TrustedCertificateEntry> loadInBackground()
		{
			TrustedCertificateManager certman = TrustedCertificateManager.getInstance().load();
			Hashtable<String, X509Certificate> certificates = certman.getCACertificates(mSource);
			List<TrustedCertificateEntry> selected;

			selected = new ArrayList<TrustedCertificateEntry>();
			for (Entry<String, X509Certificate> entry : certificates.entrySet())
			{
				selected.add(new TrustedCertificateEntry(entry.getKey(), entry.getValue()));
			}
			Collections.sort(selected);
			return selected;
		}

		@Override
		protected void onStartLoading()
		{
			if (mData != null)
			{	/* if we have data ready, deliver it directly */
				deliverResult(mData);
			}
			if (takeContentChanged() || mData == null)
			{
				forceLoad();
			}
		}

		@Override
		public void deliverResult(List<TrustedCertificateEntry> data)
		{
			if (isReset())
			{
				return;
			}
			mData = data;
			if (isStarted())
			{	/* if it is started we deliver the data directly,
				 * otherwise this is handled in onStartLoading */
				if (mObserver == null)
				{
					mObserver = new TrustedCertificateManagerObserver();
					TrustedCertificateManager.getInstance().addObserver(mObserver);
				}
				super.deliverResult(data);
			}
		}

		@Override
		protected void onReset()
		{
			if (mObserver != null)
			{
				TrustedCertificateManager.getInstance().deleteObserver(mObserver);
				mObserver = null;
			}
			mData = null;
			super.onReset();
		}

		@Override
		protected void onAbandon()
		{
			if (mObserver != null)
			{
				TrustedCertificateManager.getInstance().deleteObserver(mObserver);
				mObserver = null;
			}
		}

		private class TrustedCertificateManagerObserver implements Observer
		{
			private ForceLoadContentObserver mContentObserver = new ForceLoadContentObserver();

			@Override
			public void update(Observable observable, Object data)
			{
				mContentObserver.onChange(false);
			}
		}
	}
}
