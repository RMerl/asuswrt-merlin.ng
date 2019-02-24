#
# The hidden service subsystem has two type of index. The first type is a
# value that each node in the network gets assigned to using their identity
# key which is their position in the hashring. (hs_build_hsdir_index()).
#
# The second type is a value that both the client and service computes to
# store/fetch the descriptor on the hashring. (hs_build_hs_index()).
#

import sys
import hashlib
import struct
import base64

# Python 3.6+, the SHA3 is available in hashlib natively. Else this requires
# the pysha3 package (pip install pysha3).
if sys.version_info < (3, 6):
    import sha3
    # Test vector to make sure the right sha3 version will be used. pysha3 < 1.0
    # used the old Keccak implementation. During the finalization of SHA3, NIST
    # changed the delimiter suffix from 0x01 to 0x06. The Keccak sponge function
    # stayed the same. pysha3 1.0 provides the previous Keccak hash, too.
    TEST_VALUE = "e167f68d6563d75bb25f3aa49c29ef612d41352dc00606de7cbd630bb2665f51"
    if TEST_VALUE != sha3.sha3_256(b"Hello World").hexdigest():
        print("pysha3 version is < 1.0. Please install from:")
        print("https://github.com/tiran/pysha3https://github.com/tiran/pysha3")
        sys.exit(1)

# The first index we'll build is the position index in the hashring that is
# constructed by the hs_build_hsdir_index() function. Construction is:
#   SHA3-256("node-idx" | node_identity |
#            shared_random_value | INT_8(period_length) | INT_8(period_num) )

PREFIX = "node-idx".encode()
# 32 bytes ed25519 pubkey.
IDENTITY = ("\x42" * 32).encode()
# SRV is 32 bytes.
SRV = ("\x43" * 32).encode()
# Time period length is a 8 bytes value.
PERIOD_LEN = 1440
# Period number is a 8 bytes value.
PERIOD_NUM = 42

data = struct.pack('!8s32s32sQQ', PREFIX, IDENTITY, SRV, PERIOD_NUM,
                                  PERIOD_LEN)
hsdir_index = hashlib.sha3_256(data).hexdigest()

print("[hs_build_hsdir_index] %s" % (hsdir_index))

# The second index we'll build is where the HS stores and the client fetches
# the descriptor on the hashring. It is constructed by the hs_build_hs_index()
# function and the construction is:
#   SHA3-256("store-at-idx" | blinded_public_key |
#            INT_8(replicanum) | INT_8(period_num) | INT_8(period_length) )

PREFIX = "store-at-idx".encode()
# 32 bytes ed25519 pubkey.
PUBKEY = ("\x42" * 32).encode()
# Replica number is a 8 bytes value.
REPLICA_NUM = 1
# Time period length is a 8 bytes value.
PERIOD_LEN = 1440
# Period number is a 8 bytes value.
PERIOD_NUM = 42

data = struct.pack('!12s32sQQQ', PREFIX, PUBKEY, REPLICA_NUM, PERIOD_LEN,
                                   PERIOD_NUM)
hs_index = hashlib.sha3_256(data).hexdigest()

print("[hs_build_hs_index]   %s" % (hs_index))
