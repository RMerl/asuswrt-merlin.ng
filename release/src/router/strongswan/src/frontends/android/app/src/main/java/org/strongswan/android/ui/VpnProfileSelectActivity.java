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

import android.content.Intent;
import android.os.Bundle;
import android.support.v4.content.pm.ShortcutInfoCompat;
import android.support.v4.content.pm.ShortcutManagerCompat;
import android.support.v4.graphics.drawable.IconCompat;
import android.support.v7.app.AppCompatActivity;

import org.strongswan.android.R;
import org.strongswan.android.data.VpnProfile;
import org.strongswan.android.ui.VpnProfileListFragment.OnVpnProfileSelectedListener;

public class VpnProfileSelectActivity extends AppCompatActivity implements OnVpnProfileSelectedListener
{
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.vpn_profile_select);

		/* we should probably return a result also if the user clicks the back
		 * button before selecting a profile */
		setResult(RESULT_CANCELED);
	}

	@Override
	public void onVpnProfileSelected(VpnProfile profile)
	{
		Intent shortcut = new Intent(VpnProfileControlActivity.START_PROFILE);
		shortcut.putExtra(VpnProfileControlActivity.EXTRA_VPN_PROFILE_ID, profile.getUUID().toString());

		ShortcutInfoCompat.Builder builder = new ShortcutInfoCompat.Builder(this, profile.getUUID().toString());
		builder.setIntent(shortcut);
		builder.setShortLabel(profile.getName());
		builder.setIcon(IconCompat.createWithResource(this, R.mipmap.ic_shortcut));
		setResult(RESULT_OK, ShortcutManagerCompat.createShortcutResultIntent(this, builder.build()));
		finish();
	}
}
