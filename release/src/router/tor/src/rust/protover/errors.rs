// Copyright (c) 2018-2019, The Tor Project, Inc.
// Copyright (c) 2018, isis agora lovecruft
// See LICENSE for licensing information

//! Various errors which may occur during protocol version parsing.

use std::fmt;
use std::fmt::Display;

/// All errors which may occur during protover parsing routines.
#[derive(Clone, Copy, Debug, Eq, PartialEq, Hash)]
#[allow(missing_docs)] // See Display impl for error descriptions
pub enum ProtoverError {
    Overlap,
    LowGreaterThanHigh,
    Unparseable,
    ExceedsMax,
    ExceedsExpansionLimit,
    UnknownProtocol,
    ExceedsNameLimit,
    InvalidProtocol,
}

/// Descriptive error messages for `ProtoverError` variants.
impl Display for ProtoverError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match *self {
            ProtoverError::Overlap => write!(
                f,
                "Two or more (low, high) protover ranges would overlap once expanded."
            ),
            ProtoverError::LowGreaterThanHigh => write!(
                f,
                "The low in a (low, high) protover range was greater than high."
            ),
            ProtoverError::Unparseable => write!(f, "The protover string was unparseable."),
            ProtoverError::ExceedsMax => write!(
                f,
                "The high in a (low, high) protover range exceeds 63."
            ),
            ProtoverError::ExceedsExpansionLimit => write!(
                f,
                "The protover string would exceed the maximum expansion limit."
            ),
            ProtoverError::UnknownProtocol => write!(
                f,
                "A protocol in the protover string we attempted to parse is unknown."
            ),
            ProtoverError::ExceedsNameLimit => {
                write!(f, "An unrecognised protocol name was too long.")
            }
            ProtoverError::InvalidProtocol => {
                write!(f, "A protocol name includes invalid characters.")
            }
        }
    }
}
