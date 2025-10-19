# test.py - Runs tests for the fscryptctl binary
#
# Copyright 2017 Google Inc.
# Author: Joe Richey (joerichey@google.com)
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

import keyutils
import pytest
import os

key_length = 64 # 512 bit keys
test_env_var = "TEST_FILESYSTEM_ROOT"
test_data = "some test file data"

# Key descriptor test cases generated using:
#   printf <key_contents>
#       | sha512sum --binary
#       | head --bytes=128
#       | xxd -r -p
#       | sha512sum --binary
#       | head --bytes=16

# The first and second groups of 256 bits must be different.
half_length = key_length/2
test_key = (b'a' * half_length) + (b'1' * half_length)
test_descriptor = b'e355a76a11a1be18'
key_tests = [
    (test_key, test_descriptor),
    ((b'b' * half_length) + (b'2' * half_length), b'89bb6950c691e91a'),
    ((b'c' * half_length) + (b'3' * half_length), b'338561500b313743'),
]


def first_line(text):
    """ returns the first line of text without a newline """
    return text.split('\n', 1)[0]


def invoke(binary_path, stdin, *args):
    """ Calls the binary at binary_path with the specified stdin and arguments.
    If the command completes successfully, returns the first line of stdin. On
    failure, raises an error with the first line of stderr. """
    from subprocess import Popen, PIPE, CalledProcessError

    p = Popen([binary_path] + list(args), stdin=PIPE, stdout=PIPE, stderr=PIPE)
    stdout, stderr = p.communicate(stdin)

    # Check for errors
    if p.returncode != 0:
        if stderr != "":
            raise SystemError(first_line(stderr))
        else:
            raise CalledProcessError(p.returncode, binary_path)

    return first_line(stdout)

def device(mountpoint):
    """ Returns the device of a mountpoint. """
    from subprocess import check_output
    for line in check_output(['mount', '-l']).split('\n'):
        parts = line.split()
        if len(parts) > 2 and parts[2] == mountpoint:
            return parts[0]
    raise SystemError("No device for: " + mountpoint)

def remount(mountpoint):
    """ Remounts the filesystem and clears the inode cache. """
    dev = device(mountpoint)
    invoke("umount", "", mountpoint)
    invoke("mount", "", "-t", "ext4", dev, mountpoint)

def write_file(path):
    """ writes some sample data to the file at path """
    with open(path, "w+") as f:
        f.write(test_data)


def read_file(path):
    """ reads some data from the file at path """
    with open(path, "r") as f:
        return f.read()


@pytest.fixture(scope="module")
def program():
    """ This fixture provides a lambda which, when called, passes the first
    argument as stdin and the subsequent arguments as command-line arguments to
    the fscryptctl binary. """

    # Get the fscryptctl binary path
    dir_path = os.path.dirname(os.path.realpath(__file__))
    binary_path = os.path.join(dir_path, "fscryptctl")

    def invoke_fscryptctl(stdin, *args):
        return invoke(binary_path, stdin, *args)

    return invoke_fscryptctl


@pytest.yield_fixture(scope='function')
def keyring():
    """ This fixture creates a new anonymous session keyring and subscribes the
    process to it. The id of this keyring is returned. On cleanup, the keyring
    will be cleared. """
    keyring_id = keyutils.join_session_keyring()
    yield keyring_id
    keyutils.clear(keyring_id)


@pytest.yield_fixture(scope='module')
def filesystem():
    """ This fixture returns the mountpoint of the designated testing
    filesystem. Before the path is returned and on cleanup, the filesystem is
    unmounted and remounted to clear any cached keys. Throws if the required
    environment variable is not set. """
    mountpoint = os.environ.get(test_env_var)
    if mountpoint == None:
        raise SystemError("Need to set: " + test_env_var)

    remount(mountpoint)
    yield mountpoint
    remount(mountpoint)


@pytest.yield_fixture(scope='function')
def directory(filesystem):
    """ Returns a new testing directory on the testing filesystem. """
    from shutil import rmtree
    test_dir = os.path.join(filesystem, "test")

    os.mkdir(test_dir)
    yield test_dir
    rmtree(test_dir)


def test_get_descriptor(program):
    """ Tests that the get_descriptor command returns the expected value """
    for key, expected_descriptor in key_tests:
        output = program(key, "get_descriptor")
        assert output == expected_descriptor


def test_insert_key(program, keyring):
    """ Tests that insert_key command actually puts the keys in the keyring """
    for key, descriptor in key_tests:
        # Inserting should give the appropriate descriptor
        output = program(key, "insert_key")
        assert output == descriptor

    # After insertion, check that all three keys are there
    for _, descriptor in key_tests:
        # Key should be in the keyring
        id1 = keyutils.search(keyring, b'fscrypt:' +
                              descriptor, keyType=b'logon')
        assert id1 != None

        # Accessing the session keyring should give the same result
        id2 = keyutils.search(keyutils.KEY_SPEC_SESSION_KEYRING, b'fscrypt:' +
                              descriptor, keyType=b'logon')
        assert id1 == id2

        # There should not be keys of type user
        id3 = keyutils.search(keyutils.KEY_SPEC_SESSION_KEYRING, b'fscrypt:' +
                              descriptor)
        assert id3 == None


def test_insert_flags(program, keyring):
    """ tests that the insertion flags give the correct prefixes """
    for flag, prefix in [("--ext4", b'ext4:'), ("--f2fs", b'f2fs:')]:
        output = program(test_key, "insert_key", flag)
        assert output == test_descriptor

        key_id = keyutils.search(
            keyring, prefix + test_descriptor, keyType=b'logon')
        assert key_id != None


def test_set_get_policy(program, directory):
    """ tests that setting a policy on a directory then getting the policy
    does not return an error. """
    program("", "set_policy", test_descriptor, directory)
    program("", "get_policy", directory)


def test_set_get_policy_file(program, directory, keyring):
    """ tests that setting a policy on a directory then getting the policy
    for a file in that directory does not return an error. """
    program("", "set_policy", test_descriptor, directory)
    file = os.path.join(directory, "foo.txt")

    # Should not be able to write file without key present
    with pytest.raises(Exception) as e:
        write_file(file)

    program(test_key, "insert_key", "--ext4")
    write_file(file)

    program("", "get_policy", file)


def test_file_read(program, filesystem, directory, keyring):
    """ tests that we can create an encrypted file, read it and then fail to
    read it if the required key is not present """
    program("", "set_policy", test_descriptor, directory)
    program(test_key, "insert_key", "--ext4")

    file = os.path.join(directory, "bar.txt")
    write_file(file)

    # Should be able to read with key in the keyring (even if we remount)
    assert read_file(file) == test_data
    remount(filesystem)
    assert read_file(file) == test_data

    # After key removed (and cache cleared), filename should not exist.
    keyutils.clear(keyring)
    remount(filesystem)
    assert not os.path.isfile(file)

    # There should be one encrypted file, and it should not be readable
    [encryptedName] = os.listdir(directory)
    encryptedFile = os.path.join(directory, encryptedName)
    assert os.path.isfile(encryptedFile)
    with pytest.raises(Exception) as e:
        read_file(encryptedFile)

    # Putting the key back in should make the file readable again
    program(test_key, "insert_key", "--ext4")
    assert read_file(file) == test_data
