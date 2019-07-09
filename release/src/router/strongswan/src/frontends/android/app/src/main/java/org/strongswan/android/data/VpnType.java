/*
 * Copyright (C) 2012-2014 Tobias Brunner
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

package org.strongswan.android.data;

import java.util.EnumSet;

public enum VpnType
{
	/* the order here must match the items in R.array.vpn_types */
	IKEV2_EAP("ikev2-eap", EnumSet.of(VpnTypeFeature.USER_PASS)),
	IKEV2_CERT("ikev2-cert", EnumSet.of(VpnTypeFeature.CERTIFICATE)),
	IKEV2_CERT_EAP("ikev2-cert-eap", EnumSet.of(VpnTypeFeature.USER_PASS, VpnTypeFeature.CERTIFICATE)),
	IKEV2_EAP_TLS("ikev2-eap-tls", EnumSet.of(VpnTypeFeature.CERTIFICATE)),
	IKEV2_BYOD_EAP("ikev2-byod-eap", EnumSet.of(VpnTypeFeature.USER_PASS, VpnTypeFeature.BYOD));

	/**
	 * Features of a VPN type.
	 */
	public enum VpnTypeFeature
	{
		/** client certificate is required */
		CERTIFICATE,
		/** username and password are required */
		USER_PASS,
		/** enable BYOD features */
		BYOD;
	}

	private String mIdentifier;
	private EnumSet<VpnTypeFeature> mFeatures;

	/**
	 * Enum which provides additional information about the supported VPN types.
	 *
	 * @param id identifier used to store and transmit this specific type
	 * @param features of the given VPN type
	 * @param certificate true if a client certificate is required
	 */
	VpnType(String id, EnumSet<VpnTypeFeature> features)
	{
		mIdentifier = id;
		mFeatures = features;
	}

	/**
	 * The identifier used to store this value in the database
	 * @return identifier
	 */
	public String getIdentifier()
	{
		return mIdentifier;
	}

	/**
	 * Checks whether a feature is supported/required by this type of VPN.
	 *
	 * @return true if the feature is supported/required
	 */
	public boolean has(VpnTypeFeature feature)
	{
		return mFeatures.contains(feature);
	}

	/**
	 * Get the enum entry with the given identifier.
	 *
	 * @param identifier get the enum entry with this identifier
	 * @return the enum entry, or the default if not found
	 */
	public static VpnType fromIdentifier(String identifier)
	{
		for (VpnType type : VpnType.values())
		{
			if (identifier.equals(type.mIdentifier))
			{
				return type;
			}
		}
		return VpnType.IKEV2_EAP;
	}
}
