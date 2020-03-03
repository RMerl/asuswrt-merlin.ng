package com.broadcom.bcacpe.cmsxml2bbfxml;

import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;

import static com.broadcom.bcacpe.cmsxml2bbfxml.Cms2BbfParserConstant.*;

public class Cms2BbfDmObj {
    boolean isNotSupported = false;
    String objName = null;
    String objDescription = null;
    String objAccess = null;
    String objNumEntriesParameter = null;
    String objMinEntries = null;
    String objMaxEntries = null;
    List<Cms2BbfDmParam> cms2BbfDmParamList;

    public Cms2BbfDmObj() {
        cms2BbfDmParamList = new ArrayList<Cms2BbfDmParam>();
    }

    public void addParam(Cms2BbfDmParam cms2BbfDmParam) {
        cms2BbfDmParamList.add(cms2BbfDmParam);
    }

    public void setName(String name) {
        if (name != null)
            objName = new String(name);
    }
    
    public void setDescription(String description) {
        if (description != null && objDescription == null) {
            Scanner sc = new Scanner(description);
            objDescription = new String();
            while (sc.hasNextLine()) {
                objDescription += sc.nextLine() + " ";
            }
        }
    }

    public void setAccess(String name, String supportLevel) {
        if (supportLevel.equalsIgnoreCase(PARAM_NOT_SUPPORTED))
            isNotSupported = true;
		
        if (name != null) {
            if (isObjInstanceName(name) == true) {
                if (supportLevel != null &&
                    supportLevel.equals("MultipleInstances") == true)
                    objAccess = new String(ACCESS_READ);
                else
                    objAccess = new String(ACCESS_WRITE);
            }
            else
                objAccess = new String(ACCESS_READ);
        }
        else
            objAccess = new String();
    }

    public void setNumEntriesParameter(String name) {
    	boolean found = false;
    	
    	if (name != null) {
            String str = new String(name);
            String[] splitNames = str.split(DOT_DELIMITER);
    	    int len = splitNames.length;
    	
            if ((len > 1) && (splitNames[len-1].compareToIgnoreCase(OBJ_INSTANCE) == 0)) {
    	        objNumEntriesParameter = new String(splitNames[len-2]+"NumberOfEntries");
    	        found = true;
            }
    	}
    	
    	if (found == false)
            objNumEntriesParameter = new String();
    }

    public void setMinMaxEntries(String name) {
        if (name != null) {
            if (isObjInstanceName(name) == false) {
                objMinEntries = new String("1");
                objMaxEntries = new String("1");
            }
            else {
                objMinEntries = new String("0");
                objMaxEntries = new String(OBJ_MAX_VALUE_UNBOUNDED);
            }
        }
        else {
            objMinEntries = new String();
            objMinEntries = new String();
        }
    }
    
    @Override
    public String toString() {
    	String str = null;

        if (isNotSupported == true) {
            str = new String("");
            return str;
        }

        str = new String("    <object ");
    	
    	if (objName != null) {
            str += "name=\"" + objName + "\"";
        }

    	if (objAccess != null) {
            str += " access=\"" + objAccess + "\"";
        }

    	if (objNumEntriesParameter != null &&
			objNumEntriesParameter.equals("") == false) {
            str += " numEntriesParameter=\"" + objNumEntriesParameter + "\"";
        }

    	if (objMinEntries != null) {
            str += " minEntries=\"" + objMinEntries + "\"";
        }

    	if (objMaxEntries != null) {
            str += " maxEntries=\"" + objMaxEntries + "\"";

        }

    	str += ">\n";

    	if (objDescription != null)
            str += "      <description>" + objDescription.trim() + "</description>\n";

        for (Cms2BbfDmParam cms2BbfDmParam : cms2BbfDmParamList) {
            str += cms2BbfDmParam.toString();
        }

        str += "    </object>\n";
        
        return str;
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            //for (Cms2BbfDmParam tmp : cms2BbfDmParamList) {
            //    tmp = null;
            //}
        } catch (Throwable t) {
            throw t;
        } finally {
            super.finalize();
        }
    }

    private boolean isObjInstanceName(String name) {
        boolean ret = false;

        if (name != null) {
            String str = new String(name);
            String[] splitNames = str.split(DOT_DELIMITER);
            int len = splitNames.length;

            // return true if last tring in name is "{i}"
            if ((len > 0) &&
                splitNames[len-1].compareToIgnoreCase(OBJ_INSTANCE) == 0)
                ret = true;
        }

        return ret;
    }
}
