#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>

#define ___asm_opcode_identity32(x) ((x) & 0xFFFFFFFF)
#define __opcode_to_mem_arm(x) ___opcode_identity32(x)
#define ___asm_opcode_to_mem_arm(x) ___asm_opcode_identity32(x)

#define ___opcode_identity32(x) ((u32)(x))
#define ___inst_arm(x) .long x
#define __inst_arm(x) ___inst_arm(___asm_opcode_to_mem_arm(x))

#define __inst_arm_thumb32(arm_opcode, thumb_opcode) __inst_arm(arm_opcode)

#define __SMC(imm4) __inst_arm_thumb32(                                 \
        0xE1600070 | (((imm4) & 0xF) << 0),                             \
        0xF7F08000 | (((imm4) & 0xF) << 16)                             \
)

ENTRY(__invoke_nexell_fn_smc)
	__SMC(0)
	bx      lr
ENDPROC(__invoke_nexell_fn_smc)
