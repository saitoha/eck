/*----------------------------------------------------------------------------
 * Copyright 2007,2009  Kazuo Ishii <kish@wb3.so-net.ne.jp>
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
#define _UNICODE 1
#include <Windows.h>
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#pragma comment(linker, "/nodefaultlib:libcmtd.lib")
#pragma comment(linker, "/subsystem:console")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "cygwin1_con.lib")


#if 0
void trace(const char* fmt, ...){
    char buf [256];
    va_list va;
    va_start(va,fmt);
    int n = wvsprintfA(buf,fmt,va);
    va_end(va);
    if(n>0)WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),buf,n,(DWORD*)&n,0);
}
#endif

namespace cygwin{
    enum{
        SIGCHLD = 20,
        //
        SIG_DFL = 0,
        SIG_IGN = 1,
        SIG_ERR = -1,
    };
    struct passwd{
        char*    pw_name;
        char*    pw_passwd;
        int    pw_uid;
        int    pw_gid;
        char*    pw_comment;
        char*    pw_gecos;
        char*    pw_dir;
        char*    pw_shell;
    };
    extern "C"{
        __declspec(dllimport) int   close(int);
        __declspec(dllimport) void  cygwin_dll_init();
        __declspec(dllimport) void  execvp(char*,char**);
        __declspec(dllimport) void  exit(int);
        __declspec(dllimport) int   forkpty(int*,void*,void*,void*);
        __declspec(dllimport) void  free(void*);
        __declspec(dllimport) char* getenv(const char*);
        __declspec(dllimport) struct passwd* getpwuid(int);
        __declspec(dllimport) int   getuid();
        __declspec(dllimport) void* malloc(unsigned int);
        __declspec(dllimport) int   signal(int,int);
    }
}

typedef LPWSTR* (WINAPI * tCommandLineToArgvW)(LPCWSTR,int*);
typedef HRESULT (WINAPI * tOleInitialize)(LPVOID);
typedef HRESULT (WINAPI * tOleUninitialize)();
typedef void    (* tAppStartup)();

tCommandLineToArgvW gCommandLineToArgvW = 0;

extern "C" __declspec(dllexport) HRESULT execpty(LPCWSTR cmdline, int* out_pid, int* out_fd){
    if(!out_pid || !out_fd)
        return E_POINTER;
    *out_pid = 0;
    *out_fd = -1;

    char** args;

    if(!cmdline || !cmdline[0]){
        char* sh = cygwin::getenv("SHELL");
        if(!sh || !sh[0]){
            struct cygwin::passwd* pw = cygwin::getpwuid(cygwin::getuid());
            if(pw) sh = pw->pw_shell;
        }
        if(!sh || !sh[0]){
            sh = "/bin/bash";
        }
        args = (char**) cygwin::malloc( sizeof(char*)*2 + lstrlenA(sh) + 4);
        args[0] = (char*)(args+2);
        args[1] = 0;
        lstrcpyA(args[0], sh);
    }
    else{
        int wargc=0;
        LPWSTR* wargv = gCommandLineToArgvW(cmdline, &wargc);
        if(!wargv)
            return E_FAIL;
        int n = (int) LocalSize(wargv);
        n = (n/2)*3;
        args = (char**) cygwin::malloc(n);
        char* p = (char*)(args+wargc+1);
        char* e = ((char*)args) + n;
        for(int i=0; i < wargc; i++){
            args[i] = p;
            n = WideCharToMultiByte(CP_UTF8,0, wargv[i], lstrlenW(wargv[i]), p, (int)(e-p), 0,0);
            if(n>0) p+=n;
            *p++ = '\0';
        }
        args[wargc] = 0;
        LocalFree(wargv);
    }

    int fd=-1;
    int pid=cygwin::forkpty(&fd,0,0,0);
    if(pid==0){
        cygwin::signal(cygwin::SIGCHLD, cygwin::SIG_DFL);
        cygwin::execvp(args[0],args);
        cygwin::exit(0);
    }

    cygwin::free(args);

    if(pid<0 || fd==-1){
        return E_FAIL;
    }

    *out_pid = pid;
    *out_fd = fd;
    return S_OK;
}

HRESULT cpp_startup(){
    HRESULT hr;
    HMODULE app = LoadLibrary(L"eck.app.dll");
    if(!app){
        hr = HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
    }
    else{
        tAppStartup  pAppStartup = (tAppStartup) GetProcAddress(app,"AppStartup");
        if(!pAppStartup){
            hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        }
        else{
            pAppStartup();
        }
    }
    if(app) FreeLibrary(app);
    return hr;
}

#if 0
#include <mscoree.h>
typedef HRESULT (WINAPI * tCorBindToRuntimeEx)(LPWSTR,LPWSTR,DWORD,REFCLSID,REFIID,LPVOID*);
HRESULT clr_startup(){
    HRESULT hr;
    HMODULE cor = LoadLibrary(L"mscoree.dll");
    if(!ole || !cor){
        hr = HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
    }
    else{
        tCorBindToRuntimeEx  pCorBindToRuntimeEx = (tCorBindToRuntimeEx) GetProcAddress(cor, "CorBindToRuntimeEx");
        if(!pCorBindToRuntimeEx){
            hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        }
        else{
            ICLRRuntimeHost* host=0;
            hr = pCorBindToRuntimeEx(L"v2.0.50727", L"wks", 0, CLSID_CLRRuntimeHost, __uuidof(ICLRRuntimeHost), (void**)&host);
            if(FAILED(hr)){
            }
            else{
                hr = host->Start();
                if(FAILED(hr)){
                }
                else{
                    DWORD dw;
                    hr = host->ExecuteInDefaultAppDomain(L"eck.clr.dll", L"Eck.App", L"Startup", L"", &dw);
                    if(FAILED(hr)){
                    }
                    else{
                    }
                    host->Stop();
                }
                host->Release();
            }
        }
    }
    if(cor) FreeLibrary(cor);
    return hr;
}
#endif


HRESULT startup(){
    #if 0
    __asm{//clear StartupInfo.wShowWindow
        push eax;
        push edx;
        mov eax, dword ptr fs:[0x18];           //TEB (Thread Environment Block)
        mov eax, dword ptr [eax+0x30];          //TEB->PEB (Process Environment Block)
        mov eax, dword ptr [eax+0x10];          //PEB->ProcessParameters
        mov edx, ~STARTF_USESHOWWINDOW;
        and dword ptr [eax+0x68], edx;          //ProcessParameters->WindowFlags &= ~STARTF_USESHOWWINDOW;
        mov  word ptr [eax+0x6C], SW_SHOWNORMAL;//ProcessParameters->ShowWindowFlags = SW_SHOWNORMAL;
        pop edx;
        pop eax;
    }
    #endif

    cygwin::cygwin_dll_init();
    cygwin::signal(cygwin::SIGCHLD, cygwin::SIG_IGN);

    {//close cygwin-fhandle
        HANDLE ci,co,ce,process = GetCurrentProcess();
        DuplicateHandle(process, GetStdHandle(STD_INPUT_HANDLE),  process, &ci, 0, FALSE, DUPLICATE_SAME_ACCESS);
        DuplicateHandle(process, GetStdHandle(STD_OUTPUT_HANDLE), process, &co, 0, FALSE, DUPLICATE_SAME_ACCESS);
        DuplicateHandle(process, GetStdHandle(STD_ERROR_HANDLE),  process, &ce, 0, FALSE, DUPLICATE_SAME_ACCESS);
        cygwin::close(0);
        cygwin::close(1);
        cygwin::close(2);
        SetStdHandle(STD_INPUT_HANDLE,  ci);
        SetStdHandle(STD_OUTPUT_HANDLE, co);
        SetStdHandle(STD_ERROR_HANDLE,  ce);
    }

    HRESULT hr;
    HMODULE ole = LoadLibrary(L"ole32.dll");
    HMODULE shl = LoadLibrary(L"shell32.dll");
    if(!ole || !shl){
        hr = HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
    }
    else{
        gCommandLineToArgvW = (tCommandLineToArgvW) GetProcAddress(shl,"CommandLineToArgvW");
        tOleInitialize  pOleInitialize = (tOleInitialize) GetProcAddress(ole, "OleInitialize");
        tOleUninitialize  pOleUninitialize = (tOleUninitialize) GetProcAddress(ole, "OleUninitialize");
        if(!gCommandLineToArgvW || !pOleInitialize || !pOleUninitialize){
            hr = HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        }
        else{
            hr = pOleInitialize(0);
            if(FAILED(hr)){
            }
            else{
                hr = cpp_startup();
                //hr = clr_startup();
            }
            pOleUninitialize();
        }
    }
    if(shl) FreeLibrary(shl);
    if(ole) FreeLibrary(ole);

    if(FAILED(hr)){
        LPWSTR  msg = 0;
        DWORD size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
            0,hr,0,(LPWSTR)&msg,0,NULL);
        if(size){
            MessageBox(NULL, msg, L"ERROR", MB_OK|MB_ICONSTOP);
            LocalFree(msg);
        }
    }
    return hr;
}

extern "C" void mainCRTStartup(){
    char cygtls [32768];
    __asm mov eax, dword ptr cygtls; //cancel optimize
    ExitProcess( (UINT) startup() );
}

//EOF
