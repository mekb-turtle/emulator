#include <stdio.h>
#include <stdlib.h>
#include "./core.h"
struct game *new_game(uint8_t *rom, uint32_t rom_length) {
	if (rom_length == 0) return NULL;
	if (!rom) return NULL;
	struct game *game;
	game = calloc(1, sizeof(struct game));
	if (!game) return NULL;
	game->rom = rom;
	game->rom_length = rom_length;
	game->rom_index = 0;
	game->stack_index = 0;
	reset_registers(game);
	game->flags = 0;
	return game;
}
void dispose_game(struct game *game) {
	if (!game) return;
	free(game);
}
void reset_registers(struct game *game) {
	for (uint8_t i = 0; i < REGISTERS_SIZE; ++i) game->registers[i] = 0;
}
uint8_t *get_register(struct game *game, uint8_t reg) {
	return &(game->registers[reg & 7]);
}
uint8_t rom_jump_without_stack(struct game *game, uint32_t index) {
	if (index >= game->rom_length) { game->flags |= FLAG_HALT; return 1; }
	game->rom_index = index;
	return 0;
}
uint8_t rom_jump_push(struct game *game, uint32_t index) {
	if (game->stack_index >= STACK_SIZE) { game->flags |= FLAG_HALT; return 1; }
	game->stack[game->stack_index++] = game->rom_index;
	game->rom_index = index;
	return 0;
}
uint8_t rom_jump_pop(struct game *game) {
	if (game->stack_index <= 0) { game->flags |= FLAG_HALT; return 1; }
	game->rom_index = game->stack[--game->stack_index];
	return 0;
}
uint8_t rom_next(struct game *game) {
	if (game->rom_index >= game->rom_length) { game->flags |= FLAG_HALT; return 1; }
	return rom_jump_without_stack(game, game->rom_index + 1);
}
uint8_t rom_get(struct game *game) {
	return game->rom[game->rom_index];
}
uint8_t step(struct game *game) {
	if (game->flags & FLAG_HALT) return STATUS_HALTED;
	uint8_t *reg1 = NULL;
	uint8_t *reg2 = NULL;
	uint8_t *reg3 = NULL;
	uint16_t mem = 0;
	uint8_t byte = 0;
	uint8_t instr = rom_get(game);
	if (game->flags & FLAG_IGNORING) {
		if (instr == OP_UNIGNORE) game->flags &= ~FLAG_IGNORING;
	} else {
		switch (instr) {
			case OP_NOOP: // no-op
				break;
			case OP_NEXT_FRAME: // next frame
				if (rom_next(game)) return STATUS_END_OF_ROM;
				return STATUS_NEXT_FRAME;
			case OP_COPY_REGISTER_TO_MEMORY: // copy byte from register to memory
			case OP_COPY_MEMORY_TO_REGISTER: // copy byte from memory to register
			case OP_SET_REGISTER: // set register to byte
			case OP_SET_MEMORY: // set at memory address to byte
				if (instr == OP_COPY_REGISTER_TO_MEMORY || instr == OP_COPY_MEMORY_TO_REGISTER || instr == OP_SET_REGISTER) {
					if (rom_next(game)) return STATUS_END_OF_ROM;
					reg1 = get_register(game, rom_get(game));
				}
				if (instr == OP_COPY_REGISTER_TO_MEMORY || instr == OP_COPY_MEMORY_TO_REGISTER || instr == OP_SET_MEMORY) {
					if (rom_next(game)) return STATUS_END_OF_ROM;
					mem = ((uint16_t)rom_get(game)) << 8;
					if (rom_next(game)) return STATUS_END_OF_ROM;
					mem |= rom_get(game);
				}
				if (instr == OP_SET_REGISTER || instr == OP_SET_MEMORY) {
					if (rom_next(game)) return STATUS_END_OF_ROM;
					byte = rom_get(game);
				}
				if (mem >= MEMORY_SIZE)
					return STATUS_MEMORY_OUT_OF_BOUNDS;
				if (instr == OP_COPY_REGISTER_TO_MEMORY) game->memory[mem] = *reg1;
				if (instr == OP_COPY_MEMORY_TO_REGISTER) *reg1 = game->memory[mem];
				if (instr == OP_SET_REGISTER) *reg1 = byte;
				if (instr == OP_SET_MEMORY) game->memory[mem] = byte;
				break;
			case OP_ADDITION:
			case OP_SUBTRACTION:
			case OP_MULTIPLICATION:
			case OP_DIVISION:
			case OP_MODULO:
			case OP_BIT_XOR:
			case OP_BIT_AND:
			case OP_BIT_OR:
			case OP_BIT_XNOR:
			case OP_BIT_NAND:
			case OP_BIT_NOR:
			case OP_BIT_SHIFT_LEFT:
			case OP_BIT_SHIFT_RIGHT:
				if (rom_next(game)) return STATUS_END_OF_ROM;
				reg1 = get_register(game, rom_get(game));
				if (rom_next(game)) return STATUS_END_OF_ROM;
				reg2 = get_register(game, rom_get(game));
				if (rom_next(game)) return STATUS_END_OF_ROM;
				reg3 = get_register(game, rom_get(game));
				if (instr == OP_ADDITION)        *reg3 =   *reg1 +  *reg2;
				if (instr == OP_SUBTRACTION)     *reg3 =   *reg1 -  *reg2;
				if (instr == OP_MULTIPLICATION)  *reg3 =   *reg1 *  *reg2;
				if (instr == OP_DIVISION)        *reg3 =   *reg1 /  *reg2;
				if (instr == OP_MODULO)          *reg3 =   *reg1 %  *reg2;
				if (instr == OP_BIT_XOR)         *reg3 =   *reg1 ^  *reg2;
				if (instr == OP_BIT_AND)         *reg3 =   *reg1 &  *reg2;
				if (instr == OP_BIT_OR)          *reg3 =   *reg1 |  *reg2;
				if (instr == OP_BIT_XNOR)        *reg3 = ~(*reg1 ^  *reg2);
				if (instr == OP_BIT_NAND)        *reg3 = ~(*reg1 |  *reg2);
				if (instr == OP_BIT_NOR)         *reg3 = ~(*reg1 &  *reg2);
				if (instr == OP_BIT_SHIFT_LEFT)  *reg3 =   *reg1 << *reg2;
				if (instr == OP_BIT_SHIFT_RIGHT) *reg3 =   *reg1 >> *reg2;
				break;
			case OP_BIT_NOT: // bit NOT
				if (rom_next(game)) return STATUS_END_OF_ROM;
				reg1 = get_register(game, rom_get(game));
				if (rom_next(game)) return STATUS_END_OF_ROM;
				reg2 = get_register(game, rom_get(game));
				*reg2 = ~(*reg1);
				break;
			case OP_PUSH_STACK: // push stack
				if (rom_next(game)) return STATUS_END_OF_ROM;
				mem = ((uint16_t)rom_get(game)) << 8;
				if (rom_next(game)) return STATUS_END_OF_ROM;
				mem |= rom_get(game);
				return STATUS_STACK_OUT_OF_BOUNDS;
				break;
			case OP_POP_STACK: // pop stack
				return STATUS_STACK_OUT_OF_BOUNDS;
				break;
			case OP_IGNORE: // ignore instructions until next OP_UNIGNORE
				break;
			case OP_HALT: // halt
				game->flags |= FLAG_HALT;
				return STATUS_HALTED;
			default:
				game->flags |= FLAG_HALT;
				return STATUS_INVALID_INSTRUCTION;
		}
	}
	if (rom_next(game)) return STATUS_END_OF_ROM;
	return STATUS_CONTINUE;
}
uint8_t frame(struct game *game) {
	uint8_t r;
	while ((r = step(game)) == STATUS_CONTINUE);
	return r;
}
