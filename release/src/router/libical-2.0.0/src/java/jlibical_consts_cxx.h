
/*======================================================================
 FILE: jlibical_consts_cxx/h
 CREATOR: Srinivasa Boppana/George Norman
 (C) COPYRIGHT 2002, Critical Path
======================================================================*/

#ifndef JLIBICAL_CONSTS_CXX_H
#define JLIBICAL_CONSTS_CXX_H

/*
 * Error code constants.
 */
#define JLIBICAL_OK                         0	/* good return code */

/* system/API errors */
#define JLIBICAL_ERR_NETWORK                100 /* general network error */
#define JLIBICAL_ERR_SERVER_INTERNAL        101 /* internal server error (database, file system, etc.) */
#define JLIBICAL_ERR_CLIENT_INTERNAL        102 /* internal error in client API (memory, parsing errors, etc.)*/
#define JLIBICAL_ERR_ILLEGAL_ARGUMENT       103 /* incorrect API use */
#define JLIBICAL_ERR_API_NOT_INITED         104 /* either the InitModule API call was called prior to use of the API or it failed to initialize correctly */
#define JLIBICAL_ERR_HOST_INVALID           105 /* the host name specified cannot be resolved */

/* Java classes */
#define JLIBICAL_CLASS_ICALVALUE                "net/cp/jlibical/ICalValue"
#define JLIBICAL_CLASS_VCOMPONENT               "net/cp/jlibical/VComponent"
#define JLIBICAL_CLASS_VALARM			"net/cp/jlibical/VAlarm"
#define JLIBICAL_CLASS_VCALENDAR		"net/cp/jlibical/VCalendar"
#define JLIBICAL_CLASS_VAGENDA                  "net/cp/jlibical/VAgenda"
#define JLIBICAL_CLASS_VEVENT			"net/cp/jlibical/VEvent"
#define JLIBICAL_CLASS_VQUERY			"net/cp/jlibical/VQuery"
#define JLIBICAL_CLASS_VTODO			"net/cp/jlibical/VToDo"
#define JLIBICAL_CLASS_ICALPARAMETER            "net/cp/jlibical/ICalParameter"
#define JLIBICAL_CLASS_ICALPROPERTY             "net/cp/jlibical/ICalProperty"

#define JLIBICAL_CLASS_ICALDURATIONTYPE         "net/cp/jlibical/ICalDurationType"
#define JLIBICAL_CLASS_ICALTIMETYPE             "net/cp/jlibical/ICalTimeType"
#define JLIBICAL_CLASS_ICALTRIGGERTYPE          "net/cp/jlibical/ICalTriggerType"
#define JLIBICAL_CLASS_ICALRECURRENCETYPE       "net/cp/jlibical/ICalRecurrenceType"
#define JLIBICAL_CLASS_ICALPERIODTYPE      		"net/cp/jlibical/ICalPeriodType"

#endif /* JLIBICAL_CONSTS_CXX_H */
