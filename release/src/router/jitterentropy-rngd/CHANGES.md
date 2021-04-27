1.2.2:
 * enhancement: Add SP800-90B compliant entropy injection
 * fix: proper use of the RNDRESEEDCRNG IOCTL which otherwise causes an
   endless loop due to kernel change 11a0b5e0ec8c13bef06f7414f9e914506140d5cb
 * enhancement: Catch runtime FIPS health failures
 * enhancement: use Jitter RNG library 3.0.2

1.2.1:
  * on older GCC versions use -fstack-protector as suggested by Warszawski,
   Diego
 * prevent creating the internal timer thread if a high-res hardware timer is
   found as reported by Lonnie Abelbeck
 * disable RNDRESEEDCRNG on kernels < 4.17 as suggested by Warszawski, Diego
 * Use Jitter RNG library 3.0.1

1.2.0:
 * Due to the removal of the blocking pool in kernel 5.6, it is becoming
   very unlikely that the user space rngd is ever triggered by the kernel.
   Thus, the jitterentropy-rngd now injects entropy every 10 minutes
   unconditionally.
 * Use the RNDRESEEDCRNG ioctl after injecting entropy to guarantee that
   the new entropy is immediately forwarded to the ChaCha20 DRNG. Otherwise
   the ChaCha20 DRNG will not benefit from the new entropy up to 5 minutes
   after the injection of the entropy.
 * Use Jitter RNG library 3.0.0

1.1.0:
 * avert crash during shutdown when the kernel sends a SIGALRM while the
   Jitter RNG is deallocated
 * Fix: unsafe signal handling by Gerald Lledo
 * import jitterentropy library 2.2.0 to make rngd fully SP800-90B compliant

1.0.8:
 * Fix incomplete jitterentropy core 2.1.1 import

1.0.7:
 * Inlcude jitterentropy core 2.1.1

1.0.6:
 * Include jitterentropy core 2.0.1
 * Compile jitterentropy core without optimizations using GCC pragmas instead
   of -O0 as suggested by Paul Wouters
 * Change CFLAGS and LDFLAGS from += to ?= to allow smooth integration with
   build environment as suggested by Paul Wouters
 * Version information now can obtained as unprivileged user.

1.0.5:
 * inject 32 bytes of entropy into /dev/random before daemonizing as suggested
   by Pascal de Bruijn
 * add jitterentropy-rngd.1 man page as suggested by Pascal de Bruijn
 * small changes to systemd unit file suggested by Pascal de Bruijn

1.0.4:
 * inject only 32 bytes of entropy of entropy instead of 256 bytes
 * apply oversampling factor -- i.e. obtain OVERSAMPLINGFACTOR bytes more from
   Jitter RNG than required for the 32 bytes of entropic data
 * do not install sig_alarm handler if the LRNG is present
 * Use Jitter RNG logic v2.0.0

1.0.3:
 * Ensure that the memset on the buffer holding entropy is always performed.

1.0.2:
 * change jitterentropy.service: move RNGd startup up the boot ladder
   to allow all cryptographic services to benefit from a RNGd-updated
   /dev/?random

1.0.1:
 * mark function jentrng_versionstring static (thanks to Kevin Fowler)
 * use errno with strerror (thanks to Kevin Fowler)
 * compile with -pedanic and make appropriate code changes

1.0.0:
 * start new numbering schema

