// Copyright (c) 2016-2019, The Tor Project, Inc. */
// See LICENSE for licensing information */

//! FFI functions to announce Rust support during tor startup, only to be
//! called from C.
//!

use tor_log::{LogDomain, LogSeverity};

/// Returns a short string to announce Rust support during startup.
///
/// # Examples
/// ```c
/// char *rust_str = rust_welcome_string();
/// printf("%s", rust_str);
/// tor_free(rust_str);
/// ```
#[no_mangle]
pub extern "C" fn rust_log_welcome_string() {
    tor_log_msg!(
        LogSeverity::Notice,
        LogDomain::General,
        "rust_log_welcome_string",
        "Tor is running with Rust integration. Please report \
         any bugs you encounter."
    );
}
