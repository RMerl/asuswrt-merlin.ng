/**
 ** Simple entropy harvester based upon the havege RNG
 **
 ** Copyright 2018-2022 Jirka Hladky hladky DOT jiri AT gmail DOT com
 ** Copyright 2009-2014 Gary Wuertz gary@issiweb.com
 ** Copyright 2011-2012 BenEleventh Consulting manolson@beneleventh.com
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "config.h"
#if defined(HAVE_SYS_AUXV_H)
#include <sys/auxv.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#ifndef NO_DAEMON
#include <syslog.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/random.h>
#endif

#ifndef NO_COMMAND_MODE
#include "havegecmd.h"
#include <limits.h>
#endif

#include <errno.h>
#include "haveged.h"
#include "havegecollect.h"
/**
 * stringize operators for maintainable text
 */
#define STRZ(a) #a
#define SETTINGL(msg,val) STRZ(val) msg
#define SETTINGR(msg,val) msg STRZ(val)

// {{{ VERSION_TEXT
static const char* VERSION_TEXT =
  "haveged %s\n\n"
  "Copyright (C) 2018-2022 Jirka Hladky <hladky.jiri@gmail.com>\n"
  "Copyright (C) 2009-2014 Gary Wuertz <gary@issiweb.com>\n"
  "Copyright (C) 2011-2012 BenEleventh Consulting <manolson@beneleventh.com>\n\n"
  "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n"
  "This is free software: you are free to change and redistribute it.\n"
  "There is NO WARRANTY, to the extent permitted by law.\n";
// }}} VERSION_TEXT

/**
 * Parameters
 */
static struct pparams defaults = {
  .daemon         = PACKAGE,
  .exit_code      = 1,
  .setup          = 0,
  .ncores         = 0,
  .buffersz       = 0,
  .detached       = 0,
  .foreground     = 0,
  .once           = 0,
  .d_cache        = 0,
  .i_cache        = 0,
  .run_level      = 0,
  .low_water      = 0,
  .tests_config   = 0,
  .os_rel         = "/proc/sys/kernel/osrelease",
  .pid_file       = PID_DEFAULT,
  .poolsize       = "/proc/sys/kernel/random/poolsize",
  .random_device  = "/dev/random",
  .sample_in      = INPUT_DEFAULT,
  .sample_out     = OUTPUT_DEFAULT,
  .verbose        = 0,
  .watermark      = "/proc/sys/kernel/random/write_wakeup_threshold",
  .command        = 0
  };
struct pparams *params = &defaults;

#ifdef  RAW_IN_ENABLE
FILE *fd_in;
/**
 * The injection diagnostic
 */
static int injectFile(volatile H_UINT *pData, H_UINT szData);
#endif
/**
 * havege instance used by application
 */
static H_PTR handle = NULL;
/**
 * Local prototypes
 */
#ifndef NO_DAEMON
static int poolSize = 0;

static void daemonize(void);
static int  get_poolsize(void);
static void run_daemon(H_PTR handle, const volatile char *path, char *const argv[]);
static void set_watermark(int level);
#endif

static void anchor_info(H_PTR h);
static int  get_runsize(unsigned int *bufct, unsigned int *bufrem, char *bp);
static char *ppSize(char *buffer, double sz);

static void run_app(H_PTR handle, H_UINT bufct, H_UINT bufres);
static void show_meterInfo(H_UINT id, H_UINT event);
static void tidy_exit(int signum);
static void usage(int db, int nopts, struct option *long_options, const char **cmds);

static sigset_t mask, omask;

#define  ATOU(a)     (unsigned int)atoi(a)
/**
 * Entry point
 */
int main(int argc, char **argv)
{
   volatile char *path = strdup(argv[0]);
   volatile char *arg0 = argv[0];
#if defined(HAVE_SYS_AUXV_H)
   if (path[0] != '/')
      path = (char*)getauxval(AT_EXECFN);
#endif
   static const char* cmds[] = {
      "b", "buffer",      "1", SETTINGR("Buffer size [KW], default: ",COLLECT_BUFSIZE),
      "d", "data",        "1", SETTINGR("Data cache size [KB], with fallback to: ", GENERIC_DCACHE ),
#ifndef NO_COMMAND_MODE
      "c", "command",     "1", "Send a command mode to an already running haveged",
#endif
      "i", "inst",        "1", SETTINGR("Instruction cache size [KB], with fallback to: ", GENERIC_ICACHE),
      "f", "file",        "1", "Sample output file,  default: '" OUTPUT_DEFAULT "', '-' for stdout",
      "F", "Foreground",  "0", "Run daemon in foreground",
      "e", "once",        "0", "Provide entropy to the kernel once and quit immediatelly",
      "r", "run",         "1", "0=daemon, 1=config info, >1=<r>KB sample",
      "n", "number",      "1", "Output size in [k|m|g|t] bytes, 0 = unlimited to stdout",
      "o", "onlinetest",  "1", "[t<x>][c<x>] x=[a[n][w]][b[w]] 't'ot, 'c'ontinuous, default: ta8b",
      "p", "pidfile",     "1", "daemon pidfile, default: " PID_DEFAULT ,
      "s", "source",      "1", "Injection source file, default: '" INPUT_DEFAULT "', '-' for stdin",
#if  NUMBER_CORES>1
      "t", "threads",     "1", "Number of threads",
#endif
      "v", "verbose",     "1", "Verbose mask 0=none,1=summary,2=retries,4=timing,8=loop,16=code,32=test,64=RNDADDENTROPY",
      "w", "write",       "1", "Set write_wakeup_threshold [bits]",
      "V", "version",     "0", "Print version information and exit",
      "h", "help",        "0", "This help"
      };
   static int nopts = sizeof(cmds)/(4*sizeof(char *));
   struct option long_options[nopts+1];
   char short_options[1+nopts*2];
   int c,i,j;
   H_UINT bufct, bufrem, ierr;
   H_PARAMS cmd;

   if (havege_version(HAVEGE_PREP_VERSION)==NULL)
      error_exit("version conflict %s!=%s", HAVEGE_PREP_VERSION, havege_version(NULL));
#if NO_DAEMON==1
   params->setup |= RUN_AS_APP;
#endif
#ifdef  RAW_IN_ENABLE
#define DIAG_USAGE2 SETTINGL("=inject ticks,", DIAG_RUN_INJECT)\
  SETTINGL("=inject data", DIAG_RUN_TEST)

   params->setup |= INJECT | RUN_AS_APP;
#else
#define DIAG_USAGE2 ""
#endif
#ifdef  RAW_OUT_ENABLE
#define DIAG_USAGE1 SETTINGL("=capture,", DIAG_RUN_CAPTURE)

   params->setup |= CAPTURE | RUN_AS_APP;
#else
#define DIAG_USAGE1 ""
#endif
#if NUMBER_CORES>1
   params->setup |= MULTI_CORE;
#endif

#ifndef NO_COMMAND_MODE
   first_byte = arg0[0];
#endif
   if (access("/etc/initrd-release", F_OK) >= 0) {
      arg0[0] = '@';
      path[0] = '/';
      }
#ifdef SIGHUP
   signal(SIGHUP, tidy_exit);
#endif
   signal(SIGINT, tidy_exit);
   signal(SIGTERM, tidy_exit);
#ifndef NO_COMMAND_MODE
   signal(SIGPIPE, SIG_IGN);
#endif

   sigemptyset(&mask);
#ifdef SIGHUP
   sigaddset(&mask, SIGHUP);
#endif
   sigaddset(&mask, SIGINT);
   sigaddset(&mask, SIGTERM);
#ifndef NO_COMMAND_MODE
   sigaddset(&mask, SIGPIPE);
#endif
#if  NUMBER_CORES>1
   /* Hmmm ... currently the code  does not use pthread_create(3) but fork(2) */
   pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
#else
   sigprocmask(SIG_UNBLOCK, &mask, NULL);
#endif
   strcpy(short_options,"");
   bufct  = bufrem = 0;
  /**
   * Build options
   */
   for(i=j=0;j<(nopts*4);j+=4) {
      switch(cmds[j][0]) {
         case 'o':
#ifdef  ONLINE_TESTS_ENABLE
            break;
#else
            continue;
#endif
         case 'r':
#if defined(RAW_IN_ENABLE) || defined (RAW_OUT_ENABLE)
            if (0!=(params->setup & (INJECT|CAPTURE))) {
              params->daemon = "havege_diagnostic";
              cmds[j+3] = "run level, 0=diagnostic off,1=config info," DIAG_USAGE1 DIAG_USAGE2 ;
              }
            else
#endif
            if (0!=(params->setup & RUN_AS_APP))
               continue;
            break;
         case 's':
            if (0 == (params->setup & INJECT))
               continue;
            break;
         case 't':
            if (0 == (params->setup & MULTI_CORE))
               continue;
            break;
         case 'p':   case 'w':  case 'F':
            if (0 !=(params->setup & RUN_AS_APP))
               continue;
            break;
         }
      long_options[i].name      = cmds[j+1];
      long_options[i].has_arg   = atoi(cmds[j+2]);
      long_options[i].flag      = NULL;
      long_options[i].val       = cmds[j][0];
      strcat(short_options,cmds[j]);
      if (long_options[i].has_arg!=0) strcat(short_options,":");
      i += 1;
      }
   memset(&long_options[i], 0, sizeof(struct option));

   do {
      c = getopt_long (argc, argv, short_options, long_options, NULL);
      switch(c) {
         case 'F':
            params->setup |= RUN_IN_FG;
            params->foreground = 1;
            break;
         case 'e':
            params->setup |= RUN_ONCE;
            params->once = 1;
            break;
         case 'b':
            params->buffersz = ATOU(optarg) * 1024;
            if (params->buffersz<4)
               error_exit("invalid size %s", optarg);
            break;
#ifndef NO_COMMAND_MODE
         case 'c':
            params->command = optarg;
            params->setup |= CMD_MODE;
            break;
#endif
         case 'd':
            params->d_cache = ATOU(optarg);
            break;
         case 'i':
            params->i_cache = ATOU(optarg);
            break;
         case 'f':
            params->sample_out = optarg;
            if (strcmp(optarg,"-") == 0 )
               params->setup |= USE_STDOUT;
            break;
         case 'n':
            if (get_runsize(&bufct, &bufrem, optarg))
               error_exit("invalid count: %s", optarg);
            params->setup |= RUN_AS_APP|RANGE_SPEC;
            if (bufct==0 && bufrem==0)
               params->setup |= USE_STDOUT;             /* ugly but documented behavior! */
            break;
         case 'o':
            params->tests_config = optarg;
            break;
         case 'p':
            params->pid_file = optarg;
            break;
         case 'r':
            params->run_level  = ATOU(optarg);
            if (params->run_level != 0)
               params->setup |= RUN_AS_APP;
            break;
         case 's':
            params->sample_in = optarg;
            break;
         case 't':
            params->ncores = ATOU(optarg);
            if (params->ncores > NUMBER_CORES)
               error_exit("invalid thread count: %s", optarg);
            break;
         case 'v':
            params->verbose  = ATOU(optarg);
            break;
         case 'w':
            params->setup |= SET_LWM;
            params->low_water = ATOU(optarg);
            break;
         case '?':
         case 'h':
            usage(0, nopts, long_options, cmds);
            /* fallthrough */
         case 'V':
            printf(VERSION_TEXT, HAVEGE_PREP_VERSION);
            exit(EXIT_SUCCESS);
         case -1:
            break;
         }
      } while (c!=-1);
#ifndef NO_COMMAND_MODE
   if (params->setup & CMD_MODE) {
      int ret = 0, len;
      uint32_t size;
      char *ptr, answer[6], cmd[2];
      fd_set read_fd;
      sigset_t block;

      socket_fd = cmd_connect(params);
      if (socket_fd < 0) {
         ret = -1;
         goto err;
         }
      cmd[0] = getcmd(params->command);
      if (cmd[0] < 0) {
         ret = -1;
         goto err;
         }
      cmd[1] = '\0';
      switch (cmd[0]) {
         char *root;
         case MAGIC_CHROOT:
            root = optarg;
            size = (uint32_t)strlen(root)+1;
            cmd[1] = '\002';
            safeout(socket_fd, &cmd[0], 2);
            send_uinteger(socket_fd, size);
            safeout(socket_fd, root, size);
            break;
         case MAGIC_CLOSE:
            ptr = &cmd[0];
            len = (int)strlen(cmd)+1;
            safeout(socket_fd, ptr, len);
            break;
         case '?':
         default:
            ret = -1;
            break;
         }
      answer[0] = '\0';
      ptr = &answer[0];

      sigemptyset(&block);
      sigaddset(&block, SIGPIPE);

      FD_ZERO(&read_fd);
      FD_SET(socket_fd, &read_fd);

      do {
         struct timeval two = {6, 0};
         ret = select(socket_fd+1, &read_fd, NULL, NULL, &two);
         if (ret >= 0) break;
         if (errno != EINTR)
            error_exit("Select error: %s", strerror(errno));
         }
      while (1);

      ret = safein(socket_fd, ptr, 1);
      if (ret < 0)
         goto err;
      switch (answer[0]) {
         case '\002': {
               char *msg;
               ret = receive_uinteger(socket_fd, &size);
               if (ret < 0)
                  goto err;		   
               msg = calloc(size, sizeof(char));
               if (!msg)
                  error_exit("can not allocate memory for message from UNIX socket: %s",
                              strerror(errno));
               ret = safein(socket_fd, msg, size);
               if (ret < 0)
                  goto err;
               fprintf(stderr, "%s: %s", params->daemon, msg);
               free(msg);
               ret = -1;
            }
            break;
         case '\x6':
            ret = 0;
            break;
         case '\x15':
         default:
            ret = -1;
            break;
         }
   err:
      close(socket_fd);
      return ret;
      }
   else if (!(params->setup & RUN_AS_APP)){
      socket_fd = cmd_listen(params);
      if (socket_fd >= 0)
         fprintf(stderr, "%s: command socket is listening at fd %d\n", params->daemon, socket_fd);
      else {
         if (socket_fd == -2) {
            fprintf(stderr, "%s: command socket already in use\n", params->daemon);
            error_exit("%s: please check if there is another instance of haveged running\n", params->daemon);
         } else {
            fprintf(stderr, "%s: can not initialize command socket: %s\n", params->daemon, strerror(errno));
            fprintf(stderr, "%s: disabling command mode for this instance\n", params->daemon);
         }
      }
    }
#endif
   if (params->tests_config == 0)
     params->tests_config = (0 != (params->setup & RUN_AS_APP))? TESTS_DEFAULT_APP : TESTS_DEFAULT_RUN;
   memset(&cmd, 0, sizeof(H_PARAMS));
   cmd.collectSize = params->buffersz;
   cmd.icacheSize  = params->i_cache;
   cmd.dcacheSize  = params->d_cache;
   cmd.options     = params->verbose & 0xff;
   cmd.nCores      = params->ncores;
   cmd.testSpec    = params->tests_config;
   cmd.msg_out     = print_msg;
   if (0 != (params->setup & RUN_AS_APP)) {
      cmd.ioSz = APP_BUFF_SIZE * sizeof(H_UINT);
      if (params->verbose!=0 && 0==(params->setup & RANGE_SPEC))
         params->run_level = 1;
      }
#ifndef NO_DAEMON
   else  {
      poolSize = get_poolsize();
      i = (poolSize + 7)/8 * sizeof(H_UINT);
      cmd.ioSz = sizeof(struct rand_pool_info) + i *sizeof(H_UINT);
      }
#endif
   if (0 != (params->verbose & H_DEBUG_TIME))
      cmd.metering = show_meterInfo;

   if (0 !=(params->setup & CAPTURE) && 0 != (params->run_level == DIAG_RUN_CAPTURE))
      cmd.options |= H_DEBUG_RAW_OUT;
#ifdef  RAW_IN_ENABLE
   if (0 !=(params->setup & INJECT) && 0 != (params->run_level & (DIAG_RUN_INJECT|DIAG_RUN_TEST))) {
      if (strcmp(params->sample_in,"-") == 0 )
        fd_in = stdin;
      else fd_in = fopen(params->sample_in, "rb");
      if (NULL == fd_in)
         error_exit("Unable to open: %s", params->sample_in);
      cmd.injection = injectFile;
      if (params->run_level==DIAG_RUN_INJECT)
         cmd.options |= H_DEBUG_RAW_IN;
      else if (params->run_level==DIAG_RUN_TEST)
         cmd.options |= H_DEBUG_TEST_IN;
      else usage(1, nopts, long_options, cmds);
      }
#endif
   handle = havege_create(&cmd);
   ierr = handle==NULL? H_NOHANDLE : handle->error;
   switch(ierr) {
      case H_NOERR:
         break;
      case H_NOTESTSPEC:
         error_exit("unrecognized test setup: %s", cmd.testSpec);
         break;
      default:
         error_exit("Couldn't initialize haveged (%d)", ierr);
      }
   if (0 != (params->setup & RUN_AS_APP)) {
      if (params->run_level==1)
        anchor_info(handle);
      else if (0==(params->setup&(INJECT|CAPTURE))) {
        /* must specify range with --number or --run > 1 but not both */
        if (params->run_level>1) {
          if (0==(params->setup&RANGE_SPEC)) {        /* --run specified    */
            bufct  = params->run_level/sizeof(H_UINT);
            bufrem = (params->run_level%sizeof(H_UINT))*1024;
            }
          else  usage(2, nopts, long_options, cmds);  /* both specified     */
          }
        else if (0==(params->setup&RANGE_SPEC))
          usage(3,nopts, long_options, cmds);        /* neither specified  */
        else if (0==(params->setup&USE_STDOUT)&&(bufct+bufrem)==0)
          usage(4, nopts, long_options, cmds);       /* only with stdout   */
        run_app(handle, bufct, bufrem);
        }
      else if (0==(params->setup&USE_STDOUT)&&(bufct+bufrem)==0)
        usage(5, nopts, long_options, cmds);       /* only with stdout   */
      else run_app(handle, bufct, bufrem);
      }
#ifndef NO_DAEMON
   else run_daemon(handle, path, argv);
#endif
   havege_destroy(handle);
   exit(0);
}
#ifndef NO_DAEMON
/**
 * The usual daemon setup
 */
static void daemonize(     /* RETURN: nothing   */
   void)                   /* IN: nothing       */
{
   FILE *fh;
   openlog(params->daemon, LOG_CONS, LOG_DAEMON);
   syslog(LOG_NOTICE, "%s starting up", params->daemon);
   if (daemon(0, 0) == -1)
      error_exit("Cannot fork into the background");
   fh = fopen(params->pid_file, "w");
   if (!fh)
      error_exit("Couldn't open PID file \"%s\" for writing: %s.", params->pid_file, strerror(errno));
   fprintf(fh, "%i", getpid());
   fclose(fh);
   params->detached = 1;
}
/**
 * Get configured poolsize
 */
static int get_poolsize(   /* RETURN: number of bits  */
   void)                   /* IN: nothing             */
{
   FILE *poolsize_fh,*osrel_fh;
   unsigned int major,minor;
   int max_bits;

   poolsize_fh = fopen(params->poolsize, "rb");
   if (poolsize_fh) {
      if (fscanf(poolsize_fh, "%d", &max_bits)!=1)
         max_bits = -1;
      fclose(poolsize_fh);
      osrel_fh = fopen(params->os_rel, "rb");
      if (osrel_fh) {
         if (fscanf(osrel_fh,"%u.%u", &major, &minor)<2)
           major = minor = 0;
         fclose(osrel_fh);
         if (major==2 && minor==4) max_bits *= 8;
         }
      }
   else max_bits = -1;
   if (max_bits < 1)
      error_exit("Couldn't get poolsize");
   return max_bits;
}
/**
 * Run as a daemon writing to random device entropy pool
 */
static void run_daemon(    /* RETURN: nothing   */
   H_PTR h,                /* IN: app instance  */
   const volatile char *path,
   char *const argv[])
{
   int                     random_fd = -1;
#ifndef NO_COMMAND_MODE
   int                     conn_fd = -1;
#endif
   struct rand_pool_info   *output;
   struct stat stat_buf;
   time_t t[2];

   if (0 != params->run_level) {
      anchor_info(h);
      return;
      }
   if (params->foreground==0) {
     daemonize();
     havege_reparent(handle);
   }
   else printf ("%s starting up\n", params->daemon);
   if (0 != havege_run(h))
      error_exit("Couldn't initialize HAVEGE rng %d", h->error);
   if (0 != (params->verbose & H_DEBUG_INFO))
     anchor_info(h);
   if (params->low_water>0)
      set_watermark(params->low_water);
   if ( lstat(params->random_device, &stat_buf) != 0 )
      error_exit("lstat has failed for the random device \"%s\": %s", params->random_device, strerror(errno));
   if ( S_ISLNK(stat_buf.st_mode) )
      error_exit("random device \"%s\" is a link. This is not supported for the security reasons.", params->random_device);
   random_fd = open(params->random_device, O_RDWR);
   if (random_fd == -1)
     error_exit("Couldn't open random device: %s", strerror(errno));

   output = (struct rand_pool_info *) h->io_buf;

#if  NUMBER_CORES>1
   pthread_sigmask(SIG_BLOCK, &mask, &omask);
#else
   sigprocmask(SIG_BLOCK, &mask, &omask);
#endif


   t[0] = 0;
   for(;;) {
      int current,nbytes,r,max=0;
      H_UINT fills;
      char buf[120];
      fd_set write_fd;
#ifndef NO_COMMAND_MODE
      fd_set read_fd;
#endif

      if (params->exit_code > 128)
         error_exit("Stopping due to signal %d\n", params->exit_code - 128);

      t[1] = time(NULL);
      if (t[1] - t[0] > 600) {
        /* add entropy on daemon start and then every 600 seconds unconditionally */
        nbytes = poolSize;
        r = (nbytes+sizeof(H_UINT)-1)/sizeof(H_UINT);
        fills = h->n_fills;
        if (havege_rng(h, (H_UINT *)output->buf, r)<1)
          error_exit("RNG failed! %d", h->error);
        output->buf_size = nbytes;
        /* entropy is 8 bits per byte */
        output->entropy_count = nbytes * 8;
        if (ioctl(random_fd, RNDADDENTROPY, output) == -1)
          error_exit("RNDADDENTROPY failed!");
        h->n_entropy_bytes += nbytes;
        if (params->once == 1) {
          params->exit_code = 0;
          error_exit("Entropy refilled once (%d bytes), exiting.", nbytes);
        }
        if (0 != (params->verbose & H_RNDADDENTROPY_INFO) && h->n_fills > fills) {
          if (havege_status_dump(h, H_SD_TOPIC_SUM, buf, sizeof(buf))>0)
            print_msg("%s\n", buf);
        }
        t[0] = t[1];
        continue;
      }

      FD_ZERO(&write_fd);
#ifndef NO_COMMAND_MODE
      if (socket_fd >= 0) {
        FD_ZERO(&read_fd);
      }
#endif
      FD_SET(random_fd, &write_fd);
      if (random_fd > max)
         max = random_fd;
#ifndef NO_COMMAND_MODE
      if (socket_fd >= 0) {
        FD_SET(socket_fd, &read_fd);
        if (socket_fd > max)
           max = socket_fd;
        if (conn_fd >= 0) {
           FD_SET(conn_fd, &read_fd);
           if (conn_fd > max)
              max = conn_fd;
           }
       }
#endif
      for(;;)  {
         struct timespec two = {2, 0};
         int rc;
#ifndef NO_COMMAND_MODE
         if (socket_fd >= 0) {
           rc = pselect(max+1, &read_fd, &write_fd, NULL, &two, &omask);
         } else {
           rc = pselect(max+1, NULL, &write_fd, NULL, &two, &omask);
         }
#else
         rc = pselect(max+1, NULL, &write_fd, NULL, &two, &omask);
#endif
         if (rc >= 0) break;
         if (params->exit_code > 128)
            break;
         if (errno != EINTR)
            error_exit("Select error: %s", strerror(errno));
         }
      if (params->exit_code > 128)
         continue;

#ifndef NO_COMMAND_MODE
      if ( socket_fd >= 0 && FD_ISSET(socket_fd, &read_fd) && conn_fd < 0) {
# ifdef HAVE_ACCEPT4
         conn_fd = accept4(socket_fd, NULL, NULL, SOCK_CLOEXEC|SOCK_NONBLOCK);
         if (conn_fd < 0 && (errno == ENOSYS || errno == ENOTSUP)) {
# endif
            conn_fd = accept(socket_fd, NULL, NULL);
            if (conn_fd >= 0) {
               fcntl(conn_fd, F_SETFL, O_NONBLOCK);
               fcntl(conn_fd, F_SETFD, FD_CLOEXEC);
               }
# ifdef HAVE_ACCEPT4
            }
# endif
         if (conn_fd >= 0)
            continue;
         }

      if (conn_fd >= 0 && FD_ISSET(conn_fd, &read_fd))
         conn_fd = socket_handler(conn_fd, path, argv, params);
#endif
      if (!FD_ISSET(random_fd, &write_fd))
         continue;

      if (ioctl(random_fd, RNDGETENTCNT, &current) == -1)
         error_exit("Couldn't query entropy-level from kernel");
      /* get number of bytes needed to fill pool */
      nbytes = (poolSize - current + 7)/8;
      if(nbytes<1)   continue;
      /* get that many random bytes */
      r = (nbytes+sizeof(H_UINT)-1)/sizeof(H_UINT);
      fills = h->n_fills;
      if (havege_rng(h, (H_UINT *)output->buf, r)<1)
         error_exit("RNG failed! %d", h->error);
      output->buf_size = nbytes;
      /* entropy is 8 bits per byte */
      output->entropy_count = nbytes * 8;
      t[0] = t[1];
      if (ioctl(random_fd, RNDADDENTROPY, output) == -1)
         error_exit("RNDADDENTROPY failed!");
      h->n_entropy_bytes += nbytes;
      if (0 != (params->verbose & H_RNDADDENTROPY_INFO) && h->n_fills > fills) {
        if (havege_status_dump(h, H_SD_TOPIC_SUM, buf, sizeof(buf))>0)
          print_msg("%s\n", buf);
      }
      }
}
/**
 * Set random write threshold
 */
static void set_watermark( /* RETURN: nothing   */
   int level)              /* IN: threshold     */
{
   FILE *wm_fh;

   if ( level > (poolSize - 32))
      level = poolSize - 32;
   wm_fh = fopen(params->watermark, "w");
   if (wm_fh) {
      fprintf(wm_fh, "%d\n", level);
      fclose(wm_fh);
      }
   else if (errno == EACCES)
       fprintf(stderr, "No access to %s, can't set watermark (maybe running in a container?)\n",
               params->watermark);
   else error_exit("Fail:set_watermark()!");
}
#endif
/**
 * Display handle information
 */
static void anchor_info(H_PTR h)
{
   char       buf[120];
   H_SD_TOPIC topics[4] = {H_SD_TOPIC_BUILD, H_SD_TOPIC_TUNE, H_SD_TOPIC_TEST, H_SD_TOPIC_SUM};
   int        i;

   for(i=0;i<4;i++)
      if (havege_status_dump(h, topics[i], buf, sizeof(buf))>0)
         print_msg("%s\n", buf);
}
/**
 * Bail....
 */
void error_exit(           /* RETURN: nothing   */
   const char *format,     /* IN: msg format    */
   ...)                    /* IN: varadic args  */
{
   char buffer[4096];

   va_list ap;
   va_start(ap, format);
   vsnprintf(buffer, sizeof(buffer), format, ap);
   va_end(ap);
#ifndef NO_DAEMON
   if (params->detached!=0) {
      unlink(params->pid_file);
      syslog(LOG_INFO, "%s: %s", params->daemon, buffer);
      }
   else
#endif
   {
   fprintf(stderr, "%s: %s\n", params->daemon, buffer);
   if (0 !=(params->setup & (RUN_AS_APP | RUN_IN_FG) ) && 0 != handle) {
      if (havege_status_dump(handle, H_SD_TOPIC_TEST, buffer, sizeof(buffer))>0)
         fprintf(stderr, "%s\n", buffer);
      if (havege_status_dump(handle, H_SD_TOPIC_SUM, buffer, sizeof(buffer))>0)
         fprintf(stderr, "%s\n", buffer);
      }
   }
   havege_destroy(handle);
   exit(params->exit_code);
}
/**
 * Implement fixed point shorthand for run sizes
 */
static int get_runsize(    /* RETURN: the size        */
   H_UINT *bufct,          /* OUT: nbr app buffers    */
   H_UINT *bufrem,         /* OUT: residue            */
   char *bp)               /* IN: the specification   */
{
   char        *suffix;
   double      f;
   int         p2 = 0;
   int         p10 = APP_BUFF_SIZE * sizeof(H_UINT);
   long long   ct;


   f = strtod(bp, &suffix);
   if (f < 0 || strlen(suffix)>1)
      return 1;
   switch(*suffix) {
      case 't': case 'T':
         p2 += 1;
         /* fallthrough */
      case 'g': case 'G':
         p2 += 1;
         /* fallthrough */
      case 'm': case 'M':
         p2 += 1;
         /* fallthrough */
      case 'k': case 'K':
         p2 += 1;
         /* fallthrough */
      case 0:
         break;
      default:
         return 2;
      }
   while(p2-- > 0)
      f *= 1024;
   ct = f;
   if (f != 0 && ct==0)
      return 3;
   if ((double) (ct+1) < f)
      return 3;
   *bufrem = (H_UINT)(ct%p10);
   *bufct  = (H_UINT)(ct/p10);
   if (*bufct == (ct/p10))
      return 0;
   /* hack to allow 16t */
   ct -= 1;
   *bufrem = (H_UINT)(ct%p10);
   *bufct  = (H_UINT)(ct/p10);
   return (*bufct == (ct/p10))? 0 : 4;
}
#ifdef  RAW_IN_ENABLE
/**
 * The injection diagnostic
 */
static int injectFile(     /* RETURN: not used  */
   volatile H_UINT *pData, /* OUT: data buffer  */
   H_UINT szData)          /* IN: H_UINT needed  */
{
   int r;
   if ((r=fread((void *)pData, sizeof(H_UINT), szData, fd_in)) != szData)
      error_exit("Cannot read data in file: %d!=%d", r, szData);
   return 0;
}
#endif
/**
 * Pretty print the collection size
 */
static char *ppSize(       /* RETURN: the formatted size */
   char *buffer,           /* IN: work space             */
   double sz)              /* IN: the size               */
{
   char   units[] = {'T', 'G', 'M', 'K', 0};
   double factor  = 1024.0 * 1024.0 * 1024.0 * 1024.0;
   int i;

   for (i=0;0 != units[i];i++) {
      if (sz >= factor)
         break;
      factor /= 1024.0;
      }
   snprintf(buffer, 32, "%.4g %c byte", sz / factor, units[i]);
   return buffer;
}
/**
 * Execution notices - to stderr or syslog
 */
void print_msg(            /* RETURN: nothing   */
   const char *format,     /* IN: format string */
   ...)                    /* IN: args          */
{
   char buffer[128];

   va_list ap;
   va_start(ap, format);
   snprintf(buffer, sizeof(buffer), "%s: %s", params->daemon, format);
#ifndef NO_DAEMON
   if (params->detached != 0)
      vsyslog(LOG_INFO, buffer, ap);
   else
#endif
   vfprintf(stderr, buffer, ap);
   va_end(ap);
}
/**
* Run as application writing to a file
*/
static void run_app(       /* RETURN: nothing         */
   H_PTR h,                /* IN: app instance        */
   H_UINT bufct,           /* IN: # buffers to fill   */
   H_UINT bufres)          /* IN: # bytes extra       */
{
   H_UINT   *buffer;
   FILE     *fout = NULL;
   H_UINT    ct=0;
   int       limits = bufct;

   if (0 != havege_run(h))
      error_exit("Couldn't initialize HAVEGE rng %d", h->error);
   if (0 != (params->setup & USE_STDOUT)) {
      params->sample_out = "stdout";
      fout = stdout;
      }
   else if (!(fout = fopen (params->sample_out, "wb")))
      error_exit("Cannot open file <%s> for writing.\n", params->sample_out);
   limits = bufct!=0? 1 : bufres != 0;
   buffer = (H_UINT *)h->io_buf;
#ifdef RAW_IN_ENABLE
   {
      char *format, *in="",*out,*sz,*src="";

      if (params->run_level==DIAG_RUN_INJECT)
         in = "tics";
      else if (params->run_level==DIAG_RUN_TEST)
         in = "data";
      if (*in!=0) {
         src =(fd_in==stdin)? "stdin" : params->sample_in;
         format = "Inject %s from %s, writing %s bytes to %s\n";
         }
      else format = "Writing %s%s%s bytes to %s\n";
      if (limits)
         sz = ppSize((char *)buffer, (1.0 * bufct) * APP_BUFF_SIZE * sizeof(H_UINT) + bufres);
      else sz = "unlimited";
      out = (fout==stdout)? "stdout" : params->sample_out;
      fprintf(stderr, format, in, src, sz, out);
   }
#else
   if (limits)
      fprintf(stderr, "Writing %s output to %s\n",
         ppSize((char *)buffer, (1.0 * bufct) * APP_BUFF_SIZE * sizeof(H_UINT) + bufres), params->sample_out);
   else fprintf(stderr, "Writing unlimited bytes to stdout\n");
#endif
   while(!limits || ct++ < bufct) {
      if (havege_rng(h, buffer, APP_BUFF_SIZE)<1)
         error_exit("RNG failed %d!", h->error);
      if (fwrite (buffer, 1, APP_BUFF_SIZE * sizeof(H_UINT), fout) == 0)
         error_exit("Cannot write data in file: %s", strerror(errno));
   }
   ct = (bufres + sizeof(H_UINT) - 1)/sizeof(H_UINT);
   if (ct) {
      if (havege_rng(h, buffer, ct)<1)
         error_exit("RNG failed %d!", h->error);
      if (fwrite (buffer, 1, bufres, fout) == 0)
         error_exit("Cannot write data in file: %s", strerror(errno));
      }
   fclose(fout);
   if (0 != (params->verbose & H_DEBUG_INFO))
      anchor_info(h);
}
/**
 * Show collection info.
 */
static void show_meterInfo(      /* RETURN: nothing   */
   H_UINT id,                    /* IN: identifier    */
   H_UINT event)                 /* IN: start/stop    */
{
   struct timeval tm;
   /* N.B. if multiple thread, each child gets its own copy of this */
   static H_METER status;

   gettimeofday(&tm, NULL);
   if (event == 0)
      status.estart = ((double)tm.tv_sec*1000.0 + (double)tm.tv_usec/1000.0);
   else {
      status.etime  = ((double)tm.tv_sec*1000.0 + (double)tm.tv_usec/1000.0);
      if ((status.etime -= status.estart)<0.0)
         status.etime=0.0;
      status.n_fill += 1;
      print_msg("%d fill %g ms\n", id, status.etime);
      }
}
/**
 * Signal handler
 */
static void tidy_exit(           /* OUT: nothing      */
   int signum)                   /* IN: signal number */
{
  params->exit_code = 128 + signum;
}

/**
 * send usage display to stderr
 */
static void usage(               /* OUT: nothing            */
   int loc,                      /* IN: debugging aid       */
   int nopts,                    /* IN: number of options   */
   struct option *long_options,  /* IN: long options        */
   const char **cmds)            /* IN: associated text     */
{
  int i, j;
  
  (void)loc;
  fprintf(stderr, "\nUsage: %s [options]\n\n", params->daemon);
#ifndef NO_DAEMON
  fprintf(stderr, "Collect entropy and feed into random pool or write to file.\n");
#else
  fprintf(stderr, "Collect entropy and write to file.\n");
#endif
  fprintf(stderr, "  Options:\n");
  for(i=j=0;long_options[i].val != 0;i++,j+=4) {
    while(cmds[j][0] != long_options[i].val && (j+4) < (nopts * 4))
      j += 4;
    fprintf(stderr,"     --%-10s, -%c %s %s\n",
      long_options[i].name, long_options[i].val,
      long_options[i].has_arg? "[]":"  ",cmds[j+3]);
    }
  fprintf(stderr, "\n");
  exit(1);
}
