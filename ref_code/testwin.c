// cl  winasm.c -nologo -Oi -GS- -Gs9999999 -link -subsystem:windows -nodefaultlib -stack:0x100000,0x100000 -machine:x64 -entry:"_start"
//gcc -g testwin.c && ./a.exe
//#include <windows.h>
//#include <stdio.h>
#include "min_common_win.c"

u64* Win32EventHandler(void* window, u32 msg, u32* wp, u64* lp) {
	if(msg == WM_DESTROY)
		PostQuitMessage(0);
	return DefWindowProcA(window, msg, wp, lp);
}

int WinMain(void* I, void* PI, void* C, int S) {
	// register class
	WNDCLASSA winclass = {};
	winclass.lpfnWndProc = Win32EventHandler;
	winclass.hInstance = I;
	winclass.lpszClassName = "winclass";
	RegisterClassA(&winclass);
	printf("%d\n", sizeof(winclass)); //0x48
	printf("%d\n", offsetof(WNDCLASSA, lpfnWndProc)); //0x08
	printf("%d\n", offsetof(WNDCLASSA, hInstance)); //0x18
	printf("%d\n", offsetof(WNDCLASSA, lpszClassName)); //0x40
	printf("%x\n", WS_OVERLAPPEDWINDOW|WS_VISIBLE);

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
			// TODO: see if we can get keyinput and stuff here or need to be in callback
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
		if(!running)
			break;
	}
	return(0);
}

