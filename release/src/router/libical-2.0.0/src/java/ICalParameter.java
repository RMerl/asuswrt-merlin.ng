/*======================================================================
 FILE: ICalParameter.java
 CREATOR: gnorman 01/09/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

package net.cp.jlibical;

public class ICalParameter
{
	/** It's not typesafe, but it's simple to understand! */
	public interface ICalParameterKind
	{
		// icalparameter_kind
		int ICAL_ANY_PARAMETER = 0;
		int ICAL_ALTREP_PARAMETER = 1;
		int ICAL_CN_PARAMETER = 2;
		int ICAL_CUTYPE_PARAMETER = 3;
		int ICAL_DELEGATEDFROM_PARAMETER = 4;
		int ICAL_DELEGATEDTO_PARAMETER = 5;
		int ICAL_DIR_PARAMETER = 6;
		int ICAL_ENCODING_PARAMETER = 7;
		int ICAL_FBTYPE_PARAMETER = 8;
		int ICAL_FMTTYPE_PARAMETER = 9;
		int ICAL_LANGUAGE_PARAMETER = 10;
		int ICAL_MEMBER_PARAMETER = 11;
		int ICAL_PARTSTAT_PARAMETER = 12;
		int ICAL_RANGE_PARAMETER = 13;
		int ICAL_RELATED_PARAMETER = 14;
		int ICAL_RELTYPE_PARAMETER = 15;
		int ICAL_RIGHTREF_PARAMETER = 16;
		int ICAL_ROLE_PARAMETER = 17;
		int ICAL_RSVP_PARAMETER = 18;
		int ICAL_SENTBY_PARAMETER = 19;
		int ICAL_TZID_PARAMETER = 20;
		int ICAL_VALUE_PARAMETER = 21;
		int ICAL_WKST_PARAMETER = 22;
		int ICAL_X_PARAMETER = 23;
		int ICAL_XLICCOMPARETYPE_PARAMETER = 24;
		int ICAL_XLICERRORTYPE_PARAMETER = 25;
		int ICAL_NO_PARAMETER = 26;
	}

	/** It's not typesafe, but it's simple to understand! */
	public interface ICalParameterEncoding
	{
		// icalparameter_encoding
		int ICAL_ENCODING_X = 20007;
		int ICAL_ENCODING_8BIT = 20008;
		int ICAL_ENCODING_BASE64 = 20009;
		int ICAL_ENCODING_NONE = 20010;
	}

	public interface ICalParameterPartStat {
	    int ICAL_PARTSTAT_X = 20017;
	    int ICAL_PARTSTAT_NEEDSACTION = 20018;
	    int ICAL_PARTSTAT_ACCEPTED = 20019;
	    int ICAL_PARTSTAT_DECLINED = 20020;
	    int ICAL_PARTSTAT_TENTATIVE = 20021;
	    int ICAL_PARTSTAT_DELEGATED = 20022;
	    int ICAL_PARTSTAT_COMPLETED = 20023;
	    int ICAL_PARTSTAT_INPROCESS = 20024;
	    int ICAL_PARTSTAT_NONE = 20025;
	}

	public interface ICalParameterRole {
	    int ICAL_ROLE_X = 20047;
	    int ICAL_ROLE_CHAIR = 20048;
	    int ICAL_ROLE_REQPARTICIPANT = 20049;
	    int ICAL_ROLE_OPTPARTICIPANT = 20050;
	    int ICAL_ROLE_NONPARTICIPANT = 20051;
	    int ICAL_ROLE_NONE = 20052;
	}

	/**
	 * Constructor for ICalParameter
	 * @param obj c++ pointer
	 */
	private ICalParameter(long obj)
	{
		init(obj);
	}

	/**
	 * Constructor for ICalParameter
	 */
	public ICalParameter()
	{
		init();
	}

	/**
	 * Create instance from a string of form "PARAMNAME=VALUE"
	 */
	public ICalParameter(String str)
	{
		init(str);
	}

	/**
	 * Create from just the value, the part after the "="
	 */
	public ICalParameter(/* ICalParameterKind */ int kind, String  str)
	{
		init(kind,str);
	}

	/**
	 * Create empty value for specified kind
	 */
	public ICalParameter(/* ICalParameterKind */ int kind)
	{
		init(kind);
	}

	/**
	 * Return ical String representation
	 */
	public native String as_ical_string();

	/**
	 * Return true if this instance is valid
	 */
	//public native boolean is_valid();

	/**
	 * Return what kind of parameter this instance represents
	 */
	public native /* ICalParameterKind */ int isa();

	/**
	 * Return true if this is a parameter
	 */
	public native boolean isa_parameter(Object parameter);

	/* Convert enumerations */
	//public native static String kind_to_string(/* ICalParameterKind */ int kind);
	//public native static /* ICalParameterKind */ int string_to_kind(String  str);

	/* DELEGATED-FROM */
	//public native String get_delegatedfrom();
	//public native void set_delegatedfrom(String  v);

	/* RELATED */
	//public native icalparameter_related get_related();
	//public native void set_related(icalparameter_related v);

	/* SENT-BY */
	//public native String get_sentby();
	//public native void set_sentby(String  v);

	/* LANGUAGE */
	public native String get_language();
	public native void set_language(String  v);

	/* RELTYPE */
	//public native icalparameter_reltype get_reltype();
	//public native void set_reltype(icalparameter_reltype v);

	/* ENCODING */
	public native /* ICalParameterEncoding */ int get_encoding();
	public native void set_encoding(/* ICalParameterEncoding */ int v);

	/* ALTREP */
	//public native String get_altrep();
	//public native void set_altrep(String  v);

	/* FMTTYPE */
	//public native String get_fmttype();
	//public native void set_fmttype(String  v);

	/* FBTYPE */
	//public native icalparameter_fbtype get_fbtype();
	//public native void set_fbtype(icalparameter_fbtype v);

	/* RSVP */
	//public native icalparameter_rsvp get_rsvp();
	//public native void set_rsvp(icalparameter_rsvp v);

	/* RANGE */
	//public native icalparameter_range get_range();
	//public native void set_range(icalparameter_range v);

	/* DELEGATED-TO */
	//public native String get_delegatedto();
	//public native void set_delegatedto(String  v);

	/* CN */
	//public native String get_cn();
	//public native void set_cn(String  v);

	/* ROLE */
	public native /* ICalParameterRole */ int get_role();
	public native void set_role(/* ICalParameterRole */ int v);

	/* X-LIC-COMPARETYPE */
	//public native icalparameter_xliccomparetype get_xliccomparetype();
	//public native void set_xliccomparetype(icalparameter_xliccomparetype v);

	/* PARTSTAT */
	public native /* ICalParameterPartStat */ int get_partstat();
	public native void set_partstat(/* ICalParameterPartStat */ int v);

	/* X-LIC-ERRORTYPE */
	//public native icalparameter_xlicerrortype get_xlicerrortype();
	//public native void set_xlicerrortype(icalparameter_xlicerrortype v);

	/* MEMBER */
	//public native String get_member();
	//public native void set_member(String  v);

	/* X */
	//public native String get_x();
	//public native void set_x(String  v);

	/* CUTYPE */
	//public native icalparameter_cutype get_cutype();
	//public native void set_cutype(icalparameter_cutype v);

	/* TZID */
	//public native String get_tzid();
	//public native void set_tzid(String  v);

	/* VALUE */
	//public native /* ICalParameterValue */ int get_value();
	//public native void set_value(/* ICalParameterValue */ int v);

	/* DIR */
	//public native String get_dir();
	//public native void set_dir(String  v);

	/**
	 * init the native class
	 */
	private void init(long obj)
	{
		m_Obj = obj;
	}

	private native void init();
	private native void init(String str);
	private native void init(/* ICalParameterKind */ int kind, String str);
	private native void init(/* ICalParameterKind */ int kind);

	/**
	 * load the jni library for this class
	 */
	static {
		System.loadLibrary("ical_jni");
	}

	public static void main(String[] args)
	{
		System.out.println("*** ICalParameter main called ok.");
	}

	/** pointer to C++ object  */
	private long	m_Obj = 0;
}
