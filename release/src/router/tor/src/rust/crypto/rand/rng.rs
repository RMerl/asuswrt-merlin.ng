// Copyright (c) 2018-2019, The Tor Project, Inc.
// Copyright (c) 2018, isis agora lovecruft
// See LICENSE for licensing information

//! Wrappers for Tor's random number generators to provide implementations of
//! `rand_core` traits.

// This is the real implementation, in use in production, which calls into our C
// wrappers in /src/common/crypto_rand.c, which call into OpenSSL, system
// libraries, and make syscalls.
#[cfg(not(test))]
mod internal {
    use std::u64;

    use rand_core::impls::next_u32_via_fill;
    use rand_core::impls::next_u64_via_fill;
    use rand_core::CryptoRng;
    use rand_core::Error;
    use rand_core::RngCore;

    use external::c_tor_crypto_rand;
    use external::c_tor_crypto_seed_rng;
    use external::c_tor_crypto_strongest_rand;

    use tor_log::LogDomain;
    use tor_log::LogSeverity;

    /// Largest strong entropy request permitted.
    //
    // C_RUST_COUPLED: `MAX_STRONGEST_RAND_SIZE` /src/common/crypto_rand.c
    const MAX_STRONGEST_RAND_SIZE: usize = 256;

    /// A wrapper around OpenSSL's RNG.
    pub struct TorRng {
        // This private, zero-length field forces the struct to be treated the
        // same as its opaque C counterpart.
        _unused: [u8; 0],
    }

    /// Mark `TorRng` as being suitable for cryptographic purposes.
    impl CryptoRng for TorRng {}

    impl TorRng {
        // C_RUST_COUPLED: `crypto_seed_rng()` /src/common/crypto_rand.c
        #[allow(dead_code)]
        pub fn new() -> Self {
            if !c_tor_crypto_seed_rng() {
                tor_log_msg!(
                    LogSeverity::Warn,
                    LogDomain::General,
                    "TorRng::from_seed()",
                    "The RNG could not be seeded!"
                );
            }
            // XXX also log success at info level —isis
            TorRng { _unused: [0u8; 0] }
        }
    }

    impl RngCore for TorRng {
        // C_RUST_COUPLED: `crypto_strongest_rand()` /src/common/crypto_rand.c
        fn next_u32(&mut self) -> u32 {
            next_u32_via_fill(self)
        }

        // C_RUST_COUPLED: `crypto_strongest_rand()` /src/common/crypto_rand.c
        fn next_u64(&mut self) -> u64 {
            next_u64_via_fill(self)
        }

        // C_RUST_COUPLED: `crypto_strongest_rand()` /src/common/crypto_rand.c
        fn fill_bytes(&mut self, dest: &mut [u8]) {
            c_tor_crypto_rand(dest);
        }

        // C_RUST_COUPLED: `crypto_strongest_rand()` /src/common/crypto_rand.c
        fn try_fill_bytes(&mut self, dest: &mut [u8]) -> Result<(), Error> {
            Ok(self.fill_bytes(dest))
        }
    }

    /// A CSPRNG which hashes together randomness from OpenSSL's RNG and entropy
    /// obtained from the operating system.
    pub struct TorStrongestRng {
        // This private, zero-length field forces the struct to be treated the
        // same as its opaque C counterpart.
        _unused: [u8; 0],
    }

    /// Mark `TorRng` as being suitable for cryptographic purposes.
    impl CryptoRng for TorStrongestRng {}

    impl TorStrongestRng {
        // C_RUST_COUPLED: `crypto_seed_rng()` /src/common/crypto_rand.c
        #[allow(dead_code)]
        pub fn new() -> Self {
            if !c_tor_crypto_seed_rng() {
                tor_log_msg!(
                    LogSeverity::Warn,
                    LogDomain::General,
                    "TorStrongestRng::from_seed()",
                    "The RNG could not be seeded!"
                );
            }
            // XXX also log success at info level —isis
            TorStrongestRng { _unused: [0u8; 0] }
        }
    }

    impl RngCore for TorStrongestRng {
        // C_RUST_COUPLED: `crypto_strongest_rand()` /src/common/crypto_rand.c
        fn next_u32(&mut self) -> u32 {
            next_u32_via_fill(self)
        }

        // C_RUST_COUPLED: `crypto_strongest_rand()` /src/common/crypto_rand.c
        fn next_u64(&mut self) -> u64 {
            next_u64_via_fill(self)
        }

        // C_RUST_COUPLED: `crypto_strongest_rand()` /src/common/crypto_rand.c
        fn fill_bytes(&mut self, dest: &mut [u8]) {
            debug_assert!(dest.len() <= MAX_STRONGEST_RAND_SIZE);

            c_tor_crypto_strongest_rand(dest);
        }

        // C_RUST_COUPLED: `crypto_strongest_rand()` /src/common/crypto_rand.c
        fn try_fill_bytes(&mut self, dest: &mut [u8]) -> Result<(), Error> {
            Ok(self.fill_bytes(dest))
        }
    }
}

// For testing, we expose a pure-Rust implementation.
#[cfg(test)]
mod internal {
    // It doesn't matter if we pretend ChaCha is a CSPRNG in tests.
    pub use rand_crate::ChaChaRng as TorRng;
    pub use rand_crate::ChaChaRng as TorStrongestRng;
}

// Finally, expose the public functionality of whichever appropriate internal
// module.
pub use self::internal::*;
