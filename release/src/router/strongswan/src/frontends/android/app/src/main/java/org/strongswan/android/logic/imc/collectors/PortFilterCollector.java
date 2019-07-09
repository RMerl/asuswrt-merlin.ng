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

package org.strongswan.android.logic.imc.collectors;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.strongswan.android.logic.imc.attributes.Attribute;
import org.strongswan.android.logic.imc.attributes.PortFilterAttribute;

public class PortFilterCollector implements Collector
{
	private static Pattern LISTEN = Pattern.compile("\\bLISTEN\\b");
	private static Pattern PROTOCOL = Pattern.compile("\\b(tcp|udp)6?\\b");
	private static Pattern PORT = Pattern.compile("[:]{1,3}(\\d{1,5})\\b(?!\\.)");

	@Override
	public Attribute getMeasurement()
	{
		PortFilterAttribute attribute = null;
		try
		{
			Process netstat = Runtime.getRuntime().exec("netstat -n");
			try
			{
				BufferedReader reader = new BufferedReader(new InputStreamReader(netstat.getInputStream()));
				String line;
				attribute = new PortFilterAttribute();
				while ((line = reader.readLine()) != null)
				{
					if (!LISTEN.matcher(line).find())
					{
						continue;
					}
					Matcher protocolMatcher = PROTOCOL.matcher(line);
					Matcher portMatcher = PORT.matcher(line);
					if (protocolMatcher.find() && portMatcher.find())
					{
						Protocol protocol = Protocol.fromName(protocolMatcher.group());
						if (protocol == null)
						{
							continue;
						}
						int port = Integer.parseInt(portMatcher.group(1));
						attribute.addPort(protocol, (short)port);
					}
				}
			}
			finally
			{
				netstat.destroy();
			}
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		return attribute;
	}

}
