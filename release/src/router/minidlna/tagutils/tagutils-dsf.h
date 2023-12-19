//=========================================================================
// FILENAME	: tagutils-dsf.h
// DESCRIPTION	: DSF metadata reader
//=========================================================================
// Copyright (c) 2014 Takeshich NAKAMURA
//=========================================================================

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
static int _get_dsffileinfo(char *file, struct song_metadata *psong);
static int _get_dsftags(char *file, struct song_metadata *psong);
