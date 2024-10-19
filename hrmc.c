// cl hrmc.c -nologo -Oi -Zi -GS- -Gs9999999 -link -subsystem:windows -nodefaultlib -stack:0x100000,0x100000 -machine:x64 -entry:"_start" && ./hrmc.exe ; echo $?
//gcc hrmc.c -o hrmc.exe -O0 -g -w -mwindows -m64 -nostdlib -Wl,-e_start && ./hrmc.exe ; echo $?
/* hrmc.c might turn into a C implementaiton of the hrmc compiler/editor
for now I just used it to debug what was going on with DefWindowProcA and it helped
*/

typedef          char      i8;
typedef unsigned char      u8;
typedef          short     i16;
typedef unsigned short     u16;
typedef          int       i32;
typedef unsigned int       u32;
typedef          long long i64;
typedef unsigned long long u64;
typedef          float     f32;
typedef          double    f64;

//// start min_crt.h ////
#pragma function(memset)
void* memset(void* dest, int val, i64 count) {
	u8* bytes = (u8*)dest;
	while(count--)
		*bytes++ = (u8)val;
	return dest;
}
#pragma function(memcpy)
void* memcpy(void* dest, void* src, i64 n) {
	u8* destBytes = (u8*)dest;
	u8* srcBytes = (u8*)src;
	while(n--) *destBytes++ = *srcBytes++;
	return dest;
}
#pragma function(memmove)
void* memmove(void* dest, void* src, i64 n) {
	u8* destBytes = (u8*)dest;
	u8* srcBytes = (u8*)src;
	if(destBytes > srcBytes && destBytes < srcBytes + n) {
		destBytes += n - 1;
		srcBytes += n - 1;
		while(n--) *destBytes-- = *srcBytes--;
	}
	else {
		while(n--) *destBytes++ = *srcBytes++;
	}
	return dest;
}
#pragma function(memcmp)
int memcmp(void* a, void* b, i64 size) {
	u8* ab = (u8*)a;
	u8* bb = (u8*)b;
	while(size--) {
		if(*ab != *bb) return *ab - *bb;
		ab++; bb++;
	}
	return 0;
}
#pragma function(strcmp)
int strcmp(char* s1, char* s2) {
	while(*s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return (*s1 == *s2) ? 0 : *s1 - *s2;
}
#pragma function(strlen)
u64 strlen(char* str) {
	u64 size = 0;
	while(str[size])
		size++;
	return size;
}
void assert(int value) {
	if(!value) {volatile u8* c = 0; *c = 1;}
}
#define offsetof(t, f) (&(((t *)0)->f))

// stack check and floating point stuff some compilers need
void __chkstk() {}
void ___chkstk_ms() {}
int _fltused;
u64 __fixunssfdi(f32 a1) {
	if(a1 < 0) return 0;
	return (u64)((i64)a1);
}
u64 __fixunsdfdi(f64 a1) {
	if(a1 < 0) return 0;
	return (u64)((i64)a1);
}
f32 __floatundisf(u64 a) {
	return (f32)((i64)a);
}
f64 __floatundidf(u64 a) {
	return (f64)((i64)a);
}
//// end min_crt.h ////

//// start common.h ////
char parse_escape_char(char* in) {
	if(in[0] != '\\') return in[0];
	if(in[1] == '\\') return '\\';
	if(in[1] ==  '0') return '\0';
	if(in[1] ==  'n') return '\n';
	if(in[1] ==  't') return '\t';
	if(in[1] ==  'r') return '\r';
	if(in[1] == '\'') return '\'';
	if(in[1] == '\"') return '\"';
	return in[1];
}
u32 parse_based_int(char* s, u32 i, u64* num, u32 base) {
	for(*num = 0; ; i += 1) {
		     if(s[i] >= '0' && s[i] <= '9') { *num *= base; *num += s[i] - '0'; }
		else if(s[i] >= 'a' && s[i] <= 'f') { *num *= base; *num += s[i] - 'a' + 10; }
		else if(s[i] >= 'A' && s[i] <= 'F') { *num *= base; *num += s[i] - 'A' + 10; }
		else if(s[i] != '_') break;
	}
	return i;
}
u32 parse_int(char* s, u32 i, u64* num) {
	u32 base = 10;
	if(s[i] == '0') {
		     if(s[i + 1] == 'x') { i += 2; base = 16; }
		else if(s[i + 1] == 'o') { i += 2; base =  8; }
		else if(s[i + 1] == 'b') { i += 2; base =  2; }
		else                     { i += 1; base =  8; } // comment out to default to decimal
	}
	return parse_based_int(s, i, num, base);
}
u32 parse_num(char* s, u32 i, u64* pi, f64** ppf) {
	i = parse_int(s, i, pi);
	if(s[i] == '.') {
		f64 a = (f64)*pi;
		u32 denom_start = i + 1;
		i = parse_int(s, denom_start, pi);
		*ppf = (void*)pi;
		**ppf = a + (f64)(*pi) / (10.0 * (f64)(i - denom_start));
	}
	else { *ppf = 0; }
	return i;
}

typedef struct arena arena;
struct arena {
	u8* mem;
	u32 cap;
	u32 len;
	u32 i;
	u32 reserved_cap;
};
void* arena_push(arena* ar, u32 size) {
	assert(ar->len + size <= ar->cap);
	u8* ret = ar->mem + ar->len;
	ar->len += size;
	return ret;
}
void* arena_pop(arena* ar, u32 size) {
	assert(ar->len >= size);
	ar->len -= size;
	u8* ret = ar->mem + ar->len;
	return ret;
}
void arena_gap_splice(arena* ar, u32 i, u32 del_len, u8* new_data, u32 new_data_len) {
	// move gap to i from ar->i
	u8* end_of_gap = ar->mem + ar->i + (ar->cap - ar->len);
	if(i > ar->i)
		memcpy(ar->mem + ar->i, end_of_gap, i - ar->i);
	else
		memcpy(end_of_gap - (ar->i - i), ar->mem + i, ar->i - i);
	// delete chars from i
	ar->i = i - del_len;
	ar->len -= del_len;
	// push new_data
	assert(ar->len + new_data_len <= ar->cap);
	memcpy(ar->mem + ar->i, new_data, new_data_len);
	ar->i += new_data_len;
	ar->len += new_data_len;
}
//// end common.h ////

//// start min_win.h ////
typedef struct UNICODE_STRING {
	u16 Length;
	u16 MaximumLength;
	u16* Buffer;
} UNICODE_STRING;
typedef struct LIST_ENTRY {
	struct LIST_ENTRY *Flink;
	struct LIST_ENTRY *Blink;
} LIST_ENTRY;
typedef union ULARGE_INTEGER {
	struct {
		u32 LowPart;
		u32 HighPart;
	};
	struct {
		u32 LowPart;
		u32 HighPart;
	} u;
	u64 QuadPart;
} ULARGE_INTEGER;
typedef union LARGE_INTEGER {
	struct {
		u32 LowPart;
		u32 HighPart;
	};
	struct {
		u32 LowPart;
		u32 HighPart;
	} u;
	i64 QuadPart;
} LARGE_INTEGER;
typedef struct POINT {
	i32 x;
	i32 y;
} POINT;
typedef struct RECT {
	i32 left;
	i32 top;
	i32 right;
	i32 bottom;
} RECT;

#define MAX_PATH 260
typedef struct OVERLAPPED {
	u64 Internal;
	u64 InternalHigh;
	union {
		struct {
			u32 Offset;
			u32 OffsetHigh;
		};
		void* Pointer;
	};
	void* hEvent;
} OVERLAPPED;
typedef enum GET_FILEEX_INFO_LEVELS {
	GetFileExInfoStandard,
	GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;
typedef struct FILETIME {
	u32 dwLowDateTime;
	u32 dwHighDateTime;
} FILETIME;
typedef struct WIN32_FILE_ATTRIBUTE_DATA {
	u32    dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	u32    nFileSizeHigh;
	u32    nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA;
typedef struct WIN32_FIND_DATAA {
	u32 dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	u32 nFileSizeHigh;
	u32 nFileSizeLow;
	u32 dwReserved0;
	u32 dwReserved1;
	char cFileName[MAX_PATH];
	char cAlternateFileName[14];
} WIN32_FIND_DATAA;

typedef struct WNDCLASSA {
	u32 style;
	void* lpfnWndProc;
	i32 cbClsExtra;
	i32 cbWndExtra;
	void* hInstance;
	void* hIcon;
	void* hCursor;
	void* hbrBackground;
	char* lpszMenuName;
	char* lpszClassName;
} WNDCLASSA;
typedef struct MSG {
	void* hwnd;
	u32 message;
	u32* wParam;
	u64* lParam;
	u32 time;
	POINT pt;
} MSG;
typedef struct MONITORINFO {
	u32 cbSize;
	RECT rcMonitor;
	RECT rcWork;
	u32 dwFlags;
} MONITORINFO;
typedef struct BITMAPINFOHEADER {
  u32 biSize;
  i32  biWidth;
  i32  biHeight;
  i16  biPlanes;
  i16  biBitCount;
  u32 biCompression;
  u32 biSizeImage;
  i32  biXPelsPerMeter;
  i32  biYPelsPerMeter;
  u32 biClrUsed;
  u32 biClrImportant;
} BITMAPINFOHEADER;
typedef struct PAINTSTRUCT {
  void*  hdc;
  i32 fErase;
  RECT rcPaint;
  i32 fRestore;
  i32 fIncUpdate;
  u8 rgbReserved[32];
} PAINTSTRUCT;

// Console / File IO stuff
#define STD_INPUT_HANDLE -10
#define STD_OUTPUT_HANDLE -11
#define STD_ERROR_HANDLE -12
#define MAX_PATH 260
#define GENERIC_READ    0x80000000
#define GENERIC_WRITE   0x40000000
#define FILE_SHARE_READ 0x00000001
#define CREATE_ALWAYS 2
#define OPEN_EXISTING 3
#define INVALID_HANDLE_VALUE -1

// VirtualAlloc stuff
#define PAGE_READWRITE 0x04
#define PAGE_EXECUTE_READWRITE 0x40
#define MEM_COMMIT  0x1000
#define MEM_RESERVE 0x2000
#define MEM_RELEASE 0x8000

#define IMAGE_DOS_SIGNATURE 0x5A4D // MZ in ASCII
#define IMAGE_NT_SIGNATURE 0x00004550 // PE\0\0 in ASCII
// magic num for optional header (which is not actually optional for executables and DLLs)
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20b

// IMAGE_FILE_HEADER.Machine
#define IMAGE_FILE_MACHINE_I386 0x014c
#define IMAGE_FILE_MACHINE_AMD64 0x8664
#define IMAGE_FILE_MACHINE_ARM 0x01c0
#define IMAGE_FILE_MACHINE_ARM64 0xaa64
#define IMAGE_FILE_MACHINE_RISCV32 0x5032
#define IMAGE_FILE_MACHINE_RISCV64 0x5064
#define IMAGE_FILE_MACHINE_RISCV128 0x5128

// IMAGE_FILE_HEADER.Characteristics
#define IMAGE_FILE_RELOCS_STRIPPED 0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE 0x0002
#define IMAGE_FILE_LINE_NUMS_STRIPPED 0x0004 // deprecated, docs says should be 0 but w/e
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED 0x0008 // deprecated, docs says should be 0 but w/e
#define IMAGE_FILE_LARGE_ADDRESS_AWARE 0x0020
#define IMAGE_FILE_32BIT_MACHINE 0x0100
#define IMAGE_FILE_DEBUG_STRIPPED 0x0200
#define IMAGE_FILE_DLL 0x2000

// IMAGE_OPTIONAL_HEADER.Subsystem
#define IMAGE_SUBSYSTEM_UNKNOWN 0
#define IMAGE_SUBSYSTEM_WINDOWS_GUI 2
#define IMAGE_SUBSYSTEM_WINDOWS_CUI 3
#define IMAGE_SUBSYSTEM_EFI_APPLICATION 10
#define IMAGE_SUBSYSTEM_XBOX 14

// IMAGE_OPTIONAL_HEADER.DllCharacteristics
#define IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA 0x0020
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE 0x0040
#define IMAGE_DLLCHARACTERISTICS_NX_COMPAT 0x0100 // prevents executing memory with no exec flag (I think)
#define IMAGE_DLLCHARACTERISTICS_NO_SEH 0x0400 // no structured exception handling
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE 0x8000

// IMAGE_OPTIONAL_HEADER.DataDirectory[16] (unused directories not listed)
#define IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define IMAGE_DIRECTORY_ENTRY_RESOURCE 2
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3
#define IMAGE_DIRECTORY_ENTRY_SECURITY 4
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5
#define IMAGE_DIRECTORY_ENTRY_DEBUG 6
#define IMAGE_DIRECTORY_ENTRY_TLS 9
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT 11
#define IMAGE_DIRECTORY_ENTRY_IAT 12
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT 13

// IMAGE_SECTION_HEADER.Characteristics
#define IMAGE_SCN_CNT_CODE 0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA 0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080
#define IMAGE_SCN_MEM_EXECUTE 0x20000000
#define IMAGE_SCN_MEM_READ 0x40000000
#define IMAGE_SCN_MEM_WRITE 0x80000000

// misc size info
#define IMAGE_SIZEOF_DOS_HEADER 64
#define IMAGE_SIZEOF_FILE_HEADER 20
#define IMAGE_SIZEOF_NT_OPTIONAL64_HEADER 240
#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16
#define IMAGE_SIZEOF_SHORT_NAME 8

// Window Message stuff
#define WM_MOVE 0x0003
#define WM_SIZE 0x0005
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010
#define WM_DESTROY 0x0002
#define WM_QUIT 0x0012
#define PM_REMOVE 0x0001

// Window Style stuff
#define WS_OVERLAPPED       (0x00000000)
#define WS_CAPTION          (0x00C00000)
#define WS_SYSMENU          (0x00080000)
#define WS_THICKFRAME       (0x00040000)
#define WS_MINIMIZEBOX      (0x00020000)
#define WS_MAXIMIZEBOX      (0x00010000)
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_VISIBLE          (0x10000000)
#define WS_POPUP            (0x80000000)
#define CW_USEDEFAULT  ((int)0x80000000)
#define MONITOR_DEFAULTTOPRIMARY 0x00000001
#define SRCCOPY              0x00CC0020
#define DIB_RGB_COLORS 0
#define BI_RGB 0

//// start EXE format macros and structs (winnt.h) ////
// e_magic and e_lfanew are the only fields used today
typedef struct IMAGE_DOS_HEADER {
	u16 e_magic;    // IMAGE_DOS_SIGNATURE (aka MZ in ASCII)
	u16 e_cblp;     //0x02
	u16 e_cp;       //0x04
	u16 e_crlc;     //0x06
	u16 e_cparhdr;  //0x08
	u16 e_minalloc; //0x0A
	u16 e_maxalloc; //0x0C
	u16 e_ss;       //0x0E
	u16 e_sp;       //0x10
	u16 e_csum;     //0x12
	u16 e_ip;       //0x14
	u16 e_cs;       //0x16
	u16 e_lfarlc;   //0x18
	u16 e_ovno;     //0x1A
	u16 e_res[4];   //0x1C
	u16 e_oemid;    //0x24
	u16 e_oeminfo;  //0x26
	u16 e_res2[10]; //0x28
	u32 e_lfanew;   //0x3C address of IMAGE_NT_HEADERS64 relative to base of image
} IMAGE_DOS_HEADER;

typedef struct IMAGE_FILE_HEADER {
	u16 Machine;
	u16 NumberOfSections;
	u32 TimeDateStamp;        // UNSUED
	u32 PointerToSymbolTable; // UNSUED
	u32 NumberOfSymbols;      // UNSUED
	u16 SizeOfOptionalHeader;
	u16 Characteristics;
} IMAGE_FILE_HEADER;
typedef struct IMAGE_DATA_DIRECTORY {
	u32 VirtualAddress;
	u32 Size;
} IMAGE_DATA_DIRECTORY;

typedef struct IMAGE_OPTIONAL_HEADER64 {
	u16 Magic;
	u8 MajorLinkerVersion;          // UNSUED //0x2
	u8 MinorLinkerVersion;          // UNSUED //0x3
	u32 SizeOfCode;//0x4
	u32 SizeOfInitializedData;      // UNSUED //0x8
	u32 SizeOfUninitializedData;    // UNSUED //0xC
	u32 AddressOfEntryPoint;//0x10
	u32 BaseOfCode;                 // UNSUED//0x14
	u64 ImageBase;//0x18
	u32 SectionAlignment;//0x20
	u32 FileAlignment;//0x24
	u16 MajorOperatingSystemVersion; // UNSUED//0x28
	u16 MinorOperatingSystemVersion; // UNSUED//0x2A
	u16 MajorImageVersion;           // UNSUED//0x2C
	u16 MinorImageVersion;           // UNSUED//0x2E
	u16 MajorSubsystemVersion;//0x30
	u16 MinorSubsystemVersion;       // UNSUED//0x32
	u32 Win32VersionValue;          // UNSUED//0x34
	u32 SizeOfImage;//0x38
	u32 SizeOfHeaders;//0x3C
	u32 CheckSum;                   // UNSUED//0x40
	u16 Subsystem;//0x44
	u16 DllCharacteristics;          // UNSUED//0x46
	u64 SizeOfStackReserve;     // UNSUED//0x48
	u64 SizeOfStackCommit;//0x50
	u64 SizeOfHeapReserve;//0x58
	u64 SizeOfHeapCommit;       // UNSUED//0x60
	u32 LoaderFlags;                // UNSUED//0x68
	u32 NumberOfRvaAndSizes;        // UNSUED//0x6C
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];//0x70
} IMAGE_OPTIONAL_HEADER64;

typedef struct IMAGE_NT_HEADERS64 {
	u32 Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64;

// funcAddr = functionsList[ordinalsList[indexOfNameOfFuncInNamesList]]
typedef struct IMAGE_EXPORT_DIRECTORY {
	u32 Characteristics; // UNUSED
	u32 TimeDateStamp;   // UNUSED //0x04
	u16 MajorVersion;     // UNUSED //0x08
	u16 MinorVersion;     // UNUSED //0x0A
	u32 Name; // RVA (relative virtual address) of ASCII name of DLL //0x0C
	u32 Base; // OrdinalBase (unused? see https://stackoverflow.com/a/5654463) //0x10
	u32 NumberOfFunctions; //0x14
	u32 NumberOfNames; //0x18
	u32 AddressOfFunctions; // RVA of list of functions //0x1C
	u32 AddressOfNames; // RVA of list of 32-bit RVAs of ASCII names of functions //0x20
	u32 AddressOfNameOrdinals; // RVA of list of ordinals aka indices into functions list //0x24
} IMAGE_EXPORT_DIRECTORY;

typedef struct IMAGE_SECTION_HEADER {
	u8 Name[IMAGE_SIZEOF_SHORT_NAME];
	union {
		u32 PhysicalAddress;
		u32 VirtualSize;
	} Misc;
	u32 VirtualAddress;
	u32 SizeOfRawData;
	u32 PointerToRawData;
	u32 PointerToRelocations;
	u32 PointerToLinenumbers;
	u16 NumberOfRelocations;
	u16 NumberOfLinenumbers;
	u32 Characteristics;
} IMAGE_SECTION_HEADER;
//// end EXE format macros and structs (winnt.h) ////

//// start Thread Information/Environment Block (TIB/TEB) structs (winternl.h) ////
typedef struct PEB_LDR_DATA {
	void* Reserved1[4];
	LIST_ENTRY InMemoryOrderModuleList; // 0x20 offset
} PEB_LDR_DATA;
typedef struct LDR_DATA_TABLE_ENTRY {
	void* Reserved1[2];
	LIST_ENTRY InMemoryOrderLinks; // PEB_LDR_DATA.InMemoryOrderModuleList.Flink points here
	void* Reserved2[2];
	void* DllBase;
	void* Reserved3[2];
	UNICODE_STRING FullDllName;
} LDR_DATA_TABLE_ENTRY;
typedef struct PEB {
	void* Reserved1[3];
	PEB_LDR_DATA* Ldr; // 0x18 offset
} PEB;
// address of TEB is in GS register upon entry to our program
typedef struct TEB {
	void* Reserved1[12];
	PEB* ProcessEnvironmentBlock; // 0x60 offset
} TEB;
//// end min_win.h ////

#ifndef _MSC_VER
u64 __readgsqword(u32 Offset) {
	void *ret;
	__asm__ volatile ("movq	%%gs:%1,%0"
			: "=r" (ret) ,"=m" ((*(volatile u64*) Offset)));
	return (u64) ret;
}
#endif // _MSC_VER
/* some useful links to understand get_kernel32 and GetProcAddress:
https://hero.handmade.network/forums/code-discussion/t/129-howto_-_building_without_import_libraries
https://www.ired.team/offensive-security/code-injection-process-injection/finding-kernel32-base-and-function-addresses-in-shellcode
https://en.wikipedia.org/wiki/Win32_Thread_Information_Block
https://stackoverflow.com/questions/10810203/what-is-the-fs-gs-register-intended-for
https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#export-address-table
*/
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

void* GetProcAddress(u8* handle, char* proc_name, void* (*LoadLibraryA)(char*)) {
	u8* pe_hdr = handle + *(u32*)(handle+0x3C);
	u8* exports_table = handle + *(u32*)(pe_hdr+0x18+0x70);
	u32* names_table = handle + *(u32*)(exports_table+0x20);
	i64 index;
	if(proc_name[0] == '#') { // TODO: test this path
		parse_based_int(proc_name, 1, &index, 10);
	}
	else {
		i64 low = 0;
		i64 high = *(i32*)(exports_table+0x18);
		i64 comparison = 0;
		do {
			if(comparison > 0) low = index + 1;
			if(comparison < 0) high = index - 1;
			index = (high + low) / 2;
			char* cur_proc_name = handle + names_table[index];
			comparison = strcmp(proc_name, cur_proc_name);
		} while(comparison != 0 && low != high);
		if(strcmp(proc_name, handle+names_table[index]) != 0)
			return 0;
	}
	u16* ordinal_table = handle + *(u32*)(exports_table+0x24);
	u32* export_address_table = handle + *(u32*)(exports_table+0x1C);
	u8* addr = handle + export_address_table[ordinal_table[index]];
	u32 exports_table_size = *(u32*)(pe_hdr+0x18+0x70+4);
	if(addr < exports_table || addr >= exports_table + exports_table_size)
		return addr;
	char* forward_name = addr;
	u32 dotIdx = 0;
	while(forward_name[dotIdx] != '.') dotIdx += 1;
	char dll_name[256] = {0};
	memcpy(dll_name, forward_name, dotIdx+1);
	dll_name[dotIdx+1] = 'd';
	dll_name[dotIdx+2] = 'l';
	dll_name[dotIdx+3] = 'l';
	void* other_handle = LoadLibraryA(dll_name);
	return GetProcAddress(other_handle, forward_name+dotIdx+1, LoadLibraryA);
}

void* (*VirtualAlloc)         (void* lpAddress, u64 dwSize, u32 flAllocationType, u32 flProtect);
void* (*GetStdHandle)         (u32 nStdHandle);
void* (*CreateFileA)          (char* lpFileName, u32 dwDesiredAccess, u32 dwShareMode, void* lpSecurityAttributes, u32 dwCreationDisposition, u32 dwFlagsAndAttributes, void* hTemplateFile);
int   (*ReadFile)             (void* hFile, void* lpBuffer, u32 nNumberOfBytesToRead, u32* lpNumberOfBytesRead, void* lpOverlapped);
int   (*WriteFile)            (void* hFile, void* lpBuffer, u32 nNumberOfBytesToWrite, u32* lpNumberOfBytesWritten, void* lpOverlapped);
int   (*CloseHandle)          (void* hObject);
void* (*GetModuleHandleA)     (char* lpModuleName);
void* (*LoadLibraryA)         (char* LoadLibraryA);
u16   (*RegisterClassA)       (WNDCLASSA *lpWndClass);
void* (*CreateWindowExA)      (u32,char*,char*,u32,int,int,int,int,void*,void*,void*,void*);
u64*  (*DefWindowProcA)       (void* hWnd, u32 Msg, u32* wParam, u64* lParam);
void  (*PostQuitMessage)      (int nExitCode);
i32   (*PeekMessageA)         (MSG* lpMsg,void* hWnd,u32 wMsgFilterMin,u32 wMsgFilterMax,u32 wRemoveMsg);
i32   (*TranslateMessage)     (MSG *lpMsg);
u64*  (*DispatchMessageA)     (MSG *lpMsg);
int   (*GetFileAttributesExA) (char* lpFileName, int fInfoLevelId, void* lpFileInformation);
char* (*GetCommandLineA)      (void);
void  (*ExitProcess)          (u32 uExitCode);
int   (*GetMonitorInfoA)      (void* hMonitor,void* lpmi);
void* (*MonitorFromPoint)     (POINT,u32 dwFlags);
void* (*BeginPaint)           (void*, void*);
int   (*EndPaint)             (void*, void*);
int   (*StretchDIBits)        (void* hdc, int xDest, int yDest, int DestWidth, int DestHeight, int xSrc, int ySrc, int SrcWidth, int SrcHeight, void *lpBits, void *lpbmi, u32 iUsage, u32 rop);
void* (*GetDC)                (void*);
int   (*ReleaseDC)            (void*, void*);

//// start common_win.h ////
void* mem_alloc(u64 size) {
	return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

u64 file_modified_time(char* filename) {
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;
	if(!GetFileAttributesExA(filename, GetFileExInfoStandard, &fileInfo))
		return 0;
	ULARGE_INTEGER modified_time = { .HighPart = fileInfo.ftLastWriteTime.dwHighDateTime, .LowPart = fileInfo.ftLastWriteTime.dwLowDateTime };
	return modified_time.QuadPart;
}
u64 file_size(char* filename) {
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;
	if(!GetFileAttributesExA(filename, GetFileExInfoStandard, &fileInfo))
		return 0;
	ULARGE_INTEGER fileSize = { .HighPart = fileInfo.nFileSizeHigh, .LowPart = fileInfo.nFileSizeLow };
	return fileSize.QuadPart;
}
u32 file_read(char* filename, u8* in_buffer, u32 in_size) {
	// TODO: if we want to read / write files > 4GB we need to probably use the Ex version
	void* fin = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(fin == (void*)INVALID_HANDLE_VALUE)
		return 0;
	u32 bytes_read;
	if(!ReadFile(fin, in_buffer, in_size, &bytes_read, 0) || bytes_read != in_size)
		return 0;
	CloseHandle(fin);
	return bytes_read;
}
u32 file_write(char* filename, u8* out_buffer, u32 out_size) {
	void* fout = CreateFileA(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(fout == (void*)INVALID_HANDLE_VALUE)
		return 0;
	u32 bytes_written;
	if(!WriteFile(fout, out_buffer, out_size, &bytes_written, 0) || bytes_written != out_size)
		return 0;
	CloseHandle(fout);
	return bytes_written;
}

char* int2str(i64 value, char digits[16]) {
	int i = 14;
	do {
		digits[i--] = (value % 10) + '0';
		value /= 10;
	} while(value > 0 && i >= 0);
	return digits+i+1;
}
u32 console_log(char* message, u32 size) {
	void* fout = GetStdHandle(STD_OUTPUT_HANDLE);
	u32 bytes_written;
	if(size == 0) size = strlen(message);
	WriteFile(fout, message, size, &bytes_written, 0);
	return bytes_written;
}
u32 console_log_i64(i64 value) {
	void* fout = GetStdHandle(STD_OUTPUT_HANDLE);
	u32 bytes_written;
	char digits[16] = {0};
	int i = int2str(value, digits);
	WriteFile(fout, digits + i + 1, 14 - i, &bytes_written, 0);
	return bytes_written;
}

typedef struct simple_pe simple_pe;
struct simple_pe {
	IMAGE_DOS_HEADER dh;
	IMAGE_NT_HEADERS64 nt;
	IMAGE_SECTION_HEADER textsect;
};
simple_pe default_pe = {
	{ // IMAGE_DOS_HEADER
		IMAGE_DOS_SIGNATURE,
		0, // UNUSED
		.e_lfanew = sizeof(IMAGE_DOS_HEADER), // e_lfanew (ptr to new header - put right after this one)
	},
	{ // IMAGE_NT_HEADERS64
		IMAGE_NT_SIGNATURE,
		{ // IMAGE_FILE_HEADER
			IMAGE_FILE_MACHINE_AMD64,
			1, // NumberOfSections
			0, // TimeDateStamp
			0, 0, // PointerToSymbolTable, NumberOfSymbols
			sizeof(IMAGE_OPTIONAL_HEADER64), // SizeOfOptionalHeader
			IMAGE_FILE_RELOCS_STRIPPED | IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE | IMAGE_FILE_DEBUG_STRIPPED, // Characteristics
		},
		{ // IMAGE_OPTIONAL_HEADER64
			IMAGE_NT_OPTIONAL_HDR64_MAGIC,
			0, 0, // MajorLinkerVersion, MinorLinkerVersion
			1024, // SizeOfCode
			0, 0, // SizeOfInitializedData, SizeOfUninitializedData
			sizeof(simple_pe), // AddressOfEntryPoint
			0x1000, // BaseOfCode (compilers seem to just set this to 0x1000, dunno y)
			0x400000, // ImageBase
			4, 4, // SectionAlignment, FileAlignment
			0, 0, 0, 0, // MajorOperatingSystemVersion, MinorOperatingSystemVersion, MajorImageVersion, MinorImageVersion
			4, 0, // MajorSubsystemVersion, MinorSubsystemVersion
			0, // Win32VersionValue
			sizeof(simple_pe) + 1024, // SizeOfImage
			sizeof(simple_pe), // SizeOfHeaders
			0, // CheckSum
			IMAGE_SUBSYSTEM_WINDOWS_GUI, // Subsystem (2=GUI, 3=CUI, 10=EFI)
			IMAGE_DLLCHARACTERISTICS_NO_SEH,// | IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE | IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA, // DllCharacteristics
			0x100000,  // SizeOfStackReserve
			0x1000,    // SizeOfStackCommit
			0x1000000, // SizeOfHeapReserve
			0x1000,    // SizeOfHeapCommit
			0, // LoaderFlags
			IMAGE_NUMBEROF_DIRECTORY_ENTRIES, // NumberOfRvaAndSizes
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // DataDirectory array (first is Exports)
		},
	},
	{ // IMAGE_SECTION_HEADER
		'.', 't', 'e', 'x', 't', '\0', '\0', '\0', // Name
		1024, // VirtualSize
		sizeof(simple_pe), // VirtualAddress
		1024, // SizeOfRawData
		sizeof(simple_pe), // PointerToRawData
		0, 0, 0, 0, // PointerToRelocations, PointerToLinenumbers, NumberOfRelocations, NumberOfLinenumbers
		IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE, // Characteristics
	}
};
void write_exe(char* filename, u8* out_buf, u32 codesize) {
	// move code down to make room for executable
	memmove(out_buf + sizeof(default_pe), out_buf, codesize);
	memcpy(out_buf, &default_pe, sizeof(default_pe));
	simple_pe* pe = (simple_pe*)out_buf;
	pe->nt.OptionalHeader.SizeOfCode = codesize;
	pe->nt.OptionalHeader.SizeOfImage = sizeof(default_pe) + codesize;
	pe->textsect.Misc.VirtualSize = codesize;
	pe->textsect.SizeOfRawData = codesize;
	file_write("out.exe", out_buf, codesize + sizeof(default_pe));
}
//// end common_win.h ////

//https://courses.cs.washington.edu/courses/cse457/98a/tech/OpenGL/font.c
u8 rasters[][13] = {
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x36},
{0x00, 0x00, 0x00, 0x66, 0x66, 0xff, 0x66, 0x66, 0xff, 0x66, 0x66, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x7e, 0xff, 0x1b, 0x1f, 0x7e, 0xf8, 0xd8, 0xff, 0x7e, 0x18},
{0x00, 0x00, 0x0e, 0x1b, 0xdb, 0x6e, 0x30, 0x18, 0x0c, 0x76, 0xdb, 0xd8, 0x70},
{0x00, 0x00, 0x7f, 0xc6, 0xcf, 0xd8, 0x70, 0x70, 0xd8, 0xcc, 0xcc, 0x6c, 0x38},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x1c, 0x0c, 0x0e},
{0x00, 0x00, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0c},
{0x00, 0x00, 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x18, 0x30},
{0x00, 0x00, 0x00, 0x00, 0x99, 0x5a, 0x3c, 0xff, 0x3c, 0x5a, 0x99, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x18, 0x1c, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x60, 0x60, 0x30, 0x30, 0x18, 0x18, 0x0c, 0x0c, 0x06, 0x06, 0x03, 0x03},
{0x00, 0x00, 0x3c, 0x66, 0xc3, 0xe3, 0xf3, 0xdb, 0xcf, 0xc7, 0xc3, 0x66, 0x3c},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x38, 0x18},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0xe7, 0x7e},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0x07, 0x03, 0x03, 0xe7, 0x7e},
{0x00, 0x00, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0xff, 0xcc, 0x6c, 0x3c, 0x1c, 0x0c},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x03, 0x03, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x03, 0x7f, 0xe7, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x18, 0x1c, 0x1c, 0x00, 0x00, 0x1c, 0x1c, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06},
{0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60},
{0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x18, 0x0c, 0x06, 0x03, 0xc3, 0xc3, 0x7e},
{0x00, 0x00, 0x3f, 0x60, 0xcf, 0xdb, 0xd3, 0xdd, 0xc3, 0x7e, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x18},
{0x00, 0x00, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x7e, 0xe7, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0xfc, 0xce, 0xc7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc7, 0xce, 0xfc},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfc, 0xc0, 0xc0, 0xc0, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xcf, 0xc0, 0xc0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e},
{0x00, 0x00, 0x7c, 0xee, 0xc6, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06},
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xe0, 0xf0, 0xd8, 0xcc, 0xc6, 0xc3},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xdb, 0xff, 0xff, 0xe7, 0xc3},
{0x00, 0x00, 0xc7, 0xc7, 0xcf, 0xcf, 0xdf, 0xdb, 0xfb, 0xf3, 0xf3, 0xe3, 0xe3},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0x7e},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x3f, 0x6e, 0xdf, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x3c},
{0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xd8, 0xf0, 0xfe, 0xc7, 0xc3, 0xc3, 0xc7, 0xfe},
{0x00, 0x00, 0x7e, 0xe7, 0x03, 0x03, 0x07, 0x7e, 0xe0, 0xc0, 0xc0, 0xe7, 0x7e},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff},
{0x00, 0x00, 0x7e, 0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0xc3, 0xe7, 0xff, 0xff, 0xdb, 0xdb, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3},
{0x00, 0x00, 0xc3, 0x66, 0x66, 0x3c, 0x3c, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3},
{0x00, 0x00, 0xff, 0xc0, 0xc0, 0x60, 0x30, 0x7e, 0x0c, 0x06, 0x03, 0x03, 0xff},
{0x00, 0x00, 0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c},
{0x00, 0x03, 0x03, 0x06, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x30, 0x30, 0x60, 0x60},
{0x00, 0x00, 0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x66, 0x3c, 0x18},
{0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x38, 0x30, 0x70},
{0x00, 0x00, 0x7f, 0xc3, 0xc3, 0x7f, 0x03, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x7e, 0xc3, 0xc0, 0xc0, 0xc0, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x7f, 0xc3, 0xc3, 0xc3, 0xc3, 0x7f, 0x03, 0x03, 0x03, 0x03, 0x03},
{0x00, 0x00, 0x7f, 0xc0, 0xc0, 0xfe, 0xc3, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x33, 0x1e},
{0x7e, 0xc3, 0x03, 0x03, 0x7f, 0xc3, 0xc3, 0xc3, 0x7e, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x00},
{0x38, 0x6c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x00, 0x00, 0x0c, 0x00},
{0x00, 0x00, 0xc6, 0xcc, 0xf8, 0xf0, 0xd8, 0xcc, 0xc6, 0xc0, 0xc0, 0xc0, 0xc0},
{0x00, 0x00, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78},
{0x00, 0x00, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xdb, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xfc, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00, 0x00, 0x00, 0x00},
{0xc0, 0xc0, 0xc0, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x03, 0x03, 0x03, 0x7f, 0xc3, 0xc3, 0xc3, 0xc3, 0x7f, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xe0, 0xfe, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xfe, 0x03, 0x03, 0x7e, 0xc0, 0xc0, 0x7f, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x1c, 0x36, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x30, 0x00},
{0x00, 0x00, 0x7e, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x18, 0x3c, 0x3c, 0x66, 0x66, 0xc3, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0xe7, 0xff, 0xdb, 0xc3, 0xc3, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xc3, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0xc0, 0x60, 0x60, 0x30, 0x18, 0x3c, 0x66, 0x66, 0xc3, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0xff, 0x60, 0x30, 0x18, 0x0c, 0x06, 0xff, 0x00, 0x00, 0x00, 0x00},
{0x00, 0x00, 0x0f, 0x18, 0x18, 0x18, 0x38, 0xf0, 0x38, 0x18, 0x18, 0x18, 0x0f},
{0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},
{0x00, 0x00, 0xf0, 0x18, 0x18, 0x18, 0x1c, 0x0f, 0x1c, 0x18, 0x18, 0x18, 0xf0},
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x8f, 0xf1, 0x60, 0x00, 0x00, 0x00}
};

u64* Win32EventHandler(void* window, u32 msg, u32* wp, u64* lp) {
	if(msg == WM_DESTROY || msg == WM_CLOSE)
		ExitProcess(0);
	// TODO: key and mouse input
	return DefWindowProcA(window, msg, wp, lp);
}

void _start() {
	void* kernel32       = get_kernel32();
	LoadLibraryA         = GetProcAddress(kernel32, "LoadLibraryA", 0);
	VirtualAlloc         = GetProcAddress(kernel32, "VirtualAlloc", LoadLibraryA);
	GetModuleHandleA     = GetProcAddress(kernel32, "GetModuleHandleA", LoadLibraryA);
	GetCommandLineA      = GetProcAddress(kernel32, "GetCommandLineA", LoadLibraryA);
	ExitProcess          = GetProcAddress(kernel32, "ExitProcess", LoadLibraryA);
	GetStdHandle         = GetProcAddress(kernel32, "GetStdHandle", LoadLibraryA);
	GetFileAttributesExA = GetProcAddress(kernel32, "GetFileAttributesExA", LoadLibraryA);
	CreateFileA          = GetProcAddress(kernel32, "CreateFileA", LoadLibraryA);
	ReadFile             = GetProcAddress(kernel32, "ReadFile", LoadLibraryA);
	WriteFile            = GetProcAddress(kernel32, "WriteFile", LoadLibraryA);
	CloseHandle          = GetProcAddress(kernel32, "CloseHandle", LoadLibraryA);
	void* user32         = LoadLibraryA("user32.dll");
	RegisterClassA       = GetProcAddress(user32, "RegisterClassA", LoadLibraryA);
	CreateWindowExA      = GetProcAddress(user32, "CreateWindowExA", LoadLibraryA);
	PostQuitMessage      = GetProcAddress(user32, "PostQuitMessage", LoadLibraryA);
	PeekMessageA         = GetProcAddress(user32, "PeekMessageA", LoadLibraryA);
	TranslateMessage     = GetProcAddress(user32, "TranslateMessage", LoadLibraryA);
	DispatchMessageA     = GetProcAddress(user32, "DispatchMessageA", LoadLibraryA);
	DefWindowProcA       = GetProcAddress(user32, "DefWindowProcA", LoadLibraryA);
	GetMonitorInfoA      = GetProcAddress(user32, "GetMonitorInfoA", LoadLibraryA);
	MonitorFromPoint     = GetProcAddress(user32, "MonitorFromPoint", LoadLibraryA);
	BeginPaint           = GetProcAddress(user32, "BeginPaint", LoadLibraryA);
	EndPaint             = GetProcAddress(user32, "EndPaint", LoadLibraryA);
	GetDC                = GetProcAddress(user32, "GetDC", LoadLibraryA);
	ReleaseDC            = GetProcAddress(user32, "ReleaseDC", LoadLibraryA);
	void* gdi32          = LoadLibraryA("gdi32.dll");
	StretchDIBits        = GetProcAddress(gdi32, "StretchDIBits", LoadLibraryA);

	// create fullscreen window
	WNDCLASSA winclass = {0};
	winclass.lpfnWndProc = Win32EventHandler;
	winclass.hInstance = GetModuleHandleA(0);
	winclass.lpszClassName = "winclass";
	assert(RegisterClassA(&winclass));
	MONITORINFO mi = { sizeof(mi) };
	POINT ptZero = { 0, 0 };
	assert(GetMonitorInfoA(MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY), &mi));
	RECT monitor_rect = mi.rcMonitor;
	void* window = CreateWindowExA(0, winclass.lpszClassName, "title",
			//WS_OVERLAPPEDWINDOW|WS_VISIBLE, CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
			WS_POPUP|WS_VISIBLE, monitor_rect.left, monitor_rect.top, monitor_rect.right - monitor_rect.left, monitor_rect.bottom - monitor_rect.top,
			0, 0, winclass.hInstance, 0);

	int width = monitor_rect.right - monitor_rect.left;
	int height = monitor_rect.bottom - monitor_rect.top;
	BITMAPINFOHEADER bmih = {0};
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biHeight = -height;
	bmih.biWidth = width;
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;

	u32* pixel_data = mem_alloc(width*height*4);
	u32 tick = 0;

	char str[] = "the quick brown fox jumped over the lazy dog `1234567890-=~!@#$%^&*()_+";
	// event loop
	while(1) {
		MSG msg;
		while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		tick += 1;

		// TODO: audio

		u32* pixel = pixel_data;
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				*pixel++ = (((x+tick*2)%255) << 8) | ((y+tick)%255);
				//*pixel++ = 0;
			}
		}

		//char* str2 = int2str(tick, str);
		char* str2 = str;
		int yoff = 200;
		int xoff = 0;
		int spacing = 10;
		for(int i = 0; i < strlen(str2); i++) {
			u8* raster = rasters[str2[i] - 0x20];
			for(int y = 0; y < 13; y++) {
				for(int x = 0; x < spacing; x++) {
					if(raster[y] & (1 << x)) pixel_data[((13-y)+yoff)*width+((8-x)+xoff+i*spacing)] = -1;
					else                     pixel_data[((13-y)+yoff)*width+((8-x)+xoff+i*spacing)] = 0;
				}
			}
		}

		void* dc = GetDC(window);
		StretchDIBits(dc, 0, 0, width, height, 0, 0, width, height, pixel_data, &bmih, DIB_RGB_COLORS, SRCCOPY);
		ReleaseDC(window, dc);
	}
	ExitProcess(0);
}

