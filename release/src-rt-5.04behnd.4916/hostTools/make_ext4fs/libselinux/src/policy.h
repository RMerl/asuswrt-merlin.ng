#ifndef _POLICY_H_
#define _POLICY_H_

/* Private definitions used internally by libselinux. */

/* xattr name for SELinux attributes. */
#define XATTR_NAME_SELINUX "security.selinux"

/* Initial length guess for getting contexts. */
#define INITCONTEXTLEN 255

/* selinuxfs magic number */
#define SELINUX_MAGIC 0xf97cff8c

/* Preferred selinuxfs mount point directory paths. */
#define SELINUXMNT "/sys/fs/selinux"
#define OLDSELINUXMNT "/selinux"

/* selinuxfs filesystem type string. */
#define SELINUXFS "selinuxfs"

/* selinuxfs mount point determined at runtime */
extern char *selinux_mnt;

/* First version of policy supported in mainline Linux. */
#define DEFAULT_POLICY_VERSION 15

#endif
