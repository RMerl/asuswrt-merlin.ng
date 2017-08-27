/*======================================================================
 FILE: VComponent.java
 CREATOR: gnorman 01/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

package net.cp.jlibical;

public class VComponent
{
	/** It's not typesafe, but it's simple to understand! */
	public interface ICalComponentKind
	{
		// icalcomponent_kind
		int ICAL_NO_COMPONENT = 0;
		int ICAL_ANY_COMPONENT = 1;
		int ICAL_XROOT_COMPONENT = 2;
		int ICAL_XATTACH_COMPONENT = 3;
		int ICAL_VEVENT_COMPONENT = 4;
		int ICAL_VTODO_COMPONENT = 5;
		int ICAL_VJOURNAL_COMPONENT = 6;
		int ICAL_VCALENDAR_COMPONENT = 7;
		int ICAL_VAGENDA_COMPONENT = 8;
		int ICAL_VFREEBUSY_COMPONENT = 9;
		int ICAL_VALARM_COMPONENT = 10;
		int ICAL_XAUDIOALARM_COMPONENT = 11;
		int ICAL_XDISPLAYALARM_COMPONENT = 12;
		int ICAL_XEMAILALARM_COMPONENT = 13;
		int ICAL_XPROCEDUREALARM_COMPONENT = 14;
		int ICAL_VTIMEZONE_COMPONENT = 15;
		int ICAL_XSTANDARD_COMPONENT = 16;
		int ICAL_XDAYLIGHT_COMPONENT = 17;
		int ICAL_X_COMPONENT = 18;
		int ICAL_VSCHEDULE_COMPONENT = 19;
		int ICAL_VQUERY_COMPONENT = 20;
		int ICAL_VCOMMAND_COMPONENT = 21;
		int ICAL_XLICINVALID_COMPONENT = 22;
		int ICAL_XLICMIMEPART_COMPONENT = 23;
		int ICAL_XPREFERENCES_COMPONENT = 24;
	}

	/**
	 * Constructor for VComponent
	 * @param obj c++ pointer
	 */
	protected VComponent(long obj)
	{
		init(obj);
	}

	public VComponent()
	{
		init();
	}

	public VComponent(/* ICalComponentKind */ int kind)
	{
		init(kind);
	}

	public VComponent(String str)
	{
		init(str);
	}

	public native String as_ical_string();
	//public native boolean is_valid();
	public native /* ICalComponentKind */ int isa();
	public native boolean isa_component(Object component);

	/* Working with properties */
	public native void add_property(ICalProperty property);
	public native void remove_property(ICalProperty property);
	public native int count_properties(/* ICalPropertyKind */ int kind);

	/* Iterate through the properties */
	public native ICalProperty get_current_property();
	public native ICalProperty get_first_property(/* ICalPropertyKind */ int kind);
	public native ICalProperty get_next_property(/* ICalPropertyKind */ int kind);

	/* Working with components */

	/* Return the first VEVENT, VTODO or VJOURNAL sub-component if it is one of those types */
	public native VComponent get_inner();

	public native void add_component(VComponent child);
	public native void remove_component(VComponent child);
	public native int count_components(/* ICalComponentKind */ int kind);

	/* Iteration Routines. There are two forms of iterators, internal and
	   external. The internal ones came first, and are almost completely
	   sufficient, but they fail badly when you want to construct a loop that
	   removes components from the container.
	*/

	/* Iterate through components */
	public native VComponent get_current_component();
	public native VComponent get_first_component(/* ICalComponentKind */ int kind);
	public native VComponent get_next_component(/* ICalComponentKind */ int kind);

	/* Using external iterators */
	//public native icalcompiter begin_component(/* ICalComponentKind */ int kind);
	//public native icalcompiter end_component(/* ICalComponentKind */ int kind);
	//public native VComponent next(icalcompiter i);
	//public native VComponent prev(icalcompiter i);
	//public native VComponent current(icalcompiter i);

	/* Working with embedded error properties */
	//public native int count_errors();

	/* Remove all X-LIC-ERROR properties*/
	//public native void strip_errors();

	/* Convert some X-LIC-ERROR properties into RETURN-STATUS properties*/
	//public native void convert_errors();

	/* Kind conversion routines */
	//public native static /* ICalComponentKind */ int string_to_kind(String str);
	//public native static String kind_to_string(/* ICalComponentKind */ int kind);

	public native ICalTimeType get_dtstart();
	public native void set_dtstart(ICalTimeType v);

	/* For the icalcomponent routines only, dtend and duration are tied
	   together. If you call the set routine for one and the other exists,
	   the routine will calculate the change to the other. That is, if
	   there is a DTEND and you call set_duration, the routine will modify
	   DTEND to be the sum of DTSTART and the duration. If you call a get
	   routine for one and the other exists, the routine will calculate
	   the return value. If you call a set routine and neither exists, the
	   routine will create the apcompriate comperty */

	public native ICalTimeType get_dtend();
	public native void set_dtend(ICalTimeType v);

	public native ICalDurationType get_duration();
	public native void set_duration(ICalDurationType v);

	public native /* ICalPropertyMethod */ int get_method();
	public native void set_method(/* ICalPropertyMethod */ int method);

	public native ICalTimeType get_dtstamp();
	public native void set_dtstamp(ICalTimeType v);

	public native String get_summary();
	public native void set_summary(String v);

	public native String get_location();
	public native void set_location(String v);

	public native String get_description();
	public native void set_description(String v);

	//public native String get_comment();
	//public native void set_comment(String v);

	public native String get_uid();
	public native void set_uid(String v);

	public native String get_relcalid();
	public native void set_relcalid(String v);

	public native ICalTimeType get_recurrenceid();
	public native void set_recurrenceid(ICalTimeType v);

	/* For VCOMPONENT: Return a reference to the first VEVENT, VTODO, or VJOURNAL */
	public native VComponent get_first_real_component();

	/* For VEVENT, VTODO, VJOURNAL and VTIMEZONE: report the start and end
	   times of an event in UTC */
	//public native virtual struct icaltime_span get_span();

	/**
	 * init the native class
	 */
	private void init(long obj)
	{
		m_Obj = obj;
	}

	private native void init();
	private native void init(String str);
	private native void init(/* ICalComponentKind */ int kind);

	/**
	 * load the jni library for this class
	 */
	static {
		System.loadLibrary("ical_jni");
	}

	public static void main(String[] args)
	{
		System.out.println("*** VComponent main called ok.");
	}

	/** pointer to C++ object  */
	private long	m_Obj = 0;
}
