#ifndef _SELINUX_H_
#define _SELINUX_H_

#include <sys/types.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Return 1 if we are running on a SELinux kernel, or 0 if not or -1 if we get an error. */
extern int is_selinux_enabled(void);
/* Return 1 if we are running on a SELinux MLS kernel, or 0 otherwise. */
extern int is_selinux_mls_enabled(void);

typedef char *security_context_t;

/* Free the memory allocated for a context by any of the below get* calls. */
extern void freecon(security_context_t con);

/* Free the memory allocated for a context array by security_compute_user. */
extern void freeconary(security_context_t * con);

/* Wrappers for the /proc/pid/attr API. */

/* Get current context, and set *con to refer to it.
   Caller must free via freecon. */
extern int getcon(security_context_t * con);

/* Set the current security context to con.  
   Note that use of this function requires that the entire application
   be trusted to maintain any desired separation between the old and new 
   security contexts, unlike exec-based transitions performed via setexeccon.  
   When possible, decompose your application and use setexeccon()+execve() 
   instead. Note that the application may lose access to its open descriptors
   as a result of a setcon() unless policy allows it to use descriptors opened
   by the old context. */
extern int setcon(const security_context_t con);

/* Get context of process identified by pid, and 
   set *con to refer to it.  Caller must free via freecon. */
extern int getpidcon(pid_t pid, security_context_t * con);

/* Get previous context (prior to last exec), and set *con to refer to it.
   Caller must free via freecon. */
extern int getprevcon(security_context_t * con);

/* Get exec context, and set *con to refer to it.
   Sets *con to NULL if no exec context has been set, i.e. using default.
   If non-NULL, caller must free via freecon. */
extern int getexeccon(security_context_t * con);

/* Set exec security context for the next execve. 
   Call with NULL if you want to reset to the default. */
extern int setexeccon(const security_context_t con);

/* Get fscreate context, and set *con to refer to it.
   Sets *con to NULL if no fs create context has been set, i.e. using default.
   If non-NULL, caller must free via freecon. */
extern int getfscreatecon(security_context_t * con);

/* Set the fscreate security context for subsequent file creations.
   Call with NULL if you want to reset to the default. */
extern int setfscreatecon(const security_context_t context);

/* Get keycreate context, and set *con to refer to it.
   Sets *con to NULL if no key create context has been set, i.e. using default.
   If non-NULL, caller must free via freecon. */
extern int getkeycreatecon(security_context_t * con);

/* Set the keycreate security context for subsequent key creations.
   Call with NULL if you want to reset to the default. */
extern int setkeycreatecon(const security_context_t context);

/* Get sockcreate context, and set *con to refer to it.
   Sets *con to NULL if no socket create context has been set, i.e. using default.
   If non-NULL, caller must free via freecon. */
extern int getsockcreatecon(security_context_t * con);

/* Set the sockcreate security context for subsequent socket creations.
   Call with NULL if you want to reset to the default. */
extern int setsockcreatecon(const security_context_t context);

/* Wrappers for the xattr API. */

/* Get file context, and set *con to refer to it.
   Caller must free via freecon. */
extern int getfilecon(const char *path, security_context_t * con);
extern int lgetfilecon(const char *path, security_context_t * con);
extern int fgetfilecon(int fd, security_context_t * con);

/* Set file context */
extern int setfilecon(const char *path, security_context_t con);
extern int lsetfilecon(const char *path, security_context_t con);
extern int fsetfilecon(int fd, security_context_t con);

/* Wrappers for the socket API */

/* Get context of peer socket, and set *con to refer to it.
   Caller must free via freecon. */
extern int getpeercon(int fd, security_context_t * con);

/* Wrappers for the selinuxfs (policy) API. */

typedef unsigned int access_vector_t;
typedef unsigned short security_class_t;

struct av_decision {
	access_vector_t allowed;
	access_vector_t decided;
	access_vector_t auditallow;
	access_vector_t auditdeny;
	unsigned int seqno;
	unsigned int flags;
};

/* Definitions of av_decision.flags */
#define SELINUX_AVD_FLAGS_PERMISSIVE	0x0001

/* Structure for passing options, used by AVC and label subsystems */
struct selinux_opt {
	int type;
	const char *value;
};

/* Callback facilities */
union selinux_callback {
	/* log the printf-style format and arguments,
	   with the type code indicating the type of message */
	int 
#ifdef __GNUC__
__attribute__ ((format(printf, 2, 3)))
#endif
	(*func_log) (int type, const char *fmt, ...);
	/* store a string representation of auditdata (corresponding
	   to the given security class) into msgbuf. */
	int (*func_audit) (void *auditdata, security_class_t cls,
			   char *msgbuf, size_t msgbufsize);
	/* validate the supplied context, modifying if necessary */
	int (*func_validate) (security_context_t *ctx);
	/* netlink callback for setenforce message */
	int (*func_setenforce) (int enforcing);
	/* netlink callback for policyload message */
	int (*func_policyload) (int seqno);
};

#define SELINUX_CB_LOG		0
#define SELINUX_CB_AUDIT	1
#define SELINUX_CB_VALIDATE	2
#define SELINUX_CB_SETENFORCE	3
#define SELINUX_CB_POLICYLOAD	4

extern union selinux_callback selinux_get_callback(int type);
extern void selinux_set_callback(int type, union selinux_callback cb);

	/* Logging type codes, passed to the logging callback */
#define SELINUX_ERROR	        0
#define SELINUX_WARNING		1
#define SELINUX_INFO		2
#define SELINUX_AVC		3

/* Compute an access decision. */
extern int security_compute_av(const security_context_t scon,
			       const security_context_t tcon,
			       security_class_t tclass,
			       access_vector_t requested,
			       struct av_decision *avd);

/* Compute a labeling decision and set *newcon to refer to it.
   Caller must free via freecon. */
extern int security_compute_create(const security_context_t scon,
				   const security_context_t tcon,
				   security_class_t tclass,
				   security_context_t * newcon);

/* Compute a relabeling decision and set *newcon to refer to it.
   Caller must free via freecon. */
extern int security_compute_relabel(const security_context_t scon,
				    const security_context_t tcon,
				    security_class_t tclass,
				    security_context_t * newcon);

/* Compute a polyinstantiation member decision and set *newcon to refer to it.
   Caller must free via freecon. */
extern int security_compute_member(const security_context_t scon,
				   const security_context_t tcon,
				   security_class_t tclass,
				   security_context_t * newcon);

/* Compute the set of reachable user contexts and set *con to refer to 
   the NULL-terminated array of contexts.  Caller must free via freeconary. */
extern int security_compute_user(const security_context_t scon,
				 const char *username,
				 security_context_t ** con);

/* Load a policy configuration. */
extern int security_load_policy(void *data, size_t len);

/* Get the context of an initial kernel security identifier by name.  
   Caller must free via freecon */
extern int security_get_initial_context(const char *name,
					security_context_t * con);

/* Translate boolean strict to name value pair. */
typedef struct {
	char *name;
	int value;
} SELboolean;
/* save a list of booleans in a single transaction.  */
extern int security_set_boolean_list(size_t boolcnt,
				     SELboolean * boollist, int permanent);

/* Check the validity of a security context. */
extern int security_check_context(const security_context_t con);

/* Canonicalize a security context. */
extern int security_canonicalize_context(const security_context_t con,
					 security_context_t * canoncon);

/* Get the enforce flag value. */
extern int security_getenforce(void);

/* Set the enforce flag value. */
extern int security_setenforce(int value);

/* Get the behavior for undefined classes/permissions */
extern int security_deny_unknown(void);

/* Disable SELinux at runtime (must be done prior to initial policy load). */
extern int security_disable(void);

/* Get the policy version number. */
extern int security_policyvers(void);

/* Get the boolean names */
extern int security_get_boolean_names(char ***names, int *len);

/* Get the pending value for the boolean */
extern int security_get_boolean_pending(const char *name);

/* Get the active value for the boolean */
extern int security_get_boolean_active(const char *name);

/* Set the pending value for the boolean */
extern int security_set_boolean(const char *name, int value);

/* Commit the pending values for the booleans */
extern int security_commit_booleans(void);

/* Userspace class mapping support */
struct security_class_mapping {
	const char *name;
	const char *perms[sizeof(access_vector_t) * 8 + 1];
};

extern int selinux_set_mapping(struct security_class_mapping *map);

/* Common helpers */

/* Convert between security class values and string names */
extern security_class_t string_to_security_class(const char *name);
extern const char *security_class_to_string(security_class_t cls);

/* Convert between individual access vector permissions and string names */
extern const char *security_av_perm_to_string(security_class_t tclass,
					      access_vector_t perm);
extern access_vector_t string_to_av_perm(security_class_t tclass,
					 const char *name);

/* Returns an access vector in a string representation.  User must free the
 * returned string via free(). */
extern int security_av_string(security_class_t tclass,
			      access_vector_t av, char **result);

/* Check permissions and perform appropriate auditing. */
extern int selinux_check_access(const security_context_t scon,
				const security_context_t tcon,
				const char *tclass,
				const char *perm, void *aux);

/* Set the path to the selinuxfs mount point explicitly.
   Normally, this is determined automatically during libselinux 
   initialization, but this is not always possible, e.g. for /sbin/init
   which performs the initial mount of selinuxfs. */
void set_selinuxmnt(char *mnt);

#ifdef __cplusplus
}
#endif
#endif
