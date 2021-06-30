#ifndef WINSERVICE_H
#define WINSERVICE_H

/* Windows Service related function declarations. */

#ifdef __cplusplus
extern          "C" {
#elif 0
}
#endif                          /*  */

/*
 * Define Constants for Register, De-register , Run As service or Console mode
 */
enum net_snmp_cmd_line_action {
    REGISTER_SERVICE = 0,
    UN_REGISTER_SERVICE = 1,
    RUN_AS_SERVICE = 2,
    RUN_AS_CONSOLE = 3,
};

/*
 * Input parameter structure to thread
 */
typedef struct _InputParams {
    DWORD           Argc;
    char          **Argv;
} InputParams;

/*
 * Define Service Related functions
 */

/*
 * To register application as windows service with SCM
 */
int             RegisterService(const char *lpszServiceName,
                                const char *lpszServiceDisplayName,
                                const char *lpszServiceDescription,
                                InputParams *StartUpArg, int quiet);

/*
 * To unregister service
 */
int             UnregisterService(const char *lpszServiceName, int quiet);

/*
 * To parse command line for startup option
 */
enum net_snmp_cmd_line_action
                ParseCmdLineForServiceOption(int argc, char *argv[],
                                             int *quiet);

/*
 * To start Service
 */
BOOL            RunAsService(int (*ServiceFunction)(int, char **));

/*
 * Service STOP function registration with this framewrok
 * * this function must be invoked before calling RunAsService
 */
void            RegisterStopFunction(void (*StopFunc)(void));

#if 0
{
#elif defined(__cplusplus)
}
#endif                          /*  */
#endif                          /* WINSERVICE_H */
