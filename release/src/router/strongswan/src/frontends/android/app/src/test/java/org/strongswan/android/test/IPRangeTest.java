/*
 * Copyright (C) 2017 Tobias Brunner
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
import org.strongswan.android.utils.IPRange;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.List;

import static org.junit.Assert.assertEquals;

public class IPRangeTest
{
	@Test
	public void testRangeReversed() throws UnknownHostException
	{
		IPRange test = new IPRange("192.168.0.10", "192.168.0.1");
		assertEquals("from", "192.168.0.1", test.getFrom().getHostAddress());
		assertEquals("to", "192.168.0.10", test.getTo().getHostAddress());
	}

	@Test(expected = IllegalArgumentException.class)
	public void testRangeInvalid() throws UnknownHostException
	{
		IPRange test = new IPRange("192.168.0.1", "fec1::1");
		assertEquals("from", "192.168.0.1", test.getFrom().getHostAddress());
	}

	@Test(expected = UnknownHostException.class)
	public void testPrefixAddrInvalid() throws UnknownHostException
	{
		IPRange test = new IPRange("a.b.c.d", 24);
		assertEquals("from", "192.168.0.1", test.getFrom().getHostAddress());
	}

	@Test(expected = IllegalArgumentException.class)
	public void testPrefixNegative() throws UnknownHostException
	{
		IPRange test = new IPRange("192.168.0.1", -5);
		assertEquals("from", "192.168.0.1", test.getFrom().getHostAddress());
	}

	@Test(expected = IllegalArgumentException.class)
	public void testPrefixLarge() throws UnknownHostException
	{
		IPRange test = new IPRange("192.168.0.1", 42);
		assertEquals("from", "192.168.0.1", test.getFrom().getHostAddress());
	}

	private void testPrefix(String from, String to, Integer prefix) throws UnknownHostException
	{
		IPRange test = new IPRange(from, to);
		assertEquals("prefix", prefix, test.getPrefix());
	}

	@Test
	public void testPrefix() throws UnknownHostException
	{
		testPrefix("192.168.0.0", "192.168.0.255", 24);
		testPrefix("192.168.0.8", "192.168.0.15", 29);
		testPrefix("192.168.0.1", "192.168.0.255", null);
		testPrefix("192.168.0.1", "192.168.0.1", 32);
		testPrefix("fec1::0", "fec1::ffff", 112);
		testPrefix("fec1::1", "fec1::ffff", null);
		testPrefix("fec1::1", "fec1::1", 128);
	}

	private void testPrefixRange(String base, int prefix, String from, String to) throws UnknownHostException
	{
		IPRange test = new IPRange(InetAddress.getByName(base), prefix);
		assertEquals("from", from, test.getFrom().getHostAddress());
		assertEquals("to", to, test.getTo().getHostAddress());
	}

	@Test
	public void testPrefixRange() throws UnknownHostException
	{
		testPrefixRange("0.0.0.0", 0, "0.0.0.0", "255.255.255.255");
		testPrefixRange("0.0.0.0", 32, "0.0.0.0", "0.0.0.0");
		testPrefixRange("192.168.1.0", 24, "192.168.1.0", "192.168.1.255");
		testPrefixRange("192.168.1.10", 24, "192.168.1.0", "192.168.1.255");
		testPrefixRange("192.168.1.64", 26, "192.168.1.64", "192.168.1.127");
		testPrefixRange("192.168.1.64", 27, "192.168.1.64", "192.168.1.95");
		testPrefixRange("192.168.1.64", 28, "192.168.1.64", "192.168.1.79");
		testPrefixRange("192.168.1.255", 29, "192.168.1.248", "192.168.1.255");
		testPrefixRange("192.168.1.255", 30, "192.168.1.252", "192.168.1.255");
		testPrefixRange("192.168.1.255", 31, "192.168.1.254", "192.168.1.255");
		testPrefixRange("192.168.1.255", 32, "192.168.1.255", "192.168.1.255");

		testPrefixRange("::", 0, "0:0:0:0:0:0:0:0", "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff");
		testPrefixRange("fec1::", 64, "fec1:0:0:0:0:0:0:0", "fec1:0:0:0:ffff:ffff:ffff:ffff");
		testPrefixRange("fec1::1", 128, "fec1:0:0:0:0:0:0:1", "fec1:0:0:0:0:0:0:1");
		testPrefixRange("fec1::10:0", 108, "fec1:0:0:0:0:0:10:0", "fec1:0:0:0:0:0:1f:ffff");
		testPrefixRange("fec1::10:0", 112, "fec1:0:0:0:0:0:10:0", "fec1:0:0:0:0:0:10:ffff");
		testPrefixRange("fec1::10:0", 113, "fec1:0:0:0:0:0:10:0", "fec1:0:0:0:0:0:10:7fff");
		testPrefixRange("fec1::10:0", 116, "fec1:0:0:0:0:0:10:0", "fec1:0:0:0:0:0:10:fff");
		testPrefixRange("fec1::1:ffff", 112, "fec1:0:0:0:0:0:1:0", "fec1:0:0:0:0:0:1:ffff");
		testPrefixRange("fec1::1:ffff", 113, "fec1:0:0:0:0:0:1:8000", "fec1:0:0:0:0:0:1:ffff");
		testPrefixRange("fec1::1:ffff", 114, "fec1:0:0:0:0:0:1:c000", "fec1:0:0:0:0:0:1:ffff");
		testPrefixRange("fec1::1:ffff", 115, "fec1:0:0:0:0:0:1:e000", "fec1:0:0:0:0:0:1:ffff");
		testPrefixRange("fec1::1:ffff", 116, "fec1:0:0:0:0:0:1:f000", "fec1:0:0:0:0:0:1:ffff");
		testPrefixRange("fec1::1:ffff", 117, "fec1:0:0:0:0:0:1:f800", "fec1:0:0:0:0:0:1:ffff");
	}

	@Test(expected = IllegalArgumentException.class)
	public void testCIDRAddressInvalid() throws UnknownHostException
	{
		IPRange test = new IPRange("asdf");
		assertEquals("not reached", null, test);
	}

	@Test(expected = IllegalArgumentException.class)
	public void testCIDRIncomplete() throws UnknownHostException
	{
		IPRange test = new IPRange("/32");
		assertEquals("not reached", null, test);
	}

	@Test(expected = IllegalArgumentException.class)
	public void testCIDRIncompletePrefix() throws UnknownHostException
	{
		IPRange test = new IPRange("192.168.0.1/");
		assertEquals("not reached", null, test);
	}

	@Test(expected = IllegalArgumentException.class)
	public void testCIDRPrefixNegative() throws UnknownHostException
	{
		IPRange test = new IPRange("192.168.0.1/-5");
		assertEquals("not reached", null, test);
	}

	@Test(expected = IllegalArgumentException.class)
	public void testCIDRPrefixLarge() throws UnknownHostException
	{
		IPRange test = new IPRange("192.168.0.1/33");
		assertEquals("not reached", null, test);
	}

	@Test(expected = IllegalArgumentException.class)
	public void testCIDRPrefixLargev6() throws UnknownHostException
	{
		IPRange test = new IPRange("fec1::1/129");
		assertEquals("not reached", null, test);
	}

	@Test(expected = IllegalArgumentException.class)
	public void testRangeMixed() throws UnknownHostException
	{
		IPRange test = new IPRange("192.168.1.1-fec1::1");
		assertEquals("not reached", null, test);
	}

	private void testCIDR(String cidr, IPRange exp) throws UnknownHostException
	{
		IPRange test = new IPRange(cidr);
		assertEquals("cidr", exp, test);
	}

	@Test
	public void testCIDR() throws UnknownHostException
	{
		testCIDR("0.0.0.0/0", new IPRange("0.0.0.0", 0));
		testCIDR("192.168.1.0/24", new IPRange("192.168.1.0", 24));
		testCIDR("192.168.1.10/24", new IPRange("192.168.1.0", 24));
		testCIDR("192.168.1.1/32", new IPRange("192.168.1.1", 32));
		testCIDR("192.168.1.1", new IPRange("192.168.1.1", 32));
		testCIDR("192.168.1.1-192.168.1.10", new IPRange("192.168.1.1", "192.168.1.10"));
		testCIDR("::/0", new IPRange("::", 0));
		testCIDR("fec1::/64", new IPRange("fec1::", 64));
		testCIDR("fec1::10/64", new IPRange("fec1::", 64));
		testCIDR("fec1::1/128", new IPRange("fec1::1", 128));
		testCIDR("fec1::1", new IPRange("fec1::1", 128));
		testCIDR("fec1::1-fec1::5", new IPRange("fec1::1", "fec1::5"));
	}

	private void testToString(String f, String t, String exp) throws UnknownHostException
	{
		IPRange a = new IPRange(f, t);
		assertEquals("string", exp, a.toString());
	}

	@Test
	public void testToString() throws UnknownHostException
	{
		testToString("192.168.1.1", "192.168.1.1", "192.168.1.1/32");
		testToString("192.168.1.0", "192.168.1.255", "192.168.1.0/24");
		testToString("192.168.1.1", "192.168.1.255", "192.168.1.1-192.168.1.255");
		testToString("0.0.0.0", "255.255.255.255", "0.0.0.0/0");
		testToString("fec1::1", "fec1::1", "fec1:0:0:0:0:0:0:1/128");
		testToString("fec1::", "fec1::ffff", "fec1:0:0:0:0:0:0:0/112");
		testToString("::", "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", "0:0:0:0:0:0:0:0/0");
	}

	private void compare(String af, String at, String bf, String bt, int exp) throws UnknownHostException
	{
		IPRange a = new IPRange(af, at);
		IPRange b = new IPRange(bf, bt);
		assertEquals("compare", exp, a.compareTo(b));
	}

	private void compare(String a, int pa, String b, int pb, int exp) throws UnknownHostException
	{
		IPRange ar = new IPRange(a, pa);
		IPRange br = new IPRange(b, pb);
		assertEquals("compare", exp, ar.compareTo(br));
	}

	@Test
	public void testCompareTo() throws UnknownHostException
	{
		compare("192.168.0.0", "192.168.0.10", "192.168.0.0", "192.168.0.10", 0);
		compare("192.168.0.1", "192.168.0.10", "192.168.0.0", "192.168.0.10", 1);
		compare("192.168.0.0", "192.168.0.9", "192.168.0.0", "192.168.0.10", -1);

		compare("192.168.0.0", 24, "192.168.0.0", 24, 0);
		compare("192.168.0.0", 24, "192.168.0.0", 28, 1);
		compare("192.168.0.0", 28, "192.168.0.0", 24, -1);
		compare("192.168.0.0", 32, "192.168.0.255", 32, -1);
		compare("10.0.1.0", 24, "192.168.1.0", 24, -1);
		compare("10.0.1.0", 24, "fec1::", 64, -1);
		compare("fec1::1", 128, "fec1::2", 128, -1);
		compare("fec1::1", 126, "fec1::2", 126, 0);
	}

	@Test
	public void testEquals() throws UnknownHostException
	{
		IPRange a = new IPRange("192.168.1.0/24");
		IPRange b = new IPRange("192.168.1.0/24");
		assertEquals("same", true, a.equals(a));
		assertEquals("equals", true, a.equals(b));
		InetAddress c = InetAddress.getByName("192.168.1.0");
		assertEquals("null", false, a.equals(c));
		assertEquals("null", false, a.equals(null));
	}

	private void contains(String af, String at, String bf, String bt, boolean exp) throws UnknownHostException
	{
		IPRange a = new IPRange(af, at);
		IPRange b = new IPRange(bf, bt);
		assertEquals("contains", exp, a.contains(b));
	}

	@Test
	public void testContains() throws UnknownHostException
	{
		contains("192.168.1.0", "192.168.1.10", "192.168.1.0", "192.168.1.10", true);
		contains("192.168.1.0", "192.168.1.10", "192.168.1.1", "192.168.1.10", true);
		contains("192.168.1.0", "192.168.1.10", "192.168.1.0", "192.168.1.9", true);
		contains("192.168.1.0", "192.168.1.10", "192.168.1.1", "192.168.1.9", true);
		contains("192.168.1.0", "192.168.1.10", "192.168.1.2", "192.168.1.2", true);
		contains("192.168.1.0", "192.168.1.10", "192.168.1.1", "192.168.1.11", false);
		contains("192.168.1.0", "192.168.1.10", "192.168.0.255", "192.168.1.10", false);
		contains("192.168.1.0", "192.168.1.10", "192.168.0.255", "192.168.1.11", false);
		contains("192.168.1.1", "192.168.1.1", "192.168.1.0", "192.168.1.0", false);
		contains("192.168.1.1", "192.168.1.1", "192.168.1.2", "192.168.1.2", false);
		contains("192.168.1.0", "192.168.1.10", "fec1::", "fec1::10", false);
		contains("fec1::", "fec1::10", "192.168.1.0", "192.168.1.10", false);
	}

	private void overlaps(String af, String at, String bf, String bt, boolean exp) throws UnknownHostException
	{
		IPRange a = new IPRange(af, at);
		IPRange b = new IPRange(bf, bt);
		assertEquals("b overlaps with a", exp, a.overlaps(b));
		assertEquals("a overlaps with b", exp, b.overlaps(a));
	}

	@Test
	public void testOverlaps() throws UnknownHostException
	{
		overlaps("192.168.1.0", "192.168.1.10", "192.168.1.0", "192.168.1.10", true);
		overlaps("192.168.1.0", "192.168.1.10", "192.168.1.1", "192.168.1.9", true);
		overlaps("192.168.1.0", "192.168.1.10", "192.168.1.10", "192.168.1.20", true);
		overlaps("192.168.1.0", "192.168.1.10", "192.168.1.9", "192.168.1.20", true);
		overlaps("192.168.1.0", "192.168.1.10", "192.168.1.11", "192.168.1.20", false);
		overlaps("192.168.1.0", "192.168.1.10", "192.168.0.245", "192.168.1.1", true);
		overlaps("192.168.1.0", "192.168.1.10", "192.168.0.245", "192.168.1.0", true);
		overlaps("192.168.1.0", "192.168.1.10", "192.168.0.245", "192.168.0.255", false);
		overlaps("192.168.1.0", "192.168.1.10", "fec1::", "fec1::10", false);
	}

	private void remove(String af, String at, String bf, String bt, IPRange...exp) throws UnknownHostException
	{
		IPRange a = new IPRange(af, at);
		IPRange b = new IPRange(bf, bt);
		List<IPRange> l = a.remove(b);
		assertEquals("ranges", exp.length, l.size());
		for (int i = 0; i < exp.length; i++)
		{
			assertEquals("range", exp[i], l.get(i));
		}
	}

	@Test
	public void testRemove() throws UnknownHostException
	{
		remove("192.168.1.0", "192.168.1.10", "192.168.1.0", "192.168.1.10");
		remove("192.168.1.0", "192.168.1.10", "192.168.0.0", "192.168.1.255");
		remove("192.168.1.0", "192.168.1.10", "10.0.1.0", "10.0.1.10",
			   new IPRange("192.168.1.0", "192.168.1.10"));
		remove("192.168.1.0", "192.168.1.10", "192.168.1.0", "192.168.1.5",
			   new IPRange("192.168.1.6", "192.168.1.10"));
		remove("192.168.1.0", "192.168.1.10", "192.168.1.6", "192.168.1.10",
			   new IPRange("192.168.1.0", "192.168.1.5"));
		remove("192.168.1.0", "192.168.1.10", "192.168.0.0", "192.168.1.5",
			   new IPRange("192.168.1.6", "192.168.1.10"));
		remove("192.168.1.0", "192.168.1.10", "192.168.1.6", "192.168.1.255",
			   new IPRange("192.168.1.0", "192.168.1.5"));
		remove("192.168.1.0", "192.168.1.10", "192.168.1.1", "192.168.1.9",
			   new IPRange("192.168.1.0", "192.168.1.0"), new IPRange("192.168.1.10", "192.168.1.10"));
		remove("192.168.1.0", "192.168.1.10", "192.168.1.3", "192.168.1.7",
			   new IPRange("192.168.1.0", "192.168.1.2"), new IPRange("192.168.1.8", "192.168.1.10"));
		remove("192.168.0.0", "192.168.1.255", "192.168.1.0", "192.168.1.255",
			   new IPRange("192.168.0.0", "192.168.0.255"));
		remove("192.168.0.0", "192.168.1.255", "192.168.0.0", "192.168.0.255",
			   new IPRange("192.168.1.0", "192.168.1.255"));
		remove("192.168.1.0", "192.168.1.10", "0.0.0.0", "192.168.1.10");
		remove("192.168.1.0", "192.168.1.10", "192.168.1.0", "255.255.255.255");
	}

	private void merge(String af, String at, String bf, String bt, IPRange exp) throws UnknownHostException
	{
		IPRange a = new IPRange(af, at);
		IPRange b = new IPRange(bf, bt);
		IPRange r = a.merge(b);
		assertEquals("merge", exp, r);
		r = b.merge(a);
		assertEquals("reverse", exp, r);
	}

	@Test
	public void testMerge() throws UnknownHostException
	{
		merge("192.168.1.0", "192.168.1.10", "192.168.1.0", "192.168.1.10", new IPRange("192.168.1.0", "192.168.1.10"));
		merge("192.168.1.0", "192.168.1.10", "192.168.0.0", "192.168.1.10", new IPRange("192.168.0.0", "192.168.1.10"));
		merge("192.168.1.0", "192.168.1.10", "192.168.0.0", "192.168.0.255", new IPRange("192.168.0.0", "192.168.1.10"));
		merge("192.168.1.0", "192.168.1.10", "192.168.1.0", "192.168.1.255", new IPRange("192.168.1.0", "192.168.1.255"));
		merge("192.168.1.0", "192.168.1.10", "192.168.1.11", "192.168.1.255", new IPRange("192.168.1.0", "192.168.1.255"));
		merge("192.168.1.0", "192.168.1.10", "192.168.0.0", "192.168.1.255", new IPRange("192.168.0.0", "192.168.1.255"));
		merge("255.255.255.0", "255.255.255.255", "0.0.0.0", "0.0.0.255", null);
		merge("0.0.0.1", "255.255.255.255", "0.0.0.0", "0.0.0.0", new IPRange("0.0.0.0", 0));
		merge("0.0.0.0", "255.255.255.254", "255.255.255.255", "255.255.255.255", new IPRange("0.0.0.0", 0));
		merge("192.168.1.0", "192.168.1.10", "192.168.0.0", "192.168.0.254", null);
		merge("192.168.1.0", "192.168.1.10", "192.168.1.12", "192.168.1.255", null);
		merge("192.168.1.0", "192.168.1.10", "fec1::0", "fec1::10", null);
		merge("fec1::1:0", "fec1::1:10", "fec1::1:8", "fec1::1:20", new IPRange("fec1::1:0", "fec1::1:20"));
	}

	private void toSubnet(String f, String t, IPRange...exp) throws UnknownHostException
	{
		IPRange a = new IPRange(f, t);
		List<IPRange> l = a.toSubnets();
		assertEquals("ranges", exp.length, l.size());
		for (int i = 0; i < exp.length; i++)
		{
			assertEquals("range", exp[i], l.get(i));
		}
	}

	@Test
	public void testToSubnet() throws UnknownHostException
	{
		toSubnet("0.0.0.0", "255.255.255.255", new IPRange("0.0.0.0", 0));
		toSubnet("192.168.1.1", "192.168.1.1", new IPRange("192.168.1.1", 32));
		toSubnet("192.168.1.0", "192.168.1.255", new IPRange("192.168.1.0", 24));
		toSubnet("192.168.1.0", "192.168.1.10", new IPRange("192.168.1.0", 29),
				 new IPRange("192.168.1.8", 31), new IPRange("192.168.1.10", 32));
		toSubnet("192.168.1.1", "192.168.1.10", new IPRange("192.168.1.1", 32),
				 new IPRange("192.168.1.2", 31), new IPRange("192.168.1.4", 30),
				 new IPRange("192.168.1.8", 31), new IPRange("192.168.1.10", 32));
		toSubnet("192.168.1.241", "192.168.1.255", new IPRange("192.168.1.241", 32),
				new IPRange("192.168.1.242", 31), new IPRange("192.168.1.244", 30),
				new IPRange("192.168.1.248", 29));
		toSubnet("192.168.254.255", "192.168.255.1", new IPRange("192.168.254.255", 32),
				 new IPRange("192.168.255.0", 31));
		toSubnet("::", "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", new IPRange("::", 0));
		toSubnet("fec1::0", "fec1::a", new IPRange("fec1::0", 125), new IPRange("fec1::8", 127),
				new IPRange("fec1::a", 128));
	}
}
