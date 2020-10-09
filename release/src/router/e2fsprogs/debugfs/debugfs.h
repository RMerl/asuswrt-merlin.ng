/*
 * debugfs.h --- header file for the debugfs program
 */

#include "ss/ss.h"
#include "ext2fs/ext2_fs.h"
#include "ext2fs/ext2fs.h"
#include "../misc/create_inode.h"
#include "support/quotaio.h"

#ifdef __STDC__
#define NOARGS void
#else
#define NOARGS
#define const
#endif

/*
 * Flags used by the common argument processing functions
 */
#define CHECK_FS_RW		0x0001
#define CHECK_FS_BITMAPS	0x0002
#define CHECK_FS_NOTOPEN	0x0004

extern ext2_filsys current_fs;
extern quota_ctx_t current_qctx;
extern ext2_ino_t	root, cwd;
extern int ss_sci_idx;
extern ss_request_table debug_cmds, extent_cmds;

extern void reset_getopt(void);
extern FILE *open_pager(void);
extern void close_pager(FILE *stream);
extern int check_fs_open(char *name);
extern int check_fs_not_open(char *name);
extern int check_fs_read_write(char *name);
extern int check_fs_bitmaps(char *name);
extern ext2_ino_t string_to_inode(char *str);
extern char *inode_time_to_string(__u32 xtime, __u32 xtime_extra);
extern char *time_to_string(__s64);
extern __s64 string_to_time(const char *);
extern unsigned long parse_ulong(const char *str, const char *cmd,
				 const char *descr, int *err);
extern unsigned long long parse_ulonglong(const char *str, const char *cmd,
					  const char *descr, int *err);
extern int strtoblk(const char *cmd, const char *str, const char *errmsg,
		    blk64_t *ret);
extern int common_args_process(int argc, char *argv[], int min_argc,
			       int max_argc, const char *cmd,
			       const char *usage, int flags);
extern int common_inode_args_process(int argc, char *argv[],
				     ext2_ino_t *inode, int flags);
extern int common_block_args_process(int argc, char *argv[],
				     blk64_t *block, blk64_t *count);
extern int debugfs_read_inode(ext2_ino_t ino, struct ext2_inode * inode,
			      const char *cmd);
extern int debugfs_read_inode2(ext2_ino_t ino, struct ext2_inode * inode,
			       const char *cmd, int bufsize, int flags);
extern int debugfs_write_inode(ext2_ino_t ino, struct ext2_inode * inode,
			       const char *cmd);
extern int debugfs_write_inode2(ext2_ino_t ino, struct ext2_inode * inode,
				const char *cmd, int bufsize, int flags);
extern int debugfs_write_new_inode(ext2_ino_t ino, struct ext2_inode * inode,
				   const char *cmd);
extern int ext2_file_type(unsigned int mode);

/* ss command functions */

/* dump.c */
extern void do_dump(int argc, char **argv, int sci_idx, void *infop);
extern void do_cat(int argc, char **argv, int sci_idx, void *infop);
extern void do_rdump(int argc, char **argv, int sci_idx, void *infop);

/* extent_inode.c */
extern void do_extent_open(int argc, char **argv, int sci_idx, void *infop);
extern void do_extent_close(int argc, char **argv, int sci_idx, void *infop);
extern void do_current_node(int argc, char **argv, int sci_idx, void *infop);
extern void do_root_node(int argc, char **argv, int sci_idx, void *infop);
extern void do_last_leaf(int argc, char **argv, int sci_idx, void *infop);
extern void do_first_sib(int argc, char **argv, int sci_idx, void *infop);
extern void do_last_sib(int argc, char **argv, int sci_idx, void *infop);
extern void do_next_sib(int argc, char **argv, int sci_idx, void *infop);
extern void do_prev_sib(int argc, char **argv, int sci_idx, void *infop);
extern void do_next_leaf(int argc, char **argv, int sci_idx, void *infop);
extern void do_prev_leaf(int argc, char **argv, int sci_idx, void *infop);
extern void do_next(int argc, char **argv, int sci_idx, void *infop);
extern void do_prev(int argc, char **argv, int sci_idx, void *infop);
extern void do_up(int argc, char **argv, int sci_idx, void *infop);
extern void do_down(int argc, char **argv, int sci_idx, void *infop);
extern void do_delete_node(int argc, char **argv, int sci_idx, void *infop);
extern void do_replace_node(int argc, char **argv, int sci_idx, void *infop);
extern void do_split_node(int argc, char **argv, int sci_idx, void *infop);
extern void do_insert_node(int argc, char **argv, int sci_idx, void *infop);
extern void do_set_bmap(int argc, char **argv, int sci_idx, void *infop);
extern void do_print_all(int argc, char **argv, int sci_idx, void *infop);
extern void do_fix_parents(int argc, char **argv, int sci_idx, void *infop);
extern void do_info(int argc, char **argv, int sci_idx, void *infop);
extern void do_goto_block(int argc, char **argv, int sci_idx, void *infop);

/* htree.c */
extern void do_htree_dump(int argc, char **argv, int sci_idx, void *infop);
extern void do_dx_hash(int argc, char **argv, int sci_idx, void *infop);
extern void do_dirsearch(int argc, char **argv, int sci_idx, void *infop);

/* logdump.c */
extern void do_logdump(int argc, char **argv, int sci_idx, void *infop);

/* lsdel.c */
extern void do_lsdel(int argc, char **argv, int sci_idx, void *infop);

/* icheck.c */
extern void do_icheck(int argc, char **argv, int sci_idx, void *infop);

/* ncheck.c */
extern void do_ncheck(int argc, char **argv, int sci_idx, void *infop);

/* set_fields.c */
extern void do_set_super(int argc, char **, int sci_idx, void *infop);
extern void do_set_inode(int argc, char **, int sci_idx, void *infop);
extern void do_set_block_group_descriptor(int argc, char **, int sci_idx, void *infop);

/* unused.c */
extern void do_dump_unused(int argc, char **argv, int sci_idx, void *infop);

/* debugfs.c */
extern ss_request_table *extra_cmds;
extern const char *debug_prog_name;
extern void internal_dump_inode(FILE *, const char *, ext2_ino_t,
				struct ext2_inode *, int);

extern void do_dirty_filesys(int argc, char **argv, int sci_idx, void *infop);
extern void do_open_filesys(int argc, char **argv, int sci_idx, void *infop);
extern void do_close_filesys(int argc, char **argv, int sci_idx, void *infop);
extern void do_lcd(int argc, char **argv, int sci_idx, void *infop);
extern void do_init_filesys(int argc, char **argv, int sci_idx, void *infop);
extern void do_show_super_stats(int argc, char **argv, int sci_idx, void *infop);
extern void do_kill_file(int argc, char **argv, int sci_idx, void *infop);
extern void do_rm(int argc, char **argv, int sci_idx, void *infop);
extern void do_link(int argc, char **argv, int sci_idx, void *infop);
extern void do_undel(int argc, char **argv, int sci_idx, void *infop);
extern void do_unlink(int argc, char **argv, int sci_idx, void *infop);
extern void do_copy_inode(int argc, char *argv[], int sci_idx, void *infop);
extern void do_find_free_block(int argc, char **argv, int sci_idx, void *infop);
extern void do_find_free_inode(int argc, char **argv, int sci_idx, void *infop);
extern void do_stat(int argc, char **argv, int sci_idx, void *infop);
extern void do_dump_extents(int argc, char **argv, int sci_idx, void *infop);
extern void do_blocks(int argc, char *argv[], int sci_idx, void *infop);

extern void do_chroot(int argc, char **argv, int sci_idx, void *infop);
extern void do_clri(int argc, char **argv, int sci_idx, void *infop);
extern void do_freei(int argc, char **argv, int sci_idx, void *infop);
extern void do_seti(int argc, char **argv, int sci_idx, void *infop);
extern void do_testi(int argc, char **argv, int sci_idx, void *infop);
extern void do_freeb(int argc, char **argv, int sci_idx, void *infop);
extern void do_setb(int argc, char **argv, int sci_idx, void *infop);
extern void do_testb(int argc, char **argv, int sci_idx, void *infop);
extern void do_modify_inode(int argc, char **argv, int sci_idx, void *infop);
extern void do_list_dir(int argc, char **argv, int sci_idx, void *infop);
extern void do_change_working_dir(int argc, char **argv, int sci_idx, void *infop);
extern void do_print_working_directory(int argc, char **argv, int sci_idx, void *infop);
extern void do_write(int argc, char **argv, int sci_idx, void *infop);
extern void do_mknod(int argc, char **argv, int sci_idx, void *infop);
extern void do_mkdir(int argc, char **argv, int sci_idx, void *infop);
extern void do_rmdir(int argc, char **argv, int sci_idx, void *infop);
extern void do_show_debugfs_params(int argc, char **argv, int sci_idx, void *infop);
extern void do_expand_dir(int argc, char **argv, int sci_idx, void *infop);
extern void do_features(int argc, char **argv, int sci_idx, void *infop);
extern void do_bmap(int argc, char **argv, int sci_idx, void *infop);
extern void do_imap(int argc, char **argv, int sci_idx, void *infop);
extern void do_idump(int argc, char *argv[], int sci_idx, void *infop);
extern void do_set_current_time(int argc, char **argv, int sci_idx, void *infop);
extern void do_supported_features(int argc, char **argv, int sci_idx, void *infop);
extern void do_punch(int argc, char **argv, int sci_idx, void *infop);
extern void do_fallocate(int argc, char **argv, int sci_idx, void *infop);
extern void do_symlink(int argc, char **argv, int sci_idx, void *infop);

extern void do_dump_mmp(int argc, char **argv, int sci_idx, void *infop);
extern void do_set_mmp_value(int argc, char **argv, int sci_idx, void *infop);

extern void do_freefrag(int argc, char **argv, int sci_idx, void *infop);
extern void do_filefrag(int argc, char *argv[], int sci_idx, void *infop);

/* do_journal.c */

extern void do_journal_write(int argc, char *argv[], int sci_idx, void *infop);
extern void do_journal_open(int argc, char *argv[], int sci_idx, void *infop);
extern void do_journal_close(int argc, char *argv[], int sci_idx, void *infop);
extern void do_journal_run(int argc, char *argv[], int sci_idx, void *infop);

/* quota.c */
extern void do_list_quota(int argc, char *argv[], int sci_idx, void *infop);
extern void do_get_quota(int argc, char *argv[], int sci_idx, void *infop);

/* util.c */
extern __s64 string_to_time(const char *arg);
errcode_t read_list(char *str, blk64_t **list, size_t *len);

/* xattrs.c */
void dump_inode_attributes(FILE *out, ext2_ino_t ino);
void do_get_xattr(int argc, char **argv, int sci_idx, void *infop);
void do_set_xattr(int argc, char **argv, int sci_idx, void *infop);
void do_rm_xattr(int argc, char **argv, int sci_idx, void *infop);
void do_list_xattr(int argc, char **argv, int sci_idx, void *infop);
void raw_inode_xattr_dump(FILE *f, unsigned char *buf, unsigned int len);
void block_xattr_dump(FILE *f, unsigned char *buf, unsigned int len);

/* zap.c */
extern void do_zap_block(int argc, char **argv, int sci_idx, void *infop);
extern void do_block_dump(int argc, char **argv, int sci_idx, void *infop);
extern void do_byte_hexdump(FILE *fp, unsigned char *buf, size_t bufsize);
