=============================================================================
How "makefile.wlan" works for the release process
=============================================================================

This file is included from each "implx/Makefile", which in term is called
from "kernel/linux/Makefile" to build the wireless driver.

To make a release, we usually run "release/dorel963xx".  That Bash script
builds the wireless driver and applications for each profile.  In order to
preserve binary files for each profile while the next profile is being built,
there are three targets to help in each "implx/Makefile": create_list,
save_binary, and release.  These targets are usually called only from
"dorel963xx".

If we need to make a wireless-only release, the steps are:

(1) Caution: The release procedure removes non-public source code.  If we
    need to preserve working files, make a copy of it, and build from the
    copy.

(2) Build source file tar ball.  Go to: "bcmdrivers/broadcom/net/wl", and
    execute:

    "build_src_tar_files.sh x",

    where the "x" is an integer indicating which impl needs to be built.

(3) Create a list of all source files.  Go to the correct impl, for example
    impl3, and execute:
 
    "make create_list".

    This will create a file "files_all" that lists all source files.  Later
    on in the release process step (6), "files_all" will be compared with 
    "files_to_keep", and non-public files will be deleted.

(4) Make the whole image, at the top of the source tree, for example, for
    96368GW profile:
    
    "make PROFILE=96368GW".

(5) Once the build finishes, we'll preserve binary files.  Go to 
    "wl/bcm96368" (this needs to be the symbolic link one, not the impl), and
    execute:

    "make save_binary".

(6) The next step is to remove non-public source files.  Go to "wl/bcm96368",
    and execute:

    "make release".

(7) The final step is to make the release binary tar file.  Simply go to the
    "wl" directory and execute:

    "build_bin_tar_files.sh x",

    where the "x" is an integer indicating which impl needs to be built.
