= Fuzzing Tor

== The simple version (no fuzzing, only tests)

Check out fuzzing-corpora, and set TOR_FUZZ_CORPORA to point to the place
where you checked it out.

To run the fuzzing test cases in a deterministic fashion, use:
      make test-fuzz-corpora

This won't actually fuzz Tor!  It will just run all the fuzz binaries
on our existing set of testcases for the fuzzer.


== Different kinds of fuzzing

Right now we support three different kinds of fuzzer.

First, there's American Fuzzy Lop (AFL), a fuzzer that works by forking
a target binary and passing it lots of different inputs on stdin.  It's the
trickiest one to set up, so I'll be describing it more below.

Second, there's libFuzzer, a llvm-based fuzzer that you link in as a library,
and it runs a target function over and over.  To use this one, you'll need to
have a reasonably recent clang and libfuzzer installed.  At that point, you
just build with --enable-expensive-hardening and --enable-libfuzzer.  That
will produce a set of binaries in src/test/fuzz/lf-fuzz-* .  These programs
take as input a series of directories full of fuzzing examples.  For more
information on libfuzzer, see http://llvm.org/docs/LibFuzzer.html

Third, there's Google's OSS-Fuzz infrastructure, which expects to get all of
its.  For more on this, see https://github.com/google/oss-fuzz and the
projects/tor subdirectory.  You'll need to mess around with Docker a bit to
test this one out; it's meant to run on Google's infrastructure.

In all cases, you'll need some starting examples to give the fuzzer when it
starts out.  There's a set in the "fuzzing-corpora" git repository.  Try
setting TOR_FUZZ_CORPORA to point to a checkout of that repository

== Writing Tor fuzzers

A tor fuzzing harness should have:
* a fuzz_init() function to set up any necessary global state.
* a fuzz_main() function to receive input and pass it to a parser.
* a fuzz_cleanup() function to clear global state.

Most fuzzing frameworks will produce many invalid inputs - a tor fuzzing
harness should rejecting invalid inputs without crashing or behaving badly.

But the fuzzing harness should crash if tor fails an assertion, triggers a
bug, or accesses memory it shouldn't. This helps fuzzing frameworks detect
"interesting" cases.


== Guided Fuzzing with AFL

There is no HTTPS, hash, or signature for American Fuzzy Lop's source code, so
its integrity can't be verified. That said, you really shouldn't fuzz on a
machine you care about, anyway.

To Build:
  Get AFL from http://lcamtuf.coredump.cx/afl/ and unpack it
  cd afl
  make
  cd ../tor
  PATH=$PATH:../afl/ CC="../afl/afl-gcc" ./configure --enable-expensive-hardening
  AFL_HARDEN=1 make clean fuzzers

To Find The ASAN Memory Limit: (64-bit only)

On 64-bit platforms, afl needs to know how much memory ASAN uses,
because ASAN tends to allocate a ridiculous amount of virtual memory,
and then not actually use it.

Read afl/docs/notes_for_asan.txt for more details.

  Download recidivm from http://jwilk.net/software/recidivm
  Download the signature
  Check the signature
  tar xvzf recidivm*.tar.gz
  cd recidivm*
  make
  /path/to/recidivm -v src/test/fuzz/fuzz-http
  Use the final "ok" figure as the input to -m when calling afl-fuzz
  (Normally, recidivm would output a figure automatically, but in some cases,
  the fuzzing harness will hang when the memory limit is too small.)

You could also just say "none" instead of the memory limit below, if you
don't care about memory limits.


To Run:
  mkdir -p src/test/fuzz/fuzz_http_findings
  ../afl/afl-fuzz -i ${TOR_FUZZ_CORPORA}/http -o src/test/fuzz/fuzz_http_findings -m <asan-memory-limit> -- src/test/fuzz/fuzz-http


AFL has a multi-core mode, check the documentation for details.
You might find the included fuzz-multi.sh script useful for this.

macOS (OS X) requires slightly more preparation, including:
* using afl-clang (or afl-clang-fast from the llvm directory)
* disabling external crash reporting (AFL will guide you through this step)

== Triaging Issues

Crashes are usually interesting, particularly if using AFL_HARDEN=1 and --enable-expensive-hardening. Sometimes crashes are due to bugs in the harness code.

Hangs might be interesting, but they might also be spurious machine slowdowns.
Check if a hang is reproducible before reporting it. Sometimes, processing
valid inputs may take a second or so, particularly with the fuzzer and
sanitizers enabled.

To see what fuzz-http is doing with a test case, call it like this:
  src/test/fuzz/fuzz-http --debug < /path/to/test.case

(Logging is disabled while fuzzing to increase fuzzing speed.)

== Reporting Issues

Please report any issues discovered using the process in Tor's security issue
policy:

https://trac.torproject.org/projects/tor/wiki/org/meetings/2016SummerDevMeeting/Notes/SecurityIssuePolicy
