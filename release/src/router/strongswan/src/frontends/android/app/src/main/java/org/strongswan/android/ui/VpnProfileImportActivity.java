/*
 * Copyright (C) 2016-2018 Tobias Brunner
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

import android.app.Activity;
import android.app.LoaderManager;
import android.content.ActivityNotFoundException;
import android.content.AsyncTaskLoader;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.Loader;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.security.KeyChain;
import android.security.KeyChainAliasCallback;
import android.security.KeyChainException;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.util.Base64;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.strongswan.android.R;
import org.strongswan.android.data.VpnProfile;
import org.strongswan.android.data.VpnProfile.SelectedAppsHandling;
import org.strongswan.android.data.VpnProfileDataSource;
import org.strongswan.android.data.VpnType;
import org.strongswan.android.data.VpnType.VpnTypeFeature;
import org.strongswan.android.logic.TrustedCertificateManager;
import org.strongswan.android.security.TrustedCertificateEntry;
import org.strongswan.android.ui.widget.TextInputLayoutHelper;
import org.strongswan.android.utils.Constants;
import org.strongswan.android.utils.IPRangeSet;
import org.strongswan.android.utils.Utils;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.UnknownHostException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.UUID;

import javax.net.ssl.SSLHandshakeException;

public class VpnProfileImportActivity extends AppCompatActivity
{
	private static final String PKCS12_INSTALLED = "PKCS12_INSTALLED";
	private static final String PROFILE_URI = "PROFILE_URI";
	private static final int INSTALL_PKCS12 = 0;
	private static final int OPEN_DOCUMENT = 1;
	private static final int PROFILE_LOADER = 0;
	private static final int USER_CERT_LOADER = 1;

	private VpnProfileDataSource mDataSource;
	private ParsedVpnProfile mProfile;
	private VpnProfile mExisting;
	private TrustedCertificateEntry mCertEntry;
	private TrustedCertificateEntry mUserCertEntry;
	private String mUserCertLoading;
	private boolean mHideImport;
	private android.support.v4.widget.ContentLoadingProgressBar mProgressBar;
	private TextView mExistsWarning;
	private ViewGroup mBasicDataGroup;
	private TextView mName;
	private TextView mGateway;
	private TextView mSelectVpnType;
	private ViewGroup mUsernamePassword;
	private EditText mUsername;
	private TextInputLayoutHelper mUsernameWrap;
	private EditText mPassword;
	private ViewGroup mUserCertificate;
	private RelativeLayout mSelectUserCert;
	private Button mImportUserCert;
	private ViewGroup mRemoteCertificate;
	private RelativeLayout mRemoteCert;

	private LoaderManager.LoaderCallbacks<ProfileLoadResult> mProfileLoaderCallbacks = new LoaderManager.LoaderCallbacks<ProfileLoadResult>()
	{
		@Override
		public Loader<ProfileLoadResult> onCreateLoader(int id, Bundle args)
		{
			return new ProfileLoader(VpnProfileImportActivity.this, (Uri)args.getParcelable(PROFILE_URI));
		}

		@Override
		public void onLoadFinished(Loader<ProfileLoadResult> loader, ProfileLoadResult data)
		{
			handleProfile(data);
		}

		@Override
		public void onLoaderReset(Loader<ProfileLoadResult> loader)
		{

		}
	};

	private LoaderManager.LoaderCallbacks<TrustedCertificateEntry> mUserCertificateLoaderCallbacks = new LoaderManager.LoaderCallbacks<TrustedCertificateEntry>()
	{
		@Override
		public Loader<TrustedCertificateEntry> onCreateLoader(int id, Bundle args)
		{
			return new UserCertificateLoader(VpnProfileImportActivity.this, mUserCertLoading);
		}

		@Override
		public void onLoadFinished(Loader<TrustedCertificateEntry> loader, TrustedCertificateEntry data)
		{
			handleUserCertificate(data);
		}

		@Override
		public void onLoaderReset(Loader<TrustedCertificateEntry> loader)
		{

		}
	};

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		getSupportActionBar().setHomeAsUpIndicator(R.drawable.ic_close_white_24dp);
		getSupportActionBar().setDisplayHomeAsUpEnabled(true);

		mDataSource = new VpnProfileDataSource(this);
		mDataSource.open();

		setContentView(R.layout.profile_import_view);

		mProgressBar = findViewById(R.id.progress_bar);
		mExistsWarning = (TextView)findViewById(R.id.exists_warning);
		mBasicDataGroup = (ViewGroup)findViewById(R.id.basic_data_group);
		mName = (TextView)findViewById(R.id.name);
		mGateway = (TextView)findViewById(R.id.gateway);
		mSelectVpnType = (TextView)findViewById(R.id.vpn_type);

		mUsernamePassword = (ViewGroup)findViewById(R.id.username_password_group);
		mUsername = (EditText)findViewById(R.id.username);
		mUsernameWrap = (TextInputLayoutHelper) findViewById(R.id.username_wrap);
		mPassword = (EditText)findViewById(R.id.password);

		mUserCertificate = (ViewGroup)findViewById(R.id.user_certificate_group);
		mSelectUserCert = (RelativeLayout)findViewById(R.id.select_user_certificate);
		mImportUserCert = (Button)findViewById(R.id.import_user_certificate);

		mRemoteCertificate = (ViewGroup)findViewById(R.id.remote_certificate_group);
		mRemoteCert = (RelativeLayout)findViewById(R.id.remote_certificate);

		mExistsWarning.setVisibility(View.GONE);
		mBasicDataGroup.setVisibility(View.GONE);
		mUsernamePassword.setVisibility(View.GONE);
		mUserCertificate.setVisibility(View.GONE);
		mRemoteCertificate.setVisibility(View.GONE);

		mSelectUserCert.setOnClickListener(new SelectUserCertOnClickListener());
		mImportUserCert.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v)
			{
				Intent intent = KeyChain.createInstallIntent();
				intent.putExtra(KeyChain.EXTRA_NAME, getString(R.string.profile_cert_alias, mProfile.getName()));
				intent.putExtra(KeyChain.EXTRA_PKCS12, mProfile.PKCS12);
				startActivityForResult(intent, INSTALL_PKCS12);
			}
		});

		Intent intent = getIntent();
		String action = intent.getAction();
		if (Intent.ACTION_VIEW.equals(action))
		{
			loadProfile(getIntent().getData());
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

		if (savedInstanceState != null)
		{
			mUserCertLoading = savedInstanceState.getString(VpnProfileDataSource.KEY_USER_CERTIFICATE);
			if (mUserCertLoading != null)
			{
				getLoaderManager().initLoader(USER_CERT_LOADER, null, mUserCertificateLoaderCallbacks);
			}
			mImportUserCert.setEnabled(!savedInstanceState.getBoolean(PKCS12_INSTALLED));
		}
	}

	@Override
	protected void onDestroy()
	{
		super.onDestroy();
		mDataSource.close();
	}

	@Override
	protected void onSaveInstanceState(Bundle outState)
	{
		super.onSaveInstanceState(outState);
		if (mUserCertEntry != null)
		{
			outState.putString(VpnProfileDataSource.KEY_USER_CERTIFICATE, mUserCertEntry.getAlias());
		}
		outState.putBoolean(PKCS12_INSTALLED, !mImportUserCert.isEnabled());
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		MenuInflater inflater = getMenuInflater();
		inflater.inflate(R.menu.profile_import, menu);
		if (mHideImport)
		{
			MenuItem item = menu.findItem(R.id.menu_accept);
			item.setVisible(false);
		}
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId())
		{
			case android.R.id.home:
				finish();
				return true;
			case R.id.menu_accept:
				saveProfile();
				return true;
			default:
				return super.onOptionsItemSelected(item);
		}
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		super.onActivityResult(requestCode, resultCode, data);
		switch (requestCode)
		{
			case INSTALL_PKCS12:
				if (resultCode == Activity.RESULT_OK)
				{	/* no need to import twice */
					mImportUserCert.setEnabled(false);
					mSelectUserCert.performClick();
				}
				break;
			case OPEN_DOCUMENT:
				if (resultCode == Activity.RESULT_OK && data != null)
				{
					loadProfile(data.getData());
					return;
				}
				finish();
				break;
		}
	}

	private void loadProfile(Uri uri)
	{
		mProgressBar.show();

		Bundle args = new Bundle();
		args.putParcelable(PROFILE_URI, uri);
		getLoaderManager().initLoader(PROFILE_LOADER, args, mProfileLoaderCallbacks);
	}

	public void handleProfile(ProfileLoadResult data)
	{
		mProgressBar.hide();

		mProfile = null;
		if (data != null && data.ThrownException == null)
		{
			try
			{
				JSONObject obj = new JSONObject(data.Profile);
				mProfile = parseProfile(obj);
			}
			catch (JSONException e)
			{
				mExistsWarning.setVisibility(View.VISIBLE);
				mExistsWarning.setText(e.getLocalizedMessage());
				mHideImport = true;
				invalidateOptionsMenu();
				return;
			}
		}
		if (mProfile == null)
		{
			String error = null;
			if (data.ThrownException != null)
			{
				try
				{
					throw data.ThrownException;
				}
				catch (FileNotFoundException e)
				{
					error = getString(R.string.profile_import_failed_not_found);
				}
				catch (UnknownHostException e)
				{
					error = getString(R.string.profile_import_failed_host);
				}
				catch (SSLHandshakeException e)
				{
					error = getString(R.string.profile_import_failed_tls);
				}
				catch (Exception e)
				{
					e.printStackTrace();
				}
			}
			if (error != null)
			{
				Toast.makeText(this, getString(R.string.profile_import_failed_detail, error), Toast.LENGTH_LONG).show();
			}
			else
			{
				Toast.makeText(this, R.string.profile_import_failed, Toast.LENGTH_LONG).show();
			}
			finish();
			return;
		}
		mExisting = mDataSource.getVpnProfile(mProfile.getUUID());
		mExistsWarning.setVisibility(mExisting != null ? View.VISIBLE : View.GONE);

		mBasicDataGroup.setVisibility(View.VISIBLE);
		mName.setText(mProfile.getName());
		mGateway.setText(mProfile.getGateway());
		mSelectVpnType.setText(getResources().getStringArray(R.array.vpn_types)[mProfile.getVpnType().ordinal()]);

		mUsernamePassword.setVisibility(mProfile.getVpnType().has(VpnTypeFeature.USER_PASS) ? View.VISIBLE : View.GONE);
		if (mProfile.getVpnType().has(VpnTypeFeature.USER_PASS))
		{
			mUsername.setText(mProfile.getUsername());
			if (mProfile.getUsername() != null && !mProfile.getUsername().isEmpty())
			{
				mUsername.setEnabled(false);
			}
		}

		mUserCertificate.setVisibility(mProfile.getVpnType().has(VpnTypeFeature.CERTIFICATE) ? View.VISIBLE : View.GONE);
		mRemoteCertificate.setVisibility(mProfile.Certificate != null ? View.VISIBLE : View.GONE);
		mImportUserCert.setVisibility(mProfile.PKCS12 != null ? View.VISIBLE : View.GONE);

		if (mProfile.getVpnType().has(VpnTypeFeature.CERTIFICATE))
		{	/* try to load an existing certificate with the default name */
			if (mUserCertLoading == null)
			{
				mUserCertLoading = getString(R.string.profile_cert_alias, mProfile.getName());
				getLoaderManager().initLoader(USER_CERT_LOADER, null, mUserCertificateLoaderCallbacks);
			}
			updateUserCertView();
		}

		if (mProfile.Certificate != null)
		{
			try
			{
				CertificateFactory factory = CertificateFactory.getInstance("X.509");
				X509Certificate certificate = (X509Certificate)factory.generateCertificate(new ByteArrayInputStream(mProfile.Certificate));
				KeyStore store = KeyStore.getInstance("LocalCertificateStore");
				store.load(null, null);
				String alias = store.getCertificateAlias(certificate);
				mCertEntry = new TrustedCertificateEntry(alias, certificate);
				((TextView)mRemoteCert.findViewById(android.R.id.text1)).setText(mCertEntry.getSubjectPrimary());
				((TextView)mRemoteCert.findViewById(android.R.id.text2)).setText(mCertEntry.getSubjectSecondary());
			}
			catch (CertificateException | NoSuchAlgorithmException | KeyStoreException | IOException e)
			{
				e.printStackTrace();
				mRemoteCertificate.setVisibility(View.GONE);
			}
		}
	}

	private void handleUserCertificate(TrustedCertificateEntry data)
	{
		mUserCertEntry = data;
		mUserCertLoading = null;
		updateUserCertView();
	}

	private void updateUserCertView()
	{
		if (mUserCertLoading != null)
		{
			((TextView)mSelectUserCert.findViewById(android.R.id.text1)).setText(mUserCertLoading);
			((TextView)mSelectUserCert.findViewById(android.R.id.text2)).setText(R.string.loading);
		}
		else if (mUserCertEntry != null)
		{	/* clear any errors and set the new data */
			((TextView)mSelectUserCert.findViewById(android.R.id.text1)).setError(null);
			((TextView)mSelectUserCert.findViewById(android.R.id.text1)).setText(mUserCertEntry.getAlias());
			((TextView)mSelectUserCert.findViewById(android.R.id.text2)).setText(mUserCertEntry.getCertificate().getSubjectDN().toString());
		}
		else
		{
			((TextView)mSelectUserCert.findViewById(android.R.id.text1)).setText(R.string.profile_user_select_certificate_label);
			((TextView)mSelectUserCert.findViewById(android.R.id.text2)).setText(R.string.profile_user_select_certificate);
		}
	}

	private ParsedVpnProfile parseProfile(JSONObject obj) throws JSONException
	{
		UUID uuid;
		try
		{
			uuid = UUID.fromString(obj.getString("uuid"));
		}
		catch (IllegalArgumentException e)
		{
			e.printStackTrace();
			return null;
		}
		ParsedVpnProfile profile = new ParsedVpnProfile();
		Integer flags = 0;

		profile.setUUID(uuid);
		profile.setName(obj.getString("name"));
		VpnType type = VpnType.fromIdentifier(obj.getString("type"));
		profile.setVpnType(type);

		JSONObject remote = obj.getJSONObject("remote");
		profile.setGateway(remote.getString("addr"));
		profile.setPort(getInteger(remote, "port", 1, 65535));
		profile.setRemoteId(remote.optString("id", null));
		profile.Certificate = decodeBase64(remote.optString("cert", null));

		if (!remote.optBoolean("certreq", true))
		{
			flags |= VpnProfile.FLAGS_SUPPRESS_CERT_REQS;
		}

		JSONObject revocation = remote.optJSONObject("revocation");
		if (revocation != null)
		{
			if (!revocation.optBoolean("crl", true))
			{
				flags |= VpnProfile.FLAGS_DISABLE_CRL;
			}
			if (!revocation.optBoolean("ocsp", true))
			{
				flags |= VpnProfile.FLAGS_DISABLE_OCSP;
			}
			if (revocation.optBoolean("strict", false))
			{
				flags |= VpnProfile.FLAGS_STRICT_REVOCATION;
			}
		}

		JSONObject local = obj.optJSONObject("local");
		if (local != null)
		{
			if (type.has(VpnTypeFeature.USER_PASS))
			{
				profile.setUsername(local.optString("eap_id", null));
			}

			if (type.has(VpnTypeFeature.CERTIFICATE))
			{
				profile.setLocalId(local.optString("id", null));
				profile.PKCS12 = decodeBase64(local.optString("p12", null));

				if (local.optBoolean("rsa-pss", false))
				{
					flags |= VpnProfile.FLAGS_RSA_PSS;
				}
			}
		}

		profile.setIkeProposal(getProposal(obj, "ike-proposal", true));
		profile.setEspProposal(getProposal(obj, "esp-proposal", false));
		profile.setMTU(getInteger(obj, "mtu", Constants.MTU_MIN, Constants.MTU_MAX));
		profile.setNATKeepAlive(getInteger(obj, "nat-keepalive", Constants.NAT_KEEPALIVE_MIN, Constants.NAT_KEEPALIVE_MAX));
		JSONObject split = obj.optJSONObject("split-tunneling");
		if (split != null)
		{
			String included = getSubnets(split, "subnets");
			profile.setIncludedSubnets(included != null ? included : null);
			String excluded = getSubnets(split, "excluded");
			profile.setExcludedSubnets(excluded != null ? excluded : null);
			int st = 0;
			st |= split.optBoolean("block-ipv4") ? VpnProfile.SPLIT_TUNNELING_BLOCK_IPV4 : 0;
			st |= split.optBoolean("block-ipv6") ? VpnProfile.SPLIT_TUNNELING_BLOCK_IPV6 : 0;
			profile.setSplitTunneling(st == 0 ? null : st);
		}
		/* only one of these can be set, prefer specific apps */
		String selectedApps = getApps(obj.optJSONArray("apps"));
		String excludedApps = getApps(obj.optJSONArray("excluded-apps"));
		if (!TextUtils.isEmpty(selectedApps))
		{
			profile.setSelectedApps(selectedApps);
			profile.setSelectedAppsHandling(SelectedAppsHandling.SELECTED_APPS_ONLY);
		}
		else if (!TextUtils.isEmpty(excludedApps))
		{
			profile.setSelectedApps(excludedApps);
			profile.setSelectedAppsHandling(SelectedAppsHandling.SELECTED_APPS_EXCLUDE);
		}
		profile.setFlags(flags);
		return profile;
	}

	private Integer getInteger(JSONObject obj, String key, int min, int max)
	{
		Integer res = obj.optInt(key);
		return res < min || res > max ? null : res;
	}

	private String getProposal(JSONObject obj, String key, boolean ike) throws JSONException
	{
		String value = obj.optString(key, null);
		if (!TextUtils.isEmpty(value))
		{
			if (!Utils.isProposalValid(ike, value))
			{
				throw new JSONException(getString(R.string.profile_import_failed_value, key));
			}
		}
		return value;
	}

	private String getSubnets(JSONObject split, String key) throws JSONException
	{
		ArrayList<String> subnets = new ArrayList<>();
		JSONArray arr = split.optJSONArray(key);
		if (arr != null)
		{
			for (int i = 0; i < arr.length(); i++)
			{	/* replace all spaces, e.g. in "192.168.1.1 - 192.168.1.10" */
				subnets.add(arr.getString(i).replace(" ", ""));
			}
		}
		else
		{
			String value = split.optString(key, null);
			if (!TextUtils.isEmpty(value))
			{
				subnets.add(value);
			}
		}
		if (subnets.size() > 0)
		{
			String joined = TextUtils.join(" ", subnets);
			IPRangeSet ranges = IPRangeSet.fromString(joined);
			if (ranges == null)
			{
				throw new JSONException(getString(R.string.profile_import_failed_value,
												  "split-tunneling." + key));
			}
			return ranges.toString();
		}
		return null;
	}

	private String getApps(JSONArray arr) throws JSONException
	{
		ArrayList<String> apps = new ArrayList<>();
		if (arr != null)
		{
			for (int i = 0; i < arr.length(); i++)
			{
				apps.add(arr.getString(i));
			}
		}
		return TextUtils.join(" ", apps);
	}

	/**
	 * Save or update the profile depending on whether we actually have a
	 * profile object or not (this was created in updateProfileData)
	 */
	private void saveProfile()
	{
		if (verifyInput())
		{
			updateProfileData();
			if (mExisting != null)
			{
				mProfile.setId(mExisting.getId());
				mDataSource.updateVpnProfile(mProfile);
			}
			else
			{
				mDataSource.insertProfile(mProfile);
			}
			if (mCertEntry != null)
			{
				try
				{	/* store the CA/server certificate */
					KeyStore store = KeyStore.getInstance("LocalCertificateStore");
					store.load(null, null);
					store.setCertificateEntry(null, mCertEntry.getCertificate());
					TrustedCertificateManager.getInstance().reset();
				}
				catch (KeyStoreException | CertificateException | NoSuchAlgorithmException | IOException e)
				{
					e.printStackTrace();
				}
			}
			Intent intent = new Intent(Constants.VPN_PROFILES_CHANGED);
			intent.putExtra(Constants.VPN_PROFILES_SINGLE, mProfile.getId());
			LocalBroadcastManager.getInstance(this).sendBroadcast(intent);

			intent = new Intent(this, MainActivity.class);
			intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			startActivity(intent);

			setResult(RESULT_OK, new Intent().putExtra(VpnProfileDataSource.KEY_ID, mProfile.getId()));
			finish();
		}
	}

	/**
	 * Verify the user input and display error messages.
	 * @return true if the input is valid
	 */
	private boolean verifyInput()
	{
		boolean valid = true;
		if (mProfile.getVpnType().has(VpnTypeFeature.USER_PASS))
		{
			if (mUsername.getText().toString().trim().isEmpty())
			{
				mUsernameWrap.setError(getString(R.string.alert_text_no_input_username));
				valid = false;
			}
		}
		if (mProfile.getVpnType().has(VpnTypeFeature.CERTIFICATE) && mUserCertEntry == null)
		{	/* let's show an error icon */
			((TextView)mSelectUserCert.findViewById(android.R.id.text1)).setError("");
			valid = false;
		}
		return valid;
	}

	/**
	 * Update the profile object with the data entered by the user
	 */
	private void updateProfileData()
	{
		if (mProfile.getVpnType().has(VpnTypeFeature.USER_PASS))
		{
			mProfile.setUsername(mUsername.getText().toString().trim());
			String password = mPassword.getText().toString().trim();
			password = password.isEmpty() ? null : password;
			mProfile.setPassword(password);
		}
		if (mProfile.getVpnType().has(VpnTypeFeature.CERTIFICATE))
		{
			mProfile.setUserCertificateAlias(mUserCertEntry.getAlias());
		}
		if (mCertEntry != null)
		{
			mProfile.setCertificateAlias(mCertEntry.getAlias());
		}
	}

	/**
	 * Load the JSON-encoded VPN profile from the given URI
	 */
	private static class ProfileLoader extends AsyncTaskLoader<ProfileLoadResult>
	{
		private final Uri mUri;
		private ProfileLoadResult mData;

		public ProfileLoader(Context context, Uri uri)
		{
			super(context);
			mUri = uri;
		}

		@Override
		public ProfileLoadResult loadInBackground()
		{
			ProfileLoadResult result = new ProfileLoadResult();
			InputStream in = null;

			if (ContentResolver.SCHEME_CONTENT.equals(mUri.getScheme()) ||
				ContentResolver.SCHEME_FILE.equals(mUri.getScheme()))
			{
				try
				{
					in = getContext().getContentResolver().openInputStream(mUri);
				}
				catch (FileNotFoundException e)
				{
					result.ThrownException = e;
				}
			}
			else
			{
				try
				{
					URL url = new URL(mUri.toString());
					in = url.openStream();
				}
				catch (IOException e)
				{
					result.ThrownException = e;
				}
			}
			if (in != null)
			{
				try
				{
					result.Profile = streamToString(in);
				}
				catch (OutOfMemoryError e)
				{	/* just use a generic exception */
					result.ThrownException = new RuntimeException();
				}
			}
			return result;
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
		public void deliverResult(ProfileLoadResult data)
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

		private String streamToString(InputStream in)
		{
			ByteArrayOutputStream out = new ByteArrayOutputStream();
			byte[] buf = new byte[1024];
			int len;

			try
			{
				while ((len = in.read(buf)) != -1)
				{
					out.write(buf, 0, len);
				}
				return out.toString("UTF-8");
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}
			return null;
		}
	}

	private static class ProfileLoadResult
	{
		public String Profile;
		public Exception ThrownException;
	}

	/**
	 * Ask the user to select an available certificate.
	 */
	private class SelectUserCertOnClickListener implements View.OnClickListener, KeyChainAliasCallback
	{
		@Override
		public void onClick(View v)
		{
			String alias = null;
			if (mUserCertEntry != null)
			{
				alias = mUserCertEntry.getAlias();
				mUserCertEntry = null;
			}
			else if (mProfile != null)
			{
				alias = getString(R.string.profile_cert_alias, mProfile.getName());
			}
			KeyChain.choosePrivateKeyAlias(VpnProfileImportActivity.this, this, new String[] { "RSA" }, null, null, -1, alias);
		}

		@Override
		public void alias(final String alias)
		{
			/* alias() is not called from our main thread */
			runOnUiThread(new Runnable() {
				@Override
				public void run()
				{
					mUserCertLoading = alias;
					updateUserCertView();
					if (alias != null)
					{	/* otherwise the dialog was canceled, the request denied */
						getLoaderManager().restartLoader(USER_CERT_LOADER, null, mUserCertificateLoaderCallbacks);
					}
				}
			});
		}
	}

	/**
	 * Load the selected user certificate asynchronously.  This cannot be done
	 * from the main thread as getCertificateChain() calls back to our main
	 * thread to bind to the KeyChain service resulting in a deadlock.
	 */
	private static class UserCertificateLoader extends AsyncTaskLoader<TrustedCertificateEntry>
	{
		private final String mAlias;
		private TrustedCertificateEntry mData;

		public UserCertificateLoader(Context context, String alias)
		{
			super(context);
			mAlias = alias;
		}

		@Override
		public TrustedCertificateEntry loadInBackground()
		{
			X509Certificate[] chain = null;
			try
			{
				chain = KeyChain.getCertificateChain(getContext(), mAlias);
			}
			catch (KeyChainException | InterruptedException e)
			{
				e.printStackTrace();
			}
			if (chain != null && chain.length > 0)
			{
				return new TrustedCertificateEntry(mAlias, chain[0]);
			}
			return null;
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
		public void deliverResult(TrustedCertificateEntry data)
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

	private byte[] decodeBase64(String encoded)
	{
		if (encoded == null || encoded.isEmpty())
		{
			return null;
		}
		byte[] data = null;
		try
		{
			data = Base64.decode(encoded, Base64.DEFAULT);
		}
		catch (IllegalArgumentException e)
		{
			e.printStackTrace();
		}
		return data;
	}

	private class ParsedVpnProfile extends VpnProfile
	{
		public byte[] Certificate;
		public byte[] PKCS12;
	}
}
