#include <stdio.h>
#include <stdlib.h>
#include "./core.h"
void reset_registers(struct registers *registers) {
	registers->reg0 = registers->reg1 = registers->reg2 = registers->reg3 = \
	registers->reg4 = registers->reg5 = registers->reg6 = registers->reg7 = 0;
}
struct game *new_game(uint8_t *rom, uint32_t rom_length) {
	if (rom_length == 0) return NULL;
	if (!rom) return NULL;
	struct game *g;
	g = malloc(sizeof(struct game));
	g->rom = rom;
	g->rom_length = rom_length;
	g->registers = malloc(sizeof(struct registers));
	reset_registers(g->registers);
	g->ram = malloc(RAM_SIZE);
	g->index = 0;
	g->halt = 0;
	return g;
}
uint8_t next(struct game *g) {
	++g->index;
	if (g->index == g->rom_length - 1) {
		return g->halt = 1;
	}
	return 0;
}
uint8_t step(struct game *g) {
	if (g->halt) return 1;
	switch (g->rom[g->index]) {
		case 0x00: // noop
			return 0;
		case 0x01: // next frame
			return 1;
		case 0xff: // halt
			g->halt = 1;
			return 1;
		default:
			return 2;
	}
	next(g);
	return 0;
}
uint8_t frame(struct game *g) {
	uint8_t r;
	while ((r = step(g)) == 0);
	return r;
}
