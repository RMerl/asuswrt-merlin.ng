#!/usr/bin/env python
#
# Copyright (C) 2014 Tobias Brunner
# Hochschule fuer Technik Rapperswil
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.

"""
Parses strongswan.conf option descriptions and produces configuration file
and man page snippets.

The format for description files is as follows:

full.option.name [[:]= default]
	Short description intended as comment in config snippet

	Long description for use in the man page, with
	simple formatting: _italic_, **bold**

	Second paragraph of the long description

The descriptions must be indented by tabs or spaces but are both optional.
If only a short description is given it is used for both intended usages.
Line breaks within a paragraph of the long description or the short description
are not preserved.  But multiple paragraphs will be separated in the man page.
Any formatting in the short description is removed when producing config
snippets.

Options for which a value is assigned with := are not commented out in the
produced configuration file snippet.  This allows to override a default value,
that e.g. has to be preserved for legacy reasons, in the generated default
config.

To describe sections the following format can be used:

full.section.name {[#]}
	Short description of this section

	Long description as above

If a # is added between the curly braces the section header will be commented
out in the configuration file snippet, which is useful for example sections.
"""

import sys
import re
from textwrap import TextWrapper
from optparse import OptionParser
from operator import attrgetter

class ConfigOption:
	"""Representing a configuration option or described section in strongswan.conf"""
	def __init__(self, name, default = None, section = False, commented = False):
		self.name = name.split('.')[-1]
		self.fullname = name
		self.default = default
		self.section = section
		self.commented = commented
		self.desc = []
		self.options = []

	def __lt__(self, other):
		return  self.name < other.name

	def add_paragraph(self):
		"""Adds a new paragraph to the description"""
		if len(self.desc) and len(self.desc[-1]):
			self.desc.append("")

	def add(self, line):
		"""Adds a line to the last paragraph"""
		if not len(self.desc):
			self.desc.append(line)
		elif not len(self.desc[-1]):
			self.desc[-1] = line
		else:
			self.desc[-1] += ' ' + line

	def adopt(self, other):
		"""Adopts settings from other, which should be more recently parsed"""
		self.default = other.default
		self.commented = other.commented
		self.desc = other.desc

class Parser:
	"""Parses one or more files of configuration options"""
	def __init__(self, sort = True):
		self.options = []
		self.sort = sort

	def parse(self, file):
		"""Parses the given file and adds all options to the internal store"""
		self.__current = None
		for line in file:
			self.__parse_line(line)
		if self.__current:
			self.__add_option(self.__current)

	def __parse_line(self, line):
		"""Parses a single line"""
		if re.match(r'^\s*#', line):
			return
		# option definition
		m = re.match(r'^(?P<name>\S+)\s*((?P<assign>:)?=\s*(?P<default>.+)?)?\s*$', line)
		if m:
			if self.__current:
				self.__add_option(self.__current)
			self.__current = ConfigOption(m.group('name'), m.group('default'),
										  commented = not m.group('assign'))
			return
		# section definition
		m = re.match(r'^(?P<name>\S+)\s*\{\s*(?P<comment>#)?\s*\}\s*$', line)
		if m:
			if self.__current:
				self.__add_option(self.__current)
			self.__current = ConfigOption(m.group('name'), section = True,
										  commented = m.group('comment'))
			return
		# paragraph separator
		m = re.match(r'^\s*$', line)
		if m and self.__current:
			self.__current.add_paragraph()
		# description line
		m = re.match(r'^\s+(?P<text>.+?)\s*$', line)
		if m and self.__current:
			self.__current.add(m.group('text'))

	def __add_option(self, option):
		"""Adds the given option to the abstract storage"""
		option.desc = [desc for desc in option.desc if len(desc)]
		parts = option.fullname.split('.')
		parent = self.__get_option(parts[:-1], True)
		if not parent:
			parent = self
		found = next((x for x in parent.options if x.name == option.name
										and x.section == option.section), None)
		if found:
			found.adopt(option)
		else:
			parent.options.append(option)
			if self.sort:
				parent.options.sort()

	def __get_option(self, parts, create = False):
		"""Searches/Creates the option (section) based on a list of section names"""
		option = None
		options = self.options
		fullname = ""
		for name in parts:
			fullname += '.' + name if len(fullname) else name
			option = next((x for x in options if x.name == name and x.section), None)
			if not option:
				if not create:
					break
				option = ConfigOption(fullname, section = True)
				options.append(option)
				if self.sort:
					options.sort()
			options = option.options
		return option

	def get_option(self, name):
		"""Retrieves the option with the given name"""
		return self.__get_option(name.split('.'))

class TagReplacer:
	"""Replaces formatting tags in text"""
	def __init__(self):
		self.__matcher_b = self.__create_matcher('**')
		self.__matcher_i = self.__create_matcher('_')
		self.__replacer = None

	def __create_matcher(self, tag):
		tag = re.escape(tag)
		return re.compile(r'''
			(^|\s|(?P<brack>[(\[])) # prefix with optional opening bracket
			(?P<tag>''' + tag + r''') # start tag
			(?P<text>\w|\S.*?\S) # text
			''' + tag + r''' # end tag
			(?P<punct>([.,!:)\]]|\(\d+\))*) # punctuation
			(?=$|\s) # suffix (don't consume it so that subsequent tags can match)
			''', flags = re.DOTALL | re.VERBOSE)

	def _create_replacer(self):
		def replacer(m):
			punct = m.group('punct')
			if not punct:
				punct = ''
			return '{0}{1}{2}'.format(m.group(1), m.group('text'), punct)
		return replacer

	def replace(self, text):
		if not self.__replacer:
			self.__replacer = self._create_replacer()
		text = re.sub(self.__matcher_b, self.__replacer, text)
		return re.sub(self.__matcher_i, self.__replacer, text)

class GroffTagReplacer(TagReplacer):
	def _create_replacer(self):
		def replacer(m):
			nl = '\n' if m.group(1) else ''
			format = 'I' if m.group('tag') == '_' else 'B'
			brack = m.group('brack')
			if not brack:
				brack = ''
			punct = m.group('punct')
			if not punct:
				punct = ''
			text = re.sub(r'[\r\n\t]', ' ', m.group('text'))
			return '{0}.R{1} "{2}" "{3}" "{4}"\n'.format(nl, format, brack, text, punct)
		return replacer

class ConfFormatter:
	"""Formats options to a strongswan.conf snippet"""
	def __init__(self):
		self.__indent = '    '
		self.__wrapper = TextWrapper(width = 80, replace_whitespace = True,
									 break_long_words = False, break_on_hyphens = False)
		self.__tags = TagReplacer()

	def __print_description(self, opt, indent):
		if len(opt.desc):
			self.__wrapper.initial_indent = '{0}# '.format(self.__indent * indent)
			self.__wrapper.subsequent_indent = self.__wrapper.initial_indent
			print(self.__wrapper.fill(self.__tags.replace(opt.desc[0])))

	def __print_option(self, opt, indent, commented):
		"""Print a single option with description and default value"""
		comment = "# " if commented or opt.commented else ""
		self.__print_description(opt, indent)
		if opt.default:
			print('{0}{1}{2} = {3}'.format(self.__indent * indent, comment, opt.name, opt.default))
		else:
			print('{0}{1}{2} ='.format(self.__indent * indent, comment, opt.name))
		print('')

	def __print_section(self, section, indent, commented):
		"""Print a section with all options"""
		commented = commented or section.commented
		comment = "# " if commented else ""
		self.__print_description(section, indent)
		print('{0}{1}{2} {{'.format(self.__indent * indent, comment, section.name))
		print('')
		for o in sorted(section.options, key=attrgetter('section')):
			if o.section:
				self.__print_section(o, indent + 1, commented)
			else:
				self.__print_option(o, indent + 1, commented)
		print('{0}{1}}}'.format(self.__indent * indent, comment))
		print('')

	def format(self, options):
		"""Print a list of options"""
		if not options:
			return
		for option in sorted(options, key=attrgetter('section')):
			if option.section:
				self.__print_section(option, 0, False)
			else:
				self.__print_option(option, 0, False)

class ManFormatter:
	"""Formats a list of options into a groff snippet"""
	def __init__(self):
		self.__wrapper = TextWrapper(width = 80, replace_whitespace = False,
									 break_long_words = False, break_on_hyphens = False)
		self.__tags = GroffTagReplacer()

	def __groffize(self, text):
		"""Encode text as groff text"""
		text = self.__tags.replace(text)
		text = re.sub(r'(?<!\\)-', r'\\-', text)
		# remove any leading whitespace
		return re.sub(r'^\s+', '', text, flags = re.MULTILINE)

	def __format_option(self, option):
		"""Print a single option"""
		if option.section and not len(option.desc):
			return
		if option.section:
			print('.TP\n.B {0}\n.br'.format(option.fullname))
		else:
			print('.TP')
			default = option.default if option.default else ''
			print('.BR {0} " [{1}]"'.format(option.fullname, default))
		for para in option.desc if len(option.desc) < 2 else option.desc[1:]:
			print(self.__groffize(self.__wrapper.fill(para)))
			print('')

	def format(self, options):
		"""Print a list of options"""
		if not options:
			return
		for option in options:
			if option.section:
				self.__format_option(option)
				self.format(option.options)
			else:
				self.__format_option(option)

options = OptionParser(usage = "Usage: %prog [options] file1 file2\n\n"
					   "If no filenames are provided the input is read from stdin.")
options.add_option("-f", "--format", dest="format", type="choice", choices=["conf", "man"],
				   help="output format: conf, man [default: %default]", default="conf")
options.add_option("-r", "--root", dest="root", metavar="NAME",
				   help="root section of which options are printed, "
				   "if not found everything is printed")
options.add_option("-n", "--nosort", action="store_false", dest="sort",
				   default=True, help="do not sort sections alphabetically")

(opts, args) = options.parse_args()

parser = Parser(opts.sort)
if len(args):
	for filename in args:
		try:
			with open(filename, 'r') as file:
				parser.parse(file)
		except IOError as e:
			sys.stderr.write("Unable to open '{0}': {1}\n".format(filename, e.strerror))
else:
	parser.parse(sys.stdin)

options = parser.options
if (opts.root):
	root = parser.get_option(opts.root)
	if root:
		options = root.options

if opts.format == "conf":
	formatter = ConfFormatter()
elif opts.format == "man":
	formatter = ManFormatter()

formatter.format(options)
