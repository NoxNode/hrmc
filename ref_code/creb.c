//gcc creb.c -o creb.exe -g  -mwindows -m64 -nostdlib -w -Wl,--stack,0x100000 -DSTACK_CHECK_BUILTIN -Wl,-e_winstart && ./creb.exe creb.c && ./out.exe ; echo $?
/* Compiler Runner Emitter for Bonfire
creb is a top-down approach to a simple C compiler, unfinished
*/
#include "min_common_win.c"

typedef struct file_idx file_idx;
struct file_idx {
	u32 src_i;    // start of file contents in src_text
	i32 parent_i; // where we left off in contents of file that included us (-1 for main)
};

typedef struct sym_def sym_def;
struct sym_def {
	u32 src_i;      // start of name in src_text
	u16 locals_len; // local vars if func, member vars if struct
	u8 args_len;    // 0 = local var, 1 = global var, 2+ = func (inherited list if struct?)
	u8 type;        // (0-9 = int types, DF67E = float/vec types, ABC = forth word, union, struct)
	                // 2nd hexit 0-5 = levels of indirection, A-F = array with 0-5 levels of indirection
	                          // 6789 = constant, typename, skipword, keyword
};

typedef struct hrmc_instr hrmc_instr;
struct hrmc_instr {
	u8 op;
	u8 op2;
	u8 regs;
	u8 regs2;
	u32 imm;
};

#define TOTAL_MEMORY   32 * 1024 * 1024
#define TEXT_MEMORY    10 * 1024 * 1024
#define FILES_MEMORY          10 * 1024
#define SYM_DEFS_MEMORY     1024 * 1024
#define HRMC_MEMORY     5 * 1024 * 1024
#define MC_MEMORY       5 * 1024 * 1024
#define VO_STACK_MEMORY       10 * 1024
arena ga; // global arena
arena* src_text;
arena* src_files;
arena* sym_defs;
// TODO: arena for dims of array syms, store { src_i, dim }, search by src_i
arena* hrmc;
arena* mc;
sym_def prev_type;
sym_def prev_keyword;
char builtins[]  = { "u8 i8 u16 i16 u32 i32 u64 i64 f32 f64 f32x4 f32x8 f32x16 f32x4x4 void char short int long float double unsigned const struct union enum return if continue while else for break do typedef register" };
u8 builtin_types[] = {0, 1, 2,  3,  4,  5,  8,  9,  0xF,0xD,0x6,  0x7,  0xE,   0xE,    0,   1,   3,    5,  9,   0xF,  0xD,   0x90,    0x90, 0x90,  0x90, 0x90,0x90,  0x90,0x90,  0x90, 0x90,0x90,0x90,0x90,0x80, 0x80 };

file_idx* load_src_file(char* filename, i32 parent_file_i) {
	file_idx* new_file_idx = arena_push(src_files, sizeof(file_idx));
	new_file_idx->parent_i = parent_file_i;
	new_file_idx->src_i = src_text->len;
	u32 filesize = file_size(filename);
	if(filesize == 0) return 0;
	file_read(filename, arena_push(src_text, filesize + 1), filesize);
	return new_file_idx;
}
file_idx* file_lookup(u32 src_i) {
	file_idx* cur_file = (file_idx*)src_files->mem;
	for(cur_file += 1; cur_file < src_files->mem + src_files->len; cur_file += 1)
		if(src_i < cur_file->src_i) // we passed it, return previous
			return cur_file - 1;
	return cur_file - 1;
}

// TODO: separate arena for alphabetical sorted indices into sym_defs for fuzzy search or lookup
sym_def* sym_lookup(char* name) {
	u32 j = 0;
	for(u32 i = 0; i < strlen(builtins); i = skip_name(builtins, i) + 1) {
		if(namecmp(name, builtins+i) == 0) {
			if((builtin_types[j] & 0xF0) == 0) builtin_types[j] |= 0x70;
			prev_keyword.type = builtin_types[j];
			return &prev_keyword;
		}
		j += 1;
	}
	u32 name_len = skip_name(name, 0);
	arena_gap_splice(sym_defs, sym_defs->len, 0, 0, 0); // move gap to end
	sym_def* cur_sym = (sym_def*)sym_defs->mem;
	for(; cur_sym < sym_defs->mem + sym_defs->len; cur_sym += 1) {
		u32 sym_len = skip_name(src_text->mem, cur_sym->src_i) - cur_sym->src_i;
		if(sym_len == name_len && memcmp(src_text->mem + cur_sym->src_i, name, name_len) == 0)
			return cur_sym;
	}
	return 0;
}
void sym_add(sym_def sym) {
	// TODO: maybe add to src_text or something
	arena_gap_splice(sym_defs, sym_defs->i, 0, &sym, sizeof(sym_def));
}

i32 is_type(sym_def* sym) {
	if(!sym) return 0;
	return (sym->type & 0x0F) == 0x0B || (sym->type & 0x0F) == 0x0C || (sym->type & 0xF0) == 0x70;
}
i32 is_keyword(sym_def* sym) {
	if(!sym) return 0;
	return (sym->type & 0xF0) == 0x90;
}
i32 is_function(sym_def* sym) {
	if(!sym) return 0;
	return sym->args_len >= 2;
}
i32 is_local(sym_def* sym) {
	if(!sym) return 0;
	return sym->args_len == 0;
}
i32 is_global(sym_def* sym) {
	if(!sym) return 0;
	return sym->args_len == 1;
}

/* TODO: do stuff with val op stack to make infix into postfix type hrmc
TODO: if we parse through as if we're doing infix
	and find that we have too many vals on stack and are still expecting more vals, try postfix
		for example: 2 3 + 1 - instead of 2 + 3 - 1
		push 2
		push +
		push 3
		push - (+ is equal precedence so consume it and top 2 vals)
		push 1
		end expr so look to see if arity of ops matches vals
		OR
		push 2
		push 3
		push +
		push 1
		push - (+ is equal precedence so consume it and top 2 vals which are 1 and 3)
		end expr so look to see if arity of ops matches vals
		wait its same arity, I think the way to distinguish is to check positions of the vals
			if both vals are left of a binary operator, maybe start over as postfix notation
		or simply check at the end if the most recently pushed thing was an op or val
			if op: postfix; if val: infix
			to check prefix see if ( is followed by operator that's not normally prefix
parse into hrmc arena
when evaluating const exprs for global vars
	save hrmc->i
	save mc->i
	parse_expr
	add ret to hrmc
	hrmc_to_mc
	value = call mc at saved index (may have to do something diff for floats)
	restore mc->i
	restore hrmc->i
or
	save hrmc->i
	parse_expr
	add ret to hrmc
	value = hrmc_interp at saved index
	restore hrmc->i

// from boncc
// https://github.com/lotabout/write-a-C-interpreter/blob/master/tutorial/en/8-Expressions.md#precedence-of-operators
// see above link for an example of how the below works
// operators handled by expr
	// * / %
	// + -
	// << >>
	// < <= > >=
	// == !=
	// &
	// ^
	// |
	// &&
	// ||
	// ? :
	// = += -= ...
	// ,
Node* expr(CState* st) {
	while(st->tk != ';' && st->tk != 0) {
		push_val(st, primary(st));
		int new_op = st->tk;
		if(new_op == ';') break;
		int old_op = opstack_pos > 0 ? opstack[opstack_pos-1]->tok : 0;
		while(precedence(old_op) >= precedence(new_op)) {
			Node* node = pop_op(st);
			node->rhs = pop_val(st);
			node->lhs = pop_val(st);
			push_val(st, node);
			old_op = opstack_pos > 0 ? opstack[opstack_pos-1]->tok : 0;
		}
		push_op(st, mknode(new_op, 0, 0, 0));
		next(st);
	}
	while(opstack_pos > 0) {
		Node* node = pop_op(st);
		node->rhs = pop_val(st);
		node->lhs = pop_val(st);
		push_val(st, node);
	}
	Node* n = pop_val(st);
	printNode(n);
	return n;
}
*/

u32 parse_expr(char* s, u32 i) {
	for(; s[i] && s[i] != ';' && s[i] != '\n' && s[i] != '\r' && s[i] != '{' && s[i] != ':' && s[i] != ')'; i += 1) {
		if(isdigit(s[i])) {
			u64 v;
			f64* pf;
			u32 num_len = parse_num(s, i, &v, &pf) - i;
			if(pf) {
				// emit push float imm
				console_log(" pflt", 0);
			}
			else {
				// emit push int imm
				console_log(" pint", 0);
			}
			i += num_len - 1;
			continue;
		}
		if(isalpha(s[i]) || s[i] == '_') {
			u32 name_len = skip_name(s, i) - i;
			sym_def* sym = sym_lookup(s+i);
			if(is_function(sym)) {
				// parse param list then emit func call
			}
			if(is_type(sym)) {
				// typecast i guess
			}
			if(is_local(sym)) {
				// emit push local variable value
			}
			// emit push global variable value
			i += name_len - 1;
			continue;
		}
		//if(s[i] == ' ') continue;

		u32 op = s[i];
		     if(s[i] == '<' && s[i+1] == '<' && s[i+2] == '=') {console_log(s+i, 3);i+=2;}
		else if(s[i] == '>' && s[i+1] == '>' && s[i+2] == '=') {console_log(s+i, 3);i+=2;}
		else if(s[i] == '=' && s[i+1] == '=' && s[i+2] == '=') {console_log(s+i, 3);i+=2;}
		else if(s[i] == '!' && s[i+1] == '=' && s[i+2] == '=') {console_log(s+i, 3);i+=2;}
		else if(s[i] == '-' && s[i+1] == '>') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '+' && s[i+1] == '+') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '-' && s[i+1] == '-') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '<' && s[i+1] == '<') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '>' && s[i+1] == '>') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '<' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '>' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '=' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '!' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '&' && s[i+1] == '&') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '|' && s[i+1] == '|') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '+' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '-' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '*' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '/' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '%' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '&' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '^' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else if(s[i] == '|' && s[i+1] == '=') {console_log(s+i, 2);i+=1;}
		else {console_log(s+i, 1);}
		// do somethin with op
	}
	return i;
}

/*
for(;;) {
	sym_def* sym = sym_lookup(s+i);
	if(sym == 0) {
		// new local variable, use prev_type, count indirection and array dim
		add_sym((sym_def){i, 0, 0, prev_type->type);
		continue;	
	}
	if(is_type(sym))               { prev_type = *sym; i = skip_name(s, i); continue; }
	if((sym->type & 0xF0) == 0x80) {                   i = skip_name(s, i); continue; }
	if(namecmp(prev_keyword.src_i, "if") == 0)
	if(namecmp(prev_keyword.src_i, "else") == 0)
	if(namecmp(prev_keyword.src_i, "while") == 0)
	if(namecmp(prev_keyword.src_i, "for") == 0)
	if(namecmp(prev_keyword.src_i, "do") == 0)
	i = parse_expr(s, i);
	continue;
}

maybe push scopes when you enter
and while scopes_left:
	parse stuff
all parsing goes through the above
except maybe expr parsing

parse_var
	make sym
	use preceding type or type after :
	if followed by =
		parse expr and output (caller can check this by seeing if hrmc->i changed)
	return sym
parse_block(indent_level)
	track types and keywords
	if unknown
		var defn if preceded by type or followed by : or =
			parse_var then add to parent scope locals or not if global
		func def if followed by (
			parse_var til ) and add to params
				can maybe just parse_block with +1 to indent_level so it just does same line
				then put all those stuff in params instead of locals
			parse_block
		struct def otherwise
			parse_block
		continue
	expr (til ; or \n and only goes multiple lines if ends in binary infix op)
	if ':' or indent on next line
		emit start scope before previous expr aka condition
		emit break if 0
		parse block
		if there were two ';'s in scope start line
			put 3rd expr before end scope (iter expr)
		if do was on scope start line
			find while, parse expr, replace condition expr
		emit end scope
		if emitting machine code
			emit for scope we just finished
more generally
	keywords/tags can be assigned to symbols in an indented hierarchy
	going in a level of indentation usually only happens under a certain condition
	if not defining a symbol, it's a symbolic expression
	I guess that can also be a symbolic expression
	so really we just have a hierarchy of conditionally executed symbolic expressions
	maybe should start with filling scope_idx arena with indices of start of scopes with scope level
*/

u32 get_indent_level(char* s, u32 i) {
	i = index_of_char(s, i, "\n", 1, -1) + 1;
	u32 start_of_tabs = i;
	while(s[i] == '\t') i += 1;
	return i - start_of_tabs;
}

u32 parse_block(char* s, u32 i, u32 scope) {
	for(; s[i]; i += 1) {
		if(isalpha(s[i]) || s[i] == '_') {
			u32 name_i = i;
			i = skip_name(s, i);
			sym_def* sym = sym_lookup(s+name_i);
			if(is_type(sym)) {
				u32 prev_i = i;
				i = index_of_char(s, i, "A\n", 1, 1);
				u32 type_byte = 0;
				for(; prev_i < i; prev_i += 1) {
					if(s[prev_i] == '*') type_byte += 1;
				}
				u32 punct_i = index_of_char(s, name_i, "[=\n", 1, 1);
				if(type_byte > 5) console_log("too much indirection\n", 0);
				if(s[punct_i] == '[') type_byte += 0xA;
				if((type_byte & 0xF0) != 0) console_log("weird type\n", 0);
				type_byte << 4;
				type_byte |= sym->type;
				u32 name_len = skip_name(s, i) - i;
				console_log("\t", 0);
				console_log(s+i, name_len);
				console_log_i64(type_byte);
				console_log(" local var def", 0);
				punct_i = index_of_char(s, i, "=\n", 1, 1);
				if(s[punct_i] == '=') i = parse_expr(s, i);
				console_log("\n", 0);
			}
			if(is_keyword(sym)) {
				// TODO: sizeof, offsetof
				console_log("\t", 0);
				console_log(s+name_i, i - name_i);
				i = parse_expr(s, i);
				console_log("\n", 0);
				i = index_of_char(s, i, "{\n", 1, 1);
				if(s[i] == '{' || s[i] == ':') i = parse_block(s, i, get_indent_level(s, i));
			}
			i = index_of_char(s, i, "\n", 1, 1);
		}
		// expr (at end, check keyword and so stuff differently maybe)
		if(s[i] == '\n' && s[i+1] != '\t' && s[i+1] != '\n') {
			// cleanup func
			break;
		}
	}
	return i;
}

int WinMain(void* I, void* PI, char* C, int S) {
	link_win();

	//// init arenas
	ga = (arena){mem_alloc(TOTAL_MEMORY), TOTAL_MEMORY, 0};
	src_text      = arena_push(&ga, sizeof(arena));
	src_files     = arena_push(&ga, sizeof(arena));
	sym_defs      = arena_push(&ga, sizeof(arena));
	hrmc          = arena_push(&ga, sizeof(arena));
	mc            = arena_push(&ga, sizeof(arena));
	// can add padding for more arenas here
	*src_text     = (arena){arena_push(&ga, TEXT_MEMORY),     TEXT_MEMORY,     0};
	*src_files    = (arena){arena_push(&ga, FILES_MEMORY),    FILES_MEMORY,    0};
	*sym_defs     = (arena){arena_push(&ga, SYM_DEFS_MEMORY), SYM_DEFS_MEMORY, 0};
	*hrmc         = (arena){arena_push(&ga, HRMC_MEMORY),     HRMC_MEMORY,     0};
	*mc           = (arena){arena_push(&ga, MC_MEMORY),       MC_MEMORY,       0};

	//// load main file
	char* command_line_text = GetCommandLineA();
	char* main_filename = command_line_text + index_of_char(command_line_text, 0, " ", -1, 1) + 1;
	if(main_filename == command_line_text || !load_src_file(main_filename, -1)) {
		console_log("expected valid src file\n", 0);
		ExitProcess(0);
	}

	//// parse src text into hrmc
	char* s = src_text->mem;
	for(u32 i = 0;;) {
		if(s[i] == '/' && s[i + 1] == '/') { i = index_of_char(s, i, "\n", 1, 1); continue; }
		if(s[i] == '/' && s[i + 1] == '*') {
			i += 1;
			while(s[i]) {
				i = index_of_char(s, i + 1, "*", 1, 1);
				if(s[i + 1] == '/') { i += 2; break; }
			}
			continue;
		}
		if(s[i] == 0) { // end of file, go to parent or break if no parent
			file_idx* cur_file = file_lookup(i);
			if(cur_file->parent_i == -1) break;
			i = cur_file->parent_i;
			continue;
		}
		if(s[i] == '#') {
			if(namecmp(s+i+1, "include") == 0) {
				u32 filename_i = index_of_char(s, i+1+strlen("include"), "A", 1, 1);
				i = index_of_char(s, filename_i, ">\"\n", 1, 1);
				char prev_c = s[i];
				s[i] = 0; // make null terminated for os file_load func
				file_idx* new_file = load_src_file(s+filename_i, index_of_char(s, i, "\n", 1, 1));
				s[i] = prev_c; // put the char back
				i = new_file->src_i;
				continue;
			}
			if(namecmp(s+i+1, "define") == 0) { // TODO: global constant
				i = index_of_char(s, i+1+strlen("define"), "A", 1, 1);
				sym_add((sym_def){i, 0, 0, 0x65});
				console_log("#define ", 0);
				console_log(s+i, skip_name(s, i) - i); console_log("\n", 0);
			}
			i = index_of_char(s, i, "\n", 1, 1); // ignore other macros
			continue;
		}
		if(s[i] == ':') {
			console_log("forth word def\n", 0);
		}
		if(s[i] == '(') {
			console_log("lisp expr\n", 0);
		}
		if(!isalpha(s[i]) && s[i] != '_') { i += 1; continue; }

		sym_def* sym = sym_lookup(s+i);
		if(sym != 0) { // known symbol in global scope, track and skip
			if((sym->type & 0xF0) == 0x70) prev_type = *sym;
			if((sym->type & 0xF0) == 0x90) prev_keyword = *sym;
			// TODO: forth words and global func calls like unix commands
			i = skip_name(s, i);
			continue;
		}
		console_log(s+i, skip_name(s, i) - i);
		// TODO: determine what this is based on prev keyword before here
			// below should handle the lack of a keyword
		u32 punct_i = index_of_char(s, i, "=:(;\n", 1, 1);
		if(s[punct_i] == '(') { // function def
			console_log(" function\n", 0);
			i = parse_block(s, i, 0);
		}
		else if(s[punct_i] == ':' || s[punct_i] == '=' || s[punct_i] == ';') { // global var def
			console_log(" global var\n", 0);
		}
		else if(index_of_char(s, i, "\n", 1, 1) + 1 == index_of_char(s, i, "\t", 1, 1)) { // struct def
			console_log(" struct\n", 0);
			sym_add((sym_def){i, 0, 0, 0x0C});
			// if prev_keyword was union its a union
			// if prev_keyword was enum its enum
		}
		i = index_of_char(s, i, "\n", 1, 1);
		u32 start_of_block = i;
		while(s[i] && (s[i+1] == '\t' || s[i+1] == '\n'))
			i = index_of_char(s, i+1, "\n", 1, 1);
		continue;
	}

	//// test code
	u8* out = mc->mem;
	if(1) {
		char test_code[] = "6a 2a 58 c3";
		for(int i = 0; i < sizeof(test_code); i += 3)
			*(out++) = (parse_xdigit(test_code[i]) << 4) | parse_xdigit(test_code[i+1]);
	}

	write_exe("out.exe", mc->mem, out - mc->mem);
	ExitProcess(0);
}

// can't change entry point for nostdlib tcc, and gcc w/ standard linking can't redirect to here
	// so to support both we just gotta have 2 entry points and 1 calls the other
#ifndef STD_LINK
int _winstart() {
	WinMain(0, 0, 0, 0);
}
#endif
