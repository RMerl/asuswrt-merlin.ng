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

package org.strongswan.android.logic.imc;

public enum ImcState
{
	UNKNOWN(0),
	ALLOW(1),
	BLOCK(2),
	ISOLATE(3);

	private final int mValue;

	private ImcState(int value)
	{
		mValue = value;
	}

	/**
	 * Get the numeric value of the IMC state.
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
	public static ImcState fromValue(int value)
	{
		for (ImcState state : ImcState.values())
		{
			if (state.mValue == value)
			{
				return state;
			}
		}
		return null;
	}
}
