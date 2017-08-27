/*======================================================================
 FILE: VCalendar.java
 CREATOR: echoi 01/28/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

package net.cp.jlibical;

public class VCalendar extends VComponent {
	public VCalendar()
	{
		super(ICalComponentKind.ICAL_VCALENDAR_COMPONENT);
	}	

	public VCalendar(long obj)
	{
		super(obj);
	}

	public VCalendar(String str)
	{
		super(str);
	}
}
