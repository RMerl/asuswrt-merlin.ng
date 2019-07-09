/*
 * Copyright (C) 2017 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version. See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

package org.strongswan.android.ui;

import android.Manifest;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.ListFragment;
import android.support.v4.app.LoaderManager;
import android.support.v4.content.AsyncTaskLoader;
import android.support.v4.content.Loader;
import android.support.v7.widget.SearchView;
import android.text.TextUtils;
import android.util.Pair;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.Filter;
import android.widget.ListView;

import org.strongswan.android.R;
import org.strongswan.android.data.VpnProfileDataSource;
import org.strongswan.android.ui.adapter.SelectedApplicationEntry;
import org.strongswan.android.ui.adapter.SelectedApplicationsAdapter;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.SortedSet;
import java.util.TreeSet;

public class SelectedApplicationsListFragment extends ListFragment implements LoaderManager.LoaderCallbacks<Pair<List<SelectedApplicationEntry>, List<String>>>, SearchView.OnQueryTextListener
{
	private SelectedApplicationsAdapter mAdapter;
	private SortedSet<String> mSelection;

	@Override
	public void onActivityCreated(@Nullable Bundle savedInstanceState)
	{
		super.onActivityCreated(savedInstanceState);
		setHasOptionsMenu(true);

		getListView().setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);

		mAdapter = new SelectedApplicationsAdapter(getActivity());
		setListAdapter(mAdapter);
		setListShown(false);

		ArrayList<String> selection;
		if (savedInstanceState == null)
		{
			selection = getActivity().getIntent().getStringArrayListExtra(VpnProfileDataSource.KEY_SELECTED_APPS_LIST);
		}
		else
		{
			selection = savedInstanceState.getStringArrayList(VpnProfileDataSource.KEY_SELECTED_APPS_LIST);
		}
		mSelection = new TreeSet<>(selection);

		getLoaderManager().initLoader(0, null, this);
	}

	@Override
	public void onSaveInstanceState(Bundle outState)
	{
		super.onSaveInstanceState(outState);
		outState.putStringArrayList(VpnProfileDataSource.KEY_SELECTED_APPS_LIST, new ArrayList<>(mSelection));
	}

	/**
	 * Returns the package names of all selected apps
	 */
	public ArrayList<String> getSelectedApplications()
	{
		return new ArrayList<>(mSelection);
	}

	/**
	 * Track check state as ListView is unable to do that when using filters
	 */
	@Override
	public void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l, v, position, id);
		SelectedApplicationEntry item = (SelectedApplicationEntry)getListView().getItemAtPosition(position);
		item.setSelected(!item.isSelected());
		if (item.isSelected())
		{
			mSelection.add(item.getInfo().packageName);
		}
		else
		{
			mSelection.remove(item.getInfo().packageName);
		}
	}

	/**
	 * Manually set the check state as ListView is unable to track that when using filters
	 */
	private void setCheckState()
	{
		for (int i = 0; i < getListView().getCount(); i++)
		{
			SelectedApplicationEntry item = mAdapter.getItem(i);
			getListView().setItemChecked(i, item.isSelected());
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

		super.onCreateOptionsMenu(menu, inflater);
	}

	@Override
	public Loader<Pair<List<SelectedApplicationEntry>, List<String>>> onCreateLoader(int id, Bundle args)
	{
		return new InstalledPackagesLoader(getActivity(), mSelection);
	}

	@Override
	public void onLoadFinished(Loader<Pair<List<SelectedApplicationEntry>, List<String>>> loader, Pair<List<SelectedApplicationEntry>, List<String>> data)
	{
		mAdapter.setData(data.first);
		mSelection.removeAll(data.second);
		setCheckState();

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
	public void onLoaderReset(Loader<Pair<List<SelectedApplicationEntry>, List<String>>> loader)
	{
		mAdapter.setData(null);
	}

	@Override
	public boolean onQueryTextSubmit(String query)
	{
		return true;
	}

	@Override
	public boolean onQueryTextChange(String newText)
	{
		String search = TextUtils.isEmpty(newText) ? null : newText;
		mAdapter.getFilter().filter(search, new Filter.FilterListener()
		{
			@Override
			public void onFilterComplete(int count)
			{
				setCheckState();
			}
		});
		return true;
	}

	public static class InstalledPackagesLoader extends AsyncTaskLoader<Pair<List<SelectedApplicationEntry>, List<String>>>
	{
		private final PackageManager mPackageManager;
		private final SortedSet<String> mSelection;
		private Pair<List<SelectedApplicationEntry>, List<String>> mData;

		public InstalledPackagesLoader(Context context, SortedSet<String> selection)
		{
			super(context);
			mPackageManager = context.getPackageManager();
			mSelection = selection;
		}

		@Override
		public Pair<List<SelectedApplicationEntry>, List<String>> loadInBackground()
		{
			List<SelectedApplicationEntry> apps = new ArrayList<>();
			SortedSet<String> seen = new TreeSet<>();
			for (ApplicationInfo info : mPackageManager.getInstalledApplications(PackageManager.GET_META_DATA))
			{
				/* skip apps that can't access the network anyway */
				if (mPackageManager.checkPermission(Manifest.permission.INTERNET, info.packageName) == PackageManager.PERMISSION_GRANTED)
				{
					SelectedApplicationEntry entry = new SelectedApplicationEntry(mPackageManager, info);
					entry.setSelected(mSelection.contains(info.packageName));
					apps.add(entry);
					seen.add(info.packageName);
				}
			}
			Collections.sort(apps);
			/* check for selected packages that don't exist anymore */
			List<String> missing = new ArrayList<>();
			for (String pkg : mSelection)
			{
				if (!seen.contains(pkg))
				{
					missing.add(pkg);
				}
			}
			return new Pair<>(apps, missing);
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
		public void deliverResult(Pair<List<SelectedApplicationEntry>, List<String>> data)
		{
			if (isReset())
			{
				return;
			}
			mData = data;
			if (isStarted())
			{	/* if it is started we deliver the data directly,
				 * otherwise this is handled in onStartLoading */
				super.deliverResult(data);
			}
		}

		@Override
		protected void onReset()
		{
			mData = null;
			super.onReset();
		}
	}
}
