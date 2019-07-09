/*
 * Copyright (C) 2012-2018 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
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

import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.app.AppCompatDialogFragment;
import android.text.format.Formatter;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

import org.strongswan.android.R;
import org.strongswan.android.data.VpnProfile;
import org.strongswan.android.logic.TrustedCertificateManager;
import org.strongswan.android.ui.VpnProfileListFragment.OnVpnProfileSelectedListener;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity implements OnVpnProfileSelectedListener
{
	public static final String CONTACT_EMAIL = "android@strongswan.org";
	public static final String EXTRA_CRL_LIST = "org.strongswan.android.CRL_LIST";

	/**
	 * Use "bring your own device" (BYOD) features
	 */
	public static final boolean USE_BYOD = true;

	private static final String DIALOG_TAG = "Dialog";

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		ActionBar bar = getSupportActionBar();
		bar.setDisplayShowHomeEnabled(true);
		bar.setDisplayShowTitleEnabled(false);
		bar.setIcon(R.drawable.ic_launcher);

		/* load CA certificates in a background task */
		new LoadCertificatesTask().executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu)
	{
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT)
		{
			menu.removeItem(R.id.menu_import_profile);
		}
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId())
		{
			case R.id.menu_import_profile:
				Intent intent = new Intent(this, VpnProfileImportActivity.class);
				startActivity(intent);
				return true;
			case R.id.menu_manage_certs:
				Intent certIntent = new Intent(this, TrustedCertificatesActivity.class);
				startActivity(certIntent);
				return true;
			case R.id.menu_crl_cache:
				clearCRLs();
				return true;
			case R.id.menu_show_log:
				Intent logIntent = new Intent(this, LogActivity.class);
				startActivity(logIntent);
				return true;
			case R.id.menu_settings:
				Intent settingsIntent = new Intent(this, SettingsActivity.class);
				startActivity(settingsIntent);
				return true;
			default:
				return super.onOptionsItemSelected(item);
		}
	}

	@Override
	public void onVpnProfileSelected(VpnProfile profile)
	{
		Intent intent = new Intent(this, VpnProfileControlActivity.class);
		intent.setAction(VpnProfileControlActivity.START_PROFILE);
		intent.putExtra(VpnProfileControlActivity.EXTRA_VPN_PROFILE_ID, profile.getUUID().toString());
		startActivity(intent);
	}

	/**
	 * Ask the user whether to clear the CRL cache.
	 */
	private void clearCRLs()
	{
		final String FILE_PREFIX = "crl-";
		ArrayList<String> list = new ArrayList<>();

		for (String file : fileList())
		{
			if (file.startsWith(FILE_PREFIX))
			{
				list.add(file);
			}
		}
		if (list.size() == 0)
		{
			Toast.makeText(this, R.string.clear_crl_cache_msg_none, Toast.LENGTH_SHORT).show();
			return;
		}
		removeFragmentByTag(DIALOG_TAG);

		Bundle args = new Bundle();
		args.putStringArrayList(EXTRA_CRL_LIST, list);

		CRLCacheDialog dialog = new CRLCacheDialog();
		dialog.setArguments(args);
		dialog.show(this.getSupportFragmentManager(), DIALOG_TAG);
	}

	/**
	 * Class that loads the cached CA certificates.
	 */
	private class LoadCertificatesTask extends AsyncTask<Void, Void, TrustedCertificateManager>
	{
		@Override
		protected TrustedCertificateManager doInBackground(Void... params)
		{
			return TrustedCertificateManager.getInstance().load();
		}
	}

	/**
	 * Dismiss dialog if shown
	 */
	public void removeFragmentByTag(String tag)
	{
		FragmentManager fm = getSupportFragmentManager();
		Fragment login = fm.findFragmentByTag(tag);
		if (login != null)
		{
			FragmentTransaction ft = fm.beginTransaction();
			ft.remove(login);
			ft.commit();
		}
	}

	/**
	 * Confirmation dialog to clear CRL cache
	 */
	public static class CRLCacheDialog extends AppCompatDialogFragment
	{
		@Override
		public Dialog onCreateDialog(Bundle savedInstanceState)
		{
			final List<String> list = getArguments().getStringArrayList(EXTRA_CRL_LIST);
			String size;
			long s = 0;

			for (String file : list)
			{
				File crl = getActivity().getFileStreamPath(file);
				s += crl.length();
			}
			size = Formatter.formatFileSize(getActivity(), s);

			AlertDialog.Builder builder = new AlertDialog.Builder(getActivity())
					.setTitle(R.string.clear_crl_cache_title)
					.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener()
					{
						@Override
						public void onClick(DialogInterface dialog, int which)
						{
							dismiss();
						}
					})
					.setPositiveButton(R.string.clear, new DialogInterface.OnClickListener()
					{
						@Override
						public void onClick(DialogInterface dialog, int whichButton)
						{
							for (String file : list)
							{
								getActivity().deleteFile(file);
							}
						}
					});
			builder.setMessage(getActivity().getResources().getQuantityString(R.plurals.clear_crl_cache_msg, list.size(), list.size(), size));
			return builder.create();
		}
	}
}
