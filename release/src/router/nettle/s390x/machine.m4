C Register usage:
define(`RA', `%r14') C Return address
define(`SP', `%r15') C Stack pointer

define(`STANDARD_STACK_FRAME',`160')

C Dynamic stack space allocation
C AP is a general register to which the allocated space is assigned
C SPACE_LEN is the length of space, must be a multiple of 8
C FREE_STACK macro can be used to free the allocated space
C ALLOC_STACK(AP, SPACE_LEN)
define(`ALLOC_STACK',
`lgr            $1,SP
    aghi           SP,-(STANDARD_STACK_FRAME+$2)
    stg            $1,0(SP)
    la             $1,STANDARD_STACK_FRAME (SP)')

C Free allocated stack space
C FREE_STACK(SPACE_LEN)
define(`FREE_STACK',
`aghi           SP,STANDARD_STACK_FRAME+$1')

C XOR contents of two areas in storage with specific length
C DST: The destination area of storage
C SRC: The source area of storage
C LEN: The length of storage area. LEN <= 256
C TMP_ADDR: A temporary register that handles the XOR instrcution address
C All parameters are general registers and cannot be assigned to
C general register 0
C XOR_LEN(DST, SRC, LEN, TMP_ADDR)
define(`XOR_LEN',
`larl           $4,18f
    aghi           $3,-1
    jm             19f
    ex             $3,0($4)
    j              19f
18: xc             0(1,$1),0($2)
19:')
