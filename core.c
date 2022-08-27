#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "./core.h"
#define BLOCK 1024
struct game *new_game(uint8_t *rom, uint32_t rom_length) {
	if (rom_length == 0) return NULL;
	if (!rom) return NULL;
	struct game *game;
	game = calloc(1, sizeof(struct game));
	if (!game) return NULL;
	game->rom = rom;
	game->rom_length = rom_length;
	game->rom_index = 0;
	game->stack_index = 0;
	reset_registers(game);
	game->flags = 0;
	return game;
}
void dispose_game(struct game *game) {
	if (!game) return;
	free(game);
}
void reset_registers(struct game *game) {
	for (uint8_t i = 0; i < REGISTERS_SIZE; ++i) game->registers[i] = 0;
}
uint8_t *get_register(struct game *game, uint8_t reg) {
	return &(game->registers[reg & 7]);
}
uint8_t rom_jump_without_stack(struct game *game, uint32_t index) {
	if (index >= game->rom_length) { game->flags |= FLAG_HALT; return 1; }
	game->rom_index = index;
	return 0;
}
uint8_t rom_jump_push(struct game *game, uint32_t index) {
	if (game->stack_index >= STACK_SIZE) { game->flags |= FLAG_HALT; return 1; }
	game->stack[game->stack_index++] = game->rom_index;
	game->rom_index = index;
	return 0;
}
uint8_t rom_jump_pop(struct game *game) {
	if (game->stack_index <= 0) { game->flags |= FLAG_HALT; return 1; }
	game->rom_index = game->stack[--game->stack_index];
	return 0;
}
uint8_t rom_next(struct game *game) {
	if (game->rom_index >= game->rom_length) { game->flags |= FLAG_HALT; return 1; }
	return rom_jump_without_stack(game, game->rom_index + 1);
}
uint8_t rom_get(struct game *game) {
	return game->rom[game->rom_index];
}
uint8_t step(struct game *game) {
	if (game->flags & FLAG_HALT) return STATUS_HALTED;
	uint8_t *reg1 = NULL;
	uint8_t *reg2 = NULL;
	uint8_t *reg3 = NULL;
	uint16_t mem = 0;
	uint16_t rom = 0;
	uint8_t byte = 0;
	uint8_t instr = rom_get(game);
	if (game->flags & FLAG_IGNORING) {
		if (instr == OP_UNIGNORE) game->flags &= ~FLAG_IGNORING;
	} else {
		switch (instr) {
			case OP_NOOP: // no-op
				break;
			case OP_NEXT_FRAME: // next frame
				if (rom_next(game)) return STATUS_END_OF_ROM;
				return STATUS_NEXT_FRAME;
			case OP_COPY_REGISTER_TO_MEMORY: // copy byte from register to memory
			case OP_COPY_MEMORY_TO_REGISTER: // copy byte from memory to register
			case OP_SET_REGISTER: // set register to byte
			case OP_SET_MEMORY: // set at memory address to byte
			case OP_SET_REGISTER_REGISTER: // set register to other register
				if (instr == OP_COPY_REGISTER_TO_MEMORY || instr == OP_COPY_MEMORY_TO_REGISTER || instr == OP_SET_REGISTER || instr == OP_SET_REGISTER_REGISTER) {
					if (rom_next(game)) return STATUS_END_OF_ROM;
					reg1 = get_register(game, rom_get(game));
				}
				if (instr == OP_SET_REGISTER_REGISTER) {
					if (rom_next(game)) return STATUS_END_OF_ROM;
					reg2 = get_register(game, rom_get(game));
				}
				if (instr == OP_COPY_REGISTER_TO_MEMORY || instr == OP_COPY_MEMORY_TO_REGISTER || instr == OP_SET_MEMORY) {
					if (rom_next(game)) return STATUS_END_OF_ROM;
					mem = ((uint16_t)rom_get(game)) << 8;
					if (rom_next(game)) return STATUS_END_OF_ROM;
					mem |= rom_get(game);
				}
				if (instr == OP_SET_REGISTER || instr == OP_SET_MEMORY) {
					if (rom_next(game)) return STATUS_END_OF_ROM;
					byte = rom_get(game);
				}
				if (mem >= MEMORY_SIZE)
					return STATUS_MEMORY_OUT_OF_BOUNDS;
				if (instr == OP_COPY_REGISTER_TO_MEMORY) game->memory[mem] = *reg1;
				if (instr == OP_COPY_MEMORY_TO_REGISTER) *reg1 = game->memory[mem];
				if (instr == OP_SET_REGISTER) *reg1 = byte;
				if (instr == OP_SET_MEMORY) game->memory[mem] = byte;
				if (instr == OP_SET_REGISTER_REGISTER) *reg1 = *reg2;
				break;
			case OP_ADDITION:
			case OP_SUBTRACTION:
			case OP_MULTIPLICATION:
			case OP_DIVISION:
			case OP_MODULO:
			case OP_BIT_XOR:
			case OP_BIT_AND:
			case OP_BIT_OR:
			case OP_BIT_XNOR:
			case OP_BIT_NAND:
			case OP_BIT_NOR:
			case OP_BIT_SHIFT_LEFT:
			case OP_BIT_SHIFT_RIGHT:
				if (rom_next(game)) return STATUS_END_OF_ROM;
				reg1 = get_register(game, rom_get(game));
				if (rom_next(game)) return STATUS_END_OF_ROM;
				reg2 = get_register(game, rom_get(game));
				if (rom_next(game)) return STATUS_END_OF_ROM;
				reg3 = get_register(game, rom_get(game));
				switch (instr) {
					case OP_ADDITION:        *reg3 =   *reg1 +  *reg2;  break;
					case OP_SUBTRACTION:     *reg3 =   *reg1 -  *reg2;  break;
					case OP_MULTIPLICATION:  *reg3 =   *reg1 *  *reg2;  break;
					case OP_DIVISION:        *reg3 =   *reg1 /  *reg2;  break;
					case OP_MODULO:          *reg3 =   *reg1 %  *reg2;  break;
					case OP_BIT_XOR:         *reg3 =   *reg1 ^  *reg2;  break;
					case OP_BIT_AND:         *reg3 =   *reg1 &  *reg2;  break;
					case OP_BIT_OR:          *reg3 =   *reg1 |  *reg2;  break;
					case OP_BIT_XNOR:        *reg3 = ~(*reg1 ^  *reg2); break;
					case OP_BIT_NAND:        *reg3 = ~(*reg1 |  *reg2); break;
					case OP_BIT_NOR:         *reg3 = ~(*reg1 &  *reg2); break;
					case OP_BIT_SHIFT_LEFT:  *reg3 =   *reg1 << *reg2;  break;
					case OP_BIT_SHIFT_RIGHT: *reg3 =   *reg1 >> *reg2;  break;
				}
				break;
			case OP_BIT_NOT:
				if (rom_next(game)) return STATUS_END_OF_ROM;
				reg1 = get_register(game, rom_get(game));
				if (rom_next(game)) return STATUS_END_OF_ROM;
				reg2 = get_register(game, rom_get(game));
				*reg2 = ~(*reg1);
				break;
			case OP_PUSH_STACK: // push stack
			case OP_PUSH_IF_EQUAL:
			case OP_PUSH_IF_NOT_EQUAL:
			case OP_PUSH_IF_GREATER:
			case OP_PUSH_IF_LESS:
			case OP_PUSH_IF_GREATER_OR_EQUAL:
			case OP_PUSH_IF_LESS_OR_EQUAL:
			case OP_JUMP:
			case OP_JUMP_IF_EQUAL:
			case OP_JUMP_IF_NOT_EQUAL:
			case OP_JUMP_IF_GREATER:
			case OP_JUMP_IF_LESS:
			case OP_JUMP_IF_GREATER_OR_EQUAL:
				if (instr != OP_PUSH_STACK) {
					if (rom_next(game)) return STATUS_END_OF_ROM;
					reg1 = get_register(game, rom_get(game));
					if (rom_next(game)) return STATUS_END_OF_ROM;
					reg2 = get_register(game, rom_get(game));
				}
				if (rom_next(game)) return STATUS_END_OF_ROM;
				rom = ((uint16_t)rom_get(game)) << 8;
				if (rom_next(game)) return STATUS_END_OF_ROM;
				rom |= rom_get(game);
				uint8_t is_true = 0;
				switch (instr) {
					case OP_PUSH_STACK:               case OP_JUMP:                                         is_true = 1; break;
					case OP_PUSH_IF_EQUAL:            case OP_JUMP_IF_EQUAL:            if (*reg1 == *reg2) is_true = 1; break;
					case OP_PUSH_IF_NOT_EQUAL:        case OP_JUMP_IF_NOT_EQUAL:        if (*reg1 != *reg2) is_true = 1; break;
					case OP_PUSH_IF_GREATER:          case OP_JUMP_IF_GREATER:          if (*reg1 >  *reg2) is_true = 1; break;
					case OP_PUSH_IF_LESS:             case OP_JUMP_IF_LESS:             if (*reg1 <  *reg2) is_true = 1; break;
					case OP_PUSH_IF_GREATER_OR_EQUAL: case OP_JUMP_IF_GREATER_OR_EQUAL: if (*reg1 >= *reg2) is_true = 1; break;
					case OP_PUSH_IF_LESS_OR_EQUAL:    case OP_JUMP_IF_LESS_OR_EQUAL:    if (*reg1 <= *reg2) is_true = 1; break;
				}
				if (is_true) {
					if (instr >= OP_JUMP_MIN ? rom_jump_without_stack(game, rom) : rom_jump_push(game, rom))
						return STATUS_STACK_OUT_OF_BOUNDS;
					return STATUS_CONTINUE;
				}
				break;
			case OP_POP_STACK: // pop stack
				if (rom_jump_pop(game)) return STATUS_STACK_OUT_OF_BOUNDS;
				break;
			case OP_IGNORE: // ignore instructions until next OP_UNIGNORE
				break;
			case OP_HALT: // halt
				game->flags |= FLAG_HALT;
				return STATUS_HALTED;
			default:
				game->flags |= FLAG_HALT;
				return STATUS_INVALID_INSTRUCTION;
		}
	}
	if (rom_next(game)) return STATUS_END_OF_ROM;
	return STATUS_CONTINUE;
}
uint8_t frame(struct game *game) {
	uint8_t r;
	while ((r = step(game)) == STATUS_CONTINUE);
	return r;
}

uint8_t add(uint8_t byte, uint8_t **rom, size_t *rom_real_len, size_t *rom_len) {
	++*rom_len;
	size_t new_len = (*rom_len-(*rom_len%BLOCK))+BLOCK;
	if (*rom_real_len < new_len) {
		*rom_real_len = new_len;
		*rom = realloc(*rom, new_len);
		if (!*rom) return 1;
	}
	(*rom)[(*rom_len)-1] = byte;
	return 0;
}
struct label {
	uint8_t *name;
	size_t index;
	size_t line;
	size_t col;
};
size_t getsize(uint8_t *str) {
	uint8_t *l = str;
	while (*l != 0) ++l;
	return l-str;
}
void copydata(uint8_t *dest, uint8_t *src, size_t size) {
	for (size_t i = 0; i < size; ++i) {
		dest[i] = src[i];
	}
}
uint8_t compare(uint8_t *str1, uint8_t *str2) {
	size_t str1_len = getsize(str1);
	size_t str2_len  = getsize(str2);
	if (str2_len != str1_len) return 0;
	for (size_t i = 0; i < str2_len; ++i) {
		if (str1[i] != str2[i]) return 0;
	}
	return 1;
}
uint8_t add_label(struct label **labels, size_t *labels_len, uint8_t *name, size_t index, size_t line, size_t col) {
	*labels = realloc(*labels, ((*labels_len)+1)*sizeof(struct label));
	if (!*labels) return 1;
	++*labels_len;
	size_t label_size = getsize(name);
	uint8_t *label_name = malloc(label_size);
	if (!label_name) return 1;
	copydata(label_name, name, label_size);
	(*labels)[(*labels_len)-1] = (struct label) { .name = label_name, .index = index, .line = line, .col = col };
	return 0;
}
struct label *find_label(struct label *labels, size_t labels_len, uint8_t *label_name) {
	for (size_t i = 0; i < labels_len; ++i) {
		if (compare(labels[i].name, label_name)) return &(labels[i]);
	}
	return NULL;
}
// TODO: use a struct for this instead of having 15 arguments lmao
uint8_t add_word(uint8_t *word, uint8_t *word_len, uint8_t **rom, size_t *rom_real_len, size_t *rom_len,
	struct label **label, size_t *label_len, struct label **label_ref, size_t *label_ref_len,
	char **error, uint8_t **last_instruction, uint8_t *arg_count, uint8_t *can_go,
	size_t line, size_t col) {
	word[*word_len] = '\0';
	if (*word_len > 0) {
		uint8_t
			arg_target_count = 0,
			arg_type_1 = ARG_TYPE_NONE,
			arg_type_2 = ARG_TYPE_NONE,
			arg_type_3 = ARG_TYPE_NONE,
			arg_type_4 = ARG_TYPE_NONE,
			not_instr = 0,
			instruction;
		if (*can_go) {
			if (*last_instruction) free(*last_instruction);
			size_t s = getsize(word) + 1;
			*last_instruction = malloc(s);
			if (!*last_instruction) return 1;
			copydata(*last_instruction, word, s);
		}
		// i'm sorry for this spaghetti code
		if (compare(*last_instruction, OPC_NOOP)) { arg_target_count = 0;
			instruction = OP_NOOP; } else
		if (compare(*last_instruction, OPC_NEXT_FRAME)) { arg_target_count = 0;
			instruction = OP_NEXT_FRAME; } else
		if (compare(*last_instruction, OPC_COPY_REGISTER_TO_MEMORY)) { arg_target_count = 2;
			arg_type_1 = ARG_TYPE_REGISTER; arg_type_2 = ARG_TYPE_2_BYTE; 
			instruction = OP_COPY_REGISTER_TO_MEMORY; } else
		if (compare(*last_instruction, OPC_COPY_MEMORY_TO_REGISTER)) { arg_target_count = 2;
			arg_type_1 = ARG_TYPE_REGISTER; arg_type_2 = ARG_TYPE_2_BYTE; 
			instruction = OP_COPY_MEMORY_TO_REGISTER; } else
		if (compare(*last_instruction, OPC_SET_REGISTER)) { arg_target_count = 2;
			arg_type_1 = ARG_TYPE_REGISTER; arg_type_2 = ARG_TYPE_BYTE; 
			instruction = OP_SET_REGISTER; } else
		if (compare(*last_instruction, OPC_SET_MEMORY)) { arg_target_count = 2;
			arg_type_1 = ARG_TYPE_2_BYTE; arg_type_2 = ARG_TYPE_BYTE; 
			instruction = OP_SET_MEMORY; } else
		if (compare(*last_instruction, OPC_SET_REGISTER_REGISTER)) { arg_target_count = 2;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; 
			instruction = OP_SET_REGISTER_REGISTER; } else
		if (compare(*last_instruction, OPC_ADDITION)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_ADDITION; } else
		if (compare(*last_instruction, OPC_SUBTRACTION)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_SUBTRACTION; } else
		if (compare(*last_instruction, OPC_MULTIPLICATION)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_MULTIPLICATION; } else
		if (compare(*last_instruction, OPC_DIVISION)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_DIVISION; } else
		if (compare(*last_instruction, OPC_MODULO)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_MODULO; } else
		if (compare(*last_instruction, OPC_BIT_XOR)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_BIT_XOR; } else
		if (compare(*last_instruction, OPC_BIT_AND)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_BIT_AND; } else
		if (compare(*last_instruction, OPC_BIT_OR)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_BIT_OR; } else
		if (compare(*last_instruction, OPC_BIT_XNOR)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_BIT_XNOR; } else
		if (compare(*last_instruction, OPC_BIT_NAND)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_BIT_NAND; } else
		if (compare(*last_instruction, OPC_BIT_NOR)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_BIT_NOR; } else
		if (compare(*last_instruction, OPC_BIT_NOT)) { arg_target_count = 2;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; 
			instruction = OP_BIT_NOT; } else
		if (compare(*last_instruction, OPC_BIT_SHIFT_LEFT)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_BIT_SHIFT_LEFT; } else
		if (compare(*last_instruction, OPC_BIT_SHIFT_RIGHT)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = arg_type_3 = ARG_TYPE_REGISTER; 
			instruction = OP_BIT_SHIFT_RIGHT; } else
		if (compare(*last_instruction, OPC_PUSH_STACK)) { arg_target_count = 1;
			arg_type_1 = ARG_TYPE_LABEL;
			instruction = OP_PUSH_STACK; } else
		if (compare(*last_instruction, OPC_PUSH_IF_EQUAL)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_PUSH_IF_EQUAL; } else
		if (compare(*last_instruction, OPC_PUSH_IF_NOT_EQUAL)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_PUSH_IF_NOT_EQUAL; } else
		if (compare(*last_instruction, OPC_PUSH_IF_GREATER)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_PUSH_IF_GREATER; } else
		if (compare(*last_instruction, OPC_PUSH_IF_LESS)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_PUSH_IF_LESS; } else
		if (compare(*last_instruction, OPC_PUSH_IF_GREATER_OR_EQUAL)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_PUSH_IF_GREATER_OR_EQUAL; } else
		if (compare(*last_instruction, OPC_PUSH_IF_LESS_OR_EQUAL)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_PUSH_IF_LESS_OR_EQUAL; } else
		if (compare(*last_instruction, OPC_POP_STACK)) { arg_target_count = 0;
			instruction = OP_POP_STACK; } else
		if (compare(*last_instruction, OPC_POP_IF_EQUAL)) { arg_target_count = 2;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER;
			instruction = OP_POP_IF_EQUAL; } else
		if (compare(*last_instruction, OPC_POP_IF_NOT_EQUAL)) { arg_target_count = 2;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER;
			instruction = OP_POP_IF_NOT_EQUAL; } else
		if (compare(*last_instruction, OPC_POP_IF_GREATER)) { arg_target_count = 2;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER;
			instruction = OP_POP_IF_GREATER; } else
		if (compare(*last_instruction, OPC_POP_IF_LESS)) { arg_target_count = 2;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER;
			instruction = OP_POP_IF_LESS; } else
		if (compare(*last_instruction, OPC_POP_IF_GREATER_OR_EQUAL)) { arg_target_count = 2;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER;
			instruction = OP_POP_IF_GREATER_OR_EQUAL; } else
		if (compare(*last_instruction, OPC_POP_IF_LESS_OR_EQUAL)) { arg_target_count = 2;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER;
			instruction = OP_POP_IF_LESS_OR_EQUAL; } else
		if (compare(*last_instruction, OPC_JUMP)) { arg_target_count = 1;
			arg_type_1 = ARG_TYPE_LABEL;
			instruction = OP_JUMP; } else
		if (compare(*last_instruction, OPC_JUMP_IF_EQUAL)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_JUMP_IF_EQUAL; } else
		if (compare(*last_instruction, OPC_JUMP_IF_NOT_EQUAL)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_JUMP_IF_NOT_EQUAL; } else
		if (compare(*last_instruction, OPC_JUMP_IF_GREATER)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_JUMP_IF_GREATER; } else
		if (compare(*last_instruction, OPC_JUMP_IF_LESS)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_JUMP_IF_LESS; } else
		if (compare(*last_instruction, OPC_JUMP_IF_GREATER_OR_EQUAL)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_JUMP_IF_GREATER_OR_EQUAL; } else
		if (compare(*last_instruction, OPC_JUMP_IF_LESS_OR_EQUAL)) { arg_target_count = 3;
			arg_type_1 = arg_type_2 = ARG_TYPE_REGISTER; arg_type_3 = ARG_TYPE_LABEL;
			instruction = OP_JUMP_IF_LESS_OR_EQUAL; } else
		if (compare(*last_instruction, OPC_HALT)) { arg_target_count = 0;
			instruction = OP_HALT; } else
		not_instr = 1;
#define ARG_TYPE (*arg_count == 1 ? arg_type_1 : *arg_count == 2 ? arg_type_2 : *arg_count == 3 ? arg_type_3 : *arg_count == 4 ? arg_type_4 : ARG_TYPE_NONE)
		uint8_t arg_type = ARG_TYPE;
		*can_go = *arg_count == 0;
		if (*can_go) {
			if (not_instr) {
				if (word[0] == ':') {
					if (word[1] == '\0') {
						*error = "Invalid label name";
						return 1;
					}
					if (find_label(*label, *label_len, word+1)) {
						*error = malloc(100);
						if (*error) { sprintf(*error, "Label already exists: %s", word+1); }
						return 1;
					}
					if (add_label(label, label_len, word+1, *rom_len, line, col)) return 1;
				} else {
					*error = malloc(100);
					if (*error) { sprintf(*error, "Unknown word: %s", word); }
					return 1;
				}
			} else {
				if (add(instruction, rom, rom_real_len, rom_len)) return 1;
			}
		} else {
			if (word[0] == ':') {
				*error = "Can't have a label here, was expecting an argument";
				return 1;
			}
			if (arg_type == ARG_TYPE_REGISTER) {
				if (word[1] != '\0' || word[0] < '1' || word[0] > '7') {
					*error = "Invalid register";
					return 1;
				}
				if (add(word[0] - '0', rom, rom_real_len, rom_len)) return 1;
			} else
#define NUMERIC(x) ((x >= 'a' && x <= 'f') || (x >= 'A' && x <= 'F') || (x >= '0' && x <= '9'))
#define TO_INT(x) ((x >= 'a' && x <= 'f') ? (x - 'a' + 0xA) : (x >= 'A' && x <= 'F') ? (x - 'A' + 0xA) : (x >= '0' && x <= '9') ? x - '0': 0)
#define TO_INT2(x, y) ((TO_INT(x) << 010) | TO_INT(y))
			if (arg_type == ARG_TYPE_BYTE) {
				if (!(NUMERIC(word[0]) && NUMERIC(word[1]) && word[2] == '\0')) {
					*error = "Expecting byte here";
					return 1;
				}
				if (add(TO_INT2(word[0], word[1]), rom, rom_real_len, rom_len)) return 1;
			} else
			if (arg_type == ARG_TYPE_2_BYTE) {
				if (!(NUMERIC(word[0]) && NUMERIC(word[1]) && NUMERIC(word[2]) && NUMERIC(word[3]) && word[4] == '\0')) {
					*error = "Expecting 2 byte here";
					return 1;
				}
				if (add(TO_INT2(word[0], word[1]), rom, rom_real_len, rom_len)) return 1;
				if (add(TO_INT2(word[2], word[3]), rom, rom_real_len, rom_len)) return 1;
			} else
			if (arg_type == ARG_TYPE_LABEL) {
				if (word[0] == '\0') {
					*error = "Expecting a label here";
					return 1;
				}
				if (add_label(label_ref, label_ref_len, word, *rom_len, line, col)) return 1;
				if (add(0, rom, rom_real_len, rom_len)) return 1;
				if (add(0, rom, rom_real_len, rom_len)) return 1;
			} else if (*arg_count > arg_target_count) {
				// how did we get here??????
				*error = "This message should not appear";
				return 1;
			}
		}
		*word_len = 0;
		word[0] = '\0';
		++*arg_count;
		if (*arg_count > arg_target_count) *arg_count = 0;
		*can_go = *arg_count == 0;
	}
	return 0;
}
uint8_t compile_to_rom(uint8_t *in, size_t in_len, uint8_t **out, size_t *out_len, char **error, size_t *line, size_t *col) {
	*line = 1;
	*col = 0;
	uint8_t *word = malloc(32);
	if (!word) return 0;
	uint8_t word_len = 0;
	uint8_t is_string_escape = 0;
	uint8_t is_string = 0;
	uint8_t is_comment_line = 0;
	uint8_t is_comment_multiline = 0;
	uint8_t *rom = NULL;
	size_t rom_real_len = 0;
	size_t rom_len = 0;
	struct label *label = NULL;
	size_t label_len = 0;
	struct label *label_ref = NULL;
	size_t label_ref_len = 0;
	uint8_t *last_instruction = NULL;
	uint8_t arg_count = 0;
	uint8_t can_go = 1;
#define ERR() { if (rom) { free(rom); } if (word) { free(word); }; return 0; }
#define INVALID_CHAR(c) { *error = malloc(100); if (*error) { sprintf(*error, "Invalid character: %c", c); } ERR() }
#define ADD(byte) { if (add(byte, &rom, &rom_real_len, &rom_len)) ERR() }
#define ADD_WORD() { for (uint8_t i = 0; i < word_len; ++i) { if (word[i] == '/') INVALID_CHAR('/') } \
	if (add_word(word, &word_len, &rom, &rom_real_len, &rom_len,\
		&label, &label_len, &label_ref, &label_ref_len, error,\
		&last_instruction, &arg_count, &can_go, *line, *col)) ERR() }
	for (size_t i = 0; i < in_len; ++i) {
		if (is_string_escape) {
			is_string_escape = 0;
			switch (in[i]) {
				case 'a':  ADD('\x07'); break;
				case 'b':  ADD('\x08'); break;
				case 'e':  ADD('\x1b'); break;
				case 'f':  ADD('\x0c'); break;
				case 'n':  ADD('\x0a'); break;
				case 'r':  ADD('\x0d'); break;
				case 't':  ADD('\x09'); break;
				case 'v':  ADD('\x0b'); break;
				default:
					if (in[i] == 'x') {
						if (i + 2 < in_len && NUMERIC(in[i+1]) && NUMERIC(in[i+2])) {
							ADD(TO_INT2(in[i+1], in[i+2]));
							i += 2;
							continue;
							is_string_escape = 1;
						} else {
							*error = "Invalid escape code";
							ERR();
						}
					} else {
						ADD(in[i]);
					}
					break;
			}
		}
		if (i > 0 && in[i-1] == '\n') { *col = 0; ++*line; } else { ++*col; }
		if (!is_string_escape && is_string && in[i] == '"') { ADD(OP_UNIGNORE); is_string  = 0; continue; }
		if ((in[i] == '\n' || in[i] == '\r') && is_comment_line)    { is_comment_line      = 0; continue; }
		if (is_comment_multiline && in[i-1] == '*' && in[i] == '/') { is_comment_multiline = 0; continue; }
		if (is_comment_line || is_comment_multiline) continue;
		if (is_string) {
			if (in[i] == '\\') {
				is_string_escape = 1;
			} else {
				ADD(in[i]);
			}
			continue;
		}
		if (in[i-1] == '/') {
			if (in[i] == '*') { word[--word_len] = '\0'; ADD_WORD(); is_comment_multiline = 1; continue; }
			if (in[i] == '/') { word[--word_len] = '\0'; ADD_WORD(); is_comment_line      = 1; continue; }
		}
		if (in[i] == '"') {
			if (can_go) {
				ADD_WORD(); ADD(OP_IGNORE); is_string = 1; continue;
			} else {
				*error = "Can't start a string here, was expecting an argument";
				ERR();
			}
		}
		if (in[i] == ' ' || in[i] == '\0' || in[i] == '\t' || in[i] == '\r' || in[i] == '\n') {
			ADD_WORD();
		} else if ((in[i] >= 'a' && in[i] <= 'z') || (in[i] >= '0' && in[i] <= '9') || in[i] == '_' || (in[i] == ':' && word_len == 0) || in[i] == '/') {
			if (word_len > 16) {
				*error = "Word too long";
				ERR()
			}
			word[word_len] = in[i];
			word[++word_len] = '\0';
		} else {
			INVALID_CHAR(in[i]);
			ERR()
		}
	}
	ADD_WORD();
	for (size_t i = 0; i < label_ref_len; ++i) {
		struct label *l = find_label(label, label_len, label_ref[i].name);
		if (!l) {
			*error = malloc(100);
			*line = label_ref[i].line;
			*col  = label_ref[i].col;
			if (*error) { sprintf(*error, "Can't find label: %s", label_ref[i].name); }
			ERR();
		} else {
			rom[(label_ref[i].index)+0] = (l->index & 0xff00) >> 010;
			rom[(label_ref[i].index)+1] = (l->index & 0x00ff) >> 000;
		}
	}
	*out = rom;
	*out_len = rom_len;
	free(word);
	return 1;
}
