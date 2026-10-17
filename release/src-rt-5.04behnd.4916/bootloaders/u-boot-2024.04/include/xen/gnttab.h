/* SPDX-License-Identifier: GPL-2.0
 *
 * (C) 2006, Steven Smith <sos22@cam.ac.uk>
 * (C) 2006, Grzegorz Milos <gm281@cam.ac.uk>
 * (C) 2020, EPAM Systems Inc.
 */
#ifndef __GNTTAB_H__
#define __GNTTAB_H__

#include <xen/interface/grant_table.h>

void init_gnttab(void);
void fini_gnttab(void);

grant_ref_t gnttab_alloc_and_grant(void **map);
grant_ref_t gnttab_grant_access(domid_t domid, unsigned long frame,
				int readonly);
int gnttab_end_access(grant_ref_t ref);
const char *gnttabop_error(int16_t status);

void get_gnttab_base(phys_addr_t *gnttab_base, phys_size_t *gnttab_sz);

#endif /* !__GNTTAB_H__ */
