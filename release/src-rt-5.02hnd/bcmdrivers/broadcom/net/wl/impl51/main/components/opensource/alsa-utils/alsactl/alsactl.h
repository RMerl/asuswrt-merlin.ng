extern int debugflag;
extern int force_restore;
extern int ignore_nocards;
extern char *command;
extern char *statefile;

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define info(...) do {\
	fprintf(stdout, "%s: %s:%d: ", command, __FUNCTION__, __LINE__); \
	fprintf(stdout, __VA_ARGS__); \
	putc('\n', stdout); \
} while (0)
#else
#define info(args...) do {\
	fprintf(stdout, "%s: %s:%d: ", command, __FUNCTION__, __LINE__); \
	fprintf(stdout, ##args); \
	putc('\n', stdout); \
} while (0)
#endif	

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define error(...) do {\
	fprintf(stderr, "%s: %s:%d: ", command, __FUNCTION__, __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	putc('\n', stderr); \
} while (0)
#else
#define error(args...) do {\
	fprintf(stderr, "%s: %s:%d: ", command, __FUNCTION__, __LINE__); \
	fprintf(stderr, ##args); \
	putc('\n', stderr); \
} while (0)
#endif	

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define cerror(cond, ...) do {\
	if (cond || debugflag) { \
		fprintf(stderr, "%s%s: %s:%d: ", debugflag ? "WARNING: " : "", command, __FUNCTION__, __LINE__); \
		fprintf(stderr, __VA_ARGS__); \
		putc('\n', stderr); \
	} \
} while (0)
#else
#define cerror(cond, args...) do {\
	if (cond || debugflag) { \
		fprintf(stderr, "%s%s: %s:%d: ", debugflag ? "WARNING: " : "", command, __FUNCTION__, __LINE__); \
		fprintf(stderr, ##args); \
		putc('\n', stderr); \
	} \
} while (0)
#endif	

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define dbg(...) do {\
	if (!debugflag) break; \
	fprintf(stderr, "%s: %s:%d: ", command, __FUNCTION__, __LINE__); \
	fprintf(stderr, __VA_ARGS__); \
	putc('\n', stderr); \
} while (0)
#else
#define dbg(args...) do {\
	if (!debugflag) break; \
	fprintf(stderr, "%s: %s:%d: ", command, __FUNCTION__, __LINE__); \
	fprintf(stderr, ##args); \
	putc('\n', stderr); \
} while (0)
#endif	

int init(const char *file, const char *cardname);
int save_state(const char *file, const char *cardname);
int load_state(const char *file, const char *initfile, const char *cardname,
	       int do_init);
int power(const char *argv[], int argc);
int generate_names(const char *cfgfile);

/* utils */

int file_map(const char *filename, char **buf, size_t *bufsize);
void file_unmap(void *buf, size_t bufsize);
size_t line_width(const char *buf, size_t bufsize, size_t pos);
void initfailed(int cardnumber, const char *reason, int exitcode);

static inline int hextodigit(int c)
{
        if (c >= '0' && c <= '9')
                c -= '0';
        else if (c >= 'a' && c <= 'f')
                c = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')
                c = c - 'A' + 10;
        else
                return -1;
        return c;
}
