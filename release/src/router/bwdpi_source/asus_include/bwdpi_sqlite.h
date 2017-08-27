
#include "sqlite3.h"

#define MON_SEC 86400 * 31
#define DAY_SEC 86400
#define HOURSEC 3600
#define QUERY_LEN 960 // BWSQL_LOG buffer size is 1024
#define LEN_MAX	8000

// traffic_analyzer.c
extern int traffic_analyzer_main(int argc, char **argv);

// sqlite_stat.c
extern int sql_get_table(sqlite3 *db, const char *sql, char ***pazResult, int *pnRow, int *pnColumn);
extern char *AiProtectionMontior_GetType(char *c);
extern void AiProtectionMonitor_result(int *tmp, char **result, int rows, int cols, int shift);
extern void sqlite_Stat_hook(int type, char *client, char *mode, char *dura, char *date, int *retval, webs_t wp);
extern void get_web_hook(char *client, char *page, char *num, int *retval, webs_t wp);
extern time_t Date_Of_Timestamp(time_t now);
extern void bwdpi_monitor_stat(int *retval, webs_t wp);
extern void bwdpi_monitor_info(char *type, char *event, int *retval, webs_t wp);
extern void bwdpi_monitor_ips(char *type, char *date, int *retval, webs_t wp);
extern void bwdpi_monitor_nonips(char *type, char *date, int *retval, webs_t wp);

// web_history.c
extern int web_history_main(int argc, char **argv);

// AiProctionMonitor.c
extern int aiprotection_monitor_main(int argc, char **argv);
