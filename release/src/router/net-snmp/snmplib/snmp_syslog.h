#ifndef _SNMP_SYSLOG_H_
#define _SNMP_SYSLOG_H_

/*
 * These definitions handle 4.2 systems without additional syslog facilities.
 */
#ifndef NETSNMP_FEATURE_REMOVE_LOGGING_SYSLOG
#ifndef LOG_CONS
#define LOG_CONS	0       /* Don't bother if not defined... */
#endif
#ifndef LOG_PID
#define LOG_PID		0       /* Don't bother if not defined... */
#endif
#ifndef LOG_LOCAL0
#define LOG_LOCAL0	0
#endif
#ifndef LOG_LOCAL1
#define LOG_LOCAL1	0
#endif
#ifndef LOG_LOCAL2
#define LOG_LOCAL2	0
#endif
#ifndef LOG_LOCAL3
#define LOG_LOCAL3	0
#endif
#ifndef LOG_LOCAL4
#define LOG_LOCAL4	0
#endif
#ifndef LOG_LOCAL5
#define LOG_LOCAL5	0
#endif
#ifndef LOG_LOCAL6
#define LOG_LOCAL6	0
#endif
#ifndef LOG_LOCAL7
#define LOG_LOCAL7	0
#endif
#ifndef LOG_DAEMON
#define LOG_DAEMON	0
#endif
#ifndef LOG_USER
#define LOG_USER	0
#endif
#endif /* NETSNMP_FEATURE_REMOVE_LOGGING_SYSLOG */

#endif /* _SNMP_SYSLOG_H_ */
