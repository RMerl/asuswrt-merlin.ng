// Copyright (c) 2018-2019, The Tor Project, Inc.
// Copyright (c) 2018, isis agora lovecruft
// See LICENSE for licensing information

//! Common cryptographic functions and utilities.
//!
//! # Hash Digests and eXtendable Output Functions (XOFs)
//!
//! The `digests` module contains submodules for specific hash digests
//! and extendable output functions.
//!
//! ```rust,no_run
//! use crypto::digests::sha2::*;
//!
//! let mut hasher: Sha256 = Sha256::default();
//! let mut result: [u8; 32] = [0u8; 32];
//!
//! hasher.input(b"foo");
//! hasher.input(b"bar");
//! hasher.input(b"baz");
//!
//! result.copy_from_slice(hasher.result().as_slice());
//!
//! assert!(result == [b'X'; DIGEST256_LEN]);
//! ```

// XXX: add missing docs
//#![deny(missing_docs)]

// External crates from cargo or TOR_RUST_DEPENDENCIES.
extern crate digest;
extern crate libc;
extern crate rand_core;

// External dependencies for tests.
#[cfg(test)]
extern crate rand as rand_crate;

// Our local crates.
extern crate external;
#[cfg(not(test))]
#[macro_use]
extern crate tor_log;

pub mod digests; // Unfortunately named "digests" plural to avoid name conflict with the digest crate
pub mod rand;
