/*======================================================================
 FILE: ICalRecurrenceType.java
 CREATOR: structConverter 01/11/02
======================================================================*/

package net.cp.jlibical;

public class ICalRecurrenceType
{
	public interface ICalRecurrenceTypeFrequency
	{
		int ICAL_SECONDLY_RECURRENCE=0;
		int ICAL_MINUTELY_RECURRENCE=1;
		int ICAL_HOURLY_RECURRENCE=2;
		int ICAL_DAILY_RECURRENCE=3;
		int ICAL_WEEKLY_RECURRENCE=4;
		int ICAL_MONTHLY_RECURRENCE=5;
		int ICAL_YEARLY_RECURRENCE=6;
    		int ICAL_NO_RECURRENCE=7;
	}

	public interface ICalRecurrenceTypeWeekday
	{
		int ICAL_NO_WEEKDAY=0;
		int ICAL_SUNDAY_WEEKDAY=1;
		int ICAL_MONDAY_WEEKDAY=2;
		int ICAL_TUESDAY_WEEKDAY=3;
		int ICAL_WEDNESDAY_WEEKDAY=4;
		int ICAL_THURSDAY_WEEKDAY=5;
		int ICAL_FRIDAY_WEEKDAY=6;
    		int ICAL_SATURDAY_WEEKDAY=7;
	}

	ICalRecurrenceType(long obj)
	{
		init(obj);
	}

	public ICalRecurrenceType()
	{
	}

	public void setUntil(ICalTimeType lcl_arg0)
	{
		until = lcl_arg0;
	}
	public ICalTimeType getUntil()
	{
		return until;
	}

	public void setFreq(int lcl_arg0)
	{
		freq = lcl_arg0;
	}
	public int getFreq()
	{
		return freq;
    }

	public void setWeek_start(int lcl_arg0)
	{
		week_start = lcl_arg0;
	}
	public int getWeek_start()
	{
		return week_start;
    }

    public void setCount(int lcl_arg0)
    {
		count = lcl_arg0;
    }
    public int getCount()
    {
		return count;
    }

    public void setInterval(short lcl_arg0)
    {
		interval = lcl_arg0;
    }
    public short getInterval()
    {
		return interval;
    }

    public void setBy_second(short[] lcl_arg0)
    {
		by_second = lcl_arg0;
    }

    public void setBy_secondIndexed(int ix,short lcl_arg0)
    {
		by_second[ix] = lcl_arg0;
    }
    public short[] getBy_second()
    {
		return by_second;
    }
    public short getBy_secondIndexed(int ix)
    {
		return by_second[ix];
    }

    public void setBy_minute(short[] lcl_arg0)
    {
		by_minute = lcl_arg0;
    }

    public void setBy_minuteIndexed(int ix,short lcl_arg0)
    {
		by_minute[ix] = lcl_arg0;
    }
    public short[] getBy_minute()
    {
		return by_minute;
    }
    public short getBy_minuteIndexed(int ix)
    {
		return by_minute[ix];
    }

    public void setBy_hour(short[] lcl_arg0)
    {
		by_hour = lcl_arg0;
    }

    public void setBy_hourIndexed(int ix,short lcl_arg0)
    {
		by_hour[ix] = lcl_arg0;
    }
    public short[] getBy_hour()
    {
		return by_hour;
    }
    public short getBy_hourIndexed(int ix)
    {
		return by_hour[ix];
    }

    public void setBy_day(short[] lcl_arg0)
    {
		by_day = lcl_arg0;
    }

    public void setBy_dayIndexed(int ix,short lcl_arg0)
    {
		by_day[ix] = lcl_arg0;
    }
    public short[] getBy_day()
    {
		return by_day;
    }
    public short getBy_dayIndexed(int ix)
    {
		return by_day[ix];
    }

    public void setBy_month_day(short[] lcl_arg0)
    {
		by_month_day = lcl_arg0;
    }

    public void setBy_month_dayIndexed(int ix,short lcl_arg0)
    {
		by_month_day[ix] = lcl_arg0;
    }
    public short[] getBy_month_day()
    {
		return by_month_day;
    }
    public short getBy_month_dayIndexed(int ix)
    {
		return by_month_day[ix];
    }

    public void setBy_year_day(short[] lcl_arg0)
    {
		by_year_day = lcl_arg0;
    }

    public void setBy_year_dayIndexed(int ix,short lcl_arg0)
    {
		by_year_day[ix] = lcl_arg0;
    }
    public short[] getBy_year_day()
    {
		return by_year_day;
    }
    public short getBy_year_dayIndexed(int ix)
    {
		return by_year_day[ix];
    }

    public void setBy_week_no(short[] lcl_arg0)
    {
		by_week_no = lcl_arg0;
    }

    public void setBy_week_noIndexed(int ix,short lcl_arg0)
    {
		by_week_no[ix] = lcl_arg0;
    }
    public short[] getBy_week_no()
    {
		return by_week_no;
    }
    public short getBy_week_noIndexed(int ix)
    {
		return by_week_no[ix];
    }

    public void setBy_month(short[] lcl_arg0)
    {
		by_month = lcl_arg0;
    }

    public void setBy_monthIndexed(int ix,short lcl_arg0)
    {
		by_month[ix] = lcl_arg0;
    }
    public short[] getBy_month()
    {
		return by_month;
    }
    public short getBy_monthIndexed(int ix)
    {
		return by_month[ix];
    }

    public void setBy_set_pos(short[] lcl_arg0)
    {
		by_set_pos = lcl_arg0;
    }

    public void setBy_set_posIndexed(int ix,short lcl_arg0)
    {
		by_set_pos[ix] = lcl_arg0;
    }
    public short[] getBy_set_pos()
    {
		return by_set_pos;
    }
    public short getBy_set_posIndexed(int ix)
    {
		return by_set_pos[ix];
    }

	private native void init(long obj);

    private native static void initFIDs();

    static {
		System.loadLibrary("ical_jni");
		initFIDs();
    }

	private /* ICalRecurrenceTypeFrequency */ int freq;
	private /* ICalRecurrenceTypeWeekday */ int week_start;
	private int	count;
	private short	interval;
	private short[]	by_second = new short[ICAL_BY_SECOND_SIZE];		//  Converted from short[61]
	private short[]	by_minute = new short[ICAL_BY_MINUTE_SIZE];		//  Converted from short[61]
	private short[]	by_hour = new short[ICAL_BY_HOUR_SIZE];		//  Converted from short[25]
	private short[] by_day = new short[ICAL_BY_DAY_SIZE];			//  Converted from short[364]
	private short[]	by_month_day = new short[ICAL_BY_MONTHDAY_SIZE];	//  Converted from short[32]
	private short[]	by_year_day = new short[ICAL_BY_YEARDAY_SIZE];	//  Converted from short[367]
	private short[]	by_week_no = new short[ICAL_BY_WEEKNO_SIZE];		//  Converted from short[54]
	private short[]	by_month = new short[ICAL_BY_MONTH_SIZE];		//  Converted from short[13]
	private short[]	by_set_pos = new short[ICAL_BY_SETPOS_SIZE];		//  Converted from short[367]
	private ICalTimeType		until = new ICalTimeType();

	public static final int ICAL_BY_SECOND_SIZE  	= 61;
	public static final int ICAL_BY_MINUTE_SIZE 	= 61;
	public static final int ICAL_BY_HOUR_SIZE 		= 25;
	public static final int ICAL_BY_DAY_SIZE 		= 364;
	public static final int ICAL_BY_MONTHDAY_SIZE 	= 32;
	public static final int ICAL_BY_YEARDAY_SIZE 	= 367;
	public static final int ICAL_BY_WEEKNO_SIZE 	= 54;
	public static final int ICAL_BY_MONTH_SIZE 		= 13;
	public static final int ICAL_BY_SETPOS_SIZE 	= 367;
}

