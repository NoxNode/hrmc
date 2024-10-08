// cl nolibs.c -nologo -Oi -Zi -GS- -Gs9999999 -link -subsystem:windows -nodefaultlib -stack:0x100000,0x100000 -machine:x64 -entry:"_start" && ./nolibs.exe ; echo $?
//gcc nolibs.c -o nolibs.exe -O0 -g -w -mwindows -m64 -nostdlib -Wl,-e_start && ./nolibs.exe ; echo $?
//objdump -M intel -d -j .text nolibs.exe
//#include <windows.h>
#include "minwin.h"

LRESULT CALLBACK Win32EventHandler(HWND window, UINT msg, WPARAM wp, LPARAM lp) {
	if(msg == WM_DESTROY)
		PostQuitMessage(0);
	return DefWindowProcA(window, msg, wp, lp);
}

int CALLBACK WinMain(HINSTANCE I, HINSTANCE PI, LPSTR C, int S) {
	// register class
	WNDCLASSA winclass = {0};
	winclass.lpfnWndProc = Win32EventHandler;
	winclass.hInstance = I;
	winclass.lpszClassName = "winclass";
	RegisterClassA(&winclass);

	// create window
	HWND window = CreateWindowExA(0, winclass.lpszClassName, "title",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
			0, 0, winclass.hInstance, 0);
	int running = 1;
	MSG msg;
	while(running) {
		while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
			if(msg.message == WM_QUIT) {
				running = 0;
				break;
			}
			// TODO: see if we can get keyinput and stuff here or need to bein callback
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		if(!running)
			break;
	}
	return(0);
}

//#define CHECK_CRT_FUNCTIONALITY
#ifdef CHECK_CRT_FUNCTIONALITY
void check_crt_functionality();
#endif
void __stdcall _start() {
#ifdef CHECK_CRT_FUNCTIONALITY
	check_crt_functionality();
#endif
	DynamicLink();
	int Result = WinMain(GetModuleHandleA(0), 0, 0, 0);
	ExitProcess(Result);
}

// https://hero.handmade.network/forums/code-discussion/t/94-guide_-_how_to_avoid_c_c++_runtime_on_windows
#ifdef CHECK_CRT_FUNCTIONALITY
void check_crt_functionality() {
	// check if you can allocate > 4k chunks on stack
	char BigArray[4096];
    BigArray[0] = 0;

	// check if you can do operations on 64-bit types in 32-bit mode
	volatile int64_t s = 1;
    volatile uint64_t u = 1;

    s += s;
    s -= s;
    s *= s;
    s /= s;
    s %= s;
    s >>= 33;
    s <<= 33;

    u += u;
    u -= u;
    u *= u;
    u /= u;
    u %= u;
    u >>= 33;
    u <<= 33;

	// check if you can use floating point numers
	float f;
    f = 0.0f;

	// check if you can convert from int to float and vise versa in 32-bit mode
    f = 1000.0f;
    double d = 1000000000.0;

    int32_t i32f = (int32_t)f;
    int32_t i32d = (int32_t)d;
    uint32_t u32f = (uint32_t)f;
    uint32_t u32d = (uint32_t)d;

    int64_t i64f = (int64_t)f;
    int64_t i64d = (int64_t)d;
    uint64_t u64f = (uint64_t)f;
    uint64_t u64d = (uint64_t)d;

    f = (float)i32f;
    d = (double)i32d;
    f = (float)u32f;
    d = (double)u32d;

    f = (float)i64f;
    d = (double)i64d;
    f = (float)u64f;
    d = (double)u64d;

	// check if you can initialize and assign big arrays or structs
	char BigArray2[100] = {};
}
#endif
