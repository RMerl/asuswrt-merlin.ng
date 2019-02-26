// Copyright (c) 2016-2019, The Tor Project, Inc. */
// See LICENSE for licensing information */

extern crate libc;

mod smartlist;

pub use smartlist::*;

// When testing we may be compiled with sanitizers which are incompatible with
// Rust's default allocator, jemalloc (unsure why at this time). Most crates
// link to `tor_allocate` which switches by default to a non-jemalloc allocator,
// but we don't already depend on `tor_allocate` so make sure that while testing
// we don't use jemalloc. (but rather malloc/free)
#[global_allocator]
#[cfg(test)]
static A: std::alloc::System = std::alloc::System;
