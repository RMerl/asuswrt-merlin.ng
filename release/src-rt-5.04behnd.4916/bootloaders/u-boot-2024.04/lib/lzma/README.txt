The lib_lzma functionality was written by Igor Pavlov.
The original source cames from the LZMA SDK web page:

URL: 		http://www.7-zip.org/sdk.html
Author:         Igor Pavlov

The import is made using the import_lzmasdk.sh script that:

* untars the lzmaXYY.tar.bz2 file (from the download web page)
* copies the files LzmaDec.h, Types.h, LzmaDec.c, history.txt,
  and lzma.txt from source archive into the lib_lzma directory (pwd).

Example:

 . import_lzmasdk.sh ~/lzma465.tar.bz2

Notice: The files from lzma sdk are _not modified_ by this script!

The files LzmaTools.{c,h} are provided to export the lzmaBuffToBuffDecompress()
function that wraps the complex LzmaDecode() function from the LZMA SDK. The
do_bootm() function uses the lzmaBuffToBuffDecopress() function to expand the
compressed image.

The directory U-BOOT/include/lzma contains stubs files that permit to use the
library directly from U-BOOT code without touching the original LZMA SDK's
files.

Luigi 'Comio' Mantellini <luigi.mantellini@idf-hit.com>
