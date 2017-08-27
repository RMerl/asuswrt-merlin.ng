/*======================================================================
 FILE: VToDo.java
 CREATOR: fnguyen 01/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

package net.cp.jlibical;

public class VToDo extends VComponent {
	public VToDo()
	{
		super(ICalComponentKind.ICAL_VTODO_COMPONENT);
	}	

	public VToDo(long obj)
	{
		super(obj);
	}

	public VToDo(String str)
	{
		super(str);
	}
}
