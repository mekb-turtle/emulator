### Emulator

Frontend usage: `./gameboy [rom]`

Core:

Create a `struct game*` with `new_game(uint8_t *rom, uint32_t rom_length)`. You can open a file with `fopen` and read it with `fread` to an array/pointer for example, check [frontend.c](frontend.c) for an example.

`game` refers to the `struct game*` from the `new_game` command. Advance frame with `frame(game)`, or one instruction with `step(game)`.

Instruction cheat sheet:

- `0x00`: no-op
- `0x01`: next frame
- `0xff`: halt
