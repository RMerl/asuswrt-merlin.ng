/* -*- mode: c; c-basic-offset: 2 -*- */
/* 
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 * Copyright (C) 2007-2012 David Bird (Coova Technologies) <support@coova.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef _RADIUS_WISPR_H
#define _RADIUS_WISPR_H

#define RADIUS_VENDOR_WISPR                         14122
#define	RADIUS_ATTR_WISPR_LOCATION_ID	                1
#define	RADIUS_ATTR_WISPR_LOCATION_NAME		        2 /* string */
#define	RADIUS_ATTR_WISPR_LOGOFF_URL		        3 /* string */
#define	RADIUS_ATTR_WISPR_REDIRECTION_URL		4 /* string */
#define	RADIUS_ATTR_WISPR_BANDWIDTH_MIN_UP		5 /* integer */
#define	RADIUS_ATTR_WISPR_BANDWIDTH_MIN_DOWN	        6 /* integer */
#define	RADIUS_ATTR_WISPR_BANDWIDTH_MAX_UP		7 /* integer */
#define	RADIUS_ATTR_WISPR_BANDWIDTH_MAX_DOWN	        8 /* integer */
#define	RADIUS_ATTR_WISPR_SESSION_TERMINATE_TIME	9 /* string */
#define	RADIUS_ATTR_WISPR_SESSION_TERMINATE_END_OF_DAY 10 /* string */
#define	RADIUS_ATTR_WISPR_BILLING_CLASS_OF_SERVICE     11 /* string */

#endif	/* !_RADIUS_WISPR_H */
