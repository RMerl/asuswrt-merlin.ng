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

import java.util.LinkedList;

import org.strongswan.android.utils.BufferedByteWriter;

import android.util.Pair;

/**
 * PA-TNC Installed Packages attribute (see section 4.2.7 of RFC 5792)
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |          Reserved             |         Package Count         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | Pkg Name Len  |        Package Name (Variable Length)         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Version Len  |    Package Version Number (Variable Length)   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
public class InstalledPackagesAttribute implements Attribute
{
	private final short RESERVED = 0;
	private final LinkedList<Pair<String, String>> mPackages = new LinkedList<Pair<String, String>>();

	/**
	 * Add an installed package to this attribute.
	 * @param name name of the package
	 * @param version version number of the package
	 */
	public void addPackage(String name, String version)
	{
		mPackages.add(new Pair<String, String>(name, version));
	}

	@Override
	public byte[] getEncoding()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put16(RESERVED);
		writer.put16((short)mPackages.size());
		for (Pair<String, String> pair : mPackages)
		{
			writer.putLen8(pair.first.getBytes());
			writer.putLen8(pair.second.getBytes());
		}
		return writer.toByteArray();
	}
}
