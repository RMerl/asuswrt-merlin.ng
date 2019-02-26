// Copyright (c) 2016-2019, The Tor Project, Inc. */
// See LICENSE for licensing information */

use libc::{c_char, c_int};
use std::ffi::CStr;
use std::slice;

/// Smartlists are a type used in C code in tor to define a collection of a
/// generic type, which has a capacity and a number used. Each Smartlist
/// defines how to extract the list of values from the underlying C structure
///
/// Implementations are required to have a C representation, as this module
/// serves purely to translate smartlists as defined in tor to vectors in Rust.
pub trait Smartlist<T> {
    fn get_list(&self) -> Vec<T>;
}

#[repr(C)]
pub struct Stringlist {
    pub list: *const *const c_char,
    pub num_used: c_int,
    pub capacity: c_int,
}

impl Smartlist<String> for Stringlist {
    fn get_list(&self) -> Vec<String> {
        let empty: Vec<String> = Vec::new();
        let mut rust_list: Vec<String> = Vec::new();

        if self.list.is_null() || self.num_used == 0 {
            return empty;
        }

        // unsafe, as we need to extract the smartlist list into a vector of
        // pointers, and then transform each element into a Rust string.
        let elems: &[*const c_char] =
            unsafe { slice::from_raw_parts(self.list, self.num_used as usize) };

        for elem in elems.iter() {
            if elem.is_null() {
                continue;
            }

            // unsafe, as we need to create a cstring from the referenced
            // element
            let c_string = unsafe { CStr::from_ptr(*elem) };

            let r_string = match c_string.to_str() {
                Ok(n) => n,
                Err(_) => return empty,
            };

            rust_list.push(String::from(r_string));
        }

        rust_list
    }
}

// TODO: CHK: this module maybe should be tested from a test in C with a
// smartlist as defined in tor.
#[cfg(test)]
mod test {
    #[test]
    fn test_get_list_of_strings() {
        extern crate libc;

        use libc::c_char;
        use std::ffi::CString;

        use super::Smartlist;
        use super::Stringlist;

        {
            // test to verify that null pointers are gracefully handled
            use std::ptr;

            let sl = Stringlist {
                list: ptr::null(),
                num_used: 0,
                capacity: 0,
            };

            let data = sl.get_list();
            assert_eq!(0, data.len());
        }

        {
            let args = vec![String::from("a"), String::from("b")];

            // for each string, transform  it into a CString
            let c_strings: Vec<_> = args
                .iter()
                .map(|arg| CString::new(arg.as_str()).unwrap())
                .collect();

            // then, collect a pointer for each CString
            let p_args: Vec<_> = c_strings.iter().map(|arg| arg.as_ptr()).collect();

            let p: *const *const c_char = p_args.as_ptr();

            // This is the representation that we expect when receiving a
            // smartlist at the Rust/C FFI layer.
            let sl = Stringlist {
                list: p,
                num_used: 2,
                capacity: 2,
            };

            let data = sl.get_list();
            assert_eq!("a", &data[0]);
            assert_eq!("b", &data[1]);
        }
    }
}
