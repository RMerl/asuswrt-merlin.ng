dnl #
dnl # check if were compiling with CLANG, autoconf GCC macro identifies CLANG as GCC
dnl #
AC_DEFUN([AX_CC_IS_CLANG],[
  AC_CACHE_CHECK([if compiler is clang], [ax_cv_cc_clang],[

  AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([], [[
    #ifndef __clang__
         not clang
    #endif
    ]])],
    [ax_cv_cc_clang=yes],
    [ax_cv_cc_clang=no])
  ])
])

AC_DEFUN([AX_CC_QUNUSED_ARGUMENTS_FLAG],[
  AC_CACHE_CHECK([for the compiler flag "-Qunused-arguments"], [ax_cv_cc_qunused_arguments_flag],[

    CFLAGS_SAVED=$CFLAGS
    CFLAGS="$CFLAGS -Werror -Qunused-arguments -foobar"
    
    AC_LANG_PUSH(C)
    AC_TRY_COMPILE(
      [],
      [return 0;],
      [ax_cv_cc_qunused_arguments_flag="yes"],
      [ax_cv_cc_qunused_arguments_flag="no"])
    AC_LANG_POP

    CFLAGS="$CFLAGS_SAVED"
  ])
])

AC_DEFUN([AX_CC_WDOCUMENTATION_FLAG],[
  AC_CACHE_CHECK([for the compiler flag "-Wdocumentation"], [ax_cv_cc_wdocumentation_flag],[

    CFLAGS_SAVED=$CFLAGS
    CFLAGS="$CFLAGS -Werror -Wdocumentation"

    AC_LANG_PUSH(C)
    AC_TRY_COMPILE(
      [],
      [return 0;],
      [ax_cv_cc_wdocumentation_flag="yes"],
      [ax_cv_cc_wdocumentation_flag="no"])
    AC_LANG_POP

    CFLAGS="$CFLAGS_SAVED"
  ])
])

dnl #
dnl # Determine the number of system cores we have
dnl #
AC_DEFUN([AX_SYSTEM_CORES],[
  AC_CACHE_CHECK([number of system cores], [ax_cv_system_cores],
    [
      AC_LANG_PUSH(C)
      AC_TRY_RUN(
        [
          #include <stdio.h>
          #include <stdint.h>
          #ifdef _WIN32
          #  include <windows.h>
          #elif MACOS
          #  include <sys/param.h>
          #  include <sys/sysctl.h>
          #else
          #  include <unistd.h>
          #endif
          
          int main (int argc, char *argv[])
          {
            uint32_t count;
            
            #ifdef WIN32
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
      
            count = sysinfo.dwNumberOfProcessors;
            
            #elif MACOS
            int nm[2];
            size_t len = 4;
      
            nm[0] = CTL_HW;
            nm[1] = HW_AVAILCPU;
            sysctl(nm, 2, &count, &len, NULL, 0);
      
            if(count < 1) {
              nm[1] = HW_NCPU;
              sysctl(nm, 2, &count, &len, NULL, 0);
              if(count < 1) {
                count = 1;
              }
            }
            
            #else
      	    count = sysconf(_SC_NPROCESSORS_ONLN);
            #endif

            return count;
          }
        ],
        [ax_cv_system_cores=$?],
        [ax_cv_system_cores=$?],
        [ax_cv_system_cores=]
    )
    AC_LANG_POP
  ])
])

