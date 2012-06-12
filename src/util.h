#pragma once

//----------------------------------------------------------------------------
//app.cpp
extern ITypeLibPtr g_typelib;
extern HMODULE     g_this_module;
extern HRESULT (* cyg_execpty)(LPCWSTR,int*,int*);
extern char*   (* cyg_getenv)(const char*);
extern int     (* cyg_setenv)(const char*, const char*,int);
extern int     (* cyg_unsetenv)(const char*);
extern char*   (* cyg_getcwd)(char*,size_t);
extern int*    (* cyg_errno)();
extern void    trace(const char*, ...);


//----------------------------------------------------------------------------
//util.cpp

namespace Ck{
namespace Util{

BOOL show_window(HWND hwnd, int cmd);

BSTR get_module_version(HMODULE module, BOOL full);

LPWSTR load_script_rsrc(LPCWSTR name);
LPWSTR load_script_file(LPCWSTR path);

void set_clipboard_text(BSTR str);
BSTR get_clipboard_text();

void set_cygwin_env(BSTR name, BSTR value);
BSTR get_cygwin_env(BSTR name);

BSTR get_cygwin_current_directory();
void set_cygwin_current_directory(BSTR cygpath);

BSTR to_cygwin_path(BSTR src);
BSTR to_windows_path(BSTR src);

BOOL is_desktop_composition();
WinTransp  set_window_transp(HWND hwnd, WinTransp n);

}//namespace Util
}//namespace Ck


//----------------------------------------------------------------------------
//interface implement

#define BEGIN_COM_MAP \
	private: ULONG m_ref; \
	public: STDMETHOD(QueryInterface)(REFIID riid, void** pp){ \
	if(!pp) return E_POINTER; \

#define COM_INTERFACE_ENTRY(type) \
	else if(riid==__uuidof(type)) *pp = (type*)(this);

#define COM_INTERFACE_ENTRY2(type,type2) \
	else if(riid==__uuidof(type)) *pp = (type*)(type2*)(this);

#define END_COM_MAP \
	else { *pp=NULL; return E_NOINTERFACE;} \
	AddRef(); \
	return S_OK; \
	} \
	STDMETHOD_(ULONG, AddRef)(){return ++m_ref;} \
	STDMETHOD_(ULONG, Release)(){ULONG n=--m_ref;if(!n)delete this;return n;} \
	private:

//---

template<class T> class IDispatchImpl_: public T{
protected:
	static ITypeInfoPtr g_typeinfo;
	virtual ~IDispatchImpl_<T>(){}
public:
	IDispatchImpl_<T>(){
		if(!g_typeinfo){
			g_typelib->GetTypeInfoOfGuid(__uuidof(T),&g_typeinfo);
		}
	}
	STDMETHOD(GetTypeInfoCount)(unsigned int* n){
		if(!n) return E_POINTER;
		*n=1;
		return S_OK;
	}
	STDMETHOD(GetTypeInfo)(unsigned int i, LCID lcid, ITypeInfo** pp){
		if(!pp) return E_POINTER;
		if(i) return DISP_E_BADINDEX;
		if(!g_typeinfo) return E_FAIL;
		(*pp) = g_typeinfo;
		(*pp)->AddRef();
		return S_OK;
	}
	STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR** names, unsigned int cnames, LCID lcid, DISPID* dispids){
		if(!g_typeinfo) return E_FAIL;
		return g_typeinfo->GetIDsOfNames(names, cnames, dispids);
	}
	STDMETHOD(Invoke)(DISPID dispid, REFIID riid, LCID lcid, WORD flag, DISPPARAMS* params, VARIANT* result, EXCEPINFO* excep, unsigned int* err){
		if(!g_typeinfo) return E_FAIL;
		return g_typeinfo->Invoke(this, dispid, flag, params, result, excep, err);
	}
};
template<class T> ITypeInfoPtr IDispatchImpl_<T>::g_typeinfo;

//---

template<class T> class IUnknownImpl2: public T{
	BEGIN_COM_MAP
	COM_INTERFACE_ENTRY(T)
	COM_INTERFACE_ENTRY(IUnknown)
	END_COM_MAP
protected:
	virtual ~IUnknownImpl2<T>(){}
	IUnknownImpl2<T>():m_ref(0){}
};

//---

extern ITypeLibPtr g_typelib;

template<class T> class IDispatchImpl3: public IDispatchImpl_<T>{
	BEGIN_COM_MAP
	COM_INTERFACE_ENTRY(T)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IUnknown)
	END_COM_MAP
protected:
	virtual ~IDispatchImpl3<T>(){}
	IDispatchImpl3<T>():m_ref(0){}
};

//EOF
