/*
 * Copyright (C) 2015 Tobias Brunner
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

package org.strongswan.android.test;

import org.junit.Test;
import org.strongswan.android.utils.SettingsWriter;

import static org.junit.Assert.assertEquals;

public class SettingsWriterTest
{
	@Test
	public void testString()
	{
		SettingsWriter writer = new SettingsWriter();
		assertEquals("serialized", "", writer.serialize());
		writer.setValue("key", "value");
		assertEquals("serialized", "key=\"value\"\n", writer.serialize());
	}

	@Test
	public void testInt()
	{
		SettingsWriter writer = new SettingsWriter();
		writer.setValue("key", 1234);
		assertEquals("serialized", "key=\"1234\"\n", writer.serialize());
		writer.setValue("key", (Integer)null);
		assertEquals("serialized", "key=\n", writer.serialize());
	}

	@Test
	public void testBoolean()
	{
		SettingsWriter writer = new SettingsWriter();
		writer.setValue("key1", true);
		writer.setValue("key2", false);
		assertEquals("serialized", "key1=\"1\"\nkey2=\"0\"\n", writer.serialize());
		writer.setValue("key2", (Boolean)null);
		assertEquals("serialized", "key1=\"1\"\nkey2=\n", writer.serialize());
	}

	@Test
	public void testReplace()
	{
		SettingsWriter writer = new SettingsWriter();
		writer.setValue("key", "value");
		writer.setValue("key", "other");
		assertEquals("serialized", "key=\"other\"\n", writer.serialize());
	}

	@Test
	public void testUnset()
	{
		SettingsWriter writer = new SettingsWriter();
		writer.setValue("key", (String)null);
		assertEquals("serialized", "key=\n", writer.serialize());
	}

	@Test
	public void testChain()
	{
		SettingsWriter writer = new SettingsWriter();
		writer.setValue("key1", "value").setValue("key2", 1234).setValue("key3", true);
		assertEquals("serialized", "key1=\"value\"\nkey2=\"1234\"\nkey3=\"1\"\n", writer.serialize());
	}

	@Test
	public void testInvalid()
	{
		SettingsWriter writer = new SettingsWriter();
		writer.setValue("", "value");
		writer.setValue(null, "value");
		writer.setValue("se{c.key", "value");
		writer.setValue("sec}.key", "value");
		writer.setValue("sec\n.key", "value");
		assertEquals("serialized", "", writer.serialize());
	}

	@Test
	public void testEscape()
	{
		SettingsWriter writer = new SettingsWriter();
		writer.setValue("key", "val\"ue");
		writer.setValue("nl", "val\nue");
		writer.setValue("bs", "val\\ue");
		assertEquals("serialized", "key=\"val\\\"ue\"\nnl=\"val\nue\"\nbs=\"val\\\\ue\"\n", writer.serialize());
	}

	@Test
	public void testSections()
	{
		SettingsWriter writer = new SettingsWriter();
		writer.setValue("sec.key1", "value");
		writer.setValue("sec.sub.key", true);
		writer.setValue("sec.key2", 1234);
		assertEquals("serialized", "sec {\nkey1=\"value\"\nkey2=\"1234\"\nsub {\nkey=\"1\"\n}\n}\n", writer.serialize());
	}

	@Test
	public void testOrder()
	{
		SettingsWriter writer = new SettingsWriter();
		writer.setValue("key1", "value");
		writer.setValue("key2", 1234);
		writer.setValue("key1", "other");
		assertEquals("serialized", "key1=\"other\"\nkey2=\"1234\"\n", writer.serialize());
	}
}
