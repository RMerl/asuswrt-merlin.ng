#ifndef DB_H
#define DB_H

#ifdef CONFIG_CLIENT_DB
/**
 * mapd_db_read - Parse non-voltatile client database and store it in internal clientDB
 * @global: Pointer to mapd_global
 * Returns: 0 on success and -1 on failure
 *
 * This function reads client data, parses its contents, and stores them in
 * client_db.
 *
 */
int mapd_client_db_read(struct mapd_global *global);

/**
 * mapd_client_db_flush - Flush the internal clientDB to persistent nv storage
 * @global: Pointer to mapd_global.
 * @all: 0: Flush only the "dirty" clients,  1: Flush all
 * Returns: 0 on success, -1 on failure
 *
 * This function write all persistent attributes of all clients into an
 * external database (e.g. a text file) in a format that can be
 * read with mapd_client_db_read().
 */
int mapd_client_db_flush(struct mapd_global *global, uint8_t all);

/**
 * mapd_write_client_db_entry - Write a new client's entry into 
 * into persistent nv storage. Stores only persistent atttributes.
 * @global: Pointer to mapd_global.
 * @cli: Pointer to client struct
 * Returns: 0 on success, -1 on failure
 *
 * This function writes all persistent client attribute into an external database (e.g.,
 * a text file) in a format that can be read with mapd_client_db_read().
 * Caller needs to ensure that it's not a duplicate entry.
 */
int mapd_write_client_db_entry(struct mapd_global *global, struct client *cli);

/**
 * mapd_uppdate_client_db_entry - Update an already existing client entry in 
 * external database.
 * @global: Pointer to mapd_global.
 * @cli: Pointer to client struct
 * Returns: 0 on success, -1 on failure
 *
 * This function updates all persistent client attribute into an external database (e.g.,
 * a text file) in a format that can be read with mapd_client_db_read().
 * Caller needs to ensure that it's not a duplicate entry.
 */
int mapd_update_client_db_entry(struct mapd_global *global, struct client *cli);

int mapd_read_config_file(struct own_1905_device *dev);

#else
static inline int mapd_client_db_read(struct mapd_global *global)
{
	mapd_printf(MSG_INFO, "Persistent client_db not available");
	return 0;
}

static inline int mapd_client_db_flush(struct mapd_global *global, uint8_t all)
{
	mapd_printf(MSG_INFO, "Persistent client_db not available");
	return 0;
}

static inline int mapd_write_client_db_entry(struct mapd_global *global, struct client *cli)
{
	mapd_printf(MSG_INFO, "Persistent client_db not available");
	return 0;
}

static inline int mapd_update_client_db_entry(struct mapd_global *global, struct client *cli)
{
	mapd_printf(MSG_INFO, "Persistent client_db not available");
	return 0;
}
static inline int mapd_read_config_file(struct own_1905_device *dev)
{
	mapd_printf(MSG_INFO, "Persistent client_db not available");
	return 0;
}

#endif

#endif /* DB_H */
