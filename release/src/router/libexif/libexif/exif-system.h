/*! \file exif-system.h
 * \brief System specific definitions, not for installation!
 */
/*
 * Copyright (c) 2007 Hans Ulrich Niedermann <gp@n-dimensional.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA.
 */

#ifndef LIBEXIF_EXIF_SYSTEM_H
#define LIBEXIF_EXIF_SYSTEM_H

#if defined(__GNUC__) && (__GNUC__ >= 2)
# define UNUSED(param) UNUSED_PARAM_##param __attribute__((unused))
#else
# define UNUSED(param) param
#endif

#endif /* !defined(LIBEXIF_EXIF_SYSTEM_H) */
