#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0+

"""
Simple tool to swap the byte endianness of a binary file.
"""

import argparse
import io

def parse_args():
    """Parse command line arguments."""
    description = "Swap endianness of given input binary and write to output binary."

    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("input_bin", type=str, help="input binary")
    parser.add_argument("output_bin", type=str, help="output binary")
    parser.add_argument("-c", action="store", dest="chunk_size", type=int,
        default=io.DEFAULT_BUFFER_SIZE, help="chunk size for reading")

    return parser.parse_args()

def swap_chunk(chunk_orig):
    """Swap byte endianness of the given chunk.

    Returns:
        swapped chunk
    """
    chunk = bytearray(chunk_orig)

    # align to 4 bytes and pad with 0x0
    chunk_len = len(chunk)
    pad_len = chunk_len % 4
    if pad_len > 0:
        chunk += b'\x00' * (4 - pad_len)

    chunk[0::4], chunk[1::4], chunk[2::4], chunk[3::4] =\
        chunk[3::4], chunk[2::4], chunk[1::4], chunk[0::4]

    return chunk

def main():
    args = parse_args()

    with open(args.input_bin, "rb") as input_bin:
        with open(args.output_bin, "wb") as output_bin:
            while True:
                chunk = bytearray(input_bin.read(args.chunk_size))
                if not chunk:
                    break

                output_bin.write(swap_chunk(chunk))

if __name__ == '__main__':
    main()
