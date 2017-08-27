package net.cp.jlibical;

public class VFreeBusy extends VComponent {
	public VFreeBusy()
	{
		super(ICalComponentKind.ICAL_VFREEBUSY_COMPONENT);
	}	

	public VFreeBusy(long obj)
	{
		super(obj);
	}

	public VFreeBusy(String str)
	{
		super(str);
	}
}
