--
--  Copyright (C) 2012 Reto Buerki
--  Copyright (C) 2012 Adrian-Ken Rueegsegger
--  HSR Hochschule fuer Technik Rapperswil
--
--  This program is free software; you can redistribute it and/or modify it
--  under the terms of the GNU General Public License as published by the
--  Free Software Foundation; either version 2 of the License, or (at your
--  option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
--
--  This program is distributed in the hope that it will be useful, but
--  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
--  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
--  for more details.
--

with Interfaces.C.Strings;

package Esa_Event_Service
is

   procedure Init (Address : Interfaces.C.Strings.chars_ptr);
   pragma Export (C, Init, "ees_server_init");
   --  Initialize Esa Event Service (EES) with given address.

   procedure Finalize;
   pragma Export (C, Finalize, "ees_server_finalize");
   --  Finalize EES.

end Esa_Event_Service;
