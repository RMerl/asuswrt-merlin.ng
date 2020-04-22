# Fuzzing Dropbear

Dropbear is process-per-session so it assumes calling `dropbear_exit()`
is fine at any point to clean up. This makes fuzzing a bit trickier. 
A few pieces of wrapping infrastructure are used to work around this.

The [libfuzzer](http://llvm.org/docs/LibFuzzer.html#fuzz-target) harness
expects a long running process to continually run a test function with 
a string of crafted input. That process should not leak resources or exit.

## longjmp

When dropbear runs in fuzz mode it sets up a 
[`setjmp()`](http://man7.org/linux/man-pages/man3/setjmp.3.html) target prior 
to launching the code to be fuzzed, and then [`dropbear_exit()`](dbutil.c#L125)
calls `longjmp()` back there. This avoids exiting though it doesn't free 
memory or other resources.

## malloc Wrapper

Dropbear normally uses a [`m_malloc()`](dbmalloc.c) function that is the same as `malloc()` but
exits if allocation fails. In fuzzing mode this is replaced with a tracking allocator
that stores all allocations in a linked list. After the `longjmp()` occurs the fuzzer target
calls [`m_malloc_free_epoch(1, 1)`](dbmalloc.c) to clean up any unreleased memory.

If the fuzz target runs to completion it calls `m_malloc_free_epoch(1, 0)` which will reset 
the tracked allocations but will not free memory - that allows libfuzzer's leak checking
to detect leaks in normal operation.

## File Descriptor Input

As a network process Dropbear reads and writes from a socket. The wrappers for
`read()`/`write()`/`select()` in [fuzz-wrapfd.c](fuzz-wrapfd.c) will read from the
fuzzer input that has been set up with `wrapfd_add()`. `write()` output is
currently discarded.
These also test error paths such as EINTR and short reads with certain probabilities.

This allows running the entire dropbear server process with network input provided by the
fuzzer, without many modifications to the main code. At the time of writing this 
only runs the pre-authentication stages, though post-authentication could be run similarly.

## Encryption and Randomness

When running in fuzzing mode Dropbear uses a [fixed seed](dbrandom.c#L185)
every time so that failures can be reproduced. 

Since the fuzzer cannot generate valid encrypted input the packet decryption and
message authentication calls are disabled, see [packet.c](packet.c). 
MAC failures are set to occur with a low probability to test that error path.

## Fuzzers

Current fuzzers are

- [fuzzer-preauth](fuzzer-preauth.c) - the fuzzer input is treated as a stream of session input. This will
  test key exchange, packet ordering, authentication attempts etc.

- [fuzzer-preauth_nomaths](fuzzer-preauth_nomaths.c) - the same as fuzzer-preauth but with asymmetric crypto
  routines replaced with dummies for faster runtime. corpora are shared 
  between fuzzers by [oss-fuzz](https://github.com/google/oss-fuzz) so this 
  will help fuzzer-preauth too.

- [fuzzer-verify](fuzzer-verify.c) - read a key and signature from fuzzer input and verify that signature. 
  It would not be expected to pass, though some keys with bad parameters are 
  able to validate with a trivial signature - extra checks are added for that.

- [fuzzer-pubkey](fuzzer-pubkey.c) - test parsing of an `authorized_keys` line.

- [fuzzer-kexdh](fuzzer-kexdh.c) - test Diffie-Hellman key exchange where the fuzz input is the 
  ephemeral public key that would be received over the network. This is testing `mp_expt_mod()`
  and and other libtommath routines.

- [fuzzer-kexecdh](fuzzer-kexecdh.c) - test Elliptic Curve Diffie-Hellman key exchange like fuzzer-kexdh.
  This is testing libtommath ECC routines.

- [fuzzer-kexcurve25519](fuzzer-kexcurve25519.c) - test Curve25519 Elliptic Curve Diffie-Hellman key exchange
  like fuzzer-kexecdh. This is testing `dropbear_curve25519_scalarmult()` and other libtommath routines.
