#!/usr/bin/env python3

# Run this script like so:
#
#  ./update-gtranslit.py /path/to/glibc/localedata/locales > gtranslit-data.h

import sys, os

localedir = sys.argv[1]

# returns true if the name looks like a POSIX locale name
def looks_like_locale(name):
    name, _, variant = name.partition('@')

    if '_' not in name:
        return False

    lang, _, land = name.partition('_')

    return len(lang) == 2 or len(lang) == 3 and len(land) == 2

# handles <U1234> style escapes
def unescape(string):
    chunks = []

    n = len(string)
    i = 0

    while i < n:
        start_escape = string.find('<', i)

        if start_escape == -1:
            chunks.append(string[i:])
            break

        assert string[start_escape:start_escape + 2] == '<U'
        start_escape += 2

        end_escape = string.find('>', start_escape)
        assert end_escape != -1

        chunks.append(chr(int(string[start_escape:end_escape], 16)))
        i = end_escape + 1

    return ''.join(chunks)

# Checks if a string is ascii
def is_ascii(string):
    return all(ord(c) < 0x80 for c in string)

# A Mapping is a map from non-ascii strings to ascii strings.
#
# It corresponds to a sequence of one or more mapping lines:
#
#   <U00C4> "<U0041><U0308>";"<U0041><U0045>"
#
# in a file.
class Mapping:
    def __init__(self):
        self.serialised = None
        self.mapping = {}

    # Scans a string like
    #
    #   <U00C4> "<U0041><U0308>";"<U0041><U0045>" % LATIN CAPITAL LETTER A WITH DIAERESIS.
    #
    # and adds the first all-ascii choice (or IGNORE) to the mapping
    # dictionary, with the origin string as the key.  In the case of
    # IGNORE, stores the empty string.
    def consider_mapping_line(self, line):
        key, value, rest = (line + ' % comment').split(maxsplit=2)

        key = unescape(key)

        for alternative in value.split(';'):
            if alternative[0] == '"' and alternative[-1] == '"':
                unescaped = unescape(alternative[1:-1])
                if is_ascii(unescaped):
                    self.mapping[key] = unescaped
                    break

            elif alternative[0] == '<' and alternative[-1] == '>':
                unescaped = unescape(alternative)
                if is_ascii(unescaped):
                    self.mapping[key] = unescaped
                    break

            elif alternative == 'IGNORE':
                self.mapping[key] = ''
                break

    # Performs a normal dictionary merge, but ensures that there are no
    # conflicting entries between the original dictionary and the requested
    # changes
    def merge_mapping(self, changes):
        for key in changes.mapping:
            if key in self.mapping:
                assert self.mapping[key] == changes.mapping[key]

        self.mapping.update(changes.mapping)

    # Can't get much flatter...
    def get_flattened(self):
        return [self]

    def serialise(self, serialiser):
        if self.serialised is None:
            self.serialised = serialiser.add_mapping(self.mapping)

        return self.serialised

# A Chain is a sequence of mappings and chains.
#
# A chain contains another chain whenever "copy" or "include" is
# encountered in a source file.
#
# A chain contains a mapping whenever a sequence of mapping lines:
#
#   <U00C4> "<U0041><U0308>";"<U0041><U0045>"
#
# is encountered in a file.
#
# The order of lookup is reverse: later entries override earlier ones.
class Chain:
    def __init__(self, name):
        self.serialised = None
        self.name = name
        self.chain = []
        self.links = 0

        self.read_from_file(os.path.join(localedir, name))

    def read_from_file(self, filename):
        current_mapping = None
        in_lc_ctype = False
        in_translit = False

        fp = open(filename, encoding='ascii', errors='surrogateescape')

        for line in fp:
            line = line.strip()

            if in_lc_ctype:
                if line == 'END LC_CTYPE':
                    break

                if line.startswith('copy') or line.startswith('include'):
                    if current_mapping:
                        self.chain.append(current_mapping)

                    copyname = unescape(line.split('"', 3)[1])
                    copyfile = get_chain(copyname)
                    self.chain.append(copyfile)
                    copyfile.links += 1

                    current_mapping = None

                elif line == 'translit_start':
                    in_translit = True

                elif line == 'translit_end':
                    in_translit = False

                elif in_translit and line.startswith('<U'):
                    if not current_mapping:
                        current_mapping = Mapping()

                    current_mapping.consider_mapping_line(line)

                elif line == '' or line.startswith('%'):
                    pass

                elif 'default_missing <U003F>':
                    pass

                elif in_translit:
                    print('unknown line:', line)
                    assert False

            elif line == 'LC_CTYPE':
                in_lc_ctype = True

        if current_mapping:
            self.chain.append(current_mapping)

    # If there is only one link to this chain, we may as well just
    # return the contents of the chain so that they can be merged into
    # our sole parent directly.  Otherwise, return ourselves.
    def get_flattened(self):
        if self.links == 1:
            return sum((item.get_flattened() for item in self.chain), [])
        else:
            return [self]

    def serialise(self, serialiser):
        if self.serialised is None:
            # Before we serialise, see if we can optimise a bit
            self.chain = sum((item.get_flattened() for item in self.chain), [])

            i = 0
            while i < len(self.chain) - 1:
                if isinstance(self.chain[i], Mapping) and isinstance(self.chain[i + 1], Mapping):
                    # We have two mappings in a row.  Try to merge them.
                    self.chain[i].merge_mapping(self.chain[i + 1])
                    del self.chain[i + 1]
                else:
                    i += 1

            # If all that is left is one item, just serialise that directly
            if len(self.chain) == 1:
                self.serialised = self.chain[0].serialise(serialiser)
            else:
                ids = [item.serialise(serialiser) for item in self.chain]
                self.serialised = serialiser.add_chain(ids)

        return self.serialised

# Chain cache -- allows sharing of common chains
chains = {}
def get_chain(name):
    if not name in chains:
        chains[name] = Chain(name)

    return chains[name]


# Remove the country name from a locale, preserving variant
# eg: 'sr_RS@latin' -> 'sr@latin'
def remove_country(string):
    base, at, variant = string.partition('@')
    lang, _, land = base.partition('_')
    return lang + at + variant

def encode_range(start, end):
    assert start <= end
    length = end - start

    assert start < 0x1000
    assert length < 0x8

    result = 0x8000 + (length << 12) + start

    assert result < 0x10000

    return result

def c_pair_array(array):
    return '{ ' + ', '.join ('{ %u, %u }' % pair for pair in array) + ' };'

class Serialiser:
    def __init__(self):
        self.mappings = []
        self.chains = []
        self.locales = {}

    def add_mapping(self, mapping):
        if mapping in self.mappings:
            mapping_id = self.mappings.index(mapping)
        else:
            mapping_id = len(self.mappings)
            self.mappings.append(mapping)

        assert mapping_id < 128
        return mapping_id

    def add_chain(self, chain):
        if chain in self.chains:
            chain_id = self.chains.index(chain)
        else:
            chain_id = len(self.chains)
            self.chains.append(chain)

        assert chain_id < 128
        return 128 + chain_id

    def add_locale(self, name, item_id):
        self.locales[name] = item_id

    def add_default(self, item_id):
        self.default = item_id

    def optimise_locales(self):
        # Check if all regions of a language/variant agree
        languages = list(set(remove_country(locale) for locale in self.locales))

        for language in languages:
            locales = [locale for locale in self.locales if remove_country(locale) == language]

            item_id = self.locales[locales[0]]
            if all(self.locales[locale] == item_id for locale in locales):
                self.locales[language] = item_id
                for locale in locales:
                    del self.locales[locale]

        # Check if a variant is the same as the non-variant form
        # eg: 'de@euro' and 'de'
        for variant in list(locale for locale in self.locales if '@' in locale):
            base, _, _ = variant.partition('@')
            if base in self.locales and self.locales[base] == self.locales[variant]:
                del self.locales[variant]

        # Eliminate any entries that are just the same as the C locale
        for locale in list(self.locales):
            if self.locales[locale] == self.default:
                del self.locales[locale]

    def to_c(self):
        src_table = ''
        ascii_table = ''
        mappings_table = []
        mapping_ranges = []
        chains_table = []
        chain_starts = []
        locale_names = ''
        locale_index = []
        max_lookup = 0
        max_localename = 0

        for mapping in self.mappings:
            mapping_ranges.append ((len(mappings_table), len(mapping)))

            for key in sorted(mapping):
                if len(key) == 1 and ord(key[0]) < 0x8000:
                    src_range = ord(key[0])
                else:
                    existing = src_table.find(key)
                    if existing == -1:
                        start = len(src_table)
                        assert all(ord(c) <= 0x10ffff for c in key)
                        src_table += key
                        src_range = encode_range(start, len(src_table))
                        max_lookup = max(max_lookup, len(key))
                    else:
                        src_range = encode_range(existing, existing + len(key))

                value = mapping[key]
                if len(value) == 1 and ord(value[0]) < 0x80:
                    ascii_range = ord(value[0])
                else:
                    existing = ascii_table.find(value)
                    if existing == -1:
                        start = len(ascii_table)
                        assert all(ord(c) < 0x80 for c in value)
                        ascii_table += value
                        ascii_range = encode_range(start, len(ascii_table))
                    else:
                        ascii_range = encode_range(existing, existing + len(value))

                mappings_table.append ((src_range, ascii_range))

            mapping_end = len(mappings_table)

        for chain in self.chains:
            chain_starts.append(len(chains_table))

            for item_id in reversed(chain):
                assert item_id < 0xff
                chains_table.append(item_id)
            chains_table.append(0xff)

        for locale in sorted(self.locales):
            max_localename = max(max_localename, len(locale))
            name_offset = len(locale_names)
            assert all(ord(c) <= 0x7f for c in locale)
            locale_names += (locale + '\0')

            item_id = self.locales[locale]

            assert name_offset < 256
            assert item_id < 256
            locale_index.append((name_offset, item_id))

        print('/* Generated by update-gtranslit.py */')
        print('#define MAX_KEY_SIZE', max_lookup)
        print('#define MAX_LOCALE_NAME', max_localename)
        print('static const gunichar src_table[] = {', ', '.join(str(ord(c)) for c in src_table), '};')
        # cannot do this in plain ascii because of trigraphs... :(
        print('static const gchar ascii_table[] = {', ', '.join(str(ord(c)) for c in ascii_table), '};')
        print('static const struct mapping_entry mappings_table[] =', c_pair_array (mappings_table))
        print('static const struct mapping_range mapping_ranges[] =', c_pair_array (mapping_ranges))
        print('static const guint8 chains_table[] = {', ', '.join(str(i) for i in chains_table), '};')
        print('static const guint8 chain_starts[] = {', ', '.join(str(i) for i in chain_starts), '};')
        print('static const gchar locale_names[] = "' + locale_names.replace('\0', '\\0') + '";')
        print('static const struct locale_entry locale_index[] = ', c_pair_array (locale_index))
        print('static const guint8 default_item_id = %u;' % (self.default,))

    def dump(self):
        print(self.mappings)
        print(self.chains)
        print(self.locales)

locales = []
for name in os.listdir(localedir):
    if looks_like_locale(name):
        chain = get_chain(name)
        locales.append (chain)
        chain.links += 1

serialiser = Serialiser()

for locale in locales:
    serialiser.add_locale(locale.name, locale.serialise(serialiser))

i18n = get_chain('i18n').serialise(serialiser)
combining = get_chain('translit_combining').serialise(serialiser)
serialiser.add_default(serialiser.add_chain([i18n, combining]))

serialiser.optimise_locales()

serialiser.to_c()
