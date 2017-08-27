#
# DUMM for Ruby
#

require 'mkmf'

$defs << " -DHAVE_CONFIG_H"
$CFLAGS << " -Wno-format -include \"../../../config.h\""

dir_config('dumm', '../../../src/dumm', '../.libs')
dir_config('strongswan', '../../../src/libstrongswan', '../../libstrongswan/.libs')

unless find_library('dumm', 'dumm_create')
  puts "... failed: 'libdumm' not found!"
  exit
end

create_makefile('dumm', '../../../src/dumm/ext')

