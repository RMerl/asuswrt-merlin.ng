/*======================================================================
 FILE: ICalTriggerType.java
 CREATOR: structConverter 01/11/02
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

package net.cp.jlibical;

/** struct icaltriggertype */
public class ICalTriggerType
{
	/**
	 * Constructor for pre-existing native icaltriggertype
	 * @param obj c++ pointer
	 */
	ICalTriggerType(long obj)
	{
		init(obj);
	}

	/**
	* Constructor for pre-existing native icaltriggertype
	* @param aTime c++ pointer
	* @param aDuration c++ pointer
	*/
	ICalTriggerType(long aTime, long aDuration)
	{
		init(aTime, aDuration);
	}

	/**
	* Constructor for default ICalTriggerType
	*/
	public ICalTriggerType()
	{
	}

	public void setTime(ICalTimeType lcl_arg0)
	{
		time = lcl_arg0;
	}
	public ICalTimeType getTime()
	{
		return time;
	}

	public void setDuration(ICalDurationType lcl_arg0)
	{
		duration = lcl_arg0;
	}
	public ICalDurationType getDuration()
	{
		return duration;
	}

	// --------------------------------------------------------
	// Initialization
	// --------------------------------------------------------

	/**
	* init with a native object
	*/
	private void init(long aTime, long aDuration)
	{
		time = new ICalTimeType(aTime);
		duration = new ICalDurationType(aDuration);
	}

	/**
	 * copy data from an existing struct.
	 */
	private native void init(long obj);

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
	private ICalTimeType		time = new ICalTimeType();
	private ICalDurationType	duration = new ICalDurationType();
}

