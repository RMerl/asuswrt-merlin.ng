//! Copyright (c) 2016-2019, The Tor Project, Inc. */
//! See LICENSE for licensing information */

//! Logging wrapper for Rust to utilize Tor's logger, found at
//! src/common/log.c and src/common/torlog.h
//!
//! Exposes different interfaces depending on whether we are running in test
//! or non-test mode. When testing, we use a no-op implementation,
//! otherwise we link directly to C.

extern crate libc;
extern crate tor_allocate;

mod tor_log;

pub use tor_log::*;
