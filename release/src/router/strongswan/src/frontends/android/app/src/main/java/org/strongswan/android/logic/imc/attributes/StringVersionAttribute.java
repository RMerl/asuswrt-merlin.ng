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

import org.strongswan.android.utils.BufferedByteWriter;

/**
 * PA-TNC String Version attribute (see section 4.2.4 of RFC 5792)
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Version Len  |   Product Version Number (Variable Length)    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Build Num Len |   Internal Build Number (Variable Length)     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Config. Len  | Configuration Version Number (Variable Length)|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
public class StringVersionAttribute implements Attribute
{
	private String mVersionNumber;
	private String mBuildNumber;

	/**
	 * Set the product version number
	 * @param version version number
	 */
	public void setProductVersionNumber(String version)
	{
		this.mVersionNumber = version;
	}

	/**
	 * Set the internal build number
	 * @param build build number
	 */
	public void setInternalBuildNumber(String build)
	{
		this.mBuildNumber = build;
	}

	@Override
	public byte[] getEncoding()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.putLen8(mVersionNumber.getBytes());
		writer.putLen8(mBuildNumber.getBytes());
		/* we don't provide a configuration number */
		writer.put((byte)0);
		return writer.toByteArray();
	}
}
