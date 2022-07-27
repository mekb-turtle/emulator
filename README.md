### Emulator

Frontend usage: `./gameboy [rom]`

Core:

Create a `struct game*` with `new_game(uint8_t *rom, uint32_t rom_length)`. You can open a file with `fopen` and read it with `fread` to an array/pointer for example, check [frontend.c](frontend.c) for an example.

`game` refers to the `struct game*` from the `new_game` command. Advance frame with `frame(game)`, or one instruction with `step(game)`.

Instruction cheat sheet:

- `0x00`\
  no-op
- `0x01`\
  next frame
- `0x02`, takes: register, high byte mem, low byte mem\
  copy byte from register to memory
- `0x03`, takes same as `0x02`\
  copy byte from memory to register
- `0x10`, takes: `a` register, `b` register, `c` register\
  addition, `a + b = c`
- `0x11`, takes same as `0x10`\
  subtraction, `a - b = c`
- `0x12`, takes same as `0x10`\
  multiplication, `a * b = c`
- `0x13`, takes same as `0x10`\
  division, `a / b = c`
- `0x14`, takes same as `0x10`\
  modulo, `a % b = c`
- `0xff`\
  halt
