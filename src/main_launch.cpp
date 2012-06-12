/*----------------------------------------------------------------------------
 * Copyright 2007-2011  Kazuo Ishii <kish@wb3.so-net.ne.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *--------------------------------------------------------------------------*/
#define _WIN32_WINNT 0x600
#define WIN32_LEAN_AND_MEAN 1
#define UNICODE  1
#include <Windows.h>
#include <ShellApi.h>
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#pragma comment(linker, "/nodefaultlib:libcmtd.lib")
#pragma comment(linker, "/subsystem:windows")
#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shell32.lib")

void* operator new(size_t n){ return HeapAlloc(GetProcessHeap(), 0, n);}
void  operator delete(void* p){ HeapFree(GetProcessHeap(), 0, p);}

static bool  send_command(HWND hwnd){
	LPWSTR  cmdline = GetCommandLineW();
	WCHAR   workdir [MAX_PATH+8];
	DWORD   len = GetCurrentDirectory(MAX_PATH, workdir);
	struct{ DWORD  magic, len0, len1;  LPCWSTR ptr0, ptr1; } lp = {
		0x80E082C3,
		lstrlen(cmdline),
		len,
		cmdline,
		workdir,
	};

	#if 0
	//wait ForegroundLockTimeout
	DWORD timeout = 0;
	SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &timeout, 0);
	if(1 <= timeout && timeout <= 250 /*250msec*/){
		Sleep(timeout);
	}
	#endif

	LRESULT lr = SendMessage(hwnd, WM_USER+77, (WPARAM)GetCurrentProcessId(), (LPARAM)&lp);
	return (lr == 0xB73D6A44) ? true : false;
}

static HWND  find_ck_window(HWND current){
	return FindWindowEx(HWND_MESSAGE, current, L"ckApplicationClass", 0);
}

static HWND  find_parent_ck_window(){
	HWND  result = 0;
	//BOOL (WINAPI *attach)(DWORD) = (BOOL(WINAPI*)(DWORD)) GetProcAddress(GetModuleHandle(L"kernel32.dll"),"AttachConsole");
	//if(attach && attach(ATTACH_PARENT_PROCESS)){
	if(AttachConsole(ATTACH_PARENT_PROCESS)){
		DWORD  i = GetConsoleProcessList(&i,1) + 64;
		DWORD* pids = new DWORD[i];
		if(pids){
			DWORD nb = GetConsoleProcessList(pids,i);
			if(1<=nb && nb<=i){
				HWND  hwnd=0;
				DWORD pid, idx=nb;
				while(hwnd = find_ck_window(hwnd)){
					pid = 0;
					GetWindowThreadProcessId(hwnd, &pid);
					for(i=0; i < nb; i++){
						if(pids[i] == pid && idx > i){
							result = hwnd;
							idx = i;
						}
					}
				}
			}
			delete [] pids;
		}
		FreeConsole();
	}
	return result;
}

static HRESULT startup(){
	// 1. process tree
	HWND hwnd = find_parent_ck_window();
	if(!hwnd){
		int     argc = 0;
		WCHAR** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
		if(argv){
			if(argc >= 2){
				if(lstrcmp(argv[1], L"-share") == 0){
					// 2. share mode
					hwnd = find_ck_window(0);
				}
			}
			LocalFree(argv);
		}
	}
	if(hwnd){
		// exists
		if( send_command(hwnd) ){
			// success
			return S_OK;
		}
	}

	// new instance

	WCHAR  image [MAX_PATH+8];
	DWORD  n = GetModuleFileName(0, image, MAX_PATH+4);
	if(n<1) return E_FAIL;

	for(; n>0 && image[n-1] != '\\'; n--);
	lstrcpy(image+n, L"ck.con.exe");

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	si.cb = sizeof(si);
	si.lpReserved = 0;
	si.lpDesktop = 0;
	si.lpTitle = 0;
	si.dwX = 0;
	si.dwY = 0;
	si.dwXSize = 0;
	si.dwYSize = 0;
	si.dwXCountChars = 0;
	si.dwYCountChars = 0;
	si.dwFillAttribute = 0;
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	si.cbReserved2 = 0;
	si.lpReserved2 = 0;
	si.hStdInput = 0;
	si.hStdOutput = 0;
	si.hStdError = 0;

	pi.hProcess = 0;
	pi.hThread = 0;
	pi.dwProcessId = 0;
	pi.dwThreadId = 0;

	if(CreateProcess(image, GetCommandLine(), 0,0,FALSE, CREATE_DEFAULT_ERROR_MODE|CREATE_NEW_CONSOLE|CREATE_NEW_PROCESS_GROUP|NORMAL_PRIORITY_CLASS, 0,0,&si,&pi)){
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return S_OK;
	}
	return E_FAIL;
}

extern "C" void WinMainCRTStartup(){
	ExitProcess((UINT)startup());
}

//EOF
