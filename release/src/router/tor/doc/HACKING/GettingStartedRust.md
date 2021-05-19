# Hacking on Rust in Tor

## Getting Started

Please read or review our documentation on Rust coding standards
(`doc/HACKING/CodingStandardsRust.md`) before doing anything.

Please also read
[the Rust Code of Conduct](https://www.rust-lang.org/en-US/conduct.html). We
aim to follow the good example set by the Rust community and be
excellent to one another.  Let's be careful with each other, so we can
be memory-safe together!

Next, please contact us before rewriting anything!  Rust in Tor is still
an experiment.  It is an experiment that we very much want to see
succeed, so we're going slowly and carefully.  For the moment, it's also
a completely volunteer-driven effort: while many, if not most, of us are
paid to work on Tor, we are not yet funded to write Rust code for Tor.
Please be patient with the other people who are working on getting more
Rust code into Tor, because they are graciously donating their free time
to contribute to this effort.

## Resources for learning Rust

**Beginning resources**

The primary resource for learning Rust is
[The Book](https://doc.rust-lang.org/book/).  If you'd like to start writing
Rust immediately, without waiting for anything to install, there is
[an interactive browser-based playground](https://play.rust-lang.org/).

**Advanced resources**

If you're interested in playing with various Rust compilers and viewing
a very nicely displayed output of the generated assembly, there is
[the Godbolt compiler explorer](https://rust.godbolt.org/)

For learning how to write unsafe Rust, read
[The Rustonomicon](https://doc.rust-lang.org/nomicon/).

For learning everything you ever wanted to know about Rust macros, there
is
[The Little Book of Rust Macros](https://danielkeep.github.io/tlborm/book/index.html).

For learning more about FFI and Rust, see Jake Goulding's
[Rust FFI Omnibus](https://jakegoulding.com/rust-ffi-omnibus/).

## Compiling Tor with Rust enabled

You will need to run the `configure` script with the `--enable-rust`
flag to explicitly build with Rust. Additionally, you will need to
specify where to fetch Rust dependencies, as we allow for either
fetching dependencies from Cargo or specifying a local directory.

**Fetch dependencies from Cargo**

```console
$ ./configure --enable-rust --enable-cargo-online-mode
```

**Using a local dependency cache**

You'll need the following Rust dependencies (as of this writing):

    libc==0.2.39

We vendor our Rust dependencies in a separate repo using
[cargo-vendor](https://github.com/alexcrichton/cargo-vendor).  To use
them, do:

```console
$ git submodule init
$ git submodule update
```

To specify the local directory containing the dependencies, (assuming
you are in the top level of the repository) configure tor with:

```console
$ TOR_RUST_DEPENDENCIES='path_to_dependencies_directory' ./configure --enable-rust
```

(Note that `TOR_RUST_DEPENDENCIES` must be the full path to the directory; it
cannot be relative.)

Assuming you used the above `git submodule` commands and you're in the
topmost directory of the repository, this would be:

```console
$ TOR_RUST_DEPENDENCIES=`pwd`/src/ext/rust/crates ./configure --enable-rust
```

## Identifying which modules to rewrite

The places in the Tor codebase that are good candidates for porting to
Rust are:

1. loosely coupled to other Tor submodules,
2. have high test coverage, and
3. would benefit from being implemented in a memory safe language.

Help in either identifying places such as this, or working to improve
existing areas of the C codebase by adding regression tests and
simplifying dependencies, would be really helpful.

Furthermore, as submodules in C are implemented in Rust, this is a good
opportunity to refactor, add more tests, and split modules into smaller
areas of responsibility.

A good first step is to build a module-level callgraph to understand how
interconnected your target module is.

```console
$ git clone https://git.torproject.org/user/nickm/calltool.git
$ cd tor
$ CFLAGS=0 ./configure
$ ../calltool/src/main.py module_callgraph
```

The output will tell you each module name, along with a set of every module that
the module calls.  Modules which call fewer other modules are better targets.

## Writing your Rust module

Strive to change the C API as little as possible.

We are currently targeting Rust stable. (See `CodingStandardsRust.md` for more
details.)

It is on our TODO list to try to cultivate good
standing with various distro maintainers of `rustc` and `cargo`, in
order to ensure that whatever version we solidify on is readily
available.

If parts of your Rust code needs to stay in sync with C code (such as
handling enums across the FFI boundary), annonotate these places in a
comment structured as follows:

  `/// C_RUST_COUPLED: <path_to_file> <name_of_c_object>`

Where `<name_of_c_object>` can be an enum, struct, constant, etc.  Then,
do the same in the C code, to note that rust will need to be changed
when the C does.

## Adding your Rust module to Tor's build system

0. Your translation of the C module should live in its own crate(s)
   in the `src/rust/` directory.
1. Add your crate to `src/rust/Cargo.toml`, in the
   `[workspace.members]` section.
2. Add your crate's files to src/rust/include.am

If your crate should be available to C (rather than just being included as a
dependency of other Rust modules):
0. Declare the crate as a dependency of tor_rust in
   `src/rust/tor_util/Cargo.toml` and include it in
   `src/rust/tor_rust/lib.rs`

## How to test your Rust code

Everything should be tested full stop.  Even non-public functionality.

Be sure to edit `src/test/test_rust.sh` to add the name of your
crate to the `crates` variable! This will ensure that `cargo test` is
run on your crate.

Configure Tor's build system to build with Rust enabled:

```console
$ ./configure --enable-fatal-warnings --enable-rust --enable-cargo-online-mode
```

Tor's test should be run by doing:

```console
$ make check
```

Tor's integration tests should also pass:

```console
$ make test-stem
```

## Submitting a patch

Please follow the instructions in `doc/HACKING/GettingStarted.md`.
