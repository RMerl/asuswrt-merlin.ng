******************** RPCAP AND CODE MODIFICATIONS ********************

These files are the ones which have to be modified when compiling a new release of WinPcap.

All the modifications are included within:

  #ifdef REMOTE
    ....
  #endif

therefore it should be quite easy to locate which point you have to modify.

******************** COMPILE LIBPCAP WITH RPCAP SUPPORT UNDER UNIX ********************

First, you must unpack all text files according to the UNIX standard. For this, you may use
"unzip -a" (it extracts the files using the platform-native format for text files) or the
dos2unix conversion utility.

Second, you have to type '/.configure' in order to create the makefile.
In case this step fails, you should re-create the configure by launching 'autoconf'
(version 2.50 or higher) in the libpcap folder.

Finally, you have to compile the project by typing 'make'.
In case some error occurs, let's try to delete all the config.* files from the 'libpcap'
folder and restart this process from scratch.

These steps are able to compile the libpcap library; now you have to compile the rpcapd daemon.
For this, you have to type 'make' in the 'rpcapd' folder.
