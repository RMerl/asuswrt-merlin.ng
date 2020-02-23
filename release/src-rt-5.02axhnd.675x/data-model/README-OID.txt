
Starting in 4.16L.01, all objects must have an object ID (OID) assigned to them.
Objects should get OID's in the following range:

   0 -   499: TR98 objects (including X_BROADCOM_COM objects embedded in TR98 tree)
  500 -  799: TR104 voice objects (including X_BROADCOM_COM objects in the TR104 tree)
  800 -  899: TR140 storage objects (including X_BROADCOM_COM objects in the TR140 tree)
  900 -  919: TR143 Diag objects
  920 -  929: TR157 Periodic Statistics Objects
  930 -  999: reserved
 1000 - 1999: TR181 objects
 2000 - 2099: Broadcom PON related objects
 2100 - 2499: ITU GPON objects
 2500 - 2599: EPON objects
 2600 - 2699: WiFi related objects
 2700 - 2999: reserved
 3000 - 3999: Broadcom objects
 4000 - 4999: Customer objects
 5000 - 65535: reserved

The first real object in each cms-dm-*.xml file MUST have an oid=num attribute.
The generate_from_dm.pl script will give all subsequent objects in that file 
the next oid number.

