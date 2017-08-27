#ifndef DB_NAMES_H_
#define DB_NAMES_H_ 1

#define IDNAME_MAX 256

struct db_entry {
	struct db_entry *next;
	unsigned int id;
	char *name;
};

struct db_names {
	unsigned int size;
	struct db_entry *cached;
	struct db_entry **hash;
	int max;
};

struct db_names *db_names_alloc(void);
int db_names_load(struct db_names *db, const char *path);
void db_names_free(struct db_names *db);

char *id_to_name(struct db_names *db, int id, char *name);
int name_to_id(struct db_names *db, int *id, const char *name);

#endif
