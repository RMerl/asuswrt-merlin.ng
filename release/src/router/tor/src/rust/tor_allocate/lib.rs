// Copyright (c) 2016-2019, The Tor Project, Inc. */
// See LICENSE for licensing information */

//! Allocation helper functions that allow data to be allocated in Rust
//! using tor's specified allocator. In doing so, this can be later freed
//! from C.
//!
//! This is currently a temporary solution, we will later use tor's allocator
//! by default for any allocation that occurs in Rust. However, as this will
//! stabalize in 2018, we can use this as a temporary measure.

extern crate libc;

use std::alloc::System;

mod tor_allocate;
pub use tor_allocate::*;

#[global_allocator]
static A: System = System;
