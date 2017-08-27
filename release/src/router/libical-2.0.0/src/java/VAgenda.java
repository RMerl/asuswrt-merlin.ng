/*======================================================================
 FILE: VAgenda.java
 CREATOR: fnguyen 01/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

package net.cp.jlibical;

public class VAgenda extends VComponent {
	public VAgenda()
	{
		super(ICalComponentKind.ICAL_VAGENDA_COMPONENT);
	}	

	public VAgenda(long obj)
	{
		super(obj);
	}

	public VAgenda(String str)
	{
		super(str);
	}
}
