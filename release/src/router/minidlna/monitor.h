int monitor_insert_file(const char *name, const char *path);
int monitor_insert_directory(int fd, char *name, const char * path);
int monitor_remove_file(const char * path);
int monitor_remove_tree(const char * path);
int monitor_remove_directory(int fd, const char * path);

#if defined(HAVE_INOTIFY) || defined(HAVE_KQUEUE)
#define	HAVE_WATCH 1
int	monitor_add_watch(int, const char *);
int	monitor_remove_watch(int, const char *);
void 	monitor_start();
void 	monitor_stop();
#endif
