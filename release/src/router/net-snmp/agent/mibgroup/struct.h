#ifndef UCD_SNMP_STRUCT
#define UCD_SNMP_STRUCT

#define STRMAX 1024
#define SHPROC 1
#define EXECPROC 2
#define PASSTHRU 3
#define PASSTHRU_PERSIST 4
#define MIBMAX 30

struct extensible {
    char            name[STRMAX];
    char           *command;
    char            fixcmd[STRMAX];
    int             type;
    int             result;
    char            output[STRMAX];
    struct extensible *next;
    oid             miboid[MIBMAX];
    size_t          miblen;
    int             mibpriority;
    netsnmp_pid_t   pid;
#if defined(WIN32)
    HANDLE          tid;                /* WIN32 thread */
#endif
#ifdef USING_SINGLE_COMMON_PASSPERSIST_INSTANCE
    struct extensible *passpersist_inst;
#endif /* USING_SINGLE_COMMON_PASSPERSIST_INSTANCE */
};

struct myproc {
    char            name[STRMAX];
#if HAVE_PCRE_H
    netsnmp_regex_ptr regexp;
#endif
    char            fixcmd[STRMAX];
    int             min;
    int             max;
    struct myproc  *next;
};

/*
 * struct mibinfo 
 * {
 * int numid;
 * unsigned long mibid[10];
 * char *name;
 * void (*handle) ();
 * };
 */

#endif
