/*======================================================================
 FILE: ICalDurationType.java
 CREATOR: structConverter 01/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

package net.cp.jlibical;

/** struct icaldurationtype */
public class ICalDurationType
{
	/**
	 * Constructor for pre-existing native icaldurationtype.
	 * @param obj c++ pointer
	 */
	ICalDurationType(long obj)
	{
		init(obj);
	}

	/**
	 * Constructor for default ICalDurationType
	 */
	public ICalDurationType()
	{
	}

	public void setIs_neg(int lcl_arg0)
	{
	is_neg = lcl_arg0;
	}
	public int getIs_neg()
	{
	return is_neg;
	}

	public void setDays(long lcl_arg0)
	{
	days = lcl_arg0;
	}
	public long getDays()
	{
	return days;
	}

	public void setWeeks(long lcl_arg0)
	{
	weeks = lcl_arg0;
	}
	public long getWeeks()
	{
	return weeks;
	}

	public void setHours(long lcl_arg0)
	{
	hours = lcl_arg0;
	}
	public long getHours()
	{
	return hours;
	}

	public void setMinutes(long lcl_arg0)
	{
	minutes = lcl_arg0;
	}
	public long getMinutes()
	{
	return minutes;
	}

	public void setSeconds(long lcl_arg0)
	{
	seconds = lcl_arg0;
	}
	public long getSeconds()
	{
	return seconds;
	}

	// --------------------------------------------------------
	// Initialization
	// --------------------------------------------------------

	/**
	 * native code inits from an existing struct.
	 */
	private native void init(long aDuration);

	/**
	* optimization: init field id cache,
	*/
	private native static void initFIDs();

	/**
	* load the jni library for this class
	*/
	static {
		System.loadLibrary("ical_jni");
		initFIDs();
	}

	// --------------------------------------------------------
	// Fields
	// --------------------------------------------------------

	private int	is_neg;
	private long	days;	//  unsigned int
	private long	weeks;	//  unsigned int
	private long	hours;	//  unsigned int
	private long	minutes;	//  unsigned int
	private long	seconds;	//  unsigned int
}
