/*
 * tune2fs.h - Change the file system parameters on an ext2 file system
 *
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 */

#ifndef _TUNE2FS_H_
#define _TUNE2FS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Takes exactly the same args as the tune2fs executable.
 * Is the entry point for libtune2fs.
 */
int tune2fs_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif

#endif
