bp3 is a tool which directly provisions a Broadcom chip by securely enabling/disabling IP features on the Broadcom SOC.

Source Location:
    BSEAV/tools/bp3

Source dependency location:
    BSEAV/opensource/openssl
    BSEAV/opensource/zlib
    BSEAV/opensource/curl
    BSEAV/opensource/cjson
    BSEAV/opensource/civetweb

1. TO BUILD BP3 HOST APP
  BP3 host is a nexus app. There are four modes that nexus and nexus apps run: kernel or user mode, and with or without nxclient support for apps.

  These modes can be configured via environment variables:

  a). kernel mode, nxclient support:
    export NEXUS_MODE=proxy
    export NXCLIENT_SUPPORT=y

  b). kernel mode, no nxclient support:
    export NEXUS_MODE=proxy
    unset NXCLIENT_SUPPORT

  c). user mode, nxclient support:
    unset NEXUS_MODE
    export NXCLIENT_SUPPORT=y

  d). user mode, no nxclient support:
    unset NEXUS_MODE
    unset NXCLIENT_SUPPORT

  All modes require BP3 to be enabled:
  export BP3_PROVISIONING=y

  For example, to build bp3 host with kernel mode, nxclient support, export the following variables:
    export NEXUS_MODE=proxy
    export NXCLIENT_SUPPORT=y
    export BP3_PROVISIONING=y

  To start the build (using 7268 b0 as an example):
    cd <refsw folder>
    plat 97268 b0
    cd BSEAV/tools/bp3
    make -j install

  Executables is generated in <refsw folder>/$(B_REFSW_OBJ_DIR)/nexus/bin, where B_REFSW_OBJ_DIR is an environment variable created by plat script.

2. TO RUN BP3 APP
  Because there are four build configurations, the steps to run bp3 host app are slightly different:

  cd nexus/bin
  # if nxclient support
  ./nexus nxserver &
  ./nexus.client bp3
  # if no nxclient support
  ./nexus bp3

  The above command should print bp3 usage. The following examples assume nxclient support is enabled

  1) To provision licenses 3 and 4 from broadcom bp3 portal site, on the set top box, type:
        STB> ./nexus.client bp3 provision -portal https://bp3.broadcom.com -key e906224b7cca497da11c1433c7157d4f -license 3,4

     You should see sample following messages on the console:
            00:00:00.035 bp3_curl: Server: https://bp3.broadcom.com
            ...

  2) To check what features has been provisioned on the set top box, type:
       STB> ./nexus.client bp3 status

  3) To run bp3 in host service mode on the set top box, type:
       STB> ./nexus.client bp3 service

     You should see something like this in the console, among other nexus messages:
     SSDP listening on <your boards ip>:80

     Now you should be able to search and provision the STB from Provision Server remotely. The default port # can be modified in bp3_host.c, #define PORT 80.
     If you want to use HTTPS, get a valid SSL certificate (self-signed will not work), create server.pem civetweb's documentation, and use port 443 or specify -S option.