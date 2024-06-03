import sys
import bbs
import clr
import wpf
import time
from os.path import exists
from os.path import dirname
from os.path import splitext
from System import *
clr.AddReference("System.Windows.Forms")
from System.Windows.Forms import *

def op_status():
    while(not Flash.IsOperationCompleted):
        time.sleep(0.5)

ferase = False
skip_smc = False
skip = False
path = '.'

files =   ["smc_bootl-68880B0.prodkey.hsm_signed.flash", "smc_os-68880B0.bin.prodkey.hsm_signed.flash", "bcm968880GO_ubootenv_lun.bin", "bcm968880GO_meminit_lun.bin", "bcm968880GO_armbl_lun.bin", "bcm968880GO_bstrap.pkgtb"]
emmc_offsets = [0x00000000,                                   0x00080000,                                   0x00000000,                     0x00040000,                    0x00140000,                  0x00440000] 
nand128_offs = [0x00000000,                                   0x000C0000,                                   0x004C0000,                     0x00540000,                    0x00660000,                  0x009C0000] 
nand256_offs = [0x00000000,                                   0x00100000,                                   0x00600000,                     0x00700000,                    0x00840000,                  0x00C00000] 

try:
	Flash = Bcm968880.Flash[0]
except:
	Flash = Bcm968880c.Flash[0]
Flash.Initialize(False)

type = format(Flash.DetectedFlashType)
if type == 'eMMC' :
    emmc = True
    offsets = emmc_offsets
    ferase = False
else :
    emmc = False
    selection = MessageBox.Show("Is block size 128KB?", "Settings", MessageBoxButtons.YesNo, MessageBoxIcon.Question)
    if selection == DialogResult.Yes :
    	offsets = nand128_offs
    	bsize = 0x20000
    else:
     	offsets = nand256_offs
     	bsize = 0x40000
    selection = MessageBox.Show("Erase entire flash?", "Settings", MessageBoxButtons.YesNo, MessageBoxIcon.Question)
    if selection == DialogResult.Yes :
        ferase = True
    else:
        ferase = False

selection = MessageBox.Show("Burn SMC images?", "Settings", MessageBoxButtons.YesNo, MessageBoxIcon.Question)
if selection == DialogResult.Yes :
    skip_smc = False
    inc = '(including smc)'
else:
    skip_smc = True
    inc = '(without smc)'
    
print('Programming Bcm968880 board with {}'.format(Flash.DetectedFlashType) +  ' flash {}'.format(inc))
if ferase == True :
    print('Erasing whole flash...')
    Flash.EraseAsync(0, Flash.DetectedFlashSize)
    op_status()
        
for (image, offset) in zip(files, offsets):
    fpath = path + '\\' + image
    soffset = offset
    if image.find('smc') != -1 :
        if skip_smc == True :
            continue
        if emmc == True :
            Flash.eMMCPartitionAccess = 1 # boot partition 1
    else:
        if emmc == True :
            Flash.eMMCPartitionAccess = 0 # user data partition
            
    if exists(fpath) == False :
        FileDialog = OpenFileDialog()
        FileDialog.ValidateNames = True
        FileDialog.CheckFileExists = True
        FileDialog.CheckPathExists = True
        FileDialog.Title = 'Choose ' + image + ' image file...'
        FileDialog.FileName = image
        name, extension = splitext(image)
        FileDialog.Filter = '*' + extension + '|*' + extension
        selection = FileDialog.ShowDialog()
        if selection == DialogResult.OK :
            fpath = FileDialog.FileName
        else :
            break

    if emmc == False :
    	while offset < Flash.DetectedFlashSize and Flash.IsBadBlock(offset) == True :
    	    offset += bsize
    	if offset >= Flash.DetectedFlashSize :
    		print('\tFailed to burn' + fpath + ' at 0x{:08X}'.format(soffset))
    		continue
    		
    if soffset == offset :	    
    	print('\t' + fpath + ' at 0x{:08X}...'.format(offset))
    else :
    	print('\t' + fpath + ' at !0x{:08X}...'.format(offset))
    Flash.DownloadAsync(fpath, offset, False)
    op_status()
    path = dirname(fpath)
    
print('done')
