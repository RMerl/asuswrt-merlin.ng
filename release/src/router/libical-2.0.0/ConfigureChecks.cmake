include(CheckIncludeFiles)
check_include_files(byteswap.h HAVE_BYTESWAP_H)
check_include_files(dirent.h HAVE_DIRENT_H)
check_include_files(endian.h HAVE_ENDIAN_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(pthread.h HAVE_PTHREAD_H)
check_include_files(sys/endian.h HAVE_SYS_ENDIAN_H)
check_include_files(sys/param.h HAVE_SYS_PARAM_H)
check_include_files(sys/utsname.h HAVE_SYS_UTSNAME_H)
check_include_files(fcntl.h HAVE_FCNTL_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(wctype.h HAVE_WCTYPE_H)

include(CheckFunctionExists)
if(WIN32 AND MSVC)
  check_function_exists(_access HAVE__ACCESS) #Windows <io.h>
  check_function_exists(_getpid HAVE__GETPID) #Windows <process.h>
  check_function_exists(_mkdir HAVE__MKDIR) #Windows <direct.h>
  check_function_exists(_open HAVE__OPEN) #Windows <io.h>
  check_function_exists(_snprintf HAVE__SNPRINTF) #Windows <stdio.h>
  check_function_exists(_stat HAVE__STAT) #Windows <sys/types.h>,<sys/stat.h>
  check_function_exists(_strdup HAVE__STRDUP) #Windows <string.h>
  check_function_exists(_stricmp HAVE__STRICMP) #Windows <string.h>
  check_function_exists(_strnicmp HAVE__STRNICMP) #Windows <string.h>
  check_function_exists(_read HAVE__READ) #Windows <io.h>
  check_function_exists(_write HAVE__WRITE) #Windows <io.h>
else()
  check_function_exists(access HAVE_ACCESS) #Unix <unistd.h>
  check_function_exists(fork HAVE_FORK) #Unix <unistd.h>
  check_function_exists(getopt HAVE_GETOPT) #Unix <unistd.h>
  check_function_exists(getpid HAVE_GETPID) #Unix <unistd.h>
  check_function_exists(getpwent HAVE_GETPWENT) #Unix <sys/types.h>,<pwd.h>
  check_function_exists(gmtime_r HAVE_GMTIME_R) #Unix <time.h>
  check_function_exists(localtime_r HAVE_LOCALTIME_R) #Unix <time.h>
  check_function_exists(mkdir HAVE_MKDIR) #Unix <sys/stat.h>,<sys/types.h>
  check_function_exists(open HAVE_OPEN) #Unix <sys/stat.h>,<sys/types.h>,<fcntl.h>
  check_function_exists(nanosleep HAVE_NANOSLEEP) #Unix <time.h>
  check_function_exists(signal HAVE_SIGNAL) #Unix <signal.h>
  check_function_exists(snprintf HAVE_SNPRINTF) #Unix <stdio.h>
  check_function_exists(stat HAVE_STAT) #Unix <sys/stat.h>,<sys/types.h>,<unistd.h>
  check_function_exists(strdup HAVE_STRDUP) #Unix <string.h>
  check_function_exists(strcasecmp HAVE_STRCASECMP) #Unix <strings.h>
  check_function_exists(strncasecmp HAVE_STRNCASECMP) #Unix <strings.h>
  check_function_exists(read HAVE_READ) #Unix <unistd.h>
  check_function_exists(unlink HAVE_UNLINK) #Unix <unistd.h>
  check_function_exists(usleep HAVE_USLEEP) #Unix <unistd.h>
  check_function_exists(waitpid HAVE_WAITPID) #Unix <sys/types.h>,<sys/wait.h>
  check_function_exists(write HAVE_WRITE) #Unix <unistd.h>
  if(NOT MINGW)
    check_function_exists(alarm HAVE_ALARM) #Unix <unistd.h>
  endif()
endif()

check_function_exists(backtrace HAVE_BACKTRACE)
check_function_exists(iswspace HAVE_ISWSPACE) #Linux <wctype.h>
check_function_exists(setenv HAVE_SETENV)
check_function_exists(unsetenv HAVE_UNSETENV)

set(_SAVE_RQL ${CMAKE_REQUIRED_LIBRARIES})
set(CMAKE_REQUIRED_LIBRARIES kernel32.lib)
check_function_exists(GetNumberFormat HAVE_GETNUMBERFORMAT) #Windows <windows.h>
set(CMAKE_REQUIRED_LIBRARIES ${_SAVE_RQL})

include(CheckTypeSize)
check_type_size(intptr_t SIZEOF_INTPTR_T)
check_type_size(pid_t SIZEOF_PID_T)
check_type_size(size_t SIZEOF_SIZE_T)
check_type_size(ssize_t SIZEOF_SSIZE_T)
check_type_size(time_t SIZEOF_TIME_T)
check_type_size(wint_t SIZEOF_WINT_T)

include(FindThreads)
check_library_exists(pthread pthread_attr_get_np "" HAVE_PTHREAD_ATTR_GET_NP)
check_library_exists(pthread pthread_getattr_np "" HAVE_PTHREAD_GETATTR_NP)
check_library_exists(pthread pthread_create "" HAVE_PTHREAD_CREATE)
check_include_files("pthread.h;pthread_np.h" HAVE_PTHREAD_NP_H)
