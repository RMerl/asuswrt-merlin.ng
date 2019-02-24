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

with Anet.Sockets.Unix;
with Anet.Receivers.Stream;

with Tkmrpc.Dispatchers.Ees;
with Tkmrpc.Process_Stream;

pragma Elaborate_All (Anet.Receivers.Stream);
pragma Elaborate_All (Tkmrpc.Process_Stream);

package body Esa_Event_Service
is

   package Unix_TCP_Receiver is new Anet.Receivers.Stream
     (Socket_Type       => Anet.Sockets.Unix.TCP_Socket_Type,
      Address_Type      => Anet.Sockets.Unix.Full_Path_Type,
      Accept_Connection => Anet.Sockets.Unix.Accept_Connection);

   procedure Dispatch is new Tkmrpc.Process_Stream
     (Dispatch     => Tkmrpc.Dispatchers.Ees.Dispatch,
      Address_Type => Anet.Sockets.Unix.Full_Path_Type);

   Sock     : aliased Anet.Sockets.Unix.TCP_Socket_Type;
   Receiver : Unix_TCP_Receiver.Receiver_Type (S => Sock'Access);

   -------------------------------------------------------------------------

   procedure Finalize
   is
   begin
      Receiver.Stop;
   end Finalize;

   -------------------------------------------------------------------------

   procedure Init (Address : Interfaces.C.Strings.chars_ptr)
   is
      Path : constant String := Interfaces.C.Strings.Value (Address);
   begin
      Sock.Init;
      Sock.Bind (Path => Anet.Sockets.Unix.Path_Type (Path));
      Receiver.Listen (Callback => Dispatch'Access);
   end Init;

end Esa_Event_Service;
