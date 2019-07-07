/*
 * Copyright (C) 2018 Tobias Brunner
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

import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.preference.ListPreference;
import android.support.v7.preference.Preference;
import android.support.v7.preference.PreferenceFragmentCompat;
import android.support.v7.preference.PreferenceManager;

import org.strongswan.android.R;
import org.strongswan.android.data.VpnProfile;
import org.strongswan.android.data.VpnProfileDataSource;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import static org.strongswan.android.utils.Constants.PREF_DEFAULT_VPN_PROFILE;
import static org.strongswan.android.utils.Constants.PREF_DEFAULT_VPN_PROFILE_MRU;

public class SettingsFragment extends PreferenceFragmentCompat implements Preference.OnPreferenceChangeListener
{
	private ListPreference mDefaultVPNProfile;

	@Override
	public void onCreatePreferences(Bundle bundle, String s)
	{
		setPreferencesFromResource(R.xml.settings, s);

		mDefaultVPNProfile = (ListPreference)findPreference(PREF_DEFAULT_VPN_PROFILE);
		mDefaultVPNProfile.setOnPreferenceChangeListener(this);
		if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N)
		{
			mDefaultVPNProfile.setEnabled(false);
		}
	}

	@Override
	public void onResume()
	{
		super.onResume();

		VpnProfileDataSource profiles = new VpnProfileDataSource(getActivity());
		profiles.open();

		List<VpnProfile> all = profiles.getAllVpnProfiles();
		Collections.sort(all, new Comparator<VpnProfile>() {
			@Override
			public int compare(VpnProfile lhs, VpnProfile rhs)
			{
				return lhs.getName().compareToIgnoreCase(rhs.getName());
			}
		});

		ArrayList<CharSequence> entries = new ArrayList<>();
		ArrayList<CharSequence> entryvalues = new ArrayList<>();

		entries.add(getString(R.string.pref_default_vpn_profile_mru));
		entryvalues.add(PREF_DEFAULT_VPN_PROFILE_MRU);

		for (VpnProfile profile : all)
		{
			entries.add(profile.getName());
			entryvalues.add(profile.getUUID().toString());
		}
		profiles.close();

		if (entries.size() <= 1)
		{
			mDefaultVPNProfile.setEnabled(false);
		}
		else
		{
			mDefaultVPNProfile.setEnabled(true);
			mDefaultVPNProfile.setEntries(entries.toArray(new CharSequence[0]));
			mDefaultVPNProfile.setEntryValues(entryvalues.toArray(new CharSequence[0]));
		}

		SharedPreferences pref = PreferenceManager.getDefaultSharedPreferences(getActivity());
		setCurrentProfileName(pref.getString(PREF_DEFAULT_VPN_PROFILE, PREF_DEFAULT_VPN_PROFILE_MRU));
	}

	@Override
	public boolean onPreferenceChange(Preference preference, Object newValue)
	{
		if (preference == mDefaultVPNProfile)
		{
			setCurrentProfileName((String)newValue);
		}
		return true;
	}

	private void setCurrentProfileName(String uuid)
	{
		VpnProfileDataSource profiles = new VpnProfileDataSource(getActivity());
		profiles.open();

		if (!uuid.equals(PREF_DEFAULT_VPN_PROFILE_MRU))
		{
			VpnProfile current = profiles.getVpnProfile(uuid);
			if (current != null)
			{
				mDefaultVPNProfile.setSummary(current.getName());
			}
			else
			{
				mDefaultVPNProfile.setSummary(R.string.profile_not_found);
			}
		}
		else
		{
			mDefaultVPNProfile.setSummary(R.string.pref_default_vpn_profile_mru);
		}
		profiles.close();
	}
}
