package body Tkmrpc.Servers.Ees
is

   --------------------------------
   -- charon callback signatures --
   --------------------------------

   procedure Charon_Esa_Acquire
     (Result : out Results.Result_Type;
      Sp_Id  :     Types.Sp_Id_Type);
   pragma Import (C, Charon_Esa_Acquire, "charon_esa_acquire");

   procedure Charon_Esa_Expire
     (Result   : out Results.Result_Type;
      Sp_Id    :     Types.Sp_Id_Type;
      Spi_Rem  :     Types.Esp_Spi_Type;
      Protocol :     Types.Protocol_Type;
      Hard     :     Types.Expiry_Flag_Type);
   pragma Import (C, Charon_Esa_Expire, "charon_esa_expire");

   -------------------------------------------------------------------------

   procedure Esa_Acquire
     (Result : out Results.Result_Type;
      Sp_Id  :     Types.Sp_Id_Type)
   is
   begin
      Charon_Esa_Acquire (Result => Result,
                          Sp_Id  => Sp_Id);
   end Esa_Acquire;

   -------------------------------------------------------------------------

   procedure Esa_Expire
     (Result   : out Results.Result_Type;
      Sp_Id    :     Types.Sp_Id_Type;
      Spi_Rem  :     Types.Esp_Spi_Type;
      Protocol :     Types.Protocol_Type;
      Hard     :     Types.Expiry_Flag_Type)
   is
   begin
      Charon_Esa_Expire (Result   => Result,
                         Sp_Id    => Sp_Id,
                         Spi_Rem  => Spi_Rem,
                         Protocol => Protocol,
                         Hard     => Hard);
   end Esa_Expire;

   -------------------------------------------------------------------------

   procedure Finalize
   is
   begin
      null;
   end Finalize;

   -------------------------------------------------------------------------

   procedure Init
   is
   begin
      null;
   end Init;

end Tkmrpc.Servers.Ees;
