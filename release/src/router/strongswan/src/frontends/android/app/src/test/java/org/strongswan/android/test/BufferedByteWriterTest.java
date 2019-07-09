/*
 * Copyright (C) 2013-2015 Tobias Brunner
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
import org.strongswan.android.utils.BufferedByteWriter;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;

public class BufferedByteWriterTest
{
	byte[] testArray = {0, 1, 2, 3, 4, 5, 6, 7};

	private void testArray(byte[] array, int offset)
	{
		for (int i = offset; i < array.length; i++)
		{
			assertEquals("value " + i, array[i], (i - offset) % 8);
		}
	}

	private void testArray(byte[] array)
	{
		testArray(array, 0);
	}

	@Test
	public void testPutByteArray()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		byte[] array = writer.toByteArray();
		assertEquals("length", array.length, 0);
		writer.put(testArray);
		array = writer.toByteArray();
		assertEquals("length", array.length, testArray.length);
		testArray(array);
	}

	@Test
	public void testResize()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put(testArray);
		writer.put(testArray);
		writer.put(testArray);
		writer.put(testArray);
		byte[] array = writer.toByteArray();
		assertEquals("length", array.length, testArray.length * 4);
		testArray(array);
		writer.put(testArray);
		array = writer.toByteArray();
		assertEquals("length", array.length, testArray.length * 5);
		testArray(array);
	}

	@Test
	public void testResizeCapacity()
	{
		BufferedByteWriter writer = new BufferedByteWriter(4);
		writer.put(testArray);
		writer.put(testArray);
		writer.put(testArray);
		writer.put(testArray);
		byte[] array = writer.toByteArray();
		assertEquals("length", array.length, testArray.length * 4);
		testArray(array);
		writer.put(testArray);
		array = writer.toByteArray();
		assertEquals("length", array.length, testArray.length * 5);
		testArray(array);
	}

	@Test
	public void testPutByte()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put((byte)1);
		byte[] array = writer.toByteArray();
		assertEquals("length", array.length, 1);
		assertEquals("value", array[0], 1);
	}

	@Test
	public void testPutLen8()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.putLen8(testArray);
		byte[] array = writer.toByteArray();
		assertEquals("length", array.length, testArray.length + 1);
		assertEquals("value", array[0], testArray.length);
		testArray(array, 1);
	}

	@Test
	public void testPutLen16()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.putLen16(testArray);
		byte[] array = writer.toByteArray();
		assertEquals("length", array.length, testArray.length + 2);
		assertEquals("value 0", array[0], 0);
		assertEquals("value 1", array[1], testArray.length);
		testArray(array, 2);
	}

	@Test
	public void testPut16()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put16((short)0xbeef);
		byte[] exp = new byte[]{(byte)0xbe, (byte)0xef};
		assertArrayEquals(exp, writer.toByteArray());
		writer.put16((short)0xfeed);
		exp = new byte[]{(byte)0xbe, (byte)0xef, (byte)0xfe, (byte)0xed};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut16Byte()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put16((byte)0xfe);
		byte[] exp = new byte[]{0, (byte)0xfe};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut24Half()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put24(0xbeef);
		byte[] exp = new byte[]{0, (byte)0xbe, (byte)0xef};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut24Full()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put24(0xbeeffe);
		byte[] exp = new byte[]{(byte)0xbe, (byte)0xef, (byte)0xfe};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut24Int()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put24(0xcafebeef);
		byte[] exp = new byte[]{(byte)0xfe, (byte)0xbe, (byte)0xef};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut24Short()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put24((short)0xfeed);
		byte[] exp = new byte[]{0, (byte)0xfe, (byte)0xed};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut24Byte()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put24((byte)0xfe);
		byte[] exp = new byte[]{0, 0, (byte)0xfe};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut32()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put32(0xbeeffeed);
		byte[] exp = new byte[]{(byte)0xbe, (byte)0xef, (byte)0xfe, (byte)0xed};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut32Short()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put32((short)0xbeef);
		byte[] exp = new byte[]{0, 0, (byte)0xbe, (byte)0xef};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut32Byte()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put32((byte)0xfe);
		byte[] exp = new byte[]{0, 0, 0, (byte)0xfe};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut64()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put64(0xdeadcafebeeffeedL);
		byte[] exp = new byte[]{(byte)0xde, (byte)0xad, (byte)0xca, (byte)0xfe, (byte)0xbe, (byte)0xef, (byte)0xfe, (byte)0xed};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut64Half()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put64(0xbeeffeedL);
		byte[] exp = new byte[]{0, 0, 0, 0, (byte)0xbe, (byte)0xef, (byte)0xfe, (byte)0xed};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut64Int()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put64(0xbeeffeed);
		byte[] exp = new byte[]{0, 0, 0, 0, (byte)0xbe, (byte)0xef, (byte)0xfe, (byte)0xed};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut64Short()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put64((short)0xfeed);
		byte[] exp = new byte[]{0, 0, 0, 0, 0, 0, (byte)0xfe, (byte)0xed};
		assertArrayEquals(exp, writer.toByteArray());
	}

	@Test
	public void testPut64Byte()
	{
		BufferedByteWriter writer = new BufferedByteWriter();
		writer.put64((byte)0xfe);
		byte[] exp = new byte[]{0, 0, 0, 0, 0, 0, 0, (byte)0xfe};
		assertArrayEquals(exp, writer.toByteArray());
	}
}
