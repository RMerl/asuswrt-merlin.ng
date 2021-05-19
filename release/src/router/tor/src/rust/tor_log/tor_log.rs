// Copyright (c) 2016-2019, The Tor Project, Inc. */
// See LICENSE for licensing information */

// Note that these functions are untested due to the fact that there are no
// return variables to test and they are calling into a C API.

/// The related domain which the logging message is relevant. For example,
/// log messages relevant to networking would use LogDomain::LdNet, whereas
/// general messages can use LdGeneral.
#[derive(Eq, PartialEq)]
pub enum LogDomain {
    Net,
    General,
}

/// The severity level at which to log messages.
#[derive(Eq, PartialEq)]
pub enum LogSeverity {
    Notice,
    Warn,
}

/// Main entry point for Rust modules to log messages.
///
/// # Inputs
///
/// * A `severity` of type LogSeverity, which defines the level of severity the
/// message will be logged.
/// * A `domain` of type LogDomain, which defines the domain the log message
/// will be associated with.
/// * A `function` of type &str, which defines the name of the function where
/// the message is being logged. There is a current RFC for a macro that
/// defines function names. When it is, we should use it. See
/// https://github.com/rust-lang/rfcs/pull/1719
/// * A `message` of type &str, which is the log message itself.
#[macro_export]
macro_rules! tor_log_msg {
    ($severity: path,
     $domain: path,
     $function: expr,
     $($message:tt)*) =>
    {
        {
            let msg = format!($($message)*);
            $crate::tor_log_msg_impl($severity, $domain, $function, msg)
        }
    };
}

#[inline]
pub fn tor_log_msg_impl(severity: LogSeverity, domain: LogDomain, function: &str, message: String) {
    use std::ffi::CString;

    /// Default function name to log in case of errors when converting
    /// a function name to a CString
    const ERR_LOG_FUNCTION: &str = "tor_log_msg";

    /// Default message to log in case of errors when converting a log
    /// message to a CString
    const ERR_LOG_MSG: &str = "Unable to log message from Rust \
                               module due to error when converting to CString";

    let func = match CString::new(function) {
        Ok(n) => n,
        Err(_) => CString::new(ERR_LOG_FUNCTION).unwrap(),
    };

    let msg = match CString::new(message) {
        Ok(n) => n,
        Err(_) => CString::new(ERR_LOG_MSG).unwrap(),
    };

    // Bind to a local variable to preserve ownership. This is essential so
    // that ownership is guaranteed until these local variables go out of scope
    let func_ptr = func.as_ptr();
    let msg_ptr = msg.as_ptr();

    let c_severity = unsafe { log::translate_severity(severity) };
    let c_domain = unsafe { log::translate_domain(domain) };

    unsafe { log::tor_log_string(c_severity, c_domain, func_ptr, msg_ptr) }
}

/// This implementation is used when compiling for actual use, as opposed to
/// testing.
#[cfg(not(test))]
pub mod log {
    use super::LogDomain;
    use super::LogSeverity;
    use libc::{c_char, c_int};

    /// Severity log types. These mirror definitions in src/lib/log/log.h
    /// C_RUST_COUPLED: src/lib/log/log.c, log domain types
    extern "C" {
        static LOG_WARN_: c_int;
        static LOG_NOTICE_: c_int;
    }

    /// Domain log types. These mirror definitions in src/lib/log/log.h
    /// C_RUST_COUPLED: src/lib/log/log.c, log severity types
    extern "C" {
        static LD_NET_: u64;
        static LD_GENERAL_: u64;
    }

    /// Translate Rust definitions of log domain levels to C. This exposes a 1:1
    /// mapping between types.
    #[inline]
    pub unsafe fn translate_domain(domain: LogDomain) -> u64 {
        match domain {
            LogDomain::Net => LD_NET_,
            LogDomain::General => LD_GENERAL_,
        }
    }

    /// Translate Rust definitions of log severity levels to C. This exposes a
    /// 1:1 mapping between types.
    #[inline]
    pub unsafe fn translate_severity(severity: LogSeverity) -> c_int {
        match severity {
            LogSeverity::Warn => LOG_WARN_,
            LogSeverity::Notice => LOG_NOTICE_,
        }
    }

    /// The main entry point into Tor's logger. When in non-test mode, this
    /// will link directly with `tor_log_string` in torlog.c
    extern "C" {
        pub fn tor_log_string(
            severity: c_int,
            domain: u64,
            function: *const c_char,
            string: *const c_char,
        );
    }
}

/// This module exposes no-op functionality for testing other Rust modules
/// without linking to C.
#[cfg(test)]
pub mod log {
    use super::LogDomain;
    use super::LogSeverity;
    use libc::{c_char, c_int};

    pub static mut LAST_LOGGED_FUNCTION: *mut String = 0 as *mut String;
    pub static mut LAST_LOGGED_MESSAGE: *mut String = 0 as *mut String;

    pub unsafe fn tor_log_string(
        _severity: c_int,
        _domain: u32,
        function: *const c_char,
        message: *const c_char,
    ) {
        use std::ffi::CStr;

        let f = CStr::from_ptr(function);
        let fct = match f.to_str() {
            Ok(n) => n,
            Err(_) => "",
        };
        LAST_LOGGED_FUNCTION = Box::into_raw(Box::new(String::from(fct)));

        let m = CStr::from_ptr(message);
        let msg = match m.to_str() {
            Ok(n) => n,
            Err(_) => "",
        };
        LAST_LOGGED_MESSAGE = Box::into_raw(Box::new(String::from(msg)));
    }

    pub unsafe fn translate_domain(_domain: LogDomain) -> u32 {
        1
    }

    pub unsafe fn translate_severity(_severity: LogSeverity) -> c_int {
        1
    }
}

#[cfg(test)]
mod test {
    use tor_log::log::{LAST_LOGGED_FUNCTION, LAST_LOGGED_MESSAGE};
    use tor_log::*;

    #[test]
    fn test_get_log_message() {
        {
            fn test_macro() {
                tor_log_msg!(
                    LogSeverity::Warn,
                    LogDomain::Net,
                    "test_macro",
                    "test log message {}",
                    "a",
                );
            }

            test_macro();

            let function = unsafe { Box::from_raw(LAST_LOGGED_FUNCTION) };
            assert_eq!("test_macro", *function);

            let message = unsafe { Box::from_raw(LAST_LOGGED_MESSAGE) };
            assert_eq!("test log message a", *message);
        }

        // test multiple inputs into the log message
        {
            fn test_macro() {
                tor_log_msg!(
                    LogSeverity::Warn,
                    LogDomain::Net,
                    "next_test_macro",
                    "test log message {} {} {} {} {}",
                    1,
                    2,
                    3,
                    4,
                    5
                );
            }

            test_macro();

            let function = unsafe { Box::from_raw(LAST_LOGGED_FUNCTION) };
            assert_eq!("next_test_macro", *function);

            let message = unsafe { Box::from_raw(LAST_LOGGED_MESSAGE) };
            assert_eq!("test log message 1 2 3 4 5", *message);
        }

        // test how a long log message will be formatted
        {
            fn test_macro() {
                tor_log_msg!(
                    LogSeverity::Warn,
                    LogDomain::Net,
                    "test_macro",
                    "{}",
                    "All the world's a stage, and all the men and women \
                     merely players: they have their exits and their \
                     entrances; and one man in his time plays many parts, his \
                     acts being seven ages."
                );
            }

            test_macro();

            let expected_string = "All the world's a \
                                   stage, and all the men \
                                   and women merely players: \
                                   they have their exits and \
                                   their entrances; and one man \
                                   in his time plays many parts, \
                                   his acts being seven ages.";

            let function = unsafe { Box::from_raw(LAST_LOGGED_FUNCTION) };
            assert_eq!("test_macro", *function);

            let message = unsafe { Box::from_raw(LAST_LOGGED_MESSAGE) };
            assert_eq!(expected_string, *message);
        }
    }
}
