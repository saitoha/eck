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
#define PACKAGE_NAME "eck"
#define _WIN32_WINNT 0x600
#define WIN32_LEAN_AND_MEAN 1
#define UNICODE  1
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <pty.h>
#include <signal.h>

typedef LPWSTR* (WINAPI * tCommandLineToArgvW)(LPCWSTR,int*);
typedef int     (WINAPI * tMessageBoxW)(HWND,LPCWSTR,LPCWSTR,DWORD);
typedef HRESULT (WINAPI * tOleInitialize)(LPVOID);
typedef HRESULT (WINAPI * tOleUninitialize)();
typedef void    (* tAppStartup)();

tCommandLineToArgvW  gCommandLineToArgvW = 0;


__attribute__((dllexport)) HRESULT  execpty(LPCWSTR cmdline, int* out_pid, int* out_fd){
	char** args;
	int    fd;
	int    pid;

	if(!out_pid || !out_fd)
		return E_POINTER;
	*out_pid = 0;
	*out_fd = -1;

	if(!cmdline || !cmdline[0]){
		const char* sh = getenv("SHELL");
		if(!sh || !sh[0]){
			struct passwd* pw = getpwuid(getuid());
			if(pw) sh = pw->pw_shell;
		}
		if(!sh || !sh[0]){
			sh = "/bin/bash";
		}
		args = (char**) malloc( sizeof(char*)*2 + lstrlenA(sh) + 4);
		args[0] = (char*)(args+2);
		args[1] = 0;
		lstrcpyA(args[0], sh);
	}
	else{
		int  i, n;
		char *p, *e;

		int     wargc=0;
		LPWSTR* wargv = gCommandLineToArgvW(cmdline, &wargc);
		if(!wargv)
			return E_FAIL;
		n = (int) LocalSize(wargv);
		n = (n/2)*3;
		args = (char**) malloc(n);
		p = (char*)(args+wargc+1);
		e = ((char*)args) + n;
		for(i=0; i < wargc; i++){
			args[i] = p;
			n = WideCharToMultiByte(CP_UTF8,0, wargv[i], lstrlenW(wargv[i]), p, (int)(e-p), 0,0);
			if(n>0) p+=n;
			*p++ = '\0';
		}
		args[wargc] = 0;
		LocalFree(wargv);
	}

	fd  = -1;
	pid = forkpty(&fd,0,0,0);
	if(pid==0){
		signal(SIGCHLD, SIG_DFL);
		execvp(args[0],args);
		exit(0);
	}

	free(args);

	if(pid<0 || fd==-1){
		return E_FAIL;
	}

	*out_pid = pid;
	*out_fd  = fd;
	return S_OK;
}


int  main(int argc, char* argv[]){
	HRESULT  hr;
	HMODULE  shell =0;
	HMODULE  user =0;
	HMODULE  ole =0;
	HMODULE  app =0;
	tOleInitialize    pOleInitialize =0;
	tOleUninitialize  pOleUninitialize =0;
	tMessageBoxW      pMessageBoxW =0;
	tAppStartup       pAppStartup =0;

	signal(SIGCHLD, SIG_IGN);

	{/* close cygwin-fhandle */
		HANDLE ci,co,ce,process = GetCurrentProcess();
		DuplicateHandle(process, GetStdHandle(STD_INPUT_HANDLE),  process, &ci, 0, FALSE, DUPLICATE_SAME_ACCESS);
		DuplicateHandle(process, GetStdHandle(STD_OUTPUT_HANDLE), process, &co, 0, FALSE, DUPLICATE_SAME_ACCESS);
		DuplicateHandle(process, GetStdHandle(STD_ERROR_HANDLE),  process, &ce, 0, FALSE, DUPLICATE_SAME_ACCESS);
		close(0);
		close(1);
		close(2);
		SetStdHandle(STD_INPUT_HANDLE,  ci);
		SetStdHandle(STD_OUTPUT_HANDLE, co);
		SetStdHandle(STD_ERROR_HANDLE,  ce);
	}

	shell = LoadLibrary(L"shell32.dll");
	ole   = LoadLibrary(L"ole32.dll");
	user  = LoadLibrary(L"user32.dll");
	if(!user || !ole || !shell){
		hr = HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
	}
	else{
		gCommandLineToArgvW = (tCommandLineToArgvW) GetProcAddress(shell,"CommandLineToArgvW");
		pMessageBoxW        = (tMessageBoxW)        GetProcAddress(user, "MessageBoxW");
		pOleInitialize      = (tOleInitialize)      GetProcAddress(ole,  "OleInitialize");
		pOleUninitialize    = (tOleUninitialize)    GetProcAddress(ole,  "OleUninitialize");
		if(!gCommandLineToArgvW || !pMessageBoxW || !pOleInitialize || !pOleUninitialize){
			hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
		}
		else{
			hr = pOleInitialize(0);
			if(FAILED(hr)){
			}
			else{
				app = LoadLibrary(PACKAGE_NAME L".app.dll");
				if(!app){
					hr = HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
				}
				else{
					pAppStartup = (tAppStartup) GetProcAddress(app,"AppStartup");
					if(!pAppStartup){
						hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
					}
					else{
						pAppStartup();
						hr = S_OK;
					}
				}
				pOleUninitialize();
			}
		}
	}

	if(FAILED(hr)){
		if(pMessageBoxW){
			LPWSTR  msg = 0;
			DWORD size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
						   0,hr,0,(LPWSTR)&msg,0,NULL);
			if(size){
				pMessageBoxW(NULL, msg, L"ERROR", MB_OK|MB_ICONSTOP);
				LocalFree(msg);
			}
		}
	}

	if(user)  FreeLibrary(user);
	if(ole)   FreeLibrary(ole);
	if(shell) FreeLibrary(shell);
	return (int) hr;
}

/* EOF */
