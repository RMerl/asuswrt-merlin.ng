rsRetVal sun_openklog(char *name);
void prepare_sys_poll(void);
void sun_sys_poll(void);
void sun_open_door(void);
void sun_delete_doorfiles(void);

extern struct pollfd sun_Pfd;		/* Pollfd for local log device */
