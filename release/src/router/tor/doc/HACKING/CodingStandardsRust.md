# Rust Coding Standards

You MUST follow the standards laid out in `doc/HACKING/CodingStandards.md`,
where applicable.

## Module/Crate Declarations

Each Tor C module which is being rewritten MUST be in its own crate.
See the structure of `src/rust` for examples.

In your crate, you MUST use `lib.rs` ONLY for pulling in external
crates (e.g. `extern crate libc;`) and exporting public objects from
other Rust modules (e.g. `pub use mymodule::foo;`).  For example, if
you create a crate in `src/rust/yourcrate`, your Rust code should
live in `src/rust/yourcrate/yourcode.rs` and the public interface
to it should be exported in `src/rust/yourcrate/lib.rs`.

If your code is to be called from Tor C code, you MUST define a safe
`ffi.rs`.  See the "Safety" section further down for more details.

For example, in a hypothetical `tor_addition` Rust module:

In `src/rust/tor_addition/addition.rs`:

```rust
pub fn get_sum(a: i32, b: i32) -> i32 {
    a + b
}
```

In `src/rust/tor_addition/lib.rs`:

```rust
pub use addition::*;
```

In `src/rust/tor_addition/ffi.rs`:

```rust
#[no_mangle]
pub extern "C" fn tor_get_sum(a: c_int, b: c_int) -> c_int {
    get_sum(a, b)
}
```

If your Rust code must call out to parts of Tor's C code, you must
declare the functions you are calling in the `external` crate, located
at `src/rust/external`.

<!-- XXX get better examples of how to declare these externs, when/how they -->
<!-- XXX are unsafe, what they are expected to do —isis                     -->

Modules should strive to be below 500 lines (tests excluded). Single
responsibility and limited dependencies should be a guiding standard.

If you have any external modules as dependencies (e.g. `extern crate
libc;`), you MUST declare them in your crate's `lib.rs` and NOT in any
other module.

## Dependencies and versions

In general, we use modules from only the Rust standard library
whenever possible. We will review including external crates on a
case-by-case basis.

If a crate only contains traits meant for compatibility between Rust
crates, such as [the digest crate](https://crates.io/crates/digest) or
[the failure crate](https://crates.io/crates/failure), it is very likely
permissible to add it as a dependency.  However, a brief review should
be conducted as to the usefulness of implementing external traits
(i.e. how widespread is the usage, how many other crates either
implement the traits or have trait bounds based upon them), as well as
the stability of the traits (i.e. if the trait is going to change, we'll
potentially have to re-do all our implementations of it).

For large external libraries, especially which implement features which
would be labour-intensive to reproduce/maintain ourselves, such as
cryptographic or mathematical/statistics libraries, only crates which
have stabilised to 1.0.0 should be considered, however, again, we may
make exceptions on a case-by-case basis.

Currently, Tor requires that you use the latest stable Rust version. At
some point in the future, we will freeze on a given stable Rust version,
to ensure backward compatibility with stable distributions that ship it.

## Updating/Adding Dependencies

To add/remove/update dependencies, first add your dependencies,
exactly specifying their versions, into the appropriate *crate-level*
`Cargo.toml` in `src/rust/` (i.e. *not* `/src/rust/Cargo.toml`, but
instead the one for your crate).  Also, investigate whether your
dependency has any optional dependencies which are unnecessary but are
enabled by default.  If so, you'll likely be able to enable/disable
them via some feature, e.g.:

```toml
[dependencies]
foo = { version = "1.0.0", default-features = false }
```

Next, run `/scripts/maint/updateRustDependencies.sh`.  Then, go into
`src/ext/rust` and commit the changes to the `tor-rust-dependencies`
repo.

## Documentation

You MUST include `#![deny(missing_docs)]` in your crate.

For function/method comments, you SHOULD include a one-sentence, "first person"
description of function behaviour (see requirements for documentation as
described in `src/HACKING/CodingStandards.md`), then an `# Inputs` section
for inputs or initialisation values, a `# Returns` section for return
values/types, a `# Warning` section containing warnings for unsafe behaviours or
panics that could happen.  For publicly accessible
types/constants/objects/functions/methods, you SHOULD also include an
`# Examples` section with runnable doctests.

You MUST document your module with _module docstring_ comments,
i.e. `//!` at the beginning of each line.

## Style

You SHOULD consider breaking up large literal numbers with `_` when it makes it
more human readable to do so, e.g. `let x: u64 = 100_000_000_000`.

## Testing

All code MUST be unittested and integration tested.

Public functions/objects exported from a crate SHOULD include doctests
describing how the function/object is expected to be used.

Integration tests SHOULD go into a `tests/` directory inside your
crate.  Unittests SHOULD go into their own module inside the module
they are testing, e.g. in `src/rust/tor_addition/addition.rs` you
should put:

```rust
#[cfg(test)]
mod test {
    use super::*;

#[test]
    fn addition_with_zero() {
        let sum: i32 = get_sum(5i32, 0i32);
        assert_eq!(sum, 5);
    }
}
```

## Benchmarking

The external `test` crate can be used for most benchmarking.  However, using
this crate requires nightly Rust.  Since we may want to switch to a more
stable Rust compiler eventually, we shouldn't do things which will automatically
break builds for stable compilers.  Therefore, you MUST feature-gate your
benchmarks in the following manner.

If you wish to benchmark some of your Rust code, you MUST put the
following in the `[features]` section of your crate's `Cargo.toml`:

```toml
[features]
bench = []
```

Next, in your crate's `lib.rs` you MUST put:

```rust
#[cfg(all(test, feature = "bench"))]
extern crate test;
```

This ensures that the external crate `test`, which contains utilities
for basic benchmarks, is only used when running benchmarks via `cargo
bench --features bench`.

Finally, to write your benchmark code, in
`src/rust/tor_addition/addition.rs` you SHOULD put:

```rust
#[cfg(all(test, features = "bench"))]
mod bench {
    use test::Bencher;
    use super::*;

#[bench]
    fn addition_small_integers(b: &mut Bencher) {
        b.iter(| | get_sum(5i32, 0i32));
    }
}
```

## Fuzzing

If you wish to fuzz parts of your code, please see the
[cargo fuzz](https://github.com/rust-fuzz/cargo-fuzz) crate, which uses
[libfuzzer-sys](https://github.com/rust-fuzz/libfuzzer-sys).

## Whitespace & Formatting

You MUST run `rustfmt` (https://github.com/rust-lang-nursery/rustfmt)
on your code before your code will be merged.  You can install rustfmt
by doing `cargo install rustfmt-nightly` and then run it with `cargo
fmt`.

## Safety

You SHOULD read [the nomicon](https://doc.rust-lang.org/nomicon/) before writing
Rust FFI code.  It is *highly advised* that you read and write normal Rust code
before attempting to write FFI or any other unsafe code.

Here are some additional bits of advice and rules:

0. Any behaviours which Rust considers to be undefined are forbidden

   From https://doc.rust-lang.org/reference/behavior-considered-undefined.html:

   > Behavior considered undefined
   >
   > The following is a list of behavior which is forbidden in all Rust code,
   > including within unsafe blocks and unsafe functions. Type checking provides the
   > guarantee that these issues are never caused by safe code.
   > 
   > * Data races
   > * Dereferencing a null/dangling raw pointer
   > * Reads of [undef](https://llvm.org/docs/LangRef.html#undefined-values)
   >   (uninitialized) memory
   > * Breaking the
   >   [pointer aliasing rules](https://llvm.org/docs/LangRef.html#pointer-aliasing-rules)
   >   with raw pointers (a subset of the rules used by C)
   > * `&mut T` and `&T` follow LLVM’s scoped noalias model, except if the `&T`
   >   contains an `UnsafeCell<U>`. Unsafe code must not violate these aliasing
   >   guarantees.
   > * Mutating non-mutable data (that is, data reached through a shared
   >   reference or data owned by a `let` binding), unless that data is
   >   contained within an `UnsafeCell<U>`.
   > * Invoking undefined behavior via compiler intrinsics:
   >     - Indexing outside of the bounds of an object with
   >       `std::ptr::offset` (`offset` intrinsic), with the exception of
   >       one byte past the end which is permitted.
   >     - Using `std::ptr::copy_nonoverlapping_memory` (`memcpy32`/`memcpy64`
   >       intrinsics) on overlapping buffers
   > * Invalid values in primitive types, even in private fields/locals:
   >     - Dangling/null references or boxes
   >     - A value other than `false` (0) or `true` (1) in a `bool`
   >     - A discriminant in an `enum` not included in the type definition
   >     - A value in a `char` which is a surrogate or above `char::MAX`
   >     - Non-UTF-8 byte sequences in a `str`
   > * Unwinding into Rust from foreign code or unwinding from Rust into foreign
   >   code. Rust's failure system is not compatible with exception handling in other
   >   languages. Unwinding must be caught and handled at FFI boundaries.

1. `unwrap()`

   If you call `unwrap()`, anywhere, even in a test, you MUST include
   an inline comment stating how the unwrap will either 1) never fail,
   or 2) should fail (i.e. in a unittest).

   You SHOULD NOT use `unwrap()` anywhere in which it is possible to handle the
   potential error with the eel operator, `?` or another non panicking way.
   For example, consider a function which parses a string into an integer:

   ```rust
   fn parse_port_number(config_string: &str) -> u16 {
       u16::from_str_radix(config_string, 10).unwrap()
   }
   ```

   There are numerous ways this can fail, and the `unwrap()` will cause the
   whole program to byte the dust!  Instead, either you SHOULD use `ok()`
   (or another equivalent function which will return an `Option` or a `Result`)
   and change the return type to be compatible:

   ```rust
   fn parse_port_number(config_string: &str) -> Option<u16> {
       u16::from_str_radix(config_string, 10).ok()
   }
   ```

   or you SHOULD use `or()` (or another similar method):

    ```rust
    fn parse_port_number(config_string: &str) -> Option<u16> {
        u16::from_str_radix(config_string, 10).or(Err("Couldn't parse port into a u16")
    }
    ```

   Using methods like `or()` can be particularly handy when you must do
   something afterwards with the data, for example, if we wanted to guarantee
   that the port is high.  Combining these methods with the eel operator (`?`)
   makes this even easier:

    ```rust
    fn parse_port_number(config_string: &str) -> Result<u16, Err> {
        let port = u16::from_str_radix(config_string, 10).or(Err("Couldn't parse port into a u16"))?;

        if port > 1024 {
            return Ok(port);
        } else {
            return Err("Low ports not allowed");
        }
    }
    ```

2. `unsafe`

   If you use `unsafe`, you MUST describe a contract in your
   documentation which describes how and when the unsafe code may
   fail, and what expectations are made w.r.t. the interfaces to
   unsafe code.  This is also REQUIRED for major pieces of FFI between
   C and Rust.

   When creating an FFI in Rust for C code to call, it is NOT REQUIRED
   to declare the entire function `unsafe`.  For example, rather than doing:

    ```rust
    #[no_mangle]
    pub unsafe extern "C" fn increment_and_combine_numbers(mut numbers: [u8; 4]) -> u32 {
        for number in &mut numbers {
            *number += 1;
        }
        std::mem::transmute::<[u8; 4], u32>(numbers)
    }
    ```

   You SHOULD instead do:

    ```rust
    #[no_mangle]
    pub extern "C" fn increment_and_combine_numbers(mut numbers: [u8; 4]) -> u32 {
        for index in 0..numbers.len() {
            numbers[index] += 1;
        }
        unsafe {
            std::mem::transmute::<[u8; 4], u32>(numbers)
        }
    }
    ```

3. Pass only C-compatible primitive types and bytes over the boundary

   Rust's C-compatible primitive types are integers and floats.
   These types are declared in the [libc crate](https://doc.rust-lang.org/libc/x86_64-unknown-linux-gnu/libc/index.html#types).
   Most Rust objects have different [representations](https://doc.rust-lang.org/libc/x86_64-unknown-linux-gnu/libc/index.html#types)
   in C and Rust, so they can't be passed using FFI.

   Tor currently uses the following Rust primitive types from libc for FFI:
   * defined-size integers: `uint32_t`
   * native-sized integers: `c_int`
   * native-sized floats: `c_double`
   * native-sized raw pointers: `* c_void`, `* c_char`, `** c_char`

   TODO: C smartlist to Stringlist conversion using FFI

   The only non-primitive type which may cross the FFI boundary is
   bytes, e.g. `&[u8]`.  This SHOULD be done on the Rust side by
   passing a pointer (`*mut libc::c_char`). The length can be passed
   explicitly (`libc::size_t`), or the string can be NUL-byte terminated
   C string.

   One might be tempted to do this via doing
   `CString::new("blah").unwrap().into_raw()`. This has several problems:

   a) If you do `CString::new("bl\x00ah")` then the unwrap() will fail
      due to the additional NULL terminator, causing a dangling
      pointer to be returned (as well as a potential use-after-free).

   b) Returning the raw pointer will cause the CString to run its deallocator,
      which causes any C code which tries to access the contents to dereference a
      NULL pointer.

   c) If we were to do `as_raw()` this would result in a potential double-free
      since the Rust deallocator would run and possibly Tor's deallocator.

   d) Calling `into_raw()` without later using the same pointer in Rust to call
      `from_raw()` and then deallocate in Rust can result in a
      [memory leak](https://doc.rust-lang.org/std/ffi/struct.CString.html#method.into_raw).

      [It was determined](https://github.com/rust-lang/rust/pull/41074) that this
      is safe to do if you use the same allocator in C and Rust and also specify
      the memory alignment for CString (except that there is no way to specify
      the alignment for CString).  It is believed that the alignment is always 1,
      which would mean it's safe to dealloc the resulting `*mut c_char` in Tor's
      C code.  However, the Rust developers are not willing to guarantee the
      stability of, or a contract for, this behaviour, citing concerns that this
      is potentially extremely and subtly unsafe.

4. Perform an allocation on the other side of the boundary

   After crossing the boundary, the other side MUST perform an
   allocation to copy the data and is therefore responsible for
   freeing that memory later.

5. No touching other language's enums

   Rust enums should never be touched from C (nor can they be safely
   `#[repr(C)]`) nor vice versa:

   >  "The chosen size is the default enum size for the target platform's C
   >  ABI. Note that enum representation in C is implementation defined, so this is
   >  really a "best guess". In particular, this may be incorrect when the C code
   >  of interest is compiled with certain flags."

   (from https://gankro.github.io/nomicon/other-reprs.html)

6. Type safety

   Wherever possible and sensical, you SHOULD create new types in a
   manner which prevents type confusion or misuse.  For example,
   rather than using an untyped mapping between strings and integers
   like so:

    ```rust
    use std::collections::HashMap;

    pub fn get_elements_with_over_9000_points(map: &HashMap<String, usize>) -> Vec<String> {
        ...
    }
    ```

   It would be safer to define a new type, such that some other usage
   of `HashMap<String, usize>` cannot be confused for this type:

   ```rust
   pub struct DragonBallZPowers(pub HashMap<String, usize>);

   impl DragonBallZPowers {
       pub fn over_nine_thousand<'a>(&'a self) -> Vec<&'a String> {
           let mut powerful_enough: Vec<&'a String> = Vec::with_capacity(5);

           for (character, power) in &self.0 {
               if *power > 9000 {
                   powerful_enough.push(character);
               }
           }
           powerful_enough
       }
   }
   ```

   Note the following code, which uses Rust's type aliasing, is valid
   but it does NOT meet the desired type safety goals:

    ```rust
    pub type Power = usize;

    pub fn over_nine_thousand(power: &Power) -> bool {
        if *power > 9000 {
            return true;
        }
        false
    }

    // We can still do the following:
    let his_power: usize = 9001;
    over_nine_thousand(&his_power);
    ```

7. Unsafe mucking around with lifetimes

   Because lifetimes are technically, in type theory terms, a kind, i.e. a
   family of types, individual lifetimes can be treated as types.  For example,
   one can arbitrarily extend and shorten lifetime using `std::mem::transmute`:

    ```rust
    struct R<'a>(&'a i32);

    unsafe fn extend_lifetime<'b>(r: R<'b>) -> R<'static> {
        std::mem::transmute::<R<'b>, R<'static>>(r)
    }

    unsafe fn shorten_invariant_lifetime<'b, 'c>(r: &'b mut R<'static>) -> &'b mut R<'c> {
        std::mem::transmute::<&'b mut R<'static>, &'b mut R<'c>>(r)
    }
    ```

   Calling `extend_lifetime()` would cause an `R` passed into it to live forever
   for the life of the program (the `'static` lifetime).  Similarly,
   `shorten_invariant_lifetime()` could be used to take something meant to live
   forever, and cause it to disappear!  This is incredibly unsafe.  If you're
   going to be mucking around with lifetimes like this, first, you better have
   an extremely good reason, and second, you may as be honest and explicit about
   it, and for ferris' sake just use a raw pointer.

   In short, just because lifetimes can be treated like types doesn't mean you
   should do it.

8. Doing excessively unsafe things when there's a safer alternative

   Similarly to #7, often there are excessively unsafe ways to do a task and a
   simpler, safer way.  You MUST choose the safer option where possible.

   For example, `std::mem::transmute` can be abused in ways where casting with
   `as` would be both simpler and safer:

    ```rust
    // Don't do this
    let ptr = &0;
    let ptr_num_transmute = unsafe { std::mem::transmute::<&i32, usize>(ptr)};

    // Use an `as` cast instead
    let ptr_num_cast = ptr as *const i32 as usize;
    ```

   In fact, using `std::mem::transmute` for *any* reason is a code smell and as
   such SHOULD be avoided.

9. Casting integers with `as`

   This is generally fine to do, but it has some behaviours which you should be
   aware of.  Casting down chops off the high bits, e.g.:

    ```rust
    let x: u32 = 4294967295;
    println!("{}", x as u16); // prints 65535
    ```

   Some cases which you MUST NOT do include:

   * Casting an `u128` down to an `f32` or vice versa (e.g.
     `u128::MAX as f32` but this isn't only a problem with overflowing
     as it is also undefined behaviour for `42.0f32 as u128`),

   * Casting between integers and floats when the thing being cast
     cannot fit into the type it is being casted into, e.g.:

    ```rust
     println!("{}", 42949.0f32 as u8); // prints 197 in debug mode and 0 in release
     println!("{}", 1.04E+17 as u8);   // prints 0 in both modes
     println!("{}", (0.0/0.0) as i64); // prints whatever the heck LLVM wants
     ```

     Because this behaviour is undefined, it can even produce segfaults in
     safe Rust code.  For example, the following program built in release
     mode segfaults:

    ```rust
     #[inline(never)]
     pub fn trigger_ub(sl: &[u8; 666]) -> &[u8] {
         // Note that the float is out of the range of `usize`, invoking UB when casting.
         let idx = 1e99999f64 as usize;
         &sl[idx..] // The bound check is elided due to `idx` being of an undefined value.
     }

     fn main() {
         println!("{}", trigger_ub(&[1; 666])[999999]); // ~ out of bound
     }
     ```

      And in debug mode panics with:

         thread 'main' panicked at 'slice index starts at 140721821254240 but ends at 666', /checkout/src/libcore/slice/mod.rs:754:4
