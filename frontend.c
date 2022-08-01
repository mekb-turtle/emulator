#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include "./core.h"
#define strerr strerror(errno)
#define ERROR(...) fprintf(stderr, ##__VA_ARGS__)
#define BLOCK 1024
struct game *game;
int usage(char *argv0) {
	fprintf(stderr, "\
Usage: %s [rom]\n", argv0);
	return 2;
}
int main(int argc, char *argv[]) {
#define INVALID { return usage(argv[0]); }
	bool flag_done = 0;
	char *rom_filename = NULL;
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1] != '\0' && !flag_done) {
			if (argv[i][1] == '-' && argv[i][2] == '\0') flag_done = 1; // -- denotes end of flags
			else INVALID
		} else {
			if (rom_filename) INVALID
			rom_filename = argv[i];
		}
	}
	if (!rom_filename) INVALID
	FILE *rom_f;
	if (rom_filename[0] == '-' && rom_filename[1] == '\0') {
		rom_f = stdin;
	} else {
		rom_f = fopen(rom_filename, "r");
		if (!rom_f) { ERROR("fopen: %s: %s\n", rom_filename, strerr); return errno; }
	}
	uint8_t* rom = NULL;
	size_t len = 0;
	while (1) {
		if (feof(rom_f)) break;
		if (ferror(rom_f)) {
			ERROR("ferror\n");
			return 1;
		}
		rom = realloc(rom, len + BLOCK);
		len += fread(rom + len, 1, BLOCK, rom_f);
	}
	if (len == 0) {
		ERROR("File is empty");
		return 2;
	}
	if (len > UINT32_MAX) {
		ERROR("File is too big");
		return 2;
	}
	game = new_game(rom, len);
	return 0;
}
