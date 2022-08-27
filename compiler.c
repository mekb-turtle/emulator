#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include "./core.h"
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define strerr strerror(errno)
#define BLOCK 1024
struct game *game;
int usage(char *argv0) {
	eprintf("\
Usage: %s [in instructions] [out rom]\n", argv0);
	return 2;
}
int main(int argc, char *argv[]) {
#define INVALID { return usage(argv[0]); }
	uint8_t flag_done = 0;
	char *in_filename = NULL;
	char *out_filename = NULL;
	for (int i = 1; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1] != '\0' && !flag_done) {
			if (argv[i][1] == '-' && argv[i][2] == '\0') flag_done = 1; // -- denotes end of flags
			else INVALID
		} else {
			if (in_filename) {
				if (out_filename) {
					INVALID
				} else {
					out_filename = argv[i];
				}
			} else {
				in_filename = argv[i];
			}
		}
	}
	if (!in_filename) INVALID
	if (!out_filename) INVALID
	FILE *in_f;
	FILE *out_f;
	if (in_filename[0] == '-' && in_filename[1] == '\0') {
		in_f = stdin;
	} else {
		in_f = fopen(in_filename, "r");
		if (!in_f) { eprintf("fopen: %s: %s\n", in_filename, strerr); return errno; }
	}
	uint8_t *in = NULL;
	size_t in_len = 0;
	while (1) {
		if (feof(in_f)) break;
		if (ferror(in_f)) {
			eprintf("ferror\n");
			return 1;
		}
		in = realloc(in, in_len + BLOCK);
		in_len += fread(in + in_len, 1, BLOCK, in_f);
	}
	uint8_t *out;
	size_t out_len;
	char *error = NULL;
	size_t line = 0;
	size_t col = 0;
	errno = 0;
	if (!compile_to_rom(in, in_len, &out, &out_len, &error, &line, &col)) {
		if (error) {
			eprintf("Error: L%liC%li: %s\n", line, col, error);
		} else {
			eprintf("Error: %s\n", strerr);
		}
		return 1;
	}
	if (out_filename[0] == '-' && out_filename[1] == '\0') {
		out_f = stdout;
	} else {
		out_f = fopen(out_filename, "wb");
		if (!out_f) { eprintf("fopen: %s: %s\n", out_filename, strerr); return errno; }
	}
	fwrite(out, 1, out_len, out_f);
	return 0;
}
