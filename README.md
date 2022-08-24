### Emulator

Frontend usage: `./gameboy [rom]`

Core:

Create a `struct game*` with `struct game* = new_game(uint8_t *rom, uint32_t rom_length)`. You can open a file with `fopen` and read it with `fread` to an array/pointer for example, check [frontend.c](frontend.c) for an example.

Advance frame with `frame(game)`, or one instruction with `step(game)`.

A ROM for this literally just contains the raw bytes of the instructions/arguments.

Instruction cheat sheet, the "arguments" will read the next bytes and use those.

- `0x00`, no arguments\
  no-op
- `0x01`, no arguments\
  next frame
- `0x02`, arguments: register, high byte mem, low byte mem\
  copy byte from register to memory
- `0x03`, same\
  copy byte from memory to register
- `0x04`, arguments: register, byte\
  set register to byte
- `0x05`, arguments: high byte mem, low byte mem, byte\
  set at memory address to byte
- `0x10`, arguments: `a` register, `b` register, `c` register\
  addition, `a + b = c`
- `0x11`, same\
  subtraction, `a - b = c`
- `0x12`, same\
  multiplication, `a * b = c`
- `0x13`, same\
  division, `a / b = c`
- `0x14`, same\
  modulo, `a % b = c`
- `0x20`, same\
  bit XOR, `a ^ b = c`
- `0x21`, same\
  bit AND, `a & b = c`
- `0x22`, same\
  bit OR, `a | b = c`
- `0x23`, same\
  bit XNOR, `a ^ b = ~c`
- `0x24`, same\
  bit NAND, `a & b = ~c`
- `0x25`, same\
  bit NOR, `a | b = ~c`
- `0x26`, arguments: `a` register, `b` register\
  bit NOT, `a = ~b`
- `0x30`, arguments: high byte mem, low byte mem\
  push memory index to stack, add current memory index to stack, and jump to that memory
- `0x31`, no arguments\
  pop memory index from stack, jump to the last memory index in stack and remove it from stack
- `0xfe`, no arguments\
  begin ignoring instructions until the next `0x00`, useful for embedding strings\
- `0xff`, no arguments\
  halt
