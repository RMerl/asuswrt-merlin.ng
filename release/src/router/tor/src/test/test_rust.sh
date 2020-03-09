#!/bin/sh
# Test all Rust crates

set -e

export LSAN_OPTIONS=suppressions=${abs_top_srcdir:-../../..}/src/test/rust_supp.txt

# When testing Cargo we pass a number of very specific linker flags down
# through Cargo. We do not, however, want these flags to affect things like
# build scripts, only the tests that we're compiling. To ensure this happens
# we unconditionally pass `--target` into Cargo, ensuring that `RUSTFLAGS` in
# the environment won't make their way into build scripts.
rustc_host=$(rustc -vV | grep host | sed 's/host: //')

for cargo_toml_dir in "${abs_top_srcdir:-../../..}"/src/rust/*; do
    if [ -e "${cargo_toml_dir}/Cargo.toml" ]; then
        # shellcheck disable=SC2086
	cd "${abs_top_builddir:-../../..}/src/rust" && \
	    CARGO_TARGET_DIR="${abs_top_builddir:-../../..}/src/rust/target" \
	    "${CARGO:-cargo}" test "${CARGO_ONLINE-'--frozen'}" \
            --features "test_linking_hack" \
            --target "$rustc_host" \
	    ${EXTRA_CARGO_OPTIONS} \
	    --manifest-path "${cargo_toml_dir}/Cargo.toml" || exitcode=1
    fi
done

exit $exitcode
