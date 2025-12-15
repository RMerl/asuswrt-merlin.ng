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

C Increase index of general-purpose register by specific value
C INC_GPR(GPR, INC)
define(`INC_GPR',`ifelse(substr($1,0,1),`r',
``r'eval($2+substr($1,1,len($1)))',
`eval($2+$1)')')

C Increase index of vector register by specific value
C INC_VR(VR, INC)
define(`INC_VR',`ifelse(substr($1,0,1),`v',
``v'eval($2+substr($1,1,len($1)))',
`eval($2+$1)')')

C Apply op x, x, y, for each x.
C OPN_XXY(OP, Y, X1, X2, ...)
define(`OPN_XXY',
`$1 $3, $3, $2
ifelse(eval($# > 3), 1,
`OPN_XXY($1, $2, shift(shift(shift($@))))dnl
')')

C Apply op x, x, x, y, for each x.
C OPN_XXXY(OP, Y, X1, X2, ...)
define(`OPN_XXXY',
`$1 $3, $3, $3, $2
ifelse(eval($# > 3), 1,
`OPN_XXXY($1, $2, shift(shift(shift($@))))dnl
')')

C Polynomial reduction R += x^{-64} F mod P
C where x^{-64} = x^{64} + P1 (mod P)
C GHASH_REDUCE(R, F, P1, T1, T2)
define(`GHASH_REDUCE', `
    vpmsumd        $4, $2, $3
    xxswapd        VSR($5),VSR($2)
    vxor           $1, $1, $5
    vxor           $1, $1, $4
')

C GF multification of L/M and data
C GF_MUL(
C GF_MUL(F, R, HL, HM, S)
define(`GF_MUL',
  `vpmsumd $1,$3,$5
   vpmsumd $2,$4,$5')
