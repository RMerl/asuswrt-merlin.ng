## Purpose:
  1. Work as a development environment for Broadcom Broadband Router Software.
     Using the tool, we will get rid of most environment related building problems.
     Because it provides the same building environment for everyone using it. No more differences in linux distros, installed libraries or binaries.

  2. Work as a sandbox to run DESKTOP BEEP.
     Features as a simulator:
     * Support using broadcom's dbus-daemon and lxc.
     * Support running smd, pmd, httpd and other builtin services during the simulator startup.
     * Support DU installation and uninstall from WebGui.
     * Support EU start and stop from WebGui for both "Super User" mode and normal mode.
     * Support consoled and loglevel set/get.
     * Easily available debugging tools such as strace, gdb and objdump to debug the programs.
     * Will not mess up the host linux environment. Whatever unwanted side-effect happens inside a simulator, it stays inside the simulator.
     * Destroy && restart a simulator is fast and light-weighted.

## Dependency:
   This project depends on docker. You will need to install docker on your linux host.
   Please refer to below document for docker installation:
       https://www.docker.com/get-docker
   The "docker" command, after fresh installation to Linux host, can only be run by using "sudo" command.
   However, I recommend to add user to run "docker" command without "sudo":
       https://docs.docker.com/engine/installation/linux/linux-postinstall/#manage-docker-as-a-non-root-user

## How to build:
  * Build the docker image:
  `docker build -t TAGNAME .`
   to create a docker image named "desktop-beep"

## Usage:
### Use the docker container as a building environment:
  You can start the container by command:
```bash
  docker run --rm -it -v /path_to_branch:/home/beepuser/workspace desktop-beep
```

  The "path\_to\_branch" is the directory to the code branch of SDK.
  For example, if you want to work with "REL\_502L03" branch, and synced the code to "/projects/REL\_502L03",
  the command to start the docker container is:
```bash
  docker run --rm -it -v /projects/REL_502L03:/home/beepuser/workspace desktop-beep
```

  The docker container includes ccache, which can speed up builds significantly after the first build.
  To use ccache, please follow below steps:
  1. mount the ccache directory to the container. For example:
```bash
  docker run --rm -it -v /path_to_branch:/home/beepuser/workspace -v /path_to_ccache_dir:/home/beepuser/ccache desktop-beep
```
  2. Once inside the console of the container, add the wrapper of toolchains. For example:
```bash
  sudo add_cached.sh /opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25
```
  3. Export the wrapped toolchain path to the build system:
```bash
  export TOOLCHAIN_BASE="/opt/toolchains/cached"
```

  You can now build broadcom CPE software inside docker container:
  1. Regular test build: `make PROFILE=XXXX` in "/home/beepuser/workspace".
  2. Run release script: `dorel963xx -p PROFILE_NAME -y` in _release_ directory of the workspace.

### Generate data release which can run DESKTOP BEEP
  To generate DESKTOP BEEP objects properly for a data release, both the base profile and beep profile need to be generated:
```bash
  ./dorel963xx -F -y -p 963138GW -p 963138GW_BEEP
```
  Once the data release is generated, untar both data_src tar ball and bin_beep tar ball to create the code base for data release:
```bash
   mkdir data_full_release; cd ./data_full_release
   tar xvzf ../bcm963xx_5.02L.05test4_data_binary_release.tar.gz
   mkdir bcm963xx; cd bcm963xx
   tar xvzf ../bcm963xx_5.02L.05test4_data_src.tar.gz
   cd ../../; mkdir bin_beep; cd ./bin_beep
   tar xvzf ../bcm963xx_5.02L.05test4_bin_beep.tar.gz
   cd ../data_full_release/bcm963xx
   tar xvzf ../../bin_beep/bcm963xx_5.02L.05test4_data_bin_beep.tar.gz
```
   You can run DESKTOP BEEP simulator in the standard way.

### Generate consumer release which can run DESKTOP BEEP
   In the data release source code:
```bash
   cd ./release
   ./do_consumer_release -F -p 963138GW_BEEP_DESKTOP
```

   Please note the script do\_consumer\_release supports a single profile currently. If you input more than one profile,the behaviour is broken.
   With the generated consumer release, start desktop-beep container from its root directory, and run below command to build DESKTOP BEEP from there:
```bash
   echo "963138GW_BEEP_DESKBEEP" > .last_profile
   dt.sh noregen
```

### Use the docker container as a sand box for DESKTOP LINUX:
  1. Start the docker container the same way as above: <br>
```bash
     docker run --rm -it --privileged=true -p 44480:44480 -p 44843:44843 -v /path_to_branch:/home/beepuser/workspace desktop-beep
```

     The docker container needs to be a privileged one, because BEEP needs to access the cgroup tree from within the container.
  2. In "/home/beepuser/workspace", run:
```bash
     ./release/maketargets 963138GW_BEEP;

     dt.sh beep 963138GW_BEEP

```
     After the first build, if you want to rebuild the DESKTOP_LINUX for the same PROFILE, simply run:  
```bash
     dt.sh noregen
```

  3. Now start the simulater. In "/home/beepuser/workspace", run:
```bash
     ../scripts/simulate.sh
```
     The script will start dbus-daemon, smd, pmd, and cwmpd.

  4. Start a browser in the host, accessing below address:
     http://localhost:44480

  5. Navigate to the "Modular Software"->"Deployment Unit" tab, install cwmpd DU from the webgui.

  6. Navigate to the "Modular Software"->"Execution Unit" tab, start the cwmpd as a service.

  7. Verify the system is running by below command in the simulator's terminal):  
```bash
     $cd /local/modsw/tr157du/beep/app_cwmpctl
     $./cwmpctl getparametervalues --name InternetGatewayDevice.DeviceInfo.
     [#1]name:InternetGatewayDevice.DeviceInfo.Manufacturer type:string value:Broadcom
     [#2]name:InternetGatewayDevice.DeviceInfo.ManufacturerOUI type:string value:001122
     [#3]name:InternetGatewayDevice.DeviceInfo.ModelName type:string value:DESKTOP_LINUX
     [#4]name:InternetGatewayDevice.DeviceInfo.Description type:string value:
     [#5]name:InternetGatewayDevice.DeviceInfo.ProductClass type:string value:DESKTOP_LINUX
     [#6]name:InternetGatewayDevice.DeviceInfo.SerialNumber type:string value:001122334455
     [#7]name:InternetGatewayDevice.DeviceInfo.HardwareVersion type:string value:tmp_hardware1.0
     [#8]name:InternetGatewayDevice.DeviceInfo.SoftwareVersion type:string value:5.02L.03
     [#9]name:InternetGatewayDevice.DeviceInfo.ModemFirmwareVersion type:string value:
     [#10]name:InternetGatewayDevice.DeviceInfo.EnabledOptions type:string value:
     [#11]name:InternetGatewayDevice.DeviceInfo.AdditionalHardwareVersion type:string value:BoardId=DESKTOP_LINUX
     [#12]name:InternetGatewayDevice.DeviceInfo.AdditionalSoftwareVersion type:string value:CFE=1.0.37-9.14
     [#13]name:InternetGatewayDevice.DeviceInfo.SpecVersion type:string value:1.0
     [#14]name:InternetGatewayDevice.DeviceInfo.ProvisioningCode type:string value:
     [#15]name:InternetGatewayDevice.DeviceInfo.UpTime type:unsignedInt value:34070
     [#16]name:InternetGatewayDevice.DeviceInfo.FirstUseDate type:dateTime value:0001-01-01T00:00:00Z
     [#17]name:InternetGatewayDevice.DeviceInfo.X_BROADCOM_COM_NumberOfCpuThreads type:unsignedInt value:1
     [#18]name:InternetGatewayDevice.DeviceInfo.X_BROADCOM_COM_SwBuildTimestamp type:string value:170801_0259
     [#19]name:InternetGatewayDevice.DeviceInfo.DeviceLog type:string value:device log not supported on DESKTOP LINUX
     [#20]name:InternetGatewayDevice.DeviceInfo.VendorConfigFileNumberOfEntries type:unsignedInt value:0
     CWMPCTL - GetParameterValues is performed completely. It returns array of 20 parameters. Return code 0
```

  8. Set loglevel of cms applications via consoled:  
     To set the debugging level of a particular application,
     run the `loglevel` command as usual.  
     For example, if we want to set the loglevel of pmd to be "Debug", run below command in consoled:  
```bash
     >loglevel set pmd Debug
     pmd:debug:170.123:pmd_msg_handler:395:got set log destination to 1
     new log level set.
     >
```

##### Example output:
```bash
    jingqiu@Dune:~/workspace/depot/CommEngine/devel$ docker run --rm --privileged=true -it -v $PWD:/home/beepuser/devel desktop-beep

    to run a command as administrator (user "root"), use "sudo <command>".
    see "man sudo_root" for details.

    beepuser@2d3e0ca3c50f:~$ ls
    devel  com.broadcom.conf  simulate.sh
    beepuser@6ac8886a7bdd:~$ cd devel/
    beepuser@2d3e0ca3c50f:~/devel$ ../simulate.sh

    ===== Release Version 5.02L.04pre1 (build timestamp 170612_0824) =====

    initializing CMS MDM in Hybrid98+181 mode
    ssk:error:1.708:oalFil_removeDir:238:rmdir of /home/beepuser/devel/targets/963138GW_BEEP_DESKTOP/fs.install/local/modsw/tr157du/beep failed!
    ssk:error:1.714:rut_setIfState:1831:Cannot ioctl SIOCGIFFLAGS on the socket
    [INF vlanctl] vlanctl_dev_open: vlanctl_open(/dev/bcmvlan,0x2)

    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x74, 0x0xffd17ef7)
    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x64, 0x0xffd17ec0)
    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x76, 0x0xffd17ed8)
    ssk:error:1.714:rut_setIfState:1831:Cannot ioctl SIOCGIFFLAGS on the socket
    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x75, 0x0xffd17eb0)
    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x75, 0x0xffd17eb0)
    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x75, 0x0xffd17eb0)
    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x69, 0x0xffd17d40)
    created new Tag Rule: dev=eth1, dir=1, tags=0, id=-1

    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x69, 0x0xffd17d40)
    created new Tag Rule: dev=eth1, dir=1, tags=1, id=-1

    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x69, 0x0xffd17d40)
    created new Tag Rule: dev=eth1, dir=1, tags=2, id=-1

    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x69, 0x0xffd17d40)
    created new Tag Rule: dev=eth1, dir=0, tags=0, id=-1

    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x69, 0x0xffd17d40)
    created new Tag Rule: dev=eth1, dir=0, tags=1, id=-1

    [INF vlanctl] vlanctl_dev_ioctl: vlanctl_ioctl(0x0, 0x69, 0x0xffd17d40)
    created new Tag Rule: dev=eth1, dir=0, tags=2, id=-1

    [INF vlanctl] vlanctl_dev_close: vlanctl_close(0x0)
    ./smd:error:1.732:startApp:2526:invalid eid 550
    ssk:error:1.732:rcl_periodicStatObject:86:failed to start periodic statistic daemon.
    ssk:error:1.732:mdm_activateObjects:796:rcl handler reports error=9002 on PeriodicStatistics oid=920 iidStack={}
    beepuser@2d3e0ca3c50f:~/devel$ sh: /sys/fs/cgroup/memory/memory.use_hierarchy: Permission denied
    ./smd:error:1.761:validateAppLaunchedMsg:873:dInfo for pmd already has a commFd 6, ignore this one
    pmd:error:989.961:rutMulti_configIgmpSnooping:127:enable igmp snooping on br1 mode 2 l2l 0 failed, ret=-5
    pmd:error:989.961:qdmMulti_getSnoopingInfoLocked_igd:106:could not find brIfName br2
    pmd:error:989.964:rutMulti_configMldSnooping:111:enable mld snooping on br1 mode 2 l2l 0 failed, ret=-5
    pmd:error:989.964:qdmMulti_getSnoopingInfoLocked_igd:106:could not find brIfName br2
    pmd:error:989.972:rutMulti_configIgmpSnooping:127:enable igmp snooping on br2 mode 2 l2l 0 failed, ret=-5
    pmd:error:989.973:rutMulti_configMldSnooping:111:enable mld snooping on br2 mode 2 l2l 0 failed, ret=-5
    BCM9ffffffff Broadband Router

    Login: admin
    Password:
     > sh


    BusyBox v1.26.2 (2017-08-01 02:54:06 UTC) built-in shell (ash)
    Enter 'help' for a list of built-in commands.

    $
```

## Technique Details to be noted:
  1. As of today(2017-08-08), the cgroupv2 and cgroup namespace has not been mature enough. As a result, running lxc inside docker would fail
    due to the creation of cgroup directory using "/proc/self/cgroup" file. This file, under docker, contains below contents:
```bash
    beepuser@1d411e9e3505:~/bezeq$ cat /proc/self/cgroup
    11:memory:/docker/1d411e9e35056547950183c77d6c1e66e400ed8c0c22b6685aafc1949414022b
    10:devices:/docker/1d411e9e35056547950183c77d6c1e66e400ed8c0c22b6685aafc1949414022b
    9:hugetlb:/docker/1d411e9e35056547950183c77d6c1e66e400ed8c0c22b6685aafc1949414022b
    8:freezer:/docker/1d411e9e35056547950183c77d6c1e66e400ed8c0c22b6685aafc1949414022b
    7:net_cls,net_prio:/docker/1d411e9e35056547950183c77d6c1e66e400ed8c0c22b6685aafc1949414022b
    6:blkio:/docker/1d411e9e35056547950183c77d6c1e66e400ed8c0c22b6685aafc1949414022b
    5:cpuset:/docker/1d411e9e35056547950183c77d6c1e66e400ed8c0c22b6685aafc1949414022b
    4:perf_event:/docker/1d411e9e35056547950183c77d6c1e66e400ed8c0c22b6685aafc1949414022b
    3:pids:/docker/1d411e9e35056547950183c77d6c1e66e400ed8c0c22b6685aafc1949414022b
    2:cpu,cpuacct:/docker/1d411e9e35056547950183c77d6c1e66e400ed8c0c22b6685aafc1949414022b
    1:name=systemd:/docker/1d411e9e35056547950183c77d6c1e66e400ed8c0c22b6685aafc1949414022b
```

    Please note the preceding "/docker/container_id" in each of the cgroup controller path.
    However, under "/sys/fs/cgroup/", each controller doesn't have above preceding path:
```bash
    beepuser@1d411e9e3505:~/bezeq$ ls /sys/fs/cgroup/memory/
    cgroup.clone_children               memory.kmem.usage_in_bytes
    cgroup.event_control                memory.limit_in_bytes
    cgroup.procs                        memory.max_usage_in_bytes
    lxc                                 memory.move_charge_at_immigrate
    memory.failcnt                      memory.numa_stat
    memory.force_empty                  memory.oom_control
    memory.kmem.failcnt                 memory.pressure_level
    memory.kmem.limit_in_bytes          memory.soft_limit_in_bytes
    memory.kmem.max_usage_in_bytes      memory.stat
    memory.kmem.slabinfo                memory.swappiness
    memory.kmem.tcp.failcnt             memory.usage_in_bytes
    memory.kmem.tcp.limit_in_bytes      memory.use_hierarchy
    memory.kmem.tcp.max_usage_in_bytes  notify_on_release
    memory.kmem.tcp.usage_in_bytes      tasks
```

    This differences would confuse lxc and cause it to exit. In theory, we shouldn't see the 
    preceding path in cgroup controllers inside docker, as the path should only be visible to the 
    host. This problem will be addressed in cgroupv2 and cgroup namespace.
    For now, we fix the issue by telling lxc to use a marco as the preceding path instead of reading 
    from "/proc/self/cgroup".

  2. Creating user namespace inside fs.simulate
    We pivot_root into fs.simulate. This is required by kernel to create a new user namespace. See:
    https://github.com/torvalds/linux/commit/3151527ee007b73a0ebd296010f1c0454a919c7d

  3. The magic for "pivot_root"  
    "pivot_root" would require the source old and new root filesystem to be a mount point.
    The pivot_root system call is used both in lxc and in preparing fs.simulate.
    As a result, we bind mount fs.simulate to fs.install. See the comment in kernel's pivot_root code:

```
    /*
     * pivot_root Semantics:
     * Moves the root file system of the current process to the directory put_old,
     * makes new_root as the new root file system of the current process, and sets
     * root/cwd of all processes which had them on the current root to new_root.
     *
     * Restrictions:
     * The new_root and put_old must be directories, and  must not be on the
     * same file  system as the current process root. The put_old  must  be
     * underneath new_root,  i.e. adding a non-zero number of /.. to the string
     * pointed to by put_old must yield the same directory as new_root. No other
     * file system may be mounted on put_old. After all, new_root is a mountpoint.
     *
     * Also, the current root cannot be on the 'rootfs' (initial ramfs) filesystem.
     * See Documentation/filesystems/ramfs-rootfs-initramfs.txt for alternatives
     * in this situation.
     *
     * Notes:
     *  - we don't move root/cwd if they are not at the root (reason: if something
     *    cared enough to change them, it's probably wrong to force them elsewhere)
     *  - it's okay to pick a root that isn't the root of a file system, e.g.
     *    /nfs/my_root where /nfs is the mount point. It must be a mountpoint,
     *    though, so you may need to say mount --bind /nfs/my_root /nfs/my_root
     *    first.
     */
    SYSCALL_DEFINE2(pivot_root, const char __user *, new_root,
    ›   ›   const char __user *, put_old)
    {
      ...
    }
```

