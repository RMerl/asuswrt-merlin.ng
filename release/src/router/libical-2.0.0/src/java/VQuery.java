/*======================================================================
 FILE: VQuery.java
 CREATOR: fnguyen 01/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

package net.cp.jlibical;

public class VQuery extends VComponent {
	public VQuery()
	{
		super(ICalComponentKind.ICAL_VQUERY_COMPONENT);
	}	

	public VQuery(long obj)
	{
		super(obj);
	}

	public VQuery(String str)
	{
		super(str);
	}
}
