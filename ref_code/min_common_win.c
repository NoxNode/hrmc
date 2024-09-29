/*
min_common_win.c is all the main common things I use from libc and win32 and my own stuff
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
#pragma function(assert)
void assert(int value) {
	if(!value) {volatile u8* c = 0; *c = 1;}
}

// below are from ctype.h
int isspace(int c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';// || c == '\v' || c == '\f';
}
int isdigit(int c) {
	return c >= '0' && c <= '9';
}
int isxdigit(int c) {
	return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
int isalpha(int c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
int isalnum(int c) {
	return isalpha(c) || isdigit(c);
}
int ispunct(int c) {
	return !isalnum(c) && c >= '!' && c <= '~';
	//return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~');
}
int toupper(int c) {
	if(c <= 'Z') return c;
	return c - ('a' - 'A');
}
int tolower(int c) {
	if(c >= 'a') return c;
	return c + ('a' - 'A');
}

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

//// start min_win.h ////
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

// define these first because we manually load these and use them to load the rest
void* (*GetProcAddress) (void* hModule, char* lpProcName);
void* (*LoadLibraryA) (char* lpLibFileName);

void  (*ExitProcess)          (u32 uExitCode);
char* (*GetCommandLineA)      (void);
void* (*VirtualAlloc)         (void* lpAddress, u64 dwSize, u32 flAllocationType, u32 flProtect);
int   (*VirtualProtect)       (void* lpAddress, u64 dwSize, u32 flNewProtect, u32* lpflOldProtect);
void* (*GetStdHandle)         (u32 nStdHandle);
void* (*CreateFileA)          (char* lpFileName, u32 dwDesiredAccess, u32 dwShareMode, void* lpSecurityAttributes, u32 dwCreationDisposition, u32 dwFlagsAndAttributes, void* hTemplateFile);
int   (*GetFileAttributesExA) (char* lpFileName, int fInfoLevelId, void* lpFileInformation);
int   (*ReadFile)             (void* hFile, void* lpBuffer, u32 nNumberOfBytesToRead, u32* lpNumberOfBytesRead, OVERLAPPED* lpOverlapped);
int   (*WriteFile)            (void* hFile, void* lpBuffer, u32 nNumberOfBytesToWrite, u32* lpNumberOfBytesWritten, OVERLAPPED* lpOverlapped);
void* (*FindFirstFileA)       (char* lpFileName, WIN32_FIND_DATAA* lpFindFileData);
int   (*FindNextFileA)        (void* hFindFile, WIN32_FIND_DATAA* lpFindFileData);
int   (*FindClose)            (void* hFindFile);
int   (*CloseHandle)          (void* hObject);

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

// e_magic and e_lfanew are the only fields used today
typedef struct IMAGE_DOS_HEADER {
	u16 e_magic; // IMAGE_DOS_SIGNATURE (aka MZ in ASCII)
	u16 e_cblp; //0x02
	u16 e_cp; //0x04
	u16 e_crlc; //0x06
	u16 e_cparhdr; //0x08
	u16 e_minalloc; //0x0A
	u16 e_maxalloc; //0x0C
	u16 e_ss; //0x0E
	u16 e_sp; //0x10
	u16 e_csum; //0x12
	u16 e_ip; //0x14
	u16 e_cs; //0x16
	u16 e_lfarlc; //0x18
	u16 e_ovno; //0x1A
	u16 e_res[4]; //0x1C
	u16 e_oemid; //0x24
	u16 e_oeminfo; //0x26
	u16 e_res2[10]; //0x28
	u32 e_lfanew; //0x3C address of IMAGE_NT_HEADERS64 relative to base of image
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
// TODO: see how gcc and msvc setup the memory for this (is it addresses, names, ordinals?)
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

//// start link_win.h ////
#ifndef _MSC_VER
u64 __readgsqword(u32 Offset) {
	void *ret;
	__asm__ volatile ("movq	%%gs:%1,%0"
			: "=r" (ret) ,"=m" ((*(volatile u64*) Offset)));
	return (u64) ret;
}
#endif // _MSC_VER
int link_win() {
	//// find kernel32.dll in loaded module list via TEB->PEB->Ldr->ModuleList
	PEB* peb = (PEB*)__readgsqword(0x60);
	char* moduleNode = (char*)peb->Ldr->InMemoryOrderModuleList.Flink;
	LDR_DATA_TABLE_ENTRY* moduleInfo = (LDR_DATA_TABLE_ENTRY*)(moduleNode - sizeof(void*)*2);
	char* kernel32 = 0;
	while(moduleInfo->DllBase) {
		// check moduleInfo->FullDllName.Buffer's last couple letters == kernel32.dll (case insensitive)
		int i = (moduleInfo->FullDllName.Length / 2) - 1; // FullDllName.Length is in bytes, and its wchars aka 2 bytes per char
		int j = 11; // 11 == index of last 'l' in "kernel32.dll"
		u16* kernel32Name = L"KERNEL32.DLL";
		while(j >= 0) {
			int upperToLowerCaseOffset = 'a' - 'A';
			if(moduleInfo->FullDllName.Buffer[i] != kernel32Name[j] && moduleInfo->FullDllName.Buffer[i] != kernel32Name[j] + upperToLowerCaseOffset)
				break; // does not match "kernel32.dll"
			i--;
			j--;
		}
		if(j < 0) { // found kernel32.dll
			kernel32 = (void*)moduleInfo->DllBase;
			break;
		}
		moduleNode = (char*)((LIST_ENTRY*)moduleNode)->Flink;
		moduleInfo = (LDR_DATA_TABLE_ENTRY*)(moduleNode - sizeof(void*)*2);
	}
	if(!kernel32) return 0;

	//// find GetProcAddress from kernel32.dll's DOS->PE->ExportTable
	// TODO: since this is basically what GetProcAddress does, we could make this a func and use it instead
	// also curious if you can load 2 modules with same name but diff path
	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)kernel32;
	IMAGE_NT_HEADERS64* peHeader = (IMAGE_NT_HEADERS64*)(kernel32 + dosHeader->e_lfanew);
	IMAGE_EXPORT_DIRECTORY* exportTable = (IMAGE_EXPORT_DIRECTORY*)(kernel32 + peHeader->OptionalHeader.DataDirectory[0].VirtualAddress);
	u32* namesTable = (u32*)(kernel32 + exportTable->AddressOfNames);
	// binary search for GetProcAddress in namesTable
	int low = 0;
	int high = exportTable->NumberOfNames;
	int index;
	char* funcName;
	int comparison = 0;
	do {
		if(comparison > 0) {
			low = index;
		}
		else if(comparison < 0) {
			high = index;
		}
		index = (high + low) / 2;
		funcName = (char*)(kernel32 + namesTable[index]);
		comparison = strcmp("GetProcAddress", funcName);
	} while(comparison != 0);
	// use index in names table as index to ordinals which holds the index to the function table
	u16* ordinalTable = (u16*)(kernel32 + exportTable->AddressOfNameOrdinals);
	u16 ordinal = ordinalTable[index];
	u32* exportAddressTable = (u32*)(kernel32 + exportTable->AddressOfFunctions);
	GetProcAddress = (void*)(kernel32 + exportAddressTable[ordinal]);
	if(!GetProcAddress) return 0;

	//// load libraries
	LoadLibraryA = GetProcAddress(kernel32, "LoadLibraryA");
	//void* user32      = LoadLibraryA("user32.dll");

	//// get func pointers from libraries
	ExitProcess = GetProcAddress(kernel32, "ExitProcess");
	GetCommandLineA = GetProcAddress(kernel32, "GetCommandLineA");
	VirtualAlloc = GetProcAddress(kernel32, "VirtualAlloc");
	VirtualProtect = GetProcAddress(kernel32, "VirtualProtect");
	GetStdHandle = GetProcAddress(kernel32, "GetStdHandle");
	CreateFileA = GetProcAddress(kernel32, "CreateFileA");
	GetFileAttributesExA = GetProcAddress(kernel32, "GetFileAttributesExA");
	ReadFile = GetProcAddress(kernel32, "ReadFile");
	WriteFile = GetProcAddress(kernel32, "WriteFile");
	FindFirstFileA = GetProcAddress(kernel32, "FindFirstFileA");
	FindNextFileA = GetProcAddress(kernel32, "FindNextFileA");
	FindClose = GetProcAddress(kernel32, "FindClose");
	CloseHandle = GetProcAddress(kernel32, "CloseHandle");
	return 1;
}
//// end link_win.h ////

//// start common_win.h ////
void* mem_alloc(u64 size) {
	return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}
int mem_setprot(void* p, u64 size, u32 prot) {
	u32 old_prot;
	return VirtualProtect(p, size, prot, &old_prot);
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
	int i = 14;
	do {
		digits[i--] = (value % 10) + '0';
		value /= 10;
	} while(value > 0 && i >= 0);
	WriteFile(fout, digits + i + 1, 14 - i, &bytes_written, 0);
	return bytes_written;
}

// TODO: add to common_win.h
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

//// start common.h ////
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
u32 hash_fnv1a(u8* ptr, u32 len) {
	u32 x = 0x811c9dc5;
	for(int i = 0; i < len; i++)
		x = (x ^ ptr[i]) * 0x01000193;
	return x;
}

// TODO: add below to common.c
u32 skip_name(char* s, u32 i) {
	while(s[i] == '_' || isalnum(s[i]))
		i += 1;
	return i;
}
u32 namecmp(char* a, char* b) {
	u32 alen = skip_name(a, 0);
	u32 blen = skip_name(b, 0);
	if(alen != blen) return alen - blen;
	return memcmp(a, b, alen);
}
u32 index_of_char(char* s, u32 i, char* chars, u32 instance, i32 dir) {
	u32 last_index = i;
	u32 cur_instance = 0;
	while(s[i]) {
		for(u32 j = 0; j < strlen(chars); j += 1) {
			if(s[i] != chars[j] && !(chars[j] == 'A' && (isalpha(s[i]) || s[i] == '_')))
				continue;
			cur_instance += 1;
			if(instance == cur_instance)
				return i;
			last_index = i;
		}
		i += dir;
	}
	return last_index;
}

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
char parse_xdigit(char hexchar) {
	if(hexchar >= 'a' && hexchar <= 'f')
		return hexchar - 'a' + 10;
	if(hexchar >= 'A' && hexchar <= 'F')
		return hexchar - 'A' + 10;
	return hexchar - '0';
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

// TODO: arena_grow (increase cap as long as below reserved_cap)
// TODO: arena_multi_gap_splice or pass arena_gap_splice an arena with indices+len into main arena
	// need rules for where to splice in at end and how to realloc when a gap is depleted in middle
// TODO: custom arena_stretch per application
	// if application has all arena structs at top of global arena
	// you can stretch an arena and memmove down all the contents of the others
	// as long as everything is relative pointers/indices it should all be fine
		// just need to update the mem pointers in the arenas but they're all at the top so ez
//// end common.h ////
