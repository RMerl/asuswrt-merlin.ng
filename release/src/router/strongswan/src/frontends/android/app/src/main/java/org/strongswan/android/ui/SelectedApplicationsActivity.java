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

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.FragmentManager;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.view.MenuItem;

import org.strongswan.android.data.VpnProfileDataSource;

public class SelectedApplicationsActivity extends AppCompatActivity
{
	private static final String LIST_TAG = "ApplicationList";
	private SelectedApplicationsListFragment mApps;

	@Override
	protected void onCreate(@Nullable Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		ActionBar actionBar = getSupportActionBar();
		actionBar.setDisplayHomeAsUpEnabled(true);

		FragmentManager fm = getSupportFragmentManager();
		mApps = (SelectedApplicationsListFragment)fm.findFragmentByTag(LIST_TAG);
		if (mApps == null)
		{
			mApps = new SelectedApplicationsListFragment();
			fm.beginTransaction().add(android.R.id.content, mApps, LIST_TAG).commit();
		}
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId())
		{
			case android.R.id.home:
				prepareResult();
				finish();
				return true;
		}
		return super.onOptionsItemSelected(item);
	}

	@Override
	public void onBackPressed()
	{
		prepareResult();
		super.onBackPressed();
	}

	private void prepareResult()
	{
		Intent data = new Intent();
		data.putExtra(VpnProfileDataSource.KEY_SELECTED_APPS_LIST, mApps.getSelectedApplications());
		setResult(RESULT_OK, data);
	}
}
