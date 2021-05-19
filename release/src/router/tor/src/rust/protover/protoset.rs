// Copyright (c) 2018-2019, The Tor Project, Inc.
// Copyright (c) 2018, isis agora lovecruft
// See LICENSE for licensing information

//! Sets for lazily storing ordered, non-overlapping ranges of integers.

use std::cmp;
use std::iter;
use std::slice;
use std::str::FromStr;
use std::u32;

use errors::ProtoverError;

/// A single version number.
pub type Version = u32;

/// A `ProtoSet` stores an ordered `Vec<T>` of `(low, high)` pairs of ranges of
/// non-overlapping protocol versions.
///
/// # Examples
///
/// ```
/// use std::str::FromStr;
///
/// use protover::errors::ProtoverError;
/// use protover::protoset::ProtoSet;
/// use protover::protoset::Version;
///
/// # fn do_test() -> Result<ProtoSet, ProtoverError> {
/// let protoset: ProtoSet = ProtoSet::from_str("3-5,8")?;
///
/// // We could also equivalently call:
/// let protoset: ProtoSet = "3-5,8".parse()?;
///
/// assert!(protoset.contains(&4));
/// assert!(!protoset.contains(&7));
///
/// let expanded: Vec<Version> = protoset.clone().into();
///
/// assert_eq!(&expanded[..], &[3, 4, 5, 8]);
///
/// let contracted: String = protoset.clone().to_string();
///
/// assert_eq!(contracted, "3-5,8".to_string());
/// # Ok(protoset)
/// # }
/// # fn main() { do_test(); }  // wrap the test so we can use the ? operator
#[derive(Clone, Debug, Eq, PartialEq, Hash)]
pub struct ProtoSet {
    pub(crate) pairs: Vec<(Version, Version)>,
}

impl Default for ProtoSet {
    fn default() -> Self {
        let pairs: Vec<(Version, Version)> = Vec::new();

        ProtoSet { pairs }
    }
}

impl<'a> ProtoSet {
    /// Create a new `ProtoSet` from a slice of `(low, high)` pairs.
    ///
    /// # Inputs
    ///
    /// We do not assume the input pairs are deduplicated or ordered.
    pub fn from_slice(low_high_pairs: &'a [(Version, Version)]) -> Result<Self, ProtoverError> {
        let mut pairs: Vec<(Version, Version)> = Vec::with_capacity(low_high_pairs.len());

        for &(low, high) in low_high_pairs {
            pairs.push((low, high));
        }
        // Sort the pairs without reallocation and remove all duplicate pairs.
        pairs.sort_unstable();
        pairs.dedup();

        ProtoSet { pairs }.is_ok()
    }
}

/// Expand this `ProtoSet` to a `Vec` of all its `Version`s.
///
/// # Examples
///
/// ```
/// use std::str::FromStr;
/// use protover::protoset::ProtoSet;
/// use protover::protoset::Version;
/// # use protover::errors::ProtoverError;
///
/// # fn do_test() -> Result<Vec<Version>, ProtoverError> {
/// let protoset: ProtoSet = ProtoSet::from_str("3-5,21")?;
/// let versions: Vec<Version> = protoset.into();
///
/// assert_eq!(&versions[..], &[3, 4, 5, 21]);
/// #
/// # Ok(versions)
/// # }
/// # fn main() { do_test(); }  // wrap the test so we can use the ? operator
/// ```
impl Into<Vec<Version>> for ProtoSet {
    fn into(self) -> Vec<Version> {
        let mut versions: Vec<Version> = Vec::new();

        for &(low, high) in self.iter() {
            versions.extend(low..high + 1);
        }
        versions
    }
}

impl ProtoSet {
    /// Get an iterator over the `(low, high)` `pairs` in this `ProtoSet`.
    pub fn iter(&self) -> slice::Iter<(Version, Version)> {
        self.pairs.iter()
    }

    /// Expand this `ProtoSet` into a `Vec` of all its `Version`s.
    ///
    /// # Examples
    ///
    /// ```
    /// # use protover::errors::ProtoverError;
    /// use protover::protoset::ProtoSet;
    ///
    /// # fn do_test() -> Result<bool, ProtoverError> {
    /// let protoset: ProtoSet = "3-5,9".parse()?;
    ///
    /// assert_eq!(protoset.expand(), vec![3, 4, 5, 9]);
    ///
    /// let protoset: ProtoSet = "1,3,5-7".parse()?;
    ///
    /// assert_eq!(protoset.expand(), vec![1, 3, 5, 6, 7]);
    /// #
    /// # Ok(true)
    /// # }
    /// # fn main() { do_test(); }  // wrap the test so we can use the ? operator
    /// ```
    pub fn expand(self) -> Vec<Version> {
        self.into()
    }

    pub fn len(&self) -> usize {
        let mut length: usize = 0;

        for &(low, high) in self.iter() {
            length += (high as usize - low as usize) + 1;
        }

        length
    }

    /// Check that this `ProtoSet` is well-formed.
    ///
    /// This is automatically called in `ProtoSet::from_str()`.
    ///
    /// # Errors
    ///
    /// * `ProtoverError::LowGreaterThanHigh`: if its `pairs` were not
    ///   well-formed, i.e. a `low` in a `(low, high)` was higher than the
    ///   previous `high`,
    /// * `ProtoverError::Overlap`: if one or more of the `pairs` are
    ///   overlapping,
    /// * `ProtoverError::ExceedsMax`: if the number of versions when expanded
    ///   would exceed `MAX_PROTOCOLS_TO_EXPAND`, and
    ///
    /// # Returns
    ///
    /// A `Result` whose `Ok` is this `Protoset`, and whose `Err` is one of the
    /// errors enumerated in the Errors section above.
    fn is_ok(self) -> Result<ProtoSet, ProtoverError> {
        let mut last_high: Version = 0;

        for &(low, high) in self.iter() {
            if low == u32::MAX || high == u32::MAX {
                return Err(ProtoverError::ExceedsMax);
            }
            if low <= last_high {
                return Err(ProtoverError::Overlap);
            } else if low > high {
                return Err(ProtoverError::LowGreaterThanHigh);
            }
            last_high = high;
        }

        Ok(self)
    }

    /// Determine if this `ProtoSet` contains no `Version`s.
    ///
    /// # Returns
    ///
    /// * `true` if this `ProtoSet`'s length is zero, and
    /// * `false` otherwise.
    ///
    /// # Examples
    ///
    /// ```
    /// use protover::protoset::ProtoSet;
    ///
    /// let protoset: ProtoSet = ProtoSet::default();
    ///
    /// assert!(protoset.is_empty());
    /// ```
    pub fn is_empty(&self) -> bool {
        self.pairs.len() == 0
    }

    /// Determine if `version` is included within this `ProtoSet`.
    ///
    /// # Inputs
    ///
    /// * `version`: a `Version`.
    ///
    /// # Returns
    ///
    /// `true` if the `version` is contained within this set; `false` otherwise.
    ///
    /// # Examples
    ///
    /// ```
    /// # use protover::errors::ProtoverError;
    /// use protover::protoset::ProtoSet;
    ///
    /// # fn do_test() -> Result<ProtoSet, ProtoverError> {
    /// let protoset: ProtoSet = ProtoSet::from_slice(&[(0, 5), (7, 9), (13, 14)])?;
    ///
    /// assert!(protoset.contains(&5));
    /// assert!(!protoset.contains(&10));
    /// #
    /// # Ok(protoset)
    /// # }
    /// # fn main() { do_test(); }  // wrap the test so we can use the ? operator
    /// ```
    pub fn contains(&self, version: &Version) -> bool {
        for &(low, high) in self.iter() {
            if low <= *version && *version <= high {
                return true;
            }
        }
        false
    }

    /// Returns all the `Version`s in `self` which are not also in the `other`
    /// `ProtoSet`.
    ///
    /// # Examples
    ///
    /// ```
    /// # use protover::errors::ProtoverError;
    /// use protover::protoset::ProtoSet;
    ///
    /// # fn do_test() -> Result<bool, ProtoverError> {
    /// let protoset: ProtoSet = "1,3-6,10-12,15-16".parse()?;
    /// let other: ProtoSet = "2,5-7,9-11,14-20".parse()?;
    ///
    /// let subset: ProtoSet = protoset.and_not_in(&other);
    ///
    /// assert_eq!(subset.expand(), vec![1, 3, 4, 12]);
    /// #
    /// # Ok(true)
    /// # }
    /// # fn main() { do_test(); }  // wrap the test so we can use the ? operator
    /// ```
    pub fn and_not_in(&self, other: &Self) -> Self {
        if self.is_empty() || other.is_empty() {
            return self.clone();
        }

        let pairs = self.iter().flat_map(|&(lo, hi)| {
            let the_end = (hi + 1, hi + 1); // special case to mark the end of the range.
            let excluded_ranges = other
                .iter()
                .cloned() // have to be owned tuples, to match iter::once(the_end).
                .skip_while(move|&(_, hi2)| hi2 < lo) // skip the non-overlapping ranges.
                .take_while(move|&(lo2, _)| lo2 <= hi) // take all the overlapping ones.
                .chain(iter::once(the_end));

            let mut nextlo = lo;
            excluded_ranges.filter_map(move |(excluded_lo, excluded_hi)| {
                let pair = if nextlo < excluded_lo {
                    Some((nextlo, excluded_lo - 1))
                } else {
                    None
                };
                nextlo = cmp::min(excluded_hi, u32::MAX - 1) + 1;
                pair
            })
        });

        let pairs = pairs.collect();
        ProtoSet::is_ok(ProtoSet { pairs }).expect("should be already sorted")
    }
}

/// Largest allowed protocol version.
/// C_RUST_COUPLED: protover.c `MAX_PROTOCOL_VERSION`
const MAX_PROTOCOL_VERSION: Version = 63;

impl FromStr for ProtoSet {
    type Err = ProtoverError;

    /// Parse the unique version numbers supported by a subprotocol from a string.
    ///
    /// # Inputs
    ///
    /// * `version_string`, a string comprised of "[0-9,-]"
    ///
    /// # Returns
    ///
    /// A `Result` whose `Ok` value is a `ProtoSet` holding all of the unique
    /// version numbers.
    ///
    /// The returned `Result`'s `Err` value is an `ProtoverError` appropriate to
    /// the error.
    ///
    /// # Errors
    ///
    /// This function will error if:
    ///
    /// * the `version_string` is an equals (`"="`) sign,
    /// * the expansion of a version range produces an error (see
    ///   `expand_version_range`),
    /// * any single version number is not parseable as an `u32` in radix 10, or
    /// * there are greater than 2^16 version numbers to expand.
    ///
    /// # Examples
    ///
    /// ```
    /// use std::str::FromStr;
    ///
    /// use protover::errors::ProtoverError;
    /// use protover::protoset::ProtoSet;
    ///
    /// # fn do_test() -> Result<ProtoSet, ProtoverError> {
    /// let protoset: ProtoSet = ProtoSet::from_str("2-5,8")?;
    ///
    /// assert!(protoset.contains(&5));
    /// assert!(!protoset.contains(&10));
    ///
    /// // We can also equivalently call `ProtoSet::from_str` by doing (all
    /// // implementations of `FromStr` can be called this way, this one isn't
    /// // special):
    /// let protoset: ProtoSet = "4-6,12".parse()?;
    ///
    /// // Calling it (either way) can take really large ranges (up to `u32::MAX`):
    /// let protoset: ProtoSet = "1-70000".parse()?;
    /// let protoset: ProtoSet = "1-4294967296".parse()?;
    ///
    /// // There are lots of ways to get an `Err` from this function.  Here are
    /// // a few:
    /// assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("="));
    /// assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("-"));
    /// assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("not_an_int"));
    /// assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("3-"));
    /// assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("1-,4"));
    ///
    /// // An empty string is, however, legal, and results in an
    /// // empty `ProtoSet`:
    /// assert_eq!(Ok(ProtoSet::default()), ProtoSet::from_str(""));
    /// #
    /// # Ok(protoset)
    /// # }
    /// # fn main() { do_test(); }  // wrap the test so we can use the ? operator
    /// ```
    fn from_str(version_string: &str) -> Result<Self, Self::Err> {
        // If we were passed in an empty string, then return an empty ProtoSet.
        if version_string.is_empty() {
            return Ok(Self::default());
        }

        let mut pairs: Vec<(Version, Version)> = Vec::new();
        let pieces: ::std::str::Split<char> = version_string.split(',');

        for p in pieces {
            let (lo,hi) = if p.contains('-') {
                let mut pair = p.splitn(2, '-');

                let low = pair.next().ok_or(ProtoverError::Unparseable)?;
                let high = pair.next().ok_or(ProtoverError::Unparseable)?;

                let lo: Version = low.parse().or(Err(ProtoverError::Unparseable))?;
                let hi: Version = high.parse().or(Err(ProtoverError::Unparseable))?;

                (lo,hi)
            } else {
                let v: u32 = p.parse().or(Err(ProtoverError::Unparseable))?;

                (v, v)
            };

            if lo > MAX_PROTOCOL_VERSION || hi > MAX_PROTOCOL_VERSION {
                return Err(ProtoverError::ExceedsMax);
            }
            pairs.push((lo, hi));
        }

        ProtoSet::from_slice(&pairs[..])
    }
}

impl ToString for ProtoSet {
    /// Contracts a `ProtoSet` of versions into a string.
    ///
    /// # Returns
    ///
    /// A `String` representation of this `ProtoSet` in ascending order.
    fn to_string(&self) -> String {
        let mut final_output: Vec<String> = Vec::new();

        for &(lo, hi) in self.iter() {
            if lo != hi {
                debug_assert!(lo < hi);
                final_output.push(format!("{}-{}", lo, hi));
            } else {
                final_output.push(format!("{}", lo));
            }
        }
        final_output.join(",")
    }
}

/// Checks to see if there is a continuous range of integers, starting at the
/// first in the list. Returns the last integer in the range if a range exists.
///
/// # Inputs
///
/// `list`, an ordered  vector of `u32` integers of "[0-9,-]" representing the
/// supported versions for a single protocol.
///
/// # Returns
///
/// A `bool` indicating whether the list contains a range, starting at the first
/// in the list, a`Version` of the last integer in the range, and a `usize` of
/// the index of that version.
///
/// For example, if given vec![1, 2, 3, 5], find_range will return true,
/// as there is a continuous range, and 3, which is the last number in the
/// continuous range, and 2 which is the index of 3.
fn find_range(list: &Vec<Version>) -> (bool, Version, usize) {
    if list.len() == 0 {
        return (false, 0, 0);
    }

    let mut index: usize = 0;
    let mut iterable = list.iter().peekable();
    let mut range_end = match iterable.next() {
        Some(n) => *n,
        None => return (false, 0, 0),
    };

    let mut has_range = false;

    while iterable.peek().is_some() {
        let n = *iterable.next().unwrap();
        if n != range_end + 1 {
            break;
        }

        has_range = true;
        range_end = n;
        index += 1;
    }

    (has_range, range_end, index)
}

impl From<Vec<Version>> for ProtoSet {
    fn from(mut v: Vec<Version>) -> ProtoSet {
        let mut version_pairs: Vec<(Version, Version)> = Vec::new();

        v.sort_unstable();
        v.dedup();

        'vector: while !v.is_empty() {
            let (has_range, end, index): (bool, Version, usize) = find_range(&v);

            if has_range {
                let first: Version = match v.first() {
                    Some(x) => *x,
                    None => continue,
                };
                let last: Version = match v.get(index) {
                    Some(x) => *x,
                    None => continue,
                };
                debug_assert!(last == end, format!("last = {}, end = {}", last, end));

                version_pairs.push((first, last));
                v = v.split_off(index + 1);

                if v.len() == 0 {
                    break 'vector;
                }
            } else {
                let last: Version = match v.get(index) {
                    Some(x) => *x,
                    None => continue,
                };
                version_pairs.push((last, last));
                v.remove(index);
            }
        }
        ProtoSet::from_slice(&version_pairs[..]).unwrap_or(ProtoSet::default())
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_find_range() {
        assert_eq!((false, 0, 0), find_range(&vec![]));
        assert_eq!((false, 1, 0), find_range(&vec![1]));
        assert_eq!((true, 2, 1), find_range(&vec![1, 2]));
        assert_eq!((true, 3, 2), find_range(&vec![1, 2, 3]));
        assert_eq!((true, 3, 2), find_range(&vec![1, 2, 3, 5]));
    }

    macro_rules! assert_contains_each {
        ($protoset:expr, $versions:expr) => {
            for version in $versions {
                assert!($protoset.contains(version));
            }
        };
    }

    macro_rules! test_protoset_contains_versions {
        ($list:expr, $str:expr) => {
            let versions: &[Version] = $list;
            let protoset: Result<ProtoSet, ProtoverError> = ProtoSet::from_str($str);

            assert!(protoset.is_ok());
            let p = protoset.unwrap();
            assert_contains_each!(p, versions);
        };
    }

    #[test]
    fn test_versions_from_str() {
        test_protoset_contains_versions!(&[], "");
        test_protoset_contains_versions!(&[1], "1");
        test_protoset_contains_versions!(&[1, 2], "1,2");
        test_protoset_contains_versions!(&[1, 2, 3], "1-3");
        test_protoset_contains_versions!(&[1, 2, 5], "1-2,5");
        test_protoset_contains_versions!(&[1, 3, 4, 5], "1,3-5");
        test_protoset_contains_versions!(&[42, 55, 56, 57, 58], "42,55-58");
    }

    #[test]
    fn test_versions_from_str_ab() {
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("a,b"));
    }

    #[test]
    fn test_versions_from_str_negative_1() {
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("-1"));
    }

    #[test]
    fn test_versions_from_str_commas() {
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str(","));
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("1,,2"));
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("1,2,"));
    }

    #[test]
    fn test_versions_from_str_hyphens() {
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("--1"));
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("-1-2"));
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("1--2"));
    }

    #[test]
    fn test_versions_from_str_triple() {
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("1-2-3"));
    }

    #[test]
    fn test_versions_from_str_1exclam() {
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("1,!"));
    }

    #[test]
    fn test_versions_from_str_percent_equal() {
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("%="));
    }

    #[test]
    fn test_versions_from_str_whitespace() {
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("1,2\n"));
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("1\r,2"));
        assert_eq!(Err(ProtoverError::Unparseable), ProtoSet::from_str("1,\t2"));
    }

    #[test]
    fn test_versions_from_str_overlap() {
        assert_eq!(Err(ProtoverError::Overlap), ProtoSet::from_str("1-3,2-4"));
    }

    #[test]
    fn test_versions_from_slice_overlap() {
        assert_eq!(
            Err(ProtoverError::Overlap),
            ProtoSet::from_slice(&[(1, 3), (2, 4)])
        );
    }

    #[test]
    fn test_versions_from_str_max() {
        assert_eq!(
            Err(ProtoverError::ExceedsMax),
            ProtoSet::from_str("4294967295")
        );
    }

    #[test]
    fn test_versions_from_slice_max() {
        assert_eq!(
            Err(ProtoverError::ExceedsMax),
            ProtoSet::from_slice(&[(4294967295, 4294967295)])
        );
    }

    #[test]
    fn test_protoset_contains() {
        let protoset: ProtoSet = ProtoSet::from_slice(&[(1, 5), (7, 9), (13, 14)]).unwrap();

        for x in 1..6 {
            assert!(protoset.contains(&x), format!("should contain {}", x));
        }
        for x in 7..10 {
            assert!(protoset.contains(&x), format!("should contain {}", x));
        }
        for x in 13..15 {
            assert!(protoset.contains(&x), format!("should contain {}", x));
        }

        for x in [6, 10, 11, 12, 15, 42, 43, 44, 45, 1234584].iter() {
            assert!(!protoset.contains(&x), format!("should not contain {}", x));
        }
    }

    #[test]
    fn test_protoset_contains_1_3() {
        let protoset: ProtoSet = ProtoSet::from_slice(&[(1, 3)]).unwrap();

        for x in 1..4 {
            assert!(protoset.contains(&x), format!("should contain {}", x));
        }
    }

    macro_rules! assert_protoset_from_vec_contains_all {
        ($($x:expr),*) => (
            let vec: Vec<Version> = vec!($($x),*);
            let protoset: ProtoSet = vec.clone().into();

            for x in vec.iter() {
                assert!(protoset.contains(&x));
            }
        )
    }

    #[test]
    fn test_protoset_from_vec_123() {
        assert_protoset_from_vec_contains_all!(1, 2, 3);
    }

    #[test]
    fn test_protoset_from_vec_1_315() {
        assert_protoset_from_vec_contains_all!(1, 2, 3, 15);
    }

    #[test]
    fn test_protoset_from_vec_unordered() {
        let v: Vec<Version> = vec![2, 3, 8, 4, 3, 9, 7, 2];
        let ps: ProtoSet = v.into();

        assert_eq!(ps.to_string(), "2-4,7-9");
    }

    #[test]
    fn test_protoset_into_vec() {
        let ps: ProtoSet = "1-13,42".parse().unwrap();
        let v: Vec<Version> = ps.into();

        assert!(v.contains(&7));
        assert!(v.contains(&42));
    }
}

#[cfg(all(test, feature = "bench"))]
mod bench {
    use super::*;
}
