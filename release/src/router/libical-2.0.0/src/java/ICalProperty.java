/*======================================================================
 FILE: ICalProperty.java
 CREATOR: gnorman 01/09/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

package net.cp.jlibical;

public class ICalProperty
{
	/** It's not typesafe, but it's simple to understand! */
	public interface ICalPropertyMethod
	{
		// icalproperty_method
		int ICAL_METHOD_X = 10011;
		int ICAL_METHOD_PUBLISH = 10012;
		int ICAL_METHOD_REQUEST = 10013;
		int ICAL_METHOD_REPLY = 10014;
		int ICAL_METHOD_ADD = 10015;
		int ICAL_METHOD_CANCEL = 10016;
		int ICAL_METHOD_REFRESH = 10017;
		int ICAL_METHOD_COUNTER = 10018;
		int ICAL_METHOD_DECLINECOUNTER = 10019;
		int ICAL_METHOD_CREATE = 10020;
		int ICAL_METHOD_READ = 10021;
		int ICAL_METHOD_RESPONSE = 10022;
		int ICAL_METHOD_MOVE = 10023;
		int ICAL_METHOD_MODIFY = 10024;
		int ICAL_METHOD_GENERATEUID = 10025;
		int ICAL_METHOD_DELETE = 10026;
		int ICAL_METHOD_NONE = 10027;
	}

	/** It's not typesafe, but it's simple to understand! */
	public interface ICalPropertyAction
	{
		// icalproperty_action
		int ICAL_ACTION_X = 10000;
		int ICAL_ACTION_AUDIO = 10001;
		int ICAL_ACTION_DISPLAY = 10002;
		int ICAL_ACTION_EMAIL = 10003;
		int ICAL_ACTION_PROCEDURE = 10004;
		int ICAL_ACTION_NONE = 10005;
	}

	/** It's not typesafe, but it's simple to understand! */
	public interface ICalPropertyKind
	{
		// icalproperty_kind
		int ICAL_ANY_PROPERTY = 0;
		int ICAL_ACTION_PROPERTY=1;
		int ICAL_ALLOWCONFLICT_PROPERTY=2;
		int ICAL_ATTACH_PROPERTY=3;
		int ICAL_ATTENDEE_PROPERTY=4;
		int ICAL_CALID_PROPERTY=5;
		int ICAL_CALMASTER_PROPERTY=6;
		int ICAL_CALSCALE_PROPERTY=7;
		int ICAL_CARID_PROPERTY=8;
		int ICAL_CATEGORIES_PROPERTY=9;
		int ICAL_CLASS_PROPERTY=10;
		int ICAL_COMMENT_PROPERTY=11;
		int ICAL_COMPLETED_PROPERTY=12;
		int ICAL_CONTACT_PROPERTY=13;
		int ICAL_CREATED_PROPERTY=14;
		int ICAL_DATEFORMAT_PROPERTY=15;
		int ICAL_DECREED_PROPERTY=16;
		int ICAL_DEFAULTCHARSET_PROPERTY=17;
		int ICAL_DEFAULTLOCALE_PROPERTY=18;
		int ICAL_DEFAULTTZID_PROPERTY=19;
		int ICAL_DESCRIPTION_PROPERTY=20;
		int ICAL_DTEND_PROPERTY=21;
		int ICAL_DTSTAMP_PROPERTY=22;
		int ICAL_DTSTART_PROPERTY=23;
		int ICAL_DUE_PROPERTY=24;
		int ICAL_DURATION_PROPERTY=25;
		int ICAL_EXDATE_PROPERTY=26;
		int ICAL_EXPAND_PROPERTY=27;
		int ICAL_EXRULE_PROPERTY=28;
		int ICAL_FREEBUSY_PROPERTY=29;
		int ICAL_GEO_PROPERTY=30;
		int ICAL_GRANT_PROPERTY=31;
		int ICAL_LASTMODIFIED_PROPERTY=32;
		int ICAL_LOCATION_PROPERTY=33;
		int ICAL_MAXRESULTS_PROPERTY=34;
		int ICAL_MAXRESULTSSIZE_PROPERTY=35;
		int ICAL_METHOD_PROPERTY=36;
		int ICAL_ORGANIZER_PROPERTY=37;
		int ICAL_OWNER_PROPERTY=38;
		int ICAL_PERCENTCOMPLETE_PROPERTY=39;
		int ICAL_PRIORITY_PROPERTY=40;
		int ICAL_PRODID_PROPERTY=41;
		int ICAL_QUERY_PROPERTY=42;
		int ICAL_QUERYNAME_PROPERTY=43;
		int ICAL_RDATE_PROPERTY=44;
		int ICAL_RECURRENCEID_PROPERTY=45;
		int ICAL_RELATEDTO_PROPERTY=46;
		int ICAL_RELCALID_PROPERTY=47;
		int ICAL_REPEAT_PROPERTY=48;
		int ICAL_REQUESTSTATUS_PROPERTY=49;
		int ICAL_RESOURCES_PROPERTY=50;
		int ICAL_RRULE_PROPERTY=51;
		int ICAL_SCOPE_PROPERTY=52;
		int ICAL_SEQUENCE_PROPERTY=53;
		int ICAL_STATUS_PROPERTY=54;
		int ICAL_SUMMARY_PROPERTY=55;
		int ICAL_TARGET_PROPERTY=56;
		int ICAL_TIMEFORMAT_PROPERTY=57;
		int ICAL_TRANSP_PROPERTY=58;
		int ICAL_TRIGGER_PROPERTY=59;
		int ICAL_TZID_PROPERTY=60;
		int ICAL_TZNAME_PROPERTY=61;
		int ICAL_TZOFFSETFROM_PROPERTY=62;
		int ICAL_TZOFFSETTO_PROPERTY=63;
		int ICAL_TZURL_PROPERTY=64;
		int ICAL_UID_PROPERTY=65;
		int ICAL_URL_PROPERTY=66;
		int ICAL_VERSION_PROPERTY=67;
		int ICAL_X_PROPERTY=68;
		int ICAL_XLICCLASS_PROPERTY=69;
		int ICAL_XLICCLUSTERCOUNT_PROPERTY=70;
		int ICAL_XLICERROR_PROPERTY=71;
		int ICAL_XLICMIMECHARSET_PROPERTY=72;
		int ICAL_XLICMIMECID_PROPERTY=73;
		int ICAL_XLICMIMECONTENTTYPE_PROPERTY=74;
		int ICAL_XLICMIMEENCODING_PROPERTY=75;
		int ICAL_XLICMIMEFILENAME_PROPERTY=76;
		int ICAL_XLICMIMEOPTINFO_PROPERTY=77;
		int ICAL_NO_PROPERTY=78;
	}

	public interface ICalPropertyStatus
	{
		// icalproperty_status
		int ICAL_STATUS_X = 10028;
		int ICAL_STATUS_TENTATIVE = 10029;
		int ICAL_STATUS_CONFIRMED = 10030;
		int ICAL_STATUS_COMPLETED = 10031;
		int ICAL_STATUS_NEEDSACTION = 10032;
		int ICAL_STATUS_CANCELLED = 10033;
		int ICAL_STATUS_INPROCESS = 10034;
		int ICAL_STATUS_DRAFT = 10035;
		int ICAL_STATUS_FINAL = 10036;
		int ICAL_STATUS_NONE = 10037;
	}

	/**
	 * Constructor for ICalProperty
	 * @param obj c++ pointer
	 */
	private ICalProperty(long obj)
	{
		init(obj);
	}

	public ICalProperty()
	{
		init();
	}

	public ICalProperty(String str)
	{
		init(str);
	}

	public ICalProperty(/* ICalPropertyKind */ int kind)
	{
		init(kind);
	}

	public native String as_ical_string();
	public native /* ICalPropertyKind */ int isa();
	public native boolean isa_property(Object property);

	public native void add_parameter(ICalParameter parameter);
	public native void set_parameter(ICalParameter parameter);
	public native void set_parameter_from_string(String name, String val);
	public native String get_parameter_as_string(String name);
	public native void remove_parameter(/* ICalParameterKind */ int kind);
	public native int count_parameters();

	/* Iterate through the parameters */
	public native ICalParameter get_first_parameter(/* ICalParameterKind */ int kind);
	public native ICalParameter get_next_parameter(/* ICalParameterKind */ int kind);

	/* Access the value of the property */
	public native void set_value(ICalValue val);
	public native void set_value_from_string(String val, String kind);

	public native ICalValue get_value();
	public native String get_value_as_string();

	/* Return the name of the property -- the type name converted to a
       String, or the value of get_x_name if the type is X property
	*/
	public native String get_name();

	/* Deal with X properties */
	//static void set_x_name(ICalProperty prop, String name);
	//static String get_x_name(ICalProperty prop);

	//public native static /* ICalValueKind */ int value_to_value_kind(/* ICalParameterValue */ int val);

	/* Convert kinds to String and get default value type */
	//public native static /* ICalValueKind */ int kind_to_value_kind(/* ICalPropertyKind */ int kind);
	//public native static /* ICalValueKind */ int value_kind_to_kind(/* ICalValueKind */ int kind);
	//public native static String kind_to_string(/* ICalPropertyKind */ int kind);
	//public native static /* ICalPropertyKind */ int string_to_kind(String str);

	//public native static /* ICalPropertyMethod */ int string_to_method(String str);
	//public native static String method_to_string(/* ICalPropertyMethod */ int method);

	//public native static String enum_to_string(int e);
	//public native static int string_to_enum(String str);

	//public native static String status_to_string(/* ICalPropertyStatus */ int status);
	//public native static /* ICalPropertyStatus */ int string_to_status(String str);

	//public native static int enum_belongs_to_property(/* ICalPropertyKind */ int kind, int e);

	/* ACTION */
	public native void set_action(/* ICalPropertyAction */ int v);
	public native /* ICalPropertyAction */ int get_action();

	/* ATTACH */
	//void set_attach(struct icalattachtype v);
	//struct icalattachtype get_attach();

	/* ATTENDEE */
	public native void set_attendee(String val);
	public native String get_attendee();

	/* CALSCALE */
	//public native void set_calscale(String val);
	//public native String get_calscale();

	/* CATEGORIES */
	//public native void set_categories(String val);
	//public native String get_categories();

	/* CLASS */
	//public native void set_class(String val);
	//public native String get_class();

	/* COMMENT */
	public native void set_comment(String val);
	public native String get_comment();

	/* COMPLETED */
	//void set_completed(ICalTimeType val);
	//ICalTimeType get_completed();

	/* CONTACT */
	//public native void set_contact(String val);
	//public native String get_contact();

	/* CREATED */
	//void set_created(ICalTimeType val);
	//ICalTimeType get_created();

	/* DESCRIPTION */
	public native void set_description(String val);
	public native String get_description();

	/* DTEND */
	public native void set_dtend(ICalTimeType val);
	public native ICalTimeType get_dtend();

	/* DTSTAMP */
	public native void set_dtstamp(ICalTimeType val);
	public native ICalTimeType get_dtstamp();

	/* DTSTART */
	public native void set_dtstart(ICalTimeType val);
	public native ICalTimeType get_dtstart();

	/* DUE */
	public native void set_due(ICalTimeType val);
	public native ICalTimeType get_due();

	/* DURATION */
	public native void set_duration(ICalDurationType val);
	public native ICalDurationType get_duration();

	/* EXDATE */
	public native void set_exdate(ICalTimeType val);
	public native ICalTimeType get_exdate();

	/* EXRULE */
	public native void set_exrule(ICalRecurrenceType val);
	public native ICalRecurrenceType get_exrule();

	/* EXPAND */
	public void set_expand(int val) {} // @-@:p0 TMA TODO
	public int get_expand() {return 0;} // @-@:p0 TMA TODO

	/* FREEBUSY */
	public native void set_freebusy(ICalPeriodType val);
	public native ICalPeriodType get_freebusy();

	/* GEO */
	//void set_geo(struct icalgeotype val);
	//struct icalgeotype get_geo();

	/* LAST-MODIFIED */
	//void set_lastmodified(ICalTimeType val);
	//ICalTimeType get_lastmodified();

	/* LOCATION */
	public native void set_location(String val);
	public native String get_location();

	/* MAXRESULTS */
	//public native void set_maxresults(int val);
	//public native int get_maxresults();

	/* MAXRESULTSSIZE */
	//public native void set_maxresultsize(int val);
	//public native int get_maxresultsize();

	/* METHOD */
	public native void set_method(/* ICalPropertyMethod */ int val);
	public native /* ICalPropertyMethod */ int get_method();

	/* ORGANIZER */
	public native void set_organizer(String val);
	public native String get_organizer();

	/* OWNER */
	public native void set_owner(String val);
	public native String get_owner();

	/* PERCENT-COMPLETE */
	//void set_percentcomplete(int val);
	//int get_percentcomplete();

	/* PRIORITY */
	//void set_priority(int val);
	//int get_priority();

	/* PRODID */
	public native void set_prodid(String val);
	public native String get_prodid();

	/* QUERY */
	public native void set_query(String val);
	public native String get_query();

	/* QUERYNAME */
	public native void set_queryname(String val);
	public native String get_queryname();

	/* RDATE */
	//void set_rdate(struct icaldatetimeperiodtype val);
	//struct icaldatetimeperiodtype get_rdate();

	/* RECURRENCE-ID */
	public native  void set_recurrenceid(ICalTimeType val);
	public native  ICalTimeType get_recurrenceid();

	/* RELATED-TO */
	//public native void set_relatedto(String val);
	//public native String get_relatedto();

        /* RELCALID */
        public native void set_relcalid(String val);
        public native String get_relcalid();

	/* REPEAT */
	public native void set_repeat(int val);
	public native int get_repeat();

	/* REQUEST-STATUS */
	//public native void set_requeststatus(String val);
	//public native String get_requeststatus();

	/* RESOURCES */
	//public native void set_resources(String val);
	//public native String get_resources();

	/* RRULE */
	public native void set_rrule(ICalRecurrenceType val);
	public native ICalRecurrenceType get_rrule();

	/* SCOPE */
	//public native void set_scope(String val);
	//public native String get_scope();

	/* SEQUENCE */
	//public native void set_sequence(int val);
	//public native int get_sequence();

	/* STATUS */
	public native void set_status(/* ICalPropertyStatus */ int val);
	public native /* ICalPropertyStatus */ int get_status();

	/* SUMMARY */
	public native void set_summary(String val);
	public native String get_summary();

	/* TARGET */
	public native void set_target(String val);
	public native String get_target();

	/* TRANSP */
	//public native void set_transp(String val);
	//public native String get_transp();

	/* TRIGGER */
	public native void set_trigger(ICalTriggerType val);
	public native ICalTriggerType get_trigger();

	/* TZID */
	public native void set_tzid(String val);
	public native String get_tzid();

	/* TZNAME */
	//public native void set_tzname(String val);
	//public native String get_tzname();

	/* TZOFFSETFROM */
	//public native void set_tzoffsetfrom(int val);
	//public native int get_tzoffsetfrom();


	/* TZOFFSETTO */
	//public native void set_tzoffsetto(int val);
	//public native int get_tzoffsetto();

	/* TZURL */
	//public native void set_tzurl(String val);
	//public native String get_tzurl();

	/* UID */
	public native void set_uid(String val);
	public native String get_uid();

	/* URL */
	//public native void set_url(String val);
	//public native String get_url();

	/* VERSION */
	//public native void set_version(String val);
	//public native String get_version();

	/* X */
	//void set_x(String val);
	//String get_x();

	/* X-LIC-CLUSTERCOUNT */
	//void set_xlicclustercount(String val);
	//String get_xlicclustercount();

	/* X-LIC-ERROR */
	//void set_xlicerror(String val);
	//String get_xlicerror();

	/* X-LIC-MIMECHARSET */
	//void set_xlicmimecharset(String val);
	//String get_xlicmimecharset();

	/* X-LIC-MIMECID */
	//void set_xlicmimecid(String val);
	//String get_xlicmimecid();

	/* X-LIC-MIMECONTENTTYPE */
	//void set_xlicmimecontenttype(String val);
	//String get_xlicmimecontenttype();

	/* X-LIC-MIMEENCODING */
	//void set_xlicmimeencoding(String val);
	//String get_xlicmimeencoding();

	/* X-LIC-MIMEFILENAME */
	//void set_xlicmimefilename(String val);
	//String get_xlicmimefilename();

	/* X-LIC-MIMEOPTINFO */
	//void set_xlicmimeoptinfo(String val);
	//String get_xlicmimeoptinfo();

	/**
	 * init the native class
	 */
	private void init(long obj)
	{
		m_Obj = obj;
	}

	private native void init();
	private native void init(String str);
	private native void init(/* ICalPropertyKind */ int kind);

	/**
	 * load the jni library for this class
	 */
	static {
		System.loadLibrary("ical_jni");
	}

	public static void main(String[] args)
	{
		System.out.println("*** ICalProperty main called ok.");
	}

	/** pointer to C++ object  */
	private long	m_Obj = 0;
}
