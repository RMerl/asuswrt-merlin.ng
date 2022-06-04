package com.broadcom.bcacpe.cmsxml2bbfxml;

import java.util.Scanner;

import static com.broadcom.bcacpe.cmsxml2bbfxml.Cms2BbfParserConstant.*;

public class Cms2BbfDmParam {
    boolean isNotSupported = false;
    String paramName = null;
    String paramDescription = null;
    String paramAccess = null;
    String paramType = null;
    String paramActiveNotify = null;
    String paramDefaultValue = null;
    String paramMinValue = null;
    String paramMaxValue = null;
    String paramMaxLength = null;

    public Cms2BbfDmParam() {
    }

    public void setName(String name) {
        if (name != null)
            paramName = new String(name);
    }
  
    public void setDescription(String description) {
        if (description != null && paramDescription == null) {
            Scanner sc = new Scanner(description);
            paramDescription = new String();
            while (sc.hasNextLine()) {
                paramDescription += sc.nextLine() + " ";
            }
        }
    }

    public void setAccess(String requirements, String supportLevel) {
        if (supportLevel.equalsIgnoreCase(PARAM_NOT_SUPPORTED))
            isNotSupported = true;

        if (requirements != null) {
  	        if (requirements.equals("R"))
                paramAccess = new String(ACCESS_READ);
  	        else if (requirements.equals("W"))
                paramAccess = new String(ACCESS_WRITE);
            else
                paramAccess = new String(requirements);
        }
        else if (supportLevel != null) {
  	        if (supportLevel.equals("ReadOnly"))
                paramAccess = new String(ACCESS_READ);
  	        else if (supportLevel.equals("ReadWrite"))
                paramAccess = new String(ACCESS_WRITE);
            else
                paramAccess = new String(supportLevel);
        }
    }
  
    public void setDataType(String value) {
        if (value != null)
            paramType = new String(value);
    }
  
    public void setActiveNotify(String value) {
        if (value != null)
            paramActiveNotify = new String(value);
    }
  
    public void setDefaultValue(String value) {
        if (value != null)
            paramDefaultValue = new String(value);
    }
  
    public void setMinValue(String value) {
        if (value != null)
            paramMinValue = new String(value);
    }
  
    public void setMaxValue(String value) {
        if (value != null)
            paramMaxValue = new String(value);
    }
  
    public void setMaxLength(String value) {
        if (value != null)
            paramMaxLength = new String(value);
    }

    @Override
    public String toString() {
        String str = null;

        if (isNotSupported == true) {
            str = new String("");
            return str;
        }

        str = new String("      <parameter");

        if (paramName != null)
            str += " name=\"" + paramName + "\"";
  	
        if (paramAccess != null)
            str += " access=\"" + paramAccess + "\"";
  	
        if (paramActiveNotify != null)
            str += " activeNotify=\"canDeny\"";

        str += ">\n";

        if (paramDescription != null)
            str += "        <description>" + paramDescription.trim() + "</description>\n";

        if (paramType != null) {
            str += "        <syntax>\n";

  	        if (paramType.equals("string")) {
                if (paramMaxLength != null) {
                    str += "          <string>\n";
                    str += "            <size maxLength=\"" + paramMaxLength + "\"/>\n";
                    str += "          </string>\n";
                }
                else 
                    str += "          <string/>\n";
  	        }
  	        else if (paramType.equals("base64")) {
                if (paramMaxLength != null) {
                    str += "          <base64>\n";
                    str += "            <size minLength=\"0\" maxLength=\"" + paramMaxLength + "\"/>\n";
                    str += "          </base64>\n";
                }
                else 
                    str += "          <base64/>\n";
  	        }
  	        else if (paramType.equals("hexBinary")) {
                if (paramMaxLength != null) {
                    str += "          <hexBinary>\n";
                    str += "            <size maxLength=\"" + paramMaxLength + "\"/>\n";
                    str += "          </hexBinary>\n";
                }
                else 
                    str += "          <hexBinary/>\n";
  	        }
  	        else if (paramType.equals("int")) {
                if (paramMinValue != null || paramMaxValue != null) {
                    str += "          <int>\n";
                    str += "            <range";
                    if (paramMinValue != null)
                        str += " minInclusive=\"" + paramMinValue + "\"";
                    if (paramMaxValue != null)
                        str += " maxInclusive=\"" + paramMaxValue + "\"";
                    str += "/>\n";
                    str += "          </int>\n";
                }
                else 
                    str += "          <int/>\n";
  	        }
  	        else if (paramType.equals("unsignedInt")) {
                if (paramMinValue != null || paramMaxValue != null) {
                    str += "          <unsignedInt>\n";
                    str += "            <range";
                    if (paramMinValue != null)
                        str += " minInclusive=\"" + paramMinValue + "\"";
                    if (paramMaxValue != null)
                        str += " maxInclusive=\"" + paramMaxValue + "\"";
                    str += "/>\n";
                    str += "          </unsignedInt>\n";
                }
                else 
                    str += "          <unsignedInt/>\n";
  	        }
  	        else if (paramType.equals("boolean"))
                str += "          <boolean/>\n";
  	        else if (paramType.equalsIgnoreCase("DateTime"))
                str += "          <dateTime/>\n";

            if (paramDefaultValue != null)
                str += "          <default type=\"object\" value=\"" + paramDefaultValue + "\"/>\n";
				
            str += "        </syntax>\n";
        }

        str += "      </parameter>\n";
      
        return str;
    }
}
