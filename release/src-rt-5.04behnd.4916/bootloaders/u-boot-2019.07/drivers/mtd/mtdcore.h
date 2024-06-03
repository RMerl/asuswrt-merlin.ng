/*
 * These are exported solely for the purpose of mtd_blkdevs.c and mtdchar.c.
 * You should not use them for _anything_ else.
 */

extern struct mutex mtd_table_mutex;

int add_mtd_device(struct mtd_info *mtd);
int del_mtd_device(struct mtd_info *mtd);
int add_mtd_partitions(struct mtd_info *, const struct mtd_partition *, int);
int del_mtd_partitions(struct mtd_info *);
int parse_mtd_partitions(struct mtd_info *master, const char * const *types,
			 struct mtd_partition **pparts,
			 struct mtd_part_parser_data *data);

int __init init_mtdchar(void);
void __exit cleanup_mtdchar(void);
