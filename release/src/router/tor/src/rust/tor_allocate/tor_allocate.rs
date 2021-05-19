// Copyright (c) 2016-2019, The Tor Project, Inc. */
// See LICENSE for licensing information */
// No-op defined purely for testing at the module level
use libc::c_char;

use libc::c_void;
#[cfg(not(feature = "testing"))]
use std::{mem, ptr, slice};

// Define a no-op implementation for testing Rust modules without linking to C
#[cfg(feature = "testing")]
pub fn allocate_and_copy_string(s: &str) -> *mut c_char {
    use std::ffi::CString;
    CString::new(s).unwrap().into_raw()
}

// Defined only for tests, used for testing purposes, so that we don't need
// to link to tor C files. Uses the system allocator
#[cfg(test)]
unsafe extern "C" fn tor_malloc_(size: usize) -> *mut c_void {
    use libc::malloc;
    malloc(size)
}

#[cfg(all(not(test), not(feature = "testing")))]
extern "C" {
    fn tor_malloc_(size: usize) -> *mut c_void;
}

/// Allocate memory using tor_malloc_ and copy an existing string into the
/// allocated buffer, returning a pointer that can later be called in C.
///
/// # Inputs
///
/// * `src`, a reference to a String.
///
/// # Returns
///
/// A `*mut c_char` that should be freed by tor_free in C
///
#[cfg(not(feature = "testing"))]
pub fn allocate_and_copy_string(src: &str) -> *mut c_char {
    let bytes: &[u8] = src.as_bytes();

    let size = mem::size_of_val::<[u8]>(bytes);
    let size_one_byte = mem::size_of::<u8>();

    // handle integer overflow when adding one to the calculated length
    let size_with_null_byte = match size.checked_add(size_one_byte) {
        Some(n) => n,
        None => return ptr::null_mut(),
    };

    let dest = unsafe { tor_malloc_(size_with_null_byte) as *mut u8 };

    if dest.is_null() {
        return ptr::null_mut();
    }

    unsafe { ptr::copy_nonoverlapping(bytes.as_ptr(), dest, size) };

    // set the last byte as null, using the ability to index into a slice
    // rather than doing pointer arithmetic
    let slice = unsafe { slice::from_raw_parts_mut(dest, size_with_null_byte) };
    slice[size] = 0; // add a null terminator

    dest as *mut c_char
}

#[cfg(test)]
mod test {

    #[test]
    fn test_allocate_and_copy_string_with_empty() {
        use libc::{c_void, free};
        use std::ffi::CStr;

        use tor_allocate::allocate_and_copy_string;

        let allocated_empty = allocate_and_copy_string("");

        let allocated_empty_rust = unsafe { CStr::from_ptr(allocated_empty).to_str().unwrap() };

        assert_eq!("", allocated_empty_rust);

        unsafe { free(allocated_empty as *mut c_void) };
    }

    #[test]
    fn test_allocate_and_copy_string_with_not_empty_string() {
        use libc::{c_void, free};
        use std::ffi::CStr;

        use tor_allocate::allocate_and_copy_string;

        let allocated_empty = allocate_and_copy_string("foo bar biz");

        let allocated_empty_rust = unsafe { CStr::from_ptr(allocated_empty).to_str().unwrap() };

        assert_eq!("foo bar biz", allocated_empty_rust);

        unsafe { free(allocated_empty as *mut c_void) };
    }
}
