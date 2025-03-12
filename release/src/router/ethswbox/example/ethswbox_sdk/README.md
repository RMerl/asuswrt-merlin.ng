# MxL862xx ethswbox_sdk V1.1.0.0


Copyright 2024 MaxLinear, Inc.

For licensing information, see the file 'LICENSE' in the root folder of this software module.


**MxL strongly recommends to customers to use secure version of string functions/libraries to avoid unexpected access to memory without permission. This is in customer responsibility to ensure this.**


Getting started
---------------

The "ethswbox_sdk" folder contains the following three scripts:

- example/ethswbox_sdk/ethswbox-prepare.sh:   Prepare to create softlink of switch_api and CMakefile before compiling the tool.
                                              The script must be run with "user" or "rpi4evk" option.
- example/ethswbox_sdk/ethswbox-set-build.sh: Build the ethswbox tool.
- example/ethswbox_sdk/ethswbox-cleanup.sh:   Clean build the tool.

### Build options

    ./ethswbox-prepare.sh user       ; To build tool including MxL862xx host API. MDIO function is empty.
                                       The option is used for customization.
    ./ethswbox-prepare.sh rpi4evk    ; To build tool including MxL862xx host API, pin-scanning, and the pyrpio MDIO library. 
                                       The option is used for running the tool on Raspberry Pi 4.


How to run the tool on Raspberry Pi 4
-------------------------------------

### 0/ Prerequisite: Cmake, Python

    sudo apt update
    sudo apt install snapd
    sudo snap install cmake --classic
    sudo apt-get install cython3
    sudo reboot -f


### 1/ On RPI4 HOST - clone the repository, prepare, set build and python

    mkdir mydir -p
    cd mydir
    git clone https://git.maxlinear.com/mxl/ethernet/2.5g/managed_attach_mxl862xx
    cd managed_attach_mxl862xx/example/ethswbox_sdk/

    . ethswbox-prepare.sh rpi4evk
    or
    . ethswbox-prepare.sh user
    
    . ethswbox-set-build.sh

Expected structure after ". ethswbox-prepare.sh rpi4evk":

```
 ... /managed_attach_mxl862xx/example $ tree
.
├── ethswbox_sdk
│   ├── ChangeLog.txt
│   ├── CMakeLists.txt -> CMakeLists.txt.rpi4evk
│   ├── CMakeLists.txt.rpi4evk
│   ├── CMakeLists.txt.user
│   ├── ethswbox-cleanup.sh
│   ├── ethswbox.code-workspace
│   ├── ethswbox-kw.sh
│   ├── ethswbox-prepare.sh
│   ├── ethswbox-set-build.sh
│   ├── LICENSE
│   ├── packages
│   │   ├── PyRPIO -> PyRPIO-0.4.1/
│   │   ├── PyRPIO-0.4.1
│   │   │   ├── LICENSE.md
│   │   │   ├── PKG-INFO
│   │   │   ├── pyproject.toml
│   │   │   ├── pyrpio
│   │   │   │   ├── gpio.py
│   │   │   │   ├── i2c
│   │   │   │   │   ├── ftdii2c.py
│   │   │   │   │   ├── httpi2c.py
│   │   │   │   │   ├── i2c.py
│   │   │   │   │   ├── __init__.py
│   │   │   │   │   └── types.py
│   │   │   │   ├── i2c_register_device.py
│   │   │   │   ├── __init__.py
│   │   │   │   ├── lib
│   │   │   │   │   ├── bcm2835.c
│   │   │   │   │   ├── bcm2835_ext.h
│   │   │   │   │   ├── bcm2835.h
│   │   │   │   │   ├── mdio.c
│   │   │   │   │   ├── mdio_ext.h
│   │   │   │   │   ├── mdio.h
│   │   │   │   │   └── module.c
│   │   │   │   ├── mdio.py
│   │   │   │   ├── pinmap.py
│   │   │   │   ├── pwm.py
│   │   │   │   ├── spi
│   │   │   │   │   ├── __init__.py
│   │   │   │   │   ├── spi.py
│   │   │   │   │   └── types.py
│   │   │   │   └── types.py
│   │   │   ├── README.md
│   │   │   └── setup.py
│   │   ├── PyRPIO-0.4.1.tar.gz
│   │   └── switch_hostapi -> ../../../switch_hostapi/
│   ├── python
│   │   └── cbindings
│   │       └── cython
│   │           ├── compile.py
│   │           └── lif.pyx
│   ├── README.txt
│   └── src
│       ├── apps
│       │   └── download
│       │       ├── smdio_access.c
│       │       ├── smdio_access.h
│       │       ├── smdio_ssb.c
│       │       └── smdio_ssb.h
│       ├── cli
│       │   ├── cmds
│       │   │   ├── cmds_apps_ssb.c
│       │   │   ├── cmds_apps_ssb.h
│       │   │   ├── cmds.c
│       │   │   ├── cmds_fapi.c
│       │   │   ├── cmds_fapi.h
│       │   │   └── cmds.h
│       │   ├── ethswbox.c
│       │   └── ethswbox.h
│       ├── ethswbox_version.h
│       ├── fapi
│       │   ├── fapi_gsw_hostapi.c
│       │   ├── fapi_gsw_hostapi.h
│       │   ├── fapi_gsw_hostapi_mdio_relay.c
│       │   └── fapi_gsw_hostapi_mdio_relay.h
│       ├── lif
│       │   ├── lif_api.c
│       │   ├── lif_api.h
│       │   └── mdio -> ../../packages/PyRPIO/pyrpio
│       └── os
│           ├── os_linux.c
│           ├── os_linux.h
│           └── os_types.h
└── LICENSE

21 directories, 64 files
```


### 2/ On RPI4 HOST - go to build directory (ethswbox/build)

Available commands:
```
fapi-ext-mdio-mod
fapi-ext-mdio-read
fapi-ext-mdio-write
fapi-GSW-BridgeAlloc
fapi-GSW-BridgeConfigGet
fapi-GSW-BridgeConfigSet
fapi-GSW-BridgeFree
fapi-GSW-BridgePortAlloc
fapi-GSW-BridgePortConfigGet
fapi-GSW-BridgePortConfigSet
fapi-GSW-BridgePortFree
fapi-GSW-BridgePortLoopGet
fapi-GSW-BridgePortLoopRead
fapi-GSW-CfgGet
fapi-GSW-CfgSet
fapi-GSW-Cml-Clk-Get
fapi-GSW-Cml-Clk-Set
fapi-GSW-CPU-PortCfgGet
fapi-GSW-CPU-PortCfgSet
fapi-GSW-CPU-PortGet
fapi-GSW-CPU-PortSet
fapi-GSW-CTP-PortAssignmentGet
fapi-GSW-CTP-PortAssignmentSet
fapi-GSW-CtpPortConfigGet
fapi-GSW-CtpPortConfigSet
fapi-GSW-Debug-PMAC-RMON-Get-All
fapi-GSW-Debug-RMON-Port-Get
fapi-GSW-Debug-RMON-Port-GetAll
fapi-GSW-Delay
fapi-GSW-ExtendedVlanAlloc
fapi-GSW-ExtendedVlanFree
fapi-GSW-ExtendedVlanGet
fapi-GSW-ExtendedVlanSet
fapi-GSW-Freeze
fapi-GSW-FW-Update
fapi-GSW-FW-Version
fapi-GSW-GPIO-Configure
fapi-GSW-MAC-DefaultFilterGet
fapi-GSW-MAC-DefaultFilterSet
fapi-GSW-MAC-TableClear
fapi-GSW-MAC-TableClear-Cond
fapi-GSW-MAC-TableEntryAdd
fapi-GSW-MAC-TableEntryQuery
fapi-GSW-MAC-TableEntryRead
fapi-GSW-MAC-TableEntryRemove
fapi-GSW-Mac-TableLoopDetect
fapi-GSW-MonitorPortCfgGet
fapi-GSW-MonitorPortCfgSet
fapi-GSW-MulticastRouterPortAdd
fapi-GSW-MulticastRouterPortRead
fapi-GSW-MulticastRouterPortRemove
fapi-GSW-MulticastSnoopCfgGet
fapi-GSW-MulticastSnoopCfgSet
fapi-GSW-MulticastTableEntryAdd
fapi-GSW-MulticastTableEntryRead
fapi-GSW-MulticastTableEntryRemove
fapi-GSW-PBB-TunnelTempate-Alloc
fapi-GSW-PBB-TunnelTempate-Config-Get
fapi-GSW-PBB-TunnelTempate-Config-Set
fapi-GSW-PBB-TunnelTempate-Free
fapi-GSW-PceRuleAlloc
fapi-GSW-Pce-RuleBlockSize
fapi-GSW-PceRuleDelete
fapi-GSW-PceRuleDisable
fapi-GSW-PceRuleEnable
fapi-GSW-PceRuleFree
fapi-GSW-PceRuleMove
fapi-GSW-PceRuleRead
fapi-GSW-PceRuleWrite
fapi-GSW-PMAC-BM-CfgGet
fapi-GSW-PMAC-BM-CfgSet
fapi-GSW-PMAC-EG-CfgGet
fapi-GSW-PMAC-EG-CfgSet
fapi-GSW-PMAC-GLBL-CfgGet
fapi-GSW-PMAC-GLBL-CfgSet
fapi-GSW-PMAC-IG-CfgGet
fapi-GSW-PMAC-IG-CfgSet
fapi-GSW-PMAC-RMON-Get
fapi-GSW-PortLinkCfgGet
fapi-GSW-PortLinkCfgSet
fapi-GSW-PVT-Meas
fapi-GSW-QoS-ColorMarkingTableGet
fapi-GSW-QoS-ColorMarkingTableSet
fapi-GSW-QoS-ColorReMarkingTableGet
fapi-GSW-QoS-ColorReMarkingTableSet
fapi-GSW-QoS-DSCP2-PCPTableGet
fapi-GSW-QoS-DSCP2-PCPTableSet
fapi-GSW-QoS-DSCP-ClassGet
fapi-GSW-QoS-DSCP-ClassSet
fapi-GSW-QoS-DSCP-DropPrecedenceCfgGet
fapi-GSW-QoS-DSCP-DropPrecedenceCfgSet
fapi-GSW-QoS-FlowctrlCfgGet
fapi-GSW-QoS-FlowctrlCfgSet
fapi-GSW-QoS-FlowctrlPortCfgGet
fapi-GSW-QoS-FlowctrlPortCfgSet
fapi-GSW-QoS-MeterAlloc
fapi-GSW-QoS-MeterCfgGet
fapi-GSW-QoS-MeterCfgSet
fapi-GSW-QoS-MeterFree
fapi-GSW-QoS-PCP-ClassGet
fapi-GSW-QoS-PCP-ClassSet
fapi-GSW-QoS-PmapperTableGet
fapi-GSW-QoS-PmapperTableSet
fapi-GSW-QoS-PortCfgGet
fapi-GSW-QoS-PortCfgSet
fapi-GSW-QoS-PortReMarkingCfgGet
fapi-GSW-QoS-PortReMarkingCfgSet
fapi-GSW-QoS-QueueCfgGet
fapi-GSW-QoS-QueueCfgSet
fapi-GSW-QoS-QueuePortGet
fapi-GSW-QoS-QueuePortSet
fapi-GSW-QoS-SchedulerCfgGet
fapi-GSW-QoS-SchedulerCfgSet
fapi-GSW-QoS-ShaperCfgGet
fapi-GSW-QoS-ShaperCfgSet
fapi-GSW-QoS-ShaperQueueAssign
fapi-GSW-QoS-ShaperQueueDeassign
fapi-GSW-QoS-ShaperQueueGet
fapi-GSW-QoS-StormCfgGet
fapi-GSW-QoS-StormCfgSet
fapi-GSW-QoS-SVLAN-PCP-ClassGet
fapi-GSW-QoS-SVLAN-PCP-ClassSet
fapi-GSW-QoS-WredCfgGet
fapi-GSW-QoS-WredCfgSet
fapi-GSW-QoS-WredPortCfgGet
fapi-GSW-QoS-WredPortCfgSet
fapi-GSW-QoS-WredQueueCfgGet
fapi-GSW-QoS-WredQueueCfgSet
fapi-GSW-Reboot
fapi-GSW-RegisterGet
fapi-GSW-RegisterMod
fapi-GSW-RegisterSet
fapi-GSW-RMON-Clear
fapi-GSW-RMON-FlowGet
fapi-GSW-RMON-MeterGet
fapi-GSW-RMON-ModeSet
fapi-GSW-RMON-PortGet
fapi-GSW-RMON-TFlowClear
fapi-GSW-Sfp-Get
fapi-GSW-Sfp-Set
fapi-GSW-SS-Sptag-Get
fapi-GSW-SS-Sptag-Set
fapi-GSW-STP-BPDU-RuleGet
fapi-GSW-STP-BPDU-RuleSet
fapi-GSW-STP-PortCfgGet
fapi-GSW-STP-PortCfgSet
fapi-GSW-SysReg-Mod
fapi-GSW-SysReg-Rd
fapi-GSW-SysReg-Wr
fapi-GSW-TflowCountModeGet
fapi-GSW-TflowCountModeSet
fapi-GSW-TrunkingCfgGet
fapi-GSW-TrunkingCfgSet
fapi-GSW-UnFreeze
fapi-GSW-VlanCounterMapGet
fapi-GSW-VlanCounterMapSet
fapi-GSW-VlanFilterAlloc
fapi-GSW-VlanFilterFree
fapi-GSW-VlanFilterGet
fapi-GSW-VlanFilterSet
fapi-GSW-Vlan-RMON-Clear
fapi-GSW-Vlan-RMONControl-Get
fapi-GSW-Vlan-RMONControl-Set
fapi-GSW-Vlan-RMON-Get
fapi-int-gphy-mod
fapi-int-gphy-read
fapi-int-gphy-write
ssb_smdio_download
```

### 3/ On MxL86282C-EVK: Flash boot mode, FW loaded from flash by default

### 4/ Connect RPI4 HOST to e.g. MxL86282C-EVK thru MDIO pins (clock, data, ground)


MxL86282C-EVK : J61

```
        1  2   3   4
J61  +---+-c-+-d-+-g-+

pin 2: MDIO clock
pin 3: MDIO data
pin 4: ground
```

RPI4 pins Header:

```
+-02-+-04-+-06-+-08-+-10-+-12-+-14-+-16-+-18-+-20-+-22-+-24-+-26-+-28-+-30-+-32-+-34-+-36-+-38-+-40-+
+-01-+-03-+-05-+-07-+-09-+-11-+-13-+-15-+-17-+-19-+-21-+-23-+-25-+-27-+-29-+-31-+-33-+-35-+-37-+-39-+
                                                                        c    d                   g

pin 29 (GPIO 5): MDIO clock
pin 31 (GPIO 6): MDIO data
pin 39 (Ground): ground
```


On RPI4 HOST to Upgrade MxL862xxC firmware via SMDIO 
----------------------------------------------------

From build directory (ethswbox/build) to check the MDIO access to the board by reading FW version.

```
pi@raspberrypi:~/mydir/managed_attach_mxl862xx/example/ethswbox_sdk/build $ ./fapi-GSW-FW-Version
cmd: fapi-GSW-FW-Version 0: 
	                                   Major:	1
	                                   Minor:	0
	                                Revision:	45
	                            APP Revision:	45
```

Enable FW dowload mode and execute flash update.

```
pi@raspberrypi:~/mydir/managed_attach_mxl862xx/example/ethswbox_sdk/build $ ./fapi-GSW-FW-Update
cmd: fapi-GSW-FW-Update 0: 

pi@raspberrypi:~/mydir/managed_attach_mxl862xx/example/ethswbox_sdk/build $ ./ssb_smdio_download  ../../../firmware/mxl862xxc_1026_1054_1054_003E_signed_upgrade_xfi.bin 
cmd: ssb_smdio_download 1: ../../../firmware/mxl862xxc_1026_1054_1054_003E_signed_upgrade_xfi.bin 
FW size: 1421332 bytes
Target is ready for downloading.
Received START ACK from target, starting...
image type: f48af48a, size 1: 15b000, checksum 1: f80a0878, size 2: 0, checksum 2: 0
Received ACK of image header from target
Erase flash
Program flash
..........................................
successfully download firmware to target
FW Upload Sucessful
```

Double-check the the MDIO access to the board by reading preloaded FW version.

```
pi@raspberrypi:~/mydir/managed_attach_mxl862xx/example/ethswbox_sdk/build $ ./fapi-GSW-FW-Version
cmd: fapi-GSW-FW-Version 0: 
                                           Major:       1
                                           Minor:       0
                                        Revision:       54
                                    APP Revision:       54
```
