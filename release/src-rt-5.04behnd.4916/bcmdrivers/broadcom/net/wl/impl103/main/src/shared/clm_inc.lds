OUTPUT_ARCH(arm)
ENTRY(startup)
SECTIONS {
	. = 0;
	startup = .;
	.clmdata : {
		*(.*.clm_inc_header)
		*(.*.clm_inc_data)
		*(.clmdataini2.*)
	}
}
