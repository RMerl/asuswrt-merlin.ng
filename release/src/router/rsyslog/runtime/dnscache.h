/* Definitions for dnscache module.
 *
 * Copyright 2011-2013 Adiscon GmbH.
 *
 * This file is part of the rsyslog runtime library.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *       -or-
 *       see COPYING.ASL20 in the source distribution
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef INCLUDED_DNSCACHE_H
#define INCLUDED_DNSCACHE_H

rsRetVal dnscacheInit(void);
rsRetVal dnscacheDeinit(void);
rsRetVal dnscacheLookup(struct sockaddr_storage *addr, prop_t **fqdn, prop_t **fqdnLowerCase,
prop_t **localName, prop_t **ip);

#endif /* #ifndef INCLUDED_DNSCACHE_H */
