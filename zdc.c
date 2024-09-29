//gcc zdc.c -o zdc.exe -O0 -g -w -mwindows -m64 -nostdlib -Wl,-e_start && ./zdc.exe ; echo $?
//objdump -M intel -d -j .text zdc.exe
//tcc zdc.c && ./zdc.exe ; echo $?
/* Zero Dependency Compiler
zdc is a bottom-up approach to a simple C compiler, unfinished
*/

typedef          char      i8;
typedef unsigned char      u8;
typedef          short     i16;
typedef unsigned short     u16;
typedef          int       i32;
typedef unsigned int       u32;
typedef          long long i64;
typedef unsigned long long u64;

#pragma function(memcmp)
int memcmp(u8* a, u8* b, i64 size) {
	for(; size >= 0; size -= 1) {
		if(*a != *b)
			return *a - *b;
		a++; b++;
	}
	return 0;
}
#pragma function(strcmp)
int strcmp(char* s1, char* s2) {
	while(*s1 && *s2 && *s1 == *s2) {
		s1++; s2++;
	}
	if(*s1 == *s2)
		return 0;
	return *s1 - *s2;
}
#pragma function(strlen)
u64 strlen(char* str) {
	u64 size = 0;
	while(str[size])
		size++;
	return size;
}

#ifndef _MSC_VER
u64 __readgsqword(u32 offset) {
	void *ret;
	__asm__ volatile ("movq	%%gs:%1,%0" : "=r" (ret) ,"=m" ((*(volatile u64*) (u64)offset)));
	return (u64) ret;
}
#endif // _MSC_VER
void* get_kernel32() {
	u8* peb = (u8*)__readgsqword(0x60); // find kernel32 in TEB->PEB->Ldr->ModuleList
	u8* ldr = *(u8**)(peb+0x18);
	u8* module_node = *(u8**)(ldr+0x20);
	u8* dll_base = *(u8**)(module_node+0x20);
	while(dll_base) { // loop through loaded modules looking for kernel32
		u16 dll_name_len = *(u16*)(module_node+0x38);
		u16* dll_name = *(u16**)(module_node+0x40);
		u16* kernel32_name = L"KERNEL32.DLL";
		i32 i = dll_name_len/2 - 1; // length is in bytes, but it's unicode so /2 for num chars
		i32 j = 11; // index of last L in KERNEL32.DLL
		while(j >= 0) { // see if last 11 characters are KERNEL32.DLL (case insensitive)
			if(dll_name[i] != kernel32_name[j] && dll_name[i] != kernel32_name[j] + ('a'-'A'))
				break;
			i--; j--;
		}
		if(j < 0) return dll_base; // found kernel32.dll
		module_node = *(u8**)module_node;
		dll_base = *(u8**)(module_node+0x20);
	}
	return 0; // not found
}

void* GetProcAddress(void* handle, char* proc_name) {
	u8* pe_hdr = handle + *(u32*)(handle+0x3C);
	u8* exports_table = handle + *(u32*)(pe_hdr+0x18+0x70);
	u32* names_table = handle + *(u32*)(exports_table+0x20);
	i32 low = 0;
	i32 high = *(i32*)(exports_table+0x18);
	i32 index;
	i32 comparison = 0;
	do {
		if(comparison > 0) low = index;
		if(comparison < 0) high = index;
		index = (high + low) / 2;
		char* cur_proc_name = handle + names_table[index];
		comparison = strcmp(proc_name, cur_proc_name);
	} while(comparison != 0);
	if(strcmp(proc_name, handle+names_table[index]) != 0) return 0;
	u16* ordinal_table = handle + *(u32*)(exports_table+0x24);
	u32* export_address_table = handle + *(u32*)(exports_table+0x1C);
	return handle + export_address_table[ordinal_table[index]];
}

void* (*VirtualAlloc)         (void* lpAddress, u64 dwSize, u32 flAllocationType, u32 flProtect);
void* (*GetStdHandle)         (u32 nStdHandle);
void* (*CreateFileA)          (char* lpFileName, u32 dwDesiredAccess, u32 dwShareMode, void* lpSecurityAttributes, u32 dwCreationDisposition, u32 dwFlagsAndAttributes, void* hTemplateFile);
int   (*ReadFile)             (void* hFile, void* lpBuffer, u32 nNumberOfBytesToRead, u32* lpNumberOfBytesRead, void* lpOverlapped);
int   (*WriteFile)            (void* hFile, void* lpBuffer, u32 nNumberOfBytesToWrite, u32* lpNumberOfBytesWritten, void* lpOverlapped);
int   (*CloseHandle)          (void* hObject);

u32 file_read(char* filename, u8* in_buffer, u32 in_size) {
	void* fin = CreateFileA(filename, 0x80000000, 1, 0, 3, 0, 0);
	u32 bytes_read;
	ReadFile(fin, in_buffer, in_size, &bytes_read, 0);
	CloseHandle(fin);
	return bytes_read;
}
u32 console_log(char* message, u32 size) {
	void* fout = GetStdHandle(-11); // STD_OUTPUT_HANDLE
	u32 bytes_written;
	if(size == 0) size = strlen(message);
	WriteFile(fout, message, size, &bytes_written, 0);
	return bytes_written;
}
i32 identlen(char* a) {
	char* pa = a;
	while(*a == '_' || (*a >= 'A' && *a <= 'Z') || (*a >= 'a' && *a <= 'z') || (*a >= '0' && *a <= '9'))
		a++;
	return a - pa;
}
i32 identcmp(char* a, char* b) {
	u32 alen = identlen(a);
	u32 blen = identlen(b);
	if(alen != blen) return alen - blen;
	return memcmp(a, b, alen);
}

typedef struct symbol symbol;
struct symbol {
	u32 src_i;
	u32 mc_i;
	u8 type;
	u8 flags;
	u16 reserved;
};

i32 _start() {
	void* kernel32 = get_kernel32();
	VirtualAlloc = GetProcAddress(kernel32, "VirtualAlloc");
	GetStdHandle = GetProcAddress(kernel32, "GetStdHandle");
	CreateFileA = GetProcAddress(kernel32, "CreateFileA");
	ReadFile = GetProcAddress(kernel32, "ReadFile");
	WriteFile = GetProcAddress(kernel32, "WriteFile");
	CloseHandle = GetProcAddress(kernel32, "CloseHandle");
	u32 mc_cap = 4096;
	u8* mc = VirtualAlloc(0, mc_cap, 0x3000, 0x40); // MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE
	u32 mc_len = 0;
	int (*jitmain)() = (int (*)())mc;
	u32 syms_cap = 1024;
	symbol* syms = VirtualAlloc(0, syms_cap*sizeof(symbol), 0x3000, 0x40); // MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE
	u32 syms_len = 0;

	u8 src[] = "0x1f 2+3-";
	i32 src_len = sizeof(src);
	for(i32 i = 0; i < src_len;) {
//	char* src = VirtualAlloc(0, 1024*1024, 0x3000, 0x40); // MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE
//	file_read("zdc.c", src, 1024*1024-1);
//	src[1024*1024-1] = 0;
//	for(u32 i = 0; src[i] != 0;) {
		// macros
		if(src[i] == '#') {
			while(src[i] && src[i] != '\n') i += 1;
			continue;
		}
		// comments
		if(src[i] == '/' && src[i+1] == '/') {
			while(src[i] && src[i] != '\n') i += 1;
			continue;
		}
		if(src[i] == '/' && src[i+1] == '*') {
			i += 2;
			while(src[i] && src[i] != '*' && src[i+1] != '/') i += 1;
			continue;
		}

		// operators TODO: push onto stack and codegen later according to precedence
		if(src[i] == '+') { *((u64*)(mc+mc_len)) = 0x50C801485859;     mc_len += 6; i += 1; continue; } // pop rcx; pop rax; add rax, rcx; push rax
		if(src[i] == '-') { *((u64*)(mc+mc_len)) = 0x50C829485859;     mc_len += 6; i += 1; continue; } // pop rcx; pop rax; sub rax, rcx; push rax
		if(src[i] == '*') { *((u64*)(mc+mc_len)) = 0x50C1AF0F485859;   mc_len += 7; i += 1; continue; } // pop rcx; pop rax; imul rax, rcx; push rax
		if(src[i] == '/') { *((u64*)(mc+mc_len)) = 0x50F9F74899485859; mc_len += 8; i += 1; continue; } // pop rcx; pop rax; cqo; idiv rcx; push rax
		if(src[i] == '%') { *((u64*)(mc+mc_len)) = 0x52F9F74899485859; mc_len += 8; i += 1; continue; } // pop rcx; pop rax; cqo; idiv rcx; push rdx
		// numbers
		if(src[i] >= '0' && src[i] <= '9') {
			u64 ival = 0;
			i32 base = 10;
			if(src[i+1] == 'x') { base = 16; i += 2; }
			while((src[i] >= '0' && src[i] <= '9') || (src[i] >= 'A' && src[i] <= 'F') || (src[i] >= 'a' && src[i] <= 'f')) {
				ival *= base;
				if(src[i] >= '0' && src[i] <= '9') ival += src[i] - '0';
				if(src[i] >= 'A' && src[i] <= 'F') ival += 10 + src[i] - 'A';
				if(src[i] >= 'a' && src[i] <= 'f') ival += 10 + src[i] - 'a';
				i += 1;
			}
			mc[mc_len++] = 0x48; // mov rax, i64
			mc[mc_len++] = 0xB8;
			*((u64*)(mc+mc_len)) = ival;
			mc_len += 8;
			mc[mc_len++] = 0x50; // push rax
			continue;
		}
		// identifiers
		if(src[i] == '_' || (src[i] >= 'A' && src[i] <= 'Z') || (src[i] >= 'a' && src[i] <= 'z')) {
			u32 found = 0;
			for(i32 j = 0; j < syms_len; j++) {
				if(identcmp(src+i, src+syms[j].src_i) == 0) {
					// found symbol, skip if keyword or push addr if user defined symbol
					// defer val loading to when we encounter the next binop and it's not an assign
					i += identlen(src+i);
					found = 1;
					break;
				}
			}
			if(found) continue;
			// new word? add defn to hierarchy lookup
			syms[syms_len++] = (symbol) {i, 0, 9, 0, 0};
			console_log(src+i, identlen(src+i));
			console_log("\n", 0);
			i += identlen(src+i);
			continue;
		}
		i += 1;
	}

	mc[mc_len++] = 0x58; // pop rax
	mc[mc_len++] = 0xc3; // ret
	// print machine code (can paste into https://defuse.ca/online-x86-assembler.htm)
	char hexchars[] = "0123456789ABCDEF";
	for(u32 i = 0; i < mc_len; i++) {
		console_log(&hexchars[(mc[i] & 0xF0)>>4], 1);
		console_log(&hexchars[mc[i] & 0xF], 1);
		if(mc[i] == 0x50) console_log("\n", 1);
	}
	console_log("\n", 1);
	return jitmain(); // run the jitted code
}
