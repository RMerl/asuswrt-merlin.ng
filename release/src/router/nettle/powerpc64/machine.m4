define(`PROLOGUE',
`.globl C_NAME($1)
DECLARE_FUNC(C_NAME($1))
ifelse(ELFV2_ABI,yes,
`ifdef(`FUNC_ALIGN',`.align FUNC_ALIGN')
C_NAME($1):
addis 2,12,(.TOC.-C_NAME($1))@ha
addi 2,2,(.TOC.-C_NAME($1))@l
.localentry C_NAME($1), .-C_NAME($1)',
`.section ".opd","aw"
.align 3
C_NAME($1):
.quad .C_NAME($1),.TOC.@tocbase,0
.previous
ifdef(`FUNC_ALIGN',`.align FUNC_ALIGN')
.C_NAME($1):')
undefine(`FUNC_ALIGN')')

define(`EPILOGUE',
`ifelse(ELFV2_ABI,yes,
`.size C_NAME($1), . - C_NAME($1)',
`.size .C_NAME($1), . - .C_NAME($1)
.size C_NAME($1), . - .C_NAME($1)')')

C Get vector-scalar register from vector register
C VSR(VR)
define(`VSR',`ifelse(substr($1,0,1),`v',
``vs'eval(32+substr($1,1,len($1)))',
`eval(32+$1)')')

C Load the quadword in DATA_SRC storage into
C VEC_DST. GPR is general-purpose register
C used to obtain the effective address of
C DATA_SRC storage.
C DATA_LOAD_VEC(VEC_DST, DATA_SRC, GPR)
define(`DATA_LOAD_VEC',
`addis $3,r2,$2@got@ha
ld $3,$2@got@l($3)
lvx $1,0,$3')

dnl  Usage: r0 ... r31, cr0 ... cr7
dnl
dnl  Registers names, either left as "r0" etc or mapped to plain 0 etc,
dnl  according to the result of the ASM_PPC_WANT_R_REGISTERS configure
dnl  test.

ifelse(ASM_PPC_WANT_R_REGISTERS,no,`
forloop(i,0,31,`deflit(`r'i,i)')
forloop(i,0,31,`deflit(`v'i,i)')
forloop(i,0,63,`deflit(`vs'i,i)')
forloop(i,0,31,`deflit(`f'i,i)')
forloop(i,0,7, `deflit(`cr'i,i)')
')
