import subprocess
import tempfile

import pytest

keytypes = [
	"rsa", "rsa-4096",
	"ed25519",
	"ecdsa", "ecdsa-256", "ecdsa-384", "ecdsa-521",
	]

def parse_keytype(kt):
	if '-' in kt:
		return kt.split('-')
	else:
		return (kt, None)

@pytest.mark.parametrize("keytype", keytypes)
@pytest.mark.parametrize("keyformat", [None, "PEM"])
def test_from_openssh(request, tmp_path, keytype, keyformat):
	"""
	Convert OpenSSH to Dropbear format,
	PEM and OpenSSH internal
	"""
	opt = request.config.option
	kt, keybits = parse_keytype(keytype)

	if kt == 'dss' and keyformat is None:
		pytest.skip("dss doesn't support openssh format")

	os_kt = kt
	if os_kt == 'dss':
		# OpenSSH calls it 'dsa', Dropbear calls it 'dss'
		os_kt = 'dsa'

	os_key = tmp_path / 'oskey1'
	db_key = tmp_path / 'dbkey1'

	# Generate an OpenSSH key
	args = [
		opt.ssh_keygen,
		'-f', os_key,
		'-t', os_kt,
		'-N', '', # no password
	]
	if keybits is not None:
		args += ['-b', keybits]
	if keyformat:
		args += ['-m', keyformat]
	p = subprocess.run(args, check=True)

	# Convert to dropbear format
	args = [
		opt.dropbearconvert,
		'openssh', 'dropbear',
		os_key, db_key,
	]
	p = subprocess.run(args, check=True)

	# Compare pubkeys
	args = [
		opt.dropbearkey,
		'-f', db_key,
		'-y'
	]
	p = subprocess.run(args, check=True, stdout=subprocess.PIPE, text=True)
	db_pubkey = p.stdout.splitlines()[1].strip()
	os_pubkey = os_key.with_suffix('.pub').open().read().strip()
	# we compare the whole key including comment since it currently matches
	assert db_pubkey == os_pubkey

@pytest.mark.parametrize("keytype", keytypes)
def test_roundtrip(request, tmp_path, keytype):
	"""
	Dropbear's private key format is deterministic so
	we can compare round trip conversion. (OpenSSH's
	format has more variable comments and other fields).
	"""
	opt = request.config.option
	kt, keybits = parse_keytype(keytype)

	os_key = tmp_path / 'oskey1'
	db_key1 = tmp_path / 'dbkey1'
	db_key2 = tmp_path / 'dbkey2'

	# generate a key
	args = [
		opt.dropbearkey,
		'-t', kt,
		'-f', db_key1,
	]
	if keybits is not None:
		args += ['-s', keybits]
	p = subprocess.run(args, check=True)

	# convert to openssh
	args = [
		opt.dropbearconvert,
		'dropbear', 'openssh',
		db_key1, os_key,
	]
	p = subprocess.run(args, check=True)

	# Check ssh-keygen can read it
	args = [
		opt.ssh_keygen,
		'-f', os_key,
		'-y',
	]
	p = subprocess.run(args, check=True, text=True, stdout=subprocess.PIPE)
	os_pubkey = p.stdout.strip()

	# Compare public keys
	args = [
		opt.dropbearkey,
		'-f', db_key1,
		'-y',
	]
	p = subprocess.run(args, check=True, text=True, stdout=subprocess.PIPE)
	db_pubkey = p.stdout.splitlines()[1].strip()
	# comment may differ
	db_pubkey = db_pubkey.split(' ')[:2]
	os_pubkey = os_pubkey.split(' ')[:2]
	assert db_pubkey == os_pubkey

	# convert back to dropbear
	args = [
		opt.dropbearconvert,
		'openssh', 'dropbear',
		os_key, db_key2,
	]
	p = subprocess.run(args, check=True)
	# check the round trip is identical
	assert db_key1.open('rb').read() == db_key2.open('rb').read()
