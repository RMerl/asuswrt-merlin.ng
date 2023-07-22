project('glib', 'c', 'cpp',
  version : '2.59.0',
  meson_version : '>= 0.48.0',
  default_options : [
    'buildtype=debugoptimized',
    'warning_level=1',
    'c_std=gnu89'
  ]
)

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

cc_can_run = not meson.is_cross_build() or meson.has_exe_wrapper()

if cc.get_id() == 'msvc'
  # Ignore several spurious warnings for things glib does very commonly
  # If a warning is completely useless and spammy, use '/wdXXXX' to suppress it
  # If a warning is harmless but hard to fix, use '/woXXXX' so it's shown once
  # NOTE: Only add warnings here if you are sure they're spurious
  add_project_arguments('/wd4035', '/wd4715', '/wd4116',
    '/wd4046', '/wd4068', '/wo4090', '/FImsvc_recommended_pragmas.h',language : 'c')
  # Disable SAFESEH with MSVC for plugins and libs that use external deps that
  # are built with MinGW
  noseh_link_args = ['/SAFESEH:NO']
  # Set the input and exec encoding to utf-8, like is the default with GCC
  add_project_arguments(cc.get_supported_arguments(['/utf-8']), language: 'c')
else
  noseh_link_args = []
  # -mms-bitfields vs -fnative-struct ?
endif

host_system = host_machine.system()

glib_version = meson.project_version()
glib_api_version = '2.0'
version_arr = glib_version.split('.')
major_version = version_arr[0].to_int()
minor_version = version_arr[1].to_int()
micro_version = version_arr[2].to_int()

interface_age = minor_version.is_odd() ? 0 : micro_version
binary_age = 100 * minor_version + micro_version

soversion = 0
# Maintain compatibility with previous libtool versioning
# current = minor * 100 + micro
current = binary_age - interface_age
library_version = '@0@.@1@.@2@'.format(soversion, current, interface_age)
darwin_versions = [current + 1, '@0@.@1@'.format(current + 1, interface_age)]

configinc = include_directories('.')
glibinc = include_directories('glib')
gobjectinc = include_directories('gobject')
gmoduleinc = include_directories('gmodule')
gioinc = include_directories('gio')

glib_prefix = get_option('prefix')
glib_bindir = join_paths(glib_prefix, get_option('bindir'))
glib_libdir = join_paths(glib_prefix, get_option('libdir'))
glib_libexecdir = join_paths(glib_prefix, get_option('libexecdir'))
glib_datadir = join_paths(glib_prefix, get_option('datadir'))
glib_pkgdatadir = join_paths(glib_datadir, 'glib-2.0')
glib_includedir = join_paths(glib_prefix, get_option('includedir'))
glib_giomodulesdir = get_option('gio_module_dir')
if glib_giomodulesdir == ''
  glib_giomodulesdir = join_paths(glib_libdir, 'gio', 'modules')
endif

glib_pkgconfigreldir = join_paths(glib_libdir, 'pkgconfig')

installed_tests_metadir = join_paths(glib_datadir, 'installed-tests', meson.project_name())
installed_tests_execdir = join_paths(glib_libexecdir, 'installed-tests', meson.project_name())
installed_tests_enabled = get_option('installed_tests')
installed_tests_template = files('template.test.in')
installed_tests_template_tap = files('template-tap.test.in')

add_project_arguments('-D_GNU_SOURCE', language: 'c')

# Disable strict aliasing;
# see https://bugzilla.gnome.org/show_bug.cgi?id=791622
if cc.has_argument('-fno-strict-aliasing')
  add_project_arguments('-fno-strict-aliasing', language: 'c')
endif

########################
# Configuration begins #
########################
glib_conf = configuration_data()
glibconfig_conf = configuration_data()

# accumulated list of defines as we check for them, so we can easily
# use them later in test programs (autoconf does this automatically)
glib_conf_prefix = ''

glib_conf.set('GLIB_MAJOR_VERSION', major_version)
glib_conf.set('GLIB_MINOR_VERSION', minor_version)
glib_conf.set('GLIB_MICRO_VERSION', micro_version)
glib_conf.set('GLIB_INTERFACE_AGE', interface_age)
glib_conf.set('GLIB_BINARY_AGE', binary_age)
glib_conf.set_quoted('GETTEXT_PACKAGE', 'glib20')
glib_conf.set_quoted('PACKAGE_BUGREPORT', 'https://gitlab.gnome.org/GNOME/glib/issues/new')
glib_conf.set_quoted('PACKAGE_NAME', 'glib')
glib_conf.set_quoted('PACKAGE_STRING', 'glib @0@'.format(meson.project_version()))
glib_conf.set_quoted('PACKAGE_TARNAME', 'glib')
glib_conf.set_quoted('PACKAGE_URL', '')
glib_conf.set_quoted('PACKAGE_VERSION', meson.project_version())
glib_conf.set('ENABLE_NLS', 1)

# used by the .rc.in files
glibconfig_conf.set('LT_CURRENT_MINUS_AGE', soversion)

glib_conf.set('_GNU_SOURCE', 1)

if host_system == 'windows'
  # Poll doesn't work on devices on Windows
  glib_conf.set('BROKEN_POLL', true)
endif

if host_system == 'windows' and cc.get_id() != 'msvc'
  # FIXME: Ideally we shouldn't depend on this on Windows and should use
  # 64 bit capable Windows API that also works with MSVC.
  # The autotools build did set this for mingw and while meson sets it
  # for gcc/clang by default, it doesn't do so on Windows.
  glib_conf.set('_FILE_OFFSET_BITS', 64)
endif

# Check for GNU visibility attributes
g_have_gnuc_visibility = cc.compiles('''
  void
  __attribute__ ((visibility ("hidden")))
       f_hidden (void)
  {
  }
  void
  __attribute__ ((visibility ("internal")))
       f_internal (void)
  {
  }
  void
  __attribute__ ((visibility ("protected")))
       f_protected (void)
  {
  }
  void
  __attribute__ ((visibility ("default")))
       f_default (void)
  {
  }
  int main (void)
  {
    f_hidden();
    f_internal();
    f_protected();
    f_default();
    return 0;
  }
  ''',
  # Not supported by MSVC, but MSVC also won't support visibility,
  # so it's OK to pass -Werror explicitly. Replace with
  # override_options : 'werror=true' once that is supported
  args: ['-Werror'],
  name : 'GNU C visibility attributes test')

if g_have_gnuc_visibility
  glibconfig_conf.set('G_HAVE_GNUC_VISIBILITY', '1')
endif

# Detect and set symbol visibility
glib_hidden_visibility_args = []
if get_option('default_library') != 'static'
  if host_system == 'windows'
    glib_conf.set('DLL_EXPORT', true)
    if cc.get_id() == 'msvc'
      glib_conf.set('_GLIB_EXTERN', '__declspec(dllexport) extern')
    elif cc.has_argument('-fvisibility=hidden')
      glib_conf.set('_GLIB_EXTERN', '__attribute__((visibility("default"))) __declspec(dllexport) extern')
      glib_hidden_visibility_args = ['-fvisibility=hidden']
    endif
  elif cc.has_argument('-fvisibility=hidden')
    glib_conf.set('_GLIB_EXTERN', '__attribute__((visibility("default"))) extern')
    glib_hidden_visibility_args = ['-fvisibility=hidden']
  endif
endif

if host_system == 'windows' and get_option('default_library') == 'static'
    glibconfig_conf.set('GLIB_STATIC_COMPILATION', '1')
    glibconfig_conf.set('GOBJECT_STATIC_COMPILATION', '1')
endif

# FIXME: what about Cygwin (G_WITH_CYGWIN)
if host_system == 'windows'
  glib_os = '''#define G_OS_WIN32
#define G_PLATFORM_WIN32'''
else
  glib_os = '#define G_OS_UNIX'
endif
glibconfig_conf.set('glib_os', glib_os)

# We need to know the build type to determine what .lib files we need on Visual Studio
# for dependencies that don't normally come with pkg-config files for Visual Studio builds
buildtype = get_option('buildtype')

glib_debug_cflags = []
if buildtype.startswith('debug')
  glib_debug_cflags += ['-DG_ENABLE_DEBUG']
elif buildtype == 'release'
  glib_debug_cflags += ['-DG_DISABLE_CAST_CHECKS']
endif

add_project_arguments(glib_debug_cflags, language: 'c')

# check for header files

headers = [
  'alloca.h',
  'crt_externs.h',
  'dirent.h', # MSC does not come with this by default
  'float.h',
  'fstab.h',
  'grp.h',
  'inttypes.h',
  'limits.h',
  'linux/magic.h',
  'locale.h',
  'mach/mach_time.h',
  'memory.h',
  'mntent.h',
  'poll.h',
  'pwd.h',
  'sched.h',
  'spawn.h',
  'stdint.h',
  'stdlib.h',
  'string.h',
  'strings.h',
  'sys/auxv.h',
  'sys/event.h',
  'sys/filio.h',
  'sys/inotify.h',
  'sys/mkdev.h',
  'sys/mntctl.h',
  'sys/mnttab.h',
  'sys/mount.h',
  'sys/param.h',
  'sys/resource.h',
  'sys/select.h',
  'sys/statfs.h',
  'sys/stat.h',
  'sys/statvfs.h',
  'sys/sysctl.h',
  'sys/time.h', # MSC does not come with this by default
  'sys/times.h',
  'sys/types.h',
  'sys/uio.h',
  'sys/vfs.h',
  'sys/vfstab.h',
  'sys/vmount.h',
  'sys/wait.h',
  'termios.h',
  'unistd.h',
  'values.h',
  'xlocale.h',
]

foreach h : headers
  if cc.has_header(h)
    define = 'HAVE_' + h.underscorify().to_upper()
    glib_conf.set(define, 1)
    glib_conf_prefix = glib_conf_prefix + '#define @0@ 1\n'.format(define)
  endif
endforeach

# FIXME: Use cc.check_header from Meson 0.47.
# FreeBSD includes a malloc.h which always throw compilation error.
if cc.compiles('#include <malloc.h>', name : 'malloc.h')
  glib_conf.set('HAVE_MALLOC_H', 1)
  glib_conf_prefix = glib_conf_prefix + '#define HAVE_MALLOC_H 1\n'
endif

if cc.has_header('linux/netlink.h')
  glib_conf.set('HAVE_NETLINK', 1)
endif

if glib_conf.has('HAVE_LOCALE_H')
  if cc.has_header_symbol('locale.h', 'LC_MESSAGES')
    glib_conf.set('HAVE_LC_MESSAGES', 1)
  endif
endif

struct_stat_blkprefix = '''
#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_STATFS_H
#include <sys/statfs.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
'''

struct_members = [
  [ 'stat', 'st_mtimensec' ],
  [ 'stat', 'st_mtim.tv_nsec' ],
  [ 'stat', 'st_atimensec' ],
  [ 'stat', 'st_atim.tv_nsec' ],
  [ 'stat', 'st_ctimensec' ],
  [ 'stat', 'st_ctim.tv_nsec' ],
  [ 'stat', 'st_birthtime' ],
  [ 'stat', 'st_birthtimensec' ],
  [ 'stat', 'st_birthtim' ],
  [ 'stat', 'st_birthtim.tv_nsec' ],
  [ 'stat', 'st_blksize', struct_stat_blkprefix ],
  [ 'stat', 'st_blocks', struct_stat_blkprefix ],
  [ 'statfs', 'f_fstypename', struct_stat_blkprefix ],
  [ 'statfs', 'f_bavail', struct_stat_blkprefix ],
  [ 'dirent', 'd_type', '''#include <sys/types.h>
                           #include <dirent.h>''' ],
  [ 'statvfs', 'f_basetype', '#include <sys/statvfs.h>' ],
  [ 'statvfs', 'f_fstypename', '#include <sys/statvfs.h>' ],
  [ 'tm', 'tm_gmtoff', '#include <time.h>' ],
  [ 'tm', '__tm_gmtoff', '#include <time.h>' ],
]

foreach m : struct_members
  header_check_prefix = glib_conf_prefix
  if m.length() == 3
    header_check_prefix = header_check_prefix + m[2]
  else
    header_check_prefix = header_check_prefix + '#include <sys/stat.h>'
  endif
  if cc.has_member('struct ' + m[0], m[1], prefix : header_check_prefix)
    define = 'HAVE_STRUCT_@0@_@1@'.format(m[0].to_upper(), m[1].underscorify().to_upper())
    glib_conf.set(define, 1)
    glib_conf_prefix = glib_conf_prefix + '#define @0@ 1\n'.format(define)
  else
  endif
endforeach

# Compiler flags
if cc.get_id() == 'gcc' or cc.get_id() == 'clang'
  warning_c_args = [
    '-Wall',
    '-Wduplicated-branches',
    '-Wimplicit-fallthrough',
    '-Wmisleading-indentation',
    '-Wstrict-prototypes',
    '-Wunused',
    # Due to pervasive use of things like GPOINTER_TO_UINT(), we do not support
    # building with -Wbad-function-cast.
    '-Wno-bad-function-cast',
    '-Werror=declaration-after-statement',
    '-Werror=format=2',
    '-Werror=implicit-function-declaration',
    '-Werror=init-self',
    '-Werror=missing-include-dirs',
    '-Werror=missing-prototypes',
    '-Werror=pointer-arith',
  ]
  warning_c_link_args = [
    '-Wl,-z,nodelete',
  ]
  if get_option('bsymbolic_functions')
    warning_c_link_args += ['-Wl,-Bsymbolic-functions']
  endif
else
  warning_c_args = []
  warning_c_link_args = []
endif

add_project_arguments(cc.get_supported_arguments(warning_c_args), language: 'c')

# FIXME: We cannot build some of the GResource tests with -z nodelete, which
# means we cannot use that flag in add_project_link_arguments(), and must add
# it to the relevant targets manually. We do the same with -Bsymbolic-functions
# because that is what the autotools build did.
# See https://github.com/mesonbuild/meson/pull/3520 for a way to eventually
# improve this.
glib_link_flags = cc.get_supported_link_arguments(warning_c_link_args)

# Windows Support (7+)
if host_system == 'windows'
  glib_conf.set('_WIN32_WINNT', '0x0601')
endif

functions = [
  'endmntent',
  'endservent',
  'fallocate',
  'fchmod',
  'fchown',
  'fdwalk',
  'fsync',
  'getauxval',
  'getc_unlocked',
  'getfsstat',
  'getgrgid_r',
  'getmntent_r',
  'getpwuid_r',
  'getresuid',
  'getvfsstat',
  'gmtime_r',
  'hasmntopt',
  'inotify_init1',
  'issetugid',
  'kevent',
  'kqueue',
  'lchmod',
  'lchown',
  'link',
  'localtime_r',
  'lstat',
  'mbrtowc',
  'memalign',
  'mmap',
  'newlocale',
  'pipe2',
  'poll',
  'prlimit',
  'readlink',
  'recvmmsg',
  'sendmmsg',
  'setenv',
  'setmntent',
  'strerror_r',
  'strnlen',
  'strsignal',
  'strtod_l',
  'strtoll_l',
  'strtoull_l',
  'symlink',
  'timegm',
  'unsetenv',
  'uselocale',
  'utimes',
  'valloc',
  'vasprintf',
  'vsnprintf',
  'wcrtomb',
  'wcslen',
  'wcsnlen',
  'sysctlbyname',
  '_NSGetEnviron',
]

if glib_conf.has('HAVE_SYS_STATVFS_H')
  functions += ['statvfs']
else
  have_func_statvfs = false
endif
if glib_conf.has('HAVE_SYS_STATFS_H') or glib_conf.has('HAVE_SYS_MOUNT_H')
  functions += ['statfs']
else
  have_func_statfs = false
endif

if host_system == 'windows'
  iphlpapi_dep = cc.find_library('iphlpapi')
  iphlpapi_funcs = ['if_nametoindex', 'if_indextoname']
  foreach ifunc : iphlpapi_funcs
    iphl_prefix =  '''#define _WIN32_WINNT @0@
      #include <winsock2.h>
      #include <iphlpapi.h>'''.format(glib_conf.get('_WIN32_WINNT'))
    if cc.has_function(ifunc,
                       prefix : iphl_prefix,
                       dependencies : iphlpapi_dep)
      idefine = 'HAVE_' + ifunc.underscorify().to_upper()
      glib_conf.set(idefine, 1)
      glib_conf_prefix = glib_conf_prefix + '#define @0@ 1\n'.format(idefine)
      set_variable('have_func_' + ifunc, true)
    else
      set_variable('have_func_' + ifunc, false)
    endif
  endforeach
else
  functions += ['if_indextoname', 'if_nametoindex']
endif

# AIX splice is something else
if host_system != 'aix'
  functions += ['splice']
endif

foreach f : functions
  if cc.has_function(f)
    define = 'HAVE_' + f.underscorify().to_upper()
    glib_conf.set(define, 1)
    glib_conf_prefix = glib_conf_prefix + '#define @0@ 1\n'.format(define)
    set_variable('have_func_' + f, true)
  else
    set_variable('have_func_' + f, false)
  endif
endforeach

# Check that stpcpy() is usable; must use header
if cc.has_function('stpcpy', prefix : '#include <string.h>')
  glib_conf.set('HAVE_STPCPY', 1)
endif

# Check that posix_memalign() is usable; must use header
if cc.has_function('posix_memalign', prefix : '#include <stdlib.h>')
  glib_conf.set('HAVE_POSIX_MEMALIGN', 1)
endif

# Check that posix_spawn() is usable; must use header
if cc.has_function('posix_spawn', prefix : '#include <spawn.h>')
  glib_conf.set('HAVE_POSIX_SPAWN', 1)
endif

# Check whether strerror_r returns char *
if have_func_strerror_r
  if cc.compiles('''#define _GNU_SOURCE
                    #include <string.h>
                    int func (void) {
                      char error_string[256];
                      char *ptr = strerror_r (-2, error_string, 256);
                      char c = *strerror_r (-2, error_string, 256);
                      return c != 0 && ptr != (void*) 0L;
                    }
                 ''',
                 name : 'strerror_r() returns char *')
    glib_conf.set('STRERROR_R_CHAR_P', 1,
                  description: 'Defined if strerror_r returns char *')
  endif
endif

# Special-case these functions that have alternative names on Windows/MSVC
if cc.has_function('snprintf') or cc.has_header_symbol('stdio.h', 'snprintf')
  glib_conf.set('HAVE_SNPRINTF', 1)
  glib_conf_prefix = glib_conf_prefix + '#define HAVE_SNPRINTF 1\n'
elif cc.has_function('_snprintf') or cc.has_header_symbol('stdio.h', '_snprintf')
  hack_define = '1\n#define snprintf _snprintf'
  glib_conf.set('HAVE_SNPRINTF', hack_define)
  glib_conf_prefix = glib_conf_prefix + '#define HAVE_SNPRINTF ' + hack_define
endif

if cc.has_function('strcasecmp')
  glib_conf.set('HAVE_STRCASECMP', 1)
  glib_conf_prefix = glib_conf_prefix + '#define HAVE_STRCASECMP 1\n'
elif cc.has_function('_stricmp')
  hack_define = '1\n#define strcasecmp _stricmp'
  glib_conf.set('HAVE_STRCASECMP', hack_define)
  glib_conf_prefix = glib_conf_prefix + '#define HAVE_STRCASECMP ' + hack_define
endif

if cc.has_function('strncasecmp')
  glib_conf.set('HAVE_STRNCASECMP', 1)
  glib_conf_prefix = glib_conf_prefix + '#define HAVE_STRNCASECMP 1\n'
elif cc.has_function('_strnicmp')
  hack_define = '1\n#define strncasecmp _strnicmp'
  glib_conf.set('HAVE_STRNCASECMP', hack_define)
  glib_conf_prefix = glib_conf_prefix + '#define HAVE_STRNCASECMP ' + hack_define
endif

if cc.has_header_symbol('sys/sysmacros.h', 'major')
  glib_conf.set('MAJOR_IN_SYSMACROS', 1)
elif cc.has_header_symbol('sys/mkdev.h', 'major')
  glib_conf.set('MAJOR_IN_MKDEV', 1)
elif cc.has_header_symbol('sys/types.h', 'major')
  glib_conf.set('MAJOR_IN_TYPES', 1)
endif

if cc.has_header_symbol('dlfcn.h', 'RTLD_LAZY')
  glib_conf.set('HAVE_RTLD_LAZY', 1)
endif

if cc.has_header_symbol('dlfcn.h', 'RTLD_NOW')
  glib_conf.set('HAVE_RTLD_NOW', 1)
endif

if cc.has_header_symbol('dlfcn.h', 'RTLD_GLOBAL')
  glib_conf.set('HAVE_RTLD_GLOBAL', 1)
endif

# Check whether to use statfs or statvfs
# Some systems have both statfs and statvfs, pick the most "native" for these
if have_func_statfs and have_func_statvfs
  # on solaris and irix, statfs doesn't even have the f_bavail field
  if not glib_conf.has('HAVE_STRUCT_STATFS_F_BAVAIL')
    have_func_statfs = false
  else
    # at least on linux, statfs is the actual syscall
    have_func_statvfs = false
  endif
endif
if have_func_statfs
  glib_conf.set('USE_STATFS', 1)
  stat_func_to_use = 'statfs'
elif have_func_statvfs
  glib_conf.set('USE_STATVFS', 1)
  stat_func_to_use = 'statvfs'
else
  stat_func_to_use = 'neither'
endif
message('Checking whether to use statfs or statvfs .. ' + stat_func_to_use)

if host_system == 'linux'
  if cc.has_function('mkostemp',
                     prefix: '''#define _GNU_SOURCE
                                #include <stdlib.h>''')
    glib_conf.set('HAVE_MKOSTEMP', 1)
  endif
endif

osx_ldflags = []
glib_have_os_x_9_or_later = false
glib_have_carbon = false
glib_have_cocoa = false
if host_system == 'darwin'
  add_languages('objc')
  objcc = meson.get_compiler('objc')

  osx_ldflags += ['-Wl,-framework,CoreFoundation']

  # Mac OS X Carbon support
  glib_have_carbon = objcc.compiles('''#include <Carbon/Carbon.h>
                                       #include <CoreServices/CoreServices.h>''',
                                    name : 'Mac OS X Carbon support')

  if glib_have_carbon
    glib_conf.set('HAVE_CARBON', true)
    osx_ldflags += '-Wl,-framework,Carbon'
    glib_have_os_x_9_or_later = objcc.compiles('''#include <AvailabilityMacros.h>
                                                  #if MAC_OS_X_VERSION_MIN_REQUIRED < 1090
                                                  #error Compiling for minimum OS X version before 10.9
                                                  #endif''',
                                               name : 'OS X 9 or later')
  endif

  # Mac OS X Cocoa support
  glib_have_cocoa = objcc.compiles('''#include <Cocoa/Cocoa.h>
                                      #ifdef GNUSTEP_BASE_VERSION
                                      #error "Detected GNUstep, not Cocoa"
                                      #endif''',
                                   name : 'Mac OS X Cocoa support')

  if glib_have_cocoa
    glib_conf.set('HAVE_COCOA', true)
    osx_ldflags += ['-Wl,-framework,Foundation', '-Wl,-framework,AppKit']
  endif

  # FIXME: libgio mix C and objC source files and there is no way to reliably
  # know which language flags it's going to use to link. Add to both languages
  # for now. See https://github.com/mesonbuild/meson/issues/3585.
  add_project_link_arguments(osx_ldflags, language : ['objc', 'c'])
endif

# Check for futex(2)
if cc.links('''#include <linux/futex.h>
               #include <sys/syscall.h>
               #include <unistd.h>
               int main (int argc, char ** argv) {
                 syscall (__NR_futex, NULL, FUTEX_WAKE, FUTEX_WAIT);
                 return 0;
               }''', name : 'futex(2) system call')
  glib_conf.set('HAVE_FUTEX', 1)
endif

# Check for eventfd(2)
if cc.links('''#include <sys/eventfd.h>
               #include <unistd.h>
               int main (int argc, char ** argv) {
                 eventfd (0, EFD_CLOEXEC);
                 return 0;
               }''', name : 'eventfd(2) system call')
  glib_conf.set('HAVE_EVENTFD', 1)
endif

clock_gettime_test_code = '''
  #include <time.h>
  struct timespec t;
  int main (int argc, char ** argv) {
    return clock_gettime(CLOCK_REALTIME, &t);
  }'''
librt = []
if cc.links(clock_gettime_test_code, name : 'clock_gettime')
  glib_conf.set('HAVE_CLOCK_GETTIME', 1)
elif cc.links(clock_gettime_test_code, args : '-lrt', name : 'clock_gettime in librt')
  glib_conf.set('HAVE_CLOCK_GETTIME', 1)
  librt = cc.find_library('rt')
endif

# if statfs() takes 2 arguments (Posix) or 4 (Solaris)
if have_func_statfs
  if cc.compiles(glib_conf_prefix + '''
                 #include <unistd.h>
                        #ifdef HAVE_SYS_PARAM_H
                        #include <sys/param.h>
                        #endif
                        #ifdef HAVE_SYS_VFS_H
                        #include <sys/vfs.h>
                        #endif
                        #ifdef HAVE_SYS_MOUNT_H
                        #include <sys/mount.h>
                        #endif
                        #ifdef HAVE_SYS_STATFS_H
                        #include <sys/statfs.h>
                        #endif
                        void some_func (void) {
                          struct statfs st;
                          statfs("/", &st);
                        }''', name : 'number of arguments to statfs() (n=2)')
    glib_conf.set('STATFS_ARGS', 2)
  elif cc.compiles(glib_conf_prefix + '''
                   #include <unistd.h>
                          #ifdef HAVE_SYS_PARAM_H
                          #include <sys/param.h>
                          #endif
                          #ifdef HAVE_SYS_VFS_H
                          #include <sys/vfs.h>
                          #endif
                          #ifdef HAVE_SYS_MOUNT_H
                          #include <sys/mount.h>
                          #endif
                          #ifdef HAVE_SYS_STATFS_H
                          #include <sys/statfs.h>
                          #endif
                          void some_func (void) {
                            struct statfs st;
                            statfs("/", &st, sizeof (st), 0);
                          }''', name : 'number of arguments to statfs() (n=4)')
    glib_conf.set('STATFS_ARGS', 4)
  else
    error('Unable to determine number of arguments to statfs()')
  endif
endif

# open takes O_DIRECTORY as an option
#AC_MSG_CHECKING([])
if cc.compiles('''#include <fcntl.h>
                  #include <sys/types.h>
                  #include <sys/stat.h>],
                  void some_func (void) {
                    open(0, O_DIRECTORY, 0);
                  }''', name : 'open() option O_DIRECTORY')
  glib_conf.set('HAVE_OPEN_O_DIRECTORY', 1)
endif

# Check whether there is a vsnprintf() function with C99 semantics installed.
# (similar tests to AC_FUNC_VSNPRINTF_C99)
# Check whether there is a snprintf() function with C99 semantics installed.
# (similar tests to AC_FUNC_SNPRINTF_C99)
# Check whether there is a printf() function with Unix98 semantics installed.
# (similar tests to AC_FUNC_PRINTF_UNIX98)
have_good_vsnprintf = false
have_good_snprintf = false
have_good_printf = false

if host_system == 'windows' and cc.get_id() == 'msvc'
  # Unfortunately the Visual Studio 2015+ implementations of C99-style
  # snprintf and vsnprintf don't seem to be quite good enough.
  # (Sorry, I don't know exactly what is the problem,
  # but it is related to floating point formatting and decimal point vs. comma.)
  # The simple tests in AC_FUNC_VSNPRINTF_C99 and AC_FUNC_SNPRINTF_C99 aren't
  # rigorous enough to notice, though.
  glib_conf.set('HAVE_C99_SNPRINTF', false)
  glib_conf.set('HAVE_C99_VSNPRINTF', false)
  glib_conf.set('HAVE_UNIX98_PRINTF', false)
else
  vsnprintf_c99_test_code = '''
#include <stdio.h>
#include <stdarg.h>

int
doit(char * s, ...)
{
  char buffer[32];
  va_list args;
  int r;

  va_start(args, s);
  r = vsnprintf(buffer, 5, s, args);
  va_end(args);

  if (r != 7)
    exit(1);

  /* AIX 5.1 and Solaris seems to have a half-baked vsnprintf()
     implementation. The above will return 7 but if you replace
     the size of the buffer with 0, it borks! */
  va_start(args, s);
  r = vsnprintf(buffer, 0, s, args);
  va_end(args);

  if (r != 7)
    exit(1);

  exit(0);
}

int
main(void)
{
  doit("1234567");
  exit(1);
}'''

  if cc_can_run
    rres = cc.run(vsnprintf_c99_test_code, name : 'C99 vsnprintf')
    if rres.compiled() and rres.returncode() == 0
      glib_conf.set('HAVE_C99_VSNPRINTF', 1)
      have_good_vsnprintf = true
    endif
  else
      have_good_vsnprintf = meson.get_cross_property('have_c99_vsnprintf', false)
      glib_conf.set('HAVE_C99_VSNPRINTF', have_good_vsnprintf)
  endif

  snprintf_c99_test_code = '''
#include <stdio.h>
#include <stdarg.h>

int
doit()
{
  char buffer[32];
  va_list args;
  int r;

  r = snprintf(buffer, 5, "1234567");

  if (r != 7)
    exit(1);

  r = snprintf(buffer, 0, "1234567");

  if (r != 7)
    exit(1);

  r = snprintf(NULL, 0, "1234567");

  if (r != 7)
    exit(1);

  exit(0);
}

int
main(void)
{
  doit();
  exit(1);
}'''

  if cc_can_run
    rres = cc.run(snprintf_c99_test_code, name : 'C99 snprintf')
    if rres.compiled() and rres.returncode() == 0
      glib_conf.set('HAVE_C99_SNPRINTF', 1)
      have_good_snprintf = true
    endif
  else
      have_good_snprintf = meson.get_cross_property('have_c99_snprintf', false)
      glib_conf.set('HAVE_C99_SNPRINTF', have_good_snprintf)
  endif

  printf_unix98_test_code = '''
#include <stdio.h>

int
main (void)
{
  char buffer[128];

  sprintf (buffer, "%2\$d %3\$d %1\$d", 1, 2, 3);
  if (strcmp ("2 3 1", buffer) == 0)
    exit (0);
  exit (1);
}'''

  if cc_can_run
    rres = cc.run(printf_unix98_test_code, name : 'Unix98 printf positional parameters')
    if rres.compiled() and rres.returncode() == 0
      glib_conf.set('HAVE_UNIX98_PRINTF', 1)
      have_good_printf = true
    endif
  else
      have_good_printf = meson.get_cross_property('have_unix98_printf', false)
      glib_conf.set('HAVE_UNIX98_PRINTF', have_good_printf)
  endif
endif

if host_system == 'windows'
  glib_conf.set_quoted('EXEEXT', '.exe')
else
  glib_conf.set('EXEEXT', '')
endif

if have_good_vsnprintf and have_good_snprintf and have_good_printf
  # Our printf is 'good' only if vsnpintf()/snprintf()/printf() supports C99 well enough
  glib_conf.set('HAVE_GOOD_PRINTF', 1)
else
  glib_conf.set('HAVE_VASPRINTF', 1)
endif

glibconfig_conf.set('GLIB_USING_SYSTEM_PRINTF',
                    have_good_vsnprintf and have_good_snprintf and have_good_printf)

# Check whether the printf() family supports Unix98 %n$ positional parameters
# AC_FUNC_PRINTF_UNIX98
# Nothing uses HAVE_UNIX98_PRINTF


# Check for nl_langinfo and CODESET
if cc.links('''#include <langinfo.h>
               int main (int argc, char ** argv) {
                 char *codeset = nl_langinfo (CODESET);
                 return 0;
               }''', name : 'nl_langinfo and CODESET')
  glib_conf.set('HAVE_LANGINFO_CODESET', 1)
  glib_conf.set('HAVE_CODESET', 1)
endif

# Check for nl_langinfo and LC_TIME parts that are needed in gdatetime.c
if cc.links('''#include <langinfo.h>
               int main (int argc, char ** argv) {
                 char *str;
                 str = nl_langinfo (PM_STR);
                 str = nl_langinfo (D_T_FMT);
                 str = nl_langinfo (D_FMT);
                 str = nl_langinfo (T_FMT);
                 str = nl_langinfo (T_FMT_AMPM);
                 str = nl_langinfo (MON_1);
                 str = nl_langinfo (ABMON_12);
                 str = nl_langinfo (DAY_1);
                 str = nl_langinfo (ABDAY_7);
                 return 0;
               }''', name : 'nl_langinfo (PM_STR)')
  glib_conf.set('HAVE_LANGINFO_TIME', 1)
endif
if cc.links('''#include <langinfo.h>
               int main (int argc, char ** argv) {
                 char *str;
                 str = nl_langinfo (_NL_CTYPE_OUTDIGIT0_MB);
                 str = nl_langinfo (_NL_CTYPE_OUTDIGIT1_MB);
                 str = nl_langinfo (_NL_CTYPE_OUTDIGIT2_MB);
                 str = nl_langinfo (_NL_CTYPE_OUTDIGIT3_MB);
                 str = nl_langinfo (_NL_CTYPE_OUTDIGIT4_MB);
                 str = nl_langinfo (_NL_CTYPE_OUTDIGIT5_MB);
                 str = nl_langinfo (_NL_CTYPE_OUTDIGIT6_MB);
                 str = nl_langinfo (_NL_CTYPE_OUTDIGIT7_MB);
                 str = nl_langinfo (_NL_CTYPE_OUTDIGIT8_MB);
                 str = nl_langinfo (_NL_CTYPE_OUTDIGIT9_MB);
                 return 0;
               }''', name : 'nl_langinfo (_NL_CTYPE_OUTDIGITn_MB)')
  glib_conf.set('HAVE_LANGINFO_OUTDIGIT', 1)
endif

# Check for nl_langinfo and alternative month names
if cc.links('''#ifndef _GNU_SOURCE
              # define _GNU_SOURCE
              #endif
              #include <langinfo.h>
               int main (int argc, char ** argv) {
                 char *str;
                 str = nl_langinfo (ALTMON_1);
                 str = nl_langinfo (ALTMON_2);
                 str = nl_langinfo (ALTMON_3);
                 str = nl_langinfo (ALTMON_4);
                 str = nl_langinfo (ALTMON_5);
                 str = nl_langinfo (ALTMON_6);
                 str = nl_langinfo (ALTMON_7);
                 str = nl_langinfo (ALTMON_8);
                 str = nl_langinfo (ALTMON_9);
                 str = nl_langinfo (ALTMON_10);
                 str = nl_langinfo (ALTMON_11);
                 str = nl_langinfo (ALTMON_12);
                 return 0;
               }''', name : 'nl_langinfo (ALTMON_n)')
  glib_conf.set('HAVE_LANGINFO_ALTMON', 1)
endif

# Check for nl_langinfo and abbreviated alternative month names
if cc.links('''#ifndef _GNU_SOURCE
              # define _GNU_SOURCE
              #endif
              #include <langinfo.h>
               int main (int argc, char ** argv) {
                 char *str;
                 str = nl_langinfo (_NL_ALTMON_1);
                 str = nl_langinfo (_NL_ALTMON_2);
                 str = nl_langinfo (_NL_ALTMON_3);
                 str = nl_langinfo (_NL_ALTMON_4);
                 str = nl_langinfo (_NL_ALTMON_5);
                 str = nl_langinfo (_NL_ALTMON_6);
                 str = nl_langinfo (_NL_ALTMON_7);
                 str = nl_langinfo (_NL_ALTMON_8);
                 str = nl_langinfo (_NL_ALTMON_9);
                 str = nl_langinfo (_NL_ALTMON_10);
                 str = nl_langinfo (_NL_ALTMON_11);
                 str = nl_langinfo (_NL_ALTMON_12);
                 return 0;
               }''', name : 'nl_langinfo (_NL_ALTMON_n)')
  glib_conf.set('HAVE_LANGINFO_ABALTMON', 1)
endif

# Check if C compiler supports the 'signed' keyword
if not cc.compiles('''signed char x;''', name : 'signed')
  glib_conf.set('signed', '/* NOOP */')
endif

# Check if the ptrdiff_t type exists
if cc.has_header_symbol('stddef.h', 'ptrdiff_t')
  glib_conf.set('HAVE_PTRDIFF_T', 1)
endif

# Check for sig_atomic_t type
if cc.links('''#include <signal.h>
               #include <sys/types.h>
               sig_atomic_t val = 42;
               int main (int argc, char ** argv) {
                 return val == 42 ? 0 : 1;
               }''', name : 'sig_atomic_t')
  glib_conf.set('HAVE_SIG_ATOMIC_T', 1)
endif

# Check if 'long long' works
# jm_AC_TYPE_LONG_LONG
if cc.compiles('''long long ll = 1LL;
                  int i = 63;
                  int some_func (void) {
                    long long llmax = (long long) -1;
                    return ll << i | ll >> i | llmax / ll | llmax % ll;
                  }''', name : 'long long')
  glib_conf.set('HAVE_LONG_LONG', 1)
  have_long_long = true
else
  have_long_long = false
endif

# Test whether the compiler supports the 'long double' type.
if cc.compiles('''/* The Stardent Vistra knows sizeof(long double), but does not support it.  */
                  long double foo = 0.0;
                  /* On Ultrix 4.3 cc, long double is 4 and double is 8.  */
                  int array [2*(sizeof(long double) >= sizeof(double)) - 1];''',
               name : 'long double')
  glib_conf.set('HAVE_LONG_DOUBLE', 1)
endif

# Test whether <stddef.h> has the 'wchar_t' type.
if cc.has_header_symbol('stddef.h', 'wchar_t')
  glib_conf.set('HAVE_WCHAR_T', 1)
endif

# Test whether <wchar.h> has the 'wint_t' type.
if cc.has_header_symbol('wchar.h', 'wint_t')
  glib_conf.set('HAVE_WINT_T', 1)
endif

found_uintmax_t = false

# Define HAVE_INTTYPES_H_WITH_UINTMAX if <inttypes.h> exists,
# doesn't clash with <sys/types.h>, and declares uintmax_t.
# jm_AC_HEADER_INTTYPES_H
if cc.compiles('''#include <sys/types.h>
                  #include <inttypes.h>
                  void some_func (void) {
                    uintmax_t i = (uintmax_t) -1;
                  }''', name : 'uintmax_t in inttypes.h')
  glib_conf.set('HAVE_INTTYPES_H_WITH_UINTMAX', 1)
  found_uintmax_t = true
endif

# Define HAVE_STDINT_H_WITH_UINTMAX if <stdint.h> exists,
# doesn't clash with <sys/types.h>, and declares uintmax_t.
# jm_AC_HEADER_STDINT_H
if cc.compiles('''#include <sys/types.h>
                  #include <stdint.h>
                  void some_func (void) {
                    uintmax_t i = (uintmax_t) -1;
                  }''', name : 'uintmax_t in stdint.h')
  glib_conf.set('HAVE_STDINT_H_WITH_UINTMAX', 1)
  found_uintmax_t = true
endif

# Define intmax_t to 'long' or 'long long'
# if it is not already defined in <stdint.h> or <inttypes.h>.
# For simplicity, we assume that a header file defines 'intmax_t' if and
# only if it defines 'uintmax_t'.
if found_uintmax_t
  glib_conf.set('HAVE_INTMAX_T', 1)
elif have_long_long
  glib_conf.set('intmax_t', 'long long')
else
  glib_conf.set('intmax_t', 'long')
endif

char_size = cc.sizeof('char')
short_size = cc.sizeof('short')
int_size = cc.sizeof('int')
voidp_size = cc.sizeof('void*')
long_size = cc.sizeof('long')
if have_long_long
  long_long_size = cc.sizeof('long long')
else
  long_long_size = 0
endif
sizet_size = cc.sizeof('size_t')
if cc.get_id() == 'msvc'
  ssizet_size = cc.sizeof('SSIZE_T', prefix : '#include <BaseTsd.h>')
else
  ssizet_size = cc.sizeof('ssize_t')
endif

# Some platforms (Apple) hard-code int64_t to long long instead of
# using long on 64-bit architectures. This can cause type mismatch
# warnings when trying to interface with code using the standard
# library type. Test for the warnings and set gint64 to whichever
# works.
if long_long_size == long_size
  if cc.compiles('''#if defined(_AIX) && !defined(__GNUC__)
                    #pragma options langlvl=stdc99
                    #endif
                    #pragma GCC diagnostic error "-Wincompatible-pointer-types"
                    #include <stdint.h>
                    #include <stdio.h>
                    int main () {
                      int64_t i1 = 1;
                      long *i2 = &i1;
                      return 1;
                    }''', name : 'int64_t is long')
    int64_t_typedef = 'long'
  elif cc.compiles('''#if defined(_AIX) && !defined(__GNUC__)
                      #pragma options langlvl=stdc99
                      #endif
                      #pragma GCC diagnostic error "-Wincompatible-pointer-types"
                      #include <stdint.h>
                      #include <stdio.h>
                      int main () {
                        int64_t i1 = 1;
                        long long *i2 = &i1;
                        return 1;
                      }''', name : 'int64_t is long long')
    int64_t_typedef = 'long long'
  endif
endif

int64_m = 'll'
char_align = cc.alignment('char')
short_align = cc.alignment('short')
int_align = cc.alignment('int')
voidp_align = cc.alignment('void*')
long_align = cc.alignment('long')
long_long_align = cc.alignment('long long')
# NOTE: We don't check for size of __int64 because long long is guaranteed to
# be 64-bit in C99, and it is available on all supported compilers
sizet_align = cc.alignment('size_t')

glib_conf.set('ALIGNOF_UNSIGNED_LONG', long_align)

glib_conf.set('SIZEOF_CHAR', char_size)
glib_conf.set('SIZEOF_INT', int_size)
glib_conf.set('SIZEOF_SHORT', short_size)
glib_conf.set('SIZEOF_LONG', long_size)
glib_conf.set('SIZEOF_LONG_LONG', long_long_size)
glib_conf.set('SIZEOF_SIZE_T', sizet_size)
glib_conf.set('SIZEOF_SSIZE_T', ssizet_size)
glib_conf.set('SIZEOF_VOID_P', voidp_size)

if short_size == 2
  gint16 = 'short'
  gint16_modifier='h'
  gint16_format='hi'
  guint16_format='hu'
elif int_size == 2
  gint16 = 'int'
  gint16_modifier=''
  gint16_format='i'
  guint16_format='u'
else
  error('Compiler provides no native 16-bit integer type')
endif
glibconfig_conf.set('gint16', gint16)
glibconfig_conf.set_quoted('gint16_modifier', gint16_modifier)
glibconfig_conf.set_quoted('gint16_format', gint16_format)
glibconfig_conf.set_quoted('guint16_format', guint16_format)

if short_size == 4
  gint32 = 'short'
  gint32_modifier='h'
  gint32_format='hi'
  guint32_format='hu'
  guint32_align = short_align
elif int_size == 4
  gint32 = 'int'
  gint32_modifier=''
  gint32_format='i'
  guint32_format='u'
  guint32_align = int_align
elif long_size == 4
  gint32 = 'long'
  gint32_modifier='l'
  gint32_format='li'
  guint32_format='lu'
  guint32_align = long_align
else
  error('Compiler provides no native 32-bit integer type')
endif
glibconfig_conf.set('gint32', gint32)
glibconfig_conf.set_quoted('gint32_modifier', gint32_modifier)
glibconfig_conf.set_quoted('gint32_format', gint32_format)
glibconfig_conf.set_quoted('guint32_format', guint32_format)
glib_conf.set('ALIGNOF_GUINT32', guint32_align)

if int_size == 8
  gint64 = 'int'
  gint64_modifier=''
  gint64_format='i'
  guint64_format='u'
  glib_extension=''
  gint64_constant='(val)'
  guint64_constant='(val)'
  guint64_align = int_align
elif long_size == 8 and (long_long_size != long_size or int64_t_typedef == 'long')
  gint64 = 'long'
  glib_extension=''
  gint64_modifier='l'
  gint64_format='li'
  guint64_format='lu'
  gint64_constant='(val##L)'
  guint64_constant='(val##UL)'
  guint64_align = long_align
elif long_long_size == 8 and (long_long_size != long_size or int64_t_typedef == 'long long')
  gint64 = 'long long'
  glib_extension='G_GNUC_EXTENSION '
  gint64_modifier=int64_m
  gint64_format=int64_m + 'i'
  guint64_format=int64_m + 'u'
  gint64_constant='(G_GNUC_EXTENSION (val##LL))'
  guint64_constant='(G_GNUC_EXTENSION (val##ULL))'
  guint64_align = long_long_align
else
  error('Compiler provides no native 64-bit integer type')
endif
glibconfig_conf.set('glib_extension', glib_extension)
glibconfig_conf.set('gint64', gint64)
glibconfig_conf.set_quoted('gint64_modifier', gint64_modifier)
glibconfig_conf.set_quoted('gint64_format', gint64_format)
glibconfig_conf.set_quoted('guint64_format', guint64_format)
glibconfig_conf.set('gint64_constant', gint64_constant)
glibconfig_conf.set('guint64_constant', guint64_constant)
glib_conf.set('ALIGNOF_GUINT64', guint64_align)

if host_system == 'windows'
  glibconfig_conf.set('g_pid_type', 'void*')
  glibconfig_conf.set_quoted('g_pid_format', 'p')
  if host_machine.cpu_family() == 'x86_64'
    glibconfig_conf.set_quoted('g_pollfd_format', '%#' + int64_m + 'x')
  else
    glibconfig_conf.set_quoted('g_pollfd_format', '%#x')
  endif
  glibconfig_conf.set('g_dir_separator', '\\\\')
  glibconfig_conf.set('g_searchpath_separator', ';')
else
  glibconfig_conf.set('g_pid_type', 'int')
  glibconfig_conf.set_quoted('g_pid_format', 'i')
  glibconfig_conf.set_quoted('g_pollfd_format', '%d')
  glibconfig_conf.set('g_dir_separator', '/')
  glibconfig_conf.set('g_searchpath_separator', ':')
endif

if sizet_size == short_size
  glibconfig_conf.set('glib_size_type_define', 'short')
  glibconfig_conf.set_quoted('gsize_modifier', 'h')
  glibconfig_conf.set_quoted('gssize_modifier', 'h')
  glibconfig_conf.set_quoted('gsize_format', 'hu')
  glibconfig_conf.set_quoted('gssize_format', 'hi')
  glibconfig_conf.set('glib_msize_type', 'SHRT')
elif sizet_size == int_size
  glibconfig_conf.set('glib_size_type_define', 'int')
  glibconfig_conf.set_quoted('gsize_modifier', '')
  glibconfig_conf.set_quoted('gssize_modifier', '')
  glibconfig_conf.set_quoted('gsize_format', 'u')
  glibconfig_conf.set_quoted('gssize_format', 'i')
  glibconfig_conf.set('glib_msize_type', 'INT')
elif sizet_size == long_size
  glibconfig_conf.set('glib_size_type_define', 'long')
  glibconfig_conf.set_quoted('gsize_modifier', 'l')
  glibconfig_conf.set_quoted('gssize_modifier', 'l')
  glibconfig_conf.set_quoted('gsize_format', 'lu')
  glibconfig_conf.set_quoted('gssize_format', 'li')
  glibconfig_conf.set('glib_msize_type', 'LONG')
elif sizet_size == long_long_size
  glibconfig_conf.set('glib_size_type_define', 'long long')
  glibconfig_conf.set_quoted('gsize_modifier', int64_m)
  glibconfig_conf.set_quoted('gssize_modifier', int64_m)
  glibconfig_conf.set_quoted('gsize_format', int64_m + 'u')
  glibconfig_conf.set_quoted('gssize_format', int64_m + 'i')
  glibconfig_conf.set('glib_msize_type', 'INT64')
else
  error('Could not determine size of size_t.')
endif

if voidp_size == int_size
  glibconfig_conf.set('glib_intptr_type_define', 'int')
  glibconfig_conf.set_quoted('gintptr_modifier', '')
  glibconfig_conf.set_quoted('gintptr_format', 'i')
  glibconfig_conf.set_quoted('guintptr_format', 'u')
  glibconfig_conf.set('glib_gpi_cast', '(gint)')
  glibconfig_conf.set('glib_gpui_cast', '(guint)')
elif voidp_size == long_size
  glibconfig_conf.set('glib_intptr_type_define', 'long')
  glibconfig_conf.set_quoted('gintptr_modifier', 'l')
  glibconfig_conf.set_quoted('gintptr_format', 'li')
  glibconfig_conf.set_quoted('guintptr_format', 'lu')
  glibconfig_conf.set('glib_gpi_cast', '(glong)')
  glibconfig_conf.set('glib_gpui_cast', '(gulong)')
elif voidp_size == long_long_size
  glibconfig_conf.set('glib_intptr_type_define', 'long long')
  glibconfig_conf.set_quoted('gintptr_modifier', int64_m)
  glibconfig_conf.set_quoted('gintptr_format', int64_m + 'i')
  glibconfig_conf.set_quoted('guintptr_format', int64_m + 'u')
  glibconfig_conf.set('glib_gpi_cast', '(gint64)')
  glibconfig_conf.set('glib_gpui_cast', '(guint64)')
else
  error('Could not determine size of void *')
endif

if long_size != 8 and long_long_size != 8 and int_size != 8
  error('GLib requires a 64-bit type. You might want to consider using the GNU C compiler.')
endif

glibconfig_conf.set('gintbits', int_size * 8)
glibconfig_conf.set('glongbits', long_size * 8)
glibconfig_conf.set('gsizebits', sizet_size * 8)
glibconfig_conf.set('gssizebits', ssizet_size * 8)

# XXX: https://gitlab.gnome.org/GNOME/glib/issues/1413
if host_system == 'windows'
  g_module_suffix = 'dll'
else
  g_module_suffix = 'so'
endif
glibconfig_conf.set('g_module_suffix', g_module_suffix)

glibconfig_conf.set('GLIB_MAJOR_VERSION', major_version)
glibconfig_conf.set('GLIB_MINOR_VERSION', minor_version)
glibconfig_conf.set('GLIB_MICRO_VERSION', micro_version)
glibconfig_conf.set('GLIB_VERSION', glib_version)

glibconfig_conf.set('glib_void_p', voidp_size)
glibconfig_conf.set('glib_long', long_size)
glibconfig_conf.set('glib_size_t', sizet_size)
glibconfig_conf.set('glib_ssize_t', ssizet_size)
if host_machine.endian() == 'big'
  glibconfig_conf.set('g_byte_order', 'G_BIG_ENDIAN')
  glibconfig_conf.set('g_bs_native', 'BE')
  glibconfig_conf.set('g_bs_alien', 'LE')
else
  glibconfig_conf.set('g_byte_order', 'G_LITTLE_ENDIAN')
  glibconfig_conf.set('g_bs_native', 'LE')
  glibconfig_conf.set('g_bs_alien', 'BE')
endif

# === va_copy checks ===
# we currently check for all three va_copy possibilities, so we get
# all results in config.log for bug reports.

va_copy_func = ''
foreach try_func : [ '__va_copy', 'va_copy' ]
  if cc.compiles('''#include <stdarg.h>
                    #include <stdlib.h>
                    #ifdef _MSC_VER
                    # include "msvc_recommended_pragmas.h"
                    #endif
                    void f (int i, ...) {
                    va_list args1, args2;
                    va_start (args1, i);
                    @0@ (args2, args1);
                    if (va_arg (args2, int) != 42 || va_arg (args1, int) != 42)
                      exit (1);
                    va_end (args1); va_end (args2);
                    }
                    int main() {
                      f (0, 42);
                      return 0;
                    }'''.format(try_func),
                    name : try_func + ' check')
    va_copy_func = try_func
  endif
endforeach
if va_copy_func != ''
  glib_conf.set('G_VA_COPY', va_copy_func)
  glib_vacopy = '#define G_VA_COPY ' + va_copy_func
else
  glib_vacopy = '/* #undef G_VA_COPY */'
endif

va_list_val_copy_prog = '''
  #include <stdarg.h>
  #include <stdlib.h>
  void f (int i, ...) {
    va_list args1, args2;
    va_start (args1, i);
    args2 = args1;
    if (va_arg (args2, int) != 42 || va_arg (args1, int) != 42)
      exit (1);
    va_end (args1); va_end (args2);
  }
  int main() {
    f (0, 42);
    return 0;
  }'''

if cc_can_run
  rres = cc.run(va_list_val_copy_prog, name : 'va_lists can be copied as values')
  glib_va_val_copy = rres.returncode() == 0
else
  glib_va_val_copy = meson.get_cross_property('va_val_copy', true)
endif
if not glib_va_val_copy
  glib_vacopy = glib_vacopy + '\n#define G_VA_COPY_AS_ARRAY 1'
  glib_conf.set('G_VA_COPY_AS_ARRAY', 1)
endif
glibconfig_conf.set('glib_vacopy', glib_vacopy)

# check for flavours of varargs macros
g_have_iso_c_varargs = cc.compiles('''
  void some_func (void) {
    int a(int p1, int p2, int p3);
    #define call_a(...) a(1,__VA_ARGS__)
    call_a(2,3);
  }''', name : 'ISO C99 varargs macros in C')

if g_have_iso_c_varargs
  glibconfig_conf.set('g_have_iso_c_varargs', '''
#ifndef __cplusplus
# define G_HAVE_ISO_VARARGS 1
#endif''')
endif

g_have_iso_cxx_varargs = cxx.compiles('''
  void some_func (void) {
    int a(int p1, int p2, int p3);
    #define call_a(...) a(1,__VA_ARGS__)
    call_a(2,3);
  }''', name : 'ISO C99 varargs macros in C++')

if g_have_iso_cxx_varargs
  glibconfig_conf.set('g_have_iso_cxx_varargs', '''
#ifdef __cplusplus
# define G_HAVE_ISO_VARARGS 1
#endif''')
endif

g_have_gnuc_varargs = cc.compiles('''
  void some_func (void) {
    int a(int p1, int p2, int p3);
    #define call_a(params...) a(1,params)
    call_a(2,3);
  }''', name : 'GNUC varargs macros')

if cc.has_header('alloca.h')
  glibconfig_conf.set('GLIB_HAVE_ALLOCA_H', true)
endif
has_syspoll = cc.has_header('sys/poll.h')
has_systypes = cc.has_header('sys/types.h')
if has_syspoll
  glibconfig_conf.set('GLIB_HAVE_SYS_POLL_H', true)
endif
has_winsock2 = cc.has_header('winsock2.h')

if has_syspoll and has_systypes
  poll_includes = '''
      #include<sys/poll.h>
      #include<sys/types.h>'''
elif has_winsock2
  poll_includes = '''
      #define _WIN32_WINNT @0@
      #include <winsock2.h>'''.format(glib_conf.get('_WIN32_WINNT'))
else
  # FIXME?
  error('FIX POLL* defines')
endif

poll_defines = [
  [ 'POLLIN', 'g_pollin', 1 ],
  [ 'POLLOUT', 'g_pollout', 4 ],
  [ 'POLLPRI', 'g_pollpri', 2 ],
  [ 'POLLERR', 'g_pollerr', 8 ],
  [ 'POLLHUP', 'g_pollhup', 16 ],
  [ 'POLLNVAL', 'g_pollnval', 32 ],
]

if has_syspoll and has_systypes
  foreach d : poll_defines
    val = cc.compute_int(d[0], prefix: poll_includes)
    glibconfig_conf.set(d[1], val)
  endforeach
elif has_winsock2
  # Due to a missed bug in configure.ac the poll test
  # never succeeded on Windows and used some pre-defined
  # values as a fallback. Keep using them to maintain
  # ABI compatibility with autotools builds of glibs
  # and with *any* glib-using code compiled against them,
  # since these values end up in a public header glibconfig.h.
  foreach d : poll_defines
    glibconfig_conf.set(d[1], d[2])
  endforeach
endif

# Internet address families
# FIXME: what about Cygwin (G_WITH_CYGWIN)
if host_system == 'windows'
  inet_includes = '''
      #include <winsock2.h>'''
else
  inet_includes = '''
      #include <sys/types.h>
      #include <sys/socket.h>'''
endif

inet_defines = [
  [ 'AF_UNIX', 'g_af_unix' ],
  [ 'AF_INET', 'g_af_inet' ],
  [ 'AF_INET6', 'g_af_inet6' ],
  [ 'MSG_OOB', 'g_msg_oob' ],
  [ 'MSG_PEEK', 'g_msg_peek' ],
  [ 'MSG_DONTROUTE', 'g_msg_dontroute' ],
]
foreach d : inet_defines
  val = cc.compute_int(d[0], prefix: inet_includes)
  glibconfig_conf.set(d[1], val)
endforeach

# We need a more robust approach here...
host_cpu_family = host_machine.cpu_family()
if host_cpu_family == 'x86' or host_cpu_family == 'x86_64' or host_cpu_family == 's390' or host_cpu_family == 's390x' or host_cpu_family.startswith('arm') or host_cpu_family == 'aarch64' or host_cpu_family.startswith('crisv32') or host_cpu_family.startswith('etrax')
  glib_memory_barrier_needed = false
elif host_cpu_family.startswith('sparc') or host_cpu_family.startswith('alpha') or host_cpu_family.startswith('powerpc') or host_cpu_family == 'ia64'
  glib_memory_barrier_needed = true
else
  warning('Unknown host cpu: ' + host_cpu_family)
  glib_memory_barrier_needed = true
endif
glibconfig_conf.set('G_ATOMIC_OP_MEMORY_BARRIER_NEEDED', glib_memory_barrier_needed)

# We need to decide at configure time if GLib will use real atomic
# operations ("lock free") or emulated ones with a mutex.  This is
# because we must put this information in glibconfig.h so we know if
# it is safe or not to inline using compiler intrinsics directly from
# the header.
#
# We also publish the information via G_ATOMIC_LOCK_FREE in case the
# user is interested in knowing if they can use the atomic ops across
# processes.
#
# We can currently support the atomic ops natively when building GLib
# with recent versions of GCC or MSVC.
#
# Note that the atomic ops are only available with GCC on x86 when
# using -march=i486 or higher.  If we detect that the atomic ops are
# not available but would be available given the right flags, we want
# to abort and advise the user to fix their CFLAGS.  It's better to do
# that then to silently fall back on emulated atomic ops just because
# the user had the wrong build environment.
atomictest = '''int main() {
  volatile int atomic = 2;
  __sync_bool_compare_and_swap (&atomic, 2, 3);
  return 0;
}
'''

atomicdefine = '''
#ifndef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
#error "compiler has atomic ops, but doesn't define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4"
#endif
'''

# We know that we can always use real ("lock free") atomic operations with MSVC
if cc.get_id() == 'msvc' or cc.links(atomictest, name : 'atomic ops')
  have_atomic_lock_free = true
  if (host_system == 'android' or host_system == 'linux') and not cc.compiles(atomicdefine, name : 'atomic ops define')
    # When building for armv5 on Linux, gcc provides
    # __sync_bool_compare_and_swap but doesn't define
    # __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
    glib_conf.set('__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4', true)
  endif
else
  have_atomic_lock_free = false
  if host_machine.cpu_family() == 'x86' and cc.links(atomictest, args : '-march=i486')
    error('GLib must be built with -march=i486 or later.')
  endif
endif
glibconfig_conf.set('G_ATOMIC_LOCK_FREE', have_atomic_lock_free)

# === Threads ===

# Determination of thread implementation
if host_system == 'windows' and not get_option('force_posix_threads')
  thread_dep = []
  threads_implementation = 'win32'
  glibconfig_conf.set('g_threads_impl_def', 'WIN32')
  glib_conf.set('THREADS_WIN32', 1)
else
  thread_dep = dependency('threads')
  threads_implementation = 'posix'
  pthread_prefix = '''
      #ifndef _GNU_SOURCE
      # define _GNU_SOURCE
      #endif
      #include <pthread.h>'''
  glibconfig_conf.set('g_threads_impl_def', 'POSIX')
  glib_conf.set('THREADS_POSIX', 1)
  if cc.has_header_symbol('pthread.h', 'pthread_attr_setstacksize')
    glib_conf.set('HAVE_PTHREAD_ATTR_SETSTACKSIZE', 1)
  endif
  if cc.has_header_symbol('pthread.h', 'pthread_condattr_setclock')
    glib_conf.set('HAVE_PTHREAD_CONDATTR_SETCLOCK', 1)
  endif
  if cc.has_header_symbol('pthread.h', 'pthread_cond_timedwait_relative_np')
    glib_conf.set('HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE_NP', 1)
  endif
  if cc.has_header_symbol('pthread.h', 'pthread_getname_np', prefix : pthread_prefix)
    glib_conf.set('HAVE_PTHREAD_GETNAME_NP', 1)
  endif
  # Assume that pthread_setname_np is available in some form; same as configure
  if cc.links(pthread_prefix + '''
              int main() {
                pthread_setname_np("example");
                return 0;
              }''',
              name : 'pthread_setname_np(const char*)',
              dependencies : thread_dep)
    # macOS and iOS
    glib_conf.set('HAVE_PTHREAD_SETNAME_NP_WITHOUT_TID', 1)
  elif cc.links(pthread_prefix + '''
                int main() {
                  pthread_setname_np(pthread_self(), "example");
                  return 0;
                }''',
                name : 'pthread_setname_np(pthread_t, const char*)',
                dependencies : thread_dep)
    # Linux, Solaris, etc.
    glib_conf.set('HAVE_PTHREAD_SETNAME_NP_WITH_TID', 1)
  endif
endif

# FIXME: we should make it print the result and always return 0, so that
# the output in meson shows up as green
stack_grows_check_prog = '''
  volatile int *a = 0, *b = 0;
  void f (int i) {
    volatile int x = 5;
    if (i == 0)
      b = &x;
    else
      f (i - 1);
  }
  int main () {
    volatile int y = 7;
    a = &y;
    f (100);
    return b > a ? 0 : 1;
  }'''

if cc_can_run
  rres = cc.run(stack_grows_check_prog, name : 'stack grows check')
  growing_stack = rres.returncode() == 0
else
  growing_stack = meson.get_cross_property('growing_stack', false)
endif

glibconfig_conf.set10('G_HAVE_GROWING_STACK', growing_stack)

# Tests for iconv
#
# USE_LIBICONV_GNU: Using GNU libiconv
# USE_LIBICONV_NATIVE: Using a native impl of iconv in a separate library
#
# We should never use the MinGW C library's iconv. On Windows we use the
# GNU implementation that ships with MinGW.

# On Windows, just always use the built-in implementation
if host_system == 'windows'
  libiconv = []
  glib_conf.set('USE_LIBICONV_NATIVE', true)
else
  found_iconv = false
  iconv_opt = get_option('iconv')
  if iconv_opt == 'libc'
    if cc.has_function('iconv_open')
      libiconv = []
      found_iconv = true
    endif
  elif iconv_opt == 'gnu'
    if cc.has_header_symbol('iconv.h', 'libiconv_open')
      glib_conf.set('USE_LIBICONV_GNU', true)
      libiconv = [cc.find_library('iconv')]
      found_iconv = true
    endif
  elif iconv_opt == 'native'
    if cc.has_header_symbol('iconv.h', 'iconv_open')
      glib_conf.set('USE_LIBICONV_NATIVE', true)
      libiconv = [cc.find_library('iconv')]
      found_iconv = true
    endif
  endif

  if not found_iconv
    error('iconv implementation "@0@" not found'.format(iconv_opt))
  endif

endif

if get_option('internal_pcre')
  pcre = []
  use_system_pcre = false
else
  pcre = dependency('libpcre', version: '>= 8.31', required : false) # Should check for Unicode support, too. FIXME
  if not pcre.found()
    if cc.get_id() == 'msvc'
    # MSVC: Search for the PCRE library by the configuration, which corresponds
    # to the output of CMake builds of PCRE.  Note that debugoptimized
    # is really a Release build with .PDB files.
      if buildtype == 'debug'
        pcre = cc.find_library('pcred', required : false)
      else
        pcre = cc.find_library('pcre', required : false)
      endif
    endif
  endif
  use_system_pcre = pcre.found()
endif
glib_conf.set('USE_SYSTEM_PCRE', use_system_pcre)

use_pcre_static_flag = false

if host_system == 'windows'
  if not use_system_pcre
    use_pcre_static_flag = true
  else
    pcre_static = cc.links('''#define PCRE_STATIC
                              #include <pcre.h>
                              int main() {
                                void *p = NULL;
                                pcre_free(p);
                                return 0;
                              }''',
                           dependencies: pcre,
                           name : 'Windows system PCRE is a static build')
    if pcre_static
      use_pcre_static_flag = true
    endif
  endif
endif

libm = cc.find_library('m', required : false)
libffi_dep = dependency('libffi', version : '>= 3.0.0', fallback : ['libffi', 'ffi_dep'])

# Don't use the bundled ZLib sources until we are sure that we can't find it on
# the system
libz_dep = dependency('zlib', required : false)
if not libz_dep.found()
  if cc.get_id() != 'msvc'
    libz_dep = cc.find_library('z', required : false)
  else
    libz_dep = cc.find_library('zlib1', required : false)
    if not libz_dep.found()
      libz_dep = cc.find_library('zlib', required : false)
    endif
  endif
  if not libz_dep.found() or not cc.has_header('zlib.h')
    libz_dep = subproject('zlib').get_variable('zlib_dep')
  endif
endif

# First check in libc, fallback to libintl, and as last chance build
# proxy-libintl subproject.
# FIXME: glib-gettext.m4 has much more checks to detect broken/uncompatible
# implementations. This could be extended if issues are found in some platforms.
if cc.has_function('ngettext')
  libintl = []
  have_bind_textdomain_codeset = cc.has_function('bind_textdomain_codeset')
else
  libintl = cc.find_library('intl', required : false)
  if not libintl.found()
    libintl = subproject('proxy-libintl').get_variable('intl_dep')
    have_bind_textdomain_codeset = true  # proxy-libintl supports it
  else
    have_bind_textdomain_codeset = cc.has_function('bind_textdomain_codeset',
                                                   dependencies : libintl)
  endif
endif

glib_conf.set('HAVE_BIND_TEXTDOMAIN_CODESET', have_bind_textdomain_codeset)

# We require gettext to always be present
glib_conf.set('HAVE_DCGETTEXT', 1)
glib_conf.set('HAVE_GETTEXT', 1)

glib_conf.set_quoted('GLIB_LOCALE_DIR', join_paths(glib_datadir, 'locale'))

# libmount is only used by gio, but we need to fetch the libs to generate the
# pkg-config file below
libmount_dep = []
if host_system == 'linux' and get_option('libmount')
  libmount_dep = [dependency('mount', version : '>=2.23', required : true)]
  glib_conf.set('HAVE_LIBMOUNT', 1)
endif

if host_system == 'windows'
  winsock2 = cc.find_library('ws2_32')
endif

selinux_dep = []
if host_system == 'linux'
  selinux_dep = dependency('libselinux', required: get_option('selinux'))

  glib_conf.set('HAVE_SELINUX', selinux_dep.found())
endif

xattr_dep = []
if host_system != 'windows' and get_option('xattr')
  # either glibc or libattr can provide xattr support
  # for both of them, we check for getxattr being in
  # the library and a valid xattr header.

  # try glibc
  if cc.has_function('getxattr') and cc.has_header('sys/xattr.h')
    glib_conf.set('HAVE_SYS_XATTR_H', 1)
    glib_conf_prefix = glib_conf_prefix + '#define @0@ 1\n'.format('HAVE_SYS_XATTR_H')
  #failure. try libattr
  elif cc.has_header_symbol('attr/xattr.h', 'getxattr')
    glib_conf.set('HAVE_ATTR_XATTR_H', 1)
    glib_conf_prefix = glib_conf_prefix + '#define @0@ 1\n'.format('HAVE_ATTR_XATTR_H')
    xattr_dep = [cc.find_library('xattr')]
  else
    error('No getxattr implementation found in C library or libxattr')
  endif

  glib_conf.set('HAVE_XATTR', 1)
  if cc.compiles(glib_conf_prefix + '''
                 #include <stdio.h>
                 #ifdef HAVE_SYS_TYPES_H
                 #include <sys/types.h>
                 #endif
                 #ifdef HAVE_SYS_XATTR_H
                 #include <sys/xattr.h>
                 #elif HAVE_ATTR_XATTR_H
                 #include <attr/xattr.h>
                 #endif

                 int main (void) {
                   ssize_t len = getxattr("", "", NULL, 0, 0, XATTR_NOFOLLOW);
                   return len;
                 }''',
                 name : 'XATTR_NOFOLLOW')
    glib_conf.set('HAVE_XATTR_NOFOLLOW', 1)
  endif
endif

# Test if we have strlcpy/strlcat with a compatible implementation:
# https://bugzilla.gnome.org/show_bug.cgi?id=53933
if cc_can_run
  rres = cc.run('''#include <stdlib.h>
                   #include <string.h>
                   int main() {
                     char p[10];
                     (void) strlcpy (p, "hi", 10);
                     if (strlcat (p, "bye", 0) != 3)
                       return 1;
                     return 0;
                   }''',
                name : 'OpenBSD strlcpy/strlcat')
  if rres.compiled() and rres.returncode() == 0
    glib_conf.set('HAVE_STRLCPY', 1)
  endif
elif meson.get_cross_property('have_strlcpy', false)
  glib_conf.set('HAVE_STRLCPY', 1)
endif

python = import('python').find_installation('python3')
# used for '#!/usr/bin/env <name>'
python_name = 'python3'

python_version = python.language_version()
python_version_req = '>=3.4'
if not python_version.version_compare(python_version_req)
  error('Requires Python @0@, @1@ found.'.format(python_version_req, python_version))
endif

# Determine which user environment-dependent files that we want to install
have_bash = find_program('bash', required : false).found() # For completion scripts
have_sh = find_program('sh', required : false).found() # For glib-gettextize

# FIXME: How to detect Solaris? https://github.com/mesonbuild/meson/issues/1578
if host_system == 'sunos'
  glib_conf.set('_XOPEN_SOURCE_EXTENDED', 1)
  glib_conf.set('_XOPEN_SOURCE', 2)
  glib_conf.set('__EXTENSIONS__',1)
endif

# Sadly Meson does not expose this value:
# https://github.com/mesonbuild/meson/pull/3460
if host_system == 'windows'
  # Autotools explicitly removed --Wl,--export-all-symbols from windows builds,
  # with no explanation. Do the same here for now but this could be revisited if
  # if causes issues.
  export_dynamic_ldflags = []
elif host_system == 'cygwin'
  export_dynamic_ldflags = ['-Wl,--export-all-symbols']
elif host_system == 'darwin'
  export_dynamic_ldflags = []
else
  export_dynamic_ldflags = ['-Wl,--export-dynamic']
endif

win32_cflags = []
win32_ldflags = []
if host_system == 'windows' and cc.get_id() != 'msvc'
  # Ensure MSVC-compatible struct packing convention is used when
  # compiling for Win32 with gcc. It is used for the whole project and exposed
  # in glib-2.0.pc.
  win32_cflags = ['-mms-bitfields']
  add_project_arguments(win32_cflags, language : 'c')

  # Win32 API libs, used only by libglib and exposed in glib-2.0.pc
  win32_ldflags = ['-lws2_32', '-lole32', '-lwinmm', '-lshlwapi']
elif host_system == 'cygwin'
  win32_ldflags = ['-luser32', '-lkernel32']
endif

# Tracing: dtrace
want_dtrace = get_option('dtrace')
enable_dtrace = false

# Since dtrace support is opt-in we just error out if it was requested but
# is not available. We don't bother with autodetection yet.
if want_dtrace
  if glib_have_carbon
    error('GLib dtrace support not yet compatible with macOS dtrace')
  endif
  dtrace = find_program('dtrace', required : true) # error out if not found
  if not cc.has_header('sys/sdt.h')
    error('dtrace support needs sys/sdt.h header')
  endif
  # FIXME: autotools build also passes -fPIC -DPIC but is it needed in this case?
  dtrace_obj_gen = generator(dtrace,
    output : '@BASENAME@.o',
    arguments : ['-G', '-s', '@INPUT@', '-o', '@OUTPUT@'])
  # FIXME: $(SED) -e "s,define STAP_HAS_SEMAPHORES 1,undef STAP_HAS_SEMAPHORES,"
  #               -e "s,define _SDT_HAS_SEMAPHORES 1,undef _SDT_HAS_SEMAPHORES,"
  dtrace_hdr_gen = generator(dtrace,
    output : '@BASENAME@.h',
    arguments : ['-h', '-s', '@INPUT@', '-o', '@OUTPUT@'])
  glib_conf.set('HAVE_DTRACE', 1)
  enable_dtrace = true
endif

# systemtap
want_systemtap = get_option('systemtap')
enable_systemtap = false

if want_systemtap and enable_dtrace
  tapset_install_dir = get_option('tapset_install_dir')
  if tapset_install_dir == ''
    tapset_install_dir = join_paths(get_option('datadir'), 'systemtap/tapset', host_machine.cpu_family())
  endif
  stp_cdata = configuration_data()
  stp_cdata.set('ABS_GLIB_RUNTIME_LIBDIR', glib_libdir)
  stp_cdata.set('LT_CURRENT', minor_version * 100)
  stp_cdata.set('LT_REVISION', micro_version)
  enable_systemtap = true
endif

test_timeout = 60
test_timeout_slow = 120

pkg = import('pkgconfig')
windows = import('windows')
subdir('glib')
subdir('gobject')
subdir('gthread')
subdir('gmodule')
subdir('gio')
subdir('fuzzing')
subdir('tests')

# xgettext is optional (on Windows for instance)
if find_program('xgettext', required : get_option('nls')).found()
  subdir('po')
endif

# Install glib-gettextize executable, if a UNIX-style shell is found
if have_sh
  # These should not contain " quotes around the values
  gettextize_conf = configuration_data()
  gettextize_conf.set('PACKAGE', 'glib')
  gettextize_conf.set('VERSION', meson.project_version())
  gettextize_conf.set('prefix', glib_prefix)
  gettextize_conf.set('datarootdir', glib_datadir)
  gettextize_conf.set('datadir', glib_datadir)
  configure_file(input : 'glib-gettextize.in',
    install : true,
    install_dir : glib_bindir,
    output : 'glib-gettextize',
    configuration : gettextize_conf)
endif

# Install m4 macros that other projects use
install_data('m4macros/glib-2.0.m4', 'm4macros/glib-gettext.m4', 'm4macros/gsettings.m4',
  install_dir : join_paths(get_option('datadir'), 'aclocal'))

if host_system != 'windows'
  # Install Valgrind suppression file (except on Windows,
  # as Valgrind is currently not supported on Windows)
  install_data('glib.supp',
    install_dir : join_paths(get_option('datadir'), 'glib-2.0', 'valgrind'))
endif

configure_file(output : 'config.h', configuration : glib_conf)

if host_system == 'windows'
  install_headers([ 'msvc_recommended_pragmas.h' ], subdir : 'glib-2.0')
endif

if get_option('man')
  xsltproc = find_program('xsltproc', required : true)
  xsltproc_command = [
    xsltproc,
    '--nonet',
    '--stringparam', 'man.output.quietly', '1',
    '--stringparam', 'funcsynopsis.style', 'ansi',
    '--stringparam', 'man.th.extra1.suppress', '1',
    '--stringparam', 'man.authors.section.enabled', '0',
    '--stringparam', 'man.copyright.section.enabled', '0',
    '-o', '@OUTPUT@',
    'http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl',
    '@INPUT@',
  ]
  man1_dir = get_option('mandir') + '/man1'
endif

gnome = import('gnome')
subdir('docs/reference/glib')
subdir('docs/reference/gobject')
subdir('docs/reference/gio')
