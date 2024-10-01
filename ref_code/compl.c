//gcc -Os -c compl.c -o compl.o && objdump -d compl.o
/*
example code to compile to get ideas for how to implement the hrmc compiler in assembly
*/
typedef unsigned char u8;
typedef unsigned int u32;
extern u8* hrmc_table;
extern u8* hrmc_bytecode_start;
extern u8* hrmc_dest_start;
void memcpy(u8* a, u8* b, u32 size) {
    while(size--) { *a++ = *b++; }
}
void hrmc_compl() {
	u8* hrmc_bytecode = hrmc_bytecode_start;
	u8* hrmc_dest = hrmc_dest_start;
	while(1) {
		u8 c = hrmc_bytecode[0];
		u8 d = hrmc_bytecode[1];
		hrmc_bytecode += 2;
		u32 tbl_idx = (u32)c << 5;
		u32 dtbl_idx = (u32)d << 5;
		u8 category = c & 0xF0;
		memcpy(hrmc_dest, hrmc_table + tbl_idx, 32);
		hrmc_dest += 32;
		if(c == 0) ((void (*)())hrmc_dest)();
		if(category == 0xD0) *(u32*)(hrmc_dest - 16) = dtbl_idx;
		if(category == 0xE0) *(u8*)(hrmc_dest - 16) = d;
		if(c == 0xBB) *(u32*)(hrmc_dest - 16) = *(u8**)(hrmc_table+dtbl_idx) - hrmc_dest;
		if(category != 0x70) continue;
		if(c == 0x7E) *(u8**)(hrmc_table+dtbl_idx) = hrmc_dest;
		if(c == 0x7F) {
			u8* src_iter = hrmc_bytecode;
			u8* dest_iter = hrmc_dest;
			u8 target_d = d;
			while(1) {
				src_iter -= 2;
				c = src_iter[0];
				d = src_iter[1];
				dest_iter -= 32;
				if(d != target_d) continue;
				if(c == 0xBF || c == 0xB0) *(u32*)(dest_iter-16) = hrmc_dest - dest_iter;
				if(c == 0x7E) break;
			}
		}
	}
}
