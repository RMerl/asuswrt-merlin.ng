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

import org.strongswan.android.logic.imc.collectors.Protocol;
import org.strongswan.android.utils.BufferedByteWriter;

import android.util.Pair;

/**
 * PA-TNC Port Filter attribute (see section 4.2.6 of RFC 5792)
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   Reserved  |B|    Protocol   |         Port Number           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   Reserved  |B|    Protocol   |         Port Number           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
public class PortFilterAttribute implements Attribute
{
	private final LinkedList<Pair<Protocol, Short>> mPorts = new LinkedList<Pair<Protocol, Short>>();

	/**
	 * Add an open port with the given protocol and port number
	 * @param protocol transport protocol
	 * @param port port number
	 */
	public void addPort(Protocol protocol, short port)
	{
		mPorts.add(new Pair<Protocol, Short>(protocol, port));
	}

	@Override
	public byte[] getEncoding()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		for (Pair<Protocol, Short> port : mPorts)
		{
			/* we report open ports, so the BLOCKED flag is not set */
			writer.put((byte)0);
			writer.put(port.first.getValue());
			writer.put16(port.second);
		}
		return writer.toByteArray();
	}
}
