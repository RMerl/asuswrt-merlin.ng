
OBJECT ID's (OID's)
-------------------

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
 2700 - 2899: reserved
 2900 - 2999: BBF TR181 XPON objects
 3000 - 3999: Broadcom objects
 4000 - 4999: Customer objects
 5000 - 65535: reserved

The first real object in each cms-dm-*.xml file MUST have an oid=num attribute.
The generate_from_dm.pl script will give all subsequent objects in that file 
the next oid number.

The data model generation script will verify that all OID's are unique, and
will stop with a fatal error if there are duplicate or overlapping OID's.

You can see the OID assignments in the generated data model files or on the
running system by typing "dumpoid" in the CMS or MDM CLI.



Data Model Merge Directories:
------------------------------

Because BDK has multiple Distributed MDM's, multiple data model files are 
needed (cms-data-model-merged2*).  These data model files are created by the
various "merge" directories.  For example, merge-dev2-devinfo.d contains a
set of text control files which assembles the data model for the DeviceInfo
component (cms-data-model-merge2-devinfo.xml).  merge-dev2-sysmgmt.d contains
a set of files which assembles the data model for
the sysmgmt component (cms-data-model-merged2-sysmgmt.xml).

Each text control file contains one or more lines with:
<cmd> <data model snippet to use> <where to attach the snippet to the existing data model>

The text files are processed in lexicographical order.  For example, in 
merge-dev2-sysmgmt.d, 1000-base-tr181-dev2.txt is processed first, followed
by 1010-tr181-begin.txt, and so on.  Note that even though the 4 digit number
used in the filenames are similar to the OID's, (and in some cases the number
in the filename exactly matches the OID in the data model snippet), they are
actually independent of each other.  The numbers in the filenames specifies
the order in which the files are processed, and these files will build up the
data model for its component.  If file B depends on (needs to add its objects
onto an existing object) that was added to the data model by file A, then
file B must be put later in the order (has a higher number) than file A.

For example, 3000-bcm-appcfgstub.txt adds the X_BROADCOM_COM_AppCfg object to
the data model.  Any other file which wants to add an object to 
the X_BROADCOM_COM_AppCfg object (e.g. 3280-bcm-httpapp.txt) must come after
3000-bcm-appcfgstub.txt.  X_BROADCOM_COM_AppCfg has OID 3130, and
X_BROADCOM_COM_AppCfg.HttpdCfg. has OID 3150.  So there is no requirement that
the 4 digit number in the filename matches the OIDs that it adds to the
data model.  However, you cannot rename 3280-bcm-httpdapp.txt to
2800-bcm-httpdapp.txt because it wants to add its object to the 
X_BROADCOM_COM_AppCfg object, but that object will get added by
3000-bcm-appcfgstub.txt, so 3280-bcm-httpdapp.txt must be processed AFTER
3000-bcm-appcfgstub.txt.
