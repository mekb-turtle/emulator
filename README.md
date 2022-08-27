### Emulator

Frontend usage: `./gameboy [rom]`

Core:

Create a `struct game*` with `struct game* = new_game(uint8_t *rom, uint32_t rom_length)`. You can open a file with `fopen` and read it with `fread` to an array/pointer for example, check [frontend.c](frontend.c) for an example.

Advance frame with `frame(game)`, or one instruction with `step(game)`.

A ROM for this literally just contains the raw bytes of the instructions/arguments.

The compiler (which is really just a basic assembly language) will read word by word, words can be either instructions, numbers as hex (register 0-7 or memory address), or :labels. `//` for line comment, `/*` and `*/` for multi-line comment, `"` will start and end ignoring (embed a string, escape codes are supported)

Instruction cheat sheet, the "arguments" will read the next bytes and use those.

- `0x00`, `nop`, no arguments\
  no-op
- `0x01`, `nxt`, no arguments\
  next frame
- `0x02`, arguments: register, high byte mem, low byte mem\
- `smr`, arguments: register, mem\
  copy byte from register to memory
- `0x03`, `srm`, same\
  copy byte from memory to register
- `0x04`, `srb`, arguments: register, byte\
  set register to byte
- `0x05`, arguments: high byte mem, low byte mem, byte\
- `smb`, arguments: mem, byte\
  set at memory address to byte
- `0x06`, `srr`, arguments: `a` register, `b` register\
  set `a` register to `b` register
- `0x10`, `add`, arguments: `a` register, `b` register, `c` register\
  addition, `c = a + b`
- `0x11`, `sub`, same\
  subtraction, `c = a - b`
- `0x12`, `mul`, same\
  multiplication, `c = a * b`
- `0x13`, `div`, same\
  division, `c = a / b`
- `0x14`, `mod`, same\
  modulo, `c = a % b`
- `0x20`, `xor`, same\
  bit XOR, `c = a ^ b`
- `0x21`, `and`, same\
  bit AND, `c = a & b`
- `0x22`, `orr`, same\
  bit OR, `c = a | b`
- `0x23`, `xnr`, same\
  bit XNOR, `c = ~(a ^ b)`
- `0x24`, `nan`, same\
  bit NAND, `c = ~(a & b)`
- `0x25`, `nor`, same\
  bit NOR, `c = ~(a | b)`
- `0x26`, `not`, arguments: `a` register, `b` register\
  bit NOT, `b = ~a`
- `0x27`, `bsl`, arguments: `a` register, `b` register, `c` register\
  bit shift left, `c = a << b`
- `0x29`, `bsr`, same\
  bit shift right, `c = a >> b`
- `0x9f`, arguments: high byte mem, low byte mem\
  `psh`, arguments: label\
  push rom index to stack, add current rom index to stack, and jump to that rom index
- `0xa0`, arguments: `a` register, `b` register, high byte rom index, low byte rom index\
  `teq`, arguments: `a` register, `b` register, label\
  `psh` if `a == b`, `a` is equal to `b`
- `0xa1`, `tnq`, same\
  `psh` if `a != b`, `a` is not equal to `b`
- `0xa2`, `tgt`, same\
  `psh` if `a > b`, `a` is greater than `b`
- `0xa3`, `tlt`, same\
  `psh` if `a < b`, `a` is less than `b`
- `0xa4`, `tge`, same\
  `psh` if `a >= b`, `a` is greater than or equal to `b`
- `0xa5`, `tle`, same\
  `psh` if `a <= b`, `a` is less than or equal to `b`
- `0xaf`, `pop`, no arguments\
  pop rom index from stack, jump to the last rom index in stack and remove it from stack
- `0xb0`, `peq`, arguments: `a` register, `b` register\
  `pop` if `a == b`, `a` is equal to `b`
- `0xb1`, `pnq`, same\
  `pop` if `a != b`, `a` is not equal to `b`
- `0xb2`, `pgt`, same\
  `pop` if `a > b`, `a` is greater than `b`
- `0xb3`, `plt`, same\
  `pop` if `a < b`, `a` is less than `b`
- `0xb4`, `pge`, same\
  `pop` if `a >= b`, `a` is greater than or equal to `b`
- `0xb5`, `ple`, same\
  `pop` if `a <= b`, `a` is less than or equal to `b`
- `0xbf`, arguments: high byte rom index, low byte rom index\
  `jmp`, arguments: label\
  jump to rom index without pushing
- `0xc0`, arguments: `a` register, `b` register, high byte rom index, low byte rom index\
  `jeq`, arguments: `a` register, `b` register, label\
  `jmp` if `a == b`, `a` is equal to `b`
- `0xc1`, `jnq`, same\
  `jmp` if `a != b`, `a` is not equal to `b`
- `0xc2`, `jgt`, same\
  `jmp` if `a > b`, `a` is greater than `b`
- `0xc3`, `jlt`, same\
  `jmp` if `a < b`, `a` is less than `b`
- `0xc4`, `jge`, same\
  `jmp` if `a >= b`, `a` is greater than or equal to `b`
- `0xc5`, `jle`, same\
  `jmp` if `a <= b`, `a` is less than or equal to `b`
- `0xfe`, no arguments\
  begin ignoring instructions until the next `0x00`, useful for embedding strings\
- `0xff`, `hlt`, no arguments\
  halt
