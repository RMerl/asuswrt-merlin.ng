Date:Aug 2019

The purpose of the "haltest" is to test the wifi HAL list of APIs.
The HAL test suite can be tested on the BRCM reference platform for all the HAL command level APIs.

The HAL test suite follows the existing UTF framework used by SVT test team(Arvind Kebbali/Archana Murthy)

The UTF framework needs the DUT(Device Under Test) to be connected to a Console logger and Endpoint
The Bridge interface on the DUT and the console logger/Endpoint should all be in the same subnet. For Example 192.168.0.xx

The Console logger and Endpoint can be configured on the same machine too(Preferred to have this).

The DUT and the Console logger/Endpoint is connected to the lab network through USB interface to execute the
commands from remote PC as well through SSH/telnet session.

To have the initial setup, it is NECESSARY to follow the steps at confluence page link
http://confluence.broadcom.net/display/WLAN/Test+Automation+for+RDKB+WiFi+HAL+API
and have the Console logger/Endpoint ready. Please refer the image under "Test setup" to know the basic setup.

A Dummy STA setup is also needed same as above, but without DUT or STA.
A Dummy STA does not need console logger setup unless it executes commands remotely in the script.

Checkpoints before exectuing HAL test suite:
1. Be able to access the NFS mounted home directory from Console logger/Endpoint
2. Be able to ping Bridge interface from Console logger/Endpoint on which DUT is connected
3. Be able to ping Bridge interface from Console logger/Endpoint on which dummy STA is connected
4. Be able to SSH to Ethernet interface and loging to mapped home directory like /home/sj943186/unittest
5. Once step 4 is confirmed, try accessing RG console.
   For Example, run command "./apshell 10.19.87.227:40003" (port num may change - refer confluence page)

About utf25.tcl:
This is the file used for configuration of Console logger/Endpoint. Please make sure the IP addresses match

About hal_set_config:
This file stores the IOVAR/command values used for SET commands

About hal_latest.test
This is the TCL script file used to execute the set of commands to be validated

All the above mentioned files need to be placed under the UTF framework in their respective folders before execution
/home/sj943186/unittest/Test/hal_latest.test
/home/sj943186/unittest/Test/hal_set_config
/home/sj943186/unittest/utfconf/utf25.tcl
