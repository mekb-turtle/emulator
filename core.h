#include <stdint.h>
#include <stdbool.h>
#define RAM_SIZE (1024*1024*8)
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
	uint32_t index;
	uint8_t *ram;
	bool halt;
};
struct game *new_game(uint8_t *rom, uint32_t rom_length);
uint8_t next(struct game *g);
uint8_t step(struct game *g);
uint8_t frame(struct game *g);
