Brief Description of Watchdog Timer support in BCA SoC's

Michael Wang
8/24/20

Background:

Most recent SoC from the BCA group includes support for a watchdog timer.
If the SoC supports a watchdog timer, UBoot will use that watchdog timer
during its startup and reboot sequences.  In particular, UBoot will set the
watchdog timer for 80 seconds right before it hands control to the kernel
and userspace, so userspace MUST deal with the watchdog timer, otherwise, the
watchdog timer will expire and the system will reboot.


Menuconfig:

Watchdog functionality in the kernel and userspace is controlled via 2 
menuconfig options:

kernel configuration > Misc Drivers > Watchdog Timer

and

Other Features > Firmware Upgrade Watchdog Timer


Basic Watchdog:

The first option is considered the minimal/basic watchdog timer support.
Recall that UBoot will always set the watchdog timer before it hands off
control to the kernel/userspace.  There is no support in the kernel to
ping/feed/pet the watchdog as the kernel boots, so the first opportunity to
ping/feed/pet the watchdog is during userspace startup.  This is done in
S26wdtd (installed from userspace/publis/apps/wdtctl).  This script will
start a watchdog timer daemon (wdtd) which will configure the watchdog timer
for 30 seconds and ping the watchdog every 30/4=7 seconds.  This wdtd will run
continuously while userspace is up.  When the system is rebooted, it is killed
along with all the other userspace daemons.  When wdtd is killed (with SIGHUP),
it will stop the watchdog timer.


Firmware Upgrade Watchdog Timer:

The second option is a more complex option which is not enabled by default
(it may be enabled by default later).  It is used with the "Boot once" 
feature of the firmware image management code to implement protection against
bad firmware upgrades.  Also, the CMS/BDK TR181 data model is required for
this feature.

Briefly, it works like this:

1. For example, suppose the system is currently running the firmware image in
   partition 1.  When httpd or tr69c receives a new firmware image, it will
   write it to partition 2.  Then it will set BOOT_SET_PART2_IMAGE_ONCE.
   It will then kill the wdtd that is currently running, and then start the
   watchdog with a 60 second timer.  It will then reboot the system.  (TODO: add code pointer here)
   This means userspace and kernel has 60 seconds to shutdown and hand control
   to UBoot.

2. When control reaches UBoot, it will ping the timer one more more times
   during its run sequence, and will set the timer for 80 seconds right before
   it hands control to the kernel in the new image.

3. The kernel does not ping the watchdog, so it must complete its bootup and
   hand control to userspace before the watchdog timer expires.  Kernel bootup
   is very fast (typically only about 10 seconds), so kernel should be
   able to hand control to userspace before watchdog expires.

4. When control is handed to userspace, it runs bcm_boot_launcher to 
   execute the startup scripts.  S26wdtd will set the watchdog for 60 seconds,
   but does NOT start the daemon (wdtd).  That means from this point,
   the system has 60 seconds to reach the final point.

5. When control reaches either CMS TR181 MDM, or the BDK deviceinfo_md, that
   is considered a successful boot of the new firmware image.  The code will (TODO: add code pointer here)
   commit the new firmware image in partition 2 with BOOT_SET_PART2_IMAGE,
   and it will start the wdtd daemon.  The wdtd daemon will keep running as
   long as userspace is still up.

If the system hangs or crashes at any point between steps 1 and 5, the
watchdog timer will expire, reboot the system, and UBoot will revert to the
previous image in partition 1.

Note that step 5 requires the TR181 Device.DeviceInfo.FirmwareImage object.
CMS TR98 systems can port the relevant code from TR181 to implement the same
functionality.

Note that other runtime systems (e.g. RDK) which do not use CMS/BDK may also
port the relevant code snippets to their system.



