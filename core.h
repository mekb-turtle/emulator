#include <stdint.h>
#include <stdbool.h>

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
#define OP_PUSH_STACK 0x30
#define OP_POP_STACK 0x31
#define OP_IGNORE 0xfe
#define OP_HALT 0xff
#define OP_UNIGNORE 0x00

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
