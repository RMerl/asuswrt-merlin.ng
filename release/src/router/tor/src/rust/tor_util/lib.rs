// Copyright (c) 2016-2019, The Tor Project, Inc. */
// See LICENSE for licensing information */

//! Small module to announce Rust support during startup for demonstration
//! purposes.

extern crate libc;
extern crate tor_allocate;

#[macro_use]
extern crate tor_log;

pub mod ffi;
pub mod strings;
