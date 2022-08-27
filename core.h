#include <stdint.h>

#define MEMORY_SIZE 0x16384
#define STACK_SIZE 0x1024
#define REGISTERS_SIZE 0x08

#define STATUS_CONTINUE 0
#define STATUS_NEXT_FRAME 1
#define STATUS_HALTED 2
#define STATUS_INVALID_INSTRUCTION 3
#define STATUS_END_OF_ROM 4
#define STATUS_MEMORY_OUT_OF_BOUNDS 5
#define STATUS_STACK_OUT_OF_BOUNDS 6

#define FLAG_HALT 0x01
#define FLAG_IGNORING 0x02

#define OP_NOOP 0x00
#define OP_NEXT_FRAME 0x01
#define OP_COPY_REGISTER_TO_MEMORY 0x02
#define OP_COPY_MEMORY_TO_REGISTER 0x03
#define OP_SET_REGISTER 0x04
#define OP_SET_MEMORY 0x05
#define OP_SET_REGISTER_REGISTER 0x06
#define OP_ADDITION 0x10
#define OP_SUBTRACTION 0x11
#define OP_MULTIPLICATION 0x12
#define OP_DIVISION 0x13
#define OP_MODULO 0x14
#define OP_BIT_XOR 0x20
#define OP_BIT_AND 0x21
#define OP_BIT_OR 0x22
#define OP_BIT_XNOR 0x23
#define OP_BIT_NAND 0x24
#define OP_BIT_NOR 0x25
#define OP_BIT_NOT 0x26
#define OP_BIT_SHIFT_LEFT 0x27
#define OP_BIT_SHIFT_RIGHT 0x29
#define OP_PUSH_STACK 0x9f
#define OP_PUSH_IF_EQUAL 0xa0
#define OP_PUSH_IF_NOT_EQUAL 0xa1
#define OP_PUSH_IF_GREATER 0xa2
#define OP_PUSH_IF_LESS 0xa3
#define OP_PUSH_IF_GREATER_OR_EQUAL 0xa4
#define OP_PUSH_IF_LESS_OR_EQUAL 0xa5
#define OP_POP_STACK 0xaf
#define OP_POP_IF_EQUAL 0xb0
#define OP_POP_IF_NOT_EQUAL 0xb1
#define OP_POP_IF_GREATER 0xb2
#define OP_POP_IF_LESS 0xb3
#define OP_POP_IF_GREATER_OR_EQUAL 0xb4
#define OP_POP_IF_LESS_OR_EQUAL 0xb5
#define OP_JUMP_MIN 0xbf // all jump opcodes must be equal to or greater than this, and push must be less than this
#define OP_JUMP 0xbf
#define OP_JUMP_IF_EQUAL 0xc0
#define OP_JUMP_IF_NOT_EQUAL 0xc1
#define OP_JUMP_IF_GREATER 0xc2
#define OP_JUMP_IF_LESS 0xc3
#define OP_JUMP_IF_GREATER_OR_EQUAL 0xc4
#define OP_JUMP_IF_LESS_OR_EQUAL 0xc5
#define OP_IGNORE 0xfe
#define OP_HALT 0xff
#define OP_UNIGNORE 0x00

#define OPC_NOOP "nop"
#define OPC_NEXT_FRAME "nxt"
#define OPC_COPY_REGISTER_TO_MEMORY "smr"
#define OPC_COPY_MEMORY_TO_REGISTER "srm"
#define OPC_SET_REGISTER "srb"
#define OPC_SET_MEMORY "smb"
#define OPC_SET_REGISTER_REGISTER "srr"
#define OPC_ADDITION "add"
#define OPC_SUBTRACTION "sub"
#define OPC_MULTIPLICATION "mul"
#define OPC_DIVISION "div"
#define OPC_MODULO "mod"
#define OPC_BIT_XOR "xor"
#define OPC_BIT_AND "and"
#define OPC_BIT_OR "orr"
#define OPC_BIT_XNOR "xnr"
#define OPC_BIT_NAND "nan"
#define OPC_BIT_NOR "nor"
#define OPC_BIT_NOT "not"
#define OPC_BIT_SHIFT_LEFT "bsl"
#define OPC_BIT_SHIFT_RIGHT "bsr"
#define OPC_PUSH_STACK "psh"
#define OPC_PUSH_IF_EQUAL "teq"
#define OPC_PUSH_IF_NOT_EQUAL "tnq"
#define OPC_PUSH_IF_GREATER "tgt"
#define OPC_PUSH_IF_LESS "tlt"
#define OPC_PUSH_IF_GREATER_OR_EQUAL "tge"
#define OPC_PUSH_IF_LESS_OR_EQUAL "tle"
#define OPC_POP_STACK "pop"
#define OPC_POP_IF_EQUAL "peq"
#define OPC_POP_IF_NOT_EQUAL "pnq"
#define OPC_POP_IF_GREATER "pgt"
#define OPC_POP_IF_LESS "plt"
#define OPC_POP_IF_GREATER_OR_EQUAL "pge"
#define OPC_POP_IF_LESS_OR_EQUAL "ple"
#define OPC_JUMP "jmp"
#define OPC_JUMP_IF_EQUAL "jeq"
#define OPC_JUMP_IF_NOT_EQUAL "jnq"
#define OPC_JUMP_IF_GREATER "jgt"
#define OPC_JUMP_IF_LESS "jlt"
#define OPC_JUMP_IF_GREATER_OR_EQUAL "jge"
#define OPC_JUMP_IF_LESS_OR_EQUAL "jle"
#define OPC_HALT "hlt"

#define ARG_TYPE_NONE 0
#define ARG_TYPE_REGISTER 1
#define ARG_TYPE_BYTE 2
#define ARG_TYPE_2_BYTE 3
#define ARG_TYPE_LABEL 4

struct game {
	uint8_t registers[REGISTERS_SIZE];
	uint8_t *rom;
	uint32_t rom_length;
	uint32_t rom_index;
	uint8_t memory[MEMORY_SIZE];
	uint16_t stack[STACK_SIZE];
	uint16_t stack_index;
	uint8_t flags;
};

struct game *new_game(uint8_t *rom, uint32_t rom_length);
void dispose_game(struct game *game);

void reset_registers(struct game *game);
uint8_t *get_register(struct game *game, uint8_t reg);

uint8_t rom_jump_without_stack(struct game *game, uint32_t index);
uint8_t rom_jump_push(struct game *game, uint32_t index);
uint8_t rom_jump_pop(struct game *game);
uint8_t rom_next(struct game *game);
uint8_t rom_get(struct game *game);

uint8_t step(struct game *game);
uint8_t frame(struct game *game);

uint8_t compile_to_rom(uint8_t *in, size_t in_len, uint8_t **out, size_t *out_len, char **error, size_t *line);
