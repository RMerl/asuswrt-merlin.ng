package com.broadcom.bcacpe.cmsxml2bbfxml;

import java.io.IOException;
import java.io.BufferedWriter;
	 
import java.text.StringCharacterIterator;
import java.text.CharacterIterator;

import java.util.List;
import java.util.UUID;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
	 
import org.xml.sax.Attributes;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import static com.broadcom.bcacpe.cmsxml2bbfxml.Cms2BbfParserConstant.*;

public class Cms2BbfParserHandler extends DefaultHandler {
    boolean endObj = false, endParam = false;
    String tmpValue = null;
    Cms2BbfDmObj cms2BbfDmObj = null;
    Cms2BbfDmParam cms2BbfDmParam = null;
    BufferedWriter writer = null;

    public Cms2BbfParserHandler(InputSource inputSource, BufferedWriter writer) {
        this.writer = writer;
        parseDocument(inputSource, writer);
    }

    private void parseDocument(InputSource inputSource, BufferedWriter writer) {
        // parse
        SAXParserFactory factory = SAXParserFactory.newInstance();
        UUID uuid = UUID.randomUUID();
        try {
            SAXParser parser = factory.newSAXParser();
            writer.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
            writer.write("<dm:document xmlns:dm=\"urn:broadband-forum-org:cwmp:datamodel-1-5\"");
            writer.write(" uuid=\"" + uuid.toString() + "\">\n");
//            writer.write("             xmlns:dmr=\"urn:broadband-forum-org:cwmp:datamodel-report-0-1\"\n");
//            writer.write("             xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n");
//            writer.write("             xsi:schemaLocation=\"urn:broadband-forum-org:cwmp:datamodel-1-5 http://www.broadband-forum.org/cwmp/cwmp-datamodel-1-5.xsd urn:broadband-forum-org:cwmp:datamodel-report-0-1 http://www.broadband-forum.org/cwmp/cwmp-datamodel-report.xsd\"\n");
//            writer.write("             spec=\"urn:broadband-forum-org:tr-181-2-10-0\" file=\"tr-181-2-10-0\">\n");
            writer.write("  <model>\n");

            parser.parse(inputSource, this);

            writer.write("  </model>\n");
            writer.write("</dm:document>\n\n");
        } catch (ParserConfigurationException e) {
            System.out.println("ParserConfig error");
        } catch (SAXException e) {
            System.out.println("SAXException : xml not well formed");
            System.out.println("Exception: " + e.getMessage() + " " + e.getClass().getName());
            //e.printStackTrace();
        } catch (IOException e) {
            System.out.println("IO error");
        }
    }

    @Override
    public void startElement(String s, String s1, String elementName, Attributes attributes) throws SAXException {
        // if current element is cms2BbfDmObj,
        if (elementName.equalsIgnoreCase("object")) {
            try {
                // if old cms2BbfDmObj exists,
                if (cms2BbfDmObj != null) {
                    // if old cms2BbfDmParam exists,
                    if (cms2BbfDmParam != null) {
                        // add cms2BbfDmParam into cms2BbfDmObj,
                        cms2BbfDmObj.addParam(cms2BbfDmParam);
                        // delete old cms2BbfDmParam
                        cms2BbfDmParam = null;
                    }

                    // write its information to file,
                    writer.write(cms2BbfDmObj.toString());
                    // delete old cms2BbfDmObj
                    cms2BbfDmObj = null;
                }
            } catch (IOException e) {
                e.printStackTrace();
            }

            // create new cms2BbfDmObj
            cms2BbfDmObj = new Cms2BbfDmObj();

            // set object information
            if (attributes.getValue("name") != null) {
				String name = new String(attributes.getValue("name"));
				// set object name
                cms2BbfDmObj.setName(name);
				// set object access
                cms2BbfDmObj.setAccess(name, attributes.getValue("supportLevel"));
				// set object numEntriesParameter
                cms2BbfDmObj.setNumEntriesParameter(name);
				// set object min and max entries
                cms2BbfDmObj.setMinMaxEntries(name);
                name = null;
            }
        }

        // if current element is cms2BbfDmParam,
        if (elementName.equalsIgnoreCase("parameter")) {
            // if old cms2BbfDmParam exists,
            if (cms2BbfDmObj != null && cms2BbfDmParam != null) {
                // add cms2BbfDmParam into cms2BbfDmObj,
                cms2BbfDmObj.addParam(cms2BbfDmParam);
                // delete old cms2BbfDmParam
                cms2BbfDmParam = null;
            }

            // create new cms2BbfDmParam
            cms2BbfDmParam = new Cms2BbfDmParam();

            // set parameter information
            cms2BbfDmParam.setName(attributes.getValue("name"));
            cms2BbfDmParam.setAccess(attributes.getValue("requirements"),
                                     attributes.getValue("supportLevel"));
            cms2BbfDmParam.setDataType(attributes.getValue("type"));
            cms2BbfDmParam.setActiveNotify(attributes.getValue("mayDenyActiveNotification"));
            cms2BbfDmParam.setDefaultValue(attributes.getValue("defaultValue"));
            cms2BbfDmParam.setMinValue(attributes.getValue("minValue"));
            cms2BbfDmParam.setMaxValue(attributes.getValue("maxValue"));
            cms2BbfDmParam.setMaxLength(attributes.getValue("maxLength"));
        }

        // if start element is description
        // release tmpValue for not keeping unwanted info.
        if (elementName.equalsIgnoreCase("description")) {
            tmpValue = null;
        }
    }

    @Override
    public void endElement(String s, String s1, String element) throws SAXException {
        // if current element is object
        if (element.equalsIgnoreCase("object")) {
            endObj = true;
        }

        // if current element is parameter
        if (element.equalsIgnoreCase("parameter")) {
            endParam = true;
        }

        // if current element is description
        if (element.equalsIgnoreCase("description")) {
            if (endObj == true && cms2BbfDmObj != null) {
                cms2BbfDmObj.setDescription(tmpValue);
                endObj = false;
            }
            if (endParam == true && cms2BbfDmParam != null) {
                cms2BbfDmParam.setDescription(tmpValue);
                endParam = false;
            }
            tmpValue = null;
        }

        // if current element is xmlMandatorySingleRootNode
        if (element.equalsIgnoreCase("xmlMandatorySingleRootNode")) {
            // write the last object
            try {
                // if old cms2BbfDmObj exists,
                if (cms2BbfDmObj != null) {
                    // if old cms2BbfDmParam exists,
                    if (cms2BbfDmParam != null) {
                        // add cms2BbfDmParam into cms2BbfDmObj,
                        cms2BbfDmObj.addParam(cms2BbfDmParam);
                        // delete old cms2BbfDmParam
                        cms2BbfDmParam = null;
                    }

                    // write its information to file,
                    writer.write(cms2BbfDmObj.toString());
                    // delete old cms2BbfDmObj
                    cms2BbfDmObj = null;
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void characters(char[] ac, int i, int j) throws SAXException {
    	String val = new String(ac,i, j);

        // replace '<' with "&lt;" and '>' with "&gt;"
        val = forXML(val);

        if (tmpValue == null)
            tmpValue = new String(val);
    	else
            tmpValue += new String(val);
    }

    private static String forXML(String aText) {
        final StringBuilder result = new StringBuilder();
        final StringCharacterIterator iterator = new StringCharacterIterator(aText);
        char character = iterator.current();

        while (character != CharacterIterator.DONE ) {
            if (character == '<') {
                result.append("&lt;");
            }
            else if (character == '>') {
                result.append("&gt;");
            }
            /*else if (character == '\"') {
                result.append("&quot;");
            }
            else if (character == '\'') {
                result.append("&#039;");
            }
            else if (character == '&') {
                result.append("&amp;");
            }*/
            else {
                //the char is not a special one
                //add it to the result as is
                result.append(character);
            }

            character = iterator.next();
        }

        return result.toString();
    }

}
