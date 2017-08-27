/*======================================================================
 FILE: ICalPeriodType.java
 CREATOR: structConverter 01/11/02
======================================================================*/

package net.cp.jlibical;

public class ICalPeriodType
{
	public ICalPeriodType(long obj)
	{
		init(obj);
	}

	public ICalPeriodType()
	{
	}

	public ICalPeriodType(long aStart, long aEnd, long aDuration)
	{
		init(aStart, aEnd, aDuration);
	}

	public void setStart(ICalTimeType lcl_arg0)
	{
		start = lcl_arg0;
	}
	public ICalTimeType getStart()
	{
		return start;
	}

	public void setEnd(ICalTimeType lcl_arg0)
	{
		end = lcl_arg0;
	}
	public ICalTimeType getEnd()
	{
		return end;
	}

	public void setDuration(ICalDurationType lcl_arg0)
	{
		duration = lcl_arg0;
	}
	public ICalDurationType getDuration()
	{
		return duration;
	}

	private void init(long aStart, long aEnd, long aDuration)
	{
		start = new ICalTimeType(aStart);
		end = new ICalTimeType(aEnd);
		duration = new ICalDurationType(aDuration);
	}

	private native void init(long obj);

    private native static void initFIDs();

    static {
		System.loadLibrary("ical_jni");
		initFIDs();
    }

	private ICalTimeType		start = new ICalTimeType();
	private ICalTimeType		end = new ICalTimeType();
	private ICalDurationType	duration = new ICalDurationType();
}

