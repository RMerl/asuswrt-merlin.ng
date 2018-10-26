

TOOLS_BTRM_BIN_DIR = ../bin

all: mkbindir bootromcrc_4_arm bootromcrc_4_mips gen_crc crc2bin dec2bin mod2bin component2bin SRECParser

bootromcrc_4_arm bootromcrc_4_mips gen_crc crc2bin dec2bin mod2bin component2bin SRECParser : mkbindir

.PHONY: mkbindir

mkbindir:
	mkdir -p $(TOOLS_BTRM_BIN_DIR)
bootromcrc_4_arm: bootromcrc_4_arm.c
	gcc -m32 -o $(TOOLS_BTRM_BIN_DIR)/bootromcrc_4_arm bootromcrc_4_arm.c
bootromcrc_4_mips: bootromcrc_4_mips.c
	gcc -m32 -o $(TOOLS_BTRM_BIN_DIR)/bootromcrc_4_mips bootromcrc_4_mips.c
crc2bin: crc2bin.c
	gcc -m32 -o $(TOOLS_BTRM_BIN_DIR)/crc2bin crc2bin.c
gen_crc: gen_crc.c
	gcc -o $(TOOLS_BTRM_BIN_DIR)/gen_crc gen_crc.c
dec2bin: dec2bin.c
	gcc -m32 -o $(TOOLS_BTRM_BIN_DIR)/dec2bin dec2bin.c 
mod2bin: mod2bin.c
	gcc -m32 -o $(TOOLS_BTRM_BIN_DIR)/mod2bin mod2bin.c 
component2bin: component2bin.c
	gcc -m32 -o $(TOOLS_BTRM_BIN_DIR)/component2bin component2bin.c 
SRECParser: SRECParser.cpp
	g++ -o $(TOOLS_BTRM_BIN_DIR)/SRECParser SRECParser.cpp
clean:
	rm -rf $(TOOLS_BTRM_BIN_DIR)
distclean:clean
