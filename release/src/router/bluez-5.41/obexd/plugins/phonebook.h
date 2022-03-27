/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#define EOL	"\r\n"
#define VCARD_LISTING_BEGIN \
	"<?xml version=\"1.0\"?>" EOL\
	"<!DOCTYPE vcard-listing SYSTEM \"vcard-listing.dtd\">" EOL\
	"<vCard-listing version=\"1.0\">" EOL
#define VCARD_LISTING_ELEMENT "<card handle = \"%d.vcf\" name = \"%s\"/>" EOL
#define VCARD_LISTING_END "</vCard-listing>"

#define PB_TELECOM_FOLDER "/telecom"
#define PB_CONTACTS_FOLDER "/telecom/pb"
#define PB_CALENDAR_FOLDER "/telecom/cal"
#define PB_NOTES_FOLDER "/telecom/nt"
#define PB_CALLS_COMBINED_FOLDER "/telecom/cch"
#define PB_CALLS_INCOMING_FOLDER "/telecom/ich"
#define PB_CALLS_MISSED_FOLDER "/telecom/mch"
#define PB_CALLS_OUTGOING_FOLDER "/telecom/och"
#define PB_CALLS_SPEEDDIAL_FOLDER "/telecom/spd"
#define PB_CALLS_FAVORITE_FOLDER "/telecom/fav"
#define PB_LUID_FOLDER "/telecom/pb/luid"

#define PB_CONTACTS "/telecom/pb.vcf"
#define PB_CALLS_COMBINED "/telecom/cch.vcf"
#define PB_CALLS_INCOMING "/telecom/ich.vcf"
#define PB_CALLS_MISSED "/telecom/mch.vcf"
#define PB_CALLS_OUTGOING "/telecom/och.vcf"
#define PB_CALLS_SPEEDDIAL "/telecom/spd.vcf"
#define PB_CALLS_FAVORITE "/telecom/fav.vcf"
#define PB_DEVINFO "/telecom/devinfo.txt"
#define PB_INFO_LOG "/telecom/pb/info.log"
#define PB_CC_LOG "/telecom/pb/luid/cc.log"


struct apparam_field {
	/* list and pull attributes */
	uint16_t maxlistcount;
	uint16_t liststartoffset;

	/* pull and vcard attributes */
	uint64_t filter;
	uint8_t format;

	/* list attributes only */
	uint8_t order;
	uint8_t searchattrib;
	char *searchval;
};

/*
 * Interface between the PBAP core and backends to retrieve
 * all contacts that match the application parameters rules.
 * Contacts will be returned in the vcard format.
 */
typedef void (*phonebook_cb) (const char *buffer, size_t bufsize,
		int vcards, int missed, gboolean lastpart, void *user_data);

/*
 * Interface between the PBAP core and backends to
 * append a new entry in the PBAP folder cache.
 */
#define PHONEBOOK_INVALID_HANDLE 0xffffffff
typedef void (*phonebook_entry_cb) (const char *id, uint32_t handle,
					const char *name, const char *sound,
					const char *tel, void *user_data);

/*
 * After notify all entries to PBAP core, the backend
 * needs to notify that the operation has finished.
 */
typedef void (*phonebook_cache_ready_cb) (void *user_data);


int phonebook_init(void);
void phonebook_exit(void);

/*
 * Changes the current folder in the phonebook back-end. The PBAP core
 * doesn't validate or restrict the possible values for the folders,
 * allowing non-standard backends implementation which doesn't follow
 * the PBAP virtual folder architecture. Validate the folder's name
 * is responsibility of the back-ends.
*/
char *phonebook_set_folder(const char *current_folder,
		const char *new_folder, uint8_t flags, int *err);

/*
 * phonebook_pull should be used only to prepare pull request - prepared
 * request data is returned by this function. Start of fetching data from
 * back-end will be done only after calling phonebook_pull_read with this
 * returned request given as a parameter.
 *
 * phonebook_req_finalize MUST always be used to free associated resources.
 */
void *phonebook_pull(const char *name, const struct apparam_field *params,
				phonebook_cb cb, void *user_data, int *err);

/*
 * phonebook_pull_read should be used to start getting results from back-end.
 * The back-end can return data as one response or can return it many parts.
 * After obtaining one part, PBAP core need to call phonebook_pull_read with
 * the same request again to get more results from back-end.
 * The back-end MUST return only the content based on the application
 * parameters requested by the client.
 *
 * Returns error code or 0 in case of success
 */
int phonebook_pull_read(void *request);

/*
 * Function used to retrieve a contact from the backend. Only contacts
 * found in the cache are requested to the back-ends. The back-end MUST
 * return only the content based on the application parameters requested
 * by the client.
 *
 * Return value is a pointer to asynchronous request to phonebook back-end.
 * phonebook_req_finalize MUST always be used to free associated resources.
 */
void *phonebook_get_entry(const char *folder, const char *id,
				const struct apparam_field *params,
				phonebook_cb cb, void *user_data, int *err);

/*
 * PBAP core will keep the contacts cache per folder. SetPhoneBook or
 * PullvCardListing can invalidate the cache if the current folder changes.
 * Cache will store only the necessary information required to reply to
 * PullvCardListing request and verify if a given contact belongs to the
 * source.
 *
 * Return value is a pointer to asynchronous request to phonebook back-end.
 * phonebook_req_finalize MUST always be used to free associated resources.
 */
void *phonebook_create_cache(const char *name, phonebook_entry_cb entry_cb,
		phonebook_cache_ready_cb ready_cb, void *user_data, int *err);

/*
 * Finalizes request to phonebook back-end and deallocates associated
 * resources. Operation is canceled if not completed. This function MUST
 * always be used after any of phonebook_pull, phonebook_get_entry, and
 * phonebook_create_cache invoked.
 *
 * request is a pointer to asynchronous operation returned by phonebook_pull,
 * phonebook_get_entry, and phonebook_create_cache.
 */
void phonebook_req_finalize(void *request);
