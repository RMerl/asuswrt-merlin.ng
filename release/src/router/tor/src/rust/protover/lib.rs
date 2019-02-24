//! Copyright (c) 2016-2019, The Tor Project, Inc. */
//! See LICENSE for licensing information */

//! Versioning information for different pieces of the Tor protocol.
//!
//! The below description is taken from src/rust/protover.c, which is currently
//! enabled by default. We are in the process of experimenting with Rust in
//! tor, and this protover module is implemented to help achieve this goal.
//!
//! Starting in version 0.2.9.3-alpha, Tor places separate version numbers on
//! each of the different components of its protocol. Relays use these numbers
//! to advertise what versions of the protocols they can support, and clients
//! use them to find what they can ask a given relay to do.  Authorities vote
//! on the supported protocol versions for each relay, and also vote on the
//! which protocols you should have to support in order to be on the Tor
//! network. All Tor instances use these required/recommended protocol versions
//! to tell what level of support for recent protocols each relay has, and
//! to decide whether they should be running given their current protocols.
//!
//! The main advantage of these protocol versions numbers over using Tor
//! version numbers is that they allow different implementations of the Tor
//! protocols to develop independently, without having to claim compatibility
//! with specific versions of Tor.

// XXX: add missing docs
//#![deny(missing_docs)]

extern crate external;
extern crate libc;
extern crate smartlist;
extern crate tor_allocate;
#[macro_use]
extern crate tor_util;

pub mod errors;
pub mod ffi;
pub mod protoset;
mod protover;

pub use protover::*;
