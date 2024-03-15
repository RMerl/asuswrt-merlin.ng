/* Copyright (c) 2020 tevador <tevador@gmail.com> */
/* See LICENSE for licensing information */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "program.h"
#include "unreachable.h"
#include "siphash_rng.h"

/* instructions are generated until this CPU cycle */
#define TARGET_CYCLE 192

/* requirements for the program to be acceptable */
#define REQUIREMENT_SIZE 512
#define REQUIREMENT_MUL_COUNT 192
#define REQUIREMENT_LATENCY 195

/* R5 (x86 = r13) register cannot be used as the destination of INSTR_ADD_RS */
#define REGISTER_NEEDS_DISPLACEMENT 5

#define PORT_MAP_SIZE (TARGET_CYCLE + 4)
#define NUM_PORTS 3
#define MAX_RETRIES 1
#define LOG2_BRANCH_PROB 4
#define BRANCH_MASK 0x80000000

#define TRACE false
#define TRACE_PRINT(...) do { if (TRACE) printf(__VA_ARGS__); } while (false)

#define MAX(a,b) ((a) > (b) ? (a) : (b))

/* If the instruction is a multiplication.  */
static inline bool is_mul(instr_type type) {
	return type <= INSTR_MUL_R;
}

#ifdef HASHX_PROGRAM_STATS
/* If the instruction is a 64x64->128 bit multiplication.  */
static inline bool is_wide_mul(instr_type type) {
	return type < INSTR_MUL_R;
}
#endif

/* Ivy Bridge integer execution ports: P0, P1, P5 */
typedef enum execution_port {
	PORT_NONE = 0,
	PORT_P0 = 1,
	PORT_P1 = 2,
	PORT_P5 = 4,
	PORT_P01 = PORT_P0 | PORT_P1,
	PORT_P05 = PORT_P0 | PORT_P5,
	PORT_P015 = PORT_P0 | PORT_P1 | PORT_P5
} execution_port;

static const char* execution_port_names[] = {
	"PORT_NONE", "PORT_P0", "PORT_P1", "PORT_P01", "PORT_P5", "PORT_P05", "PORT_P15", "PORT_P015"
};

typedef struct instr_template {
	instr_type type;          /* instruction type */
	const char* x86_asm;      /* x86 assembly */
	int x86_size;             /* x86 code size */
	int latency;              /* latency in cycles */
	execution_port uop1;      /* ex. ports for the 1st uop */
	execution_port uop2;      /* ex. ports for the 2nd uop */
	uint32_t immediate_mask;  /* mask for imm32 */
	instr_type group;         /* instruction group */
	bool imm_can_be_0;        /* if imm32 can be zero */
	bool distinct_dst;        /* if dst and src must be distinct */
	bool op_par_src;          /* operation parameter is equal to src */
	bool has_src;             /* if the instruction has a source operand */
	bool has_dst;             /* if the instr. has a destination operand */
} instr_template;

typedef struct register_info {
	int latency;              /* cycle when the register value will be ready */
	instr_type last_op;       /* last op applied to the register */
	uint32_t last_op_par;     /* parameter of the last op (~0 = constant) */
} register_info;

typedef struct program_item {
	const instr_template** templates;
	uint32_t mask0;
	uint32_t mask1;
	bool duplicates;
} program_item;

typedef struct generator_ctx {
	int cycle;
	int sub_cycle;
	int mul_count;
	bool chain_mul;
	int latency;
	siphash_rng gen;
	register_info registers[8];
	execution_port ports[PORT_MAP_SIZE][NUM_PORTS];
} generator_ctx;

static const instr_template tpl_umulh_r = {
	.type = INSTR_UMULH_R,
	.x86_asm = "mul r",
	.x86_size = 9, /* mov, mul, mov */
	.latency = 4,
	.uop1 = PORT_P1,
	.uop2 = PORT_P5,
	.immediate_mask = 0,
	.group = INSTR_UMULH_R,
	.imm_can_be_0 = false,
	.distinct_dst = false,
	.op_par_src = false,
	.has_src = true,
	.has_dst = true,
};

static const instr_template tpl_smulh_r = {
	.type = INSTR_SMULH_R,
	.x86_asm = "imul r",
	.x86_size = 9, /* mov, mul, mov */
	.latency = 4,
	.uop1 = PORT_P1,
	.uop2 = PORT_P5,
	.immediate_mask = 0,
	.group = INSTR_SMULH_R,
	.imm_can_be_0 = false,
	.distinct_dst = false,
	.op_par_src = false,
	.has_src = true,
	.has_dst = true,
};

static const instr_template tpl_mul_r = {
	.type = INSTR_MUL_R,
	.x86_asm = "imul r,r",
	.x86_size = 4,
	.latency = 3,
	.uop1 = PORT_P1,
	.uop2 = PORT_NONE,
	.immediate_mask = 0,
	.group = INSTR_MUL_R,
	.imm_can_be_0 = false,
	.distinct_dst = true,
	.op_par_src = true,
	.has_src = true,
	.has_dst = true,
};

static const instr_template tpl_sub_r = {
	.type = INSTR_SUB_R,
	.x86_asm = "sub r,r",
	.x86_size = 3,
	.latency = 1,
	.uop1 = PORT_P015,
	.uop2 = PORT_NONE,
	.immediate_mask = 0,
	.group = INSTR_ADD_RS,
	.imm_can_be_0 = false,
	.distinct_dst = true,
	.op_par_src = true,
	.has_src = true,
	.has_dst = true,
};

static const instr_template tpl_xor_r = {
	.type = INSTR_XOR_R,
	.x86_asm = "xor r,r",
	.x86_size = 3,
	.latency = 1,
	.uop1 = PORT_P015,
	.uop2 = PORT_NONE,
	.immediate_mask = 0,
	.group = INSTR_XOR_R,
	.imm_can_be_0 = false,
	.distinct_dst = true,
	.op_par_src = true,
	.has_src = true,
	.has_dst = true,
};

static const instr_template tpl_add_rs = {
	.type = INSTR_ADD_RS,
	.x86_asm = "lea r,r+r*s",
	.x86_size = 4,
	.latency = 1,
	.uop1 = PORT_P01,
	.uop2 = PORT_NONE,
	.immediate_mask = 3,
	.group = INSTR_ADD_RS,
	.imm_can_be_0 = true,
	.distinct_dst = true,
	.op_par_src = true,
	.has_src = true,
	.has_dst = true,
};

static const instr_template tpl_ror_c = {
	.type = INSTR_ROR_C,
	.x86_asm = "ror r,i",
	.x86_size = 4,
	.latency = 1,
	.uop1 = PORT_P05,
	.uop2 = PORT_NONE,
	.immediate_mask = 63,
	.group = INSTR_ROR_C,
	.imm_can_be_0 = false,
	.distinct_dst = true,
	.op_par_src = false,
	.has_src = false,
	.has_dst = true,
};

static const instr_template tpl_add_c = {
	.type = INSTR_ADD_C,
	.x86_asm = "add r,i",
	.x86_size = 7,
	.latency = 1,
	.uop1 = PORT_P015,
	.uop2 = PORT_NONE,
	.immediate_mask = UINT32_MAX,
	.group = INSTR_ADD_C,
	.imm_can_be_0 = false,
	.distinct_dst = true,
	.op_par_src = false,
	.has_src = false,
	.has_dst = true,
};

static const instr_template tpl_xor_c = {
	.type = INSTR_XOR_C,
	.x86_asm = "xor r,i",
	.x86_size = 7,
	.latency = 1,
	.uop1 = PORT_P015,
	.uop2 = PORT_NONE,
	.immediate_mask = UINT32_MAX,
	.group = INSTR_XOR_C,
	.imm_can_be_0 = false,
	.distinct_dst = true,
	.op_par_src = false,
	.has_src = false,
	.has_dst = true,
};


static const instr_template tpl_target = {
	.type = INSTR_TARGET,
	.x86_asm = "cmovz esi, edi",
	.x86_size = 5, /* test, cmovz */
	.latency = 1,
	.uop1 = PORT_P015,
	.uop2 = PORT_P015,
	.immediate_mask = 0,
	.group = INSTR_TARGET,
	.imm_can_be_0 = false,
	.distinct_dst = true,
	.op_par_src = false,
	.has_src = false,
	.has_dst = false,
};

static const instr_template tpl_branch = {
	.type = INSTR_BRANCH,
	.x86_asm = "jz target",
	.x86_size = 10, /* or, test, jz */
	.latency = 1,
	.uop1 = PORT_P015,
	.uop2 = PORT_P015,
	.immediate_mask = BRANCH_MASK,
	.group = INSTR_BRANCH,
	.imm_can_be_0 = false,
	.distinct_dst = true,
	.op_par_src = false,
	.has_src = false,
	.has_dst = false,
};

static const instr_template* instr_lookup[] = {
	&tpl_ror_c,
	&tpl_xor_c,
	&tpl_add_c,
	&tpl_add_c,
	&tpl_sub_r,
	&tpl_xor_r,
	&tpl_xor_c,
	&tpl_add_rs,
};

static const instr_template* wide_mul_lookup[] = {
	&tpl_smulh_r,
	&tpl_umulh_r
};

static const instr_template* mul_lookup = &tpl_mul_r;
static const instr_template* target_lookup = &tpl_target;
static const instr_template* branch_lookup = &tpl_branch;

static const program_item item_mul = {
	.templates = &mul_lookup,
	.mask0 = 0,
	.mask1 = 0,
	.duplicates = true
};

static const program_item item_target = {
	.templates = &target_lookup,
	.mask0 = 0,
	.mask1 = 0,
	.duplicates = true
};

static const program_item item_branch = {
	.templates = &branch_lookup,
	.mask0 = 0,
	.mask1 = 0,
	.duplicates = true
};

static const program_item item_wide_mul = {
	.templates = wide_mul_lookup,
	.mask0 = 1,
	.mask1 = 1,
	.duplicates = true
};

static const program_item item_any = {
	.templates = instr_lookup,
	.mask0 = 7,
	.mask1 = 3, /* instructions that don't need a src register */
	.duplicates = false
};

static const program_item* program_layout[] = {
	&item_mul,
	&item_target,
	&item_any,
	&item_mul,
	&item_any,
	&item_any,
	&item_mul,
	&item_any,
	&item_any,
	&item_mul,
	&item_any,
	&item_any,
	&item_wide_mul,
	&item_any,
	&item_any,
	&item_mul,
	&item_any,
	&item_any,
	&item_mul,
	&item_branch,
	&item_any,
	&item_mul,
	&item_any,
	&item_any,
	&item_wide_mul,
	&item_any,
	&item_any,
	&item_mul,
	&item_any,
	&item_any,
	&item_mul,
	&item_any,
	&item_any,
	&item_mul,
	&item_any,
	&item_any,
};

static const instr_template* select_template(generator_ctx* ctx, instr_type last_instr, int attempt) {
	const program_item* item = program_layout[ctx->sub_cycle % 36];
	const instr_template* tpl;
	do {
		int index = item->mask0 ? hashx_siphash_rng_u8(&ctx->gen) & (attempt > 0 ? item->mask1 : item->mask0) : 0;
		tpl = item->templates[index];
	} while (!item->duplicates && tpl->group == last_instr);
	return tpl;
}

static uint32_t branch_mask(siphash_rng* gen) {
	uint32_t mask = 0;
	int popcnt = 0;
	while (popcnt < LOG2_BRANCH_PROB) {
		int bit = hashx_siphash_rng_u8(gen) % 32;
		uint32_t bitmask = 1U << bit;
		if (!(mask & bitmask)) {
			mask |= bitmask;
			popcnt++;
		}
	}
	return mask;
}

static void instr_from_template(const instr_template* tpl, siphash_rng* gen, instruction* instr) {
	instr->opcode = tpl->type;
	if (tpl->immediate_mask) {
		if (tpl->immediate_mask == BRANCH_MASK) {
			instr->imm32 = branch_mask(gen);
		}
		else do {
			instr->imm32 = hashx_siphash_rng_u32(gen) & tpl->immediate_mask;
		} while (instr->imm32 == 0 && !tpl->imm_can_be_0);
	}
	if (!tpl->op_par_src) {
		if (tpl->distinct_dst) {
			instr->op_par = UINT32_MAX;
		}
		else {
			instr->op_par = hashx_siphash_rng_u32(gen);
		}
	}
	if (!tpl->has_src) {
		instr->src = -1;
	}
	if (!tpl->has_dst) {
		instr->dst = -1;
	}
}

static bool select_register(int available_regs[8], int regs_count, siphash_rng* gen, int* reg_out) {
	if (regs_count == 0)
		return false;

	int index;

	if (regs_count > 1) {
		index = hashx_siphash_rng_u32(gen) % regs_count;
	}
	else {
		index = 0;
	}
	*reg_out = available_regs[index];
	return true;
}

static bool select_destination(const instr_template* tpl, instruction* instr, generator_ctx* ctx, int cycle) {
	int available_regs[8];
	int regs_count = 0;
	/* Conditions for the destination register:
	// * value must be ready at the required cycle
	// * cannot be the same as the source register unless the instruction allows it
	//   - this avoids optimizable instructions such as "xor r, r" or "sub r, r"
	// * register cannot be multiplied twice in a row unless chain_mul is true
	//   - this avoids accumulation of trailing zeroes in registers due to excessive multiplication
	//   - allowChainedMul is set to true if an attempt to find source/destination registers failed (this is quite rare, but prevents a catastrophic failure of the generator)
	// * either the last instruction applied to the register or its source must be different than this instruction
	//   - this avoids optimizable instruction sequences such as "xor r1, r2; xor r1, r2" or "ror r, C1; ror r, C2" or "add r, C1; add r, C2"
	// * register r5 cannot be the destination of the IADD_RS instruction (limitation of the x86 lea instruction) */
	for (int i = 0; i < 8; ++i) {
		bool available = ctx->registers[i].latency <= cycle;
		available &= ((!tpl->distinct_dst) | (i != instr->src));
		available &= (ctx->chain_mul | (tpl->group != INSTR_MUL_R) | (ctx->registers[i].last_op != INSTR_MUL_R));
		available &= ((ctx->registers[i].last_op != tpl->group) | (ctx->registers[i].last_op_par != instr->op_par));
		available &= ((instr->opcode != INSTR_ADD_RS) | (i != REGISTER_NEEDS_DISPLACEMENT));
		available_regs[regs_count] = available ? i : 0;
		regs_count += available;
	}
	return select_register(available_regs, regs_count, &ctx->gen, &instr->dst);
}

static bool select_source(const instr_template* tpl, instruction* instr, generator_ctx* ctx, int cycle) {
	int available_regs[8];
	int regs_count = 0;
	/* all registers that are ready at the cycle */
	for (int i = 0; i < 8; ++i) {
		if (ctx->registers[i].latency <= cycle)
			available_regs[regs_count++] = i;
	}
	/* if there are only 2 available registers for ADD_RS and one of them is r5, select it as the source because it cannot be the destination */
	if (regs_count == 2 && instr->opcode == INSTR_ADD_RS) {
		if (available_regs[0] == REGISTER_NEEDS_DISPLACEMENT || available_regs[1] == REGISTER_NEEDS_DISPLACEMENT) {
			instr->op_par = instr->src = REGISTER_NEEDS_DISPLACEMENT;
			return true;
		}
	}
	if (select_register(available_regs, regs_count, &ctx->gen, &instr->src)) {
		if (tpl->op_par_src)
			instr->op_par = instr->src;
		return true;
	}
	return false;
}

static int schedule_uop(execution_port uop, generator_ctx* ctx, int cycle, bool commit) {
	/* The scheduling here is done optimistically by checking port availability in order P5 -> P0 -> P1 to not overload
	   port P1 (multiplication) by instructions that can go to any port. */
	for (; cycle < PORT_MAP_SIZE; ++cycle) {
		if ((uop & PORT_P5) && !ctx->ports[cycle][2]) {
			if (commit) {
				ctx->ports[cycle][2] = uop;
			}
			TRACE_PRINT("%s scheduled to port P5 at cycle %i (commit = %i)\n", execution_port_names[uop], cycle, commit);
			return cycle;
		}
		if ((uop & PORT_P0) && !ctx->ports[cycle][0]) {
			if (commit) {
				ctx->ports[cycle][0] = uop;
			}
			TRACE_PRINT("%s scheduled to port P0 at cycle %i (commit = %i)\n", execution_port_names[uop], cycle, commit);
			return cycle;
		}
		if ((uop & PORT_P1) != 0 && !ctx->ports[cycle][1]) {
			if (commit) {
				ctx->ports[cycle][1] = uop;
			}
			TRACE_PRINT("%s scheduled to port P1 at cycle %i (commit = %i)\n", execution_port_names[uop], cycle, commit);
			return cycle;
		}
	}
	return -1;
}

static int schedule_instr(const instr_template* tpl, generator_ctx* ctx, bool commit) {
	if (tpl->uop2 == PORT_NONE) {
		/* this instruction has only one uOP */
		return schedule_uop(tpl->uop1, ctx, ctx->cycle, commit);
	}
	else {
		/* instructions with 2 uOPs are scheduled conservatively by requiring both uOPs to execute in the same cycle */
		for (int cycle = ctx->cycle; cycle < PORT_MAP_SIZE; ++cycle) {

			int cycle1 = schedule_uop(tpl->uop1, ctx, cycle, false);
			int cycle2 = schedule_uop(tpl->uop2, ctx, cycle, false);

			if (cycle1 >= 0 && cycle1 == cycle2) {
				if (commit) {
					schedule_uop(tpl->uop1, ctx, cycle, true);
					schedule_uop(tpl->uop2, ctx, cycle, true);
				}
				return cycle1;
			}
		}
	}

	return -1;
}

static void print_registers(const generator_ctx* ctx) {
	for (int i = 0; i < 8; ++i) {
		printf("   R%i = %i\n", i, ctx->registers[i].latency);
	}
}

bool hashx_program_generate(const siphash_state* key, hashx_program* program) {
	generator_ctx ctx = {
		.cycle = 0,
		.sub_cycle = 0, /* 3 sub-cycles = 1 cycle */
		.mul_count = 0,
		.chain_mul = false,
		.latency = 0,
		.ports = {{ 0 }}
	};
	hashx_siphash_rng_init(&ctx.gen, key);
#ifdef HASHX_RNG_CALLBACK
	ctx.gen.callback = program->rng_callback;
	ctx.gen.callback_user_data = program->rng_callback_user_data;
#endif
	for (int i = 0; i < 8; ++i) {
		ctx.registers[i].last_op = -1;
		ctx.registers[i].latency = 0;
		ctx.registers[i].last_op_par = (uint32_t)-1;
	}
	program->code_size = 0;

	int attempt = 0;
	instr_type last_instr = -1;
#ifdef HASHX_PROGRAM_STATS
	program->x86_size = 0;
#endif

	while (program->code_size < HASHX_PROGRAM_MAX_SIZE) {
		instruction* instr = &program->code[program->code_size];
		TRACE_PRINT("CYCLE: %i/%i\n", ctx.sub_cycle, ctx.cycle);

		/* select an instruction template */
		const instr_template* tpl = select_template(&ctx, last_instr, attempt);
		last_instr = tpl->group;

		TRACE_PRINT("Template: %s\n", tpl->x86_asm);

		instr_from_template(tpl, &ctx.gen, instr);

		/* calculate the earliest cycle when this instruction (all of its uOPs) can be scheduled for execution */
		int scheduleCycle = schedule_instr(tpl, &ctx, false);
		if (scheduleCycle < 0) {
			TRACE_PRINT("Unable to map operation '%s' to execution port (cycle %i)\n", tpl->x86_asm, ctx.cycle);
			/* __debugbreak(); */
			break;
		}

		ctx.chain_mul = attempt > 0;

		/* find a source register (if applicable) that will be ready when this instruction executes */
		if (tpl->has_src) {
			if (!select_source(tpl, instr, &ctx, scheduleCycle)) {
				TRACE_PRINT("; src STALL (attempt %i)\n", attempt);
				if (attempt++ < MAX_RETRIES) {
					continue;
				}
				if (TRACE) {
					printf("; select_source FAILED at cycle %i\n", ctx.cycle);
					print_registers(&ctx);
					/* __debugbreak(); */
				}
				ctx.sub_cycle += 3;
				ctx.cycle = ctx.sub_cycle / 3;
				attempt = 0;
				continue;
			}
			TRACE_PRINT("; src = r%i\n", instr->src);
		}

		/* find a destination register that will be ready when this instruction executes */
		if (tpl->has_dst) {
			if (!select_destination(tpl, instr, &ctx, scheduleCycle)) {
				TRACE_PRINT("; dst STALL (attempt %i)\n", attempt);
				if (attempt++ < MAX_RETRIES) {
					continue;
				}
				if (TRACE) {
					printf("; select_destination FAILED at cycle %i\n", ctx.cycle);
					print_registers(&ctx);
					/* __debugbreak(); */
				}
				ctx.sub_cycle += 3;
				ctx.cycle = ctx.sub_cycle / 3;
				attempt = 0;
				continue;
			}
			TRACE_PRINT("; dst = r%i\n", instr->dst);
		}
		attempt = 0;

		/* recalculate when the instruction can be scheduled for execution based on operand availability */
		scheduleCycle = schedule_instr(tpl, &ctx, true);

		if (scheduleCycle < 0) {
			TRACE_PRINT("Unable to map operation '%s' to execution port (cycle %i)\n", tpl->x86_asm, ctx.cycle);
			break;
		}

		/* terminating condition */
		if (scheduleCycle >= TARGET_CYCLE) {
			break;
		}

		if (tpl->has_dst) {
			register_info* ri = &ctx.registers[instr->dst];
			int retireCycle = scheduleCycle + tpl->latency;
			ri->latency = retireCycle;
			ri->last_op = tpl->group;
			ri->last_op_par = instr->op_par;
			ctx.latency = MAX(retireCycle, ctx.latency);
			TRACE_PRINT("; RETIRED at cycle %i\n", retireCycle);
		}

		program->code_size++;
#ifdef HASHX_PROGRAM_STATS
		program->x86_size += tpl->x86_size;
#endif

		ctx.mul_count += is_mul(instr->opcode);

		++ctx.sub_cycle;
		ctx.sub_cycle += (tpl->uop2 != PORT_NONE);
		ctx.cycle = ctx.sub_cycle / 3;
	}

#ifdef HASHX_PROGRAM_STATS
	memset(program->asic_latencies, 0, sizeof(program->asic_latencies));

	program->counter = ctx.gen.counter;
	program->wide_mul_count = 0;
	program->mul_count = ctx.mul_count;

	/* Calculate ASIC latency:
	   Assumes 1 cycle latency for all operations and unlimited parallelization. */
	for (size_t i = 0; i < program->code_size; ++i) {
		instruction* instr = &program->code[i];
		if (instr->dst < 0)
			continue;
		int last_dst = program->asic_latencies[instr->dst] + 1;
		int lat_src = instr->dst != instr->src ? program->asic_latencies[instr->src] + 1 : 0;
		program->asic_latencies[instr->dst] = MAX(last_dst, lat_src);
		program->wide_mul_count += is_wide_mul(instr->opcode);
	}

	program->asic_latency = 0;
	program->cpu_latency = 0;
	for (int i = 0; i < 8; ++i) {
		program->asic_latency = MAX(program->asic_latency, program->asic_latencies[i]);
		program->cpu_latencies[i] = ctx.registers[i].latency;
		program->cpu_latency = MAX(program->cpu_latency, program->cpu_latencies[i]);
	}

	program->ipc = program->code_size / (double)program->cpu_latency;
	program->branch_count = 0;
	memset(program->branches, 0, sizeof(program->branches));

	if (TRACE) {
		printf("; ALU port utilization:\n");
		printf("; (* = in use, _ = idle)\n");
		for (int i = 0; i < PORT_MAP_SIZE; ++i) {
			printf("; %3i ", i);
			for (int j = 0; j < NUM_PORTS; ++j) {
				printf("%c", (ctx.ports[i][j] ? '*' : '_'));
			}
			printf("\n");
		}
	}
#endif

	/* reject programs that don't meet the uniform complexity requirements */
	/* this happens in less than 1 seed out of 10000 */
	return
		(program->code_size == REQUIREMENT_SIZE) &&
		(ctx.mul_count == REQUIREMENT_MUL_COUNT) &&
		(ctx.latency == REQUIREMENT_LATENCY - 1); /* cycles are numbered from 0 */
}

static const char* x86_reg_map[] = { "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15" };

void hashx_program_asm_x86(const hashx_program* program) {
	size_t target = 0;
	for (size_t i = 0; i < program->code_size; ++i) {
		const instruction* instr = &program->code[i];
		switch (instr->opcode)
		{
		case INSTR_SUB_R:
			printf("sub %s, %s\n", x86_reg_map[instr->dst], x86_reg_map[instr->src]);
			break;
		case INSTR_XOR_R:
			printf("xor %s, %s\n", x86_reg_map[instr->dst], x86_reg_map[instr->src]);
			break;
		case INSTR_ADD_RS:
			printf("lea %s, [%s+%s*%u]\n", x86_reg_map[instr->dst], x86_reg_map[instr->dst], x86_reg_map[instr->src], 1 << instr->imm32);
			break;
		case INSTR_MUL_R:
			printf("imul %s, %s\n", x86_reg_map[instr->dst], x86_reg_map[instr->src]);
			break;
		case INSTR_ROR_C:
			printf("ror %s, %u\n", x86_reg_map[instr->dst], instr->imm32);
			break;
		case INSTR_ADD_C:
			printf("add %s, %i\n", x86_reg_map[instr->dst], instr->imm32);
			break;
		case INSTR_XOR_C:
			printf("xor %s, %i\n", x86_reg_map[instr->dst], instr->imm32);
			break;
		case INSTR_UMULH_R:
			printf("mov rax, %s\n", x86_reg_map[instr->dst]);
			printf("mul %s\n", x86_reg_map[instr->src]);
			printf("mov %s, rdx\n", x86_reg_map[instr->dst]);
			break;
		case INSTR_SMULH_R:
			printf("mov rax, %s\n", x86_reg_map[instr->dst]);
			printf("imul %s\n", x86_reg_map[instr->src]);
			printf("mov %s, rdx\n", x86_reg_map[instr->dst]);
			break;
		case INSTR_TARGET:
			printf("test edi, edi\n");
			printf("target_%i: cmovz esi, edi\n", (int)i);
			target = i;
			break;
		case INSTR_BRANCH:
			printf("or edx, esi\n");
			printf("test edx, %i\n", instr->imm32);
			printf("jz target_%i\n", (int)target);
			break;
		default:
			UNREACHABLE;
		}
	}
}
