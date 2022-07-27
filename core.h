#include <stdint.h>
#include <stdbool.h>
#define RAM_SIZE 0x10000
struct registers {
	uint8_t reg0;
	uint8_t reg1;
	uint8_t reg2;
	uint8_t reg3;
	uint8_t reg4;
	uint8_t reg5;
	uint8_t reg6;
	uint8_t reg7;
};
struct game {
	struct registers *registers;
	uint8_t *rom;
	uint32_t rom_length;
	uint32_t rom_index;
	uint8_t *memory;
	bool halt;
};
uint8_t *get_register(struct game *g, uint8_t reg);
struct game *new_game(uint8_t *rom, uint32_t rom_length);
uint8_t rom_next(struct game *g);
uint8_t rom_get(struct game *g);
uint8_t step(struct game *g);
uint8_t frame(struct game *g);
