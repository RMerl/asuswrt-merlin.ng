/*
 * Copyright (C) 2014 Tobias Brunner
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

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.Dialog;
import android.content.ActivityNotFoundException;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.app.AppCompatDialogFragment;
import android.widget.Toast;

import org.strongswan.android.R;
import org.strongswan.android.data.VpnProfileDataSource;
import org.strongswan.android.logic.TrustedCertificateManager;

import java.io.FileNotFoundException;
import java.io.InputStream;
import java.security.KeyStore;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;

public class TrustedCertificateImportActivity extends AppCompatActivity
{
	private static final int OPEN_DOCUMENT = 0;
	private static final String DIALOG_TAG = "Dialog";
	private Uri mCertificateUri;

	@TargetApi(Build.VERSION_CODES.KITKAT)
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		if (savedInstanceState != null)
		{	/* do nothing when we are restoring */
			return;
		}

		Intent intent = getIntent();
		String action = intent.getAction();
		if (Intent.ACTION_VIEW.equals(action))
		{
			importCertificate(intent.getData());
		}
		else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT)
		{
			Intent openIntent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
			openIntent.setType("*/*");
			try
			{
				startActivityForResult(openIntent, OPEN_DOCUMENT);
			}
			catch (ActivityNotFoundException e)
			{	/* some devices are unable to browse for files */
				finish();
				return;
			}
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		super.onActivityResult(requestCode, resultCode, data);
		switch (requestCode)
		{
			case OPEN_DOCUMENT:
				if (resultCode == Activity.RESULT_OK && data != null)
				{
					mCertificateUri = data.getData();
					return;
				}
				finish();
				return;
		}
	}

	@Override
	protected void onPostResume()
	{
		super.onPostResume();
		if (mCertificateUri != null)
		{
			importCertificate(mCertificateUri);
			mCertificateUri = null;
		}
	}

	/**
	 * Import the file pointed to by the given URI as a certificate.
	 *
	 * @param uri
	 */
	private void importCertificate(Uri uri)
	{
		X509Certificate certificate = parseCertificate(uri);
		if (certificate == null)
		{
			Toast.makeText(this, R.string.cert_import_failed, Toast.LENGTH_LONG).show();
			finish();
			return;
		}
		/* Ask the user whether to import the certificate.  This is particularly
		 * necessary because the import activity can be triggered by any app on
		 * the system.  Also, if our app is the only one that is registered to
		 * open certificate files by MIME type the user would have no idea really
		 * where the file was imported just by reading the Toast we display. */
		ConfirmImportDialog dialog = new ConfirmImportDialog();
		Bundle args = new Bundle();
		args.putSerializable(VpnProfileDataSource.KEY_CERTIFICATE, certificate);
		dialog.setArguments(args);
		FragmentTransaction ft = getSupportFragmentManager().beginTransaction();
		ft.add(dialog, DIALOG_TAG);
		ft.commit();
	}

	/**
	 * Load the file from the given URI and try to parse it as X.509 certificate.
	 *
	 * @param uri
	 * @return certificate or null
	 */
	private X509Certificate parseCertificate(Uri uri)
	{
		X509Certificate certificate = null;
		try
		{
			CertificateFactory factory = CertificateFactory.getInstance("X.509");
			InputStream in = getContentResolver().openInputStream(uri);
			certificate = (X509Certificate)factory.generateCertificate(in);
			/* we don't check whether it's actually a CA certificate or not */
		}
		catch (CertificateException e)
		{
			e.printStackTrace();
		}
		catch (FileNotFoundException e)
		{
			e.printStackTrace();
		}
		return certificate;
	}


	/**
	 * Try to store the given certificate in the KeyStore.
	 *
	 * @param certificate
	 * @return whether it was successfully stored
	 */
	private boolean storeCertificate(X509Certificate certificate)
	{
		try
		{
			KeyStore store = KeyStore.getInstance("LocalCertificateStore");
			store.load(null, null);
			store.setCertificateEntry(null, certificate);
			TrustedCertificateManager.getInstance().reset();
			return true;
		}
		catch (Exception e)
		{
			e.printStackTrace();
			return false;
		}
	}

	/**
	 * Class that displays a confirmation dialog when a certificate should get
	 * imported. If the user confirms the import we try to store it.
	 */
	public static class ConfirmImportDialog extends AppCompatDialogFragment
	{
		@Override
		public Dialog onCreateDialog(Bundle savedInstanceState)
		{
			final X509Certificate certificate;

			certificate = (X509Certificate)getArguments().getSerializable(VpnProfileDataSource.KEY_CERTIFICATE);

			return new AlertDialog.Builder(getActivity())
				.setIcon(R.drawable.ic_launcher)
				.setTitle(R.string.import_certificate)
				.setMessage(certificate.getSubjectDN().toString())
				.setPositiveButton(R.string.import_certificate, new DialogInterface.OnClickListener()
				{
					@Override
					public void onClick(DialogInterface dialog, int whichButton)
					{
						TrustedCertificateImportActivity activity = (TrustedCertificateImportActivity)getActivity();
						if (activity.storeCertificate(certificate))
						{
							Toast.makeText(getActivity(), R.string.cert_imported_successfully, Toast.LENGTH_LONG).show();
							getActivity().setResult(Activity.RESULT_OK);
						}
						else
						{
							Toast.makeText(getActivity(), R.string.cert_import_failed, Toast.LENGTH_LONG).show();
						}
						getActivity().finish();
					}
				})
				.setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener()
				{
					@Override
					public void onClick(DialogInterface dialog, int which)
					{
						getActivity().finish();
					}
				}).create();
		}

		@Override
		public void onCancel(DialogInterface dialog)
		{
			getActivity().finish();
		}
	}
}
