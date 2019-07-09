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

import java.io.IOException;
import java.io.StringReader;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Xml;

public class RemediationInstruction implements Parcelable
{
	private String mTitle;
	private String mDescription;
	private String mHeader;
	private final List<String> mItems = new LinkedList<String>();

	@Override
	public int describeContents()
	{
		return 0;
	}

	@Override
	public void writeToParcel(Parcel dest, int flags)
	{
		dest.writeString(mTitle);
		dest.writeString(mDescription);
		dest.writeString(mHeader);
		dest.writeStringList(mItems);
	}

	public static final Parcelable.Creator<RemediationInstruction> CREATOR = new Creator<RemediationInstruction>() {

		@Override
		public RemediationInstruction[] newArray(int size)
		{
			return new RemediationInstruction[size];
		}

		@Override
		public RemediationInstruction createFromParcel(Parcel source)
		{
			return new RemediationInstruction(source);
		}
	};

	private RemediationInstruction()
	{
	}

	private RemediationInstruction(Parcel source)
	{
		mTitle = source.readString();
		mDescription = source.readString();
		mHeader = source.readString();
		source.readStringList(mItems);
	}

	public String getTitle()
	{
		return mTitle;
	}

	private void setTitle(String title)
	{
		mTitle = title;
	}

	public String getDescription()
	{
		return mDescription;
	}

	private void setDescription(String description)
	{
		mDescription = description;
	}

	public String getHeader()
	{
		return mHeader;
	}

	private void setHeader(String header)
	{
		mHeader = header;
	}

	public List<String> getItems()
	{
		return Collections.unmodifiableList(mItems);
	}

	private void addItem(String item)
	{
		mItems.add(item);
	}

	/**
	 * Create a list of RemediationInstruction objects from the given XML data.
	 *
	 * @param xml XML data
	 * @return list of RemediationInstruction objects
	 */
	public static List<RemediationInstruction> fromXml(String xml)
	{
		List<RemediationInstruction> instructions = new LinkedList<RemediationInstruction>();
		XmlPullParser parser = Xml.newPullParser();
		try
		{
			parser.setInput(new StringReader(xml));
			parser.nextTag();
			readInstructions(parser, instructions);
		}
		catch (XmlPullParserException e)
		{
			e.printStackTrace();
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		return instructions;
	}

	/**
	 * Read a &lt;remediationinstructions&gt; element and store the extracted
	 * RemediationInstruction objects in the given list.
	 *
	 * @param parser
	 * @param instructions
	 * @throws XmlPullParserException
	 * @throws IOException
	 */
	private static void readInstructions(XmlPullParser parser, List<RemediationInstruction> instructions) throws XmlPullParserException, IOException
	{
		parser.require(XmlPullParser.START_TAG, null, "remediationinstructions");
		while (parser.next() != XmlPullParser.END_TAG)
		{
			if (parser.getEventType() != XmlPullParser.START_TAG)
			{
				continue;
			}
			if (parser.getName().equals("instruction"))
			{
				RemediationInstruction instruction = new RemediationInstruction();
				readInstruction(parser, instruction);
				instructions.add(instruction);
			}
			else
			{
				skipTag(parser);
			}
		}
	}

	/**
	 * Read an &lt;instruction&gt; element and store the information in the
	 * given RemediationInstruction object.
	 *
	 * @param parser
	 * @param instruction
	 * @throws XmlPullParserException
	 * @throws IOException
	 */
	private static void readInstruction(XmlPullParser parser, RemediationInstruction instruction) throws XmlPullParserException, IOException
	{
		parser.require(XmlPullParser.START_TAG, null, "instruction");
		while (parser.next() != XmlPullParser.END_TAG)
		{
			if (parser.getEventType() != XmlPullParser.START_TAG)
			{
				continue;
			}
			String name = parser.getName();
			if (name.equals("title"))
			{
				instruction.setTitle(parser.nextText());
			}
			else if (name.equals("description"))
			{
				instruction.setDescription(parser.nextText());
			}
			else if (name.equals("itemsheader"))
			{
				instruction.setHeader(parser.nextText());
			}
			else if (name.equals("items"))
			{
				readItems(parser, instruction);
			}
			else
			{
				skipTag(parser);
			}
		}
	}

	/**
	 * Read all items of an &lt;items&gt; node and add them to the given
	 * RemediationInstruction object.
	 *
	 * @param parser
	 * @param instruction
	 * @throws XmlPullParserException
	 * @throws IOException
	 */
	private static void readItems(XmlPullParser parser, RemediationInstruction instruction) throws XmlPullParserException, IOException
	{
		while (parser.next() != XmlPullParser.END_TAG)
		{
			if (parser.getEventType() != XmlPullParser.START_TAG)
			{
				continue;
			}
			if (parser.getName().equals("item"))
			{
				instruction.addItem(parser.nextText());
			}
			else
			{
				skipTag(parser);
			}
		}
	}

	/**
	 * Skip the current tag and all child elements.
	 *
	 * @param parser
	 * @throws XmlPullParserException
	 * @throws IOException
	 */
	private static void skipTag(XmlPullParser parser) throws XmlPullParserException, IOException
	{
		int depth = 1;

		parser.require(XmlPullParser.START_TAG, null, null);
		while (depth != 0)
		{
			switch (parser.next())
			{
				case XmlPullParser.END_TAG:
					depth--;
					break;
				case XmlPullParser.START_TAG:
					depth++;
					break;
			}
		}
	}
}
