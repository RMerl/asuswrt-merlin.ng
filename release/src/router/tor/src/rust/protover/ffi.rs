// Copyright (c) 2016-2019, The Tor Project, Inc. */
// See LICENSE for licensing information */

//! FFI functions, only to be called from C.
//!
//! Equivalent C versions of this api are in `protover.c`

use libc::{c_char, c_int, uint32_t};
use std::ffi::CStr;

use smartlist::*;
use tor_allocate::allocate_and_copy_string;

use errors::ProtoverError;
use protover::*;

/// Translate C enums to Rust Proto enums, using the integer value of the C
/// enum to map to its associated Rust enum.
///
/// C_RUST_COUPLED: protover.h `protocol_type_t`
fn translate_to_rust(c_proto: uint32_t) -> Result<Protocol, ProtoverError> {
    match c_proto {
        0 => Ok(Protocol::Link),
        1 => Ok(Protocol::LinkAuth),
        2 => Ok(Protocol::Relay),
        3 => Ok(Protocol::DirCache),
        4 => Ok(Protocol::HSDir),
        5 => Ok(Protocol::HSIntro),
        6 => Ok(Protocol::HSRend),
        7 => Ok(Protocol::Desc),
        8 => Ok(Protocol::Microdesc),
        9 => Ok(Protocol::Cons),
        10 => Ok(Protocol::Padding),
        11 => Ok(Protocol::FlowCtrl),
        _ => Err(ProtoverError::UnknownProtocol),
    }
}

/// Provide an interface for C to translate arguments and return types for
/// protover::all_supported
#[no_mangle]
pub extern "C" fn protover_all_supported(
    c_relay_version: *const c_char,
    missing_out: *mut *mut c_char,
) -> c_int {
    if c_relay_version.is_null() {
        return 1;
    }

    // Require an unsafe block to read the version from a C string. The pointer
    // is checked above to ensure it is not null.
    let c_str: &CStr = unsafe { CStr::from_ptr(c_relay_version) };

    let relay_version = match c_str.to_str() {
        Ok(n) => n,
        Err(_) => return 1,
    };

    let relay_proto_entry: UnvalidatedProtoEntry =
        match UnvalidatedProtoEntry::from_str_any_len(relay_version) {
            Ok(n) => n,
            Err(_) => return 1,
        };

    if let Some(unsupported) = relay_proto_entry.all_supported() {
        if missing_out.is_null() {
            return 0;
        }
        let ptr = allocate_and_copy_string(&unsupported.to_string());
        unsafe { *missing_out = ptr };

        return 0;
    }

    1
}

/// Provide an interface for C to translate arguments and return types for
/// protover::list_supports_protocol
#[no_mangle]
pub extern "C" fn protocol_list_supports_protocol(
    c_protocol_list: *const c_char,
    c_protocol: uint32_t,
    version: uint32_t,
) -> c_int {
    if c_protocol_list.is_null() {
        return 0;
    }

    // Require an unsafe block to read the version from a C string. The pointer
    // is checked above to ensure it is not null.
    let c_str: &CStr = unsafe { CStr::from_ptr(c_protocol_list) };

    let protocol_list = match c_str.to_str() {
        Ok(n) => n,
        Err(_) => return 0,
    };
    let proto_entry: UnvalidatedProtoEntry = match protocol_list.parse() {
        Ok(n) => n,
        Err(_) => return 0,
    };
    let protocol: UnknownProtocol = match translate_to_rust(c_protocol) {
        Ok(n) => n.into(),
        Err(_) => return 0,
    };
    if proto_entry.supports_protocol(&protocol, &version) {
        1
    } else {
        0
    }
}

#[no_mangle]
pub extern "C" fn protover_contains_long_protocol_names_(c_protocol_list: *const c_char) -> c_int {
    if c_protocol_list.is_null() {
        return 1;
    }

    // Require an unsafe block to read the version from a C string. The pointer
    // is checked above to ensure it is not null.
    let c_str: &CStr = unsafe { CStr::from_ptr(c_protocol_list) };

    let protocol_list = match c_str.to_str() {
        Ok(n) => n,
        Err(_) => return 1,
    };

    match protocol_list.parse::<UnvalidatedProtoEntry>() {
        Ok(_) => 0,
        Err(_) => 1,
    }
}

/// Provide an interface for C to translate arguments and return types for
/// protover::list_supports_protocol_or_later
#[no_mangle]
pub extern "C" fn protocol_list_supports_protocol_or_later(
    c_protocol_list: *const c_char,
    c_protocol: uint32_t,
    version: uint32_t,
) -> c_int {
    if c_protocol_list.is_null() {
        return 0;
    }

    // Require an unsafe block to read the version from a C string. The pointer
    // is checked above to ensure it is not null.
    let c_str: &CStr = unsafe { CStr::from_ptr(c_protocol_list) };

    let protocol_list = match c_str.to_str() {
        Ok(n) => n,
        Err(_) => return 0,
    };

    let protocol = match translate_to_rust(c_protocol) {
        Ok(n) => n,
        Err(_) => return 0,
    };

    let proto_entry: UnvalidatedProtoEntry = match protocol_list.parse() {
        Ok(n) => n,
        Err(_) => return 0,
    };

    if proto_entry.supports_protocol_or_later(&protocol.into(), &version) {
        return 1;
    }
    0
}

/// Provide an interface for C to translate arguments and return types for
/// protover::get_supported_protocols
#[no_mangle]
pub extern "C" fn protover_get_supported_protocols() -> *const c_char {
    let supported: &'static CStr;

    supported = get_supported_protocols_cstr();
    supported.as_ptr()
}

/// Provide an interface for C to translate arguments and return types for
/// protover::compute_vote
//
// Why is the threshold a signed integer? —isis
#[no_mangle]
pub extern "C" fn protover_compute_vote(list: *const Stringlist, threshold: c_int) -> *mut c_char {
    if list.is_null() {
        return allocate_and_copy_string("");
    }

    // Dereference of raw pointer requires an unsafe block. The pointer is
    // checked above to ensure it is not null.
    let data: Vec<String> = unsafe { (*list).get_list() };
    let hold: usize = threshold as usize;
    let mut proto_entries: Vec<UnvalidatedProtoEntry> = Vec::new();

    for datum in data {
        let entry: UnvalidatedProtoEntry = match datum.parse() {
            Ok(n) => n,
            Err(_) => continue,
        };
        proto_entries.push(entry);
    }
    let vote: UnvalidatedProtoEntry = ProtoverVote::compute(&proto_entries, &hold);

    allocate_and_copy_string(&vote.to_string())
}

/// Provide an interface for C to translate arguments and return types for
/// protover::is_supported_here
#[no_mangle]
pub extern "C" fn protover_is_supported_here(c_protocol: uint32_t, version: uint32_t) -> c_int {
    let protocol = match translate_to_rust(c_protocol) {
        Ok(n) => n,
        Err(_) => return 0,
    };

    let is_supported = is_supported_here(&protocol, &version);

    return if is_supported { 1 } else { 0 };
}

/// Provide an interface for C to translate arguments and return types for
/// protover::compute_for_old_tor
#[no_mangle]
pub extern "C" fn protover_compute_for_old_tor(version: *const c_char) -> *const c_char {
    let supported: &'static CStr;
    let empty: &'static CStr;

    empty = cstr!("");

    if version.is_null() {
        return empty.as_ptr();
    }

    // Require an unsafe block to read the version from a C string. The pointer
    // is checked above to ensure it is not null.
    let c_str: &CStr = unsafe { CStr::from_ptr(version) };

    let version = match c_str.to_str() {
        Ok(n) => n,
        Err(_) => return empty.as_ptr(),
    };

    supported = compute_for_old_tor_cstr(&version);
    supported.as_ptr()
}
