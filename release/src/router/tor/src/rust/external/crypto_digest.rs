// Copyright (c) 2018-2019, The Tor Project, Inc.
// Copyright (c) 2018, isis agora lovecruft
// See LICENSE for licensing information

//! Bindings to external digest and XOF functions which live within
//! src/common/crypto_digest.[ch].
//!
//! We wrap our C implementations in src/common/crypto_digest.[ch] with more
//! Rusty types and interfaces in src/rust/crypto/digest/.

use std::process::abort;

use libc::c_char;
use libc::c_int;
use libc::size_t;
use libc::uint8_t;

use smartlist::Stringlist;

/// Length of the output of our message digest.
pub const DIGEST_LEN: usize = 20;

/// Length of the output of our second (improved) message digests.  (For now
/// this is just sha256, but it could be any other 256-bit digest.)
pub const DIGEST256_LEN: usize = 32;

/// Length of the output of our 64-bit optimized message digests (SHA512).
pub const DIGEST512_LEN: usize = 64;

/// Length of a sha1 message digest when encoded in base32 with trailing = signs
/// removed.
pub const BASE32_DIGEST_LEN: usize = 32;

/// Length of a sha1 message digest when encoded in base64 with trailing = signs
/// removed.
pub const BASE64_DIGEST_LEN: usize = 27;

/// Length of a sha256 message digest when encoded in base64 with trailing =
/// signs removed.
pub const BASE64_DIGEST256_LEN: usize = 43;

/// Length of a sha512 message digest when encoded in base64 with trailing =
/// signs removed.
pub const BASE64_DIGEST512_LEN: usize = 86;

/// Length of hex encoding of SHA1 digest, not including final NUL.
pub const HEX_DIGEST_LEN: usize = 40;

/// Length of hex encoding of SHA256 digest, not including final NUL.
pub const HEX_DIGEST256_LEN: usize = 64;

/// Length of hex encoding of SHA512 digest, not including final NUL.
pub const HEX_DIGEST512_LEN: usize = 128;

/// Our C code uses an enum to declare the digest algorithm types which we know
/// about.  However, because enums are implementation-defined in C, we can
/// neither work with them directly nor translate them into Rust enums.
/// Instead, we represent them as a u8 (under the assumption that we'll never
/// support more than 256 hash functions).
#[allow(non_camel_case_types)]
type digest_algorithm_t = u8;

const DIGEST_SHA1: digest_algorithm_t = 0;
const DIGEST_SHA256: digest_algorithm_t = 1;
const DIGEST_SHA512: digest_algorithm_t = 2;
const DIGEST_SHA3_256: digest_algorithm_t = 3;
const DIGEST_SHA3_512: digest_algorithm_t = 4;

/// The number of hash digests we produce for a `common_digests_t`.
///
/// We can't access these from Rust, because their definitions in C require
/// introspecting the `digest_algorithm_t` typedef, which is an enum, so we have
/// to redefine them here.
const N_COMMON_DIGEST_ALGORITHMS: usize = DIGEST_SHA256 as usize + 1;

/// A digest function.
#[repr(C)]
#[derive(Debug, Copy, Clone)]
#[allow(non_camel_case_types)]
struct crypto_digest_t {
    // This private, zero-length field forces the struct to be treated the same
    // as its opaque C counterpart.
    _unused: [u8; 0],
}

/// An eXtendible Output Function (XOF).
#[repr(C)]
#[derive(Debug, Copy, Clone)]
#[allow(non_camel_case_types)]
struct crypto_xof_t {
    // This private, zero-length field forces the struct to be treated the same
    // as its opaque C counterpart.
    _unused: [u8; 0],
}

/// A set of all the digests we commonly compute, taken on a single
/// string.  Any digests that are shorter than 512 bits are right-padded
/// with 0 bits.
///
/// Note that this representation wastes 44 bytes for the SHA1 case, so
/// don't use it for anything where we need to allocate a whole bunch at
/// once.
#[repr(C)]
#[derive(Debug, Copy, Clone)]
#[allow(non_camel_case_types)]
struct common_digests_t {
    pub d: [[c_char; N_COMMON_DIGEST_ALGORITHMS]; DIGEST256_LEN],
}

/// A `smartlist_t` is just an alias for the `#[repr(C)]` type `Stringlist`, to
/// make it more clear that we're working with a smartlist which is owned by C.
#[allow(non_camel_case_types)]
// BINDGEN_GENERATED: This type isn't actually bindgen generated, but the code
// below it which uses it is.  As such, this comes up as "dead code" as well.
#[allow(dead_code)]
type smartlist_t = Stringlist;

/// All of the external functions from `src/common/crypto_digest.h`.
///
/// These are kept private because they should be wrapped with Rust to make their usage safer.
//
// BINDGEN_GENERATED: These definitions were generated with bindgen and cleaned
// up manually.  As such, there are more bindings than are likely necessary or
// which are in use.
#[allow(dead_code)]
extern "C" {
    fn crypto_digest(digest: *mut c_char, m: *const c_char, len: size_t) -> c_int;
    fn crypto_digest256(
        digest: *mut c_char,
        m: *const c_char,
        len: size_t,
        algorithm: digest_algorithm_t,
    ) -> c_int;
    fn crypto_digest512(
        digest: *mut c_char,
        m: *const c_char,
        len: size_t,
        algorithm: digest_algorithm_t,
    ) -> c_int;
    fn crypto_common_digests(ds_out: *mut common_digests_t, m: *const c_char, len: size_t)
        -> c_int;
    fn crypto_digest_smartlist_prefix(
        digest_out: *mut c_char,
        len_out: size_t,
        prepend: *const c_char,
        lst: *const smartlist_t,
        append: *const c_char,
        alg: digest_algorithm_t,
    );
    fn crypto_digest_smartlist(
        digest_out: *mut c_char,
        len_out: size_t,
        lst: *const smartlist_t,
        append: *const c_char,
        alg: digest_algorithm_t,
    );
    fn crypto_digest_algorithm_get_name(alg: digest_algorithm_t) -> *const c_char;
    fn crypto_digest_algorithm_get_length(alg: digest_algorithm_t) -> size_t;
    fn crypto_digest_algorithm_parse_name(name: *const c_char) -> c_int;
    fn crypto_digest_new() -> *mut crypto_digest_t;
    fn crypto_digest256_new(algorithm: digest_algorithm_t) -> *mut crypto_digest_t;
    fn crypto_digest512_new(algorithm: digest_algorithm_t) -> *mut crypto_digest_t;
    fn crypto_digest_free_(digest: *mut crypto_digest_t);
    fn crypto_digest_add_bytes(digest: *mut crypto_digest_t, data: *const c_char, len: size_t);
    fn crypto_digest_get_digest(digest: *mut crypto_digest_t, out: *mut c_char, out_len: size_t);
    fn crypto_digest_dup(digest: *const crypto_digest_t) -> *mut crypto_digest_t;
    fn crypto_digest_assign(into: *mut crypto_digest_t, from: *const crypto_digest_t);
    fn crypto_hmac_sha256(
        hmac_out: *mut c_char,
        key: *const c_char,
        key_len: size_t,
        msg: *const c_char,
        msg_len: size_t,
    );
    fn crypto_mac_sha3_256(
        mac_out: *mut uint8_t,
        len_out: size_t,
        key: *const uint8_t,
        key_len: size_t,
        msg: *const uint8_t,
        msg_len: size_t,
    );
    fn crypto_xof_new() -> *mut crypto_xof_t;
    fn crypto_xof_add_bytes(xof: *mut crypto_xof_t, data: *const uint8_t, len: size_t);
    fn crypto_xof_squeeze_bytes(xof: *mut crypto_xof_t, out: *mut uint8_t, len: size_t);
    fn crypto_xof_free(xof: *mut crypto_xof_t);
}

/// A wrapper around a `digest_algorithm_t`.
pub enum DigestAlgorithm {
    SHA2_256,
    SHA2_512,
    SHA3_256,
    SHA3_512,
}

impl From<DigestAlgorithm> for digest_algorithm_t {
    fn from(digest: DigestAlgorithm) -> digest_algorithm_t {
        match digest {
            DigestAlgorithm::SHA2_256 => DIGEST_SHA256,
            DigestAlgorithm::SHA2_512 => DIGEST_SHA512,
            DigestAlgorithm::SHA3_256 => DIGEST_SHA3_256,
            DigestAlgorithm::SHA3_512 => DIGEST_SHA3_512,
        }
    }
}

/// A wrapper around a mutable pointer to a `crypto_digest_t`.
pub struct CryptoDigest(*mut crypto_digest_t);

/// Explicitly copy the state of a `CryptoDigest` hash digest context.
///
/// # C_RUST_COUPLED
///
/// * `crypto_digest_dup`
impl Clone for CryptoDigest {
    fn clone(&self) -> CryptoDigest {
        let digest: *mut crypto_digest_t;

        unsafe {
            digest = crypto_digest_dup(self.0 as *const crypto_digest_t);
        }

        // See the note in the implementation of CryptoDigest for the
        // reasoning for `abort()` here.
        if digest.is_null() {
            abort();
        }

        CryptoDigest(digest)
    }
}

impl CryptoDigest {
    /// A wrapper to call one of the C functions `crypto_digest_new`,
    /// `crypto_digest256_new`, or `crypto_digest512_new`.
    ///
    /// # Warnings
    ///
    /// This function will `abort()` the entire process in an "abnormal" fashion,
    /// i.e. not unwinding this or any other thread's stack, running any
    /// destructors, or calling any panic/exit hooks) if `tor_malloc()` (called in
    /// `crypto_digest256_new()`) is unable to allocate memory.
    ///
    /// # Returns
    ///
    /// A new `CryptoDigest`, which is a wrapper around a opaque representation
    /// of a `crypto_digest_t`.  The underlying `crypto_digest_t` _MUST_ only
    /// ever be handled via a raw pointer, and never introspected.
    ///
    /// # C_RUST_COUPLED
    ///
    /// * `crypto_digest_new`
    /// * `crypto_digest256_new`
    /// * `crypto_digest512_new`
    /// * `tor_malloc` (called by `crypto_digest256_new`, but we make
    ///    assumptions about its behaviour and return values here)
    pub fn new(algorithm: Option<DigestAlgorithm>) -> CryptoDigest {
        let digest: *mut crypto_digest_t;

        if algorithm.is_none() {
            unsafe {
                digest = crypto_digest_new();
            }
        } else {
            let algo: digest_algorithm_t = algorithm.unwrap().into(); // can't fail because it's Some

            unsafe {
                // XXX This is a pretty awkward API to use from Rust...
                digest = match algo {
                    DIGEST_SHA1 => crypto_digest_new(),
                    DIGEST_SHA256 => crypto_digest256_new(DIGEST_SHA256),
                    DIGEST_SHA3_256 => crypto_digest256_new(DIGEST_SHA3_256),
                    DIGEST_SHA512 => crypto_digest512_new(DIGEST_SHA512),
                    DIGEST_SHA3_512 => crypto_digest512_new(DIGEST_SHA3_512),
                    _ => abort(),
                }
            }
        }

        // In our C code, `crypto_digest*_new()` allocates memory with
        // `tor_malloc()`.  In `tor_malloc()`, if the underlying malloc
        // implementation fails to allocate the requested memory and returns a
        // NULL pointer, we call `exit(1)`.  In the case that this `exit(1)` is
        // called within a worker, be that a process or a thread, the inline
        // comments within `tor_malloc()` mention "that's ok, since the parent
        // will run out of memory soon anyway".  However, if it takes long
        // enough for the worker to die, and it manages to return a NULL pointer
        // to our Rust code, our Rust is now in an irreparably broken state and
        // may exhibit undefined behaviour.  An even worse scenario, if/when we
        // have parent/child processes/threads controlled by Rust, would be that
        // the UB contagion in Rust manages to spread to other children before
        // the entire process (hopefully terminates).
        //
        // However, following the assumptions made in `tor_malloc()` that
        // calling `exit(1)` in a child is okay because the parent will
        // eventually run into the same errors, and also to stymie any UB
        // contagion in the meantime, we call abort!() here to terminate the
        // entire program immediately.
        if digest.is_null() {
            abort();
        }

        CryptoDigest(digest)
    }

    /// A wrapper to call the C function `crypto_digest_add_bytes`.
    ///
    /// # Inputs
    ///
    /// * `bytes`: a byte slice of bytes to be added into this digest.
    ///
    /// # C_RUST_COUPLED
    ///
    /// * `crypto_digest_add_bytes`
    pub fn add_bytes(&self, bytes: &[u8]) {
        unsafe {
            crypto_digest_add_bytes(
                self.0 as *mut crypto_digest_t,
                bytes.as_ptr() as *const c_char,
                bytes.len() as size_t,
            )
        }
    }
}

impl Drop for CryptoDigest {
    fn drop(&mut self) {
        unsafe {
            crypto_digest_free_(self.0 as *mut crypto_digest_t);
        }
    }
}

/// Get the 256-bit digest output of a `crypto_digest_t`.
///
/// # Inputs
///
/// * `digest`: A `CryptoDigest` which wraps either a `DIGEST_SHA256` or a
///   `DIGEST_SHA3_256`.
///
/// # Warning
///
/// Calling this function with a `CryptoDigest` which is neither SHA2-256 or
/// SHA3-256 is a programming error.  Since we cannot introspect the opaque
/// struct from Rust, however, there is no way for us to check that the correct
/// one is being passed in.  That is up to you, dear programmer.  If you mess
/// up, you will get a incorrectly-sized hash digest in return, and it will be
/// your fault.  Don't do that.
///
/// # Returns
///
/// A 256-bit hash digest, as a `[u8; 32]`.
///
/// # C_RUST_COUPLED
///
/// * `crypto_digest_get_digest`
/// * `DIGEST256_LEN`
//
// FIXME: Once const generics land in Rust, we should genericise calling
// crypto_digest_get_digest w.r.t. output array size.
pub fn get_256_bit_digest(digest: CryptoDigest) -> [u8; DIGEST256_LEN] {
    let mut buffer: [u8; DIGEST256_LEN] = [0u8; DIGEST256_LEN];

    unsafe {
        crypto_digest_get_digest(
            digest.0,
            buffer.as_mut_ptr() as *mut c_char,
            DIGEST256_LEN as size_t,
        );

        if buffer.as_ptr().is_null() {
            abort();
        }
    }
    buffer
}

/// Get the 512-bit digest output of a `crypto_digest_t`.
///
/// # Inputs
///
/// * `digest`: A `CryptoDigest` which wraps either a `DIGEST_SHA512` or a
///   `DIGEST_SHA3_512`.
///
/// # Warning
///
/// Calling this function with a `CryptoDigest` which is neither SHA2-512 or
/// SHA3-512 is a programming error.  Since we cannot introspect the opaque
/// struct from Rust, however, there is no way for us to check that the correct
/// one is being passed in.  That is up to you, dear programmer.  If you mess
/// up, you will get a incorrectly-sized hash digest in return, and it will be
/// your fault.  Don't do that.
///
/// # Returns
///
/// A 512-bit hash digest, as a `[u8; 64]`.
///
/// # C_RUST_COUPLED
///
/// * `crypto_digest_get_digest`
/// * `DIGEST512_LEN`
//
// FIXME: Once const generics land in Rust, we should genericise calling
// crypto_digest_get_digest w.r.t. output array size.
pub fn get_512_bit_digest(digest: CryptoDigest) -> [u8; DIGEST512_LEN] {
    let mut buffer: [u8; DIGEST512_LEN] = [0u8; DIGEST512_LEN];

    unsafe {
        crypto_digest_get_digest(
            digest.0,
            buffer.as_mut_ptr() as *mut c_char,
            DIGEST512_LEN as size_t,
        );

        if buffer.as_ptr().is_null() {
            abort();
        }
    }
    buffer
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_layout_common_digests_t() {
        assert_eq!(
            ::std::mem::size_of::<common_digests_t>(),
            64usize,
            concat!("Size of: ", stringify!(common_digests_t))
        );
        assert_eq!(
            ::std::mem::align_of::<common_digests_t>(),
            1usize,
            concat!("Alignment of ", stringify!(common_digests_t))
        );
    }

    #[test]
    fn test_layout_crypto_digest_t() {
        assert_eq!(
            ::std::mem::size_of::<crypto_digest_t>(),
            0usize,
            concat!("Size of: ", stringify!(crypto_digest_t))
        );
        assert_eq!(
            ::std::mem::align_of::<crypto_digest_t>(),
            1usize,
            concat!("Alignment of ", stringify!(crypto_digest_t))
        );
    }
}
