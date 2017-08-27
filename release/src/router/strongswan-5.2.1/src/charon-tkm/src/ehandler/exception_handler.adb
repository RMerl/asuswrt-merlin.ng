--
--  Copyright (C) 2012 Reto Buerki
--  Copyright (C) 2012 Adrian-Ken Rueegsegger
--  Hochschule fuer Technik Rapperswil
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

with Ada.Exceptions;

with GNAT.Exception_Actions;

with Interfaces.C.Strings;

package body Exception_Handler
is

   procedure Charon_Terminate (Message : Interfaces.C.Strings.chars_ptr);
   pragma Import (C, Charon_Terminate, "charon_terminate");

   procedure Bailout (Ex : Ada.Exceptions.Exception_Occurrence);
   --  Signal critical condition to charon daemon.

   -------------------------------------------------------------------------

   procedure Bailout (Ex : Ada.Exceptions.Exception_Occurrence)
   is
   begin
      if Ada.Exceptions.Exception_Name (Ex) = "_ABORT_SIGNAL" then

         --  Ignore runtime-internal abort signal exception.

         return;
      end if;

      Charon_Terminate (Message => Interfaces.C.Strings.New_String
                        (Ada.Exceptions.Exception_Information (Ex)));
   end Bailout;

   -------------------------------------------------------------------------

   procedure Init
   is
   begin
      GNAT.Exception_Actions.Register_Global_Action
        (Action => Bailout'Access);
   end Init;

end Exception_Handler;
