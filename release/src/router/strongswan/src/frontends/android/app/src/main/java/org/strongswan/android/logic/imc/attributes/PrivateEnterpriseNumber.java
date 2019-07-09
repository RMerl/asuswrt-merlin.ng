/*
 * Copyright (C) 2013 Tobias Brunner
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

public enum PrivateEnterpriseNumber
{
	IETF(0x000000),
	GOOGLE(0x002B79),
	ITA(0x00902a),
	UNASSIGNED(0xfffffe),
	RESERVED(0xffffff);

	private int mValue;

	/**
	 * Enum for private enterprise numbers (PEN) as allocated by IANA
	 *
	 * @param value numeric value
	 */
	private PrivateEnterpriseNumber(int value)
	{
		mValue = value;
	}

	/**
	 * Get the numeric value of a PEN
	 *
	 * @return numeric value
	 */
	public int getValue()
	{
		return mValue;
	}

	/**
	 * Get the enum entry from a numeric value, if defined
	 *
	 * @param value numeric value
	 * @return the enum entry or null
	 */
	public static PrivateEnterpriseNumber fromValue(int value)
	{
		for (PrivateEnterpriseNumber pen : PrivateEnterpriseNumber.values())
		{
			if (pen.mValue == value)
			{
				return pen;
			}
		}
		return null;
	}
}
