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
#include "config.h"
//#import  "interface.tlb" raw_interfaces_only
#include "interface.tlh"
#include "util.h"
#include <io.h> //_isatty


ITypeLibPtr  g_typelib;
HMODULE      g_this_module = 0;

//ck.con.exe
HRESULT (* cyg_execpty)(LPCWSTR, int*, int*) = 0;
//cygwin1.dll
char*   (* cyg_getenv)(const char*) = 0;
int     (* cyg_setenv)(const char*,const char*,int) = 0;
int     (* cyg_unsetenv)(const char*) = 0;
char*   (* cyg_getcwd)(char*,size_t) = 0;
int*    (* cyg_errno)() = 0;

void trace(const char* fmt, ...){
	va_list va;
	va_start(va,fmt);
	vfprintf(stdout, fmt, va);
	va_end(va);
	fflush(stdout);
}

//pty.cpp
extern "C" HRESULT CreatePty(Ck::IPtyNotify* cb, BSTR cmdline, Ck::IPty** pp);
//window.cpp
extern "C" HRESULT CreateWnd(Ck::IWndNotify* cb, Ck::IWnd** pp);


namespace Ck{


static LPCWSTR _errmsg_unknown = L"error";

static LPCWSTR  errmsg_get(HRESULT hr){
	DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
	LPWSTR  msg;
	DWORD size = FormatMessage(flags, 0, hr,0,(LPWSTR)&msg,0,NULL);
	return size ? msg : _errmsg_unknown;
}

static void  errmsg_free(LPCWSTR msg){
	if(msg && msg != _errmsg_unknown)
		LocalFree((HLOCAL)msg);
}


class Console{
private:
	HWND	m_hwnd;
	HANDLE	m_out;
	bool	m_wincon;
	//
	void setcolor(WORD attr){
		if(m_wincon){
			SetConsoleTextAttribute(m_out, attr);
		}
	}
	void put(LPCWSTR msg){
		if(!msg) return;
		DWORD dw;
		int wlen = (int) wcslen(msg);
		if(m_wincon){
			if(wlen > 0){
				WriteConsole(m_out, msg,wlen, &dw, 0);
			}
		}
		else{
			int alen = wlen * 3 + 32;
			if(alen > 0){
				char* abuf = new char[alen];
				alen = WideCharToMultiByte(CP_ACP,0, msg,wlen, abuf,alen-1, 0,0);
				if(alen > 0){
					WriteFile(m_out, abuf,alen,&dw, 0);
				}
				delete [] abuf;
			}
		}
	}
public:
	Console(): m_hwnd(0), m_out(0), m_wincon(false){
		m_hwnd = GetConsoleWindow();
		m_out = GetStdHandle(STD_OUTPUT_HANDLE);
		m_wincon = _isatty(1) ? true : false;
		//trace("isatty=%d\n", m_wincon);
	}
	VARIANT_BOOL IsShow(){
		return (m_hwnd && IsWindowVisible(m_hwnd)) ? TRUE : FALSE;
	}
	void Show(VARIANT_BOOL show){
		if(m_hwnd){
			int b = (show ? 1 : 0) ^ (IsWindowVisible(m_hwnd) ? 1 : 0);
			if(b) Util::show_window(m_hwnd, show ? SW_SHOW : SW_HIDE);
		}
	}
	void PutInfo(LPCWSTR msg){
		put(msg);
	}
	void PutError(LPCWSTR msg){
		setcolor(0x0C);
		put(msg);
		setcolor(0x07);
		if(m_wincon) Show(TRUE);
	}
	void PutError(HRESULT hr){
		WCHAR wbuf [256];
		StringCbPrintf(wbuf, sizeof(wbuf), L"[ERROR] (x%x)\n\t", hr);

		LPCWSTR msg = errmsg_get(hr);

		setcolor(0x0C);
		put(wbuf);
		put(msg);
		put(L"\n");
		setcolor(0x07);
		if(m_wincon) Show(TRUE);

		errmsg_free(msg);
	}
	void PutError(IActiveScriptError* err){
		if(!err) return;
		EXCEPINFO excep;
		DWORD ctx=0;
		ULONG line=0;
		LONG  charpos=0;
		BSTR  srccode = 0;
		memset(&excep, 0, sizeof(excep));
		err->GetExceptionInfo(&excep);
		if(excep.pfnDeferredFillIn)
			excep.pfnDeferredFillIn(&excep);
		err->GetSourceLineText(&srccode);
		err->GetSourcePosition(&ctx, &line, &charpos);

		LPCWSTR file;
		switch(ctx){
		case 0:  file=L"SystemScript";break;
		case 1:  file=L"UserScript";break;
		default: file=L"unknown";break;
		}

		WCHAR wbuf [256];
		StringCbPrintf(wbuf, sizeof(wbuf), L"[SCRIPT ERROR] File '%s'  LINE:%d POS:%d\t(x%x)\n", file, (line+1),(charpos+1),excep.scode);

		setcolor(0x0C);
		put(wbuf);
		put(L"\t");
		put(srccode);
		put(L"\n\t");
		put(excep.bstrDescription);
		put(L"\n\t");
		put(excep.bstrSource);
		put(L"\n");
		setcolor(0x07);
		if(m_wincon) Show(TRUE);

		SysFreeString(excep.bstrSource);
		SysFreeString(excep.bstrDescription);
		SysFreeString(excep.bstrHelpFile);
		SysFreeString(srccode);
	}
};
static Console g_console;




static ModKey get_mod_key(DWORD vk){
	ModKey key = (ModKey)(vk & ModKey_Key);
	if(GetKeyState(VK_LSHIFT  )&0x8000) key = (ModKey)(key | ModKey_ShiftL);
	if(GetKeyState(VK_RSHIFT  )&0x8000) key = (ModKey)(key | ModKey_ShiftR);
	if(GetKeyState(VK_LCONTROL)&0x8000) key = (ModKey)(key | ModKey_CtrlL);
	if(GetKeyState(VK_RCONTROL)&0x8000) key = (ModKey)(key | ModKey_CtrlR);
	if(GetKeyState(VK_LMENU   )&0x8000) key = (ModKey)(key | ModKey_AltL);
	if(GetKeyState(VK_RMENU   )&0x8000) key = (ModKey)(key | ModKey_AltR);
	if(GetKeyState(VK_CAPITAL )&0x0001) key = (ModKey)(key | ModKey_Caps);
	return key;
}

static DISPID get_disp_id(IDispatch* disp, LPWSTR name){
	DISPID id = 0;
	if(disp){
		disp->GetIDsOfNames(IID_NULL, &name, 1, LOCALE_USER_DEFAULT, &id);
	}
	return id;
}
static HRESULT invoke_v(IDispatch* disp, DISPID id, UINT argc, VARIANTARG* argv){
	HRESULT hr=E_FAIL;
	if(disp && id){
		DISPPARAMS param = { argv, NULL, argc, 0 };
		hr = disp->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &param, 0,0,0);
	}
	return hr;
}
static void set_error_info(HRESULT hr){
	ICreateErrorInfo* cei=0;
	IErrorInfo* ei=0;
	::CreateErrorInfo(&cei);
	if(cei){
		LPCWSTR msg = errmsg_get(hr);
		cei->SetDescription( (LPOLESTR) msg);
		errmsg_free(msg);
		cei->QueryInterface(__uuidof(IErrorInfo), (void**)&ei);
		if(ei){
			SetErrorInfo(0, ei);
			ei->Release();
		}
		cei->Release();
	}
}
static SAFEARRAY* cmdline_to_vbarray(LPCWSTR cmdline){
	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(cmdline, &argc);
	if(argv){
		SAFEARRAY* sa = SafeArrayCreateVector(VT_VARIANT, 0, argc);
		if(sa){
			VARIANTARG* pvar;
			if(SUCCEEDED(SafeArrayAccessData(sa, (void**)&pvar))){
				for(int i=0; i < argc; i++){
					VariantInit(&pvar[i]);
					pvar[i].vt = VT_BSTR;
					pvar[i].bstrVal = SysAllocString(argv[i]);
				}
				SafeArrayUnaccessData(sa);
				LocalFree(argv);
				return sa;
			}
			SafeArrayDestroy(sa);
		}
		LocalFree(argv);
	}
	return NULL;
}



class App: public IDispatchImpl_<IApp>, public IWndNotify, public IPtyNotify, public IActiveScriptSite{
	BEGIN_COM_MAP
	COM_INTERFACE_ENTRY(IApp)
	COM_INTERFACE_ENTRY(IWndNotify)
	COM_INTERFACE_ENTRY(IPtyNotify)
	COM_INTERFACE_ENTRY(IActiveScriptSite)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY2(IUnknown,IApp)
	END_COM_MAP
	//
private:
	static App* g_app;
	HWND	m_hwnd;
	IActiveScriptPtr m_engine;
	IDispatchPtr     m_event_root;
	DISPID	m_event_app_on_startup;
	DISPID	m_event_app_on_new_command;
	DISPID	m_event_wnd_on_closed;
	DISPID	m_event_wnd_on_key_down;
	DISPID	m_event_wnd_on_mouse_wheel;
	DISPID	m_event_wnd_on_menu_init;
	DISPID	m_event_wnd_on_menu_execute;
	DISPID	m_event_wnd_on_title_init;
	DISPID	m_event_wnd_on_drop;
	DISPID	m_event_pty_on_closed;
	DISPID	m_event_pty_on_update_screen;
	DISPID	m_event_pty_on_update_title;
	DISPID	m_event_pty_on_req_font;
	DISPID	m_event_pty_on_req_move;
	DISPID	m_event_pty_on_req_resize;
	HMENU	m_work_menu;
protected:
	static LRESULT CALLBACK _wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp){
		switch(msg){
		case WM_NCDESTROY:
			DefWindowProc(hwnd,msg,wp,lp);
			g_app->m_hwnd = 0;
			g_app->wm_on_nc_destroy();
			PostQuitMessage(0);
			return 0;
		case WM_CREATE:
			g_app->m_hwnd = hwnd;
			break;
		case WM_USER+13: g_app->pty_on_closed       ((IPty*)wp);return 0;
		case WM_USER+14: g_app->pty_on_update_screen((IPty*)wp);return 0;
		case WM_USER+15: g_app->pty_on_update_title ((IPty*)wp);return 0;
		case WM_USER+16: g_app->pty_on_req_font     ((IPty*)wp, (int)lp );return 0;
		case WM_USER+17: g_app->pty_on_req_move     ((IPty*)wp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));return 0;
		case WM_USER+18: g_app->pty_on_req_resize   ((IPty*)wp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));return 0;
		case WM_USER+77: return g_app->app_on_new_command_op((DWORD)wp, (void*)lp);
		}
		return DefWindowProc(hwnd,msg,wp,lp);
	}

	#if 1
	LRESULT app_on_new_command_op(DWORD pid, void* lp){
		LRESULT result = 0;
		HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
		if(process){
			struct{ DWORD  magic, len0, len1;  LPCWSTR ptr0, ptr1; } data;
			DWORD n;
			if(ReadProcessMemory(process, lp, &data, sizeof(data), &n)){
				if(data.magic == 0x80E082C3 &&
				   data.len0 < 32768 /* max 32kB */ &&
				   data.len1 < 32768 /* max 32kB */ ){
					BSTR  cmdline = SysAllocStringLen(0, data.len0);
					BSTR  workdir = SysAllocStringLen(0, data.len1);
					if(cmdline && workdir){
						if(ReadProcessMemory(process, data.ptr0, cmdline, sizeof(WCHAR) * data.len0, &n) &&
						   ReadProcessMemory(process, data.ptr1, workdir, sizeof(WCHAR) * data.len1, &n)){
							((UINT*)cmdline)[-1] = sizeof(WCHAR) * data.len0;
							((UINT*)workdir)[-1] = sizeof(WCHAR) * data.len1;
							cmdline[data.len0] = '\0';
							workdir[data.len1] = '\0';
							app_on_new_command(cmdline, workdir);
							result = 0xB73D6A44;//magic
						}
					}
					if(cmdline) SysFreeString(cmdline);
					if(workdir) SysFreeString(workdir);
				}
			}
			CloseHandle(process);
		}
		return result;
	}
	void app_on_new_command(BSTR cmdline, BSTR workdir){
		::trace("command='%S'\n", cmdline);
		::trace("workdir='%S'\n", workdir);
		SAFEARRAY* sa = cmdline_to_vbarray(cmdline);
		if(sa){
			VARIANTARG args[2];
			args[1].vt = VT_ARRAY | VT_VARIANT;
			args[1].parray = sa;
			args[0].vt = VT_BSTR;
			args[0].bstrVal = workdir;
			invoke_v(m_event_root, m_event_app_on_new_command, 2, args);
			SafeArrayDestroy(sa);
		}
	}
	#endif

	void pty_on_closed(IPty* sender){
		VARIANTARG args[1];
		args[0].vt = VT_DISPATCH;
		args[0].pdispVal = sender;
		invoke_v(m_event_root, m_event_pty_on_closed, 1, args);
	}
	void pty_on_update_screen(IPty* sender){
		VARIANTARG args[1];
		args[0].vt = VT_DISPATCH;
		args[0].pdispVal = sender;
		invoke_v(m_event_root, m_event_pty_on_update_screen, 1, args);
	}
	void pty_on_update_title(IPty* sender){
		VARIANTARG args[1];
		args[0].vt = VT_DISPATCH;
		args[0].pdispVal = sender;
		invoke_v(m_event_root, m_event_pty_on_update_title, 1, args);
	}
	void pty_on_req_font(IPty* sender, int id){
		VARIANTARG args[2];
		args[1].vt = VT_DISPATCH;
		args[1].pdispVal = sender;
		args[0].vt = VT_I4;
		args[0].lVal = (LONG)id;
		invoke_v(m_event_root, m_event_pty_on_req_font, 2, args);
	}
	void pty_on_req_move(IPty* sender, int x, int y){
		VARIANTARG args[3];
		args[2].vt = VT_DISPATCH;
		args[2].pdispVal = sender;
		args[1].vt = VT_I4;
		args[1].lVal = (LONG)x;
		args[0].vt = VT_I4;
		args[0].lVal = (LONG)y;
		invoke_v(m_event_root, m_event_pty_on_req_move, 3, args);
	}
	void pty_on_req_resize(IPty* sender, int x, int y){
		VARIANTARG args[3];
		args[2].vt = VT_DISPATCH;
		args[2].pdispVal = sender;
		args[1].vt = VT_I4;
		args[1].lVal = (LONG)x;
		args[0].vt = VT_I4;
		args[0].lVal = (LONG)y;
		invoke_v(m_event_root, m_event_pty_on_req_resize, 3, args);
	}

	void wm_on_nc_destroy();
	virtual ~App();
public:
	App();
	HRESULT initialize();

	//IPtyNotify
	STDMETHOD(OnClosed)(IPty* sender)		{ PostMessage(m_hwnd, WM_USER+13, (WPARAM)sender, 0);return S_OK;}
	STDMETHOD(OnUpdateScreen)(IPty* sender)		{ PostMessage(m_hwnd, WM_USER+14, (WPARAM)sender, 0);return S_OK;}
	STDMETHOD(OnUpdateTitle)(IPty* sender)		{ PostMessage(m_hwnd, WM_USER+15, (WPARAM)sender, 0);return S_OK;}
	STDMETHOD(OnReqFont)(IPty* sender, int id)	{ PostMessage(m_hwnd, WM_USER+16, (WPARAM)sender, (LPARAM)id);return S_OK;}
	STDMETHOD(OnReqMove)(IPty* sender, int x, int y){ PostMessage(m_hwnd, WM_USER+17, (WPARAM)sender, MAKELPARAM(y,x));return S_OK;}
	STDMETHOD(OnReqResize)(IPty* sender, int x, int y){PostMessage(m_hwnd,WM_USER+18, (WPARAM)sender, MAKELPARAM(y,x));return S_OK;}

	//IWndNotify
	STDMETHOD(OnClosed)(IWnd* sender){
		VARIANTARG args[1];
		args[0].vt = VT_DISPATCH;
		args[0].pdispVal = sender;
		invoke_v(m_event_root, m_event_wnd_on_closed, 1, args);
		return S_OK;
	}
	STDMETHOD(OnKeyDown)(IWnd* sender, DWORD vk){
		VARIANTARG args[2];
		args[1].vt = VT_DISPATCH;
		args[1].pdispVal = sender;
		args[0].vt = VT_I4;
		args[0].lVal = (LONG) get_mod_key(vk);
		invoke_v(m_event_root, m_event_wnd_on_key_down, 2, args);
		return S_OK;
	}
	STDMETHOD(OnMouseWheel)(IWnd* sender, int delta){
		VARIANTARG args[3];
		args[2].vt = VT_DISPATCH;
		args[2].pdispVal = sender;
		args[1].vt = VT_I4;
		args[1].lVal = (LONG) get_mod_key(0);
		args[0].vt = VT_I4;
		args[0].lVal = (LONG) delta;
		invoke_v(m_event_root, m_event_wnd_on_mouse_wheel, 3, args);
		return S_OK;
	}
	STDMETHOD(OnMenuInit)(IWnd* sender, int* menu){
		VARIANTARG args[1];
		args[0].vt = VT_DISPATCH;
		args[0].pdispVal = sender;
		m_work_menu = (HMENU) menu;
		invoke_v(m_event_root, m_event_wnd_on_menu_init, 1, args);
		m_work_menu = (HMENU) 0;
		return S_OK;
	}
	STDMETHOD(OnMenuExec)(IWnd* sender, DWORD id){
		VARIANTARG args[2];
		args[1].vt = VT_DISPATCH;
		args[1].pdispVal = sender;
		args[0].vt = VT_I4;
		args[0].lVal = (LONG)(id >> 4);
		invoke_v(m_event_root, m_event_wnd_on_menu_execute, 2, args);
		return S_OK;
	}
	STDMETHOD(OnTitleInit)(IWnd* sender){
		VARIANTARG args[1];
		args[0].vt = VT_DISPATCH;
		args[0].pdispVal = sender;
		invoke_v(m_event_root, m_event_wnd_on_title_init, 1, args);
		return S_OK;
	}
	STDMETHOD(OnDrop)(IWnd* sender, BSTR bs, int type, DWORD key){
		VARIANTARG args[4];
		args[3].vt = VT_DISPATCH;
		args[3].pdispVal = sender;
		args[2].vt = VT_BSTR;
		args[2].bstrVal = bs;
		args[1].vt = VT_I4;
		args[1].lVal = (LONG)type;
		args[0].vt = VT_I4;
		args[0].lVal = (LONG)key;
		invoke_v(m_event_root, m_event_wnd_on_drop, 4, args);
		return S_OK;
	}

	//IActiveScriptSite
	STDMETHOD(GetLCID)(LCID* plcid){ return E_NOTIMPL; }
	STDMETHOD(GetItemInfo)(LPCOLESTR name, DWORD mask, IUnknown** ppunk, ITypeInfo** ppti){
		if(ppti)  *ppti=0;
		if(ppunk) *ppunk=0;
		if(_wcsicmp(name, L"app")==0){
			if(ppti && (mask & SCRIPTINFO_ITYPEINFO)){
				this->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, ppti);
			}
			if(ppunk && (mask & SCRIPTINFO_IUNKNOWN)){
				(*ppunk) = (IUnknown*)(IApp*)this;
				(*ppunk)->AddRef();
			}
			return S_OK;
		}
		return TYPE_E_ELEMENTNOTFOUND;
	}
	STDMETHOD(GetDocVersionString)(BSTR* pstr){ return E_NOTIMPL; }
	STDMETHOD(OnScriptTerminate)(const VARIANT* result, const EXCEPINFO* excep){ return S_OK; }
	STDMETHOD(OnStateChange)(SCRIPTSTATE state){ return S_OK; }
	STDMETHOD(OnScriptError)(IActiveScriptError* err){ g_console.PutError(err); return S_OK;}
	STDMETHOD(OnEnterScript)(){ return S_OK; }
	STDMETHOD(OnLeaveScript)(){ return S_OK; }

	//IApp
	STDMETHOD(trace)(BSTR str){ g_console.PutInfo(str); return S_OK;}
	STDMETHOD(alert)(BSTR str){ g_console.PutError(str); return S_OK;}
	STDMETHOD(Quit)(){
		if(m_hwnd) DestroyWindow(m_hwnd);
		return S_OK;
	}
	STDMETHOD(get_Version)(BSTR* pp){
		*pp = Util::get_module_version(g_this_module, FALSE);
		return S_OK;
	}
	STDMETHOD(get_IsDesktopComposition)(VARIANT_BOOL* p){
		*p = Util::is_desktop_composition();
		return S_OK;
	}
	STDMETHOD(get_ActiveWindow)(UINT* p){
		*p = (UINT) GetActiveWindow();
		return S_OK;
	}
	STDMETHOD(get_Clipboard)(BSTR* pp){ *pp = Util::get_clipboard_text(); return S_OK;}
	STDMETHOD(put_Clipboard)(BSTR p){ Util::set_clipboard_text(p); return S_OK;}
	STDMETHOD(get_Env)(BSTR name, BSTR* pp){ *pp = Util::get_cygwin_env(name); return S_OK;}
	STDMETHOD(put_Env)(BSTR name, BSTR value){ Util::set_cygwin_env(name, value); return S_OK;}
	STDMETHOD(get_ShowConsole)(VARIANT_BOOL* p){ *p = g_console.IsShow(); return S_OK;}
	STDMETHOD(put_ShowConsole)(VARIANT_BOOL b){ g_console.Show(b); return S_OK;}
	STDMETHOD(get_CurrentDirectory)(BSTR* pp){ *pp = Util::get_cygwin_current_directory(); return S_OK;}
	STDMETHOD(put_CurrentDirectory)(BSTR cygpath){ Util::set_cygwin_current_directory(cygpath); return S_OK;}
	STDMETHOD(NewPty)(BSTR cmdline, IPty** pp){
		HRESULT hr = CreatePty(this, cmdline, pp);
		if(FAILED(hr)){
			*pp = 0;
			set_error_info(hr);
		}
		return hr;
	}
	STDMETHOD(NewWnd)(IWnd** pp){
		HRESULT hr = CreateWnd(this, pp);
		if(FAILED(hr)){
			*pp = 0;
			set_error_info(hr);
		}
		return hr;
	}
	STDMETHOD(AddMenu)(int id, BSTR str, VARIANT_BOOL check){
		if(m_work_menu){
			MENUITEMINFO mi;
			mi.cbSize = sizeof(mi);
			mi.fMask = MIIM_STRING | MIIM_ID | MIIM_STATE;
			mi.fState = 0;
			if(check)   mi.fState |= MFS_CHECKED;
			//if(!enable) mi.fState |= MFS_DISABLED;
			mi.wID = (id << 4);
			mi.dwTypeData = (LPWSTR)str;
			mi.cch = (int) SysStringLen(str);
			InsertMenuItem(m_work_menu, GetMenuItemCount(m_work_menu), TRUE, &mi);
		}
		return S_OK;
	}
	STDMETHOD(AddMenuSeparator)(){
		if(m_work_menu){
			MENUITEMINFO mi;
			mi.cbSize = sizeof(mi);
			mi.fType = MFT_SEPARATOR;
			mi.fMask = MIIM_FTYPE;
			InsertMenuItem(m_work_menu, GetMenuItemCount(m_work_menu), TRUE, &mi);
		}
		return S_OK;
	}
	STDMETHOD(ToCygwinPath)(BSTR src, BSTR* pp){ *pp = Util::to_cygwin_path(src);return S_OK;}
	STDMETHOD(ToWindowsPath)(BSTR src, BSTR* pp){ *pp = Util::to_windows_path(src);return S_OK;}
	STDMETHOD(EvalFile)(BSTR cygpath){
		BSTR winpath = Util::to_windows_path(cygpath);
		if(winpath){
			LPWSTR text = Util::load_script_file(winpath);
			if(text){
				IActiveScriptParse* parser = 0;
				if(SUCCEEDED(m_engine->QueryInterface(&parser))){
					parser->ParseScriptText(text, 0,0,0,1, 0,SCRIPTTEXT_ISEXPRESSION | SCRIPTTEXT_ISVISIBLE, 0,0);
					parser->Release();
				}
				delete [] text;
			}
			SysFreeString(winpath);
		}
		return S_OK;
	}
};

App* App::g_app = 0;

void App::wm_on_nc_destroy(){
	::trace("App::wm_on_nc_destroy\n");
	if(m_event_root){
		m_event_root.Release();
	}
	if(m_engine){
		m_engine->Close();
		m_engine.Release();
	}
	if(m_hwnd){
		DestroyWindow(m_hwnd);
	}
}
App::~App(){
	wm_on_nc_destroy();//
	g_app = NULL;
	::trace("App::dtor\n");
}

App::App()
	: m_ref(0),
	  m_hwnd(0),
	  m_event_app_on_startup(0),
	  m_event_app_on_new_command(0),
	  m_event_wnd_on_closed(0),
	  m_event_wnd_on_key_down(0),
	  m_event_wnd_on_mouse_wheel(0),
	  m_event_wnd_on_menu_init(0),
	  m_event_wnd_on_menu_execute(0),
	  m_event_wnd_on_title_init(0),
	  m_event_wnd_on_drop(0),
	  m_event_pty_on_closed(0),
	  m_event_pty_on_update_screen(0),
	  m_event_pty_on_update_title(0),
	  m_event_pty_on_req_font(0),
	  m_event_pty_on_req_move(0),
	  m_event_pty_on_req_resize(0),
	  m_work_menu(0){
	g_app = this;
	::trace("App::ctor\n");
}
HRESULT App::initialize(){
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = _wndproc;
	wc.hInstance = g_this_module;
	wc.lpszClassName = L"ckApplicationClass";
	if(!RegisterClassEx(&wc))
		return E_FAIL;

	HWND hwnd = CreateWindowEx(0, wc.lpszClassName,NULL, 0, 0,0,0,0, HWND_MESSAGE,0,g_this_module,this);
	if(!hwnd)
		return E_FAIL;

	HRESULT  hr = S_OK;
	LPCWSTR  script0 = 0;
	IActiveScriptParse* parser = 0;

	if(SUCCEEDED(hr)){ script0=Util::load_script_rsrc(MAKEINTRESOURCE(201)); if(!script0) hr=E_FAIL; }
	if(SUCCEEDED(hr)) hr = m_engine.CreateInstance(L"JScript");
	if(SUCCEEDED(hr)) hr=  m_engine->QueryInterface(&parser);
	if(SUCCEEDED(hr)) hr = m_engine->SetScriptSite(this);
	if(SUCCEEDED(hr)) hr = parser->InitNew();
	if(SUCCEEDED(hr)) hr = m_engine->AddNamedItem(L"app", SCRIPTITEM_GLOBALMEMBERS | SCRIPTITEM_ISVISIBLE | SCRIPTITEM_ISSOURCE);
	if(SUCCEEDED(hr) && script0) hr = parser->ParseScriptText(script0, 0,0,0,0, 0,SCRIPTTEXT_ISEXPRESSION | SCRIPTTEXT_ISVISIBLE, 0,0);
	if(SUCCEEDED(hr)) hr = m_engine->SetScriptState(SCRIPTSTATE_CONNECTED);
	if(SUCCEEDED(hr)){
		IDispatchPtr root;
		hr = m_engine->GetScriptDispatch(NULL, &root);
		if(SUCCEEDED(hr)){
			DISPID ev = get_disp_id(root, L"Events");
			DISPPARAMS param = {NULL,NULL, 0,0};
			_variant_t retval;
			hr = root->Invoke(ev, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &param, &retval,0,0);
			if(SUCCEEDED(hr)){
				m_event_root = retval;
				if(m_event_root == NULL)
					hr = E_FAIL;
			}
		}
	}
	if(SUCCEEDED(hr)){
		m_event_app_on_startup      = get_disp_id(m_event_root, L"app_on_startup");
		m_event_app_on_new_command  = get_disp_id(m_event_root, L"app_on_new_command");
		m_event_wnd_on_closed       = get_disp_id(m_event_root, L"wnd_on_closed");
		m_event_wnd_on_key_down     = get_disp_id(m_event_root, L"wnd_on_key_down");
		m_event_wnd_on_mouse_wheel  = get_disp_id(m_event_root, L"wnd_on_mouse_wheel");
		m_event_wnd_on_menu_init    = get_disp_id(m_event_root, L"wnd_on_menu_init");
		m_event_wnd_on_menu_execute = get_disp_id(m_event_root, L"wnd_on_menu_execute");
		m_event_wnd_on_title_init   = get_disp_id(m_event_root, L"wnd_on_title_init");
		m_event_wnd_on_drop         = get_disp_id(m_event_root, L"wnd_on_drop");
		m_event_pty_on_closed       = get_disp_id(m_event_root, L"pty_on_closed");
		m_event_pty_on_update_screen= get_disp_id(m_event_root, L"pty_on_update_screen");
		m_event_pty_on_update_title = get_disp_id(m_event_root, L"pty_on_update_title");
		m_event_pty_on_req_font     = get_disp_id(m_event_root, L"pty_on_req_font");
		m_event_pty_on_req_move     = get_disp_id(m_event_root, L"pty_on_req_move");
		m_event_pty_on_req_resize   = get_disp_id(m_event_root, L"pty_on_req_resize");
		//
		SAFEARRAY* sa = cmdline_to_vbarray(GetCommandLine());
		if(!sa)
			hr = E_FAIL;
		else{
			VARIANTARG args[1];
			args[0].vt = VT_ARRAY | VT_VARIANT;
			args[0].parray = sa;
			hr = invoke_v(m_event_root, m_event_app_on_startup, 1, args);
			SafeArrayDestroy(sa);
		}
	}
	if(parser) parser->Release();
	if(script0) delete [] script0;
	return hr;
}

}//namespace Ck


extern "C" __declspec(dllexport) void AppStartup(){
	::trace(">>AppStartup pid=%d tid=%d\n", GetCurrentProcessId(), GetCurrentThreadId());

	#ifdef _DEBUG
	SetUnhandledExceptionFilter(0);
	char* a = (char*) malloc(1);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetDbgFlag(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif

	{
		BSTR bs = Ck::Util::get_module_version(GetModuleHandle(0), TRUE);
		Ck::g_console.PutInfo(L"[exe] version ");
		Ck::g_console.PutInfo(bs);
		SysFreeString(bs);
		bs = Ck::Util::get_module_version(g_this_module, TRUE);
		Ck::g_console.PutInfo(L"\n[dll] version ");
		Ck::g_console.PutInfo(bs);
		SysFreeString(bs);
		Ck::g_console.PutInfo(L"\n");
	}

	HRESULT  hr;
	ULONG_PTR  gdip_token = 0;
	Gdiplus::GdiplusStartupInput   gdip_in;
	Gdiplus::GdiplusStartupOutput  gdip_out;

	if(Gdiplus::Ok != Gdiplus::GdiplusStartup(&gdip_token, &gdip_in, &gdip_out)){
		hr = E_FAIL;
	}
	else{
		Ck::App* app = 0;
		try{
			app = new Ck::App();
			app->AddRef();
			hr = app->initialize();
			if(SUCCEEDED(hr)){
				::trace(">>MessageLoop\n");
				MSG msg;
				while(GetMessage(&msg,0,0,0)){
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				::trace("<<MessageLoop\n");
			}
		}
		catch(HRESULT e){
			hr = e;
		}
		catch(std::bad_alloc&){
			hr = E_OUTOFMEMORY;
		}
		catch(...){
			hr = E_FAIL;
		}
		if(app) app->Release();
	}

	Gdiplus::GdiplusShutdown(gdip_token);
	if(FAILED(hr)){
		Ck::g_console.PutError(hr);
		MessageBox(NULL, L"error", L"error", MB_OK|MB_ICONSTOP);
	}
	::trace("<<AppStartup\n");
}


extern "C" BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID lp){
	if(reason == DLL_PROCESS_ATTACH){
		g_this_module = module;
		{
			module = GetModuleHandle(0);//ck.con.exe
			if(!module) return FALSE;
			cyg_execpty = (HRESULT(*)(LPCWSTR,int*,int*)) GetProcAddress(module, "execpty");

			module = GetModuleHandle(L"cygwin1.dll");//cygwin1.dll
			if(!module) return FALSE;
			cyg_getenv = (char*(*)(const char*)) GetProcAddress(module, "getenv");
			cyg_setenv = (int(*)(const char*,const char*,int)) GetProcAddress(module, "setenv");
			cyg_unsetenv = (int(*)(const char*)) GetProcAddress(module, "unsetenv");
			cyg_getcwd = (char*(*)(char*,size_t)) GetProcAddress(module, "getcwd");
			cyg_errno = (int*(*)()) GetProcAddress(module, "__errno");

			if(!cyg_execpty || !cyg_getenv || !cyg_setenv || !cyg_unsetenv || !cyg_getcwd || !cyg_errno)
				return FALSE;
		}
		{
			WCHAR path [MAX_PATH+1];
			UINT n = GetModuleFileName(g_this_module, path, MAX_PATH);
			path[n] = 0;
			HRESULT hr = LoadTypeLib(path, &g_typelib);
			if(FAILED(hr))
				return FALSE;
		}
	}
	else if(reason == DLL_PROCESS_DETACH){
		g_this_module = 0;
	}
	return TRUE;
}
//EOF
