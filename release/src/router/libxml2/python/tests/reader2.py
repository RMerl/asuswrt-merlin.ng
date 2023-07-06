#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# this tests the DTD validation with the XmlTextReader interface
#
import sys
import glob
import os
import string
import libxml2
try:
    import StringIO
    str_io = StringIO.StringIO
except:
    import io
    str_io = io.StringIO

# Memory debug specific
libxml2.debugMemory(1)

err = ""
basedir = os.path.dirname(os.path.realpath(__file__))
dir_prefix = os.path.join(basedir, "../../test/valid/")
# This dictionary reflects the contents of the files
# ../../test/valid/*.xml.err that are not empty, except that
# the file paths in the messages start with ../../test/

expect = {
    '766956':
"""../../test/valid/dtds/766956.dtd:2: parser error : PEReference: expecting ';'
%ä%ent;
   ^
../../test/valid/dtds/766956.dtd:2: parser error : Content error in the external subset
%ä%ent;
        ^
Entity: line 1: 
value
^
""",
    '781333':
"""../../test/valid/781333.xml:4: element a: validity error : Element a content does not follow the DTD, expecting ( ..., got 
<a/>
    ^
../../test/valid/781333.xml:5: element a: validity error : Element a content does not follow the DTD, Expecting more child

^
""",
    'cond_sect2':
"""../../test/valid/dtds/cond_sect2.dtd:15: parser error : All markup of the conditional section is not in the same entity
    %ent;
         ^
Entity: line 1: 
]]>
^
../../test/valid/dtds/cond_sect2.dtd:17: parser error : Content error in the external subset

^
""",
    'rss':
"""../../test/valid/rss.xml:177: element rss: validity error : Element rss does not carry attribute version
</rss>
      ^
""",
    't8':
"""../../test/valid/t8.xml:6: parser error : internal error: xmlParseInternalSubset: error detected in Markup declaration

%defroot; %defmiddle; %deftest;
         ^
Entity: line 1: 
&lt;!ELEMENT root (middle) >
^
../../test/valid/t8.xml:6: parser error : internal error: xmlParseInternalSubset: error detected in Markup declaration

%defroot; %defmiddle; %deftest;
                     ^
Entity: line 1: 
&lt;!ELEMENT middle (test) >
^
../../test/valid/t8.xml:6: parser error : internal error: xmlParseInternalSubset: error detected in Markup declaration

%defroot; %defmiddle; %deftest;
                               ^
Entity: line 1: 
&lt;!ELEMENT test (#PCDATA) >
^
""",
    't8a':
"""../../test/valid/t8a.xml:6: parser error : internal error: xmlParseInternalSubset: error detected in Markup declaration

%defroot;%defmiddle;%deftest;
         ^
Entity: line 1: 
&lt;!ELEMENT root (middle) >
^
../../test/valid/t8a.xml:6: parser error : internal error: xmlParseInternalSubset: error detected in Markup declaration

%defroot;%defmiddle;%deftest;
                    ^
Entity: line 1: 
&lt;!ELEMENT middle (test) >
^
../../test/valid/t8a.xml:6: parser error : internal error: xmlParseInternalSubset: error detected in Markup declaration

%defroot;%defmiddle;%deftest;
                             ^
Entity: line 1: 
&lt;!ELEMENT test (#PCDATA) >
^
""",
    'xlink':
"""../../test/valid/xlink.xml:450: element termdef: validity error : ID dt-arc already defined
	<p><termdef id="dt-arc" term="Arc">An <ter
	                                  ^
validity error : attribute def line 199 references an unknown ID "dt-xlg"
""",
}

# Add prefix_dir and extension to the keys
expect = {"{}{}.xml".format(dir_prefix, key): val for key, val in expect.items()}

def callback(ctx, str):
    global err
    err = err + "%s" % (str)
libxml2.registerErrorHandler(callback, "")

parsing_error_files = ["766956", "cond_sect2", "t8", "t8a"]
expect_parsing_error = ["{}{}.xml".format(dir_prefix, f) for f in parsing_error_files]

valid_files = glob.glob(dir_prefix + "*.x*")
assert valid_files, "found no valid files in '{}'".format(dir_prefix)
valid_files.sort()
for file in valid_files:
    err = ""
    reader = libxml2.newTextReaderFilename(file)
    #print "%s:" % (file)
    reader.SetParserProp(libxml2.PARSER_VALIDATE, 1)
    ret = reader.Read()
    while ret == 1:
        ret = reader.Read()
    if ret != 0 and file not in expect_parsing_error:
        print("Error parsing and validating %s" % (file))
        #sys.exit(1)
    if (err):
        if not(file in expect and err == expect[file]):
            print("Error: ", err)
            if file in expect:
                print("Expected: ", expect[file])
#
# another separate test based on Stephane Bidoul one
#
s = """
<!DOCTYPE test [
<!ELEMENT test (x,b)>
<!ELEMENT x (c)>
<!ELEMENT b (#PCDATA)>
<!ELEMENT c (#PCDATA)>
<!ENTITY x "<x><c>xxx</c></x>">
]>
<test>
    &x;
    <b>bbb</b>
</test>
"""
expect="""10,test
1,test
14,#text
1,x
1,c
3,#text
15,c
15,x
14,#text
1,b
3,#text
15,b
14,#text
15,test
"""
res=""
err=""

input = libxml2.inputBuffer(str_io(s))
reader = input.newTextReader("test2")
reader.SetParserProp(libxml2.PARSER_LOADDTD,1)
reader.SetParserProp(libxml2.PARSER_DEFAULTATTRS,1)
reader.SetParserProp(libxml2.PARSER_SUBST_ENTITIES,1)
reader.SetParserProp(libxml2.PARSER_VALIDATE,1)
while reader.Read() == 1:
    res = res + "%s,%s\n" % (reader.NodeType(),reader.Name())

if res != expect:
    print("test2 failed: unexpected output")
    print(res)
    sys.exit(1)
if err != "":
    print("test2 failed: validation error found")
    print(err)
    sys.exit(1)

#
# Another test for external entity parsing and validation
#

s = """<!DOCTYPE test [
<!ELEMENT test (x)>
<!ELEMENT x (#PCDATA)>
<!ENTITY e SYSTEM "tst.ent">
]>
<test>
  &e;
</test>
"""
tst_ent = """<x>hello</x>"""
expect="""10 test
1 test
14 #text
1 x
3 #text
15 x
14 #text
15 test
"""
res=""

def myResolver(URL, ID, ctxt):
    if URL == "tst.ent":
        return(str_io(tst_ent))
    return None

libxml2.setEntityLoader(myResolver)

input = libxml2.inputBuffer(str_io(s))
reader = input.newTextReader("test3")
reader.SetParserProp(libxml2.PARSER_LOADDTD,1)
reader.SetParserProp(libxml2.PARSER_DEFAULTATTRS,1)
reader.SetParserProp(libxml2.PARSER_SUBST_ENTITIES,1)
reader.SetParserProp(libxml2.PARSER_VALIDATE,1)
while reader.Read() == 1:
    res = res + "%s %s\n" % (reader.NodeType(),reader.Name())

if res != expect:
    print("test3 failed: unexpected output")
    print(res)
    sys.exit(1)
if err != "":
    print("test3 failed: validation error found")
    print(err)
    sys.exit(1)

#
# Another test for recursive entity parsing, validation, and replacement of
# entities, making sure the entity ref node doesn't show up in that case
#

s = """<!DOCTYPE test [
<!ELEMENT test (x, x)>
<!ELEMENT x (y)>
<!ELEMENT y (#PCDATA)>
<!ENTITY x "<x>&y;</x>">
<!ENTITY y "<y>yyy</y>">
]>
<test>
  &x;
  &x;
</test>"""
expect="""10 test 0
1 test 0
14 #text 1
1 x 1
1 y 2
3 #text 3
15 y 2
15 x 1
14 #text 1
1 x 1
1 y 2
3 #text 3
15 y 2
15 x 1
14 #text 1
15 test 0
"""
res=""
err=""

input = libxml2.inputBuffer(str_io(s))
reader = input.newTextReader("test4")
reader.SetParserProp(libxml2.PARSER_LOADDTD,1)
reader.SetParserProp(libxml2.PARSER_DEFAULTATTRS,1)
reader.SetParserProp(libxml2.PARSER_SUBST_ENTITIES,1)
reader.SetParserProp(libxml2.PARSER_VALIDATE,1)
while reader.Read() == 1:
    res = res + "%s %s %d\n" % (reader.NodeType(),reader.Name(),reader.Depth())

if res != expect:
    print("test4 failed: unexpected output")
    print(res)
    sys.exit(1)
if err != "":
    print("test4 failed: validation error found")
    print(err)
    sys.exit(1)

#
# The same test but without entity substitution this time
#

s = """<!DOCTYPE test [
<!ELEMENT test (x, x)>
<!ELEMENT x (y)>
<!ELEMENT y (#PCDATA)>
<!ENTITY x "<x>&y;</x>">
<!ENTITY y "<y>yyy</y>">
]>
<test>
  &x;
  &x;
</test>"""
expect="""10 test 0
1 test 0
14 #text 1
5 x 1
14 #text 1
5 x 1
14 #text 1
15 test 0
"""
res=""
err=""

input = libxml2.inputBuffer(str_io(s))
reader = input.newTextReader("test5")
reader.SetParserProp(libxml2.PARSER_VALIDATE,1)
while reader.Read() == 1:
    res = res + "%s %s %d\n" % (reader.NodeType(),reader.Name(),reader.Depth())

if res != expect:
    print("test5 failed: unexpected output")
    print(res)
if err != "":
    print("test5 failed: validation error found")
    print(err)

#
# cleanup
#
del input
del reader

# Memory debug specific
libxml2.cleanupParser()
if libxml2.debugMemory(1) == 0:
    print("OK")
else:
    print("Memory leak %d bytes" % (libxml2.debugMemory(1)))
    libxml2.dumpMemory()
