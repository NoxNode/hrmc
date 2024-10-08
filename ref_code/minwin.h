#ifndef _MINWIN_H_
#define _MINWIN_H_
////// types.h //////

//// CRT declaration replacements
#define NULL  ((void*)0)
#define __CRT_INLINE static __inline__

//typedef int     _Bool;
#define bool	_Bool
#define true	1
#define false	0
#define __bool_true_false_are_defined 1

typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef short              int16_t;
typedef unsigned short     uint16_t;
typedef int                int32_t;
typedef unsigned           uint32_t;
typedef long long          int64_t;
typedef unsigned long long uint64_t;

// size_t is compiler and 32-bit vs 64-bit specific
#if defined(__GNUC__) || defined(__TINYC__)
typedef __SIZE_TYPE__      size_t;
#elif defined( _WIN64)
typedef unsigned __int64   size_t;
#elif defined(_WIN32)
typedef _W64 unsigned int  size_t;
#endif

// custom types
typedef int8_t   s8;
typedef uint8_t  u8;
typedef int16_t  s16;
typedef uint16_t u16;
typedef int32_t  s32;
typedef uint32_t u32;
typedef int64_t  s64;
typedef uint64_t u64;
typedef float    r32;
typedef double   r64;

int CompareMemory(void *p1, void *p2, size_t n) {
	u8* b1 = (u8*)p1;
	u8* b2 = (u8*)p2;
	while (n)
	{
		u8 c1 = *b1;
		u8 c2 = *b2;
		c1 = c1 >= 'A' && c1 <= 'Z' ? c1 - 'A' + 'a' : c1;
		c2 = c2 >= 'A' && c2 <= 'Z' ? c2 - 'A' + 'a' : c2;
		if (c1 != c2) return c1 - c2;
		n--;
		b1++;
		b2++;
	}
	return n ? *b1 - *b2 : 0;
}
int CompareStrings(char *s1, char *s2) {
	while (*s1 && *s2 && *s1 == *s2) {
		s1++;
		s2++;
	}
	return (*s1 == *s2) ? 0 : *s1 - *s2;
}

////// types.h //////
////// crt.h //////
// CRT definition replacements
#if defined(_WIN32)
int _fltused;
#endif
#pragma function(memset)
void* memset(void* dest, int val, size_t count) {
	char* bytes = (char*)dest;
	while(count--) {
		*bytes++ = (char)val;
	}
	return dest;
}
#if defined(__GNUC__) || defined(__TINYC__)
// IEEE single-precision floating point
#define EXCESS		126
#define SIGNBIT		0x80000000
#define HIDDEN		(1 << 23)
#define SIGN(fp)	((fp) & SIGNBIT)
#define EXP(fp)		(((fp) >> 23) & 0xFF)
#define MANT(fp)	(((fp) & 0x7FFFFF) | HIDDEN)

// IEEE double-precision floating point
#define EXCESSD		1022
#define EXPD(fp)	(((fp.l.upper) >> 20) & 0x7FF)
#define SIGND(fp)	((fp.l.upper) & SIGNBIT)
#define HIDDEND_LL	((long long)1 << 52)
#define MANTD_LL(fp)	((fp.ll & (HIDDEND_LL-1)) | HIDDEND_LL)

typedef int Wtype;
typedef long long DWtype;
struct DWstruct {
	Wtype low, high;
};
typedef union {
	struct DWstruct s;
	DWtype ll;
} DWunion;
typedef long double XFtype;
union double_long {
	double d;
	struct {
		unsigned int lower;
		int upper;
	} l;
	long long ll;
};
union float_long {
	float f;
	unsigned int l;
};
unsigned long long __fixunssfdi (float a1) {
	register union float_long fl1;
	register int exp;
	register unsigned long long l;

	fl1.f = a1;

	if (fl1.l == 0)
		return (0);

	exp = EXP (fl1.l) - EXCESS - 24;
	l = MANT(fl1.l);

	if (exp >= 41)
		return 1ULL << 63;
	else if (exp >= 0)
		l <<= exp;
	else if (exp >= -23)
		l >>= -exp;
	else
		return 0;
	if (SIGN(fl1.l))
		l = (unsigned long long)-l;
	return l;
}
unsigned long long __fixunsdfdi (double a1) {
	register union double_long dl1;
	register int exp;
	register unsigned long long l;

	dl1.d = a1;

	if (dl1.ll == 0)
		return (0);

	exp = EXPD (dl1) - EXCESSD - 53;
	l = MANTD_LL(dl1);

	if (exp >= 12)
		return 1ULL << 63; /* overflow result (like gcc, somewhat) */
	else if (exp >= 0)
		l <<= exp;
	else if (exp >= -52)
		l >>= -exp;
	else
		return 0;
	if (SIGND(dl1))
		l = (unsigned long long)-l;
	return l;
}

float __floatundisf(unsigned long long a) {
	DWunion uu; 
	XFtype r;

	uu.ll = a;
	if (uu.s.high >= 0) {
		return (float)uu.ll;
	} else {
		r = (XFtype)uu.ll;
		r += 18446744073709551616.0;
		return (float)r;
	}
}

double __floatundidf(unsigned long long a) {
	DWunion uu; 
	XFtype r;

	uu.ll = a;
	if (uu.s.high >= 0) {
		return (double)uu.ll;
	} else {
		r = (XFtype)uu.ll;
		r += 18446744073709551616.0;
		return (double)r;
	}
}
#if 1
void __chkstk() {}
#else
__asm {
	.globl _(__chkstk)
		_(__chkstk):
			xchg    (%rsp),%rbp     /* store ebp, get ret.addr */
			push    %rbp            /* push ret.addr */
			lea     8(%rsp),%rbp    /* setup frame ptr */
			push    %rcx            /* save ecx */
			mov     %rbp,%rcx
			movslq  %eax,%rax
			P0:
			sub     $4096,%rcx
			test    %rax,(%rcx)
			sub     $4096,%rax
			cmp     $4096,%rax
			jge     P0
			sub     %rax,%rcx
			test    %rax,(%rcx)

			mov     %rsp,%rax
				mov     %rcx,%rsp
				mov     (%rax),%rcx     /* restore ecx */
				jmp     *8(%rax)
}
#endif
#else //#elif defined(_WIN32)
//__declspec(naked) void _ftol2() {
//	__asm {
//		fistp qword ptr [esp-8]
//			mov   edx,[esp-4]
//			mov   eax,[esp-8]
//			ret
//	}
//}
//
//__declspec(naked) void _ftol2_sse() {
//	__asm {
//		fistp dword ptr [esp-4]
//			mov   eax,[esp-4]
//			ret
//	}
//}
#endif
////// crt.h //////
////// minwin.h //////
#define __declspec
#define __stdcall
#define DECLSPEC_NORETURN __declspec (noreturn)
#define CALLBACK __stdcall
#define WINAPI __stdcall
#define WINBASEAPI
#define WINUSERAPI
#define WINGDIAPI
#define APIENTRY
#define CONST const

#define FALSE 0
#define TRUE  1

typedef char  CHAR;
typedef CHAR* LPSTR;
typedef CONST CHAR* LPCSTR;
typedef u16   wchar_t;
typedef wchar_t WCHAR;
typedef WCHAR *NWPSTR,*LPWSTR,*PWSTR;

typedef s32   WINBOOL;
typedef u8    BYTE;
typedef u16   ATOM;
typedef u16   WORD;
typedef u16   USHORT;
typedef u32   UINT;
typedef u32   DWORD;
typedef u64   DWORD64;
typedef s32   LONG;
typedef u64*  LRESULT;
typedef u64*  LPARAM;
typedef u32*  WPARAM;

typedef void  VOID;
typedef void* PVOID;
typedef void* LPVOID;
typedef void* PROC;
typedef s32*  (WINAPI *FARPROC) ();

// H stands for handle aka a pointer
typedef void* HANDLE;
typedef void* HWND;
typedef void* HMENU;
typedef void* HINSTANCE;
typedef void* HICON;
typedef void* HCURSOR;
typedef void* HBRUSH;
typedef void* HDC;
typedef void* HGLRC;
typedef void* HMODULE;

#define WM_MOVE 0x0003
#define WM_SIZE 0x0005
#define WM_PAINT 0x000F
#define WM_CLOSE 0x0010
#define WM_DESTROY 0x0002
#define WM_QUIT 0x0012

#define __MSABI_LONG(x) x ## l
#define WS_OVERLAPPED __MSABI_LONG(0x00000000)
#define WS_CAPTION __MSABI_LONG(0x00C00000)
#define WS_SYSMENU __MSABI_LONG(0x00080000)
#define WS_THICKFRAME __MSABI_LONG(0x00040000)
#define WS_MINIMIZEBOX __MSABI_LONG(0x00020000)
#define WS_MAXIMIZEBOX __MSABI_LONG(0x00010000)
#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX)
#define WS_VISIBLE __MSABI_LONG(0x10000000)

#define CW_USEDEFAULT ((int)0x80000000)

#define PM_REMOVE 0x0001

#define PFD_TYPE_RGBA 0
#define PFD_MAIN_PLANE 0
#define PFD_DOUBLEBUFFER 0x00000001
#define PFD_DRAW_TO_WINDOW 0x00000004
#define PFD_SUPPORT_OPENGL 0x00000020

typedef LRESULT (CALLBACK *WNDPROC)(HWND,UINT,WPARAM,LPARAM);

typedef struct tagWNDCLASSA {
	UINT style;
	WNDPROC lpfnWndProc;
	int cbClsExtra;
	int cbWndExtra;
	HINSTANCE hInstance;
	HICON hIcon;
	HCURSOR hCursor;
	HBRUSH hbrBackground;
	LPCSTR lpszMenuName;
	LPCSTR lpszClassName;
} WNDCLASSA,*PWNDCLASSA,*NPWNDCLASSA,*LPWNDCLASSA;

typedef struct tagPOINT {
	LONG x;
	LONG y;
} POINT, *PPOINT, *NPPOINT, *LPPOINT;

typedef struct tagRECT {
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT, *PRECT, *NPRECT, *LPRECT;

typedef struct tagMSG {
	HWND hwnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	DWORD time;
	POINT pt;
} MSG,*PMSG,*NPMSG,*LPMSG;

typedef struct tagPIXELFORMATDESCRIPTOR {
	WORD nSize;
	WORD nVersion;
	DWORD dwFlags;
	BYTE iPixelType;
	BYTE cColorBits;
	BYTE cRedBits;
	BYTE cRedShift;
	BYTE cGreenBits;
	BYTE cGreenShift;
	BYTE cBlueBits;
	BYTE cBlueShift;
	BYTE cAlphaBits;
	BYTE cAlphaShift;
	BYTE cAccumBits;
	BYTE cAccumRedBits;
	BYTE cAccumGreenBits;
	BYTE cAccumBlueBits;
	BYTE cAccumAlphaBits;
	BYTE cDepthBits;
	BYTE cStencilBits;
	BYTE cAuxBuffers;
	BYTE iLayerType;
	BYTE bReserved;
	DWORD dwLayerMask;
	DWORD dwVisibleMask;
	DWORD dwDamageMask;
} PIXELFORMATDESCRIPTOR,*PPIXELFORMATDESCRIPTOR,*LPPIXELFORMATDESCRIPTOR;

// define these first because we manually load these to use to load the rest
typedef FARPROC GetProcAddress_t (HMODULE hModule, LPCSTR lpProcName);
static GetProcAddress_t* GetProcAddress;
typedef HMODULE LoadLibraryA_t (LPCSTR lpLibFileName);
static LoadLibraryA_t* LoadLibraryA;

#define DLLFUNC(dll, returnType, name, params) \
    typedef returnType name##_t params; \
    static name##_t* name;

#define WIN32_DLL_FUNCS \
DLLFUNC(kernel32, VOID WINAPI,    ExitProcess,       (UINT uExitCode)) \
DLLFUNC(kernel32, WINBOOL WINAPI, FreeLibrary,       (HMODULE hLibModule)) \
DLLFUNC(kernel32, HMODULE WINAPI, GetModuleHandleA,  (LPCSTR lpModuleName)) \
DLLFUNC(user32,   ATOM WINAPI,    RegisterClassA,    (CONST WNDCLASSA *lpWndClass)) \
DLLFUNC(user32,   HWND WINAPI,    CreateWindowExA,   (DWORD,LPCSTR,LPCSTR,DWORD,int,int,int,int,HWND,HMENU,HINSTANCE,LPVOID)) \
DLLFUNC(user32,   LRESULT WINAPI, DefWindowProcA,    (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)) \
DLLFUNC(user32,   VOID WINAPI,    PostQuitMessage,   (int nExitCode)) \
DLLFUNC(user32,   WINBOOL WINAPI, PeekMessageA,      (LPMSG lpMsg,HWND hWnd,UINT wMsgFilterMin,UINT wMsgFilterMax,UINT wRemoveMsg)) \
DLLFUNC(user32,   WINBOOL WINAPI, TranslateMessage,  (CONST MSG *lpMsg)) \
DLLFUNC(user32,   LRESULT WINAPI, DispatchMessageA,  (CONST MSG *lpMsg)) \
DLLFUNC(user32,   HDC WINAPI,     GetDC,             (HWND hWnd)) \
DLLFUNC(user32,   WINBOOL WINAPI, GetClientRect,     (HWND hWnd,LPRECT lpRect)) \
DLLFUNC(user32,   int WINAPI,     ReleaseDC,         (HWND hWnd,HDC hDC)) \
DLLFUNC(gdi32,    int WINAPI,     ChoosePixelFormat, (HDC hdc,CONST PIXELFORMATDESCRIPTOR *ppfd)) \
DLLFUNC(gdi32,    WINBOOL WINAPI, SetPixelFormat,    (HDC hdc,int format,CONST PIXELFORMATDESCRIPTOR *ppfd)) \
DLLFUNC(gdi32,    HGLRC WINAPI,   wglCreateContext,  (HDC)) \
DLLFUNC(gdi32,    WINBOOL WINAPI, wglDeleteContext,  (HGLRC)) \
DLLFUNC(gdi32,    PROC WINAPI,    wglGetProcAddress, (LPCSTR)) \
DLLFUNC(gdi32,    WINBOOL WINAPI, wglMakeCurrent,    (HDC,HGLRC)) \
DLLFUNC(gdi32,    WINBOOL WINAPI, SwapBuffers,       (HDC))

WIN32_DLL_FUNCS

#define PE_GET_OFFSET(module, offset) ((u8*)(module) + offset)

typedef struct _LIST_ENTRY {
	struct _LIST_ENTRY *Flink;
	struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY, PRLIST_ENTRY;

typedef struct win32_ldr_data_entry {
	LIST_ENTRY LinkedList;
	LIST_ENTRY UnusedList;
	PVOID BaseAddress;
	PVOID Reserved2[1];
	PVOID DllBase;
	PVOID EntryPoint;
	PVOID Reserved3;
	USHORT DllNameLength;
	USHORT DllNameMaximumLength;
	PWSTR  DllNameBuffer;
} win32_ldr_data_entry;

typedef struct win32_ldr_data {
	u8 Padding1[0x20];
	win32_ldr_data_entry *LoaderDataEntry;
} win32_ldr_data;

typedef struct win32_peb {
	u8 Padding1[0x18];
	win32_ldr_data *LoaderData;
} win32_peb;

typedef struct win32_teb {
	u8 Padding1[0x60];
	win32_peb *PEB;
} win32_teb;

typedef struct win32_msdos {
	u8 Padding1[0x3C];
	u32 PEOffset;
} win32_msdos;

typedef struct win32_pe_image_data {
	u32 VirtualAddress;
	u32 Size;
} win32_pe_image_data;

typedef struct win32_pe {
	u8 Signature[4 +2*2 +4*3 +2*3 +2 +4*5 +8 +4*2 +2*6 +4*4 +2*2 +8*4 +2*4];
	win32_pe_image_data ExportTable;
} win32_pe;

typedef struct win32_pe_export_table {
	u8 Padding[4*2 +2*2 +4];
	u32 OrdinalBase;
	u32 AddressTableEntries;
	u32 NumberofNamePointers;
	u32 ExportAddressTableRVA;
	u32 NamePointerRVA;
	u32 OrdinalTableRVA;
} win32_pe_export_table;

#ifndef _MSC_VER
u64 __readgsqword(u32 Offset) {
	void *ret;
	__asm__ volatile ("movq	%%gs:%1,%0"
			: "=r" (ret) ,"=m" ((*(volatile u64*) Offset)));
	return (u64) ret;
}
#endif // _MSC_VER

HMODULE GetKernel32Module() {
	wchar_t Kernel32Name[] = L"kernel32.dll";
	win32_teb* TEB = (win32_teb*)__readgsqword(0x30);
	win32_ldr_data_entry* LoaderDataEntry = TEB->PEB->LoaderData->LoaderDataEntry;

	while(LoaderDataEntry->DllBase) {
		size_t minSize = sizeof(Kernel32Name) < LoaderDataEntry->DllNameLength ?
			sizeof(Kernel32Name) : LoaderDataEntry->DllNameLength;
		if(CompareMemory(LoaderDataEntry->DllNameBuffer, Kernel32Name, minSize) == 0) {
			return (HMODULE)LoaderDataEntry->BaseAddress;
		}
		LoaderDataEntry = (win32_ldr_data_entry*)(LoaderDataEntry->LinkedList.Flink);
	}

	return NULL;
}
GetProcAddress_t* GetGetProcAddress(HMODULE Kernel32) {
	// Module is now in the EXE Format
	win32_msdos* MSDOSHeader = (win32_msdos*)PE_GET_OFFSET(Kernel32, 0);
	win32_pe* PEHeader = (win32_pe*)PE_GET_OFFSET(Kernel32, MSDOSHeader->PEOffset);
	win32_pe_export_table *ExportTable = (win32_pe_export_table*)PE_GET_OFFSET(Kernel32, PEHeader->ExportTable.VirtualAddress);
	u32* NamePointerTable = (u32*)PE_GET_OFFSET(Kernel32, ExportTable->NamePointerRVA);

	// binary search for GetProcAddress
	int Low = 0;
	int High = ExportTable->NumberofNamePointers - 1;
	int Index;
	char *ProcName;
	int CompareResult = 0;
	do {
		if(CompareResult > 0) {
			Low = Index;
		}
		else if(CompareResult < 0) {
			High = Index;
		}
		Index = (High + Low) / 2;
		ProcName = (char*)PE_GET_OFFSET(Kernel32, NamePointerTable[Index]);
	} while ((CompareResult = CompareStrings("GetProcAddress", ProcName)) != 0);

	// the same Index is used for the ordinal value
	u16* OrdinalTable = (u16*)PE_GET_OFFSET(Kernel32, ExportTable->OrdinalTableRVA);
	u16 GetProcAddressOrdinal = OrdinalTable[Index];

	u32* ExportAddressTable = (u32*)PE_GET_OFFSET(Kernel32, ExportTable->ExportAddressTableRVA);
	// The PE Documentation explicitly says you must subtract the OrdinalBase from the Ordinal to get the true
	// index into the address table, however, I found through testing that this is not the case.
	// This appears to confirm a problem with the documentation: http://stackoverflow.com/questions/5653316/pe-export-directory-tables-ordinalbase-field-ignored
	u32 GetProcAddressRVA = ExportAddressTable[GetProcAddressOrdinal];
	return (GetProcAddress_t*)PE_GET_OFFSET(Kernel32, GetProcAddressRVA);
}

void DynamicLink(void) {
	HMODULE kernel32 = GetKernel32Module();
	GetProcAddress = GetGetProcAddress(kernel32);
	LoadLibraryA =
		(LoadLibraryA_t*)GetProcAddress(kernel32, "LoadLibraryA");
	HMODULE user32 = LoadLibraryA("user32.dll");
	HMODULE gdi32 = LoadLibraryA("gdi32.dll");

#undef DLLFUNC
#define DLLFUNC(dll, returnType, name, params) \
    name = (name##_t*)GetProcAddress(dll, #name);

    WIN32_DLL_FUNCS
}

////// minwin.h //////
////// mingl.h //////
typedef s8    GLbyte;
typedef u8    GLubyte;
typedef u8    GLboolean;
typedef s16   GLshort;
typedef u16   GLushort;
typedef s32   GLint;
typedef s32   GLsizei;
typedef u32   GLuint;
typedef u32   GLenum;
typedef u32   GLbitfield;
typedef r32   GLclampf;
typedef r32   GLfloat;
typedef r64   GLclampd;
typedef r64   GLdouble;
typedef void  GLvoid;

#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406

#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100

#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_TRIANGLES 0x0004

#define GL_TRUE 1
#define GL_FALSE 0

#define GL_TEXTURE_2D 0x0DE1
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_CLAMP 0x2900
#define GL_REPEAT 0x2901

#define GL_VENDOR 0x1F00
#define GL_RENDERER 0x1F01
#define GL_VERSION 0x1F02
#define GL_EXTENSIONS 0x1F03

WINGDIAPI void APIENTRY glClearColor(GLclampf red,GLclampf green,GLclampf blue,GLclampf alpha);
WINGDIAPI void APIENTRY glClear(GLbitfield mask);
WINGDIAPI void APIENTRY glViewport(GLint x,GLint y,GLsizei width,GLsizei height);
WINGDIAPI void APIENTRY glDrawElements(GLenum mode,GLsizei count,GLenum type,const GLvoid *indices);
WINGDIAPI const GLubyte *APIENTRY glGetString(GLenum name);
WINGDIAPI void APIENTRY glGetIntegerv(GLenum pname,GLint *params);
WINGDIAPI void APIENTRY glGetBooleanv(GLenum pname,GLboolean *params);
WINGDIAPI void APIENTRY glEnable(GLenum cap);
WINGDIAPI void APIENTRY glDisable(GLenum cap);
WINGDIAPI void APIENTRY glDepthFunc(GLenum func);
WINGDIAPI void APIENTRY glCullFace(GLenum mode);
WINGDIAPI GLenum APIENTRY glGetError(void);

WINGDIAPI void APIENTRY glGenTextures(GLsizei n,GLuint *textures);
WINGDIAPI void APIENTRY glBindTexture(GLenum target,GLuint texture);
WINGDIAPI void APIENTRY glTexImage2D(GLenum target,GLint level,GLint internalformat,GLsizei width,GLsizei height,GLint border,GLenum format,GLenum type,const GLvoid *pixels);
WINGDIAPI void APIENTRY glTexParameterf(GLenum target,GLenum pname,GLfloat param);
WINGDIAPI void APIENTRY glTexParameterfv(GLenum target,GLenum pname,const GLfloat *params);
WINGDIAPI void APIENTRY glTexParameteri(GLenum target,GLenum pname,GLint param);
WINGDIAPI void APIENTRY glTexParameteriv(GLenum target,GLenum pname,const GLint *params);
WINGDIAPI void APIENTRY glTexSubImage1D(GLenum target,GLint level,GLint xoffset,GLsizei width,GLenum format,GLenum type,const GLvoid *pixels);
WINGDIAPI void APIENTRY glTexSubImage2D(GLenum target,GLint level,GLint xoffset,GLint yoffset,GLsizei width,GLsizei height,GLenum format,GLenum type,const GLvoid *pixels);

////// mingl.h //////
#endif // _MINWIN_H_

