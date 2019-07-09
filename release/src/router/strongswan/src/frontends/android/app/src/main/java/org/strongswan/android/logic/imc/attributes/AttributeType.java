/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2012 Christoph Buehler
 * Copyright (C) 2012 Patrick Loetscher
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

package org.strongswan.android.logic.imc.attributes;

public enum AttributeType
{
	/* IETF standard PA-TNC attribute types defined by RFC 5792 */
	IETF_TESTING(PrivateEnterpriseNumber.IETF, 0),
	IETF_ATTRIBUTE_REQUEST(PrivateEnterpriseNumber.IETF, 1),
	IETF_PRODUCT_INFORMATION(PrivateEnterpriseNumber.IETF, 2),
	IETF_NUMERIC_VERSION(PrivateEnterpriseNumber.IETF, 3),
	IETF_STRING_VERSION(PrivateEnterpriseNumber.IETF, 4),
	IETF_OPERATIONAL_STATUS(PrivateEnterpriseNumber.IETF, 5),
	IETF_PORT_FILTER(PrivateEnterpriseNumber.IETF, 6),
	IETF_INSTALLED_PACKAGES(PrivateEnterpriseNumber.IETF, 7),
	IETF_PA_TNC_ERROR(PrivateEnterpriseNumber.IETF, 8),
	IETF_ASSESSMENT_RESULT(PrivateEnterpriseNumber.IETF, 9),
	IETF_REMEDIATION_INSTRUCTIONS(PrivateEnterpriseNumber.IETF, 10),
	IETF_FORWARDING_ENABLED(PrivateEnterpriseNumber.IETF, 11),
	IETF_FACTORY_DEFAULT_PWD_ENABLED(PrivateEnterpriseNumber.IETF, 12),
	IETF_RESERVED(PrivateEnterpriseNumber.IETF, 0xffffffff),
	/* ITA attributes */
	ITA_SETTINGS(PrivateEnterpriseNumber.ITA, 4),
	ITA_DEVICE_ID(PrivateEnterpriseNumber.ITA, 8);

	private PrivateEnterpriseNumber mVendor;
	private int mType;

	/**
	 * Enum type for vendor specific attributes (defined in their namespace)
	 *
	 * @param vendor private enterprise number of vendor
	 * @param type vendor specific attribute type
	 */
	private AttributeType(PrivateEnterpriseNumber vendor, int type)
	{
		mVendor = vendor;
		mType = type;
	}

	/**
	 * Get private enterprise number of vendor
	 *
	 * @return PEN
	 */
	public PrivateEnterpriseNumber getVendor()
	{
		return mVendor;
	}

	/**
	 * Get vendor specific type
	 *
	 * @return type
	 */
	public int getType()
	{
		return mType;
	}

	/**
	 * Get the enum entry from the given numeric values, if defined
	 *
	 * @param vendor vendor id
	 * @param type vendor specific type
	 * @return enum entry or null
	 */
	public static AttributeType fromValues(int vendor, int type)
	{
		PrivateEnterpriseNumber pen = PrivateEnterpriseNumber.fromValue(vendor);

		if (pen == null)
		{
			return null;
		}
		for (AttributeType attr : AttributeType.values())
		{
			if (attr.mVendor == pen && attr.mType == type)
			{
				return attr;
			}
		}
		return null;
	}
}
