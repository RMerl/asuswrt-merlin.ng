/*======================================================================
 FILE: VAlarm.java
 CREATOR: fnguyen 01/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

package net.cp.jlibical;

public class VAlarm extends VComponent {
	public VAlarm()
	{
		super(ICalComponentKind.ICAL_VALARM_COMPONENT);
	}
	
	public VAlarm(long obj)
	{
		super(obj);
	}

	public VAlarm(String str)
	{
		super(str);
	}
}
