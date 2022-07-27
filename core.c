#include <stdio.h>
#include <stdlib.h>
#include "./core.h"
void reset_registers(struct registers *reg) {
	reg->reg0 = reg->reg1 = reg->reg2 = reg->reg3 = \
	reg->reg4 = reg->reg5 = reg->reg6 = reg->reg7 = 0;
}
uint8_t *get_register(struct game *g, uint8_t reg) {
	switch (reg & 7) {
		case 0: return &(g->registers->reg0);
		case 1: return &(g->registers->reg1);
		case 2: return &(g->registers->reg2);
		case 3: return &(g->registers->reg3);
		case 4: return &(g->registers->reg4);
		case 5: return &(g->registers->reg5);
		case 6: return &(g->registers->reg6);
		case 7: return &(g->registers->reg7);
		default: return NULL;
	}
}
struct game *new_game(uint8_t *rom, uint32_t rom_length) {
	if (rom_length == 0) return NULL;
	if (!rom) return NULL;
	struct game *g;
	g = calloc(1, sizeof(struct game));
	g->rom = rom;
	g->rom_length = rom_length;
	g->rom_index = 0;
	g->registers = calloc(1, sizeof(struct registers));
	reset_registers(g->registers);
	g->memory = calloc(1, RAM_SIZE);
	g->halt = 0;
	return g;
}
uint8_t rom_next(struct game *g) {
	++g->rom_index;
	if (g->rom_index == g->rom_length - 1) {
		return g->halt = 1;
	}
	return 0;
}
uint8_t rom_get(struct game *g) {
	return g->rom[g->rom_index];
}
uint8_t step(struct game *g) {
	if (g->halt) return 1;
	uint8_t *reg1;
	uint8_t *reg2;
	uint8_t *reg3;
	uint16_t mem;
	uint8_t instr = rom_get(g);
	switch (instr) {
		case 0x00: // noop
			break;
		case 0x01: // next frame
			rom_next(g);
			return 1;
		case 0x02: // copy byte from register to memory
		case 0x03: // copy byte from memory to register
			if (rom_next(g)) return 1;
			reg1 = get_register(g, rom_get(g));
			if (rom_next(g)) return 1;
			mem = ((uint16_t)rom_get(g)) << 8;
			if (rom_next(g)) return 1;
			mem |= rom_get(g);
			if (instr == 0x02)
				g->memory[mem] = *reg1;
			else
				*reg1 = g->memory[mem];
		case 0x10: // addition
		case 0x11: // subtraction
		case 0x12: // multiplication
		case 0x13: // division
		case 0x14: // modulo
			if (rom_next(g)) return 1;
			reg1 = get_register(g, rom_get(g));
			if (rom_next(g)) return 1;
			reg2 = get_register(g, rom_get(g));
			if (rom_next(g)) return 1;
			reg3 = get_register(g, rom_get(g));
			if (instr == 0x10) *reg3 = *reg1 + *reg2;
			if (instr == 0x11) *reg3 = *reg1 - *reg2;
			if (instr == 0x12) *reg3 = *reg1 * *reg2;
			if (instr == 0x13) *reg3 = *reg1 / *reg2;
			if (instr == 0x14) *reg3 = *reg1 % *reg2;
			break;
		case 0xff: // halt
			g->halt = 1;
			return 1;
		default:
			g->halt = 1;
			return 2;
	}
	rom_next(g);
	return 0;
}
uint8_t frame(struct game *g) {
	uint8_t r;
	while ((r = step(g)) == 0);
	return r;
}
