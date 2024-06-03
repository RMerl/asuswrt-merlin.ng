
Any files added to this directory, ending in ".conf" will be executed
in collating sequence order with 2 arguments 
1) The path to the target profile
2) The path to the configuration file as it currently stands

each script should read the target profile and use that information to
read-modify-write the configuration file.

Customers should add new files to make further configuration changes
to avoid future merge issues on subsequent releases.


