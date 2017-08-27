/*======================================================================
 FILE: VEvent.java
 CREATOR: fnguyen 01/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

package net.cp.jlibical;

public class VEvent extends VComponent {
	public VEvent()
	{
		super(ICalComponentKind.ICAL_VEVENT_COMPONENT);
	}	

	public VEvent(long obj)
	{
		super(obj);
	}

	public VEvent(String str)
	{
		super(str);
	}
}
