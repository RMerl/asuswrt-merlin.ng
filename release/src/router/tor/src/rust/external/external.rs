// Copyright (c) 2016-2019, The Tor Project, Inc. */
// See LICENSE for licensing information */

use libc::{c_char, c_int};
use std::ffi::CString;

extern "C" {
    fn tor_version_as_new_as(platform: *const c_char, cutoff: *const c_char) -> c_int;
}

/// Wrap calls to tor_version_as_new_as, defined in routerparse.c
pub fn c_tor_version_as_new_as(platform: &str, cutoff: &str) -> bool {
    // CHK: These functions should log a warning if an error occurs. This
    // can be added when integration with tor's logger is added to rust
    let c_platform = match CString::new(platform) {
        Ok(n) => n,
        Err(_) => return false,
    };

    let c_cutoff = match CString::new(cutoff) {
        Ok(n) => n,
        Err(_) => return false,
    };

    let result: c_int = unsafe { tor_version_as_new_as(c_platform.as_ptr(), c_cutoff.as_ptr()) };

    result == 1
}

extern "C" {
    fn tor_is_using_nss() -> c_int;
}

/// Return true if Tor was built to use NSS.
pub fn c_tor_is_using_nss() -> bool {
    0 != unsafe { tor_is_using_nss() }
}
