// cl hrmc.c -nologo -Oi -Zi -GS- -Gs9999999 -link -subsystem:windows -nodefaultlib -stack:0x100000,0x100000 -machine:x64 -entry:"_start" && ./hrmc.exe ; echo $?
//gcc hrmc.c -o hrmc.exe -O0 -g -w -mwindows -m64 -nostdlib -Wl,-e_start && ./hrmc.exe ; echo $?
/* hrmc.c started as a thing to just help debug Win32 oddities.
It's now turned into a reference implementation of the code editor that I plan on eventually having in HRMC



viewspecs and controlspecs for each window and link
links also can address content in multiple ways
content address first char | address type
	letter | symbol name
	number | outline index
	#      | hash/id

things you can do with a mouse that right hand on keyboard can ideally also do
	select char/word/line/paragraph across windows
	scroll/zoom within window
	minimize/resize/close/move windows (if they have decorations)
	open context menu

left 00 right left hand guide  right hand guide
1234 56 7890  num param input  num param input
qwer ty uiop  local  commands  fast selection/reference
asdf g; hjkl  global commands  mouse motion replacement
zxcv bn m,./  typing ascii     mouse button replacement

shift and space are modifiers for commands and parameters
	hjkl with 00 = char/line, 0p = word/paragraph, h0 = line/buffer, hp = ?
alt+space to switch normal/engelbart mode
alt alone to use keys as other mode while alt held
/ = left click b/c can still do hjkl while holding
. = mostly vertical stuff
m = mostly horizontal stuff
, = least common stuff cuz difficult to do hjkl with it
lots of unprintable ascii characters can be repurposed as commands

global commands
	undo
	redo or maybe we just want a goto action history and select an entry there
	cut
	paste
	copy is cut and paste
if action history and command bindings are all done with text that could be cool
while inputting a command, show the table of commands and the binary/hex for them
commands and action history should be visible, configurable, and aligned with hexchords
can use a mouse button to hold the previous command
	and use the next command as the hjkl or uiop parameter

have 31 commands accessible at any time within a window (15 global/local)
	1 redo             repeat
	2 erase/delete     enter/open
	3 mark             match
	4 undo             wash/undo
	5 select           store
	6 indent           unindent
	7 tab/to           tag/symbol
	8 quit/close       query/search
	9 tab character
	A newline character
	B branch?
	C copy/compare
	D duplicate
	E end
	F find/cancel?

navigation
	move cursor/view
		by char/word/line/page/file
		goto prev/next/first/matching word/search/reference/visited spot/marker/scope
		get list of matching word/search/refs
	file
		open/list file/dir/buffer typed/highlighted
	window
		split/expand/shrink/restore vertical/horizontal
		restore sizeof all
		minimize all others
		close
		goto hjkl prev/next
		swap hjkl prev/next
	tab
		open/close
		goto next/prev
manipulation
	cut copy (un)indent join [motion]
	refer to register [character] for next command
	start/end/replay macro recording
		make macro from x history items
	undo/redo
	paste before/after
	next/prev autocomplete item
	replace insert math

text changes
	new_i, del, character
window/view changes
	split, scale, close, scroll, swap, select, zoom
	cursor_i, cursor_xoff
	move (when not tiled)
buffer/file changes
	open, switch, delete

  asdf            qwer            uiop
1 *#              ./^r            pP     
2 dD              :e              Over/back
3 mark fileloc    mark history    matching
4 Scroll          Window          Into/forward
5 Select          :w              Searched term
6 indent          gf              inner scope
7 To/Jump         Tab             Tag/symbol
8 yY              :q/q            u   
9 unindent        parent file     outer scope
A Replace & Find  Autocomplete    Above/top
B Bring together  prev autocomple Below/bottom
C Compare         :cd             Center
D Duplicate       Deploy/run      Door/home/data
E Replace mode    Enter reg       End/line
F Cancel          Cancel          Find/Til (fFtT)

Scroll/Window/Tab can be held so hjkl move w.r.t those scopes
	mouse movements could also be done at those scopes like the middle mouse click thing
	have like command and pending_command
		pending is what they're currently holding
		command is what they've just released

^s to save all modified
^S to save as
^e to open file
^t open tab
^w to close window/tab
^a select all
^tab and ^shift tab to switch tabs?
^zxcv to undo/cut/copy/paste selection
^Z to redo
^f find/replace
^r refresh
^b bookmarks
^h history
^l select url


memory
  num_arenas :i32
  reserved :i32
  arenas :mem_arena[]
view
  pos
  size
  zoom
  scroll_pos
  buffer_start_i
  cursor_i    // from text_arena.i
  cursor_xoff // relative to line start
  selection_start_i
  hrmc_i // could be used for multimedia?






c to hrmc compiler
  structs are just scoped consts
  names list and start stack slot
  then just shunting yard dat sheet
translate hrmc to everything by default
  toggle to not translate
    click or vimium select
      also show tool tip on hover
  gray out if translation failed

global
  text_i
  type_i
  num_locals
  locals_start_i
  locals_start_val
local
  text_i
  type_i
  globals_i_of_parent
text_arena      :u8[]
globals_arena   :global[]
globals_types_i :i32
globals_hrmc_i  :i32
locals_arena    :local[]

if compiling and see #include
  splice it in right there
within function, edits are dumb
  we just recompile the function every edit
if you edit the local names comment
  we update all the instances of that var within the func
  same for editing a global name
  we do that by re-translating the function from hrmc
can compile a hrmc compile function then call it to compile the rest of the hrmc
  is the only point of that adding extra features? if so the compile function can't use those features then
can split the locals meta info into 3 things
  initial stack slot number
  one array of numbers that's indices into the string array
  a string array that's null terminated names
  97 "store there" followed by a byte for the id of what's being stored (can also be used as a data table index to store an address - or can reuse 7E but store in data table if param is not 7x) then a u16 for size of following const data
  so the locals meta info can be just be two 97s before the func
  could also have a 97 preceded by the size at the end so we can traverse forward or backwards past meta data (or just don't interleave with function ops)
  can check for 97 as we render, set a global ptr to local meta data, and use that when rendering
  actually, to make it remain nice to program in and nice to specify meta data in, just do 97 size first_slot {comma separated names} and make the self-hosted compiler translate the ascii (or maybe just skip)
  can build up the indices list as we parse/render
  can make this in a comment that the hrmc self-compiler recognizes
#@ [names list] for naming hrmc table
#@0F [names list] for naming the things pointed to by that hrmc slot
  find something that works in C/Python/JS
would like to not be bound by global slots for functions and global variables and still be able to access them in 1 op
  maybe 8x ops?
  8Fzz could be E600 EEzz D80F
  but how do we enscribe a func?
  would rather not do more in machine code
  maybe we have a function for moving the addr from hrmc table to func list that we call at compile time and just have a new mechanism for calling funcs at compile time
get_kernel32 should handle small names
  check j <= i or do something in condition of loop to handle that

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

//// start common.h ////
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

u32 index_of_char(char* s, i32 i, char* chars, u32 instance, i32 dir) {
	u32 last_index = i;
	u32 cur_instance = 0;
	while((i >= 0 || dir > 0) && s[i]) {
		for(u32 j = 0; j < strlen(chars); j += 1) {
			if(chars[j] == 'A') {
				if(!isalpha(s[i]) && s[i] != '_') continue;
			}
			else if(chars[j] == 'N') {
				if(isspace(s[i])) continue;
			}
			else if(s[i] != chars[j]) continue;
			cur_instance += 1;
			if(instance == cur_instance)
				return i;
			last_index = i;
		}
		i += dir;
	}
	if(i < 0) return i;
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
	i32 i;
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
	if(del_len > i) del_len = i;
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
	u64 wParam;
	u64 lParam;
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
#define WM_KEYDOWN 0x0100
#define WM_KEYUP 0x0101
#define WM_SYSKEYDOWN 0x0104
#define WM_SYSKEYUP 0x0105
#define PM_REMOVE 0x0001

#define WM_MOUSEFIRST 0x0200
#define WM_MOUSEMOVE 0x0200
#define WM_LBUTTONDOWN 0x0201
#define WM_LBUTTONUP 0x0202
#define WM_LBUTTONDBLCLK 0x0203
#define WM_RBUTTONDOWN 0x0204
#define WM_RBUTTONUP 0x0205
#define WM_RBUTTONDBLCLK 0x0206
#define WM_MBUTTONDOWN 0x0207
#define WM_MBUTTONUP 0x0208
#define WM_MBUTTONDBLCLK 0x0209
#define WM_MOUSEWHEEL 0x020A
#define WM_XBUTTONDOWN 0x020B
#define WM_XBUTTONUP 0x020C
#define WM_XBUTTONDBLCLK 0x020D
#if _WIN32_WINNT >= 0x0600
#define WM_MOUSEHWHEEL 0x020e
#endif
#define WM_MOUSEHOVER 0x02A1
#define WM_MOUSELEAVE 0x02A3
#define WM_NCMOUSEHOVER 0x02A0
#define WM_NCMOUSELEAVE 0x02A2

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

// mouse xbuttons
#define XBUTTON1 0x0001
#define XBUTTON2 0x0002

// virtual keycodes
#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_CANCEL 0x03
#define VK_MBUTTON 0x04
#define VK_XBUTTON1 0x05
#define VK_XBUTTON2 0x06
#define VK_BACK 0x08
#define VK_TAB 0x09
#define VK_CLEAR 0x0C
#define VK_RETURN 0x0D
#define VK_SHIFT 0x10
#define VK_CONTROL 0x11
#define VK_MENU 0x12
#define VK_PAUSE 0x13
#define VK_CAPITAL 0x14
#define VK_KANA 0x15
#define VK_HANGEUL 0x15
#define VK_HANGUL 0x15
#define VK_IME_ON 0x16
#define VK_JUNJA 0x17
#define VK_FINAL 0x18
#define VK_HANJA 0x19
#define VK_KANJI 0x19
#define VK_IME_OFF 0x1A
#define VK_ESCAPE 0x1B
#define VK_CONVERT 0x1C
#define VK_NONCONVERT 0x1D
#define VK_ACCEPT 0x1E
#define VK_MODECHANGE 0x1F
#define VK_SPACE 0x20
#define VK_PRIOR 0x21
#define VK_NEXT 0x22
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_UP 0x26
#define VK_RIGHT 0x27
#define VK_DOWN 0x28
#define VK_SELECT 0x29
#define VK_PRINT 0x2A
#define VK_EXECUTE 0x2B
#define VK_SNAPSHOT 0x2C
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_HELP 0x2F

#define VK_LWIN 0x5B
#define VK_RWIN 0x5C
#define VK_APPS 0x5D
#define VK_SLEEP 0x5F
#define VK_NUMPAD0 0x60
#define VK_NUMPAD1 0x61
#define VK_NUMPAD2 0x62
#define VK_NUMPAD3 0x63
#define VK_NUMPAD4 0x64
#define VK_NUMPAD5 0x65
#define VK_NUMPAD6 0x66
#define VK_NUMPAD7 0x67
#define VK_NUMPAD8 0x68
#define VK_NUMPAD9 0x69
#define VK_MULTIPLY 0x6A
#define VK_ADD 0x6B
#define VK_SEPARATOR 0x6C
#define VK_SUBTRACT 0x6D
#define VK_DECIMAL 0x6E
#define VK_DIVIDE 0x6F
#define VK_F1 0x70
#define VK_F2 0x71
#define VK_F3 0x72
#define VK_F4 0x73
#define VK_F5 0x74
#define VK_F6 0x75
#define VK_F7 0x76
#define VK_F8 0x77
#define VK_F9 0x78
#define VK_F10 0x79
#define VK_F11 0x7A
#define VK_F12 0x7B
#define VK_F13 0x7C
#define VK_F14 0x7D
#define VK_F15 0x7E
#define VK_F16 0x7F
#define VK_F17 0x80
#define VK_F18 0x81
#define VK_F19 0x82
#define VK_F20 0x83
#define VK_F21 0x84
#define VK_F22 0x85
#define VK_F23 0x86
#define VK_F24 0x87
#if _WIN32_WINNT >= 0x0604
#define VK_NAVIGATION_VIEW 0x88
#define VK_NAVIGATION_MENU 0x89
#define VK_NAVIGATION_UP 0x8A
#define VK_NAVIGATION_DOWN 0x8B
#define VK_NAVIGATION_LEFT 0x8C
#define VK_NAVIGATION_RIGHT 0x8D
#define VK_NAVIGATION_ACCEPT 0x8E
#define VK_NAVIGATION_CANCEL 0x8F
#endif /* _WIN32_WINNT >= 0x0604 */
#define VK_NUMLOCK 0x90
#define VK_SCROLL 0x91
#define VK_OEM_NEC_EQUAL 0x92
#define VK_OEM_FJ_JISHO 0x92
#define VK_OEM_FJ_MASSHOU 0x93
#define VK_OEM_FJ_TOUROKU 0x94
#define VK_OEM_FJ_LOYA 0x95
#define VK_OEM_FJ_ROYA 0x96
#define VK_LSHIFT 0xA0
#define VK_RSHIFT 0xA1
#define VK_LCONTROL 0xA2
#define VK_RCONTROL 0xA3
#define VK_LMENU 0xA4
#define VK_RMENU 0xA5
#define VK_BROWSER_BACK 0xA6
#define VK_BROWSER_FORWARD 0xA7
#define VK_BROWSER_REFRESH 0xA8
#define VK_BROWSER_STOP 0xA9
#define VK_BROWSER_SEARCH 0xAA
#define VK_BROWSER_FAVORITES 0xAB
#define VK_BROWSER_HOME 0xAC
#define VK_VOLUME_MUTE 0xAD
#define VK_VOLUME_DOWN 0xAE
#define VK_VOLUME_UP 0xAF
#define VK_MEDIA_NEXT_TRACK 0xB0
#define VK_MEDIA_PREV_TRACK 0xB1
#define VK_MEDIA_STOP 0xB2
#define VK_MEDIA_PLAY_PAUSE 0xB3
#define VK_LAUNCH_MAIL 0xB4
#define VK_LAUNCH_MEDIA_SELECT 0xB5
#define VK_LAUNCH_APP1 0xB6
#define VK_LAUNCH_APP2 0xB7
#define VK_OEM_1 0xBA
#define VK_OEM_PLUS 0xBB
#define VK_OEM_COMMA 0xBC
#define VK_OEM_MINUS 0xBD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_2 0xBF
#define VK_OEM_3 0xC0
#if _WIN32_WINNT >= 0x0604
#define VK_GAMEPAD_A 0xC3
#define VK_GAMEPAD_B 0xC4
#define VK_GAMEPAD_X 0xC5
#define VK_GAMEPAD_Y 0xC6
#define VK_GAMEPAD_RIGHT_SHOULDER 0xC7
#define VK_GAMEPAD_LEFT_SHOULDER 0xC8
#define VK_GAMEPAD_LEFT_TRIGGER 0xC9
#define VK_GAMEPAD_RIGHT_TRIGGER 0xCA
#define VK_GAMEPAD_DPAD_UP 0xCB
#define VK_GAMEPAD_DPAD_DOWN 0xCC
#define VK_GAMEPAD_DPAD_LEFT 0xCD
#define VK_GAMEPAD_DPAD_RIGHT 0xCE
#define VK_GAMEPAD_MENU 0xCF
#define VK_GAMEPAD_VIEW 0xD0
#define VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON 0xD1
#define VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON 0xD2
#define VK_GAMEPAD_LEFT_THUMBSTICK_UP 0xD3
#define VK_GAMEPAD_LEFT_THUMBSTICK_DOWN 0xD4
#define VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT 0xD5
#define VK_GAMEPAD_LEFT_THUMBSTICK_LEFT 0xD6
#define VK_GAMEPAD_RIGHT_THUMBSTICK_UP 0xD7
#define VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN 0xD8
#define VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT 0xD9
#define VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT 0xDA
#endif /* _WIN32_WINNT >= 0x0604 */
#define VK_OEM_4 0xDB
#define VK_OEM_5 0xDC
#define VK_OEM_6 0xDD
#define VK_OEM_7 0xDE
#define VK_OEM_8 0xDF
#define VK_OEM_AX 0xE1
#define VK_OEM_102 0xE2
#define VK_ICO_HELP 0xE3
#define VK_ICO_00 0xE4
#define VK_PROCESSKEY 0xE5
#define VK_ICO_CLEAR 0xE6
#define VK_PACKET 0xE7
#define VK_OEM_RESET 0xE9
#define VK_OEM_JUMP 0xEA
#define VK_OEM_PA1 0xEB
#define VK_OEM_PA2 0xEC
#define VK_OEM_PA3 0xED
#define VK_OEM_WSCTRL 0xEE
#define VK_OEM_CUSEL 0xEF
#define VK_OEM_ATTN 0xF0
#define VK_OEM_FINISH 0xF1
#define VK_OEM_COPY 0xF2
#define VK_OEM_AUTO 0xF3
#define VK_OEM_ENLW 0xF4
#define VK_OEM_BACKTAB 0xF5
#define VK_ATTN 0xF6
#define VK_CRSEL 0xF7
#define VK_EXSEL 0xF8
#define VK_EREOF 0xF9
#define VK_PLAY 0xFA
#define VK_ZOOM 0xFB
#define VK_NONAME 0xFC
#define VK_PA1 0xFD
#define VK_OEM_CLEAR 0xFE

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
u64   (*DefWindowProcA)       (void* hWnd, u32 Msg, u64 wParam, u64 lParam);
void  (*PostQuitMessage)      (int nExitCode);
i32   (*PeekMessageA)         (MSG* lpMsg,void* hWnd,u32 wMsgFilterMin,u32 wMsgFilterMax,u32 wRemoveMsg);
i32   (*TranslateMessage)     (MSG *lpMsg);
u64   (*DispatchMessageA)     (MSG *lpMsg);
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

void link_win() {
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
}

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
	if(!ReadFile(fin, in_buffer, in_size, &bytes_read, 0))
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
#define KEY_MODIFIER_SHIFT (1 << 0)
#define KEY_MODIFIER_CTRL  (1 << 1)
#define KEY_MODIFIER_ALT   (1 << 2)
#define KEY_MODIFIER_FN    (1 << 3)
#define KEY_MODIFIER_SUPER (1 << 4)

arena text_arena;
u8 modifiers_held = 0;
i32 cursor_xoff = 1;
u8 prev_char = 0;

u64 Win32EventHandler(void* window, u32 msg, u64 wp, u64 lp) {
	if(msg == WM_DESTROY || msg == WM_CLOSE)
		ExitProcess(0);
	u64 keycode = wp;
	u64 mousecode = (wp >> 16) & 0xFFFF;
	//u64 was_down = lp & (1 << 30);
	//u64 is_down  = lp & (1 << 31);
	u64 alt_was_down  = lp & (1 << 29);
	if(alt_was_down) modifiers_held |= KEY_MODIFIER_ALT;
	else             modifiers_held &= ~KEY_MODIFIER_ALT;
	if(msg == WM_SYSKEYDOWN || msg == WM_KEYDOWN) {
		prev_char = keycode;
		u8 character = 0;
		i32 new_i = text_arena.i;
		i32 prev_i = new_i;
		i32 del = 0;
		i32 preserve_cursor_xoff = 0;
		arena_gap_splice(&text_arena, text_arena.len, 0, &character, 0); // make buffer contiguous
		i32 prev_newline      = index_of_char(text_arena.mem, new_i - 1,        "\n",    1, -1);
		i32 prev_prev_newline = index_of_char(text_arena.mem, prev_newline - 1, "\n",    1, -1);
		i32 next_newline      = index_of_char(text_arena.mem, new_i,            "\n",    1,  1);
		i32 next_next_newline = index_of_char(text_arena.mem, next_newline + 1, "\n",    1,  1);
		i32 prev_word_end     = index_of_char(text_arena.mem, new_i - 1,        "N",     1, -1);
		i32 prev_word_start   = index_of_char(text_arena.mem, prev_word_end,    " \t\n", 1, -1) + 1;
		i32 cur_word_end      = index_of_char(text_arena.mem, new_i,            " \t\n", 1,  1);
		i32 next_word_start   = index_of_char(text_arena.mem, cur_word_end,     "N",     1,  1);
		i32 next_word_end     = index_of_char(text_arena.mem, next_word_start,  " \t\n", 1,  1) - 1;
		i32 line_up   = prev_prev_newline + MAX(1, MIN(cursor_xoff, prev_newline - prev_prev_newline));
		i32 line_down = next_newline      + MAX(1, MIN(cursor_xoff, next_next_newline - next_newline));
		i32 empty_line_up = index_of_char(text_arena.mem, new_i - 1, "A", 1, -1);
		i32 empty_line_down = index_of_char(text_arena.mem, new_i + 1, "A", 1, 1);
		for(; empty_line_up > 0; empty_line_up--) // TODO: remember to remove \r from text_arena.mem
			if(text_arena.mem[empty_line_up] == '\n' && text_arena.mem[empty_line_up - 1] == '\n')
				break;
		for(; empty_line_down < text_arena.len; empty_line_down++)
			if(text_arena.mem[empty_line_down] == '\n' && text_arena.mem[empty_line_down - 1] == '\n')
				break;
		if(keycode == VK_SHIFT)   modifiers_held |= KEY_MODIFIER_SHIFT;
		if(keycode == VK_CONTROL) modifiers_held |= KEY_MODIFIER_CTRL;

		// insertion
		if(!modifiers_held) {
			     if(keycode == VK_RETURN)             character = '\n';
			else if(keycode == VK_TAB)                character = '\t';
			else if(keycode == VK_OEM_1)              character = ';';
			else if(keycode == VK_OEM_PLUS)           character = '=';
			else if(keycode == VK_OEM_COMMA)          character = ',';
			else if(keycode == VK_OEM_MINUS)          character = '-';
			else if(keycode == VK_OEM_PERIOD)         character = '.';
			else if(keycode == VK_OEM_2)              character = '/';
			else if(keycode == VK_OEM_3)              character = '`';
			else if(keycode == VK_OEM_4)              character = '[';
			else if(keycode == VK_OEM_5)              character = '\\';
			else if(keycode == VK_OEM_6)              character = ']';
			else if(keycode == VK_OEM_7)              character = '\'';
			else if(keycode == ' ')                   character = keycode;
			else if(isdigit(keycode))                 character = keycode;
			else if(keycode >= 'A' && keycode <= 'Z') character = keycode + 'a' - 'A';
		}
		if(modifiers_held == KEY_MODIFIER_SHIFT) {
			     if(keycode >= 'A' && keycode <= 'Z') character = keycode;
			else if(keycode == VK_OEM_1)              character = ':';
			else if(keycode == VK_OEM_PLUS)           character = '+';
			else if(keycode == VK_OEM_COMMA)          character = '<';
			else if(keycode == VK_OEM_MINUS)          character = '_';
			else if(keycode == VK_OEM_PERIOD)         character = '>';
			else if(keycode == VK_OEM_2)              character = '?';
			else if(keycode == VK_OEM_3)              character = '~';
			else if(keycode == VK_OEM_4)              character = '{';
			else if(keycode == VK_OEM_5)              character = '|';
			else if(keycode == VK_OEM_6)              character = '}';
			else if(keycode == VK_OEM_7)              character = '\"';
			else if(keycode == '1')                   character = '!';
			else if(keycode == '2')                   character = '@';
			else if(keycode == '3')                   character = '#';
			else if(keycode == '4')                   character = '$';
			else if(keycode == '5')                   character = '%';
			else if(keycode == '6')                   character = '^';
			else if(keycode == '7')                   character = '&';
			else if(keycode == '8')                   character = '*';
			else if(keycode == '9')                   character = '(';
			else if(keycode == '0')                   character = ')';
		}

		// deletion
		if(!modifiers_held) {
			     if(keycode == VK_BACK)   { del = 1; }
			else if(keycode == VK_DELETE) { del = 1; new_i += del; }
		}
		if(modifiers_held == KEY_MODIFIER_CTRL) {
			     if(keycode == VK_BACK)   { del = new_i - prev_word_start; }
			else if(keycode == VK_DELETE) { del = next_word_start - new_i; new_i += del; }
		}
		if(modifiers_held == (KEY_MODIFIER_CTRL | KEY_MODIFIER_SHIFT)) {
			     if(keycode == VK_BACK)   { del = new_i - prev_newline - 1; }
			else if(keycode == VK_DELETE) { del = next_newline - new_i; new_i += del; }
		}
		if(modifiers_held == KEY_MODIFIER_SHIFT) {
			     if(keycode == VK_TAB)    {  } // remove tab at start of line if present, if selection, remove from start of selected lines
		}

		// movement
		if(!(modifiers_held & KEY_MODIFIER_CTRL)) {
			     if(keycode == VK_LEFT)   { new_i -= 1; }
			else if(keycode == VK_RIGHT)  { new_i += 1; }
			else if(keycode == VK_UP)     { new_i = line_up; preserve_cursor_xoff = 1; }
			else if(keycode == VK_DOWN)   { new_i = line_down; preserve_cursor_xoff = 1; }
			else if(keycode == VK_HOME)   { new_i = prev_newline + 1; }
			else if(keycode == VK_END)    { new_i = next_newline; cursor_xoff = 99999; preserve_cursor_xoff = 1; }
		}
		if(modifiers_held & KEY_MODIFIER_CTRL) {
			     if(keycode == VK_LEFT)   { new_i = prev_word_start; }
			else if(keycode == VK_RIGHT)  { new_i = next_word_start; }
			else if(keycode == VK_UP)     { new_i = empty_line_up; }
			else if(keycode == VK_DOWN)   { new_i = empty_line_down; }
			else if(keycode == VK_HOME)   { new_i = 0; }
			else if(keycode == VK_END)    { new_i = text_arena.len; cursor_xoff = 99999; preserve_cursor_xoff = 1; }
		}
		if(modifiers_held == KEY_MODIFIER_ALT) {
			     if(keycode == 'H')       { new_i -= 1; }
			else if(keycode == 'L')       { new_i += 1; }
			else if(keycode == 'K')       { new_i = line_up; preserve_cursor_xoff = 1; }
			else if(keycode == 'J')       { new_i = line_down; preserve_cursor_xoff = 1; }
			else if(keycode == 'B')       { new_i = prev_word_start; }
			else if(keycode == 'N')       { new_i = next_word_start; }
		}
		if((modifiers_held & KEY_MODIFIER_ALT) && (modifiers_held & KEY_MODIFIER_SHIFT)) {
			     if(keycode == VK_OEM_4)  { new_i = empty_line_up; }
			else if(keycode == VK_OEM_6)  { new_i = empty_line_down; }
			else if(keycode == 'H')       { new_i = prev_word_start; }
			else if(keycode == 'L')       { new_i = next_word_start; }
			else if(keycode == 'K')       { new_i = empty_line_up; }
			else if(keycode == 'J')       { new_i = empty_line_down; }
		}
		// TODO: rename the OEM stuff with a custom define, ideally what the ascii value is if avail
		// if new_i != prev_i and command != 0 then do command with that motion

		if(new_i < 0) { new_i = 0; preserve_cursor_xoff = 1; } // if going up on first line, preserve xoff
		arena_gap_splice(&text_arena, new_i, del, &character, !!character);
		if(!preserve_cursor_xoff) cursor_xoff = text_arena.i - index_of_char(text_arena.mem, text_arena.i - 1, "\n", 1, -1);
	}
	if(msg == WM_SYSKEYUP || msg == WM_KEYUP) {
		if(keycode == VK_SHIFT)   modifiers_held &= ~KEY_MODIFIER_SHIFT;
		if(keycode == VK_CONTROL) modifiers_held &= ~KEY_MODIFIER_CTRL;
	}
	if(msg == WM_LBUTTONDOWN) {
		console_log("lmb\n", 0);
	}
	if(msg == WM_RBUTTONDOWN) {
		console_log("rmb\n", 0);
	}
	if(msg == WM_MBUTTONDOWN) {
		console_log("mmb\n", 0);
	}
	if(msg == WM_XBUTTONDOWN) {
		if(mousecode == XBUTTON1) console_log("bmb\n", 0);
		if(mousecode == XBUTTON2) console_log("fmb\n", 0);
	}
	return DefWindowProcA(window, msg, wp, lp);
}

void _start() {
	link_win();

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
	text_arena.cap = 5*1024*1024;
	text_arena.len = text_arena.i = 0;
	text_arena.mem = mem_alloc(text_arena.cap);
	text_arena.len = text_arena.i = file_read("hrmc.c", text_arena.mem, text_arena.cap);
	arena_gap_splice(&text_arena, 0, 0, 0, 0);

	char str[] = "the quick brown fox jumped over the lazy dog `1234567890-=~!@#$%^&*()_+";
	while(1) {
		// handle events
		MSG msg;
		while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		tick += 1;

		// TODO: pageup pagedown, shift selection, copy paste, undo redo, etc
		// TODO: audio
		// TODO: load htmc.dmp from file
		// parse name comments
		// show hrmc as either hex or name-aliased code
		// be able to write in name-aliased view and have it translated to hex
		// live reload
		// custom views
		// view-making toolkit

		u32* pixel = pixel_data;
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				//*pixel++ = (((x+tick*2)%255) << 8) | ((y+tick)%255);
				*pixel++ = 0;
			}
		}

		//char digits[16] = {0};
		//char* yo = int2str(cursor_xoff, digits);
		//memcpy(text_arena.mem, yo, strlen(yo));

		int yoff = 13;
		int xoff = 0;
		int cursor_xoff = xoff;
		int cursor_yoff = yoff;
		int xstride = 9;
		int ystride = 15;
		for(int i = 0; i < text_arena.len; i++) {
			u8 character = text_arena.mem[i];
			if(i >= text_arena.i) character = text_arena.mem[i + (text_arena.cap - text_arena.len)];
			if(i == text_arena.i) {
				cursor_xoff = xoff;
				cursor_yoff = yoff;
			}
			if(character == '\n') {
				yoff += ystride;
				xoff = 0;
				continue;
			}
			if(character == '\t') {
				xoff += xstride * 1;
				continue;
			}
			u8* raster = rasters[character - 0x20];
			for(int y = 0; y < 13; y++) {
				if((13-y)+yoff >= monitor_rect.bottom) continue;
				for(int x = 0; x < xstride; x++) {
					if((8-x)+xoff >= monitor_rect.right) continue;
					if(raster[y] & (1 << x)) pixel_data[((13-y)+yoff)*width+((8-x)+xoff)] = -1;
					else                     pixel_data[((13-y)+yoff)*width+((8-x)+xoff)] = 0;
				}
			}
			if(yoff >= monitor_rect.bottom) break;
			xoff += xstride;
		}
		if(text_arena.i == text_arena.len) {
			cursor_xoff = xoff;
			cursor_yoff = yoff;
		}
		for(int y = 0; y < 13; y++) {
			pixel_data[((13-y)+cursor_yoff)*width+((8)+cursor_xoff)] = -1;
			pixel_data[((13-y)+cursor_yoff)*width+((0)+cursor_xoff)] = -1;
		}
		for(int x = 0; x < xstride; x++) {
			pixel_data[((13)+cursor_yoff)*width+((8-x)+cursor_xoff)] = -1;
			pixel_data[((0)+cursor_yoff)*width+((8-x)+cursor_xoff)] = -1;
		}

		// draw pixel data to screen
		void* dc = GetDC(window);
		StretchDIBits(dc, 0, 0, width, height, 0, 0, width, height, pixel_data, &bmih, DIB_RGB_COLORS, SRCCOPY);
		ReleaseDC(window, dc);
	}
	ExitProcess(0);
}

