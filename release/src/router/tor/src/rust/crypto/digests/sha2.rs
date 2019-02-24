// Copyright (c) 2018-2019, The Tor Project, Inc.
// Copyright (c) 2018, isis agora lovecruft
// See LICENSE for licensing information

//! Hash Digests and eXtendible Output Functions (XOFs)

pub use digest::Digest;

use digest::generic_array::typenum::U32;
use digest::generic_array::typenum::U64;
use digest::generic_array::GenericArray;
use digest::BlockInput;
use digest::FixedOutput;
use digest::Input;

use external::crypto_digest::get_256_bit_digest;
use external::crypto_digest::get_512_bit_digest;
use external::crypto_digest::CryptoDigest;
use external::crypto_digest::DigestAlgorithm;

pub use external::crypto_digest::DIGEST256_LEN;
pub use external::crypto_digest::DIGEST512_LEN;

/// The block size for both SHA-256 and SHA-512 digests is 512 bits/64 bytes.
///
/// Unfortunately, we have to use the generic_array crate currently to express
/// this at compile time.  Later, in the future, when Rust implements const
/// generics, we'll be able to remove this dependency (actually, it will get
/// removed from the digest crate, which is currently `pub use`ing it).
type BlockSize = U64;

/// A SHA2-256 digest.
///
/// # C_RUST_COUPLED
///
/// * `crypto_digest_dup`
#[derive(Clone)]
pub struct Sha256 {
    engine: CryptoDigest,
}

/// Construct a new, default instance of a `Sha256` hash digest function.
///
/// # Examples
///
/// ```rust,no_run
/// use crypto::digests::sha2::{Sha256, Digest};
///
/// let mut hasher: Sha256 = Sha256::default();
/// ```
///
/// # Returns
///
/// A new `Sha256` digest.
impl Default for Sha256 {
    fn default() -> Sha256 {
        Sha256 {
            engine: CryptoDigest::new(Some(DigestAlgorithm::SHA2_256)),
        }
    }
}

impl BlockInput for Sha256 {
    type BlockSize = BlockSize;
}

/// Input `msg` into the digest.
///
/// # Examples
///
/// ```rust,no_run
/// use crypto::digests::sha2::{Sha256, Digest};
///
/// let mut hasher: Sha256 = Sha256::default();
///
/// hasher.input(b"foo");
/// hasher.input(b"bar");
/// ```
impl Input for Sha256 {
    fn process(&mut self, msg: &[u8]) {
        self.engine.add_bytes(&msg);
    }
}

/// Retrieve the output hash from everything which has been fed into this
/// `Sha256` digest thus far.
///
//
// FIXME: Once const generics land in Rust, we should genericise calling
// crypto_digest_get_digest in external::crypto_digest.
impl FixedOutput for Sha256 {
    type OutputSize = U32;

    fn fixed_result(self) -> GenericArray<u8, Self::OutputSize> {
        let buffer: [u8; DIGEST256_LEN] = get_256_bit_digest(self.engine);

        GenericArray::from(buffer)
    }
}

/// A SHA2-512 digest.
///
/// # C_RUST_COUPLED
///
/// * `crypto_digest_dup`
#[derive(Clone)]
pub struct Sha512 {
    engine: CryptoDigest,
}

/// Construct a new, default instance of a `Sha512` hash digest function.
///
/// # Examples
///
/// ```rust,no_run
/// use crypto::digests::sha2::{Sha512, Digest};
///
/// let mut hasher: Sha512 = Sha512::default();
/// ```
///
/// # Returns
///
/// A new `Sha512` digest.
impl Default for Sha512 {
    fn default() -> Sha512 {
        Sha512 {
            engine: CryptoDigest::new(Some(DigestAlgorithm::SHA2_512)),
        }
    }
}

impl BlockInput for Sha512 {
    type BlockSize = BlockSize;
}

/// Input `msg` into the digest.
///
/// # Examples
///
/// ```rust,no_run
/// use crypto::digests::sha2::{Sha512, Digest};
///
/// let mut hasher: Sha512 = Sha512::default();
///
/// hasher.input(b"foo");
/// hasher.input(b"bar");
/// ```
impl Input for Sha512 {
    fn process(&mut self, msg: &[u8]) {
        self.engine.add_bytes(&msg);
    }
}

/// Retrieve the output hash from everything which has been fed into this
/// `Sha512` digest thus far.
///
//
// FIXME: Once const generics land in Rust, we should genericise calling
// crypto_digest_get_digest in external::crypto_digest.
impl FixedOutput for Sha512 {
    type OutputSize = U64;

    fn fixed_result(self) -> GenericArray<u8, Self::OutputSize> {
        let buffer: [u8; DIGEST512_LEN] = get_512_bit_digest(self.engine);

        GenericArray::clone_from_slice(&buffer)
    }
}

#[cfg(test)]
mod test {
    #[cfg(feature = "test-c-from-rust")]
    use digest::Digest;

    #[cfg(feature = "test-c-from-rust")]
    use super::*;

    #[cfg(feature = "test-c-from-rust")]
    #[test]
    fn sha256_default() {
        let _: Sha256 = Sha256::default();
    }

    #[cfg(feature = "test-c-from-rust")]
    #[test]
    fn sha256_digest() {
        let mut h: Sha256 = Sha256::new();
        let mut result: [u8; DIGEST256_LEN] = [0u8; DIGEST256_LEN];
        let expected = [
            151, 223, 53, 136, 181, 163, 242, 75, 171, 195, 133, 27, 55, 47, 11, 167, 26, 157, 205,
            222, 212, 59, 20, 185, 208, 105, 97, 191, 193, 112, 125, 157,
        ];

        h.input(b"foo");
        h.input(b"bar");
        h.input(b"baz");

        result.copy_from_slice(h.fixed_result().as_slice());

        println!("{:?}", &result[..]);

        assert_eq!(result, expected);
    }

    #[cfg(feature = "test-c-from-rust")]
    #[test]
    fn sha512_default() {
        let _: Sha512 = Sha512::default();
    }

    #[cfg(feature = "test-c-from-rust")]
    #[test]
    fn sha512_digest() {
        let mut h: Sha512 = Sha512::new();
        let mut result: [u8; DIGEST512_LEN] = [0u8; DIGEST512_LEN];

        let expected = [
            203, 55, 124, 16, 176, 245, 166, 44, 128, 54, 37, 167, 153, 217, 233, 8, 190, 69, 231,
            103, 245, 209, 71, 212, 116, 73, 7, 203, 5, 89, 122, 164, 237, 211, 41, 160, 175, 20,
            122, 221, 12, 244, 24, 30, 211, 40, 250, 30, 121, 148, 38, 88, 38, 179, 237, 61, 126,
            246, 240, 103, 202, 153, 24, 90,
        ];

        h.input(b"foo");
        h.input(b"bar");
        h.input(b"baz");

        result.copy_from_slice(h.fixed_result().as_slice());

        println!("{:?}", &result[..]);

        assert_eq!(&result[..], &expected[..]);
    }
}
