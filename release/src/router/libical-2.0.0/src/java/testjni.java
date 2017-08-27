package net.cp.jlibical;

import java.lang.String;

public class testjni
{
	static final String content = "BEGIN:VCALENDAR\nVERSION:2.1\nBEGIN:VEVENT\nUID:abcd12345\nDTSTART:20020307T180000Z\nDTEND:20020307T190000Z\nSUMMARY:Important Meeting\nEND:VEVENT\nEND:VCALENDAR";
	public static void main(String[] args)
	{
                // VAGENDA test case

                ICalProperty calidProp = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_RELCALID_PROPERTY);
                ICalProperty ownerProp = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_OWNER_PROPERTY);

                calidProp.set_relcalid("1212");
                ownerProp.set_owner("Bill Smith");

                VAgenda vAgenda = new VAgenda();
                vAgenda.add_property(calidProp);
                vAgenda.add_property(ownerProp);

                System.out.println("VAgenda:\n" + vAgenda.as_ical_string());

                // VEVENT test case

		ICalProperty summProp  = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_SUMMARY_PROPERTY);
		ICalProperty startProp = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_DTSTART_PROPERTY);
		ICalProperty endProp   = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_DTEND_PROPERTY);

        	ICalProperty locationProp = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_LOCATION_PROPERTY);
        	ICalProperty descProp = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_DESCRIPTION_PROPERTY);

		ICalTimeType starttime = new ICalTimeType();
		starttime.setYear(2001);
		starttime.setMonth(12);
		starttime.setDay(21);
		starttime.setHour(18);
		starttime.setMinute(0);
		starttime.setSecond(0);
		starttime.setIs_utc(true);
		System.out.println("done creating starttime");

		ICalTimeType endtime = new ICalTimeType();
		endtime.setYear(2002);
		endtime.setMonth(1);
		endtime.setDay(1);
		endtime.setHour(8);
		endtime.setMinute(0);
		endtime.setSecond(0);
		endtime.setIs_utc(true);
		System.out.println("done creating endtime");


		// START - get, set_exdate
		ICalProperty property_tma = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_EXDATE_PROPERTY);
		property_tma.set_exdate(starttime);
		ICalTimeType starttime_return = property_tma.get_exdate();
		System.out.println("****** property - exdate ******");
		System.out.println("setYear=" + starttime_return.getYear());
		System.out.println("setMonth=" + starttime_return.getMonth());
		System.out.println("setDay=" + starttime_return.getDay());
		System.out.println("setHour=" + starttime_return.getHour());
		System.out.println("setMinute=" + starttime_return.getMinute());
		System.out.println("setSecond=" + starttime_return.getSecond());
		System.out.println("******* property - exdate *****");

		// END - get, set exdate

		// START - get, set exrule
		ICalProperty property_exrule_tma = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_EXRULE_PROPERTY);
		ICalRecurrenceType exrule_tma = new ICalRecurrenceType();
		exrule_tma.setUntil(starttime);
		exrule_tma.setFreq(ICalRecurrenceType.ICalRecurrenceTypeFrequency.ICAL_MINUTELY_RECURRENCE);
		exrule_tma.setWeek_start(ICalRecurrenceType.ICalRecurrenceTypeWeekday.ICAL_SUNDAY_WEEKDAY);
		short test_tma[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61};

		exrule_tma.setBy_second(test_tma);
		exrule_tma.setBy_minute(test_tma);
		property_exrule_tma.set_exrule(exrule_tma);

		ICalRecurrenceType recurence_tma_return = property_exrule_tma.get_exrule();
		System.out.println("****** property - exrule ******");
		System.out.println("setFreq=" + recurence_tma_return.getFreq());
		System.out.println("setWeek_start=" + recurence_tma_return.getWeek_start());
		System.out.println("setBy_second[30]=" + recurence_tma_return.getBy_secondIndexed(30));
		System.out.println("setBy_minute[28]=" + recurence_tma_return.getBy_minuteIndexed(28));
		System.out.println("****** property - exrule ******");

		// END - get, set exrule

		// START - get, set recurrenceid
		ICalProperty property_recurrenceid_tma = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_RECURRENCEID_PROPERTY);
		property_recurrenceid_tma.set_recurrenceid(starttime);
		ICalTimeType recurrenceid_return = property_recurrenceid_tma.get_recurrenceid();
		System.out.println("****** property - recurrenceid ******");
		System.out.println("setYear=" + recurrenceid_return.getYear());
		System.out.println("setMonth=" + recurrenceid_return.getMonth());
		System.out.println("setDay=" + recurrenceid_return.getDay());
		System.out.println("setHour=" + recurrenceid_return.getHour());
		System.out.println("setMinute=" + recurrenceid_return.getMinute());
		System.out.println("setSecond=" + recurrenceid_return.getSecond());
		System.out.println("******* property - recurrenceid *****");

		// END - get, set recurrenceid

		// START - get, set rrule
		ICalProperty property_rrule_tma = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_RRULE_PROPERTY);
		ICalRecurrenceType rrule_tma = new ICalRecurrenceType();
		rrule_tma.setUntil(starttime);
		rrule_tma.setFreq(ICalRecurrenceType.ICalRecurrenceTypeFrequency.ICAL_MINUTELY_RECURRENCE);
		rrule_tma.setWeek_start(ICalRecurrenceType.ICalRecurrenceTypeWeekday.ICAL_SUNDAY_WEEKDAY);
		short test_hour_tma[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25};
		rrule_tma.setBy_hour(test_hour_tma);
		property_rrule_tma.set_rrule(rrule_tma);

		ICalRecurrenceType rrule_tma_return = property_rrule_tma.get_rrule();
		System.out.println("****** property - rrule ******");
		System.out.println("setFreq=" + rrule_tma_return.getFreq());
		System.out.println("setWeek_start=" + rrule_tma_return.getWeek_start());
		System.out.println("setBy_hour[11]=" + rrule_tma_return.getBy_hourIndexed(11));
		System.out.println("****** property - rrule ******");
		// END - get, set rrule

		// START - get, set freebusy
		ICalProperty property_freebusy_tma = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_FREEBUSY_PROPERTY);
		ICalPeriodType period_tma = new ICalPeriodType();
		ICalDurationType duration_tma = new ICalDurationType();
		duration_tma.setDays((long) 0);
		duration_tma.setHours((long) 10);
		duration_tma.setMinutes((long) 15);

		period_tma.setStart(starttime);
		period_tma.setEnd(endtime);
		period_tma.setDuration(duration_tma);

		property_freebusy_tma.set_freebusy(period_tma);

		ICalPeriodType period_tma_return = property_freebusy_tma.get_freebusy();
		ICalTimeType start_tma_return = period_tma_return.getStart();
		ICalTimeType end_tma_return = period_tma_return.getEnd();
		ICalDurationType duration_tma_return = period_tma_return.getDuration();

		System.out.println("****** property - freebusy - start ******");
		System.out.println("setYear=" + start_tma_return.getYear());
		System.out.println("setMonth=" + start_tma_return.getMonth());
		System.out.println("setDay=" + start_tma_return.getDay());
		System.out.println("setHour=" + start_tma_return.getHour());
		System.out.println("setMinute=" + start_tma_return.getMinute());
		System.out.println("setSecond=" + start_tma_return.getSecond());
		System.out.println("******* property - freebusy - start *****");

		System.out.println("****** property - freebusy - end ******");
		System.out.println("setYear=" + end_tma_return.getYear());
		System.out.println("setMonth=" + end_tma_return.getMonth());
		System.out.println("setDay=" + end_tma_return.getDay());
		System.out.println("setHour=" + end_tma_return.getHour());
		System.out.println("setMinute=" + end_tma_return.getMinute());
		System.out.println("setSecond=" + end_tma_return.getSecond());
		System.out.println("******* property - freebusy - end *****");

		System.out.println("****** property - freebusy - duration ******");
		System.out.println("setYear=" + duration_tma_return.getDays());
		System.out.println("setMonth=" + duration_tma_return.getHours());
		System.out.println("setDay=" + duration_tma_return.getMinutes());
		System.out.println("******* property - freebusy - duration *****");

		// END - get, set freebusy

		// START - get, set dtstamp
		ICalProperty property_dtstamp_tma = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_DTSTAMP_PROPERTY);
		property_dtstamp_tma.set_dtstamp(starttime);
		ICalTimeType dtstamp_tma_return = property_dtstamp_tma.get_dtstamp();
		System.out.println("****** property - dtstamp ******");
		System.out.println("setYear=" + dtstamp_tma_return.getYear());
		System.out.println("setMonth=" + dtstamp_tma_return.getMonth());
		System.out.println("setDay=" + dtstamp_tma_return.getDay());
		System.out.println("setHour=" + dtstamp_tma_return.getHour());
		System.out.println("setMinute=" + dtstamp_tma_return.getMinute());
		System.out.println("setSecond=" + dtstamp_tma_return.getSecond());
		System.out.println("******* property - dtstamp *****");
		// END - get, set dtstamp
		// START - get, set attendee
		ICalProperty property_attendee_tma = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_ATTENDEE_PROPERTY);
		property_attendee_tma.set_attendee("John");
		String attendee_tma_return = property_attendee_tma.get_attendee();
		System.out.println("****** property - attendee ******");
		System.out.println("setAttendee=" + attendee_tma_return);
		System.out.println("****** property - attendee ******");
		// END - get, set attendee
		// START - get, set comment
		ICalProperty property_comment_tma = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_COMMENT_PROPERTY);
		property_comment_tma.set_comment("John");
		String comment_tma_return = property_comment_tma.get_comment();
		System.out.println("****** property - comment ******");
		System.out.println("setComment=" + comment_tma_return);
		System.out.println("****** property - comment ******");
		// END - get, set comment
		// START - get, set organizer
		ICalProperty property_organizer_tma = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_ORGANIZER_PROPERTY);
		property_organizer_tma.set_organizer("John");
		String organizer_tma_return = property_organizer_tma.get_organizer();
		System.out.println("****** property - organizer ******");
		System.out.println("setOrganizer=" + organizer_tma_return);
		System.out.println("****** property - organizer ******");
		// END - get, set organizer

		summProp.set_summary("New Year's Eve Party, and more");
		System.out.println("done setting summProp");
		startProp.set_dtstart(starttime);
		System.out.println("done setting startProp");
		endProp.set_dtend(endtime);
		System.out.println("done setting endProp");
		locationProp.set_location("Bothin, Marin County, CA, USA");
		System.out.println("done setting locationProp\n");
                descProp.set_description("Drive carefully; come to have fun!");
                System.out.println("done setting descProp\n");

		VEvent vEvent = new VEvent();
		vEvent.add_property(summProp);
		vEvent.add_property(startProp);
		vEvent.add_property(endProp);
		vEvent.add_property(locationProp);
                vEvent.add_property(descProp);

                ICalTimeType sTime = vEvent.get_dtstart();
                ICalTimeType eTime = vEvent.get_dtend();

		//System.out.println("VEvent:\n" + vEvent.as_ical_string());
                System.out.println("Summary: \n" + vEvent.get_summary());
                System.out.println("DTSTART: \n" + sTime.getYear() + sTime.getMonth() + sTime.getDay() +
                                                   sTime.getHour() + sTime.getMinute() + sTime.getSecond());
                System.out.println("DTEND: \n"   + eTime.getYear() + eTime.getMonth() + eTime.getDay() +
                                                   eTime.getHour() + eTime.getMinute() + eTime.getSecond());
                System.out.println("Location: \n" + vEvent.get_location());
                System.out.println("Description: \n" +vEvent.get_description());

		VComponent ic = new VComponent(content);
		if (ic == null)
			System.err.println("Failed to parse component");
		System.out.println("VComponent:\n" + ic.as_ical_string());

		// component is wrapped within BEGIN:VCALENDAR END:VCALENDAR
		// we need to unwrap it.
		VEvent sub_ic = (VEvent)ic.get_first_component(VComponent.ICalComponentKind.ICAL_VEVENT_COMPONENT);
		if (sub_ic == null)
			System.out.println("Failed to get subcomponent");
		while (sub_ic != null)
		{
			System.out.println("subcomponent:\n" + sub_ic.as_ical_string());
			sub_ic = (VEvent)ic.get_next_component(VComponent.ICalComponentKind.ICAL_VEVENT_COMPONENT);
		}

		// START - get, set recurrenceid
		ICalTimeType time_tma = new ICalTimeType();
		time_tma.setYear(2002);
		time_tma.setMonth(2);
		time_tma.setDay(2);
		time_tma.setHour(2);
		time_tma.setMinute(2);
		time_tma.setSecond(2);
		time_tma.setIs_utc(true);

		ic.set_recurrenceid(time_tma);

		ICalTimeType time_tma_return = new ICalTimeType();
		time_tma_return = ic.get_recurrenceid();
		System.out.println("****** vcomponent - recurrenceid ******");
		System.out.println("setYear=" + time_tma_return.getYear());
		System.out.println("setMonth=" + time_tma_return.getMonth());
		System.out.println("setDay=" + time_tma_return.getDay());
		System.out.println("setHour=" + time_tma_return.getHour());
		System.out.println("setMinute=" + time_tma_return.getMinute());
		System.out.println("setSecond=" + time_tma_return.getSecond());
		System.out.println("****** vcomponent - recurrenceid ******");
		// END - get, set recurrenceid
		// START - get, set dtstamp
		ic.set_dtstamp(time_tma);
		ICalTimeType vcomponent_dtstamp_tma_return = ic.get_dtstamp();
		System.out.println("****** vcomponent - dtstamp ******");
		System.out.println("setYear=" + vcomponent_dtstamp_tma_return.getYear());
		System.out.println("setMonth=" + vcomponent_dtstamp_tma_return.getMonth());
		System.out.println("setDay=" + vcomponent_dtstamp_tma_return.getDay());
		System.out.println("setHour=" + vcomponent_dtstamp_tma_return.getHour());
		System.out.println("setMinute=" + vcomponent_dtstamp_tma_return.getMinute());
		System.out.println("setSecond=" + vcomponent_dtstamp_tma_return.getSecond());
		System.out.println("****** vcomponent - dtstamp ******");

		// END - get, set dtstamp

           	// VTODO test cases

		ICalProperty statusProp = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_STATUS_PROPERTY);
                ICalProperty dueProp = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_DUE_PROPERTY);

                ICalTimeType duetime = new ICalTimeType();
                duetime.setYear(2002);
                duetime.setMonth(12);
                duetime.setDay(21);
                duetime.setHour(18);
                duetime.setMinute(0);
                duetime.setSecond(0);
                duetime.setIs_utc(true);
                System.out.println("done creating duetime");

                statusProp.set_status(ICalProperty.ICalPropertyStatus.ICAL_STATUS_COMPLETED);
                dueProp.set_due(duetime);

		VToDo vtodo = new VToDo();

		vtodo.add_property(statusProp);
		vtodo.add_property(dueProp);

		System.out.println("VToDo:\n" + vtodo.as_ical_string());

                // VALARM test cases

                VAlarm valarm = new VAlarm();
                System.out.println("created valarm");

                // 1. Create a ICAL_DURATION_PROPERTY using the TimePeriod.
                ICalDurationType duration = new ICalDurationType();
                duration.setDays((long) 0);
                duration.setHours((long) 0);
                duration.setMinutes((long) 15);
                System.out.println("created duration object");

                // Since we want to trigger before the event or task,
                // we always want to set this to 1. If we say this is not
                // a negative duration, that means the alarm will trigger
                // AFTER the event or task start or due date, which is useless.
                duration.setIs_neg(1);

                // 2. Create a ICalTriggerType oject and set the duration on it.
                ICalTriggerType trigger = new ICalTriggerType();
                trigger.setDuration(duration);
                System.out.println("set trigger to duration object");

                // 3. Create a trigger ICalProperty and set the trigger on it.
                ICalProperty triggerProp = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_TRIGGER_PROPERTY);
                System.out.println("created trigger property");
                triggerProp.set_trigger(trigger);
                System.out.println("set trigger property");
		trigger = triggerProp.get_trigger();
                System.out.println("get trigger property");

                // 4. set the trigger property on the Valarm object.
                valarm.add_property(triggerProp);
                System.out.println("VAlarm:\n" + valarm.as_ical_string());

		// 5. create attendee property
		String userEmail = "userid@domain";
		ICalProperty attendeeProp = new ICalProperty(ICalProperty.ICalPropertyKind.ICAL_ATTENDEE_PROPERTY);
		attendeeProp.set_attendee("MAILTO:" + userEmail);
                System.out.println("set attendee property");
		userEmail = attendeeProp.get_attendee();
                System.out.println("get attendee property");
		valarm.add_property(attendeeProp);
                System.out.println("VAlarm:\n" + valarm.as_ical_string());

		// START - get, set_role
		ICalParameter parameter_role_tma = new ICalParameter(ICalParameter.ICalParameterKind.ICAL_ROLE_PARAMETER);
		parameter_role_tma.set_role(ICalParameter.ICalParameterRole.ICAL_ROLE_REQPARTICIPANT);
		int role_tma_return = parameter_role_tma.get_role();
		System.out.println("******* parameter - role *****");
		System.out.println("setRole=" + role_tma_return);
		System.out.println("******* parameter - role *****");
		// END - get, set_role
		// START - get, set_partstat
		ICalParameter parameter_partstat_tma = new ICalParameter(ICalParameter.ICalParameterKind.ICAL_PARTSTAT_PARAMETER);
		parameter_partstat_tma.set_partstat(ICalParameter.ICalParameterPartStat.ICAL_PARTSTAT_DECLINED);
		int partstat_tma_return = parameter_partstat_tma.get_partstat();
		System.out.println("******* parameter - partstat *****");
		System.out.println("setPartstat=" + partstat_tma_return);
		System.out.println("******* parameter - partstat *****");
		// END - get, set_partstat
	}
}
