# SPDX-License-Identifier:  GPL-2.0+
# Copyright (c) 2020
# Author: Sam Protsenko <joe.skb7@gmail.com>

# Test U-Boot's "abootimg" commands.

import os
import pytest
import u_boot_utils

"""
These tests rely on disk image (boot.img), which is automatically created by
the test from the stored hex dump. This is done to avoid the dependency on the
most recent mkbootimg tool from AOSP/master. Here is the list of commands which
was used to generate the boot.img and obtain compressed hex dump from it:

    $ echo '/dts-v1/; / { model = "x1"; compatible = "y1,z1"; };' > test1.dts
    $ echo '/dts-v1/; / { model = "x2"; compatible = "y2,z2"; };' > test2.dts
    $ dtc test1.dts > dt1.dtb
    $ dtc test2.dts > dt2.dtb
    $ cat dt1.dtb dt2.dtb > dtb.img
    $ echo 'kernel payload' > kernel
    $ echo 'ramdisk payload' > ramdisk.img
    $ mkbootimg --kernel ./kernel --ramdisk ./ramdisk.img  \
                --cmdline "cmdline test" --dtb ./dtb.img   \
                --os_version R --os_patch_level 2019-06-05 \
                --header_version 2 --output boot.img
    $ gzip -9 boot.img
    $ xxd -p boot.img.gz > boot.img.gz.hex

Now one can obtain original boot.img from this hex dump like this:

    $ xxd -r -p boot.img.gz.hex boot.img.gz
    $ gunzip -9 boot.img.gz

For boot image header version 4, these tests rely on two images that are generated
using the same steps above :

1- boot.img :
    $ mkbootimg --kernel ./kernel --ramdisk ./ramdisk.img  \
                --cmdline "cmdline test" --dtb ./dtb.img   \
                --os_version R --os_patch_level 2019-06-05 \
                --header_version 4 --output ./boot.img

2- vendor_boot.img
    $ mkbootimg --kernel ./kernel --ramdisk ./ramdisk.img  \
                --cmdline "cmdline test" --dtb ./dtb.img   \
                --os_version R --os_patch_level 2019-06-05 \
                --pagesize 4096  --vendor_ramdisk ./ramdisk.img \
                --header_version 4 --vendor_boot ./vboot.img \

"""

# boot.img.gz hex dump
img_hex = """1f8b08084844af5d0203626f6f742e696d670073f47309f2f77451e46700
820606010106301084501f04181819041838181898803c3346060c909c9b
92939997aa50925a5cc2300a461c3078b2e1793c4b876fd92db97939fb6c
b7762ffff07d345446c1281805e8a0868d81e117a45e111c0d8dc101b253
8bf25273140a122b73f21353b8460364148c8251300a46c1281801a02831
3725b3387bb401300a46c1281805a360148c207081f7df5b20550bc41640
9c03c41a0c90f17fe85400986d82452b6c3680198a192a0ce17c3610ae34
d4a9820881a70f3873f35352731892f3730b124b32937252a96bb9119ae5
463a5546f82c1f05a360148c8251300a462e000085bf67f200200000"""

# boot img v4 hex dump
boot_img_hex = """1f8b080827b0cd630203626f6f742e696d6700edd8bd0d82601885d1d7c4
58d8c808b88195bd098d8d246e40e42b083f1aa0717be99d003d277916b8
e5bddc8a7b792d8e8788c896ce9b88d32ebe6c971e7ddd3543cae734cd01
c0ffc84c0000b0766d1a87d4e5afeadd3dab7a6f10000000f84163d5d7cd
d43a000000000000000060c53e7544995700400000"""

# vendor boot image v4 hex dump
vboot_img_hex = """1f8b0808baaecd63020376626f6f742e696d6700edd8310b824018c6f1b3
222a08f41b3436b4280dcdd19c11d16ee9109d18d59042d047ec8b04cd0d
d19d5a4345534bf6ffc173ef29272f38e93b1d0ec67dd79d548462aa1cd2
d5d20b0000f8438678f90c18d584b8a4bbb3a557991ecb2a0000f80d6b2f
f4179b656be5c532f2fc066f040000000080e23936af2755f62a3d918df1
db2a7ab67f9ffdeb7df7cda3465ecb79c4ce7e5c577562bb9364b74449a5
1e467e20c53c0a57de763193c1779b3b4fcd9d4ee27c6a0e00000000c0ff
309ffea7010000000040f1dc004129855400400000"""

# Expected response for "abootimg dtb_dump" command
dtb_dump_resp="""## DTB area contents (concat format):
 - DTB #0:
           (DTB)size = 125
          (DTB)model = x1
     (DTB)compatible = y1,z1
 - DTB #1:
           (DTB)size = 125
          (DTB)model = x2
     (DTB)compatible = y2,z2"""
# Address in RAM where to load the boot image ('abootimg' looks in $loadaddr)
loadaddr = 0x1000
# Address in RAM where to load the vendor boot image ('abootimg' looks in $vloadaddr)
vloadaddr= 0x10000
# Expected DTB #1 offset from the boot image start address
dtb1_offset = 0x187d
# Expected DTB offset from the vendor boot image start address
dtb2_offset = 0x207d
# DTB #1 start address in RAM
dtb1_addr = loadaddr + dtb1_offset
# DTB #2 start address in RAM
dtb2_addr = vloadaddr + dtb2_offset

class AbootimgTestDiskImage(object):
    """Disk image used by abootimg tests."""

    def __init__(self, u_boot_console, image_name, hex_img):
        """Initialize a new AbootimgDiskImage object.

        Args:
            u_boot_console: A U-Boot console.

        Returns:
            Nothing.
        """

        gz_hex = u_boot_console.config.persistent_data_dir + '/' + image_name  + '.gz.hex'
        gz = u_boot_console.config.persistent_data_dir + '/' + image_name + '.gz'

        filename = image_name
        persistent = u_boot_console.config.persistent_data_dir + '/' + filename
        self.path = u_boot_console.config.result_dir  + '/' + filename
        u_boot_console.log.action('persistent is ' + persistent)
        with u_boot_utils.persistent_file_helper(u_boot_console.log, persistent):
            if os.path.exists(persistent):
                u_boot_console.log.action('Disk image file ' + persistent +
                    ' already exists')
            else:
                u_boot_console.log.action('Generating ' + persistent)

                f = open(gz_hex, "w")
                f.write(hex_img)
                f.close()
                cmd = ('xxd', '-r', '-p', gz_hex, gz)
                u_boot_utils.run_and_log(u_boot_console, cmd)
                cmd = ('gunzip', '-9', gz)
                u_boot_utils.run_and_log(u_boot_console, cmd)

        cmd = ('cp', persistent, self.path)
        u_boot_utils.run_and_log(u_boot_console, cmd)

gtdi1 = None
@pytest.fixture(scope='function')
def abootimg_disk_image(u_boot_console):
    """pytest fixture to provide a AbootimgTestDiskImage object to tests.
    This is function-scoped because it uses u_boot_console, which is also
    function-scoped. However, we don't need to actually do any function-scope
    work, so this simply returns the same object over and over each time."""

    global gtdi1
    if not gtdi1:
        gtdi1 = AbootimgTestDiskImage(u_boot_console, 'boot.img', img_hex)
    return gtdi1

gtdi2 = None
@pytest.fixture(scope='function')
def abootimgv4_disk_image_vboot(u_boot_console):
    """pytest fixture to provide a AbootimgTestDiskImage object to tests.
    This is function-scoped because it uses u_boot_console, which is also
    function-scoped. However, we don't need to actually do any function-scope
    work, so this simply returns the same object over and over each time."""

    global gtdi2
    if not gtdi2:
        gtdi2 = AbootimgTestDiskImage(u_boot_console, 'vendor_boot.img', vboot_img_hex)
    return gtdi2

gtdi3 = None
@pytest.fixture(scope='function')
def abootimgv4_disk_image_boot(u_boot_console):
    """pytest fixture to provide a AbootimgTestDiskImage object to tests.
    This is function-scoped because it uses u_boot_console, which is also
    function-scoped. However, we don't need to actually do any function-scope
    work, so this simply returns the same object over and over each time."""

    global gtdi3
    if not gtdi3:
        gtdi3 = AbootimgTestDiskImage(u_boot_console, 'bootv4.img', boot_img_hex)
    return gtdi3

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('android_boot_image')
@pytest.mark.buildconfigspec('cmd_abootimg')
@pytest.mark.buildconfigspec('cmd_fdt')
@pytest.mark.requiredtool('xxd')
@pytest.mark.requiredtool('gunzip')
def test_abootimg(abootimg_disk_image, u_boot_console):
    """Test the 'abootimg' command."""

    u_boot_console.log.action('Loading disk image to RAM...')
    u_boot_console.run_command('setenv loadaddr 0x%x' % (loadaddr))
    u_boot_console.run_command('host load hostfs - 0x%x %s' % (loadaddr,
        abootimg_disk_image.path))

    u_boot_console.log.action('Testing \'abootimg get ver\'...')
    response = u_boot_console.run_command('abootimg get ver')
    assert response == "2"
    u_boot_console.run_command('abootimg get ver v')
    response = u_boot_console.run_command('env print v')
    assert response == 'v=2'

    u_boot_console.log.action('Testing \'abootimg get recovery_dtbo\'...')
    response = u_boot_console.run_command('abootimg get recovery_dtbo a')
    assert response == 'Error: recovery_dtbo_size is 0'

    u_boot_console.log.action('Testing \'abootimg dump dtb\'...')
    response = u_boot_console.run_command('abootimg dump dtb').replace('\r', '')
    assert response == dtb_dump_resp

    u_boot_console.log.action('Testing \'abootimg get dtb_load_addr\'...')
    u_boot_console.run_command('abootimg get dtb_load_addr a')
    response = u_boot_console.run_command('env print a')
    assert response == 'a=11f00000'

    u_boot_console.log.action('Testing \'abootimg get dtb --index\'...')
    u_boot_console.run_command('abootimg get dtb --index=1 dtb1_start')
    response = u_boot_console.run_command('env print dtb1_start')
    correct_str = "dtb1_start=%x" % (dtb1_addr)
    assert response == correct_str
    u_boot_console.run_command('fdt addr $dtb1_start')
    u_boot_console.run_command('fdt get value v / model')
    response = u_boot_console.run_command('env print v')
    assert response == 'v=x2'

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('android_boot_image')
@pytest.mark.buildconfigspec('cmd_abootimg')
@pytest.mark.buildconfigspec('cmd_fdt')
@pytest.mark.requiredtool('xxd')
@pytest.mark.requiredtool('gunzip')
def test_abootimgv4(abootimgv4_disk_image_vboot, abootimgv4_disk_image_boot, u_boot_console):
    """Test the 'abootimg' command with boot image header v4."""

    cons = u_boot_console
    cons.log.action('Loading disk image to RAM...')
    cons.run_command('setenv loadaddr 0x%x' % (loadaddr))
    cons.run_command('setenv vloadaddr 0x%x' % (vloadaddr))
    cons.run_command('host load hostfs - 0x%x %s' % (vloadaddr,
	abootimgv4_disk_image_vboot.path))
    cons.run_command('host load hostfs - 0x%x %s' % (loadaddr,
        abootimgv4_disk_image_boot.path))
    cons.run_command('abootimg addr 0x%x 0x%x' % (loadaddr, vloadaddr))
    cons.log.action('Testing \'abootimg get ver\'...')
    response = cons.run_command('abootimg get ver')
    assert response == "4"
    cons.run_command('abootimg get ver v')
    response = cons.run_command('env print v')
    assert response == 'v=4'

    cons.log.action('Testing \'abootimg get recovery_dtbo\'...')
    response = cons.run_command('abootimg get recovery_dtbo a')
    assert response == 'Error: header version must be >= 1 and <= 2 to get dtbo'

    cons.log.action('Testing \'abootimg get dtb_load_addr\'...')
    cons.run_command('abootimg get dtb_load_addr a')
    response = cons.run_command('env print a')
    assert response == 'a=11f00000'

    cons.log.action('Testing \'abootimg get dtb --index\'...')
    cons.run_command('abootimg get dtb --index=1 dtb2_start')
    response = cons.run_command('env print dtb2_start')
    correct_str = "dtb2_start=%x" % (dtb2_addr)
    assert response == correct_str

    cons.run_command('fdt addr $dtb2_start')
    cons.run_command('fdt get value v / model')
    response = cons.run_command('env print v')
    assert response == 'v=x2'
