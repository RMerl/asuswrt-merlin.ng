// Copyright (c) 2016-2019, The Tor Project, Inc. */
// See LICENSE for licensing information */

//! Utilities for working with static strings.

/// Create a `CStr` from a literal byte slice, appending a NUL byte to it first.
///
/// # Warning
///
/// The literal byte slice which is taken as an argument *MUST NOT* have any NUL
/// bytes (`b"\0"`) in it, anywhere, or else an empty string will be returned
/// (`CStr::from_bytes_with_nul_unchecked(b"\0")`) so as to avoid `panic!()`ing.
///
/// # Examples
///
/// ```
/// #[macro_use]
/// extern crate tor_util;
///
/// use std::ffi::CStr;
///
/// # fn do_test() -> Result<&'static CStr, &'static str> {
/// let message: &'static str = "This is a test of the tsunami warning system.";
/// let tuesday: &'static CStr;
/// let original: &str;
///
/// tuesday = cstr!("This is a test of the tsunami warning system.");
/// original = tuesday.to_str().or(Err("Couldn't unwrap CStr!"))?;
///
/// assert!(original == message);
/// #
/// # Ok(tuesday)
/// # }
/// # fn main() {
/// #     do_test();  // so that we can use the ? operator in the test
/// # }
/// ```
/// It is also possible to pass several string literals to this macro.  They
/// will be concatenated together in the order of the arguments, unmodified,
/// before finally being suffixed with a NUL byte:
///
/// ```
/// #[macro_use]
/// extern crate tor_util;
/// #
/// # use std::ffi::CStr;
/// #
/// # fn do_test() -> Result<&'static CStr, &'static str> {
///
/// let quux: &'static CStr = cstr!("foo", "bar", "baz");
/// let orig: &'static str = quux.to_str().or(Err("Couldn't unwrap CStr!"))?;
///
/// assert!(orig == "foobarbaz");
/// # Ok(quux)
/// # }
/// # fn main() {
/// #     do_test();  // so that we can use the ? operator in the test
/// # }
/// ```
/// This is useful for passing static strings to C from Rust FFI code.  To do so
/// so, use the `.as_ptr()` method on the resulting `&'static CStr` to convert
/// it to the Rust equivalent of a C `const char*`:
///
/// ```
/// #[macro_use]
/// extern crate tor_util;
///
/// use std::ffi::CStr;
/// use std::os::raw::c_char;
///
/// pub extern "C" fn give_static_borrowed_string_to_c() -> *const c_char {
///     let hello: &'static CStr = cstr!("Hello, language my parents wrote.");
///
///     hello.as_ptr()
/// }
/// # fn main() {
/// #     let greetings = give_static_borrowed_string_to_c();
/// # }
/// ```
/// Note that the C code this static borrowed string is passed to *MUST NOT*
/// attempt to free the memory for the string.
///
/// # Note
///
/// An unfortunate limitation of the rustc compiler (as of 1.25.0-nightly), is
/// that the first example above compiles, but if we were to change the
/// assignment of `tuesday` as follows, it will fail to compile, because Rust
/// macros are expanded at parse time, and at parse time there is no symbol
/// table available.
///
/// ```ignore
/// tuesday = cstr!(message);
/// ```
/// with the error message `error: expected a literal`.
///
/// # Returns
///
/// If the string literals passed as arguments contain no NUL bytes anywhere,
/// then an `&'static CStr` containing the (concatenated) bytes of the string
/// literal(s) passed as arguments, with a NUL byte appended, is returned.
/// Otherwise, an `&'static CStr` containing a single NUL byte is returned (an
/// "empty" string in C).
#[macro_export]
macro_rules! cstr {
    ($($bytes:expr),*) => (
        ::std::ffi::CStr::from_bytes_with_nul(
            concat!($($bytes),*, "\0").as_bytes()
        ).unwrap_or_default()
    )
}

#[cfg(test)]
mod test {
    use std::ffi::CStr;

    #[test]
    fn cstr_macro() {
        let _: &'static CStr = cstr!("boo");
    }

    #[test]
    fn cstr_macro_multi_input() {
        let quux: &'static CStr = cstr!("foo", "bar", "baz");

        assert!(quux.to_str().unwrap() == "foobarbaz");
    }

    #[test]
    fn cstr_macro_bad_input() {
        let waving: &'static CStr = cstr!("waving not drowning o/");
        let drowning: &'static CStr = cstr!("\0 drowning not waving");

        assert!(waving.to_str().unwrap() == "waving not drowning o/");
        assert!(drowning.to_str().unwrap() == "")
    }
}
